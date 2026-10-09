/* 한국어 학습용 참고 복사본
 * 원본: Server/Private/ClientSession.cpp
 * 제품 소스를 수정하지 않고 함수 본문을 그대로 보존한 문서용 코드다.
 * 원본과 줄 번호가 다르므로 함수 이름으로 대조한다. 제품 프로젝트에 컴파일 항목으로 추가하지 않는다.
 */

#include "ClientSession.h"

#include <Windows.h>
#include <WS2tcpip.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
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

	std::uint64_t Current_UnixMilliseconds()
	{
		return static_cast<std::uint64_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count());
	}
/* 학습 주석
 * 호출자: Receive_Frame 또는 Send_All이 WSAEWOULDBLOCK을 만났을 때 호출한다.
 * 왜: nonblocking 소켓을 반복해서 헛돌지 않고 select로 준비 상태를 기다린다.
 * 범위: 이 함수의 fd_set에는 이 세션의 소켓 하나만 들어간다.
 */
	int Wait_SocketReady(const SOCKET socket, const bool writing,
		const std::uint32_t waitMilliseconds)
	{
		fd_set ready;
		FD_ZERO(&ready);
		FD_SET(socket, &ready);
		const timeval timeout{static_cast<long>(waitMilliseconds / 1000u),
			static_cast<long>((waitMilliseconds % 1000u) * 1000u)};
		return ::select(0, writing ? nullptr : &ready,
			writing ? &ready : nullptr, nullptr, &timeout);
	}

}

/* 학습 주석
 * reliable 송신 준비가 끝날 때까지 세션 수명, 잠금, 임시 큐를 함께 보유한다.
 * 파괴되면 unique_lock이 잠금을 해제한다. Commit 전에는 실제 송신 큐에 게시하지 않는다.
 */
struct LostArk::Server::CClientSession::RELIABLE_BATCH_TRANSACTION::LOCKED_QUEUE
{
	std::shared_ptr<CClientSession> pSession;
	std::unique_lock<std::mutex> DiagnosticLock;
	std::unique_lock<std::mutex> Lock;
	std::deque<OUTBOUND_FRAME> Frames;
	std::size_t iQueuedBytes = 0u;
	std::size_t iAddedFrameCount = 0u;
};

LostArk::Server::CClientSession::RELIABLE_BATCH_TRANSACTION::RELIABLE_BATCH_TRANSACTION() = default;
LostArk::Server::CClientSession::RELIABLE_BATCH_TRANSACTION::~RELIABLE_BATCH_TRANSACTION() = default;

/* 학습 주석
 * 호출자: 방의 입장 준비처럼 gameplay 변경 전에 reliable 송신 공간을 확보하는 코드.
 * 왜: 대상별로 일부만 큐에 들어간 상태에서 입장을 확정하지 않도록 먼저 모두 준비한다.
 * 상태: session ID 순서로 DiagnosticMutex 다음 OutboundMutex를 잠그고 임시 큐를 만든다.
 * 실패: 연결 종료, 중복 세션, 인코딩 또는 용량 실패 시 준비 상태를 버리고 잠금을 푼다. 여기서는 socket send를 하지 않는다.
 */
