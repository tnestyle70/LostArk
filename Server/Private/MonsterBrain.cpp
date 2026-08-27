#include "MonsterBrain.h"

#include "Gameplay/CombatCollisionContract.h"
#include "Gameplay/WorldCollisionContract.h"
#include "PlayerSkillSystem.h"
#include "ServerCombatHitRuntime.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float MILLISECONDS_TO_SECONDS = 0.001f;
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
	constexpr float DEGREES_TO_RADIANS = 0.0174532925f;
	constexpr float GOLDEN_ANGLE_RADIANS = 2.39996323f;
	constexpr float TWO_PI = 6.28318531f;
	constexpr float WAYPOINT_REACHED_DISTANCE = 0.12f;

	bool IsAvailableCombatTarget(const LostArk::Server::SERVER_PLAYER& player)
	{
		return 0u != player.iCurrentHp && player.isCombatReady &&
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD != player.eAction &&
			LostArk::Shared::PLAYER_ACTION_STATE::FALLING != player.eAction &&
			LostArk::Shared::PLAYER_ACTION_STATE::GRABBED != player.eAction;
	}

	LostArk::Server::SERVER_PLAYER* FindPlayerByEntityId(
		std::map<LostArk::Shared::PLAYER_ID,
			LostArk::Server::SERVER_PLAYER>& players,
		const LostArk::Shared::NET_ENTITY_ID entityId)
	{
		if (LostArk::Shared::INVALID_NET_ENTITY_ID == entityId)
			return nullptr;
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			if (player.iNetEntityId == entityId && IsAvailableCombatTarget(player))
				return &player;
		}
		return nullptr;
	}

	LostArk::Server::SERVER_PLAYER* FindNearestPlayer(
		const LostArk::Server::SERVER_WORLD_ENTITY& monster,
		std::map<LostArk::Shared::PLAYER_ID,
			LostArk::Server::SERVER_PLAYER>& players,
		const float maximumDistanceSquared,
		float& outDistanceSquared)
	{
		LostArk::Server::SERVER_PLAYER* nearest = nullptr;
		outDistanceSquared = 0.f;
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			if (!IsAvailableCombatTarget(player))
				continue;
			const float deltaX = player.fPositionX - monster.fPositionX;
			const float deltaZ = player.fPositionZ - monster.fPositionZ;
			const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
			if (distanceSquared > maximumDistanceSquared)
				continue;
			if (nullptr == nearest || distanceSquared < outDistanceSquared)
			{
				nearest = &player;
				outDistanceSquared = distanceSquared;
			}
		}
		return nearest;
	}

	float DistanceSquared(
		const LostArk::Server::SERVER_WORLD_ENTITY& monster,
		const LostArk::Server::SERVER_PLAYER& player)
	{
		const float deltaX = player.fPositionX - monster.fPositionX;
		const float deltaZ = player.fPositionZ - monster.fPositionZ;
		return deltaX * deltaX + deltaZ * deltaZ;
	}

	float NormalizeDegrees(float value)
	{
		while (value > 180.f)
			value -= 360.f;
		while (value < -180.f)
			value += 360.f;
		return value;
	}

	float Approach(const float current, const float target, const float delta)
	{
		if (current < target)
			return (std::min)(current + delta, target);
		return (std::max)(current - delta, target);
	}

	void Transition(LostArk::Server::SERVER_WORLD_ENTITY& monster,
		const LostArk::Server::SERVER_ENTITY_ACTION action,
		const std::uint32_t serverTick)
	{
		monster.eAction = action;
		monster.fActionElapsedSeconds = 0.f;
		monster.iActionStartTick = 0u == serverTick ? 1u : serverTick;
		monster.hasAppliedPatternDamage = false;
		if (LostArk::Server::SERVER_ENTITY_ACTION::CHASE != action)
			monster.fCurrentMoveSpeed = 0.f;
		if (LostArk::Server::SERVER_ENTITY_ACTION::PATTERN_WINDUP == action)
			monster.strActionId = "monster.basic.attack";
		else if (LostArk::Server::SERVER_ENTITY_ACTION::IDLE == action ||
			LostArk::Server::SERVER_ENTITY_ACTION::CHASE == action)
			monster.strActionId.clear();
	}
}

