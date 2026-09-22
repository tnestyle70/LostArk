#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameplayCatalog.h"
#include "KoukuSaydonBrain.h"
#include "BossCombatRuntime.h"
#include "GameRoom.h"
#include "Network/PacketReader.h"
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



int LostArk::Server::Run_ServerKoukuSupportSurfaceContractTests()
{
	using namespace LostArk::Shared;
	TESTS tests;
	namespace fs = std::filesystem;
	const auto fixture = fs::temp_directory_path() / (L"LostArkKoukuSupport-" + std::to_wstring(_getpid()));
	fs::create_directories(fixture / L"Navigation");
	{
		std::ofstream grid(fixture / L"Navigation/NAV_SUPPORT.navgrid", std::ios::binary);
		const std::uint32_t size = 4u;
		const float cell = 4.f, origin = 0.f;
		std::array<std::uint8_t, 16u> walkable; walkable.fill(1u); walkable[1] = 0u;
		std::array<float, 16u> heights; heights.fill(1.f);
		for (const auto value : { size, size }) grid.write(reinterpret_cast<const char*>(&value), sizeof(value));
		for (const auto value : { cell, origin, origin }) grid.write(reinterpret_cast<const char*>(&value), sizeof(value));
		grid.write(reinterpret_cast<const char*>(walkable.data()), walkable.size());
		grid.write(reinterpret_cast<const char*>(heights.data()), heights.size() * sizeof(float));
		std::ofstream(fixture / L"Navigation/NAV_SUPPORT.navpolicy") << R"(LOSTARK_NAVIGATION_POLICY 1 "NAV_SUPPORT" 1
)";
		std::ofstream(fixture / L"Navigation/NAV_SUPPORT.navblockers") << R"(LOSTARK_NAVGRID_BLOCKERS 1 "NAV_SUPPORT" 4 4 4 0 0 1
REGION "blocked" "closed" 0 1
2 0
)";
	}
	std::vector<wchar_t> previous(32768u);
	const DWORD previousLength = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", previous.data(), static_cast<DWORD>(previous.size()));
	SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", fixture.c_str());
	CServerNavigation nav;
	const bool loaded = nav.Load("NAV_SUPPORT");
	SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", previousLength ? previous.data() : nullptr);
	tests.Require(loaded, "Load isolated four-metre navigation with blocked and missing ground");
	std::string status;
	SERVER_NAVIGATION_SUPPORT_SURFACE floor{ "test.floor", 6.f, 6.f, 2.f, 1.6f };
	SERVER_NAV_POINT point;
	bool applied = loaded && nav.Set_RuntimeSupportSurfaces({ floor }, status);
	tests.Require(applied && nav.Sample_Position(6.f, 6.f, point) && std::abs(point.y - 1.6f) < .00001f,
		"Circular support raises the existing ground");
	tests.Require(nav.Sample_Position(8.f, 6.f, point) && std::abs(point.y - 1.6f) < .00001f &&
		nav.Sample_Position(8.01f, 6.f, point) && point.y == 1.f,
		"Exact circle edge does not expand to the four-metre cell");
	std::vector<SERVER_NAV_POINT> path;
	tests.Require(nav.Find_Path(2.f, 6.f, 10.f, 6.f, path) && path.size() >= 2u &&
		std::abs(path.front().y - 1.6f) < .00001f && nav.Has_LineOfSight(2.f, 6.f, 10.f, 6.f) &&
		nav.Resolve_TraversalStep(3.9f, 6.f, 4.1f, 6.f, point) && std::abs(point.y - 1.6f) < .00001f,
		"A-star, line of sight and live movement use the same supported height");
	{
		// A wall can truncate a root step after crossing the exact raised-floor edge.
		CServerCollisionSystem collision;
		collision.Set_BlockingBodies({ { 4.25f, 6.f, .05f } });
		BOSS_PATTERN_DEFINITION pattern; pattern.strPatternId = "KAKULSAYDON_ROOT_SUPPORT_CONTRACT";
		pattern.Stages.emplace_back(); pattern.Stages.front().iDurationMs = 1000u;
		pattern.Stages.front().Motion.RootMotion = { {0u, 0.f, 0.f, 0.f}, {1000u, 0.f, .4f, 0.f} };
		SERVER_WORLD_ENTITY boss; boss.strPatternId = pattern.strPatternId;
		boss.fPositionX = 3.9f; boss.fPositionY = 1.f; boss.fPositionZ = 6.f;
		boss.fCollisionRadius = .05f; boss.iPatternStageFirstEvaluationTick = 1u;
		boss.PatternStageRootMotion = pattern.Stages.front().Motion.RootMotion;
		const bool moved = CKoukuSaydonBrain::Apply_StageRootMotion(boss, pattern, 30u, nav, collision, status);
		tests.Require(moved && boss.fPositionX > 4.f && boss.fPositionX < 4.25f &&
			std::abs(boss.fPositionY - 1.6f) < .00001f && boss.fPositionZ == 6.f,
			"A collision-clipped root step uses ground at resolved XZ instead of sinking into the raised support");
	}
	floor.fRadiusM = .25f; floor.fHeightY = 3.f;
	nav.Set_RuntimeSupportSurfaces({ floor }, status);
	tests.Require(!nav.Resolve_TraversalStep(5.f, 6.f, 7.f, 6.f, point) &&
		!nav.Has_LineOfSight(5.f, 6.f, 7.f, 6.f) && !nav.Find_Path(5.f, 6.f, 7.f, 6.f, path),
		"A tall sub-cell disk cannot bypass step policy when both endpoints lie outside it");
	floor.fCenterX = 6.f; floor.fCenterZ = 2.f; floor.fRadiusM = 1.f;
	SERVER_NAVIGATION_SUPPORT_SURFACE blockerFloor{ "blocked.floor", 10.f, 2.f, 1.f, 1.6f };
	nav.Set_RuntimeSupportSurfaces({ floor, blockerFloor }, status);
	tests.Require(!nav.Sample_Position(6.f, 2.f, point) && !nav.Is_PointWalkableExact(6.f, 2.f) &&
		!nav.Is_PointWalkableExact(10.f, 2.f) && !nav.Has_LineOfSight(10.f, 2.f, 10.f, 2.f),
		"Supports do not open missing ground or runtime blockers");
	const auto revision = nav.Get_Revision();
	floor.fRadiusM = -1.f;
	tests.Require(!nav.Set_RuntimeSupportSurfaces({ floor }, status) && nav.Get_Revision() == revision &&
		nav.Get_RuntimeSupportSurfaceCount() == 2u, "Invalid surface replacement preserves the previous committed list");
	nav.Set_RuntimeSupportSurfaces({}, status);