bool LostArk::Server::CClientSession::RELIABLE_BATCH_TRANSACTION::Prepare(
	const std::vector<CLIENT_SESSION_RELIABLE_BATCH>& batches,
	std::string& status)
{
	using namespace LostArk::Shared;
	m_Queues.clear();
	status.clear();
	const auto reject = [this, &status](const char* reason)
	{
		status = reason;
		m_Queues.clear();
		return false;
	};
	if (batches.empty())
		return reject("reliable admission batch is empty");
	std::vector<const CLIENT_SESSION_RELIABLE_BATCH*> ordered;
	ordered.reserve(batches.size());
	for (const auto& batch : batches)
	{
		if (nullptr == batch.pSession ||
			INVALID_SESSION_ID == batch.pSession->Get_SessionId() ||
			batch.Frames.empty())
		{
			return reject("reliable admission batch has invalid session/frames");
		}
		ordered.push_back(&batch);
	}
	std::sort(ordered.begin(), ordered.end(), [](const auto* left, const auto* right)
	{
		return left->pSession->Get_SessionId() < right->pSession->Get_SessionId();
	});
	for (std::size_t index = 0; index < ordered.size(); ++index)
	{
		const auto& batch = *ordered[index];
		if (0u != index && ordered[index - 1u]->pSession->Get_SessionId() ==
			batch.pSession->Get_SessionId())
		{
			return reject("reliable admission batch contains a duplicate session");
		}
		auto staged = std::make_unique<LOCKED_QUEUE>();
		staged->pSession = batch.pSession;
		// Match Record_TerminalDiagnostic's lock order. A disconnect either
		// wins before admission (reject) or starts after the whole commit.
		staged->DiagnosticLock = std::unique_lock{ batch.pSession->m_DiagnosticMutex };
		staged->Lock = std::unique_lock{ batch.pSession->m_OutboundMutex };
		CClientSession& session = *batch.pSession;
		if (!session.m_isSendRunning.load() || session.m_closeAfterOutboundFlush.load() ||
			SESSION_DIAGNOSTIC_REASON::NONE != session.m_CloseDiagnostic.eReason)
			return reject("reliable admission participant is terminal");
		if (batch.Frames.size() > MAX_OUTBOUND_FRAME_COUNT -
			(std::min)(session.m_OutboundFrames.size(), MAX_OUTBOUND_FRAME_COUNT))
		{
			return reject("reliable admission frame capacity is exhausted");
		}
		staged->Frames = session.m_OutboundFrames;
		staged->iQueuedBytes = session.m_iQueuedOutboundBytes;
		for (const PACKET_FRAME& frame : batch.Frames)
		{
			std::vector<std::uint8_t> bytes;
			if (PACKET_TYPE::S2C_WORLD_SNAPSHOT == frame.ePacketType ||
				!Build_Packet_Frame(frame.ePacketType, frame.Payload, bytes))
			{
				return reject("reliable admission frame failed encoding");
			}
			if (bytes.size() > MAX_OUTBOUND_BYTE_COUNT -
				(std::min)(staged->iQueuedBytes, MAX_OUTBOUND_BYTE_COUNT))
			{
				return reject("reliable admission byte capacity is exhausted");
			}
			staged->iQueuedBytes += bytes.size();
			staged->Frames.push_back({ frame.ePacketType, std::move(bytes) });
		}
		staged->iAddedFrameCount = batch.Frames.size();
		m_Queues.push_back(std::move(staged));
	}
	return true;
}

/* 학습 주석
 * 호출자: Prepare 성공 후 gameplay 변경을 확정하는 owner.
 * 상태: 준비한 큐를 실제 큐와 교체한 다음 모든 잠금을 풀고 sender를 깨운다.
 * 범위: 송신 준비의 게시 완료이지 원격 Client의 수신 확인은 아니다.
 */
void LostArk::Server::CClientSession::RELIABLE_BATCH_TRANSACTION::Commit() noexcept
{
	for (const auto& staged : m_Queues)
	{
		CClientSession& session = *staged->pSession;
		session.m_OutboundFrames.swap(staged->Frames);
		session.m_iQueuedOutboundBytes = staged->iQueuedBytes;
		auto& metrics = session.m_OutboundMetrics;
		metrics.iReliableEnqueuedFrameCount += staged->iAddedFrameCount;
		metrics.iCurrentQueuedFrameCount = session.m_OutboundFrames.size();
		metrics.iCurrentQueuedByteCount = session.m_iQueuedOutboundBytes;
		metrics.iQueuedFrameHighWatermark = (std::max)(
			metrics.iQueuedFrameHighWatermark, metrics.iCurrentQueuedFrameCount);
		metrics.iQueuedByteHighWatermark = (std::max)(
			metrics.iQueuedByteHighWatermark, metrics.iCurrentQueuedByteCount);
	}
	for (const auto& staged : m_Queues)
	{
		staged->Lock.unlock();
		staged->DiagnosticLock.unlock();
	}
	for (const auto& staged : m_Queues)
		staged->pSession->m_OutboundCondition.notify_one();
	m_Queues.clear();
}

/* 학습 주석
 * 호출자: ServerApp::Accept_Loop.
 * 왜: 받아들인 TCP socket과 서버 session ID, 프레임/종료 콜백을 한 수명으로 묶는다.
 * 스레드: 생성 자체는 worker를 시작하지 않으며 Start에서 수신과 송신 스레드를 만든다.
 */
