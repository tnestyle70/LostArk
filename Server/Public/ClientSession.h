#pragma once

#include "ServerIds.h"

#include "Network/NetworkIds.h"
#include "Network/PacketFrame.h"
#include "Network/PacketStreamParser.h"

#include <WinSock2.h>

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <span>
#include <thread>
#include <vector>

namespace LostArk::Server
{
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
		void Request_Close();
		// Stop receiving immediately, drain already queued reliable frames on the
		// sender worker, and close only after the queue becomes empty.  This is
		// used for typed terminal replies such as ROOM_FULL; the room thread never
		// waits for socket I/O.
		void Request_Close_After_Flush();
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

		[[nodiscard]] int Get_LastErrorCode() const;
		[[nodiscard]] CLIENT_SESSION_OUTBOUND_METRICS
			Get_OutboundMetrics() const;

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

		// Sender worker only. Room and receive threads never call send directly.
		bool Send_All(std::span<const std::uint8_t> bytes);

		void Notify_Closed();

	private:
		const SESSION_ID m_iSessionId =
			INVALID_SESSION_ID;

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
