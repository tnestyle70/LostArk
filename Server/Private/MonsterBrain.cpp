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
	/* How fast a monster may pivot while its telegraph plays. A 450 ms jab
	windup covers about 190 degrees at this rate, so a player circling the
	monster is tracked without the body snapping to face them in one tick. */
	constexpr float WINDUP_TURN_DEGREES_PER_SECOND = 420.f;
	/* Golden angle: consecutive net entity ids land far apart on the approach
	ring, so a wave that spawns in id order surrounds its target instead of
	filing along one line. Derived from the id rather than a random draw so the
	same tick always produces the same formation. */
	constexpr float APPROACH_RING_GOLDEN_DEGREES = 137.507764f;
	/* How far a monster pushes itself out of a body it overlaps, per second.
	Both sides run it, so a stacked pair separates at twice this rate. */
	constexpr float SEPARATION_METRES_PER_SECOND = 1.6f;
	constexpr float NEGLIGIBLE_DISTANCE_SQUARED = 1e-6f;
	/* A waypoint counts as reached inside this radius. Shared by the chase and
	the patrol so both consume a path the same way. */
	constexpr float WAYPOINT_ARRIVAL_METRES = 0.08f;
	/* How far from its spawn anchor a disengaged monster has to be before it
	walks home. Wider than the patrol ring so arriving does not immediately
	trigger another return. */
	constexpr float PATROL_RETURN_METRES = 4.f;
	/* Radius of the wander the monster keeps once it is home. */
	constexpr float PATROL_RING_METRES = 3.f;
	/* Standing time at each patrol point before choosing the next one. */
	constexpr float PATROL_DWELL_SECONDS = 2.5f;
	/* Disengaged walking is a stroll, not the battle run the chase uses. The
	catalog's patrol clip is a walk, so the speed has to match it. */
	constexpr float PATROL_SPEED_SCALE = 0.45f;
	/* Turns the patrol step into a heading. Coprime with 360 so a monster works
	its way around the ring instead of bouncing between two points. */
	constexpr float PATROL_STEP_DEGREES = 97.f;

	bool Is_Engageable(const LostArk::Server::SERVER_PLAYER& player)
	{
		return 0u != player.iCurrentHp && player.isCombatReady &&
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD != player.eAction;
	}

	LostArk::Server::SERVER_PLAYER* Find_PlayerByEntityId(
		std::map<LostArk::Shared::PLAYER_ID,
			LostArk::Server::SERVER_PLAYER>& players,
		const LostArk::Shared::NET_ENTITY_ID entityId)
	{
		if (LostArk::Shared::INVALID_NET_ENTITY_ID == entityId)
			return nullptr;
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			if (player.iNetEntityId == entityId)
				return &player;
		}
		return nullptr;
	}

	float Wrap_Degrees(float degrees)
	{
		while (degrees > 180.f)
			degrees -= 360.f;
		while (degrees < -180.f)
			degrees += 360.f;
		return degrees;
	}

	/* Rotates the body at most maxTurnDegrees toward the point, along the
	shorter arc. The yaw convention matches the chase step's own
	atan2(deltaX, deltaZ). */
	void Turn_Toward(
		LostArk::Server::SERVER_WORLD_ENTITY& monster,
		const float targetX,
		const float targetZ,
		const float maxTurnDegrees)
	{
		const float deltaX = targetX - monster.fPositionX;
		const float deltaZ = targetZ - monster.fPositionZ;
		if (deltaX * deltaX + deltaZ * deltaZ <= NEGLIGIBLE_DISTANCE_SQUARED)
			return;
		const float desiredYaw =
			std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
		const float difference = Wrap_Degrees(desiredYaw - monster.fYawDegrees);
		monster.fYawDegrees = Wrap_Degrees(monster.fYawDegrees +
			(std::clamp)(difference, -maxTurnDegrees, maxTurnDegrees));
	}

	/* One fixed tick along the staged path at the given speed. Returns true once
	the path is spent, which is how the caller learns it arrived. Both the chase
	and the patrol run this, so there is one movement step rather than a second
	copy that could drift from it. */
	bool Advance_AlongPath(
		LostArk::Server::SERVER_WORLD_ENTITY& monster,
		const float moveSpeed,
		const float fixedDeltaSeconds)
	{
		if (monster.MovePath.empty() ||
			monster.iMovePathIndex >= monster.MovePath.size())
		{
			return true;
		}
		const LostArk::Server::SERVER_NAV_POINT& point =
			monster.MovePath[monster.iMovePathIndex];
		const float deltaX = point.x - monster.fPositionX;
		const float deltaZ = point.z - monster.fPositionZ;
		const float stepDistance =
			std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
		if (stepDistance <= WAYPOINT_ARRIVAL_METRES)
		{
			++monster.iMovePathIndex;
			if (monster.iMovePathIndex >= monster.MovePath.size())
			{
				monster.MovePath.clear();
				return true;
			}
			return false;
		}
		const float moveDistance =
			(std::min)(moveSpeed * fixedDeltaSeconds, stepDistance);
		const float ratio = moveDistance / stepDistance;
		monster.fPositionX += deltaX * ratio;
		monster.fPositionY += (point.y - monster.fPositionY) * ratio;
		monster.fPositionZ += deltaZ * ratio;
		monster.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
		return false;
	}

	/* One fixed tick of body separation against the other living monsters. The
	push is applied to this monster only; every overlapping neighbour runs the
	same step on its own tick, so a pile resolves symmetrically without one
	entity being declared the mover. The boss is excluded because its authored
	body radius lives in the encounter profile, not on the entity. */
	void Separate_FromNeighbours(
		LostArk::Server::SERVER_WORLD_ENTITY& monster,
		const std::vector<LostArk::Server::SERVER_WORLD_ENTITY>& worldEntities,
		const LostArk::Server::CServerNavigation& navigation,
		const float fixedDeltaSeconds)
	{
		if (monster.fCollisionRadius <= 0.f)
			return;
		float pushX = 0.f;
		float pushZ = 0.f;
		for (const LostArk::Server::SERVER_WORLD_ENTITY& other : worldEntities)
		{
			if (&other == &monster ||
				LostArk::Server::WORLD_BOOTSTRAP_KIND::MONSTER != other.eKind ||
				other.isEstherSummon || 0u == other.iCurrentHp ||
				LostArk::Server::SERVER_ENTITY_ACTION::DEAD == other.eAction ||
				other.fCollisionRadius <= 0.f)
			{
				continue;
			}
			const float deltaX = monster.fPositionX - other.fPositionX;
			const float deltaZ = monster.fPositionZ - other.fPositionZ;
			const float contact =
				monster.fCollisionRadius + other.fCollisionRadius;
			const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
			if (distanceSquared >= contact * contact)
				continue;
			if (distanceSquared <= NEGLIGIBLE_DISTANCE_SQUARED)
			{
				/* A wave entry spawns every one of its monsters on the same
				anchor, so a pair really can share a position exactly. Their own
				ids break the tie instead of leaving them fused forever. */
				const float radians = static_cast<float>(
					monster.iNetEntityId % 360u) * DEGREES_TO_RADIANS;
				pushX += std::sin(radians);
				pushZ += std::cos(radians);
				continue;
			}
			const float distance = std::sqrt(distanceSquared);
			const float overlapRatio = (contact - distance) / contact;
			pushX += deltaX / distance * overlapRatio;
			pushZ += deltaZ / distance * overlapRatio;
		}
		const float pushLengthSquared = pushX * pushX + pushZ * pushZ;
		if (pushLengthSquared <= NEGLIGIBLE_DISTANCE_SQUARED)
			return;
		const float pushLength = std::sqrt(pushLengthSquared);
		const float step = SEPARATION_METRES_PER_SECOND * fixedDeltaSeconds;
		const float desiredX =
			monster.fPositionX + pushX / pushLength * step;
		const float desiredZ =
			monster.fPositionZ + pushZ / pushLength * step;
		LostArk::Server::SERVER_NAV_POINT reachable{
			desiredX, monster.fPositionY, desiredZ };
		/* A push that runs into a wall simply stops at it. Unlike a knockback
		there is no window to end early, so the clamp flag carries nothing to
		act on here. */
		bool wasClamped = false;
		LostArk::Server::CPlayerSkillSystem::Clamp_StepToWalkable(
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
	}

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
			LostArk::Server::SERVER_ENTITY_ACTION::CHASE == action ||
			LostArk::Server::SERVER_ENTITY_ACTION::PATROL == action)
			monster.strActionId.clear();
	}
}

