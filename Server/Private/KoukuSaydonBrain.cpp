#include "KoukuSaydonBrain.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>
#include <utility>

namespace
{
	constexpr std::uint64_t SERVER_TICK_HZ = 30u;
	constexpr std::uint64_t MILLISECONDS_PER_SECOND = 1000u;

	std::uint32_t Next_ServerTick(const std::uint32_t tick) noexcept
	{
		return tick == (std::numeric_limits<std::uint32_t>::max)() ?
			1u : tick + 1u;
	}

	std::uint64_t Stage_ElapsedTicks(
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const std::uint32_t serverTick) noexcept
	{
		const std::uint32_t firstTick = boss.iPatternStageFirstEvaluationTick;
		if (0u == firstTick || 0u == serverTick)
			return 0u;
		const std::uint64_t age = serverTick >= firstTick ?
			static_cast<std::uint64_t>(serverTick - firstTick) :
			static_cast<std::uint64_t>(
				(std::numeric_limits<std::uint32_t>::max)() - firstTick) +
			static_cast<std::uint64_t>(serverTick);
		return age + 1u;
	}

	bool Has_ElapsedMilliseconds(
		const std::uint64_t elapsedTicks,
		const std::uint32_t durationMs) noexcept
	{
		return elapsedTicks * MILLISECONDS_PER_SECOND >=
			static_cast<std::uint64_t>(durationMs) * SERVER_TICK_HZ;
	}

	LostArk::Server::SERVER_ENTITY_ACTION To_ServerAction(
		const LostArk::Server::BOSS_PATTERN_STAGE_KIND kind) noexcept
	{
		using namespace LostArk::Server;
		switch (kind)
		{
		case BOSS_PATTERN_STAGE_KIND::WINDUP:
			return SERVER_ENTITY_ACTION::PATTERN_WINDUP;
		case BOSS_PATTERN_STAGE_KIND::ACTIVE:
			return SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
		case BOSS_PATTERN_STAGE_KIND::RECOVERY:
			return SERVER_ENTITY_ACTION::PATTERN_RECOVERY;
		default:
			return SERVER_ENTITY_ACTION::IDLE;
		}
	}

	bool Is_ZeroMotion(
		const LostArk::Server::BOSS_PATTERN_STAGE_MOTION& motion) noexcept
	{
		using namespace LostArk::Server;
		return BOSS_PATTERN_STAGE_MOTION_KIND::NONE == motion.eKind &&
			0u == motion.iRetargetDelayMs && 0.f == motion.fSpeedMps &&
			0.f == motion.fDistance && 0u == motion.iCornerIndex &&
			0.f == motion.fHalfExtentsX && 0.f == motion.fHalfExtentsZ &&
			motion.RootMotion.empty();
	}
}

bool LostArk::Server::CKoukuSaydonBrain::Is_GateOneBoss(
	const LostArk::Shared::WORLD_ID worldId,
	const SERVER_WORLD_ENTITY& boss) noexcept
{
	return LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == worldId &&
		WORLD_BOOTSTRAP_KIND::BOSS == boss.eKind &&
		LostArk::Shared::INVALID_NET_ENTITY_ID == boss.iOwnerBossNetEntityId &&
		KOUKUSAYDON_G1_ENCOUNTER_ID == boss.strEncounterId &&
		KOUKUSAYDON_G1_BOSS_ARCHETYPE_ID == boss.strArchetypeId &&
		KOUKUSAYDON_G1_BOSS_PLACEMENT_ID == boss.strPlacementId;
}

bool LostArk::Server::CKoukuSaydonBrain::Is_ArenaBoss(
	const LostArk::Shared::WORLD_ID worldId,
	const SERVER_WORLD_ENTITY& boss) noexcept
{
	return LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == worldId &&
		WORLD_BOOTSTRAP_KIND::BOSS == boss.eKind &&
		LostArk::Shared::INVALID_NET_ENTITY_ID == boss.iOwnerBossNetEntityId &&
		KOUKUSAYDON_G1_ENCOUNTER_ID == boss.strEncounterId &&
		boss.strArchetypeId.starts_with(
			KOUKUSAYDON_ARENA_BOSS_ARCHETYPE_PREFIX);
}

