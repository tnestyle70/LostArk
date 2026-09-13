#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "BossCombatRuntime.h"
#include "EncounterPropRuntime.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_ValtanMechanicLedger(TESTS& tests, CGameplayCatalog& catalog, CServerNavigation& navigation, CValtanBrain& brain)
{


	{
		/* Source cooldowns are selection gates, not display-only metadata. Run a
		deterministic normal-pattern simulation long enough to see positive-
		cooldown patterns repeat and reject any early repeat. */
		std::map<PLAYER_ID, SERVER_PLAYER> cooldownPlayers;
		SERVER_PLAYER cooldownTarget{};
		cooldownTarget.iPlayerId = 77u;
		cooldownTarget.iNetEntityId = 177u;
		cooldownTarget.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		cooldownTarget.iCurrentHp = 1000000u;
		cooldownTarget.iMaximumHp = 1000000u;
		cooldownTarget.fPositionX = 6.f;
		cooldownTarget.fPositionY = 22.97f;
		cooldownTarget.fPositionZ = 0.f;
		cooldownTarget.isCombatReady = true;
		cooldownPlayers.emplace(cooldownTarget.iPlayerId, cooldownTarget);
		SERVER_WORLD_ENTITY cooldownBoss{};
		cooldownBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		cooldownBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		cooldownBoss.strArchetypeId = "BOSS_VALTAN";
		cooldownBoss.strEncounterId = "ENCOUNTER_VALTAN";
		cooldownBoss.iCurrentHp = 60000u;
		cooldownBoss.iMaximumHp = 60000u;
		cooldownBoss.iMaximumHealthBars = 160u;
		cooldownBoss.iLastEvaluatedHealthBar = 160u;
		cooldownBoss.iPhaseTwoHpPercent = 50u;
		cooldownBoss.fPositionY = 22.97f;
		cooldownBoss.fSpawnPositionY = 22.97f;
		cooldownBoss.fEngageDistance = 35.f;
		cooldownBoss.fMoveSpeed = 3.f;
		cooldownBoss.bIntroPatternConsumed = true;
		cooldownBoss.bScriptedPatternPlayback = true;
		std::map<std::uint32_t, std::uint32_t> lastStartTickBySourceAction;
		std::uint32_t previousSequence = 0u;
		bool respectedCooldowns = true;
		bool observedPositiveCooldownRepeat = false;
		bool observedFourSlash = false;
		bool observedIndependentSource = false;
		bool replayedIntro = false;
		bool observedDeterministicDecisionTrace = false;
		bool observedCooldownExclusionTrace = false;
		std::vector<DAMAGE_EVENT> cooldownDamageEvents;
		const std::vector<BOSS_PATTERN_DEFINITION>* cooldownPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		for (std::uint32_t tick = 1000u; tick < 11000u; ++tick)
		{
			SERVER_PLAYER& liveTarget = cooldownPlayers.begin()->second;
			liveTarget.iCurrentHp = liveTarget.iMaximumHp;
			liveTarget.eAction = PLAYER_ACTION_STATE::NONE;
			cooldownDamageEvents.clear();
			brain.Update(
				cooldownBoss, cooldownPlayers, catalog, navigation,
				1.f / 30.f, tick, {}, cooldownDamageEvents);
			if (cooldownBoss.iPatternSequence == previousSequence)
				continue;
			previousSequence = cooldownBoss.iPatternSequence;
			if (const VALTAN_DECISION_TRACE* trace =
				brain.Get_LatestDecisionTrace())
			{
				const auto selectedCandidate = std::find_if(
					trace->Candidates.begin(), trace->Candidates.end(),
					[trace](const VALTAN_DECISION_CANDIDATE_TRACE& candidate)
					{
						return candidate.strPatternId ==
							trace->strSelectedPatternId;
					});
				observedDeterministicDecisionTrace =
					observedDeterministicDecisionTrace ||
					(VALTAN_DECISION_RESULT::SELECTED == trace->eResult &&
					 0u != trace->iRawRandomInput &&
					 trace->iMixedRandomValue >= trace->iRandomTicket &&
					 trace->iTotalWeight > trace->iRandomTicket &&
					 trace->Candidates.end() != selectedCandidate &&
					 selectedCandidate->bSelected &&
					 selectedCandidate->iEffectiveWeight > 0u &&
					 trace->iRandomTicket >=
						selectedCandidate->iWeightBeginInclusive &&
					 trace->iRandomTicket <
						selectedCandidate->iWeightEndExclusive);
				observedCooldownExclusionTrace =
					observedCooldownExclusionTrace || std::any_of(
						trace->Candidates.begin(), trace->Candidates.end(),
						[](const VALTAN_DECISION_CANDIDATE_TRACE& candidate)
						{
							return candidate.iCooldownRemainingTicks > 0u &&
								0u != (candidate.iExclusionMask &
									VALTAN_EXCLUDE_COOLDOWN);
						});
			}
			replayedIntro = replayedIntro ||
				cooldownBoss.strPatternId == "VALTAN_ENTRANCE_WHIRLWIND";
			if (nullptr == cooldownPatterns)
			{
				respectedCooldowns = false;
				break;
			}
			const auto definition = std::find_if(
				cooldownPatterns->begin(), cooldownPatterns->end(),
				[&cooldownBoss](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == cooldownBoss.strPatternId; });
			if (cooldownPatterns->end() == definition)
			{
				respectedCooldowns = false;
				break;
			}
			const auto previous = lastStartTickBySourceAction.find(
				definition->iSourcePrimaryActionId);
			if (lastStartTickBySourceAction.end() != previous &&
				0u != definition->iSourceCooldownTicks)
			{
				observedPositiveCooldownRepeat = true;
				respectedCooldowns = respectedCooldowns &&
					static_cast<std::uint32_t>(tick - previous->second) >=
					definition->iSourceCooldownTicks;
			}
			lastStartTickBySourceAction[
				definition->iSourcePrimaryActionId] = tick;
			observedFourSlash = observedFourSlash ||
				"VALTAN_FOUR_SLASH" == definition->strPatternId;
			observedIndependentSource = observedIndependentSource ||
				420601u == definition->iSourcePrimaryActionId;
		}
		const std::size_t fourSlashCooldownCount =
			static_cast<std::size_t>(std::count_if(
				cooldownBoss.PatternCooldowns.begin(),
				cooldownBoss.PatternCooldowns.end(),
				[](const SERVER_BOSS_PATTERN_COOLDOWN& cooldown)
				{ return 420609u == cooldown.iSourcePrimaryActionId; }));
		const bool hasIndependentCooldown = std::any_of(
			cooldownBoss.PatternCooldowns.begin(),
			cooldownBoss.PatternCooldowns.end(),
			[](const SERVER_BOSS_PATTERN_COOLDOWN& cooldown)
			{ return 420601u == cooldown.iSourcePrimaryActionId; });
		tests.Require(
			respectedCooldowns && observedPositiveCooldownRepeat &&
			observedFourSlash && observedIndependentSource &&
			1u == fourSlashCooldownCount &&
			hasIndependentCooldown && !replayedIntro &&
			observedDeterministicDecisionTrace &&
			observedCooldownExclusionTrace,
			"Gate normal Valtan reselection by four-slash source-action cooldown, keep other sources independent, and never reroll the intro");
	}

	{
		/* Drive the published rejoined pattern through the real fixed-step brain.
		The authored millisecond offsets deliberately do not all fall on exact
		30 Hz boundaries, so each pulse must wait for the first crossing tick. */
		std::map<PLAYER_ID, SERVER_PLAYER> slashPlayers;
		SERVER_PLAYER slashTarget{};
		slashTarget.iPlayerId = 78u;
		slashTarget.iNetEntityId = 178u;
		slashTarget.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		slashTarget.iCurrentHp = 1000000u;
		slashTarget.iMaximumHp = 1000000u;
		slashTarget.fPositionY = 22.97f;
		slashTarget.fPositionZ = 4.f;
		slashTarget.isCombatReady = true;
		slashPlayers.emplace(slashTarget.iPlayerId, slashTarget);

		SERVER_WORLD_ENTITY slashBoss{};
		slashBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		slashBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		slashBoss.strArchetypeId = "BOSS_VALTAN";
		slashBoss.strEncounterId = "ENCOUNTER_VALTAN";
		slashBoss.iCurrentHp = 60000u;
		slashBoss.iMaximumHp = 60000u;
		slashBoss.iMaximumHealthBars = 160u;
		slashBoss.iLastEvaluatedHealthBar = 160u;
		slashBoss.iPhaseTwoHpPercent = 50u;
		slashBoss.fPositionY = 22.97f;
		slashBoss.fSpawnPositionY = 22.97f;
		slashBoss.fEngageDistance = 35.f;
		slashBoss.fMoveSpeed = 3.f;
		slashBoss.bIntroPatternConsumed = true;
		slashBoss.bScriptedPatternPlayback = true;
		slashBoss.PendingPatternIds.push_back("VALTAN_FOUR_SLASH");

		CValtanBrain slashBrain;
		std::vector<DAMAGE_EVENT> slashDamageEvents;
		std::uint32_t slashServerTick = 12000u;
		bool enteredSlashStage = false;
		bool windupStayedDamageFree = true;
		for (std::uint32_t tick = 0u; tick < 20u; ++tick)
		{
			slashDamageEvents.clear();
			slashBrain.Update(
				slashBoss, slashPlayers, catalog, navigation,
				1.f / 30.f, slashServerTick++, {}, slashDamageEvents);
			windupStayedDamageFree =
				windupStayedDamageFree && slashDamageEvents.empty();
			if ("SLASHES" == slashBoss.strPatternStageId)
			{
				enteredSlashStage = true;
				break;
			}
		}

		std::vector<std::uint32_t> observedContactTicks;
		std::size_t observedContactCount = 0u;
		bool crossingStateExact = enteredSlashStage;
		std::uint32_t slashTransitionTick = 0u;
		for (std::uint32_t activeTick = 1u;
			activeTick <= 120u && enteredSlashStage; ++activeTick)
		{
			slashDamageEvents.clear();
			slashBrain.Update(
				slashBoss, slashPlayers, catalog, navigation,
				1.f / 30.f, slashServerTick++, {}, slashDamageEvents);
			observedContactCount += slashDamageEvents.size();
			if (!slashDamageEvents.empty())
				observedContactTicks.push_back(activeTick);

			const bool expectsContact =
				54u == activeTick || 77u == activeTick || 100u == activeTick;
			crossingStateExact = crossingStateExact &&
				(expectsContact ?
					1u == slashDamageEvents.size() &&
					178u == slashDamageEvents.front().iTargetNetEntityId &&
					!slashDamageEvents.front().isOutgoing :
					slashDamageEvents.empty());
			if (53u == activeTick)
				crossingStateExact = crossingStateExact &&
					0u == slashBoss.iAppliedPatternHitCount;
			else if (54u == activeTick || 76u == activeTick)
				crossingStateExact = crossingStateExact &&
					1u == slashBoss.iAppliedPatternHitCount;
			else if (77u == activeTick || 99u == activeTick)
				crossingStateExact = crossingStateExact &&
					2u == slashBoss.iAppliedPatternHitCount;
			else if (100u == activeTick)
				crossingStateExact = crossingStateExact &&
					3u == slashBoss.iAppliedPatternHitCount;

			if ("SPIN" == slashBoss.strPatternStageId)
			{
				slashTransitionTick = activeTick;
				break;
			}
		}
		tests.Require(
			windupStayedDamageFree && crossingStateExact &&
			105u == slashTransitionTick && 3u == observedContactCount &&
			std::vector<std::uint32_t>{ 54u, 77u, 100u } ==
				observedContactTicks,
			"Consume 1790/2560/3330 ms slash contacts at ticks 54/77/100 and enter the joined SPIN at tick 105 with no extra pulse");

		/* SPIN is the next stage of the same pattern, not a separately rolled
		pattern. It owns one contact at 600 ms and then hands off to recovery. */
		std::vector<std::uint32_t> spinContactTicks;
		std::size_t spinContactCount = 0u;
		std::uint32_t spinTransitionTick = 0u;
		bool spinCrossingStateExact = true;
		for (std::uint32_t activeTick = 1u; activeTick <= 100u; ++activeTick)
		{
			slashDamageEvents.clear();
			slashBrain.Update(
				slashBoss, slashPlayers, catalog, navigation,
				1.f / 30.f, slashServerTick++, {}, slashDamageEvents);
			spinContactCount += slashDamageEvents.size();
			if (!slashDamageEvents.empty())
				spinContactTicks.push_back(activeTick);

			const bool expectsContact = 18u == activeTick;
			spinCrossingStateExact = spinCrossingStateExact &&
				(expectsContact ?
					1u == slashDamageEvents.size() &&
					178u == slashDamageEvents.front().iTargetNetEntityId &&
					!slashDamageEvents.front().isOutgoing :
					slashDamageEvents.empty());
			if (17u == activeTick)
				spinCrossingStateExact = spinCrossingStateExact &&
					0u == slashBoss.iAppliedPatternHitCount;
			else if (18u <= activeTick &&
				"SPIN" == slashBoss.strPatternStageId)
				spinCrossingStateExact = spinCrossingStateExact &&
					1u == slashBoss.iAppliedPatternHitCount;
			else if ("RECOVERY" == slashBoss.strPatternStageId)
				spinCrossingStateExact = spinCrossingStateExact &&
					0u == slashBoss.iAppliedPatternHitCount;

			if ("RECOVERY" == slashBoss.strPatternStageId)
			{
				spinTransitionTick = activeTick;
				break;
			}
		}
		tests.Require(
			spinCrossingStateExact && 96u == spinTransitionTick &&
			1u == spinContactCount &&
			std::vector<std::uint32_t>{ 18u } == spinContactTicks,
			"Consume the joined SPIN contact at tick 18 and enter recovery at ceil(3167ms*30Hz)=tick 96");
	}

	{
		/* The stele set is raised by the RECOVERY stage of the 100-bar mechanic
		and by nothing else, so the raid never sees a pillar unless that pattern
		actually runs and actually reaches that stage. Cross the bar and walk the
		pattern to the end so a break anywhere in that chain names itself. */
		std::map<PLAYER_ID, SERVER_PLAYER> pillarPlayers;
		SERVER_PLAYER pillarTarget{};
		pillarTarget.iPlayerId = 91u;
		pillarTarget.iNetEntityId = 191u;
		pillarTarget.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		pillarTarget.iCurrentHp = 1000000u;
		pillarTarget.iMaximumHp = 1000000u;
		pillarTarget.fPositionX = 6.f;
		pillarTarget.fPositionY = 22.97f;
		pillarTarget.fPositionZ = 0.f;
		pillarTarget.isCombatReady = true;
		pillarPlayers.emplace(pillarTarget.iPlayerId, pillarTarget);

		SERVER_WORLD_ENTITY pillarBoss{};
		pillarBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		pillarBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		pillarBoss.strArchetypeId = "BOSS_VALTAN";
		pillarBoss.strEncounterId = "ENCOUNTER_VALTAN";
		pillarBoss.iMaximumHp = 60000u;
		pillarBoss.iMaximumHealthBars = 160u;
		pillarBoss.iPhaseTwoHpPercent = 68u;
		pillarBoss.fPositionY = 22.97f;
		pillarBoss.fSpawnPositionY = 22.97f;
		pillarBoss.fEngageDistance = 35.f;
		pillarBoss.fMoveSpeed = 3.f;
		pillarBoss.bIntroPatternConsumed = true;
		pillarBoss.bAutomaticPatternSequenceAuditionOverride = true;
		/* Sit one bar above the trigger so the very next damage crosses it. */
		pillarBoss.iCurrentHp = CValtanBrain::Resolve_HealthBarHp(pillarBoss, 101u);
		pillarBoss.iLastEvaluatedHealthBar = 101u;

		CValtanBrain pillarBrain;
		std::vector<DAMAGE_EVENT> pillarEvents;
		pillarBoss.iCurrentHp = CValtanBrain::Resolve_HealthBarHp(pillarBoss, 100u);
		pillarBrain.Update(
			pillarBoss, pillarPlayers, catalog, navigation, 1.f / 30.f, 2000u,
			{}, pillarEvents);
		const bool queuedOrRunning =
			"VALTAN_FOUR_PILLARS_105" == pillarBoss.strPatternId ||
			pillarBoss.PendingPatternIds.end() != std::find(
				pillarBoss.PendingPatternIds.begin(),
				pillarBoss.PendingPatternIds.end(),
				std::string("VALTAN_FOUR_PILLARS_105"));
		tests.Require(
			queuedOrRunning,
			"Queue the 100-bar stele mechanic when Valtan crosses its trigger bar");

		bool reachedRecovery = false;
		bool ranPillarPattern = false;
		for (std::uint32_t tick = 2001u; tick < 2400u; ++tick)
		{
			pillarEvents.clear();
			pillarBrain.Update(
				pillarBoss, pillarPlayers, catalog, navigation, 1.f / 30.f, tick,
				{}, pillarEvents);
			if ("VALTAN_FOUR_PILLARS_105" != pillarBoss.strPatternId)
				continue;
			ranPillarPattern = true;
			if ("RECOVERY" == pillarBoss.strPatternStageId)
			{
				reachedRecovery = true;
				break;
			}
		}
		tests.Require(
			ranPillarPattern,
			"Start the 100-bar stele mechanic from the queue");
		tests.Require(
			reachedRecovery && 0u != pillarBoss.iPatternSequence,
			"Reach the stele RECOVERY stage that raises the four pillars");
	}

	{
		/* The stage edge the Brain reaches still has to reach the prop runtime.
		This is the room-side half of the raise: the same pattern and stage the
		previous test proved reachable, handed to the entry the room tick calls. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto pillarRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& pillarRoom = *pillarRoomStorage;
		SERVER_WORLD_ENTITY stageBoss{};
		stageBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		stageBoss.strArchetypeId = "BOSS_VALTAN";
		stageBoss.strEncounterId = "ENCOUNTER_VALTAN";
		stageBoss.strPatternId = "VALTAN_FOUR_PILLARS_105";
		stageBoss.strPatternStageId = "RECOVERY";
		stageBoss.iPatternStageIndex = 3u;
		stageBoss.iPatternSequence = 1u;
		stageBoss.PinnedDefinitionRevision =
			pillarRoom.m_GameplayCatalog.Get_ActiveRevision();
		const bool roomReady = pillarRoom.Is_Ready() &&
			pillarRoom.m_EncounterPropRuntime.Is_Initialized();
		const bool entered =
			pillarRoom.Apply_EncounterPropStageEntry(stageBoss, 500u);
		const auto& raisedSlots =
			pillarRoom.m_EncounterPropRuntime.Get_SlotStates();
		const bool everySlotIntact = 4u == raisedSlots.size() &&
			std::all_of(raisedSlots.begin(), raisedSlots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return ENCOUNTER_PROP_STATE::INTACT == slot.eState;
				});
		tests.Require(
			roomReady && entered && everySlotIntact,
			"Raise the four stele from the room stage entry the tick loop calls");
	}

	{
		/* Ordered playback is one exact authored program. This explicit review
		fixture covers entrance, lethal wipe, pursuit, pizza targeting and grab
		mechanics independently of the user's editable Product default sequence. */
		std::map<PLAYER_ID, SERVER_PLAYER> entrancePlayers;
		SERVER_PLAYER entranceTarget{};
		entranceTarget.iPlayerId = 78u;
		entranceTarget.iNetEntityId = 178u;
		entranceTarget.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		entranceTarget.iCurrentHp = 1000000u;
		entranceTarget.iMaximumHp = 1000000u;
		entranceTarget.fPositionX = 196.03f;
		entranceTarget.fPositionY = 22.97f;
		entranceTarget.fPositionZ = -122.06f;
		entranceTarget.isCombatReady = true;
		entrancePlayers.emplace(entranceTarget.iPlayerId, entranceTarget);
		const auto makeEntranceBoss = []()
		{
			SERVER_WORLD_ENTITY boss{};
			boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			boss.eAction = SERVER_ENTITY_ACTION::IDLE;
			boss.strArchetypeId = "BOSS_VALTAN";
			boss.strEncounterId = "ENCOUNTER_VALTAN";
			boss.iCurrentHp = 60000u;
			boss.iMaximumHp = 60000u;
			boss.iMaximumHealthBars = 160u;
			boss.iLastEvaluatedHealthBar = 160u;
			boss.iPhaseTwoHpPercent = 68u;
			boss.iPhase = 1u;
			boss.fPositionX = 156.03f;
			boss.fPositionY = 22.97f;
			boss.fPositionZ = -122.06f;
			boss.fSpawnPositionX = boss.fPositionX;
			boss.fSpawnPositionY = 22.97f;
			boss.fSpawnPositionZ = boss.fPositionZ;
			boss.fEngageDistance = 35.f;
			boss.fMoveSpeed = 3.f;
			return boss;
		};
		// Preserve the detailed legacy mechanics fixture independently of the
		// editable Product default. Both use this same ordered Brain path.
		const std::vector<std::string> expectedOrder{
			"VALTAN_ENTRANCE_CINEMATIC",
			"VALTAN_WHIRLWIND",
			"VALTAN_FOUR_SLASH",
			"VALTAN_FIST_IN_OUT",
			"VALTAN_HIGH_JUMP",
			"VALTAN_FLOOR_WIPE_130",
			"VALTAN_DASH_CHARGE",
			"VALTAN_ARENA_BREAK_109",
			"VALTAN_SIX_PIZZA_106",
			"VALTAN_ATTACK_WHIRLWIND",
			"VALTAN_CHARGE",
			"VALTAN_SEQUENCE_FOUR",
			"VALTAN_ROAR_CHARGE",
			"VALTAN_SEQUENCE_RUSH",
			"VALTAN_THREE",
			"VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
			"VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
			"VALTAN_TERRAIN_DESTRUCTION",
			"VALTAN_WARP",
			"VALTAN_SEQUENCE_TWOHAND",
			"VALTAN_SEQUENCE_WHIRLWIND",
			"VALTAN_TRASH",
			"VALTAN_TRASH_CATCH_SUCCESS",
			"VALTAN_TRASH_CATCH_FAIL",
			"VALTAN_TRASH_CATCH_IF",
			"VALTAN_CATCH_BREATH",
			"VALTAN_COUNTER",
			"VALTAN_CHARGE_2" };
		BOSS_PATTERN_SEQUENCE_DEFINITION reviewSequence{};
		reviewSequence.strEncounterId = "ENCOUNTER_VALTAN";
		reviewSequence.strSequenceId = "sequence.valtan.server-authored.v1";
		reviewSequence.eMode = BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE;
		reviewSequence.iInterStepPursuitMs = 1000u;
		reviewSequence.iInterStepPursuitTicks = 30u;
		reviewSequence.PatternIds = expectedOrder;
		reviewSequence.TransitionPursuitMs.assign(
			expectedOrder.size() - 1u, 1000u);
		reviewSequence.TransitionPursuitTicks.assign(
			expectedOrder.size() - 1u, 30u);
		/* The selected edge after Arena Break owns a 100 ms wait. Its neighbors
		   intentionally retain the one-second default. */
		reviewSequence.TransitionPursuitMs[7u] = 100u;
		reviewSequence.TransitionPursuitTicks[7u] = 3u;
		reviewSequence.iExpectedStepCount = static_cast<std::uint32_t>(expectedOrder.size());
		std::uint32_t automaticTick = 1000u;
		const auto advanceBoss = [&] (
			SERVER_WORLD_ENTITY& boss, const std::uint32_t count,
			std::vector<std::string>* observedOrder = nullptr)
		{
			std::vector<DAMAGE_EVENT> events;
			CValtanBrain brain;
			for (std::uint32_t index = 0u; index < count; ++index)
			{
				events.clear();
				// The entrance override still evaluates the health ledger, exactly as
				// a room does with its nonzero active catalog generation.
				brain.Update(
					boss, entrancePlayers, catalog, navigation,
					1.f / 30.f, automaticTick++, {}, events,
					&catalog, 1u, nullptr, &reviewSequence);
				if (nullptr != observedOrder && !boss.strPatternId.empty() &&
					0u == boss.iPatternFollowupDepth &&
					(observedOrder->empty() ||
					 observedOrder->back() != boss.strPatternId))
				{
					observedOrder->push_back(boss.strPatternId);
				}
			}
		};
		SERVER_WORLD_ENTITY waitingBoss = makeEntranceBoss();
		waitingBoss.bAutomaticPatternSequenceAuditionOverride = true;
		advanceBoss(waitingBoss, 60u);
		const bool heldAtSpawn =
			!waitingBoss.bIntroPatternConsumed &&
			!waitingBoss.bMechanicLedgerRequiresReset &&
			1u == waitingBoss.iLastHealthMechanicGenerationEpoch &&
			waitingBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::IDLE == waitingBoss.eAction &&
			std::abs(waitingBoss.fPositionX - 156.03f) < 0.001f &&
			std::abs(waitingBoss.fPositionZ + 122.06f) < 0.001f;
		entrancePlayers.begin()->second.fPositionX = 164.03f;
		advanceBoss(waitingBoss, 2u);
		tests.Require(
			heldAtSpawn && waitingBoss.bIntroPatternConsumed &&
			!waitingBoss.bMechanicLedgerRequiresReset &&
			!waitingBoss.bAutomaticPatternSequenceStepRunning &&
			!waitingBoss.ProductSequencePinnedDefinitionRevision.Is_Valid() &&
			"VALTAN_ENTRANCE_WHIRLWIND" == waitingBoss.strPatternId,
			"Keep explicit entrance audition available without admitting it to automatic playback");
		entrancePlayers.begin()->second.fPositionX = 196.03f;
		SERVER_WORLD_ENTITY hurtBoss = makeEntranceBoss();
		hurtBoss.bAutomaticPatternSequenceAuditionOverride = true;
		hurtBoss.iCurrentHp = hurtBoss.iMaximumHp - 1u;
		advanceBoss(hurtBoss, 2u);
		tests.Require(
			hurtBoss.bIntroPatternConsumed &&
			!hurtBoss.bMechanicLedgerRequiresReset &&
			1u == hurtBoss.iLastHealthMechanicGenerationEpoch &&
			"VALTAN_ENTRANCE_WHIRLWIND" != hurtBoss.strPatternId,
			"Drop the pending Valtan entrance once the boss is already taking damage");

		entrancePlayers.begin()->second.fPositionX = 164.03f;
		entrancePlayers.begin()->second.iCurrentHp = 5500u;
		entrancePlayers.begin()->second.iMaximumHp = 5500u;
		SERVER_PLAYER pizzaSecondTarget = entrancePlayers.begin()->second;
		pizzaSecondTarget.iPlayerId = 79u;
		pizzaSecondTarget.iNetEntityId = 179u;
		pizzaSecondTarget.fPositionX = 166.03f;
		entrancePlayers.emplace(
			pizzaSecondTarget.iPlayerId, pizzaSecondTarget);
		SERVER_WORLD_ENTITY pacingBoss = makeEntranceBoss();
		pacingBoss.strRotationId = "sequence.valtan.server-authored.v1";
		pacingBoss.iRotationStepIndex = 7u;
		pacingBoss.iAutomaticPatternSequenceInterStepPursuitTicks = 30u;
		for (std::uint32_t tick = 0u;
			tick < 300u && 7u == pacingBoss.iRotationStepIndex; ++tick)
		{
			advanceBoss(pacingBoss, 1u);
		}
		const bool pursuitArmedAfterArenaBreak =
			8u == pacingBoss.iRotationStepIndex &&
			3u == pacingBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			pacingBoss.strPatternId.empty();
		const float pursuitStartX = pacingBoss.fPositionX;
		const float pursuitStartZ = pacingBoss.fPositionZ;
		advanceBoss(pacingBoss, 1u);
		const float pursuitDeltaX = pacingBoss.fPositionX - pursuitStartX;
		const float pursuitDeltaZ = pacingBoss.fPositionZ - pursuitStartZ;
		const bool chasedDuringPursuit =
			2u == pacingBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			pacingBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::CHASE == pacingBoss.eAction &&
			pursuitDeltaX * pursuitDeltaX + pursuitDeltaZ * pursuitDeltaZ > 0.f;
		advanceBoss(pacingBoss, 2u);
		const bool heldNextStepForExactWindow =
			0u == pacingBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			pacingBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::CHASE == pacingBoss.eAction;
		advanceBoss(pacingBoss, 1u);
		tests.Require(
			pursuitArmedAfterArenaBreak && chasedDuringPursuit &&
			heldNextStepForExactWindow &&
			"VALTAN_SIX_PIZZA_106" == pacingBoss.strPatternId,
			"Use only the selected saved transition's 100 ms chase window before starting the next Valtan pattern");
		const NET_ENTITY_ID lockedPizzaTarget =
			pacingBoss.iPatternTargetEntityId;
		auto lockedPizzaPlayer = std::find_if(
			entrancePlayers.begin(), entrancePlayers.end(),
			[lockedPizzaTarget](const auto& entry)
			{
				return entry.second.iNetEntityId == lockedPizzaTarget;
			});
		auto nonlockedPizzaPlayer = std::find_if(
			entrancePlayers.begin(), entrancePlayers.end(),
			[lockedPizzaTarget](const auto& entry)
			{
				return entry.second.iNetEntityId != lockedPizzaTarget;
			});
		const auto yawMatches = [](const float actual, const float expected)
		{
			return std::abs(std::remainder(actual - expected, 360.f)) <
				0.001f;
		};
		const auto trackedPizzaYaw = [&pacingBoss](const SERVER_PLAYER& target)
		{
			return std::atan2(
				target.fPositionX - pacingBoss.fLeapLandingX,
				target.fPositionZ - pacingBoss.fLeapLandingZ) *
				180.f / 3.14159265358979323846f;
		};
		bool retainedRandomTargetWhileNearestChanged = false;
		bool trackedIndependentLockedTargetMove = false;
		bool retainedAndTrackedAcrossStage = false;
		if (entrancePlayers.end() != lockedPizzaPlayer &&
			entrancePlayers.end() != nonlockedPizzaPlayer)
		{
			SERVER_PLAYER& lockedPlayer = lockedPizzaPlayer->second;
			SERVER_PLAYER& nonlockedPlayer = nonlockedPizzaPlayer->second;
			lockedPlayer.fPositionX = pacingBoss.fPositionX + 12.f;
			lockedPlayer.fPositionZ = pacingBoss.fPositionZ + 8.f;
			nonlockedPlayer.fPositionX = pacingBoss.fPositionX + 0.25f;
			nonlockedPlayer.fPositionZ = pacingBoss.fPositionZ;
			advanceBoss(pacingBoss, 1u);
			retainedRandomTargetWhileNearestChanged =
				lockedPizzaTarget == pacingBoss.iPatternTargetEntityId &&
				nonlockedPlayer.iNetEntityId == pacingBoss.iTargetEntityId &&
				yawMatches(
					pacingBoss.fYawDegrees,
					trackedPizzaYaw(lockedPlayer));

			lockedPlayer.fPositionX = pacingBoss.fPositionX - 9.f;
			lockedPlayer.fPositionZ = pacingBoss.fPositionZ + 5.f;
			advanceBoss(pacingBoss, 1u);
			trackedIndependentLockedTargetMove =
				lockedPizzaTarget == pacingBoss.iPatternTargetEntityId &&
				nonlockedPlayer.iNetEntityId == pacingBoss.iTargetEntityId &&
				yawMatches(
					pacingBoss.fYawDegrees,
					trackedPizzaYaw(lockedPlayer));

			const std::uint32_t initialPizzaStageIndex =
				pacingBoss.iPatternStageIndex;
			for (std::uint32_t tick = 0u;
				tick < 60u &&
				"VALTAN_SIX_PIZZA_106" == pacingBoss.strPatternId &&
				initialPizzaStageIndex == pacingBoss.iPatternStageIndex;
				++tick)
			{
				advanceBoss(pacingBoss, 1u);
			}
			const bool crossedIntoNextPizzaStage =
				"VALTAN_SIX_PIZZA_106" == pacingBoss.strPatternId &&
				initialPizzaStageIndex + 1u == pacingBoss.iPatternStageIndex &&
				"STEP_02" == pacingBoss.strPatternStageId;
			lockedPlayer.fPositionX = pacingBoss.fPositionX + 7.f;
			lockedPlayer.fPositionZ = pacingBoss.fPositionZ - 11.f;
			nonlockedPlayer.fPositionX = pacingBoss.fPositionX;
			nonlockedPlayer.fPositionZ = pacingBoss.fPositionZ + 0.25f;
			advanceBoss(pacingBoss, 1u);
			retainedAndTrackedAcrossStage =
				crossedIntoNextPizzaStage &&
				lockedPizzaTarget == pacingBoss.iPatternTargetEntityId &&
				nonlockedPlayer.iNetEntityId == pacingBoss.iTargetEntityId &&
				yawMatches(
					pacingBoss.fYawDegrees,
					trackedPizzaYaw(lockedPlayer));
		}
		tests.Require(
			INVALID_NET_ENTITY_ID != lockedPizzaTarget &&
			retainedRandomTargetWhileNearestChanged,
			"Keep six-pizza on its random start target when another alive player becomes nearest");
		tests.Require(
			trackedIndependentLockedTargetMove,
			"Track six-pizza yaw from arena center to its locked target after that target moves");
		tests.Require(
			retainedAndTrackedAcrossStage,
			"Retain the same six-pizza target and facing after advancing from STEP_01 to STEP_02");
		entrancePlayers.erase(pizzaSecondTarget.iPlayerId);
		entrancePlayers.begin()->second.fPositionX = 164.03f;
		entrancePlayers.begin()->second.fPositionZ = -122.06f;

		entrancePlayers.begin()->second.iCurrentHp = 5500u;
		entrancePlayers.begin()->second.eAction = PLAYER_ACTION_STATE::NONE;
		entrancePlayers.begin()->second.isCombatReady = true;
		SERVER_WORLD_ENTITY automaticBoss = makeEntranceBoss();
		std::vector<std::string> observedOrder;
		bool observedFloorWipeDeath = false;
		for (std::uint32_t tick = 0u;
			tick < 4000u && !observedFloorWipeDeath; ++tick)
		{
			advanceBoss(automaticBoss, 1u, &observedOrder);
			const SERVER_PLAYER& player = entrancePlayers.begin()->second;
			observedFloorWipeDeath = 0u == player.iCurrentHp &&
				PLAYER_ACTION_STATE::DEAD == player.eAction &&
				"VALTAN_FLOOR_WIPE_130" == automaticBoss.strPatternId &&
				5u == automaticBoss.iRotationStepIndex &&
				automaticBoss.bAutomaticPatternSequenceStepRunning;
		}
		for (std::uint32_t tick = 0u;
			tick < 300u && automaticBoss.iRotationStepIndex < 6u; ++tick)
		{
			advanceBoss(automaticBoss, 1u, &observedOrder);
		}
		const std::vector<std::string> phaseOneThroughWipe{
			"VALTAN_ENTRANCE_CINEMATIC",
			"VALTAN_WHIRLWIND",
			"VALTAN_FOUR_SLASH",
			"VALTAN_FIST_IN_OUT",
			"VALTAN_HIGH_JUMP",
			"VALTAN_FLOOR_WIPE_130" };
		const bool heldDashCursorWhileDead =
			observedFloorWipeDeath && observedOrder == phaseOneThroughWipe &&
			6u == automaticBoss.iRotationStepIndex &&
			30u == automaticBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			!automaticBoss.bAutomaticPatternSequenceStepRunning &&
			!automaticBoss.bMechanicLedgerRequiresReset &&
			automaticBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::IDLE == automaticBoss.eAction &&
			PLAYER_ACTION_STATE::DEAD == entrancePlayers.begin()->second.eAction;
		advanceBoss(automaticBoss, 60u, &observedOrder);
		const bool stayedAtDashCursorUntilRevive =
			observedOrder == phaseOneThroughWipe &&
			6u == automaticBoss.iRotationStepIndex &&
			30u == automaticBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			!automaticBoss.bAutomaticPatternSequenceStepRunning &&
			!automaticBoss.bMechanicLedgerRequiresReset &&
			automaticBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::IDLE == automaticBoss.eAction;
		SERVER_PLAYER& revivedPlayer = entrancePlayers.begin()->second;
		revivedPlayer.iCurrentHp = revivedPlayer.iMaximumHp;
		revivedPlayer.eAction = PLAYER_ACTION_STATE::NONE;
		revivedPlayer.isCombatReady = true;
		advanceBoss(automaticBoss, 1u, &observedOrder);
		const bool resumedPursuitImmediatelyAfterRevive =
			6u == automaticBoss.iRotationStepIndex &&
			29u == automaticBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			automaticBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::CHASE == automaticBoss.eAction;
		/* The ordering fixture drives the counter branch only after actual
		attachment capture. NONE/PARTIAL/ALL and impact transactions are exercised
		by the dedicated room fixture above. Auxiliary authored slots stay intact. */
		bool publishedTrashCounter = false;
		bool trashCounterReachedGroggy = false;
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto trashSequenceRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& trashSequenceRoom = *trashSequenceRoomStorage;
		automaticBoss.iNetEntityId = 19700u;
		for (std::uint32_t tick = 0u; tick < 8000u; ++tick)
		{
			SERVER_PLAYER& sequencePlayer = entrancePlayers.begin()->second;
			if (PLAYER_ACTION_STATE::DEAD == sequencePlayer.eAction)
			{
				sequencePlayer.iCurrentHp = sequencePlayer.iMaximumHp;
				sequencePlayer.eAction = PLAYER_ACTION_STATE::NONE;
				sequencePlayer.isCombatReady = true;
			}
			if ("VALTAN_TRASH" == automaticBoss.strPatternId &&
				("STEP_08" == automaticBoss.strPatternStageId ||
				 "GROGGY" == automaticBoss.strPatternStageId))
			{
				trashSequenceRoom.m_WorldEntities = { automaticBoss };
				trashSequenceRoom.m_Players = entrancePlayers;
				trashSequenceRoom.m_PlayerIdByEntityId.clear();
				for (const auto& [id, player] : entrancePlayers)
					trashSequenceRoom.m_PlayerIdByEntityId.emplace(player.iNetEntityId, id);
				if ("STEP_08" == automaticBoss.strPatternStageId)
				{
					for (const auto& [id, player] : entrancePlayers)
					{
						(void)id;
						(void)trashSequenceRoom.Capture_PlayerAttachment(player.iNetEntityId,
							automaticBoss.iNetEntityId, PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, automaticTick);
					}
				}
				else
					(void)trashSequenceRoom.Release_PlayerAttachments(automaticBoss.iNetEntityId,
						0.f, 0u, false, 0u, automaticTick);
				entrancePlayers = trashSequenceRoom.m_Players;
			}
			if (!publishedTrashCounter && "VALTAN_TRASH" == automaticBoss.strPatternId &&
				"STEP_07" == automaticBoss.strPatternStageId)
			{
				publishedTrashCounter = CBossCombatRuntime::Publish_PatternOutcome(
					automaticBoss, BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT, automaticTick);
			}
			advanceBoss(automaticBoss, 1u, &observedOrder);
			trashCounterReachedGroggy = trashCounterReachedGroggy ||
				("VALTAN_TRASH" == automaticBoss.strPatternId &&
				 "GROGGY" == automaticBoss.strPatternStageId);
		}
		const bool stoppedAtTerminalStep =
			publishedTrashCounter && observedOrder == expectedOrder &&
			"sequence.valtan.server-authored.v1" ==
				automaticBoss.strRotationId &&
			reviewSequence.iExpectedStepCount == automaticBoss.iRotationStepIndex &&
			entrancePlayers.begin()->second.iCurrentHp > 0u &&
			!automaticBoss.bAutomaticPatternSequenceStepRunning &&
			automaticBoss.strPatternId.empty() &&
			automaticBoss.PendingPatternIds.empty() &&
			automaticBoss.MechanicOccurrences.empty() &&
			SERVER_ENTITY_ACTION::IDLE == automaticBoss.eAction;
		tests.Require(
			heldDashCursorWhileDead &&
			stayedAtDashCursorUntilRevive &&
			resumedPursuitImmediatelyAfterRevive &&
			trashCounterReachedGroggy && stoppedAtTerminalStep,
			"Finish the lethal floor wipe, hold Dash until revive, then run the remaining Phase-1 transition and all 20 Phase-2 animations exactly once before terminal IDLE");

		SERVER_PLAYER& pausePlayer = entrancePlayers.begin()->second;
		pausePlayer.iCurrentHp = pausePlayer.iMaximumHp;
		pausePlayer.eAction = PLAYER_ACTION_STATE::NONE;
		pausePlayer.isCombatReady = true;
		SERVER_WORLD_ENTITY pausedBoss = makeEntranceBoss();
		std::vector<std::string> pausedOrder;
		advanceBoss(pausedBoss, 1u, &pausedOrder);
		const std::string pausedPatternId = pausedBoss.strPatternId;
		const std::uint32_t targetlessPatternSequence =
			pausedBoss.iPatternSequence;
		pausePlayer.iCurrentHp = 0u;
		pausePlayer.eAction = PLAYER_ACTION_STATE::DEAD;
		pausePlayer.isCombatReady = false;
		advanceBoss(pausedBoss, 60u, &pausedOrder);
		const bool waitedIdleAtOrderedCursor =
			pausedOrder == std::vector<std::string>{ "VALTAN_ENTRANCE_CINEMATIC" } &&
			0u == pausedBoss.iRotationStepIndex &&
			!pausedBoss.bAutomaticPatternSequenceStepRunning &&
			!pausedBoss.bAutomaticPatternSequencePausedForRevive &&
			0u == pausedBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			!pausedBoss.bMechanicLedgerRequiresReset &&
			pausedBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::IDLE == pausedBoss.eAction &&
			targetlessPatternSequence ==
				pausedBoss.PatternTerminalReceipt.iPatternSequence &&
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED ==
				pausedBoss.PatternTerminalReceipt.eResult;
		pausePlayer.iCurrentHp = pausePlayer.iMaximumHp;
		pausePlayer.eAction = PLAYER_ACTION_STATE::NONE;
		pausePlayer.isCombatReady = true;
		advanceBoss(pausedBoss, 1u, &pausedOrder);
		const bool retriedSameOrderedStep =
			!pausedBoss.bAutomaticPatternSequencePausedForRevive &&
			0u == pausedBoss.iAutomaticPatternSequencePauseLastTick &&
			pausedBoss.strPatternId == pausedPatternId &&
			pausedBoss.iPatternSequence > targetlessPatternSequence &&
			0u == pausedBoss.iRotationStepIndex &&
			pausedBoss.bAutomaticPatternSequenceStepRunning &&
			!pausedBoss.bMechanicLedgerRequiresReset;
		for (std::uint32_t tick = 0u;
			tick < 600u && 0u == pausedBoss.iRotationStepIndex; ++tick)
		{
			advanceBoss(pausedBoss, 1u, &pausedOrder);
		}
		const bool advancedOnlyAfterRevive =
			1u == pausedBoss.iRotationStepIndex &&
			!pausedBoss.bMechanicLedgerRequiresReset;
		tests.Require(
			waitedIdleAtOrderedCursor && retriedSameOrderedStep &&
			advancedOnlyAfterRevive,
			"Wait IDLE without mechanic reset while an ordered Valtan step has no valid target, then retry the same stable cursor after target admission");

		SERVER_WORLD_ENTITY disconnectedBoss = makeEntranceBoss();
		std::vector<std::string> disconnectedOrder;
		advanceBoss(disconnectedBoss, 1u, &disconnectedOrder);
		const SERVER_PLAYER disconnectedPlayer = entrancePlayers.begin()->second;
		entrancePlayers.clear();
		advanceBoss(disconnectedBoss, 1u, &disconnectedOrder);
		const bool emptyRoomWaitedIdle =
			disconnectedOrder ==
				std::vector<std::string>{ "VALTAN_ENTRANCE_CINEMATIC" } &&
			0u == disconnectedBoss.iRotationStepIndex &&
			!disconnectedBoss.bAutomaticPatternSequenceStepRunning &&
			!disconnectedBoss.bAutomaticPatternSequencePausedForRevive &&
			0u == disconnectedBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			!disconnectedBoss.bMechanicLedgerRequiresReset &&
			disconnectedBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::IDLE == disconnectedBoss.eAction;
		entrancePlayers.emplace(
			disconnectedPlayer.iPlayerId, disconnectedPlayer);
		advanceBoss(disconnectedBoss, 1u, &disconnectedOrder);
		const bool restartedAfterPlayerReturned =
			0u == disconnectedBoss.iRotationStepIndex &&
			disconnectedBoss.bAutomaticPatternSequenceStepRunning &&
			"VALTAN_ENTRANCE_CINEMATIC" == disconnectedBoss.strPatternId &&
			!disconnectedBoss.bMechanicLedgerRequiresReset;
		tests.Require(
			emptyRoomWaitedIdle && restartedAfterPlayerReturned,
			"Keep an empty-room or disconnect target loss IDLE without mechanic reset and retry when a valid player returns");
		entrancePlayers.begin()->second.iCurrentHp = 1000000u;
		entrancePlayers.begin()->second.isCombatReady = true;
		entrancePlayers.begin()->second.fPositionX = 164.03f;
		entrancePlayers.begin()->second.fPositionZ = -122.06f;

		/* Rotations remain admitted for authoring and explicit Debug evaluation,
		but the Product sequence above is the only automatic selector consumer. */
		const BOSS_PATTERN_ROTATION_DEFINITION* openingSpan =
			catalog.Find_BossPatternRotation("ENCOUNTER_VALTAN", 1u, 160u);
		const BOSS_PATTERN_ROTATION_DEFINITION* secondSpan =
			catalog.Find_BossPatternRotation("ENCOUNTER_VALTAN", 1u, 129u);
		const BOSS_PATTERN_ROTATION_DEFINITION* firstLegacySpan =
			catalog.Find_BossPatternRotation("ENCOUNTER_VALTAN", 2u, 108u);
		const bool managedPoolsMatch = nullptr != openingSpan &&
			nullptr != secondSpan &&
			openingSpan->Candidates.size() == secondSpan->Candidates.size() &&
			std::equal(
				openingSpan->Candidates.begin(), openingSpan->Candidates.end(),
				secondSpan->Candidates.begin(),
				[](const BOSS_PATTERN_ROTATION_CANDIDATE& left,
					const BOSS_PATTERN_ROTATION_CANDIDATE& right)
				{
					return left.strPatternId == right.strPatternId &&
						left.iSelectionWeight == right.iSelectionWeight &&
						left.bEnabled == right.bEnabled;
				});
		tests.Require(
			nullptr != openingSpan &&
			BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
				openingSpan->eSelectionMode &&
			160u == openingSpan->iFromHealthBar &&
			130u == openingSpan->iToHealthBar &&
			6u == openingSpan->Candidates.size() &&
			openingSpan == catalog.Find_BossPatternRotation(
				"ENCOUNTER_VALTAN", 1u, 131u) &&
			nullptr != secondSpan && openingSpan != secondSpan &&
			BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
				secondSpan->eSelectionMode &&
			130u == secondSpan->iFromHealthBar &&
			109u == secondSpan->iToHealthBar &&
			secondSpan == catalog.Find_BossPatternRotation(
				"ENCOUNTER_VALTAN", 1u, 130u) &&
			managedPoolsMatch &&
			nullptr == catalog.Find_BossPatternRotation(
				"ENCOUNTER_VALTAN", 2u, 159u) &&
			nullptr != firstLegacySpan &&
			BOSS_PATTERN_ROTATION_SELECTION_MODE::
				ORDERED_INTRO_THEN_WEIGHTED == firstLegacySpan->eSelectionMode &&
			firstLegacySpan == catalog.Find_BossPatternRotation(
				"ENCOUNTER_VALTAN", 1u, 108u) &&
			nullptr == catalog.Find_BossPatternRotation(
				"ENCOUNTER_VALTAN", 1u, 1u),
			"Retain phase-owned rotation data for authoring without using it as automatic fallback");
	}
}

