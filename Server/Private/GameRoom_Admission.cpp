#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

void LostArk::Server::CGameRoom::Handle_Register(
	const std::shared_ptr<CClientSession>& session)
{
	if (nullptr == session || session->Get_SessionId() == INVALID_SESSION_ID)
		return;
	m_Sessions.insert_or_assign(session->Get_SessionId(), session);
}

bool LostArk::Server::CGameRoom::Stage_PlayerEntry(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
	const std::span<const STAGED_PLAYER_ENTRY> precedingEntries,
	STAGED_PLAYER_ENTRY& staged,
	LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outReason, std::string& status,
	const std::string& spawnPlacementOverrideId,
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& carriedInventory)
{
	using namespace LostArk::Shared;
	outReason = SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED;
	status.clear();
	const auto reject = [&outReason, &status](
		const SESSION_DIAGNOSTIC_REASON reason, const char* detail)
	{
		outReason = reason;
		status = detail;
		return false;
	};
	const std::size_t offset = precedingEntries.size();
	if (!m_isReady || nullptr == session || session->Is_Closing() ||
		INVALID_SESSION_ID == session->Get_SessionId() ||
		!Is_Valid_EnterWorld(enterWorld) || enterWorld.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.contains(session->Get_SessionId()) ||
		m_Players.size() + offset >= MAX_WORLD_SNAPSHOT_PLAYERS ||
		INVALID_PLAYER_ID == m_iNextPlayerId ||
		INVALID_NET_ENTITY_ID == m_iNextNetEntityId ||
		offset > (std::numeric_limits<PLAYER_ID>::max)() - m_iNextPlayerId ||
		offset > (std::numeric_limits<NET_ENTITY_ID>::max)() - m_iNextNetEntityId)
	{
		return reject(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
			"player entry room/session/identity validation failed");
	}
	const WORLD_BOOTSTRAP_PLACEMENT* spawn = nullptr;
	if (!spawnPlacementOverrideId.empty())
	{
		/* Not restricted to PLAYER_SPAWN kind or exclusivity -- an override names
		one specific placement (e.g. a guide NPC) directly, and several returning
		players landing at the same NPC concurrently is fine (unlike normal
		PLAYER_SPAWN slots, which are one-player-at-a-time). */
		spawn = Find_Placement(spawnPlacementOverrideId);
		if (nullptr == spawn)
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
				"spawn placement override id does not exist in this world's bootstrap");
	}
	else
	{
		for (const auto& candidate : m_WorldBootstrap.Get_Placements())
		{
			if (!candidate.isEnabled || WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != candidate.eKind)
				continue;
			const bool occupied = std::any_of(m_Players.begin(), m_Players.end(),
				[&candidate](const auto& value)
				{ return value.second.strSpawnPlacementId == candidate.strPlacementId; });
			const bool reserved = std::any_of(precedingEntries.begin(), precedingEntries.end(),
				[&candidate](const auto& value)
				{ return value.Player.strSpawnPlacementId == candidate.strPlacementId; });
			if (!occupied && !reserved) { spawn = &candidate; break; }
		}
		if (nullptr == spawn)
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL,
				"target room has fewer free player spawns than the transfer batch");
	}
	STAGED_PLAYER_ENTRY candidate{};
	candidate.pSession = session;
	SERVER_PLAYER& player = candidate.Player;
	player.iSessionId = session->Get_SessionId();
	player.iPlayerId = m_iNextPlayerId + static_cast<PLAYER_ID>(offset);
	player.iNetEntityId = m_iNextNetEntityId + static_cast<NET_ENTITY_ID>(offset);
	player.eCharacterClass = enterWorld.eCharacterClass;
	player.strNickName = enterWorld.strNickName;
	player.strSpawnPlacementId = spawn->strPlacementId;
	player.fPositionY = spawn->fPositionY;
	if (!spawnPlacementOverrideId.empty())
	{
		/* An override names an NPC's own placement, not an authored player-standing
		spot -- its exact point is often flush against a wall or counter (the NPC's
		back), so landing there directly can navigation-project to the wrong side
		of that geometry. Stand where a player who walked up to talk to it would:
		NPC_APPROACH_OFFSET_M out along its own forward direction, facing back
		toward it (matches this codebase's yaw convention, forward = (sin, cos),
		e.g. MonsterBrain.cpp's own movement step). */
		constexpr float NPC_APPROACH_OFFSET_M = 2.5f;
		const float yawRadians = spawn->fYawDegrees * DEGREES_TO_RADIANS;
		player.fPositionX = spawn->fPositionX + std::sin(yawRadians) * NPC_APPROACH_OFFSET_M;
		player.fPositionZ = spawn->fPositionZ + std::cos(yawRadians) * NPC_APPROACH_OFFSET_M;
		player.fYawDegrees = std::fmod(spawn->fYawDegrees + 180.f, 360.f);
	}
	else
	{
		player.fPositionX = spawn->fPositionX;
		player.fPositionZ = spawn->fPositionZ;
		player.fYawDegrees = spawn->fYawDegrees;
	}
	const PLAYER_RUNTIME_PROFILE* profile = m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile)
		return reject(SESSION_DIAGNOSTIC_REASON::SERVER_PROFILE_MISSING,
			"selected character class has no runtime profile");
	player.eStance = profile->eDefaultStance;
	player.iCurrentHp = player.iMaximumHp = profile->iMaximumHp;
	player.iCurrentResource = player.iMaximumResource = profile->iMaximumResource;
	player.fMoveSpeed = profile->fMoveSpeed;
	player.iCurrentIdentity = player.iMaximumIdentity = profile->iMaximumIdentity;
	player.iCurrentMadness = 0u;
	player.iMaximumMadness = SERVER_PLAYER::MADNESS_GAUGE_MAXIMUM;
	player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
	player.Clear_KoukuInteractionState();
	player.isCombatReady = WORLD_ID::VALTAN_ARENA != m_eWorldId;
	if (m_ServerNavigation.Is_Loaded())
	{
		SERVER_NAV_POINT projected{};
		if (!m_ServerNavigation.Project_Point(player.fPositionX, player.fPositionZ, projected))
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_NAVIGATION_FAILED,
				"player spawn could not project onto Server navigation");
		player.fPositionX = projected.x;
		player.fPositionY = projected.y;
		player.fPositionZ = projected.z;
	}
	if (!carriedInventory.empty())
	{
		// A world transfer carrying the departing player's own live inventory
		// (e.g. Handle_ReturnToBern) replaces the default fresh-entry grant
		// entirely -- Valtan clear rewards must survive the trip back to Bern.
		player.Inventory = carriedInventory;
	}
	else
	{
		for (const char* potionId : { "POTION_HP_SMALL", "POTION_HP_MEDIUM", "POTION_HP_LARGE" })
		{
			const SERVER_ITEM_DEFINITION* definition = m_ItemCatalog.Find_Item(potionId);
			if (nullptr == definition) continue;
			INVENTORY_ITEM_SNAPSHOT item{};
			item.strItemId = potionId;
			item.iQuantity = (std::min)(500u, definition->iMaxStack);
			player.Inventory.push_back(std::move(item));
		}
	}
	staged = std::move(candidate);
	return true;
}

