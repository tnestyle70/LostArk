#include "GameRoom.h"

#include "ClientSession.h"

//Packet message와 packet writer
#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"

#include <cstdint>
#include <iostream>
#include <utility>

namespace
{
	//C2S_ENTER_WORLD의 character class와 nickname을 바탕으로,
	//유효한 월드인지 검토
	bool Is_Valid_EnterWorld(
		const LostArk::Shared::C2S_ENTER_WORLD& message)
	{
		const std::uint8_t rawClass =
			static_cast<std::uint8_t>(
				message.eCharacterClass);
		//class가 enum end보다 작고, nickname이 있고, 길이가 max nickname bytes 보다 작을 경우
		return rawClass < static_cast<std::uint8_t>(
			LostArk::Shared::CHARACTER_CLASS_ID::END) &&
			!message.strNickName.empty() &&
			message.strNickName.size() <= LostArk::Shared::MAX_NICKNAME_BYTES;
	}
}

bool LostArk::Server::CGameRoom::Enqueue(ROOM_COMMAND command)
{
	//다른 thread가 room 상태를 직접 수정하지 않고, 요청을 전달하기 위한 입구
	//session id 유효성 검사 -> Register라면 session 포인터와 id 일치 확인 -> command mutex 잠금
	//queue 뒤에 command 이동 -> 잠금 해제
	//queue에 명령을 직접 넣는 것까지만 담당한다.
	if (command.iSessionId == INVALID_SESSION_ID)
		return false;

	if (command.eType == ROOM_COMMAND_TYPE::REGISTER_SESSION &&
		(nullptr == command.pSession ||
			command.pSession->Get_SessionId() != command.iSessionId))
	{
		return false;
	}
	//scoped_lock의 의미가 정확하게 뭘까?
	std::scoped_lock lock{ m_CommandMutex };
	m_InboundCommands.push_back(std::move(command));
	return true;
}

void LostArk::Server::CGameRoom::Tick(float fixedDeltaSeconds)
{
	//Room 상태를 변경하는 유일한 실행 지점이다.
	//tick을 server에서 시뮬레이션하면서, 추후에 player 이동, monster brain, boss brain, projectile,
	//skill cooldown, server collision, snapshot을 생성한다.
	(void)fixedDeltaSeconds;
	//ROOM_COMMAND struct를 deque으로 들고 있고, for문을 통한 순회를 통해서,
	//command mutex 잠금
	std::deque<ROOM_COMMAND> commands;
	//이 부분이 이해가 안 간다. 왜 member queue를 지역 queue랑 swap하지?
	//member queue를 지역 queue와 swap
	{
		std::scoped_lock lock{ m_CommandMutex };
		commands.swap(m_InboundCommands);
	}
	//잠금 해제? 지역 queue를 순서대로 처리 -> switch 분기로 COMMAND_TYPE에 따른 처리
	for (ROOM_COMMAND& command : commands)
	{
		switch (command.eType)
		{
		case ROOM_COMMAND_TYPE::REGISTER_SESSION:
			Handle_Register(command.pSession);
			break;
		case ROOM_COMMAND_TYPE::ENTER_WORLD:
			Join(command.iSessionId, command.EnterWorld);
			break;
		case ROOM_COMMAND_TYPE::LEAVE:
			Leave(command.iSessionId, command.eLeaveReason);
			break;
		default:
			break;
		}
	}
}

