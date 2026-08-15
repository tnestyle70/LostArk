#include "GameRoom.h"

#include "ClientSession.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"

#include <algorithm>
#include <chrono>
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
	constexpr float DEGREES_TO_RADIANS = 0.01745329251994329577f;
	constexpr float PLAYER_TURN_DEGREES_PER_SECOND = 540.f;
	constexpr float DIRECT_BEARING_DISTANCE = 1.5f;

	std::uint64_t To_Microseconds(
		const std::chrono::steady_clock::duration duration)
	{
		return static_cast<std::uint64_t>(
			std::chrono::duration_cast<std::chrono::microseconds>(
				duration).count());
	}

	bool Is_BestEffortCommand(const ROOM_COMMAND_TYPE type)
	{
		return ROOM_COMMAND_TYPE::MOVE == type ||
			ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM == type;
	}

	bool Is_SameBestEffortStream(
		const ROOM_COMMAND& queued,
		const ROOM_COMMAND& incoming)
	{
		if (queued.iSessionId != incoming.iSessionId ||
			queued.eType != incoming.eType ||
			!Is_BestEffortCommand(incoming.eType))
		{
			return false;
		}
		if (ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM == incoming.eType)
		{
			return queued.UpdateSkillAim.iSkillId ==
				incoming.UpdateSkillAim.iSkillId;
		}
		return true;
	}

	bool Try_RemoveCoalescedCommand(
		std::deque<ROOM_COMMAND>& commands,
		const ROOM_COMMAND& incoming)
	{
		// A command from the same session is an ordering barrier. This keeps a
		// newer move/aim from jumping across skill, release, or lifecycle intent.
		for (std::size_t index = commands.size(); index > 0u; --index)
		{
			const ROOM_COMMAND& queued = commands[index - 1u];
			if (queued.iSessionId != incoming.iSessionId)
				continue;
			if (!Is_SameBestEffortStream(queued, incoming))
				return false;
			commands.erase(commands.begin() +
				static_cast<std::ptrdiff_t>(index - 1u));
			return true;
		}
		return false;
	}

	float Wrap_Degrees(float degrees)
	{
		while (degrees > 180.f)
			degrees -= 360.f;
		while (degrees < -180.f)
			degrees += 360.f;
		return degrees;
	}
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

	void Smooth_MovePath(
		const CServerNavigation& navigation,
		const float startX,
		const float startZ,
		const float goalX,
		const float goalZ,
		std::vector<SERVER_NAV_POINT>& path)
	{
		if (path.empty())
			return;
		SERVER_NAV_POINT exactGoal{};
		if (navigation.Sample_Position(goalX, goalZ, exactGoal) &&
			navigation.Has_LineOfSight(
				path.back().x, path.back().z, goalX, goalZ))
		{
			path.back() = exactGoal;
		}
		std::vector<SERVER_NAV_POINT> smoothed;
		smoothed.reserve(path.size());
		float fromX = startX;
		float fromZ = startZ;
		std::size_t index = 0;
		while (index < path.size())
		{
			std::size_t visible = index;
			for (std::size_t candidate = path.size();
				candidate > index + 1; --candidate)
			{
				const SERVER_NAV_POINT& point = path[candidate - 1];
				if (navigation.Has_LineOfSight(fromX, fromZ, point.x, point.z))
				{
					visible = candidate - 1;
					break;
				}
			}
			smoothed.push_back(path[visible]);
			fromX = path[visible].x;
			fromZ = path[visible].z;
			index = visible + 1;
		}
		path = std::move(smoothed);
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

	WORLD_DESTRUCTION_RUNTIME_STATE To_NetworkDestructionState(
		const WORLD_DESTRUCTION_STATE state)
	{
		switch (state)
		{
		case WORLD_DESTRUCTION_STATE::INTACT:
			return WORLD_DESTRUCTION_RUNTIME_STATE::INTACT;
		case WORLD_DESTRUCTION_STATE::BREAKING:
			return WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING;
		case WORLD_DESTRUCTION_STATE::FRACTURED:
			return WORLD_DESTRUCTION_RUNTIME_STATE::FRACTURED;
		case WORLD_DESTRUCTION_STATE::DESPAWNED:
			return WORLD_DESTRUCTION_RUNTIME_STATE::DESPAWNED;
		default:
			return WORLD_DESTRUCTION_RUNTIME_STATE::END;
		}
	}

	WORLD_DESTRUCTION_STATE_WIRE To_NetworkDestructionState(
		const WORLD_DESTRUCTION_GROUP_STATE& state)
	{
		WORLD_DESTRUCTION_STATE_WIRE wire{};
		wire.strGroupId = state.strGroupId;
		wire.eState = To_NetworkDestructionState(state.eState);
		wire.iStateVersion = state.iStateVersion;
		wire.iStateStartTick = state.iStateStartTick;
		wire.iCommitTick = state.iCommitTick;
		return wire;
	}

	std::uint32_t Hash_DestructionEventIdentity(
		const std::uint32_t encounterEpoch,
		const std::uint64_t eventSequence,
		const WORLD_DESTRUCTION_STATE_TRANSITION& transition,
		const WORLD_DESTRUCTION_BINDING_APPLICATION& application,
		const std::uint32_t serverTick)
	{
		std::uint32_t hash = 2166136261u;
		const auto appendByte = [&hash](const std::uint8_t value)
		{
			hash ^= value;
			hash *= 16777619u;
		};
		const auto appendU64 = [&appendByte](const std::uint64_t value)
		{
			for (std::uint32_t shift = 0u; shift < 64u; shift += 8u)
				appendByte(static_cast<std::uint8_t>(value >> shift));
		};
		const auto appendString = [&appendByte](const std::string& value)
		{
			for (const unsigned char character : value)
				appendByte(character);
			appendByte(0xffu);
		};

		appendU64(encounterEpoch);
		appendU64(eventSequence);
		appendString(transition.strGroupId);
		appendString(transition.strMutationId);
		appendString(application.strBindingId);
		appendU64(application.iPatternSequence);
		appendU64(application.iSourceNetEntityId);
		appendU64(serverTick);
		return 0u == hash ? 1u : hash;
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
	m_EstherSkillSystem.Initialize(worldId);
	/* Bern joins the areas that require navigation. Without a grid the room keeps
	the spawn height for the whole session and straight-line XZ movement walks
	through the castle stairs, so a missing or malformed grid fails admission here
	instead of degrading silently. */
	if ((LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::TRAINING_GROUND == worldId ||
		LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::BERN == worldId) &&
		!m_ServerNavigation.Load(m_WorldBootstrap.Get_AreaId()))
	{
		m_strStatus = m_ServerNavigation.Get_Status();
		return;
	}
	if (!m_ServerTriggerSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), m_strStatus,
		LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId))
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
	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId)
	{
		if (!m_WorldDestructionBootstrap.Load_ValtanArena())
		{
			m_strStatus = m_WorldDestructionBootstrap.Get_Status();
			return;
		}
		if (!m_WorldDestructionRuntime.Initialize(
			m_WorldDestructionBootstrap.Get_DescriptorGraph(),
			m_strStatus, 1u))
		{
			return;
		}
		for (const WORLD_DESTRUCTION_MUTATION_DESCRIPTOR& mutation :
			m_WorldDestructionBootstrap.Get_DescriptorGraph().Mutations)
		{
			if ((!mutation.strCollisionStateId.empty() &&
				 !m_ServerCollisionSystem.Has_CollisionStateTarget(
					 mutation.strCollisionStateId)) ||
				(!mutation.strNavigationStateId.empty() &&
				 !m_ServerNavigation.Has_Condition(
					 mutation.strNavigationStateId)))
			{
				m_strStatus = "World destruction dynamic state reference is unknown: " +
					mutation.strMutationId;
				return;
			}
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
	if (!m_acceptsCommands)
		return false;

	if (Is_BestEffortCommand(command.eType))
	{
		if (Try_RemoveCoalescedCommand(m_InboundCommands, command))
		{
			if (ROOM_COMMAND_TYPE::MOVE == command.eType)
				++m_PerformanceMetrics.iCoalescedMoveCommandCount;
			else
				++m_PerformanceMetrics.iCoalescedAimCommandCount;
			m_InboundCommands.push_back(std::move(command));
			return true;
		}
		if (m_InboundCommands.size() >= MAX_BEST_EFFORT_COMMAND_COUNT)
		{
			++m_PerformanceMetrics.iDroppedBestEffortCommandCount;
			return true;
		}
	}
	else if (ROOM_COMMAND_TYPE::LEAVE != command.eType &&
		m_InboundCommands.size() >= MAX_RELIABLE_COMMAND_COUNT)
	{
		++m_PerformanceMetrics.iRejectedReliableCommandCount;
		return false;
	}
	else if (ROOM_COMMAND_TYPE::LEAVE == command.eType &&
		m_InboundCommands.size() >= MAX_INBOUND_COMMAND_COUNT)
	{
		++m_PerformanceMetrics.iRejectedCleanupCommandCount;
		return false;
	}

	m_InboundCommands.push_back(std::move(command));
	m_PerformanceMetrics.iIngressHighWatermark = (std::max)(
		m_PerformanceMetrics.iIngressHighWatermark,
		m_InboundCommands.size());
	return true;
}

LostArk::Server::SERVER_ROOM_PERFORMANCE_METRICS
LostArk::Server::CGameRoom::Get_PerformanceMetrics() const
{
	std::scoped_lock lock{ m_CommandMutex };
	return m_PerformanceMetrics;
}

bool LostArk::Server::CGameRoom::Try_SealPrivateArenaForRetirement()
{
	using LostArk::Shared::WORLD_ID;
	if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId)
		return false;

	// Gameplay containers are room-thread-owned. The mutex makes the empty
	// decision atomic against every receive-thread Enqueue call.
	std::scoped_lock lock{ m_CommandMutex };
	if (!m_acceptsCommands)
		return true;
	if (!m_InboundCommands.empty() ||
		!m_PendingWorldTransfers.empty() ||
		!m_Sessions.empty() ||
		!m_Players.empty() ||
		!m_PlayerIdBySessionId.empty() ||
		!m_PlayerIdByEntityId.empty())
	{
		return false;
	}
	m_acceptsCommands = false;
	return true;
}

