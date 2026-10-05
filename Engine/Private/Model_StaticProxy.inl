// Included by Model.cpp after Model_StaticCluster.inl and its decode cache.
#pragma push_macro("new")
#undef new
#include "../ThirdPartyLib/xatlas/xatlas.h"
#include <bcrypt.h>
#include <fstream>
#include <numeric>
#pragma pop_macro("new")
#pragma comment(lib, "bcrypt.lib")

namespace
{
    class ProxyHash final
    {
    public:
        ProxyHash()
        {
            valid = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) >= 0 &&
                BCryptCreateHash(algorithm, &hash, nullptr, 0, nullptr, 0, 0) >= 0;
        }
        ~ProxyHash() { if (hash) BCryptDestroyHash(hash); if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0); }
        void Add(const void* bytes, size_t size)
        {
            if (!valid || size == 0u) return;
            if (!bytes || size > ULONG_MAX) { valid = false; return; }
            valid = BCryptHashData(hash, static_cast<PUCHAR>(const_cast<void*>(bytes)), static_cast<ULONG>(size), 0) >= 0;
        }
        template<class T> void Add(const vector<T>& data) { Add(data.data(), data.size() * sizeof(T)); }
        bool Finish(array<uint8_t, 32>& digest)
        { return valid && BCryptFinishHash(hash, digest.data(), static_cast<ULONG>(digest.size()), 0) >= 0; }
    private:
        BCRYPT_ALG_HANDLE algorithm = nullptr;
        BCRYPT_HASH_HANDLE hash = nullptr;
        bool valid = false;
    };

    struct PROXY_CACHE_HEADER final
    {
        uint32_t magic = 0x3158504cu, version = 1u;
        uint32_t vertices = 0u, indices = 0u, sources = 0u;
        uint32_t width = 0u, height = 0u, charts = 0u;
        float texelsPerUnit = 0.f;
        array<uint8_t, 32> input{}, output{};
    };
    static_assert(sizeof(PROXY_CACHE_HEADER) == 100u);

    std::filesystem::path ProxyCachePath(const array<uint8_t, 32>& digest)
    {
        wchar_t local[MAX_PATH]{};
        const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH);
        if (!length || length >= MAX_PATH) return {};
        const char digits[] = "0123456789abcdef";
        string name;
        for (uint8_t byte : digest) { name += digits[byte >> 4u]; name += digits[byte & 15u]; }
        return std::filesystem::path(local) / L"LostArk" / L"MapProxy" / L"Geometry-v1" / (name + ".bin");
    }

    uint64_t ProxyGeometryBytes(size_t vertices, size_t indices, size_t sources)
    {
        // Includes capacity for the full dynamic visible IB and source metadata.
        return uint64_t(vertices) * (sizeof(VTXMESH) + sizeof(CModel::STATIC_PROXY_VERTEX)) +
            uint64_t(indices) * sizeof(uint32_t) * 2u +
            uint64_t(sources) * (sizeof(VTXMESHINSTANCE) + sizeof(CModel::STATIC_PROXY_METADATA));
    }

    bool ProxyGeometryDigest(const PROXY_CACHE_HEADER& header, const vector<VTXMESH>& vertices,
        const vector<CModel::STATIC_PROXY_VERTEX>& metadata, const vector<uint32_t>& indices,
        const vector<VTXMESHINSTANCE>& records, array<uint8_t, 32>& digest)
    {
        ProxyHash hash;
        hash.Add(&header.width, sizeof(header.width)); hash.Add(&header.height, sizeof(header.height));
        hash.Add(&header.texelsPerUnit, sizeof(header.texelsPerUnit));
        hash.Add(vertices); hash.Add(metadata); hash.Add(indices); hash.Add(records);
        return hash.Finish(digest);
    }

    bool ValidateProxyLayout(const PROXY_CACHE_HEADER& header, const CModel::STATIC_PROXY_OPTIONS& options,
        const vector<VTXMESH>& vertices, const vector<CModel::STATIC_PROXY_VERTEX>& metadata,
        const vector<uint32_t>& indices, const vector<CModel::STATIC_PROXY_SOURCE_RANGE>& ranges)
    {
        if (header.magic != 0x3158504cu || header.version != 1u || vertices.empty() || indices.empty() ||
            header.vertices != vertices.size() || metadata.size() != vertices.size() ||
            header.indices != indices.size() || header.sources != ranges.size() ||
            header.width != options.atlasResolution || header.height != options.atlasResolution ||
            !header.charts || !std::isfinite(header.texelsPerUnit) || header.texelsPerUnit <= 0.f ||
            ProxyGeometryBytes(vertices.size(), indices.size(), ranges.size()) > options.maximumGeometryBytes) return false;
        for (size_t i = 0u; i < vertices.size(); ++i)
        {
            const auto& uv = metadata[i].atlasUV;
            if (!ClusterFinite(&vertices[i].vPosition.x, 16u) || !ClusterFinite(&vertices[i].vTexcoord2.x, 2u) ||
                !std::isfinite(uv.x) || !std::isfinite(uv.y) || uv.x < 0.f || uv.y < 0.f ||
                uv.x > 1.f || uv.y > 1.f || metadata[i].sourceIndex >= ranges.size()) return false;
        }
        size_t end = 0u;
        for (size_t source = 0u; source < ranges.size(); ++source)
        {
            const auto& range = ranges[source];
            if (range.firstIndex != end || range.indexCount == 0u || range.indexCount % 3u ||
                range.indexCount > indices.size() - end) return false;
            end += range.indexCount;
            for (size_t index = range.firstIndex; index < end; ++index)
                if (indices[index] >= vertices.size() || metadata[indices[index]].sourceIndex != source) return false;
        }
        return end == indices.size();
    }

    bool LoadProxyLayout(const std::filesystem::path& path, const array<uint8_t, 32>& input,
        const CModel::STATIC_PROXY_OPTIONS& options, const vector<CModel::STATIC_PROXY_SOURCE_RANGE>& ranges,
        const vector<VTXMESHINSTANCE>& records, PROXY_CACHE_HEADER& header, vector<VTXMESH>& vertices,
        vector<CModel::STATIC_PROXY_VERTEX>& metadata, vector<uint32_t>& indices)
    {
        if (path.empty()) return false;
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) return false;
        const auto bytes = file.tellg();
        file.seekg(0);
        PROXY_CACHE_HEADER staged{};
        if (!file.read(reinterpret_cast<char*>(&staged), sizeof(staged)) || staged.magic != 0x3158504cu ||
            staged.version != 1u || staged.input != input || !staged.vertices || !staged.indices ||
            staged.sources != ranges.size() || ProxyGeometryBytes(staged.vertices, staged.indices, staged.sources) > options.maximumGeometryBytes ||
            uint64_t(bytes) != sizeof(staged) + uint64_t(staged.vertices) * 88u + uint64_t(staged.indices) * 4u) return false;
        vertices.resize(staged.vertices); metadata.resize(staged.vertices); indices.resize(staged.indices);
        if (!file.read(reinterpret_cast<char*>(vertices.data()), vertices.size() * sizeof(VTXMESH)) ||
            !file.read(reinterpret_cast<char*>(metadata.data()), metadata.size() * sizeof(CModel::STATIC_PROXY_VERTEX)) ||
            !file.read(reinterpret_cast<char*>(indices.data()), indices.size() * sizeof(uint32_t)) ||
            !ValidateProxyLayout(staged, options, vertices, metadata, indices, ranges)) return false;
        array<uint8_t, 32> digest{};
        if (!ProxyGeometryDigest(staged, vertices, metadata, indices, records, digest) || digest != staged.output) return false;
        header = staged;
        return true;
    }

    void SaveProxyLayout(const std::filesystem::path& path, const PROXY_CACHE_HEADER& header,
        const vector<VTXMESH>& vertices, const vector<CModel::STATIC_PROXY_VERTEX>& metadata,
        const vector<uint32_t>& indices)
    {
        if (path.empty()) return;
        std::error_code error;
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) return;
        static volatile LONG serial = 0;
        auto temporary = path;
        temporary += L"." + std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(InterlockedIncrement(&serial)) + L".tmp";
        {
            std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
            if (!file) return;
            file.write(reinterpret_cast<const char*>(&header), sizeof(header));
            file.write(reinterpret_cast<const char*>(vertices.data()), vertices.size() * sizeof(VTXMESH));
            file.write(reinterpret_cast<const char*>(metadata.data()), metadata.size() * sizeof(CModel::STATIC_PROXY_VERTEX));
            file.write(reinterpret_cast<const char*>(indices.data()), indices.size() * sizeof(uint32_t));
            file.flush();
            if (!file) { file.close(); std::filesystem::remove(temporary, error); return; }
        }
        if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            std::filesystem::remove(temporary, error);
    }

    HRESULT ProxyStructuredBuffer(ID3D11Device* device, const void* data, uint32_t count,
        uint32_t stride, ComPtr<ID3D11ShaderResourceView>& output)
    {
        if (!device || !data || !count || !stride || count > UINT32_MAX / stride) return E_INVALIDARG;
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = count * stride; desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        desc.StructureByteStride = stride;
        D3D11_SUBRESOURCE_DATA initial{}; initial.pSysMem = data;
        ComPtr<ID3D11Buffer> buffer;
        HRESULT result = device->CreateBuffer(&desc, &initial, &buffer);
        if (FAILED(result)) return result;
        D3D11_SHADER_RESOURCE_VIEW_DESC srv{};
        srv.Format = DXGI_FORMAT_UNKNOWN; srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER; srv.Buffer.NumElements = count;
        return device->CreateShaderResourceView(buffer.Get(), &srv, &output);
    }
}

