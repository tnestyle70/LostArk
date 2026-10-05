#include "OcclusionCuller.h"

#pragma push_macro("new")
#pragma push_macro("min")
#pragma push_macro("max")
#undef new
#undef min
#undef max
#define PRECISE_COVERAGE 1
#define USE_D3D 1
#define USE_AVX512 0
#include "../ThirdPartyLib/MaskedOcclusionCulling/MaskedOcclusionCulling.h"
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#pragma pop_macro("max")
#pragma pop_macro("min")
#pragma pop_macro("new")

using namespace Engine;

namespace
{
    bool OcclusionFinite(const float* values, size_t count)
    {
        for (size_t i = 0u; i < count; ++i) if (!std::isfinite(values[i])) return false;
        return true;
    }

    bool OcclusionAffine(const float4x4_t& matrix)
    {
        return OcclusionFinite(&matrix._11, 16u) && matrix._14 == 0.f &&
            matrix._24 == 0.f && matrix._34 == 0.f && matrix._44 == 1.f;
    }

    // MOC compares 1/w. Admit only D3D perspective projections where z/w is
    // monotone in w and the near plane is a positive constant w.
    bool OcclusionPerspective(const float4x4_t& projection, float& nearW)
    {
        if (!OcclusionFinite(&projection._11, 16u) || projection._11 <= 0.f || projection._22 <= 0.f ||
            projection._12 != 0.f || projection._13 != 0.f || projection._14 != 0.f ||
            projection._21 != 0.f || projection._23 != 0.f || projection._24 != 0.f ||
            projection._41 != 0.f || projection._42 != 0.f || projection._44 != 0.f ||
            std::abs(projection._34) != 1.f) return false;
        const double slope = double(projection._33) / projection._34;
        const double nearValue = -double(projection._43) / slope;
        if (slope < 1.0 || !std::isfinite(nearValue) || nearValue <= 0.0 || nearValue > 1.e6) return false;
        nearW = static_cast<float>(nearValue);
        return std::isfinite(nearW) && nearW > 0.f;
    }

    void* OcclusionAllocate(size_t alignment, size_t bytes)
    {
        void* result = _aligned_malloc(bytes ? bytes : 1u, alignment);
        if (!result) throw std::bad_alloc();
        return result;
    }
    void OcclusionFree(void* memory) { _aligned_free(memory); }

    struct OCCLUSION_POINT final { double x = 0.0, y = 0.0; };

}

struct COcclusionCuller::STATE final
{
    std::unique_ptr<MaskedOcclusionCulling, void(*)(MaskedOcclusionCulling*)> rasterizer{nullptr, MaskedOcclusionCulling::Destroy};
    float4x4_t viewProjection{};
    vector<float4_t> clipPositions, triangles;
    vector<uint32_t> sequentialIndices;
    uint32_t width = 0u, height = 0u, inputTriangles = 0u, rasterizedTriangles = 0u;
    float nearW = 0.f, viewportScaleX = 1.f, viewportScaleY = 1.f;
    bool ready = false;
};

COcclusionCuller::COcclusionCuller(bool_t forceSse2) : m_ForceSse2(forceSse2) {}
COcclusionCuller::~COcclusionCuller() = default;

