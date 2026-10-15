// 외부 simplifier API를 호출하되 원본 정점은 유지하고 파생 인덱스만 생성한다.
#include "../ThirdPartyLib/meshoptimizer/meshoptimizer.h"
// 입력 정점 레이아웃과 LOD 소유/선택 인터페이스를 가져온다.
#include "Engine_VertexTypes.h"
#include "StaticMeshLod.h"

// 입력 범위·유한값 검사와 수치 한계·메모리 크기 예외를 처리한다.
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
// 입력/결과 SHA-256으로 선택적 디스크 캐시의 정합성을 확인한다.
#include <bcrypt.h>
#include <filesystem>
#include <fstream>
#pragma comment(lib, "bcrypt.lib")

NS_BEGIN(Engine)

namespace
{
    // 다시 만들 수 있는 파생 캐시이며 모델 로드의 정본은 아니다.
    // 단순화 규칙이 달라지면 version과 알고리즘 표식을 함께 변경해야 한다.
    struct LOD_CACHE_RANGE final
    {
        uint32_t count = 0u, first = 0u;
        float error = 0.f;
    };
    struct LOD_CACHE_HEADER final
    {
        uint32_t magic = 0x33444f4cu, version = 3u;
        uint32_t vertices = 0u, sourceIndices = 0u, rangeCount = 1u, combinedCount = 0u;
        std::array<LOD_CACHE_RANGE, CStaticMeshLod::MAXIMUM_RANGES> ranges{};
        std::array<uint8_t, 32> input{}, output{};
    };
    static_assert(sizeof(LOD_CACHE_HEADER) == 148u);

    // CNG algorithm/hash 핸들을 한 계산 동안 소유한다. 초기화나 Add가 실패하면 Finish도 실패하고 소멸 시 핸들을 정리한다.
    class LodHash final
    {
    public:
        LodHash()
        {
            m_Valid = BCryptOpenAlgorithmProvider(&m_Algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) >= 0 &&
                BCryptCreateHash(m_Algorithm, &m_Hash, nullptr, 0, nullptr, 0, 0) >= 0;
        }
        ~LodHash()
        {
            if (m_Hash)
                BCryptDestroyHash(m_Hash);
            if (m_Algorithm)
                BCryptCloseAlgorithmProvider(m_Algorithm, 0);
        }
        // 데이터는 호출 중에만 읽는다. CNG의 ULONG 길이를 넘는 입력은 분할하지 않고 계산을 무효화한다.
        void Add(const void* data, size_t bytes)
        {
            if (!m_Valid || bytes == 0u)
                return;
            if (!data || bytes > ULONG_MAX)
            {
                m_Valid = false;
                return;
            }
            m_Valid = BCryptHashData(m_Hash, static_cast<PUCHAR>(const_cast<void*>(data)),
                static_cast<ULONG>(bytes), 0) >= 0;
        }
        // 누적 데이터의 digest를 기록한다. 호출자는 true인 경우에만 출력 digest를 사용한다.
        bool Finish(std::array<uint8_t, 32>& digest)
        {
            return m_Valid && BCryptFinishHash(m_Hash, digest.data(), static_cast<ULONG>(digest.size()), 0) >= 0;
        }
    private:
        BCRYPT_ALG_HANDLE m_Algorithm = nullptr;
        BCRYPT_HASH_HANDLE m_Hash = nullptr;
        bool m_Valid = false;
    };