struct CModel::STATIC_PROXY_STORAGE final
{
    STATIC_PROXY_STATS stats;
    vector<STATIC_PROXY_SOURCE_RANGE> ranges;
    vector<uint32_t> indices;
    ComPtr<ID3D11Buffer> vertexMetadata;
    ComPtr<ID3D11ShaderResourceView> sourceRecords;
    array<uint8_t, 32> identity{};
};

HRESULT CModel::Create_StaticProxy(std::span<const STATIC_CLUSTER_SOURCE> sources,
    const STATIC_PROXY_OPTIONS& options, STATIC_CLUSTER_BUILD_CACHE& cache,
    shared_ptr<CModel>& output, STATIC_PROXY_STATS& stats)
{
    if (sources.size() < 2u || sources.size() > options.maximumSources || options.maximumSources > 256u ||
        !sources.front().model || options.atlasResolution < 256u || options.atlasResolution > 8192u ||
        (options.atlasResolution & (options.atlasResolution - 1u)) || options.padding > options.atlasResolution / 16u ||
        !std::isfinite(options.texelsPerUnit) || options.texelsPerUnit <= 0.f ||
        options.maximumGeometryBytes == 0u || options.maximumGeometryBytes > 64ull * 1024ull * 1024ull) return E_INVALIDARG;
    try
    {
        const auto& first = *sources.front().model;
        auto storage = make_shared<STATIC_PROXY_STORAGE>();
        vector<VTXMESH> original;
        vector<float3_t> worldPositions;
        vector<uint32_t> originalIndices, firstVertices, vertexCounts;
        vector<VTXMESHINSTANCE> records;
        bool mirrored = false;
        const float maximum = (std::numeric_limits<float>::max)();
        float3_t minimum{maximum,maximum,maximum}, maximumPoint{-maximum,-maximum,-maximum};
        for (size_t sourceIndex = 0u; sourceIndex < sources.size(); ++sourceIndex)
        {
            const auto& source = sources[sourceIndex];
            if (!source.model) return E_INVALIDARG;
            const auto& model = *source.model;
            if (model.m_eType != MODEL::NONANIM || model.m_StaticCluster || model.m_StaticProxy ||
                model.m_pDevice != first.m_pDevice || model.m_pContext != first.m_pContext ||
                source.meshIndex >= model.m_Meshes.size() || !model.m_Meshes[source.meshIndex] ||
                model.m_Meshes[source.meshIndex]->Has_MorphBaseVertices()) return S_FALSE;
            if (!ClusterAffine(model.m_PreTransformMatrix) || !ClusterAffine(source.instance.World) ||
                !ClusterFinite(reinterpret_cast<const float*>(&source.instance), sizeof(VTXMESHINSTANCE) / sizeof(float))) return E_INVALIDARG;
            const matrix_t world = XMLoadFloat4x4(&source.instance.World);
            const float determinant = XMVectorGetX(XMMatrixDeterminant(world));
            if (!std::isfinite(determinant) || std::abs(determinant) < 1.e-12f) return E_INVALIDARG;
            if (sourceIndex == 0u) mirrored = determinant < 0.f;
            else if (mirrored != (determinant < 0.f)) return S_FALSE;
            const vector<MODEL_MESH_DATA>* meshes = model.m_pOrderedStaticGeometrySource.get();
            if (!meshes)
            {
                if (!model.m_pMaterialSource || !model.m_bHasSelfConsistentUnauthenticatedGeometryMetadata) return S_FALSE;
                const auto& identity = model.m_pMaterialSource->identity;
                const auto path = identity.meshPath.lexically_normal();
                auto found = cache.assets.find(path);
                if (found == cache.assets.end())
                {
                    auto decoded = make_shared<MODEL_ASSET_DATA>();
                    if (!CModelDecoderRegistry::Get().Decode(identity, *decoded)) return E_FAIL;
                    found = cache.assets.emplace(path, std::move(decoded)).first;
                }
                const auto& decoded = *found->second;
                if (!decoded.geometryMetadata.present || decoded.hasSkeleton ||
                    decoded.geometryMetadata.payloadSha256 != model.m_GeometryPayloadSha256 ||
                    decoded.geometryMetadata.metadataIdentitySha256 != model.m_GeometryMetadataIdentitySha256 ||
                    decoded.meshes.size() != model.m_Meshes.size()) return E_FAIL;
                meshes = &decoded.meshes;
            }
            if (source.meshIndex >= meshes->size()) return E_INVALIDARG;
            const auto& mesh = (*meshes)[source.meshIndex];
            if (mesh.vertexKind != MODEL_VERTEX_KIND::STATIC || mesh.vertices.empty() || mesh.indices.empty() ||
                mesh.indices.size() % 3u || (mesh.hasColor0 && mesh.color0Rgba8.size() != mesh.vertices.size())) return E_INVALIDARG;
            if (ProxyGeometryBytes(original.size() + mesh.vertices.size(), originalIndices.size() + mesh.indices.size(), sources.size()) >
                options.maximumGeometryBytes) return S_FALSE;
            const auto base = static_cast<uint32_t>(original.size());
            firstVertices.push_back(base); vertexCounts.push_back(static_cast<uint32_t>(mesh.vertices.size()));
            STATIC_PROXY_SOURCE_RANGE range{};
            range.sourceId = source.sourceId; range.sourceMeshIndex = source.meshIndex;
            range.firstIndex = static_cast<uint32_t>(originalIndices.size()); range.indexCount = static_cast<uint32_t>(mesh.indices.size());
            range.boundsMin = {maximum,maximum,maximum}; range.boundsMax = {-maximum,-maximum,-maximum};
            const matrix_t pre = XMLoadFloat4x4(&model.m_PreTransformMatrix);
            for (size_t i = 0u; i < mesh.vertices.size(); ++i)
            {
                VTXMESH vertex = mesh.vertices[i];
                vertex.color0Rgba8 = mesh.hasColor0 ? mesh.color0Rgba8[i] : 0xffffffffu;
                XMStoreFloat3(&vertex.vPosition, XMVector3TransformCoord(XMLoadFloat3(&vertex.vPosition), pre));
                XMStoreFloat3(&vertex.vNormal, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vNormal), pre)));
                XMStoreFloat3(&vertex.vTangent, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vTangent), pre)));
                XMStoreFloat3(&vertex.vBinormal, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vBinormal), pre)));
                if (!ClusterFinite(&vertex.vPosition.x, 16u) || !ClusterFinite(&vertex.vTexcoord2.x, 2u)) return E_INVALIDARG;
                float3_t position;
                XMStoreFloat3(&position, XMVector3TransformCoord(XMLoadFloat3(&vertex.vPosition), world));
                if (!ClusterFinite(&position.x, 3u)) return E_INVALIDARG;
                for (size_t axis = 0u; axis < 3u; ++axis)
                {
                    (&range.boundsMin.x)[axis] = (std::min)((&range.boundsMin.x)[axis], (&position.x)[axis]);
                    (&range.boundsMax.x)[axis] = (std::max)((&range.boundsMax.x)[axis], (&position.x)[axis]);
                }
                original.push_back(vertex); worldPositions.push_back(position);
            }
            for (uint32_t index : mesh.indices)
            { if (index >= mesh.vertices.size()) return E_INVALIDARG; originalIndices.push_back(base + index); }
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                (&minimum.x)[axis] = (std::min)((&minimum.x)[axis], (&range.boundsMin.x)[axis]);
                (&maximumPoint.x)[axis] = (std::max)((&maximumPoint.x)[axis], (&range.boundsMax.x)[axis]);
            }
            storage->ranges.push_back(range); records.push_back(source.instance);
        }
        ProxyHash key;
        constexpr char version[] = "StaticProxy-v1-xatlas-f700c7790aaa030e794b52ba7791a05c085faf0c";
        key.Add(version, sizeof(version)); key.Add(original); key.Add(originalIndices); key.Add(records);
        key.Add(firstVertices); key.Add(vertexCounts);
        key.Add(&options.atlasResolution, sizeof(options.atlasResolution)); key.Add(&options.padding, sizeof(options.padding));
        key.Add(&options.texelsPerUnit, sizeof(options.texelsPerUnit));
        for (const auto& range : storage->ranges) { key.Add(&range.sourceId, sizeof(range.sourceId)); key.Add(&range.sourceMeshIndex, sizeof(range.sourceMeshIndex)); }
        array<uint8_t, 32> input{};
        if (!key.Finish(input)) return E_FAIL;
        const auto cachePath = ProxyCachePath(input);
        PROXY_CACHE_HEADER header{};
        vector<VTXMESH> vertices;
        vector<STATIC_PROXY_VERTEX> metadata;
        bool cached = LoadProxyLayout(cachePath, input, options, storage->ranges, records, header, vertices, metadata, storage->indices);
        if (!cached)
        {
            vertices.clear(); metadata.clear(); storage->indices.clear();
            std::unique_ptr<xatlas::Atlas, void(*)(xatlas::Atlas*)> atlas(xatlas::Create(), xatlas::Destroy);
            if (!atlas) return E_OUTOFMEMORY;
            for (size_t source = 0u; source < sources.size(); ++source)
            {
                xatlas::MeshDecl decl;
                decl.vertexPositionData = worldPositions.data() + firstVertices[source];
                decl.vertexPositionStride = sizeof(float3_t); decl.vertexCount = vertexCounts[source];
                decl.indexData = originalIndices.data() + storage->ranges[source].firstIndex;
                decl.indexCount = storage->ranges[source].indexCount; decl.indexFormat = xatlas::IndexFormat::UInt32;
                decl.indexOffset = -static_cast<int32_t>(firstVertices[source]);
                if (xatlas::AddMesh(atlas.get(), decl, static_cast<uint32_t>(sources.size())) != xatlas::AddMeshError::Success) return E_FAIL;
            }
            xatlas::PackOptions packing;
            packing.resolution = options.atlasResolution; packing.texelsPerUnit = options.texelsPerUnit;
            packing.padding = options.padding; packing.bilinear = true; packing.blockAlign = true;
            xatlas::Generate(atlas.get(), xatlas::ChartOptions{}, packing);
            if (atlas->atlasCount != 1u || atlas->meshCount != sources.size() ||
                atlas->width != options.atlasResolution || atlas->height != options.atlasResolution) return S_FALSE;
            for (size_t source = 0u; source < sources.size(); ++source)
            {
                const auto& mesh = atlas->meshes[source];
                if (!mesh.vertexCount || mesh.indexCount != storage->ranges[source].indexCount ||
                    ProxyGeometryBytes(vertices.size() + mesh.vertexCount, originalIndices.size(), sources.size()) > options.maximumGeometryBytes) return S_FALSE;
                const uint32_t base = static_cast<uint32_t>(vertices.size());
                for (uint32_t i = 0u; i < mesh.vertexCount; ++i)
                {
                    const auto& vertex = mesh.vertexArray[i];
                    if (vertex.xref >= vertexCounts[source] || vertex.atlasIndex != 0) return S_FALSE;
                    vertices.push_back(original[firstVertices[source] + vertex.xref]);
                    metadata.push_back({{vertex.uv[0] / atlas->width, vertex.uv[1] / atlas->height}, static_cast<uint32_t>(source)});
                }
                for (uint32_t i = 0u; i < mesh.indexCount; ++i)
                { if (mesh.indexArray[i] >= mesh.vertexCount) return E_FAIL; storage->indices.push_back(base + mesh.indexArray[i]); }
            }
            header.vertices = static_cast<uint32_t>(vertices.size()); header.indices = static_cast<uint32_t>(storage->indices.size());
            header.sources = static_cast<uint32_t>(sources.size()); header.width = atlas->width; header.height = atlas->height;
            header.charts = atlas->chartCount; header.texelsPerUnit = atlas->texelsPerUnit; header.input = input;
            if (!ValidateProxyLayout(header, options, vertices, metadata, storage->indices, storage->ranges) ||
                !ProxyGeometryDigest(header, vertices, metadata, storage->indices, records, header.output)) return E_FAIL;
        }
        auto staged = shared_ptr<CModel>(new CModel(first.m_pDevice, first.m_pContext));
        staged->m_eType = MODEL::NONANIM; staged->m_fGeometryPreScale = 1.f;
        XMStoreFloat4x4(&staged->m_PreTransformMatrix, XMMatrixIdentity());
        MODEL_MESH_DATA data;
        data.name = "static-atlas-proxy"; data.materialIndex = 0u; data.vertexKind = MODEL_VERTEX_KIND::STATIC;
        data.hasColor0 = data.hasTexcoord1 = data.hasTexcoord2 = true;
        data.vertices = std::move(vertices); data.indices = storage->indices;
        data.color0Rgba8.reserve(data.vertices.size());
        for (const auto& vertex : data.vertices) data.color0Rgba8.push_back(vertex.color0Rgba8);
        auto mesh = shared_ptr<CMesh>(new CMesh(first.m_pDevice, first.m_pContext));
        HRESULT result = mesh->Initialize_Prototype(MODEL::NONANIM, data, MODEL_SKELETON_DATA{}, XMMatrixIdentity(), true, false);
        if (FAILED(result)) return result;
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = static_cast<UINT>(metadata.size() * sizeof(STATIC_PROXY_VERTEX));
        desc.Usage = D3D11_USAGE_IMMUTABLE; desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA initial{}; initial.pSysMem = metadata.data();
        if (FAILED(result = first.m_pDevice->CreateBuffer(&desc, &initial, &storage->vertexMetadata))) return result;
        if (FAILED(result = ProxyStructuredBuffer(first.m_pDevice.Get(), records.data(), static_cast<uint32_t>(records.size()),
            sizeof(VTXMESHINSTANCE), storage->sourceRecords))) return result;
        storage->stats = {header.sources, header.vertices, header.indices, header.width, header.height, header.charts,
            header.texelsPerUnit, ProxyGeometryBytes(header.vertices, header.indices, header.sources), cached};
        storage->identity = header.output;
        staged->m_Meshes.push_back(std::move(mesh)); staged->m_iNumMeshes = 1u;
        staged->m_vLocalBoundsMin = minimum; staged->m_vLocalBoundsMax = maximumPoint; staged->m_bHasLocalBounds = true;
        staged->m_StaticProxy = storage;
        if (!cached) SaveProxyLayout(cachePath, header, data.vertices, metadata, storage->indices);
        stats = storage->stats; output = std::move(staged);
        return S_OK;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (const std::length_error&) { return E_OUTOFMEMORY; }
    catch (const std::filesystem::filesystem_error&) { return E_FAIL; }
}