bool LostArk::Server::CGameRoom::Build_PlayerEntryFrames(
	STAGED_PLAYER_ENTRY& entry, const std::span<const STAGED_PLAYER_ENTRY> batch,
	std::string& status)
{
	using namespace LostArk::Shared;
	std::vector<PACKET_FRAME> frames;
	const auto append = [&frames, &status](const PACKET_TYPE type, const auto& message)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
		{
			status = "initial entry payload failed validation, packet=" +
				std::to_string(static_cast<std::uint16_t>(type));
			return false;
		}
		frames.push_back({ type, writer.Get_Buffer() });
		return true;
	};
	S2C_ENTER_ACCEPTED accepted{};
	accepted.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	accepted.eWorldId = m_eWorldId;
	accepted.iPlayerId = entry.Player.iPlayerId;
	accepted.iNetEntityId = entry.Player.iNetEntityId;
	accepted.ActiveGameplayRevision = m_GameplayCatalog.Get_ActiveRevision();
	if (!Build_RequiredPinnedGameplayRevisions(accepted.RequiredPinnedGameplayRevisions))
	{
		status = "initial entry pinned gameplay revisions failed validation";
		return false;
	}
	if (!append(PACKET_TYPE::S2C_ENTER_ACCEPTED, accepted)) return false;
	S2C_INVENTORY_SNAPSHOT inventory{};
	inventory.Items = entry.Player.Inventory;
	if (!append(PACKET_TYPE::S2C_INVENTORY_SNAPSHOT, inventory)) return false;
	if (WORLD_ID::VALTAN_ARENA == m_eWorldId)
	{
		if (!m_WorldDestructionRuntime.Is_Initialized())
		{
			status = "initial world destruction runtime is not initialized";
			return false;
		}
		S2C_WORLD_DESTRUCTION_FULL_SYNC fullSync{};
		fullSync.strCombatRuntimeRevision = m_WorldDestructionBootstrap.Get_CombatRuntimeRevision();
		fullSync.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
		fullSync.iEncounterEpoch = m_WorldDestructionRuntime.Get_EncounterEpoch();
		for (const auto& state : m_WorldDestructionRuntime.Get_GroupStates())
			fullSync.GroupStates.push_back(To_NetworkDestructionState(state));
		fullSync.Diagnostics = Build_WorldDestructionDiagnostics();
		if (!append(PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC, fullSync)) return false;
	}
	if (m_EncounterPropRuntime.Is_Initialized())
	{
		S2C_ENCOUNTER_PROP_SYNC props{};
		props.strPropSetId = m_EncounterPropRuntime.Get_PropSetId();
		props.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
		props.iEncounterEpoch = m_EncounterPropRuntime.Get_EncounterEpoch();
		for (const auto& slot : m_EncounterPropRuntime.Get_SlotStates())
		{
			ENCOUNTER_PROP_SLOT_WIRE wire{};
			wire.strSlotId = slot.strSlotId;
			wire.eState = slot.eState;
			wire.iStateVersion = slot.iStateVersion;
			wire.iStateStartTick = slot.iStateStartTick;
			wire.iOccurrenceSequence = slot.iOccurrenceSequence;
			props.Slots.push_back(std::move(wire));
		}
		if (!append(PACKET_TYPE::S2C_ENCOUNTER_PROP_SYNC, props)) return false;
	}
	std::unordered_set<NET_ENTITY_ID> admittedWorldEntityIds;
	for (const bool dependentPass : { false, true })
	{
		for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
		{
			const bool isDependent = INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId;
			if (isDependent != dependentPass)
				continue;
			if (!admittedWorldEntityIds.insert(entity.iNetEntityId).second)
			{
				status = "Initial world entity ID is duplicated";
				return false;
			}
			if (isDependent)
			{
				const auto owner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
					[&entity](const SERVER_WORLD_ENTITY& candidate)
					{ return candidate.iNetEntityId == entity.iOwnerBossNetEntityId; });
				if (!admittedWorldEntityIds.contains(entity.iOwnerBossNetEntityId) ||
					owner == m_WorldEntities.end() || WORLD_BOOTSTRAP_KIND::BOSS != owner->eKind ||
					INVALID_NET_ENTITY_ID != owner->iOwnerBossNetEntityId ||
					owner->strEncounterId != entity.strEncounterId ||
					owner->PinnedDefinitionRevision !=
						entity.PinnedDefinitionRevision)
				{
					status = "Initial dependent boss has no preceding primary owner";
					return false;
				}
			}
			std::vector<std::uint8_t> payload;
			if (!Build_WorldEntitySpawnedPayload(entity, payload))
			{
				status = "World entity spawn payload preflight failed: " + entity.strPlacementId;
				return false;
			}
			frames.push_back({ PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED, std::move(payload) });
		}
	}
