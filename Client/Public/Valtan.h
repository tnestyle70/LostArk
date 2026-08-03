#pragma once

#include "Client_Defines.h"
#include "ContainerObject.h"
#include "NavPathFollower.h"

NS_BEGIN(Engine)
class CNavigation;
class CTransform;
NS_END

NS_BEGIN(Client)

class CBody_Valtan;

class CValtan final : public CContainerObject
{
public:
	typedef struct tagValtanDesc :
		public CContainerObject::CONTAINEROBJECT_DESC
	{
		const tchar_t* pNavigationPrototypeTag = { nullptr };
		shared_ptr<CTransform> pTargetTransform = { nullptr };
		float3_t vPosition = {};
		f32_t fScale = 1.5f;
		/* Which level registered the boss prototypes. Defaults to the level
		that has always owned them so existing callers keep working. */
		uint32_t iPrototypeLevelIndex = { ETOUI(LEVEL::ASSET_TEST) };
	} VALTAN_DESC;

	/* Mutually exclusive: the boss cannot chase and run a pattern at once, so
	this is a plain enum rather than a bit mask. */
	enum class VALTAN_STATE : uint32_t
	{
		IDLE,
		CHASE,
		PATTERN,
	};

	/* PATTERN is the behaviour state; this names the pattern running inside it.
	Only one exists so far, but the two mean different things. */
	enum class VALTAN_PATTERN : uint32_t
	{
		NONE,
		AXE_COMBO,
	};

private:
	CValtan(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CValtan();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	VALTAN_STATE Get_State() const {
		return m_eState;
	}
	VALTAN_PATTERN Get_Pattern() const {
		return m_ePattern;
	}
	uint32_t Get_PatternStep() const {
		return m_iPatternStep;
	}
	PATH_RESULT_CODE Get_PathResult() const {
		return m_PathFollower.Get_LastResult();
	}
	uint32_t Get_PathExpandedNodes() const {
		return m_PathFollower.Get_LastExpandedNodes();
	}
	uint32_t Get_PathWaypointCount() const {
		return m_PathFollower.Get_NumWaypoints();
	}

#ifdef _DEBUG
	void Set_NavigationDebugVisible(bool_t isVisible) {
		m_isNavigationDebugVisible = isVisible;
	}
#endif

private:
	VALTAN_STATE m_eState = { VALTAN_STATE::IDLE };
	VALTAN_PATTERN m_ePattern = { VALTAN_PATTERN::NONE };
	uint32_t m_iPatternStep = {};

	f32_t m_fMoveSpeed = { 3.f };
	f32_t m_fRepathTime = {};
	f32_t m_fStopDistance = { 2.5f };
	/* First-pass gameplay values. They move to the tuning data once the feel is
	confirmed on screen. */
	f32_t m_fAttackDistance = { 3.f };
	f32_t m_fPatternInterval = { 2.f };
	f32_t m_fPatternCooldown = {};

	bool_t m_hasLastPathGoal = { false };
	float3_t m_vLastPathGoal = {};
	wstring_t m_strNavigationPrototypeTag;
	uint32_t m_iPrototypeLevelIndex = { ETOUI(LEVEL::ASSET_TEST) };
	weak_ptr<CTransform> m_pTargetTransform;

	shared_ptr<CNavigation> m_pNavigationCom = { nullptr };
	shared_ptr<CBody_Valtan> m_pBodyPart = { nullptr };
	CNavPathFollower m_PathFollower;

#ifdef _DEBUG
	bool_t m_isNavigationDebugVisible = { false };
#endif

private:
	HRESULT Ready_PartObjects();
	HRESULT Ready_Components();

	void Update_Chase(
		f32_t fTimeDelta,
		fvector_t vTargetPosition);
	PATH_RESULT_CODE Request_PathToTarget(
		fvector_t vGoalPosition);
	void Stop_Chase();
	void Set_LocomotionState(bool_t isChasing);

	bool_t Begin_AxeCombo(fvector_t vTargetPosition);
	void Update_Pattern();
	bool_t Play_AxeComboStep();
	void Finish_Pattern();

public:
	static unique_ptr<CValtan> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
