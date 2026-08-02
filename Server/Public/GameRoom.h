#pragma once
//GameRoom에서 실행할 수 있는 명령 header를 추가한다.
//다른 스레드가 room에 요청할 명령 형식이 들어 있다.
//REGISTER_SESSION, ENTER_WORLD, LEAVE
#include "RoomCommand.h"
//Room이 소유할 서버 player 상태이다.
//session id, playerid, net entity id, class, nickname, pos, yaw
#include "ServerPlayer.h"

//들어온 command를 순서대로 저장
#include <deque>
//playerid -> server player, id 순서가 일정해서 spawn 순서 관찰이 쉽다.
#include <map>
//sessionid나 netentityid로 player를 빠르게 찾는 검색 인덱스
#include <unordered_map>
//weak ptr 사용
#include <memory>
//여러 쓰레드가 command queue에 넣을 때 보호
#include <mutex>
//크기 자료형
#include <cstddef>

//GameRoom의 존재 의의와 의미가 뭘까?
//ServerApp에서 RoomCommand를 enque해서 GameRoom으로 보내고,
//GameRoom에서 ClientSession으로 SendFrame과 BroadCast를 하면 된다.
//Player와 Entity Server 정본을 소유.
//ID 발급, 입장, 퇴장, Spawn, Despawn 방송 담당
//Socket을 직접 생성하거나 recv()하지 않는다

//Accept Thread - client 접속 수락, session 생성, REGISTER_SESSION enqueue
//Session Receive Thread - recv, packetStreamParser, 완성된 FrameCallback
//Room Thread - 고정 tick, command queue 소비, player 생성/삭제, packet broadcast

//m_players와 room의 검색 자료구조는 오직 room thread의 tick 경로에서만 변경한다.

namespace LostArk::Server
{
	class CClientSession;

	class CGameRoom final
	{
	public:
		//외부에 공개할 함수 - enqueue, tick
		//enqueue - 다른 스레드가 room 상태를 직접 수정하지 않고, 요청을 전달하기 위한
		//입구다.호출자 - serverapp - accept loop, on session frame,
		//on session closed session 등록 명령, 입장 명령, 퇴장 명령
		bool Enqueue(ROOM_COMMAND command);
		//Room 상태를 변경하는 유일한 실행 지점이다.
		//player 이동, monster brain, boss brain,투사체,skill cooldown,
		//server collison, snapshot 생성 전부 들어오게 된다
		//호출자 - serverapp room loop
		void Tick(float fixedDeltaSeconds);

	private:
		//Session이 Serverapp에 생성됐다는 사실을 room에 등록한다.
		//room은 session을 강하게 소유하지 않고, weak_ptr만 저장한다.
		void Handle_Register(
			const std::shared_ptr<CClientSession>& session
		);
		//C2S_ENTER_WORLD르르 받아서 서버 player를 생성하는 핵심 함수다.
		bool Join(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld
		);
		//접속 종료나 room 퇴장을 처리한다.
		void Leave(
			SESSION_ID sessionId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason
		);
		//서버 player상태를 S2C_PLAYER_SPAWNED로 변환하여, 해당 session에 보낸다.
		bool Send_Accepted(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player
		);
		//서버 player 상태를 S2C_PLAYER_SPAWNED로 변환하여 한 session에 보낸다.
		//playerid, netentityid, character class, nickname, pos, yaw 사용해서 생성
		bool Send_Spawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player
		);
		//한 session에 player 제거 사실을 전송
		bool Send_Despawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason
		);
		//현재 Room 참가자들을 순회하면서 새 player 정보를 전송한다.
		//excepsessionid가 필요한 이유는 신규 client 자신의 spawn은 이미
		//별도로 보냈기 때문이다.중복 Spawn을 막기 위해 신규 session은 제외한다.
		void Broadcast_Spawned(
			const SERVER_PLAYER& player,
			SESSION_ID exceptSessionId
		);
		//현재 room 참가자들을 순회하면서, 새 player 정보를 전송
		void Broadcast_Despawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason
		);
		//Room에 저장한 weak_ptr를 shared_ptr로 잠깐 복원한다.
		std::shared_ptr<CClientSession> Find_Session(
			SESSION_ID sessionId) const;
		//입장 도중 전송 실패가 발생했을 때, 부분적으로 등록된 상태를 되돌린다.
		void Rollback_Join(SESSION_ID sessionId);

	private:
		//command queue - 여러 session thread가 동시에 명령을 넣을 수 있으므로, mutex가 필요
		mutable std::mutex m_CommandMutex;
		std::deque<ROOM_COMMAND> m_InboundCommands;
		//GameRoom은 session을 사용하지만, 소유하지 않는다.
		std::unordered_map<SESSION_ID, std::weak_ptr<CClientSession>> m_Sessions;
		//player owner 서버 player 상태의 진짜 owner
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER> m_Players;
		//검색 인덱스, m_players에 접근하기 위한 자료표
		std::unordered_map<SESSION_ID, LostArk::Shared::PLAYER_ID> m_PlayerIdBySessionId;
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::PLAYER_ID> m_PlayerIdByEntityId;
		//다음 ID
		LostArk::Shared::PLAYER_ID m_iNextPlayerId = 1;
		LostArk::Shared::NET_ENTITY_ID m_iNextNetEntityId = 100;
	};
}