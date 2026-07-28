#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CBackGround final : public CUIObject
{
public:
	typedef struct tagBackGroundDesc : public CUIObject::UIOBJECT_DESC
	{

	}BACKGROUND_DESC;

private:
	CBackGround(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CBackGround();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	shared_ptr<CShader>				m_pShaderCom = { nullptr };
	shared_ptr<CTexture>			m_pTextureCom = { nullptr };
	shared_ptr<CVIBuffer_Rect>		m_pVIBufferCom = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static unique_ptr<CBackGround> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
	
};

NS_END