#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerCombatHitRuntime.h"
#include "ServerTriggerSystem.h"
#include "WorldBootstrap.h"
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

int LostArk::Server::CServerGameplayContractRunner::Run_DebugTeleport(TESTS& tests)
{
		for (const WORLD_ID world : { WORLD_ID::KAKULSAYDON_ARENA, WORLD_ID::VALTAN_ARENA })
		{
			auto room = std::make_unique<CGameRoom>(world);
			const auto* spawn = room->Find_AvailablePlayerSpawn();
			SERVER_NAV_POINT ground{};
			const bool ready = room->Is_Ready() && nullptr != spawn &&
				room->m_ServerNavigation.Sample_Position(spawn->fPositionX, spawn->fPositionZ, ground);
			tests.Require(ready, "Teleport loads current world navigation and authored spawn");
			if (!ready) continue;
			auto storage = std::make_unique<SERVER_PLAYER>();
			auto& player = *storage;
			player.iPlayerId = 123u;
			player.iNetEntityId = 456u;
			player.iCurrentHp = player.iMaximumHp = 100u;
			player.fPositionX = ground.x + 10.f;
			player.fPositionY = ground.y;
			player.fPositionZ = ground.z;
			player.hasMoveGoal = true;
			player.MovePath.push_back(ground);
			player.iCurrentSkillId = 34010u;
			player.eAction = PLAYER_ACTION_STATE::SKILL;
			C2S_DEBUG_TELEPORT_TO_POSITION request{};
			request.iRequestSequence = 1u;
			request.eWorldId = world;
			request.fPositionX = ground.x;
			request.fPositionY = ground.y + 100.f;
			request.fPositionZ = ground.z;
			auto verdict = room->Apply_DebugTeleportToPosition(player, request);
#ifndef _DEBUG
			tests.Require(verdict.eResult == DEBUG_TELEPORT_RESULT::REJECTED_DISABLED &&
				player.hasMoveGoal && player.iCurrentSkillId == 34010u,
				"Release refuses teleport without changing player actions");
#else
			tests.Require(verdict.eResult == DEBUG_TELEPORT_RESULT::REJECTED_HEIGHT &&
				player.hasMoveGoal && player.MovePath.size() == 1u &&
				player.iCurrentSkillId == 34010u && player.fPositionX == ground.x + 10.f,
				"Wrong picked deck preserves transform, skill and path");
			request.iRequestSequence = 2u;
			request.fPositionY = ground.y + 0.1f;
			verdict = room->Apply_DebugTeleportToPosition(player, request);
			tests.Require(verdict.eResult == DEBUG_TELEPORT_RESULT::ACCEPTED &&
				verdict.iRequestSequence == 2u && verdict.eWorldId == world &&
				verdict.fPositionY == ground.y && player.fPositionX == ground.x &&
				player.fPositionY == ground.y && player.fPositionZ == ground.z &&
				!player.hasMoveGoal && player.MovePath.empty() &&
				player.iCurrentSkillId == INVALID_SKILL_ID && player.iCurrentHp == 100u,
				"Accepted teleport uses Server ground and clears only own transient action");
			player.fPositionX += 2.f;
			const auto duplicate = room->Apply_DebugTeleportToPosition(player, request);
			tests.Require(duplicate.eResult == DEBUG_TELEPORT_RESULT::ACCEPTED &&
				duplicate.fPositionX == ground.x && player.fPositionX == ground.x + 2.f,
				"Duplicate request replays verdict without applying teleport twice");
			request.iRequestSequence = 1u;
			verdict = room->Apply_DebugTeleportToPosition(player, request);
			tests.Require(verdict.eResult == DEBUG_TELEPORT_RESULT::REJECTED_STALE_SEQUENCE &&
				player.fPositionX == ground.x + 2.f,
				"Stale sequence cannot reset position");
			request.iRequestSequence = 3u;
			request.eWorldId = WORLD_ID::BERN;
			verdict = room->Apply_DebugTeleportToPosition(player, request);
			tests.Require(verdict.eResult == DEBUG_TELEPORT_RESULT::REJECTED_WRONG_WORLD,
				"Cross-world teleport intent is refused");
			request.eWorldId = world;
			request.fPositionX = 100001.f;
			verdict = room->Apply_DebugTeleportToPosition(player, request);
			tests.Require(verdict.eResult == DEBUG_TELEPORT_RESULT::REJECTED_INVALID_POSITION,
				"Invalid coordinates rejected before navigation cell conversion");
			request.iRequestSequence = 4u;
			request.fPositionX = ground.x;
			player.iCurrentHp = 0u;
			verdict = room->Apply_DebugTeleportToPosition(player, request);
			tests.Require(verdict.eResult == DEBUG_TELEPORT_RESULT::REJECTED_PLAYER_STATE &&
				player.iCurrentHp == 0u, "Debug movement cannot revive a dead player");
			player.iCurrentHp = 100u;
			request.iRequestSequence = 5u;
			player.bPatternBound = true;
			player.hasMoveGoal = true;
			player.iCurrentSkillId = 34010u;
			verdict = room->Apply_DebugTeleportToPosition(player, request);
			tests.Require(verdict.eResult == DEBUG_TELEPORT_RESULT::REJECTED_PLAYER_STATE &&
				player.bPatternBound && player.hasMoveGoal && player.iCurrentSkillId == 34010u,
				"Bound player cannot bypass capture by sending a teleport packet");
			player.bPatternBound = false;
			request.iRequestSequence = 6u;
			SERVER_WORLD_ENTITY blocker{};
			blocker.iNetEntityId = 789u;
			blocker.eKind = WORLD_BOOTSTRAP_KIND::NPC;
			blocker.fPositionX = ground.x;
			blocker.fPositionY = ground.y;
			blocker.fPositionZ = ground.z;
			room->m_WorldEntities.push_back(blocker);
			verdict = room->Apply_DebugTeleportToPosition(player, request);
			tests.Require(verdict.eResult == DEBUG_TELEPORT_RESULT::REJECTED_COLLISION &&
				player.fPositionX == ground.x + 2.f,
				"Destination overlap refuses teleport into current world entity");
			if (world == WORLD_ID::VALTAN_ARENA)
			{
				const auto previousEntityCount = room->m_WorldEntities.size();
				verdict = room->Apply_DebugReturnToKoukuStart(player, 7u);
				tests.Require(verdict.eResult == DEBUG_TELEPORT_RESULT::REJECTED_WRONG_WORLD && room->m_WorldEntities.size() == previousEntityCount &&
					player.fPositionX == ground.x + 2.f, "Kouku arena start cannot reset another world");
			}
#endif
		}
#ifdef _DEBUG
		{
			auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
			const auto* spawn = room->Find_Placement("player.spawn.kakul.party01");
			SERVER_NAV_POINT start{};
			const bool ready = room->Is_Ready() && spawn && room->m_ServerNavigation.Sample_Position(spawn->fPositionX, spawn->fPositionZ, start);
			tests.Require(ready, "Arena start resolves the authored stable spawn ID");
			if (ready)
			{
				auto& player = room->m_Players[123u];
				player.iPlayerId = 123u; player.iNetEntityId = 456u;
				player.iCurrentHp = player.iMaximumHp = 100u;
				player.fPositionX = start.x + 20.f; player.fPositionY = start.y; player.fPositionZ = start.z;
				player.hasMoveGoal = true; player.MovePath.push_back(start);
				auto& peer = room->m_Players[124u]; peer.iPlayerId = 124u; peer.iNetEntityId = 457u;
				peer.iCurrentHp = peer.iMaximumHp = 100u;
				peer.fPositionX = start.x + 30.f; peer.fPositionY = start.y; peer.fPositionZ = start.z;
				const auto initialBoss = std::find_if(room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
					[](const auto& entity) { return entity.eKind == WORLD_BOOTSTRAP_KIND::BOSS && !entity.isEstherSummon; });
				tests.Require(initialBoss != room->m_WorldEntities.end(), "Arena start fixture includes the statically enabled boss");
				const auto ownerId = initialBoss != room->m_WorldEntities.end() ? initialBoss->iNetEntityId : INVALID_NET_ENTITY_ID;
				SERVER_WORLD_ENTITY dependent{}; dependent.iNetEntityId = 987650u;
				dependent.eKind = WORLD_BOOTSTRAP_KIND::MONSTER; dependent.iOwnerBossNetEntityId = ownerId;
				dependent.fPositionX = start.x + 100.f; dependent.fPositionZ = start.z + 100.f;
				room->m_WorldEntities.insert(room->m_WorldEntities.begin(), dependent);
				SERVER_WORLD_ENTITY unrelated = dependent; unrelated.iNetEntityId = 987651u;
				unrelated.eKind = WORLD_BOOTSTRAP_KIND::NPC; unrelated.iOwnerBossNetEntityId = INVALID_NET_ENTITY_ID;
				room->m_WorldEntities.push_back(unrelated);
				SERVER_WORLD_ENTITY esther = dependent; esther.iNetEntityId = 987652u;
				esther.eKind = WORLD_BOOTSTRAP_KIND::BOSS; esther.isEstherSummon = true;
				room->m_WorldEntities.push_back(esther);
				const auto& placements = room->m_WorldBootstrap.Get_Placements();
				const auto once = std::find_if(placements.begin(), placements.end(), [](const auto& value) {
					return value.isEnabled && value.isTriggerOnce && value.TriggerActions.size() == 1u &&
						value.TriggerActions.front().eKind == WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE; });
				tests.Require(once != placements.end(), "Arena start fixture has an authored once-only sequence trigger");
				std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
				const auto acceptSequence = [](WORLD_TRIGGER_ACTION_KIND, const std::string&) { return true; };
				if (once != placements.end())
				{
					tests.Require(room->m_ServerTriggerSystem.Debug_Activate(123u, once->strPlacementId, false, room->m_Players, 1u, transfers, acceptSequence) == DEBUG_WORLD_PLAYBACK_RESULT::ACCEPTED,
						"Authored entry trigger fires before reset");
					tests.Require(room->m_ServerTriggerSystem.Debug_Activate(123u, once->strPlacementId, false, room->m_Players, 2u, transfers, acceptSequence) == DEBUG_WORLD_PLAYBACK_RESULT::ALREADY_USED,
						"Once trigger is spent before arena reset");
				}
				player.iCurrentHp = 0u;
				const auto countBeforeReject = room->m_WorldEntities.size();
				auto verdict = room->Apply_DebugReturnToKoukuStart(player, 1u);
				tests.Require(verdict.eResult == DEBUG_TELEPORT_RESULT::REJECTED_PLAYER_STATE && room->m_WorldEntities.size() == countBeforeReject &&
					player.fPositionX == start.x + 20.f && player.hasMoveGoal, "Rejected arena reset preserves bosses, position and movement");
				player.iCurrentHp = 100u;
				verdict = room->Apply_DebugReturnToKoukuStart(player, 2u);
				tests.Require(verdict.eResult == DEBUG_TELEPORT_RESULT::ACCEPTED && player.fPositionX == start.x && player.fPositionY == start.y &&
					player.fPositionZ == start.z && !player.hasMoveGoal && player.MovePath.empty(), "Arena start uses Server spawn and resets only requesting player movement");
				tests.Require(peer.fPositionX == start.x + 30.f && peer.fPositionY == start.y && peer.fPositionZ == start.z,
					"Arena reset preserves another player's world position");
				tests.Require(std::none_of(room->m_WorldEntities.begin(), room->m_WorldEntities.end(), [](const auto& entity) {
					return (entity.eKind == WORLD_BOOTSTRAP_KIND::BOSS && !entity.isEstherSummon) || entity.iNetEntityId == 987650u; }),
					"Arena start removes static bosses and earlier-stored owned dependents");
				tests.Require(std::any_of(room->m_WorldEntities.begin(), room->m_WorldEntities.end(), [](const auto& entity) { return entity.iNetEntityId == 987651u; }) &&
					std::any_of(room->m_WorldEntities.begin(), room->m_WorldEntities.end(), [](const auto& entity) { return entity.iNetEntityId == 987652u; }),
					"Arena start preserves unrelated NPC and Esther entities");
				if (once != placements.end())
					tests.Require(room->m_ServerTriggerSystem.Debug_Activate(123u, once->strPlacementId, false, room->m_Players, 3u, transfers, acceptSequence) == DEBUG_WORLD_PLAYBACK_RESULT::ACCEPTED,
						"Arena start rearms the actual once-only entry sequence trigger");
				player.fPositionX += 2.f;
				verdict = room->Apply_DebugReturnToKoukuStart(player, 2u);
				tests.Require(verdict.eResult == DEBUG_TELEPORT_RESULT::ACCEPTED && player.fPositionX == start.x + 2.f,
					"Duplicate arena-start request replays approval without a second reset");
				if (once != placements.end())
					tests.Require(room->m_ServerTriggerSystem.Debug_Activate(123u, once->strPlacementId, false, room->m_Players, 4u, transfers, acceptSequence) == DEBUG_WORLD_PLAYBACK_RESULT::ALREADY_USED,
						"Duplicate arena-start request does not rearm a newly consumed trigger");
			}
		}
#endif
		for (const char* entranceId : { "Mario1_go", "Mario2_go", "Mario3_go", "Mario4_go" })
		{
			const std::uint8_t stage = static_cast<std::uint8_t>(entranceId[5] - '0');
			auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
			const auto* entrance = room->Find_Placement(entranceId);
			tests.Require(room->Is_Ready() && nullptr != entrance &&
				!entrance->TriggerActions.empty(), "Mario jump loads authored entrance");
			if (!room->Is_Ready() || nullptr == entrance || entrance->TriggerActions.empty()) continue;
			const std::string sourceGroup = "spawn.mario" + std::to_string(stage) + ".source";
			const auto layout = room->Begin_MarioStageObjects(stage);
			tests.Require(layout >= 1u && layout <= 3u, "Mario source entry selects a bounded original layout");
			room->m_SpawnGroupRuntime.Update(1.f / 30.f, room->m_SpawnGroupBootstrap,
				[&](const std::string& id) { return room->Count_SpawnGroupEntities(id); },
				[&](const auto& id, const auto& entry, const auto& anchor, const auto& profile, auto ordinal) {
					return room->Spawn_Monster(id, entry, anchor, profile, ordinal);
				});
			const unsigned expectedSourceCounts[] = {0u, 3u, 7u, 14u, 16u};
			tests.Require(room->Count_SpawnGroupEntities(sourceGroup) == expectedSourceCounts[stage],
				"Mario source entry spawns every authored monster exactly once");
			for (const auto& spawned : room->m_WorldEntities)
			{
				if (spawned.strSpawnGroupId != sourceGroup) continue;
				tests.Require(spawned.iCurrentHp == 1u && spawned.iMarioPatrolStage == stage &&
					std::abs(std::hypot(spawned.fMarioPatrolAxisX, spawned.fMarioPatrolAxisZ) - 1.f) < .001f,
					"Mario source monster has one HP and an authored patrol axis");
				auto patrol = spawned;
				patrol.fMarioPatrolMinimum = -.5f; patrol.fMarioPatrolMaximum = .5f;
				std::map<PLAYER_ID, SERVER_PLAYER> patrolPlayers;
				std::vector<DAMAGE_EVENT> patrolDamage;
				CMonsterBrain brain;
				bool moved = false, turned = false;
				for (uint32_t tick = 1u; tick <= 150u; ++tick)
				{
					brain.Update(patrol, patrolPlayers, room->m_GameplayCatalog,
						room->m_ServerNavigation, 1.f / 30.f, tick, patrolDamage);
					const float dx = patrol.fPositionX - spawned.fPositionX;
					const float dz = patrol.fPositionZ - spawned.fPositionZ;
					moved |= std::hypot(dx, dz) > .1f;
					turned |= patrol.bMarioPatrolForward != spawned.bMarioPatrolForward;
					tests.Require(std::abs(dx * patrol.fMarioPatrolAxisZ - dz * patrol.fMarioPatrolAxisX) < .01f,
						"Mario patrol preserves its lateral line");
				}
				tests.Require(moved && turned, "Mario patrol walks and turns at its endpoint or floor edge");
				auto blockedPatrol = spawned;
				blockedPatrol.bMarioPatrolForward = true;
				blockedPatrol.fYawDegrees = std::atan2(spawned.fMarioPatrolAxisX, spawned.fMarioPatrolAxisZ) * 57.2957795f;
				CServerCollisionSystem patrolCollision;
				std::string patrolCollisionStatus;
				patrolCollision.Initialize({}, patrolCollisionStatus);
				patrolCollision.Set_BlockingBodies({ SERVER_BLOCKING_BODY{
					spawned.fPositionX + spawned.fMarioPatrolAxisX * (2.f * spawned.fCollisionRadius + .001f),
					spawned.fPositionZ + spawned.fMarioPatrolAxisZ * (2.f * spawned.fCollisionRadius + .001f),
					spawned.fCollisionRadius, spawned.fPositionY + spawned.fCollisionRadius,
					spawned.fCollisionRadius, 98765u } });
				brain.Update(blockedPatrol, patrolPlayers, room->m_GameplayCatalog,
					room->m_ServerNavigation, patrolCollision, 1.f / 30.f, 175u, patrolDamage);
				tests.Require(!blockedPatrol.bMarioPatrolForward &&
					blockedPatrol.fPositionX == spawned.fPositionX && blockedPatrol.fPositionZ == spawned.fPositionZ,
					"Mario patrol reverses at a blocking body without tangent sliding");
				patrol = spawned;
				patrol.fYawDegrees = std::atan2(patrol.fMarioPatrolAxisX, patrol.fMarioPatrolAxisZ) * 57.2957795f;
				auto& victim = patrolPlayers[12345u];
				victim.iNetEntityId = 12345u; victim.iMarioStage = stage;
				victim.iCurrentHp = victim.iMaximumHp = 50000u; victim.isCombatReady = true;
				victim.fPositionX = patrol.fPositionX + patrol.fMarioPatrolAxisX * .7f;
				victim.fPositionZ = patrol.fPositionZ + patrol.fMarioPatrolAxisZ * .7f;
				victim.fPositionY = patrol.fPositionY + 4.f;
				brain.Update(patrol, patrolPlayers, room->m_GameplayCatalog,
					room->m_ServerNavigation, 1.f / 30.f, 200u, patrolDamage);
				tests.Require(patrol.eAction != SERVER_ENTITY_ACTION::PATTERN_WINDUP,
					"Mario monster ignores a player on another floor");
				patrol = spawned;
				patrol.fYawDegrees = std::atan2(patrol.fMarioPatrolAxisX, patrol.fMarioPatrolAxisZ) * 57.2957795f;
				victim.fPositionY = patrol.fPositionY;
				for (uint32_t tick = 201u; tick <= 230u; ++tick)
					brain.Update(patrol, patrolPlayers, room->m_GameplayCatalog,
						room->m_ServerNavigation, 1.f / 30.f, tick, patrolDamage);
				const auto* victimProfile = room->m_GameplayCatalog.Find_Player(victim.eCharacterClass);
				const auto expectedDamage = CGameplayCatalog::Apply_Defense(patrol.iAttackPower,
					victimProfile ? victimProfile->iDefense : 0u);
				tests.Require(victim.iCurrentHp == 50000u - expectedDamage && patrolDamage.size() == 1u,
					"Mario nearby attack lands exactly once and does not force player death");
				SERVER_PLAYER_TO_WORLD_HIT strike{};
				strike.iSkillId = 34010u; strike.iRawDamage = 1u; strike.iServerTick = 231u;
				tests.Require(CServerCombatHitRuntime::Apply_PlayerToWorld(patrol, strike, patrolDamage) ==
					SERVER_COMBAT_HIT_RESULT::KILLED && !patrol.iCurrentHp,
					"One admitted player hit kills the Mario monster");
				// Each actual source spawn is tested, not just one archetype.
			}
			auto& participant = room->m_Players[999u];
			participant.iMarioStage = stage; participant.iMarioLayoutVariant = layout; participant.iCurrentHp = 100u;
			tests.Require(room->Begin_MarioStageObjects(stage) == layout &&
				room->Count_SpawnGroupEntities(sourceGroup) == expectedSourceCounts[stage],
				"Another Mario participant shares layout without duplicating source monsters");
			room->Cleanup_EmptyMarioStages();
			tests.Require(room->Count_SpawnGroupEntities(sourceGroup) == expectedSourceCounts[stage],
				"Occupied Mario stage survives cleanup");
			participant.Clear_MarioControl();
			tests.Require(!participant.iMarioLayoutVariant, "Leaving Mario clears selected layout");
			room->m_Players.erase(999u);
			room->Cleanup_EmptyMarioStages();
			tests.Require(!room->Count_SpawnGroupEntities(sourceGroup) &&
				!room->m_SpawnGroupRuntime.Is_ActiveOrCompleted(sourceGroup),
				"Last Mario departure despawns source group and resets its schedule");
			room->m_MarioLayoutRandom.seed(37081u);
			unsigned cases = 0u;
			for (unsigned retry = 0u; retry < 100u; ++retry)
				cases |= 1u << room->Begin_MarioStageObjects(stage);
			tests.Require(cases == 14u, "Repeated Mario entries can select all three original cases");
			room->Cleanup_EmptyMarioStages();
			const auto& destination = entrance->TriggerActions.front();
			SERVER_NAV_POINT ground{};
			const bool sampled = room->m_ServerNavigation.Sample_Position(
				destination.fTargetX, destination.fTargetZ, ground);
			tests.Require(sampled && room->m_ServerNavigation.Is_InSameNavigationGrid(
				ground.x, ground.z, destination.fTargetX, destination.fTargetZ),
				"Mario jump entrance belongs to actual navigation grid");
			if (!sampled) continue;
			auto& player = room->m_Players[123u];
			player.iPlayerId = 123u;
			player.iNetEntityId = 456u;
			player.iCurrentHp = player.iMaximumHp = 100u;
			player.fPositionX = ground.x;
			player.fPositionY = ground.y;
			player.fPositionZ = ground.z;
			const bool entranceWasEnabled = entrance->isEnabled;
			room->Update_MarioControlState(player);
			tests.Require(player.iMarioStage == stage && player.eMadnessForm == PLAYER_MADNESS_FORM::CLOWN &&
				player.iCurrentHp == 100u, "Authored Mario intro admits mode and existing clown form without HP change");
			tests.Require(player.bMarioRailReady && entrance->isEnabled == entranceWasEnabled,
				"Admitted Mario uses entrance rail metadata without enabling its trigger");
			player.iSessionId = 789u;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			room->m_PlayerIdBySessionId[player.iSessionId] = player.iPlayerId;
			C2S_MOVE ordinaryMove{};
			ordinaryMove.iClientSequence = 1u;
			ordinaryMove.fGoalX = ground.x + 20.f;
			ordinaryMove.fGoalZ = ground.z;
			room->Handle_Move(player.iSessionId, ordinaryMove);
			C2S_USE_SKILL ordinarySkill{};
			ordinarySkill.iClientSequence = 1u;
			ordinarySkill.iSkillId = 34010u;
			ordinarySkill.fAimX = ground.x;
			ordinarySkill.fAimZ = ground.z;
			room->Handle_UseSkill(player.iSessionId, ordinarySkill);
			tests.Require(player.eAction == PLAYER_ACTION_STATE::NONE && !player.hasMoveGoal,
				"Mario mode rejects ordinary mouse movement and class skills while grounded");
			{
				const auto savedPlayer = player;
				const auto savedTick = room->m_iServerTick;
				auto savedEntities = std::move(room->m_WorldEntities);
				auto savedDamage = std::move(room->m_TickDamageEvents);
				room->m_WorldEntities.clear(); room->m_TickDamageEvents.clear();
				player.isCombatReady = true;
				player.eKoukuHudMode = KOUKU_HUD_MODE::MARIO;
				player.ModeSkillIndexBySlot[0] = 0; player.ModeSkillIndexBySlot[1] = 1;
				player.fYawDegrees = 0.f;
				for (uint32_t index = 0; index < 5u; ++index)
				{
					SERVER_WORLD_ENTITY target{};
					target.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
					target.iNetEntityId = 9000u + index;
					target.iMarioPatrolStage = stage;
					target.iCurrentHp = target.iMaximumHp = 1u;
					target.fCollisionRadius = .6f;
					target.fPositionX = player.fPositionX;
					target.fPositionY = player.fPositionY;
					target.fPositionZ = player.fPositionZ + 1.f;
					if (index == 1u) target.fPositionZ = player.fPositionZ - 1.f;
					if (index == 2u) target.fPositionY += 4.f;
					if (index == 3u) target.iMarioPatrolStage = stage == 4u ? 1u : stage + 1u;
					if (index == 4u) target.fPositionZ += 10.f;
					room->m_WorldEntities.push_back(target);
				}
				room->m_iServerTick = 1000u;
				C2S_INTERACTION_SLOT press{};
				press.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
				press.eSlot = static_cast<INTERACTION_SLOT>(0);
				press.iRequestSequence = player.iLastKoukuInteractionSequence + 1u;
				room->Handle_InteractionSlot(player.iSessionId, press);
				tests.Require(player.eAction == PLAYER_ACTION_STATE::INTERACTION && player.iCurrentSkillId == 0u,
					"Mario Q command starts replicated hammer action");
				const auto hitTick = player.iActionStartTick + CKoukuCardMazeRuntime::HAMMER_HIT_TICK_OFFSET;
				for (; room->m_iServerTick + 1u < hitTick; ++room->m_iServerTick)
					room->Update_Players(1.f / 30.f);
				tests.Require(room->m_WorldEntities[0].iCurrentHp == 1u && room->m_TickDamageEvents.empty(),
					"Mario Q does not damage before the hammer contact tick");
				room->Update_Players(1.f / 30.f);
				tests.Require(room->m_WorldEntities[0].eAction == SERVER_ENTITY_ACTION::DEAD &&
					!room->m_WorldEntities[0].iCurrentHp && room->m_TickDamageEvents.size() == 1u,
					"Mario Q command through player update kills its front target in one hit");
				for (size_t index = 1; index < room->m_WorldEntities.size(); ++index)
					tests.Require(room->m_WorldEntities[index].iCurrentHp == 1u,
						"Mario Q excludes behind, other floor, other stage and out-of-range targets");
				++room->m_iServerTick;
				room->Update_Players(1.f / 30.f);
				tests.Require(room->m_TickDamageEvents.size() == 1u, "Mario hammer damage is emitted only once");
				player.eAction = PLAYER_ACTION_STATE::NONE;
				press.eSlot = static_cast<INTERACTION_SLOT>(1); ++press.iRequestSequence;
				room->Handle_InteractionSlot(player.iSessionId, press);
				room->m_WorldEntities[0].eAction = SERVER_ENTITY_ACTION::IDLE;
				room->m_WorldEntities[0].iCurrentHp = 1u;
				room->m_iServerTick = player.iActionStartTick + CKoukuCardMazeRuntime::HAMMER_HIT_TICK_OFFSET - 1u;
				room->Update_Players(1.f / 30.f);
				tests.Require(player.eAction == PLAYER_ACTION_STATE::INTERACTION && player.iCurrentSkillId == 1u &&
					room->m_WorldEntities[0].iCurrentHp == 1u, "Mario W retains its action without borrowing Q hammer damage");
				player = savedPlayer; room->m_iServerTick = savedTick;
				room->m_WorldEntities = std::move(savedEntities);
				room->m_TickDamageEvents = std::move(savedDamage);
			}
			{
				/* Source balls: the published layout slots pop on the same contact tick,
				once each and only in front on this floor; a colour's curse lifts with
				its last ball. Entities are parked so a swing near a patrol changes nothing else. */
				const auto savedPlayer = player;
				const auto savedTick = room->m_iServerTick;
				auto savedEntities = std::move(room->m_WorldEntities);
				auto savedDamage = std::move(room->m_TickDamageEvents);
				room->m_WorldEntities.clear(); room->m_TickDamageEvents.clear();
				std::vector<const MARIO_SOURCE_BALL*> balls;
				for (const auto& ball : room->m_WorldBootstrap.Get_MarioBalls())
					if (ball.stage == stage && ball.layout == player.iMarioLayoutVariant) balls.push_back(&ball);
				bool slotsOrdered = true;
				for (size_t index = 0; index < balls.size(); ++index) slotsOrdered &= balls[index]->slot == index;
				tests.Require(balls.size() == (stage == 4u ? 12u : 9u) && slotsOrdered,
					"Published Mario layout carries every source ball slot in order");
				player.isCombatReady = true;
				player.eKoukuHudMode = KOUKU_HUD_MODE::MARIO;
				player.ModeSkillIndexBySlot[0] = 0; player.ModeSkillIndexBySlot[1] = 1;
				const auto swing = [&](const float x, const float y, const float z, const float yawDegrees)
				{
					player.fPositionX = x; player.fPositionY = y; player.fPositionZ = z;
					player.fYawDegrees = yawDegrees;
					player.eAction = PLAYER_ACTION_STATE::NONE;
					room->m_iServerTick += 100u;
					C2S_INTERACTION_SLOT press{};
					press.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
					press.eSlot = static_cast<INTERACTION_SLOT>(0);
					press.iRequestSequence = player.iLastKoukuInteractionSequence + 1u;
					room->Handle_InteractionSlot(player.iSessionId, press);
					if (player.eAction != PLAYER_ACTION_STATE::INTERACTION) return false;
					room->m_iServerTick = player.iActionStartTick + CKoukuCardMazeRuntime::HAMMER_HIT_TICK_OFFSET - 1u;
					room->Update_Players(1.f / 30.f);
					return true;
				};
				if (balls.size() >= 2u)
				{
					const auto& first = *balls[0];
					// Facing +Z from one metre behind the ball: ahead, on this floor.
					tests.Require(swing(first.x, first.y, first.z - 1.f, 0.f) &&
						room->m_MarioPoppedBalls[stage] == 1u, "Mario Q pops the source ball in front on the contact tick");
					tests.Require(swing(first.x, first.y, first.z - 1.f, 0.f) &&
						room->m_MarioPoppedBalls[stage] == 1u, "A popped ball stays popped under a second swing");
					std::uint16_t expected = 1u;
					for (const auto* ball : balls)
						if (ball->color == first.color && ball->slot != first.slot)
						{
							tests.Require(swing(ball->x, ball->y, ball->z - 1.f, 0.f), "Mario Q swings at each remaining ball of the colour");
							expected |= static_cast<std::uint16_t>(1u << ball->slot);
						}
					tests.Require(room->m_MarioPoppedBalls[stage] == expected &&
						room->Mario_CurseReleasedMask(stage, player.iMarioLayoutVariant) == (1u << first.color),
						"Popping every ball of one colour releases exactly that colour's curse");
					const auto* other = *std::find_if(balls.begin(), balls.end(),
						[&first](const auto* ball) { return ball->color != first.color; });
					// Ball behind the swing, then the same ball from one floor up: neither pops.
					tests.Require(swing(other->x, other->y, other->z + 1.f, 0.f) &&
						swing(other->x, other->y + 2.56f, other->z - 1.f, 0.f) &&
						room->m_MarioPoppedBalls[stage] == expected, "Mario Q leaves balls behind the swing or on another floor");
				}
				room->m_MarioPoppedBalls[stage] = 0u;
				player = savedPlayer; room->m_iServerTick = savedTick;
				room->m_WorldEntities = std::move(savedEntities);
				room->m_TickDamageEvents = std::move(savedDamage);
			}
			C2S_DEBUG_MARIO_JUMP request{};
			request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
			request.iClientSequence = 1u;
			request.eDirection = MARIO_DIRECTION::RIGHT;
#ifndef _DEBUG
			const auto disabled = room->Apply_DebugMarioJump(player, request);
			tests.Require(disabled.eResult == DEBUG_MARIO_JUMP_RESULT::REJECTED_DISABLED &&
				!player.TriggerMove.isActive && player.iCurrentHp == 100u,
				"Release rejects Mario jump without mutation");
#else
			/* A repeatable synthetic box measures membership through the public
			trigger API; jumping must not emulate a Debug teleport/remove-player. */
			WORLD_BOOTSTRAP_PLACEMENT membershipBox{};
			membershipBox.strPlacementId = "test.mario.membership";
			membershipBox.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
			membershipBox.fPositionX = ground.x;
			membershipBox.fPositionY = ground.y;
			membershipBox.fPositionZ = ground.z;
			membershipBox.fHalfExtentX = membershipBox.fHalfExtentY = membershipBox.fHalfExtentZ = 20.f;
			membershipBox.isTriggerOnce = false;
			WORLD_TRIGGER_ACTION sequence{};
			sequence.eKind = WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE;
			sequence.strTargetId = "test.mario.sequence";
			membershipBox.TriggerActions.push_back(sequence);
			std::string triggerStatus;
			tests.Require(room->m_ServerTriggerSystem.Initialize({ membershipBox }, triggerStatus),
				"Mario jump prepares membership observation");
			std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
			std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
			unsigned entryCount = 0u;
			const auto onEntry = [&entryCount](WORLD_TRIGGER_ACTION_KIND, const std::string&)
			{
				++entryCount;
				return true;
			};
			room->m_ServerTriggerSystem.Evaluate_Entries(room->m_Players, 1u, transfers, onEntry, prompts);
			S2C_DEBUG_MARIO_JUMP_RESULT accepted{};
			/* Only the two Server-owned lane directions are valid jump intents. */
			for (const auto direction : { MARIO_DIRECTION::RIGHT, MARIO_DIRECTION::LEFT })
			{
				request.eDirection = direction;
				accepted = room->Apply_DebugMarioJump(player, request);
				if (accepted.eResult == DEBUG_MARIO_JUMP_RESULT::ACCEPTED) break;
				++request.iClientSequence;
			}
			tests.Require(accepted.eResult == DEBUG_MARIO_JUMP_RESULT::ACCEPTED &&
				player.eAction == PLAYER_ACTION_STATE::TRIGGER_MOVE && player.iCurrentHp == 100u,
				"Mario jump starts existing Server scripted motion without changing HP");
			if (accepted.eResult != DEBUG_MARIO_JUMP_RESULT::ACCEPTED) continue;
			const auto landing = player.TriggerMove;
			C2S_MOVE move{};
			move.iClientSequence = 2u;
			move.fGoalX = ground.x;
			move.fGoalZ = ground.z;
			room->Handle_Move(player.iSessionId, move);
			C2S_USE_SKILL skill{};
			skill.iClientSequence = 2u;
			skill.iSkillId = 34010u;
			skill.fAimX = ground.x;
			skill.fAimZ = ground.z;
			room->Handle_UseSkill(player.iSessionId, skill);
			tests.Require(player.eAction == PLAYER_ACTION_STATE::TRIGGER_MOVE &&
				player.TriggerMove.fTargetX == landing.fTargetX &&
				!player.hasMoveGoal && player.PendingCommand.eKind == PLAYER_PENDING_COMMAND_KIND::NONE,
				"Normal move and skill commands cannot interrupt or buffer over Mario jump");
			const float dx = landing.fTargetX - ground.x;
			const float dz = landing.fTargetZ - ground.z;
			tests.Require(dx * dx + dz * dz <= 16.01f && dx * dx + dz * dz >= 0.56f &&
				std::abs(landing.fTargetY - ground.y) <= 1.f &&
				room->m_ServerNavigation.Is_InSameNavigationGrid(ground.x, ground.z, landing.fTargetX, landing.fTargetZ) &&
				room->m_ServerNavigation.Is_PointWalkableExact(landing.fTargetX, landing.fTargetZ),
				"Mario jump landing remains walkable in same detail region and floor");
			(void)room->m_ServerTriggerSystem.Update_PlayerMotion(player, 0.3f);
			const float midpointY = player.fPositionY;
			tests.Require(std::abs(midpointY - (ground.y + landing.fTargetY) * 0.5f - 1.5f) < 0.001f,
				"Mario jump uses 1.5 metre arc apex at half of 0.6 second motion");
			const auto repeated = room->Apply_DebugMarioJump(player, request);
			tests.Require(repeated.eResult == DEBUG_MARIO_JUMP_RESULT::ACCEPTED &&
				player.TriggerMove.fElapsedSeconds == 0.3f && player.fPositionY == midpointY,
				"Duplicate Mario jump returns cached verdict without restarting arc");
			++request.iClientSequence;
			tests.Require(room->Apply_DebugMarioJump(player, request).eResult ==
				DEBUG_MARIO_JUMP_RESULT::REJECTED_PLAYER_STATE, "Mario jump rejects air chaining");
			C2S_MARIO_MOVE arrows{};
			arrows.iClientSequence = 1u;
			arrows.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
			arrows.eDirection = MARIO_DIRECTION::RIGHT;
			room->Handle_MarioMove(player.iSessionId, arrows);
			tests.Require(player.iMarioMoveExpiryTick == 0u && player.eAction == PLAYER_ACTION_STATE::TRIGGER_MOVE,
				"Mario directional input cannot steer or buffer over active jump");
			arrows.iClientSequence = 2u;
			arrows.eDirection = MARIO_DIRECTION::STOP;
			room->Handle_MarioMove(player.iSessionId, arrows);
			tests.Require(player.TriggerMove.isActive && player.TriggerMove.fElapsedSeconds == 0.3f,
				"Mario key-up stop after jump request cannot cancel scripted motion");
			(void)room->m_ServerTriggerSystem.Update_PlayerMotion(player, 0.3f);
			room->m_ServerTriggerSystem.Evaluate_Entries(room->m_Players, 20u, transfers, onEntry, prompts);
			tests.Require(entryCount == 1u && player.eAction == PLAYER_ACTION_STATE::NONE &&
				player.fPositionX == landing.fTargetX && player.fPositionY == landing.fTargetY,
				"Landing finishes motion and preserves existing trigger membership");
			arrows.iClientSequence = 3u;
			arrows.eDirection = MARIO_DIRECTION::RIGHT;
			room->Handle_MarioMove(player.iSessionId, arrows);
			room->Update_MarioMoveGoal(player, 1u);
			tests.Require(player.hasMoveGoal && player.MovePath.empty() &&
				std::abs(player.fMoveGoalX - player.fPositionX - player.fMarioRailRightX * 0.75f) < 0.001f &&
				std::abs(player.fMoveGoalZ - player.fPositionZ - player.fMarioRailRightZ * 0.75f) < 0.001f &&
				player.iMarioMoveExpiryTick == 9u,
				"Mario direction feeds short direct goal without A-star path and owns 300ms lease");
			arrows.iClientSequence = 4u;
			arrows.eDirection = MARIO_DIRECTION::STOP;
			room->Handle_MarioMove(player.iSessionId, arrows);
			tests.Require(!player.hasMoveGoal && player.iMarioMoveExpiryTick == 0u,
				"Mario key release immediately stops directional movement");
			arrows.iClientSequence = 5u;
			arrows.eDirection = MARIO_DIRECTION::LEFT;
			room->Handle_MarioMove(player.iSessionId, arrows);
			room->Update_MarioMoveGoal(player, 9u);
			tests.Require(!player.hasMoveGoal && player.iMarioMoveExpiryTick == 0u,
				"Lost Mario input expires without stale held movement");
			request.iClientSequence = accepted.iClientSequence;
			tests.Require(room->Apply_DebugMarioJump(player, request).eResult ==
				DEBUG_MARIO_JUMP_RESULT::REJECTED_STALE_SEQUENCE, "Mario jump rejects stale request");
			request.iClientSequence += 2u;
			player.iCurrentHp = 0u;
			tests.Require(room->Apply_DebugMarioJump(player, request).eResult ==
				DEBUG_MARIO_JUMP_RESULT::REJECTED_PLAYER_STATE && player.iCurrentHp == 0u,
				"Mario jump cannot revive a dead player");
			player.iCurrentHp = 100u;
			++request.iClientSequence;
			player.bPatternBound = true;
			tests.Require(room->Apply_DebugMarioJump(player, request).eResult ==
				DEBUG_MARIO_JUMP_RESULT::REJECTED_PLAYER_STATE, "Mario jump rejects captured player");
			player.bPatternBound = false;
			++request.iClientSequence;
			player.fPositionY += 0.5f;
			tests.Require(room->Apply_DebugMarioJump(player, request).eResult ==
				DEBUG_MARIO_JUMP_RESULT::REJECTED_PLAYER_STATE, "Mario jump requires grounded start");
			player.fPositionY -= 0.5f;
			++request.iClientSequence;
			request.eWorldId = WORLD_ID::BERN;
			tests.Require(room->Apply_DebugMarioJump(player, request).eResult ==
				DEBUG_MARIO_JUMP_RESULT::REJECTED_WRONG_WORLD, "Mario jump rejects another world");
			request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
			++request.iClientSequence;
			WORLD_BOOTSTRAP_PLACEMENT blocker = membershipBox;
			blocker.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX;
			blocker.TriggerActions.clear();
			tests.Require(room->m_ServerCollisionSystem.Initialize({ blocker }, triggerStatus),
				"Mario jump prepares blocked landing footprint");
			tests.Require(room->Apply_DebugMarioJump(player, request).eResult ==
				DEBUG_MARIO_JUMP_RESULT::REJECTED_NO_LANDING && !player.TriggerMove.isActive,
				"Mario jump refuses when every landing is blocked");
			++request.iClientSequence;
			player.fPositionX = entrance->fPositionX;
			player.fPositionY = entrance->fPositionY;
			player.fPositionZ = entrance->fPositionZ;
			room->Reset_PlayerForDebugTeleport(player);
			room->Update_MarioControlState(player);
			tests.Require(player.iMarioStage == 0u && player.eMadnessForm == PLAYER_MADNESS_FORM::NORMAL,
				"Other F1 destination clears Mario mode and restores pre-entry form");
			tests.Require(room->Apply_DebugMarioJump(player, request).eResult ==
				DEBUG_MARIO_JUMP_RESULT::REJECTED_OUTSIDE_MARIO,
				"Mario jump rejects main arena/base-grid position");
			player.fPositionX = ground.x;
			player.fPositionY = ground.y;
			player.fPositionZ = ground.z;
			player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
			room->Update_MarioControlState(player);
			WORLD_TRIGGER_ACTION exitMove{};
			exitMove.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
			exitMove.fTargetX = entrance->fPositionX;
			exitMove.fTargetY = entrance->fPositionY;
			exitMove.fTargetZ = entrance->fPositionZ;
			exitMove.fDurationSeconds = 1.5f;
			tests.Require(CServerTriggerSystem::Begin_MovePlayer(player, exitMove, 30u),
				"Mario exit uses existing authored movement owner");
			room->Update_MarioControlState(player);
			tests.Require(player.iMarioStage == 0u && player.eMadnessForm == PLAYER_MADNESS_FORM::CLOWN &&
				player.TriggerMove.isActive, "Mario exit releases input mode but preserves prior F1 clown and exit motion");
			player.TriggerMove = {};
			player.eAction = PLAYER_ACTION_STATE::NONE;
			room->Update_MarioControlState(player);
			player.iCurrentHp = 0u;
			room->Update_MarioControlState(player);
			tests.Require(player.iMarioStage == 0u && player.iMarioMoveExpiryTick == 0u,
				"Death clears Mario input state and held direction");
			player.iCurrentHp = 100u;
			player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
			room->Update_MarioControlState(player);
			player.TriggerMove.isActive = true;
			player.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
			tests.Require(room->Place_PartyForCutscene("world.sequence.instance.original_kouku") == 1u &&
				player.iMarioStage == 0u && player.eMadnessForm == PLAYER_MADNESS_FORM::NORMAL &&
				!player.TriggerMove.isActive && player.eAction == PLAYER_ACTION_STATE::NONE &&
				player.iCurrentHp == 100u,
				"Party cutscene relocation restores pre-Mario form and retires old jump without HP change");
#endif
		}
		{
			auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
			struct EXPECTED_LANE { std::uint8_t stage; const char* arrival; const char* exit; float sign; };
			const EXPECTED_LANE lanes[]{
				{1u,"Mario1_go","Mario1_Trigger_1",1.f},
				{1u,"Mario1_Trigger_1","Mario1_Trigger_3",1.f},
				{1u,"Mario1_Trigger_3","Mario1_Trigger_5",1.f},
				{2u,"Mario2_go","Mario2_Trigger_2",1.f},
				{2u,"Mario2_Trigger_2","Mario2_Trigger_4",-1.f},
				{2u,"Mario2_Trigger_4","Mario2_Trigger_7",1.f},
				{3u,"Mario3_go","Mario3_Trigger_4",1.f},
				{3u,"Mario3_Trigger_4","Mario3_Trigger_5",-1.f},
				{3u,"Mario3_Trigger_5","Mario3_Trigger_6",1.f},
				{3u,"Mario3_Trigger_6","Mario3_Trigger_8",1.f},
				{3u,"Mario3_Trigger_8","Mario3_Trigger_10",-1.f},
				{3u,"Mario3_Trigger_10","Mario3_Trigger_12",1.f},
				{4u,"Mario4_go","Mario4_Tigger_2",1.f},
				{4u,"Mario4_Tigger_3","Mario4_Tigger_6",1.f},
				{4u,"Mario4_Tigger_6","Mario4_Tigger_7",-1.f},
				{4u,"Mario4_Tigger_7","Mario4_Tigger_13",1.f},
				{4u,"Mario4_Tigger_5","Mario4_Tigger_7",1.f}
			};
			for (const auto& lane : lanes)
			{
				const auto* arrival = room->Find_Placement(lane.arrival);
				const auto* exit = room->Find_Placement(lane.exit);
				tests.Require(arrival && exit && arrival->TriggerActions.size() == 1u,
					"Mario lane resolves published stable placement bindings");
				if (!arrival || !exit || arrival->TriggerActions.size() != 1u) continue;
				const auto& action = arrival->TriggerActions.front();
				SERVER_PLAYER player{};
				player.iMarioStage = lane.stage;
				player.fPositionX = action.fTargetX;
				player.fPositionY = action.fTargetY;
				player.fPositionZ = action.fTargetZ;
				const bool configured = room->Configure_MarioRail(player, lane.arrival);
				const float dx = exit->fPositionX - action.fTargetX;
				const float dz = exit->fPositionZ - action.fTargetZ;
				const float length = std::hypot(dx, dz);
				tests.Require(configured && player.bMarioRailReady &&
					std::abs(player.fMarioRailRightX - dx / length * lane.sign) < 0.00001f &&
					std::abs(player.fMarioRailRightZ - dz / length * lane.sign) < 0.00001f &&
					player.fMarioRailOriginX == action.fTargetX && player.fMarioRailOriginZ == action.fTargetZ,
					"All 17 Mario lanes derive fixed signed axes from actual gameplay placement coordinates");
				if (3u == lane.stage && configured)
				{
					/* Diagnose the complete current movement route, not just the
					short start-point round trip below. A legitimate authored jump
					gap must be distinguished from a prematurely blocked approach. */
					const auto previousTick = room->m_iServerTick;
					auto& walker = room->m_Players[321u];
					walker = player;
					walker.iPlayerId = 321u; walker.iNetEntityId = 654u; walker.iSessionId = 987u;
					walker.iCurrentHp = walker.iMaximumHp = 100u;
					walker.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
					room->m_PlayerIdBySessionId[walker.iSessionId] = walker.iPlayerId;
					room->Refresh_PlayerBlockingBodies();
					C2S_MARIO_MOVE movement{};
					movement.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
					movement.eDirection = lane.sign > 0.f ? MARIO_DIRECTION::RIGHT : MARIO_DIRECTION::LEFT;
					unsigned stationaryTicks = 0u;
					unsigned walkedTicks = 0u;
					bool reachedExit = CServerTriggerSystem::Contains_Placement(*exit, walker);
					for (; walkedTicks < 2700u && !reachedExit && stationaryTicks < 15u; ++walkedTicks)
					{
						if (0u == walkedTicks % 3u)
						{
							++movement.iClientSequence;
							room->Handle_MarioMove(walker.iSessionId, movement);
						}
						const float oldX = walker.fPositionX, oldZ = walker.fPositionZ;
						room->Update_Players(1.f / 30.f);
						++room->m_iServerTick;
						stationaryTicks = std::hypot(walker.fPositionX - oldX, walker.fPositionZ - oldZ) < 0.00001f ?
							stationaryTicks + 1u : 0u;
						reachedExit = CServerTriggerSystem::Contains_Placement(*exit, walker);
					}
					const float nextX = walker.fPositionX + walker.fMarioRailRightX * lane.sign * (walker.fMoveSpeed / 30.f);
					const float nextZ = walker.fPositionZ + walker.fMarioRailRightZ * lane.sign * (walker.fMoveSpeed / 30.f);
					SERVER_NAV_POINT currentGround{}, nextGround{}, traversal{};
					const bool currentSampled = room->m_ServerNavigation.Sample_Position(walker.fPositionX, walker.fPositionZ, currentGround);
					const bool nextWalkable = room->m_ServerNavigation.Is_PointWalkableExact(nextX, nextZ);
					const bool nextSampled = room->m_ServerNavigation.Sample_Position(nextX, nextZ, nextGround);
					const bool traversable = room->m_ServerNavigation.Resolve_TraversalStep(
						walker.fPositionX, walker.fPositionZ, nextX, nextZ, traversal);
					float resolvedX = 0.f, resolvedY = 0.f, resolvedZ = 0.f;
					bool collisionBlocked = false;
					const bool collisionResolved = room->m_ServerCollisionSystem.Resolve_PlayerMove(walker,
						nextX, nextSampled ? nextGround.y : walker.fPositionY, nextZ,
						resolvedX, resolvedY, resolvedZ, collisionBlocked);
					std::cout << "[MarioFullTraversal] " << lane.arrival << " -> " << lane.exit
						<< " reached=" << reachedExit << " ticks=" << walkedTicks
						<< " stop=(" << walker.fPositionX << ',' << walker.fPositionY << ',' << walker.fPositionZ << ')'
						<< " remaining=" << std::hypot(exit->fPositionX - walker.fPositionX, exit->fPositionZ - walker.fPositionZ)
						<< " next=(" << nextX << ',' << nextZ << ") currentSample=" << currentSampled
						<< " currentY=" << currentGround.y << " nextWalkable=" << nextWalkable
						<< " nextSample=" << nextSampled << " nextY=" << nextGround.y
						<< " traversal=" << traversable << " collisionResolved=" << collisionResolved
						<< " collisionBlocked=" << collisionBlocked << '\n';
					/* This regression owns the reported E-floor obstruction only.
					Other lanes keep their diagnostic output: an authored jump gap
					is not evidence that a walk-only route should be opened. */
					if (std::string(lane.arrival) == "Mario3_Trigger_8")
					{
						tests.Require(reachedExit, "Mario3 E-floor walk reaches authored T10 trigger OBB");
						if (reachedExit)
						{
							std::string triggerStatus;
							const bool initialized = room->m_ServerTriggerSystem.Initialize(
								room->m_WorldBootstrap.Get_Placements(), triggerStatus);
							std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
							std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
							room->m_ServerTriggerSystem.Evaluate_Entries(room->m_Players,
								room->m_iServerTick + 1u, transfers,
								[](WORLD_TRIGGER_ACTION_KIND, const std::string&) { return true; }, prompts);
							tests.Require(initialized && walker.TriggerMove.isActive &&
								walker.TriggerMove.strSourcePlacementId == "Mario3_Trigger_10",
								"Mario3 E-floor contact runs the actual published T10 trigger action");
							if (walker.TriggerMove.isActive && exit->TriggerActions.size() == 1u)
							{
								const auto& descent = exit->TriggerActions.front();
								const unsigned descentTickLimit = static_cast<unsigned>(
									std::ceil(descent.fDurationSeconds * 30.f)) + 2u;
								for (unsigned descentTick = 0u;
									descentTick < descentTickLimit && walker.TriggerMove.isActive; ++descentTick)
								{
									room->Update_Players(1.f / 30.f);
									++room->m_iServerTick;
								}
								SERVER_NAV_POINT landingGround{};
								const bool landingWalkable = room->m_ServerNavigation.Sample_Position(
									walker.fPositionX, walker.fPositionZ, landingGround);
								tests.Require(!walker.TriggerMove.isActive && walker.iMarioStage == 3u &&
									walker.eMadnessForm == PLAYER_MADNESS_FORM::CLOWN && walker.bMarioRailReady &&
									walker.strMarioRailArrivalId == "Mario3_Trigger_10" &&
									walker.fPositionX == descent.fTargetX && walker.fPositionY == descent.fTargetY &&
									walker.fPositionZ == descent.fTargetZ && landingWalkable &&
									std::abs(landingGround.y - walker.fPositionY) < 0.25f &&
									walker.fMarioRailOriginX == walker.fPositionX && walker.fMarioRailOriginZ == walker.fPositionZ,
									"Mario3 T10 descent lands on F floor and switches its authoritative fixed lane");
								std::cout << "[MarioFullTraversalExit] Mario3_Trigger_10 landing=("
									<< walker.fPositionX << ',' << walker.fPositionY << ',' << walker.fPositionZ
									<< ") lane=" << walker.strMarioRailArrivalId << " ready=" << walker.bMarioRailReady << '\n';
							}
						}
					}
					room->m_PlayerIdBySessionId.erase(walker.iSessionId);
					room->m_Players.erase(321u);
					room->m_iServerTick = previousTick;
				}
				if (4u == lane.stage && configured && std::string(lane.arrival) == "Mario4_Tigger_7")
				{
					/* The user explicitly authored invisible walk support across the
					two card gaps. Walk only, observing actual trigger entries every
					tick, then keep the existing final return action and form policy. */
					const auto previousTick = room->m_iServerTick;
					std::string triggerStatus;
					tests.Require(room->m_ServerTriggerSystem.Initialize(room->m_WorldBootstrap.Get_Placements(), triggerStatus),
						"Mario4 card route loads actual published triggers");
					auto& walker = room->m_Players[321u];
					walker = player;
					walker.iPlayerId = 321u; walker.iNetEntityId = 654u; walker.iSessionId = 987u;
					walker.iCurrentHp = walker.iMaximumHp = 100u;
					walker.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
					room->m_PlayerIdBySessionId[walker.iSessionId] = walker.iPlayerId;
					room->Refresh_PlayerBlockingBodies();
					C2S_MARIO_MOVE movement{};
					movement.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
					movement.eDirection = MARIO_DIRECTION::RIGHT;
					std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
					std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
					unsigned stationaryTicks = 0u, walkedTicks = 0u;
					bool exitStarted = false, cleared = false;
					for (; walkedTicks < 2700u && !cleared && stationaryTicks < 15u; ++walkedTicks)
					{
						if (0u == walkedTicks % 3u && !walker.TriggerMove.isActive && 4u == walker.iMarioStage)
						{
							++movement.iClientSequence;
							room->Handle_MarioMove(walker.iSessionId, movement);
						}
						const float oldX = walker.fPositionX, oldZ = walker.fPositionZ;
						room->Update_Players(1.f / 30.f);
						++room->m_iServerTick;
						room->m_ServerTriggerSystem.Evaluate_Entries(room->m_Players, room->m_iServerTick,
							transfers, [](WORLD_TRIGGER_ACTION_KIND, const std::string&) { return true; }, prompts);
						exitStarted = exitStarted || (walker.TriggerMove.isActive &&
							walker.TriggerMove.strSourcePlacementId == "Mario4_Tigger_13");
						cleared = exitStarted && !walker.TriggerMove.isActive && 0u == walker.iMarioStage;
						stationaryTicks = !walker.TriggerMove.isActive &&
							std::hypot(walker.fPositionX - oldX, walker.fPositionZ - oldZ) < 0.00001f ?
							stationaryTicks + 1u : 0u;
					}
					std::cout << "[Mario4CardRoute] walkOnly=1 ticks=" << walkedTicks << " exitStarted=" << exitStarted
						<< " cleared=" << cleared << " stop=(" << walker.fPositionX << ',' << walker.fPositionY
						<< ',' << walker.fPositionZ << ") stage=" << static_cast<unsigned>(walker.iMarioStage) << '\n';
					tests.Require(exitStarted && cleared,
						"Mario4 walks across the authored card support and activates the final published return trigger");
					if (exit->TriggerActions.size() == 1u)
					{
						const auto& returning = exit->TriggerActions.front();
						tests.Require(cleared && walker.fPositionX == returning.fTargetX &&
							walker.fPositionY == returning.fTargetY && walker.fPositionZ == returning.fTargetZ &&
							walker.eMadnessForm == PLAYER_MADNESS_FORM::NORMAL && !walker.bMarioRailReady &&
							walker.eAction == PLAYER_ACTION_STATE::NONE && walker.iCurrentHp == 100u &&
							walker.eCharacterClass == CHARACTER_CLASS_ID::LANCE_MASTER,
							"Mario4 completion restores arena position and original form without changing HP or class");
					}
					room->m_PlayerIdBySessionId.erase(walker.iSessionId);
					room->m_Players.erase(321u);
					room->m_iServerTick = previousTick;
				}
			}
			for (std::uint8_t stage = 1u; stage <= 4u; ++stage)
			{
				const std::string goId = "Mario" + std::to_string(stage) + "_go";
				const auto* go = room->Find_Placement(goId);
				if (!go || go->TriggerActions.size() != 1u) continue;
				const auto& action = go->TriggerActions.front();
				SERVER_NAV_POINT ground{};
				if (!room->m_ServerNavigation.Sample_Position(action.fTargetX, action.fTargetZ, ground)) continue;
				auto& player = room->m_Players[123u];
				player = {};
				player.iPlayerId = 123u; player.iNetEntityId = 456u; player.iSessionId = 789u;
				player.iCurrentHp = player.iMaximumHp = 100u;
				player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
				player.fPositionX = ground.x; player.fPositionY = ground.y; player.fPositionZ = ground.z;
				room->m_PlayerIdBySessionId[player.iSessionId] = player.iPlayerId;
				room->Update_MarioControlState(player);
				const float originX = player.fMarioRailOriginX, originZ = player.fMarioRailOriginZ;
				const auto depthError = [&player]()
				{
					return std::abs((player.fPositionX - player.fMarioRailOriginX) * player.fMarioRailRightZ -
						(player.fPositionZ - player.fMarioRailOriginZ) * player.fMarioRailRightX);
				};
				C2S_MARIO_MOVE intent{};
				intent.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
				bool stableDepth = true;
				for (unsigned cycle = 0; cycle < 30u; ++cycle)
				{
					for (const auto direction : { MARIO_DIRECTION::RIGHT, MARIO_DIRECTION::LEFT })
					{
						intent.eDirection = direction; ++intent.iClientSequence;
						room->Handle_MarioMove(player.iSessionId, intent);
						for (unsigned tick = 0; tick < 6u; ++tick)
						{
							room->Update_Players(1.f / 30.f);
							stableDepth = stableDepth && depthError() < 0.002f;
						}
					}
				}
				tests.Require(stableDepth && player.fMarioRailOriginX == originX && player.fMarioRailOriginZ == originZ,
					"Repeated Mario left/right traversal never accumulates depth or rebases its lane");
				const auto lastSequence = player.iLastMarioMoveSequence;
				++intent.iClientSequence; intent.eDirection = static_cast<MARIO_DIRECTION>(99u);
				room->Handle_MarioMove(player.iSessionId, intent);
				tests.Require(player.iLastMarioMoveSequence == lastSequence,
					"Mario malformed enum rejected before input-state commit");
				intent.eDirection = MARIO_DIRECTION::RIGHT; intent.eWorldId = WORLD_ID::BERN;
				room->Handle_MarioMove(player.iSessionId, intent);
				tests.Require(player.iLastMarioMoveSequence == lastSequence,
					"Mario direction for another world rejected without changing current lease");
				player.fKnockbackDirectionX = -player.fMarioRailRightZ;
				player.fKnockbackDirectionZ = player.fMarioRailRightX;
				player.fKnockbackRemainingSeconds = 0.1f; player.fKnockbackSpeed = 2.f;
				room->Advance_PlayerKnockback(player, 0.1f);
				tests.Require(depthError() < 0.002f, "Mario knockback cannot displace the player into stage depth");
				/* Re-enter in flight: Clown is immediate, but the fixed lane must
				be derived from the completed authored landing, never its arc. */
				player.Clear_MarioControl(); player.eAction = PLAYER_ACTION_STATE::NONE;
				player.fPositionX = ground.x + 1.f; player.fPositionZ = ground.z + 1.f;
				(void)CServerTriggerSystem::Begin_MovePlayer(player, action, 30u);
				player.TriggerMove.strSourcePlacementId = goId;
				room->Update_MarioControlState(player);
				tests.Require(player.iMarioStage == stage && player.eMadnessForm == PLAYER_MADNESS_FORM::CLOWN &&
					!player.bMarioRailReady, "Mario entry in authored flight defers fixed lane until landing");
				room->Update_Players(action.fDurationSeconds);
				tests.Require(player.bMarioRailReady && player.strMarioRailArrivalId == goId &&
					player.fMarioRailOriginX == action.fTargetX && player.fMarioRailOriginZ == action.fTargetZ,
					"Authored Mario arrival arms lane from its exact completed target");
			}
			/* T2's tiny transfer in Mario4 preserves A's direction, then T3
			uses its real placement target to establish C's new fixed lane. */
			auto& player = room->m_Players[123u];
			for (const char* source : { "Mario4_Tigger_2", "Mario4_Tigger_3" })
			{
				const auto* original = room->Find_Placement(source);
				if (!original) continue;
				auto trigger = *original; trigger.requiresInteract = false; trigger.isTriggerOnce = false;
				std::string status;
				(void)room->m_ServerTriggerSystem.Initialize({trigger}, status);
				player.fPositionX = trigger.fPositionX; player.fPositionY = trigger.fPositionY;
				player.fPositionZ = trigger.fPositionZ; player.eAction = PLAYER_ACTION_STATE::NONE;
				std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
				std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
				room->m_ServerTriggerSystem.Evaluate_Entries(room->m_Players, 30u, transfers,
					[](WORLD_TRIGGER_ACTION_KIND, const std::string&) { return true; }, prompts);
				tests.Require(player.TriggerMove.isActive && player.TriggerMove.strSourcePlacementId == source,
					"Authored trigger motion preserves its stable source placement owner");
				room->Update_Players(trigger.TriggerActions.front().fDurationSeconds);
				tests.Require(player.bMarioRailReady && player.strMarioRailArrivalId == source &&
					player.fMarioRailOriginX == player.fPositionX && player.fMarioRailOriginZ == player.fPositionZ,
					"Mario authored transfer rebases lane only at completed authoritative landing");
				if (std::string(source) == "Mario4_Tigger_2")
				{
					const auto* nextExit = room->Find_Placement("Mario4_Tigger_5");
					const float dx = nextExit ? nextExit->fPositionX - player.fPositionX : 0.f;
					const float dz = nextExit ? nextExit->fPositionZ - player.fPositionZ : 0.f;
					const float length = std::hypot(dx, dz);
					tests.Require(nextExit && length > .1f &&
						std::abs(player.fMarioRailRightX - dx / length) < .00001f &&
						std::abs(player.fMarioRailRightZ - dz / length) < .00001f,
						"Mario4 T2 follows the currently authored T5 continuation axis");
				}
			}
			CServerCollisionSystem collision;
			std::string collisionStatus;
			(void)collision.Initialize({}, collisionStatus);
			collision.Set_BlockingBodies({SERVER_BLOCKING_BODY{0.f, 0.5f, 0.75f}});
			SERVER_PLAYER runner{}; runner.iMarioStage = 1u; runner.fPositionX = -3.f;
			float x = 0.f, y = 0.f, z = 0.f; bool blocked = false;
			tests.Require(collision.Resolve_PlayerMove(runner, 3.f, 0.f, 0.f, x, y, z, blocked) &&
				blocked && x < 0.f && std::abs(z) < 0.0001f,
				"Mario body collision stops on its lane instead of tangent sliding");
			runner.iMarioStage = 0u;
			tests.Require(collision.Resolve_PlayerMove(runner, 3.f, 0.f, 0.f, x, y, z, blocked) &&
				std::abs(z) > 0.01f, "Non-Mario body collision retains ordinary tangent sliding");
		}
		std::cout << "failures : " << tests.failures << '\n';
		return 0 == tests.failures ? 0 : 1;
	}
