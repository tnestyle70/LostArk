#include "ValtanBrain.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
	constexpr float MILLISECONDS_TO_SECONDS = 0.001f;
	constexpr float PATH_POINT_STOP_DISTANCE = 0.1f;

	void Transition(
		LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const LostArk::Server::SERVER_ENTITY_ACTION action,
		const std::uint32_t serverTick)
	{
		if (boss.eAction == action)
			return;
		boss.eAction = action;
		boss.fActionElapsedSeconds = 0.f;
		boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
		boss.hasAppliedPatternDamage = false;
	}

	bool MoveAlongPath(
		LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const float fixedDeltaSeconds)
	{
		while (boss.iMovePathIndex < boss.MovePath.size())
		{
			const LostArk::Server::SERVER_NAV_POINT& point =
				boss.MovePath[boss.iMovePathIndex];
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
			const float step = (std::min)(boss.fMoveSpeed * fixedDeltaSeconds, distance);
			boss.fPositionX += deltaX / distance * step;
			boss.fPositionZ += deltaZ / distance * step;
			return true;
		}
		boss.MovePath.clear();
		boss.iMovePathIndex = 0;
		return false;
	}
}

void LostArk::Server::CValtanBrain::Update(
	SERVER_WORLD_ENTITY& boss,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CGameplayCatalog& catalog,
	const CServerNavigation& navigation,
	const float fixedDeltaSeconds,
	const std::uint32_t serverTick) const
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
	if (boss.iMaximumHp > 0u && boss.iPhaseTwoHpPercent > 0u &&
		static_cast<std::uint64_t>(boss.iCurrentHp) * 100u <=
		static_cast<std::uint64_t>(boss.iMaximumHp) * boss.iPhaseTwoHpPercent)
	{
		boss.iPhase = 2;
	}

	SERVER_PLAYER* target = nullptr;
	float targetDistanceSquared = (std::numeric_limits<float>::max)();
	for (auto& [playerId, player] : players)
	{
		(void)playerId;
		if (0u == player.iCurrentHp ||
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
		boss.fEngageDistance, boss.fPatternMaximumRange);
	if (nullptr == target || targetDistanceSquared > engageDistance * engageDistance)
	{
		Transition(boss, SERVER_ENTITY_ACTION::IDLE, serverTick);
		boss.iTargetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		boss.MovePath.clear();
		return;
	}
	boss.iTargetEntityId = target->iNetEntityId;
	const float distance = std::sqrt(targetDistanceSquared);

	if (SERVER_ENTITY_ACTION::IDLE == boss.eAction ||
		SERVER_ENTITY_ACTION::CHASE == boss.eAction)
	{
		if (distance > boss.fPatternMaximumRange)
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
		Transition(boss, SERVER_ENTITY_ACTION::PATTERN_WINDUP, serverTick);
		const float deltaX = target->fPositionX - boss.fPositionX;
		const float deltaZ = target->fPositionZ - boss.fPositionZ;
		boss.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
	}

	boss.fActionElapsedSeconds += fixedDeltaSeconds;
	switch (boss.eAction)
	{
	case SERVER_ENTITY_ACTION::PATTERN_WINDUP:
		if (boss.fActionElapsedSeconds >=
			static_cast<float>(boss.iPatternTelegraphMs) * MILLISECONDS_TO_SECONDS)
		{
			Transition(boss, SERVER_ENTITY_ACTION::PATTERN_ACTIVE, serverTick);
		}
		break;
	case SERVER_ENTITY_ACTION::PATTERN_ACTIVE:
		if (!boss.hasAppliedPatternDamage)
		{
			const std::uint32_t damage = catalog.Find_Damage(boss.strDamageProfileId);
			const float hitRangeSquared =
				boss.fPatternMaximumRange * boss.fPatternMaximumRange;
			for (auto& [playerId, player] : players)
			{
				(void)playerId;
				if (0u == player.iCurrentHp)
					continue;
				const float deltaX = player.fPositionX - boss.fPositionX;
				const float deltaZ = player.fPositionZ - boss.fPositionZ;
				if (deltaX * deltaX + deltaZ * deltaZ > hitRangeSquared)
					continue;
				player.iCurrentHp = damage >= player.iCurrentHp ?
					0u : player.iCurrentHp - damage;
				if (0u == player.iCurrentHp)
				{
					player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::DEAD;
					player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
					player.iActionStartTick = 0u == serverTick ? 1u : serverTick;
					player.hasMoveGoal = false;
					player.MovePath.clear();
				}
			}
			boss.hasAppliedPatternDamage = true;
		}
		if (boss.fActionElapsedSeconds >=
			static_cast<float>(boss.iPatternActiveMs) * MILLISECONDS_TO_SECONDS)
		{
			Transition(boss, SERVER_ENTITY_ACTION::PATTERN_RECOVERY, serverTick);
		}
		break;
	case SERVER_ENTITY_ACTION::PATTERN_RECOVERY:
		if (boss.fActionElapsedSeconds >=
			static_cast<float>(boss.iPatternRecoveryMs) * MILLISECONDS_TO_SECONDS)
		{
			Transition(boss, SERVER_ENTITY_ACTION::IDLE, serverTick);
		}
		break;
	default:
		break;
	}
}
