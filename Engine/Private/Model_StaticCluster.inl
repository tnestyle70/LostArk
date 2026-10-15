// Model.cpp가 private 재질 source 정의 뒤에 포함하는 구현 조각이다.
#include "../ThirdPartyLib/meshoptimizer/meshoptimizer.h"
#include <bit>
#include <unordered_map>
#include <bcrypt.h>
#include <fstream>
#pragma comment(lib, "bcrypt.lib")

struct CModel::STATIC_CLUSTER_BUILD_CACHE final
{
    std::map<std::filesystem::path, std::shared_ptr<const MODEL_ASSET_DATA>> assets;
    // 로딩 단계의 파생 geometry 캐시는 64 MiB까지만 소유한다. 큰 단일
    // decode는 현재 source 처리 동안만 빌리고 다음 cluster까지 보관하지 않는다.
    uint64_t decodedBytes = 0u;
};

struct CModel::STATIC_CLUSTER_STORAGE final
{
    STATIC_CLUSTER_STATS stats;
    vector<uint64_t> sourceIds;
    vector<STATIC_CLUSTER_SOURCE_RANGE> ranges;
    vector<uint32_t> farIndices;
    vector<shared_ptr<CMaterial>> materials;
    ComPtr<ID3D11ShaderResourceView> sourceRecords;
};

// Immutable geometry는 Clone과 공유하지만 카메라별 선택 버퍼는 CModel이
// 따로 소유한다. render thread에서만 prepare/render하고 소멸 시 함께 해제한다.
struct CModel::STATIC_CLUSTER_SELECTION final
{
    vector<uint32_t> sources;
    ComPtr<ID3D11Buffer> indices;
    uint32_t indexCount = 0u;
    bool prepared = false;
};

shared_ptr<CModel::STATIC_CLUSTER_BUILD_CACHE> CModel::Create_StaticClusterBuildCache()
{
    return make_shared<STATIC_CLUSTER_BUILD_CACHE>();
}

uint32_t CModel::Get_MeshIndexCount(uint32_t meshIndex) const
{
    return meshIndex < m_Meshes.size() && m_Meshes[meshIndex] ?
        m_Meshes[meshIndex]->m_iNumIndices : 0u;
}

bool_t CModel::Can_ShareStaticClusterMaterialWith(uint32_t meshIndex,
    const CModel& other, uint32_t otherMeshIndex) const
{
    if (m_eType != MODEL::NONANIM || other.m_eType != MODEL::NONANIM ||
        m_StaticCluster || other.m_StaticCluster || m_pDevice != other.m_pDevice ||
        m_pContext != other.m_pContext || meshIndex >= m_Meshes.size() ||
        otherMeshIndex >= other.m_Meshes.size() || !m_Meshes[meshIndex] ||
        !other.m_Meshes[otherMeshIndex] || m_Meshes[meshIndex]->Has_MorphBaseVertices() ||
        other.m_Meshes[otherMeshIndex]->Has_MorphBaseVertices()) return false;
    const uint32_t a = m_Meshes[meshIndex]->Get_MaterialIndex();
    const uint32_t b = other.m_Meshes[otherMeshIndex]->Get_MaterialIndex();
    if (a >= m_Materials.size() || b >= other.m_Materials.size() ||
        !m_Materials[a] || !other.m_Materials[b]) return false;
    const auto& surface = m_Materials[a]->Get_Surface();
    const auto& otherSurface = other.m_Materials[b]->Get_Surface();
    // 불변 cluster 경계·LOD 오차에는 애니메이션 정점 이동이 포함되지 않는다.
    if (surface.sourceFoliageWind || otherSurface.sourceFoliageWind ||
        Has_MaterialTexture(meshIndex, aiTextureType_OPACITY) ||
        other.Has_MaterialTexture(otherMeshIndex, aiTextureType_OPACITY)) return false;
    return m_Materials[a]->Can_BatchStaticLightingWith(*other.m_Materials[b], true);
}

namespace
{

    uint64_t ClusterDecodedBytes(const MODEL_ASSET_DATA& asset)
    {
        uint64_t bytes = 0u;
        for (const auto& mesh : asset.meshes)
            bytes += uint64_t(mesh.vertices.capacity()) * sizeof(VTXMESH) +
                uint64_t(mesh.skinnedVertices.capacity()) * sizeof(VTXANIMMESH) +
                uint64_t(mesh.tangentHandedness.capacity()) * sizeof(f32_t) +
                uint64_t(mesh.indices.capacity() + mesh.color0Rgba8.capacity()) * sizeof(uint32_t);
        return bytes;
    }

