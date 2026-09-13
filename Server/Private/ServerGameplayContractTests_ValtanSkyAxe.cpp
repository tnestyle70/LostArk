#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "BossCombatRuntime.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "Network/PacketWriter.h"
#include "ServerNavigation.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_ValtanSkyAxe(TESTS& tests)
{

	{
		/* HIGH_JUMP snapshots exactly one axe at each living raider's position.
		There is no arena-random supplement and no later scheduled wave. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto volleyRoomStorage = std::make_unique<CGameRoom>(LostArk::Shared::WORLD_ID::VALTAN_ARENA);
		CGameRoom& volleyRoom = *volleyRoomStorage;
		tests.Require(volleyRoom.Initialize_WorldEntities(),
			"Initialize the Valtan room for the sky axe volley");
		/* The arena authors Valtan as a disabled Debug spawn, so the room owns
		no boss until one is activated. Stand one up directly. */
		SERVER_WORLD_ENTITY volleyEntity{};
		volleyEntity.iNetEntityId = 8300u;
		volleyEntity.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		volleyEntity.eAction = SERVER_ENTITY_ACTION::IDLE;
		volleyEntity.strArchetypeId = "BOSS_VALTAN";
		volleyEntity.strEncounterId = "ENCOUNTER_VALTAN";
		volleyEntity.iCurrentHp = 60000u;
		volleyEntity.iMaximumHp = 60000u;
		volleyEntity.iMaximumHealthBars = 160u;
		volleyEntity.iAttackPower = 100u;
		volleyEntity.iLastEvaluatedHealthBar = 160u;
		volleyEntity.fPositionX = 156.03f;
		volleyEntity.fPositionY = 22.99751f;
		volleyEntity.fPositionZ = -122.06f;
		volleyEntity.fSpawnPositionX = volleyEntity.fPositionX;
		volleyEntity.fSpawnPositionY = volleyEntity.fPositionY;
		volleyEntity.fSpawnPositionZ = volleyEntity.fPositionZ;
		/* A combat object belongs to a running pattern occurrence, so the boss
		must already have begun one. */
		volleyEntity.iPatternSequence = 1u;
		volleyEntity.strPatternId = "VALTAN_HIGH_JUMP";
		volleyEntity.strActionId = "valtan.attack.high-jump.airborne";
		volleyEntity.PinnedDefinitionRevision =
			volleyRoom.m_GameplayCatalog.Get_ActiveRevision();
		volleyRoom.m_WorldEntities.push_back(volleyEntity);
		SERVER_WORLD_ENTITY* volleyBoss = &volleyRoom.m_WorldEntities.back();
		for (std::uint32_t index = 0u; index < 4u; ++index)
		{
			SERVER_PLAYER raider{};
			raider.iPlayerId = 8100u + index;
			raider.iNetEntityId = 8200u + index;
			raider.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			raider.iCurrentHp = 5000u;
			raider.iMaximumHp = 5000u;
			raider.isCombatReady = true;
			raider.fPositionX = 137.f + static_cast<float>(index);
			raider.fPositionZ = -113.5f;
			volleyRoom.m_Players.emplace(raider.iPlayerId, raider);
		}
		/* One raider is down, so the volley must pass them over. */
		volleyRoom.m_Players.at(8103u).iCurrentHp = 0u;
		volleyRoom.m_Players.at(8103u).eAction = PLAYER_ACTION_STATE::DEAD;
		const bool staged = nullptr != volleyBoss &&
			volleyRoom.Apply_BossPatternStageActions(
				*volleyBoss, "VALTAN_HIGH_JUMP",
				"valtan.attack.high-jump.airborne",
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 900u);
		const auto& liveObjects =
			volleyRoom.m_CombatObjectRuntime.Get_LiveObjects();
		std::set<std::uint32_t> lockedTargets;
		std::vector<std::uint32_t> lockedTargetOrder;
		for (const SERVER_COMBAT_OBJECT& object : liveObjects)
		{
			lockedTargets.insert(object.iLockedTargetNetEntityId);
			lockedTargetOrder.push_back(
				static_cast<std::uint32_t>(object.iLockedTargetNetEntityId));
		}
		std::vector<S2C_COMBAT_OBJECT_SPAWNED> initialSpawned;
		std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT>
			initialPresentationEvents;
		std::vector<S2C_COMBAT_OBJECT_DESPAWNED> initialDespawned;
		volleyRoom.m_CombatObjectRuntime.Drain_Lifecycle(
			initialSpawned, initialPresentationEvents, initialDespawned);
		std::vector<COMBAT_OBJECT_SNAPSHOT> initialSnapshots;
		const bool builtInitialSnapshots =
			volleyRoom.m_CombatObjectRuntime.Build_Snapshots(initialSnapshots);
		const GameplayDataRevision volleyRevision =
			volleyRoom.m_GameplayCatalog.Get_ActiveRevision();
		/* ENTER wave: one locked tracking axe per alive raider (three) plus the
		   four arena-random axes, which carry no locked target. */
		std::size_t arenaRandomAxes = 0u;
		std::vector<std::uint32_t> trackingTargetOrder;
		for (const SERVER_COMBAT_OBJECT& object : liveObjects)
		{
			if (INVALID_NET_ENTITY_ID == object.iLockedTargetNetEntityId)
				++arenaRandomAxes;
			else
				trackingTargetOrder.push_back(
					static_cast<std::uint32_t>(object.iLockedTargetNetEntityId));
		}
		bool eachAlivePlayerOwnsOneAxe =
			nullptr != volleyBoss && 7u == liveObjects.size() &&
			4u == arenaRandomAxes;
		for (const SERVER_COMBAT_OBJECT& object : liveObjects)
		{
			if (INVALID_NET_ENTITY_ID == object.iLockedTargetNetEntityId)
				continue;
			const auto player = std::find_if(
				volleyRoom.m_Players.begin(), volleyRoom.m_Players.end(),
				[&object](const auto& entry)
			{
					return entry.second.iNetEntityId ==
						object.iLockedTargetNetEntityId;
				});
			eachAlivePlayerOwnsOneAxe = eachAlivePlayerOwnsOneAxe &&
				!object.bTrackLockedTargetUntilFirstPulse &&
				volleyRoom.m_Players.end() != player &&
				0u != player->second.iCurrentHp &&
				std::abs(object.LiveState.CurrentPose.fPositionX -
					player->second.fPositionX) < 0.001f &&
				std::abs(object.LiveState.CurrentPose.fPositionY -
					player->second.fPositionY) < 0.001f &&
				std::abs(object.LiveState.CurrentPose.fPositionZ -
					player->second.fPositionZ) < 0.001f;
		}
		const bool reliableAndSnapshotPins =
			7u == initialSpawned.size() && initialDespawned.empty() &&
			initialPresentationEvents.empty() &&
			7u == initialSnapshots.size() &&
			std::all_of(initialSpawned.begin(), initialSpawned.end(),
				[&volleyRevision](const S2C_COMBAT_OBJECT_SPAWNED& message)
				{
					CPacketWriter writer;
					return message.PinnedDefinitionRevision == volleyRevision &&
						Write_Message(writer, message);
				}) &&
			std::all_of(initialSnapshots.begin(), initialSnapshots.end(),
				[&volleyRevision](const COMBAT_OBJECT_SNAPSHOT& snapshot)
				{
					return snapshot.PinnedDefinitionRevision == volleyRevision;
				});
		tests.Require(
			staged && 7u == liveObjects.size() &&
			4u == lockedTargets.size() &&
			0u == lockedTargets.count(8203u) &&
			trackingTargetOrder == std::vector<std::uint32_t>{
				8200u, 8201u, 8202u } &&
			eachAlivePlayerOwnsOneAxe && builtInitialSnapshots &&
			reliableAndSnapshotPins,
			"Deal exactly one spawn-position axe to each living raider plus four arena-random axes");
		std::vector<SERVER_COMBAT_OBJECT_POSE> spawnPoses;
		for (const SERVER_COMBAT_OBJECT& object : liveObjects)
			spawnPoses.push_back(object.LiveState.CurrentPose);
		for (auto& [raiderId, raider] : volleyRoom.m_Players)
		{
			(void)raiderId;
			if (0u != raider.iCurrentHp)
				raider.fPositionZ += 0.5f;
		}
		std::vector<DAMAGE_EVENT> volleyDamageEvents;
		volleyRoom.m_CombatObjectRuntime.Update(
			volleyRoom.m_Players, volleyRoom.m_WorldEntities,
			volleyRoom.m_GameplayCatalog, 1.f / 30.f, 901u,
			volleyDamageEvents);
		bool fixedAtSpawn = spawnPoses.size() == liveObjects.size();
		for (std::size_t index = 0u;
			fixedAtSpawn && index < liveObjects.size(); ++index)
		{
			fixedAtSpawn =
				!liveObjects[index].bTrackLockedTargetUntilFirstPulse &&
				std::abs(spawnPoses[index].fPositionX -
					liveObjects[index].LiveState.CurrentPose.fPositionX) < 0.001f &&
				std::abs(spawnPoses[index].fPositionZ -
					liveObjects[index].LiveState.CurrentPose.fPositionZ) < 0.001f;
		}
		tests.Require(fixedAtSpawn,
			"Keep every sky axe at the pose resolved when its wave spawned");
		for (std::uint32_t tick = 902u; tick <= 936u; ++tick)
		{
			volleyRoom.m_CombatObjectRuntime.Update(
				volleyRoom.m_Players, volleyRoom.m_WorldEntities,
				volleyRoom.m_GameplayCatalog, 1.f / 30.f, tick,
				volleyDamageEvents);
		}
		std::vector<SERVER_COMBAT_OBJECT_POSE> firstPulsePoses;
		for (const SERVER_COMBAT_OBJECT& object : liveObjects)
			firstPulsePoses.push_back(object.LiveState.CurrentPose);
		for (auto& [raiderId, raider] : volleyRoom.m_Players)
		{
			(void)raiderId;
			if (0u != raider.iCurrentHp)
				raider.fPositionZ += 5.f;
		}
		volleyRoom.m_CombatObjectRuntime.Update(
			volleyRoom.m_Players, volleyRoom.m_WorldEntities,
			volleyRoom.m_GameplayCatalog, 1.f / 30.f, 937u,
			volleyDamageEvents);
		std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawnedAtPulse;
		std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> pulseEvents;
		std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawnedAtPulse;
		volleyRoom.m_CombatObjectRuntime.Drain_Lifecycle(
			spawnedAtPulse, pulseEvents, despawnedAtPulse);
		bool semanticPulseEvents = spawnedAtPulse.empty() &&
			despawnedAtPulse.empty() && pulseEvents.size() == liveObjects.size();
		for (std::size_t index = 0u;
			semanticPulseEvents && index < pulseEvents.size(); ++index)
		{
			const S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event =
				pulseEvents[index];
			CPacketWriter writer;
			semanticPulseEvents =
				event.iEventSequence == index + 1u &&
				event.iCombatObjectId == liveObjects[index].iCombatObjectId &&
				event.iSourceNetEntityId == volleyBoss->iNetEntityId &&
				event.eKind ==
					COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE &&
				event.strCombatObjectArchetypeId ==
					"combatobject.valtan.high-jump.target-axe" &&
				event.strOwnerPatternId == "VALTAN_HIGH_JUMP" &&
				event.strOwnerStageActionId ==
					"valtan.attack.high-jump.airborne" &&
				event.strHitId ==
					"hit.valtan.high-jump.target-axe.01" &&
				0u == event.iRepeatIndex &&
				event.PinnedDefinitionRevision == volleyRevision &&
				Write_Message(writer, event);
		}
		tests.Require(semanticPulseEvents,
			"Emit one reliable semantic hit event per HIGH_JUMP axe pulse");
		bool fixedAfterFirstPulse = firstPulsePoses.size() == liveObjects.size();
		for (std::size_t index = 0u;
			fixedAfterFirstPulse && index < liveObjects.size(); ++index)
		{
			fixedAfterFirstPulse =
				std::abs(firstPulsePoses[index].fPositionX -
					liveObjects[index].LiveState.CurrentPose.fPositionX) < 0.001f &&
				std::abs(firstPulsePoses[index].fPositionZ -
					liveObjects[index].LiveState.CurrentPose.fPositionZ) < 0.001f;
		}
		tests.Require(fixedAfterFirstPulse,
			"Keep each snapshotted sky axe fixed after its first timed pulse");

		auto* mutableVolleyPatterns = const_cast<
			std::vector<BOSS_PATTERN_DEFINITION>*>(
				volleyRoom.m_GameplayCatalog.Find_BossPatterns(
					"ENCOUNTER_VALTAN"));
		BOSS_PATTERN_STAGE_ACTION* typedVolleyAction = nullptr;
		BOSS_PATTERN_STAGE_DEFINITION* typedVolleyStage = nullptr;
		if (nullptr != mutableVolleyPatterns)
		{
			for (BOSS_PATTERN_DEFINITION& pattern : *mutableVolleyPatterns)
			{
				if ("VALTAN_HIGH_JUMP" != pattern.strPatternId)
					continue;
				for (BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
				{
					if ("valtan.attack.high-jump.airborne" != stage.strActionId)
						continue;
					typedVolleyStage = &stage;
					for (BOSS_PATTERN_STAGE_ACTION& action : stage.Actions)
					{
						if (BOSS_PATTERN_STAGE_ACTION_KIND::
							SPAWN_COMBAT_OBJECT_VOLLEY == action.eKind)
						{
							typedVolleyAction = &action;
						}
					}
				}
			}
		}
		tests.Require(nullptr != typedVolleyAction,
			"Publish HIGH_JUMP as a typed per-alive-player volley action");
		if (nullptr != typedVolleyAction && nullptr != typedVolleyStage &&
			nullptr != volleyBoss)
		{
			const BOSS_PATTERN_STAGE_ACTION originalVolleyAction =
				*typedVolleyAction;
			volleyRoom.m_CombatObjectRuntime.Reset();
			volleyBoss->iActionStartTick = 1000u;
			volleyBoss->iAppliedPatternStageSpawnWaveCount = 0u;
			bool scheduledRaidersProjected = true;
			for (auto& [raiderId, raider] : volleyRoom.m_Players)
			{
				raider.iCurrentHp = raiderId <= 8101u ? 5000u : 0u;
				raider.eAction = raiderId <= 8101u ?
					PLAYER_ACTION_STATE::NONE : PLAYER_ACTION_STATE::DEAD;
				SERVER_NAV_POINT projected{};
				const bool projectedOntoLiveGround =
					volleyRoom.m_ServerNavigation.Project_Point(
						151.f + static_cast<float>(raiderId - 8100u) * 2.f,
						-122.f, projected) &&
					volleyRoom.m_ServerNavigation.Is_PointWalkableExact(
						projected.x, projected.z);
				scheduledRaidersProjected &= projectedOntoLiveGround;
				if (projectedOntoLiveGround)
				{
					raider.fPositionX = projected.x;
					raider.fPositionY = projected.y;
					raider.fPositionZ = projected.z;
				}
			}
			tests.Require(scheduledRaidersProjected,
				"Project scheduled HIGH_JUMP raiders onto live walkable ground before wave timing");
			const bool firstWaveCommitted =
				scheduledRaidersProjected &&
				volleyRoom.Apply_BossPatternStageActions(
					*volleyBoss, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne",
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1000u);
			volleyBoss->iAppliedPatternStageSpawnWaveCount =
				firstWaveCommitted ? 1u : 0u;
			const bool earlySchedulerNoOp =
				volleyRoom.Apply_BossPatternScheduledSpawnWave(
					*volleyBoss, 1039u);
			const std::size_t afterEarlyScheduler =
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().size();
			volleyRoom.m_Players.at(8101u).iCurrentHp = 0u;
			volleyRoom.m_Players.at(8101u).eAction =
				PLAYER_ACTION_STATE::DEAD;
			const bool lateSchedulerNoOp =
				volleyRoom.Apply_BossPatternScheduledSpawnWave(
					*volleyBoss, 1080u);
			/* ENTER wave: two tracking axes (8200, 8201) plus four arena-random
			   axes. Tick 1039 (1300 ms) is before the second wave; tick 1080
			   (2666 ms) commits wave ordinal one: raider 8101 died, so one
			   tracking axe plus four arena-random axes join the six. */
			std::set<NET_ENTITY_ID> enterWaveTargets;
			std::set<NET_ENTITY_ID> secondWaveTargets;
			std::size_t enterWaveArenaAxes = 0u;
			std::size_t secondWaveArenaAxes = 0u;
			bool waveClocksExact =
				11u == volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().size();
			for (const SERVER_COMBAT_OBJECT& object :
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects())
			{
				const bool arenaAxe =
					INVALID_NET_ENTITY_ID == object.iLockedTargetNetEntityId;
				waveClocksExact = waveClocksExact &&
					(1000u == object.iSpawnTick || 1080u == object.iSpawnTick) &&
					!object.bTrackLockedTargetUntilFirstPulse;
				if (1000u == object.iSpawnTick)
				{
					if (arenaAxe) ++enterWaveArenaAxes;
					else enterWaveTargets.insert(object.iLockedTargetNetEntityId);
				}
				else
				{
					if (arenaAxe) ++secondWaveArenaAxes;
					else secondWaveTargets.insert(object.iLockedTargetNetEntityId);
				}
			}
			std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET> arenaOrigins;
			const BOSS_COMBAT_OBJECT_DEFINITION* scheduledDefinition =
				volleyRoom.m_GameplayCatalog.Find_BossCombatObject(
					"combatobject.valtan.high-jump.target-axe");
			const bool arenaSupplementExact = nullptr != scheduledDefinition &&
				volleyRoom.Resolve_ArenaRandomVolleyOrigins(
					*volleyBoss, *typedVolleyAction, *scheduledDefinition,
					0u, arenaOrigins) && 4u == arenaOrigins.size();
			tests.Require(
				firstWaveCommitted && earlySchedulerNoOp && lateSchedulerNoOp &&
					6u == afterEarlyScheduler && waveClocksExact &&
					2u == volleyBoss->iAppliedPatternStageSpawnWaveCount &&
					enterWaveTargets == std::set<NET_ENTITY_ID>{ 8200u, 8201u } &&
					4u == enterWaveArenaAxes &&
					secondWaveTargets == std::set<NET_ENTITY_ID>{ 8200u } &&
					4u == secondWaveArenaAxes && arenaSupplementExact,
				"Commit the tracking-plus-arena HIGH_JUMP wave at ENTER and the second wave only at its 1333 ms fixed tick");

			/* A nonzero first offset keeps only the volley out of the ENTER
			   transaction. Other ENTER actions commit once, ordinal zero remains
			   pending, and the fixed-tick scheduler owns its exact due edge. */
			volleyRoom.m_CombatObjectRuntime.Reset();
			*typedVolleyAction = originalVolleyAction;
			for (auto& [raiderId, raider] : volleyRoom.m_Players)
			{
				raider.iCurrentHp = 8100u == raiderId ? 5000u : 0u;
				raider.eAction = 8100u == raiderId ?
					PLAYER_ACTION_STATE::NONE : PLAYER_ACTION_STATE::DEAD;
			}
			volleyBoss->iActionStartTick = 990u;
			volleyBoss->iAppliedPatternStageSpawnWaveCount = 0u;
			const GameplayDataRevision immediateRevision =
				volleyRoom.m_GameplayCatalog.Get_ActiveRevision();
			const bool immediateTransitionCommitted =
				volleyRoom.Apply_BossPatternStageTransition(
					*volleyBoss, {}, {}, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne", immediateRevision,
					immediateRevision, 990u);
			tests.Require(
				immediateTransitionCommitted &&
				5u == volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().size() &&
				990u == volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().front().iSpawnTick &&
				1u == volleyBoss->iAppliedPatternStageSpawnWaveCount,
				"Commit zero-offset volley ordinal zero in the ENTER transaction and initialize its applied-wave counter to one");

			volleyRoom.m_CombatObjectRuntime.Reset();
			typedVolleyAction->Volley.iFirstSpawnOffsetMs = 100u;
			BOSS_PATTERN_STAGE_ACTION delayedEnterFlag{};
			delayedEnterFlag.eTrigger = BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER;
			delayedEnterFlag.eKind =
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG;
			delayedEnterFlag.strTargetId = "boss.flag.counterable";
			delayedEnterFlag.iValue = 1u;
			typedVolleyStage->Actions.push_back(delayedEnterFlag);
			typedVolleyAction = &typedVolleyStage->Actions.front();
			for (auto& [raiderId, raider] : volleyRoom.m_Players)
			{
				raider.iCurrentHp = 8100u == raiderId ? 5000u : 0u;
				raider.eAction = 8100u == raiderId ?
					PLAYER_ACTION_STATE::NONE : PLAYER_ACTION_STATE::DEAD;
			}
			volleyBoss->iActionStartTick = 1000u;
			volleyBoss->iAppliedPatternStageSpawnWaveCount = 99u;
			const GameplayDataRevision delayedRevision =
				volleyRoom.m_GameplayCatalog.Get_ActiveRevision();
			const bool delayedEnterCommitted =
				volleyRoom.Apply_BossPatternStageTransition(
					*volleyBoss, {}, {}, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne", delayedRevision,
					delayedRevision, 1000u);
			const bool delayedAbsentAtEnter =
				delayedEnterCommitted &&
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().empty() &&
				0u == volleyBoss->iAppliedPatternStageSpawnWaveCount &&
				CBossCombatRuntime::Has_Flag(
					volleyBoss->BossCombat,
					SERVER_BOSS_COMBAT_FLAG::COUNTERABLE);
			(void)CBossCombatRuntime::Set_Flag(
				volleyBoss->BossCombat,
				SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, false);
			const bool delayedEarlyNoOp =
				volleyRoom.Apply_BossPatternScheduledSpawnWave(
					*volleyBoss, 1002u) &&
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().empty() &&
				0u == volleyBoss->iAppliedPatternStageSpawnWaveCount;
			const bool delayedDueCommitted =
				volleyRoom.Apply_BossPatternScheduledSpawnWave(
					*volleyBoss, 1003u);
			const auto& delayedObjects =
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects();
			const bool delayedDueExact = delayedDueCommitted &&
				5u == delayedObjects.size() &&
				1003u == delayedObjects.front().iSpawnTick &&
				1u == volleyBoss->iAppliedPatternStageSpawnWaveCount &&
				!CBossCombatRuntime::Has_Flag(
					volleyBoss->BossCombat,
					SERVER_BOSS_COMBAT_FLAG::COUNTERABLE);
			tests.Require(
				delayedAbsentAtEnter && delayedEarlyNoOp && delayedDueExact,
				"Delay volley ordinal zero until its exact fixed tick while committing and never replaying other ENTER actions");

			volleyRoom.m_CombatObjectRuntime.Reset();
			BOSS_PATTERN_STAGE_ACTION mismatchedClock = *typedVolleyAction;
			mismatchedClock.Volley.iFirstSpawnOffsetMs = 200u;
			typedVolleyStage->Actions.push_back(std::move(mismatchedClock));
			const SERVER_WORLD_ENTITY beforeMismatchedClock = *volleyBoss;
			const bool mismatchedClockAccepted =
				volleyRoom.Apply_BossPatternStageActions(
					*volleyBoss, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne",
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1010u);
			tests.Require(
				!mismatchedClockAccepted &&
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().empty(),
				"Reject multiple scheduled volleys with mismatched clocks before any partial spawn");
			*volleyBoss = beforeMismatchedClock;
			typedVolleyStage->Actions.pop_back();
			typedVolleyStage->Actions.pop_back();
			typedVolleyAction = &typedVolleyStage->Actions.front();
			*typedVolleyAction = originalVolleyAction;

			volleyRoom.m_CombatObjectRuntime.Reset();
			for (auto& [raiderId, raider] : volleyRoom.m_Players)
			{
				raider.iCurrentHp = 8100u == raiderId ? 5000u : 0u;
				raider.eAction = 8100u == raiderId ?
					PLAYER_ACTION_STATE::NONE : PLAYER_ACTION_STATE::DEAD;
			}
			SERVER_PLAYER& radialTarget = volleyRoom.m_Players.at(8100u);
			radialTarget.fPositionX = 137.f;
			radialTarget.fPositionY = volleyBoss->fPositionY;
			radialTarget.fPositionZ = -105.5f;
			typedVolleyAction->iValue = 8u;
			typedVolleyAction->Volley.iCountPerResolvedTarget = 8u;
			typedVolleyAction->Volley.eLayout =
				BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL;
			typedVolleyAction->Volley.fRadiusM = 12.f;
			typedVolleyAction->Volley.fStartAngleDegrees = 0.f;
			typedVolleyAction->Volley.fAngleStepDegrees = 45.f;
			typedVolleyAction->Volley.bAllowOverlap = false;
			typedVolleyAction->Volley.iMaximumTotalObjects = 32u;
			typedVolleyAction->Volley.iArenaRandomCount = 0u;
			typedVolleyAction->Volley.fArenaRandomRadiusM = 0.f;
			typedVolleyAction->Volley.fArenaHeightToleranceM = 0.f;
			typedVolleyAction->Volley.eArenaAnchorPolicy =
				BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::NONE;
			const auto radialFirstCombatObjectId =
				volleyRoom.m_CombatObjectRuntime.Begin_Transaction().
					iNextCombatObjectId;
			const bool radialStaged =
				volleyRoom.Apply_BossPatternStageActions(
					*volleyBoss, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne",
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 910u);
			const auto& radialObjects =
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects();
			std::set<std::pair<int, int>> quantizedRadialPositions;
			bool radialIdentityIsStable = radialObjects.size() == 8u;
			for (std::size_t ordinal = 0u;
				radialIdentityIsStable && ordinal < radialObjects.size(); ++ordinal)
			{
				const SERVER_COMBAT_OBJECT& object = radialObjects[ordinal];
				radialIdentityIsStable =
					object.iCombatObjectId ==
						radialFirstCombatObjectId + ordinal &&
					object.iLockedTargetNetEntityId == radialTarget.iNetEntityId &&
					910u == object.iSpawnTick;
				quantizedRadialPositions.emplace(
					static_cast<int>(std::lround(
						object.LiveState.CurrentPose.fPositionX * 1000.f)),
					static_cast<int>(std::lround(
						object.LiveState.CurrentPose.fPositionZ * 1000.f)));
			}
			tests.Require(
				radialStaged && radialIdentityIsStable &&
				8u == quantizedRadialPositions.size(),
				"Spawn all eight radial ordinals in deterministic identity order on one tick");

			volleyRoom.m_CombatObjectRuntime.Reset();
			for (auto& [raiderId, raider] : volleyRoom.m_Players)
			{
				(void)raiderId;
				raider.iCurrentHp = 5000u;
				raider.eAction = PLAYER_ACTION_STATE::NONE;
			}
			SERVER_PLAYER fifthRaider = radialTarget;
			fifthRaider.iPlayerId = 8104u;
			fifthRaider.iNetEntityId = 8204u;
			volleyRoom.m_Players.emplace(fifthRaider.iPlayerId, fifthRaider);
			const bool overTotalAccepted =
				volleyRoom.Apply_BossPatternStageActions(
					*volleyBoss, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne",
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 920u);
			tests.Require(
				!overTotalAccepted &&
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().empty(),
				"Reject a five-player eight-axe volley above the total-32 bound with zero spawn");

			volleyRoom.m_CombatObjectRuntime.Reset();
			for (auto& [raiderId, raider] : volleyRoom.m_Players)
			{
				raider.iCurrentHp = raiderId <= 8101u ? 5000u : 0u;
				raider.eAction = raiderId <= 8101u ?
					PLAYER_ACTION_STATE::NONE : PLAYER_ACTION_STATE::DEAD;
			}
			volleyRoom.m_Players.at(8100u).fPositionX = 137.f;
			volleyRoom.m_Players.at(8100u).fPositionZ = -105.5f;
			volleyRoom.m_Players.at(8101u).fPositionX = 138.f;
			volleyRoom.m_Players.at(8101u).fPositionZ = -105.5f;
			typedVolleyAction->iValue = 1u;
			typedVolleyAction->Volley.iCountPerResolvedTarget = 1u;
			typedVolleyAction->Volley.eLayout =
				BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE;
			typedVolleyAction->Volley.fRadiusM = 0.f;
			typedVolleyAction->Volley.fStartAngleDegrees = 0.f;
			typedVolleyAction->Volley.fAngleStepDegrees = 0.f;
			const bool overlappingAccepted =
				volleyRoom.Apply_BossPatternStageActions(
					*volleyBoss, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne",
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 930u);
			tests.Require(
				overlappingAccepted &&
				2u == volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().size(),
				"Allow different players' resolved volley positions to overlap");

			volleyRoom.m_CombatObjectRuntime.Reset();
			volleyRoom.m_Players.at(8101u).iCurrentHp = 0u;
			volleyRoom.m_Players.at(8101u).eAction = PLAYER_ACTION_STATE::DEAD;
			volleyRoom.m_Players.at(8100u).fPositionX = 100000.f;
			volleyRoom.m_Players.at(8100u).fPositionZ = 100000.f;
			const bool outsideNavigationAccepted =
				volleyRoom.Apply_BossPatternStageActions(
					*volleyBoss, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne",
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 940u);
			tests.Require(
				!outsideNavigationAccepted &&
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().empty(),
				"Reject a target-center volley outside navigation with zero spawn");

			volleyRoom.m_CombatObjectRuntime.Reset();
			const BOSS_COMBAT_OBJECT_DEFINITION* volleyDefinition =
				volleyRoom.m_GameplayCatalog.Find_BossCombatObject(
					"combatobject.valtan.high-jump.target-axe");
			SERVER_COMBAT_OBJECT_LOCKED_TARGET capacityTarget{};
			capacityTarget.iNetEntityId = radialTarget.iNetEntityId;
			capacityTarget.fPositionX = volleyBoss->fPositionX;
			capacityTarget.fPositionY = volleyBoss->fPositionY;
			capacityTarget.fPositionZ = volleyBoss->fPositionZ;
			BOSS_COMBAT_OBJECT_VOLLEY capacityVolley{};
			capacityVolley.ePolicy =
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER;
			capacityVolley.iCountPerResolvedTarget = 8u;
			capacityVolley.eLayout =
				BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL;
			capacityVolley.fRadiusM = 12.f;
			capacityVolley.fAngleStepDegrees = 45.f;
			capacityVolley.iMaximumTotalObjects = 32u;
			bool filledSnapshotCapacity = nullptr != volleyDefinition;
			for (std::uint32_t batch = 0u;
				filledSnapshotCapacity && batch < 16u; ++batch)
			{
				SERVER_COMBAT_OBJECT_TRANSACTION transaction =
					volleyRoom.m_CombatObjectRuntime.Begin_Transaction();
				std::string capacityStatus;
				filledSnapshotCapacity =
					volleyRoom.m_CombatObjectRuntime.Stage_BossCombatObject(
						transaction, *volleyBoss, &capacityTarget,
						*volleyDefinition, &capacityVolley,
						volleyRoom.m_GameplayCatalog, 8u, 950u + batch,
						capacityStatus) &&
					volleyRoom.m_CombatObjectRuntime.Commit(
						std::move(transaction));
			}
			volleyRoom.m_CombatObjectRuntime.Discard_PendingLifecycle();
			BOSS_COMBAT_OBJECT_VOLLEY oneMoreVolley{};
			oneMoreVolley.ePolicy =
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER;
			oneMoreVolley.iCountPerResolvedTarget = 1u;
			oneMoreVolley.eLayout =
				BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE;
			oneMoreVolley.iMaximumTotalObjects = 32u;
			SERVER_COMBAT_OBJECT_TRANSACTION rejectedTransaction =
				volleyRoom.m_CombatObjectRuntime.Begin_Transaction();
			std::string capacityStatus;
			const bool acceptedOverSnapshot = nullptr != volleyDefinition &&
				volleyRoom.m_CombatObjectRuntime.Stage_BossCombatObject(
					rejectedTransaction, *volleyBoss, &capacityTarget,
					*volleyDefinition, &oneMoreVolley,
					volleyRoom.m_GameplayCatalog, 1u, 999u, capacityStatus);
			std::vector<S2C_COMBAT_OBJECT_SPAWNED> pendingAfterFailure;
			std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT>
				presentationAfterFailure;
			std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawnedAfterFailure;
			volleyRoom.m_CombatObjectRuntime.Drain_Lifecycle(
				pendingAfterFailure, presentationAfterFailure,
				despawnedAfterFailure);
			tests.Require(
				filledSnapshotCapacity &&
				128u == volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().size() &&
				!acceptedOverSnapshot && rejectedTransaction.Objects.empty() &&
				pendingAfterFailure.empty() && presentationAfterFailure.empty(),
				"Reject object 129 before allocation and emit no partial lifecycle edge");
			*typedVolleyAction = originalVolleyAction;
		}
	}
}

