#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Instance_Point;
NS_END

NS_BEGIN(Client)

class CExplosion final : public CPartObject
{
public:
	typedef struct tagEffectDesc : public CPartObject::PARTOBJECT_DESC
	{
		shared_ptr<CModel>	pSocketModel;
		const uint32_t* pParentState = {};
	}EFFECT_DESC;
private:
	CExplosion(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CExplosion();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	shared_ptr<CShader>						m_pShaderCom = { nullptr };
	shared_ptr<CTexture>					m_TextureCom = { nullptr };
	shared_ptr<CVIBuffer_Instance_Point>	m_pVIBufferCom = { nullptr };
	const uint32_t*							m_pParentState = { nullptr };
	shared_ptr<CModel>						m_pSocketModelCom = { nullptr };
private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static unique_ptr<CExplosion> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END