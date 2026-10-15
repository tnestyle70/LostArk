// Renderer가 사용하는 CPU 가림 판정과 입력 형상 검증을 구현한다.
#include "OcclusionCuller.h"

//Windows min/max 매크로가 외부 backend와 표준 헤더를 바꾸지 않게 잠시 해제하고 복구한다.
#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max
// 제품의 D3D coverage 규칙을 사용하며 이 번역 단위의 backend 구성은 AVX512를 사용하지 않는다.
#define PRECISE_COVERAGE 1
#define USE_D3D 1
#define USE_AVX512 0
// SIMD 기반 CPU 깊이 buffer와 삼각형/사각형 query API다. 생성/파괴는 backend의 짝을 사용한다.
#include "../ThirdPartyLib/MaskedOcclusionCulling/MaskedOcclusionCulling.h"
// 유한성·오차 한계·연속 index 생성과 할당/크기 예외 처리를 사용한다.
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#pragma pop_macro("max")
#pragma pop_macro("min")

using namespace Engine;

namespace
{
    // 차용한 연속 float 범위의 NaN/무한대를 거른다. 메모리 길이와 수명은 호출자가 보장한다.
    bool OcclusionFinite(const float* values, size_t count)
    {
        for (size_t i = 0u; i < count; ++i)
            if (!std::isfinite(values[i]))
                return false;
        return true;
    }

    // row-vector 행렬의 원근 성분이 없고 동차 성분이 정확히 1인지 검사한다. object world에는 이 조건을 그대로 적용한다.
    bool OcclusionAffine(const float4x4_t& matrix)
    {
        return OcclusionFinite(&matrix._11, 16u) && matrix._14 == 0.f &&
            matrix._24 == 0.f && matrix._34 == 0.f && matrix._44 == 1.f;
    }

    // CCamera의 역행렬은 동차 성분이 1에서 몇 ULP 벗어날 수 있다.
    // Renderer가 전달한 view만 정규화해 동일한 원근 나눗셈을 유지한다.
    // projective/비정상 입력은 거부하고, Rasterize의 object world 검증은 유지한다.
    bool OcclusionCameraAffine(const float4x4_t& source, float4x4_t& canonical)
    {
        if (!OcclusionFinite(&source._11, 16u) || source._14 != 0.f ||
            source._24 != 0.f || source._34 != 0.f ||
            std::abs(double(source._44) - 1.) > 32. * std::numeric_limits<float>::epsilon())
            return false;
        for (size_t row = 0u; row < 4u; ++row)
            for (size_t column = 0u; column < 4u; ++column)
                canonical.m[row][column] = static_cast<float>(double(source.m[row][column]) / source._44);
        return OcclusionAffine(canonical);
    }

    // backend는 1/w 깊이를 비교한다. z/w가 w에 따라 단조롭게 증가하고 near가 양의 상수 w인 D3D 원근 투영만 받는다.
    // _31/_32의 투영 offset은 허용하지만 직교 투영이나 역방향 깊이 행렬은 이 경로에서 받지 않는다.
    bool OcclusionPerspective(const float4x4_t& projection, float& nearW)
    {
        if (!OcclusionFinite(&projection._11, 16u) || projection._11 <= 0.f || projection._22 <= 0.f ||
            projection._12 != 0.f || projection._13 != 0.f || projection._14 != 0.f ||
            projection._21 != 0.f || projection._23 != 0.f || projection._24 != 0.f ||
            projection._41 != 0.f || projection._42 != 0.f || projection._44 != 0.f ||
            std::abs(projection._34) != 1.f)
                return false;
        const double slope = double(projection._33) / projection._34;
        const double nearValue = -double(projection._43) / slope;
        if (slope < 1.0 || !std::isfinite(nearValue) || nearValue <= 0.0 || nearValue > 1.e6)
            return false;
        nearW = static_cast<float>(nearValue);
        return std::isfinite(nearW) && nearW > 0.f;
    }

