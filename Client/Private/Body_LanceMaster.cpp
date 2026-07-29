#include "Body_LanceMaster.h"

#include "GameInstance.h"
#include "LanceMaster.h"

CBody_LanceMaster::CBody_LanceMaster(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CBody_LanceMaster::~CBody_LanceMaster()
{
}

HRESULT CBody_LanceMaster::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBody_LanceMaster::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<BODY_LANCEMASTER_DESC*>(pArg);
	m_pParentState = pDesc->pParentState;
	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components()))
		return E_FAIL;

	/* FBX take name is "<armature>_<action>", so the idle clip is prefixed. */
	if (!m_pModelCom->Set_Animation("flm_idle_battle_1", true))
		m_pModelCom->Set_Animation(0u, true);
	return S_OK;
}

void CBody_LanceMaster::Priority_Update(f32_t fTimeDelta)
{
}

void CBody_LanceMaster::Update(f32_t fTimeDelta)
{
	if (nullptr != m_pParentState && (*m_pParentState & CLanceMaster::LANCEMASTER_STATE::IDLE))
		m_pModelCom->Play_Animation(fTimeDelta);

	__super::Update_CombinedWorldMatrix(
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CBody_LanceMaster::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CBody_LanceMaster::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		const uint32_t hasNormalTexture =
			m_pModelCom->Has_MaterialTexture(i, aiTextureType_NORMALS) ? 1u : 0u;
		if (FAILED(m_pModelCom->Bind_Material(
			m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)) ||
			FAILED(m_pShaderCom->Bind_RawValue(
				"g_HasNormalTexture", &hasNormalTexture, sizeof(hasNormalTexture))) ||
			(0 != hasNormalTexture && FAILED(m_pModelCom->Bind_Material(
				m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0))) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CBody_LanceMaster::Ready_Components()
{
	if (FAILED(__super::Add_Component(
		ETOUI(LEVEL::TEST_LEVEL2),
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
		TEXT("Com_Shader"),
		m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(
		ETOUI(LEVEL::TEST_LEVEL2),
		TEXT("Prototype_Component_Model_LanceMaster"),
		TEXT("Com_Model"),
		m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBody_LanceMaster::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

unique_ptr<CBody_LanceMaster> CBody_LanceMaster::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CBody_LanceMaster>(new CBody_LanceMaster(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBody_LanceMaster");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CBody_LanceMaster::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CBody_LanceMaster>(new CBody_LanceMaster(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CBody_LanceMaster");
		return nullptr;
	}
	return pInstance;
}
