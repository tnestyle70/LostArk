#include "NpcBehaviorRuntime.h"

#include "Gameplay/WorldCollisionContract.h"
#include "ServerCollisionSystem.h"
#include "ServerNavigation.h"
#include "ServerWorldEntity.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace
{
	using namespace LostArk::Server;

	constexpr float RADIANS_TO_DEGREES = 57.29577951308232f;
	constexpr float PI = 3.14159265358979323846f;
	constexpr float WANDER_DISPLACEMENT_EPSILON = 0.05f;
	constexpr float ARRIVAL_EPSILON = 0.001f;

	enum class PATH_ADVANCE_RESULT : std::uint8_t
	{
		MOVING,
		ARRIVED,
		BLOCKED,
		INVALID
	};

	bool Has_ReachedTick(
		const std::uint32_t currentTick,
		const std::uint32_t targetTick)
	{
		return currentTick == targetTick ||
			static_cast<std::int32_t>(currentTick - targetTick) > 0;
	}

	std::uint32_t Add_Ticks(
		const std::uint32_t startTick,
		const std::uint32_t durationMs)
	{
		const std::uint64_t elapsedTicks = (std::max)(
			std::uint64_t{ 1u },
			(static_cast<std::uint64_t>(durationMs) * 30u + 999u) / 1000u);
		constexpr std::uint64_t CARDINALITY =
			static_cast<std::uint64_t>(
				(std::numeric_limits<std::uint32_t>::max)());
		const std::uint32_t normalizedStart = 0u == startTick ? 1u : startTick;
		return static_cast<std::uint32_t>(
			((static_cast<std::uint64_t>(normalizedStart - 1u) + elapsedTicks) %
				CARDINALITY) + 1u);
	}

	std::uint32_t Next_Random(std::uint32_t& state)
	{
		if (0u == state)
			state = 1u;
		state ^= state << 13u;
		state ^= state >> 17u;
		state ^= state << 5u;
		return state;
	}

	std::uint32_t Random_Range(
		SERVER_NPC_BEHAVIOR_STATE& state,
		const std::uint32_t minimum,
		const std::uint32_t maximum)
	{
		if (maximum <= minimum)
			return minimum;
		const std::uint64_t span =
			static_cast<std::uint64_t>(maximum) - minimum + 1u;
		return minimum + static_cast<std::uint32_t>(
			static_cast<std::uint64_t>(Next_Random(state.iRandomState)) % span);
	}

	float Random_Unit(SERVER_NPC_BEHAVIOR_STATE& state)
	{
		return static_cast<float>(Next_Random(state.iRandomState)) /
			static_cast<float>((std::numeric_limits<std::uint32_t>::max)());
	}

	void Set_Action(
		SERVER_WORLD_ENTITY& entity,
		const char* actionId,
		const std::uint32_t updateTick,
		const SERVER_ENTITY_ACTION action)
	{
		entity.strActionId = actionId;
		entity.iActionStartTick = 0u == updateTick ? 1u : updateTick;
		entity.eAction = action;
	}

	void Face_Target(
		SERVER_WORLD_ENTITY& entity,
		const SERVER_WORLD_ENTITY* target)
	{
		if (nullptr == target)
			return;
		const float deltaX = target->fPositionX - entity.fPositionX;
		const float deltaZ = target->fPositionZ - entity.fPositionZ;
		if (deltaX * deltaX + deltaZ * deltaZ <= 0.000001f)
			return;
		entity.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
	}

	std::size_t Select_Action(
		const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor,
		SERVER_NPC_BEHAVIOR_STATE& state)
	{
		if (descriptor.Actions.empty())
			return descriptor.Actions.size();
		if (NPC_ACTION_SELECTION::SEQUENCE == descriptor.eActionSelection)
			return state.iActionIndex % descriptor.Actions.size();

		std::uint64_t totalWeight = 0u;
		for (const WORLD_NPC_BEHAVIOR_ACTION& action : descriptor.Actions)
			totalWeight += action.iWeight;
		const std::uint64_t roll =
			static_cast<std::uint64_t>(Next_Random(state.iRandomState)) % totalWeight;
		std::uint64_t cursor = 0u;
		for (std::size_t index = 0u; index < descriptor.Actions.size(); ++index)
		{
			cursor += descriptor.Actions[index].iWeight;
			if (roll < cursor)
				return index;
		}
		return descriptor.Actions.size() - 1u;
	}

	void Begin_Idle(
		const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor,
		const std::uint32_t extraWaitMs,
		const std::uint32_t updateTick,
		SERVER_WORLD_ENTITY& entity)
	{
		SERVER_NPC_BEHAVIOR_STATE& state = entity.NpcBehavior;
		const std::uint32_t randomWait = Random_Range(
			state, descriptor.iIdleMinMs, descriptor.iIdleMaxMs);
		const std::uint64_t totalWait =
			static_cast<std::uint64_t>(extraWaitMs) + randomWait;
		state.ePhase = state.bRouteComplete ?
			SERVER_NPC_BEHAVIOR_PHASE::COMPLETE :
			SERVER_NPC_BEHAVIOR_PHASE::IDLE_WAIT;
		state.iWaitUntilTick = Add_Ticks(
			updateTick,
			static_cast<std::uint32_t>((std::min)(
				totalWait, static_cast<std::uint64_t>(1200000u))));
		Set_Action(entity, CNpcBehaviorRuntime::IDLE_ACTION_ID,
			updateTick, SERVER_ENTITY_ACTION::IDLE);
	}

	void Advance_Route(
		const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor,
		SERVER_NPC_BEHAVIOR_STATE& state)
	{
		const std::size_t count = descriptor.Waypoints.size();
		if (count < 2u)
		{
			state.bRouteComplete = true;
			return;
		}
		switch (descriptor.eRouteMode)
		{
		case NPC_ROUTE_MODE::LOOP:
			state.iWaypointIndex = (state.iWaypointIndex + 1u) % count;
			break;
		case NPC_ROUTE_MODE::PING_PONG:
			if (state.bRouteForward)
			{
				if (state.iWaypointIndex + 1u >= count)
				{
					state.bRouteForward = false;
					--state.iWaypointIndex;
				}
				else
					++state.iWaypointIndex;
			}
			else if (0u == state.iWaypointIndex)
			{
				state.bRouteForward = true;
				++state.iWaypointIndex;
			}
			else
				--state.iWaypointIndex;
			break;
		case NPC_ROUTE_MODE::ONCE:
			if (state.iWaypointIndex + 1u >= count)
				state.bRouteComplete = true;
			else
				++state.iWaypointIndex;
			break;
		default:
			state.bRouteComplete = true;
			break;
		}
	}

	bool Begin_Action(
		const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor,
		const std::uint32_t updateTick,
		SERVER_WORLD_ENTITY& entity)
	{
		SERVER_NPC_BEHAVIOR_STATE& state = entity.NpcBehavior;
		const std::size_t selected = Select_Action(descriptor, state);
		if (selected >= descriptor.Actions.size())
			return false;
		state.iActionIndex = selected;
		state.ePhase = SERVER_NPC_BEHAVIOR_PHASE::AMBIENT_ACTION;
		state.iWaitUntilTick = Add_Ticks(
			updateTick, descriptor.Actions[selected].iDurationMs);
		Set_Action(entity, descriptor.Actions[selected].strActionId.c_str(),
			updateTick, SERVER_ENTITY_ACTION::IDLE);
		return true;
	}

	void Commit_MovePath(
		std::vector<SERVER_NAV_POINT>&& path,
		const CServerNavigation& navigation,
		const std::uint32_t updateTick,
		SERVER_WORLD_ENTITY& entity)
	{
		entity.MovePath = std::move(path);
		entity.iMovePathIndex = 0u;
		entity.NpcBehavior.ePhase = SERVER_NPC_BEHAVIOR_PHASE::MOVING;
		entity.NpcBehavior.iNavigationRevision = navigation.Get_Revision();
		Set_Action(entity, CNpcBehaviorRuntime::WALK_ACTION_ID,
			updateTick, SERVER_ENTITY_ACTION::CHASE);
	}

	bool Begin_Path(
		const float goalX,
		const float goalZ,
		const CServerNavigation& navigation,
		const std::uint32_t updateTick,
		SERVER_WORLD_ENTITY& entity)
	{
		std::vector<SERVER_NAV_POINT> path;
		if (!navigation.Find_Path(
			entity.fPositionX, entity.fPositionZ,
			goalX, goalZ, path))
		{
			return false;
		}
		Commit_MovePath(std::move(path), navigation, updateTick, entity);
		return true;
	}

	bool Try_BuildWanderPath(
		const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor,
		const CServerNavigation& navigation,
		const float spawnX,
		const float spawnZ,
		const float currentX,
		const float currentZ,
		const float candidateX,
		const float candidateZ,
		std::vector<SERVER_NAV_POINT>& outPath)
	{
		SERVER_NAV_POINT projected{};
		if (!navigation.Project_Point(candidateX, candidateZ, projected))
			return false;
		const float fromSpawnX = projected.x - spawnX;
		const float fromSpawnZ = projected.z - spawnZ;
		const float radiusSquared =
			descriptor.fWanderRadius * descriptor.fWanderRadius;
		if (fromSpawnX * fromSpawnX + fromSpawnZ * fromSpawnZ >
			radiusSquared + ARRIVAL_EPSILON)
		{
			return false;
		}
		const float fromCurrentX = projected.x - currentX;
		const float fromCurrentZ = projected.z - currentZ;
		if (fromCurrentX * fromCurrentX + fromCurrentZ * fromCurrentZ <
			WANDER_DISPLACEMENT_EPSILON * WANDER_DISPLACEMENT_EPSILON)
		{
			return false;
		}
		if (!navigation.Find_Path(
			currentX, currentZ, projected.x, projected.z, outPath) ||
			outPath.empty())
		{
			return false;
		}
		return std::all_of(
			outPath.begin(), outPath.end(),
			[spawnX, spawnZ, radiusSquared](const SERVER_NAV_POINT& point)
			{
				const float deltaX = point.x - spawnX;
				const float deltaZ = point.z - spawnZ;
				return deltaX * deltaX + deltaZ * deltaZ <=
					radiusSquared + ARRIVAL_EPSILON;
			});
	}

	bool Begin_Wander_Path(
		const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor,
		const CServerNavigation& navigation,
		const std::uint32_t updateTick,
		SERVER_WORLD_ENTITY& entity)
	{
		std::vector<SERVER_NAV_POINT> path;
		for (std::uint32_t attempt = 0u; attempt < 16u; ++attempt)
		{
			const float angle = Random_Unit(entity.NpcBehavior) * 2.f * PI;
			const float radius = std::sqrt(Random_Unit(entity.NpcBehavior)) *
				descriptor.fWanderRadius;
			const float candidateX = entity.fSpawnPositionX + std::sin(angle) * radius;
			const float candidateZ = entity.fSpawnPositionZ + std::cos(angle) * radius;
			if (!Try_BuildWanderPath(
				descriptor, navigation,
				entity.fSpawnPositionX, entity.fSpawnPositionZ,
				entity.fPositionX, entity.fPositionZ,
				candidateX, candidateZ, path))
			{
				continue;
			}
			Commit_MovePath(std::move(path), navigation, updateTick, entity);
			return true;
		}
		/* Random probing is presentation variety, not the admission oracle.
		The navigation-owned bounded BFS deterministically finds any nontrivial
		destination whose whole route remains inside the authored radius. */
		if (!navigation.Find_PathToReachablePointWithinRadius(
			entity.fPositionX, entity.fPositionZ,
			entity.fSpawnPositionX, entity.fSpawnPositionZ,
			descriptor.fWanderRadius, WANDER_DISPLACEMENT_EPSILON,
			path))
		{
			return false;
		}
		Commit_MovePath(std::move(path), navigation, updateTick, entity);
		return true;
	}

	PATH_ADVANCE_RESULT Advance_Path(
		const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor,
		const CServerNavigation& navigation,
		const CServerCollisionSystem& collision,
		const float fixedDeltaSeconds,
		SERVER_WORLD_ENTITY& entity)
	{
		using namespace LostArk::Shared::WorldCollision;
		float remainingDistance = descriptor.fMoveSpeed * fixedDeltaSeconds;
		while (remainingDistance > 0.f &&
			entity.iMovePathIndex < entity.MovePath.size())
		{
			const SERVER_NAV_POINT& target =
				entity.MovePath[entity.iMovePathIndex];
			const float startX = entity.fPositionX;
			const float startY = entity.fPositionY;
			const float startZ = entity.fPositionZ;
			const float deltaX = target.x - entity.fPositionX;
			const float deltaZ = target.z - entity.fPositionZ;
			const float distance = std::hypot(deltaX, deltaZ);
			if (distance > 0.0001f)
				entity.fYawDegrees =
					std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
			const bool canReachTarget =
				distance <= 0.0001f || remainingDistance >= distance;
			const float ratio = distance <= 0.0001f ? 1.f :
				(canReachTarget ? 1.f : remainingDistance / distance);
			const float proposedX = entity.fPositionX + deltaX * ratio;
			const float proposedY = entity.fPositionY +
				(target.y - entity.fPositionY) * ratio;
			const float proposedZ = entity.fPositionZ + deltaZ * ratio;
			float resolvedX = entity.fPositionX;
			float resolvedY = entity.fPositionY;
			float resolvedZ = entity.fPositionZ;
			bool wasBlocked = false;
			if (!collision.Resolve_CircleMove(
				entity.fPositionX, entity.fPositionY, entity.fPositionZ,
				proposedX, proposedY, proposedZ,
				PLAYER_HALF_EXTENT_X, PLAYER_HALF_EXTENT_Y,
				PLAYER_CENTER_OFFSET_Y,
				resolvedX, resolvedY, resolvedZ, wasBlocked,
				entity.iNetEntityId))
			{
				return PATH_ADVANCE_RESULT::INVALID;
			}
			SERVER_NAV_POINT sampled{};
			if (!navigation.Resolve_TraversalStep(
				entity.fPositionX, entity.fPositionZ,
				resolvedX, resolvedZ, sampled))
			{
				return PATH_ADVANCE_RESULT::BLOCKED;
			}
			entity.fPositionX = resolvedX;
			entity.fPositionY = sampled.y;
			entity.fPositionZ = resolvedZ;
			if (NPC_BEHAVIOR_MODE::WANDER == descriptor.eMode)
			{
				const float spawnDeltaX =
					entity.fPositionX - entity.fSpawnPositionX;
				const float spawnDeltaZ =
					entity.fPositionZ - entity.fSpawnPositionZ;
				if (spawnDeltaX * spawnDeltaX + spawnDeltaZ * spawnDeltaZ >
					descriptor.fWanderRadius * descriptor.fWanderRadius +
					ARRIVAL_EPSILON)
				{
					entity.fPositionX = startX;
					entity.fPositionY = startY;
					entity.fPositionZ = startZ;
					return PATH_ADVANCE_RESULT::BLOCKED;
				}
			}
			if (wasBlocked)
				return PATH_ADVANCE_RESULT::BLOCKED;

			const bool reachedProposed =
				std::hypot(resolvedX - proposedX, resolvedZ - proposedZ) <=
				ARRIVAL_EPSILON;
			if (canReachTarget && reachedProposed)
			{
				remainingDistance -= distance;
				++entity.iMovePathIndex;
				continue;
			}
			/* A body slide consumes this tick's movement but does not claim the
			authored path point.  The next fixed tick approaches it again. */
			remainingDistance = 0.f;
		}
		return entity.iMovePathIndex >= entity.MovePath.size() ?
			PATH_ADVANCE_RESULT::ARRIVED : PATH_ADVANCE_RESULT::MOVING;
	}
}