bool LostArk::Server::CGameRoom::Commit_WorldTransferDeparture(
	const SESSION_ID sessionId)
{
	if (!m_isReady || !m_PlayerIdBySessionId.contains(sessionId))
		return false;

	// CServerApp invokes this on the room thread after staging the target
	// REGISTER/ENTER commands. The target cannot process them until this call
	// has cleared the old CClientSession player binding.
	Leave(
		sessionId,
		LostArk::Shared::PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
	return !m_PlayerIdBySessionId.contains(sessionId);
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
	const auto tickStart = std::chrono::steady_clock::now();
	const auto recordTickDuration = [this, tickStart]()
		{
			const std::uint64_t elapsedMicroseconds = To_Microseconds(
				std::chrono::steady_clock::now() - tickStart);
			std::scoped_lock lock{ m_CommandMutex };
			++m_PerformanceMetrics.iTickCount;
			m_PerformanceMetrics.iLastTickMicroseconds = elapsedMicroseconds;
			m_PerformanceMetrics.iMaximumTickMicroseconds = (std::max)(
				m_PerformanceMetrics.iMaximumTickMicroseconds,
				elapsedMicroseconds);
		};

	std::deque<ROOM_COMMAND> commands;
	{
		std::scoped_lock lock{ m_CommandMutex };
		const std::size_t ingressDepth = m_InboundCommands.size();
		const std::size_t drainCount = (std::min)(
			ingressDepth, MAX_COMMANDS_DRAINED_PER_TICK);
		for (std::size_t index = 0u; index < drainCount; ++index)
		{
			commands.push_back(std::move(m_InboundCommands.front()));
			m_InboundCommands.pop_front();
		}
		m_PerformanceMetrics.iLastIngressDepth = ingressDepth;
		m_PerformanceMetrics.iLastDrainedCommandCount = drainCount;
		m_PerformanceMetrics.iLastRemainingCommandCount =
			m_InboundCommands.size();
		if (!m_InboundCommands.empty())
			++m_PerformanceMetrics.iDrainLimitedTickCount;
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
		case ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM:
			Handle_UpdateSkillAim(command.iSessionId, command.UpdateSkillAim);
			break;
		case ROOM_COMMAND_TYPE::USE_ESTHER_SKILL:
			Handle_UseEstherSkill(command.iSessionId, command.UseEstherSkill);
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
		case ROOM_COMMAND_TYPE::VALTAN_AUDITION:
			Handle_ValtanAudition(
				command.iSessionId,
				command.ValtanAudition);
			break;
		case ROOM_COMMAND_TYPE::LEAVE:
			Leave(command.iSessionId, command.eLeaveReason);
			break;
		}
	}

	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
	{
		recordTickDuration();
		return;
	}

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
			{
				const bool activated = Activate_Encounter(targetId);
#ifdef _DEBUG
				if (activated && WORLD_ID::VALTAN_ARENA == m_eWorldId &&
					"boss.valtan.center" == targetId)
				{
					SERVER_WORLD_ENTITY* boss = Find_AuditionBoss();
					if (nullptr == boss)
						return false;
					const std::uint32_t openingHp =
						CValtanBrain::Resolve_HealthBarHp(*boss, 159u);
					if (0u == openingHp)
						return false;
					/* The next normal brain tick observes only 160 -> 159 and
					queues the real opening wall charge. Nothing here plays a
					camera, breaks a wall or emits a Client cue directly. */
					boss->iCurrentHp = openingHp;
					boss->iLastEvaluatedHealthBar = 160u;
				}
#endif
				return activated;
			}
			return false;
		});
	for (SERVER_WORLD_TRANSFER_REQUEST& transfer : transfers)
	{
		if (!m_PlayerIdBySessionId.contains(transfer.iSessionId))
			continue;
		const bool alreadyStaged = std::any_of(
			m_PendingWorldTransfers.begin(),
			m_PendingWorldTransfers.end(),
			[sessionId = transfer.iSessionId](
				const SERVER_WORLD_TRANSFER_REQUEST& pending)
			{
				return pending.iSessionId == sessionId;
			});
		if (!alreadyStaged)
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
	m_EstherSkillSystem.Update(fixedDeltaSeconds, !m_Players.empty());
	Update_WorldEntities(fixedDeltaSeconds);
	if (!m_isReady)
	{
		recordTickDuration();
		return;
	}
	if (!Commit_DueWorldDestruction(updateTick))
	{
		m_isReady = false;
		recordTickDuration();
		return;
	}
	m_iServerTick = updateTick;
	if (!m_Players.empty())
		Broadcast_WorldSnapshot();
	recordTickDuration();
	if (!m_Players.empty() && 0u == (m_iServerTick % 300u))
	{
		const SERVER_ROOM_PERFORMANCE_METRICS metrics =
			Get_PerformanceMetrics();
		std::size_t maximumCurrentOutboundFrames = 0u;
		std::size_t maximumOutboundFrameHighWatermark = 0u;
		std::uint64_t snapshotCoalescedCount = 0u;
		std::uint64_t snapshotDroppedCount = 0u;
		std::uint64_t reliableRejectedCount = 0u;
		std::uint64_t sendFailureCount = 0u;
		std::uint64_t maximumWireSendMicroseconds = 0u;
		for (const auto& [sessionId, weakSession] : m_Sessions)
		{
			(void)sessionId;
			const std::shared_ptr<CClientSession> session = weakSession.lock();
			if (nullptr == session)
				continue;
			const CLIENT_SESSION_OUTBOUND_METRICS sessionMetrics =
				session->Get_OutboundMetrics();
			maximumCurrentOutboundFrames = (std::max)(
				maximumCurrentOutboundFrames,
				sessionMetrics.iCurrentQueuedFrameCount);
			maximumOutboundFrameHighWatermark = (std::max)(
				maximumOutboundFrameHighWatermark,
				sessionMetrics.iQueuedFrameHighWatermark);
			snapshotCoalescedCount +=
				sessionMetrics.iSnapshotCoalescedFrameCount;
			snapshotDroppedCount +=
				sessionMetrics.iSnapshotDroppedFrameCount;
			reliableRejectedCount +=
				sessionMetrics.iReliableRejectedFrameCount;
			sendFailureCount += sessionMetrics.iSendFailureCount;
			maximumWireSendMicroseconds = (std::max)(
				maximumWireSendMicroseconds,
				sessionMetrics.iMaximumFrameSendMicroseconds);
		}
		std::cout << "[RoomPerf] World=" << static_cast<unsigned>(m_eWorldId)
			<< " Tick=" << m_iServerTick
			<< " TickUs=" << metrics.iLastTickMicroseconds
			<< " TickMaxUs=" << metrics.iMaximumTickMicroseconds
			<< " Ingress=" << metrics.iLastIngressDepth
			<< " IngressHigh=" << metrics.iIngressHighWatermark
			<< " Drained=" << metrics.iLastDrainedCommandCount
			<< " Remaining=" << metrics.iLastRemainingCommandCount
			<< " MoveCoalesced=" << metrics.iCoalescedMoveCommandCount
			<< " AimCoalesced=" << metrics.iCoalescedAimCommandCount
			<< " BestEffortDropped="
			<< metrics.iDroppedBestEffortCommandCount
			<< " ReliableRejected="
			<< metrics.iRejectedReliableCommandCount
			<< " SnapshotEncodeUs="
			<< metrics.iLastSnapshotEncodeMicroseconds
			<< " SnapshotEnqueueUs="
			<< metrics.iLastSnapshotEnqueueMicroseconds
			<< " SessionEnqueueMaxUs="
			<< metrics.iMaximumSessionEnqueueMicroseconds
			<< " SnapshotEnqueueFailures="
			<< metrics.iSnapshotEnqueueFailureCount
			<< " OutboundQueuedMax=" << maximumCurrentOutboundFrames
			<< " OutboundHighMax=" << maximumOutboundFrameHighWatermark
			<< " SnapshotCoalesced=" << snapshotCoalescedCount
			<< " SnapshotDropped=" << snapshotDroppedCount
			<< " OutboundReliableRejected=" << reliableRejectedCount
			<< " WireSendMaxUs=" << maximumWireSendMicroseconds
			<< " WireSendFailures=" << sendFailureCount << '\n';
	}
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
	if (nullptr == session || !Is_Valid_EnterWorld(enterWorld) ||
		enterWorld.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.contains(sessionId) ||
		m_Players.size() >= MAX_WORLD_SNAPSHOT_PLAYERS ||
		m_iNextPlayerId == INVALID_PLAYER_ID ||
		m_iNextNetEntityId == INVALID_NET_ENTITY_ID)
	{
		if (nullptr != session)
			session->Request_Close();
		return false;
	}
	if (Is_PlayerAdmissionFull())
	{
		if (Send_EnterRejected(
				session, ENTER_WORLD_REJECTION_REASON::ROOM_FULL))
		{
			session->Request_Close_After_Flush();
		}
		else
		{
			session->Request_Close();
		}
		return false;
	}
	const WORLD_BOOTSTRAP_PLACEMENT* spawn = Find_AvailablePlayerSpawn();
	if (nullptr == spawn)
	{
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
	// A gauge starts full: the stance it pays for is available on entry, and a
	// class without one keeps both at 0 so nothing ever drains.
	player.iMaximumIdentity = playerProfile->iMaximumIdentity;
	player.iCurrentIdentity = playerProfile->iMaximumIdentity;
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
	if (!Send_WorldDestructionFullSync(session))
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

	m_ValtanAuditionSequenceBySessionId.erase(sessionId);
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

	if (!Reset_ReplayableArenaWhenEmpty())
	{
		std::cerr << "Replayable arena reset failed. World="
			<< static_cast<unsigned>(m_eWorldId) << ", Status="
			<< m_strStatus << '\n';
	}
	if (!Reset_ValtanArenaWhenEmpty())
	{
		std::cerr << "Valtan arena reset failed: "
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
		Smooth_MovePath(
			m_ServerNavigation,
			player.fPositionX,
			player.fPositionZ,
			move.fGoalX,
			move.fGoalZ,
			player.MovePath);
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
	player.iCurrentIdentity = player.iMaximumIdentity;
	player.iIdentityAccumulator = 0u;
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

void LostArk::Server::CGameRoom::Handle_UpdateSkillAim(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_UPDATE_SKILL_AIM& updateSkillAim)
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

	m_PlayerSkillSystem.Update_Aim(
		playerIter->second,
		updateSkillAim,
		m_GameplayCatalog);
}

void LostArk::Server::CGameRoom::Handle_UseEstherSkill(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_USE_ESTHER_SKILL& useEstherSkill)
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
	if (LostArk::Shared::PLAYER_ACTION_STATE::DEAD ==
		playerIter->second.eAction)
	{
		return;
	}
	/* The entity id is checked before the gauge so a consume can never be
	followed by a failed spawn: rejecting here leaves the gauge untouched and
	the snapshot keeps telling every party member it is still full. */
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == m_iNextNetEntityId)
		return;

	std::string archetypeId;
	if (ESTHER_USE_REJECTION::NONE != m_EstherSkillSystem.Try_Consume(
		useEstherSkill.iSlotIndex, archetypeId))
	{
		return;
	}
	Spawn_EstherSummon(
		archetypeId,
		playerIter->second,
		useEstherSkill.fAimX,
		useEstherSkill.fAimZ);
}

