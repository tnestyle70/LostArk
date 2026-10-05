#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "MapAssetCatalog.h"
#include "MapLoadScope.h"
#include <memory>
#include <span>
#include <vector>

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)
class CMapStaticBatchObject;
class CMapStaticChunkObject;

// Presentation-only controls. No authoring document or placement is modified.
struct MAP_CHUNK_POLICY final
{
    bool enabled = true;
    bool hlodEnabled = true;
    // Experimental atlas coverage has not passed the stored-camera quality gate.
    bool atlasEnabled = false;
};
struct MAP_CHUNK_CLAIM final
{
    std::shared_ptr<MAP_CHUNK_POLICY> policy;
    bool valid = true;
    bool frameActive = false;
    CMapStaticChunkObject* owner = nullptr;
    std::vector<uint8_t> activeMembers;
    bool Is_Active(uint32_t member = UINT32_MAX) const;
};
struct MAP_CHUNK_BUILD_STATS final
{
    uint32_t chunks = 0, sourceDraws = 0, sources = 0, farChunks = 0, rejectedGroups = 0;
    uint64_t gpuBytes = 0, nearIndices = 0, farIndices = 0;
    double buildMilliseconds = 0.;
};
#ifdef _DEBUG
struct MAP_CHUNK_DEBUG_ROW final
{
    uint32_t chunkId = 0;
    float3_t minimum{}, maximum{};
    std::string materialName;
    std::vector<uint64_t> placementIds;
    std::vector<std::string> assetIds;
    uint32_t sourceDraws = 0, nearIndices = 0, farIndices = 0;
    bool active = false, valid = false, farSelected = false, submitted = false;
};
#endif

class CMapStaticChunkObject final : public CGameObject
{
public:
    struct SOURCE_MEMBER final
    {
        std::weak_ptr<CMapStaticBatchObject> batch;
        uint32_t mesh = 0u;
        // Local batch instance index -> immutable proxy source slot.
        std::vector<uint32_t> sourceSlots;
    };
    struct DESC final : GAMEOBJECT_DESC
    {
        uint32_t prototypeLevelIndex = 0;
        std::shared_ptr<Engine::CModel> model;
        std::shared_ptr<MAP_CHUNK_CLAIM> claim;
        std::shared_ptr<CMapStaticBatchObject> timeSource;
        MAP_ASSET_RENDER_PROFILE profile;
        MAP_FRUSTUM_CULLING_POLICY culling;
        bool mirrored = false;
        bool proxy = false;
        uint32_t sourceDraws = 0;
        std::vector<SOURCE_MEMBER> members;
#ifdef _DEBUG
        std::vector<std::string> sourceAssetIds;
#endif
    };
    static constexpr float CellMetres = 32.f;
    static constexpr const wchar_t* LayerTag = L"Layer_MapStaticChunk";
    static constexpr const wchar_t* PrototypeTag = L"Prototype_GameObject_MapStaticChunk";
    static constexpr const wchar_t* ShaderTag = L"Prototype_Component_Shader_VtxMeshMapChunk";
    static void Stage(uint32_t levelIndex,
        std::span<const std::shared_ptr<CMapStaticBatchObject>> batches,
        const MAP_FRUSTUM_CULLING_POLICY& culling,
        const std::shared_ptr<MAP_CHUNK_POLICY>& policy,
        std::vector<std::shared_ptr<CMapStaticChunkObject>>& output, MAP_CHUNK_BUILD_STATS& stats);
    static void Stage_Proxy(uint32_t levelIndex,
        std::span<const std::shared_ptr<CMapStaticBatchObject>> batches,
        const MAP_FRUSTUM_CULLING_POLICY& culling,
        const std::shared_ptr<MAP_CHUNK_POLICY>& policy,
        std::vector<std::shared_ptr<CMapStaticChunkObject>>& output, MAP_CHUNK_BUILD_STATS& stats);
    static void Remove(uint32_t levelIndex, std::vector<std::shared_ptr<CMapStaticChunkObject>>& objects);

    HRESULT Initialize_Prototype() override { return S_OK; }
    ~CMapStaticChunkObject() override;
    HRESULT Initialize(void* arg) override;
    uint8_t Get_UpdatePhaseMask() const override { return UPDATE_PHASE_LATE; }
    void Late_Update(float) override;
    bool_t Uses_FinalCameraSubmission() const override { return true; }
    bool_t Try_GetFinalCameraSpatialBounds(FINAL_CAMERA_SPATIAL_BOUNDS& bounds) const override;
    void Submit_FinalCamera() override;
    HRESULT Render() override;
#ifdef _DEBUG
    MAP_CHUNK_DEBUG_ROW Get_DebugRow(uint32_t chunkId) const;
#endif
    static std::unique_ptr<CMapStaticChunkObject> Create(ComPtr<ID3D11Device>, ComPtr<ID3D11DeviceContext>);
    std::shared_ptr<CPrototype> Clone(void* arg) override;

private:
    friend struct MAP_CHUNK_CLAIM;
    void Resolve_RenderSelection();
    void Resolve_ProxySelection(const struct MAP_CAMERA_CULL_SNAPSHOT& camera);
    CMapStaticChunkObject(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context);
    CMapStaticChunkObject(const CMapStaticChunkObject& prototype);
    std::shared_ptr<Engine::CModel> m_Model;
    std::shared_ptr<Engine::CShader> m_Shader;
    std::shared_ptr<MAP_CHUNK_CLAIM> m_Claim;
    std::weak_ptr<CMapStaticBatchObject> m_TimeSource;
    MAP_ASSET_RENDER_PROFILE m_Profile;
    MAP_FRUSTUM_CULLING_POLICY m_Culling;
    MAP_FRUSTUM_RUNTIME_STATE m_FrustumState{};
    float3_t m_Center{};
    float m_Radius = 0.f;
    bool m_Mirrored = false, m_Far = false, m_Proxy = false;
    uint32_t m_SourceDraws = 0;
    uint64_t m_GpuBytes = 0;
    std::vector<SOURCE_MEMBER> m_Members;
    std::vector<uint32_t> m_VisibleSourceSlots, m_DrawRepresentatives, m_DrawRepresentativeLods;
    std::vector<uint8_t> m_CandidateActiveMembers;
    uint64_t m_SelectedIndices = 0u, m_OriginalVisibleIndices = 0u;
    uint32_t m_SelectedSourceDraws = 0u;
    bool m_ModeAllowed = false, m_SelectionResolved = false;
#ifdef _DEBUG
    std::vector<std::string> m_SourceAssetIds;
    bool m_Submitted = false;
#endif
};
NS_END
