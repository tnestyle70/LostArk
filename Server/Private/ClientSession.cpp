#include "ClientSession.h"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

namespace
{
	std::uint64_t To_Microseconds(
		const std::chrono::steady_clock::duration duration)
	{
		return static_cast<std::uint64_t>(
			std::chrono::duration_cast<std::chrono::microseconds>(
				duration).count());
	}
}

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
		m_ReceiveThread.joinable() ||
		m_SendThread.joinable() ||
		!Configure_SendTimeout())
	{
		return false;
	}

	m_iLastErrorCode.store(0);
	m_hasNotifiedClosed.store(false);
	m_closeAfterOutboundFlush.store(false);
	m_isReceiveRunning.store(true);
	m_isSendRunning.store(true);
	{
		std::scoped_lock lock{ m_OutboundMutex };
		m_OutboundFrames.clear();
		m_iQueuedOutboundBytes = 0u;
		m_OutboundMetrics = {};
		m_hasSenderExited = false;
	}

	try
	{
		m_SendThread = std::thread(
			&CClientSession::Sender_Loop,
			this);
		m_ReceiveThread = std::thread(
			&CClientSession::Receive_Loop,
			this);
	}
	catch (...)
	{
		Stop();
		return false;
	}

	return true;
}

void LostArk::Server::CClientSession::Request_Close()
{
	m_isReceiveRunning.store(false);
	m_isSendRunning.store(false);
	{
		std::scoped_lock lock{ m_OutboundMutex };
		m_OutboundFrames.clear();
		m_iQueuedOutboundBytes = 0u;
		m_OutboundMetrics.iCurrentQueuedFrameCount = 0u;
		m_OutboundMetrics.iCurrentQueuedByteCount = 0u;
	}
	m_OutboundCondition.notify_all();

	const SOCKET clientSocket = m_hClientSocket.load();
	if (INVALID_SOCKET != clientSocket)
		::shutdown(clientSocket, SD_BOTH);
}

void LostArk::Server::CClientSession::Request_Close_After_Flush()
{
	if (!m_isSendRunning.load())
	{
		Request_Close();
		return;
	}

	/* Keep the send half alive for the sender worker.  The receive worker must
	   not call the hard-close path when SD_RECEIVE wakes it, otherwise it would
	   clear the terminal reliable frame that this operation promises to drain. */
	m_closeAfterOutboundFlush.store(true);
	m_isReceiveRunning.store(false);
	if (!m_isSendRunning.load())
	{
		// A hard close can overtake the first running-state check.  In that
		// case neither worker still owns the graceful notification handoff.
		Request_Close();
		Notify_Closed();
		return;
	}
	const SOCKET clientSocket = m_hClientSocket.load();
	if (INVALID_SOCKET != clientSocket)
		(void)::shutdown(clientSocket, SD_RECEIVE);
	m_OutboundCondition.notify_all();
}

