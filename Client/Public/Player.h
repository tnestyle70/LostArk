#pragma once

/* 플레이어를 구성하는 요소들을 들고 있는다. */
#include "Client_Defines.h"
#include "ContainerObject.h"

NS_BEGIN(Engine)
class CCollider;
class CNavigation;
NS_END

NS_BEGIN(Client)

class CPlayer final : public CContainerObject
{
public:
	typedef struct tagPlayerDesc : public CContainerObject::CONTAINEROBJECT_DESC
	{

	}PLAYER_DESC;

	enum PLAYER_STATE {
		IDLE = 0x00000001, 
		WALK = 0x00000002,
		ATTACK = 0x00000004,		
	};

private:
	CPlayer(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CPlayer();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	uint32_t				m_iState = {};

private:
	shared_ptr<CNavigation>		m_pNavigationCom = { nullptr };
	shared_ptr<CCollider>		m_pColliderCom = { nullptr };


private:
	HRESULT Ready_PartObjects();
	HRESULT Ready_Components();


public:
	static unique_ptr<CPlayer> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END