#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CCollider;
class CNavigation;
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CMonster final : public CGameObject
{
public:
	typedef struct tagMonsterDesc : public CGameObject::GAMEOBJECT_DESC
	{

	}MONSTER_DESC;

	enum COLLIDER { AABB, OBB, SPHERE, END };

private:
	CMonster(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CMonster();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;

private:
	shared_ptr<CNavigation>			m_pNavigationCom = { nullptr };
	shared_ptr<CShader>				m_pShaderCom = { nullptr };	
	shared_ptr<CModel>				m_pModelCom = { nullptr };
	shared_ptr<CCollider>			m_ColliderCom[COLLIDER::END] = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();
	void Collision_ToPlayer();

public:
	static unique_ptr<CMonster> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
	
};

NS_END