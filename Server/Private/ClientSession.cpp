// 연결별 socket·worker·송신 FIFO의 구현이다.
#include "ClientSession.h"

// 송신 I/O 취소, 종료 유예 시각, 실행 파일 위치와 프로세스 ID를 사용한다.
#include <Windows.h>
// peer IPv4/IPv6 주소를 진단 문자열로 바꾼다.
#include <WS2tcpip.h>

// 상한 검사와 session ID 정렬, 대기 snapshot 검색에 사용한다.
#include <algorithm>
// 한 번의 수신 버퍼와 주소 변환 버퍼를 스택에 보관한다.
#include <array>
// 진행 경과는 steady_clock, 진단 시각은 system_clock으로 구분한다.
#include <chrono>
// 송수신 bytes와 누적 시간의 고정 폭 정수를 사용한다.
#include <cstdint>
// 송신 실패 문맥을 고정 크기 버퍼에 작성한다.
#include <cstdio>
// 실행 파일 옆 Diagnostics 경로와 로그 용량·순환 파일을 관리한다.
#include <filesystem>
// 송신 지연 진단을 JSONL에 추가한다.
#include <fstream>
// 받은 bytes와 송신할 bytes를 복사 없이 범위로 전달한다.
#include <span>
// socket callback과 frame bytes의 소유권을 큐·세션으로 이동한다.
#include <utility>
// reliable 일괄 준비와 직렬화 bytes를 소유한다.
#include <vector>

namespace
{
	// 송신 한 프레임의 steady_clock 경과를 계측 단위인 마이크로초로 변환한다.
	std::uint64_t To_Microseconds(
		const std::chrono::steady_clock::duration duration)
	{
		return static_cast<std::uint64_t>(
			std::chrono::duration_cast<std::chrono::microseconds>(
				duration).count());
	}

	// 진단 파일과 종료 기록을 서로 대조할 수 있도록 Unix 기준 ms를 만든다.
	std::uint64_t Current_UnixMilliseconds()
	{
		return static_cast<std::uint64_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count());
	}
	// nonblocking recv/send가 다시 진행 가능한지 유한 시간만 기다린다. select의 오류/시간초과/준비 결과를 호출자에게 보낸다.
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

// 룸 이동의 admission이 모든 세션 FIFO의 후보를 만든다. 실패는 기존 큐를 유지하며 준비 중 잡은 잠금을 해제한다.
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
		// Record_TerminalDiagnostic과 같은 잠금 순서를 쓴다. 먼저 종료됐으면 거부하고,
		// 준비가 먼저 잠금을 얻었으면 종료 기록은 모든 큐의 commit 뒤에 진행된다.
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

// 준비한 FIFO를 전부 교체한 뒤 잠금을 풀고 sender를 깨운다. 이 단계에 socket I/O와 추가 프레임 인코딩은 없다.
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

// Accept_Loop에서 연결 socket을 넘겨받는다. peer를 즉시 복사하고 worker 시작은 별도 Start까지 미룬다.
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


// ServerApp이 worker 밖에서 소유권을 끝낼 때 남은 송수신과 socket을 정리한다.
LostArk::Server::CClientSession::~CClientSession()
{
	Stop();
}

// 설정과 worker 생성을 하나의 시작 경계로 묶는다. 두 번째 worker 생성 실패도 이미 시작한 sender를 정리한다.
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

// 여러 경로가 요청해도 최초 진단 원인을 보존한다. 큐를 비운 뒤 shutdown으로 I/O 대기를 깨운다.
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

