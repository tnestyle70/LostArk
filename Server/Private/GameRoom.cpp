#include "GameRoom.h"

#include "ClientSession.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
	using namespace LostArk::Shared;
	using namespace LostArk::Server;

	constexpr float MAX_ABS_MOVE_GOAL = 10000.f;
	constexpr float MOVE_STOP_DISTANCE = 0.05f;
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
#ifdef _DEBUG
	constexpr std::uint32_t CHARACTER_SELECT_AUDITION_COOLDOWN_TICKS = 90u;

	constexpr std::uint32_t Add_ServerTicksSkippingReservedZero(
		const std::uint32_t startTick,
		const std::uint32_t elapsedTicks)
	{
		constexpr std::uint64_t SERVER_TICK_CARDINALITY =
			(static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)()));
		return static_cast<std::uint32_t>(
			((static_cast<std::uint64_t>(startTick - 1u) + elapsedTicks) %
				SERVER_TICK_CARDINALITY) + 1u);
	}

	static_assert(91u == Add_ServerTicksSkippingReservedZero(1u, 90u));
	static_assert(90u == Add_ServerTicksSkippingReservedZero(
		(std::numeric_limits<std::uint32_t>::max)(), 90u));
	static_assert(1u == Add_ServerTicksSkippingReservedZero(
		(std::numeric_limits<std::uint32_t>::max)() - 89u, 90u));
#endif

	bool Is_Valid_EnterWorld(const C2S_ENTER_WORLD& message)
	{
		return message.iProtocolVersion == NETWORK_PROTOCOL_VERSION &&
			Is_Known_World_Id(message.eWorldId) &&
			Is_Supported_Playable_Character_Class(message.eCharacterClass) &&
			!message.strNickName.empty() &&
			message.strNickName.size() <= MAX_NICKNAME_BYTES;
	}

	bool Is_NewerSequence(
		const std::uint32_t candidate,
		const std::uint32_t previous)
	{
		return 0u != candidate &&
			static_cast<std::int32_t>(candidate - previous) > 0;
	}

	WORLD_ENTITY_KIND To_NetworkKind(const WORLD_BOOTSTRAP_KIND kind)
	{
		switch (kind)
		{
		case WORLD_BOOTSTRAP_KIND::NPC: return WORLD_ENTITY_KIND::NPC;
		case WORLD_BOOTSTRAP_KIND::BOSS: return WORLD_ENTITY_KIND::BOSS;
		case WORLD_BOOTSTRAP_KIND::MONSTER: return WORLD_ENTITY_KIND::MONSTER;
		default: return WORLD_ENTITY_KIND::END;
		}
	}

	WORLD_ENTITY_ACTION To_NetworkAction(const SERVER_ENTITY_ACTION action)
	{
		switch (action)
		{
		case SERVER_ENTITY_ACTION::IDLE: return WORLD_ENTITY_ACTION::IDLE;
		case SERVER_ENTITY_ACTION::CHASE: return WORLD_ENTITY_ACTION::CHASE;
		case SERVER_ENTITY_ACTION::PATTERN_WINDUP: return WORLD_ENTITY_ACTION::PATTERN_WINDUP;
		case SERVER_ENTITY_ACTION::PATTERN_ACTIVE: return WORLD_ENTITY_ACTION::PATTERN_ACTIVE;
		case SERVER_ENTITY_ACTION::PATTERN_RECOVERY: return WORLD_ENTITY_ACTION::PATTERN_RECOVERY;
		case SERVER_ENTITY_ACTION::DEAD: return WORLD_ENTITY_ACTION::DEAD;
		default: return WORLD_ENTITY_ACTION::END;
		}
	}
}

LostArk::Server::CGameRoom::CGameRoom(
	const LostArk::Shared::WORLD_ID worldId)
	: m_eWorldId(worldId)
{
	if (!LostArk::Shared::Is_Known_World_Id(worldId))
	{
		m_strStatus = "Unknown room world ID";
		return;
	}
	if (!m_WorldBootstrap.Load(worldId))
	{
		m_strStatus = m_WorldBootstrap.Get_Status();
		return;
	}
	if (!m_GameplayCatalog.Load())
	{
		m_strStatus = m_GameplayCatalog.Get_Status();
		return;
	}
	if (!m_SpawnGroupBootstrap.Load(worldId))
	{
		m_strStatus = m_SpawnGroupBootstrap.Get_Status();
		return;
	}
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, m_strStatus))
		return;
	if ((LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::TRAINING_GROUND == worldId ||
		LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == worldId) &&
		!m_ServerNavigation.Load(m_WorldBootstrap.Get_AreaId()))
	{
		m_strStatus = m_ServerNavigation.Get_Status();
		return;
	}
	if (!m_ServerTriggerSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), m_strStatus))
	{
		return;
	}
	if (!m_ServerCollisionSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), m_strStatus))
	{
		return;
	}
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (placement.isEnabled &&
			WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
			!m_ServerCollisionSystem.Is_PlayerSpawnClear(placement))
		{
			m_strStatus = "Player spawn overlaps a collision box: " +
				placement.strPlacementId;
			return;
		}
	}
	if (!Initialize_WorldEntities())
		return;
	if (nullptr == Find_AvailablePlayerSpawn())
	{
		m_strStatus = "World bootstrap has no enabled player spawn";
		return;
	}

	m_isReady = true;
	m_strStatus = m_WorldBootstrap.Get_Status();
}

bool LostArk::Server::CGameRoom::Enqueue(ROOM_COMMAND command)
{
	if (!m_isReady || command.iSessionId == INVALID_SESSION_ID)
		return false;
	if (command.eType == ROOM_COMMAND_TYPE::REGISTER_SESSION &&
		(nullptr == command.pSession ||
			command.pSession->Get_SessionId() != command.iSessionId))
	{
		return false;
	}

	std::scoped_lock lock{ m_CommandMutex };
	m_InboundCommands.push_back(std::move(command));
	return true;
}

