#pragma once

// 연결의 프로세스 내 identity와 미지정 값을 사용한다.
#include "ServerIds.h"

// 방이 승인한 player ID를 연결 ID와 별개로 보관한다.
#include "Network/NetworkIds.h"
// 헤더와 payload를 한 게임 프레임으로 전달하고 송신 bytes로 직렬화한다.
#include "Network/PacketFrame.h"
// TCP 조각을 누적해 완성된 게임 프레임만 수신 callback에 전달한다.
#include "Network/PacketStreamParser.h"
// 종료 원인과 마지막 수신 packet을 ServerApp 진단 경로에 전달한다.
#include "Network/SessionDiagnostic.h"

// accept에서 인수한 연결 SOCKET의 수명과 송수신 API를 사용한다.
#include <WinSock2.h>

// socket handle과 worker 실행·종료 상태를 스레드 사이에서 공유한다.
#include <atomic>
// 송신 큐에 작업이 생겼을 때 worker를 깨우고 종료 완료를 Stop에 알린다.
#include <condition_variable>
// 큐 bytes와 프레임 개수를 컨테이너 크기 타입으로 나타낸다.
#include <cstddef>
// 진단 시각·송신량·포트를 고정 폭 정수로 보관한다.
#include <cstdint>
// reliable 프레임의 FIFO를 유지하며 앞에서 송신할 항목을 꺼낸다.
#include <deque>
// 완성 프레임과 종료 알림을 ServerApp callback으로 전달한다.
#include <functional>
// 여러 세션의 일괄 송신을 준비하는 동안 세션과 잠금 상태의 수명을 소유한다.
#include <memory>
// 진단의 최초 원인 확정과 송신 큐의 복합 변경을 보호한다.
#include <mutex>
// 호출 중만 유효한 payload·송신 bytes 범위를 차용한다.
#include <span>
// 원격 주소와 종료 문맥을 세션 수명 동안 소유한다.
#include <string>
// 종료 요청의 문맥을 차용해 최초 진단에 복사한다.
#include <string_view>
// 수신과 실제 socket 송신을 수행하는 두 worker를 소유한다.
#include <thread>
// 직렬화한 frame bytes와 여러 세션의 준비 결과를 소유한다.
#include <vector>

namespace LostArk::Server
{
	class CServerGameplayContractRunner;

	class CClientSession;

	// 한 세션에 반드시 순서대로 들어갈 프레임 묶음이다. transaction이 준비하는 동안 세션 수명을 유지한다.
	struct CLIENT_SESSION_RELIABLE_BATCH final
	{
		std::shared_ptr<CClientSession> pSession;
		std::vector<LostArk::Shared::PACKET_FRAME> Frames;
	};

	// 송신 큐 잠금 안에서 갱신·복사하는 누적 계측이다. 현재 큐 크기, 최고치, 결과별 개수와 실제 송신 시간을 구분한다.
	struct CLIENT_SESSION_OUTBOUND_METRICS final
	{
		std::size_t iCurrentQueuedFrameCount = 0;
		std::size_t iCurrentQueuedByteCount = 0;
		std::size_t iQueuedFrameHighWatermark = 0;
		std::size_t iQueuedByteHighWatermark = 0;
		std::uint64_t iReliableEnqueuedFrameCount = 0;
		std::uint64_t iReliableRejectedFrameCount = 0;
		std::uint64_t iSnapshotEnqueuedFrameCount = 0;
		std::uint64_t iSnapshotCoalescedFrameCount = 0;
		std::uint64_t iSnapshotDroppedFrameCount = 0;
		std::uint64_t iSentFrameCount = 0;
		std::uint64_t iSentByteCount = 0;
		std::uint64_t iSendFailureCount = 0;
		std::uint64_t iLastFrameSendMicroseconds = 0;
		std::uint64_t iMaximumFrameSendMicroseconds = 0;
	};

	// 생성 시 socket에서 얻는 원격 endpoint다. 이후 바뀌지 않으며 조회 실패는 unknown/0을 유지한다.
	struct CLIENT_SESSION_PEER_ENDPOINT final
	{
		std::string strAddress = "unknown";
		std::uint16_t iPort = 0u;
	};

	// 첫 종료 요청의 원인과 그 시점의 수신·송신 상태다. 후속 종료 요청은 최초 원인을 덮지 않는다.
	struct CLIENT_SESSION_CLOSE_DIAGNOSTIC final
	{
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON eReason =
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE;
		LostArk::Shared::PACKET_TYPE eLastInboundPacket =
			LostArk::Shared::PACKET_TYPE::INVALID;
		int iNativeErrorCode = 0;
		std::uint64_t iOccurredUnixMilliseconds = 0u;
		std::uint64_t iLastInboundUnixMilliseconds = 0u;
		std::size_t iQueuedFrameCountAtClose = 0u;
		std::size_t iQueuedByteCountAtClose = 0u;
		std::string strContext;
	};