    // 입력 정점·인덱스·알고리즘 표식을 함께 해시한다. 생성 규칙이 다른 캐시를 재사용하지 않게 하는 식별자다.
    bool LodInputHash(std::span<const VTXMESH> vertices, std::span<const uint32_t> indices,
        std::array<uint8_t, 32>& digest, bool_t preserveAttributes)
    {
        // VTXMESH는 float 채널 18개와 color uint 하나로 구성된다.
        // 크기/오프셋 단언으로 해시 입력에 예상 밖의 padding이 들어가는 레이아웃 변경을 막는다.
        static_assert(sizeof(VTXMESH) == 18u * sizeof(float) + sizeof(uint32_t));
        static_assert(offsetof(VTXMESH, color0Rgba8) == 64u && offsetof(VTXMESH, vTexcoord2) == 68u);
        constexpr char algorithm[] = "StaticLod-v3-meshopt73583c335e541c139821d0de2bf5f12960a04941-attrs19-uv100fine-uv10far-border-.001-.002-.01-.02-.55-.30-.85";
        const uint64_t counts[] = { vertices.size(), indices.size() };
        LodHash hash;
        hash.Add(algorithm, sizeof(algorithm));
        // 기존 opaque v3 hash는 유지한다. 더 엄격한 재질용 생성 규칙은 별도 v4 정본이다.
        constexpr char attributeAlgorithm[] = "attribute-v4-uv100-color16-border-.0005-.001-.002-.004-.70-.50-.35-.20-min128";
        if (preserveAttributes)
            hash.Add(attributeAlgorithm, sizeof(attributeAlgorithm));
        hash.Add(counts, sizeof(counts));
        hash.Add(vertices.data(), vertices.size_bytes());
        hash.Add(indices.data(), indices.size_bytes());
        return hash.Finish(digest);
    }

