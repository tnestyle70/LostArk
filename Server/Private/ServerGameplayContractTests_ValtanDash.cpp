#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "BossCombatRuntime.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ValtanBrain.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_ValtanDash(TESTS& tests, CGameplayCatalog& catalog, const char* VALTAN_WALL_COLLISION_STATE, float VALTAN_WALL_CENTER_X, float VALTAN_WALL_CENTER_Y, float VALTAN_WALL_CENTER_Z)
{


	{
		/* The wall sweep does not know where the ground stops. One charge stride
		out of the arena floor would otherwise leave the boss standing inside a wall's own
		navigation blocker, which nothing can then path out of without being
		projected first. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto navigableStepRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& navigableStepRoom = *navigableStepRoomStorage;
		constexpr float STEP_FROM_Z = -120.2f;
		constexpr float STEP_TARGET_Z = -117.9778f;
		float reachedX = 0.f;
		float reachedZ = 0.f;
		CGameRoom::Resolve_NavigableStep(
			navigableStepRoom.m_ServerNavigation,
			156.03f, STEP_FROM_Z, 156.03f, STEP_TARGET_Z, reachedX, reachedZ);
		/* A start the grid already refuses is passed through untouched, because
		refusing it there would strand a boss that is somehow off the floor. */
		float strandedX = 0.f;
		float strandedZ = 0.f;
		CGameRoom::Resolve_NavigableStep(
			navigableStepRoom.m_ServerNavigation,
			156.03f, STEP_TARGET_Z, 156.03f, STEP_FROM_Z, strandedX, strandedZ);
		tests.Require(
			navigableStepRoom.Is_Ready() &&
			navigableStepRoom.m_ServerNavigation.Is_PointWalkableExact(
				156.03f, STEP_FROM_Z) &&
			!navigableStepRoom.m_ServerNavigation.Is_PointWalkableExact(
				156.03f, STEP_TARGET_Z) &&
			navigableStepRoom.m_ServerNavigation.Is_PointWalkableExact(
				reachedX, reachedZ) &&
			reachedZ > STEP_FROM_Z && reachedZ < STEP_TARGET_Z &&
			156.03f == strandedX && STEP_FROM_Z == strandedZ,
			"Stop a boss pattern stride against the ground instead of inside a wall");
	}

	{
		/* Valtan walks its own animation the way a player skill does. A stage
		whose clip bakes travel carries that curve, and the stride is the
		difference between the curve at two ticks, so the transform arrives with
		the pose instead of sliding out from under it. A stage that authored its
		own distance keeps the constant slide, because those two carry the boss
		far past anything the bound clip animates. */
		const auto* rootMotionPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const BOSS_PATTERN_DEFINITION* rushPattern = nullptr;
		const BOSS_PATTERN_DEFINITION* chargePattern = nullptr;
		if (nullptr != rootMotionPatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *rootMotionPatterns)
			{
				if ("VALTAN_PORTAL_RUSH" == pattern.strPatternId)
					rushPattern = &pattern;
				else if ("VALTAN_DASH_CHARGE" == pattern.strPatternId)
					chargePattern = &pattern;
			}
		}
		constexpr std::size_t RUSH_STAGE_INDEX = 1u;
		constexpr std::size_t CHARGE_STAGE_INDEX = 1u;
		const bool hasRushCurve =
			nullptr != rushPattern &&
			RUSH_STAGE_INDEX < rushPattern->Stages.size() &&
			!rushPattern->Stages[RUSH_STAGE_INDEX].Motion.RootMotion.empty();
		/* The authored charge is deliberately absent from the curve document. */
		const bool chargeKeepsAuthoredSlide =
			nullptr != chargePattern &&
			CHARGE_STAGE_INDEX < chargePattern->Stages.size() &&
			chargePattern->Stages[CHARGE_STAGE_INDEX].Motion.RootMotion.empty() &&
			BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD ==
				chargePattern->Stages[CHARGE_STAGE_INDEX].Motion.eKind;

		CValtanBrain rootMotionBrain;
		constexpr float FIXED_DELTA_SECONDS = 1.f / 30.f;
		SERVER_WORLD_ENTITY curveBoss{};
		curveBoss.fYawDegrees = 0.f;
		curveBoss.ePatternStageMotionKind =
			BOSS_PATTERN_STAGE_MOTION_KIND::NONE;
		if (hasRushCurve)
		{
			curveBoss.PatternStageRootMotion =
				rushPattern->Stages[RUSH_STAGE_INDEX].Motion.RootMotion;
		}
		/* Walking the whole stage on the curve has to land on the travel the
		clip baked, and never on a constant slide the stage never declared. */
		float travelled = 0.f;
		bool everyStepFinite = true;
		for (std::uint32_t step = 0u; step < 60u; ++step)
		{
			curveBoss.fActionElapsedSeconds =
				static_cast<float>(step + 1u) * FIXED_DELTA_SECONDS;
			float proposedX = 0.f;
			float proposedZ = 0.f;
			if (!rootMotionBrain.Try_BuildStageMotion(
				curveBoss, FIXED_DELTA_SECONDS, proposedX, proposedZ))
			{
				continue;
			}
			if (!std::isfinite(proposedX) || !std::isfinite(proposedZ))
			{
				everyStepFinite = false;
				break;
			}
			travelled += proposedZ - curveBoss.fPositionZ;
			curveBoss.fPositionX = proposedX;
			curveBoss.fPositionZ = proposedZ;
		}
		const float bakedTravel = hasRushCurve ?
			rushPattern->Stages[RUSH_STAGE_INDEX].Motion.RootMotion.back().fForward :
			0.f;

		/* A stage with neither a curve nor an authored distance never proposes a
		step, so ordinary footwork cannot be mistaken for a charge. */
		SERVER_WORLD_ENTITY stillBoss{};
		stillBoss.fActionElapsedSeconds = 0.5f;
		float stillX = 0.f;
		float stillZ = 0.f;
		const bool stillHolds = !rootMotionBrain.Try_BuildStageMotion(
			stillBoss, FIXED_DELTA_SECONDS, stillX, stillZ);

		tests.Require(
			hasRushCurve && chargeKeepsAuthoredSlide && everyStepFinite &&
			stillHolds && bakedTravel > 1.f &&
			std::abs(travelled - bakedTravel) < 0.01f,
			"Walk a Valtan stage along the travel its clip baked and leave the "
			"authored charge on its own distance");
	}

	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		SERVER_WORLD_ENTITY boss{};
		boss.iNetEntityId = 7002u;
		boss.strPatternId = "VALTAN_ARMOR_BREAK_OPENING";
		boss.strPatternStageId = "WALL_CHARGE";
		boss.strActionId =
			"valtan.mechanic.armor-break-opening.charge";
		boss.iPatternStageIndex = 0u;
		boss.iPatternSequence = 159u;
		boss.fPositionX = 151.f;
		boss.fPositionY = 23.04f;
		boss.fPositionZ = -133.312236f;
		boss.fYawDegrees = 90.f;
		bool triggered = false;
		WORLD_DESTRUCTION_GROUP_STATE groupState{};
		std::vector<SERVER_NAV_POINT> wallPassagePath;
		tests.Require(
			room.Is_Ready() &&
			!room.m_ServerNavigation.Is_PointWalkableExact(
				161.402061f, -133.312236f) &&
			!room.m_ServerNavigation.Find_Path(
				160.25f, -130.75f, 162.25f, -135.75f,
				wallPassagePath) &&
			room.Apply_WorldDestructionImpact(
				boss, "collision.valtan.wallgroup.11047903315509031966.15719065619666776634.receiver",
				500u, triggered) && triggered &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				"destroyable.group.valtan.wall159.15719065619666776634",
				groupState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == groupState.eState &&
			!room.m_ServerNavigation.Is_PointWalkableExact(
				161.402061f, -133.312236f),
			"Commit one exact Valtan impact while keeping BREAKING navigation blocked");
		SERVER_BOSS_RECEIVER_HIT duplicateHit{};
		tests.Require(
			(!room.m_ServerCollisionSystem.Sweep_BossCircleAgainstReceivers(
				145.f, 23.04f, -133.312236f,
				175.f, 23.04f, -133.312236f,
				1.f, duplicateHit) ||
			 duplicateHit.strReceiverPlacementId !=
				"collision.valtan.wallgroup.11047903315509031966.15719065619666776634.receiver") &&
			!room.m_ServerCollisionSystem.Is_ImpactReceiverEnabled(
				"collision.valtan.wallgroup.11047903315509031966.15719065619666776634.receiver") &&
			room.Commit_DueWorldDestruction(507u) &&
			!room.m_ServerNavigation.Is_PointWalkableExact(
				161.402061f, -133.312236f),
			"Suppress only the struck receiver and keep its wall closed before the due tick");
		tests.Require(
			room.Commit_DueWorldDestruction(508u) &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				"destroyable.group.valtan.wall159.15719065619666776634",
				groupState) &&
			WORLD_DESTRUCTION_STATE::DESPAWNED == groupState.eState &&
			room.m_ServerNavigation.Is_PointWalkableExact(
				161.402061f, -133.312236f) &&
			!room.m_ServerNavigation.Find_Path(
				160.25f, -130.75f, 162.25f, -135.75f,
				wallPassagePath) &&
			!room.m_ServerCollisionSystem.Is_PlayerBlocking(
				"collision.valtan.wallgroup.11047903315509031966.15719065619666776634"),
			"Atomically open only the struck wall cells and collision at the DESPAWNED due tick");
	}

	{
		/* Exercise the room's real fixed-tick ordering. The charge body first
		moves to the swept contact margin, then commits the exact destruction
		binding, and only that successful binding may publish WALL_CONTACT. */
		const auto stageDashCharge = [](
			CGameRoom& room,
			const float startX,
			const float startY,
			const float startZ,
			const float yawDegrees,
			const std::uint32_t startTick,
			const std::uint32_t patternSequence) -> SERVER_WORLD_ENTITY&
		{
			const BOSS_RUNTIME_PROFILE* profile =
				room.m_GameplayCatalog.Find_Boss("BOSS_VALTAN");
			room.m_WorldEntities.clear();
			room.m_Players.clear();
			SERVER_PLAYER target{};
			target.iPlayerId = 99001u;
			target.iNetEntityId = 99002u;
			target.iCurrentHp = 1000u;
			target.iMaximumHp = 1000u;
			target.isCombatReady = true;
			const float yawRadians = yawDegrees *
				3.14159265358979323846f / 180.f;
			target.fPositionX = startX + std::sin(yawRadians) * 2.f;
			target.fPositionY = startY;
			target.fPositionZ = startZ + std::cos(yawRadians) * 2.f;
			room.m_Players.emplace(target.iPlayerId, target);

			SERVER_WORLD_ENTITY boss{};
			boss.iNetEntityId = 99003u;
			boss.strPlacementId = "boss.valtan.dash-contract";
			boss.strArchetypeId = "BOSS_VALTAN";
			boss.strEncounterId = "ENCOUNTER_VALTAN";
			boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			boss.eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
			boss.fPositionX = startX;
			boss.fPositionY = startY;
			boss.fPositionZ = startZ;
			boss.fSpawnPositionX = startX;
			boss.fSpawnPositionY = startY;
			boss.fSpawnPositionZ = startZ;
			boss.fYawDegrees = yawDegrees;
			boss.iCurrentHp = nullptr == profile ? 60000u : profile->iMaximumHp;
			boss.iMaximumHp = boss.iCurrentHp;
			boss.iMaximumHealthBars =
				nullptr == profile ? 160u : profile->iMaximumHealthBars;
			boss.iLastEvaluatedHealthBar = boss.iMaximumHealthBars;
			boss.iLastHealthMechanicGenerationEpoch =
				room.m_GameplayCatalog.Get_ActiveGenerationEpoch();
			boss.iPhase = 1u;
			boss.PhasePolicy = nullptr == profile ?
				BOSS_PHASE_POLICY{} : profile->PhasePolicy;
			boss.fCollisionRadius =
				nullptr == profile ? 0.f : profile->fCollisionRadius;
			boss.fEngageDistance =
				nullptr == profile ? 20.f : profile->fEngageDistance;
			boss.fMoveSpeed = nullptr == profile ? 2.6f : profile->fMoveSpeed;
			boss.bIntroPatternConsumed = true;
			boss.strPatternId = "VALTAN_DASH_CHARGE";
			boss.strPatternStageId = "CHARGE";
			boss.strActionId = "valtan.attack.dash-charge.active";
			boss.iPatternSequence = patternSequence;
			boss.iPatternStageIndex = 1u;
			boss.iPatternStageDurationMs = 1500u;
			boss.iPatternStageFirstEvaluationTick = startTick;
			boss.ePatternStageMotionKind =
				BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD;
			boss.fPatternForcedMotionSpeed = 20.f / 1.5f;
			boss.fPatternMinimumRange = 5.f;
			boss.fPatternMaximumRange = 20.f;
			boss.bPatternChargeImpact = true;
			boss.iActionStartTick = startTick;
			boss.iPatternTargetEntityId = target.iNetEntityId;
			boss.bHasPatternTargetLastPosition = true;
			boss.fPatternTargetLastPositionX = target.fPositionX;
			boss.fPatternTargetLastPositionY = target.fPositionY;
			boss.fPatternTargetLastPositionZ = target.fPositionZ;
			boss.PinnedDefinitionRevision =
				room.m_GameplayCatalog.Get_ActiveRevision();
			boss.BossCombat.iStateRevision = 1u;
			room.m_WorldEntities.push_back(std::move(boss));
			room.m_iServerTick = startTick;
			return room.m_WorldEntities.back();
		};

		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto ordinaryWallRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& ordinaryWallRoom = *ordinaryWallRoomStorage;
		const BOSS_RUNTIME_PROFILE* ordinaryWallProfile =
			ordinaryWallRoom.m_GameplayCatalog.Find_Boss("BOSS_VALTAN");
		constexpr float FRESH_CENTER_X = 156.03f;
		constexpr float FRESH_CENTER_Y = 22.99751f;
		constexpr float FRESH_CENTER_Z = -122.06f;
		constexpr float FRESH_DASH_DISTANCE = 20.f;
		SERVER_BOSS_WALL_HIT ordinaryWallContact{};
		const bool foundOrdinaryWall = ordinaryWallRoom.Is_Ready() &&
			nullptr != ordinaryWallProfile &&
			ordinaryWallRoom.m_ServerCollisionSystem.
				Sweep_BossCircleAgainstWalls(
					FRESH_CENTER_X, FRESH_CENTER_Y, FRESH_CENTER_Z,
					FRESH_CENTER_X, FRESH_CENTER_Y,
					FRESH_CENTER_Z + FRESH_DASH_DISTANCE,
					ordinaryWallProfile->fCollisionRadius, ordinaryWallContact) &&
			!ordinaryWallContact.strCollisionPlacementId.empty() &&
			!ordinaryWallContact.strCollisionPlacementId.ends_with(".receiver");
		const float ordinaryWallContactDistance =
			FRESH_DASH_DISTANCE * ordinaryWallContact.fHitRatio;
		SERVER_WORLD_ENTITY& ordinaryWallBoss = stageDashCharge(
			ordinaryWallRoom, FRESH_CENTER_X, FRESH_CENTER_Y,
			FRESH_CENTER_Z, 0.f, 450u, 650u);
		(void)ordinaryWallBoss;
		bool ordinaryWallSameTick = false;
		for (std::uint32_t tick = 0u;
			tick < 45u && ordinaryWallRoom.m_isReady &&
			!ordinaryWallSameTick; ++tick)
		{
			const std::uint32_t updateTick =
				(std::numeric_limits<std::uint32_t>::max)() ==
					ordinaryWallRoom.m_iServerTick ?
				1u : ordinaryWallRoom.m_iServerTick + 1u;
			ordinaryWallRoom.Update_WorldEntities(1.f / 30.f);
			ordinaryWallRoom.m_iServerTick = updateTick;
			const SERVER_WORLD_ENTITY& liveBoss =
				ordinaryWallRoom.m_WorldEntities.front();
			const float travelledDistance =
				liveBoss.fPositionZ - FRESH_CENTER_Z;
			const auto states =
				ordinaryWallRoom.m_WorldDestructionRuntime.Get_GroupStates();
			const std::size_t breakingCount =
				static_cast<std::size_t>(std::count_if(
					states.begin(), states.end(),
					[](const WORLD_DESTRUCTION_GROUP_STATE& state)
					{
						return WORLD_DESTRUCTION_STATE::BREAKING == state.eState;
					}));
			ordinaryWallSameTick = 1u == breakingCount &&
				travelledDistance > 0.f &&
				travelledDistance < ordinaryWallContactDistance &&
				"VALTAN_DASH_CHARGE" == liveBoss.strPatternId &&
				"GROGGY" == liveBoss.strPatternStageId &&
				"valtan.attack.dash-charge.recovery" == liveBoss.strActionId &&
				2u == liveBoss.iPatternStageIndex &&
				650u == liveBoss.iPatternSequence &&
				!liveBoss.PendingPatternFollowup.Is_Pending() &&
				0u == liveBoss.iPatternFollowupDepth &&
				SERVER_ENTITY_ACTION::PATTERN_ACTIVE == liveBoss.eAction &&
				liveBoss.bPatternGroggy && !liveBoss.bPatternChargeImpact &&
				0.f == liveBoss.fPatternForcedMotionSpeed &&
				CBossCombatRuntime::Has_Flag(
					liveBoss.BossCombat,
					SERVER_BOSS_COMBAT_FLAG::GROGGY);
		}
		tests.Require(
			foundOrdinaryWall && ordinaryWallContactDistance > 0.f &&
			ordinaryWallContactDistance < FRESH_DASH_DISTANCE,
			"Resolve a fresh centre-origin Dash to the first ordinary collision wall before the outer receivers");
		tests.Require(
			ordinaryWallRoom.m_isReady && ordinaryWallSameTick,
			"Break the first ordinary wall, stop the Dash, and enter its same-pattern GROGGY on that fixed tick");
		const std::uint32_t ordinaryGroggySecondTick =
			ordinaryWallRoom.m_iServerTick + 1u;
		ordinaryWallRoom.Update_WorldEntities(1.f / 30.f);
		ordinaryWallRoom.m_iServerTick = ordinaryGroggySecondTick;
		const SERVER_WORLD_ENTITY& ordinaryGroggyBoss =
			ordinaryWallRoom.m_WorldEntities.front();
		const bool remainedInOrdinaryGroggy = ordinaryWallRoom.m_isReady &&
			!ordinaryGroggyBoss.PendingPatternFollowup.Is_Pending() &&
			"VALTAN_DASH_CHARGE" == ordinaryGroggyBoss.strPatternId &&
			"GROGGY" == ordinaryGroggyBoss.strPatternStageId &&
			"valtan.attack.dash-charge.recovery" == ordinaryGroggyBoss.strActionId &&
			650u == ordinaryGroggyBoss.iPatternSequence &&
			0u == ordinaryGroggyBoss.iPatternFollowupDepth &&
			ordinaryGroggyBoss.bPatternGroggy &&
			CBossCombatRuntime::Has_Flag(
				ordinaryGroggyBoss.BossCombat,
				SERVER_BOSS_COMBAT_FLAG::GROGGY);
		ordinaryWallRoom.Update_WorldEntities(1.f / 30.f);
		const SERVER_WORLD_ENTITY& ordinaryGroggyThirdTick =
			ordinaryWallRoom.m_WorldEntities.front();
		tests.Require(
			remainedInOrdinaryGroggy &&
			650u == ordinaryGroggyThirdTick.iPatternSequence &&
			"VALTAN_DASH_CHARGE" == ordinaryGroggyThirdTick.strPatternId &&
			"GROGGY" == ordinaryGroggyThirdTick.strPatternStageId,
			"Keep the same Dash occurrence in GROGGY without starting a second pattern");

		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto boundRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& boundRoom = *boundRoomStorage;
		const BOSS_RUNTIME_PROFILE* boundProfile =
			boundRoom.m_GameplayCatalog.Find_Boss("BOSS_VALTAN");
		/* The 109 transition ring's impact receivers can precede the ten 159
		   charge walls. Disable only those receivers here so this fixture isolates
		   the exact Dash -> 159 mutation join; the separate outer-bound fixture
		   below keeps the ring enabled and proves its real first-contact join. */
		std::vector<SERVER_COLLISION_STATE_CHANGE> outerReceiverChanges;
		for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
			boundRoom.m_WorldBootstrap.Get_Placements())
		{
			if (0u == placement.strPlacementId.rfind(
					"collision.valtan.wallgroup.sector", 0u) &&
				placement.strPlacementId.ends_with(".receiver"))
			{
				outerReceiverChanges.push_back(
					{ placement.strPlacementId, true, false });
			}
		}
		SERVER_COLLISION_STATE_STAGE isolatedReceiverStage{};
		std::string isolatedReceiverStatus;
		const bool isolatedBoundReceiver = 30u == outerReceiverChanges.size() &&
			boundRoom.m_ServerCollisionSystem.Prepare_StateChanges(
				outerReceiverChanges, isolatedReceiverStage,
				isolatedReceiverStatus);
		if (isolatedBoundReceiver)
			boundRoom.m_ServerCollisionSystem.Commit_StateChanges(
				std::move(isolatedReceiverStage));
		SERVER_BOSS_RECEIVER_HIT boundContact{};
		const bool foundBoundContact = boundRoom.Is_Ready() &&
			nullptr != boundProfile &&
			isolatedBoundReceiver &&
			boundRoom.m_ServerCollisionSystem.Sweep_BossCircleAgainstReceivers(
				145.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
				175.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
				boundProfile->fCollisionRadius, boundContact) &&
			0u == boundContact.strReceiverPlacementId.rfind(
				"collision.valtan.wallgroup.11047903315509031966.", 0u);
		const std::size_t boundReceiverSuffix =
			boundContact.strReceiverPlacementId.rfind(".receiver");
		const std::string boundCollisionId =
			std::string::npos == boundReceiverSuffix ? std::string{} :
			boundContact.strReceiverPlacementId.substr(0u, boundReceiverSuffix);
		std::vector<SERVER_COLLISION_STATE_CHANGE> otherWallChanges;
		for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
			boundRoom.m_WorldBootstrap.Get_Placements())
		{
			if (WORLD_BOOTSTRAP_KIND::COLLISION_BOX != placement.eKind ||
				placement.strPlacementId.ends_with(".receiver") ||
				placement.strPlacementId == boundCollisionId)
			{
				continue;
			}
			otherWallChanges.push_back(
				{ placement.strPlacementId, false, false });
		}
		SERVER_COLLISION_STATE_STAGE isolatedWallStage{};
		std::string isolatedWallStatus;
		const bool isolatedBoundWall = foundBoundContact &&
			98u == otherWallChanges.size() &&
			boundRoom.m_ServerCollisionSystem.Prepare_StateChanges(
				otherWallChanges, isolatedWallStage, isolatedWallStatus);
		if (isolatedBoundWall)
			boundRoom.m_ServerCollisionSystem.Commit_StateChanges(
				std::move(isolatedWallStage));
		const float boundContactX =
			145.f + 30.f * boundContact.fHitRatio;
		SERVER_WORLD_ENTITY& boundBoss = stageDashCharge(
			boundRoom, boundContactX - 0.2f, VALTAN_WALL_CENTER_Y,
			VALTAN_WALL_CENTER_Z, 90.f, 500u, 701u);
		const float boundStartX = boundBoss.fPositionX;
		const SERVER_WORLD_ENTITY consumedCharge = boundBoss;
		boundRoom.Update_WorldEntities(1.f / 30.f);
		const auto boundStates =
			boundRoom.m_WorldDestructionRuntime.Get_GroupStates();
		const std::size_t breakingCount = static_cast<std::size_t>(std::count_if(
			boundStates.begin(), boundStates.end(),
			[](const WORLD_DESTRUCTION_GROUP_STATE& state)
			{
				return WORLD_DESTRUCTION_STATE::BREAKING == state.eState;
			}));
		const SERVER_WORLD_ENTITY& recoveryBoss = boundRoom.m_WorldEntities.front();
		tests.Require(
			foundBoundContact && isolatedBoundWall &&
			boundRoom.m_isReady && 1u == breakingCount &&
			recoveryBoss.fPositionX >= boundStartX &&
			recoveryBoss.fPositionX < boundContactX &&
			"VALTAN_DASH_CHARGE" == recoveryBoss.strPatternId &&
			"GROGGY" == recoveryBoss.strPatternStageId &&
			"valtan.attack.dash-charge.recovery" == recoveryBoss.strActionId &&
			2u == recoveryBoss.iPatternStageIndex &&
			701u == recoveryBoss.iPatternSequence &&
			!recoveryBoss.PendingPatternFollowup.Is_Pending() &&
			SERVER_ENTITY_ACTION::PATTERN_ACTIVE == recoveryBoss.eAction &&
			recoveryBoss.bPatternGroggy && !recoveryBoss.bPatternChargeImpact &&
			CBossCombatRuntime::Has_Flag(
				recoveryBoss.BossCombat, SERVER_BOSS_COMBAT_FLAG::GROGGY),
			"Commit BREAKING and enter same-pattern GROGGY on the exact swept collision tick");

		SERVER_WORLD_ENTITY consumedProbe = consumedCharge;
		consumedProbe.iNetEntityId = 99004u;
		consumedProbe.iPatternSequence = 702u;
		bool consumedTriggered = true;
		const bool consumedAccepted = boundRoom.Apply_WorldDestructionImpact(
			consumedProbe, boundContact.strReceiverPlacementId,
			502u, consumedTriggered);
		tests.Require(
			consumedAccepted && !consumedTriggered &&
			"CHARGE" == consumedProbe.strPatternStageId &&
			!consumedProbe.bPatternGroggy,
			"Refuse a consumed dash receiver without changing the active CHARGE stage");

	#ifdef _DEBUG
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto outerRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& outerRoom = *outerRoomStorage;
		const BOSS_RUNTIME_PROFILE* outerProfile =
			outerRoom.m_GameplayCatalog.Find_Boss("BOSS_VALTAN");
		constexpr float ARENA_CENTER_X = 156.03f;
		constexpr float ARENA_CENTER_Y = 22.99751f;
		constexpr float ARENA_CENTER_Z = -122.06f;
		constexpr float DASH_DISTANCE = 20.f;
		SERVER_WORLD_ENTITY arenaStateOwner{};
		arenaStateOwner.iNetEntityId = 99005u;
		arenaStateOwner.iPatternSequence = 800u;
		WORLD_DESTRUCTION_TRANSACTION ordinaryWallsGone{};
		std::vector<std::string> expectedOrdinaryGoneGroupIds;
		std::string ordinaryArenaStatus;
		const bool preparedOrdinaryArena = outerRoom.Is_Ready() &&
			outerRoom.Prepare_ValtanTimelineArenaState(
				outerRoom.m_WorldDestructionRuntime, arenaStateOwner,
				VALTAN_TIMELINE_ARENA_STATE::ORDINARY_WALLS_GONE, 580u,
				ordinaryWallsGone, expectedOrdinaryGoneGroupIds,
				ordinaryArenaStatus);
		std::uint32_t ordinaryArenaCommitTick = 580u;
		for (const WORLD_DESTRUCTION_STATE_TRANSITION& transition :
			ordinaryWallsGone.Transitions)
		{
			ordinaryArenaCommitTick = (std::max)(
				ordinaryArenaCommitTick, transition.iCommitTick);
		}
		const bool committedOrdinaryArena = preparedOrdinaryArena &&
			69u == expectedOrdinaryGoneGroupIds.size() &&
			outerRoom.Commit_WorldDestructionTransaction(
				ordinaryWallsGone, {}, 580u, ordinaryArenaStatus) &&
			outerRoom.Commit_DueWorldDestruction(ordinaryArenaCommitTick);
		std::size_t ordinaryGoneCount = 0u;
		for (const std::string& groupId : expectedOrdinaryGoneGroupIds)
		{
			WORLD_DESTRUCTION_GROUP_STATE state{};
			if (outerRoom.m_WorldDestructionRuntime.Find_GroupState(
					groupId, state) &&
				WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState)
			{
				++ordinaryGoneCount;
			}
		}
		const auto stagedOuterStates =
			outerRoom.m_WorldDestructionRuntime.Get_GroupStates();
		const std::size_t intactOuterCount =
			static_cast<std::size_t>(std::count_if(
				stagedOuterStates.begin(), stagedOuterStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return 0u == state.strGroupId.rfind(
						"destroyable.group.valtan.outerwall109.", 0u) &&
						WORLD_DESTRUCTION_STATE::INTACT == state.eState;
				}));
		const bool ordinaryArenaReady = committedOrdinaryArena &&
			69u == ordinaryGoneCount && 30u == intactOuterCount;
		tests.Require(
			ordinaryArenaReady,
			"Stage the product ORDINARY_WALLS_GONE arena state while keeping all thirty outer-ring walls intact");

		/* The product timeline removes the 69 ordinary walls before this replay;
		   the canonical centre-to-positive-Z lane then reaches the intact 109
		   transition ring as its first swept receiver contact. */
		constexpr float OUTER_CONTACT_YAW_DEGREES = 0.f;
		constexpr float outerDirectionX = 0.f;
		constexpr float outerDirectionZ = 1.f;
		constexpr float outerEndX = ARENA_CENTER_X;
		constexpr float outerEndZ = ARENA_CENTER_Z + DASH_DISTANCE;
		SERVER_BOSS_RECEIVER_HIT outerContact{};
		const bool foundOuterContact = ordinaryArenaReady &&
			nullptr != outerProfile &&
			outerRoom.m_ServerCollisionSystem.Sweep_BossCircleAgainstReceivers(
				ARENA_CENTER_X, ARENA_CENTER_Y, ARENA_CENTER_Z,
				outerEndX, ARENA_CENTER_Y, outerEndZ,
				outerProfile->fCollisionRadius, outerContact) &&
			0u == outerContact.strReceiverPlacementId.rfind(
				"collision.valtan.wallgroup.sector", 0u);
		const std::size_t receiverSuffix =
			outerContact.strReceiverPlacementId.rfind(".receiver");
		const std::size_t memberSeparator =
			std::string::npos == receiverSuffix ? std::string::npos :
			outerContact.strReceiverPlacementId.rfind(
				'.', receiverSuffix - 1u);
		const std::string outerMemberId =
			std::string::npos == memberSeparator ||
			std::string::npos == receiverSuffix ?
			std::string{} : outerContact.strReceiverPlacementId.substr(
				memberSeparator + 1u,
				receiverSuffix - memberSeparator - 1u);
		const float outerContactDistance =
			DASH_DISTANCE * outerContact.fHitRatio;
		SERVER_WORLD_ENTITY& outerBoss = stageDashCharge(
			outerRoom, ARENA_CENTER_X, ARENA_CENTER_Y,
			ARENA_CENTER_Z, OUTER_CONTACT_YAW_DEGREES, 600u, 801u);
		(void)outerBoss;
		bool sameTickOuterImpact = false;
		for (std::uint32_t tick = 0u;
			tick < 45u && outerRoom.m_isReady && !sameTickOuterImpact; ++tick)
		{
			const std::uint32_t updateTick =
				(std::numeric_limits<std::uint32_t>::max)() ==
					outerRoom.m_iServerTick ?
				1u : outerRoom.m_iServerTick + 1u;
			/* Observe the fixed-tick collision transaction before Tick commits the
			   due destruction and advances BREAKING to DESPAWNED. This is the same
			   room update boundary used by the isolated 159 receiver fixture above. */
			outerRoom.Update_WorldEntities(1.f / 30.f);
			outerRoom.m_iServerTick = updateTick;
			const SERVER_WORLD_ENTITY& liveBoss =
				outerRoom.m_WorldEntities.front();
			const float travelledDistance =
				(liveBoss.fPositionX - ARENA_CENTER_X) * outerDirectionX +
				(liveBoss.fPositionZ - ARENA_CENTER_Z) * outerDirectionZ;
			const auto tickStates =
				outerRoom.m_WorldDestructionRuntime.Get_GroupStates();
			const std::size_t tickOuterBreakingCount =
				static_cast<std::size_t>(std::count_if(
					tickStates.begin(), tickStates.end(),
					[](const WORLD_DESTRUCTION_GROUP_STATE& state)
					{
						return 0u == state.strGroupId.rfind(
							"destroyable.group.valtan.outerwall109.", 0u) &&
							WORLD_DESTRUCTION_STATE::BREAKING == state.eState;
					}));
			sameTickOuterImpact = 1u == tickOuterBreakingCount &&
				travelledDistance > 0.f &&
				travelledDistance < outerContactDistance &&
				"VALTAN_DASH_CHARGE" == liveBoss.strPatternId &&
				"GROGGY" == liveBoss.strPatternStageId &&
				"valtan.attack.dash-charge.recovery" == liveBoss.strActionId &&
				2u == liveBoss.iPatternStageIndex &&
				801u == liveBoss.iPatternSequence &&
				!liveBoss.PendingPatternFollowup.Is_Pending() &&
				SERVER_ENTITY_ACTION::PATTERN_ACTIVE == liveBoss.eAction &&
				liveBoss.bPatternGroggy &&
				!liveBoss.bPatternChargeImpact &&
				0.f == liveBoss.fPatternForcedMotionSpeed &&
				CBossCombatRuntime::Has_Flag(
					liveBoss.BossCombat,
					SERVER_BOSS_COMBAT_FLAG::GROGGY);
		}
		const auto outerStates =
			outerRoom.m_WorldDestructionRuntime.Get_GroupStates();
		const std::size_t outerBreakingCount =
			static_cast<std::size_t>(std::count_if(
				outerStates.begin(), outerStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return WORLD_DESTRUCTION_STATE::BREAKING == state.eState;
				}));
		tests.Require(
			foundOuterContact && !outerMemberId.empty(),
			"Resolve the centre-origin Dash Charge's first swept contact to one stable outer-ring receiver");
		tests.Require(
			outerRoom.m_isReady && sameTickOuterImpact,
			"Stop the centre-origin Dash Charge at its first outer-ring receiver and commit BREAKING plus same-pattern GROGGY on that fixed tick");
		tests.Require(
			1u == outerBreakingCount,
			"Break exactly one outer-ring wall from the centre-origin Dash Charge");

	#endif

		/* This contract deliberately stays outside _DEBUG. First put one real
		   outer receiver into BREAKING, then expose its receiver again to emulate
		   a stale collision frame. The one-way mutation still answers NO_CHANGE,
		   but the intact collision surface is a real wall contact and must end the
		   charge in RECOVERY. The same consumed group then proves the later 109 stage
		   admits only the other twenty-nine in Release as well. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto partialOuterRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& partialOuterRoom = *partialOuterRoomStorage;
		const BOSS_RUNTIME_PROFILE* partialOuterProfile =
			partialOuterRoom.m_GameplayCatalog.Find_Boss("BOSS_VALTAN");
		constexpr float PARTIAL_ARENA_CENTER_X = 156.03f;
		constexpr float PARTIAL_ARENA_CENTER_Y = 22.99751f;
		constexpr float PARTIAL_ARENA_CENTER_Z = -122.06f;
		constexpr float PARTIAL_DASH_DISTANCE = 20.f;
		SERVER_BOSS_RECEIVER_HIT partialOuterContact{};
		const bool foundPartialOuterContact = partialOuterRoom.Is_Ready() &&
			nullptr != partialOuterProfile &&
			partialOuterRoom.m_ServerCollisionSystem.
				Sweep_BossCircleAgainstReceivers(
					PARTIAL_ARENA_CENTER_X, PARTIAL_ARENA_CENTER_Y,
					PARTIAL_ARENA_CENTER_Z, PARTIAL_ARENA_CENTER_X,
					PARTIAL_ARENA_CENTER_Y,
					PARTIAL_ARENA_CENTER_Z + PARTIAL_DASH_DISTANCE,
					partialOuterProfile->fCollisionRadius, partialOuterContact) &&
			0u == partialOuterContact.strReceiverPlacementId.rfind(
				"collision.valtan.wallgroup.sector", 0u);
		const std::size_t partialReceiverSuffix =
			partialOuterContact.strReceiverPlacementId.rfind(".receiver");
		const std::size_t partialMemberSeparator =
			std::string::npos == partialReceiverSuffix ? std::string::npos :
			partialOuterContact.strReceiverPlacementId.rfind(
				'.', partialReceiverSuffix - 1u);
		const std::string partialOuterMemberId =
			std::string::npos == partialMemberSeparator ||
			std::string::npos == partialReceiverSuffix ?
			std::string{} : partialOuterContact.strReceiverPlacementId.substr(
				partialMemberSeparator + 1u,
				partialReceiverSuffix - partialMemberSeparator - 1u);
		const std::string partialOuterGroupId = partialOuterMemberId.empty() ?
			std::string{} :
			"destroyable.group.valtan.outerwall109." + partialOuterMemberId;
		const std::string partialOuterCollisionId =
			std::string::npos == partialReceiverSuffix ? std::string{} :
			partialOuterContact.strReceiverPlacementId.substr(0u, partialReceiverSuffix);

		SERVER_WORLD_ENTITY partialImpactOwner{};
		partialImpactOwner.iNetEntityId = 99006u;
		partialImpactOwner.strPatternId = "VALTAN_DASH_CHARGE";
		partialImpactOwner.strPatternStageId = "CHARGE";
		partialImpactOwner.strActionId = "valtan.attack.dash-charge.active";
		partialImpactOwner.iPatternStageIndex = 1u;
		partialImpactOwner.iPatternSequence = 900u;
		partialImpactOwner.fPositionX = PARTIAL_ARENA_CENTER_X;
		partialImpactOwner.fPositionY = PARTIAL_ARENA_CENTER_Y;
		partialImpactOwner.fPositionZ = PARTIAL_ARENA_CENTER_Z;
		partialImpactOwner.fYawDegrees = 0.f;
		bool partialImpactTriggered = false;
		WORLD_DESTRUCTION_GROUP_STATE partialOuterState{};
		const bool consumedPartialOuter = foundPartialOuterContact &&
			!partialOuterGroupId.empty() &&
			partialOuterRoom.Apply_WorldDestructionImpact(
				partialImpactOwner,
				partialOuterContact.strReceiverPlacementId,
				880u, partialImpactTriggered) &&
			partialImpactTriggered &&
			partialOuterRoom.m_WorldDestructionRuntime.Find_GroupState(
				partialOuterGroupId, partialOuterState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == partialOuterState.eState &&
			0u != partialOuterState.iCommitTick;

		/* Isolate the actual source/receiver pair for this consumed-contact
		   probe. Keeping the other ordinary walls here used to let an unrelated
		   first hit manufacture the expected RECOVERY result. Restore the complete
		   collision state before the later 109 batch exercises the whole arena. */
		CServerCollisionSystem collisionBeforeStaleReceiver =
			partialOuterRoom.m_ServerCollisionSystem;
		CServerCollisionSystem isolatedStaleReceiver;
		std::string staleReceiverStatus;
		std::vector<WORLD_BOOTSTRAP_PLACEMENT> staleReceiverPlacements;
		for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
			partialOuterRoom.m_WorldBootstrap.Get_Placements())
		{
			if (WORLD_BOOTSTRAP_KIND::COLLISION_BOX == placement.eKind &&
				placement.isEnabled &&
				(placement.strPlacementId == partialOuterCollisionId ||
				 placement.strPlacementId == partialOuterContact.strReceiverPlacementId))
			{
				staleReceiverPlacements.push_back(placement);
			}
		}
		const bool stagedStaleReceiver = consumedPartialOuter &&
			2u == staleReceiverPlacements.size() &&
			isolatedStaleReceiver.Initialize(staleReceiverPlacements, staleReceiverStatus);
		if (stagedStaleReceiver)
		{
			partialOuterRoom.m_ServerCollisionSystem = std::move(isolatedStaleReceiver);
		}
		tests.Require(
			consumedPartialOuter,
			"Put one exact outer Dash receiver into BREAKING before the stale-receiver probe");
		tests.Require(
			stagedStaleReceiver &&
			2u == partialOuterRoom.m_ServerCollisionSystem.Get_ActivePlayerBlockingCount() &&
			partialOuterRoom.m_ServerCollisionSystem.Is_PlayerBlocking(partialOuterCollisionId) &&
			partialOuterRoom.m_ServerCollisionSystem.Is_ImpactReceiverEnabled(
				partialOuterContact.strReceiverPlacementId),
			"Isolate the consumed outer source and re-expose only its exact receiver for the safe-stop probe");
		const float partialContactZ = PARTIAL_ARENA_CENTER_Z +
			PARTIAL_DASH_DISTANCE * partialOuterContact.fHitRatio;
		SERVER_WORLD_ENTITY& staleReceiverBoss = stageDashCharge(
			partialOuterRoom, PARTIAL_ARENA_CENTER_X,
			PARTIAL_ARENA_CENTER_Y, partialContactZ - 0.2f,
			0.f, 880u, 901u);
		const float staleReceiverStartZ = staleReceiverBoss.fPositionZ;
		float staleProposedX = 0.f;
		float staleProposedZ = 0.f;
		SERVER_BOSS_WALL_HIT exactStaleContact{};
		const bool hitExactConsumedReceiver = stagedStaleReceiver &&
			partialOuterRoom.m_ValtanBrain.Try_BuildStageMotion(
				staleReceiverBoss, 1.f / 30.f, staleProposedX, staleProposedZ) &&
			partialOuterRoom.m_ServerCollisionSystem.Sweep_BossCircleAgainstWalls(
				staleReceiverBoss.fPositionX, staleReceiverBoss.fPositionY,
				staleReceiverBoss.fPositionZ, staleProposedX,
				staleReceiverBoss.fPositionY, staleProposedZ,
				staleReceiverBoss.fCollisionRadius, exactStaleContact) &&
			exactStaleContact.strCollisionPlacementId == partialOuterCollisionId &&
			exactStaleContact.strImpactReceiverPlacementId ==
				partialOuterContact.strReceiverPlacementId &&
			exactStaleContact.fHitRatio > 0.f && exactStaleContact.fHitRatio < 1.f;
		tests.Require(
			hitExactConsumedReceiver,
			"Sweep the consumed outer source and receiver identity before the real Dash room tick");
		const std::uint64_t destructionSequenceBeforeStaleContact =
			partialOuterRoom.m_iNextWorldDestructionEventSequence;
		partialOuterRoom.Update_WorldEntities(1.f / 30.f);
		const SERVER_WORLD_ENTITY& safeStoppedBoss =
			partialOuterRoom.m_WorldEntities.front();
		WORLD_DESTRUCTION_GROUP_STATE safeStoppedOuterState{};
		const bool stoppedOnConsumedOuter = hitExactConsumedReceiver &&
			partialOuterRoom.m_isReady &&
			partialOuterRoom.m_ServerCollisionSystem.Is_ImpactReceiverEnabled(
				partialOuterContact.strReceiverPlacementId) &&
			partialOuterRoom.m_WorldDestructionRuntime.Find_GroupState(
				partialOuterGroupId, safeStoppedOuterState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == safeStoppedOuterState.eState &&
			safeStoppedOuterState.iStateVersion == partialOuterState.iStateVersion &&
			safeStoppedOuterState.iStateStartTick == partialOuterState.iStateStartTick &&
			safeStoppedOuterState.iCommitTick == partialOuterState.iCommitTick &&
			safeStoppedOuterState.strPendingMutationId == partialOuterState.strPendingMutationId &&
			destructionSequenceBeforeStaleContact ==
				partialOuterRoom.m_iNextWorldDestructionEventSequence &&
			safeStoppedBoss.fPositionZ >= staleReceiverStartZ &&
			safeStoppedBoss.fPositionZ < partialContactZ &&
			0.f == safeStoppedBoss.fPatternForcedMotionSpeed &&
			"VALTAN_DASH_CHARGE" == safeStoppedBoss.strPatternId &&
			"GROGGY" == safeStoppedBoss.strPatternStageId &&
			"valtan.attack.dash-charge.recovery" == safeStoppedBoss.strActionId &&
			2u == safeStoppedBoss.iPatternStageIndex &&
			901u == safeStoppedBoss.iPatternSequence &&
			!safeStoppedBoss.PendingPatternFollowup.Is_Pending() &&
			SERVER_ENTITY_ACTION::PATTERN_ACTIVE == safeStoppedBoss.eAction &&
			!safeStoppedBoss.bPatternChargeImpact &&
			safeStoppedBoss.bPatternGroggy &&
			CBossCombatRuntime::Has_Flag(
				safeStoppedBoss.BossCombat,
				SERVER_BOSS_COMBAT_FLAG::GROGGY);
		tests.Require(
			partialOuterRoom.m_isReady &&
			partialOuterRoom.m_WorldDestructionRuntime.Find_GroupState(
				partialOuterGroupId, safeStoppedOuterState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == safeStoppedOuterState.eState,
			"Keep the consumed outer mutation in BREAKING after its repeat swept contact");
		tests.Require(
			safeStoppedBoss.fPositionZ >= staleReceiverStartZ &&
			safeStoppedBoss.fPositionZ < partialContactZ &&
			0.f == safeStoppedBoss.fPatternForcedMotionSpeed,
			"Clamp the repeat Dash to its swept safe position and zero its forced speed");
		tests.Require(
			!safeStoppedBoss.PendingPatternFollowup.Is_Pending() &&
			"VALTAN_DASH_CHARGE" == safeStoppedBoss.strPatternId &&
			"GROGGY" == safeStoppedBoss.strPatternStageId &&
			!safeStoppedBoss.bPatternChargeImpact &&
			safeStoppedBoss.bPatternGroggy &&
			CBossCombatRuntime::Has_Flag(
				safeStoppedBoss.BossCombat,
				SERVER_BOSS_COMBAT_FLAG::GROGGY),
			"Enter same-pattern GROGGY on a consumed receiver's still-blocking wall surface");
		tests.Require(
			stoppedOnConsumedOuter,
			"Stop at the swept safe position and enter GROGGY without repeating the consumed mutation");
		const std::size_t collisionCountBeforeStaleReceiver =
			collisionBeforeStaleReceiver.Get_CollisionBoxCount();
		partialOuterRoom.m_ServerCollisionSystem = std::move(collisionBeforeStaleReceiver);
		const bool restoredOuterCollisionState = stoppedOnConsumedOuter &&
			collisionCountBeforeStaleReceiver ==
				partialOuterRoom.m_ServerCollisionSystem.Get_CollisionBoxCount() &&
			partialOuterRoom.m_ServerCollisionSystem.Is_PlayerBlocking(partialOuterCollisionId) &&
			!partialOuterRoom.m_ServerCollisionSystem.Is_ImpactReceiverEnabled(
				partialOuterContact.strReceiverPlacementId);
		tests.Require(
			restoredOuterCollisionState,
			"Restore the complete arena collision state and consumed receiver before the partial 109 batch");

		const std::uint32_t partialOuterCommitTick =
			consumedPartialOuter ? partialOuterState.iCommitTick : 0u;
		const bool committedPartialOuter = restoredOuterCollisionState &&
			partialOuterRoom.Commit_DueWorldDestruction(
				partialOuterCommitTick) &&
			partialOuterRoom.m_WorldDestructionRuntime.Find_GroupState(
				partialOuterGroupId, partialOuterState) &&
			WORLD_DESTRUCTION_STATE::DESPAWNED == partialOuterState.eState;
		const auto partialStatesBeforeArenaBreak =
			partialOuterRoom.m_WorldDestructionRuntime.Get_GroupStates();
		SERVER_WORLD_ENTITY partialArenaBreakBoss = safeStoppedBoss;
		partialArenaBreakBoss.strPatternId = "VALTAN_ARENA_BREAK_109";
		partialArenaBreakBoss.strPatternStageId = "IMPACT";
		partialArenaBreakBoss.strActionId =
			"valtan.mechanic.arena-break-109.impact";
		partialArenaBreakBoss.iPatternStageIndex = 2u;
		partialArenaBreakBoss.iPatternSequence = 902u;
		const std::uint32_t partialArenaBreakTick =
			partialOuterCommitTick + 1u;
		const bool appliedPartialArenaBreak = committedPartialOuter &&
			partialOuterRoom.Apply_WorldDestructionStageEntry(
				partialArenaBreakBoss, partialArenaBreakTick);
		const auto partialStatesAfterArenaBreak =
			partialOuterRoom.m_WorldDestructionRuntime.Get_GroupStates();
		std::size_t partialOuterBreakingCount = 0u;
		std::size_t partialOuterDespawnedCount = 0u;
		bool partialConsumedStayedDespawned = false;
		std::uint32_t partialArenaCommitTick = partialArenaBreakTick;
		for (const WORLD_DESTRUCTION_GROUP_STATE& state :
			partialStatesAfterArenaBreak)
		{
			if (0u != state.strGroupId.rfind(
					"destroyable.group.valtan.outerwall109.", 0u))
			{
				continue;
			}
			if (WORLD_DESTRUCTION_STATE::BREAKING == state.eState)
			{
				++partialOuterBreakingCount;
				partialArenaCommitTick = (std::max)(
					partialArenaCommitTick, state.iCommitTick);
			}
			if (WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState)
				++partialOuterDespawnedCount;
			if (partialOuterGroupId == state.strGroupId)
			{
				partialConsumedStayedDespawned =
					WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState;
			}
		}
		const bool partialUnrelatedGroupsUnchanged =
			partialStatesBeforeArenaBreak.size() ==
				partialStatesAfterArenaBreak.size() &&
			std::all_of(
				partialStatesAfterArenaBreak.begin(),
				partialStatesAfterArenaBreak.end(),
				[&partialStatesBeforeArenaBreak](
					const WORLD_DESTRUCTION_GROUP_STATE& after)
				{
					/* The 109 batch owns the outer ring and the interior walls, so
					only they may move here. What must survive the collapse is the
					floor, which its own 84/30 patterns drop later, and the entrance
					walls the first-appearance sweep already owns. */
					if (0u == after.strGroupId.rfind(
							"destroyable.group.valtan.outerwall109.", 0u) ||
					0u == after.strGroupId.rfind(
							"destroyable.group.valtan.wall159.", 0u) ||
					0u == after.strGroupId.rfind(
							"destroyable.group.valtan.wall.", 0u))
					{
						return true;
					}
					const auto beforeIt = std::find_if(
						partialStatesBeforeArenaBreak.begin(),
						partialStatesBeforeArenaBreak.end(),
						[&after](const WORLD_DESTRUCTION_GROUP_STATE& before)
						{
							return before.strGroupId == after.strGroupId;
						});
					return partialStatesBeforeArenaBreak.end() != beforeIt &&
						beforeIt->eState == after.eState &&
						beforeIt->iStateVersion == after.iStateVersion &&
						beforeIt->iStateStartTick == after.iStateStartTick &&
						beforeIt->iCommitTick == after.iCommitTick &&
						beforeIt->strPendingMutationId ==
							after.strPendingMutationId;
				});
		tests.Require(
			committedPartialOuter,
			"Commit the consumed outer wall from BREAKING to DESPAWNED at its due tick in every configuration");
		tests.Require(
			partialOuterRoom.m_isReady && appliedPartialArenaBreak,
			"Accept the later 109 impact after one shared outer mutation is already final in every configuration");
		tests.Require(
			partialConsumedStayedDespawned &&
			1u == partialOuterDespawnedCount &&
			29u == partialOuterBreakingCount,
			"Keep one consumed outer wall DESPAWNED and commit only the other twenty-nine to BREAKING");
		tests.Require(
			partialUnrelatedGroupsUnchanged,
			"Leave every floor sector and entrance wall unchanged across the Release-safe partial 109 batch");
		const bool committedPartialArena = appliedPartialArenaBreak &&
			partialOuterRoom.Commit_DueWorldDestruction(
				partialArenaCommitTick);
		const auto partialFinalStates =
			partialOuterRoom.m_WorldDestructionRuntime.Get_GroupStates();
		const std::size_t partialFinalDespawnedCount =
			static_cast<std::size_t>(std::count_if(
				partialFinalStates.begin(), partialFinalStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return 0u == state.strGroupId.rfind(
							"destroyable.group.valtan.outerwall109.", 0u) &&
						WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState;
				}));
		tests.Require(
			committedPartialArena && 30u == partialFinalDespawnedCount,
			"Finish the Release-safe partial 109 batch with all thirty outer walls DESPAWNED");
	}

	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		SERVER_WORLD_ENTITY boss{};
		boss.iNetEntityId = 7003u;
		/* Stand in front of the wall instead of inside it.  These are the exact
		   DOWN_SMASH authored proxy semantics compiled above: a ten-metre cross
		   with a 1.8-metre half-width. */
		boss.fPositionX = VALTAN_WALL_CENTER_X;
		boss.fPositionY = VALTAN_WALL_CENTER_Y;
		boss.fPositionZ = VALTAN_WALL_CENTER_Z - 4.f;
		boss.fYawDegrees = 0.f;
		boss.fCollisionRadius = 1.f;
		boss.ePatternHitShape = BOSS_PATTERN_HIT_SHAPE::CROSS;
		boss.fPatternHitLength = 10.f;
		boss.fPatternHitHalfWidth = 1.8f;
		WORLD_DESTRUCTION_GROUP_STATE groupState{};
		const std::string groupId =
			"destroyable.group.valtan.wall159.15719065619666776634";
		const bool initiallyIntact = room.Is_Ready() &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				groupId, groupState) &&
			WORLD_DESTRUCTION_STATE::INTACT == groupState.eState;
		boss.bPatternWallContact = false;
		const bool refusedUnmarkedHit =
			room.Apply_WorldDestructionPatternHitContact(boss, 700u) &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				groupId, groupState) &&
			WORLD_DESTRUCTION_STATE::INTACT == groupState.eState;
		boss.bPatternWallContact = true;
		std::vector<std::string> observedAxeContacts;
		room.m_ServerCollisionSystem.Collect_BossPatternHitContacts(
			boss.ePatternHitShape,
			boss.fPositionX, boss.fPositionY, boss.fPositionZ,
			boss.fYawDegrees, boss.fCollisionRadius,
			boss.fPatternHitOuterRadius, boss.fPatternHitInnerRadius,
			boss.fPatternHitAngleDegrees, boss.fPatternHitLength,
			boss.fPatternHitHalfWidth, observedAxeContacts);
		const bool foundTargetContact = observedAxeContacts.end() != std::find(
			observedAxeContacts.begin(), observedAxeContacts.end(),
			VALTAN_WALL_COLLISION_STATE);
		const bool acceptedAxeHit =
			room.Apply_WorldDestructionPatternHitContact(boss, 701u) &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				groupId, groupState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == groupState.eState &&
			room.m_ServerCollisionSystem.Is_PlayerBlocking(
				VALTAN_WALL_COLLISION_STATE);
		tests.Require(initiallyIntact,
			"Start the axe-contact wall integration from INTACT");
		tests.Require(refusedUnmarkedHit,
			"Keep a physical hit volume harmless when its action is not wall-contact authored");
		tests.Require(foundTargetContact,
			"Resolve the authored down-smash axe proxy against the wall in front of Valtan");
		tests.Require(acceptedAxeHit,
			"Commit BREAKING when an authored axe hit volume touches one wall collider");
		const bool committedAxeHit = room.Commit_DueWorldDestruction(709u) &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				groupId, groupState) &&
			WORLD_DESTRUCTION_STATE::DESPAWNED == groupState.eState &&
			!room.m_ServerCollisionSystem.Is_PlayerBlocking(
				VALTAN_WALL_COLLISION_STATE) &&
			room.m_ServerNavigation.Is_PointWalkableExact(
				VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z);
		tests.Require(committedAxeHit,
			"Gate axe wall contact by authored action and atomically open its collision and navigation at the due tick");
	}
}

