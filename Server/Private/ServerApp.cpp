#include "ServerApp.h"
#include "ClientSession.h"

#include "Network/PacketMessages.h"
#include "Network/PacketReader.h"

#include <WinSock2.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

LostArk::Server::CServerApp::~CServerApp()
{
	//중간 실패나 정상 종료 여부와 상관없이 thread와 socket을 정리한다.
	Shutdown();
}

int LostArk::Server::CServerApp::Run()
{
	//WinSock 초기화 -> Listener 7777 Open ->
	//Running true -> Room Thread 시작 -> Accept Thread 시작 ->
	//서버 종료 입력 대기 -> shutdown

	//서버의 진입점 - room thread를 먼저 시작한 다음 접속을 받는 게 안전하다.
	//session이 등록되자마자 command를 처리할 room이 준비돼 있기 때문이다

	//WinSock 초기화
	if (!m_WinSockContext.Initialize())
	{
		std::cerr << "Failed to Initialize Winsock 2.2 \n";
		return 1;
	}
	//Listener 7777 Open
	constexpr std::uint16_t SERVER_PORT = 7777;
	//tcplistener를 통해서 server open
	if (!m_TcpListener.Open(SERVER_PORT))
	{
		std::cerr
			<< "Failed to open TCP listener. Error: "
			<< m_TcpListener.Get_LastErrorCode()
			<< '\n';
		return 1;
	}

	//running true
	m_isRunning.store(true);
	//room thread 시작
	m_RoomThread = std::thread(
		&CServerApp::Room_Loop,
		this);

	//Accept Thread 시작
	m_AcceptThread = std::thread(
		&CServerApp::Accept_Loop,
		this);

	//서버 입력 종료 대기 -> shutdown
	std::cout
		<< "Listening on 127.0.0.1:"
		<< SERVER_PORT
		<< '\n'
		<< "Open two Clients, then press Enter to stop.\n";

	std::cin.get();

	Shutdown();
	return 0;
}

void LostArk::Server::CServerApp::Accept_Loop()
{
	//접속을 받아서 새로운 session을 계속해서 생성

	//처리 순서
	//Listener Accept -> SessionId 발급 -> ClientSession 생성
	//Frame Callback 연결 -> closed callback 연결
	//m_sessions에 강한 소유권 등록 -> REGISTER_SESSION command queue
	//Session receive thread start

	//실행상태를 여러 thread가 읽고 쓰기 때문에 atomic을 사용한다.
	while (m_isRunning.load())
	{
		//listener accept
		SOCKET clientSocket = m_TcpListener.Accept();
		//유효하지 않은 소켓이면 error code를 확인하고 반복을 끝낸다.
		if (INVALID_SOCKET == clientSocket)
		{
			if (m_isRunning.load())
			{
				std::cerr
					<< "Accept failed. Error: "
					<< m_TcpListener.Get_LastErrorCode()
					<< '\n';
			}

			break;
		}
		//session id 발급
		const SESSION_ID sessionId =
			m_iNextSessionId.fetch_add(1);

		if (sessionId == INVALID_SESSION_ID)
		{
			::closesocket(clientSocket);
			continue;
		}
		//client session 생성 make_shard
		auto session = std::make_shared<CClientSession>(
			sessionId,
			clientSocket,
			//frame callback 연결
			[this](SESSION_ID id,
				const LostArk::Shared::PACKET_FRAME& frame)
			{
				On_SessionFrame(id, frame);
			},
			//closed callback 연결
			[this](SESSION_ID id)
			{
				On_SessionClosed(id);
			});
		//sessions에 강한 소유권으로 등록
		{
			//lock으로 잠그고, {}를 벗어나면 mutex를 자동으로 해제
			std::scoped_lock lock{ m_SessionsMutex };
			m_Sessions.emplace(sessionId, session);
		}
		//register session command queue
		ROOM_COMMAND registerCommand{};
		registerCommand.eType =
			ROOM_COMMAND_TYPE::REGISTER_SESSION;
		registerCommand.iSessionId = sessionId;
		registerCommand.pSession = session;

		//session receive thread start
		//gameroom에 enqueue를 실패했거나, session 시작을 실패했을 경우
		if (!m_GameRoom.Enqueue(std::move(registerCommand)) ||
			!session->Start())
		{
			//session 닫기
			session->Request_Close();
			On_SessionClosed(sessionId);
			continue;
		}
		std::cout
			<< "Client connected. SessionId="
			<< sessionId
			<< '\n';
	}
}

void LostArk::Server::CServerApp::Room_Loop()
{
	//GameRoom을 일정한 간격으로 Tick() 30Hz로 설정
	//1/30초 주기 -> GameRoom::Tick -> 종료된 Session 정리 ->
	//다음 Tick 시각까지 sleep
	using namespace std::chrono;
	//tick() 30hz로 설정 1/30초 주기
	constexpr float FIXED_DELTA_SECONDS = 1.f / 30.f;
	constexpr auto FIXED_STEP = milliseconds{ 33 };

	while (m_isRunning.load())
	{
		const steady_clock::time_point nextTick =
			steady_clock::now() + FIXED_STEP;

		m_GameRoom.Tick(FIXED_DELTA_SECONDS);
		Reap_ClosedSessions();

		std::this_thread::sleep_until(nextTick);
	}

	//종료된 session 정리
	m_GameRoom.Tick(FIXED_DELTA_SECONDS);
	Reap_ClosedSessions();
}