bool LostArk::Server::CGameRoom::Try_DequeueWorldTransfer(
	SERVER_WORLD_TRANSFER_REQUEST& outTransfer)
{
	if (m_PendingWorldTransfers.empty())
		return false;
	outTransfer = std::move(m_PendingWorldTransfers.front());
	m_PendingWorldTransfers.pop_front();
	return true;
}

void LostArk::Server::CGameRoom::Tick(const float fixedDeltaSeconds)
{
	if (!m_isReady)
		return;

	std::deque<ROOM_COMMAND> commands;
	{
		std::scoped_lock lock{ m_CommandMutex };
		commands.swap(m_InboundCommands);
	}

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
		case ROOM_COMMAND_TYPE::MOVE:
			Handle_Move(command.iSessionId, command.Move);
			break;
		case ROOM_COMMAND_TYPE::USE_SKILL:
			Handle_UseSkill(command.iSessionId, command.UseSkill);
			break;
		case ROOM_COMMAND_TYPE::RELEASE_SKILL:
			Handle_ReleaseSkill(command.iSessionId, command.ReleaseSkill);
			break;
		case ROOM_COMMAND_TYPE::REVIVE_PLAYER:
			Handle_RevivePlayer(command.iSessionId, command.RevivePlayer);
			break;
		case ROOM_COMMAND_TYPE::CHANGE_CHARACTER_CLASS:
			Handle_ChangeCharacterClass(
				command.iSessionId, command.ChangeCharacterClass);
			break;
		case ROOM_COMMAND_TYPE::SPAWN_WORLD_ENTITY:
			Handle_SpawnWorldEntity(
				command.iSessionId,
				command.SpawnWorldEntity);
			break;
		case ROOM_COMMAND_TYPE::LEAVE:
			Leave(command.iSessionId, command.eLeaveReason);
			break;
		}
	}

	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
		return;

	m_TickDamageEvents.clear();
	Update_Players(fixedDeltaSeconds);
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
	m_ServerTriggerSystem.Evaluate_Entries(
		m_Players,
		updateTick,
		transfers,
		[this](const WORLD_TRIGGER_ACTION_KIND kind,
			const std::string& targetId)
		{
			if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP == kind)
				return m_SpawnGroupRuntime.Activate(targetId);
			if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER == kind)
				return Activate_Encounter(targetId);
			return false;
		});
	for (SERVER_WORLD_TRANSFER_REQUEST& transfer : transfers)
	{
		if (!m_PlayerIdBySessionId.contains(transfer.iSessionId))
			continue;
		Leave(
			transfer.iSessionId,
			LostArk::Shared::PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
		m_PendingWorldTransfers.push_back(std::move(transfer));
	}
	m_SpawnGroupRuntime.Update(
		fixedDeltaSeconds,
		m_SpawnGroupBootstrap,
		[this](const std::string& spawnGroupId)
		{
			return Count_SpawnGroupEntities(spawnGroupId);
		},
		[this](const std::string& spawnGroupId,
			const SPAWN_GROUP_ENTRY& entry,
			const SPAWN_GROUP_ANCHOR& anchor,
			const MONSTER_RUNTIME_PROFILE& profile,
			const std::uint32_t ordinal)
		{
			return Spawn_Monster(
				spawnGroupId, entry, anchor, profile, ordinal);
		});
	Update_WorldEntities(fixedDeltaSeconds);
	++m_iServerTick;
	if (0u == m_iServerTick)
		m_iServerTick = 1u;
	if (!m_Players.empty())
		Broadcast_WorldSnapshot();
}

void LostArk::Server::CGameRoom::Handle_Register(
	const std::shared_ptr<CClientSession>& session)
{
	if (nullptr == session || session->Get_SessionId() == INVALID_SESSION_ID)
		return;
	m_Sessions.insert_or_assign(session->Get_SessionId(), session);
}