void LostArk::Server::CGameRoom::Handle_Register(const std::shared_ptr<CClientSession>&session)
{
	//session이 serverapp에 생성됐다는 사실을 room에 등록한다.
	//room은 session을 강하게 소유하지 않고, weak_ptr만 저장한다.
	//강한 owner는 serverapp :: m_session이다.
	//왜 이런 구분이 필요한 걸까?
	if (nullptr == session || session->Get_SessionId() == INVALID_SESSION_ID)
	{
		return;
	}

	m_Sessions.insert_or_assign(
		session->Get_SessionId(), session);
}
//session id와 enter world packet을 매개변수로 보낸 다음 join 함수 동작을 가능하게 한다.
bool LostArk::Server::CGameRoom::Join(SESSION_ID sessionId,
	const LostArk::Shared::C2S_ENTER_WORLD & enterWorld)
{
	//C2S_ENTER_WORLD를 받아 서버 player를 생성한다. player의 생성 위치와 순서도 서버가 결정을 내린다
	//session 검색 -> EnterWorld 데이터 검증 ->
	// 이미 입장한 session인지 검사 -> playerId 발금
	//-> NetEntityId 발급 -> SERVER_PLAYER 생성 ->
	// players와 검색 인덱스에 등록 ->  session에 playerId 연결
	//accepted 전송 -> 기존 player들을 신규 client에게 spawn 전송 ->
	// 신규 client 자신의 spawn 전송 -> 기존 client들에게 신규 player spawn 방송

	using namespace LostArk::Shared;

	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	//session이 존재하지 않거나, enterworld가 유효하지 않은 상태로 들어왔거나,
	//이미 sessionid에 포함되어있거나,
	//왜 next player id, nextnetentityid로 invalid를 검증하는 거지?
	if (nullptr == session || !Is_Valid_EnterWorld(enterWorld) ||
		m_PlayerIdBySessionId.contains(sessionId) ||
		m_iNextPlayerId == INVALID_PLAYER_ID ||
		m_iNextNetEntityId == INVALID_NET_ENTITY_ID)
	{
		if (nullptr != session)
			session->Request_Close();

		return false;
	}
	//player id, net entity id 발급, server player 생성
	SERVER_PLAYER player{};
	player.iSessionId = sessionId;
	player.iPlayerId = m_iNextPlayerId;
	player.iNetEntityId = m_iNextNetEntityId;
	player.eCharacterClass = enterWorld.eCharacterClass;
	player.strNickName = enterWorld.strNickName;

	//두 Client가 겹치지 않도록 server가 임시 spawn 위치를 정한다.
	//나중에, 실제 베른성과 발탄맵 위치 보면서 플레이어 스폰 위치 결정한다.
	player.fPositionX = static_cast<float>(m_Players.size()) * 2.f;
	player.fPositionY = 0.f;
	player.fPositionZ = 0.f;

	player.fYawDegrees = 180.f;

	++m_iNextPlayerId;
	++m_iNextNetEntityId;

	//player와 검색 인덱스에 등록
	m_Players.emplace(player.iPlayerId, player);
	m_PlayerIdBySessionId.emplace(sessionId, player.iPlayerId);
	m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
	//session에 player id 등록
	session->Bind_PlayerId(player.iPlayerId);

	if (!Send_Accepted(session, player))
	{
		Rollback_Join(sessionId);
		session->Request_Close();
		return false;
	}
	//새 Client에게 이미 Room에 있던 Player들을 먼저 알려 준다.
	for (const auto& [existingPlayerId, existingPlayer] : m_Players)
	{
		(void)existingPlayerId;

		if (existingPlayer.iSessionId == sessionId)
			continue;

		if (!Send_Spawned(session, existingPlayer))
		{
			Rollback_Join(sessionId);
			session->Request_Close();
			return false;
		}
	}
	//새 Client가 자기 자신도 생성하게 한다.
	if (!Send_Spawned(session, player))
	{
		Rollback_Join(sessionId);
		session->Request_Close();
		return false;
	}

	//기존 Client들에게 새 Player를 알린다
	Broadcast_Spawned(player, sessionId);


	std::cout
		<< "Player joined. SessionId=" << sessionId
		<< ", PlayerId=" << player.iPlayerId
		<< ", NetEntityId=" << player.iNetEntityId
		<< ", Nickname=" << player.strNickName
		<< ", RoomPlayers=" << m_Players.size()
		<< '\n';

	return true;
}
//session id와 despawnreason을 매개변수로 받아서, leave 함수를 구현한다.
void LostArk::Server::CGameRoom::Leave(SESSION_ID sessionId, LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	//접속 종료나 Room 퇴장을 알린다
	//session id로 playerid 검색 -> player 검색 ->
	//net entity id 보관 -> session의 player id 연결 해제
	//entity 검색 인덱스 제거 -> session 검색 인덱스 제거
	//-> player owner map 제거 -> session weak_ptr 제거
	//남은 client들에게 despawn 방송

	//중요한 순서는 player를 자료구조에서 제거한 뒤 남은 참가자에게 despawn을 보내는 것이다.
	//아직 입장하지 않은 session이 끊겼다면, player는 없기 때문에, session등록만 제거한다

	using namespace LostArk::Shared;
	//왜 이게 iter인 걸까?
	const auto sessionPlayerIter = m_PlayerIdBySessionId.find(sessionId);
	//playeridbysessionid가 end일 경우, sessionplayeriter일 경우 session에서 해당 player 제거
	if (sessionPlayerIter == m_PlayerIdBySessionId.end())
	{
		m_Sessions.erase(sessionId);
		return;
	}

	const PLAYER_ID playerId = sessionPlayerIter->second;
	//server plaeyr map에서 찾기
	const auto playerIter = m_Players.find(playerId);
	//
	if (playerIter == m_Players.end())
	{
		m_PlayerIdBySessionId.erase(sessionPlayerIter);
		m_Sessions.erase(sessionId);
		return;
	}

	const NET_ENTITY_ID netEntityId = playerIter->second.iNetEntityId;

	if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
	{
		session->Bind_PlayerId(INVALID_PLAYER_ID);
	}
	//entity, session id, player, session에서 전부 제거
	m_PlayerIdByEntityId.erase(netEntityId);
	m_PlayerIdBySessionId.erase(sessionPlayerIter);
	m_Players.erase(playerIter);
	m_Sessions.erase(sessionId);

	//남은 플레이어들에게 despawn 방송
	Broadcast_Despawned(netEntityId, reason);
	//server console에 출력
	std::cout
		<< "Player left. SessionId=" << sessionId
		<< ", NetEntityId=" << netEntityId
		<< ", RoomPlayers=" << m_Players.size()
		<< '\n';
}

