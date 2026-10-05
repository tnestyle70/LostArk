#pragma once

#include "Client_Defines.h"
#include "Engine_VertexTypes.h"
#include "GameObject.h"
#include "MapAssetCatalog.h"
#include "MapLoadScope.h"

#include <cstdint>
#include <memory>
#include <string>
#include <span>
#include <unordered_map>
#include <vector>

NS_BEGIN(Engine)

class CModel;
class CShader;
struct MESH_SCREEN_LOD_DESC;

NS_END

NS_BEGIN(Client)

class CMapStaticChunkObject;
struct MAP_CHUNK_CLAIM;

// One level-owned clock replaces per-object Update/Late callbacks for static batches.
struct MAP_STATIC_BATCH_FRAME_STATE final
{
	f32_t elapsedTime = 0.f;
	uint64_t frameNumber = 0u;
};

struct FMapStaticInstance final
{
	uint64_t PlacementId = {};
	Engine::MODEL_BAKED_LIGHTING_INSTANCE BakedLighting;
	Engine::MODEL_SOURCE_FOLIAGE_WIND_INSTANCE SourceWind;
	float4x4_t World = {};
	float4x4_t WorldInvTranspose = {};
	float3_t WorldBoundsCenter = {};
	f32_t WorldBoundsRadius = {};
	bool_t Visible = true;
	// Cinematic stage overlay, see CMapAssetObject::Set_StageSuppressed.
	bool_t Suppressed = false;
	// Map Tool camera inspection overlay. It does not alter the stage overlay.
	bool_t CameraPreviewSuppressed = false;
	MAP_FRUSTUM_RUNTIME_STATE FrustumState{};
    // Derived presentation state, never serialized with placements.
    float3_t OcclusionBoundsMin{}, OcclusionBoundsMax{};
    bool OcclusionBoundsValid = false, DistanceHidden = false;
    uint64_t DistanceSettingsRevision = 0u;
};

