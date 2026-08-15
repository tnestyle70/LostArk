#include "ServerApp.h"

#include "ClientSession.h"

#include "Network/PacketMessages.h"
#include "Network/PacketReader.h"

#include <WinSock2.h>

#include <cstdio>
#include <io.h>

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
	const std::string_view bindAddress,
	const std::uint16_t port,
	const bool headless)
{
	using LostArk::Shared::WORLD_ID;

	std::map<WORLD_ID, std::shared_ptr<CGameRoom>> stagedSharedSimulations;
	const auto stageSharedSimulation =
		[&stagedSharedSimulations](const WORLD_ID worldId)
		{
			auto simulation = std::make_shared<CGameRoom>(worldId);
			if (nullptr == simulation || !simulation->Is_Ready())
			{
				std::cerr << "World simulation failed to initialize. World="
					<< static_cast<unsigned>(worldId) << ", Status="
					<< (nullptr == simulation ?
						"Simulation allocation failed" : simulation->Get_Status())
					<< '\n';
				return false;
			}
			return stagedSharedSimulations.emplace(
				worldId, std::move(simulation)).second;
		};

	if (!stageSharedSimulation(WORLD_ID::BERN) ||
		!stageSharedSimulation(WORLD_ID::VALTAN_ARENA) ||
		!stageSharedSimulation(WORLD_ID::TRAINING_GROUND))
	{
		return 1;
	}

	// Character Select uses the same Server gameplay runtime as every other
	// world, but each admitted session receives a private simulation instance.
	// Construct one temporary instance here to retain startup fail-fast validation.
	{
		const auto validationSimulation =
			std::make_shared<CGameRoom>(WORLD_ID::CHARACTER_SELECT_ARENA);
		if (nullptr == validationSimulation || !validationSimulation->Is_Ready())
		{
			std::cerr << "Character Select simulation failed to initialize. Status="
				<< (nullptr == validationSimulation ?
					"Simulation allocation failed" :
					validationSimulation->Get_Status())
				<< '\n';
			return 1;
		}
	}

	{
		std::scoped_lock lock{ m_SessionsMutex };
		m_SharedGameRooms = std::move(stagedSharedSimulations);
		m_CharacterSelectArenas.clear();
		m_GameplayBindingBySessionId.clear();
	}

	if (!m_WinSockContext.Initialize())
	{
		std::cerr << "Failed to initialize WinSock 2.2\n";
		return 1;
	}
	if (0u == port || !m_TcpListener.Open(bindAddress, port))
	{
		std::cerr << "Failed to open TCP listener. Address="
			<< bindAddress << ':' << port << ", Error="
			<< m_TcpListener.Get_LastErrorCode() << '\n';
		return 1;
	}

	m_isRunning.store(true);
	m_RoomThread = std::thread(&CServerApp::Room_Loop, this);
	m_AcceptThread = std::thread(&CServerApp::Accept_Loop, this);
	const bool useHeadlessMode = headless ||
		0 == ::_isatty(::_fileno(stdin));
	std::cout << "Listening on " << bindAddress << ':' << port
		<< " with shared BERN, VALTAN_ARENA, TRAINING_GROUND and "
		<< "session-private CHARACTER_SELECT_ARENA simulations.";
	if (0u == automaticShutdownMilliseconds && useHeadlessMode)
	{
		std::cout << " Headless mode; terminate the process to stop.\n";
		while (m_isRunning.load())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
	}
	else if (0u == automaticShutdownMilliseconds)
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
		Tick_GameplaySimulations(FIXED_DELTA_SECONDS);
		Reap_ClosedSessions();
		std::this_thread::sleep_until(nextTick);
		if (steady_clock::now() > nextTick + fixedStep)
			nextTick = steady_clock::now();
	}
	Tick_GameplaySimulations(FIXED_DELTA_SECONDS);
	Reap_ClosedSessions();
}