void LostArk::Server::CServerApp::On_SessionFrame(
	SESSION_ID sessionId,
	const LostArk::Shared::PACKET_FRAME& frame)
{
	//이 함수는 player를 생성하지 않고, Accepted도 보내지 않는다.
	//packet을 번역하는 어댑터 역할만한다. <- 어제 구현했던 packet reader, frame parser
	// Client -> Server 어댑터는 이 함수이고, Server -> Client 송신은
	// CGameRoom이 메시지를 만든 뒤 CClientSession::Send_Frame을 호출하는 경로다.

	//session receive thread가 완성된 frame을 전달하는 callback
	//처리 순서
	//packettype 검사 -> packetreader 생성 -> C2S_ENTER_WORLD 복원
	//-> 남은 payload가 0인지 검사 -> ROOM COMMAND ENTER_WORLD 생성
	//GameRoom::Enqueue

	using namespace LostArk::Shared;

	if (frame.ePacketType != PACKET_TYPE::C2S_ENTER_WORLD)
	{
		Request_SessionClose(sessionId);
		return;
	}

	CPacketReader reader{ frame.Payload };
	C2S_ENTER_WORLD enterWorld{};
	//남은 payload가 0인지 검사
	if (!Read_Message(reader, enterWorld) ||
		0 != reader.Get_RemainingSize())
	{
		Request_SessionClose(sessionId);
		return;
	}
	//room command enter world 생성
	ROOM_COMMAND command{};
	command.eType = ROOM_COMMAND_TYPE::ENTER_WORLD;
	command.iSessionId = sessionId;
	command.EnterWorld = std::move(enterWorld);

	if (!m_GameRoom.Enqueue(std::move(command)))
		Request_SessionClose(sessionId);
}

void LostArk::Server::CServerApp::On_SessionClosed(SESSION_ID sessionId)
{
	//session receive thread가 연결 종료를 발견했을 떄 호출
	//leave command를 gameroom에 enqueue 종료된 sessionid를 정리 queue에 기록
	//여기서 바로 session->stop()을 호출하면 안 된다. 현재 session 자신의 receive thread에서
	//callback이 호출됐을 수 있기 때문에, 자기 자신을 join()하는 문제가 발생할 수 있다.
	ROOM_COMMAND command{};
	command.eType = ROOM_COMMAND_TYPE::LEAVE;
	command.iSessionId = sessionId;
	command.eLeaveReason = LostArk::Shared::PLAYER_DESPAWN_REASON::DISCONNECTED;

	m_GameRoom.Enqueue(std::move(command));

	std::scoped_lock lock{ m_ClosedSessionMutex };
	m_ClosedSessionIds.push_back(sessionId);
}

void LostArk::Server::CServerApp::Request_SessionClose(SESSION_ID sessionId)
{
	//session map에서 대상을 찾고, request close()만 호출한다.
	//mutex 안에서는 session 포인터만 복사하고, 실제 request close는 잠금을 해제한 뒤 실행
	std::shared_ptr<CClientSession> session;

	{
		//mutex를 생성시 잠그고, {} 볌위를 벗어나면 자동으로 해제한다.
		//scoped_lock의 의미 mutex를 생성시, 잠그고, 현재 {} 범위를 벗어나면 자동으로 해제한다.
		std::scoped_lock lock{ m_SessionsMutex };
		const auto iter = m_Sessions.find(sessionId);

		if (iter != m_Sessions.end())
			session = iter->second;
	}

	if (nullptr != session)
		session->Request_Close();
}

void LostArk::Server::CServerApp::Reap_ClosedSessions()
{
	//종료 callback이 남긴 sessionid를 안전하게 정리
	//closed queue를 지역 queue와 swap -> session map에서 shared_ptr 제거
	//mutex 해제 -> session::stop() -> receive thread join -> socket close
	//stop()은 session map mutex 밖에서 호출해야 한다.
	std::deque<SESSION_ID> closedIds;
	//closed queue를 지역 queue와 swap
	{
		std::scoped_lock lock{ m_ClosedSessionMutex };
		closedIds.swap(m_ClosedSessionIds);
	}
	//session map에서 shared_ptr 제거
	for (const SESSION_ID sessionId : closedIds)
	{
		std::shared_ptr<CClientSession> session;

		{
			std::scoped_lock lock{ m_SessionsMutex };
			const auto iter = m_Sessions.find(sessionId);

			if (iter == m_Sessions.end())
				continue;

			session = std::move(iter->second);
			m_Sessions.erase(iter);
		}
		if (nullptr != session)
			session->Stop();
	}
}

void LostArk::Server::CServerApp::Shutdown()
{
	//서버 전체 종료 순서를 한 곳에 모아서 정리
	//Running false ->  Listener Close -> Accept thread join
	//Room thread join -> 모든 session Request_Close -> Session Stop
	//Session map clear -> WinSock shutdown

	//socket을 모두 닫기 전에 wsacleanup()을 호출하면 안 된다.
	m_isRunning.store(false);
	//Tcp listener close
	m_TcpListener.Close();
	//accept thread join
	if (m_AcceptThread.joinable())
		m_AcceptThread.join();
	//room thread join
	if (m_RoomThread.joinable())
		m_RoomThread.join();

	//모든 session request clone,  session map clear
	std::vector<std::shared_ptr<CClientSession>> sessions;

	{
		std::scoped_lock lock{ m_SessionsMutex };

		for (auto& [sessionId, session] : m_Sessions)
		{
			(void)sessionId;
			sessions.push_back(std::move(session));
		}

		m_Sessions.clear();
	}

	//session request close
	for (const auto& session : sessions)
	{
		if (nullptr != session)
			session->Request_Close();
	}
	//session close
	for (const auto& session : sessions)
	{
		if (nullptr != session)
			session->Stop();
	}

	m_WinSockContext.Shutdown();
}