bool LostArk::Server::CGameRoom::Join(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_ENTER_WORLD& enterWorld)
{
	using namespace LostArk::Shared;

	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	const WORLD_BOOTSTRAP_PLACEMENT* spawn = Find_AvailablePlayerSpawn();
	if (nullptr == session || !Is_Valid_EnterWorld(enterWorld) ||
		enterWorld.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.contains(sessionId) ||
		m_Players.size() >= MAX_WORLD_SNAPSHOT_PLAYERS ||
		nullptr == spawn ||
		m_iNextPlayerId == INVALID_PLAYER_ID ||
		m_iNextNetEntityId == INVALID_NET_ENTITY_ID)
	{
		if (nullptr != session)
			session->Request_Close();
		return false;
	}

	SERVER_PLAYER player{};
	player.iSessionId = sessionId;
	player.iPlayerId = m_iNextPlayerId;
	player.iNetEntityId = m_iNextNetEntityId;
	player.eCharacterClass = enterWorld.eCharacterClass;
	player.strNickName = enterWorld.strNickName;
	player.strSpawnPlacementId = spawn->strPlacementId;
	player.fPositionX = spawn->fPositionX;
	player.fPositionY = spawn->fPositionY;
	player.fPositionZ = spawn->fPositionZ;
	player.fYawDegrees = spawn->fYawDegrees;
	const PLAYER_RUNTIME_PROFILE* playerProfile =
		m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == playerProfile)
	{
		session->Request_Close();
		return false;
	}
	player.eStance = playerProfile->eDefaultStance;
	player.iCurrentHp = playerProfile->iMaximumHp;
	player.iMaximumHp = playerProfile->iMaximumHp;
	player.iCurrentResource = playerProfile->iMaximumResource;
	player.iMaximumResource = playerProfile->iMaximumResource;
	player.fMoveSpeed = playerProfile->fMoveSpeed;
	player.isCombatReady = WORLD_ID::VALTAN_ARENA != m_eWorldId;
	if (m_ServerNavigation.Is_Loaded())
	{
		SERVER_NAV_POINT projected{};
		if (!m_ServerNavigation.Project_Point(
			player.fPositionX, player.fPositionZ, projected))
		{
			session->Request_Close();
			return false;
		}
		player.fPositionX = projected.x;
		player.fPositionY = projected.y;
		player.fPositionZ = projected.z;
	}

	++m_iNextPlayerId;
	++m_iNextNetEntityId;
	m_Players.emplace(player.iPlayerId, player);
	m_PlayerIdBySessionId.emplace(sessionId, player.iPlayerId);
	m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
	session->Bind_PlayerId(player.iPlayerId);

	if (!Send_Accepted(session, player))
	{
		Rollback_Join(sessionId);
		session->Request_Close();
		return false;
	}
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (!Send_WorldEntitySpawned(session, entity))
		{
			Rollback_Join(sessionId);
			session->Request_Close();
			return false;
		}
	}
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
	if (!Send_Spawned(session, player))
	{
		Rollback_Join(sessionId);
		session->Request_Close();
		return false;
	}
	Broadcast_Spawned(player, sessionId);

	std::cout << "Player joined. World=" << static_cast<unsigned>(m_eWorldId)
		<< ", SessionId=" << sessionId
		<< ", PlayerId=" << player.iPlayerId
		<< ", Spawn=" << player.strSpawnPlacementId
		<< ", RoomPlayers=" << m_Players.size() << '\n';
	return true;
}

void LostArk::Server::CGameRoom::Leave(
	const SESSION_ID sessionId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	using namespace LostArk::Shared;

	const auto sessionPlayerIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionPlayerIter == m_PlayerIdBySessionId.end())
	{
		m_Sessions.erase(sessionId);
		return;
	}
	const PLAYER_ID playerId = sessionPlayerIter->second;
	const auto playerIter = m_Players.find(playerId);
	if (playerIter == m_Players.end())
	{
		m_PlayerIdBySessionId.erase(sessionPlayerIter);
		m_Sessions.erase(sessionId);
		return;
	}

	const NET_ENTITY_ID netEntityId = playerIter->second.iNetEntityId;
	m_ServerTriggerSystem.Remove_Player(playerId);
	if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
		session->Bind_PlayerId(INVALID_PLAYER_ID);
	m_PlayerIdByEntityId.erase(netEntityId);
	m_PlayerIdBySessionId.erase(sessionPlayerIter);
	m_Players.erase(playerIter);
	m_Sessions.erase(sessionId);
	Broadcast_Despawned(netEntityId, reason);

	std::cout << "Player left. World=" << static_cast<unsigned>(m_eWorldId)
		<< ", SessionId=" << sessionId
		<< ", RoomPlayers=" << m_Players.size() << '\n';

	if (!Reset_CharacterSelectArenaWhenEmpty())
	{
		std::cerr << "Character Select arena reset failed: "
			<< m_strStatus << '\n';
	}
}

void LostArk::Server::CGameRoom::Handle_Move(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_MOVE& move)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	SERVER_PLAYER& player = playerIter->second;
	if (!Is_NewerSequence(move.iClientSequence, player.iLastMoveSequence) ||
		!std::isfinite(move.fGoalX) || !std::isfinite(move.fGoalZ) ||
		std::abs(move.fGoalX) > MAX_ABS_MOVE_GOAL ||
		std::abs(move.fGoalZ) > MAX_ABS_MOVE_GOAL)
	{
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}

	player.iLastMoveSequence = move.iClientSequence;
	if (LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction ||
		0u == player.iCurrentHp)
	{
		return;
	}
	player.MovePath.clear();
	player.iMovePathIndex = 0;
	if (m_ServerNavigation.Is_Loaded())
	{
		if (!m_ServerNavigation.Find_Path(
			player.fPositionX,
			player.fPositionZ,
			move.fGoalX,
			move.fGoalZ,
			player.MovePath))
		{
			player.hasMoveGoal = false;
			return;
		}
		const SERVER_NAV_POINT& goal = player.MovePath.back();
		player.fMoveGoalX = goal.x;
		player.fMoveGoalZ = goal.z;
	}
	else
	{
		player.fMoveGoalX = move.fGoalX;
		player.fMoveGoalZ = move.fGoalZ;
	}
	player.hasMoveGoal = true;
	player.isCombatReady = true;
}

void LostArk::Server::CGameRoom::Handle_UseSkill(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_USE_SKILL& useSkill)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	const std::uint32_t actionStartTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
#ifdef _DEBUG
	/* Character Select Server Arena is the presentation audition room.  Keep
	its retries Server-authoritative with a fixed three-second audition cooldown
	and full resources; action-running, sequence, class, aim and snapshot gates
	remain in CPlayerSkillSystem::Try_Start.  Release rooms retain authored
	balance. */
	if (LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId)
	{
		playerIter->second.iCurrentResource =
			playerIter->second.iMaximumResource;
		playerIter->second.iResourceAccumulator = 0u;
	}
