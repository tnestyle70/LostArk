#include "DeployPropObject.h"

#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <string_view>
#include <unordered_map>

namespace
{
	bool_t Matches_LogicalAnimationName(
		const char_t* pStoredName,
		std::string_view logicalName)
	{
		if (nullptr == pStoredName || logicalName.empty())
			return false;

		const std::string_view storedName(pStoredName);
		if (storedName == logicalName)
			return true;
		if (storedName.size() <= logicalName.size())
			return false;

		const size_t suffixOffset = storedName.size() - logicalName.size();
		if (0 != storedName.compare(suffixOffset, logicalName.size(), logicalName))
			return false;

		const char_t separator = storedName[suffixOffset - 1];
		return '_' == separator || '.' == separator ||
			'|' == separator || ' ' == separator;
	}

	bool_t Apply_LogicalAnimation(
		const shared_ptr<CModel>& pModel,
		std::string_view logicalName)
	{
		if (nullptr == pModel)
			return false;

		const uint32_t animationCount = pModel->Get_NumAnimations();
		uint32_t resolvedIndex = animationCount;
		for (uint32_t index = 0; index < animationCount; ++index)
		{
			if (!Matches_LogicalAnimationName(
				pModel->Get_AnimationName(index), logicalName))
				continue;

			if (resolvedIndex != animationCount)
				return false;
			resolvedIndex = index;
		}

		if (resolvedIndex == animationCount)
			return false;
		pModel->Set_Animation(resolvedIndex, false);
		if (!pModel->Set_AnimTrackPosition(resolvedIndex, 0.f))
			return false;
		pModel->Play_Animation(0.f);
		return true;
	}
}

CDeployPropObject::CDeployPropObject(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CDeployPropObject::~CDeployPropObject() = default;

HRESULT CDeployPropObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDeployPropObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;
	const DEPLOY_PROP_DESC desc = *static_cast<DEPLOY_PROP_DESC*>(pArg);
	if (desc.prototypeLevelIndex >= ETOUI(LEVEL::END) ||
		0 == desc.placement.runtimePlacementId ||
		desc.placement.assetId.empty() || desc.intactPrototypeTag.empty() ||
		!std::isfinite(desc.placement.uniformScale) ||
		desc.placement.uniformScale <= 0.000001f ||
		(desc.modelKind == DEPLOY_PROP_MODEL_KIND::STATIC &&
			desc.fracturedPrototypeTag.empty()))
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components(desc)))
		return E_FAIL;
	m_Placement = desc.placement;
	m_ModelKind = desc.modelKind;
	m_iPrototypeLevelIndex = desc.prototypeLevelIndex;
	Apply_Transform();
	if (m_ModelKind == DEPLOY_PROP_MODEL_KIND::ANIM &&
		m_pIntactModelCom->Get_NumAnimations() > 0 &&
		!Apply_LogicalAnimation(m_pIntactModelCom, "on"))
		return E_FAIL;
	return S_OK;
}

void CDeployPropObject::Update(f32_t fTimeDelta)
{
	if (!m_bPhysicsPreviewActive &&
		m_State == DEPLOY_PROP_STATE::FRACTURED &&
		m_ModelKind == DEPLOY_PROP_MODEL_KIND::ANIM &&
		m_pIntactModelCom->Get_NumAnimations() > 0)
		m_pIntactModelCom->Play_Animation(fTimeDelta);
}

void CDeployPropObject::Late_Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	const bool_t sourceVisible =
		m_State != DEPLOY_PROP_STATE::DESPAWNED &&
		!Is_BasePresentationSuppressed();
	if (!sourceVisible && !Has_VisibleDebrisPreviewInstance())
		return;
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
	if (CGameInstance::Get().Is_ShadowLightEnabled())
	{
		CGameInstance::Get().Add_RenderObject(
			RENDERGROUP::SHADOW,
			static_pointer_cast<CGameObject>(shared_from_this()));
	}
}

HRESULT CDeployPropObject::Render()
{
	const bool_t sourceVisible =
		m_State != DEPLOY_PROP_STATE::DESPAWNED &&
		!Is_BasePresentationSuppressed();
	if (sourceVisible)
	{
		if (FAILED(Bind_CommonShaderResources()))
			return E_FAIL;
		if (m_ModelKind == DEPLOY_PROP_MODEL_KIND::ANIM)
		{
			if (FAILED(Render_Animated(0u)))
				return E_FAIL;
		}
		else if (FAILED(Render_Static(
			m_State == DEPLOY_PROP_STATE::FRACTURED ?
			m_pFracturedModelCom : m_pIntactModelCom,
			m_pShaderCom, 0u)))
		{
			return E_FAIL;
		}
	}

	return Has_VisibleDebrisPreviewInstance() ?
		Render_DebrisPreview(false) : S_OK;
}