bool LostArk::Server::CGameRoom::Spawn_EstherSummon(
	const std::string& archetypeId,
	const SERVER_PLAYER& caster,
	const float aimX,
	const float aimZ)
{
	if (archetypeId.empty() ||
		LostArk::Shared::INVALID_NET_ENTITY_ID == m_iNextNetEntityId)
	{
		return false;
	}

	// The summon stands where the caster stands and looks where the caster
	// aimed. A degenerate aim (cursor on the caster) keeps the caster's yaw.
	float yawDegrees = caster.fYawDegrees;
	const float directionX = aimX - caster.fPositionX;
	const float directionZ = aimZ - caster.fPositionZ;
	if (std::isfinite(directionX) && std::isfinite(directionZ) &&
		(directionX * directionX + directionZ * directionZ) > 0.0001f)
	{
		yawDegrees = std::atan2(directionX, directionZ) * RADIANS_TO_DEGREES;
	}

	const std::uint32_t startTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;

	SERVER_WORLD_ENTITY staged{};
	staged.iNetEntityId = m_iNextNetEntityId;
	staged.strPlacementId =
		"esther." + archetypeId + "." + std::to_string(staged.iNetEntityId);
	staged.strArchetypeId = archetypeId;
	staged.eKind = WORLD_BOOTSTRAP_KIND::NPC;
	staged.eAction = SERVER_ENTITY_ACTION::IDLE;
	staged.strActionId = ESTHER_ACTION_APPEAR;
	staged.isEstherSummon = true;
	staged.fPositionX = caster.fPositionX;
	staged.fPositionY = caster.fPositionY;
	staged.fPositionZ = caster.fPositionZ;
	staged.fYawDegrees = yawDegrees;
	staged.iActionStartTick = startTick;
	staged.iCurrentHp = 1u;
	staged.iMaximumHp = 1u;

	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	return true;
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

LostArk::Server::SERVER_WORLD_ENTITY*
LostArk::Server::CGameRoom::Find_AuditionBoss()
{
	const auto found = std::find_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind &&
				"boss.valtan.center" == entity.strPlacementId &&
				"BOSS_VALTAN" == entity.strArchetypeId;
		});
	return m_WorldEntities.end() == found ? nullptr : &*found;
}