#endif
	// A valid but currently unavailable skill is rejected as gameplay state;
	// malformed payloads are already closed at the ServerApp packet boundary.
	if (m_PlayerSkillSystem.Try_Start(
		playerIter->second,
		useSkill,
		m_GameplayCatalog,
		actionStartTick))
	{
#ifdef _DEBUG
		if (LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId)
		{
			playerIter->second.CooldownEndTickBySkillId.insert_or_assign(
				useSkill.iSkillId,
				Add_ServerTicksSkippingReservedZero(
					actionStartTick,
					CHARACTER_SELECT_AUDITION_COOLDOWN_TICKS));
		}
#endif
		playerIter->second.isCombatReady = true;
	}
}

void LostArk::Server::CGameRoom::Handle_RevivePlayer(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer)
{
	using namespace LostArk::Shared;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId ||
		sessionIter == m_PlayerIdBySessionId.end())
	{
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	SERVER_PLAYER& player = playerIter->second;
	if (!Is_NewerSequence(
		revivePlayer.iClientSequence, player.iLastReviveSequence))
	{
		return;
	}
	player.iLastReviveSequence = revivePlayer.iClientSequence;
	if (0u != player.iCurrentHp || PLAYER_ACTION_STATE::DEAD != player.eAction)
		return;

	const PLAYER_RUNTIME_PROFILE* profile =
		m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile)
		return;
	player.iCurrentHp = player.iMaximumHp;
	player.iCurrentResource = player.iMaximumResource;
	player.iResourceAccumulator = 0u;
	player.eAction = PLAYER_ACTION_STATE::NONE;
	player.eStance = profile->eDefaultStance;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.iActionStartTick = 0u;
	player.fActionElapsedSeconds = 0.f;
	player.fSkillAimDirectionX = 0.f;
	player.fSkillAimDirectionZ = 1.f;
	player.hasAppliedSkillDamage = false;
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.CooldownEndTickBySkillId.clear();
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.TriggerMove = {};
	player.isCombatReady = false;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
}

void LostArk::Server::CGameRoom::Handle_ReleaseSkill(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RELEASE_SKILL& releaseSkill)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	m_PlayerSkillSystem.Release(
		playerIter->second,
		releaseSkill,
		m_GameplayCatalog);
}

LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT
LostArk::Server::CGameRoom::Apply_CharacterClassChange(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId)
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_WRONG_WORLD;
	if (!Is_NewerSequence(
		request.iClientSequence, player.iLastClassChangeSequence))
	{
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STALE_SEQUENCE;
	}
	if (!Is_Supported_Playable_Character_Class(request.eCharacterClass) ||
		nullptr == m_GameplayCatalog.Find_Player(request.eCharacterClass))
	{
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_UNSUPPORTED_CLASS;
	}
	if (request.eCharacterClass == player.eCharacterClass)
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_SAME_CLASS;

	const bool isDead = 0u == player.iCurrentHp &&
		PLAYER_ACTION_STATE::DEAD == player.eAction;
	if ((0u == player.iCurrentHp) !=
		(PLAYER_ACTION_STATE::DEAD == player.eAction))
	{
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STATE;
	}

	const PLAYER_RUNTIME_PROFILE* profile =
		m_GameplayCatalog.Find_Player(request.eCharacterClass);
	SERVER_PLAYER staged = player;
	if (isDead)
	{
		const WORLD_BOOTSTRAP_PLACEMENT* spawn =
			Find_Placement(player.strSpawnPlacementId);
		if (nullptr == spawn || !spawn->isEnabled ||
			WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != spawn->eKind)
		{
			return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STATE;
		}
		staged.fPositionX = spawn->fPositionX;
		staged.fPositionY = spawn->fPositionY;
		staged.fPositionZ = spawn->fPositionZ;
		staged.fYawDegrees = spawn->fYawDegrees;
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT projected{};
			if (!m_ServerNavigation.Project_Point(
				staged.fPositionX, staged.fPositionZ, projected))
			{
				return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STATE;
			}
			staged.fPositionX = projected.x;
			staged.fPositionY = projected.y;
			staged.fPositionZ = projected.z;
		}
	}

	staged.eCharacterClass = request.eCharacterClass;
	staged.iLastClassChangeSequence = request.iClientSequence;
	staged.fMoveGoalX = 0.f;
	staged.fMoveGoalZ = 0.f;
	staged.fMoveSpeed = profile->fMoveSpeed;
	staged.hasMoveGoal = false;
	staged.MovePath.clear();
	staged.iMovePathIndex = 0u;
	staged.iCurrentHp = profile->iMaximumHp;
	staged.iMaximumHp = profile->iMaximumHp;
	staged.iCurrentResource = profile->iMaximumResource;
	staged.iMaximumResource = profile->iMaximumResource;
	staged.iResourceAccumulator = 0u;
	staged.eAction = PLAYER_ACTION_STATE::NONE;
	staged.eStance = profile->eDefaultStance;
	staged.iCurrentSkillId = INVALID_SKILL_ID;
	staged.iActionStartTick = 0u;
	staged.TriggerMove = {};
	staged.fActionElapsedSeconds = 0.f;
	staged.fSkillAimDirectionX = 0.f;
	staged.fSkillAimDirectionZ = 1.f;
	staged.hasAppliedSkillDamage = false;
	staged.iComboStage = 0u;
	staged.hasBufferedComboInput = false;
	staged.hasReleasedHold = false;
	staged.CooldownEndTickBySkillId.clear();
	staged.isCombatReady = true;

	player = std::move(staged);
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
	return CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED;
}

