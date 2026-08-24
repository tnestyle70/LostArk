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

	void Transition(LostArk::Server::SERVER_WORLD_ENTITY& monster,
		const LostArk::Server::SERVER_ENTITY_ACTION action,
		const std::uint32_t serverTick)
	{
		monster.eAction = action;
		monster.fActionElapsedSeconds = 0.f;
		monster.iActionStartTick = 0u == serverTick ? 1u : serverTick;
		monster.hasAppliedPatternDamage = false;
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

	SERVER_PLAYER* target = nullptr;
	float targetDistanceSquared = 0.f;
	for (auto& [playerId, player] : players)
	{
		(void)playerId;
		if (0u == player.iCurrentHp || !player.isCombatReady ||
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction)
			continue;
		const float deltaX = player.fPositionX - monster.fPositionX;
		const float deltaZ = player.fPositionZ - monster.fPositionZ;
		const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
		if (nullptr == target || distanceSquared < targetDistanceSquared)
		{
			target = &player;
			targetDistanceSquared = distanceSquared;
		}
	}
	monster.fActionElapsedSeconds += fixedDeltaSeconds;
	if (nullptr == target ||
		targetDistanceSquared > monster.fEngageDistance * monster.fEngageDistance)
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
			navigation.Find_Path(monster.fPositionX, monster.fPositionZ,
				target->fPositionX, target->fPositionZ, monster.MovePath);
			monster.iNextPathReplanTick = serverTick + 15u +
				static_cast<std::uint32_t>(monster.iNetEntityId % 10u);
		}
		if (!monster.MovePath.empty() && monster.iMovePathIndex < monster.MovePath.size())
		{
			const SERVER_NAV_POINT& point = monster.MovePath[monster.iMovePathIndex];
			const float deltaX = point.x - monster.fPositionX;
			const float deltaZ = point.z - monster.fPositionZ;
			const float stepDistance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
			if (stepDistance <= 0.08f)
			{
				++monster.iMovePathIndex;
				if (monster.iMovePathIndex >= monster.MovePath.size())
					monster.MovePath.clear();
			}
			else
			{
				const float moveDistance = (std::min)(
					monster.fMoveSpeed * fixedDeltaSeconds, stepDistance);
				const float ratio = moveDistance / stepDistance;
				monster.fPositionX += deltaX * ratio;
				monster.fPositionY += (point.y - monster.fPositionY) * ratio;
				monster.fPositionZ += deltaZ * ratio;
				monster.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
			}
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
	monster.fPositionX = reachable.x;
	monster.fPositionY = reachable.y;
	monster.fPositionZ = reachable.z;
	monster.fKnockbackRemainingSeconds = wasClamped ?
		0.f : monster.fKnockbackRemainingSeconds - step;
	/* Pattern timers keep running so a windup is not frozen by a hit. */
	monster.fActionElapsedSeconds += fixedDeltaSeconds;
	return true;
}
