#include "Sky.h"
#include "GameInstance.h"

CSky::CSky(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CSky::~CSky()
{


}

HRESULT CSky::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSky::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;


	return S_OK;
}

void CSky::Priority_Update(f32_t fTimeDelta)
{
}

void CSky::Update(f32_t fTimeDelta)
{
	m_pTransformCom->Set_State(STATE::POSITION, 
		XMLoadFloat4(CGameInstance::Get().Get_CamPosition()));
}

void CSky::Late_Update(f32_t fTimeDelta)
{

	CGameInstance::Get().Add_RenderObject(RENDERGROUP::PRIORITY, static_pointer_cast<CGameObject>(shared_from_this()));	
}

HRESULT CSky::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Resources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CSky::Ready_Components()
{
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Sky"),
		TEXT("Com_Texture"), m_pTextureCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxCube"),
		TEXT("Com_Shader"), m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_VIBuffer_Cube"),
		TEXT("Com_VIBuffer"), m_pVIBufferCom)))
		return E_FAIL;


	return S_OK;
}

HRESULT CSky::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;	

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 3)))
		return E_FAIL;
	
	return S_OK;
}

unique_ptr<CSky> CSky::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto	pInstance = unique_ptr<CSky>(new CSky(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CSky");

	return move(pInstance);
}

shared_ptr<CPrototype> CSky::Clone(void* pArg)
{
	auto	pInstance = shared_ptr<CSky>(new CSky(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CSky");

	return pInstance;
}