bool_t COcclusionCuller::Begin(const float4x4_t& view, const float4x4_t& projection,
    uint32_t viewportWidth, uint32_t viewportHeight)
{
    if (m_State) { m_State->ready = false; m_State->inputTriangles = m_State->rasterizedTriangles = 0u; }
    float nearW = 0.f;
    if (viewportWidth < 8u || viewportHeight < 4u || viewportWidth > 4096u || viewportHeight > 4096u || !OcclusionAffine(view) || !OcclusionPerspective(projection, nearW)) return false;
    try
    {
        if (!m_State) m_State = std::make_unique<STATE>();
        auto& state = *m_State;
        if (!state.rasterizer)
            state.rasterizer.reset(MaskedOcclusionCulling::Create(m_ForceSse2 ? MaskedOcclusionCulling::SSE2 :
                MaskedOcclusionCulling::AVX2, OcclusionAllocate, OcclusionFree));
        if (!state.rasterizer) return false;
        // Padded storage keeps exactly the viewport's pixel centers. A lower
        // resolution can hide subpixel openings; independently shrinking every
        // triangle instead creates artificial gaps along shared internal edges.
        const uint32_t width = (viewportWidth + 31u) / 32u * 32u;
        const uint32_t height = (viewportHeight + 7u) / 8u * 8u;
        state.viewportScaleX = static_cast<float>(viewportWidth) / width;
        state.viewportScaleY = static_cast<float>(viewportHeight) / height;
        if (state.width != width || state.height != height)
        {
            state.rasterizer->SetResolution(width, height);
            state.width = width; state.height = height;
        }
        XMStoreFloat4x4(&state.viewProjection, XMLoadFloat4x4(&view) * XMLoadFloat4x4(&projection));
        if (!OcclusionFinite(&state.viewProjection._11, 16u)) return false;
        state.nearW = nearW;
        state.rasterizer->SetNearClipPlane(nearW);
        state.rasterizer->ClearBuffer();
        state.ready = true;
        return true;
    }
    catch (const std::bad_alloc&)
    {
        // Upstream SetResolution releases the old allocation before allocating
        // the new one. Do not reuse that partially resized rasterizer on retry.
        if (m_State) { m_State->rasterizer.reset(); m_State->width = m_State->height = 0u; }
        return false;
    }
    catch (const std::length_error&) { return false; }
}

bool_t COcclusionCuller::Rasterize(const float* positions, uint32_t vertexCount,
    uint32_t strideBytes, std::span<const uint32_t> indices,
    const float4x4_t& world, CULL_MODE cullMode)
{
    if (!m_State || !m_State->ready || !positions || !vertexCount || vertexCount > MAXIMUM_INPUT_TRIANGLES * 3u ||
        strideBytes < 12u || strideBytes > 1024u || strideBytes % alignof(float) || indices.empty() || indices.size() % 3u ||
        indices.size() / 3u > MAXIMUM_INPUT_TRIANGLES - m_State->inputTriangles || !OcclusionAffine(world) ||
        (cullMode != CULL_MODE::NONE && cullMode != CULL_MODE::BACK && cullMode != CULL_MODE::FRONT)) return false;
    auto& state = *m_State;
    state.inputTriangles += static_cast<uint32_t>(indices.size() / 3u);
    try
    {
        state.clipPositions.resize(vertexCount);
        state.triangles.clear(); state.triangles.reserve(indices.size());
        const matrix_t matrix = XMLoadFloat4x4(&world) * XMLoadFloat4x4(&state.viewProjection);
        for (size_t i = 0u; i < vertexCount; ++i)
        {
            const auto* vertex = reinterpret_cast<const float3_t*>(reinterpret_cast<const uint8_t*>(positions) + i * strideBytes);
            if (!OcclusionFinite(&vertex->x, 3u)) return false;
            XMStoreFloat4(&state.clipPositions[i], XMVector4Transform(XMVectorSet(vertex->x, vertex->y, vertex->z, 1.f), matrix));
        }
        for (size_t triangle = 0u; triangle < indices.size(); triangle += 3u)
        {
            OCCLUSION_POINT points[3]; float farW = 0.f; bool usable = true;
            for (size_t corner = 0u; corner < 3u; ++corner)
            {
                const auto index = indices[triangle + corner]; if (index >= vertexCount) return false;
                const auto& vertex = state.clipPositions[index];
                if (!OcclusionFinite(&vertex.x, 4u) || vertex.w <= state.nearW + 1.e-4f ||
                    vertex.z <= 0.f || vertex.z >= vertex.w) { usable = false; break; }
                points[corner] = {state.viewportScaleX * (double(vertex.x) / vertex.w + 1.0) - 1.0,
                    state.viewportScaleY * (double(vertex.y) / vertex.w - 1.0) + 1.0};
                if (std::abs(points[corner].x) > 1.e5 || std::abs(points[corner].y) > 1.e5) { usable = false; break; }
                farW = (std::max)(farW, vertex.w);
            }
            if (!usable) continue;
            const double area = (points[1].x - points[0].x) * (points[2].y - points[0].y) -
                (points[1].y - points[0].y) * (points[2].x - points[0].x);
            // The product rasterizer uses FrontCounterClockwise = FALSE.
            // Its front-facing triangles have negative signed area in NDC.
            if (std::abs(area) < 1.e-16 || (cullMode == CULL_MODE::BACK && area >= 0.0) ||
                (cullMode == CULL_MODE::FRONT && area <= 0.0)) continue;
            if (area < 0.0) std::swap(points[1], points[2]);
            farW += (std::max)(.001f, farW * .0001f);
            for (const auto& point : points)
                state.triangles.push_back({static_cast<float>(point.x * farW), static_cast<float>(point.y * farW), 0.f, farW});
        }
        if (state.triangles.empty()) return true;
        if (state.sequentialIndices.size() < state.triangles.size())
        {
            const auto start = state.sequentialIndices.size(); state.sequentialIndices.resize(state.triangles.size());
            std::iota(state.sequentialIndices.begin() + start, state.sequentialIndices.end(), static_cast<uint32_t>(start));
        }
        state.rasterizer->RenderTriangles(&state.triangles.front().x, state.sequentialIndices.data(),
            static_cast<int>(state.triangles.size() / 3u), nullptr, MaskedOcclusionCulling::BACKFACE_NONE,
            MaskedOcclusionCulling::CLIP_PLANE_ALL);
        state.rasterizedTriangles += static_cast<uint32_t>(state.triangles.size() / 3u);
        return true;
    }
    catch (const std::bad_alloc&) { state.ready = false; return false; }
    catch (const std::length_error&) { state.ready = false; return false; }
}

