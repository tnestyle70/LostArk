#include "ServerGameplayContractTests_Runner.h"
#include "KoukuSaydonLogicRuntime.h"
#include "ServerGameplayContractTests.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerCombatHitRuntime.h"
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



int LostArk::Server::Run_ServerCardMazeContractTests()
{
	using namespace LostArk::Shared;
	using Maze = CKoukuCardMazeRuntime;
	TESTS tests;
	CWorldBootstrap bootstrap;
	CServerNavigation navigation;
	const bool ready = bootstrap.Load(WORLD_ID::KAKULSAYDON_ARENA) && navigation.Load("LV_LUT_MIDNIGHTC_ED");
	tests.Require(ready && bootstrap.Get_CardMazeLanes().size() == 36u, "Load published maze and all 36 authoritative lanes");
	if (!ready) { std::cout << bootstrap.Get_Status() << '\n'; return 1; }
	SERVER_NAV_POINT returnGround{};
	tests.Require(navigation.Sample_Position(3.38f, 323.92f, returnGround) && std::abs(returnGround.y - 10.56f) < 1.f,
		"Gate 2 default return destination is on published navigation");
#ifdef _DEBUG
	{
		auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
		tests.Require(room->Is_Ready(), "Actual maze room bootstrap and profiles load");
		if (!room->Is_Ready()) { std::cout << room->Get_Status() << '\n'; return 1; }
		auto& entrant = room->m_Players[1u];
		entrant.iPlayerId = 1u; entrant.iNetEntityId = 101u; entrant.iSessionId = 11u;
		entrant.iCurrentHp = entrant.iMaximumHp = 50000u;
		entrant.fPositionX = .09f; entrant.fPositionY = -.01f; entrant.fPositionZ = 1351.48f;
		entrant.fYawDegrees = 225.f; // Actual entry is inside the box, facing away from its center.
		room->m_PlayerIdBySessionId[11u] = 1u;
		C2S_DEBUG_SET_KOUKU_HUD_MODE mode{};
		mode.eWorldId = WORLD_ID::KAKULSAYDON_ARENA; mode.iRequestSequence = 1u; mode.eMode = KOUKU_HUD_MODE::MAZE;
		const auto accepted = room->Apply_DebugKoukuHudMode(entrant, mode);
		room->Update_KoukuPlayerModes(1u);
		tests.Require(accepted.eResult == DEBUG_KOUKU_HUD_MODE_RESULT::ACCEPTED &&
			entrant.eMadnessForm == PLAYER_MADNESS_FORM::NORMAL && entrant.eKoukuHudMode == KOUKU_HUD_MODE::MAZE,
			"MAZE entry keeps the selected normal player body and enables Q slot");
		C2S_INTERACTION_SLOT press{};
		press.eWorldId = WORLD_ID::KAKULSAYDON_ARENA; press.iRequestSequence = 1u;
		press.eSlot = static_cast<INTERACTION_SLOT>(0u);
		room->Handle_InteractionSlot(11u, press);
		tests.Require(entrant.eAction == PLAYER_ACTION_STATE::INTERACTION, "Q reaches the actual interaction action handler");
		// Drive the same fixed-tick player update that lands the hammer once.
		const auto hitTick = entrant.iActionStartTick + Maze::HAMMER_HIT_TICK_OFFSET;
		room->m_iServerTick = hitTick - 1u;
		room->Update_Players(1.f / 30.f);
		tests.Require(room->m_KoukuCardMaze.Get_Phase() == Maze::PHASE::HUNTING &&
			room->m_KoukuCardMaze.Get_Targets().size() == 1u && (entrant.CardMaze.flags & 1u),
			"Center Q hit facing away starts solo telescope and actually spawns a target");
		if (room->m_KoukuCardMaze.Get_Phase() != Maze::PHASE::HUNTING) std::cout << room->Get_Status() << '\n';
	}
#endif
	std::map<PLAYER_ID, SERVER_PLAYER> players;
	for (PLAYER_ID id = 1u; id <= 4u; ++id)
	{
		auto& p = players[id]; p.iPlayerId = id; p.iNetEntityId = 100u + id; p.iCurrentHp = 50000u;
		p.fPositionX = Maze::CENTER_X; p.fPositionY = -.01f; p.fPositionZ = Maze::CENTER_Z;
	}
	{
		CGameplayCatalog catalog;
		BOSS_PATTERN_DEFINITION pattern{};
		pattern.strPatternId = "cardmaze.entry.contract";
		CServerCollisionSystem collision;
		std::string collisionStatus;
		tests.Require(collision.Initialize(bootstrap.Get_Placements(), collisionStatus), "Maze entry loads the published collision layer");
		BOSS_PATTERN_MECHANIC_TRIGGER enter{};
		enter.strTriggerId = "entry"; enter.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_ENTER;
		enter.iStartMs = 5000u; enter.iDurationMs = 200u;
		enter.fTeleportX = Maze::CENTER_X; enter.fTeleportY = -.01f; enter.fTeleportZ = Maze::CENTER_Z;
		pattern.MechanicTriggers.push_back(enter); // File order is deliberately not time order.
		for (std::uint32_t index = 0u; index < 4u; ++index)
		{
			auto hide = enter; hide.strTriggerId = "hide." + std::to_string(index);
			hide.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_HIDE_NEXT;
			hide.iStartMs = (index + 1u) * 1000u;
			pattern.MechanicTriggers.push_back(hide);
		}
		SERVER_WORLD_ENTITY boss{}; boss.iPatternSequence = 1u; boss.iCurrentHp = boss.iMaximumHp = 1000u;
		boss.strPatternId = pattern.strPatternId;
		KOUKUSAYDON_LOGIC_LEDGER ledger;
		auto entrants = players;
		for (auto& [id, player] : entrants) { player.fPositionX = float(id); player.fPositionZ = 324.f; player.isCombatReady = true; }
		CKoukuSaydonLogicRuntime::Build(pattern, boss, 10u, ledger);
		std::vector<DAMAGE_EVENT> damage;
		KOUKUSAYDON_LOGIC_OUTPUT output;
		const auto update = [&](std::uint32_t elapsedMs, const CServerNavigation* nav) {
			output = {};
			CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, entrants, catalog, nullptr,
				10u + CKoukuSaydonLogicRuntime::Ticks_FromMs(elapsedMs), damage, output, nav, &collision);
		};
		update(1000u, &navigation);
		tests.Require((entrants[4u].CardMaze.flags & CARD_MAZE_ENTRY_HIDDEN) && entrants[3u].CardMaze.flags == 0u,
			"First maze cue hides the rightmost player");
		entrants[1u].fPositionX = 99.f;
		update(2000u, &navigation);
		tests.Require((entrants[3u].CardMaze.flags & CARD_MAZE_ENTRY_HIDDEN) && entrants[1u].CardMaze.flags == 0u,
			"Movement after first hide cannot reorder the captured roster");
		update(5000u, &navigation);
		tests.Require(std::all_of(entrants.begin(), entrants.end(), [&](const auto& row) {
			return !(row.second.CardMaze.flags & CARD_MAZE_ENTRY_HIDDEN) && row.second.fPositionX == Maze::CENTER_X &&
				row.second.fPositionZ == Maze::CENTER_Z && row.second.eKoukuAreaHudMode == KOUKU_HUD_MODE::MAZE;
		}), "Late tick processes remaining hides before one transactional maze entry");
		CKoukuSaydonLogicRuntime::Update_PlayerModes(entrants, &ledger, nullptr, 160u);
		tests.Require(entrants[1u].eKoukuHudMode == KOUKU_HUD_MODE::MAZE && entrants[1u].eMadnessForm == PLAYER_MADNESS_FORM::NORMAL,
			"Card maze entry enables Q while preserving the normal class body");
		entrants = players;
		for (auto& [id, player] : entrants) { player.fPositionX = float(id); player.fPositionZ = 324.f; player.isCombatReady = true; }
		CKoukuSaydonLogicRuntime::Build(pattern, boss, 10u, ledger);
		update(5000u, nullptr);
		tests.Require(std::all_of(entrants.begin(), entrants.end(), [](const auto& row) {
			return row.second.fPositionZ == 324.f && !(row.second.CardMaze.flags & CARD_MAZE_ENTRY_HIDDEN);
		}) && output.strStatus.find("preserved") != std::string::npos,
			"Missing maze navigation preserves every position and reveals the roster");
		CKoukuSaydonLogicRuntime::Build(pattern, boss, 10u, ledger);
		update(1000u, &navigation);
		entrants[2u].TriggerMove.isActive = true;
		update(5000u, &navigation);
		tests.Require(std::all_of(entrants.begin(), entrants.end(), [](const auto& row) {
			return row.second.fPositionZ == 324.f && !(row.second.CardMaze.flags & CARD_MAZE_ENTRY_HIDDEN);
		}), "One busy participant prevents partial entry and reveals everyone");
		entrants[2u].TriggerMove = {};
		CKoukuSaydonLogicRuntime::Build(pattern, boss, 10u, ledger);
		update(1000u, &navigation);
		CKoukuSaydonLogicRuntime::Discard(ledger, entrants, &boss);
		tests.Require(!(entrants[4u].CardMaze.flags & CARD_MAZE_ENTRY_HIDDEN), "Stopping the cutscene reveals hidden participants");
	}
	Maze maze;
	std::vector<Maze::SPAWN_REQUEST> spawns;
	std::string status;
	std::map<PLAYER_ID, SERVER_PLAYER> solo;
	solo.emplace(1u, players[1u]);
