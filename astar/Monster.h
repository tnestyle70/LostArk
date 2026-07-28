#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CNavigation;
class CCollider;
NS_END

NS_BEGIN(Client)

class CMonster final : public CGameObject
{	
private:
	CMonster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonster(const CMonster& Prototype); /* 메모리 복사를 통해 객체를 생성. */
	virtual ~CMonster() = default;

public:
	virtual HRESULT Initialize_Prototype(); /* 원형객체가 만들어질 때 호출되는 함수. 무거운 초기화작업.  */
	virtual HRESULT Initialize(void* pArg); /* 원형을 복제하여 만든 사본객체가 생성시에 호출되는 함수. 추가적인 초기화를 위해 */
	virtual void Update_Priority(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Update_Late(_float fTimeDelta);
	virtual HRESULT Render();

private:
	CModel*				m_pModelCom = { nullptr };	
	CShader*			m_pShaderCom = { nullptr };
	CCollider*			m_pColliderCom[ENUM_TO_UINT(COLLIDER::END)] = {nullptr};
	CNavigation*		m_pNavigationCom = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

	void Collision_ToPlayer();

public:
	static CMonster* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END