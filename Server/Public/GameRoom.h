#pragma once

#include "RoomCommand.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"
#include "WorldBootstrap.h"
#include "GameplayCatalog.h"
#include "ItemCatalog.h"
#include "PlayerSkillSystem.h"
#include "CombatObjectRuntime.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerTriggerSystem.h"
#include "SpawnGroupBootstrap.h"
#include "SpawnGroupRuntime.h"
#include "MonsterBrain.h"
#include "NpcBehaviorRuntime.h"
#include "ValtanBrain.h"
#include "EncounterPropRuntime.h"
#include "EstherSkillSystem.h"
#include "WorldDestructionBootstrap.h"
#include "WorldDestructionRuntime.h"
#include "Network/PacketFrame.h"
#include "Network/SessionDiagnostic.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace LostArk::Server
{
	class CClientSession;

	/* A room never mutates an admitted gameplay catalog. The facade preserves
	   the established lookup surface while retaining immutable old generations
	   until every replicated occurrence releases its revision pin. */
	class CGameplayCatalogGenerations final
	{
	public:
		static constexpr std::size_t MAX_GENERATION_COUNT = 16u;

		CGameplayCatalogGenerations();
		bool Load();
		bool Initialize(
			const std::shared_ptr<const CGameplayCatalog>& initialGeneration);
		bool Stage(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);
		bool Commit(std::uint32_t transactionSequence) noexcept;
		void Abort(std::uint32_t transactionSequence) noexcept;
		void Collect_Garbage(
			const std::vector<LostArk::Shared::GameplayDataRevision>& livePins);

		[[nodiscard]] const CGameplayCatalog* Resolve(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept;
		[[nodiscard]] const CGameplayCatalog& Active() const noexcept;
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGeneration() const noexcept { return m_pActiveGeneration; }
		[[nodiscard]] std::size_t Get_GenerationCount() const noexcept
		{
			return m_Generations.size();
		}
		[[nodiscard]] std::uint16_t Get_ActiveGenerationEpoch() const noexcept
		{
			return m_iActiveGenerationEpoch;
		}

		const PLAYER_SKILL_DEFINITION* Find_Skill(
			LostArk::Shared::SKILL_ID skillId) const;
		const BOSS_RUNTIME_PROFILE* Find_Boss(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PART_DEFINITION>* Find_BossParts(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PATTERN_DEFINITION>* Find_BossPatterns(
			const std::string& encounterId) const;
		const BOSS_COMBAT_OBJECT_DEFINITION* Find_BossCombatObject(
			const std::string& archetypeId) const;
		const VALTAN_TIMELINE_DEFINITION* Find_ValtanTimeline(
			const std::string& encounterId) const;
		const VALTAN_TIMELINE_ROW* Find_ValtanTimelineRow(
			const std::string& encounterId, std::uint32_t commandId) const;
		const BOSS_PATTERN_ROTATION_DEFINITION* Find_BossPatternRotation(
			const std::string& encounterId, std::uint32_t gameplayPhase,
			std::uint32_t healthBar) const;
		const std::string& Find_IntroPatternId(
			const std::string& encounterId) const;
		const PLAYER_RUNTIME_PROFILE* Find_Player(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;
		std::uint32_t Find_DamageRatePercent(
			const std::string& damageProfileId) const;
		[[nodiscard]] const LostArk::Shared::GameplayDataRevision&
			Get_ActiveRevision() const noexcept;
		[[nodiscard]] const std::string& Get_Status() const noexcept;
		operator const CGameplayCatalog&() const noexcept { return Active(); }

	private:
		std::shared_ptr<const CGameplayCatalog> m_pActiveGeneration;
		std::shared_ptr<const CGameplayCatalog> m_pStagedGeneration;
		std::uint32_t m_iStagedTransactionSequence = 0u;
		std::uint16_t m_iActiveGenerationEpoch = 0u;
		std::vector<std::shared_ptr<const CGameplayCatalog>> m_Generations;
		std::string m_strStatus;
	};

	struct SERVER_ROOM_PERFORMANCE_METRICS final
	{
		std::uint64_t iTickCount = 0;
		std::uint64_t iLastTickMicroseconds = 0;
		std::uint64_t iMaximumTickMicroseconds = 0;
		std::size_t iLastIngressDepth = 0;
		std::size_t iIngressHighWatermark = 0;
		std::size_t iLastDrainedCommandCount = 0;
		std::size_t iLastRemainingCommandCount = 0;
		std::size_t iLastCleanupIngressDepth = 0;
		std::size_t iCleanupIngressHighWatermark = 0;
		std::size_t iLastDrainedCleanupCommandCount = 0;
		std::size_t iLastRemainingCleanupCommandCount = 0;
		std::uint64_t iDrainLimitedTickCount = 0;
		std::uint64_t iCoalescedMoveCommandCount = 0;
		std::uint64_t iCoalescedAimCommandCount = 0;
		std::uint64_t iDroppedBestEffortCommandCount = 0;
		std::uint64_t iRejectedReliableCommandCount = 0;
		std::uint64_t iRejectedCleanupCommandCount = 0;
		std::uint64_t iDeduplicatedCleanupCommandCount = 0;
		std::uint64_t iCancelledCommandCountByCleanup = 0;
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
		friend int Run_ServerGameplayContractTests(bool);
	public:
		explicit CGameRoom(
			LostArk::Shared::WORLD_ID worldId,
			std::shared_ptr<const CGameplayCatalog> initialGameplayGeneration = {});

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
		/* Room-thread only. Stage is allowed to fail before publication; Commit
		   is a bounded pointer swap after every process room has staged. */
		bool Stage_GameplayGeneration(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);
		bool Commit_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;
		void Abort_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGameplayGeneration() const noexcept
		{
			return m_GameplayCatalog.Get_ActiveGeneration();
		}
		[[nodiscard]] const CGameplayCatalog* Resolve_GameplayGeneration(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept
		{
			return m_GameplayCatalog.Resolve(revision);
		}
		/* Room-thread only. Decision observability reads the same authoritative
		   brain and immutable selector generation as the Valtan simulation. */
		bool Build_ValtanDecisionTraceResponse(
			const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request,
			LostArk::Shared::S2C_VALTAN_DECISION_TRACE_RESPONSE& outResponse,
			std::string& status) const;
		[[nodiscard]] SERVER_ROOM_PERFORMANCE_METRICS
			Get_PerformanceMetrics() const;

		// Room thread only. A sealed private arena rejects every later command.
		[[nodiscard]] bool Try_SealPrivateArenaForRetirement();
		// Room thread only. Removes the source player before the target room can
		// process its queued ENTER_WORLD and bind the same session again.
		[[nodiscard]] bool Commit_WorldTransferDeparture(SESSION_ID sessionId);
		// Room-thread only, while ServerApp holds its session-binding mutex.
		bool Transfer_PartyTo(CGameRoom& target,
			const std::vector<SESSION_ID>& leaderFirstSessionIds,
			LostArk::Shared::PARTY_TRANSFER_RESULT& outResult, std::string& status);
		void Notify_PartyTransferFailure(SESSION_ID sessionId,
			std::uint32_t requestSequence, LostArk::Shared::WORLD_ID targetWorldId,
			LostArk::Shared::PARTY_TRANSFER_RESULT result);

	private:
		struct STAGED_PLAYER_ENTRY final
		{
			std::shared_ptr<CClientSession> pSession;
			SERVER_PLAYER Player;
			std::vector<LostArk::Shared::PACKET_FRAME> Frames;
		};
		bool Stage_PlayerEntry(const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
			std::span<const STAGED_PLAYER_ENTRY> precedingEntries,
			STAGED_PLAYER_ENTRY& staged,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outReason, std::string& status);
		bool Build_PlayerEntryFrames(STAGED_PLAYER_ENTRY& entry,
			std::span<const STAGED_PLAYER_ENTRY> batch, std::string& status);
		void Commit_PlayerEntry(const STAGED_PLAYER_ENTRY& entry);
		void Flush_PartyTransferResults();
		void Handle_Register(const std::shared_ptr<CClientSession>& session);
		bool Join(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld);
		void Leave(
			SESSION_ID sessionId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason, bool publishDeparture = true);
		void Close_SessionForBindingFailure(
			SESSION_ID sessionId,
			std::string_view packetName,
			std::string_view validation);
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
			float positionX,
			float positionY,
			float positionZ,
			float yawDegrees);
		void Update_PendingEstherSummons(float fixedDeltaSeconds);
		void Handle_RevivePlayer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer);
		/* Debug/Development-build test aid only -- zeroes the caster's own HP and
		sets PLAYER_ACTION_STATE::DEAD so a death-screen tester does not have to
		survive a real hit. Real body is compiled out in Release, matching
		Evaluate_ValtanAudition's convention. */
		void Handle_DebugKillSelf(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_KILL_SELF& debugKillSelf);
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
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStart(
				SESSION_ID sessionId,
				const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStopAfterCurrent(
				SESSION_ID sessionId,
				const LostArk::Shared::
					C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		void Handle_ValtanPatternFlowStart(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request);
		void Handle_ValtanPatternFlowStopAfterCurrent(
			SESSION_ID sessionId,
			const LostArk::Shared::
				C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request);
		SERVER_WORLD_ENTITY* Find_AuditionBoss();
		SERVER_WORLD_ENTITY* Find_AuditionBoss(
			const std::string& placementId);
		bool Has_EngagedAuditionPlayer(const SERVER_WORLD_ENTITY& boss) const;
		bool Build_ValtanBossOnlyAuditionReset(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			SERVER_WORLD_ENTITY& outBoss,
			std::string& status);
		bool Reset_ValtanBossOnlyAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		bool Reset_ValtanAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
#ifdef _DEBUG
		enum class VALTAN_PATTERN_ID_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE,
			COMPLETED_HOLD,
			IDLE_HOLD
		};

		struct VALTAN_PATTERN_ID_AUDITION_STATE final
		{
			VALTAN_PATTERN_ID_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0u;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bResetlessContinuation = false;
			bool bReportedWaitingForPlayer = false;
			// A live predecessor has no Client Play request to report a lifecycle for.
			bool bAdoptedLivePredecessor = false;
			// Keep only the current Flow occurrence on its existing ordered Brain path.
			std::optional<BOSS_PATTERN_SEQUENCE_DEFINITION> AdoptedFlowSequence;
		};

		struct VALTAN_NEXT_PATTERN_RESERVATION final
		{
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iPredecessorPatternSequence = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bReportedWaitingForPlayer = false;
		};

		struct VALTAN_NEXT_PATTERN_COMMAND_RECEIPT final
		{
			LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
			LostArk::Shared::VALTAN_AUDITION_RESULT Result =
				LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			std::uint32_t iCurrentHealthBar = 0u;
		};

		[[nodiscard]] bool Is_ValtanPatternIdAuditionRunning() const noexcept;
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanNextPatternControl(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		LostArk::Shared::VALTAN_AUDITION_RESULT Adopt_ValtanLiveNextPattern(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			SERVER_WORLD_ENTITY& boss);
		void Cancel_ValtanNextPatternReservation(std::string reason);
		void Cancel_ValtanPatternIdAudition(std::string reason);
		void Try_PromoteValtanNextPattern(SERVER_WORLD_ENTITY& boss);
		bool Prepare_ValtanPatternIdAuditionBeforeBrain(SERVER_WORLD_ENTITY& boss);
		bool Refresh_ValtanPatternIdAuditionState();
		void Queue_ValtanAuditionLifecycle(
			SESSION_ID ownerSessionId,
			std::uint32_t requestSequence,
			std::uint32_t roomEpoch,
			std::uint32_t patternSequence,
			const std::string& patternId,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanNextPatternLifecycle(
			const VALTAN_NEXT_PATTERN_RESERVATION& reservation,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		bool Flush_ValtanPatternIdAuditionLifecycle();

		enum class VALTAN_PATTERN_FLOW_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE
		};

		struct VALTAN_PATTERN_FLOW_AUDITION_STATE final
		{
			VALTAN_PATTERN_FLOW_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_FLOW_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::uint32_t iFirstPatternSequence = 0u;
			std::size_t iStartSlotIndex = 0u;
			std::size_t iReportedSequenceIndex =
				(static_cast<std::size_t>(-1));
			std::uint32_t iReportedPatternSequence = 0u;
			bool bReportedPausedForRevive = false;
			bool bStopAfterCurrent = false;
			std::string strBossPlacementId;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strStartSlotId;
			std::vector<LostArk::Shared::VALTAN_PATTERN_FLOW_SLOT_WIRE> Slots;
			BOSS_PATTERN_SEQUENCE_DEFINITION Sequence;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
		};

		[[nodiscard]] bool Is_ValtanPatternFlowRunning() const noexcept;
		[[nodiscard]] const BOSS_PATTERN_SEQUENCE_DEFINITION*
			Resolve_ValtanPatternFlowSequence(
				const SERVER_WORLD_ENTITY& boss) const noexcept;
		void Refresh_ValtanPatternFlowState(SERVER_WORLD_ENTITY& boss);
		void Finish_ValtanPatternFlow(
			SERVER_WORLD_ENTITY& boss,
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE terminalState,
			std::string reason = {});
		void Abort_ValtanPatternFlowForOwner(
			SESSION_ID sessionId,
			std::string reason);
		void Queue_ValtanPatternFlowLifecycle(
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE state,
			const SERVER_WORLD_ENTITY* boss,
			std::string reason = {});
		bool Flush_ValtanPatternFlowLifecycle();

		enum class VALTAN_TIMELINE_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			WAITING_ENVIRONMENT,
			READY,
			WAITING_PATTERN_START,
			WAITING_PATTERN_FINISH,
			COMPLETED_HOLD,
			FAILED_HOLD
		};

		struct VALTAN_TIMELINE_AUDITION_STATE final
		{
			VALTAN_TIMELINE_AUDITION_PHASE ePhase =
				VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::size_t iRowIndex = 0u;
			std::size_t iActionIndex = 0u;
			std::uint32_t iRepeatIndex = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::uint32_t iHeldBossHp = 0u;
			std::uint32_t iHeldBossHealthBar = 0u;
			bool bAllowProductPropBreak = false;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			std::string strExpectedPatternId;
			std::vector<std::string> ExpectedGoneGroupIds;
		};

		/* A page start differs from a one-row timeline audition: it stages the
		already-destroyed arena, releases the real Brain at that page boundary,
		and then leaves the encounter running normally. */
		struct VALTAN_FIGHT_PAGE_START_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iCommandId = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::vector<std::string> ExpectedGoneGroupIds;

			bool Is_Active() const noexcept
			{
				return LostArk::Shared::INVALID_NET_ENTITY_ID != iBossEntityId;
			}
		};

		bool Prepare_ValtanTimelineArenaState(
			const CWorldDestructionRuntime& runtime,
			const SERVER_WORLD_ENTITY& boss,
			VALTAN_TIMELINE_ARENA_STATE arenaState,
			std::uint32_t requestTick,
			WORLD_DESTRUCTION_TRANSACTION& outTransaction,
			std::vector<std::string>& outExpectedGoneGroupIds,
			std::string& status) const;
		bool Stage_ValtanTimelineRowStart(
			SESSION_ID sessionId,
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			SERVER_PLAYER& outOwner,
			std::string& status) const;
		bool Start_ValtanTimelineRow(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Stop_ValtanTimelineRow(bool resetEncounter = false);
		bool Prepare_ValtanTimelineRowBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		void Restore_ValtanTimelineRowAfterBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		bool Start_ValtanFightPage(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Prepare_ValtanFightPageBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
#endif
		struct VALTAN_DECISION_TRACE_REVISION_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::string strBossPlacementId;
			std::uint64_t iTraceSequence = 0u;
			LostArk::Shared::GameplayDataRevision DefinitionRevision{};
		};
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
		/* Same-room-only: request.iTargetNetEntityId must resolve to a real
		   player currently in this room's m_PlayerIdByEntityId. There is no
		   cross-room player identity yet (nickname is display text only, see
		   CLAUDE.md), so an invite naming a player in a different room or a
		   stale/unknown NetEntityId is rejected, not queued. */
		void Handle_PartyInvite(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE& request);
		void Handle_PartyInviteRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE_RESPOND& request);
		void Broadcast_PartyRoster(std::uint32_t partyId);
		/* Leave() calls this so a disconnecting player does not linger as a
		   ghost roster entry for whoever they partied with. */
		void Remove_FromParty(LostArk::Shared::PLAYER_ID playerId);
		/* Same room-scoped broadcast Broadcast_PartyRoster already uses --
		   every current session in this room receives the relayed line,
		   including the sender (its own head bubble is driven off the same
		   S2C_CHAT the rest of the room gets, not a second local-only path). */
		void Handle_Chat(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHAT& request);

		bool Send_Accepted(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		bool Send_EnterRejected(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason);
		bool Send_Spawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		static bool Build_WorldEntitySpawnedPayload(
			const SERVER_WORLD_ENTITY& entity,
			std::vector<std::uint8_t>& outPayload);
		bool Send_WorldEntitySpawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_WORLD_ENTITY& entity);
		bool Send_WorldEntityDespawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		bool Send_CombatObjectSpawned(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned);
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
		bool Send_ValtanPatternFlowResult(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t commandSequence,
			LostArk::Shared::VALTAN_PATTERN_FLOW_COMMAND command,
			LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT result,
			const std::string& flowId,
			const std::string& flowRevision,
			std::uint32_t roomFlowEpoch,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			const std::string& reason);
		bool Build_RequiredPinnedGameplayRevisions(
			std::vector<LostArk::Shared::GameplayDataRevision>&
				outRevisions) const;
		[[nodiscard]] const CGameplayCatalog* Resolve_ValtanGameplayCatalog(
			const SERVER_WORLD_ENTITY& boss) const noexcept;
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
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
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
			SERVER_WORLD_ENTITY& outEntity,
			const CGameplayCatalog* definitionCatalog = nullptr,
			LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID);
		bool Initialize_WorldEntities();
		bool Reset_ReplayableArenaWhenEmpty();
		bool Reset_ValtanArenaWhenEmpty();
		bool Apply_BossPatternStageActions(
			SERVER_WORLD_ENTITY& boss,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u);
		bool Apply_BossPatternScheduledSpawnWave(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_BossPatternStageTransition(
			SERVER_WORLD_ENTITY& boss,
			const std::string& previousPatternId,
			const std::string& previousActionId,
			const std::string& nextPatternId,
			const std::string& nextActionId,
			const LostArk::Shared::GameplayDataRevision&
				previousDefinitionRevision,
			const LostArk::Shared::GameplayDataRevision& nextDefinitionRevision,
			std::uint32_t serverTick);
		bool Stage_BossPatternStageActions(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			SERVER_BOSS_COMBAT_STATE& stagedCombat,
			std::uint8_t& stagedGameplayPhase,
			SERVER_COMBAT_OBJECT_TRANSACTION& combatObjectTransaction,
			std::uint32_t spawnWaveOrdinal = 0u);
		/* Runs only after every stage-action preflight transaction commits. These
		actions own player/target state and therefore cannot be staged inside the
		boss-combat or combat-object value transactions above. */
		bool Prepare_GrabbedPlayerImpact(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& stagedPlayers,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& stagedDamageEvents);
		bool Commit_BossPatternPlayerStageActions(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u);
		bool Resolve_ArenaRandomVolleyOrigins(
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			const BOSS_COMBAT_OBJECT_DEFINITION& definition,
			std::uint32_t spawnWaveOrdinal,
			std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET>& outOrigins);
		bool Broadcast_CombatObjectLifecycle();
		void Drain_BossCombatEvents();
		bool Apply_WorldDestructionStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
#ifdef _DEBUG
		/* Commit the 69 ordinary contact walls and the 30 outer ring walls in one
		transaction, leaving every floor sector INTACT. A floor-collapse bar only
		arrives after the fight has already taken those walls down, so the
		audition for such a bar has to clear them inside the same atomic request
		instead of a second one the boss could start a pattern between. */
		bool Break_EveryWallForAudition(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
#endif
		/* The navigation grid is the ground a boss pattern stride may cross.
		The collision sweep owns wall contact, while the furthest sample the grid
		still owns is what any stride is allowed to reach, so a charge cannot
		leave the floor before its wall contact is evaluated. A start the grid
		already refuses passes through
		unchanged, because refusing it there would strand the boss for good. */
		static void Resolve_NavigableStep(
			const CServerNavigation& navigation,
			float fromX,
			float fromZ,
			float targetX,
			float targetZ,
			float& outX,
			float& outZ);
		/* Raise the pillar slots on the authored stage edge of the pattern that
		owns them. The shatter has no identified product owner yet. */
		bool Apply_EncounterPropStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Commit_DueEncounterProps(std::uint32_t serverTick);
		bool Send_EncounterPropSync(
			const std::shared_ptr<CClientSession>& session);
		void Broadcast_EncounterPropSync();
		/* Break whatever a non-impact boss body physically reached between its
		previous and current position. A charge-impact stage bypasses this generic
		pass and owns one exact swept wall transaction: impact receiver first,
		then the co-located ordinary contact binding. */
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
		void Begin_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		/* Product boss-pattern adapters call these with replicated identities. The
		room owns interruption, fixed-tick fallback motion and release reaction so
		no pattern can leave half of a grabbed player state behind. */
		bool Capture_PlayerAttachment(
			LostArk::Shared::NET_ENTITY_ID playerEntityId,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
			std::uint32_t serverTick);
		bool Update_PlayerAttachment(
			SERVER_PLAYER& player,
			std::uint32_t serverTick);
		bool Release_PlayerAttachment(
			SERVER_PLAYER& player,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		std::size_t Release_PlayerAttachments(
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		void Update_Players(float fixedDeltaSeconds);
		bool Prepare_ArenaEjection(
			SERVER_PLAYER& staged,
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick);
		bool Resolve_ArenaCenter(
			const SERVER_WORLD_ENTITY& boss,
			SERVER_NAV_POINT& point);
		bool Update_DependentBosses(std::uint32_t serverTick);
		/* Slides a hit player along the armed knockback window, clamped to
		walkable floor and blocking bodies; a wall ends the window early. */
		void Advance_PlayerKnockback(
			SERVER_PLAYER& player, float fixedDeltaSeconds);
		void Update_WorldEntities(float fixedDeltaSeconds);

	private:
		// Best-effort traffic leaves room for gameplay/control commands. LEAVE
		// never shares this bounded queue: disconnect cleanup has its own
		// session-deduplicated priority queue below.
		static constexpr std::size_t MAX_BEST_EFFORT_COMMAND_COUNT = 768u;
		static constexpr std::size_t MAX_RELIABLE_COMMAND_COUNT = 960u;
		static constexpr std::size_t MAX_COMMANDS_DRAINED_PER_TICK = 256u;

		mutable std::mutex m_CommandMutex;
		std::deque<ROOM_COMMAND> m_InboundCommands;
		std::deque<ROOM_COMMAND> m_CleanupCommands;
		std::unordered_set<SESSION_ID> m_QueuedCleanupSessionIds;
		SERVER_ROOM_PERFORMANCE_METRICS m_PerformanceMetrics;
		SERVER_ROOM_PERFORMANCE_METRICS m_LastRoomPerfLogSample;
		std::uint64_t m_iLastRoomPerfSnapshotDroppedCount = 0;
		std::uint64_t m_iLastRoomPerfReliableRejectedCount = 0;
		std::uint64_t m_iLastRoomPerfWireSendFailureCount = 0;
		std::size_t m_iLastRoomPerfOutboundHighWatermark = 0u;
		bool m_acceptsCommands = true;
		std::deque<SERVER_WORLD_TRANSFER_REQUEST> m_PendingWorldTransfers;
		struct PENDING_ESTHER_SUMMON final
		{
			const ESTHER_ROSTER_ENTRY* pRosterEntry = nullptr;
			float fPositionX = 0.f;
			float fPositionY = 0.f;
			float fPositionZ = 0.f;
			float fYawDegrees = 0.f;
			float fRemainingSeconds = 0.f;
		};
		std::vector<PENDING_ESTHER_SUMMON> m_PendingEstherSummons;

		std::unordered_map<SESSION_ID, std::weak_ptr<CClientSession>> m_Sessions;
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER> m_Players;
		std::unordered_map<SESSION_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdBySessionId;
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdByEntityId;

		/* Same-room party state -- PLAYER_ID is room-local (freshly allocated
		   per room on Join), so this map does not by itself survive a member
		   moving to a different room. 0 means "no party" -- never a real party
		   ID. Invite/accept/join only; leave/kick/leader promotion is a
		   separate follow-up.
		   A party-leader-triggered group Valtan entry (Handle_ConfirmNpcEntry
		   -> Transfer_PartyTo) is the one case
		   that does survive a room change: every member transfers together in
		   one batch and gets re-grouped into a fresh room-local party in the
		   target room, so the party itself is never actually split across two
		   rooms at once. There is still no general cross-room party identity
		   (e.g. inviting or chatting with someone in a different room). */
		std::uint32_t m_iNextPartyId = 1u;
		std::unordered_map<LostArk::Shared::PLAYER_ID, std::uint32_t>
			m_PartyIdByPlayerId;
		std::unordered_map<std::uint32_t, std::vector<LostArk::Shared::PLAYER_ID>>
			m_PartyMembersByPartyId;
		// One pending invite per target at a time; a new invite silently
		// replaces whatever that target's last unanswered invite was.
		std::unordered_map<LostArk::Shared::PLAYER_ID, LostArk::Shared::PLAYER_ID>
			m_PendingPartyInviteByTargetPlayerId;
		// At most one latest failure per present player. A full reliable queue
		// delays the notice instead of disconnecting a rejected source party.
		std::unordered_map<SESSION_ID, LostArk::Shared::S2C_PARTY_TRANSFER_RESULT>
			m_PendingPartyTransferResults;

		LostArk::Shared::WORLD_ID m_eWorldId = LostArk::Shared::WORLD_ID::END;
		CWorldBootstrap m_WorldBootstrap;
		CGameplayCatalogGenerations m_GameplayCatalog;
		CItemCatalog m_ItemCatalog;
		CServerNavigation m_ServerNavigation;
		CServerCollisionSystem m_ServerCollisionSystem;
		CServerTriggerSystem m_ServerTriggerSystem;
		CSpawnGroupBootstrap m_SpawnGroupBootstrap;
		CSpawnGroupRuntime m_SpawnGroupRuntime;
		CPlayerSkillSystem m_PlayerSkillSystem;
		CCombatObjectRuntime m_CombatObjectRuntime;
		CMonsterBrain m_MonsterBrain;
		CNpcBehaviorRuntime m_NpcBehaviorRuntime;
		CValtanBrain m_ValtanBrain;
		std::unique_ptr<CValtanBrain> m_DependentValtanBrain =
			std::make_unique<CValtanBrain>();
		VALTAN_DECISION_TRACE_REVISION_STATE m_ValtanDecisionTraceRevision;
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
		std::vector<LostArk::Shared::BOSS_COMBAT_EVENT>
			m_TickBossCombatEvents;
		std::string m_strStatus;
		bool m_isReady = false;

		LostArk::Shared::PLAYER_ID m_iNextPlayerId = 1;
		LostArk::Shared::NET_ENTITY_ID m_iNextNetEntityId = 100;
		std::uint32_t m_iServerTick = 0;
		std::uint64_t m_iNextWorldDestructionEventSequence = 1u;
		std::uint64_t m_iNextBossCombatEventSequence = 1u;
		/* Debug Valtan audition. The armed bar is the one an ARM parked the boss
		above; a CROSS is only honoured for that same bar, so a crossing can
		never span an unknown number of authored thresholds. Both reset with the
		encounter, and the handled sequences reject a resent request instead of
		replaying it. Stable-ID pattern requests have an independent ledger because
		the Effect Tool and the Valtan level own independent sequence counters. */
		std::uint32_t m_iValtanAuditionArmedHealthBar = 0;
		std::unordered_map<SESSION_ID, std::uint32_t>
			m_ValtanAuditionSequenceBySessionId;
		std::unordered_map<SESSION_ID, std::uint32_t>
			m_ValtanPatternIdAuditionSequenceBySessionId;
		struct VALTAN_PATTERN_FLOW_COMMAND_RECEIPT final
		{
			std::uint32_t iSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strRequestIdentity;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
		};
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowStartSequenceBySessionId;
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowControlSequenceBySessionId;
#ifdef _DEBUG
		struct TARGETED_VALTAN_AUDITION_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanAuditionEpoch = 1u;
		std::vector<TARGETED_VALTAN_AUDITION_LIFECYCLE>
			m_PendingValtanAuditionLifecycle;
		struct TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanPatternFlowEpoch = 1u;
		std::vector<TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE>
			m_PendingValtanPatternFlowLifecycle;
		VALTAN_PATTERN_ID_AUDITION_STATE m_ValtanPatternIdAudition;
		std::optional<VALTAN_NEXT_PATTERN_RESERVATION> m_ValtanNextPattern;
		std::unordered_map<SESSION_ID, VALTAN_NEXT_PATTERN_COMMAND_RECEIPT>
			m_ValtanNextPatternReceiptBySessionId;
		VALTAN_PATTERN_FLOW_AUDITION_STATE m_ValtanPatternFlowAudition;
		VALTAN_TIMELINE_AUDITION_STATE m_ValtanTimelineAudition;
		VALTAN_FIGHT_PAGE_START_STATE m_ValtanFightPageStart;
#endif
	};
}