bool LostArk::Server::CGameRoom::Send_Accepted(const std::shared_ptr<CClientSession>&session, const SERVER_PLAYER& player)
{
	//서버가 입장을 승인했다는 패킷 하나를 만들어, 해당 session에 전송한다.
	//매개변수로 받은 session과 server player에게 전송
	//사용 데이터 - playerid, netEntityId
	//SERVER_PLAYER -> S2C_ENTER_ACCEPTED -> Write Messgae -> Session::Send_Frame
	using namespace LostArk::Shared;

	S2C_ENTER_ACCEPTED message{};
	message.iPlayerId = player.iPlayerId;
	message.iNetEntityId = player.iNetEntityId;

	CPacketWriter writer;

	return nullptr != session &&
		Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_ENTER_ACCEPTED,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_Spawned(const std::shared_ptr<CClientSession>& session, const SERVER_PLAYER& player)
{
	//서버 플레이어 상태를 S2C_PLAYER_SPAWNED로 변환하여, 한 Session에 보낸다.
	//playerid, netentityid, character class, nickname, position, yaw

	//한 명에게 한 Player 정보를 전송하는 함수다.
	using namespace LostArk::Shared;

	S2C_PLAYER_SPAWNED message{};
	message.iPlayerId = player.iPlayerId;
	message.iNetEntityId = player.iNetEntityId;
	message.eCharacterClass = player.eCharacterClass;
	message.strNickName = player.strNickName;
	message.fPositionX = player.fPositionX;
	message.fPositionY = player.fPositionY;
	message.fPositionZ = player.fPositionZ;
	message.fYawDegrees = player.fYawDegrees;

	CPacketWriter writer;

	return nullptr != session &&
		Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_PLAYER_SPAWNED,
			writer.Get_Buffer());
}
//session과 entity의 id, reason을 매개변수로 받는다.
bool LostArk::Server::CGameRoom::Send_Despawned(const std::shared_ptr<CClientSession>& session,
	LostArk::Shared::NET_ENTITY_ID netEntityId, LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	//한 세션에 플레이어 제거 사실을 알린다
	using namespace LostArk::Shared;

	S2C_PLAYER_DESPAWNED message{};

	message.iNetEntityId = netEntityId;
	message.eReason = reason;

	CPacketWriter writer;

	return nullptr != session &&
		Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_PLAYER_DESPAWNED,
			writer.Get_Buffer());
}
//이미 spawn된 session의 경우는 spawn하지 않고 continue를 한다.
void LostArk::Server::CGameRoom::Broadcast_Spawned(const SERVER_PLAYER& player, SESSION_ID exceptSessionId)
{
	//현재 Room 참가자들을 순회하면서 새 Player 정보를 전송한다.
	//exceptsessionid가 필요한 이유는, 신규 client 자신의 spawn은 이미 별도로 전송을 했기 때문에,
	//중복 생성을 방지하기 위해서이다.
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		//exception, 즉 spawn에서 제외될 player, 즉 이미 spawn된 플레이어의 경우는
		//continue를 통해서 생성하지 않는다
		if (sessionId == exceptSessionId)
			continue;

		const std::shared_ptr<CClientSession> session =
			Find_Session(sessionId);

		if (nullptr != session && !Send_Spawned(session, player))
			session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Broadcast_Despawned(LostArk::Shared::NET_ENTITY_ID netEntityId,
	LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;

		const std::shared_ptr<CClientSession> session =
			Find_Session(sessionId);

		if (nullptr != session &&
			!Send_Despawned(session, netEntityId, reason))
		{
			session->Request_Close();
		}
	}
}