    // backend의 정렬 요구를 CRT 할당에 연결한다. 실패를 bad_alloc으로 변환해 호출부에서 판정을 중단하게 한다.
    void* OcclusionAllocate(size_t alignment, size_t bytes)
    {
        void* result = _aligned_malloc(bytes ? bytes : 1u, alignment);
        if (!result)
            throw std::bad_alloc();
        return result;
    }
    // OcclusionAllocate와 짝을 이루는 backend 해제 callback이다.
    void OcclusionFree(void* memory)
    {
        _aligned_free(memory);
    }

    // padding을 반영한 NDC XY다. winding/면적 계산은 double로 수행하고 제출 시 float clip 값으로 바꾼다.
    struct OCCLUSION_POINT final
    {
        double x = 0.0, y = 0.0;
    };

}

// 판정기가 소유하는 CPU 상태다. 원본 장면/정점 포인터 대신 변환한 위치와 제출 삼각형을 복사해 보관한다.
struct COcclusionCuller::STATE final
{
    // 일반 delete 대신 backend Destroy로 해제해야 내부 정렬 메모리도 정리된다.
    std::unique_ptr<MaskedOcclusionCulling, void(*)(MaskedOcclusionCulling*)> rasterizer{nullptr, MaskedOcclusionCulling::Destroy};
    // 카메라 합성 행렬과 CPU 작업 배열이다. clipPositions는 정점별, triangles는 선별 삼각형의 3개 점씩 저장한다.
    float4x4_t viewProjection{};
    vector<float4_t> clipPositions, triangles;
    vector<uint32_t> sequentialIndices;
    // 저장 해상도와 이번 Begin의 입력/제출 통계다. ready만 현재 깊이 query의 사용 가능 여부를 나타낸다.
    uint32_t width = 0u, height = 0u, inputTriangles = 0u, rasterizedTriangles = 0u;
    float nearW = 0.f, viewportScaleX = 1.f, viewportScaleY = 1.f;
    bool ready = false;
};

// backend 선택 옵션만 저장한다. 실제 메모리는 첫 Begin에서 필요할 때 준비한다.
COcclusionCuller::COcclusionCuller(bool_t forceSse2)
    : m_ForceSse2(forceSse2)
{
}

// STATE의 unique_ptr와 backend Destroy deleter가 소유 메모리를 차례로 정리한다.
COcclusionCuller::~COcclusionCuller() = default;

// Renderer가 새 깊이를 구축할 때 호출한다. 입력/준비 실패가 이전 카메라의 숨김 판정을 재사용하지 않게 ready를 먼저 내린다.
bool_t COcclusionCuller::Begin(const float4x4_t& view, const float4x4_t& projection,
    uint32_t viewportWidth, uint32_t viewportHeight)
{
    if (m_State)
    {
        m_State->ready = false;
        m_State->inputTriangles = m_State->rasterizedTriangles = 0u;
    }
    float nearW = 0.f;
    float4x4_t canonicalView{};
    if (viewportWidth < 8u || viewportHeight < 4u || viewportWidth > 4096u || viewportHeight > 4096u ||
        !OcclusionCameraAffine(view, canonicalView) || !OcclusionPerspective(projection, nearW))
            return false;
    try
    {
        if (!m_State)
            m_State = std::make_unique<STATE>();
        auto& state = *m_State;
        if (!state.rasterizer)
            state.rasterizer.reset(MaskedOcclusionCulling::Create(m_ForceSse2 ? MaskedOcclusionCulling::SSE2 :
                MaskedOcclusionCulling::AVX2, OcclusionAllocate, OcclusionFree));
        if (!state.rasterizer)
            return false;
        // 해상도를 줄이지 않고 저장 영역만 올림해 실제 viewport의 pixel 중심을 유지한다.
        // 축별 비율은 이후 NDC를 이 저장 영역으로 옮길 때 사용한다. 삼각형마다 별도 축소를 하지는 않는다.
        const uint32_t width = (viewportWidth + 31u) / 32u * 32u;
        const uint32_t height = (viewportHeight + 7u) / 8u * 8u;
        state.viewportScaleX = static_cast<float>(viewportWidth) / width;
        state.viewportScaleY = static_cast<float>(viewportHeight) / height;
        if (state.width != width || state.height != height)
        {
            state.rasterizer->SetResolution(width, height);
            state.width = width;
            state.height = height;
        }
        XMStoreFloat4x4(&state.viewProjection, XMLoadFloat4x4(&canonicalView) * XMLoadFloat4x4(&projection));
        if (!OcclusionFinite(&state.viewProjection._11, 16u))
            return false;
        state.nearW = nearW;
        state.rasterizer->SetNearClipPlane(nearW);
        state.rasterizer->ClearBuffer();
        state.ready = true;
        return true;
    }
    catch (const std::bad_alloc&)
    {
        // backend의 SetResolution은 이전 메모리를 해제한 뒤 새 영역을 할당한다.
        // 도중 실패한 rasterizer는 폐기해 다음 Begin이 불완전한 크기 상태를 재사용하지 않게 한다.
        if (m_State)
        {
            m_State->rasterizer.reset();
            m_State->width = m_State->height = 0u;
        }
        return false;
    }
    catch (const std::length_error&)
    {
        return false;
    }
}

