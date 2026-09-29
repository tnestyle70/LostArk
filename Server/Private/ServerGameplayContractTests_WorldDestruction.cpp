#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "ClientSession.h"
#include "EncounterPropRuntime.h"
#include "Gameplay/CombatCollisionContract.h"
#include "GameplayCatalog.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include "GameRoom.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "ServerCombatHitRuntime.h"
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
#include <iomanip>
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

int LostArk::Server::CServerGameplayContractRunner::Run_ValtanArenaSupport()
{
	TESTS tests;
	auto storage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
	CGameRoom& room = *storage;
	std::string status;
	/* Keep the published arena ground and destruction conditions. This fixture
	isolates ground support after the walls have gone; the dedicated wall case
	below restores an actual authoritative collision box on that same ground. */
	const bool ready = room.Is_Ready() &&
		room.m_ServerCollisionSystem.Initialize({}, status);
	tests.Require(ready, "Load the published Valtan arena for forced-movement support contracts");
	if (!ready)
		return 1;

	constexpr float delta = 1.f / 30.f;
	constexpr PLAYER_ID firstPlayer = 29200u;
	SERVER_NAV_POINT core{};
	const bool coreSupported = room.m_ServerNavigation.Sample_SurfacePosition(
		154.296f, -125.219f, core);
	tests.Require(coreSupported, "Find physical support under the Valtan raid core");
	if (!coreSupported)
		return 1;
	const auto playerAt = [&](const PLAYER_ID id, const SERVER_NAV_POINT& position)
	{
		SERVER_PLAYER player{};
		player.iPlayerId = id;
		player.iNetEntityId = id + 100u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = player.iMaximumHp = 5000u;
		player.isCombatReady = true;
		player.fPositionX = position.x;
		player.fPositionY = position.y;
		player.fPositionZ = position.z;
		return player;
	};
	const auto push = [&](SERVER_PLAYER& player, const float directionX,
		const float directionZ, const float distance, const std::uint32_t tick)
	{
		CPlayerSkillSystem::Arm_PlayerHitReaction(player,
			player.fPositionX - directionX, player.fPositionZ - directionZ,
			distance, 100u, true, 1000u, tick);
	};
	for (PLAYER_ID id = firstPlayer; id != firstPlayer + 4u; ++id)
	{
		SERVER_PLAYER player = playerAt(id, core);
		room.m_PlayerIdByEntityId.emplace(player.iNetEntityId, id);
		room.m_Players.emplace(id, std::move(player));
	}

	SERVER_NAVIGATION_CONDITION_STAGE collapse{};
	const bool prepared = room.m_ServerNavigation.Prepare_ConditionChanges(
		{ { "condition.valtan.floor30.rail.7000000000000000001.collapsed", true } },
		collapse, status);
	if (prepared)
		room.m_ServerNavigation.Commit_ConditionChanges(std::move(collapse));
	/* Discover a supported-to-void cell edge in the actual destroyed sector,
	so this tests a hit moving into a hole rather than a player placed in one. */
	SERVER_NAV_POINT edgeStart{};
	float directionX = 0.f, directionZ = 0.f;
	bool foundVoidEdge = false;
	constexpr std::array<std::pair<float, float>, 4> directions = {
		std::pair{ 1.f, 0.f }, { -1.f, 0.f }, { 0.f, 1.f }, { 0.f, -1.f } };
	const float cell = room.m_ServerNavigation.Get_CellSize();
	for (float x = 147.25f; x <= 163.25f && !foundVoidEdge; x += cell)
	{
		for (float z = -115.25f; z <= -99.25f && !foundVoidEdge; z += cell)
		{
			SERVER_NAV_POINT support{};
			if (!room.m_ServerNavigation.Sample_SurfacePosition(x, z, support) ||
				std::abs(support.y - core.y) > 1.f)
				continue;
			for (const auto& [dx, dz] : directions)
			{
				if (room.m_ServerNavigation.Is_PointInVoidRegion(x + dx * cell, z + dz * cell))
				{
					edgeStart = support;
					directionX = dx;
					directionZ = dz;
					foundVoidEdge = true;
					break;
				}
			}
		}
	}
	tests.Require(prepared && foundVoidEdge,
		"Find a real supported-to-collapsed-floor boundary in the published Valtan arena");
	if (prepared && foundVoidEdge)
	{
		SERVER_PLAYER& struck = room.m_Players.at(firstPlayer);
		struck = playerAt(firstPlayer, edgeStart);
		push(struck, directionX, directionZ, cell * 4.f, 100u);
		const bool ordinaryHit = !struck.bKnockbackCanLeaveArena &&
			!struck.bArenaEjectionActive && !struck.bKnockbackBallistic;
		room.m_iServerTick = 99u;
		room.Advance_PlayerKnockback(struck, delta);
		const float travel = std::hypot(struck.fPositionX - edgeStart.x,
			struck.fPositionZ - edgeStart.z);
		tests.Require(ordinaryHit && PLAYER_ACTION_STATE::FALLING == struck.eAction &&
			travel > 0.f && travel < cell * 2.f &&
			std::abs(struck.fPositionY - edgeStart.y) < 0.1f &&
			struck.iFallDeathTick > struck.iActionStartTick &&
			0.f == struck.fKnockbackRemainingSeconds && !struck.isCombatReady,
			"An ordinary Valtan hit crosses the actual void edge and begins falling without a launch flag or lower-floor snap");
		const std::uint32_t deadline = struck.iFallDeathTick;
		if (PLAYER_ACTION_STATE::FALLING == struck.eAction)
		{
			for (std::uint32_t tick = struck.iActionStartTick + 1u; tick <= deadline; ++tick)
			{
				for (auto& [id, player] : room.m_Players)
					room.Update_PlayerFall(player, delta, tick);
			}
		}
		const bool teammatesUntouched = std::all_of(std::next(room.m_Players.begin()),
			room.m_Players.end(), [&](const auto& entry)
			{
				const auto& player = entry.second;
				return PLAYER_ACTION_STATE::NONE == player.eAction && player.iCurrentHp == 5000u &&
					player.isCombatReady && player.fPositionX == core.x &&
					player.fPositionY == core.y && player.fPositionZ == core.z;
			});
		tests.Require(PLAYER_ACTION_STATE::DEAD == struck.eAction && struck.iCurrentHp == 0u &&
			teammatesUntouched && room.m_Players.size() == 4u,
			"In a four-player room only the player pushed over the collapsed edge dies at the fall deadline");
	}

	/* These adjacent published cells exposed the original lower-deck route.
	Their surface survives even when a walking blocker covers the upper cell. */
	SERVER_NAVIGATION_CONDITION_STAGE restore{};
	const bool restored = room.m_ServerNavigation.Prepare_ConditionChanges(
		{ { "condition.valtan.floor30.rail.7000000000000000001.collapsed", false } },
		restore, status);
	if (restored)
		room.m_ServerNavigation.Commit_ConditionChanges(std::move(restore));
	SERVER_NAV_POINT upper{}, lower{};
	const bool measuredDrop = restored && room.m_ServerNavigation.Sample_SurfacePosition(
		142.25f, -114.25f, upper) && room.m_ServerNavigation.Sample_SurfacePosition(
		141.75f, -114.25f, lower) && upper.y - lower.y > 10.f;
	tests.Require(measuredDrop, "Measure the published twelve-metre Valtan deck discontinuity");
	if (measuredDrop)
	{
		SERVER_PLAYER player = playerAt(firstPlayer, upper);
		push(player, -1.f, 0.f, 2.f, 200u);
		room.m_iServerTick = 199u;
		room.Advance_PlayerKnockback(player, delta);
		tests.Require(PLAYER_ACTION_STATE::FALLING == player.eAction &&
			player.fPositionX < upper.x && player.fPositionY > upper.y - 0.1f &&
			player.fPositionY > lower.y + 10.f,
			"A real Valtan hit over the lower deck falls from its former height instead of snapping to lower navigation");

		SERVER_PLAYER rising = playerAt(firstPlayer, lower);
		push(rising, 1.f, 0.f, 2.f, 201u);
		room.m_iServerTick = 200u;
		room.Advance_PlayerKnockback(rising, delta);
		tests.Require(PLAYER_ACTION_STATE::FALLING != rising.eAction &&
			std::abs(rising.fPositionY - lower.y) < 0.1f &&
			rising.fPositionX < upper.x && 0.f == rising.fKnockbackRemainingSeconds,
			"A forced move toward the upper deck stops below it without teleporting upward");

		SERVER_PLAYER supported = playerAt(firstPlayer, upper);
		push(supported, 1.f, 0.f, 0.3f, 202u);
		room.m_iServerTick = 201u;
		room.Advance_PlayerKnockback(supported, delta);
		tests.Require(!room.m_ServerNavigation.Is_PointWalkableExact(upper.x, upper.z) &&
			supported.fPositionX > upper.x + 0.05f &&
			PLAYER_ACTION_STATE::FALLING != supported.eAction &&
			std::abs(supported.fPositionY - upper.y) < 0.1f,
			"Physical floor supports forced movement even when its walking cell is blocked");

		SERVER_NAV_POINT beforeWall{};
		WORLD_BOOTSTRAP_PLACEMENT edgeWall{};
		edgeWall.strPlacementId = "collision.contract.valtan-edge-wall";
		edgeWall.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX;
		edgeWall.fPositionX = upper.x + 0.25f;
		edgeWall.fPositionY = upper.y + 1.f;
		edgeWall.fPositionZ = upper.z;
		edgeWall.fHalfExtentX = 0.1f;
		edgeWall.fHalfExtentY = 2.f;
		edgeWall.fHalfExtentZ = 2.f;
		const bool edgeWallReady = room.m_ServerNavigation.Sample_SurfacePosition(
			upper.x + 1.f, upper.z, beforeWall) &&
			std::abs(beforeWall.y - upper.y) < 0.1f &&
			room.m_ServerCollisionSystem.Initialize({ edgeWall }, status);
		SERVER_PLAYER protectedPlayer = playerAt(firstPlayer, beforeWall);
		push(protectedPlayer, -1.f, 0.f, 6.f, 203u);
		room.m_iServerTick = 202u;
		room.Advance_PlayerKnockback(protectedPlayer, delta);
		tests.Require(edgeWallReady && PLAYER_ACTION_STATE::FALLING != protectedPlayer.eAction &&
			protectedPlayer.fPositionX > edgeWall.fPositionX &&
			std::abs(protectedPlayer.fPositionY - beforeWall.y) < 0.1f &&
			0.f == protectedPlayer.fKnockbackRemainingSeconds,
			"A wall before the real lower-deck edge prevents falling even when the requested push ends beyond that edge");
	}

	WORLD_BOOTSTRAP_PLACEMENT wall{};
	wall.strPlacementId = "collision.contract.valtan-support-wall";
	wall.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX;
	wall.isEnabled = true;
	wall.fPositionX = core.x + 1.f;
	wall.fPositionY = core.y + 1.f;
	wall.fPositionZ = core.z;
	wall.fHalfExtentX = 0.1f;
	wall.fHalfExtentY = 2.f;
	wall.fHalfExtentZ = 2.f;
	const bool wallReady = room.m_ServerCollisionSystem.Initialize({ wall }, status);
	SERVER_PLAYER walled = playerAt(firstPlayer, core);
	push(walled, 1.f, 0.f, 6.f, 300u);
	room.m_iServerTick = 299u;
	room.Advance_PlayerKnockback(walled, delta);
	tests.Require(wallReady && walled.fPositionX < wall.fPositionX - wall.fHalfExtentX &&
		PLAYER_ACTION_STATE::FALLING != walled.eAction &&
		std::abs(walled.fPositionY - core.y) < 0.1f &&
		0.f == walled.fKnockbackRemainingSeconds,
		"An authoritative wall still stops an ordinary Valtan hit before support is tested past the wall");

	room.m_WorldEntities.clear();
	SERVER_WORLD_ENTITY boss{};
	boss.iNetEntityId = 29900u;
	boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	boss.iCurrentHp = boss.iMaximumHp = 1000u;
	boss.iPatternSequence = 1u;
	boss.fPositionX = boss.fSpawnPositionX = core.x;
	boss.fPositionY = boss.fSpawnPositionY = core.y;
	boss.fPositionZ = boss.fSpawnPositionZ = core.z;
	boss.fYawDegrees = -90.f;
	room.m_WorldEntities.push_back(boss);
	SERVER_PLAYER& thrown = room.m_Players.at(firstPlayer);
	thrown = playerAt(firstPlayer, core);
	BOSS_PATTERN_STAGE_ACTION release{};
	release.eReleaseMode = BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION;
	release.fReleaseSpeedMps = 24.f;
	release.iDurationMs = 500u;
	const bool ejectionReady = room.Capture_PlayerAttachment(thrown.iNetEntityId, boss.iNetEntityId,
		PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 400u) &&
		room.Prepare_ArenaEjection(thrown, room.m_WorldEntities.front(), release, 401u);
	for (std::uint32_t step = 0u; step != 200u && thrown.bArenaEjectionActive; ++step)
	{
		room.m_iServerTick = 400u + step;
		room.Advance_PlayerKnockback(thrown, delta);
	}
	tests.Require(ejectionReady && PLAYER_ACTION_STATE::FALLING == thrown.eAction &&
		!thrown.bArenaEjectionActive && thrown.fPositionX > wall.fPositionX + 2.f &&
		thrown.iFallDeathTick > thrown.iActionStartTick,
		"Rear-grab ARENA_EJECTION still crosses the wall and ends in the existing finite falling state");

	{
		// Keep Bahuntur's existing grant and mitigation while adding the wipe verdict text.
		const CGameplayCatalog& catalog = room.m_GameplayCatalog.Active();
		const auto* guard = EstherStrike::Find_Guard(ESTHER_ID::BAHUNTUR);
		tests.Require(guard && guard->iGrantTimeMs == 4000u && guard->iDurationMs == 30000u &&
			guard->fRadiusM == 7.f && guard->fOffsetForwardM == 4.2f && guard->iDamageTakenPercent == -50,
			"Bahuntur retains the four-second, seven-metre, thirty-second protection contract");
		if (!guard) return 1;
		constexpr std::uint32_t grantTick = 1001u;
		const std::uint32_t guardEnd = grantTick + 900u;
		SERVER_PLAYER seed = playerAt(firstPlayer, core);
		seed.iCurrentHp = seed.iMaximumHp = 100u;
		room.m_Players.clear();
		room.m_Players.emplace(firstPlayer, seed);
		SERVER_PLAYER edge = seed;
		edge.iPlayerId += 1u; edge.iNetEntityId += 1u;
		edge.fPositionX = core.x + guard->fOffsetForwardM + guard->fRadiusM - .01f;
		room.m_Players.emplace(edge.iPlayerId, edge);
		std::array<PLAYER_ID, 4u> partyIds{firstPlayer, edge.iPlayerId, firstPlayer + 10u, firstPlayer + 11u};
		for (size_t index = 2u; index < partyIds.size(); ++index)
		{
			SERVER_PLAYER member = seed;
			member.iPlayerId = partyIds[index]; member.iNetEntityId += static_cast<NET_ENTITY_ID>(10u + index);
			member.fPositionX += index == 2u ? 2.f : -2.f;
			room.m_Players.emplace(member.iPlayerId, member);
		}
		SERVER_PLAYER outside = edge;
		outside.iPlayerId += 1u; outside.iNetEntityId += 1u; outside.fPositionX += .02f;
		room.m_Players.emplace(outside.iPlayerId, outside);
		SERVER_PLAYER dead = seed;
		dead.iPlayerId += 3u; dead.iNetEntityId += 3u;
		dead.iCurrentHp = 0u; dead.eAction = PLAYER_ACTION_STATE::DEAD;
		room.m_Players.emplace(dead.iPlayerId, dead);
		SERVER_WORLD_ENTITY summon{};
		summon.eEstherId = ESTHER_ID::BAHUNTUR;
		summon.fPositionX = core.x; summon.fPositionZ = core.z; summon.fYawDegrees = 90.f;
		summon.fActionElapsedSeconds = 3.999f;
		room.Apply_EstherStrikeHits(summon, grantTick - 1u);
		tests.Require(!summon.bEstherSupportApplied && !room.m_Players.at(firstPlayer).iEstherGuardEndTick,
			"Bahuntur grants no protection before the actual four-second strike point");
		summon.fActionElapsedSeconds = 4.f;
		room.Apply_EstherStrikeHits(summon, grantTick);
		auto& guarded = room.m_Players.at(firstPlayer);
		tests.Require(summon.bEstherSupportApplied && guarded.iEstherGuardEndTick == guardEnd &&
			guarded.iEstherGuardDamageTakenPercent == -50 && !guarded.iInvulnerableEndTick &&
			std::all_of(partyIds.begin(), partyIds.end(), [&](const auto id) {
				return room.m_Players.at(id).iEstherGuardEndTick == guardEnd; }) &&
			!room.m_Players.at(outside.iPlayerId).iEstherGuardEndTick &&
			!room.m_Players.at(dead.iPlayerId).iEstherGuardEndTick,
			"Bahuntur grants thirty seconds to all four living party members inside the forward-offset circle");
		room.Apply_EstherStrikeHits(summon, grantTick + 1u);
		tests.Require(guarded.iEstherGuardEndTick == guardEnd,
			"Later summon ticks never extend the one-time Bahuntur grant");

		std::vector<DAMAGE_EVENT> events;
		SERVER_WORLD_TO_PLAYER_HIT hit{};
		hit.iRawDamage = 20u; hit.bIgnoreDefense = true; hit.bIgnoreCounter = true;
		hit.iServerTick = grantTick + 2u;
		hit.fSourceX = guarded.fPositionX - 1.f; hit.fSourceZ = guarded.fPositionZ;
		hit.fPushRangeM = 2.f; hit.iPushMs = 250u; hit.bKnockdown = true; hit.iDownMs = 1000u;
		tests.Require(CServerCombatHitRuntime::Apply_WorldToPlayer(guarded, hit, catalog, events) == SERVER_COMBAT_HIT_RESULT::LANDED &&
			guarded.iCurrentHp == 90u && events.size() == 1u && events.front().iAmount == 10u &&
			guarded.eAction == PLAYER_ACTION_STATE::NONE && guarded.fKnockbackRemainingSeconds == 0.f &&
			!guarded.iInvulnerabilityZoneContactTick && !guarded.iInvulnerabilityZonePulseTick,
			"Bahuntur keeps fifty-percent ordinary damage and hit-reaction immunity without false invulnerable text");
		events.clear();
		guarded.iShield = 50u;
		hit.iServerTick += 1u; hit.bEstherGuardBlockable = true;
		tests.Require(CServerCombatHitRuntime::Apply_WorldToPlayer(guarded, hit, catalog, events) == SERVER_COMBAT_HIT_RESULT::ABSORBED &&
			guarded.iCurrentHp == 90u && guarded.iShield == 50u && events.empty() &&
			guarded.iInvulnerabilityZoneContactTick == hit.iServerTick && guarded.iInvulnerabilityZonePulseTick == hit.iServerTick,
			"A Bahuntur-blockable wipe preserves HP and shield and emits the existing invulnerable text pulse");
		guarded.iInvulnerableEndTick = guardEnd + 100u;
		hit.iServerTick += 1u;
		tests.Require(CServerCombatHitRuntime::Apply_WorldToPlayer(guarded, hit, catalog, events) == SERVER_COMBAT_HIT_RESULT::ABSORBED &&
			guarded.iInvulnerabilityZoneContactTick == hit.iServerTick && guarded.iInvulnerabilityZonePulseTick == hit.iServerTick &&
			guarded.iInvulnerableEndTick == guardEnd + 100u && events.empty(),
			"Overlapping ordinary invulnerability cannot hide or shorten Bahuntur's wipe verdict text");
		SERVER_PLAYER expired = guarded;
		expired.iInvulnerableEndTick = 0u; expired.iShield = 0u;
		hit.iServerTick = guardEnd;
		const auto previousPulse = expired.iInvulnerabilityZonePulseTick;
		tests.Require(CServerCombatHitRuntime::Apply_WorldToPlayer(expired, hit, catalog, events) == SERVER_COMBAT_HIT_RESULT::LANDED &&
			expired.iCurrentHp == 70u && expired.iInvulnerabilityZonePulseTick == previousPulse &&
			expired.iInvulnerabilityZoneContactTick != guardEnd,
			"The exact thirty-second expiry restores full damage and emits no new invulnerable text");
		events.clear();
		SERVER_PLAYER forcedWipe = guarded;
		hit.iServerTick = guardEnd - 1u; hit.bEncounterWipe = true;
		const auto wipePreviousPulse = forcedWipe.iInvulnerabilityZonePulseTick;
		tests.Require(CServerCombatHitRuntime::Apply_WorldToPlayer(forcedWipe, hit, catalog, events) == SERVER_COMBAT_HIT_RESULT::KILLED &&
			!forcedWipe.iCurrentHp && !forcedWipe.iShield && !forcedWipe.iEstherGuardEndTick &&
			!forcedWipe.iInvulnerableEndTick && forcedWipe.iInvulnerabilityZonePulseTick == wipePreviousPulse,
			"A forced encounter-failure wipe still bypasses Bahuntur and ordinary invulnerability without false protection text");

		DAMAGE_EVENT bossInvincibleEvent{};
		bool checkedBossInvulnerability = false;
		// Run the published six-direction pattern so its real damage-profile mapping owns the verdict.
		const auto* placement = room.Find_Placement("boss.valtan.center");
		SERVER_WORLD_ENTITY floorBoss{};
		const bool floorReady = placement && room.Build_WorldEntity(*placement, 29950u, floorBoss);
		tests.Require(floorReady, "Load the actual Valtan boss for Bahuntur's six-direction wipe verdict");
		if (floorReady)
		{
			std::map<PLAYER_ID, SERVER_PLAYER> targets;
			SERVER_PLAYER protectedTarget = seed;
			protectedTarget.iEstherGuardEndTick = guardEnd;
			protectedTarget.iEstherGuardDamageTakenPercent = -50;
			protectedTarget.fPositionX = floorBoss.fPositionX + 20.f;
			protectedTarget.fPositionZ = floorBoss.fPositionZ;
			std::array<NET_ENTITY_ID, 4u> protectedIds{};
			for (size_t index = 0u; index < partyIds.size(); ++index)
			{
				auto member = protectedTarget;
				member.iPlayerId = partyIds[index]; member.iNetEntityId += static_cast<NET_ENTITY_ID>(index);
				protectedIds[index] = member.iNetEntityId;
				targets.emplace(member.iPlayerId, member);
			}
			SERVER_PLAYER unprotectedTarget = protectedTarget;
			unprotectedTarget.iPlayerId += 100u; unprotectedTarget.iNetEntityId += 100u;
			unprotectedTarget.iEstherGuardEndTick = 0u; unprotectedTarget.iEstherGuardDamageTakenPercent = 0;
			targets.emplace(unprotectedTarget.iPlayerId, unprotectedTarget);
			floorBoss.bIntroPatternConsumed = true;
			floorBoss.bAutomaticPatternSequenceAuditionOverride = true;
			floorBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
			floorBoss.PendingPatternIds = { "VALTAN_FLOOR_WIPE_130" };
			floorBoss.TriggeredPatternIds.push_back("VALTAN_FLOOR_WIPE_130");
			CValtanBrain floorBrain;
			bool observedWipe = false;
			for (std::uint32_t tick = grantTick + 10u; tick < grantTick + 250u && !observedWipe; ++tick)
			{
				events.clear();
				floorBrain.Update(floorBoss, targets, catalog, room.m_ServerNavigation, delta, tick, {}, events);
				if (!checkedBossInvulnerability && floorBoss.strPatternId == "VALTAN_FLOOR_WIPE_130")
				{
					const auto hp = floorBoss.iCurrentHp;
					const auto armor = floorBoss.ArmorPlates;
					const auto combat = floorBoss.BossCombat;
					SERVER_PLAYER_TO_WORLD_HIT attack{};
					attack.iSourcePlayerId = firstPlayer; attack.iSkillId = 34010u;
					attack.iRawDamage = 1000u; attack.iStaggerDamage = 50u; attack.iPartDamage = 20u;
					attack.iCounterPower = 50u; attack.bCounterFromPrimarySlot = true; attack.iServerTick = tick;
					std::vector<DAMAGE_EVENT> blockedEvents;
					const auto verdict = CServerCombatHitRuntime::Apply_PlayerToWorld(floorBoss, attack, blockedEvents);
					checkedBossInvulnerability = floorBoss.bPatternInvulnerable &&
						verdict == SERVER_COMBAT_HIT_RESULT::ABSORBED && floorBoss.iCurrentHp == hp &&
						floorBoss.MvpLedger.empty() && floorBoss.BossCombat.iShieldCurrent == combat.iShieldCurrent &&
						floorBoss.BossCombat.iStaggerCurrent == combat.iStaggerCurrent &&
						floorBoss.BossCombat.iStateRevision == combat.iStateRevision &&
						std::equal(armor.begin(), armor.end(), floorBoss.ArmorPlates.begin(), floorBoss.ArmorPlates.end(),
							[](const auto& before, const auto& after) { return before.iRemainingDurability == after.iRemainingDurability; }) &&
						std::equal(combat.Parts.begin(), combat.Parts.end(), floorBoss.BossCombat.Parts.begin(), floorBoss.BossCombat.Parts.end(),
							[](const auto& before, const auto& after) { return before.iCurrentDurability == after.iCurrentDurability; }) &&
						blockedEvents.size() == 1u &&
						blockedEvents.front().eHitFlag == DAMAGE_HIT_FLAG::INVINCIBLE && !blockedEvents.front().iAmount &&
						!blockedEvents.front().iStaggerAmount && !blockedEvents.front().isCounterSuccess && !blockedEvents.front().isStaggerSuccess;
					if (checkedBossInvulnerability) bossInvincibleEvent = blockedEvents.front();
					tests.Require(checkedBossInvulnerability,
						"The published 130-bar pattern blocks attacks without HP or MVP changes and emits the Server's zero-damage INVINCIBLE verdict");
				}
				if (targets.at(protectedTarget.iPlayerId).iInvulnerabilityZonePulseTick != tick) continue;
				observedWipe = true;
				tests.Require(floorBoss.strPatternId == "VALTAN_FLOOR_WIPE_130" && floorBoss.strPatternStageId == "SECOND_SMASH" &&
					floorBoss.strDamageProfileId == "damage.valtan.omnidirectional-wipe-130" &&
					std::all_of(partyIds.begin(), partyIds.end(), [&](const auto id) {
						return targets.at(id).iCurrentHp == 100u && targets.at(id).iInvulnerabilityZonePulseTick == tick; }) &&
					targets.at(protectedTarget.iPlayerId).iInvulnerabilityZoneContactTick == tick &&
					targets.at(unprotectedTarget.iPlayerId).iCurrentHp < 100u &&
					!targets.at(unprotectedTarget.iPlayerId).iInvulnerabilityZonePulseTick &&
					std::none_of(events.begin(), events.end(), [&](const auto& event) {
						return std::find(protectedIds.begin(), protectedIds.end(), event.iTargetNetEntityId) != protectedIds.end(); }),
					"The published delayed SECOND_SMASH damages the control player and gives all four Bahuntur survivors invulnerable text without damage");
			}
			tests.Require(observedWipe, "The actual six-direction pattern reaches its Bahuntur-blockable wipe");
		}

		// The live send queue replaces pending snapshots; the occurrence must survive that replacement.
		room.m_WorldEntities.clear();
		room.m_TickDamageEvents.clear();
		room.m_TickBossCombatEvents.clear();
		room.m_Players.clear();
		constexpr SESSION_ID snapshotSessionId = 29990u;
		constexpr std::uint32_t pulseTick = 2000u;
		SERVER_PLAYER snapshotSeed = seed;
		snapshotSeed.iSessionId = snapshotSessionId;
		snapshotSeed.iEstherGuardEndTick = pulseTick + 1u;
		snapshotSeed.iEstherGuardDamageTakenPercent = -50;
		SERVER_WORLD_TO_PLAYER_HIT blocked{};
		blocked.iServerTick = pulseTick; blocked.iRawDamage = 100u; blocked.bEstherGuardBlockable = true;
		events.clear();
		const auto blockedResult = CServerCombatHitRuntime::Apply_WorldToPlayer(snapshotSeed, blocked, catalog, events);
		room.m_Players.emplace(firstPlayer, snapshotSeed);
		auto session = std::make_shared<CClientSession>(snapshotSessionId, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		session->m_isSendRunning.store(true);
		room.m_Sessions.emplace(snapshotSessionId, session);
		room.m_PlayerIdBySessionId.emplace(snapshotSessionId, firstPlayer);
		const auto snapshotHasPulse = [&](const std::uint32_t tick, const std::uint32_t expectedPulse)
		{
			room.m_iServerTick = tick;
			room.Broadcast_WorldSnapshot();
			if (session->m_OutboundFrames.size() != 1u ||
				session->m_OutboundFrames.front().ePacketType != PACKET_TYPE::S2C_WORLD_SNAPSHOT)
				return false;
			const auto& bytes = session->m_OutboundFrames.front().Bytes;
			if (bytes.size() <= PACKET_HEADER_BYTES) return false;
			CPacketReader reader{ std::span<const std::uint8_t>(bytes).subspan(PACKET_HEADER_BYTES) };
			S2C_WORLD_SNAPSHOT snapshot{};
			return Read_Message(reader, snapshot) && !reader.Get_RemainingSize() && snapshot.iServerTick == tick &&
				snapshot.eWorldId == room.m_eWorldId && snapshot.Players.size() == 1u &&
				snapshot.Players.front().iNetEntityId == snapshotSeed.iNetEntityId &&
				snapshot.Players.front().iInvulnerabilityZonePulseTick == expectedPulse;
		};
		tests.Require(blockedResult == SERVER_COMBAT_HIT_RESULT::ABSORBED && snapshotHasPulse(pulseTick, pulseTick),
			"A blocked Bahuntur hit reaches the real world snapshot with its original occurrence tick");
		if (checkedBossInvulnerability && !session->m_OutboundFrames.empty())
		{
			const auto& bytes = session->m_OutboundFrames.front().Bytes;
			CPacketReader sourceReader{std::span<const std::uint8_t>(bytes).subspan(PACKET_HEADER_BYTES)};
			S2C_WORLD_SNAPSHOT wire{};
			const bool read = Read_Message(sourceReader, wire);
			wire.DamageEvents = {bossInvincibleEvent};
			CPacketWriter writer;
			const bool written = read && Write_Message(writer, wire);
			CPacketReader reader{writer.Get_Buffer()};
			S2C_WORLD_SNAPSHOT decoded{};
			tests.Require(written && Read_Message(reader, decoded) && !reader.Get_RemainingSize() &&
				decoded.DamageEvents.size() == 1u && decoded.DamageEvents.front().eHitFlag == DAMAGE_HIT_FLAG::INVINCIBLE &&
				!decoded.DamageEvents.front().iAmount,
				"The zero-damage boss invulnerability verdict survives actual snapshot encode and decode");
			bool rejectedContradictions = true;
			for (int kind = 0; kind < 6; ++kind)
			{
				wire.DamageEvents = {bossInvincibleEvent};
				auto& event = wire.DamageEvents.front();
				if (kind == 0) event.iAmount = 1u;
				if (kind == 1) event.iStaggerAmount = 1u;
				if (kind == 2) event.isCounterSuccess = true;
				if (kind == 3) event.isStaggerSuccess = true;
				if (kind == 4) event.isOutgoing = false;
				if (kind == 5) event.eHitFlag = DAMAGE_HIT_FLAG::NORMAL;
				CPacketWriter rejected;
				rejectedContradictions = !Write_Message(rejected, wire) && rejectedContradictions;
			}
			tests.Require(rejectedContradictions,
				"Snapshot validation rejects nonzero, mechanic-credit, incoming, and untyped empty invulnerability verdicts");
		}
		tests.Require(snapshotHasPulse(pulseTick + 1u, pulseTick) && snapshotHasPulse(pulseTick + 29u, pulseTick) &&
			session->Get_OutboundMetrics().iSnapshotCoalescedFrameCount == 2u &&
			room.m_Players.at(firstPlayer).iEstherGuardEndTick == pulseTick + 1u,
			"Latest-wins snapshot coalescing retains Bahuntur's same text occurrence for one second without extending protection");
		tests.Require(snapshotHasPulse(pulseTick + 30u, 0u),
			"The retained Bahuntur text occurrence expires at exactly thirty snapshot ticks");
		auto& snapshotPlayer = room.m_Players.at(firstPlayer);
		snapshotPlayer.iCurrentHp = 0u; snapshotPlayer.eAction = PLAYER_ACTION_STATE::DEAD;
		snapshotPlayer.iActionStartTick = pulseTick + 1u;
		tests.Require(snapshotHasPulse(pulseTick + 2u, 0u),
			"A player who died after the block cannot retransmit its old invulnerable text occurrence");
		snapshotPlayer = snapshotSeed;
		snapshotPlayer.isCombatReady = false;
		tests.Require(snapshotHasPulse(pulseTick + 2u, 0u),
			"A player leaving combat readiness cannot retransmit its old invulnerable text occurrence");
		snapshotPlayer = snapshotSeed;
		room.m_eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
		tests.Require(snapshotHasPulse(pulseTick + 2u, 0u),
			"The new Bahuntur retention window does not extend another world's one-tick protection pulse");
		room.m_eWorldId = WORLD_ID::VALTAN_ARENA;
		session->Request_Close();
	}

	{
		auto orbStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& orbRoom = *orbStorage;
		tests.Require(orbRoom.Is_Ready() && !orbRoom.m_WorldPickups.empty(),
			"World ether bootstrap resolves real destruction walls and navigable landing points");
		if (orbRoom.Is_Ready() && !orbRoom.m_WorldPickups.empty())
		{
			const auto descriptor = orbRoom.m_WorldPickups.front().Descriptor;
			const auto parserPath = std::filesystem::temp_directory_path() /
				("lostark-world-pickups-" + std::to_string(GetCurrentProcessId()) + "-" +
				 std::to_string(GetTickCount64()) + ".bootstrap");
			const auto writePickupFile = [&](const int invalid)
			{
				std::ofstream output(parserPath, std::ios::binary | std::ios::trunc);
				output << "LOSTARK_WORLD_PICKUPS 1 \"LV_LUT_HEARTRB_ED\" 30 " << (invalid == 3 ? 2 : 1) << '\n';
				for (int row = 0; row < (invalid == 3 ? 2 : 1); ++row)
					output << std::quoted(descriptor.strPlacementId) << ' ' << std::quoted(descriptor.strWallGroupId) <<
						" 1 " << (invalid == 1 ? 0.f : 10.f) << " 2 1 0 2 " << (invalid == 2 ? 0u : 27u) <<
						' ' << (invalid == 4 ? 0.0001f : 0.75f) << '\n';
				if (invalid == 5) output << "unexpected trailing row\n";
				return output.good();
			};
			std::vector<WORLD_PICKUP_DESCRIPTOR> parsedPickups;
			tests.Require(writePickupFile(0) && Load_WorldPickupsFromFile(parserPath, parsedPickups, status) &&
				parsedPickups.size() == 1u && parsedPickups.front().strPlacementId == descriptor.strPlacementId,
				"World pickup bootstrap parser accepts the published quoted-ID format");
			for (int invalid = 1; invalid <= 5; ++invalid)
			{
				tests.Require(writePickupFile(invalid) && !Load_WorldPickupsFromFile(parserPath, parsedPickups, status) &&
					parsedPickups.size() == 1u && parsedPickups.front().fStartY == 10.f,
					"Invalid drop height, duration, duplicate ID, radius or trailing row preserves prior pickup descriptors");
			}
			std::error_code removeError; std::filesystem::remove(parserPath, removeError);

			const SERVER_NAV_POINT landing{descriptor.fLandingX, descriptor.fLandingY, descriptor.fLandingZ};
			constexpr PLAYER_ID collectorId = 29801u, contenderId = 29802u;
			orbRoom.m_Players.clear();
			orbRoom.m_Players.emplace(collectorId, playerAt(collectorId, landing));
			orbRoom.m_Players.emplace(contenderId, playerAt(contenderId, landing));
			orbRoom.Update_WorldPickups(9u);
			tests.Require(orbRoom.m_WorldPickups.front().Snapshot.eState == WORLD_PICKUP_STATE::WALL &&
				!orbRoom.m_Players.at(collectorId).bRonaunGuard,
				"A wall-mounted ether cannot be collected before its authoritative destruction commit");
			const auto& graph = orbRoom.m_WorldDestructionBootstrap.Get_DescriptorGraph();
			const WORLD_DESTRUCTION_BINDING_DESCRIPTOR* wallBinding = nullptr;
			for (const auto& binding : graph.Bindings)
			{
				if (binding.eTriggerKind != WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT &&
					binding.eTriggerKind != WORLD_DESTRUCTION_TRIGGER_KIND::COLLIDER_CONTACT) continue;
				const auto mutation = std::find_if(graph.Mutations.begin(), graph.Mutations.end(),
					[&](const auto& row) { return row.strMutationId == binding.strMutationId && row.strGroupId == descriptor.strWallGroupId; });
				if (mutation != graph.Mutations.end()) { wallBinding = &binding; break; }
			}
			WORLD_DESTRUCTION_TRANSACTION wallTransaction{};
			WORLD_DESTRUCTION_PREPARE_RESULT preparedWall = WORLD_DESTRUCTION_PREPARE_RESULT::REJECTED;
			if (wallBinding)
			{
				WORLD_DESTRUCTION_ACTION_TUPLE action{};
				action.strPatternId = wallBinding->strPatternId; action.strStageId = wallBinding->strStageId;
				action.strActionId = wallBinding->strActionId; action.iStageIndex = wallBinding->iStageIndex;
				preparedWall = wallBinding->eTriggerKind == WORLD_DESTRUCTION_TRIGGER_KIND::COLLIDER_CONTACT ?
					orbRoom.m_WorldDestructionRuntime.Prepare_ContactTrigger(wallBinding->strImpactReceiverId, 991u, 1u, 10u, wallTransaction, status) :
					orbRoom.m_WorldDestructionRuntime.Prepare_ImpactTrigger(action, wallBinding->strImpactReceiverId, 991u, 1u, 10u, wallTransaction, status);
			}
			const bool wallCommitted = preparedWall == WORLD_DESTRUCTION_PREPARE_RESULT::READY &&
				orbRoom.Commit_WorldDestructionTransaction(wallTransaction, {}, 10u, status);
			tests.Require(wallCommitted && orbRoom.m_WorldPickups.front().Snapshot.eState == WORLD_PICKUP_STATE::FALLING &&
				orbRoom.m_WorldPickups.front().Snapshot.iStateStartTick == 10u,
				"A real dash/contact destruction transaction releases only its wall's ether");
			const std::uint32_t landingTick = 10u + descriptor.iFallDurationTicks;
			orbRoom.Update_WorldPickups(landingTick - 1u);
			tests.Require(orbRoom.m_WorldPickups.front().Snapshot.eState == WORLD_PICKUP_STATE::FALLING &&
				!orbRoom.m_Players.at(collectorId).bRonaunGuard && !orbRoom.m_Players.at(contenderId).bRonaunGuard,
				"An overlapping player cannot collect ether during its fall");
			// Landing is a real collider query: a player above the orb must not collect it.
			orbRoom.m_Players.at(collectorId).fPositionY += 100.f;
			orbRoom.m_Players.at(contenderId).isCombatReady = false;
			orbRoom.Update_WorldPickups(landingTick);
			tests.Require(orbRoom.m_WorldPickups.front().Snapshot.eState == WORLD_PICKUP_STATE::GROUNDED &&
				!orbRoom.m_Players.at(collectorId).bRonaunGuard,
				"Grounded ether rejects vertical separation and a player who is not combat ready");
			orbRoom.m_Players.at(collectorId).fPositionY = landing.y;
			orbRoom.m_Players.at(contenderId).isCombatReady = true;
			orbRoom.Update_WorldPickups(landingTick + 1u);
			orbRoom.Update_WorldPickups(landingTick + 2u);
			auto& collector = orbRoom.m_Players.at(collectorId);
			tests.Require(collector.bRonaunGuard && collector.iRonaunGrantTick == landingTick + 1u &&
				!orbRoom.m_Players.at(contenderId).bRonaunGuard &&
				orbRoom.m_WorldPickups.front().Snapshot.eState == WORLD_PICKUP_STATE::COLLECTED,
				"Simultaneous player collider contact commits exactly one ether grant and never re-grants the removed orb");
			std::vector<DAMAGE_EVENT> etherEvents;
			SERVER_WORLD_TO_PLAYER_HIT ordinary{};
			ordinary.iRawDamage = 100u; ordinary.bIgnoreDefense = true; ordinary.iServerTick = landingTick + 3u;
			const auto ordinaryResult = CServerCombatHitRuntime::Apply_WorldToPlayer(collector, ordinary, orbRoom.m_GameplayCatalog, etherEvents);
			tests.Require(ordinaryResult == SERVER_COMBAT_HIT_RESULT::LANDED && collector.iCurrentHp == 4900u && collector.bRonaunGuard,
				"Ronaun guard neither absorbs nor consumes itself on ordinary damage");
			SERVER_WORLD_TO_PLAYER_HIT wipe = ordinary;
			wipe.bEstherGuardBlockable = true; wipe.iRawDamage = 10000u; wipe.iServerTick += 1u;
			const auto blocked = CServerCombatHitRuntime::Apply_WorldToPlayer(collector, wipe, orbRoom.m_GameplayCatalog, etherEvents);
			tests.Require(blocked == SERVER_COMBAT_HIT_RESULT::ABSORBED && collector.iCurrentHp == 4900u &&
				!collector.bRonaunGuard && collector.iRonaunGrantTick == landingTick + 1u &&
				collector.iInvulnerabilityZonePulseTick == wipe.iServerTick,
				"Ronaun spends its one wipe guard and emits the existing blue invulnerability occurrence");
			constexpr SESSION_ID etherSessionId = 29890u;
			collector.iSessionId = etherSessionId;
			auto etherSession = std::make_shared<CClientSession>(etherSessionId, INVALID_SOCKET,
				CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
			etherSession->m_isSendRunning.store(true);
			orbRoom.m_Sessions.emplace(etherSessionId, etherSession);
			orbRoom.m_PlayerIdBySessionId.emplace(etherSessionId, collectorId);
			orbRoom.m_iServerTick = wipe.iServerTick;
			orbRoom.Broadcast_WorldSnapshot();
			++orbRoom.m_iServerTick; orbRoom.Broadcast_WorldSnapshot();
			bool latestPickupPreserved = false;
			if (etherSession->m_OutboundFrames.size() == 1u)
			{
				const auto& bytes = etherSession->m_OutboundFrames.front().Bytes;
				if (bytes.size() > PACKET_HEADER_BYTES)
				{
					CPacketReader reader{std::span<const std::uint8_t>(bytes).subspan(PACKET_HEADER_BYTES)};
					S2C_WORLD_SNAPSHOT decoded{};
					latestPickupPreserved = Read_Message(reader, decoded) && !decoded.WorldPickups.empty() &&
						decoded.WorldPickups.front().eState == WORLD_PICKUP_STATE::COLLECTED &&
						std::any_of(decoded.Players.begin(), decoded.Players.end(), [&](const auto& row)
						{ return row.iNetEntityId == collector.iNetEntityId && !row.bRonaunGuard &&
							row.iRonaunGrantTick == landingTick + 1u && row.iInvulnerabilityZonePulseTick == wipe.iServerTick; });
				}
			}
			tests.Require(latestPickupPreserved,
				"Coalesced actual World snapshots preserve ether removal, acquisition text and consumed-guard blue text");
			etherSession->Request_Close(); orbRoom.m_Sessions.clear();
			wipe.iServerTick += 1u;
			tests.Require(CServerCombatHitRuntime::Apply_WorldToPlayer(collector, wipe, orbRoom.m_GameplayCatalog, etherEvents) ==
				SERVER_COMBAT_HIT_RESULT::KILLED && collector.iRonaunGrantTick == 0u,
				"A second wipe kills the spent guard holder and clears its acquisition occurrence");
			collector = playerAt(collectorId, landing); collector.bRonaunGuard = true; collector.iRonaunGrantTick = landingTick;
			wipe.bEncounterWipe = true;
			tests.Require(CServerCombatHitRuntime::Apply_WorldToPlayer(collector, wipe, orbRoom.m_GameplayCatalog, etherEvents) ==
				SERVER_COMBAT_HIT_RESULT::KILLED && !collector.bRonaunGuard,
				"Ronaun cannot protect against a forced encounter failure verdict");
			WORLD_DESTRUCTION_ACTION_TUPLE outerAction{};
			outerAction.strPatternId = "VALTAN_ARENA_BREAK_109"; outerAction.strStageId = "IMPACT";
			outerAction.strActionId = "valtan.mechanic.arena-break-109.impact"; outerAction.iStageIndex = 2u;
			WORLD_DESTRUCTION_TRANSACTION outerTransaction{};
			const bool outerCommitted = orbRoom.m_WorldDestructionRuntime.Prepare_StageTrigger(outerAction, 991u, 2u,
				landingTick + 20u, outerTransaction, status) == WORLD_DESTRUCTION_PREPARE_RESULT::READY &&
				orbRoom.Commit_WorldDestructionTransaction(outerTransaction, {}, landingTick + 20u, status);
			tests.Require(outerCommitted && std::all_of(orbRoom.m_WorldPickups.begin(), orbRoom.m_WorldPickups.end(),
				[](const auto& row) { return row.Snapshot.eState == WORLD_PICKUP_STATE::COLLECTED || row.Snapshot.eState == WORLD_PICKUP_STATE::REMOVED; }),
				"Actual 109 arena-break stage commit removes all remaining ether while ordinary dash only releases its own");
			collector = playerAt(collectorId, landing); collector.bRonaunGuard = true; collector.iRonaunGrantTick = landingTick;
			const bool reset = orbRoom.m_WorldDestructionRuntime.Reset(status, landingTick + 30u);
			orbRoom.Update_WorldPickups(landingTick + 30u, false);
			tests.Require(reset && !collector.bRonaunGuard && collector.iRonaunGrantTick == 0u &&
				std::all_of(orbRoom.m_WorldPickups.begin(), orbRoom.m_WorldPickups.end(), [](const auto& row)
				{ return row.Snapshot.eState == WORLD_PICKUP_STATE::WALL && row.Snapshot.fPositionY == row.Descriptor.fStartY; }),
				"Encounter reset restores every ether to its authored wall pose and clears previous Ronaun guards");
		}
	}
	std::cout << "Valtan arena support failures : " << tests.failures << '\n';
	return tests.failures == 0 ? 0 : 1;
}

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
		const bool landsOnTheCenter =
			std::abs(leapArcBoss.fLeapLandingX - highJump->Motion.fLandingX) < 0.01f &&
			std::abs(leapArcBoss.fLeapLandingZ - highJump->Motion.fLandingZ) < 0.01f;
		const bool ignoresPlayerLanding =
			std::hypot(leapArcBoss.fLeapLandingX - leapArcTarget.fPositionX,
				leapArcBoss.fLeapLandingZ - leapArcTarget.fPositionZ) > 1.f;
		tests.Require(
			nullptr != highJump &&
			BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR == highJump->Motion.eKind &&
			1133u == highJump->Motion.iTakeoffStartMs &&
			1500u == highJump->Motion.iTakeoffEndMs &&
			0u == highJump->Motion.iTravelStartMs &&
			267u == highJump->Motion.iTravelEndMs &&
			startedTheJump && landsOnTheCenter && ignoresPlayerLanding &&
			30.f == highJump->Motion.fApexHeight &&
			leapArcBoss.fPatternLeapApexHeight == highJump->Motion.fApexHeight,
			"Land the 30-metre tracking-axe jump at the authored arena centre independently of its player target");

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
					std::abs(leapArcBoss.fPositionX - highJump->Motion.fLandingX) < 0.01f &&
					std::abs(leapArcBoss.fPositionY - highJump->Motion.fLandingY) < 0.01f &&
					std::abs(leapArcBoss.fPositionZ - highJump->Motion.fLandingZ) < 0.01f;
			}
			enteredRecovery = "RECOVERY" == leapArcBoss.strPatternStageId;
		}
		tests.Require(
			heldAtApex && enteredLand && airborneHoldTicks >= 240u && airborneHoldTicks <= 241u &&
			landedInsideFastWindow && enteredRecovery &&
			2u == leapArcBoss.iPatternLeapTravelStageIndex &&
			std::abs(leapArcBoss.fPositionX - highJump->Motion.fLandingX) < 0.01f &&
			std::abs(leapArcBoss.fPositionY - highJump->Motion.fLandingY) < 0.01f &&
			std::abs(leapArcBoss.fPositionZ - highJump->Motion.fLandingZ) < 0.01f,
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
		/* 131 degrees is the mouth of the walk-in corridor. On 2026-09-18 the
		user approved removing ring slots 011/025/026 (120, 132 and 144 degrees)
		to open it again, so that bearing must now let a walker through while
		the 159 gaps and every other bearing stay sealed until the collapse. */
		const auto sweepAcrossRing = [&](const float bearingDegrees,
			const bool expectBlocked = true)
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
			return resolved && expectBlocked == blocked;
		};
		tests.Require(
			room.Is_Ready() && sweepAcrossRing(0.f) && sweepAcrossRing(60.f) &&
			sweepAcrossRing(216.f),
			"Block outward movement through the intact 109 outer ring");
		tests.Require(
			room.Is_Ready() && sweepAcrossRing(150.f) && sweepAcrossRing(294.f),
			"Seal the entrance flank and 159 gaps with the 109 ring");
		tests.Require(
			room.Is_Ready() && sweepAcrossRing(131.f, false),
			"Leave the approved entrance corridor open through the 109 ring");

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