// 거절 응답을 큐에 넣은 뒤 호출하는 종료 경로다. 수신을 멈추고 sender에게 최종 송신·종료 알림을 넘긴다.
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

	// sender가 최종 응답을 보낼 수 있도록 송신 방향은 유지한다. SD_RECEIVE로 수신 worker가 깨어나도
	// 즉시 종료 경로로 들어가 큐를 비우지 않도록 graceful 종료 표식을 함께 사용한다.
	// 첫 명시적 종료 요청만 유예 시작 시각을 정한다. 반복 요청이 거절된 연결의 수명을 늘리지 못한다.
	// 정상 사용 중인 연결에는 이 종료 기한을 적용하지 않는다.
	std::uint64_t unsetStart = 0u;
	(void)m_iTerminalDrainStartTicks.compare_exchange_strong(unsetStart,
		(std::max)(std::uint64_t{1u}, static_cast<std::uint64_t>(GetTickCount64())));
	m_closeAfterOutboundFlush.store(true);
	m_isReceiveRunning.store(false);
	if (!m_isSendRunning.load())
	{
		// 첫 실행 상태 검사 뒤 다른 경로가 즉시 종료할 수 있다. 이 경우 worker의 graceful 알림에만
		// 기대지 않고 여기서 종료 callback을 요청한다. Notify_Closed가 중복 호출을 막는다.
		Request_Close(reason, nativeErrorCode, context);
		Notify_Closed();
		return;
	}
	const SOCKET clientSocket = m_hClientSocket.load();
	if (INVALID_SOCKET != clientSocket)
		(void)::shutdown(clientSocket, SD_RECEIVE);
	m_OutboundCondition.notify_all();
}

// 소유자인 ServerApp의 회수 경로가 호출한다. worker 밖에서 socket을 닫고 join하며 sender 지연은 취소를 요청한 뒤 기다린다.
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

// 패킷을 독립된 byte 버퍼로 만든 뒤 큐에 소유권을 넘긴다. 실제 전달 완료와 enqueue 허용 결과를 구분한다.
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

// GameRoom이 승인한 player identity를 진단 조회에 게시한다. 연결 ID는 생성 때 정한 값을 유지한다.
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

// 수신 worker만 parser를 소유한다. 프레임은 callback이 돌아올 때까지 지역 변수로 살아 있다.
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

// FIFO에서 꺼낸 frame은 worker의 지역 소유물이다. mutex를 놓고 보내므로 느린 peer가 enqueue 잠금을 붙잡지 않는다.
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

		// 실제 socket 송신은 outbound 잠금 밖에서 수행한다. 계측 결과만 다시 잠금 안에서 합친다.
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

LostArk::Server::CClientSession::OUTBOUND_ENQUEUE_RESULT
// snapshot의 최신 값 교체와 reliable FIFO 상한을 한 임계 구역에서 판단한다. 상위 Send_Frame이 결과별 종료 정책을 정한다.
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

		// 아직 송신하지 않은 이전 snapshot을 제거해 최신 상태만 남긴다. 이미 worker가 꺼낸 frame은 건드리지 않는다.
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

			// reliable 최종 응답·전투 이벤트가 들어갈 여유를 남기기 위해 snapshot에 더 낮은 상한을 적용한다.
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