bool LostArk::Server::CKoukuSaydonBrain::Is_ArenaBossPlacement(
	const LostArk::Shared::WORLD_ID worldId,
	const WORLD_BOOTSTRAP_PLACEMENT& placement) noexcept
{
	return LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == worldId &&
		!placement.isEnabled &&
		WORLD_BOOTSTRAP_KIND::BOSS == placement.eKind &&
		KOUKUSAYDON_G1_ENCOUNTER_ID == placement.strEncounterId &&
		placement.strArchetypeId.starts_with(
			KOUKUSAYDON_ARENA_BOSS_ARCHETYPE_PREFIX);
}

bool LostArk::Server::CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
	const BOSS_PATTERN_DEFINITION& pattern,
	std::string& status)
{
	if (KOUKUSAYDON_G1_ENCOUNTER_ID != pattern.strEncounterId ||
		pattern.strPatternId.empty() ||
		BOSS_PATTERN_SELECTION::AUDITION_ONLY != pattern.eSelection ||
		BOSS_PATTERN_TARGET_POLICY::NONE != pattern.eTargetPolicy ||
		BOSS_PATTERN_AIM_POLICY::NONE != pattern.eAimPolicy ||
		BOSS_PATTERN_MOTION_KIND::NONE != pattern.Motion.eKind ||
		!pattern.Motion.strAnchorId.empty() ||
		0.f != pattern.Motion.fLandingX || 0.f != pattern.Motion.fLandingY ||
		0.f != pattern.Motion.fLandingZ || 0.f != pattern.Motion.fApexHeight ||
		pattern.Motion.bMoveToAnchorBeforeTakeoff ||
		BOSS_PATTERN_FINALE_KIND::NONE != pattern.Finale.eKind ||
		!pattern.Finale.strGhostArchetypeId.empty() ||
		!pattern.Finale.GhostPatternIds.empty() ||
		0.f != pattern.fVerticalOffsetM || pattern.bInvulnerableWhileRunning ||
		pattern.Stages.empty() ||
		pattern.Stages.size() != pattern.iExpectedStageCount)
	{
		status = "KoukuSaydon pattern is not an animation-only audition definition";
		return false;
	}

	std::unordered_set<std::string> stageIds;
	std::unordered_set<std::string> actionIds;
	for (std::size_t index = 0u; index < pattern.Stages.size(); ++index)
	{
		const BOSS_PATTERN_STAGE_DEFINITION& stage = pattern.Stages[index];
		const bool supportedActions = stage.Actions.empty() ||
			(stage.Actions.size() == 1u && !pattern.BossMotion && [&]() {
				const auto& action = stage.Actions.front();
				return action.eTrigger == BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER &&
					action.eKind == BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE &&
					action.strTargetId == "boss.target.pattern" && action.iValue == 1u &&
					action.iDurationMs == 0u && action.eReleaseMode == BOSS_GRABBED_RELEASE_MODE::NONE &&
					action.fReleaseSpeedMps == 0.f && action.fReleaseYawOffsetDegrees == 0.f &&
					action.Volley.ePolicy == BOSS_COMBAT_OBJECT_VOLLEY_POLICY::NONE;
			}());
		const bool supportedKind =
			BOSS_PATTERN_STAGE_KIND::WINDUP == stage.eStageKind ||
			BOSS_PATTERN_STAGE_KIND::ACTIVE == stage.eStageKind ||
			BOSS_PATTERN_STAGE_KIND::RECOVERY == stage.eStageKind;
		const bool validTimeoutBranch = stage.Branches.empty() ||
			(1u == stage.Branches.size() &&
			 BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT == stage.Branches.front().eOutcome &&
			 stage.Branches.front().strNextPatternId.empty() &&
			 stage.Branches.front().strNextActionId ==
				(index + 1u < pattern.Stages.size() ?
					pattern.Stages[index + 1u].strActionId : std::string{}));
		if (stage.strStageId.empty() || stage.strActionId.empty() ||
			!stageIds.insert(stage.strStageId).second ||
			!actionIds.insert(stage.strActionId).second ||
			!supportedKind || 0u == stage.iDurationMs ||
			!stage.strDamageProfileId.empty() ||
			BOSS_PATTERN_HIT_SHAPE::NONE != stage.eHitShape ||
			BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT !=
				stage.eHitAnchorKind ||
			BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE !=
				stage.eHitActivationKind ||
			BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE != stage.ePlayerResponse ||
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE != stage.eAttachmentSlot ||
			BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL != stage.ePartDamagePolicy ||
			stage.bHasCounterProxy ||
			BOSS_PATTERN_COUNTER_PROXY_KIND::NONE != stage.eCounterProxyKind ||
			0.f != stage.fCounterProxyForwardOffsetM ||
			0.f != stage.fCounterProxyRightOffsetM ||
			0.f != stage.fCounterProxyRadiusM ||
			0.f != stage.fCounterProxyArcDegrees ||
			BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE != stage.eBossResponseKind ||
			0u != stage.iBossResponseThreshold || 0.f != stage.fVerticalOffsetM ||
			0.f != stage.fHitOuterRadius || 0.f != stage.fHitInnerRadius ||
			0.f != stage.fHitAngleDegrees || 0.f != stage.fHitLength ||
			0.f != stage.fHitHalfWidth || 0u != stage.iHitCount ||
			0u != stage.iHitIntervalMs || 0u != stage.iHitDelayMs ||
			!stage.HitOffsetsMs.empty() ||
			0.f != stage.fHitAnchorForwardOffsetM ||
			0.f != stage.fHitAnchorRightOffsetM ||
			0.f != stage.fHitAnchorYawOffsetDegrees ||
			0u != stage.iHitActivationStartMs ||
			0u != stage.iHitActivationLifetimeMs ||
			0.f != stage.fPushRangeM || 0u != stage.iPushMs ||
			stage.bKnockdown || 0u != stage.iDownMs || stage.bWallContact ||
			stage.bChargeImpact || stage.bPiercesCover ||
			!Is_ZeroMotion(stage.Motion) || !stage.strPropBreakSetId.empty() ||
			!stage.PropBreakSlotIds.empty() || !supportedActions ||
			!validTimeoutBranch)
		{
			status = "KoukuSaydon pattern contains unsupported combat, motion, action, or branch data";
			return false;
		}
	}
	std::uint64_t patternDurationMs = 0u;
	for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
		patternDurationMs += stage.iDurationMs;
	if (pattern.BossMotion)
	{
		const auto& motion = *pattern.BossMotion;
		bool valid = !pattern.bResetBossToSpawn && !pattern.ResetBossYawDegrees &&
			motion.iStartMs < motion.iEndMs && motion.iEndMs <= patternDurationMs &&
			motion.StartPosition[1] == motion.EndPosition[1] &&
			std::isfinite(motion.fYawDegrees) && std::abs(motion.fYawDegrees) <= 360.f;
		for (std::size_t axis = 0u; axis < 3u; ++axis)
			valid = valid && std::isfinite(motion.StartPosition[axis]) && std::isfinite(motion.EndPosition[axis]) &&
				std::abs(motion.StartPosition[axis]) <= 100000.f && std::abs(motion.EndPosition[axis]) <= 100000.f;
		if (!valid)
		{ status = "KoukuSaydon Boss Motion interval, base height or reset policy is invalid"; return false; }
	}
	std::unordered_set<std::string> triggerIds;
	for (const auto& trigger : pattern.MechanicTriggers)
	{
		if (trigger.strTriggerId.empty() || !triggerIds.insert(trigger.strTriggerId).second ||
			trigger.iStartMs >= patternDurationMs || 0u == trigger.iDurationMs ||
			static_cast<std::uint64_t>(trigger.iStartMs) + trigger.iDurationMs > patternDurationMs ||
			(BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT == trigger.eKind &&
			 (pattern.BossMotion || trigger.strClonePatternId.empty() || trigger.ClockHours.size() != 3u)))
		{
			status = "KoukuSaydon mechanic trigger is invalid";
			return false;
		}
	}
	std::unordered_set<std::string> windowIds;
	for (const BOSS_PATTERN_LOGIC_WINDOW& window : pattern.LogicWindows)
	{
		const std::uint64_t endMs =
			static_cast<std::uint64_t>(window.iStartMs) + window.iDurationMs;
		const bool endTickKind =
			BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS == window.eKind;
		bool valuesValid = true;
		if (((window.bRearmOnExit || window.bRepeatAfterKnockback) && window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA) ||
			(window.bRearmOnExit && window.bRepeatAfterKnockback))
		{ status = "ENTER_AREA accepts one contact repeat policy"; return false; }
		if (window.bRepeatAfterKnockback && (window.OnSuccess.size() != 1u ||
			window.OnSuccess.front().eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE ||
			window.OnSuccess.front().fPushRangeM <= 0.f || window.OnSuccess.front().iPushMs == 0u))
		{ status = "Repeat after knockback requires one damage Success with positive knockback"; return false; }
		switch (window.eKind)
		{
		case BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH:
			valuesValid = window.CardRegions.size() == 8u || (window.iSectorCount >= 2u &&
				window.SectorSymbols.size() == window.iSectorCount && window.fOuterRadiusM > 0.f);
			break;
		case BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP:
		case BOSS_PATTERN_LOGIC_KIND::ENTER_AREA:
			valuesValid = !window.CardRegions.empty();
			break;
		case BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP:
			valuesValid = !window.CardRegions.empty() && !window.strTargetWorldInstanceId.empty() &&
				std::isfinite(window.fTargetWorldX) && std::isfinite(window.fTargetWorldZ) &&
				std::abs(window.fTargetWorldX) <= 100000.f && std::abs(window.fTargetWorldZ) <= 100000.f &&
				window.fTargetRadiusM >= .01f && window.fTargetRadiusM <= 1000.f;
			break;
		case BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT:
			valuesValid = !window.CardRegions.empty() && !window.ContactTargets.empty() && window.ContactTargets.size() <= 64u &&
				!window.OnSuccess.empty() && window.OnFail.empty() && window.OnTimeout.empty() && !window.bEndsPatternOnSuccess;
			break;
        case BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD:
            valuesValid = window.CardRegions.empty() && window.OnSuccess.empty() && window.OnFail.empty() &&
                window.OnTimeout.empty() && !window.bEndsPatternOnSuccess;
            break;
		case BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW:
		case BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL:
			valuesValid = window.CardRegions.empty() && window.OnFail.empty();
			break;
		case BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS:
			valuesValid = window.fHalfAngleDegrees > 0.f;
			break;
		case BOSS_PATTERN_LOGIC_KIND::POSE_INPUT:
			valuesValid = window.iPoseIndex < 4u;
			break;
		case BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW:
			valuesValid = window.iThreshold > 0u;
			break;
		default:
			valuesValid = false;
			break;
		}
        if (!std::isfinite(window.fBossChargeDistanceM) || window.fBossChargeDistanceM < 0.f ||
            window.fBossChargeDistanceM > 1000.f || !std::isfinite(window.fChargeYawOffsetDegrees) ||
            std::abs(window.fChargeYawOffsetDegrees) > 360.f ||
            (window.fChargeYawOffsetDegrees != 0.f && window.fBossChargeDistanceM <= 0.f) ||
            (window.fBossChargeDistanceM > 0.f &&
            (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || pattern.BossMotion)))
            valuesValid = false;
        if (window.fBossChargeDistanceM > 0.f)
        {
            for (const auto& other : pattern.LogicWindows)
                if (&other != &window && other.fBossChargeDistanceM > 0.f &&
                    other.iStartMs < endMs && window.iStartMs < std::uint64_t(other.iStartMs) + other.iDurationMs)
                    valuesValid = false;
            std::uint64_t stageStartMs = 0u;
            for (const auto& stage : pattern.Stages)
            {
                if (!stage.Actions.empty() && window.iStartMs <= stageStartMs && stageStartMs < endMs)
                    valuesValid = false;
                stageStartMs += stage.iDurationMs;
            }
        }
        const auto captureCount = std::count_if(window.OnSuccess.begin(), window.OnSuccess.end(), [](const auto& result) {
            return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER;
        });
        if (captureCount > 1 || (captureCount == 1 && window.OnSuccess.size() != 1u) ||
            (captureCount == 1) != !window.strHoldLogicOccurrenceId.empty()) valuesValid = false;
        if (!window.strHoldLogicOccurrenceId.empty())
        {
            const auto hold = std::find_if(pattern.LogicWindows.begin(), pattern.LogicWindows.end(), [&](const auto& row) {
                return row.strWindowId == window.strHoldLogicOccurrenceId;
            });
            if (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || hold == pattern.LogicWindows.end() ||
                hold->eKind != BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD || hold->iStartMs > window.iStartMs ||
                std::uint64_t(hold->iStartMs) + hold->iDurationMs < endMs) valuesValid = false;
        }
		const auto resultsValid = [&window](
			const std::vector<BOSS_PATTERN_LOGIC_RESULT>& results)
		{
			if (results.size() > 4u)
				return false;
			for (const BOSS_PATTERN_LOGIC_RESULT& result : results)
			{
				if (!std::isfinite(result.fPushRangeM) || result.fPushRangeM < 0.f || result.fPushRangeM > 20.f ||
					result.iPushMs > 600000u || ((result.fPushRangeM > 0.f) != (result.iPushMs > 0u)) ||
					(result.fPushRangeM > 0.f && result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE))
					return false;
				const bool worldMotion = BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION == result.eKind;
				if ((worldMotion && (result.strTargetWorldInstanceId.empty() || result.strMotionInstanceId.empty() ||
					result.iPercent != 0u || result.iDurationMs != 0u || !result.strPatternId.empty())) ||
					(!worldMotion && (!result.strTargetWorldInstanceId.empty() || !result.strMotionInstanceId.empty())))
					return false;
				const bool contactMotion = result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION;
				const bool contactSignal = result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW;
				if (((contactMotion || contactSignal) && window.eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT) ||
					(window.eKind == BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT && !contactMotion && !contactSignal) ||
					(contactMotion && result.ContactMotions.size() != window.ContactTargets.size()) ||
					(contactSignal && result.strTargetLogicOccurrenceId.empty()))
					return false;
                const bool capture = result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER;
                if (capture ? (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || &results != &window.OnSuccess ||
                    result.eAttachmentSlot != LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ||
                    result.iPercent || result.iDurationMs ||
                    std::any_of(result.GripLocalOffset.begin(), result.GripLocalOffset.end(), [](const float value) {
                        return !std::isfinite(value) || std::abs(value) > 10.f;
                    })) : (result.eAttachmentSlot != LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE ||
                    result.GripLocalOffset != std::array<float, 3u>{})) return false;
                if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR &&
                    (result.iDurationMs == 0u || result.iDurationMs > 600000u ||
                     result.iPercent != 0u || result.strFearPresentationId.empty())) return false;
				if (BOSS_PATTERN_LOGIC_RESULT_KIND::NONE == result.eKind)
					return false;
				/* Boss-level completion may hand the audition a follow-up;
				a per-player verdict cannot move the boss. */
				if (BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN == result.eKind &&
					BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW != window.eKind && BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW != window.eKind && BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL != window.eKind)
					return false;
			}
			return true;
		};
		if (window.strWindowId.empty() || !windowIds.insert(window.strWindowId).second ||
			0u == window.iDurationMs || endMs > patternDurationMs || !valuesValid ||
			(window.bInsideIsFail && BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP != window.eKind &&
				BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP != window.eKind &&
				BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS != window.eKind) ||
			std::any_of(window.CardRegions.begin(),window.CardRegions.end(),[&](const BOSS_LOGIC_REGION& region)
			{ return region.WorldTrack.bEnabled && (region.WorldTrack.Keys.empty() ||
				(BOSS_PATTERN_LOGIC_KIND::ENTER_AREA != window.eKind && BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP != window.eKind &&
				 BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT != window.eKind) ||
				region.WorldTrack.iStartMs > window.iStartMs); }) ||
			!resultsValid(window.OnSuccess) || !resultsValid(window.OnFail) ||
			!resultsValid(window.OnTimeout) ||
			(endTickKind && !window.OnTimeout.empty()) ||
			(BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW == window.eKind &&
				!window.OnFail.empty()))
		{
			status = "KoukuSaydon pattern logic window is out of the pattern lifetime or carries invalid values";
			return false;
		}
	}
	for (const BOSS_PATTERN_WORLD_SEQUENCE& sequence : pattern.WorldSequences)
	{
		if (sequence.Placement)
		{
			const auto& placement = *sequence.Placement;
			const float values[] = { placement.fPositionX, placement.fPositionY, placement.fPositionZ,
				placement.fRotationXDegrees, placement.fRotationYDegrees, placement.fRotationZDegrees,
				placement.fScaleX, placement.fScaleY, placement.fScaleZ };
			bool valid = !sequence.strOccurrenceId.empty() && sequence.SupportWindows.empty() && !sequence.bAnchorBossSpawn &&
				sequence.fPositionOffsetX == 0.f && sequence.fPositionOffsetY == 0.f && sequence.fPositionOffsetZ == 0.f &&
				sequence.fAnchorPositionX == 0.f && sequence.fAnchorPositionY == 0.f && sequence.fAnchorPositionZ == 0.f;
			for (std::size_t index = 0u; index < 9u; ++index)
				valid = valid && std::isfinite(values[index]) && (index < 3u ? std::abs(values[index]) <= 100000.f :
					index < 6u ? std::abs(values[index]) <= 36000.f : values[index] >= .001f && values[index] <= 1000.f);
			if (!valid)
			{
				status = "KoukuSaydon absolute World placement or occurrence ownership is invalid";
				return false;
			}
		}
		if (sequence.strInstanceId.empty() || sequence.iStartMs > patternDurationMs ||
			sequence.iDurationMs > 600000u ||
			!(sequence.fPlaybackSpeed >= 0.05f && sequence.fPlaybackSpeed <= 16.f))
		{
			status = "KoukuSaydon pattern world sequence cue is invalid";
			return false;
		}
	}
	for (const BOSS_PATTERN_SCENE_PROFILE& profile : pattern.SceneProfiles)
	{
		if (profile.strProfileId.empty() || profile.iStartMs > patternDurationMs)
		{
			status = "KoukuSaydon pattern scene profile cue is invalid";
			return false;
		}
	}
	status.clear();
	return true;
}

