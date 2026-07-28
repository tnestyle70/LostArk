#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CCollider;
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CWeapon final : public CPartObject
{
public:
	typedef struct tagWeaponDesc: public CPartObject::PARTOBJECT_DESC
	{
		shared_ptr<CModel>	pSocketModel;
		const uint32_t* pParentState = {};
	}WEAPON_DESC;

private:
	CWeapon(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CWeapon();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	shared_ptr<CCollider>			m_pColliderCom = { nullptr };
	shared_ptr<CShader>				m_pShaderCom = { nullptr };	
	shared_ptr<CModel>				m_pModelCom = { nullptr };
	const uint32_t*					m_pParentState = { nullptr };
	shared_ptr<CModel>				m_pSocketModelCom = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static unique_ptr<CWeapon> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
	
};

NS_END