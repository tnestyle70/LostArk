#pragma once

#include "Client_Defines.h"
#include "ContainerObject.h"
#include "NavPathFollower.h"
#include "Network/PacketMessages.h"

#include <string_view>

NS_BEGIN(Engine)
class CModel;
class CNavigation;
class CTransform;
class CCollider;
NS_END

NS_BEGIN(Client)

class CValtan final : public CContainerObject
{
public:
	typedef struct tagValtanDesc : public CContainerObject::CONTAINEROBJECT_DESC
	{
		const tchar_t* pNavigationPrototypeTag = { nullptr };
		uint32_t iPrototypeLevelIndex = {};
		shared_ptr<CTransform> pTargetTransform = { nullptr };
		float3_t vPosition = {};
		f32_t fScale = 1.5f;
		bool_t isServerAuthoritative = false;
		f32_t fCollisionRadius = 0.f;
	} VALTAN_DESC;

	enum VALTAN_STATE
	{
		IDLE = 0x00000001,
		CHASE = 0x00000002,
		PATTERN_WINDUP = 0x00000004,
		PATTERN_ACTIVE = 0x00000008,
		PATTERN_RECOVERY = 0x00000010,
		DEAD = 0x00000020,
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

	uint32_t Get_State() const { return m_iState; }
	PATH_RESULT_CODE Get_PathResult() const { return m_PathFollower.Get_LastResult(); }
	uint32_t Get_PathExpandedNodes() const { return m_PathFollower.Get_LastExpandedNodes(); }
	uint32_t Get_PathWaypointCount() const { return m_PathFollower.Get_NumWaypoints(); }
	bool_t Apply_NetworkState(
		const float3_t& position,
		f32_t yawDegrees,
		LostArk::Shared::WORLD_ENTITY_ACTION action,
		std::string_view actionId);
	const std::string& Get_ServerActionId() const { return m_strServerActionId; }
#ifdef _DEBUG
	void Set_NavigationDebugVisible(bool_t isVisible) { m_isNavigationDebugVisible = isVisible; }
	void Set_CombatColliderDebugVisible(bool_t isVisible) {
		m_isCombatColliderDebugVisible = isVisible;
	}
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
	shared_ptr<Engine::CCollider> m_pColliderCom = { nullptr };
	shared_ptr<CModel> m_pBodyModelCom = { nullptr };
	CNavPathFollower m_PathFollower;
	uint32_t m_iPrototypeLevelIndex = {};
	bool_t m_isServerAuthoritative = false;
	std::string m_strServerActionId;
#ifdef _DEBUG
	bool_t m_isNavigationDebugVisible = { false };
	bool_t m_isCombatColliderDebugVisible = { false };
#endif

private:
	HRESULT Ready_PartObjects();
	HRESULT Ready_Components(f32_t collisionRadius);
	PATH_RESULT_CODE Request_PathToTarget(fvector_t vGoalPosition);
	void Set_ChaseState(bool_t isChasing);

public:
	static unique_ptr<CValtan> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
