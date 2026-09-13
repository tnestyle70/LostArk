#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "BossCombatRuntime.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "PlayerSkillSystem.h"
#include "Network/PacketReader.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_ValtanPinnedGeneration(TESTS& tests, CGameplayCatalog& catalog)
{

	{
		/* This fixture uses the same Brain -> capture -> room stage-action seam
		as the fixed tick, including the real typed grab impact transactions. */
		const auto* patterns = catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto trashDefinition = std::find_if(patterns->begin(), patterns->end(),
			[](const BOSS_PATTERN_DEFINITION& row) { return row.strPatternId == "VALTAN_TRASH"; });
		const auto stageById = [&trashDefinition](const char* stageId)
			-> const BOSS_PATTERN_STAGE_DEFINITION&
		{
			return *std::find_if(trashDefinition->Stages.begin(), trashDefinition->Stages.end(),
				[stageId](const BOSS_PATTERN_STAGE_DEFINITION& row) { return row.strStageId == stageId; });
		};
		const auto prepareRoom = [&]()
		{
			auto room = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
			room->m_Players.clear();
			room->m_PlayerIdByEntityId.clear();
			room->m_WorldEntities.clear();
			SERVER_WORLD_ENTITY boss{};
			boss.iNetEntityId = 19100u;
			boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			boss.strArchetypeId = "BOSS_VALTAN";
			boss.strEncounterId = "ENCOUNTER_VALTAN";
			boss.strPlacementId = "boss.valtan.trash-contract";
			boss.iCurrentHp = boss.iMaximumHp = 80000u;
			boss.iAttackPower = catalog.Find_Boss("BOSS_VALTAN")->iAttackPower;
			boss.iMaximumHealthBars = boss.iLastEvaluatedHealthBar = 160u;
			boss.iPhase = 2u;
			boss.fPositionX = boss.fSpawnPositionX = 156.03f;
			boss.fPositionY = boss.fSpawnPositionY = 22.97f;
			boss.fPositionZ = boss.fSpawnPositionZ = -122.06f;
			boss.fEngageDistance = 100.f;
			boss.bIntroPatternConsumed = true;
			boss.bScriptedPatternPlayback = true;
			boss.bAutomaticPatternSequenceAuditionOverride = true;
			boss.PendingPatternIds.push_back("VALTAN_TRASH");
			boss.PinnedDefinitionRevision = catalog.Get_ActiveRevision();
			room->m_WorldEntities.push_back(boss);
			for (std::uint32_t ordinal = 0u; ordinal != 2u; ++ordinal)
			{
				SERVER_PLAYER player{};
				player.iPlayerId = 19200u + ordinal;
				player.iNetEntityId = 19300u + ordinal;
				player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
				player.iCurrentHp = player.iMaximumHp = 100000u;
				player.isCombatReady = true;
				/* The Lance Master counter (34580) requires the short-spear stance. */
				player.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
				player.fPositionX = boss.fPositionX;
				player.fPositionY = boss.fPositionY;
				player.fPositionZ = boss.fPositionZ + 1.f;
				room->m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
				room->m_Players.emplace(player.iPlayerId, player);
			}
			return room;
		};
		const auto advance = [&catalog](CGameRoom& room, const std::uint32_t tick)
		{
			SERVER_WORLD_ENTITY& boss = room.m_WorldEntities.front();
			const std::string beforePattern = boss.strPatternId;
			const std::string beforeAction = boss.strActionId;
			const auto beforeRevision = boss.PinnedDefinitionRevision;
			const std::uint32_t beforeSequence = boss.iPatternSequence;
			room.m_TickDamageEvents.clear();
			std::vector<SERVER_PLAYER_CAPTURE_REQUEST> captures;
			room.m_ValtanBrain.Update(boss, room.m_Players, catalog,
				room.m_ServerNavigation, 1.f / 30.f, tick, {},
				room.m_TickDamageEvents, nullptr, 1u, &captures);
			for (const auto& capture : captures)
			{
				if (!room.Capture_PlayerAttachment(capture.iPlayerNetEntityId,
					boss.iNetEntityId, capture.eAttachmentSlot, tick)) return false;
			}
			if (beforePattern != boss.strPatternId || beforeAction != boss.strActionId ||
				beforeSequence != boss.iPatternSequence)
			{
				return room.Apply_BossPatternStageTransition(boss, beforePattern,
					beforeAction, boss.strPatternId, boss.strActionId,
					beforeRevision, boss.PinnedDefinitionRevision, tick);
			}
			return true;
		};
		for (const std::uint32_t branch : { 0u, 1u, 2u, 3u })
		{
			auto room = prepareRoom();
			SERVER_WORLD_ENTITY& boss = room->m_WorldEntities.front();
			bool valid = true, captured = false, counter = false, reachedMiss = false;
			bool reachedSlam = false, reachedExecution = false, detached = false;
			bool reachedRetryWait02 = false, reachedRetryRush02 = false;
			bool reachedRetryWait03 = false, reachedRetryRush03 = false;
			bool reachedRetryExhausted = false;
			bool counterStarted = false, counterDamageLess = false;
			CPlayerSkillSystem counterSkills;
			std::uint32_t counterTick = 0u, impactTick = 0u, terminalTick = 0u;
			for (std::uint32_t tick = 1000u; tick < 2200u && valid; ++tick)
			{
				if ("STEP_08" == boss.strPatternStageId)
				{
					const float radians = boss.fYawDegrees * 3.14159265358979323846f / 180.f;
					for (auto& [id, player] : room->m_Players)
					{
						if (PLAYER_ACTION_STATE::GRABBED == player.eAction) continue;
						const bool inside = branch == 2u || (branch != 0u && id == 19200u);
						const float distance = inside ? 2.f : 12.f;
						player.fPositionX = boss.fPositionX + std::sin(radians) * distance;
						player.fPositionZ = boss.fPositionZ + std::cos(radians) * distance;
					}
				}
				if (branch == 3u && "STEP_07" == boss.strPatternStageId &&
					!counterStarted)
				{
					const float radians = boss.fYawDegrees *
						3.14159265358979323846f / 180.f;
					SERVER_PLAYER& counterPlayer = room->m_Players.at(19200u);
					counterPlayer.fPositionX = boss.fPositionX +
						std::sin(radians) * boss.fPatternCounterProxyForwardOffsetM +
						std::cos(radians) * boss.fPatternCounterProxyRightOffsetM;
					counterPlayer.fPositionZ = boss.fPositionZ +
						std::cos(radians) * boss.fPatternCounterProxyForwardOffsetM -
						std::sin(radians) * boss.fPatternCounterProxyRightOffsetM;
					C2S_USE_SKILL counterCommand{};
					counterCommand.iClientSequence = 1u;
					counterCommand.iSkillId = 34580u;
					counterCommand.fAimX = boss.fPositionX;
					counterCommand.fAimZ = boss.fPositionZ;
					counterStarted = counterSkills.Try_Start(
						counterPlayer, counterCommand, catalog, tick);
				}
				const std::uint32_t hpBeforeAdvance = boss.iCurrentHp;
				valid = advance(*room, tick);
				if (branch == 3u && counterStarted && !counter &&
					2u == room->m_Players.at(19200u).iComboStage)
				{
					counter = true;
					counterDamageLess = hpBeforeAdvance == boss.iCurrentHp &&
						room->m_TickDamageEvents.empty();
				}
				captured = captured || 0u != CValtanBrain::Classify_GrabbedPlayers(boss, room->m_Players).iGrabbedCount;
				if ("CATCH_COUNTER" == boss.strPatternStageId && 0u == counterTick) counterTick = tick;
				reachedMiss = reachedMiss || "RUSH_MISS" == boss.strPatternStageId;
				reachedRetryWait02 = reachedRetryWait02 ||
					"RECHARGE_WAIT_02" == boss.strPatternStageId;
				reachedRetryRush02 = reachedRetryRush02 ||
					"RETRY_RUSH_02" == boss.strPatternStageId;
				reachedRetryWait03 = reachedRetryWait03 ||
					"RECHARGE_WAIT_03" == boss.strPatternStageId;
				reachedRetryRush03 = reachedRetryRush03 ||
					"RETRY_RUSH_03" == boss.strPatternStageId;
				reachedRetryExhausted = reachedRetryExhausted ||
					"RETRY_EXHAUSTED" == boss.strPatternStageId;
				if ("CATCH_SLAM" == boss.strPatternStageId && !reachedSlam)
				{
					reachedSlam = true;
					impactTick = tick;
					const auto& first = room->m_Players.at(19200u);
					const auto& second = room->m_Players.at(19201u);
					detached = PLAYER_ACTION_STATE::NONE == first.eAction &&
						0u < first.iCurrentHp && first.iCurrentHp < first.iMaximumHp &&
						PLAYER_ATTACHMENT_SLOT::NONE == first.eAttachmentSlot &&
						second.iCurrentHp == second.iMaximumHp && room->m_TickDamageEvents.size() == 1u;
				}
				if ("EXECUTE_TAIL" == boss.strPatternStageId && !reachedExecution)
				{
					reachedExecution = true;
					impactTick = tick;
					detached = room->m_TickDamageEvents.size() == 2u &&
						std::all_of(room->m_Players.begin(), room->m_Players.end(), [](const auto& entry)
						{
							return entry.second.iCurrentHp == 0u && entry.second.eAction == PLAYER_ACTION_STATE::DEAD &&
								entry.second.eAttachmentSlot == PLAYER_ATTACHMENT_SLOT::NONE &&
								entry.second.iAttachmentOwnerNetEntityId == INVALID_NET_ENTITY_ID;
						});
				}
				if (branch == 3u && "GROGGY" == boss.strPatternStageId)
				{
					detached = CBossCombatRuntime::Has_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::GROGGY) &&
						room->m_Players.at(19200u).eAttachmentSlot == PLAYER_ATTACHMENT_SLOT::NONE;
				}
				if (0u != boss.PatternTerminalReceipt.iPatternSequence)
				{
					terminalTick = tick;
					break;
				}
			}
			const bool completed = terminalTick != 0u &&
				boss.PatternTerminalReceipt.eResult == SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED;
			if (branch == 0u)
				tests.Require(valid && reachedMiss && reachedRetryWait02 &&
					reachedRetryRush02 && reachedRetryWait03 && reachedRetryRush03 &&
					reachedRetryExhausted && !captured && counterTick == 0u && completed,
					"Trash NONE completes its full finite three-rush miss path before exhausting retries");
			if (branch == 1u)
				tests.Require(valid && captured && reachedSlam && !reachedExecution && detached &&
					impactTick - counterTick == 45u && completed,
					"Trash PARTIAL applies captured-only damage, detaches and completes its finite tail");
			if (branch == 2u)
				tests.Require(valid && captured && reachedExecution && detached && completed &&
					impactTick - counterTick == 45u && terminalTick - impactTick == 45u,
					"Trash ALL executes every captured living player atomically and completes its targetless 1500ms tail");
			if (branch == 3u)
				tests.Require(valid && !captured && counterStarted && counter &&
					counterDamageLess && detached && completed && !reachedSlam &&
					!reachedExecution,
					"Trash windup consumes an in-proxy active Counter guard without damage, enters groggy, and completes without capture or execution");
		}
		{
			auto room = prepareRoom();
			SERVER_WORLD_ENTITY& boss = room->m_WorldEntities.front();
			bool valid = true;
			bool outsideRejected = false;
			bool proxyGeometryLoaded = false;
			bool outsideProxyRejected = false;
			bool insideCounterStarted = false;
			bool insideAccepted = false;
			bool insideResolutionDamageLess = false;
			bool selectedAuthoredGroggy = false;
			std::uint32_t hpBeforeInsideCounter = 0u;
			CPlayerSkillSystem insideCounterSkills;
			for (std::uint32_t tick = 2000u; tick < 2800u && valid; ++tick)
			{
				if (!outsideRejected && 0u != boss.iPatternSequence &&
					"STEP_07" != boss.strPatternStageId)
				{
					BOSS_INCOMING_HIT outsideWindow{};
					outsideWindow.iCounterPower = 1u;
					outsideWindow.iServerTick = tick;
					outsideWindow.fSourceX = boss.fPositionX;
					outsideWindow.fSourceZ = boss.fPositionZ;
					outsideRejected = !CBossCombatRuntime::Apply_PlayerHit(
						boss, outsideWindow).bCounterTriggered;
				}
				if (!insideCounterStarted && "STEP_07" == boss.strPatternStageId)
				{
					proxyGeometryLoaded = boss.bPatternHasCounterProxy &&
						1.f == boss.fPatternCounterProxyForwardOffsetM &&
						0.f == boss.fPatternCounterProxyRightOffsetM &&
						2.25f == boss.fPatternCounterProxyRadiusM;
					const float radians = boss.fYawDegrees *
						3.14159265358979323846f / 180.f;
					const float forwardX = std::sin(radians);
					const float forwardZ = std::cos(radians);
					const float rightX = forwardZ;
					const float rightZ = -forwardX;
					const float proxyX = boss.fPositionX +
						forwardX * boss.fPatternCounterProxyForwardOffsetM +
						rightX * boss.fPatternCounterProxyRightOffsetM;
					const float proxyZ = boss.fPositionZ +
						forwardZ * boss.fPatternCounterProxyForwardOffsetM +
						rightZ * boss.fPatternCounterProxyRightOffsetM;
					BOSS_INCOMING_HIT outsideProxy{};
					outsideProxy.iCounterPower = 1u;
					outsideProxy.iServerTick = tick;
					outsideProxy.fSourceX = proxyX +
						boss.fPatternCounterProxyRadiusM + 1.f;
					outsideProxy.fSourceZ = proxyZ;
					outsideProxyRejected =
						!CBossCombatRuntime::Apply_PlayerHit(
							boss, outsideProxy).bCounterTriggered &&
						CBossCombatRuntime::Has_Flag(boss.BossCombat,
							SERVER_BOSS_COMBAT_FLAG::COUNTERABLE);
					SERVER_PLAYER& counterPlayer = room->m_Players.at(19200u);
					counterPlayer.fPositionX = proxyX;
					counterPlayer.fPositionZ = proxyZ;
					C2S_USE_SKILL counterCommand{};
					counterCommand.iClientSequence = 1u;
					counterCommand.iSkillId = 34580u;
					counterCommand.fAimX = boss.fPositionX;
					counterCommand.fAimZ = boss.fPositionZ;
					hpBeforeInsideCounter = boss.iCurrentHp;
					insideCounterStarted = insideCounterSkills.Try_Start(
						counterPlayer, counterCommand, catalog, tick);
				}
				valid = advance(*room, tick);
				if (insideCounterStarted && !insideAccepted &&
					2u == room->m_Players.at(19200u).iComboStage)
				{
					insideAccepted = true;
					insideResolutionDamageLess =
						hpBeforeInsideCounter == boss.iCurrentHp &&
						room->m_TickDamageEvents.empty();
				}
				selectedAuthoredGroggy = selectedAuthoredGroggy ||
					("GROGGY" == boss.strPatternStageId &&
					 "valtan.sequence.center-trash-rush-if.groggy" ==
						boss.strActionId &&
					 CBossCombatRuntime::Has_Flag(
						boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::GROGGY));
				if (selectedAuthoredGroggy)
					break;
			}

			auto disabledRoom = prepareRoom();
			SERVER_WORLD_ENTITY& disabledBoss =
				disabledRoom->m_WorldEntities.front();
			bool disabledValid = true;
			bool disabledAttempted = false;
			bool disabledCounterStarted = false;
			bool disabledRejected = false;
			bool disabledPlayerStayedGuard = false;
			bool disabledResolutionDamageLess = false;
			bool disabledEnteredGroggy = false;
			bool disabledAdvancedPastWindow = false;
			std::uint32_t disabledHpBeforeCounter = 0u;
			CPlayerSkillSystem disabledCounterSkills;
			for (std::uint32_t tick = 3000u;
				tick < 3800u && disabledValid; ++tick)
			{
				if (!disabledAttempted &&
					"STEP_07" == disabledBoss.strPatternStageId)
				{
					disabledAttempted = true;
					(void)CBossCombatRuntime::Set_Flag(
						disabledBoss.BossCombat,
						SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, false);
					const float radians = disabledBoss.fYawDegrees *
						3.14159265358979323846f / 180.f;
					SERVER_PLAYER& counterPlayer =
						disabledRoom->m_Players.at(19200u);
					counterPlayer.fPositionX = disabledBoss.fPositionX +
						std::sin(radians) *
							disabledBoss.fPatternCounterProxyForwardOffsetM +
						std::cos(radians) *
							disabledBoss.fPatternCounterProxyRightOffsetM;
					counterPlayer.fPositionZ = disabledBoss.fPositionZ +
						std::cos(radians) *
							disabledBoss.fPatternCounterProxyForwardOffsetM -
						std::sin(radians) *
							disabledBoss.fPatternCounterProxyRightOffsetM;
					C2S_USE_SKILL counterCommand{};
					counterCommand.iClientSequence = 1u;
					counterCommand.iSkillId = 34580u;
					counterCommand.fAimX = disabledBoss.fPositionX;
					counterCommand.fAimZ = disabledBoss.fPositionZ;
					disabledHpBeforeCounter = disabledBoss.iCurrentHp;
					disabledCounterStarted = disabledCounterSkills.Try_Start(
						counterPlayer, counterCommand, catalog, tick);
				}
				disabledValid = advance(*disabledRoom, tick);
				if (disabledAttempted && !disabledRejected)
				{
					disabledPlayerStayedGuard =
						1u == disabledRoom->m_Players.at(19200u).iComboStage;
					disabledResolutionDamageLess =
						disabledHpBeforeCounter == disabledBoss.iCurrentHp &&
						disabledRoom->m_TickDamageEvents.empty();
					disabledRejected = disabledPlayerStayedGuard &&
						!CBossCombatRuntime::Has_Flag(
							disabledBoss.BossCombat,
							SERVER_BOSS_COMBAT_FLAG::COUNTERABLE);
				}
				disabledEnteredGroggy = disabledEnteredGroggy ||
					"GROGGY" == disabledBoss.strPatternStageId;
				disabledAdvancedPastWindow = disabledAdvancedPastWindow ||
					(disabledAttempted &&
					 "STEP_07" != disabledBoss.strPatternStageId);
				if (disabledAdvancedPastWindow)
					break;
			}
			tests.Require(
				valid && outsideRejected && proxyGeometryLoaded &&
				outsideProxyRejected && insideCounterStarted && insideAccepted &&
				insideResolutionDamageLess &&
				selectedAuthoredGroggy && disabledValid &&
				disabledAttempted && disabledCounterStarted &&
				disabledRejected && disabledPlayerStayedGuard &&
				disabledResolutionDamageLess &&
				disabledAdvancedPastWindow && !disabledEnteredGroggy,
				"Trash counter proxy fixed-tick admission: an in-range active guard promotes without damage; outside-window/proxy and disabled windows do not transition or mutate the guard");
		}
		{
			auto room = prepareRoom();
			SERVER_WORLD_ENTITY& boss = room->m_WorldEntities.front();
			boss.PendingPatternIds.clear();
			boss.PendingPatternIds.push_back("VALTAN_TRIPLE_COUNTER");
			bool valid = true;
			bool guardStarted = false;
			bool remoteFrontRejected = false;
			bool behindRejected = false;
			bool secondGuardStarted = false;
			bool frontAccepted = false;
			bool damageLess = false;
			bool singleCounterConsumer = false;
			bool reachedSharedGroggy = false;
			std::uint32_t hpBeforeCounter = 0u;
			CPlayerSkillSystem counterSkills;
			for (std::uint32_t tick = 4000u; tick < 4300u && valid; ++tick)
			{
				if ("COUNTER_1" == boss.strPatternStageId && !guardStarted)
				{
					SERVER_PLAYER& counterPlayer = room->m_Players.at(19200u);
					SERVER_PLAYER& facingTarget = room->m_Players.at(19201u);
					counterPlayer.fPositionX = boss.fPositionX;
					counterPlayer.fPositionZ = boss.fPositionZ + 5.f;
					facingTarget.fPositionX = boss.fPositionX;
					facingTarget.fPositionZ = boss.fPositionZ + 1.f;
					C2S_USE_SKILL counterCommand{};
					counterCommand.iClientSequence = 1u;
					counterCommand.iSkillId = 34580u;
					counterCommand.fAimX = boss.fPositionX;
					counterCommand.fAimZ = boss.fPositionZ;
					guardStarted = counterSkills.Try_Start(
						counterPlayer, counterCommand, catalog, tick);
				}
				else if (remoteFrontRejected && !behindRejected)
				{
					SERVER_PLAYER& counterPlayer = room->m_Players.at(19200u);
					counterPlayer.fPositionX = boss.fPositionX;
					counterPlayer.fPositionZ = boss.fPositionZ - 2.f;
				}
				else if (behindRejected && !frontAccepted)
				{
					SERVER_PLAYER& counterPlayer = room->m_Players.at(19200u);
					SERVER_PLAYER& secondCounterPlayer = room->m_Players.at(19201u);
					counterPlayer.fPositionX = boss.fPositionX;
					counterPlayer.fPositionZ = boss.fPositionZ + 2.f;
					C2S_USE_SKILL secondCounterCommand{};
					secondCounterCommand.iClientSequence = 1u;
					secondCounterCommand.iSkillId = 34580u;
					secondCounterCommand.fAimX = boss.fPositionX;
					secondCounterCommand.fAimZ = boss.fPositionZ;
					secondGuardStarted = counterSkills.Try_Start(
						secondCounterPlayer, secondCounterCommand, catalog, tick);
					hpBeforeCounter = boss.iCurrentHp;
				}

				valid = advance(*room, tick);
				if (guardStarted && !remoteFrontRejected)
				{
					remoteFrontRejected =
						"COUNTER_1" == boss.strPatternStageId &&
						1u == room->m_Players.at(19200u).iComboStage &&
						CBossCombatRuntime::Has_Flag(
							boss.BossCombat,
							SERVER_BOSS_COMBAT_FLAG::COUNTERABLE);
				}
				else if (remoteFrontRejected && !behindRejected)
				{
					behindRejected =
						"COUNTER_1" == boss.strPatternStageId &&
						1u == room->m_Players.at(19200u).iComboStage &&
						CBossCombatRuntime::Has_Flag(
							boss.BossCombat,
							SERVER_BOSS_COMBAT_FLAG::COUNTERABLE);
				}
				else if (behindRejected && !frontAccepted && secondGuardStarted)
				{
					frontAccepted = 2u ==
						room->m_Players.at(19200u).iComboStage;
					damageLess = hpBeforeCounter == boss.iCurrentHp &&
						room->m_TickDamageEvents.empty();
					singleCounterConsumer = frontAccepted &&
						1u == room->m_Players.at(19201u).iComboStage &&
						boss.BossCombat.PendingOutcomes.empty() &&
						!CBossCombatRuntime::Has_Flag(
							boss.BossCombat,
							SERVER_BOSS_COMBAT_FLAG::COUNTERABLE);
				}
				reachedSharedGroggy = reachedSharedGroggy ||
					("VALTAN_GROGGY_FOLLOWUP" == boss.strPatternId &&
					 "GROGGY" == boss.strPatternStageId &&
					 CBossCombatRuntime::Has_Flag(
						 boss.BossCombat,
						 SERVER_BOSS_COMBAT_FLAG::GROGGY));
				if (reachedSharedGroggy)
					break;
			}
			tests.Require(
				valid && guardStarted && remoteFrontRejected && behindRejected &&
				secondGuardStarted && frontAccepted && damageLess &&
				singleCounterConsumer && reachedSharedGroggy,
				"Triple Counter proxy rejects a remote front or in-range rear guard, accepts one in-range front guard without damage, consumes one player, and enters the shared Groggy follow-up");
		}
		{
			auto room = prepareRoom();
			SERVER_WORLD_ENTITY& boss = room->m_WorldEntities.front();
			boss.PendingPatternIds.clear();
			boss.PendingPatternIds.push_back("VALTAN_DASH_CHARGE");
			bool valid = true;
			bool guardStarted = false;
			bool hitPromotedGuard = false;
			bool damageWasAbsorbed = false;
			CPlayerSkillSystem counterSkills;
			for (std::uint32_t tick = 5000u; tick < 5300u && valid; ++tick)
			{
				if ("CHARGE" == boss.strPatternStageId && !guardStarted)
				{
					SERVER_PLAYER& counterPlayer = room->m_Players.at(19200u);
					SERVER_PLAYER& outsidePlayer = room->m_Players.at(19201u);
					const float radians = boss.fYawDegrees *
						3.14159265358979323846f / 180.f;
					counterPlayer.fPositionX = boss.fPositionX +
						std::sin(radians);
					counterPlayer.fPositionZ = boss.fPositionZ +
						std::cos(radians);
					outsidePlayer.fPositionX = boss.fPositionX -
						std::sin(radians) * 20.f;
					outsidePlayer.fPositionZ = boss.fPositionZ -
						std::cos(radians) * 20.f;
					C2S_USE_SKILL counterCommand{};
					counterCommand.iClientSequence = 1u;
					counterCommand.iSkillId = 34580u;
					counterCommand.fAimX = boss.fPositionX;
					counterCommand.fAimZ = boss.fPositionZ;
					guardStarted = counterSkills.Try_Start(
						counterPlayer, counterCommand, catalog, tick);
				}
				const std::uint32_t playerHpBefore =
					room->m_Players.at(19200u).iCurrentHp;
				valid = advance(*room, tick);
				if (guardStarted && !hitPromotedGuard &&
					2u == room->m_Players.at(19200u).iComboStage)
				{
					hitPromotedGuard = !boss.bPatternHasCounterProxy;
					damageWasAbsorbed = playerHpBefore ==
						room->m_Players.at(19200u).iCurrentHp &&
						room->m_TickDamageEvents.empty();
					break;
				}
			}
			tests.Require(
				valid && guardStarted && hitPromotedGuard && damageWasAbsorbed,
				"Dash Charge keeps the existing hit-driven Counter guard path: its real BOX hit promotes stage two and is absorbed without a counter proxy");
		}
		{
			auto room = prepareRoom();
			SERVER_WORLD_ENTITY& boss = room->m_WorldEntities.front();
			boss.iPatternSequence = 5u;
			boss.strPatternId = "VALTAN_TRASH";
			boss.strActionId = stageById("EXECUTE_TAIL").strActionId;
			boss.strPatternStageId = "EXECUTE_TAIL";
			boss.eAction = SERVER_ENTITY_ACTION::PATTERN_RECOVERY;
			for (auto& [id, player] : room->m_Players)
				(void)room->Capture_PlayerAttachment(player.iNetEntityId, boss.iNetEntityId,
					PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 99u);
			const auto all = CValtanBrain::Classify_GrabbedPlayers(boss, room->m_Players);
			SERVER_PLAYER saved = room->m_Players.at(19201u);
			auto& changed = room->m_Players.at(19201u);
			changed.Clear_Attachment(); changed.eAction = PLAYER_ACTION_STATE::NONE;
			changed.isCombatReady = false;
			const auto unreadyAlive = CValtanBrain::Classify_GrabbedPlayers(boss, room->m_Players);
			changed = saved; changed.iAttachmentPatternSequence = 4u;
			const auto stale = CValtanBrain::Classify_GrabbedPlayers(boss, room->m_Players);
			changed = saved; changed.iAttachmentOwnerNetEntityId += 1u;
			const auto wrongBoss = CValtanBrain::Classify_GrabbedPlayers(boss, room->m_Players);
			changed = saved; changed.eAttachmentSlot = PLAYER_ATTACHMENT_SLOT::NONE;
			const auto wrongSlot = CValtanBrain::Classify_GrabbedPlayers(boss, room->m_Players);
			changed = saved; changed.fAttachmentLocalOffsetX = std::numeric_limits<float>::quiet_NaN();
			const auto malformedOffset = CValtanBrain::Classify_GrabbedPlayers(boss, room->m_Players);
			changed = saved;
			std::map<PLAYER_ID, SERVER_PLAYER> staged;
			std::vector<DAMAGE_EVENT> stagedEvents;
			room->m_TickDamageEvents.resize(MAX_DAMAGE_EVENTS);
			const bool rejectedCapacity = !room->Prepare_GrabbedPlayerImpact(boss, catalog,
				stageById("EXECUTE_TAIL").Actions.front(), 100u, staged, stagedEvents);
			room->m_TickDamageEvents.clear();
			changed.fPositionX = std::numeric_limits<float>::quiet_NaN();
			const bool rejectedSecondTarget = !room->Commit_BossPatternPlayerStageActions(boss, catalog,
				"VALTAN_TRASH", boss.strActionId, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 100u);
			const auto& first = room->m_Players.at(19200u);
			const bool noPartial = first.iCurrentHp == first.iMaximumHp &&
				first.eAction == PLAYER_ACTION_STATE::GRABBED && first.iAttachmentPatternSequence == 5u &&
				room->m_TickDamageEvents.empty() && boss.iGrabExecutionCommittedPatternSequence == 0u;
			room->m_Players.clear();
			const auto empty = CValtanBrain::Classify_GrabbedPlayers(boss, room->m_Players);
			tests.Require(all.eClassification == SERVER_BOSS_GRAB_CLASSIFICATION::ALL &&
				unreadyAlive.eClassification == SERVER_BOSS_GRAB_CLASSIFICATION::PARTIAL &&
				unreadyAlive.iAliveCount == 2u && stale.bHasInvalidAttachment && wrongBoss.bHasInvalidAttachment &&
				wrongSlot.bHasInvalidAttachment && malformedOffset.bHasInvalidAttachment &&
				malformedOffset.eClassification != SERVER_BOSS_GRAB_CLASSIFICATION::ALL &&
				stale.eClassification != SERVER_BOSS_GRAB_CLASSIFICATION::ALL &&
				empty.eClassification == SERVER_BOSS_GRAB_CLASSIFICATION::NONE &&
				rejectedCapacity && rejectedSecondTarget && noPartial,
				"Trash classifier counts unready living participants, rejects stale ownership and empty ALL, and rolls back full-capacity or partial-target execution failure");
		}
		for (const bool captureBeforeBoundary : { false, true })
		{
			auto room = prepareRoom();
			auto& boss = room->m_WorldEntities.front();
			std::uint32_t tick = 1000u;
			bool valid = true;
			while (tick < 1700u && "STEP_08" != boss.strPatternStageId && valid)
				valid = advance(*room, tick++);
			float boundaryZ = boss.fSpawnPositionZ;
			bool foundBoundary = false;
			for (float distance = 0.f; distance < 64.f; distance += 0.25f)
			{
				const float z = boss.fSpawnPositionZ + distance;
				if (!room->m_ServerNavigation.Is_PointWalkableExact(boss.fSpawnPositionX, z))
				{
					foundBoundary = true;
					break;
				}
				boundaryZ = z;
			}
			boss.fPositionX = boss.fSpawnPositionX;
			boss.fPositionZ = boundaryZ;
			boss.fYawDegrees = 0.f;
			for (auto& [id, player] : room->m_Players)
			{
				player.fPositionX = boss.fPositionX;
				player.fPositionZ = boss.fPositionZ - 10.f;
			}
			if (captureBeforeBoundary)
				valid = valid && room->Capture_PlayerAttachment(
					room->m_Players.at(19200u).iNetEntityId, boss.iNetEntityId,
					PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, tick - 1u);
			room->m_iServerTick = tick - 1u;
			room->Update_WorldEntities(1.f / 30.f);
			tests.Require(valid && foundBoundary && room->Is_Ready() &&
				boss.strPatternStageId == (captureBeforeBoundary ? "CATCH_COUNTER" : "RUSH_MISS"),
				captureBeforeBoundary ?
					"Trash nav blockage resolves the committed capture before its miss branch on the same tick" :
					"Trash stops at missing front navigation and enters finite miss recovery immediately");
			SERVER_PLAYER ordinary = room->m_Players.at(19201u);
			ordinary.Clear_Attachment();
			ordinary.eAction = PLAYER_ACTION_STATE::NONE;
			ordinary.fPositionX = boss.fSpawnPositionX;
			ordinary.fPositionZ = boundaryZ;
			ordinary.fKnockbackDirectionX = 0.f;
			ordinary.fKnockbackDirectionZ = 1.f;
			ordinary.fKnockbackSpeed = 24.f;
			ordinary.fKnockbackRemainingSeconds = 1.f;
			room->Advance_PlayerKnockback(ordinary, 1.f / 30.f);
			tests.Require(ordinary.fKnockbackRemainingSeconds == 0.f &&
				room->m_ServerNavigation.Is_PointWalkableExact(
					ordinary.fPositionX, ordinary.fPositionZ) &&
				PLAYER_ACTION_STATE::FALLING != ordinary.eAction,
				"Ordinary knockback still stops at navigation while arena ejection is opt-in");
		}
		{
			auto room = prepareRoom();
			auto& boss = room->m_WorldEntities.front();
			boss.PendingPatternIds.clear();
			boss.strPatternId = "VALTAN_CATCH_BREATH";
			boss.iPatternSequence = 1u;
			boss.fYawDegrees = 0.f;
			const auto pattern = std::find_if(patterns->begin(), patterns->end(),
				[](const auto& row) { return "VALTAN_CATCH_BREATH" == row.strPatternId; });
			const auto& release = pattern->Stages.back().Actions.front();
			SERVER_PLAYER& player = room->m_Players.at(19200u);
			bool prepared = room->Capture_PlayerAttachment(player.iNetEntityId,
				boss.iNetEntityId, PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 10u);
			SERVER_PLAYER staged = player;
			prepared = prepared && room->Prepare_ArenaEjection(staged, boss, release, 11u);
			const float totalDistance = staged.fKnockbackSpeed * staged.fKnockbackRemainingSeconds;
			const float startZ = staged.fPositionZ;
			if (prepared)
				room->Advance_PlayerKnockback(staged, 0.1f);
			const bool speedExact = prepared &&
				std::fabs(staged.fPositionZ - startZ - 2.4f) < 0.001f &&
				std::fabs(staged.fKnockbackSpeed - 24.f) < 0.0001f;
			for (std::uint32_t step = 0u; staged.bArenaEjectionActive && step < 200u; ++step)
			{
				room->m_iServerTick = 12u + step;
				room->Advance_PlayerKnockback(staged, 1.f / 30.f);
			}
			tests.Require(speedExact && totalDistance >= 12.f && totalDistance <= 128.f &&
				!staged.bArenaEjectionActive && PLAYER_ACTION_STATE::FALLING == staged.eAction &&
				!room->m_ServerNavigation.Is_PointWalkableExact(staged.fPositionX, staged.fPositionZ),
				"The authored 180-degree arena ejection moves forward from boss facing, crosses navigation and enters finite FALLING");
			BOSS_PATTERN_STAGE_ACTION invalid = release;
			invalid.fReleaseSpeedMps = std::numeric_limits<float>::quiet_NaN();
			SERVER_PLAYER rejected = player;
			tests.Require(!room->Prepare_ArenaEjection(rejected, boss, invalid, 12u) &&
				PLAYER_ACTION_STATE::GRABBED == rejected.eAction &&
				rejected.iAttachmentOwnerNetEntityId == boss.iNetEntityId,
				"Invalid ejection preflight preserves the captured player without partial detach");
			BOSS_PATTERN_STAGE_ACTION leftRelease = release;
			leftRelease.fReleaseYawOffsetDegrees = 90.f;
			SERVER_PLAYER left = player;
			const bool leftPrepared = room->Prepare_ArenaEjection(
				left, boss, leftRelease, 13u);
			BOSS_PATTERN_STAGE_ACTION rightRelease = release;
			rightRelease.fReleaseYawOffsetDegrees = -90.f;
			SERVER_PLAYER right = player;
			const bool rightPrepared = room->Prepare_ArenaEjection(
				right, boss, rightRelease, 14u);
			tests.Require(leftPrepared && rightPrepared &&
				std::fabs(left.fKnockbackDirectionX + 1.f) < 0.001f &&
				std::fabs(left.fKnockbackDirectionZ) < 0.001f &&
				std::fabs(right.fKnockbackDirectionX - 1.f) < 0.001f &&
				std::fabs(right.fKnockbackDirectionZ) < 0.001f,
				"Apply grabbed-player release yaw relative to the Server-owned boss facing");
			BOSS_PATTERN_STAGE_ACTION invalidYaw = release;
			invalidYaw.fReleaseYawOffsetDegrees = 181.f;
			SERVER_PLAYER yawRejected = player;
			tests.Require(!room->Prepare_ArenaEjection(
				yawRejected, boss, invalidYaw, 15u) &&
				PLAYER_ACTION_STATE::GRABBED == yawRejected.eAction &&
				yawRejected.iAttachmentOwnerNetEntityId == boss.iNetEntityId,
				"Reject out-of-range release yaw without partial attachment mutation");
		}
		for (const bool shouldCaptureAll : { false, true })
		{
			auto room = prepareRoom();
			SERVER_WORLD_ENTITY& boss = room->m_WorldEntities.front();
			boss.PendingPatternIds = { "VALTAN_CATCH_BREATH" };
			bool valid = true;
			bool capturedAny = false;
			bool capturedAll = false;
			bool enteredHold = false;
			bool enteredRelease = false;
			std::uint32_t terminalTick = 0u;
			for (std::uint32_t tick = 4000u;
				tick < 4500u && valid; ++tick)
			{
				if ("STEP_02" == boss.strPatternStageId)
				{
					const float radians = boss.fYawDegrees *
						3.14159265358979323846f / 180.f;
					for (auto& [playerId, player] : room->m_Players)
					{
						(void)playerId;
						if (PLAYER_ACTION_STATE::GRABBED == player.eAction)
							continue;
						const float distance = shouldCaptureAll ? 2.f : 12.f;
						player.fPositionX = boss.fPositionX +
							std::sin(radians) * distance;
						player.fPositionZ = boss.fPositionZ +
							std::cos(radians) * distance;
					}
				}
				valid = advance(*room, tick);
				const SERVER_BOSS_GRAB_ROSTER roster =
					CValtanBrain::Classify_GrabbedPlayers(
						boss, room->m_Players);
				capturedAny = capturedAny || 0u != roster.iGrabbedCount;
				capturedAll = capturedAll ||
					roster.iAliveCount == roster.iGrabbedCount;
				enteredHold = enteredHold ||
					"STEP_03" == boss.strPatternStageId;
				enteredRelease = enteredRelease ||
					"STEP_04" == boss.strPatternStageId;
				if (0u != boss.PatternTerminalReceipt.iPatternSequence)
				{
					terminalTick = tick;
					break;
				}
			}
			const bool terminalClean = 0u != terminalTick &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
					boss.PatternTerminalReceipt.eResult &&
				boss.strPatternId.empty() &&
				SERVER_ENTITY_ACTION::IDLE == boss.eAction &&
				INVALID_NET_ENTITY_ID == boss.iTargetEntityId &&
				INVALID_NET_ENTITY_ID == boss.iPatternTargetEntityId &&
				!boss.bHasPatternTargetLastPosition && boss.MovePath.empty() &&
				0u == boss.iMovePathIndex &&
				!boss.bMechanicLedgerRequiresReset;
			if (!shouldCaptureAll)
			{
				boss.PendingPatternIds.push_back("VALTAN_COUNTER");
				const bool resumed = advance(*room, terminalTick + 1u) &&
					"VALTAN_COUNTER" == boss.strPatternId &&
					INVALID_NET_ENTITY_ID != boss.iTargetEntityId;
				tests.Require(
					valid && !capturedAny && !enteredHold && !enteredRelease &&
					terminalClean && resumed,
					"Catch-breath miss takes its explicit TIMEOUT terminal and reacquires a target for the next pattern");
			}
			else
			{
				const bool waitedIdleWithoutTarget =
					advance(*room, terminalTick + 1u) &&
					SERVER_ENTITY_ACTION::IDLE == boss.eAction &&
					boss.strPatternId.empty() &&
					!boss.bMechanicLedgerRequiresReset;
				SERVER_PLAYER& revived = room->m_Players.begin()->second;
				revived.iCurrentHp = revived.iMaximumHp;
				revived.eAction = PLAYER_ACTION_STATE::NONE;
				revived.isCombatReady = true;
				revived.bArenaEjectionActive = false;
				revived.iEjectionOwnerNetEntityId = INVALID_NET_ENTITY_ID;
				revived.fKnockbackSpeed = 0.f;
				revived.fKnockbackRemainingSeconds = 0.f;
				revived.fPositionX = boss.fPositionX;
				revived.fPositionY = boss.fPositionY;
				revived.fPositionZ = boss.fPositionZ + 1.f;
				boss.PendingPatternIds.push_back("VALTAN_COUNTER");
				const bool resumedAfterTarget =
					advance(*room, terminalTick + 2u) &&
					"VALTAN_COUNTER" == boss.strPatternId &&
					INVALID_NET_ENTITY_ID != boss.iTargetEntityId &&
					!boss.bMechanicLedgerRequiresReset;
				tests.Require(
					valid && capturedAny && capturedAll && enteredHold &&
					enteredRelease && terminalClean &&
					waitedIdleWithoutTarget && resumedAfterTarget,
					"Catch-breath may capture and eject the whole party, finish its release stage, wait IDLE without mechanic reset, and resume after a valid target returns");
			}
		}
		{
			auto room = prepareRoom();
			auto& boss = room->m_WorldEntities.front();
			boss.PendingPatternIds = { "VALTAN_WARP" };
			std::set<std::uint32_t> portalStages;
			std::map<std::uint32_t, std::pair<float, float>> portalStageStarts;
			std::map<std::uint32_t, float> portalStageEntryYaws;
			std::map<std::uint32_t, std::pair<float, float>> portalStageEnds;
			std::map<std::uint32_t, float> portalStageTravelMeters;
			std::map<std::pair<std::uint32_t, NET_ENTITY_ID>, std::uint32_t> hitCounts;
			std::set<std::uint32_t> stageStartLockedPortalStages;
			bool targetRushExact = true;
			bool routeImmutableDuringDelay = true;
			bool noMotionOrHitBeforeRetarget = true;
			bool noTeleport = true;
			bool crossedMissingNavigation = false;
			bool completed = false;
			for (std::uint32_t tick = 1u; tick < 700u && room->Is_Ready(); ++tick)
			{
				const float yaw = boss.fYawDegrees * 3.14159265358979323846f / 180.f;
				SERVER_PLAYER& target = room->m_Players.at(19200u);
				target.iCurrentHp = target.iMaximumHp;
				target.isCombatReady = true;
				target.eAction = PLAYER_ACTION_STATE::NONE;
				target.fPositionX = boss.fSpawnPositionX;
				target.fPositionZ = boss.fSpawnPositionZ + 12.f;
				const bool retargetRight = 0u == boss.iPatternStageIndex % 2u;
				if (boss.bPortalMotionActive &&
					boss.bPortalRushTargetLocked &&
					boss.fActionElapsedSeconds <
						static_cast<float>(boss.iPortalRushRetargetDelayMs) / 1000.f)
				{
					/* Move the selected target after the Stage edge. Both portals
					   already snapshot one route at t=0, so the authored body-hidden
					   wait must not rotate or move that committed endpoint. */
					const auto lockedTarget = std::find_if(
						room->m_Players.begin(), room->m_Players.end(),
						[&boss](const auto& entry)
						{
							return entry.second.iNetEntityId ==
								boss.iPatternTargetEntityId;
						});
					if (room->m_Players.end() != lockedTarget)
					{
						lockedTarget->second.fPositionX = boss.fPositionX +
							(retargetRight ? 12.f : -12.f);
						lockedTarget->second.fPositionZ = boss.fPositionZ;
					}
				}
				SERVER_PLAYER& swept = room->m_Players.at(19201u);
				swept.iCurrentHp = swept.iMaximumHp;
				swept.eAction = PLAYER_ACTION_STATE::NONE;
				swept.fKnockbackSpeed = 0.f;
				swept.fKnockbackRemainingSeconds = 0.f;
				swept.iKnockdownEndTick = 0u;
				swept.iHitReactionGraceEndTick = 0u;
				swept.isCombatReady = boss.bPortalMotionActive &&
					1u == boss.iAppliedPatternHitCount &&
					boss.PortalStageHitTargets.end() == std::find(
						boss.PortalStageHitTargets.begin(),
						boss.PortalStageHitTargets.end(), swept.iNetEntityId);
				if (swept.isCombatReady)
				{
					/* Behind the current BOX, but within the circle-expanded start of
					   the segment swept since the first travel pulse. */
					swept.fPositionX = boss.fPortalLastHitSampleX - std::sin(yaw) * 0.2f;
					swept.fPositionZ = boss.fPortalLastHitSampleZ - std::cos(yaw) * 0.2f;
				}
				const float beforeX = boss.fPositionX;
				const float beforeZ = boss.fPositionZ;
				const bool beforePortalMotion = boss.bPortalMotionActive;
				const std::uint32_t beforePortalStage = boss.iPatternStageIndex;
				const float beforePortalEndX = boss.fPortalEndX;
				const float beforePortalEndZ = boss.fPortalEndZ;
				const bool beforePortalRushTargetLocked =
					boss.bPortalRushTargetLocked;
				room->m_iServerTick = tick - 1u;
				room->m_TickDamageEvents.clear();
				room->Update_WorldEntities(1.f / 30.f);
				if (beforePortalMotion &&
					(!boss.bPortalMotionActive ||
					 beforePortalStage != boss.iPatternStageIndex))
				{
					const auto start = portalStageStarts.find(beforePortalStage);
					if (portalStageStarts.end() != start)
					{
						const float completedX = boss.bPortalMotionActive ?
							boss.fPositionX : beforePortalEndX;
						const float completedZ = boss.bPortalMotionActive ?
							boss.fPositionZ : beforePortalEndZ;
						portalStageTravelMeters[beforePortalStage] = (std::max)(
							portalStageTravelMeters[beforePortalStage],
							std::hypot(completedX - start->second.first,
								completedZ - start->second.second));
					}
				}
				if (boss.bPortalMotionActive)
				{
					if (portalStages.insert(boss.iPatternStageIndex).second)
					{
						if (boss.bPortalRushTargetLocked)
							stageStartLockedPortalStages.insert(
								boss.iPatternStageIndex);
						portalStageEntryYaws.emplace(
							boss.iPatternStageIndex, boss.fYawDegrees);
						portalStageStarts.emplace(boss.iPatternStageIndex,
							std::make_pair(boss.fPortalStartX, boss.fPortalStartZ));
						portalStageEnds.emplace(boss.iPatternStageIndex,
							std::make_pair(boss.fPortalEndX, boss.fPortalEndZ));
						const float targetDeltaX =
							boss.fPatternTargetLastPositionX - boss.fPortalStartX;
						const float targetDeltaZ =
							boss.fPatternTargetLastPositionZ - boss.fPortalStartZ;
						const float expectedYaw =
							std::atan2(targetDeltaX, targetDeltaZ) *
								180.f / 3.14159265358979323846f;
						const float expectedYawRadians = expectedYaw *
							3.14159265358979323846f / 180.f;
						const std::uint32_t expectedPortalDelayMs =
							"valtan.sequence.warp.step-02" == boss.strActionId ?
							300u : 600u;
						const std::uint32_t expectedPortalDurationMs =
							"valtan.sequence.warp.step-02" == boss.strActionId ?
							1600u : 1900u;
						targetRushExact = targetRushExact &&
							BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH ==
								boss.ePatternStageMotionKind &&
							expectedPortalDelayMs == boss.iPortalRushRetargetDelayMs &&
							expectedPortalDurationMs == boss.iPatternStageDurationMs &&
							std::fabs(boss.fPortalRushSpeedMps - 12.3076925f) < 0.0001f &&
							std::fabs(boss.fPortalRushDistanceM - 16.f) < 0.0001f &&
							26u == boss.iPatternHitCount &&
							boss.PatternStageRootMotion.empty() &&
							boss.bPortalRushTargetLocked &&
							boss.bHasPatternTargetLastPosition &&
							std::fabs(std::remainder(
								boss.fYawDegrees - expectedYaw, 360.f)) < 0.001f &&
							std::fabs(boss.fPortalEndX -
								(boss.fPortalStartX +
								 std::sin(expectedYawRadians) * 16.f)) < 0.001f &&
							std::fabs(boss.fPortalEndZ -
								(boss.fPortalStartZ +
								 std::cos(expectedYawRadians) * 16.f)) < 0.001f &&
							std::hypot(boss.fPositionX - boss.fPortalStartX,
								boss.fPositionZ - boss.fPortalStartZ) < 0.001f;
					}
					const auto entryYaw =
						portalStageEntryYaws.find(boss.iPatternStageIndex);
					const auto committedEnd =
						portalStageEnds.find(boss.iPatternStageIndex);
					if (portalStageEntryYaws.end() != entryYaw &&
						portalStageEnds.end() != committedEnd)
					{
						routeImmutableDuringDelay =
							routeImmutableDuringDelay &&
							boss.bPortalRushTargetLocked &&
							std::fabs(std::remainder(
								boss.fYawDegrees - entryYaw->second, 360.f)) < 0.001f &&
							std::fabs(boss.fPortalEndX -
								committedEnd->second.first) < 0.001f &&
							std::fabs(boss.fPortalEndZ -
								committedEnd->second.second) < 0.001f;
						if (beforePortalMotion &&
							beforePortalStage == boss.iPatternStageIndex &&
							beforePortalRushTargetLocked)
						{
							routeImmutableDuringDelay =
								routeImmutableDuringDelay &&
								std::fabs(boss.fPortalEndX - beforePortalEndX) < 0.001f &&
								std::fabs(boss.fPortalEndZ - beforePortalEndZ) < 0.001f;
						}
					}
					const auto start = portalStageStarts.find(boss.iPatternStageIndex);
					if (portalStageStarts.end() != start)
					{
						const float travel = std::hypot(
							boss.fPositionX - start->second.first,
							boss.fPositionZ - start->second.second);
						portalStageTravelMeters[boss.iPatternStageIndex] = (std::max)(
							portalStageTravelMeters[boss.iPatternStageIndex], travel);
						if (boss.fActionElapsedSeconds <
							static_cast<float>(boss.iPortalRushRetargetDelayMs) / 1000.f)
						{
							noMotionOrHitBeforeRetarget =
								noMotionOrHitBeforeRetarget && travel < 0.001f &&
								0u == boss.iAppliedPatternHitCount &&
								room->m_TickDamageEvents.empty();
						}
					}
					crossedMissingNavigation = crossedMissingNavigation ||
						!room->m_ServerNavigation.Is_PointWalkableExact(boss.fPositionX, boss.fPositionZ);
					for (const auto& damage : room->m_TickDamageEvents)
						++hitCounts[{ boss.iPatternStageIndex, damage.iTargetNetEntityId }];
				}
				if (boss.bPortalMotionActive)
				{
					noTeleport = noTeleport &&
						std::hypot(boss.fPositionX - beforeX,
							boss.fPositionZ - beforeZ) < 2.f;
				}
				if (boss.strPatternId.empty() &&
					SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == boss.PatternTerminalReceipt.eResult)
				{
					completed = true;
					break;
				}
			}
			const bool noRepeatHits = std::all_of(hitCounts.begin(), hitCounts.end(),
				[](const auto& entry) { return entry.second <= 1u; });
			const std::size_t sweptBodyHits = std::count_if(hitCounts.begin(), hitCounts.end(),
				[](const auto& entry) { return entry.first.second == 19301u; });
			const bool exactLegDistances = portalStageTravelMeters.size() == 8u &&
				std::all_of(portalStageTravelMeters.begin(), portalStageTravelMeters.end(),
					[](const auto& entry)
					{
						return std::fabs(entry.second - 16.f) < 0.01f;
					});
			std::cout << "[DIAGNOSTIC] Portal legs=" << portalStages.size()
				<< " sweptBodyHitLegs=" << sweptBodyHits
				<< " totalHitPairs=" << hitCounts.size() << " noRepeat=" << noRepeatHits
				<< " exactDistances=" << exactLegDistances << '\n';
			SERVER_NAV_POINT center{};
			const bool centerResolved = room->Resolve_ArenaCenter(boss, center);
			const bool returnedToCenter = centerResolved &&
				std::hypot(boss.fPositionX - center.x,
					boss.fPositionZ - center.z) < 0.001f;
			std::cout << "[DIAGNOSTIC] Portal completed=" << completed
				<< " ready=" << room->Is_Ready()
				<< " pattern=" << boss.strPatternId
				<< " stage=" << boss.iPatternStageIndex
				<< " stageId=" << boss.strPatternStageId
				<< " elapsed=" << boss.fActionElapsedSeconds
				<< " startTick=" << boss.iActionStartTick
				<< " paused=" << boss.bAutomaticPatternSequencePausedForRevive
				<< " typed=" << targetRushExact
				<< " stageStartLocked=" << stageStartLockedPortalStages.size()
				<< " routeImmutable=" << routeImmutableDuringDelay
				<< " noWaitMotionOrHit=" << noMotionOrHitBeforeRetarget
				<< " noTeleport=" << noTeleport
				<< " crossedMissingNav=" << crossedMissingNavigation
				<< " returnedCenter=" << returnedToCenter << '\n';
			tests.Require(completed && room->Is_Ready() && portalStages.size() == 8u &&
				targetRushExact && stageStartLockedPortalStages.size() == 8u &&
				routeImmutableDuringDelay &&
				exactLegDistances &&
				noMotionOrHitBeforeRetarget && noTeleport && crossedMissingNavigation &&
				returnedToCenter,
				"Portal locks eight t=0 start/end routes, waits 300 ms on the first leg and 600 ms thereafter, travels exact 16 m in 1.3 s without root-motion double-add, then returns to valid arena center");
			tests.Require(sweptBodyHits == 8u && noRepeatHits,
				"Portal 50 ms swept contact catches an inter-pulse body at most once per traversal leg");
		}
		{
			BOSS_PATTERN_STAGE_DEFINITION zeroDelayStage{};
			zeroDelayStage.Motion.eKind =
				BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH;
			zeroDelayStage.Motion.iRetargetDelayMs = 0u;
			zeroDelayStage.Motion.fSpeedMps = 20.f;
			zeroDelayStage.Motion.fDistance = 8.f;
			SERVER_WORLD_ENTITY zeroDelayBoss{};
			zeroDelayBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			zeroDelayBoss.eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
			zeroDelayBoss.iCurrentHp = 1u;
			zeroDelayBoss.fPositionX = 10.f;
			zeroDelayBoss.fPositionZ = 20.f;
			zeroDelayBoss.fYawDegrees = 0.f;
			zeroDelayBoss.bHasPatternTargetLastPosition = true;
			zeroDelayBoss.fPatternTargetLastPositionX = 22.f;
			zeroDelayBoss.fPatternTargetLastPositionZ = 20.f;
			zeroDelayBoss.ePatternStageMotionKind =
				BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH;
			CValtanBrain::Configure_PortalMotion(zeroDelayBoss, zeroDelayStage);
			const bool zeroDelayLocked =
				CValtanBrain::Lock_PortalTargetRushAtStageStart(zeroDelayBoss);
			tests.Require(
				zeroDelayLocked && zeroDelayBoss.bPortalRushTargetLocked &&
				std::fabs(zeroDelayBoss.fYawDegrees - 90.f) < 0.001f &&
				std::fabs(zeroDelayBoss.fPortalEndX - 18.f) < 0.001f &&
				std::fabs(zeroDelayBoss.fPortalEndZ - 20.f) < 0.001f,
				"Portal zero-delay tuning locks its selected target on the ENTER boundary without one fixed-tick latency");
			zeroDelayBoss.strPatternId = "VALTAN_WARP";
			zeroDelayBoss.strPatternStageId = "STEP_02";
			zeroDelayBoss.strActionId = "valtan.sequence.warp.step-02";
			zeroDelayBoss.iCurrentHp = 0u;
			std::map<PLAYER_ID, SERVER_PLAYER> noPlayers;
			std::vector<DAMAGE_EVENT> noDamage;
			CServerNavigation deathNavigation;
			CValtanBrain deathBrain;
			deathBrain.Update(zeroDelayBoss, noPlayers, catalog, deathNavigation,
				1.f / 30.f, 400u, {}, noDamage);
			tests.Require(
				!zeroDelayBoss.bPortalMotionActive &&
				!zeroDelayBoss.bPortalRushTargetLocked &&
				BOSS_PATTERN_STAGE_MOTION_KIND::NONE ==
					zeroDelayBoss.ePatternStageMotionKind &&
				zeroDelayBoss.PortalStageHitTargets.empty(),
				"Boss death clears every pending Portal rush motion and hit cursor");
		}
		{
			auto room = prepareRoom();
			SERVER_WORLD_ENTITY& primaryBoss = room->m_WorldEntities.front();
			primaryBoss.strPlacementId = "boss.valtan.center";
			primaryBoss.fCollisionRadius = catalog.Find_Boss("BOSS_VALTAN")->fCollisionRadius;
			primaryBoss.bScriptedPatternPlayback = false;
			primaryBoss.PendingPatternIds = { "VALTAN_GHOST_RESPAWN_AUDITION" };
			for (auto& [playerId, player] : room->m_Players)
			{
				(void)playerId;
				player.iCurrentHp = player.iMaximumHp = 1000000000u;
			}
			const std::vector<std::string> expectedPrimaryLoop{
				"VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH",
				"VALTAN_SEQUENCE_FOUR", "VALTAN_CROSS",
				"VALTAN_CHARGE", "VALTAN_CHARGE_2" };
			std::vector<std::string> expectedTwoPrimaryCycles = expectedPrimaryLoop;
			expectedTwoPrimaryCycles.insert(expectedTwoPrimaryCycles.end(),
				expectedPrimaryLoop.begin(), expectedPrimaryLoop.end());
			std::vector<std::string> observedPrimaryLoop;
			std::vector<std::uint32_t> portalSpawnTicks;
			std::vector<std::uint32_t> auxiliarySpawnTicks;
			std::map<NET_ENTITY_ID, std::string> auxiliarySkills;
			std::uint32_t observedPortalSequence = 0u;
			std::uint32_t observedAuxiliarySequence = 0u;
			std::uint32_t auxiliaryDespawnTick = 0u;
			NET_ENTITY_ID previousAuxiliaryId = INVALID_NET_ENTITY_ID;
			bool activated = false;
			bool primaryIdentityStable = true;
			bool primaryNeverAutoRelocated = true;
			bool auxiliaryContractExact = true;
			bool portalRunnerContractExact = true;
			bool auxiliaryReplacementCadenceExact = true;
			bool observedAuxiliaryDespawn = false;
			bool visibleDamageAdmitted = false;
			for (std::uint32_t tick = 1u; tick <= 6000u && room->Is_Ready(); ++tick)
			{
				room->m_iServerTick = tick - 1u;
				room->m_TickDamageEvents.clear();
				room->Update_WorldEntities(1.f / 30.f);
				const auto primary = std::find_if(
					room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
					[](const SERVER_WORLD_ENTITY& entity)
					{ return 19100u == entity.iNetEntityId; });
				if (primary == room->m_WorldEntities.end())
				{
					primaryIdentityStable = false;
					break;
				}
				const std::size_t primaryCount = std::count_if(
					room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
					[](const SERVER_WORLD_ENTITY& entity)
					{ return INVALID_NET_ENTITY_ID == entity.iOwnerBossNetEntityId; });
				const std::size_t auxiliaryCount = std::count_if(
					room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
					[](const SERVER_WORLD_ENTITY& entity)
					{
						return SERVER_DEPENDENT_BOSS_ROLE::AUXILIARY ==
							entity.eDependentBossRole;
					});
				const std::size_t portalRunnerCount = std::count_if(
					room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
					[](const SERVER_WORLD_ENTITY& entity)
					{
						return SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER ==
							entity.eDependentBossRole;
					});
				primaryIdentityStable = primaryIdentityStable &&
					1u == primaryCount && auxiliaryCount <= 1u &&
					(0u == portalRunnerCount || 3u == portalRunnerCount) &&
					INVALID_NET_ENTITY_ID == primary->iOwnerBossNetEntityId &&
					"BOSS_VALTAN" == primary->strArchetypeId &&
					"boss.valtan.center" == primary->strPlacementId;
				activated = activated || primary->bGhostPhasePatternLoopActive;
				if (!primary->bGhostPhasePatternLoopActive)
					continue;

				primaryNeverAutoRelocated = primaryNeverAutoRelocated &&
					!primary->bGhostRepositionPending &&
					!primary->bGhostRelocationRetryPending &&
					!primary->bGhostRelocationOwnsHiddenFlag &&
					!primary->bGhostRelocationOwnsInvulnerableFlag &&
					0u == primary->iGhostRelocationSequence &&
					!CBossCombatRuntime::Has_Flag(
						primary->BossCombat,
						SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN);

				if (observedPrimaryLoop.size() < expectedTwoPrimaryCycles.size() &&
					!primary->strPatternId.empty() &&
					std::find(expectedPrimaryLoop.begin(), expectedPrimaryLoop.end(),
						primary->strPatternId) != expectedPrimaryLoop.end() &&
					(observedPrimaryLoop.empty() ||
					 observedPrimaryLoop.back() != primary->strPatternId))
				{
					observedPrimaryLoop.push_back(primary->strPatternId);
				}

				if (primary->iGhostPortalOccurrenceSequence != observedPortalSequence)
				{
					const std::uint32_t expectedSequence =
						0u == observedPortalSequence ? 1u : observedPortalSequence + 1u;
					primaryIdentityStable = primaryIdentityStable &&
						primary->iGhostPortalOccurrenceSequence == expectedSequence &&
						3u == portalRunnerCount;
					observedPortalSequence = primary->iGhostPortalOccurrenceSequence;
					portalSpawnTicks.push_back(tick);
				}
				for (const SERVER_WORLD_ENTITY& runner : room->m_WorldEntities)
				{
					if (SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER !=
						runner.eDependentBossRole)
					{
						continue;
					}
					portalRunnerContractExact = portalRunnerContractExact &&
						19100u == runner.iOwnerBossNetEntityId &&
						"BOSS_VALTAN_GHOST" == runner.strArchetypeId &&
						"VALTAN_GHOST_PORTAL_ONCE" == runner.strPatternId &&
						"ACTIVE" == runner.strPatternStageId &&
						"valtan.ghost.portal-once.active" == runner.strActionId &&
						SERVER_ENTITY_ACTION::PATTERN_ACTIVE == runner.eAction &&
						runner.bPortalMotionActive &&
						runner.bPortalRushTargetLocked &&
						BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH ==
							runner.ePatternStageMotionKind &&
						std::fabs(runner.fPortalRushDistanceM - 15.5884572681f) <
							0.001f &&
						std::fabs(runner.fPortalRushSpeedMps - 11.9911209755f) <
							0.001f &&
						runner.DependentPatternSequence.PatternIds.empty();
				}

				const auto auxiliary = std::find_if(
					room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
					[](const SERVER_WORLD_ENTITY& entity)
					{
						return SERVER_DEPENDENT_BOSS_ROLE::AUXILIARY ==
							entity.eDependentBossRole;
					});
				const bool hasAuxiliary = auxiliary != room->m_WorldEntities.end();
				if (hasAuxiliary)
				{
					SERVER_NAV_POINT auxiliaryGround{};
					const bool selectedOneUsableSkill =
						1u == auxiliary->DependentPatternSequence.iExpectedStepCount &&
						1u == auxiliary->DependentPatternSequence.PatternIds.size() &&
						std::find(expectedPrimaryLoop.begin(), expectedPrimaryLoop.end(),
							auxiliary->DependentPatternSequence.PatternIds.front()) !=
							expectedPrimaryLoop.end();
					auxiliaryContractExact = auxiliaryContractExact &&
						SERVER_DEPENDENT_BOSS_ROLE::AUXILIARY ==
							auxiliary->eDependentBossRole &&
						19100u == auxiliary->iOwnerBossNetEntityId &&
						"BOSS_VALTAN_GHOST" == auxiliary->strArchetypeId &&
						selectedOneUsableSkill &&
						auxiliary->iRotationStepIndex <= 1u &&
						room->m_ServerNavigation.Is_PointWalkableExact(
							auxiliary->fPositionX, auxiliary->fPositionZ) &&
						room->m_ServerNavigation.Sample_Position(
							auxiliary->fPositionX, auxiliary->fPositionZ,
							auxiliaryGround) &&
						std::fabs(auxiliaryGround.y - auxiliary->fPositionY) <= 1.5f;
					if (selectedOneUsableSkill)
					{
						const std::string& selectedSkill =
							auxiliary->DependentPatternSequence.PatternIds.front();
						const auto [entry, inserted] = auxiliarySkills.emplace(
							auxiliary->iNetEntityId, selectedSkill);
						auxiliaryContractExact = auxiliaryContractExact &&
							(inserted || entry->second == selectedSkill);
					}
					if (INVALID_NET_ENTITY_ID == previousAuxiliaryId)
					{
						if (0u != auxiliaryDespawnTick)
						{
							auxiliaryReplacementCadenceExact =
								auxiliaryReplacementCadenceExact &&
								tick == auxiliaryDespawnTick + 1u;
							auxiliaryDespawnTick = 0u;
						}
					}
					else
					{
						auxiliaryContractExact = auxiliaryContractExact &&
							previousAuxiliaryId == auxiliary->iNetEntityId;
					}
					previousAuxiliaryId = auxiliary->iNetEntityId;
				}
				else if (INVALID_NET_ENTITY_ID != previousAuxiliaryId)
				{
					observedAuxiliaryDespawn = true;
					auxiliaryDespawnTick = tick;
					auxiliaryReplacementCadenceExact =
						auxiliaryReplacementCadenceExact &&
						primary->iGhostAuxiliaryNextSpawnTick == tick + 1u;
					previousAuxiliaryId = INVALID_NET_ENTITY_ID;
				}

				if (primary->iGhostAuxiliaryOccurrenceSequence !=
					observedAuxiliarySequence)
				{
					const std::uint32_t expectedSequence =
						0u == observedAuxiliarySequence ?
						1u : observedAuxiliarySequence + 1u;
					auxiliaryContractExact = auxiliaryContractExact &&
						hasAuxiliary &&
						primary->iGhostAuxiliaryOccurrenceSequence ==
							expectedSequence;
					observedAuxiliarySequence =
						primary->iGhostAuxiliaryOccurrenceSequence;
					auxiliarySpawnTicks.push_back(tick);
				}

				if (!visibleDamageAdmitted && !primary->strPatternId.empty())
				{
					BOSS_INCOMING_HIT visibleHit{};
					visibleHit.iRawDamage = 1u;
					visibleHit.iServerTick = tick;
					const std::uint32_t beforeHp = primary->iCurrentHp;
					const BOSS_HIT_RESULT landed =
						CBossCombatRuntime::Apply_PlayerHit(*primary, visibleHit);
					visibleDamageAdmitted = 1u == landed.iHealthDamage &&
						primary->iCurrentHp + 1u == beforeHp;
				}

				if (observedPrimaryLoop == expectedTwoPrimaryCycles &&
					observedAuxiliarySequence >= 3u &&
					observedAuxiliaryDespawn && 0u == auxiliaryDespawnTick &&
					portalSpawnTicks.size() >= 2u)
				{
					break;
				}
			}
			bool portalCadenceExact = portalSpawnTicks.size() >= 2u;
			for (std::size_t index = 1u;
				portalCadenceExact && index < portalSpawnTicks.size(); ++index)
			{
				portalCadenceExact =
					237u == portalSpawnTicks[index] - portalSpawnTicks[index - 1u];
			}
			const auto primary = std::find_if(
				room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
				[](const auto& entity) { return entity.iNetEntityId == 19100u; });
			const bool primaryLoopPassed =
				room->Is_Ready() && activated && primaryIdentityStable &&
				primary != room->m_WorldEntities.end() &&
				observedPrimaryLoop == expectedTwoPrimaryCycles &&
				primaryNeverAutoRelocated && portalCadenceExact &&
				portalRunnerContractExact;
			tests.Require(
				primaryLoopPassed,
				"Phase-three respawn keeps one primary Valtan identity visible, never enters the random-relocation lane, and continuously repeats the exact six-pattern loop");
			tests.Require(
				visibleDamageAdmitted,
				"The persistent phase-three primary remains a real health-damage target while its foreground loop is active");
			tests.Require(
				auxiliaryContractExact && observedAuxiliarySequence >= 3u &&
				auxiliarySkills.size() >= 3u && observedAuxiliaryDespawn &&
				auxiliaryReplacementCadenceExact,
				"The auxiliary lane repeatedly spawns one ghost on exact walkable navigation, assigns exactly one usable finale skill, despawns it at completion, and replaces it on the next fixed tick");
			if (primary != room->m_WorldEntities.end()) primary->iCurrentHp = 0u;
			room->m_iServerTick = 6001u;
			room->Update_WorldEntities(1.f / 30.f);
			tests.Require(room->m_WorldEntities.empty() &&
				room->m_CombatObjectRuntime.Get_LiveObjects().empty(),
				"Phase-three primary death immediately removes the authoritative boss and every owned portal object");
		}
		{
			auto room = prepareRoom();
			SERVER_WORLD_ENTITY& primary = room->m_WorldEntities.front();
			primary.strPlacementId = "boss.valtan.center";
			primary.iPhase = 3u;
			primary.strPatternId.clear();
			primary.eAction = SERVER_ENTITY_ACTION::IDLE;
			primary.iCurrentHp = (std::max)(1u, primary.iCurrentHp);
			const float validX = primary.fPositionX;
			const float validY = primary.fPositionY;
			const float validZ = primary.fPositionZ;
			const float validSpawnX = primary.fSpawnPositionX;
			const float validSpawnY = primary.fSpawnPositionY;
			const float validSpawnZ = primary.fSpawnPositionZ;
			const bool activated = room->Activate_ValtanGhostPhaseLoop(
				primary, catalog);
			/* The Product primary loop never calls this helper. Keep its explicit
			   debug/recovery lifecycle isolated from a foreground attack here. */
			primary.bAutomaticPatternSequenceAuditionHold = true;
			primary.fSpawnPositionX = 100000.f;
			primary.fSpawnPositionY = validY;
			primary.fSpawnPositionZ = 100000.f;
			const bool deferred = activated &&
				room->Begin_ValtanGhostRelocation(primary, catalog, 100u);
			const bool preservedValidPose = deferred && room->Is_Ready() &&
				primary.bGhostRelocationRetryPending &&
				101u == primary.iGhostRelocationRetryTick &&
				!primary.bGhostRepositionPending &&
				0u == primary.iGhostRelocationSequence &&
				std::fabs(primary.fPositionX - validX) < 0.001f &&
				std::fabs(primary.fPositionY - validY) < 0.001f &&
				std::fabs(primary.fPositionZ - validZ) < 0.001f &&
				!CBossCombatRuntime::Has_Flag(
					primary.BossCombat,
					SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN) &&
				!primary.bGhostRelocationOwnsHiddenFlag &&
				!primary.bGhostRelocationOwnsInvulnerableFlag;
			primary.fSpawnPositionX = validSpawnX;
			primary.fSpawnPositionY = validSpawnY;
			primary.fSpawnPositionZ = validSpawnZ;
			const bool retried = preservedValidPose &&
				room->Begin_ValtanGhostRelocation(primary, catalog, 101u) &&
				primary.bGhostRepositionPending &&
				!primary.bGhostRelocationRetryPending &&
				1u == primary.iGhostRelocationSequence &&
				primary.bGhostRelocationOwnsHiddenFlag &&
				primary.bGhostRelocationOwnsInvulnerableFlag;
			room->m_iServerTick = 101u;
			room->Update_WorldEntities(1.f / 30.f);
			const auto live = std::find_if(
				room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
				[](const SERVER_WORLD_ENTITY& entity)
				{ return 19100u == entity.iNetEntityId; });
			tests.Require(
				retried && room->Is_Ready() && live != room->m_WorldEntities.end() &&
				!live->bGhostRepositionPending &&
				!live->bGhostRelocationRetryPending &&
				!live->bGhostRelocationOwnsHiddenFlag &&
				!live->bGhostRelocationOwnsInvulnerableFlag &&
				!CBossCombatRuntime::Has_Flag(
					live->BossCombat,
					SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN) &&
				!CBossCombatRuntime::Has_Flag(
					live->BossCombat,
					SERVER_BOSS_COMBAT_FLAG::INVULNERABLE),
				"The explicit relocation helper keeps a valid pose after 128 misses, retries deterministically, and closes its one-tick owned flags");
		}
		{
			auto room = prepareRoom();
			SERVER_WORLD_ENTITY& primary = room->m_WorldEntities.front();
			primary.bGhostPhasePatternLoopActive = true;
			primary.iPhase = 2u;
			primary.bGhostRepositionPending = true;
			primary.iGhostReappearTick = 200u;
			primary.bGhostRelocationOwnsHiddenFlag = true;
			primary.bGhostRelocationOwnsInvulnerableFlag = false;
			primary.bAutomaticPatternSequenceAuditionHold = true;
			(void)CBossCombatRuntime::Set_Flag(
				primary.BossCombat,
				SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN, true);
			(void)CBossCombatRuntime::Set_Flag(
				primary.BossCombat,
				SERVER_BOSS_COMBAT_FLAG::INVULNERABLE, true);
			room->m_iServerTick = 150u;
			room->Update_WorldEntities(1.f / 30.f);
			const auto live = std::find_if(
				room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
				[](const SERVER_WORLD_ENTITY& entity)
				{ return 19100u == entity.iNetEntityId; });
			tests.Require(
				room->Is_Ready() && live != room->m_WorldEntities.end() &&
				!live->bGhostRepositionPending &&
				!live->bGhostRelocationOwnsHiddenFlag &&
				!CBossCombatRuntime::Has_Flag(
					live->BossCombat,
					SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN) &&
				CBossCombatRuntime::Has_Flag(
					live->BossCombat,
					SERVER_BOSS_COMBAT_FLAG::INVULNERABLE),
				"Ghost relocation cancellation clears only the hidden flag it owned and preserves foreign invulnerability");
		}
		{
			auto room = prepareRoom();
			auto& owner = room->m_WorldEntities.front();
			owner.strPatternId = "VALTAN_GHOST_FINALE";
			owner.iPatternSequence = 1u;
			owner.fSpawnPositionX = owner.fSpawnPositionZ = 100000.f;
			const NET_ENTITY_ID nextId = room->m_iNextNetEntityId;
			const bool deferred = room->Update_DependentBosses(2u) &&
				room->Is_Ready() && room->m_iNextNetEntityId == nextId &&
				room->m_WorldEntities.size() == 1u &&
				0u == owner.iGhostAuxiliaryOccurrenceSequence &&
				3u == owner.iGhostAuxiliaryNextSpawnTick &&
				room->m_strStatus.find("no live same-deck footprint") != std::string::npos;
			const bool heldBeforeDue = room->Update_DependentBosses(2u) &&
				room->Is_Ready() && room->m_iNextNetEntityId == nextId &&
				room->m_WorldEntities.size() == 1u &&
				0u == owner.iGhostAuxiliaryOccurrenceSequence &&
				3u == owner.iGhostAuxiliaryNextSpawnTick;
			const bool retriedOnNextTick = room->Update_DependentBosses(3u) &&
				room->Is_Ready() && room->m_iNextNetEntityId == nextId &&
				room->m_WorldEntities.size() == 1u &&
				0u == owner.iGhostAuxiliaryOccurrenceSequence &&
				4u == owner.iGhostAuxiliaryNextSpawnTick &&
				room->m_strStatus.find("no live same-deck footprint") != std::string::npos;
			tests.Require(deferred && heldBeforeDue && retriedOnNextTick,
				"Auxiliary ghost spawn exhaustion remains non-fatal, consumes neither entity nor occurrence identity, and retries only on the next fixed tick");
		}
		{
			auto room = prepareRoom();
			auto& owner = room->m_WorldEntities.front();
			owner.strPatternId = "VALTAN_GHOST_FINALE";
			owner.iPatternSequence = 1u;
			WORLD_BOOTSTRAP_PLACEMENT placement{};
			placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			placement.strPlacementId = "boss.valtan.contract-dependent";
			placement.strArchetypeId = "BOSS_VALTAN_GHOST";
			placement.strEncounterId = "ENCOUNTER_VALTAN";
			placement.fPositionX = owner.fPositionX;
			placement.fPositionY = owner.fPositionY;
			placement.fPositionZ = owner.fPositionZ;
			SERVER_WORLD_ENTITY output{};
			output.iNetEntityId = 19999u;
			output.strPlacementId = "preserved.output";
			const NET_ENTITY_ID nextId = room->m_iNextNetEntityId;
			const auto rejected = [&](const NET_ENTITY_ID ownerId)
			{
				return !room->Build_WorldEntity(placement, 19101u, output, &catalog, ownerId) &&
					output.iNetEntityId == 19999u && output.strPlacementId == "preserved.output" &&
					room->m_iNextNetEntityId == nextId && room->m_WorldEntities.size() == 1u;
			};
			bool rejectsRoles = rejected(INVALID_NET_ENTITY_ID) && rejected(19101u) && rejected(19998u);
			const SERVER_WORLD_ENTITY validOwner = owner;
			owner.strPatternId = "VALTAN_TRASH";
			rejectsRoles = rejected(owner.iNetEntityId) && rejectsRoles;
			owner = validOwner;
			owner.strEncounterId = "ENCOUNTER_OTHER";
			rejectsRoles = rejected(owner.iNetEntityId) && rejectsRoles;
			owner = validOwner;
			owner.iOwnerBossNetEntityId = 19102u;
			rejectsRoles = rejected(owner.iNetEntityId) && rejectsRoles;
			owner = validOwner;
			owner.iCurrentHp = 0u;
			rejectsRoles = rejected(owner.iNetEntityId) && rejectsRoles;
			owner = validOwner;
			owner.PinnedDefinitionRevision.Bytes[0] ^= 0x42u;
			rejectsRoles = rejected(owner.iNetEntityId) && rejectsRoles;
			owner = validOwner;
			placement.strArchetypeId = "BOSS_VALTAN";
			rejectsRoles = rejected(owner.iNetEntityId) && rejectsRoles;
			placement.strArchetypeId = "BOSS_VALTAN_GHOST";
			const bool admitted = room->Build_WorldEntity(placement, 19101u, output, &catalog, owner.iNetEntityId);
			tests.Require(rejectsRoles && admitted && output.iOwnerBossNetEntityId == owner.iNetEntityId &&
				output.PinnedDefinitionRevision == owner.PinnedDefinitionRevision,
				"Dependent spawn requires its declared live primary, active finale and matching pin; invalid roles preserve staging output");
		}
		{
			auto room = prepareRoom();
			auto& owner = room->m_WorldEntities.front();
			owner.strPatternId = "VALTAN_GHOST_FINALE";
			owner.iPatternSequence = 1u;
			owner.fCollisionRadius = catalog.Find_Boss("BOSS_VALTAN")->fCollisionRadius;
			for (auto& [id, player] : room->m_Players)
				player.strNickName = "Ghost-Late-Join";
			const bool spawned = room->Update_DependentBosses(2u) && room->m_WorldEntities.size() == 2u;
			const GameplayDataRevision activeRevision =
				room->m_GameplayCatalog.Get_ActiveRevision();
			GameplayDataRevision retainedOccurrenceRevision = activeRevision;
			retainedOccurrenceRevision.Bytes[0] ^= 0x40u;
			if (!retainedOccurrenceRevision.Is_Valid())
				retainedOccurrenceRevision.Bytes[1] = 0x01u;
			for (SERVER_WORLD_ENTITY& entity : room->m_WorldEntities)
			{
				entity.PinnedDefinitionRevision = retainedOccurrenceRevision;
				if (entity.ProductSequencePinnedDefinitionRevision.Is_Valid())
				{
					entity.ProductSequencePinnedDefinitionRevision =
						retainedOccurrenceRevision;
				}
			}
			if (spawned)
				std::swap(room->m_WorldEntities.front(), room->m_WorldEntities.back());
			CGameRoom::STAGED_PLAYER_ENTRY entry{};
			entry.Player = room->m_Players.at(19200u);
			entry.Player.iPlayerId = 19202u;
			entry.Player.iNetEntityId = 19302u;
			std::string status;
			const auto batch = std::span<const CGameRoom::STAGED_PLAYER_ENTRY>(&entry, 1u);
			bool ordered = spawned && room->Build_PlayerEntryFrames(entry, batch, status);
			std::map<NET_ENTITY_ID, S2C_WORLD_ENTITY_SPAWNED> observed;
			for (const auto& frame : entry.Frames)
			{
				if (frame.ePacketType != PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED) continue;
				CPacketReader reader{ frame.Payload };
				S2C_WORLD_ENTITY_SPAWNED message{};
				if (!Read_Message(reader, message)) { ordered = false; break; }
				const auto source = std::find_if(
					room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
					[&message](const SERVER_WORLD_ENTITY& entity)
					{ return entity.iNetEntityId == message.iNetEntityId; });
				const auto parent = observed.find(message.iOwnerBossNetEntityId);
				ordered = ordered &&
					source != room->m_WorldEntities.end() &&
					message.PinnedDefinitionRevision ==
						source->PinnedDefinitionRevision &&
					message.PinnedDefinitionRevision ==
						retainedOccurrenceRevision &&
					Is_Valid_WorldEntitySpawnOwner(message,
						parent == observed.end() ? nullptr : &parent->second);
				observed.emplace(message.iNetEntityId, message);
			}
			const auto savedFrames = entry.Frames;
			const auto savedEntities = room->m_WorldEntities;
			std::erase_if(room->m_WorldEntities, [](const auto& entity)
				{ return entity.iOwnerBossNetEntityId == INVALID_NET_ENTITY_ID; });
			const bool rejectedOrphan = !room->Build_PlayerEntryFrames(entry, batch, status) &&
				status == "Initial dependent boss has no preceding primary owner" &&
				entry.Frames.size() == savedFrames.size() &&
				std::equal(entry.Frames.begin(), entry.Frames.end(), savedFrames.begin(),
					[](const PACKET_FRAME& left, const PACKET_FRAME& right)
					{ return left.ePacketType == right.ePacketType && left.Payload == right.Payload; });
			room->m_WorldEntities = savedEntities;
			room->m_Players.clear();
			const bool reset = room->Reset_ValtanArenaWhenEmpty() &&
				std::none_of(room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
					[](const auto& entity) { return entity.iOwnerBossNetEntityId != INVALID_NET_ENTITY_ID; }) &&
				room->m_CombatObjectRuntime.Get_LiveObjects().empty();
			tests.Require(ordered && activeRevision.Is_Valid() &&
				retainedOccurrenceRevision.Is_Valid() &&
				activeRevision != retainedOccurrenceRevision &&
				observed.size() == 2u && rejectedOrphan && reset,
				"Late entry with active R_new serializes an existing primary and ghost at their exact retained R_old, rejects orphan batches atomically and reset removes dependents");
		}
		{
			auto room = prepareRoom();
			auto& boss = room->m_WorldEntities.front();
			boss.PendingPatternIds = { "VALTAN_FIST_IN_OUT" };
			SERVER_COMBAT_OBJECT_POSE birthPose{};
			COMBAT_OBJECT_ID objectId = INVALID_COMBAT_OBJECT_ID;
			bool fixedPose = true;
			bool immediateSuccessor = false;
			std::size_t donutHits = 0u;
			for (std::uint32_t tick = 1u; tick <= 90u && room->Is_Ready(); ++tick)
			{
				room->m_iServerTick = tick - 1u;
				room->m_TickDamageEvents.clear();
				room->Update_WorldEntities(1.f / 30.f);
				const auto& live = room->m_CombatObjectRuntime.Get_LiveObjects();
				if (INVALID_COMBAT_OBJECT_ID == objectId && !live.empty())
				{
					objectId = live.front().iCombatObjectId;
					birthPose = live.front().LiveState.CurrentPose;
					boss.PendingPatternIds.push_back("VALTAN_FOUR_SLASH");
					boss.bAutomaticPatternSequenceStepRunning = true;
					boss.iAutomaticPatternSequenceInterStepPursuitTicks = 30u;
					boss.fPositionX += 40.f;
					for (auto& [id, player] : room->m_Players)
					{
						player.fPositionX = birthPose.fPositionX + 10.f;
						player.fPositionZ = birthPose.fPositionZ;
					}
				}
				for (const auto& object : live)
				{
					if (object.iCombatObjectId != objectId) continue;
					fixedPose = fixedPose &&
						std::hypot(object.LiveState.CurrentPose.fPositionX - birthPose.fPositionX,
							object.LiveState.CurrentPose.fPositionZ - birthPose.fPositionZ) < 0.001f;
					if (boss.strPatternId == "VALTAN_FOUR_SLASH" && tick < 10u &&
						boss.iAutomaticPatternSequencePursuitTicksRemaining == 0u)
						immediateSuccessor = true;
				}
				room->m_TickDamageEvents.clear();
				room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities,
					catalog, 1.f / 30.f, tick, room->m_TickDamageEvents);
				donutHits += room->m_TickDamageEvents.size();
			}
			tests.Require(room->Is_Ready() && objectId != INVALID_COMBAT_OBJECT_ID &&
				fixedPose && immediateSuccessor && donutHits == 2u &&
				room->m_CombatObjectRuntime.Get_LiveObjects().empty(),
				"Donut keeps immutable birth pose and timed hits while the next foreground starts without pursuit delay");
		}
		{
			auto room = prepareRoom();
			const SERVER_WORLD_ENTITY originalBoss = room->m_WorldEntities.front();
			for (std::uint32_t ordinal = 2u; ordinal < 4u; ++ordinal)
			{
				SERVER_PLAYER extra = room->m_Players.at(19200u);
				extra.iPlayerId = 19200u + ordinal;
				extra.iNetEntityId = 19300u + ordinal;
				room->m_PlayerIdByEntityId.emplace(extra.iNetEntityId, extra.iPlayerId);
				room->m_Players.emplace(extra.iPlayerId, extra);
			}
			std::set<NET_ENTITY_ID> selectedPlayers;
			bool facingTracked = true;
			bool centeredBeforeTakeoff = true;
			bool landedAtCenter = true;
			const auto yawMatches = [](const float actual, const float expected)
			{
				return std::fabs(std::remainder(actual - expected, 360.f)) < 0.001f;
			};
			for (std::uint32_t seed = 0u; seed < 32u && room->Is_Ready(); ++seed)
			{
				room->m_WorldEntities.front() = originalBoss;
				auto& boss = room->m_WorldEntities.front();
				boss.iPatternSequence = seed;
				boss.PendingPatternIds = { "VALTAN_SIX_PIZZA_106" };
				boss.fPositionX += 7.f;
				boss.fPositionZ -= 3.f;
				for (auto& [id, player] : room->m_Players)
				{
					const float angle = static_cast<float>(id - 19200u) * 1.57079632679f;
					player.fPositionX = boss.fSpawnPositionX + std::sin(angle) * 10.f;
					player.fPositionZ = boss.fSpawnPositionZ + std::cos(angle) * 10.f;
				}
				for (std::uint32_t tick = 1u; tick <= 80u && room->Is_Ready(); ++tick)
				{
					room->m_iServerTick = seed * 100u + tick - 1u;
					room->Update_WorldEntities(1.f / 30.f);
					const auto selected = std::find_if(
						room->m_Players.begin(), room->m_Players.end(),
						[&boss](const auto& entry)
						{ return entry.second.iNetEntityId == boss.iPatternTargetEntityId; });
					if (room->m_Players.end() == selected)
					{
						facingTracked = false;
					}
					else
					{
						const float expectedYaw = std::atan2(
							selected->second.fPositionX - boss.fLeapLandingX,
							selected->second.fPositionZ - boss.fLeapLandingZ) *
							57.2957795131f;
						facingTracked = facingTracked &&
							yawMatches(boss.fYawDegrees, expectedYaw);
					}
					if (tick == 1u)
					{
						selectedPlayers.insert(boss.iPatternTargetEntityId);
						for (auto& [id, player] : room->m_Players) player.fPositionX += 5.f;
					}
					if (tick == 25u)
						centeredBeforeTakeoff = centeredBeforeTakeoff &&
							std::hypot(boss.fPositionX - boss.fLeapLandingX,
								boss.fPositionZ - boss.fLeapLandingZ) < 0.001f;
				}
				landedAtCenter = landedAtCenter &&
					std::fabs(boss.fPositionY - boss.fLeapLandingY) < 0.001f;
			}
			tests.Require(room->Is_Ready() && selectedPlayers.size() == 4u && facingTracked &&
				centeredBeforeTakeoff && landedAtCenter,
				"Pizza selects across four alive players and tracks center-relative yaw through approach, takeoff and landing");
		}
		for (const char* patternId : { "VALTAN_TRASH_CATCH_IF",
			"VALTAN_TRASH_CATCH_SUCCESS", "VALTAN_TRASH_CATCH_FAIL" })
		{
			auto room = prepareRoom();
			room->m_WorldEntities.front().PendingPatternIds = { patternId };
			bool finished = false;
			for (std::uint32_t tick = 1u; tick < 600u && room->Is_Ready(); ++tick)
			{
				room->m_iServerTick = tick - 1u;
				room->Update_WorldEntities(1.f / 30.f);
				const auto& boss = room->m_WorldEntities.front();
				finished = boss.strPatternId.empty() && boss.iPatternSequence == 1u &&
					SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == boss.PatternTerminalReceipt.eResult;
				if (finished) break;
			}
			tests.Require(room->Is_Ready() && finished &&
				std::all_of(room->m_Players.begin(), room->m_Players.end(),
					[](const auto& entry) { return entry.second.iAttachmentOwnerNetEntityId == INVALID_NET_ENTITY_ID; }),
				"Trash IF, SUCCESS and FAIL standalone fragments finish without cross-pattern attachment carry");
		}
		const auto releaseFailurePreservesPlayers = [&prepareRoom, &stageById,
			&trashDefinition](const std::shared_ptr<CGameplayCatalog>& exitGeneration)
		{
			auto room = prepareRoom();
			if (!room->Is_Ready() || (nullptr != exitGeneration &&
				!room->m_GameplayCatalog.Initialize(exitGeneration))) return false;
			const bool exitRelease = nullptr != exitGeneration;
			const auto& stage = stageById(exitRelease ? "GROGGY" : "STEP_07");
			auto& boss = room->m_WorldEntities.front();
			boss.PendingPatternIds.clear();
			boss.iPatternSequence = 5u;
			boss.strPatternId = "VALTAN_TRASH";
			boss.strPatternStageId = stage.strStageId;
			boss.strActionId = stage.strActionId;
			boss.iPatternStageIndex = static_cast<std::uint32_t>(
				&stage - trashDefinition->Stages.data());
			boss.iPatternStageDurationMs = stage.iDurationMs;
			boss.iPatternStageFirstEvaluationTick = exitRelease ? 1u : 100u;
			boss.iActionStartTick = boss.iPatternStageFirstEvaluationTick;
			boss.eAction = exitRelease ? SERVER_ENTITY_ACTION::PATTERN_ACTIVE :
				SERVER_ENTITY_ACTION::PATTERN_WINDUP;
			boss.PinnedDefinitionRevision = room->m_GameplayCatalog.Get_ActiveRevision();
			if (!room->Apply_BossPatternStageTransition(boss, "", "",
				boss.strPatternId, boss.strActionId, boss.PinnedDefinitionRevision,
				boss.PinnedDefinitionRevision, 99u)) return false;
			for (auto& [id, player] : room->m_Players)
			{
				if (!room->Capture_PlayerAttachment(player.iNetEntityId, boss.iNetEntityId,
					PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 99u)) return false;
			}
			/* Keep one valid attachment and make only the second target invalid.
			Call the actual boss update seam before player sanitization can hide it. */
			auto& malformed = room->m_Players.at(19201u);
			malformed.eAction = PLAYER_ACTION_STATE::NONE;
			if (!exitRelease && !CBossCombatRuntime::Try_TriggerCounter(boss, 101u))
				return false;
			room->m_TickDamageEvents.clear();
			room->m_iServerTick = exitRelease ? 133u : 100u;
			room->Update_WorldEntities(1.f / 30.f);
			const auto& first = room->m_Players.at(19200u);
			return !room->Is_Ready() &&
				room->m_strStatus == "Boss grabbed-player release target is invalid" &&
				boss.strPatternId == "VALTAN_TRASH" && boss.strPatternStageId == stage.strStageId &&
				boss.iPatternSequence == 5u &&
				boss.PatternTerminalReceipt.eResult == SERVER_BOSS_PATTERN_TERMINAL_RESULT::NONE &&
				first.iCurrentHp == first.iMaximumHp && first.eAction == PLAYER_ACTION_STATE::GRABBED &&
				first.iAttachmentOwnerNetEntityId == boss.iNetEntityId &&
				first.eAttachmentSlot == PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND &&
				first.iAttachmentPatternSequence == 5u &&
				malformed.iCurrentHp == malformed.iMaximumHp && malformed.eAction == PLAYER_ACTION_STATE::NONE &&
				malformed.iAttachmentOwnerNetEntityId == boss.iNetEntityId &&
				malformed.iAttachmentPatternSequence == 5u && room->m_TickDamageEvents.empty();
		};
		tests.Require(releaseFailurePreservesPlayers(nullptr),
			"Trash failed counter ENTER release preserves every player and rolls back the boss stage");
		/* EXIT release is an admitted catalog contract too. Load a temporary
		variant through normal hash/parse admission; never mutate the live catalog. */
		namespace fs = std::filesystem;
		std::vector<wchar_t> pathBuffer(32768u);
		fs::path dataRoot;
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != configuredLength && configuredLength < pathBuffer.size())
			dataRoot = fs::path(pathBuffer.data()).lexically_normal();
		else
		{
			const DWORD moduleLength = GetModuleFileNameW(nullptr, pathBuffer.data(),
				static_cast<DWORD>(pathBuffer.size()));
			if (0u != moduleLength && moduleLength < pathBuffer.size())
				dataRoot = fs::path(pathBuffer.data()).parent_path().parent_path() / L"DataFiles";
		}
		std::ifstream source(dataRoot / L"Gameplay" / L"Gameplay.bootstrap", std::ios::binary);
		std::string exitBootstrapText((std::istreambuf_iterator<char>(source)),
			std::istreambuf_iterator<char>());
		const std::string releaseEnter =
			"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TRASH\tvaltan.sequence.center-trash-rush-if.groggy\t0\tENTER\tRELEASE_GRABBED_PLAYERS";
		const std::size_t releaseAt = exitBootstrapText.find(releaseEnter);
		bool exitVariantReady = !source.bad() && std::string::npos != releaseAt;
		if (exitVariantReady)
			exitBootstrapText.replace(releaseAt + releaseEnter.find("ENTER\tRELEASE"), 5u, "EXIT");
		std::error_code variantError;
		/* Immutable catalog admission requires the canonical artifact filename. */
		const fs::path exitDirectory = fs::temp_directory_path() /
			(L"LostArkTrashExitReleaseContract-" + std::to_wstring(GetCurrentProcessId()));
		const fs::path exitBootstrap = exitDirectory / L"Gameplay.bootstrap";
		if (exitVariantReady)
		{
			fs::create_directories(exitDirectory, variantError);
			exitVariantReady = !variantError;
		}
		if (exitVariantReady)
		{
			std::ofstream stream(exitBootstrap, std::ios::binary | std::ios::trunc);
			stream.write(exitBootstrapText.data(), static_cast<std::streamsize>(exitBootstrapText.size()));
			exitVariantReady = stream.good();
		}
		const fs::path canonicalExitBootstrap = exitVariantReady ?
			fs::canonical(exitBootstrap, variantError) : fs::path{};
		auto exitGeneration = std::make_shared<CGameplayCatalog>();
		GameplayDataRevision exitBootstrapRevision{};
		GameplayDataRevision exitDefinitionRevision = catalog.Get_ActiveRevision();
		exitDefinitionRevision.Bytes[0] ^= 0x10u;
		if (!exitDefinitionRevision.Is_Valid()) exitDefinitionRevision.Bytes[1] ^= 1u;
		std::string exitStatus;
		struct EXIT_GENERATION_LOAD_CONTEXT final
		{
			std::shared_ptr<CGameplayCatalog>* pGeneration;
			const fs::path* pPath;
			GameplayDataRevision* pBootstrapRevision;
			const GameplayDataRevision* pDefinitionRevision;
			std::string* pStatus;
			bool loaded = false;
		} loadContext{ &exitGeneration, &canonicalExitBootstrap, &exitBootstrapRevision,
			&exitDefinitionRevision, &exitStatus };
		const auto loadExitGeneration = [](void* opaque)
		{
			auto& context = *static_cast<EXIT_GENERATION_LOAD_CONTEXT*>(opaque);
			context.loaded = CServerApp::Hash_GameplayFileForAdmission(*context.pPath,
				*context.pBootstrapRevision, *context.pStatus) &&
				(*context.pGeneration)->Load_FromBootstrap(*context.pPath,
					*context.pBootstrapRevision, *context.pDefinitionRevision);
		};
		const bool exitLoaded = exitVariantReady && !variantError &&
			Run_WithContractWorkerStack(loadExitGeneration, &loadContext) && loadContext.loaded;
		tests.Require(exitLoaded && releaseFailurePreservesPlayers(exitGeneration),
			"Trash failed terminal EXIT release preserves every player and rolls back the completed receipt");
		fs::remove(exitBootstrap, variantError);
		fs::remove(exitDirectory, variantError);
	}
}