bool LostArk::Server::CGameRoom::Has_EngagedAuditionPlayer(
	const SERVER_WORLD_ENTITY& boss) const
{
	/* CValtanBrain::Update drops its target and calls FinishPattern when no
	combat-ready living player is inside the engage distance, so a pattern
	queued while that is true would be discarded before it ever started. The
	brain widens the radius to the longest authored pattern range; this checks
	only the boss's own engage distance, which is the narrower of the two, so an
	accepted audition is always one the brain will also act on. */
	for (const auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (0u == player.iCurrentHp || !player.isCombatReady ||
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction)
		{
			continue;
		}
		const float deltaX = player.fPositionX - boss.fPositionX;
		const float deltaZ = player.fPositionZ - boss.fPositionZ;
		if (deltaX * deltaX + deltaZ * deltaZ <=
			boss.fEngageDistance * boss.fEngageDistance)
		{
			return true;
		}
	}
	return false;
}

bool LostArk::Server::CGameRoom::Reset_ValtanAuditionState(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t resetTick,
	std::string& status)
{
	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA != m_eWorldId ||
		0u == resetTick || !m_WorldDestructionRuntime.Is_Initialized())
	{
		status = "Valtan audition reset is unavailable";
		return false;
	}
	const WORLD_BOOTSTRAP_PLACEMENT* placement =
		Find_Placement(boss.strPlacementId);
	if (nullptr == placement ||
		WORLD_BOOTSTRAP_KIND::BOSS != placement->eKind)
	{
		status = "Valtan audition boss placement is unavailable";
		return false;
	}

	SERVER_WORLD_ENTITY stagedBoss{};
	if (!Build_WorldEntity(*placement, boss.iNetEntityId, stagedBoss))
	{
		status = m_strStatus;
		return false;
	}
	/* Pattern sequence is an edge identity on the Client. Preserve the live
	   monotonic base so Reset + Play can never reuse a completed camera key. */
	stagedBoss.iPatternSequence = boss.iPatternSequence;

	CWorldDestructionRuntime stagedDestruction = m_WorldDestructionRuntime;
	if (!stagedDestruction.Reset(status, resetTick))
		return false;

	boss = std::move(stagedBoss);
	m_WorldDestructionRuntime = std::move(stagedDestruction);
	m_ServerCollisionSystem.Reset_RuntimeStates();
	m_ServerNavigation.Reset_RuntimeBlockers();
	m_iNextWorldDestructionEventSequence = 1u;
	m_iValtanAuditionArmedHealthBar = 0u;
	Invalidate_DynamicNavigationPaths();

	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_WorldDestructionFullSync(session))
			session->Request_Close();
	}
	status = "Valtan audition reset and full-sync completed";
	return true;
}

LostArk::Shared::VALTAN_AUDITION_RESULT
LostArk::Server::CGameRoom::Evaluate_ValtanAudition(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
	std::uint32_t& outCurrentHealthBar)
{
	using namespace LostArk::Shared;
	outCurrentHealthBar = 0u;

#ifndef _DEBUG
	/* A Release Server keeps the packet type known so this answers an explicit
	rejection instead of closing the session on an unrecognised frame, but it
	never moves a boss. */
	(void)sessionId;
	(void)request;
	return VALTAN_AUDITION_RESULT::REJECTED_RELEASE_BUILD;
#else
	std::uint32_t& currentHealthBar = outCurrentHealthBar;
	const auto evaluate = [&]() -> VALTAN_AUDITION_RESULT
	{
		if (WORLD_ID::VALTAN_ARENA != m_eWorldId ||
			!m_PlayerIdBySessionId.contains(sessionId))
		{
			return VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD;
		}
		const auto handled =
			m_ValtanAuditionSequenceBySessionId.find(sessionId);
		if (m_ValtanAuditionSequenceBySessionId.end() != handled &&
			handled->second >= request.iRequestSequence)
		{
			return VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED;
		}
		SERVER_WORLD_ENTITY* boss = Find_AuditionBoss();
		if (nullptr == boss &&
			VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR == request.eOperation)
		{
			(void)Activate_Encounter("boss.valtan.center");
			boss = Find_AuditionBoss();
		}
		if (nullptr == boss)
			return VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS;

		currentHealthBar = CValtanBrain::Calculate_HealthBar(*boss);
		if (0u == boss->iCurrentHp ||
			SERVER_ENTITY_ACTION::DEAD == boss->eAction)
		{
			return VALTAN_AUDITION_RESULT::REJECTED_BOSS_DEAD;
		}

		const std::vector<BOSS_PATTERN_DEFINITION>* patterns =
			m_GameplayCatalog.Find_BossPatterns(boss->strEncounterId);
		if (nullptr == patterns)
			return VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR;
		const auto authored = std::find_if(
			patterns->begin(),
			patterns->end(),
			[&request](const BOSS_PATTERN_DEFINITION& pattern)
			{
				return BOSS_PATTERN_SELECTION::HEALTH_BAR ==
					pattern.eSelection &&
					pattern.iTriggerHealthBar == request.iTargetHealthBar;
			});
		if (patterns->end() == authored)
			return VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR;

		const bool isOneClickPlay =
			VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR == request.eOperation;
		/* ARM/CROSS are diagnostics over the current encounter. PLAY is the
		   repeatable user-facing audition: it resets the authoritative boss and
		   destruction generation first, so a previous 159/80 run cannot make the
		   button silently unavailable. */
		if (!isOneClickPlay &&
			(boss->TriggeredPatternIds.end() != std::find(
			boss->TriggeredPatternIds.begin(),
			boss->TriggeredPatternIds.end(),
			authored->strPatternId) ||
			!boss->PendingPatternIds.empty() ||
			!boss->strPatternId.empty()))
		{
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}
		if (isOneClickPlay)
		{
			const auto playerId = m_PlayerIdBySessionId.find(sessionId);
			if (m_PlayerIdBySessionId.end() != playerId)
			{
				const auto player = m_Players.find(playerId->second);
				if (m_Players.end() != player && 0u != player->second.iCurrentHp)
				{
					player->second.isCombatReady = true;
					if (!m_ServerTriggerSystem.Place_PlayerAtValtanAuditionBait(
						player->second,
						0u == m_iServerTick ? 1u : m_iServerTick))
					{
						return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
					}
				}
			}
		}
		if (!Has_EngagedAuditionPlayer(*boss))
			return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;

		if (VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR == request.eOperation)
		{
			const std::uint32_t armedBar = request.iTargetHealthBar + 1u;
			const std::uint32_t armedHp =
				CValtanBrain::Resolve_HealthBarHp(*boss, armedBar);
			if (0u == armedHp)
				return VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR;

			boss->iCurrentHp = armedHp;
			/* The brain compares this against the bar it recomputes, so parking
			it one above the target is what limits the next crossing to the one
			authored threshold. */
			boss->iLastEvaluatedHealthBar = armedBar;
			m_iValtanAuditionArmedHealthBar = request.iTargetHealthBar;
			currentHealthBar = armedBar;
			m_ValtanAuditionSequenceBySessionId.insert_or_assign(
				sessionId, request.iRequestSequence);
			return VALTAN_AUDITION_RESULT::ARMED;
		}

		if (VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR == request.eOperation)
		{
			std::string resetStatus;
			const std::uint32_t resetTick =
				0u == m_iServerTick ? 1u : m_iServerTick;
			if (!Reset_ValtanAuditionState(*boss, resetTick, resetStatus))
			{
				m_strStatus = std::move(resetStatus);
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}
			const std::uint32_t previousBar = request.iTargetHealthBar + 1u;
			const std::uint32_t targetHp =
				CValtanBrain::Resolve_HealthBarHp(
					*boss, request.iTargetHealthBar);
			if (previousBar > boss->iMaximumHealthBars || 0u == targetHp)
				return VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR;

			/* PLAY is one atomic Debug command. Unlike two UI clicks, there is no
			brain tick between priming the previous bar and crossing the target,
			so a normal pattern cannot steal the audition window. */
			boss->iCurrentHp = targetHp;
			boss->iLastEvaluatedHealthBar = previousBar;
			m_iValtanAuditionArmedHealthBar = 0u;
			currentHealthBar = request.iTargetHealthBar;
			m_ValtanAuditionSequenceBySessionId.insert_or_assign(
				sessionId, request.iRequestSequence);
			return VALTAN_AUDITION_RESULT::QUEUED;
		}

		if (m_iValtanAuditionArmedHealthBar != request.iTargetHealthBar)
			return VALTAN_AUDITION_RESULT::REJECTED_NOT_ARMED;
		const std::uint32_t targetHp =
			CValtanBrain::Resolve_HealthBarHp(*boss, request.iTargetHealthBar);
		if (0u == targetHp)
			return VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR;

		/* iLastEvaluatedHealthBar is deliberately left where ARM put it. The
		next CValtanBrain tick is what observes the crossing and queues the
		pattern, so the camera cue, the wall binding and the debris all run
		through the product path rather than being started from here. */
		boss->iCurrentHp = targetHp;
		m_iValtanAuditionArmedHealthBar = 0u;
		currentHealthBar = request.iTargetHealthBar;
		m_ValtanAuditionSequenceBySessionId.insert_or_assign(
			sessionId, request.iRequestSequence);
		return VALTAN_AUDITION_RESULT::QUEUED;
	};
	return evaluate();
#endif
}