	// ServerApp이 연결별로 소유하는 transport 경계다. 완성 프레임만 상위로 전달하고 gameplay를 직접 변경하지 않는다.
	// 연결 socket·두 worker·송신 FIFO를 소유하며, ServerApp은 worker callback 밖에서 Stop으로 수명을 끝낸다.
	class CClientSession final
	{
		friend class CServerGameplayContractRunner;
        friend int Run_ServerCardMazeContractTests();
	public:
		// 수신 스레드가 완성한 한 프레임을 ServerApp에 전달하는 계약이다.
		using FRAME_HANDLER = std::function<void(
			SESSION_ID,
			const LostArk::Shared::PACKET_FRAME&)>;

		// 연결 종료를 ServerApp에 한 번만 알리는 계약이다.
		using CLOSED_HANDLER = std::function<void(
			SESSION_ID)>;

		// 룸 스레드가 gameplay를 바꾸기 전에 모든 참가자의 reliable FIFO를 준비한다.
		// Commit 없이 파괴하면 잠금만 풀고 어느 세션에도 준비 프레임을 공개하지 않는다. socket I/O는 하지 않는다.
		class RELIABLE_BATCH_TRANSACTION final
		{
		public:
			RELIABLE_BATCH_TRANSACTION();
			~RELIABLE_BATCH_TRANSACTION();
			RELIABLE_BATCH_TRANSACTION(const RELIABLE_BATCH_TRANSACTION&) = delete;
			RELIABLE_BATCH_TRANSACTION& operator=(const RELIABLE_BATCH_TRANSACTION&) = delete;
			// session ID 순서로 진단 → 송신 큐를 잠근다. 실패는 status를 남기고 준비한 큐·잠금을 모두 해제한다.
			bool Prepare(
				const std::vector<CLIENT_SESSION_RELIABLE_BATCH>& batches,
				std::string& status);
			// 검증된 큐를 전부 교체하고 잠금을 푼 뒤 sender를 깨운다. 방 상태 확정과 같은 admission 경로가 호출한다.
			void Commit() noexcept;
		private:
			struct LOCKED_QUEUE;
			std::vector<std::unique_ptr<LOCKED_QUEUE>> m_Queues;
		};

	public:
		// Accept_Loop가 발급한 ID와 연결 socket의 소유권을 받는다. callback의 ServerApp은 Stop 완료까지 살아 있어야 한다.
		CClientSession(
			SESSION_ID sessionId,
			SOCKET clientSocket,
			FRAME_HANDLER onFrame,
			CLOSED_HANDLER onClosed);

		// 남은 worker와 socket을 Stop으로 정리한다. worker 자기 자신이 마지막 owner가 되는 사용은 허용하지 않는다.
		~CClientSession();

		CClientSession(const CClientSession&) = delete;
		CClientSession& operator=(const CClientSession&) = delete;

	public:
		// ServerApp이 owner map에 등록한 뒤 호출한다. transport 설정과 두 worker 생성에 성공하면 true다.
		// worker 생성 예외는 종료 요청과 Stop으로 정리한다. 이미 시작됐거나 socket이 없으면 false다.
		bool Start();

		// 첫 진단 원인을 남기고 송수신을 중단한다. 큐를 비우고 shutdown으로 대기를 깨우지만 여기서 worker를 join하지 않는다.
		void Request_Close(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason =
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE,
			int nativeErrorCode = 0,
			std::string_view context = {});
		// 수신은 즉시 멈추되 이미 큐에 든 최종 응답은 sender가 보낸다. 큐가 비거나 종료 유예 기한이 지나면 닫는다.
		// ROOM_FULL 같은 typed 거절 응답을 위한 경로이며 룸 스레드가 socket I/O를 기다리지 않는다.
		void Request_Close_After_Flush(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason =
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE,
			int nativeErrorCode = 0,
			std::string_view context = {});

		// 소유자인 ServerApp이 worker 밖에서 호출한다. socket을 닫고 수신·송신 스레드를 join한다.
		// sender 대기 기한을 넘으면 동기 I/O 취소를 요청한 뒤 최종 join한다.
		void Stop();

