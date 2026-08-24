#include "ValtanBrain.h"

#include "BossCombatRuntime.h"
#include "Gameplay/CombatCollisionContract.h"
#include "Gameplay/WorldCollisionContract.h"
#include "PlayerSkillSystem.h"
#include "ServerCombatHitRuntime.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
	using namespace LostArk::Server;

	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
	constexpr float DEGREES_TO_RADIANS = 0.0174532925f;
	constexpr float MILLISECONDS_TO_SECONDS = 0.001f;
	constexpr std::uint32_t SERVER_TICK_HZ = 30u;
	constexpr std::uint64_t MILLISECONDS_PER_SECOND = 1000u;
	constexpr float PATH_POINT_STOP_DISTANCE = 0.1f;
	/* The converted Valtan model carries no jump clip, so the leap is a Server
	transform arc instead of root motion. Both the apex and the landing point
	come from the pattern's compiled serverMotion anchor, never from a constant
	here and never from the boss placement. */

	/* A leap is declared by the pattern's compiled motion rather than by its
	name, so any pattern that authors one gets the same arc. The rise is always
	the first stage; the authored travel stage owns descent, and any stages in
	between hold the boss at the apex. */
	bool Is_LeapPattern(const BOSS_PATTERN_DEFINITION& pattern)
	{
		return BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR == pattern.Motion.eKind ||
			BOSS_PATTERN_MOTION_KIND::LEAP_TO_TARGET == pattern.Motion.eKind;
	}

	constexpr std::uint32_t LEAP_TAKEOFF_STAGE_INDEX = 0u;

	std::uint32_t NextServerTickSkippingReservedZero(
		const std::uint32_t serverTick)
	{
		return (std::numeric_limits<std::uint32_t>::max)() == serverTick ?
			1u : serverTick + 1u;
	}

	std::uint64_t StageElapsedTicks(
		const SERVER_WORLD_ENTITY& boss,
		const std::uint32_t serverTick)
	{
		const std::uint32_t firstTick =
			boss.iPatternStageFirstEvaluationTick;
		if (0u == firstTick || 0u == serverTick)
			return 0u;
		/* The Server reserves zero and wraps UINT32_MAX directly to one. Count
		only that nonzero tick cardinality, then include the first evaluation. */
		const std::uint64_t ageTicks = serverTick >= firstTick ?
			static_cast<std::uint64_t>(serverTick - firstTick) :
			static_cast<std::uint64_t>(
				(std::numeric_limits<std::uint32_t>::max)() - firstTick) +
				static_cast<std::uint64_t>(serverTick);
		return ageTicks + 1u;
	}

	bool HasElapsedMilliseconds(
		const std::uint64_t elapsedTicks,
		const std::uint32_t milliseconds)
	{
		return static_cast<std::uint64_t>(elapsedTicks) *
			MILLISECONDS_PER_SECOND >=
			static_cast<std::uint64_t>(milliseconds) * SERVER_TICK_HZ;
	}

	bool Is_LeapArcStage(const SERVER_WORLD_ENTITY& boss)
	{
		return boss.fPatternLeapApexHeight > 0.f &&
			(LEAP_TAKEOFF_STAGE_INDEX == boss.iPatternStageIndex ||
				boss.iPatternLeapTravelStageIndex == boss.iPatternStageIndex);
	}

	/* Stage progress in [0,1]. A zero-length stage reads as finished so the
	arc never divides by it. */
	float StageRatio(const SERVER_WORLD_ENTITY& boss)
	{
		if (0u == boss.iPatternStageDurationMs)
			return 1.f;
		const float duration =
			static_cast<float>(boss.iPatternStageDurationMs) *
			MILLISECONDS_TO_SECONDS;
		if (!std::isfinite(boss.fActionElapsedSeconds) || duration <= 0.f)
			return 1.f;
		return (std::min)(1.f, (std::max)(0.f,
			boss.fActionElapsedSeconds / duration));
	}

	/* TAKEOFF rises in place; DROP carries the boss to the pattern's compiled
	landing anchor while it falls, so IMPACT always lands on the same point the
	camera frames and the walls fly away from. */
	void Advance_ArenaBreakLeap(SERVER_WORLD_ENTITY& boss)
	{
		if (!Is_LeapArcStage(boss))
			return;
		const float ratio = StageRatio(boss);
		const float apex = boss.fLeapApexHeight;
		if (LEAP_TAKEOFF_STAGE_INDEX == boss.iPatternStageIndex)
		{
			boss.fPositionX = boss.fLeapOriginX;
			boss.fPositionZ = boss.fLeapOriginZ;
			boss.fPositionY = boss.fLeapOriginY + apex * ratio;
			return;
		}
		boss.fPositionX = boss.fLeapOriginX +
			(boss.fLeapLandingX - boss.fLeapOriginX) * ratio;
		boss.fPositionZ = boss.fLeapOriginZ +
			(boss.fLeapLandingZ - boss.fLeapOriginZ) * ratio;
		const float apexY = boss.fLeapOriginY + apex;
		boss.fPositionY = apexY + (boss.fLeapLandingY - apexY) * ratio * ratio;
	}

	void Transition(
		SERVER_WORLD_ENTITY& boss,
		const SERVER_ENTITY_ACTION action,
		const std::uint32_t serverTick)
	{
		if (boss.eAction == action)
			return;
		boss.eAction = action;
		boss.fActionElapsedSeconds = 0.f;
		boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
	}

	bool ContainsPatternId(
		const std::vector<std::string>& ids,
		const std::string& patternId)
	{
		return ids.end() != std::find(ids.begin(), ids.end(), patternId);
	}

	bool IsSamePatternCooldownFamily(
		const SERVER_BOSS_PATTERN_COOLDOWN& cooldown,
		const BOSS_PATTERN_DEFINITION& pattern)
	{
		if (0u != pattern.iSourcePrimaryActionId)
		{
			return cooldown.iSourcePrimaryActionId ==
				pattern.iSourcePrimaryActionId;
		}
		return 0u == cooldown.iSourcePrimaryActionId &&
			cooldown.strPatternId == pattern.strPatternId;
	}

	bool IsPatternCooldownReady(
		const SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		const std::uint32_t serverTick)
	{
		const auto found = std::find_if(
			boss.PatternCooldowns.begin(), boss.PatternCooldowns.end(),
			[&pattern](const SERVER_BOSS_PATTERN_COOLDOWN& cooldown)
			{ return IsSamePatternCooldownFamily(cooldown, pattern); });
		return boss.PatternCooldowns.end() == found ||
			static_cast<std::int32_t>(serverTick - found->iReadyTick) >= 0;
	}

	void StartPatternCooldown(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		const std::uint32_t serverTick)
	{
		if (0u == pattern.iSourceCooldownTicks)
			return;
		const std::uint32_t readyTick =
			serverTick + pattern.iSourceCooldownTicks;
		auto found = std::find_if(
			boss.PatternCooldowns.begin(), boss.PatternCooldowns.end(),
			[&pattern](const SERVER_BOSS_PATTERN_COOLDOWN& cooldown)
			{ return IsSamePatternCooldownFamily(cooldown, pattern); });
		if (boss.PatternCooldowns.end() == found)
		{
			SERVER_BOSS_PATTERN_COOLDOWN cooldown{};
			cooldown.iSourcePrimaryActionId = pattern.iSourcePrimaryActionId;
			if (0u == pattern.iSourcePrimaryActionId)
				cooldown.strPatternId = pattern.strPatternId;
			cooldown.iReadyTick = readyTick;
			boss.PatternCooldowns.push_back(std::move(cooldown));
			return;
		}
		found->iReadyTick = readyTick;
	}

	const BOSS_PATTERN_DEFINITION* FindPattern(
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns,
		const std::string& patternId)
	{
		const auto found = std::find_if(patterns.begin(), patterns.end(),
			[&patternId](const BOSS_PATTERN_DEFINITION& pattern)
			{ return pattern.strPatternId == patternId; });
		return patterns.end() == found ? nullptr : &*found;
	}

	const BOSS_PATTERN_STAGE_DEFINITION* FindStageByActionId(
		const BOSS_PATTERN_DEFINITION& pattern,
		const std::string& actionId,
		std::uint32_t& outStageIndex)
	{
		for (std::size_t index = 0u; index < pattern.Stages.size(); ++index)
		{
			if (pattern.Stages[index].strActionId != actionId)
				continue;
			outStageIndex = static_cast<std::uint32_t>(index);
			return &pattern.Stages[index];
		}
		return nullptr;
	}

	bool IsTargetable(const SERVER_PLAYER& player)
	{
		return 0u != player.iCurrentHp && player.isCombatReady &&
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD != player.eAction &&
			LostArk::Shared::PLAYER_ACTION_STATE::FALLING != player.eAction;
	}

	SERVER_PLAYER* FindPatternTarget(
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		const LostArk::Shared::NET_ENTITY_ID netEntityId)
	{
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			if (player.iNetEntityId == netEntityId && IsTargetable(player))
				return &player;
		}
		return nullptr;
	}

	void RememberPatternTargetPosition(
		SERVER_WORLD_ENTITY& boss,
		const SERVER_PLAYER& target)
	{
		boss.bHasPatternTargetLastPosition = true;
		boss.fPatternTargetLastPositionX = target.fPositionX;
		boss.fPatternTargetLastPositionY = target.fPositionY;
		boss.fPatternTargetLastPositionZ = target.fPositionZ;
	}

	void ClearPatternTargetPosition(SERVER_WORLD_ENTITY& boss)
	{
		boss.bHasPatternTargetLastPosition = false;
		boss.fPatternTargetLastPositionX = 0.f;
		boss.fPatternTargetLastPositionY = 0.f;
		boss.fPatternTargetLastPositionZ = 0.f;
	}

	void FacePoint(SERVER_WORLD_ENTITY& boss, const float x, const float z)
	{
		const float deltaX = x - boss.fPositionX;
		const float deltaZ = z - boss.fPositionZ;
		if (deltaX * deltaX + deltaZ * deltaZ <= 0.000001f)
			return;
		boss.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
	}

	SERVER_PLAYER* SelectRandomTarget(
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		const std::uint32_t patternSequence)
	{
		std::vector<SERVER_PLAYER*> candidates;
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			if (IsTargetable(player))
				candidates.push_back(&player);
		}
		if (candidates.empty())
			return nullptr;
		std::sort(candidates.begin(), candidates.end(),
			[](const SERVER_PLAYER* left, const SERVER_PLAYER* right)
			{
				return left->iPlayerId < right->iPlayerId;
			});
		std::uint64_t random =
			static_cast<std::uint64_t>(patternSequence) + 0x9e3779b97f4a7c15ull;
		random ^= random >> 30u;
		random *= 0xbf58476d1ce4e5b9ull;
		random ^= random >> 27u;
		random *= 0x94d049bb133111ebull;
		random ^= random >> 31u;
		return candidates[static_cast<std::size_t>(random % candidates.size())];
	}

	void BeginPatternTargetAndAim(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		SERVER_PLAYER* nearestTarget)
	{
		SERVER_PLAYER* selected = nullptr;
		switch (pattern.eTargetPolicy)
		{
		case BOSS_PATTERN_TARGET_POLICY::NEAREST_EACH_TICK:
		case BOSS_PATTERN_TARGET_POLICY::LOCK_NEAREST_ON_START:
			selected = nearestTarget;
			break;
		case BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_ON_START:
			selected = SelectRandomTarget(players, boss.iPatternSequence);
			break;
		default:
			break;
		}
		boss.iPatternTargetEntityId = nullptr == selected ?
			LostArk::Shared::INVALID_NET_ENTITY_ID : selected->iNetEntityId;
		if (nullptr != selected)
			RememberPatternTargetPosition(boss, *selected);
		else
			ClearPatternTargetPosition(boss);

		if (BOSS_PATTERN_AIM_POLICY::FACE_MOTION_ANCHOR == pattern.eAimPolicy &&
			BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR == pattern.Motion.eKind)
		{
			FacePoint(boss, pattern.Motion.fLandingX, pattern.Motion.fLandingZ);
		}
		else if (nullptr != selected &&
			(BOSS_PATTERN_AIM_POLICY::TRACK_TARGET_EACH_TICK == pattern.eAimPolicy ||
			 BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START == pattern.eAimPolicy))
		{
			FacePoint(boss, selected->fPositionX, selected->fPositionZ);
		}
	}

	void UpdatePatternTargetAndAim(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		SERVER_PLAYER* nearestTarget)
	{
		if (BOSS_PATTERN_TARGET_POLICY::NEAREST_EACH_TICK ==
			pattern.eTargetPolicy)
		{
			boss.iPatternTargetEntityId = nullptr == nearestTarget ?
				LostArk::Shared::INVALID_NET_ENTITY_ID :
				nearestTarget->iNetEntityId;
		}
		SERVER_PLAYER* patternTarget =
			FindPatternTarget(players, boss.iPatternTargetEntityId);
		if (nullptr != patternTarget)
			RememberPatternTargetPosition(boss, *patternTarget);
		if (BOSS_PATTERN_AIM_POLICY::TRACK_TARGET_EACH_TICK ==
			pattern.eAimPolicy && nullptr != patternTarget)
		{
			FacePoint(
				boss, patternTarget->fPositionX, patternTarget->fPositionZ);
		}
	}

	void QueueCrossedHealthBarPatterns(
		SERVER_WORLD_ENTITY& boss,
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns,
		const std::uint32_t currentHealthBar)
	{
		std::vector<const BOSS_PATTERN_DEFINITION*> crossed;
		for (const BOSS_PATTERN_DEFINITION& pattern : patterns)
		{
			if (BOSS_PATTERN_SELECTION::HEALTH_BAR != pattern.eSelection ||
				boss.iPhase < pattern.iMinimumPhase ||
				boss.iPhase > pattern.iMaximumPhase ||
				ContainsPatternId(boss.TriggeredPatternIds, pattern.strPatternId) ||
				boss.iLastEvaluatedHealthBar <= pattern.iTriggerHealthBar ||
				currentHealthBar > pattern.iTriggerHealthBar)
			{
				continue;
			}
			crossed.push_back(&pattern);
		}
		std::sort(crossed.begin(), crossed.end(),
			[](const BOSS_PATTERN_DEFINITION* left,
				const BOSS_PATTERN_DEFINITION* right)
			{
				if (left->iTriggerHealthBar != right->iTriggerHealthBar)
					return left->iTriggerHealthBar > right->iTriggerHealthBar;
				if (left->iTriggerOrder != right->iTriggerOrder)
					return left->iTriggerOrder < right->iTriggerOrder;
				return left->strPatternId < right->strPatternId;
			});
		for (const BOSS_PATTERN_DEFINITION* pattern : crossed)
		{
			boss.PendingPatternIds.push_back(pattern->strPatternId);
			boss.TriggeredPatternIds.push_back(pattern->strPatternId);
		}
		boss.iLastEvaluatedHealthBar = currentHealthBar;
	}

	const BOSS_PATTERN_DEFINITION* SelectNormalPattern(
		const SERVER_WORLD_ENTITY& boss,
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns,
		const std::string& introPatternId,
		const std::uint32_t currentHealthBar,
		const float targetDistance,
		const std::uint32_t serverTick,
		const std::vector<std::string>* eligiblePatternIds = nullptr)
	{
		std::vector<const BOSS_PATTERN_DEFINITION*> eligible;
		std::vector<const BOSS_PATTERN_DEFINITION*> repeatAllowed;
		for (const BOSS_PATTERN_DEFINITION& pattern : patterns)
		{
			if (BOSS_PATTERN_SELECTION::NORMAL != pattern.eSelection ||
				pattern.strPatternId == introPatternId ||
				(nullptr != eligiblePatternIds &&
					eligiblePatternIds->end() == std::find(
						eligiblePatternIds->begin(), eligiblePatternIds->end(),
						pattern.strPatternId)) ||
				!CValtanBrain::Is_ArmorRequirementMet(
					boss, pattern.eArmorRequirement) ||
				!CValtanBrain::Is_PhaseRequirementMet(
					boss, pattern.ePhaseRequirement) ||
				boss.iPhase < pattern.iMinimumPhase ||
				boss.iPhase > pattern.iMaximumPhase ||
				currentHealthBar < pattern.iMinimumHealthBar ||
				currentHealthBar > pattern.iMaximumHealthBar ||
				targetDistance < pattern.fMinimumRange ||
				targetDistance > pattern.fMaximumRange ||
				!IsPatternCooldownReady(boss, pattern, serverTick))
			{
				continue;
			}
			eligible.push_back(&pattern);
			if (pattern.strPatternId != boss.strLastPatternId ||
				boss.iConsecutivePatternUses < pattern.iMaximumConsecutiveUses)
			{
				repeatAllowed.push_back(&pattern);
			}
		}
		const auto& candidates = repeatAllowed.empty() ? eligible : repeatAllowed;
		std::uint64_t totalWeight = 0u;
		for (const BOSS_PATTERN_DEFINITION* pattern : candidates)
			totalWeight += pattern->iSelectionWeight;
		if (0u == totalWeight)
			return nullptr;

		std::uint64_t random =
			(static_cast<std::uint64_t>(serverTick) << 32u) |
			static_cast<std::uint64_t>(boss.iPatternSequence + 1u);
		random ^= random >> 30u;
		random *= 0xbf58476d1ce4e5b9ull;
		random ^= random >> 27u;
		random *= 0x94d049bb133111ebull;
		random ^= random >> 31u;
		std::uint64_t ticket = random % totalWeight;
		for (const BOSS_PATTERN_DEFINITION* pattern : candidates)
		{
			if (ticket < pattern->iSelectionWeight)
				return pattern;
			ticket -= pattern->iSelectionWeight;
		}
		return candidates.back();
	}

	const BOSS_PATTERN_DEFINITION* SelectPattern(
		SERVER_WORLD_ENTITY& boss,
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns,
		const std::string& introPatternId,
		const BOSS_PATTERN_ROTATION_DEFINITION* rotation,
		const std::uint32_t currentHealthBar,
		const float targetDistance,
		const std::uint32_t serverTick)
	{
		/* The first appearance runs before the health-bar queue and before any
		weighted roll, so the entrance sweep can never come up again later. It
		waits at the spawn for its own authored range instead of burning on the
		tick the encounter activates, because the arena trigger sits far from the
		boss and the entrance is authored around the spawn point. A missing
		pattern is consumed anyway, so a broken catalog cannot stall the boss on
		every tick. */
		if (!boss.bIntroPatternConsumed)
		{
			const BOSS_PATTERN_DEFINITION* intro = introPatternId.empty() ?
				nullptr : FindPattern(patterns, introPatternId);
			if (nullptr == intro)
			{
				boss.bIntroPatternConsumed = true;
			}
			else if (targetDistance >= intro->fMinimumRange &&
				targetDistance <= intro->fMaximumRange)
			{
				boss.bIntroPatternConsumed = true;
				return intro;
			}
			else if (boss.iCurrentHp < boss.iMaximumHp)
			{
				/* Combat started without the entrance ever coming into range, so it
				is dropped instead of holding a boss that is already being hit. */
				boss.bIntroPatternConsumed = true;
			}
			else
			{
				/* Nothing may run ahead of the entrance, so the pending intro
				owns this tick even though it selected no pattern. */
				return nullptr;
			}
		}
		while (!boss.PendingPatternIds.empty())
		{
			const std::string patternId = boss.PendingPatternIds.front();
			boss.PendingPatternIds.erase(boss.PendingPatternIds.begin());
			if (const BOSS_PATTERN_DEFINITION* pattern =
				FindPattern(patterns, patternId))
			{
				return pattern;
			}
		}
		/* Phase-one master ranges are weighted from their first normal choice.
		PatternIds is a whitelist, while each pattern continues to own its weight,
		range, phase, armour, cooldown, and consecutive-use conditions. Scripted
		health-bar mechanics already returned from the queue above. */
		if (!boss.bScriptedPatternPlayback && nullptr != rotation &&
			BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
				rotation->eSelectionMode)
		{
			boss.strRotationId = rotation->strRotationId;
			boss.iRotationStepIndex = 0u;
			return SelectNormalPattern(
				boss, patterns, introPatternId, currentHealthBar,
				targetDistance, serverTick, &rotation->PatternIds);
		}
		/* The authored list introduces the band rather than looping it. Every
		pattern the band brings is shown once, in order, so nothing the encounter
		adds can be missed; the stretch then hands over to the weighted roll, which
		draws from the whole eligible moveset instead of the list alone. A mechanic
		that interrupts the introduction resumes where it left off because the
		cursor lives on the boss, and a new band restarts it. */
		if (!boss.bScriptedPatternPlayback && nullptr != rotation &&
			BOSS_PATTERN_ROTATION_SELECTION_MODE::
				ORDERED_INTRO_THEN_WEIGHTED == rotation->eSelectionMode &&
			!rotation->PatternIds.empty())
		{
			if (boss.strRotationId != rotation->strRotationId)
			{
				boss.strRotationId = rotation->strRotationId;
				boss.iRotationStepIndex = 0u;
			}
			if (boss.iRotationStepIndex <
				static_cast<std::uint32_t>(rotation->PatternIds.size()))
			{
				const std::size_t stepIndex = boss.iRotationStepIndex;
				++boss.iRotationStepIndex;
				if (const BOSS_PATTERN_DEFINITION* step =
					FindPattern(patterns, rotation->PatternIds[stepIndex]))
				{
					return step;
				}
				/* A step the catalog cannot resolve is skipped rather than
				stalling the introduction, and the weighted roll covers the gap. */
			}
		}
		return SelectNormalPattern(
			boss, patterns, introPatternId,
			currentHealthBar, targetDistance, serverTick);
	}

	/* A GROGGY stage is the stun a charge earns by meeting a wall, so only an
	impact may enter it. A charge that runs its full travel without hitting
	anything falls through to the recovery behind the stun instead. */
	std::uint32_t NextClockStageIndex(
		const BOSS_PATTERN_DEFINITION& pattern,
		const std::uint32_t currentStageIndex)
	{
		std::uint32_t next = currentStageIndex + 1u;
		while (next < pattern.Stages.size() &&
			Is_EventEnteredStage(pattern.Stages[next].eStageKind))
		{
			++next;
		}
		return next;
	}

	SERVER_ENTITY_ACTION ToServerAction(const BOSS_PATTERN_STAGE_KIND kind)
	{
		switch (kind)
		{
		case BOSS_PATTERN_STAGE_KIND::WINDUP:
			return SERVER_ENTITY_ACTION::PATTERN_WINDUP;
		case BOSS_PATTERN_STAGE_KIND::ACTIVE:
			return SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
		case BOSS_PATTERN_STAGE_KIND::RECOVERY:
			return SERVER_ENTITY_ACTION::PATTERN_RECOVERY;
		case BOSS_PATTERN_STAGE_KIND::PART_BREAK:
			/* The boss is reacting, not attacking, so it replicates as the
			recovery the client already renders for that. */
			return SERVER_ENTITY_ACTION::PATTERN_RECOVERY;
		case BOSS_PATTERN_STAGE_KIND::GROGGY:
			/* The stage still plays its authored action, so it replicates as the
			same active stage the client already renders. Only the server needs to
			know this one is the armour-break window. */
			return SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
		default:
			return SERVER_ENTITY_ACTION::PATTERN_WINDUP;
		}
	}

	void EnterPatternStage(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_STAGE_DEFINITION& stage,
		const std::uint32_t stageIndex,
		const std::uint32_t serverTick,
		const bool evaluatesOnEntryTick = false)
	{
		const std::string previousActionId = boss.strActionId;
		if (!previousActionId.empty())
			CBossCombatRuntime::Discard_PatternOutcomes(
				boss, previousActionId);
		boss.iPatternStageIndex = stageIndex;
		boss.strPatternStageId = stage.strStageId;
		boss.iPatternStageDurationMs = stage.iDurationMs;
		boss.iPatternStageFirstEvaluationTick = evaluatesOnEntryTick ?
			serverTick : NextServerTickSkippingReservedZero(serverTick);
		boss.ePatternStageMotionKind = stage.Motion.eKind;
		boss.strActionId = stage.strActionId;
		boss.strDamageProfileId = stage.strDamageProfileId;
		boss.ePatternHitShape = stage.eHitShape;
		boss.fPatternHitOuterRadius = stage.fHitOuterRadius;
		boss.fPatternHitInnerRadius = stage.fHitInnerRadius;
		boss.fPatternHitAngleDegrees = stage.fHitAngleDegrees;
		boss.fPatternHitLength = stage.fHitLength;
		boss.fPatternHitHalfWidth = stage.fHitHalfWidth;
		boss.iPatternHitCount = stage.iHitCount;
		boss.iPatternHitIntervalMs = stage.iHitIntervalMs;
		boss.iPatternHitDelayMs = stage.iHitDelayMs;
		boss.iAppliedPatternHitCount = 0u;
		boss.bPatternWallContact = stage.bWallContact;
		boss.bPatternPiercesCover = stage.bPiercesCover;
		boss.fPatternPushRangeM = stage.fPushRangeM;
		boss.iPatternPushMs = stage.iPushMs;
		boss.bPatternKnockdown = stage.bKnockdown;
		boss.iPatternDownMs = stage.iDownMs;
		boss.bPatternGroggy =
			BOSS_PATTERN_STAGE_KIND::GROGGY == stage.eStageKind;
		boss.bPatternChargeImpact = stage.bChargeImpact;
		const float durationSeconds =
			static_cast<float>(stage.iDurationMs) * MILLISECONDS_TO_SECONDS;
		boss.fPatternForcedMotionSpeed =
			BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD == stage.Motion.eKind &&
			durationSeconds > 0.f ?
			stage.Motion.fDistance / durationSeconds : 0.f;
		boss.PatternStageRootMotion = stage.Motion.RootMotion;
		boss.eAction = ToServerAction(stage.eStageKind);
		boss.fActionElapsedSeconds = 0.f;
		boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
		if (boss.fPatternLeapApexHeight > 0.f &&
			LEAP_TAKEOFF_STAGE_INDEX == stageIndex)
		{
			boss.fLeapOriginX = boss.fPositionX;
			boss.fLeapOriginY = boss.fPositionY;
			boss.fLeapOriginZ = boss.fPositionZ;
			boss.fLeapApexHeight = boss.fPatternLeapApexHeight;
			boss.MovePath.clear();
		}
		else if (boss.fPatternLeapApexHeight > 0.f &&
			stageIndex < boss.iPatternLeapTravelStageIndex)
		{
			/* An authored airborne hold keeps the same apex and horizontal origin
			   until the descent stage begins. */
			boss.fPositionX = boss.fLeapOriginX;
			boss.fPositionY = boss.fLeapOriginY + boss.fLeapApexHeight;
			boss.fPositionZ = boss.fLeapOriginZ;
		}
		else if (boss.fPatternLeapApexHeight > 0.f &&
			stageIndex > boss.iPatternLeapTravelStageIndex)
		{
			/* Everything from IMPACT onward is played from the compiled landing
			anchor, so the landing is exact rather than wherever the last
			interpolated frame happened to leave the boss. */
			boss.fPositionX = boss.fLeapLandingX;
			boss.fPositionY = boss.fLeapLandingY;
			boss.fPositionZ = boss.fLeapLandingZ;
			boss.fLeapApexHeight = 0.f;
		}
		Advance_ArenaBreakLeap(boss);
	}

	void BeginPattern(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		SERVER_PLAYER* nearestTarget,
		const std::uint32_t serverTick)
	{
		boss.strPatternId = pattern.strPatternId;
		boss.bPatternInvulnerable = pattern.bInvulnerableWhileRunning;
		boss.fPatternMinimumRange = pattern.fMinimumRange;
		boss.fPatternMaximumRange = pattern.fMaximumRange;
		StartPatternCooldown(boss, pattern, serverTick);
		boss.iPatternSequence = boss.iPatternSequence ==
			(std::numeric_limits<std::uint32_t>::max)() ?
			1u : boss.iPatternSequence + 1u;
		if (boss.strLastPatternId == pattern.strPatternId)
			++boss.iConsecutivePatternUses;
		else
		{
			boss.strLastPatternId = pattern.strPatternId;
			boss.iConsecutivePatternUses = 1u;
		}
		/* The target has to be locked before the landing is chosen, because a
		leap that follows its target lands where that lock put it. */
		BeginPatternTargetAndAim(boss, pattern, players, nearestTarget);
		/* A pattern that owns a compiled landing anchor lands on it, and one that
		follows its target lands where the lock found it. The authored position is
		the anchor a targetless leap falls back to, so the arc always has a real
		destination. Everything else keeps standing on its authored placement. */
		if (BOSS_PATTERN_MOTION_KIND::LEAP_TO_TARGET == pattern.Motion.eKind &&
			boss.bHasPatternTargetLastPosition)
		{
			boss.fLeapLandingX = boss.fPatternTargetLastPositionX;
			boss.fLeapLandingY = boss.fPatternTargetLastPositionY;
			boss.fLeapLandingZ = boss.fPatternTargetLastPositionZ;
			boss.fPatternLeapApexHeight = pattern.Motion.fApexHeight;
			boss.iPatternLeapTravelStageIndex =
				pattern.Motion.iTravelStageIndex;
		}
		else if (Is_LeapPattern(pattern))
		{
			boss.fLeapLandingX = pattern.Motion.fLandingX;
			boss.fLeapLandingY = pattern.Motion.fLandingY;
			boss.fLeapLandingZ = pattern.Motion.fLandingZ;
			boss.fPatternLeapApexHeight = pattern.Motion.fApexHeight;
			boss.iPatternLeapTravelStageIndex =
				pattern.Motion.iTravelStageIndex;
		}
		else
		{
			boss.fLeapLandingX = boss.fSpawnPositionX;
			boss.fLeapLandingY = boss.fSpawnPositionY;
			boss.fLeapLandingZ = boss.fSpawnPositionZ;
			boss.fPatternLeapApexHeight = 0.f;
			boss.iPatternLeapTravelStageIndex = 1u;
		}
		/* BeginPattern continues through Update below, so the first stage consumes
		this entry tick. Every other stage entry returns or happens at the end of
		the current Update and starts evaluating on the following tick. */
		EnterPatternStage(
			boss, pattern.Stages.front(), 0u, serverTick, true);
	}

	/* Linear between the two samples that bracket the time, holding the ends.
	The curve is authored in metres of forward travel from the stage start. */
	void Sample_StageRootMotionForward(
		const std::vector<ROOT_MOTION_SAMPLE>& samples,
		const float elapsedSeconds,
		float& outForward)
	{
		outForward = 0.f;
		if (samples.empty() || !std::isfinite(elapsedSeconds))
			return;
		const float elapsedMs = (std::max)(0.f, elapsedSeconds) * 1000.f;
		if (elapsedMs <= static_cast<float>(samples.front().iTimeMs))
		{
			outForward = samples.front().fForward;
			return;
		}
		for (std::size_t index = 1u; index < samples.size(); ++index)
		{
			const ROOT_MOTION_SAMPLE& previous = samples[index - 1u];
			const ROOT_MOTION_SAMPLE& current = samples[index];
			if (elapsedMs > static_cast<float>(current.iTimeMs))
				continue;
			const float span = static_cast<float>(
				current.iTimeMs - previous.iTimeMs);
			if (span <= 0.f)
			{
				outForward = current.fForward;
				return;
			}
			const float ratio =
				(elapsedMs - static_cast<float>(previous.iTimeMs)) / span;
			outForward = previous.fForward +
				(current.fForward - previous.fForward) * ratio;
			return;
		}
		outForward = samples.back().fForward;
	}

	void FinishPattern(
		SERVER_WORLD_ENTITY& boss,
		const std::uint32_t serverTick)
	{
		if (boss.fLeapApexHeight > 0.f)
		{
			/* An aborted leap must not leave the boss standing in mid-air, so
			drop it back onto the anchor it was falling toward. */
			boss.fPositionX = boss.fLeapLandingX;
			boss.fPositionY = boss.fLeapLandingY;
			boss.fPositionZ = boss.fLeapLandingZ;
			boss.fLeapApexHeight = 0.f;
		}
		boss.strPatternId.clear();
		boss.strPatternStageId.clear();
		boss.strActionId.clear();
		boss.strDamageProfileId.clear();
		boss.iPatternStageIndex = 0u;
		boss.iPatternStageDurationMs = 0u;
		boss.iPatternStageFirstEvaluationTick = 0u;
		boss.fPatternForcedMotionSpeed = 0.f;
		boss.iPatternLeapTravelStageIndex = 1u;
		boss.ePatternStageMotionKind = BOSS_PATTERN_STAGE_MOTION_KIND::NONE;
		boss.PatternStageRootMotion.clear();
		boss.ePatternHitShape = BOSS_PATTERN_HIT_SHAPE::NONE;
		boss.iPatternHitCount = 0u;
		boss.iPatternHitDelayMs = 0u;
		boss.iAppliedPatternHitCount = 0u;
		boss.bPatternWallContact = false;
		boss.bPatternPiercesCover = false;
		boss.fPatternPushRangeM = 0.f;
		boss.iPatternPushMs = 0u;
		boss.bPatternKnockdown = false;
		boss.iPatternDownMs = 0u;
		boss.bPatternGroggy = false;
		boss.bPatternChargeImpact = false;
		boss.bPatternInvulnerable = false;
		boss.bPendingArmorBreakReaction = false;
		boss.iPatternTargetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		ClearPatternTargetPosition(boss);
		CBossCombatRuntime::Clear_PatternOutcomes(boss);
		Transition(boss, SERVER_ENTITY_ACTION::IDLE, serverTick);
	}

	bool ApplyStageBranch(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		const BOSS_PATTERN_STAGE_BRANCH& branch,
		const std::uint32_t serverTick)
	{
		if (branch.strNextActionId.empty())
		{
			FinishPattern(boss, serverTick);
			return true;
		}
		std::uint32_t nextStageIndex = 0u;
		const BOSS_PATTERN_STAGE_DEFINITION* next =
			FindStageByActionId(pattern, branch.strNextActionId, nextStageIndex);
		if (nullptr == next)
		{
			/* The publisher rejects this graph, but a corrupt bootstrap must not
			leave the room in a stage that can never finish. */
			FinishPattern(boss, serverTick);
			return true;
		}
		EnterPatternStage(boss, *next, nextStageIndex, serverTick);
		return true;
	}

	bool ApplyPublishedOutcomeBranch(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		const BOSS_PATTERN_STAGE_DEFINITION& stage,
		const std::uint32_t serverTick)
	{
		/* A landed result wins over a deadline on the same fixed tick. TIMEOUT is
		evaluated only after the current stage has advanced its clock and hits. */
		for (const BOSS_PATTERN_STAGE_BRANCH& branch : stage.Branches)
		{
			if (BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT == branch.eOutcome ||
				!CBossCombatRuntime::Consume_PatternOutcome(
					boss, stage.strActionId, branch.eOutcome))
			{
				continue;
			}
			if (BOSS_PATTERN_STAGE_OUTCOME::PART_DESTROYED ==
				branch.eOutcome)
			{
				/* PlayerSkillSystem keeps this legacy edge for patterns that do not
				   yet publish a PART_DESTROYED branch. A typed branch consumed the
				   same outcome, so it also consumes the fallback latch. */
				boss.bPendingArmorBreakReaction = false;
			}
			return ApplyStageBranch(boss, pattern, branch, serverTick);
		}
		return false;
	}

	bool ApplyTimeoutBranch(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		const BOSS_PATTERN_STAGE_DEFINITION& stage,
		const std::uint32_t serverTick)
	{
		const auto branch = std::find_if(
			stage.Branches.begin(), stage.Branches.end(),
			[](const BOSS_PATTERN_STAGE_BRANCH& candidate)
			{
				return BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT ==
					candidate.eOutcome;
			});
		return stage.Branches.end() != branch &&
			ApplyStageBranch(boss, pattern, *branch, serverTick);
	}

	float MaximumPatternRange(
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns)
	{
		float maximum = 0.f;
		for (const BOSS_PATTERN_DEFINITION& pattern : patterns)
			maximum = (std::max)(maximum, pattern.fMaximumRange);
		return maximum;
	}

	bool MoveAlongPath(
		SERVER_WORLD_ENTITY& boss,
		const float fixedDeltaSeconds)
	{
		while (boss.iMovePathIndex < boss.MovePath.size())
		{
			const SERVER_NAV_POINT& point = boss.MovePath[boss.iMovePathIndex];
			const float deltaX = point.x - boss.fPositionX;
			const float deltaZ = point.z - boss.fPositionZ;
			const float distance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
			if (distance <= PATH_POINT_STOP_DISTANCE)
			{
				boss.fPositionX = point.x;
				boss.fPositionY = point.y;
				boss.fPositionZ = point.z;
				++boss.iMovePathIndex;
				continue;
			}
			boss.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
			const float step =
				(std::min)(boss.fMoveSpeed * fixedDeltaSeconds, distance);
			boss.fPositionX += deltaX / distance * step;
			boss.fPositionZ += deltaZ / distance * step;
			return true;
		}
		boss.MovePath.clear();
		boss.iMovePathIndex = 0;
		return false;
	}

	/* The stele answers the blow instead of the player. The test is the straight
	   line from the boss to the player against the standing prop's circle, so a
	   player beside the stele is still exposed and one directly behind it is not.
	   A stage that is authored to pierce cover never reaches here. */
	bool IsShieldedByCover(
		const SERVER_WORLD_ENTITY& boss,
		const SERVER_PLAYER& player,
		const std::vector<LostArk::Shared::CombatCollision::CIRCLE_XZ>&
			coverCircles)
	{
		return std::any_of(
			coverCircles.begin(), coverCircles.end(),
			[&boss, &player](
				const LostArk::Shared::CombatCollision::CIRCLE_XZ& circle)
			{
				return LostArk::Shared::CombatCollision::Segment_IntersectsCircle(
					boss.fPositionX, boss.fPositionZ,
					player.fPositionX, player.fPositionZ, circle);
			});
	}

	bool ContainsPatternHit(
		const SERVER_WORLD_ENTITY& boss,
		const SERVER_PLAYER& player)
	{
		const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ playerBody{
			player.fPositionX,
			player.fPositionZ,
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X
		};
		const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
		const float forwardX = std::sin(yawRadians);
		const float forwardZ = std::cos(yawRadians);
		switch (boss.ePatternHitShape)
		{
		case BOSS_PATTERN_HIT_SHAPE::CIRCLE:
			return LostArk::Shared::CombatCollision::Circles_Overlap(
				LostArk::Shared::CombatCollision::CIRCLE_XZ{
					boss.fPositionX,
					boss.fPositionZ,
					boss.fPatternHitOuterRadius
				},
				playerBody);
		case BOSS_PATTERN_HIT_SHAPE::RING:
			return LostArk::Shared::CombatCollision::Circle_IntersectsRing(
				playerBody,
				boss.fPositionX,
				boss.fPositionZ,
				boss.fPatternHitInnerRadius,
				boss.fPatternHitOuterRadius);
		case BOSS_PATTERN_HIT_SHAPE::CONE:
			return LostArk::Shared::CombatCollision::Circle_IntersectsCone(
				playerBody,
				boss.fPositionX,
				boss.fPositionZ,
				forwardX,
				forwardZ,
				boss.fPatternHitLength,
				boss.fPatternHitAngleDegrees);
		case BOSS_PATTERN_HIT_SHAPE::BOX:
			return LostArk::Shared::CombatCollision::Circle_IntersectsForwardBox(
				playerBody,
				boss.fPositionX,
				boss.fPositionZ,
				forwardX,
				forwardZ,
				boss.fPatternHitLength,
				boss.fPatternHitHalfWidth);
		case BOSS_PATTERN_HIT_SHAPE::CROSS:
			return LostArk::Shared::CombatCollision::Circle_IntersectsCross(
				playerBody,
				boss.fPositionX,
				boss.fPositionZ,
				forwardX,
				forwardZ,
				boss.fPatternHitLength,
				boss.fPatternHitHalfWidth);
		case BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS:
			return LostArk::Shared::CombatCollision::Circle_IntersectsSixDirections(
				playerBody,
				boss.fPositionX,
				boss.fPositionZ,
				forwardX,
				forwardZ,
				boss.fPatternHitLength,
				boss.fPatternHitHalfWidth);
		default:
			return false;
		}
	}

	void ApplyPatternHit(
		SERVER_WORLD_ENTITY& boss,
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		const CGameplayCatalog& catalog,
		const std::uint32_t serverTick,
		const std::vector<LostArk::Shared::CombatCollision::CIRCLE_XZ>&
			coverCircles,
		std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents)
	{
		if (BOSS_PATTERN_HIT_SHAPE::NONE == boss.ePatternHitShape ||
			boss.strDamageProfileId.empty())
		{
			return;
		}
		const BOSS_RUNTIME_PROFILE* bossProfile =
			catalog.Find_Boss(boss.strArchetypeId);
		const std::uint32_t rawDamage = CGameplayCatalog::Resolve_Damage(
			nullptr == bossProfile ? 0u : bossProfile->iAttackPower,
			catalog.Find_DamageRatePercent(boss.strDamageProfileId));
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			/* A successful player counter answers the hit instead of taking it,
			so it is consulted before any damage is resolved. */
			if (0u == player.iCurrentHp || !player.isCombatReady ||
				!ContainsPatternHit(boss, player) ||
				(!boss.bPatternPiercesCover &&
					IsShieldedByCover(boss, player, coverCircles)) ||
				CPlayerSkillSystem::Try_Counter(player, catalog, serverTick))
			{
				continue;
			}
			SERVER_WORLD_TO_PLAYER_HIT incoming{};
			incoming.iRawDamage = rawDamage;
			incoming.fSourceX = boss.fPositionX;
			incoming.fSourceZ = boss.fPositionZ;
			incoming.fPushRangeM = boss.fPatternPushRangeM;
			incoming.iPushMs = boss.iPatternPushMs;
			incoming.bKnockdown = boss.bPatternKnockdown;
			incoming.iDownMs = boss.iPatternDownMs;
			incoming.iServerTick = serverTick;
			(void)CServerCombatHitRuntime::Apply_WorldToPlayer(
				player, incoming, catalog, outDamageEvents);
		}
	}
}