LostArk::Server::CClientSession::CClientSession(
	SESSION_ID sessionId,
	SOCKET clientSocket,
	FRAME_HANDLER onFrame,
	CLOSED_HANDLER onClosed)
	: m_iSessionId{ sessionId }
	, m_PeerEndpoint{ Resolve_PeerEndpoint(clientSocket) }
	, m_hClientSocket{ clientSocket }
	, m_OnFrame{ std::move(onFrame) }
	, m_OnClosed{ std::move(onClosed) }
{}


/* 학습 주석
 * 호출자: 마지막 세션 소유자가 shared_ptr를 놓을 때 호출된다.
 * 왜: 소멸 전에 Stop으로 socket과 worker를 정리해야 this를 사용하는 worker가 남지 않는다.
 */
LostArk::Server::CClientSession::~CClientSession()
{
	Stop();
}

/* 학습 주석
 * 호출자: Accept_Loop.
 * 상태: TCP_NODELAY/nonblocking 설정, 큐와 플래그 초기화 후 세션별 sender와 receiver를 시작한다.
 * 실패: 시작 조건이나 소켓 설정이 잘못되면 false, 스레드 생성 예외는 종료 요청과 Stop으로 정리한다.
 * 현재 구조: 이 파일에는 IOCP worker 연결이 없으며 연결당 두 std::thread를 사용한다.
 */
bool LostArk::Server::CClientSession::Start()
{
	if (!Is_Open() ||
		m_iSessionId == INVALID_SESSION_ID ||
		m_ReceiveThread.joinable() ||
		m_SendThread.joinable())
	{
		Record_TerminalDiagnostic(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_START_FAILED,
			0,
			"invalid session start state");
		return false;
	}
	if (!Configure_TransportOptions())
	{
		return false;
	}

	m_iLastErrorCode.store(0);
	m_hasNotifiedClosed.store(false);
	m_closeAfterOutboundFlush.store(false);
	m_iTerminalDrainStartTicks.store(0u);
	m_isReceiveRunning.store(true);
	m_isSendRunning.store(true);
	{
		std::scoped_lock lock{ m_OutboundMutex };
		m_OutboundFrames.clear();
		m_iQueuedOutboundBytes = 0u;
		m_OutboundMetrics = {};
		m_hasSenderExited = false;
		m_iSendStallOrdinal = 0u;
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
		Request_Close(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_START_FAILED,
			0,
			"session worker creation failed");
		Stop();
		return false;
	}

	return true;
}

/* 학습 주석
 * 호출자: 전송 실패, 명령 오류 또는 서버 종료 경로.
 * 왜: 즉시 종료가 필요하면 큐를 비우고 worker를 깨우며 shutdown으로 대기를 해제한다.
 * 상태: 첫 종료 사유를 기록하고 수신/송신 실행 플래그를 내린다. thread join과 최종 closesocket은 Stop이 한다.
 */
void LostArk::Server::CClientSession::Request_Close(
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const int nativeErrorCode,
	const std::string_view context)
{
	Record_TerminalDiagnostic(reason, nativeErrorCode, context);
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

/* 학습 주석
 * 호출자: ROOM_FULL 같은 마지막 reliable 응답을 보낸 뒤 연결을 닫는 경로.
 * 왜: 수신은 멈추되 이미 큐에 넣은 응답을 sender가 보낼 기회를 주어야 한다.
 * 상태: 처음 요청한 시각을 종료 제한의 기준으로 보존하고 송신 worker에 마무리를 맡긴다.
 * 실패: 동시에 강제 종료가 진행되면 그 경로로 합류한다. room 스레드가 socket 송신을 기다리지는 않는다.
 */
void LostArk::Server::CClientSession::Request_Close_After_Flush(
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const int nativeErrorCode,
	const std::string_view context)
{
	Record_TerminalDiagnostic(reason, nativeErrorCode, context);
	if (!m_isSendRunning.load())
	{
		Request_Close(reason, nativeErrorCode, context);
		return;
	}

	/* Keep the send half alive for the sender worker.  The receive worker must
	   not call the hard-close path when SD_RECEIVE wakes it, otherwise it would
	   clear the terminal reliable frame that this operation promises to drain. */
	// The first explicit close owns the deadline. Repeated close requests must
	// not extend a rejected session; active sessions never get this deadline.
	std::uint64_t unsetStart = 0u;
	(void)m_iTerminalDrainStartTicks.compare_exchange_strong(unsetStart,
		(std::max)(std::uint64_t{1u}, static_cast<std::uint64_t>(GetTickCount64())));
	m_closeAfterOutboundFlush.store(true);
	m_isReceiveRunning.store(false);
	if (!m_isSendRunning.load())
	{
		// A hard close can overtake the first running-state check.  In that
		// case neither worker still owns the graceful notification handoff.
		Request_Close(reason, nativeErrorCode, context);
		Notify_Closed();
		return;
	}
	const SOCKET clientSocket = m_hClientSocket.load();
	if (INVALID_SOCKET != clientSocket)
		(void)::shutdown(clientSocket, SD_RECEIVE);
	m_OutboundCondition.notify_all();
}

/* 학습 주석
 * 호출자: ServerApp의 세션 회수/종료와 소멸자.
 * 상태: 종료를 요청하고 socket을 닫은 뒤 수신·송신 worker를 join한다.
 * 주의: sender 종료 조건은 제한 시간 뒤 취소를 시도하지만 최종 std::thread::join 자체는 시간 제한 함수가 아니다.
 * 현재 동작을 모든 스케줄에서 종료 시간이 보장된다고 과장하지 않는다.
 */
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
			Record_TerminalDiagnostic(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SEND_ERROR_OR_TIMEOUT,
				WSAETIMEDOUT,
				"sender worker join timed out");
			(void)::CancelSynchronousIo(
				reinterpret_cast<HANDLE>(m_SendThread.native_handle()));
		}
		m_SendThread.join();
	}
}

