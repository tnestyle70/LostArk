#include "ServerCollisionSystem.h"

#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
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
