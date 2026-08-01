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

//client 세션은 server 기준으로 클라이언트 세션을 생성해야 한다.

namespace LostArk::Server
{
	class CClientSession final
	{
	public:
		//이 함수 객체들이 의미하는 바가 과연 뭘까?
		//session id와 packet frame을 가지는 함수 객체?
		using FRAME_HANDLER = std::function<void(
			SESSION_ID,
			const LostArk::Shared::PACKET_FRAME&)>;

		using CLOSED_HANDLER = std::function<void(
			SESSION_ID)>;

	public:
		//왜 explicit이 있다가 빠져?
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

		//server -> client로 payload를 packettype에 맞춰서 보낸다.
		bool Send_Frame(
			LostArk::Shared::PACKET_TYPE packetType,
			std::span<const std::uint8_t> payload);

		void Bind_PlayerId(
			LostArk::Shared::PLAYER_ID playerId);

		[[nodiscard]] SESSION_ID Get_SessionId() const;

		[[nodiscard]] LostArk::Shared::PLAYER_ID Get_PlayerId() const;

		[[nodiscard]] bool Is_Open() const
		{
			return INVALID_SOCKET != m_hClientSocket;
		}

		[[nodiscard]] int Get_LastErrorCode() const
		{
			return m_iLastErrorCode;
		}

	private:
		void Receive_Loop();

		bool Receive_Frame(
			LostArk::Shared::PACKET_FRAME& frame);

		//모든 bytes를 tcpclient로 보낸다? GameRoom으로 보낸다?
		bool Send_All(std::span<const std::uint8_t> bytes);

		void Notify_Closed();

	private:
		const SESSION_ID m_iSessionid =
			INVALID_SESSION_ID;

		SOCKET m_hClientSocket = INVALID_SOCKET;
		
		std::atomic<int> m_iLastErrorCode{ 0 };
		std::atomic_bool m_isReceiveRunning{ false };
		std::atomic_bool m_hasNotifiedClosed{ false };

		std::atomic<LostArk::Shared::PLAYER_ID> m_iPlayerId
		{
			LostArk::Shared::INVALID_PLAYER_ID
		};

		//stream parser
		LostArk::Shared::CPacketStreamParser m_StreamParser;
		std::thread m_ReceiveThread;
		std::mutex m_SendMutex;

		FRAME_HANDLER m_OnFrame;
		CLOSED_HANDLER m_OnClosed;
	};
}
