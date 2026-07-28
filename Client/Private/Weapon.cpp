#include "Weapon.h"
#include "GameInstance.h"

#include "Player.h"

CWeapon::CWeapon(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CWeapon::~CWeapon()
{
	int a = 10;

}

HRESULT CWeapon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWeapon::Initialize(void* pArg)
{
	auto	pDesc = static_cast<CWeapon::WEAPON_DESC*>(pArg);
	m_pParentState = pDesc->pParentState;
	m_pSocketModelCom = pDesc->pSocketModel;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Scale(0.1f, 0.1f, 0.1f);
	m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 90.f);
	

	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(0.75f, 0.f, 0.f, 1.f));

	return S_OK;
}

void CWeapon::Priority_Update(f32_t fTimeDelta)
{
}

void CWeapon::Update(f32_t fTimeDelta)
{	
	matrix_t		SocketMatrix = m_pSocketModelCom->Get_BoneMatrix("SWORD");

	for (uint32_t i = 0; i < 3; i++)
	{
		SocketMatrix.r[i] = XMVector3Normalize(SocketMatrix.r[i]);
	}
	



	matrix_t		ChildMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) * SocketMatrix;

	__super::Update_CombinedWorldMatrix(ChildMatrix);

	m_pColliderCom->Update(XMLoadFloat4x4(&m_CombinedWorldMatrix));
}

void CWeapon::Late_Update(f32_t fTimeDelta)
{

	CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONBLEND, static_pointer_cast<CGameObject>(shared_from_this()));	
#ifdef _DEBUG
	CGameInstance::Get().Add_DebugComponent(m_pColliderCom);
#endif
}

HRESULT CWeapon::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	uint32_t		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (uint32_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		//if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
		//	return E_FAIL;
	
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}


	return S_OK;
}

HRESULT CWeapon::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_ForkLift"),
		TEXT("Com_Model"), m_pModelCom)))
		return E_FAIL;

	/* For.Com_Collider_OBB */
	CBounding_OBB::BOUNDING_OBB_DESC		OBBDesc{};
	OBBDesc.vSize = float3_t(1.5f, 2.5f, 3.f);
	OBBDesc.vDegree = float3_t(0.f, 0.f, 0.f);
	OBBDesc.vCenter = float3_t(0.f, OBBDesc.vSize.y * 0.5f, 0.f);	

	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider_OBB"), m_pColliderCom, &OBBDesc)))
		return E_FAIL;
	return S_OK;
}

HRESULT CWeapon::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;


	return S_OK;
}

unique_ptr<CWeapon> CWeapon::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto	pInstance = unique_ptr<CWeapon>(new CWeapon(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CWeapon");

	return move(pInstance);
}

shared_ptr<CPrototype> CWeapon::Clone(void* pArg)
{
	auto	pInstance = shared_ptr<CWeapon>(new CWeapon(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CWeapon");

	return pInstance;
}