bool LostArk::Server::CKoukuSaydonBrain::Select_AnimationOnlySequence(
	const std::vector<BOSS_PATTERN_DEFINITION>& definitions,
	const BOSS_PATTERN_SEQUENCE_DEFINITION& sequence,
	const std::string_view bossArchetypeId,
	std::vector<std::string>& outPatternIds,
	std::vector<std::uint32_t>& outTransitionTicks,
	std::string& status,
	const std::string_view gateId,
	const std::string_view targetBossPlacementId)
{
	if (sequence.PatternIds.empty() ||
		sequence.PatternIds.size() != sequence.iExpectedStepCount ||
		sequence.TransitionPursuitTicks.size() + 1u != sequence.PatternIds.size())
	{
		status = "KoukuSaydon Product pattern sequence is unavailable";
		return false;
	}
	std::vector<std::string> selectedPatterns;
	std::vector<std::uint32_t> selectedTransitions;
	std::size_t previousSelectedIndex = 0u;
	for (std::size_t index = 0u; index < sequence.PatternIds.size(); ++index)
	{
		const std::string& patternId = sequence.PatternIds[index];
		const auto pattern = std::find_if(definitions.begin(), definitions.end(),
			[&patternId](const BOSS_PATTERN_DEFINITION& candidate)
			{ return candidate.strPatternId == patternId; });
		if (definitions.end() == pattern)
		{
			status = "KoukuSaydon Product sequence names an unknown pattern: " + patternId;
			return false;
		}
		if (!Validate_AnimationOnlyPattern(*pattern, status))
			return false;
		const bool admitted = pattern->AuditionBossArchetypeIds.empty() ?
			KOUKUSAYDON_G1_BOSS_ARCHETYPE_ID == bossArchetypeId :
			pattern->AuditionBossArchetypeIds.end() != std::find(
				pattern->AuditionBossArchetypeIds.begin(),
				pattern->AuditionBossArchetypeIds.end(), bossArchetypeId);
		if (!admitted ||
			(!gateId.empty() && !pattern->strGateId.empty() && pattern->strGateId != gateId) ||
			(!targetBossPlacementId.empty() && !pattern->strTargetBossPlacementId.empty() &&
				pattern->strTargetBossPlacementId != targetBossPlacementId))
			continue;
		if (!selectedPatterns.empty())
			selectedTransitions.push_back(
				sequence.TransitionPursuitTicks[previousSelectedIndex]);
		selectedPatterns.push_back(patternId);
		previousSelectedIndex = index;
	}
	if (selectedPatterns.empty())
	{
		status = "KoukuSaydon Play All has no Product pattern for the target boss: " +
			std::string{ bossArchetypeId };
		return false;
	}
	outPatternIds = std::move(selectedPatterns);
	outTransitionTicks = std::move(selectedTransitions);
	status.clear();
	return true;
}

