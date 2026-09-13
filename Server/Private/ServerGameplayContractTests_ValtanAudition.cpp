#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "EncounterPropRuntime.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "ServerNavigation.h"
#include "ValtanBrain.h"
#include "WorldBootstrap.h"
#include "WorldDestructionRuntime.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_ValtanAudition(TESTS& tests)
{


	{
		/* Debug Valtan audition. The point of the two-step ARM/CROSS contract is
		that dropping straight onto a low bar crosses every threshold above it,
		so this checks the single-crossing property against the real encounter
		patterns rather than a synthetic pair. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr SESSION_ID AUDITION_SESSION = 4242u;
		constexpr PLAYER_ID AUDITION_PLAYER = 77u;
		constexpr std::uint32_t TARGET_BAR = 109u;

		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* auditionBoss = room.Find_AuditionBoss();
		/* Drive one exact pattern: stage the encounter intro as already
		consumed so the first-appearance sweep is not the first sequence. */
		if (nullptr != auditionBoss)
			auditionBoss->bIntroPatternConsumed = true;
		tests.Require(
			activated && nullptr != auditionBoss &&
			600000u == (nullptr == auditionBoss ? 0u : auditionBoss->iMaximumHp) &&
			160u == (nullptr == auditionBoss ?
				0u : auditionBoss->iMaximumHealthBars),
			"Activate the audition Valtan with its authored health bar scale");

		tests.Require(
			nullptr != auditionBoss &&
			303750u == CValtanBrain::Resolve_HealthBarHp(*auditionBoss, 81u) &&
			300000u == CValtanBrain::Resolve_HealthBarHp(*auditionBoss, 80u) &&
			0u == CValtanBrain::Resolve_HealthBarHp(*auditionBoss, 161u),
			"Resolve authored health bar boundaries and reject bars off the scale");

		C2S_VALTAN_AUDITION_REQUEST arm{};
		arm.iRequestSequence = 1u;
		arm.eOperation = VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
		arm.iTargetHealthBar = TARGET_BAR;
		std::uint32_t reportedBar = 0u;

		SERVER_PLAYER auditionPlayer{};
		auditionPlayer.iPlayerId = AUDITION_PLAYER;
		auditionPlayer.iNetEntityId = 900u;
		auditionPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		auditionPlayer.iCurrentHp = 1000u;
		auditionPlayer.iMaximumHp = 1000u;
		/* SERVER_PLAYER starts combat-ready, so the not-engaged path has to be
		asked for explicitly rather than left to the default. */
		auditionPlayer.isCombatReady = false;
		if (nullptr != auditionBoss)
		{
			auditionPlayer.fPositionX = auditionBoss->fPositionX + 2.f;
			auditionPlayer.fPositionY = auditionBoss->fPositionY;
			auditionPlayer.fPositionZ = auditionBoss->fPositionZ;
		}
		room.m_Players.emplace(AUDITION_PLAYER, auditionPlayer);

#ifndef _DEBUG
		/* A Release Server never auditions. It still answers, because the packet
		type stays known so a Debug Client gets a verdict instead of a closed
		socket, but the boss must not move even for an otherwise valid request. */
		room.m_PlayerIdBySessionId.emplace(AUDITION_SESSION, AUDITION_PLAYER);
		room.m_Players.at(AUDITION_PLAYER).isCombatReady = true;
		const std::uint32_t releaseHpBefore =
			nullptr == auditionBoss ? 0u : auditionBoss->iCurrentHp;
		reportedBar = 12345u;
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_RELEASE_BUILD ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, arm, reportedBar) &&
			0u == reportedBar &&
			nullptr != auditionBoss &&
			releaseHpBefore ==
				(nullptr == auditionBoss ? 1u : auditionBoss->iCurrentHp),
			"Reject every Valtan audition in a Release Server without moving the boss");
		for (const auto operation : { VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID,
			VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID,
			VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID })
		{
			C2S_VALTAN_AUDITION_REQUEST next{};
			next.iRequestSequence = 2u;
			next.eOperation = operation;
			next.strBossPlacementId = "boss.valtan.center";
			next.strPatternId = "VALTAN_FIST_IN_OUT";
			const bool live = VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID == operation;
			next.iPredecessorRoomAuditionEpoch = live ? 0u : 1u;
			next.iPredecessorPatternSequence = live ? 0u : 1u;
			next.iExpectedNextRequestSequence = live ? 0u : 1u;
			tests.Require(VALTAN_AUDITION_RESULT::REJECTED_RELEASE_BUILD ==
				room.Evaluate_ValtanAudition(AUDITION_SESSION, next, reportedBar) &&
				0u == reportedBar && nullptr != auditionBoss &&
				releaseHpBefore == auditionBoss->iCurrentHp && auditionBoss->PendingPatternIds.empty(),
				"Release rejects Next queue and clear without touching gameplay");
		}
