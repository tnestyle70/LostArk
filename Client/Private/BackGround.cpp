#include "BackGround.h"

#include "GameInstance.h"

CBackGround::CBackGround(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CUIObject { pDevice, pContext }
{
}

CBackGround::~CBackGround()
{
	int a = 10;

}

HRESULT CBackGround::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBackGround::Initialize(void* pArg)
{
	BACKGROUND_DESC			Desc{};

	Desc.fSizeX = 300.f;
	Desc.fSizeY = 300.f;
	Desc.fX = 150.f;
	Desc.fY = 150.f;
	Desc.fSpeedPerSec = 10.f;
	Desc.fRotationPerSec = 90.f;

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;


	return S_OK;
}

void CBackGround::Priority_Update(f32_t fTimeDelta)
{
}

void CBackGround::Update(f32_t fTimeDelta)
{
}

void CBackGround::Late_Update(f32_t fTimeDelta)
{

	CGameInstance::Get().Add_RenderObject(RENDERGROUP::PRIORITY, static_pointer_cast<CGameObject>(shared_from_this()));	
}

HRESULT CBackGround::Render()
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

HRESULT CBackGround::Ready_Components()
{
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::LOGO), TEXT("Prototype_Component_Texture_BackGround"),
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

HRESULT CBackGround::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	
	if (FAILED(__super::Bind_ShaderResource(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;

	if (FAILED(__super::Bind_ShaderResource(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;	

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	return S_OK;
}

unique_ptr<CBackGround> CBackGround::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto	pInstance = unique_ptr<CBackGround>(new CBackGround(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CBackGround");

	return move(pInstance);
}

shared_ptr<CPrototype> CBackGround::Clone(void* pArg)
{
	auto	pInstance = shared_ptr<CBackGround>(new CBackGround(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CBackGround");

	return pInstance;
}
