#pragma once

//Room 객체를 값으로 소유
#include "GameRoom.h"
//sessionid 사용
#include "ServerIds.h"
//접속 수락
#include "TcpListener.h"
//winsock 시작과 종료
#include "WinSockContext.h"
//PACKET_FRAME header
#include "Network/PacketFrame.h"

//서버 실행 상태와 다음 sessionid
#include <atomic>
//accept thread와 room thread
#include <thread>
//shared_ptr
#include <memory>
//sessionid로 session 검색
#include <unordered_map>
//session map과 종료 queue 보호
#include <mutex>
//종료된 sessionid 순서 보관
#include <deque>
#include <map>
#include <cstdint>
#include <string_view>

// CServerApp은 서버 전체의 시작, 실행, 종료를 조율하는 최상위 객체다.
// WinSock/Listener 수명, Accept Thread, Room Thread, ClientSession 집합을 소유한다.
// ClientSession이 전달한 Frame을 Shared 메시지로 읽어 ROOM_COMMAND로 번역할 뿐,
// Player 상태 변경과 Broadcast 결정은 CGameRoom에 위임한다.

namespace LostArk::Server
{
	//clientsession 전방 선언
	class CClientSession;

	class CServerApp final
	{
	public:
		//소멸자 - 중간 실패나 정상 종료 여부 상관 없이
		//socket과 thread를 정리
		~CServerApp();
		//서버의 진입점
		int Run(
			std::uint32_t automaticShutdownMilliseconds = 0,
			std::string_view bindAddress = "127.0.0.1");

	private:
		//접속을 계속 받아 새로운 session을 생성
		void Accept_Loop();
		//GameRoom을 일정한 간격으로 tick한다. 초기 30Hz
		void Room_Loop();
		//session receive thread가 완성된 frame을 전달하는 callback이다
		void On_SessionFrame(
			SESSION_ID sessionId,
			const LostArk::Shared::PACKET_FRAME& frame);
		//session receive thread가 연결 종료를 발견했을 때 호출한다.
		void On_SessionClosed(SESSION_ID sessionId);
		//Session map에서 대상을 찾고, request_close만 호출
		void Request_SessionClose(SESSION_ID sessionId);
		//종료 callback이 남긴 sessionid를 안전하게 정리
		void Reap_ClosedSessions();
		CGameRoom* Find_Room(LostArk::Shared::WORLD_ID worldId);
		CGameRoom* Find_AssignedRoom(SESSION_ID sessionId);
		bool Bind_SessionWorld(
			SESSION_ID sessionId,
			LostArk::Shared::WORLD_ID worldId);
		void Handle_WorldTransfers(CGameRoom& sourceRoom);
		bool Transfer_SessionWorld(
			LostArk::Shared::WORLD_ID sourceWorldId,
			const SERVER_WORLD_TRANSFER_REQUEST& transfer);
		void Unbind_SessionWorld(SESSION_ID sessionId);
		//서버 전체 종료 순서 한 곳으로 모아서 정리
		void Shutdown();

	private:
		//서버 기반 객체
		//멤버 순서 중요! 생성 : WinSockContext -> TcpListener
		//소멸 : TcpListener -> WinSockContext 소멸은 역순
		CWinSockContext m_WinSockContext;
		CTcpListener m_TcpListener;
		std::map<
			LostArk::Shared::WORLD_ID,
			std::unique_ptr<CGameRoom>> m_GameRooms;
		//실행 상태 - 여러 스레드가 읽고 쓰기 때문에 atomic을 사용한다.
		std::atomic_bool m_isRunning{ false };
		std::atomic<SESSION_ID> m_iNextSessionId{ 1 };

		//thread
		std::thread m_AcceptThread;
		std::thread m_RoomThread;

		//session owner map - sessionid의 유일한 장기 강한 owner
		//gameroom은 같은 session을 weak_ptr로만 참조
		std::mutex m_SessionsMutex;
		std::unordered_map<
			SESSION_ID,
			std::shared_ptr<CClientSession>> m_Sessions;
		std::unordered_map<
			SESSION_ID,
			LostArk::Shared::WORLD_ID> m_WorldBySessionId;
		//종료 대기 Queue
		std::mutex m_ClosedSessionMutex;
		std::deque<SESSION_ID> m_ClosedSessionIds;
	};
}
