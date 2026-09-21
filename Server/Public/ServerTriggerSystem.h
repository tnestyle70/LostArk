#pragma once

#include "ServerPlayer.h"
#include "WorldBootstrap.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
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
		// Worn honor title, re-applied by the target room's admission.
		LostArk::Shared::HONOR_TITLE_ID iHonorTitleId = LostArk::Shared::INVALID_HONOR_TITLE_ID;
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

	/* One player's view of one interact-gated box changing. The room turns
	   these into the prompt the Client draws; nothing else consumes them. */
	struct SERVER_INTERACT_PROMPT_EDGE final
	{
		LostArk::Shared::PLAYER_ID iPlayerId = 0;
		std::string strTriggerPlacementId;
		bool bAvailable = false;
	};

	/* The room may own a specific move entry's interruption/admission policy.
	   A rejected owned entry must remain eligible while the player overlaps. */
	enum class SERVER_TRIGGER_MOVE_ENTRY_RESULT
	{
		USE_DEFAULT,
		STARTED,
		RETRY_WHILE_INSIDE
	};

	using SERVER_TRIGGER_MOVE_ENTRY_HANDLER = std::function<SERVER_TRIGGER_MOVE_ENTRY_RESULT(
		const WORLD_BOOTSTRAP_PLACEMENT&, SERVER_PLAYER&, std::uint32_t)>;

	/* Debug F1 "Normal Monster 1/2" pairs one wave-monster button of one world with
	   the trigger box that used to raise the wave and the spawn group it raised.
	   This table is the only place that pairing lives: a Debug room reads it to
	   keep the box quiet, and the room's re-summon handler reads it to find the
	   group the button owns. */
	struct WAVE_MONSTER_BUTTON_ROW final
	{
		LostArk::Shared::WORLD_ID eWorld;
		LostArk::Shared::WAVE_MONSTER_BUTTON eButton;
		const char* pTriggerPlacementId;
		const char* pSpawnGroupId;
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
				const std::string&)>& activateTarget,
			std::vector<SERVER_INTERACT_PROMPT_EDGE>& outPromptEdges,
			const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry = {});
		/* Runs one G-only box for one player (a box that does not fire on entry,
		   see Fires_OnEntry). False means the request named a box that does not
		   exist, fires on entry instead, is spent, that this player is no longer
		   standing in, or a press inside the per-player debounce -- the caller
		   changes nothing. moveEntry is the room's own admission for a movement box
		   (a Kouku Mario lane), the same handler Evaluate_Entries uses, so a G press
		   cannot bypass it. */
		bool Activate_Interact(
			LostArk::Shared::PLAYER_ID playerId,
			const std::string& triggerPlacementId,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			std::uint32_t actionStartTick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
				const std::string&)>& activateTarget,
			const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry = {});
		/* G pressed with no offer standing: runs every G-only trigger box this
		   player is inside, checked against the Server's own position. Boxes that
		   fire on entry are not touched. Returns how many boxes were used. Zero --
		   nobody inside, dead, or a press inside the per-player debounce -- changes
		   nothing. */
		std::uint32_t Activate_Here(
			LostArk::Shared::PLAYER_ID playerId,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			std::uint32_t actionStartTick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
				const std::string&)>& activateTarget,
			const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry = {});
		/* Which world's rules Fires_OnEntry applies (AUTO_ENTRY_RULES). Set before
		   Initialize; a system that never learns its world only fires the
		   world-independent scripted kinds on entry. */
		void Set_WorldId(const LostArk::Shared::WORLD_ID worldId) { m_eWorldId = worldId; }
		/* One line per fired trigger: id, player, ENTER or KEY. The room owns
		   where it goes so the trigger system stays free of I/O. */
		void Set_FireLog(std::function<void(const std::string&)> sink)
		{
			m_FireLog = std::move(sink);
		}
		/* Floor height under an XZ point, supplied by the room from its navigation. When
		   unset, or when it refuses the point, the Valtan boss start lands the player at
		   the authored entrance box height instead. */
		void Set_GroundSampler(std::function<bool(float, float, float&)> sampler)
		{
			m_GroundSampler = std::move(sampler);
		}
		/* Product triggers are always repeatable: Initialize clears the authored
		   isTriggerOnce so no room-wide latch can spend a box for everyone else.
		   A test that verifies the latch mechanism itself opts back in, before
		   Initialize. */
		void Set_HonourTriggerOnce(const bool honour) { m_bHonourTriggerOnce = honour; }
		/* Debug rooms hand the four wave-monster boxes (WAVE_MONSTER_BUTTON_ROW) to the
		   F1 "Normal Monster 1/2" buttons: stepping in neither fires them nor offers
		   G, and G cannot run them. Release rooms never call this, so the flag stays
		   false and every box behaves exactly as authored. Every other box, and
		   Debug_Activate, are untouched either way. */
		void Set_SuppressWaveMonsterTriggers(const bool suppress) { m_bSuppressWaveMonsterTriggers = suppress; }
		/* The row a (world, button) pair owns, or null when that world has no such
		   button. */
		static const WAVE_MONSTER_BUTTON_ROW* Find_WaveMonsterButton(
			LostArk::Shared::WORLD_ID worldId,
			LostArk::Shared::WAVE_MONSTER_BUTTON button);
		/* True only for one of the table's boxes in its own world whose single
		   action is that row's spawn group activation; a box that merely shares an
		   id but authors something else is not one. */
		static bool Is_WaveMonsterTrigger(
			LostArk::Shared::WORLD_ID worldId,
			const WORLD_BOOTSTRAP_PLACEMENT& placement);
		/* Minimum server ticks between two G activations by one player (0.3 s at
		   30 Hz), so mashing the key cannot flood the room. */
		static constexpr std::uint32_t KEY_ACTIVATION_DEBOUNCE_TICKS = 9u;
		void Remove_Player(LostArk::Shared::PLAYER_ID playerId);
		void Reset_SequenceActivation(const std::string& instanceId);
		LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT Debug_Activate(
			LostArk::Shared::PLAYER_ID playerId, const std::string& triggerId, bool replay,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players, std::uint32_t tick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& transfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND, const std::string&)>& activateTarget);
		static bool Contains_Placement(
			const WORLD_BOOTSTRAP_PLACEMENT& box, const SERVER_PLAYER& player);
		/* Entry for a Server-validated scripted displacement. Authored triggers
		and Debug jump admission share the same motion and snapshot state. */
		static bool Begin_MovePlayer(
			SERVER_PLAYER& player,
			const WORLD_TRIGGER_ACTION& action,
			std::uint32_t actionStartTick);
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
			/* Inert unless Set_HonourTriggerOnce kept isTriggerOnce at load. */
			bool hasFired = false;
		};

		static bool Contains(
			const RUNTIME_TRIGGER& trigger,
			const SERVER_PLAYER& player);