const CModel::STATIC_PROXY_STATS* CModel::Get_StaticProxyStats() const
{ return m_StaticProxy ? &m_StaticProxy->stats : nullptr; }

array<uint8_t, 32> CModel::Get_StaticProxyGeometryIdentity() const
{ return m_StaticProxy ? m_StaticProxy->identity : array<uint8_t, 32>{}; }

std::span<const CModel::STATIC_PROXY_SOURCE_RANGE> CModel::Get_StaticProxySourceRanges() const
{ return m_StaticProxy ? std::span<const STATIC_PROXY_SOURCE_RANGE>(m_StaticProxy->ranges) : std::span<const STATIC_PROXY_SOURCE_RANGE>{}; }

bool_t CModel::Can_BakeStaticProxy(uint32_t meshIndex) const
{
    if (m_eType != MODEL::NONANIM || m_StaticCluster || m_StaticProxy || meshIndex >= m_Meshes.size() ||
        !m_Meshes[meshIndex] || m_Meshes[meshIndex]->Has_MorphBaseVertices()) return false;
    const uint32_t material = m_Meshes[meshIndex]->Get_MaterialIndex();
    return material < m_Materials.size() && m_Materials[material] && m_Materials[material]->Can_BakeStaticProxy();
}

std::span<const std::filesystem::path> CModel::Get_StaticProxySourcePaths(uint32_t meshIndex) const
{
    if (meshIndex >= m_Meshes.size() || !m_Meshes[meshIndex]) return {};
    const uint32_t material = m_Meshes[meshIndex]->Get_MaterialIndex();
    return material < m_Materials.size() && m_Materials[material] ?
        m_Materials[material]->Get_StaticProxySourcePaths() : std::span<const std::filesystem::path>{};
}

