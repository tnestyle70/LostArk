#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "MapAssetCatalog.h"
#include "MapLoadScope.h"

#include <optional>
#include <vector>

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CMapAssetObject final : public CGameObject
{
public:
	/* Narrowly scoped runtime treatments for the masked Valtan proxy planes.
	   NONE remains the invariant for every ordinary map asset. */
	enum class PRESENTATION_VORTEX_PROFILE : uint32_t
	{
		NONE = 0u,
		DARK_APERTURE = 1u,
		RED_RING = 2u,
		RED_CLOUD_DISC = 3u,
		END
	};

	struct MAP_ASSET_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		uint32_t prototypeLevelIndex = ETOUI(LEVEL::DEVELOPMENT);
		uint64_t placementId = {};
		std::string assetId;
		std::string assetGroupId;
		std::wstring modelPrototypeTag;
		float3_t position = {};
		float4_t rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
		float3_t signedScale = float3_t(1.f, 1.f, 1.f);
		bool_t applyBottomCenter = false;
		bool_t visible = true;
		MAP_ASSET_RENDER_PROFILE renderProfile;
		Engine::MODEL_BAKED_LIGHTING_INSTANCE bakedLighting;
		/* Optional immutable material clone; geometry identity must match the prototype. */
		std::optional<Engine::MODEL_ASSET_LOAD_DESC> materialVariant;
		MAP_FRUSTUM_CULLING_POLICY frustumCulling{};
		/* Only set when the catalog resolved a water row for this asset. */
		bool_t hasWaterProfile = false;
		MAP_ASSET_WATER_PROFILE waterProfile;
	};

private:
	CMapAssetObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CMapAssetObject();

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Group(RENDERGROUP group) override;
	virtual int32_t Get_BlendSortPriority() const override;
	virtual HRESULT Render_Shadow() override;
	virtual bool_t Try_GetStaticShadowRevision(uint64_t& outRevision) const override;

	uint64_t Get_PlacementId() const { return m_iPlacementId; }
	const std::string& Get_AssetId() const { return m_AssetId; }
	const float3_t& Get_Position() const { return m_vPlacementPosition; }
	const float4_t& Get_RotationQuaternion() const { return m_vRotationQuaternion; }
	const float3_t& Get_SignedScale() const { return m_vSignedScale; }
	bool_t Is_Visible() const { return m_bVisible; }
	bool_t Is_Mirrored() const { return m_bMirrored; }
	void Set_PlacementTransform(const float3_t& position,
		const float4_t& rotationQuaternion, const float3_t& signedScale);
	void Set_Visible(bool_t visible) { m_bVisible = visible; }
	/* Overlay that hides the object without touching the logical visibility that
	   gameplay and Sequences own. A cinematic stage suppresses the areas around it
	   through this flag and clears it when the cinematic ends. */
	bool_t Is_StageSuppressed() const { return m_bStageSuppressed; }
	void Set_StageSuppressed(bool_t suppressed) { m_bStageSuppressed = suppressed; }
	/* Map Tool's camera inspection overlay is independent from a cutscene
	   stage overlay, so restoring the inspection view never revives an object
	   a sequence intentionally keeps hidden. */
	bool_t Is_CameraPreviewSuppressed() const { return m_bCameraPreviewSuppressed; }
	void Set_CameraPreviewSuppressed(bool_t suppressed)
	{ m_bCameraPreviewSuppressed = suppressed; }
	void Set_PresentationOpacityMultiplier(f32_t multiplier);
	void Set_PresentationVortexProfile(
		PRESENTATION_VORTEX_PROFILE profile,
		f32_t strength);