    // LOCALAPPDATA 아래 입력 해시별 파생 캐시 경로를 만든다. 환경 경로를 얻지 못하면 빈 경로로 캐시 사용을 생략한다.
    std::filesystem::path LodCachePath(const std::array<uint8_t, 32>& digest, bool_t preserveAttributes)
    {
        wchar_t local[MAX_PATH]{};
        const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH);
        if (!length || length >= MAX_PATH)
            return {};
        constexpr char hex[] = "0123456789abcdef";
        std::string name;
        name.reserve(68u);
        for (uint8_t byte : digest)
        {
            name += hex[byte >> 4u];
            name += hex[byte & 15u];
        }
        return std::filesystem::path(local) / L"LostArk" / L"StaticMeshLod" / (preserveAttributes ? L"v4" : L"v3") / (name + ".bin");
    }

    // header 사본의 output 필드를 0으로 만든 뒤 header와 결합 인덱스를 함께 검증한다. 입력 header를 바꾸지 않는다.
    bool LodOutputHash(LOD_CACHE_HEADER header, std::span<const uint32_t> indices,
        std::array<uint8_t, 32>& digest)
    {
        header.output = {};
        LodHash hash;
        hash.Add(&header, sizeof(header));
        hash.Add(indices.data(), indices.size_bytes());
        return hash.Finish(digest);
    }

    // 선택적 캐시를 임시 저장소로 읽는다. 파일 크기·범위 연속성·원본 prefix·인덱스 한계·해시를 모두 통과해야 출력에 반영한다.
    bool LoadLodCache(const std::filesystem::path& path, const std::array<uint8_t, 32>& input,
        size_t vertexCount, std::span<const uint32_t> source, LOD_CACHE_HEADER& header,
        std::vector<uint32_t>& combined, bool_t preserveAttributes)
    {
        if (path.empty())
            return false;
        try
        {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file)
                return false;
            const auto bytes = file.tellg();
            file.seekg(0);
            LOD_CACHE_HEADER staged{};
            if (!file.read(reinterpret_cast<char*>(&staged), sizeof(staged)) || staged.magic != (preserveAttributes ? 0x34444f4cu : 0x33444f4cu) ||
                staged.version != (preserveAttributes ? 4u : 3u) || staged.input != input || staged.vertices != vertexCount ||
                staged.sourceIndices != source.size() || staged.rangeCount < 1u || staged.rangeCount > CStaticMeshLod::MAXIMUM_RANGES ||
                staged.combinedCount > source.size() * CStaticMeshLod::MAXIMUM_RANGES ||
                uint64_t(bytes) != sizeof(staged) + uint64_t(staged.combinedCount) * sizeof(uint32_t))
                return false;
            if (staged.ranges[0].count != source.size() || staged.ranges[0].first != 0u ||
                staged.ranges[0].error != 0.f || (staged.rangeCount == 1u && staged.combinedCount != 0u))
                return false;
            // 각 축소 범위가 바로 앞 범위 뒤에서 시작하며 충분히 줄어들었는지 확인한다.
            size_t end = staged.rangeCount == 1u ? 0u : source.size();
            for (uint32_t range = 1u; range < staged.rangeCount; ++range)
            {
                const auto& current = staged.ranges[range];
                if (!current.count || current.count % 3u || current.first != end ||
                    current.count >= staged.ranges[range - 1u].count * .85 ||
                    !std::isfinite(current.error) || current.error < 0.f)
                    return false;
                end += current.count;
            }
            if (end != staged.combinedCount)
                return false;
            // 결합 버퍼가 원본 인덱스로 시작하고 모든 참조가 현재 정점 범위 안에 있는지 검증한다.
            std::vector<uint32_t> payload(staged.combinedCount);
            if (!file.read(reinterpret_cast<char*>(payload.data()), payload.size() * sizeof(uint32_t)))
                return false;
            if (std::any_of(payload.begin(), payload.end(), [&](uint32_t index) { return index >= vertexCount; }) ||
                (!payload.empty() && !std::equal(source.begin(), source.end(), payload.begin())))
                return false;
            std::array<uint8_t, 32> output{};
            if (!LodOutputHash(staged, payload, output) || output != staged.output)
                return false;
            header = staged;
            combined = std::move(payload);
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    // 완성한 파생 데이터를 프로세스/일련번호 임시 파일에 쓴 뒤 같은 경로를 교체한다.
    // 캐시 저장 실패는 호출자의 GPU LOD 생성 성공 여부를 바꾸지 않는다.
    void SaveLodCache(const std::filesystem::path& path, LOD_CACHE_HEADER header,
        std::span<const uint32_t> combined)
    {
        if (path.empty())
            return;
        try
        {
            if (!LodOutputHash(header, combined, header.output))
                return;
            std::error_code error;
            std::filesystem::create_directories(path.parent_path(), error);
            if (error)
                return;
            static volatile LONG serial = 0;
            auto temporary = path;
            temporary += L"." + std::to_wstring(GetCurrentProcessId()) + L"." +
                std::to_wstring(InterlockedIncrement(&serial)) + L".tmp";
            {
                std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
                if (!file)
                    return;
                file.write(reinterpret_cast<const char*>(&header), sizeof(header));
                file.write(reinterpret_cast<const char*>(combined.data()), combined.size_bytes());
                file.flush();
                if (!file)
                {
                    file.close();
                    std::filesystem::remove(temporary, error);
                    return;
                }
            }
            if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING))
                std::filesystem::remove(temporary, error);
        }
        catch (const std::exception&)
        {
            /* 선택적 캐시 저장이 실패해도 생성한 LOD는 계속 사용할 수 있다. */
        }
    }
}

