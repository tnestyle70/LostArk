#pragma once

//Room 객체를 값으로 소유
// 현재 구현은 방을 shared_ptr로 소유한다. 아래 include는 방 관련 자료형의 완전한 정의에도 필요하다.
// CGameRoom의 명령·전환·계측 자료형까지 사용하므로 포인터 전방 선언만으로는 부족하다.
#include "GameRoom.h"
// 숫자 밸런스의 후보 검증·디스크 저장·활성 세대 교체를 분리하는 저장소와 준비 결과다.
#include "ServerBalanceNumericStore.h"
// 룸 스레드는 콜로세움 준비와 밸런스 저장 worker의 결과를 기다리지 않고 조회한다.
// 정상 실행 중에는 완료 여부만 조회한다. Shutdown은 accept/룸 스레드 합류 뒤 각 future를 최대 30초 기다려 회수한다.
#include <future>
//sessionid 사용
// ServerApp이 발급하는 연결 ID와 미연결 표식은 Shared의 플레이어 ID와 별개다.
#include "ServerIds.h"
//접속 수락
// 접속을 받는 소켓의 소유권은 Listener에, 받아 낸 연결 소켓은 ClientSession에 둔다.
#include "TcpListener.h"
//winsock 시작과 종료
// 모든 소켓보다 먼저 초기화하고 나중에 해제해야 하는 프로세스 Winsock 사용권이다.
#include "WinSockContext.h"
//PACKET_FRAME header
// 수신 callback과 저장 중 미룬 입장 명령이 사용하는 완성된 게임 프레임이다.
#include "Network/PacketFrame.h"
// 세션 종료 원인을 연결 계층과 게임 명령 처리 사이에 같은 분류로 전달한다.
#include "Network/SessionDiagnostic.h"

//서버 실행 상태와 다음 sessionid
// 실행 여부·연결 ID 발급·비동기 준비 취소를 서로 다른 스레드에서 공유한다.
#include <atomic>
//accept thread와 room thread
// 접속 수락과 고정 간격 게임 진행을 별도 실행 흐름으로 소유한다.
#include <thread>
//shared_ptr
// 세션·시뮬레이션·읽기 전용 카탈로그가 대기 중에도 살아 있도록 강한 참조를 보관한다.
#include <memory>
//sessionid로 session 검색
// 연결 ID로 소유 세션과 현재 명령 전달 대상을 찾는다.
#include <unordered_map>
//session map과 종료 queue 보호
// 소유 컨테이너와 큐의 복합 변경을 보호한다. atomic만으로 map의 수명은 보호되지 않는다.
#include <mutex>
//종료된 sessionid 순서 보관
// 종료 알림과 제어 이벤트를 받은 순서대로 보관하고 룸 스레드가 회수한다.
#include <deque>
// 한 번의 세대 교체에 참여하는 세션·시뮬레이션 집합을 고정해 보관한다.
#include <vector>
// 월드 ID별 공유 방과 경기 ID별 콜로세움 방의 강한 참조를 소유한다.
#include <map>
// 패킷 sequence, 포트, 세대 출처의 크기를 선언에서 고정한다.
#include <cstdint>
// 주소·오류 문맥을 호출 동안만 차용한다. 오래 보관할 문맥은 std::string으로 복사한다.
#include <string_view>
// 벽시계 변경의 영향을 받지 않는 steady_clock으로 응답 만료를 판정한다.
#include <chrono>
// 검증 후보와 복구 journal의 물리 경로를 전달한다.
#include <filesystem>

// CServerApp은 서버 전체의 시작, 실행, 종료를 조율하는 최상위 객체다.
// WinSock/Listener 수명, Accept Thread, Room Thread, ClientSession 집합을 소유한다.
// ClientSession이 전달한 Frame을 Shared 메시지로 읽어 ROOM_COMMAND로 번역할 뿐,
// Player 상태 변경과 Broadcast 결정은 CGameRoom에 위임한다.

namespace LostArk::Server
{
	class CServerGameplayContractRunner;

