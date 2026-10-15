#include "ServerCollisionSystem.h"

#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <set>
#include <utility>

namespace
{
	using namespace LostArk::Shared::WorldCollision;
	using namespace LostArk::Server;

	constexpr float DEGREES_TO_RADIANS = 0.0174532925f;
	constexpr float SWEEP_EPSILON = 0.000001f;

	// box 중심과 yaw를 제거한 XZ 좌표다. 아래 WORLD_POINT는 월드 XZ 좌표를 표현한다.
	struct LOCAL_POINT final
	{
		float x = 0.f;
		float z = 0.f;
	};

	struct WORLD_POINT final
	{
		float x = 0.f;
		float z = 0.f;
	};

	// 월드 XZ의 중심, 단위 오른쪽/전방 축, 각 축의 반길이로 회전 사각형을 표현한다.
	struct OBB_2D final
	{
		WORLD_POINT center{};
		WORLD_POINT right{};
		WORLD_POINT forward{};
		float halfRight = 0.f;
		float halfForward = 0.f;
	};

	// placement의 yaw를 역적용해 월드 XZ를 box 로컬로 바꾼다. Y는 별도의 높이 검사에서 처리한다.
	LOCAL_POINT To_BoxLocal(
		const float worldX,
		const float worldZ,
		const WORLD_BOOTSTRAP_PLACEMENT& box)
	{
		const float yaw = box.fYawDegrees * DEGREES_TO_RADIANS;
		const float cosine = std::cos(yaw);
		const float sine = std::sin(yaw);
		const float deltaX = worldX - box.fPositionX;
		const float deltaZ = worldZ - box.fPositionZ;
		return
		{
			cosine * deltaX - sine * deltaZ,
			sine * deltaX + cosine * deltaZ
		};
	}

	// start + t*delta가 [-extent, extent] 안에 있는 t 구간을 기존 enter/exit와 교차한다.
	// 거의 정지한 축은 시작점의 포함 여부만 검사한다. false여도 누적 구간 출력은 이미 좁혀졌을 수 있다.
	bool Update_Slab(
		const float start,
		const float delta,
		const float extent,
		float& inOutEnter,
		float& inOutExit)
	{
		if (std::abs(delta) <= SWEEP_EPSILON)
			return std::abs(start) <= extent;
		float first = (-extent - start) / delta;
		float second = (extent - start) / delta;
		if (first > second)
			std::swap(first, second);
		inOutEnter = (std::max)(inOutEnter, first);
		inOutExit = (std::min)(inOutExit, second);
		return inOutEnter <= inOutExit;
	}

	// XZ 벡터의 내적이다. 투영 거리와 축 방향 비교에 사용한다.
	float Dot(const WORLD_POINT& left, const WORLD_POINT& right)
	{
		return left.x * right.x + left.z * right.z;
	}

	// 두 월드 XZ 위치의 차이를 만들어 투영/다각형 변 계산에 전달한다.
	WORLD_POINT Subtract(const WORLD_POINT& left, const WORLD_POINT& right)
	{
		return { left.x - right.x, left.z - right.z };
	}

	// 벽 placement의 중심, yaw degree, 반길이를 XZ OBB 값으로 변환한다.
	OBB_2D To_Obb(const WORLD_BOOTSTRAP_PLACEMENT& box)
	{
		const float yaw = box.fYawDegrees * DEGREES_TO_RADIANS;
		const float cosine = std::cos(yaw);
		const float sine = std::sin(yaw);
		return
		{
			{ box.fPositionX, box.fPositionZ },
			{ cosine, -sine },
			{ sine, cosine },
			box.fHalfExtentX,
			box.fHalfExtentZ
		};
	}

	// 공격 도형이 계산한 중심과 반길이로 OBB를 만든다. 도형별 원점→중심 이동은 호출 측에서 한다.
	OBB_2D Make_AttackObb(
		const float centerX,
		const float centerZ,
		const float yawDegrees,
		const float halfRight,
		const float halfForward)
	{
		const float yaw = yawDegrees * DEGREES_TO_RADIANS;
		const float cosine = std::cos(yaw);
		const float sine = std::sin(yaw);
		return
		{
			{ centerX, centerZ },
			{ cosine, -sine },
			{ sine, cosine },
			halfRight,
			halfForward
		};
	}

	// OBB의 오른쪽/전방 반길이를 더하고 빼서 경계 순서의 네 꼭짓점을 만든다.
	std::array<WORLD_POINT, 4u> Get_Corners(const OBB_2D& box)
	{
		const auto corner = [&box](
			const float rightSign, const float forwardSign)
		{
			return WORLD_POINT
			{
				box.center.x + box.right.x * box.halfRight * rightSign +
					box.forward.x * box.halfForward * forwardSign,
				box.center.z + box.right.z * box.halfRight * rightSign +
					box.forward.z * box.halfForward * forwardSign
			};
		};
		return
		{
			corner(-1.f, -1.f),
			corner(1.f, -1.f),
			corner(1.f, 1.f),
			corner(-1.f, 1.f)
		};
	}

	// 두 OBB를 하나의 축에 투영한다. 중심 거리와 두 투영 반지름 합으로 겹침 여부를 비교한다.
	bool Overlaps_OnAxis(
		const OBB_2D& left,
		const OBB_2D& right,
		const WORLD_POINT& axis)
	{
		const float centerDistance = std::abs(Dot(
			Subtract(right.center, left.center), axis));
		const float leftRadius =
			left.halfRight * std::abs(Dot(left.right, axis)) +
			left.halfForward * std::abs(Dot(left.forward, axis));
		const float rightRadius =
			right.halfRight * std::abs(Dot(right.right, axis)) +
			right.halfForward * std::abs(Dot(right.forward, axis));
		return centerDistance <= leftRadius + rightRadius + CONTACT_MARGIN;
	}

	// 각 OBB의 두 축, 총 네 분리축을 검사한다. 어느 한 축에서 떨어지면 XZ 충돌이 없다.
	bool Obbs_Intersect(const OBB_2D& left, const OBB_2D& right)
	{
		return Overlaps_OnAxis(left, right, left.right) &&
			Overlaps_OnAxis(left, right, left.forward) &&
			Overlaps_OnAxis(left, right, right.right) &&
			Overlaps_OnAxis(left, right, right.forward);
	}

	// box 로컬의 가장 가까운 점과 원 중심의 거리 제곱을 반지름으로 비교한다.
	bool Circle_IntersectsObb(
		const float centerX,
		const float centerZ,
		const float radius,
		const WORLD_BOOTSTRAP_PLACEMENT& box)
	{
		const LOCAL_POINT local = To_BoxLocal(centerX, centerZ, box);
		const float closestX = (std::clamp)(
			local.x, -box.fHalfExtentX, box.fHalfExtentX);
		const float closestZ = (std::clamp)(
			local.z, -box.fHalfExtentZ, box.fHalfExtentZ);
		const float deltaX = local.x - closestX;
		const float deltaZ = local.z - closestZ;
		return deltaX * deltaX + deltaZ * deltaZ <=
			(radius + CONTACT_MARGIN) * (radius + CONTACT_MARGIN);
	}