/* 학습 주석
 * 호출자: GameRoom의 snapshot/응답 전송 코드와 ServerApp.
 * 왜: gameplay 스레드가 느린 socket send를 직접 기다리지 않도록 frame을 만들어 큐에 맡긴다.
 * 성공: 큐 정책이 받아들였다는 뜻이며 원격 수신 완료가 아니다. snapshot 생략도 허용된 성공이다.
 * 실패: frame 생성 실패, 닫힌 세션, reliable 용량 초과를 처리한다. reliable 초과는 연결을 닫는다.
 */
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
		Request_Close(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_RELIABLE_OUTBOUND_OVERFLOW,
			WSAENOBUFS,
			"reliable outbound queue capacity exceeded");
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

bool LostArk::Server::CClientSession::Is_Closing() const
{
	std::scoped_lock lock{ m_DiagnosticMutex };
	return LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE !=
		m_CloseDiagnostic.eReason;
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

const LostArk::Server::CLIENT_SESSION_PEER_ENDPOINT&
LostArk::Server::CClientSession::Get_PeerEndpoint() const noexcept
{
	return m_PeerEndpoint;
}

LostArk::Server::CLIENT_SESSION_CLOSE_DIAGNOSTIC
LostArk::Server::CClientSession::Get_CloseDiagnostic() const
{
	std::scoped_lock lock{ m_DiagnosticMutex };
	return m_CloseDiagnostic;
}

std::uint64_t
LostArk::Server::CClientSession::Get_LastInboundUnixMilliseconds() const noexcept
{
	return m_iLastInboundUnixMilliseconds.load();
}

/* 학습 주석
 * 실행 주체: 이 세션의 수신 스레드.
 * 흐름: Receive_Frame으로 완성된 frame 하나를 얻고 m_OnFrame으로 ServerApp에 바로 전달한다.
 * 왜: callback도 이 수신 스레드에서 실행되며, gameplay 변경은 ServerApp이 방 큐에 넣은 뒤 room에서 한다.
 * 실패: 수신 종료 후 즉시 종료 또는 마지막 응답을 비우는 경로의 소유권에 맞춰 한 번만 알린다.
 */
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

/* 학습 주석
 * 실행 주체: 이 세션의 송신 스레드.
 * 흐름: condition variable 대기 → 잠금 안에서 큐의 첫 frame 이동 → 잠금 해제 → Send_All.
 * 왜: 송신 대기 동안 큐 잠금을 잡지 않아 room의 새 응답 접수를 오래 막지 않게 한다.
 * 종료: 남은 큐와 지표를 정리하고 종료 조건을 알린다. 게임 상태를 직접 변경하지 않는다.
 */
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
		const bool sent = Send_All(frame.Bytes, frame.ePacketType);
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

/* 학습 주석
 * 호출자: Send_Frame. 여러 생산자가 만질 수 있는 송신 deque를 mutex로 보호한다.
 * 정책: WORLD_SNAPSHOT은 대기 중인 이전 snapshot을 없애고 최신 것으로 교체할 수 있다. 다른 종류는 reliable FIFO다.
 * 왜: 느린 연결에 오래된 상태가 계속 쌓이는 것을 줄이면서 reliable 공간을 남긴다.
 * 실패: snapshot 혼잡은 생략할 수 있고 reliable 초과는 구분하여 호출자에게 돌려준다.
 */
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

/* 학습 주석
 * 호출자: Start.
 * 왜: 작은 게임 메시지의 묶음 대기를 줄이도록 TCP_NODELAY를 켜고, send/recv의 긴 blocking을 피하도록 nonblocking을 설정한다.
 * 실패: 설정 API 오류를 진단에 남기고 세션 시작을 거부한다.
 */
bool LostArk::Server::CClientSession::Configure_TransportOptions()
{
	const SOCKET clientSocket = m_hClientSocket.load();
	if (INVALID_SOCKET == clientSocket)
		return false;
	/* Small input and snapshot frames must leave immediately instead of
	waiting for Nagle's batching and the peer's delayed acknowledgement. */
	const BOOL noDelay = TRUE;
	if (SOCKET_ERROR == ::setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY,
		reinterpret_cast<const char*>(&noDelay), static_cast<int>(sizeof(noDelay))))
	{
		const int errorCode = ::WSAGetLastError();
		m_iLastErrorCode.store(errorCode);
		Record_TerminalDiagnostic(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_START_FAILED,
			errorCode, "failed to disable TCP movement batching");
		return false;
	}
	// A blocking SO_SNDTIMEO expiry leaves the stream indeterminate. Use
	// nonblocking send/recv and readiness waits so only WSAEWOULDBLOCK retries.
	u_long nonblocking = 1u;
	if (SOCKET_ERROR == ::ioctlsocket(clientSocket, FIONBIO, &nonblocking))
	{
		const int errorCode = ::WSAGetLastError();
		m_iLastErrorCode.store(errorCode);
		Record_TerminalDiagnostic(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_START_FAILED,
			errorCode,
			"failed to configure nonblocking session transport");
		return false;
	}
	return true;
}