	//clientsession 전방 선언
	// 이 헤더는 shared_ptr로만 보관한다. socket 종료·송신 같은 실제 호출은 CPP에서 정의를 포함해 수행한다.
	class CClientSession;

	// 활성 세대가 설치 bootstrap인지 검증된 후보인지 기록해 재시작 시 같은 경로로 복구한다.
	enum class RUNTIME_GAMEPLAY_GENERATION_SOURCE : std::uint8_t
	{
		PACKAGED_BASELINE,
		CANDIDATE
	};

	// Server가 2단계 승인 뒤 확정한 활성 세대 식별자다. 디스크에 남겨 다음 시작 시 복구한다.
	// publisher의 current-candidate 포인터만으로 활성 세대를 선택하지 않는다.
	struct RUNTIME_ACTIVE_GAMEPLAY_GENERATION final
	{
		RUNTIME_GAMEPLAY_GENERATION_SOURCE eSource =
			RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
		// Revision은 세대 identity, 나머지는 전체 bootstrap과 발탄 외 행의 실제 내용 identity다.
		// 세 값이 함께 있어야 발탄 후보가 더 최신인 다른 gameplay 행을 되돌리는 것을 검출할 수 있다.
		LostArk::Shared::GameplayDataRevision Revision{};
		LostArk::Shared::GameplayDataRevision BootstrapContentRevision{};
		LostArk::Shared::GameplayDataRevision NonValtanGameplayRevision{};

		// 세 identity가 모두 채워졌는지만 확인한다. 파일 내용과의 일치는 admission 단계가 검사한다.
		[[nodiscard]] bool Is_Valid() const noexcept
		{
			return Revision.Is_Valid() && BootstrapContentRevision.Is_Valid() &&
				NonValtanGameplayRevision.Is_Valid();
		}
	};

	// 실행 프로세스의 listener·세션·공유/개인 룸과 작업 스레드를 소유하고 연결 명령을 해당 룸으로 전달한다.
	// Shutdown은 명령 생산자를 멈춘 뒤 세션·룸을 회수하므로 소멸 뒤 참조와 콜백이 남지 않게 한다.
	class CServerApp final
	{
		friend class CServerGameplayContractRunner;
		friend int Run_ServerBingoContractTests();
	public:
		//소멸자 - 중간 실패나 정상 종료 여부 상관 없이
		//socket과 thread를 정리
		// Run의 초기화 실패로 중간 반환한 경우에도 남은 소유 자원을 Shutdown에서 회수한다.
		~CServerApp();
		//서버의 진입점
		// ServerMain이 호출한다. catalog·방·Winsock·listener를 준비한 뒤 두 worker를 시작한다.
		// 실제 제품 호출자는 Server/Private/Main.cpp의 main이다. 별도의 ServerMain 함수는 없다.
		// timeout은 ms이며 0이면 콘솔 입력 또는 headless 종료까지 유지한다. 초기화 실패는 1을 반환한다.
		int Run(
			std::uint32_t automaticShutdownMilliseconds = 0,
			std::string_view bindAddress = "0.0.0.0",
			std::uint16_t port = 7777u,
			bool headless = false);

		// Debug의 명시적인 오프라인 복구 진입점이다. Run과 같은 프로세스 잠금으로 동시 실행을 막는다.
		// 저장 포인터와 journal의 이전/다음 identity를 검증하고 다음 시작 세대를 설치본으로 돌린다.
		// 실행 중 encounter를 초기화하거나 폐기한 후보 파일을 다시 승인하지 않는다.
		static int Reset_ValtanRuntimeToPackaged();

