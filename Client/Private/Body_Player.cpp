#include "Body_Player.h"
#include "GameInstance.h"

#include "Player.h"

CBody_Player::CBody_Player(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CBody_Player::~CBody_Player()
{
	int a = 10;

}

HRESULT CBody_Player::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBody_Player::Initialize(void* pArg)
{
	auto	pDesc = static_cast<CBody_Player::BODY_DESC*>(pArg);
	m_pParentState = pDesc->pParentState;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	/*m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(CGameInstance::Get().Random(1.f, 10), 1.f, CGameInstance::Get().Random(1.f, 10), 1.f));*/

	m_pModelCom->Set_Animation(3, true);


	return S_OK;
}

void CBody_Player::Priority_Update(f32_t fTimeDelta)
{
}

void CBody_Player::Update(f32_t fTimeDelta)
{
	if (true == m_pModelCom->Play_Animation(fTimeDelta))
		int a = 10;

	if (*m_pParentState & CPlayer::PLAYER_STATE::IDLE)
		m_pModelCom->Set_Animation(3, true);
	if (*m_pParentState & CPlayer::PLAYER_STATE::WALK)
		m_pModelCom->Set_Animation(4, true);

	__super::Update_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	m_pColliderCom->Update(XMLoadFloat4x4(&m_CombinedWorldMatrix));


}

void CBody_Player::Late_Update(f32_t fTimeDelta)
{

	CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONBLEND, static_pointer_cast<CGameObject>(shared_from_this()));	
#ifdef _DEBUG
	CGameInstance::Get().Add_DebugComponent(m_pColliderCom);	
#endif
}

HRESULT CBody_Player::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	uint32_t		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (uint32_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;
	
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}



	

	return S_OK;
}

HRESULT CBody_Player::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Com_Shader"), m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Fiona"),
		TEXT("Com_Model"), m_pModelCom)))
		return E_FAIL;

	/* For.Com_Collider_Sphere */
	CBounding_Sphere::BOUNDING_SPHERE_DESC		SphereDesc{};
	SphereDesc.fRadius = 0.5f;
	SphereDesc.vCenter = float3_t(0.f, SphereDesc.fRadius, 0.f);

	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider_Sphere"), m_pColliderCom, &SphereDesc)))
		return E_FAIL;


	return S_OK;
}

HRESULT CBody_Player::Bind_ShaderResources()
{
	/*if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;*/
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;


	return S_OK;
}

unique_ptr<CBody_Player> CBody_Player::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto	pInstance = unique_ptr<CBody_Player>(new CBody_Player(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CBody_Player");

	return move(pInstance);
}

shared_ptr<CPrototype> CBody_Player::Clone(void* pArg)
{
	auto	pInstance = shared_ptr<CBody_Player>(new CBody_Player(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CBody_Player");

	return pInstance;
}