void LostArk::Server::CGameRoom::Handle_ChangeCharacterClass(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (nullptr == session || sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	SERVER_PLAYER& player = playerIter->second;
	const CHARACTER_CLASS_CHANGE_RESULT result =
		Apply_CharacterClassChange(player, request);
	if (!Send_CharacterClassChangeResult(
		session, request, result, player.eCharacterClass))
	{
		session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_SpawnWorldEntity(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_SPAWN_WORLD_ENTITY& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId ||
		!m_PlayerIdBySessionId.contains(sessionId) || nullptr == session)
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	const WORLD_BOOTSTRAP_PLACEMENT* placement =
		Find_Placement(request.strPlacementId);
	if (nullptr == placement)
	{
		const auto group = std::find_if(
			m_SpawnGroupBootstrap.Get_Groups().begin(),
			m_SpawnGroupBootstrap.Get_Groups().end(),
			[&request](const SPAWN_GROUP_DEFINITION& definition)
			{
				return definition.strSpawnGroupId == request.strPlacementId;
			});
		if (m_SpawnGroupBootstrap.Get_Groups().end() == group)
		{
			Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::REJECTED,
				INVALID_NET_ENTITY_ID);
			return;
		}
		if (!m_SpawnGroupRuntime.Is_ActiveOrCompleted(request.strPlacementId) &&
			!m_SpawnGroupRuntime.Activate_Immediate(
				request.strPlacementId,
				m_SpawnGroupBootstrap,
				[this](const std::string& spawnGroupId,
					const SPAWN_GROUP_ENTRY& entry,
					const SPAWN_GROUP_ANCHOR& anchor,
					const MONSTER_RUNTIME_PROFILE& profile,
					const std::uint32_t ordinal)
				{
					return Spawn_Monster(
						spawnGroupId, entry, anchor, profile, ordinal);
				}))
		{
			Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::REJECTED,
				INVALID_NET_ENTITY_ID);
			return;
		}

		if (!Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::ACTIVATED,
			INVALID_NET_ENTITY_ID))
		{
			session->Request_Close();
		}
		return;
	}

	if (placement->isEnabled ||
		WORLD_BOOTSTRAP_KIND::BOSS != placement->eKind ||
		placement->strArchetypeId != "BOSS_VALTAN")
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}
	const auto existing = std::find_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[&request](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.strPlacementId == request.strPlacementId;
		});
	if (m_WorldEntities.end() != existing)
	{
		if (!Send_WorldEntitySpawned(session, *existing) ||
			!Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS,
				existing->iNetEntityId))
		{
			session->Request_Close();
		}
		return;
	}
	if (m_iNextNetEntityId == INVALID_NET_ENTITY_ID)
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	SERVER_WORLD_ENTITY staged{};
	if (!Build_WorldEntity(*placement, m_iNextNetEntityId, staged))
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	if (!Send_WorldEntitySpawnResult(
		session,
		request.strPlacementId,
		WORLD_ENTITY_SPAWN_RESULT::SPAWNED,
		m_WorldEntities.back().iNetEntityId))
	{
		session->Request_Close();
	}
}

bool LostArk::Server::CGameRoom::Send_Accepted(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	S2C_ENTER_ACCEPTED message{};
	message.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	message.eWorldId = m_eWorldId;
	message.iPlayerId = player.iPlayerId;
	message.iNetEntityId = player.iNetEntityId;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_ENTER_ACCEPTED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_Spawned(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_PLAYER& player)
{
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
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_PLAYER_SPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_WorldEntitySpawned(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_WORLD_ENTITY& entity)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_SPAWNED message{};
	message.iNetEntityId = entity.iNetEntityId;
	message.eKind = To_NetworkKind(entity.eKind);
	message.strArchetypeId = entity.strArchetypeId;
	message.strEncounterId = entity.strEncounterId;
	message.fPositionX = entity.fPositionX;
	message.fPositionY = entity.fPositionY;
	message.fPositionZ = entity.fPositionZ;
	message.fYawDegrees = entity.fYawDegrees;
	message.fCollisionRadius = entity.fCollisionRadius;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_WorldEntityDespawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::NET_ENTITY_ID netEntityId)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_DESPAWNED message{};
	message.iNetEntityId = netEntityId;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_ENTITY_DESPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_Despawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	using namespace LostArk::Shared;
	S2C_PLAYER_DESPAWNED message{};
	message.iNetEntityId = netEntityId;
	message.eReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_PLAYER_DESPAWNED, writer.Get_Buffer());
}

void LostArk::Server::CGameRoom::Broadcast_Spawned(
	const SERVER_PLAYER& player,
	const SESSION_ID exceptSessionId)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		if (sessionId == exceptSessionId)
			continue;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_Spawned(session, player))
			session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Broadcast_Despawned(
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_Despawned(session, netEntityId, reason))
			session->Request_Close();
	}
}

bool LostArk::Server::CGameRoom::Send_WorldEntitySpawnResult(
	const std::shared_ptr<CClientSession>& session,
	const std::string& placementId,
	const LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT result,
	const LostArk::Shared::NET_ENTITY_ID netEntityId)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_SPAWN_RESULT message{};
	message.strPlacementId = placementId;
	message.eResult = result;
	message.iNetEntityId = netEntityId;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_ENTITY_SPAWN_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_CharacterClassChangeResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request,
	const LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT result,
	const LostArk::Shared::CHARACTER_CLASS_ID activeClass)
{
	using namespace LostArk::Shared;
	S2C_CHARACTER_CLASS_CHANGE_RESULT message{};
	message.iClientSequence = request.iClientSequence;
	message.eResult = result;
	message.eRequestedClass = request.eCharacterClass;
	message.eActiveClass = activeClass;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_CHARACTER_CLASS_CHANGE_RESULT,
			writer.Get_Buffer());
}

void LostArk::Server::CGameRoom::Broadcast_WorldEntitySpawned(
	const SERVER_WORLD_ENTITY& entity)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_WorldEntitySpawned(session, entity))
			session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Broadcast_WorldEntityDespawned(
	const LostArk::Shared::NET_ENTITY_ID netEntityId)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session &&
			!Send_WorldEntityDespawned(session, netEntityId))
		{
			session->Request_Close();
		}
	}
}