#ifdef _DEBUG
	S2C_KOUKUSAYDON_BUNDLE_STATE bundleState;
	if (Build_KoukuBundleState(bundleState))
	{
		if (!append(PACKET_TYPE::S2C_KOUKUSAYDON_BUNDLE_STATE, bundleState)) return false;
		for (auto play : m_KoukuSaydonPatternAudition.WorldPlays)
		{
			if (play.iDurationMs && Has_ReachedServerTick(m_iServerTick, Add_ServerTicksSkippingReservedZero(play.iStartTick, CKoukuSaydonLogicRuntime::Ticks_FromMs(play.iDurationMs)))) continue;
			play.iServerTick = m_iServerTick;
			if (!append(PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, play)) return false;
		}
	}
#endif
	std::vector<S2C_COMBAT_OBJECT_SPAWNED> combatObjects;
	m_CombatObjectRuntime.Build_LiveSpawnMessages(0u == m_iServerTick ? 1u : m_iServerTick, combatObjects);
	for (const auto& object : combatObjects)
		if (!append(PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED, object)) return false;
	const auto appendPlayer = [&append](const SERVER_PLAYER& player)
	{
		S2C_PLAYER_SPAWNED message{};
		message.iPlayerId = player.iPlayerId;
		message.iNetEntityId = player.iNetEntityId;
		message.eCharacterClass = player.eCharacterClass;
		message.strNickName = player.strNickName;
		message.fPositionX = player.fPositionX;
		message.fPositionY = player.fPositionY;
		message.fPositionZ = player.fPositionZ;
		message.fYawDegrees = player.fYawDegrees;
		return append(PACKET_TYPE::S2C_PLAYER_SPAWNED, message);
	};
	for (const auto& [id, player] : m_Players)
	{
		(void)id;
		if (!appendPlayer(player)) return false;
	}
	for (const auto& staged : batch)
		if (!appendPlayer(staged.Player)) return false;
	entry.Frames = std::move(frames);
	return true;
}