// worker 시작 전에 작은 packet의 지연 정책과 nonblocking을 설정한다. 오류는 최초 종료 진단에 남긴다.
bool LostArk::Server::CClientSession::Configure_TransportOptions()
{
	const SOCKET clientSocket = m_hClientSocket.load();
	if (INVALID_SOCKET == clientSocket)
		return false;
	// 작은 입력·snapshot을 Nagle 묶음과 상대 delayed ACK 때문에 기다리지 않도록 TCP_NODELAY를 켠다.
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
	// blocking 송신 시간 초과 뒤의 불명확한 진행 상태를 피한다. nonblocking send/recv와 준비 대기를
	// 사용하고 WSAEWOULDBLOCK만 일시적인 압력으로 재시도한다.
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

// handle을 교환해 정리 권한을 한 호출에만 준다. worker가 이전 handle로 진행 중이면 API 실패로 빠져나온다.
void LostArk::Server::CClientSession::Close_Socket()
{
	const SOCKET clientSocket = m_hClientSocket.exchange(INVALID_SOCKET);
	if (INVALID_SOCKET == clientSocket)
		return;
	::shutdown(clientSocket, SD_BOTH);
	::closesocket(clientSocket);
}

// 누적 버퍼에서 완성 프레임을 먼저 꺼낸다. 모자란 bytes만 recv로 추가하고 오류·종료는 false로 상위 루프에 전달한다.
bool LostArk::Server::CClientSession::Receive_Frame(
	LostArk::Shared::PACKET_FRAME& frame)
{
	using namespace LostArk::Shared;

	for (;;)
	{
		if (!m_isReceiveRunning.load())
			return false;
		// 직전 recv에 여러 frame이 함께 들어왔을 수 있으므로 socket을 더 읽기 전에 parser부터 확인한다.
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
				if (!m_isReceiveRunning.load())
			return false;
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

		// span은 이 반복의 스택 버퍼만 차용한다. Append가 내용을 자체 누적 버퍼에 보관한다.
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

// sender가 한 frame의 남은 bytes를 반복 송신한다. 부분 송신은 offset을 전진시키고 WSAEWOULDBLOCK은 준비 대기 뒤 재시도한다.
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
		if (!m_isSendRunning.load())
			return false;
		if (m_closeAfterOutboundFlush.load() &&
			GetTickCount64() - m_iTerminalDrainStartTicks.load() >= TERMINAL_DRAIN_TIMEOUT_MILLISECONDS)
		{
			Record_SendProgressDiagnostic("send.terminal-drain-expired", packetType,
				sentByteCount, bytes.size(),
				To_Microseconds(std::chrono::steady_clock::now() - lastProgress) / 1000u);
			// 이미 종료가 확정된 연결의 최종 응답 유예만 취소한다. 정상 연결의 일시 송신 압력에는 적용하지 않는다.
			// Request_Close는 먼저 기록한 ROOM_FULL 등의 원인과 문맥을 보존한다.
			Request_Close();
			return false;
		}
		const SOCKET clientSocket = m_hClientSocket.load();
		if (INVALID_SOCKET == clientSocket)
			return false;
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
				// 1, 2, 4, 8번째 지연만 기록해 반복적인 압력으로 로그가 과도하게 늘지 않게 한다.
				reportStall = (m_iSendStallOrdinal & (m_iSendStallOrdinal - 1u)) == 0u;
				if (reportStall) Record_SendProgressDiagnostic("send.stalled", packetType,
					sentByteCount, bytes.size(), noProgressMs);
			}
			if (SOCKET_ERROR == Wait_SocketReady(clientSocket, true, TRANSPORT_POLL_MILLISECONDS))
				return fail(::WSAGetLastError(), "send readiness wait failed");
			continue;
		}
		if (0 == result)
			return fail(WSAECONNRESET, "send returned zero bytes");
		sentByteCount += static_cast<std::size_t>(result);
		if (stalled && reportStall) Record_SendProgressDiagnostic("send.recovered", packetType,
			sentByteCount, bytes.size(), To_Microseconds(std::chrono::steady_clock::now() - lastProgress) / 1000u);
		lastProgress = std::chrono::steady_clock::now();
		stalled = false;
		reportStall = false;
	}
	return true;
}

// 송신 정지·복구 정보를 프로세스별 JSONL 두 파일에 제한해 기록한다. static mutex는 서로 다른 session sender의 파일 접근을 직렬화한다.
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
	catch (...) { } // 진단 실패로 송신 진행이나 연결 종료 결과를 바꾸지 않는다.
}

// 프레임 파싱 성공 시점만 기록하므로 불완전한 TCP 조각이 최근 정상 입력 시각을 갱신하지 않는다.
void LostArk::Server::CClientSession::Record_InboundPacket(
	const LostArk::Shared::PACKET_TYPE packetType) noexcept
{
	m_eLastInboundPacket.store(packetType);
	m_iLastInboundUnixMilliseconds.store(Current_UnixMilliseconds());
}

// 최초 종료 원인과 당시 큐량을 복사한다. Reliable transaction과 같은 진단 → outbound 잠금 순서를 유지한다.
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
// 생성자의 socket 조회 결과를 값으로 보관한다. 주소 변환 실패는 기본 unknown을 남기며 연결을 닫지 않는다.
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

// 수신·송신 worker가 경쟁해도 exchange에서 이긴 한 번만 상위 회수를 예약한다.
void LostArk::Server::CClientSession::Notify_Closed()
{
	if (m_hasNotifiedClosed.exchange(true))
		return;

	if (m_OnClosed)
		m_OnClosed(m_iSessionId);
}
