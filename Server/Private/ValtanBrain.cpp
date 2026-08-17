#include "ValtanBrain.h"

#include "Gameplay/CombatCollisionContract.h"
#include "Gameplay/WorldCollisionContract.h"
#include "PlayerSkillSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
	using namespace LostArk::Server;

	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
	constexpr float DEGREES_TO_RADIANS = 0.0174532925f;
	constexpr float MILLISECONDS_TO_SECONDS = 0.001f;
	constexpr float PATH_POINT_STOP_DISTANCE = 0.1f;
	constexpr const char* ARMOR_BREAK_PATTERN_ID =
		"VALTAN_ARMOR_BREAK_OPENING";
	constexpr const char* ARMOR_BREAK_STAGE_ID = "WALL_CHARGE";
	constexpr const char* ARMOR_BREAK_ACTION_ID =
		"valtan.mechanic.armor-break-opening.charge";
	constexpr const char* ARENA_BREAK_PATTERN_ID = "VALTAN_ARENA_BREAK_109";
	constexpr const char* ARENA_BREAK_TAKEOFF_STAGE_ID = "TAKEOFF";
	constexpr const char* ARENA_BREAK_DROP_STAGE_ID = "DROP";
	/* The converted Valtan model carries no jump clip, so the leap is a Server
	transform arc instead of root motion. Both the apex and the landing point
	come from the pattern's compiled serverMotion anchor, never from a constant
	here and never from the boss placement. */

	bool Is_ArenaBreakLeapStage(const SERVER_WORLD_ENTITY& boss)
	{
		return boss.strPatternId == ARENA_BREAK_PATTERN_ID &&
			(boss.strPatternStageId == ARENA_BREAK_TAKEOFF_STAGE_ID ||
				boss.strPatternStageId == ARENA_BREAK_DROP_STAGE_ID);
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
		if (!Is_ArenaBreakLeapStage(boss))
			return;
		const float ratio = StageRatio(boss);
		const float apex = boss.fLeapApexHeight;
		if (boss.strPatternStageId == ARENA_BREAK_TAKEOFF_STAGE_ID)
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

	bool IsPatternCooldownReady(
		const SERVER_WORLD_ENTITY& boss,
		const std::string& patternId,
		const std::uint32_t serverTick)
	{
		const auto found = std::find_if(
			boss.PatternCooldowns.begin(), boss.PatternCooldowns.end(),
			[&patternId](const SERVER_BOSS_PATTERN_COOLDOWN& cooldown)
			{ return cooldown.strPatternId == patternId; });
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
			{ return cooldown.strPatternId == pattern.strPatternId; });
		if (boss.PatternCooldowns.end() == found)
		{
			boss.PatternCooldowns.push_back(
				SERVER_BOSS_PATTERN_COOLDOWN{ pattern.strPatternId, readyTick });
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

	void QueueCrossedHealthBarPatterns(
		SERVER_WORLD_ENTITY& boss,
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns,
		const std::uint32_t currentHealthBar)
	{
		std::vector<const BOSS_PATTERN_DEFINITION*> crossed;
		for (const BOSS_PATTERN_DEFINITION& pattern : patterns)
		{
			if (BOSS_PATTERN_SELECTION::HEALTH_BAR != pattern.eSelection ||
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
		const std::uint32_t serverTick)
	{
		std::vector<const BOSS_PATTERN_DEFINITION*> eligible;
		std::vector<const BOSS_PATTERN_DEFINITION*> repeatAllowed;
		for (const BOSS_PATTERN_DEFINITION& pattern : patterns)
		{
			if (BOSS_PATTERN_SELECTION::NORMAL != pattern.eSelection ||
				pattern.strPatternId == introPatternId ||
				currentHealthBar < pattern.iMinimumHealthBar ||
				currentHealthBar > pattern.iMaximumHealthBar ||
				targetDistance < pattern.fMinimumRange ||
				targetDistance > pattern.fMaximumRange ||
				!IsPatternCooldownReady(
					boss, pattern.strPatternId, serverTick))
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
		const std::uint32_t currentHealthBar,
		const float targetDistance,
		const std::uint32_t serverTick)
	{
		/* The first appearance runs before the health-bar queue and before any
		weighted roll, so the entrance sweep can never come up again later. It is
		consumed even if the pattern is missing, so a broken catalog cannot stall
		the boss on every tick. */
		if (!boss.bIntroPatternConsumed)
		{
			boss.bIntroPatternConsumed = true;
			if (!introPatternId.empty())
			{
				if (const BOSS_PATTERN_DEFINITION* intro =
					FindPattern(patterns, introPatternId))
				{
					return intro;
				}
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
		return SelectNormalPattern(
			boss, patterns, introPatternId,
			currentHealthBar, targetDistance, serverTick);
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
		default:
			return SERVER_ENTITY_ACTION::PATTERN_WINDUP;
		}
	}

	void EnterPatternStage(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_STAGE_DEFINITION& stage,
		const std::uint32_t stageIndex,
		const std::uint32_t serverTick)
	{
		boss.iPatternStageIndex = stageIndex;
		boss.strPatternStageId = stage.strStageId;
		boss.iPatternStageDurationMs = stage.iDurationMs;
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
		boss.iAppliedPatternHitCount = 0u;
		boss.bPatternWallContact = stage.bWallContact;
		boss.eAction = ToServerAction(stage.eStageKind);
		boss.fActionElapsedSeconds = 0.f;
		boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
		if (boss.strPatternId == ARENA_BREAK_PATTERN_ID &&
			stage.strStageId == ARENA_BREAK_TAKEOFF_STAGE_ID)
		{
			boss.fLeapOriginX = boss.fPositionX;
			boss.fLeapOriginY = boss.fPositionY;
			boss.fLeapOriginZ = boss.fPositionZ;
			boss.fLeapApexHeight = boss.fPatternLeapApexHeight;
			boss.MovePath.clear();
		}
		else if (boss.strPatternId == ARENA_BREAK_PATTERN_ID &&
			stage.strStageId != ARENA_BREAK_DROP_STAGE_ID)
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
		const std::uint32_t serverTick)
	{
		boss.strPatternId = pattern.strPatternId;
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
		/* A pattern that owns a compiled landing anchor lands on it. Everything
		else keeps standing on its authored placement. */
		if (BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR == pattern.Motion.eKind)
		{
			boss.fLeapLandingX = pattern.Motion.fLandingX;
			boss.fLeapLandingY = pattern.Motion.fLandingY;
			boss.fLeapLandingZ = pattern.Motion.fLandingZ;
			boss.fPatternLeapApexHeight = pattern.Motion.fApexHeight;
		}
		else
		{
			boss.fLeapLandingX = boss.fSpawnPositionX;
			boss.fLeapLandingY = boss.fSpawnPositionY;
			boss.fLeapLandingZ = boss.fSpawnPositionZ;
			boss.fPatternLeapApexHeight = 0.f;
		}
		EnterPatternStage(boss, pattern.Stages.front(), 0u, serverTick);
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
		boss.fPatternForcedMotionSpeed = 0.f;
		boss.ePatternHitShape = BOSS_PATTERN_HIT_SHAPE::NONE;
		boss.iPatternHitCount = 0u;
		boss.iAppliedPatternHitCount = 0u;
		boss.bPatternWallContact = false;
		Transition(boss, SERVER_ENTITY_ACTION::IDLE, serverTick);
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
			if (0u == player.iCurrentHp || !player.isCombatReady ||
				!ContainsPatternHit(boss, player) ||
				CPlayerSkillSystem::Try_Counter(player, catalog, serverTick))
			{
				continue;
			}
			const PLAYER_RUNTIME_PROFILE* playerProfile =
				catalog.Find_Player(player.eCharacterClass);
			const std::uint32_t damage = CGameplayCatalog::Apply_Defense(
				rawDamage, nullptr == playerProfile ? 0u : playerProfile->iDefense);
			player.iCurrentHp = damage >= player.iCurrentHp ?
				0u : player.iCurrentHp - damage;
			if (0u != damage &&
				outDamageEvents.size() < LostArk::Shared::MAX_DAMAGE_EVENTS)
			{
				LostArk::Shared::DAMAGE_EVENT event{};
				event.iTargetNetEntityId = player.iNetEntityId;
				event.iAmount = damage;
				event.fPositionX = player.fPositionX;
				event.fPositionY = player.fPositionY;
				event.fPositionZ = player.fPositionZ;
				event.isOutgoing = false;
				outDamageEvents.push_back(event);
			}
			if (0u == player.iCurrentHp)
			{
				player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::DEAD;
				player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
				player.iActionStartTick = 0u == serverTick ? 1u : serverTick;
				player.hasMoveGoal = false;
				player.MovePath.clear();
			}
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
		boss.iPhase = 2;
	}
	const std::uint32_t currentHealthBar = Calculate_HealthBar(boss);
	QueueCrossedHealthBarPatterns(boss, *patterns, currentHealthBar);

	SERVER_PLAYER* target = nullptr;
	float targetDistanceSquared = (std::numeric_limits<float>::max)();
	for (auto& [playerId, player] : players)
	{
		(void)playerId;
		if (0u == player.iCurrentHp || !player.isCombatReady ||
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction)
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
			currentHealthBar, distance, serverTick);
		if (nullptr == selected)
		{
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
		BeginPattern(boss, *selected, serverTick);
		if (boss.strPatternId == ARMOR_BREAK_PATTERN_ID &&
			boss.strPatternStageId == ARMOR_BREAK_STAGE_ID &&
			boss.strActionId == ARMOR_BREAK_ACTION_ID &&
			0u != boss.iPatternStageDurationMs)
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

	boss.fActionElapsedSeconds += fixedDeltaSeconds;
	Advance_ArenaBreakLeap(boss);
	while (boss.iAppliedPatternHitCount < boss.iPatternHitCount &&
		boss.fActionElapsedSeconds * 1000.f >=
			static_cast<float>(boss.iAppliedPatternHitCount *
				boss.iPatternHitIntervalMs))
	{
		ApplyPatternHit(boss, players, catalog, serverTick, outDamageEvents);
		++boss.iAppliedPatternHitCount;
	}
	if (boss.fActionElapsedSeconds <
		static_cast<float>(boss.iPatternStageDurationMs) *
			MILLISECONDS_TO_SECONDS)
	{
		return;
	}

	const std::uint32_t nextStageIndex = boss.iPatternStageIndex + 1u;
	if (nextStageIndex >= currentPattern->Stages.size())
	{
		FinishPattern(boss, serverTick);
		return;
	}
	EnterPatternStage(
		boss, currentPattern->Stages[nextStageIndex], nextStageIndex, serverTick);
}

bool LostArk::Server::CValtanBrain::Try_BuildImpactMotion(
	const SERVER_WORLD_ENTITY& boss,
	const float fixedDeltaSeconds,
	float& outProposedX,
	float& outProposedZ) const
{
	if (boss.strPatternId != ARMOR_BREAK_PATTERN_ID ||
		boss.strPatternStageId != ARMOR_BREAK_STAGE_ID ||
		boss.strActionId != ARMOR_BREAK_ACTION_ID ||
		SERVER_ENTITY_ACTION::PATTERN_WINDUP != boss.eAction ||
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

bool LostArk::Server::CValtanBrain::Complete_ImpactStage(
	SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const std::uint32_t serverTick) const
{
	if (boss.strPatternId != ARMOR_BREAK_PATTERN_ID ||
		boss.strPatternStageId != ARMOR_BREAK_STAGE_ID ||
		boss.strActionId != ARMOR_BREAK_ACTION_ID)
	{
		return false;
	}
	const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
	const BOSS_PATTERN_DEFINITION* pattern = nullptr;
	if (nullptr != patterns)
		pattern = FindPattern(*patterns, boss.strPatternId);
	const std::uint32_t nextStageIndex = boss.iPatternStageIndex + 1u;
	if (nullptr == pattern || nextStageIndex >= pattern->Stages.size())
		return false;
	EnterPatternStage(
		boss, pattern->Stages[nextStageIndex], nextStageIndex, serverTick);
	boss.fPatternForcedMotionSpeed = 0.f;
	return true;
}
