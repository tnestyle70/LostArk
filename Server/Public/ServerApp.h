#pragma once

//Room 객체를 값으로 소유
#include "GameRoom.h"
//sessionid 사용
#include "ServerIds.h"
//접속 수락
#include "TcpListener.h"
//winsock 시작과 종료
#include "WinSockContext.h"
//PACKET_FRAME header
#include "Network/PacketFrame.h"
#include "Network/SessionDiagnostic.h"

//서버 실행 상태와 다음 sessionid
#include <atomic>
//accept thread와 room thread
#include <thread>
//shared_ptr
#include <memory>
//sessionid로 session 검색
#include <unordered_map>
//session map과 종료 queue 보호
#include <mutex>
//종료된 sessionid 순서 보관
#include <deque>
#include <vector>
#include <map>
#include <cstdint>
#include <string_view>
#include <chrono>
#include <filesystem>

// CServerApp은 서버 전체의 시작, 실행, 종료를 조율하는 최상위 객체다.
// WinSock/Listener 수명, Accept Thread, Room Thread, ClientSession 집합을 소유한다.
// ClientSession이 전달한 Frame을 Shared 메시지로 읽어 ROOM_COMMAND로 번역할 뿐,
// Player 상태 변경과 Broadcast 결정은 CGameRoom에 위임한다.

namespace LostArk::Server
{
	//clientsession 전방 선언
	class CClientSession;

	enum class RUNTIME_GAMEPLAY_GENERATION_SOURCE : std::uint8_t
	{
		PACKAGED_BASELINE,
		CANDIDATE
	};

	/* Process-local durable identity. It never trusts the publisher's
	current-candidate pointer: only a Server 2PC commit writes this record. */
	struct RUNTIME_ACTIVE_GAMEPLAY_GENERATION final
	{
		RUNTIME_GAMEPLAY_GENERATION_SOURCE eSource =
			RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
		LostArk::Shared::GameplayDataRevision Revision{};
		LostArk::Shared::GameplayDataRevision BootstrapContentRevision{};
		LostArk::Shared::GameplayDataRevision NonValtanGameplayRevision{};

		[[nodiscard]] bool Is_Valid() const noexcept
		{
			return Revision.Is_Valid() && BootstrapContentRevision.Is_Valid() &&
				NonValtanGameplayRevision.Is_Valid();
		}
	};

	class CServerApp final
	{
		friend int Run_ServerGameplayContractTests(bool, bool);
	public:
		//소멸자 - 중간 실패나 정상 종료 여부 상관 없이
		//socket과 thread를 정리
		~CServerApp();
		//서버의 진입점
		int Run(
			std::uint32_t automaticShutdownMilliseconds = 0,
			std::string_view bindAddress = "0.0.0.0",
			std::uint16_t port = 7777u,
			bool headless = false);
		/* Debug-only offline recovery. It never resets a live encounter: it
		acquires the same process lock as Run, validates the durable pointer/journal
		old/new identity structure, and atomically selects the packaged bootstrap
		for next start without re-admitting the discarded candidate artifacts. */
		static int Reset_ValtanRuntimeToPackaged();

	private:
		struct SERVER_CONTROL_EVENT;
		//접속을 계속 받아 새로운 session을 생성
		void Accept_Loop();
		//GameRoom을 일정한 간격으로 tick한다. 초기 30Hz
		void Room_Loop();
		//session receive thread가 완성된 frame을 전달하는 callback이다
		void On_SessionFrame(
			SESSION_ID sessionId,
			const LostArk::Shared::PACKET_FRAME& frame);
		//session receive thread가 연결 종료를 발견했을 때 호출한다.
		void On_SessionClosed(SESSION_ID sessionId);
		//Session map에서 대상을 찾고, request_close만 호출
		void Request_SessionClose(
			SESSION_ID sessionId,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason =
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE,
			int nativeErrorCode = 0,
			std::string_view context = {});
		//종료 callback이 남긴 sessionid를 안전하게 정리
		void Reap_ClosedSessions();

