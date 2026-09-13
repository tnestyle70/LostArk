#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameplayCatalog.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_GenerationRetention(TESTS& tests, CGameplayCatalog& catalog)
{

	{
		const VALTAN_TIMELINE_DEFINITION* timeline =
			catalog.Find_ValtanTimeline("ENCOUNTER_VALTAN");
		bool hasExactOrder = nullptr != timeline &&
			timeline->strTimelineId == "VALTAN_AUDITION_TIMELINE" &&
			timeline->Rows.size() == 52u;
		if (hasExactOrder)
		{
			for (std::uint32_t ordinal = 1u; ordinal <= 52u; ++ordinal)
			{
				const VALTAN_TIMELINE_ROW& row = timeline->Rows[ordinal - 1u];
				hasExactOrder = row.iOrdinal == ordinal &&
					!row.strRowId.empty() &&
					row.iCommandId ==
						Calculate_TestTimelineCommandId(row.strRowId) &&
					&row == catalog.Find_ValtanTimelineRow(
						"ENCOUNTER_VALTAN", row.iCommandId) &&
					!row.PatternActions.empty();
				if (!hasExactOrder)
					break;
			}
		}
		tests.Require(
			hasExactOrder && 160u == timeline->Rows[0].iSectionHealthBar &&
			"valtan.timeline.160-entrance-whirlwind" ==
				timeline->Rows[0].strRowId &&
			VALTAN_TIMELINE_ARENA_STATE::ORDINARY_WALLS_GONE ==
				timeline->Rows[19].eArenaState &&
			VALTAN_TIMELINE_ARENA_STATE::FLOOR84_AND_30_GONE ==
				timeline->Rows[42].eArenaState &&
			2u == timeline->Rows[28].PatternActions.size() &&
			"VALTAN_MAGIC_CHOICE" ==
				timeline->Rows[28].PatternActions[0].strPatternId &&
			"VALTAN_RED_BLADE_WAVE" ==
				timeline->Rows[28].PatternActions[1].strPatternId &&
			2u == timeline->Rows[2].PatternActions.size() &&
			1u == timeline->Rows[15].PatternActions.size() &&
			2u == timeline->Rows[31].PatternActions.size(),
			"Load the 52-row Valtan timeline with pattern-before-row bootstrap ordering");
	}
	{
		const std::vector<BOSS_PATTERN_DEFINITION>* patterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto findStage = [patterns](
			const std::string& patternId,
			const std::string& stageId) -> const BOSS_PATTERN_STAGE_DEFINITION*
		{
			if (nullptr == patterns)
				return nullptr;
			for (const BOSS_PATTERN_DEFINITION& pattern : *patterns)
			{
				if (pattern.strPatternId != patternId)
					continue;
				const auto stage = std::find_if(
					pattern.Stages.begin(), pattern.Stages.end(),
					[&stageId](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
					{ return candidate.strStageId == stageId; });
				return stage == pattern.Stages.end() ? nullptr : &*stage;
			}
			return nullptr;
		};
		const BOSS_PATTERN_STAGE_DEFINITION* swing =
			findStage("VALTAN_SWING", "SWEEP");
		const BOSS_PATTERN_STAGE_DEFINITION* downSmash =
			findStage("VALTAN_DOWN_SMASH", "IMPACT");
		const BOSS_PATTERN_STAGE_DEFINITION* floorWipe =
			findStage("VALTAN_FLOOR_WIPE_130", "FIRST_SMASH");
		const BOSS_PATTERN_STAGE_DEFINITION* roar =
			findStage("VALTAN_IMPRISON_ROAR", "ROAR");
		const BOSS_PATTERN_STAGE_DEFINITION* fourSlashes =
			findStage("VALTAN_FOUR_SLASH", "SLASHES");
		const BOSS_PATTERN_STAGE_DEFINITION* fourSlashSpin =
			findStage("VALTAN_FOUR_SLASH", "SPIN");
		const BOSS_PATTERN_STAGE_DEFINITION* highJumpTakeoff =
			findStage("VALTAN_HIGH_JUMP", "TAKEOFF");
		const BOSS_PATTERN_STAGE_DEFINITION* highJumpAirborne =
			findStage("VALTAN_HIGH_JUMP", "AIRBORNE");
		const BOSS_PATTERN_STAGE_DEFINITION* highJumpLand =
			findStage("VALTAN_HIGH_JUMP", "LAND");
		const BOSS_PATTERN_STAGE_DEFINITION* highJumpRecovery =
			findStage("VALTAN_HIGH_JUMP", "RECOVERY");
		const BOSS_COMBAT_OBJECT_DEFINITION* highJumpTargetAxe =
			catalog.Find_BossCombatObject(
				"combatobject.valtan.high-jump.target-axe");
		const BOSS_PATTERN_STAGE_DEFINITION* groundRoarStep =
			findStage("VALTAN_GROUND_ROAR", "STEP_01");
		const BOSS_COMBAT_OBJECT_DEFINITION* groundRoarRock =
			catalog.Find_BossCombatObject(
				"combatobject.valtan.ground-roar.rock");
		const BOSS_PATTERN_STAGE_DEFINITION* strugglingRockStep =
			findStage("VALTAN_STRUGGLING", "STEP_04");
		const BOSS_PATTERN_STAGE_DEFINITION* partBreakStage =
			findStage("VALTAN_PART_BREAK", "PART_BREAK");
		const BOSS_PATTERN_STAGE_DEFINITION* partBreakRecovery =
			findStage("VALTAN_PART_BREAK", "PART_BREAK_RECOVERY");
		const BOSS_COMBAT_OBJECT_DEFINITION* partBreakRock =
			catalog.Find_BossCombatObject(
				"combatobject.valtan.part-break.rock");
		const auto findPattern = [patterns](
			const std::string& patternId) -> const BOSS_PATTERN_DEFINITION*
		{
			if (nullptr == patterns)
				return nullptr;
			const auto found = std::find_if(
				patterns->begin(), patterns->end(),
				[&patternId](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == patternId; });
			return patterns->end() == found ? nullptr : &*found;
		};
		const BOSS_PATTERN_DEFINITION* swingPattern =
			findPattern("VALTAN_SWING");
		const BOSS_PATTERN_DEFINITION* fourSlashPattern =
			findPattern("VALTAN_FOUR_SLASH");
		const BOSS_PATTERN_DEFINITION* highJumpPattern =
			findPattern("VALTAN_HIGH_JUMP");
		const BOSS_PATTERN_DEFINITION* arenaBreakPattern =
			findPattern("VALTAN_ARENA_BREAK_109");
		const BOSS_PATTERN_DEFINITION* entranceCinematic =
			findPattern("VALTAN_ENTRANCE_CINEMATIC");
		const BOSS_PATTERN_STAGE_DEFINITION* entranceEstablish =
			findStage("VALTAN_ENTRANCE_CINEMATIC", "ESTABLISH");
		const BOSS_PATTERN_STAGE_DEFINITION* entranceArenaReveal =
			findStage("VALTAN_ENTRANCE_CINEMATIC", "ARENA_REVEAL");
		const BOSS_PATTERN_STAGE_DEFINITION* entranceHeroHandoff =
			findStage("VALTAN_ENTRANCE_CINEMATIC", "HERO_HANDOFF");
		const auto hasAction = [](
			const BOSS_PATTERN_STAGE_DEFINITION* stage,
			const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			const BOSS_PATTERN_STAGE_ACTION_KIND kind,
			const std::string_view targetId,
			const std::uint32_t value)
		{
			return nullptr != stage && std::any_of(
				stage->Actions.begin(), stage->Actions.end(),
				[trigger, kind, targetId, value](
					const BOSS_PATTERN_STAGE_ACTION& action)
				{
					return action.eTrigger == trigger && action.eKind == kind &&
						action.strTargetId == targetId && action.iValue == value &&
						0u == action.iDurationMs;
				});
		};
		const auto hasBranch = [](
			const BOSS_PATTERN_STAGE_DEFINITION* stage,
			const BOSS_PATTERN_STAGE_OUTCOME outcome,
			const std::string_view nextActionId)
		{
			return nullptr != stage && std::any_of(
				stage->Branches.begin(), stage->Branches.end(),
				[outcome, nextActionId](
					const BOSS_PATTERN_STAGE_BRANCH& branch)
				{
					return branch.eOutcome == outcome &&
						branch.strNextActionId == nextActionId;
				});
		};
		const auto hasPatternFollowup = [](
			const BOSS_PATTERN_STAGE_DEFINITION* stage,
			const BOSS_PATTERN_STAGE_OUTCOME outcome,
			const std::string_view nextPatternId)
		{
			return nullptr != stage && std::any_of(
				stage->Branches.begin(), stage->Branches.end(),
				[outcome, nextPatternId](const BOSS_PATTERN_STAGE_BRANCH& branch)
				{
					return branch.eOutcome == outcome &&
						branch.strNextActionId.empty() &&
						branch.strNextPatternId == nextPatternId;
				});
		};
		const auto hasClosedFlag = [&hasAction](
			const BOSS_PATTERN_STAGE_DEFINITION* stage,
			const std::string_view targetId)
		{
			return nullptr != stage && 2u == stage->Actions.size() &&
				hasAction(stage, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
					BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG,
					targetId, 1u) &&
				hasAction(stage, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
					BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG,
					targetId, 0u);
		};
		std::size_t valtanLivePatternCount = 0u;
		std::size_t valtanStageCount = 0u;
		std::size_t valtanStageActionCount = 0u;
		std::size_t valtanStageVolleyActionCount = 0u;
		std::size_t valtanRuntimeBranchCount = 0u;
		std::size_t valtanMotionCount = 0u;
		if (nullptr != patterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *patterns)
			{
				if (BOSS_PATTERN_SELECTION::AUDITION_ONLY == pattern.eSelection)
					continue;
				++valtanLivePatternCount;
				valtanStageCount += pattern.Stages.size();
				for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
				{
					valtanStageActionCount += stage.Actions.size();
					valtanStageVolleyActionCount += static_cast<std::size_t>(std::count_if(
						stage.Actions.begin(), stage.Actions.end(), [](const BOSS_PATTERN_STAGE_ACTION& action)
						{ return BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY == action.eKind; }));
					valtanRuntimeBranchCount += stage.Branches.size();
					if (BOSS_PATTERN_STAGE_MOTION_KIND::NONE != stage.Motion.eKind)
						++valtanMotionCount;
				}
			}
		}
		const BOSS_PATTERN_STAGE_DEFINITION* parryStance =
			findStage("VALTAN_PARRY", "STANCE");
		const BOSS_PATTERN_STAGE_DEFINITION* parrySlash =
			findStage("VALTAN_PARRY", "COUNTER_SLASH");
		const BOSS_PATTERN_STAGE_DEFINITION* parryNormal =
			findStage("VALTAN_PARRY", "NORMAL_SLASH");
		const BOSS_PATTERN_DEFINITION* triplePattern =
			findPattern("VALTAN_TRIPLE_COUNTER");
		const BOSS_PATTERN_STAGE_DEFINITION* tripleSetup =
			findStage("VALTAN_TRIPLE_COUNTER", "SETUP");
		const BOSS_PATTERN_STAGE_DEFINITION* tripleFirst =
			findStage("VALTAN_TRIPLE_COUNTER", "COUNTER_1");
		const BOSS_PATTERN_STAGE_DEFINITION* tripleFirstFail =
			findStage("VALTAN_TRIPLE_COUNTER", "FAIL_1");
		const BOSS_PATTERN_STAGE_DEFINITION* tripleSecond =
			findStage("VALTAN_TRIPLE_COUNTER", "COUNTER_2");
		const BOSS_PATTERN_STAGE_DEFINITION* tripleSecondFail =
			findStage("VALTAN_TRIPLE_COUNTER", "FAIL_2");
		const BOSS_PATTERN_STAGE_DEFINITION* tripleThird =
			findStage("VALTAN_TRIPLE_COUNTER", "COUNTER_3");
		const BOSS_PATTERN_STAGE_DEFINITION* tripleThirdFail =
			findStage("VALTAN_TRIPLE_COUNTER", "FAIL_3");
		const BOSS_PATTERN_STAGE_DEFINITION* armorCharge =
			findStage("VALTAN_ARMOR_BREAK_OPENING", "WALL_CHARGE");
		const BOSS_PATTERN_STAGE_DEFINITION* armorGroggy =
			findStage("VALTAN_ARMOR_BREAK_OPENING", "GROGGY");
		const BOSS_PATTERN_STAGE_DEFINITION* orbShield =
			findStage("VALTAN_MAGIC_ORB_STAGGER_76", "SHIELD");
		const BOSS_PATTERN_STAGE_DEFINITION* orbWindow =
			findStage("VALTAN_MAGIC_ORB_STAGGER_76", "STAGGER_WINDOW");
		const BOSS_PATTERN_STAGE_DEFINITION* orbGroggy =
			findStage("VALTAN_MAGIC_ORB_STAGGER_76", "GROGGY");
		const BOSS_PATTERN_STAGE_DEFINITION* orbWipe =
			findStage("VALTAN_MAGIC_ORB_STAGGER_76", "WIPE");
		const BOSS_PATTERN_STAGE_DEFINITION* centerCounter =
			findStage("VALTAN_CENTER_GRAB_COUNTER_64", "COUNTER_WINDOW");
		const BOSS_PATTERN_STAGE_DEFINITION* counterSlam =
			findStage("VALTAN_COUNTER", "STEP_03");
		const BOSS_PATTERN_STAGE_DEFINITION* counterWindow =
			findStage("VALTAN_COUNTER", "STEP_02");
		const BOSS_PATTERN_DEFINITION* retiredCounterGroggyPattern =
			findPattern("VALTAN_COUNTER_GROGGY");
		const BOSS_PATTERN_DEFINITION* sharedGroggyPattern =
			findPattern("VALTAN_GROGGY_FOLLOWUP");
		const BOSS_PATTERN_STAGE_DEFINITION* sharedGroggy =
			findStage("VALTAN_GROGGY_FOLLOWUP", "GROGGY");
		const auto hasForwardCounter = [&hasClosedFlag](
			const BOSS_PATTERN_STAGE_DEFINITION* stage)
		{
			return hasClosedFlag(stage, "boss.flag.counterable") &&
				stage->bHasCounterProxy &&
				BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC ==
					stage->eCounterProxyKind &&
				0.f == stage->fCounterProxyForwardOffsetM &&
				0.f == stage->fCounterProxyRightOffsetM &&
				0.f == stage->fCounterProxyRadiusM &&
				180.f == stage->fCounterProxyArcDegrees;
		};
		const auto hasTripleSlam = [&hasBranch](
			const BOSS_PATTERN_STAGE_DEFINITION* stage,
			const std::string_view nextActionId)
		{
			return nullptr != stage &&
				BOSS_PATTERN_STAGE_KIND::ACTIVE == stage->eStageKind &&
				1667u == stage->iDurationMs &&
				!stage->bHasCounterProxy && stage->Actions.empty() &&
				BOSS_PATTERN_HIT_SHAPE::CIRCLE == stage->eHitShape &&
				std::abs(stage->fHitOuterRadius - 12.f) < 1.0e-6f &&
				std::vector<std::uint32_t>{ 900u } == stage->HitOffsetsMs &&
				"damage.valtan.triple-counter" == stage->strDamageProfileId &&
				1u == stage->Branches.size() &&
				hasBranch(stage, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
					nextActionId);
		};
		const bool entranceCameraGateExact =
			nullptr != entranceCinematic &&
			BOSS_PATTERN_SELECTION::NORMAL == entranceCinematic->eSelection &&
			entranceCinematic->bInvulnerableWhileRunning &&
			BOSS_PATTERN_TARGET_POLICY::NONE ==
				entranceCinematic->eTargetPolicy &&
			BOSS_PATTERN_AIM_POLICY::NONE == entranceCinematic->eAimPolicy &&
			3u == entranceCinematic->Stages.size() &&
			nullptr != entranceEstablish &&
			8600u == entranceEstablish->iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::NONE == entranceEstablish->eHitShape &&
			entranceEstablish->Actions.empty() &&
			hasBranch(entranceEstablish, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.cinematic.entrance.arena-reveal") &&
			nullptr != entranceArenaReveal &&
			5800u == entranceArenaReveal->iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::NONE == entranceArenaReveal->eHitShape &&
			entranceArenaReveal->Actions.empty() &&
			hasBranch(entranceArenaReveal, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.cinematic.entrance.hero-handoff") &&
			nullptr != entranceHeroHandoff &&
			5467u == entranceHeroHandoff->iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::NONE == entranceHeroHandoff->eHitShape &&
			entranceHeroHandoff->Actions.empty() &&
			hasBranch(entranceHeroHandoff, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT, "");
		tests.Require(
			entranceCameraGateExact,
			"Load the exact invulnerable 19.867-second Valtan entrance camera gate");

		const bool reactiveTopologyExact = nullptr != patterns &&
			nullptr != parryStance && 2u == parryStance->Actions.size() &&
			2u == parryStance->Branches.size() &&
			hasAction(parryStance,
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE,
				"boss.gauge.stagger", 30u) &&
			hasAction(parryStance,
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE,
				"boss.gauge.stagger", 0u) &&
			hasBranch(parryStance, BOSS_PATTERN_STAGE_OUTCOME::STAGGER_BROKEN,
				"valtan.reactive.parry.slash") &&
			hasBranch(parryStance, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reactive.parry.normal-slash") &&
			nullptr != parrySlash && 1u == parrySlash->Branches.size() &&
			hasBranch(parrySlash, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reactive.parry.recovery") &&
			nullptr != parryNormal && 1u == parryNormal->Branches.size() &&
			hasBranch(parryNormal, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reactive.parry.recovery") &&
			nullptr != triplePattern && 7u == triplePattern->Stages.size() &&
			nullptr != tripleSetup &&
			BOSS_PATTERN_STAGE_KIND::WINDUP == tripleSetup->eStageKind &&
			2000u == tripleSetup->iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::NONE == tripleSetup->eHitShape &&
			!tripleSetup->bHasCounterProxy && tripleSetup->Actions.empty() &&
			1u == tripleSetup->Branches.size() &&
			hasBranch(tripleSetup, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reactive.triple-counter.first") &&
			hasForwardCounter(tripleFirst) &&
			BOSS_PATTERN_STAGE_KIND::WINDUP == tripleFirst->eStageKind &&
			1800u == tripleFirst->iDurationMs &&
			2u == tripleFirst->Branches.size() &&
			hasPatternFollowup(tripleFirst,
				BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT,
				"VALTAN_GROGGY_FOLLOWUP") &&
			hasBranch(tripleFirst, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reactive.triple-counter.first-fail") &&
			hasTripleSlam(tripleFirstFail,
				"valtan.reactive.triple-counter.second") &&
			hasForwardCounter(tripleSecond) &&
			BOSS_PATTERN_STAGE_KIND::WINDUP == tripleSecond->eStageKind &&
			1800u == tripleSecond->iDurationMs &&
			2u == tripleSecond->Branches.size() &&
			hasPatternFollowup(tripleSecond,
				BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT,
				"VALTAN_GROGGY_FOLLOWUP") &&
			hasBranch(tripleSecond, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reactive.triple-counter.second-fail") &&
			hasTripleSlam(tripleSecondFail,
				"valtan.reactive.triple-counter.third") &&
			hasForwardCounter(tripleThird) &&
			BOSS_PATTERN_STAGE_KIND::WINDUP == tripleThird->eStageKind &&
			1800u == tripleThird->iDurationMs &&
			2u == tripleThird->Branches.size() &&
			hasPatternFollowup(tripleThird,
				BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT,
				"VALTAN_GROGGY_FOLLOWUP") &&
			hasBranch(tripleThird, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reactive.triple-counter.third-fail") &&
			hasTripleSlam(tripleThirdFail, "") &&
			nullptr != armorCharge &&
			BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD == armorCharge->Motion.eKind &&
			std::abs(armorCharge->Motion.fDistance - 20.f) < 1.0e-6f &&
			2u == armorCharge->Branches.size() &&
			hasBranch(armorCharge, BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT,
				"valtan.mechanic.armor-break-opening.groggy") &&
			hasBranch(armorCharge, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT, "") &&
			hasClosedFlag(armorGroggy, "boss.flag.groggy") &&
			2u == armorGroggy->Branches.size() &&
			hasBranch(armorGroggy, BOSS_PATTERN_STAGE_OUTCOME::PART_DESTROYED,
				"valtan.mechanic.armor-break-opening.recovery") &&
			hasBranch(armorGroggy, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.mechanic.armor-break-opening.recovery") &&
			nullptr != orbShield && 2u == orbShield->Actions.size() &&
			hasAction(orbShield, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG,
				"boss.flag.invulnerable", 1u) &&
			hasAction(orbShield, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_SHIELD,
				"boss.gauge.shield", 6000u) &&
			nullptr != orbWindow && 4u == orbWindow->Actions.size() &&
			2u == orbWindow->Branches.size() &&
			hasAction(orbWindow, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE,
				"boss.gauge.stagger", 100u) &&
			hasAction(orbWindow, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE,
				"boss.gauge.stagger", 0u) &&
			hasAction(orbWindow, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_SHIELD,
				"boss.gauge.shield", 0u) &&
			hasAction(orbWindow, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG,
				"boss.flag.invulnerable", 0u) &&
			hasBranch(orbWindow, BOSS_PATTERN_STAGE_OUTCOME::STAGGER_BROKEN,
				"valtan.mechanic.magic-orb-stagger-76.groggy") &&
			hasBranch(orbWindow, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.mechanic.magic-orb-stagger-76.wipe") &&
			hasClosedFlag(orbGroggy, "boss.flag.groggy") &&
			1u == orbGroggy->Branches.size() &&
			hasBranch(orbGroggy, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.mechanic.magic-orb-stagger-76.recovery") &&
			nullptr != orbWipe && 1u == orbWipe->Branches.size() &&
			hasBranch(orbWipe, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.mechanic.magic-orb-stagger-76.recovery") &&
			hasClosedFlag(centerCounter, "boss.flag.counterable") &&
			2u == centerCounter->Branches.size() &&
			hasBranch(centerCounter, BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT,
				"valtan.mechanic.center-grab-counter-64.recovery") &&
			hasBranch(centerCounter, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.mechanic.center-grab-counter-64.failed-charge") &&
			nullptr != counterSlam && 1667u == counterSlam->iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::CIRCLE == counterSlam->eHitShape &&
			std::abs(counterSlam->fHitOuterRadius - 12.f) < 1.0e-6f &&
			std::vector<std::uint32_t>{ 900u } == counterSlam->HitOffsetsMs &&
			1u == counterSlam->Branches.size() &&
			hasBranch(counterSlam, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT, "") &&
			nullptr != counterWindow &&
			BOSS_PATTERN_STAGE_KIND::WINDUP == counterWindow->eStageKind &&
			1800u == counterWindow->iDurationMs &&
			hasClosedFlag(counterWindow, "boss.flag.counterable") &&
			2u == counterWindow->Branches.size() &&
			hasPatternFollowup(counterWindow,
				BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT,
				"VALTAN_GROGGY_FOLLOWUP") &&
			hasBranch(counterWindow, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.sequence.counter.step-03") &&
			nullptr == retiredCounterGroggyPattern &&
			nullptr != sharedGroggyPattern && nullptr != sharedGroggy &&
			BOSS_PATTERN_SELECTION::AUDITION_ONLY ==
				sharedGroggyPattern->eSelection &&
			BOSS_PATTERN_TARGET_POLICY::NONE ==
				sharedGroggyPattern->eTargetPolicy &&
			BOSS_PATTERN_AIM_POLICY::NONE == sharedGroggyPattern->eAimPolicy &&
			hasClosedFlag(sharedGroggy, "boss.flag.groggy");
		tests.Require(reactiveTopologyExact,
			"Load the named reactive Valtan actions, branches, motions, and terminal counter slam without constraining unrelated Pattern totals");
		if (!reactiveTopologyExact)
		{
			std::cout << "[STATUS] Valtan topology counts: patterns=" << valtanLivePatternCount
				<< ", stages=" << valtanStageCount << ", actions=" << valtanStageActionCount
				<< ", volleys=" << valtanStageVolleyActionCount << ", branches=" << valtanRuntimeBranchCount
				<< ", motions=" << valtanMotionCount << '\n';
		}
		tests.Require(
			nullptr != fourSlashes && 3u == fourSlashes->iHitCount &&
			0u == fourSlashes->iHitDelayMs &&
			0u == fourSlashes->iHitIntervalMs &&
			std::vector<std::uint32_t>{ 1790u, 2560u, 3330u } ==
				fourSlashes->HitOffsetsMs &&
			nullptr != fourSlashSpin && 1u == fourSlashSpin->iHitCount &&
			0u == fourSlashSpin->iHitDelayMs &&
			0u == fourSlashSpin->iHitIntervalMs &&
			std::vector<std::uint32_t>{ 600u } ==
				fourSlashSpin->HitOffsetsMs &&
			fourSlashes->bWallContact && fourSlashSpin->bWallContact,
			"Compile the rejoined four-slash explicit Server hit schedule exactly");
		const bool hasExactHighJumpVolley = nullptr != highJumpAirborne &&
			1u == highJumpAirborne->Actions.size() &&
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
				highJumpAirborne->Actions.front().eTrigger &&
			BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY ==
				highJumpAirborne->Actions.front().eKind &&
			"combatobject.valtan.high-jump.target-axe" ==
				highJumpAirborne->Actions.front().strTargetId &&
			1u == highJumpAirborne->Actions.front().iValue &&
			0u == highJumpAirborne->Actions.front().iDurationMs &&
			BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER ==
				highJumpAirborne->Actions.front().Volley.ePolicy &&
			1u == highJumpAirborne->Actions.front().Volley.iCountPerResolvedTarget &&
			BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE ==
				highJumpAirborne->Actions.front().Volley.eLayout &&
			0.f == highJumpAirborne->Actions.front().Volley.fRadiusM &&
			0.f == highJumpAirborne->Actions.front().Volley.fStartAngleDegrees &&
			0.f == highJumpAirborne->Actions.front().Volley.fAngleStepDegrees &&
			!highJumpAirborne->Actions.front().Volley.bAllowOverlap &&
			/* Restored 2026-09-03: three waves 1333 ms apart, one tracking axe per
			   alive raider plus four arena-random axes around the boss spawn. */
			36u == highJumpAirborne->Actions.front().Volley.iMaximumTotalObjects &&
			3u == highJumpAirborne->Actions.front().Volley.iSpawnCount &&
			0u == highJumpAirborne->Actions.front().Volley.iFirstSpawnOffsetMs &&
			1333u == highJumpAirborne->Actions.front().Volley.iSpawnIntervalMs &&
			4u == highJumpAirborne->Actions.front().Volley.iArenaRandomCount &&
			14.f == highJumpAirborne->Actions.front().Volley.fArenaRandomRadiusM &&
			1.f == highJumpAirborne->Actions.front().Volley.fArenaHeightToleranceM &&
			BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION ==
				highJumpAirborne->Actions.front().Volley.eArenaAnchorPolicy;
		tests.Require(
			nullptr != highJumpTakeoff &&
			1933u == highJumpTakeoff->iDurationMs &&
			nullptr != highJumpAirborne &&
			8000u == highJumpAirborne->iDurationMs &&
			hasExactHighJumpVolley &&
			nullptr != highJumpLand && 3200u == highJumpLand->iDurationMs &&
			nullptr != highJumpRecovery &&
			400u == highJumpRecovery->iDurationMs &&
			nullptr != highJumpTargetAxe &&
			highJumpAirborne->iDurationMs == highJumpTargetAxe->iLifeMs &&
			BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER ==
				highJumpTargetAxe->eOriginPolicy &&
			1u == highJumpTargetAxe->Hits.size() &&
			BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED ==
				highJumpTargetAxe->Hits.front().eTrigger &&
			1200u == highJumpTargetAxe->Hits.front().iAtMs &&
			1u == highJumpTargetAxe->Hits.front().iRepeatCount,
			"Derive each target-axe Product lifetime from the 8-second AIRBORNE clock, keep its timed contact object-local at +1.2 seconds, and leave LAND at 3.2 seconds");
		const bool hasExactGroundRoarCardinalRocks =
			nullptr != groundRoarStep &&
			BOSS_PATTERN_HIT_SHAPE::CIRCLE == groundRoarStep->eHitShape &&
			std::abs(groundRoarStep->fHitOuterRadius - 12.f) < 0.0001f &&
			std::vector<std::uint32_t>{ 600u, 1300u, 2700u } ==
				groundRoarStep->HitOffsetsMs &&
			"damage.valtan.ledge-roar" ==
				groundRoarStep->strDamageProfileId &&
			1u == groundRoarStep->Actions.size() &&
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
				groundRoarStep->Actions.front().eTrigger &&
			BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY ==
				groundRoarStep->Actions.front().eKind &&
			"combatobject.valtan.ground-roar.rock" ==
				groundRoarStep->Actions.front().strTargetId &&
			4u == groundRoarStep->Actions.front().iValue &&
			0u == groundRoarStep->Actions.front().iDurationMs &&
			BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE ==
				groundRoarStep->Actions.front().Volley.ePolicy &&
			4u == groundRoarStep->Actions.front().Volley.
				iCountPerResolvedTarget &&
			BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL ==
				groundRoarStep->Actions.front().Volley.eLayout &&
			std::abs(groundRoarStep->Actions.front().Volley.fRadiusM -
				6.3639610307f) <
				1.0e-6f &&
			45.f == groundRoarStep->Actions.front().Volley.fStartAngleDegrees &&
			90.f == groundRoarStep->Actions.front().Volley.fAngleStepDegrees &&
			!groundRoarStep->Actions.front().Volley.bAllowOverlap &&
			4u == groundRoarStep->Actions.front().Volley.iMaximumTotalObjects &&
			1u == groundRoarStep->Actions.front().Volley.iSpawnCount &&
			0u == groundRoarStep->Actions.front().Volley.iFirstSpawnOffsetMs &&
			0u == groundRoarStep->Actions.front().Volley.iSpawnIntervalMs &&
			0u == groundRoarStep->Actions.front().Volley.iArenaRandomCount &&
			0.f == groundRoarStep->Actions.front().Volley.fArenaRandomRadiusM &&
			0.f == groundRoarStep->Actions.front().Volley.fArenaHeightToleranceM &&
			BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::NONE ==
				groundRoarStep->Actions.front().Volley.eArenaAnchorPolicy &&
			nullptr != groundRoarRock &&
			"VALTAN_GROUND_ROAR" == groundRoarRock->strOwnerPatternId &&
			"valtan.sequence.sequence.400440.0.step-01" ==
				groundRoarRock->strOwnerStageActionId &&
			"combatobject.visual.valtan.ground-roar.rock.v1" ==
				groundRoarRock->strClientVisualId &&
			BOSS_COMBAT_OBJECT_KIND::FIXED_AREA == groundRoarRock->eKind &&
			BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION ==
				groundRoarRock->eOriginPolicy &&
			BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE ==
				groundRoarRock->eDirectionPolicy &&
			6200u == groundRoarRock->iLifeMs &&
			std::abs(groundRoarRock->fCoverRadiusM - 1.5f) < 0.0001f &&
			1u == groundRoarRock->Hits.size() &&
			groundRoarRock->PresentationPulses.empty() &&
			"hit.valtan.ground-roar.rock.explode" ==
				groundRoarRock->Hits.front().strHitId &&
			BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED ==
				groundRoarRock->Hits.front().eTrigger &&
			5000u == groundRoarRock->Hits.front().iAtMs &&
			1u == groundRoarRock->Hits.front().iRepeatCount &&
			BOSS_PATTERN_HIT_SHAPE::CIRCLE ==
				groundRoarRock->Hits.front().eHitShape &&
			std::abs(groundRoarRock->Hits.front().fHitOuterRadius - 3.f) <
				0.0001f &&
			"damage.valtan.stomp" ==
				groundRoarRock->Hits.front().strDamageProfileId;
		tests.Require(
			hasExactGroundRoarCardinalRocks && nullptr != entranceEstablish &&
				entranceEstablish->Actions.empty(),
			"Own the exact stomp/roar hit track and one four-root diagonal damaging rock volley in GROUND_ROAR STEP_01, and never place it in the entrance cinematic");
		const bool hasExactStrugglingRockAndImpactTrack =
			nullptr != strugglingRockStep &&
			BOSS_PATTERN_HIT_SHAPE::CIRCLE == strugglingRockStep->eHitShape &&
			std::abs(strugglingRockStep->fHitOuterRadius - 8.f) < 0.0001f &&
			std::vector<std::uint32_t>{ 1233u, 2233u, 3233u, 4200u } ==
				strugglingRockStep->HitOffsetsMs &&
			"damage.valtan.stomp" ==
				strugglingRockStep->strDamageProfileId &&
			1u == strugglingRockStep->Actions.size() &&
			BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY ==
				strugglingRockStep->Actions.front().eKind &&
			"combatobject.valtan.struggling.rock-pillar" ==
				strugglingRockStep->Actions.front().strTargetId;
		tests.Require(
			hasExactStrugglingRockAndImpactTrack,
			"Admit only the reviewed STRUGGLING STEP_04 boss impact track beside its delayed rock volley");
		const bool hasExactPartBreakRecoveryCardinalRocks =
			nullptr != partBreakStage && 1800u == partBreakStage->iDurationMs &&
			nullptr != partBreakRecovery &&
			BOSS_PATTERN_STAGE_KIND::RECOVERY ==
				partBreakRecovery->eStageKind &&
			5183u == partBreakRecovery->iDurationMs &&
			1u == partBreakRecovery->Actions.size() &&
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
				partBreakRecovery->Actions.front().eTrigger &&
			BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY ==
				partBreakRecovery->Actions.front().eKind &&
			"combatobject.valtan.part-break.rock" ==
				partBreakRecovery->Actions.front().strTargetId &&
			BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE ==
				partBreakRecovery->Actions.front().Volley.ePolicy &&
			4u == partBreakRecovery->Actions.front().Volley.
				iCountPerResolvedTarget &&
			BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL ==
				partBreakRecovery->Actions.front().Volley.eLayout &&
			std::abs(partBreakRecovery->Actions.front().Volley.fRadiusM -
				4.9497475f) < 1.0e-6f &&
			45.f == partBreakRecovery->Actions.front().Volley.
				fStartAngleDegrees &&
			90.f == partBreakRecovery->Actions.front().Volley.
				fAngleStepDegrees &&
			!partBreakRecovery->Actions.front().Volley.bAllowOverlap &&
			4u == partBreakRecovery->Actions.front().Volley.
				iMaximumTotalObjects &&
			1u == partBreakRecovery->Actions.front().Volley.iSpawnCount &&
			0u == partBreakRecovery->Actions.front().Volley.iFirstSpawnOffsetMs &&
			0u == partBreakRecovery->Actions.front().Volley.iSpawnIntervalMs &&
			0u == partBreakRecovery->Actions.front().Volley.iArenaRandomCount &&
			nullptr != partBreakRock &&
			"VALTAN_PART_BREAK" == partBreakRock->strOwnerPatternId &&
			"valtan.reaction.part-break.recovery" ==
				partBreakRock->strOwnerStageActionId &&
			"combatobject.visual.valtan.part-break.rock.v1" ==
				partBreakRock->strClientVisualId &&
			BOSS_COMBAT_OBJECT_KIND::FIXED_AREA == partBreakRock->eKind &&
			BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION ==
				partBreakRock->eOriginPolicy &&
			BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE ==
				partBreakRock->eDirectionPolicy &&
			6200u == partBreakRock->iLifeMs &&
			std::abs(partBreakRock->fCoverRadiusM - 1.5f) < 0.0001f &&
			1u == partBreakRock->Hits.size() &&
			partBreakRock->PresentationPulses.empty() &&
			"hit.valtan.part-break.rock.explode" ==
				partBreakRock->Hits.front().strHitId &&
			BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED ==
				partBreakRock->Hits.front().eTrigger &&
			5000u == partBreakRock->Hits.front().iAtMs &&
			BOSS_PATTERN_HIT_SHAPE::CIRCLE ==
				partBreakRock->Hits.front().eHitShape &&
			std::abs(partBreakRock->Hits.front().fHitOuterRadius - 3.f) <
				0.0001f &&
			"damage.valtan.stomp" ==
				partBreakRock->Hits.front().strDamageProfileId;
		tests.Require(
			hasExactPartBreakRecoveryCardinalRocks,
			"Split Part Break into 1800ms reaction plus 5183ms recovery and own one exact four-root damaging rock volley at recovery ENTER");
		tests.Require(
			nullptr != fourSlashPattern && nullptr != swingPattern &&
			420609u == fourSlashPattern->iSourcePrimaryActionId &&
			fourSlashPattern->iSourcePrimaryActionId !=
				swingPattern->iSourcePrimaryActionId,
			"Compile one four-slash source-action cooldown family and keep other sources independent");
		tests.Require(
			nullptr != highJumpPattern &&
			2u == highJumpPattern->Motion.iTravelStageIndex &&
			nullptr != arenaBreakPattern &&
			1u == arenaBreakPattern->Motion.iTravelStageIndex,
			"Compile the authored high-jump LAND descent and arena-break DROP descent stage indices");
		const bool everyPatternHasSourceTiming = nullptr != patterns &&
			std::all_of(
				patterns->begin(), patterns->end(),
				[](const BOSS_PATTERN_DEFINITION& pattern)
				{
					return 0u != pattern.iSourcePrimaryActionId &&
						pattern.iSourceShapeCount <= 256u &&
						pattern.iSourceCooldownTicks ==
						static_cast<std::uint32_t>(
							(static_cast<std::uint64_t>(
								pattern.iSourceCooldownMs) * 30u + 999u) /
							1000u);
				});
		tests.Require(
			nullptr != swing && swing->bWallContact &&
			BOSS_PATTERN_HIT_SHAPE::CONE == swing->eHitShape &&
			nullptr != downSmash && downSmash->bWallContact &&
			BOSS_PATTERN_HIT_SHAPE::CROSS == downSmash->eHitShape &&
			downSmash->fHitLength >= 9.9f &&
			downSmash->fHitHalfWidth >= 1.7f &&
			nullptr != roar && !roar->bWallContact,
			"Compile the down-smash and other allowlisted physical axe stages as wall contacts");
		tests.Require(
			nullptr != floorWipe && !floorWipe->bWallContact &&
			BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS == floorWipe->eHitShape &&
			floorWipe->fHitLength >= 13.9f &&
			floorWipe->fHitHalfWidth >= 2.1f,
			"Compile the 130 floor wipe as six Server-authoritative directions");
		tests.Require(
			everyPatternHasSourceTiming && nullptr != swingPattern &&
			420601u == swingPattern->iSourcePrimaryActionId &&
			12u == swingPattern->iSourceShapeCount &&
			5000u == swingPattern->iSourceCooldownMs &&
			150u == swingPattern->iSourceCooldownTicks &&
			350u == swingPattern->iSourceRangeUnits &&
			300u == swingPattern->iSourceApproachUnits &&
			180u == swingPattern->iSourceTurnDegrees &&
			nullptr != arenaBreakPattern &&
			420629u == arenaBreakPattern->iSourcePrimaryActionId &&
			0u == arenaBreakPattern->iSourceCooldownTicks &&
			10000u == arenaBreakPattern->iSourceRangeUnits,
			"Compile all Valtan entry-action timing records from Valtan.skilltiming");
		std::uint32_t damagingStageCount = 0u;
		std::uint32_t authoredHitPulseCount = 0u;
		bool everyDamagingStageResolves = nullptr != patterns;
		if (nullptr != patterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *patterns)
			{
				for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
				{
					if (BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape)
						continue;
					++damagingStageCount;
					const bool pulseSchedule =
						BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE ==
							stage.eHitActivationKind && 0u != stage.iHitCount;
					const bool activeWindow =
						BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW ==
							stage.eHitActivationKind && 0u == stage.iHitCount &&
						stage.iHitActivationLifetimeMs > 0u &&
						static_cast<std::uint64_t>(stage.iHitActivationStartMs) +
							stage.iHitActivationLifetimeMs <= stage.iDurationMs;
					authoredHitPulseCount += pulseSchedule ? stage.iHitCount :
						(activeWindow ? 1u : 0u);
					everyDamagingStageResolves = everyDamagingStageResolves &&
						(pulseSchedule || activeWindow) &&
						!stage.strDamageProfileId.empty() &&
						0u != catalog.Find_DamageRatePercent(
							stage.strDamageProfileId);
				}
			}
		}
		tests.Require(
			everyDamagingStageResolves && 0u != damagingStageCount &&
			authoredHitPulseCount >= damagingStageCount &&
			700u == catalog.Find_DamageRatePercent(
				"damage.valtan.arena-destroy-109") &&
			450u == catalog.Find_DamageRatePercent(
				"damage.valtan.six-direction-130") &&
			900u == catalog.Find_DamageRatePercent(
				"damage.valtan.ghost-transition-15"),
			"Resolve every authored Valtan hit stage and pulse through project-tuned damage profiles");
	}
}