void LostArk::Server::CGameRoom::Broadcast_WorldSnapshot()
{
	using namespace LostArk::Shared;
	S2C_WORLD_SNAPSHOT message{};
	message.iServerTick = m_iServerTick;
	message.eWorldId = m_eWorldId;
	message.Players.reserve(m_Players.size());
	message.Entities.reserve(m_WorldEntities.size());
	for (const auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		PLAYER_SNAPSHOT snapshot{};
		snapshot.iNetEntityId = player.iNetEntityId;
		snapshot.eCharacterClass = player.eCharacterClass;
		snapshot.fPositionX = player.fPositionX;
		snapshot.fPositionY = player.fPositionY;
		snapshot.fPositionZ = player.fPositionZ;
		snapshot.fYawDegrees = player.fYawDegrees;
		snapshot.eLocomotionState =
			(player.hasMoveGoal || player.TriggerMove.isActive) ?
			PLAYER_LOCOMOTION_STATE::MOVING : PLAYER_LOCOMOTION_STATE::IDLE;
		snapshot.eAction = player.eAction;
		snapshot.eStance = player.eStance;
		snapshot.iSkillId = player.iCurrentSkillId;
		snapshot.iActionStartTick = player.iActionStartTick;
		snapshot.iCurrentHp = player.iCurrentHp;
		snapshot.iMaximumHp = player.iMaximumHp;
		snapshot.iCurrentResource = player.iCurrentResource;
		snapshot.iMaximumResource = player.iMaximumResource;
		snapshot.isCombatReady = player.isCombatReady;
		snapshot.iComboStage = player.iComboStage;
		/* Collect, sort, then truncate: cutting during unordered_map iteration
		made the surviving cooldowns depend on hash order. Signed difference keeps
		ordering across a wrapped tick counter. */
		for (const auto& [skillId, cooldownEndTick] :
			player.CooldownEndTickBySkillId)
		{
			if (static_cast<std::int32_t>(cooldownEndTick - m_iServerTick) > 0)
				snapshot.Cooldowns.push_back({ skillId, cooldownEndTick });
		}
		std::sort(snapshot.Cooldowns.begin(), snapshot.Cooldowns.end(),
			[](const SKILL_COOLDOWN_SNAPSHOT& left,
				const SKILL_COOLDOWN_SNAPSHOT& right)
			{
				return left.iSkillId < right.iSkillId;
			});
		if (snapshot.Cooldowns.size() > MAX_PLAYER_COOLDOWNS)
			snapshot.Cooldowns.resize(MAX_PLAYER_COOLDOWNS);
		message.Players.push_back(snapshot);
	}
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		WORLD_ENTITY_SNAPSHOT snapshot{};
		snapshot.iNetEntityId = entity.iNetEntityId;
		snapshot.eAction = To_NetworkAction(entity.eAction);
		snapshot.strPatternId = entity.strPatternId;
		if (entity.eAction == SERVER_ENTITY_ACTION::PATTERN_WINDUP ||
			entity.eAction == SERVER_ENTITY_ACTION::PATTERN_ACTIVE ||
			entity.eAction == SERVER_ENTITY_ACTION::PATTERN_RECOVERY)
		{
			snapshot.strActionId = entity.strActionId;
		}
		snapshot.fPositionX = entity.fPositionX;
		snapshot.fPositionY = entity.fPositionY;
		snapshot.fPositionZ = entity.fPositionZ;
		snapshot.fYawDegrees = entity.fYawDegrees;
		snapshot.iActionStartTick = entity.iActionStartTick;
		snapshot.iPatternSequence = entity.iPatternSequence;
		snapshot.iPatternStageIndex = entity.iPatternStageIndex;
		snapshot.iCurrentHp = entity.iCurrentHp;
		snapshot.iMaximumHp = entity.iMaximumHp;
		snapshot.iPhase = entity.iPhase;
		message.Entities.push_back(std::move(snapshot));
	}
	message.DamageEvents = m_TickDamageEvents;

	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_SNAPSHOT, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}

std::shared_ptr<LostArk::Server::CClientSession>
LostArk::Server::CGameRoom::Find_Session(const SESSION_ID sessionId) const
{
	const auto iter = m_Sessions.find(sessionId);
	return iter == m_Sessions.end() ? nullptr : iter->second.lock();
}

void LostArk::Server::CGameRoom::Rollback_Join(const SESSION_ID sessionId)
{
	using namespace LostArk::Shared;
	const auto sessionPlayerIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionPlayerIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID playerId = sessionPlayerIter->second;
	const auto playerIter = m_Players.find(playerId);
	if (playerIter != m_Players.end())
	{
		m_PlayerIdByEntityId.erase(playerIter->second.iNetEntityId);
		m_Players.erase(playerIter);
	}
	m_PlayerIdBySessionId.erase(sessionPlayerIter);
	if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
		session->Bind_PlayerId(INVALID_PLAYER_ID);
}

const LostArk::Server::WORLD_BOOTSTRAP_PLACEMENT*
LostArk::Server::CGameRoom::Find_AvailablePlayerSpawn() const
{
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (!placement.isEnabled ||
			placement.eKind != WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN)
		{
			continue;
		}
		const bool isUsed = std::any_of(
			m_Players.begin(), m_Players.end(),
			[&placement](const auto& playerEntry)
			{
				return playerEntry.second.strSpawnPlacementId ==
					placement.strPlacementId;
			});
		if (!isUsed)
			return &placement;
	}
	return nullptr;
}