#ifdef _DEBUG
	const bool soloPlanned = maze.Plan(1u, solo, navigation, 99u, spawns, status);
	tests.Require(soloPlanned && spawns.size() == 1u, "Debug solo claim places one random suit target");
	if (!soloPlanned || spawns.size() != 1u) return 1;
	const auto soloSpawn = spawns.front();
	maze.Commit(solo);
	tests.Require(maze.Is_SoloHunter(1u) && solo[1u].eCardMazeRole == CARD_MAZE_ROLE::HUNTER &&
		solo[1u].eCardMazeSuit == soloSpawn.eSuit && soloSpawn.eSuit != MECHANIC_CARD_SYMBOL::NONE &&
		solo[1u].iCardMazeKillTarget == 3u && (solo[1u].CardMaze.flags & 1u),
		"Solo receives floor-mark hunter suit and initial telescope overhead together");
	tests.Require(navigation.Is_PointWalkableExact(soloSpawn.fPositionX, soloSpawn.fPositionZ) &&
		!Maze::In_SafeZone(soloSpawn.fPositionX, soloSpawn.fPositionZ), "Solo target is on a corridor outside central immunity");
	tests.Require(maze.Toggle_Telescope(solo[1u]) && !(solo[1u].CardMaze.flags & 1u) &&
		maze.Toggle_Telescope(solo[1u]), "Solo may toggle telescope without losing the hunter role");
	for (unsigned kill = 0u; kill < 3u; ++kill)
	{
		SERVER_WORLD_ENTITY target{}; target.iNetEntityId = 900u + kill;
		maze.Register_Target(target.iNetEntityId, soloSpawn.eSuit);
		tests.Require(maze.Can_Hit(solo[1u], target.iNetEntityId), "Solo may strike its own matching target");
		const auto outcome = maze.On_TargetHit(solo[1u], target, true);
		tests.Require(outcome.bStartMarch == (kill == 0u) && outcome.bKillCounted &&
			outcome.bHunterComplete == (kill == 2u), "Solo first hit alone starts Seto march, third kill unlocks exit");
		maze.Retire_Target(target.iNetEntityId);
	}
	tests.Require(!maze.All_LivingCentral(solo), "Solo still must take an exit before completion");
	maze.Reset_Progress(solo[1u]);
	tests.Require(solo[1u].iCardMazeKills == 0u && (solo[1u].CardMaze.flags & 1u),
		"Solo Seto reset clears progress without removing overhead");
	maze.Reset(solo);
	tests.Require(!maze.Is_SoloHunter(1u) && solo[1u].eCardMazeRole == CARD_MAZE_ROLE::NONE,
		"Solo reset removes combined role and camera state");
