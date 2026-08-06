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
	Shutdown();
}

int LostArk::Server::CServerApp::Run(
	const std::uint32_t automaticShutdownMilliseconds,
	const std::string_view bindAddress)
{
	using LostArk::Shared::WORLD_ID;

	m_GameRooms.clear();
	m_GameRooms.emplace(WORLD_ID::BERN, std::make_unique<CGameRoom>(WORLD_ID::BERN));
	m_GameRooms.emplace(
		WORLD_ID::VALTAN_ARENA,
		std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA));
	m_GameRooms.emplace(
		WORLD_ID::TRAINING_GROUND,
		std::make_unique<CGameRoom>(WORLD_ID::TRAINING_GROUND));
	m_GameRooms.emplace(
		WORLD_ID::CHARACTER_SELECT_ARENA,
		std::make_unique<CGameRoom>(WORLD_ID::CHARACTER_SELECT_ARENA));
	for (const auto& [worldId, room] : m_GameRooms)
	{
		if (nullptr == room || !room->Is_Ready())
		{
			std::cerr << "World room failed to initialize. World="
				<< static_cast<unsigned>(worldId) << ", Status="
				<< (nullptr == room ? "Room allocation failed" : room->Get_Status())
				<< '\n';
			return 1;
		}
	}

	if (!m_WinSockContext.Initialize())
	{
		std::cerr << "Failed to initialize WinSock 2.2\n";
		return 1;
	}
	constexpr std::uint16_t SERVER_PORT = 7777;
	if (!m_TcpListener.Open(bindAddress, SERVER_PORT))
	{
		std::cerr << "Failed to open TCP listener. Error="
			<< m_TcpListener.Get_LastErrorCode() << '\n';
		return 1;
	}

	m_isRunning.store(true);
	m_RoomThread = std::thread(&CServerApp::Room_Loop, this);
	m_AcceptThread = std::thread(&CServerApp::Accept_Loop, this);
	std::cout << "Listening on " << bindAddress << ':' << SERVER_PORT
		<< " with BERN, VALTAN_ARENA, TRAINING_GROUND, and "
		<< "CHARACTER_SELECT_ARENA.";
	if (0u == automaticShutdownMilliseconds)
	{
		std::cout << " Press Enter to stop.\n";
		std::cin.get();
	}
	else
	{
		std::cout << " Smoke timeout=" << automaticShutdownMilliseconds
			<< "ms.\n";
		std::this_thread::sleep_for(
			std::chrono::milliseconds(automaticShutdownMilliseconds));
	}
	Shutdown();
	return 0;
}

void LostArk::Server::CServerApp::Accept_Loop()
{
	while (m_isRunning.load())
	{
		const SOCKET clientSocket = m_TcpListener.Accept();
		if (INVALID_SOCKET == clientSocket)
		{
			if (m_isRunning.load())
			{
				std::cerr << "Accept failed. Error="
					<< m_TcpListener.Get_LastErrorCode() << '\n';
			}
			break;
		}

		const SESSION_ID sessionId = m_iNextSessionId.fetch_add(1);
		if (sessionId == INVALID_SESSION_ID)
		{
			::closesocket(clientSocket);
			continue;
		}
		auto session = std::make_shared<CClientSession>(
			sessionId,
			clientSocket,
			[this](const SESSION_ID id, const LostArk::Shared::PACKET_FRAME& frame)
			{
				On_SessionFrame(id, frame);
			},
			[this](const SESSION_ID id)
			{
				On_SessionClosed(id);
			});
		{
			std::scoped_lock lock{ m_SessionsMutex };
			m_Sessions.emplace(sessionId, session);
		}
		if (!session->Start())
		{
			session->Request_Close();
			On_SessionClosed(sessionId);
			continue;
		}
		std::cout << "Client connected. SessionId=" << sessionId << '\n';
	}
}

void LostArk::Server::CServerApp::Room_Loop()
{
	using namespace std::chrono;
	constexpr duration<double> FIXED_STEP_SECONDS{ 1.0 / 30.0 };
	constexpr float FIXED_DELTA_SECONDS =
		static_cast<float>(FIXED_STEP_SECONDS.count());
	const steady_clock::duration fixedStep =
		duration_cast<steady_clock::duration>(FIXED_STEP_SECONDS);
	steady_clock::time_point nextTick = steady_clock::now();

	while (m_isRunning.load())
	{
		nextTick += fixedStep;
		for (const auto& [worldId, room] : m_GameRooms)
		{
			(void)worldId;
			room->Tick(FIXED_DELTA_SECONDS);
			Handle_WorldTransfers(*room);
		}
		Reap_ClosedSessions();
		std::this_thread::sleep_until(nextTick);
		if (steady_clock::now() > nextTick + fixedStep)
			nextTick = steady_clock::now();
	}
	for (const auto& [worldId, room] : m_GameRooms)
	{
		(void)worldId;
		room->Tick(FIXED_DELTA_SECONDS);
		Handle_WorldTransfers(*room);
	}
	Reap_ClosedSessions();
}

