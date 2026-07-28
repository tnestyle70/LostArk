#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Instance_Rect;
NS_END

NS_BEGIN(Client)

class CSnow final : public CGameObject
{
private:
	CSnow(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CSnow();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	shared_ptr<CShader>					m_pShaderCom = { nullptr };
	shared_ptr<CTexture>				m_TextureCom = {nullptr};
	shared_ptr<CVIBuffer_Instance_Rect>	m_pVIBufferCom = { nullptr };	

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static unique_ptr<CSnow> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
	
};

NS_END