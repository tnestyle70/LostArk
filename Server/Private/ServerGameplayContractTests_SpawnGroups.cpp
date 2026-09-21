#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "ClientSession.h"
#include "Gameplay/WorldCollisionContract.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerTriggerSystem.h"
#include "SpawnGroupBootstrap.h"
#include "SpawnGroupRuntime.h"
#include "WorldBootstrap.h"
#include "WorldDestructionRuntime.h"
#include "WorldDestructionBootstrapContractTests.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;

void LostArk::Server::CServerGameplayContractRunner::Run_SpawnGroups(TESTS& tests, CGameplayCatalog& catalog)
{

	{
		/* A trigger fires again: a dormant group starts, a running group is never
		   stacked, and a finished group restarts only once its monsters are gone. */
		CSpawnGroupBootstrap repeatBootstrap;
		CSpawnGroupRuntime repeatRuntime;
		std::string repeatStatus;
		std::uint32_t alive = 0u;
		std::uint32_t spawned = 0u;
		const CSpawnGroupRuntime::ACTIVE_COUNT_QUERY aliveNow =
			[&alive](const std::string&) { return alive; };
		const CSpawnGroupRuntime::SPAWN_CALLBACK spawnOne =
			[&](const std::string&, const SPAWN_GROUP_ENTRY&, const SPAWN_GROUP_ANCHOR&,
				const MONSTER_RUNTIME_PROFILE&, std::uint32_t)
			{
				++spawned;
				++alive;
				return true;
			};
		const std::string groupId = "spawn.character-select.monster";
		tests.Require(repeatBootstrap.Load(WORLD_ID::CHARACTER_SELECT_ARENA) &&
			repeatRuntime.Initialize(repeatBootstrap, repeatStatus),
			"Load a real audition spawn group for the repeat activation contract");
		tests.Require(repeatRuntime.Activate_Repeat(groupId, aliveNow) &&
			!repeatRuntime.Activate_Repeat(groupId, aliveNow),
			"A dormant group starts on a trigger and a running one is not stacked");
		repeatRuntime.Update(1.f / 30.f, repeatBootstrap, aliveNow, spawnOne);
		repeatRuntime.Update(1.f / 30.f, repeatBootstrap, aliveNow, spawnOne);
		tests.Require(1u == spawned && !repeatRuntime.Activate_Repeat(groupId, aliveNow),
			"A group whose monster is alive ignores a second trigger");
		alive = 0u;
		repeatRuntime.Update(1.f / 30.f, repeatBootstrap, aliveNow, spawnOne);
		tests.Require(repeatRuntime.Is_Completed(groupId),
			"The group completes once its monster is gone");
		alive = 1u;
		tests.Require(!repeatRuntime.Activate_Repeat(groupId, aliveNow) &&
			repeatRuntime.Is_Completed(groupId),
			"A completed group is not restarted while one of its monsters still lives");
		alive = 0u;
		tests.Require(repeatRuntime.Activate_Repeat(groupId, aliveNow) &&
			!repeatRuntime.Is_Completed(groupId),
			"A completed group restarts once its field is clear");
		repeatRuntime.Update(1.f / 30.f, repeatBootstrap, aliveNow, spawnOne);
		tests.Require(2u == spawned,
			"The restarted group spawns its wave again");
	}


	{
		CSpawnGroupBootstrap spawnBootstrap;
		const bool loaded =
			spawnBootstrap.Load(WORLD_ID::CHARACTER_SELECT_ARENA);
		const auto& groups = spawnBootstrap.Get_Groups();
		const auto monsterGroup = std::find_if(
			groups.begin(), groups.end(),
			[](const SPAWN_GROUP_DEFINITION& group)
			{
				return group.strSpawnGroupId ==
					"spawn.character-select.monster";
			});
		const auto minibossGroup = std::find_if(
			groups.begin(), groups.end(),
			[](const SPAWN_GROUP_DEFINITION& group)
			{
				return group.strSpawnGroupId ==
					"spawn.character-select.miniboss";
			});
		const SPAWN_GROUP_ANCHOR* monsterAnchor = spawnBootstrap.Find_Anchor(
			"anchor.character-select.monster");
		const SPAWN_GROUP_ANCHOR* minibossAnchor = spawnBootstrap.Find_Anchor(
			"anchor.character-select.miniboss");
		const MONSTER_RUNTIME_PROFILE* monsterProfile =
			spawnBootstrap.Find_Profile("MONSTER_VALTAN_PADD_01");
		const MONSTER_RUNTIME_PROFILE* minibossProfile =
			spawnBootstrap.Find_Profile("MINIBOSS_LUGARU");
		/* Named rather than counted: the audition Area gains a group whenever a
		new archetype needs somewhere to be tried out, and a bare count would
		make every such addition look like a regression. */
		tests.Require(
			loaded && 1u == spawnBootstrap.Get_Revision() && !groups.empty() &&
			groups.end() != monsterGroup && groups.end() != minibossGroup &&
			nullptr != monsterAnchor && nullptr != minibossAnchor &&
			nullptr != monsterProfile && nullptr != minibossProfile,
			"Load the Character Select spawn groups, anchors, and profiles");
		tests.Require(
			nullptr != monsterProfile && nullptr != minibossProfile &&
			std::fabs(monsterProfile->fHitKnockbackScale - 1.f) < 0.0001f &&
			0.f == minibossProfile->fHitKnockbackScale,
			"Read the published hit knockback scale for the monster and the miniboss");
		tests.Require(
			nullptr != monsterProfile && nullptr != minibossProfile &&
			std::fabs(monsterProfile->fTargetReleaseRange - 24.f) < 0.0001f &&
			std::fabs(monsterProfile->fTurnSpeedDegreesPerSecond - 420.f) < 0.0001f &&
			std::fabs(monsterProfile->fAcceleration - 12.f) < 0.0001f &&
			std::fabs(monsterProfile->fDeceleration - 16.f) < 0.0001f &&
			std::fabs(monsterProfile->fArrivalSlowRadius - 1.6f) < 0.0001f &&
			std::fabs(minibossProfile->fTargetReleaseRange - 36.f) < 0.0001f,
			"Read monster target hysteresis, turn, acceleration, deceleration, and arrival slowdown from bootstrap v4");
		tests.Require(
			nullptr != monsterProfile && nullptr != minibossProfile &&
			std::fabs(monsterProfile->fAttackPushRangeM - 0.5f) < 0.0001f &&
			150u == monsterProfile->iAttackPushMs &&
			!monsterProfile->bAttackKnockdown &&
			0u == monsterProfile->iAttackDownMs &&
			std::fabs(minibossProfile->fAttackPushRangeM - 2.f) < 0.0001f &&
			250u == minibossProfile->iAttackPushMs &&
			minibossProfile->bAttackKnockdown &&
			2000u == minibossProfile->iAttackDownMs,
			"Read the published attack push and knockdown for the monster and the miniboss");

		const auto hasImmediateEntry = [](
			const SPAWN_GROUP_DEFINITION& group,
			const char* archetypeId,
			const char* anchorId)
		{
			return group.strRequiredCompletedGroupId.empty() &&
				1u == group.iMaxAlive && 1u == group.Waves.size() &&
				0u == group.Waves[0].iStartDelayMs &&
				1u == group.Waves[0].Entries.size() &&
				group.Waves[0].Entries[0].strArchetypeId == archetypeId &&
				1u == group.Waves[0].Entries[0].iCount &&
				group.Waves[0].Entries[0].strAnchorId == anchorId &&
				0u == group.Waves[0].Entries[0].iInitialDelayMs &&
				0u == group.Waves[0].Entries[0].iSpawnIntervalMs;
		};
		tests.Require(
			groups.end() != monsterGroup && groups.end() != minibossGroup &&
			hasImmediateEntry(
				*monsterGroup,
				"MONSTER_VALTAN_PADD_01",
				"anchor.character-select.monster") &&
			hasImmediateEntry(
				*minibossGroup,
				"MINIBOSS_LUGARU",
				"anchor.character-select.miniboss"),
			"Keep Character Select audition groups single-wave and zero-delay");

		CSpawnGroupRuntime immediateRuntime;
		std::string immediateStatus;
		std::uint32_t immediateSpawnCount = 0u;
		const bool immediateInitialized =
			immediateRuntime.Initialize(spawnBootstrap, immediateStatus);
		const bool failedImmediatePreservedDormant = immediateInitialized &&
			!immediateRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				spawnBootstrap,
				[](const std::string&, const SPAWN_GROUP_ENTRY&,
					const SPAWN_GROUP_ANCHOR&,
					const MONSTER_RUNTIME_PROFILE&, std::uint32_t)
				{
					return false;
				}) &&
			!immediateRuntime.Is_ActiveOrCompleted(
				"spawn.character-select.monster");
		const auto countImmediateSpawn = [&immediateSpawnCount](
			const std::string&, const SPAWN_GROUP_ENTRY&,
			const SPAWN_GROUP_ANCHOR&, const MONSTER_RUNTIME_PROFILE&,
			std::uint32_t)
			{
				++immediateSpawnCount;
				return true;
			};
		tests.Require(
			failedImmediatePreservedDormant &&
			immediateRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				spawnBootstrap,
				countImmediateSpawn) &&
			immediateRuntime.Activate_Immediate(
				"spawn.character-select.miniboss",
				spawnBootstrap,
				countImmediateSpawn) &&
			2u == immediateSpawnCount &&
			immediateRuntime.Is_ActiveOrCompleted(
				"spawn.character-select.monster") &&
			!immediateRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				spawnBootstrap,
				countImmediateSpawn),
			"Commit immediate audition activation only after its spawn callback succeeds");

		CSpawnGroupRuntime spawnRuntime;
		std::string spawnStatus;
		const bool initialized =
			spawnRuntime.Initialize(spawnBootstrap, spawnStatus);
		tests.Require(
			initialized &&
			spawnRuntime.Activate("spawn.character-select.monster") &&
			spawnRuntime.Activate("spawn.character-select.miniboss") &&
			spawnRuntime.Is_ActiveOrCompleted(
				"spawn.character-select.monster") &&
			!spawnRuntime.Activate("spawn.character-select.monster"),
			"Activate both Character Select audition groups");
		std::array<std::uint32_t, 2> scheduledByGroup{};
		bool callbackContractValid = true;
		spawnRuntime.Update(
			1.f / 30.f,
			spawnBootstrap,
			[](const std::string&) { return 0u; },
			[&scheduledByGroup, &callbackContractValid](
				const std::string& spawnGroupId,
				const SPAWN_GROUP_ENTRY& entry,
				const SPAWN_GROUP_ANCHOR& anchor,
				const MONSTER_RUNTIME_PROFILE& profile,
				const std::uint32_t ordinal)
			{
				if (spawnGroupId == "spawn.character-select.monster")
				{
					++scheduledByGroup[0];
					callbackContractValid = callbackContractValid &&
						entry.strArchetypeId == "MONSTER_VALTAN_PADD_01" &&
						anchor.strAnchorId == "anchor.character-select.monster" &&
						profile.strArchetypeId == entry.strArchetypeId &&
						0u == ordinal;
				}
				else if (spawnGroupId == "spawn.character-select.miniboss")
				{
					++scheduledByGroup[1];
					callbackContractValid = callbackContractValid &&
						entry.strArchetypeId == "MINIBOSS_LUGARU" &&
						anchor.strAnchorId == "anchor.character-select.miniboss" &&
						profile.strArchetypeId == entry.strArchetypeId &&
						0u == ordinal;
				}
				else
				{
					callbackContractValid = false;
				}
				return true;
			});
		tests.Require(
			callbackContractValid && 1u == scheduledByGroup[0] &&
			1u == scheduledByGroup[1] &&
			2u == scheduledByGroup[0] + scheduledByGroup[1],
			"Schedule exactly two Character Select callbacks in the first update");

		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto resetRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::CHARACTER_SELECT_ARENA);
		CGameRoom& resetRoom = *resetRoomStorage;
		SERVER_PLAYER resetPlayer{};
		resetPlayer.iSessionId = 501u;
		resetPlayer.iPlayerId = 502u;
		resetPlayer.iNetEntityId = 503u;
		resetPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		resetPlayer.strNickName = "ResetFixture";
		resetPlayer.iCurrentHp = 100u;
		resetPlayer.iMaximumHp = 100u;
		resetPlayer.isCombatReady = true;
		resetRoom.m_Players.emplace(resetPlayer.iPlayerId, resetPlayer);
		resetRoom.m_PlayerIdBySessionId.emplace(
			resetPlayer.iSessionId, resetPlayer.iPlayerId);
		resetRoom.m_PlayerIdByEntityId.emplace(
			resetPlayer.iNetEntityId, resetPlayer.iPlayerId);
		const bool resetGroupActivated =
			resetRoom.m_SpawnGroupRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				resetRoom.m_SpawnGroupBootstrap,
				[&resetRoom](const std::string& spawnGroupId,
					const SPAWN_GROUP_ENTRY& entry,
					const SPAWN_GROUP_ANCHOR& anchor,
					const MONSTER_RUNTIME_PROFILE& profile,
					const std::uint32_t ordinal)
				{
					return resetRoom.Spawn_Monster(
						spawnGroupId, entry, anchor, profile, ordinal);
				});
		const bool spawnedBeforeLeave = std::any_of(
			resetRoom.m_WorldEntities.begin(),
			resetRoom.m_WorldEntities.end(),
			[](const SERVER_WORLD_ENTITY& entity)
			{
				return entity.strSpawnGroupId ==
					"spawn.character-select.monster";
			});
		resetRoom.Leave(
			resetPlayer.iSessionId,
			PLAYER_DESPAWN_REASON::DISCONNECTED);
		tests.Require(
			resetGroupActivated && spawnedBeforeLeave &&
			resetRoom.m_Players.empty() &&
			resetRoom.m_WorldEntities.empty() &&
			resetRoom.m_SpawnGroupRuntime.Activate(
				"spawn.character-select.monster"),
			"Reset Character Select dynamic entities and spawn groups after the room becomes empty");

		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto retirementRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::CHARACTER_SELECT_ARENA);
		CGameRoom& retirementRoom = *retirementRoomStorage;
		ROOM_COMMAND queuedLeave{};
		queuedLeave.eType = ROOM_COMMAND_TYPE::LEAVE;
		queuedLeave.iSessionId = 601u;
		queuedLeave.eLeaveReason = PLAYER_DESPAWN_REASON::DISCONNECTED;
		const bool queuedBeforeRetirement =
			retirementRoom.Enqueue(std::move(queuedLeave));
		const bool sealedBeforeDrain =
			retirementRoom.Try_SealPrivateArenaForRetirement();
		retirementRoom.Tick(1.f / 30.f);
		const bool sealedAfterDrain =
			retirementRoom.Try_SealPrivateArenaForRetirement();

		ROOM_COMMAND commandAfterSeal{};
		commandAfterSeal.eType = ROOM_COMMAND_TYPE::LEAVE;
		commandAfterSeal.iSessionId = 602u;
		commandAfterSeal.eLeaveReason =
			PLAYER_DESPAWN_REASON::DISCONNECTED;
		tests.Require(
			queuedBeforeRetirement && !sealedBeforeDrain &&
			sealedAfterDrain &&
			!retirementRoom.Enqueue(std::move(commandAfterSeal)),
			"Retire a private Character Select arena only after queued leave work drains");

		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto sharedRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::BERN);
		CGameRoom& sharedRoom = *sharedRoomStorage;
		tests.Require(
			!sharedRoom.Try_SealPrivateArenaForRetirement(),
			"Never seal a shared world through the private arena retirement path");
	}

	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto raidRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& raidRoom = *raidRoomStorage;
		const auto& placements = raidRoom.m_WorldBootstrap.Get_Placements();
		std::vector<const WORLD_BOOTSTRAP_PLACEMENT*> playerSpawns;
		for (const WORLD_BOOTSTRAP_PLACEMENT& placement : placements)
		{
			if (placement.isEnabled &&
				WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind)
			{
				playerSpawns.push_back(&placement);
			}
		}

		auto addPlayer = [&raidRoom](
			const SESSION_ID sessionId,
			const PLAYER_ID playerId,
			const NET_ENTITY_ID entityId,
			const WORLD_BOOTSTRAP_PLACEMENT& spawn)
		{
			SERVER_PLAYER player{};
			player.iSessionId = sessionId;
			player.iPlayerId = playerId;
			player.iNetEntityId = entityId;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.strNickName = "RaidFixture" + std::to_string(playerId);
			player.strSpawnPlacementId = spawn.strPlacementId;
			player.fPositionX = spawn.fPositionX;
			player.fPositionY = spawn.fPositionY;
			player.fPositionZ = spawn.fPositionZ;
			player.iCurrentHp = 100u;
			player.iMaximumHp = 100u;
			player.isCombatReady = true;
			raidRoom.m_Players.emplace(playerId, player);
			raidRoom.m_PlayerIdBySessionId.emplace(sessionId, playerId);
			raidRoom.m_PlayerIdByEntityId.emplace(entityId, playerId);
		};

		std::vector<std::shared_ptr<CClientSession>> registeredSessions;
		for (SESSION_ID sessionId = 1001u; sessionId <= 1009u; ++sessionId)
		{
			auto session = std::make_shared<CClientSession>(
				sessionId, INVALID_SOCKET,
				CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
			session->m_isSendRunning.store(true);
			raidRoom.Handle_Register(session);
			registeredSessions.push_back(std::move(session));
		}
		bool firstEightJoined = raidRoom.Is_Ready() &&
			MAX_VALTAN_RAID_PLAYERS == playerSpawns.size();
		for (std::size_t index = 0u;
			firstEightJoined && index < MAX_VALTAN_RAID_PLAYERS; ++index)
		{
			C2S_ENTER_WORLD entry{};
			entry.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
			entry.eWorldId = WORLD_ID::VALTAN_ARENA;
			entry.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			entry.strNickName = "RaidFixture" + std::to_string(index + 1u);
			firstEightJoined = raidRoom.Join(1001u + index, entry) &&
				raidRoom.m_Players.size() == index + 1u;
		}
		const auto firstPlayerOwner =
			raidRoom.m_PlayerIdBySessionId.find(1001u);
		const PLAYER_ID firstRaidPlayerId =
			raidRoom.m_PlayerIdBySessionId.end() == firstPlayerOwner ?
				0u : firstPlayerOwner->second;
		const auto firstPlayer = raidRoom.m_Players.find(firstRaidPlayerId);
		const NET_ENTITY_ID firstRaidEntityId =
			raidRoom.m_Players.end() == firstPlayer ?
				0u : firstPlayer->second.iNetEntityId;
		const PLAYER_ID allocatorBeforeFull = raidRoom.m_iNextPlayerId;
		const NET_ENTITY_ID entityAllocatorBeforeFull = raidRoom.m_iNextNetEntityId;
		const std::size_t playersBeforeFull = raidRoom.m_Players.size();
		const std::size_t sessionOwnersBeforeFull =
			raidRoom.m_PlayerIdBySessionId.size();
		const std::size_t entityOwnersBeforeFull =
			raidRoom.m_PlayerIdByEntityId.size();
		C2S_ENTER_WORLD ninthEntry{};
		ninthEntry.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		ninthEntry.eWorldId = WORLD_ID::VALTAN_ARENA;
		ninthEntry.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		ninthEntry.strNickName = "NinthRaidFixture";
		const bool ninthRejected = !raidRoom.Join(1009u, ninthEntry);
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC ninthDiagnostic =
			registeredSessions.back()->Get_CloseDiagnostic();
		tests.Require(
			firstEightJoined && raidRoom.Is_Ready() &&
			MAX_VALTAN_RAID_PLAYERS == playerSpawns.size() &&
			raidRoom.Is_PlayerAdmissionFull() &&
			nullptr == raidRoom.Find_AvailablePlayerSpawn() &&
			ninthRejected &&
			SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL ==
				ninthDiagnostic.eReason &&
			std::string::npos !=
				ninthDiagnostic.strContext.find("activePlayers=8") &&
			std::string::npos !=
				ninthDiagnostic.strContext.find(
					"registeredSessionsIncludingCandidate=9") &&
			std::string::npos !=
				ninthDiagnostic.strContext.find("candidateSessionId=1009") &&
			std::string::npos !=
				ninthDiagnostic.strContext.find("candidateRegistered=true") &&
			std::string::npos !=
				ninthDiagnostic.strContext.find("enabledPlayerSpawns=8") &&
			std::string::npos !=
				ninthDiagnostic.strContext.find("sessionId=1001") &&
			std::string::npos !=
				ninthDiagnostic.strContext.find(
					"playerId=" + std::to_string(firstRaidPlayerId)) &&
			std::string::npos !=
				ninthDiagnostic.strContext.find("peer=unknown:0") &&
			ninthDiagnostic.strContext.size() <= 1024u &&
			allocatorBeforeFull == raidRoom.m_iNextPlayerId &&
			entityAllocatorBeforeFull == raidRoom.m_iNextNetEntityId &&
			playersBeforeFull == raidRoom.m_Players.size() &&
			sessionOwnersBeforeFull == raidRoom.m_PlayerIdBySessionId.size() &&
			entityOwnersBeforeFull == raidRoom.m_PlayerIdByEntityId.size(),
			"Reject a ninth Valtan admission as room full without mutating room ownership or allocators");
		registeredSessions.back()->Request_Close();
		raidRoom.m_Sessions.erase(1009u);

		const std::string releasedSpawnId =
			playerSpawns.size() < 2u ? std::string{} :
			playerSpawns[1]->strPlacementId;
		raidRoom.Leave(1002u, PLAYER_DESPAWN_REASON::DISCONNECTED);
		const WORLD_BOOTSTRAP_PLACEMENT* releasedSpawn =
			raidRoom.Find_AvailablePlayerSpawn();
		const bool releasedSlotAvailable = nullptr != releasedSpawn &&
			releasedSpawn->strPlacementId == releasedSpawnId;
		auto replacementSession = std::make_shared<CClientSession>(
			1010u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		replacementSession->m_isSendRunning.store(true);
		raidRoom.Handle_Register(replacementSession);
		registeredSessions.push_back(replacementSession);
		C2S_ENTER_WORLD replacementEntry{};
		replacementEntry.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		replacementEntry.eWorldId = WORLD_ID::VALTAN_ARENA;
		replacementEntry.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		replacementEntry.strNickName = "ReplacementRaidFixture";
		const bool replacementJoined = releasedSlotAvailable &&
			raidRoom.Join(1010u, replacementEntry);
		const auto replacementOwner =
			raidRoom.m_PlayerIdBySessionId.find(1010u);
		const auto replacementPlayer =
			raidRoom.m_PlayerIdBySessionId.end() == replacementOwner ?
				raidRoom.m_Players.end() :
				raidRoom.m_Players.find(replacementOwner->second);
		tests.Require(
			replacementJoined &&
			raidRoom.m_Players.end() != replacementPlayer &&
			releasedSpawnId == replacementPlayer->second.strSpawnPlacementId &&
			raidRoom.Is_PlayerAdmissionFull() &&
			MAX_VALTAN_RAID_PLAYERS == raidRoom.m_Players.size(),
			"Join eight Valtan players through the real admission path, reject the ninth, and admit a replacement into the released stable spawn");

		const WORLD_BOOTSTRAP_PLACEMENT* bossTrigger =
			raidRoom.Find_Placement("Stage_Boss");
		if (nullptr != bossTrigger && !raidRoom.m_Players.empty())
		{
			SERVER_PLAYER& triggerPlayer = raidRoom.m_Players.begin()->second;
			triggerPlayer.fPositionX = bossTrigger->fPositionX;
			triggerPlayer.fPositionY = bossTrigger->fPositionY -
				LostArk::Shared::WorldCollision::PLAYER_CENTER_OFFSET_Y;
			triggerPlayer.fPositionZ = bossTrigger->fPositionZ;
		}
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> promptEdges;
		std::uint32_t encounterActivationCount = 0u;
		const auto activateBossEncounter = [&raidRoom, &encounterActivationCount](
			const WORLD_TRIGGER_ACTION_KIND kind,
			const std::string& targetId)
		{
			if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER != kind)
				return false;
			++encounterActivationCount;
			return raidRoom.Activate_Encounter(targetId);
		};
		raidRoom.m_ServerTriggerSystem.Evaluate_Entries(
			raidRoom.m_Players,
			700u,
			transfers,
			activateBossEncounter,
			promptEdges);
#ifdef _DEBUG
		/* The Debug Valtan corridor shortcut makes Stage_Boss wait for G; Release
		   fires the boss start on entry above. */
		if (!raidRoom.m_Players.empty())
		{
			(void)raidRoom.m_ServerTriggerSystem.Activate_Here(
				raidRoom.m_Players.begin()->first,
				raidRoom.m_Players,
				700u,
				transfers,
				activateBossEncounter);
		}
#endif
		auto bossBeforeReset = std::find_if(
			raidRoom.m_WorldEntities.begin(),
			raidRoom.m_WorldEntities.end(),
			[](const SERVER_WORLD_ENTITY& entity)
			{
				return "boss.valtan.center" == entity.strPlacementId;
			});
		const bool bossActivatedBeforeReset =
			1u == encounterActivationCount &&
			raidRoom.m_WorldEntities.end() != bossBeforeReset;
		if (raidRoom.m_WorldEntities.end() != bossBeforeReset)
		{
			bossBeforeReset->iCurrentHp = 1u;
			bossBeforeReset->iPhase = 2u;
			bossBeforeReset->eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
			bossBeforeReset->strPatternId = "reset.fixture.pattern";
		}
		const bool spawnGroupActivatedBeforeReset =
			raidRoom.m_SpawnGroupRuntime.Activate("spawn.valtan.stage01");
		SERVER_WORLD_ENTITY dynamicMonster{};
		dynamicMonster.iNetEntityId = 9001u;
		dynamicMonster.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
		dynamicMonster.strPlacementId = "reset.fixture.monster";
		dynamicMonster.strSpawnGroupId = "spawn.valtan.stage01";
		raidRoom.m_WorldEntities.push_back(std::move(dynamicMonster));
		DAMAGE_EVENT damageEvent{};
		damageEvent.iTargetNetEntityId = firstRaidEntityId;
		damageEvent.iAmount = 1u;
		raidRoom.m_TickDamageEvents.push_back(damageEvent);
		raidRoom.m_iServerTick = 777u;
		SERVER_WORLD_TRANSFER_REQUEST pendingTransfer{};
		pendingTransfer.iSessionId = 8080u;
		pendingTransfer.eTargetWorldId = WORLD_ID::BERN;
		pendingTransfer.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		pendingTransfer.strNickName = "PendingTransfer";
		raidRoom.m_PendingWorldTransfers.push_back(pendingTransfer);
		const PLAYER_ID playerAllocatorBeforeReset = raidRoom.m_iNextPlayerId;
		const NET_ENTITY_ID netAllocatorBeforeReset = raidRoom.m_iNextNetEntityId;

		std::vector<SESSION_ID> activeSessions;
		for (const auto& [sessionId, playerId] : raidRoom.m_PlayerIdBySessionId)
		{
			(void)playerId;
			activeSessions.push_back(sessionId);
		}
		for (const SESSION_ID sessionId : activeSessions)
		{
			raidRoom.Leave(sessionId, PLAYER_DESPAWN_REASON::DISCONNECTED);
		}
		const bool emptyResetPreservedMonotonicState =
			raidRoom.Is_Ready() && raidRoom.m_Players.empty() &&
			raidRoom.m_PlayerIdBySessionId.empty() &&
			raidRoom.m_PlayerIdByEntityId.empty() &&
			raidRoom.m_WorldEntities.empty() &&
			raidRoom.m_TickDamageEvents.empty() &&
			777u == raidRoom.m_iServerTick &&
			playerAllocatorBeforeReset == raidRoom.m_iNextPlayerId &&
			netAllocatorBeforeReset == raidRoom.m_iNextNetEntityId &&
			1u == raidRoom.m_PendingWorldTransfers.size() &&
			8080u == raidRoom.m_PendingWorldTransfers.front().iSessionId;
		const bool spawnGroupReactivated =
			raidRoom.m_SpawnGroupRuntime.Activate("spawn.valtan.stage01");

		if (!playerSpawns.empty() && nullptr != bossTrigger)
		{
			addPlayer(1020u, 2020u, 3020u, *playerSpawns.front());
			SERVER_PLAYER& nextGenerationPlayer =
				raidRoom.m_Players.find(2020u)->second;
			nextGenerationPlayer.fPositionX = bossTrigger->fPositionX;
			nextGenerationPlayer.fPositionY = bossTrigger->fPositionY -
				LostArk::Shared::WorldCollision::PLAYER_CENTER_OFFSET_Y;
			nextGenerationPlayer.fPositionZ = bossTrigger->fPositionZ;
		}
		encounterActivationCount = 0u;
		raidRoom.m_ServerTriggerSystem.Evaluate_Entries(
			raidRoom.m_Players,
			778u,
			transfers,
			[&raidRoom, &encounterActivationCount](
				const WORLD_TRIGGER_ACTION_KIND kind,
				const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER != kind)
					return false;
				++encounterActivationCount;
				return raidRoom.Activate_Encounter(targetId);
			},
				promptEdges);
		const auto bossAfterReset = std::find_if(
			raidRoom.m_WorldEntities.begin(),
			raidRoom.m_WorldEntities.end(),
			[](const SERVER_WORLD_ENTITY& entity)
			{
				return "boss.valtan.center" == entity.strPlacementId;
			});
		const bool encounterRestarted =
			1u == encounterActivationCount &&
			raidRoom.m_WorldEntities.end() != bossAfterReset &&
			bossAfterReset->iCurrentHp == bossAfterReset->iMaximumHp &&
			1u == bossAfterReset->iPhase &&
			SERVER_ENTITY_ACTION::IDLE == bossAfterReset->eAction &&
			bossAfterReset->strPatternId.empty();
		tests.Require(
			bossActivatedBeforeReset && spawnGroupActivatedBeforeReset &&
			emptyResetPreservedMonotonicState && spawnGroupReactivated &&
			encounterRestarted,
			"Reset Valtan boss, triggers, spawn groups, dynamic entities, and damage after the last disconnect while preserving IDs, tick, and transfers");
	}

	{
		CSpawnGroupBootstrap spawnBootstrap;
		const bool loaded = spawnBootstrap.Load(WORLD_ID::VALTAN_ARENA);
		tests.Require(
			loaded && 3u == spawnBootstrap.Get_Groups().size(),
			"Load three authored Valtan spawn groups");
		tests.Require(
			loaded && Reject_CorruptSpawnAnchorReloadTransactionally(
				spawnBootstrap),
			"Reject nonfinite or out-of-range spawn anchors and preserve the admitted bootstrap");
		const std::array<std::string_view, 3u> valtanSpawnGroupIds{
			"spawn.valtan.stage01",
			"spawn.valtan.stage02.miniboss",
			"spawn.valtan.stage03" };
		const bool independentGroupAuthoringExact = loaded && std::all_of(
			valtanSpawnGroupIds.begin(), valtanSpawnGroupIds.end(),
			[&spawnBootstrap](const std::string_view groupId)
			{
				const auto found = std::find_if(
					spawnBootstrap.Get_Groups().begin(),
					spawnBootstrap.Get_Groups().end(),
					[groupId](const SPAWN_GROUP_DEFINITION& definition)
					{ return definition.strSpawnGroupId == groupId; });
				return spawnBootstrap.Get_Groups().end() != found &&
					found->strRequiredCompletedGroupId.empty() &&
					0u != found->iMaxAlive && !found->Waves.empty() &&
					std::all_of(found->Waves.begin(), found->Waves.end(),
						[](const SPAWN_GROUP_WAVE& wave)
						{ return !wave.strWaveId.empty() && !wave.Entries.empty(); });
			});
		tests.Require(
			independentGroupAuthoringExact,
			"Keep all three Valtan groups independently activatable with authored waves");

		CSpawnGroupRuntime independentRuntime;
		std::string independentStatus;
		tests.Require(
			independentRuntime.Initialize(spawnBootstrap, independentStatus) &&
			independentRuntime.Activate("spawn.valtan.stage01") &&
			independentRuntime.Activate("spawn.valtan.stage02.miniboss") &&
			independentRuntime.Activate("spawn.valtan.stage03"),
			"Activate every Valtan group without another group's completion prerequisite");

		CSpawnGroupRuntime spawnRuntime;
		std::string spawnStatus;
		tests.Require(
			spawnRuntime.Initialize(spawnBootstrap, spawnStatus),
			"Initialize Valtan spawn group runtime");
		tests.Require(
			spawnRuntime.Activate("spawn.valtan.stage01"),
			"Activate Stage 1 spawn group exactly once");
		/* Counted from the loaded groups rather than written down, because the
		wave composition is authoring that changes whenever the corridor is
		retuned. What the contract owns is that every authored entry is
		scheduled exactly once and the group then completes. */
		std::uint32_t authoredMonsterCount = 0;
		for (const SPAWN_GROUP_DEFINITION& definition : spawnBootstrap.Get_Groups())
		{
			if (definition.strSpawnGroupId != "spawn.valtan.stage01")
				continue;
			for (const SPAWN_GROUP_WAVE& wave : definition.Waves)
				for (const SPAWN_GROUP_ENTRY& groupEntry : wave.Entries)
					authoredMonsterCount += groupEntry.iCount;
		}
		std::uint32_t scheduledMonsterCount = 0;
		for (std::uint32_t step = 0; step < 64u &&
			!spawnRuntime.Is_Completed("spawn.valtan.stage01"); ++step)
		{
			spawnRuntime.Update(
				1.f,
				spawnBootstrap,
				[](const std::string&) { return 0u; },
				[&scheduledMonsterCount](const std::string&,
					const SPAWN_GROUP_ENTRY&,
					const SPAWN_GROUP_ANCHOR&,
					const MONSTER_RUNTIME_PROFILE&,
					const std::uint32_t)
				{
					++scheduledMonsterCount;
					return true;
				});
		}
		tests.Require(
			0u != authoredMonsterCount &&
			authoredMonsterCount == scheduledMonsterCount &&
			spawnRuntime.Is_Completed("spawn.valtan.stage01"),
			"Schedule all Stage 1 waves and complete after all entities clear");
		tests.Require(
			!spawnRuntime.Activate("spawn.valtan.stage01"),
			"Keep the completed Stage 1 ONCE group from activating again");
	}

	{
		auto makeDestructionGraph = [](
			const std::uint32_t breakingDurationTicks,
			const WORLD_DESTRUCTION_TRIGGER_KIND triggerKind,
			const bool hasWorldMutationChannels = true)
		{
			WORLD_DESTRUCTION_DESCRIPTOR_GRAPH graph{};
			graph.Groups.push_back({
				"destroyable.group.valtan.wall.3705102",
				{
					"deploy.valtan.wall.3705102.0",
					"deploy.valtan.wall.3705102.1",
					"deploy.valtan.wall.3705102.2",
					"deploy.valtan.wall.3705102.3",
					"deploy.valtan.wall.3705102.4"
				},
				WORLD_DESTRUCTION_STATE::INTACT });
			graph.Mutations.push_back({
				"mutation.valtan.wall.3705102.fracture",
				"destroyable.group.valtan.wall.3705102",
				WORLD_DESTRUCTION_STATE::FRACTURED,
				breakingDurationTicks,
				hasWorldMutationChannels ?
					"collision.valtan.wall.3705102.fractured" : "",
				hasWorldMutationChannels ?
					"navigation.valtan.wall.3705102.open" : "" });
			graph.Bindings.push_back({
				"binding.valtan.wall.3705102.impact",
				"mutation.valtan.wall.3705102.fracture",
				triggerKind,
				"VALTAN_ARMOR_BREAK_OPENING",
				"WALL_CHARGE",
				"valtan.armor_break.wall_charge",
				1u,
				WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT == triggerKind ?
					"receiver.valtan.wall.3705102" : "" });
			return graph;
		};

		const WORLD_DESTRUCTION_ACTION_TUPLE exactAction{
			"VALTAN_ARMOR_BREAK_OPENING",
			"WALL_CHARGE",
			"valtan.armor_break.wall_charge",
			1u };
		constexpr std::uint64_t SOURCE_BOSS_ENTITY_ID = 7001u;
		CWorldDestructionRuntime runtime;
		std::string status;
		WORLD_DESTRUCTION_TRANSACTION transaction{};
		tests.Require(
			runtime.Initialize(makeDestructionGraph(
				3u, WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT), status) &&
			1u == runtime.Get_EncounterEpoch(),
			"Initialize a nonzero-epoch world destruction graph transactionally");

		WORLD_DESTRUCTION_ACTION_TUPLE wrongAction = exactAction;
		wrongAction.strActionId = "valtan.armor_break.wall_charge.wrong";
		tests.Require(
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH ==
				runtime.Prepare_ImpactTrigger(
					wrongAction, "receiver.valtan.wall.3705102",
					SOURCE_BOSS_ENTITY_ID, 7u, 10u, transaction, status) &&
			transaction.Transitions.empty() &&
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH ==
				runtime.Prepare_ImpactTrigger(
					exactAction, "receiver.valtan.wall.other",
					SOURCE_BOSS_ENTITY_ID, 7u, 10u, transaction, status),
			"Reject non-exact action tuples and impact receivers");

		const bool preparedBreaking =
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
			runtime.Prepare_ImpactTrigger(
				exactAction, "receiver.valtan.wall.3705102",
				SOURCE_BOSS_ENTITY_ID, 7u, 10u, transaction, status) &&
			1u == transaction.BindingApplications.size() &&
			1u == transaction.Transitions.size() &&
			WORLD_DESTRUCTION_STATE::INTACT ==
				transaction.Transitions.front().ePreviousState &&
			WORLD_DESTRUCTION_STATE::BREAKING ==
				transaction.Transitions.front().eNextState &&
			13u == transaction.Transitions.front().iCommitTick &&
			!transaction.Transitions.front().bApplyPersistentMutation &&
			5u == transaction.Transitions.front().MemberPlacementIds.size() &&
			transaction.Transitions.front().strCollisionStateId ==
				"collision.valtan.wall.3705102.fractured" &&
			transaction.Transitions.front().strNavigationStateId ==
				"navigation.valtan.wall.3705102.open";
		tests.Require(
			preparedBreaking && runtime.Commit(transaction, status),
			"Prepare and atomically commit one BREAKING transition");

		WORLD_DESTRUCTION_GROUP_STATE groupState{};
		tests.Require(
			runtime.Find_GroupState(
				"destroyable.group.valtan.wall.3705102", groupState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == groupState.eState &&
			2u == groupState.iStateVersion && 10u == groupState.iStateStartTick &&
			13u == groupState.iCommitTick &&
			groupState.strPendingMutationId ==
				"mutation.valtan.wall.3705102.fracture",
			"Persist the BREAKING version and exact final commit tick");
		tests.Require(
			WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST ==
				runtime.Prepare_ImpactTrigger(
					exactAction, "receiver.valtan.wall.3705102",
					SOURCE_BOSS_ENTITY_ID, 7u, 11u, transaction, status) &&
			transaction.Transitions.empty(),
			"Treat the same pattern-sequence binding request as an idempotent no-op");
		tests.Require(
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE ==
				runtime.Prepare_DueStateCommits(12u, transaction, status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				runtime.Prepare_DueStateCommits(13u, transaction, status) &&
			WORLD_DESTRUCTION_STATE::FRACTURED ==
				transaction.Transitions.front().eNextState &&
			transaction.Transitions.front().bApplyPersistentMutation &&
			runtime.Commit(transaction, status),
			"Commit the persistent wall, collision, and navigation plan at the exact tick");
		tests.Require(
			runtime.Find_GroupState(
				"destroyable.group.valtan.wall.3705102", groupState) &&
			WORLD_DESTRUCTION_STATE::FRACTURED == groupState.eState &&
			3u == groupState.iStateVersion && 13u == groupState.iStateStartTick &&
			0u == groupState.iCommitTick,
			"Converge on a persistent FRACTURED state with one final version");

		const std::uint32_t previousEpoch = runtime.Get_EncounterEpoch();
		tests.Require(
			runtime.Reset(status) &&
			previousEpoch + 1u == runtime.Get_EncounterEpoch() &&
			runtime.Find_GroupState(
				"destroyable.group.valtan.wall.3705102", groupState) &&
			WORLD_DESTRUCTION_STATE::INTACT == groupState.eState &&
			1u == groupState.iStateVersion && 1u == groupState.iStateStartTick &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				runtime.Prepare_ImpactTrigger(
					exactAction, "receiver.valtan.wall.3705102",
					SOURCE_BOSS_ENTITY_ID, 7u, 20u, transaction, status),
			"Reset to a new epoch/version baseline and admit the same sequence in the new encounter");
		tests.Require(
			1u == runtime.Get_GroupStates().size() &&
			"destroyable.group.valtan.wall.3705102" ==
				runtime.Get_GroupStates().front().strGroupId,
			"Enumerate persistent group state in canonical stable-ID order");

		CWorldDestructionRuntime zeroDurationRuntime;
		tests.Require(
			zeroDurationRuntime.Initialize(makeDestructionGraph(
				0u, WORLD_DESTRUCTION_TRIGGER_KIND::STAGE, false), status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				zeroDurationRuntime.Prepare_StageTrigger(
					exactAction, SOURCE_BOSS_ENTITY_ID, 1u, 30u,
					transaction, status) &&
			WORLD_DESTRUCTION_STATE::FRACTURED ==
				transaction.Transitions.front().eNextState &&
			30u == transaction.Transitions.front().iCommitTick &&
			transaction.Transitions.front().strCollisionStateId.empty() &&
			transaction.Transitions.front().strNavigationStateId.empty() &&
			transaction.Transitions.front().bApplyPersistentMutation &&
			zeroDurationRuntime.Commit(transaction, status) &&
			zeroDurationRuntime.Find_GroupState(
				"destroyable.group.valtan.wall.3705102", groupState) &&
			WORLD_DESTRUCTION_STATE::FRACTURED == groupState.eState &&
			2u == groupState.iStateVersion,
			"Commit a zero-duration stage binding directly in one version");

		CWorldDestructionRuntime wrapRuntime;
		const std::uint32_t beforeWrap =
			(std::numeric_limits<std::uint32_t>::max)() - 1u;
		tests.Require(
			wrapRuntime.Initialize(makeDestructionGraph(
				3u, WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT), status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				wrapRuntime.Prepare_ImpactTrigger(
					exactAction, "receiver.valtan.wall.3705102",
					SOURCE_BOSS_ENTITY_ID, 1u, beforeWrap,
					transaction, status) &&
			2u == transaction.Transitions.front().iCommitTick &&
			wrapRuntime.Commit(transaction, status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE ==
				wrapRuntime.Prepare_DueStateCommits(
					(std::numeric_limits<std::uint32_t>::max)(),
					transaction, status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE ==
				wrapRuntime.Prepare_DueStateCommits(1u, transaction, status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				wrapRuntime.Prepare_DueStateCommits(2u, transaction, status) &&
			wrapRuntime.Commit(transaction, status),
			"Skip reserved tick zero and commit exactly after uint32 wrap");
	}

	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		SERVER_WORLD_ENTITY boss{};
		boss.iNetEntityId = 7001u;
		boss.strPatternId = "VALTAN_ARENA_BREAK_109";
		boss.strPatternStageId = "IMPACT";
		boss.strActionId =
			"valtan.mechanic.arena-break-109.impact";
		boss.iPatternStageIndex = 2u;
		boss.iPatternSequence = 15u;
		boss.fPositionX = 151.25f;
		boss.fPositionY = 22.97f;
		boss.fPositionZ = -121.75f;
		boss.fYawDegrees = 90.f;

		const bool applied = room.Is_Ready() &&
			room.Apply_WorldDestructionStageEntry(boss, 450u);
		const auto breakingStates =
			room.m_WorldDestructionRuntime.Get_GroupStates();
		const std::size_t breakingCount = static_cast<std::size_t>(
			std::count_if(breakingStates.begin(), breakingStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return WORLD_DESTRUCTION_STATE::BREAKING == state.eState;
				}));
		/* The 109 batch is the twenty-seven outer ring walls plus every interior
		wall still standing, so the cutscene leaves no wall behind. A floor
		sector or an entrance wall leaving INTACT on this edge would take the
		arena's footing away a whole health-bar chain too early. */
		const std::size_t interiorBreakingCount = static_cast<std::size_t>(
			std::count_if(breakingStates.begin(), breakingStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return WORLD_DESTRUCTION_STATE::INTACT != state.eState &&
						(0u == state.strGroupId.rfind(
							"destroyable.group.valtan.wall159.", 0u) ||
						0u == state.strGroupId.rfind(
							"destroyable.group.valtan.wall.", 0u));
				}));
		const std::size_t outsideBreakingCount = static_cast<std::size_t>(
			std::count_if(breakingStates.begin(), breakingStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return WORLD_DESTRUCTION_STATE::INTACT != state.eState &&
						0u != state.strGroupId.rfind(
							"destroyable.group.valtan.outerwall109.", 0u) &&
						0u != state.strGroupId.rfind(
							"destroyable.group.valtan.wall159.", 0u) &&
						0u != state.strGroupId.rfind(
							"destroyable.group.valtan.wall.", 0u);
				}));
		tests.Require(
			applied && 94u == breakingCount && 67u == interiorBreakingCount &&
			0u == outsideBreakingCount &&
			95u == room.m_iNextWorldDestructionEventSequence,
			"Emit one monotonically sequenced live event for every 109-bar wall");

		const std::uint64_t sequenceAfterFirstEdge =
			room.m_iNextWorldDestructionEventSequence;
		tests.Require(
			room.Apply_WorldDestructionStageEntry(boss, 451u) &&
			sequenceAfterFirstEdge ==
				room.m_iNextWorldDestructionEventSequence,
			"Do not allocate a live event for a duplicate pattern-stage edge");

		WORLD_DESTRUCTION_ACTION_TUPLE action{};
		action.strPatternId = boss.strPatternId;
		action.strStageId = boss.strPatternStageId;
		action.strActionId = boss.strActionId;
		action.iStageIndex = boss.iPatternStageIndex;
		CWorldDestructionRuntime isolatedRuntime;
		std::string status;
		WORLD_DESTRUCTION_TRANSACTION transaction{};
		const bool prepared = isolatedRuntime.Initialize(
			room.m_WorldDestructionBootstrap.Get_DescriptorGraph(), status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				isolatedRuntime.Prepare_StageTrigger(
					action, boss.iNetEntityId, boss.iPatternSequence,
					450u, transaction, status);
		CWorldDestructionRuntime activeRuntime =
			std::move(room.m_WorldDestructionRuntime);
		room.m_WorldDestructionRuntime = std::move(isolatedRuntime);
		room.m_iNextWorldDestructionEventSequence = 1u;
		std::vector<WORLD_DESTRUCTION_EVENT_WIRE> firstEvents;
		std::vector<WORLD_DESTRUCTION_EVENT_WIRE> repeatedEvents;
		const bool builtFirst = prepared &&
			room.Build_WorldDestructionLiveEvents(
				transaction, boss, firstEvents, status);
		const bool builtRepeated =
			room.Build_WorldDestructionLiveEvents(
				transaction, boss, repeatedEvents, status);
		room.m_WorldDestructionRuntime = std::move(activeRuntime);
		tests.Require(
			builtFirst && builtRepeated && 94u == firstEvents.size() &&
			firstEvents.size() == repeatedEvents.size() &&
			1u == firstEvents.front().iEventSequence &&
			94u == firstEvents.back().iEventSequence &&
			firstEvents.front().iRandomSeed ==
				repeatedEvents.front().iRandomSeed &&
			firstEvents.front().fImpactOriginX == boss.fPositionX &&
			firstEvents.front().fImpactOriginY == boss.fPositionY &&
			firstEvents.front().fImpactOriginZ == boss.fPositionZ &&
			std::fabs(firstEvents.front().fImpactDirectionX - 1.f) <= 0.001f &&
			std::fabs(firstEvents.front().fImpactDirectionY) <= 0.001f &&
			std::fabs(firstEvents.front().fImpactDirectionZ) <= 0.001f,
			"Build canonical deterministic events from the authoritative boss pose");

		/* One sequence short of what this batch needs, derived from the batch
		itself so the ledger guard stays covered if the ring ever changes size. */
		room.m_iNextWorldDestructionEventSequence =
			(std::numeric_limits<std::uint64_t>::max)() -
			(static_cast<std::uint64_t>(transaction.Transitions.size()) - 2u);
		std::vector<WORLD_DESTRUCTION_EVENT_WIRE> exhaustedEvents;
		CWorldDestructionRuntime exhaustionRuntime;
		const bool preparedExhaustion = exhaustionRuntime.Initialize(
			room.m_WorldDestructionBootstrap.Get_DescriptorGraph(), status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				exhaustionRuntime.Prepare_StageTrigger(
					action, boss.iNetEntityId, boss.iPatternSequence,
					450u, transaction, status);
		activeRuntime = std::move(room.m_WorldDestructionRuntime);
		room.m_WorldDestructionRuntime = std::move(exhaustionRuntime);
		tests.Require(
			preparedExhaustion &&
			!room.Build_WorldDestructionLiveEvents(
				transaction, boss, exhaustedEvents, status) &&
			exhaustedEvents.empty(),
			"Fail closed before a destruction live-event sequence can wrap");
		room.m_WorldDestructionRuntime = std::move(activeRuntime);

		room.m_iNextWorldDestructionEventSequence = sequenceAfterFirstEdge;
		tests.Require(
			room.Commit_DueWorldDestruction(458u) &&
			sequenceAfterFirstEdge ==
				room.m_iNextWorldDestructionEventSequence,
			"Commit due FRACTURED states without emitting a second live event");

		/* The real 109 commit clears 94 walls. The two independent entrance
		impacts must also clear their sibling receivers, not only source leaves. */
		constexpr const char* FRONT_A = "collision.valtan.wallgroup.frontwallA.receiver";
		constexpr const char* FRONT_B = "collision.valtan.wallgroup.frontwallB.receiver";
		constexpr const char* FRONT_A_CONDITION =
			"condition.valtan.entrance.frontwallA.destroyed";
		constexpr float PASSAGE_START_X = 142.75f;
		constexpr float PASSAGE_END_X = 150.75f;
		constexpr float PASSAGE_Z = -115.25f;
		const BOSS_RUNTIME_PROFILE* bodyProfile = catalog.Find_Boss("BOSS_VALTAN");
		SERVER_BOSS_WALL_HIT entranceHit{};
		tests.Require(
			4u == room.m_ServerCollisionSystem.Get_ActivePlayerBlockingCount() &&
			2u == room.m_ServerNavigation.Get_ActiveBlockerRegionCount() &&
			nullptr != bodyProfile &&
			room.m_ServerCollisionSystem.Sweep_BossCircleAgainstWalls(
				PASSAGE_START_X, 23.f, PASSAGE_Z,
				PASSAGE_END_X, 23.f, PASSAGE_Z,
				bodyProfile->fCollisionRadius, entranceHit),
			"Keep only both intact entrance source boxes and receivers after the real 109 commit");
		const auto strikeEntrance = [&](const char* receiver, const std::uint32_t tick)
		{
			for (const WORLD_DESTRUCTION_BINDING_DESCRIPTOR& binding :
				room.m_WorldDestructionBootstrap.Get_DescriptorGraph().Bindings)
			{
				if (WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT != binding.eTriggerKind ||
					binding.strImpactReceiverId != receiver)
					continue;
				boss.strPatternId = binding.strPatternId;
				boss.strPatternStageId = binding.strStageId;
				boss.strActionId = binding.strActionId;
				boss.iPatternStageIndex = binding.iStageIndex;
				++boss.iPatternSequence;
				bool triggered = false;
				return room.Apply_WorldDestructionImpact(boss, receiver, tick, triggered) && triggered;
			}
			return false;
		};
		WORLD_DESTRUCTION_GROUP_STATE frontAState{};
		const bool frontAStaged = strikeEntrance(FRONT_A, 470u) &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				"destroyable.group.valtan.entrance.frontwallA", frontAState);
		tests.Require(
			frontAStaged && frontAState.iCommitTick > 470u &&
			room.Commit_DueWorldDestruction(frontAState.iCommitTick - 1u) &&
			room.m_ServerCollisionSystem.Is_PlayerBlocking(FRONT_A) &&
			!room.m_ServerCollisionSystem.Is_ImpactReceiverEnabled(FRONT_A) &&
			!room.m_ServerNavigation.Is_PointWalkableExact(145.75f, PASSAGE_Z) &&
			!room.m_ServerNavigation.Has_LineOfSight(145.75f, PASSAGE_Z, 145.75f, PASSAGE_Z),
			"Keep the entrance wall blocked during BREAKING while disabling its impact receiver");
		tests.Require(
			frontAStaged && room.Commit_DueWorldDestruction(frontAState.iCommitTick) &&
			!room.m_ServerCollisionSystem.Is_PlayerBlocking(FRONT_A) &&
			room.m_ServerCollisionSystem.Is_PlayerBlocking(FRONT_B) &&
			2u == room.m_ServerCollisionSystem.Get_ActivePlayerBlockingCount() &&
			room.m_ServerNavigation.Is_PointWalkableExact(145.75f, PASSAGE_Z) &&
			!room.m_ServerNavigation.Is_PointWalkableExact(143.75f, -114.25f),
			"Clear frontwallA source and receiver together while frontwallB keeps overlapping cells closed");
		WORLD_DESTRUCTION_GROUP_STATE frontBState{};
		const bool frontBStaged = strikeEntrance(FRONT_B, 490u) &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				"destroyable.group.valtan.entrance.frontwallB", frontBState);
		const bool everyWallGone = frontBStaged &&
			room.Commit_DueWorldDestruction(frontBState.iCommitTick);
		tests.Require(
			everyWallGone && nullptr != bodyProfile &&
			0u == room.m_ServerCollisionSystem.Get_ActivePlayerBlockingCount() &&
			0u == room.m_ServerNavigation.Get_ActiveBlockerRegionCount() &&
			!room.m_ServerCollisionSystem.Sweep_BossCircleAgainstWalls(
				PASSAGE_START_X, 23.f, PASSAGE_Z,
				PASSAGE_END_X, 23.f, PASSAGE_Z,
				bodyProfile->fCollisionRadius, entranceHit),
			"Leave zero phantom wall colliders and zero wall blockers after all 99 actual wall mutations");
		SERVER_PLAYER passagePlayer{};
		passagePlayer.fPositionX = PASSAGE_START_X;
		passagePlayer.fPositionY = 23.f;
		passagePlayer.fPositionZ = PASSAGE_Z;
		float passageX = 0.f, passageY = 0.f, passageZ = 0.f;
		bool passageBlocked = true;
		SERVER_NAV_POINT passageGround{};
		bool passageClamped = true;
		CPlayerSkillSystem::Clamp_StepToWalkable(
			room.m_ServerNavigation, PASSAGE_START_X, PASSAGE_Z,
			PASSAGE_END_X, PASSAGE_Z, passageGround, passageClamped);
		tests.Require(
			everyWallGone && room.m_ServerCollisionSystem.Resolve_PlayerMove(
				passagePlayer, PASSAGE_END_X, 23.f, PASSAGE_Z,
				passageX, passageY, passageZ, passageBlocked) && !passageBlocked &&
			std::abs(passageX - PASSAGE_END_X) < 0.001f &&
			!passageClamped && std::abs(passageGround.x - PASSAGE_END_X) < 0.001f &&
			room.m_ServerNavigation.Has_LineOfSight(
				PASSAGE_START_X, PASSAGE_Z, PASSAGE_END_X, PASSAGE_Z),
			"Move a player and an authoritative navigation step through the disappeared entrance walls");

		CServerNavigation smoothingNavigation = room.m_ServerNavigation;
		SERVER_NAVIGATION_CONDITION_STAGE smoothingStage{};
		std::vector<SERVER_NAV_POINT> smoothedPassage;
		const bool reclosedFrontA = smoothingNavigation.Prepare_ConditionChanges(
			{ { FRONT_A_CONDITION, false } }, smoothingStage, status);
		if (reclosedFrontA)
			smoothingNavigation.Commit_ConditionChanges(std::move(smoothingStage));
		const bool detourFound = smoothingNavigation.Find_Path(
			PASSAGE_START_X, PASSAGE_Z, PASSAGE_END_X, PASSAGE_Z, smoothedPassage);
		smoothingNavigation.Smooth_Path(
			PASSAGE_START_X, PASSAGE_Z, PASSAGE_END_X, PASSAGE_Z, smoothedPassage);
		bool safeDetour = detourFound && smoothedPassage.size() > 1u;
		float previousX = PASSAGE_START_X, previousZ = PASSAGE_Z;
		for (const SERVER_NAV_POINT& point : smoothedPassage)
		{
			safeDetour = safeDetour && smoothingNavigation.Has_LineOfSight(
				previousX, previousZ, point.x, point.z);
			previousX = point.x;
			previousZ = point.z;
		}
		tests.Require(
			reclosedFrontA && safeDetour && !smoothingNavigation.Has_LineOfSight(
				PASSAGE_START_X, PASSAGE_Z, PASSAGE_END_X, PASSAGE_Z),
			"String-pull player and boss paths around an intact dynamic wall instead of through its baked floor");
		const bool reopenedFrontA = smoothingNavigation.Prepare_ConditionChanges(
			{ { FRONT_A_CONDITION, true } }, smoothingStage, status);
		if (reopenedFrontA)
			smoothingNavigation.Commit_ConditionChanges(std::move(smoothingStage));
		smoothingNavigation.Smooth_Path(
			PASSAGE_START_X, PASSAGE_Z, PASSAGE_END_X, PASSAGE_Z, smoothedPassage);
		tests.Require(
			reopenedFrontA && 1u == smoothedPassage.size() &&
			std::abs(smoothedPassage.back().x - PASSAGE_END_X) < 0.001f,
			"Shorten the same navigation path to a direct segment after its exact wall is destroyed");
		std::vector<SERVER_NAVIGATION_CONDITION_CHANGE> floorChanges;
		for (const WORLD_DESTRUCTION_MUTATION_DESCRIPTOR& mutation :
			room.m_WorldDestructionBootstrap.Get_DescriptorGraph().Mutations)
		{
			if (mutation.bRemovesGround)
				floorChanges.push_back({ mutation.strNavigationStateId, true });
		}
		const bool stagedFloorCollapse = smoothingNavigation.Prepare_ConditionChanges(
			floorChanges, smoothingStage, status);
		if (stagedFloorCollapse)
			smoothingNavigation.Commit_ConditionChanges(std::move(smoothingStage));
		SERVER_NAV_POINT fallenFloorHeight{};
		tests.Require(
			6u == floorChanges.size() && stagedFloorCollapse &&
			6u == smoothingNavigation.Get_ActiveBlockerRegionCount() &&
			smoothingNavigation.Is_PointInVoidRegion(161.75f, -113.75f) &&
			!smoothingNavigation.Is_PointWalkableExact(161.75f, -113.75f) &&
			!smoothingNavigation.Has_LineOfSight(161.75f, -113.75f, 161.75f, -113.75f) &&
			smoothingNavigation.Sample_Position(161.75f, -113.75f, fallenFloorHeight) &&
			!smoothingNavigation.Is_PointWalkableExact(-5.75f, -164.75f),
			"Keep all six collapsed floor regions and base-unwalkable cells closed without losing fall ground samples");
		const std::uint32_t previousEpoch =
			room.m_WorldDestructionRuntime.Get_EncounterEpoch();
		tests.Require(
			room.Reset_ValtanArenaWhenEmpty() &&
			previousEpoch + 1u ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			1u == room.m_iNextWorldDestructionEventSequence,
			"Reset the room live-event ledger only with the encounter epoch");
	}
}

void LostArk::Server::CServerGameplayContractRunner::Run_WaveMonsterButtons(TESTS& tests)
{
	/* Debug F1 "Normal Monster 1/2": the wire request, the Debug room that hands the four
	   wave boxes (Kouku Book1/Book2, Valtan Stage_1/Stage_2) to the buttons, and the
	   Release room that keeps raising them when a player steps in. The room-level cases
	   compile differently per configuration on purpose: Debug proves suppression plus
	   re-summon, Release proves nothing changed. */
	struct WAVE_CASE final
	{
		WORLD_ID eWorld;
		WAVE_MONSTER_BUTTON eButton;
		const char* pTrigger;
		const char* pGroup;
		std::uint32_t iMaxAlive;
	};
	const WAVE_CASE cases[] =
	{
		{ WORLD_ID::KAKULSAYDON_ARENA, WAVE_MONSTER_BUTTON::NORMAL_MONSTER_1, "Book1_Monsters", "spawn.kouku.book1", 22u },
		{ WORLD_ID::KAKULSAYDON_ARENA, WAVE_MONSTER_BUTTON::NORMAL_MONSTER_2, "Book2_Monsters", "spawn.kouku.book2", 15u },
		{ WORLD_ID::VALTAN_ARENA, WAVE_MONSTER_BUTTON::NORMAL_MONSTER_1, "Stage_1", "spawn.valtan.stage01", 10u },
		{ WORLD_ID::VALTAN_ARENA, WAVE_MONSTER_BUTTON::NORMAL_MONSTER_2, "Stage_2", "spawn.valtan.stage03", 10u },
	};
	const auto named = [](const char* what, const char* trigger)
	{
		return std::string(what) + " (" + trigger + ")";
	};

	{
		/* Wire: one new client-to-server packet, appended after the last known one. */
		tests.Require(
			NETWORK_PROTOCOL_VERSION == 100u &&
			Is_Known_Packet_Type(PACKET_TYPE::C2S_DEBUG_RESUMMON_WAVE_MONSTERS) &&
			static_cast<std::uint16_t>(PACKET_TYPE::C2S_DEBUG_RESUMMON_WAVE_MONSTERS) ==
				static_cast<std::uint16_t>(PACKET_TYPE::C2S_SET_EQUIPMENT) + 1u,
			"Wave monster re-summon bumps the protocol from 99 to 100 and the packet type is appended after the last known one");
		bool roundTrips = true;
		for (const WAVE_CASE& c : cases)
		{
			C2S_DEBUG_RESUMMON_WAVE_MONSTERS request{};
			request.iRequestSequence = 41u;
			request.eWorldId = c.eWorld;
			request.eButton = c.eButton;
			CPacketWriter writer;
			C2S_DEBUG_RESUMMON_WAVE_MONSTERS decoded{};
			const bool wrote = Write_Message(writer, request) && 7u == writer.Get_Buffer().size();
			CPacketReader reader{ std::span<const std::uint8_t>(writer.Get_Buffer()) };
			roundTrips = roundTrips && wrote && Read_Message(reader, decoded) &&
				0u == reader.Get_RemainingSize() &&
				decoded.iRequestSequence == 41u && decoded.eWorldId == c.eWorld &&
				decoded.eButton == c.eButton;
		}
		tests.Require(roundTrips,
			"Wave monster re-summon request round-trips as seven bytes for every world and button");

		C2S_DEBUG_RESUMMON_WAVE_MONSTERS request{};
		request.iRequestSequence = 1u;
		request.eWorldId = WORLD_ID::VALTAN_ARENA;
		request.eButton = WAVE_MONSTER_BUTTON::NORMAL_MONSTER_1;
		CPacketWriter scratch;
		C2S_DEBUG_RESUMMON_WAVE_MONSTERS zeroSequence = request;
		zeroSequence.iRequestSequence = 0u;
		C2S_DEBUG_RESUMMON_WAVE_MONSTERS unknownWorld = request;
		unknownWorld.eWorldId = WORLD_ID::END;
		C2S_DEBUG_RESUMMON_WAVE_MONSTERS unknownButton = request;
		unknownButton.eButton = WAVE_MONSTER_BUTTON::END;
		tests.Require(
			!Write_Message(scratch, zeroSequence) && !Write_Message(scratch, unknownWorld) &&
			!Write_Message(scratch, unknownButton),
			"Wave monster re-summon refuses to encode a zero sequence, an unknown world or an unknown button");

		const auto decodeRaw = [](const std::uint32_t sequence, const std::uint16_t world,
			const std::uint8_t button, const bool truncate)
		{
			CPacketWriter raw;
			raw.Write_U32(sequence);
			raw.Write_U16(world);
			if (!truncate)
				raw.Write_U8(button);
			CPacketReader reader{ std::span<const std::uint8_t>(raw.Get_Buffer()) };
			C2S_DEBUG_RESUMMON_WAVE_MONSTERS decoded{};
			return Read_Message(reader, decoded);
		};
		const std::uint16_t valtan = static_cast<std::uint16_t>(WORLD_ID::VALTAN_ARENA);
		tests.Require(
			decodeRaw(1u, valtan, 1u, false) &&
			!decodeRaw(0u, valtan, 1u, false) &&
			!decodeRaw(1u, 0xFFFFu, 1u, false) &&
			!decodeRaw(1u, valtan, 2u, false) &&
			!decodeRaw(1u, valtan, 0xFFu, false) &&
			!decodeRaw(1u, valtan, 0u, true),
			"Wave monster re-summon decoder rejects a zero sequence, an unknown world, an out-of-range button and a truncated payload");
	}

	{
		/* The (world, button) table is the only pairing the Server trusts. */
		bool tableMatches = true;
		for (const WAVE_CASE& c : cases)
		{
			const WAVE_MONSTER_BUTTON_ROW* row =
				CServerTriggerSystem::Find_WaveMonsterButton(c.eWorld, c.eButton);
			tableMatches = tableMatches && nullptr != row &&
				std::string_view(row->pTriggerPlacementId) == c.pTrigger &&
				std::string_view(row->pSpawnGroupId) == c.pGroup;
		}
		tests.Require(tableMatches,
			"Wave monster buttons map Kouku 1/2 to Book1/Book2 and Valtan 1/2 to Stage_1/Stage_2");
		tests.Require(
			nullptr == CServerTriggerSystem::Find_WaveMonsterButton(WORLD_ID::BERN, WAVE_MONSTER_BUTTON::NORMAL_MONSTER_1) &&
			nullptr == CServerTriggerSystem::Find_WaveMonsterButton(WORLD_ID::CHARACTER_SELECT_ARENA, WAVE_MONSTER_BUTTON::NORMAL_MONSTER_2) &&
			nullptr == CServerTriggerSystem::Find_WaveMonsterButton(WORLD_ID::TRAINING_GROUND, WAVE_MONSTER_BUTTON::NORMAL_MONSTER_1) &&
			nullptr == CServerTriggerSystem::Find_WaveMonsterButton(WORLD_ID::VALTAN_ARENA, WAVE_MONSTER_BUTTON::END),
			"Worlds without wave monster buttons and the END button map to nothing");
	}

	/* Trigger system: the suppression flag changes exactly the four boxes and nothing else,
	   measured against the authored bootstrap of both worlds. */
	struct TRIGGER_OUTCOME final
	{
		std::vector<std::string> Activations;
		std::vector<std::string> Prompts;
		std::size_t iTransfers = 0u;
		std::uint32_t iKeyUses = 0u;
		bool bInteract = false;
		bool operator==(const TRIGGER_OUTCOME&) const = default;
	};
	const auto evaluate = [](const WORLD_ID world,
		const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
		const WORLD_BOOTSTRAP_PLACEMENT& at, const bool suppress,
		TRIGGER_OUTCOME& outcome, bool& inside)
	{
		CServerTriggerSystem system;
		system.Set_WorldId(world);
		system.Set_SuppressWaveMonsterTriggers(suppress);
		std::string status;
		if (!system.Initialize(placements, status))
			return false;
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER& player = players[1u];
		player.iPlayerId = 1u;
		player.iNetEntityId = 2u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = player.iMaximumHp = 1000u;
		player.fPositionX = at.fPositionX;
		player.fPositionY = at.fPositionY - WorldCollision::PLAYER_CENTER_OFFSET_Y;
		player.fPositionZ = at.fPositionZ;
		inside = CServerTriggerSystem::Contains_Placement(at, player);
		const auto activate = [&outcome](const WORLD_TRIGGER_ACTION_KIND kind, const std::string& target)
		{
			outcome.Activations.push_back(std::to_string(static_cast<int>(kind)) + ":" + target);
			return true;
		};
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> edges;
		system.Evaluate_Entries(players, 100u, transfers, activate, edges);
		for (const SERVER_INTERACT_PROMPT_EDGE& edge : edges)
			outcome.Prompts.push_back(edge.strTriggerPlacementId + (edge.bAvailable ? "+" : "-"));
		outcome.iTransfers = transfers.size();
		outcome.iKeyUses = system.Activate_Here(1u, players, 200u, transfers, activate);
		outcome.bInteract = system.Activate_Interact(1u, at.strPlacementId, players, 300u, transfers, activate);
		return true;
	};
	const std::string spawnGroupKind =
		std::to_string(static_cast<int>(WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP)) + ":";
	for (const WORLD_ID world : { WORLD_ID::KAKULSAYDON_ARENA, WORLD_ID::VALTAN_ARENA })
	{
		auto roomStorage = std::make_unique<CGameRoom>(world);
		CGameRoom& room = *roomStorage;
		const auto& placements = room.m_WorldBootstrap.Get_Placements();
		bool everyOtherBoxUnchanged = room.Is_Ready();
		bool everyWaveBoxQuiet = true;
		bool everyGatedWaveBoxQuiet = true;
		std::size_t waveBoxes = 0u;
		std::size_t otherBoxes = 0u;
		for (const WORLD_BOOTSTRAP_PLACEMENT& placement : placements)
		{
			if (!placement.isEnabled || WORLD_BOOTSTRAP_KIND::TRIGGER_BOX != placement.eKind)
				continue;
			TRIGGER_OUTCOME plain;
			TRIGGER_OUTCOME held;
			bool insidePlain = false;
			bool insideHeld = false;
			if (!evaluate(world, placements, placement, false, plain, insidePlain) ||
				!evaluate(world, placements, placement, true, held, insideHeld) ||
				!insidePlain || !insideHeld)
			{
				if (CServerTriggerSystem::Is_WaveMonsterTrigger(world, placement))
					everyWaveBoxQuiet = false;
				continue;
			}
			if (!CServerTriggerSystem::Is_WaveMonsterTrigger(world, placement))
			{
				++otherBoxes;
				everyOtherBoxUnchanged = everyOtherBoxUnchanged && plain == held;
				continue;
			}
			++waveBoxes;
			const std::string wave = spawnGroupKind + placement.TriggerActions.front().strTargetId;
			TRIGGER_OUTCOME expected = plain;
			const std::size_t before = expected.Activations.size();
			expected.Activations.erase(
				std::remove(expected.Activations.begin(), expected.Activations.end(), wave),
				expected.Activations.end());
			everyWaveBoxQuiet = everyWaveBoxQuiet &&
				before != expected.Activations.size() && expected == held &&
				std::find(held.Activations.begin(), held.Activations.end(), wave) == held.Activations.end();

			/* The same box authored as G-only: without the flag it offers itself and G raises
			   the group; with the flag it offers nothing and G cannot raise it either. */
			std::vector<WORLD_BOOTSTRAP_PLACEMENT> gatedPlacements = placements;
			WORLD_BOOTSTRAP_PLACEMENT gatedBox = placement;
			gatedBox.requiresInteract = true;
			for (WORLD_BOOTSTRAP_PLACEMENT& candidate : gatedPlacements)
				if (candidate.strPlacementId == placement.strPlacementId)
					candidate = gatedBox;
			TRIGGER_OUTCOME gatedPlain;
			TRIGGER_OUTCOME gatedHeld;
			bool insideGatedPlain = false;
			bool insideGatedHeld = false;
			const bool gatedRan =
				evaluate(world, gatedPlacements, gatedBox, false, gatedPlain, insideGatedPlain) &&
				evaluate(world, gatedPlacements, gatedBox, true, gatedHeld, insideGatedHeld);
			const auto raises = [&wave](const TRIGGER_OUTCOME& outcome)
			{
				return std::find(outcome.Activations.begin(), outcome.Activations.end(), wave) !=
					outcome.Activations.end();
			};
			const auto offers = [&placement](const TRIGGER_OUTCOME& outcome)
			{
				return std::find(outcome.Prompts.begin(), outcome.Prompts.end(),
					placement.strPlacementId + "+") != outcome.Prompts.end();
			};
			everyGatedWaveBoxQuiet = everyGatedWaveBoxQuiet && gatedRan &&
				insideGatedPlain && insideGatedHeld &&
				offers(gatedPlain) && raises(gatedPlain) && gatedPlain.bInteract &&
				!offers(gatedHeld) && !raises(gatedHeld) && !gatedHeld.bInteract;
		}
		const std::string worldName = WORLD_ID::VALTAN_ARENA == world ? "Valtan" : "Kouku";
		tests.Require(2u == waveBoxes && everyWaveBoxQuiet,
			(worldName + ": with the flag set, stepping into or pressing G in the two wave boxes raises nothing and offers nothing, while flag off raises their spawn group").c_str());
		tests.Require(2u == waveBoxes && everyGatedWaveBoxQuiet,
			(worldName + ": a wave box authored as G-only offers and raises its group without the flag, and with the flag offers nothing and G cannot raise it").c_str());
		tests.Require(otherBoxes > 0u && everyOtherBoxUnchanged,
			(worldName + ": the flag leaves every other authored trigger box's entry, prompt, G and transfer outcome identical").c_str());
	}

	/* Room level. Every case stands one player in the wave box of a real room. */
	static constexpr SESSION_ID SESSION = 9401u;
	static constexpr PLAYER_ID PLAYER = 94001u;
	const auto addPlayer = [](CGameRoom& room, const float x, const float y, const float z)
	{
		SERVER_PLAYER player{};
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 94002u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = player.iMaximumHp = 1000000000u;
		player.isCombatReady = true;
		player.fPositionX = x;
		player.fPositionY = y;
		player.fPositionZ = z;
		room.m_Players.emplace(player.iPlayerId, player);
		room.m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
		room.m_PlayerIdBySessionId.emplace(SESSION, player.iPlayerId);
	};
	const auto tickRoom = [](CGameRoom& room, const int ticks)
	{
		for (int tick = 0; tick < ticks && room.Is_Ready(); ++tick)
			room.Tick(1.f / 30.f);
	};

	for (const WAVE_CASE& c : cases)
	{
		auto stepRoom = std::make_unique<CGameRoom>(c.eWorld);
		const WORLD_BOOTSTRAP_PLACEMENT* box = stepRoom->Find_Placement(c.pTrigger);
		const auto& groups = stepRoom->m_SpawnGroupBootstrap.Get_Groups();
		const auto definition = std::find_if(groups.begin(), groups.end(),
			[&c](const SPAWN_GROUP_DEFINITION& group) { return group.strSpawnGroupId == c.pGroup; });
		const bool dataPresent = stepRoom->Is_Ready() && nullptr != box &&
			groups.end() != definition && definition->iMaxAlive == c.iMaxAlive &&
			WORLD_BOOTSTRAP_KIND::TRIGGER_BOX == box->eKind && box->isEnabled &&
			1u == box->TriggerActions.size() &&
			WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP == box->TriggerActions.front().eKind &&
			box->TriggerActions.front().strTargetId == c.pGroup;
		tests.Require(dataPresent,
			named("Authored wave box is an enabled activateSpawnGroup on the mapped group with the tooltip's max alive", c.pTrigger).c_str());
		if (!dataPresent)
			continue;

		addPlayer(*stepRoom, box->fPositionX, box->fPositionY - WorldCollision::PLAYER_CENTER_OFFSET_Y, box->fPositionZ);
		const bool standingInside =
			CServerTriggerSystem::Contains_Placement(*box, stepRoom->m_Players.at(PLAYER));
		tests.Require(standingInside,
			named("Fixture player stands inside the wave box", c.pTrigger).c_str());
		tickRoom(*stepRoom, 45);
		const std::string group = c.pGroup;
#ifdef _DEBUG
		tests.Require(
			!stepRoom->m_SpawnGroupRuntime.Is_ActiveOrCompleted(group) &&
			0u == stepRoom->Count_SpawnGroupEntities(group),
			named("Debug room: a player standing in the wave box does not raise the wave", c.pTrigger).c_str());
		/* Control: the same player in the same room raises it once the flag is cleared, so the
		   silence above came from the suppression and not from the player missing the box. */
		stepRoom->m_ServerTriggerSystem.Set_SuppressWaveMonsterTriggers(false);
		tickRoom(*stepRoom, 45);
		tests.Require(
			stepRoom->m_SpawnGroupRuntime.Is_ActiveOrCompleted(group) &&
			stepRoom->Count_SpawnGroupEntities(group) > 0u,
			named("Debug room control: clearing the flag lets the same player raise the wave", c.pTrigger).c_str());
#else
		tests.Require(
			stepRoom->m_SpawnGroupRuntime.Is_ActiveOrCompleted(group) &&
			stepRoom->Count_SpawnGroupEntities(group) > 0u &&
			stepRoom->Count_SpawnGroupEntities(group) <= c.iMaxAlive,
			named("Release room: a player stepping into the wave box raises the wave as before", c.pTrigger).c_str());
#endif
	}

	/* Buttons. The player waits at the world's first authored spawn, away from every wave box. */
	for (const WORLD_ID world : { WORLD_ID::KAKULSAYDON_ARENA, WORLD_ID::VALTAN_ARENA })
	{
		auto roomStorage = std::make_unique<CGameRoom>(world);
		CGameRoom& room = *roomStorage;
		const WAVE_CASE* first = nullptr;
		const WAVE_CASE* second = nullptr;
		for (const WAVE_CASE& c : cases)
		{
			if (c.eWorld != world)
				continue;
			(nullptr == first ? first : second) = &c;
		}
		const auto& placements = room.m_WorldBootstrap.Get_Placements();
		const auto spawn = std::find_if(placements.begin(), placements.end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return placement.isEnabled && WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind;
			});
		const bool ready = room.Is_Ready() && placements.end() != spawn &&
			nullptr != first && nullptr != second;
		tests.Require(ready, "Button fixture room is ready with an authored player spawn");
		if (!ready)
			continue;
		addPlayer(room, spawn->fPositionX, spawn->fPositionY, spawn->fPositionZ);
		const std::string worldName = WORLD_ID::VALTAN_ARENA == world ? "Valtan" : "Kouku";
		bool awayFromBoxes = true;
		for (const WAVE_CASE* c : { first, second })
			awayFromBoxes = awayFromBoxes && !CServerTriggerSystem::Contains_Placement(
				*room.Find_Placement(c->pTrigger), room.m_Players.at(PLAYER));
		tests.Require(awayFromBoxes, (worldName + ": button fixture player is outside both wave boxes").c_str());

		const auto press = [&room](const WAVE_MONSTER_BUTTON button, const WORLD_ID requestWorld)
		{
			C2S_DEBUG_RESUMMON_WAVE_MONSTERS request{};
			request.iRequestSequence = 1u;
			request.eWorldId = requestWorld;
			request.eButton = button;
			room.Handle_DebugResummonWaveMonsters(SESSION, request);
		};
		const std::string groupOne = first->pGroup;
		const std::string groupTwo = second->pGroup;
		const auto groupIds = [&room](const std::string& group)
		{
			std::set<NET_ENTITY_ID> ids;
			for (const SERVER_WORLD_ENTITY& entity : room.m_WorldEntities)
				if (WORLD_BOOTSTRAP_KIND::MONSTER == entity.eKind && entity.strSpawnGroupId == group)
					ids.insert(entity.iNetEntityId);
			return ids;
		};
		tickRoom(room, 30);
		tests.Require(
			!room.m_SpawnGroupRuntime.Is_ActiveOrCompleted(groupOne) &&
			!room.m_SpawnGroupRuntime.Is_ActiveOrCompleted(groupTwo) &&
			0u == room.Count_SpawnGroupEntities(groupOne) && 0u == room.Count_SpawnGroupEntities(groupTwo),
			(worldName + ": both wave groups start dormant with no monsters while nobody steps in and nobody presses").c_str());

#ifdef _DEBUG
		/* Two bystanders the press must never remove: a monster of another group and a
		non-monster entity carrying this group's id. They are present only across a press
		(never across a Tick), so the monster brain never simulates them. */
		const auto injectBystanders = [&room](const std::string& group)
		{
			SERVER_WORLD_ENTITY otherGroup{};
			otherGroup.iNetEntityId = 94900u;
			otherGroup.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
			otherGroup.strSpawnGroupId = "spawn.test.foreign";
			SERVER_WORLD_ENTITY sameGroupNpc{};
			sameGroupNpc.iNetEntityId = 94901u;
			sameGroupNpc.eKind = WORLD_BOOTSTRAP_KIND::NPC;
			sameGroupNpc.strSpawnGroupId = group;
			room.m_WorldEntities.push_back(otherGroup);
			room.m_WorldEntities.push_back(sameGroupNpc);
		};
		const auto bystandersPresent = [&room]()
		{
			return std::count_if(room.m_WorldEntities.begin(), room.m_WorldEntities.end(),
				[](const SERVER_WORLD_ENTITY& entity) { return 94900u == entity.iNetEntityId || 94901u == entity.iNetEntityId; });
		};
		const auto removeBystanders = [&room]()
		{
			std::erase_if(room.m_WorldEntities,
				[](const SERVER_WORLD_ENTITY& entity) { return 94900u == entity.iNetEntityId || 94901u == entity.iNetEntityId; });
		};
		/* Refusals change nothing. */
		const WORLD_ID otherWorld = WORLD_ID::VALTAN_ARENA == world ? WORLD_ID::KAKULSAYDON_ARENA : WORLD_ID::VALTAN_ARENA;
		press(first->eButton, otherWorld);
		press(WAVE_MONSTER_BUTTON::END, world);
		{
			C2S_DEBUG_RESUMMON_WAVE_MONSTERS request{};
			request.iRequestSequence = 1u;
			request.eWorldId = world;
			request.eButton = first->eButton;
			room.Handle_DebugResummonWaveMonsters(SESSION + 1u, request);
		}
		tests.Require(
			!room.m_SpawnGroupRuntime.Is_ActiveOrCompleted(groupOne) &&
			0u == room.Count_SpawnGroupEntities(groupOne),
			(worldName + ": a request naming another world, an unknown button or a session without a player is refused").c_str());
		if (WORLD_ID::VALTAN_ARENA == world)
		{
			room.m_ValtanTimelineAudition.ePhase = CGameRoom::VALTAN_TIMELINE_AUDITION_PHASE::READY;
			press(first->eButton, world);
			room.m_ValtanTimelineAudition.ePhase = CGameRoom::VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE;
			tests.Require(!room.m_SpawnGroupRuntime.Is_ActiveOrCompleted(groupOne),
				"Valtan: the button is refused while the pattern audition owns the arena");
		}

		/* Accepted: the group starts and its monsters appear at the authored anchors. */
		injectBystanders(groupOne);
		press(first->eButton, world);
		tests.Require(room.m_SpawnGroupRuntime.Is_ActiveOrCompleted(groupOne) &&
			!room.m_SpawnGroupRuntime.Is_ActiveOrCompleted(groupTwo) && 2 == bystandersPresent(),
			(worldName + ": Normal Monster 1 starts only its own group and removes no bystander").c_str());
		removeBystanders();
		tickRoom(room, 60);
		const std::set<NET_ENTITY_ID> firstRun = groupIds(groupOne);
		bool atAnchors = !firstRun.empty();
		const auto groupDefinition = std::find_if(room.m_SpawnGroupBootstrap.Get_Groups().begin(),
			room.m_SpawnGroupBootstrap.Get_Groups().end(),
			[&groupOne](const SPAWN_GROUP_DEFINITION& group) { return group.strSpawnGroupId == groupOne; });
		for (const SERVER_WORLD_ENTITY& entity : room.m_WorldEntities)
		{
			if (entity.strSpawnGroupId != groupOne)
				continue;
			float nearest = (std::numeric_limits<float>::max)();
			for (const SPAWN_GROUP_WAVE& wave : groupDefinition->Waves)
				for (const SPAWN_GROUP_ENTRY& entry : wave.Entries)
					if (const SPAWN_GROUP_ANCHOR* anchor = room.m_SpawnGroupBootstrap.Find_Anchor(entry.strAnchorId))
						nearest = (std::min)(nearest, std::hypot(
							entity.fSpawnPositionX - anchor->fPositionX, entity.fSpawnPositionZ - anchor->fPositionZ));
			atAnchors = atAnchors && nearest <= 6.f;
		}
		tests.Require(
			!firstRun.empty() && firstRun.size() <= first->iMaxAlive && atAnchors &&
			0u == room.Count_SpawnGroupEntities(groupTwo),
			(worldName + ": the summoned monsters stay within max alive at the group's authored anchors, wherever the player stands, and the other group stays empty").c_str());

		/* Re-summon with survivors: the old monsters go, a fresh wave starts from the beginning. */
		injectBystanders(groupOne);
		press(first->eButton, world);
		const std::set<NET_ENTITY_ID> straightAfter = groupIds(groupOne);
		tests.Require(straightAfter.empty() && room.m_SpawnGroupRuntime.Is_ActiveOrCompleted(groupOne) &&
			!room.m_SpawnGroupRuntime.Is_Completed(groupOne) && 2 == bystandersPresent(),
			(worldName + ": pressing again removes every surviving monster of the group, restarts it and leaves the bystanders").c_str());
		removeBystanders();
		tickRoom(room, 60);
		const std::set<NET_ENTITY_ID> secondRun = groupIds(groupOne);
		bool disjoint = !secondRun.empty();
		for (const NET_ENTITY_ID id : secondRun)
			disjoint = disjoint && !firstRun.contains(id);
		tests.Require(disjoint && secondRun.size() <= first->iMaxAlive,
			(worldName + ": the re-summoned wave is made of new monsters within max alive").c_str());

		/* A finished ONCE group is summoned again as well. */
		bool completed = false;
		for (int tick = 0; tick < 1800 && !completed && room.Is_Ready(); ++tick)
		{
			for (auto entity = room.m_WorldEntities.begin(); entity != room.m_WorldEntities.end();)
				entity = (WORLD_BOOTSTRAP_KIND::MONSTER == entity->eKind && entity->strSpawnGroupId == groupOne) ?
					room.m_WorldEntities.erase(entity) : std::next(entity);
			room.Tick(1.f / 30.f);
			completed = room.m_SpawnGroupRuntime.Is_Completed(groupOne);
		}
		tests.Require(completed,
			(worldName + ": fixture drives the group to COMPLETED by clearing its monsters").c_str());
		press(first->eButton, world);
		tickRoom(room, 60);
		tests.Require(
			room.m_SpawnGroupRuntime.Is_ActiveOrCompleted(groupOne) && !room.m_SpawnGroupRuntime.Is_Completed(groupOne) &&
			room.Count_SpawnGroupEntities(groupOne) > 0u,
			(worldName + ": pressing a COMPLETED group summons its wave again").c_str());

		/* Normal Monster 2 is its own group, independent of the first. */
		const std::set<NET_ENTITY_ID> firstBeforeSecond = groupIds(groupOne);
		press(second->eButton, world);
		tickRoom(room, 60);
		tests.Require(
			!groupIds(groupTwo).empty() && groupIds(groupTwo).size() <= second->iMaxAlive &&
			firstBeforeSecond.size() == groupIds(groupOne).size() &&
			std::all_of(firstBeforeSecond.begin(), firstBeforeSecond.end(),
				[&groupIds, &groupOne](const NET_ENTITY_ID id) { return groupIds(groupOne).contains(id); }),
			(worldName + ": Normal Monster 2 summons its own group and leaves the first group's monsters alone").c_str());
#else
		/* Release: the button does nothing. */
		press(first->eButton, world);
		press(second->eButton, world);
		tickRoom(room, 60);
		tests.Require(
			!room.m_SpawnGroupRuntime.Is_ActiveOrCompleted(groupOne) &&
			!room.m_SpawnGroupRuntime.Is_ActiveOrCompleted(groupTwo) &&
			0u == room.Count_SpawnGroupEntities(groupOne) && 0u == room.Count_SpawnGroupEntities(groupTwo),
			(worldName + ": Release ignores the wave monster button request").c_str());
#endif
	}

#ifdef _DEBUG
	/* Only the four boxes are handed over: the Valtan mini boss box still fires when stepped on. */
	{
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		const WORLD_BOOTSTRAP_PLACEMENT* miniBoss = room.Find_Placement("Stage_MiniBoss_Spawn");
		const bool present = room.Is_Ready() && nullptr != miniBoss && 1u == miniBoss->TriggerActions.size();
		tests.Require(present, "Valtan mini boss spawn box is authored");
		if (present)
		{
			const std::string group = miniBoss->TriggerActions.front().strTargetId;
			addPlayer(room, miniBoss->fPositionX, miniBoss->fPositionY - WorldCollision::PLAYER_CENTER_OFFSET_Y, miniBoss->fPositionZ);
			tickRoom(room, 45);
			tests.Require(
				CServerTriggerSystem::Contains_Placement(*miniBoss, room.m_Players.at(PLAYER)) &&
				room.m_SpawnGroupRuntime.Is_ActiveOrCompleted(group),
				"Debug room: the Stage_MiniBoss_Spawn box still raises its group when stepped on (not handed to the buttons)");
		}
	}
#endif
}