void LostArk::Server::CServerGameplayContractRunner::Run_InannaProtection(
    TESTS& tests, const CGameplayCatalog& catalog)
{
    auto generation = std::make_shared<CGameplayCatalog>(catalog);
    auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA, generation);
    auto otherRoom = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA, generation);
    tests.Require(room->Is_Ready() && otherRoom->Is_Ready(), "Inanna fixtures admit two independent Kouku rooms");
    if (!room->Is_Ready() || !otherRoom->Is_Ready()) return;
    constexpr SESSION_ID casterSession = 741u;
    constexpr std::uint32_t callTick = 1000u;
    constexpr std::uint32_t endTick = callTick + 900u;
    SERVER_PLAYER seed;
    seed.iPlayerId = 1u; seed.iNetEntityId = 101u;
    seed.iCurrentHp = seed.iMaximumHp = 100u; seed.isCombatReady = true;
    seed.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
    room->m_Players.emplace(1u, seed);
    seed.iPlayerId = 2u; seed.iNetEntityId = 102u; seed.fPositionX = 1000.f;
    room->m_Players.emplace(2u, seed);
    seed.iPlayerId = 3u; seed.iNetEntityId = 103u; seed.iCurrentHp = 0u; seed.eAction = PLAYER_ACTION_STATE::DEAD;
    room->m_Players.emplace(3u, seed);
    seed.iPlayerId = 4u; seed.iNetEntityId = 104u; seed.iCurrentHp = 100u; seed.eAction = PLAYER_ACTION_STATE::NONE;
    room->m_Players.emplace(4u, seed);
    otherRoom->m_Players.emplace(4u, seed);
    room->m_PlayerIdBySessionId.emplace(casterSession, 1u);
    room->m_KoukuRaid.State.ePhase = KOUKUSAYDON_RAID_PHASE::COMBAT;
    room->m_KoukuRaid.PlayerIds = {1u, 2u, 3u};
    room->m_iServerTick = callTick - 1u;
    C2S_USE_ESTHER_SKILL use; use.iClientSequence = 1u; use.iSlotIndex = 3u;
    room->Handle_UseEstherSkill(casterSession, use);
    tests.Require(room->m_Players.at(1u).iInvulnerableEndTick == 0u && room->m_PendingEstherSummons.empty(),
        "Rejected Inanna call grants no protection while the gauge is empty");
    room->m_EstherSkillSystem.Update(5.f, true);
    room->Handle_UseEstherSkill(casterSession, use);
    auto& caster = room->m_Players.at(1u);
    auto& peer = room->m_Players.at(2u);
    tests.Require(caster.iInvulnerableEndTick == 0u && peer.iInvulnerableEndTick == 0u &&
        caster.eAction == PLAYER_ACTION_STATE::ESTHER_CAST && room->m_EstherSkillSystem.Get_Gauge() == 0u,
        "Accepted Inanna protects nobody until her zone opens");
    tests.Require(room->m_PendingEstherSummons.size() == 1u &&
        room->m_PendingEstherSummons.front().pRosterEntry->eEstherId == ESTHER_ID::INANNA &&
        room->m_PendingEstherSummons.front().pRosterEntry->iStrikeMs == 4100u,
        "Inanna keeps the authored 4.1-second summon presentation separate from her protection zone");
    const auto* zone = LostArk::Shared::EstherStrike::Find_Zone(ESTHER_ID::INANNA);
    tests.Require(nullptr != zone && 7.f == zone->fRadiusM && 10000u == zone->iDurationMs,
        "Inanna's zone is the source 700 cm aura held for ten seconds");
    if (nullptr == zone) return;
    constexpr std::uint32_t zoneTick = callTick + 90u;
    const std::uint32_t zoneEndTick = zoneTick + CKoukuSaydonLogicRuntime::Ticks_FromMs(zone->iDurationMs);
    peer.fPositionX = 3.f;
    room->m_Players.at(3u).fPositionX = 0.f;
    room->m_Players.at(4u).fPositionX = 0.f;
    caster.iMaximumMadness = 100u; caster.iCurrentMadness = 50u;
    room->Open_EstherZone(*zone, 0.f, 0.f, zoneTick);
    room->Update_EstherZones(zoneTick);
    tests.Require(caster.iInvulnerableEndTick == zoneTick + 2u && peer.iInvulnerableEndTick == zoneTick + 2u &&
        room->m_Players.at(3u).iInvulnerableEndTick == 0u && room->m_Players.at(4u).iInvulnerableEndTick == 0u &&
        otherRoom->m_Players.at(4u).iInvulnerableEndTick == 0u,
        "Inanna's zone protects only living raid participants inside it and never leaks into another Server room");
    tests.Require(caster.iCurrentMadness == 40u, "Inanna's first pulse drains ten percent madness as the zone opens");
    room->Update_EstherZones(zoneTick + 29u);
    tests.Require(caster.iCurrentMadness == 40u, "Inanna's madness drain waits for the next one-second pulse");
    room->Update_EstherZones(zoneTick + 30u);
    tests.Require(caster.iCurrentMadness == 30u, "Inanna drains another ten percent madness every second");
    std::vector<DAMAGE_EVENT> events;
    SERVER_WORLD_TO_PLAYER_HIT hit; hit.iRawDamage = 10u; hit.bIgnoreDefense = true;
    hit.iServerTick = zoneTick + 30u;
    tests.Require(CServerCombatHitRuntime::Apply_WorldToPlayer(caster, hit, catalog, events) == SERVER_COMBAT_HIT_RESULT::ABSORBED &&
        CServerCombatHitRuntime::Apply_WorldToPlayer(peer, hit, catalog, events) == SERVER_COMBAT_HIT_RESULT::ABSORBED &&
        caster.iCurrentHp == 100u && peer.iCurrentHp == 100u && events.empty(),
        "Inanna's zone absorbs ordinary Server damage for everyone standing in it");
    peer.fPositionX = 8.f;
    room->Update_EstherZones(zoneTick + 31u);
    auto left = peer; hit.iServerTick = zoneTick + 32u;
    tests.Require(CServerCombatHitRuntime::Apply_WorldToPlayer(left, hit, catalog, events) == SERVER_COMBAT_HIT_RESULT::LANDED && left.iCurrentHp == 90u,
        "Leaving Inanna's zone ends the protection within two ticks");
    auto unrelatedWipe = caster; hit.iServerTick = zoneTick + 31u; hit.bEncounterWipe = true;
    tests.Require(CServerCombatHitRuntime::Apply_WorldToPlayer(unrelatedWipe, hit, catalog, events) == SERVER_COMBAT_HIT_RESULT::KILLED &&
        unrelatedWipe.iCurrentHp == 0u,
        "Other encounter wipes still bypass Inanna through the unchanged global damage arbiter");
    events.clear();
    room->m_TickDamageEvents.clear();
    caster.iCurrentHp = 50u;
    room->Update_EstherZones(zoneEndTick);
    tests.Require(caster.iCurrentHp == 85u && peer.iCurrentHp == 100u && room->m_EstherZones.empty() &&
        room->m_TickDamageEvents.size() == 1u && DAMAGE_HIT_FLAG::HEAL == room->m_TickDamageEvents.front().eHitFlag,
        "Inanna's zone release heals thirty-five percent of maximum HP for the participants still inside");
    caster.iInvulnerableEndTick = peer.iInvulnerableEndTick = endTick;
    peer.fPositionX = 1000.f;
    auto& audition = room->m_KoukuSaydonPatternAudition;
    audition.ePhase = CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
    audition.iRoomAuditionEpoch = 741u;
    audition.PinnedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
    audition.pProductGeneration = generation;
    audition.iPinnedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(*generation);
    const auto* placement = room->Find_Placement("boss.kakulsaydon.bingo.saydon");
    SERVER_WORLD_ENTITY boss;
    const bool ready = placement && room->Build_WorldEntity(*placement, 900u, boss) && room->Resolve_KoukuProductCatalog();
    tests.Require(ready, "Inanna black-hole fixture uses the admitted Bingo boss and pinned Product catalog");
    if (!ready) return;
    boss.iPatternSequence = 1u; boss.strPatternId = "test.inanna.blackhole";
    boss.iCurrentHp = boss.iMaximumHp = 10000u; boss.iMaximumHealthBars = 100u;
    room->m_WorldEntities.clear(); room->m_WorldEntities.push_back(std::move(boss));
    auto& owner = room->m_WorldEntities.front();
    room->m_KoukuBingo.Reset();
    room->m_KoukuBingoDuration.bLastLineCompletionSucceeded = false;
    BOSS_PATTERN_MECHANIC_TRIGGER detonation; detonation.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_DETONATION;
    room->m_PendingKoukuMechanicTriggers.push_back({owner.iNetEntityId, owner.iPatternSequence, detonation});
    const auto protectedCasterHp = caster.iCurrentHp;
    room->Commit_KoukuMechanicTriggers(endTick - 1u);
    tests.Require(caster.iCurrentHp == protectedCasterHp && peer.iCurrentHp == 100u && owner.iCurrentHp == 10000u &&
        room->m_KoukuBingo.Get_RedMask() == 0u,
        "Inanna survives the Bingo black hole with zero red lines until the last protected tick without earning boss damage");
    tests.Require(caster.iInvulnerabilityZonePulseTick == endTick - 1u && peer.iInvulnerabilityZonePulseTick == endTick - 1u &&
        caster.iInvulnerabilityZoneContactTick == endTick - 1u && peer.iInvulnerabilityZoneContactTick == endTick - 1u &&
        !room->m_Players.at(3u).iInvulnerabilityZonePulseTick && !room->m_Players.at(4u).iInvulnerabilityZonePulseTick,
        "Inanna emits the identical black-hole protection pulse only for living protected raid participants");
    room->m_PendingKoukuMechanicTriggers.push_back({owner.iNetEntityId, owner.iPatternSequence, detonation});
    room->Commit_KoukuMechanicTriggers(endTick);
    tests.Require(caster.iCurrentHp == 0u && peer.iCurrentHp == 0u && owner.iCurrentHp == 10000u && room->m_Players.at(4u).iCurrentHp == 100u,
        "Bingo black hole wipes unprotected raid participants at exact Inanna expiry while preserving nonparticipants");
    caster.iCurrentHp = peer.iCurrentHp = 100u;
    caster.eAction = peer.eAction = PLAYER_ACTION_STATE::NONE;
    caster.iInvulnerableEndTick = peer.iInvulnerableEndTick = endTick;
    caster.iShield = peer.iShield = 5000u;
    room->m_KoukuBingoDuration.bLastLineCompletionSucceeded = true;
    room->m_PendingKoukuMechanicTriggers.push_back({owner.iNetEntityId, owner.iPatternSequence, detonation});
    room->Commit_KoukuMechanicTriggers(endTick - 1u);
    tests.Require(caster.iCurrentHp == 100u && peer.iCurrentHp == 100u && caster.iShield == 5000u && peer.iShield == 5000u &&
        owner.iCurrentHp == 8700u && room->m_Players.at(4u).iCurrentHp == 100u,
        "Successful Bingo judgement damages thirteen boss bars while active protection saves only raid participants from the blast");
    room->m_PendingKoukuMechanicTriggers.push_back({owner.iNetEntityId, owner.iPatternSequence, detonation});
    room->Commit_KoukuMechanicTriggers(endTick);
    tests.Require(caster.iCurrentHp == 0u && peer.iCurrentHp == 0u && caster.iShield == 0u && peer.iShield == 0u &&
        owner.iCurrentHp == 7400u && room->m_Players.at(4u).iCurrentHp == 100u,
        "An earlier successful line reward cannot let shields survive the black hole after Inanna's protection expires");

    // A new gate must not inherit a previous gate's delayed strike or aura.
    auto& resetPlayer = otherRoom->m_Players.at(4u);
    resetPlayer.iCurrentHp = 50u; resetPlayer.iEstherGuardEndTick = zoneEndTick;
    resetPlayer.iEstherGuardDamageTakenPercent = -50;
    otherRoom->Open_EstherZone(*zone, 0.f, 0.f, zoneTick);
    auto& pending = otherRoom->m_PendingEstherSummons.emplace_back();
    pending.pRosterEntry = Find_EstherDefinition(ESTHER_ID::INANNA);
    pending.iCasterPlayerId = 4u;
    SERVER_WORLD_ENTITY priorSummon;
    priorSummon.iNetEntityId = 99001u; priorSummon.eKind = WORLD_BOOTSTRAP_KIND::NPC;
    priorSummon.isEstherSummon = true; priorSummon.strArchetypeId = "NPC_59620";
    otherRoom->m_WorldEntities.push_back(std::move(priorSummon));
    const auto hasPriorSummon = [&]() { return std::any_of(otherRoom->m_WorldEntities.begin(),
        otherRoom->m_WorldEntities.end(), [](const auto& entity) { return entity.iNetEntityId == 99001u; }); };
    tests.Require(otherRoom->Despawn_KoukuSaydonArenaDebugEntities(true, true) && hasPriorSummon() &&
        otherRoom->m_PendingEstherSummons.size() == 1u && otherRoom->m_EstherZones.size() == 1u &&
        resetPlayer.iEstherGuardEndTick == zoneEndTick,
        "Gate preflight preserves active, delayed and support Esther state");
    tests.Require(otherRoom->Despawn_KoukuSaydonArenaDebugEntities(true) && !hasPriorSummon() &&
        otherRoom->m_PendingEstherSummons.empty() && otherRoom->m_EstherZones.empty() &&
        resetPlayer.iEstherGuardEndTick == 0u && resetPlayer.iEstherGuardDamageTakenPercent == 0,
        "Committed gate reset ends all previous Esther strikes and protection");
    otherRoom->Update_EstherZones(zoneEndTick);
    otherRoom->Update_PendingEstherSummons(2.f);
    tests.Require(resetPlayer.iCurrentHp == 50u && !hasPriorSummon() && otherRoom->m_TickDamageEvents.empty(),
        "Reset Esther cannot heal, strike or respawn in the next encounter");
}