private:
	uint64_t m_iPlacementId = {};
	std::string m_AssetId;
	std::string m_AssetGroupId;
	float3_t m_vPlacementPosition = {};
	float4_t m_vRotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
	float3_t m_vSignedScale = float3_t(1.f, 1.f, 1.f);

	bool_t m_bApplyBottomCenter = false;
	bool_t m_bVisible = true;
	bool_t m_bStageSuppressed = false;
	bool_t m_bCameraPreviewSuppressed = false;
	bool_t Is_Rendered() const
	{ return m_bVisible && !m_bStageSuppressed && !m_bCameraPreviewSuppressed; }
	bool_t m_bMirrored = false;
	//Frustum Culling을 위한 멤버 변수 추가 
	bool_t m_bHasLocalCullBounds = false;
	bool_t m_bHasWorldCullBounds = false;
	float3_t m_vLocalCullCenter = {};
	f32_t m_fLocalCullRadius = {};
	float3_t m_vWorldCullCenter = {};
	f32_t m_fWorldCullRadius = {};
	MAP_FRUSTUM_CULLING_POLICY m_FrustumCulling{};
	MAP_FRUSTUM_RUNTIME_STATE m_FrustumState{};

	MAP_ASSET_RENDER_PROFILE m_RenderProfile;
	Engine::MODEL_BAKED_LIGHTING_INSTANCE m_BakedLighting;
	bool_t m_bHasWaterProfile = false;
	MAP_ASSET_WATER_PROFILE m_WaterProfile;
	/* Runtime presentation may fade a placement without mutating the authored
	   catalog profile shared by every occurrence of the asset. */
	f32_t m_fPresentationOpacityMultiplier = 1.f;
	PRESENTATION_VORTEX_PROFILE m_ePresentationVortexProfile =
		PRESENTATION_VORTEX_PROFILE::NONE;
	f32_t m_fPresentationVortexStrength = 0.f;
	f32_t m_fElapsedTime = {};

	/* Cache immutable geometry with the existing opaque/alpha shadow pass
	   only when its inputs are independent of time and camera. Compare the
	   actual Transform, bounds and draw selection rather than a state hash. */
	struct STATIC_SHADOW_SNAPSHOT
	{
		const CModel* model = nullptr;
		float4x4_t world{};
		float3_t worldCullCenter{};
		f32_t worldCullRadius = 0.f;
		f32_t presentationOpacity = 1.f;
		bool_t visible = false;
		bool_t mirrored = false;
		bool_t hasWorldCullBounds = false;
		bool_t hasVertexDisplacement = false;
	};
	mutable STATIC_SHADOW_SNAPSHOT m_StaticShadowSnapshot{};
	mutable std::vector<uint32_t> m_StaticShadowMeshPasses;
	mutable std::vector<uint32_t> m_StaticShadowCandidateMeshPasses;
	mutable uint64_t m_iStaticShadowRevision = 0u;
	mutable bool_t m_bStaticShadowSnapshotValid = false;

	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };

private:
	MAP_ASSET_RENDER_PROFILE Get_MaterialRenderProfile(uint32_t meshIndex) const;
	HRESULT Ready_Components(uint32_t prototypeLevelIndex,
		const std::wstring& modelPrototypeTag,
		const std::optional<Engine::MODEL_ASSET_LOAD_DESC>& materialVariant);
	HRESULT Bind_ShaderResources(
		const struct MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot);
	HRESULT Bind_ShadowShaderResources();
	HRESULT Bind_PresentationVortexShaderResources(
		PRESENTATION_VORTEX_PROFILE profile,
		f32_t strength);
	HRESULT Reset_PresentationVortexShaderResources();
	/* Pushes the authored water parameters and clears them again, so an
	   ordinary asset drawn later through the same shared FX11 effect cannot
	   inherit another placement's water values. */
	HRESULT Bind_WaterShaderResources(bool_t bEnabled);
	//Frustum Culling
	void Ready_CullBounds();
	void Update_WorldCullBounds();

	float3_t Compute_WorldOrigin(const float3_t& placementPosition,
		const float4_t& rotationQuaternion, const float3_t& signedScale) const;
	MAP_ASSET_RENDER_PROFILE Get_EffectiveRenderProfile() const;

public:
	static unique_ptr<CMapAssetObject> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
