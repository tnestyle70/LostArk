#include "Terrain.h"
#include "GameInstance.h"

CTerrain::CTerrain(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CTerrain::~CTerrain()
{
	int a = 10;

}

HRESULT CTerrain::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTerrain::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;


	return S_OK;
}

void CTerrain::Priority_Update(f32_t fTimeDelta)
{
}

void CTerrain::Update(f32_t fTimeDelta)
{
	m_pVIBufferCom->Culling(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CTerrain::Late_Update(f32_t fTimeDelta)
{

	CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONBLEND, static_pointer_cast<CGameObject>(shared_from_this()));	
}

HRESULT CTerrain::Render()
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

HRESULT CTerrain::Ready_Components()
{
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Terrain"),
		TEXT("Com_Texture"), m_TextureCom[TEXTURE::DIFFUSE])))
		return E_FAIL;
	/* For.Com_Texture_Mask */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Terrain_Mask"),
		TEXT("Com_Texture_Mask"), m_TextureCom[TEXTURE::MASK])))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxNorTex"),
		TEXT("Com_Shader"), m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_VIBuffer_Terrain"),
		TEXT("Com_VIBuffer"), m_pVIBufferCom)))
		return E_FAIL;

	/* For.Com_Navigation */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_TerrainNavigation"),
		TEXT("Com_Navigation"), m_pNavigationCom)))
		return E_FAIL;


	return S_OK;
}

HRESULT CTerrain::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;	

	if (FAILED(m_TextureCom[TEXTURE::DIFFUSE]->Bind_ShaderResources(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;


	if (FAILED(m_TextureCom[TEXTURE::MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0)))
		return E_FAIL;

	

	return S_OK;
}

unique_ptr<CTerrain> CTerrain::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto	pInstance = unique_ptr<CTerrain>(new CTerrain(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CTerrain");

	return move(pInstance);
}

shared_ptr<CPrototype> CTerrain::Clone(void* pArg)
{
	auto	pInstance = shared_ptr<CTerrain>(new CTerrain(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CTerrain");

	return pInstance;
}
