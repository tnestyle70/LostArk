#pragma once

#include "RoomCommand.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"
#include "WorldBootstrap.h"
#include "GameplayCatalog.h"
#include "ItemCatalog.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerTriggerSystem.h"
#include "SpawnGroupBootstrap.h"
#include "SpawnGroupRuntime.h"
#include "MonsterBrain.h"
#include "ValtanBrain.h"
#include "EncounterPropRuntime.h"
#include "EstherSkillSystem.h"
#include "WorldDestructionBootstrap.h"
#include "WorldDestructionRuntime.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace LostArk::Server
{
	class CClientSession;

	struct SERVER_ROOM_PERFORMANCE_METRICS final
	{
		std::uint64_t iTickCount = 0;
		std::uint64_t iLastTickMicroseconds = 0;
		std::uint64_t iMaximumTickMicroseconds = 0;
		std::size_t iLastIngressDepth = 0;
		std::size_t iIngressHighWatermark = 0;
		std::size_t iLastDrainedCommandCount = 0;
		std::size_t iLastRemainingCommandCount = 0;
		std::uint64_t iDrainLimitedTickCount = 0;
		std::uint64_t iCoalescedMoveCommandCount = 0;
		std::uint64_t iCoalescedAimCommandCount = 0;
		std::uint64_t iDroppedBestEffortCommandCount = 0;
		std::uint64_t iRejectedReliableCommandCount = 0;
		std::uint64_t iRejectedCleanupCommandCount = 0;
		std::uint64_t iSnapshotEncodeCount = 0;
		std::uint64_t iSnapshotEncodeFailureCount = 0;
		std::uint64_t iLastSnapshotEncodeMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEncodeMicroseconds = 0;
		std::uint64_t iSnapshotEnqueueBatchCount = 0;
		std::uint64_t iSnapshotRecipientCount = 0;
		std::uint64_t iSnapshotEnqueueFailureCount = 0;
		std::uint64_t iLastSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iLastMaximumSessionEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSessionEnqueueMicroseconds = 0;
	};

	class CGameRoom final
	{
		friend int Run_ServerGameplayContractTests();
	public:
		explicit CGameRoom(LostArk::Shared::WORLD_ID worldId);

		bool Enqueue(ROOM_COMMAND command);
		void Tick(float fixedDeltaSeconds);
		bool Try_DequeueWorldTransfer(
			SERVER_WORLD_TRANSFER_REQUEST& outTransfer);

		[[nodiscard]] LostArk::Shared::WORLD_ID Get_WorldId() const
		{
			return m_eWorldId;
		}

		[[nodiscard]] bool Is_Ready() const { return m_isReady; }
		[[nodiscard]] const std::string& Get_Status() const
		{
			return m_strStatus;
		}
		[[nodiscard]] SERVER_ROOM_PERFORMANCE_METRICS
			Get_PerformanceMetrics() const;

		// Room thread only. A sealed private arena rejects every later command.
		[[nodiscard]] bool Try_SealPrivateArenaForRetirement();
		// Room thread only. Removes the source player before the target room can
		// process its queued ENTER_WORLD and bind the same session again.
		[[nodiscard]] bool Commit_WorldTransferDeparture(SESSION_ID sessionId);

	private:
		void Handle_Register(const std::shared_ptr<CClientSession>& session);
		bool Join(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld);
		void Leave(
			SESSION_ID sessionId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		void Handle_Move(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_MOVE& move);
		[[nodiscard]] bool Is_BufferableComboAction(
			const SERVER_PLAYER& player) const;
		[[nodiscard]] bool Commit_MoveGoal(
			SERVER_PLAYER& player, float goalX, float goalZ);
		void Commit_PendingPlayerCommand(
			SERVER_PLAYER& player, std::uint32_t actionStartTick);
		void Handle_UseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SKILL& useSkill);
		void Handle_ReleaseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RELEASE_SKILL& releaseSkill);
		void Handle_UpdateSkillAim(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_UPDATE_SKILL_AIM& updateSkillAim);
		void Handle_UseEstherSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ESTHER_SKILL& useEstherSkill);
		bool Spawn_EstherSummon(
			const ESTHER_ROSTER_ENTRY& rosterEntry,
			const SERVER_PLAYER& caster,
			float aimX,
			float aimZ);
		void Handle_RevivePlayer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer);
		void Handle_ChangeCharacterClass(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT Apply_CharacterClassChange(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		void Handle_SpawnWorldEntity(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SPAWN_WORLD_ENTITY& request);
		/* Debug-only. Moves a live Valtan onto an authored health-bar threshold
		so CValtanBrain judges the crossing itself on a later fixed tick. The
		room never starts a pattern, breaks a wall or plays a cue directly.
		Evaluate owns the decision and the boss mutation and is what the contract
		tests drive; Handle only resolves the session and answers it. */
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		void Handle_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request);
		SERVER_WORLD_ENTITY* Find_AuditionBoss();
		bool Has_EngagedAuditionPlayer(const SERVER_WORLD_ENTITY& boss) const;
		bool Reset_ValtanAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
#ifdef _DEBUG
		enum class VALTAN_ORDERED_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			READY,
			WAITING_PATTERN_START,
			WAITING_PATTERN_FINISH,
			PAUSE,
			COMPLETED_HOLD,
			FAILED_HOLD
		};

		struct VALTAN_ORDERED_AUDITION_STATE final
		{
			VALTAN_ORDERED_AUDITION_PHASE ePhase =
				VALTAN_ORDERED_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::size_t iStepIndex = 0u;
			std::uint32_t iRepeatIndex = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iPauseUntilTick = 0u;
			std::uint32_t iHeldBossHp = 0u;
			std::uint32_t iHeldBossHealthBar = 0u;
			std::string strExpectedPatternId;
		};

		bool Stage_ValtanOrderedAuditionStart(
			SESSION_ID sessionId,
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t startTick,
			SERVER_PLAYER& outOwner,
			std::string& status) const;
		bool Start_ValtanOrderedAudition(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t startTick,
			std::string& status);
		void Stop_ValtanOrderedAudition();
		bool Prepare_ValtanOrderedAuditionBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		void Restore_ValtanOrderedAuditionAfterBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