bool LostArk::Server::CNpcBehaviorRuntime::Validate_Descriptor(
	const WORLD_BOOTSTRAP_PLACEMENT& placement,
	const CServerNavigation& navigation,
	std::string& outStatus)
{
	if (placement.eKind != WORLD_BOOTSTRAP_KIND::NPC ||
		!placement.bHasNpcBehavior)
	{
		outStatus = "NPC behavior descriptor owner is invalid: " +
			placement.strPlacementId;
		return false;
	}
	const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor = placement.NpcBehavior;
	const bool ownsReservedAction = std::any_of(
		descriptor.Actions.begin(), descriptor.Actions.end(),
		[](const WORLD_NPC_BEHAVIOR_ACTION& action)
		{
			return action.strActionId == CNpcBehaviorRuntime::IDLE_ACTION_ID ||
				action.strActionId == CNpcBehaviorRuntime::WALK_ACTION_ID;
		});
	if (ownsReservedAction)
	{
		outStatus = "NPC behavior action uses a runtime-reserved ID: " +
			placement.strPlacementId;
		return false;
	}
	if (NPC_BEHAVIOR_MODE::STATIONARY == descriptor.eMode)
	{
		outStatus = "NPC behavior descriptor accepted: " + placement.strPlacementId;
		return true;
	}
	if (!std::isfinite(descriptor.fMoveSpeed) ||
		descriptor.fMoveSpeed <= 0.f ||
		(NPC_BEHAVIOR_MODE::PATROL == descriptor.eMode &&
			descriptor.Waypoints.size() < 2u) ||
		(NPC_BEHAVIOR_MODE::WANDER == descriptor.eMode &&
			(!std::isfinite(descriptor.fWanderRadius) ||
			 descriptor.fWanderRadius < 0.5f)) ||
		(NPC_BEHAVIOR_MODE::PATROL != descriptor.eMode &&
		 NPC_BEHAVIOR_MODE::WANDER != descriptor.eMode))
	{
		outStatus = "Moving NPC behavior descriptor shape is invalid: " +
			placement.strPlacementId;
		return false;
	}
	SERVER_NAV_POINT spawn{};
	if (!navigation.Project_Point(
		placement.fPositionX, placement.fPositionZ, spawn))
	{
		outStatus = "NPC placement is outside server navigation: " +
			placement.strPlacementId;
		return false;
	}
	if (NPC_BEHAVIOR_MODE::WANDER == descriptor.eMode)
	{
		std::vector<SERVER_NAV_POINT> reachablePath;
		if (!navigation.Find_PathToReachablePointWithinRadius(
			spawn.x, spawn.z, spawn.x, spawn.z,
			descriptor.fWanderRadius, WANDER_DISPLACEMENT_EPSILON,
			reachablePath))
		{
			outStatus = "NPC wander owns no nontrivial reachable destination: " +
				placement.strPlacementId;
			return false;
		}
		outStatus = "NPC behavior descriptor accepted: " + placement.strPlacementId;
		return true;
	}

	std::vector<SERVER_NAV_POINT> segment;
	float startX = spawn.x;
	float startZ = spawn.z;
	for (const WORLD_NPC_BEHAVIOR_WAYPOINT& waypoint : descriptor.Waypoints)
	{
		if (!navigation.Is_PointWalkableExact(
			waypoint.fPositionX, waypoint.fPositionZ) ||
			!navigation.Find_Path(
				startX, startZ,
				waypoint.fPositionX, waypoint.fPositionZ, segment))
		{
			outStatus = "NPC patrol waypoint is unreachable: " +
				placement.strPlacementId + "/" + waypoint.strWaypointId;
			return false;
		}
		startX = waypoint.fPositionX;
		startZ = waypoint.fPositionZ;
	}
	if (NPC_ROUTE_MODE::LOOP == descriptor.eRouteMode &&
		!navigation.Find_Path(
			startX, startZ,
			descriptor.Waypoints.front().fPositionX,
			descriptor.Waypoints.front().fPositionZ,
			segment))
	{
		outStatus = "NPC patrol loop cannot return to its first waypoint: " +
			placement.strPlacementId;
		return false;
	}
	outStatus = "NPC behavior descriptor accepted: " + placement.strPlacementId;
	return true;
}