void LostArk::Server::CServerApp::On_SessionFrame(
	const SESSION_ID sessionId,
	const LostArk::Shared::PACKET_FRAME& frame)
{
	using namespace LostArk::Shared;

	CPacketReader reader{ frame.Payload };
	if (frame.ePacketType == PACKET_TYPE::C2S_ENTER_WORLD)
	{
		C2S_ENTER_WORLD enterWorld{};
		if (!Read_Message(reader, enterWorld) ||
			0u != reader.Get_RemainingSize() ||
			enterWorld.iProtocolVersion != NETWORK_PROTOCOL_VERSION ||
			!Is_Known_World_Id(enterWorld.eWorldId))
		{
			Request_SessionClose(sessionId);
			return;
		}

		const std::shared_ptr<CGameRoom> targetSimulation =
			Acquire_EntrySimulation(sessionId, enterWorld.eWorldId);
		if (nullptr == targetSimulation ||
			!Bind_SessionSimulation(
				sessionId, enterWorld.eWorldId, targetSimulation))
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
		registerCommand.pSession = session;

		ROOM_COMMAND enterCommand{};
		enterCommand.eType = ROOM_COMMAND_TYPE::ENTER_WORLD;
		enterCommand.iSessionId = sessionId;
		enterCommand.EnterWorld = std::move(enterWorld);

		if (nullptr == session ||
			!targetSimulation->Enqueue(std::move(registerCommand)) ||
			!targetSimulation->Enqueue(std::move(enterCommand)))
		{
			// REGISTER may already be queued. LEAVE is ordered after it and
			// removes the weak session before a private arena can retire.
			ROOM_COMMAND rollbackCommand{};
			rollbackCommand.eType = ROOM_COMMAND_TYPE::LEAVE;
			rollbackCommand.iSessionId = sessionId;
			rollbackCommand.eLeaveReason =
				PLAYER_DESPAWN_REASON::DISCONNECTED;
			(void)targetSimulation->Enqueue(std::move(rollbackCommand));
			Unbind_SessionSimulation(sessionId, targetSimulation);
			Request_SessionClose(sessionId);
		}
		return;
	}

	ROOM_COMMAND command{};
	command.iSessionId = sessionId;
	if (frame.ePacketType == PACKET_TYPE::C2S_MOVE)
	{
		C2S_MOVE move{};
		if (!Read_Message(reader, move) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
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
		command.eType = ROOM_COMMAND_TYPE::RELEASE_SKILL;
		command.ReleaseSkill = releaseSkill;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_UPDATE_SKILL_AIM)
	{
		C2S_UPDATE_SKILL_AIM updateSkillAim{};
		if (!Read_Message(reader, updateSkillAim) ||
			0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM;
		command.UpdateSkillAim = updateSkillAim;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_USE_ESTHER_SKILL)
	{
		C2S_USE_ESTHER_SKILL useEstherSkill{};
		if (!Read_Message(reader, useEstherSkill) ||
			0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::USE_ESTHER_SKILL;
		command.UseEstherSkill = useEstherSkill;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_REVIVE_PLAYER)
	{
		C2S_REVIVE_PLAYER revivePlayer{};
		if (!Read_Message(reader, revivePlayer) ||
			0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::REVIVE_PLAYER;
		command.RevivePlayer = revivePlayer;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_CHANGE_CHARACTER_CLASS)
	{
		C2S_CHANGE_CHARACTER_CLASS request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::CHANGE_CHARACTER_CLASS;
		command.ChangeCharacterClass = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_SPAWN_WORLD_ENTITY)
	{
		C2S_SPAWN_WORLD_ENTITY request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::SPAWN_WORLD_ENTITY;
		command.SpawnWorldEntity = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_VALTAN_AUDITION_REQUEST)
	{
		C2S_VALTAN_AUDITION_REQUEST request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::VALTAN_AUDITION;
		command.ValtanAudition = request;
	}
	else
	{
		Request_SessionClose(sessionId);
		return;
	}

	if (!Enqueue_AssignedCommand(sessionId, std::move(command)))
		Request_SessionClose(sessionId);
}

void LostArk::Server::CServerApp::On_SessionClosed(const SESSION_ID sessionId)
{
	{
		std::scoped_lock lock{ m_SessionsMutex };
		const auto bindingIter =
			m_GameplayBindingBySessionId.find(sessionId);
		if (bindingIter != m_GameplayBindingBySessionId.end())
		{
			if (nullptr != bindingIter->second.pSimulation)
			{
				ROOM_COMMAND command{};
				command.eType = ROOM_COMMAND_TYPE::LEAVE;
				command.iSessionId = sessionId;
				command.eLeaveReason =
					LostArk::Shared::PLAYER_DESPAWN_REASON::DISCONNECTED;
				(void)bindingIter->second.pSimulation->Enqueue(
					std::move(command));
			}
			m_GameplayBindingBySessionId.erase(bindingIter);
		}
	}

	// A private Character Select simulation owns the queued LEAVE until the
	// room thread consumes it, seals the empty queue, and retires the arena.
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

std::shared_ptr<LostArk::Server::CGameRoom>
LostArk::Server::CServerApp::Acquire_EntrySimulation(
	const SESSION_ID sessionId,
	const LostArk::Shared::WORLD_ID worldId)
{
	using LostArk::Shared::WORLD_ID;

	if (!LostArk::Shared::Is_Known_World_Id(worldId))
		return nullptr;
	if (WORLD_ID::CHARACTER_SELECT_ARENA != worldId)
		return Find_SharedSimulation(worldId);

	{
		std::scoped_lock lock{ m_SessionsMutex };
		if (!m_Sessions.contains(sessionId))
			return nullptr;

		const auto bindingIter =
			m_GameplayBindingBySessionId.find(sessionId);
		if (bindingIter != m_GameplayBindingBySessionId.end())
		{
			const SESSION_GAMEPLAY_BINDING& binding = bindingIter->second;
			return binding.eWorldId == worldId &&
				binding.iPrivateArenaOwnerSessionId == sessionId ?
					binding.pSimulation : nullptr;
		}
	}

	auto simulation = std::make_shared<CGameRoom>(worldId);
	if (nullptr == simulation || !simulation->Is_Ready())
	{
		std::cerr << "Private Character Select simulation failed. SessionId="
			<< sessionId << ", Status="
			<< (nullptr == simulation ?
				"Simulation allocation failed" : simulation->Get_Status())
			<< '\n';
		return nullptr;
	}
	return simulation;
}

std::shared_ptr<LostArk::Server::CGameRoom>
LostArk::Server::CServerApp::Find_SharedSimulation(
	const LostArk::Shared::WORLD_ID worldId)
{
	std::scoped_lock lock{ m_SessionsMutex };
	const auto iter = m_SharedGameRooms.find(worldId);
	return iter == m_SharedGameRooms.end() ? nullptr : iter->second;
}

bool LostArk::Server::CServerApp::Bind_SessionSimulation(
	const SESSION_ID sessionId,
	const LostArk::Shared::WORLD_ID worldId,
	const std::shared_ptr<CGameRoom>& simulation)
{
	using LostArk::Shared::WORLD_ID;

	if (nullptr == simulation ||
		!LostArk::Shared::Is_Known_World_Id(worldId) ||
		simulation->Get_WorldId() != worldId)
	{
		return false;
	}

	std::scoped_lock lock{ m_SessionsMutex };
	if (!m_Sessions.contains(sessionId))
		return false;

	const SESSION_ID privateOwnerSessionId =
		WORLD_ID::CHARACTER_SELECT_ARENA == worldId ?
			sessionId : INVALID_SESSION_ID;
	const auto existingBinding =
		m_GameplayBindingBySessionId.find(sessionId);
	if (existingBinding != m_GameplayBindingBySessionId.end())
	{
		return existingBinding->second.eWorldId == worldId &&
			existingBinding->second.iPrivateArenaOwnerSessionId ==
				privateOwnerSessionId &&
			existingBinding->second.pSimulation == simulation;
	}

	bool insertedPrivateArena = false;
	if (WORLD_ID::CHARACTER_SELECT_ARENA == worldId)
	{
		const auto [arenaIter, inserted] =
			m_CharacterSelectArenas.emplace(sessionId, simulation);
		if (!inserted && arenaIter->second != simulation)
			return false;
		insertedPrivateArena = inserted;
	}
	else
	{
		const auto sharedIter = m_SharedGameRooms.find(worldId);
		if (sharedIter == m_SharedGameRooms.end() ||
			sharedIter->second != simulation)
		{
			return false;
		}
	}

	SESSION_GAMEPLAY_BINDING binding{};
	binding.eWorldId = worldId;
	binding.iPrivateArenaOwnerSessionId = privateOwnerSessionId;
	binding.pSimulation = simulation;
	const auto [bindingIter, insertedBinding] =
		m_GameplayBindingBySessionId.emplace(sessionId, std::move(binding));
	(void)bindingIter;
	if (!insertedBinding)
	{
		if (insertedPrivateArena)
			m_CharacterSelectArenas.erase(sessionId);
		return false;
	}
	return true;
}

bool LostArk::Server::CServerApp::Enqueue_AssignedCommand(
	const SESSION_ID sessionId,
	ROOM_COMMAND command)
{
	if (command.iSessionId != sessionId)
		return false;

	// Keep lookup plus Enqueue atomic with Transfer_SessionWorld. Both paths
	// use m_SessionsMutex -> CGameRoom::m_CommandMutex ordering.
	std::scoped_lock lock{ m_SessionsMutex };
	const auto iter = m_GameplayBindingBySessionId.find(sessionId);
	return iter != m_GameplayBindingBySessionId.end() &&
		nullptr != iter->second.pSimulation &&
		iter->second.pSimulation->Enqueue(std::move(command));
}

void LostArk::Server::CServerApp::Tick_GameplaySimulations(
	const float fixedDeltaSeconds)
{
	std::vector<std::shared_ptr<CGameRoom>> simulations;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		simulations.reserve(
			m_SharedGameRooms.size() + m_CharacterSelectArenas.size());
		for (const auto& [worldId, simulation] : m_SharedGameRooms)
		{
			(void)worldId;
			if (nullptr != simulation)
				simulations.push_back(simulation);
		}
		for (const auto& [sessionId, simulation] : m_CharacterSelectArenas)
		{
			(void)sessionId;
			if (nullptr != simulation)
				simulations.push_back(simulation);
		}
	}

	// The room thread is the only writer of gameplay state. The mutex is
	// released before Tick so receive and session threads never wait on a tick.
	for (const std::shared_ptr<CGameRoom>& simulation : simulations)
	{
		simulation->Tick(fixedDeltaSeconds);
		Handle_WorldTransfers(simulation);
	}
	Retire_QuiescentCharacterSelectArenas();
}

void LostArk::Server::CServerApp::Retire_QuiescentCharacterSelectArenas()
{
	// Lock order is ServerApp sessions mutex -> room command mutex. Try_Seal
	// serializes the empty decision with each receive-thread Enqueue.
	std::scoped_lock lock{ m_SessionsMutex };
	for (auto iter = m_CharacterSelectArenas.begin();
		iter != m_CharacterSelectArenas.end();)
	{
		const SESSION_ID ownerSessionId = iter->first;
		const std::shared_ptr<CGameRoom>& simulation = iter->second;
		const auto bindingIter =
			m_GameplayBindingBySessionId.find(ownerSessionId);
		const bool isStillBound =
			bindingIter != m_GameplayBindingBySessionId.end() &&
			bindingIter->second.pSimulation == simulation;

		if (isStillBound || nullptr == simulation ||
			!simulation->Try_SealPrivateArenaForRetirement())
		{
			++iter;
			continue;
		}
		iter = m_CharacterSelectArenas.erase(iter);
	}
}

void LostArk::Server::CServerApp::Handle_WorldTransfers(
	const std::shared_ptr<CGameRoom>& sourceSimulation)
{
	if (nullptr == sourceSimulation)
		return;

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	while (sourceSimulation->Try_DequeueWorldTransfer(transfer))
	{
		if (!Transfer_SessionWorld(sourceSimulation, transfer))
			Request_SessionClose(transfer.iSessionId);
	}
}

bool LostArk::Server::CServerApp::Transfer_SessionWorld(
	const std::shared_ptr<CGameRoom>& sourceSimulation,
	const SERVER_WORLD_TRANSFER_REQUEST& transfer)
{
	using namespace LostArk::Shared;

	if (nullptr == sourceSimulation ||
		CHARACTER_CLASS_ID::END == transfer.eCharacterClass ||
		transfer.strNickName.empty())
	{
		return false;
	}

	const WORLD_ID sourceWorldId = sourceSimulation->Get_WorldId();
	const std::shared_ptr<CGameRoom> targetSimulation =
		Find_SharedSimulation(transfer.eTargetWorldId);
	if (nullptr == targetSimulation ||
		targetSimulation == sourceSimulation ||
		sourceWorldId == transfer.eTargetWorldId)
	{
		return false;
	}

	// Enqueue_AssignedCommand and On_SessionClosed use the same mutex. Route
	// lookup, queue ordering, and the binding commit therefore have one order.
	std::scoped_lock lock{ m_SessionsMutex };
	const auto sessionIter = m_Sessions.find(transfer.iSessionId);
	const auto bindingIter =
		m_GameplayBindingBySessionId.find(transfer.iSessionId);
	if (sessionIter == m_Sessions.end() ||
		bindingIter == m_GameplayBindingBySessionId.end() ||
		bindingIter->second.eWorldId != sourceWorldId ||
		bindingIter->second.pSimulation != sourceSimulation ||
		(WORLD_ID::CHARACTER_SELECT_ARENA == sourceWorldId &&
			bindingIter->second.iPrivateArenaOwnerSessionId !=
				transfer.iSessionId))
	{
		return false;
	}

	ROOM_COMMAND registerCommand{};
	registerCommand.eType = ROOM_COMMAND_TYPE::REGISTER_SESSION;
	registerCommand.iSessionId = transfer.iSessionId;
	registerCommand.pSession = sessionIter->second;
	if (!targetSimulation->Enqueue(std::move(registerCommand)))
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
	if (!targetSimulation->Enqueue(std::move(enterCommand)))
	{
		ROOM_COMMAND targetRollback{};
		targetRollback.eType = ROOM_COMMAND_TYPE::LEAVE;
		targetRollback.iSessionId = transfer.iSessionId;
		targetRollback.eLeaveReason =
			PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
		(void)targetSimulation->Enqueue(std::move(targetRollback));
		return false;
	}

	if (!sourceSimulation->Commit_WorldTransferDeparture(
		transfer.iSessionId))
	{
		ROOM_COMMAND targetRollback{};
		targetRollback.eType = ROOM_COMMAND_TYPE::LEAVE;
		targetRollback.iSessionId = transfer.iSessionId;
		targetRollback.eLeaveReason =
			PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
		(void)targetSimulation->Enqueue(std::move(targetRollback));
		return false;
	}

	bindingIter->second.eWorldId = transfer.eTargetWorldId;
	bindingIter->second.iPrivateArenaOwnerSessionId = INVALID_SESSION_ID;
	bindingIter->second.pSimulation = targetSimulation;
	return true;
}

void LostArk::Server::CServerApp::Unbind_SessionSimulation(
	const SESSION_ID sessionId,
	const std::shared_ptr<CGameRoom>& expectedSimulation)
{
	std::scoped_lock lock{ m_SessionsMutex };
	const auto iter = m_GameplayBindingBySessionId.find(sessionId);
	if (iter == m_GameplayBindingBySessionId.end())
		return;
	if (nullptr != expectedSimulation &&
		iter->second.pSimulation != expectedSimulation)
	{
		return;
	}
	m_GameplayBindingBySessionId.erase(iter);
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
		m_GameplayBindingBySessionId.clear();
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

	// Receive callbacks and the room thread are stopped before simulations are
	// destroyed, so no producer can enqueue into an unowned room.
	{
		std::scoped_lock lock{ m_SessionsMutex };
		m_CharacterSelectArenas.clear();
		m_SharedGameRooms.clear();
	}
	{
		std::scoped_lock lock{ m_ClosedSessionMutex };
		m_ClosedSessionIds.clear();
	}
	m_WinSockContext.Shutdown();
}
