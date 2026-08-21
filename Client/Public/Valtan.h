#pragma once

#include "Client_Defines.h"
#include "ContainerObject.h"
#include "DeferredMaterialRenderUtils.h"
#include "NavPathFollower.h"
#include "Network/PacketMessages.h"
#include "ValtanPatternEffectCueDocument.h"

#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
	static constexpr const tchar_t* BODY_PART_TAG = TEXT("Part_Body");
	static constexpr const tchar_t* WEAPON_PART_TAG = TEXT("Part_Weapon_R");
	static constexpr const char_t* WEAPON_SOCKET_BONE = "b_wp_r_01";
	static constexpr f32_t MODEL_VIEW_SCALE = 1.f;
	/* Armour parts are authored on the body rig, so they are skinned parts
	with no socket bone. The stable state mask, never array order, joins them
	to Server-owned alive-part state. */
	static wstring_t Build_ArmorModelPrototypeTag(uint32_t iStateMask);
	static wstring_t Build_ArmorPartTag(uint32_t iStateMask);

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

	void Trigger_HitFlash();
	uint32_t Get_State() const { return m_iState; }
	PATH_RESULT_CODE Get_PathResult() const { return m_PathFollower.Get_LastResult(); }
	uint32_t Get_PathExpandedNodes() const { return m_PathFollower.Get_LastExpandedNodes(); }
	uint32_t Get_PathWaypointCount() const { return m_PathFollower.Get_NumWaypoints(); }
	shared_ptr<Engine::CModel> Get_BodyModel() const {
		return m_pBodyModelCom;
	}
	shared_ptr<Engine::CTransform> Get_Transform() const {
		return m_pTransformCom;
	}
	/* Effect/animation authoring must use the same visual root that the
	   socketed weapon consumes: Valtan body local (-90 degree source-axis
	   correction) composed with the owning actor world transform. */
	bool_t Try_Get_PresentationRootMatrix(float4x4_t* pOut) const;
	bool_t Apply_NetworkState(
		const float3_t& position,
		f32_t yawDegrees,
		LostArk::Shared::WORLD_ENTITY_ACTION action,
		std::string_view patternId,
		std::string_view actionId,
		uint32_t iServerTick,
		uint32_t iActionStartTick,
		uint32_t iPatternSequence,
		uint32_t iPatternStageIndex);
	bool_t Apply_BossCombatState(
		const LostArk::Shared::BOSS_COMBAT_SNAPSHOT& state);
	bool_t Apply_BossCombatEvent(
		const LostArk::Shared::BOSS_COMBAT_EVENT& event);
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
	shared_ptr<Engine::CTransform> m_pBodyVisualRootCom = { nullptr };
	/* A failed plate load leaves its stable mask absent instead of shifting
	other plates onto a different wire bit. */
	std::unordered_map<uint32_t, wstring_t> m_ArmorPartTagsByStateMask;
	LostArk::Shared::BOSS_COMBAT_SNAPSHOT m_BossCombatState;
	bool_t m_hasBossCombatState = false;
	std::uint64_t m_iLastBossCombatEventSequence = 0u;
	DEFERRED_EMISSIVE_OVERRIDE m_HitFlash;
	f32_t m_fHitFlashRemainingSeconds = { 0.f };
	CNavPathFollower m_PathFollower;
	uint32_t m_iPrototypeLevelIndex = {};
	bool_t m_isServerAuthoritative = false;
	std::string m_strServerPatternId;
	std::string m_strServerActionId;
	uint32_t m_iLastServerTick = 0u;
	uint32_t m_iServerActionStartTick = 0u;
	uint32_t m_iServerPatternSequence = 0u;
	uint32_t m_iServerPatternStageIndex = 0u;
	f32_t m_fServerActionAgeSeconds = 0.f;
	/* Presentation only: pattern stage actionId -> original clip, from
	Data/Animation/Authored/Valtan/Valtan.patternbindings.json. A missing or
	corrupt document leaves this empty and every pattern falls back to the
	catalog's generic clips; it never blocks the spawn. */
	std::unordered_map<std::string, std::string> m_PatternClipByActionId;
	/* Product presentation only: exact authoritative stage actionId -> Effect
	   cues.  This map is replaced only after the cue document, encounter join,
	   runtime catalog and root/bone anchors all validate.  Animation bindings
	   are an independent optional presentation registry. */
	std::unordered_map<std::string,
		std::vector<VALTAN_PATTERN_EFFECT_CUE>> m_PatternEffectCuesByActionId;
	std::unordered_set<std::string> m_SpawnedPatternEffectBindingIds;
#ifdef _DEBUG
	bool_t m_isNavigationDebugVisible = { false };
	bool_t m_isCombatColliderDebugVisible = { false };
#endif

private:
	HRESULT Ready_PartObjects();
	void Ready_ArmorParts();
	void Set_ArmorPartVisible(uint32_t iStateMask, bool_t isVisible);
	HRESULT Ready_Components(f32_t collisionRadius);
	void Load_PatternBindings();
	void Load_PatternEffectCues();
	void Spawn_DuePatternEffectCues(f32_t fActionAgeSeconds);
	PATH_RESULT_CODE Request_PathToTarget(fvector_t vGoalPosition);
	void Set_ChaseState(bool_t isChasing);

public:
	static unique_ptr<CValtan> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