#ifdef _DEBUG
	CServerGameplayContractRunner::Run_RuntimeSupportPrediction(tests, nav);
	// The same owned schedule methods used before walking also cover stationary players and cleanup.
	auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
	room->m_ServerNavigation = nav;
	// This isolated grid owns its actors; the real bootstrap may already spawn G1 Kouku.
	room->m_WorldEntities.clear();
	SERVER_PLAYER player; player.iPlayerId = 1u; player.iCurrentHp = 100u;
	player.fPositionX = 6.f; player.fPositionY = 1.f; player.fPositionZ = 6.f;
	room->m_Players[1u] = player;
	auto supportedBoss = std::make_unique<SERVER_WORLD_ENTITY>();
	supportedBoss->iNetEntityId = 700u; supportedBoss->eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	supportedBoss->strEncounterId = "ENCOUNTER_KAKULSAYDON_G1";
	supportedBoss->strArchetypeId = "BOSS_KAKULSAYDON_G1_SAYDON";
	supportedBoss->iCurrentHp = 100u; supportedBoss->fPositionX = 6.f; supportedBoss->fPositionZ = 6.f; supportedBoss->fPositionY = 1.f;
	room->m_WorldEntities.push_back(std::move(*supportedBoss));
	auto outsideBoss = std::make_unique<SERVER_WORLD_ENTITY>(room->m_WorldEntities.front());
	outsideBoss->iNetEntityId = 701u; outsideBoss->fPositionX = 15.f; outsideBoss->fPositionY = 8.63f;
	room->m_WorldEntities.push_back(std::move(*outsideBoss));
	for (const bool rootMotion : { false, true })
	{
		auto airborneBoss = std::make_unique<SERVER_WORLD_ENTITY>(room->m_WorldEntities.front());
		airborneBoss->iNetEntityId = rootMotion ? 703u : 702u; airborneBoss->fPositionY = 8.63f;
		if (rootMotion) airborneBoss->PatternStageRootMotion.push_back({ 0u, 0.f, 0.f });
		else airborneBoss->fPatternForcedMotionSpeed = 1.f;
		room->m_WorldEntities.push_back(std::move(*airborneBoss));
	}
	room->m_KoukuSaydonPatternAudition.iRoomAuditionEpoch = 1u;
	CGameRoom::KOUKU_SCHEDULED_SUPPORT_SURFACE scheduled;
	scheduled.strMemberId = "member.one"; scheduled.iStartTick = 100u; scheduled.iEndTick = 102u;
	scheduled.Surface = { "1/member.one/pattern/world", 6.f, 6.f, 2.f, 1.6f };
	room->m_KoukuSaydonPatternAudition.SupportSchedule = { scheduled };
	tests.Require(room->Refresh_KoukuSupportSurfaces(99u) && room->m_Players[1u].fPositionY == 1.f &&
		room->Refresh_KoukuSupportSurfaces(100u) && std::abs(room->m_Players[1u].fPositionY - 1.6f) < .00001f &&
		!room->m_Players[1u].hasMoveGoal, "WORLD start tick supports a stationary player without a movement command");
	tests.Require(std::abs(room->m_WorldEntities[0].fPositionY - 1.6f) < .00001f && room->m_WorldEntities[1].fPositionY == 8.63f,
		"The same WORLD surface supports a stationary Saydon and preserves a boss outside its footprint");
	tests.Require(room->m_WorldEntities[2].fPositionY == 8.63f && std::abs(room->m_WorldEntities[3].fPositionY - 9.23f) < .00001f,
		"An active WORLD floor preserves forced motion Y and the source root height above ground");
	tests.Require(room->Refresh_KoukuSupportSurfaces(102u) && room->m_Players[1u].fPositionY == 1.f &&
		room->m_ServerNavigation.Get_RuntimeSupportSurfaceCount() == 0u,
		"WORLD end tick removes the support and restores stationary ground height");
	tests.Require(room->m_WorldEntities[0].fPositionY == 1.f, "WORLD end restores the stationary boss ground height");
	scheduled.iEndTick = 200u;
	auto other = scheduled; other.strMemberId = "member.two"; other.Surface.strOwnerKey = "other.floor";
	other.Surface.fHeightY = 1.8f;
	room->m_KoukuSaydonPatternAudition.SupportSchedule = { scheduled, other };
	room->Refresh_KoukuSupportSurfaces(110u);
	room->Stop_KoukuWorldOwner("member.two");
	tests.Require(room->Refresh_KoukuSupportSurfaces(110u) && room->m_ServerNavigation.Get_RuntimeSupportSurfaceCount() == 1u &&
		std::abs(room->m_Players[1u].fPositionY - 1.6f) < .00001f, "Stopping one WORLD owner preserves another owner's support");
	room->Stop_KoukuWorldOwner();
	tests.Require(room->Refresh_KoukuSupportSurfaces(110u) && room->m_Players[1u].fPositionY == 1.f,
		"Stopping the complete run returns players to the original floor");
	tests.Require(room->m_WorldEntities[0].fPositionY == 1.f && room->m_WorldEntities[1].fPositionY == 8.63f,
		"Stop owner restores only the boss previously supported by the removed floor");
	tests.Require(room->m_WorldEntities[2].fPositionY == 8.63f && std::abs(room->m_WorldEntities[3].fPositionY - 8.63f) < .00001f,
		"Removing the WORLD floor restores the root terrain delta and preserves forced motion Y");

	for (const bool alreadyCaptured : { false, true })
	{
		auto rootRoom = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
		rootRoom->m_ServerNavigation = nav;
		rootRoom->m_WorldEntities.clear();
		auto rootBoss = std::make_unique<SERVER_WORLD_ENTITY>(room->m_WorldEntities.front());
		rootBoss->fPositionY = 1.f; rootBoss->fCollisionRadius = .05f;
		BOSS_PATTERN_DEFINITION pattern; pattern.strPatternId = "KAKULSAYDON_ROOT_FLOOR_LIFECYCLE"; pattern.bFixedTimelineClock = true;
		pattern.Stages.emplace_back(); pattern.Stages.front().iDurationMs = 1000u;
		pattern.Stages.front().Motion.RootMotion = {{0u, 0.f, 0.f, 0.f}, {1000u, 0.f, 0.f, .3f}};
		rootBoss->strPatternId = pattern.strPatternId; rootBoss->iPatternStartTick = 100u;
		rootBoss->iPatternStageFirstEvaluationTick = 100u;
		rootBoss->PatternStageRootMotion = pattern.Stages.front().Motion.RootMotion;
		rootBoss->bPatternStageRootOriginCaptured = alreadyCaptured;
		rootBoss->fPatternStageOriginX = 6.f; rootBoss->fPatternStageOriginY = 1.f; rootBoss->fPatternStageOriginZ = 6.f;
		rootBoss->fPatternStageRootGroundY = 1.f;
		rootRoom->m_WorldEntities.push_back(std::move(*rootBoss));
		auto& liveRoot = rootRoom->m_WorldEntities.front();
		auto rootFloor = scheduled; rootFloor.iStartTick = 100u; rootFloor.iEndTick = 102u;
		rootRoom->m_KoukuSaydonPatternAudition.SupportSchedule = {rootFloor};
		CServerCollisionSystem collision;
		const bool started = rootRoom->Refresh_KoukuSupportSurfaces(100u) &&
			CKoukuSaydonBrain::Apply_StageRootMotion(liveRoot, pattern, 100u, rootRoom->m_ServerNavigation, collision, status);
		tests.Require(started && std::abs(liveRoot.fPositionY - 1.6f) < .00001f &&
			std::abs(liveRoot.fPatternStageOriginY - liveRoot.fPatternStageRootGroundY) < .00001f,
			alreadyCaptured ? "A floor raises an already captured root without doubling the terrain offset" :
				"A floor raises the boss before its first root capture instead of capturing a negative ground offset");
		const bool next = rootRoom->Refresh_KoukuSupportSurfaces(100u) && rootRoom->Refresh_KoukuSupportSurfaces(101u) &&
			CKoukuSaydonBrain::Apply_StageRootMotion(liveRoot, pattern, 101u, rootRoom->m_ServerNavigation, collision, status);
		tests.Require(next && std::abs(liveRoot.fPositionY - 1.61f) < .00001f,
			"Repeated support refresh applies no extra lift and preserves the native vertical sample");
		const bool removed = rootRoom->Refresh_KoukuSupportSurfaces(102u) &&
			CKoukuSaydonBrain::Apply_StageRootMotion(liveRoot, pattern, 102u, rootRoom->m_ServerNavigation, collision, status);
		tests.Require(removed && std::abs(liveRoot.fPositionY - 1.02f) < .00001f,
			"Support removal returns a running root to arena ground plus its source height, never below the arena");
	}

	for (const bool skippedTicks : { false, true })
	{
		// Production Whirlwind is a 10 m / 3204 ms Logic charge, not source root motion.
		BOSS_PATTERN_DEFINITION chargePattern; chargePattern.strPatternId = "KAKULSAYDON_G1_PATTERN_24";
		BOSS_PATTERN_LOGIC_WINDOW charge; charge.strWindowId = "KAKULSAYDON_G1_PATTERN_24.logic.1";
		charge.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA; charge.iDurationMs = 3204u; charge.fBossChargeDistanceM = 10.f;
		chargePattern.LogicWindows = {charge};
		auto chargeBoss = std::make_unique<SERVER_WORLD_ENTITY>(room->m_WorldEntities.front());
		chargeBoss->strPatternId = chargePattern.strPatternId; chargeBoss->iPatternSequence = 1u;
		chargeBoss->fPositionX = 2.f; chargeBoss->fPositionY = 1.f; chargeBoss->fPositionZ = 2.f;
		chargeBoss->fCollisionRadius = .05f;
		std::map<PLAYER_ID, SERVER_PLAYER> targets;
		auto& target = targets[1u]; target.iPlayerId = 1u; target.iNetEntityId = 100u;
		target.iCurrentHp = target.iMaximumHp = 100u; target.isCombatReady = true;
		target.fPositionX = 14.f; target.fPositionY = 1.f; target.fPositionZ = 2.f;
		KOUKUSAYDON_LOGIC_LEDGER chargeLedger; KOUKUSAYDON_LOGIC_OUTPUT chargeOutput;
		std::vector<DAMAGE_EVENT> damage; CServerCollisionSystem collision;
		CKoukuSaydonLogicRuntime::Build(chargePattern, *chargeBoss, 100u, chargeLedger);
		bool everyStepNavigable = true;
		for (std::uint32_t tick = 100u; tick <= 197u; ++tick)
		{
			if (skippedTicks && tick > 100u && tick < 197u) continue;
			const float previousX = chargeBoss->fPositionX, previousZ = chargeBoss->fPositionZ;
			CKoukuSaydonLogicRuntime::Update(*chargeBoss, chargePattern, chargeLedger, targets,
				room->m_GameplayCatalog.Active(), nullptr, tick, damage, chargeOutput, &nav, &collision);
			everyStepNavigable = everyStepNavigable && nav.Has_LineOfSight(previousX, previousZ,
				chargeBoss->fPositionX, chargeBoss->fPositionZ);
		}
		tests.Require(everyStepNavigable && chargeBoss->fPositionX > 3.998f && chargeBoss->fPositionX < 4.f &&
			chargeBoss->fPositionZ == 2.f && chargeBoss->fPositionY == 1.f && chargeLedger.Windows.front().bChargeStopped,
			skippedTicks ? "Whirlwind cannot tunnel across blocked intermediate cells even when both endpoints are walkable" :
				"Whirlwind stops within one millimetre of navigation and every committed fixed-tick segment stays navigable");
		const float stoppedX = chargeBoss->fPositionX;
		CKoukuSaydonLogicRuntime::Update(*chargeBoss, chargePattern, chargeLedger, targets,
			room->m_GameplayCatalog.Active(), nullptr, 200u, damage, chargeOutput, &nav, &collision);
		tests.Require(chargeBoss->fPositionX == stoppedX, "Stopped Whirlwind cannot resume beyond navigation on later ticks");
	}

	for (const bool withFollowup : { false, true })
	{
		// Counter success closes the WORLD owner with or without a followup outcome.
		auto& counterBoss = room->m_WorldEntities.front();
		const auto counterBossBefore = std::make_unique<SERVER_WORLD_ENTITY>(counterBoss);
		const auto counterTickBefore = room->m_iServerTick;
		const auto counterStatusBefore = room->m_strStatus;
		counterBoss.strPatternId = "KAKULSAYDON_COUNTER_LANDING"; counterBoss.strActionId = "counter.action";
		counterBoss.iPatternSequence = 91u; counterBoss.fPositionY = 3.9f; counterBoss.fCollisionRadius = .5f;
		counterBoss.fSpawnPositionX = counterBoss.fPositionX; counterBoss.fSpawnPositionZ = counterBoss.fPositionZ;
		counterBoss.fSpawnPositionY = 1.25f;
		counterBoss.PatternStageRootMotion = {{0u, 0.f, 0.f, 0.f}, {3000u, 0.f, 0.f, 2.6f}};
		counterBoss.bPatternStageRootOriginCaptured = true;
		CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_MEMBER counterMember;
		counterMember.strMemberId = "counter.member"; counterMember.iBossEntityId = counterBoss.iNetEntityId;
		counterMember.PatternIds = {counterBoss.strPatternId}; counterMember.iPatternSequence = counterBoss.iPatternSequence;
		counterMember.WorldCueByInstance["rolling.ball"] = "ball.cue";
		counterMember.WorldCueByOccurrence["counter.world.1"] = "ball.cue";
		room->m_KoukuSaydonPatternAudition.Members = {counterMember};
		S2C_WORLD_SEQUENCE_PLAY ball; ball.iRunEpoch = 1u; ball.strMemberId = counterMember.strMemberId;
		ball.strCueId = "ball.cue"; ball.strOccurrenceId = "counter.world.1"; ball.strSequenceInstanceId = "rolling.ball";
		ball.iBossNetEntityId = counterBoss.iNetEntityId; ball.iPatternSequence = counterBoss.iPatternSequence;
		auto otherBall = ball; otherBall.strMemberId = "other.member"; otherBall.strCueId = "other.cue";
		room->m_KoukuSaydonPatternAudition.WorldPlays = {ball, otherBall};
		room->m_PendingKoukuMechanicTriggers = {{counterBoss.iNetEntityId, counterBoss.iPatternSequence, {}}, {701u, 1u, {}}};
		BOSS_PATTERN_DEFINITION counterPattern; counterPattern.strPatternId = counterBoss.strPatternId;
		BOSS_PATTERN_LOGIC_WINDOW counterWindow; counterWindow.strWindowId = "counter.window";
		counterWindow.eKind = BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW; counterWindow.iDurationMs = 2837u;
		counterWindow.bEndsPatternOnSuccess = true;
		if (withFollowup)
			counterWindow.OnSuccess.push_back({BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN, 0u, 0u, "KAKULSAYDON_G1_PATTERN_4"});
		counterPattern.LogicWindows = {counterWindow};
		auto& counterLedger = room->m_KoukuSaydonPatternAudition.Members.front().LogicLedger;
		KOUKUSAYDON_LOGIC_OUTPUT counterOutput; std::vector<DAMAGE_EVENT> counterDamage;
		CKoukuSaydonLogicRuntime::Build(counterPattern, counterBoss, 120u, counterLedger);
		CKoukuSaydonLogicRuntime::Update(counterBoss, counterPattern, counterLedger, room->m_Players,
			room->m_GameplayCatalog, nullptr, 120u, counterDamage, counterOutput, &room->m_ServerNavigation, &room->m_ServerCollisionSystem);
		const bool counterHit = CBossCombatRuntime::Try_TriggerCounter(counterBoss, 121u);
		CKoukuSaydonLogicRuntime::Update(counterBoss, counterPattern, counterLedger, room->m_Players,
			room->m_GameplayCatalog, nullptr, 121u, counterDamage, counterOutput, &room->m_ServerNavigation, &room->m_ServerCollisionSystem);
		room->m_iServerTick = 121u;
		const bool completed = counterHit && room->Apply_KoukuLogicOutput(counterOutput, counterBoss, 121u);
		const auto& committed = room->m_KoukuSaydonPatternAudition.Members.front();
		tests.Require(completed && counterOutput.bCounterSuccessLanded && std::abs(counterBoss.fPositionY - 1.25f) < .0001f &&
			counterBoss.strPatternId.empty() && counterBoss.PatternStageRootMotion.empty() && !counterBoss.bPatternStageRootOriginCaptured &&
			(withFollowup ? (committed.PatternIds.size() == 2u && committed.PatternIds.back() == "KAKULSAYDON_G1_PATTERN_4" &&
				committed.TransitionTicks == std::vector<std::uint32_t>{1u}) :
				(committed.PatternIds.size() == 1u && committed.TransitionTicks.empty())),
			withFollowup ? "Actual counter verdict lands, clears source root motion and schedules existing groggy one tick later" :
				"A counter with no outcomes lands and ends the Pattern without inventing a followup");
		tests.Require(room->m_KoukuSaydonPatternAudition.WorldPlays.size() == 1u &&
			room->m_KoukuSaydonPatternAudition.WorldPlays.front().strMemberId == "other.member" &&
			committed.WorldCueByInstance.empty() && committed.WorldCueByOccurrence.empty() &&
			room->m_PendingKoukuMechanicTriggers.size() == 1u && room->m_PendingKoukuMechanicTriggers.front().iBossEntityId == 701u,
			"Counter interruption stops only its active ball WORLD owner and pending mechanics, preserving another member");
		room->m_KoukuSaydonPatternAudition.Members.clear(); room->m_KoukuSaydonPatternAudition.WorldPlays.clear();
		room->m_PendingKoukuMechanicTriggers.clear();
		counterBoss = *counterBossBefore; room->m_iServerTick = counterTickBefore; room->m_strStatus = counterStatusBefore;
	}

	// Albion uses this same ground authority and the existing combat-object transaction.
	room->m_CombatObjectRuntime.Reset();
	auto& albionOwner = room->m_WorldEntities.front();
	albionOwner.strPatternId = "KAKULSAYDON_TEST_ALBION";
	albionOwner.iPatternSequence = 1u;
	albionOwner.PinnedDefinitionRevision = room->m_GameplayCatalog.Get_ActiveRevision();
	room->m_Players.clear();
	for (PLAYER_ID id = 1u; id <= 3u; ++id)
	{
		auto& target = room->m_Players[id];
		target.iPlayerId = id; target.iNetEntityId = 100u + id;
		target.iCurrentHp = id == 3u ? 0u : 100u; target.iMaximumHp = 100u;
		target.isCombatReady = true; target.fPositionX = id == 2u ? 10.f : 6.f;
		target.fPositionY = 99.f; target.fPositionZ = 6.f;
	}
	BOSS_PATTERN_MECHANIC_TRIGGER albion{};
	albion.strTriggerId = "test.albion.logic.1";
	albion.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_BLUE_CIRCLE;
	albion.iCountPerPlayer = 1u; albion.iEffectLifetimeMs = 7000u;
	const auto queueAlbion = [&](const std::uint32_t tick)
	{
		room->m_PendingKoukuMechanicTriggers.push_back({ albionOwner.iNetEntityId, albionOwner.iPatternSequence, albion });
		room->Commit_KoukuMechanicTriggers(tick);
	};
	queueAlbion(1000u);
	const auto& initialObjects = room->m_CombatObjectRuntime.Get_LiveObjects();
	tests.Require(initialObjects.size() == 2u && std::all_of(initialObjects.begin(), initialObjects.end(), [&](const auto& object)
		{ return object.Hits.empty() && object.PresentationPulses.size() == 1u && !object.bTrackLockedTargetUntilFirstPulse &&
			object.LiveState.CurrentPose.fPositionY == 1.f && object.PinnedDefinitionRevision == albionOwner.PinnedDefinitionRevision; }),
		"Albion stages one grounded presentation per living player with no damage or fabricated boss");
	std::vector<S2C_COMBAT_OBJECT_SPAWNED> restored;
	room->m_CombatObjectRuntime.Build_LiveSpawnMessages(1030u, restored);
	std::vector<COMBAT_OBJECT_SNAPSHOT> snapshots;
	bool wireValid = restored.size() == 2u && room->m_CombatObjectRuntime.Build_Snapshots(snapshots) && snapshots.size() == 2u;
	for (const auto& spawn : restored)
	{
		CPacketWriter writer; S2C_COMBAT_OBJECT_SPAWNED roundTrip;
		wireValid = Write_Message(writer, spawn) && wireValid;
		CPacketReader reader{ writer.Get_Buffer() };
		wireValid = Read_Message(reader, roundTrip) && wireValid && roundTrip.iSpawnTick == 1000u && roundTrip.iServerTick == 1030u &&
			roundTrip.PinnedDefinitionRevision == albionOwner.PinnedDefinitionRevision && roundTrip.fPositionY == 1.f;
	}
	tests.Require(wireValid, "Albion reconnect and snapshot retain the original spawn clock and pinned revision without catalog object lookup");
	room->m_CombatObjectRuntime.Discard_PendingLifecycle();
	const auto nextBeforeFailure = room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId;
	room->m_Players[2u].fPositionX = -1000.f;
	queueAlbion(1001u);
	tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == 2u &&
		room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId == nextBeforeFailure && room->m_PendingKoukuMechanicTriggers.empty(),
		"Invalid Albion point rolls back the entire volley after valid players stage, preserving objects and IDs");
	room->m_CombatObjectRuntime.Reset();
	room->m_Players[2u].isCombatReady = false;
	albion.iCountPerPlayer = 4u; albion.fPlayerEffectRadiusM = 2.f;
	queueAlbion(1100u);
	const auto& radialObjects = room->m_CombatObjectRuntime.Get_LiveObjects();
	tests.Require(radialObjects.size() == 4u && std::all_of(radialObjects.begin(), radialObjects.end(), [](const auto& object)
		{ const auto& pose = object.LiveState.CurrentPose; const float x = pose.fPositionX - 6.f, z = pose.fPositionZ - 6.f;
		  return std::abs(x*x + z*z - 4.f) < .001f && pose.fPositionY == 1.f; }),
		"Albion radial authoring reuses the volley layout and excludes non-ready and dead players");
	std::vector<DAMAGE_EVENT> albionDamage;
	room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 2.f, 1160u, albionDamage);
	std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
	std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> pulses;
	std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
	room->m_CombatObjectRuntime.Drain_Lifecycle(spawned, pulses, despawned);
	tests.Require(albionDamage.empty() && pulses.size() == 4u && std::all_of(pulses.begin(), pulses.end(), [](const auto& pulse)
		{ return pulse.eKind == COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE && pulse.strHitId == "combatpresentation.kouku.albion.started"; }) &&
		room->m_CombatObjectRuntime.Get_LiveObjects().size() == 4u,
		"Albion emits only its admitted lifecycle marker and keeps the authored explosion tail alive");
	room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 5.f, 1310u, albionDamage);
	tests.Require(albionDamage.empty() && room->m_CombatObjectRuntime.Get_LiveObjects().empty(),
		"Albion expires at the authored effect lifetime without damage");

	for (const bool completed : { true, false })
	{
		room->m_CombatObjectRuntime.Reset();
		albionOwner.strPatternId = "KAKULSAYDON_TEST_ALBION";
		albionOwner.iPatternSequence = 2u;
		queueAlbion(1400u);
		room->m_KoukuSaydonPatternAudition.Members.clear();
		auto& member = room->m_KoukuSaydonPatternAudition.Members.emplace_back();
		member.strMemberId = "test.albion.owner"; member.iBossEntityId = albionOwner.iNetEntityId;
		member.iPatternSequence = albionOwner.iPatternSequence; member.PatternIds = { albionOwner.strPatternId };
		room->Clear_KoukuSaydonPatternAudition(completed);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == (completed ? 4u : 0u),
			completed ? "Natural Kouku completion retains the Albion visual tail" : "Explicit Kouku stop cancels the owned Albion objects");
	}

	// A blue-circle wave freezes one selected player and five independent nav points.
	room->m_CombatObjectRuntime.Reset();
	albionOwner.strPatternId = "KAKULSAYDON_TEST_ALBION";
	albionOwner.iPatternSequence = 3u;
	albionOwner.fSpawnPositionX = 8.f; albionOwner.fSpawnPositionY = 1.f; albionOwner.fSpawnPositionZ = 8.f;
	room->m_ServerNavigation.Set_RuntimeSupportSurfaces({ { "albion.floor", 8.f, 8.f, 30.f, 1.6f } }, status);
	room->m_Players.clear();
	for (PLAYER_ID id = 1u; id <= 7u; ++id)
	{
		auto& target = room->m_Players[id];
		target.iPlayerId = id; target.iNetEntityId = 100u + id;
		target.iCurrentHp = id == 3u ? 0u : 100u; target.iMaximumHp = 100u;
		target.isCombatReady = id != 4u;
		target.eAction = id == 5u ? PLAYER_ACTION_STATE::DEAD :
			id == 6u ? PLAYER_ACTION_STATE::FALLING : PLAYER_ACTION_STATE::NONE;
		target.iMarioStage = id == 7u ? 1u : 0u;
		target.fPositionX = id == 1u ? 6.f : 10.f; target.fPositionY = 99.f; target.fPositionZ = 6.f;
	}
	albion.iCountPerPlayer = 1u; albion.fPlayerEffectRadiusM = 0.f;
	albion.bRandomPlayerOnly = true; albion.iArenaRandomCount = 5u;
	albion.fArenaRandomRadiusM = 6.f; albion.fArenaHeightToleranceM = 1.f; albion.fArenaMinimumSpacingM = 3.2f;
	queueAlbion(1600u);
	const auto readPoses = [&]()
	{
		std::vector<std::array<float, 3u>> result;
		for (const auto& object : room->m_CombatObjectRuntime.Get_LiveObjects())
		{
			const auto& pose = object.LiveState.CurrentPose;
			result.push_back({ pose.fPositionX, pose.fPositionY, pose.fPositionZ });
		}
		return result;
	};
	const auto firstPoses = readPoses();
	const auto& sixObjects = room->m_CombatObjectRuntime.Get_LiveObjects();
	std::size_t targetedCount = 0u, arenaCount = 0u;
	bool admittedSix = sixObjects.size() == 6u;
	for (const auto& object : sixObjects)
	{
		const auto& pose = object.LiveState.CurrentPose;
		SERVER_NAV_POINT sample{};
		admittedSix = admittedSix && object.Hits.empty() && !object.bTrackLockedTargetUntilFirstPulse &&
			room->m_ServerNavigation.Is_PointWalkableExact(pose.fPositionX, pose.fPositionZ) &&
			room->m_ServerNavigation.Sample_Position(pose.fPositionX, pose.fPositionZ, sample) && pose.fPositionY == sample.y;
		if (object.iLockedTargetNetEntityId != INVALID_NET_ENTITY_ID)
		{
			++targetedCount;
			admittedSix = admittedSix && (object.iLockedTargetNetEntityId == 101u || object.iLockedTargetNetEntityId == 102u) &&
				pose.fPositionZ == 6.f && pose.fPositionX == (object.iLockedTargetNetEntityId == 101u ? 6.f : 10.f);
		}
		else
		{
			++arenaCount;
			const float x = pose.fPositionX - 8.f, z = pose.fPositionZ - 8.f;
			admittedSix = admittedSix && x*x + z*z <= 36.001f;
			for (const auto& otherObject : sixObjects)
			{
				if (otherObject.iCombatObjectId >= object.iCombatObjectId || otherObject.iLockedTargetNetEntityId != INVALID_NET_ENTITY_ID) continue;
				const float dx = pose.fPositionX - otherObject.LiveState.CurrentPose.fPositionX;
				const float dz = pose.fPositionZ - otherObject.LiveState.CurrentPose.fPositionZ;
				admittedSix = admittedSix && dx*dx + dz*dz + .001f >= 3.2f * 3.2f;
			}
		}
	}
	tests.Require(admittedSix && targetedCount == 1u && arenaCount == 5u,
		"Albion admits exactly five spaced nav points and one living ready non-Mario player on exact supported ground");
	room->m_CombatObjectRuntime.Build_LiveSpawnMessages(1601u, restored);
	tests.Require(restored.size() == 6u && std::all_of(restored.begin(), restored.end(), [&](const auto& spawn)
		{ return spawn.fPositionY == 1.6f && spawn.iSpawnTick == 1600u && spawn.PinnedDefinitionRevision == albionOwner.PinnedDefinitionRevision; }),
		"The six Albion reconnect spawns preserve their sampled floor, effect clock and pinned revision");
	room->m_Players[1u].fPositionX += 1.f; room->m_Players[2u].fPositionX += 1.f;
	room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, .1f, 1603u, albionDamage);
	tests.Require(readPoses() == firstPoses && albionDamage.empty(),
		"Moving either eligible player after spawn leaves all six Albion points fixed and deals no damage");
	room->m_Players[1u].fPositionX -= 1.f; room->m_Players[2u].fPositionX -= 1.f;
	room->m_CombatObjectRuntime.Reset();
	queueAlbion(1600u);
	tests.Require(readPoses() == firstPoses, "The same Albion trigger and pattern occurrence resolve a deterministic six-point volley");
	std::set<NET_ENTITY_ID> selectedTargets;
	bool wavesChanged = false;
	for (std::uint32_t wave = 1u; wave <= 12u; ++wave)
	{
		room->m_CombatObjectRuntime.Reset();
		albion.strTriggerId = "test.albion.logic.wave." + std::to_string(wave);
		queueAlbion(1700u);
		wavesChanged = wavesChanged || readPoses() != firstPoses;
		for (const auto& object : room->m_CombatObjectRuntime.Get_LiveObjects())
			if (object.iLockedTargetNetEntityId != INVALID_NET_ENTITY_ID) selectedTargets.insert(object.iLockedTargetNetEntityId);
	}
	tests.Require(wavesChanged && selectedTargets == std::set<NET_ENTITY_ID>{ 101u, 102u },
		"Distinct authored Albion triggers vary the wave and can choose either eligible player");
	const auto beforeRejectedWave = readPoses();
	const auto beforeRejectedId = room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId;
	room->m_Players[1u].fPositionX = room->m_Players[2u].fPositionX = -1000.f;
	queueAlbion(1800u);
	tests.Require(readPoses() == beforeRejectedWave && room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId == beforeRejectedId,
		"A selected player outside navigation rejects all six newly staged Albion objects and preserves live IDs");
	room->m_Players[1u].fPositionX = 6.f; room->m_Players[2u].fPositionX = 10.f;
	albion.fArenaRandomRadiusM = 1.f; albion.fArenaMinimumSpacingM = 20.f;
	queueAlbion(1801u);
	tests.Require(readPoses() == beforeRejectedWave && room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId == beforeRejectedId,
		"An impossible five-point spacing request rejects the whole Albion wave and preserves live IDs");
	albion.fArenaRandomRadiusM = 6.f; albion.fArenaMinimumSpacingM = 3.2f;
	room->m_Players[1u].isCombatReady = room->m_Players[2u].isCombatReady = false;
	queueAlbion(1802u);
	tests.Require(readPoses() == beforeRejectedWave && room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId == beforeRejectedId,
		"No eligible arena player preserves the entire previous Albion wave instead of spawning only its random supplement");

	// Pursuit shares the actual room clock and combat-object lifecycle; it owns no damage.
	{
		room->m_CombatObjectRuntime.Reset(); room->m_CombatObjectRuntime.Discard_PendingLifecycle();
		room->m_Players.clear();
		auto& target = room->m_Players[1u];
		target.iPlayerId = 1u; target.iNetEntityId = 101u; target.iCurrentHp = target.iMaximumHp = 100u;
		target.isCombatReady = true; target.fPositionX = 10000.f; target.fPositionZ = 10000.f;
		albionOwner.iCurrentHp = 100u; albionOwner.eAction = SERVER_ENTITY_ACTION::IDLE;
		albionOwner.strPatternId = "KAKULSAYDON_TEST_PURSUIT"; albionOwner.iPatternSequence = 70u;
		albionOwner.iPatternTargetEntityId = target.iNetEntityId;
		albionOwner.fPositionX = albionOwner.fPositionZ = 0.f;
		BOSS_PATTERN_MECHANIC_TRIGGER pursuit;
		pursuit.strTriggerId = "test.pursuit"; pursuit.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES;
		pursuit.iStartMs = 100u; pursuit.iDurationMs = 901u;
		pursuit.ProjectileVisualIds = { "test.heart", "test.clover", "test.diamond", "test.spade" };
		pursuit.strContactVisualId = "test.card.explosion";
		pursuit.fProjectileSpeedMps = 1.f; pursuit.fProjectileContactRadiusM = .4f;
		pursuit.fProjectileSpawnRadiusM = 2.f; pursuit.iProjectileCountPerWave = 4u;
		pursuit.bProjectileHoming = true;
		BOSS_PATTERN_DEFINITION pattern; pattern.strPatternId = albionOwner.strPatternId; pattern.MechanicTriggers = { pursuit };
		KOUKUSAYDON_LOGIC_LEDGER ledger;
		CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6000u, ledger);
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6002u);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "Pursuit respects the authored spawn start");
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6003u);
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6004u);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == 4u, "One infinite homing wave creates four distinct visual objects once");
		std::vector<DAMAGE_EVENT> damage;
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 601.f, 6005u, damage);
		const auto firstPose = room->m_CombatObjectRuntime.Get_LiveObjects().front().LiveState.CurrentPose;
		target.fPositionX = -10000.f;
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f, 6006u, damage);
		const auto& moving = room->m_CombatObjectRuntime.Get_LiveObjects();
		tests.Require(moving.size() == 4u && moving.front().LiveState.CurrentPose.fPositionX < firstPose.fPositionX && damage.empty(),
			"Infinite cards survive the old ten-minute lifetime and turn toward the current authoritative target without damage");
		room->Clear_KoukuPlayerTargets(albionOwner, ledger);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == 4u, "Natural pattern completion leaves persistent cards alive");
		target.iCurrentHp = 0u;
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f / 30.f, 6007u, damage);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "Target death retires every homing card");
		target.iCurrentHp = 100u;
		room->m_CombatObjectRuntime.Discard_PendingLifecycle();
		pursuit.iStartMs = 0u; pursuit.iProjectileLifetimeMs = 5000u; pursuit.iSpawnIntervalMs = 300u;
		pursuit.iProjectileCountPerWave = 3u; pursuit.bProjectileHoming = false; pursuit.fProjectileSpeedMps = 15.f;
		pattern.MechanicTriggers = { pursuit }; CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6100u, ledger);
		for (std::uint32_t tick = 6100u; tick <= 6128u; ++tick)
			room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, tick);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == 12u, "Source 300 ms cadence emits three cards in each of four waves");
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 5.1f, 6129u, damage);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "Finite spinning cards retire at their authored five-second lifetime");
		std::vector<S2C_COMBAT_OBJECT_SPAWNED> expiredSpawns;
		std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> expiredEvents;
		std::vector<S2C_COMBAT_OBJECT_DESPAWNED> expiredObjects;
		room->m_CombatObjectRuntime.Drain_Lifecycle(expiredSpawns, expiredEvents, expiredObjects);
		// The same drain includes the admitted birth pulses, not just terminal bursts.
		tests.Require(expiredEvents.size() == 24u && expiredObjects.size() == 12u &&
			std::count_if(expiredEvents.begin(), expiredEvents.end(), [](const auto& event) {
				return event.strHitId == "combatpresentation.kouku.pursuit.started"; }) == 12 &&
			std::count_if(expiredEvents.begin(), expiredEvents.end(), [&](const auto& event) {
				return event.strHitId == pursuit.strContactVisualId; }) == 12,
			"Every finite card emits one birth pulse and its shared burst exactly once at lifetime end");
		room->m_CombatObjectRuntime.Discard_PendingLifecycle();
		// Source Speed is 8 m/s; MaxDistance is independent of its five-second lifetime.
		pursuit.iDurationMs = 4100u; pursuit.fProjectileSpeedMps = 8.f; pursuit.fProjectileMaxDistanceM = 15.f;
		pattern.MechanicTriggers = { pursuit }; CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6150u, ledger);
		for (std::uint32_t tick = 6150u; tick <= 6273u; ++tick)
			room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, tick);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == 42u,
			"Current 4.8-second spin extends source cadence to fourteen three-card waves, with no wave past the window");
		const auto distanceOrigin = room->m_CombatObjectRuntime.Get_LiveObjects().front().LiveState.CurrentPose;
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f, 6274u, damage);
		const auto& travelling = room->m_CombatObjectRuntime.Get_LiveObjects();
		tests.Require(travelling.size() == 42u && std::abs(travelling.front().fRemainingDistanceM - 7.f) < .0001f &&
			std::abs(std::hypot(travelling.front().LiveState.CurrentPose.fPositionX - distanceOrigin.fPositionX,
				travelling.front().LiveState.CurrentPose.fPositionZ - distanceOrigin.fPositionZ) - 8.f) < .0001f,
			"Spinning cards move eight metres in one second and retain the remaining seven metres");
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f, 6275u, damage);
		expiredSpawns.clear(); expiredEvents.clear(); expiredObjects.clear();
		room->m_CombatObjectRuntime.Drain_Lifecycle(expiredSpawns, expiredEvents, expiredObjects);
		std::erase_if(expiredEvents, [&](const auto& event) { return event.strHitId != pursuit.strContactVisualId; });
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty() && expiredEvents.size() == 42u && expiredObjects.size() == 42u &&
			std::abs(std::hypot(expiredEvents.front().fPositionX - distanceOrigin.fPositionX,
				expiredEvents.front().fPositionZ - distanceOrigin.fPositionZ) - 15.f) < .0001f,
			"Distance-limited cards stop at fifteen metres and burst exactly once before the five-second lifetime");
		room->m_CombatObjectRuntime.Discard_PendingLifecycle();
		pursuit.fProjectileMaxDistanceM = 0.f; pursuit.fProjectileSpeedMps = 15.f;
		pursuit.iSpawnIntervalMs = 0u; pursuit.iProjectileCountPerWave = 1u;
		pattern.MechanicTriggers = { pursuit }; CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6200u, ledger);
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6200u);
		const auto contactPose = room->m_CombatObjectRuntime.Get_LiveObjects().front().LiveState.CurrentPose;
		target.fPositionX = contactPose.fPositionX + contactPose.fDirectionX * 7.5f;
		target.fPositionZ = contactPose.fPositionZ + contactPose.fDirectionZ * 7.5f;
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f, 6201u, damage);
		std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned; std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> events;
		std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
		room->m_CombatObjectRuntime.Drain_Lifecycle(spawned, events, despawned);
		tests.Require(events.size() == 2u && std::count_if(events.begin(), events.end(), [](const auto& event) {
			return event.strHitId == "combatpresentation.kouku.pursuit.started"; }) == 1,
			"Swept card contact preserves its single birth pulse alongside the terminal burst");
		std::erase_if(events, [](const auto& event) { return event.strHitId == "combatpresentation.kouku.pursuit.started"; });
		tests.Require(events.size() == 1u && events.front().strHitId == pursuit.strContactVisualId && despawned.size() == 1u &&
			events.front().fPositionX == target.fPositionX && events.front().fPositionZ == target.fPositionZ &&
			room->m_CombatObjectRuntime.Get_LiveObjects().empty() && damage.empty(), "Swept card contact emits one pinned reliable explosion and retires the object");
		// The new P78 PLAY rows are four independent, finite 4.5-second homing windows.
		room->m_CombatObjectRuntime.Discard_PendingLifecycle();
		target.fPositionX = target.fPositionZ = 10000.f;
		pattern.MechanicTriggers.clear();
		for (std::uint32_t suit = 0u; suit < 4u; ++suit)
		{
			auto card = pursuit; card.strTriggerId = "p78.card." + std::to_string(suit);
			card.iStartMs = 0u; card.iDurationMs = card.iProjectileLifetimeMs = 4500u;
			card.ProjectileVisualIds = {pursuit.ProjectileVisualIds[suit]};
			card.iProjectileCountPerWave = 1u; card.bProjectileHoming = true;
			card.fProjectileContactRadiusM = 1.25f; card.fProjectileSpeedMps = 1.f;
			pattern.MechanicTriggers.push_back(card);
		}
		CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6250u, ledger);
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6250u);
		const auto& finiteCards = room->m_CombatObjectRuntime.Get_LiveObjects();
		tests.Require(finiteCards.size() == 4u && std::all_of(finiteCards.begin(), finiteCards.end(), [](const auto& card) {
			return card.bHoming && !card.bPersistentLifetime && card.fRemainingMilliseconds == 4500.f &&
				card.fContactPresentationRadiusM == 1.25f && card.Hits.empty(); }),
			"Four P78 windows each create one finite 4500ms homing card with 1.25m radius and no damage");
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 4.499f, 6384u, damage);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == 4u,
			"P78 cards remain alive immediately before their saved 4.5-second lifetime");
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, .002f, 6385u, damage);
		spawned.clear(); events.clear(); despawned.clear();
		room->m_CombatObjectRuntime.Drain_Lifecycle(spawned, events, despawned);
		std::erase_if(events, [&](const auto& event) { return event.strHitId != pursuit.strContactVisualId; });
		tests.Require(events.size() == 4u && despawned.size() == 4u && room->m_CombatObjectRuntime.Get_LiveObjects().empty(),
			"Each finite P78 card expires with one reliable burst and one despawn at its saved lifetime");
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f, 6386u, damage);
		spawned.clear(); events.clear(); despawned.clear();
		room->m_CombatObjectRuntime.Drain_Lifecycle(spawned, events, despawned);
		tests.Require(events.empty() && despawned.empty(), "Retired finite cards cannot emit duplicate terminal events");
		pattern.MechanicTriggers = {pursuit};
		pattern.MechanicTriggers.front().bProjectileHoming = true;
		CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6300u, ledger);
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6300u);
		room->m_CombatObjectRuntime.Cancel_Source(albionOwner.iNetEntityId);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "Explicit Stop uses source cancellation for surviving cards");
		room->m_CombatObjectRuntime.Reset(); room->m_CombatObjectRuntime.Discard_PendingLifecycle();
		pursuit.iStartMs = 913u; pursuit.iDurationMs = 1u; pursuit.iProjectileLifetimeMs = 0u;
		pursuit.iSpawnIntervalMs = 0u; pursuit.iProjectileCountPerWave = 1u; pursuit.bProjectileHoming = true;
		pursuit.fProjectileMaxDistanceM = 0.f; pursuit.fProjectileSpeedMps = 1.f;
		albionOwner.fYawDegrees = 0.f; albionOwner.fPositionX = albionOwner.fPositionZ = 0.f;
		target.fPositionX = target.fPositionZ = 10000.f; target.iCurrentHp = 100u;
		LostArk::Shared::ATTACK_HIT_TEMPLATE fullHit;
		fullHit.strHitId = "card.full-lifetime"; fullHit.strTrigger = "CONTACT";
		fullHit.iEndMs = 600000u; fullHit.fRadiusM = .4; fullHit.iDamagePercent = 10u;
		auto shortHit = fullHit; shortHit.strHitId = "card.short-window"; shortHit.iEndMs = 1000u;
		pursuit.ProjectileHits = { fullHit, shortHit };
		pattern.MechanicTriggers = { pursuit }; CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6400u, ledger);
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6427u);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "Sub-tick pursuit Trigger never emits before its authored start");
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6428u);
		const auto& persistentCards = room->m_CombatObjectRuntime.Get_LiveObjects();
		tests.Require(persistentCards.size() == 1u, "One-ms Trigger produces one card on its quantized birth tick");
		if (!persistentCards.empty())
		{
			const auto& card = persistentCards.front();
			tests.Require(std::abs(card.LiveState.CurrentPose.fPositionX - pursuit.fProjectileSpawnRadiusM) < .001f &&
				std::abs(card.LiveState.CurrentPose.fPositionZ) < .001f, "Pursuit spawn is Saydon +X front, 180 degrees opposite the old rear origin");
			tests.Require(card.Hits.size() == 2u && card.Hits[0].iEndMs == 0u && card.Hits[1].iEndMs == 1000u,
				"Persistent pursuit extends only full-lifetime CONTACT hits and preserves shorter authored windows");
			room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6429u);
			damage.clear();
			room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 601.f, 6430u, damage);
			if (!room->m_CombatObjectRuntime.Get_LiveObjects().empty())
			{
				const auto pose = room->m_CombatObjectRuntime.Get_LiveObjects().front().LiveState.CurrentPose;
				target.fPositionX = pose.fPositionX; target.fPositionY = pose.fPositionY; target.fPositionZ = pose.fPositionZ;
				room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f / 30.f, 6431u, damage);
			}
			tests.Require(damage.size() == 1u && room->m_CombatObjectRuntime.Get_LiveObjects().empty(),
				"Persistent card still damages and despawns on contact after ten minutes");
		}
		room->m_CombatObjectRuntime.Reset(); room->m_CombatObjectRuntime.Discard_PendingLifecycle(); room->m_Players.clear();
	}
	// Showtime duration uses the same room-owned roster, clock, ground and object lifecycle.
	room->m_CombatObjectRuntime.Reset();
	room->m_ServerNavigation.Set_RuntimeSupportSurfaces({}, status);
	room->m_Players.clear();
	for (PLAYER_ID id = 1u; id <= 3u; ++id)
	{
		auto& target = room->m_Players[id];
		target.iPlayerId = id; target.iNetEntityId = 100u + id;
		target.iCurrentHp = id == 3u ? 0u : 100u; target.iMaximumHp = 100u; target.isCombatReady = true;
		target.fPositionX = id == 2u ? 10.f : 6.f; target.fPositionY = 99.f; target.fPositionZ = 6.f;
		target.fMoveSpeed = id == 2u ? 12.f : 6.f;
	}
	BOSS_PATTERN_DEFINITION showtime;
	showtime.strPatternId = "KAKULSAYDON_TEST_SHOWTIME";
	BOSS_PATTERN_MECHANIC_TRIGGER targets{};
	targets.strTriggerId = "test.showtime.duration"; targets.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS;
	targets.iDurationMs = 5000u; targets.strFixedVisualId = "test.showtime.fixed";
	targets.strTrackingVisualId = "test.showtime.tracking"; targets.iFixedLifetimeMs = 4384u;
	targets.iSpawnIntervalMs = 2000u; targets.fFollowSpeedScale = .5f;
	showtime.MechanicTriggers.push_back(targets);
	// Saydon model +X is already aimed at the +X target when its body yaw is zero.
	albionOwner.strPatternId = showtime.strPatternId; albionOwner.iPatternSequence = 4u; albionOwner.fYawDegrees = 0.f;
	KOUKUSAYDON_LOGIC_LEDGER showtimeLedger;
	CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 2000u, showtimeLedger);
	const auto updateTargets = [&](const std::uint32_t tick) {
		room->Update_KoukuPlayerTargets(albionOwner, showtime, showtimeLedger, room->m_GameplayCatalog, tick);
	};
	const auto countTargetObjects = [&](const bool tracking) {
		return std::count_if(room->m_CombatObjectRuntime.Get_LiveObjects().begin(), room->m_CombatObjectRuntime.Get_LiveObjects().end(),
			[&](const auto& object) { return object.strCombatObjectArchetypeId == (tracking ? "combatobject.kouku.showtime.tracking" : "combatobject.kouku.showtime.fixed"); });
	};
	const auto targetPose = [&](const COMBAT_OBJECT_ID id) {
		for (const auto& object : room->m_CombatObjectRuntime.Get_LiveObjects()) if (object.iCombatObjectId == id) return object.LiveState.CurrentPose;
		return SERVER_COMBAT_OBJECT_POSE{};
	};
	updateTargets(1999u);
	tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty() && showtimeLedger.MechanicTriggers.empty() &&
		showtimeLedger.PlayerTargetWindows.size() == 1u, "Showtime is a duration window and creates nothing before its start");
	albionOwner.iPatternTargetEntityId = 102u;
	albionOwner.fPositionX = 6.f; albionOwner.fPositionZ = 6.f;
	updateTargets(2000u);
	tests.Require(albionOwner.iPatternTargetEntityId == 102u && std::abs(albionOwner.fYawDegrees) < .0001f,
		"Showtime keeps the existing server pattern target and the already aligned Saydon +X yaw, independently of roster order");
	auto& trackingIds = showtimeLedger.PlayerTargetWindows.front().TrackingObjects;
	tests.Require(countTargetObjects(false) == 2 && countTargetObjects(true) == 2 && trackingIds.size() == 2u &&
		std::all_of(room->m_CombatObjectRuntime.Get_LiveObjects().begin(), room->m_CombatObjectRuntime.Get_LiveObjects().end(),
			[](const auto& object) { return object.Hits.empty() && object.LiveState.CurrentPose.fPositionY == 1.f && object.LiveState.CurrentPose.fYawDegrees == 0.f; }),
		"Showtime tick zero creates a fixed group and one independent grounded tracker for every living player, with no damage or boss yaw");
	const auto firstTracker = trackingIds.at(1u), secondTracker = trackingIds.at(2u);
	const auto firstBirth = targetPose(firstTracker), secondBirth = targetPose(secondTracker);
	tests.Require(firstBirth.fPositionX == albionOwner.fSpawnPositionX && firstBirth.fPositionZ == albionOwner.fSpawnPositionZ &&
		secondBirth.fPositionX == albionOwner.fSpawnPositionX && secondBirth.fPositionZ == albionOwner.fSpawnPositionZ,
		"Each Showtime tracker starts at the authored arena centre rather than at its player");
	room->m_Players[1u].fPositionX = 9.f; room->m_Players[2u].fPositionX = 13.f;
	updateTargets(2001u);
	const auto firstStep = targetPose(firstTracker), secondStep = targetPose(secondTracker);
	tests.Require(std::abs(std::hypot(firstStep.fPositionX - firstBirth.fPositionX, firstStep.fPositionZ - firstBirth.fPositionZ) - .1f) < .0001f &&
		std::abs(std::hypot(secondStep.fPositionX - secondBirth.fPositionX, secondStep.fPositionZ - secondBirth.fPositionZ) - .2f) < .0001f &&
		std::hypot(9.f - firstStep.fPositionX, 6.f - firstStep.fPositionZ) < std::hypot(9.f - firstBirth.fPositionX, 6.f - firstBirth.fPositionZ) &&
		std::hypot(13.f - secondStep.fPositionX, 6.f - secondStep.fPositionZ) < std::hypot(13.f - secondBirth.fPositionX, 6.f - secondBirth.fPositionZ),
		"Each tracker advances toward its own player by half that player's effective speed per 30 Hz tick");
	updateTargets(2001u);
	tests.Require(targetPose(firstTracker).fPositionX == firstStep.fPositionX && targetPose(firstTracker).fPositionZ == firstStep.fPositionZ &&
		targetPose(secondTracker).fPositionX == secondStep.fPositionX && targetPose(secondTracker).fPositionZ == secondStep.fPositionZ &&
		countTargetObjects(false) == 2 && countTargetObjects(true) == 2,
		"Repeating the same server tick neither moves either tracker twice nor duplicates a fixed volley");
	room->m_CombatObjectRuntime.Build_LiveSpawnMessages(2001u, restored);
	wireValid = restored.size() == 4u && room->m_CombatObjectRuntime.Build_Snapshots(snapshots) && snapshots.size() == 4u;
	for (const auto& spawn : restored)
	{
		CPacketWriter writer; S2C_COMBAT_OBJECT_SPAWNED roundTrip;
		wireValid = Write_Message(writer, spawn) && wireValid;
		CPacketReader reader{ writer.Get_Buffer() };
		wireValid = Read_Message(reader, roundTrip) && wireValid && roundTrip.iSpawnTick == 2000u && roundTrip.iServerTick == 2001u &&
			roundTrip.PinnedDefinitionRevision == albionOwner.PinnedDefinitionRevision;
	}
	tests.Require(wireValid, "Showtime fixed and tracking instances round-trip through existing spawn/snapshot and late-join clock metadata");
	room->m_Players[2u].iCurrentHp = 0u;
	auto joinedPlayer = room->m_Players[1u]; joinedPlayer.iPlayerId = 4u; joinedPlayer.iNetEntityId = 104u;
	joinedPlayer.fPositionX = 8.f; joinedPlayer.fPositionZ = 10.f; room->m_Players[4u] = joinedPlayer;
	updateTargets(2002u);
	tests.Require(albionOwner.iPatternTargetEntityId != 102u && albionOwner.bHasPatternTargetLastPosition &&
		(albionOwner.iPatternTargetEntityId == 101u || albionOwner.iPatternTargetEntityId == 104u),
		"Death reselects one living target through the existing random-alive server policy");
	tests.Require(trackingIds.size() == 2u && !trackingIds.contains(2u) && trackingIds.contains(4u) && countTargetObjects(false) == 2,
		"Death removes only that player's tracker; joining creates one tracker without an early fixed volley");
	auto returningPlayer = room->m_Players[1u]; room->m_Players.erase(1u); updateTargets(2003u);
	tests.Require(!trackingIds.contains(1u) && trackingIds.size() == 1u && countTargetObjects(false) == 2, "Leaving removes the tracker and preserves already fixed groups");
	room->m_Players[1u] = returningPlayer; room->m_Players[2u].iCurrentHp = 100u; updateTargets(2004u);
	tests.Require(trackingIds.size() == 3u && trackingIds.at(1u) != firstTracker && trackingIds.at(2u) != secondTracker,
		"Return and revival allocate fresh independent tracking instances");
	room->m_Players[3u].iCurrentHp = 100u; updateTargets(2005u);
	tests.Require(trackingIds.size() == 4u && countTargetObjects(true) == 4,
		"A fourth living server player receives exactly one tracker");
	updateTargets(2059u);
	tests.Require(countTargetObjects(false) == 2, "No fixed group is emitted between the authored two-second ticks");
	updateTargets(2060u); updateTargets(2120u);
	tests.Require(countTargetObjects(false) == 10 && countTargetObjects(true) == 4,
		"Ticks 60 and 120 create one fixed group per currently living player without duplicating trackers");
	bool fixedFrozen = true;
	for (const auto& object : room->m_CombatObjectRuntime.Get_LiveObjects())
		if (object.strCombatObjectArchetypeId == "combatobject.kouku.showtime.fixed" && object.iSpawnTick == 2000u)
			fixedFrozen = fixedFrozen && object.LiveState.CurrentPose.fPositionX == (object.iLockedTargetNetEntityId == 101u ? 6.f : 10.f);
	tests.Require(fixedFrozen, "The first fixed groups remain at their sampled positions while their players move");
	albionOwner.fYawDegrees = -123.f;
	updateTargets(2150u);
	tests.Require(albionOwner.fYawDegrees == -123.f, "Showtime stops updating boss yaw at the exclusive duration end");
	tests.Require(countTargetObjects(false) == 10 && countTargetObjects(true) == 0 && trackingIds.empty(),
		"The exclusive duration end creates no volley, removes all trackers and retains authored fixed tails");
	std::vector<DAMAGE_EVENT> showtimeDamage;
	room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 4.4f, 2282u, showtimeDamage);
	tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty() && showtimeDamage.empty(),
		"Fixed groups expire through the ordinary authored lifetime and never apply gameplay damage");

	for (const bool trackingOnly : { false, true })
	{
		room->m_CombatObjectRuntime.Reset(); showtime.MechanicTriggers.front() = targets;
		if (trackingOnly) { showtime.MechanicTriggers.front().strFixedVisualId.clear(); showtime.MechanicTriggers.front().iFixedLifetimeMs = 0u; }
		else showtime.MechanicTriggers.front().strTrackingVisualId.clear();
		albionOwner.fYawDegrees = -123.f;
		const auto targetBeforeDrop = albionOwner.iPatternTargetEntityId;
		CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 2300u, showtimeLedger); updateTargets(2300u);
		if (!trackingOnly) tests.Require(albionOwner.fYawDegrees == -123.f && albionOwner.iPatternTargetEntityId == targetBeforeDrop,
			"Fixed-only player drops preserve the authored boss yaw and target during rope motion");
		tests.Require(countTargetObjects(trackingOnly) == 4 && countTargetObjects(!trackingOnly) == 0,
			trackingOnly ? "A tracking-only duration creates no fixed groups" : "A fixed-only duration allocates no trackers");
	}
	room->m_CombatObjectRuntime.Reset(); showtime.MechanicTriggers.front() = targets;
	CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 2400u, showtimeLedger);
	const auto targetIdBeforeFailure = room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId;
	room->m_Players[2u].fPositionX = -1000.f; updateTargets(2400u);
	tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty() &&
		room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId == targetIdBeforeFailure &&
		showtimeLedger.PlayerTargetWindows.front().TrackingObjects.empty(),
		"An invalid later player rolls back all newly staged Showtime instances and preserves stable IDs");
	room->m_Players[2u].fPositionX = 13.f; updateTargets(2401u);
	tests.Require(countTargetObjects(false) == 4 && countTargetObjects(true) == 4,
		"A rejected target transaction retries after valid ground returns");
	albionOwner.iPatternTargetEntityId = 101u; albionOwner.fPositionX = 6.f; albionOwner.fPositionZ = 6.f;
	room->m_Players[1u].fPositionX = 9.f; room->m_Players[1u].fPositionZ = 9.f;
	updateTargets(2402u);
	tests.Require(albionOwner.iPatternTargetEntityId == 101u && std::abs(albionOwner.fYawDegrees + 45.f) < .0001f,
		"The existing SHOWTIME_PLAYER_TARGETS immediately aims Saydon model +X independently of the rotate-only duration");
	for (const bool completed : { true, false })
	{
		room->m_CombatObjectRuntime.Reset(); albionOwner.strPatternId = showtime.strPatternId;
		CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 2500u, showtimeLedger); updateTargets(2500u);
		room->m_KoukuSaydonPatternAudition.Members.clear();
		auto& member = room->m_KoukuSaydonPatternAudition.Members.emplace_back();
		member.strMemberId = "test.showtime.owner"; member.iBossEntityId = albionOwner.iNetEntityId;
		member.iPatternSequence = albionOwner.iPatternSequence; member.PatternIds = { showtime.strPatternId }; member.LogicLedger = showtimeLedger;
		room->Clear_KoukuSaydonPatternAudition(completed);
		tests.Require(countTargetObjects(true) == 0 && countTargetObjects(false) == (completed ? 4 : 0),
			completed ? "Natural completion clears Showtime trackers while retaining fixed tails" : "Explicit stop cancels every owned Showtime instance");
	}


	// Rotation consumes the same duration clock as the target window, not render frames.
	{
		const auto savedPlayers = room->m_Players;
		const auto savedOwner = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		room->m_Players.clear();
		auto& target = room->m_Players[1u];
		target.iPlayerId = 1u; target.iNetEntityId = 101u; target.iCurrentHp = target.iMaximumHp = 100u;
		target.isCombatReady = true; target.fMoveSpeed = 6.f; target.fPositionY = 1.f;
		const auto setDirection = [&](const float yaw) {
			const double radians = double(yaw) * 3.14159265358979323846 / 180.0;
			target.fPositionX = 8.f + static_cast<float>(std::sin(radians));
			target.fPositionZ = 8.f + static_cast<float>(std::cos(radians));
		};
		const auto beginFacing = [&](const float yaw, const std::uint32_t durationMs) {
			room->m_CombatObjectRuntime.Reset(); showtime.MechanicTriggers.front() = {};
			showtime.MechanicTriggers.front().strTriggerId = "test.boss.track.duration";
			showtime.MechanicTriggers.front().eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET;
			showtime.MechanicTriggers.front().iDurationMs = durationMs;
			albionOwner.strPatternId = showtime.strPatternId; albionOwner.strArchetypeId = "BOSS_KAKULSAYDON_G1_KOUKU";
			albionOwner.fPositionX = albionOwner.fPositionZ = 8.f; albionOwner.fYawDegrees = yaw;
			albionOwner.iPatternTargetEntityId = albionOwner.iTargetEntityId = 101u;
			CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 5000u, showtimeLedger);
		};
		const auto nearYaw = [](const float actual, const float expected) {
			return std::abs(std::remainder(actual - expected, 360.f)) < .001f;
		};
		setDirection(90.f); beginFacing(0.f, 1000u); updateTargets(4999u);
		tests.Require(albionOwner.fYawDegrees == 0.f, "Showtime interpolation preserves yaw before the duration begins");
		updateTargets(5000u);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty() && showtimeLedger.MechanicTriggers.empty() &&
			showtimeLedger.PlayerTargetWindows.size() == 1u, "Rotate-only duration uses the actual clock without visual templates or spawned objects");
		tests.Require(nearYaw(albionOwner.fYawDegrees, 30.f), "Rotate-only tracking reacts on the first tick with ten times the previous three-degree step");
		updateTargets(5000u); updateTargets(4999u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 30.f), "Duplicate and older ticks do not rotate the target twice");
		for (std::uint32_t tick = 5001u; tick <= 5014u; ++tick) updateTargets(tick);
		const float singleTickHalfYaw = albionOwner.fYawDegrees;
		tests.Require(nearYaw(singleTickHalfYaw, 89.9910045f), "The accelerated response reaches the fixed target early without overshooting");
		beginFacing(0.f, 1000u); updateTargets(5000u); updateTargets(5014u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, singleTickHalfYaw), "The same elapsed ticks give the same yaw when updates are grouped");
		updateTargets(5029u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 90.f), "The last valid duration tick reaches the current target exactly");
		setDirection(-90.f); updateTargets(5030u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 90.f), "The exclusive duration end cannot turn toward a changed target");
		setDirection(90.f); beginFacing(0.f, 2000u); updateTargets(5000u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 15.f), "A two-second window retains ten times the previous 1.5-degree first step");
		setDirection(-179.f); beginFacing(179.f, 1000u); updateTargets(5014u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, -179.0001999f), "Crossing positive 180 degrees uses the short two-degree arc");
		setDirection(179.f); beginFacing(-179.f, 1000u); updateTargets(5014u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 179.0001999f), "Crossing negative 180 degrees uses the short reverse arc");
		setDirection(90.f); beginFacing(0.f, 1000u); updateTargets(5014u); setDirection(-90.f); updateTargets(5015u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, -30.0029985f) && albionOwner.iPatternTargetEntityId == 101u,
			"A moving selected target is followed from the current yaw using the remaining duration");
		setDirection(90.f); beginFacing(-90.f, 1000u); albionOwner.strArchetypeId = "BOSS_KAKULSAYDON_G2_BIG_SAYDON"; updateTargets(5014u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, -0.0089955f), "Big Saydon keeps its existing minus-ninety-degree model forward basis during interpolation");
		for (const auto* archetype : { "BOSS_KAKULSAYDON_G1_SAYDON", "BOSS_KAKULSAYDON_G3_SAYDON",
			"BOSS_KAKULSAYDON_BINGO_SAYDON", "BOSS_KAKULSAYDON_G2_BIG_SAYDON" })
			for (const bool immediate : { false, true })
				for (const float heading : { 0.f, 90.f, 180.f, -90.f })
				{
					setDirection(heading); beginFacing(45.f, 1000u);
					albionOwner.strArchetypeId = archetype;
					if (immediate) { showtime.MechanicTriggers.front() = targets; }
					CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 5000u, showtimeLedger);
					updateTargets(immediate ? 5000u : 5029u);
					const float forward = (albionOwner.fYawDegrees + 90.f) * .017453292519943295f;
					const float targetHeading = heading * .017453292519943295f;
					const float dot = std::sin(forward) * std::sin(targetHeading) + std::cos(forward) * std::cos(targetHeading);
					tests.Require(dot > .99999f && nearYaw(albionOwner.fYawDegrees, heading - 90.f),
						immediate ? "Showtime immediate tracking aims the actual Saydon model +X front at the player" :
						"Stationary Showtime tracking aims the actual Saydon model +X front at the player");
				}
		// Movement-enabled tracking still commits the body yaw before its root/navigation step.
		beginFacing(0.f, 8582u); setDirection(90.f);
		showtime.MechanicTriggers.front().fFollowSpeedScale = 1.f;
		albionOwner.strArchetypeId = "BOSS_KAKULSAYDON_G1_SAYDON"; albionOwner.fYawDegrees = -90.f;
		albionOwner.fPositionY = 1.f; albionOwner.fCollisionRadius = .1f;
		albionOwner.iPatternStartTick = 5000u; albionOwner.iPatternStageIndex = 0u;
		albionOwner.iPatternStageFirstEvaluationTick = 5000u;
		albionOwner.bPatternStageRootOriginCaptured = false; albionOwner.iPatternStageRootLastTick = 0u;
		albionOwner.AlbionAirborne = {};
		albionOwner.PatternStageRootMotion = {{0u,0.f,0.f,0.f},{10000u,0.f,0.f,0.f}};
		auto trackingPattern = showtime; trackingPattern.Stages.resize(1u); trackingPattern.Stages[0].iDurationMs = 10000u;
		room->m_ServerNavigation.Set_RuntimeSupportSurfaces({}, status);
		room->m_ServerCollisionSystem.Initialize({}, status); room->m_ServerCollisionSystem.Set_BlockingBodies({});
		bool trackingRootValid = true;
		for (std::uint32_t tick = 5000u; tick < 5030u; ++tick)
		{
			trackingRootValid = CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, trackingPattern, tick,
				room->m_ServerNavigation, room->m_ServerCollisionSystem, status) && trackingRootValid;
			target.fPositionX = albionOwner.fPositionX + 100.f; target.fPositionZ = albionOwner.fPositionZ;
			updateTargets(tick);
			if (tick == 5000u) tests.Require(nearYaw(albionOwner.fYawDegrees, -84.f), "Pursuit turns six degrees on its first 30 Hz tick regardless of the 8582 ms lifetime");
			if (tick == 5014u) tests.Require(nearYaw(albionOwner.fYawDegrees, 0.f), "Moving pursuit turns ninety degrees within half a second");
		}
		const float trackingYaw = albionOwner.fYawDegrees;
		tests.Require(trackingRootValid && nearYaw(trackingYaw, 0.f) && albionOwner.fPositionX > 8.f,
			"Moving Saydon tracks with a changing body yaw through thirty real root/navigation ticks");
		tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, trackingPattern, 5030u,
			room->m_ServerNavigation, room->m_ServerCollisionSystem, status) && albionOwner.fYawDegrees == trackingYaw,
			"The next absolute root-motion sample preserves the committed tracking yaw");
		beginFacing(0.f, 20000u); setDirection(90.f); showtime.MechanicTriggers.front().fFollowSpeedScale = 1.f; updateTargets(5000u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 6.f), "A longer pursuit lifetime does not slow its first turn");
		target.fPositionX = albionOwner.fPositionX + 100.f; target.fPositionZ = albionOwner.fPositionZ; updateTargets(5014u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 90.f), "Grouped pursuit ticks consume the same bounded angular time");
		beginFacing(179.f, 8582u); setDirection(-179.f); showtime.MechanicTriggers.front().fFollowSpeedScale = 1.f; updateTargets(5000u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, -179.f), "Moving pursuit crosses the yaw seam along the shortest arc");
		room->m_CombatObjectRuntime.Reset(); room->m_Players = savedPlayers; albionOwner = *savedOwner;
	}


	// The optional ordered random volley is one global instance, independent of the roster.
	{
		const auto savedPlayers = room->m_Players;
		const auto savedOwner = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		const auto beginRandom = [&](const std::uint32_t count, const std::uint32_t start, const std::uint32_t duration) {
			room->m_CombatObjectRuntime.Reset(); room->m_Players.clear();
			for (PLAYER_ID id = 1u; id <= count; ++id) {
				auto& player = room->m_Players[id]; player.iPlayerId = id; player.iNetEntityId = 100u + id;
				player.iCurrentHp = player.iMaximumHp = 100u; player.isCombatReady = true; player.fMoveSpeed = 6.f;
				player.fPositionX = 7.f + float(id); player.fPositionY = 1.f; player.fPositionZ = 8.f;
			}
			albionOwner.strPatternId = showtime.strPatternId; albionOwner.iPatternSequence = 71u;
			albionOwner.fPositionX = albionOwner.fSpawnPositionX = 8.f;
			albionOwner.fPositionY = albionOwner.fSpawnPositionY = 1.f;
			albionOwner.fPositionZ = albionOwner.fSpawnPositionZ = 8.f;
			showtime.MechanicTriggers = {targets}; auto& definition = showtime.MechanicTriggers.front();
			definition.iDurationMs = duration;
			definition.RandomVolleys = { { {}, "test.random.a", 5000u}, { {}, "test.random.a", 5000u},
				{ {}, "test.random.b", 5000u}, { {}, "test.random.b", 5000u}, { {}, "test.random.c", 5000u}, { {}, "test.random.c", 5000u} };
			definition.iRandomSpawnIntervalMs = 500u; definition.fRandomArenaRadiusM = 2.f; definition.fRandomArenaHeightToleranceM = .2f;
			CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, start, showtimeLedger);
		};
		const auto randomObjects = [&] {
			std::vector<SERVER_COMBAT_OBJECT> result;
			for (const auto& object : room->m_CombatObjectRuntime.Get_LiveObjects())
				if (object.strClientVisualId.starts_with("test.random.")) result.push_back(object);
			return result;
		};
		for (std::uint32_t count = 1u; count <= 4u; ++count) {
			beginRandom(count, 8000u, 3000u); updateTargets(8000u);
			tests.Require(randomObjects().size() == 1u && countTargetObjects(true) == count &&
				countTargetObjects(false) == count + 1u, "Random volley creates one global object while preserving every player's fixed and tracking instances");
			updateTargets(8014u); updateTargets(8015u); updateTargets(8015u);
			tests.Require(randomObjects().size() == 2u && showtimeLedger.PlayerTargetWindows.front().iRandomWaveOrdinal == 2u,
				"Random cadence is fifteen fixed ticks and repeating one tick never creates a second instance");
			for (auto tick : {8030u, 8045u, 8060u, 8075u}) updateTargets(tick);
			auto objects = randomObjects();
			bool positionsValid = objects.size() == 6u, varies = false;
			for (std::size_t i = 0u; i < objects.size(); ++i) {
				const auto& pose = objects[i].LiveState.CurrentPose;
				positionsValid = positionsValid && std::hypot(pose.fPositionX - 8.f, pose.fPositionZ - 8.f) <= 2.0001f &&
					room->m_ServerNavigation.Is_PointWalkableExact(pose.fPositionX, pose.fPositionZ) && pose.fPositionY == 1.f &&
					pose.fYawDegrees == 0.f && objects[i].iLockedTargetNetEntityId == INVALID_NET_ENTITY_ID && objects[i].Hits.empty() &&
					objects[i].strClientVisualId == (i < 2u ? "test.random.a" : i < 4u ? "test.random.b" : "test.random.c");
				if (i != 0u) varies = varies || pose.fPositionX != objects.front().LiveState.CurrentPose.fPositionX || pose.fPositionZ != objects.front().LiveState.CurrentPose.fPositionZ;
			}
			tests.Require(positionsValid && varies, "Ordered duplicate templates cycle A1 B1 A2 B2 A3 B3 at changing deterministic walkable map positions");
			updateTargets(8090u);
			tests.Require(randomObjects().size() == 6u && countTargetObjects(true) == 0u && countTargetObjects(false) == 6u + count * 2u,
				"Duration end stops new random volleys and tracking but preserves already emitted random and fixed tails");
		}
		beginRandom(1u, 9000u, 5000u); room->m_Players[1u].isCombatReady = false; updateTargets(9000u);
		tests.Require(randomObjects().empty() && showtimeLedger.PlayerTargetWindows.front().iNextRandomTick == 9015u &&
			showtimeLedger.PlayerTargetWindows.front().iRandomWaveOrdinal == 0u,
			"No alive player consumes schedule time without spawning or consuming the next authored volley");
		room->m_Players[1u].isCombatReady = true; updateTargets(9010u);
		tests.Require(randomObjects().empty(), "Returning before the next deadline does not replay the missed empty-room interval");
		updateTargets(9015u); updateTargets(9080u);
		tests.Require(randomObjects().size() == 2u && showtimeLedger.PlayerTargetWindows.front().iNextRandomTick == 9090u,
			"A skipped update emits one current volley and advances past missed deadlines without a burst");
		const auto beforeFailureId = room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId;
		albionOwner.fSpawnPositionY = 1000.f; updateTargets(9090u);
		tests.Require(randomObjects().size() == 2u && showtimeLedger.PlayerTargetWindows.front().iRandomWaveOrdinal == 2u &&
			showtimeLedger.PlayerTargetWindows.front().iNextRandomTick == 9090u &&
			room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId == beforeFailureId,
			"Invalid arena height preserves random ordinal, deadline, existing objects and next object identity");
		albionOwner.fSpawnPositionY = 1.f; updateTargets(9091u);
		tests.Require(randomObjects().size() == 3u && randomObjects().back().strClientVisualId == "test.random.b" &&
			showtimeLedger.PlayerTargetWindows.front().iNextRandomTick == 9105u,
			"Restoring navigable ground retries the unconsumed ordered volley once on the next tick");
        beginRandom(1u, 9500u, 2000u);
        auto& rain = showtime.MechanicTriggers.front();
        rain.strFixedVisualId.clear(); rain.strTrackingVisualId.clear(); rain.iFixedLifetimeMs = 0u;
        rain.strRandomAnchorKind = "BOSS"; rain.fRandomScaleMin = 1.f; rain.fRandomScaleMax = 2.f;
        rain.fRandomArenaRadiusM = .5f;
        ATTACK_HIT_TEMPLATE cardHit; cardHit.strHitId = "cardrain.impact"; cardHit.iAtMs = 1650u;
        for (auto& volley : rain.RandomVolleys) volley.Hits = {cardHit};
        albionOwner.fPositionX = 12.f; albionOwner.fYawDegrees = 37.f;
        const auto cardRainTargetBefore = albionOwner.iPatternTargetEntityId;
        CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 9500u, showtimeLedger);
        updateTargets(9500u); updateTargets(9515u);
        auto rainObjects = randomObjects();
        bool rainValid = rainObjects.size() == 2u && countTargetObjects(true) == 0u && albionOwner.fYawDegrees == 37.f &&
            albionOwner.iPatternTargetEntityId == cardRainTargetBefore;
        for (const auto& object : rainObjects)
        {
            const auto& pose = object.LiveState.CurrentPose;
            rainValid = rainValid && std::hypot(pose.fPositionX - 12.f, pose.fPositionZ - 8.f) <= .5001f &&
                room->m_ServerNavigation.Is_PointWalkableExact(pose.fPositionX, pose.fPositionZ) &&
                object.fUniformScale >= 1.f && object.fUniformScale <= 2.f && object.Hits.size() == 1u;
        }
        tests.Require(rainValid, "Card rain owns one nav-projected random object per interval around current Saydon with scale 1..2 and no target-facing mutation");
        tests.Require(rainObjects.size() == 2u && rainObjects[0].fUniformScale != rainObjects[1].fUniformScale,
            "Card rain scale changes deterministically between independently spawned groups");
		beginRandom(2u, 10000u, 2000u);
		auto secondWindow = showtime.MechanicTriggers.front(); secondWindow.strTriggerId += ".second"; secondWindow.iStartMs = 250u; secondWindow.iDurationMs = 1500u;
		showtime.MechanicTriggers.push_back(secondWindow); CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 10000u, showtimeLedger);
		updateTargets(10000u); updateTargets(10008u); updateTargets(10015u); updateTargets(10023u);
		tests.Require(randomObjects().size() == 4u && showtimeLedger.PlayerTargetWindows[0].iRandomWaveOrdinal == 2u &&
			showtimeLedger.PlayerTargetWindows[1].iRandomWaveOrdinal == 2u && countTargetObjects(true) == 4u,
			"Concurrent duration windows keep separate random cadences and their own player trackers");
		room->m_CombatObjectRuntime.Reset(); room->m_Players = savedPlayers; albionOwner = *savedOwner;
	}

	// Typed Albion phases use the actual room transaction and existing root/navigation sweep.
	{
		using Phase = ALBION_AIRBORNE_PHASE;
		const auto savedOwner = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		const auto savedPlayers = room->m_Players;
		room->m_ServerNavigation.Set_RuntimeSupportSurfaces({}, status);
		room->m_ServerCollisionSystem.Initialize({}, status); room->m_ServerCollisionSystem.Set_BlockingBodies({});
		room->m_Players.clear();
		SERVER_PLAYER target; target.iPlayerId=1u; target.iNetEntityId=401u; target.iCurrentHp=100u; target.isCombatReady=true;
		target.fPositionX=10.f; target.fPositionY=1.f; target.fPositionZ=10.f; room->m_Players[1u]=target;
		BOSS_PATTERN_DEFINITION pattern; pattern.strPatternId="KAKULSAYDON_AIRBORNE_CONTRACT"; pattern.bFixedTimelineClock=true;
		pattern.Stages.emplace_back(); pattern.Stages.front().iDurationMs=8000u;
		albionOwner.strPatternId=pattern.strPatternId; albionOwner.iPatternSequence=73u; albionOwner.iPatternStageIndex=0u;
		albionOwner.iPatternStartTick=12000u; albionOwner.iActionStartTick=12000u; albionOwner.iPatternStageFirstEvaluationTick=12000u;
		albionOwner.strActionId="keep.source.clip"; albionOwner.fPositionX=6.f; albionOwner.fPositionY=1.f; albionOwner.fPositionZ=6.f;
		albionOwner.fYawDegrees=0.f; albionOwner.fCollisionRadius=.1f; albionOwner.AlbionAirborne={};
		albionOwner.PatternStageRootMotion.clear(); albionOwner.bPatternStageRootOriginCaptured=false; albionOwner.iPatternStageRootLastTick=0u;
		BOSS_PATTERN_MECHANIC_TRIGGER phase; phase.strTriggerId="air.jump"; phase.eKind=BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE;
		phase.eAirbornePhase=Phase::JUMP; phase.fAirborneHeightM=13.6788133f; phase.iAirborneDurationMs=200u;
		const auto rootAt=[&](std::uint32_t tick) { return CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner,pattern,tick,room->m_ServerNavigation,room->m_ServerCollisionSystem,status); };
		const auto beforeInstant = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		phase.iAirborneDurationMs=0u; phase.fAirborneHeightM=11.1910783f;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12000u) &&
			std::abs(albionOwner.fPositionY-12.1910783f)<.00001f,
			"Zero-duration JUMP initializes the requested high pose on its first server tick");
		albionOwner.PatternStageRootMotion={{0u,0.f,0.f,0.f},{1000u,0.f,.5f,-5.59553915f},{2000u,0.f,1.f,-11.1910783f},{8000u,0.f,1.f,-10.7271685f}};
		phase.eAirbornePhase=Phase::SLAM; phase.fAirborneHeightM=0.f;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12000u) &&
			std::abs(albionOwner.fPositionY-12.1910783f)<.00001f && rootAt(12030u) &&
			std::abs(albionOwner.fPositionY-6.59553915f)<.00001f && rootAt(12060u) && albionOwner.fPositionY==1.f &&
			albionOwner.fPositionX==6.f && albionOwner.fPositionZ==6.f,
			"Same-tick high initialization and native descent preserve full height, midpoint and grounded endpoint");
		{
			auto clone = std::make_unique<SERVER_WORLD_ENTITY>(*beforeInstant);
			clone->bKoukuSummonClone = true; clone->iNetEntityId = 790u;
			clone->PatternStageRootMotion = {{0u,0.f,0.f,0.f},{1000u,0.f,0.f,-5.59553915f},{2000u,0.f,0.f,-11.1910783f}};
			auto clonePattern = pattern;
			auto jump = phase; jump.strTriggerId = "clone.jump"; jump.eAirbornePhase = Phase::JUMP; jump.fAirborneHeightM = 11.1910783f;
			auto slam = phase; slam.strTriggerId = "clone.slam";
			clonePattern.MechanicTriggers = {slam, jump}; // Deliberately reverse same-clock authored order.
			tests.Require(room->Update_KoukuSummonTriggers(*clone, clonePattern, 12000u) &&
				clone->KoukuSummonStartedTriggers.size() == 2u && std::abs(clone->fPositionY - 12.1910783f) < .00001f,
				"A summoned actor commits initial JUMP before SLAM even when equal-clock rows are reversed");
			tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(*clone, clonePattern, 12030u,
				room->m_ServerNavigation, room->m_ServerCollisionSystem, status) &&
				room->Update_KoukuSummonTriggers(*clone, clonePattern, 12030u) &&
				std::abs(clone->fPositionY - 6.59553915f) < .00001f && clone->KoukuSummonStartedTriggers.size() == 2u,
				"Repeated clone ticks do not reapply initial height and preserve native midpoint descent");
			tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(*clone, clonePattern, 12060u,
				room->m_ServerNavigation, room->m_ServerCollisionSystem, status) && clone->fPositionY == 1.f,
				"The summoned actor lands on the same authoritative navigation floor");
		}
		pattern.Stages.emplace_back(); pattern.Stages.back().iDurationMs=1000u;
		albionOwner.iPatternStageIndex=1u; albionOwner.iPatternStageFirstEvaluationTick=12240u;
		albionOwner.PatternStageRootMotion={{0u,0.f,0.f,0.f},{1000u,0.f,2.f,4.f}};
		albionOwner.bPatternStageRootOriginCaptured=false; albionOwner.iPatternStageRootLastTick=0u;
		tests.Require(rootAt(12255u) && std::abs(albionOwner.fPositionY-3.f)<.00001f && std::abs(albionOwner.fPositionX-7.f)<.00001f,
			"A later stage resumes its own native root rise and lateral movement after the landing owner ends");
		pattern.Stages.resize(1u);
		albionOwner=*beforeInstant; phase.eAirbornePhase=Phase::JUMP; phase.fAirborneHeightM=13.6788133f; phase.iAirborneDurationMs=200u;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12000u) && albionOwner.fPositionY==1.f,
			"Albion jump begins at the current ground without an instant vertical snap even on an empty source curve");
		tests.Require(rootAt(12003u) && std::abs(albionOwner.fPositionY-1.f-13.6788133f*.5f)<.00001f,
			"Actual fixed-tick navigation/root consumer reaches half original jump height in 100ms");
		tests.Require(rootAt(12006u) && std::abs(albionOwner.fPositionY-14.6788133f)<.00001f && rootAt(12015u) && std::abs(albionOwner.fPositionY-14.6788133f)<.00001f,
			"The 200ms ascent reaches the unchanged original height and holds after the shortened ramp");
		phase.eAirbornePhase=Phase::SELECT_PLAYER; phase.fAirborneHeightM=0.f; phase.iAirborneDurationMs=0u;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12016u) && albionOwner.AlbionAirborne.iSelectedPlayer==401u && albionOwner.AlbionAirborne.ePhase==Phase::JUMP,
			"SELECT_PLAYER pins an alive server player without changing the active jump phase");
		target.iPlayerId=2u; target.iNetEntityId=402u; target.fPositionX=12.f; target.fPositionZ=12.f; room->m_Players[2u]=target;
		albionOwner.PatternStageRootMotion={{0u,0.f,0.f,0.f},{2000u,0.f,0.f,0.f},{3000u,0.f,0.f,-3.2783114f}};
		phase.eAirbornePhase=Phase::APPEAR_PLAYER; phase.fAirborneHeightM=3.2783114f; phase.iStartMs=600u;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12018u) && albionOwner.fPositionX==10.f && albionOwner.fPositionZ==10.f && std::abs(albionOwner.fPositionY-4.2783114f)<.00001f,
			"Appearance reuses the pinned player XZ and the original landing clip height");
		tests.Require(rootAt(12019u) && albionOwner.fPositionX==10.f && albionOwner.fPositionZ==10.f,
			"The next source root sample retains the teleported XZ instead of returning to the old stage origin");
		room->m_Players[1u].iCurrentHp=0u;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12020u) && albionOwner.AlbionAirborne.iSelectedPlayer==402u && albionOwner.fPositionX==12.f,
			"An invalid pinned target is replaced by the remaining living server player");
		phase.eAirbornePhase=Phase::DISAPPEAR; phase.fAirborneHeightM=0.f;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12021u) && std::abs(albionOwner.fPositionY-14.6788133f)<.00001f,
			"Disappear restores the initial jump height rather than accumulating the landing offset");
		room->m_Players[2u].iCurrentHp=0u; phase.eAirbornePhase=Phase::APPEAR_PLAYER; phase.fAirborneHeightM=3.2783114f;
		const auto beforeNoPlayer=std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		tests.Require(!room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12022u) && albionOwner.fPositionY==beforeNoPlayer->fPositionY &&
			albionOwner.fPositionX==beforeNoPlayer->fPositionX && albionOwner.AlbionAirborne.ePhase==Phase::DISAPPEAR,
			"No selectable player preserves the high boss position and previous phase transactionally");
		phase.eAirbornePhase=Phase::CENTER; phase.fAirborneHeightM=0.f; phase.fTeleportX=6.f; phase.fTeleportY=1.f; phase.fTeleportZ=6.f;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12023u) && albionOwner.fPositionX==6.f && albionOwner.fPositionZ==6.f &&
			std::abs(albionOwner.fPositionY-14.6788133f)<.00001f,
			"CENTER consumes an absolute navigation-checked XZ at the original jump height");
		phase.fTeleportX=6.f; phase.fTeleportZ=2.f;
		tests.Require(!room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12024u) && albionOwner.fPositionZ==6.f && albionOwner.AlbionAirborne.ePhase==Phase::CENTER,
			"Invalid center navigation preserves the current position and airborne phase");
		albionOwner.PatternStageRootMotion={{0u,0.f,0.f,0.f},{1000u,0.f,0.f,0.f},{2000u,0.f,0.f,-3.5f},{2501u,0.f,0.f,-7.f},{3000u,0.f,0.f,-6.9f},{8000u,0.f,0.f,-6.9f}};
		phase.eAirbornePhase=Phase::SLAM; phase.iStartMs=1000u; phase.fTeleportX=phase.fTeleportY=phase.fTeleportZ=0.f;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12030u) && std::abs(albionOwner.fPositionY-14.6788133f)<.00001f,
			"SLAM starts from the held height and commits through the existing navigation/body sweep");
		tests.Require(rootAt(12060u) && std::abs(albionOwner.fPositionY-1.f-13.6788133f*.5f)<.00001f,
			"SLAM follows the original remaining source displacement normalized from trigger to source minimum");
		tests.Require(rootAt(12090u) && albionOwner.fPositionY==1.f && rootAt(12230u) && albionOwner.fPositionY==1.f,
			"Skipped minimum samples and an extended Stage tail remain exactly on navigation ground without rebound");
		tests.Require(albionOwner.iPatternStartTick==12000u && albionOwner.iActionStartTick==12000u && albionOwner.iPatternStageIndex==0u &&
			albionOwner.strActionId=="keep.source.clip" && albionOwner.fPositionX==6.f && albionOwner.fPositionZ==6.f,
			"All phases preserve animation, Stage, Pattern clocks and the central slam XZ");
		const auto beforeFlat=std::make_unique<SERVER_WORLD_ENTITY>(albionOwner); albionOwner.PatternStageRootMotion={{0u,0.f,0.f,0.f},{8000u,0.f,0.f,0.f}};
		tests.Require(!room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12231u) && albionOwner.fPositionY==beforeFlat->fPositionY &&
			albionOwner.AlbionAirborne.fSourceUpMinimum==beforeFlat->AlbionAirborne.fSourceUpMinimum,
			"A source curve without remaining descent rejects the phase without replacing the prior state");
        // SELECT policy captures navigation ground once and shares the reliable fixed object pivot.
        const auto savedAuditionPhase=room->m_KoukuSaydonPatternAudition.ePhase;
        const auto savedAuditionRevision=room->m_KoukuSaydonPatternAudition.PinnedGameplayRevision;
        const auto savedAuditionSource=room->m_KoukuSaydonPatternAudition.iPinnedSourceRevision;
        room->m_KoukuSaydonPatternAudition.ePhase=CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
        room->m_KoukuSaydonPatternAudition.PinnedGameplayRevision=room->m_GameplayCatalog.Get_ActiveRevision();
        room->m_KoukuSaydonPatternAudition.iPinnedSourceRevision=CKoukuSaydonBrain::Resolve_ProductSourceRevision(room->m_GameplayCatalog);
        room->m_CombatObjectRuntime.Reset();
        room->m_Players.clear(); target.iPlayerId=1u; target.iNetEntityId=401u;
        target.iCurrentHp=100u; target.fPositionX=10.f; target.fPositionY=99.f; target.fPositionZ=10.f;
        room->m_Players[1u]=target;
        phase.eAirbornePhase=Phase::SELECT_PLAYER; phase.iStartMs=0u; phase.fAirborneHeightM=0.f;
        phase.bCaptureAirborneTargetPosition=true; phase.strSelectedEffectVisualId="test.selected.group"; phase.iSelectedEffectLifetimeMs=2000u;
        tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12231u) &&
            albionOwner.AlbionAirborne.bHasSelectedGround && albionOwner.AlbionAirborne.SelectedGround.x==10.f &&
            albionOwner.AlbionAirborne.SelectedGround.y==1.f && albionOwner.AlbionAirborne.SelectedGround.z==10.f,
            "SELECT policy pins the actual navigation ground rather than the player's transient visual height");
        const auto& selectedObjects=room->m_CombatObjectRuntime.Get_LiveObjects();
        tests.Require(selectedObjects.size()==1u && selectedObjects.front().strClientVisualId=="test.selected.group" &&
            selectedObjects.front().LiveState.CurrentPose.fPositionX==10.f && selectedObjects.front().LiveState.CurrentPose.fPositionY==1.f &&
            selectedObjects.front().LiveState.CurrentPose.fPositionZ==10.f && selectedObjects.front().LiveState.CurrentPose.fYawDegrees==0.f &&
            selectedObjects.front().Hits.empty(), "The single reliable fixed Effect object shares the captured XYZ and has no damage or boss yaw");
        const auto beforeSelectedFailure=albionOwner.AlbionAirborne;
        phase.iSelectedEffectLifetimeMs=0u;
        tests.Require(!room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12231u) &&
            room->m_CombatObjectRuntime.Get_LiveObjects().size()==1u && albionOwner.AlbionAirborne.SelectedGround.x==beforeSelectedFailure.SelectedGround.x,
            "A rejected fixed-object transaction preserves the previous target anchor and live Effect");
        phase.strSelectedEffectVisualId.clear(); phase.bCaptureAirborneTargetPosition=false;
        phase.eAirbornePhase=Phase::APPEAR_PLAYER; phase.fAirborneHeightM=3.2783114f;
        room->m_Players[1u].fPositionX=12.f; room->m_Players[1u].fPositionZ=12.f; room->m_Players[1u].iCurrentHp=0u;
        tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12231u) &&
            albionOwner.AlbionAirborne.iSelectedPlayer==401u && albionOwner.fPositionX==10.f && albionOwner.fPositionZ==10.f &&
            std::abs(albionOwner.fPositionY-4.2783114f)<.00001f,
            "SELECT policy appearance reuses its exact ground point after player movement and death without replacement");
        room->m_CombatObjectRuntime.Cancel_Source(albionOwner.iNetEntityId);
        room->m_KoukuSaydonPatternAudition.ePhase=savedAuditionPhase;
        room->m_KoukuSaydonPatternAudition.PinnedGameplayRevision=savedAuditionRevision;
        room->m_KoukuSaydonPatternAudition.iPinnedSourceRevision=savedAuditionSource;
        tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "Existing Stop/source cleanup removes the captured group object");
        room->m_KoukuSaydonBrain.Abort_Pattern(albionOwner,12232u);
		tests.Require(albionOwner.AlbionAirborne.ePhase==Phase::NONE && albionOwner.AlbionAirborne.iSelectedPlayer==INVALID_NET_ENTITY_ID,
			"Pattern abort clears the airborne phase and pinned player without a second movement authority");
		albionOwner=*savedOwner; room->m_Players=savedPlayers;
	}

	// Teleport rebases only XZ; the next actual animation root sample retains height and clocks.
	{
		room->m_ServerCollisionSystem.Initialize({}, status);
		room->m_ServerCollisionSystem.Set_BlockingBodies({});
		room->m_ServerNavigation.Set_RuntimeSupportSurfaces({ { "teleport.ground", 10.f, 10.f, 2.f, 1.6f } }, status);
		BOSS_PATTERN_DEFINITION teleportPattern; teleportPattern.strPatternId = "KAKULSAYDON_TEST_TELEPORT_XZ";
		teleportPattern.Stages.emplace_back(); teleportPattern.Stages.front().iDurationMs = 1000u;
		teleportPattern.Stages.front().Motion.RootMotion = { {0u, 0.f, 0.f, 0.f}, {1000u, 0.f, 3.f, 2.f} };
		albionOwner.strPatternId = teleportPattern.strPatternId; albionOwner.iPatternSequence = 22u;
		albionOwner.fPositionX = 6.f; albionOwner.fPositionY = 4.f; albionOwner.fPositionZ = 6.f;
		albionOwner.fYawDegrees = 0.f; albionOwner.fCollisionRadius = .1f;
		albionOwner.iPatternStageIndex = 0u; albionOwner.iPatternStageFirstEvaluationTick = 3000u;
		albionOwner.iPatternStartTick = 3000u; albionOwner.iActionStartTick = 3000u;
		albionOwner.strActionId = "teleport.keep.animation";
		albionOwner.bPatternStageRootOriginCaptured = false; albionOwner.iPatternStageRootLastTick = 0u;
		albionOwner.PatternStageRootMotion = teleportPattern.Stages.front().Motion.RootMotion;
		tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, teleportPattern, 3003u,
			room->m_ServerNavigation, room->m_ServerCollisionSystem, status), "Capture a running airborne root curve before XZ teleport");
		const auto beforeTeleport = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		BOSS_PATTERN_MECHANIC_TRIGGER teleport;
		teleport.strTriggerId = "test.teleport.xz"; teleport.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_XZ;
		teleport.iDurationMs = 100u; teleport.fTeleportX = 10.f; teleport.fTeleportY = 1.3f; teleport.fTeleportZ = 10.f;
		const auto queueTeleport = [&](const std::uint32_t tick = 3003u) {
			room->m_PendingKoukuMechanicTriggers.push_back({ albionOwner.iNetEntityId, albionOwner.iPatternSequence, teleport });
			room->Commit_KoukuMechanicTriggers(tick);
		};
		queueTeleport();
		tests.Require(albionOwner.fPositionX == 10.f && albionOwner.fPositionZ == 10.f &&
			albionOwner.fPositionY == beforeTeleport->fPositionY && albionOwner.fPatternStageOriginY == beforeTeleport->fPatternStageOriginY &&
			albionOwner.iPatternStageRootLastTick == 3003u && albionOwner.iPatternStartTick == 3000u &&
			albionOwner.iActionStartTick == 3000u && albionOwner.strActionId == beforeTeleport->strActionId &&
			albionOwner.iPatternSequence == 22u && albionOwner.fYawDegrees == beforeTeleport->fYawDegrees,
			"XZ teleport ignores reference Y and preserves airborne height, action, stage and pattern clocks without clones");
		tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, teleportPattern, 3004u,
			room->m_ServerNavigation, room->m_ServerCollisionSystem, status) &&
			std::abs(albionOwner.fPositionX - 10.1f) < .0001f && albionOwner.fPositionZ == 10.f &&
			std::abs(albionOwner.fPositionY - beforeTeleport->fPositionY - 2.f / 30.f) < .0001f,
			"The next root frame continues at the new XZ and preserves root-up across different ground heights");
		const auto stablePose = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		room->m_ServerCollisionSystem.Set_BlockingBodies({ {12.f, 8.f, 1.f, albionOwner.fPositionY, 2.f, 999u} });
		teleport.fTeleportX = 12.f; teleport.fTeleportZ = 8.f; queueTeleport();
		const auto preserved = [&] { return albionOwner.fPositionX == stablePose->fPositionX &&
			albionOwner.fPositionY == stablePose->fPositionY && albionOwner.fPositionZ == stablePose->fPositionZ &&
			albionOwner.fPatternStageOriginX == stablePose->fPatternStageOriginX &&
			albionOwner.fPatternStageOriginZ == stablePose->fPatternStageOriginZ &&
			albionOwner.fPatternStageRootGroundY == stablePose->fPatternStageRootGroundY; };
		tests.Require(preserved(), "Occupied destination rejects XZ teleport without partially rebasing the root origin");
		teleport.fTeleportX = 6.f; teleport.fTeleportZ = 2.f; queueTeleport();
		tests.Require(preserved(), "Missing navigation rejects XZ teleport and preserves its full previous pose");
        // The explicit grounded variant lands the source root baseline as one transaction.
        room->m_ServerCollisionSystem.Set_BlockingBodies({});
        albionOwner = *beforeTeleport;
        teleport.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED;
        teleport.fTeleportX = teleport.fTeleportZ = 10.f; teleport.fTeleportY = 1.3f;
        queueTeleport();
        tests.Require(albionOwner.bPatternRootGrounded && albionOwner.fPositionY == 1.6f && albionOwner.fPositionX == 10.f && albionOwner.fPositionZ == 10.f &&
            std::abs(albionOwner.fPatternStageOriginY - (beforeTeleport->fPatternStageOriginY + 1.6f - beforeTeleport->fPositionY)) < .0001f &&
            albionOwner.fYawDegrees == beforeTeleport->fYawDegrees && albionOwner.iPatternStageRootLastTick == 3003u &&
            albionOwner.iActionStartTick == beforeTeleport->iActionStartTick && albionOwner.iPatternSequence == beforeTeleport->iPatternSequence,
            "Grounded teleport samples the actual floor and rebases vertical root without changing yaw or action clock");
        tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, teleportPattern, 3004u,
            room->m_ServerNavigation, room->m_ServerCollisionSystem, status) &&
            std::abs(albionOwner.fPositionX - 10.1f) < .0001f && std::abs(albionOwner.fPositionY - (1.6f + 2.f / 30.f)) < .0001f,
            "The next grounded root tick continues only its authored delta and cannot restore the former airborne offset");
        const auto groundedPose = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
        const auto groundedPreserved = [&] { return albionOwner.fPositionX == groundedPose->fPositionX &&
            albionOwner.fPositionY == groundedPose->fPositionY && albionOwner.fPositionZ == groundedPose->fPositionZ &&
            albionOwner.fPatternStageOriginX == groundedPose->fPatternStageOriginX &&
            albionOwner.fPatternStageOriginY == groundedPose->fPatternStageOriginY &&
            albionOwner.fPatternStageOriginZ == groundedPose->fPatternStageOriginZ &&
            albionOwner.fPatternStageRootGroundY == groundedPose->fPatternStageRootGroundY &&
            albionOwner.iPatternStageRootLastTick == groundedPose->iPatternStageRootLastTick &&
            albionOwner.bPatternRootGrounded == groundedPose->bPatternRootGrounded; };
        teleport.fTeleportY = 4.f; queueTeleport();
        tests.Require(groundedPreserved(), "Grounded destination deck mismatch preserves position and every root baseline");
        teleport.fTeleportY = 1.3f; teleport.fTeleportX = 6.f; teleport.fTeleportZ = 2.f; queueTeleport();
        tests.Require(groundedPreserved(), "Missing grounded destination navigation preserves position and every root baseline");
        teleport.fTeleportX = teleport.fTeleportZ = 10.f;
        room->m_ServerCollisionSystem.Set_BlockingBodies({ {10.f, 10.f, 1.f, 1.6f, 2.f, 999u} });
        queueTeleport();
        tests.Require(groundedPreserved(), "Ground-level blocking body rejects the complete teleport before any root update");
        room->m_ServerCollisionSystem.Set_BlockingBodies({});
        albionOwner.bPatternStageRootOriginCaptured = false; albionOwner.iPatternStageRootLastTick = 0u;
        albionOwner.iPatternStageFirstEvaluationTick = albionOwner.iActionStartTick = albionOwner.iPatternStartTick = 4000u;
        albionOwner.fPositionY = 8.f;
        queueTeleport(4000u);
        // This stage clock evaluates its first 1/30-second step on the entry tick.
        constexpr float firstRootUp = 2.f / 30.f;
        tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, teleportPattern, 4000u,
            room->m_ServerNavigation, room->m_ServerCollisionSystem, status) &&
            std::abs(albionOwner.fPositionY - (1.6f + firstRootUp)) < .0001f &&
            std::abs(albionOwner.fPatternStageOriginY - 1.6f) < .0001f,
            "A first root sample after grounded teleport captures the landed floor instead of the old airborne height");
        albionOwner.bPatternStageRootOriginCaptured = false; albionOwner.iPatternStageRootLastTick = 0u;
        albionOwner.fPositionY = 1.85f;
        tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, teleportPattern, 4000u,
            room->m_ServerNavigation, room->m_ServerCollisionSystem, status) &&
            std::abs(albionOwner.fPositionY - (1.6f + firstRootUp)) < .0001f &&
            std::abs(albionOwner.fPatternStageOriginY - 1.6f) < .0001f,
            "A later grounded stage captures the floor without inheriting the previous clip residual Up");
        albionOwner.PatternStageRootMotion.back().fUp = -2.f;
        tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, teleportPattern, 4001u,
            room->m_ServerNavigation, room->m_ServerCollisionSystem, status) && albionOwner.fPositionY >= 1.6f,
            "Grounded root continuation cannot send the boss below the resolved floor");
        room->m_KoukuSaydonBrain.Abort_Pattern(albionOwner, 4002u);
        tests.Require(!albionOwner.bPatternRootGrounded, "Explicit Stop/abort clears the pattern-local grounded continuation policy");

	}

	// Consume the published Showtime tail through the real stage clock, floor and teleport owner.
	{
		const auto savedBoss = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		const auto savedNavigation = room->m_ServerNavigation;
		CServerNavigation publishedNavigation;
		const auto* showtime = CKoukuSaydonBrain::Find_AnimationOnlyPattern(room->m_GameplayCatalog,
			"KAKULSAYDON_G1_PATTERN_35", status);
		const auto center = showtime ? std::find_if(showtime->MechanicTriggers.begin(), showtime->MechanicTriggers.end(),
			[](const auto& trigger) { return trigger.strTriggerId == "KAKULSAYDON_G1_PATTERN_35.logic.17"; }) :
			std::vector<BOSS_PATTERN_MECHANIC_TRIGGER>::const_iterator{};
		const bool ready = showtime && center != showtime->MechanicTriggers.end() &&
			center->eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED &&
			publishedNavigation.Load("LV_LUT_MIDNIGHTC_ED");
		tests.Require(ready, "Published Showtime final return uses its grounded occurrence and actual G3 navigation");
		if (ready)
		{
			room->m_ServerNavigation = std::move(publishedNavigation);
			room->m_ServerCollisionSystem.Set_BlockingBodies({});
			albionOwner.fPositionX = center->fTeleportX; albionOwner.fPositionZ = center->fTeleportZ;
			albionOwner.fPositionY = 8.f; albionOwner.fCollisionRadius = .1f;
			SERVER_NAV_POINT floor;
			const bool began = room->m_ServerNavigation.Sample_Position(center->fTeleportX, center->fTeleportZ, floor) &&
				room->m_KoukuSaydonBrain.Begin_Pattern(albionOwner, *showtime,
					room->m_GameplayCatalog.Get_ActiveRevision(), 10000u, status);
			tests.Require(began && !albionOwner.bPatternRootGrounded, "New pattern admission clears any former grounded root policy");
			if (began)
			{
				for (std::uint32_t tick = 10001u; tick <= 11676u; ++tick)
					(void)room->m_KoukuSaydonBrain.Update(albionOwner, room->m_GameplayCatalog, tick, status);
				tests.Require(albionOwner.strPatternStageId == "STAGE_11" &&
					CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, *showtime, 11677u,
						room->m_ServerNavigation, room->m_ServerCollisionSystem, status),
					"Showtime 55.869s return samples the actual final Stage 11 curve at its first fixed tick");
				room->m_PendingKoukuMechanicTriggers.push_back({ albionOwner.iNetEntityId, albionOwner.iPatternSequence, *center });
				room->Commit_KoukuMechanicTriggers(11677u);
				tests.Require(albionOwner.bPatternRootGrounded && std::abs(albionOwner.fPositionY - floor.y) < .00001f,
					"The actual published grounded trigger atomically lands on the G3 center floor");
				float minimumUp = 10000.f, maximumUp = -10000.f;
				bool sampled = true;
				for (std::uint32_t tick = 11678u; tick <= 11770u && !albionOwner.strPatternId.empty(); ++tick)
				{
					sampled = CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, *showtime, tick,
						room->m_ServerNavigation, room->m_ServerCollisionSystem, status) && sampled;
					SERVER_NAV_POINT ground;
					if (room->m_ServerNavigation.Sample_Position(albionOwner.fPositionX, albionOwner.fPositionZ, ground))
					{
						minimumUp = (std::min)(minimumUp, albionOwner.fPositionY - ground.y);
						maximumUp = (std::max)(maximumUp, albionOwner.fPositionY - ground.y);
					}
					else sampled = false;
					(void)room->m_KoukuSaydonBrain.Update(albionOwner, room->m_GameplayCatalog, tick, status);
				}
				SERVER_NAV_POINT endFloor;
				tests.Require(sampled && minimumUp >= -.00001f && maximumUp > 2.15f && maximumUp < 2.23f,
					"The complete published Showtime tail stays above the floor and retains the original 2.19m Stage 12 jump");
				tests.Require(room->m_ServerNavigation.Sample_Position(albionOwner.fPositionX, albionOwner.fPositionZ, endFloor) &&
					std::abs(albionOwner.fPositionY - endFloor.y) < .00001f && albionOwner.strPatternId.empty() && !albionOwner.bPatternRootGrounded,
					"Showtime completion leaves neither residual hover nor a grounded policy on the next pattern");
			}
		}
		albionOwner = *savedBoss;
		room->m_ServerNavigation = savedNavigation;
	}

	// Parse the actual supplemental row through catalog admission, preserving its previous generation on failure.
	std::vector<wchar_t> dataRootBuffer(32768u);
	const auto dataRootLength = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", dataRootBuffer.data(), static_cast<DWORD>(dataRootBuffer.size()));
	if (dataRootLength && dataRootLength < dataRootBuffer.size())
	{
		std::ifstream baselineInput(fs::path(dataRootBuffer.data()) / L"Gameplay/Gameplay.bootstrap", std::ios::binary);
		std::string baseline((std::istreambuf_iterator<char>(baselineInput)), std::istreambuf_iterator<char>());
		if (!baseline.empty() && baseline.back() != '\n') baseline += '\n';
		const std::string encounter = "ENCOUNTER_KAKULSAYDON_G1", patternId = "KAKULSAYDON_G1_SHOWTIME_CONTRACT";
		baseline += "PATTERN\t" + encounter + "\t" + patternId + "\t" + patternId + ".action\tAUDITION_ONLY\t0\t0\t0\t0\t0\t0\t0\t1\t1\tANY\tANY\t0\n";
		baseline += "PATTERNBOSS\t" + encounter + "\t" + patternId + "\tBOSS_KAKULSAYDON_G3_SAYDON\n";
		baseline += "PATTERNPOLICY\t" + encounter + "\t" + patternId + "\tNORMAL\t1\t1\tNONE\tNONE\n";
		baseline += "PATTERNSOURCE\t" + encounter + "\t" + patternId + "\t1\t0\t0\t0\t0\t0\t0\n";
		baseline += "PATTERNSTAGE\t" + encounter + "\t" + patternId + "\t0\tSTAGE_1\t" + patternId + ".stage.1\tACTIVE\t5000\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n";
		baseline += "PATTERNSTAGEBRANCH\t" + encounter + "\t" + patternId + "\t" + patternId + ".stage.1\tTIMEOUT\t-\n";
		baseline += "PATTERNTARGET\t" + patternId + "\tGATE3\tboss.kakulsaydon.g3.saydon\n";
		fs::create_directories(fixture / L"Gameplay");
		CGameplayCatalog parsedTargets;
		const auto loadSupplement = [&](const std::string& row) {
			auto bytes = baseline + row + '\n';
			const auto headerEnd = bytes.find('\n'), headerCount = bytes.rfind('\t', headerEnd);
			bytes.replace(headerCount + 1u, headerEnd - headerCount - 1u, std::to_string(std::count(bytes.begin(), bytes.end(), '\n') - 1u));
			{ std::ofstream output(fixture / L"Gameplay/Gameplay.bootstrap", std::ios::binary | std::ios::trunc); output.write(bytes.data(), bytes.size()); }
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", fixture.c_str());
			const bool loadedTargets = parsedTargets.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", dataRootBuffer.data());
			if (!loadedTargets && row.find("PATTERNSUMMONSPAWN") != std::string::npos) std::cout << "Summon admission: " << parsedTargets.Get_Status() << std::endl;
			return loadedTargets;
		};
		const auto loadTargetRow = [&](const std::string& fields) {
			return loadSupplement("PATTERNSHOWTIMETARGETS\t" + encounter + "\t" + patternId + "\ttest.showtime.duration\t0\t5000\t" + fields);
		};
		const bool parsedBoth = loadTargetRow("test.fixed\ttest.tracking\t4384\t2000\t0.5");
		const auto* parsedPattern = parsedBoth ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status) : nullptr;
		tests.Require(parsedPattern && parsedPattern->MechanicTriggers.size() == 1u &&
			parsedPattern->MechanicTriggers.front().eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS &&
			parsedPattern->MechanicTriggers.front().strFixedVisualId == "test.fixed" && parsedPattern->MechanicTriggers.front().strTrackingVisualId == "test.tracking",
			"The eleven-field Showtime row reaches the actual admitted Kouku pattern and visual consumers");
		tests.Require(loadTargetRow("-\ttest.tracking\t0\t2000\t0.5") && loadTargetRow("test.fixed\t-\t4384\t2000\t0.5"),
			"Catalog admission accepts independent fixed-only and tracking-only duration rows");
		const auto admittedRevision = parsedTargets.Get_ActiveRevision();
		bool rejectedInvalid = true;
		for (const std::string fields : { "-\t-\t0\t2000\t0.5", "test.fixed\ttest.tracking\t0\t2000\t0.5",
			"test.fixed\ttest.fixed\t4384\t2000\t0.5", "test.fixed\ttest.tracking\t4384\t0\t0.5",
			"test.fixed\ttest.tracking\t4384\t2000\tNaN", "test.fixed\ttest.tracking\t4384\t2000\t11" })
			rejectedInvalid = !loadTargetRow(fields) && parsedTargets.Get_ActiveRevision() == admittedRevision && rejectedInvalid;
		tests.Require(rejectedInvalid, "Invalid Showtime identity, lifetime, interval and speed preserve the previous admitted catalog");
		const std::string pursuitRow = "PATTERNPURSUITPROJECTILES\t" + encounter + "\t" + patternId +
			"\ttest.pursuit\t0\t901\ttest.heart\ttest.clover\ttest.diamond\ttest.spade\ttest.explosion\t15\t0.4\t2\t5000\t300\t0\t3";
		const bool parsedPursuit = loadSupplement(pursuitRow);
		const auto* pursuitPattern = parsedPursuit ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status) : nullptr;
		tests.Require(pursuitPattern && pursuitPattern->MechanicTriggers.size() == 1u &&
			pursuitPattern->MechanicTriggers.front().ProjectileVisualIds.size() == 4u &&
			pursuitPattern->MechanicTriggers.front().iProjectileCountPerWave == 3u,
			"Eighteen-field pursuit bootstrap reaches the admitted typed mechanic");
		tests.Require(loadSupplement(pursuitRow + "\t15"), "Nineteen-field pursuit bootstrap accepts an independent distance cap");
		pursuitPattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status);
		tests.Require(pursuitPattern && pursuitPattern->MechanicTriggers.front().fProjectileMaxDistanceM == 15.f,
			"Pursuit distance cap reaches the admitted Server mechanic");
		const auto pursuitRevision = parsedTargets.Get_ActiveRevision();
		for (const std::string distance : { "-1", "1000.1", "NaN", "wrong" })
			tests.Require(!loadSupplement(pursuitRow + "\t" + distance) && parsedTargets.Get_ActiveRevision() == pursuitRevision,
				"Invalid pursuit distance preserves the admitted catalog");
		std::string badPursuit = pursuitRow;
		badPursuit.replace(badPursuit.find("\t5000\t300"), 9u, "\t0\t300");
		tests.Require(!loadSupplement(badPursuit) && parsedTargets.Get_ActiveRevision() == pursuitRevision,
			"Unbounded repeated projectile publication preserves the previous catalog");
		const std::string randomOwner = "PATTERNSHOWTIMETARGETS	" + encounter + "	" + patternId + "	test.random.owner	0	5000	test.fixed	test.tracking	4384	2000	0.5";
		const auto randomRow = [&](const std::string& ownerId, const unsigned ordinal, const std::string& settings) {
			return "PATTERNSHOWTIMERANDOM	" + encounter + "	" + patternId + "	" + ownerId + "	" + std::to_string(ordinal) + "	test.random.a	5000	" + settings;
		};
		const auto randomZero = randomRow("test.random.owner", 0u, "500	2	0.2");
		const auto randomOne = randomRow("test.random.owner", 1u, "500	2	0.2");
		const bool randomAdmitted = loadSupplement(randomOwner + '\n' + randomZero + '\n' + randomOne);
		const auto* randomPattern = randomAdmitted ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status) : nullptr;
		tests.Require(randomPattern && randomPattern->MechanicTriggers.front().RandomVolleys.size() == 2u &&
			randomPattern->MechanicTriggers.front().RandomVolleys[0].strClientVisualId == randomPattern->MechanicTriggers.front().RandomVolleys[1].strClientVisualId &&
			randomPattern->MechanicTriggers.front().iRandomSpawnIntervalMs == 500u,
			"Supplemental random rows exact-join their duration owner and preserve ordered duplicate template IDs");
		const auto randomRevision = parsedTargets.Get_ActiveRevision();
		bool randomRejected = true;
		for (const auto& invalid : std::vector<std::string>{randomZero, randomOwner + '\n' + randomOne,
			randomOwner + '\n' + randomZero + '\n' + randomZero, randomOwner + '\n' + randomZero + '\n' + randomRow("test.random.owner",1u,"1000	2	0.2"),
			randomOwner + '\n' + randomRow("missing.owner",0u,"500	2	0.2"), randomOwner + '\n' + randomRow("test.random.owner",0u,"0	2	0.2"),
			randomOwner + '\n' + randomRow("test.random.owner",0u,"500	NaN	0.2"), randomOwner + '\n' + randomRow("test.random.owner",0u,"500	1001	0.2")})
			randomRejected = !loadSupplement(invalid) && parsedTargets.Get_ActiveRevision() == randomRevision && randomRejected;
		tests.Require(randomRejected, "Invalid random owner, ordinal, duplicate ordinal, drifted cadence or arena bounds preserve the admitted catalog");
		const std::string trackPrefix = "PATTERNMECHANICTRIGGER\t" + encounter + "\t" + patternId +
			"\ttest.track.duration\tBOSS_TRACK_TARGET\t612\t1079\t0\t";
		const std::string trackSuffix = "\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0";
		const bool parsedTrack = loadSupplement(trackPrefix + "0\t0\t0" + trackSuffix);
		const auto* trackPattern = parsedTrack ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status) : nullptr;
		tests.Require(trackPattern && trackPattern->MechanicTriggers.size() == 1u &&
			trackPattern->MechanicTriggers.front().eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET &&
			trackPattern->MechanicTriggers.front().iStartMs == 612u && trackPattern->MechanicTriggers.front().iDurationMs == 1079u,
			"The existing 25-field row admits a rotation-only duration with no visual templates");
		const auto trackRevision = parsedTargets.Get_ActiveRevision();
		tests.Require(!loadSupplement(trackPrefix + "1\t0\t0" + trackSuffix) && parsedTargets.Get_ActiveRevision() == trackRevision,
			"Rotation-only duration rejects unrelated teleport values and preserves the admitted generation");
		const std::string airBase="PATTERNMECHANICTRIGGER\t"+encounter+"\t"+patternId+"\tair.jump\tALBION_AIRBORNE\t0\t1000\t0\t0\t0\t0\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0";
		const std::string airPrefix="PATTERNALBIONAIRBORNE\t"+encounter+"\t"+patternId+"\tair.jump\t";
		tests.Require(loadSupplement(airBase+"\n"+airPrefix+"JUMP\t13.6788133\t200"),
			"Full catalog admission resolves the seven-field airborne supplement after its exact base occurrence");
		tests.Require(loadSupplement(airBase+"\n"+airPrefix+"JUMP\t11.1910783\t0"),
			"The unchanged airborne bootstrap row admits explicit zero-duration initial height");
		const auto airRevision=parsedTargets.Get_ActiveRevision();
		bool badAirRejected=true;
		for (const std::string fields : {"UNKNOWN\t13\t200","JUMP\t0\t200","JUMP\t13\t600001","CENTER\t1\t0","SLAM\t0\t0","APPEAR_PLAYER\t3\t0","JUMP\tNaN\t200"})
			badAirRejected=!loadSupplement(airBase+"\n"+airPrefix+fields) && parsedTargets.Get_ActiveRevision()==airRevision && badAirRejected;
		tests.Require(badAirRejected,"Unknown phase, invalid height/duration, missing prior jump and absent source descent reject without replacing the catalog");
		tests.Require(!loadSupplement(airBase) && !loadSupplement(airPrefix+"JUMP\t13\t200\n"+airBase) &&
			!loadSupplement(airBase+"\n"+airPrefix+"JUMP\t13\t200\n"+airPrefix+"JUMP\t13\t200") && parsedTargets.Get_ActiveRevision()==airRevision,
			"Missing, reversed and duplicate airborne supplements preserve the previous catalog generation");
        const std::string selectedFields="SELECT_PLAYER\t0\t0\tSELECT\ttest.selected.group\t2000";
        tests.Require(loadSupplement(airBase+"\n"+airPrefix+selectedFields), "Ten-field SELECT supplement admits a captured group while legacy rows stay valid");
        const auto* selectedPattern=CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets,patternId,status);
        tests.Require(selectedPattern && selectedPattern->MechanicTriggers.front().bCaptureAirborneTargetPosition &&
            selectedPattern->MechanicTriggers.front().strSelectedEffectVisualId=="test.selected.group" &&
            selectedPattern->MechanicTriggers.front().iSelectedEffectLifetimeMs==2000u, "Selected point policy and visual exact-join the existing occurrence");
        const auto selectedRevision=parsedTargets.Get_ActiveRevision();
        bool invalidSelectedRejected=true;
        for (const std::string fields : {"SELECT_PLAYER\t0\t0\tUNKNOWN\ttest.selected.group\t2000", "JUMP\t13\t200\tSELECT\ttest.selected.group\t2000",
            "SELECT_PLAYER\t0\t0\tSELECT\ttest.selected.group\t0", "SELECT_PLAYER\t0\t0\tSELECT\t\t2000", "SELECT_PLAYER\t0\t0\tSELECT\ttest.selected.group\t600001"})
            invalidSelectedRejected=!loadSupplement(airBase+"\n"+airPrefix+fields) && parsedTargets.Get_ActiveRevision()==selectedRevision && invalidSelectedRejected;
        tests.Require(invalidSelectedRejected, "Malformed selected policies, phase owners and lifetime pairs preserve the admitted catalog");
		const std::string teleportPrefix = "PATTERNMECHANICTRIGGER\t" + encounter + "\t" + patternId + "\ttest.teleport.xz\tBOSS_TELEPORT_XZ\t0\t1000\t0\t";
		const std::string teleportSuffix = "\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0";
		const bool parsedTeleport = loadSupplement(teleportPrefix + "2.57\t1.3\t952.27" + teleportSuffix);
		const auto* teleportedPattern = parsedTeleport ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status) : nullptr;
		tests.Require(teleportedPattern && teleportedPattern->MechanicTriggers.size() == 1u &&
			teleportedPattern->MechanicTriggers.front().eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_XZ &&
			teleportedPattern->MechanicTriggers.front().fTeleportY == 1.3f &&
			teleportedPattern->MechanicTriggers.front().fTeleportZ == 952.27f,
			"The existing 25-field mechanic row admits XZ teleport and preserves its authored reference Y");
		const auto teleportRevision = parsedTargets.Get_ActiveRevision();
		tests.Require(!loadSupplement(teleportPrefix + "100001\t1.3\t952.27" + teleportSuffix) &&
			parsedTargets.Get_ActiveRevision() == teleportRevision,
			"Out-of-bounds XZ teleport preserves the previous admitted catalog");
        auto groundedPrefix = teleportPrefix;
        groundedPrefix.replace(groundedPrefix.find("BOSS_TELEPORT_XZ"), std::string("BOSS_TELEPORT_XZ").size(), "BOSS_TELEPORT_GROUNDED");
        const bool parsedGrounded = loadSupplement(groundedPrefix + "-0.07\t1.32\t942.33" + teleportSuffix);
        const auto* groundedPattern = parsedGrounded ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status) : nullptr;
        tests.Require(groundedPattern && groundedPattern->MechanicTriggers.size() == 1u &&
            groundedPattern->MechanicTriggers.front().eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED &&
            groundedPattern->MechanicTriggers.front().fTeleportY == 1.32f,
            "Grounded teleport joins the existing 25-field mechanic row with its authored reference floor");
        const auto groundedRevision = parsedTargets.Get_ActiveRevision();
        tests.Require(!loadSupplement(groundedPrefix + "-0.07\t100001\t942.33" + teleportSuffix) &&
            parsedTargets.Get_ActiveRevision() == groundedRevision, "Invalid grounded height preserves the admitted product catalog");
		const std::string groupId="world.object.kouku.saydon.circus.split", groupCue="test.circus.group.world.1";
		const std::string groupRow="PATTERNWORLDSEQUENCE\t"+encounter+"\t"+patternId+"\t0\t"+groupId+"\t1\t0\t0\t0\tNONE\t0\t0\t0\t11500\t"+groupCue;
		const std::string groupPlacement="PATTERNWORLDPLACEMENT\t"+encounter+"\t"+patternId+"\t"+groupCue+"\t2\t1\t3\t0\t40\t0\t1\t1\t1";
		const bool groupLoaded=loadSupplement(groupRow+"\n"+groupPlacement);
		const auto* groupPattern=groupLoaded?CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets,patternId,status):nullptr;
		tests.Require(groupPattern && groupPattern->WorldSequences.size()==1u && groupPattern->WorldSequences[0].strInstanceId==groupId &&
			groupPattern->WorldSequences[0].iDurationMs==11500u && groupPattern->WorldSequences[0].Placement.has_value(),
			"One model-less group retains its stable ID, placement and full 63-ball presentation lifetime through actual catalog admission");
		if (groupPattern)
		{
			auto groupBoss=std::make_unique<SERVER_WORLD_ENTITY>(albionOwner); groupBoss->strPatternId=patternId; groupBoss->iPatternSequence=91u;
			KOUKUSAYDON_LOGIC_LEDGER groupLedger; KOUKUSAYDON_LOGIC_OUTPUT groupOutput;
			std::map<PLAYER_ID,SERVER_PLAYER> groupPlayers; std::vector<DAMAGE_EVENT> groupDamage;
			CKoukuSaydonLogicRuntime::Build(*groupPattern,*groupBoss,13000u,groupLedger);
			CKoukuSaydonLogicRuntime::Update(*groupBoss,*groupPattern,groupLedger,groupPlayers,parsedTargets,nullptr,13000u,groupDamage,groupOutput);
			tests.Require(groupOutput.WorldSequencePlays.size()==1u && groupOutput.WorldSequencePlays[0].strInstanceId==groupId &&
				groupOutput.WorldSequencePlays[0].strOccurrenceId==groupCue && groupOutput.WorldSequencePlays[0].iDurationMs==11500u &&
				groupOutput.WorldSequencePlays[0].iStartTick==13000u && groupOutput.WorldSequencePlays[0].Placement->fRotationYDegrees==40.f && groupDamage.empty(),
				"Existing Server Logic emits one owned group play at its authored clock without splitting the world cue or creating damage");
			groupOutput={}; CKoukuSaydonLogicRuntime::Update(*groupBoss,*groupPattern,groupLedger,groupPlayers,parsedTargets,nullptr,13001u,groupDamage,groupOutput);
			tests.Require(groupOutput.WorldSequencePlays.empty(),"Later fixed ticks do not replay or duplicate the group volley");
		}

		// The actual dependent-boss spawn transaction consumes the optional MAP pose and all ten rows.
		{
			const std::string childId = "KAKULSAYDON_G1_SUMMON_CONTRACT_CHILD";
			const auto start = baseline.find("PATTERN\t" + encounter + "\t" + patternId + "\t");
			std::string childRows = baseline.substr(start);
			for (std::size_t pos = 0u; (pos = childRows.find(patternId, pos)) != std::string::npos; pos += childId.size())
				childRows.replace(pos, patternId.size(), childId);
			const auto category = childRows.find("\tNORMAL\t");
			childRows.replace(category, 8u, "\tMECHANIC\t");
			{
				const std::string prefix = encounter + "\t" + patternId;
				const std::string tailRows = childRows + "PATTERNTIMELINE\t" + prefix + "\t11000\n" +
					"PATTERNWORLDSEQUENCE\t" + prefix + "\t6000\tworld.tail.contract\t1\t0\t0\t0\tNONE\t0\t0\t0\t3000\ttail.world.1\n" +
					"PATTERNMECHANICTRIGGER\t" + prefix + "\ttail.summon\tSUMMON_PATTERNS\t6000\t5000\t0\t0\t0\t0\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0\n" +
					"PATTERNSUMMONSPAWN\t" + prefix + "\ttail.summon\ttail.spawn\t" + childId + "\t4\t1\t4\t45\tMAP\n" +
					"PATTERNLOGIC\t" + prefix + "\t0\ttail.hit\tENTER_AREA\t6000\t3000\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\n" +
					"PATTERNLOGICREGION\t" + prefix + "\ttail.hit\t0\ttail.body\tBOSS_SPAWN\tCIRCLE\t0\t0\t0\t0\t1\t1\t1\t2\t45\tNONE\tNONE\n" +
					"PATTERNLOGICREGION\t" + prefix + "\ttail.hit\t1\ttail.follow\tBOSS_CURRENT\tCIRCLE\t0\t0\t0\t0\t1\t1\t1\t2\t45\tNONE\tNONE\n" +
					"PATTERNLOGICOUTCOME\t" + prefix + "\ttail.hit\tSUCCESS\t0\tMAX_HP_PERCENT_DAMAGE\t10\t0\t-";
				const bool tailLoaded = loadSupplement(tailRows);
				tests.Require(tailLoaded, "Catalog admits independent late Logic, World and Summon rows without extending the five-second Stage");
				if (!tailLoaded) std::cout << "Tail admission: " << parsedTargets.Get_Status() << '\n';
				if (tailLoaded) for (const bool explicitStop : {false, true})
				{
					auto tailRoom = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
					tailRoom->m_WorldEntities.clear(); tailRoom->m_ServerNavigation = nav;
					tailRoom->m_ServerCollisionSystem.Initialize({}, status);
					auto& run = tailRoom->m_KoukuSaydonPatternAudition;
					run.ePhase = CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
					run.iRoomAuditionEpoch = 99u;
					run.iOwnerSessionId = 77u; run.Request.Scope.strGateId = "GATE3";
					run.PinnedGameplayRevision = tailRoom->m_GameplayCatalog.Get_ActiveRevision();
					run.pProductGeneration = std::make_shared<CGameplayCatalog>(parsedTargets);
					run.iPinnedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(parsedTargets);
					const auto* definition = CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status);
					WORLD_BOOTSTRAP_PLACEMENT place;
					place.eKind = WORLD_BOOTSTRAP_KIND::BOSS; place.strPlacementId = "boss.kakulsaydon.g3.saydon";
					place.strArchetypeId = "BOSS_KAKULSAYDON_G3_SAYDON"; place.strEncounterId = encounter;
					place.fPositionX = place.fPositionZ = 8.f; place.fPositionY = 1.f;
					SERVER_WORLD_ENTITY primary;
					const bool ready = definition && tailRoom->Build_WorldEntity(place, 900u, primary) &&
						tailRoom->m_KoukuSaydonBrain.Begin_Pattern(primary, *definition, run.PinnedGameplayRevision, 10000u, status);
					tests.Require(ready, "Independent row fixture admits the real boss and pinned catalog");
					if (!ready) continue;
					tailRoom->m_WorldEntities.push_back(primary); tailRoom->m_iNextNetEntityId = 1000u;
					auto& member = run.Members.emplace_back(); member.strMemberId = "tail.member";
					member.iBossEntityId = 900u; member.iPatternSequence = primary.iPatternSequence;
					member.PatternIds = {patternId}; member.ePhase = CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
					CKoukuSaydonLogicRuntime::Build(*definition, primary, 10000u, member.LogicLedger);
					auto& player = tailRoom->m_Players[1u]; player.iPlayerId = 1u; player.iNetEntityId = 901u;
					player.iMaximumHp = player.iCurrentHp = 100u; player.isCombatReady = true;
					player.fPositionX = player.fPositionZ = 8.f; player.fPositionY = 1.f;
					auto& follower = tailRoom->m_Players[2u]; follower = player; follower.iPlayerId = 2u; follower.iNetEntityId = 902u;
					follower.fPositionX = follower.fPositionZ = 14.f;
					tailRoom->m_iServerTick = 10150u;
					tests.Require(tailRoom->Update_KoukuSaydonBoss(tailRoom->m_WorldEntities.front(), 10150u) &&
						tailRoom->m_WorldEntities.front().strPatternId.empty() && run.Tails.size() == 1u &&
						run.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE,
						"The five-second Stage finishes and emits natural completion before the eleven-second row lifetime");
					if (run.Tails.empty()) continue;
					// A newer body occurrence must not steal the old row origin, identity or ledger.
					auto& newer = tailRoom->m_WorldEntities.front(); newer.strPatternId = childId;
					newer.iPatternSequence = 77u; newer.fPositionX = newer.fPositionZ = 14.f;
					tailRoom->Update_KoukuPatternTails(10180u); tailRoom->Commit_KoukuMechanicTriggers(10180u);

					tests.Require(player.iCurrentHp == 90u && follower.iCurrentHp == 90u && run.WorldPlays.size() == 1u &&
						run.WorldPlays.front().iPatternSequence == primary.iPatternSequence &&
						run.WorldPlays.front().iStartTick == 10180u && tailRoom->m_WorldEntities.size() == 2u &&
						tailRoom->m_WorldEntities.front().iPatternSequence == 77u && tailRoom->m_WorldEntities.front().fPositionX == 14.f,
						"Late Logic preserves BOSS_SPAWN and follows BOSS_CURRENT; World keeps its clock and Summon leaves the newer Pattern intact");
					tailRoom->Update_KoukuPatternTails(10181u); tailRoom->Commit_KoukuMechanicTriggers(10181u);
					tailRoom->Update_KoukuGazeClones(10181u);
					tests.Require(player.iCurrentHp == 90u && follower.iCurrentHp == 90u && run.WorldPlays.size() == 1u && tailRoom->m_WorldEntities.size() == 2u,
						"Later ticks neither duplicate row outputs nor remove the summon when the primary sequence has advanced");
					{
						const auto oldPin = run.PinnedGameplayRevision;
						const bool newGeneration = tailRoom->Stage_GameplayGeneration(1u, oldPin,
							std::make_shared<CGameplayCatalog>(parsedTargets), status) && tailRoom->Commit_GameplayGeneration(1u);
						for (auto& entity : tailRoom->m_WorldEntities) entity.PinnedDefinitionRevision = tailRoom->m_GameplayCatalog.Get_ActiveRevision();
						std::vector<GameplayDataRevision> pins;
						const bool pinned = tailRoom->Build_RequiredPinnedGameplayRevisions(pins);
						tailRoom->m_GameplayCatalog.Collect_Garbage(pins);
						tests.Require(newGeneration && pinned && std::find(pins.begin(), pins.end(), oldPin) != pins.end() &&
							tailRoom->m_GameplayCatalog.Resolve(oldPin) != nullptr,
							"A naturally completed run pins its row generation through catalog replacement and garbage collection");
					}
					{
						auto& oldOwner = *run.Tails.front().pOwner;
						auto& currentBody = tailRoom->m_WorldEntities.front();
						(void)CBossCombatRuntime::Set_Flag(oldOwner.BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, true);
						const bool countered = CBossCombatRuntime::Try_TriggerCounter(currentBody, 10182u);
						tests.Require(countered && oldOwner.BossCombat.PendingOutcomes.size() == 1u &&
							oldOwner.BossCombat.PendingOutcomes.front().iPatternSequence == primary.iPatternSequence &&
							currentBody.BossCombat.PendingOutcomes.empty() && currentBody.iPatternSequence == 77u &&
							!CBossCombatRuntime::Try_TriggerCounter(currentBody, 10182u),
							"A retained counter flag consumes once and publishes only to its born Pattern, not the newer actor");
						oldOwner.bKoukuShieldActive = true; oldOwner.fKoukuShieldArcDegrees = 60.f; oldOwner.fYawDegrees = 0.f;
						tests.Require(CKoukuSaydonLogicRuntime::Is_ShieldReflected(currentBody, 14.f, 18.f) &&
							!CKoukuSaydonLogicRuntime::Is_ShieldReflected(currentBody, 14.f, 10.f),
							"The actual damage consumer sees the retained shield at its live BOSS_CURRENT root");
						oldOwner.bKoukuShieldActive = false;
					}

					auto support = scheduled; support.iStartTick = 10150u; support.iEndTick = 10330u; support.strMemberId = "tail.member";
					run.SupportSchedule.push_back(support);
					tailRoom->Stop_KoukuWorldOwner("tail.member", true);
					tests.Require(run.Tails.size() == 1u && run.WorldPlays.size() == 1u && run.SupportSchedule.size() == 1u,
						"FINISH_OWNER preserves independent Logic, World and navigation support rows");
					if (!explicitStop)
					{
						auto& raid = tailRoom->m_KoukuRaid;
						raid.State.ePhase = KOUKUSAYDON_RAID_PHASE::COMBAT; raid.State.strGateId = "GATE3";
						raid.iOwnerSessionId = run.iOwnerSessionId; raid.iAuditionEpoch = run.iRoomAuditionEpoch;
						auto& body = tailRoom->m_WorldEntities.front(); body.strPatternId.clear(); body.strActionId.clear();
						C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request;
						request.iRequestSequence = 1u; request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
						request.strPatternId = childId; request.Scope.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
						request.Scope.strEncounterId = encounter; request.Scope.strGateId = "GATE3";
						request.Scope.strBossPlacementId = "boss.kakulsaydon.g3.saydon";
						request.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G3_SAYDON";
						request.Scope.ExpectedGameplayRevision = run.PinnedGameplayRevision;
						request.Scope.iExpectedSourceRevision = run.iPinnedSourceRevision;
						S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result;
						tailRoom->m_iServerTick = 10182u;
						const auto verdict = tailRoom->Evaluate_KoukuSaydonPatternAudition(77u, request, result, true);
						if (verdict != KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED) std::cout << "Continuation admission: " << result.strReason << '\n';
						tailRoom->Prepare_KoukuAuditionTick(10183u);
						tests.Require(verdict == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED && run.iRoomAuditionEpoch == 99u &&
							run.Tails.size() == 1u && run.WorldPlays.size() == 1u && run.SupportSchedule.size() == 1u &&
							tailRoom->m_WorldEntities.front().strPatternId == childId,
							"Automatic next Flow Entry starts the next actor Pattern in the same epoch while prior rows remain alive");
					}
					if (explicitStop)
					{
						tailRoom->Clear_KoukuSaydonPatternAudition(); tailRoom->Update_KoukuGazeClones(10182u);
						tests.Require(run.Tails.empty() && run.WorldPlays.empty() && run.SupportSchedule.empty() && tailRoom->m_WorldEntities.size() == 1u,
							"Explicit Stop cancels tails, pending supports and already spawned summons");
					}
					else
					{
						tailRoom->Update_KoukuPatternTails(10330u); tailRoom->Update_KoukuGazeClones(10330u);
						tailRoom->Update_KoukuPatternTails(10331u);
						tests.Require(run.Tails.empty() && run.WorldPlays.empty() && tailRoom->m_WorldEntities.size() == 1u,
							"Each row and summon expires at its own absolute deadline after the Stage has completed");
					}
				}
			}
			const std::string triggerRow = "PATTERNMECHANICTRIGGER\t" + encounter + "\t" + patternId +
				"\ttest.summon\tSUMMON_PATTERNS\t0\t5000\t0\t0\t0\t0\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0\n";
			const auto spawnRows = [&](const unsigned count, const std::string& anchor) {
				std::string result = childRows + triggerRow;
				for (unsigned index = 0u; index < count; ++index)
					result += "PATTERNSUMMONSPAWN\t" + encounter + "\t" + patternId + "\ttest.summon\ttest.spawn." +
						std::to_string(index) + "\t" + childId + "\t" + (index < 5u ? "4" : "12") + "\t1\t" +
						std::to_string(4u + 2u * (index % 5u)) + "\t45" + anchor + "\n";
				if (!result.empty() && result.back() == '\n') result.pop_back();
				return result;
			};
			tests.Require(loadSupplement(spawnRows(16u, "\tMAP")), "Catalog admits sixteen bounded MAP Pattern spawn rows");
			const auto beforeInvalidSummon = parsedTargets.Get_ActiveRevision();
			tests.Require(!loadSupplement(spawnRows(17u, "\tMAP")) && parsedTargets.Get_ActiveRevision() == beforeInvalidSummon,
				"A seventeenth Summon spawn preserves the admitted catalog");
			tests.Require(!loadSupplement(spawnRows(10u, "\tBAD")) && parsedTargets.Get_ActiveRevision() == beforeInvalidSummon,
				"An invalid spawn anchor preserves the admitted catalog");
			tests.Require(loadSupplement(spawnRows(10u, "")), "Legacy ten-field summon rows retain the BOSS anchor default");
			tests.Require(loadSupplement(spawnRows(10u, "\tMAP")), "Catalog admits ten independently placed MAP actors");
			auto summonRoom = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
			summonRoom->m_WorldEntities.clear(); summonRoom->m_ServerNavigation = nav;
			summonRoom->m_ServerCollisionSystem.Initialize({}, status); summonRoom->m_ServerCollisionSystem.Set_BlockingBodies({});
			auto& run = summonRoom->m_KoukuSaydonPatternAudition;
			run.ePhase = CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
			run.PinnedGameplayRevision = summonRoom->m_GameplayCatalog.Get_ActiveRevision();
			run.pProductGeneration = std::make_shared<CGameplayCatalog>(parsedTargets);
			run.iPinnedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(parsedTargets);
			const auto* parent = CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status);
			WORLD_BOOTSTRAP_PLACEMENT placement;
			placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS; placement.strPlacementId = "boss.kakulsaydon.g3.saydon";
			placement.strArchetypeId = "BOSS_KAKULSAYDON_G3_SAYDON"; placement.strEncounterId = encounter;
			placement.fPositionX = placement.fPositionZ = 8.f; placement.fPositionY = 1.f; placement.fYawDegrees = 120.f;
			SERVER_WORLD_ENTITY primary;
			const bool ready = parent && !parent->MechanicTriggers.empty() && summonRoom->Build_WorldEntity(placement, 900u, primary) &&
				summonRoom->m_KoukuSaydonBrain.Begin_Pattern(primary, *parent, run.PinnedGameplayRevision, 13000u, status);
			tests.Require(ready, "The real primary boss admits the Summon parent in its pinned Product");
			if (ready)
			{
				summonRoom->m_WorldEntities.push_back(primary); summonRoom->m_iNextNetEntityId = 1000u;
				auto trigger = parent->MechanicTriggers.front();
				const auto submit = [&](const BOSS_PATTERN_MECHANIC_TRIGGER& value) {
					summonRoom->m_PendingKoukuMechanicTriggers.push_back({900u,primary.iPatternSequence,value});
					summonRoom->Commit_KoukuMechanicTriggers(13000u);
				};
				auto bad = trigger; bad.PatternSpawns.back().PositionOffset = {6.f,1.f,2.f}; submit(bad);
				tests.Require(summonRoom->m_WorldEntities.size() == 1u && summonRoom->m_iNextNetEntityId == 1000u,
					"An invalid tenth navigation point rolls back all ten staged summon actors and IDs");
				bad = trigger; bad.PatternSpawns.back().PositionOffset[1] = 10.f; submit(bad);
				tests.Require(summonRoom->m_WorldEntities.size() == 1u && summonRoom->m_iNextNetEntityId == 1000u,
					"A MAP point on the wrong navigation floor rolls back the entire formation");
				submit(trigger);
				bool poses = summonRoom->m_WorldEntities.size() == 11u;
				for (const auto& actor : summonRoom->m_WorldEntities) if (actor.bKoukuSummonClone)
					poses = poses && actor.iOwnerBossNetEntityId == 900u && actor.strPatternId == childId && actor.fYawDegrees == 45.f &&
						(actor.fPositionX == 4.f || actor.fPositionX == 12.f) && actor.fPositionY == 1.f && actor.KoukuSummonStartedTriggers.empty();
				tests.Require(poses && summonRoom->m_iNextNetEntityId == 1010u,
					"Ten MAP actors commit independent Patterns and absolute poses despite the primary yaw");
				summonRoom->Update_KoukuGazeClones(13001u);
				tests.Require(summonRoom->m_WorldEntities.size() == 11u, "The next clone simulation tick preserves all ten active children");
				summonRoom->m_WorldEntities.front().strPatternId.clear(); summonRoom->Update_KoukuGazeClones(13002u);
				tests.Require(summonRoom->m_WorldEntities.size() == 1u, "Stopping the parent removes all ten owned actors together");
			}
		}
	}
	else tests.Require(false, "Showtime catalog admission requires the configured isolated Server data root");
#endif
	std::error_code cleanupError; fs::remove_all(fixture, cleanupError);
	std::cout << "failures : " << tests.failures << std::endl;
	return tests.failures ? 1 : 0;
}
