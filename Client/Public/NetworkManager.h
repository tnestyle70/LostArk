#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "ClientReplicationEvent.h"
#include "ClientSessionDiagnostic.h"
#include "ValtanPresentationGenerationAdmission.h"

#include "Network/PacketFrame.h"
#include "Network/PacketMessages.h"
#include "Network/PacketStreamParser.h"

//race�� �����ϱ� ���ؼ� atomic header�� �߰�
#include <atomic>
#include <deque>
#include <memory>
//���� ���� race�� ���� ���ؼ� mutex ���� �� ���
#include <mutex>
#include <thread>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>


class CNetworkManager final
{
public:
	static constexpr std::size_t MAX_REPLICATION_EVENT_QUEUE = 4096u;
	static constexpr std::size_t MAX_INBOUND_FRAME_QUEUE = 4096u;
	static constexpr std::size_t MAX_REVISION_CONTROL_QUEUE = 64u;
	static constexpr std::size_t MAX_PRESENTATION_ALIAS_GENERATIONS = 16u;

	struct PRESENTATION_ARTIFACT_BASELINE final
	{
		std::string strRelativePath;
		std::string strLane;
		std::string strSha256;
		std::uint64_t iBytes = 0u;
	};

	enum class PRESENTATION_CANDIDATE_PREFLIGHT_RESULT : std::uint8_t
	{
		CURRENT_GENERATION_READY = 0u,
		REJECTED,
	};

	/* A prepared Valtan revision owns one exact receipt for the typed
	   presentation closure that was saved before PREPARE.  The Server revision
	   remains the CAS identity; the receipt prevents a later physical edit from
	   being relabelled as that already prepared generation. */
	struct GAMEPLAY_REVISION_CLIENT_STATE final
	{
		/* Captured at world admission for the entry revision and read-only entry
		   diagnostics. PREPARE does not compare against these historical bytes; it
		   captures the current saved typed closure into StagedPresentationReceipt. */
		bool hasPresentationArtifactBaseline = false;
		/* Entry may overlap a short canonical reader/writer transaction. Gameplay
		   admission remains authoritative while this bounded, main-thread retry
		   reacquires the same saved presentation closure after that transaction. */
		bool hasPendingEntryPresentationBaselineRecovery = false;
		std::uint64_t iNextEntryPresentationBaselineRecoveryAtMilliseconds = 0u;
		std::vector<PRESENTATION_ARTIFACT_BASELINE>
			PresentationArtifactBaseline;
		Client::VALTAN_PRESENTATION_GENERATION_RECEIPT
			BootstrapPresentationReceipt;
		bool hasBootstrapPresentationRevision = false;
		LostArk::Shared::GameplayDataRevision BootstrapPresentationRevision{};
		LostArk::Shared::GameplayDataRevision ServerActiveRevision{};
		std::vector<LostArk::Shared::GameplayDataRevision>
			RequiredPinnedRevisions;
		std::uint32_t iLatestTransactionSequence = 0u;
		bool hasLatestPrepare = false;
		LostArk::Shared::GameplayDataRevision LatestPrepareBaseRevision{};
		LostArk::Shared::GameplayDataRevision LatestCandidateRevision{};
		std::uint32_t iLatestRequiredPresentationLaneMask = 0u;
		bool hasOutstandingPrepareRequest = false;
		std::uint32_t iOutstandingPrepareRequestSequence = 0u;
		LostArk::Shared::GameplayDataRevision
			OutstandingPrepareCandidateRevision{};
		/* A participant that rejected PREPARE still belongs to the Server's
		   process-wide transaction and must consume its matching terminal ABORT.
		   Keep this separate from the staged READY alias so a rejected overlapping
		   transaction cannot discard an earlier legitimate stage. */
		bool hasRejectedPrepareAwaitingAbort = false;
		std::uint32_t iRejectedPrepareTransactionSequence = 0u;
		LostArk::Shared::GameplayDataRevision RejectedPrepareBaseRevision{};
		LostArk::Shared::GameplayDataRevision RejectedPrepareCandidateRevision{};
		bool hasStagedPresentationAlias = false;
		LostArk::Shared::GameplayDataRevision StagedPresentationAlias{};
		std::uint32_t iStagedPresentationTransactionSequence = 0u;
		std::uint32_t iStagedPresentationLaneMask = 0u;
		Client::VALTAN_PRESENTATION_GENERATION_RECEIPT
			StagedPresentationReceipt;
		std::vector<LostArk::Shared::GameplayDataRevision>
			AvailablePresentationAliases;
		std::vector<Client::VALTAN_PRESENTATION_GENERATION_RECEIPT>
			AvailablePresentationReceipts;
		LostArk::Shared::DATA_REVISION_PREPARE_STATUS eLatestPrepareResponse =
			LostArk::Shared::DATA_REVISION_PREPARE_STATUS::NACK;
		bool hasLatestResult = false;
		LostArk::Shared::DATA_REVISION_RESULT eLatestResult =
			LostArk::Shared::DATA_REVISION_RESULT::ABORTED;
		std::string strLatestTransactionReason;
		bool isPresentationIsolated = false;
		std::string strIsolationReason;
	};