		// Server에서 Client로 한 게임 패킷 프레임을 전송한다.
		// 실제 send는 하지 않고 bytes를 송신 큐에 넣는다. true는 전달 완료가 아니며, 압력으로 snapshot을 버린 경우도 포함한다.
		// reliable 큐 초과는 연결 종료와 false로 보고해 중요한 이벤트 유실을 정상 성공으로 숨기지 않는다.
		bool Send_Frame(
			LostArk::Shared::PACKET_TYPE packetType,
			std::span<const std::uint8_t> payload);

		// GameRoom이 승인·해제한 player identity를 기록한다. session ID나 socket의 소유권은 바뀌지 않는다.
		void Bind_PlayerId(
			LostArk::Shared::PLAYER_ID playerId);

		// 생성 시 고정된 연결 ID와 방이 갱신한 player ID다. 둘은 서로 다른 identity다.
		[[nodiscard]] SESSION_ID Get_SessionId() const;

		[[nodiscard]] LostArk::Shared::PLAYER_ID Get_PlayerId() const;

		// Open은 socket handle 존재, Closing은 최초 종료 진단 존재를 뜻한다. graceful flush 중에는 둘 다 true일 수 있다.
		[[nodiscard]] bool Is_Open() const;
		[[nodiscard]] bool Is_Closing() const;

		// 진단 조회는 값을 복사하고 endpoint만 불변 내부 참조를 차용한다. 참조는 세션 수명을 넘겨 보관하지 않는다.
		[[nodiscard]] int Get_LastErrorCode() const;
		[[nodiscard]] CLIENT_SESSION_OUTBOUND_METRICS
			Get_OutboundMetrics() const;
		[[nodiscard]] const CLIENT_SESSION_PEER_ENDPOINT&
			Get_PeerEndpoint() const noexcept;
		[[nodiscard]] CLIENT_SESSION_CLOSE_DIAGNOSTIC
			Get_CloseDiagnostic() const;
		[[nodiscard]] std::uint64_t
			Get_LastInboundUnixMilliseconds() const noexcept;

	private:
		struct OUTBOUND_FRAME final
		{
			LostArk::Shared::PACKET_TYPE ePacketType =
				LostArk::Shared::PACKET_TYPE::INVALID;
			std::vector<std::uint8_t> Bytes;
		};

		enum class OUTBOUND_ENQUEUE_RESULT
		{
			QUEUED,
			COALESCED,
			DROPPED_SNAPSHOT,
			RELIABLE_OVERFLOW,
			CLOSED
		};

		// 상대가 잠시 읽지 않아 전투 이벤트가 몰려도 reliable FIFO를 보존한다. 프레임 수와 bytes에 각각 상한을 둔다.
		// snapshot은 아래 reserve만큼 여유를 남기며 대체·폐기할 수 있고 reliable은 상한 초과를 실패로 처리한다.
		static constexpr std::size_t MAX_OUTBOUND_FRAME_COUNT = 4096u;
		static constexpr std::size_t RELIABLE_FRAME_RESERVE = 16u;
		static constexpr std::size_t MAX_OUTBOUND_BYTE_COUNT = 8u * 1024u * 1024u;
		static constexpr std::size_t RELIABLE_BYTE_RESERVE = 128u * 1024u;
		// socket은 nonblocking이다. 일시적인 송신 압력은 연결 오류로 취급하지 않고 준비 여부를 다시 기다린다.
		static constexpr std::uint32_t TRANSPORT_POLL_MILLISECONDS = 100u;
		static constexpr std::uint32_t SEND_STALL_REPORT_MILLISECONDS = 250u;
		static constexpr std::uint32_t TERMINAL_DRAIN_TIMEOUT_MILLISECONDS = 2000u;
		static constexpr std::uint32_t SENDER_JOIN_TIMEOUT_MILLISECONDS = 2000u;

		// 완성 프레임 callback을 순차 호출한다. graceful flush 종료는 sender에게 종료 알림을 맡긴다.
		void Receive_Loop();
		// 큐 잠금 밖에서 한 프레임을 끝까지 전송한다. 마지막에는 큐와 종료 상태를 정리하고 Stop 대기를 깨운다.
		void Sender_Loop();

