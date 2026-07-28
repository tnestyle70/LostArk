#include "Explosion.h"
#include "GameInstance.h"

CExplosion::CExplosion(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject{ pDevice, pContext }
{
}

CExplosion::~CExplosion()
{
	int a = 10;

}

HRESULT CExplosion::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CExplosion::Initialize(void* pArg)
{
	auto	pDesc = static_cast<CExplosion::EFFECT_DESC*>(pArg);
	m_pParentState = pDesc->pParentState;
	m_pSocketModelCom = pDesc->pSocketModel;


	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	/*m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(64.f, 15.f, 64.f, 1.f));*/

	return S_OK;
}

void CExplosion::Priority_Update(f32_t fTimeDelta)
{
}

void CExplosion::Update(f32_t fTimeDelta)
{
	matrix_t		SocketMatrix = m_pSocketModelCom->Get_BoneMatrix("SWORD");

	for (uint32_t i = 0; i < 3; i++)
	{
		SocketMatrix.r[i] = XMVector3Normalize(SocketMatrix.r[i]);
	}

	matrix_t		ChildMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) * SocketMatrix;

	__super::Update_CombinedWorldMatrix(ChildMatrix);

	m_pVIBufferCom->Spread(fTimeDelta);
}

void CExplosion::Late_Update(f32_t fTimeDelta)
{

	CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONLIGHT, static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CExplosion::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Resources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

#ifdef _DEBUG
	// m_pNavigationCom->Render();
#endif

	return S_OK;
}

HRESULT CExplosion::Ready_Components()
{
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Snow"),
		TEXT("Com_Texture"), m_TextureCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxInstanceParticlePoint"),
		TEXT("Com_Shader"), m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_VIBuffer_Instance_Point_Explosion"),
		TEXT("Com_VIBuffer"), m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CExplosion::Bind_ShaderResources()
{
	/*if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;*/
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;


	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;

	if (FAILED(m_TextureCom->Bind_ShaderResources(m_pShaderCom, "g_Texture")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", CGameInstance::Get().Get_CamPosition(), sizeof(float4_t))))
		return E_FAIL;

	return S_OK;
}

unique_ptr<CExplosion> CExplosion::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto	pInstance = unique_ptr<CExplosion>(new CExplosion(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CExplosion");

	return move(pInstance);
}

shared_ptr<CPrototype> CExplosion::Clone(void* pArg)
{
	auto	pInstance = shared_ptr<CExplosion>(new CExplosion(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CExplosion");

	return pInstance;
}
