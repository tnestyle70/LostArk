#pragma once

#include "GameplayCatalog.h"
#include "ServerPlayer.h"
#include "WorldBootstrap.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_COLLISION_STATE_CHANGE final
	{
		std::string strPlacementId;
		bool bPlayerBlocking = true;
		bool bImpactReceiverEnabled = true;
	};

	struct SERVER_COLLISION_STATE_STAGE final
	{
		std::uint64_t iBaseRevision = 0u;
		std::uint64_t iNextRevision = 0u;
		std::vector<bool> PlayerBlocking;
		std::vector<bool> ImpactReceiverEnabled;
		bool bChanged = false;
	};

	struct SERVER_BOSS_RECEIVER_HIT final
	{
		std::string strReceiverPlacementId;
		float fHitRatio = 1.f;
	};

	/* One living world entity body, on the XZ plane, that a player cannot
	walk or root-motion into. The room rebuilds the list every tick from its
	entities; the collision system never learns what the body belongs to. */
	struct SERVER_BLOCKING_BODY final
	{
		float fX = 0.f;
		float fZ = 0.f;
		float fRadius = 0.f;
	};

	class CServerCollisionSystem final
	{
	public:
		bool Initialize(
			const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
			std::string& outStatus);
		bool Is_PlayerSpawnClear(
			const WORLD_BOOTSTRAP_PLACEMENT& spawn) const;
		void Set_BlockingBodies(std::vector<SERVER_BLOCKING_BODY> bodies);
		bool Resolve_PlayerMove(
			const SERVER_PLAYER& player,
			float proposedX,
			float proposedY,
			float proposedZ,
			float& outX,
			float& outY,
			float& outZ,
			bool& outWasBlocked) const;
		bool Sweep_BossCircleAgainstReceivers(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			SERVER_BOSS_RECEIVER_HIT& outHit) const;
		/* Every collision box the boss body actually reached on this tick, not
		just the earliest one and not just the authored impact receivers. A body
		wider than one wall can touch several, and each of them is its own wall
		with its own contact binding. */
		void Collect_BossCircleContacts(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			std::vector<std::string>& outContactPlacementIds) const;
		/* Project one authored Server attack pulse against intact wall OBBs.
		The Client axe/bone collider is presentation-only; this deterministic
		query is the product collision authority. */
		void Collect_BossPatternHitContacts(
			BOSS_PATTERN_HIT_SHAPE hitShape,
			float originX,
			float originY,
			float originZ,
			float yawDegrees,
			float verticalReach,
			float outerRadius,
			float innerRadius,
			float angleDegrees,
			float length,
			float halfWidth,
			std::vector<std::string>& outContactPlacementIds) const;
		bool Prepare_StateChanges(
			const std::vector<SERVER_COLLISION_STATE_CHANGE>& changes,
			SERVER_COLLISION_STATE_STAGE& outStage,
			std::string& outStatus) const;
		void Commit_StateChanges(SERVER_COLLISION_STATE_STAGE&& stage) noexcept;
		void Reset_RuntimeStates() noexcept;
		bool Has_CollisionBox(const std::string& placementId) const;
		bool Has_CollisionStateTarget(const std::string& stateId) const;
		bool Is_PlayerBlocking(const std::string& placementId) const;
		bool Is_ImpactReceiverEnabled(const std::string& placementId) const;
		std::uint64_t Get_Revision() const noexcept { return m_iRevision; }

		[[nodiscard]] std::size_t Get_CollisionBoxCount() const
		{
			return m_CollisionBoxes.size();
		}
		// Diagnostic counter for the Debug audition panel: how many collision
		// boxes still block a player right now.
		[[nodiscard]] std::size_t Get_ActivePlayerBlockingCount() const
		{
			std::size_t active = 0u;
			for (const bool blocking : m_PlayerBlocking)
			{
				if (blocking)
					++active;
			}
			return active;
		}

	private:
		static bool Is_PlayerCenterInsideExpandedBox(
			float playerX,
			float playerY,
			float playerZ,
			const WORLD_BOOTSTRAP_PLACEMENT& box);
		static bool Sweep_PlayerAgainstBox(
			const SERVER_PLAYER& player,
			float proposedX,
			float proposedY,
			float proposedZ,
			const WORLD_BOOTSTRAP_PLACEMENT& box,
			float& outHitRatio);
		static bool Sweep_CircleAgainstBox(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			const WORLD_BOOTSTRAP_PLACEMENT& box,
			float& outHitRatio);
		static bool Is_ImpactReceiverPlacementId(
			std::string_view placementId) noexcept;
		/* XZ sweep of the player's body circle against one entity body. A
		start already inside the combined radius only blocks motion toward the
		body's centre, so a player a boss landed on can always step out. */
		static bool Sweep_PlayerAgainstBody(
			const SERVER_PLAYER& player,
			float proposedX,
			float proposedZ,
			const SERVER_BLOCKING_BODY& body,
			float& outHitRatio);

	private:
		std::vector<WORLD_BOOTSTRAP_PLACEMENT> m_CollisionBoxes;
		std::vector<bool> m_PlayerBlocking;
		std::vector<bool> m_ImpactReceiverEnabled;
		std::vector<SERVER_BLOCKING_BODY> m_BlockingBodies;
		std::uint64_t m_iRevision = 0u;
	};
}