		// parser에 남은 완성 프레임을 먼저 꺼낸다. 부족하면 recv bytes를 추가하며 잘못된 헤더·용량 초과·연결 종료를 false로 알린다.
		bool Receive_Frame(
			LostArk::Shared::PACKET_FRAME& frame);
		// 한 큐 안에서 snapshot은 최신 값으로 교체하고 reliable은 FIFO로 누적한다. 상한 검사를 잠금과 함께 수행한다.
		OUTBOUND_ENQUEUE_RESULT Queue_OutboundFrame(
			LostArk::Shared::PACKET_TYPE packetType,
			std::vector<std::uint8_t> frameBytes);
		// Start가 TCP_NODELAY와 nonblocking을 설정한다. 실패하면 진단을 남기고 worker를 시작하지 않는다.
		bool Configure_TransportOptions();
		// atomic exchange로 handle을 회수해 shutdown/closesocket을 한 번만 수행한다.
		void Close_Socket();
		// 완성된 프레임을 얻은 시점의 종류와 Unix ms를 기록한다. TCP 조각 수신 시각과 구분한다.
		void Record_InboundPacket(
			LostArk::Shared::PACKET_TYPE packetType) noexcept;
		// 진단 → 송신 큐 순으로 잠그고 최초 종료 원인·대기량을 고정한다. 기존 원인은 유지한다.
		void Record_TerminalDiagnostic(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
			int nativeErrorCode,
			std::string_view context);
		// 생성 시 getpeername으로 IPv4/IPv6 주소와 host byte order의 포트를 얻는다.
		static CLIENT_SESSION_PEER_ENDPOINT Resolve_PeerEndpoint(
			SOCKET clientSocket) noexcept;

		// sender worker만 실제 send를 호출한다. bytes는 호출이 끝날 때까지 꺼낸 OUTBOUND_FRAME이 소유한다.
		bool Send_All(std::span<const std::uint8_t> bytes,
			LostArk::Shared::PACKET_TYPE packetType = LostArk::Shared::PACKET_TYPE::INVALID);
		// sender의 진행 정지·복구를 용량 제한 JSONL에 기록한다. 로그 실패는 연결 상태를 바꾸지 않는다.
		void Record_SendProgressDiagnostic(const char* eventName,
			LostArk::Shared::PACKET_TYPE packetType, std::size_t sentBytes,
			std::size_t totalBytes, std::uint64_t noProgressMilliseconds) noexcept;

		// 두 worker 중 먼저 도달한 한 번만 ServerApp 종료 callback을 호출한다.
		void Notify_Closed();

	private:
		// 생성 후 불변인 연결 identity와 endpoint다. peer 주소는 진단 용도이며 인증 identity가 아니다.
		const SESSION_ID m_iSessionId =
			INVALID_SESSION_ID;
		const CLIENT_SESSION_PEER_ENDPOINT m_PeerEndpoint;


		// transport의 소유 handle이다. Close_Socket이 INVALID_SOCKET으로 교환한 호출만 OS handle을 닫는다.
		std::atomic<SOCKET> m_hClientSocket{ INVALID_SOCKET };

		std::atomic<int> m_iLastErrorCode{ 0 };
		std::atomic_bool m_isReceiveRunning{ false };
		std::atomic_bool m_isSendRunning{ false };
		std::atomic_bool m_closeAfterOutboundFlush{ false };
		std::atomic<std::uint64_t> m_iTerminalDrainStartTicks{ 0u };
		std::atomic_bool m_hasNotifiedClosed{ false };

		std::atomic<LostArk::Shared::PLAYER_ID> m_iPlayerId
		{
			LostArk::Shared::INVALID_PLAYER_ID
		};
		std::atomic<LostArk::Shared::PACKET_TYPE> m_eLastInboundPacket
		{
			LostArk::Shared::PACKET_TYPE::INVALID
		};
		std::atomic<std::uint64_t> m_iLastInboundUnixMilliseconds{ 0u };
		mutable std::mutex m_DiagnosticMutex;
		CLIENT_SESSION_CLOSE_DIAGNOSTIC m_CloseDiagnostic;


		// stream parser는 수신 worker만 사용한다. outbound queue·계측·sender 종료 표식은 m_OutboundMutex가 보호한다.
		LostArk::Shared::CPacketStreamParser m_StreamParser;
		std::thread m_ReceiveThread;
		std::thread m_SendThread;
		mutable std::mutex m_OutboundMutex;
		std::condition_variable m_OutboundCondition;
		std::condition_variable m_SenderExitCondition;
		std::deque<OUTBOUND_FRAME> m_OutboundFrames;
		std::size_t m_iQueuedOutboundBytes = 0u;
		CLIENT_SESSION_OUTBOUND_METRICS m_OutboundMetrics;
		bool m_hasSenderExited = true;
		std::uint64_t m_iSendStallOrdinal = 0u; // sender worker만 증가시키는 지연 발생 순번이다.


		// 생성 시 받은 callback은 변경하지 않는다. 호출 대상인 ServerApp의 수명은 모든 worker join까지 유지된다.
		FRAME_HANDLER m_OnFrame;
		CLOSED_HANDLER m_OnClosed;
	};
}
