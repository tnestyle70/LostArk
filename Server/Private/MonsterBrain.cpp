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

	// 최근접/고정 대상 조회와 Mario 순찰이 같은 인간 플레이어 참가 조건을 사용한다. 사망/낙하/잡힘은 제외한다.
	bool IsAvailableCombatTarget(const LostArk::Server::SERVER_PLAYER& player)
	{
		return player.Is_Human() && 0u != player.iCurrentHp && player.isCombatReady &&
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD != player.eAction &&
			LostArk::Shared::PLAYER_ACTION_STATE::FALLING != player.eAction &&
			LostArk::Shared::PLAYER_ACTION_STATE::GRABBED != player.eAction;
	}

	// 이전 tick에 저장한 NetEntityId를 현재 players에서 재조회한다. 포인터는 map 요소를 차용하며 없으면 nullptr다.
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

	// 참가 가능한 대상 중 XZ 거리 제곱이 제한 안에 있는 가장 가까운 대상을 찾는다.
	// 찾지 못하면 nullptr와 거리 0을 남긴다. 유한 위치와 검증된 profile 범위는 호출 측 상태의 전제다.
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

	// 몬스터/대상의 월드 XZ 거리 제곱(m²)을 구한다. Y 차이는 Mario 등 필요한 경로가 별도로 검사한다.
	float DistanceSquared(
		const LostArk::Server::SERVER_WORLD_ENTITY& monster,
		const LostArk::Server::SERVER_PLAYER& player)
	{
		const float deltaX = player.fPositionX - monster.fPositionX;
		const float deltaZ = player.fPositionZ - monster.fPositionZ;
		return deltaX * deltaX + deltaZ * deltaZ;
	}

	// 회전 차이를 360도 주기로 접어 가까운 방향을 고른다. 비유한 입력은 0으로 반환한다.
	float NormalizeDegrees(const float value)
	{
		if (!std::isfinite(value))
			return 0.f;
		return std::remainder(value, 360.f);
	}

	// 가속/감속에서 한 tick의 최대 변화량 delta만큼 목표값에 접근한다. 양수 delta 사용이 호출 전제다.
	float Approach(const float current, const float target, const float delta)
	{
		if (current < target)
			return (std::min)(current + delta, target);
		return (std::max)(current - delta, target);
	}

	// 행동을 바꾸며 경과 시간과 한 번 타격 소비 상태를 초기화한다. CHASE 이외의 상태에서는 이동 속도도 비운다.
	// 대기/추적은 action ID를 지우고 공격 예고는 공용 기본 공격 ID를 설정한다.
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

	// Mario stage가 지정된 몬스터는 일반 자유 추적 대신 저작된 한 축을 왕복한다.
	// 같은 Mario stage/높이의 대상만 검사하고, 방향 회전을 마친 뒤 전방 공격을 시작한다.
	// 이동 실패나 끝점 도달은 순찰 방향을 바꾸며 피해 처리는 공용 runtime에 전달한다.
	void UpdateMarioPatrol(LostArk::Server::SERVER_WORLD_ENTITY& monster,
		std::map<LostArk::Shared::PLAYER_ID, LostArk::Server::SERVER_PLAYER>& players,
		const LostArk::Server::CGameplayCatalog& catalog,
		const LostArk::Server::CServerNavigation& navigation,
		const LostArk::Server::CServerCollisionSystem& collision,
		const float deltaSeconds, const std::uint32_t tick,
		std::vector<LostArk::Shared::DAMAGE_EVENT>& damageEvents)
	{
		using namespace LostArk::Server;
		using namespace LostArk::Shared;
		// 순찰 대상은 일반 참가 조건 외에 같은 Mario 번호, trigger 이동 아님, 높이 차 0.8m 이하가 필요하다.
		const auto validTarget = [&monster](const SERVER_PLAYER& player) {
			return IsAvailableCombatTarget(player) && player.iMarioStage == monster.iMarioPatrolStage &&
				!player.TriggerMove.isActive && std::abs(player.fPositionY - monster.fPositionY) <= .8f;
		};
		monster.fActionElapsedSeconds += deltaSeconds;
		const bool attacking = monster.eAction == SERVER_ENTITY_ACTION::PATTERN_WINDUP ||
			monster.eAction == SERVER_ENTITY_ACTION::PATTERN_ACTIVE ||
			monster.eAction == SERVER_ENTITY_ACTION::PATTERN_RECOVERY;
		SERVER_PLAYER* target = nullptr;
		if (attacking)
		{
			for (auto& [id, player] : players)
				if (player.iNetEntityId == monster.iTargetEntityId && validTarget(player))
					target = &player;
			if (monster.eAction == SERVER_ENTITY_ACTION::PATTERN_WINDUP &&
				monster.fActionElapsedSeconds >= monster.iPatternTelegraphMs * MILLISECONDS_TO_SECONDS)
				Transition(monster, SERVER_ENTITY_ACTION::PATTERN_ACTIVE, tick);
			else if (monster.eAction == SERVER_ENTITY_ACTION::PATTERN_ACTIVE)
			{
				if (!monster.hasAppliedPatternDamage)
				{
					monster.hasAppliedPatternDamage = true;
					const float reach = monster.fAttackRange + monster.fCollisionRadius;
					if (target && DistanceSquared(monster, *target) <= reach * reach)
					{
						const float dx = target->fPositionX - monster.fPositionX;
						const float dz = target->fPositionZ - monster.fPositionZ;
						const float yaw = monster.fYawDegrees * DEGREES_TO_RADIANS;
						// 고정된 전방을 기준으로 타격하여 모델 뒤로 넘어간 대상은 맞히지 않는다.
						if (dx * std::sin(yaw) + dz * std::cos(yaw) >= 0.f)
						{
							SERVER_WORLD_TO_PLAYER_HIT hit{};
							hit.iRawDamage = monster.iAttackPower;
							hit.fSourceX = monster.fPositionX;
							hit.fSourceZ = monster.fPositionZ;
							hit.fPushRangeM = monster.fAttackPushRangeM;
							hit.iPushMs = monster.iAttackPushMs;
							hit.bKnockdown = monster.bAttackKnockdown;
							hit.iDownMs = monster.iAttackDownMs;
							hit.iServerTick = tick;
							if (monster.strArchetypeId == "MONSTER_MARIO_REUP")
								Configure_MarioHazardLaunch(hit);
							(void)CServerCombatHitRuntime::Apply_WorldToPlayer(*target, hit, catalog, damageEvents);
						}
					}
				}
				if (monster.fActionElapsedSeconds >= monster.iPatternActiveMs * MILLISECONDS_TO_SECONDS)
					Transition(monster, SERVER_ENTITY_ACTION::PATTERN_RECOVERY, tick);
			}
			else if (monster.eAction == SERVER_ENTITY_ACTION::PATTERN_RECOVERY &&
				monster.fActionElapsedSeconds >= monster.iPatternRecoveryMs * MILLISECONDS_TO_SECONDS)
				Transition(monster, SERVER_ENTITY_ACTION::IDLE, tick);
			return;
		}
		float nearest = monster.fEngageDistance * monster.fEngageDistance;
		for (auto& [id, player] : players)
		{
			if (!validTarget(player))
				continue;
			const float distance = DistanceSquared(monster, player);
			if (distance <= nearest)
			{
				nearest = distance;
				target = &player;
			}
		}
		float direction = monster.bMarioPatrolForward ? 1.f : -1.f;
		if (target)
		{
			const float along = (target->fPositionX - monster.fPositionX) * monster.fMarioPatrolAxisX +
				(target->fPositionZ - monster.fPositionZ) * monster.fMarioPatrolAxisZ;
			direction = along >= 0.f ? 1.f : -1.f;
			monster.bMarioPatrolForward = direction > 0.f;
		}
		const float desiredYaw = std::atan2(monster.fMarioPatrolAxisX * direction,
			monster.fMarioPatrolAxisZ * direction) * RADIANS_TO_DEGREES;
		const float difference = NormalizeDegrees(desiredYaw - monster.fYawDegrees);
		const float maxTurn = monster.fTurnSpeedDegreesPerSecond * deltaSeconds;
		monster.fYawDegrees = NormalizeDegrees(monster.fYawDegrees + (std::clamp)(difference, -maxTurn, maxTurn));
		if (std::abs(NormalizeDegrees(desiredYaw - monster.fYawDegrees)) > 2.f)
		{
			if (monster.eAction != SERVER_ENTITY_ACTION::IDLE)
				Transition(monster, SERVER_ENTITY_ACTION::IDLE, tick);
			return;
		}
		if (target)
		{
			monster.iTargetEntityId = target->iNetEntityId;
			Transition(monster, SERVER_ENTITY_ACTION::PATTERN_WINDUP, tick);
			return;
		}
		monster.iTargetEntityId = INVALID_NET_ENTITY_ID;
		const float along = (monster.fPositionX - monster.fSpawnPositionX) * monster.fMarioPatrolAxisX +
			(monster.fPositionZ - monster.fSpawnPositionZ) * monster.fMarioPatrolAxisZ;
		const float goal = monster.bMarioPatrolForward ? monster.fMarioPatrolMaximum : monster.fMarioPatrolMinimum;
		if (std::abs(goal - along) < .1f)
		{
			monster.bMarioPatrolForward = !monster.bMarioPatrolForward;
			Transition(monster, SERVER_ENTITY_ACTION::IDLE, tick);
			return;
		}
		if (monster.eAction != SERVER_ENTITY_ACTION::CHASE)
			Transition(monster, SERVER_ENTITY_ACTION::CHASE, tick);
		monster.fCurrentMoveSpeed = Approach(monster.fCurrentMoveSpeed, monster.fMoveSpeed,
			monster.fMoveAcceleration * deltaSeconds);
		const float next = along + direction * (std::min)(std::abs(goal - along), monster.fCurrentMoveSpeed * deltaSeconds);
		const float x = monster.fSpawnPositionX + monster.fMarioPatrolAxisX * next;
		const float z = monster.fSpawnPositionZ + monster.fMarioPatrolAxisZ * next;
		SERVER_NAV_POINT ground{};
		float resolvedX = 0.f, resolvedY = 0.f, resolvedZ = 0.f;
		bool blocked = false;
		if (!navigation.Resolve_TraversalStep(monster.fPositionX, monster.fPositionZ, x, z, ground) ||
			std::hypot(ground.x - x, ground.z - z) > .01f || std::abs(ground.y - monster.fPositionY) > .25f ||
			!collision.Resolve_CircleMove(monster.fPositionX, monster.fPositionY, monster.fPositionZ,
				ground.x, ground.y, ground.z, monster.fCollisionRadius, monster.fCollisionRadius,
				monster.fCollisionRadius, resolvedX, resolvedY, resolvedZ, blocked, monster.iNetEntityId, false) || blocked)
		{
			monster.bMarioPatrolForward = !monster.bMarioPatrolForward;
			Transition(monster, SERVER_ENTITY_ACTION::IDLE, tick);
			return;
		}
		// Mario 순찰선에서 옆으로 미끄러지지 않도록 검증한 navigation 점을 그대로 확정한다.
		monster.fPositionX = ground.x;
		monster.fPositionY = ground.y;
		monster.fPositionZ = ground.z;
	}
}

