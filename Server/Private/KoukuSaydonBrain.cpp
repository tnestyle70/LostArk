#include "KoukuSaydonBrain.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"

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

	bool Is_AnimationRootMotion(
		const LostArk::Server::BOSS_PATTERN_STAGE_MOTION& motion,
		const std::uint32_t durationMs) noexcept
	{
		using namespace LostArk::Server;
		if (BOSS_PATTERN_STAGE_MOTION_KIND::NONE != motion.eKind ||
			0u != motion.iRetargetDelayMs || 0.f != motion.fSpeedMps ||
			0.f != motion.fDistance || 0u != motion.iCornerIndex ||
			0.f != motion.fHalfExtentsX || 0.f != motion.fHalfExtentsZ) return false;
		if (motion.RootMotion.empty()) return true;
		const auto& samples = motion.RootMotion;
		if (samples.size() < 2u || samples.size() > 512u || samples.front().iTimeMs != 0u ||
			samples.back().iTimeMs != durationMs || samples.front().fForward != 0.f ||
			samples.front().fLateral != 0.f || samples.front().fUp != 0.f) return false;
		for (std::size_t index = 0u; index < samples.size(); ++index)
		{
			const auto& sample = samples[index];
			if (!std::isfinite(sample.fForward) || !std::isfinite(sample.fLateral) ||
				!std::isfinite(sample.fUp) || std::abs(sample.fForward) > 100000.f ||
				std::abs(sample.fLateral) > 100000.f || std::abs(sample.fUp) > 100000.f ||
				(index && sample.iTimeMs <= samples[index - 1u].iTimeMs)) return false;
		}
		return true;
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

	const bool hasRootMotion = std::any_of(pattern.Stages.begin(), pattern.Stages.end(),
		[](const auto& stage) { return !stage.Motion.RootMotion.empty(); });
	if (hasRootMotion && (pattern.BossMotion ||
		std::any_of(pattern.LogicWindows.begin(), pattern.LogicWindows.end(),
			[](const auto& window) { return window.fBossChargeDistanceM > 0.f; }) ||
		std::any_of(pattern.MechanicTriggers.begin(), pattern.MechanicTriggers.end(),
			[](const auto& trigger) { return trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT; })))
	{ status = "KoukuSaydon animation root motion conflicts with authored movement"; return false; }
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
					(action.strTargetId == "boss.target.pattern" ||
					 action.strTargetId == "boss.target.nearest") && action.iValue == 1u &&
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
			!Is_AnimationRootMotion(stage.Motion, stage.iDurationMs) || !stage.strPropBreakSetId.empty() ||
			!stage.PropBreakSlotIds.empty() || !supportedActions ||
			!validTimeoutBranch)
		{
			status = "KoukuSaydon pattern contains unsupported combat, motion, action, or branch data";
			return false;
		}
	}
	std::uint64_t patternDurationMs = 0u;
	for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
	{
		if (pattern.bFixedTimelineClock &&
			((patternDurationMs + stage.iDurationMs) * SERVER_TICK_HZ + 999u) / 1000u <=
			(patternDurationMs * SERVER_TICK_HZ + 999u) / 1000u)
		{ status = "Parent stage boundaries must occupy distinct Server ticks"; return false; }
		patternDurationMs += stage.iDurationMs;
	}
	// Rows use their independent lifetime; Stage progression still uses only Stages.
	patternDurationMs = (std::max)(patternDurationMs, std::uint64_t(pattern.iTimelineDurationMs));
	if (pattern.BossMotion)
	{
		const auto& motion = *pattern.BossMotion;
		bool valid = !pattern.bResetBossToSpawn && !pattern.ResetBossYawDegrees &&
			motion.iStartMs < motion.iEndMs && motion.iEndMs <= patternDurationMs &&
			(!motion.Keys.empty() || motion.StartPosition[1] == motion.EndPosition[1]) &&
			std::isfinite(motion.fYawDegrees) && std::abs(motion.fYawDegrees) <= 360.f;
		for (std::size_t axis = 0u; axis < 3u; ++axis)
			valid = valid && std::isfinite(motion.StartPosition[axis]) && std::isfinite(motion.EndPosition[axis]) &&
				std::abs(motion.StartPosition[axis]) <= 100000.f && std::abs(motion.EndPosition[axis]) <= 100000.f;
		if (!motion.Keys.empty())
		{
			valid = valid && motion.Keys.size() >= 2u && motion.Keys.size() <= 512u &&
				motion.Keys.front().iTimeMs == motion.iStartMs && motion.Keys.back().iTimeMs == motion.iEndMs &&
				motion.Keys.front().Position == motion.StartPosition && motion.Keys.back().Position == motion.EndPosition;
			for (std::size_t i = 0u; i < motion.Keys.size(); ++i)
				valid = valid && (i == 0u || motion.Keys[i - 1u].iTimeMs < motion.Keys[i].iTimeMs) &&
					std::all_of(motion.Keys[i].Position.begin(), motion.Keys[i].Position.end(), [](float value) {
						return std::isfinite(value) && std::abs(value) <= 100000.f; });
		}
		if (!valid)
		{ status = "KoukuSaydon Boss Motion interval, keys, base height or reset policy is invalid"; return false; }
	}
	std::unordered_set<std::string> triggerIds;
	for (const auto& trigger : pattern.MechanicTriggers)
	{
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES)
		{
			std::unordered_set<std::string> directions(trigger.DirectionPatternIds.begin(), trigger.DirectionPatternIds.end());
			if (pattern.BossMotion || trigger.DirectionPatternIds.size() != 4u || directions.size() != 4u ||
				directions.count("") || directions.count(pattern.strPatternId) || trigger.strCloneEndStageId.empty() ||
				trigger.eHudMode != LostArk::Shared::KOUKU_HUD_MODE::NONE || trigger.fTeleportX != 0.f ||
				trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f || !trigger.strClonePatternId.empty() ||
				!trigger.ClockHours.empty() || !trigger.PatternSpawns.empty() || trigger.iCountPerPlayer ||
				trigger.iEffectLifetimeMs || trigger.iArenaRandomCount || trigger.bRandomPlayerOnly)
			{ status = "Cross direction requires four distinct child Patterns and one clone end Stage"; return false; }
		}
		else if (!trigger.DirectionPatternIds.empty() || !trigger.strCloneEndStageId.empty())
		{ status = "Only cross direction owns directional Pattern references"; return false; }

		using Air = ALBION_AIRBORNE_PHASE;
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE)
		{
			const auto phase = trigger.eAirbornePhase;
			const bool jump = phase == Air::JUMP, appear = phase == Air::APPEAR_PLAYER;
			if (phase == Air::NONE || phase > Air::SLAM || pattern.BossMotion ||
				!std::isfinite(trigger.fAirborneHeightM) || trigger.fAirborneHeightM < 0.f || trigger.fAirborneHeightM > 100000.f ||
				((jump || appear) != (trigger.fAirborneHeightM > 0.f)) ||
				(trigger.bCaptureAirborneTargetPosition && phase != Air::SELECT_PLAYER) ||
				trigger.strSelectedEffectVisualId.empty() != (trigger.iSelectedEffectLifetimeMs == 0u) ||
				(!trigger.strSelectedEffectVisualId.empty() && !trigger.bCaptureAirborneTargetPosition) ||
				std::uint64_t(trigger.iStartMs) + trigger.iSelectedEffectLifetimeMs > patternDurationMs ||
				(jump ? (trigger.iAirborneDurationMs > 600000u ||
				 std::uint64_t(trigger.iStartMs) + trigger.iAirborneDurationMs > patternDurationMs) : trigger.iAirborneDurationMs != 0u) ||
				trigger.eHudMode != LostArk::Shared::KOUKU_HUD_MODE::NONE || !trigger.strClonePatternId.empty() || !trigger.ClockHours.empty() ||
				trigger.fFaceCenterYawOffsetDegrees != 0.f || trigger.iCountPerPlayer || trigger.fPlayerEffectRadiusM != 0.f || trigger.iEffectLifetimeMs ||
				trigger.iArenaRandomCount || trigger.bRandomPlayerOnly || !trigger.PatternSpawns.empty() ||
				!std::isfinite(trigger.fTeleportX) || !std::isfinite(trigger.fTeleportY) || !std::isfinite(trigger.fTeleportZ) ||
				std::abs(trigger.fTeleportX) > 100000.f || std::abs(trigger.fTeleportY) > 100000.f || std::abs(trigger.fTeleportZ) > 100000.f ||
				(phase != Air::CENTER && (trigger.fTeleportX != 0.f || trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f)))
			{ status = "Albion airborne phase has invalid or unrelated values"; return false; }
			if (phase != Air::JUMP && phase != Air::SELECT_PLAYER && !std::any_of(pattern.MechanicTriggers.begin(), pattern.MechanicTriggers.end(),
				[&](const auto& row) { return row.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE &&
				 row.eAirbornePhase == Air::JUMP && row.iStartMs <= trigger.iStartMs; }))
			{ status = "Albion airborne phase requires its initial jump"; return false; }
			if (phase == Air::SLAM)
			{
				std::uint64_t start = 0u; const BOSS_PATTERN_STAGE_DEFINITION* source = nullptr;
				for (const auto& stage : pattern.Stages) { if (trigger.iStartMs < start + stage.iDurationMs) { source = &stage; break; } start += stage.iDurationMs; }
				if (!source || source->Motion.RootMotion.empty()) { status = "Albion slam requires source root motion"; return false; }
				const double age = double(trigger.iStartMs - start);
				const float up = Sample_StageRootMotion(source->Motion.RootMotion, age).fUp;
				float minimum = up;
				for (const auto& sample : source->Motion.RootMotion) if (sample.iTimeMs >= age) minimum = (std::min)(minimum, sample.fUp);
				if (up - minimum <= .000001f) { status = "Albion slam has no remaining source descent"; return false; }
			}
		}
		else if (trigger.eAirbornePhase != Air::NONE || trigger.fAirborneHeightM != 0.f || trigger.iAirborneDurationMs != 0u ||
			trigger.bCaptureAirborneTargetPosition || !trigger.strSelectedEffectVisualId.empty() || trigger.iSelectedEffectLifetimeMs != 0u)
		{ status = "Non-Albion trigger carries airborne values"; return false; }
		const bool hitShowtime = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS;
		const bool hitPursuit = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES;
		const bool hitAlbion = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_BLUE_CIRCLE;
		if ((!hitShowtime && !trigger.TrackingHits.empty()) ||
			(!hitShowtime && !hitAlbion && !trigger.FixedHits.empty()) ||
			(!hitPursuit && !trigger.ProjectileHits.empty()) ||
			(!hitAlbion && !trigger.FixedHits.empty() && trigger.strFixedVisualId.empty()) ||
			(!trigger.TrackingHits.empty() && trigger.strTrackingVisualId.empty()) ||
			!LostArk::Shared::Validate_AttackHitTemplates(trigger.FixedHits, hitAlbion ? trigger.iEffectLifetimeMs : trigger.iFixedLifetimeMs) ||
			!LostArk::Shared::Validate_AttackHitTemplates(trigger.TrackingHits, trigger.iDurationMs) ||
			!LostArk::Shared::Validate_AttackHitTemplates(trigger.ProjectileHits, trigger.iProjectileLifetimeMs ? trigger.iProjectileLifetimeMs : 600000u) ||
			std::any_of(trigger.RandomVolleys.begin(), trigger.RandomVolleys.end(), [](const auto& volley) { return !LostArk::Shared::Validate_AttackHitTemplates(volley.Hits, volley.iLifetimeMs); }))
		{ status = "Dynamic attack templates have invalid owners, shapes or hit windows"; return false; }
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES)
		{
			const auto count = trigger.ProjectileVisualIds.size();
			if (!count || count > 4u || trigger.strContactVisualId.empty() ||
                (!trigger.ProjectileCardSymbols.empty() && (trigger.ProjectileCardSymbols.size() != count ||
                 std::any_of(trigger.ProjectileCardSymbols.begin(), trigger.ProjectileCardSymbols.end(), [](const auto symbol) {
                     return symbol <= LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE || symbol >= LostArk::Shared::MECHANIC_CARD_SYMBOL::END; }))) ||
				std::any_of(trigger.ProjectileVisualIds.begin(), trigger.ProjectileVisualIds.end(), [](const auto& id) { return id.empty(); }) ||
				!std::isfinite(trigger.fProjectileSpeedMps) || trigger.fProjectileSpeedMps < .01f || trigger.fProjectileSpeedMps > 100.f ||
				!std::isfinite(trigger.fProjectileContactRadiusM) || trigger.fProjectileContactRadiusM < .01f || trigger.fProjectileContactRadiusM > 10.f ||
				!std::isfinite(trigger.fProjectileSpawnRadiusM) || trigger.fProjectileSpawnRadiusM < 0.f || trigger.fProjectileSpawnRadiusM > 100.f ||
				!std::isfinite(trigger.fProjectileMaxDistanceM) || trigger.fProjectileMaxDistanceM < 0.f || trigger.fProjectileMaxDistanceM > 1000.f ||
				trigger.iProjectileLifetimeMs > 600000u || trigger.iSpawnIntervalMs > 600000u ||
				!trigger.iProjectileCountPerWave || trigger.iProjectileCountPerWave > 16u ||
				(trigger.iProjectileLifetimeMs == 0u && (!trigger.bProjectileHoming || trigger.iSpawnIntervalMs != 0u || trigger.fProjectileMaxDistanceM != 0.f)))
			{ status = "Pursuit projectile visual, motion or lifetime contract is invalid"; return false; }
		}
		else if (!trigger.ProjectileVisualIds.empty() || !trigger.ProjectileCardSymbols.empty() || !trigger.strContactVisualId.empty() ||
			trigger.fProjectileSpeedMps != 0.f || trigger.fProjectileMaxDistanceM != 0.f || trigger.fProjectileContactRadiusM != 0.f || trigger.fProjectileSpawnRadiusM != 0.f ||
			trigger.iProjectileLifetimeMs != 0u || trigger.iProjectileCountPerWave != 0u || trigger.bProjectileHoming)
		{ status = "Only pursuit projectiles own projectile values"; return false; }
		const bool hasRandomVolleys = !trigger.RandomVolleys.empty();
		if (hasRandomVolleys ?
			(trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS || trigger.RandomVolleys.size() > 32u ||
             (trigger.strRandomAnchorKind != "BOSS_SPAWN" && trigger.strRandomAnchorKind != "BOSS") ||
             !std::isfinite(trigger.fRandomScaleMin) || !std::isfinite(trigger.fRandomScaleMax) ||
             trigger.fRandomScaleMin < .01f || trigger.fRandomScaleMax > 10.f || trigger.fRandomScaleMax < trigger.fRandomScaleMin ||
			 trigger.iRandomSpawnIntervalMs == 0u || trigger.iRandomSpawnIntervalMs > 600000u ||
			 !std::isfinite(trigger.fRandomArenaRadiusM) || trigger.fRandomArenaRadiusM <= 0.f || trigger.fRandomArenaRadiusM > 1000.f ||
			 !std::isfinite(trigger.fRandomArenaHeightToleranceM) || trigger.fRandomArenaHeightToleranceM <= 0.f || trigger.fRandomArenaHeightToleranceM > 10.f ||
			 std::any_of(trigger.RandomVolleys.begin(), trigger.RandomVolleys.end(), [](const auto& volley) {
				return volley.strClientVisualId.empty() || volley.iLifetimeMs == 0u || volley.iLifetimeMs > 600000u; })) :
			(trigger.iRandomSpawnIntervalMs != 0u || trigger.fRandomArenaRadiusM != 0.f || trigger.fRandomArenaHeightToleranceM != 0.f || trigger.strRandomAnchorKind != "BOSS_SPAWN" || trigger.fRandomScaleMin != 1.f || trigger.fRandomScaleMax != 1.f))
		{ status = "Showtime random volley pool, cadence or arena contract is invalid"; return false; }
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS &&
			((trigger.strFixedVisualId == trigger.strTrackingVisualId && (!hasRandomVolleys || !trigger.strFixedVisualId.empty())) ||
			 trigger.strFixedVisualId.empty() != (trigger.iFixedLifetimeMs == 0u) || trigger.iFixedLifetimeMs > 600000u ||
			 trigger.iSpawnIntervalMs == 0u || trigger.iSpawnIntervalMs > 600000u ||
			 !std::isfinite(trigger.fFollowSpeedScale) || trigger.fFollowSpeedScale < .01f || trigger.fFollowSpeedScale > 10.f))
		{ status = "Showtime player-target visual or timing contract is invalid"; return false; }
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET &&
			(trigger.iDurationMs > 600000u || !trigger.strFixedVisualId.empty() || !trigger.strTrackingVisualId.empty() ||
			 trigger.iFixedLifetimeMs != 0u || trigger.iSpawnIntervalMs != 0u ||
			 // Zero rotates only; an authored scale walks the body at the tracked player's speed.
			 !std::isfinite(trigger.fFollowSpeedScale) || trigger.fFollowSpeedScale < 0.f ||
			 (trigger.fFollowSpeedScale != 0.f && (trigger.fFollowSpeedScale < .01f || trigger.fFollowSpeedScale > 10.f)) ||
			 trigger.eHudMode != LostArk::Shared::KOUKU_HUD_MODE::NONE || trigger.fTeleportX != 0.f || trigger.fTeleportY != 0.f ||
			 trigger.fTeleportZ != 0.f || !trigger.strClonePatternId.empty() || !trigger.ClockHours.empty() ||
			 trigger.fFaceCenterYawOffsetDegrees != 0.f || trigger.iCountPerPlayer != 0u || trigger.fPlayerEffectRadiusM != 0.f ||
			 trigger.iEffectLifetimeMs != 0u || trigger.iArenaRandomCount != 0u || trigger.bRandomPlayerOnly || !trigger.PatternSpawns.empty()))
		{ status = "Boss tracking duration must not create visuals or carry another mechanic's values"; return false; }
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS)
		{
			std::unordered_set<std::string> spawnIds;
			if (trigger.PatternSpawns.empty() || trigger.PatternSpawns.size() > KOUKU_SUMMON_MAX_PATTERN_SPAWNS)
			{ status = "Summon Pattern trigger requires one to sixteen spawns"; return false; }
			for (const auto& spawn : trigger.PatternSpawns)
			{
				if (spawn.strSpawnId.empty() || !spawnIds.insert(spawn.strSpawnId).second || spawn.strPatternId.empty() ||
					spawn.strPatternId == pattern.strPatternId || !std::isfinite(spawn.fYawOffsetDegrees) || std::abs(spawn.fYawOffsetDegrees) > 360.f ||
					(spawn.eAnchorKind != BOSS_PATTERN_SUMMON_ANCHOR_KIND::BOSS && spawn.eAnchorKind != BOSS_PATTERN_SUMMON_ANCHOR_KIND::MAP) ||
					std::any_of(spawn.PositionOffset.begin(), spawn.PositionOffset.end(), [&](const float value) { return !std::isfinite(value) ||
						std::abs(value) > (spawn.eAnchorKind == BOSS_PATTERN_SUMMON_ANCHOR_KIND::MAP ? 100000.f : 1000.f); }))
				{ status = "Summon Pattern spawn identity or transform is invalid"; return false; }
			}
		}
		if ((trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_STAGE_PLAYERS) != !trigger.PlayerEntryPositions.empty() ||
			trigger.PlayerEntryPositions.size() > 4u ||
			std::any_of(trigger.PlayerEntryPositions.begin(), trigger.PlayerEntryPositions.end(), [](const auto& position) {
				return std::any_of(position.begin(), position.end(), [](const float value) { return !std::isfinite(value) || std::abs(value) > 100000.f; }); }))
		{ status = "Card maze staging requires one to four finite destinations on its own trigger"; return false; }
		if (trigger.strTriggerId.empty() || !triggerIds.insert(trigger.strTriggerId).second ||
			trigger.iStartMs >= patternDurationMs || 0u == trigger.iDurationMs ||
			static_cast<std::uint64_t>(trigger.iStartMs) + trigger.iDurationMs > patternDurationMs ||
			((BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_XZ == trigger.eKind ||
             BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED == trigger.eKind ||
			 BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER == trigger.eKind ||
			 BOSS_PATTERN_MECHANIC_TRIGGER_KIND::MARIO_PHASE2_PLAYERS == trigger.eKind) &&
			 (pattern.BossMotion || !std::isfinite(trigger.fTeleportX) || !std::isfinite(trigger.fTeleportY) ||
			  !std::isfinite(trigger.fTeleportZ) || std::abs(trigger.fTeleportX) > 100000.f ||
			  std::abs(trigger.fTeleportY) > 100000.f || std::abs(trigger.fTeleportZ) > 100000.f)) ||
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
			(window.iRepeatIntervalMs && window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA && window.eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP) ||
			(window.bRearmOnExit && window.bRepeatAfterKnockback) || window.iRepeatIntervalMs > 600000u ||
			(window.iRepeatIntervalMs && (window.bRearmOnExit || window.bRepeatAfterKnockback)))
		{ status = "ENTER_AREA accepts one contact repeat policy"; return false; }
		if (window.bRepeatAfterKnockback && (window.OnSuccess.size() != 1u ||
			window.OnSuccess.front().eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE ||
			window.OnSuccess.front().fPushRangeM <= 0.f || window.OnSuccess.front().iPushMs == 0u))
		{ status = "Repeat after knockback requires one damage Success with positive knockback"; return false; }
		switch (window.eKind)
		{
		case BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT:
			valuesValid = !window.PatternIds.empty() && window.PatternIds.size() <= 16u && window.iCompletionCount > 0u &&
				window.iCompletionCount <= window.PatternIds.size() && window.CardRegions.empty() && window.OnFail.empty() &&
				window.OnTimeout.empty() && window.OnSuccess.size() <= 1u && (window.OnSuccess.empty() ||
				 window.OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN);
			break;
		case BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH:
			valuesValid = window.CardRegions.size() == 8u || (window.iSectorCount >= 2u &&
				window.SectorSymbols.size() == window.iSectorCount && window.fOuterRadiusM > 0.f);
			break;
		case BOSS_PATTERN_LOGIC_KIND::INVULNERABILITY_ZONE:
			valuesValid = !window.CardRegions.empty() && window.OnSuccess.empty() && window.OnFail.empty() &&
				window.OnTimeout.empty() && !window.bEndsPatternOnSuccess &&
				std::all_of(window.CardRegions.begin(), window.CardRegions.end(), [](const auto& region) {
					return region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::WORLD && !region.WorldTrack.bEnabled;
				});
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
        case BOSS_PATTERN_LOGIC_KIND::CARD_DICE_BIND:
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
		case BOSS_PATTERN_LOGIC_KIND::BINGO_COMPLETED_LINES:
			valuesValid = window.iThreshold >= 1u && window.iThreshold <= 10u && window.CardRegions.empty() &&
				!window.bInsideIsFail && !window.bEndsPatternOnSuccess && window.OnTimeout.empty();
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
				if (!std::isfinite(result.fPushRangeM) || result.fPushRangeM < 0.f || result.fPushRangeM > (result.bPushBallistic ? 100.f : 20.f) ||
					(result.ePushDirection != BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_BOSS && result.ePushDirection != BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD && result.ePushDirection != BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_CONTACT) ||
					(result.ePushDirection == BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD && result.fPushRangeM <= 0.f && result.fPushHeightM <= 0.f) ||
					result.iPushMs > (result.bPushBallistic ? 5000u : 600000u) ||
					(result.bPushBallistic && result.iPushMs < 100u) ||
					!std::isfinite(result.fPushHeightM) || result.fPushHeightM < 0.f || result.fPushHeightM > 100.f ||
					(result.fPushHeightM > 0.f && !result.bPushBallistic) ||
					(result.ePushDirection == BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_CONTACT &&
						((result.fPushRangeM <= 0.f && result.fPushHeightM <= 0.f) || &results == &window.OnTimeout || window.CardRegions.empty() ||
						 (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA && window.eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP))) || ((result.fPushRangeM > 0.f || result.fPushHeightM > 0.f) != (result.iPushMs > 0u)) ||
					((result.fPushRangeM > 0.f || result.fPushHeightM > 0.f) && (result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE && result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE)) ||
					((result.bForcePush || result.bPushCanLeaveArena) && result.fPushRangeM <= 0.f && result.fPushHeightM <= 0.f) ||
					!std::isfinite(result.fPushYawOffsetDegrees) || std::abs(result.fPushYawOffsetDegrees) > 360.f ||
					(result.fPushYawOffsetDegrees != 0.f && (result.ePushDirection != BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD || (result.fPushRangeM <= 0.f && result.fPushHeightM <= 0.f))))
					return false;
				if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE ?
					(result.iDamageAmount == 0u || result.iDamageAmount > 1000000000u || result.iPercent != 0u || result.iDurationMs != 0u || !result.strPatternId.empty()) : result.iDamageAmount != 0u)
					return false;
				if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY &&
					(!result.iDurationMs || result.iDurationMs > 600000u || result.iPercent || !result.strPatternId.empty())) return false;
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
					BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW != window.eKind && BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW != window.eKind && BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL != window.eKind && BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT != window.eKind)
					return false;
			}
			return true;
		};
        for (const auto& region : window.CardRegions)
        {
            const auto& track = region.WorldTrack;
            const bool hasGrip = std::any_of(track.Keys.begin(), track.Keys.end(), [](const auto& key) { return key.bHasGripPosition; });
            if (!hasGrip) continue;
            if (!track.bEnabled || region.eAnchor != BOSS_LOGIC_REGION_ANCHOR::WORLD ||
                window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || window.OnSuccess.size() != 1u ||
                window.OnSuccess.front().eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT ||
                !window.OnFail.empty() || !window.OnTimeout.empty() || region.bCircle || region.bSector ||
                track.iStartMs != window.iStartMs || track.iDurationMs < window.iDurationMs || track.iStartDelayMs ||
                track.fPlaybackSpeed != 1.f || track.bSmoothStep || track.fBaselineX != 0.f || track.fBaselineY != 0.f ||
                track.fBaselineZ != 0.f || track.fBaselineYawDegrees != 0.f || track.fBaselineScaleX != 1.f ||
                track.fBaselineScaleY != 1.f || track.fBaselineScaleZ != 1.f || track.Keys.front().iTimeMs != 0u ||
                track.Keys.back().iTimeMs != track.iDurationMs ||
                std::any_of(track.Keys.begin(), track.Keys.end(), [](const auto& key) {
                    return !key.bHasGripPosition || std::any_of(key.GripPosition.begin(), key.GripPosition.end(),
                        [](float x) { return !std::isfinite(x) || std::abs(x) > 100000.f; }); }))
            { status = "Physical hook grip requires a complete identity-baseline ENTER_AREA BOX track and sole world-hook result."; return false; }
        }
		if (window.strWindowId.empty() || !windowIds.insert(window.strWindowId).second ||
			0u == window.iDurationMs || endMs > patternDurationMs || !valuesValid ||
			(window.bInsideIsFail && BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP != window.eKind &&
				BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP != window.eKind &&
				BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS != window.eKind) ||
			std::any_of(window.CardRegions.begin(),window.CardRegions.end(),[&](const BOSS_LOGIC_REGION& region)
			{ return region.WorldTrack.bEnabled && (region.WorldTrack.Keys.empty() ||
				(BOSS_PATTERN_LOGIC_KIND::ENTER_AREA != window.eKind && BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP != window.eKind && BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP != window.eKind &&
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
		if (sequence.CombatBody)
		{
			const auto& body = *sequence.CombatBody;
			if (sequence.strOccurrenceId.empty() || sequence.bAnchorBossSpawn || !sequence.SupportWindows.empty() ||
				!body.iMaximumHp || body.iMaximumHp > 1000000000u || !std::isfinite(body.fRadiusM) ||
				body.fRadiusM <= .001f || body.fRadiusM > 1000.f || !std::isfinite(body.fCenterX) ||
				!std::isfinite(body.fCenterY) || !std::isfinite(body.fCenterZ) || std::abs(body.fCenterX) > 100000.f ||
				std::abs(body.fCenterY) > 100000.f || std::abs(body.fCenterZ) > 100000.f)
			{ status = "World combat body is outside the fixed owned cue contract"; return false; }
		}
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

bool LostArk::Server::CKoukuSaydonBrain::Validate_SummonedPattern(
	const BOSS_PATTERN_DEFINITION& owner, const BOSS_PATTERN_DEFINITION& child, std::string& status,
	const bool allowActorLocalAirborne)
{
	if (!Validate_AnimationOnlyPattern(child, status)) return false;
	if (owner.strPatternId == child.strPatternId || owner.strEncounterId != child.strEncounterId ||
		owner.strGateId.empty() || owner.strGateId != child.strGateId ||
		child.eCategory != BOSS_PATTERN_CATEGORY::MECHANIC || owner.strTargetBossPlacementId != child.strTargetBossPlacementId ||
		owner.AuditionBossArchetypeIds.empty() || owner.AuditionBossArchetypeIds.size() != child.AuditionBossArchetypeIds.size() ||
		!std::all_of(owner.AuditionBossArchetypeIds.begin(), owner.AuditionBossArchetypeIds.end(), [&](const auto& archetype) {
			return std::find(child.AuditionBossArchetypeIds.begin(), child.AuditionBossArchetypeIds.end(), archetype) != child.AuditionBossArchetypeIds.end(); }) ||
		child.BossMotion || child.bResetBossToSpawn || child.ResetBossYawDegrees ||
		std::any_of(child.LogicWindows.begin(), child.LogicWindows.end(), [](const auto& window) {
			if (window.eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP && window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA) return true;
			const auto unsupported = [](const auto& result) { return result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE &&
				result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE &&
				result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT; };
			return std::any_of(window.OnSuccess.begin(), window.OnSuccess.end(), unsupported) ||
				std::any_of(window.OnFail.begin(), window.OnFail.end(), unsupported) ||
				std::any_of(window.OnTimeout.begin(), window.OnTimeout.end(), unsupported);
		}) ||
		std::any_of(child.MechanicTriggers.begin(), child.MechanicTriggers.end(), [&](const auto& trigger) {
			return !allowActorLocalAirborne || trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE ||
				(trigger.eAirbornePhase != ALBION_AIRBORNE_PHASE::JUMP && trigger.eAirbornePhase != ALBION_AIRBORNE_PHASE::SLAM) ||
				trigger.bCaptureAirborneTargetPosition || !trigger.strSelectedEffectVisualId.empty(); }) ||
		!child.WorldSequences.empty() || !child.SceneProfiles.empty() ||
		std::any_of(child.Stages.begin(), child.Stages.end(), [](const auto& stage) { return !stage.Actions.empty(); }))
	{
		status = "Summoned Pattern requires another same-body, same-Gate leaf with only supported actor-local motion and no world, player or reset actions: " + child.strPatternId;
		return false;
	}
	status.clear();
	return true;
}

bool LostArk::Server::CKoukuSaydonBrain::Select_CrossDirection(
	const SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& parent,
	const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, const CGameplayCatalog& catalog,
	std::size_t& selected, std::array<std::uint32_t, 4u>& cloneDurationsMs,
	std::string& status)
{
	if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES ||
		trigger.DirectionPatternIds.size() != 4u || trigger.strCloneEndStageId.empty())
	{ status = "Cross direction requires four Patterns and a clone end Stage"; return false; }
	std::array<std::uint32_t, 4u> stagedDurations{};
	std::size_t stagedSelection = 0u;
	double bestDistance = (std::numeric_limits<double>::max)();
	const double yaw = boss.fYawDegrees * 0.01745329251994329577;
	std::unordered_set<std::string> identities;
	for (std::size_t index = 0u; index < 4u; ++index)
	{
		const auto* child = Find_AnimationOnlyPattern(catalog, trigger.DirectionPatternIds[index], status);
		if (!identities.insert(trigger.DirectionPatternIds[index]).second || !child ||
			!Validate_SummonedPattern(parent, *child, status)) return false;
		double forward = 0., lateral = 0.;
		std::uint64_t duration = 0u;
		bool foundEnd = false;
		for (const auto& stage : child->Stages)
		{
			duration += stage.iDurationMs;
			if (!foundEnd)
			{
				const auto endpoint = Sample_StageRootMotion(stage.Motion.RootMotion, stage.iDurationMs);
				forward += endpoint.fForward; lateral += endpoint.fLateral;
				if (stage.strStageId == trigger.strCloneEndStageId)
				{ foundEnd = true; stagedDurations[index] = static_cast<std::uint32_t>(duration); }
			}
		}
		if (!child->bFixedTimelineClock || !foundEnd || duration > trigger.iDurationMs || stagedDurations[index] >= duration)
		{ status = "Cross direction child requires a clone end Stage before its full ending, within the Logic duration"; return false; }
		const double x = boss.fPositionX + lateral * std::cos(yaw) + forward * std::sin(yaw);
		const double z = boss.fPositionZ - lateral * std::sin(yaw) + forward * std::cos(yaw);
		const double dx = x - boss.fSpawnPositionX, dz = z - boss.fSpawnPositionZ;
		const double distance = dx * dx + dz * dz;
		if (!std::isfinite(distance) || forward * forward + lateral * lateral < .000001)
		{ status = "Cross direction child has no finite directional movement at the clone end Stage"; return false; }
		// Equal distances keep the authored front/back/left/right order.
		if (distance < bestDistance - .00001) { bestDistance = distance; stagedSelection = index; }
	}
	selected = stagedSelection; cloneDurationsMs = stagedDurations; status.clear(); return true;
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
	boss.PatternStageRootMotion = stage.Motion.RootMotion;
	boss.bPatternStageRootOriginCaptured = false;
	boss.iPatternStageRootLastTick = 0u;
	boss.fPatternStageOriginY = 0.f;
	boss.fPatternStageRootGroundY = 0.f;
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
	boss.KoukuContactLedger.reset();
	boss.KoukuDirectionPlayback.reset();
	boss.iKoukuDirectionEndTick = 0u;
	boss.bKoukuDirectionPlaybackComplete = false;
	boss.PatternTerminalReceipt = {};
	boss.bPatternRootGrounded = false;
	boss.AlbionAirborne = {};
	boss.KoukuSummonStartedTriggers.clear();
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
	boss.PatternStageRootMotion.clear();
	boss.KoukuContactLedger.reset();
	boss.bPatternRootGrounded = false;
	boss.KoukuDirectionPlayback.reset();
	boss.iKoukuDirectionEndTick = 0u;
	boss.bKoukuDirectionPlaybackComplete = false;
	boss.AlbionAirborne = {};
	boss.bPatternStageRootOriginCaptured = false;
	boss.iPatternStageRootLastTick = 0u;
	boss.fPatternStageOriginX = boss.fPatternStageOriginY = boss.fPatternStageOriginZ = 0.f;
	boss.fPatternStageOriginYawDegrees = boss.fPatternStageRootGroundY = 0.f;
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
	if (pattern->bFixedTimelineClock)
	{
		// Parent stage boundaries share the Logic/effect clock. Rounding each
		// separate stage would accumulate delay before the next authored phase.
		const std::uint64_t ticks = serverTick >= boss.iPatternStartTick ? serverTick - boss.iPatternStartTick :
			static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - boss.iPatternStartTick) + serverTick;
		std::uint64_t startMs = 0u;
		std::uint32_t selected = 0u;
		while (selected < pattern->Stages.size() &&
			ticks * MILLISECONDS_PER_SECOND >= (startMs + pattern->Stages[selected].iDurationMs) * SERVER_TICK_HZ)
			startMs += pattern->Stages[selected++].iDurationMs;
		if (selected == pattern->Stages.size())
		{
			Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED);
			status.clear();
			return KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED;
		}
		const bool changed = selected != boss.iPatternStageIndex;
		if (changed) Enter_Stage(boss, pattern->Stages[selected], selected, serverTick, false);
		const auto offset = static_cast<std::uint32_t>((startMs * SERVER_TICK_HZ + MILLISECONDS_PER_SECOND - 1u) / MILLISECONDS_PER_SECOND);
		boss.iActionStartTick = boss.iPatternStartTick + offset;
		if (boss.iActionStartTick < boss.iPatternStartTick) ++boss.iActionStartTick;
		if (boss.iActionStartTick == 0u) boss.iActionStartTick = 1u;
		boss.fActionElapsedSeconds = static_cast<float>((std::max)(0.0,
			static_cast<double>(ticks) / SERVER_TICK_HZ - static_cast<double>(startMs) / MILLISECONDS_PER_SECOND));
		status.clear();
		return changed ? KOUKUSAYDON_BRAIN_UPDATE_RESULT::STAGE_CHANGED : KOUKUSAYDON_BRAIN_UPDATE_RESULT::RUNNING;
	}
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
    if (!motion.Keys.empty())
    {
        if (timeMs <= motion.Keys.front().iTimeMs) return motion.Keys.front().Position;
        if (timeMs >= motion.Keys.back().iTimeMs) return motion.Keys.back().Position;
        const auto next = std::upper_bound(motion.Keys.begin(), motion.Keys.end(), timeMs,
            [](double time, const auto& key) { return time < key.iTimeMs; });
        const auto& previous = *(next - 1);
        const double alpha = (timeMs - previous.iTimeMs) / (next->iTimeMs - previous.iTimeMs);
        std::array<float, 3u> position{};
        for (std::size_t axis = 0u; axis < position.size(); ++axis)
            position[axis] = static_cast<float>(previous.Position[axis] + (next->Position[axis] - previous.Position[axis]) * alpha);
        return position;
    }
    const double alpha = (std::clamp)((timeMs - motion.iStartMs) /
        static_cast<double>(motion.iEndMs - motion.iStartMs), 0.0, 1.0);
    return {
        static_cast<float>(motion.StartPosition[0] + (motion.EndPosition[0] - motion.StartPosition[0]) * alpha),
        motion.StartPosition[1],
        static_cast<float>(motion.StartPosition[2] + (motion.EndPosition[2] - motion.StartPosition[2]) * alpha)};
}

LostArk::Server::ROOT_MOTION_SAMPLE
LostArk::Server::CKoukuSaydonBrain::Sample_StageRootMotion(
	const std::vector<ROOT_MOTION_SAMPLE>& samples, const double timeMs) noexcept
{
	if (samples.empty() || !std::isfinite(timeMs)) return {};
	if (timeMs <= samples.front().iTimeMs) return samples.front();
	if (timeMs >= samples.back().iTimeMs) return samples.back();
	const auto next = std::upper_bound(samples.begin(), samples.end(), timeMs,
		[](const double time, const ROOT_MOTION_SAMPLE& sample) { return time < sample.iTimeMs; });
	const auto& previous = *(next - 1);
	const double alpha = (timeMs - previous.iTimeMs) / (next->iTimeMs - previous.iTimeMs);
	return { static_cast<std::uint32_t>(timeMs),
		static_cast<float>(previous.fForward + (next->fForward - previous.fForward) * alpha),
		static_cast<float>(previous.fLateral + (next->fLateral - previous.fLateral) * alpha),
		static_cast<float>(previous.fUp + (next->fUp - previous.fUp) * alpha) };
}

double LostArk::Server::CKoukuSaydonBrain::Pattern_ElapsedMs(
	const SERVER_WORLD_ENTITY& boss, const std::uint32_t serverTick) noexcept
{
	const std::uint64_t ticks = serverTick >= boss.iPatternStartTick ? serverTick - boss.iPatternStartTick :
	 static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - boss.iPatternStartTick) + serverTick;
	return double(ticks) * 1000.0 / SERVER_TICK_HZ;
}

double LostArk::Server::CKoukuSaydonBrain::Stage_RootTimeMs(
	const SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, const std::uint32_t serverTick) noexcept
{
	if (!pattern.bFixedTimelineClock) return double(Stage_ElapsedTicks(boss, serverTick)) * 1000.0 / SERVER_TICK_HZ;
	std::uint64_t start = 0u;
	for (std::uint32_t index = 0u; index < boss.iPatternStageIndex && index < pattern.Stages.size(); ++index)
	 start += pattern.Stages[index].iDurationMs;
	return (std::max)(0.0, Pattern_ElapsedMs(boss, serverTick) - double(start));
}

bool LostArk::Server::CKoukuSaydonBrain::Sample_AlbionAirborneHeight(
	SERVER_ALBION_AIRBORNE_STATE& state, const double patternMs, const std::uint32_t stageIndex,
	const float sourceUp, float& outHeight) noexcept
{
	using Phase = ALBION_AIRBORNE_PHASE;
	switch (state.ePhase)
	{
	case Phase::JUMP:
	 if (state.iDurationMs == 0u) { outHeight = state.fJumpHeightM; break; }
	 outHeight = static_cast<float>(state.fStartHeightM + (state.fJumpHeightM - state.fStartHeightM) *
	  (std::clamp)((patternMs - state.iStartMs) / state.iDurationMs, 0.0, 1.0));
	 break;
	case Phase::APPEAR_PLAYER:
	 outHeight = stageIndex == state.iSourceStageIndex ? state.fPhaseHeightM + sourceUp : 0.f;
	 break;
	case Phase::DISAPPEAR: case Phase::CENTER:
	 outHeight = state.fJumpHeightM;
	 break;
	case Phase::SLAM:
	 if (state.fSourceUpAtStart - state.fSourceUpMinimum <= .000001f) return false;
	 if (stageIndex != state.iSourceStageIndex) state.fLandingProgress = 1.f;
	 else state.fLandingProgress = (std::max)(state.fLandingProgress,
	  (std::clamp)((state.fSourceUpAtStart - sourceUp) / (state.fSourceUpAtStart - state.fSourceUpMinimum), 0.f, 1.f));
	 outHeight = state.fStartHeightM * (1.f - state.fLandingProgress);
	 break;
	default: return false;
	}
	return std::isfinite(outHeight) && std::abs(outHeight) <= 100000.f;
}

bool LostArk::Server::CKoukuSaydonBrain::Apply_StageRootMotion(
	SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern,
	const std::uint32_t serverTick, const CServerNavigation& navigation,
	const CServerCollisionSystem& collision, std::string& status)
{
	status.clear();
	const bool airborne = boss.AlbionAirborne.iPatternSequence == boss.iPatternSequence &&
		boss.AlbionAirborne.ePhase != ALBION_AIRBORNE_PHASE::NONE &&
		(boss.AlbionAirborne.ePhase != ALBION_AIRBORNE_PHASE::SLAM ||
		 boss.iPatternStageIndex == boss.AlbionAirborne.iSourceStageIndex);
	if ((!airborne && boss.PatternStageRootMotion.empty()) || boss.strPatternId.empty() ||
		boss.iPatternStageRootLastTick == serverTick) return true;
	if (serverTick == 0u || boss.strPatternId != pattern.strPatternId ||
		boss.iPatternStageIndex >= pattern.Stages.size())
	{ status = "KoukuSaydon root motion lost its running stage"; return false; }
	// Capture after the Room commits spawn reset and the stage's ENTER actions.
	SERVER_NAV_POINT originGround;
	if (!boss.bPatternStageRootOriginCaptured)
	{
		if (!std::isfinite(boss.fPositionX) || !std::isfinite(boss.fPositionY) ||
			!std::isfinite(boss.fPositionZ) || !std::isfinite(boss.fYawDegrees) ||
			!navigation.Sample_Position(boss.fPositionX, boss.fPositionZ, originGround))
		{ status = "KoukuSaydon root motion origin is not navigable"; return false; }
		boss.fPatternStageOriginX = boss.fPositionX;
		// Continue the authored jump, but do not carry a previous clip's residual Up
		// into every later stage after an explicit landing. Other patterns retain Y.
		boss.fPatternStageOriginY = boss.bPatternRootGrounded ? originGround.y : boss.fPositionY;
		boss.fPatternStageOriginZ = boss.fPositionZ;
		boss.fPatternStageOriginYawDegrees = boss.fYawDegrees;
		boss.fPatternStageRootGroundY = originGround.y;
		boss.bPatternStageRootOriginCaptured = true;
	}
	const double elapsedMs = Stage_RootTimeMs(boss, pattern, serverTick);
	const auto sample = Sample_StageRootMotion(boss.PatternStageRootMotion, elapsedMs);
	constexpr double RADIANS_PER_DEGREE = 3.14159265358979323846 / 180.0;
	const double yaw = boss.fPatternStageOriginYawDegrees * RADIANS_PER_DEGREE;
	// lateral = model X = the visual front axis of the Kouku/Saydon rigs (MN_RPCZ_00,
	// MN_RPCT_05/06 face model +X; see GameRoom_BossSimulation forwardYawOffset), so a
	// negative lateral sample is the visual recoil. forward = model Z (the side axis).
	SERVER_NAV_POINT destination{
		static_cast<float>(boss.fPatternStageOriginX + sample.fLateral * std::cos(yaw) + sample.fForward * std::sin(yaw)),
		boss.fPatternStageOriginY + sample.fUp,
		static_cast<float>(boss.fPatternStageOriginZ - sample.fLateral * std::sin(yaw) + sample.fForward * std::cos(yaw)) };
	// SLAM lands on the committed target/center anchor; native lateral sway is pose-only.
	if (airborne && boss.AlbionAirborne.ePhase == ALBION_AIRBORNE_PHASE::SLAM)
	{ destination.x = boss.fPositionX; destination.z = boss.fPositionZ; }
	if (!std::isfinite(destination.x) || !std::isfinite(destination.y) || !std::isfinite(destination.z))
	{ status = "KoukuSaydon root motion destination is invalid"; return false; }
	SERVER_NAV_POINT ground;
	if (!navigation.Has_LineOfSight(boss.fPositionX, boss.fPositionZ, destination.x, destination.z) ||
		!navigation.Resolve_TraversalStep(boss.fPositionX, boss.fPositionZ, destination.x, destination.z, ground))
	{
		// Clamp this tick's segment to its farthest navigable point so the boss rests
		// flush at the navigation edge instead of up to one tick of travel short of it.
		// The stage clock keeps running and sampling stays origin-relative, so a curve
		// that later re-enters the grid resumes from origin+sample without drift.
		const double segmentX = double(destination.x) - double(boss.fPositionX);
		const double segmentZ = double(destination.z) - double(boss.fPositionZ);
		const double segmentLength = std::sqrt(segmentX * segmentX + segmentZ * segmentZ);
		constexpr double CLAMP_RESOLUTION_M = 0.001;
		double low = 0.0, high = 1.0;
		bool found = false;
		SERVER_NAV_POINT clamped{};
		for (unsigned iteration = 0u; iteration < 40u && (high - low) * segmentLength > CLAMP_RESOLUTION_M; ++iteration)
		{
			const double mid = (low + high) * 0.5;
			const float candidateX = static_cast<float>(boss.fPositionX + segmentX * mid);
			const float candidateZ = static_cast<float>(boss.fPositionZ + segmentZ * mid);
			SERVER_NAV_POINT candidate{};
			if (navigation.Has_LineOfSight(boss.fPositionX, boss.fPositionZ, candidateX, candidateZ) &&
				navigation.Resolve_TraversalStep(boss.fPositionX, boss.fPositionZ, candidateX, candidateZ, candidate))
			{ low = mid; clamped = candidate; clamped.x = candidateX; clamped.z = candidateZ; found = true; }
			else high = mid;
		}
		// Below one millimetre the whole segment is blocked: hold the exact pose.
		if (!found || low * segmentLength < CLAMP_RESOLUTION_M)
		{
			boss.iPatternStageRootLastTick = serverTick;
			return true; // The animation clock continues while authority holds at the boundary.
		}
		destination.x = clamped.x;
		destination.z = clamped.z;
		ground = clamped;
	}
	// Traversal resolves terrain Y; source up remains relative to the captured stage base.
	SERVER_NAV_POINT startGround;
	if (!navigation.Sample_Position(boss.fPositionX, boss.fPositionZ, startGround))
	{ status = "KoukuSaydon root motion lost its current ground"; return false; }
	const float targetGroundY = ground.y;
	destination.y += targetGroundY - boss.fPatternStageRootGroundY;
	if (boss.bPatternRootGrounded)
		destination.y = (std::max)(destination.y, targetGroundY);
	auto airborneState = boss.AlbionAirborne;
	if (airborne)
	{
		float height = 0.f;
		float airborneSourceUp = sample.fUp;
		if (airborneState.ePhase == ALBION_AIRBORNE_PHASE::SLAM && boss.iPatternStageIndex == airborneState.iSourceStageIndex)
		{
			std::uint64_t stageStart = 0u;
			for (std::uint32_t index = 0u; index < boss.iPatternStageIndex; ++index) stageStart += pattern.Stages[index].iDurationMs;
			const double triggerAge = (std::max)(0.0, double(airborneState.iStartMs) - double(stageStart));
			// A delayed fixed tick still consumes the original minimum it crossed.
			for (const auto& key : boss.PatternStageRootMotion)
				if (key.iTimeMs >= triggerAge && key.iTimeMs <= elapsedMs) airborneSourceUp = (std::min)(airborneSourceUp, key.fUp);
		}
		if (!Sample_AlbionAirborneHeight(airborneState, Pattern_ElapsedMs(boss, serverTick), boss.iPatternStageIndex, airborneSourceUp, height))
		{ status = "Albion airborne height lost its validated phase"; return false; }
		destination.y = targetGroundY + height;
	}
	const SERVER_NAV_POINT proposed = destination;
	bool blocked = false;
	if (!collision.Resolve_CircleMove(boss.fPositionX, boss.fPositionY, boss.fPositionZ,
		destination.x, destination.y, destination.z, boss.fCollisionRadius, boss.fCollisionRadius,
		boss.fCollisionRadius, destination.x, destination.y, destination.z, blocked, boss.iNetEntityId, false))
	{ status = "KoukuSaydon root motion collision resolution failed"; return false; }
	if (!navigation.Has_LineOfSight(boss.fPositionX, boss.fPositionZ, destination.x, destination.z) ||
		!navigation.Resolve_TraversalStep(boss.fPositionX, boss.fPositionZ, destination.x, destination.z, ground))
	{
		boss.iPatternStageRootLastTick = serverTick;
		return true;
	}
	if (blocked)
	{
		// The sweep truncates XYZ together. Replace only its interpolated terrain
		// component with ground at the resolved XZ, preserving the clipped source up.
		const double dx = proposed.x - boss.fPositionX, dy = proposed.y - boss.fPositionY,
			dz = proposed.z - boss.fPositionZ;
		const double squaredDistance = dx * dx + dy * dy + dz * dz;
		const double ratio = squaredDistance > 1e-12 ? (std::clamp)(
			((destination.x - boss.fPositionX) * dx + (destination.y - boss.fPositionY) * dy +
			 (destination.z - boss.fPositionZ) * dz) / squaredDistance, 0.0, 1.0) : 0.0;
		const float terrainCorrection = ground.y - static_cast<float>(startGround.y + (targetGroundY - startGround.y) * ratio);
		if (std::abs(terrainCorrection) > .00001f)
		{
			bool correctionBlocked = false;
			SERVER_NAV_POINT corrected;
			if (!collision.Resolve_CircleMove(destination.x, destination.y, destination.z,
				destination.x, destination.y + terrainCorrection, destination.z, boss.fCollisionRadius,
				boss.fCollisionRadius, boss.fCollisionRadius, corrected.x, corrected.y, corrected.z,
				correctionBlocked, boss.iNetEntityId, false))
			{ status = "KoukuSaydon root ground correction failed collision validation"; return false; }
			if (correctionBlocked)
			{ boss.iPatternStageRootLastTick = serverTick; return true; }
			destination = corrected;
		}
	}
	// Commit XYZ together only after both navigation and the body sweep succeed.
	boss.fPositionX = destination.x; boss.fPositionY = destination.y; boss.fPositionZ = destination.z;
	if (airborne) boss.AlbionAirborne = airborneState;
	boss.iPatternStageRootLastTick = serverTick;
	return true;
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
