#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "BossCombatRuntime.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
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
			const std::uint32_t tick)
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
			room.m_iServerTick = 0u == tick ? 0u : tick - 1u;
			room.Update_Players(1.f / 30.f);
			room.m_iServerTick = tick;
			return room.Is_Ready();
		};

		auto magicRoom = prepareStatusRoom("VALTAN_STAGGER_SLOT", 19700u);
		bool magicOccurrenceValid = advanceStatusOccurrence(*magicRoom, 1000u);
		SERVER_WORLD_ENTITY& magicBoss = magicRoom->m_WorldEntities.front();
		const float magicBaseY = magicBoss.fSpawnPositionY;
		magicOccurrenceValid = magicOccurrenceValid &&
			"VALTAN_STAGGER_SLOT" == magicBoss.strPatternId &&
			"CHANNEL" == magicBoss.strPatternStageId &&
			"valtan.authoring.stagger-slot.channel" == magicBoss.strActionId &&
			12000u == magicBoss.iPatternStageDurationMs &&
			!magicBoss.bPatternVerticalOffsetApplied &&
			magicBoss.bPatternStageVerticalOffsetApplied &&
			std::abs(magicBoss.fPatternStageVerticalBaseY - magicBaseY) < 0.001f &&
			std::abs(magicBoss.fPositionY - magicBaseY - 0.5f) < 0.001f &&
			BOSS_PATTERN_BOSS_RESPONSE_KIND::ACCUMULATED_HEALTH_DAMAGE ==
				magicBoss.ePatternBossResponseKind &&
			1000u == magicBoss.iPatternBossResponseThreshold &&
			0u == magicBoss.iPatternBossResponseAccumulatedHealthDamage &&
			!magicBoss.bPatternBossResponsePublished;

		BOSS_INCOMING_HIT underThreshold{};
		underThreshold.iSourcePlayerId = 19600u;
		underThreshold.iSkillId = 34040u;
		underThreshold.iRawDamage = 999u;
		underThreshold.iServerTick = 1001u;
		const BOSS_HIT_RESULT underThresholdResult =
			CBossCombatRuntime::Apply_PlayerHit(magicBoss, underThreshold);
		magicOccurrenceValid = magicOccurrenceValid &&
			999u == underThresholdResult.iHealthDamage &&
			!underThresholdResult.bHealthDamageThresholdReached &&
			999u == magicBoss.iPatternBossResponseAccumulatedHealthDamage &&
			magicBoss.BossCombat.PendingOutcomes.empty();

		magicBoss.BossCombat.iShieldCurrent = 1u;
		magicBoss.BossCombat.iShieldMaximum = 1u;
		(void)CBossCombatRuntime::Set_Flag(
			magicBoss.BossCombat, SERVER_BOSS_COMBAT_FLAG::SHIELDED, true);
		BOSS_INCOMING_HIT shieldOnly = underThreshold;
		shieldOnly.iRawDamage = 1u;
		shieldOnly.iServerTick = 1002u;
		const BOSS_HIT_RESULT shieldOnlyResult =
			CBossCombatRuntime::Apply_PlayerHit(magicBoss, shieldOnly);
		magicOccurrenceValid = magicOccurrenceValid &&
			1u == shieldOnlyResult.iShieldDamage &&
			0u == shieldOnlyResult.iHealthDamage &&
			999u == magicBoss.iPatternBossResponseAccumulatedHealthDamage;

		(void)CBossCombatRuntime::Set_Flag(
			magicBoss.BossCombat, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE, true);
		BOSS_INCOMING_HIT invulnerable = underThreshold;
		invulnerable.iRawDamage = 100u;
		invulnerable.iServerTick = 1003u;
		const BOSS_HIT_RESULT invulnerableResult =
			CBossCombatRuntime::Apply_PlayerHit(magicBoss, invulnerable);
		(void)CBossCombatRuntime::Set_Flag(
			magicBoss.BossCombat, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE, false);
		magicOccurrenceValid = magicOccurrenceValid &&
			invulnerableResult.bBlockedByInvulnerability &&
			0u == invulnerableResult.iHealthDamage &&
			999u == magicBoss.iPatternBossResponseAccumulatedHealthDamage;

		BOSS_INCOMING_HIT thresholdEdge = underThreshold;
		thresholdEdge.iRawDamage = 1u;
		thresholdEdge.iServerTick = 1004u;
		const BOSS_HIT_RESULT thresholdEdgeResult =
			CBossCombatRuntime::Apply_PlayerHit(magicBoss, thresholdEdge);
		BOSS_INCOMING_HIT afterThreshold = underThreshold;
		afterThreshold.iRawDamage = 50u;
		afterThreshold.iServerTick = 1005u;
		const BOSS_HIT_RESULT afterThresholdResult =
			CBossCombatRuntime::Apply_PlayerHit(magicBoss, afterThreshold);
		const bool magicPublishedOnce = magicOccurrenceValid &&
			thresholdEdgeResult.bHealthDamageThresholdReached &&
			!afterThresholdResult.bHealthDamageThresholdReached &&
			1000u == magicBoss.iPatternBossResponseAccumulatedHealthDamage &&
			magicBoss.bPatternBossResponsePublished &&
			1u == magicBoss.BossCombat.PendingOutcomes.size() &&
			BOSS_PATTERN_STAGE_OUTCOME::HEALTH_DAMAGE_THRESHOLD_REACHED ==
				magicBoss.BossCombat.PendingOutcomes.front().eOutcome;
		magicOccurrenceValid = magicPublishedOnce &&
			advanceStatusOccurrence(*magicRoom, 1006u) &&
			magicBoss.PendingPatternFollowup.Is_Pending() &&
			"VALTAN_GROGGY_FOLLOWUP" ==
				magicBoss.PendingPatternFollowup.strPatternId &&
			magicBoss.strPatternId.empty() &&
			!magicBoss.bPatternVerticalOffsetApplied &&
			!magicBoss.bPatternStageVerticalOffsetApplied &&
			std::abs(magicBoss.fPositionY - magicBaseY) < 0.001f &&
			BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE ==
				magicBoss.ePatternBossResponseKind &&
			0u == magicBoss.iPatternBossResponseThreshold &&
			0u == magicBoss.iPatternBossResponseAccumulatedHealthDamage &&
			!magicBoss.bPatternBossResponsePublished &&
			advanceStatusOccurrence(*magicRoom, 1007u);
		tests.Require(
			magicOccurrenceValid &&
			"VALTAN_GROGGY_FOLLOWUP" == magicBoss.strPatternId &&
			"GROGGY" == magicBoss.strPatternStageId &&
			"valtan.followup.groggy.active" == magicBoss.strActionId &&
			std::abs(magicBoss.fPositionY - magicBaseY) < 0.001f &&
			CBossCombatRuntime::Has_Flag(
				magicBoss.BossCombat, SERVER_BOSS_COMBAT_FLAG::GROGGY),
			"Magic-orb channel counts only confirmed HP damage, saturates at 1000, publishes once, restores base Y and starts the separate Groggy follow-up");

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
				player.iCurrentHp = player.iMaximumHp = 10000u;
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
				5000u == bindBoss.iPatternStageDurationMs &&
				0u != bound.iCurrentHp &&
				150u == bound.iPatternBindEndTick - 2000u &&
				std::abs(bound.fPositionX - bindRestoreX) < 0.001f &&
				std::abs(bound.fPositionY - bindRestoreY - 5.f) < 0.001f &&
				std::abs(bound.fPositionZ - bindRestoreZ) < 0.001f &&
				!bound.isCombatReady && !bound.hasMoveGoal &&
				!bindRoom->m_Players.at(19602u).bPatternBound;
		}
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
		for (std::uint32_t tick = 2001u;
			bindOccurrenceValid && tick < 2149u; ++tick)
		{
			bindOccurrenceValid = advanceStatusOccurrence(*bindRoom, tick);
		}
		const auto lastBoundTickEntry = bindRoom->m_Players.find(boundPlayerId);
		const bool boundThroughLastTick = bindOccurrenceValid &&
			bindRoom->m_Players.end() != lastBoundTickEntry &&
			lastBoundTickEntry->second.bPatternBound &&
			2150u == lastBoundTickEntry->second.iPatternBindEndTick;
		bindOccurrenceValid = bindOccurrenceValid &&
			advanceStatusOccurrence(*bindRoom, 2149u);
		const auto recoveryBindEntry = bindRoom->m_Players.find(boundPlayerId);
		const bool releasedOnRecoveryEntry = bindOccurrenceValid &&
			"RECOVERY" == bindBoss.strPatternStageId &&
			3533u == bindBoss.iPatternStageDurationMs &&
			bindRoom->m_Players.end() != recoveryBindEntry &&
			!recoveryBindEntry->second.bPatternBound &&
			INVALID_NET_ENTITY_ID ==
				recoveryBindEntry->second.iPatternBindOwnerNetEntityId &&
			0u == recoveryBindEntry->second.iPatternBindEndTick &&
			recoveryBindEntry->second.isCombatReady &&
			std::abs(recoveryBindEntry->second.fPositionX - bindRestoreX) < 0.001f &&
			std::abs(recoveryBindEntry->second.fPositionY - bindRestoreY) < 0.001f &&
			std::abs(recoveryBindEntry->second.fPositionZ - bindRestoreZ) < 0.001f;
		for (std::uint32_t tick = 2150u;
			bindOccurrenceValid && tick <= 2260u &&
			0u == bindBoss.PatternTerminalReceipt.iPatternSequence; ++tick)
		{
			bindOccurrenceValid = advanceStatusOccurrence(*bindRoom, tick);
		}
		const auto releasedBindEntry = bindRoom->m_Players.find(boundPlayerId);
		/* Recovery owns a separate authored roar hit at 900 ms. The EXIT action
		   must restore the captured pose exactly on Recovery entry, which is
		   asserted above; after that point the legitimate roar push may move the
		   released player before the occurrence completes. */
		const bool bindExited = bindOccurrenceValid && boundThroughLastTick &&
			releasedOnRecoveryEntry &&
			bindRoom->m_Players.end() != releasedBindEntry &&
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
				bindBoss.PatternTerminalReceipt.eResult &&
			!releasedBindEntry->second.bPatternBound &&
			INVALID_NET_ENTITY_ID ==
				releasedBindEntry->second.iPatternBindOwnerNetEntityId &&
			0u == releasedBindEntry->second.iPatternBindEndTick &&
			releasedBindEntry->second.isCombatReady;

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
		tests.Require(
			bindOccurrenceValid && bindExited && bindCancelValid,
			"Catalog-loaded Bind slot locks one random alive target at Y+5m for the authored 5000 ms, blocks movement/skills, restores on Recovery entry, and also restores on occurrence cancel");

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
			!soloBindRoom->m_Players.at(19600u).isCombatReady;
		for (std::uint32_t tick = 2601u;
			soloBindValid && tick <= 2749u; ++tick)
		{
			soloBindValid = advanceStatusOccurrence(*soloBindRoom, tick);
		}
		soloBindValid = soloBindValid &&
			"RECOVERY" == soloBindBoss.strPatternStageId &&
			!soloBindRoom->m_Players.at(19600u).bPatternBound &&
			soloBindRoom->m_Players.at(19600u).isCombatReady &&
			!soloBindBoss.bMechanicLedgerRequiresReset;
		std::uint32_t soloTick = 2750u;
		for (; soloBindValid && soloTick <= 2870u &&
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