    class ClusterHash final
    {
    public:
        ClusterHash()
        { valid = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) >= 0 &&
            BCryptCreateHash(algorithm, &hash, nullptr, 0, nullptr, 0, 0) >= 0; }
        ~ClusterHash() { if (hash) BCryptDestroyHash(hash); if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0); }
        void Add(const void* data, size_t bytes)
        {
            if (!valid || !bytes) return;
            if (!data || bytes > ULONG_MAX) { valid = false; return; }
            valid = BCryptHashData(hash, static_cast<PUCHAR>(const_cast<void*>(data)), static_cast<ULONG>(bytes), 0) >= 0;
        }
        template<class T> void Add(const vector<T>& data) { Add(data.data(), data.size() * sizeof(T)); }
        bool Finish(array<uint8_t, 32>& output)
        { return valid && BCryptFinishHash(hash, output.data(), static_cast<ULONG>(output.size()), 0) >= 0; }
    private:
        BCRYPT_ALG_HANDLE algorithm = nullptr;
        BCRYPT_HASH_HANDLE hash = nullptr;
        bool valid = false;
    };

    struct CLUSTER_CACHE_HEADER final
    {
        uint32_t magic = 0x32434c42u, version = 2u, sources = 0u, indices = 0u;
        array<uint8_t, 32> input{}, output{};
    };
    struct CLUSTER_CACHE_RANGE final { uint32_t count = 0u; float error = 0.f; };

    std::filesystem::path ClusterCachePath(const array<uint8_t, 32>& digest)
    {
        wchar_t local[MAX_PATH]{};
        const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH);
        if (!length || length >= MAX_PATH) return {};
        string name;
        for (uint8_t byte : digest) { name += "0123456789abcdef"[byte >> 4u]; name += "0123456789abcdef"[byte & 15u]; }
        return std::filesystem::path(local) / L"LostArk" / L"MapHlod" / L"Geometry-v2" / (name + ".bin");
    }

    // 파일은 geometry만 재사용한다. 현재 재질/RNM admission과 원본 hash 검증은
    // 호출자가 먼저 수행하고, count/error/출력 SHA가 하나라도 다르면 재생성한다.
    bool LoadClusterLayout(const std::filesystem::path& path, const array<uint8_t, 32>& input,
        size_t sourceCount, size_t maximumIndices, float maximumError,
        const vector<uint32_t>& vertexSources, vector<CLUSTER_CACHE_RANGE>& ranges, vector<uint32_t>& indices)
    {
        if (path.empty()) return false;
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) return false;
        const auto bytes = file.tellg(); file.seekg(0);
        CLUSTER_CACHE_HEADER header{};
        if (!file.read(reinterpret_cast<char*>(&header), sizeof(header)) || header.magic != 0x32434c42u ||
            header.version != 2u || header.input != input || header.sources != sourceCount ||
            header.indices > maximumIndices || uint64_t(bytes) != sizeof(header) +
            uint64_t(sourceCount) * sizeof(CLUSTER_CACHE_RANGE) + uint64_t(header.indices) * sizeof(uint32_t)) return false;
        vector<CLUSTER_CACHE_RANGE> stagedRanges(sourceCount);
        vector<uint32_t> stagedIndices(header.indices);
        if (!file.read(reinterpret_cast<char*>(stagedRanges.data()), stagedRanges.size() * sizeof(CLUSTER_CACHE_RANGE)) ||
            !file.read(reinterpret_cast<char*>(stagedIndices.data()), stagedIndices.size() * sizeof(uint32_t))) return false;
        ClusterHash hash; hash.Add(stagedRanges); hash.Add(stagedIndices);
        array<uint8_t, 32> digest{};
        if (!hash.Finish(digest) || digest != header.output) return false;
        size_t first = 0u;
        for (size_t source = 0u; source < sourceCount; ++source)
        {
            const auto& range = stagedRanges[source];
            if (range.count % 3u || range.count > stagedIndices.size() - first ||
                !std::isfinite(range.error) || range.error < 0.f || range.error > maximumError * 1.001f) return false;
            for (size_t i = first; i < first + range.count; ++i)
                if (stagedIndices[i] >= vertexSources.size() || vertexSources[stagedIndices[i]] != source) return false;
            first += range.count;
        }
        if (first != stagedIndices.size()) return false;
        ranges.swap(stagedRanges); indices.swap(stagedIndices); return true;
    }

    void SaveClusterLayout(const std::filesystem::path& path, const array<uint8_t, 32>& input,
        const vector<CLUSTER_CACHE_RANGE>& ranges, const vector<uint32_t>& indices)
    {
        if (path.empty()) return;
        CLUSTER_CACHE_HEADER header{};
        header.sources = static_cast<uint32_t>(ranges.size()); header.indices = static_cast<uint32_t>(indices.size()); header.input = input;
        ClusterHash hash; hash.Add(ranges); hash.Add(indices);
        if (!hash.Finish(header.output)) return;
        std::error_code error; std::filesystem::create_directories(path.parent_path(), error);
        if (error) return;
        static volatile LONG serial = 0;
        auto temporary = path;
        temporary += L"." + std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(InterlockedIncrement(&serial)) + L".tmp";
        {
            std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
            if (!file) return;
            file.write(reinterpret_cast<const char*>(&header), sizeof(header));
            file.write(reinterpret_cast<const char*>(ranges.data()), ranges.size() * sizeof(CLUSTER_CACHE_RANGE));
            file.write(reinterpret_cast<const char*>(indices.data()), indices.size() * sizeof(uint32_t)); file.flush();
            if (!file) { file.close(); std::filesystem::remove(temporary, error); return; }
        }
        if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            std::filesystem::remove(temporary, error);
    }

    bool ClusterFinite(const float* values, size_t count)
    {
        return std::all_of(values, values + count, [](float value) { return std::isfinite(value); });
    }

    bool ClusterAffine(const float4x4_t& value)
    {
        return ClusterFinite(&value._11, 16u) && value._14 == 0.f && value._24 == 0.f &&
            value._34 == 0.f && value._44 == 1.f;
    }

    struct CLUSTER_POSITION_KEY final
    {
        std::array<uint32_t, 3> bits;
        bool operator==(const CLUSTER_POSITION_KEY&) const = default;
    };
    struct CLUSTER_POSITION_HASH final
    {
        size_t operator()(const CLUSTER_POSITION_KEY& key) const noexcept
        { return (size_t(key.bits[0]) * 73856093u) ^ (size_t(key.bits[1]) * 19349663u) ^
            (size_t(key.bits[2]) * 83492791u); }
    };
    CLUSTER_POSITION_KEY ClusterPositionKey(const float3_t& position)
    {
        // Treat +/- zero as the same position, just as the simplifier does.
        return {{ std::bit_cast<uint32_t>(position.x == 0.f ? 0.f : position.x),
            std::bit_cast<uint32_t>(position.y == 0.f ? 0.f : position.y),
            std::bit_cast<uint32_t>(position.z == 0.f ? 0.f : position.z) }};
    }
}

