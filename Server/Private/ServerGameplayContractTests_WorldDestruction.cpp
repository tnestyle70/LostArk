#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "EncounterPropRuntime.h"
#include "Gameplay/CombatCollisionContract.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "ServerNavigation.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_WorldDestruction(TESTS& tests, CGameplayCatalog& catalog, CServerNavigation& navigation)
{


	{
		/* A product revive is the authoritative signal that the only dead player
		is back in the raid. Drive the real room handler and tick instead of
		setting isCombatReady by hand, because the latter used to let the Brain
		test pass while the live room kept Valtan latched in IDLE. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr PLAYER_ID REVIVED_PLAYER = 211u;
		constexpr SESSION_ID REVIVED_SESSION = 4411u;
		constexpr NET_ENTITY_ID REVIVED_ENTITY = 1211u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();

		SERVER_PLAYER player{};
		player.iSessionId = REVIVED_SESSION;
		player.iPlayerId = REVIVED_PLAYER;
		player.iNetEntityId = REVIVED_ENTITY;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.strNickName = "PartyWipeRevive";
		player.strSpawnPlacementId = "player_1";
		player.iCurrentHp = 0u;
		player.iMaximumHp = 5500u;
		player.iCurrentResource = 0u;
		player.iMaximumResource = 1000u;
		player.eAction = PLAYER_ACTION_STATE::DEAD;
		player.isCombatReady = false;
		if (nullptr != boss)
		{
			player.fPositionX = boss->fPositionX + 2.f;
			player.fPositionY = boss->fPositionY;
			player.fPositionZ = boss->fPositionZ;
		}
		room.m_Players.emplace(REVIVED_PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(REVIVED_SESSION, REVIVED_PLAYER);
		room.m_PlayerIdByEntityId.emplace(REVIVED_ENTITY, REVIVED_PLAYER);

		if (nullptr != boss)
		{
			boss->bIntroPatternConsumed = true;
			boss->bAutomaticPatternSequenceAuditionOverride = true;
			boss->eAction = SERVER_ENTITY_ACTION::IDLE;
			boss->strPatternId.clear();
			boss->strPatternStageId.clear();
			boss->strActionId.clear();
			boss->PendingPatternIds = { "VALTAN_FLOOR_WIPE_130" };
			boss->TriggeredPatternIds.push_back("VALTAN_FLOOR_WIPE_130");
			SERVER_BOSS_MECHANIC_OCCURRENCE waiting{};
			waiting.strPatternId = "VALTAN_FLOOR_WIPE_130";
			waiting.PinnedDefinitionRevision =
				room.m_GameplayCatalog.Get_ActiveRevision();
			waiting.eState = SERVER_BOSS_MECHANIC_STATE::QUEUED;
			waiting.eFailure = SERVER_BOSS_MECHANIC_FAILURE::NONE;
			waiting.iTriggerHealthBar = 130u;
			waiting.iQueuedTick = 10u;
			boss->MechanicOccurrences.push_back(std::move(waiting));
			boss->bMechanicLedgerRequiresReset = false;
		}

		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		const bool waitedWithoutReset = nullptr != boss &&
			SERVER_ENTITY_ACTION::IDLE == boss->eAction &&
			boss->strPatternId.empty() &&
			!boss->bMechanicLedgerRequiresReset &&
			boss->PendingPatternIds.end() != std::find(
				boss->PendingPatternIds.begin(), boss->PendingPatternIds.end(),
				std::string("VALTAN_FLOOR_WIPE_130"));
		C2S_REVIVE_PLAYER revive{};
		revive.iClientSequence = 1u;
		room.Handle_RevivePlayer(REVIVED_SESSION, revive);
		const bool admittedOnRevive =
			room.m_Players.at(REVIVED_PLAYER).isCombatReady;
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		const auto occurrence = nullptr == boss ?
			std::vector<SERVER_BOSS_MECHANIC_OCCURRENCE>::const_iterator{} :
			std::find_if(
				boss->MechanicOccurrences.cbegin(),
				boss->MechanicOccurrences.cend(),
				[](const SERVER_BOSS_MECHANIC_OCCURRENCE& value)
				{
					return "VALTAN_FLOOR_WIPE_130" == value.strPatternId;
				});
		const bool occurrenceResumed = nullptr != boss &&
			boss->MechanicOccurrences.cend() != occurrence &&
			SERVER_BOSS_MECHANIC_STATE::ACTIVE == occurrence->eState &&
			"VALTAN_FLOOR_WIPE_130" == boss->strPatternId;
		tests.Require(
			activated && waitedWithoutReset && nullptr != boss &&
			admittedOnRevive && !boss->bMechanicLedgerRequiresReset &&
			occurrenceResumed &&
			SERVER_ENTITY_ACTION::IDLE != boss->eAction,
			"Keep product Valtan IDLE without mechanic reset while targetless and resume its queued occurrence on the authoritative revive tick");
	}

	{
		/* The arena floor collapse is the only authored thing that takes ground
		away from a player, and it kills. Navigation owns where the hole is; the
		room owns the descent and the death tick. This runs in both configurations
		because a fall is product gameplay, not a Debug audition. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr PLAYER_ID FALL_PLAYER = 79u;
		constexpr SESSION_ID FALL_SESSION = 4243u;
		/* Exclusively inside navregion.valtan.floor30.rail.7000000000000000001:
		no other authored region owns this cell, so the stage-B collapse is the
		only thing that can open it. */
		constexpr float FALL_RAIL_X = 155.25f;
		constexpr float FALL_RAIL_Z = -107.25f;
		/* The arena core the audition bait stands on. It belongs to no collapse
		region at all and has to stay solid through both stages. Product revive
		uses the boss.valtan.center placement, whose navigation projection remains
		within this small core neighborhood rather than the remote entry spawn. */
		constexpr float ARENA_CORE_X = 154.296f;
		constexpr float ARENA_CORE_Z = -125.219f;
		constexpr float ARENA_CENTER_REVIVE_MAX_METERS = 5.f;
		const std::string railConditionId =
			"condition.valtan.floor30.rail.7000000000000000001.collapsed";

		std::string voidStatus;
		const bool rejectedObstaclePolarity =
			!room.m_ServerNavigation.Set_VoidConditions(
				{ "condition.valtan.entrance.frontwallA.destroyed" }, voidStatus);
		tests.Require(
			room.Is_Ready() && room.m_ServerNavigation.Is_Loaded() &&
			rejectedObstaclePolarity &&
			!room.m_ServerNavigation.Is_PointInVoidRegion(
				FALL_RAIL_X, FALL_RAIL_Z) &&
			!room.m_ServerNavigation.Is_PointInVoidRegion(
				ARENA_CORE_X, ARENA_CORE_Z),
			"Refuse a wall condition as a fall region and start the arena with no holes");

		SERVER_NAV_POINT railGround{};
		const bool projectedRail = room.m_ServerNavigation.Project_Point(
			FALL_RAIL_X, FALL_RAIL_Z, railGround);
		SERVER_PLAYER faller{};
		faller.iPlayerId = FALL_PLAYER;
		faller.iNetEntityId = 902u;
		faller.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		faller.iCurrentHp = 1000u;
		faller.iMaximumHp = 1000u;
		faller.isCombatReady = true;
		faller.strSpawnPlacementId = "player_1";
		faller.fPositionX = FALL_RAIL_X;
		faller.fPositionY = railGround.y;
		faller.fPositionZ = FALL_RAIL_Z;
		room.m_Players.emplace(FALL_PLAYER, faller);
		room.m_PlayerIdBySessionId.emplace(FALL_SESSION, FALL_PLAYER);

		room.Tick(1.f / 30.f);
		const SERVER_PLAYER& standing = room.m_Players.at(FALL_PLAYER);
		tests.Require(
			projectedRail &&
			PLAYER_ACTION_STATE::NONE == standing.eAction &&
			1000u == standing.iCurrentHp &&
			standing.fPositionY == railGround.y,
			"Leave a player standing on an intact stage-B rail sector alone");

		/* This is the exact navigation edge the collapse commit applies. Flipping
		it directly keeps the fall contract independent of the pattern schedule
		the destruction bootstrap tests already pin. */
		std::vector<SERVER_NAVIGATION_CONDITION_CHANGE> collapseChanges;
		collapseChanges.push_back({ railConditionId, true });
		SERVER_NAVIGATION_CONDITION_STAGE collapseStage{};
		std::string collapseStatus;
		const bool openedHole =
			room.m_ServerNavigation.Prepare_ConditionChanges(
				collapseChanges, collapseStage, collapseStatus);
		if (openedHole)
			room.m_ServerNavigation.Commit_ConditionChanges(
				std::move(collapseStage));
		tests.Require(
			openedHole &&
			room.m_ServerNavigation.Is_PointInVoidRegion(
				FALL_RAIL_X, FALL_RAIL_Z) &&
			!room.m_ServerNavigation.Is_PointInVoidRegion(
				ARENA_CORE_X, ARENA_CORE_Z),
			"Open a fall region only where the stage-B rail sector collapsed");

		const float heightBeforeFall =
			room.m_Players.at(FALL_PLAYER).fPositionY;
		room.Tick(1.f / 30.f);
		const SERVER_PLAYER& falling = room.m_Players.at(FALL_PLAYER);
		tests.Require(
			PLAYER_ACTION_STATE::FALLING == falling.eAction &&
			0u != falling.iActionStartTick &&
			0u != falling.iFallDeathTick &&
			falling.fPositionY < heightBeforeFall &&
			0u != falling.iCurrentHp &&
			!falling.isCombatReady && !falling.hasMoveGoal &&
			falling.MovePath.empty(),
			"Drop the player into a falling state on the tick after the rail sector collapses");

		/* Falling is not dying yet. The boss must not be able to reach the body
		on the way down, which is what the not-combat-ready flag above buys. */
		for (std::uint32_t tick = 0u; tick < 44u; ++tick)
			room.Tick(1.f / 30.f);
		const bool stillFallingBeforeDeadline =
			PLAYER_ACTION_STATE::FALLING ==
				room.m_Players.at(FALL_PLAYER).eAction &&
			0u != room.m_Players.at(FALL_PLAYER).iCurrentHp;
		room.Tick(1.f / 30.f);
		const SERVER_PLAYER& landed = room.m_Players.at(FALL_PLAYER);
		tests.Require(
			stillFallingBeforeDeadline &&
			PLAYER_ACTION_STATE::DEAD == landed.eAction &&
			0u == landed.iCurrentHp &&
			0u == landed.iFallDeathTick,
			"Kill a falling player at the authored death tick and not before it");

		C2S_REVIVE_PLAYER revive{};
		revive.iClientSequence = 1u;
		room.Handle_RevivePlayer(FALL_SESSION, revive);
		const SERVER_PLAYER& revived = room.m_Players.at(FALL_PLAYER);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == revived.eAction &&
			revived.iCurrentHp == revived.iMaximumHp &&
			revived.isCombatReady &&
			0u == revived.iFallDeathTick &&
			room.m_ServerNavigation.Is_PointWalkableExact(
				revived.fPositionX, revived.fPositionZ) &&
			!room.m_ServerNavigation.Is_PointInVoidRegion(
				revived.fPositionX, revived.fPositionZ),
			"Revive a fall death on walkable ground and immediately restore Valtan target admission");

		/* The arena progression triggers are room-wide triggerOnce, so a revive
		that returns to the entry spawn leaves the player outside a boss fight no
		surviving trigger can let them back into. The authoritative center is also
		far enough from the collapsed rail that the next tick cannot re-enter FALLING. */
		const float reviveDeltaX = revived.fPositionX - ARENA_CORE_X;
		const float reviveDeltaZ = revived.fPositionZ - ARENA_CORE_Z;
		tests.Require(
			reviveDeltaX * reviveDeltaX + reviveDeltaZ * reviveDeltaZ <=
				ARENA_CENTER_REVIVE_MAX_METERS *
				ARENA_CENTER_REVIVE_MAX_METERS,
			"Revive a fall death at the safe arena center instead of the entry spawn or void edge");

		room.Tick(1.f / 30.f);
		tests.Require(
			PLAYER_ACTION_STATE::NONE ==
				room.m_Players.at(FALL_PLAYER).eAction,
			"Keep a revived player standing instead of falling again");
	}
	{
		/* The 109 leap and roar now have exact Product clips, but their world arc
		remains Server state. The authoritative subwindows must carry the body
		from TAKEOFF through DROP to the authored placement before the joined
		IMPACT -> WIDE_REVEAL roar sequence can begin. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr PLAYER_ID LEAP_PLAYER = 78u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* leapBoss = room.Find_AuditionBoss();
		/* Drive one exact pattern: stage the encounter intro as already
		consumed so the first-appearance sweep is not the first sequence. */
		if (nullptr != leapBoss)
		{
			leapBoss->bIntroPatternConsumed = true;
			leapBoss->bAutomaticPatternSequenceAuditionOverride = true;
		}

		SERVER_PLAYER leapPlayer{};
		leapPlayer.iPlayerId = LEAP_PLAYER;
		leapPlayer.iNetEntityId = 901u;
		leapPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		leapPlayer.iCurrentHp = 1000u;
		leapPlayer.iMaximumHp = 1000u;
		leapPlayer.isCombatReady = true;
		if (nullptr != leapBoss)
		{
			leapPlayer.fPositionX = leapBoss->fPositionX + 2.f;
			leapPlayer.fPositionY = leapBoss->fPositionY;
			leapPlayer.fPositionZ = leapBoss->fPositionZ;
		}
		room.m_Players.emplace(LEAP_PLAYER, leapPlayer);

		/* The 109 leap lands on the pattern's compiled anchor, which is the
		measured centre of the outer ring. The boss spawns on that same centre,
		so the anchor is checked against the authored coordinate rather than
		against its distance from the spawn. */
		const BOSS_PATTERN_MOTION* leapMotion = nullptr;
		if (const std::vector<BOSS_PATTERN_DEFINITION>* leapPatterns =
			room.m_GameplayCatalog.Find_BossPatterns("ENCOUNTER_VALTAN"))
		{
			for (const BOSS_PATTERN_DEFINITION& candidate : *leapPatterns)
			{
				if ("VALTAN_ARENA_BREAK_109" == candidate.strPatternId)
				{
					leapMotion = &candidate.Motion;
					break;
				}
			}
		}
		const float anchorX = nullptr == leapMotion ? 0.f : leapMotion->fLandingX;
		const float anchorY = nullptr == leapMotion ? 0.f : leapMotion->fLandingY;
		const float anchorZ = nullptr == leapMotion ? 0.f : leapMotion->fLandingZ;
		const float groundY = anchorY;
		tests.Require(
			activated && nullptr != leapBoss && nullptr != leapMotion &&
			BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR == leapMotion->eKind &&
			"anchor.valtan.arena-break-109.landing" == leapMotion->strAnchorId &&
			leapMotion->fApexHeight > 0.f &&
			0u == leapMotion->iTakeoffStartMs &&
			900u == leapMotion->iTakeoffEndMs &&
			0u == leapMotion->iTravelStartMs &&
			700u == leapMotion->iTravelEndMs &&
			std::abs(anchorX - 156.03f) < 0.01f &&
			std::abs(anchorZ + 122.06f) < 0.01f,
			"Compile the 109 landing anchor on the authored outer-ring centre");

		/* Drive the real pattern rather than assigning stages by hand, so the
		arc is exercised through the same edges the room replicates. */
		if (nullptr != leapBoss)
		{
			leapBoss->fPositionX = leapBoss->fSpawnPositionX + 6.f;
			leapBoss->fPositionZ = leapBoss->fSpawnPositionZ + 6.f;
			leapBoss->iCurrentHp =
				CValtanBrain::Resolve_HealthBarHp(*leapBoss, 109u);
			leapBoss->iLastEvaluatedHealthBar = 110u;
		}
		CValtanBrain leapBrain;
		std::vector<DAMAGE_EVENT> leapDamage;
		std::uint32_t leapTick = 600u;
		const auto tickLeap = [&](const std::uint32_t count)
		{
			for (std::uint32_t index = 0u; index < count; ++index)
			{
				if (nullptr == leapBoss)
					return;
				leapBrain.Update(
					*leapBoss, room.m_Players, room.m_GameplayCatalog,
					room.m_ServerNavigation, 1.f / 30.f, leapTick++, {},
					leapDamage);
			}
		};

		tickLeap(1u);
		tests.Require(
			nullptr != leapBoss &&
			"VALTAN_ARENA_BREAK_109" ==
				(nullptr == leapBoss ? std::string{} : leapBoss->strPatternId) &&
			"TAKEOFF" == (nullptr == leapBoss ?
				std::string{} : leapBoss->strPatternStageId),
			"Begin the 109 phase transition on the authored crossing");

		/* Half of the 900ms TAKEOFF stage at 30Hz. */
		tickLeap(13u);
		tests.Require(
			nullptr != leapBoss &&
			leapBoss->fPositionY > groundY + 1.f,
			"Lift Valtan off the ground during the authored TAKEOFF stage");

		/* Follow the rest of TAKEOFF and all of DROP one tick at a time so the
		arc itself is checked, not just its endpoints: it has to reach the
		authored apex and come all the way back down to the floor. */
		float peakY = nullptr == leapBoss ? 0.f : leapBoss->fPositionY;
		float peakPlanarError = 0.f;
		for (std::uint32_t index = 0u; index < 35u; ++index)
		{
			tickLeap(1u);
			if (nullptr == leapBoss)
				break;
			if (leapBoss->fPositionY > peakY)
				peakY = leapBoss->fPositionY;
			if ("DROP" == leapBoss->strPatternStageId)
			{
				const float dx = leapBoss->fPositionX - anchorX;
				const float dz = leapBoss->fPositionZ - anchorZ;
				peakPlanarError = (std::max)(
					peakPlanarError, std::sqrt(dx * dx + dz * dz));
			}
		}
		tests.Require(
			nullptr != leapBoss && nullptr != leapMotion &&
			std::abs(peakY - (groundY + leapMotion->fApexHeight)) < 0.5f &&
			peakPlanarError > 1.f &&
			leapBoss->fPositionY <= groundY + 0.001f,
			"Carry Valtan through the authored apex and back down to the floor");

		/* TAKEOFF (27 ticks) plus DROP (21) lands inside the 12-tick IMPACT. */
		tickLeap(1u);
		const bool landedExactly = nullptr != leapBoss &&
			"IMPACT" == leapBoss->strPatternStageId &&
			std::abs(leapBoss->fPositionX - anchorX) < 0.001f &&
			std::abs(leapBoss->fPositionY - anchorY) < 0.001f &&
			std::abs(leapBoss->fPositionZ - anchorZ) < 0.001f;
		tests.Require(
			landedExactly,
			"Land the 109 leap exactly on the compiled anchor at IMPACT");
	}

	{
		/* The high jump used to take off and land on its own feet because it
		owned no motion at all. It now follows the target it locked, so the arc
		has to end where that player stood and not where the boss started. */
		const std::vector<BOSS_PATTERN_DEFINITION>* leapPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const BOSS_PATTERN_DEFINITION* highJump = nullptr;
		if (nullptr != leapPatterns)
		{
			const auto found = std::find_if(
				leapPatterns->begin(), leapPatterns->end(),
				[](const BOSS_PATTERN_DEFINITION& candidate)
				{ return candidate.strPatternId == "VALTAN_HIGH_JUMP"; });
			if (leapPatterns->end() != found)
				highJump = &(*found);
		}
		std::map<PLAYER_ID, SERVER_PLAYER> leapArcPlayers;
		SERVER_PLAYER leapArcTarget{};
		leapArcTarget.iPlayerId = 91;
		leapArcTarget.iNetEntityId = 9100;
		leapArcTarget.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		leapArcTarget.iCurrentHp = 1000;
		leapArcTarget.iMaximumHp = 1000;
		leapArcTarget.isCombatReady = true;
		leapArcTarget.fPositionX = 160.5f;
		leapArcTarget.fPositionY = 22.97f;
		leapArcTarget.fPositionZ = -125.5f;
		leapArcPlayers.emplace(leapArcTarget.iPlayerId, leapArcTarget);

		SERVER_WORLD_ENTITY leapArcBoss{};
		leapArcBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		leapArcBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		leapArcBoss.strArchetypeId = "BOSS_VALTAN";
		leapArcBoss.strEncounterId = "ENCOUNTER_VALTAN";
		leapArcBoss.iCurrentHp = 60000;
		leapArcBoss.iMaximumHp = 60000;
		leapArcBoss.iMaximumHealthBars = 160;
		leapArcBoss.iLastEvaluatedHealthBar = 160;
		leapArcBoss.iPhaseTwoHpPercent = 68;
		leapArcBoss.iPhase = 1;
		leapArcBoss.fPositionX = 156.03f;
		leapArcBoss.fPositionY = 22.97f;
		leapArcBoss.fPositionZ = -122.06f;
		leapArcBoss.fSpawnPositionX = leapArcBoss.fPositionX;
		leapArcBoss.fSpawnPositionY = leapArcBoss.fPositionY;
		leapArcBoss.fSpawnPositionZ = leapArcBoss.fPositionZ;
		leapArcBoss.fEngageDistance = 35.f;
		leapArcBoss.fMoveSpeed = 3.f;
		leapArcBoss.bIntroPatternConsumed = true;
		const SERVER_WORLD_ENTITY initialLeapBoss = leapArcBoss;
		/* Queued the way a crossed health bar queues its mechanic, so the jump
		starts on the next tick without waiting for a weighted roll to name it. */
		leapArcBoss.PendingPatternIds.push_back("VALTAN_HIGH_JUMP");
		CValtanBrain leapArcBrain;
		std::vector<DAMAGE_EVENT> leapArcDamage;
		leapArcBrain.Update(
			leapArcBoss, leapArcPlayers, catalog, navigation,
			1.f / 30.f, 900u, {}, leapArcDamage);
		const bool startedTheJump =
			"VALTAN_HIGH_JUMP" == leapArcBoss.strPatternId;
		const bool landsOnTheTarget =
			std::abs(leapArcBoss.fLeapLandingX - leapArcTarget.fPositionX) < 0.01f &&
			std::abs(leapArcBoss.fLeapLandingZ - leapArcTarget.fPositionZ) < 0.01f;
		const bool leftItsOwnFeet =
			std::abs(leapArcBoss.fLeapLandingX - leapArcBoss.fSpawnPositionX) > 1.f ||
			std::abs(leapArcBoss.fLeapLandingZ - leapArcBoss.fSpawnPositionZ) > 1.f;
		tests.Require(
			nullptr != highJump &&
			BOSS_PATTERN_MOTION_KIND::LEAP_TO_TARGET == highJump->Motion.eKind &&
			1133u == highJump->Motion.iTakeoffStartMs &&
			1500u == highJump->Motion.iTakeoffEndMs &&
			0u == highJump->Motion.iTravelStartMs &&
			267u == highJump->Motion.iTravelEndMs &&
			startedTheJump && landsOnTheTarget && leftItsOwnFeet &&
			leapArcBoss.fPatternLeapApexHeight == highJump->Motion.fApexHeight,
			"Land the high jump on its locked player with the authored fast lift/drop windows");

		std::uint32_t leapArcTick = 901u;
		float heightAtOneSecond = leapArcBoss.fPositionY;
		float heightAtTwelveHundredMs = leapArcBoss.fPositionY;
		float heightAtFifteenHundredMs = leapArcBoss.fPositionY;
		for (std::uint32_t tick = 0u;
			tick < 70u && "TAKEOFF" == leapArcBoss.strPatternStageId; ++tick)
		{
			leapArcDamage.clear();
			leapArcBrain.Update(
				leapArcBoss, leapArcPlayers, catalog, navigation,
				1.f / 30.f, leapArcTick++, {}, leapArcDamage);
			if (29u == tick)
				heightAtOneSecond = leapArcBoss.fPositionY;
			else if (35u == tick)
				heightAtTwelveHundredMs = leapArcBoss.fPositionY;
			else if (44u == tick)
				heightAtFifteenHundredMs = leapArcBoss.fPositionY;
		}
		const float expectedApexY =
			leapArcBoss.fLeapOriginY +
			(nullptr == highJump ? 0.f : highJump->Motion.fApexHeight);
		tests.Require(
			std::abs(heightAtOneSecond - leapArcBoss.fLeapOriginY) < 0.01f &&
			heightAtTwelveHundredMs > leapArcBoss.fLeapOriginY + 1.f &&
			heightAtTwelveHundredMs < expectedApexY - 1.f &&
			std::abs(heightAtFifteenHundredMs - expectedApexY) < 0.1f,
			"Keep the high-jump anticipation grounded, then reach the apex in the authored short lift window");
		bool heldAtApex = "AIRBORNE" == leapArcBoss.strPatternStageId;
		bool enteredLand = false;
		const std::uint32_t airborneHoldStartTick = leapArcTick;
		for (std::uint32_t tick = 0u;
			tick < 260u && !enteredLand; ++tick)
		{
			if ("AIRBORNE" == leapArcBoss.strPatternStageId)
			{
				heldAtApex = heldAtApex &&
					std::abs(leapArcBoss.fPositionX - leapArcBoss.fLeapOriginX) <
						0.001f &&
					std::abs(leapArcBoss.fPositionY - expectedApexY) < 0.001f &&
					std::abs(leapArcBoss.fPositionZ - leapArcBoss.fLeapOriginZ) <
						0.001f;
			}
			leapArcDamage.clear();
			leapArcBrain.Update(
				leapArcBoss, leapArcPlayers, catalog, navigation,
				1.f / 30.f, leapArcTick++, {}, leapArcDamage);
			enteredLand = "LAND" == leapArcBoss.strPatternStageId;
		}
		const std::uint32_t airborneHoldTicks = leapArcTick - airborneHoldStartTick;
		bool enteredRecovery = false;
		bool landedInsideFastWindow = false;
		for (std::uint32_t tick = 0u;
			tick < 110u && !enteredRecovery; ++tick)
		{
			leapArcDamage.clear();
			leapArcBrain.Update(
				leapArcBoss, leapArcPlayers, catalog, navigation,
				1.f / 30.f, leapArcTick++, {}, leapArcDamage);
			if (8u == tick)
			{
				landedInsideFastWindow =
					std::abs(leapArcBoss.fPositionX - leapArcTarget.fPositionX) < 0.01f &&
					std::abs(leapArcBoss.fPositionY - leapArcTarget.fPositionY) < 0.01f &&
					std::abs(leapArcBoss.fPositionZ - leapArcTarget.fPositionZ) < 0.01f;
			}
			enteredRecovery = "RECOVERY" == leapArcBoss.strPatternStageId;
		}
		tests.Require(
			heldAtApex && enteredLand && airborneHoldTicks >= 240u && airborneHoldTicks <= 241u &&
			landedInsideFastWindow && enteredRecovery &&
			2u == leapArcBoss.iPatternLeapTravelStageIndex &&
			std::abs(leapArcBoss.fPositionX - leapArcTarget.fPositionX) < 0.01f &&
			std::abs(leapArcBoss.fPositionY - leapArcTarget.fPositionY) < 0.01f &&
			std::abs(leapArcBoss.fPositionZ - leapArcTarget.fPositionZ) < 0.01f,
			"Hold the high jump at its apex for AIRBORNE and finish the drop in LAND's authored fast window");

		// Only Six Pizza's first landing uses this authored Server arc.
		// Its anticipation, animation stages and later jumps stay unchanged.
		SERVER_WORLD_ENTITY pizzaLeapBoss = initialLeapBoss;
		pizzaLeapBoss.fPositionX -= 4.f;
		pizzaLeapBoss.PendingPatternIds.push_back("VALTAN_SIX_PIZZA_106");
		std::map<PLAYER_ID, SERVER_PLAYER> pizzaLeapPlayers;
		pizzaLeapPlayers.emplace(leapArcTarget.iPlayerId, leapArcTarget);
		std::vector<DAMAGE_EVENT> pizzaLeapDamage;
		std::uint32_t pizzaTick = 3000u;
		const auto advancePizza = [&]()
		{
			pizzaLeapDamage.clear();
			leapArcBrain.Update(pizzaLeapBoss, pizzaLeapPlayers, catalog,
				navigation, 1.f / 30.f, pizzaTick++, {}, pizzaLeapDamage);
		};
		advancePizza();
		const bool pizzaStarted = "VALTAN_SIX_PIZZA_106" == pizzaLeapBoss.strPatternId;
		tests.Require(pizzaStarted && nullptr != highJump &&
			800u == pizzaLeapBoss.iPatternLeapTakeoffStartMs &&
			1100u == pizzaLeapBoss.iPatternLeapTakeoffEndMs &&
			10.f == pizzaLeapBoss.fPatternLeapApexHeight &&
			2u == pizzaLeapBoss.iPatternLeapTravelStageIndex &&
			pizzaLeapBoss.iPatternLeapTravelEndMs - pizzaLeapBoss.iPatternLeapTravelStartMs ==
				highJump->Motion.iTravelEndMs - highJump->Motion.iTravelStartMs,
			"Match only Six Pizza's first landing window to the axe jump and preserve its lift");
		for (std::uint32_t tick = 0u; tick < 90u &&
			"STEP_03" != pizzaLeapBoss.strPatternStageId; ++tick)
		{
			advancePizza();
		}
		const bool pizzaEnteredLand = "STEP_03" == pizzaLeapBoss.strPatternStageId;
		const float pizzaApexY = pizzaLeapBoss.fPositionY;
		for (std::uint32_t tick = 0u; tick < 4u; ++tick)
			advancePizza();
		const bool pizzaFallsInsideWindow =
			pizzaLeapBoss.fPositionY < pizzaApexY - 1.f &&
			pizzaLeapBoss.fPositionY > pizzaLeapBoss.fLeapLandingY + 1.f;
		for (std::uint32_t tick = 4u; tick < 9u; ++tick)
			advancePizza();
		const bool pizzaLanded =
			std::abs(pizzaLeapBoss.fPositionX - pizzaLeapBoss.fLeapLandingX) < 0.01f &&
			std::abs(pizzaLeapBoss.fPositionY - pizzaLeapBoss.fLeapLandingY) < 0.01f &&
			std::abs(pizzaLeapBoss.fPositionZ - pizzaLeapBoss.fLeapLandingZ) < 0.01f;
		tests.Require(pizzaEnteredLand && pizzaFallsInsideWindow && pizzaLanded &&
			"STEP_03" == pizzaLeapBoss.strPatternStageId,
			"Land Six Pizza at its center anchor by the first tick after 267ms without shortening the animation stage");
		for (std::uint32_t tick = 0u; tick < 40u &&
			"STEP_04" != pizzaLeapBoss.strPatternStageId; ++tick)
		{
			advancePizza();
		}
		tests.Require("STEP_04" == pizzaLeapBoss.strPatternStageId &&
			0.f == pizzaLeapBoss.fLeapApexHeight &&
			std::abs(pizzaLeapBoss.fPositionY - pizzaLeapBoss.fLeapLandingY) < 0.01f,
			"Release Six Pizza's first leap before subsequent animation stages");

		/* With nobody to lock, the arc still needs a real destination or the
		boss would drop through an uninitialised landing. */
		std::map<PLAYER_ID, SERVER_PLAYER> emptyLeapPlayers;
		SERVER_WORLD_ENTITY targetlessBoss = leapArcBoss;
		targetlessBoss.strPatternId.clear();
		targetlessBoss.strPatternStageId.clear();
		targetlessBoss.PendingPatternIds.clear();
		targetlessBoss.PendingPatternIds.push_back("VALTAN_HIGH_JUMP");
		targetlessBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		targetlessBoss.fLeapLandingX = 0.f;
		targetlessBoss.fLeapLandingZ = 0.f;
		std::vector<DAMAGE_EVENT> targetlessDamage;
		CValtanBrain targetlessBrain;
		targetlessBrain.Update(
			targetlessBoss, emptyLeapPlayers, catalog, navigation,
			1.f / 30.f, 940u, {}, targetlessDamage);
		tests.Require(
			nullptr != highJump &&
			0.f == targetlessBoss.fLeapLandingX &&
			0.f == targetlessBoss.fLeapLandingZ,
			"Leave a targetless high jump alone rather than starting a blind arc");
	}

	{
		/* The completed 109 outer ring encloses the arena on every bearing, so
		nothing walks in or out of it until the collapse. The player reaches the
		arena through Stage_Boss_ArenaEntry instead, and the 159 wall's own
		passage is proved inside the ring by the FRACTURED collision test above.
		All bearings here were measured against the published navgrid. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr float ARENA_CENTER_X = 156.03f;
		constexpr float ARENA_CENTER_Z = -122.06f;
		/* 131 degrees used to be the mouth of the walk-in corridor, back when
		the ring was still missing six slabs. The completed ring seals every
		bearing until the 109 collapse, and Stage_Boss_ArenaEntry now carries
		the player across it, so this bearing must block like the rest. */
		const auto sweepAcrossRing = [&](const float bearingDegrees)
		{
			const float radians = bearingDegrees * 3.14159265f / 180.f;
			SERVER_PLAYER walker{};
			walker.fPositionX = ARENA_CENTER_X + std::cos(radians) * 15.f;
			walker.fPositionY = 23.04f;
			walker.fPositionZ = ARENA_CENTER_Z + std::sin(radians) * 15.f;
			float resolvedX = 0.f;
			float resolvedY = 0.f;
			float resolvedZ = 0.f;
			bool blocked = false;
			const bool resolved = room.m_ServerCollisionSystem.Resolve_PlayerMove(
				walker,
				ARENA_CENTER_X + std::cos(radians) * 21.f,
				walker.fPositionY,
				ARENA_CENTER_Z + std::sin(radians) * 21.f,
				resolvedX, resolvedY, resolvedZ, blocked);
			return resolved && blocked;
		};
		tests.Require(
			room.Is_Ready() && sweepAcrossRing(0.f) && sweepAcrossRing(60.f) &&
			sweepAcrossRing(216.f),
			"Block outward movement through the intact 109 outer ring");
		tests.Require(
			room.Is_Ready() && sweepAcrossRing(131.f) &&
			sweepAcrossRing(150.f) && sweepAcrossRing(294.f),
			"Seal the former entrance and 159 gaps with the completed 109 ring");

		/* Every wall that stands on authored floor now owns a blocker region,
		so pathfinding stops at it instead of walking through, and both the
		Stage_Boss_ArenaEntry landing point and the boss spawn stay reachable
		inside the sealed ring because their cells were kept out of them. */
		std::vector<SERVER_NAV_POINT> wallPath;
		const bool bossActivated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		const SERVER_WORLD_ENTITY* spawnedBoss = room.Find_AuditionBoss();
		std::vector<SERVER_NAV_POINT> bossPath;
		tests.Require(
			bossActivated && nullptr != spawnedBoss &&
			room.m_ServerNavigation.Find_Path(
				147.75f, -117.25f, 156.25f, -122.25f, wallPath) &&
			!wallPath.empty() &&
			room.m_ServerNavigation.Find_Path(
				spawnedBoss->fSpawnPositionX, spawnedBoss->fSpawnPositionZ,
				156.25f, -122.25f, bossPath) &&
			!bossPath.empty(),
			"Keep the arena and the Valtan spawn connected under the wall blockers");
	}

	{
		/* The pillars are the one encounter prop that has to come back. Wall
		groups leave INTACT once and never return, so this runtime is checked
		on exactly that difference: the same four slots must cycle. */
		ENCOUNTER_PROP_SET_DESCRIPTOR descriptor{};
		descriptor.strPropSetId = "encounterprop.valtan.four-pillars";
		descriptor.strEncounterId = "ENCOUNTER_VALTAN";
		descriptor.fCoverRadiusMeters = 0.9f;
		/* Authored order is preserved, so the set is deliberately not sorted. */
		descriptor.Slots = {
			{ "pillar.valtan.slot03", 152.423755f, -118.453755f },
			{ "pillar.valtan.slot00", 159.636245f, -118.453755f },
			{ "pillar.valtan.slot02", 152.423755f, -125.666245f },
			{ "pillar.valtan.slot01", 159.636245f, -125.666245f } };

		std::string status;
		CEncounterPropRuntime runtime;
		const bool initialized = runtime.Initialize(descriptor, status, 1u);
		const std::vector<ENCOUNTER_PROP_SLOT_STATE>& slots =
			runtime.Get_SlotStates();
		tests.Require(
			initialized && 4u == slots.size() &&
			std::is_sorted(slots.begin(), slots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& left,
					const ENCOUNTER_PROP_SLOT_STATE& right)
				{
					return left.strSlotId < right.strSlotId;
				}) &&
			std::all_of(slots.begin(), slots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return ENCOUNTER_PROP_STATE::HIDDEN == slot.eState &&
						0u == slot.iOccurrenceSequence;
				}),
			"Initialize the four pillar slots hidden in canonical slot order");

		ENCOUNTER_PROP_SET_DESCRIPTOR duplicated = descriptor;
		duplicated.Slots.push_back(
			{ "pillar.valtan.slot00", 152.423755f, -118.453755f });
		CEncounterPropRuntime rejected;
		ENCOUNTER_PROP_SET_DESCRIPTOR nameless = descriptor;
		nameless.Slots[1u].strSlotId.clear();
		CEncounterPropRuntime alsoRejected;
		tests.Require(
			!rejected.Initialize(duplicated, status, 1u) &&
			!rejected.Is_Initialized() &&
			!alsoRejected.Initialize(nameless, status, 1u) &&
			!alsoRejected.Is_Initialized(),
			"Reject a duplicate or nameless pillar slot without a partial set");

		ENCOUNTER_PROP_TRANSACTION raise{};
		const bool raised =
			ENCOUNTER_PROP_PREPARE_RESULT::READY == runtime.Prepare_Spawn(
				7u, 100u, raise, status) &&
			4u == raise.Slots.size() &&
			runtime.Commit(raise, status);
		ENCOUNTER_PROP_TRANSACTION repeated{};
		const ENCOUNTER_PROP_PREPARE_RESULT repeatedResult =
			runtime.Prepare_Spawn(7u, 104u, repeated, status);
		tests.Require(
			raised && 7u == runtime.Get_OccurrenceSequence() &&
			std::all_of(slots.begin(), slots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return ENCOUNTER_PROP_STATE::INTACT == slot.eState &&
						7u == slot.iOccurrenceSequence &&
						100u == slot.iStateStartTick;
				}) &&
			ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE == repeatedResult &&
			repeated.Slots.empty(),
			"Raise the four pillars once and ignore the repeated raise edge");

		ENCOUNTER_PROP_TRANSACTION stale = raise;
		ENCOUNTER_PROP_TRANSACTION shatter{};
		const bool shattered =
			!runtime.Commit(stale, status) &&
			ENCOUNTER_PROP_PREPARE_RESULT::READY == runtime.Prepare_Break(
				7u, 200u, shatter, status) &&
			runtime.Commit(shatter, status);
		ENCOUNTER_PROP_TRANSACTION early{};
		ENCOUNTER_PROP_TRANSACTION due{};
		const ENCOUNTER_PROP_PREPARE_RESULT earlyResult =
			runtime.Prepare_DueRemoval(207u, 8u, early, status);
		const bool retired =
			ENCOUNTER_PROP_PREPARE_RESULT::READY == runtime.Prepare_DueRemoval(
				208u, 8u, due, status) &&
			runtime.Commit(due, status);
		tests.Require(
			shattered && ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE ==
				earlyResult && early.Slots.empty() && retired &&
			std::all_of(slots.begin(), slots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return ENCOUNTER_PROP_STATE::HIDDEN == slot.eState;
				}),
			"Shatter the pillars and retire them only on the authored due tick");

		ENCOUNTER_PROP_TRANSACTION nextCycle{};
		const bool cycled =
			ENCOUNTER_PROP_PREPARE_RESULT::READY == runtime.Prepare_Spawn(
				8u, 300u, nextCycle, status) &&
			runtime.Commit(nextCycle, status);
		tests.Require(
			cycled && 8u == runtime.Get_OccurrenceSequence() &&
			std::all_of(slots.begin(), slots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return ENCOUNTER_PROP_STATE::INTACT == slot.eState &&
						8u == slot.iOccurrenceSequence;
				}),
			"Raise the same four slots again on the next pattern occurrence");

		ENCOUNTER_PROP_TRANSACTION crossEpoch{};
		const bool preparedBeforeReset =
			ENCOUNTER_PROP_PREPARE_RESULT::READY == runtime.Prepare_Break(
				8u, 320u, crossEpoch, status);
		const std::uint32_t epochBeforeReset = runtime.Get_EncounterEpoch();
		tests.Require(
			preparedBeforeReset && runtime.Reset(status, 400u) &&
			epochBeforeReset != runtime.Get_EncounterEpoch() &&
			0u == runtime.Get_OccurrenceSequence() &&
			!runtime.Commit(crossEpoch, status) &&
			std::all_of(slots.begin(), slots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return ENCOUNTER_PROP_STATE::HIDDEN == slot.eState &&
						0u == slot.iOccurrenceSequence;
				}),
			"Reset the pillars to hidden and refuse a transaction from the old epoch");
	}

	{
		/* Cover is the attack segment against the stele circle, not a
		containment test on either end. A player directly behind the stele is
		answered by it; one standing beside it, or in front of it, is not. */
		using LostArk::Shared::CombatCollision::CIRCLE_XZ;
		using LostArk::Shared::CombatCollision::Segment_IntersectsCircle;
		const CIRCLE_XZ stele{ 0.f, 5.f, 0.9f };
		const bool behindIsAnswered =
			Segment_IntersectsCircle(0.f, 0.f, 0.f, 10.f, stele);
		const bool besideIsExposed =
			!Segment_IntersectsCircle(0.f, 0.f, 6.f, 10.f, stele);
		const bool inFrontIsExposed =
			!Segment_IntersectsCircle(0.f, 0.f, 0.f, 3.f, stele);
		const bool zeroRadiusIsExposed = !Segment_IntersectsCircle(
			0.f, 0.f, 0.f, 10.f, CIRCLE_XZ{ 0.f, 5.f, 0.f });
		tests.Require(
			behindIsAnswered && besideIsExposed && inFrontIsExposed &&
			zeroRadiusIsExposed,
			"Answer a blow only where the stele stands between boss and player");
	}

	{
		/* A stele stops being cover on the tick it starts breaking, which is
		what hands the raid over to the opposite diagonal. */
		ENCOUNTER_PROP_SET_DESCRIPTOR coverSet{};
		coverSet.strPropSetId = "encounterprop.valtan.four-pillars";
		coverSet.strEncounterId = "ENCOUNTER_VALTAN";
		coverSet.fCoverRadiusMeters = 0.9f;
		coverSet.Slots = {
			{ "pillar.valtan.slot00", 159.636245f, -118.453755f },
			{ "pillar.valtan.slot01", 159.636245f, -125.666245f },
			{ "pillar.valtan.slot02", 152.423755f, -125.666245f },
			{ "pillar.valtan.slot03", 152.423755f, -118.453755f } };
		std::string coverStatus;
		CEncounterPropRuntime coverRuntime;
		const auto intactCount = [&coverRuntime]()
		{
			const std::vector<ENCOUNTER_PROP_SLOT_STATE>& live =
				coverRuntime.Get_SlotStates();
			return std::count_if(live.begin(), live.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{ return ENCOUNTER_PROP_STATE::INTACT == slot.eState; });
		};
		ENCOUNTER_PROP_TRANSACTION coverRaise{};
		const bool coverRaised =
			coverRuntime.Initialize(coverSet, coverStatus, 1u) &&
			ENCOUNTER_PROP_PREPARE_RESULT::READY == coverRuntime.Prepare_Spawn(
				1u, 10u, coverRaise, coverStatus) &&
			coverRuntime.Commit(coverRaise, coverStatus);
		/* The authored position has to survive the raise, because the cover
		circle is built from the live slot rather than a second lookup. */
		const std::vector<ENCOUNTER_PROP_SLOT_STATE>& coverSlots =
			coverRuntime.Get_SlotStates();
		const bool carriedPositions = coverRaised && 4u == coverSlots.size() &&
			std::all_of(coverSlots.begin(), coverSlots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return 0.f != slot.fPositionX && 0.f != slot.fPositionZ;
				});
		/* Counted before the shatter, because the same query answers both
		states and the order of the two checks would otherwise decide them. */
		const auto raisedCoverCount = intactCount();
		ENCOUNTER_PROP_TRANSACTION coverShatter{};
		const std::vector<std::string> firstDiagonal = {
			"pillar.valtan.slot00", "pillar.valtan.slot02" };
		const bool diagonalShattered =
			ENCOUNTER_PROP_PREPARE_RESULT::READY ==
				coverRuntime.Prepare_BreakSlots(
					firstDiagonal, 1u, 20u, coverShatter, coverStatus) &&
			coverRuntime.Commit(coverShatter, coverStatus);
		const auto remainingCoverCount = intactCount();
		tests.Require(
			carriedPositions && 4 == raisedCoverCount,
			"Raise four stele slots carrying the authored cover position");
		tests.Require(
			diagonalShattered && 2 == remainingCoverCount &&
			0.9f == coverRuntime.Get_CoverRadiusMeters(),
			"Leave only the opposite diagonal standing as cover");
	}

	{
		/* The stele contracts are only real once the publisher wrote them into
		the bootstrap the Server actually reads, so they are checked against the
		loaded catalog rather than against the authoring documents. */
		const std::vector<BOSS_PATTERN_DEFINITION>* valtanPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto findStage = [valtanPatterns](
			const char* patternId, const char* stageId)
			-> const BOSS_PATTERN_STAGE_DEFINITION*
		{
			if (nullptr == valtanPatterns)
				return nullptr;
			const auto pattern = std::find_if(
				valtanPatterns->begin(), valtanPatterns->end(),
				[patternId](const BOSS_PATTERN_DEFINITION& candidate)
				{ return candidate.strPatternId == patternId; });
			if (valtanPatterns->end() == pattern)
				return nullptr;
			const auto stage = std::find_if(
				pattern->Stages.begin(), pattern->Stages.end(),
				[stageId](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strStageId == stageId; });
			return pattern->Stages.end() == stage ? nullptr : &(*stage);
		};
		const BOSS_PATTERN_STAGE_DEFINITION* pierceStage =
			findStage("VALTAN_FOUR_PILLARS_105", "TARGET_CONE");
		const BOSS_PATTERN_STAGE_DEFINITION* ringStage =
			findStage("VALTAN_FOUR_PILLARS_105", "YELLOW_ZONE");
		tests.Require(
			nullptr != pierceStage && nullptr != ringStage &&
			pierceStage->bPiercesCover && !ringStage->bPiercesCover &&
			pierceStage->strDamageProfileId !=
				ringStage->strDamageProfileId,
			"Grant cover piercing to the authored cone alone and on its own damage");

		const BOSS_PATTERN_STAGE_DEFINITION* firstBreak =
			findStage("VALTAN_RED_BLADE_WAVE", "PROJECTILE");
		const BOSS_PATTERN_STAGE_DEFINITION* secondBreak =
			findStage("VALTAN_RED_BLADE_WAVE", "RECOVERY");
		const BOSS_PATTERN_STAGE_DEFINITION* noBreak =
			findStage("VALTAN_RED_BLADE_WAVE", "WINDUP");
		bool pairsAreDisjoint =
			nullptr != firstBreak && nullptr != secondBreak;
		if (pairsAreDisjoint)
		{
			for (const std::string& slotId : firstBreak->PropBreakSlotIds)
			{
				pairsAreDisjoint = pairsAreDisjoint &&
					secondBreak->PropBreakSlotIds.end() == std::find(
						secondBreak->PropBreakSlotIds.begin(),
						secondBreak->PropBreakSlotIds.end(), slotId);
			}
		}
		tests.Require(
			nullptr != firstBreak && nullptr != secondBreak &&
			nullptr != noBreak &&
			2u == firstBreak->PropBreakSlotIds.size() &&
			2u == secondBreak->PropBreakSlotIds.size() &&
			noBreak->PropBreakSlotIds.empty() && pairsAreDisjoint &&
			firstBreak->strPropBreakSetId ==
				"encounterprop.valtan.four-pillars",
			"Break the stele two at a time on two disjoint authored stage edges");
	}

	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto valtanRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& valtanRoom = *valtanRoomStorage;
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto bernRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::BERN);
		CGameRoom& bernRoom = *bernRoomStorage;
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto charSelectRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::CHARACTER_SELECT_ARENA);
		CGameRoom& charSelectRoom = *charSelectRoomStorage;
		tests.Require(
			valtanRoom.Is_Ready() && bernRoom.Is_Ready() &&
			charSelectRoom.Is_Ready() &&
			valtanRoom.m_EstherSkillSystem.Is_Enabled() &&
			1000u == valtanRoom.m_EstherSkillSystem.Get_GaugeMaximum() &&
			charSelectRoom.m_EstherSkillSystem.Is_Enabled() &&
			1000u == charSelectRoom.m_EstherSkillSystem.Get_GaugeMaximum() &&
			!bernRoom.m_EstherSkillSystem.Is_Enabled() &&
			0u == bernRoom.m_EstherSkillSystem.Get_GaugeMaximum(),
			"Own a shared Esther gauge in the Valtan raid room and the Character Select test arena only");

		valtanRoom.m_EstherSkillSystem.Update(2.f, false);
		const std::uint32_t emptyRoomGauge =
			valtanRoom.m_EstherSkillSystem.Get_Gauge();
		for (int tick = 0; tick < 200; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		tests.Require(
			0u == emptyRoomGauge &&
			1000u == valtanRoom.m_EstherSkillSystem.Get_Gauge(),
			"Charge the shared gauge to full only while players occupy the room");

		const ESTHER_ROSTER_ENTRY* pRosterEntry = nullptr;
		const ESTHER_USE_REJECTION wrongSlot =
			valtanRoom.m_EstherSkillSystem.Try_Consume(4u, pRosterEntry);
		tests.Require(
			ESTHER_USE_REJECTION::UNSUPPORTED_SLOT == wrongSlot &&
			nullptr == pRosterEntry &&
			1000u == valtanRoom.m_EstherSkillSystem.Get_Gauge(),
			"Reject an out-of-roster Esther slot without touching the gauge");

		const ESTHER_USE_REJECTION disabledWorld =
			bernRoom.m_EstherSkillSystem.Try_Consume(1u, pRosterEntry);
		tests.Require(
			ESTHER_USE_REJECTION::DISABLED_WORLD == disabledWorld,
			"Reject an Esther use outside the raid world");

		const ESTHER_USE_REJECTION weiSlot =
			valtanRoom.m_EstherSkillSystem.Try_Consume(2u, pRosterEntry);
		tests.Require(
			ESTHER_USE_REJECTION::NONE == weiSlot &&
			nullptr != pRosterEntry &&
			std::string("NPC_58700") == pRosterEntry->pArchetypeId &&
			7100u == pRosterEntry->iStrikeMs &&
			0u == valtanRoom.m_EstherSkillSystem.Get_Gauge(),
			"Consume slot 2 into Wei's all-in-one clip timeline");

		for (int tick = 0; tick < 200; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		pRosterEntry = nullptr;
		const ESTHER_USE_REJECTION bahunturSlot =
			valtanRoom.m_EstherSkillSystem.Try_Consume(3u, pRosterEntry);
		tests.Require(
			ESTHER_USE_REJECTION::NONE == bahunturSlot &&
			nullptr != pRosterEntry &&
			std::string("NPC_59060") == pRosterEntry->pArchetypeId &&
			4100u == pRosterEntry->iStrikeMs &&
			0u == valtanRoom.m_EstherSkillSystem.Get_Gauge(),
			"Consume slot 3 into the Bahuntur summon with its authored strike length");

		/* Handler path: an authenticated caster with a full gauge summons at
		its own feet, aimed east, and the gauge drains to zero atomically. */
		constexpr SESSION_ID casterSessionId = 41u;
		constexpr LostArk::Shared::PLAYER_ID casterPlayerId = 9u;
		SERVER_PLAYER caster{};
		caster.iNetEntityId = 4100u;
		caster.fPositionX = 150.f;
		caster.fPositionY = 22.97f;
		caster.fPositionZ = -120.f;
		caster.fYawDegrees = 0.f;
		valtanRoom.m_Players.emplace(casterPlayerId, caster);
		valtanRoom.m_PlayerIdBySessionId.emplace(
			casterSessionId, casterPlayerId);

		const std::size_t entitiesBeforeSummon =
			valtanRoom.m_WorldEntities.size();
		LostArk::Shared::C2S_USE_ESTHER_SKILL notFullUse{};
		notFullUse.iClientSequence = 1u;
		notFullUse.iSlotIndex = 1u;
		notFullUse.fAimX = 160.f;
		notFullUse.fAimZ = -120.f;
		valtanRoom.m_EstherSkillSystem.Reset();
		valtanRoom.Handle_UseEstherSkill(casterSessionId, notFullUse);
		tests.Require(
			entitiesBeforeSummon == valtanRoom.m_WorldEntities.size(),
			"Reject an Esther use before the gauge is full");

		for (int tick = 0; tick < 200; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		valtanRoom.Handle_UseEstherSkill(casterSessionId, notFullUse);
		const auto findSummon = [&valtanRoom]() -> SERVER_WORLD_ENTITY*
		{
			for (SERVER_WORLD_ENTITY& entity : valtanRoom.m_WorldEntities)
			{
				if (entity.isEstherSummon)
					return &entity;
			}
			return nullptr;
		};
		tests.Require(
			nullptr == findSummon() &&
			0u == valtanRoom.m_EstherSkillSystem.Get_Gauge() &&
			1u == valtanRoom.m_PendingEstherSummons.size(),
			"Drain the Esther gauge at once but hold the summon for the landing delay");

		/* The accepted call locks the caster: ESTHER_CAST with no skill id,
		turned to the aim (east of the caster is +90 degrees). */
		SERVER_PLAYER& roomCaster = valtanRoom.m_Players.at(casterPlayerId);
		tests.Require(
			LostArk::Shared::PLAYER_ACTION_STATE::ESTHER_CAST ==
				roomCaster.eAction &&
			LostArk::Shared::INVALID_SKILL_ID == roomCaster.iCurrentSkillId &&
			0u != roomCaster.iActionStartTick &&
			std::abs(roomCaster.fYawDegrees - 90.f) < 0.01f,
			"Lock the caster into the Esther call turned to the aim");

		for (int tick = 0; tick < 200; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		valtanRoom.Handle_UseEstherSkill(casterSessionId, notFullUse);
		tests.Require(
			1000u == valtanRoom.m_EstherSkillSystem.Get_Gauge() &&
			1u == valtanRoom.m_PendingEstherSummons.size(),
			"Reject an Esther use while the caster is still casting");

		/* The room releases the cast through the player update once the call
		clip's 1500 ms have elapsed on the tick clock. */
		valtanRoom.m_iServerTick = 60u;
		valtanRoom.Update_Players(1.f / 30.f);
		tests.Require(
			LostArk::Shared::PLAYER_ACTION_STATE::NONE == roomCaster.eAction &&
			0u == roomCaster.iActionStartTick,
			"Release the caster to NONE once the call clip has run out");

		/* Drain the recharged gauge again so the emptied-gauge rejection
		below still tests the gauge and not the cast lock. */
		pRosterEntry = nullptr;
		(void)valtanRoom.m_EstherSkillSystem.Try_Consume(1u, pRosterEntry);

		/* The landing spot is two metres along the aim (east here), sampled on
		the navigation grid; an unwalkable sample falls back to the caster. */
		SERVER_NAV_POINT expectedLanding{
			caster.fPositionX, caster.fPositionY, caster.fPositionZ };
		(void)valtanRoom.m_ServerNavigation.Sample_Position(
			caster.fPositionX + ESTHER_SUMMON_FORWARD_METERS,
			caster.fPositionZ,
			expectedLanding);
		for (int tick = 0; tick < 29; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		tests.Require(
			nullptr == findSummon(),
			"Keep the Esther summon pending until the full landing delay has elapsed");
		valtanRoom.Update_WorldEntities(1.f / 30.f);
		SERVER_WORLD_ENTITY* summon = findSummon();
		tests.Require(
			nullptr != summon &&
			valtanRoom.m_PendingEstherSummons.empty() &&
			"NPC_59030" == summon->strArchetypeId &&
			5300u == summon->iEstherStrikeMs &&
			WORLD_BOOTSTRAP_KIND::NPC == summon->eKind &&
			std::string(ESTHER_ACTION_STRIKE) == summon->strActionId &&
			SERVER_ENTITY_ACTION::PATTERN_ACTIVE == summon->eAction &&
			std::abs(summon->fPositionX - expectedLanding.x) < 0.001f &&
			std::abs(summon->fPositionY - expectedLanding.y) < 0.001f &&
			std::abs(summon->fPositionZ - expectedLanding.z) < 0.001f &&
			std::abs(summon->fYawDegrees - 90.f) < 0.01f,
			"Land Sillian two metres along the aim straight into its all-in-one strike");

		valtanRoom.Handle_UseEstherSkill(casterSessionId, notFullUse);
		std::size_t summonCount = 0u;
		for (const SERVER_WORLD_ENTITY& entity : valtanRoom.m_WorldEntities)
		{
			if (entity.isEstherSummon)
				++summonCount;
		}
		tests.Require(
			1u == summonCount && valtanRoom.m_PendingEstherSummons.empty(),
			"Reject a second Esther use on the emptied gauge");

		for (int tick = 0; tick < 162; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		tests.Require(
			nullptr == findSummon(),
			"Despawn Sillian the moment its clip ends without the skyward rise");

		for (int tick = 0; tick < 200; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		LostArk::Shared::C2S_USE_ESTHER_SKILL bahunturUse{};
		bahunturUse.iClientSequence = 3u;
		bahunturUse.iSlotIndex = 3u;
		bahunturUse.fAimX = 160.f;
		bahunturUse.fAimZ = -120.f;
		valtanRoom.Handle_UseEstherSkill(casterSessionId, bahunturUse);
		for (int tick = 0; tick < 30; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		SERVER_WORLD_ENTITY* bahunturSummon = findSummon();
		tests.Require(
			nullptr != bahunturSummon &&
			"NPC_59060" == bahunturSummon->strArchetypeId &&
			4100u == bahunturSummon->iEstherStrikeMs &&
			std::string(ESTHER_ACTION_STRIKE) == bahunturSummon->strActionId &&
			SERVER_ENTITY_ACTION::PATTERN_ACTIVE == bahunturSummon->eAction,
			"Spawn Bahuntur straight into the strike with no appear stage");

		for (int tick = 0; tick < 126; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		tests.Require(
			nullptr == findSummon(),
			"Despawn Bahuntur the moment its clip ends without the skyward rise");

		/* Release the Bahuntur call before the next use; the caster would
		otherwise still hold ESTHER_CAST and reject it. */
		valtanRoom.m_iServerTick = 120u;
		valtanRoom.Update_Players(1.f / 30.f);
		for (int tick = 0; tick < 200; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		LostArk::Shared::C2S_USE_ESTHER_SKILL weiUse{};
		weiUse.iClientSequence = 2u;
		weiUse.iSlotIndex = 2u;
		weiUse.fAimX = 160.f;
		weiUse.fAimZ = -120.f;
		valtanRoom.Handle_UseEstherSkill(casterSessionId, weiUse);
		for (int tick = 0; tick < 30; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		SERVER_WORLD_ENTITY* weiSummon = findSummon();
		tests.Require(
			nullptr != weiSummon &&
			"NPC_58700" == weiSummon->strArchetypeId &&
			std::string(ESTHER_ACTION_STRIKE) == weiSummon->strActionId &&
			SERVER_ENTITY_ACTION::PATTERN_ACTIVE == weiSummon->eAction,
			"Spawn Wei straight into the strike with no appear stage");

		for (int tick = 0; tick < 216; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		tests.Require(
			nullptr == findSummon(),
			"Despawn Wei the moment its clip ends without the skyward rise");

		valtanRoom.m_Players.clear();
		valtanRoom.m_PlayerIdBySessionId.clear();
		for (int tick = 0; tick < 30; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		tests.Require(
			valtanRoom.Reset_ValtanArenaWhenEmpty() &&
			0u == valtanRoom.m_EstherSkillSystem.Get_Gauge(),
			"Re-arm the Esther gauge from zero when the arena empties");
	}
}