void LostArk::Server::CClientSession::Stop()
{
	Request_Close();
	Close_Socket();

	if (m_ReceiveThread.joinable())
		m_ReceiveThread.join();
	if (m_SendThread.joinable())
	{
		bool senderExited = false;
		{
			std::unique_lock lock{ m_OutboundMutex };
			senderExited = m_SenderExitCondition.wait_for(
				lock,
				std::chrono::milliseconds(
					SENDER_JOIN_TIMEOUT_MILLISECONDS),
				[this]() { return m_hasSenderExited; });
		}
		if (!senderExited)
		{
			m_iLastErrorCode.store(WSAETIMEDOUT);
			(void)::CancelSynchronousIo(
				reinterpret_cast<HANDLE>(m_SendThread.native_handle()));
		}
		m_SendThread.join();
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

	const OUTBOUND_ENQUEUE_RESULT result = Queue_OutboundFrame(
		packetType, std::move(frameBytes));
	if (OUTBOUND_ENQUEUE_RESULT::RELIABLE_OVERFLOW == result)
	{
		Request_Close();
		return false;
	}
	return OUTBOUND_ENQUEUE_RESULT::CLOSED != result;
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
	return INVALID_SOCKET != m_hClientSocket.load();
}

int LostArk::Server::CClientSession::Get_LastErrorCode() const
{
	return m_iLastErrorCode.load();
}

LostArk::Server::CLIENT_SESSION_OUTBOUND_METRICS
LostArk::Server::CClientSession::Get_OutboundMetrics() const
{
	std::scoped_lock lock{ m_OutboundMutex };
	return m_OutboundMetrics;
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

	if (!m_closeAfterOutboundFlush.load())
	{
		Request_Close();
		Notify_Closed();
	}
}

void LostArk::Server::CClientSession::Sender_Loop()
{
	bool isolatedSendFailure = false;
	bool gracefulDrainComplete = false;
	for (;;)
	{
		OUTBOUND_FRAME frame{};
		{
			std::unique_lock lock{ m_OutboundMutex };
			m_OutboundCondition.wait(lock,
				[this]()
				{
					return !m_isSendRunning.load() ||
						!m_OutboundFrames.empty() ||
						m_closeAfterOutboundFlush.load();
				});
			if (!m_isSendRunning.load())
				break;
			if (m_OutboundFrames.empty())
			{
				gracefulDrainComplete =
					m_closeAfterOutboundFlush.load();
				break;
			}

			frame = std::move(m_OutboundFrames.front());
			m_OutboundFrames.pop_front();
			m_iQueuedOutboundBytes -= frame.Bytes.size();
			m_OutboundMetrics.iCurrentQueuedFrameCount =
				m_OutboundFrames.size();
			m_OutboundMetrics.iCurrentQueuedByteCount =
				m_iQueuedOutboundBytes;
		}

		const auto sendStart = std::chrono::steady_clock::now();
		const bool sent = Send_All(frame.Bytes);
		const std::uint64_t sendMicroseconds = To_Microseconds(
			std::chrono::steady_clock::now() - sendStart);
		const bool sendFailedWhileRunning =
			!sent && m_isSendRunning.load();
		{
			std::scoped_lock lock{ m_OutboundMutex };
			m_OutboundMetrics.iLastFrameSendMicroseconds = sendMicroseconds;
			m_OutboundMetrics.iMaximumFrameSendMicroseconds = (std::max)(
				m_OutboundMetrics.iMaximumFrameSendMicroseconds,
				sendMicroseconds);
			if (sent)
			{
				++m_OutboundMetrics.iSentFrameCount;
				m_OutboundMetrics.iSentByteCount += frame.Bytes.size();
			}
			else if (sendFailedWhileRunning)
			{
				++m_OutboundMetrics.iSendFailureCount;
			}
		}
		if (!sent)
		{
			isolatedSendFailure = sendFailedWhileRunning;
			break;
		}
	}

	const bool flushCloseWasHardStopped =
		m_closeAfterOutboundFlush.load() && !gracefulDrainComplete;
	if (isolatedSendFailure || gracefulDrainComplete)
		Request_Close();
	{
		std::scoped_lock lock{ m_OutboundMutex };
		m_OutboundFrames.clear();
		m_iQueuedOutboundBytes = 0u;
		m_OutboundMetrics.iCurrentQueuedFrameCount = 0u;
		m_OutboundMetrics.iCurrentQueuedByteCount = 0u;
		m_hasSenderExited = true;
	}
	m_isSendRunning.store(false);
	m_SenderExitCondition.notify_all();
	if (isolatedSendFailure || gracefulDrainComplete ||
		flushCloseWasHardStopped)
		Notify_Closed();
}

LostArk::Server::CClientSession::OUTBOUND_ENQUEUE_RESULT
LostArk::Server::CClientSession::Queue_OutboundFrame(
	const LostArk::Shared::PACKET_TYPE packetType,
	std::vector<std::uint8_t> frameBytes)
{
	using LostArk::Shared::PACKET_TYPE;
	const bool isSnapshot = PACKET_TYPE::S2C_WORLD_SNAPSHOT == packetType;
	OUTBOUND_ENQUEUE_RESULT result = OUTBOUND_ENQUEUE_RESULT::QUEUED;
	{
		std::scoped_lock lock{ m_OutboundMutex };
		if (!m_isSendRunning.load() ||
			m_closeAfterOutboundFlush.load())
			return OUTBOUND_ENQUEUE_RESULT::CLOSED;

		if (isSnapshot)
		{
			const auto snapshotIter = std::find_if(
				m_OutboundFrames.begin(),
				m_OutboundFrames.end(),
				[](const OUTBOUND_FRAME& queued)
				{
					return PACKET_TYPE::S2C_WORLD_SNAPSHOT ==
						queued.ePacketType;
				});
			if (snapshotIter != m_OutboundFrames.end())
			{
				m_iQueuedOutboundBytes -= snapshotIter->Bytes.size();
				m_OutboundFrames.erase(snapshotIter);
				++m_OutboundMetrics.iSnapshotCoalescedFrameCount;
				result = OUTBOUND_ENQUEUE_RESULT::COALESCED;
			}

			constexpr std::size_t SNAPSHOT_FRAME_LIMIT =
				MAX_OUTBOUND_FRAME_COUNT - RELIABLE_FRAME_RESERVE;
			constexpr std::size_t SNAPSHOT_BYTE_LIMIT =
				MAX_OUTBOUND_BYTE_COUNT - RELIABLE_BYTE_RESERVE;
			if (m_OutboundFrames.size() >= SNAPSHOT_FRAME_LIMIT ||
				frameBytes.size() >
					SNAPSHOT_BYTE_LIMIT - (std::min)(
						m_iQueuedOutboundBytes, SNAPSHOT_BYTE_LIMIT))
			{
				++m_OutboundMetrics.iSnapshotDroppedFrameCount;
				m_OutboundMetrics.iCurrentQueuedFrameCount =
					m_OutboundFrames.size();
				m_OutboundMetrics.iCurrentQueuedByteCount =
					m_iQueuedOutboundBytes;
				return OUTBOUND_ENQUEUE_RESULT::DROPPED_SNAPSHOT;
			}
		}
		else if (m_OutboundFrames.size() >= MAX_OUTBOUND_FRAME_COUNT ||
			frameBytes.size() >
				MAX_OUTBOUND_BYTE_COUNT - (std::min)(
					m_iQueuedOutboundBytes, MAX_OUTBOUND_BYTE_COUNT))
		{
			++m_OutboundMetrics.iReliableRejectedFrameCount;
			return OUTBOUND_ENQUEUE_RESULT::RELIABLE_OVERFLOW;
		}

		m_iQueuedOutboundBytes += frameBytes.size();
		m_OutboundFrames.push_back({ packetType, std::move(frameBytes) });
		if (isSnapshot)
			++m_OutboundMetrics.iSnapshotEnqueuedFrameCount;
		else
			++m_OutboundMetrics.iReliableEnqueuedFrameCount;
		m_OutboundMetrics.iCurrentQueuedFrameCount =
			m_OutboundFrames.size();
		m_OutboundMetrics.iCurrentQueuedByteCount =
			m_iQueuedOutboundBytes;
		m_OutboundMetrics.iQueuedFrameHighWatermark = (std::max)(
			m_OutboundMetrics.iQueuedFrameHighWatermark,
			m_OutboundFrames.size());
		m_OutboundMetrics.iQueuedByteHighWatermark = (std::max)(
			m_OutboundMetrics.iQueuedByteHighWatermark,
			m_iQueuedOutboundBytes);
	}
	m_OutboundCondition.notify_one();
	return result;
}

bool LostArk::Server::CClientSession::Configure_SendTimeout()
{
	const SOCKET clientSocket = m_hClientSocket.load();
	if (INVALID_SOCKET == clientSocket)
		return false;
	const DWORD timeoutMilliseconds = SEND_TIMEOUT_MILLISECONDS;
	if (SOCKET_ERROR == ::setsockopt(
		clientSocket,
		SOL_SOCKET,
		SO_SNDTIMEO,
		reinterpret_cast<const char*>(&timeoutMilliseconds),
		static_cast<int>(sizeof(timeoutMilliseconds))))
	{
		m_iLastErrorCode.store(::WSAGetLastError());
		return false;
	}
	return true;
}

void LostArk::Server::CClientSession::Close_Socket()
{
	const SOCKET clientSocket = m_hClientSocket.exchange(INVALID_SOCKET);
	if (INVALID_SOCKET == clientSocket)
		return;
	::shutdown(clientSocket, SD_BOTH);
	::closesocket(clientSocket);
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

		const SOCKET clientSocket = m_hClientSocket.load();
		if (INVALID_SOCKET == clientSocket)
			return false;

		const int receivedByteCount = ::recv(
			clientSocket,
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
	std::size_t sentByteCount = 0;

	while (sentByteCount < bytes.size())
	{
		if (!m_isSendRunning.load())
			return false;

		const SOCKET clientSocket = m_hClientSocket.load();
		if (INVALID_SOCKET == clientSocket)
			return false;

		const int result = ::send(
			clientSocket,
			reinterpret_cast<const char*>(
				bytes.data() + sentByteCount),
			static_cast<int>(
				bytes.size() - sentByteCount),
			0);

		if (SOCKET_ERROR == result)
		{
			if (m_isSendRunning.load())
				m_iLastErrorCode.store(::WSAGetLastError());
			return false;
		}

		if (0 == result)
		{
			if (m_isSendRunning.load())
				m_iLastErrorCode.store(WSAECONNRESET);
			return false;
		}

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
