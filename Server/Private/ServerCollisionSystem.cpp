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
	constexpr float CONTACT_MARGIN = 0.001f;

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

	struct OBB_2D final
	{
		WORLD_POINT center{};
		WORLD_POINT right{};
		WORLD_POINT forward{};
		float halfRight = 0.f;
		float halfForward = 0.f;
	};

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

	float Dot(const WORLD_POINT& left, const WORLD_POINT& right)
	{
		return left.x * right.x + left.z * right.z;
	}

	WORLD_POINT Subtract(const WORLD_POINT& left, const WORLD_POINT& right)
	{
		return { left.x - right.x, left.z - right.z };
	}

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

	bool Obbs_Intersect(const OBB_2D& left, const OBB_2D& right)
	{
		return Overlaps_OnAxis(left, right, left.right) &&
			Overlaps_OnAxis(left, right, left.forward) &&
			Overlaps_OnAxis(left, right, right.right) &&
			Overlaps_OnAxis(left, right, right.forward);
	}

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
	if (!std::isfinite(player.fPositionX) ||
		!std::isfinite(player.fPositionY) ||
		!std::isfinite(player.fPositionZ) ||
		!std::isfinite(proposedX) ||
		!std::isfinite(proposedY) ||
		!std::isfinite(proposedZ))
	{
		return false;
	}

	float earliestHit = (std::numeric_limits<float>::max)();
	for (std::size_t index = 0u; index < m_CollisionBoxes.size(); ++index)
	{
		if (index >= m_PlayerBlocking.size() || !m_PlayerBlocking[index])
			continue;
		const WORLD_BOOTSTRAP_PLACEMENT& box = m_CollisionBoxes[index];
		float hitRatio = 0.f;
		if (Sweep_PlayerAgainstBox(
			player, proposedX, proposedY, proposedZ, box, hitRatio))
		{
			earliestHit = (std::min)(earliestHit, hitRatio);
		}
	}

	outWasBlocked = earliestHit != (std::numeric_limits<float>::max)();
	if (!outWasBlocked)
	{
		outX = proposedX;
		outY = proposedY;
		outZ = proposedZ;
		return true;
	}

	const float deltaX = proposedX - player.fPositionX;
	const float deltaY = proposedY - player.fPositionY;
	const float deltaZ = proposedZ - player.fPositionZ;
	const float distance = std::sqrt(
		deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
	const float marginRatio = distance > SWEEP_EPSILON ?
		CONTACT_MARGIN / distance : 0.f;
	const float safeRatio = (std::max)(0.f, earliestHit - marginRatio);
	outX = player.fPositionX + deltaX * safeRatio;
	outY = player.fPositionY + deltaY * safeRatio;
	outZ = player.fPositionZ + deltaZ * safeRatio;
	return true;
}

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
		/* A box that no longer blocks is a wall that already fell, so it is not
		   a surface anything can hit again. */
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

void LostArk::Server::CServerCollisionSystem::Commit_StateChanges(
	SERVER_COLLISION_STATE_STAGE&& stage) noexcept
{
	if (!stage.bChanged)
		return;
	m_PlayerBlocking = std::move(stage.PlayerBlocking);
	m_ImpactReceiverEnabled = std::move(stage.ImpactReceiverEnabled);
	m_iRevision = stage.iNextRevision;
}

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

bool LostArk::Server::CServerCollisionSystem::Is_ImpactReceiverPlacementId(
	const std::string_view placementId) noexcept
{
	constexpr std::string_view suffix = ".receiver";
	return placementId.size() > suffix.size() &&
		placementId.substr(placementId.size() - suffix.size()) == suffix;
}

bool LostArk::Server::CServerCollisionSystem::Has_CollisionBox(
	const std::string& placementId) const
{
	return std::any_of(
		m_CollisionBoxes.begin(), m_CollisionBoxes.end(),
		[&placementId](const WORLD_BOOTSTRAP_PLACEMENT& box)
		{ return box.strPlacementId == placementId; });
}

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
					{ return Obbs_Intersect(attack, wall); });
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

bool LostArk::Server::CServerCollisionSystem::Sweep_PlayerAgainstBox(
	const SERVER_PLAYER& player,
	const float proposedX,
	const float proposedY,
	const float proposedZ,
	const WORLD_BOOTSTRAP_PLACEMENT& box,
	float& outHitRatio)
{
	using namespace LostArk::Shared::WorldCollision;
	const float startCenterY = player.fPositionY + PLAYER_CENTER_OFFSET_Y;
	const float endCenterY = proposedY + PLAYER_CENTER_OFFSET_Y;
	const float expandedY = box.fHalfExtentY + PLAYER_HALF_EXTENT_Y;
	if ((std::max)(startCenterY, endCenterY) < box.fPositionY - expandedY ||
		(std::min)(startCenterY, endCenterY) > box.fPositionY + expandedY)
	{
		return false;
	}

	const LOCAL_POINT start = To_BoxLocal(
		player.fPositionX, player.fPositionZ, box);
	const LOCAL_POINT end = To_BoxLocal(proposedX, proposedZ, box);
	const float expandedX = box.fHalfExtentX + PLAYER_HALF_EXTENT_X;
	const float expandedZ = box.fHalfExtentZ + PLAYER_HALF_EXTENT_Z;
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
