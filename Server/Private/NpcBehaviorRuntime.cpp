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

	// 서버 월드 위치는 m, yaw는 degree다. 배회 목적지는 현재 위치에서 5cm 이상 떨어져야 한다.
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

	// 대기 시각을 signed tick 차이로 비교한다. 반 주기보다 짧은 대기 구간을 전제로 wrap 이후도 판정한다.
	bool Has_ReachedTick(const std::uint32_t currentTick, const std::uint32_t targetTick)
	{
		return currentTick == targetTick || static_cast<std::int32_t>(currentTick - targetTick) > 0;
	}

	// 저작 ms를 30Hz tick으로 올림하며 0ms도 최소 한 tick 기다린다. 예약값 0을 건너뛰어 1부터 순환한다.
	std::uint32_t Add_Ticks(const std::uint32_t startTick, const std::uint32_t durationMs)
	{
		const std::uint64_t elapsedTicks =
		    (std::max)(std::uint64_t{1u}, (static_cast<std::uint64_t>(durationMs) * 30u + 999u) / 1000u);
		constexpr std::uint64_t CARDINALITY =
		    static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)());
		const std::uint32_t normalizedStart = 0u == startTick ? 1u : startTick;
		return static_cast<std::uint32_t>(
		    ((static_cast<std::uint64_t>(normalizedStart - 1u) + elapsedTicks) % CARDINALITY) + 1u);
	}

	// 개체가 소유한 xorshift 상태를 제자리에서 진행한다. 0 seed를 1로 바꾸며 암호용 난수가 아니다.
	std::uint32_t Next_Random(std::uint32_t& state)
	{
		if (0u == state)
			state = 1u;
		state ^= state << 13u;
		state ^= state >> 17u;
		state ^= state << 5u;
		return state;
	}

	// 양끝을 포함하는 정수 범위를 나머지 연산으로 고른다. 범위가 접히면 난수 상태를 진행하지 않고 minimum을 쓴다.
	std::uint32_t Random_Range(SERVER_NPC_BEHAVIOR_STATE& state,
	                           const std::uint32_t minimum,
	                           const std::uint32_t maximum)
	{
		if (maximum <= minimum)
			return minimum;
		const std::uint64_t span = static_cast<std::uint64_t>(maximum) - minimum + 1u;
		return minimum + static_cast<std::uint32_t>(
		                     static_cast<std::uint64_t>(Next_Random(state.iRandomState)) % span);
	}

	// 배회 각도/반경 계산에 쓰는 실수 표본이다. 난수 상태는 entity.NpcBehavior에 남는다.
	float Random_Unit(SERVER_NPC_BEHAVIOR_STATE& state)
	{
		return static_cast<float>(Next_Random(state.iRandomState)) /
		       static_cast<float>((std::numeric_limits<std::uint32_t>::max)());
	}

	// Client가 표현할 논리 action ID와 시작 tick을 복사하고 서버 action을 바꾼다. clip을 직접 재생하지 않는다.
	void Set_Action(SERVER_WORLD_ENTITY& entity,
	                const char* actionId,
	                const std::uint32_t updateTick,
	                const SERVER_ENTITY_ACTION action)
	{
		entity.strActionId = actionId;
		entity.iActionStartTick = 0u == updateTick ? 1u : updateTick;
		entity.eAction = action;
	}

	// 차용한 시선 대상의 현재 월드 XZ를 향하도록 yaw를 바꾼다. 대상 없음/거의 같은 위치면 기존 방향을 유지한다.
	void Face_Target(SERVER_WORLD_ENTITY& entity, const SERVER_WORLD_ENTITY* target)
	{
		if (nullptr == target)
			return;
		const float deltaX = target->fPositionX - entity.fPositionX;
		const float deltaZ = target->fPositionZ - entity.fPositionZ;
		if (deltaX * deltaX + deltaZ * deltaZ <= 0.000001f)
			return;
		entity.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
	}

	// 순차 정책은 현재 index, 가중 정책은 누적 weight 구간을 사용한다. 빈 목록은 size를 실패 표식으로 반환한다.
	// 양의 weight/개수 상한은 저작 로드 검증을 전제로 하며 여기서 총합 0을 다시 검사하지 않는다.
	std::size_t Select_Action(const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor,
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

	// 추가 대기와 무작위 idle 대기를 합쳐 종료 tick을 예약한다. 경로가 끝난 ONCE는 COMPLETE로 남긴다.
	void Begin_Idle(const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor,
	                const std::uint32_t extraWaitMs,
	                const std::uint32_t updateTick,
	                SERVER_WORLD_ENTITY& entity)
	{
		SERVER_NPC_BEHAVIOR_STATE& state = entity.NpcBehavior;
		const std::uint32_t randomWait = Random_Range(state, descriptor.iIdleMinMs, descriptor.iIdleMaxMs);
		const std::uint64_t totalWait = static_cast<std::uint64_t>(extraWaitMs) + randomWait;
		state.ePhase = state.bRouteComplete ? SERVER_NPC_BEHAVIOR_PHASE::COMPLETE
		                                    : SERVER_NPC_BEHAVIOR_PHASE::IDLE_WAIT;
		state.iWaitUntilTick = Add_Ticks(
		    updateTick,
		    static_cast<std::uint32_t>((std::min)(totalWait, static_cast<std::uint64_t>(1200000u))));
		Set_Action(entity, CNpcBehaviorRuntime::IDLE_ACTION_ID, updateTick, SERVER_ENTITY_ACTION::IDLE);
	}

	// 도착한 waypoint 다음 index/왕복 방향을 정한다. ONCE의 마지막 도착은 재출발 대신 route 완료로 기록한다.
	void Advance_Route(const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor, SERVER_NPC_BEHAVIOR_STATE& state)
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

	// 도착 뒤 또는 제자리 NPC가 논리 action을 시작한다. action이 없으면 false, 있으면 기간과 복제할 action을 기록한다.
	bool Begin_Action(const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor,
	                  const std::uint32_t updateTick,
	                  SERVER_WORLD_ENTITY& entity)
	{
		SERVER_NPC_BEHAVIOR_STATE& state = entity.NpcBehavior;
		const std::size_t selected = Select_Action(descriptor, state);
		if (selected >= descriptor.Actions.size())
			return false;
		state.iActionIndex = selected;
		state.ePhase = SERVER_NPC_BEHAVIOR_PHASE::AMBIENT_ACTION;
		state.iWaitUntilTick = Add_Ticks(updateTick, descriptor.Actions[selected].iDurationMs);
		Set_Action(entity,
		           descriptor.Actions[selected].strActionId.c_str(),
		           updateTick,
		           SERVER_ENTITY_ACTION::IDLE);
		return true;
	}

	// 성공한 지역 경로 vector의 소유권을 entity로 옮기고 해당 navigation revision과 이동 action을 함께 기록한다.
	void Commit_MovePath(std::vector<SERVER_NAV_POINT>&& path,
	                     const CServerNavigation& navigation,
	                     const std::uint32_t updateTick,
	                     SERVER_WORLD_ENTITY& entity)
	{
		entity.MovePath = std::move(path);
		entity.iMovePathIndex = 0u;
		entity.NpcBehavior.ePhase = SERVER_NPC_BEHAVIOR_PHASE::MOVING;
		entity.NpcBehavior.iNavigationRevision = navigation.Get_Revision();
		Set_Action(entity, CNpcBehaviorRuntime::WALK_ACTION_ID, updateTick, SERVER_ENTITY_ACTION::CHASE);
	}

	// 순찰 목적지까지 지역 경로를 만든 뒤에만 entity에 넘긴다. Find_Path 실패는 기존 entity 경로를 바꾸지 않는다.
	bool Begin_Path(const float goalX,
	                const float goalZ,
	                const CServerNavigation& navigation,
	                const std::uint32_t updateTick,
	                SERVER_WORLD_ENTITY& entity)
	{
		std::vector<SERVER_NAV_POINT> path;
		if (!navigation
		         .Find_Path(entity.fPositionX, entity.fPositionZ, goalX, goalZ, path, entity.fPositionY))
		{
			return false;
		}
		Commit_MovePath(std::move(path), navigation, updateTick, entity);
		return true;
	}

	// 배회 후보를 navigation에 투영한 뒤 목적지와 경로의 모든 점이 spawn 반경 안인지 확인한다.
	// false라도 outPath에 탐색 결과가 남을 수 있어 호출자는 성공 때만 실행 경로로 채택한다.
	bool Try_BuildWanderPath(const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor,
	                         const CServerNavigation& navigation,
	                         const float spawnX,
	                         const float spawnZ,
	                         const float currentX,
	                         const float currentZ,
	                         const float currentY,
	                         const float candidateX,
	                         const float candidateZ,
	                         std::vector<SERVER_NAV_POINT>& outPath)
	{
		SERVER_NAV_POINT projected{};
		if (!navigation.Project_Point(candidateX, candidateZ, projected, currentY))
			return false;
		const float fromSpawnX = projected.x - spawnX;
		const float fromSpawnZ = projected.z - spawnZ;
		const float radiusSquared = descriptor.fWanderRadius * descriptor.fWanderRadius;
		if (fromSpawnX * fromSpawnX + fromSpawnZ * fromSpawnZ > radiusSquared + ARRIVAL_EPSILON)
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
		if (!navigation.Find_Path(currentX, currentZ, projected.x, projected.z, outPath, currentY) ||
		    outPath.empty())
		{
			return false;
		}
		return std::all_of(outPath.begin(),
		                   outPath.end(),
		                   [spawnX, spawnZ, radiusSquared](const SERVER_NAV_POINT& point)
		                   {
			                   const float deltaX = point.x - spawnX;
			                   const float deltaZ = point.z - spawnZ;
			                   return deltaX * deltaX + deltaZ * deltaZ <= radiusSquared + ARRIVAL_EPSILON;
		                   });
	}

	// spawn 중심 원판에서 sqrt 반경으로 후보를 뽑아 최대 16번 경로를 시도한다. 실패해도 소비한 난수 상태는 복원하지 않는다.
	bool Begin_Wander_Path(const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor,
	                       const CServerNavigation& navigation,
	                       const std::uint32_t updateTick,
	                       SERVER_WORLD_ENTITY& entity)
	{
		std::vector<SERVER_NAV_POINT> path;
		for (std::uint32_t attempt = 0u; attempt < 16u; ++attempt)
		{
			const float angle = Random_Unit(entity.NpcBehavior) * 2.f * PI;
			const float radius = std::sqrt(Random_Unit(entity.NpcBehavior)) * descriptor.fWanderRadius;
			const float candidateX = entity.fSpawnPositionX + std::sin(angle) * radius;
			const float candidateZ = entity.fSpawnPositionZ + std::cos(angle) * radius;
			if (!Try_BuildWanderPath(descriptor,
			                         navigation,
			                         entity.fSpawnPositionX,
			                         entity.fSpawnPositionZ,
			                         entity.fPositionX,
			                         entity.fPositionZ,
			                         entity.fPositionY,
			                         candidateX,
			                         candidateZ,
			                         path))
			{
				continue;
			}
			Commit_MovePath(std::move(path), navigation, updateTick, entity);
			return true;
		}
		// 무작위 후보는 배회 경로의 변화를 위한 시도다. 모두 실패하면 navigation의 반경 제한 BFS로 도달 가능한 목적지를 찾는다.
		// 무작위 표본에 안 잡혔다는 이유만으로 배회 불가능을 확정하지 않는다.
		if (!navigation.Find_PathToReachablePointWithinRadius(entity.fPositionX,
		                                                      entity.fPositionZ,
		                                                      entity.fSpawnPositionX,
		                                                      entity.fSpawnPositionZ,
		                                                      descriptor.fWanderRadius,
		                                                      WANDER_DISPLACEMENT_EPSILON,
		                                                      path,
		                                                      entity.fPositionY))
		{
			return false;
		}
		Commit_MovePath(std::move(path), navigation, updateTick, entity);
		return true;
	}

	// 속도(m/s)와 이번 초 단위 delta로 이동 예산을 만들고 여러 경로 점을 차례로 소비한다.
	// 각 이동은 몸체 충돌 해결 뒤 navigation 지면을 확인한다. INVALID/BLOCKED 전에도 앞선 이동이나 yaw 변경은 남을 수 있다.
	PATH_ADVANCE_RESULT Advance_Path(const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor,
	                                 const CServerNavigation& navigation,
	                                 const CServerCollisionSystem& collision,
	                                 const float fixedDeltaSeconds,
	                                 SERVER_WORLD_ENTITY& entity)
	{
		using namespace LostArk::Shared::WorldCollision;
		float remainingDistance = descriptor.fMoveSpeed * fixedDeltaSeconds;
		while (remainingDistance > 0.f && entity.iMovePathIndex < entity.MovePath.size())
		{
			const SERVER_NAV_POINT& target = entity.MovePath[entity.iMovePathIndex];
			const float startX = entity.fPositionX;
			const float startY = entity.fPositionY;
			const float startZ = entity.fPositionZ;
			const float deltaX = target.x - entity.fPositionX;
			const float deltaZ = target.z - entity.fPositionZ;
			const float distance = std::hypot(deltaX, deltaZ);
			if (distance > 0.0001f)
				entity.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
			const bool canReachTarget = distance <= 0.0001f || remainingDistance >= distance;
			const float ratio =
			    distance <= 0.0001f ? 1.f : (canReachTarget ? 1.f : remainingDistance / distance);
			const float proposedX = entity.fPositionX + deltaX * ratio;
			const float proposedY = entity.fPositionY + (target.y - entity.fPositionY) * ratio;
			const float proposedZ = entity.fPositionZ + deltaZ * ratio;
			float resolvedX = entity.fPositionX;
			float resolvedY = entity.fPositionY;
			float resolvedZ = entity.fPositionZ;
			bool wasBlocked = false;
			if (!collision.Resolve_CircleMove(entity.fPositionX,
			                                  entity.fPositionY,
			                                  entity.fPositionZ,
			                                  proposedX,
			                                  proposedY,
			                                  proposedZ,
			                                  PLAYER_HALF_EXTENT_X,
			                                  PLAYER_HALF_EXTENT_Y,
			                                  PLAYER_CENTER_OFFSET_Y,
			                                  resolvedX,
			                                  resolvedY,
			                                  resolvedZ,
			                                  wasBlocked,
			                                  entity.iNetEntityId))
			{
				return PATH_ADVANCE_RESULT::INVALID;
			}
			SERVER_NAV_POINT sampled{};
			if (!navigation.Resolve_TraversalStep(entity.fPositionX,
			                                      entity.fPositionZ,
			                                      resolvedX,
			                                      resolvedZ,
			                                      sampled,
			                                      entity.fPositionY))
			{
				return PATH_ADVANCE_RESULT::BLOCKED;
			}
			entity.fPositionX = resolvedX;
			entity.fPositionY = sampled.y;
			entity.fPositionZ = resolvedZ;
			// 배회 반경을 벗어난 마지막 이동만 직전 XYZ로 되돌린다. 이번 호출에서 이미 지나온 경로 점 전체를 rollback하지 않는다.
			if (NPC_BEHAVIOR_MODE::WANDER == descriptor.eMode)
			{
				const float spawnDeltaX = entity.fPositionX - entity.fSpawnPositionX;
				const float spawnDeltaZ = entity.fPositionZ - entity.fSpawnPositionZ;
				if (spawnDeltaX * spawnDeltaX + spawnDeltaZ * spawnDeltaZ >
				    descriptor.fWanderRadius * descriptor.fWanderRadius + ARRIVAL_EPSILON)
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
			    std::hypot(resolvedX - proposedX, resolvedZ - proposedZ) <= ARRIVAL_EPSILON;
			if (canReachTarget && reachedProposed)
			{
				remainingDistance -= distance;
				++entity.iMovePathIndex;
				continue;
			}
			// 몸체가 미끄러져 요청 지점에 도달하지 못하면 이번 tick의 이동 예산을 끝낸다. 다음 tick도 같은 경로 점에 접근한다.
			remainingDistance = 0.f;
		}
		return entity.iMovePathIndex >= entity.MovePath.size() ? PATH_ADVANCE_RESULT::ARRIVED
		                                                       : PATH_ADVANCE_RESULT::MOVING;
	}
}

// 입장/초기화가 공유하는 이동 정의 검사다. 이미 파싱된 정의를 전제로 경로 연결과 배회 가능 지점을 확인한다.
bool LostArk::Server::CNpcBehaviorRuntime::Validate_Descriptor(const WORLD_BOOTSTRAP_PLACEMENT& placement,
                                                               const CServerNavigation& navigation,
                                                               std::string& outStatus)
{
	if (placement.eKind != WORLD_BOOTSTRAP_KIND::NPC || !placement.bHasNpcBehavior)
	{
		outStatus = "NPC behavior descriptor owner is invalid: " + placement.strPlacementId;
		return false;
	}
	const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor = placement.NpcBehavior;
	const bool ownsReservedAction =
	    std::any_of(descriptor.Actions.begin(),
	                descriptor.Actions.end(),
	                [](const WORLD_NPC_BEHAVIOR_ACTION& action)
	                {
		                return action.strActionId == CNpcBehaviorRuntime::IDLE_ACTION_ID ||
		                       action.strActionId == CNpcBehaviorRuntime::WALK_ACTION_ID;
	                });
	if (ownsReservedAction)
	{
		outStatus = "NPC behavior action uses a runtime-reserved ID: " + placement.strPlacementId;
		return false;
	}
	// 제자리 행동은 navigation을 요구하지 않는다. action weight/기간 등 모든 필드를 이 함수가 다시 검증하는 것은 아니다.
	if (NPC_BEHAVIOR_MODE::STATIONARY == descriptor.eMode)
	{
		outStatus = "NPC behavior descriptor accepted: " + placement.strPlacementId;
		return true;
	}
	if (!std::isfinite(descriptor.fMoveSpeed) || descriptor.fMoveSpeed <= 0.f ||
	    (NPC_BEHAVIOR_MODE::PATROL == descriptor.eMode && descriptor.Waypoints.size() < 2u) ||
	    (NPC_BEHAVIOR_MODE::WANDER == descriptor.eMode &&
	     (!std::isfinite(descriptor.fWanderRadius) || descriptor.fWanderRadius < 0.5f)) ||
	    (NPC_BEHAVIOR_MODE::PATROL != descriptor.eMode && NPC_BEHAVIOR_MODE::WANDER != descriptor.eMode))
	{
		outStatus = "Moving NPC behavior descriptor shape is invalid: " + placement.strPlacementId;
		return false;
	}
	SERVER_NAV_POINT spawn{};
	if (!navigation.Project_Point(placement.fPositionX, placement.fPositionZ, spawn, placement.fPositionY))
	{
		outStatus = "NPC placement is outside server navigation: " + placement.strPlacementId;
		return false;
	}
	if (NPC_BEHAVIOR_MODE::WANDER == descriptor.eMode)
	{
		std::vector<SERVER_NAV_POINT> reachablePath;
		if (!navigation.Find_PathToReachablePointWithinRadius(spawn.x,
		                                                      spawn.z,
		                                                      spawn.x,
		                                                      spawn.z,
		                                                      descriptor.fWanderRadius,
		                                                      WANDER_DISPLACEMENT_EPSILON,
		                                                      reachablePath,
		                                                      spawn.y))
		{
			outStatus = "NPC wander owns no nontrivial reachable destination: " + placement.strPlacementId;
			return false;
		}
		outStatus = "NPC behavior descriptor accepted: " + placement.strPlacementId;
		return true;
	}

	// 투영한 spawn에서 waypoint 순서로 길을 확인한다. LOOP이면 마지막에서 첫 점까지의 경로도 검사한다.
	std::vector<SERVER_NAV_POINT> segment;
	float startX = spawn.x;
	float startZ = spawn.z;
	for (const WORLD_NPC_BEHAVIOR_WAYPOINT& waypoint : descriptor.Waypoints)
	{
		if (!navigation.Is_PointWalkableExact(waypoint.fPositionX, waypoint.fPositionZ, spawn.y) ||
		    !navigation
		         .Find_Path(startX, startZ, waypoint.fPositionX, waypoint.fPositionZ, segment, spawn.y))
		{
			outStatus = "NPC patrol waypoint is unreachable: " + placement.strPlacementId + "/" +
			            waypoint.strWaypointId;
			return false;
		}
		startX = waypoint.fPositionX;
		startZ = waypoint.fPositionZ;
	}
	if (NPC_ROUTE_MODE::LOOP == descriptor.eRouteMode &&
	    !navigation.Find_Path(startX,
	                          startZ,
	                          descriptor.Waypoints.front().fPositionX,
	                          descriptor.Waypoints.front().fPositionZ,
	                          segment,
	                          spawn.y))
	{
		outStatus = "NPC patrol loop cannot return to its first waypoint: " + placement.strPlacementId;
		return false;
	}
	outStatus = "NPC behavior descriptor accepted: " + placement.strPlacementId;
	return true;
}

// GameRoom 생성 시 참조 대상을 먼저 확인하고, 이동 NPC가 있으면 navigation 로드를 요구한다.
// 시선 참조 검사는 disabled 소유자도 포함하지만 행동별 경로 검사는 enabled placement만 수행한다.
bool LostArk::Server::CNpcBehaviorRuntime::Validate_Admission(
    const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
    const CServerNavigation& navigation,
    std::string& outStatus) const
{
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement : placements)
	{
		if (!placement.bHasNpcBehavior || placement.NpcBehavior.strLookTargetPlacementId.empty())
		{
			continue;
		}
		const auto target = std::find_if(placements.begin(),
		                                 placements.end(),
		                                 [&placement](const WORLD_BOOTSTRAP_PLACEMENT& candidate)
		                                 {
			                                 return candidate.strPlacementId ==
			                                        placement.NpcBehavior.strLookTargetPlacementId;
		                                 });
		if (target == placements.end() || !target->isEnabled ||
		    target->eKind != WORLD_BOOTSTRAP_KIND::NPC ||
		    target->strPlacementId == placement.strPlacementId)
		{
			outStatus = "NPC behavior look target is unavailable: " + placement.strPlacementId;
			return false;
		}
	}
	const bool requiresNavigation =
	    std::any_of(placements.begin(),
	                placements.end(),
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

// Build_WorldEntity의 준비 중 entity를 초기화한다. 모든 명시적 false 검사 뒤에 위치와 행동 상태를 교체한다.
bool LostArk::Server::CNpcBehaviorRuntime::Initialize(const WORLD_BOOTSTRAP_PLACEMENT& placement,
                                                      const CServerNavigation& navigation,
                                                      const std::uint32_t startTick,
                                                      SERVER_WORLD_ENTITY& entity,
                                                      std::string& outStatus) const
{
	if (!Validate_Descriptor(placement, navigation, outStatus) ||
	    entity.eKind != WORLD_BOOTSTRAP_KIND::NPC || entity.strPlacementId != placement.strPlacementId)
	{
		if (outStatus.empty())
			outStatus = "NPC behavior entity identity is invalid";
		return false;
	}
	if (NPC_BEHAVIOR_MODE::STATIONARY != placement.NpcBehavior.eMode)
	{
		SERVER_NAV_POINT spawn{};
		if (!navigation.Project_Point(entity.fPositionX, entity.fPositionZ, spawn, entity.fPositionY))
		{
			outStatus = "NPC behavior spawn projection failed: " + placement.strPlacementId;
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
	// seed/단계/경로 index를 초기 상태로 되돌리고 시작 지연을 예약한다. 저작 Waypoints/Actions는 복사하지 않는다.
	entity.MovePath.clear();
	entity.iMovePathIndex = 0u;
	entity.NpcBehavior = {};
	entity.NpcBehavior.eMode = placement.NpcBehavior.eMode;
	entity.NpcBehavior.ePhase = SERVER_NPC_BEHAVIOR_PHASE::START_DELAY;
	entity.NpcBehavior.iRandomState = placement.NpcBehavior.iRandomSeed;
	entity.NpcBehavior.iWaitUntilTick = Add_Ticks(startTick, placement.NpcBehavior.iStartDelayMs);
	entity.NpcBehavior.iNavigationRevision = navigation.Get_Revision();
	entity.NpcBehavior.bInitialized = true;
	Set_Action(entity, IDLE_ACTION_ID, startTick, SERVER_ENTITY_ACTION::IDLE);
	outStatus = "NPC behavior initialized: " + placement.strPlacementId;
	return true;
}

// Room이 현재 placement와 시선 대상을 찾아 전달한다. 입력 검사를 통과한 뒤에는 실행 상태를 직접 갱신한다.
bool LostArk::Server::CNpcBehaviorRuntime::Update(const WORLD_BOOTSTRAP_PLACEMENT& placement,
                                                  const SERVER_WORLD_ENTITY* lookTarget,
                                                  const CServerNavigation& navigation,
                                                  const CServerCollisionSystem& collision,
                                                  const float fixedDeltaSeconds,
                                                  const std::uint32_t updateTick,
                                                  SERVER_WORLD_ENTITY& entity,
                                                  std::string& outStatus) const
{
	if (!entity.NpcBehavior.bInitialized || entity.eKind != WORLD_BOOTSTRAP_KIND::NPC ||
	    entity.strPlacementId != placement.strPlacementId || !placement.bHasNpcBehavior ||
	    !std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
	{
		outStatus = "NPC behavior update input is invalid: " + placement.strPlacementId;
		return false;
	}
	const WORLD_NPC_BEHAVIOR_DESCRIPTOR& descriptor = placement.NpcBehavior;
	SERVER_NPC_BEHAVIOR_STATE& state = entity.NpcBehavior;
	// 지형 revision이 바뀌면 오래된 경로를 폐기하고 같은 호출에서 새 경로를 준비할 수 있도록 대기를 만료시킨다.
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
		const PATH_ADVANCE_RESULT pathResult =
		    Advance_Path(descriptor, navigation, collision, fixedDeltaSeconds, entity);
		if (PATH_ADVANCE_RESULT::INVALID == pathResult)
		{
			outStatus = "NPC collision resolution failed: " + placement.strPlacementId;
			return false;
		}
		// 막힘은 오류 종료가 아니라 경로 폐기와 idle 재시도 예약이다. 해결된 위치가 일부 전진했어도 그대로 유지한다.
		if (PATH_ADVANCE_RESULT::BLOCKED == pathResult)
		{
			entity.MovePath.clear();
			entity.iMovePathIndex = 0u;
			Begin_Idle(descriptor, 0u, updateTick, entity);
			Face_Target(entity, lookTarget);
			outStatus = "NPC movement blocked; retry scheduled: " + placement.strPlacementId;
			return true;
		}
		if (PATH_ADVANCE_RESULT::MOVING == pathResult)
		{
			outStatus = "NPC behavior moving";
			return true;
		}
		// 도착한 waypoint의 대기/yaw를 가져오고 다음 경로 index를 먼저 정한다. 논리 action이 있으면 대기는 action 종료 뒤로 미룬다.
		entity.MovePath.clear();
		entity.iMovePathIndex = 0u;
		if (NPC_BEHAVIOR_MODE::PATROL == descriptor.eMode)
		{
			const WORLD_NPC_BEHAVIOR_WAYPOINT& arrived = descriptor.Waypoints[state.iWaypointIndex];
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

	// 논리 action 종료 뒤 순차 index를 진행하고 waypoint 대기와 action 후 대기를 합쳐 idle로 전환한다.
	if (state.ePhase == SERVER_NPC_BEHAVIOR_PHASE::AMBIENT_ACTION)
	{
		const std::uint32_t actionWait = state.iActionIndex < descriptor.Actions.size()
		                                     ? descriptor.Actions[state.iActionIndex].iWaitAfterMs
		                                     : 0u;
		if (NPC_ACTION_SELECTION::SEQUENCE == descriptor.eActionSelection && !descriptor.Actions.empty())
		{
			state.iActionIndex = (state.iActionIndex + 1u) % descriptor.Actions.size();
		}
		const std::uint32_t pending = state.iPendingWaitMs;
		state.iPendingWaitMs = 0u;
		Begin_Idle(descriptor, pending + actionWait, updateTick, entity);
		Face_Target(entity, lookTarget);
		outStatus = "NPC behavior action completed";
		return true;
	}

	// 대기 만료 후 제자리 action, 순찰 경로, 배회 경로 중 정의의 이동 방식 하나를 실행한다.
	if (NPC_BEHAVIOR_MODE::STATIONARY == descriptor.eMode)
	{
		if (!Begin_Action(descriptor, updateTick, entity))
			Begin_Idle(descriptor, 0u, updateTick, entity);
	}
	else if (NPC_BEHAVIOR_MODE::PATROL == descriptor.eMode)
	{
		if (!Begin_Path(descriptor.Waypoints[state.iWaypointIndex].fPositionX,
		                descriptor.Waypoints[state.iWaypointIndex].fPositionZ,
		                navigation,
		                updateTick,
		                entity))
		{
			outStatus = "NPC patrol path became unreachable: " + placement.strPlacementId;
			return false;
		}
	}
	else if (!Begin_Wander_Path(descriptor, navigation, updateTick, entity))
	{
		outStatus = "NPC wander destination became unavailable: " + placement.strPlacementId;
		return false;
	}
	Face_Target(entity, SERVER_NPC_BEHAVIOR_PHASE::MOVING == state.ePhase ? nullptr : lookTarget);
	outStatus = "NPC behavior advanced";
	return true;
}
