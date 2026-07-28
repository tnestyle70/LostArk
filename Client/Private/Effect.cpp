#include "Effect.h"

#include "GameInstance.h"

CEffect::CEffect(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CEffect::~CEffect()
{
	int a = 10;

}

HRESULT CEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEffect::Initialize(void* pArg)
{	

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(
		rand() % 20, 
		2.f, 
		rand() % 20,
		1.f	
	));


	return S_OK;
}

void CEffect::Priority_Update(f32_t fTimeDelta)
{
}

void CEffect::Update(f32_t fTimeDelta)
{
	m_fFrame += 90.f * fTimeDelta;

	if (m_fFrame >= 90.f)
		m_fFrame = 0.f;
}

void CEffect::Late_Update(f32_t fTimeDelta)
{

	CGameInstance::Get().Add_RenderObject(RENDERGROUP::BLEND, static_pointer_cast<CGameObject>(shared_from_this()));	
}

HRESULT CEffect::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Resources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(1)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;



	return S_OK;
}

HRESULT CEffect::Ready_Components()
{
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Effect"),
		TEXT("Com_Texture"), m_pTextureCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxTex"),
		TEXT("Com_Shader"), m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), m_pVIBufferCom)))
		return E_FAIL;


	return S_OK;
}

HRESULT CEffect::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	
	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;


	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", static_cast<uint32_t>(m_fFrame))))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Depth"), m_pShaderCom, "g_DepthTexture")))
		return E_FAIL;

	

	return S_OK;
}

unique_ptr<CEffect> CEffect::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto	pInstance = unique_ptr<CEffect>(new CEffect(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CEffect");

	return move(pInstance);
}

shared_ptr<CPrototype> CEffect::Clone(void* pArg)
{
	auto	pInstance = shared_ptr<CEffect>(new CEffect(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CEffect");

	return pInstance;
}