std::uint32_t LostArk::Server::CValtanBrain::Calculate_HealthBar(
	const SERVER_WORLD_ENTITY& boss)
{
	if (0u == boss.iCurrentHp || 0u == boss.iMaximumHp ||
		0u == boss.iMaximumHealthBars)
	{
		return 0u;
	}
	const std::uint64_t scaled =
		static_cast<std::uint64_t>(boss.iCurrentHp) * boss.iMaximumHealthBars;
	return static_cast<std::uint32_t>((scaled + boss.iMaximumHp - 1u) /
		boss.iMaximumHp);
}

std::uint32_t LostArk::Server::CValtanBrain::Resolve_HealthBarHp(
	const SERVER_WORLD_ENTITY& boss,
	const std::uint32_t healthBar)
{
	if (0u == healthBar || 0u == boss.iMaximumHp ||
		0u == boss.iMaximumHealthBars ||
		healthBar > boss.iMaximumHealthBars)
	{
		return 0u;
	}
	/* Floor of the exact boundary. Calculate_HealthBar rounds up, and the
	discarded remainder is always smaller than one bar, so this HP reads back as
	healthBar rather than the bar below it. */
	const std::uint64_t scaled =
		static_cast<std::uint64_t>(boss.iMaximumHp) * healthBar;
	const std::uint32_t resolved =
		static_cast<std::uint32_t>(scaled / boss.iMaximumHealthBars);
	return 0u == resolved ? 1u : resolved;
}