bool_t COcclusionCuller::TestBounds(const float3_t& worldMin, const float3_t& worldMax) const
{
    if (!m_State || !m_State->ready || !m_State->rasterizedTriangles ||
        !OcclusionFinite(&worldMin.x, 3u) || !OcclusionFinite(&worldMax.x, 3u) ||
        worldMin.x > worldMax.x || worldMin.y > worldMax.y || worldMin.z > worldMax.z) return false;
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
            vertex.z <= 0.f || vertex.z >= vertex.w) return false;
        const double x = state.viewportScaleX * (double(vertex.x) / vertex.w + 1.0) - 1.0;
        const double y = state.viewportScaleY * (double(vertex.y) / vertex.w - 1.0) + 1.0;
        xMin = (std::min)(xMin, x); xMax = (std::max)(xMax, x);
        yMin = (std::min)(yMin, y); yMax = (std::max)(yMax, y); nearW = (std::min)(nearW, double(vertex.w));
    }
    // The closest enclosing box point is even closer than any model triangle.
    // W bias and two pixel expansion cover transform/coverage rounding.
    nearW -= (std::max)(.001, nearW * .0002);
    if (nearW <= state.nearW || xMin >= 1.0 || xMax <= -1.0 || yMin >= 1.0 || yMax <= -1.0) return false;
    xMin = (std::max)(-1.0, xMin - 4.0 / state.width); xMax = (std::min)(1.0, xMax + 4.0 / state.width);
    yMin = (std::max)(-1.0, yMin - 4.0 / state.height); yMax = (std::min)(1.0, yMax + 4.0 / state.height);
    if (xMin >= xMax || yMin >= yMax) return false;
    return state.rasterizer->TestRect(static_cast<float>(xMin), static_cast<float>(yMin),
        static_cast<float>(xMax), static_cast<float>(yMax), static_cast<float>(nearW)) == MaskedOcclusionCulling::OCCLUDED;
}

uint32_t COcclusionCuller::GetInputTriangles() const { return m_State ? m_State->inputTriangles : 0u; }
uint32_t COcclusionCuller::GetRasterizedTriangles() const { return m_State ? m_State->rasterizedTriangles : 0u; }
uint32_t COcclusionCuller::GetWidth() const { return m_State ? m_State->width : 0u; }
uint32_t COcclusionCuller::GetHeight() const { return m_State ? m_State->height : 0u; }
uint32_t COcclusionCuller::GetImplementation() const
{ return m_State && m_State->rasterizer ? static_cast<uint32_t>(m_State->rasterizer->GetImplementation()) : UINT32_MAX; }
