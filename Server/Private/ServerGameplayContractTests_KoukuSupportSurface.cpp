#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameplayCatalog.h"
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
	tests.Require(room->m_WorldEntities[2].fPositionY == 8.63f && room->m_WorldEntities[3].fPositionY == 8.63f,
		"An active WORLD floor preserves forced motion and authored root motion vertical authority");
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
	tests.Require(room->m_WorldEntities[2].fPositionY == 8.63f && room->m_WorldEntities[3].fPositionY == 8.63f,
		"Removing the WORLD floor preserves both scripted airborne motion authorities");

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
#endif
	std::error_code cleanupError; fs::remove_all(fixture, cleanupError);
	std::cout << "failures : " << tests.failures << std::endl;
	return tests.failures ? 1 : 0;
}
