#include "ValtanBrain.h"

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

	std::uint32_t CalculateHealthBar(const SERVER_WORLD_ENTITY& boss)
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

	bool ContainsPatternId(
		const std::vector<std::string>& ids,
		const std::string& patternId)
	{
		return ids.end() != std::find(ids.begin(), ids.end(), patternId);
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
		const std::uint32_t currentHealthBar,
		const float targetDistance,
		const std::uint32_t serverTick)
	{
		std::vector<const BOSS_PATTERN_DEFINITION*> eligible;
		std::vector<const BOSS_PATTERN_DEFINITION*> repeatAllowed;
		for (const BOSS_PATTERN_DEFINITION& pattern : patterns)
		{
			if (BOSS_PATTERN_SELECTION::NORMAL != pattern.eSelection ||
				currentHealthBar < pattern.iMinimumHealthBar ||
				currentHealthBar > pattern.iMaximumHealthBar ||
				targetDistance < pattern.fMinimumRange ||
				targetDistance > pattern.fMaximumRange)
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
		const std::uint32_t currentHealthBar,
		const float targetDistance,
		const std::uint32_t serverTick)
	{
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
			boss, patterns, currentHealthBar, targetDistance, serverTick);
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
		boss.eAction = ToServerAction(stage.eStageKind);
		boss.fActionElapsedSeconds = 0.f;
		boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
	}

	void BeginPattern(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		const std::uint32_t serverTick)
	{
		boss.strPatternId = pattern.strPatternId;
		boss.fPatternMinimumRange = pattern.fMinimumRange;
		boss.fPatternMaximumRange = pattern.fMaximumRange;
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
		EnterPatternStage(boss, pattern.Stages.front(), 0u, serverTick);
	}

	void FinishPattern(
		SERVER_WORLD_ENTITY& boss,
		const std::uint32_t serverTick)
	{
		boss.strPatternId.clear();
		boss.strActionId.clear();
		boss.strDamageProfileId.clear();
		boss.iPatternStageIndex = 0u;
		boss.iPatternStageDurationMs = 0u;
		boss.ePatternHitShape = BOSS_PATTERN_HIT_SHAPE::NONE;
		boss.iPatternHitCount = 0u;
		boss.iAppliedPatternHitCount = 0u;
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
		const float deltaX = player.fPositionX - boss.fPositionX;
		const float deltaZ = player.fPositionZ - boss.fPositionZ;
		const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
		switch (boss.ePatternHitShape)
		{
		case BOSS_PATTERN_HIT_SHAPE::CIRCLE:
			return distanceSquared <=
				boss.fPatternHitOuterRadius * boss.fPatternHitOuterRadius;
		case BOSS_PATTERN_HIT_SHAPE::RING:
			return distanceSquared >=
				boss.fPatternHitInnerRadius * boss.fPatternHitInnerRadius &&
				distanceSquared <=
				boss.fPatternHitOuterRadius * boss.fPatternHitOuterRadius;
		default:
			break;
		}

		const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
		const float forwardX = std::sin(yawRadians);
		const float forwardZ = std::cos(yawRadians);
		const float rightX = std::cos(yawRadians);
		const float rightZ = -std::sin(yawRadians);
		const float localForward = deltaX * forwardX + deltaZ * forwardZ;
		const float localRight = deltaX * rightX + deltaZ * rightZ;
		switch (boss.ePatternHitShape)
		{
		case BOSS_PATTERN_HIT_SHAPE::CONE:
		{
			if (localForward < 0.f ||
				distanceSquared > boss.fPatternHitLength * boss.fPatternHitLength)
			{
				return false;
			}
			if (distanceSquared <= 0.0001f)
				return true;
			const float cosine = localForward / std::sqrt(distanceSquared);
			return cosine >= std::cos(
				boss.fPatternHitAngleDegrees * 0.5f * DEGREES_TO_RADIANS);
		}
		case BOSS_PATTERN_HIT_SHAPE::BOX:
			return localForward >= 0.f &&
				localForward <= boss.fPatternHitLength &&
				std::abs(localRight) <= boss.fPatternHitHalfWidth;
		case BOSS_PATTERN_HIT_SHAPE::CROSS:
			return (std::abs(localForward) <= boss.fPatternHitLength &&
					std::abs(localRight) <= boss.fPatternHitHalfWidth) ||
				(std::abs(localRight) <= boss.fPatternHitLength &&
					std::abs(localForward) <= boss.fPatternHitHalfWidth);
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
	const std::uint32_t currentHealthBar = CalculateHealthBar(boss);
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
			boss, *patterns, currentHealthBar, distance, serverTick);
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
