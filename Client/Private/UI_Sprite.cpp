#include "UI_Sprite.h"

#include "GameInstance.h"

Client::CUI_Sprite::CUI_Sprite(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CUIObject { pDevice, pContext }
{
}

Client::CUI_Sprite::~CUI_Sprite()
{
}

HRESULT Client::CUI_Sprite::Initialize_Prototype()
{
	return S_OK;
}

HRESULT Client::CUI_Sprite::Initialize(void* pArg)
{
	auto pDesc = static_cast<UI_SPRITE_DESC*>(pArg);

	m_strTextureTag = pDesc->strTextureTag;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void Client::CUI_Sprite::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(RENDERGROUP::UI, static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT Client::CUI_Sprite::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Resources()))
		return E_FAIL;

	/* Pass 2 (UIBlend) alpha-blends and skips the depth-based soft fade pass 1 needs, since
	this sprite has no depth texture to sample -- pass 0's default blend state would otherwise
	paint every transparent pixel as opaque, showing whatever raw color sits behind the alpha. */
	if (FAILED(m_pShaderCom->Begin(2)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void Client::CUI_Sprite::Set_Rect(f32_t fCenterX, f32_t fCenterY, f32_t fSizeX, f32_t fSizeY)
{
	m_fX = fCenterX;
	m_fY = fCenterY;
	m_fSizeX = fSizeX;
	m_fSizeY = fSizeY;

	m_pTransformCom->Scale(m_fSizeX, m_fSizeY);
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(
			m_fX - CGameInstance::Get().Get_ViewportSize().x * 0.5f,
			-m_fY + CGameInstance::Get().Get_ViewportSize().y * 0.5f,
			0.f, 1.f));
}

HRESULT Client::CUI_Sprite::Ready_Components()
{
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::STATIC), m_strTextureTag,
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

HRESULT Client::CUI_Sprite::Bind_ShaderResources()
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

unique_ptr<Client::CUI_Sprite> Client::CUI_Sprite::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CUI_Sprite>(new CUI_Sprite(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CUI_Sprite");

	return move(pInstance);
}

shared_ptr<CPrototype> Client::CUI_Sprite::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CUI_Sprite>(new CUI_Sprite(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CUI_Sprite");

	return pInstance;
}