#else
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE ==
				room.Evaluate_ValtanAudition(AUDITION_SESSION, arm, reportedBar),
			"Reject retired Valtan Level audition commands before session lookup");
		room.m_PlayerIdBySessionId.emplace(AUDITION_SESSION, AUDITION_PLAYER);
		room.m_Players.at(AUDITION_PLAYER).isCombatReady = true;

		const std::vector<WORLD_DESTRUCTION_GROUP_STATE>
			legacyDestructionBefore =
				room.m_WorldDestructionRuntime.Get_GroupStates();
		const std::vector<ENCOUNTER_PROP_SLOT_STATE> legacyPropsBefore =
			room.m_EncounterPropRuntime.Get_SlotStates();
		const std::uint32_t legacyDestructionEpochBefore =
			room.m_WorldDestructionRuntime.Get_EncounterEpoch();
		const std::uint64_t legacyNavigationRevisionBefore =
			room.m_ServerNavigation.Get_Revision();
		const std::uint64_t legacyCollisionRevisionBefore =
			room.m_ServerCollisionSystem.Get_Revision();
		const std::uint32_t legacyBossHpBefore =
			nullptr == auditionBoss ? 0u : auditionBoss->iCurrentHp;
		const std::uint32_t legacyBossBarBefore =
			nullptr == auditionBoss ? 0u : auditionBoss->iLastEvaluatedHealthBar;
		const std::uint32_t legacyBossSequenceBefore =
			nullptr == auditionBoss ? 0u : auditionBoss->iPatternSequence;
		const SERVER_ENTITY_ACTION legacyBossActionBefore =
			nullptr == auditionBoss ? SERVER_ENTITY_ACTION::IDLE :
				auditionBoss->eAction;
		const float legacyBossPositionXBefore =
			nullptr == auditionBoss ? 0.f : auditionBoss->fPositionX;
		const float legacyBossPositionYBefore =
			nullptr == auditionBoss ? 0.f : auditionBoss->fPositionY;
		const float legacyBossPositionZBefore =
			nullptr == auditionBoss ? 0.f : auditionBoss->fPositionZ;
		const float legacyBossYawBefore =
			nullptr == auditionBoss ? 0.f : auditionBoss->fYawDegrees;
		const std::string legacyBossPatternBefore =
			nullptr == auditionBoss ? std::string{} : auditionBoss->strPatternId;
		const std::string legacyBossStageBefore =
			nullptr == auditionBoss ? std::string{} : auditionBoss->strPatternStageId;
		const std::vector<std::string> legacyPendingBefore =
			nullptr == auditionBoss ? std::vector<std::string>{} :
				auditionBoss->PendingPatternIds;
		const std::vector<std::string> legacyTriggeredBefore =
			nullptr == auditionBoss ? std::vector<std::string>{} :
				auditionBoss->TriggeredPatternIds;
		const SERVER_PLAYER legacyPlayerBefore =
			room.m_Players.at(AUDITION_PLAYER);
		const auto legacyTimelineBefore = room.m_ValtanTimelineAudition;
		const auto legacyFightPageBefore = room.m_ValtanFightPageStart;
		const std::uint32_t legacyArmedBarBefore =
			room.m_iValtanAuditionArmedHealthBar;
		const std::size_t legacyReceiptCountBefore =
			room.m_ValtanAuditionSequenceBySessionId.size();
		const std::size_t legacyPatternReceiptCountBefore =
			room.m_ValtanPatternIdAuditionSequenceBySessionId.size();
		const std::uint32_t legacyAuditionEpochBefore =
			room.m_iNextValtanAuditionEpoch;
		const std::size_t legacyLifecycleCountBefore =
			room.m_PendingValtanAuditionLifecycle.size();
		const std::uint32_t legacyPillarBreakTickBefore =
			room.m_iPillarAuditionBreakTick;
		const bool legacyPillarCycleBefore =
			room.m_bPillarAuditionCycleArmed;
		const bool legacyAutomaticOverrideBefore =
			nullptr != auditionBoss &&
			auditionBoss->bAutomaticPatternSequenceAuditionOverride;
		const bool legacyAutomaticHoldBefore =
			nullptr != auditionBoss &&
			auditionBoss->bAutomaticPatternSequenceAuditionHold;
		const std::array<VALTAN_AUDITION_OPERATION, 12u> retiredOperations{
			VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR,
			VALTAN_AUDITION_OPERATION::CROSS_HEALTH_BAR,
			VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR,
			VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE,
			VALTAN_AUDITION_OPERATION::PLAY_PILLAR_CYCLE,
			VALTAN_AUDITION_OPERATION::PLAY_WALL_ATTACK,
			VALTAN_AUDITION_OPERATION::SHOW_FINAL_ARENA,
			VALTAN_AUDITION_OPERATION::BREAK_EVERY_WALL,
			VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW,
			VALTAN_AUDITION_OPERATION::STOP_TIMELINE_ROW,
			VALTAN_AUDITION_OPERATION::PLAY_PATTERN,
			VALTAN_AUDITION_OPERATION::START_FIGHT_PAGE };
		bool allRetiredRejected = true;
		for (std::size_t index = 0u; index < retiredOperations.size(); ++index)
		{
			C2S_VALTAN_AUDITION_REQUEST retired{};
			retired.iRequestSequence =
				1u + static_cast<std::uint32_t>(index);
			retired.eOperation = retiredOperations[index];
			retired.iTargetHealthBar = 109u;
			reportedBar = 999u;
			const VALTAN_AUDITION_RESULT result =
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, retired, reportedBar);
			allRetiredRejected =
				VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE == result &&
				0u == reportedBar && allRetiredRejected;
		}
		const auto sameLegacyDestruction = [](
			const std::vector<WORLD_DESTRUCTION_GROUP_STATE>& leftStates,
			const std::vector<WORLD_DESTRUCTION_GROUP_STATE>& rightStates)
		{
			return leftStates.size() == rightStates.size() &&
				std::equal(
					leftStates.begin(), leftStates.end(), rightStates.begin(),
					[](const WORLD_DESTRUCTION_GROUP_STATE& left,
						const WORLD_DESTRUCTION_GROUP_STATE& right)
					{
						return left.strGroupId == right.strGroupId &&
							left.eState == right.eState &&
							left.iStateVersion == right.iStateVersion &&
							left.iStateStartTick == right.iStateStartTick &&
							left.iCommitTick == right.iCommitTick &&
							left.strPendingMutationId == right.strPendingMutationId;
					});
		};
		const auto sameLegacyProps = [](
			const std::vector<ENCOUNTER_PROP_SLOT_STATE>& leftStates,
			const std::vector<ENCOUNTER_PROP_SLOT_STATE>& rightStates)
		{
			return leftStates.size() == rightStates.size() &&
				std::equal(
					leftStates.begin(), leftStates.end(), rightStates.begin(),
					[](const ENCOUNTER_PROP_SLOT_STATE& left,
						const ENCOUNTER_PROP_SLOT_STATE& right)
					{
						return left.strSlotId == right.strSlotId &&
							left.eState == right.eState &&
							left.iStateVersion == right.iStateVersion &&
							left.iStateStartTick == right.iStateStartTick &&
							left.iOccurrenceSequence == right.iOccurrenceSequence &&
							left.fPositionX == right.fPositionX &&
							left.fPositionZ == right.fPositionZ;
					});
		};
		auditionBoss = room.Find_AuditionBoss();
		const SERVER_PLAYER& legacyPlayerAfter =
			room.m_Players.at(AUDITION_PLAYER);
		tests.Require(
			allRetiredRejected && nullptr != auditionBoss &&
			legacyBossHpBefore == auditionBoss->iCurrentHp &&
			legacyBossBarBefore == auditionBoss->iLastEvaluatedHealthBar &&
			legacyBossSequenceBefore == auditionBoss->iPatternSequence &&
			legacyBossActionBefore == auditionBoss->eAction &&
			legacyBossPositionXBefore == auditionBoss->fPositionX &&
			legacyBossPositionYBefore == auditionBoss->fPositionY &&
			legacyBossPositionZBefore == auditionBoss->fPositionZ &&
			legacyBossYawBefore == auditionBoss->fYawDegrees &&
			legacyBossPatternBefore == auditionBoss->strPatternId &&
			legacyBossStageBefore == auditionBoss->strPatternStageId &&
			legacyPendingBefore == auditionBoss->PendingPatternIds &&
			legacyTriggeredBefore == auditionBoss->TriggeredPatternIds &&
			legacyAutomaticOverrideBefore ==
				auditionBoss->bAutomaticPatternSequenceAuditionOverride &&
			legacyAutomaticHoldBefore ==
				auditionBoss->bAutomaticPatternSequenceAuditionHold &&
			legacyPlayerBefore.iCurrentHp == legacyPlayerAfter.iCurrentHp &&
			legacyPlayerBefore.isCombatReady == legacyPlayerAfter.isCombatReady &&
			legacyPlayerBefore.fPositionX == legacyPlayerAfter.fPositionX &&
			legacyPlayerBefore.fPositionY == legacyPlayerAfter.fPositionY &&
			legacyPlayerBefore.fPositionZ == legacyPlayerAfter.fPositionZ &&
			legacyPlayerBefore.fYawDegrees == legacyPlayerAfter.fYawDegrees &&
			legacyPlayerBefore.eAction == legacyPlayerAfter.eAction &&
			legacyPlayerBefore.iCurrentSkillId == legacyPlayerAfter.iCurrentSkillId &&
			legacyPlayerBefore.iActionStartTick == legacyPlayerAfter.iActionStartTick &&
			legacyPlayerBefore.iAttachmentOwnerNetEntityId ==
				legacyPlayerAfter.iAttachmentOwnerNetEntityId &&
			sameLegacyDestruction(
				legacyDestructionBefore,
				room.m_WorldDestructionRuntime.Get_GroupStates()) &&
			sameLegacyProps(
				legacyPropsBefore,
				room.m_EncounterPropRuntime.Get_SlotStates()) &&
			legacyDestructionEpochBefore ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			legacyNavigationRevisionBefore ==
				room.m_ServerNavigation.Get_Revision() &&
			legacyCollisionRevisionBefore ==
				room.m_ServerCollisionSystem.Get_Revision() &&
			legacyArmedBarBefore == room.m_iValtanAuditionArmedHealthBar &&
			legacyReceiptCountBefore ==
				room.m_ValtanAuditionSequenceBySessionId.size() &&
			legacyPatternReceiptCountBefore ==
				room.m_ValtanPatternIdAuditionSequenceBySessionId.size() &&
			legacyAuditionEpochBefore == room.m_iNextValtanAuditionEpoch &&
			legacyLifecycleCountBefore ==
				room.m_PendingValtanAuditionLifecycle.size() &&
			legacyPillarBreakTickBefore == room.m_iPillarAuditionBreakTick &&
			legacyPillarCycleBefore == room.m_bPillarAuditionCycleArmed &&
			legacyTimelineBefore.ePhase == room.m_ValtanTimelineAudition.ePhase &&
			legacyTimelineBefore.iOwnerSessionId ==
				room.m_ValtanTimelineAudition.iOwnerSessionId &&
			legacyTimelineBefore.iOwnerPlayerId ==
				room.m_ValtanTimelineAudition.iOwnerPlayerId &&
			legacyTimelineBefore.iBossEntityId ==
				room.m_ValtanTimelineAudition.iBossEntityId &&
			legacyTimelineBefore.iRowIndex ==
				room.m_ValtanTimelineAudition.iRowIndex &&
			legacyTimelineBefore.iActionIndex ==
				room.m_ValtanTimelineAudition.iActionIndex &&
			legacyTimelineBefore.iRepeatIndex ==
				room.m_ValtanTimelineAudition.iRepeatIndex &&
			legacyTimelineBefore.iExpectedPatternSequence ==
				room.m_ValtanTimelineAudition.iExpectedPatternSequence &&
			legacyTimelineBefore.iEnvironmentDeadlineTick ==
				room.m_ValtanTimelineAudition.iEnvironmentDeadlineTick &&
			legacyTimelineBefore.iHeldBossHp ==
				room.m_ValtanTimelineAudition.iHeldBossHp &&
			legacyTimelineBefore.iHeldBossHealthBar ==
				room.m_ValtanTimelineAudition.iHeldBossHealthBar &&
			legacyTimelineBefore.bAllowProductPropBreak ==
				room.m_ValtanTimelineAudition.bAllowProductPropBreak &&
			legacyTimelineBefore.PinnedDefinitionRevision ==
				room.m_ValtanTimelineAudition.PinnedDefinitionRevision &&
			legacyTimelineBefore.strExpectedPatternId ==
				room.m_ValtanTimelineAudition.strExpectedPatternId &&
			legacyTimelineBefore.ExpectedGoneGroupIds ==
				room.m_ValtanTimelineAudition.ExpectedGoneGroupIds &&
			legacyFightPageBefore.iBossEntityId ==
				room.m_ValtanFightPageStart.iBossEntityId &&
			legacyFightPageBefore.iCommandId ==
				room.m_ValtanFightPageStart.iCommandId &&
			legacyFightPageBefore.iEnvironmentDeadlineTick ==
				room.m_ValtanFightPageStart.iEnvironmentDeadlineTick &&
			legacyFightPageBefore.ExpectedGoneGroupIds ==
				room.m_ValtanFightPageStart.ExpectedGoneGroupIds,
			"Reject every revision-unaware Valtan audition operation before mutating boss, player, arena, receipts, or lifecycle state");
		struct ARENA_PRESET_EXPECTATION final
		{
			VALTAN_ARENA_PRESET ePreset = VALTAN_ARENA_PRESET::END;
			std::size_t iDespawned = 0u;
			std::size_t iIntactFloor84 = 0u;
			std::size_t iIntactFloor30 = 0u;
			const char* pContract = nullptr;
		};
		const std::array<ARENA_PRESET_EXPECTATION, 5u> presetExpectations{
			ARENA_PRESET_EXPECTATION{
				VALTAN_ARENA_PRESET::FRESH, 0u, 3u, 3u,
				"Reset the Server arena preset to all walls and floor groups intact" },
			ARENA_PRESET_EXPECTATION{
				VALTAN_ARENA_PRESET::CIRCLE_WALLS_GONE, 99u, 3u, 3u,
				"Stage the circular Server arena with every wall gone and every floor group intact" },
			ARENA_PRESET_EXPECTATION{
				VALTAN_ARENA_PRESET::THREE_OCLOCK_BROKEN, 102u, 0u, 3u,
				"Stage only the 3 o'clock floor break after opening the Server arena" },
			ARENA_PRESET_EXPECTATION{
				VALTAN_ARENA_PRESET::NINE_OCLOCK_BROKEN, 102u, 3u, 0u,
				"Stage only the 9 o'clock floor break without requiring the 3 o'clock chronology" },
			ARENA_PRESET_EXPECTATION{
				VALTAN_ARENA_PRESET::BOTH_SIDES_BROKEN, 105u, 0u, 0u,
				"Stage both independent floor breaks through one Server-authoritative preset" }
		};
		for (std::size_t index = 0u;
			index < presetExpectations.size(); ++index)
		{
			const ARENA_PRESET_EXPECTATION& expected =
				presetExpectations[index];
			C2S_VALTAN_AUDITION_REQUEST presetRequest{};
			presetRequest.iRequestSequence =
				12u + static_cast<std::uint32_t>(index);
			presetRequest.eOperation =
				VALTAN_AUDITION_OPERATION::SET_ARENA_PRESET;
			presetRequest.iTargetHealthBar =
				static_cast<std::uint32_t>(expected.ePreset);
			const bool acceptedPreset =
				VALTAN_AUDITION_RESULT::QUEUED ==
					room.Evaluate_ValtanAudition(
						AUDITION_SESSION, presetRequest, reportedBar);
			for (std::uint32_t tick = 0u; tick < 9u; ++tick)
				room.Tick(1.f / 30.f);

			std::size_t despawned = 0u;
			std::size_t intactFloor84 = 0u;
			std::size_t intactFloor30 = 0u;
			const std::vector<WORLD_DESTRUCTION_GROUP_STATE> states =
				room.m_WorldDestructionRuntime.Get_GroupStates();
			for (const WORLD_DESTRUCTION_GROUP_STATE& state : states)
			{
				if (WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState)
					++despawned;
				if (0u == state.strGroupId.rfind(
						"destroyable.group.valtan.floor84.", 0u) &&
					WORLD_DESTRUCTION_STATE::INTACT == state.eState)
				{
					++intactFloor84;
				}
				if (0u == state.strGroupId.rfind(
						"destroyable.group.valtan.floor30.", 0u) &&
					WORLD_DESTRUCTION_STATE::INTACT == state.eState)
				{
					++intactFloor30;
				}
			}
			tests.Require(
				acceptedPreset && 105u == states.size() &&
				expected.iDespawned == despawned &&
				expected.iIntactFloor84 == intactFloor84 &&
				expected.iIntactFloor30 == intactFloor30 &&
				nullptr != auditionBoss &&
				auditionBoss->bAutomaticPatternSequenceAuditionHold &&
				auditionBoss->PendingPatternIds.empty(),
				expected.pContract);
		}

		/* Complete Play is a boss restart, not an implicit Fresh-arena preset.
		   Keep the exact final-arena, prop, collision and navigation state while
		   replacing only the boss-owned occurrence. */
		const std::vector<WORLD_DESTRUCTION_GROUP_STATE>
			completePlayDestructionBefore =
				room.m_WorldDestructionRuntime.Get_GroupStates();
		const std::vector<ENCOUNTER_PROP_SLOT_STATE>
			completePlayPropsBefore =
				room.m_EncounterPropRuntime.Get_SlotStates();
		const std::uint32_t completePlayEncounterEpochBefore =
			room.m_WorldDestructionRuntime.Get_EncounterEpoch();
		const std::uint64_t completePlayNavigationRevisionBefore =
			room.m_ServerNavigation.Get_Revision();
		const std::uint64_t completePlayCollisionRevisionBefore =
			room.m_ServerCollisionSystem.Get_Revision();
		room.m_iPillarAuditionBreakTick = 999999u;
		room.m_bPillarAuditionCycleArmed = true;

		C2S_VALTAN_AUDITION_REQUEST completePlay{};
		completePlay.iRequestSequence = 1u;
		completePlay.eOperation =
			VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
		completePlay.strBossPlacementId = "boss.valtan.center";
		completePlay.strPatternId = "VALTAN_FIST_IN_OUT";
		completePlay.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		const VALTAN_AUDITION_RESULT completePlayResult =
			room.Evaluate_ValtanAudition(
				AUDITION_SESSION, completePlay, reportedBar);

		const std::vector<WORLD_DESTRUCTION_GROUP_STATE>
			completePlayDestructionAfter =
				room.m_WorldDestructionRuntime.Get_GroupStates();
		const std::vector<ENCOUNTER_PROP_SLOT_STATE>
			completePlayPropsAfter =
				room.m_EncounterPropRuntime.Get_SlotStates();
		const auto sameDestructionState = [](
			const std::vector<WORLD_DESTRUCTION_GROUP_STATE>& leftStates,
			const std::vector<WORLD_DESTRUCTION_GROUP_STATE>& rightStates)
		{
			return leftStates.size() == rightStates.size() &&
				std::equal(
				leftStates.begin(), leftStates.end(), rightStates.begin(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& left,
					const WORLD_DESTRUCTION_GROUP_STATE& right)
				{
					return left.strGroupId == right.strGroupId &&
						left.eState == right.eState &&
						left.iStateVersion == right.iStateVersion &&
						left.iStateStartTick == right.iStateStartTick &&
						left.iCommitTick == right.iCommitTick &&
						left.strPendingMutationId ==
							right.strPendingMutationId;
				});
		};
		const auto samePropState = [](
			const std::vector<ENCOUNTER_PROP_SLOT_STATE>& leftStates,
			const std::vector<ENCOUNTER_PROP_SLOT_STATE>& rightStates)
		{
			return leftStates.size() == rightStates.size() &&
				std::equal(
				leftStates.begin(), leftStates.end(), rightStates.begin(),
				[](const ENCOUNTER_PROP_SLOT_STATE& left,
					const ENCOUNTER_PROP_SLOT_STATE& right)
				{
					return left.strSlotId == right.strSlotId &&
						left.eState == right.eState &&
						left.iStateVersion == right.iStateVersion &&
						left.iStateStartTick == right.iStateStartTick &&
						left.iOccurrenceSequence ==
							right.iOccurrenceSequence &&
						left.fPositionX == right.fPositionX &&
						left.fPositionZ == right.fPositionZ;
				});
		};
		const bool sameDestruction = sameDestructionState(
			completePlayDestructionBefore, completePlayDestructionAfter);
		const bool sameProps = samePropState(
			completePlayPropsBefore, completePlayPropsAfter);
		auditionBoss = room.Find_AuditionBoss();
		tests.Require(
			VALTAN_AUDITION_RESULT::QUEUED == completePlayResult &&
			sameDestruction && sameProps &&
			completePlayEncounterEpochBefore ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			completePlayNavigationRevisionBefore ==
				room.m_ServerNavigation.Get_Revision() &&
			completePlayCollisionRevisionBefore ==
				room.m_ServerCollisionSystem.Get_Revision() &&
			nullptr != auditionBoss &&
			1u == auditionBoss->PendingPatternIds.size() &&
			"VALTAN_FIST_IN_OUT" ==
				auditionBoss->PendingPatternIds.front() &&
			0u == room.m_iPillarAuditionBreakTick &&
			!room.m_bPillarAuditionCycleArmed,
			"Complete Play preserves the selected Server arena, Debris, collision and Nav while restarting only the boss pattern");

		room.Tick(1.f / 30.f);
		auditionBoss = room.Find_AuditionBoss();
		const std::vector<WORLD_DESTRUCTION_GROUP_STATE>
			completePlayDestructionStarted =
				room.m_WorldDestructionRuntime.Get_GroupStates();
		const std::vector<ENCOUNTER_PROP_SLOT_STATE>
			completePlayPropsStarted =
				room.m_EncounterPropRuntime.Get_SlotStates();
		tests.Require(
			nullptr != auditionBoss &&
			"VALTAN_FIST_IN_OUT" == auditionBoss->strPatternId &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
				room.m_ValtanPatternIdAudition.ePhase &&
			sameDestructionState(
				completePlayDestructionBefore,
				completePlayDestructionStarted) &&
			samePropState(completePlayPropsBefore, completePlayPropsStarted) &&
			completePlayEncounterEpochBefore ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			completePlayNavigationRevisionBefore ==
				room.m_ServerNavigation.Get_Revision() &&
			completePlayCollisionRevisionBefore ==
				room.m_ServerCollisionSystem.Get_Revision(),
			"Complete Play starts the selected Server pattern on the first room tick without changing the chosen arena");

		/* Saving and activating a new canonical gameplay Product must not rewrite
		   the live predecessor pin. Restart carries the old occurrence revision and
		   the newly active replacement revision as two independent CAS values. */
		const GameplayDataRevision predecessorDefinitionRevision =
			room.m_ValtanPatternIdAudition.PinnedDefinitionRevision;
		GameplayDataRevision replacementDefinitionRevision =
			predecessorDefinitionRevision;
		replacementDefinitionRevision.Bytes[30] ^= 0x6du;
		std::vector<wchar_t> restartPathBuffer(32768u);
		std::filesystem::path restartDataRoot;
		const DWORD restartConfigured = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", restartPathBuffer.data(),
			static_cast<DWORD>(restartPathBuffer.size()));
		if (restartConfigured > 0u &&
			restartConfigured < restartPathBuffer.size())
		{
			restartDataRoot = restartPathBuffer.data();
		}
		else if (GetModuleFileNameW(
			nullptr, restartPathBuffer.data(),
			static_cast<DWORD>(restartPathBuffer.size())) > 0u)
		{
			restartDataRoot = std::filesystem::path(restartPathBuffer.data()).
				parent_path().parent_path() / L"DataFiles";
		}
		std::error_code restartPathError;
		const auto restartBootstrapPath = std::filesystem::canonical(
			restartDataRoot / L"Gameplay" / L"Gameplay.bootstrap",
			restartPathError);
		auto replacementGeneration = std::make_shared<CGameplayCatalog>();
		std::string replacementActivationStatus;
		const bool replacementActivated = !restartPathError &&
			replacementGeneration->Load_FromBootstrap(
				restartBootstrapPath, predecessorDefinitionRevision,
				replacementDefinitionRevision) &&
			room.Stage_GameplayGeneration(
				7001u, predecessorDefinitionRevision, replacementGeneration,
				replacementActivationStatus) &&
			room.Commit_GameplayGeneration(7001u) &&
			predecessorDefinitionRevision ==
				room.m_ValtanPatternIdAudition.PinnedDefinitionRevision &&
			replacementDefinitionRevision ==
				room.m_GameplayCatalog.Get_ActiveRevision();

		const std::uint32_t activePatternSequenceBeforeRestart =
			nullptr == auditionBoss ? 0u : auditionBoss->iPatternSequence;
		const std::uint32_t activeRoomEpochBeforeRestart =
			room.m_ValtanPatternIdAudition.iRoomAuditionEpoch;
		C2S_VALTAN_AUDITION_REQUEST restartPattern = completePlay;
		restartPattern.iRequestSequence = 2u;
		restartPattern.eOperation =
			VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID;
		restartPattern.iPredecessorRoomAuditionEpoch =
			activeRoomEpochBeforeRestart;
		restartPattern.iPredecessorPatternSequence =
			activePatternSequenceBeforeRestart;
		restartPattern.ExpectedDefinitionRevision =
			room.m_ValtanPatternIdAudition.PinnedDefinitionRevision;
		restartPattern.ReplacementDefinitionRevision =
			replacementDefinitionRevision;
		const VALTAN_AUDITION_RESULT restartPatternResult =
			room.Evaluate_ValtanAudition(
				AUDITION_SESSION, restartPattern, reportedBar);
		auditionBoss = room.Find_AuditionBoss();
		tests.Require(
			replacementActivated &&
			VALTAN_AUDITION_RESULT::QUEUED == restartPatternResult &&
			nullptr != auditionBoss &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING ==
				room.m_ValtanPatternIdAudition.ePhase &&
			AUDITION_SESSION ==
				room.m_ValtanPatternIdAudition.iOwnerSessionId &&
			2u == room.m_ValtanPatternIdAudition.iRequestSequence &&
			activeRoomEpochBeforeRestart !=
				room.m_ValtanPatternIdAudition.iRoomAuditionEpoch &&
			activePatternSequenceBeforeRestart + 1u ==
				room.m_ValtanPatternIdAudition.iExpectedPatternSequence &&
			replacementDefinitionRevision ==
				room.m_ValtanPatternIdAudition.PinnedDefinitionRevision &&
			replacementDefinitionRevision ==
				auditionBoss->PinnedDefinitionRevision &&
			1u == auditionBoss->PendingPatternIds.size() &&
			"VALTAN_FIST_IN_OUT" == auditionBoss->PendingPatternIds.front() &&
			sameDestructionState(
				completePlayDestructionBefore,
				room.m_WorldDestructionRuntime.Get_GroupStates()) &&
			samePropState(
				completePlayPropsBefore,
				room.m_EncounterPropRuntime.Get_SlotStates()) &&
			completePlayEncounterEpochBefore ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			completePlayNavigationRevisionBefore ==
				room.m_ServerNavigation.Get_Revision() &&
			completePlayCollisionRevisionBefore ==
				room.m_ServerCollisionSystem.Get_Revision(),
			"Restart the exact old occurrence into the newly activated canonical gameplay revision while preserving the arena");

		room.Tick(1.f / 30.f);
		auditionBoss = room.Find_AuditionBoss();
		tests.Require(
			nullptr != auditionBoss &&
			"VALTAN_FIST_IN_OUT" == auditionBoss->strPatternId &&
			activePatternSequenceBeforeRestart + 1u ==
				auditionBoss->iPatternSequence &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
				room.m_ValtanPatternIdAudition.ePhase &&
			sameDestructionState(
				completePlayDestructionBefore,
				room.m_WorldDestructionRuntime.Get_GroupStates()) &&
			samePropState(
				completePlayPropsBefore,
				room.m_EncounterPropRuntime.Get_SlotStates()) &&
			completePlayEncounterEpochBefore ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			completePlayNavigationRevisionBefore ==
				room.m_ServerNavigation.Get_Revision() &&
			completePlayCollisionRevisionBefore ==
				room.m_ServerCollisionSystem.Get_Revision(),
			"The restarted occurrence becomes ACTIVE on the next fixed tick without refreshing walls, props, collision, or Nav");

		/* A new request identity does not make an old predecessor current. Both
		   sequence and revision mismatches must reject before boss/player state. */
		const std::uint32_t restartedEpoch =
			room.m_ValtanPatternIdAudition.iRoomAuditionEpoch;
		const std::uint32_t restartedSequence =
			room.m_ValtanPatternIdAudition.iExpectedPatternSequence;
		const GameplayDataRevision restartedRevision =
			room.m_ValtanPatternIdAudition.PinnedDefinitionRevision;
		const VALTAN_AUDITION_RESULT exactRestartRetryResult =
			room.Evaluate_ValtanAudition(
				AUDITION_SESSION, restartPattern, reportedBar);
		C2S_VALTAN_AUDITION_REQUEST alteredRestartRetry = restartPattern;
		alteredRestartRetry.strPatternId = "VALTAN_WHIRLWIND";
		const VALTAN_AUDITION_RESULT alteredRestartRetryResult =
			room.Evaluate_ValtanAudition(
				AUDITION_SESSION, alteredRestartRetry, reportedBar);
		C2S_VALTAN_AUDITION_REQUEST staleRestart = restartPattern;
		staleRestart.iRequestSequence = 3u;
		const VALTAN_AUDITION_RESULT staleRestartResult =
			room.Evaluate_ValtanAudition(
				AUDITION_SESSION, staleRestart, reportedBar);
		C2S_VALTAN_AUDITION_REQUEST wrongRevisionRestart = restartPattern;
		wrongRevisionRestart.iRequestSequence = 4u;
		wrongRevisionRestart.iPredecessorRoomAuditionEpoch = restartedEpoch;
		wrongRevisionRestart.iPredecessorPatternSequence = restartedSequence;
		wrongRevisionRestart.ExpectedDefinitionRevision = restartedRevision;
		wrongRevisionRestart.ExpectedDefinitionRevision.Bytes[0] ^= 0x5au;
		const VALTAN_AUDITION_RESULT wrongRevisionResult =
			room.Evaluate_ValtanAudition(
				AUDITION_SESSION, wrongRevisionRestart, reportedBar);
		C2S_VALTAN_AUDITION_REQUEST wrongReplacementRestart = restartPattern;
		wrongReplacementRestart.iRequestSequence = 5u;
		wrongReplacementRestart.iPredecessorRoomAuditionEpoch = restartedEpoch;
		wrongReplacementRestart.iPredecessorPatternSequence = restartedSequence;
		wrongReplacementRestart.ExpectedDefinitionRevision = restartedRevision;
		wrongReplacementRestart.ReplacementDefinitionRevision.Bytes[0] ^=
			0xa5u;
		const VALTAN_AUDITION_RESULT wrongReplacementResult =
			room.Evaluate_ValtanAudition(
				AUDITION_SESSION, wrongReplacementRestart, reportedBar);
		auditionBoss = room.Find_AuditionBoss();
		tests.Require(
			VALTAN_AUDITION_RESULT::QUEUED == exactRestartRetryResult &&
			VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST ==
				alteredRestartRetryResult &&
			VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION ==
				staleRestartResult &&
			VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION ==
				wrongRevisionResult &&
			VALTAN_AUDITION_RESULT::REJECTED_OCCURRENCE_PRESERVED ==
				wrongReplacementResult &&
			nullptr != auditionBoss &&
			"VALTAN_FIST_IN_OUT" == auditionBoss->strPatternId &&
			restartedSequence == auditionBoss->iPatternSequence &&
			restartedEpoch == room.m_ValtanPatternIdAudition.iRoomAuditionEpoch &&
			restartedRevision ==
				room.m_ValtanPatternIdAudition.PinnedDefinitionRevision,
			"Reject stale predecessor or inactive replacement Restart CAS without replacing the successor occurrence");

		/* Force the canonical reset preflight to fail after bait staging. The live
		   player's pose, movement and combat-ready state must remain byte-for-byte
		   equivalent for every field the bait helper can change. */
		const auto sameBaitMutablePlayerState = [](
			const SERVER_PLAYER& left, const SERVER_PLAYER& right)
		{
			const bool samePath = left.MovePath.size() == right.MovePath.size() &&
				std::equal(left.MovePath.begin(), left.MovePath.end(),
					right.MovePath.begin(), [](const SERVER_NAV_POINT& a,
						const SERVER_NAV_POINT& b)
					{
						return a.x == b.x && a.y == b.y && a.z == b.z;
					});
			const SERVER_TRIGGER_MOVE& a = left.TriggerMove;
			const SERVER_TRIGGER_MOVE& b = right.TriggerMove;
			const bool sameTrigger = a.fStartX == b.fStartX &&
				a.fStartY == b.fStartY && a.fStartZ == b.fStartZ &&
				a.fTargetX == b.fTargetX && a.fTargetY == b.fTargetY &&
				a.fTargetZ == b.fTargetZ &&
				a.fDurationSeconds == b.fDurationSeconds &&
				a.fElapsedSeconds == b.fElapsedSeconds &&
				a.fArcHeight == b.fArcHeight && a.isActive == b.isActive;
			const SERVER_PENDING_PLAYER_COMMAND& c = left.PendingCommand;
			const SERVER_PENDING_PLAYER_COMMAND& d = right.PendingCommand;
			const bool samePending = c.eKind == d.eKind &&
				c.iClientSequence == d.iClientSequence &&
				c.iSkillId == d.iSkillId && c.eTargetIntent == d.eTargetIntent &&
				c.fX == d.fX && c.fZ == d.fZ;
			return left.fPositionX == right.fPositionX &&
				left.fPositionY == right.fPositionY &&
				left.fPositionZ == right.fPositionZ &&
				left.fYawDegrees == right.fYawDegrees &&
				left.fMoveGoalX == right.fMoveGoalX &&
				left.fMoveGoalZ == right.fMoveGoalZ &&
				left.hasMoveGoal == right.hasMoveGoal &&
				left.isCombatReady == right.isCombatReady && samePath &&
				left.iMovePathIndex == right.iMovePathIndex &&
				left.eAction == right.eAction &&
				left.iCurrentSkillId == right.iCurrentSkillId &&
				left.iActionStartTick == right.iActionStartTick &&
				left.fActionElapsedSeconds == right.fActionElapsedSeconds &&
				left.hasSkillTarget == right.hasSkillTarget &&
				left.fSkillTargetX == right.fSkillTargetX &&
				left.fSkillTargetY == right.fSkillTargetY &&
				left.fSkillTargetZ == right.fSkillTargetZ &&
				left.iComboStage == right.iComboStage &&
				left.hasBufferedComboInput == right.hasBufferedComboInput &&
				left.fBufferedComboAimX == right.fBufferedComboAimX &&
				left.fBufferedComboAimZ == right.fBufferedComboAimZ &&
				left.fBufferedComboAimDistance ==
					right.fBufferedComboAimDistance &&
				samePending && sameTrigger;
		};
		const auto sameRestartMutableBossState = [](
			const SERVER_WORLD_ENTITY& left, const SERVER_WORLD_ENTITY& right)
		{
			const bool sameCooldowns =
				left.PatternCooldowns.size() == right.PatternCooldowns.size() &&
				std::equal(left.PatternCooldowns.begin(), left.PatternCooldowns.end(),
					right.PatternCooldowns.begin(),
					[](const SERVER_BOSS_PATTERN_COOLDOWN& a,
						const SERVER_BOSS_PATTERN_COOLDOWN& b)
					{
						return a.iSourcePrimaryActionId == b.iSourcePrimaryActionId &&
							a.strPatternId == b.strPatternId &&
							a.iReadyTick == b.iReadyTick;
					});
			const bool sameOccurrences =
				left.MechanicOccurrences.size() == right.MechanicOccurrences.size() &&
				std::equal(left.MechanicOccurrences.begin(),
					left.MechanicOccurrences.end(),
					right.MechanicOccurrences.begin(),
					[](const SERVER_BOSS_MECHANIC_OCCURRENCE& a,
						const SERVER_BOSS_MECHANIC_OCCURRENCE& b)
					{
						return a.strPatternId == b.strPatternId &&
							a.PinnedDefinitionRevision == b.PinnedDefinitionRevision &&
							a.eState == b.eState && a.eFailure == b.eFailure &&
							a.iTriggerHealthBar == b.iTriggerHealthBar &&
							a.iQueuedTick == b.iQueuedTick &&
							a.iStartedTick == b.iStartedTick &&
							a.iFinishedTick == b.iFinishedTick &&
							a.iPatternSequence == b.iPatternSequence;
					});
			const bool samePath = left.MovePath.size() == right.MovePath.size() &&
				std::equal(left.MovePath.begin(), left.MovePath.end(),
					right.MovePath.begin(), [](const SERVER_NAV_POINT& a,
						const SERVER_NAV_POINT& b)
					{
						return a.x == b.x && a.y == b.y && a.z == b.z;
					});
			return left.iNetEntityId == right.iNetEntityId &&
				left.strPlacementId == right.strPlacementId &&
				left.strPatternId == right.strPatternId &&
				left.strPatternStageId == right.strPatternStageId &&
				left.strActionId == right.strActionId &&
				left.strDamageProfileId == right.strDamageProfileId &&
				left.eAction == right.eAction &&
				left.fPositionX == right.fPositionX &&
				left.fPositionY == right.fPositionY &&
				left.fPositionZ == right.fPositionZ &&
				left.fYawDegrees == right.fYawDegrees &&
				left.fActionElapsedSeconds == right.fActionElapsedSeconds &&
				left.iPatternSequence == right.iPatternSequence &&
				left.PatternTerminalReceipt.iPatternSequence ==
					right.PatternTerminalReceipt.iPatternSequence &&
				left.PatternTerminalReceipt.iRootPatternSequence ==
					right.PatternTerminalReceipt.iRootPatternSequence &&
				left.PatternTerminalReceipt.eResult ==
					right.PatternTerminalReceipt.eResult &&
				left.iPatternStageIndex == right.iPatternStageIndex &&
				left.iPatternStageDurationMs == right.iPatternStageDurationMs &&
				left.iPatternStageFirstEvaluationTick ==
					right.iPatternStageFirstEvaluationTick &&
				left.iAppliedPatternHitCount == right.iAppliedPatternHitCount &&
				left.iAppliedPatternStageSpawnWaveCount ==
					right.iAppliedPatternStageSpawnWaveCount &&
				left.PinnedDefinitionRevision == right.PinnedDefinitionRevision &&
				left.ProductSequencePinnedDefinitionRevision ==
					right.ProductSequencePinnedDefinitionRevision &&
				left.bIntroPatternConsumed == right.bIntroPatternConsumed &&
				left.bScriptedPatternPlayback == right.bScriptedPatternPlayback &&
				left.bAutomaticPatternSequenceAuditionOverride ==
					right.bAutomaticPatternSequenceAuditionOverride &&
				left.bAutomaticPatternSequenceAuditionHold ==
					right.bAutomaticPatternSequenceAuditionHold &&
				left.bAutomaticPatternSequenceStepRunning ==
					right.bAutomaticPatternSequenceStepRunning &&
				left.bAutomaticPatternSequencePausedForRevive ==
					right.bAutomaticPatternSequencePausedForRevive &&
				left.iPatternTargetEntityId == right.iPatternTargetEntityId &&
				left.iTargetEntityId == right.iTargetEntityId &&
				left.ePatternAttachmentSlot == right.ePatternAttachmentSlot &&
				left.iGrabExecutionCommittedPatternSequence ==
					right.iGrabExecutionCommittedPatternSequence &&
				left.iGrabExecutionCommittedStageIndex ==
					right.iGrabExecutionCommittedStageIndex &&
				left.BossCombat.iStateRevision == right.BossCombat.iStateRevision &&
				left.BossCombat.iFlags == right.BossCombat.iFlags &&
				left.BossCombat.iStaggerCurrent == right.BossCombat.iStaggerCurrent &&
				left.BossCombat.iShieldCurrent == right.BossCombat.iShieldCurrent &&
				left.PendingPatternIds == right.PendingPatternIds &&
				left.PendingPatternFollowup.strPatternId ==
					right.PendingPatternFollowup.strPatternId &&
				left.PendingPatternFollowup.PinnedDefinitionRevision ==
					right.PendingPatternFollowup.PinnedDefinitionRevision &&
				left.PendingPatternFollowup.iSourcePatternSequence ==
					right.PendingPatternFollowup.iSourcePatternSequence &&
				left.PendingPatternFollowup.iRootPatternSequence ==
					right.PendingPatternFollowup.iRootPatternSequence &&
				left.PendingPatternFollowup.iDepth ==
					right.PendingPatternFollowup.iDepth &&
				left.iPatternFollowupDepth == right.iPatternFollowupDepth &&
				left.iPatternFollowupRootSequence ==
					right.iPatternFollowupRootSequence &&
				left.TriggeredPatternIds == right.TriggeredPatternIds &&
				sameCooldowns && sameOccurrences &&
				left.bMechanicLedgerRequiresReset ==
					right.bMechanicLedgerRequiresReset && samePath &&
				left.iMovePathIndex == right.iMovePathIndex;
		};
		SERVER_PLAYER& liveAuditionPlayer = room.m_Players.at(AUDITION_PLAYER);
		liveAuditionPlayer.fPositionX += 37.f;
		liveAuditionPlayer.fPositionZ -= 19.f;
		liveAuditionPlayer.fYawDegrees = 123.f;
		liveAuditionPlayer.isCombatReady = false;
		liveAuditionPlayer.hasMoveGoal = true;
		liveAuditionPlayer.fMoveGoalX = liveAuditionPlayer.fPositionX + 1.f;
		liveAuditionPlayer.fMoveGoalZ = liveAuditionPlayer.fPositionZ + 2.f;
		liveAuditionPlayer.MovePath = {
			SERVER_NAV_POINT{ 1.f, 2.f, 3.f },
			SERVER_NAV_POINT{ 4.f, 5.f, 6.f } };
		liveAuditionPlayer.iMovePathIndex = 1u;
		liveAuditionPlayer.iCurrentSkillId = 34010u;
		liveAuditionPlayer.fActionElapsedSeconds = 1.25f;
		liveAuditionPlayer.hasSkillTarget = true;
		liveAuditionPlayer.fSkillTargetX = 7.f;
		liveAuditionPlayer.fSkillTargetY = 8.f;
		liveAuditionPlayer.fSkillTargetZ = 9.f;
		liveAuditionPlayer.iComboStage = 2u;
		liveAuditionPlayer.hasBufferedComboInput = true;
		liveAuditionPlayer.fBufferedComboAimX = -0.25f;
		liveAuditionPlayer.fBufferedComboAimZ = 0.75f;
		liveAuditionPlayer.fBufferedComboAimDistance = 12.f;
		liveAuditionPlayer.PendingCommand.eKind =
			PLAYER_PENDING_COMMAND_KIND::SKILL;
		liveAuditionPlayer.PendingCommand.iClientSequence = 44u;
		liveAuditionPlayer.PendingCommand.iSkillId = 34010u;
		liveAuditionPlayer.PendingCommand.fX = 10.f;
		liveAuditionPlayer.PendingCommand.fZ = 11.f;
		liveAuditionPlayer.TriggerMove = SERVER_TRIGGER_MOVE{
			{}, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 0.75f, 0.25f, 1.5f, KOUKU_HUD_MODE::END, true };
		const SERVER_PLAYER playerBeforeFailedRestart = liveAuditionPlayer;
		const SERVER_WORLD_ENTITY bossBeforeFailedRestart = *auditionBoss;
		const CGameRoom::VALTAN_PATTERN_ID_AUDITION_STATE
			auditionBeforeFailedRestart = room.m_ValtanPatternIdAudition;
		const std::uint32_t nextEpochBeforeFailedRestart =
			room.m_iNextValtanAuditionEpoch;
		const std::size_t lifecycleCountBeforeFailedRestart =
			room.m_PendingValtanAuditionLifecycle.size();
		std::vector<COMBAT_OBJECT_SNAPSHOT> combatObjectsBeforeFailedRestart;
		const bool capturedCombatObjectsBeforeFailedRestart =
			room.m_CombatObjectRuntime.Build_Snapshots(
				combatObjectsBeforeFailedRestart);
		auto* resetPlacement = const_cast<WORLD_BOOTSTRAP_PLACEMENT*>(
			room.Find_Placement("boss.valtan.center"));
		const bool placementEnabledBefore =
			nullptr != resetPlacement && resetPlacement->isEnabled;
		if (nullptr != resetPlacement)
			resetPlacement->isEnabled = true;
		C2S_VALTAN_AUDITION_REQUEST failedRestart = restartPattern;
		failedRestart.iRequestSequence = 6u;
		failedRestart.iPredecessorRoomAuditionEpoch = restartedEpoch;
		failedRestart.iPredecessorPatternSequence = restartedSequence;
		failedRestart.ExpectedDefinitionRevision = restartedRevision;
		const VALTAN_AUDITION_RESULT failedRestartResult =
			room.Evaluate_ValtanAudition(
				AUDITION_SESSION, failedRestart, reportedBar);
		if (nullptr != resetPlacement)
			resetPlacement->isEnabled = placementEnabledBefore;
		auditionBoss = room.Find_AuditionBoss();
		const SERVER_PLAYER& playerAfterFailedRestart =
			room.m_Players.at(AUDITION_PLAYER);
		tests.Require(
			nullptr != resetPlacement &&
			VALTAN_AUDITION_RESULT::REJECTED_OCCURRENCE_PRESERVED ==
				failedRestartResult &&
			playerBeforeFailedRestart.fPositionX ==
				playerAfterFailedRestart.fPositionX &&
			playerBeforeFailedRestart.fPositionY ==
				playerAfterFailedRestart.fPositionY &&
			playerBeforeFailedRestart.fPositionZ ==
				playerAfterFailedRestart.fPositionZ &&
			playerBeforeFailedRestart.fYawDegrees ==
				playerAfterFailedRestart.fYawDegrees &&
			playerBeforeFailedRestart.isCombatReady ==
				playerAfterFailedRestart.isCombatReady &&
			playerBeforeFailedRestart.eAction ==
				playerAfterFailedRestart.eAction &&
			playerBeforeFailedRestart.hasMoveGoal ==
				playerAfterFailedRestart.hasMoveGoal &&
			playerBeforeFailedRestart.fMoveGoalX ==
				playerAfterFailedRestart.fMoveGoalX &&
			playerBeforeFailedRestart.fMoveGoalZ ==
				playerAfterFailedRestart.fMoveGoalZ &&
			playerBeforeFailedRestart.TriggerMove.isActive ==
				playerAfterFailedRestart.TriggerMove.isActive &&
			nullptr != auditionBoss &&
			bossBeforeFailedRestart.iPatternSequence ==
				auditionBoss->iPatternSequence &&
			bossBeforeFailedRestart.strPatternId == auditionBoss->strPatternId &&
			bossBeforeFailedRestart.eAction == auditionBoss->eAction &&
			bossBeforeFailedRestart.fPositionX == auditionBoss->fPositionX &&
			bossBeforeFailedRestart.fPositionZ == auditionBoss->fPositionZ,
			"Rollback player pose/combat state and boss occurrence when Restart reset preflight fails");

		/* Complete Play carries the exact definition revision observed by the
		   Client. A stale revision must reject before any boss, player, arena, or
		   lifecycle mutation even though the request receipt itself is admitted. */
		const SERVER_PLAYER playerBeforeStaleCompletePlay =
			room.m_Players.at(AUDITION_PLAYER);
		const SERVER_WORLD_ENTITY bossBeforeStaleCompletePlay =
			*auditionBoss;
		const CGameRoom::VALTAN_PATTERN_ID_AUDITION_STATE
			auditionBeforeStaleCompletePlay =
				room.m_ValtanPatternIdAudition;
		const std::uint32_t nextEpochBeforeStaleCompletePlay =
			room.m_iNextValtanAuditionEpoch;
		const std::size_t lifecycleCountBeforeStaleCompletePlay =
			room.m_PendingValtanAuditionLifecycle.size();
		const std::vector<WORLD_DESTRUCTION_GROUP_STATE>
			destructionBeforeStaleCompletePlay =
				room.m_WorldDestructionRuntime.Get_GroupStates();
		const std::vector<ENCOUNTER_PROP_SLOT_STATE>
			propsBeforeStaleCompletePlay =
				room.m_EncounterPropRuntime.Get_SlotStates();
		const std::uint64_t navigationBeforeStaleCompletePlay =
			room.m_ServerNavigation.Get_Revision();
		const std::uint64_t collisionBeforeStaleCompletePlay =
			room.m_ServerCollisionSystem.Get_Revision();
		const std::uint32_t pillarBreakBeforeStaleCompletePlay =
			room.m_iPillarAuditionBreakTick;
		const bool pillarCycleBeforeStaleCompletePlay =
			room.m_bPillarAuditionCycleArmed;
		C2S_VALTAN_AUDITION_REQUEST staleCompletePlay{};
		staleCompletePlay.iRequestSequence = 7u;
		staleCompletePlay.eOperation =
			VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
		staleCompletePlay.strBossPlacementId = "boss.valtan.center";
		staleCompletePlay.strPatternId = "VALTAN_WHIRLWIND";
		staleCompletePlay.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		staleCompletePlay.ExpectedDefinitionRevision.Bytes[0] ^= 0x5au;
		const VALTAN_AUDITION_RESULT staleCompletePlayResult =
			room.Evaluate_ValtanAudition(
				AUDITION_SESSION, staleCompletePlay, reportedBar);
		auditionBoss = room.Find_AuditionBoss();
		const SERVER_PLAYER& playerAfterStaleCompletePlay =
			room.m_Players.at(AUDITION_PLAYER);
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST ==
				staleCompletePlayResult &&
			nullptr != auditionBoss &&
			sameBaitMutablePlayerState(
				playerBeforeStaleCompletePlay,
				playerAfterStaleCompletePlay) &&
			sameRestartMutableBossState(
				bossBeforeStaleCompletePlay, *auditionBoss) &&
			auditionBeforeStaleCompletePlay.ePhase ==
				room.m_ValtanPatternIdAudition.ePhase &&
			auditionBeforeStaleCompletePlay.iOwnerSessionId ==
				room.m_ValtanPatternIdAudition.iOwnerSessionId &&
			auditionBeforeStaleCompletePlay.iRequestSequence ==
				room.m_ValtanPatternIdAudition.iRequestSequence &&
			auditionBeforeStaleCompletePlay.iRoomAuditionEpoch ==
				room.m_ValtanPatternIdAudition.iRoomAuditionEpoch &&
			auditionBeforeStaleCompletePlay.iExpectedPatternSequence ==
				room.m_ValtanPatternIdAudition.iExpectedPatternSequence &&
			auditionBeforeStaleCompletePlay.PinnedDefinitionRevision ==
				room.m_ValtanPatternIdAudition.PinnedDefinitionRevision &&
			nextEpochBeforeStaleCompletePlay ==
				room.m_iNextValtanAuditionEpoch &&
			lifecycleCountBeforeStaleCompletePlay ==
				room.m_PendingValtanAuditionLifecycle.size() &&
			sameDestructionState(
				destructionBeforeStaleCompletePlay,
				room.m_WorldDestructionRuntime.Get_GroupStates()) &&
			samePropState(
				propsBeforeStaleCompletePlay,
				room.m_EncounterPropRuntime.Get_SlotStates()) &&
			navigationBeforeStaleCompletePlay ==
				room.m_ServerNavigation.Get_Revision() &&
			collisionBeforeStaleCompletePlay ==
				room.m_ServerCollisionSystem.Get_Revision() &&
			pillarBreakBeforeStaleCompletePlay ==
				room.m_iPillarAuditionBreakTick &&
			pillarCycleBeforeStaleCompletePlay ==
				room.m_bPillarAuditionCycleArmed,
			"Reject stale Complete Play revision before mutating boss, player, arena, or lifecycle state");
#endif

		room.m_Players.clear();
		room.m_PlayerIdBySessionId.clear();
		tests.Require(
			room.Reset_ValtanArenaWhenEmpty() &&
			0u == room.m_iValtanAuditionArmedHealthBar,
			"Drop the armed audition bar with the encounter reset");
	}
}

