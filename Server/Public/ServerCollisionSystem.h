#pragma once

#include "ServerPlayer.h"
#include "WorldBootstrap.h"

#include <cstddef>
#include <string>
#include <vector>

namespace LostArk::Server
{
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

	private:
		std::vector<WORLD_BOOTSTRAP_PLACEMENT> m_CollisionBoxes;
	};
}