	/* One in-flight query and one retained trace keep the F1 observatory
	bounded. UNCHANGED and typed rejection responses update the response status
	without discarding the last real Server decision. */
	struct VALTAN_DECISION_TRACE_CLIENT_STATE final
	{
		bool isQueryPending = false;
		std::uint32_t iSubmittedRequestSequence = 0u;
		std::string strSubmittedBossPlacementId;
		std::uint64_t iSubmittedAfterTraceSequence = 0u;
		bool hasLatestResponse = false;
		std::uint32_t iLatestResponseRequestSequence = 0u;
		LostArk::Shared::VALTAN_DECISION_TRACE_QUERY_RESULT eLatestResponse =
			LostArk::Shared::VALTAN_DECISION_TRACE_QUERY_RESULT::NO_TRACE;
		bool hasLatestTrace = false;
		std::string strLatestBossPlacementId;
		LostArk::Shared::GameplayDataRevision LatestDefinitionRevision{};
		LostArk::Shared::VALTAN_DECISION_TRACE_WIRE LatestTrace{};
	};

	CNetworkManager() = default;
	CNetworkManager(const CNetworkManager&) = delete;
	CNetworkManager& operator=(const CNetworkManager&) = delete;

private:
	~CNetworkManager() = default;

public:
	static CNetworkManager& Get();
	static constexpr std::uint16_t DEFAULT_SERVER_PORT = 7777;
	static std::string Resolve_ServerHost();
	static std::string Resolve_MapEditorServerHost();

	bool Initialize();
	void Shutdown();
	void Update();

	bool Connect_To_Server(
		std::string_view host,
		std::uint16_t port);
	bool Connect_To_Server(std::uint16_t port)
	{
		return Connect_To_Server(Resolve_ServerHost(), port);
	}

	bool Send_EnterWorld(
		LostArk::Shared::WORLD_ID worldId,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		std::string_view nickName);
	//playercontroller�� ��ǥ XZ�� �����ϴ� public ���
	bool Send_MoveGoal(
		std::uint32_t clientSequence,
		float goalX,
		float goalZ);
	bool Send_UseSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float aimX,
		float aimZ);
	bool Send_UseGroundTargetSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float targetX,
		float targetZ);
	bool Send_ReleaseSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId);
	bool Send_SkillAim(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float aimX,
		float aimZ);
	bool Send_RevivePlayer(std::uint32_t clientSequence);
#ifdef _DEBUG
	// Debug/Development-build test aid only -- see PACKET_TYPE::C2S_DEBUG_KILL_SELF.
	bool Send_DebugKillSelf(std::uint32_t clientSequence);
