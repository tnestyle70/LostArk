#include "Snow.h"
#include "GameInstance.h"

CSnow::CSnow(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CSnow::~CSnow()
{
	int a = 10;

}

HRESULT CSnow::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSnow::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(64.f, 15.f, 64.f, 1.f));

	return S_OK;
}

void CSnow::Priority_Update(f32_t fTimeDelta)
{
}

void CSnow::Update(f32_t fTimeDelta)
{
	m_pVIBufferCom->Drop(fTimeDelta);
}

void CSnow::Late_Update(f32_t fTimeDelta)
{

	CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONLIGHT, static_pointer_cast<CGameObject>(shared_from_this()));	
}

HRESULT CSnow::Render()
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

HRESULT CSnow::Ready_Components()
{
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Snow"),
		TEXT("Com_Texture"), m_TextureCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxInstanceParticleRect"),
		TEXT("Com_Shader"), m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_VIBuffer_Instance_Rect_Snow"),
		TEXT("Com_VIBuffer"), m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSnow::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;	

	if (FAILED(m_TextureCom->Bind_ShaderResources(m_pShaderCom, "g_Texture")))
		return E_FAIL;

	return S_OK;
}

unique_ptr<CSnow> CSnow::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto	pInstance = unique_ptr<CSnow>(new CSnow(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CSnow");

	return move(pInstance);
}

shared_ptr<CPrototype> CSnow::Clone(void* pArg)
{
	auto	pInstance = shared_ptr<CSnow>(new CSnow(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CSnow");

	return pInstance;
}
