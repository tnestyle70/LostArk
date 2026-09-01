#pragma once

#include "Client_Defines.h"
#include "ActionPresentationTimeline.h"
#include "AnimationSkillBindingDocument.h"
#include "ContainerObject.h"
#include "DeferredMaterialRenderUtils.h"
#include "NavPathFollower.h"
#include "Network/PacketMessages.h"
#include "ValtanPatternEffectCueDocument.h"
#include "ValtanPatternShakeCueDocument.h"
#include "ValtanPatternSoundCueDocument.h"
#include "ValtanCombatObjectSoundCueDocument.h"
#include "ValtanPresentationGenerationAdmission.h"

#include <algorithm>
#include <cmath>
#include <limits>
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

struct VALTAN_PATTERN_VIEW;

inline constexpr size_t VALTAN_MAX_PATTERN_EFFECT_OCCURRENCES_PER_SCAN = 256u;

struct VALTAN_PATTERN_EFFECT_OCCURRENCE_SCAN_DESC final
{
	f32_t fPreviousActionAgeSeconds = 0.f;
	f32_t fCurrentActionAgeSeconds = 0.f;
	f32_t fFirstOccurrenceWallSeconds = 0.f;
	f32_t fLoopWallDurationSeconds = 0.f;
	f32_t fPlaybackRate = 1.f;
	f32_t fLiveSourceDurationSeconds = 0.f;
	bool_t bHasPreviousActionAge = false;
	bool_t bEachLoop = false;
};

struct VALTAN_PATTERN_EFFECT_OCCURRENCE_SAMPLE final
{
	uint64_t iLoopEpoch = 0u;
	f32_t fOccurrenceWallSeconds = 0.f;
	f32_t fInitialSampleSeconds = 0.f;
};

/* Resolves only occurrences crossed by the accepted snapshot interval.  A
   first snapshot has no previous edge, so it catches up every occurrence that
   is still naturally live.  Large loop jumps retain the latest 256 live
   epochs deterministically instead of walking unbounded history. */
