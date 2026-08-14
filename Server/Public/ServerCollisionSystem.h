#pragma once

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

	class CServerCollisionSystem final
	{
	public:
		bool Initialize(
			const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
			std::string& outStatus);
		bool Is_PlayerSpawnClear(
			const WORLD_BOOTSTRAP_PLACEMENT& spawn) const;
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
		bool Prepare_StateChanges(
			const std::vector<SERVER_COLLISION_STATE_CHANGE>& changes,
			SERVER_COLLISION_STATE_STAGE& outStage,
			std::string& outStatus) const;
		void Commit_StateChanges(SERVER_COLLISION_STATE_STAGE&& stage) noexcept;
		void Reset_RuntimeStates() noexcept;
		bool Has_CollisionBox(const std::string& placementId) const;
		bool Has_CollisionStateTarget(const std::string& stateId) const;
		std::uint64_t Get_Revision() const noexcept { return m_iRevision; }

		[[nodiscard]] std::size_t Get_CollisionBoxCount() const
		{
			return m_CollisionBoxes.size();
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

	private:
		std::vector<WORLD_BOOTSTRAP_PLACEMENT> m_CollisionBoxes;
		std::vector<bool> m_PlayerBlocking;
		std::vector<bool> m_ImpactReceiverEnabled;
		std::uint64_t m_iRevision = 0u;
	};
}
