#pragma once

#include "Client_Defines.h"
#include "DeployPropCatalog.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CDeployPropObject final : public CGameObject
{
public:
	struct DEPLOY_PROP_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		uint32_t prototypeLevelIndex = ETOUI(LEVEL::END);
		DEPLOY_PROP_PLACEMENT placement;
		DEPLOY_PROP_MODEL_KIND modelKind = DEPLOY_PROP_MODEL_KIND::STATIC;
		std::wstring intactPrototypeTag;
		std::wstring fracturedPrototypeTag;
	};

	/* A debris preview instance names a CModel prototype already admitted by
	   MapTool and the additional scale applied after that prototype's fixed
	   asset pretransform. Runtime owns the deterministic choice/scale; this
	   object only clones and presents it. */
	struct DEBRIS_PREVIEW_INSTANCE_DESC
	{
		std::wstring modelPrototypeTag;
		f32_t uniformScale = 1.f;
	};

	struct DEBRIS_PREVIEW_DESC
	{
		std::vector<DEBRIS_PREVIEW_INSTANCE_DESC> instances;
		/* false keeps the source fractured Deploy presentation visible while
		   independent rigid bodies drive the flying debris. */
		bool_t suppressSource = false;
	};

private:
	CDeployPropObject(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CDeployPropObject();
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	bool_t Set_State(DEPLOY_PROP_STATE state);
	DEPLOY_PROP_STATE Get_State() const { return m_State; }
	uint64_t Get_RuntimePlacementId() const { return m_Placement.runtimePlacementId; }
	uint32_t Get_DeployActorId() const { return m_Placement.deployActorId; }
	bool_t Is_Destructible() const { return m_Placement.destructible; }
	bool_t Is_AnimBindPoseOnly() const;
	/* World axis-aligned bounds of the model this prop currently renders.
	   Authoring tools use it to hit-test and outline a prop instead of
	   re-cloning a prototype by tag. Returns false when the model carries no
	   baked local bounds so the caller can skip the prop instead of using a
	   zero-sized box. */
	bool_t Get_WorldBounds(
		float3_t& outCenter,
		float3_t& outHalfExtents) const;
	/* Local box used when the object enters its fractured preview state.
	   Values include placement uniform scale but not root rotation/translation. */
	bool_t Get_PhysicsPreviewLocalBounds(
		float3_t& outCenter,
		float3_t& outHalfExtents) const;
	/* MapTool destruction audition keeps the existing Deploy object and model
	   alive, then lets a presentation-only rigid body drive its root pose.
	   This is deliberately separate from the authored placement transform and
	   from the persistent destroyable state. End_PhysicsPreview restores both
	   values exactly, so scrubbing/restarting never dirties map data. */
	bool_t Begin_PhysicsPreview(DEPLOY_PROP_STATE previewState);
	bool_t Apply_PhysicsPreviewPose(
		const float3_t& position,
		const float4_t& rotationQuaternion);
	/* Advances an animated fractured model on the destruction preview's fixed
	   clock. Regular Update deliberately stops advancing it while the preview
	   seam is active, so seek/restart and PhysX always sample the same tick. */
	bool_t Advance_PhysicsPreviewAnimation(f32_t fixedDeltaSeconds);
	void End_PhysicsPreview();
	bool_t Is_PhysicsPreviewActive() const
	{
		return m_bPhysicsPreviewActive;
	}
	/* Stages every referenced static CModel before committing any preview
	   state. Instances begin hidden and do not mutate the Deploy state or root
	   transform; Begin_PhysicsPreview(FRACTURED) remains the runtime's separate
	   source-state seam. */
	bool_t Begin_DebrisPreview(
		const DEBRIS_PREVIEW_DESC& desc,
		std::string& outError);
	uint32_t Get_DebrisPreviewInstanceCount() const;
	/* Model-local box after the admitted prototype's asset pretransform and
	   the authored per-instance uniform scale. Runtime uses its center as the
	   rigid-body local shape offset. */
	bool_t Get_DebrisPreviewLocalBounds(
		uint32_t instanceIndex,
		float3_t& outCenter,
		float3_t& outHalfExtents) const;
	/* position/rotation describe the model root in world space. Visibility is
	   explicit so staging and deterministic seek never flash at the origin. */
	bool_t Apply_DebrisPreviewPose(
		uint32_t instanceIndex,
		const float3_t& position,
		const float4_t& rotationQuaternion,
		bool_t visible);
	void End_DebrisPreview();
	bool_t Is_DebrisPreviewActive() const
	{
		return m_bDebrisPreviewActive;
	}

private:
	struct DEBRIS_PREVIEW_RESOURCE
	{
		std::wstring modelPrototypeTag;
		shared_ptr<CModel> model;
	};

	struct DEBRIS_PREVIEW_INSTANCE
	{
		uint32_t resourceIndex = UINT32_MAX;
		f32_t uniformScale = 1.f;
		float3_t position = {};
		float4_t rotation = float4_t(0.f, 0.f, 0.f, 1.f);
		bool_t visible = false;
	};

	HRESULT Ready_Components(const DEPLOY_PROP_DESC& desc);
	HRESULT Bind_CommonShaderResources();
	HRESULT Bind_DebrisShaderResources(const float4x4_t& worldMatrix);
	HRESULT Render_Static(
		const shared_ptr<CModel>& model,
		const shared_ptr<CShader>& shader);
	HRESULT Render_Animated();
	HRESULT Render_DebrisPreview();
	bool_t Has_VisibleDebrisPreviewInstance() const;
	void Apply_Transform();

private:
	DEPLOY_PROP_PLACEMENT m_Placement;
	DEPLOY_PROP_MODEL_KIND m_ModelKind = DEPLOY_PROP_MODEL_KIND::STATIC;
	DEPLOY_PROP_STATE m_State = DEPLOY_PROP_STATE::INTACT;
	DEPLOY_PROP_STATE m_PrePhysicsPreviewState = DEPLOY_PROP_STATE::INTACT;
	uint32_t m_iPrePhysicsPreviewAnimationIndex = UINT32_MAX;
	f32_t m_fPrePhysicsPreviewAnimationTrackPosition = 0.f;
	bool_t m_bPhysicsPreviewActive = false;
	float3_t m_PhysicsPreviewPosition = {};
	float4_t m_PhysicsPreviewRotation = float4_t(0.f, 0.f, 0.f, 1.f);
	uint32_t m_iPrototypeLevelIndex = ETOUI(LEVEL::END);
	bool_t m_bDebrisPreviewActive = false;
	bool_t m_bDebrisSuppressSource = false;
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CShader> m_pDebrisShaderCom = { nullptr };
	shared_ptr<CModel> m_pIntactModelCom = { nullptr };
	shared_ptr<CModel> m_pFracturedModelCom = { nullptr };
	std::vector<DEBRIS_PREVIEW_RESOURCE> m_DebrisPreviewResources;
	std::vector<DEBRIS_PREVIEW_INSTANCE> m_DebrisPreviewInstances;

public:
	static unique_ptr<CDeployPropObject> Create(
		ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