void LostArk::Server::CValtanBrain::Update(
	SERVER_WORLD_ENTITY& boss,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CGameplayCatalog& catalog,
	const CServerNavigation& navigation,
	const float fixedDeltaSeconds,
	const std::uint32_t serverTick,
	const std::vector<LostArk::Shared::CombatCollision::CIRCLE_XZ>&
		coverCircles,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const
{
	if (WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind)
		return;
	if (0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction)
	{
		boss.iCurrentHp = 0u;
		boss.strPatternId.clear();
		boss.strPatternStageId.clear();
		boss.strActionId.clear();
		Transition(boss, SERVER_ENTITY_ACTION::DEAD, serverTick);
		boss.MovePath.clear();
		return;
	}
	const BOSS_RUNTIME_PROFILE* bossProfile =
		catalog.Find_Boss(boss.strArchetypeId);
	const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
	if (nullptr == bossProfile || nullptr == patterns || patterns->empty())
		return;
	if (boss.iMaximumHp > 0u && boss.iPhaseTwoHpPercent > 0u &&
		static_cast<std::uint64_t>(boss.iCurrentHp) * 100u <=
		static_cast<std::uint64_t>(boss.iMaximumHp) * boss.iPhaseTwoHpPercent)
	{
		/* The phase ships inside the boss combat snapshot, so it moves through
		the runtime that owns that snapshot's revision rather than being written
		here. */
		(void)CBossCombatRuntime::Set_GameplayPhase(boss, 2u);
	}
	const std::uint32_t currentHealthBar = Calculate_HealthBar(boss);
	QueueCrossedHealthBarPatterns(boss, *patterns, currentHealthBar);

	SERVER_PLAYER* target = nullptr;
	float targetDistanceSquared = (std::numeric_limits<float>::max)();
	for (auto& [playerId, player] : players)
	{
		(void)playerId;
		if (0u == player.iCurrentHp || !player.isCombatReady ||
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction ||
			LostArk::Shared::PLAYER_ACTION_STATE::FALLING == player.eAction)
		{
			continue;
		}
		const float deltaX = player.fPositionX - boss.fPositionX;
		const float deltaZ = player.fPositionZ - boss.fPositionZ;
		const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
		if (distanceSquared < targetDistanceSquared)
		{
			target = &player;
			targetDistanceSquared = distanceSquared;
		}
	}
	const float engageDistance = (std::max)(
		boss.fEngageDistance, MaximumPatternRange(*patterns));
	if (nullptr == target || targetDistanceSquared > engageDistance * engageDistance)
	{
		FinishPattern(boss, serverTick);
		boss.iTargetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		boss.MovePath.clear();
		return;
	}
	boss.iTargetEntityId = target->iNetEntityId;
	const float distance = std::sqrt(targetDistanceSquared);

	if (SERVER_ENTITY_ACTION::IDLE == boss.eAction ||
		SERVER_ENTITY_ACTION::CHASE == boss.eAction)
	{
		const BOSS_PATTERN_DEFINITION* selected = SelectPattern(
			boss, *patterns, catalog.Find_IntroPatternId(boss.strEncounterId),
			catalog.Find_BossPatternRotation(
				boss.strEncounterId, currentHealthBar),
			currentHealthBar, distance, serverTick);
		if (nullptr == selected)
		{
			if (!boss.bIntroPatternConsumed)
			{
				/* The entrance is authored around the spawn point, so the boss holds
				there until a player reaches its range instead of walking the arena
				approach to meet them. */
				Transition(boss, SERVER_ENTITY_ACTION::IDLE, serverTick);
				boss.MovePath.clear();
				return;
			}
			Transition(boss, SERVER_ENTITY_ACTION::CHASE, serverTick);
			const float pathDeltaX = target->fPositionX - boss.fLastPathGoalX;
			const float pathDeltaZ = target->fPositionZ - boss.fLastPathGoalZ;
			if (boss.MovePath.empty() ||
				pathDeltaX * pathDeltaX + pathDeltaZ * pathDeltaZ > 1.f)
			{
				std::vector<SERVER_NAV_POINT> path;
				if (navigation.Find_Path(
					boss.fPositionX, boss.fPositionZ,
					target->fPositionX, target->fPositionZ, path))
				{
					boss.MovePath = std::move(path);
					boss.iMovePathIndex = 0;
					boss.fLastPathGoalX = target->fPositionX;
					boss.fLastPathGoalZ = target->fPositionZ;
				}
			}
			MoveAlongPath(boss, fixedDeltaSeconds);
			return;
		}
		boss.MovePath.clear();
		const float deltaX = target->fPositionX - boss.fPositionX;
		const float deltaZ = target->fPositionZ - boss.fPositionZ;
		boss.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
		BeginPattern(boss, *selected, players, target, serverTick);
		if (boss.bPatternChargeImpact && 0u != boss.iPatternStageDurationMs)
		{
			const float durationSeconds =
				static_cast<float>(boss.iPatternStageDurationMs) *
				MILLISECONDS_TO_SECONDS;
			/* The target locks the charge heading, but Valtan must continue past
			   that player until it reaches a receiver. The encounter's authored
			   maximumRange is therefore the travel distance; using target distance
			   would stop at the bait player before the wall. */
			boss.fPatternForcedMotionSpeed = durationSeconds > 0.f ?
				boss.fPatternMaximumRange / durationSeconds : 0.f;
		}
	}

	const BOSS_PATTERN_DEFINITION* currentPattern =
		FindPattern(*patterns, boss.strPatternId);
	if (nullptr == currentPattern ||
		boss.iPatternStageIndex >= currentPattern->Stages.size())
	{
		FinishPattern(boss, serverTick);
		return;
	}
	UpdatePatternTargetAndAim(boss, *currentPattern, players, target);
	const BOSS_PATTERN_STAGE_DEFINITION& currentStage =
		currentPattern->Stages[boss.iPatternStageIndex];
	if (ApplyPublishedOutcomeBranch(
		boss, *currentPattern, currentStage, serverTick))
	{
		return;
	}

	/* A plate coming off replaces the pattern's ordinary recovery, so the
	reaction is the pattern's own PART_BREAK stage rather than whatever
	happens to sit behind the stun. */
	if (boss.bPendingArmorBreakReaction)
	{
		boss.bPendingArmorBreakReaction = false;
		for (std::uint32_t reactionIndex = 0u;
			reactionIndex < currentPattern->Stages.size(); ++reactionIndex)
		{
			if (BOSS_PATTERN_STAGE_KIND::PART_BREAK !=
				currentPattern->Stages[reactionIndex].eStageKind)
			{
				continue;
			}
			EnterPatternStage(
				boss, currentPattern->Stages[reactionIndex],
				reactionIndex, serverTick);
			return;
		}
	}
	const std::uint64_t elapsedStageTicks =
		StageElapsedTicks(boss, serverTick);
	/* Keep the presentation/motion clock derived from the same integer stage
	clock. It may still be represented as float, but no gameplay boundary ever
	depends on accumulated floating-point deltas. */
	boss.fActionElapsedSeconds =
		static_cast<float>(elapsedStageTicks) /
		static_cast<float>(SERVER_TICK_HZ);
	Advance_ArenaBreakLeap(boss);
	while (boss.iAppliedPatternHitCount < boss.iPatternHitCount)
	{
		const std::uint32_t hitOffsetMs =
			currentStage.HitOffsetsMs.empty() ?
				boss.iPatternHitDelayMs + boss.iAppliedPatternHitCount *
					boss.iPatternHitIntervalMs :
				currentStage.HitOffsetsMs[boss.iAppliedPatternHitCount];
		if (!HasElapsedMilliseconds(elapsedStageTicks, hitOffsetMs))
		{
			break;
		}
		ApplyPatternHit(
			boss, players, catalog, serverTick, coverCircles,
			outDamageEvents);
		++boss.iAppliedPatternHitCount;
	}
	if (!HasElapsedMilliseconds(
		elapsedStageTicks, boss.iPatternStageDurationMs))
	{
		return;
	}
	if (ApplyTimeoutBranch(
		boss, *currentPattern, currentStage, serverTick))
	{
		return;
	}

	const std::uint32_t nextStageIndex =
		NextClockStageIndex(*currentPattern, boss.iPatternStageIndex);
	if (nextStageIndex >= currentPattern->Stages.size())
	{
		FinishPattern(boss, serverTick);
		return;
	}
	EnterPatternStage(
		boss, currentPattern->Stages[nextStageIndex], nextStageIndex, serverTick);
}

bool LostArk::Server::CValtanBrain::Is_ArmorRequirementMet(
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_ARMOR_REQUIREMENT requirement)
{
	if (BOSS_PATTERN_ARMOR_REQUIREMENT::ANY == requirement)
		return true;
	bool wearsAnyPlate = false;
	for (const SERVER_BOSS_ARMOR_PLATE_STATE& plate : boss.ArmorPlates)
	{
		if (0u != plate.iRemainingDurability)
		{
			wearsAnyPlate = true;
			break;
		}
	}
	return BOSS_PATTERN_ARMOR_REQUIREMENT::ARMORED == requirement ?
		wearsAnyPlate : !wearsAnyPlate;
}

bool LostArk::Server::CValtanBrain::Is_PhaseRequirementMet(
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_PHASE_REQUIREMENT requirement)
{
	switch (requirement)
	{
	case BOSS_PATTERN_PHASE_REQUIREMENT::ANY:
		return true;
	case BOSS_PATTERN_PHASE_REQUIREMENT::PHASE_ONE:
		return 1u == boss.iPhase;
	case BOSS_PATTERN_PHASE_REQUIREMENT::PHASE_TWO:
		return boss.iPhase >= 2u;
	}
	return false;
}

bool LostArk::Server::CValtanBrain::Try_BuildImpactMotion(
	const SERVER_WORLD_ENTITY& boss,
	const float fixedDeltaSeconds,
	float& outProposedX,
	float& outProposedZ) const
{
	if (!boss.bPatternChargeImpact ||
		!std::isfinite(boss.fPatternForcedMotionSpeed) ||
		boss.fPatternForcedMotionSpeed <= 0.f ||
		!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
	{
		return false;
	}
	const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
	const float distance = boss.fPatternForcedMotionSpeed * fixedDeltaSeconds;
	outProposedX = boss.fPositionX + std::sin(yawRadians) * distance;
	outProposedZ = boss.fPositionZ + std::cos(yawRadians) * distance;
	return std::isfinite(outProposedX) && std::isfinite(outProposedZ);
}

bool LostArk::Server::CValtanBrain::Try_BuildStageMotion(
	const SERVER_WORLD_ENTITY& boss,
	const float fixedDeltaSeconds,
	float& outProposedX,
	float& outProposedZ) const
{
	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
		return false;

	/* A stage whose clip already carries the travel steps along that curve,
	the same contract a player skill uses: the difference between the curve at
	the previous tick and at this one is the step, so the body and the mesh
	arrive together instead of the transform sliding out from under the pose.
	A stage that authored its own distance keeps the constant slide, because
	those two carry Valtan far past anything the bound clip animates. */
	float distance = 0.f;
	if (!boss.PatternStageRootMotion.empty())
	{
		const float previousSeconds = (std::max)(
			0.f, boss.fActionElapsedSeconds - fixedDeltaSeconds);
		float previousForward = 0.f;
		float currentForward = 0.f;
		Sample_StageRootMotionForward(
			boss.PatternStageRootMotion, previousSeconds, previousForward);
		Sample_StageRootMotionForward(
			boss.PatternStageRootMotion, boss.fActionElapsedSeconds,
			currentForward);
		distance = currentForward - previousForward;
		if (0.f == distance)
			return false;
	}
	else
	{
		if (BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD !=
			boss.ePatternStageMotionKind ||
			!std::isfinite(boss.fPatternForcedMotionSpeed) ||
			boss.fPatternForcedMotionSpeed <= 0.f)
		{
			return false;
		}
		distance = boss.fPatternForcedMotionSpeed * fixedDeltaSeconds;
	}
	if (!std::isfinite(distance))
		return false;
	const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
	outProposedX = boss.fPositionX + std::sin(yawRadians) * distance;
	outProposedZ = boss.fPositionZ + std::cos(yawRadians) * distance;
	return std::isfinite(outProposedX) && std::isfinite(outProposedZ);
}

bool LostArk::Server::CValtanBrain::Complete_ImpactStage(
	SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const std::uint32_t serverTick) const
{
	if (!boss.bPatternChargeImpact)
		return false;
	const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
	const BOSS_PATTERN_DEFINITION* pattern = nullptr;
	if (nullptr != patterns)
		pattern = FindPattern(*patterns, boss.strPatternId);
	if (nullptr == pattern ||
		boss.iPatternStageIndex >= pattern->Stages.size())
		return false;
	const BOSS_PATTERN_STAGE_DEFINITION& stage =
		pattern->Stages[boss.iPatternStageIndex];
	const auto branch = std::find_if(
		stage.Branches.begin(), stage.Branches.end(),
		[](const BOSS_PATTERN_STAGE_BRANCH& candidate)
		{
			return BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT ==
				candidate.eOutcome;
		});
	if (stage.Branches.end() == branch ||
		!ApplyStageBranch(boss, *pattern, *branch, serverTick))
	{
		return false;
	}
	boss.fPatternForcedMotionSpeed = 0.f;
	return true;
}
