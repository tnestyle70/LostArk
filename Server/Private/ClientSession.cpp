#include "ClientSession.h"

#include <array>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

LostArk::Server::CClientSession::CClientSession(
	SESSION_ID sessionId,
	SOCKET clientSocket,
	FRAME_HANDLER onFrame,
	CLOSED_HANDLER onClosed)
	: m_iSessionId{ sessionId }
	, m_hClientSocket{ clientSocket }
	, m_OnFrame{ std::move(onFrame) }
	, m_OnClosed{ std::move(onClosed) }
{}


LostArk::Server::CClientSession::~CClientSession()
{
	Stop();
}

bool LostArk::Server::CClientSession::Start()
{
	if (!Is_Open() ||
		m_iSessionId == INVALID_SESSION_ID ||
		m_ReceiveThread.joinable())
	{
		return false;
	}

	m_iLastErrorCode.store(0);
	m_hasNotifiedClosed.store(false);
	m_isReceiveRunning.store(true);

	m_ReceiveThread = std::thread(
		&CClientSession::Receive_Loop,
		this);

	return true;
}

void LostArk::Server::CClientSession::Request_Close()
{
	m_isReceiveRunning.store(false);

	if (Is_Open())
		::shutdown(m_hClientSocket, SD_BOTH);
}

void LostArk::Server::CClientSession::Stop()
{
	Request_Close();

	if (m_ReceiveThread.joinable())
		m_ReceiveThread.join();

	std::scoped_lock lock{ m_SendMutex };

	if (Is_Open())
	{
		::closesocket(m_hClientSocket);
		m_hClientSocket = INVALID_SOCKET;
	}
}

bool LostArk::Server::CClientSession::Send_Frame(
	LostArk::Shared::PACKET_TYPE packetType,
	std::span<const std::uint8_t> payload)
{
	std::vector<std::uint8_t> frameBytes;

	if (!LostArk::Shared::Build_Packet_Frame(
		packetType,
		payload,
		frameBytes))
	{
		return false;
	}

	return Send_All(frameBytes);
}

void LostArk::Server::CClientSession::Bind_PlayerId(
	LostArk::Shared::PLAYER_ID playerId)
{
	m_iPlayerId.store(playerId);
}

LostArk::Server::SESSION_ID
LostArk::Server::CClientSession::Get_SessionId() const
{
	return m_iSessionId;
}

LostArk::Shared::PLAYER_ID
LostArk::Server::CClientSession::Get_PlayerId() const
{
	return m_iPlayerId.load();
}

bool LostArk::Server::CClientSession::Is_Open() const
{
	return INVALID_SOCKET != m_hClientSocket;
}

int LostArk::Server::CClientSession::Get_LastErrorCode() const
{
	return m_iLastErrorCode.load();
}

void LostArk::Server::CClientSession::Receive_Loop()
{
	using namespace LostArk::Shared;

	while (m_isReceiveRunning.load())
	{
		PACKET_FRAME frame{};

		if (!Receive_Frame(frame))
			break;

		if (m_OnFrame)
			m_OnFrame(m_iSessionId, frame);
	}

	m_isReceiveRunning.store(false);
	Notify_Closed();
}

bool LostArk::Server::CClientSession::Receive_Frame(
	LostArk::Shared::PACKET_FRAME& frame)
{
	using namespace LostArk::Shared;

	for (;;)
	{
		const PACKET_PARSE_RESULT parseResult =
			m_StreamParser.Try_Pop(frame);

		if (PACKET_PARSE_RESULT::FRAME_READY == parseResult)
			return true;

		if (PACKET_PARSE_RESULT::INVALID_FRAME == parseResult)
		{
			m_iLastErrorCode.store(WSAEPROTONOSUPPORT);
			return false;
		}

		std::array<std::uint8_t, 4096> receiveBuffer{};

		const int receivedByteCount = ::recv(
			m_hClientSocket,
			reinterpret_cast<char*>(receiveBuffer.data()),
			static_cast<int>(receiveBuffer.size()),
			0);

		if (0 == receivedByteCount)
			return false;

		if (SOCKET_ERROR == receivedByteCount)
		{
			const int errorCode = ::WSAGetLastError();

			if (m_isReceiveRunning.load())
				m_iLastErrorCode.store(errorCode);

			return false;
		}

		const std::span<const std::uint8_t> receivedBytes
		{
			receiveBuffer.data(),
			static_cast<std::size_t>(receivedByteCount)
		};

		if (!m_StreamParser.Append(receivedBytes))
		{
			m_iLastErrorCode.store(WSAEMSGSIZE);
			return false;
		}
	}
}

bool LostArk::Server::CClientSession::Send_All(
	std::span<const std::uint8_t> bytes)
{
	std::scoped_lock lock{ m_SendMutex };

	if (!Is_Open())
		return false;

	std::size_t sentByteCount = 0;

	while (sentByteCount < bytes.size())
	{
		const int result = ::send(
			m_hClientSocket,
			reinterpret_cast<const char*>(
				bytes.data() + sentByteCount),
			static_cast<int>(
				bytes.size() - sentByteCount),
			0);

		if (SOCKET_ERROR == result)
		{
			m_iLastErrorCode.store(::WSAGetLastError());
			return false;
		}

		if (0 == result)
			return false;

		sentByteCount += static_cast<std::size_t>(result);
	}

	return true;
}

void LostArk::Server::CClientSession::Notify_Closed()
{
	if (m_hasNotifiedClosed.exchange(true))
		return;

	if (m_OnClosed)
		m_OnClosed(m_iSessionId);
}