// 현재 Begin의 깊이에 불투명 형상을 누적한다. near/far 경계에 걸친 삼각형은 가림 기여에서 제외한다.
bool_t COcclusionCuller::Rasterize(const float* positions, uint32_t vertexCount,
    uint32_t strideBytes, std::span<const uint32_t> indices,
    const float4x4_t& world, CULL_MODE cullMode)
{
    if (!m_State || !m_State->ready || !positions || !vertexCount || vertexCount > MAXIMUM_INPUT_TRIANGLES * 3u ||
        strideBytes < 12u || strideBytes > 1024u || strideBytes % alignof(float) || indices.empty() || indices.size() % 3u ||
        indices.size() / 3u > MAXIMUM_INPUT_TRIANGLES - m_State->inputTriangles || !OcclusionAffine(world) ||
        (cullMode != CULL_MODE::NONE && cullMode != CULL_MODE::BACK && cullMode != CULL_MODE::FRONT))
            return false;
    // 여기서 입력 예산을 소비한다. 이후 잘못된 정점/index로 false여도 예산과 이전에 누적한 깊이는 유지한다.
    auto& state = *m_State;
    state.inputTriangles += static_cast<uint32_t>(indices.size() / 3u);
    try
    {
        // 이번 입력을 먼저 지역 작업 배열에 변환/선별한다. 선별을 끝내기 전에는 backend 깊이에 제출하지 않는다.
        state.clipPositions.resize(vertexCount);
        state.triangles.clear();
        state.triangles.reserve(indices.size());
        const matrix_t matrix = XMLoadFloat4x4(&world) * XMLoadFloat4x4(&state.viewProjection);
        for (size_t i = 0u; i < vertexCount; ++i)
        {
            const auto* vertex = reinterpret_cast<const float3_t*>(reinterpret_cast<const uint8_t*>(positions) + i * strideBytes);
            if (!OcclusionFinite(&vertex->x, 3u))
                return false;
            XMStoreFloat4(&state.clipPositions[i], XMVector4Transform(XMVectorSet(vertex->x, vertex->y, vertex->z, 1.f), matrix));
        }
        // 정점이 near/far 내부에 완전히 들어오는 삼각형만 선별한다. 제외한 삼각형은 숨김 기회를 줄이는 방향으로 처리된다.
        for (size_t triangle = 0u; triangle < indices.size(); triangle += 3u)
        {
            OCCLUSION_POINT points[3];
            float farW = 0.f;
            bool usable = true;
            for (size_t corner = 0u; corner < 3u; ++corner)
            {
                const auto index = indices[triangle + corner];
                if (index >= vertexCount)
                    return false;
                const auto& vertex = state.clipPositions[index];
                if (!OcclusionFinite(&vertex.x, 4u) || vertex.w <= state.nearW + 1.e-4f ||
                    vertex.z <= 0.f || vertex.z >= vertex.w)
                {
                    usable = false;
                    break;
                }
                points[corner] = {state.viewportScaleX * (double(vertex.x) / vertex.w + 1.0) - 1.0,
                    state.viewportScaleY * (double(vertex.y) / vertex.w - 1.0) + 1.0};
                if (std::abs(points[corner].x) > 1.e5 || std::abs(points[corner].y) > 1.e5)
                {
                    usable = false;
                    break;
                }
                farW = (std::max)(farW, vertex.w);
            }
            if (!usable)
                continue;
            const double area = (points[1].x - points[0].x) * (points[2].y - points[0].y) -
                (points[1].y - points[0].y) * (points[2].x - points[0].x);
            // 제품 rasterizer의 FrontCounterClockwise=FALSE에 맞춰 NDC 부호 면적이 음수인 삼각형을 앞면으로 본다.
            // 요청한 면 제거를 마친 뒤 backend 제출용 winding을 통일한다.
            if (std::abs(area) < 1.e-16 || (cullMode == CULL_MODE::BACK && area >= 0.0) ||
                (cullMode == CULL_MODE::FRONT && area <= 0.0))
                    continue;
            if (area < 0.0)
                std::swap(points[1], points[2]);
            // 가림 삼각형의 세 점에 가장 먼 w와 작은 양의 여유를 공통 적용한다. 실제 면보다 가까운 가짜 깊이를 만들지 않는다.
            farW += (std::max)(.001f, farW * .0001f);
            for (const auto& point : points)
                state.triangles.push_back({static_cast<float>(point.x * farW), static_cast<float>(point.y * farW), 0.f, farW});
        }
        // 모든 삼각형이 제외돼도 입력 처리는 성공이다. 제출 통계와 깊이는 이 호출에서 증가하지 않는다.
        if (state.triangles.empty())
            return true;
        if (state.sequentialIndices.size() < state.triangles.size())
        {
            const auto start = state.sequentialIndices.size();
            state.sequentialIndices.resize(state.triangles.size());
            std::iota(state.sequentialIndices.begin() + start, state.sequentialIndices.end(), static_cast<uint32_t>(start));
        }
        // 이미 변환/면 선택을 끝낸 XYZ W 배열과 연속 index를 제출한다. backend가 쓰는 좌표는 X/Y/W다.
        state.rasterizer->RenderTriangles(&state.triangles.front().x, state.sequentialIndices.data(),
            static_cast<int>(state.triangles.size() / 3u), nullptr, MaskedOcclusionCulling::BACKFACE_NONE,
            MaskedOcclusionCulling::CLIP_PLANE_ALL);
        state.rasterizedTriangles += static_cast<uint32_t>(state.triangles.size() / 3u);
        return true;
    }
    catch (const std::bad_alloc&)
    {
        state.ready = false;
        return false;
    }
    catch (const std::length_error&)
    {
        state.ready = false;
        return false;
    }
}