	private:
		struct SERVER_CONTROL_EVENT;
		//접속을 계속 받아 새로운 session을 생성
		// AcceptThread가 실행한다. 연결 소켓의 소유권을 새 ClientSession에 넘기고 수신·송신 worker를 시작한다.
		void Accept_Loop();
		//GameRoom을 일정한 간격으로 tick한다. 초기 30Hz
		// RoomThread가 세대 교체·방 tick·종료 세션 회수를 순서대로 수행한다. gameplay 상태의 쓰기 주체다.
		void Room_Loop();
		//session receive thread가 완성된 frame을 전달하는 callback이다
		// packet을 typed 명령으로 읽고 승인된 방에 enqueue한다. callback 안에서 player 상태를 직접 변경하지 않는다.
		void On_SessionFrame(
			SESSION_ID sessionId,
			const LostArk::Shared::PACKET_FRAME& frame);
		//session receive thread가 연결 종료를 발견했을 때 호출한다.
		// 송신 실패 경로도 이 callback을 부를 수 있다. LEAVE와 종료 회수를 예약하며 자기 worker를 여기서 join하지 않는다.
		void On_SessionClosed(SESSION_ID sessionId);
		//Session map에서 대상을 찾고, request_close만 호출
		// mutex 안에서 강한 참조를 얻고 잠금을 푼 뒤 종료를 요청해 컨테이너 잠금을 socket 처리까지 끌고 가지 않는다.
		void Request_SessionClose(
			SESSION_ID sessionId,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason =
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE,
			int nativeErrorCode = 0,
			std::string_view context = {});
		//종료 callback이 남긴 sessionid를 안전하게 정리
		// RoomThread가 소유 map에서 제거하고 Stop으로 worker를 회수한다. 종료 queue mutex는 회수 대기 중 잡지 않는다.
		void Reap_ClosedSessions();

		// 한 연결의 명령 전달 대상이다. world ID만으로 개인 선택창 방을 구분할 수 없어 owner session도 보관한다.
		struct SESSION_GAMEPLAY_BINDING
		{
			LostArk::Shared::WORLD_ID eWorldId =
				LostArk::Shared::WORLD_ID::END;
			SESSION_ID iPrivateArenaOwnerSessionId =
				INVALID_SESSION_ID;
			std::shared_ptr<CGameRoom> pSimulation;
		};