bool_t CModel::Validate_StaticProxySourceFiles(uint32_t meshIndex) const
{
    if (meshIndex >= m_Meshes.size() || !m_Meshes[meshIndex]) return false;
    const uint32_t material = m_Meshes[meshIndex]->Get_MaterialIndex();
    return material < m_Materials.size() && m_Materials[material] && m_Materials[material]->Validate_StaticProxySourceFiles();
}

HRESULT CModel::Set_StaticProxyMetadata(std::span<const STATIC_PROXY_METADATA> metadata)
{
    if (!m_StaticProxy || metadata.size() != m_StaticProxy->ranges.size()) return E_INVALIDARG;
    ComPtr<ID3D11ShaderResourceView> staged;
    const HRESULT result = ProxyStructuredBuffer(m_pDevice.Get(), metadata.data(), static_cast<uint32_t>(metadata.size()), sizeof(STATIC_PROXY_METADATA), staged);
    if (SUCCEEDED(result)) m_StaticProxyMetadata = std::move(staged);
    return result;
}

HRESULT CModel::Set_StaticProxyAtlases(std::span<const ComPtr<ID3D11ShaderResourceView>> atlases)
{
    if (!m_StaticProxy || atlases.empty() || atlases.size() > 8u) return E_INVALIDARG;
    for (const auto& atlas : atlases) if (!atlas) return E_INVALIDARG;
    try { vector<ComPtr<ID3D11ShaderResourceView>> staged(atlases.begin(), atlases.end()); m_StaticProxyAtlases.swap(staged); return S_OK; }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
}

