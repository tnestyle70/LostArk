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
		raidRoom.m_ServerTriggerSystem.Evaluate_Entries(
			raidRoom.m_Players,
			700u,
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
		/* The 109 batch is the thirty outer ring walls plus every interior
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
			applied && 97u == breakingCount && 67u == interiorBreakingCount &&
			0u == outsideBreakingCount &&
			98u == room.m_iNextWorldDestructionEventSequence,
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
			builtFirst && builtRepeated && 97u == firstEvents.size() &&
			firstEvents.size() == repeatedEvents.size() &&
			1u == firstEvents.front().iEventSequence &&
			97u == firstEvents.back().iEventSequence &&
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

		/* The real 109 commit clears 97 walls. The two independent entrance
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