/* 학습 주석
 * 호출자: Stop. atomic exchange로 유효 socket을 한 번만 가져와 닫는다.
 * 왜: 여러 종료 요청이 같은 socket 값을 중복해서 closesocket하지 않도록 소유권을 INVALID_SOCKET으로 옮긴다.
 */
void LostArk::Server::CClientSession::Close_Socket()
{
	const SOCKET clientSocket = m_hClientSocket.exchange(INVALID_SOCKET);
	if (INVALID_SOCKET == clientSocket)
		return;
	::shutdown(clientSocket, SD_BOTH);
	::closesocket(clientSocket);
}

/* 학습 주석
 * 호출자: Receive_Loop. parser 상태는 이 수신 스레드가 순서대로 다룬다.
 * 흐름: 기존 버퍼 Try_Pop → 부족하면 recv → Append → 다시 Try_Pop.
 * 왜: recv 한 번과 게임 패킷 한 개의 크기는 같지 않다. 분할 수신과 여러 frame 동시 수신을 함께 처리한다.
 * 실패: 잘못된 header, 누적 버퍼 초과, peer 종료, 실제 Winsock 오류를 구분한다. WSAEWOULDBLOCK은 대기 후 재시도한다.
 */
bool LostArk::Server::CClientSession::Receive_Frame(
	LostArk::Shared::PACKET_FRAME& frame)
{
	using namespace LostArk::Shared;

	for (;;)
	{
		if (!m_isReceiveRunning.load()) return false;
		const PACKET_PARSE_RESULT parseResult =
			m_StreamParser.Try_Pop(frame);

		if (PACKET_PARSE_RESULT::FRAME_READY == parseResult)
		{
			Record_InboundPacket(frame.ePacketType);
			return true;
		}

		if (PACKET_PARSE_RESULT::INVALID_FRAME == parseResult)
		{
			m_iLastErrorCode.store(WSAEPROTONOSUPPORT);
			Record_TerminalDiagnostic(
				SESSION_DIAGNOSTIC_REASON::SERVER_INVALID_FRAME,
				WSAEPROTONOSUPPORT,
				"packet header or frame contract invalid");
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
		{
			Record_TerminalDiagnostic(
				SESSION_DIAGNOSTIC_REASON::SERVER_PEER_CLOSED,
				0,
				"peer completed orderly TCP shutdown");
			return false;
		}

		if (SOCKET_ERROR == receivedByteCount)
		{
			int errorCode = ::WSAGetLastError();
			if (WSAEWOULDBLOCK == errorCode)
			{
				if (!m_isReceiveRunning.load()) return false;
				if (SOCKET_ERROR != Wait_SocketReady(clientSocket, false,
					TRANSPORT_POLL_MILLISECONDS)) continue;
				errorCode = ::WSAGetLastError();
			}

			if (m_isReceiveRunning.load())
			{
				m_iLastErrorCode.store(errorCode);
				Record_TerminalDiagnostic(
					SESSION_DIAGNOSTIC_REASON::SERVER_RECEIVE_ERROR,
					errorCode,
					"recv failed while session was active");
			}

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
			Record_TerminalDiagnostic(
				SESSION_DIAGNOSTIC_REASON::SERVER_PARSER_OVERFLOW,
				WSAEMSGSIZE,
				"packet stream parser buffer capacity exceeded");
			return false;
		}
	}
}

/* 학습 주석
 * 호출자: Sender_Loop만 호출한다.
 * 상태: sentByteCount는 이 frame에서 이미 소켓에 넘긴 바이트 수이며 부분 송신 후 남은 구간부터 이어 보낸다.
 * 왜: send가 모든 바이트를 한 번에 받는다는 가정을 하면 frame이 중간에서 잘린다.
 * 실패: WSAEWOULDBLOCK은 select로 기다리고, 실제 오류나 명시적 종료는 중단한다. 진행 정지 진단과 terminal drain 제한을 구분한다.
 */
bool LostArk::Server::CClientSession::Send_All(
	std::span<const std::uint8_t> bytes,
	const LostArk::Shared::PACKET_TYPE packetType)
{
	std::size_t sentByteCount = 0;
	auto lastProgress = std::chrono::steady_clock::now();
	bool stalled = false;
	bool reportStall = false;
	const auto fail = [&](const int errorCode, const char* reason)
	{
		if (m_isSendRunning.load())
		{
			m_iLastErrorCode.store(errorCode);
			const auto noProgressMs = To_Microseconds(
				std::chrono::steady_clock::now() - lastProgress) / 1000u;
			char context[256]{};
			std::snprintf(context, sizeof(context),
				"%s; sentFrameBytes=%zu; totalFrameBytes=%zu; noProgressMs=%llu",
				reason, sentByteCount, bytes.size(),
				static_cast<unsigned long long>(noProgressMs));
			Record_SendProgressDiagnostic("send.terminal", packetType,
				sentByteCount, bytes.size(), noProgressMs);
			Record_TerminalDiagnostic(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SEND_ERROR_OR_TIMEOUT,
				errorCode, context);
		}
		return false;
	};
	while (sentByteCount < bytes.size())
	{
		if (!m_isSendRunning.load()) return false;
		if (m_closeAfterOutboundFlush.load() &&
			GetTickCount64() - m_iTerminalDrainStartTicks.load() >= TERMINAL_DRAIN_TIMEOUT_MILLISECONDS)
		{
			Record_SendProgressDiagnostic("send.terminal-drain-expired", packetType,
				sentByteCount, bytes.size(),
				To_Microseconds(std::chrono::steady_clock::now() - lastProgress) / 1000u);
			// This cancels an already terminal drain, not active backpressure.
			// Request_Close preserves the original ROOM_FULL reason and context.
			Request_Close();
			return false;
		}
		const SOCKET clientSocket = m_hClientSocket.load();
		if (INVALID_SOCKET == clientSocket) return false;
		const int result = ::send(clientSocket,
			reinterpret_cast<const char*>(bytes.data() + sentByteCount),
			static_cast<int>(bytes.size() - sentByteCount), 0);
		if (SOCKET_ERROR == result)
		{
			const int errorCode = ::WSAGetLastError();
			if (WSAEWOULDBLOCK != errorCode)
				return fail(errorCode, "nonblocking send failed while session was active");
			const auto noProgressMs = To_Microseconds(
				std::chrono::steady_clock::now() - lastProgress) / 1000u;
			if (!stalled && noProgressMs >= SEND_STALL_REPORT_MILLISECONDS)
			{
				stalled = true;
				++m_iSendStallOrdinal;
				reportStall = (m_iSendStallOrdinal & (m_iSendStallOrdinal - 1u)) == 0u;
				if (reportStall) Record_SendProgressDiagnostic("send.stalled", packetType,
					sentByteCount, bytes.size(), noProgressMs);
			}
			if (SOCKET_ERROR == Wait_SocketReady(clientSocket, true, TRANSPORT_POLL_MILLISECONDS))
				return fail(::WSAGetLastError(), "send readiness wait failed");
			continue;
		}
		if (0 == result) return fail(WSAECONNRESET, "send returned zero bytes");
		sentByteCount += static_cast<std::size_t>(result);
		if (stalled && reportStall) Record_SendProgressDiagnostic("send.recovered", packetType,
			sentByteCount, bytes.size(), To_Microseconds(std::chrono::steady_clock::now() - lastProgress) / 1000u);
		lastProgress = std::chrono::steady_clock::now();
		stalled = false;
		reportStall = false;
	}
	return true;
}

/* 학습 주석
 * 호출자: Send_All의 정지·회복·실패 경로.
 * 왜: 보낸 양과 진행이 멈춘 시간을 JSONL로 남겨 socket 혼잡을 조사한다.
 * 범위: lock의 대기 관계나 순환을 추적하는 deadlock profiler는 아니다. 로그 실패는 전송 결과를 바꾸지 않는다.
 */
void LostArk::Server::CClientSession::Record_SendProgressDiagnostic(
	const char* eventName, const LostArk::Shared::PACKET_TYPE packetType,
	const std::size_t sentBytes, const std::size_t totalBytes,
	const std::uint64_t noProgressMilliseconds) noexcept
{
	try
	{
		static std::mutex fileMutex;
		std::scoped_lock lock{fileMutex};
		wchar_t modulePath[32768]{};
		const auto length = GetModuleFileNameW(nullptr, modulePath, 32768u);
		if (!length || length >= 32768u) return;
		const auto directory = std::filesystem::path(modulePath).parent_path() / L"Diagnostics";
		std::error_code error;
		std::filesystem::create_directories(directory, error);
		if (error) return;
		const auto path = directory / (L"server-send-progress-" +
			std::to_wstring(GetCurrentProcessId()) + L".jsonl");
		const auto size = std::filesystem::file_size(path, error);
		if (!error && size >= 2u * 1024u * 1024u)
		{
			const auto previous = path.wstring() + L".previous";
			std::filesystem::remove(previous, error);
			if (error) return;
			std::filesystem::rename(path, previous, error);
			if (error) return;
		}
		std::ofstream log{path, std::ios::binary | std::ios::app};
		if (!log) return;
		const auto now = Current_UnixMilliseconds();
		const auto inbound = m_iLastInboundUnixMilliseconds.load();
		const bool terminalDrain = m_closeAfterOutboundFlush.load();
		const auto drainStart = m_iTerminalDrainStartTicks.load();
		const auto terminal = Get_CloseDiagnostic();
		log << "{\"schema\":\"lostark.server-send-progress\",\"formatVersion\":1"
			<< ",\"unixMs\":" << now << ",\"processId\":" << GetCurrentProcessId()
			<< ",\"sessionId\":" << m_iSessionId << ",\"peerAddress\":\"" << m_PeerEndpoint.strAddress
			<< "\",\"peerPort\":" << m_PeerEndpoint.iPort << ",\"event\":\"" << eventName
			<< "\",\"packetType\":" << static_cast<unsigned>(packetType)
			<< ",\"stallOrdinal\":" << m_iSendStallOrdinal << ",\"sentFrameBytes\":" << sentBytes
			<< ",\"totalFrameBytes\":" << totalBytes << ",\"noProgressMs\":" << noProgressMilliseconds
			<< ",\"closeOnBackpressure\":false"
			<< ",\"terminalDrainRequested\":" << (terminalDrain ? "true" : "false")
			<< ",\"terminalDrainElapsedMs\":" << (terminalDrain ? GetTickCount64() - drainStart : 0u)
			<< ",\"terminalDrainLimitMs\":" << (terminalDrain ? TERMINAL_DRAIN_TIMEOUT_MILLISECONDS : 0u)
			<< ",\"terminalReason\":\"" << LostArk::Shared::To_SessionDiagnosticReasonName(terminal.eReason) << '"'
			<< ",\"lastInboundAgeMs\":" << (inbound && now >= inbound ? now - inbound : 0u) << "}\n";
	}
	catch (...) { } // Diagnostic failures cannot change stream progress or closure.
}

void LostArk::Server::CClientSession::Record_InboundPacket(
	const LostArk::Shared::PACKET_TYPE packetType) noexcept
{
	m_eLastInboundPacket.store(packetType);
	m_iLastInboundUnixMilliseconds.store(Current_UnixMilliseconds());
}

/* 학습 주석
 * 호출자: 최초의 종료 사유를 기록하는 경로들.
 * 상태: DiagnosticMutex 다음 OutboundMutex 순서로 종료 원인과 당시 큐 크기를 저장한다.
 * 왜: 후속 정리 요청이 최초 원인을 덮어쓰지 않게 이미 사유가 있으면 반환한다.
 */
void LostArk::Server::CClientSession::Record_TerminalDiagnostic(
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const int nativeErrorCode,
	const std::string_view context)
{
	using LostArk::Shared::SESSION_DIAGNOSTIC_REASON;
	if (SESSION_DIAGNOSTIC_REASON::NONE == reason)
		return;

	std::scoped_lock lock{ m_DiagnosticMutex };
	if (SESSION_DIAGNOSTIC_REASON::NONE != m_CloseDiagnostic.eReason)
		return;
	m_CloseDiagnostic.eReason = reason;
	m_CloseDiagnostic.eLastInboundPacket = m_eLastInboundPacket.load();
	m_CloseDiagnostic.iNativeErrorCode = nativeErrorCode;
	m_CloseDiagnostic.iOccurredUnixMilliseconds = Current_UnixMilliseconds();
	m_CloseDiagnostic.iLastInboundUnixMilliseconds =
		m_iLastInboundUnixMilliseconds.load();
	{
		std::scoped_lock outboundLock{ m_OutboundMutex };
		m_CloseDiagnostic.iQueuedFrameCountAtClose =
			m_OutboundFrames.size();
		m_CloseDiagnostic.iQueuedByteCountAtClose =
			m_iQueuedOutboundBytes;
	}
	m_CloseDiagnostic.strContext.assign(context.begin(), context.end());
}

LostArk::Server::CLIENT_SESSION_PEER_ENDPOINT
LostArk::Server::CClientSession::Resolve_PeerEndpoint(
	const SOCKET clientSocket) noexcept
{
	CLIENT_SESSION_PEER_ENDPOINT endpoint{};
	if (INVALID_SOCKET == clientSocket)
		return endpoint;

	sockaddr_storage peer{};
	int peerLength = static_cast<int>(sizeof(peer));
	if (SOCKET_ERROR == ::getpeername(
		clientSocket,
		reinterpret_cast<sockaddr*>(&peer),
		&peerLength))
	{
		return endpoint;
	}

	std::array<char, INET6_ADDRSTRLEN> address{};
	if (AF_INET == peer.ss_family)
	{
		const auto* ipv4 = reinterpret_cast<const sockaddr_in*>(&peer);
		if (nullptr != ::InetNtopA(
			AF_INET, const_cast<IN_ADDR*>(&ipv4->sin_addr),
			address.data(), static_cast<DWORD>(address.size())))
		{
			endpoint.strAddress = address.data();
		}
		endpoint.iPort = ::ntohs(ipv4->sin_port);
	}
	else if (AF_INET6 == peer.ss_family)
	{
		const auto* ipv6 = reinterpret_cast<const sockaddr_in6*>(&peer);
		if (nullptr != ::InetNtopA(
			AF_INET6, const_cast<IN6_ADDR*>(&ipv6->sin6_addr),
			address.data(), static_cast<DWORD>(address.size())))
		{
			endpoint.strAddress = address.data();
		}
		endpoint.iPort = ::ntohs(ipv6->sin6_port);
	}
	return endpoint;
}

/* 학습 주석
 * 호출자: 수신·송신 종료 경로.
 * 왜: 양쪽 worker가 종료를 발견해도 atomic exchange로 callback을 한 번만 실행한다.
 * 범위: callback은 현재 호출 worker에서 실행된다. ServerApp은 이를 room의 LEAVE 정리로 연결한다.
 */
void LostArk::Server::CClientSession::Notify_Closed()
{
	if (m_hasNotifiedClosed.exchange(true))
		return;

	if (m_OnClosed)
		m_OnClosed(m_iSessionId);
}
