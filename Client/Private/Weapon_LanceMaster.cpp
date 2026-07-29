#include "Weapon_LanceMaster.h"

#include "GameInstance.h"

CWeapon_LanceMaster::CWeapon_LanceMaster(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CWeapon_LanceMaster::~CWeapon_LanceMaster()
{
}

HRESULT CWeapon_LanceMaster::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWeapon_LanceMaster::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<WEAPON_LANCEMASTER_DESC*>(pArg);
	m_pSocketModelCom = pDesc->pSocketModel;
	m_pSocketBoneName = pDesc->pSocketBoneName;

	if (nullptr == m_pSocketModelCom || nullptr == m_pSocketBoneName)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CWeapon_LanceMaster::Priority_Update(f32_t fTimeDelta)
{
}

void CWeapon_LanceMaster::Update(f32_t fTimeDelta)
{
	/* The socket matrix already carries the body's pre-transform, so the weapon
	model is registered with an identity pre-transform and inherits the scale
	from the bone. Normalising the rows here would strip that scale. */
	const matrix_t SocketMatrix = m_pSocketModelCom->Get_BoneMatrix(m_pSocketBoneName);

	const matrix_t ChildMatrix =
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) * SocketMatrix;

	__super::Update_CombinedWorldMatrix(ChildMatrix);
}

void CWeapon_LanceMaster::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CWeapon_LanceMaster::Render()
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
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CWeapon_LanceMaster::Ready_Components()
{
	/* Static mesh path: no bone matrices, so the non-skinned binary shader. */
	if (FAILED(__super::Add_Component(
		ETOUI(LEVEL::TEST_LEVEL2),
		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		TEXT("Com_Shader"),
		m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(
		ETOUI(LEVEL::TEST_LEVEL2),
		TEXT("Prototype_Component_Model_LanceMaster_Weapon"),
		TEXT("Com_Model"),
		m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CWeapon_LanceMaster::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

unique_ptr<CWeapon_LanceMaster> CWeapon_LanceMaster::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CWeapon_LanceMaster>(new CWeapon_LanceMaster(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CWeapon_LanceMaster");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CWeapon_LanceMaster::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CWeapon_LanceMaster>(new CWeapon_LanceMaster(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CWeapon_LanceMaster");
		return nullptr;
	}
	return pInstance;
}
