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

	struct SERVER_BOSS_WALL_HIT final
	{
		std::string strCollisionPlacementId;
		std::string strImpactReceiverPlacementId;
		float fHitRatio = 1.f;
	};

	/* One living world entity body. XZ owns the circular footprint and the
	vertical span prevents entities on separate floors from blocking each other.
	The stable NetEntityId lets a generic mover exclude itself. */
	struct SERVER_BLOCKING_BODY final
	{
		float fX = 0.f;
		float fZ = 0.f;
		float fRadius = 0.f;
		/* Legacy three-field bodies intentionally keep an unbounded vertical
		span. Product world entities fill an actual centre/half-height so NPCs
		on different castle floors do not block one another. */
		float fCenterY = 0.f;
		float fHalfHeight = 100000.f;
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
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
		bool Update_BlockingBody(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			float x,
			float centerY,
			float z) noexcept;
		/* Shared authoritative mover for any upright circular body. Collision
		boxes use the supplied vertical centre/half-height, while dynamic bodies
		use their authored vertical spans and XZ circles. */
		bool Resolve_CircleMove(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			float halfHeight,
			float centerOffsetY,
			float& outX,
			float& outY,
			float& outZ,
			bool& outWasBlocked,
			LostArk::Shared::NET_ENTITY_ID ignoredBodyId =
				LostArk::Shared::INVALID_NET_ENTITY_ID) const;
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
		/* The first intact authoritative wall reached by a charging boss. Keep
		the stable base collision ID and an optional co-located active receiver so
		the caller can try exact impact destruction and then generic contact
		destruction without losing the surface that actually stopped the body. */
		bool Sweep_BossCircleAgainstWalls(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			SERVER_BOSS_WALL_HIT& outHit) const;
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
		static bool Sweep_MovingBodyAgainstBox(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			float halfHeight,
			float centerOffsetY,
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
		/* XZ sweep of one upright body circle against another. A
		start already inside the combined radius only blocks motion toward the
		body's centre, so a mover can always step out of a pre-existing overlap. */
		static bool Sweep_CircleAgainstBody(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			float halfHeight,
			float centerOffsetY,
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