HRESULT CDeployPropObject::Render_Shadow()
{
	constexpr uint32_t ANIMATED_SHADOW_PASS = 1u;
	constexpr uint32_t STATIC_SHADOW_PASS = 12u;
	const bool_t sourceVisible =
		m_State != DEPLOY_PROP_STATE::DESPAWNED &&
		!Is_BasePresentationSuppressed();
	if (sourceVisible)
	{
		if (FAILED(Bind_ShadowShaderResources(
			m_pShaderCom, *m_pTransformCom->Get_WorldMatrixPtr())))
		{
			return E_FAIL;
		}
		if (m_ModelKind == DEPLOY_PROP_MODEL_KIND::ANIM)
		{
			if (FAILED(Render_Animated(ANIMATED_SHADOW_PASS)))
				return E_FAIL;
		}
		else if (FAILED(Render_Static(
			m_State == DEPLOY_PROP_STATE::FRACTURED ?
			m_pFracturedModelCom : m_pIntactModelCom,
			m_pShaderCom, STATIC_SHADOW_PASS)))
		{
			return E_FAIL;
		}
	}

	return Has_VisibleDebrisPreviewInstance() ?
		Render_DebrisPreview(true) : S_OK;
}

bool_t CDeployPropObject::Set_State(DEPLOY_PROP_STATE state)
{
	if (state != DEPLOY_PROP_STATE::INTACT &&
		state != DEPLOY_PROP_STATE::FRACTURED &&
		state != DEPLOY_PROP_STATE::DESPAWNED)
	{
		return false;
	}
	if (m_State == state)
		return true;

	if (m_ModelKind == DEPLOY_PROP_MODEL_KIND::ANIM &&
		m_pIntactModelCom->Get_NumAnimations() > 0)
	{
		if (state == DEPLOY_PROP_STATE::INTACT &&
			!Apply_LogicalAnimation(m_pIntactModelCom, "on"))
			return false;
		if (state == DEPLOY_PROP_STATE::FRACTURED &&
			!Apply_LogicalAnimation(m_pIntactModelCom, "off"))
			return false;
	}
	m_State = state;
	return true;
}

bool_t CDeployPropObject::Is_AnimBindPoseOnly() const
{
	return m_ModelKind == DEPLOY_PROP_MODEL_KIND::ANIM &&
		m_pIntactModelCom->Get_NumAnimations() == 0;
}

