#include "DeployPropObject.h"

#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <string_view>

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
	if (m_State == DEPLOY_PROP_STATE::DESPAWNED)
		return;
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CDeployPropObject::Render()
{
	if (m_State == DEPLOY_PROP_STATE::DESPAWNED ||
		FAILED(Bind_CommonShaderResources()))
		return m_State == DEPLOY_PROP_STATE::DESPAWNED ? S_OK : E_FAIL;
	if (m_ModelKind == DEPLOY_PROP_MODEL_KIND::ANIM)
		return Render_Animated();
	return Render_Static(
		m_State == DEPLOY_PROP_STATE::FRACTURED ?
		m_pFracturedModelCom : m_pIntactModelCom);
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

HRESULT CDeployPropObject::Render_Static(const shared_ptr<CModel>& model)
{
	if (nullptr == model)
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
			m_pShaderCom, "g_DiffuseTexture", index, aiTextureType_DIFFUSE)) ||
			FAILED(m_pShaderCom->Bind_RawValue("g_UVScale", &uvScale, sizeof(uvScale))) ||
			FAILED(m_pShaderCom->Bind_RawValue("g_UVOffset", &uvOffset, sizeof(uvOffset))) ||
			FAILED(m_pShaderCom->Bind_RawValue("g_Opacity", &opacity, sizeof(opacity))) ||
			FAILED(m_pShaderCom->Bind_RawValue("g_ColorTint", &colorTint, sizeof(colorTint))) ||
			FAILED(m_pShaderCom->Bind_RawValue("g_HasNormalTexture", &hasNormal, sizeof(hasNormal))) ||
			(0 != hasNormal && FAILED(model->Bind_Material(
				m_pShaderCom, "g_NormalTexture", index, aiTextureType_NORMALS))) ||
			FAILED(m_pShaderCom->Bind_RawValue("g_HasEmissiveTexture", &hasEmissive, sizeof(hasEmissive))) ||
			FAILED(m_pShaderCom->Bind_RawValue("g_EmissiveIntensity", &emissiveIntensity, sizeof(emissiveIntensity))) ||
			(0 != hasEmissive && FAILED(model->Bind_Material(
				m_pShaderCom, "g_EmissiveTexture", index, aiTextureType_EMISSIVE))) ||
			FAILED(m_pShaderCom->Bind_RawValue("g_HasSpecularTexture", &hasSpecular, sizeof(hasSpecular))) ||
			FAILED(m_pShaderCom->Bind_RawValue("g_SpecularIntensity", &specularIntensity, sizeof(specularIntensity))) ||
			FAILED(m_pShaderCom->Bind_RawValue("g_SpecularPower", &specularPower, sizeof(specularPower))) ||
			(0 != hasSpecular && FAILED(model->Bind_Material(
				m_pShaderCom, "g_SpecularTexture", index, aiTextureType_SPECULAR))) ||
			FAILED(m_pShaderCom->Bind_RawValue("g_HasOpacityTexture", &hasOpacity, sizeof(hasOpacity))) ||
			FAILED(m_pShaderCom->Begin(0)) || FAILED(model->Render(index)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CDeployPropObject::Render_Animated()
{
	for (uint32_t index = 0; index < m_pIntactModelCom->Get_NumMeshes(); ++index)
	{
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pIntactModelCom, m_pShaderCom, index)) ||
			FAILED(m_pIntactModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", index)) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pIntactModelCom->Render(index)))
			return E_FAIL;
	}
	return S_OK;
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
