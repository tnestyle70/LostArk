#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_ValtanLifecycle(TESTS& tests, CGameplayCatalog& catalog)
{

	{
		const BOSS_PATTERN_SEQUENCE_DEFINITION* sequence =
			catalog.Find_BossPatternSequence("ENCOUNTER_VALTAN");
		tests.Require(
			nullptr != sequence && !sequence->strSequenceId.empty() &&
			BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE == sequence->eMode &&
			sequence->iInterStepPursuitMs >= 100u && sequence->iInterStepPursuitMs <= 10000u &&
			sequence->iInterStepPursuitTicks ==
				(sequence->iInterStepPursuitMs * 30u + 999u) / 1000u &&
			!sequence->PatternIds.empty() &&
			sequence->PatternIds.size() <= MAX_VALTAN_PATTERN_FLOW_SLOTS &&
			sequence->iExpectedStepCount == sequence->PatternIds.size() &&
			sequence->TransitionPursuitMs.size() + 1u ==
				sequence->PatternIds.size() &&
			sequence->TransitionPursuitTicks.size() ==
				sequence->TransitionPursuitMs.size(),
			"Load the saved Product sequence as exact ordered occurrences without a fixed review-list length");
		const auto* patterns = catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		tests.Require(
			nullptr != patterns && nullptr != sequence &&
			std::none_of(patterns->begin(), patterns->end(),
				[](const BOSS_PATTERN_DEFINITION& pattern)
				{ return "VALTAN_SEQUENCE_FRONT_BACK_FRONT" == pattern.strPatternId; }) &&
			1 == std::count_if(patterns->begin(), patterns->end(),
				[](const BOSS_PATTERN_DEFINITION& pattern)
				{ return "VALTAN_SEQUENCE_FOUR" == pattern.strPatternId; }) &&
			0 == std::count(sequence->PatternIds.begin(), sequence->PatternIds.end(),
				"VALTAN_SEQUENCE_FRONT_BACK_FRONT"),
			"Exclude retired FRONT_BACK_FRONT and retain one FOUR definition while saved occurrences may repeat it");
	}
	{
		/* Ghost death is a visual transition inside the Product sequence, not a
		one-second chase opportunity. Its terminal EXIT consumes only the delay
		that FinishPattern reserved; the cursor then admits Respawn on the next
		fixed tick. An isolated Debug audition keeps its pre-existing hold bit. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		const BOSS_PATTERN_SEQUENCE_DEFINITION* sequence =
			room.m_GameplayCatalog.Active().Find_BossPatternSequence(
				"ENCOUNTER_VALTAN");
		const auto deathStep = nullptr == sequence ?
			std::vector<std::string>::const_iterator{} :
			std::find(sequence->PatternIds.cbegin(), sequence->PatternIds.cend(),
				"VALTAN_GHOST_DEATH_AUDITION");
		const bool adjacentRespawn = nullptr != sequence &&
			deathStep != sequence->PatternIds.cend() &&
			std::next(deathStep) != sequence->PatternIds.cend() &&
			"VALTAN_GHOST_RESPAWN_AUDITION" == *std::next(deathStep);
		const auto* patterns =
			room.m_GameplayCatalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto findPattern = [patterns](const std::string_view patternId)
			-> const BOSS_PATTERN_DEFINITION*
			{
				if (nullptr == patterns) return nullptr;
				const auto found = std::find_if(
					patterns->begin(), patterns->end(),
					[patternId](const BOSS_PATTERN_DEFINITION& candidate)
					{ return candidate.strPatternId == patternId; });
				return patterns->end() == found ? nullptr : &*found;
			};
		const BOSS_PATTERN_DEFINITION* death =
			findPattern("VALTAN_GHOST_DEATH_AUDITION");
		const BOSS_PATTERN_DEFINITION* respawn =
			findPattern("VALTAN_GHOST_RESPAWN_AUDITION");
		const bool exactDeathSuppression = nullptr != death &&
			1u == death->Stages.size() &&
			1 == std::count_if(
				death->Stages.front().Actions.begin(),
				death->Stages.front().Actions.end(),
				[](const BOSS_PATTERN_STAGE_ACTION& action)
				{
					return BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT ==
							action.eTrigger &&
						BOSS_PATTERN_STAGE_ACTION_KIND::
							SUPPRESS_INTER_STEP_PURSUIT == action.eKind &&
						"boss.sequence.inter-step-pursuit" == action.strTargetId &&
						0u == action.iValue && 0u == action.iDurationMs;
				});
		const bool exactRespawnPhase = nullptr != respawn &&
			1u == respawn->Stages.size() &&
			1 == std::count_if(
				respawn->Stages.front().Actions.begin(),
				respawn->Stages.front().Actions.end(),
				[](const BOSS_PATTERN_STAGE_ACTION& action)
				{
					return BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
							action.eTrigger &&
						BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE ==
							action.eKind &&
						"boss.phase.gameplay" == action.strTargetId &&
						3u == action.iValue;
				});

		SERVER_WORLD_ENTITY* admitted = room.Find_AuditionBoss();
		SERVER_WORLD_ENTITY boss = nullptr == admitted ?
			SERVER_WORLD_ENTITY{} : *admitted;
		boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		boss.eAction = SERVER_ENTITY_ACTION::IDLE;
		boss.strArchetypeId = "BOSS_VALTAN";
		boss.strEncounterId = "ENCOUNTER_VALTAN";
		boss.strPlacementId = "boss.valtan.center";
		boss.iCurrentHp = (std::max)(1u, boss.iMaximumHp);
		boss.iMaximumHp = boss.iCurrentHp;
		boss.iMaximumHealthBars = 160u;
		boss.iPhase = 1u;
		boss.bIntroPatternConsumed = true;
		boss.strPatternId.clear();
		boss.strPatternStageId.clear();
		boss.strActionId.clear();
		boss.iPatternSequence = 77u;
		boss.PatternTerminalReceipt.iPatternSequence = boss.iPatternSequence;
		boss.PatternTerminalReceipt.iRootPatternSequence = boss.iPatternSequence;
		boss.PatternTerminalReceipt.eResult =
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED;
		boss.iAutomaticPatternSequencePursuitTicksRemaining = 30u;
		boss.iAutomaticPatternSequenceInterStepPursuitTicks = 30u;
		boss.strRotationId = nullptr == sequence ? std::string{} :
			sequence->strSequenceId;
		boss.iRotationStepIndex = adjacentRespawn ?
			static_cast<std::uint32_t>(
				std::distance(sequence->PatternIds.cbegin(), deathStep) + 1) : 0u;
		const GameplayDataRevision revision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		const bool suppressed = exactDeathSuppression &&
			room.Apply_BossPatternStageTransition(
				boss, "VALTAN_GHOST_DEATH_AUDITION",
				"valtan.sequence.dead.step-01", {}, {}, revision, revision, 101u) &&
			0u == boss.iAutomaticPatternSequencePursuitTicksRemaining &&
			!boss.bAutomaticPatternSequenceAuditionHold;

		SERVER_WORLD_ENTITY held = boss;
		held.iAutomaticPatternSequencePursuitTicksRemaining = 30u;
		held.bAutomaticPatternSequenceAuditionHold = true;
		const bool isolatedHoldPreserved = exactDeathSuppression &&
			room.Apply_BossPatternStageTransition(
				held, "VALTAN_GHOST_DEATH_AUDITION",
				"valtan.sequence.dead.step-01", {}, {}, revision, revision, 102u) &&
			0u == held.iAutomaticPatternSequencePursuitTicksRemaining &&
			held.bAutomaticPatternSequenceAuditionHold;

		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 991u;
		player.iNetEntityId = 1991u;
		player.iCurrentHp = 100000u;
		player.iMaximumHp = player.iCurrentHp;
		player.isCombatReady = true;
		player.fPositionX = boss.fPositionX;
		player.fPositionY = boss.fPositionY;
		player.fPositionZ = boss.fPositionZ;
		players.emplace(player.iPlayerId, player);
		std::vector<DAMAGE_EVENT> damageEvents;
		CValtanBrain brain;
		if (suppressed && adjacentRespawn)
		{
			brain.Update(
				boss, players, room.m_GameplayCatalog.Active(),
				room.m_ServerNavigation, 1.f / 30.f, 103u, {}, damageEvents,
				&room.m_GameplayCatalog.Active(),
				room.m_GameplayCatalog.Get_ActiveGenerationEpoch(), nullptr,
				sequence);
		}
		const bool respawnStartedWithoutChase =
			"VALTAN_GHOST_RESPAWN_AUDITION" == boss.strPatternId &&
			SERVER_ENTITY_ACTION::CHASE != boss.eAction;
		const bool respawnEnteredPhaseThree = respawnStartedWithoutChase &&
			room.Apply_BossPatternStageTransition(
				boss, {}, {}, boss.strPatternId, boss.strActionId,
				revision, boss.PinnedDefinitionRevision, 103u) &&
			3u == boss.iPhase;
		tests.Require(
			activated && adjacentRespawn && exactDeathSuppression &&
			exactRespawnPhase && suppressed && isolatedHoldPreserved &&
			respawnStartedWithoutChase && respawnEnteredPhaseThree,
			"Suppress only Ghost Death pursuit, preserve Debug hold, then enter Ghost Respawn and phase 3 on the next fixed tick");
	}
	{
		const auto* patterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto findPattern = [patterns](const char* patternId)
			-> const BOSS_PATTERN_DEFINITION*
			{
				if (nullptr == patterns) return nullptr;
				const auto found = std::find_if(
					patterns->begin(), patterns->end(),
					[patternId](const BOSS_PATTERN_DEFINITION& pattern)
					{ return pattern.strPatternId == patternId; });
				return patterns->end() == found ? nullptr : &*found;
			};
		const auto findStage = [](const BOSS_PATTERN_DEFINITION* pattern,
			const char* stageId) -> const BOSS_PATTERN_STAGE_DEFINITION*
			{
				if (nullptr == pattern) return nullptr;
				const auto found = std::find_if(
					pattern->Stages.begin(), pattern->Stages.end(),
					[stageId](const BOSS_PATTERN_STAGE_DEFINITION& stage)
					{ return stage.strStageId == stageId; });
				return pattern->Stages.end() == found ? nullptr : &*found;
			};
		const BOSS_PATTERN_DEFINITION* trash = findPattern("VALTAN_TRASH");
		const BOSS_PATTERN_DEFINITION* pizza =
			findPattern("VALTAN_SIX_PIZZA_106");
		const BOSS_PATTERN_DEFINITION* attackWhirlwind =
			findPattern("VALTAN_ATTACK_WHIRLWIND");
		const BOSS_PATTERN_STAGE_DEFINITION* trashCounter =
			findStage(trash, "STEP_07");
		const BOSS_PATTERN_STAGE_DEFINITION* trashRelease =
			findStage(trash, "GROGGY");
		const BOSS_PATTERN_STAGE_DEFINITION* trashRush =
			findStage(trash, "STEP_08");
		const BOSS_PATTERN_DEFINITION* catchBreath =
			findPattern("VALTAN_CATCH_BREATH");
		const BOSS_PATTERN_DEFINITION* dash =
			findPattern("VALTAN_DASH_CHARGE");
		const BOSS_PATTERN_STAGE_DEFINITION* dashGroggy =
			findStage(dash, "GROGGY");
		const BOSS_PATTERN_STAGE_DEFINITION* catchGrab =
			findStage(catchBreath, "STEP_02");
		const BOSS_PATTERN_STAGE_DEFINITION* catchRelease =
			findStage(catchBreath, "STEP_04");
		const auto hasAction = [](const BOSS_PATTERN_STAGE_DEFINITION* stage,
			const BOSS_GRABBED_RELEASE_MODE mode,
			const float speed, const std::uint32_t durationMs,
			const float yawOffsetDegrees)
			{
				return nullptr != stage && stage->Actions.end() != std::find_if(
					stage->Actions.begin(), stage->Actions.end(),
					[mode, speed, durationMs, yawOffsetDegrees](
						const BOSS_PATTERN_STAGE_ACTION& action)
					{
						return BOSS_PATTERN_STAGE_ACTION_KIND::
							RELEASE_GRABBED_PLAYERS == action.eKind &&
							BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
								action.eTrigger &&
							"boss.attachment.left-hand" == action.strTargetId &&
							action.eReleaseMode == mode &&
							action.fReleaseSpeedMps == speed &&
							action.iDurationMs == durationMs &&
							action.fReleaseYawOffsetDegrees ==
								yawOffsetDegrees;
					});
			};
		const auto hasBossFlagAction = [](
			const BOSS_PATTERN_STAGE_DEFINITION* stage,
			const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			const std::string_view targetId, const std::uint32_t value)
			{
				return nullptr != stage && stage->Actions.end() != std::find_if(
					stage->Actions.begin(), stage->Actions.end(),
					[trigger, targetId, value](
						const BOSS_PATTERN_STAGE_ACTION& action)
					{
						return BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG ==
							action.eKind && trigger == action.eTrigger &&
							targetId == action.strTargetId && value == action.iValue;
					});
			};
		tests.Require(
			nullptr != pizza &&
			BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_ON_START ==
				pizza->eTargetPolicy &&
			BOSS_PATTERN_AIM_POLICY::TRACK_TARGET_EACH_TICK ==
				pizza->eAimPolicy,
			"Load one random six-pizza target and track its arena-center facing for the whole occurrence");
		tests.Require(
			nullptr != attackWhirlwind && 4u == attackWhirlwind->Stages.size() &&
			BOSS_PATTERN_TARGET_POLICY::LOCK_NEAREST_ON_START ==
				attackWhirlwind->eTargetPolicy &&
			BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START ==
				attackWhirlwind->eAimPolicy &&
			std::all_of(attackWhirlwind->Stages.begin(),
				attackWhirlwind->Stages.begin() + 3,
				[](const BOSS_PATTERN_STAGE_DEFINITION& stage)
				{
					return std::none_of(stage.Actions.begin(), stage.Actions.end(),
						[](const BOSS_PATTERN_STAGE_ACTION& action)
						{ return BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE == action.eKind; });
				}) &&
			"STEP_04" == attackWhirlwind->Stages.back().strStageId &&
			1u == attackWhirlwind->Stages.back().Actions.size() &&
			BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE ==
				attackWhirlwind->Stages.back().Actions.front().eKind &&
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
				attackWhirlwind->Stages.back().Actions.front().eTrigger,
			"Load the jump-whirlwind facing lock and its single end-stage ENTER retarget");
		tests.Require(
			nullptr != trash &&
			BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_ON_START ==
				trash->eTargetPolicy &&
			BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START ==
				trash->eAimPolicy &&
			nullptr != trashCounter && nullptr != trashRelease &&
			nullptr != trashRush &&
			BOSS_PATTERN_STAGE_KIND::WINDUP == trashCounter->eStageKind &&
			trashCounter->bHasCounterProxy &&
			BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_LOCAL_CIRCLE ==
				trashCounter->eCounterProxyKind &&
			std::abs(trashCounter->fCounterProxyForwardOffsetM - 1.f) < 1.0e-6f &&
			std::abs(trashCounter->fCounterProxyRightOffsetM) < 1.0e-6f &&
			std::abs(trashCounter->fCounterProxyRadiusM - 2.25f) < 1.0e-6f &&
			std::abs(trashCounter->fCounterProxyArcDegrees) < 1.0e-6f &&
			hasBossFlagAction(
				trashCounter, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
				"boss.flag.counterable", 1u) &&
			hasBossFlagAction(
				trashCounter, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
				"boss.flag.counterable", 0u) &&
			BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE ==
				trashRush->ePlayerResponse &&
			PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ==
				trashRush->eAttachmentSlot &&
			BOSS_PATTERN_HIT_SHAPE::BOX == trashRush->eHitShape &&
			7u == trashRush->HitOffsetsMs.size() &&
			hasAction(
				trashRelease, BOSS_GRABBED_RELEASE_MODE::HOLD,
				0.f, 0u, 0.f),
			"Load the pre-charge Valtan counter window, frontal counter proxy, and groggy hold release contract");
		tests.Require(
			nullptr != dash && 3u == dash->Stages.size() &&
			nullptr != dashGroggy && &dash->Stages[2] == dashGroggy &&
			6833u == dashGroggy->iDurationMs &&
			BOSS_PATTERN_PART_DAMAGE_POLICY::DESTROY_FIRST_ELIGIBLE ==
				dashGroggy->ePartDamagePolicy &&
			hasBossFlagAction(
				dashGroggy, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
				"boss.flag.groggy", 1u) &&
			hasBossFlagAction(
				dashGroggy, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
				"boss.flag.groggy", 0u),
			"Load Dash and its saved 6833 ms GROGGY continuation as one three-stage pattern");
		tests.Require(
			nullptr != catchBreath && nullptr != catchGrab &&
			nullptr != catchRelease &&
			BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_BEHIND_ON_START ==
				catchBreath->eTargetPolicy &&
			BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START ==
				catchBreath->eAimPolicy &&
			BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE ==
				catchGrab->ePlayerResponse &&
			PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ==
				catchGrab->eAttachmentSlot &&
			BOSS_PATTERN_HIT_SHAPE::CONE == catchGrab->eHitShape &&
			2u == catchGrab->Branches.size() &&
			catchGrab->Branches.end() != std::find_if(
				catchGrab->Branches.begin(), catchGrab->Branches.end(),
				[](const BOSS_PATTERN_STAGE_BRANCH& branch)
				{
					return BOSS_PATTERN_STAGE_OUTCOME::ANY_PLAYER_GRABBED ==
						branch.eOutcome &&
						"valtan.sequence.catch-breath.step-03" ==
						branch.strNextActionId &&
						branch.strNextPatternId.empty();
				}) &&
			catchGrab->Branches.end() != std::find_if(
				catchGrab->Branches.begin(), catchGrab->Branches.end(),
				[](const BOSS_PATTERN_STAGE_BRANCH& branch)
				{
					return BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT ==
						branch.eOutcome &&
						branch.strNextActionId.empty() &&
						branch.strNextPatternId.empty();
				}) &&
			hasAction(
				catchRelease,
				BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION,
				24.f, 500u, 180.f),
			"Load the rear-cone grab with explicit captured/timeout branches and forward-facing 180-degree, 24m/s minimum-12m arena ejection");
	}
	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		room.m_WorldEntities.clear();
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		tests.Require(activated && 1u == room.m_WorldEntities.size(),
			"Activate the real Valtan room for jump-whirlwind facing contracts");
		if (activated && 1u == room.m_WorldEntities.size())
		{
			SERVER_WORLD_ENTITY& boss = room.m_WorldEntities.front();
			boss.bIntroPatternConsumed = true;
			boss.bScriptedPatternPlayback = true;
			boss.iPhase = 2u;
			boss.fEngageDistance = 100.f;
			boss.iLastEvaluatedHealthBar = CValtanBrain::Calculate_HealthBar(boss);
			for (std::uint32_t ordinal = 0u; ordinal != 2u; ++ordinal)
			{
				SERVER_PLAYER player{};
				player.iPlayerId = 19400u + ordinal;
				player.iNetEntityId = 19500u + ordinal;
				player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
				player.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
				player.iCurrentHp = player.iMaximumHp = 100000u;
				player.iCurrentResource = player.iMaximumResource = 1000u;
				player.isCombatReady = true;
				room.m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
				room.m_Players.emplace(player.iPlayerId, player);
			}
			const auto placePlayer = [&room, &boss](const PLAYER_ID id,
				const float offsetX, const float offsetZ)
			{
				SERVER_NAV_POINT point{};
				if (!room.m_ServerNavigation.Project_Point(
					boss.fPositionX + offsetX, boss.fPositionZ + offsetZ, point) ||
					!room.m_ServerNavigation.Is_PointWalkableExact(point.x, point.z))
				{
					return false;
				}
				SERVER_PLAYER& player = room.m_Players.at(id);
				player.fPositionX = point.x;
				player.fPositionY = point.y;
				player.fPositionZ = point.z;
				return true;
			};
			const auto yawToward = [&boss](const SERVER_PLAYER& player)
			{
				return std::atan2(player.fPositionX - boss.fPositionX,
					player.fPositionZ - boss.fPositionZ) *
					(180.f / 3.14159265358979323846f);
			};
			const auto sameYaw = [](const float actual, const float expected)
			{
				return std::isfinite(actual) && std::isfinite(expected) &&
					std::abs(std::remainder(actual - expected, 360.f)) < 0.001f;
			};
			SERVER_WORLD_ENTITY enteredEndStage{};
			for (std::uint32_t occurrence = 0u; occurrence != 2u && room.Is_Ready();
				++occurrence)
			{
				const float direction = 0u == occurrence ? 1.f : -1.f;
				bool positionsValid = placePlayer(19400u, 0.f, 12.f * direction) &&
					placePlayer(19401u, 0.f, -18.f * direction);
				const float initialYaw = yawToward(room.m_Players.at(19400u));
				boss.PendingPatternIds.push_back("VALTAN_ATTACK_WHIRLWIND");
				room.Tick(1.f / 30.f);
				const std::uint32_t sequence = boss.iPatternSequence;
				tests.Require(positionsValid && room.Is_Ready() &&
					occurrence + 1u == sequence && "STEP_01" == boss.strPatternStageId &&
					19500u == boss.iPatternTargetEntityId && sameYaw(boss.fYawDegrees, initialYaw),
					0u == occurrence ?
						"Aim jump-whirlwind at the initial nearest player on the real room tick" :
						"Aim a repeated jump-whirlwind occurrence afresh without retaining its prior end facing");
				positionsValid = placePlayer(19400u, 18.f * direction, 0.f) &&
					placePlayer(19401u, -12.f * direction, 0.f) && positionsValid;
				std::uint32_t attackStagesSeen = 1u;
				std::uint32_t endEntries = 0u;
				NET_ENTITY_ID endTarget = INVALID_NET_ENTITY_ID;
				float endYaw = initialYaw;
				bool attackFacingLocked = true;
				bool endEntryOnTime = false;
				bool endFacingLocked = true;
				for (std::uint32_t tick = 0u; tick != 240u && room.Is_Ready() &&
					!boss.strPatternId.empty(); ++tick)
				{
					const std::string previousStage = boss.strPatternStageId;
					const std::uint32_t spinEndTick = "STEP_03" == previousStage ?
						boss.iPatternStageFirstEvaluationTick +
						(boss.iPatternStageDurationMs * 30u + 999u) / 1000u - 1u : 0u;
					room.Tick(1.f / 30.f);
					if (boss.strPatternId.empty()) break;
					if (boss.iPatternStageIndex < 3u)
					{
						attackStagesSeen |= 1u << boss.iPatternStageIndex;
						attackFacingLocked = attackFacingLocked &&
							sameYaw(boss.fYawDegrees, initialYaw) &&
							19500u == boss.iPatternTargetEntityId;
					}
					else if ("STEP_04" == boss.strPatternStageId)
					{
						if ("STEP_04" != previousStage)
						{
							++endEntries;
							endYaw = boss.fYawDegrees;
							endTarget = boss.iPatternTargetEntityId;
							const auto selected = std::find_if(room.m_Players.begin(),
								room.m_Players.end(), [endTarget](const auto& row)
								{ return row.second.iNetEntityId == endTarget; });
							endEntryOnTime = "STEP_03" == previousStage &&
								spinEndTick == room.m_iServerTick &&
								room.m_Players.end() != selected &&
								sameYaw(endYaw, yawToward(selected->second)) &&
								!sameYaw(endYaw, initialYaw);
							enteredEndStage = boss;
							positionsValid = placePlayer(19400u, 0.f, -18.f * direction) &&
								placePlayer(19401u, 0.f, 12.f * direction) && positionsValid;
						}
						else
						{
							endFacingLocked = endFacingLocked && sameYaw(boss.fYawDegrees, endYaw) &&
								endTarget == boss.iPatternTargetEntityId;
						}
					}
				}
				tests.Require(positionsValid && 7u == attackStagesSeen && attackFacingLocked,
					"Keep jump, preparation, and whirlwind facing locked despite a nearer moving player");
				tests.Require(room.Is_Ready() && 1u == endEntries && endEntryOnTime &&
					endFacingLocked && sameYaw(boss.fYawDegrees, endYaw) &&
					boss.strPatternId.empty() && sequence == boss.PatternTerminalReceipt.iPatternSequence &&
					SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == boss.PatternTerminalReceipt.eResult,
					"Retarget once exactly when the spin ends and hold that facing through the completed end stage");
			}
			boss.PendingPatternIds.push_back("VALTAN_ATTACK_WHIRLWIND");
			room.Tick(1.f / 30.f);
			const bool startedWithoutFailure = "STEP_01" == boss.strPatternStageId;
			const float abortedYaw = boss.fYawDegrees;
			for (auto& [id, player] : room.m_Players)
			{
				(void)id;
				player.iCurrentHp = 0u;
				player.isCombatReady = false;
				player.eAction = PLAYER_ACTION_STATE::DEAD;
			}
			room.Tick(1.f / 30.f);
			tests.Require(startedWithoutFailure && room.Is_Ready() && boss.strPatternId.empty() &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED == boss.PatternTerminalReceipt.eResult &&
				sameYaw(boss.fYawDegrees, abortedYaw),
				"Abort a targetless jump-whirlwind room occurrence without an early end-stage turn");
			const float emptyRetargetYaw = enteredEndStage.fYawDegrees;
			const bool emptyRetargetApplied = !enteredEndStage.strActionId.empty() &&
				room.Apply_BossPatternStageTransition(enteredEndStage,
					"VALTAN_ATTACK_WHIRLWIND", "valtan.sequence.attack-whirlwind.step-03",
					"VALTAN_ATTACK_WHIRLWIND", "valtan.sequence.attack-whirlwind.step-04",
					enteredEndStage.PinnedDefinitionRevision, enteredEndStage.PinnedDefinitionRevision,
					room.m_iServerTick + 1u);
			tests.Require(emptyRetargetApplied &&
				INVALID_NET_ENTITY_ID == enteredEndStage.iPatternTargetEntityId &&
				!enteredEndStage.bHasPatternTargetLastPosition &&
				sameYaw(enteredEndStage.fYawDegrees, emptyRetargetYaw),
				"Keep finite facing and clear the pattern target when the end-stage retarget has no living candidate");
		}
	}
	{
		/* A finale restart deliberately has no caller-computed nearest pointer.
		   Its ordinary target-policy admission must recover the nearest engageable
		   player from the boss's live position rather than map iteration order. */
		CValtanBrain brain;
		SERVER_WORLD_ENTITY boss{};
		boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		boss.iNetEntityId = 19690u;
		boss.iCurrentHp = boss.iMaximumHp = 1000u;
		boss.iPatternSequence = 7u;
		boss.fPositionX = 10.f;
		boss.fPositionY = 2.f;
		boss.fPositionZ = 20.f;
		boss.fYawDegrees = 0.f;

		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER mapFirstButFar{};
		mapFirstButFar.iPlayerId = 19691u;
		mapFirstButFar.iNetEntityId = 19692u;
		mapFirstButFar.iCurrentHp = mapFirstButFar.iMaximumHp = 1000u;
		mapFirstButFar.isCombatReady = true;
		mapFirstButFar.fPositionX = -10.f;
		mapFirstButFar.fPositionY = 2.f;
		mapFirstButFar.fPositionZ = 20.f;
		players.emplace(mapFirstButFar.iPlayerId, mapFirstButFar);
		SERVER_PLAYER mapLastButNear{};
		mapLastButNear.iPlayerId = 19693u;
		mapLastButNear.iNetEntityId = 19694u;
		mapLastButNear.iCurrentHp = mapLastButNear.iMaximumHp = 1000u;
		mapLastButNear.isCombatReady = true;
		mapLastButNear.fPositionX = 14.f;
		mapLastButNear.fPositionY = 2.f;
		mapLastButNear.fPositionZ = 20.f;
		players.emplace(mapLastButNear.iPlayerId, mapLastButNear);

		BOSS_PATTERN_DEFINITION pattern{};
		pattern.strEncounterId = "ENCOUNTER_VALTAN";
		pattern.strPatternId = "VALTAN_NEAREST_NULL_START_CONTRACT";
		pattern.strActionId = "valtan.contract.nearest-null-start";
		pattern.eTargetPolicy = BOSS_PATTERN_TARGET_POLICY::LOCK_NEAREST_ON_START;
		pattern.eAimPolicy = BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START;
		pattern.Finale.eKind = BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP;
		BOSS_PATTERN_STAGE_DEFINITION stage{};
		stage.strStageId = "ACTIVE";
		stage.strActionId = "valtan.contract.nearest-null-start.active";
		stage.eStageKind = BOSS_PATTERN_STAGE_KIND::ACTIVE;
		stage.iDurationMs = 1000u;
		pattern.Stages.push_back(std::move(stage));

		const bool restarted = brain.Restart_FinaleCycle(
			boss, players, pattern, 30u);
		tests.Require(
			restarted && 8u == boss.iPatternSequence &&
			mapLastButNear.iNetEntityId == boss.iPatternTargetEntityId &&
			boss.bHasPatternTargetLastPosition &&
			std::abs(boss.fPatternTargetLastPositionX - 14.f) < 0.001f &&
			std::abs(std::remainder(boss.fYawDegrees - 90.f, 360.f)) < 0.001f,
			"Recover a null pattern-start target as the nearest engageable player from the live boss position");
	}
	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		room.m_WorldEntities.clear();
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		tests.Require(activated && 1u == room.m_WorldEntities.size(),
			"Activate the real Valtan room for Charge tracking contracts");
		if (activated && 1u == room.m_WorldEntities.size())
		{
			SERVER_WORLD_ENTITY& boss = room.m_WorldEntities.front();
			boss.bIntroPatternConsumed = true;
			boss.bScriptedPatternPlayback = true;
			boss.iPhase = 2u;
			boss.fEngageDistance = 100.f;
			boss.iLastEvaluatedHealthBar =
				CValtanBrain::Calculate_HealthBar(boss);
			for (std::uint32_t ordinal = 0u; ordinal != 2u; ++ordinal)
			{
				SERVER_PLAYER player{};
				player.iPlayerId = 19700u + ordinal;
				player.iNetEntityId = 19800u + ordinal;
				player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
				player.iCurrentHp = player.iMaximumHp = 100000u;
				player.isCombatReady = true;
				room.m_PlayerIdByEntityId.emplace(
					player.iNetEntityId, player.iPlayerId);
				room.m_Players.emplace(player.iPlayerId, player);
			}
			const auto placePlayer = [&room, &boss](
				const PLAYER_ID playerId, const float offsetX,
				const float offsetZ)
			{
				SERVER_NAV_POINT point{};
				if (!room.m_ServerNavigation.Project_Point(
					boss.fPositionX + offsetX, boss.fPositionZ + offsetZ, point) ||
					!room.m_ServerNavigation.Is_PointWalkableExact(point.x, point.z))
				{
					return false;
				}
				SERVER_PLAYER& player = room.m_Players.at(playerId);
				player.fPositionX = point.x;
				player.fPositionY = point.y;
				player.fPositionZ = point.z;
				return true;
			};
			const auto yawToward = [&boss](const SERVER_PLAYER& player)
			{
				return std::atan2(player.fPositionX - boss.fPositionX,
					player.fPositionZ - boss.fPositionZ) *
					(180.f / 3.14159265358979323846f);
			};
			const auto sameYaw = [](const float actual, const float expected)
			{
				return std::isfinite(actual) && std::isfinite(expected) &&
					std::abs(std::remainder(actual - expected, 360.f)) < 0.001f;
			};
			const auto* chargePatterns =
				room.m_GameplayCatalog.Find_BossPatterns("ENCOUNTER_VALTAN");
			const BOSS_PATTERN_DEFINITION* chargeDefinition = nullptr;
			if (nullptr != chargePatterns)
			{
				const auto found = std::find_if(
					chargePatterns->begin(), chargePatterns->end(),
					[](const BOSS_PATTERN_DEFINITION& candidate)
					{ return "VALTAN_CHARGE" == candidate.strPatternId; });
				if (chargePatterns->end() != found)
					chargeDefinition = &*found;
			}
			const bool chargeTracksNearest = nullptr != chargeDefinition &&
				BOSS_PATTERN_TARGET_POLICY::NEAREST_EACH_TICK ==
					chargeDefinition->eTargetPolicy &&
				BOSS_PATTERN_AIM_POLICY::TRACK_TARGET_EACH_TICK ==
					chargeDefinition->eAimPolicy;

			bool positionsValid = placePlayer(19700u, -5.f, 0.f) &&
				placePlayer(19701u, 15.f, 0.f);
			float expectedYaw = yawToward(room.m_Players.at(19700u));
			boss.PendingPatternIds.push_back("VALTAN_CHARGE");
			room.Tick(1.f / 30.f);
			const bool enteredFacingLeft = room.Is_Ready() &&
				"VALTAN_CHARGE" == boss.strPatternId &&
				19800u == boss.iPatternTargetEntityId &&
				sameYaw(boss.fYawDegrees, expectedYaw);

			positionsValid = placePlayer(19700u, 5.f, 0.f) &&
				placePlayer(19701u, -15.f, 0.f) && positionsValid;
			expectedYaw = yawToward(room.m_Players.at(19700u));
			room.Tick(1.f / 30.f);
			const bool followedSameTargetRight = room.Is_Ready() &&
				19800u == boss.iPatternTargetEntityId &&
				sameYaw(boss.fYawDegrees, expectedYaw);

			positionsValid = placePlayer(19700u, 15.f, 0.f) &&
				placePlayer(19701u, -5.f, 0.f) && positionsValid;
			expectedYaw = yawToward(room.m_Players.at(19701u));
			room.Tick(1.f / 30.f);
			const bool switchedToNearerTargetLeft = room.Is_Ready() &&
				19801u == boss.iPatternTargetEntityId &&
				sameYaw(boss.fYawDegrees, expectedYaw);

			tests.Require(
				chargeTracksNearest && positionsValid && enteredFacingLeft &&
				followedSameTargetRight && switchedToNearerTargetLeft,
				"Track Charge yaw across left-right movement and switch its Server target id to the nearest player each tick");
		}
	}
	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto grabRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& grabRoom = *grabRoomStorage;
		SERVER_WORLD_ENTITY grabBoss{};
		grabBoss.iNetEntityId = 9100u;
		grabBoss.iPatternSequence = 1u;
		grabBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		grabBoss.eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
		grabBoss.strArchetypeId = "BOSS_VALTAN";
		grabBoss.strEncounterId = "ENCOUNTER_VALTAN";
		grabBoss.iCurrentHp = 1000u;
		grabBoss.iMaximumHp = 1000u;
		grabBoss.fPositionX = 0.f;
		grabBoss.fPositionY = 5.f;
		grabBoss.fPositionZ = 0.f;
		grabBoss.fYawDegrees = 0.f;
		grabRoom.m_WorldEntities.push_back(grabBoss);
		for (std::uint32_t ordinal = 0u; ordinal < 2u; ++ordinal)
		{
			SERVER_PLAYER player{};
			player.iPlayerId = 9200u + ordinal;
			player.iNetEntityId = 9300u + ordinal;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.iCurrentHp = 1000u;
			player.iMaximumHp = 1000u;
			player.iCurrentResource = 100u;
			player.iMaximumResource = 100u;
			player.isCombatReady = true;
			player.fPositionX = 1.f - 2.f * static_cast<float>(ordinal);
			player.fPositionY = 6.f + static_cast<float>(ordinal);
			player.fPositionZ = 2.f + static_cast<float>(ordinal);
			grabRoom.m_PlayerIdByEntityId.emplace(
				player.iNetEntityId, player.iPlayerId);
			grabRoom.m_Players.emplace(player.iPlayerId, player);
		}
		const bool capturedFirst = grabRoom.Capture_PlayerAttachment(
			9300u, 9100u, PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 10u);
		const bool capturedSecond = grabRoom.Capture_PlayerAttachment(
			9301u, 9100u, PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 10u);
		SERVER_WORLD_ENTITY& movingBoss = grabRoom.m_WorldEntities.front();
		movingBoss.fPositionX = 10.f;
		movingBoss.fPositionZ = 20.f;
		movingBoss.fYawDegrees = 90.f;
		SERVER_PLAYER& first = grabRoom.m_Players.at(9200u);
		SERVER_PLAYER& second = grabRoom.m_Players.at(9201u);
		const bool followedFirst =
			grabRoom.Update_PlayerAttachment(first, 11u);
		const bool followedSecond =
			grabRoom.Update_PlayerAttachment(second, 11u);
		tests.Require(
			capturedFirst && capturedSecond && followedFirst && followedSecond &&
			PLAYER_ACTION_STATE::GRABBED == first.eAction &&
			PLAYER_ACTION_STATE::GRABBED == second.eAction &&
			!first.isCombatReady && !second.isCombatReady &&
			(std::abs(first.fPositionX - second.fPositionX) > 0.001f ||
			 std::abs(first.fPositionZ - second.fPositionZ) > 0.001f),
			"Freeze two captured players and preserve their distinct boss-local offsets");
		const std::size_t released = grabRoom.Release_PlayerAttachments(
			9100u, 6.f, 500u, false, 0u, 12u);
		grabRoom.m_PlayerIdBySessionId.emplace(9400u, first.iPlayerId);
		C2S_MOVE blockedMove{};
		blockedMove.iClientSequence = 1u;
		blockedMove.fGoalX = 100.f;
		blockedMove.fGoalZ = 100.f;
		grabRoom.Handle_Move(9400u, blockedMove);
		C2S_USE_SKILL blockedSkill{};
		blockedSkill.iClientSequence = 1u;
		blockedSkill.iSkillId = 34010u;
		blockedSkill.fAimX = 1.f;
		blockedSkill.fAimZ = 0.f;
		grabRoom.Handle_UseSkill(9400u, blockedSkill);
		tests.Require(
			2u == released && PLAYER_ACTION_STATE::NONE == first.eAction &&
			PLAYER_ATTACHMENT_SLOT::NONE == first.eAttachmentSlot &&
			INVALID_NET_ENTITY_ID == first.iAttachmentOwnerNetEntityId &&
			std::abs(first.fKnockbackRemainingSeconds - 0.5f) < 0.000001f &&
			std::abs(first.fKnockbackSpeed - 12.f) < 0.000001f &&
			!first.hasMoveGoal && INVALID_SKILL_ID == first.iCurrentSkillId,
			"Release every captured player outward and reject move or skill overlap during knockback");
		first.fKnockbackRemainingSeconds = 0.f;
		first.fKnockbackSpeed = 0.f;
		first.isCombatReady = true;
		const bool recaptured = grabRoom.Capture_PlayerAttachment(
			first.iNetEntityId, 9100u,
			PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 13u);
		grabRoom.m_WorldEntities.clear();
		const bool keptMissingOwner =
			grabRoom.Update_PlayerAttachment(first, 14u);
		tests.Require(
			recaptured && !keptMissingOwner &&
			PLAYER_ACTION_STATE::NONE == first.eAction &&
			INVALID_NET_ENTITY_ID == first.iAttachmentOwnerNetEntityId &&
			PLAYER_ATTACHMENT_SLOT::NONE == first.eAttachmentSlot,
			"Release a captured player fail-closed when its boss owner disappears");
	}
}