namespace
{
	/* What a monster does with nobody to fight: walk back to the spawn anchor it
	was placed on, then keep a slow wander around it with a pause at each point.
	Standing wherever the chase ended left a pack strewn across the map once a
	player pulled it and ran, and nothing ever brought them home. */
	void Update_Disengaged(
		LostArk::Server::SERVER_WORLD_ENTITY& monster,
		const LostArk::Server::CServerNavigation& navigation,
		const float fixedDeltaSeconds,
		const std::uint32_t serverTick)
	{
		using ACTION = LostArk::Server::SERVER_ENTITY_ACTION;
		monster.iTargetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		if (ACTION::PATROL != monster.eAction && ACTION::IDLE != monster.eAction)
		{
			/* Leaving a fight: drop the chase route before the patrol decides
			anything, so a stale path is never walked as if it were a patrol. */
			monster.MovePath.clear();
			monster.iMovePathIndex = 0;
			Transition(monster, ACTION::IDLE, serverTick);
		}
		if (ACTION::PATROL == monster.eAction)
		{
			if (!Advance_AlongPath(monster,
				monster.fMoveSpeed * PATROL_SPEED_SCALE, fixedDeltaSeconds))
			{
				return;
			}
			Transition(monster, ACTION::IDLE, serverTick);
			return;
		}
		/* The trip home and the wander both wait out the dwell. That reads as a
		monster giving up and then turning around, and it is also what keeps a
		goal that cannot be pathed to from re-running the search every tick. */
		if (monster.fActionElapsedSeconds < PATROL_DWELL_SECONDS)
			return;
		const float homeDeltaX = monster.fSpawnPositionX - monster.fPositionX;
		const float homeDeltaZ = monster.fSpawnPositionZ - monster.fPositionZ;
		const bool isFarFromHome =
			homeDeltaX * homeDeltaX + homeDeltaZ * homeDeltaZ >
				PATROL_RETURN_METRES * PATROL_RETURN_METRES;
		float goalX = monster.fSpawnPositionX;
		float goalZ = monster.fSpawnPositionZ;
		if (!isFarFromHome)
		{
			++monster.iPatrolStep;
			const float radians = std::fmod(
				static_cast<float>(monster.iNetEntityId % 360u) +
					static_cast<float>(monster.iPatrolStep) *
						PATROL_STEP_DEGREES,
				360.f) * DEGREES_TO_RADIANS;
			goalX += std::sin(radians) * PATROL_RING_METRES;
			goalZ += std::cos(radians) * PATROL_RING_METRES;
		}
		monster.MovePath.clear();
		monster.iMovePathIndex = 0;
		navigation.Find_Path(
			monster.fPositionX, monster.fPositionZ, goalX, goalZ,
			monster.MovePath);
		if (monster.MovePath.empty())
		{
			/* An unreachable point costs one dwell, not a search per tick, and
			advancing the step means the retry picks a different heading rather
			than asking for the same blocked arc forever. */
			++monster.iPatrolStep;
			Transition(monster, ACTION::IDLE, serverTick);
			return;
		}
		monster.fPatrolGoalX = goalX;
		monster.fPatrolGoalZ = goalZ;
		Transition(monster, ACTION::PATROL, serverTick);
	}
}

