#pragma once

#include "ServerIds.h"

#include "Network/NetworkIds.h"
#include "Network/PacketFrame.h"
#include "Network/PacketStreamParser.h"

#include <WinSock2.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <span>
#include <thread>

namespace LostArk::Server
{
	class CClientSession final
	{
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

	private:
		void Receive_Loop();

		bool Receive_Frame(
			LostArk::Shared::PACKET_FRAME& frame);

		// TCP의 부분 전송을 처리해 요청한 모든 바이트를 Socket에 기록한다.
		bool Send_All(std::span<const std::uint8_t> bytes);

		void Notify_Closed();

	private:
		const SESSION_ID m_iSessionId =
			INVALID_SESSION_ID;

		SOCKET m_hClientSocket = INVALID_SOCKET;

		std::atomic<int> m_iLastErrorCode{ 0 };
		std::atomic_bool m_isReceiveRunning{ false };
		std::atomic_bool m_hasNotifiedClosed{ false };

		std::atomic<LostArk::Shared::PLAYER_ID> m_iPlayerId
		{
			LostArk::Shared::INVALID_PLAYER_ID
		};

		LostArk::Shared::CPacketStreamParser m_StreamParser;
		std::thread m_ReceiveThread;
		std::mutex m_SendMutex;

		FRAME_HANDLER m_OnFrame;
		CLOSED_HANDLER m_OnClosed;
	};
}