void LostArk::Server::CGameRoom::Handle_ValtanAudition(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request)
{
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;

	std::uint32_t currentHealthBar = 0u;
	const LostArk::Shared::VALTAN_AUDITION_RESULT result =
		Evaluate_ValtanAudition(sessionId, request, currentHealthBar);
	if (!Send_ValtanAuditionResult(
		session, request, result, currentHealthBar))
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

bool LostArk::Server::CGameRoom::Send_EnterRejected(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason)
{
	using namespace LostArk::Shared;
	S2C_ENTER_REJECTED message{};
	message.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	message.eWorldId = m_eWorldId;
	message.eReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_ENTER_REJECTED, writer.Get_Buffer());
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
	message.strPlacementId = entity.strPlacementId;
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

bool LostArk::Server::CGameRoom::Send_WorldDestructionFullSync(
	const std::shared_ptr<CClientSession>& session)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId)
		return true;
	if (nullptr == session || !m_WorldDestructionRuntime.Is_Initialized())
		return false;

	S2C_WORLD_DESTRUCTION_FULL_SYNC message{};
	message.strCombatRuntimeRevision =
		m_WorldDestructionBootstrap.Get_CombatRuntimeRevision();
	message.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
	message.iEncounterEpoch = m_WorldDestructionRuntime.Get_EncounterEpoch();
	for (const WORLD_DESTRUCTION_GROUP_STATE& state :
		m_WorldDestructionRuntime.Get_GroupStates())
	{
		message.GroupStates.push_back(To_NetworkDestructionState(state));
	}
	CPacketWriter writer;
	return Write_Message(writer, message) && session->Send_Frame(
		PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC, writer.Get_Buffer());
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