#ifdef _DEBUG
		static bool Build_ValtanStageBypassMove(
			const std::string& triggerPlacementId,
			WORLD_TRIGGER_ACTION& outAction);
#endif
		/* The one place an authored action turns into Server state, shared by
		   entry and by an interact request so the two cannot drift. */
		bool Run_Action(
			RUNTIME_TRIGGER& trigger,
			SERVER_PLAYER& player,
			std::uint32_t actionStartTick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
				const std::string&)>& activateTarget) const;
		/* Valtan boss start: sends the player to the centre of the enabled
		   Stage_Boss_ArenaEntry box (false when there is none or the player is busy). */
		bool Place_PlayerAtValtanArenaEntry(
			SERVER_PLAYER& player,
			std::uint32_t actionStartTick) const;
		void Log_Fire(
			const RUNTIME_TRIGGER& trigger,
			LostArk::Shared::PLAYER_ID playerId,
			const char* pSource) const;
		/* True when stepping into this box fires it: an authored gate always says
		   no, otherwise AUTO_ENTRY_RULES (top of the .cpp) decides by world and
		   action kind. Every other box only offers itself and waits for G. */
		bool Fires_OnEntry(const RUNTIME_TRIGGER& trigger) const;
		/* True for a wave-monster box in a room that handed it to the F1 buttons. */
		bool Is_WaveMonsterSuppressed(const RUNTIME_TRIGGER& trigger) const;
		/* What a G press runs for one box: the authored action, or in Debug the
		   Valtan corridor shortcut. Stepping in never reaches this. */
		bool Run_Trigger(
			RUNTIME_TRIGGER& trigger,
			SERVER_PLAYER& player,
			std::uint32_t actionStartTick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
				const std::string&)>& activateTarget) const;
		/* The G-press wrapper around Run_Trigger: a movement box the room owns (a
		   Kouku Mario lane) goes through the same admission handler stepping in
		   would use; every other box runs Run_Trigger unchanged. */
		bool Run_KeyTrigger(
			RUNTIME_TRIGGER& trigger,
			SERVER_PLAYER& player,
			std::uint32_t actionStartTick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
				const std::string&)>& activateTarget,
			const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry) const;
		bool Is_KeyDebounced(
			LostArk::Shared::PLAYER_ID playerId,
			std::uint32_t tick) const;
		static bool Build_WorldTransfer(
			const SERVER_PLAYER& player,
			const WORLD_TRIGGER_ACTION& action,
			SERVER_WORLD_TRANSFER_REQUEST& outTransfer);

	private:
		std::vector<RUNTIME_TRIGGER> m_Triggers;
		bool m_bDebugValtanStageBypass = false;
		bool m_bHonourTriggerOnce = false;
		bool m_bSuppressWaveMonsterTriggers = false;
		LostArk::Shared::WORLD_ID m_eWorldId = LostArk::Shared::WORLD_ID::END;
		/* Tick of each player's last accepted G activation (debounce). */
		std::unordered_map<LostArk::Shared::PLAYER_ID, std::uint32_t> m_LastKeyActivationTick;
		/* Bern only: players seen in a TRIGGER_MOVE, so the tick they land is known (see
		Evaluate_Entries). */
		std::unordered_set<LostArk::Shared::PLAYER_ID> m_TriggerMoveInFlight;
		std::function<void(const std::string&)> m_FireLog;
		std::function<bool(float, float, float&)> m_GroundSampler;
	};
}
