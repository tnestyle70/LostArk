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
		tests.Require(room->m_KoukuCardMaze.Get_Phase() == Maze::PHASE::INACTIVE &&
			room->m_KoukuCardMaze.Get_Targets().empty(),
			"Center Q hit before the clown box is broken leaves the telescope shut");
		// Original triggers 2201/2202: the box rises one second after entry.
		std::uint32_t tick = hitTick + 1u;
		for (std::uint32_t step = 0u; step <= Maze::CLOWN_BOX_SPAWN_DELAY_TICKS &&
			INVALID_NET_ENTITY_ID == room->m_iCardMazeClownBoxId; ++step)
			room->Update_CardMaze(tick++);
		const NET_ENTITY_ID boxId = room->m_iCardMazeClownBoxId;
		const auto findBox = [&]() {
			return std::find_if(room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
				[boxId](const SERVER_WORLD_ENTITY& entity) { return entity.iNetEntityId == boxId; });
		};
		tests.Require(INVALID_NET_ENTITY_ID != boxId && findBox() != room->m_WorldEntities.end() &&
			findBox()->strArchetypeId == Maze::CLOWN_BOX_ARCHETYPE_ID &&
			findBox()->strSpawnGroupId == Maze::CLOWN_BOX_SPAWN_GROUP_TAG,
			"Clown box rises on the telescope one second after maze entry");
		tests.Require(entrant.ModeSkillIndexBySlot[0] == 0 && entrant.ModeSkillIndexBySlot[1] == 1,
			"MAZE exposes Q and LMB on existing Q/W wire slots");
		const auto hpBeforeLmb = findBox()->iCurrentHp;
		room->m_iServerTick = tick;
		press.iRequestSequence = 2u; press.eSlot = INTERACTION_SLOT::W;
		room->Handle_InteractionSlot(11u, press);
		tests.Require(entrant.eAction == PLAYER_ACTION_STATE::INTERACTION && entrant.iCurrentSkillId == 1u,
			"LMB W-wire press reaches actual handler after Q recovery cancel");
		const auto lmbStart = entrant.iActionStartTick;
		const auto lmbHit = lmbStart + Maze::Hammer_HitTickOffset(1u);
		room->m_iServerTick = lmbHit - 2u; room->Update_Players(1.f / 30.f);
		tests.Require(findBox()->iCurrentHp == hpBeforeLmb, "LMB does not hit before native swing contact");
		room->m_iServerTick = lmbHit - 1u; room->Update_Players(1.f / 30.f);
		const auto hpAfterLmb = findBox()->iCurrentHp;
		tests.Require(hpBeforeLmb - hpAfterLmb == 100u, "LMB keeps 100 authoritative damage at its own contact tick");
		room->m_iServerTick = lmbHit; room->Update_Players(1.f / 30.f);
		tests.Require(findBox()->iCurrentHp == hpAfterLmb, "LMB contact is applied exactly once");
		tests.Require(Kouku_InteractionActionMs(KOUKU_HUD_MODE::MAZE, 0u) == 2500u &&
			Kouku_InteractionActionMs(KOUKU_HUD_MODE::MAZE, 1u) == 1000u, "Both locks match native clip lengths");
		// Reset this real 500-HP box to measure a full-health Q after the LMB check.
		findBox()->iCurrentHp = findBox()->iMaximumHp;
		const auto hpBeforeQ = findBox()->iCurrentHp;
		press.iRequestSequence = 3u; press.eSlot = INTERACTION_SLOT::Q;
		room->Handle_InteractionSlot(11u, press);
		tests.Require(entrant.eAction == PLAYER_ACTION_STATE::INTERACTION && entrant.iCurrentSkillId == 0u,
			"Maze Q reaches the actual interaction handler");
		const auto qHit = entrant.iActionStartTick + Maze::Hammer_HitTickOffset(0u);
		room->m_TickDamageEvents.clear();
		room->m_iServerTick = qHit - 2u; room->Update_Players(1.f / 30.f);
		tests.Require(findBox()->iCurrentHp == hpBeforeQ, "Q does not hit before native slam contact");
		room->m_iServerTick = qHit - 1u; room->Update_Players(1.f / 30.f);
		const bool qReported = std::any_of(room->m_TickDamageEvents.begin(), room->m_TickDamageEvents.end(),
			[boxId](const DAMAGE_EVENT& event) { return event.iTargetNetEntityId == boxId && event.iAmount == 500u; });
		tests.Require(hpBeforeQ == 500u && qReported && room->m_bCardMazeClownBoxDestroyed &&
			findBox()->iCurrentHp == 0u && findBox()->eAction == SERVER_ENTITY_ACTION::DEAD &&
			room->m_KoukuCardMaze.Get_Phase() == Maze::PHASE::INACTIVE,
			"Q deals 500 and destroys the full-health clown box without claiming the telescope");
		const auto eventCountAfterQ = room->m_TickDamageEvents.size();
		room->m_iServerTick = qHit; room->Update_Players(1.f / 30.f);
		tests.Require(room->m_TickDamageEvents.size() == eventCountAfterQ &&
			room->m_KoukuCardMaze.Get_Phase() == Maze::PHASE::INACTIVE, "Q contact is applied exactly once");
		tick = room->m_iServerTick + 1u;
		room->Resolve_CardMazeHammerHit(entrant, tick++);
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
		solo[1u].iCardMazeKillTarget == 1u && (solo[1u].CardMaze.flags & 1u),
		"Solo receives floor-mark hunter suit and initial telescope overhead together");
	tests.Require(navigation.Is_PointWalkableExact(soloSpawn.fPositionX, soloSpawn.fPositionZ) &&
		!Maze::In_SafeZone(soloSpawn.fPositionX, soloSpawn.fPositionZ), "Solo target is on a corridor outside central immunity");
	tests.Require(maze.Toggle_Telescope(solo[1u]) && !(solo[1u].CardMaze.flags & 1u) &&
		maze.Toggle_Telescope(solo[1u]), "Solo may toggle telescope without losing the hunter role");
	for (unsigned kill = 0u; kill < Maze::KILL_TARGET; ++kill)
	{
		SERVER_WORLD_ENTITY target{}; target.iNetEntityId = 900u + kill;
		maze.Register_Target(target.iNetEntityId, soloSpawn.eSuit);
		tests.Require(maze.Can_Hit(solo[1u], target.iNetEntityId), "Solo may strike its own matching target");
		const auto outcome = maze.On_TargetHit(solo[1u], target, true);
		tests.Require(outcome.bStartMarch == (kill == 0u) && outcome.bKillCounted &&
			outcome.bHunterComplete == (kill + 1u == Maze::KILL_TARGET), "Solo first matching kill starts Seto march and unlocks its exit");
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
	for (unsigned kill = 0u; kill < Maze::KILL_TARGET; ++kill)
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
		"The kill target alone never completes the maze before taking an exit");
	players[2u].CardMaze.flags |= 4u; players[2u].CardMaze.exitX = 10.f;
	maze.Reset_Progress(players[2u]);
	tests.Require(players[2u].iCardMazeKills == 0u && !(players[2u].CardMaze.flags & 4u) && players[2u].CardMaze.exitX == 0.f,
		"Seto reset revokes both stacks and the personal exit");
	tests.Require(Maze::In_SafeZone(Maze::CENTER_X + 5.f, Maze::CENTER_Z) &&
		!Maze::In_SafeZone(Maze::CENTER_X + 5.1f, Maze::CENTER_Z), "Central immunity has a bounded five metre radius");
	for (PLAYER_ID id = 2u; id <= 4u; ++id) maze.Mark_Escaped(players[id]);
	tests.Require(maze.All_LivingCentral(players) && !maze.Toggle_Telescope(players[2u]) && !maze.Toggle_Telescope(players[3u]) &&
		(players[1u].CardMaze.flags & 1u) && !(players[2u].CardMaze.flags & 1u) && !(players[3u].CardMaze.flags & 1u),
		"Only the telescope claimant keeps overhead after the other hunters escape");
	players[4u].CardMaze.transferStartTick = 150u;
	tests.Require(!maze.All_LivingCentral(players), "An unfinished blackout blocks final departure");
	maze.Reset(players);
	tests.Require(maze.Get_Targets().empty() && players[1u].eCardMazeRole == CARD_MAZE_ROLE::NONE &&
		players[4u].CardMaze.transferStartTick == 0u, "Reset removes run state and presentation clocks");
	// Use the room's actual hammer, portal, blackout and return consumers for 1..4 participants.
	for (PLAYER_ID count = 1u; count <= 4u; ++count)
	{
#ifndef _DEBUG
		if (count == 1u) continue;
#endif
		auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
		for (PLAYER_ID id = 1u; id <= count; ++id)
		{
			auto participant = players.at(id);
			participant.iCurrentHp = participant.iMaximumHp = 50000u;
			participant.isCombatReady = true;
			participant.fPositionX = Maze::CENTER_X; participant.fPositionY = -.01f; participant.fPositionZ = Maze::CENTER_Z;
			room->m_Players.emplace(id, participant);
		}
		room->m_iServerTick = 100u;
		const bool began = room->Begin_CardMaze(1u);
		const std::size_t expectedHunters = count == 1u ? 1u : count - 1u;
		tests.Require(began && room->m_KoukuCardMaze.Get_Targets().size() == expectedHunters,
			"Actual 1..4 player room spawns one matching target per hunter");
		if (!began) continue;
		const auto targets = room->m_KoukuCardMaze.Get_Targets();
		bool portals = true, cameras = true;
		for (const auto& [targetId, suit] : targets)
		{
			const auto target = std::find_if(room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
				[&](const auto& entity) { return entity.iNetEntityId == targetId; });
			const auto hunter = std::find_if(room->m_Players.begin(), room->m_Players.end(),
				[&](const auto& row) { return row.second.eCardMazeSuit == suit; });
			if (target == room->m_WorldEntities.end() || hunter == room->m_Players.end()) { portals = false; continue; }
			const std::array position{ target->fPositionX, target->fPositionY, target->fPositionZ };
			auto& player = hunter->second;
			player.fPositionX = position[0]; player.fPositionY = position[1]; player.fPositionZ = position[2] - 1.f;
			player.fYawDegrees = 0.f;
			room->Resolve_CardMazeHammerHit(player, 101u);
			portals = portals && player.iCardMazeKills == 1u && (player.CardMaze.flags & 4u) &&
				player.CardMaze.exitX == position[0] && player.CardMaze.exitY == position[1] && player.CardMaze.exitZ == position[2];
			player.fPositionX = player.CardMaze.exitX; player.fPositionY = player.CardMaze.exitY; player.fPositionZ = player.CardMaze.exitZ;
		}
		for (const auto& [id, player] : room->m_Players)
			cameras = cameras && ((player.CardMaze.flags & 1u) != 0u) == (id == 1u);
		tests.Require(portals && cameras, "Each matching kill puts its portal on that suit and only its claimant has telescope view");
		room->m_iCardMazeMarchStartTick = 0u; // Isolate portal arrival from the independently tested march-contact reset.
		room->Update_CardMaze(102u);
		const bool beforeArrival = room->m_KoukuCardMaze.Get_Phase() == Maze::PHASE::HUNTING;
		for (std::uint32_t tick = 103u; tick <= 180u; ++tick) room->Update_CardMaze(tick);
		const auto* returnPlacement = room->Find_Placement("cardmaze.return");
		bool returned = returnPlacement && returnPlacement->TriggerActions.size() == 1u;
		if (returned)
		{
			const auto& destination = returnPlacement->TriggerActions.front();
			for (const auto& [id, player] : room->m_Players)
			{
				(void)id;
				returned = returned && player.eCardMazeRole == CARD_MAZE_ROLE::NONE && !player.CardMaze.transferStartTick &&
					std::abs(player.fPositionX - destination.fTargetX) < .05f && std::abs(player.fPositionZ - destination.fTargetZ) < .05f;
			}
		}
		tests.Require(beforeArrival && returned && room->m_KoukuCardMaze.Get_Phase() == Maze::PHASE::INACTIVE,
			"After every hunter takes the personal portal all 1..4 participants finish the Gate 2 return together");
	}
	// Card-rain soldiers use the same catalog bodies, but never become maze targets.
	{
		auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
		const auto* placement = room->Find_Placement("boss.kakulsaydon.g1.saydon");
		tests.Require(room->Is_Ready() && placement, "Card rain uses the actual room and Saydon placement");
		if (room->Is_Ready() && placement)
		{
			SERVER_WORLD_ENTITY owner{}; owner.iNetEntityId = room->m_iNextNetEntityId++;
			owner.eKind = WORLD_BOOTSTRAP_KIND::BOSS; owner.iCurrentHp = owner.iMaximumHp = 1000u;
			owner.strPatternId = "cardrain.contract"; owner.iPatternSequence = 1u;
			owner.fPositionX = placement->fPositionX; owner.fPositionY = placement->fPositionY; owner.fPositionZ = placement->fPositionZ;
			room->m_WorldEntities.push_back(owner);
			const auto base = room->m_WorldEntities.size();
			tests.Require(!room->Spawn_KoukuCardRainSoldiers(INVALID_NET_ENTITY_ID, 100u) && room->m_WorldEntities.size() == base,
				"Invalid card-rain owner preserves all existing entities");
			BOSS_PATTERN_MECHANIC_TRIGGER trigger{}; trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_RAIN_SOLDIERS;
			trigger.strTriggerId = "cardrain.contract.trigger";
			room->m_PendingKoukuMechanicTriggers.push_back({owner.iNetEntityId, owner.iPatternSequence, trigger});
			room->Commit_KoukuMechanicTriggers(100u);
			const bool spawned = room->m_KoukuCardRainSoldiers.size() == 3u;
			tests.Require(spawned && room->m_KoukuCardRainSoldiers.size() == 3u && room->m_WorldEntities.size() == base + 3u,
				"Card rain commits exactly three normal replicated monsters");
			if (spawned)
			{
				std::set<std::string> suits; bool grounded = true;
				for (const auto& [id, state] : room->m_KoukuCardRainSoldiers)
				{
					const auto entity = std::find_if(room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
						[id](const auto& row) { return row.iNetEntityId == id; });
					if (entity == room->m_WorldEntities.end()) { grounded = false; continue; }
					suits.insert(entity->strArchetypeId); SERVER_NAV_POINT ground{};
					grounded = grounded && entity->eKind == WORLD_BOOTSTRAP_KIND::MONSTER && !room->m_KoukuCardMaze.Is_Target(id) &&
						room->m_ServerNavigation.Sample_Position(entity->fPositionX, entity->fPositionZ, ground) &&
						std::abs(ground.y - entity->fPositionY) < .01f;
				}
				tests.Require(grounded && suits == std::set<std::string>{"MONSTER_KOUKU_CARD_CLUB", "MONSTER_KOUKU_CARD_HEART", "MONSTER_KOUKU_CARD_DIAMOND"},
					"All three source suits use navigation and do not enter the maze kill ledger");
				tests.Require(room->Spawn_KoukuCardRainSoldiers(owner.iNetEntityId, 101u) && room->m_WorldEntities.size() == base + 3u,
					"Replaying the same pattern trigger does not duplicate soldiers");
				const auto expiry = 100u + CKoukuSaydonLogicRuntime::Ticks_FromMs(30000u);
				room->Update_KoukuCardRainSoldiers(expiry + 1u);
				tests.Require(room->m_KoukuCardRainSoldiers.size() == 3u && room->m_WorldEntities.size() == base + 3u,
					"Card soldiers remain authoritative monsters after the former 30-second deadline");
				auto boss = std::find_if(room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
					[&](const auto& row) { return row.iNetEntityId == owner.iNetEntityId; });
				boss->iPatternSequence = 2u;
				tests.Require(room->Spawn_KoukuCardRainSoldiers(owner.iNetEntityId, expiry + 1u) &&
					room->m_KoukuCardRainSoldiers.size() == 6u && room->m_WorldEntities.size() == base + 6u,
					"A new pattern adds a fresh batch while preserving existing chasing soldiers");
				boss = std::find_if(room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
					[&](const auto& row) { return row.iNetEntityId == owner.iNetEntityId; });
				boss->strPatternId.clear(); room->Update_KoukuCardRainSoldiers(expiry + 2u);
				tests.Require(room->m_KoukuCardRainSoldiers.size() == 6u && room->m_WorldEntities.size() == base + 6u,
					"Finishing the owning pattern preserves soldiers and their normal monster combat lifecycle");
				boss->iCurrentHp = 0u; room->Update_KoukuCardRainSoldiers(expiry + 3u);
				tests.Require(room->m_KoukuCardRainSoldiers.empty() && room->m_WorldEntities.size() == base,
					"Owner death cleans persistent soldiers without removing unrelated room entities");
			}
		}
	}
	std::cout << "card maze failures: " << tests.failures << '\n';
	return tests.failures == 0 ? 0 : 1;
}