#else
	tests.Require(!maze.Plan(1u, solo, navigation, 99u, spawns, status) && spawns.empty(),
		"Release does not enable Debug solo testing");
#endif
	auto deadParty = players;
	for (PLAYER_ID id = 2u; id <= 4u; ++id) deadParty[id].iCurrentHp = 0u;
	tests.Require(!maze.Plan(1u, deadParty, navigation, 99u, spawns, status),
		"Other players dying does not silently turn multiplayer into solo testing");
	const bool planned = maze.Plan(1u, players, navigation, 100u, spawns, status);
	tests.Require(planned && spawns.size() == 3u, "Four players receive three distinct suits with only one target per suit");
	if (!planned) { std::cout << status << '\n'; return 1; }
	maze.Commit(players);
	tests.Require(!maze.Is_SoloHunter(1u) && !maze.Is_SoloHunter(2u), "Multiplayer has no solo movement exemption");
	tests.Require(players[1u].eCardMazeSuit == MECHANIC_CARD_SYMBOL::NONE && players[1u].iCardMazeKillTarget == 0u,
		"Telescope owner has no suit or kill target");
	tests.Require(players[1u].CardMaze.flags == 1u && maze.Toggle_Telescope(players[1u]) &&
		players[1u].CardMaze.flags == 0u && maze.Toggle_Telescope(players[1u]), "Initial telescope owner can independently toggle overhead");
	tests.Require(!maze.Toggle_Telescope(players[2u]), "Unescaped hunter cannot claim overhead");
	std::set<MECHANIC_CARD_SYMBOL> suits;
	for (PLAYER_ID id = 2u; id <= 4u; ++id) suits.insert(players[id].eCardMazeSuit);
	tests.Require(suits.size() == 3u && !suits.contains(MECHANIC_CARD_SYMBOL::NONE), "Assigned hunter suits are distinct and valid");
	for (PLAYER_ID id = 2u; id <= 4u; ++id)
	for (unsigned kill = 0u; kill < 3u; ++kill)
	{
		SERVER_WORLD_ENTITY target{}; target.iNetEntityId = 1000u + id * 10u + kill;
		target.eKind = WORLD_BOOTSTRAP_KIND::MONSTER; target.strSpawnGroupId = Maze::SPAWN_GROUP_TAG;
		target.iCurrentHp = 300u; target.iDefense = 99999u;
		maze.Register_Target(target.iNetEntityId, players[id].eCardMazeSuit);
		tests.Require(maze.Can_Hit(players[id], target.iNetEntityId) && !maze.Can_Hit(players[1u], target.iNetEntityId),
			"Only the matching hunter can damage its target");
		SERVER_PLAYER_TO_WORLD_HIT hit{}; hit.iSourcePlayerId = id; hit.iSkillId = Maze::HAMMER_SKILL_ID;
		hit.iRawDamage = Maze::HAMMER_RAW_DAMAGE; hit.iServerTick = 120u;
		std::vector<DAMAGE_EVENT> damage;
		tests.Require(CServerCombatHitRuntime::Apply_PlayerToWorld(target, hit, damage) == SERVER_COMBAT_HIT_RESULT::KILLED &&
			target.iCurrentHp == 0u, "A single admitted maze hammer kills 300 HP regardless of defense");
		const auto outcome = maze.On_TargetHit(players[id], target, true);
		tests.Require(outcome.bKillCounted && players[id].iCardMazeKills == kill + 1u &&
			!maze.On_TargetHit(players[id], target, true).bKillCounted, "Each target increments once without duplicate credit");
		maze.Retire_Target(target.iNetEntityId);
	}
	tests.Require(maze.Get_Phase() == Maze::PHASE::HUNTING && !maze.All_LivingCentral(players),
		"Three stacks alone never complete the maze even when everyone stands centrally");
	players[2u].CardMaze.flags |= 4u; players[2u].CardMaze.exitX = 10.f;
	maze.Reset_Progress(players[2u]);
	tests.Require(players[2u].iCardMazeKills == 0u && !(players[2u].CardMaze.flags & 4u) && players[2u].CardMaze.exitX == 0.f,
		"Seto reset revokes both stacks and the personal exit");
	tests.Require(Maze::In_SafeZone(Maze::CENTER_X + 5.f, Maze::CENTER_Z) &&
		!Maze::In_SafeZone(Maze::CENTER_X + 5.1f, Maze::CENTER_Z), "Central immunity has a bounded five metre radius");
	for (PLAYER_ID id = 2u; id <= 4u; ++id) maze.Mark_Escaped(players[id]);
	tests.Require(maze.All_LivingCentral(players) && maze.Toggle_Telescope(players[2u]) && maze.Toggle_Telescope(players[3u]) &&
		(players[1u].CardMaze.flags & 1u) && (players[2u].CardMaze.flags & 1u) && (players[3u].CardMaze.flags & 1u),
		"Escaped players share telescope independently and all living central players satisfy completion");
	players[4u].CardMaze.transferStartTick = 150u;
	tests.Require(!maze.All_LivingCentral(players), "An unfinished blackout blocks final departure");
	maze.Reset(players);
	tests.Require(maze.Get_Targets().empty() && players[1u].eCardMazeRole == CARD_MAZE_ROLE::NONE &&
		players[4u].CardMaze.transferStartTick == 0u, "Reset removes run state and presentation clocks");
	std::cout << "card maze failures: " << tests.failures << '\n';
	return tests.failures == 0 ? 0 : 1;
}
