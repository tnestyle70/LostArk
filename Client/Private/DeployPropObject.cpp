#include "DeployPropObject.h"

#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"

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
	if (0 == desc.placement.runtimePlacementId ||
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
	if (m_State == DEPLOY_PROP_STATE::FRACTURED &&
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

void CDeployPropObject::Set_State(DEPLOY_PROP_STATE state)
{
	if (m_ModelKind == DEPLOY_PROP_MODEL_KIND::ANIM &&
		m_pIntactModelCom->Get_NumAnimations() > 0)
	{
		if (state == DEPLOY_PROP_STATE::INTACT &&
			!Apply_LogicalAnimation(m_pIntactModelCom, "on"))
			return;
		if (state == DEPLOY_PROP_STATE::FRACTURED &&
			!Apply_LogicalAnimation(m_pIntactModelCom, "off"))
			return;
	}
	m_State = state;
}

bool_t CDeployPropObject::Is_AnimBindPoseOnly() const
{
	return m_ModelKind == DEPLOY_PROP_MODEL_KIND::ANIM &&
		m_pIntactModelCom->Get_NumAnimations() == 0;
}

HRESULT CDeployPropObject::Ready_Components(const DEPLOY_PROP_DESC& desc)
{
	const wchar_t* shaderTag =
		desc.modelKind == DEPLOY_PROP_MODEL_KIND::ANIM ?
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary") :
		TEXT("Prototype_Component_Shader_VtxMeshBinary");
	if (FAILED(__super::Add_Component(
		ETOUI(LEVEL::DEVELOPMENT), shaderTag,
		TEXT("Com_Shader"), m_pShaderCom)) ||
		FAILED(__super::Add_Component(
			ETOUI(LEVEL::DEVELOPMENT), desc.intactPrototypeTag,
			TEXT("Com_Model_Intact"), m_pIntactModelCom)))
		return E_FAIL;

	if (desc.modelKind == DEPLOY_PROP_MODEL_KIND::STATIC &&
		FAILED(__super::Add_Component(
			ETOUI(LEVEL::DEVELOPMENT), desc.fracturedPrototypeTag,
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
		const uint32_t hasNormal =
			m_pIntactModelCom->Has_MaterialTexture(index, aiTextureType_NORMALS) ? 1u : 0u;
		if (FAILED(m_pIntactModelCom->Bind_Material(
			m_pShaderCom, "g_DiffuseTexture", index, aiTextureType_DIFFUSE)) ||
			FAILED(m_pShaderCom->Bind_RawValue(
				"g_HasNormalTexture", &hasNormal, sizeof(hasNormal))) ||
			(0 != hasNormal && FAILED(m_pIntactModelCom->Bind_Material(
				m_pShaderCom, "g_NormalTexture", index, aiTextureType_NORMALS))) ||
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
	const vector_t quaternion =
		XMQuaternionNormalize(XMLoadFloat4(&m_Placement.rotationQuaternion));
	const matrix_t world = XMMatrixScaling(
		m_Placement.uniformScale,
		m_Placement.uniformScale,
		m_Placement.uniformScale) * XMMatrixRotationQuaternion(quaternion);
	m_pTransformCom->Set_State(STATE::RIGHT, world.r[0]);
	m_pTransformCom->Set_State(STATE::UP, world.r[1]);
	m_pTransformCom->Set_State(STATE::LOOK, world.r[2]);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(
		m_Placement.position.x, m_Placement.position.y,
		m_Placement.position.z, 1.f));
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