void LostArk::Server::CGameRoom::Commit_PlayerEntry(const STAGED_PLAYER_ENTRY& entry)
{
	const SERVER_PLAYER& player = entry.Player;
	m_Sessions.insert_or_assign(player.iSessionId, entry.pSession);
	m_Players.emplace(player.iPlayerId, player);
	m_PlayerIdBySessionId.emplace(player.iSessionId, player.iPlayerId);
	m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
	++m_iNextPlayerId;
	++m_iNextNetEntityId;
	entry.pSession->Bind_PlayerId(player.iPlayerId);
}

bool LostArk::Server::CGameRoom::Join(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
	const std::string& spawnPlacementOverrideId,
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& carriedInventory)
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
		{
			session->Request_Close(
				SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
				WSAEINVAL,
				"ENTER_WORLD failed room/session/id validation");
		}
		return false;
	}
	if (Is_PlayerAdmissionFull())
	{
		const std::size_t enabledPlayerSpawnCount =
			static_cast<std::size_t>(std::count_if(
				m_WorldBootstrap.Get_Placements().begin(),
				m_WorldBootstrap.Get_Placements().end(),
				[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
				{
					return placement.isEnabled &&
						WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind;
				}));
		const std::string roomFullCounts =
			"activePlayers=" + std::to_string(m_Players.size()) +
			", registeredSessionsIncludingCandidate=" +
			std::to_string(m_Sessions.size()) +
			", candidateSessionId=" + std::to_string(sessionId) +
			", candidateRegistered=" +
			(m_Sessions.contains(sessionId) ? "true" : "false") +
			", enabledPlayerSpawns=" +
			std::to_string(enabledPlayerSpawnCount);
		/* Leave room for the terminal-action prefix so the copied close context
		   remains bounded to roughly one KiB. */
		constexpr std::size_t MAX_ROOM_FULL_CONTEXT_BYTES = 960u;
		const std::uint64_t observedUnixMilliseconds =
			Current_UnixMilliseconds();
		std::string roomFullContext = roomFullCounts + ", activeRoster=[";
		bool isFirstRosterEntry = true;
		for (const auto& [playerId, player] : m_Players)
		{
			const std::shared_ptr<CClientSession> incumbent =
				Find_Session(player.iSessionId);
			std::string peer = "unavailable";
			std::uint64_t lastInboundUnixMilliseconds = 0u;
			if (nullptr != incumbent)
			{
				const CLIENT_SESSION_PEER_ENDPOINT& endpoint =
					incumbent->Get_PeerEndpoint();
				peer = endpoint.strAddress + ':' +
					std::to_string(endpoint.iPort);
				lastInboundUnixMilliseconds =
					incumbent->Get_LastInboundUnixMilliseconds();
			}
			const std::uint64_t lastInboundAgeMilliseconds =
				0u != lastInboundUnixMilliseconds &&
				observedUnixMilliseconds >= lastInboundUnixMilliseconds ?
				observedUnixMilliseconds - lastInboundUnixMilliseconds : 0u;
			const std::string rosterEntry =
				(isFirstRosterEntry ? "" : ", ") +
				std::string{ "{sessionId=" } +
				std::to_string(player.iSessionId) +
				", playerId=" + std::to_string(playerId) +
				", spawn=" + player.strSpawnPlacementId +
				", peer=" + peer +
				", lastInboundUnixMs=" +
				std::to_string(lastInboundUnixMilliseconds) +
				", lastInboundAgeMs=" +
				std::to_string(lastInboundAgeMilliseconds) + '}';
			if (roomFullContext.size() + rosterEntry.size() + 1u >
				MAX_ROOM_FULL_CONTEXT_BYTES)
			{
				constexpr std::string_view TRUNCATED =
					", {truncated=true}]";
				roomFullContext.resize((std::min)(
					roomFullContext.size(),
					MAX_ROOM_FULL_CONTEXT_BYTES - TRUNCATED.size()));
				roomFullContext.append(TRUNCATED);
				break;
			}
			roomFullContext += rosterEntry;
			isFirstRosterEntry = false;
		}
		if (roomFullContext.empty() || ']' != roomFullContext.back())
			roomFullContext += ']';
		if (Send_EnterRejected(
				session, ENTER_WORLD_REJECTION_REASON::ROOM_FULL))
		{
			session->Request_Close_After_Flush(
				SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL,
				0,
				"typed ROOM_FULL rejection flushed before close; " +
					roomFullContext);
		}
		else
		{
			session->Request_Close(
				SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL,
				0,
				"ROOM_FULL rejection could not be queued; " +
					roomFullContext);
		}
		return false;
	}
	STAGED_PLAYER_ENTRY entry{};
	SESSION_DIAGNOSTIC_REASON reason{};
	std::string status;
	if (!Stage_PlayerEntry(session, enterWorld, {}, entry, reason, status,
			spawnPlacementOverrideId, carriedInventory))
	{
		session->Request_Close(reason, WSAEINVAL, status);
		return false;
	}
	if (!Build_PlayerEntryFrames(entry, std::span<const STAGED_PLAYER_ENTRY>{ &entry, 1u }, status))
	{
		m_strStatus = status;
		session->Request_Close(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, 0, status);
		return false;
	}
	CClientSession::RELIABLE_BATCH_TRANSACTION outbound;
	if (!outbound.Prepare({ { session, entry.Frames } }, status))
	{
		session->Request_Close(SESSION_DIAGNOSTIC_REASON::SERVER_INITIAL_SYNC_ENQUEUE_FAILED, 0, status);
		return false;
	}
	Commit_PlayerEntry(entry);
	outbound.Commit();
	Broadcast_Spawned(entry.Player, sessionId);
	std::cout << "Player joined. World=" << static_cast<unsigned>(m_eWorldId)
		<< ", SessionId=" << sessionId << ", PlayerId=" << entry.Player.iPlayerId
		<< ", Spawn=" << entry.Player.strSpawnPlacementId
		<< ", RoomPlayers=" << m_Players.size() << '\n';
	return true;
}