bool LostArk::Server::CGameRoom::Send_ValtanAuditionResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
	const LostArk::Shared::VALTAN_AUDITION_RESULT result,
	const std::uint32_t currentHealthBar)
{
	using namespace LostArk::Shared;
	S2C_VALTAN_AUDITION_RESULT message{};
	message.iRequestSequence = request.iRequestSequence;
	message.eOperation = request.eOperation;
	message.iTargetHealthBar = request.iTargetHealthBar;
	message.eResult = result;
	message.iCurrentHealthBar = currentHealthBar;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_VALTAN_AUDITION_RESULT,
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

bool LostArk::Server::CGameRoom::Broadcast_WorldDestructionDelta(
	const std::vector<WORLD_DESTRUCTION_STATE_TRANSITION>& transitions,
	const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>& liveEvents,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (transitions.empty() && liveEvents.empty())
		return true;

	S2C_WORLD_DESTRUCTION_DELTA message{};
	message.strCombatRuntimeRevision =
		m_WorldDestructionBootstrap.Get_CombatRuntimeRevision();
	message.iServerTick = serverTick;
	message.iEncounterEpoch = m_WorldDestructionRuntime.Get_EncounterEpoch();
	for (const WORLD_DESTRUCTION_STATE_TRANSITION& transition : transitions)
	{
		WORLD_DESTRUCTION_GROUP_STATE state{};
		if (!m_WorldDestructionRuntime.Find_GroupState(
			transition.strGroupId, state))
		{
			return false;
		}
		message.ChangedStates.push_back(To_NetworkDestructionState(state));
	}
	std::sort(message.ChangedStates.begin(), message.ChangedStates.end(),
		[](const WORLD_DESTRUCTION_STATE_WIRE& left,
			const WORLD_DESTRUCTION_STATE_WIRE& right)
		{
			return left.strGroupId < right.strGroupId;
		});
	message.LiveEvents = liveEvents;

	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return false;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_DESTRUCTION_DELTA, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
	return true;
}

void LostArk::Server::CGameRoom::Broadcast_WorldSnapshot()
{
	using namespace LostArk::Shared;
	S2C_WORLD_SNAPSHOT message{};
	message.iServerTick = m_iServerTick;
	message.eWorldId = m_eWorldId;
	message.iEstherGauge = m_EstherSkillSystem.Get_Gauge();
	message.iEstherGaugeMaximum = m_EstherSkillSystem.Get_GaugeMaximum();
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
		snapshot.iCurrentIdentity = player.iCurrentIdentity;
		snapshot.iMaximumIdentity = player.iMaximumIdentity;
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

	const auto encodeStart = std::chrono::steady_clock::now();
	CPacketWriter writer;
	const bool encoded = Write_Message(writer, message);
	const std::uint64_t encodeMicroseconds = To_Microseconds(
		std::chrono::steady_clock::now() - encodeStart);
	{
		std::scoped_lock lock{ m_CommandMutex };
		++m_PerformanceMetrics.iSnapshotEncodeCount;
		m_PerformanceMetrics.iLastSnapshotEncodeMicroseconds =
			encodeMicroseconds;
		m_PerformanceMetrics.iMaximumSnapshotEncodeMicroseconds = (std::max)(
			m_PerformanceMetrics.iMaximumSnapshotEncodeMicroseconds,
			encodeMicroseconds);
		if (!encoded)
			++m_PerformanceMetrics.iSnapshotEncodeFailureCount;
	}
	if (!encoded)
		return;

	const auto enqueueBatchStart = std::chrono::steady_clock::now();
	std::uint64_t maximumSessionEnqueueMicroseconds = 0u;
	std::uint64_t recipientCount = 0u;
	std::uint64_t enqueueFailureCount = 0u;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr == session)
			continue;
		++recipientCount;
		const auto sessionEnqueueStart = std::chrono::steady_clock::now();
		const bool enqueued = session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_SNAPSHOT, writer.Get_Buffer());
		const std::uint64_t sessionEnqueueMicroseconds = To_Microseconds(
			std::chrono::steady_clock::now() - sessionEnqueueStart);
		maximumSessionEnqueueMicroseconds = (std::max)(
			maximumSessionEnqueueMicroseconds, sessionEnqueueMicroseconds);
		if (!enqueued)
		{
			++enqueueFailureCount;
			session->Request_Close();
		}
	}
	const std::uint64_t enqueueBatchMicroseconds = To_Microseconds(
		std::chrono::steady_clock::now() - enqueueBatchStart);
	{
		std::scoped_lock lock{ m_CommandMutex };
		++m_PerformanceMetrics.iSnapshotEnqueueBatchCount;
		m_PerformanceMetrics.iSnapshotRecipientCount += recipientCount;
		m_PerformanceMetrics.iSnapshotEnqueueFailureCount += enqueueFailureCount;
		m_PerformanceMetrics.iLastSnapshotEnqueueMicroseconds =
			enqueueBatchMicroseconds;
		m_PerformanceMetrics.iMaximumSnapshotEnqueueMicroseconds = (std::max)(
			m_PerformanceMetrics.iMaximumSnapshotEnqueueMicroseconds,
			enqueueBatchMicroseconds);
		m_PerformanceMetrics.iLastMaximumSessionEnqueueMicroseconds =
			maximumSessionEnqueueMicroseconds;
		m_PerformanceMetrics.iMaximumSessionEnqueueMicroseconds = (std::max)(
			m_PerformanceMetrics.iMaximumSessionEnqueueMicroseconds,
			maximumSessionEnqueueMicroseconds);
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

bool LostArk::Server::CGameRoom::Is_PlayerAdmissionFull() const
{
	return nullptr == Find_AvailablePlayerSpawn();
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
	staged.fSpawnPositionX = staged.fPositionX;
	staged.fSpawnPositionY = staged.fPositionY;
	staged.fSpawnPositionZ = staged.fPositionZ;
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

bool LostArk::Server::CGameRoom::Reset_ReplayableArenaWhenEmpty()
{
	using LostArk::Shared::WORLD_ID;
	if ((WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId &&
		WORLD_ID::VALTAN_ARENA != m_eWorldId) || !m_Players.empty())
		return true;

	std::string resetStatus;
	if (!m_ServerTriggerSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		m_isReady = false;
		return false;
	}
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
	m_strStatus = "Replayable arena reset after the room became empty";
	return true;
}

bool LostArk::Server::CGameRoom::Reset_ValtanArenaWhenEmpty()
{
	using LostArk::Shared::WORLD_ID;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId || !m_Players.empty())
		return true;

	std::string resetStatus;
	const std::uint32_t resetTick = 0u == m_iServerTick ? 1u : m_iServerTick;
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		m_isReady = false;
		return false;
	}
	if (!m_WorldDestructionRuntime.Reset(resetStatus, resetTick))
	{
		m_strStatus = std::move(resetStatus);
		m_isReady = false;
		return false;
	}
	m_ServerCollisionSystem.Reset_RuntimeStates();
	m_ServerNavigation.Reset_RuntimeBlockers();
	m_iNextWorldDestructionEventSequence = 1u;
	if (!Initialize_WorldEntities())
	{
		m_isReady = false;
		return false;
	}
	m_TickDamageEvents.clear();
	/* The encounter is fresh, so a bar armed by the previous occupants no
	longer describes any live boss. Leave already dropped their sequences. */
	m_iValtanAuditionArmedHealthBar = 0u;
	// The next party charges its Esther gauge from zero; any live summon was
	// already discarded with the entity rebuild above.
	m_EstherSkillSystem.Reset();
	m_strStatus = "Valtan arena reset after the room became empty";
	return true;
}

bool LostArk::Server::CGameRoom::Apply_WorldDestructionStageEntry(
	const SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick)
{
	if (!m_WorldDestructionRuntime.Is_Initialized() ||
		0u == boss.iNetEntityId || 0u == boss.iPatternSequence ||
		boss.strPatternId.empty() || boss.strPatternStageId.empty() ||
		boss.strActionId.empty())
	{
		return true;
	}

	WORLD_DESTRUCTION_ACTION_TUPLE action{};
	action.strPatternId = boss.strPatternId;
	action.strStageId = boss.strPatternStageId;
	action.strActionId = boss.strActionId;
	action.iStageIndex = boss.iPatternStageIndex;
	WORLD_DESTRUCTION_TRANSACTION transaction{};
	std::string status;
	const WORLD_DESTRUCTION_PREPARE_RESULT result =
		m_WorldDestructionRuntime.Prepare_StageTrigger(
			action, boss.iNetEntityId, boss.iPatternSequence, serverTick,
			transaction, status);
	if (WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH == result ||
		WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST == result ||
		WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE == result)
	{
		return true;
	}
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE> liveEvents;
	if (WORLD_DESTRUCTION_PREPARE_RESULT::READY == result &&
		!Build_WorldDestructionLiveEvents(
			transaction, boss, liveEvents, status))
	{
		m_strStatus = std::move(status);
		return false;
	}
	if (WORLD_DESTRUCTION_PREPARE_RESULT::READY != result ||
		!Commit_WorldDestructionTransaction(
			transaction, liveEvents, serverTick, status))
	{
		m_strStatus = std::move(status);
		return false;
	}
	if (!liveEvents.empty())
	{
		const std::uint64_t lastSequence = liveEvents.back().iEventSequence;
		m_iNextWorldDestructionEventSequence =
			(std::numeric_limits<std::uint64_t>::max)() == lastSequence ?
			0u : lastSequence + 1u;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Apply_WorldDestructionImpact(
	SERVER_WORLD_ENTITY& boss,
	const std::string& receiverPlacementId,
	const std::uint32_t serverTick,
	bool& outTriggered)
{
	outTriggered = false;
	if (!m_WorldDestructionRuntime.Is_Initialized() ||
		receiverPlacementId.empty() || 0u == boss.iNetEntityId ||
		0u == boss.iPatternSequence || boss.strPatternId.empty() ||
		boss.strPatternStageId.empty() || boss.strActionId.empty())
	{
		return true;
	}
	WORLD_DESTRUCTION_ACTION_TUPLE action{};
	action.strPatternId = boss.strPatternId;
	action.strStageId = boss.strPatternStageId;
	action.strActionId = boss.strActionId;
	action.iStageIndex = boss.iPatternStageIndex;
	WORLD_DESTRUCTION_TRANSACTION transaction{};
	std::string status;
	const WORLD_DESTRUCTION_PREPARE_RESULT result =
		m_WorldDestructionRuntime.Prepare_ImpactTrigger(
			action, receiverPlacementId, boss.iNetEntityId,
			boss.iPatternSequence, serverTick, transaction, status);
	if (WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH == result ||
		WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST == result ||
		WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE == result)
	{
		return true;
	}
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE> liveEvents;
	if (WORLD_DESTRUCTION_PREPARE_RESULT::READY != result ||
		!Build_WorldDestructionLiveEvents(
			transaction, boss, liveEvents, status) ||
		!Commit_WorldDestructionTransaction(
			transaction, liveEvents, serverTick, status))
	{
		m_strStatus = std::move(status);
		return false;
	}
	if (!liveEvents.empty())
	{
		const std::uint64_t lastSequence = liveEvents.back().iEventSequence;
		m_iNextWorldDestructionEventSequence =
			(std::numeric_limits<std::uint64_t>::max)() == lastSequence ?
			0u : lastSequence + 1u;
	}
	outTriggered = true;
	return true;
}

bool LostArk::Server::CGameRoom::Commit_WorldDestructionTransaction(
	const WORLD_DESTRUCTION_TRANSACTION& transaction,
	const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>& liveEvents,
	const std::uint32_t serverTick,
	std::string& status)
{
	std::vector<SERVER_COLLISION_STATE_CHANGE> collisionChanges;
	std::vector<SERVER_NAVIGATION_CONDITION_CHANGE> navigationChanges;
	for (const WORLD_DESTRUCTION_STATE_TRANSITION& transition :
		transaction.Transitions)
	{
		if (!transition.strCollisionStateId.empty())
		{
			SERVER_COLLISION_STATE_CHANGE change{};
			change.strPlacementId = transition.strCollisionStateId;
			change.bPlayerBlocking =
				WORLD_DESTRUCTION_STATE::INTACT == transition.eNextState ||
				WORLD_DESTRUCTION_STATE::BREAKING == transition.eNextState;
			change.bImpactReceiverEnabled =
				WORLD_DESTRUCTION_STATE::INTACT == transition.eNextState;
			collisionChanges.push_back(std::move(change));
		}
		if (transition.bApplyPersistentMutation &&
			!transition.strNavigationStateId.empty())
		{
			SERVER_NAVIGATION_CONDITION_CHANGE change{};
			change.strConditionId = transition.strNavigationStateId;
			change.bValue =
				WORLD_DESTRUCTION_STATE::FRACTURED == transition.eNextState ||
				WORLD_DESTRUCTION_STATE::DESPAWNED == transition.eNextState;
			navigationChanges.push_back(std::move(change));
		}
	}

	SERVER_COLLISION_STATE_STAGE collisionStage{};
	SERVER_NAVIGATION_CONDITION_STAGE navigationStage{};
	if (!m_ServerCollisionSystem.Prepare_StateChanges(
			collisionChanges, collisionStage, status) ||
		!m_ServerNavigation.Prepare_ConditionChanges(
			navigationChanges, navigationStage, status) ||
		!m_WorldDestructionRuntime.Commit(transaction, status))
	{
		return false;
	}
	m_ServerCollisionSystem.Commit_StateChanges(std::move(collisionStage));
	m_ServerNavigation.Commit_ConditionChanges(std::move(navigationStage));
	if (!navigationChanges.empty())
		Invalidate_DynamicNavigationPaths();
	if (!Broadcast_WorldDestructionDelta(
		transaction.Transitions, liveEvents, serverTick))
	{
		status = "World destruction delta broadcast failed after commit";
		return false;
	}
	return true;
}

void LostArk::Server::CGameRoom::Invalidate_DynamicNavigationPaths()
{
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;
		if (!player.hasMoveGoal)
			continue;
		if (!m_ServerNavigation.Find_Path(
			player.fPositionX, player.fPositionZ,
			player.fMoveGoalX, player.fMoveGoalZ, player.MovePath))
		{
			player.hasMoveGoal = false;
			continue;
		}
		const SERVER_NAV_POINT& goal = player.MovePath.back();
		player.fMoveGoalX = goal.x;
		player.fMoveGoalZ = goal.z;
	}
	for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		entity.MovePath.clear();
		entity.iMovePathIndex = 0u;
		entity.iNextPathReplanTick = m_iServerTick;
	}
}

