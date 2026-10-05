#include "../ThirdPartyLib/meshoptimizer/meshoptimizer.h"
#include "Engine_VertexTypes.h"
#include "StaticMeshLod.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#pragma push_macro("new")
#undef new
#include <bcrypt.h>
#include <filesystem>
#include <fstream>
#pragma pop_macro("new")
#pragma comment(lib, "bcrypt.lib")

NS_BEGIN(Engine)

namespace
{
    // This is disposable derived data, never the authority for model loading.
    // Bump both the version and algorithm tag when the simplification changes.
    struct LOD_CACHE_RANGE final { uint32_t count = 0u, first = 0u; float error = 0.f; };
    struct LOD_CACHE_HEADER final
    {
        uint32_t magic = 0x32444f4cu, version = 2u;
        uint32_t vertices = 0u, sourceIndices = 0u, rangeCount = 1u, combinedCount = 0u;
        std::array<LOD_CACHE_RANGE, 3> ranges{};
        std::array<uint8_t, 32> input{}, output{};
    };
    static_assert(sizeof(LOD_CACHE_HEADER) == 124u);

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
            if (m_Hash) BCryptDestroyHash(m_Hash);
            if (m_Algorithm) BCryptCloseAlgorithmProvider(m_Algorithm, 0);
        }
        void Add(const void* data, size_t bytes)
        {
            if (!m_Valid || bytes == 0u) return;
            if (!data || bytes > ULONG_MAX) { m_Valid = false; return; }
            m_Valid = BCryptHashData(m_Hash, static_cast<PUCHAR>(const_cast<void*>(data)),
                static_cast<ULONG>(bytes), 0) >= 0;
        }
        bool Finish(std::array<uint8_t, 32>& digest)
        {
            return m_Valid && BCryptFinishHash(m_Hash, digest.data(), static_cast<ULONG>(digest.size()), 0) >= 0;
        }
    private:
        BCRYPT_ALG_HANDLE m_Algorithm = nullptr;
        BCRYPT_HASH_HANDLE m_Hash = nullptr;
        bool m_Valid = false;
    };

    bool LodInputHash(std::span<const VTXMESH> vertices, std::span<const uint32_t> indices,
        std::array<uint8_t, 32>& digest)
    {
        // VTXMESH contains exactly eighteen float channels and one color uint;
        // these layout assertions exclude padding from the hashed input bytes.
        static_assert(sizeof(VTXMESH) == 18u * sizeof(float) + sizeof(uint32_t));
        static_assert(offsetof(VTXMESH, color0Rgba8) == 64u && offsetof(VTXMESH, vTexcoord2) == 68u);
        constexpr char algorithm[] = "StaticLod-v2-meshopt73583c335e541c139821d0de2bf5f12960a04941-attrs19-uv100-border-.001-.002-.55-.30-.85";
        const uint64_t counts[] = { vertices.size(), indices.size() };
        LodHash hash;
        hash.Add(algorithm, sizeof(algorithm));
        hash.Add(counts, sizeof(counts));
        hash.Add(vertices.data(), vertices.size_bytes());
        hash.Add(indices.data(), indices.size_bytes());
        return hash.Finish(digest);
    }

    std::filesystem::path LodCachePath(const std::array<uint8_t, 32>& digest)
    {
        wchar_t local[MAX_PATH]{};
        const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH);
        if (!length || length >= MAX_PATH) return {};
        constexpr char hex[] = "0123456789abcdef";
        std::string name;
        name.reserve(68u);
        for (uint8_t byte : digest) { name += hex[byte >> 4u]; name += hex[byte & 15u]; }
        return std::filesystem::path(local) / L"LostArk" / L"StaticMeshLod" / L"v2" / (name + ".bin");
    }

    bool LodOutputHash(LOD_CACHE_HEADER header, std::span<const uint32_t> indices,
        std::array<uint8_t, 32>& digest)
    {
        header.output = {};
        LodHash hash;
        hash.Add(&header, sizeof(header));
        hash.Add(indices.data(), indices.size_bytes());
        return hash.Finish(digest);
    }

    bool LoadLodCache(const std::filesystem::path& path, const std::array<uint8_t, 32>& input,
        size_t vertexCount, std::span<const uint32_t> source, LOD_CACHE_HEADER& header,
        std::vector<uint32_t>& combined)
    {
        if (path.empty()) return false;
        try
        {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file) return false;
            const auto bytes = file.tellg();
            file.seekg(0);
            LOD_CACHE_HEADER staged{};
            if (!file.read(reinterpret_cast<char*>(&staged), sizeof(staged)) || staged.magic != 0x32444f4cu ||
                staged.version != 2u || staged.input != input || staged.vertices != vertexCount ||
                staged.sourceIndices != source.size() || staged.rangeCount < 1u || staged.rangeCount > 3u ||
                staged.combinedCount > source.size() * 3u ||
                uint64_t(bytes) != sizeof(staged) + uint64_t(staged.combinedCount) * sizeof(uint32_t)) return false;
            if (staged.ranges[0].count != source.size() || staged.ranges[0].first != 0u ||
                staged.ranges[0].error != 0.f || (staged.rangeCount == 1u && staged.combinedCount != 0u)) return false;
            size_t end = staged.rangeCount == 1u ? 0u : source.size();
            for (uint32_t range = 1u; range < staged.rangeCount; ++range)
            {
                const auto& current = staged.ranges[range];
                if (!current.count || current.count % 3u || current.first != end ||
                    current.count >= staged.ranges[range - 1u].count * .85 ||
                    !std::isfinite(current.error) || current.error < 0.f) return false;
                end += current.count;
            }
            if (end != staged.combinedCount) return false;
            std::vector<uint32_t> payload(staged.combinedCount);
            if (!file.read(reinterpret_cast<char*>(payload.data()), payload.size() * sizeof(uint32_t))) return false;
            if (std::any_of(payload.begin(), payload.end(), [&](uint32_t index) { return index >= vertexCount; }) ||
                (!payload.empty() && !std::equal(source.begin(), source.end(), payload.begin()))) return false;
            std::array<uint8_t, 32> output{};
            if (!LodOutputHash(staged, payload, output) || output != staged.output) return false;
            header = staged;
            combined = std::move(payload);
            return true;
        }
        catch (const std::exception&) { return false; }
    }

    void SaveLodCache(const std::filesystem::path& path, LOD_CACHE_HEADER header,
        std::span<const uint32_t> combined)
    {
        if (path.empty()) return;
        try
        {
            if (!LodOutputHash(header, combined, header.output)) return;
            std::error_code error;
            std::filesystem::create_directories(path.parent_path(), error);
            if (error) return;
            static volatile LONG serial = 0;
            auto temporary = path;
            temporary += L"." + std::to_wstring(GetCurrentProcessId()) + L"." +
                std::to_wstring(InterlockedIncrement(&serial)) + L".tmp";
            {
                std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
                if (!file) return;
                file.write(reinterpret_cast<const char*>(&header), sizeof(header));
                file.write(reinterpret_cast<const char*>(combined.data()), combined.size_bytes());
                file.flush();
                if (!file) { file.close(); std::filesystem::remove(temporary, error); return; }
            }
            if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING))
                std::filesystem::remove(temporary, error);
        }
        catch (const std::exception&) { /* Optional cache failure keeps the generated LOD usable. */ }
    }
}