std::shared_ptr<LostArk::Server::CClientSession>  LostArk::Server::CGameRoom::Find_Session(SESSION_ID sessionId) const
{
	//Room에 저장된 weak_ptr를 shared_ptr로 잠깐 복원한다
	//GameRoom이 session 수명을 연장하지 않게 하는 중요한 경계.이해가 잘 되지는 않음.
	//왜 GameRoom이 session 수명을 연장하면 안 되는 걸까?
	//SessionId 검색 -> weak_ptr 발견 -> lock() -> 살아있으면 shared_ptr 반환 ->  이미 파괴되었다면 nullptr
	const auto iter = m_Sessions.find(sessionId);

	if (iter == m_Sessions.end())
		return nullptr;

	return iter->second.lock();
}

void LostArk::Server::CGameRoom::Rollback_Join(SESSION_ID sessionId)
{
	//입장 도중 전송 실패가 발생했을 때 부분적으로 등록된 상태를 되돌린다
	//제거 대상은 - m_Players, m_PlayerIdBySessiond, m_PlayerIdByEntityId, Session에 바인딩한 playerId
	using namespace LostArk::Shared;
	//sessionid로 player id 찾기
	const auto sessionPlayerIter = m_PlayerIdBySessionId.find(sessionId);

	if (sessionPlayerIter == m_PlayerIdBySessionId.end())
		return;

	const PLAYER_ID playerId = sessionPlayerIter->second;
	const auto playerIter = m_Players.find(playerId);
	//player id, net  entityid도 삭제하고, player도 제거해준다.
	if (playerIter != m_Players.end())
	{
		m_PlayerIdByEntityId.erase(
			playerIter->second.iNetEntityId);
		m_Players.erase(playerIter);
	}

	m_PlayerIdBySessionId.erase(sessionPlayerIter);

	if (const std::shared_ptr<CClientSession> session =
		Find_Session(sessionId))
	{
		session->Bind_PlayerId(INVALID_PLAYER_ID);
	}
}