		// 입장 요청이 쓸 방을 구한다. 공유 월드는 기존 방을 차용하고 선택창은 session 전용 방을 준비한다.
		// 준비 실패는 nullptr이며, 성공만으로 binding이나 실제 입장이 확정되지는 않는다.
		std::shared_ptr<CGameRoom> Acquire_EntrySimulation(
			SESSION_ID sessionId,
			LostArk::Shared::WORLD_ID worldId);
		// m_SessionsMutex 안에서 world의 소유 참조를 복사한다. 등록되지 않은 world는 nullptr다.
		std::shared_ptr<CGameRoom> Find_SharedSimulation(
			LostArk::Shared::WORLD_ID worldId);
		// 수신 경로가 session·세대·방 identity를 다시 검사하고 binding, REGISTER_SESSION, ENTER_WORLD를 연결한다.
		// 실패 위치는 out 인자에 기록한다. 등록 뒤 입장 실패는 LEAVE를 예약해 먼저 들어간 등록을 정리한다.
		bool Bind_AndEnqueueEntry(
			SESSION_ID sessionId,
			LostArk::Shared::WORLD_ID worldId,
			const std::shared_ptr<CGameRoom>& simulation,
			LostArk::Shared::C2S_ENTER_WORLD enterWorld,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outFailureReason,
			int& outNativeErrorCode,
			std::string& outFailureContext);
		// 수신 명령을 현재 binding의 방에 넣는다. lookup과 enqueue를 같은 잠금으로 묶어 월드 전환과 순서를 맞춘다.
		ROOM_COMMAND_ENQUEUE_RESULT Enqueue_AssignedCommand(
			SESSION_ID sessionId,
			ROOM_COMMAND command,
			std::string& outContext);
		// Room_Loop가 전달한 초 단위 고정 간격으로 모든 방을 진행한다. 제어 commit은 첫 방 tick 전에 끝낸다.
		void Tick_GameplaySimulations(float fixedDeltaSeconds,
			const SERVER_ROOM_SCHEDULER_METRICS& schedulerMetrics = {});
		// 수신 callback이 남긴 제어 이벤트를 RoomThread에서 소비하고 세대 교체의 응답·기한·commit을 진행한다.
		void Advance_ServerControlTransactions();
		// 준비/저장 future가 완료된 경우만 결과를 회수한다. 검증 → 모든 방 stage → 디스크 저장을 이어 간다.
		// 평소에는 Room_Loop의 제어 진행에서 호출하고, 종료 중에는 Shutdown이 worker 합류 뒤 이미 영속화 중인 결과를 회수할 때 호출한다.
		void Advance_NumericBalanceTransaction();
		// 숫자 조회는 현재 snapshot으로 응답하고 patch는 기준 revision 검사 뒤 불변 입력을 worker로 넘긴다.
		void Process_NumericBalanceEvent(const SERVER_CONTROL_EVENT& event);
		// 요청 sequence와 최신 숫자 revision을 응답에 담는다. 세션이 없으면 종료하며 송신 실패는 연결 종료를 요청한다.
		void Send_NumericBalanceResult(SESSION_ID sessionId, std::uint32_t sequence,
			LostArk::Shared::BALANCE_APPLY_RESULT result, std::string reason);
		// 저장 성공이면 모든 staged 방과 프로세스 카탈로그를 같은 tick 경계에서 교체한다.
		// 실패면 stage를 취소하고 입장 차단·요청자 상태를 해제한다. 디스크 확정 후 방 commit 실패는 fail-fast한다.
		void Finish_NumericBalanceTransaction(bool commit, std::string status);
		// RoomThread에서 요청자의 현재 발탄 방·boss를 확인하고 제한된 추적 snapshot을 reliable 응답으로 보낸다.
		void Process_ValtanDecisionTraceQuery(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request);
		// 여러 수신 worker의 이벤트를 상한 있는 큐로 옮긴다. 가득 차면 false이고 이벤트를 추가하지 않는다.
		bool Queue_ServerControlEvent(SERVER_CONTROL_EVENT&& event);
		// 후보 상대 경로가 후보 디렉터리 내부의 허용 파일인지 검증하고 물리 경로를 돌려준다.
		static bool Resolve_CandidateArtifactForAdmission(
			const std::filesystem::path& candidateDirectory,
			const std::string& relativePath,
			std::filesystem::path& resolvedPath,
			std::string& status);
		// 전체 bootstrap에서 발탄 외 행의 내용 identity를 계산한다. 발탄 후보가 다른 domain까지 바꾸는지 비교하는 입력이다.
		static bool Build_NonValtanGameplayRevisionForAdmission(
			const std::filesystem::path& bootstrapPath,
			LostArk::Shared::GameplayDataRevision& revision,
			std::string& status);
		// 후보 파일의 실제 bytes를 SHA-256 identity로 만든다. 경로 이름만으로 같은 내용을 승인하지 않는다.
		static bool Hash_GameplayFileForAdmission(
			const std::filesystem::path& path,
			LostArk::Shared::GameplayDataRevision& revision,
			std::string& status);
		// 현재 boss 본체에 이미 복사한 필드는 catalog 교체만으로 바뀌지 않는다.
		// encounter 재설정 transaction이 없으므로 해당 필드를 바꾼 후보는 승인 단계에서 거부한다.
		static bool Validate_ValtanHotReloadBaseProfile(
			const BOSS_RUNTIME_PROFILE* activeProfile,
			const BOSS_RUNTIME_PROFILE* candidateProfile,
			std::string& status);
		// 로그에 쓰기 전 진단 JSON의 구조와 필수 필드를 검증한다. 게임 상태를 변경하지 않는다.
		static bool Validate_ServerSessionDiagnosticJson(
			std::string_view json,
			std::string& status);
		// 이전/다음 identity의 journal과 새 활성 포인터를 기록한다. 이후 메모리 commit이 실패하면 rollback 경로가 소비한다.
		static bool Persist_RuntimeGameplayActivation(
			const std::filesystem::path& runtimeRoot,
			std::uint32_t transactionSequence,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& base,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& candidate,
			std::string& status);
		// 미완료 세대 교체의 디스크 포인터를 base로 되돌린다. 실패 이유는 status에 남겨 복구 실패를 숨기지 않는다.
		static bool Rollback_RuntimeGameplayActivation(
			const std::filesystem::path& runtimeRoot,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& base,
			std::string& status);
		// 메모리 commit까지 끝난 세대 교체의 journal을 정리한다. noexcept 정리 단계다.
		static void Complete_RuntimeGameplayActivation(
			const std::filesystem::path& runtimeRoot) noexcept;
		// 시작 시 저장 포인터와 journal을 검증해 복구할 identity를 정한다. 잘못된 입력은 임의 후보 선택으로 우회하지 않는다.
		static bool Recover_RuntimeActiveGameplayPointer(
			const std::filesystem::path& runtimeRoot,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
			RUNTIME_ACTIVE_GAMEPLAY_GENERATION& active,
			bool& hasPersistedPointer,
			std::string& status,
			bool allowPackagedIdentityDrift = false);
		// 오프라인 복구가 검증한 설치본을 활성 포인터로 기록한다. 일반 입장·tick 중 호출하는 초기화가 아니다.
		static bool Reset_RuntimeGameplayActivationToPackaged(
			const std::filesystem::path& runtimeRoot,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
			std::string& status);
		// 같은 runtime 활성 기록을 두 Server가 동시에 쓰지 못하도록 제품 이름의 Win32 mutex를 얻는다.
		static bool Acquire_RuntimeGameplayProcessMutex(
			void*& handle,
			std::string& status);
		// 제품은 고정 전역 이름을 사용한다. friend 검사는 격리된 이름으로 같은 Win32 소유권 경로를 확인한다.
		static bool Acquire_NamedRuntimeGameplayProcessMutex(
			const wchar_t* name,
			void*& handle,
			std::string& status);
		// 보유한 Win32 mutex와 handle을 해제하고 nullptr로 비워 반복 종료에서도 중복 해제하지 않는다.
		static void Release_RuntimeGameplayProcessMutex(void*& handle) noexcept;
		// 시작 시 복구한 identity에 맞는 설치본 또는 후보 catalog를 로드·검증한다. 성공 결과만 Run이 활성 세대로 보관한다.
		static bool Load_RuntimeActiveGameplayGeneration(
			const std::filesystem::path& runtimeRoot,
			const std::shared_ptr<const CGameplayCatalog>& packagedGeneration,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
			std::shared_ptr<const CGameplayCatalog>& activeGeneration,
			RUNTIME_ACTIVE_GAMEPLAY_GENERATION& active,
			bool& isCandidate,
			std::string& status);
		// Run의 복구, 세대 확정, 수치 저장이 같은 영속 루트를 사용하도록 경로를 계산한다.
		// 검증용 override가 없고 저장소 루트를 찾지 못하면 빈 경로이며 폴더 생성이나 파일 검증은 하지 않는다.
		[[nodiscard]] std::filesystem::path
			Resolve_RuntimeActiveGameplayRoot() const;
		// 대기 중인 모든 방의 stage를 취소하고 참여자에게 실패를 보낸 뒤 transaction을 비운다.
		void Abort_DataRevisionTransaction(std::string reason);
		// 참여자와 기준 세대를 재검사하고 디스크 identity 및 모든 방의 세대를 확정한다. 부분 세대 적용을 정상 성공으로 반환하지 않는다.
		bool Commit_DataRevisionTransaction();
		// prepare 때 고정한 session·binding·simulation이 여전히 동일한지 검사한다. 이탈·교체는 전체 승인 실패다.
		bool Validate_DataRevisionTransactionMembership(
			std::string& status);
		// 참여 Client가 presentation을 준비하도록 후보 revision과 필요한 lane을 reliable 큐에 넣는다.
		bool Send_DataRevisionPrepare(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request);
		// prepare 요청 identity와 최종 활성 revision을 담아 성공/실패를 통지한다. bool은 응답 enqueue 결과다.
		bool Send_DataRevisionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request,
			LostArk::Shared::DATA_REVISION_RESULT result,
			const LostArk::Shared::GameplayDataRevision& activeRevision,
			std::string reason);
		// 퇴장 명령을 소비한 빈 선택창 방의 ingress를 봉한 뒤 소유 map에서 제거한다.
		void Retire_QuiescentCharacterSelectArenas();
		// 참가자가 모두 빠지고 queued 명령도 끝난 콜로세움 경기 방을 제거한다.
		void Retire_QuiescentColosseumMatches();
		// 월드 이동에 필요한 경기 방을 불변 catalog와 취소 flag를 캡처한 worker에서 준비한다. 아직 참가자를 이동하지 않는다.
		bool Begin_ColosseumPreparation(const std::shared_ptr<CGameRoom>& source,
			const SERVER_WORLD_TRANSFER_REQUEST& transfer);
		// 완료된 경기 준비 결과를 회수한다. 세대 저장으로 입장이 잠겼으면 완료 결과를 유지한 채 다음 tick까지 미룬다.
		void Advance_ColosseumPreparation();
		// 방이 확정한 이동 요청을 소비한다. 실제 연결 변경과 reliable 입장 준비는 Transfer_SessionWorld에 모은다.
		void Handle_WorldTransfers(
			const std::shared_ptr<CGameRoom>& sourceSimulation);
		// 이동 실패의 진단과 보상 작업 결과다. source가 보존된 거절과 이미 시작된 이동의 정리를 구분한다.
		struct SESSION_WORLD_TRANSFER_FAILURE final
		{
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON eReason =
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED;
			int iNativeErrorCode = 0;
			std::string strContext;
			bool bRollbackCleanupRequired = false;
			bool bSourcePreservedOnRejection = false;
			bool bRollbackCleanupEnqueued = false;
			LostArk::Shared::PARTY_TRANSFER_RESULT ePartyResult =
				LostArk::Shared::PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE;
		};
		// 파티·가이드·경기 이동은 전체 참가자의 입장과 reliable FIFO 준비 뒤 source·binding을 교체한다.
		// 일반 단독 이동은 목적지 REGISTER/ENTER를 enqueue한 뒤 source 퇴장과 binding을 확정한다.
		// 실패 출력은 source 보존 여부와 필요한 LEAVE 보상 예약 여부를 구분한다.
		bool Transfer_SessionWorld(
			const std::shared_ptr<CGameRoom>& sourceSimulation,
			const SERVER_WORLD_TRANSFER_REQUEST& transfer,
			SESSION_WORLD_TRANSFER_FAILURE& outFailure);
		//서버 전체 종료 순서 한 곳으로 모아서 정리
		// listener 차단 → accept/room join → 준비·저장 worker 회수 → 세션 Stop → 방 제거 → Winsock 해제 순서를 소유한다.
		void Shutdown();