// 충돌 저장소를 받지 않은 호출도 같은 Update 구현을 사용한다. 임시 빈 충돌 시스템은 이 호출 동안만 산다.
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

// GameRoom이 몬스터의 밀림 처리 후 호출한다. 사망→stun→Mario 특수 순찰→일반 대상/추적/공격 순서다.
// fixedDeltaSeconds는 초다. 유효하지 않은 delta이면 즉시 반환하고, 피해는 출력 event vector 뒤에 추가한다.
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

	// stun 동안 추적/공격/방향 갱신은 멈추지만 action의 경과 시간은 더한다.
	if (serverTick < monster.iStunEndTick)
	{
		monster.fActionElapsedSeconds += fixedDeltaSeconds;
		return;
	}

	if (monster.iMarioPatrolStage != 0u)
	{
		UpdateMarioPatrol(monster, players, catalog, navigation, collision, fixedDeltaSeconds, serverTick, outDamageEvents);
		return;
	}

	// 공격 중에는 저장한 대상만 재확인한다. 대기/추적에서만 기존 대상 이탈 거리와 새 교전 거리를 적용한다.
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
			// entity ID와 황금각으로 목표 주변 접근점을 분산한다. 충돌 회피를 보장하는 값은 아니며 이후 navigation/충돌 검사를 거친다.
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
			// 매 tick 전체 경로를 다시 찾지 않도록 재탐색을 15~24 tick 뒤에 예약한다. 막힌 경우에는 아래에서 즉시 재계획 표시를 남긴다.
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

			// 마지막 경로 점 근처에서는 도착 반경으로 감속하고, 현재 속도는 가속/감속 제한을 따라 변한다.
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
			// 제안 이동을 navigation→몸체 충돌→최종 navigation 순으로 검증한 뒤에만 위치에 반영한다.
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
				// 공용 hit runtime이 피해/사망/플레이어 반응을 처리한다. 이 Brain은 판정 기회를 한 번만 연다.
				SERVER_WORLD_TO_PLAYER_HIT incoming{};
				incoming.iRawDamage = monster.iAttackFixedDamage ? monster.iAttackFixedDamage : monster.iAttackPower;
				incoming.bIgnoreDefense = monster.iAttackFixedDamage != 0u;
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
			// 실제 HP 피해나 명중 여부와 무관하게 이번 기본 공격의 한 번 판정 기회를 소비한다.
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

// 별도 충돌 시스템이 없는 호출을 같은 밀림 처리로 연결한다.
bool LostArk::Server::CMonsterBrain::Advance_Knockback(
	SERVER_WORLD_ENTITY& monster,
	const CServerNavigation& navigation,
	const float fixedDeltaSeconds)
{
	CServerCollisionSystem noCollision;
	return Advance_Knockback(
		monster, navigation, noCollision, fixedDeltaSeconds);
}

// GameRoom이 일반 Update 전에 호출한다. 남은 밀림 시간과 delta 중 작은 구간만 이동한다.
// 이동을 막아도 그 tick은 소비한 것으로 true를 반환한다. 실제 위치 성공 여부와 bool을 혼동하지 않는다.
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
		wasClamped,
		monster.fPositionY);
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
	// 밀림 tick에도 action 시간은 흐른다. 다음 Update가 이어진 경과 시간으로 공격 단계 마감을 판단한다.
	monster.fActionElapsedSeconds += fixedDeltaSeconds;
	return true;
}