	// 외원과 만나는지 먼저 검사하고 가장 먼 꼭짓점이 내원 경계에 닿는지 확인한다.
	// 가운데 빈 영역에만 들어간 box를 제외하기 위한 추가 조건이다.
	bool Ring_IntersectsObb(
		const float centerX,
		const float centerZ,
		const float innerRadius,
		const float outerRadius,
		const WORLD_BOOTSTRAP_PLACEMENT& box)
	{
		if (!Circle_IntersectsObb(centerX, centerZ, outerRadius, box))
			return false;
		const auto corners = Get_Corners(To_Obb(box));
		float farthestSquared = 0.f;
		for (const WORLD_POINT& corner : corners)
		{
			const float deltaX = corner.x - centerX;
			const float deltaZ = corner.z - centerZ;
			farthestSquared = (std::max)(
				farthestSquared, deltaX * deltaX + deltaZ * deltaZ);
		}
		return farthestSquared + CONTACT_MARGIN >= innerRadius * innerRadius;
	}

	// 비어 있지 않은 다각형의 축 투영 최소/최대를 계산한다. 호출자는 원뿔 근사와 box의 꼭짓점을 넘긴다.
	void Project_Polygon(
		const std::vector<WORLD_POINT>& polygon,
		const WORLD_POINT& axis,
		float& outMinimum,
		float& outMaximum)
	{
		outMinimum = Dot(polygon.front(), axis);
		outMaximum = outMinimum;
		for (std::size_t index = 1u; index < polygon.size(); ++index)
		{
			const float projection = Dot(polygon[index], axis);
			outMinimum = (std::min)(outMinimum, projection);
			outMaximum = (std::max)(outMaximum, projection);
		}
	}

	// 양쪽 볼록 다각형의 변 법선에서 분리축을 찾는다. 퇴화한 짧은 변은 건너뛴다.
	bool Convex_PolygonsIntersect(
		const std::vector<WORLD_POINT>& left,
		const std::vector<WORLD_POINT>& right)
	{
		const auto separatedOnAnyEdge = [&left, &right](
			const std::vector<WORLD_POINT>& polygon)
		{
			for (std::size_t index = 0u; index < polygon.size(); ++index)
			{
				const WORLD_POINT edge = Subtract(
					polygon[(index + 1u) % polygon.size()], polygon[index]);
				const WORLD_POINT axis{ -edge.z, edge.x };
				const float lengthSquared = Dot(axis, axis);
				if (lengthSquared <= SWEEP_EPSILON)
					continue;
				float leftMinimum = 0.f;
				float leftMaximum = 0.f;
				float rightMinimum = 0.f;
				float rightMaximum = 0.f;
				Project_Polygon(left, axis, leftMinimum, leftMaximum);
				Project_Polygon(right, axis, rightMinimum, rightMaximum);
				if (leftMaximum + CONTACT_MARGIN < rightMinimum ||
					rightMaximum + CONTACT_MARGIN < leftMinimum)
				{
					return true;
				}
			}
			return false;
		};
		return !separatedOnAnyEdge(left) && !separatedOnAnyEdge(right);
	}

	// 원뿔의 XZ 부채꼴 호를 24개 선분으로 근사한 뒤 box와 볼록 다각형 교차를 검사한다.
	// 이는 연속 곡선의 정확한 교차 계산이 아니라 고정 분할 근사다.
	bool Cone_IntersectsObb(
		const float originX,
		const float originZ,
		const float yawDegrees,
		const float angleDegrees,
		const float length,
		const WORLD_BOOTSTRAP_PLACEMENT& box)
	{
		constexpr std::size_t ARC_SEGMENTS = 24u;
		std::vector<WORLD_POINT> cone;
		cone.reserve(ARC_SEGMENTS + 2u);
		cone.push_back({ originX, originZ });
		const float halfAngle = angleDegrees * 0.5f;
		for (std::size_t index = 0u; index <= ARC_SEGMENTS; ++index)
		{
			const float ratio = static_cast<float>(index) /
				static_cast<float>(ARC_SEGMENTS);
			const float angle = (yawDegrees - halfAngle +
				angleDegrees * ratio) * DEGREES_TO_RADIANS;
			cone.push_back(
			{
				originX + std::sin(angle) * length,
				originZ + std::cos(angle) * length
			});
		}
		const auto boxCorners = Get_Corners(To_Obb(box));
		const std::vector<WORLD_POINT> boxPolygon(
			boxCorners.begin(), boxCorners.end());
		return Convex_PolygonsIntersect(cone, boxPolygon);
	}
}

// GameRoom 초기화가 활성 충돌 상자를 검증한다. 검증 실패는 실제 멤버 교체 전에 반환한다.
// 이후 flag 준비의 할당 예외까지 rollback하는 transaction은 이 함수에 없다.
bool LostArk::Server::CServerCollisionSystem::Initialize(
	const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
	std::string& outStatus)
{
	std::vector<WORLD_BOOTSTRAP_PLACEMENT> staged;
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement : placements)
	{
		if (WORLD_BOOTSTRAP_KIND::COLLISION_BOX != placement.eKind ||
			!placement.isEnabled)
		{
			continue;
		}
		if (placement.strPlacementId.empty() ||
			!placement.strArchetypeId.empty() ||
			!placement.strEncounterId.empty() ||
			!placement.TriggerActions.empty() ||
			!std::isfinite(placement.fPositionX) ||
			!std::isfinite(placement.fPositionY) ||
			!std::isfinite(placement.fPositionZ) ||
			!std::isfinite(placement.fYawDegrees) ||
			!std::isfinite(placement.fHalfExtentX) ||
			!std::isfinite(placement.fHalfExtentY) ||
			!std::isfinite(placement.fHalfExtentZ) ||
			std::abs(placement.fPositionX) > 100000.f ||
			std::abs(placement.fPositionY) > 100000.f ||
			std::abs(placement.fPositionZ) > 100000.f ||
			placement.fHalfExtentX <= 0.f ||
			placement.fHalfExtentX > 1000.f ||
			placement.fHalfExtentY <= 0.f ||
			placement.fHalfExtentY > 1000.f ||
			placement.fHalfExtentZ <= 0.f ||
			placement.fHalfExtentZ > 1000.f)
		{
			outStatus = "Invalid collision box: " + placement.strPlacementId;
			return false;
		}
		staged.push_back(placement);
	}
	m_CollisionBoxes = std::move(staged);
	m_PlayerBlocking.assign(m_CollisionBoxes.size(), true);
	m_ImpactReceiverEnabled.clear();
	m_ImpactReceiverEnabled.reserve(m_CollisionBoxes.size());
	for (const WORLD_BOOTSTRAP_PLACEMENT& box : m_CollisionBoxes)
	{
		m_ImpactReceiverEnabled.push_back(
			Is_ImpactReceiverPlacementId(box.strPlacementId));
	}
	m_iRevision = 1u;
	outStatus = "Initialized server collision boxes: " +
		std::to_string(m_CollisionBoxes.size());
	return true;
}