bool LostArk::Server::CNpcBehaviorRuntime::Validate_Admission(
	const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
	const CServerNavigation& navigation,
	std::string& outStatus) const
{
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement : placements)
	{
		if (!placement.bHasNpcBehavior ||
			placement.NpcBehavior.strLookTargetPlacementId.empty())
		{
			continue;
		}
		const auto target = std::find_if(
			placements.begin(), placements.end(),
			[&placement](const WORLD_BOOTSTRAP_PLACEMENT& candidate)
			{
				return candidate.strPlacementId ==
					placement.NpcBehavior.strLookTargetPlacementId;
			});
		if (target == placements.end() || !target->isEnabled ||
			target->eKind != WORLD_BOOTSTRAP_KIND::NPC ||
			target->strPlacementId == placement.strPlacementId)
		{
			outStatus = "NPC behavior look target is unavailable: " +
				placement.strPlacementId;
			return false;
		}
	}
	const bool requiresNavigation = std::any_of(
		placements.begin(), placements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.isEnabled && placement.bHasNpcBehavior &&
				NPC_BEHAVIOR_MODE::STATIONARY != placement.NpcBehavior.eMode;
		});
	if (requiresNavigation && !navigation.Is_Loaded())
	{
		outStatus = "Moving NPC behavior requires loaded server navigation";
		return false;
	}
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement : placements)
	{
		if (!placement.isEnabled || !placement.bHasNpcBehavior)
			continue;
		if (!Validate_Descriptor(placement, navigation, outStatus))
			return false;
	}
	outStatus = "NPC behavior admission accepted";
	return true;
}

