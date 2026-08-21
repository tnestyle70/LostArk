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
	with no socket bone. The index is the authored order in BossCatalog and it
	is the same index the snapshot names a broken plate by. */
	static constexpr uint32_t MAX_ARMOR_PART_COUNT =
		LostArk::Shared::MAX_WORLD_ENTITY_ARMOR_PLATES;
	static wstring_t Build_ArmorModelPrototypeTag(size_t iArmorIndex);
	static wstring_t Build_ArmorPartTag(size_t iArmorIndex);

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
		uint32_t iPatternStageIndex,
		uint8_t iBrokenArmorMask);
	const std::string& Get_ServerActionId() const { return m_strServerActionId; }
#ifdef _DEBUG
	void Set_NavigationDebugVisible(bool_t isVisible) { m_isNavigationDebugVisible = isVisible; }
	void Set_CombatColliderDebugVisible(bool_t isVisible) {
		m_isCombatColliderDebugVisible = isVisible;
	}
	void Set_PatternHitAreaDebugVisible(bool_t isVisible) {
		m_isPatternHitAreaDebugVisible = isVisible;
	}
	/* Animation Tool preview: draws the same pattern hit wires from a
	   tool-driven stage clock instead of the server snapshot. */
	void Set_PatternHitAreaPreview(
		const std::string& stageActionId,
		f32_t fStageAgeSeconds);
	void Clear_PatternHitAreaPreview();
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
	/* Attached armour part tags in authored order. Empty when the boss
	wears none. */
	vector<wstring_t> m_ArmorPartTags;
	/* Last mask this presentation applied, so a steady snapshot does not walk
	the part list every tick. */
	uint8_t m_iAppliedArmorBreakMask = 0u;
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
	/* Presentation only: pattern stage actionId -> ordered original clip
	chain, from Data/Animation/Authored/Valtan/Valtan.patternbindings.json. A
	missing or corrupt document leaves this empty and every pattern falls back
	to the catalog's generic clips; it never blocks the spawn. */
	std::unordered_map<std::string, std::vector<std::string>>
		m_PatternClipByActionId;
	/* Product presentation only: exact authoritative stage actionId -> Effect
	   cues.  This map is replaced only after the cue document, encounter join,
	   runtime catalog and root/bone anchors all validate.  Animation bindings
	   are an independent optional presentation registry. */
	std::unordered_map<std::string,
		std::vector<VALTAN_PATTERN_EFFECT_CUE>> m_PatternEffectCuesByActionId;
	std::unordered_set<std::string> m_SpawnedPatternEffectBindingIds;
#ifdef _DEBUG
	/* Display copy of the encounter stage hit shapes, keyed by the snapshot's
	   stage actionId. The Server owns the judgment; this only mirrors it as a
	   wire, exactly like the player skill hit area debug. */
	struct PATTERN_HIT_AREA_DEBUG
	{
		std::string strHitShape;
		f32_t fOuterRadius = 0.f;
		f32_t fInnerRadius = 0.f;
		f32_t fAngleDegrees = 0.f;
		f32_t fLength = 0.f;
		f32_t fHalfWidth = 0.f;
		uint32_t iHitCount = 0u;
		uint32_t iHitIntervalMs = 0u;
	};
	bool_t m_isNavigationDebugVisible = { false };
	bool_t m_isCombatColliderDebugVisible = { false };
	bool_t m_isPatternHitAreaDebugVisible = { true };
	bool_t m_isPatternHitAreaDebugLoadAttempted = { false };
	std::unordered_map<std::string, PATTERN_HIT_AREA_DEBUG>
		m_PatternHitAreaByActionId;
	std::string m_strPreviewHitActionId;
	f32_t m_fPreviewHitAgeSeconds = { 0.f };
#endif

private:
	HRESULT Ready_PartObjects();
	void Ready_ArmorParts();
	/* Hides exactly the plates the Server reports broken. Presentation never
	decides this: a plate comes off because durability reached zero. */
	void Apply_ArmorBreakState(uint8_t iBrokenArmorMask);
	HRESULT Ready_Components(f32_t collisionRadius);
	void Load_PatternBindings();
	void Load_PatternEffectCues();
	void Spawn_DuePatternEffectCues(f32_t fActionAgeSeconds);
#ifdef _DEBUG
	void Load_PatternHitAreaDebug();
	void Draw_PatternHitAreaDebug() const;
#endif
	PATH_RESULT_CODE Request_PathToTarget(fvector_t vGoalPosition);
	void Set_ChaseState(bool_t isChasing);

public:
	static unique_ptr<CValtan> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