		enum class SERVER_CONTROL_EVENT_KIND : std::uint8_t
		{
			DATA_REVISION_REQUEST,
			DATA_REVISION_RESPONSE,
			SESSION_DISCONNECTED,
			VALTAN_DECISION_TRACE_QUERY,
			BALANCE_QUERY,
			BALANCE_PATCH,
			BALANCE_DEFERRED_ENTRY
		};

		// 수신 스레드에서 룸 스레드로 넘길 값 복사본이다. eKind가 사용할 payload를 고른다.
		// candidate는 읽기 전용 강한 참조로 유지하고 revision 값으로 검증 당시 내용을 묶는다.
		struct SERVER_CONTROL_EVENT final
		{
			SERVER_CONTROL_EVENT_KIND eKind =
				SERVER_CONTROL_EVENT_KIND::SESSION_DISCONNECTED;
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST RevisionRequest{};
			LostArk::Shared::C2S_DATA_REVISION_PREPARE_RESPONSE RevisionResponse{};
			LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY DecisionTraceQuery{};
			LostArk::Shared::C2S_BALANCE_QUERY BalanceQuery{};
			LostArk::Shared::C2S_BALANCE_PATCH BalancePatch{};
			LostArk::Shared::PACKET_FRAME DeferredEntry{};
			std::shared_ptr<const CGameplayCatalog> pCandidateGeneration;
			LostArk::Shared::GameplayDataRevision
				BaseBootstrapContentRevision{};
			LostArk::Shared::GameplayDataRevision
				CandidateBootstrapContentRevision{};
			LostArk::Shared::GameplayDataRevision BaseNonValtanGameplayRevision{};
			LostArk::Shared::GameplayDataRevision
				CandidateNonValtanGameplayRevision{};
		};