bool LostArk::Server::CNpcBehaviorRuntime::Initialize(
	const WORLD_BOOTSTRAP_PLACEMENT& placement,
	const CServerNavigation& navigation,
	const std::uint32_t startTick,
	SERVER_WORLD_ENTITY& entity,
	std::string& outStatus) const
{
	if (!Validate_Descriptor(placement, navigation, outStatus) ||
		entity.eKind != WORLD_BOOTSTRAP_KIND::NPC ||
		entity.strPlacementId != placement.strPlacementId)
	{
		if (outStatus.empty())
			outStatus = "NPC behavior entity identity is invalid";
		return false;
	}
	if (NPC_BEHAVIOR_MODE::STATIONARY != placement.NpcBehavior.eMode)
	{
		SERVER_NAV_POINT spawn{};
		if (!navigation.Project_Point(
			entity.fPositionX, entity.fPositionZ, spawn))
		{
			outStatus = "NPC behavior spawn projection failed: " +
				placement.strPlacementId;
			return false;
		}
		entity.fPositionX = spawn.x;
		entity.fPositionY = spawn.y;
		entity.fPositionZ = spawn.z;
		entity.fSpawnPositionX = spawn.x;
		entity.fSpawnPositionY = spawn.y;
		entity.fSpawnPositionZ = spawn.z;
	}
	else
	{
		entity.fSpawnPositionX = entity.fPositionX;
		entity.fSpawnPositionY = entity.fPositionY;
		entity.fSpawnPositionZ = entity.fPositionZ;
	}
	entity.MovePath.clear();
	entity.iMovePathIndex = 0u;
	entity.NpcBehavior = {};
	entity.NpcBehavior.eMode = placement.NpcBehavior.eMode;
	entity.NpcBehavior.ePhase = SERVER_NPC_BEHAVIOR_PHASE::START_DELAY;
	entity.NpcBehavior.iRandomState = placement.NpcBehavior.iRandomSeed;
	entity.NpcBehavior.iWaitUntilTick = Add_Ticks(
		startTick, placement.NpcBehavior.iStartDelayMs);
	entity.NpcBehavior.iNavigationRevision = navigation.Get_Revision();
	entity.NpcBehavior.bInitialized = true;
	Set_Action(entity, IDLE_ACTION_ID, startTick, SERVER_ENTITY_ACTION::IDLE);
	outStatus = "NPC behavior initialized: " + placement.strPlacementId;
	return true;
}

