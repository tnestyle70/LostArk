#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "BossCombatRuntime.h"
#include "ServerCombatHitRuntime.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "ServerApp.h"
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
#include <functional>
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

void LostArk::Server::CServerGameplayContractRunner::Run_ValtanRevision(TESTS& tests, CGameplayCatalog& catalog)
{

	{
		/* These three manual slots are Product catalog rows, not source-only
		   authoring claims. Drive each through the same Brain -> room transition
		   seam as a real fixed tick, then probe its authoritative consumer. */
		const auto prepareStatusRoom = [](const std::string& patternId,
			const NET_ENTITY_ID bossEntityId)
		{
			auto room = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
			room->m_Players.clear();
			room->m_PlayerIdByEntityId.clear();
			room->m_PlayerIdBySessionId.clear();
			room->m_WorldEntities.clear();

			SERVER_WORLD_ENTITY boss{};
			boss.iNetEntityId = bossEntityId;
			boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			boss.eAction = SERVER_ENTITY_ACTION::IDLE;
			boss.strArchetypeId = "BOSS_VALTAN";
			boss.strEncounterId = "ENCOUNTER_VALTAN";
			boss.strPlacementId = "boss.valtan.status-slot-contract";
			boss.iCurrentHp = boss.iMaximumHp = 80000u;
			boss.iAttackPower = 100u;
			boss.iMaximumHealthBars = boss.iLastEvaluatedHealthBar = 160u;
			boss.iPhase = 2u;
			boss.fPositionX = boss.fSpawnPositionX = 156.03f;
			boss.fPositionY = boss.fSpawnPositionY = 22.99751f;
			boss.fPositionZ = boss.fSpawnPositionZ = -122.06f;
			boss.fEngageDistance = 100.f;
			boss.bIntroPatternConsumed = true;
			boss.bScriptedPatternPlayback = true;
			boss.bAutomaticPatternSequenceAuditionOverride = true;
			boss.PendingPatternIds.push_back(patternId);
			boss.PinnedDefinitionRevision =
				room->m_GameplayCatalog.Get_ActiveRevision();
			room->m_WorldEntities.push_back(boss);

			for (std::uint32_t ordinal = 0u; ordinal < 3u; ++ordinal)
			{
				SERVER_PLAYER player{};
				player.iPlayerId = 19600u + ordinal;
				player.iNetEntityId = bossEntityId + 100u + ordinal;
				player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
				if (const PLAYER_RUNTIME_PROFILE* profile =
					room->m_GameplayCatalog.Find_Player(player.eCharacterClass))
				{
					player.eStance = profile->eDefaultStance;
				}
				player.iCurrentHp = player.iMaximumHp = 100000u;
				player.iCurrentResource = player.iMaximumResource = 10000u;
				player.isCombatReady = true;
				player.fPositionX = boss.fPositionX +
					(0u == ordinal ? -0.25f : 0.25f);
				player.fPositionZ = boss.fPositionZ + 0.5f;
				SERVER_NAV_POINT ground{};
				if (room->m_ServerNavigation.Sample_Position(
						player.fPositionX, player.fPositionZ, ground))
				{
					player.fPositionX = ground.x;
					player.fPositionY = ground.y;
					player.fPositionZ = ground.z;
				}
				else
				{
					player.fPositionY = boss.fPositionY;
				}
				if (2u == ordinal)
				{
					player.iCurrentHp = 0u;
					player.eAction = PLAYER_ACTION_STATE::DEAD;
					player.isCombatReady = false;
				}
				room->m_PlayerIdByEntityId.emplace(
					player.iNetEntityId, player.iPlayerId);
				room->m_PlayerIdBySessionId.emplace(
					20600u + ordinal, player.iPlayerId);
				room->m_Players.emplace(player.iPlayerId, player);
			}
			return room;
		};
		const auto advanceStatusOccurrence = [](CGameRoom& room,
			const std::uint32_t tick,
			const std::function<void(const CGameRoom&)>& afterStageTransaction = {})
		{
			if (!room.Is_Ready() || room.m_WorldEntities.empty())
				return false;
			SERVER_WORLD_ENTITY& boss = room.m_WorldEntities.front();
			const std::string previousPatternId = boss.strPatternId;
			const std::string previousActionId = boss.strActionId;
			const GameplayDataRevision previousRevision =
				boss.PinnedDefinitionRevision;
			const std::uint32_t previousSequence = boss.iPatternSequence;
			room.m_TickDamageEvents.clear();
			std::vector<SERVER_PLAYER_CAPTURE_REQUEST> captures;
			room.m_ValtanBrain.Update(
				boss, room.m_Players, room.m_GameplayCatalog,
				room.m_ServerNavigation, 1.f / 30.f, tick, {},
				room.m_TickDamageEvents, nullptr, 1u, &captures);
			if (!captures.empty())
				return false;
			if (previousPatternId != boss.strPatternId ||
				previousActionId != boss.strActionId ||
				previousSequence != boss.iPatternSequence)
			{
				if (!room.Apply_BossPatternStageTransition(
						boss, previousPatternId, previousActionId,
						boss.strPatternId, boss.strActionId,
						previousRevision, boss.PinnedDefinitionRevision, tick))
				{
					return false;
				}
			}
			if (afterStageTransaction)
				afterStageTransaction(room);
			room.m_iServerTick = 0u == tick ? 0u : tick - 1u;
			room.Update_Players(1.f / 30.f);
			room.m_iServerTick = tick;
			return room.Is_Ready();
		};

		bool dashBranchContract = true;
		for (const bool wallContact : {false, true})
		{
			auto room = prepareStatusRoom("VALTAN_DASH_CHARGE", 19670u);
			dashBranchContract = dashBranchContract && advanceStatusOccurrence(*room, 7000u) &&
				advanceStatusOccurrence(*room, 7221u);
			auto& boss = room->m_WorldEntities.front();
			dashBranchContract = dashBranchContract && "CHARGE" == boss.strPatternStageId;
			if (wallContact)
				dashBranchContract = dashBranchContract && CBossCombatRuntime::Publish_PatternOutcome(
					boss, BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT, 7266u);
			dashBranchContract = dashBranchContract && advanceStatusOccurrence(*room, 7266u);
			dashBranchContract = dashBranchContract && (wallContact ?
				("GROGGY" == boss.strPatternStageId && boss.bPatternGroggy) :
				(boss.strPatternId.empty() && !boss.bPatternGroggy &&
				 SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == boss.PatternTerminalReceipt.eResult));
		}
		tests.Require(dashBranchContract,
			"Dash deadline completes normally while a wall contact on the same deadline wins and enters GROGGY");

		auto fixedFourRoom = prepareStatusRoom("VALTAN_SEQUENCE_FOUR", 19680u);
		fixedFourRoom->m_WorldEntities.front().fYawDegrees = 37.f;
		bool fourDirectionFixed = true;
		for (std::uint32_t tick = 800u; tick < 830u && fourDirectionFixed; ++tick)
		{
			fixedFourRoom->m_Players.at(19600u).fPositionX += 0.1f;
			fixedFourRoom->m_Players.at(19600u).fPositionZ -= 0.1f;
			fourDirectionFixed = advanceStatusOccurrence(*fixedFourRoom, tick) &&
				std::abs(fixedFourRoom->m_WorldEntities.front().fYawDegrees - 37.f) < 0.001f;
		}
		tests.Require(fourDirectionFixed,
			"Four-direction slash preserves its starting authored direction as players move");

		auto strugglingFourRoom = prepareStatusRoom("VALTAN_STRUGGLING", 19685u);
		bool strugglingFourFixed = advanceStatusOccurrence(*strugglingFourRoom, 9000u) &&
			advanceStatusOccurrence(*strugglingFourRoom, 9060u) &&
			advanceStatusOccurrence(*strugglingFourRoom, 9075u);
		auto& strugglingFourBoss = strugglingFourRoom->m_WorldEntities.front();
		// Deliberately disagree with both the centre and target bearings at the edge.
		strugglingFourBoss.fPositionX += 1.f;
		strugglingFourBoss.fYawDegrees = 37.f;
		strugglingFourFixed = strugglingFourFixed && advanceStatusOccurrence(*strugglingFourRoom, 9126u) &&
			"STEP_04" == strugglingFourBoss.strPatternStageId;
		for (std::uint32_t tick = 9127u; tick < 9275u && strugglingFourFixed; ++tick)
		{
			strugglingFourRoom->m_Players.at(19600u).fPositionX += 0.03f;
			strugglingFourFixed = advanceStatusOccurrence(*strugglingFourRoom, tick) &&
				std::abs(strugglingFourBoss.fYawDegrees - 37.f) < 0.001f;
		}
		tests.Require(strugglingFourFixed && 4u == strugglingFourBoss.iAppliedPatternHitCount,
			"Struggling freezes only its four-direction stage through all four authored hit angles despite centre and player bearings");

		auto magicRoom = prepareStatusRoom("VALTAN_STAGGER_SLOT", 19700u);
		bool magicOccurrenceValid = advanceStatusOccurrence(*magicRoom, 1000u);
		SERVER_WORLD_ENTITY& magicBoss = magicRoom->m_WorldEntities.front();
		const float magicBaseY = magicBoss.fSpawnPositionY;
		const auto maximum = magicRoom->m_GameplayCatalog.Active().Get_RaidStaggerMaximum();
		if (maximum <= 4u || static_cast<std::uint64_t>(maximum) * 1000u >
			(std::numeric_limits<std::uint32_t>::max)())
		{
			tests.Require(false, "Common stagger maximum admits the explicit raw/1000 fixture inputs");
			return;
		}
		magicOccurrenceValid = magicOccurrenceValid &&
			"VALTAN_STAGGER_SLOT" == magicBoss.strPatternId &&
			"CHANNEL" == magicBoss.strPatternStageId &&
			"valtan.authoring.stagger-slot.channel" == magicBoss.strActionId &&
			12000u == magicBoss.iPatternStageDurationMs &&
			!magicBoss.bPatternVerticalOffsetApplied &&
			magicBoss.bPatternStageVerticalOffsetApplied &&
			std::abs(magicBoss.fPatternStageVerticalBaseY - magicBaseY) < 0.001f &&
			std::abs(magicBoss.fPositionY - magicBaseY - 0.5f) < 0.001f &&
			BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE == magicBoss.ePatternBossResponseKind &&
			0u == magicBoss.iPatternBossResponseThreshold &&
			maximum == magicBoss.BossCombat.iStaggerMaximum &&
			0u == magicBoss.BossCombat.iStaggerCurrent;
		const auto hpBefore = magicBoss.iCurrentHp;
		SERVER_PLAYER_TO_WORLD_HIT hit{};
		hit.iSourcePlayerId = 19600u; hit.iSkillId = 34040u;
		hit.iServerTick = 1001u; hit.iRawDamage = 999u; hit.bStaggerDisabled = true;
		(void)CServerCombatHitRuntime::Apply_PlayerToWorld(magicBoss, hit, magicRoom->m_TickDamageEvents);
		magicOccurrenceValid = magicOccurrenceValid && magicBoss.iCurrentHp == hpBefore - 999u &&
			magicBoss.BossCombat.iStaggerCurrent == 0u;
		// The independent skill stagger channel carries its raw damage basis while HP stays unchanged.
		hit.bStaggerDisabled = false; hit.bHealthDamageDisabled = true;
		hit.iRawDamage = static_cast<std::uint32_t>(static_cast<std::uint64_t>(maximum - 4u) * 1000u);
		hit.iServerTick = 1002u;
		(void)CServerCombatHitRuntime::Apply_PlayerToWorld(magicBoss, hit, magicRoom->m_TickDamageEvents);
		magicOccurrenceValid = magicOccurrenceValid && magicBoss.iCurrentHp == hpBefore - 999u &&
			magicBoss.BossCombat.iStaggerCurrent == maximum - 4u;
		// Shield absorption and HP reduction do not change the pre-HP stagger basis.
		(void)CBossCombatRuntime::Set_Shield(magicBoss.BossCombat, 1000u);
		hit.bHealthDamageDisabled = false; hit.iRawDamage = 1000u; hit.iServerTick = 1003u;
		(void)CServerCombatHitRuntime::Apply_PlayerToWorld(magicBoss, hit, magicRoom->m_TickDamageEvents);
		magicOccurrenceValid = magicOccurrenceValid && magicBoss.iCurrentHp == hpBefore - 999u &&
			0u == magicBoss.BossCombat.iShieldCurrent && magicBoss.BossCombat.iStaggerCurrent == maximum - 3u;
		magicBoss.iKoukuDamageReductionWindows = 1u;
		hit.iServerTick = 1004u;
		(void)CServerCombatHitRuntime::Apply_PlayerToWorld(magicBoss, hit, magicRoom->m_TickDamageEvents);
		magicBoss.iKoukuDamageReductionWindows = 0u;
		magicOccurrenceValid = magicOccurrenceValid && magicBoss.iCurrentHp == hpBefore - 1000u &&
			magicBoss.BossCombat.iStaggerCurrent == maximum - 2u;
		// Typed invulnerability blocks HP only; the established independent stagger policy remains active.
		(void)CBossCombatRuntime::Set_Flag(magicBoss.BossCombat, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE, true);
		hit.iServerTick = 1005u;
		(void)CServerCombatHitRuntime::Apply_PlayerToWorld(magicBoss, hit, magicRoom->m_TickDamageEvents);
		(void)CBossCombatRuntime::Set_Flag(magicBoss.BossCombat, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE, false);
		magicOccurrenceValid = magicOccurrenceValid && magicBoss.iCurrentHp == hpBefore - 1000u &&
			magicBoss.BossCombat.iStaggerCurrent == maximum - 1u && magicBoss.BossCombat.PendingOutcomes.empty();
		// Pattern invulnerability is the existing admission boundary that blocks both channels.
		magicBoss.bPatternInvulnerable = true;
		hit.iServerTick = 1006u;
		(void)CServerCombatHitRuntime::Apply_PlayerToWorld(magicBoss, hit, magicRoom->m_TickDamageEvents);
		magicBoss.bPatternInvulnerable = false;
		magicOccurrenceValid = magicOccurrenceValid && magicBoss.iCurrentHp == hpBefore - 1000u &&
			magicBoss.BossCombat.iStaggerCurrent == maximum - 1u && magicBoss.BossCombat.PendingOutcomes.empty();
		hit.bHealthDamageDisabled = true;
		for (const auto tick : {1007u, 1008u})
		{
			hit.iServerTick = tick;
			(void)CServerCombatHitRuntime::Apply_PlayerToWorld(magicBoss, hit, magicRoom->m_TickDamageEvents);
		}
		magicOccurrenceValid = magicOccurrenceValid && magicBoss.iCurrentHp == hpBefore - 1000u &&
			maximum == magicBoss.BossCombat.iStaggerCurrent &&
			1u == magicBoss.BossCombat.PendingOutcomes.size() &&
			BOSS_PATTERN_STAGE_OUTCOME::STAGGER_BROKEN == magicBoss.BossCombat.PendingOutcomes.front().eOutcome &&
			advanceStatusOccurrence(*magicRoom, 1009u) &&
			magicBoss.PendingPatternFollowup.Is_Pending() &&
			"VALTAN_GROGGY_FOLLOWUP" == magicBoss.PendingPatternFollowup.strPatternId && magicBoss.strPatternId.empty() &&
			!magicBoss.bPatternVerticalOffsetApplied &&
			!magicBoss.bPatternStageVerticalOffsetApplied &&
			std::abs(magicBoss.fPositionY - magicBaseY) < 0.001f &&
			0u == magicBoss.BossCombat.iStaggerMaximum && 0u == magicBoss.BossCombat.iStaggerCurrent &&
			BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE == magicBoss.ePatternBossResponseKind &&
			0u == magicBoss.iPatternBossResponseThreshold &&
			0u == magicBoss.iPatternBossResponseAccumulatedHealthDamage && !magicBoss.bPatternBossResponsePublished &&
			advanceStatusOccurrence(*magicRoom, 1010u);
		tests.Require(magicOccurrenceValid && "VALTAN_GROGGY_FOLLOWUP" == magicBoss.strPatternId &&
			"GROGGY" == magicBoss.strPatternStageId && "valtan.followup.groggy.active" == magicBoss.strActionId &&
			std::abs(magicBoss.fPositionY - magicBaseY) < 0.001f &&
			CBossCombatRuntime::Has_Flag(magicBoss.BossCombat, SERVER_BOSS_COMBAT_FLAG::GROGGY),
			"Magic-orb uses common raw/1000 stagger independently of HP, shield and reduction, publishes once, clears on EXIT and restores Y before Groggy");
		auto whirlwindRoom = prepareStatusRoom("VALTAN_STAGGER_SLOT", 19725u);
		bool whirlwindValid = advanceStatusOccurrence(*whirlwindRoom, 1500u);
		auto& whirlwindBoss = whirlwindRoom->m_WorldEntities.front();
		const auto whirlwindHp = whirlwindBoss.iCurrentHp;
		hit.iStaggerMaximumDivisor = 3u; hit.iRawDamage = 1u; hit.bHealthDamageDisabled = true;
		const auto third = maximum / 3u + (maximum % 3u ? 1u : 0u);
		for (std::uint32_t index = 1u; index <= 3u; ++index)
		{
			hit.iServerTick = 1500u + index;
			(void)CServerCombatHitRuntime::Apply_PlayerToWorld(whirlwindBoss, hit, whirlwindRoom->m_TickDamageEvents);
			whirlwindValid = whirlwindValid && whirlwindBoss.iCurrentHp == whirlwindHp &&
				whirlwindBoss.BossCombat.iStaggerCurrent == (std::min)(maximum, third * index) &&
				whirlwindBoss.BossCombat.PendingOutcomes.size() == (index == 3u ? 1u : 0u);
		}
		tests.Require(whirlwindValid,
			"Magic-orb consumes the existing whirlwind one-third credit and saturates the common gauge on the third confirmed hit");

		auto magicFailureRoom = prepareStatusRoom(
			"VALTAN_STAGGER_SLOT", 19750u);
		bool magicFailureValid =
			advanceStatusOccurrence(*magicFailureRoom, 2000u);
		SERVER_WORLD_ENTITY& magicFailureBoss =
			magicFailureRoom->m_WorldEntities.front();
		const float failureBaseY = magicFailureBoss.fSpawnPositionY;
		magicFailureValid = magicFailureValid &&
			advanceStatusOccurrence(*magicFailureRoom, 2360u) &&
			"FINAL_ATTACK" == magicFailureBoss.strPatternStageId &&
			0u == magicFailureBoss.iAppliedPatternHitCount &&
			!magicFailureBoss.bPatternStageVerticalOffsetApplied &&
			std::abs(magicFailureBoss.fPositionY - failureBaseY) <
				0.001f;
		for (auto& [playerId, player] : magicFailureRoom->m_Players)
		{
			(void)playerId;
			if (0u != player.iCurrentHp)
			{
				player.iCurrentHp = player.iMaximumHp = 10000u;
				player.iShield = 1000000u;
				player.iInvulnerableEndTick = 3000u;
				player.iEstherGuardEndTick = 3000u;
				player.iEstherGuardDamageTakenPercent = -100;
				player.bRonaunGuard = true;
				if (19601u == playerId)
				{
					player.isCombatReady = false;
					player.fPositionX += 200.f;
				}
			}
		}
		magicFailureValid = magicFailureValid &&
			advanceStatusOccurrence(*magicFailureRoom, 2389u) &&
			magicFailureRoom->m_TickDamageEvents.empty() &&
			2u == std::count_if(magicFailureRoom->m_Players.begin(),
				magicFailureRoom->m_Players.end(), [](const auto& entry)
				{ return 10000u == entry.second.iCurrentHp; }) &&
			0u == magicFailureBoss.iAppliedPatternHitCount &&
			advanceStatusOccurrence(*magicFailureRoom, 2390u) &&
			2u == magicFailureRoom->m_TickDamageEvents.size() &&
			std::all_of(magicFailureRoom->m_Players.begin(),
				magicFailureRoom->m_Players.end(), [](const auto& entry)
				{ return 0u == entry.second.iCurrentHp; }) &&
			1u == magicFailureBoss.iAppliedPatternHitCount &&
			advanceStatusOccurrence(*magicFailureRoom, 2450u) &&
			!magicFailureBoss.bPatternVerticalOffsetApplied &&
			!magicFailureBoss.bPatternStageVerticalOffsetApplied &&
			std::abs(magicFailureBoss.fPositionY - failureBaseY) < 0.001f &&
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
				magicFailureBoss.PatternTerminalReceipt.eResult;
		tests.Require(
			magicFailureValid,
			"Magic-orb timeout restores the Stage-owned +0.5m offset before its 1000ms final-attack wipe and stays at base Y");

		auto counterFailureRoom = prepareStatusRoom("VALTAN_TRIPLE_COUNTER", 19760u);
		for (auto& [id, player] : counterFailureRoom->m_Players)
		{
			if (!player.iCurrentHp) continue;
			player.iShield = 1000000u;
			player.iInvulnerableEndTick = player.iEstherGuardEndTick = 6000u;
			player.iEstherGuardDamageTakenPercent = -100;
			player.bRonaunGuard = true;
			if (19601u == id) { player.isCombatReady = false; player.fPositionX += 200.f; }
		}
		SERVER_PLAYER guide = counterFailureRoom->m_Players.at(19600u);
		guide.iPlayerId = 19603u; guide.iNetEntityId = 19863u;
		guide.eControlKind = PLAYER_CONTROL_KIND::GUIDE_AI;
		counterFailureRoom->m_Players.emplace(guide.iPlayerId, guide);
		bool counterFailureValid = true, sawFirstMiss = false, sawSecondMiss = false, sawFinalWipe = false;
		for (std::uint32_t tick = 5000u; tick < 5500u && counterFailureValid; ++tick)
		{
			counterFailureValid = advanceStatusOccurrence(*counterFailureRoom, tick);
			const auto& boss = counterFailureRoom->m_WorldEntities.front();
			const bool humansAlive = counterFailureRoom->m_Players.at(19600u).iCurrentHp &&
				counterFailureRoom->m_Players.at(19601u).iCurrentHp;
			if (boss.iAppliedPatternHitCount && "FAIL_1" == boss.strPatternStageId)
				sawFirstMiss = humansAlive;
			if (boss.iAppliedPatternHitCount && "FAIL_2" == boss.strPatternStageId)
				sawSecondMiss = humansAlive;
			if (boss.iAppliedPatternHitCount && "FAIL_3" == boss.strPatternStageId)
			{
				sawFinalWipe = !counterFailureRoom->m_Players.at(19600u).iCurrentHp &&
					!counterFailureRoom->m_Players.at(19601u).iCurrentHp &&
					guide.iCurrentHp == counterFailureRoom->m_Players.at(guide.iPlayerId).iCurrentHp;
				break;
			}
		}
		tests.Require(counterFailureValid && sawFirstMiss && sawSecondMiss && sawFinalWipe,
			"Only the third missed counter wipes every living human through shields, invulnerability and range while preserving Guide");

		auto magicAbortRoom = prepareStatusRoom(
			"VALTAN_STAGGER_SLOT", 19775u);
		bool magicAbortValid = advanceStatusOccurrence(*magicAbortRoom, 3000u);
		SERVER_WORLD_ENTITY& magicAbortBoss =
			magicAbortRoom->m_WorldEntities.front();
		const float abortBaseY = magicAbortBoss.fSpawnPositionY;
		CValtanBrain::Fail_Mechanic(
			magicAbortBoss, "VALTAN_STAGGER_SLOT",
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT, 3001u);
		magicAbortValid = magicAbortValid &&
			!magicAbortBoss.bPatternVerticalOffsetApplied &&
			!magicAbortBoss.bPatternStageVerticalOffsetApplied &&
			std::abs(magicAbortBoss.fPositionY - abortBaseY) < 0.001f &&
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED ==
				magicAbortBoss.PatternTerminalReceipt.eResult;
		tests.Require(
			magicAbortValid,
			"Magic-orb stage-transition abort restores its captured base Y without drift");

		const auto counterProxyAccepts = [](
			const BOSS_PATTERN_COUNTER_PROXY_KIND kind,
			const float sourceX, const float sourceZ,
			const std::uint32_t counterPower = 1u)
		{
			SERVER_WORLD_ENTITY boss{};
			boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			boss.iCurrentHp = boss.iMaximumHp = 1000u;
			boss.fPositionX = 10.f;
			boss.fPositionZ = 20.f;
			boss.fYawDegrees = 0.f;
			boss.iPatternSequence = 1u;
			boss.strPatternId = "counter.proxy.contract";
			boss.strActionId = "counter.proxy.contract.active";
			boss.bPatternHasCounterProxy = true;
			boss.ePatternCounterProxyKind = kind;
			if (BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC == kind)
			{
				boss.fPatternCounterProxyArcDegrees = 180.f;
			}
			else
			{
				boss.fPatternCounterProxyForwardOffsetM = 1.f;
				boss.fPatternCounterProxyRadiusM = 2.f;
			}
			(void)CBossCombatRuntime::Set_Flag(
				boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, true);
			BOSS_INCOMING_HIT hit{};
			hit.iCounterPower = counterPower;
			hit.iServerTick = 1u;
			hit.fSourceX = sourceX;
			hit.fSourceZ = sourceZ;
			return CBossCombatRuntime::Apply_PlayerHit(
				boss, hit).bCounterTriggered;
		};
		const bool forwardHalfPlaneExact =
			counterProxyAccepts(
				BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC, 10.f, 21.f) &&
			counterProxyAccepts(
				BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC, 11.f, 20.f) &&
			counterProxyAccepts(
				BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC, 9.f, 20.f) &&
			!counterProxyAccepts(
				BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC, 10.f, 19.f) &&
			!counterProxyAccepts(
				BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC, 11.f, 19.001f) &&
			!counterProxyAccepts(
				BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC,
				(std::numeric_limits<float>::quiet_NaN)(), 21.f) &&
			!counterProxyAccepts(
				BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC, 10.f, 21.f, 0u) &&
			counterProxyAccepts(
				BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_LOCAL_CIRCLE, 10.f, 21.f) &&
			!counterProxyAccepts(
				BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_LOCAL_CIRCLE, 10.f, 23.01f);
		tests.Require(
			forwardHalfPlaneExact,
			"Counter source admission uses a radiusless closed boss-forward 180-degree half-plane while retaining the legacy local-circle proxy");

		const auto* statusPatterns = catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		if (nullptr == statusPatterns)
		{
			tests.Require(false, "Catalog has Valtan definitions for authored Bind clocks");
			return;
		}
		const auto bindDefinition = std::find_if(statusPatterns->begin(), statusPatterns->end(),
			[](const BOSS_PATTERN_DEFINITION& definition) { return definition.strPatternId == "VALTAN_BIND_SLOT"; });
		if (statusPatterns->end() == bindDefinition || bindDefinition->Stages.size() < 2u)
		{
			tests.Require(false, "Catalog has a Bind hold and recovery definition");
			return;
		}
		const auto bindDurationMs = bindDefinition->Stages.front().iDurationMs;
		const auto bindTicks = (bindDurationMs * 30u + 999u) / 1000u;
		const auto recoveryDurationMs = bindDefinition->Stages[1u].iDurationMs;
		const auto recoveryTicks = (recoveryDurationMs * 30u + 999u) / 1000u;
		const auto bindRecoveryTick = 2000u + bindTicks - 1u;
		std::ostringstream bindDiagnostics;
		const auto recordBindState = [&bindDiagnostics](const char* label, const CGameRoom& room,
			const PLAYER_ID playerId, const bool valid)
		{
			const auto& boss = room.m_WorldEntities.front();
			bindDiagnostics << "[BindDiagnostic] " << label << " valid=" << valid
				<< " tick=" << room.m_iServerTick << " roomReady=" << room.Is_Ready()
				<< " status=" << room.m_strStatus << " stage=" << boss.strPatternStageId
				<< " duration=" << boss.iPatternStageDurationMs << " firstEvaluation=" << boss.iPatternStageFirstEvaluationTick
				<< " sequence=" << boss.iPatternSequence << " target=" << boss.iPatternTargetEntityId
				<< " terminal=" << static_cast<unsigned>(boss.PatternTerminalReceipt.eResult)
				<< " player=" << playerId;
			const auto found = room.m_Players.find(playerId);
			if (found != room.m_Players.end())
			{
				const auto& p = found->second;
				bindDiagnostics << " hp=" << p.iCurrentHp << " bound=" << p.bPatternBound
					<< " ready=" << p.isCombatReady << " restoreReady=" << p.bPatternBindRestoreCombatReady
					<< " owner=" << p.iPatternBindOwnerNetEntityId << " end=" << p.iPatternBindEndTick
					<< " entity=" << p.iNetEntityId << " action=" << static_cast<unsigned>(p.eAction)
					<< " skill=" << p.iCurrentSkillId << " skillSequence=" << p.iLastSkillSequence
					<< " resource=" << p.iCurrentResource << " moveGoal=" << p.hasMoveGoal
					<< " pose=" << p.fPositionX << ',' << p.fPositionY << ',' << p.fPositionZ
					<< " restore=" << p.fPatternBindRestoreX << ',' << p.fPatternBindRestoreY << ',' << p.fPatternBindRestoreZ
					<< " knockback=" << p.fKnockbackRemainingSeconds;
			}
			bindDiagnostics << '\n';
		};
		// Exercise real reaction integration with the published six contact clocks.
		// This fixture assumes every contact lands; the live failure log has no target pose.
		auto airborneBindRoom = prepareStatusRoom("VALTAN_BIND_SLOT", 19880u);
		auto& airborne = airborneBindRoom->m_Players.at(19600u);
		airborneBindRoom->m_Players.at(19601u).iCurrentHp = 0u;
		airborneBindRoom->m_Players.at(19601u).isCombatReady = false;
		const auto fourSlash = std::find_if(statusPatterns->begin(), statusPatterns->end(),
			[](const BOSS_PATTERN_DEFINITION& definition) { return definition.strPatternId == "VALTAN_FOUR_SLASH"; });
		std::vector<std::pair<std::uint32_t, const ATTACK_HIT_TEMPLATE*>> launchContacts;
		std::uint32_t fourSlashDurationMs = 0u;
		if (fourSlash != statusPatterns->end())
		{
			for (const auto& stage : fourSlash->Stages)
			{
				for (const auto& contact : stage.AttackContacts)
					launchContacts.emplace_back(fourSlashDurationMs + contact.iAtMs, &contact);
				fourSlashDurationMs += stage.iDurationMs;
			}
		}
		std::sort(launchContacts.begin(), launchContacts.end(),
			[](const auto& left, const auto& right) { return left.first < right.first; });
		std::size_t launchedCount = 0u;
		const auto fourSlashTicks = (fourSlashDurationMs * 30u + 999u) / 1000u;
		for (std::uint32_t offsetTick = 0u; offsetTick < fourSlashTicks; ++offsetTick)
		{
			const auto elapsedMs = offsetTick * 1000u / 30u;
			while (launchedCount < launchContacts.size() && launchContacts[launchedCount].first <= elapsedMs)
			{
				const auto& contact = *launchContacts[launchedCount++].second;
				CPlayerSkillSystem::Arm_PlayerHitReaction(airborne,
					airborne.fPositionX - 1.f, airborne.fPositionZ,
					static_cast<float>(contact.fPushRangeM), contact.iPushMs, false, 0u,
					6000u + offsetTick, contact.ForcePush.value_or(contact.fRiseHeightM > 0.0),
					false, contact.fRiseHeightM > 0.0, static_cast<float>(contact.fRiseHeightM));
			}
			airborneBindRoom->m_iServerTick = 6000u + offsetTick;
			airborneBindRoom->Advance_PlayerKnockback(airborne, 1.f / 30.f);
		}
		SERVER_NAV_POINT airborneGround{};
		const bool hasAirborneGround = airborneBindRoom->m_ServerNavigation.Sample_Position(
			airborne.fPositionX, airborne.fPositionZ, airborneGround, airborne.fKnockbackSupportY);
		const bool oldPoseGuardRejects = hasAirborneGround && launchedCount == 6u &&
			airborne.bKnockbackBallistic && airborne.fKnockbackRemainingSeconds > 0.f &&
			std::abs(airborne.fPositionY - airborneGround.y) > 1.5f;
		std::cout << "[BindAirborneDiagnostic] launched=" << launchedCount
			<< " elapsedMs=" << fourSlashDurationMs << " oldPoseGuardRejects=" << oldPoseGuardRejects
			<< " position=" << airborne.fPositionX << ',' << airborne.fPositionY << ',' << airborne.fPositionZ
			<< " groundY=" << airborneGround.y << " supportY=" << airborne.fKnockbackSupportY
			<< " remaining=" << airborne.fKnockbackRemainingSeconds << '\n';
		tests.Require(oldPoseGuardRejects,
			"Published FOUR_SLASH contacts through Arm and Advance produce a navigable airborne target rejected by the former Bind current-Y guard");
		const SERVER_PLAYER airborneBeforeBind = airborne;
		bool airborneBindValid = oldPoseGuardRejects && advanceStatusOccurrence(*airborneBindRoom, 7000u);
		airborneBindValid = airborneBindValid && airborne.bPatternBound &&
			std::abs(airborne.fPatternBindRestoreY - airborneGround.y) < 0.001f &&
			std::abs(airborne.fPositionY - airborneGround.y - 5.f) < 0.001f &&
			!airborne.bKnockbackBallistic && !airborne.bKnockbackCanLeaveArena &&
			airborne.fKnockbackRemainingSeconds == 0.f && airborne.fKnockbackVelocityY == 0.f;
		bool airborneRestoredAtExit = false;
		const auto airborneRecoveryTick = 7000u + bindTicks - 1u;
		for (std::uint32_t tick = 7001u; airborneBindValid && tick <= airborneRecoveryTick; ++tick)
		{
			airborneBindValid = advanceStatusOccurrence(*airborneBindRoom, tick,
				[&](const CGameRoom& transitionRoom)
				{
					if (tick != airborneRecoveryTick) return;
					const auto& restored = transitionRoom.m_Players.at(19600u);
					airborneRestoredAtExit = !restored.bPatternBound && restored.isCombatReady &&
						std::abs(restored.fPositionX - airborneGround.x) < 0.001f &&
						std::abs(restored.fPositionY - airborneGround.y) < 0.001f &&
						std::abs(restored.fPositionZ - airborneGround.z) < 0.001f;
				});
		}
		bool airborneCanRearm = false;
		if (airborneBindValid && airborneRestoredAtExit)
		{
			CPlayerSkillSystem::Arm_PlayerHitReaction(airborne, airborne.fPositionX - 1.f,
				airborne.fPositionZ, 1.f, 1200u, false, 0u, airborneRecoveryTick + 1u, true, false, true, 2.f);
			airborneCanRearm = airborne.bKnockbackBallistic && airborne.fKnockbackRemainingSeconds > 0.f;
		}
		if (!airborneBindValid || !airborneRestoredAtExit || !airborneCanRearm)
			std::cerr << "[BindAirborneDiagnostic] entryAndHold=" << airborneBindValid
				<< " restoredAtExit=" << airborneRestoredAtExit << " rearmed=" << airborneCanRearm
				<< " status=" << airborneBindRoom->m_strStatus << '\n';
		tests.Require(airborneBindValid && airborneRestoredAtExit && airborneCanRearm,
			"Bind after actual consecutive FOUR_SLASH launches admits support-ground pose, cancels flight, restores ground on EXIT, and permits a later reaction");

		bool invalidBindPosesRejected = true;
		for (std::uint32_t invalidCase = 0u; invalidCase < 4u; ++invalidCase)
		{
			auto invalidRoom = prepareStatusRoom("VALTAN_BIND_SLOT", 19890u + invalidCase);
			auto& invalidTarget = invalidRoom->m_Players.at(19600u);
			invalidTarget = airborneBeforeBind;
			if (invalidCase == 0u) invalidTarget.fKnockbackSupportY += 10.f;
			if (invalidCase == 1u) invalidTarget.fKnockbackSupportY = (std::numeric_limits<float>::quiet_NaN)();
			if (invalidCase == 2u)
			{
				invalidTarget.bKnockbackBallistic = false;
				invalidTarget.fKnockbackRemainingSeconds = 0.f;
				invalidTarget.eAction = PLAYER_ACTION_STATE::NONE;
			}
			if (invalidCase == 3u) invalidTarget.fPositionX = 1000000.f;
			auto& invalidBoss = invalidRoom->m_WorldEntities.front();
			invalidBoss.iPatternTargetEntityId = invalidTarget.iNetEntityId;
			SERVER_BOSS_COMBAT_STATE stagedCombat = invalidBoss.BossCombat;
			auto stagedPhase = invalidBoss.iPhase;
			SERVER_COMBAT_OBJECT_TRANSACTION transaction{};
			const bool rejected = !invalidRoom->Stage_BossPatternStageActions(invalidBoss,
				invalidRoom->m_GameplayCatalog.Active(), "VALTAN_BIND_SLOT", bindDefinition->Stages.front().strActionId,
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 7000u, stagedCombat, stagedPhase, transaction);
			invalidBindPosesRejected = invalidBindPosesRejected && rejected && !invalidTarget.bPatternBound &&
				invalidRoom->m_strStatus.find("Boss player-bind restore pose is not navigable") != std::string::npos &&
				invalidRoom->m_strStatus.find("target=") != std::string::npos &&
				invalidRoom->m_strStatus.find("supportY=") != std::string::npos;
			std::cout << "[BindAirborneDiagnostic] invalidCase=" << invalidCase
				<< " rejected=" << rejected << " status=" << invalidRoom->m_strStatus << '\n';
		}
		tests.Require(invalidBindPosesRejected,
			"Bind retains exact navigation and the 1.5m support guard for invalid support, nonfinite support, nonballistic floating and nonwalkable targets with failure-only pose diagnostics");

		auto bindRoom = prepareStatusRoom("VALTAN_BIND_SLOT", 19800u);
		const bool bindStarted = advanceStatusOccurrence(*bindRoom, 2000u);
		SERVER_WORLD_ENTITY& bindBoss = bindRoom->m_WorldEntities.front();
		const auto boundEntry = std::find_if(
			bindRoom->m_Players.begin(), bindRoom->m_Players.end(),
			[](const auto& entry) { return entry.second.bPatternBound; });
		const std::size_t boundCount = static_cast<std::size_t>(std::count_if(
			bindRoom->m_Players.begin(), bindRoom->m_Players.end(),
			[](const auto& entry) { return entry.second.bPatternBound; }));
		bool bindOccurrenceValid = bindStarted &&
			bindRoom->m_Players.end() != boundEntry && 1u == boundCount;
		PLAYER_ID boundPlayerId = INVALID_PLAYER_ID;
		float bindRestoreX = 0.f;
		float bindRestoreY = 0.f;
		float bindRestoreZ = 0.f;
		if (bindOccurrenceValid)
		{
			boundPlayerId = boundEntry->first;
			const SERVER_PLAYER& bound = boundEntry->second;
			bindRestoreX = bound.fPatternBindRestoreX;
			bindRestoreY = bound.fPatternBindRestoreY;
			bindRestoreZ = bound.fPatternBindRestoreZ;
			bindOccurrenceValid =
				bindBoss.iPatternTargetEntityId == bound.iNetEntityId &&
				bindDurationMs == bindBoss.iPatternStageDurationMs &&
				0u != bound.iCurrentHp &&
				bindTicks == bound.iPatternBindEndTick - 2000u &&
				std::abs(bound.fPositionX - bindRestoreX) < 0.001f &&
				std::abs(bound.fPositionY - bindRestoreY - 5.f) < 0.001f &&
				std::abs(bound.fPositionZ - bindRestoreZ) < 0.001f &&
				bound.isCombatReady && bound.bPatternBindRestoreCombatReady && !bound.hasMoveGoal &&
				!bindRoom->m_Players.at(19602u).bPatternBound;
		}
		recordBindState("entry", *bindRoom, boundPlayerId, bindOccurrenceValid);
		if (bindOccurrenceValid)
		{
			const SESSION_ID sessionId = 20600u + (boundPlayerId - 19600u);
			C2S_MOVE move{};
			move.iClientSequence = 1u;
			move.fGoalX = bindRestoreX + 1.f;
			move.fGoalZ = bindRestoreZ;
			bindRoom->Handle_Move(sessionId, move);
			C2S_USE_SKILL skill{};
			skill.iClientSequence = 1u;
			skill.iSkillId = 34040u;
			skill.fAimX = bindRestoreX + 1.f;
			skill.fAimZ = bindRestoreZ;
			const std::uint32_t resourceBefore =
				bindRoom->m_Players.at(boundPlayerId).iCurrentResource;
			bindRoom->Handle_UseSkill(sessionId, skill);
			const SERVER_PLAYER& blocked = bindRoom->m_Players.at(boundPlayerId);
			bindOccurrenceValid = !blocked.hasMoveGoal &&
				INVALID_SKILL_ID == blocked.iCurrentSkillId &&
				0u == blocked.iLastSkillSequence &&
				resourceBefore == blocked.iCurrentResource;
		}
		recordBindState("input-block", *bindRoom, boundPlayerId, bindOccurrenceValid);
		for (std::uint32_t tick = 2001u;
			bindOccurrenceValid && tick < bindRecoveryTick; ++tick)
		{
			bindOccurrenceValid = advanceStatusOccurrence(*bindRoom, tick);
		}
		const auto lastBoundTickEntry = bindRoom->m_Players.find(boundPlayerId);
		const bool boundThroughLastTick = bindOccurrenceValid &&
			bindRoom->m_Players.end() != lastBoundTickEntry &&
			lastBoundTickEntry->second.bPatternBound &&
			2000u + bindTicks == lastBoundTickEntry->second.iPatternBindEndTick;
		recordBindState("last-bound", *bindRoom, boundPlayerId, boundThroughLastTick);
		const float pendingBindPushSeconds = bindRoom->m_Players.end() != lastBoundTickEntry ?
			lastBoundTickEntry->second.fKnockbackRemainingSeconds : 0.f;
		const auto hpBeforeRecovery = bindRoom->m_Players.end() != lastBoundTickEntry ?
			lastBoundTickEntry->second.iCurrentHp : 0u;
		bool releasedOnRecoveryEntry = false;
		bindOccurrenceValid = bindOccurrenceValid &&
			advanceStatusOccurrence(*bindRoom, bindRecoveryTick, [&](const CGameRoom& transitionRoom)
			{
				const auto restored = transitionRoom.m_Players.find(boundPlayerId);
				const auto& transitionBoss = transitionRoom.m_WorldEntities.front();
				// Observe the actual EXIT transaction before normal movement for this tick.
				releasedOnRecoveryEntry = "RECOVERY" == transitionBoss.strPatternStageId &&
					recoveryDurationMs == transitionBoss.iPatternStageDurationMs &&
					transitionRoom.m_Players.end() != restored && !restored->second.bPatternBound &&
					INVALID_NET_ENTITY_ID == restored->second.iPatternBindOwnerNetEntityId &&
					0u == restored->second.iPatternBindEndTick && restored->second.isCombatReady &&
					std::abs(restored->second.fPositionX - bindRestoreX) < 0.001f &&
					std::abs(restored->second.fPositionY - bindRestoreY) < 0.001f &&
					std::abs(restored->second.fPositionZ - bindRestoreZ) < 0.001f &&
					restored->second.iCurrentHp == hpBeforeRecovery &&
					std::abs(restored->second.fKnockbackRemainingSeconds - pendingBindPushSeconds) < 0.0001f;
				recordBindState("exit-before-player-tick", transitionRoom, boundPlayerId, releasedOnRecoveryEntry);
			});
		const auto recoveryBindEntry = bindRoom->m_Players.find(boundPlayerId);
		// The earlier authored stomp remains damageable during Bind. Its pending
		// push resumes only after EXIT restores the pose and releases the input lock.
		const bool pendingPushResumedAfterRelease = bindOccurrenceValid && releasedOnRecoveryEntry &&
			pendingBindPushSeconds > 0.f && hpBeforeRecovery > 0u && hpBeforeRecovery < 100000u &&
			bindRoom->m_Players.end() != recoveryBindEntry && !recoveryBindEntry->second.bPatternBound &&
			recoveryBindEntry->second.isCombatReady &&
			0u == recoveryBindEntry->second.iPatternBindOwnerNetEntityId &&
			0u == recoveryBindEntry->second.iPatternBindEndTick &&
			recoveryBindEntry->second.iCurrentHp == hpBeforeRecovery &&
			recoveryBindEntry->second.fKnockbackRemainingSeconds < pendingBindPushSeconds &&
			(std::abs(recoveryBindEntry->second.fPositionX - bindRestoreX) > 0.001f ||
			 std::abs(recoveryBindEntry->second.fPositionZ - bindRestoreZ) > 0.001f);
		recordBindState("recovery-after-player-tick", *bindRoom, boundPlayerId, pendingPushResumedAfterRelease);
		bindDiagnostics << "[BindDiagnostic] expectedRestore=" << bindRestoreX << ',' << bindRestoreY << ',' << bindRestoreZ << '\n';
		for (std::uint32_t tick = bindRecoveryTick + 1u;
			bindOccurrenceValid && tick <= bindRecoveryTick + recoveryTicks &&
			0u == bindBoss.PatternTerminalReceipt.iPatternSequence; ++tick)
		{
			bindOccurrenceValid = advanceStatusOccurrence(*bindRoom, tick);
		}
		const auto releasedBindEntry = bindRoom->m_Players.find(boundPlayerId);
		/* Exact restoration is asserted at the EXIT transaction above. Normal
		   movement then consumes the pending hold-stage stomp push, and Recovery's
		   separate authored roar at 900 ms can move the released player again. */
		const bool bindExited = bindOccurrenceValid && boundThroughLastTick &&
			releasedOnRecoveryEntry && pendingPushResumedAfterRelease &&
			bindRoom->m_Players.end() != releasedBindEntry &&
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
				bindBoss.PatternTerminalReceipt.eResult &&
			!releasedBindEntry->second.bPatternBound &&
			INVALID_NET_ENTITY_ID ==
				releasedBindEntry->second.iPatternBindOwnerNetEntityId &&
			0u == releasedBindEntry->second.iPatternBindEndTick &&
			releasedBindEntry->second.isCombatReady;

		recordBindState("completed", *bindRoom, boundPlayerId, bindExited);
		auto cancelledBindRoom = prepareStatusRoom("VALTAN_BIND_SLOT", 19900u);
		bool bindCancelValid = advanceStatusOccurrence(*cancelledBindRoom, 2300u);
		const auto cancelledEntry = std::find_if(
			cancelledBindRoom->m_Players.begin(), cancelledBindRoom->m_Players.end(),
			[](const auto& entry) { return entry.second.bPatternBound; });
		PLAYER_ID cancelledPlayerId = INVALID_PLAYER_ID;
		float cancelRestoreX = 0.f;
		float cancelRestoreY = 0.f;
		float cancelRestoreZ = 0.f;
		if (bindCancelValid && cancelledBindRoom->m_Players.end() != cancelledEntry)
		{
			cancelledPlayerId = cancelledEntry->first;
			cancelRestoreX = cancelledEntry->second.fPatternBindRestoreX;
			cancelRestoreY = cancelledEntry->second.fPatternBindRestoreY;
			cancelRestoreZ = cancelledEntry->second.fPatternBindRestoreZ;
			bindCancelValid = std::abs(
				cancelledEntry->second.fPositionY - cancelRestoreY - 5.f) <
				0.001f;
			++cancelledBindRoom->m_WorldEntities.front().iPatternSequence;
			cancelledBindRoom->m_iServerTick = 2300u;
			cancelledBindRoom->Update_Players(1.f / 30.f);
			cancelledBindRoom->m_iServerTick = 2301u;
		}
		else
		{
			bindCancelValid = false;
		}
		const auto cancelledBindEntry =
			cancelledBindRoom->m_Players.find(cancelledPlayerId);
		bindCancelValid = bindCancelValid &&
			cancelledBindRoom->m_Players.end() != cancelledBindEntry &&
			!cancelledBindEntry->second.bPatternBound &&
			INVALID_NET_ENTITY_ID ==
				cancelledBindEntry->second.iPatternBindOwnerNetEntityId &&
			std::abs(cancelledBindEntry->second.fPositionX - cancelRestoreX) <
				0.001f &&
			std::abs(cancelledBindEntry->second.fPositionY - cancelRestoreY) <
				0.001f &&
			std::abs(cancelledBindEntry->second.fPositionZ - cancelRestoreZ) <
				0.001f &&
			cancelledBindEntry->second.isCombatReady;
		recordBindState("cancelled", *cancelledBindRoom, cancelledPlayerId, bindCancelValid);
		if (!bindOccurrenceValid || !bindExited || !bindCancelValid)
		{
			std::cerr << "[BindDiagnostic] summary occurrence=" << bindOccurrenceValid
				<< " lastBound=" << boundThroughLastTick << " recovery=" << releasedOnRecoveryEntry
				<< " resumedPush=" << pendingPushResumedAfterRelease
				<< " exited=" << bindExited << " cancel=" << bindCancelValid
				<< " duration=" << bindDurationMs << " ticks=" << bindTicks
				<< " recoveryTick=" << bindRecoveryTick << " recoveryTicks=" << recoveryTicks << '\n'
				<< bindDiagnostics.str();
		}
		tests.Require(
			bindOccurrenceValid && bindExited && bindCancelValid,
			"Catalog-loaded Bind slot locks one random alive target at Y+5m for the catalog-authored duration, blocks movement/skills, restores on Recovery entry, and also restores on occurrence cancel");

		auto soloBindRoom = prepareStatusRoom("VALTAN_BIND_SLOT", 19950u);
		SERVER_PLAYER soloPlayer = soloBindRoom->m_Players.at(19600u);
		soloBindRoom->m_Players.clear();
		soloBindRoom->m_PlayerIdByEntityId.clear();
		soloBindRoom->m_PlayerIdBySessionId.clear();
		soloBindRoom->m_Players.emplace(soloPlayer.iPlayerId, soloPlayer);
		soloBindRoom->m_PlayerIdByEntityId.emplace(
			soloPlayer.iNetEntityId, soloPlayer.iPlayerId);
		soloBindRoom->m_PlayerIdBySessionId.emplace(
			20600u, soloPlayer.iPlayerId);
		bool soloBindValid = advanceStatusOccurrence(*soloBindRoom, 2600u);
		SERVER_WORLD_ENTITY& soloBindBoss =
			soloBindRoom->m_WorldEntities.front();
		soloBindValid = soloBindValid &&
			soloBindRoom->m_Players.at(19600u).bPatternBound &&
			soloBindRoom->m_Players.at(19600u).isCombatReady &&
			soloBindRoom->m_Players.at(19600u).bPatternBindRestoreCombatReady;
		const auto soloRecoveryTick = 2600u + bindTicks - 1u;
		for (std::uint32_t tick = 2601u;
			soloBindValid && tick <= soloRecoveryTick; ++tick)
		{
			soloBindValid = advanceStatusOccurrence(*soloBindRoom, tick);
		}
		soloBindValid = soloBindValid &&
			"RECOVERY" == soloBindBoss.strPatternStageId &&
			!soloBindRoom->m_Players.at(19600u).bPatternBound &&
			soloBindRoom->m_Players.at(19600u).isCombatReady &&
			!soloBindBoss.bMechanicLedgerRequiresReset;
		std::uint32_t soloTick = soloRecoveryTick + 1u;
		for (; soloBindValid && soloTick <= soloRecoveryTick + recoveryTicks &&
			0u == soloBindBoss.PatternTerminalReceipt.iPatternSequence;
			++soloTick)
		{
			soloBindValid = advanceStatusOccurrence(*soloBindRoom, soloTick);
		}
		const bool soloBindCompleted = soloBindValid &&
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
				soloBindBoss.PatternTerminalReceipt.eResult &&
			soloBindBoss.strPatternId.empty() &&
			!soloBindBoss.bMechanicLedgerRequiresReset &&
			advanceStatusOccurrence(*soloBindRoom, soloTick) &&
			!soloBindBoss.bMechanicLedgerRequiresReset &&
			(SERVER_ENTITY_ACTION::IDLE == soloBindBoss.eAction ||
			 SERVER_ENTITY_ACTION::CHASE == soloBindBoss.eAction ||
			 !soloBindBoss.strPatternId.empty());
		tests.Require(
			soloBindCompleted,
			"A solo Bind occurrence keeps its bound owner alive through the targetless hold, releases on Recovery, completes terminally, and returns to selection without latching mechanic reset");

		SERVER_PLAYER unresolvedBind = bindRoom->m_Players.begin()->second;
		const float validFallbackX = unresolvedBind.fPositionX;
		const float validFallbackZ = unresolvedBind.fPositionZ;
		const float invalidCoordinate =
			(std::numeric_limits<float>::quiet_NaN)();
		unresolvedBind.bPatternBound = true;
		unresolvedBind.iPatternBindOwnerNetEntityId = 19800u;
		unresolvedBind.iPatternBindSequence = 777u;
		unresolvedBind.iPatternBindEndTick = 999u;
		unresolvedBind.fPatternBindRestoreX = invalidCoordinate;
		unresolvedBind.fPatternBindRestoreY = invalidCoordinate;
		unresolvedBind.fPatternBindRestoreZ = invalidCoordinate;
		unresolvedBind.fPositionX = invalidCoordinate;
		unresolvedBind.fPositionY = invalidCoordinate;
		unresolvedBind.fPositionZ = invalidCoordinate;
		unresolvedBind.strSpawnPlacementId.clear();
		unresolvedBind.bPatternBindRestoreCombatReady = true;
		unresolvedBind.isCombatReady = false;
		const bool unresolvedPreserved =
			!bindRoom->Restore_PatternBoundPlayer(unresolvedBind) &&
			unresolvedBind.bPatternBound &&
			19800u == unresolvedBind.iPatternBindOwnerNetEntityId &&
			777u == unresolvedBind.iPatternBindSequence &&
			999u == unresolvedBind.iPatternBindEndTick &&
			!unresolvedBind.isCombatReady;
		unresolvedBind.fPositionX = validFallbackX;
		unresolvedBind.fPositionZ = validFallbackZ;
		const bool fallbackCommitted =
			bindRoom->Restore_PatternBoundPlayer(unresolvedBind) &&
			!unresolvedBind.bPatternBound &&
			INVALID_NET_ENTITY_ID ==
				unresolvedBind.iPatternBindOwnerNetEntityId &&
			0u == unresolvedBind.iPatternBindSequence &&
			0u == unresolvedBind.iPatternBindEndTick &&
			unresolvedBind.isCombatReady &&
			std::isfinite(unresolvedBind.fPositionY);
		tests.Require(
			unresolvedPreserved && fallbackCommitted,
			"Bind restoration preserves ownership when no safe pose can commit, then clears it only after a current-position navigation fallback succeeds");

		auto silenceRoom = prepareStatusRoom("VALTAN_SILENCE_SLOT", 20000u);
		silenceRoom->m_WorldEntities.front().PendingPatternIds.push_back(
			"VALTAN_GROGGY_FOLLOWUP");
		bool silenceOccurrenceValid = advanceStatusOccurrence(*silenceRoom, 3000u);
		SERVER_WORLD_ENTITY& silenceBoss = silenceRoom->m_WorldEntities.front();
		SERVER_PLAYER& silenced = silenceRoom->m_Players.at(19600u);
		silenceOccurrenceValid = silenceOccurrenceValid &&
			2633u == silenceBoss.iPatternStageDurationMs &&
			"STEP_01" == silenceBoss.strPatternStageId &&
			3229u == silenced.iSilenceEndTick &&
			229u == silenced.iSilenceDurationTicks &&
			3229u == silenceRoom->m_Players.at(19601u).iSilenceEndTick &&
			0u == silenceRoom->m_Players.at(19602u).iSilenceEndTick;
		for (std::uint32_t tick = 3001u;
			silenceOccurrenceValid && tick < 3078u; ++tick)
		{
			silenceOccurrenceValid = advanceStatusOccurrence(*silenceRoom, tick);
		}
		silenceOccurrenceValid = silenceOccurrenceValid &&
			advanceStatusOccurrence(*silenceRoom, 3078u) &&
			"SILENCE_APPLY" == silenceBoss.strPatternStageId &&
			100u == silenceBoss.iPatternStageDurationMs &&
			151u == silenced.iSilenceEndTick - 3078u &&
			229u == silenced.iSilenceDurationTicks &&
			3229u == silenceRoom->m_Players.at(19601u).iSilenceEndTick &&
			0u == silenceRoom->m_Players.at(19602u).iSilenceEndTick;
		const std::uint32_t silencePatternSequence =
			silenceBoss.iPatternSequence;
		const std::uint32_t silenceResourceBefore = silenced.iCurrentResource;
		C2S_USE_SKILL silencedSkill{};
		silencedSkill.iClientSequence = 1u;
		silencedSkill.iSkillId = 34040u;
		silencedSkill.fAimX = silenced.fPositionX + 1.f;
		silencedSkill.fAimZ = silenced.fPositionZ;
		silenceRoom->Handle_UseSkill(20600u, silencedSkill);
		silenceOccurrenceValid = silenceOccurrenceValid &&
			INVALID_SKILL_ID == silenced.iCurrentSkillId &&
			0u == silenced.iLastSkillSequence &&
			silenceResourceBefore == silenced.iCurrentResource;
		bool silenceCompletedBeforeNextPattern = false;
		bool nextPatternStartedDuringSilence = false;
		for (std::uint32_t tick = 3079u;
			silenceOccurrenceValid && tick < 3229u; ++tick)
		{
			silenceOccurrenceValid = advanceStatusOccurrence(*silenceRoom, tick);
			silenceCompletedBeforeNextPattern =
				silenceCompletedBeforeNextPattern ||
				(silenceBoss.strPatternId.empty() &&
				 silencePatternSequence ==
					silenceBoss.PatternTerminalReceipt.iPatternSequence &&
				 SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
					silenceBoss.PatternTerminalReceipt.eResult);
			nextPatternStartedDuringSilence =
				nextPatternStartedDuringSilence ||
				("VALTAN_GROGGY_FOLLOWUP" == silenceBoss.strPatternId &&
				 silenceBoss.iPatternSequence != silencePatternSequence &&
				 3229u == silenced.iSilenceEndTick);
		}
		silenceRoom->Handle_UseSkill(20600u, silencedSkill);
		const bool rejectedThroughLastTick =
			INVALID_SKILL_ID == silenced.iCurrentSkillId &&
			0u == silenced.iLastSkillSequence;
		silenceOccurrenceValid = silenceOccurrenceValid &&
			advanceStatusOccurrence(*silenceRoom, 3229u);
		silenceRoom->Handle_UseSkill(20600u, silencedSkill);
		tests.Require(
			silenceOccurrenceValid && rejectedThroughLastTick &&
			silenceCompletedBeforeNextPattern &&
			nextPatternStartedDuringSilence &&
			0u == silenced.iSilenceEndTick &&
			0u == silenced.iSilenceDurationTicks &&
			PLAYER_ACTION_STATE::SKILL == silenced.eAction &&
			34040u == silenced.iCurrentSkillId &&
			1u == silenced.iLastSkillSequence &&
			silenced.iCurrentResource < silenceResourceBefore,
			"Catalog-loaded Silence applies immediately for 7633 ms, remains active through its nonvisual tail and the next queued Pattern, then admits the same unconsumed command at the exact deadline");
	}
	{
		namespace fs = std::filesystem;
		CGameplayCatalog stagedCatalog;
		const bool stagedLoaded = stagedCatalog.Load();
		const GameplayDataRevision bootstrapRevision =
			stagedCatalog.Get_ActiveRevision();
		GameplayDataRevision parentRevision = bootstrapRevision;
		parentRevision.Bytes[0] ^= 0x40u;
		if (!parentRevision.Is_Valid() || parentRevision == bootstrapRevision)
			parentRevision.Bytes[1] ^= 1u;
		GameplayDataRevision wrongBootstrapRevision = bootstrapRevision;
		wrongBootstrapRevision.Bytes[1] ^= 0x20u;
		if (!wrongBootstrapRevision.Is_Valid() ||
			wrongBootstrapRevision == bootstrapRevision)
		{
			wrongBootstrapRevision.Bytes[2] ^= 1u;
		}
		std::vector<wchar_t> pathBuffer(32768u);
		fs::path dataRoot;
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != configuredLength && configuredLength < pathBuffer.size())
			dataRoot = fs::path(pathBuffer.data()).lexically_normal();
		else
		{
			const DWORD moduleLength = GetModuleFileNameW(
				nullptr, pathBuffer.data(), static_cast<DWORD>(pathBuffer.size()));
			if (0u != moduleLength && moduleLength < pathBuffer.size())
			{
				dataRoot = fs::path(pathBuffer.data()).parent_path().parent_path() /
					L"DataFiles";
			}
		}
		std::error_code pathError;
		const fs::path bootstrapPath = fs::canonical(
			dataRoot / L"Gameplay" / L"Gameplay.bootstrap", pathError);
		struct PRODUCTION_STACK_HASH_CONTEXT final
		{
			const fs::path* pPath = nullptr;
			GameplayDataRevision Revision{};
			std::string Status;
			bool hashed = false;
		} hashContext{ &bootstrapPath };
		const auto hashOnProductionStack = [](void* opaque)
		{
			PRODUCTION_STACK_HASH_CONTEXT& context =
				*static_cast<PRODUCTION_STACK_HASH_CONTEXT*>(opaque);
			context.hashed = CServerApp::Hash_GameplayFileForAdmission(
				*context.pPath, context.Revision, context.Status);
		};
		const bool hashedWithinProductionStack = !pathError &&
			Run_WithProductionServerStack(
				hashOnProductionStack, &hashContext);
		tests.Require(
			hashedWithinProductionStack && hashContext.hashed &&
				hashContext.Revision.Is_Valid(),
			"Hash a gameplay candidate on Server's production 1 MiB thread stack");
		const bool rejectedWrongContent = stagedLoaded && !pathError &&
			!stagedCatalog.Load_FromBootstrap(
				bootstrapPath, wrongBootstrapRevision, parentRevision) &&
			bootstrapRevision == stagedCatalog.Get_ActiveRevision();
		const GameplayDataRevision invalidRevision{};
		const bool rejectedInvalidParent = !stagedCatalog.Load_FromBootstrap(
			bootstrapPath, bootstrapRevision, invalidRevision) &&
			bootstrapRevision == stagedCatalog.Get_ActiveRevision();
		const fs::path nonCanonicalPath = bootstrapPath.parent_path() /
			L".." / L"Gameplay" / L"Gameplay.bootstrap";
		const bool rejectedNonCanonicalPath =
			!stagedCatalog.Load_FromBootstrap(
				nonCanonicalPath, bootstrapRevision, parentRevision) &&
			bootstrapRevision == stagedCatalog.Get_ActiveRevision();
		const bool admittedParent = stagedCatalog.Load_FromBootstrap(
			bootstrapPath, bootstrapRevision, parentRevision) &&
			parentRevision == stagedCatalog.Get_ActiveRevision();
		if (!(rejectedWrongContent && rejectedInvalidParent &&
			rejectedNonCanonicalPath && admittedParent))
		{
			std::cerr << "[StagedBootstrapDiagnostic] loaded=" << stagedLoaded
				<< " wrongContent=" << rejectedWrongContent
				<< " invalidParent=" << rejectedInvalidParent
				<< " nonCanonical=" << rejectedNonCanonicalPath
				<< " admittedParent=" << admittedParent
				<< " pathError=" << pathError.value()
				<< " status=" << stagedCatalog.Get_Status() << "\n";
		}
		tests.Require(
			rejectedWrongContent && rejectedInvalidParent &&
			rejectedNonCanonicalPath && admittedParent,
			"Load an immutable staged bootstrap and expose its verified parent revision only");
	}
	{
		const BOSS_RUNTIME_PROFILE* activeValtan =
			catalog.Find_Boss("BOSS_VALTAN");
		std::string compatibilityStatus;
		const auto rejectsResetField =
			[activeValtan, &compatibilityStatus](const auto& mutate)
		{
			if (nullptr == activeValtan) return false;
			BOSS_RUNTIME_PROFILE candidate = *activeValtan;
			mutate(candidate);
			compatibilityStatus.clear();
			return !CServerApp::Validate_ValtanHotReloadBaseProfile(
				activeValtan, &candidate, compatibilityStatus) &&
				std::string::npos !=
					compatibilityStatus.find("ENCOUNTER_RESET required");
		};
		BOSS_RUNTIME_PROFILE unchanged = nullptr == activeValtan ?
			BOSS_RUNTIME_PROFILE{} : *activeValtan;
		const bool acceptsUnchanged = nullptr != activeValtan &&
			CServerApp::Validate_ValtanHotReloadBaseProfile(
				activeValtan, &unchanged, compatibilityStatus) &&
			compatibilityStatus.empty();
		const bool rejectsEveryResetField =
			rejectsResetField([](BOSS_RUNTIME_PROFILE& value)
				{ ++value.iMaximumHp; }) &&
			rejectsResetField([](BOSS_RUNTIME_PROFILE& value)
				{ ++value.iMaximumHealthBars; }) &&
			rejectsResetField([](BOSS_RUNTIME_PROFILE& value)
				{ ++value.iAttackPower; }) &&
			rejectsResetField([](BOSS_RUNTIME_PROFILE& value)
				{ value.fCollisionRadius += 0.25f; }) &&
			rejectsResetField([](BOSS_RUNTIME_PROFILE& value)
				{ value.fEngageDistance += 0.25f; }) &&
			rejectsResetField([](BOSS_RUNTIME_PROFILE& value)
				{ value.fMoveSpeed += 0.25f; }) &&
			!CServerApp::Validate_ValtanHotReloadBaseProfile(
				activeValtan, nullptr, compatibilityStatus);
		tests.Require(
			acceptsUnchanged && rejectsEveryResetField,
			"Reject every live Valtan base-field change as ENCOUNTER_RESET instead of falsely committing HOT_RELOAD");
	}
}