bool LostArk::Server::CGameRoom::Build_WorldDestructionLiveEvents(
	const WORLD_DESTRUCTION_TRANSACTION& transaction,
	const SERVER_WORLD_ENTITY& boss,
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>& liveEvents,
	std::string& status) const
{
	using namespace LostArk::Shared;
	liveEvents.clear();
	if (transaction.iEncounterEpoch !=
			m_WorldDestructionRuntime.Get_EncounterEpoch() ||
		0u == transaction.iRequestTick ||
		transaction.BindingApplications.size() != transaction.Transitions.size() ||
		0u == boss.iNetEntityId || !std::isfinite(boss.fPositionX) ||
		!std::isfinite(boss.fPositionY) || !std::isfinite(boss.fPositionZ) ||
		!std::isfinite(boss.fYawDegrees))
	{
		status = "World destruction live-event source is invalid";
		return false;
	}

	std::size_t eventCount = 0u;
	for (const WORLD_DESTRUCTION_STATE_TRANSITION& transition :
		transaction.Transitions)
	{
		if (WORLD_DESTRUCTION_STATE::INTACT == transition.ePreviousState &&
			WORLD_DESTRUCTION_STATE::BREAKING == transition.eNextState)
		{
			++eventCount;
		}
	}
	if (eventCount > MAX_WORLD_DESTRUCTION_EVENTS ||
		(0u < eventCount &&
			(0u == m_iNextWorldDestructionEventSequence ||
			 static_cast<std::uint64_t>(eventCount - 1u) >
				(std::numeric_limits<std::uint64_t>::max)() -
				m_iNextWorldDestructionEventSequence)))
	{
		status = "World destruction live-event sequence is exhausted";
		return false;
	}

	const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
	const float forwardX = std::sin(yawRadians);
	const float forwardZ = std::cos(yawRadians);
	if (!std::isfinite(forwardX) || !std::isfinite(forwardZ))
	{
		status = "World destruction live-event direction is invalid";
		return false;
	}

	liveEvents.reserve(eventCount);
	std::uint64_t sequence = m_iNextWorldDestructionEventSequence;
	for (std::size_t index = 0u; index < transaction.Transitions.size(); ++index)
	{
		const WORLD_DESTRUCTION_STATE_TRANSITION& transition =
			transaction.Transitions[index];
		const WORLD_DESTRUCTION_BINDING_APPLICATION& application =
			transaction.BindingApplications[index];
		if (application.strMutationId != transition.strMutationId ||
			application.iSourceNetEntityId != boss.iNetEntityId ||
			application.iPatternSequence != boss.iPatternSequence)
		{
			liveEvents.clear();
			status = "World destruction live-event transaction is inconsistent";
			return false;
		}
		if (WORLD_DESTRUCTION_STATE::INTACT != transition.ePreviousState ||
			WORLD_DESTRUCTION_STATE::BREAKING != transition.eNextState)
		{
			continue;
		}

		WORLD_DESTRUCTION_EVENT_WIRE event{};
		event.iEventSequence = sequence++;
		event.strGroupId = transition.strGroupId;
		event.strMutationId = transition.strMutationId;
		event.strBindingId = application.strBindingId;
		event.iPatternSequence = application.iPatternSequence;
		event.iSourceNetEntityId = application.iSourceNetEntityId;
		event.iServerTick = transaction.iRequestTick;
		event.fImpactOriginX = boss.fPositionX;
		event.fImpactOriginY = boss.fPositionY;
		event.fImpactOriginZ = boss.fPositionZ;
		event.fImpactDirectionX = forwardX;
		event.fImpactDirectionY = 0.f;
		event.fImpactDirectionZ = forwardZ;
		event.iRandomSeed = Hash_DestructionEventIdentity(
			transaction.iEncounterEpoch, event.iEventSequence,
			transition, application, transaction.iRequestTick);
		liveEvents.push_back(std::move(event));
	}
	status = liveEvents.empty() ?
		"World destruction transition has no one-shot live event" :
		"World destruction one-shot live events staged";
	return true;
}