void LostArk::Server::CServerApp::On_SessionFrame(
	const SESSION_ID sessionId,
	const LostArk::Shared::PACKET_FRAME& frame)
{
	using namespace LostArk::Shared;

	ROOM_COMMAND command{};
	command.iSessionId = sessionId;
	CGameRoom* targetRoom = nullptr;
	CPacketReader reader{ frame.Payload };

	if (frame.ePacketType == PACKET_TYPE::C2S_ENTER_WORLD)
	{
		C2S_ENTER_WORLD enterWorld{};
		if (!Read_Message(reader, enterWorld) || 0u != reader.Get_RemainingSize() ||
			enterWorld.iProtocolVersion != NETWORK_PROTOCOL_VERSION ||
			!Is_Known_World_Id(enterWorld.eWorldId))
		{
			Request_SessionClose(sessionId);
			return;
		}
		targetRoom = Find_Room(enterWorld.eWorldId);
		if (nullptr == targetRoom || !Bind_SessionWorld(sessionId, enterWorld.eWorldId))
		{
			Request_SessionClose(sessionId);
			return;
		}

		std::shared_ptr<CClientSession> session;
		{
			std::scoped_lock lock{ m_SessionsMutex };
			const auto iter = m_Sessions.find(sessionId);
			if (iter != m_Sessions.end())
				session = iter->second;
		}
		ROOM_COMMAND registerCommand{};
		registerCommand.eType = ROOM_COMMAND_TYPE::REGISTER_SESSION;
		registerCommand.iSessionId = sessionId;
		registerCommand.pSession = std::move(session);
		if (!targetRoom->Enqueue(std::move(registerCommand)))
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::ENTER_WORLD;
		command.EnterWorld = std::move(enterWorld);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_MOVE)
	{
		C2S_MOVE move{};
		if (!Read_Message(reader, move) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		targetRoom = Find_AssignedRoom(sessionId);
		command.eType = ROOM_COMMAND_TYPE::MOVE;
		command.Move = move;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_USE_SKILL)
	{
		C2S_USE_SKILL useSkill{};
		if (!Read_Message(reader, useSkill) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		targetRoom = Find_AssignedRoom(sessionId);
		command.eType = ROOM_COMMAND_TYPE::USE_SKILL;
		command.UseSkill = useSkill;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_RELEASE_SKILL)
	{
		C2S_RELEASE_SKILL releaseSkill{};
		if (!Read_Message(reader, releaseSkill) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		targetRoom = Find_AssignedRoom(sessionId);
		command.eType = ROOM_COMMAND_TYPE::RELEASE_SKILL;
		command.ReleaseSkill = releaseSkill;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_SPAWN_WORLD_ENTITY)
	{
		C2S_SPAWN_WORLD_ENTITY request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		targetRoom = Find_AssignedRoom(sessionId);
		command.eType = ROOM_COMMAND_TYPE::SPAWN_WORLD_ENTITY;
		command.SpawnWorldEntity = std::move(request);
	}
	else
	{
		Request_SessionClose(sessionId);
		return;
	}

	if (nullptr == targetRoom || !targetRoom->Enqueue(std::move(command)))
		Request_SessionClose(sessionId);
}

void LostArk::Server::CServerApp::On_SessionClosed(const SESSION_ID sessionId)
{
	if (CGameRoom* room = Find_AssignedRoom(sessionId))
	{
		ROOM_COMMAND command{};
		command.eType = ROOM_COMMAND_TYPE::LEAVE;
		command.iSessionId = sessionId;
		command.eLeaveReason =
			LostArk::Shared::PLAYER_DESPAWN_REASON::DISCONNECTED;
		room->Enqueue(std::move(command));
	}
	Unbind_SessionWorld(sessionId);
	std::scoped_lock lock{ m_ClosedSessionMutex };
	m_ClosedSessionIds.push_back(sessionId);
}

void LostArk::Server::CServerApp::Request_SessionClose(const SESSION_ID sessionId)
{
	std::shared_ptr<CClientSession> session;
	{
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
	std::deque<SESSION_ID> closedIds;
	{
		std::scoped_lock lock{ m_ClosedSessionMutex };
		closedIds.swap(m_ClosedSessionIds);
	}
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

LostArk::Server::CGameRoom* LostArk::Server::CServerApp::Find_Room(
	const LostArk::Shared::WORLD_ID worldId)
{
	const auto iter = m_GameRooms.find(worldId);
	return iter == m_GameRooms.end() ? nullptr : iter->second.get();
}

LostArk::Server::CGameRoom* LostArk::Server::CServerApp::Find_AssignedRoom(
	const SESSION_ID sessionId)
{
	LostArk::Shared::WORLD_ID worldId = LostArk::Shared::WORLD_ID::END;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		const auto iter = m_WorldBySessionId.find(sessionId);
		if (iter == m_WorldBySessionId.end())
			return nullptr;
		worldId = iter->second;
	}
	return Find_Room(worldId);
}

bool LostArk::Server::CServerApp::Bind_SessionWorld(
	const SESSION_ID sessionId,
	const LostArk::Shared::WORLD_ID worldId)
{
	if (!LostArk::Shared::Is_Known_World_Id(worldId) || nullptr == Find_Room(worldId))
		return false;
	std::scoped_lock lock{ m_SessionsMutex };
	if (!m_Sessions.contains(sessionId))
		return false;
	const auto [iter, inserted] = m_WorldBySessionId.emplace(sessionId, worldId);
	return inserted || iter->second == worldId;
}

void LostArk::Server::CServerApp::Handle_WorldTransfers(
	CGameRoom& sourceRoom)
{
	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	while (sourceRoom.Try_DequeueWorldTransfer(transfer))
	{
		if (!Transfer_SessionWorld(sourceRoom.Get_WorldId(), transfer))
			Request_SessionClose(transfer.iSessionId);
	}
}

bool LostArk::Server::CServerApp::Transfer_SessionWorld(
	const LostArk::Shared::WORLD_ID sourceWorldId,
	const SERVER_WORLD_TRANSFER_REQUEST& transfer)
{
	using namespace LostArk::Shared;
	CGameRoom* targetRoom = Find_Room(transfer.eTargetWorldId);
	if (nullptr == targetRoom || sourceWorldId == transfer.eTargetWorldId ||
		CHARACTER_CLASS_ID::END == transfer.eCharacterClass ||
		transfer.strNickName.empty())
	{
		return false;
	}

	std::shared_ptr<CClientSession> session;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		const auto sessionIter = m_Sessions.find(transfer.iSessionId);
		const auto worldIter = m_WorldBySessionId.find(transfer.iSessionId);
		if (sessionIter == m_Sessions.end() ||
			worldIter == m_WorldBySessionId.end() ||
			worldIter->second != sourceWorldId)
		{
			return false;
		}
		session = sessionIter->second;
		worldIter->second = transfer.eTargetWorldId;
	}

	ROOM_COMMAND registerCommand{};
	registerCommand.eType = ROOM_COMMAND_TYPE::REGISTER_SESSION;
	registerCommand.iSessionId = transfer.iSessionId;
	registerCommand.pSession = session;
	if (!targetRoom->Enqueue(std::move(registerCommand)))
		return false;

	C2S_ENTER_WORLD enterWorld{};
	enterWorld.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	enterWorld.eWorldId = transfer.eTargetWorldId;
	enterWorld.eCharacterClass = transfer.eCharacterClass;
	enterWorld.strNickName = transfer.strNickName;
	ROOM_COMMAND enterCommand{};
	enterCommand.eType = ROOM_COMMAND_TYPE::ENTER_WORLD;
	enterCommand.iSessionId = transfer.iSessionId;
	enterCommand.EnterWorld = std::move(enterWorld);
	return targetRoom->Enqueue(std::move(enterCommand));
}

void LostArk::Server::CServerApp::Unbind_SessionWorld(const SESSION_ID sessionId)
{
	std::scoped_lock lock{ m_SessionsMutex };
	m_WorldBySessionId.erase(sessionId);
}

void LostArk::Server::CServerApp::Shutdown()
{
	m_isRunning.store(false);
	m_TcpListener.Close();
	if (m_AcceptThread.joinable())
		m_AcceptThread.join();
	if (m_RoomThread.joinable())
		m_RoomThread.join();

	std::vector<std::shared_ptr<CClientSession>> sessions;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		for (auto& [sessionId, session] : m_Sessions)
		{
			(void)sessionId;
			sessions.push_back(std::move(session));
		}
		m_Sessions.clear();
		m_WorldBySessionId.clear();
	}
	for (const auto& session : sessions)
	{
		if (nullptr != session)
			session->Request_Close();
	}
	for (const auto& session : sessions)
	{
		if (nullptr != session)
			session->Stop();
	}
	m_WinSockContext.Shutdown();
}