bool LostArk::Server::CNpcBehaviorRuntime::Update(
	const WORLD_BOOTSTRAP_PLACEMENT& placement,
	const SERVER_WORLD_ENTITY* lookTarget,
	const CServerNavigation& navigation,
	const CServerCollisionSystem& collision,
	const float fixedDeltaSeconds,
	const std::uint32_t updateTick,
	SERVER_WORLD_ENTITY& entity,
	std::string& outStatus) const
{
	if (!entity.NpcBehavior.bInitialized ||
		entity.eKind != WORLD_BOOTSTRAP_KIND::NPC ||
		entity.strPlacementId != placement.strPlacementId ||
		!placement.bHasNpcBehavior ||
		!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
	{
		outStatus = "NPC behavior update input is invalid: " +
			placement.strPlacementId;
		return false;
	}
	const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor = placement.NpcBehavior;
	SERVER_NPC_BEHAVIOR_STATE& state = entity.NpcBehavior;
	if (state.ePhase == SERVER_NPC_BEHAVIOR_PHASE::MOVING &&
		state.iNavigationRevision != navigation.Get_Revision())
	{
		entity.MovePath.clear();
		entity.iMovePathIndex = 0u;
		state.ePhase = SERVER_NPC_BEHAVIOR_PHASE::IDLE_WAIT;
		state.iWaitUntilTick = updateTick;
	}

	if (state.ePhase == SERVER_NPC_BEHAVIOR_PHASE::MOVING)
	{
		const PATH_ADVANCE_RESULT pathResult = Advance_Path(
			descriptor, navigation, collision, fixedDeltaSeconds, entity);
		if (PATH_ADVANCE_RESULT::INVALID == pathResult)
		{
			outStatus = "NPC collision resolution failed: " +
				placement.strPlacementId;
			return false;
		}
		if (PATH_ADVANCE_RESULT::BLOCKED == pathResult)
		{
			entity.MovePath.clear();
			entity.iMovePathIndex = 0u;
			Begin_Idle(descriptor, 0u, updateTick, entity);
			Face_Target(entity, lookTarget);
			outStatus = "NPC movement blocked; retry scheduled: " +
				placement.strPlacementId;
			return true;
		}
		if (PATH_ADVANCE_RESULT::MOVING == pathResult)
		{
			outStatus = "NPC behavior moving";
			return true;
		}
		entity.MovePath.clear();
		entity.iMovePathIndex = 0u;
		if (NPC_BEHAVIOR_MODE::PATROL == descriptor.eMode)
		{
			const WORLD_NPC_BEHAVIOR_WAYPOINT& arrived =
				descriptor.Waypoints[state.iWaypointIndex];
			state.iPendingWaitMs = arrived.iWaitMs;
			if (arrived.bHasLookYaw)
				entity.fYawDegrees = arrived.fLookYawDegrees;
			Advance_Route(descriptor, state);
		}
		else
			state.iPendingWaitMs = 0u;
		if (!Begin_Action(descriptor, updateTick, entity))
		{
			const std::uint32_t pending = state.iPendingWaitMs;
			state.iPendingWaitMs = 0u;
			Begin_Idle(descriptor, pending, updateTick, entity);
		}
		Face_Target(entity, lookTarget);
		outStatus = "NPC behavior arrived";
		return true;
	}

	if (state.ePhase == SERVER_NPC_BEHAVIOR_PHASE::COMPLETE)
	{
		Face_Target(entity, lookTarget);
		outStatus = "NPC behavior route complete";
		return true;
	}

	if (!Has_ReachedTick(updateTick, state.iWaitUntilTick))
	{
		Face_Target(entity, lookTarget);
		outStatus = "NPC behavior waiting";
		return true;
	}

	if (state.ePhase == SERVER_NPC_BEHAVIOR_PHASE::AMBIENT_ACTION)
	{
		const std::uint32_t actionWait = state.iActionIndex <
			descriptor.Actions.size() ?
			descriptor.Actions[state.iActionIndex].iWaitAfterMs : 0u;
		if (NPC_ACTION_SELECTION::SEQUENCE == descriptor.eActionSelection &&
			!descriptor.Actions.empty())
		{
			state.iActionIndex = (state.iActionIndex + 1u) %
				descriptor.Actions.size();
		}
		const std::uint32_t pending = state.iPendingWaitMs;
		state.iPendingWaitMs = 0u;
		Begin_Idle(descriptor, pending + actionWait, updateTick, entity);
		Face_Target(entity, lookTarget);
		outStatus = "NPC behavior action completed";
		return true;
	}

	if (NPC_BEHAVIOR_MODE::STATIONARY == descriptor.eMode)
	{
		if (!Begin_Action(descriptor, updateTick, entity))
			Begin_Idle(descriptor, 0u, updateTick, entity);
	}
	else if (NPC_BEHAVIOR_MODE::PATROL == descriptor.eMode)
	{
		if (!Begin_Path(
			descriptor.Waypoints[state.iWaypointIndex].fPositionX,
			descriptor.Waypoints[state.iWaypointIndex].fPositionZ,
			navigation, updateTick, entity))
		{
			outStatus = "NPC patrol path became unreachable: " +
				placement.strPlacementId;
			return false;
		}
	}
	else if (!Begin_Wander_Path(
		descriptor, navigation, updateTick, entity))
	{
		outStatus = "NPC wander destination became unavailable: " +
			placement.strPlacementId;
		return false;
	}
	Face_Target(entity,
		SERVER_NPC_BEHAVIOR_PHASE::MOVING == state.ePhase ? nullptr : lookTarget);
	outStatus = "NPC behavior advanced";
	return true;
}