void LostArk::Server::CGameRoom::Leave(
	const SESSION_ID sessionId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason, const bool publishDeparture)
{
	using namespace LostArk::Shared;

#ifdef _DEBUG
	if (sessionId == m_ValtanPatternIdAudition.iOwnerSessionId)
	{
		Cancel_ValtanNextPatternReservation("owner left the room");
		if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == m_ValtanPatternIdAudition.ePhase)
		{
			if (SERVER_WORLD_ENTITY* boss =
				Find_AuditionBoss(m_ValtanPatternIdAudition.strBossPlacementId))
			{
				std::erase(boss->PendingPatternIds, m_ValtanPatternIdAudition.strPatternId);
				boss->bAutomaticPatternSequenceAuditionOverride = true;
				boss->bAutomaticPatternSequenceAuditionHold = true;
			}
			Cancel_ValtanPatternIdAudition("owner left before the occurrence started");
		}
		else
		{
			// A already running with other players may finish normally; its
			// departed owner can no longer append to that terminal anchor.
			m_ValtanPatternIdAudition.iOwnerSessionId = INVALID_SESSION_ID;
		}
	}
	m_ValtanNextPatternReceiptBySessionId.erase(sessionId);
	if (sessionId == m_ValtanPatternFlowAudition.iOwnerSessionId &&
		Is_ValtanPatternFlowRunning())
	{
		Abort_ValtanPatternFlowForOwner(
			sessionId, "Valtan pattern-flow owner left the room");
		(void)Flush_ValtanPatternFlowLifecycle();
	}
	if (sessionId == m_ValtanTimelineAudition.iOwnerSessionId &&
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
			m_ValtanTimelineAudition.ePhase)
	{
		Stop_ValtanTimelineRow();
	}
	if (sessionId == m_KoukuSaydonPatternAudition.iOwnerSessionId) Clear_KoukuSaydonPatternAudition();

	std::erase_if(m_PendingKoukuSaydonPatternAuditionLifecycle,
		[sessionId](const TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE& edge)
		{
			return edge.iSessionId == sessionId;
		});
	m_KoukuSaydonPatternAuditionReceiptBySessionId.erase(sessionId);