HRESULT CModel::Bind_StaticProxySources(const shared_ptr<CShader>& shader) const
{
    if (!shader || !m_StaticProxy) return E_INVALIDARG;
    if (FAILED(shader->Bind_Texture("g_MapProxySources", m_StaticProxy->sourceRecords))) return E_FAIL;
    // Bake VS uses source records alone. Runtime additionally requires metadata.
    if (m_StaticProxyMetadata && FAILED(shader->Bind_Texture("g_MapProxyMetadata", m_StaticProxyMetadata))) return E_FAIL;
    return S_OK;
}

HRESULT CModel::Bind_StaticProxyAtlases(const shared_ptr<CShader>& shader, std::span<const char* const> shaderNames) const
{
    if (!shader || !m_StaticProxy || !m_StaticProxyMetadata || m_StaticProxyAtlases.empty() || shaderNames.size() != m_StaticProxyAtlases.size()) return E_INVALIDARG;
    for (size_t i = 0u; i < shaderNames.size(); ++i)
        if (!shaderNames[i] || FAILED(shader->Bind_Texture(shaderNames[i], m_StaticProxyAtlases[i]))) return E_FAIL;
    return S_OK;
}

HRESULT CModel::Prepare_StaticProxyVisibleSources(std::span<const uint32_t> sourceSlots)
{
    if (!m_StaticProxy) return E_INVALIDARG;
    if (m_bStaticProxyVisiblePrepared && sourceSlots.size() == m_StaticProxyVisibleSources.size() &&
        std::equal(sourceSlots.begin(), sourceSlots.end(), m_StaticProxyVisibleSources.begin())) return S_OK;
    uint64_t count = 0u;
    for (size_t i = 0u; i < sourceSlots.size(); ++i)
    {
        if (sourceSlots[i] >= m_StaticProxy->ranges.size() || (i && sourceSlots[i - 1u] >= sourceSlots[i])) return E_INVALIDARG;
        count += m_StaticProxy->ranges[sourceSlots[i]].indexCount;
    }
    if (count > m_StaticProxy->indices.size()) return E_INVALIDARG;
    try
    {
        vector<uint32_t> selection(sourceSlots.begin(), sourceSlots.end());
        ComPtr<ID3D11Buffer> buffer = m_StaticProxyVisibleIndices;
        if (count)
        {
            if (!buffer)
            {
                D3D11_BUFFER_DESC desc{};
                desc.ByteWidth = static_cast<UINT>(m_StaticProxy->indices.size() * sizeof(uint32_t));
                desc.Usage = D3D11_USAGE_DYNAMIC; desc.BindFlags = D3D11_BIND_INDEX_BUFFER; desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                const HRESULT result = m_pDevice->CreateBuffer(&desc, nullptr, &buffer);
                if (FAILED(result)) return result;
            }
            D3D11_MAPPED_SUBRESOURCE mapped{};
            const HRESULT result = m_pContext->Map(buffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mapped);
            if (FAILED(result)) return result;
            uint32_t* target = static_cast<uint32_t*>(mapped.pData);
            for (uint32_t source : sourceSlots)
            {
                const auto& range = m_StaticProxy->ranges[source];
                std::memcpy(target, m_StaticProxy->indices.data() + range.firstIndex, range.indexCount * sizeof(uint32_t));
                target += range.indexCount;
            }
            m_pContext->Unmap(buffer.Get(), 0u);
        }
        m_StaticProxyVisibleIndices = std::move(buffer); m_StaticProxyVisibleSources.swap(selection);
        m_iStaticProxyVisibleIndexCount = static_cast<uint32_t>(count); m_bStaticProxyVisiblePrepared = true;
        return S_OK;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
}

HRESULT CModel::Render_StaticProxySource(uint32_t sourceIndex)
{
    if (!m_StaticProxy || sourceIndex >= m_StaticProxy->ranges.size() || m_Meshes.size() != 1u) return E_INVALIDARG;
    const auto& mesh = *m_Meshes[0];
    ID3D11Buffer* buffers[] = {mesh.m_pVB.Get(), m_StaticProxy->vertexMetadata.Get()};
    const UINT strides[] = {sizeof(VTXMESH), sizeof(STATIC_PROXY_VERTEX)}, offsets[] = {0u, 0u};
    m_pContext->IASetVertexBuffers(0u, 2u, buffers, strides, offsets);
    m_pContext->IASetIndexBuffer(mesh.m_pIB.Get(), DXGI_FORMAT_R32_UINT, 0u);
    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    const auto& range = m_StaticProxy->ranges[sourceIndex];
    m_pContext->DrawIndexed(range.indexCount, range.firstIndex, 0);
    return S_OK;
}

HRESULT CModel::Render_StaticProxy()
{
    if (!m_StaticProxy || !m_bStaticProxyVisiblePrepared || m_Meshes.size() != 1u) return E_INVALIDARG;
    if (!m_iStaticProxyVisibleIndexCount) return S_OK;
    if (!m_StaticProxyVisibleIndices) return E_UNEXPECTED;
    const auto& mesh = *m_Meshes[0];
    ID3D11Buffer* buffers[] = {mesh.m_pVB.Get(), m_StaticProxy->vertexMetadata.Get()};
    const UINT strides[] = {sizeof(VTXMESH), sizeof(STATIC_PROXY_VERTEX)}, offsets[] = {0u, 0u};
    m_pContext->IASetVertexBuffers(0u, 2u, buffers, strides, offsets);
    m_pContext->IASetIndexBuffer(m_StaticProxyVisibleIndices.Get(), DXGI_FORMAT_R32_UINT, 0u);
    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pContext->DrawIndexed(m_iStaticProxyVisibleIndexCount, 0u, 0);
    if (auto* profiler = CGameInstance::Get().Get_Profiler())
    {
        profiler->Add_Counter(EProfilerCounter::DrawCalls);
        profiler->Add_Counter(EProfilerCounter::Indices, m_iStaticProxyVisibleIndexCount);
        profiler->Record_MeshSubmitted(&mesh, m_iStaticProxyVisibleIndexCount, 1u, "static-atlas-proxy", mesh.m_iNumVertices, 0u);
        profiler->Record_ModelSubmitted(this);
    }
    return S_OK;
}
