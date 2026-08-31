#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

/* A single textured rect for screen-space UI (loading background, a progress bar's
   two rects). Unlike CBackGround, the texture prototype tag is passed in per-instance
   so one prototype can be Cloned for several different sprites. */
class CUI_Sprite final : public CUIObject
{
public:
	typedef struct tagUISpriteDesc : public CUIObject::UIOBJECT_DESC
	{
		wstring_t	strTextureTag;
	}UI_SPRITE_DESC;

private:
	CUI_Sprite(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CUI_Sprite();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	/* Repositions/resizes in place, e.g. a progress bar fill growing toward one edge each frame. */
	void Set_Rect(f32_t fCenterX, f32_t fCenterY, f32_t fSizeX, f32_t fSizeY);

	/* Runtime-only overrides for the JSON UI layout runtime factory (button hover swap, dim
	backdrop alpha, a mirrored corner-frame slot). Default state (white tint, no flip, no
	override texture, non-additive) renders identically to a CUI_Sprite that never calls any of
	these, so existing callers (loading background, progress bar fills) are unaffected. */
	void Set_Tint(const float4_t& vTint);
	void Set_FlipX(bool_t bFlipX);
	void Set_Additive(bool_t bAdditive);
	/* 1.f (default) draws the whole sprite; a value in [0,1) clips texels past that fraction of
	the sprite's own U axis (post-FlipX), for a gauge/health-bar drain that reveals its own art at
	native scale as it empties instead of a stretched/squished resize. */
	void Set_FillRatio(f32_t fFillRatio);
	/* Takes an already-resolved SRV (the caller owns loading/caching -- CUI_Sprite stays a thin
	render primitive, not a second texture cache) and takes over from the prototype-tag texture
	bound at construction for as long as it's set. Pass nullptr to fall back to that original
	texture again. */
	void Set_Texture(ComPtr<ID3D11ShaderResourceView> pOverrideSRV);

private:
	wstring_t						m_strTextureTag;

	shared_ptr<CShader>				m_pShaderCom = { nullptr };
	shared_ptr<CTexture>			m_pTextureCom = { nullptr };
	shared_ptr<CVIBuffer_Rect>		m_pVIBufferCom = { nullptr };

	float4_t						m_vTint = float4_t(1.f, 1.f, 1.f, 1.f);
	bool_t							m_bFlipX = false;
	bool_t							m_bAdditive = false;
	f32_t							m_fFillRatio = 1.f;
	ComPtr<ID3D11ShaderResourceView>	m_pOverrideTextureSRV;

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static unique_ptr<CUI_Sprite> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