#endif
	m_ValtanAuditionSequenceBySessionId.erase(sessionId);
	m_WorldPlaybackRequestSequences.erase(sessionId);
	m_ValtanPatternIdAuditionSequenceBySessionId.erase(sessionId);
	m_ValtanPatternFlowStartSequenceBySessionId.erase(sessionId);
	m_ValtanPatternFlowControlSequenceBySessionId.erase(sessionId);
	m_PendingPartyTransferResults.erase(sessionId);
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
	m_CombatObjectRuntime.Cancel_Source(netEntityId);
	if (m_KoukuCardMaze.Remove_Player(playerId))
		Reset_CardMaze();
	m_ServerTriggerSystem.Remove_Player(playerId);
	if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
		session->Bind_PlayerId(INVALID_PLAYER_ID);
	m_PlayerIdByEntityId.erase(netEntityId);
	m_PlayerIdBySessionId.erase(sessionPlayerIter);
	m_PendingPartyInviteByTargetPlayerId.erase(playerId);
	std::erase_if(m_PendingPartyInviteByTargetPlayerId,
		[playerId](const auto& invite) { return invite.second == playerId; });
	Cancel_RaidEntryProposalsInvolving(playerId);
	Remove_FromParty(playerId);
	m_Players.erase(playerIter);
	m_Sessions.erase(sessionId);
	if (publishDeparture)
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

void LostArk::Server::CGameRoom::Close_SessionForBindingFailure(
	const SESSION_ID sessionId,
	const std::string_view packetName,
	const std::string_view validation)
{
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	session->Request_Close(
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
		WSAENOTCONN,
		"packet=" + std::string{ packetName } + " validation=" +
			std::string{ validation });
}