		struct SESSION_GAMEPLAY_BINDING
		{
			LostArk::Shared::WORLD_ID eWorldId =
				LostArk::Shared::WORLD_ID::END;
			SESSION_ID iPrivateArenaOwnerSessionId =
				INVALID_SESSION_ID;
			std::shared_ptr<CGameRoom> pSimulation;
		};

		std::shared_ptr<CGameRoom> Acquire_EntrySimulation(
			SESSION_ID sessionId,
			LostArk::Shared::WORLD_ID worldId);
		std::shared_ptr<CGameRoom> Find_SharedSimulation(
			LostArk::Shared::WORLD_ID worldId);
		bool Bind_AndEnqueueEntry(
			SESSION_ID sessionId,
			LostArk::Shared::WORLD_ID worldId,
			const std::shared_ptr<CGameRoom>& simulation,
			LostArk::Shared::C2S_ENTER_WORLD enterWorld,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outFailureReason,
			int& outNativeErrorCode,
			std::string& outFailureContext);
		ROOM_COMMAND_ENQUEUE_RESULT Enqueue_AssignedCommand(
			SESSION_ID sessionId,
			ROOM_COMMAND command,
			std::string& outContext);
		void Tick_GameplaySimulations(float fixedDeltaSeconds);
		void Advance_ServerControlTransactions();
		void Process_ValtanDecisionTraceQuery(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request);
		bool Queue_ServerControlEvent(SERVER_CONTROL_EVENT&& event);
		static bool Resolve_CandidateArtifactForAdmission(
			const std::filesystem::path& candidateDirectory,
			const std::string& relativePath,
			std::filesystem::path& resolvedPath,
			std::string& status);
		static bool Build_NonValtanGameplayRevisionForAdmission(
			const std::filesystem::path& bootstrapPath,
			LostArk::Shared::GameplayDataRevision& revision,
			std::string& status);
		static bool Hash_GameplayFileForAdmission(
			const std::filesystem::path& path,
			LostArk::Shared::GameplayDataRevision& revision,
			std::string& status);
		/* HOT_RELOAD cannot truthfully apply fields copied into a live boss body.
		   Until an encounter-reset transaction exists, admission must reject any
		   candidate that changes those fields instead of reporting COMMITTED. */
		static bool Validate_ValtanHotReloadBaseProfile(
			const BOSS_RUNTIME_PROFILE* activeProfile,
			const BOSS_RUNTIME_PROFILE* candidateProfile,
			std::string& status);
		static bool Validate_ServerSessionDiagnosticJson(
			std::string_view json,
			std::string& status);
		static bool Persist_RuntimeGameplayActivation(
			const std::filesystem::path& runtimeRoot,
			std::uint32_t transactionSequence,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& base,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& candidate,
			std::string& status);
		static bool Rollback_RuntimeGameplayActivation(
			const std::filesystem::path& runtimeRoot,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& base,
			std::string& status);
		static void Complete_RuntimeGameplayActivation(
			const std::filesystem::path& runtimeRoot) noexcept;
		static bool Recover_RuntimeActiveGameplayPointer(
			const std::filesystem::path& runtimeRoot,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
			RUNTIME_ACTIVE_GAMEPLAY_GENERATION& active,
			bool& hasPersistedPointer,
			std::string& status,
			bool allowPackagedIdentityDrift = false);
		static bool Reset_RuntimeGameplayActivationToPackaged(
			const std::filesystem::path& runtimeRoot,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
			std::string& status);
		static bool Acquire_RuntimeGameplayProcessMutex(
			void*& handle,
			std::string& status);
		// Production keeps its fixed global name; the friend contract owns an
		// isolated name while exercising this same Win32 ownership path.
		static bool Acquire_NamedRuntimeGameplayProcessMutex(
			const wchar_t* name,
			void*& handle,
			std::string& status);
		static void Release_RuntimeGameplayProcessMutex(void*& handle) noexcept;
		static bool Load_RuntimeActiveGameplayGeneration(
			const std::filesystem::path& runtimeRoot,
			const std::shared_ptr<const CGameplayCatalog>& packagedGeneration,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
			std::shared_ptr<const CGameplayCatalog>& activeGeneration,
			RUNTIME_ACTIVE_GAMEPLAY_GENERATION& active,
			bool& isCandidate,
			std::string& status);
		[[nodiscard]] std::filesystem::path
			Resolve_RuntimeActiveGameplayRoot() const;
		void Abort_DataRevisionTransaction(std::string reason);
		bool Commit_DataRevisionTransaction();
		bool Validate_DataRevisionTransactionMembership(
			std::string& status);
		bool Send_DataRevisionPrepare(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request);
		bool Send_DataRevisionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request,
			LostArk::Shared::DATA_REVISION_RESULT result,
			const LostArk::Shared::GameplayDataRevision& activeRevision,
			std::string reason);
		void Retire_QuiescentCharacterSelectArenas();
		void Handle_WorldTransfers(
			const std::shared_ptr<CGameRoom>& sourceSimulation);
		struct SESSION_WORLD_TRANSFER_FAILURE final
		{
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON eReason =
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED;
			int iNativeErrorCode = 0;
			std::string strContext;
			bool bRollbackCleanupRequired = false;
			bool bRollbackCleanupEnqueued = false;
			LostArk::Shared::PARTY_TRANSFER_RESULT ePartyResult =
				LostArk::Shared::PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE;
		};
		bool Transfer_SessionWorld(
			const std::shared_ptr<CGameRoom>& sourceSimulation,
			const SERVER_WORLD_TRANSFER_REQUEST& transfer,
			SESSION_WORLD_TRANSFER_FAILURE& outFailure);
		//서버 전체 종료 순서 한 곳으로 모아서 정리
		void Shutdown();