HRESULT CModel::Create_StaticCluster(std::span<const STATIC_CLUSTER_SOURCE> sources,
    const STATIC_CLUSTER_OPTIONS& options, STATIC_CLUSTER_BUILD_CACHE& cache,
    shared_ptr<CModel>& output, STATIC_CLUSTER_STATS& stats)
{
    if (sources.size() < 2u || sources.size() > options.maximumSources ||
        options.maximumSources > 65536u || options.maximumVertices == 0u ||
        options.maximumVertices > 1048576u || options.maximumIndices == 0u ||
        options.maximumIndices > 4194304u || !std::isfinite(options.targetRatio) ||
        options.targetRatio <= 0.f || options.targetRatio >= 1.f ||
        !std::isfinite(options.maximumWorldError) || options.maximumWorldError <= 0.f ||
        !sources.front().model) return E_INVALIDARG;
    try
    {
        const auto& first = *sources.front().model;
        auto storage = make_shared<STATIC_CLUSTER_STORAGE>();
        auto& measured = storage->stats;
        measured.sourceCount = static_cast<uint32_t>(sources.size());
        const float maximum = (std::numeric_limits<float>::max)();
        measured.boundsMin = {maximum, maximum, maximum};
        measured.boundsMax = {-maximum, -maximum, -maximum};
        MODEL_MESH_DATA merged;
        merged.name = "static-spatial-cluster";
        merged.materialIndex = 0u;
        merged.vertexKind = MODEL_VERTEX_KIND::STATIC;
        merged.hasColor0 = true;
        vector<uint32_t> sourceIndices;
        vector<VTXMESHINSTANCE> records;
        vector<float3_t> worldPositions;
        vector<uint32_t> firstVertices, vertexCounts, firstIndices, indexCounts;
        bool mirrored = false;
        for (size_t sourceIndex = 0u; sourceIndex < sources.size(); ++sourceIndex)
        {
            const auto& source = sources[sourceIndex];
            if (!source.model || !first.Can_ShareStaticClusterMaterialWith(
                sources.front().meshIndex, *source.model, source.meshIndex)) return S_FALSE;
            const auto& model = *source.model;
            if (!ClusterAffine(model.m_PreTransformMatrix) || !ClusterAffine(source.instance.World) ||
                !ClusterFinite(reinterpret_cast<const float*>(&source.instance), sizeof(VTXMESHINSTANCE) / sizeof(float)))
                return E_INVALIDARG;
            const matrix_t world = XMLoadFloat4x4(&source.instance.World);
            const float determinant = XMVectorGetX(XMMatrixDeterminant(world));
            if (!std::isfinite(determinant) || std::abs(determinant) < 1.e-12f) return E_INVALIDARG;
            if (sourceIndex == 0u) mirrored = determinant < 0.f;
            else if (mirrored != (determinant < 0.f)) return S_FALSE;
            const auto material = model.m_Materials[model.m_Meshes[source.meshIndex]->Get_MaterialIndex()];
            size_t bank = 0u;
            while (bank < storage->materials.size() &&
                !material->Has_SameStaticLightingTextures(*storage->materials[bank])) ++bank;
            if (bank == storage->materials.size())
            {
                if (bank == 8u) return S_FALSE;
                storage->materials.push_back(material);
            }
            VTXMESHINSTANCE record = source.instance;
            record.vLightmapDirectionalScale.w = static_cast<float>(bank);
            records.push_back(record);
            storage->sourceIds.push_back(source.sourceId);

            shared_ptr<const MODEL_ASSET_DATA> decodedOwner;
            const vector<MODEL_MESH_DATA>* meshes = model.m_pOrderedStaticGeometrySource.get();
            if (!meshes)
            {
                if (!model.m_pMaterialSource || !model.m_bHasSelfConsistentUnauthenticatedGeometryMetadata)
                    return S_FALSE;
                const auto& identity = model.m_pMaterialSource->identity;
                const auto path = identity.meshPath.lexically_normal();
                const auto found = cache.assets.find(path);
                if (found != cache.assets.end()) decodedOwner = found->second;
                else
                {
                    auto decoded = make_shared<MODEL_ASSET_DATA>();
                    if (!CModelDecoderRegistry::Get().Decode(identity, *decoded)) return E_FAIL;
                    decodedOwner = decoded;
                    constexpr uint64_t maximumDecodedBytes = 64ull * 1024ull * 1024ull;
                    const uint64_t bytes = ClusterDecodedBytes(*decoded);
                    if (bytes <= maximumDecodedBytes)
                    {
                        if (cache.decodedBytes + bytes > maximumDecodedBytes)
                        { cache.assets.clear(); cache.decodedBytes = 0u; }
                        cache.assets.emplace(path, std::move(decoded)); cache.decodedBytes += bytes;
                    }
                }
                const auto& decoded = *decodedOwner;
                // prototype 로드 뒤 교체된 파일을 다른 버전의 cluster로 조용히 사용하지 않도록 검사한다.
                if (!decoded.geometryMetadata.present || decoded.hasSkeleton ||
                    decoded.geometryMetadata.payloadSha256 != model.m_GeometryPayloadSha256 ||
                    decoded.geometryMetadata.metadataIdentitySha256 != model.m_GeometryMetadataIdentitySha256 ||
                    decoded.meshes.size() != model.m_Meshes.size()) return E_FAIL;
                meshes = &decoded.meshes;
            }
            if (source.meshIndex >= meshes->size()) return E_INVALIDARG;
            const auto& mesh = (*meshes)[source.meshIndex];
            if (mesh.vertexKind != MODEL_VERTEX_KIND::STATIC || mesh.vertices.empty() ||
                mesh.indices.empty() || mesh.indices.size() % 3u != 0u ||
                (mesh.hasColor0 && mesh.color0Rgba8.size() != mesh.vertices.size()) ||
                merged.vertices.size() + mesh.vertices.size() > options.maximumVertices ||
                merged.indices.size() + mesh.indices.size() > options.maximumIndices) return E_INVALIDARG;
            const auto baseVertex = static_cast<uint32_t>(merged.vertices.size());
            firstVertices.push_back(baseVertex); vertexCounts.push_back(static_cast<uint32_t>(mesh.vertices.size()));
            firstIndices.push_back(static_cast<uint32_t>(merged.indices.size())); indexCounts.push_back(static_cast<uint32_t>(mesh.indices.size()));
            STATIC_CLUSTER_SOURCE_RANGE range{}; range.sourceId = source.sourceId;
            range.boundsMin = {maximum, maximum, maximum}; range.boundsMax = {-maximum, -maximum, -maximum};
            const matrix_t preTransform = XMLoadFloat4x4(&model.m_PreTransformMatrix);
            for (size_t index = 0u; index < mesh.vertices.size(); ++index)
            {
                VTXMESH vertex = mesh.vertices[index];
                vertex.color0Rgba8 = mesh.hasColor0 ? mesh.color0Rgba8[index] : 0xffffffffu;
                // 기존 CMesh 업로드와 같은 pretransform이다. world inverse-transpose·Gram-Schmidt·RNM basis는 VS_MAIN이 처리한다.
                XMStoreFloat3(&vertex.vPosition, XMVector3TransformCoord(XMLoadFloat3(&vertex.vPosition), preTransform));
                XMStoreFloat3(&vertex.vNormal, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vNormal), preTransform)));
                XMStoreFloat3(&vertex.vTangent, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vTangent), preTransform)));
                XMStoreFloat3(&vertex.vBinormal, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vBinormal), preTransform)));
                if (!ClusterFinite(&vertex.vPosition.x, 16u) || !ClusterFinite(&vertex.vTexcoord2.x, 2u))
                    return E_INVALIDARG;
                float3_t position;
                XMStoreFloat3(&position, XMVector3TransformCoord(XMLoadFloat3(&vertex.vPosition), world));
                if (!ClusterFinite(&position.x, 3u)) return E_INVALIDARG;
                measured.boundsMin.x = (std::min)(measured.boundsMin.x, position.x);
                measured.boundsMin.y = (std::min)(measured.boundsMin.y, position.y);
                measured.boundsMin.z = (std::min)(measured.boundsMin.z, position.z);
                measured.boundsMax.x = (std::max)(measured.boundsMax.x, position.x);
                measured.boundsMax.y = (std::max)(measured.boundsMax.y, position.y);
                measured.boundsMax.z = (std::max)(measured.boundsMax.z, position.z);
                XMStoreFloat3(&range.boundsMin, XMVectorMin(XMLoadFloat3(&range.boundsMin), XMLoadFloat3(&position)));
                XMStoreFloat3(&range.boundsMax, XMVectorMax(XMLoadFloat3(&range.boundsMax), XMLoadFloat3(&position)));
                merged.vertices.push_back(vertex);
                merged.color0Rgba8.push_back(vertex.color0Rgba8);
                worldPositions.push_back(position);
                sourceIndices.push_back(static_cast<uint32_t>(sourceIndex));
            }
            for (uint32_t index : mesh.indices)
            {
                if (index >= mesh.vertices.size()) return E_INVALIDARG;
                merged.indices.push_back(baseVertex + index);
            }
            storage->ranges.push_back(range);
            merged.hasTexcoord1 |= mesh.hasTexcoord1;
            merged.hasTexcoord2 |= mesh.hasTexcoord2;
        }
        measured.materialCount = static_cast<uint32_t>(storage->materials.size());
        measured.clusterVertices = static_cast<uint32_t>(merged.vertices.size());
        measured.sourceIndices = measured.nearIndices = static_cast<uint32_t>(merged.indices.size());

        // 여러 mesh를 변환한 공간 cluster 전체를 단순화한다. 남은 index는 원래 UV·색·source별 변환·조명 record를 그대로 참조한다.
        constexpr size_t attributeCount = 20u;
        const float weights[attributeCount] = {1.f,1.f,1.f,.5f,.5f,.5f,.5f,.5f,.5f,
            100.f,100.f,100.f,100.f,100.f,100.f,1.f,1.f,1.f,1.f,1000000.f};
        vector<std::array<float, attributeCount>> attributes(merged.vertices.size());
        vector<unsigned char> locks(merged.vertices.size(), 0u);
        std::unordered_map<CLUSTER_POSITION_KEY, uint32_t, CLUSTER_POSITION_HASH> positions;
        positions.reserve(merged.vertices.size());
        for (size_t i = 0u; i < merged.vertices.size(); ++i)
        {
            const auto& v = merged.vertices[i];
            attributes[i] = {v.vNormal.x,v.vNormal.y,v.vNormal.z,v.vTangent.x,v.vTangent.y,v.vTangent.z,
                v.vBinormal.x,v.vBinormal.y,v.vBinormal.z,v.vTexcoord.x,v.vTexcoord.y,
                v.vTexcoord1.x,v.vTexcoord1.y,v.vTexcoord2.x,v.vTexcoord2.y,
                float(v.color0Rgba8 & 255u)/255.f,float((v.color0Rgba8>>8u)&255u)/255.f,
                float((v.color0Rgba8>>16u)&255u)/255.f,float((v.color0Rgba8>>24u)&255u)/255.f,
                static_cast<float>(sourceIndices[i])};
            const auto [found, inserted] = positions.emplace(ClusterPositionKey(worldPositions[i]), static_cast<uint32_t>(i));
            if (!inserted && sourceIndices[found->second] != sourceIndices[i]) locks[found->second] = locks[i] = 1u;
        }
        // source 경계를 공유하는 중복 정점에는 같은 위치 잠금을 적용한다.
        for (size_t i = 0u; i < locks.size(); ++i)
            if (locks[positions.at(ClusterPositionKey(worldPositions[i]))]) locks[i] = 1u;

        // source 간 삼각형 연결은 금지한다. source마다 독립 오차를 측정하면
        // 근처의 큰 source 하나가 원경의 정확한 작은 source까지 막지 않는다.
        // border/UV/COLOR와 cross-source seam lock은 기존 품질 계약 그대로다.
        vector<uint32_t> farIndices;
        vector<CLUSTER_CACHE_RANGE> reducedRanges;
        ClusterHash inputHash;
        const uint32_t recipeVersion = 2u;
        inputHash.Add(&recipeVersion, sizeof(recipeVersion)); inputHash.Add(worldPositions);
        inputHash.Add(attributes); inputHash.Add(locks); inputHash.Add(merged.indices);
        inputHash.Add(firstVertices); inputHash.Add(vertexCounts); inputHash.Add(indexCounts);
        inputHash.Add(&options.targetRatio, sizeof(options.targetRatio));
        inputHash.Add(&options.maximumWorldError, sizeof(options.maximumWorldError));
        array<uint8_t, 32> inputDigest{};
        const bool canCache = inputHash.Finish(inputDigest);
        const auto cachePath = canCache ? ClusterCachePath(inputDigest) : std::filesystem::path{};
        measured.geometryCacheHit = LoadClusterLayout(cachePath, inputDigest, sources.size(), merged.indices.size(),
            options.maximumWorldError, sourceIndices, reducedRanges, farIndices);
        if (!measured.geometryCacheHit)
        {
            reducedRanges.resize(sources.size()); farIndices.reserve(merged.indices.size());
            for (size_t source = 0u; source < sources.size(); ++source)
            {
                const uint32_t base = firstVertices[source], count = indexCounts[source], vertices = vertexCounts[source];
                vector<uint32_t> local(count), reduced(count);
                for (size_t i = 0u; i < count; ++i) local[i] = merged.indices[firstIndices[source] + i] - base;
                float error = 0.f;
                const size_t target = (static_cast<size_t>(count * options.targetRatio) / 3u) * 3u;
                const size_t result = meshopt_simplifyWithAttributes(reduced.data(), local.data(), count,
                    &worldPositions[base].x, vertices, sizeof(float3_t), attributes[base].data(), sizeof(attributes[0]),
                    weights, attributeCount, locks.data() + base, (std::max)(size_t(3u), target),
                    options.maximumWorldError, meshopt_SimplifyLockBorder | meshopt_SimplifyErrorAbsolute, &error);
                const bool valid = result > 0u && result % 3u == 0u && result < count &&
                    std::isfinite(error) && error >= 0.f && error <= options.maximumWorldError * 1.001f &&
                    std::all_of(reduced.begin(), reduced.begin() + result, [&](uint32_t index) { return index < vertices; });
                // 축약 불가 source도 전체 far API에서는 원본을 유지한다.
                // 가시 source claim은 index strict 감소 검사로 이 범위를 거절한다.
                const size_t selected = valid ? result : count;
                reducedRanges[source] = {static_cast<uint32_t>(selected), valid ? error : 0.f};
                for (size_t i = 0u; i < selected; ++i) farIndices.push_back(base + (valid ? reduced[i] : local[i]));
            }
            SaveClusterLayout(cachePath, inputDigest, reducedRanges, farIndices);
        }
        uint32_t firstIndex = 0u;
        for (size_t source = 0u; source < sources.size(); ++source)
        {
            auto& range = storage->ranges[source];
            range.firstIndex = firstIndex; range.indexCount = reducedRanges[source].count;
            range.maximumWorldError = reducedRanges[source].error; firstIndex += range.indexCount;
            measured.maximumWorldError = (std::max)(measured.maximumWorldError, range.maximumWorldError);
        }
        measured.farIndices = static_cast<uint32_t>(farIndices.size());
        measured.hasFarGeometry = farIndices.size() < merged.indices.size();
        storage->farIndices = farIndices;
        // vector capacity와 나중의 선택 source 목록까지 포함한 파생 CPU 자료
        // 예약량이다. 외부 재질/driver 메모리나 전체 process 사용량은 별도 측정한다.
        measured.retainedCpuBytes = uint64_t(storage->farIndices.capacity()) * sizeof(uint32_t) +
            uint64_t(storage->ranges.capacity()) * sizeof(STATIC_CLUSTER_SOURCE_RANGE) +
            uint64_t(storage->sourceIds.capacity()) * sizeof(uint64_t) + uint64_t(sources.size()) * sizeof(uint32_t);
        measured.decodedCacheBytes = cache.decodedBytes;

        auto staged = shared_ptr<CModel>(new CModel(first.m_pDevice, first.m_pContext));
        staged->m_eType = MODEL::NONANIM;
        XMStoreFloat4x4(&staged->m_PreTransformMatrix, XMMatrixIdentity());
        staged->m_fGeometryPreScale = 1.f;
        auto mesh = shared_ptr<CMesh>(new CMesh(first.m_pDevice, first.m_pContext));
        HRESULT result = mesh->Initialize_Prototype(MODEL::NONANIM,
            merged, MODEL_SKELETON_DATA{}, XMMatrixIdentity(), true, false);
        if (FAILED(result)) return result;
        // 어떤 source도 축약되지 않았다면 기존 full-far API도 near IB를
        // 유지해야 한다. range metadata/cache는 남지만 불필요한 far GPU IB는 만들지 않는다.
        result = mesh->Prepare_StaticClusterStreams(sourceIndices, measured.hasFarGeometry ?
            std::span<const uint32_t>(farIndices) : std::span<const uint32_t>{});
        if (FAILED(result)) return result;
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = static_cast<UINT>(records.size() * sizeof(VTXMESHINSTANCE));
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        desc.StructureByteStride = sizeof(VTXMESHINSTANCE);
        D3D11_SUBRESOURCE_DATA data{};
        data.pSysMem = records.data();
        ComPtr<ID3D11Buffer> buffer;
        if (FAILED(result = first.m_pDevice->CreateBuffer(&desc, &data, &buffer))) return result;
        D3D11_SHADER_RESOURCE_VIEW_DESC srv{};
        srv.Format = DXGI_FORMAT_UNKNOWN;
        srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srv.Buffer.NumElements = static_cast<UINT>(records.size());
        if (FAILED(result = first.m_pDevice->CreateShaderResourceView(buffer.Get(), &srv, &storage->sourceRecords))) return result;
        staged->m_iNumMeshes = staged->m_iNumMaterials = 1u;
        staged->m_Meshes.push_back(std::move(mesh));
        staged->m_Materials.push_back(storage->materials.front());
        staged->m_vLocalBoundsMin = measured.boundsMin;
        staged->m_vLocalBoundsMax = measured.boundsMax;
        staged->m_bHasLocalBounds = true;
        staged->m_StaticCluster = storage;
        stats = measured;
        output = std::move(staged);
        return S_OK;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (const std::length_error&) { return E_OUTOFMEMORY; }
}