inline bool_t Resolve_ValtanPatternEffectOccurrenceScan(
	const VALTAN_PATTERN_EFFECT_OCCURRENCE_SCAN_DESC& Desc,
	std::vector<VALTAN_PATTERN_EFFECT_OCCURRENCE_SAMPLE>& OutSamples)
{
	OutSamples.clear();
	constexpr double EpsilonSeconds = 0.00001;
	const auto IsFiniteNonNegative = [](const f32_t fValue)
	{
		return std::isfinite(fValue) && fValue >= 0.f;
	};
	if (!IsFiniteNonNegative(Desc.fCurrentActionAgeSeconds) ||
		!IsFiniteNonNegative(Desc.fFirstOccurrenceWallSeconds) ||
		!std::isfinite(Desc.fPlaybackRate) || Desc.fPlaybackRate <= 0.f ||
		!IsFiniteNonNegative(Desc.fLiveSourceDurationSeconds) ||
		(Desc.bHasPreviousActionAge &&
			(!IsFiniteNonNegative(Desc.fPreviousActionAgeSeconds) ||
			 static_cast<double>(Desc.fPreviousActionAgeSeconds) >
				static_cast<double>(Desc.fCurrentActionAgeSeconds) +
				EpsilonSeconds)) ||
		(Desc.bEachLoop &&
			(!std::isfinite(Desc.fLoopWallDurationSeconds) ||
			 Desc.fLoopWallDurationSeconds <= 0.f)))
	{
		return false;
	}
	if (Desc.fLiveSourceDurationSeconds <= 0.f)
		return true;

	const double Current = static_cast<double>(
		Desc.fCurrentActionAgeSeconds);
	const double First = static_cast<double>(
		Desc.fFirstOccurrenceWallSeconds);
	const double PlaybackRate = static_cast<double>(Desc.fPlaybackRate);
	const double LiveSourceDuration = static_cast<double>(
		Desc.fLiveSourceDurationSeconds);
	if (Current + EpsilonSeconds < First)
		return true;

	const auto AppendIfLive = [&](const uint64_t iEpoch,
		const double fOccurrenceWallSeconds)
	{
		const f32_t fInitialSampleSeconds = static_cast<f32_t>((std::max)(
			0.0, (Current - fOccurrenceWallSeconds) * PlaybackRate));
		if (!std::isfinite(fInitialSampleSeconds) ||
			fInitialSampleSeconds >= Desc.fLiveSourceDurationSeconds)
		{
			return;
		}
		OutSamples.push_back({ iEpoch,
			static_cast<f32_t>(fOccurrenceWallSeconds),
			fInitialSampleSeconds });
	};

	if (!Desc.bEachLoop)
	{
		if (Desc.bHasPreviousActionAge &&
			static_cast<double>(Desc.fPreviousActionAgeSeconds) +
				EpsilonSeconds >= First)
		{
			return true;
		}
		AppendIfLive(0u, First);
		return true;
	}

	const double Loop = static_cast<double>(Desc.fLoopWallDurationSeconds);
	const double UpperQuotient = std::floor(
		(Current + EpsilonSeconds - First) / Loop);
	if (!std::isfinite(UpperQuotient) || UpperQuotient < 0.0 ||
		UpperQuotient >= static_cast<double>(
			(std::numeric_limits<uint64_t>::max)()))
	{
		return false;
	}
	const uint64_t iUpperEpoch = static_cast<uint64_t>(UpperQuotient);
	uint64_t iLowerEpoch = 0u;
	if (Desc.bHasPreviousActionAge)
	{
		const double Previous = (std::min)(Current,
			static_cast<double>(Desc.fPreviousActionAgeSeconds));
		if (Previous + EpsilonSeconds >= First)
		{
			const double PreviousQuotient = std::floor(
				(Previous + EpsilonSeconds - First) / Loop);
			if (!std::isfinite(PreviousQuotient) || PreviousQuotient < 0.0 ||
				PreviousQuotient >= static_cast<double>(
					(std::numeric_limits<uint64_t>::max)()))
			{
				return true;
			}
			iLowerEpoch = static_cast<uint64_t>(PreviousQuotient) + 1u;
		}
	}

	/* initialSample < liveDuration is equivalent to occurrence wall time being
	   strictly newer than this threshold.  Deriving the first live epoch avoids
	   an O(snapshot-age / loop-duration) scan. */
	const double LiveThreshold = Current - LiveSourceDuration / PlaybackRate;
	if (LiveThreshold >= First)
	{
		const double LiveQuotient = std::floor(
			(LiveThreshold - First) / Loop);
		if (!std::isfinite(LiveQuotient) || LiveQuotient < 0.0 ||
			LiveQuotient >= static_cast<double>(
				(std::numeric_limits<uint64_t>::max)()))
		{
			return true;
		}
		iLowerEpoch = (std::max)(iLowerEpoch,
			static_cast<uint64_t>(LiveQuotient) + 1u);
	}
	if (iLowerEpoch > iUpperEpoch)
		return true;

	const uint64_t iLatestBoundedEpoch =
		iUpperEpoch >= VALTAN_MAX_PATTERN_EFFECT_OCCURRENCES_PER_SCAN - 1u ?
		iUpperEpoch -
			(VALTAN_MAX_PATTERN_EFFECT_OCCURRENCES_PER_SCAN - 1u) : 0u;
	iLowerEpoch = (std::max)(iLowerEpoch, iLatestBoundedEpoch);
	OutSamples.reserve(static_cast<size_t>(iUpperEpoch - iLowerEpoch + 1u));
	for (uint64_t iEpoch = iLowerEpoch;; ++iEpoch)
	{
		AppendIfLive(iEpoch, First + static_cast<double>(iEpoch) * Loop);
		if (iEpoch == iUpperEpoch)
			break;
	}
	return true;
}

class CValtan final : public CContainerObject
{
public:
	static constexpr const tchar_t* BODY_PART_TAG = TEXT("Part_Body");
	static constexpr const tchar_t* WEAPON_PART_TAG = TEXT("Part_Weapon_R");
	static constexpr const char_t* WEAPON_SOCKET_BONE = "b_wp_r_01";
	/* Armour parts are authored on the body rig, so they are skinned parts
	with no socket bone. The stable state mask, never array order, joins them
	to Server-owned alive-part state. */
	static wstring_t Build_ArmorModelPrototypeTag(
		uint32_t iStateMask, std::string_view archetypeId = "BOSS_VALTAN");
	static wstring_t Build_ArmorPartTag(uint32_t iStateMask);