// 월드 AABB의 여덟 모서리로 보수적인 화면 사각형과 가장 가까운 clip w를 만든다. true만 Renderer의 숨김 근거다.
bool_t COcclusionCuller::TestBounds(const float3_t& worldMin, const float3_t& worldMax) const
{
    if (!m_State || !m_State->ready || !m_State->rasterizedTriangles ||
        !OcclusionFinite(&worldMin.x, 3u) || !OcclusionFinite(&worldMax.x, 3u) ||
        worldMin.x > worldMax.x || worldMin.y > worldMax.y || worldMin.z > worldMax.z)
            return false;
    // 입력 AABB가 유한하고 min<=max인지 확인한 뒤 현재 카메라로만 변환한다.
    const auto& state = *m_State;
    const matrix_t matrix = XMLoadFloat4x4(&state.viewProjection);
    const double limit = (std::numeric_limits<double>::max)();
    double xMin = limit, yMin = limit, xMax = -limit, yMax = -limit, nearW = limit;
    for (uint32_t corner = 0u; corner < 8u; ++corner)
    {
        float4_t vertex;
        XMStoreFloat4(&vertex, XMVector4Transform(XMVectorSet(corner & 1u ? worldMax.x : worldMin.x,
            corner & 2u ? worldMax.y : worldMin.y, corner & 4u ? worldMax.z : worldMin.z, 1.f), matrix));
        if (!OcclusionFinite(&vertex.x, 4u) || vertex.w <= state.nearW + 1.e-4f ||
            vertex.z <= 0.f || vertex.z >= vertex.w)
                return false;
        const double x = state.viewportScaleX * (double(vertex.x) / vertex.w + 1.0) - 1.0;
        const double y = state.viewportScaleY * (double(vertex.y) / vertex.w - 1.0) + 1.0;
        xMin = (std::min)(xMin, x);
        xMax = (std::max)(xMax, x);
        yMin = (std::min)(yMin, y);
        yMax = (std::max)(yMax, y);
        nearW = (std::min)(nearW, double(vertex.w));
    }
    // box의 가장 가까운 w를 더 당기고 화면 사각형을 양쪽으로 2 pixel씩 넓힌다.
    // 변환/coverage 반올림으로 경계의 실제 형상을 과도하게 숨기지 않도록 보수적인 질의를 만든다.
    nearW -= (std::max)(.001, nearW * .0002);
    if (nearW <= state.nearW || xMin >= 1.0 || xMax <= -1.0 || yMin >= 1.0 || yMax <= -1.0)
        return false;
    xMin = (std::max)(-1.0, xMin - 4.0 / state.width);
    xMax = (std::min)(1.0, xMax + 4.0 / state.width);
    yMin = (std::max)(-1.0, yMin - 4.0 / state.height);
    yMax = (std::min)(1.0, yMax + 4.0 / state.height);
    if (xMin >= xMax || yMin >= yMax)
        return false;
    // backend의 VISIBLE/VIEW_CULLED는 숨김 근거로 쓰지 않고 OCCLUDED일 때만 true를 반환한다.
    return state.rasterizer->TestRect(static_cast<float>(xMin), static_cast<float>(yMin),
        static_cast<float>(xMax), static_cast<float>(yMax), static_cast<float>(nearW)) == MaskedOcclusionCulling::OCCLUDED;
}