const LostArk::Server::WORLD_BOOTSTRAP_PLACEMENT*
LostArk::Server::CGameRoom::Find_Placement(
	const std::string& placementId) const
{
	const auto& placements = m_WorldBootstrap.Get_Placements();
	const auto iter = std::find_if(
		placements.begin(),
		placements.end(),
		[&placementId](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == placementId;
		});
	return placements.end() != iter ? &*iter : nullptr;
}

bool LostArk::Server::CGameRoom::Build_WorldEntity(
	const WORLD_BOOTSTRAP_PLACEMENT& placement,
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	SERVER_WORLD_ENTITY& outEntity)
{
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == netEntityId ||
		WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::COLLISION_BOX == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::END == placement.eKind)
	{
		m_strStatus = "World entity placement is invalid";
		return false;
	}

	SERVER_WORLD_ENTITY staged{};
	staged.iNetEntityId = netEntityId;
	staged.strPlacementId = placement.strPlacementId;
	staged.strArchetypeId = placement.strArchetypeId;
	staged.strEncounterId = placement.strEncounterId;
	staged.eKind = placement.eKind;
	staged.fPositionX = placement.fPositionX;
	staged.fPositionY = placement.fPositionY;
	staged.fPositionZ = placement.fPositionZ;
	staged.fYawDegrees = placement.fYawDegrees;
	if (WORLD_BOOTSTRAP_KIND::BOSS == staged.eKind)
	{
		const BOSS_RUNTIME_PROFILE* profile =
			m_GameplayCatalog.Find_Boss(staged.strArchetypeId);
		const auto* patterns =
			m_GameplayCatalog.Find_BossPatterns(staged.strEncounterId);
		if (nullptr == profile ||
			profile->strEncounterId != staged.strEncounterId ||
			nullptr == patterns || patterns->empty())
		{
			m_strStatus = "Boss gameplay profile or damage profile is missing";
			return false;
		}
		staged.iCurrentHp = profile->iMaximumHp;
		staged.iMaximumHp = profile->iMaximumHp;
		staged.iMaximumHealthBars = profile->iMaximumHealthBars;
		staged.iLastEvaluatedHealthBar = profile->iMaximumHealthBars;
		staged.fCollisionRadius = profile->fCollisionRadius;
		staged.fEngageDistance = profile->fEngageDistance;
		staged.fMoveSpeed = profile->fMoveSpeed;
		staged.iPhaseTwoHpPercent = profile->iPhaseTwoHpPercent;
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT projected{};
			if (!m_ServerNavigation.Project_Point(
				staged.fPositionX,
				staged.fPositionZ,
				projected))
			{
				m_strStatus = "Boss placement is outside server navigation";
				return false;
			}
			staged.fPositionX = projected.x;
			staged.fPositionY = projected.y;
			staged.fPositionZ = projected.z;
		}
	}
	outEntity = std::move(staged);
	return true;
}

bool LostArk::Server::CGameRoom::Initialize_WorldEntities()
{
	m_WorldEntities.clear();
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (!placement.isEnabled ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::TRIGGER_BOX ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::COLLISION_BOX)
		{
			continue;
		}
		if (m_iNextNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
		{
			m_strStatus = "World entity ID space exhausted";
			return false;
		}
		SERVER_WORLD_ENTITY entity{};
		if (!Build_WorldEntity(placement, m_iNextNetEntityId, entity))
			return false;
		++m_iNextNetEntityId;
		m_WorldEntities.push_back(std::move(entity));
	}
	return true;
}

bool LostArk::Server::CGameRoom::Reset_CharacterSelectArenaWhenEmpty()
{
	using LostArk::Shared::WORLD_ID;
	if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId || !m_Players.empty())
		return true;

	std::string resetStatus;
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		m_isReady = false;
		return false;
	}
	if (!Initialize_WorldEntities())
	{
		m_isReady = false;
		return false;
	}
	m_TickDamageEvents.clear();
	m_strStatus = "Character Select arena reset after the room became empty";
	return true;
}

bool LostArk::Server::CGameRoom::Activate_Encounter(
	const std::string& placementId)
{
	const WORLD_BOOTSTRAP_PLACEMENT* placement = Find_Placement(placementId);
	if (nullptr == placement ||
		placement->isEnabled ||
		WORLD_BOOTSTRAP_KIND::BOSS != placement->eKind ||
		m_iNextNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
	{
		return false;
	}

	const auto existing = std::find_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[&placementId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.strPlacementId == placementId;
		});
	if (m_WorldEntities.end() != existing)
		return false;

	SERVER_WORLD_ENTITY staged{};
	if (!Build_WorldEntity(*placement, m_iNextNetEntityId, staged))
		return false;

	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	return true;
}