std::uint32_t
LostArk::Server::CKoukuSaydonBrain::Resolve_ProductSourceRevision(
	const CGameplayCatalog& catalog) noexcept
{
	return catalog.Find_KoukuSaydonProductSourceRevision(
		std::string{ KOUKUSAYDON_G1_ENCOUNTER_ID });
}

const LostArk::Server::BOSS_PATTERN_DEFINITION*
LostArk::Server::CKoukuSaydonBrain::Find_AnimationOnlyPattern(
	const CGameplayCatalog& catalog,
	const std::string_view patternId,
	std::string& status)
{
	const auto* patterns = catalog.Find_BossPatterns(
		std::string{ KOUKUSAYDON_G1_ENCOUNTER_ID });
	if (nullptr == patterns)
	{
		status = "KoukuSaydon encounter patterns are missing";
		return nullptr;
	}
	const auto found = std::find_if(patterns->begin(), patterns->end(),
		[patternId](const BOSS_PATTERN_DEFINITION& pattern)
		{
			return pattern.strPatternId == patternId;
		});
	if (patterns->end() == found)
	{
		status = "KoukuSaydon pattern ID is unknown";
		return nullptr;
	}
	return Validate_AnimationOnlyPattern(*found, status) ? &*found : nullptr;
}

void LostArk::Server::CKoukuSaydonBrain::Enter_Stage(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_DEFINITION& stage,
	const std::uint32_t stageIndex,
	const std::uint32_t serverTick,
	const bool evaluatesOnEntryTick)
{
	boss.iPatternStageIndex = stageIndex;
	boss.strPatternStageId = stage.strStageId;
	boss.strActionId = stage.strActionId;
	boss.iPatternStageDurationMs = stage.iDurationMs;
	boss.iPatternStageFirstEvaluationTick = evaluatesOnEntryTick ?
		serverTick : Next_ServerTick(serverTick);
	boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
	boss.fActionElapsedSeconds = 0.f;
	boss.eAction = To_ServerAction(stage.eStageKind);
}

