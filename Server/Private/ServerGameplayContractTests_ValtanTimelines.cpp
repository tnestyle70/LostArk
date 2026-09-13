#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_ValtanTimelines(TESTS& tests)
{

	{
		/* Ground Roar owns four boss-relative Server roots at the authored
		45/135/225/315-degree diagonal slots. A damaging cover root that falls
		outside navigation is projected no more than two metres, and that exact
		pose is shared by gameplay state and the reliable spawn message. Each
		root emits one hit-driven presentation pulse at 5000 ms and retires at
		6200 ms; there is no second presentation-only pulse. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto groundRoarRoomStorage = std::make_unique<CGameRoom>(LostArk::Shared::WORLD_ID::VALTAN_ARENA);
		CGameRoom& groundRoarRoom = *groundRoarRoomStorage;
		const bool initializedGroundRoarRoom =
			groundRoarRoom.Initialize_WorldEntities();
		SERVER_WORLD_ENTITY groundRoarBoss{};
		groundRoarBoss.iNetEntityId = 8350u;
		groundRoarBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		groundRoarBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		groundRoarBoss.strArchetypeId = "BOSS_VALTAN";
		groundRoarBoss.strEncounterId = "ENCOUNTER_VALTAN";
		groundRoarBoss.iCurrentHp = 60000u;
		groundRoarBoss.iMaximumHp = 60000u;
		groundRoarBoss.iMaximumHealthBars = 160u;
		groundRoarBoss.iAttackPower = 100u;
		groundRoarBoss.iLastEvaluatedHealthBar = 160u;
		groundRoarBoss.fPositionX = 156.03f;
		groundRoarBoss.fPositionY = 22.99751f;
		groundRoarBoss.fPositionZ = -122.06f;
		groundRoarBoss.fSpawnPositionX = groundRoarBoss.fPositionX;
		groundRoarBoss.fSpawnPositionY = groundRoarBoss.fPositionY;
		groundRoarBoss.fSpawnPositionZ = groundRoarBoss.fPositionZ;
		groundRoarBoss.fYawDegrees = 37.f;
		groundRoarBoss.iPatternSequence = 1u;
		groundRoarBoss.strPatternId = "VALTAN_GROUND_ROAR";
		groundRoarBoss.strPatternStageId = "STEP_01";
		groundRoarBoss.strActionId =
			"valtan.sequence.sequence.400440.0.step-01";
		groundRoarBoss.PinnedDefinitionRevision =
			groundRoarRoom.m_GameplayCatalog.Get_ActiveRevision();
		groundRoarRoom.m_WorldEntities.push_back(groundRoarBoss);
		SERVER_WORLD_ENTITY* liveGroundRoarBoss =
			&groundRoarRoom.m_WorldEntities.back();
		const bool stagedGroundRoar = initializedGroundRoarRoom &&
			groundRoarRoom.Apply_BossPatternStageActions(
				*liveGroundRoarBoss, "VALTAN_GROUND_ROAR",
				"valtan.sequence.sequence.400440.0.step-01",
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 2000u);
		const auto& groundRoarObjects =
			groundRoarRoom.m_CombatObjectRuntime.Get_LiveObjects();
		std::array<COMBAT_OBJECT_ID, 4u> groundRoarObjectIds{};
		std::array<SERVER_COMBAT_OBJECT_POSE, 4u> groundRoarObjectPoses{};
		std::set<std::pair<int, int>> groundRoarPositions;
		bool hasOffNavigationAuthoredRoot = false;
		bool cardinalSpawnExact = stagedGroundRoar &&
			4u == groundRoarObjects.size();
		for (std::size_t ordinal = 0u;
			cardinalSpawnExact && ordinal < groundRoarObjects.size(); ++ordinal)
		{
			const SERVER_COMBAT_OBJECT& object = groundRoarObjects[ordinal];
			const float yawDegrees = groundRoarBoss.fYawDegrees + 45.f +
				90.f * static_cast<float>(ordinal);
			const float yawRadians = yawDegrees * 0.01745329251994329577f;
			const float expectedX = groundRoarBoss.fPositionX +
				std::sin(yawRadians) * 6.3639610307f;
			const float expectedZ = groundRoarBoss.fPositionZ +
				std::cos(yawRadians) * 6.3639610307f;
			hasOffNavigationAuthoredRoot = hasOffNavigationAuthoredRoot ||
				!groundRoarRoom.m_ServerNavigation.Is_PointWalkableExact(
					expectedX, expectedZ);
			groundRoarObjectIds[ordinal] = object.iCombatObjectId;
			groundRoarObjectPoses[ordinal] = object.LiveState.CurrentPose;
			groundRoarPositions.emplace(
				static_cast<int>(std::lround(
					object.LiveState.CurrentPose.fPositionX * 1000.f)),
				static_cast<int>(std::lround(
					object.LiveState.CurrentPose.fPositionZ * 1000.f)));
			cardinalSpawnExact =
				"combatobject.valtan.ground-roar.rock" ==
					object.strCombatObjectArchetypeId &&
				"combatobject.visual.valtan.ground-roar.rock.v1" ==
					object.strClientVisualId &&
				INVALID_NET_ENTITY_ID == object.iLockedTargetNetEntityId &&
				1u == object.Hits.size() && object.PresentationPulses.empty() &&
				"hit.valtan.ground-roar.rock.explode" ==
					object.Hits.front().strHitId &&
				std::abs(object.fCoverRadiusM - 1.5f) < 0.0001f &&
				groundRoarRoom.m_ServerNavigation.Is_PointWalkableExact(
					object.LiveState.CurrentPose.fPositionX,
					object.LiveState.CurrentPose.fPositionZ) &&
				std::hypot(
					object.LiveState.CurrentPose.fPositionX - expectedX,
					object.LiveState.CurrentPose.fPositionZ - expectedZ) <= 2.0001f &&
				std::abs(object.LiveState.CurrentPose.fYawDegrees - yawDegrees) <
					0.001f &&
				std::abs(object.LiveState.CurrentPose.fDirectionX -
					std::sin(yawRadians)) < 0.001f &&
				std::abs(object.LiveState.CurrentPose.fDirectionZ -
					std::cos(yawRadians)) < 0.001f;
		}
		std::vector<S2C_COMBAT_OBJECT_SPAWNED> groundRoarSpawned;
		std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT>
			groundRoarInitialPresentation;
		std::vector<S2C_COMBAT_OBJECT_DESPAWNED> groundRoarInitialDespawned;
		groundRoarRoom.m_CombatObjectRuntime.Drain_Lifecycle(
			groundRoarSpawned, groundRoarInitialPresentation,
			groundRoarInitialDespawned);
		const GameplayDataRevision groundRoarRevision =
			groundRoarRoom.m_GameplayCatalog.Get_ActiveRevision();
		bool reliableCardinalSpawns = 4u == groundRoarSpawned.size() &&
			groundRoarInitialPresentation.empty() &&
			groundRoarInitialDespawned.empty() &&
			std::all_of(
				groundRoarSpawned.begin(), groundRoarSpawned.end(),
				[&groundRoarRevision](const S2C_COMBAT_OBJECT_SPAWNED& message)
				{
					CPacketWriter writer;
					return message.PinnedDefinitionRevision == groundRoarRevision &&
						Write_Message(writer, message);
				});
		for (std::size_t ordinal = 0u;
			reliableCardinalSpawns && ordinal < groundRoarSpawned.size(); ++ordinal)
		{
			const S2C_COMBAT_OBJECT_SPAWNED& message =
				groundRoarSpawned[ordinal];
			reliableCardinalSpawns =
				message.iCombatObjectId == groundRoarObjectIds[ordinal] &&
				std::abs(message.fPositionX -
					groundRoarObjectPoses[ordinal].fPositionX) < 0.001f &&
				std::abs(message.fPositionY -
					groundRoarObjectPoses[ordinal].fPositionY) < 0.001f &&
				std::abs(message.fPositionZ -
					groundRoarObjectPoses[ordinal].fPositionZ) < 0.001f;
		}
		tests.Require(
			cardinalSpawnExact && hasOffNavigationAuthoredRoot &&
				4u == groundRoarPositions.size() &&
				reliableCardinalSpawns,
			"Project four damaging Ground Roar rocks onto nearby navigation and publish the exact committed poses");

		constexpr PLAYER_ID GROUND_ROAR_TARGET_PLAYER = 8351u;
		constexpr NET_ENTITY_ID GROUND_ROAR_TARGET_ENTITY = 8352u;
		SERVER_PLAYER groundRoarTarget{};
		groundRoarTarget.iPlayerId = GROUND_ROAR_TARGET_PLAYER;
		groundRoarTarget.iNetEntityId = GROUND_ROAR_TARGET_ENTITY;
		groundRoarTarget.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		groundRoarTarget.iCurrentHp = 10000u;
		groundRoarTarget.iMaximumHp = 10000u;
		groundRoarTarget.isCombatReady = true;
		if (cardinalSpawnExact)
		{
			groundRoarTarget.fPositionX = groundRoarObjectPoses[0].fPositionX;
			groundRoarTarget.fPositionY = groundRoarObjectPoses[0].fPositionY;
			groundRoarTarget.fPositionZ = groundRoarObjectPoses[0].fPositionZ;
		}
		groundRoarRoom.m_Players.emplace(
			GROUND_ROAR_TARGET_PLAYER, groundRoarTarget);

		std::vector<DAMAGE_EVENT> groundRoarDamageEvents;
		std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT>
			groundRoarTerminalPresentation;
		std::vector<S2C_COMBAT_OBJECT_DESPAWNED> groundRoarTerminalDespawned;
		/* The explosion hit fires at 5000 ms (update 150) while the rocks are
		   retained until their 6200 ms lifetime (update 186), so the hit-driven
		   presentation and later despawn remain distinct lifecycle edges. */
		std::uint32_t terminalUpdateOrdinal = 0u;
		std::uint32_t despawnUpdateOrdinal = 0u;
		bool retainedForFiveSeconds = cardinalSpawnExact;
		for (std::uint32_t updateOrdinal = 1u;
			retainedForFiveSeconds && updateOrdinal <= 187u; ++updateOrdinal)
		{
			groundRoarRoom.m_CombatObjectRuntime.Update(
				groundRoarRoom.m_Players, groundRoarRoom.m_WorldEntities,
				groundRoarRoom.m_GameplayCatalog, 1.f / 30.f,
				2000u + updateOrdinal, groundRoarDamageEvents);
			std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawnedThisTick;
			std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT>
				presentationThisTick;
			std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawnedThisTick;
			groundRoarRoom.m_CombatObjectRuntime.Drain_Lifecycle(
				spawnedThisTick, presentationThisTick, despawnedThisTick);
			retainedForFiveSeconds = spawnedThisTick.empty();
			if (!presentationThisTick.empty() && 0u == terminalUpdateOrdinal)
			{
				terminalUpdateOrdinal = updateOrdinal;
				groundRoarTerminalPresentation =
					std::move(presentationThisTick);
				retainedForFiveSeconds = retainedForFiveSeconds &&
					despawnedThisTick.empty() &&
					4u == groundRoarRoom.m_CombatObjectRuntime.
						Get_LiveObjects().size();
				continue;
			}
			if (!despawnedThisTick.empty())
			{
				despawnUpdateOrdinal = updateOrdinal;
				groundRoarTerminalDespawned = std::move(despawnedThisTick);
				retainedForFiveSeconds = retainedForFiveSeconds &&
					presentationThisTick.empty();
				break;
			}
			retainedForFiveSeconds = retainedForFiveSeconds &&
				presentationThisTick.empty() &&
				4u == groundRoarRoom.m_CombatObjectRuntime.
					Get_LiveObjects().size();
		}
		bool terminalLifecycleExact = retainedForFiveSeconds &&
			terminalUpdateOrdinal >= 150u && terminalUpdateOrdinal <= 151u &&
			despawnUpdateOrdinal >= 186u && despawnUpdateOrdinal <= 187u &&
			1u == groundRoarDamageEvents.size() &&
			GROUND_ROAR_TARGET_ENTITY ==
				groundRoarDamageEvents.front().iTargetNetEntityId &&
			groundRoarRoom.m_Players.at(GROUND_ROAR_TARGET_PLAYER).iCurrentHp <
				groundRoarTarget.iMaximumHp &&
			groundRoarRoom.m_CombatObjectRuntime.Get_LiveObjects().empty() &&
			4u == groundRoarTerminalPresentation.size() &&
			4u == groundRoarTerminalDespawned.size();
		for (std::size_t ordinal = 0u;
			terminalLifecycleExact && ordinal < 4u; ++ordinal)
		{
			const S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event =
				groundRoarTerminalPresentation[ordinal];
			const S2C_COMBAT_OBJECT_DESPAWNED& despawned =
				groundRoarTerminalDespawned[ordinal];
			CPacketWriter writer;
			terminalLifecycleExact =
				event.iEventSequence == ordinal + 1u &&
				event.iCombatObjectId == groundRoarObjectIds[ordinal] &&
				event.iSourceNetEntityId == groundRoarBoss.iNetEntityId &&
				event.eKind ==
					COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE &&
				event.strCombatObjectArchetypeId ==
					"combatobject.valtan.ground-roar.rock" &&
				event.strOwnerPatternId == "VALTAN_GROUND_ROAR" &&
				event.strOwnerStageActionId ==
					"valtan.sequence.sequence.400440.0.step-01" &&
				event.strHitId ==
					"hit.valtan.ground-roar.rock.explode" &&
				0u == event.iRepeatIndex &&
				std::abs(event.fPositionX -
					groundRoarObjectPoses[ordinal].fPositionX) < 0.001f &&
				std::abs(event.fPositionY -
					groundRoarObjectPoses[ordinal].fPositionY) < 0.001f &&
				std::abs(event.fPositionZ -
					groundRoarObjectPoses[ordinal].fPositionZ) < 0.001f &&
				std::abs(event.fYawDegrees -
					groundRoarObjectPoses[ordinal].fYawDegrees) < 0.001f &&
				event.PinnedDefinitionRevision == groundRoarRevision &&
				despawned.iCombatObjectId == groundRoarObjectIds[ordinal] &&
				Write_Message(writer, event);
		}
		tests.Require(
			terminalLifecycleExact,
			"Damage one overlapping player from the hit pulse at 5000ms, emit one presentation edge per rock, and despawn at 6200ms");
	}
	{
		/* The split Part Break recovery intentionally reuses the Ground Roar
		   radial geometry through its own exact owner. A wall-contact reaction
		   may begin at the arena edge, so its damaging roots use the same bounded
		   navigation projection and atomic transaction. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto partBreakRoomStorage = std::make_unique<CGameRoom>(LostArk::Shared::WORLD_ID::VALTAN_ARENA);
		CGameRoom& partBreakRoom = *partBreakRoomStorage;
		const bool initializedPartBreakRoom =
			partBreakRoom.Initialize_WorldEntities();
		SERVER_WORLD_ENTITY partBreakBoss{};
		partBreakBoss.iNetEntityId = 8380u;
		partBreakBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		partBreakBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		partBreakBoss.strArchetypeId = "BOSS_VALTAN";
		partBreakBoss.strEncounterId = "ENCOUNTER_VALTAN";
		partBreakBoss.iCurrentHp = 60000u;
		partBreakBoss.iMaximumHp = 60000u;
		partBreakBoss.iMaximumHealthBars = 160u;
		partBreakBoss.iAttackPower = 100u;
		partBreakBoss.iLastEvaluatedHealthBar = 160u;
		partBreakBoss.fPositionX = 156.03f;
		partBreakBoss.fPositionY = 22.99751f;
		partBreakBoss.fPositionZ = -122.06f;
		partBreakBoss.fSpawnPositionX = partBreakBoss.fPositionX;
		partBreakBoss.fSpawnPositionY = partBreakBoss.fPositionY;
		partBreakBoss.fSpawnPositionZ = partBreakBoss.fPositionZ;
		partBreakBoss.fYawDegrees = 37.f;
		partBreakBoss.iPatternSequence = 2u;
		partBreakBoss.strPatternId = "VALTAN_PART_BREAK";
		partBreakBoss.strPatternStageId = "PART_BREAK_RECOVERY";
		partBreakBoss.strActionId = "valtan.reaction.part-break.recovery";
		partBreakBoss.PinnedDefinitionRevision =
			partBreakRoom.m_GameplayCatalog.Get_ActiveRevision();
		partBreakRoom.m_WorldEntities.push_back(partBreakBoss);
		SERVER_WORLD_ENTITY* livePartBreakBoss =
			&partBreakRoom.m_WorldEntities.back();
		const bool stagedPartBreak = initializedPartBreakRoom &&
			partBreakRoom.Apply_BossPatternStageActions(
				*livePartBreakBoss, "VALTAN_PART_BREAK",
				"valtan.reaction.part-break.recovery",
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 2050u);
		const auto& objects =
			partBreakRoom.m_CombatObjectRuntime.Get_LiveObjects();
		bool exactPartBreakRoots = stagedPartBreak && 4u == objects.size();
		bool hasOffNavigationPartBreakRoot = false;
		std::set<std::pair<int, int>> positions;
		for (std::size_t ordinal = 0u;
			exactPartBreakRoots && ordinal < objects.size(); ++ordinal)
		{
			const SERVER_COMBAT_OBJECT& object = objects[ordinal];
			const float degrees = partBreakBoss.fYawDegrees + 45.f +
				90.f * static_cast<float>(ordinal);
			const float radians = degrees * 0.01745329251994329577f;
			const float expectedX = partBreakBoss.fPositionX +
				std::sin(radians) * 4.9497475f;
			const float expectedZ = partBreakBoss.fPositionZ +
				std::cos(radians) * 4.9497475f;
			hasOffNavigationPartBreakRoot = hasOffNavigationPartBreakRoot ||
				!partBreakRoom.m_ServerNavigation.Is_PointWalkableExact(
					expectedX, expectedZ);
			positions.emplace(
				static_cast<int>(std::lround(
					object.LiveState.CurrentPose.fPositionX * 1000.f)),
				static_cast<int>(std::lround(
					object.LiveState.CurrentPose.fPositionZ * 1000.f)));
			exactPartBreakRoots =
				"combatobject.valtan.part-break.rock" ==
					object.strCombatObjectArchetypeId &&
				"combatobject.visual.valtan.part-break.rock.v1" ==
					object.strClientVisualId &&
				1u == object.Hits.size() && object.PresentationPulses.empty() &&
				"hit.valtan.part-break.rock.explode" ==
					object.Hits.front().strHitId &&
				std::abs(object.fCoverRadiusM - 1.5f) < 0.0001f &&
				partBreakRoom.m_ServerNavigation.Is_PointWalkableExact(
					object.LiveState.CurrentPose.fPositionX,
					object.LiveState.CurrentPose.fPositionZ) &&
				std::hypot(
					object.LiveState.CurrentPose.fPositionX - expectedX,
					object.LiveState.CurrentPose.fPositionZ - expectedZ) <= 2.0001f;
		}
		std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
		std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> presentation;
		std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
		partBreakRoom.m_CombatObjectRuntime.Drain_Lifecycle(
			spawned, presentation, despawned);
		bool partBreakSpawnMessagesExact = 4u == spawned.size();
		for (std::size_t ordinal = 0u;
			partBreakSpawnMessagesExact && ordinal < spawned.size(); ++ordinal)
		{
			partBreakSpawnMessagesExact =
				spawned[ordinal].iCombatObjectId == objects[ordinal].iCombatObjectId &&
				std::abs(spawned[ordinal].fPositionX -
					objects[ordinal].LiveState.CurrentPose.fPositionX) < 0.001f &&
				std::abs(spawned[ordinal].fPositionY -
					objects[ordinal].LiveState.CurrentPose.fPositionY) < 0.001f &&
				std::abs(spawned[ordinal].fPositionZ -
					objects[ordinal].LiveState.CurrentPose.fPositionZ) < 0.001f;
		}
		tests.Require(
			exactPartBreakRoots && hasOffNavigationPartBreakRoot &&
			4u == positions.size() && partBreakSpawnMessagesExact &&
			presentation.empty() && despawned.empty(),
			"Project four damaging Part Break recovery rocks onto nearby navigation through one atomic Server transaction");
	}
	{
		/* Six Pizza and Struggling both delay a damaging four-pillar wave.
		   Exercise the actual fixed-tick scheduler because ENTER preflight alone
		   intentionally skips these volleys. Every authored off-navigation root
		   must resolve to one nearby walkable pose before the set commits. */
		struct DELAYED_ROCK_PILLAR_CASE final
		{
			const char* patternId = nullptr;
			const char* stageId = nullptr;
			const char* actionId = nullptr;
			const char* combatObjectId = nullptr;
			const char* hitId = nullptr;
			std::uint32_t dueTicks = 0u;
			std::uint32_t hitAtMs = 0u;
			std::uint32_t lifeMs = 0u;
			float radiusM = 0.f;
			/* ARENA_CENTER anchors on the boss spawn pose with world-absolute angles. */
			bool arenaCenter = false;
		};
		const std::array<DELAYED_ROCK_PILLAR_CASE, 2u> cases{
			DELAYED_ROCK_PILLAR_CASE{
				"VALTAN_SIX_PIZZA_106", "STEP_01",
				"valtan.sequence.center-six-pizza-charge.step-01",
				"combatobject.valtan.six-pizza.rock-pillar",
				"hit.valtan.six-pizza.rock-pillar.explode", 30u, 19500u, 20700u,
				10.f, true },
			DELAYED_ROCK_PILLAR_CASE{
				"VALTAN_STRUGGLING", "STEP_04",
				"valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-04",
				"combatobject.valtan.struggling.rock-pillar",
				"hit.valtan.struggling.rock-pillar.explode", 25u, 5000u, 6200u,
				6.3639610307f, false }
		};
		bool delayedDamagingPillarSetsExact = true;
		for (std::size_t caseIndex = 0u; caseIndex < cases.size(); ++caseIndex)
		{
			const DELAYED_ROCK_PILLAR_CASE& testCase = cases[caseIndex];
			auto roomStorage = std::make_unique<CGameRoom>(
				LostArk::Shared::WORLD_ID::VALTAN_ARENA);
			CGameRoom& room = *roomStorage;
			const bool initialized = room.Initialize_WorldEntities();
			SERVER_WORLD_ENTITY boss{};
			boss.iNetEntityId = 8381u + static_cast<NET_ENTITY_ID>(caseIndex);
			boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			boss.eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
			boss.strArchetypeId = "BOSS_VALTAN";
			boss.strEncounterId = "ENCOUNTER_VALTAN";
			boss.iCurrentHp = 60000u;
			boss.iMaximumHp = 60000u;
			boss.iMaximumHealthBars = 160u;
			boss.iAttackPower = 100u;
			boss.iLastEvaluatedHealthBar = 160u;
			boss.fPositionX = 156.03f;
			boss.fPositionY = 22.99751f;
			boss.fPositionZ = -122.06f;
			boss.fSpawnPositionX = boss.fPositionX;
			boss.fSpawnPositionY = boss.fPositionY;
			boss.fSpawnPositionZ = boss.fPositionZ;
			boss.fYawDegrees = 37.f;
			boss.iPatternSequence = 3u +
				static_cast<std::uint32_t>(caseIndex);
			boss.strPatternId = testCase.patternId;
			boss.strPatternStageId = testCase.stageId;
			boss.strActionId = testCase.actionId;
			constexpr std::uint32_t ENTER_TICK = 3000u;
			boss.iActionStartTick = ENTER_TICK;
			boss.PinnedDefinitionRevision =
				room.m_GameplayCatalog.Get_ActiveRevision();
			room.m_WorldEntities.push_back(boss);
			SERVER_WORLD_ENTITY* liveBoss = &room.m_WorldEntities.back();
			const bool entered = initialized &&
				room.Apply_BossPatternStageTransition(
					*liveBoss, {}, {}, testCase.patternId, testCase.actionId,
					liveBoss->PinnedDefinitionRevision,
					liveBoss->PinnedDefinitionRevision, ENTER_TICK);
			const bool absentAtEnter = entered &&
				room.m_CombatObjectRuntime.Get_LiveObjects().empty() &&
				0u == liveBoss->iAppliedPatternStageSpawnWaveCount;
			const bool earlyNoOp = absentAtEnter &&
				room.Apply_BossPatternScheduledSpawnWave(
					*liveBoss, ENTER_TICK + testCase.dueTicks - 1u) &&
				room.m_CombatObjectRuntime.Get_LiveObjects().empty() &&
				0u == liveBoss->iAppliedPatternStageSpawnWaveCount;
			const bool dueCommitted = earlyNoOp &&
				room.Apply_BossPatternScheduledSpawnWave(
					*liveBoss, ENTER_TICK + testCase.dueTicks);
			const auto& objects =
				room.m_CombatObjectRuntime.Get_LiveObjects();
			const bool noDuplicateOnNextTick = dueCommitted &&
				room.Apply_BossPatternScheduledSpawnWave(
					*liveBoss, ENTER_TICK + testCase.dueTicks + 1u) &&
				4u == objects.size() &&
				1u == liveBoss->iAppliedPatternStageSpawnWaveCount;
			bool exactSet = noDuplicateOnNextTick && room.Is_Ready() &&
				1u == liveBoss->iAppliedPatternStageSpawnWaveCount &&
				4u == objects.size();
			bool hasOffNavigationAuthoredRoot = false;
			std::set<std::pair<int, int>> positions;
			for (std::size_t ordinal = 0u;
				exactSet && ordinal < objects.size(); ++ordinal)
			{
				const SERVER_COMBAT_OBJECT& object = objects[ordinal];
				const float degrees =
					(testCase.arenaCenter ? 0.f : boss.fYawDegrees) + 45.f +
					90.f * static_cast<float>(ordinal);
				const float radians = degrees * 0.01745329251994329577f;
				const float expectedX =
					(testCase.arenaCenter ? boss.fSpawnPositionX : boss.fPositionX) +
					std::sin(radians) * testCase.radiusM;
				const float expectedZ =
					(testCase.arenaCenter ? boss.fSpawnPositionZ : boss.fPositionZ) +
					std::cos(radians) * testCase.radiusM;
				hasOffNavigationAuthoredRoot = hasOffNavigationAuthoredRoot ||
					!room.m_ServerNavigation.Is_PointWalkableExact(
						expectedX, expectedZ);
				positions.emplace(
					static_cast<int>(std::lround(
						object.LiveState.CurrentPose.fPositionX * 1000.f)),
					static_cast<int>(std::lround(
						object.LiveState.CurrentPose.fPositionZ * 1000.f)));
				exactSet = exactSet &&
					testCase.combatObjectId ==
						object.strCombatObjectArchetypeId &&
					1u == object.Hits.size() &&
					object.PresentationPulses.empty() &&
					testCase.hitId == object.Hits.front().strHitId &&
					testCase.hitAtMs == object.Hits.front().iAtMs &&
					std::abs(object.fRemainingMilliseconds -
						static_cast<float>(testCase.lifeMs)) < 0.001f &&
					std::abs(object.fCoverRadiusM - 1.5f) < 0.0001f &&
					room.m_ServerNavigation.Is_PointWalkableExact(
						object.LiveState.CurrentPose.fPositionX,
						object.LiveState.CurrentPose.fPositionZ) &&
					std::hypot(
						object.LiveState.CurrentPose.fPositionX - expectedX,
						object.LiveState.CurrentPose.fPositionZ - expectedZ) <=
						2.0001f &&
					std::abs(object.LiveState.CurrentPose.fYawDegrees - degrees) <
						0.001f;
			}
			std::vector<S2C_COMBAT_OBJECT_SPAWNED> pillarSpawned;
			std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> pillarPresentation;
			std::vector<S2C_COMBAT_OBJECT_DESPAWNED> pillarDespawned;
			room.m_CombatObjectRuntime.Drain_Lifecycle(
				pillarSpawned, pillarPresentation, pillarDespawned);
			bool reliableSpawnSet = 4u == pillarSpawned.size() &&
				pillarPresentation.empty() && pillarDespawned.empty() &&
				std::all_of(
					pillarSpawned.begin(), pillarSpawned.end(),
					[](const S2C_COMBAT_OBJECT_SPAWNED& message)
					{
						CPacketWriter writer;
						return Write_Message(writer, message);
					});
			for (std::size_t ordinal = 0u;
				reliableSpawnSet && ordinal < pillarSpawned.size(); ++ordinal)
			{
				reliableSpawnSet =
					pillarSpawned[ordinal].iCombatObjectId ==
						objects[ordinal].iCombatObjectId &&
					std::abs(pillarSpawned[ordinal].fPositionX -
						objects[ordinal].LiveState.CurrentPose.fPositionX) < 0.001f &&
					std::abs(pillarSpawned[ordinal].fPositionY -
						objects[ordinal].LiveState.CurrentPose.fPositionY) < 0.001f &&
					std::abs(pillarSpawned[ordinal].fPositionZ -
						objects[ordinal].LiveState.CurrentPose.fPositionZ) < 0.001f;
			}
			delayedDamagingPillarSetsExact = delayedDamagingPillarSetsExact &&
				exactSet && hasOffNavigationAuthoredRoot &&
				4u == positions.size() && reliableSpawnSet;
		}
		tests.Require(
			delayedDamagingPillarSetsExact,
			"Schedule both Six Pizza and Struggling damaging pillar sets at their exact due tick and atomically project all four roots onto navigation");
	}
	{
		/* A damaging cover set whose authored roots have no projection within the
		   two-metre bound skips the whole wave without staging a prefix. The wave
		   counter advances so a bad authored layout cannot retry every tick. */
		auto roomStorage = std::make_unique<CGameRoom>(
			LostArk::Shared::WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		const bool initialized = room.Initialize_WorldEntities();
		constexpr const char* PATTERN_ID = "VALTAN_SIX_PIZZA_106";
		constexpr const char* ACTION_ID =
			"valtan.sequence.center-six-pizza-charge.step-01";
		SERVER_WORLD_ENTITY boss{};
		boss.iNetEntityId = 8385u;
		boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		boss.eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
		boss.strArchetypeId = "BOSS_VALTAN";
		boss.strEncounterId = "ENCOUNTER_VALTAN";
		boss.iCurrentHp = 60000u;
		boss.iMaximumHp = 60000u;
		boss.iMaximumHealthBars = 160u;
		boss.iAttackPower = 100u;
		boss.iLastEvaluatedHealthBar = 160u;
		boss.fPositionX = 156.03f;
		boss.fPositionY = 22.99751f;
		boss.fPositionZ = -122.06f;
		boss.fSpawnPositionX = boss.fPositionX;
		boss.fSpawnPositionY = boss.fPositionY;
		boss.fSpawnPositionZ = boss.fPositionZ;
		boss.fYawDegrees = 37.f;
		boss.iPatternSequence = 5u;
		boss.strPatternId = PATTERN_ID;
		boss.strPatternStageId = "STEP_01";
		boss.strActionId = ACTION_ID;
		constexpr std::uint32_t ENTER_TICK = 5000u;
		boss.iActionStartTick = ENTER_TICK;
		boss.PinnedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		room.m_WorldEntities.push_back(boss);
		SERVER_WORLD_ENTITY* liveBoss = &room.m_WorldEntities.back();
		BOSS_PATTERN_STAGE_ACTION* mutableVolley = nullptr;
		if (const auto* patterns =
				room.m_GameplayCatalog.Find_BossPatterns("ENCOUNTER_VALTAN"))
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *patterns)
			{
				if (pattern.strPatternId != PATTERN_ID)
					continue;
				for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
				{
					if (stage.strActionId != ACTION_ID)
						continue;
					for (const BOSS_PATTERN_STAGE_ACTION& action : stage.Actions)
					{
						if (BOSS_PATTERN_STAGE_ACTION_KIND::
								SPAWN_COMBAT_OBJECT_VOLLEY == action.eKind)
						{
							mutableVolley =
								const_cast<BOSS_PATTERN_STAGE_ACTION*>(&action);
						}
					}
				}
			}
		}
		bool unprojectableDamagingWaveSkipped = false;
		if (initialized && nullptr != mutableVolley &&
			4u == mutableVolley->Volley.iCountPerResolvedTarget)
		{
			/* Three roots at a 500 m radius: the exact four-rock owner pin no
			   longer matches and every root is far outside navigation. */
			const std::uint32_t authoredCount =
				mutableVolley->Volley.iCountPerResolvedTarget;
			const std::uint32_t authoredValue = mutableVolley->iValue;
			const float authoredRadius = mutableVolley->Volley.fRadiusM;
			mutableVolley->Volley.iCountPerResolvedTarget = 3u;
			mutableVolley->iValue = 3u;
			mutableVolley->Volley.fRadiusM = 500.f;
			const bool entered = room.Apply_BossPatternStageTransition(
				*liveBoss, {}, {}, PATTERN_ID, ACTION_ID,
				liveBoss->PinnedDefinitionRevision,
				liveBoss->PinnedDefinitionRevision, ENTER_TICK);
			const bool dueSkipped = entered &&
				room.Apply_BossPatternScheduledSpawnWave(
					*liveBoss, ENTER_TICK + 30u);
			std::vector<S2C_COMBAT_OBJECT_SPAWNED> skippedSpawned;
			std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> skippedPresentation;
			std::vector<S2C_COMBAT_OBJECT_DESPAWNED> skippedDespawned;
			room.m_CombatObjectRuntime.Drain_Lifecycle(
				skippedSpawned, skippedPresentation, skippedDespawned);
			unprojectableDamagingWaveSkipped = dueSkipped && room.Is_Ready() &&
				room.m_CombatObjectRuntime.Get_LiveObjects().empty() &&
				skippedSpawned.empty() && skippedPresentation.empty() &&
				skippedDespawned.empty() &&
				1u == liveBoss->iAppliedPatternStageSpawnWaveCount &&
				std::string::npos !=
					room.m_strStatus.find("no nearby navigation projection") &&
				room.Apply_BossPatternScheduledSpawnWave(
					*liveBoss, ENTER_TICK + 31u) &&
				1u == liveBoss->iAppliedPatternStageSpawnWaveCount;
			mutableVolley->Volley.iCountPerResolvedTarget = authoredCount;
			mutableVolley->iValue = authoredValue;
			mutableVolley->Volley.fRadiusM = authoredRadius;
		}
		tests.Require(
			unprojectableDamagingWaveSkipped,
			"Skip an unprojectable damaging cover wave atomically, keep the room ready, and do not retry it");
	}
	{
		/* Phase-three portal charges and their visible runners start together at
		the three authored radial vertices, wait 300 ms, and traverse the
		radius-9 m triangle without navigation projection. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto portalRoomStorage = std::make_unique<CGameRoom>(LostArk::Shared::WORLD_ID::VALTAN_ARENA);
		CGameRoom& portalRoom = *portalRoomStorage;
		const bool initializedPortalRoom = portalRoom.Initialize_WorldEntities();
		portalRoom.m_ServerNavigation = CServerNavigation{};
		const bool portalNavigationUnloaded =
			!portalRoom.m_ServerNavigation.Is_Loaded();
		SERVER_WORLD_ENTITY portalBoss{};
		portalBoss.iNetEntityId = 8390u;
		portalBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		portalBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		portalBoss.strPlacementId = "boss.valtan.center";
		portalBoss.strArchetypeId = "BOSS_VALTAN";
		portalBoss.strEncounterId = "ENCOUNTER_VALTAN";
		portalBoss.iCurrentHp = 60000u;
		portalBoss.iMaximumHp = 60000u;
		portalBoss.iMaximumHealthBars = 160u;
		portalBoss.iAttackPower = 100u;
		portalBoss.iLastEvaluatedHealthBar = 160u;
		portalBoss.iPhase = 3u;
		portalBoss.bGhostPhasePatternLoopActive = true;
		portalBoss.fPositionX = 156.03f;
		portalBoss.fPositionY = 22.99751f;
		portalBoss.fPositionZ = -122.06f;
		portalBoss.fSpawnPositionX = portalBoss.fPositionX;
		portalBoss.fSpawnPositionY = portalBoss.fPositionY;
		portalBoss.fSpawnPositionZ = portalBoss.fPositionZ;
		portalBoss.fYawDegrees = 0.f;
		portalBoss.iPatternSequence = 17u;
		portalBoss.strPatternId = "VALTAN_GHOST_PORTAL_ONCE";
		portalBoss.strPatternStageId = "ACTIVE";
		portalBoss.strActionId = "valtan.ghost.portal-once.active";
		portalBoss.PinnedDefinitionRevision =
			portalRoom.m_GameplayCatalog.Get_ActiveRevision();
		portalRoom.m_WorldEntities.push_back(portalBoss);
		SERVER_WORLD_ENTITY* livePortalBoss = &portalRoom.m_WorldEntities.back();
		const bool stagedPortal = initializedPortalRoom &&
			portalNavigationUnloaded &&
			portalRoom.Update_ValtanGhostPortalScheduler(
				*livePortalBoss, portalRoom.m_GameplayCatalog, 2100u);
		const auto& portalObjects =
			portalRoom.m_CombatObjectRuntime.Get_LiveObjects();
		std::vector<const SERVER_WORLD_ENTITY*> portalRunners;
		for (const SERVER_WORLD_ENTITY& entity : portalRoom.m_WorldEntities)
		{
			if (SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER ==
				entity.eDependentBossRole)
			{
				portalRunners.push_back(&entity);
			}
		}
		bool triangleEdgeRoutesExact = stagedPortal &&
			3u == portalObjects.size() && 3u == portalRunners.size();
		std::set<std::pair<int, int>> portalStartPositions;
		std::set<int> portalUndirectedEdgeHeadings;
		constexpr float DEGREES_TO_RADIANS_TEST =
			0.01745329251994329577f;
		constexpr float PORTAL_CIRCUMRADIUS = 9.f;
		for (std::size_t ordinal = 0u;
			triangleEdgeRoutesExact && ordinal < portalObjects.size(); ++ordinal)
		{
			const SERVER_COMBAT_OBJECT& object = portalObjects[ordinal];
			const SERVER_WORLD_ENTITY& runner = *portalRunners[ordinal];
			const float startDegrees = 30.f + 120.f * static_cast<float>(ordinal);
			const std::size_t nextOrdinal = (ordinal + 1u) % portalObjects.size();
			const float endDegrees =
				30.f + 120.f * static_cast<float>(nextOrdinal);
			const float startRadians =
				startDegrees * DEGREES_TO_RADIANS_TEST;
			const float endRadians = endDegrees * DEGREES_TO_RADIANS_TEST;
			const float startOffsetX =
				std::sin(startRadians) * PORTAL_CIRCUMRADIUS;
			const float startOffsetZ =
				std::cos(startRadians) * PORTAL_CIRCUMRADIUS;
			const float routeX =
				std::sin(endRadians) * PORTAL_CIRCUMRADIUS - startOffsetX;
			const float routeZ =
				std::cos(endRadians) * PORTAL_CIRCUMRADIUS - startOffsetZ;
			const float routeLength = std::sqrt(routeX * routeX + routeZ * routeZ);
			const float expectedDirectionX = routeX / routeLength;
			const float expectedDirectionZ = routeZ / routeLength;
			const float expectedYaw = std::atan2(
				expectedDirectionX, expectedDirectionZ) /
				DEGREES_TO_RADIANS_TEST;
			float undirectedHeading = std::fmod(expectedYaw, 180.f);
			if (undirectedHeading < 0.f)
				undirectedHeading += 180.f;
			if (std::abs(undirectedHeading - 180.f) < 0.001f)
				undirectedHeading = 0.f;
			const int quantizedHeading =
				static_cast<int>(std::lround(undirectedHeading));
			const bool exactUndirectedHeading =
				std::abs(undirectedHeading - static_cast<float>(quantizedHeading)) <
					0.001f &&
				(0 == quantizedHeading || 60 == quantizedHeading ||
				 120 == quantizedHeading);
			portalUndirectedEdgeHeadings.insert(quantizedHeading);
			portalStartPositions.emplace(
				static_cast<int>(std::lround(
					object.LiveState.CurrentPose.fPositionX * 1000.f)),
				static_cast<int>(std::lround(
					object.LiveState.CurrentPose.fPositionZ * 1000.f)));
			triangleEdgeRoutesExact =
				"combatobject.valtan.ghost.portal-charge" ==
					object.strCombatObjectArchetypeId &&
				std::abs(object.LiveState.CurrentPose.fPositionX -
					(portalBoss.fPositionX + startOffsetX)) < 0.001f &&
				std::abs(object.LiveState.CurrentPose.fPositionY -
					portalBoss.fPositionY) < 0.001f &&
				std::abs(object.LiveState.CurrentPose.fPositionZ -
					(portalBoss.fPositionZ + startOffsetZ)) < 0.001f &&
				std::abs(routeLength - 15.5884573f) < 0.001f &&
				std::abs(object.LiveState.CurrentPose.fDirectionX -
					expectedDirectionX) < 0.001f &&
				std::abs(object.LiveState.CurrentPose.fDirectionZ -
					expectedDirectionZ) < 0.001f &&
				std::abs(object.LiveState.CurrentPose.fYawDegrees - expectedYaw) <
					0.001f &&
				exactUndirectedHeading &&
				std::abs(object.fSpeedMps - 11.9911210f) < 0.001f &&
				std::abs(object.fRemainingDistanceM - 15.5884573f) < 0.001f &&
				300u == object.iMovementStartDelayMs &&
				!object.bExpireOnDistanceEnd &&
				std::abs(object.fRemainingMilliseconds - 1900.f) < 0.001f &&
				SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER ==
					runner.eDependentBossRole &&
				8390u == runner.iOwnerBossNetEntityId &&
				"BOSS_VALTAN_GHOST" == runner.strArchetypeId &&
				"VALTAN_GHOST_PORTAL_ONCE" == runner.strPatternId &&
				"ACTIVE" == runner.strPatternStageId &&
				"valtan.ghost.portal-once.active" == runner.strActionId &&
				runner.DependentPatternSequence.PatternIds.empty() &&
				runner.bPortalMotionActive &&
				runner.bPortalRushTargetLocked &&
				BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH ==
					runner.ePatternStageMotionKind &&
				std::abs(runner.fPositionX -
					(portalBoss.fPositionX + startOffsetX)) < 0.001f &&
				std::abs(runner.fPositionZ -
					(portalBoss.fPositionZ + startOffsetZ)) < 0.001f &&
				std::abs(runner.fPortalEndX -
					(portalBoss.fPositionX + startOffsetX + routeX)) < 0.001f &&
				std::abs(runner.fPortalEndZ -
					(portalBoss.fPositionZ + startOffsetZ + routeZ)) < 0.001f &&
				std::abs(runner.fYawDegrees - expectedYaw) < 0.001f &&
				300u == runner.iPortalRushRetargetDelayMs &&
				std::abs(runner.fPortalRushSpeedMps - 11.9911210f) < 0.001f &&
				std::abs(runner.fPortalRushDistanceM - 15.5884573f) < 0.001f;
		}
		std::vector<S2C_COMBAT_OBJECT_SPAWNED> portalSpawned;
		std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT>
			portalPresentation;
		std::vector<S2C_COMBAT_OBJECT_DESPAWNED> portalDespawned;
		portalRoom.m_CombatObjectRuntime.Drain_Lifecycle(
			portalSpawned, portalPresentation, portalDespawned);
		const bool exactTriangleHeadings =
			portalUndirectedEdgeHeadings == std::set<int>{ 0, 60, 120 };
		const bool runnersHeldThroughDelay =
			portalRoom.Update_DependentBosses(2109u) &&
			3u == std::count_if(
				portalRoom.m_WorldEntities.begin(),
				portalRoom.m_WorldEntities.end(),
				[](const SERVER_WORLD_ENTITY& entity)
				{
					return SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER ==
						entity.eDependentBossRole &&
						std::abs(entity.fPositionX - entity.fPortalStartX) < 0.001f &&
						std::abs(entity.fPositionZ - entity.fPortalStartZ) < 0.001f;
				});
		const bool runnersAliveBeforeArrival =
			portalRoom.Update_DependentBosses(2147u) &&
			3u == std::count_if(
				portalRoom.m_WorldEntities.begin(),
				portalRoom.m_WorldEntities.end(),
				[](const SERVER_WORLD_ENTITY& entity)
				{
					return SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER ==
						entity.eDependentBossRole;
				});
		const bool runnersReachExactEndpoint =
			portalRoom.Update_DependentBosses(2148u) &&
			3u == std::count_if(
				portalRoom.m_WorldEntities.begin(),
				portalRoom.m_WorldEntities.end(),
				[](const SERVER_WORLD_ENTITY& entity)
				{
					return SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER ==
						entity.eDependentBossRole &&
						std::abs(entity.fPositionX - entity.fPortalEndX) < 0.001f &&
						std::abs(entity.fPositionZ - entity.fPortalEndZ) < 0.001f &&
						std::abs(entity.fActionElapsedSeconds - 1.6f) < 0.001f;
				});
		const bool runnersDespawnAfterEndpointSnapshot =
			portalRoom.Update_DependentBosses(2149u) &&
			std::none_of(
				portalRoom.m_WorldEntities.begin(),
				portalRoom.m_WorldEntities.end(),
				[](const SERVER_WORLD_ENTITY& entity)
				{
					return SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER ==
						entity.eDependentBossRole;
				});
		if (!triangleEdgeRoutesExact || !exactTriangleHeadings ||
			3u != portalStartPositions.size() || 3u != portalSpawned.size() ||
			!portalPresentation.empty() ||
			!portalDespawned.empty())
		{
			std::cout << "[DIAGNOSTIC] Ghost triangle portal initialized=" <<
				initializedPortalRoom << " navUnloaded=" <<
				portalNavigationUnloaded << " staged=" << stagedPortal <<
				" live=" << portalObjects.size() << " starts=" <<
				portalStartPositions.size() << " spawned=" << portalSpawned.size() <<
				" presentation=" << portalPresentation.size() << " despawned=" <<
				portalDespawned.size() << " status=" << portalRoom.m_strStatus << '\n';
		}
		tests.Require(
			portalNavigationUnloaded && triangleEdgeRoutesExact &&
			exactTriangleHeadings &&
			3u == portalStartPositions.size() && 3u == portalSpawned.size() &&
			portalPresentation.empty() &&
			portalDespawned.empty() && runnersHeldThroughDelay &&
			runnersAliveBeforeArrival && runnersReachExactEndpoint &&
			runnersDespawnAfterEndpointSnapshot,
			"Spawn three simultaneous delayed phase-three portal charges and visible runners that publish their exact endpoints on a nav-independent closed radius-7.5m equilateral triangle");
	}
	{
		auto phaseRoomStorage = std::make_unique<CGameRoom>(
			LostArk::Shared::WORLD_ID::VALTAN_ARENA);
		CGameRoom& phaseRoom = *phaseRoomStorage;
		SERVER_WORLD_ENTITY phaseBoss{};
		phaseBoss.iNetEntityId = 8400u;
		phaseBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		phaseBoss.strArchetypeId = "BOSS_VALTAN";
		phaseBoss.strEncounterId = "ENCOUNTER_VALTAN";
		phaseBoss.strPatternId = "VALTAN_ARENA_BREAK_109";
		phaseBoss.strPatternStageId = "IMPACT";
		phaseBoss.strActionId =
			"valtan.mechanic.arena-break-109.impact";
		phaseBoss.iPatternStageIndex = 2u;
		phaseBoss.iPatternSequence = 1u;
		phaseBoss.iPhase = 1u;
		phaseBoss.BossCombat.iStateRevision = 10u;
		phaseBoss.PinnedDefinitionRevision =
			phaseRoom.m_GameplayCatalog.Get_ActiveRevision();
		const bool phaseCommitted =
			phaseRoom.Apply_BossPatternStageActions(
				phaseBoss, phaseBoss.strPatternId, phaseBoss.strActionId,
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1000u);
		const std::uint32_t phaseRevision =
			phaseBoss.BossCombat.iStateRevision;
		const bool duplicatePhaseCommitted =
			phaseRoom.Apply_BossPatternStageActions(
				phaseBoss, phaseBoss.strPatternId, phaseBoss.strActionId,
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1001u);
		tests.Require(
			phaseCommitted && duplicatePhaseCommitted &&
			2u == phaseBoss.iPhase && 11u == phaseRevision &&
			phaseRevision == phaseBoss.BossCombat.iStateRevision,
			"Commit phase two exactly once at 109 IMPACT ENTER");

		auto* mutablePhasePatterns = const_cast<
			std::vector<BOSS_PATTERN_DEFINITION>*>(
				phaseRoom.m_GameplayCatalog.Find_BossPatterns(
					"ENCOUNTER_VALTAN"));
		BOSS_PATTERN_STAGE_DEFINITION* impactStage = nullptr;
		if (nullptr != mutablePhasePatterns)
		{
			for (BOSS_PATTERN_DEFINITION& pattern : *mutablePhasePatterns)
			{
				if ("VALTAN_ARENA_BREAK_109" != pattern.strPatternId)
					continue;
				for (BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
				{
					if ("IMPACT" == stage.strStageId)
						impactStage = &stage;
				}
			}
		}
		if (nullptr != impactStage)
		{
			BOSS_PATTERN_STAGE_ACTION invalidAction{};
			invalidAction.eTrigger =
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER;
			invalidAction.eKind =
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG;
			invalidAction.strTargetId = "boss.flag.unresolved";
			invalidAction.iValue = 1u;
			impactStage->Actions.push_back(invalidAction);
		}
		phaseBoss.iPhase = 1u;
		phaseBoss.BossCombat.iStateRevision = 20u;
		phaseBoss.BossCombat.iFlags = 0u;
		const std::size_t objectCountBeforeRejectedPhase =
			phaseRoom.m_CombatObjectRuntime.Get_LiveObjects().size();
		const bool partialPhaseAccepted = nullptr != impactStage &&
			phaseRoom.Apply_BossPatternStageActions(
				phaseBoss, phaseBoss.strPatternId, phaseBoss.strActionId,
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1002u);
		if (nullptr != impactStage)
			impactStage->Actions.pop_back();
		tests.Require(
			!partialPhaseAccepted && 1u == phaseBoss.iPhase &&
			20u == phaseBoss.BossCombat.iStateRevision &&
			0u == phaseBoss.BossCombat.iFlags &&
			objectCountBeforeRejectedPhase ==
				phaseRoom.m_CombatObjectRuntime.Get_LiveObjects().size(),
			"Reject an invalid second stage action without partial phase, flag, or object commit");
	}
}