// GameRoom의 spawn 초기 검사다. 정적 box만 읽으며 동적 몸체 및 navigation 승인과는 별도다.
bool LostArk::Server::CServerCollisionSystem::Is_PlayerSpawnClear(
	const WORLD_BOOTSTRAP_PLACEMENT& spawn) const
{
	if (WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != spawn.eKind ||
		!spawn.isEnabled)
	{
		return false;
	}
	return std::none_of(
		m_CollisionBoxes.begin(),
		m_CollisionBoxes.end(),
		[this, &spawn](const WORLD_BOOTSTRAP_PLACEMENT& box)
		{
			const std::size_t index = static_cast<std::size_t>(
				&box - m_CollisionBoxes.data());
			if (index >= m_PlayerBlocking.size() || !m_PlayerBlocking[index])
				return false;
			return Is_PlayerCenterInsideExpandedBox(
				spawn.fPositionX,
				spawn.fPositionY,
				spawn.fPositionZ,
				box);
		});
}

// 순간이동/착지 경로가 고정 플레이어 크기로 도착 위치 검사를 요청한다.
bool LostArk::Server::CServerCollisionSystem::Is_PlayerPositionClear(
	const float x, const float y, const float z,
	const LostArk::Shared::NET_ENTITY_ID ignoredBodyId) const
{
	return Is_CirclePositionClear(x, y, z, PLAYER_HALF_EXTENT_X,
		PLAYER_HALF_EXTENT_Y, PLAYER_CENTER_OFFSET_Y, ignoredBodyId);
}

// 도착 좌표와 크기를 검증하고 활성 box 및 자신을 제외한 몸체 겹침을 검사한다.
// box는 XZ 확장 사각형 판정이므로 원과 box 모서리의 정확한 거리 검사보다 보수적이다.
bool LostArk::Server::CServerCollisionSystem::Is_CirclePositionClear(
	const float x, const float y, const float z, const float radius,
	const float halfHeight, const float centerOffsetY,
	const LostArk::Shared::NET_ENTITY_ID ignoredBodyId) const
{
	if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
		!std::isfinite(radius) || radius <= 0.f || !std::isfinite(halfHeight) ||
		halfHeight <= 0.f || !std::isfinite(centerOffsetY))
		return false;
	const float centerY = y + centerOffsetY;
	if (!std::isfinite(centerY))
		return false;
	for (std::size_t index = 0u; index < m_CollisionBoxes.size(); ++index)
	{
		if (index >= m_PlayerBlocking.size() || !m_PlayerBlocking[index])
			continue;
		const auto& box = m_CollisionBoxes[index];
		const auto local = To_BoxLocal(x, z, box);
		if (std::abs(local.x) <= box.fHalfExtentX + radius &&
			std::abs(centerY - box.fPositionY) <= box.fHalfExtentY + halfHeight &&
			std::abs(local.z) <= box.fHalfExtentZ + radius)
			return false;
	}
	for (const auto& body : m_BlockingBodies)
	{
		if (body.iNetEntityId == ignoredBodyId ||
			centerY + halfHeight < body.fCenterY - body.fHalfHeight ||
			centerY - halfHeight > body.fCenterY + body.fHalfHeight)
			continue;
		const float dx = x - body.fX;
		const float dz = z - body.fZ;
		const float combinedRadius = radius + body.fRadius + CONTACT_MARGIN;
		if (dx * dx + dz * dz < combinedRadius * combinedRadius)
			return false;
	}
	return true;
}

// GameRoom의 Refresh_PlayerBlockingBodies가 만든 값 목록으로 갱신한다. 유효하지 않은 행은 건너뛴다.
// 기존 목록을 먼저 비우므로 원자적인 후보 교체나 실패 rollback으로 사용하지 않는다.
void LostArk::Server::CServerCollisionSystem::Set_BlockingBodies(
	std::vector<SERVER_BLOCKING_BODY> bodies)
{
	m_BlockingBodies.clear();
	m_BlockingBodies.reserve(bodies.size());
	for (const SERVER_BLOCKING_BODY& body : bodies)
	{
		if (std::isfinite(body.fX) && std::isfinite(body.fZ) &&
			std::isfinite(body.fRadius) && body.fRadius > 0.f &&
			std::isfinite(body.fCenterY) &&
			std::isfinite(body.fHalfHeight) && body.fHalfHeight > 0.f)
		{
			m_BlockingBodies.push_back(body);
		}
	}
}

// GameRoom이 entity 이동을 확정한 직후 그 ID의 몸체 좌표를 맞춘다. false이면 기존 값을 바꾸지 않는다.
bool LostArk::Server::CServerCollisionSystem::Update_BlockingBody(
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const float x,
	const float centerY,
	const float z) noexcept
{
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == netEntityId ||
		!std::isfinite(x) || !std::isfinite(centerY) || !std::isfinite(z))
	{
		return false;
	}
	const auto body = std::find_if(
		m_BlockingBodies.begin(), m_BlockingBodies.end(),
		[netEntityId](const SERVER_BLOCKING_BODY& candidate)
		{
			return candidate.iNetEntityId == netEntityId;
		});
	if (body == m_BlockingBodies.end())
		return false;
	body->fX = x;
	body->fCenterY = centerY;
	body->fZ = z;
	return true;
}

// 상대 몸체를 고정하고 한 이동 구간 t(0~1)에서 처음 겹치는 지점을 계산한다.
// 수직 이동은 Y slab와 XZ 이차식의 시간 구간을 교차하며 초기 겹침 탈출은 허용한다.
bool LostArk::Server::CServerCollisionSystem::Sweep_CircleAgainstBody(
	const float startX,
	const float startY,
	const float startZ,
	const float proposedX,
	const float proposedY,
	const float proposedZ,
	const float radius,
	const float halfHeight,
	const float centerOffsetY,
	const SERVER_BLOCKING_BODY& body,
	float& outHitRatio)
{
	const float startCenterY = startY + centerOffsetY;
	const float endCenterY = proposedY + centerOffsetY;
	const float movingMinimumY =
		(std::min)(startCenterY, endCenterY) - halfHeight;
	const float movingMaximumY =
		(std::max)(startCenterY, endCenterY) + halfHeight;
	if (movingMaximumY < body.fCenterY - body.fHalfHeight ||
		movingMinimumY > body.fCenterY + body.fHalfHeight)
	{
		return false;
	}
	const float combinedRadius = body.fRadius + radius;
	const float toBodyX = body.fX - startX;
	const float toBodyZ = body.fZ - startZ;
	const float deltaX = proposedX - startX;
	const float deltaZ = proposedZ - startZ;
	const float startDistanceSquared = toBodyX * toBodyX + toBodyZ * toBodyZ;
	if (std::abs(endCenterY - startCenterY) > SWEEP_EPSILON)
	{
		// 비행 중에는 XZ 이동이 없어도 위/아래에서 몸체 높이 범위에 들어올 수 있다.
		// 높이와 XZ의 겹침 시간 구간을 교차해 처음 닿는 비율을 구한다.
		// Y 변화가 없는 기존 지상 경로는 아래 분기에 남긴다.
		const float expandedY = body.fHalfHeight + halfHeight;
		const float closingXZ = deltaX * toBodyX + deltaZ * toBodyZ;
		if (startDistanceSquared < combinedRadius * combinedRadius &&
			std::abs(startCenterY - body.fCenterY) < expandedY && closingXZ <= 0.f)
			return false;
			// 이미 겹쳤지만 중심에서 멀어지는 이동은 허용한다.
		float enter = 0.f, exit = 1.f;
		if (!Update_Slab(startCenterY - body.fCenterY, endCenterY - startCenterY,
			expandedY, enter, exit)) return false;
		const float lengthSquared = deltaX * deltaX + deltaZ * deltaZ;
		const float c = startDistanceSquared - combinedRadius * combinedRadius;
		if (lengthSquared <= SWEEP_EPSILON * SWEEP_EPSILON)
		{
			if (c > 0.f)
				return false;
		}
		else
		{
			const float discriminant = closingXZ * closingXZ - lengthSquared * c;
			if (discriminant < 0.f)
				return false;
			const float root = std::sqrt(discriminant);
			enter = (std::max)(enter, (closingXZ - root) / lengthSquared);
			exit = (std::min)(exit, (closingXZ + root) / lengthSquared);
		}
		if (enter > exit || exit < 0.f || enter > 1.f)
			return false;
		outHitRatio = std::clamp(enter, 0.f, 1.f);
		return true;
	}
	if (startDistanceSquared < combinedRadius * combinedRadius)
	{
		// 시작부터 원이 겹쳐 있으면 중심 쪽으로 더 접근하는 이동만 막는다.
		if (deltaX * toBodyX + deltaZ * toBodyZ <= 0.f)
			return false;
		outHitRatio = 0.f;
		return true;
	}
	const float lengthSquared = deltaX * deltaX + deltaZ * deltaZ;
	if (lengthSquared <= SWEEP_EPSILON * SWEEP_EPSILON)
		return false;
	// 시작 S, 이동 D, 상대 중심 C, 합산 반지름 R에 대해 |S + tD - C|² = R²의 가장 이른 t를 구한다.
	const float b = deltaX * toBodyX + deltaZ * toBodyZ;
	if (b <= 0.f)
		return false;
	const float c = startDistanceSquared - combinedRadius * combinedRadius;
	const float discriminant = b * b - lengthSquared * c;
	if (discriminant < 0.f)
		return false;
	const float t = (b - std::sqrt(discriminant)) / lengthSquared;
	if (t < 0.f || t > 1.f)
		return false;
	outHitRatio = t;
	return true;
}