		enum class SERVER_CONTROL_EVENT_KIND : std::uint8_t
		{
			DATA_REVISION_REQUEST,
			DATA_REVISION_RESPONSE,
			SESSION_DISCONNECTED,
			VALTAN_DECISION_TRACE_QUERY
		};

		struct SERVER_CONTROL_EVENT final
		{
			SERVER_CONTROL_EVENT_KIND eKind =
				SERVER_CONTROL_EVENT_KIND::SESSION_DISCONNECTED;
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST RevisionRequest{};
			LostArk::Shared::C2S_DATA_REVISION_PREPARE_RESPONSE RevisionResponse{};
			LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY DecisionTraceQuery{};
			std::shared_ptr<const CGameplayCatalog> pCandidateGeneration;
			LostArk::Shared::GameplayDataRevision
				BaseBootstrapContentRevision{};
			LostArk::Shared::GameplayDataRevision
				CandidateBootstrapContentRevision{};
			LostArk::Shared::GameplayDataRevision BaseNonValtanGameplayRevision{};
			LostArk::Shared::GameplayDataRevision
				CandidateNonValtanGameplayRevision{};
		};

		struct DATA_REVISION_PARTICIPANT final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::WORLD_ID eWorldId =
				LostArk::Shared::WORLD_ID::END;
			std::shared_ptr<CClientSession> pSession;
			std::shared_ptr<CGameRoom> pSimulation;
			bool hasResponded = false;
			bool isReady = false;
		};

		struct DATA_REVISION_TRANSACTION final
		{
			SESSION_ID iRequesterSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST Request{};
			std::shared_ptr<const CGameplayCatalog> pCandidateGeneration;
			LostArk::Shared::GameplayDataRevision
				BaseBootstrapContentRevision{};
			LostArk::Shared::GameplayDataRevision
				CandidateBootstrapContentRevision{};
			LostArk::Shared::GameplayDataRevision BaseNonValtanGameplayRevision{};
			LostArk::Shared::GameplayDataRevision
				CandidateNonValtanGameplayRevision{};
			std::vector<DATA_REVISION_PARTICIPANT> Participants;
			std::vector<std::shared_ptr<CGameRoom>> Simulations;
			std::chrono::steady_clock::time_point Deadline{};

			[[nodiscard]] bool Is_Active() const noexcept
			{
				return INVALID_SESSION_ID != iRequesterSessionId;
			}
		};

		struct DATA_REVISION_RESPONSE_TOMBSTONE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			std::uint32_t iTransactionSequence = 0u;
			LostArk::Shared::GameplayDataRevision CandidateRevision{};
			std::uint32_t iRequiredPresentationLaneMask = 0u;
			std::chrono::steady_clock::time_point ExpiresAt{};
		};

	private:
		//서버 기반 객체
		//멤버 순서 중요! 생성 : WinSockContext -> TcpListener
		//소멸 : TcpListener -> WinSockContext 소멸은 역순
		CWinSockContext m_WinSockContext;
		CTcpListener m_TcpListener;
		std::map<
			LostArk::Shared::WORLD_ID,
			std::shared_ptr<CGameRoom>> m_SharedGameRooms;
		std::shared_ptr<const CGameplayCatalog> m_pActiveGameplayGeneration;
		/* Manifest identity and raw bootstrap content identity are deliberately
		   separate. Binding both prevents an old Valtan-only candidate from
		   rolling back newer non-Valtan rows in the full bootstrap. */
		LostArk::Shared::GameplayDataRevision
			m_ActiveGameplayBootstrapContentRevision{};
		LostArk::Shared::GameplayDataRevision
			m_ActiveNonValtanGameplayRevision{};
		bool m_isActiveGameplayGenerationFromCandidate = false;
		bool m_isRuntimeActivePersistenceEnabled = false;
		std::filesystem::path m_RuntimeActiveGameplayRootOverride;
		void* m_hRuntimeGameplayProcessMutex = nullptr;
		std::unordered_map<
			SESSION_ID,
			std::shared_ptr<CGameRoom>> m_CharacterSelectArenas;
		//실행 상태 - 여러 스레드가 읽고 쓰기 때문에 atomic을 사용한다.
		std::atomic_bool m_isRunning{ false };
		std::atomic<SESSION_ID> m_iNextSessionId{ 1 };

		//thread
		std::thread m_AcceptThread;
		std::thread m_RoomThread;

		//session owner map - sessionid의 유일한 장기 강한 owner
		//gameroom은 같은 session을 weak_ptr로만 참조
		std::mutex m_SessionsMutex;
		std::unordered_map<
			SESSION_ID,
			std::shared_ptr<CClientSession>> m_Sessions;
		// sessions, bindings, shared rooms, and private arenas share this mutex.
		std::unordered_map<
			SESSION_ID,
			SESSION_GAMEPLAY_BINDING> m_GameplayBindingBySessionId;
		//종료 대기 Queue
		std::mutex m_ClosedSessionMutex;
		std::deque<SESSION_ID> m_ClosedSessionIds;
		std::mutex m_SessionDiagnosticLogMutex;
		/* Receive callbacks publish only bounded immutable control events. The
		   room thread drains them without this mutex held, then takes sessions
		   before touching any room, preserving sessions -> room lock order. */
		static constexpr std::size_t MAX_SERVER_CONTROL_EVENTS = 256u;
		std::mutex m_ServerControlMutex;
		std::deque<SERVER_CONTROL_EVENT> m_ServerControlEvents;
		std::mutex m_DataRevisionAdmissionMutex;
		DATA_REVISION_TRANSACTION m_DataRevisionTransaction;
		static constexpr std::size_t
			MAX_DATA_REVISION_RESPONSE_TOMBSTONES = 1024u;
		std::deque<DATA_REVISION_RESPONSE_TOMBSTONE>
			m_DataRevisionResponseTombstones;
	};
}