		// prepare 당시의 연결과 방을 고정해 두고 응답 및 commit 직전에 동일한 참가자인지 확인한다.
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

		// RoomThread가 진행하는 한 번의 전원 승인 상태다. 요청자 0은 비활성이며 Deadline 이후에는 취소한다.
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

		// 완료 직후 늦게 도착한 정상 응답을 새 transaction 응답과 혼동하지 않도록 짧게 보관하는 identity다.
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

		// 프로세스가 공유 방과 활성 gameplay 세대를 소유한다. 각 방은 같은 읽기 전용 세대를 참조한다.
		std::map<
			LostArk::Shared::WORLD_ID,
			std::shared_ptr<CGameRoom>> m_SharedGameRooms;
		std::shared_ptr<const CGameplayCatalog> m_pActiveGameplayGeneration;
		// manifest 세대 identity와 bootstrap 내용 identity를 분리해 보관한다.
		// 오래된 발탄 후보에 섞인 다른 domain 행이 최신 gameplay를 되돌리는 것을 막는 비교 기준이다.
		LostArk::Shared::GameplayDataRevision
			m_ActiveGameplayBootstrapContentRevision{};
		LostArk::Shared::GameplayDataRevision
			m_ActiveNonValtanGameplayRevision{};
		bool m_isActiveGameplayGenerationFromCandidate = false;
		bool m_isRuntimeActivePersistenceEnabled = false;
		std::filesystem::path m_RuntimeActiveGameplayRootOverride;
		void* m_hRuntimeGameplayProcessMutex = nullptr;

