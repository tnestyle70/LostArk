#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "Gameplay/WorldCollisionContract.h"
#include "GameplayCatalog.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ValtanBrain.h"
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

int LostArk::Server::CServerGameplayContractRunner::Run(
    CONTRACT_TEST_RUN_CONTEXT& context)
{
    const bool dimensionMasterGroundTargetOnly = context.dimensionMasterGroundTargetOnly;

	using namespace LostArk::Shared;
	TESTS tests{ dimensionMasterGroundTargetOnly };
	if (context.worldPlaybackOnly)
		return Run_WorldPlayback(tests);

	if (context.debugTeleportOnly)
		return Run_DebugTeleport(tests);

	CGameplayCatalog catalog;
	const bool catalogLoaded = catalog.Load();
	if (!catalogLoaded)
	{
		std::cout << "[STATUS] " << catalog.Get_Status() << std::endl;
		if (dimensionMasterGroundTargetOnly)
			tests.Require_GroundTarget(
				false, "Load gameplay balance bootstrap for DimensionMaster T");
		else
			tests.Require(false, "Load gameplay balance bootstrap");
		std::cout << "failures : " << tests.failures << '\n';
		return 1;
	}
	if (dimensionMasterGroundTargetOnly)
		tests.Require_GroundTarget(
			catalogLoaded, "Load gameplay balance bootstrap for DimensionMaster T");
	else
		tests.Require(catalogLoaded, "Load gameplay balance bootstrap");
	tests.Require(
		catalog.Get_ActiveRevision().Is_Valid(),
		"Derive a nonzero gameplay data revision from admitted bootstrap bytes");
	Run_KoukuBundles(tests);

	if (context.koukuBundlesOnly) { std::cout << "failures : " << tests.failures << '\n'; return tests.failures == 0 ? 0 : 1; }
	Run_KoukuProduct(tests, catalog);

	Run_KoukuSaydonLogicRuntimeContracts(tests, catalog);
	Run_KoukuObjectOverlapContracts(tests, catalog);
	Run_KoukuObjectContactContracts(tests, catalog);
	Run_KoukuFearAndCounterContracts(tests, catalog);
	Run_KoukuWorldPlacementContracts(tests, catalog);
	Run_KoukuBoneContactContracts(tests, catalog);
	Run_ValtanLifecycle(tests, catalog);

	Run_ValtanPinnedGeneration(tests, catalog);

	Run_ValtanRevision(tests, catalog);

	Run_RevisionProtocol(tests);

	Run_GenerationRetention(tests, catalog);

	Run_SessionTransport(tests);

	Run_RoomIngress(tests);

	Run_CharacterAdmission(tests, catalog);

	Run_GroundTarget(tests, catalog);

	if (dimensionMasterGroundTargetOnly)
	{
		std::cout << "failures : " << tests.failures << '\n';
		return 0 == tests.failures ? 0 : 1;
	}
	Run_PlayerCombos(tests, catalog);


	CServerNavigation navigation;
	CWorldBootstrap world;
	tests.Require(world.Load(WORLD_ID::VALTAN_ARENA) &&
		world.Get_AreaId() == "LV_LUT_HEARTRB_ED",
		"Preserve world area ID across placement parsing");
	tests.Require(
		MAX_VALTAN_RAID_PLAYERS == static_cast<std::size_t>(std::count_if(
			world.Get_Placements().begin(),
			world.Get_Placements().end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.isEnabled;
			})),
		"Load exactly the configured Valtan raid capacity as enabled player spawns");
	tests.Require(navigation.Load("LV_LUT_HEARTRB_ED"),
		"Load Valtan server navigation");
	std::vector<SERVER_NAV_POINT> path;
	/* Both endpoints are open arena floor on the same connected component. The
	   old start sat outside the completed 109 ring, which only pathed while the
	   ring still had its six-slab gap; the arena interior is now sealed until
	   the 109 collapse, so the test walks a route that stays inside it. */
	tests.Require(navigation.Find_Path(147.75f, -117.25f, 156.25f, -122.25f, path) &&
		!path.empty(), "Find authoritative navigation path");
	SERVER_NAV_POINT rejected{};
	tests.Require(!navigation.Project_Point(10000.f, 10000.f, rejected),
		"Reject navigation point outside projection radius");

	/* Walk out from a point the path test already proved walkable until the grid
	stops answering, so the boundary is whatever the current bake says it is
	rather than a cell index frozen into this file. */
	const float navCellSize = navigation.Get_CellSize();
	const float boundaryProbeStep = navCellSize * 0.25f;
	const float boundaryProbeZ = -137.f;
	const float boundaryProbeOriginX = 152.f;
	const float boundaryProbeLimitX = boundaryProbeOriginX + 60.f;
	float lastWalkableX = boundaryProbeOriginX;
	float firstBlockedX = 0.f;
	bool foundBoundary = false;
	SERVER_NAV_POINT boundaryProbe{};
	for (float probeX = boundaryProbeOriginX + boundaryProbeStep;
		probeX < boundaryProbeLimitX;
		probeX += boundaryProbeStep)
	{
		if (!navigation.Sample_Position(probeX, boundaryProbeZ, boundaryProbe))
		{
			firstBlockedX = probeX;
			foundBoundary = true;
			break;
		}
		lastWalkableX = probeX;
	}
	tests.Require(
		navCellSize > 0.f && foundBoundary,
		"Find a Valtan navigation boundary to clamp root motion against");

	SERVER_NAV_POINT rootMotionStop{ 0.f, 0.f, 0.f };
	bool rootMotionClamped = false;
	CPlayerSkillSystem::Clamp_StepToWalkable(
		navigation,
		lastWalkableX,
		boundaryProbeZ,
		firstBlockedX + navCellSize,
		boundaryProbeZ,
		rootMotionStop,
		rootMotionClamped);
	tests.Require(
		rootMotionClamped &&
		rootMotionStop.x >= lastWalkableX &&
		rootMotionStop.x < firstBlockedX &&
		firstBlockedX - rootMotionStop.x < navCellSize &&
		std::abs(rootMotionStop.z - boundaryProbeZ) < 0.001f,
		"Stop root motion against the Valtan non-walkable boundary");

	SERVER_NAV_POINT rootMotionOpen{ 0.f, 0.f, 0.f };
	bool openClamped = true;
	CPlayerSkillSystem::Clamp_StepToWalkable(
		navigation,
		152.f,
		boundaryProbeZ,
		152.f + boundaryProbeStep,
		boundaryProbeZ,
		rootMotionOpen,
		openClamped);
	tests.Require(
		!openClamped &&
		std::abs(rootMotionOpen.x - (152.f + boundaryProbeStep)) < 0.001f,
		"Preserve root motion that stays on walkable navigation");

	/* Keep marching past the blocked band to the open floor behind it. A step
	that spans the whole band is the case a plain bisection would wave through,
	so the clamp has to answer with the near wall, not the far side. */
	float farSideX = 0.f;
	bool foundFarSide = false;
	for (float probeX = firstBlockedX + boundaryProbeStep;
		probeX < firstBlockedX + 60.f;
		probeX += boundaryProbeStep)
	{
		if (navigation.Sample_Position(probeX, boundaryProbeZ, boundaryProbe))
		{
			farSideX = probeX;
			foundFarSide = true;
			break;
		}
	}
	tests.Require(foundFarSide,
		"Find walkable navigation beyond the blocked band");
	SERVER_NAV_POINT rootMotionTunnel{ 0.f, 0.f, 0.f };
	bool tunnelClamped = false;
	CPlayerSkillSystem::Clamp_StepToWalkable(
		navigation,
		lastWalkableX,
		boundaryProbeZ,
		farSideX,
		boundaryProbeZ,
		rootMotionTunnel,
		tunnelClamped);
	tests.Require(
		tunnelClamped &&
		rootMotionTunnel.x >= lastWalkableX &&
		rootMotionTunnel.x < firstBlockedX,
		"Refuse root motion that would cross a blocked band in one step");

	constexpr const char* VALTAN_WALL_RECEIVER =
		"collision.valtan.wallgroup.11047903315509031966.15719065619666776634.receiver";
	constexpr const char* VALTAN_WALL_COLLISION_STATE =
		"collision.valtan.wallgroup.11047903315509031966.15719065619666776634";
	constexpr const char* VALTAN_WALL_CONDITION =
		"condition.valtan.wall159.15719065619666776634.destroyed";
	constexpr float VALTAN_WALL_CENTER_X = 161.402061f;
	constexpr float VALTAN_WALL_CENTER_Y = 23.04f;
	constexpr float VALTAN_WALL_CENTER_Z = -133.312236f;
	/* The approach used to stand outside the ring and reach the charge wall
	through the six-slab gap. The completed 109 ring seals that gap until the
	collapse, so the sweep now starts inside the arena: it still crosses the
	charge wall's boxes and touches no ring slab, which keeps this test about
	the 159 wall's own collision state instead of the ring's. */
	constexpr float VALTAN_WALL_APPROACH_X = 164.25f;
	constexpr float VALTAN_WALL_APPROACH_Z = -125.25f;
	constexpr float VALTAN_WALL_EXIT_X = 159.488644f;
	constexpr float VALTAN_WALL_EXIT_Z = -128.692839f;
	std::string dynamicWorldStatus;
	SERVER_NAVIGATION_CONDITION_STAGE navigationStage{};
	std::vector<SERVER_NAV_POINT> wallPassagePath;
	tests.Require(
		navigation.Has_Condition(VALTAN_WALL_CONDITION) &&
		!navigation.Is_PointWalkableExact(
			VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z) &&
		!navigation.Find_Path(
			160.25f, -130.75f, 162.25f, -135.75f,
			wallPassagePath),
		"Keep the intact Valtan wall footprint and its cross-wall path dynamically blocked");
	const std::uint64_t navigationRevisionBeforeReject =
		navigation.Get_Revision();
	tests.Require(
		!navigation.Prepare_ConditionChanges(
			{ { "condition.valtan.wall.unknown", true } },
			navigationStage, dynamicWorldStatus) &&
		navigationRevisionBeforeReject == navigation.Get_Revision() &&
		!navigation.Is_PointWalkableExact(
			VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z),
		"Reject an unknown navigation condition without changing the live blocker");
	tests.Require(
		navigation.Prepare_ConditionChanges(
			{ { VALTAN_WALL_CONDITION, true } },
			navigationStage, dynamicWorldStatus) &&
		navigationStage.bChanged &&
		navigationStage.iNextRevision == navigation.Get_Revision() + 1u,
		"Stage one runtime navigation condition without mutating the live grid");
	navigation.Commit_ConditionChanges(std::move(navigationStage));
	tests.Require(
		navigation.Is_PointWalkableExact(
			VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z) &&
		!navigation.Find_Path(
			160.25f, -130.75f, 162.25f, -135.75f,
			wallPassagePath),
		"Expose only the selected wall cells while adjacent independent walls keep the full barrier closed");
	navigation.Reset_RuntimeBlockers();
	tests.Require(
		!navigation.Is_PointWalkableExact(
			VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z) &&
		!navigation.Find_Path(
			160.25f, -130.75f, 162.25f, -135.75f,
			wallPassagePath),
		"Restore the intact Valtan wall blocker and closed path on encounter reset");

	CServerCollisionSystem valtanCollisionSystem;
	tests.Require(
		valtanCollisionSystem.Initialize(
			world.Get_Placements(), dynamicWorldStatus) &&
		/* 69 interior wall boxes plus ten independent 159 impact receivers,
		one box per 109 outer ring slab and one impact receiver twinning each
		of those thirty, and the two entrance front walls' own receivers. */
		141u == valtanCollisionSystem.Get_CollisionBoxCount() &&
		valtanCollisionSystem.Has_CollisionBox(VALTAN_WALL_RECEIVER),
		"Load the stable Valtan wall impact receiver and player blocker");
	{
		WORLD_BOOTSTRAP_PLACEMENT wall{};
		wall.strPlacementId = "collision.contract.axe.wall";
		wall.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX;
		wall.isEnabled = true;
		wall.fPositionX = 0.f;
		wall.fPositionY = 2.f;
		wall.fPositionZ = 5.f;
		wall.fYawDegrees = 30.f;
		wall.fHalfExtentX = 1.f;
		wall.fHalfExtentY = 2.f;
		wall.fHalfExtentZ = 0.5f;
		WORLD_BOOTSTRAP_PLACEMENT receiver = wall;
		receiver.strPlacementId = "collision.contract.axe.wall.receiver";
		CServerCollisionSystem axeCollision;
		std::vector<std::string> axeContacts;
		std::string axeStatus;
		const bool axeInitialized = axeCollision.Initialize(
			{ wall, receiver }, axeStatus);
		axeCollision.Collect_BossPatternHitContacts(
			BOSS_PATTERN_HIT_SHAPE::CONE,
			0.f, 0.f, 0.f, 0.f, 1.5f,
			0.f, 0.f, 80.f, 7.f, 0.f, axeContacts);
		const bool coneHit = 1u == axeContacts.size() &&
			axeContacts.front() == wall.strPlacementId;
		axeCollision.Collect_BossPatternHitContacts(
			BOSS_PATTERN_HIT_SHAPE::CONE,
			0.f, 0.f, 0.f, 180.f, 1.5f,
			0.f, 0.f, 80.f, 7.f, 0.f, axeContacts);
		const bool rearMiss = axeContacts.empty();
		axeCollision.Collect_BossPatternHitContacts(
			BOSS_PATTERN_HIT_SHAPE::CONE,
			0.f, 20.f, 0.f, 0.f, 1.5f,
			0.f, 0.f, 80.f, 7.f, 0.f, axeContacts);
		const bool highMiss = axeContacts.empty();
		tests.Require(
			axeInitialized && coneHit && rearMiss && highMiss,
			"Intersect a rotated wall with the Server axe cone while rejecting receiver, rear and high-Y false contacts");
	}
	{
		WORLD_BOOTSTRAP_PLACEMENT armWall{};
		armWall.strPlacementId = "collision.contract.six-directions.arm";
		armWall.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX;
		armWall.isEnabled = true;
		armWall.fPositionX = 4.330127f;
		armWall.fPositionY = 2.f;
		armWall.fPositionZ = 2.5f;
		armWall.fHalfExtentX = 0.2f;
		armWall.fHalfExtentY = 0.5f;
		armWall.fHalfExtentZ = 0.2f;
		WORLD_BOOTSTRAP_PLACEMENT gapWall = armWall;
		gapWall.strPlacementId = "collision.contract.six-directions.gap";
		gapWall.fPositionX = 2.5f;
		gapWall.fPositionZ = 4.330127f;
		CServerCollisionSystem sixDirectionCollision;
		std::vector<std::string> sixDirectionContacts;
		std::string sixDirectionStatus;
		const bool sixDirectionInitialized = sixDirectionCollision.Initialize(
			{ armWall, gapWall }, sixDirectionStatus);
		sixDirectionCollision.Collect_BossPatternHitContacts(
			BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS,
			0.f, 0.f, 0.f, 0.f, 1.5f,
			0.f, 0.f, 0.f, 6.f, 0.5f, sixDirectionContacts);
		tests.Require(
			sixDirectionInitialized && 1u == sixDirectionContacts.size() &&
			sixDirectionContacts.front() == armWall.strPlacementId,
			"Project six-direction Server contacts onto an arm without filling its adjacent gap");
	}
	{
		WORLD_BOOTSTRAP_PLACEMENT wall{};
		wall.strPlacementId = "collision.contract.charge-wall";
		wall.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX;
		wall.isEnabled = true;
		wall.fPositionY = 1.f;
		wall.fPositionZ = 5.f;
		wall.fHalfExtentX = 2.f;
		wall.fHalfExtentY = 1.f;
		wall.fHalfExtentZ = 0.5f;
		WORLD_BOOTSTRAP_PLACEMENT receiver = wall;
		receiver.strPlacementId = wall.strPlacementId + ".receiver";
		CServerCollisionSystem chargeWallCollision;
		std::string chargeWallStatus;
		SERVER_BOSS_WALL_HIT wallHit{};
		const bool chargeWallsInitialized = chargeWallCollision.Initialize(
			{ receiver, wall }, chargeWallStatus);
		tests.Require(
			chargeWallsInitialized &&
			chargeWallCollision.Sweep_BossCircleAgainstWalls(
				0.f, 0.f, 0.f, 0.f, 0.f, 10.f, 1.f, wallHit) &&
			wall.strPlacementId == wallHit.strCollisionPlacementId &&
			receiver.strPlacementId ==
				wallHit.strImpactReceiverPlacementId &&
			wallHit.fHitRatio > 0.f && wallHit.fHitRatio < 1.f,
			"Keep the base wall and co-located active receiver on one deterministic boss sweep");
		SERVER_COLLISION_STATE_STAGE receiverDisabled{};
		const bool stagedReceiverDisabled =
			chargeWallCollision.Prepare_StateChanges(
				{ { receiver.strPlacementId, true, false } },
				receiverDisabled, chargeWallStatus);
		if (stagedReceiverDisabled)
			chargeWallCollision.Commit_StateChanges(std::move(receiverDisabled));
		tests.Require(
			stagedReceiverDisabled &&
			chargeWallCollision.Sweep_BossCircleAgainstWalls(
				0.f, 0.f, 0.f, 0.f, 0.f, 10.f, 1.f, wallHit) &&
			wall.strPlacementId == wallHit.strCollisionPlacementId &&
			wallHit.strImpactReceiverPlacementId.empty(),
			"Keep an ordinary blocking wall eligible after its impact receiver is disabled");
	}
	SERVER_BOSS_RECEIVER_HIT receiverHit{};
	/* Both sweeps start clear of the 109 outer ring. Every slab of it now
	carries a receiver, and a sweep begun inside one resolves at ratio zero,
	which would stop exercising the travel this contract is named for. */
	tests.Require(
		valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			148.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			175.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			1.f, receiverHit) &&
		0u == receiverHit.strReceiverPlacementId.rfind(
			"collision.valtan.wallgroup.11047903315509031966.", 0u) &&
		receiverHit.fHitRatio > 0.f && receiverHit.fHitRatio < 1.f &&
		!valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			148.f, 100.f, VALTAN_WALL_CENTER_Z,
			175.f, 100.f, VALTAN_WALL_CENTER_Z,
			1.f, receiverHit),
		"Sweep a fast Valtan body into the deterministic earliest independent receiver without tunneling or high-Y false hits");
	SERVER_PLAYER valtanWallPlayer{};
	valtanWallPlayer.fPositionX = VALTAN_WALL_APPROACH_X;
	valtanWallPlayer.fPositionY = VALTAN_WALL_CENTER_Y;
	valtanWallPlayer.fPositionZ = VALTAN_WALL_APPROACH_Z;
	float wallResolvedX = 0.f;
	float wallResolvedY = 0.f;
	float wallResolvedZ = 0.f;
	bool wallMoveBlocked = false;
	tests.Require(
		valtanCollisionSystem.Resolve_PlayerMove(
			valtanWallPlayer, VALTAN_WALL_EXIT_X, VALTAN_WALL_CENTER_Y,
			VALTAN_WALL_EXIT_Z, wallResolvedX, wallResolvedY,
			wallResolvedZ, wallMoveBlocked) && wallMoveBlocked,
		"Block player movement through the intact Valtan wall receiver");
	SERVER_COLLISION_STATE_STAGE collisionStage{};
	const std::uint64_t collisionRevisionBeforeReject =
		valtanCollisionSystem.Get_Revision();
	tests.Require(
		!valtanCollisionSystem.Prepare_StateChanges(
			{ { "receiver.valtan.wall.unknown", false, false } },
			collisionStage, dynamicWorldStatus) &&
		collisionRevisionBeforeReject ==
			valtanCollisionSystem.Get_Revision(),
		"Reject an unknown collision state target without mutating the receiver");
	tests.Require(
		valtanCollisionSystem.Prepare_StateChanges(
			{ { VALTAN_WALL_COLLISION_STATE, true, false } },
			collisionStage, dynamicWorldStatus),
		"Stage BREAKING collision channels with receiver impact disabled");
	valtanCollisionSystem.Commit_StateChanges(std::move(collisionStage));
	SERVER_BOSS_RECEIVER_HIT remainingReceiverHit{};
	tests.Require(
		(!valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			145.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			175.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			1.f, remainingReceiverHit) ||
		 remainingReceiverHit.strReceiverPlacementId != VALTAN_WALL_RECEIVER) &&
		!valtanCollisionSystem.Is_ImpactReceiverEnabled(VALTAN_WALL_RECEIVER) &&
		valtanCollisionSystem.Is_PlayerBlocking(VALTAN_WALL_COLLISION_STATE) &&
		valtanCollisionSystem.Resolve_PlayerMove(
			valtanWallPlayer, VALTAN_WALL_EXIT_X, VALTAN_WALL_CENTER_Y,
			VALTAN_WALL_EXIT_Z, wallResolvedX, wallResolvedY,
			wallResolvedZ, wallMoveBlocked) && wallMoveBlocked,
		"Keep the selected BREAKING wall blocking while suppressing only its receiver");
	tests.Require(
		valtanCollisionSystem.Prepare_StateChanges(
			{ { VALTAN_WALL_COLLISION_STATE, false, false } },
			collisionStage, dynamicWorldStatus),
		"Stage the persistent FRACTURED collision state");
	valtanCollisionSystem.Commit_StateChanges(std::move(collisionStage));
	tests.Require(
		!valtanCollisionSystem.Is_PlayerBlocking(VALTAN_WALL_COLLISION_STATE) &&
		!valtanCollisionSystem.Is_PlayerBlocking(VALTAN_WALL_RECEIVER) &&
		valtanCollisionSystem.Resolve_PlayerMove(
			valtanWallPlayer, VALTAN_WALL_EXIT_X, VALTAN_WALL_CENTER_Y,
			VALTAN_WALL_EXIT_Z, wallResolvedX, wallResolvedY,
			wallResolvedZ, wallMoveBlocked) && wallMoveBlocked,
		"Disable only the selected wall collision while adjacent independent walls remain solid");
	valtanCollisionSystem.Reset_RuntimeStates();
	tests.Require(
		valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			145.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			175.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			1.f, receiverHit),
		"Restore the Valtan impact receiver when the room resets");
	SERVER_WORLD_ENTITY impactMotionBoss{};
	impactMotionBoss.strEncounterId = "ENCOUNTER_VALTAN";
	impactMotionBoss.strPatternId = "VALTAN_ARMOR_BREAK_OPENING";
	impactMotionBoss.strPatternStageId = "WALL_CHARGE";
	impactMotionBoss.strActionId =
		"valtan.mechanic.armor-break-opening.charge";
	impactMotionBoss.eAction = SERVER_ENTITY_ACTION::PATTERN_WINDUP;
	/* The authored stage, not the pattern name, is what makes a stage charge.
	Enter_PatternStage copies this out of the catalog in production. */
	impactMotionBoss.bPatternChargeImpact = true;
	impactMotionBoss.fPositionX = 150.f;
	impactMotionBoss.fPositionZ = -133.f;
	impactMotionBoss.fYawDegrees = 90.f;
	impactMotionBoss.fPatternForcedMotionSpeed = 30.f;
	float impactProposedX = 0.f;
	float impactProposedZ = 0.f;
	CValtanBrain impactMotionBrain;
	tests.Require(
		impactMotionBrain.Try_BuildImpactMotion(
			impactMotionBoss, 1.f / 30.f,
			impactProposedX, impactProposedZ) &&
		std::abs(impactProposedX - 151.f) <= 0.001f &&
		std::abs(impactProposedZ + 133.f) <= 0.001f,
		"Advance the opening charge from Server-authored fixed-tick motion");
	tests.Require(
		impactMotionBrain.Complete_ImpactStage(
			impactMotionBoss, catalog, 500u) &&
		impactMotionBoss.strPatternStageId == "GROGGY" &&
		impactMotionBoss.bPatternGroggy &&
		!impactMotionBoss.bPatternChargeImpact &&
		0.f == impactMotionBoss.fPatternForcedMotionSpeed,
		"Advance the authoritative charge action to GROGGY only after impact");
	SERVER_WORLD_ENTITY plainStageBoss = impactMotionBoss;
	plainStageBoss.strPatternStageId = "WALL_CHARGE";
	plainStageBoss.iPatternStageIndex = 0u;
	plainStageBoss.bPatternChargeImpact = false;
	plainStageBoss.fPatternForcedMotionSpeed = 30.f;
	float plainProposedX = 0.f;
	float plainProposedZ = 0.f;
	tests.Require(
		!impactMotionBrain.Try_BuildImpactMotion(
			plainStageBoss, 1.f / 30.f, plainProposedX, plainProposedZ) &&
		!impactMotionBrain.Complete_ImpactStage(
			plainStageBoss, catalog, 500u) &&
		plainStageBoss.strPatternStageId == "WALL_CHARGE",
		"Leave a stage the encounter never authored as a charge without impact motion");

	CWorldBootstrap bernWorld;
	const bool bernLoaded = bernWorld.Load(WORLD_ID::BERN);
	const auto& bernPlacements = bernWorld.Get_Placements();
	const auto bernNpc = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "npc.bern.beda.guide";
		});
	const auto bernAylara = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "npc.bern.aylara";
		});
	const auto bernSchmidt = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "npc.bern.schmidt";
		});
	const auto bernPlayerEntrySpawn = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "player_1";
		});
	const auto bernLeftGuardPatrol = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "npc.bern.25287";
		});
	const auto bernRightGuardPatrol = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "npc.bern.25287.2";
		});
	std::set<std::string> bernPlazaArchetypeIds;
	std::set<std::string> bernPlazaActionIds;
	std::array<std::size_t, 4u> bernPlazaClusterCounts{};
	std::size_t bernPlazaNpcCount = 0u;
	std::size_t bernPlazaLookTargetCount = 0u;
	bool bernPlazaNpcContractValid = true;
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement : bernPlacements)
	{
		if (0u != placement.strPlacementId.rfind("npc.bern.plaza.", 0u))
			continue;
		++bernPlazaNpcCount;
		bool belongsToAuthoredCluster = false;
		if (placement.fPositionX >= 130.5f &&
			placement.fPositionX <= 134.5f &&
			placement.fPositionZ >= -86.f &&
			placement.fPositionZ <= -81.5f)
		{
			++bernPlazaClusterCounts[0u];
			belongsToAuthoredCluster = true;
		}
		else if (placement.fPositionX >= 138.f &&
			placement.fPositionX <= 141.f &&
			placement.fPositionZ >= -86.5f &&
			placement.fPositionZ <= -83.4f)
		{
			++bernPlazaClusterCounts[1u];
			belongsToAuthoredCluster = true;
		}
		else if (placement.fPositionX >= 142.f &&
			placement.fPositionX <= 144.5f &&
			placement.fPositionZ >= -82.5f &&
			placement.fPositionZ <= -76.5f)
		{
			++bernPlazaClusterCounts[2u];
			belongsToAuthoredCluster = true;
		}
		else if (placement.fPositionX >= 131.5f &&
			placement.fPositionX <= 134.5f &&
			placement.fPositionZ >= -75.5f &&
			placement.fPositionZ <= -73.4f)
		{
			++bernPlazaClusterCounts[3u];
			belongsToAuthoredCluster = true;
		}
		bernPlazaNpcContractValid = bernPlazaNpcContractValid &&
			placement.isEnabled &&
			WORLD_BOOTSTRAP_KIND::NPC == placement.eKind &&
			belongsToAuthoredCluster &&
			placement.bHasNpcBehavior &&
			NPC_BEHAVIOR_MODE::STATIONARY == placement.NpcBehavior.eMode &&
			placement.NpcBehavior.Waypoints.empty() &&
			0.f == placement.NpcBehavior.fWanderRadius &&
			1u == placement.NpcBehavior.Actions.size() &&
			600000u == placement.NpcBehavior.Actions.front().iDurationMs &&
			0u == placement.NpcBehavior.Actions.front().iWaitAfterMs;
		bernPlazaArchetypeIds.insert(placement.strArchetypeId);
		if (!placement.NpcBehavior.Actions.empty())
		{
			bernPlazaActionIds.insert(
				placement.NpcBehavior.Actions.front().strActionId);
		}
		if (!placement.NpcBehavior.strLookTargetPlacementId.empty())
			++bernPlazaLookTargetCount;
	}
	bool bernGuardPatrolsHeadToPlayerEntry = false;
	if (bernLeftGuardPatrol != bernPlacements.end() &&
		bernRightGuardPatrol != bernPlacements.end() &&
		bernPlayerEntrySpawn != bernPlacements.end() &&
		bernLeftGuardPatrol->bHasNpcBehavior &&
		bernRightGuardPatrol->bHasNpcBehavior &&
		2u == bernLeftGuardPatrol->NpcBehavior.Waypoints.size() &&
		2u == bernRightGuardPatrol->NpcBehavior.Waypoints.size())
	{
		const auto& leftWaypoints =
			bernLeftGuardPatrol->NpcBehavior.Waypoints;
		const auto& rightWaypoints =
			bernRightGuardPatrol->NpcBehavior.Waypoints;
		const float leftLaneX = leftWaypoints[0u].fPositionX;
		const float rightLaneX = rightWaypoints[0u].fPositionX;
		const float corridorMinZ = (std::min)(
			leftWaypoints[0u].fPositionZ,
			leftWaypoints[1u].fPositionZ);
		const float corridorMaxZ = (std::max)(
			leftWaypoints[0u].fPositionZ,
			leftWaypoints[1u].fPositionZ);
		constexpr float requiredNpcLaneClearance =
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X * 2.f + 0.05f;
		const bool routesAreParallelAndSeparated =
			std::abs(leftWaypoints[0u].fPositionX -
				leftWaypoints[1u].fPositionX) < 0.001f &&
			std::abs(rightWaypoints[0u].fPositionX -
				rightWaypoints[1u].fPositionX) < 0.001f &&
			std::abs(leftWaypoints[0u].fPositionZ -
				rightWaypoints[0u].fPositionZ) < 0.001f &&
			std::abs(leftWaypoints[1u].fPositionZ -
				rightWaypoints[1u].fPositionZ) < 0.001f &&
			std::abs(rightLaneX - leftLaneX) >= requiredNpcLaneClearance &&
			std::abs(leftWaypoints[1u].fPositionZ -
				leftWaypoints[0u].fPositionZ) >= 30.f;
		const auto distanceSquaredFromPlayerEntry =
			[&bernPlayerEntrySpawn](
				const WORLD_NPC_BEHAVIOR_WAYPOINT& waypoint)
			{
				const float dx = waypoint.fPositionX -
					bernPlayerEntrySpawn->fPositionX;
				const float dz = waypoint.fPositionZ -
					bernPlayerEntrySpawn->fPositionZ;
				return dx * dx + dz * dz;
			};
		const bool routesHeadTowardPlayerEntry =
			leftWaypoints[1u].fPositionZ > leftWaypoints[0u].fPositionZ &&
			rightWaypoints[1u].fPositionZ > rightWaypoints[0u].fPositionZ &&
			distanceSquaredFromPlayerEntry(leftWaypoints[1u]) <
				distanceSquaredFromPlayerEntry(leftWaypoints[0u]) &&
			distanceSquaredFromPlayerEntry(rightWaypoints[1u]) <
				distanceSquaredFromPlayerEntry(rightWaypoints[0u]);
		bernGuardPatrolsHeadToPlayerEntry =
			routesAreParallelAndSeparated && routesHeadTowardPlayerEntry &&
			std::all_of(
				bernPlacements.begin(), bernPlacements.end(),
				[leftLaneX, rightLaneX, corridorMinZ, corridorMaxZ,
				 requiredNpcLaneClearance](
					const WORLD_BOOTSTRAP_PLACEMENT& placement)
				{
					if (WORLD_BOOTSTRAP_KIND::NPC != placement.eKind ||
						!placement.isEnabled ||
						placement.strPlacementId == "npc.bern.25287" ||
						placement.strPlacementId == "npc.bern.25287.2" ||
						placement.fPositionZ < corridorMinZ ||
						placement.fPositionZ > corridorMaxZ)
					{
						return true;
					}
					return (std::min)(
						std::abs(placement.fPositionX - rightLaneX),
						std::abs(placement.fPositionX - leftLaneX)) >=
						requiredNpcLaneClearance;
				});
	}
	const auto bernValtanTrigger = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "valtan";
		});
	const auto bernCollision = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId ==
				"collision.bern.editor-proof";
		});
	tests.Require(
		bernLoaded && bernPlacements.size() == 36u &&
		4u == static_cast<size_t>(std::count_if(
			bernPlacements.begin(), bernPlacements.end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.isEnabled;
			})) &&
		30u == static_cast<size_t>(std::count_if(
			bernPlacements.begin(), bernPlacements.end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::NPC == placement.eKind &&
					placement.isEnabled;
			})) &&
		bernNpc != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::NPC == bernNpc->eKind &&
		bernNpc->strArchetypeId == "NPC_BEDA" &&
		bernAylara != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::NPC == bernAylara->eKind &&
		bernAylara->strArchetypeId == "NPC_AYLARA" &&
		bernAylara->isEnabled &&
		bernSchmidt != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::NPC == bernSchmidt->eKind &&
		bernSchmidt->strArchetypeId == "NPC_SCHMIDT" &&
		bernSchmidt->isEnabled &&
		bernPlayerEntrySpawn != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == bernPlayerEntrySpawn->eKind &&
		bernLeftGuardPatrol != bernPlacements.end() &&
		bernLeftGuardPatrol->bHasNpcBehavior &&
		NPC_BEHAVIOR_MODE::PATROL == bernLeftGuardPatrol->NpcBehavior.eMode &&
		NPC_ROUTE_MODE::PING_PONG ==
			bernLeftGuardPatrol->NpcBehavior.eRouteMode &&
		2u == bernLeftGuardPatrol->NpcBehavior.Waypoints.size() &&
		std::abs(bernLeftGuardPatrol->NpcBehavior.fMoveSpeed - 1.5f) < 0.001f &&
		bernRightGuardPatrol != bernPlacements.end() &&
		bernRightGuardPatrol->bHasNpcBehavior &&
		NPC_BEHAVIOR_MODE::PATROL == bernRightGuardPatrol->NpcBehavior.eMode &&
		NPC_ROUTE_MODE::PING_PONG ==
			bernRightGuardPatrol->NpcBehavior.eRouteMode &&
		2u == bernRightGuardPatrol->NpcBehavior.Waypoints.size() &&
		std::abs(bernRightGuardPatrol->NpcBehavior.fMoveSpeed - 1.5f) < 0.001f &&
		bernGuardPatrolsHeadToPlayerEntry &&
		20u == bernPlazaNpcCount && bernPlazaNpcContractValid &&
		8u == bernPlazaClusterCounts[0u] &&
		3u == bernPlazaClusterCounts[1u] &&
		5u == bernPlazaClusterCounts[2u] &&
		4u == bernPlazaClusterCounts[3u] &&
		20u == bernPlazaArchetypeIds.size() &&
		20u == bernPlazaActionIds.size() &&
		19u == bernPlazaLookTargetCount &&
		bernValtanTrigger != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX == bernValtanTrigger->eKind &&
		!bernValtanTrigger->isEnabled &&
		1u == bernValtanTrigger->TriggerActions.size() &&
		bernCollision != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::COLLISION_BOX == bernCollision->eKind,
		"Load Bern spawns, Schmidt, four irregular stationary plaza crowds, two guard ping-pong patrols toward the player entry, disabled legacy trigger, and collision box");
	CServerCollisionSystem bernCollisionSystem;
	std::string bernCollisionStatus;
	tests.Require(
		bernCollisionSystem.Initialize(bernPlacements, bernCollisionStatus) &&
		1u == bernCollisionSystem.Get_CollisionBoxCount() &&
		std::all_of(
			bernPlacements.begin(), bernPlacements.end(),
			[&bernCollisionSystem](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != placement.eKind ||
					bernCollisionSystem.Is_PlayerSpawnClear(placement);
			}),
		"Stage Bern collision box without overlapping player spawns");
	SERVER_PLAYER collisionPlayer{};
	collisionPlayer.fPositionX = 138.f;
	collisionPlayer.fPositionY = 42.7f;
	collisionPlayer.fPositionZ = -65.3f;
	float resolvedX = 0.f;
	float resolvedY = 0.f;
	float resolvedZ = 0.f;
	bool wasBlocked = false;
	tests.Require(
		bernCollisionSystem.Resolve_PlayerMove(
			collisionPlayer,
			143.f,
			42.7f,
			-65.3f,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked) &&
		wasBlocked && resolvedX < 139.851f && resolvedX > 138.f,
		"Stop a fast player sweep before the Bern collision box");
	collisionPlayer.fPositionZ = -60.f;
	tests.Require(
		bernCollisionSystem.Resolve_PlayerMove(
			collisionPlayer,
			143.f,
			42.7f,
			-60.f,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked) &&
		!wasBlocked && std::abs(resolvedX - 143.f) < 0.001f,
		"Preserve movement that passes outside the collision box");

	{
		/* Living monster and boss bodies block the player the same way, on the
		XZ plane, with the player's own half extent added to the body radius. */
		CServerCollisionSystem bodyCollision;
		std::string bodyStatus;
		tests.Require(bodyCollision.Initialize({}, bodyStatus),
			"Initialize an empty collision system for body blocking");
		bodyCollision.Set_BlockingBodies({ SERVER_BLOCKING_BODY{ 0.f, 3.f, 0.55f } });
		SERVER_PLAYER walker{};
		walker.fPositionX = 0.f;
		walker.fPositionY = 0.f;
		walker.fPositionZ = 0.f;
		float bodyX = 0.f;
		float bodyY = 0.f;
		float bodyZ = 0.f;
		bool bodyBlocked = false;
		/* Dead-on: reach the combined radius (z = 2 minus the contact margin),
		then the remaining 4 m of the step deflects to a fixed side at full
		length instead of parking, so the walk keeps its goal. */
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				walker, 0.f, 0.f, 6.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked && bodyZ > 1.9f && bodyZ < 2.f &&
			std::abs(bodyX - 4.f) < 0.01f,
			"Deflect a dead-on walk into a monster body around its side at full speed");
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				walker, 6.f, 0.f, 0.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked && std::abs(bodyX - 6.f) < 0.001f,
			"Preserve a player move that passes beside a monster body");
		SERVER_PLAYER overlapped = walker;
		overlapped.fPositionZ = 2.7f;
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				overlapped, 0.f, 0.f, 0.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked && std::abs(bodyZ) < 0.001f,
			"Let a player already inside a body step away from its centre");
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				overlapped, 0.f, 0.f, 3.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked && std::abs(bodyZ - 2.7f) < 0.001f &&
			std::abs(bodyX - 0.3f) < 0.001f,
			"Turn a step toward the centre of a body the player is inside into a sideways step");
		/* Off-centre approach: contact at z = 3 - sqrt(1 - 0.25), then the rest
		of the step slides along the tangent, away from the body and past its
		side, and the walk is not reported as blocked. */
		SERVER_PLAYER slider = walker;
		slider.fPositionX = -0.5f;
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				slider, -0.5f, 0.f, 6.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked && bodyX < -1.5f && bodyZ > 2.5f &&
			std::sqrt(bodyX * bodyX + (bodyZ - 3.f) * (bodyZ - 3.f)) > 0.999f,
			"Slide a player along a monster body instead of parking against it");
		SERVER_PLAYER insideSlider = walker;
		insideSlider.fPositionX = -0.3f;
		insideSlider.fPositionZ = 2.7f;
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				insideSlider, 0.3f, 0.f, 3.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked &&
			std::abs(bodyX - 0.174f) < 0.01f && std::abs(bodyZ - 2.226f) < 0.01f,
			"Slide a player already inside a body along the tangent at full step length");
		bodyCollision.Set_BlockingBodies({});
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				walker, 0.f, 0.f, 6.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked && std::abs(bodyZ - 6.f) < 0.001f,
			"Clear body blocking when the tick has no living bodies");
	}

	CServerNavigation bernNavigation;
	tests.Require(
		bernNavigation.Load("LV_BER_BERNCASTLE"),
		"Load Bern server navigation");
	const auto bernFirstSpawn = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
				placement.isEnabled;
		});
	const auto hasReachableGuideApproach =
		[&bernNavigation, &bernFirstSpawn, &bernPlacements](
			const WORLD_BOOTSTRAP_PLACEMENT& guide)
	{
		if (bernFirstSpawn == bernPlacements.end())
			return false;
		constexpr float INTERACTION_RADIUS = 3.f;
		constexpr float CANDIDATE_STEP = 0.5f;
		constexpr int CANDIDATE_RADIUS_STEPS = 6;
		for (int zStep = -CANDIDATE_RADIUS_STEPS;
			zStep <= CANDIDATE_RADIUS_STEPS; ++zStep)
		{
			for (int xStep = -CANDIDATE_RADIUS_STEPS;
				xStep <= CANDIDATE_RADIUS_STEPS; ++xStep)
			{
				const float offsetX = static_cast<float>(xStep) * CANDIDATE_STEP;
				const float offsetZ = static_cast<float>(zStep) * CANDIDATE_STEP;
				if (offsetX * offsetX + offsetZ * offsetZ >
					INTERACTION_RADIUS * INTERACTION_RADIUS)
				{
					continue;
				}
				std::vector<SERVER_NAV_POINT> path;
				if (!bernNavigation.Find_Path(
					bernFirstSpawn->fPositionX,
					bernFirstSpawn->fPositionZ,
					guide.fPositionX + offsetX,
					guide.fPositionZ + offsetZ,
					path) || path.empty())
				{
					continue;
				}
				const SERVER_NAV_POINT& endpoint = path.back();
				if (std::hypot(
					endpoint.x - guide.fPositionX,
					endpoint.z - guide.fPositionZ) <= INTERACTION_RADIUS)
				{
					return true;
				}
			}
		}
		return false;
	};
	const bool bernGuideReachable =
		(bernNpc != bernPlacements.end() &&
			hasReachableGuideApproach(*bernNpc)) &&
		(bernAylara != bernPlacements.end() &&
			hasReachableGuideApproach(*bernAylara));
	tests.Require(
		bernGuideReachable,
		"Reach every Bern Valtan-entry guide NPC through authoritative navigation");
	bool bernSpawnsOnNavigation = bernLoaded;
	for (const WORLD_BOOTSTRAP_PLACEMENT& spawn : bernPlacements)
	{
		if (WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != spawn.eKind)
			continue;
		SERVER_NAV_POINT projected{};
		bernSpawnsOnNavigation =
			bernSpawnsOnNavigation &&
			bernNavigation.Project_Point(
				spawn.fPositionX,
				spawn.fPositionZ,
				projected) &&
			std::abs(projected.y - spawn.fPositionY) <= 0.25f;
	}
	tests.Require(
		bernSpawnsOnNavigation,
		"Project all Bern player spawns to baked navigation");
	/* The visible north-entry stair continues beyond the former z=-9.438004
	navigation boundary. Its authored final stair placement is centered at
	(136.86001, -3.30000488), so this exact world position must remain connected
	to the Bern spawn instead of being clipped by the grid extent. */
	constexpr float BERN_NORTH_ENTRY_X = 136.86001f;
	constexpr float BERN_NORTH_ENTRY_Z = -3.30000488f;
	SERVER_NAV_POINT bernNorthEntryGround{};
	std::vector<SERVER_NAV_POINT> bernNorthEntryPath;
	const bool bernNorthEntryReachable =
		bernNavigation.Sample_Position(
			BERN_NORTH_ENTRY_X,
			BERN_NORTH_ENTRY_Z,
			bernNorthEntryGround) &&
		bernNavigation.Find_Path(
			137.586334f,
			-22.4640217f,
			BERN_NORTH_ENTRY_X,
			BERN_NORTH_ENTRY_Z,
			bernNorthEntryPath) &&
		!bernNorthEntryPath.empty();
	tests.Require(
		bernNorthEntryReachable &&
		std::abs(bernNorthEntryGround.y - 42.2766418f) <= 0.25f &&
		std::abs(bernNorthEntryPath.back().x - 136.738007f) <= 0.26f &&
		std::abs(bernNorthEntryPath.back().z - (-3.1880035f)) <= 0.26f,
		"Reach the visible Bern north-entry stair through authoritative navigation");
	/* The castle approach is why Bern needs a grid at all: without one the room
	keeps the spawn height for the whole session and straight-line movement walks
	through the staircase. The authoritative path from the spawn to the top of the
	baked stair run must therefore carry a real climb, and the stair run itself
	must stay a walkable slope instead of one vertical jump. */
	std::vector<SERVER_NAV_POINT> bernStairPath;
	const bool bernStairPathFound = bernNavigation.Find_Path(
		137.586334f,
		-22.4640217f,
		137.238007f,
		-116.688004f,
		bernStairPath);
	float bernStairClimb = 0.f;
	float bernStairRunStep = 0.f;
	if (bernStairPathFound && !bernStairPath.empty())
	{
		bernStairClimb = bernStairPath.back().y - bernStairPath.front().y;
		for (size_t index = 1u; index < bernStairPath.size(); ++index)
		{
			if (bernStairPath[index].y <= 47.f)
				continue;
			bernStairRunStep = (std::max)(
				bernStairRunStep,
				std::abs(bernStairPath[index].y -
					bernStairPath[index - 1u].y));
		}
	}
	tests.Require(
		bernStairPathFound && bernStairPath.size() > 100u &&
		bernStairPath.back().y > 49.f && bernStairClimb > 6.f,
		"Climb the Bern castle stairs along the authoritative path");

	CWorldBootstrap trainingWorld;
	CServerNavigation trainingNavigation;
	tests.Require(trainingWorld.Load(WORLD_ID::TRAINING_GROUND) &&
		trainingWorld.Get_AreaId() == "LV_DEV_TRAINING_GROUND" &&
		std::all_of(
			trainingWorld.Get_Placements().begin(),
			trainingWorld.Get_Placements().end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.strArchetypeId.empty();
			}),
		"Load class-neutral training player spawns");
	tests.Require(trainingNavigation.Load("LV_DEV_TRAINING_GROUND"),
		"Load training server navigation");
	SERVER_NAV_POINT trainingPoint{};
	tests.Require(trainingNavigation.Project_Point(0.f, -4.f, trainingPoint),
		"Project training spawn to walkable cell");
	tests.Require(!trainingNavigation.Project_Point(16.01f, 0.f, trainingPoint),
		"Reject training point beyond arena navigation bounds");

	CWorldBootstrap characterSelectWorld;
	CServerNavigation characterSelectNavigation;
	const bool characterSelectWorldLoaded =
		characterSelectWorld.Load(WORLD_ID::CHARACTER_SELECT_ARENA);
	const auto& characterSelectSpawns =
		characterSelectWorld.Get_Placements();
	const auto lazyValtan = std::find_if(
		characterSelectSpawns.begin(),
		characterSelectSpawns.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId ==
				"boss.valtan.character-select.lazy";
		});
	tests.Require(
		characterSelectWorldLoaded &&
		characterSelectWorld.Get_AreaId() ==
			"LV_LOBBY_CLASSSELECT_SL00" &&
		characterSelectSpawns.size() == 5 &&
		4u == static_cast<size_t>(std::count_if(
			characterSelectSpawns.begin(),
			characterSelectSpawns.end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.strArchetypeId.empty() &&
					placement.isEnabled;
			})),
		"Load class-neutral Character Select arena player spawns");
	tests.Require(
		characterSelectSpawns.end() != lazyValtan &&
		!lazyValtan->isEnabled &&
		lazyValtan->eKind == WORLD_BOOTSTRAP_KIND::BOSS &&
		lazyValtan->strArchetypeId == "BOSS_VALTAN" &&
		lazyValtan->strEncounterId == "ENCOUNTER_VALTAN",
		"Load disabled Character Select Valtan lazy template");
	tests.Require(
		characterSelectNavigation.Load("LV_LOBBY_CLASSSELECT_SL00"),
		"Load Character Select arena server navigation");
	bool characterSelectSpawnsOnNavigation =
		characterSelectWorldLoaded && characterSelectSpawns.size() == 5;
	SERVER_NAV_POINT characterSelectPoint{};
	for (const WORLD_BOOTSTRAP_PLACEMENT& spawn : characterSelectSpawns)
	{
		if (WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != spawn.eKind)
			continue;
		SERVER_NAV_POINT projected{};
		characterSelectSpawnsOnNavigation =
			characterSelectSpawnsOnNavigation &&
			characterSelectNavigation.Project_Point(
				spawn.fPositionX,
				spawn.fPositionZ,
				projected) &&
			std::abs(projected.y - spawn.fPositionY) <= 0.25f;
	}
	tests.Require(
		characterSelectSpawnsOnNavigation,
		"Project all Character Select spawns to baked navigation");
	SERVER_NAV_POINT lazyValtanPoint{};
	tests.Require(
		characterSelectSpawns.end() != lazyValtan &&
		characterSelectNavigation.Project_Point(
			lazyValtan->fPositionX,
			lazyValtan->fPositionZ,
			lazyValtanPoint) &&
		std::abs(lazyValtanPoint.y - lazyValtan->fPositionY) <= 0.25f,
		"Project disabled Character Select Valtan template to navigation");
	if (!characterSelectSpawns.empty())
	{
		characterSelectNavigation.Project_Point(
			characterSelectSpawns.front().fPositionX,
			characterSelectSpawns.front().fPositionZ,
			characterSelectPoint);
	}
	std::vector<SERVER_NAV_POINT> characterSelectPath;
	tests.Require(
		characterSelectSpawns.size() >= 2 &&
		characterSelectNavigation.Find_Path(
			characterSelectSpawns.front().fPositionX,
			characterSelectSpawns.front().fPositionZ,
			characterSelectSpawns[1].fPositionX,
			characterSelectSpawns[1].fPositionZ,
			characterSelectPath) &&
		characterSelectPath.size() >= 2 &&
		std::adjacent_find(
			characterSelectPath.begin(),
			characterSelectPath.end(),
			[](const SERVER_NAV_POINT& left, const SERVER_NAV_POINT& right)
			{
				return std::abs(left.y - right.y) > 0.6f;
			}) == characterSelectPath.end(),
		"Find Character Select arena navigation path");
	SERVER_NAV_POINT characterSelectOutside{};
	tests.Require(
		!characterSelectNavigation.Project_Point(
			-787.6f,
			197.5f,
			characterSelectOutside),
		"Reject point beyond Character Select arena navigation bounds");

	SERVER_PLAYER arenaSkillPlayer{};
	arenaSkillPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	arenaSkillPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
	arenaSkillPlayer.iCurrentHp = 1000;
	arenaSkillPlayer.iMaximumHp = 1000;
	arenaSkillPlayer.iCurrentResource = 1000;
	arenaSkillPlayer.iMaximumResource = 1000;
	arenaSkillPlayer.fPositionX = characterSelectPoint.x;
	arenaSkillPlayer.fPositionY = characterSelectPoint.y;
	arenaSkillPlayer.fPositionZ = characterSelectPoint.z;
	C2S_USE_SKILL arenaSkillCommand{};
	arenaSkillCommand.iClientSequence = 1;
	arenaSkillCommand.iSkillId = 34120;
	arenaSkillCommand.fAimX = characterSelectPoint.x + 3.f;
	arenaSkillCommand.fAimZ = characterSelectPoint.z;
	CPlayerSkillSystem arenaSkillSystem;
	std::vector<SERVER_WORLD_ENTITY> arenaEntities;
	tests.Require(
		arenaSkillSystem.Try_Start(
			arenaSkillPlayer,
			arenaSkillCommand,
			catalog,
			10) &&
		PLAYER_ACTION_STATE::SKILL == arenaSkillPlayer.eAction &&
		34120u == arenaSkillPlayer.iCurrentSkillId &&
		10u == arenaSkillPlayer.iActionStartTick,
		"Start Character Select arena skill action");
	std::vector<DAMAGE_EVENT> arenaDamageEvents;
	arenaSkillSystem.Update(
		arenaSkillPlayer,
		arenaEntities,
		catalog,
		&characterSelectNavigation,
		nullptr,
		1.f / 30.f,
		11,
		arenaDamageEvents);
	SERVER_NAV_POINT arenaSkillPoint{};
	tests.Require(
		characterSelectNavigation.Project_Point(
			arenaSkillPlayer.fPositionX,
			arenaSkillPlayer.fPositionZ,
			arenaSkillPoint),
		"Keep Character Select skill action position on baked navigation");

	SERVER_PLAYER player{};
	player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	player.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
	player.iCurrentResource = 1000;
	player.iMaximumResource = 1000;
	player.fPositionX = 151.f;
	player.fPositionY = 22.97f;
	player.fPositionZ = -129.f;
	SERVER_WORLD_ENTITY boss{};
	boss.iNetEntityId = 900u;
	boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	boss.eAction = SERVER_ENTITY_ACTION::IDLE;
	boss.strArchetypeId = "BOSS_VALTAN";
	boss.iCurrentHp = 10000;
	boss.iMaximumHp = 10000;
	boss.fPositionX = 151.f;
	boss.fPositionY = 22.97f;
	const BOSS_RUNTIME_PROFILE* skillTargetProfile = catalog.Find_Boss("BOSS_VALTAN");
	boss.fCollisionRadius = nullptr == skillTargetProfile ? 0.f : skillTargetProfile->fCollisionRadius;
	// Keep a 1.6m gap to the authoritative body, inside all three authored hits.
	// The old four-metre centre distance assumed the retired three-metre radius.
	const float skillTargetStandOff = boss.fCollisionRadius + 1.6f;
	boss.fPositionZ = player.fPositionZ + skillTargetStandOff;
	SERVER_WORLD_ENTITY outsideBody = boss;
	outsideBody.iNetEntityId = 901u;
	// Root motion advances the caster up to 0.60m at the three hit ticks.
	// A 3m body gap misses every shape, while the larger stale mirror still overlaps.
	outsideBody.fPositionZ = player.fPositionZ + boss.fCollisionRadius + 3.f;
	outsideBody.fCollisionRadius += 1.6f; // A stale mirror must not enlarge a boss hit target.
	std::vector<SERVER_WORLD_ENTITY> entities{ boss, outsideBody };
	C2S_USE_SKILL useSkill{};
	useSkill.iClientSequence = 1;
	useSkill.iSkillId = 34120;
	useSkill.fAimX = boss.fPositionX;
	useSkill.fAimZ = boss.fPositionZ;
	CPlayerSkillSystem skills;
	tests.Require(skills.Try_Start(player, useSkill, catalog, 10),
		"Approve valid skill command");
	tests.Require(!skills.Try_Start(player, useSkill, catalog, 10),
		"Reject duplicate skill command while action is active");
	std::vector<DAMAGE_EVENT> damageEvents;
	for (std::uint32_t tick = 11; tick < 70; ++tick)
		skills.Update(player, entities, catalog, &navigation, nullptr,
			1.f / 30.f, tick, damageEvents);
	/* 34120 is official rate 361 at project-tuned attack power 1000, split across its three
	authored hit shapes so the sum stays exact. */
	const PLAYER_SKILL_DEFINITION* talonStrike = catalog.Find_Skill(34120);
	tests.Require(
		nullptr != talonStrike && 3u == talonStrike->Hits.size() &&
		talonStrike->Hits[0].iTimeMs < talonStrike->Hits[1].iTimeMs &&
		3u == talonStrike->Hits[0].iAreaType &&
		2u == talonStrike->Hits[2].iAreaType,
		"Load authored hit shapes for the skill from the gameplay bootstrap");
	tests.Require(6390u == entities[0].iCurrentHp,
		"Apply server-authoritative player damage across authored hits");
	std::uint32_t outgoingTotal = 0;
	for (const DAMAGE_EVENT& damageEvent : damageEvents)
		outgoingTotal += damageEvent.iAmount;
	tests.Require(
		3u == damageEvents.size() &&
		3610u == outgoingTotal &&
		damageEvents[0].isOutgoing &&
		entities[0].iNetEntityId == damageEvents[0].iTargetNetEntityId,
		"Emit one outgoing damage event per authored hit summing to the profile rate");
	tests.Require(
		10000u == entities[1].iCurrentHp,
		"Use the current boss profile body radius even when the entity mirror is oversized");
	entities.resize(1u);
	Run_PlayerActions(tests, catalog, navigation, navCellSize, boundaryProbeZ, lastWalkableX, firstBlockedX, boss, talonStrike, useSkill);

	C2S_USE_SKILL cooldownAttempt = useSkill;
	cooldownAttempt.iClientSequence = 2;
	tests.Require(!skills.Try_Start(player, cooldownAttempt, catalog, 70),
		"Reject skill during authoritative cooldown");

	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(34010);
		tests.Require(
			nullptr != combo &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			4u == combo->ComboStages.size() &&
			0u == combo->ComboStages[3].iInputCloseMs,
			"Resolve LanceMaster basic attack combo stages");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		comboPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 100;
		comboPlayer.iMaximumResource = 100;
		std::vector<SERVER_WORLD_ENTITY> comboEntities;
		std::vector<DAMAGE_EVENT> comboDamageEvents;
		CPlayerSkillSystem comboSkills;

		C2S_USE_SKILL press{};
		press.iClientSequence = 1;
		press.iSkillId = 34010;
		press.fAimX = 1.f;
		press.fAimZ = 0.f;
		tests.Require(
			comboSkills.Try_Start(comboPlayer, press, catalog, 10) &&
			1u == comboPlayer.iComboStage,
			"Approve basic attack first stage");

		// 329ms is where stage one opens; 100ms is deliberately before it.
		comboPlayer.fActionElapsedSeconds = 0.1f;
		press.iClientSequence = 2;
		comboSkills.Try_Start(comboPlayer, press, catalog, 12);
		tests.Require(!comboPlayer.hasBufferedComboInput,
			"Reject combo input before the window opens");

		comboPlayer.fActionElapsedSeconds = 0.4f;
		press.iClientSequence = 3;
		comboSkills.Try_Start(comboPlayer, press, catalog, 14);
		tests.Require(comboPlayer.hasBufferedComboInput,
			"Buffer combo input inside the window");

		press.iClientSequence = 4;
		comboSkills.Try_Start(comboPlayer, press, catalog, 15);
		tests.Require(1u == comboPlayer.iComboStage,
			"Ignore a second press inside the same window");

		C2S_USE_SKILL other{};
		other.iClientSequence = 5;
		other.iSkillId = 34120;
		other.fAimX = 1.f;
		other.fAimZ = 0.f;
		tests.Require(
			!comboSkills.Try_Start(comboPlayer, other, catalog, 16) &&
			34010u == comboPlayer.iCurrentSkillId,
			"Reject a different skill during a combo");

		/* This legacy stage authors comboAdvanceMs at its 470 ms hit, preserving
		the established cadence while the new field lets other stages keep their
		presentation longer. Twenty ticks is about 667 ms, past that boundary. */
		for (std::uint32_t tick = 17; tick < 37; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr,
				nullptr, 1.f / 30.f, tick, comboDamageEvents);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Advance at the authored legacy combo boundary");

		/* Nothing is buffered now, so stage two has to run its whole 1367 ms
		instead of cutting at its hit. */
		for (std::uint32_t tick = 37; tick < 57; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr,
				nullptr, 1.f / 30.f, tick, comboDamageEvents);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Hold the stage past its hit when no press was buffered");

		for (std::uint32_t tick = 57; tick < 120; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr,
				nullptr, 1.f / 30.f, tick, comboDamageEvents);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == comboPlayer.eAction &&
			0u == comboPlayer.iComboStage,
			"End the combo when no press was buffered");
	}

	std::map<PLAYER_ID, SERVER_PLAYER> players;
	SERVER_PLAYER target{};
	target.iPlayerId = 1;
	target.iNetEntityId = 100;
	target.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	target.iCurrentHp = 1000;
	target.iMaximumHp = 1000;
	target.fPositionX = 151.f;
	target.fPositionY = 22.97f;
	target.fPositionZ = -128.f;
	target.isCombatReady = false;
	players.emplace(target.iPlayerId, target);
	SERVER_WORLD_ENTITY valtan{};
	valtan.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	valtan.eAction = SERVER_ENTITY_ACTION::IDLE;
	/* The brain resolves damage through the boss's own catalog profile, so the
	test entity carries the archetype the room would have stamped on it. */
	valtan.strArchetypeId = "BOSS_VALTAN";
	valtan.strEncounterId = "ENCOUNTER_VALTAN";
	valtan.iCurrentHp = 48750;
	valtan.iMaximumHp = 60000;
	valtan.iMaximumHealthBars = 160;
	valtan.iLastEvaluatedHealthBar = 131;
	valtan.iPhaseTwoHpPercent = 50;
	valtan.iPhase = 1;
	valtan.fPositionX = 151.f;
	valtan.fPositionY = 22.97f;
	valtan.fPositionZ = -122.f;
	valtan.fEngageDistance = 35.f;
	valtan.fMoveSpeed = 3.f;
	/* This fixture asserts the observed 130-bar mechanic, so the encounter intro is
	staged as already consumed exactly as a Debug audition reset does. */
	valtan.bIntroPatternConsumed = true;
	valtan.bAutomaticPatternSequenceAuditionOverride = true;
	CValtanBrain brain;
	std::vector<DAMAGE_EVENT> valtanDamageEvents;
	brain.Update(valtan, players, catalog, navigation, 1.f / 30.f, 99,
		{}, valtanDamageEvents);
	tests.Require(
		SERVER_ENTITY_ACTION::IDLE == valtan.eAction &&
		1000u == players.begin()->second.iCurrentHp &&
		valtanDamageEvents.empty(),
		"Protect Valtan entrant until first accepted gameplay intent");
	const VALTAN_DECISION_TRACE* noTargetTrace =
		brain.Get_LatestDecisionTrace();
	const bool tracedNoTargetCandidates = nullptr != noTargetTrace &&
		std::any_of(
			noTargetTrace->Candidates.begin(), noTargetTrace->Candidates.end(),
			[](const VALTAN_DECISION_CANDIDATE_TRACE& candidate)
			{
				return 0u != (candidate.iExclusionMask &
					VALTAN_EXCLUDE_NO_TARGET);
			});
	tests.Require(
		nullptr != noTargetTrace &&
		VALTAN_DECISION_RESULT::NO_VALID_TARGET == noTargetTrace->eResult &&
		VALTAN_DECISION_SOURCE::NONE == noTargetTrace->eSource &&
		tracedNoTargetCandidates,
		"Record no-target candidate exclusions in the selector decision envelope");
	players.begin()->second.isCombatReady = true;
	for (std::uint32_t tick = 100; tick < 170 && valtanDamageEvents.empty(); ++tick)
		brain.Update(valtan, players, catalog, navigation, 1.f / 30.f, tick,
			{}, valtanDamageEvents);
	tests.Require(781u == players.begin()->second.iCurrentHp,
		"Apply the queued 130-bar Valtan six-direction hit once");
	tests.Require(
		1u == valtanDamageEvents.size() &&
		219u == valtanDamageEvents[0].iAmount &&
		!valtanDamageEvents[0].isOutgoing &&
		players.begin()->second.iNetEntityId ==
			valtanDamageEvents[0].iTargetNetEntityId,
		"Emit one incoming damage event for the 130-bar boss hit");
	tests.Require(
		"VALTAN_FLOOR_WIPE_130" == valtan.strPatternId &&
		valtan.PendingPatternIds.empty() &&
		1u == valtan.TriggeredPatternIds.size() &&
		1u == valtan.iPatternSequence &&
		1u == valtan.iPatternStageIndex,
		"Queue and advance the staged 130-bar scripted mechanic");
	const VALTAN_DECISION_TRACE* forcedHealthTrace =
		brain.Get_LatestDecisionTrace();
	const auto floorWipeOccurrence = std::find_if(
		valtan.MechanicOccurrences.begin(), valtan.MechanicOccurrences.end(),
		[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
		{
			return "VALTAN_FLOOR_WIPE_130" == occurrence.strPatternId;
		});
	tests.Require(
		nullptr != forcedHealthTrace &&
		VALTAN_DECISION_SOURCE::FORCED_HEALTH_BAR ==
			forcedHealthTrace->eSource &&
		VALTAN_DECISION_RESULT::SELECTED == forcedHealthTrace->eResult &&
		"VALTAN_FLOOR_WIPE_130" ==
			forcedHealthTrace->strSelectedPatternId &&
		"VALTAN_FLOOR_WIPE_130" ==
			forcedHealthTrace->strPendingPatternId &&
		1u == forcedHealthTrace->iExpectedPatternSequence &&
		valtan.MechanicOccurrences.end() != floorWipeOccurrence &&
		SERVER_BOSS_MECHANIC_STATE::ACTIVE == floorWipeOccurrence->eState &&
		1u == floorWipeOccurrence->iPatternSequence,
		"Trace forced health selection and advance its stable ledger occurrence");
	valtan.iCurrentHp = 30000;
	brain.Update(valtan, players, catalog, navigation, 1.f / 30.f, 171,
		{}, valtanDamageEvents);
	tests.Require(1u == valtan.iPhase,
		"Keep Valtan phase one until the authored 109 IMPACT ENTER edge");

	/* Losing the last combat-ready target is transient encounter state. Park the
	   active health occurrence at IDLE without a reset latch, then retry the same
	   stable occurrence when target admission returns. */
	const std::uint32_t parkedMechanicSequence = valtan.iPatternSequence;
	players.begin()->second.isCombatReady = false;
	brain.Update(valtan, players, catalog, navigation, 1.f / 30.f, 172u,
		{}, valtanDamageEvents);
	const auto waitingFloorWipeOccurrence = std::find_if(
		valtan.MechanicOccurrences.begin(), valtan.MechanicOccurrences.end(),
		[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
		{
			return "VALTAN_FLOOR_WIPE_130" == occurrence.strPatternId;
		});
	tests.Require(
		!valtan.bMechanicLedgerRequiresReset &&
		SERVER_ENTITY_ACTION::IDLE == valtan.eAction &&
		valtan.strPatternId.empty() &&
		SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED ==
			valtan.PatternTerminalReceipt.eResult &&
		valtan.MechanicOccurrences.end() != waitingFloorWipeOccurrence &&
		SERVER_BOSS_MECHANIC_STATE::QUEUED ==
			waitingFloorWipeOccurrence->eState &&
		SERVER_BOSS_MECHANIC_FAILURE::NONE ==
			waitingFloorWipeOccurrence->eFailure &&
		valtan.PendingPatternIds.end() != std::find(
			valtan.PendingPatternIds.begin(), valtan.PendingPatternIds.end(),
			std::string("VALTAN_FLOOR_WIPE_130")),
		"Park a targetless active mechanic at IDLE and requeue it without mechanic reset");
	players.begin()->second.isCombatReady = true;
	brain.Update(valtan, players, catalog, navigation, 1.f / 30.f, 173u,
		{}, valtanDamageEvents);
	const auto resumedFloorWipeOccurrence = std::find_if(
		valtan.MechanicOccurrences.begin(), valtan.MechanicOccurrences.end(),
		[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
		{
			return "VALTAN_FLOOR_WIPE_130" == occurrence.strPatternId;
		});
	tests.Require(
		!valtan.bMechanicLedgerRequiresReset &&
		"VALTAN_FLOOR_WIPE_130" == valtan.strPatternId &&
		valtan.iPatternSequence > parkedMechanicSequence &&
		valtan.MechanicOccurrences.end() != resumedFloorWipeOccurrence &&
		SERVER_BOSS_MECHANIC_STATE::ACTIVE ==
			resumedFloorWipeOccurrence->eState &&
		valtan.PendingPatternIds.end() == std::find(
			valtan.PendingPatternIds.begin(), valtan.PendingPatternIds.end(),
			std::string("VALTAN_FLOOR_WIPE_130")),
		"Retry the parked health occurrence once a valid target returns");
	CValtanBrain::Fail_Mechanic(
		valtan, "VALTAN_FLOOR_WIPE_130",
		SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT, 174u);
	const std::uint32_t failedMechanicSequence = valtan.iPatternSequence;
	brain.Update(valtan, players, catalog, navigation, 1.f / 30.f, 175u,
		{}, valtanDamageEvents);
	const VALTAN_DECISION_TRACE* resetRequiredTrace =
		brain.Get_LatestDecisionTrace();
	const auto failedFloorWipeOccurrence = std::find_if(
		valtan.MechanicOccurrences.begin(), valtan.MechanicOccurrences.end(),
		[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
		{
			return "VALTAN_FLOOR_WIPE_130" == occurrence.strPatternId;
		});
	tests.Require(
		valtan.bMechanicLedgerRequiresReset &&
		valtan.MechanicOccurrences.end() != failedFloorWipeOccurrence &&
		SERVER_BOSS_MECHANIC_STATE::FAILED_REQUIRES_RESET ==
			failedFloorWipeOccurrence->eState &&
		failedMechanicSequence == valtan.iPatternSequence &&
		SERVER_ENTITY_ACTION::IDLE == valtan.eAction &&
		nullptr != resetRequiredTrace &&
		VALTAN_DECISION_RESULT::MECHANIC_RESET_REQUIRED ==
			resetRequiredTrace->eResult,
		"Fail a critical mechanic closed until an encounter reset replaces its ledger");
	Run_ValtanMechanicLedger(tests, catalog, navigation, brain);

	Run_ValtanSkyAxe(tests);

	Run_ValtanTimelines(tests);


	SERVER_WORLD_ENTITY openingChargeBoss{};
	openingChargeBoss.iNetEntityId = 901u;
	openingChargeBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	openingChargeBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
	openingChargeBoss.strArchetypeId = "BOSS_VALTAN";
	openingChargeBoss.strEncounterId = "ENCOUNTER_VALTAN";
	openingChargeBoss.iCurrentHp = 59625u;
	openingChargeBoss.iMaximumHp = 60000u;
	openingChargeBoss.iMaximumHealthBars = 160u;
	openingChargeBoss.iLastEvaluatedHealthBar = 160u;
	openingChargeBoss.iPhaseTwoHpPercent = 50u;
	openingChargeBoss.fPositionX = 151.f;
	openingChargeBoss.fPositionY = 22.97f;
	openingChargeBoss.fPositionZ = -122.f;
	openingChargeBoss.fEngageDistance = 35.f;
	openingChargeBoss.fMoveSpeed = 3.f;
	openingChargeBoss.bIntroPatternConsumed = true;
	openingChargeBoss.bAutomaticPatternSequenceAuditionOverride = true;
	brain.Update(
		openingChargeBoss, players, catalog, navigation,
		1.f / 30.f, 200u, {}, valtanDamageEvents);
	tests.Require(
		openingChargeBoss.strPatternId == "VALTAN_ARMOR_BREAK_OPENING" &&
		openingChargeBoss.strPatternStageId == "WALL_CHARGE" &&
		openingChargeBoss.strActionId ==
			"valtan.mechanic.armor-break-opening.charge" &&
		std::abs(openingChargeBoss.fPatternForcedMotionSpeed -
			(20.f / 1.5f)) <= 0.001f,
		"Use the opening stage's authored arena charge distance");
	SERVER_WORLD_ENTITY continuousOpeningCharge = openingChargeBoss;
	const float openingStartX = continuousOpeningCharge.fPositionX;
	const float openingStartZ = continuousOpeningCharge.fPositionZ;
	const float openingStepDistance = (20.f / 1.5f) / 30.f;
	bool openingStepsContinuous = true;
	for (std::uint32_t tick = 0u; tick < 15u; ++tick)
	{
		float proposedX = 0.f;
		float proposedZ = 0.f;
		if (!brain.Try_BuildStageMotion(
				continuousOpeningCharge, 1.f / 30.f,
				proposedX, proposedZ))
		{
			openingStepsContinuous = false;
			break;
		}
		const float stepX = proposedX - continuousOpeningCharge.fPositionX;
		const float stepZ = proposedZ - continuousOpeningCharge.fPositionZ;
		openingStepsContinuous = openingStepsContinuous &&
			std::abs(std::sqrt(stepX * stepX + stepZ * stepZ) -
				openingStepDistance) <= 0.001f;
		continuousOpeningCharge.fPositionX = proposedX;
		continuousOpeningCharge.fPositionZ = proposedZ;
	}
	const float openingTravelX =
		continuousOpeningCharge.fPositionX - openingStartX;
	const float openingTravelZ =
		continuousOpeningCharge.fPositionZ - openingStartZ;
	tests.Require(
		openingStepsContinuous &&
		std::abs(std::sqrt(
			openingTravelX * openingTravelX + openingTravelZ * openingTravelZ) -
			openingStepDistance * 15.f) <= 0.002f,
		"Advance the opening charge through continuous fixed-tick intermediate positions");
	Run_WorldTriggers(tests, catalog);

	Run_SkillStages(tests, catalog);

	Run_SpawnGroups(tests, catalog);

	Run_ValtanDash(tests, catalog, VALTAN_WALL_COLLISION_STATE, VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z);

	Run_ValtanAudition(tests);

	Run_ValtanResetlessNext(tests, VALTAN_WALL_COLLISION_STATE);



	Run_ValtanReleaseControl(tests);

	Run_WorldDestruction(tests, catalog, navigation);


	tests.failures += Run_WorldDestructionBootstrapContractTests();
	std::cout << "failures : " << tests.failures << '\n';
	return 0 == tests.failures ? 0 : 1;

}
