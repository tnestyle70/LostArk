#pragma once

#include "ServerPlayer.h"
#include "WorldBootstrap.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

namespace LostArk::Server
{
	class CServerTriggerSystem final
	{
	public:
		bool Initialize(
			const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
			std::string& outStatus);
		bool Update_PlayerMotion(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds) const;
		void Evaluate_Entries(
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			std::uint32_t actionStartTick);
		void Remove_Player(LostArk::Shared::PLAYER_ID playerId);

		[[nodiscard]] std::size_t Get_TriggerCount() const
		{
			return m_Triggers.size();
		}

	private:
		struct RUNTIME_TRIGGER
		{
			WORLD_BOOTSTRAP_PLACEMENT Definition;
			std::unordered_set<LostArk::Shared::PLAYER_ID> PlayersInside;
			bool hasFired = false;
		};

		static bool Contains(
			const RUNTIME_TRIGGER& trigger,
			const SERVER_PLAYER& player);
		static bool Begin_MovePlayer(
			SERVER_PLAYER& player,
			const WORLD_TRIGGER_ACTION& action,
			std::uint32_t actionStartTick);

	private:
		std::vector<RUNTIME_TRIGGER> m_Triggers;
	};
}
