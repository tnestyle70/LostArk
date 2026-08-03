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

private:
	wstring_t						m_strTextureTag;

	shared_ptr<CShader>				m_pShaderCom = { nullptr };
	shared_ptr<CTexture>			m_pTextureCom = { nullptr };
	shared_ptr<CVIBuffer_Rect>		m_pVIBufferCom = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static unique_ptr<CUI_Sprite> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