// 이번 Begin에서 소비한 입력 예산이다. 후속 검증 실패분도 다시 반환하지 않는다.
uint32_t COcclusionCuller::GetInputTriangles() const
{
    return m_State ? m_State->inputTriangles : 0u;
}

// backend에 제출한 삼각형 수로 profiler 통계를 만든다. 실제 화면 coverage를 보장하는 수는 아니다.
uint32_t COcclusionCuller::GetRasterizedTriangles() const
{
    return m_State ? m_State->rasterizedTriangles : 0u;
}

// backend 저장 영역의 가로 pixel 수다. ready=false여도 이전 준비 크기가 남을 수 있다.
uint32_t COcclusionCuller::GetWidth() const
{
    return m_State ? m_State->width : 0u;
}

// backend 저장 영역의 세로 pixel 수다. 원래 viewport 높이와 구분한다.
uint32_t COcclusionCuller::GetHeight() const
{
    return m_State ? m_State->height : 0u;
}

// 실제로 생성된 backend의 SIMD 종류를 반환한다. 현재 프레임 유효성 검사를 대신하지 않는다.
uint32_t COcclusionCuller::GetImplementation() const
{
    return m_State && m_State->rasterizer ?
        static_cast<uint32_t>(m_State->rasterizer->GetImplementation()) : UINT32_MAX;
}
