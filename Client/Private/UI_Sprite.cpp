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
	if (!m_bVisible)
		return;
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
	paint every transparent pixel as opaque, showing whatever raw color sits behind the alpha.
	Pass 3 (UIBlendAdditive) is the same shader with an additive blend state, for a layer
	authored with a Scaleform additive blendMode. */
	if (FAILED(m_pShaderCom->Begin(m_bAdditive ? 3 : 2)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void Client::CUI_Sprite::Set_Tint(const float4_t& vTint)
{
	m_vTint = vTint;
}

void Client::CUI_Sprite::Set_TintMultiplier(const float4_t& vTintMultiplier)
{
	m_vTintMultiplier = vTintMultiplier;
}

void Client::CUI_Sprite::Set_FlipX(bool_t bFlipX)
{
	m_bFlipX = bFlipX;
}

void Client::CUI_Sprite::Set_Additive(bool_t bAdditive)
{
	m_bAdditive = bAdditive;
}

void Client::CUI_Sprite::Set_FillRatio(f32_t fFillRatio)
{
	m_fFillRatio = fFillRatio;
}

void Client::CUI_Sprite::Set_ArcRatio(f32_t fArcRatio)
{
	m_fArcRatio = fArcRatio;
}

void Client::CUI_Sprite::Set_UVWindow(const float2_t& vOffset, const float2_t& vScale)
{
	m_vUVOffset = vOffset;
	m_vUVScale = vScale;
}

void Client::CUI_Sprite::Set_Rotation(f32_t fDegrees)
{
	if (m_fRotationDeg == fDegrees)
		return;
	m_fRotationDeg = fDegrees;
	Apply_Transform();
}

void Client::CUI_Sprite::Set_Visible(bool_t bVisible)
{
	m_bVisible = bVisible;
}

void Client::CUI_Sprite::Set_Texture(ComPtr<ID3D11ShaderResourceView> pOverrideSRV)
{
	m_pOverrideTextureSRV = pOverrideSRV;
}

void Client::CUI_Sprite::Set_Rect(f32_t fCenterX, f32_t fCenterY, f32_t fSizeX, f32_t fSizeY)
{
	m_fX = fCenterX;
	m_fY = fCenterY;
	m_fSizeX = fSizeX;
	m_fSizeY = fSizeY;

	Apply_Transform();
}

void Client::CUI_Sprite::Apply_Transform()
{
	m_pTransformCom->Scale(m_fSizeX, m_fSizeY);
	/* Screen-space clockwise degrees (the HUD Layout Tool/HUDRuntimeView convention, y-down) map
	to a negative mathematical rotation about +Z in this y-up world space. CTransform::Rotation
	rebuilds right/up/look from axis-aligned axes at the current scale, so it must run after
	Scale and always runs (0 degrees rebuilds the identity axes a previous nonzero rotation
	left rotated -- Scale alone only re-lengthens the already-rotated axes). */
	m_pTransformCom->Rotation(XMVectorSet(0.f, 0.f, 1.f, 0.f), -m_fRotationDeg);
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

	if (nullptr != m_pOverrideTextureSRV)
	{
		if (FAILED(m_pShaderCom->Bind_Texture("g_Texture", m_pOverrideTextureSRV)))
			return E_FAIL;
	}
	else if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
	{
		return E_FAIL;
	}

	const float4_t vEffectiveTint = float4_t(
		m_vTint.x * m_vTintMultiplier.x,
		m_vTint.y * m_vTintMultiplier.y,
		m_vTint.z * m_vTintMultiplier.z,
		m_vTint.w * m_vTintMultiplier.w);
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_TintColor", &vEffectiveTint, sizeof(vEffectiveTint))))
		return E_FAIL;

	const int32_t iFlipX = m_bFlipX ? 1 : 0;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_FlipX", &iFlipX, sizeof(iFlipX))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_FillRatio", &m_fFillRatio, sizeof(m_fFillRatio))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_ArcRatio", &m_fArcRatio, sizeof(m_fArcRatio))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_UVOffset", &m_vUVOffset, sizeof(m_vUVOffset))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_UVScale", &m_vUVScale, sizeof(m_vUVScale))))
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