bool_t CDeployPropObject::Get_WorldBounds(
	float3_t& outCenter,
	float3_t& outHalfExtents) const
{
	const shared_ptr<CModel>& model =
		(DEPLOY_PROP_STATE::FRACTURED == m_State &&
			nullptr != m_pFracturedModelCom) ?
		m_pFracturedModelCom : m_pIntactModelCom;
	if (nullptr == model || !model->Has_LocalBounds())
		return false;

	const float3_t& localMinimum = model->Get_LocalBoundsMin();
	const float3_t& localMaximum = model->Get_LocalBoundsMax();
	/* Same scale and rotation Apply_Transform builds, without the translation,
	   so the eight rotated corners can be re-bounded around the placement. */
	const float4_t& rootRotation = m_bPhysicsPreviewActive ?
		m_PhysicsPreviewRotation : m_Placement.rotationQuaternion;
	const float3_t& rootPosition = m_bPhysicsPreviewActive ?
		m_PhysicsPreviewPosition : m_Placement.position;
	const vector_t quaternion =
		XMQuaternionNormalize(XMLoadFloat4(&rootRotation));
	const matrix_t rotation = XMMatrixScaling(
		m_Placement.uniformScale,
		m_Placement.uniformScale,
		m_Placement.uniformScale) * XMMatrixRotationQuaternion(quaternion);

	float3_t minimum(FLT_MAX, FLT_MAX, FLT_MAX);
	float3_t maximum(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (uint32_t corner = 0u; corner < 8u; ++corner)
	{
		float3_t rotated{};
		XMStoreFloat3(&rotated, XMVector3Transform(
			XMVectorSet(
				0u != (corner & 1u) ? localMaximum.x : localMinimum.x,
				0u != (corner & 2u) ? localMaximum.y : localMinimum.y,
				0u != (corner & 4u) ? localMaximum.z : localMinimum.z,
				1.f),
			rotation));
		if (!std::isfinite(rotated.x) || !std::isfinite(rotated.y) ||
			!std::isfinite(rotated.z))
		{
			return false;
		}
		minimum.x = (std::min)(minimum.x, rotated.x);
		minimum.y = (std::min)(minimum.y, rotated.y);
		minimum.z = (std::min)(minimum.z, rotated.z);
		maximum.x = (std::max)(maximum.x, rotated.x);
		maximum.y = (std::max)(maximum.y, rotated.y);
		maximum.z = (std::max)(maximum.z, rotated.z);
	}

	outHalfExtents = float3_t(
		(maximum.x - minimum.x) * 0.5f,
		(maximum.y - minimum.y) * 0.5f,
		(maximum.z - minimum.z) * 0.5f);
	outCenter = float3_t(
		rootPosition.x + (minimum.x + maximum.x) * 0.5f,
		rootPosition.y + (minimum.y + maximum.y) * 0.5f,
		rootPosition.z + (minimum.z + maximum.z) * 0.5f);
	return outHalfExtents.x > 0.f && outHalfExtents.y > 0.f &&
		outHalfExtents.z > 0.f;
}

bool_t CDeployPropObject::Get_PhysicsPreviewLocalBounds(
	float3_t& outCenter,
	float3_t& outHalfExtents) const
{
	const shared_ptr<CModel>& model =
		(m_ModelKind == DEPLOY_PROP_MODEL_KIND::STATIC &&
			nullptr != m_pFracturedModelCom) ?
		m_pFracturedModelCom : m_pIntactModelCom;
	if (nullptr == model || !model->Has_LocalBounds() ||
		!std::isfinite(m_Placement.uniformScale) ||
		m_Placement.uniformScale <= 0.f)
	{
		return false;
	}

	const float3_t& minimum = model->Get_LocalBoundsMin();
	const float3_t& maximum = model->Get_LocalBoundsMax();
	outCenter = float3_t(
		(minimum.x + maximum.x) * 0.5f * m_Placement.uniformScale,
		(minimum.y + maximum.y) * 0.5f * m_Placement.uniformScale,
		(minimum.z + maximum.z) * 0.5f * m_Placement.uniformScale);
	outHalfExtents = float3_t(
		(maximum.x - minimum.x) * 0.5f * m_Placement.uniformScale,
		(maximum.y - minimum.y) * 0.5f * m_Placement.uniformScale,
		(maximum.z - minimum.z) * 0.5f * m_Placement.uniformScale);
	return std::isfinite(outCenter.x) && std::isfinite(outCenter.y) &&
		std::isfinite(outCenter.z) && std::isfinite(outHalfExtents.x) &&
		std::isfinite(outHalfExtents.y) && std::isfinite(outHalfExtents.z) &&
		outHalfExtents.x > 0.f && outHalfExtents.y > 0.f &&
		outHalfExtents.z > 0.f;
}

bool_t CDeployPropObject::Begin_PhysicsPreview(
	const DEPLOY_PROP_STATE previewState)
{
	if (m_bPhysicsPreviewActive ||
		(previewState != DEPLOY_PROP_STATE::INTACT &&
			previewState != DEPLOY_PROP_STATE::FRACTURED))
	{
		return false;
	}

	const DEPLOY_PROP_STATE previousState = m_State;
	uint32_t previousAnimationIndex = UINT32_MAX;
	f32_t previousAnimationPosition = 0.f;
	f32_t previousAnimationDuration = 0.f;
	if (m_ModelKind == DEPLOY_PROP_MODEL_KIND::ANIM &&
		m_pIntactModelCom->Get_NumAnimations() > 0u)
	{
		previousAnimationIndex = m_pIntactModelCom->Get_CurrentAnimIndex();
		if (!m_pIntactModelCom->Get_AnimationProgress(
			previousAnimationIndex, previousAnimationPosition,
			previousAnimationDuration))
		{
			return false;
		}
	}
	if (!Set_State(previewState))
		return false;
	/* Set_State intentionally treats an equal persistent state as a no-op.
	   Preview restart is a one-shot event, so explicitly rewind the fractured
	   logical clip even when authoring already left the prop FRACTURED. */
	if (previewState == DEPLOY_PROP_STATE::FRACTURED &&
		m_ModelKind == DEPLOY_PROP_MODEL_KIND::ANIM &&
		m_pIntactModelCom->Get_NumAnimations() > 0u &&
		!Apply_LogicalAnimation(m_pIntactModelCom, "off"))
	{
		Set_State(previousState);
		if (UINT32_MAX != previousAnimationIndex)
		{
			m_pIntactModelCom->Set_Animation(previousAnimationIndex, false);
			m_pIntactModelCom->Set_AnimTrackPosition(
				previousAnimationIndex, previousAnimationPosition);
			m_pIntactModelCom->Play_Animation(0.f);
		}
		return false;
	}

	m_PrePhysicsPreviewState = previousState;
	m_iPrePhysicsPreviewAnimationIndex = previousAnimationIndex;
	m_fPrePhysicsPreviewAnimationTrackPosition = previousAnimationPosition;
	m_PhysicsPreviewPosition = m_Placement.position;
	m_PhysicsPreviewRotation = m_Placement.rotationQuaternion;
	m_bPhysicsPreviewActive = true;
	Apply_Transform();
	return true;
}

bool_t CDeployPropObject::Apply_PhysicsPreviewPose(
	const float3_t& position,
	const float4_t& rotationQuaternion)
{
	if (!m_bPhysicsPreviewActive ||
		!std::isfinite(position.x) || !std::isfinite(position.y) ||
		!std::isfinite(position.z) ||
		!std::isfinite(rotationQuaternion.x) ||
		!std::isfinite(rotationQuaternion.y) ||
		!std::isfinite(rotationQuaternion.z) ||
		!std::isfinite(rotationQuaternion.w))
	{
		return false;
	}

	const vector_t rotation = XMLoadFloat4(&rotationQuaternion);
	const f32_t lengthSquared = XMVectorGetX(XMVector4LengthSq(rotation));
	if (!std::isfinite(lengthSquared) || lengthSquared <= 0.000001f)
		return false;

	m_PhysicsPreviewPosition = position;
	XMStoreFloat4(
		&m_PhysicsPreviewRotation,
		XMQuaternionNormalize(rotation));
	Apply_Transform();
	return true;
}

bool_t CDeployPropObject::Advance_PhysicsPreviewAnimation(
	const f32_t fixedDeltaSeconds)
{
	if (!m_bPhysicsPreviewActive ||
		!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
	{
		return false;
	}
	if (m_ModelKind != DEPLOY_PROP_MODEL_KIND::ANIM ||
		0u == m_pIntactModelCom->Get_NumAnimations())
	{
		return true;
	}
	if (m_State != DEPLOY_PROP_STATE::FRACTURED)
		return false;

	m_pIntactModelCom->Play_Animation(fixedDeltaSeconds);
	return true;
}

void CDeployPropObject::End_PhysicsPreview()
{
	if (!m_bPhysicsPreviewActive)
		return;

	m_bPhysicsPreviewActive = false;
	m_PhysicsPreviewPosition = m_Placement.position;
	m_PhysicsPreviewRotation = m_Placement.rotationQuaternion;
	Set_State(m_PrePhysicsPreviewState);
	if (UINT32_MAX != m_iPrePhysicsPreviewAnimationIndex &&
		m_iPrePhysicsPreviewAnimationIndex <
			m_pIntactModelCom->Get_NumAnimations())
	{
		m_pIntactModelCom->Set_Animation(
			m_iPrePhysicsPreviewAnimationIndex, false);
		m_pIntactModelCom->Set_AnimTrackPosition(
			m_iPrePhysicsPreviewAnimationIndex,
			m_fPrePhysicsPreviewAnimationTrackPosition);
		m_pIntactModelCom->Play_Animation(0.f);
	}
	m_iPrePhysicsPreviewAnimationIndex = UINT32_MAX;
	m_fPrePhysicsPreviewAnimationTrackPosition = 0.f;
	Apply_Transform();
}

bool_t CDeployPropObject::Begin_DebrisPreview(
	const DEBRIS_PREVIEW_DESC& desc,
	std::string& outError)
{
	return Begin_DebrisPresentation(
		desc.instances,
		desc.suppressSource,
		DEBRIS_PRESENTATION_OWNER::MAPTOOL_PREVIEW,
		outError);
}

bool_t CDeployPropObject::Begin_DebrisPresentation(
	const std::vector<DEBRIS_PREVIEW_INSTANCE_DESC>& instances,
	const bool_t suppressSource,
	const DEBRIS_PRESENTATION_OWNER owner,
	std::string& outError)
{
	outError.clear();
	if (DEBRIS_PRESENTATION_OWNER::NONE == owner ||
		m_bDebrisPreviewActive)
	{
		outError = "A transient debris presentation is already active";
		return false;
	}
	if (instances.empty())
	{
		outError = "Transient debris requires at least one instance";
		return false;
	}
	if (m_iPrototypeLevelIndex >= ETOUI(LEVEL::END))
	{
		outError = "Debris preview shader level is invalid";
		return false;
	}

	shared_ptr<CShader> stagedShader = dynamic_pointer_cast<CShader>(
		CGameInstance::Get().Clone_Prototype(
			m_iPrototypeLevelIndex,
			TEXT("Prototype_Component_Shader_VtxMeshBinary")));
	if (nullptr == stagedShader)
	{
		outError = "Debris preview static shader clone failed";
		return false;
	}

	std::vector<DEBRIS_PREVIEW_RESOURCE> stagedResources;
	std::vector<DEBRIS_PREVIEW_INSTANCE> stagedInstances;
	stagedInstances.reserve(instances.size());
	std::unordered_map<std::wstring, uint32_t> resourceLookup;
	resourceLookup.reserve(instances.size());

	for (const DEBRIS_PREVIEW_INSTANCE_DESC& instanceDesc : instances)
	{
		if (instanceDesc.modelPrototypeTag.empty() ||
			!std::isfinite(instanceDesc.uniformScale) ||
			instanceDesc.uniformScale <= 0.000001f)
		{
			outError = "Debris preview instance prototype/scale is invalid";
			return false;
		}

		uint32_t resourceIndex = UINT32_MAX;
		const auto existing = resourceLookup.find(
			instanceDesc.modelPrototypeTag);
		if (existing != resourceLookup.end())
		{
			resourceIndex = existing->second;
		}
		else
		{
			shared_ptr<CModel> clonedModel = dynamic_pointer_cast<CModel>(
				CGameInstance::Get().Clone_Prototype(
					m_iPrototypeLevelIndex,
					instanceDesc.modelPrototypeTag));
			if (nullptr == clonedModel || clonedModel->Is_Skinned() ||
				!clonedModel->Has_LocalBounds())
			{
				outError =
					"Debris preview admitted CModel/bounds clone failed";
				return false;
			}

			resourceIndex = static_cast<uint32_t>(stagedResources.size());
			DEBRIS_PREVIEW_RESOURCE resource{};
			resource.modelPrototypeTag = instanceDesc.modelPrototypeTag;
			resource.model = std::move(clonedModel);
			stagedResources.push_back(std::move(resource));
			resourceLookup.emplace(
				instanceDesc.modelPrototypeTag, resourceIndex);
		}

		DEBRIS_PREVIEW_INSTANCE instance{};
		instance.resourceIndex = resourceIndex;
		instance.uniformScale = instanceDesc.uniformScale;
		stagedInstances.push_back(instance);
	}

	/* Commit only after every shader/model/bounds validation succeeds. */
	m_pDebrisShaderCom = std::move(stagedShader);
	m_DebrisPreviewResources = std::move(stagedResources);
	m_DebrisPreviewInstances = std::move(stagedInstances);
	m_bDebrisSuppressSource = suppressSource;
	m_eDebrisPresentationOwner = owner;
	m_bDebrisPreviewActive = true;
	return true;
}

uint32_t CDeployPropObject::Get_DebrisPreviewInstanceCount() const
{
	return Get_DebrisPresentationInstanceCount(
		DEBRIS_PRESENTATION_OWNER::MAPTOOL_PREVIEW);
}

uint32_t CDeployPropObject::Get_DebrisPresentationInstanceCount(
	const DEBRIS_PRESENTATION_OWNER owner) const
{
	return m_bDebrisPreviewActive && owner == m_eDebrisPresentationOwner ?
		static_cast<uint32_t>(m_DebrisPreviewInstances.size()) : 0u;
}

bool_t CDeployPropObject::Get_DebrisPreviewLocalBounds(
	const uint32_t instanceIndex,
	float3_t& outCenter,
	float3_t& outHalfExtents) const
{
	return Get_DebrisPresentationLocalBounds(
		DEBRIS_PRESENTATION_OWNER::MAPTOOL_PREVIEW,
		instanceIndex, outCenter, outHalfExtents);
}

bool_t CDeployPropObject::Get_DebrisPresentationLocalBounds(
	const DEBRIS_PRESENTATION_OWNER owner,
	const uint32_t instanceIndex,
	float3_t& outCenter,
	float3_t& outHalfExtents) const
{
	if (!m_bDebrisPreviewActive || owner != m_eDebrisPresentationOwner ||
		instanceIndex >= m_DebrisPreviewInstances.size())
	{
		return false;
	}

	const DEBRIS_PREVIEW_INSTANCE& instance =
		m_DebrisPreviewInstances[instanceIndex];
	if (instance.resourceIndex >= m_DebrisPreviewResources.size())
		return false;
	const shared_ptr<CModel>& model =
		m_DebrisPreviewResources[instance.resourceIndex].model;
	if (nullptr == model || !model->Has_LocalBounds())
		return false;

	const float3_t& minimum = model->Get_LocalBoundsMin();
	const float3_t& maximum = model->Get_LocalBoundsMax();
	outCenter = float3_t(
		(minimum.x + maximum.x) * 0.5f * instance.uniformScale,
		(minimum.y + maximum.y) * 0.5f * instance.uniformScale,
		(minimum.z + maximum.z) * 0.5f * instance.uniformScale);
	outHalfExtents = float3_t(
		(maximum.x - minimum.x) * 0.5f * instance.uniformScale,
		(maximum.y - minimum.y) * 0.5f * instance.uniformScale,
		(maximum.z - minimum.z) * 0.5f * instance.uniformScale);
	return std::isfinite(outCenter.x) && std::isfinite(outCenter.y) &&
		std::isfinite(outCenter.z) && std::isfinite(outHalfExtents.x) &&
		std::isfinite(outHalfExtents.y) && std::isfinite(outHalfExtents.z) &&
		outHalfExtents.x > 0.f && outHalfExtents.y > 0.f &&
		outHalfExtents.z > 0.f;
}

bool_t CDeployPropObject::Apply_DebrisPreviewPose(
	const uint32_t instanceIndex,
	const float3_t& position,
	const float4_t& rotationQuaternion,
	const bool_t visible)
{
	return Apply_DebrisPresentationPose(
		DEBRIS_PRESENTATION_OWNER::MAPTOOL_PREVIEW,
		instanceIndex, position, rotationQuaternion, visible);
}

bool_t CDeployPropObject::Apply_DebrisPresentationPose(
	const DEBRIS_PRESENTATION_OWNER owner,
	const uint32_t instanceIndex,
	const float3_t& position,
	const float4_t& rotationQuaternion,
	const bool_t visible)
{
	if (!m_bDebrisPreviewActive || owner != m_eDebrisPresentationOwner ||
		instanceIndex >= m_DebrisPreviewInstances.size() ||
		!std::isfinite(position.x) || !std::isfinite(position.y) ||
		!std::isfinite(position.z) ||
		!std::isfinite(rotationQuaternion.x) ||
		!std::isfinite(rotationQuaternion.y) ||
		!std::isfinite(rotationQuaternion.z) ||
		!std::isfinite(rotationQuaternion.w))
	{
		return false;
	}

	const vector_t rotation = XMLoadFloat4(&rotationQuaternion);
	const f32_t lengthSquared = XMVectorGetX(XMVector4LengthSq(rotation));
	if (!std::isfinite(lengthSquared) || lengthSquared <= 0.000001f)
		return false;

	DEBRIS_PREVIEW_INSTANCE& instance =
		m_DebrisPreviewInstances[instanceIndex];
	instance.position = position;
	XMStoreFloat4(&instance.rotation, XMQuaternionNormalize(rotation));
	instance.visible = visible;
	return true;
}

void CDeployPropObject::End_DebrisPreview()
{
	End_DebrisPresentation(DEBRIS_PRESENTATION_OWNER::MAPTOOL_PREVIEW);
}

void CDeployPropObject::End_DebrisPresentation(
	const DEBRIS_PRESENTATION_OWNER owner)
{
	if (!m_bDebrisPreviewActive || owner != m_eDebrisPresentationOwner)
		return;

	m_bDebrisPreviewActive = false;
	m_bDebrisSuppressSource = false;
	m_eDebrisPresentationOwner = DEBRIS_PRESENTATION_OWNER::NONE;
	m_DebrisPreviewInstances.clear();
	m_DebrisPreviewResources.clear();
	m_pDebrisShaderCom.reset();
}

bool_t CDeployPropObject::Begin_DestructionDebrisPresentation(
	const DESTRUCTION_DEBRIS_PRESENTATION_DESC& desc,
	std::string& outError)
{
	if (m_bPhysicsPreviewActive)
	{
		outError = "Product debris cannot overlap a MapTool physics preview";
		return false;
	}
	std::vector<DEBRIS_PREVIEW_INSTANCE_DESC> instances;
	instances.reserve(desc.instances.size());
	for (const DESTRUCTION_DEBRIS_INSTANCE_DESC& source : desc.instances)
		instances.push_back({ source.modelPrototypeTag, source.uniformScale });
	return Begin_DebrisPresentation(
		instances,
		desc.suppressSource,
		DEBRIS_PRESENTATION_OWNER::PRODUCT_DESTRUCTION,
		outError);
}

uint32_t CDeployPropObject::
Get_DestructionDebrisPresentationInstanceCount() const
{
	return Get_DebrisPresentationInstanceCount(
		DEBRIS_PRESENTATION_OWNER::PRODUCT_DESTRUCTION);
}

bool_t CDeployPropObject::Get_DestructionDebrisPresentationLocalBounds(
	const uint32_t instanceIndex,
	float3_t& outCenter,
	float3_t& outHalfExtents) const
{
	return Get_DebrisPresentationLocalBounds(
		DEBRIS_PRESENTATION_OWNER::PRODUCT_DESTRUCTION,
		instanceIndex, outCenter, outHalfExtents);
}

bool_t CDeployPropObject::Apply_DestructionDebrisPresentationPose(
	const uint32_t instanceIndex,
	const float3_t& position,
	const float4_t& rotationQuaternion,
	const bool_t visible)
{
	return Apply_DebrisPresentationPose(
		DEBRIS_PRESENTATION_OWNER::PRODUCT_DESTRUCTION,
		instanceIndex, position, rotationQuaternion, visible);
}

void CDeployPropObject::End_DestructionDebrisPresentation()
{
	End_DebrisPresentation(
		DEBRIS_PRESENTATION_OWNER::PRODUCT_DESTRUCTION);
}

bool_t CDeployPropObject::
Is_DestructionDebrisPresentationActive() const
{
	return m_bDebrisPreviewActive &&
		DEBRIS_PRESENTATION_OWNER::PRODUCT_DESTRUCTION ==
		m_eDebrisPresentationOwner;
}

bool_t CDeployPropObject::Begin_TransientDestructionSuppression(
	std::string& outError)
{
	outError.clear();
	if (m_bTransientDestructionSuppressed || m_bDebrisPreviewActive ||
		m_bPhysicsPreviewActive)
	{
		outError = "Transient destruction suppression conflicts with an active presentation";
		return false;
	}
	m_bTransientDestructionSuppressed = true;
	return true;
}

void CDeployPropObject::End_TransientDestructionSuppression()
{
	m_bTransientDestructionSuppressed = false;
}

HRESULT CDeployPropObject::Ready_Components(const DEPLOY_PROP_DESC& desc)
{
	const wchar_t* shaderTag =
		desc.modelKind == DEPLOY_PROP_MODEL_KIND::ANIM ?
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary") :
		TEXT("Prototype_Component_Shader_VtxMeshBinary");
	if (FAILED(__super::Add_Component(
		desc.prototypeLevelIndex, shaderTag,
		TEXT("Com_Shader"), m_pShaderCom)) ||
		FAILED(__super::Add_Component(
			desc.prototypeLevelIndex, desc.intactPrototypeTag,
			TEXT("Com_Model_Intact"), m_pIntactModelCom)))
		return E_FAIL;

	if (desc.modelKind == DEPLOY_PROP_MODEL_KIND::STATIC &&
		FAILED(__super::Add_Component(
			desc.prototypeLevelIndex, desc.fracturedPrototypeTag,
			TEXT("Com_Model_Fractured"), m_pFracturedModelCom)))
		return E_FAIL;
	return S_OK;
}

HRESULT CDeployPropObject::Bind_CommonShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(
		m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;

	if (m_ModelKind == DEPLOY_PROP_MODEL_KIND::STATIC)
	{
		matrix_t world = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
		world.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		const matrix_t inverseTranspose =
			XMMatrixTranspose(XMMatrixInverse(nullptr, world));
		float4x4_t stored{};
		XMStoreFloat4x4(&stored, inverseTranspose);
		if (FAILED(m_pShaderCom->Bind_Matrix(
			"g_WorldInvTransposeMatrix", &stored)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CDeployPropObject::Bind_DebrisShaderResources(
	const float4x4_t& worldMatrix)
{
	if (nullptr == m_pDebrisShaderCom)
		return E_FAIL;

	matrix_t worldWithoutTranslation = XMLoadFloat4x4(&worldMatrix);
	worldWithoutTranslation.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	const matrix_t inverseTranspose = XMMatrixTranspose(
		XMMatrixInverse(nullptr, worldWithoutTranslation));
	float4x4_t storedInverseTranspose{};
	XMStoreFloat4x4(&storedInverseTranspose, inverseTranspose);
	if (FAILED(m_pDebrisShaderCom->Bind_Matrix(
		"g_WorldMatrix", &worldMatrix)) ||
		FAILED(m_pDebrisShaderCom->Bind_Matrix(
			"g_WorldInvTransposeMatrix", &storedInverseTranspose)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pDebrisShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pDebrisShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CDeployPropObject::Bind_ShadowShaderResources(
	const shared_ptr<CShader>& shader,
	const float4x4_t& worldMatrix)
{
	if (nullptr == shader ||
		FAILED(shader->Bind_Matrix("g_WorldMatrix", &worldMatrix)) ||
		FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			shader, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			shader, "g_ProjMatrix", D3DTS::PROJ)))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CDeployPropObject::Render_Static(
	const shared_ptr<CModel>& model,
	const shared_ptr<CShader>& shader,
	const uint32_t passIndex)
{
	if (nullptr == model || nullptr == shader)
		return E_FAIL;
	const float2_t uvScale = float2_t(1.f, 1.f);
	const float2_t uvOffset = float2_t(0.f, 0.f);
	const float opacity = 1.f;
	const float emissiveIntensity = 1.f;
	const float specularIntensity = 1.f;
	const float specularPower = 50.f;
	const float4_t colorTint = float4_t(1.f, 1.f, 1.f, 1.f);
	const uint32_t hasOpacity = 0u;
	for (uint32_t index = 0; index < model->Get_NumMeshes(); ++index)
	{
		const uint32_t hasNormal =
			model->Has_MaterialTexture(index, aiTextureType_NORMALS) ? 1u : 0u;
		const uint32_t hasEmissive =
			model->Has_MaterialTexture(index, aiTextureType_EMISSIVE) ? 1u : 0u;
		const uint32_t hasSpecular =
			model->Has_MaterialTexture(index, aiTextureType_SPECULAR) ? 1u : 0u;
		if (FAILED(model->Bind_Material(
			shader, "g_DiffuseTexture", index, aiTextureType_DIFFUSE)) ||
			FAILED(shader->Bind_RawValue("g_UVScale", &uvScale, sizeof(uvScale))) ||
			FAILED(shader->Bind_RawValue("g_UVOffset", &uvOffset, sizeof(uvOffset))) ||
			FAILED(shader->Bind_RawValue("g_Opacity", &opacity, sizeof(opacity))) ||
			FAILED(shader->Bind_RawValue("g_ColorTint", &colorTint, sizeof(colorTint))) ||
			FAILED(shader->Bind_RawValue("g_HasNormalTexture", &hasNormal, sizeof(hasNormal))) ||
			(0 != hasNormal && FAILED(model->Bind_Material(
				shader, "g_NormalTexture", index, aiTextureType_NORMALS))) ||
			FAILED(shader->Bind_RawValue("g_HasEmissiveTexture", &hasEmissive, sizeof(hasEmissive))) ||
			FAILED(shader->Bind_RawValue("g_EmissiveIntensity", &emissiveIntensity, sizeof(emissiveIntensity))) ||
			(0 != hasEmissive && FAILED(model->Bind_Material(
				shader, "g_EmissiveTexture", index, aiTextureType_EMISSIVE))) ||
			FAILED(shader->Bind_RawValue("g_HasSpecularTexture", &hasSpecular, sizeof(hasSpecular))) ||
			FAILED(shader->Bind_RawValue("g_SpecularIntensity", &specularIntensity, sizeof(specularIntensity))) ||
			FAILED(shader->Bind_RawValue("g_SpecularPower", &specularPower, sizeof(specularPower))) ||
			(0 != hasSpecular && FAILED(model->Bind_Material(
				shader, "g_SpecularTexture", index, aiTextureType_SPECULAR))) ||
			FAILED(shader->Bind_RawValue("g_HasOpacityTexture", &hasOpacity, sizeof(hasOpacity))) ||
			FAILED(shader->Begin(passIndex)) || FAILED(model->Render(index)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CDeployPropObject::Render_Animated(const uint32_t passIndex)
{
	for (uint32_t index = 0; index < m_pIntactModelCom->Get_NumMeshes(); ++index)
	{
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pIntactModelCom, m_pShaderCom, index)) ||
			FAILED(m_pIntactModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", index)) ||
			FAILED(m_pShaderCom->Begin(passIndex)) ||
			FAILED(m_pIntactModelCom->Render(index)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CDeployPropObject::Render_DebrisPreview(const bool_t shadowPass)
{
	constexpr uint32_t STATIC_SHADOW_PASS = 12u;
	if (!m_bDebrisPreviewActive || nullptr == m_pDebrisShaderCom)
		return E_FAIL;

	for (const DEBRIS_PREVIEW_INSTANCE& instance :
		m_DebrisPreviewInstances)
	{
		if (!instance.visible)
			continue;
		if (instance.resourceIndex >= m_DebrisPreviewResources.size())
			return E_FAIL;
		const shared_ptr<CModel>& model =
			m_DebrisPreviewResources[instance.resourceIndex].model;
		if (nullptr == model)
			return E_FAIL;

		const vector_t rotation = XMQuaternionNormalize(
			XMLoadFloat4(&instance.rotation));
		const matrix_t world = XMMatrixScaling(
			instance.uniformScale,
			instance.uniformScale,
			instance.uniformScale) * XMMatrixRotationQuaternion(rotation) *
			XMMatrixTranslation(
				instance.position.x,
				instance.position.y,
				instance.position.z);
		float4x4_t storedWorld{};
		XMStoreFloat4x4(&storedWorld, world);
		const HRESULT bindResult = shadowPass ?
			Bind_ShadowShaderResources(m_pDebrisShaderCom, storedWorld) :
			Bind_DebrisShaderResources(storedWorld);
		if (FAILED(bindResult) ||
			FAILED(Render_Static(
				model, m_pDebrisShaderCom,
				shadowPass ? STATIC_SHADOW_PASS : 0u)))
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

bool_t CDeployPropObject::Has_VisibleDebrisPreviewInstance() const
{
	if (!m_bDebrisPreviewActive)
		return false;
	return m_DebrisPreviewInstances.end() != std::find_if(
		m_DebrisPreviewInstances.begin(),
		m_DebrisPreviewInstances.end(),
		[](const DEBRIS_PREVIEW_INSTANCE& instance)
		{
			return instance.visible;
		});
}

bool_t CDeployPropObject::Is_BasePresentationSuppressed() const
{
	if (m_bTransientDestructionSuppressed)
		return true;
	if (!m_bDebrisPreviewActive || !m_bDebrisSuppressSource)
		return false;
	if (DEBRIS_PRESENTATION_OWNER::PRODUCT_DESTRUCTION ==
		m_eDebrisPresentationOwner)
	{
		return true;
	}
	return DEBRIS_PRESENTATION_OWNER::MAPTOOL_PREVIEW ==
		m_eDebrisPresentationOwner && m_bPhysicsPreviewActive;
}

void CDeployPropObject::Apply_Transform()
{
	const float4_t& rootRotation = m_bPhysicsPreviewActive ?
		m_PhysicsPreviewRotation : m_Placement.rotationQuaternion;
	const float3_t& rootPosition = m_bPhysicsPreviewActive ?
		m_PhysicsPreviewPosition : m_Placement.position;
	const vector_t quaternion =
		XMQuaternionNormalize(XMLoadFloat4(&rootRotation));
	const matrix_t world = XMMatrixScaling(
		m_Placement.uniformScale,
		m_Placement.uniformScale,
		m_Placement.uniformScale) * XMMatrixRotationQuaternion(quaternion);
	m_pTransformCom->Set_State(STATE::RIGHT, world.r[0]);
	m_pTransformCom->Set_State(STATE::UP, world.r[1]);
	m_pTransformCom->Set_State(STATE::LOOK, world.r[2]);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(
		rootPosition.x, rootPosition.y,
		rootPosition.z, 1.f));
}

unique_ptr<CDeployPropObject> CDeployPropObject::Create(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CDeployPropObject>(
		new CDeployPropObject(pDevice, pContext));
	if (FAILED(instance->Initialize_Prototype()))
		return nullptr;
	return instance;
}

shared_ptr<CPrototype> CDeployPropObject::Clone(void* pArg)
{
	auto instance = shared_ptr<CDeployPropObject>(new CDeployPropObject(*this));
	if (FAILED(instance->Initialize(pArg)))
		return nullptr;
	return instance;
}