bool LostArk::Server::CKoukuSaydonBrain::Begin_Pattern(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& pattern,
	const LostArk::Shared::GameplayDataRevision& revision,
	const std::uint32_t serverTick,
	std::string& status) const
{
	if (!revision.Is_Valid() || 0u == serverTick ||
		0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction ||
		!boss.strPatternId.empty() ||
		KOUKUSAYDON_G1_ENCOUNTER_ID != boss.strEncounterId ||
		!Validate_AnimationOnlyPattern(pattern, status))
	{
		if (status.empty())
			status = "KoukuSaydon pattern cannot begin from the current boss state";
		return false;
	}
	boss.PinnedDefinitionRevision = revision;
	boss.PatternTerminalReceipt = {};
	boss.strPatternId = pattern.strPatternId;
	boss.iPatternStartTick = serverTick;
	boss.iPatternSequence = boss.iPatternSequence ==
		(std::numeric_limits<std::uint32_t>::max)() ? 1u :
		boss.iPatternSequence + 1u;
	Enter_Stage(boss, pattern.Stages.front(), 0u, serverTick, true);
	Apply_BossMotion(boss, pattern, serverTick);
	status.clear();
	return true;
}

void LostArk::Server::CKoukuSaydonBrain::Finish_Pattern(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick,
	const SERVER_BOSS_PATTERN_TERMINAL_RESULT result)
{
	(void)serverTick;
	if (!boss.strPatternId.empty())
	{
		boss.PatternTerminalReceipt.iPatternSequence = boss.iPatternSequence;
		boss.PatternTerminalReceipt.iRootPatternSequence = boss.iPatternSequence;
		boss.PatternTerminalReceipt.eResult = result;
	}
	boss.strPatternId.clear();
	boss.iPatternStartTick = 0u;
	boss.strPatternStageId.clear();
	boss.strActionId.clear();
	boss.strDamageProfileId.clear();
	boss.iPatternStageIndex = 0u;
	boss.iPatternStageDurationMs = 0u;
	boss.iPatternStageFirstEvaluationTick = 0u;
	boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
	boss.fActionElapsedSeconds = 0.f;
	boss.eAction = 0u == boss.iCurrentHp ?
		SERVER_ENTITY_ACTION::DEAD : SERVER_ENTITY_ACTION::IDLE;
}