HRESULT CStaticMeshLod::Create(ID3D11Device* device, std::span<const VTXMESH> vertices,
    std::span<const uint32_t> indices, std::shared_ptr<CStaticMeshLod>& result)
{
    result.reset();
    // Keep generation bounded at asset load time. Selection and drawing reuse
    // these immutable ranges without per-draw compute work or GPU readback.
    constexpr size_t minimumIndices = MINIMUM_INDICES;
    constexpr size_t maximumIndices = 3u * 1024u * 1024u;
    if (!device || vertices.empty() || indices.size() % 3u != 0u) return E_INVALIDARG;
    if (indices.size() < minimumIndices || indices.size() > maximumIndices ||
        vertices.size() > 1024u * 1024u || device->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
        return S_FALSE;
    try
    {
        // Validate even on cache hits: invalid source data never becomes a LOD.
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
        if (LodInputHash(vertices, indices, input))
        {
            try { cachePath = LodCachePath(input); }
            catch (const std::exception&) { /* Cache discovery is optional. */ }
        }
        LOD_CACHE_HEADER cached{};
        std::vector<uint32_t> combined;
        const bool cacheHit = LoadLodCache(cachePath, input, vertices.size(), indices, cached, combined);
        if (cacheHit && cached.rangeCount == 1u) return S_FALSE;
        auto staged = std::make_shared<CStaticMeshLod>();
        if (cacheHit)
        {
            staged->m_RangeCount = cached.rangeCount;
            for (uint32_t i = 0u; i < cached.rangeCount; ++i)
                staged->m_Ranges[i] = { cached.ranges[i].count, cached.ranges[i].first, cached.ranges[i].error };
        }
        else
        {
            // All source shading channels participate in the error metric. The
            // simplifier keeps the original vertices and locks mesh boundaries.
            constexpr size_t attributeCount = 19u;
            const float weights[attributeCount] = {
                1.f, 1.f, 1.f, .5f, .5f, .5f, .5f, .5f, .5f,
                100.f, 100.f, 100.f, 100.f, 100.f, 100.f, 1.f, 1.f, 1.f, 1.f };
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
            if (!std::isfinite(scale) || scale <= 0.f) return S_FALSE;
            staged->m_Ranges[0] = { static_cast<uint32_t>(indices.size()), 0u, 0.f };
            staged->m_RangeCount = 1u;
            combined.assign(indices.begin(), indices.end());
            std::vector<uint32_t> reduced(indices.size());
            for (uint32_t level = 1u; level < 3u; ++level)
            {
                const float fraction = level == 1u ? .55f : .30f;
                const size_t target = (static_cast<size_t>(indices.size() * fraction) / 3u) * 3u;
                float error = 0.f;
                const size_t count = meshopt_simplifyWithAttributes(reduced.data(), indices.data(), indices.size(),
                    &vertices[0].vPosition.x, vertices.size(), sizeof(VTXMESH),
                    attributes[0].data(), sizeof(attributes[0]), weights, attributeCount, nullptr,
                    target, level == 1u ? .001f : .002f, meshopt_SimplifyLockBorder, &error);
                if (count == 0u || count % 3u != 0u || !std::isfinite(error) || error < 0.f ||
                    count >= staged->m_Ranges[staged->m_RangeCount - 1u].count * .85)
                    continue;
                if (std::any_of(reduced.begin(), reduced.begin() + count,
                    [&](uint32_t i) { return i >= vertices.size(); })) return E_UNEXPECTED;
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
            if (staged->m_RangeCount == 1u) combined.clear();
            cached.combinedCount = static_cast<uint32_t>(combined.size());
            SaveLodCache(cachePath, cached, combined);
            if (staged->m_RangeCount == 1u) return S_FALSE;
        }
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = static_cast<UINT>(combined.size() * sizeof(uint32_t));
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        D3D11_SUBRESOURCE_DATA initial{};
        initial.pSysMem = combined.data();
        const HRESULT hr = device->CreateBuffer(&desc, &initial, &staged->m_IndexBuffer);
        if (FAILED(hr)) return hr;
        result = std::move(staged);
        return S_OK;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (const std::length_error&) { return E_OUTOFMEMORY; }
}

HRESULT CStaticMeshLod::Select_Range(const MESH_SCREEN_LOD_DESC& view,
    SELECTION& result) const
{
    const float values[] = { view.viewBounds.x, view.viewBounds.y, view.viewBounds.z,
        view.viewBounds.w, view.projectionPixels.x, view.projectionPixels.y,
        view.maximumScale, view.nearPlane, view.maximumPixelError };
    if (!m_IndexBuffer || m_RangeCount < 2u ||
        std::any_of(std::begin(values), std::end(values), [](float f) { return !std::isfinite(f); }) ||
        view.viewBounds.w <= 0.f || view.projectionPixels.x <= 0.f || view.projectionPixels.y <= 0.f ||
        view.maximumScale <= 0.f || view.nearPlane <= 0.f ||
        view.maximumPixelError <= 0.f || view.maximumPixelError > 1.f)
        return E_INVALIDARG;

    if (view.hasTightViewBounds &&
        (!std::isfinite(view.maximumAbsViewXY.x) || !std::isfinite(view.maximumAbsViewXY.y) ||
         !std::isfinite(view.minimumViewDepth) || view.maximumAbsViewXY.x < 0.f || view.maximumAbsViewXY.y < 0.f))
        return E_INVALIDARG;

    uint32_t selected = 0u;
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
            const double error = double(m_Ranges[level].error) * view.maximumScale;
            const double safeZ = nearestZ - error;
            if (safeZ <= view.nearPlane) continue;
            // Same conservative off-axis bound and 10% margin as the former
            // compute selector. All inputs already live on the render thread.
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

NS_END