#endif
		// Debug-only. Validates the item against the loaded catalog and
		// stacks it into the player's inventory, capped at maxStack.
		void Handle_DebugGiveItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_GIVE_ITEM& request);
		// Validates the item is owned, is a consumable (iHealPercent > 0), and
		// the player is alive; heals iMaximumHp * iHealPercent / 100, then
		// decrements/removes the stack. HP reaches the Client through the next
		// S2C_WORLD_SNAPSHOT tick like any other HP change; no separate result.
		void Handle_UseItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ITEM& request);
		// Debug Character Select Arena "되돌리기" -- despawns every world entity the
		// debug spawn buttons created in this room (Broadcast_WorldEntityDespawned per
		// entity) and resets the spawn group runtime so the same groups can be
		// re-activated. CHARACTER_SELECT_ARENA only; no-op reply for anything else.
		void Handle_DespawnAllWorldEntities(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DESPAWN_ALL_WORLD_ENTITIES& request);
		// Bern's Valtan-entry confirm window (right-click a guide NPC). Replaces the
		// old automatic changeLevel triggerBox OBB fire: validates the requesting
		// player is still near the named guide NPC world entity, alive, and idle,
		// then stages the same SERVER_WORLD_TRANSFER_REQUEST the trigger used to
		// build. BERN only; no-op for anything else.
		void Handle_ConfirmNpcEntry(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CONFIRM_NPC_ENTRY& request);

		bool Send_Accepted(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		bool Send_EnterRejected(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason);
		bool Send_Spawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		bool Send_WorldEntitySpawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_WORLD_ENTITY& entity);
		bool Send_WorldEntityDespawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId);
		bool Send_WorldEntitySpawnResult(
			const std::shared_ptr<CClientSession>& session,
			const std::string& placementId,
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT result,
			LostArk::Shared::NET_ENTITY_ID netEntityId);
		bool Send_ValtanAuditionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			LostArk::Shared::VALTAN_AUDITION_RESULT result,
			std::uint32_t currentHealthBar);
		bool Send_CharacterClassChangeResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request,
			LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT result,
			LostArk::Shared::CHARACTER_CLASS_ID activeClass);
		// Single-session send: inventory is per-player state, not room-shared
		// like S2C_WORLD_SNAPSHOT, so it never broadcasts.
		bool Send_InventorySnapshot(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t requestSequence,
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				inventory);
		bool Send_Despawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		bool Send_WorldDestructionFullSync(
			const std::shared_ptr<CClientSession>& session);
		// Server-owned collision/navigation counters carried by every
		// destruction message so the Debug audition panel never has to infer
		// passage from the replicated wall states.
		LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
			Build_WorldDestructionDiagnostics() const;
		void Broadcast_Spawned(
			const SERVER_PLAYER& player,
			SESSION_ID exceptSessionId);
		void Broadcast_Despawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		void Broadcast_WorldEntitySpawned(
			const SERVER_WORLD_ENTITY& entity);
		void Broadcast_WorldEntityDespawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId);
		bool Broadcast_WorldDestructionDelta(
			const std::vector<WORLD_DESTRUCTION_STATE_TRANSITION>& transitions,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick);
		void Broadcast_WorldSnapshot();

		std::shared_ptr<CClientSession> Find_Session(
			SESSION_ID sessionId) const;
		void Rollback_Join(SESSION_ID sessionId);
		[[nodiscard]] bool Is_PlayerAdmissionFull() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_AvailablePlayerSpawn() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_Placement(
			const std::string& placementId) const;
		bool Build_WorldEntity(
			const WORLD_BOOTSTRAP_PLACEMENT& placement,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			SERVER_WORLD_ENTITY& outEntity);
		bool Initialize_WorldEntities();
		bool Reset_ReplayableArenaWhenEmpty();
		bool Reset_ValtanArenaWhenEmpty();
		bool Apply_WorldDestructionStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		/* Raise the pillar slots on the authored stage edge of the pattern that
		owns them. The shatter has no identified product owner yet. */
		bool Apply_EncounterPropStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Commit_DueEncounterProps(std::uint32_t serverTick);
		bool Send_EncounterPropSync(
			const std::shared_ptr<CClientSession>& session);
		void Broadcast_EncounterPropSync();
		/* Break whatever the boss body physically reached between its previous
		and current position. No pattern, stage or receiver whitelist gates it;
		only geometry decides, which is what makes an ordinary charge or even a
		walk into a wall bring that one wall down. */
		bool Apply_WorldDestructionBodyContact(
			SERVER_WORLD_ENTITY& boss,
			float previousX,
			float previousY,
			float previousZ,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionPatternHitContact(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionContacts(
			SERVER_WORLD_ENTITY& boss,
			const std::vector<std::string>& contactPlacementIds,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionImpact(
			SERVER_WORLD_ENTITY& boss,
			const std::string& receiverPlacementId,
			std::uint32_t serverTick,
			bool& outTriggered);
		bool Commit_WorldDestructionTransaction(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick,
			std::string& status);
		void Invalidate_DynamicNavigationPaths();
		bool Build_WorldDestructionLiveEvents(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const SERVER_WORLD_ENTITY& boss,
			std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::string& status) const;
		bool Commit_DueWorldDestruction(std::uint32_t serverTick);
		bool Activate_Encounter(const std::string& placementId);
		bool Spawn_Monster(
			const std::string& spawnGroupId,
			const SPAWN_GROUP_ENTRY& entry,
			const SPAWN_GROUP_ANCHOR& anchor,
			const MONSTER_RUNTIME_PROFILE& profile,
			std::uint32_t ordinal);
		std::uint32_t Count_SpawnGroupEntities(
			const std::string& spawnGroupId) const;
		/* 1 unless the player is standing in the stance its identity gauge pays
		for, which is the only thing that changes how fast anyone walks. */
		float Resolve_StanceMoveSpeedScale(const SERVER_PLAYER& player) const;
		/* Hands the living monster and boss bodies to the collision system so this
		tick's player walks and root motion stop at them. */
		void Refresh_PlayerBlockingBodies();
		bool Update_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		void Update_Players(float fixedDeltaSeconds);
		/* Slides a hit player along the armed knockback window, clamped to
		walkable floor and blocking bodies; a wall ends the window early. */
		void Advance_PlayerKnockback(
			SERVER_PLAYER& player, float fixedDeltaSeconds);
		void Update_WorldEntities(float fixedDeltaSeconds);

	private:
		static constexpr std::size_t MAX_INBOUND_COMMAND_COUNT = 1024u;
		// Best-effort traffic leaves room for gameplay/control commands. The
		// final 64 slots stay available to LEAVE and failed-entry rollback.
		static constexpr std::size_t MAX_BEST_EFFORT_COMMAND_COUNT = 768u;
		static constexpr std::size_t MAX_RELIABLE_COMMAND_COUNT = 960u;
		static constexpr std::size_t MAX_COMMANDS_DRAINED_PER_TICK = 256u;

		mutable std::mutex m_CommandMutex;
		std::deque<ROOM_COMMAND> m_InboundCommands;
		SERVER_ROOM_PERFORMANCE_METRICS m_PerformanceMetrics;
		SERVER_ROOM_PERFORMANCE_METRICS m_LastRoomPerfLogSample;
		std::uint64_t m_iLastRoomPerfSnapshotDroppedCount = 0;
		std::uint64_t m_iLastRoomPerfReliableRejectedCount = 0;
		std::uint64_t m_iLastRoomPerfWireSendFailureCount = 0;
		std::size_t m_iLastRoomPerfOutboundHighWatermark = 0u;
		bool m_acceptsCommands = true;
		std::deque<SERVER_WORLD_TRANSFER_REQUEST> m_PendingWorldTransfers;

		std::unordered_map<SESSION_ID, std::weak_ptr<CClientSession>> m_Sessions;
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER> m_Players;
		std::unordered_map<SESSION_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdBySessionId;
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdByEntityId;

		LostArk::Shared::WORLD_ID m_eWorldId = LostArk::Shared::WORLD_ID::END;
		CWorldBootstrap m_WorldBootstrap;
		CGameplayCatalog m_GameplayCatalog;
		CItemCatalog m_ItemCatalog;
		CServerNavigation m_ServerNavigation;
		CServerCollisionSystem m_ServerCollisionSystem;
		CServerTriggerSystem m_ServerTriggerSystem;
		CSpawnGroupBootstrap m_SpawnGroupBootstrap;
		CSpawnGroupRuntime m_SpawnGroupRuntime;
		CPlayerSkillSystem m_PlayerSkillSystem;
		CMonsterBrain m_MonsterBrain;
		CValtanBrain m_ValtanBrain;
		CEstherSkillSystem m_EstherSkillSystem;
		CWorldDestructionBootstrap m_WorldDestructionBootstrap;
		CWorldDestructionRuntime m_WorldDestructionRuntime;
		/* The four pillars come back four times in one fight, so they live in a
		reversible prop runtime instead of a one-way destruction group. */
		CEncounterPropRuntime m_EncounterPropRuntime;
		/* Debug audition only: the tick a whole pillar cycle shatters on, and
		the flag the next raise turns into that tick. No product trigger for the
		shatter is identified yet, so nothing else writes these. */
		std::uint32_t m_iPillarAuditionBreakTick = 0u;
		bool m_bPillarAuditionCycleArmed = false;
		std::vector<SERVER_WORLD_ENTITY> m_WorldEntities;
		/* One tick's resolved hits. Cleared at the top of every simulation phase
		and consumed by Broadcast_WorldSnapshot, so an event can only ever ride
		the snapshot of the tick that produced it. */
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_TickDamageEvents;
		std::string m_strStatus;
		bool m_isReady = false;

		LostArk::Shared::PLAYER_ID m_iNextPlayerId = 1;
		LostArk::Shared::NET_ENTITY_ID m_iNextNetEntityId = 100;
		std::uint32_t m_iServerTick = 0;
		std::uint64_t m_iNextWorldDestructionEventSequence = 1u;
		/* Debug Valtan audition. The armed bar is the one an ARM parked the boss
		above; a CROSS is only honoured for that same bar, so a crossing can
		never span an unknown number of authored thresholds. Both reset with the
		encounter, and the handled sequences reject a resent request instead of
		replaying it. */
		std::uint32_t m_iValtanAuditionArmedHealthBar = 0;
		std::unordered_map<SESSION_ID, std::uint32_t>
			m_ValtanAuditionSequenceBySessionId;
#ifdef _DEBUG
		VALTAN_ORDERED_AUDITION_STATE m_ValtanOrderedAudition;
#endif
	};
}