LostArk::Server::KOUKUSAYDON_BRAIN_UPDATE_RESULT
LostArk::Server::CKoukuSaydonBrain::Update(
	SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const std::uint32_t serverTick,
	std::string& status) const
{
	if (boss.strPatternId.empty())
	{
		status.clear();
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::IDLE;
	}
	if (0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction)
	{
		Finish_Pattern(boss, serverTick,
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED);
		status = "KoukuSaydon boss died during pattern playback";
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_BOSS_DEAD;
	}
	const BOSS_PATTERN_DEFINITION* pattern =
		Find_AnimationOnlyPattern(catalog, boss.strPatternId, status);
	if (nullptr == pattern || boss.iPatternStageIndex >= pattern->Stages.size() ||
		pattern->Stages[boss.iPatternStageIndex].strStageId !=
			boss.strPatternStageId ||
		pattern->Stages[boss.iPatternStageIndex].strActionId != boss.strActionId)
	{
		Finish_Pattern(boss, serverTick,
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED);
		if (status.empty())
			status = "KoukuSaydon running stage no longer matches its pinned definition";
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_INVALID_DEFINITION;
	}
	Apply_BossMotion(boss, *pattern, serverTick);
	const std::uint64_t elapsedTicks = Stage_ElapsedTicks(boss, serverTick);
	boss.fActionElapsedSeconds = static_cast<float>(elapsedTicks) /
		static_cast<float>(SERVER_TICK_HZ);
	const BOSS_PATTERN_STAGE_DEFINITION& stage =
		pattern->Stages[boss.iPatternStageIndex];
	if (!Has_ElapsedMilliseconds(elapsedTicks, stage.iDurationMs))
	{
		status.clear();
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::RUNNING;
	}
	const std::uint32_t nextStage = boss.iPatternStageIndex + 1u;
	if (nextStage < pattern->Stages.size())
	{
		Enter_Stage(boss, pattern->Stages[nextStage], nextStage, serverTick, false);
		status.clear();
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::STAGE_CHANGED;
	}
	// Contact/deadline windows use the pattern clock, whose final sample may be
	// one tick after the last animation stage. Keep that pose until Logic has
	// evaluated the terminal tick; intermediate stage clocks remain unchanged.
	const std::uint64_t patternElapsedTicks = serverTick >= boss.iPatternStartTick ?
		static_cast<std::uint64_t>(serverTick - boss.iPatternStartTick) :
		static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - boss.iPatternStartTick) + serverTick;
	for (const auto& window : pattern->LogicWindows)
	{
		if (window.eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT && window.eKind != BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL &&
            window.eKind != BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW && window.eKind != BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD &&
            window.fBossChargeDistanceM <= 0.f) continue;
		const std::uint64_t endMs = std::uint64_t(window.iStartMs) + window.iDurationMs;
		const std::uint64_t deadlineTicks = (endMs * SERVER_TICK_HZ + MILLISECONDS_PER_SECOND - 1u) / MILLISECONDS_PER_SECOND;
		if (patternElapsedTicks < deadlineTicks)
		{
			status.clear();
			return KOUKUSAYDON_BRAIN_UPDATE_RESULT::RUNNING;
		}
	}
	Finish_Pattern(boss, serverTick,
		SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED);
	status.clear();
	return KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED;
}

