// Included by Model.cpp after its private material source definition.
#pragma push_macro("new")
#undef new
#include "../ThirdPartyLib/meshoptimizer/meshoptimizer.h"
#include <bit>
#include <unordered_map>
#pragma pop_macro("new")

struct CModel::STATIC_CLUSTER_BUILD_CACHE final
{
    std::map<std::filesystem::path, std::shared_ptr<const MODEL_ASSET_DATA>> assets;
};

struct CModel::STATIC_CLUSTER_STORAGE final
{
    STATIC_CLUSTER_STATS stats;
    vector<uint64_t> sourceIds;
    vector<shared_ptr<CMaterial>> materials;
    ComPtr<ID3D11ShaderResourceView> sourceRecords;
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
    // The immutable cluster bounds/LOD error do not include animated vertices.
    if (surface.sourceFoliageWind || otherSurface.sourceFoliageWind ||
        Has_MaterialTexture(meshIndex, aiTextureType_OPACITY) ||
        other.Has_MaterialTexture(otherMeshIndex, aiTextureType_OPACITY)) return false;
    return m_Materials[a]->Can_BatchStaticLightingWith(*other.m_Materials[b], true);
}

namespace
{
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

            const vector<MODEL_MESH_DATA>* meshes = model.m_pOrderedStaticGeometrySource.get();
            if (!meshes)
            {
                if (!model.m_pMaterialSource || !model.m_bHasSelfConsistentUnauthenticatedGeometryMetadata)
                    return S_FALSE;
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
                // A file replaced after its prototype was loaded must not silently
                // turn a derived cluster into a different version of that asset.
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
            const matrix_t preTransform = XMLoadFloat4x4(&model.m_PreTransformMatrix);
            for (size_t index = 0u; index < mesh.vertices.size(); ++index)
            {
                VTXMESH vertex = mesh.vertices[index];
                vertex.color0Rgba8 = mesh.hasColor0 ? mesh.color0Rgba8[index] : 0xffffffffu;
                // Exactly the existing CMesh upload pretransform. VS_MAIN still
                // owns the world inverse-transpose/Gram-Schmidt and RNM basis.
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
            merged.hasTexcoord1 |= mesh.hasTexcoord1;
            merged.hasTexcoord2 |= mesh.hasTexcoord2;
        }
        measured.materialCount = static_cast<uint32_t>(storage->materials.size());
        measured.clusterVertices = static_cast<uint32_t>(merged.vertices.size());
        measured.sourceIndices = measured.nearIndices = static_cast<uint32_t>(merged.indices.size());

        // Simplify the entire transformed spatial cluster, including different
        // meshes. Retain original local vertices: surviving indices still select
        // the exact source UV0/1/2, COLOR0 and per-source transform/lighting record.
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
        // All duplicates of a cross-source seam must carry the same position lock.
        for (size_t i = 0u; i < locks.size(); ++i)
            if (locks[positions.at(ClusterPositionKey(worldPositions[i]))]) locks[i] = 1u;
        vector<uint32_t> farIndices(merged.indices.size());
        float error = 0.f;
        const size_t target = (static_cast<size_t>(merged.indices.size() * options.targetRatio) / 3u) * 3u;
        const size_t reduced = meshopt_simplifyWithAttributes(farIndices.data(), merged.indices.data(),
            merged.indices.size(), &worldPositions[0].x, worldPositions.size(), sizeof(float3_t),
            attributes[0].data(), sizeof(attributes[0]), weights, attributeCount, locks.data(),
            (std::max)(size_t(3u), target), options.maximumWorldError,
            meshopt_SimplifyLockBorder | meshopt_SimplifyErrorAbsolute, &error);
        bool validFar = reduced > 0u && reduced % 3u == 0u && reduced < merged.indices.size() * .95 &&
            std::isfinite(error) && error >= 0.f && error <= options.maximumWorldError * 1.001f;
        for (size_t i = 0u; validFar && i < reduced; i += 3u)
        {
            const auto a = farIndices[i], b = farIndices[i+1u], c = farIndices[i+2u];
            validFar = a < sourceIndices.size() && b < sourceIndices.size() && c < sourceIndices.size() &&
                sourceIndices[a] == sourceIndices[b] && sourceIndices[a] == sourceIndices[c];
        }
        if (validFar)
        {
            farIndices.resize(reduced);
            measured.farIndices = static_cast<uint32_t>(reduced);
            measured.maximumWorldError = error;
            measured.hasFarGeometry = true;
        }
        else farIndices.clear();

        auto staged = shared_ptr<CModel>(new CModel(first.m_pDevice, first.m_pContext));
        staged->m_eType = MODEL::NONANIM;
        XMStoreFloat4x4(&staged->m_PreTransformMatrix, XMMatrixIdentity());
        staged->m_fGeometryPreScale = 1.f;
        auto mesh = shared_ptr<CMesh>(new CMesh(first.m_pDevice, first.m_pContext));
        HRESULT result = mesh->Initialize_Prototype(MODEL::NONANIM,
            merged, MODEL_SKELETON_DATA{}, XMMatrixIdentity(), true, false);
        if (FAILED(result)) return result;
        result = mesh->Prepare_StaticClusterStreams(sourceIndices, farIndices);
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
