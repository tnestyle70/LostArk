#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameplayCatalog.h"
#include "PlayerSkillSystem.h"
#include "ServerCombatHitRuntime.h"
#include "ServerNavigation.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_SkillStages(TESTS& tests, CGameplayCatalog& catalog)
{
	{
		// Exercise the real skill mover and body slide on a small floor with an
		// open outer edge and a two-metre deck step. The original dash is safe;
		// only collision's redirected segment leaves the floor.
		namespace fs = std::filesystem;
		const fs::path root = fs::temp_directory_path() /
			(L"LostArkSkillSlideContract-" + std::to_wstring(_getpid()) + L"-" +
				std::to_wstring(GetTickCount64()));
		std::error_code fixtureError;
		fs::create_directories(root / L"Navigation", fixtureError);
		bool fixtureWritten = !fixtureError;
		if (fixtureWritten)
		{
			for (const bool raisedFloor : { false, true })
			{
				const std::string id = raisedFloor ? "SKILL_SLIDE_STEP" : "SKILL_SLIDE";
				std::ofstream grid(root / L"Navigation" / (id + ".navgrid"), std::ios::binary);
				const std::uint32_t width = 6u, height = 6u;
				const float cellSize = 1.f, origin = 0.f;
				std::array<std::uint8_t, 36u> walkable{}; walkable.fill(1u);
				std::array<float, 36u> heights{};
				for (std::size_t i = 0u; i < heights.size(); ++i)
					heights[i] = raisedFloor ? 2.f + static_cast<float>(3u - (std::min)(3u,
						static_cast<unsigned>(i / width))) : (i % width < 3u ? 2.f : 4.f);
				grid.write(reinterpret_cast<const char*>(&width), sizeof(width));
				grid.write(reinterpret_cast<const char*>(&height), sizeof(height));
				grid.write(reinterpret_cast<const char*>(&cellSize), sizeof(cellSize));
				grid.write(reinterpret_cast<const char*>(&origin), sizeof(origin));
				grid.write(reinterpret_cast<const char*>(&origin), sizeof(origin));
				grid.write(reinterpret_cast<const char*>(walkable.data()), walkable.size());
				grid.write(reinterpret_cast<const char*>(heights.data()), sizeof(heights));
				std::ofstream policy(root / L"Navigation" / (id + ".navpolicy"), std::ios::binary);
				policy << "LOSTARK_NAVIGATION_POLICY 1 \"" << id << "\" 1\n";
				fixtureWritten = fixtureWritten && grid.good() && policy.good();
			}
		}
		wchar_t previousRoot[32768]{};
		const DWORD previousLength = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
			previousRoot, static_cast<DWORD>(std::size(previousRoot)));
		CServerNavigation navigation, steppedNavigation;
		const bool loaded = fixtureWritten && SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", root.c_str()) &&
			navigation.Load("SKILL_SLIDE") && steppedNavigation.Load("SKILL_SLIDE_STEP");
		SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
			previousLength && previousLength < std::size(previousRoot) ? previousRoot : nullptr);
		auto fixture = std::make_unique<CGameplayCatalog>(catalog);
		auto* dodge = const_cast<PLAYER_SKILL_DEFINITION*>(fixture->Find_Skill(34020u));
		tests.Require(loaded && dodge && Is_DodgeSkill(*dodge),
			"Load a floor-edge fixture and the published Space dodge for collision-slide regression");
		if (loaded && dodge)
		{
			dodge->RootMotion.clear(); dodge->ComboStages.clear(); dodge->Hits.clear(); dodge->Projectiles.clear();
			dodge->fMovementDistance = 3.f; dodge->fRootMotionScale = 1.f;
			dodge->iActionDurationMs = 1000u; dodge->iResourceCost = dodge->iIdentityCost = 0u;
			dodge->eSkillKind = PLAYER_SKILL_KIND::ACTIVE;
			dodge->strDamageProfileId.clear();
			const auto playerAt = [](const float x, const float y, const float z) {
				SERVER_PLAYER player{};
				player.iPlayerId = 99880u; player.iNetEntityId = 99881u;
				player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
				player.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
				player.iCurrentHp = player.iMaximumHp = 1000u;
				player.iCurrentResource = player.iMaximumResource = 1000u;
				player.fPositionX = x; player.fPositionY = y; player.fPositionZ = z;
				return player;
			};
			const auto runDodge = [&](SERVER_PLAYER& player, const CServerNavigation* nav,
				const CServerCollisionSystem* collision) {
				CPlayerSkillSystem skills;
				C2S_USE_SKILL command{}; command.iSkillId = dodge->iSkillId; command.iClientSequence = 1u;
				command.fAimX = player.fPositionX + 5.f; command.fAimZ = player.fPositionZ;
				const bool started = skills.Try_Start(player, command, *fixture, 1u, nav);
				std::vector<SERVER_WORLD_ENTITY> entities; std::vector<DAMAGE_EVENT> events;
				if (started) skills.Update(player, entities, *fixture, nav, collision, .5f, 16u, events);
				return started;
			};
			CServerCollisionSystem edgeBody;
			edgeBody.Set_BlockingBodies({ SERVER_BLOCKING_BODY{2.f, .2f, .4f} });
			auto edge = playerAt(1.f, 2.f, .2f);
			float rawX = 0.f, rawY = 0.f, rawZ = 0.f; bool blocked = false;
			SERVER_NAV_POINT rawGround{};
			tests.Require(edgeBody.Resolve_PlayerMove(edge, 2.5f, 2.f, .2f, rawX, rawY, rawZ, blocked) &&
				rawZ < 0.f && !navigation.Sample_Position(rawX, rawZ, rawGround, rawY),
				"A real body tangent can redirect an otherwise walkable dodge beyond the floor edge");
			tests.Require(runDodge(edge, &navigation, &edgeBody) && edge.eAction == PLAYER_ACTION_STATE::SKILL &&
				edge.fPositionX == 1.f && edge.fPositionY == 2.f && edge.fPositionZ == .2f,
				"Reject the off-floor collision slide while preserving the last safe pose and active dodge");
			auto ordinary = playerAt(1.f, 2.f, .2f);
			tests.Require(runDodge(ordinary, &navigation, nullptr) && std::abs(ordinary.fPositionX - 2.5f) < .0001f &&
				ordinary.fPositionY == 2.f && ordinary.fPositionZ == .2f,
				"Keep ordinary Space displacement and the sampled floor height");
			CServerCollisionSystem insideBody;
			insideBody.Set_BlockingBodies({ SERVER_BLOCKING_BODY{2.f, 2.f, .4f} });
			auto inside = playerAt(1.f, 2.f, 2.f);
			tests.Require(runDodge(inside, &navigation, &insideBody) && inside.fPositionX > 1.f && inside.fPositionZ < 2.f &&
				navigation.Is_PointWalkableExact(inside.fPositionX, inside.fPositionZ, inside.fPositionY) && inside.fPositionY == 2.f,
				"Preserve a body tangent slide that stays on the same walkable floor");
			// The tangent remains at Y=2; its final XZ lies one allowed step up.
			// An overhead body is missed at Y=2 but overlaps the grounded Y=3 pose.
			CServerCollisionSystem overheadBody;
			overheadBody.Set_BlockingBodies({ SERVER_BLOCKING_BODY{2.f, 3.5f, .4f},
				SERVER_BLOCKING_BODY{1.149f, 2.149f, .2f, 4.5f, .1f} });
			auto raised = playerAt(1.f, 2.f, 3.5f);
			tests.Require(overheadBody.Resolve_PlayerMove(raised, 2.5f, 2.f, 3.5f, rawX, rawY, rawZ, blocked) &&
				rawY == 2.f && steppedNavigation.Sample_Position(rawX, rawZ, rawGround, rawY) && rawGround.y == 3.f &&
				overheadBody.Is_PlayerPositionClear(rawX, rawY, rawZ, raised.iNetEntityId) &&
				!overheadBody.Is_PlayerPositionClear(rawX, rawGround.y, rawZ, raised.iNetEntityId),
				"Reproduce a collision-safe tangent whose navigation grounding enters an overhead body");
			tests.Require(runDodge(raised, &steppedNavigation, &overheadBody) &&
				raised.eAction == PLAYER_ACTION_STATE::SKILL && raised.fPositionX == 1.f &&
				raised.fPositionY == 2.f && raised.fPositionZ == 3.5f,
				"Reject a blocked post-slide grounding without changing the pose or cancelling Space");
			CServerCollisionSystem safeStepBody;
			safeStepBody.Set_BlockingBodies({ SERVER_BLOCKING_BODY{2.f, 3.5f, .4f} });
			auto safeStep = playerAt(1.f, 2.f, 3.5f);
			tests.Require(runDodge(safeStep, &steppedNavigation, &safeStepBody) && safeStep.fPositionX > 1.f &&
				safeStep.fPositionZ < 3.f && safeStep.fPositionY == 3.f &&
				safeStepBody.Is_PlayerPositionClear(safeStep.fPositionX, safeStep.fPositionY, safeStep.fPositionZ,
					safeStep.iNetEntityId),
				"Preserve a clear tangent and its traversable one-metre height correction");
			// Three individually traversable steps put the final pose above this
			// thin body. Endpoint overlap alone would miss crossing it vertically.
			CServerCollisionSystem crossedBody;
			crossedBody.Set_BlockingBodies({ SERVER_BLOCKING_BODY{2.f, 3.5f, .4f},
				SERVER_BLOCKING_BODY{1.149f, .149f, .2f, 4.1f, .1f} });
			auto crossing = playerAt(1.f, 2.f, 3.5f);
			tests.Require(crossedBody.Resolve_PlayerMove(crossing, 4.5f, 2.f, 3.5f, rawX, rawY, rawZ, blocked) &&
				rawY == 2.f && steppedNavigation.Sample_Position(rawX, rawZ, rawGround, rawY) && rawGround.y == 5.f &&
				crossedBody.Is_PlayerPositionClear(rawX, rawY, rawZ, crossing.iNetEntityId) &&
				crossedBody.Is_PlayerPositionClear(rawX, rawGround.y, rawZ, crossing.iNetEntityId),
				"A multi-step tangent has clear grounding endpoints with a thin blocker between their heights");
			dodge->fMovementDistance = 7.f;
			tests.Require(runDodge(crossing, &steppedNavigation, &crossedBody) &&
				crossing.eAction == PLAYER_ACTION_STATE::SKILL && crossing.fPositionX == 1.f &&
				crossing.fPositionY == 2.f && crossing.fPositionZ == 3.5f,
				"Sweep the grounding correction so clear endpoints cannot tunnel through an intervening body");
			CServerCollisionSystem overlappingBody;
			overlappingBody.Set_BlockingBodies({ SERVER_BLOCKING_BODY{1.f, 2.5f, .4f} });
			auto escaping = playerAt(1.f, 2.f, 2.5f);
			dodge->fMovementDistance = .2f;
			tests.Require(runDodge(escaping, &navigation, &overlappingBody) && escaping.fPositionX > 1.f &&
				escaping.fPositionY == 2.f && !overlappingBody.Is_PlayerPositionClear(
					escaping.fPositionX, escaping.fPositionY, escaping.fPositionZ, escaping.iNetEntityId),
				"Unchanged-height movement retains gradual escape from a pre-existing body overlap");
			dodge->fMovementDistance = 3.f;
			auto deck = playerAt(2.8f, 2.f, 2.f);
			tests.Require(runDodge(deck, &navigation, nullptr) && deck.fPositionX >= 2.8f && deck.fPositionX < 3.f &&
				deck.fPositionY == 2.f && deck.fPositionZ == 2.f,
				"Keep the existing per-segment clamp before a forbidden deck-height step");
			auto outside = playerAt(-1.f, 7.f, 2.f);
			tests.Require(runDodge(outside, &navigation, nullptr) && outside.fPositionX == -1.f &&
				outside.fPositionY == 7.f && outside.fPositionZ == 2.f,
				"An already off-grid dodge retains its original XYZ instead of snapping to another cell");
			auto withoutNavigation = playerAt(-1.f, 7.f, 2.f);
			tests.Require(runDodge(withoutNavigation, nullptr, nullptr) &&
				std::abs(withoutNavigation.fPositionX - .5f) < .0001f && withoutNavigation.fPositionY == 7.f,
				"Retain the existing navigation-absent skill movement contract");
		}
		fs::remove_all(root, fixtureError);
	}
	{
		SERVER_WORLD_ENTITY boss{};
		boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS; boss.strArchetypeId = "BOSS_KAKULSAYDON_G1_SAYDON";
		boss.iNetEntityId = 99801u; boss.iCurrentHp = boss.iMaximumHp = 1000u;
		boss.iPatternSequence = 1u; boss.strPatternId = "test.combat.success"; boss.strActionId = "test.combat.window";
		CBossCombatRuntime::Set_StaggerGauge(boss.BossCombat, 100u);
		CBossCombatRuntime::Set_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, true);
		SERVER_PLAYER_TO_WORLD_HIT hit{}; hit.iSkillId = 34040u; hit.iSourcePlayerId = 1u; hit.iServerTick = 1u;
		hit.iStaggerDamage = 99u; hit.iRawDamage = 99000u; hit.bHealthDamageDisabled = true;
		std::vector<DAMAGE_EVENT> events;
		(void)CServerCombatHitRuntime::Apply_PlayerToWorld(boss, hit, events);
		tests.Require(events.size() == 1u && !events.front().isStaggerSuccess && !events.front().isCounterSuccess &&
			events.front().iAmount == 0u && events.front().iStaggerAmount == 99u,
			"A hit below the boss stagger threshold carries gauge contribution without a success notification");
		hit.iStaggerDamage = 1u; hit.iRawDamage = 1000u; hit.iCounterPower = 1u; hit.iServerTick = 2u;
		(void)CServerCombatHitRuntime::Apply_PlayerToWorld(boss, hit, events);
		tests.Require(events.size() == 2u && events.back().isStaggerSuccess && events.back().isCounterSuccess &&
			events.back().isOutgoing && events.back().iSourcePlayerId == 1u && events.back().iAmount == 0u &&
			boss.iCurrentHp == 1000u && boss.BossCombat.iStaggerCurrent == 100u,
			"The authoritative hit publishes independent counter and stagger success with zero HP damage");
		hit.iRawDamage = 1u; hit.bHealthDamageDisabled = false; hit.iServerTick = 3u;
		(void)CServerCombatHitRuntime::Apply_PlayerToWorld(boss, hit, events);
		tests.Require(events.size() == 3u && !events.back().isStaggerSuccess && !events.back().isCounterSuccess &&
			std::count_if(events.begin(), events.end(), [](const auto& event) { return event.isStaggerSuccess; }) == 1,
			"Later hits cannot repeat the closed counter or completed stagger success notification");
	}
	{
		// Cast the real skill pipeline with separate DAMAGE/COUNTER/STAGGER rows.
		// Damage shares must feed stagger without double HP loss or double credit.
		for (int path = 0; path != 3; ++path) for (const bool reduction : { false, true })
		{
			auto fixture = std::make_unique<CGameplayCatalog>(catalog);
			auto* skill = const_cast<PLAYER_SKILL_DEFINITION*>(fixture->Find_Skill(34040u));
			auto* profile = const_cast<PLAYER_RUNTIME_PROFILE*>(fixture->Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER));
			auto* damage = skill ? const_cast<CGameplayCatalog::DAMAGE_PROFILE*>(fixture->Find_DamageProfile(skill->strDamageProfileId)) : nullptr;
			tests.Require(skill && profile && damage, "Resolve the published cast's skill/player/damage catalogue for channel parity");
			if (!skill || !profile || !damage) continue;
			profile->iAttackPower = 1000003u; profile->iCriticalChancePercent = 0u;
			damage->iAttackCoefficientBp = 10000u; damage->iDamageAddend = 0u;
			damage->iDamageSpreadPercent = 0u; damage->iBossHealthBarDamage = 0u;
			skill->RootMotion.clear(); skill->ComboStages.clear(); skill->Hits.clear(); skill->Projectiles.clear();
			skill->eSkillKind = PLAYER_SKILL_KIND::ACTIVE; skill->fMovementDistance = 0.f;
			skill->iActionDurationMs = 500u; skill->iResourceCost = skill->iIdentityCost = 0u;
			skill->iStaggerDamage = 137u; skill->iCounterPower = skill->iPartDamage = 0u;
			skill->fMaximumRange = 4.f;
			PLAYER_SKILL_PROJECTILE projectile{};
			projectile.eKind = PLAYER_PROJECTILE_KIND::FIXAREA;
			projectile.eOrigin = PLAYER_PROJECTILE_ORIGIN::CASTER;
			projectile.iLifeMs = 500u; projectile.fRadius = 4.f;
			for (const auto kind : { 1u, 3u, 2u })
			{
				PLAYER_SKILL_HIT shape{};
				shape.iTimeMs = 100u; shape.iRepeatCount = 3u; shape.iRepeatMs = 100u;
				shape.iAreaType = 1u; shape.fRange = 4.f; shape.fHeight = 2.f; shape.iMaxTargets = 1u;
				shape.iResultKind = kind;
				if (path == 0) skill->Hits.push_back(shape);
				else { PLAYER_PROJECTILE_HIT hit{}; hit.Hit = shape; hit.isContact = path == 2; projectile.Hits.push_back(hit); }
			}
			if (path) skill->Projectiles.push_back(projectile);
			SERVER_WORLD_ENTITY boss{};
			boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS; boss.strArchetypeId = "BOSS_KAKULSAYDON_G1_SAYDON";
			boss.iNetEntityId = 99851u; boss.fPositionZ = 1.f; boss.fCollisionRadius = 1.f;
			boss.iCurrentHp = boss.iMaximumHp = 10000000u;
			boss.iKoukuDamageReductionWindows = reduction ? 1u : 0u;
			CBossCombatRuntime::Set_StaggerGauge(boss.BossCombat, 40000u);
			std::vector<SERVER_WORLD_ENTITY> targets{ boss };
			SERVER_PLAYER player{}; player.iPlayerId = 99850u;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
			player.iCurrentHp = player.iMaximumHp = 10000u;
			player.iCurrentResource = player.iMaximumResource = 10000u;
			C2S_USE_SKILL command{}; command.iSkillId = skill->iSkillId; command.iClientSequence = 1u; command.fAimZ = 3.f;
			CPlayerSkillSystem runtime; std::vector<DAMAGE_EVENT> events;
			const bool started = runtime.Try_Start(player, command, *fixture, 1u);
			if (started) for (std::uint32_t tick = 2u; tick != 40u; ++tick)
				runtime.Update(player, targets, *fixture, nullptr, nullptr, 1.f / 30.f, tick, events);
			const auto hpDamage = reduction ? 999u : 1000003u;
			tests.Require(started && boss.iCurrentHp - targets.front().iCurrentHp == hpDamage &&
				targets.front().BossCombat.iStaggerCurrent == 999u &&
				std::count_if(events.begin(), events.end(), [](const auto& event) { return event.iAmount != 0u; }) == 3 &&
				std::count_if(events.begin(), events.end(), [](const auto& event) { return event.iStaggerAmount != 0u; }) == 3,
				"Caster and timed/contact projectile triple-result casts preserve one HP channel and damage/1000 stagger independently of Mario reduction");
		}
	}
	{
		SERVER_WORLD_ENTITY largeSaydon{};
		largeSaydon.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		largeSaydon.strArchetypeId = "BOSS_KAKULSAYDON_G2_BIG_SAYDON";
		largeSaydon.iNetEntityId = 99901u;
		largeSaydon.iCurrentHp = largeSaydon.iMaximumHp = 100000u;
		largeSaydon.fPositionZ = 1.f; largeSaydon.fCollisionRadius = 1.f;
		CBossCombatRuntime::Set_Shield(largeSaydon.BossCombat, 100u);
		CBossCombatRuntime::Set_StaggerGauge(largeSaydon.BossCombat, 100u);
		CBossCombatRuntime::Set_Flag(largeSaydon.BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, true);
		const auto revision = largeSaydon.BossCombat.iStateRevision;
		SERVER_PLAYER_TO_WORLD_HIT incoming{};
		incoming.iSkillId = 34040u; incoming.iSourcePlayerId = 1u; incoming.iServerTick = 1u;
		incoming.iRawDamage = 200u; incoming.iStaggerDamage = 100u;
		incoming.iPartDamage = 100u; incoming.iCounterPower = 1u;
		std::vector<DAMAGE_EVENT> events;
		tests.Require(!Is_PlayerDamageableWorldArchetype(largeSaydon.strArchetypeId) &&
			CServerCombatHitRuntime::Apply_PlayerToWorld(largeSaydon, incoming, events) == SERVER_COMBAT_HIT_RESULT::NOT_ADMITTED &&
			largeSaydon.iCurrentHp == largeSaydon.iMaximumHp && largeSaydon.BossCombat.iShieldCurrent == 100u &&
			!largeSaydon.BossCombat.iStaggerCurrent && largeSaydon.BossCombat.iStateRevision == revision &&
			largeSaydon.BossCombat.PendingOutcomes.empty() && largeSaydon.MvpLedger.empty() && events.empty(),
			"Gate 2 Large Saydon rejects player hits before HP, shield, stagger, counter and damage-event mutation");
		for (int path = 0; path != 4; ++path)
		{
			auto fixture = std::make_unique<CGameplayCatalog>(catalog);
			auto* skill = const_cast<PLAYER_SKILL_DEFINITION*>(fixture->Find_Skill(34040u));
			tests.Require(nullptr != skill, "The player-target filter fixture resolves a product skill");
			if (!skill) continue;
			skill->RootMotion.clear(); skill->Projectiles.clear(); skill->Hits.clear();
			skill->fMovementDistance = 0.f; skill->iResourceCost = 0u;
			skill->iActionDurationMs = 500u; skill->iHitTimeMs = 100u;
			skill->fMaximumRange = 4.f;
			PLAYER_SKILL_HIT shape{}; shape.iTimeMs = 100u; shape.iAreaType = 1u;
			shape.fRange = 4.f; shape.fHeight = 2.f; shape.iMaxTargets = 1u;
			if (path == 0) skill->Hits.push_back(shape);
			else if (path == 1 || path == 2)
			{
				PLAYER_SKILL_PROJECTILE projectile{};
				projectile.eKind = PLAYER_PROJECTILE_KIND::FIXAREA;
				projectile.eOrigin = PLAYER_PROJECTILE_ORIGIN::CASTER;
				projectile.iLifeMs = 500u; projectile.fRadius = 4.f;
				PLAYER_PROJECTILE_HIT hit{}; hit.Hit = shape; hit.isContact = path == 2;
				projectile.Hits.push_back(hit); skill->Projectiles.push_back(projectile);
			}
			auto kouku = largeSaydon;
			kouku.strArchetypeId = "BOSS_KAKULSAYDON_G2_KOUKU";
			kouku.iNetEntityId = 99902u; kouku.fPositionZ = 2.f;
			kouku.BossCombat = {};
			std::vector<SERVER_WORLD_ENTITY> targets{ largeSaydon, kouku };
			SERVER_PLAYER player{}; player.iPlayerId = 99900u;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
			player.iCurrentHp = player.iMaximumHp = 10000u;
			player.iCurrentResource = player.iMaximumResource = 10000u;
			C2S_USE_SKILL command{}; command.iSkillId = skill->iSkillId;
			command.iClientSequence = 1u; command.fAimZ = 3.f;
			CPlayerSkillSystem runtime;
			const bool started = runtime.Try_Start(player, command, *fixture, 1u);
			events.clear();
			if (started)
				for (std::uint32_t tick = 2u; tick < 40u; ++tick)
					runtime.Update(player, targets, *fixture, nullptr, nullptr, 1.f / 30.f, tick, events);
			tests.Require(started && targets[0].iCurrentHp == largeSaydon.iCurrentHp &&
				targets[0].BossCombat.iStateRevision == revision && targets[1].iCurrentHp < kouku.iCurrentHp &&
				!events.empty() && std::all_of(events.begin(), events.end(), [&](const DAMAGE_EVENT& event) {
					return event.iTargetNetEntityId == kouku.iNetEntityId; }),
				"Caster, timed/contact projectile and fallback targeting skip Large Saydon before the nearest/maximum-target limit");
		}
	}
	{
		SERVER_PLAYER shielded{};
		shielded.iNetEntityId = 991u; shielded.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		shielded.iCurrentHp = shielded.iMaximumHp = 100u; shielded.iShield = 5u;
		shielded.isCombatReady = true;
		SERVER_WORLD_TO_PLAYER_HIT hit{};
		hit.iRawDamage = 7u; hit.iServerTick = 30u; hit.bIgnoreDefense = true; hit.bIgnoreCounter = true;
		std::vector<DAMAGE_EVENT> events;
		(void)CServerCombatHitRuntime::Apply_WorldToPlayer(shielded, hit, catalog, events);
		tests.Require(shielded.iCurrentHp == 98u && shielded.iShield == 0u && events.size() == 2u &&
			events[0].eHitFlag == DAMAGE_HIT_FLAG::ABSORB && events[0].iAmount == 5u &&
			events[1].eHitFlag == DAMAGE_HIT_FLAG::NORMAL && events[1].iAmount == 2u,
			"Partial shield hit sends absorption and only actual HP damage separately");
		shielded.iShield = 10u; shielded.iCurrentHp = 100u; events.clear();
		(void)CServerCombatHitRuntime::Apply_WorldToPlayer(shielded, hit, catalog, events);
		tests.Require(shielded.iCurrentHp == 100u && shielded.iShield == 3u && events.size() == 1u &&
			events.front().eHitFlag == DAMAGE_HIT_FLAG::ABSORB && events.front().iAmount == 7u,
			"Full shield hit sends one absorption event with no false HP damage");
		shielded.iShield = 10000u; shielded.iCurrentHp = 100u;
		shielded.isCombatReady = false;
		shielded.iInvulnerableEndTick = 1000u; shielded.eAction = PLAYER_ACTION_STATE::GRABBED;
		shielded.iAttachmentOwnerNetEntityId = 992u; shielded.hasMoveGoal = true;
		hit.bEncounterWipe = true; events.clear();
		const auto wipe = CServerCombatHitRuntime::Apply_WorldToPlayer(shielded, hit, catalog, events);
		tests.Require(wipe == SERVER_COMBAT_HIT_RESULT::KILLED && !shielded.iCurrentHp &&
			!shielded.iShield && !shielded.iInvulnerableEndTick && !shielded.iAttachmentOwnerNetEntityId &&
			shielded.eAction == PLAYER_ACTION_STATE::DEAD && !shielded.hasMoveGoal &&
			events.size() == 1u && events[0].iAmount == 100u && events[0].eHitFlag == DAMAGE_HIT_FLAG::NORMAL,
			"Encounter wipe bypasses shield, invulnerability and attachment while retaining death cleanup");
		SERVER_WORLD_ENTITY boss{};
		boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS; boss.iNetEntityId = 992u;
		boss.iCurrentHp = boss.iMaximumHp = 100u;
		boss.BossCombat.iShieldCurrent = boss.BossCombat.iShieldMaximum = 5u;
		SERVER_PLAYER_TO_WORLD_HIT outgoing{};
		outgoing.iRawDamage = 7u; outgoing.iSkillId = 34010u; outgoing.iSourcePlayerId = 1u;
		outgoing.iServerTick = 30u; outgoing.bHealthDamagePreResolved = true;
		events.clear(); (void)CServerCombatHitRuntime::Apply_PlayerToWorld(boss, outgoing, events);
		tests.Require(boss.iCurrentHp == 98u && events.size() == 2u &&
			events[0].eHitFlag == DAMAGE_HIT_FLAG::ABSORB && events[0].iAmount == 5u &&
			events[1].iAmount == 2u && events[1].isOutgoing,
			"Boss shield absorption uses the same typed event and preserves HP accounting");
	}



	{
		/* A skill with no damage profile never resolves a hit, so the hit time and
		reach that only describe that hit must be zero. Accepting a reach here would
		let a movement skill silently keep a damage window. */
		namespace fs = std::filesystem;
		const fs::path noDamageRoot =
			fs::temp_directory_path() / L"LostArkNoDamageContractTest";
		const auto loadWithMovementSkill =
			[&noDamageRoot](
				const char* hitTimeMs,
				const char* maximumRange,
				const TEST_TIMELINE_VARIANT timelineVariant)
		{
			std::error_code prepareError;
			fs::remove_all(noDamageRoot, prepareError);
			fs::create_directories(noDamageRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					noDamageRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap <<
					"LOSTARK_GAMEPLAY_BOOTSTRAP\t" << GAMEPLAY_BOOTSTRAP_VERSION <<
					'\t' << (17u + Count_TestTimelineRows(timelineVariant)) << '\n' <<
					"PATTERNPRESENTATIONGENERATION\tENCOUNTER_VALTAN\t1111111111111111111111111111111111111111111111111111111111111111\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
					"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
					"DAMAGE\tdamage.player.34120\t361\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
					"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
					"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
					"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
					"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
					"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34020\tLANCE_MASTER\tSPACE\tlancemaster.skill.34020"
					"\t8000\t900\t" << hitTimeMs << "\t242\t0\t6\t" << maximumRange <<
					"\t\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLCOMBATTRAITS\t34020\t0\t0\t0\n";
				Write_TestValtanTimelineRows(bootstrap, timelineVariant);
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", noDamageRoot.c_str());
			CGameplayCatalog catalog;
			const bool loaded = catalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		tests.Require(loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::VALID),
			"Accept a skill that carries no damage profile");
		tests.Require(!loadWithMovementSkill(
			"0", "3", TEST_TIMELINE_VARIANT::VALID),
			"Reject a damageless skill that still claims reach");
		tests.Require(!loadWithMovementSkill(
			"400", "0", TEST_TIMELINE_VARIANT::VALID),
			"Reject a damageless skill that still claims a hit time");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::MISSING_ROW),
			"Reject a timeline whose declared occurrence row is missing");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::MISSING_ACTION),
			"Reject a timeline occurrence whose pattern action is missing");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::OVERSIZED_ACTION_INDEX),
			"Reject an oversized timeline action index before staging storage");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::COMMAND_HASH_MISMATCH),
			"Reject a timeline command id that does not match its semantic row id");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::COMMAND_HASH_COLLISION),
			"Reject distinct timeline row ids whose stable command ids collide");
		std::error_code noDamageCleanupError;
		fs::remove_all(noDamageRoot, noDamageCleanupError);
	}

	{
		/* Wall-contact admission is separate from player damage admission: a
		   malformed or duplicated allowlist row must fail before a room can use
		   the hit as an axe collider. */
		namespace fs = std::filesystem;
		const fs::path wallContactRoot =
			fs::temp_directory_path() / L"LostArkWallContactCatalogContractTest";
		const auto loadWithWallContactRows = [
			&wallContactRoot](
				const std::uint32_t version,
				const std::vector<std::string>& sourceRows,
				const std::vector<std::string>& wallRows)
		{
			std::error_code prepareError;
			fs::remove_all(wallContactRoot, prepareError);
			fs::create_directories(wallContactRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					wallContactRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap << "LOSTARK_GAMEPLAY_BOOTSTRAP\t" << version << "\t" <<
					(16u + VALID_VALTAN_TIMELINE_ROW_COUNT +
					 sourceRows.size() + wallRows.size()) << "\n"
					"PATTERNPRESENTATIONGENERATION\tENCOUNTER_VALTAN\t1111111111111111111111111111111111111111111111111111111111111111\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
					"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
					"DAMAGE\tdamage.player.34120\t361\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
					"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
					"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
					"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
					"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34020\tLANCE_MASTER\tSPACE\tlancemaster.skill.34020\t8000\t900\t0\t242\t0\t6\t0\t\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLCOMBATTRAITS\t34020\t0\t0\t0\n";
				for (const std::string& row : sourceRows)
					bootstrap << row << '\n';
				for (const std::string& row : wallRows)
					bootstrap << row << '\n';
				Write_ValidValtanTimelineRows(bootstrap);
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", wallContactRoot.c_str());
			CGameplayCatalog wallCatalog;
			const bool loaded = wallCatalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		const std::string validWallRow =
			"PATTERNWALLCONTACT\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active";
		const std::string validSourceRow =
			"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180";
		tests.Require(
			loadWithWallContactRows(
				GAMEPLAY_BOOTSTRAP_VERSION, { validSourceRow }, { validWallRow }),
			"Accept one exact ACTIVE axe wall-contact row");
		tests.Require(
			!loadWithWallContactRows(
				GAMEPLAY_BOOTSTRAP_VERSION - 1u,
				{ validSourceRow }, { validWallRow }),
			"Reject the obsolete gameplay bootstrap version before wall-contact load");
		tests.Require(
			!loadWithWallContactRows(GAMEPLAY_BOOTSTRAP_VERSION,
				{ validSourceRow }, {
				"PATTERNWALLCONTACT\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.wrong" }),
			"Reject a wall-contact row whose action does not exactly join its stage");
		tests.Require(
			!loadWithWallContactRows(
				GAMEPLAY_BOOTSTRAP_VERSION,
				{ validSourceRow }, { validWallRow, validWallRow }),
			"Reject duplicate axe wall-contact ownership atomically");
		tests.Require(
			!loadWithWallContactRows(GAMEPLAY_BOOTSTRAP_VERSION, {}, {}),
			"Reject a boss pattern with no compiled source timing row");
		tests.Require(
			!loadWithWallContactRows(
				GAMEPLAY_BOOTSTRAP_VERSION,
				{ validSourceRow, validSourceRow }, {}),
			"Reject duplicate source timing ownership atomically");
		tests.Require(
			!loadWithWallContactRows(GAMEPLAY_BOOTSTRAP_VERSION, {
				"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t149\t350\t300\t180" }, {}),
			"Reject a source cooldown whose 30 Hz tick conversion is inconsistent");

		const auto loadWithPatternStageRows = [&wallContactRoot](
			const std::string& stageRows,
			const std::vector<std::string>& hitOffsetRows)
		{
			std::error_code prepareError;
			fs::remove_all(wallContactRoot, prepareError);
			fs::create_directories(wallContactRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					wallContactRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap << "LOSTARK_GAMEPLAY_BOOTSTRAP\t" <<
					GAMEPLAY_BOOTSTRAP_VERSION << "\t" <<
					(17u + VALID_VALTAN_TIMELINE_ROW_COUNT +
						hitOffsetRows.size()) << "\n"
					"PATTERNPRESENTATIONGENERATION\tENCOUNTER_VALTAN\t1111111111111111111111111111111111111111111111111111111111111111\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
					"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
					"DAMAGE\tdamage.player.34120\t361\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
					"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
					"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
					<< stageRows;
				for (const std::string& hitOffsetRow : hitOffsetRows)
					bootstrap << hitOffsetRow << '\n';
				bootstrap <<
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34020\tLANCE_MASTER\tSPACE\tlancemaster.skill.34020\t8000\t900\t0\t242\t0\t6\t0\t\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLCOMBATTRAITS\t34020\t0\t0\t0\n";
				Write_ValidValtanTimelineRows(bootstrap);
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", wallContactRoot.c_str());
			CGameplayCatalog stageCatalog;
			const bool loaded = stageCatalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		const std::string acceptedContactDelayRows =
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t600\tdamage.player.34120\t2\t242\t1\t2000\n"
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
			"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
			"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
			"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n";
		const bool acceptedContactDelay =
			loadWithPatternStageRows(acceptedContactDelayRows, {});
		tests.Require(
			acceptedContactDelay,
			"Accept a stage whose first hit lands at its authored contact delay");
		std::string invalidLegacySpawnCountRows = acceptedContactDelayRows;
		const std::string validLegacySpawnSuffix =
			"SPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n";
		const std::size_t legacySpawnSuffixAt =
			invalidLegacySpawnCountRows.find(validLegacySpawnSuffix);
		if (std::string::npos != legacySpawnSuffixAt)
		{
			invalidLegacySpawnCountRows.replace(
				legacySpawnSuffixAt, validLegacySpawnSuffix.size(),
				"SPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t2\t0\n");
		}
		tests.Require(
			std::string::npos != legacySpawnSuffixAt &&
			!loadWithPatternStageRows(invalidLegacySpawnCountRows, {}),
			"Reject legacy combat-object action counts above one");
		tests.Require(
			!loadWithPatternStageRows(
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t1000\tdamage.player.34120\t2\t242\t1\t2000\n"
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
				"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
				"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
				"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n",
				{}),
			"Reject a hit delay at or beyond its stage duration");
		tests.Require(
			!loadWithPatternStageRows(
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tWINDUP\tvaltan.test.active\tWINDUP\t1000\tNONE\t0\t0\t0\t0\t0\t0\t0\t600\t-\t0\t0\t0\t0\n"
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
				"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
				"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
				"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n",
				{}),
			"Reject a hit delay on a stage without a hit shape");

		const std::string explicitHitStageRows =
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t3\t0\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
			"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
			"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
			"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n";
		const std::vector<std::string> validExplicitHitOffsets{
			"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\t0\t100",
			"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\t1\t450",
			"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\t2\t900"
		};
		tests.Require(
			loadWithPatternStageRows(
				explicitHitStageRows, validExplicitHitOffsets),
			"Accept one complete ordered explicit boss hit schedule");
		tests.Require(
			!loadWithPatternStageRows(explicitHitStageRows, {
				validExplicitHitOffsets[0], validExplicitHitOffsets[1] }),
			"Reject an explicit boss hit schedule whose count is incomplete");
		tests.Require(
			!loadWithPatternStageRows(explicitHitStageRows, {
				validExplicitHitOffsets[0],
				"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\t1\t90",
				validExplicitHitOffsets[2] }),
			"Reject explicit boss hit offsets that are not strictly increasing");
		tests.Require(
			!loadWithPatternStageRows(explicitHitStageRows, {
				"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_MISSING\tvaltan.test.active\t0\t100",
				validExplicitHitOffsets[1], validExplicitHitOffsets[2] }),
			"Reject an explicit boss hit offset whose pattern owner is unknown");
		const std::string mixedScheduleStageRows =
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t3\t300\t0\tdamage.player.34120\t2\t242\t1\t2000\n" +
			explicitHitStageRows.substr(
				explicitHitStageRows.find("PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1"));
		tests.Require(
			!loadWithPatternStageRows(
				mixedScheduleStageRows, validExplicitHitOffsets),
			"Reject simultaneous legacy and explicit boss hit schedules");
		std::error_code cleanupError;
		fs::remove_all(wallContactRoot, cleanupError);
	}

	{
		/* A staged skill carries movement per stage, because a stage advance
		resets the action clock the curve is sampled on. */
		const PLAYER_SKILL_DEFINITION* basicAttack = catalog.Find_Skill(34010);
		bool everyStageCurveFits = nullptr != basicAttack &&
			!basicAttack->ComboStages.empty() &&
			basicAttack->RootMotion.empty();
		bool anyStageMoves = false;
		if (nullptr != basicAttack)
		{
			for (const PLAYER_COMBO_STAGE& stage : basicAttack->ComboStages)
			{
				if (stage.RootMotion.empty())
					continue;
				anyStageMoves = true;
				everyStageCurveFits = everyStageCurveFits &&
					stage.RootMotion.size() >= 2u &&
					stage.RootMotion.back().iTimeMs <= stage.iActionDurationMs;
			}
		}
		tests.Require(everyStageCurveFits && anyStageMoves,
			"Resolve per-stage root motion inside each combo stage duration");

		namespace fs = std::filesystem;
		const fs::path stageRoot =
			fs::temp_directory_path() / L"LostArkStageRootMotionContractTest";
		std::error_code stagePrepareError;
		const auto loadWithStageRow = [&](
			const char* stageIndex, const char* comboAdvanceMs)
		{
			fs::remove_all(stageRoot, stagePrepareError);
			fs::create_directories(stageRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					stageRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap <<
					"LOSTARK_GAMEPLAY_BOOTSTRAP\t" << GAMEPLAY_BOOTSTRAP_VERSION <<
					'\t' << (20u + VALID_VALTAN_TIMELINE_ROW_COUNT) << "\n"
					"PATTERNPRESENTATIONGENERATION\tENCOUNTER_VALTAN\t1111111111111111111111111111111111111111111111111111111111111111\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
					"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
					"DAMAGE\tdamage.player.34010\t100\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
					"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
					"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t0\tdamage.player.34010\t0\t0\t0\t0\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
					"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
					"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34010\t0\t0\t0\t0\n"
					"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34010\tLANCE_MASTER\tLMB\tlancemaster.skill.34010"
					"\t0\t1633\t470\t0\t0\t0\t3\tdamage.player.34010\tCOMBO"
					"\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLCOMBATTRAITS\t34010\t0\t0\t0\n"
					"SKILLSTAGE\t34010\t0\t1633\t470\t" << comboAdvanceMs <<
					"\t329\t658\n"
					"SKILLSTAGE\t34010\t1\t1367\t356\t1367\t0\t0\n"
					"SKILLSTAGEROOTMOTION\t34010\t" << stageIndex <<
					"\t2\t0:0:0,1600:1.5:0\n";
				Write_ValidValtanTimelineRows(bootstrap);
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", stageRoot.c_str());
			CGameplayCatalog stageCatalog;
			const bool loaded = stageCatalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		tests.Require(loadWithStageRow("0", "470"),
			"Accept a root motion row that names an existing combo stage");
		tests.Require(!loadWithStageRow("2", "470"),
			"Reject a root motion row past the last combo stage");
		tests.Require(!loadWithStageRow("0", "469"),
			"Reject a combo boundary before its damage time atomically");
		tests.Require(!loadWithStageRow("0", "1634"),
			"Reject a combo boundary after its stage duration atomically");
		std::error_code stageCleanupError;
		fs::remove_all(stageRoot, stageCleanupError);
	}

	{
		/* ?덈！??guards, and a hit taken inside that window is what buys the
		counter: no press advances it and the guard itself lands nothing. */
		SERVER_PLAYER counterPlayer{};
		counterPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		counterPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		counterPlayer.iCurrentHp = 1000;
		counterPlayer.iMaximumHp = 1000;
		counterPlayer.iCurrentResource = 1000;
		counterPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem counterSkills;

		C2S_USE_SKILL counterCommand{};
		counterCommand.iClientSequence = 1;
		counterCommand.iSkillId = 34580;
		counterCommand.fAimX = 1.f;
		counterCommand.fAimZ = 0.f;
		tests.Require(
			counterSkills.Try_Start(counterPlayer, counterCommand, catalog, 10) &&
			1u == counterPlayer.iComboStage,
			"Approve the counter guard stage");

		std::vector<SERVER_WORLD_ENTITY> counterEntities;
		std::vector<DAMAGE_EVENT> counterDamageEvents;
		counterSkills.Update(counterPlayer, counterEntities, catalog, nullptr,
			nullptr, 1.f / 30.f, 11, counterDamageEvents);
		tests.Require(
			1u == counterPlayer.iComboStage && counterDamageEvents.empty(),
			"Hold the guard stage and land no damage while it runs");

		const std::uint32_t hpBeforeCounter = counterPlayer.iCurrentHp;
		tests.Require(
			CPlayerSkillSystem::Try_Counter(counterPlayer, catalog, 12) &&
			2u == counterPlayer.iComboStage &&
			hpBeforeCounter == counterPlayer.iCurrentHp &&
			0.f == counterPlayer.fActionElapsedSeconds,
			"Absorb the hit inside the guard window and promote to the counter");
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(counterPlayer, catalog, 13),
			"Do not counter twice from one guard");

		SERVER_PLAYER lateCounter{};
		lateCounter.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		lateCounter.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		lateCounter.iCurrentHp = 1000;
		lateCounter.iMaximumHp = 1000;
		lateCounter.iCurrentResource = 1000;
		lateCounter.iMaximumResource = 1000;
		CPlayerSkillSystem lateSkills;
		C2S_USE_SKILL lateCommand = counterCommand;
		lateSkills.Try_Start(lateCounter, lateCommand, catalog, 10);
		lateCounter.fActionElapsedSeconds = 1.5f;
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(lateCounter, catalog, 20) &&
			1u == lateCounter.iComboStage,
			"Reject a hit that lands after the guard window closed");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		comboPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 1000;
		comboPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem comboSkills;
		C2S_USE_SKILL basicAttack{};
		basicAttack.iClientSequence = 1;
		basicAttack.iSkillId = 34010;
		basicAttack.fAimX = 1.f;
		basicAttack.fAimZ = 0.f;
		comboSkills.Try_Start(comboPlayer, basicAttack, catalog, 10);
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(comboPlayer, catalog, 11),
			"Never counter out of a skill that is not a COUNTER");
	}

	{
		SERVER_PLAYER stancePlayer{};
		stancePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		stancePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		stancePlayer.iCurrentHp = 1000;
		stancePlayer.iMaximumHp = 1000;
		stancePlayer.iCurrentResource = 1000;
		stancePlayer.iMaximumResource = 1000;
		CPlayerSkillSystem stanceSkills;

		C2S_USE_SKILL shortOnlySkill{};
		shortOnlySkill.iClientSequence = 1;
		shortOnlySkill.iSkillId = 34540;
		shortOnlySkill.fAimX = 1.f;
		shortOnlySkill.fAimZ = 0.f;
		tests.Require(!stanceSkills.Try_Start(stancePlayer, shortOnlySkill, catalog, 10),
			"Reject a short spear skill while in the long spear stance");

		C2S_USE_SKILL switchToShort{};
		switchToShort.iClientSequence = 2;
		switchToShort.iSkillId = 34000;
		switchToShort.fAimX = 1.f;
		switchToShort.fAimZ = 0.f;
		tests.Require(stanceSkills.Try_Start(stancePlayer, switchToShort, catalog, 10),
			"Approve the long to short spear stance transition");
		std::vector<SERVER_WORLD_ENTITY> stanceEntities;
		std::vector<DAMAGE_EVENT> stanceDamageEvents;
		for (std::uint32_t tick = 11; tick < 40; ++tick)
		{
			stanceSkills.Update(stancePlayer, stanceEntities, catalog, nullptr,
				nullptr, 1.f / 30.f, tick, stanceDamageEvents);
		}
		tests.Require(
			PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR == stancePlayer.eStance &&
			PLAYER_ACTION_STATE::NONE == stancePlayer.eAction,
			"Flip to the short spear stance once the transition action completes");

		C2S_USE_SKILL longOnlySkill{};
		longOnlySkill.iClientSequence = 3;
		longOnlySkill.iSkillId = 34120;
		longOnlySkill.fAimX = 1.f;
		longOnlySkill.fAimZ = 0.f;
		tests.Require(!stanceSkills.Try_Start(stancePlayer, longOnlySkill, catalog, 40),
			"Reject a long spear skill after switching to the short spear stance");

		C2S_USE_SKILL shortSkillNow = shortOnlySkill;
		shortSkillNow.iClientSequence = 4;
		tests.Require(stanceSkills.Try_Start(stancePlayer, shortSkillNow, catalog, 40),
			"Approve a short spear skill after switching to the short spear stance");
	}

	{
		SERVER_PLAYER holdPlayer{};
		holdPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		holdPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		holdPlayer.iCurrentHp = 1000;
		holdPlayer.iMaximumHp = 1000;
		holdPlayer.iCurrentResource = 1000;
		holdPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem holdSkills;

		C2S_USE_SKILL chargeStart{};
		chargeStart.iClientSequence = 1;
		chargeStart.iSkillId = 34590;
		chargeStart.fAimX = 1.f;
		chargeStart.fAimZ = 0.f;
		tests.Require(holdSkills.Try_Start(holdPlayer, chargeStart, catalog, 10),
			"Approve the short spear hold skill");

		C2S_UPDATE_SKILL_AIM turnedAim{};
		turnedAim.iClientSequence = 2;
		turnedAim.iSkillId = 34590;
		turnedAim.fAimX = 0.f;
		turnedAim.fAimZ = -1.f;
		const float chargeYaw = holdPlayer.fYawDegrees;
		holdSkills.Update_Aim(holdPlayer, turnedAim, catalog);
		tests.Require(
			holdPlayer.fYawDegrees != chargeYaw &&
			holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Turn a charging hold skill toward a new aim");

		C2S_UPDATE_SKILL_AIM wrongSkillAim = turnedAim;
		wrongSkillAim.iClientSequence = 3;
		wrongSkillAim.iSkillId = 34540;
		wrongSkillAim.fAimX = 1.f;
		wrongSkillAim.fAimZ = 1.f;
		holdSkills.Update_Aim(holdPlayer, wrongSkillAim, catalog);
		tests.Require(holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Ignore an aim update naming a skill that is not running");

		C2S_RELEASE_SKILL chargeRelease{};
		chargeRelease.iClientSequence = 2u;
		chargeRelease.iSkillId = 34590u;
		holdSkills.Release(holdPlayer, chargeRelease, catalog);
		tests.Require(
			holdPlayer.hasReleasedHold &&
			2u == holdPlayer.iLastSkillSequence,
			"Consume release for HOLD while COMBO release remains unsupported");
		C2S_UPDATE_SKILL_AIM releasedAim = turnedAim;
		releasedAim.iClientSequence = 4;
		releasedAim.fAimX = 1.f;
		releasedAim.fAimZ = 0.f;
		holdSkills.Update_Aim(holdPlayer, releasedAim, catalog);
		tests.Require(holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Keep the last aim once the hold key is released");

		holdPlayer.hasReleasedHold = false;
		holdPlayer.iComboStage = 3u;
		C2S_UPDATE_SKILL_AIM firingAim = turnedAim;
		firingAim.iClientSequence = 5;
		firingAim.fAimX = 1.f;
		firingAim.fAimZ = 0.f;
		holdSkills.Update_Aim(holdPlayer, firingAim, catalog);
		tests.Require(holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Keep the last aim through the firing stage");

		SERVER_PLAYER activePlayer{};
		activePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		activePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		activePlayer.iCurrentHp = 1000;
		activePlayer.iMaximumHp = 1000;
		activePlayer.iCurrentResource = 1000;
		activePlayer.iMaximumResource = 1000;
		CPlayerSkillSystem activeSkills;
		C2S_USE_SKILL activeStart{};
		activeStart.iClientSequence = 1;
		activeStart.iSkillId = 34540;
		activeStart.fAimX = 1.f;
		activeStart.fAimZ = 0.f;
		tests.Require(activeSkills.Try_Start(activePlayer, activeStart, catalog, 10),
			"Approve a non-hold short spear skill for the aim guard");
		const float activeAimX = activePlayer.fSkillAimDirectionX;
		C2S_UPDATE_SKILL_AIM activeAim{};
		activeAim.iClientSequence = 2;
		activeAim.iSkillId = 34540;
		activeAim.fAimX = 0.f;
		activeAim.fAimZ = -1.f;
		activeSkills.Update_Aim(activePlayer, activeAim, catalog);
		tests.Require(activeAimX == activePlayer.fSkillAimDirectionX,
			"Ignore an aim update on a skill that is not a HOLD");

		/* Guardian Knight human-form S (49220) and Alt+V (49420) carry caster
		shapes; an adjacent boss must take damage from both. */
		for (const std::uint32_t knightSkillId : { 49220u, 49420u })
		{
			SERVER_WORLD_ENTITY knightTarget{};
			knightTarget.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			knightTarget.strArchetypeId = "BOSS_VALTAN";
			knightTarget.iNetEntityId = 99810u;
			knightTarget.iCurrentHp = knightTarget.iMaximumHp = 100000000u;
			knightTarget.fPositionZ = 2.f;
			std::vector<SERVER_WORLD_ENTITY> knightTargets{ knightTarget };
			SERVER_PLAYER knight{};
			knight.iPlayerId = 99811u;
			knight.eCharacterClass = CHARACTER_CLASS_ID::GUARDIANKNIGHT;
			knight.eStance = PLAYER_STANCE_ID::GUARDIANKNIGHT_HUMAN;
			knight.iCurrentHp = knight.iMaximumHp = 10000u;
			knight.iCurrentResource = knight.iMaximumResource = 10000u;
			knight.iMaximumIdentity = 100u;
			CPlayerSkillSystem::Reset_Gauges(knight, catalog);
			CPlayerSkillSystem knightSkills;
			C2S_USE_SKILL knightCommand{};
			knightCommand.iClientSequence = 1u;
			knightCommand.iSkillId = knightSkillId;
			knightCommand.fAimX = 0.f;
			knightCommand.fAimZ = 3.f;
			std::vector<DAMAGE_EVENT> knightEvents;
			const bool knightStarted = knightSkills.Try_Start(knight, knightCommand, catalog, 10u);
			std::uint32_t knightNextTick = 11u;
			if (knightStarted && knightSkillId == 49420u)
			{
				const auto* breath = catalog.Find_Skill(knightSkillId);
				tests.Require(breath && breath->iHitTimeMs == 3800u && breath->Hits.size() == 3u &&
					std::all_of(breath->Hits.begin(), breath->Hits.end(), [](const auto& hit) { return hit.iTimeMs == 3800u; }),
					"Guardian Alt V damage, counter and stagger shapes share the breath/follow-camera boundary");
				knightSkills.Update(knight, knightTargets, catalog, nullptr, nullptr, 3.799f, 123u, knightEvents);
				tests.Require(knightTargets[0].iCurrentHp == knightTarget.iCurrentHp && knightEvents.empty(),
					"Guardian Alt V cannot damage the adjacent boss during its opening camera presentation");
				knightSkills.Update(knight, knightTargets, catalog, nullptr, nullptr, .002f, 124u, knightEvents);
				tests.Require(knightTargets[0].iCurrentHp < knightTarget.iCurrentHp && knightEvents.size() == 1u &&
					knightEvents.front().iAmount == knightTarget.iCurrentHp - knightTargets[0].iCurrentHp,
					"Guardian Alt V applies its first real HP damage and matching event when breath begins after camera return");
				knightNextTick = 125u;
			}
			for (std::uint32_t tick = knightNextTick; knightStarted && tick < 200u &&
				PLAYER_ACTION_STATE::SKILL == knight.eAction; ++tick)
				knightSkills.Update(knight, knightTargets, catalog, nullptr, nullptr, 1.f / 30.f, tick, knightEvents);
			tests.Require(knightStarted && knightTargets[0].iCurrentHp < knightTarget.iCurrentHp &&
				!knightEvents.empty() && (knightSkillId != 49420u || knightEvents.size() == 1u),
				("Guardian Knight human-form skill " + std::to_string(knightSkillId) +
					" lands its caster shape on an adjacent boss").c_str());
		}
	}
}

