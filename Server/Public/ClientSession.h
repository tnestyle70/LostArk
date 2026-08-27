#pragma once

#include "ServerIds.h"

#include "Network/NetworkIds.h"
#include "Network/PacketFrame.h"
#include "Network/PacketStreamParser.h"
#include "Network/SessionDiagnostic.h"

#include <WinSock2.h>

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace LostArk::Server
{
	class CClientSession;

	struct CLIENT_SESSION_RELIABLE_BATCH final
	{
		std::shared_ptr<CClientSession> pSession;
		std::vector<LostArk::Shared::PACKET_FRAME> Frames;
	};

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

	struct CLIENT_SESSION_PEER_ENDPOINT final
	{
		std::string strAddress = "unknown";
		std::uint16_t iPort = 0u;
	};

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

	class CClientSession final
	{
		friend int Run_ServerGameplayContractTests(bool);
	public:
		// 수신 스레드가 완성한 한 프레임을 ServerApp에 전달하는 계약이다.
		using FRAME_HANDLER = std::function<void(
			SESSION_ID,
			const LostArk::Shared::PACKET_FRAME&)>;

		// 연결 종료를 ServerApp에 한 번만 알리는 계약이다.
		using CLOSED_HANDLER = std::function<void(
			SESSION_ID)>;

		/* A room-thread admission prepares every participant's reliable FIFO
		   before changing gameplay state. Destruction without Commit releases
		   the locks without publishing even one frame. No socket I/O occurs. */
		class RELIABLE_BATCH_TRANSACTION final
		{
		public:
			RELIABLE_BATCH_TRANSACTION();
			~RELIABLE_BATCH_TRANSACTION();
			RELIABLE_BATCH_TRANSACTION(const RELIABLE_BATCH_TRANSACTION&) = delete;
			RELIABLE_BATCH_TRANSACTION& operator=(const RELIABLE_BATCH_TRANSACTION&) = delete;
			bool Prepare(
				const std::vector<CLIENT_SESSION_RELIABLE_BATCH>& batches,
				std::string& status);
			void Commit() noexcept;
		private:
			struct LOCKED_QUEUE;
			std::vector<std::unique_ptr<LOCKED_QUEUE>> m_Queues;
		};

	public:
		CClientSession(
			SESSION_ID sessionId,
			SOCKET clientSocket,
			FRAME_HANDLER onFrame,
			CLOSED_HANDLER onClosed);

		~CClientSession();

		CClientSession(const CClientSession&) = delete;
		CClientSession& operator=(const CClientSession&) = delete;

	public:
		bool Start();
		void Request_Close(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason =
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE,
			int nativeErrorCode = 0,
			std::string_view context = {});
		// Stop receiving immediately, drain already queued reliable frames on the
		// sender worker, and close only after the queue becomes empty.  This is
		// used for typed terminal replies such as ROOM_FULL; the room thread never
		// waits for socket I/O.
		void Request_Close_After_Flush(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason =
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE,
			int nativeErrorCode = 0,
			std::string_view context = {});
		void Stop();

		// Server에서 Client로 한 게임 패킷 프레임을 전송한다.
		bool Send_Frame(
			LostArk::Shared::PACKET_TYPE packetType,
			std::span<const std::uint8_t> payload);

		void Bind_PlayerId(
			LostArk::Shared::PLAYER_ID playerId);

		[[nodiscard]] SESSION_ID Get_SessionId() const;

		[[nodiscard]] LostArk::Shared::PLAYER_ID Get_PlayerId() const;

		[[nodiscard]] bool Is_Open() const;
		[[nodiscard]] bool Is_Closing() const;

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

		static constexpr std::size_t MAX_OUTBOUND_FRAME_COUNT = 128u;
		static constexpr std::size_t RELIABLE_FRAME_RESERVE = 16u;
		static constexpr std::size_t MAX_OUTBOUND_BYTE_COUNT = 512u * 1024u;
		static constexpr std::size_t RELIABLE_BYTE_RESERVE = 128u * 1024u;
		static constexpr std::uint32_t SEND_TIMEOUT_MILLISECONDS = 250u;
		static constexpr std::uint32_t SENDER_JOIN_TIMEOUT_MILLISECONDS = 2000u;

		void Receive_Loop();
		void Sender_Loop();

		bool Receive_Frame(
			LostArk::Shared::PACKET_FRAME& frame);
		OUTBOUND_ENQUEUE_RESULT Queue_OutboundFrame(
			LostArk::Shared::PACKET_TYPE packetType,
			std::vector<std::uint8_t> frameBytes);
		bool Configure_SendTimeout();
		void Close_Socket();
		void Record_InboundPacket(
			LostArk::Shared::PACKET_TYPE packetType) noexcept;
		void Record_TerminalDiagnostic(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
			int nativeErrorCode,
			std::string_view context);
		static CLIENT_SESSION_PEER_ENDPOINT Resolve_PeerEndpoint(
			SOCKET clientSocket) noexcept;

		// Sender worker only. Room and receive threads never call send directly.
		bool Send_All(std::span<const std::uint8_t> bytes);

		void Notify_Closed();

	private:
		const SESSION_ID m_iSessionId =
			INVALID_SESSION_ID;
		const CLIENT_SESSION_PEER_ENDPOINT m_PeerEndpoint;

		std::atomic<SOCKET> m_hClientSocket{ INVALID_SOCKET };

		std::atomic<int> m_iLastErrorCode{ 0 };
		std::atomic_bool m_isReceiveRunning{ false };
		std::atomic_bool m_isSendRunning{ false };
		std::atomic_bool m_closeAfterOutboundFlush{ false };
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

		FRAME_HANDLER m_OnFrame;
		CLOSED_HANDLER m_OnClosed;
	};
}