HRESULT CModel::Bind_StaticClusterSources(const shared_ptr<CShader>& shader) const
{
    if (!shader || !m_StaticCluster || !m_StaticCluster->sourceRecords) return E_INVALIDARG;
    if (FAILED(shader->Bind_Texture("g_MapClusterSources", m_StaticCluster->sourceRecords))) return E_FAIL;
    if (m_StaticCluster->materials.size() == 1u)
    {
        const uint32_t disabled = 0u;
        return shader->Bind_RawValue("g_MapLightingBankSize", &disabled, sizeof(disabled));
    }
    std::array<const CMaterial*, 8u> materials{};
    for (size_t i = 0u; i < m_StaticCluster->materials.size(); ++i) materials[i] = m_StaticCluster->materials[i].get();
    return materials[0]->Bind_StaticLightingBank(shader,
        std::span<const CMaterial* const>(materials.data(), m_StaticCluster->materials.size()), true);
}

HRESULT CModel::Render_StaticCluster(bool_t farGeometry)
{
    if (!m_StaticCluster || m_Meshes.size() != 1u || !m_Meshes[0]) return E_INVALIDARG;
    const HRESULT result = m_Meshes[0]->Render_StaticCluster(farGeometry);
    if (result == S_OK)
        if (auto* profiler = CGameInstance::Get().Get_Profiler()) profiler->Record_ModelSubmitted(this);
    return result;
}