#endif
	bool Send_EstherSkill(
		std::uint32_t clientSequence,
		std::uint8_t slotIndex,
		float aimX,
		float aimZ);
	bool Send_ChangeCharacterClass(
		std::uint32_t clientSequence,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	bool Send_SpawnWorldEntity(std::string_view placementId);
	/* Debug Character Select Arena "되돌리기". The Server owns which entities exist;
	this only carries the request, and the answer arrives as one
	S2C_WORLD_ENTITY_DESPAWNED per removed entity through the normal
	replication event queue -- no separate result message. */
	bool Send_DespawnAllWorldEntities(std::uint32_t requestSequence);
	/* Debug-only authoring commands. Both remain requests: Server transfer and
	Server snapshots are the only accepted world/position results. */
	bool Send_DebugEnterKakulSaydonArena(std::uint32_t requestSequence);
	bool Send_DebugTeleportToPosition(
		std::uint32_t requestSequence, float pickedX, float pickedY, float pickedZ);
	bool Try_Consume_DebugTeleportResult(
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT& result);
	bool Send_DebugTeleportToPlacement(
		std::uint32_t requestSequence,
		std::string_view placementId);
	/* Bern's Valtan-entry confirm window's confirm button. The Server re-validates
	proximity to the named guide NPC and answers through the same
	S2C_ENTER_ACCEPTED/S2C_ENTER_REJECTED world-transfer flow the old automatic
	changeLevel trigger used -- no separate result message. */
	/* Answers the prompt the Server last offered. Carries only the box's own
	   id -- the Server re-tests that this player is still inside it. */
	bool Send_InteractTrigger(
		std::uint32_t requestSequence,
		std::string_view triggerPlacementId);
	bool Send_ConfirmNpcEntry(
		std::uint32_t requestSequence,
		std::string_view npcPlacementId);
	/* 파티 레이드 입장 투표 발의/응답. propose는 즉시 전송하지 않고 Server가 파티 전원
	투표를 열며, 결과는 S2C_RAID_ENTRY_PROMPT/S2C_RAID_ENTRY_VOTE replication 이벤트로 온다. */
	bool Send_RaidEntryPropose(
		std::uint32_t requestSequence,
		std::string_view npcPlacementId,
		LostArk::Shared::RAID_ENTRY_TARGET target);
	bool Send_RaidEntryRespond(
		std::uint32_t requestSequence,
		std::uint32_t proposalId,
		bool accepted);
	// Raid Clear screen's "돌아가기" button, Valtan Arena only -- reverse trip
	// of Send_ConfirmNpcEntry, no NPC target needed.
	bool Send_ReturnToBern(std::uint32_t requestSequence);
	/* Same-room-only party invite. The Server re-validates the target
	NetEntityId is a real player in this room; the answer (if any) arrives
	as an S2C_PARTY_INVITE_RECEIVED replication event on the target's own
	connection, not a direct reply to the sender. */
	bool Send_PartyInvite(
		std::uint32_t requestSequence,
		LostArk::Shared::NET_ENTITY_ID targetNetEntityId);
	bool Send_PartyInviteRespond(
		std::uint32_t requestSequence,
		LostArk::Shared::NET_ENTITY_ID fromNetEntityId,
		bool accepted);
	/* The Server relays this to every current room member (sender included)
	as an S2C_CHAT replication event -- there is no direct reply. */
	bool Send_Chat(const std::string& text);
	/* Debug-only. The Server owns the truth; this only carries the request and
	the answer arrives as an S2C_INVENTORY_SNAPSHOT replication event. */
	bool Send_DebugGiveItem(
		std::uint32_t requestSequence,
		std::string_view itemId,
		std::uint32_t quantity);
	/* A consumable used from a quick slot. The Server owns the heal/decrement;
	the answer arrives the same way -- an S2C_INVENTORY_SNAPSHOT replication
	event, plus the next S2C_WORLD_SNAPSHOT tick for the new HP. */
	bool Send_UseItem(
		std::uint32_t requestSequence,
		std::string_view itemId);
	/* Debug Valtan pattern audition. The Server owns the verdict; this only
	carries the request and hands back whatever it answered. */
	bool Send_ValtanAudition(
		std::uint32_t requestSequence,
		LostArk::Shared::VALTAN_AUDITION_OPERATION operation,
		std::uint32_t targetHealthBar);
	/* Stable-ID Server pattern audition with an exact active-definition CAS.
	Results use a dedicated queue so the Valtan Arena's legacy transaction
	consumer cannot drain an Effect Tool request (or vice versa). */
	bool Send_ValtanPatternAuditionById(
		std::uint32_t requestSequence,
		std::string_view bossPlacementId,
		std::string_view patternId,
		const LostArk::Shared::GameplayDataRevision&
			expectedActiveDefinitionRevision);
	/* Exact active-occurrence restart. The caller supplies the full predecessor
	   CAS identity; this boundary never reconstructs or drops wire fields. */
	bool Send_ValtanPatternRestart(
		const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& message);
	/* Queue/replace/clear carry the complete predecessor and reservation CAS
	   identity. The shared audition service owns all three stable-ID results. */
	bool Send_ValtanNextPatternCommand(
		const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& message);
	/* KoukuSaydon owns a separate exact-scope playback command. The caller
	provides the complete Product tuple; this boundary never substitutes Valtan
	state or reconstructs a missing identity. */
	bool Send_KoukuSaydonPatternAudition(
		const LostArk::Shared::
			C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& message);
	/* Debug Valtan Boss Tool ordered Flow. The UI supplies one admitted saved
	   revision; the Server preflights the full slot list and owns every
	   occurrence after the single reset. */
	bool Send_ValtanPatternFlowStart(
		const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& message);
	bool Send_ValtanPatternFlowStopAfterCurrent(
		const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT&
			message);
	/* Debug Balance Tool starts one process-wide revision transaction through
	   this typed boundary.  The Server remains authoritative for candidate
	   admission, affected-room staging, and the final room-tick commit. */
	bool Send_DataRevisionPrepareRequest(
		const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& message);
	/* Typed 2PC response boundary.  Client presentation staging never writes a
	   packet directly and must report lane masks through this message. */
	bool Send_DataRevisionPrepareResponse(
		const LostArk::Shared::C2S_DATA_REVISION_PREPARE_RESPONSE& message);
	/* Debug-only bounded observatory query. Release Server implementations keep
	   the packet known and answer with REJECTED_RELEASE_BUILD. */
	bool Send_ValtanDecisionTraceQuery(
		std::uint32_t requestSequence,
		std::string_view bossPlacementId,
		std::uint64_t afterTraceSequence);

	bool Try_Consume_EnterAccepted(
		LostArk::Shared::S2C_ENTER_ACCEPTED& message);
	bool Try_Consume_EnterRejected(
		LostArk::Shared::S2C_ENTER_REJECTED& message);
	bool Try_Consume_WorldEntitySpawnResult(
		LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT& message);
	bool Try_Consume_CharacterClassChangeResult(
		LostArk::Shared::S2C_CHARACTER_CLASS_CHANGE_RESULT& message);
	bool Try_Consume_ValtanAuditionResult(
		LostArk::Shared::S2C_VALTAN_AUDITION_RESULT& message);
	bool Try_Consume_ValtanPatternAuditionByIdResult(
		LostArk::Shared::S2C_VALTAN_AUDITION_RESULT& message);
	bool Try_Consume_ValtanAuditionLifecycle(
		LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE& message);
	bool Try_Consume_KoukuSaydonPatternAuditionResult(
		LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& message);
	bool Try_Consume_KoukuSaydonPatternAuditionLifecycle(
		LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE&
			message);
	bool Try_Consume_ValtanPatternFlowResult(
		LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT& message);
	bool Try_Consume_ValtanPatternFlowLifecycle(
		LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& message);

	bool Try_Consume_ReplicationEvent(
		Client::CLIENT_REPLICATION_EVENT& event);

	void Close_ServerConnection();

	[[nodiscard]] bool Is_Connected() const;
	[[nodiscard]] std::uint64_t Get_WorldInboundGeneration() const
	{
		return m_iWorldInboundGeneration;
	}
	[[nodiscard]] int Get_LastErrorCode() const;
	[[nodiscard]] Client::CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT
		Get_SessionDiagnosticSnapshot() const
	{
		return m_SessionDiagnostic.Get_Snapshot();
	}
	void Record_SessionEvent(
		std::string_view eventName,
		std::string_view detail = {});
	void Record_SessionRecovery(
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
		std::string_view source,
		std::string_view detail);
	bool Record_SessionTerminal(
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
		int wsaError,
		LostArk::Shared::PACKET_TYPE triggeringPacket,
		std::string_view detail);
	[[nodiscard]] LostArk::Shared::PLAYER_ID Get_LocalPlayerId() const;
	[[nodiscard]] LostArk::Shared::NET_ENTITY_ID Get_LocalEntityId() const;
	[[nodiscard]] LostArk::Shared::CHARACTER_CLASS_ID
		Get_LocalCharacterClass() const;
	[[nodiscard]] bool Try_Get_LocalSpawn(
		LostArk::Shared::S2C_PLAYER_SPAWNED& outSpawn) const;
	[[nodiscard]] const GAMEPLAY_REVISION_CLIENT_STATE&
		Get_GameplayRevisionState() const
	{
		return m_GameplayRevisionState;
	}
	[[nodiscard]] const VALTAN_DECISION_TRACE_CLIENT_STATE&
		Get_ValtanDecisionTraceState() const
	{
		return m_ValtanDecisionTraceState;
	}
	[[nodiscard]] bool Try_Get_LatestValtanDecisionTrace(
		LostArk::Shared::GameplayDataRevision& outDefinitionRevision,
		LostArk::Shared::VALTAN_DECISION_TRACE_WIRE& outTrace) const;
	[[nodiscard]] bool Is_PresentationRevisionAvailable(
		const LostArk::Shared::GameplayDataRevision& revision) const;
	[[nodiscard]] bool Try_Get_ValtanPresentationGenerationReceipt(
		const LostArk::Shared::GameplayDataRevision& revision,
		Client::VALTAN_PRESENTATION_GENERATION_RECEIPT& outReceipt,
		std::string& status);
	/* Read-only Debug truth for tools that reload repository presentation JSON.
	   This validates the current typed closure under canonical reader admission;
	   it is not a comparison with historical world-entry bytes. */
	[[nodiscard]] bool Is_CurrentPresentationBaselineIntact(
		std::string& status) const;
	/* Validate that the immutable candidate is exactly the current saved typed
	   closure before sending PREPARE. New Pattern/Stage topology and new Effect
	   paths are admitted when they belong to that validated closure. */
	[[nodiscard]] PRESENTATION_CANDIDATE_PREFLIGHT_RESULT
		Preflight_PresentationCandidate(
			const LostArk::Shared::GameplayDataRevision& candidateRevision,
			std::uint32_t requiredPresentationLaneMask,
			std::string& status) const;


private:
	bool Send_All(
		std::span<const std::uint8_t> bytes,
		LostArk::Shared::PACKET_TYPE triggeringPacket =
			LostArk::Shared::PACKET_TYPE::INVALID);
	bool Enqueue_ReplicationEvent(
		Client::CLIENT_REPLICATION_EVENT&& event);
	void Fail_Protocol(
		int errorCode,
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason =
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_INVALID_SERVER_RESPONSE,
		LostArk::Shared::PACKET_TYPE triggeringPacket =
			LostArk::Shared::PACKET_TYPE::INVALID,
		std::string_view detail = "Protocol validation failed.");
	void Reset_WorldInboundState();
	bool Try_Recover_EntryPresentationBaseline(std::string& status);
	void Record_WorldRevisionSet(
		const LostArk::Shared::GameplayDataRevision& activeRevision,
		const std::vector<LostArk::Shared::GameplayDataRevision>&
			requiredPinnedRevisions);
	void Prune_PresentationAliases();
	[[nodiscard]] bool Is_AnnouncedWorldRevision(
		const LostArk::Shared::GameplayDataRevision& revision) const;
	void Record_PresentationIsolation(
		const LostArk::Shared::GameplayDataRevision& revision,
		std::string_view context);
	bool Stage_ByteIdenticalPresentationAlias(
		const LostArk::Shared::S2C_DATA_REVISION_PREPARE& prepare,
		std::string& status);
	bool Commit_StagedPresentationAlias(
		const LostArk::Shared::S2C_DATA_REVISION_RESULT& result,
		std::string& status);
	void Discard_StagedPresentationAlias() noexcept;
	//���� worker �ϳ��� 4096-byte ���� ���۷� Server�� TCP byte stream�� �д´�.
	void Receive_Loop(SOCKET serverSocket);
	void Handle_Frame(const LostArk::Shared::PACKET_FRAME& frame);

#if defined(LOSTARK_NETWORK_MANAGER_HARNESS)
public:
	void Harness_Reset()
	{
		Close_ServerConnection();
		m_iLastErrorCode.store(0);
	}
	void Harness_HandleFrame(const LostArk::Shared::PACKET_FRAME& frame)
	{
		Handle_Frame(frame);
	}
	void Harness_SetAcceptedWorld(const LostArk::Shared::WORLD_ID worldId)
	{
		Reset_WorldInboundState();
		m_eWorldId = worldId;
	}
	void Harness_SetRequestedCharacterClass(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		m_eLocalCharacterClass = characterClass;
	}
	bool Harness_EnqueueReplicationEvent(
		Client::CLIENT_REPLICATION_EVENT&& event)
	{
		return Enqueue_ReplicationEvent(
			static_cast<Client::CLIENT_REPLICATION_EVENT&&>(event));
	}
	[[nodiscard]] std::size_t Harness_GetReplicationEventCount() const
	{
		return m_ReplicationEvents.size();
	}
#endif

private:
	SOCKET m_hServerSocket = INVALID_SOCKET;
	//main thread�� Receive worker�� ���� �ڵ带 �Բ� �а� ���Ƿ� atomic���� ��ȣ�Ѵ�.
	std::atomic<int> m_iLastErrorCode{ 0 };
	bool m_isWinSocketInitialized = false;

	std::thread m_ReceiveThread;

	std::atomic_bool m_isReceiveRunning{ false };
	std::atomic_bool m_hasProtocolFailure{ false };
	Client::CClientSessionDiagnostic m_SessionDiagnostic;

	LostArk::Shared::CPacketStreamParser m_StreamParser;

	std::mutex m_InboundMutex;
	std::deque<LostArk::Shared::PACKET_FRAME> m_InboundFrames;

	//Handle Frame�� �Һ��� ��� main thread�̴�.
	std::deque<Client::CLIENT_REPLICATION_EVENT> m_ReplicationEvents;
	std::deque<LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT>
		m_DebugTeleportResults;
	std::deque<LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT>
		m_WorldEntitySpawnResults;
	std::deque<LostArk::Shared::S2C_CHARACTER_CLASS_CHANGE_RESULT>
		m_CharacterClassChangeResults;
	std::deque<LostArk::Shared::S2C_VALTAN_AUDITION_RESULT>
		m_ValtanAuditionResults;
	std::deque<LostArk::Shared::S2C_VALTAN_AUDITION_RESULT>
		m_ValtanPatternAuditionByIdResults;
	std::deque<LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE>
		m_ValtanAuditionLifecycleEvents;
	std::deque<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT>
		m_KoukuSaydonPatternAuditionResults;
	std::deque<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE>
		m_KoukuSaydonPatternAuditionLifecycleEvents;
	std::deque<LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT>
		m_ValtanPatternFlowResults;
	std::deque<LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE>
		m_ValtanPatternFlowLifecycleEvents;
	GAMEPLAY_REVISION_CLIENT_STATE m_GameplayRevisionState;
	std::unique_ptr<Client::CValtanPresentationGenerationReadAdmission>
		m_pStagedPresentationAdmission;
	VALTAN_DECISION_TRACE_CLIENT_STATE m_ValtanDecisionTraceState;
	std::uint64_t m_iWorldInboundGeneration = 1u;

	bool m_hasPendingEnterAccepted = false;

	LostArk::Shared::S2C_ENTER_ACCEPTED m_PendingEnterAccepted{};
	bool m_hasPendingEnterRejected = false;
	LostArk::Shared::S2C_ENTER_REJECTED m_PendingEnterRejected{};

	LostArk::Shared::PLAYER_ID m_iLocalPlayerId = LostArk::Shared::INVALID_PLAYER_ID;

	LostArk::Shared::NET_ENTITY_ID m_iLocalNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	LostArk::Shared::WORLD_ID m_eWorldId =
		LostArk::Shared::WORLD_ID::END;
	LostArk::Shared::CHARACTER_CLASS_ID m_eLocalCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	bool m_hasLocalSpawn = false;
	LostArk::Shared::S2C_PLAYER_SPAWNED m_LocalSpawn = {};

};