		// 개인 선택창은 owner session ID, 콜로세움은 Server가 발급한 경기 ID로 다른 simulation을 소유한다.
		std::unordered_map<
			SESSION_ID,
			std::shared_ptr<CGameRoom>> m_CharacterSelectArenas;
		std::map<std::uint64_t, std::shared_ptr<CGameRoom>> m_ColosseumMatches;
		std::uint64_t m_iNextColosseumMatchId = 1u;
		// 준비 worker가 돌려주는 방과 실패 문맥이다. ElapsedMilliseconds는 준비 경과 시간(ms)이다.
		struct COLOSSEUM_PREPARATION_RESULT final
		{
			std::shared_ptr<CGameRoom> Room;
			std::string Status;
			double ElapsedMilliseconds = 0.;
		};
		// future의 조회·회수는 RoomThread가 소유한다. worker는 불변 입력과 취소 flag만 캡처한다.
		// this나 살아 있는 session/player 컨테이너는 worker에 넘기지 않는다.
		// 종료 시에는 RoomThread 합류 뒤 Shutdown이 취소를 요청하고 이 future의 남은 결과를 회수한다.
		std::future<COLOSSEUM_PREPARATION_RESULT> m_ColosseumPreparationWorker;
		std::shared_ptr<std::atomic_bool> m_ColosseumPreparationCancelled;
		std::shared_ptr<CGameRoom> m_ColosseumPreparationSource;
		SERVER_WORLD_TRANSFER_REQUEST m_ColosseumPreparationTransfer;
		std::shared_ptr<CGameRoom> m_PreparedColosseumRoom;
		//실행 상태 - 여러 스레드가 읽고 쓰기 때문에 atomic을 사용한다.
		std::atomic_bool m_isRunning{ false };
		std::atomic<SESSION_ID> m_iNextSessionId{ 1 };

		//thread
		// Run이 시작하고 Shutdown이 합류한다. 세션별 송수신 worker는 각 CClientSession이 소유한다.
		std::thread m_AcceptThread;
		std::thread m_RoomThread;
		bool m_bRoomPerformanceLogWarningReported = false;