bool LostArk::Server::CGameRoom::Commit_DueWorldDestruction(
	const std::uint32_t serverTick)
{
	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA != m_eWorldId)
		return true;
	WORLD_DESTRUCTION_TRANSACTION transaction{};
	std::string status;
	const WORLD_DESTRUCTION_PREPARE_RESULT result =
		m_WorldDestructionRuntime.Prepare_DueStateCommits(
			serverTick, transaction, status);
	if (WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE == result)
		return true;
	if (WORLD_DESTRUCTION_PREPARE_RESULT::READY != result ||
		!Commit_WorldDestructionTransaction(
			transaction, {}, serverTick, status))
	{
		m_strStatus = std::move(status);
		return false;
	}
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

float LostArk::Server::CGameRoom::Resolve_StanceMoveSpeedScale(
	const SERVER_PLAYER& player) const
{
	const PLAYER_RUNTIME_PROFILE* profile =
		m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile ||
		!CPlayerSkillSystem::Is_HoldingGaugedStance(player, *profile))
	{
		return 1.f;
	}
	return profile->fDefenseStanceMoveSpeedScale;
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
			&m_ServerCollisionSystem,
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
			const float desiredYaw =
				std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
			const float yawDifference =
				Wrap_Degrees(desiredYaw - player.fYawDegrees);
			const float maxYawStep =
				PLAYER_TURN_DEGREES_PER_SECOND * fixedDeltaSeconds;
			if (distance <= DIRECT_BEARING_DISTANCE ||
				std::abs(yawDifference) <= maxYawStep)
			{
				player.fYawDegrees = desiredYaw;
			}
			else
			{
				player.fYawDegrees = Wrap_Degrees(player.fYawDegrees +
					(yawDifference > 0.f ? maxYawStep : -maxYawStep));
			}
			const float moveDistance = (std::min)(
				player.fMoveSpeed * Resolve_StanceMoveSpeedScale(player) *
					fixedDeltaSeconds,
				distance);
			const float moveRatio = moveDistance / distance;
			float stepX = deltaX * moveRatio;
			float stepZ = deltaZ * moveRatio;
			if (player.fYawDegrees != desiredYaw)
			{
				const float headingRadians =
					player.fYawDegrees / RADIANS_TO_DEGREES;
				const float headingStepX =
					std::sin(headingRadians) * moveDistance;
				const float headingStepZ =
					std::cos(headingRadians) * moveDistance;
				SERVER_NAV_POINT walkable{};
				if (!m_ServerNavigation.Is_Loaded() ||
					m_ServerNavigation.Sample_Position(
						player.fPositionX + headingStepX,
						player.fPositionZ + headingStepZ,
						walkable))
				{
					stepX = headingStepX;
					stepZ = headingStepZ;
				}
				else
				{
					player.fYawDegrees = desiredYaw;
				}
			}
			proposedX = player.fPositionX + stepX;
			proposedY = player.fPositionY +
				(targetY - player.fPositionY) * moveRatio;
			proposedZ = player.fPositionZ + stepZ;
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
		if (entity.isEstherSummon)
		{
			/* Room-owned appear -> strike -> leave timeline. Stage exits are
			duration-driven; the leave stage additionally rises straight up so
			the summon departs skyward before the sweep below despawns it. */
			entity.fActionElapsedSeconds += fixedDeltaSeconds;
			const float elapsedMs = entity.fActionElapsedSeconds * 1000.f;
			if (ESTHER_ACTION_APPEAR == entity.strActionId &&
				elapsedMs >= static_cast<float>(ESTHER_APPEAR_MS))
			{
				entity.eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
				entity.strActionId = ESTHER_ACTION_STRIKE;
				entity.fActionElapsedSeconds = 0.f;
				entity.iActionStartTick = updateTick;
			}
			else if (ESTHER_ACTION_STRIKE == entity.strActionId &&
				elapsedMs >= static_cast<float>(ESTHER_STRIKE_MS))
			{
				entity.eAction = SERVER_ENTITY_ACTION::IDLE;
				entity.strActionId = ESTHER_ACTION_LEAVE;
				entity.fActionElapsedSeconds = 0.f;
				entity.iActionStartTick = updateTick;
			}
			else if (ESTHER_ACTION_LEAVE == entity.strActionId)
			{
				entity.fPositionY +=
					ESTHER_LEAVE_RISE_PER_SECOND * fixedDeltaSeconds;
			}
			continue;
		}
		if (entity.eKind == WORLD_BOOTSTRAP_KIND::BOSS &&
			m_ServerNavigation.Is_Loaded())
		{
			const std::uint32_t previousPatternSequence =
				entity.iPatternSequence;
			const std::uint32_t previousStageIndex =
				entity.iPatternStageIndex;
			const std::uint32_t previousActionStartTick =
				entity.iActionStartTick;
			const std::string previousPatternId = entity.strPatternId;
			const std::string previousStageId = entity.strPatternStageId;
			const std::string previousActionId = entity.strActionId;
			m_ValtanBrain.Update(
				entity,
				m_Players,
				m_GameplayCatalog,
				m_ServerNavigation,
				fixedDeltaSeconds,
				updateTick,
				m_TickDamageEvents);
			const bool stageChanged =
				previousPatternSequence != entity.iPatternSequence ||
				previousStageIndex != entity.iPatternStageIndex ||
				previousActionStartTick != entity.iActionStartTick ||
				previousPatternId != entity.strPatternId ||
				previousStageId != entity.strPatternStageId ||
				previousActionId != entity.strActionId;
			if (stageChanged && !Apply_WorldDestructionStageEntry(
				entity, updateTick))
			{
				m_isReady = false;
				return;
			}
			float proposedX = 0.f;
			float proposedZ = 0.f;
			if (m_ValtanBrain.Try_BuildImpactMotion(
				entity, fixedDeltaSeconds, proposedX, proposedZ))
			{
				SERVER_BOSS_RECEIVER_HIT hit{};
				if (m_ServerCollisionSystem.Sweep_BossCircleAgainstReceivers(
					entity.fPositionX, entity.fPositionY, entity.fPositionZ,
					proposedX, entity.fPositionY, proposedZ,
					entity.fCollisionRadius, hit))
				{
					const float deltaX = proposedX - entity.fPositionX;
					const float deltaZ = proposedZ - entity.fPositionZ;
					const float distance = std::sqrt(
						deltaX * deltaX + deltaZ * deltaZ);
					const float marginRatio = distance > 0.000001f ?
						0.001f / distance : 0.f;
					const float safeRatio = (std::max)(
						0.f, hit.fHitRatio - marginRatio);
					entity.fPositionX += deltaX * safeRatio;
					entity.fPositionZ += deltaZ * safeRatio;
					bool triggered = false;
					if (!Apply_WorldDestructionImpact(
						entity, hit.strReceiverPlacementId, updateTick, triggered))
					{
						m_isReady = false;
						return;
					}
					if (triggered)
					{
						if (!m_ValtanBrain.Complete_ImpactStage(
							entity, m_GameplayCatalog, updateTick) ||
							!Apply_WorldDestructionStageEntry(entity, updateTick))
						{
							m_strStatus =
								"Valtan impact stage transition failed";
							m_isReady = false;
							return;
						}
					}
				}
				else
				{
					entity.fPositionX = proposedX;
					entity.fPositionZ = proposedZ;
				}
			}
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
			(WORLD_BOOTSTRAP_KIND::MONSTER == iter->eKind &&
				SERVER_ENTITY_ACTION::DEAD == iter->eAction &&
				iter->fActionElapsedSeconds * 1000.f >=
					static_cast<float>(iter->iDeadDespawnMs)) ||
			(iter->isEstherSummon &&
				ESTHER_ACTION_LEAVE == iter->strActionId &&
				iter->fActionElapsedSeconds * 1000.f >=
					static_cast<float>(ESTHER_LEAVE_MS));
		if (!shouldDespawn)
		{
			++iter;
			continue;
		}
		Broadcast_WorldEntityDespawned(iter->iNetEntityId);
		iter = m_WorldEntities.erase(iter);
	}
}
