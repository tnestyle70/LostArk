#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CWeapon_LanceMaster final : public CPartObject
{
public:
	typedef struct tagWeaponLanceMasterDesc : public CPartObject::PARTOBJECT_DESC
	{
		/* The body's model owns the skeleton the socket bone belongs to. */
		shared_ptr<Engine::CModel> pSocketModel = { nullptr };
		const char_t* pSocketBoneName = { nullptr };
	} WEAPON_LANCEMASTER_DESC;

private:
	CWeapon_LanceMaster(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CWeapon_LanceMaster();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };
	shared_ptr<CModel> m_pSocketModelCom = { nullptr };
	const char_t* m_pSocketBoneName = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static unique_ptr<CWeapon_LanceMaster> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