// GameRoom이 이동 뒤 목표를 끝낼지 결정할 때 사용한다. 목표가 몸체 안이고 이번 경로가 그 몸체에 닿아야 true다.
bool LostArk::Server::CServerCollisionSystem::Is_PlayerMoveBlockedAtGoalBody(
	const SERVER_PLAYER& player, const float proposedX, const float proposedY,
	const float proposedZ, const float goalY) const
{
	if (!player.hasMoveGoal || !std::isfinite(goalY) ||
		!std::isfinite(proposedX) || !std::isfinite(proposedY) || !std::isfinite(proposedZ) ||
		!std::isfinite(player.fPositionX) || !std::isfinite(player.fPositionY) || !std::isfinite(player.fPositionZ) ||
		!std::isfinite(player.fMoveGoalX) || !std::isfinite(player.fMoveGoalZ))
		return false;
	const float goalCenterY = goalY + PLAYER_CENTER_OFFSET_Y;
	for (const auto& body : m_BlockingBodies)
	{
		if (body.iNetEntityId == player.iNetEntityId ||
			goalCenterY + PLAYER_HALF_EXTENT_Y < body.fCenterY - body.fHalfHeight ||
			goalCenterY - PLAYER_HALF_EXTENT_Y > body.fCenterY + body.fHalfHeight)
			continue;
		const float dx = player.fMoveGoalX - body.fX;
		const float dz = player.fMoveGoalZ - body.fZ;
		const float radius = PLAYER_HALF_EXTENT_X + body.fRadius;
		if (dx * dx + dz * dz >= radius * radius)
			continue;
		float hitRatio = 0.f;
		if (Sweep_CircleAgainstBody(player.fPositionX, player.fPositionY,
			player.fPositionZ, proposedX, proposedY, proposedZ,
			PLAYER_HALF_EXTENT_X, PLAYER_HALF_EXTENT_Y, PLAYER_CENTER_OFFSET_Y,
			body, hitRatio))
			return true;
	}
	return false;
}

// 플레이어 위치/공통 크기를 일반 원형 몸체 solver에 전달한다. Mario에서는 접선 우회를 허용하지 않는다.
bool LostArk::Server::CServerCollisionSystem::Resolve_PlayerMove(
	const SERVER_PLAYER& player,
	const float proposedX,
	const float proposedY,
	const float proposedZ,
	float& outX,
	float& outY,
	float& outZ,
	bool& outWasBlocked) const
{
	using namespace LostArk::Shared::WorldCollision;
	return Resolve_CircleMove(
		player.fPositionX,
		player.fPositionY,
		player.fPositionZ,
		proposedX,
		proposedY,
		proposedZ,
		PLAYER_HALF_EXTENT_X,
		PLAYER_HALF_EXTENT_Y,
		PLAYER_CENTER_OFFSET_Y,
		outX,
		outY,
		outZ,
		outWasBlocked,
		LostArk::Shared::INVALID_NET_ENTITY_ID,
		0u == player.iMarioStage);
}