class CMapStaticBatchObject final : public CGameObject
{
public:
	struct DESC final : public CGameObject::GAMEOBJECT_DESC
	{
		uint32_t PrototypeLevelIndex = ETOUI(LEVEL::DEVELOPMENT);
		std::string AssetId;
		std::string AssetGroupId;
		std::wstring ModelPrototypeTag;
		MAP_ASSET_RENDER_PROFILE RenderProfile;
		MAP_FRUSTUM_CULLING_POLICY FrustumCulling{};
		bool_t Mirrored = false;
		std::vector<FMapStaticInstance> Instances;
		std::shared_ptr<MAP_STATIC_BATCH_FRAME_STATE> FrameState;
	};

private:
	CMapStaticBatchObject(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	CMapStaticBatchObject(
		const CMapStaticBatchObject& prototype);

public:
	virtual ~CMapStaticBatchObject();

	virtual HRESULT Initialize_Prototype() override;
	//Clone을 할 때 사용하는 Initialize
	virtual HRESULT Initialize(void* pArg) override;

	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual uint8_t Get_UpdatePhaseMask() const override
	{ return m_FrameState ? 0u : UPDATE_PHASE_UPDATE | UPDATE_PHASE_LATE; }
	virtual bool_t Uses_FinalCameraSubmission() const override { return true; }
	virtual void Submit_FinalCamera() override;
    bool_t Try_PrepareFinalCameraCpuJob(FINAL_CAMERA_CPU_JOB& output) override;
    virtual bool_t Try_GetFinalCameraSpatialBounds(FINAL_CAMERA_SPATIAL_BOUNDS& bounds) const override;
    bool_t Try_GetStaticOcclusionDesc(STATIC_OCCLUSION_DESC& output) const override;
    uint32_t Rasterize_StaticOccluder(Engine::COcclusionCuller& culler, uint32_t triangleBudget) const override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_AdjacentNonBlend(
		std::span<const std::shared_ptr<CGameObject>> objects, size_t& consumed) override;
	virtual HRESULT Render_Shadow() override;
	virtual bool_t Try_GetStaticShadowRevision(uint64_t& outRevision) const override;

public:
	/* Queries current contiguous instances without a placement hash lookup,
	   prototype clone, vertex copy or GPU readback. Direction is normalized. */
	bool_t Try_PickMovementSurface(const float3_t& rayOrigin,
		const float3_t& rayDirection, f32_t maxDistance, f32_t& outDistance) const;
#ifdef _DEBUG
	// Read-only LOD0 inspection also includes vegetation and forward surfaces.
	bool_t Try_PickInspectionSurface(const float3_t& rayOrigin,
		const float3_t& rayDirection, f32_t maxDistance, f32_t& outDistance,
		uint64_t& outPlacementId, uint32_t& outMeshIndex, std::string& outMaterialName) const;
#endif

	//Instance Update
	HRESULT Update_Instance(
		uint64_t placementId,
		const FMapStaticInstance& instance);

	HRESULT Set_InstanceVisible(
		uint64_t placementId,
		bool_t visible);
	HRESULT Try_GetInstanceVisible(
		uint64_t placementId,
		bool_t& outVisible) const;
	/* Hides one instance from the draw and shadow lists without changing its
	   authored Visible flag, so clearing it restores exactly what was there. */
	HRESULT Set_InstanceSuppressed(
		uint64_t placementId,
		bool_t suppressed);
	/* Preview-only suppression used while reviewing camera cuts. It is kept
	   separate from the cinematic stage overlay so either can restore alone. */
	HRESULT Set_InstanceCameraPreviewSuppressed(
		uint64_t placementId,
		bool_t suppressed);

	uint32_t Get_VisibleInstanceCount() const
	{
		return static_cast<uint32_t>(
			m_VisibleInstances.size());
	}

    std::span<const uint32_t> Get_VisibleInstanceIndices() const
    { return m_VisibleInstanceIndices; }

	uint32_t Get_ShadowInstanceCount() const
	{
		return static_cast<uint32_t>(
			m_ShadowInstances.size());
	}

	const std::string& Get_AssetId() const
	{
		return m_AssetId;
	}

	f32_t Get_RenderElapsedTime() const
	{
		return m_FrameState ? m_FrameState->elapsedTime : m_fElapsedTime;
	}

	bool_t Is_Mirrored() const
	{
		return m_bMirrored;
	}

private:
    friend class CMapStaticChunkObject;
	friend class CMapPlacementRuntime;
	bool_t Is_FinalCameraPrepared() const
	{
		const auto viewport = CGameInstance::Get().Get_ViewportSize();
        return m_bFinalCameraPrepared &&
            m_VisibleViewportSize.x == viewport.x && m_VisibleViewportSize.y == viewport.y &&
			(!m_FrameState || m_iPreparedFrame == m_FrameState->frameNumber);
	}
	HRESULT Prepare_FinalCameraVisibility(const struct MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot);
    bool Is_ChunkMeshClaimed(uint32_t mesh) const;
    bool Are_AllChunkMeshesClaimed() const;
    void Rebuild_InstanceOcclusionBounds(FMapStaticInstance& instance) const;
    void Commit_DistanceSelection(uint64_t revision);
    void Invalidate_ChunkClaims();
    std::vector<std::shared_ptr<MAP_CHUNK_CLAIM>> m_ChunkClaims;
    std::vector<uint32_t> m_ChunkClaimMemberIndices;
	HRESULT Ready_Components(uint32_t prototypeLevelIndex,
		const std::wstring& modelPrototypeTag);

	HRESULT Ensure_InstanceCapacity(
		uint32_t requiredCount);
	HRESULT Ensure_ShadowInstanceCapacity(
		uint32_t requiredCount);
	HRESULT Upload_LightingBankInstances();
    HRESULT Try_RenderIdenticalInstances(
        std::span<const std::shared_ptr<CGameObject>> objects, size_t& consumed);
    HRESULT Render_VisibleInstances(ID3D11Buffer* buffer, uint32_t instanceCount,
        const struct MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot);
    bool_t Uses_MeshScreenLod(uint32_t mesh, bool_t hasScreenLod) const;


    struct CPU_VISIBILITY_PREPARATION;
    std::unique_ptr<CPU_VISIBILITY_PREPARATION> m_CpuVisibility;
    bool m_bVisibleDistanceEnabled = false;
    HRESULT Stage_VisibilityCpu(const struct MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot);
    void Compute_VisibilityCpu();
    static void Execute_VisibilityCpu(void* context) noexcept;
    void Invalidate_VisibilityCpu();

	HRESULT Upload_VisibleInstances(
		const struct MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot);
	HRESULT Upload_ShadowInstances();
	HRESULT Rebuild_PlacementLookup();
    void Rebuild_BatchCullBounds() const;
    bool_t Build_ScreenLodView(const struct MAP_CAMERA_CULL_SNAPSHOT& camera,
        Engine::MESH_SCREEN_LOD_DESC& result) const;

private:
	//배치하는 에셋의 ID
	std::string m_AssetId;
	std::string m_AssetGroupId;
	//BatckObject의 Render Profile 정보 
	MAP_ASSET_RENDER_PROFILE m_RenderProfile;
	MAP_FRUSTUM_CULLING_POLICY m_FrustumCulling{};

	bool_t m_bMirrored = false;
    mutable bool_t m_bBatchBoundsDirty = true;
    mutable bool_t m_bHasBatchBounds = false;
    // Map batch geometry is immutable after its model component is cloned.
    bool_t m_bHasStaticMeshLod = false;
    mutable float4_t m_BatchBounds = {};
    mutable MAP_FRUSTUM_RUNTIME_STATE m_BatchFrustumState{};
    float4_t m_VisibleLodBounds = {};
    // Maximum |view X|, |view Y|, minimum view Z, valid flag; same camera
    // revision and transactional lifetime as the uploaded visible payload.
    float4_t m_VisibleTightLodBounds = {};
    f32_t m_fVisibleLodScale = 0.f;
	bool_t m_bShadowInstancesDirty = true;
	bool_t m_bShadowInstancesUsedLight = false;
	uint64_t m_iShadowLightRevision = {};
	// Zero permanently opts out if the monotonic revision ever overflows.
	uint64_t m_iStaticShadowRevision = 1u;
	// Immutable surface admission, with mutable morph/override checks at use.
	std::vector<uint32_t> m_StaticShadowCasterMeshes;
	bool_t m_bStaticShadowMaterialInputs = false;
	bool_t m_bVisibleInstancesDirty = true;
	// Prepared after all frame providers; do not advance reject grace twice.
	bool_t m_bFinalCameraPrepared = false;
	uint64_t m_iPreparedFrame = 0u;
	std::shared_ptr<MAP_STATIC_BATCH_FRAME_STATE> m_FrameState;
	bool_t m_bVisibleInstancesUsedCamera = false;
	uint64_t m_iVisibleCameraRevision = {};
    uint64_t m_iVisibilitySettingsRevision = 0u;
    float2_t m_VisibleViewportSize{};
    uint64_t m_DistanceRejectedInstances = 0u;
    bool m_bDistanceEligible = false, m_bOcclusionGeometryEligible = false;
    bool m_bVisibleOcclusionBounds = false;
    float3_t m_VisibleOcclusionMin{}, m_VisibleOcclusionMax{};
    std::vector<std::pair<uint32_t, bool>> m_CandidateDistanceChanges;
    uint64_t m_DistanceSourceIndices = 0u;
	f32_t m_fElapsedTime = {};
	//해당 batch에 소속된 전체 placement
	//vector - 메모리 연속, 순회가 빠름, index O(1), 프레임마다 전체 순회 좋음
	std::vector<FMapStaticInstance>	m_Instances;
    // Derived from each instance World; refreshed only when that transform changes.
    std::vector<f32_t> m_InstanceLinearScaleBounds;
	//이번 프레임에 실제 보이는 GPU용 인스턴스 배열
	std::vector<VTXMESHINSTANCE> m_VisibleInstances;
	// Reused staging storage; m_VisibleInstances remains the committed payload.
	std::vector<VTXMESHINSTANCE> m_CandidateVisibleInstances;
    std::vector<uint32_t> m_VisibleInstanceIndices, m_CandidateVisibleInstanceIndices;
    bool m_bTrackProxySources = false;
	uint32_t m_iAuthoredVisibleInstanceCount = {};
	/* Committed light-volume casters are independent from camera visibility. */
	std::vector<VTXMESHINSTANCE> m_ShadowInstances;
	std::vector<VTXMESHINSTANCE> m_CandidateShadowInstances;
	//placementId로 m_Instances의 index를 O(1)로 찾을 수 있게 한다.
	std::unordered_map<uint64_t, uint32_t>
		m_PlacementLookup;

	uint32_t m_iInstanceCapacity = {};
	uint32_t m_iShadowInstanceCapacity = {};
	// Adjacent draw coalescing never replaces an object's committed visible payload.
	uint32_t m_iLightingBankInstanceCapacity = {};
	std::vector<VTXMESHINSTANCE> m_LightingBankInstances;
	std::vector<VTXMESHINSTANCE> m_CandidateLightingBankInstances;
	// Prefix batch index -> representative RNM slot; independent of the eight SRVs.
	std::vector<uint8_t> m_CandidateLightingBankSlots;
	
	ComPtr<ID3D11Buffer> m_pInstanceBuffer = { nullptr };
	ComPtr<ID3D11Buffer> m_pShadowInstanceBuffer = { nullptr };
	ComPtr<ID3D11Buffer> m_pLightingBankInstanceBuffer = { nullptr };

	shared_ptr<Engine::CModel> m_pModelCom = { nullptr };
	shared_ptr<Engine::CShader> m_pShaderCom = { nullptr };

public:
	static unique_ptr<CMapStaticBatchObject> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

	virtual shared_ptr<CPrototype> Clone(
		void* pArg) override;
};

NS_END