void LostArk::Server::CMonsterBrain::Update(
	SERVER_WORLD_ENTITY& monster,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CGameplayCatalog& catalog,
	const CServerNavigation& navigation,
	const float fixedDeltaSeconds,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const
{
	CServerCollisionSystem noCollision;
	Update(monster, players, catalog, navigation, noCollision,
		fixedDeltaSeconds, serverTick, outDamageEvents);
}

void LostArk::Server::CMonsterBrain::Update(
	SERVER_WORLD_ENTITY& monster,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CGameplayCatalog& catalog,
	const CServerNavigation& navigation,
	const CServerCollisionSystem& collision,
	const float fixedDeltaSeconds,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const
{
	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
		return;
	if (0u == monster.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == monster.eAction)
	{
		monster.iCurrentHp = 0u;
		if (SERVER_ENTITY_ACTION::DEAD != monster.eAction)
			Transition(monster, SERVER_ENTITY_ACTION::DEAD, serverTick);
		else
			monster.fActionElapsedSeconds += fixedDeltaSeconds;
		return;
	}

	const bool attackInProgress =
		SERVER_ENTITY_ACTION::PATTERN_WINDUP == monster.eAction ||
		SERVER_ENTITY_ACTION::PATTERN_ACTIVE == monster.eAction ||
		SERVER_ENTITY_ACTION::PATTERN_RECOVERY == monster.eAction;
	SERVER_PLAYER* target = FindPlayerByEntityId(
		players, monster.iTargetEntityId);
	float targetDistanceSquared = nullptr == target ?
		0.f : DistanceSquared(monster, *target);
	if (!attackInProgress)
	{
		const float releaseDistance = monster.fTargetReleaseDistance > 0.f ?
			monster.fTargetReleaseDistance : monster.fEngageDistance;
		if (nullptr != target &&
			targetDistanceSquared > releaseDistance * releaseDistance)
		{
			target = nullptr;
		}
		if (nullptr == target)
		{
			target = FindNearestPlayer(
				monster,
				players,
				monster.fEngageDistance * monster.fEngageDistance,
				targetDistanceSquared);
		}
	}
	monster.fActionElapsedSeconds += fixedDeltaSeconds;
	if (nullptr == target)
	{
		monster.iTargetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		monster.MovePath.clear();
		if (SERVER_ENTITY_ACTION::IDLE != monster.eAction)
			Transition(monster, SERVER_ENTITY_ACTION::IDLE, serverTick);
		return;
	}
	monster.iTargetEntityId = target->iNetEntityId;
	const LostArk::Shared::CombatCollision::CIRCLE_XZ attackCircle{
		monster.fPositionX,
		monster.fPositionZ,
		monster.fAttackRange + monster.fCollisionRadius
	};
	const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ targetBody{
		target->fPositionX,
		target->fPositionZ,
		LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X
	};
	if (SERVER_ENTITY_ACTION::IDLE == monster.eAction ||
		SERVER_ENTITY_ACTION::CHASE == monster.eAction)
	{
		if (LostArk::Shared::CombatCollision::Circles_Overlap(
			attackCircle, targetBody))
		{
			monster.MovePath.clear();
			monster.fYawDegrees = std::atan2(
				target->fPositionX - monster.fPositionX,
				target->fPositionZ - monster.fPositionZ) * RADIANS_TO_DEGREES;
			Transition(monster, SERVER_ENTITY_ACTION::PATTERN_WINDUP, serverTick);
			return;
		}
		if (SERVER_ENTITY_ACTION::CHASE != monster.eAction)
			Transition(monster, SERVER_ENTITY_ACTION::CHASE, serverTick);
		if (monster.MovePath.empty() ||
			static_cast<std::int32_t>(serverTick - monster.iNextPathReplanTick) >= 0)
		{
			monster.MovePath.clear();
			monster.iMovePathIndex = 0;
			const float approachAngle = std::fmod(
				static_cast<float>(monster.iNetEntityId) *
					GOLDEN_ANGLE_RADIANS,
				TWO_PI);
			const float approachRadius = (std::max)(
				monster.fCollisionRadius,
				monster.fAttackRange + monster.fCollisionRadius +
					LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X - 0.15f);
			float goalX = target->fPositionX +
				std::sin(approachAngle) * approachRadius;
			float goalZ = target->fPositionZ +
				std::cos(approachAngle) * approachRadius;
			SERVER_NAV_POINT approachPoint{};
			if (!navigation.Sample_Position(goalX, goalZ, approachPoint))
			{
				goalX = target->fPositionX;
				goalZ = target->fPositionZ;
			}
			navigation.Find_Path(monster.fPositionX, monster.fPositionZ,
				goalX, goalZ, monster.MovePath);
			navigation.Smooth_Path(
				monster.fPositionX, monster.fPositionZ,
				goalX, goalZ, monster.MovePath);
			monster.iNextPathReplanTick = serverTick + 15u +
				static_cast<std::uint32_t>(monster.iNetEntityId % 10u);
		}
		while (!monster.MovePath.empty() &&
			monster.iMovePathIndex < monster.MovePath.size())
		{
			const SERVER_NAV_POINT& point = monster.MovePath[monster.iMovePathIndex];
			const float deltaX = point.x - monster.fPositionX;
			const float deltaZ = point.z - monster.fPositionZ;
			const float stepDistance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
			if (stepDistance <= WAYPOINT_REACHED_DISTANCE)
			{
				++monster.iMovePathIndex;
				if (monster.iMovePathIndex >= monster.MovePath.size())
					monster.MovePath.clear();
				continue;
			}

			const float desiredYaw =
				std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
			const float turnSpeed = monster.fTurnSpeedDegreesPerSecond > 0.f ?
				monster.fTurnSpeedDegreesPerSecond : 720.f;
			const float yawDelta = NormalizeDegrees(
				desiredYaw - monster.fYawDegrees);
			const float maximumYawStep = turnSpeed * fixedDeltaSeconds;
			monster.fYawDegrees = NormalizeDegrees(monster.fYawDegrees +
				(std::clamp)(yawDelta, -maximumYawStep, maximumYawStep));

			float desiredSpeed = monster.fMoveSpeed;
			if (monster.iMovePathIndex + 1u == monster.MovePath.size())
			{
				const float slowRadius = monster.fArrivalSlowRadius > 0.f ?
					monster.fArrivalSlowRadius : monster.fCollisionRadius * 2.f;
				desiredSpeed *= (std::clamp)(stepDistance / slowRadius, 0.2f, 1.f);
			}
			const float acceleration = monster.fMoveAcceleration > 0.f ?
				monster.fMoveAcceleration : monster.fMoveSpeed * 30.f;
			const float deceleration = monster.fMoveDeceleration > 0.f ?
				monster.fMoveDeceleration : acceleration;
			monster.fCurrentMoveSpeed = Approach(
				monster.fCurrentMoveSpeed,
				desiredSpeed,
				(monster.fCurrentMoveSpeed < desiredSpeed ?
					acceleration : deceleration) * fixedDeltaSeconds);

			const float moveDistance = (std::min)(
				monster.fCurrentMoveSpeed * fixedDeltaSeconds, stepDistance);
			const float yawRadians = monster.fYawDegrees * DEGREES_TO_RADIANS;
			const float proposedX = monster.fPositionX +
				std::sin(yawRadians) * moveDistance;
			const float proposedZ = monster.fPositionZ +
				std::cos(yawRadians) * moveDistance;
			SERVER_NAV_POINT navigationStep{};
			if (!navigation.Resolve_TraversalStep(
				monster.fPositionX, monster.fPositionZ,
				proposedX, proposedZ, navigationStep))
			{
				monster.iNextPathReplanTick = serverTick;
				monster.fCurrentMoveSpeed = Approach(
					monster.fCurrentMoveSpeed, 0.f,
					deceleration * fixedDeltaSeconds);
				break;
			}
			float resolvedX = navigationStep.x;
			float resolvedY = navigationStep.y;
			float resolvedZ = navigationStep.z;
			bool wasBlocked = false;
			if (!collision.Resolve_CircleMove(
				monster.fPositionX, monster.fPositionY, monster.fPositionZ,
				navigationStep.x, navigationStep.y, navigationStep.z,
				monster.fCollisionRadius, monster.fCollisionRadius,
				monster.fCollisionRadius,
				resolvedX, resolvedY, resolvedZ, wasBlocked,
				monster.iNetEntityId))
			{
				break;
			}
			SERVER_NAV_POINT finalStep{};
			if (!navigation.Resolve_TraversalStep(
				monster.fPositionX, monster.fPositionZ,
				resolvedX, resolvedZ, finalStep))
			{
				monster.iNextPathReplanTick = serverTick;
				break;
			}
			monster.fPositionX = finalStep.x;
			monster.fPositionY = finalStep.y;
			monster.fPositionZ = finalStep.z;
			if (wasBlocked)
			{
				monster.iNextPathReplanTick = serverTick;
				monster.fCurrentMoveSpeed = Approach(
					monster.fCurrentMoveSpeed, 0.f,
					deceleration * fixedDeltaSeconds);
			}
			break;
		}
		return;
	}

	if (SERVER_ENTITY_ACTION::PATTERN_WINDUP == monster.eAction &&
		monster.fActionElapsedSeconds >=
			static_cast<float>(monster.iPatternTelegraphMs) * MILLISECONDS_TO_SECONDS)
	{
		Transition(monster, SERVER_ENTITY_ACTION::PATTERN_ACTIVE, serverTick);
		return;
	}
	if (SERVER_ENTITY_ACTION::PATTERN_ACTIVE == monster.eAction)
	{
		if (!monster.hasAppliedPatternDamage)
		{
			if (LostArk::Shared::CombatCollision::Circles_Overlap(
				attackCircle, targetBody))
			{
				const PLAYER_RUNTIME_PROFILE* playerProfile =
					catalog.Find_Player(target->eCharacterClass);
				const std::uint32_t damage = CGameplayCatalog::Apply_Defense(
					monster.iAttackPower,
					nullptr == playerProfile ? 0u : playerProfile->iDefense);
				target->iCurrentHp = damage >= target->iCurrentHp ?
					0u : target->iCurrentHp - damage;
				if (0u != damage &&
					outDamageEvents.size() < LostArk::Shared::MAX_DAMAGE_EVENTS)
				{
					LostArk::Shared::DAMAGE_EVENT event{};
					event.iTargetNetEntityId = target->iNetEntityId;
					event.iAmount = damage;
					event.fPositionX = target->fPositionX;
					event.fPositionY = target->fPositionY;
					event.fPositionZ = target->fPositionZ;
					event.isOutgoing = false;
					outDamageEvents.push_back(event);
				}
				if (0u == target->iCurrentHp)
				{
					target->eAction = LostArk::Shared::PLAYER_ACTION_STATE::DEAD;
					target->iCurrentSkillId =
						LostArk::Shared::INVALID_SKILL_ID;
					target->Clear_SkillTarget();
					target->iActionStartTick = 0u == serverTick ? 1u : serverTick;
					target->hasBufferedComboInput = false;
					target->PendingCommand.Clear();
					target->hasMoveGoal = false;
					target->MovePath.clear();
				}
				else
				{
					CPlayerSkillSystem::Arm_PlayerHitReaction(
						*target,
						monster.fPositionX,
						monster.fPositionZ,
						monster.fAttackPushRangeM,
						monster.iAttackPushMs,
						monster.bAttackKnockdown,
						monster.iAttackDownMs,
						serverTick);
				}
				SERVER_WORLD_TO_PLAYER_HIT incoming{};
				incoming.iRawDamage = monster.iAttackPower;
				incoming.fSourceX = monster.fPositionX;
				incoming.fSourceZ = monster.fPositionZ;
				incoming.fPushRangeM = monster.fAttackPushRangeM;
				incoming.iPushMs = monster.iAttackPushMs;
				incoming.bKnockdown = monster.bAttackKnockdown;
				incoming.iDownMs = monster.iAttackDownMs;
				incoming.iServerTick = serverTick;
				(void)CServerCombatHitRuntime::Apply_WorldToPlayer(
					*target, incoming, catalog, outDamageEvents);
			}
			monster.hasAppliedPatternDamage = true;
		}
		if (monster.fActionElapsedSeconds >=
			static_cast<float>(monster.iPatternActiveMs) * MILLISECONDS_TO_SECONDS)
			Transition(monster, SERVER_ENTITY_ACTION::PATTERN_RECOVERY, serverTick);
		return;
	}
	if (SERVER_ENTITY_ACTION::PATTERN_RECOVERY == monster.eAction &&
		monster.fActionElapsedSeconds >=
			static_cast<float>(monster.iPatternRecoveryMs) * MILLISECONDS_TO_SECONDS)
	{
		Transition(monster, SERVER_ENTITY_ACTION::IDLE, serverTick);
	}
}

bool LostArk::Server::CMonsterBrain::Advance_Knockback(
	SERVER_WORLD_ENTITY& monster,
	const CServerNavigation& navigation,
	const float fixedDeltaSeconds)
{
	CServerCollisionSystem noCollision;
	return Advance_Knockback(
		monster, navigation, noCollision, fixedDeltaSeconds);
}

bool LostArk::Server::CMonsterBrain::Advance_Knockback(
	SERVER_WORLD_ENTITY& monster,
	const CServerNavigation& navigation,
	const CServerCollisionSystem& collision,
	const float fixedDeltaSeconds)
{
	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f ||
		monster.fKnockbackRemainingSeconds <= 0.f ||
		SERVER_ENTITY_ACTION::DEAD == monster.eAction ||
		0u == monster.iCurrentHp)
	{
		return false;
	}
	const float step = (std::min)(
		fixedDeltaSeconds, monster.fKnockbackRemainingSeconds);
	const float desiredX = monster.fPositionX +
		monster.fKnockbackDirectionX * monster.fKnockbackSpeed * step;
	const float desiredZ = monster.fPositionZ +
		monster.fKnockbackDirectionZ * monster.fKnockbackSpeed * step;
	SERVER_NAV_POINT reachable{ desiredX, monster.fPositionY, desiredZ };
	bool wasClamped = false;
	CPlayerSkillSystem::Clamp_StepToWalkable(
		navigation,
		monster.fPositionX,
		monster.fPositionZ,
		desiredX,
		desiredZ,
		reachable,
		wasClamped);
	float resolvedX = reachable.x;
	float resolvedY = reachable.y;
	float resolvedZ = reachable.z;
	bool wasBodyBlocked = false;
	if (!collision.Resolve_CircleMove(
		monster.fPositionX, monster.fPositionY, monster.fPositionZ,
		reachable.x, reachable.y, reachable.z,
		monster.fCollisionRadius, monster.fCollisionRadius,
		monster.fCollisionRadius,
		resolvedX, resolvedY, resolvedZ, wasBodyBlocked,
		monster.iNetEntityId))
	{
		wasBodyBlocked = true;
		resolvedX = monster.fPositionX;
		resolvedY = monster.fPositionY;
		resolvedZ = monster.fPositionZ;
	}
	SERVER_NAV_POINT finalStep{};
	if (navigation.Resolve_TraversalStep(
		monster.fPositionX, monster.fPositionZ,
		resolvedX, resolvedZ, finalStep))
	{
		monster.fPositionX = finalStep.x;
		monster.fPositionY = finalStep.y;
		monster.fPositionZ = finalStep.z;
	}
	else
	{
		wasClamped = true;
	}
	monster.fKnockbackRemainingSeconds = wasClamped ?
		0.f : (wasBodyBlocked ? 0.f :
			monster.fKnockbackRemainingSeconds - step);
	/* Pattern timers keep running so a windup is not frozen by a hit. */
	monster.fActionElapsedSeconds += fixedDeltaSeconds;
	return true;
}
