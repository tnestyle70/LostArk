#pragma once

#include "ServerPlayer.h"
#include "WorldBootstrap.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_WORLD_TRANSFER_REQUEST final
	{
		SESSION_ID iSessionId = INVALID_SESSION_ID;
		LostArk::Shared::WORLD_ID eTargetWorldId =
			LostArk::Shared::WORLD_ID::END;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::string strNickName;
		/* One immutable leader-first batch, not independent transfers. The
		   room thread stages all target admissions before any source departure. */
		std::vector<SESSION_ID> PartyBatchSessionIds;
		std::uint32_t iPartyRequestSequence = 0u;
		/* Empty picks the target world's usual free PLAYER_SPAWN placement
		(Stage_PlayerEntry's default). Non-empty names ANY placement id in the
		target world's own bootstrap (of any kind, not just PLAYER_SPAWN) whose
		position/yaw is used directly instead -- e.g. Valtan's own "돌아가기"
		return trip lands the player next to Bern's Valtan-entry guide NPC
		rather than a generic spawn point. */
		std::string strSpawnPlacementOverrideId;
		/* Empty means "grant the default fresh-entry loadout" (Stage_PlayerEntry's
		3 starting potions), same as any other world entry. Non-empty replaces
		that grant with these exact items -- Handle_ReturnToBern populates this
		from the departing player's live Valtan inventory so clear rewards
		survive the "돌아가기" trip back to Bern instead of being silently reset. */
		std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT> CarriedInventory;
	};

	class CServerTriggerSystem final
	{
	public:
		bool Initialize(
			const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
			std::string& outStatus,
			bool enableDebugValtanStageBypass = false);
		bool Update_PlayerMotion(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds) const;
		void Evaluate_Entries(
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			std::uint32_t actionStartTick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
				const std::string&)>& activateTarget);
		void Remove_Player(LostArk::Shared::PLAYER_ID playerId);
#ifdef _DEBUG
		bool Place_PlayerAtValtanAuditionBait(
			SERVER_PLAYER& player,
			std::uint32_t actionStartTick) const;
#endif

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
#ifdef _DEBUG
		static bool Build_ValtanStageBypassMove(
			const std::string& triggerPlacementId,
			WORLD_TRIGGER_ACTION& outAction);
#endif
		static bool Build_WorldTransfer(
			const SERVER_PLAYER& player,
			const WORLD_TRIGGER_ACTION& action,
			SERVER_WORLD_TRANSFER_REQUEST& outTransfer);

	private:
		std::vector<RUNTIME_TRIGGER> m_Triggers;
		bool m_bDebugValtanStageBypass = false;
	};
}