bool LostArk::Server::CGameRoom::Spawn_Monster(
	const std::string& spawnGroupId,
	const SPAWN_GROUP_ENTRY& entry,
	const SPAWN_GROUP_ANCHOR& anchor,
	const MONSTER_RUNTIME_PROFILE& profile,
	const std::uint32_t ordinal)
{
	if (spawnGroupId.empty() ||
		entry.strArchetypeId != profile.strArchetypeId ||
		m_iNextNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
	{
		return false;
	}

	SERVER_NAV_POINT projected{
		anchor.fPositionX, anchor.fPositionY, anchor.fPositionZ };
	if (m_ServerNavigation.Is_Loaded() &&
		!m_ServerNavigation.Project_Point(
			anchor.fPositionX, anchor.fPositionZ, projected))
	{
		return false;
	}

	SERVER_WORLD_ENTITY staged{};
	staged.iNetEntityId = m_iNextNetEntityId;
	staged.strPlacementId = spawnGroupId + "." +
		std::to_string(staged.iNetEntityId) + "." + std::to_string(ordinal);
	staged.strArchetypeId = profile.strArchetypeId;
	staged.strSpawnGroupId = spawnGroupId;
	staged.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
	staged.eAction = SERVER_ENTITY_ACTION::IDLE;
	staged.fPositionX = projected.x;
	staged.fPositionY = projected.y;
	staged.fPositionZ = projected.z;
	staged.fYawDegrees = anchor.fYawDegrees;
	staged.iCurrentHp = profile.iMaxHp;
	staged.iMaximumHp = profile.iMaxHp;
	staged.iAttackPower = profile.iAttackPower;
	staged.iDefense = profile.iDefense;
	staged.fCollisionRadius = profile.fCollisionRadius;
	staged.fEngageDistance = profile.fEngageRange;
	staged.fMoveSpeed = profile.fMoveSpeed;
	staged.fAttackRange = profile.fAttackRange;
	staged.iPatternTelegraphMs = profile.iAttackWindupMs;
	staged.iPatternActiveMs = profile.iAttackActiveMs;
	staged.iPatternRecoveryMs = profile.iAttackRecoveryMs;
	staged.iDeadDespawnMs = profile.iDeadDespawnMs;

	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	return true;
}

std::uint32_t LostArk::Server::CGameRoom::Count_SpawnGroupEntities(
	const std::string& spawnGroupId) const
{
	return static_cast<std::uint32_t>(std::count_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[&spawnGroupId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.eKind == WORLD_BOOTSTRAP_KIND::MONSTER &&
				entity.strSpawnGroupId == spawnGroupId;
		}));
}

void LostArk::Server::CGameRoom::Update_Players(const float fixedDeltaSeconds)
{
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (m_ServerTriggerSystem.Update_PlayerMotion(
			player, fixedDeltaSeconds))
		{
			continue;
		}
		m_PlayerSkillSystem.Update(
			player,
			m_WorldEntities,
			m_GameplayCatalog,
			m_ServerNavigation.Is_Loaded() ? &m_ServerNavigation : nullptr,
			fixedDeltaSeconds,
			updateTick,
			m_TickDamageEvents);
		if (LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction)
			continue;
		if (!player.hasMoveGoal)
			continue;
		float targetX = player.fMoveGoalX;
		float targetY = player.fPositionY;
		float targetZ = player.fMoveGoalZ;
		if (player.iMovePathIndex < player.MovePath.size())
		{
			const SERVER_NAV_POINT& pathPoint =
				player.MovePath[player.iMovePathIndex];
			targetX = pathPoint.x;
			targetY = pathPoint.y;
			targetZ = pathPoint.z;
		}
		const float deltaX = targetX - player.fPositionX;
		const float deltaZ = targetZ - player.fPositionZ;
		const float distance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
		const bool reachedPathPoint = distance <= MOVE_STOP_DISTANCE;
		float proposedX = targetX;
		float proposedY = targetY;
		float proposedZ = targetZ;
		if (!reachedPathPoint)
		{
			player.fYawDegrees =
				std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
			const float moveDistance = (std::min)(
				player.fMoveSpeed * fixedDeltaSeconds, distance);
			const float moveRatio = moveDistance / distance;
			proposedX = player.fPositionX + deltaX * moveRatio;
			proposedY = player.fPositionY +
				(targetY - player.fPositionY) * moveRatio;
			proposedZ = player.fPositionZ + deltaZ * moveRatio;
		}

		float resolvedX = player.fPositionX;
		float resolvedY = player.fPositionY;
		float resolvedZ = player.fPositionZ;
		bool wasBlocked = false;
		if (!m_ServerCollisionSystem.Resolve_PlayerMove(
			player,
			proposedX,
			proposedY,
			proposedZ,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked))
		{
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0;
			continue;
		}
		player.fPositionX = resolvedX;
		player.fPositionY = resolvedY;
		player.fPositionZ = resolvedZ;
		if (wasBlocked)
		{
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0;
			continue;
		}
		if (reachedPathPoint)
		{
			if (player.iMovePathIndex < player.MovePath.size())
				++player.iMovePathIndex;
			if (player.iMovePathIndex >= player.MovePath.size())
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0;
			}
			continue;
		}
	}
}

void LostArk::Server::CGameRoom::Update_WorldEntities(
	const float fixedDeltaSeconds)
{
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (entity.eKind == WORLD_BOOTSTRAP_KIND::BOSS &&
			m_ServerNavigation.Is_Loaded())
		{
			m_ValtanBrain.Update(
				entity,
				m_Players,
				m_GameplayCatalog,
				m_ServerNavigation,
				fixedDeltaSeconds,
				updateTick,
				m_TickDamageEvents);
		}
		else if (entity.eKind == WORLD_BOOTSTRAP_KIND::MONSTER &&
			m_ServerNavigation.Is_Loaded())
		{
			m_MonsterBrain.Update(
				entity,
				m_Players,
				m_GameplayCatalog,
				m_ServerNavigation,
				fixedDeltaSeconds,
				updateTick,
				m_TickDamageEvents);
		}
	}

	for (auto iter = m_WorldEntities.begin(); iter != m_WorldEntities.end();)
	{
		const bool shouldDespawn =
			WORLD_BOOTSTRAP_KIND::MONSTER == iter->eKind &&
			SERVER_ENTITY_ACTION::DEAD == iter->eAction &&
			iter->fActionElapsedSeconds * 1000.f >=
				static_cast<float>(iter->iDeadDespawnMs);
		if (!shouldDespawn)
		{
			++iter;
			continue;
		}
		Broadcast_WorldEntityDespawned(iter->iNetEntityId);
		iter = m_WorldEntities.erase(iter);
	}
}