void LostArk::Server::CKoukuSaydonBrain::Apply_BossMotion(
	SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern,
	const std::uint32_t serverTick) noexcept
{
	if (!pattern.BossMotion || !boss.iPatternStartTick || boss.strPatternId != pattern.strPatternId) return;
	const auto& motion = *pattern.BossMotion;
	if (motion.iEndMs <= motion.iStartMs) return;
	const std::uint64_t ticks = serverTick >= boss.iPatternStartTick ? serverTick - boss.iPatternStartTick :
		static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - boss.iPatternStartTick) + serverTick;
    const auto position = Sample_BossMotion(motion, static_cast<std::uint32_t>(ticks));
    boss.fPositionX = position[0]; boss.fPositionY = position[1]; boss.fPositionZ = position[2];
    boss.fYawDegrees = motion.fYawDegrees;
}

std::array<float, 3u> LostArk::Server::CKoukuSaydonBrain::Sample_BossMotion(
    const BOSS_PATTERN_BOSS_MOTION& motion, const std::uint32_t elapsedTicks) noexcept
{
    if (motion.iEndMs <= motion.iStartMs) return motion.StartPosition;
    const double timeMs = static_cast<double>(elapsedTicks) * 1000.0 / SERVER_TICK_HZ;
    const double alpha = (std::clamp)((timeMs - motion.iStartMs) /
        static_cast<double>(motion.iEndMs - motion.iStartMs), 0.0, 1.0);
    return {
        static_cast<float>(motion.StartPosition[0] + (motion.EndPosition[0] - motion.StartPosition[0]) * alpha),
        motion.StartPosition[1],
        static_cast<float>(motion.StartPosition[2] + (motion.EndPosition[2] - motion.StartPosition[2]) * alpha)};
}

void LostArk::Server::CKoukuSaydonBrain::Abort_Pattern(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick) const
{
	Finish_Pattern(
		boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED);
}

void LostArk::Server::CKoukuSaydonBrain::Complete_Pattern(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick) const
{
	if (boss.strPatternId.empty())
		return;
	Finish_Pattern(
		boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED);
}