	typedef struct tagValtanDesc : public CContainerObject::CONTAINEROBJECT_DESC
	{
		const tchar_t* pNavigationPrototypeTag = { nullptr };
		uint32_t iPrototypeLevelIndex = {};
		shared_ptr<CTransform> pTargetTransform = { nullptr };
		float3_t vPosition = {};
		f32_t fScale = {};
		bool_t isServerAuthoritative = false;
		std::string strArchetypeId = "BOSS_VALTAN";
		LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
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

	/* Presentation-only copy of the Server-selected target pose from the same
	   S2C_WORLD_SNAPSHOT that carries the boss pattern clock.  A valid target
	   identity with bHasFinitePose=false is intentional: the target was missing
	   from that snapshot, so target-anchored cue occurrences must be isolated. */
	struct PATTERN_TARGET_SNAPSHOT_POSE final
	{
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		float3_t vPosition = {};
		f32_t fYawDegrees = 0.f;
		bool_t bHasFinitePose = false;
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
	// Reliable DEAD despawns can arrive without the final snapshot.
	bool_t Begin_NetworkDeathPresentation();
	bool_t Is_NetworkDeathPresentationComplete() const;
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
	/* The world entity snapshot reports a plate that lost its durability as
	   one bit per authored plate index, and presentation only has to hide
	   the part wearing that index. */
	bool_t Apply_BrokenArmorMask(uint8_t iBrokenArmorMask);
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
		const PATTERN_TARGET_SNAPSHOT_POSE& PatternTargetPose);
	/* Animation Tool-only local audition.  It deliberately bypasses network,
	   Effect, Sound, hit and movement, but samples the same admitted Product
	   binding through the exact helper Apply_NetworkState uses. */
	bool_t Apply_LocalPatternPresentationSample(
		LostArk::Shared::WORLD_ENTITY_ACTION patternAction,
		std::string_view actionId,
		f32_t fActionAgeSeconds,
		bool_t bForceAnimationEdge);
	/* Action Composition-only draft mirror. It never replaces Product caches
	   and is accepted only by a non-authoritative preview boss. */
	bool_t Stage_LocalPatternAuthoringPreview(
		const VALTAN_PATTERN_VIEW& Pattern,
		std::string& strOutStatus);
	/* Effect Tool-only combat-object clock. The staged Product topology is
	   reused, but boss animation, Product cues, hit debug, and Effect V2 stage
	   playback remain untouched so an independent world-root lifecycle can be
	   inspected against the paused IDLE clone. */
	bool_t Apply_LocalCombatObjectAuthoringPreviewSample(
		std::string_view actionId,
		f32_t fActionAgeSeconds,
		bool_t bResetTransport,
		std::string& strOutStatus);
	/* Rewinds only tool-owned presentation instances while preserving the
	   staged authoring draft.  Used before an explicit Sequencer seek/loop. */
	void Reset_LocalPatternPreviewTransport();
	void Reset_LocalPatternPresentationSample();
	bool_t Apply_BossCombatState(
		const LostArk::Shared::BOSS_COMBAT_SNAPSHOT& state);
	bool_t Apply_BossCombatEvent(
		const LostArk::Shared::BOSS_COMBAT_EVENT& event);
	bool_t Apply_CombatObjectPresentationEvent(
		const LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event,
		std::string& strOutStatus);
	/* Debug Workbench Save reloads only Client presentation documents. Server
	   hit identity and gameplay timing remain untouched. Each reload stages the
	   complete replacement and keeps the previously admitted map on failure. */
	bool_t Reload_PatternBindings(std::string& strOutStatus);
	bool_t Reload_PatternPresentationAuthoring(std::string& strOutStatus);
	bool_t Reload_PatternPresentationAuthoring(
		const LostArk::Shared::GameplayDataRevision& ExpectedServerRevision,
		const Client::VALTAN_PRESENTATION_GENERATION_RECEIPT& ExpectedReceipt,
		std::string& strOutStatus);
	bool_t Reload_PatternSoundCues(std::string& strOutStatus);
	bool_t Reload_CombatObjectSoundCues(std::string& strOutStatus);
	const Client::VALTAN_PATTERN_SOUND_SOURCE_RECEIPT&
		Get_PatternSoundSourceReceipt() const
	{
		return m_PatternSoundSourceReceipt;
	}
	const Client::VALTAN_PRESENTATION_GENERATION_RECEIPT&
		Get_PresentationGenerationReceipt() const
	{
		return m_PresentationGenerationReceipt;
	}
	const std::string& Get_ServerActionId() const { return m_strServerActionId; }
#ifdef _DEBUG
	/* Process-local visual A/B only.  Server pattern timing and the Product V0
	   cue document remain authoritative in both modes. */
	static void Set_PatternEffectV1AuditionEnabled(bool_t bEnabled);
	static bool_t Is_PatternEffectV1AuditionEnabled();
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
	enum class RAID_BGM_STATE : uint8_t
	{
		NONE,
		M05_INTRO,
		M06_PHASE_ONE,
		M07_GHOST_TRANSITION,
		M08_GHOST_PHASE,
		M09_DEATH,
	};

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
	/* The Server keeps this primary boss alive and damage-authoritative while
	one phase-three relocation snapshot suppresses only its part render queues. */
	bool_t m_isGhostPresentationHidden = false;
	uint8_t m_iBrokenArmorMask = 0u;
	std::uint64_t m_iLastBossCombatEventSequence = 0u;
	DEFERRED_EMISSIVE_OVERRIDE m_HitFlash;
	f32_t m_fHitFlashRemainingSeconds = { 0.f };
	CNavPathFollower m_PathFollower;
	uint32_t m_iPrototypeLevelIndex = {};
	bool_t m_isServerAuthoritative = false;
	std::string m_strArchetypeId = "BOSS_VALTAN";
	/* Gameplay/network identity stays m_strArchetypeId.  This second ID names
	   only the atomically committed body/weapon/armour presentation group. */
	std::string m_strPresentationPartArchetypeId = "BOSS_VALTAN";
	LostArk::Shared::NET_ENTITY_ID m_iOwnerBossNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	bool_t m_isRaidBgmEnabled = false;
	uint64_t m_iRaidBgmOwnershipGeneration = 0u;
	CDeathPresentationClock m_DeathPresentationClock;
	uint32_t m_iDeathAnimationIndex = (std::numeric_limits<uint32_t>::max)();
	bool_t m_hasObservedEntrancePattern = false;
	RAID_BGM_STATE m_eRaidBgmState = RAID_BGM_STATE::NONE;
	/* Presentation-only snapshot buffer. Apply_NetworkState commits the Server
	pattern/action clock immediately; Update consumes only these transforms. */
	struct NETWORK_TRANSFORM_SAMPLE
	{
		uint32_t iServerTick = 0u;
		float3_t vPosition = {};
		f32_t fYawDegrees = 0.f;
	};
	static constexpr std::size_t NETWORK_SAMPLE_CAPACITY = 8u;
	NETWORK_TRANSFORM_SAMPLE m_NetworkSamples[NETWORK_SAMPLE_CAPACITY] = {};
	std::size_t m_iNetworkSampleCount = 0u;
	f32_t m_fPlaybackServerTick = 0.f;
	f32_t m_fPresentationYawDegrees = 0.f;
	bool_t m_hasNetworkTransformState = false;
	std::string m_strServerPatternId;
	std::string m_strServerActionId;
	uint32_t m_iLastServerTick = 0u;
	uint32_t m_iServerActionStartTick = 0u;
	uint32_t m_iServerPatternSequence = 0u;
	uint32_t m_iServerPatternStageIndex = 0u;
	f32_t m_fServerActionAgeSeconds = 0.f;
	// Authoritative facing captured once per occurrence, never the interpolated visual yaw.
	f32_t m_fServerPatternFacingYawDegrees = 0.f;
	// Latest accepted authoritative boss yaw. Dynamic arena roots consume this
	// instead of the interpolated presentation transform.
	f32_t m_fServerPatternCurrentYawDegrees = 0.f;
	/* Target virtual anchors never read the interpolated Character transform.
	   These fields are replaced from each accepted snapshot and remain usable
	   only while both the locked identity and pattern sequence still match. */
	LostArk::Shared::NET_ENTITY_ID m_iServerPatternTargetNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	uint32_t m_iServerPatternTargetPoseSequence = 0u;
	float3_t m_vServerPatternTargetSnapshotPosition = {};
	f32_t m_fServerPatternTargetSnapshotYawDegrees = 0.f;
	bool_t m_bHasServerPatternTargetSnapshotPose = false;
	bool_t m_bServerPatternTargetIdentityStable = false;
	/* One composite invocation owns one root handle.  The Server pattern keeps
	   the target identity fixed while current yaw changes per fixed tick; late
	   Effect elements therefore inherit the same updated root rather than
	   rebuilding independent element transforms. */
	struct PATTERN_TARGET_FOLLOW_EFFECT_ROOT final
	{
		uint64_t iWorldRootHandle = 0u;
		uint32_t iPatternSequence = 0u;
		LostArk::Shared::NET_ENTITY_ID iTargetNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		float3_t vArenaCenter = {};
		EFFECT_TRANSFORM_DESC LocalTransform{};
		VALTAN_PATTERN_EFFECT_SCALE_POLICY eScalePolicy =
			VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
		float3_t vWorldScale{ 1.f, 1.f, 1.f };
	};
	std::vector<PATTERN_TARGET_FOLLOW_EFFECT_ROOT>
		m_PatternTargetFollowEffectRoots;
	std::size_t m_iPatternPresentationClipOccurrenceIndex =
		(std::numeric_limits<std::size_t>::max)();
	/* Presentation only: pattern stage actionId -> ordered original clip
	chain, from Data/Animation/Authored/Valtan/Valtan.patternbindings.json. A
	present empty chain is the explicit NONE variant and holds the preceding
	pose; a missing action still falls back to the catalog's generic clip. A
	missing/corrupt document never blocks the spawn. */
	std::unordered_map<std::string,
		std::vector<BOSS_PATTERN_ANIMATION_CLIP>>
		m_PatternClipByActionId;
	bool_t m_bLocalPatternAuthoringPreview = false;
	std::unordered_map<std::string,
		std::vector<BOSS_PATTERN_ANIMATION_CLIP>>
		m_LocalPreviewClipByActionId;
	std::unordered_map<std::string,
		std::vector<VALTAN_PATTERN_EFFECT_CUE>>
		m_LocalPreviewEffectCuesByActionId;
	std::unordered_map<std::string, uint32_t>
		m_LocalPreviewStageIndexByActionId;
	std::unordered_map<std::string, float3_t>
		m_LocalPreviewArenaCenterAnchors;
	struct LOCAL_PATTERN_COMBAT_OBJECT_EVENT final
	{
		std::string strPresentationEventId;
		uint32_t iAtMs = 0u;
	};
	struct LOCAL_PATTERN_COMBAT_OBJECT_TEMPLATE final
	{
		std::string strCombatObjectArchetypeId;
		std::string strClientVisualId;
		std::string strActiveEffectAssetId;
		std::string strTerminalEffectAssetId;
		uint32_t iCount = 0u;
		f32_t fRadiusM = 0.f;
		f32_t fStartAngleDegrees = 0.f;
		f32_t fAngleStepDegrees = 0.f;
		uint32_t iLifetimeMs = 0u;
		std::vector<LOCAL_PATTERN_COMBAT_OBJECT_EVENT> PresentationEvents;
	};
	struct LOCAL_PATTERN_COMBAT_OBJECT_INSTANCE final
	{
		LOCAL_PATTERN_COMBAT_OBJECT_TEMPLATE Template;
		uint32_t iOrdinal = 0u;
		float3_t vPosition = {};
		f32_t fYawDegrees = 0.f;
		uint64_t iActiveHandle = 0u;
		std::vector<uint64_t> TerminalHandles;
		std::vector<bool_t> TerminalAttempts;
	};
	std::unordered_map<std::string,
		std::vector<LOCAL_PATTERN_COMBAT_OBJECT_TEMPLATE>>
		m_LocalPreviewCombatObjectsByActionId;
	std::vector<LOCAL_PATTERN_COMBAT_OBJECT_INSTANCE>
		m_LocalPreviewCombatObjectInstances;
	std::string m_strLocalPreviewCombatObjectActionId;
	std::string m_strLocalPreviewCombatObjectStatus;
	std::string m_strLocalPreviewPatternId;
	std::string m_strLocalPreviewActionId;
	uint32_t m_iLocalPreviewStageIndex = 0u;
	uint32_t m_iLocalPreviewEffectGeneration = 0u;
	/* Product presentation only: exact authoritative stage actionId -> Effect
	   cues.  This map is replaced only after the cue document, encounter join,
	   runtime catalog and root/bone anchors all validate.  Animation bindings
	   are an independent optional presentation registry. */
	std::unordered_map<std::string,
		std::vector<VALTAN_PATTERN_EFFECT_CUE>> m_PatternEffectCuesByActionId;
	// Captured from the same admitted pattern view as the Product cue document.
	std::unordered_map<std::string, float3_t> m_PatternArenaCenterAnchors;
	std::unordered_set<std::string> m_AttemptedPatternEffectOccurrenceKeys;
	bool_t m_bPatternEffectCueScanAgeValid = false;
	f32_t m_fPatternEffectCueScanAgeSeconds = 0.f;
	/* Same role as m_PatternEffectCuesByActionId/m_AttemptedPatternEffectOccurrenceKeys
	   above, mirrored for boss voice/impact Sound cues instead of Effect spawns --
	   see CValtanPatternSoundCueDocument's own header comment for why this is a
	   separate, smaller validated map rather than reusing the Effect one. */
	std::unordered_map<std::string,
		std::vector<VALTAN_PATTERN_SOUND_CUE>> m_PatternSoundCuesByActionId;
	/* Commits with m_PatternSoundCuesByActionId inside the joined presentation
	   transaction. Boss Tool compares it with a locked current-source receipt
	   before any Server playback command is emitted. */
	Client::VALTAN_PATTERN_SOUND_SOURCE_RECEIPT
		m_PatternSoundSourceReceipt;
	/* This receipt binds the generated/read-only core closure to Server R.
	   Pattern Sound keeps the independent typed source receipt above. */
	Client::VALTAN_PRESENTATION_GENERATION_RECEIPT
		m_PresentationGenerationReceipt;
	std::unordered_set<std::string> m_AttemptedPatternSoundOccurrenceKeys;
	bool_t m_bPatternSoundCueScanAgeValid = false;
	f32_t m_fPatternSoundCueScanAgeSeconds = 0.f;
	std::unordered_map<std::string, VALTAN_COMBAT_OBJECT_SOUND_CUE>
		m_CombatObjectSoundCuesBySource;
	std::uint64_t m_iLastCombatObjectPresentationEventSequence = 0u;
	/* Boss camera-shake cues, same shape as the Sound cue registry. Every
	   client that presents the boss feels its shakes; they are not gated on a
	   locally controlled owner like player skill shakes. */
	std::unordered_map<std::string,
		std::vector<VALTAN_PATTERN_SHAKE_CUE>> m_PatternShakeCuesByActionId;
	std::unordered_set<std::string> m_AttemptedPatternShakeOccurrenceKeys;
	bool_t m_bPatternShakeCueScanAgeValid = false;
	f32_t m_fPatternShakeCueScanAgeSeconds = 0.f;
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
		uint32_t iHitDelayMs = 0u;
		uint32_t iStageDurationMs = 0u;
		std::vector<uint32_t> HitOffsetsMs;
		/* Separate player-attack -> boss hurt proxy.  It is active for the
		   whole WINDUP Stage and must never be presented as a boss damage hit. */
		bool_t bHasCounterProxy = false;
		std::string strCounterProxyKind = "BOSS_LOCAL_CIRCLE";
		f32_t fCounterProxyForwardOffsetM = 0.f;
		f32_t fCounterProxyRightOffsetM = 0.f;
		f32_t fCounterProxyRadiusM = 0.f;
		f32_t fCounterProxyArcDegrees = 0.f;
	};
	bool_t m_isNavigationDebugVisible = { false };
	bool_t m_isCombatColliderDebugVisible = { false };
	bool_t m_isPatternHitAreaDebugVisible = { true };
	bool_t m_isPatternHitAreaDebugLoadAttempted = { false };
	std::unordered_map<std::string, PATTERN_HIT_AREA_DEBUG>
		m_PatternHitAreaByActionId;
	std::unordered_map<std::string, PATTERN_HIT_AREA_DEBUG>
		m_LocalPreviewHitAreaByActionId;
	std::string m_strPreviewHitActionId;
	f32_t m_fPreviewHitAgeSeconds = { 0.f };
#endif

private:
	bool_t Reload_PatternPresentationAuthoring_Impl(
		const LostArk::Shared::GameplayDataRevision* pExpectedServerRevision,
		const Client::VALTAN_PRESENTATION_GENERATION_RECEIPT* pExpectedReceipt,
		std::string& strOutStatus);
	HRESULT Ready_PartObjects();
	void Ready_ArmorParts();
	bool_t Replace_PresentationPartGroup(
		std::string_view presentationArchetypeId,
		std::string& strOutStatus);
	/* Hides exactly the plates the Server reports broken. Presentation never
	decides this: a plate comes off because durability reached zero. */
	void Set_ArmorPartVisible(uint32_t iStateMask, bool_t isVisible);
	void Refresh_ArmorPartVisibility();
	HRESULT Ready_Components(f32_t collisionRadius);
	void Load_PatternBindings();
	bool_t Reload_PatternBindings_WhileAdmitted(std::string& strOutStatus);
	bool_t Apply_PatternPresentationSample(
		std::string_view actionId,
		std::string_view fallbackClipName,
		f32_t fActionAgeSeconds,
		bool_t bAnimationEdgeChanged,
		std::size_t iCurrentClipOccurrenceIndex,
		std::size_t& iOutClipOccurrenceIndex);
	void Load_PatternEffectCues();
	bool_t Reload_PatternEffectCues_WhileAdmitted(std::string& strOutStatus);
	void Spawn_DuePatternEffectCues(f32_t fActionAgeSeconds);
	void Update_PatternTargetFollowEffectRoots();
	void Detach_PatternTargetFollowEffectRoots();
	bool_t Sync_LocalPatternCombatObjectPreview(
		std::string_view actionId,
		f32_t fActionAgeSeconds,
		std::string& strOutStatus);
	void Stop_LocalPatternCombatObjectPreview();
	void Load_PatternSoundCues();
	bool_t Reload_PatternSoundCues_WhileAdmitted(std::string& strOutStatus);
	void Spawn_DuePatternSoundCues(f32_t fActionAgeSeconds);
	void Load_CombatObjectSoundCues();
	bool_t Reload_CombatObjectSoundCues_WhileAdmitted(
		std::string& strOutStatus);
	void Load_PatternShakeCues();
	bool_t Reload_PatternShakeCues_WhileAdmitted(std::string& strOutStatus);
	void Spawn_DuePatternShakeCues(f32_t fActionAgeSeconds);
#ifdef _DEBUG
	void Load_PatternHitAreaDebug();
	void Draw_PatternHitAreaDebug() const;
#endif
	PATH_RESULT_CODE Request_PathToTarget(fvector_t vGoalPosition);
	void Set_ChaseState(bool_t isChasing);
	void Queue_NetworkTransformSample(
		const float3_t& position,
		f32_t yawDegrees,
		uint32_t iServerTick);
	void Update_NetworkTransform(f32_t fTimeDelta);
	void Update_RaidBgm(
		LostArk::Shared::WORLD_ENTITY_ACTION action,
		std::string_view patternId,
		std::string_view actionId);
	void Transition_RaidBgm(RAID_BGM_STATE nextState);

public:
	static unique_ptr<CValtan> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