// CMesh가 원본 geometry를 준비할 때 호출한다. result를 비우고 입력 검사→캐시/단순화→GPU 버퍼 생성 후 새 소유 객체를 반환한다.
HRESULT CStaticMeshLod::Create(ID3D11Device* device, std::span<const VTXMESH> vertices,
    std::span<const uint32_t> indices, std::shared_ptr<CStaticMeshLod>& result, bool_t preserveAttributes)
{
    result.reset();
    // 에셋 로드 때 생성할 입력 크기에 상한을 둔다. 렌더링은 완성된 불변 범위를
    // 재사용하므로 draw마다 geometry를 다시 단순화하거나 GPU에서 읽어오지 않는다.
    const size_t minimumIndices = preserveAttributes ? ATTRIBUTE_MINIMUM_INDICES : MINIMUM_INDICES;
    constexpr size_t maximumIndices = 3u * 1024u * 1024u;
    if (!device || vertices.empty() || indices.size() % 3u != 0u)
        return E_INVALIDARG;
    if (indices.size() < minimumIndices || indices.size() > maximumIndices ||
        vertices.size() > 1024u * 1024u || device->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
        return S_FALSE;
    try
    {
        // 캐시 적중 여부와 관계없이 원본 채널의 유한값과 index 범위를 먼저 검사한다.
        for (const auto& v : vertices)
        {
            const float channels[] = { v.vPosition.x, v.vPosition.y, v.vPosition.z,
                v.vNormal.x, v.vNormal.y, v.vNormal.z, v.vTangent.x, v.vTangent.y, v.vTangent.z,
                v.vBinormal.x, v.vBinormal.y, v.vBinormal.z, v.vTexcoord.x, v.vTexcoord.y,
                v.vTexcoord1.x, v.vTexcoord1.y, v.vTexcoord2.x, v.vTexcoord2.y };
            if (std::any_of(std::begin(channels), std::end(channels), [](float f) { return !std::isfinite(f); }))
                return E_INVALIDARG;
        }
        if (std::any_of(indices.begin(), indices.end(), [&](uint32_t i) { return i >= vertices.size(); }))
            return E_INVALIDARG;
        std::array<uint8_t, 32> input{};
        std::filesystem::path cachePath;
        if (LodInputHash(vertices, indices, input, preserveAttributes))
        {
            try
            {
                cachePath = LodCachePath(input, preserveAttributes);
            }
            catch (const std::exception&)
            {
                /* 캐시 경로를 얻지 못해도 생성은 계속한다. */
            }
        }
        LOD_CACHE_HEADER cached{};
        if (preserveAttributes)
        {
            cached.magic = 0x34444f4cu;
            cached.version = 4u;
        }
        std::vector<uint32_t> combined;
        // 축소가 유효하지 않았던 원본도 rangeCount 1로 캐시할 수 있다. 이 경우 S_FALSE로 기본 geometry를 계속 사용한다.
        const bool cacheHit = LoadLodCache(cachePath, input, vertices.size(), indices, cached, combined, preserveAttributes);
        if (cacheHit && cached.rangeCount == 1u)
            return S_FALSE;
        auto staged = std::make_shared<CStaticMeshLod>();
        if (cacheHit)
        {
            staged->m_RangeCount = cached.rangeCount;
            for (uint32_t i = 0u; i < cached.rangeCount; ++i)
                staged->m_Ranges[i] = { cached.ranges[i].count, cached.ranges[i].first, cached.ranges[i].error };
        }
        else
        {
            // 원본 shading 채널을 가중 오차에 포함한다.
            // simplifier는 원본 정점과 mesh 경계를 유지하며 인덱스만 줄인다.
            constexpr size_t attributeCount = 19u;
            float weights[attributeCount] = {
                1.f, 1.f, 1.f, .5f, .5f, .5f, .5f, .5f, .5f,
                100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 1.f, 1.f, 1.f, 1.f };
            if (preserveAttributes)
                for (size_t channel = 15u; channel < 19u; ++channel)
                    weights[channel] = 16.f;
            std::vector<std::array<float, attributeCount>> attributes(vertices.size());
            for (size_t i = 0; i < vertices.size(); ++i)
            {
                const auto& v = vertices[i];
                auto& a = attributes[i];
                a = { v.vNormal.x, v.vNormal.y, v.vNormal.z,
                    v.vTangent.x, v.vTangent.y, v.vTangent.z,
                    v.vBinormal.x, v.vBinormal.y, v.vBinormal.z,
                    v.vTexcoord.x, v.vTexcoord.y, v.vTexcoord1.x, v.vTexcoord1.y,
                    v.vTexcoord2.x, v.vTexcoord2.y,
                    float(v.color0Rgba8 & 255u) / 255.f,
                    float((v.color0Rgba8 >> 8u) & 255u) / 255.f,
                    float((v.color0Rgba8 >> 16u) & 255u) / 255.f,
                    float((v.color0Rgba8 >> 24u) & 255u) / 255.f };
            }
            const float scale = meshopt_simplifyScale(&vertices[0].vPosition.x, vertices.size(), sizeof(VTXMESH));
            if (!std::isfinite(scale) || scale <= 0.f)
                return S_FALSE;
            staged->m_Ranges[0] = { static_cast<uint32_t>(indices.size()), 0u, 0.f };
            staged->m_RangeCount = 1u;
            combined.assign(indices.begin(), indices.end());
            std::vector<uint32_t> reduced(indices.size());
            // 기존 UV100의 미세 두 단계 생성 결과를 먼저 보관한다.
            // 원경 단계만 UV 가중치10과 더 큰 생성 오차를 허용한다. 실제 사용은 아래
            // Select_Range의 화면 오차 검사로 제한하며 vertex/UV/normal 자체는 바꾸지 않는다.
            // 단계별 후보의 목표 삼각형 수와 허용 오차를 다르게 만든다. 실제 draw 사용 가능 여부는 Select_Range에서 별도로 판정한다.
            constexpr float errors[] = { .001f, .002f, .01f, .02f };
            for (uint32_t step = 0u; step < MAXIMUM_RANGES - 1u; ++step)
            {
                if (step == 2u && !preserveAttributes)
                    for (size_t channel = 9u; channel < 15u; ++channel)
                        weights[channel] = 10.f;
                constexpr float attributeFractions[] = { .70f, .50f, .35f, .20f };
                constexpr float attributeErrors[] = { .0005f, .001f, .002f, .004f };
                const float fraction = preserveAttributes ? attributeFractions[step] : (step & 1u) == 0u ? .55f : .30f;
                const size_t target = (static_cast<size_t>(indices.size() * fraction) / 3u) * 3u;
                float error = 0.f;
                const size_t count = meshopt_simplifyWithAttributes(reduced.data(), indices.data(), indices.size(),
                    &vertices[0].vPosition.x, vertices.size(), sizeof(VTXMESH),
                    attributes[0].data(), sizeof(attributes[0]), weights, attributeCount, nullptr,
                    target, preserveAttributes ? attributeErrors[step] : errors[step], meshopt_SimplifyLockBorder, &error);
                if (count == 0u || count % 3u != 0u || !std::isfinite(error) || error < 0.f ||
                    count >= staged->m_Ranges[staged->m_RangeCount - 1u].count * .85)
                    continue;
                if (std::any_of(reduced.begin(), reduced.begin() + count,
                    [&](uint32_t i) { return i >= vertices.size(); }))
                    return E_UNEXPECTED;
                staged->m_Ranges[staged->m_RangeCount++] = {
                    static_cast<uint32_t>(count), static_cast<uint32_t>(combined.size()), error * scale };
                combined.insert(combined.end(), reduced.begin(), reduced.begin() + count);
            }
            cached.vertices = static_cast<uint32_t>(vertices.size());
            cached.sourceIndices = static_cast<uint32_t>(indices.size());
            cached.rangeCount = staged->m_RangeCount;
            cached.input = input;
            for (uint32_t i = 0u; i < staged->m_RangeCount; ++i)
                cached.ranges[i] = { staged->m_Ranges[i].count, staged->m_Ranges[i].first, staged->m_Ranges[i].error };
            if (staged->m_RangeCount == 1u)
                combined.clear();
            cached.combinedCount = static_cast<uint32_t>(combined.size());
            SaveLodCache(cachePath, cached, combined);
            if (staged->m_RangeCount == 1u)
                return S_FALSE;
        }
        // 원본과 축소 범위를 하나의 불변 index buffer로 만들고 성공한 객체만 출력에 연결한다.
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = static_cast<UINT>(combined.size() * sizeof(uint32_t));
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        D3D11_SUBRESOURCE_DATA initial{};
        initial.pSysMem = combined.data();
        const HRESULT hr = device->CreateBuffer(&desc, &initial, &staged->m_IndexBuffer);
        if (FAILED(hr))
            return hr;
        result = std::move(staged);
        return S_OK;
    }
    catch (const std::bad_alloc&)
    {
        return E_OUTOFMEMORY;
    }
    catch (const std::length_error&)
    {
        return E_OUTOFMEMORY;
    }
}

// 렌더링 입력으로 보수적인 픽셀 오차를 계산한다. 실패는 result를 보존하고, 정상 입력은 기본 또는 축소 범위 하나를 반환한다.
HRESULT CStaticMeshLod::Select_Range(const MESH_SCREEN_LOD_DESC& view,
    SELECTION& result) const
{
    const float values[] = { view.viewBounds.x, view.viewBounds.y, view.viewBounds.z,
        view.viewBounds.w, view.projectionPixels.x, view.projectionPixels.y,
        view.maximumScale, view.nearPlane, view.maximumPixelError, view.additionalWorldError };
    if (!m_IndexBuffer || m_RangeCount < 2u ||
        std::any_of(std::begin(values), std::end(values), [](float f) { return !std::isfinite(f); }) ||
        view.viewBounds.w <= 0.f || view.projectionPixels.x <= 0.f || view.projectionPixels.y <= 0.f ||
        view.maximumScale <= 0.f || view.nearPlane <= 0.f ||
        view.maximumPixelError <= 0.f || view.maximumPixelError > 1.f || view.additionalWorldError < 0.f)
        return E_INVALIDARG;

    if (view.hasTightViewBounds &&
        (!std::isfinite(view.maximumAbsViewXY.x) || !std::isfinite(view.maximumAbsViewXY.y) ||
         !std::isfinite(view.minimumViewDepth) || view.maximumAbsViewXY.x < 0.f || view.maximumAbsViewXY.y < 0.f))
        return E_INVALIDARG;

    uint32_t selected = 0u;
    // 타이트한 입력이 있으면 개별 구의 최소 깊이를 쓰고, 없으면 전체 구의 가장 가까운 깊이를 쓴다.
    const double nearestZ = view.hasTightViewBounds ? view.minimumViewDepth :
        double(view.viewBounds.z) - view.viewBounds.w;
    if (nearestZ > view.nearPlane)
    {
        const double largestX = view.hasTightViewBounds ? view.maximumAbsViewXY.x :
            std::abs(double(view.viewBounds.x)) + view.viewBounds.w;
        const double largestY = view.hasTightViewBounds ? view.maximumAbsViewXY.y :
            std::abs(double(view.viewBounds.y)) + view.viewBounds.w;
        for (uint32_t level = 1u; level < m_RangeCount; ++level)
        {
            // 로컬 geometry 오차×최대 배율에 호출자가 입증한 추가 월드 오차를 합친다.
            // 오차만큼 가까워져도 근평면을 넘지 않아야 투영 오차를 비교할 수 있다.
            const double error = double(m_Ranges[level].error) * view.maximumScale + view.additionalWorldError;
            const double safeZ = nearestZ - error;
            if (safeZ <= view.nearPlane)
                continue;
            // 화면 중심에서 벗어난 위치까지 포함한 보수적 상한과 10% 여유를 사용한다.
            // 선택에 필요한 입력은 이미 렌더 스레드에 있으며 결과를 GPU에서 읽어오지 않는다.
            const double pixelX = view.projectionPixels.x * error * (1. + largestX / nearestZ) / safeZ;
            const double pixelY = view.projectionPixels.y * error * (1. + largestY / nearestZ) / safeZ;
            if (std::isfinite(pixelX) && std::isfinite(pixelY) &&
                (std::max)(pixelX, pixelY) <= double(view.maximumPixelError) * .9)
                selected = level;
        }
    }
    result = { m_Ranges[selected].count, m_Ranges[selected].first, selected };
    return S_OK;
}

// GPU bin 등에서 쓸 불변 범위와 로컬 오차를 조회한다. 잘못된 level/버퍼 부재는 두 출력을 그대로 둔다.
bool_t CStaticMeshLod::Get_Range(uint32_t level, SELECTION& result, f32_t& localError) const
{
    if (!m_IndexBuffer || level >= m_RangeCount)
        return false;
    const auto& range = m_Ranges[level];
    result = { range.count, range.first, level };
    localError = range.error;
    return true;
}

NS_END
