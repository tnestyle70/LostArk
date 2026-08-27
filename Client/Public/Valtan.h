#pragma once

#include "Client_Defines.h"
#include "AnimationSkillBindingDocument.h"
#include "ContainerObject.h"
#include "DeferredMaterialRenderUtils.h"
#include "NavPathFollower.h"
#include "Network/PacketMessages.h"
#include "ValtanPatternEffectCueDocument.h"
#include "ValtanPatternSoundCueDocument.h"

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
	static wstring_t Build_ArmorModelPrototypeTag(uint32_t iStateMask);
	static wstring_t Build_ArmorPartTag(uint32_t iStateMask);

	typedef struct tagValtanDesc : public CContainerObject::CONTAINEROBJECT_DESC
	{
		const tchar_t* pNavigationPrototypeTag = { nullptr };
		uint32_t iPrototypeLevelIndex = {};
		shared_ptr<CTransform> pTargetTransform = { nullptr };
		float3_t vPosition = {};
		f32_t fScale = {};
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
		uint32_t iPatternStageIndex);
	/* Animation Tool-only local audition.  It deliberately bypasses network,
	   Effect, Sound, hit and movement, but samples the same admitted Product
	   binding through the exact helper Apply_NetworkState uses. */
	bool_t Apply_LocalPatternPresentationSample(
		LostArk::Shared::WORLD_ENTITY_ACTION patternAction,
		std::string_view actionId,
		f32_t fActionAgeSeconds,
		bool_t bForceAnimationEdge);
	void Reset_LocalPatternPresentationSample();
	bool_t Apply_BossCombatState(
		const LostArk::Shared::BOSS_COMBAT_SNAPSHOT& state);
	bool_t Apply_BossCombatEvent(
		const LostArk::Shared::BOSS_COMBAT_EVENT& event);
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
	uint8_t m_iBrokenArmorMask = 0u;
	std::uint64_t m_iLastBossCombatEventSequence = 0u;
	DEFERRED_EMISSIVE_OVERRIDE m_HitFlash;
	f32_t m_fHitFlashRemainingSeconds = { 0.f };
	CNavPathFollower m_PathFollower;
	uint32_t m_iPrototypeLevelIndex = {};
	bool_t m_isServerAuthoritative = false;
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
	/* Product presentation only: exact authoritative stage actionId -> Effect
	   cues.  This map is replaced only after the cue document, encounter join,
	   runtime catalog and root/bone anchors all validate.  Animation bindings
	   are an independent optional presentation registry. */
	std::unordered_map<std::string,
		std::vector<VALTAN_PATTERN_EFFECT_CUE>> m_PatternEffectCuesByActionId;
	std::unordered_set<std::string> m_AttemptedPatternEffectOccurrenceKeys;
	bool_t m_bPatternEffectCueScanAgeValid = false;
	f32_t m_fPatternEffectCueScanAgeSeconds = 0.f;
	/* Same role as m_PatternEffectCuesByActionId/m_AttemptedPatternEffectOccurrenceKeys
	   above, mirrored for boss voice/impact Sound cues instead of Effect spawns --
	   see CValtanPatternSoundCueDocument's own header comment for why this is a
	   separate, smaller validated map rather than reusing the Effect one. */
	std::unordered_map<std::string,
		std::vector<VALTAN_PATTERN_SOUND_CUE>> m_PatternSoundCuesByActionId;
	std::unordered_set<std::string> m_AttemptedPatternSoundOccurrenceKeys;
	bool_t m_bPatternSoundCueScanAgeValid = false;
	f32_t m_fPatternSoundCueScanAgeSeconds = 0.f;
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
		std::vector<uint32_t> HitOffsetsMs;
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
	void Set_ArmorPartVisible(uint32_t iStateMask, bool_t isVisible);
	void Refresh_ArmorPartVisibility();
	HRESULT Ready_Components(f32_t collisionRadius);
	void Load_PatternBindings();
	bool_t Apply_PatternPresentationSample(
		std::string_view actionId,
		std::string_view fallbackClipName,
		f32_t fActionAgeSeconds,
		bool_t bAnimationEdgeChanged,
		std::size_t iCurrentClipOccurrenceIndex,
		std::size_t& iOutClipOccurrenceIndex);
	void Load_PatternEffectCues();
	void Spawn_DuePatternEffectCues(f32_t fActionAgeSeconds);
	void Load_PatternSoundCues();
	void Spawn_DuePatternSoundCues(f32_t fActionAgeSeconds);
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

public:
	static unique_ptr<CValtan> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