		//session owner map - sessionid의 유일한 장기 강한 owner
		//gameroom은 같은 session을 weak_ptr로만 참조
		// 이 맵은 연결의 기본 소유자다. 세대 승인 대기 중 DATA_REVISION_PARTICIPANT도 pSession을 강하게 보유하며,
		// 회수·종료 처리의 지역 shared_ptr는 맵에서 제거한 뒤에도 Stop이 끝날 때까지 세션을 유지할 수 있다.
		std::mutex m_SessionsMutex;
		std::unordered_map<
			SESSION_ID,
			std::shared_ptr<CClientSession>> m_Sessions;
		// 세션, binding, 공유 방, 개인 방, 경기 방은 m_SessionsMutex를 함께 사용한다.
		// 방 명령 큐까지 잠글 때는 항상 sessions → room 순서를 유지한다.
		std::unordered_map<
			SESSION_ID,
			SESSION_GAMEPLAY_BINDING> m_GameplayBindingBySessionId;
		//종료 대기 Queue
		std::mutex m_ClosedSessionMutex;
		std::deque<SESSION_ID> m_ClosedSessionIds;
		std::mutex m_SessionDiagnosticLogMutex;
		// 수신 callback은 상한 있는 제어 이벤트만 남긴다. RoomThread는 큐를 비운 뒤 큐 잠금을 놓는다.
		// 이후 방에 접근할 때 sessions → room 잠금 순서를 유지해 수신 경로와 교착을 피한다.
		static constexpr std::size_t MAX_SERVER_CONTROL_EVENTS = 256u;
		std::mutex m_ServerControlMutex;
		std::deque<SERVER_CONTROL_EVENT> m_ServerControlEvents;

		// 입장과 세대 확정이 엇갈리지 않도록 하는 상위 잠금이다. sessions 잠금보다 먼저 얻는다.
		std::mutex m_DataRevisionAdmissionMutex;
		DATA_REVISION_TRANSACTION m_DataRevisionTransaction;
		// 밸런스 worker의 준비/저장 결과다. Prepared가 후보 bytes와 generation의 수명을 함께 유지한다.
		struct BALANCE_WORK_RESULT final
		{
			std::shared_ptr<SERVER_BALANCE_PREPARED> Prepared;
			LostArk::Shared::GameplayDataRevision BootstrapRevision{}, NonValtanRevision{};
			LostArk::Shared::BALANCE_APPLY_RESULT Failure = LostArk::Shared::BALANCE_APPLY_RESULT::INVALID_CHANGE;
			bool Succeeded = false;
			std::string Status;
		};

		// RoomThread가 숫자 세대 상태를 소유한다. worker는 저장소 복사본과 Prepared만 사용한다.
		// Shutdown은 RoomThread 합류 후 이 상태의 정리를 이어받는다. 저장 성공 결과는 방과 store에 확정한 다음 세션을 중지한다.
		CServerBalanceNumericStore m_NumericBalanceStore;
		std::future<BALANCE_WORK_RESULT> m_NumericBalanceWorker;
		BALANCE_WORK_RESULT m_NumericBalancePrepared;
		std::vector<std::shared_ptr<CGameRoom>> m_NumericBalanceSimulations;
		SESSION_ID m_NumericBalanceRequester = INVALID_SESSION_ID;
		std::uint32_t m_NumericBalanceRequestSequence = 0u;
		std::uint32_t m_NumericBalanceTransactionSequence = 0u;
		bool m_NumericBalancePersisting = false;
		// m_DataRevisionAdmissionMutex로 보호한다. 숫자 변경을 디스크에 확정하는 동안만 새 입장을 미룬다.
		bool m_NumericAdmissionPaused = false;

		// 지연 응답 기록에도 상한과 만료를 두어 종료된 transaction 때문에 메모리가 계속 늘지 않게 한다.
		static constexpr std::size_t
			MAX_DATA_REVISION_RESPONSE_TOMBSTONES = 1024u;
		std::deque<DATA_REVISION_RESPONSE_TOMBSTONE>
			m_DataRevisionResponseTombstones;
	};
}
