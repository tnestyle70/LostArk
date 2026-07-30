#pragma once

#include "Client_Defines.h"
#include "ContainerObject.h"
#include "NavPathFollower.h"

NS_BEGIN(Engine)
class CModel;
class CNavigation;
class CTransform;
NS_END

NS_BEGIN(Client)

class CValtan final : public CContainerObject
{
public:
	typedef struct tagValtanDesc : public CContainerObject::CONTAINEROBJECT_DESC
	{
		const tchar_t* pNavigationPrototypeTag = { nullptr };
		shared_ptr<CTransform> pTargetTransform = { nullptr };
		float3_t vPosition = {};
	} VALTAN_DESC;

	enum VALTAN_STATE
	{
		IDLE = 0x00000001,
		CHASE = 0x00000002,
	};

private:
	CValtan(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CValtan();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	PATH_RESULT_CODE Request_Move(fvector_t vGoalPosition);

	uint32_t Get_State() const { return m_iState; }
	PATH_RESULT_CODE Get_PathResult() const { return m_PathFollower.Get_LastResult(); }
	uint32_t Get_PathExpandedNodes() const { return m_PathFollower.Get_LastExpandedNodes(); }
	uint32_t Get_PathWaypointCount() const { return m_PathFollower.Get_NumWaypoints(); }
#ifdef _DEBUG
	void Set_NavigationDebugVisible(bool_t isVisible) { m_isNavigationDebugVisible = isVisible; }
#endif

private:
	uint32_t m_iState = { VALTAN_STATE::IDLE };
	f32_t m_fMoveSpeed = { 3.f };
	f32_t m_fRepathTime = {};
	f32_t m_fStopDistance = { 2.5f };
	bool_t m_hasLastPathGoal = { false };
	float3_t m_vLastPathGoal = {};
	wstring_t m_strNavigationPrototypeTag;
	weak_ptr<CTransform> m_pTargetTransform;
	shared_ptr<CNavigation> m_pNavigationCom = { nullptr };
	shared_ptr<CModel> m_pBodyModelCom = { nullptr };
	CNavPathFollower m_PathFollower;
#ifdef _DEBUG
	bool_t m_isNavigationDebugVisible = { false };
#endif

private:
	HRESULT Ready_PartObjects();
	HRESULT Ready_Components();
	void Set_ChaseState(bool_t isChasing);

public:
	static unique_ptr<CValtan> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