void LostArk::Server::CMonsterBrain::Update(
	SERVER_WORLD_ENTITY& monster,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const std::vector<SERVER_WORLD_ENTITY>& worldEntities,
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
	/* Runs before any action branch, so a wave that spawned every monster of one
	entry on a single anchor unstacks itself while idle, not only once something
	walks. A corpse is skipped above, so bodies stop pushing when they die. */
	Separate_FromNeighbours(
		monster, worldEntities, navigation, fixedDeltaSeconds);

	const float engageDistanceSquared =
		monster.fEngageDistance * monster.fEngageDistance;
	SERVER_PLAYER* target = nullptr;
	float targetDistanceSquared = 0.f;
	/* Hold the current target for as long as it stays engageable and in range.
	Re-electing the nearest player every tick made two players at a similar
	distance trade the monster thirty times a second, which shows up as a body
	that jitters between them instead of committing to one. */
	if (SERVER_PLAYER* held =
			Find_PlayerByEntityId(players, monster.iTargetEntityId);
		nullptr != held && Is_Engageable(*held))
	{
		const float deltaX = held->fPositionX - monster.fPositionX;
		const float deltaZ = held->fPositionZ - monster.fPositionZ;
		const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
		if (distanceSquared <= engageDistanceSquared)
		{
			target = held;
			targetDistanceSquared = distanceSquared;
		}
	}
	if (nullptr == target)
	{
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			if (!Is_Engageable(player))
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
	}
	monster.fActionElapsedSeconds += fixedDeltaSeconds;
	if (nullptr == target || targetDistanceSquared > engageDistanceSquared)
	{
		Update_Disengaged(monster, navigation, fixedDeltaSeconds, serverTick);
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
	/* PATROL belongs here with IDLE: both are disengaged states a monster has to
	be able to break out of the moment a target comes into range. Leaving it out
	would let a patrolling monster walk right past someone. */
	if (SERVER_ENTITY_ACTION::IDLE == monster.eAction ||
		SERVER_ENTITY_ACTION::PATROL == monster.eAction ||
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
		{
			/* A patrol leaves a staged route behind it. That route leads to a
			patrol point, so it is dropped here rather than walked as if it were
			a chase until the next replan tick came due. */
			monster.MovePath.clear();
			monster.iMovePathIndex = 0;
			Transition(monster, SERVER_ENTITY_ACTION::CHASE, serverTick);
		}
		if (monster.MovePath.empty() ||
			static_cast<std::int32_t>(serverTick - monster.iNextPathReplanTick) >= 0)
		{
			monster.MovePath.clear();
			monster.iMovePathIndex = 0;
			/* Each monster walks to its own point on the ring it can strike the
			target from, not to the target's exact centre, so a pack closes in
			from every side instead of converging on one tile. The ring point can
			land off the walkable floor, and an unreachable goal returns no path,
			so the target itself stays the fallback. */
			const float approachRadians = std::fmod(
				static_cast<float>(monster.iNetEntityId % 360u) *
					APPROACH_RING_GOLDEN_DEGREES, 360.f) * DEGREES_TO_RADIANS;
			const float approachRadius =
				monster.fAttackRange + monster.fCollisionRadius;
			navigation.Find_Path(monster.fPositionX, monster.fPositionZ,
				target->fPositionX + std::sin(approachRadians) * approachRadius,
				target->fPositionZ + std::cos(approachRadians) * approachRadius,
				monster.MovePath);
			if (monster.MovePath.empty())
			{
				navigation.Find_Path(monster.fPositionX, monster.fPositionZ,
					target->fPositionX, target->fPositionZ, monster.MovePath);
			}
			monster.iNextPathReplanTick = serverTick + 15u +
				static_cast<std::uint32_t>(monster.iNetEntityId % 10u);
		}
		(void)Advance_AlongPath(monster, monster.fMoveSpeed, fixedDeltaSeconds);
		return;
	}

	if (SERVER_ENTITY_ACTION::PATTERN_WINDUP == monster.eAction)
	{
		/* The telegraph is the only window where the blow may still be aimed.
		Chase used to be the sole writer of the yaw, so a monster that entered
		windup kept facing wherever it was walking and swung past a player who
		stepped around it. Once ACTIVE begins the swing is committed and the body
		stops tracking, which is what makes sidestepping it worth doing. */
		Turn_Toward(
			monster,
			target->fPositionX,
			target->fPositionZ,
			WINDUP_TURN_DEGREES_PER_SECOND * fixedDeltaSeconds);
		if (monster.fActionElapsedSeconds >=
			static_cast<float>(monster.iPatternTelegraphMs) *
				MILLISECONDS_TO_SECONDS)
		{
			Transition(monster, SERVER_ENTITY_ACTION::PATTERN_ACTIVE, serverTick);
		}
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