// 플레이어/몬스터/보스 이동 경로가 제안한 위치를 제한한다. 이 함수 자체는 entity 위치를 쓰지 않는다.
// 잘못된 입력은 출력 변경 없이 false다. 성공은 무충돌 통과, 접촉 전 정지, 몸체 접선 우회 중 하나다.
bool LostArk::Server::CServerCollisionSystem::Resolve_CircleMove(
	const float startX,
	const float startY,
	const float startZ,
	const float proposedX,
	const float proposedY,
	const float proposedZ,
	const float radius,
	const float halfHeight,
	const float centerOffsetY,
	float& outX,
	float& outY,
	float& outZ,
	bool& outWasBlocked,
	const LostArk::Shared::NET_ENTITY_ID ignoredBodyId,
	const bool allowBodySlide) const
{
	if (!std::isfinite(startX) ||
		!std::isfinite(startY) ||
		!std::isfinite(startZ) ||
		!std::isfinite(proposedX) ||
		!std::isfinite(proposedY) ||
		!std::isfinite(proposedZ) ||
		!std::isfinite(radius) ||
		!std::isfinite(halfHeight) ||
		!std::isfinite(centerOffsetY) ||
		radius <= 0.f || radius > 1000.f ||
		halfHeight <= 0.f || halfHeight > 1000.f ||
		centerOffsetY < 0.f || centerOffsetY > 1000.f)
	{
		return false;
	}

	constexpr float NO_HIT = (std::numeric_limits<float>::max)();
	float earliestBoxHit = NO_HIT;
	for (std::size_t index = 0u; index < m_CollisionBoxes.size(); ++index)
	{
		if (index >= m_PlayerBlocking.size() || !m_PlayerBlocking[index])
			continue;
		const WORLD_BOOTSTRAP_PLACEMENT& box = m_CollisionBoxes[index];
		float hitRatio = 0.f;
		if (Sweep_MovingBodyAgainstBox(
			startX, startY, startZ,
			proposedX, proposedY, proposedZ,
			radius, halfHeight, centerOffsetY, box, hitRatio))
		{
			earliestBoxHit = (std::min)(earliestBoxHit, hitRatio);
		}
	}
	float earliestBodyHit = NO_HIT;
	const SERVER_BLOCKING_BODY* hitBody = nullptr;
	for (const SERVER_BLOCKING_BODY& body : m_BlockingBodies)
	{
		if (LostArk::Shared::INVALID_NET_ENTITY_ID != ignoredBodyId &&
			body.iNetEntityId == ignoredBodyId)
		{
			continue;
		}
		float hitRatio = 0.f;
		if (Sweep_CircleAgainstBody(
			startX, startY, startZ,
			proposedX, proposedY, proposedZ,
			radius, halfHeight, centerOffsetY, body, hitRatio) &&
			hitRatio < earliestBodyHit)
		{
			earliestBodyHit = hitRatio;
			hitBody = &body;
		}
	}

	const float earliestHit = (std::min)(earliestBoxHit, earliestBodyHit);
	if (NO_HIT == earliestHit)
	{
		outWasBlocked = false;
		outX = proposedX;
		outY = proposedY;
		outZ = proposedZ;
		return true;
	}

	const float deltaX = proposedX - startX;
	const float deltaY = proposedY - startY;
	const float deltaZ = proposedZ - startZ;
	const float distance = std::sqrt(
		deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
	const float marginRatio = distance > SWEEP_EPSILON ?
		CONTACT_MARGIN / distance : 0.f;
	const float safeRatio = (std::max)(0.f, earliestHit - marginRatio);
	outX = startX + deltaX * safeRatio;
	outY = startY + deltaY * safeRatio;
	outZ = startZ + deltaZ * safeRatio;
	outWasBlocked = true;

	// 벽이 먼저 닿으면 그 지점에서 멈춘다. 몸체가 먼저 닿고 우회가 허용되면 접선 방향의 추가 후보를 검사한다.
	// 아래 우회는 반복 solver가 아니라 한 번의 접선 이동이며 원래 이동 방향과 달라질 수 있다.
	if (!allowBodySlide || nullptr == hitBody || earliestBoxHit <= earliestBodyHit)
		return true;
	const float remainingRatio = 1.f - safeRatio;
	const float remainingX = deltaX * remainingRatio;
	const float remainingZ = deltaZ * remainingRatio;
	const float remainingLength = std::sqrt(
		remainingX * remainingX + remainingZ * remainingZ);
	if (remainingLength <= CONTACT_MARGIN)
		return true;
	float normalX = outX - hitBody->fX;
	float normalZ = outZ - hitBody->fZ;
	const float normalLength = std::sqrt(normalX * normalX + normalZ * normalZ);
	if (normalLength <= SWEEP_EPSILON)
		return true;
	normalX /= normalLength;
	normalZ /= normalLength;
	// 남은 XZ 이동 길이를 접선 방향에 유지한다. 정면 접근으로 좌우 성분이 없으면 고정된 한쪽을 고른다.
	// 최종 허용 거리는 다시 벽/다른 몸체 sweep으로 제한한다.
	const float tangentX = -normalZ;
	const float tangentZ = normalX;
	float along = remainingX * tangentX + remainingZ * tangentZ;
	if (std::abs(along) <= SWEEP_EPSILON)
		along = 1.f;
	const float side = along < 0.f ? -1.f : 1.f;
	const float slideX = tangentX * side * remainingLength;
	const float slideZ = tangentZ * side * remainingLength;
	const float slideLength = remainingLength;

	const float slideTargetX = outX + slideX;
	const float slideTargetZ = outZ + slideZ;
	float earliestSlideHit = NO_HIT;
	for (std::size_t index = 0u; index < m_CollisionBoxes.size(); ++index)
	{
		if (index >= m_PlayerBlocking.size() || !m_PlayerBlocking[index])
			continue;
		float hitRatio = 0.f;
		if (Sweep_MovingBodyAgainstBox(
			outX, outY, outZ,
			slideTargetX, outY, slideTargetZ,
			radius, halfHeight, centerOffsetY,
			m_CollisionBoxes[index], hitRatio))
		{
			earliestSlideHit = (std::min)(earliestSlideHit, hitRatio);
		}
	}
	for (const SERVER_BLOCKING_BODY& body : m_BlockingBodies)
	{
		if (LostArk::Shared::INVALID_NET_ENTITY_ID != ignoredBodyId &&
			body.iNetEntityId == ignoredBodyId)
		{
			continue;
		}
		// 접선으로 벗어나는 기준 몸체는 재검사하지 않아 접촉점 반올림이 우회를 막지 않게 한다.
		if (&body == hitBody)
			continue;
		float hitRatio = 0.f;
		if (Sweep_CircleAgainstBody(
			outX, outY, outZ,
			slideTargetX, outY, slideTargetZ,
			radius, halfHeight, centerOffsetY, body, hitRatio))
		{
			earliestSlideHit = (std::min)(earliestSlideHit, hitRatio);
		}
	}
	const float slideRatio = NO_HIT == earliestSlideHit ? 1.f :
		(std::max)(0.f, earliestSlideHit - CONTACT_MARGIN / slideLength);
	if (slideRatio <= 0.f)
		return true;
	outX += slideX * slideRatio;
	outZ += slideZ * slideRatio;
	// 접선으로 조금이라도 이동하면 blocked를 지운다. GameRoom은 별도로 목표 몸체에 도달했는지 검사한다.
	outWasBlocked = false;
	return true;
}

// 보스 돌진의 receiver 접촉을 조회한다. 출력 초기화 후 가장 이른 t, 거의 같으면 stable ID 순서로 선택한다.
// receiver flag를 읽으며 player blocking flag는 이 조회의 조건이 아니다.
bool LostArk::Server::CServerCollisionSystem::Sweep_BossCircleAgainstReceivers(
	const float startX,
	const float startY,
	const float startZ,
	const float proposedX,
	const float proposedY,
	const float proposedZ,
	const float radius,
	SERVER_BOSS_RECEIVER_HIT& outHit) const
{
	outHit = {};
	if (!std::isfinite(startX) || !std::isfinite(startY) ||
		!std::isfinite(startZ) || !std::isfinite(proposedX) ||
		!std::isfinite(proposedY) || !std::isfinite(proposedZ) ||
		!std::isfinite(radius) || radius <= 0.f || radius > 1000.f)
	{
		return false;
	}

	bool found = false;
	float earliestHit = (std::numeric_limits<float>::max)();
	for (std::size_t index = 0u; index < m_CollisionBoxes.size(); ++index)
	{
		if (index >= m_ImpactReceiverEnabled.size() ||
			!m_ImpactReceiverEnabled[index])
		{
			continue;
		}
		float hitRatio = 0.f;
		const WORLD_BOOTSTRAP_PLACEMENT& box = m_CollisionBoxes[index];
		if (!Sweep_CircleAgainstBox(
			startX, startY, startZ,
			proposedX, proposedY, proposedZ, radius, box, hitRatio))
		{
			continue;
		}
		if (!found || hitRatio < earliestHit - SWEEP_EPSILON ||
			(std::abs(hitRatio - earliestHit) <= SWEEP_EPSILON &&
			 box.strPlacementId < outHit.strReceiverPlacementId))
		{
			found = true;
			earliestHit = hitRatio;
			outHit.strReceiverPlacementId = box.strPlacementId;
			outHit.fHitRatio = hitRatio;
		}
	}
	return found;
}

// GameRoom 보스 이동이 처음 막히는 벽과 선택적인 receiver를 찾는다. 거의 같은 t는 같은 표면 후보로 취급한다.
// 기존 outHit은 실패 여부와 무관하게 비우며 실제 벽 파괴는 호출자가 처리한다.
bool LostArk::Server::CServerCollisionSystem::Sweep_BossCircleAgainstWalls(
	const float startX,
	const float startY,
	const float startZ,
	const float proposedX,
	const float proposedY,
	const float proposedZ,
	const float radius,
	SERVER_BOSS_WALL_HIT& outHit) const
{
	outHit = {};
	if (!std::isfinite(startX) || !std::isfinite(startY) ||
		!std::isfinite(startZ) || !std::isfinite(proposedX) ||
		!std::isfinite(proposedY) || !std::isfinite(proposedZ) ||
		!std::isfinite(radius) || radius <= 0.f || radius > 1000.f)
	{
		return false;
	}

	bool found = false;
	float earliestHit = (std::numeric_limits<float>::max)();
	for (std::size_t index = 0u; index < m_CollisionBoxes.size(); ++index)
	{
		if (index >= m_PlayerBlocking.size() || !m_PlayerBlocking[index])
			continue;
		float hitRatio = 0.f;
		const WORLD_BOOTSTRAP_PLACEMENT& box = m_CollisionBoxes[index];
		if (!Sweep_CircleAgainstBox(
			startX, startY, startZ,
			proposedX, proposedY, proposedZ, radius, box, hitRatio))
		{
			continue;
		}
		// 시작부터 벽과 겹친 경우 가장 가까운 표면의 법선으로 탈출/접선 이동인지 확인한다.
		// 유효한 법선이 있고 더 파고들지 않으면 이 벽은 이번 돌진의 정지 표면에서 제외한다.
		if (hitRatio <= SWEEP_EPSILON)
		{
			const LOCAL_POINT start = To_BoxLocal(startX, startZ, box);
			const LOCAL_POINT end = To_BoxLocal(proposedX, proposedZ, box);
			const float closestX = (std::clamp)(
				start.x, -box.fHalfExtentX, box.fHalfExtentX);
			const float closestZ = (std::clamp)(
				start.z, -box.fHalfExtentZ, box.fHalfExtentZ);
			const float normalX = start.x - closestX;
			const float normalZ = start.z - closestZ;
			const float normalLengthSquared =
				normalX * normalX + normalZ * normalZ;
			const float intoSurface =
				(end.x - start.x) * normalX +
				(end.z - start.z) * normalZ;
			if (normalLengthSquared > SWEEP_EPSILON * SWEEP_EPSILON &&
				intoSurface >= -SWEEP_EPSILON)
			{
				continue;
			}
		}
		const bool impactReceiverEnabled =
			index < m_ImpactReceiverEnabled.size() &&
			m_ImpactReceiverEnabled[index];
		const bool earlierSurface = !found ||
			hitRatio < earliestHit - SWEEP_EPSILON;
		const bool sameSurface = found &&
			std::abs(hitRatio - earliestHit) <= SWEEP_EPSILON;
		if (!earlierSurface && !sameSurface)
			continue;
		if (earlierSurface)
		{
			found = true;
			earliestHit = hitRatio;
			outHit.strCollisionPlacementId = box.strPlacementId;
			outHit.strImpactReceiverPlacementId = impactReceiverEnabled ?
				box.strPlacementId : std::string{};
			outHit.fHitRatio = hitRatio;
			continue;
		}
		if (box.strPlacementId < outHit.strCollisionPlacementId)
			outHit.strCollisionPlacementId = box.strPlacementId;
		if (impactReceiverEnabled &&
			(outHit.strImpactReceiverPlacementId.empty() ||
			 box.strPlacementId < outHit.strImpactReceiverPlacementId))
		{
			outHit.strImpactReceiverPlacementId = box.strPlacementId;
		}
	}
	return found;
}

// GameRoom의 보스 몸체 접촉 파괴가 읽을 모든 blocking 벽 ID를 출력한다. 출력 목록은 매 호출 새로 만든다.
void LostArk::Server::CServerCollisionSystem::Collect_BossCircleContacts(
	const float startX,
	const float startY,
	const float startZ,
	const float proposedX,
	const float proposedY,
	const float proposedZ,
	const float radius,
	std::vector<std::string>& outContactPlacementIds) const
{
	outContactPlacementIds.clear();
	if (!std::isfinite(startX) || !std::isfinite(startY) ||
		!std::isfinite(startZ) || !std::isfinite(proposedX) ||
		!std::isfinite(proposedY) || !std::isfinite(proposedZ) ||
		!std::isfinite(radius) || radius <= 0.f || radius > 1000.f)
	{
		return;
	}

	for (std::size_t index = 0u; index < m_CollisionBoxes.size(); ++index)
	{
		// blocking이 해제된 벽은 이 접촉 목록에서 제외한다. receiver flag만으로 다시 포함하지 않는다.
		if (index >= m_PlayerBlocking.size() || !m_PlayerBlocking[index])
			continue;
		float hitRatio = 0.f;
		if (Sweep_CircleAgainstBox(
			startX, startY, startZ,
			proposedX, proposedY, proposedZ, radius,
			m_CollisionBoxes[index], hitRatio))
		{
			outContactPlacementIds.push_back(
				m_CollisionBoxes[index].strPlacementId);
		}
	}
}

// 파괴 transaction이 collision과 navigation을 함께 준비할 때 호출한다. 실제 flag를 복사해 출력 후보만 변경한다.
// 중복 ID, 겹치는 부모/자식 대상, 없는 대상은 false다. 실제 변경이 있고 revision이 소진되어도 false이며 부분 후보는 폐기한다.
bool LostArk::Server::CServerCollisionSystem::Prepare_StateChanges(
	const std::vector<SERVER_COLLISION_STATE_CHANGE>& changes,
	SERVER_COLLISION_STATE_STAGE& outStage,
	std::string& outStatus) const
{
	outStage = {};
	outStage.iBaseRevision = m_iRevision;
	outStage.iNextRevision = m_iRevision;
	outStage.PlayerBlocking = m_PlayerBlocking;
	outStage.ImpactReceiverEnabled = m_ImpactReceiverEnabled;
	std::set<std::string> changedIds;
	std::set<std::size_t> changedIndices;
	for (const SERVER_COLLISION_STATE_CHANGE& change : changes)
	{
		if (!changedIds.insert(change.strPlacementId).second)
		{
			outStatus = "Duplicate collision state change: " + change.strPlacementId;
			return false;
		}
		if (change.strPlacementId.empty())
		{
			outStatus = "Empty collision state target";
			return false;
		}
		const std::string childPrefix = change.strPlacementId + ".";
		bool matched = false;
		for (std::size_t index = 0u; index < m_CollisionBoxes.size(); ++index)
		{
			const std::string& boxId = m_CollisionBoxes[index].strPlacementId;
			const bool isExact = boxId == change.strPlacementId;
			const bool isChild = boxId.size() > childPrefix.size() &&
				0 == boxId.compare(0u, childPrefix.size(), childPrefix);
			if (!isExact && !isChild)
				continue;
			matched = true;
			if (!changedIndices.insert(index).second)
			{
				outStatus = "Overlapping collision state targets: " +
					change.strPlacementId;
				return false;
			}
			const bool receiverEnabled = change.bImpactReceiverEnabled &&
				Is_ImpactReceiverPlacementId(boxId);
			if (outStage.PlayerBlocking[index] != change.bPlayerBlocking ||
				outStage.ImpactReceiverEnabled[index] != receiverEnabled)
			{
				outStage.PlayerBlocking[index] = change.bPlayerBlocking;
				outStage.ImpactReceiverEnabled[index] = receiverEnabled;
				outStage.bChanged = true;
			}
		}
		if (!matched)
		{
			outStatus = "Unknown collision state target: " + change.strPlacementId;
			return false;
		}
	}
	if (outStage.bChanged)
	{
		if (m_iRevision == (std::numeric_limits<std::uint64_t>::max)())
		{
			outStatus = "Collision state revision is exhausted";
			return false;
		}
		outStage.iNextRevision = m_iRevision + 1u;
	}
	outStatus = outStage.bChanged ?
		"Collision state changes staged" :
		"Collision state changes are a no-op";
	return true;
}

// GameRoom이 WorldDestructionRuntime::Commit 성공 뒤 준비한 flag를 이동한다. 기준 revision과 vector 크기는 여기서 다시 검사하지 않는다.
void LostArk::Server::CServerCollisionSystem::Commit_StateChanges(
	SERVER_COLLISION_STATE_STAGE&& stage) noexcept
{
	if (!stage.bChanged)
		return;
	m_PlayerBlocking = std::move(stage.PlayerBlocking);
	m_ImpactReceiverEnabled = std::move(stage.ImpactReceiverEnabled);
	m_iRevision = stage.iNextRevision;
}

// 방/발탄 audition 초기화가 벽 flag를 되돌린다. 동적 몸체나 placement 소유 목록을 정리하는 함수는 아니다.
void LostArk::Server::CServerCollisionSystem::Reset_RuntimeStates() noexcept
{
	m_PlayerBlocking.assign(m_CollisionBoxes.size(), true);
	m_ImpactReceiverEnabled.clear();
	m_ImpactReceiverEnabled.reserve(m_CollisionBoxes.size());
	for (const WORLD_BOOTSTRAP_PLACEMENT& box : m_CollisionBoxes)
	{
		m_ImpactReceiverEnabled.push_back(
			Is_ImpactReceiverPlacementId(box.strPlacementId));
	}
	m_iRevision = 1u;
}

// receiver 역할은 .receiver 접미사 계약으로 정한다. 해당 접미사만 있는 문자열은 허용하지 않는다.
bool LostArk::Server::CServerCollisionSystem::Is_ImpactReceiverPlacementId(
	const std::string_view placementId) noexcept
{
	constexpr std::string_view suffix = ".receiver";
	return placementId.size() > suffix.size() &&
		placementId.substr(placementId.size() - suffix.size()) == suffix;
}

// 정확한 stable ID가 충돌 placement 목록에 있는지 찾는다. 현재 blocking 여부는 따로 조회한다.
bool LostArk::Server::CServerCollisionSystem::Has_CollisionBox(
	const std::string& placementId) const
{
	return std::any_of(
		m_CollisionBoxes.begin(), m_CollisionBoxes.end(),
		[&placementId](const WORLD_BOOTSTRAP_PLACEMENT& box)
		{
			return box.strPlacementId == placementId;
		});
}

// GameRoom의 공격 접촉 파괴 경로가 사용하는 월드 도형 검사다. 결과 ID만 모으며 벽 상태나 HP는 변경하지 않는다.
// originY에서 2*verticalReach 높이까지 검사하고 XZ에서는 도형별 원/링/부채꼴 근사/OBB를 사용한다.
void LostArk::Server::CServerCollisionSystem::Collect_BossPatternHitContacts(
	const BOSS_PATTERN_HIT_SHAPE hitShape,
	const float originX,
	const float originY,
	const float originZ,
	const float yawDegrees,
	const float verticalReach,
	const float outerRadius,
	const float innerRadius,
	const float angleDegrees,
	const float length,
	const float halfWidth,
	std::vector<std::string>& outContactPlacementIds) const
{
	outContactPlacementIds.clear();
	if (BOSS_PATTERN_HIT_SHAPE::NONE == hitShape ||
		!std::isfinite(originX) || !std::isfinite(originY) ||
		!std::isfinite(originZ) || !std::isfinite(yawDegrees) ||
		!std::isfinite(verticalReach) || !std::isfinite(outerRadius) ||
		!std::isfinite(innerRadius) || !std::isfinite(angleDegrees) ||
		!std::isfinite(length) || !std::isfinite(halfWidth) ||
		verticalReach <= 0.f || verticalReach > 1000.f)
	{
		return;
	}

	const float yaw = yawDegrees * DEGREES_TO_RADIANS;
	const float forwardX = std::sin(yaw);
	const float forwardZ = std::cos(yaw);
	for (std::size_t index = 0u; index < m_CollisionBoxes.size(); ++index)
	{
		if (index >= m_PlayerBlocking.size() || !m_PlayerBlocking[index] ||
			(index < m_ImpactReceiverEnabled.size() &&
			 m_ImpactReceiverEnabled[index]))
		{
			continue;
		}
		const WORLD_BOOTSTRAP_PLACEMENT& box = m_CollisionBoxes[index];
		const float attackCenterY = originY + verticalReach;
		if (std::abs(attackCenterY - box.fPositionY) >
			verticalReach + box.fHalfExtentY)
		{
			continue;
		}

		bool intersects = false;
		switch (hitShape)
		{
		case BOSS_PATTERN_HIT_SHAPE::CIRCLE:
			intersects = outerRadius > 0.f &&
				Circle_IntersectsObb(
					originX, originZ, outerRadius, box);
			break;
		case BOSS_PATTERN_HIT_SHAPE::RING:
			intersects = innerRadius > 0.f && outerRadius > innerRadius &&
				Ring_IntersectsObb(
					originX, originZ, innerRadius, outerRadius, box);
			break;
		case BOSS_PATTERN_HIT_SHAPE::CONE:
			intersects = angleDegrees > 0.f && angleDegrees <= 180.f &&
				length > 0.f && Cone_IntersectsObb(
					originX, originZ, yawDegrees, angleDegrees, length, box);
			break;
		case BOSS_PATTERN_HIT_SHAPE::BOX:
			if (length > 0.f && halfWidth > 0.f)
			{
				const OBB_2D attack = Make_AttackObb(
					originX + forwardX * length * 0.5f,
					originZ + forwardZ * length * 0.5f,
					yawDegrees, halfWidth, length * 0.5f);
				intersects = Obbs_Intersect(attack, To_Obb(box));
			}
			break;
		case BOSS_PATTERN_HIT_SHAPE::CROSS:
			if (length > 0.f && halfWidth > 0.f)
			{
				const OBB_2D forwardAttack = Make_AttackObb(
					originX, originZ, yawDegrees, halfWidth, length);
				const OBB_2D lateralAttack = Make_AttackObb(
					originX, originZ, yawDegrees, length, halfWidth);
				const OBB_2D wall = To_Obb(box);
				intersects = Obbs_Intersect(forwardAttack, wall) ||
					Obbs_Intersect(lateralAttack, wall);
			}
			break;
		case BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS:
			if (length > 0.f && halfWidth > 0.f)
			{
				const std::array<OBB_2D, 3u> attacks = {{
					Make_AttackObb(
						originX, originZ, yawDegrees, halfWidth, length),
					Make_AttackObb(
						originX, originZ, yawDegrees + 60.f, halfWidth, length),
					Make_AttackObb(
						originX, originZ, yawDegrees + 120.f, halfWidth, length)
				}};
				const OBB_2D wall = To_Obb(box);
				intersects = std::any_of(
					attacks.begin(), attacks.end(),
					[&wall](const OBB_2D& attack)
					{
						return Obbs_Intersect(attack, wall);
					});
			}
			break;
		case BOSS_PATTERN_HIT_SHAPE::NONE:
		default:
			break;
		}
		if (intersects)
			outContactPlacementIds.push_back(box.strPlacementId);
	}
}

// 정확한 ID의 현재 blocking flag를 읽는다. 없는 ID 또는 없는 flag는 false다.
bool LostArk::Server::CServerCollisionSystem::Is_PlayerBlocking(
	const std::string& placementId) const
{
	for (std::size_t index = 0u; index < m_CollisionBoxes.size(); ++index)
	{
		if (m_CollisionBoxes[index].strPlacementId == placementId)
		{
			return index < m_PlayerBlocking.size() && m_PlayerBlocking[index];
		}
	}
	return false;
}

// 정확한 ID의 receiver flag를 읽는다. blocking flag와 독립된 채널이다.
bool LostArk::Server::CServerCollisionSystem::Is_ImpactReceiverEnabled(
	const std::string& placementId) const
{
	for (std::size_t index = 0u; index < m_CollisionBoxes.size(); ++index)
	{
		if (m_CollisionBoxes[index].strPlacementId == placementId)
		{
			return index < m_ImpactReceiverEnabled.size() &&
				m_ImpactReceiverEnabled[index];
		}
	}
	return false;
}

// GameRoom의 파괴 연결 검증에 사용할 ID 범위를 확인한다. 정확한 ID 또는 점으로 구분한 하위 ID를 허용한다.
bool LostArk::Server::CServerCollisionSystem::Has_CollisionStateTarget(
	const std::string& stateId) const
{
	if (stateId.empty())
		return false;
	const std::string childPrefix = stateId + ".";
	return std::any_of(
		m_CollisionBoxes.begin(), m_CollisionBoxes.end(),
		[&stateId, &childPrefix](const WORLD_BOOTSTRAP_PLACEMENT& box)
		{
			return box.strPlacementId == stateId ||
				(box.strPlacementId.size() > childPrefix.size() &&
				 0 == box.strPlacementId.compare(
					0u, childPrefix.size(), childPrefix));
		});
}

// spawn의 발 위치에 공통 중심 오프셋을 더한 뒤 확장 box 안에 있는지 검사한다. 여기서는 추가 입력 검증을 하지 않는다.
bool LostArk::Server::CServerCollisionSystem::Is_PlayerCenterInsideExpandedBox(
	const float playerX,
	const float playerY,
	const float playerZ,
	const WORLD_BOOTSTRAP_PLACEMENT& box)
{
	using namespace LostArk::Shared::WorldCollision;
	const LOCAL_POINT local = To_BoxLocal(playerX, playerZ, box);
	const float playerCenterY = playerY + PLAYER_CENTER_OFFSET_Y;
	return std::abs(local.x) <= box.fHalfExtentX + PLAYER_HALF_EXTENT_X &&
		std::abs(playerCenterY - box.fPositionY) <=
			box.fHalfExtentY + PLAYER_HALF_EXTENT_Y &&
		std::abs(local.z) <= box.fHalfExtentZ + PLAYER_HALF_EXTENT_Z;
}

// 이동 높이 전체와 box의 수직 범위를 먼저 비교하고, 로컬 XZ 확장 box에 선분 slab 검사를 한다.
// Y의 접촉 시각까지 교차하는 완전한 3차원 연속 판정은 아니다. Y 필터 통과 후 XZ가 margin 안쪽에서 시작하면 t=0이다.
bool LostArk::Server::CServerCollisionSystem::Sweep_MovingBodyAgainstBox(
	const float startX,
	const float startY,
	const float startZ,
	const float proposedX,
	const float proposedY,
	const float proposedZ,
	const float radius,
	const float halfHeight,
	const float centerOffsetY,
	const WORLD_BOOTSTRAP_PLACEMENT& box,
	float& outHitRatio)
{
	const float startCenterY = startY + centerOffsetY;
	const float endCenterY = proposedY + centerOffsetY;
	const float expandedY = box.fHalfExtentY + halfHeight;
	if ((std::max)(startCenterY, endCenterY) < box.fPositionY - expandedY ||
		(std::min)(startCenterY, endCenterY) > box.fPositionY + expandedY)
	{
		return false;
	}

	const LOCAL_POINT start = To_BoxLocal(startX, startZ, box);
	const LOCAL_POINT end = To_BoxLocal(proposedX, proposedZ, box);
	const float expandedX = box.fHalfExtentX + radius;
	const float expandedZ = box.fHalfExtentZ + radius;
	if (std::abs(start.x) < expandedX - CONTACT_MARGIN &&
		std::abs(start.z) < expandedZ - CONTACT_MARGIN)
	{
		outHitRatio = 0.f;
		return true;
	}

	float enter = 0.f;
	float exit = 1.f;
	if (!Update_Slab(start.x, end.x - start.x, expandedX, enter, exit) ||
		!Update_Slab(start.z, end.z - start.z, expandedZ, enter, exit) ||
		exit < 0.f || enter > 1.f)
	{
		return false;
	}
	outHitRatio = (std::clamp)(enter, 0.f, 1.f);
	return true;
}

// 보스 반지름을 XZ 확장과 Y 반높이/중심 오프셋에 공통 적용한다.
// Y는 구간 높이 범위, XZ는 slab 교차 비율이며 false일 때 outHitRatio를 보존한다.
bool LostArk::Server::CServerCollisionSystem::Sweep_CircleAgainstBox(
	const float startX,
	const float startY,
	const float startZ,
	const float proposedX,
	const float proposedY,
	const float proposedZ,
	const float radius,
	const WORLD_BOOTSTRAP_PLACEMENT& box,
	float& outHitRatio)
{
	const float startCenterY = startY + radius;
	const float endCenterY = proposedY + radius;
	const float expandedY = box.fHalfExtentY + radius;
	if ((std::max)(startCenterY, endCenterY) < box.fPositionY - expandedY ||
		(std::min)(startCenterY, endCenterY) > box.fPositionY + expandedY)
	{
		return false;
	}

	const LOCAL_POINT start = To_BoxLocal(startX, startZ, box);
	const LOCAL_POINT end = To_BoxLocal(proposedX, proposedZ, box);
	const float expandedX = box.fHalfExtentX + radius;
	const float expandedZ = box.fHalfExtentZ + radius;
	if (std::abs(start.x) < expandedX - CONTACT_MARGIN &&
		std::abs(start.z) < expandedZ - CONTACT_MARGIN)
	{
		outHitRatio = 0.f;
		return true;
	}
	float enter = 0.f;
	float exit = 1.f;
	if (!Update_Slab(start.x, end.x - start.x, expandedX, enter, exit) ||
		!Update_Slab(start.z, end.z - start.z, expandedZ, enter, exit) ||
		exit < 0.f || enter > 1.f)
	{
		return false;
	}
	outHitRatio = (std::clamp)(enter, 0.f, 1.f);
	return true;
}