const CModel::STATIC_CLUSTER_STATS* CModel::Get_StaticClusterStats() const
{ return m_StaticCluster ? &m_StaticCluster->stats : nullptr; }

std::span<const uint64_t> CModel::Get_StaticClusterSourceIds() const
{ return m_StaticCluster ? std::span<const uint64_t>(m_StaticCluster->sourceIds) : std::span<const uint64_t>{}; }


std::span<const CModel::STATIC_CLUSTER_SOURCE_RANGE> CModel::Get_StaticClusterSourceRanges() const
{ return m_StaticCluster ? std::span<const STATIC_CLUSTER_SOURCE_RANGE>(m_StaticCluster->ranges) : std::span<const STATIC_CLUSTER_SOURCE_RANGE>{}; }

HRESULT CModel::Prepare_StaticClusterVisibleSources(std::span<const uint32_t> sourceSlots)
{
    if (!m_StaticCluster || !m_StaticCluster->stats.hasFarGeometry) return E_INVALIDARG;
    if (m_StaticClusterSelection && m_StaticClusterSelection->prepared &&
        sourceSlots.size() == m_StaticClusterSelection->sources.size() &&
        std::equal(sourceSlots.begin(), sourceSlots.end(), m_StaticClusterSelection->sources.begin())) return S_OK;
    uint64_t count = 0u;
    for (size_t i = 0u; i < sourceSlots.size(); ++i)
    {
        if (sourceSlots[i] >= m_StaticCluster->ranges.size() || (i && sourceSlots[i - 1u] >= sourceSlots[i])) return E_INVALIDARG;
        const auto& range = m_StaticCluster->ranges[sourceSlots[i]];
        if (!range.indexCount) return E_INVALIDARG;
        count += range.indexCount;
    }
    if (count > m_StaticCluster->farIndices.size()) return E_INVALIDARG;
    try
    {
        // 새 CPU 선택과 버퍼 준비가 성공한 뒤 한 번에 commit한다. 실패하면
        // 호출자가 claim을 켜지 않으므로 원본 제출을 유지하며 이전 상태도 보존한다.
        auto staged = make_shared<STATIC_CLUSTER_SELECTION>();
        staged->sources.assign(sourceSlots.begin(), sourceSlots.end());
        if (m_StaticClusterSelection) staged->indices = m_StaticClusterSelection->indices;
        if (count)
        {
            if (!staged->indices)
            {
                D3D11_BUFFER_DESC desc{};
                desc.ByteWidth = static_cast<UINT>(m_StaticCluster->farIndices.size() * sizeof(uint32_t));
                desc.Usage = D3D11_USAGE_DYNAMIC; desc.BindFlags = D3D11_BIND_INDEX_BUFFER; desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                const HRESULT result = m_pDevice->CreateBuffer(&desc, nullptr, &staged->indices);
                if (FAILED(result)) return result;
            }
            D3D11_MAPPED_SUBRESOURCE mapped{};
            const HRESULT result = m_pContext->Map(staged->indices.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mapped);
            if (FAILED(result)) return result;
            uint32_t* target = static_cast<uint32_t*>(mapped.pData);
            for (const uint32_t source : sourceSlots)
            {
                const auto& range = m_StaticCluster->ranges[source];
                std::memcpy(target, m_StaticCluster->farIndices.data() + range.firstIndex, range.indexCount * sizeof(uint32_t));
                target += range.indexCount;
            }
            m_pContext->Unmap(staged->indices.Get(), 0u);
        }
        staged->indexCount = static_cast<uint32_t>(count); staged->prepared = true;
        m_StaticClusterSelection = std::move(staged); return S_OK;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
}

HRESULT CModel::Render_StaticClusterVisible()
{
    if (!m_StaticCluster || !m_StaticClusterSelection || !m_StaticClusterSelection->prepared ||
        m_Meshes.size() != 1u || !m_Meshes[0]) return E_INVALIDARG;
    const auto& selection = *m_StaticClusterSelection;
    if (!selection.indexCount) return S_OK;
    const auto& mesh = *m_Meshes[0];
    if (!selection.indices || !mesh.m_pVB || !mesh.m_ClusterSourceIndices) return E_UNEXPECTED;
    ID3D11Buffer* buffers[] = {mesh.m_pVB.Get(), mesh.m_ClusterSourceIndices.Get()};
    const UINT strides[] = {sizeof(VTXMESH), sizeof(uint32_t)}, offsets[] = {0u, 0u};
    m_pContext->IASetVertexBuffers(0u, 2u, buffers, strides, offsets);
    m_pContext->IASetIndexBuffer(selection.indices.Get(), DXGI_FORMAT_R32_UINT, 0u);
    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pContext->DrawIndexed(selection.indexCount, 0u, 0);
    if (auto* profiler = CGameInstance::Get().Get_Profiler())
    {
        profiler->Add_Counter(EProfilerCounter::DrawCalls);
        profiler->Add_Counter(EProfilerCounter::Indices, selection.indexCount);
        profiler->Record_MeshSubmitted(&mesh, selection.indexCount, 1u, "static-cluster-visible", mesh.m_iNumVertices, 0u);
        profiler->Record_ModelSubmitted(this);
    }
    return S_OK;
}
