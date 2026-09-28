#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include "Network/PacketReader.h"
#include "Gameplay/WorldCollisionContract.h"
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
        std::vector<BALANCE_NUMERIC_ENTRY> entries, applied;
        GameplayDataRevision beforeNumeric, afterNumeric, beforeBytes, afterBytes, beforeShape, afterShape;
        std::string status, bytes, hitKey;
        std::vector<BALANCE_NUMERIC_CHANGE> changes;
        const bool enumerated = catalog.Build_NumericBalanceSnapshot(entries,beforeNumeric,status);
        for (const auto& entry : entries)
        {
            double next=entry.fValue;
            if (entry.eDomain==BALANCE_DOMAIN::PLAYER && entry.strId=="LANCE_MASTER" &&
                (entry.strField=="maximumHp" || entry.strField=="attackPower")) next*=2.;
            else if (entry.eDomain==BALANCE_DOMAIN::BOSS && entry.strField=="maximumHp" &&
                (entry.strId=="BOSS_VALTAN" || entry.strId=="BOSS_VALTAN_GHOST")) next=std::floor(next/2.);
            else if (entry.eDomain==BALANCE_DOMAIN::SKILL && entry.strId=="34010" && entry.strField=="staggerDamage") next=17.;
            else if (entry.eDomain==BALANCE_DOMAIN::PATTERN_DAMAGE && entry.strId.starts_with("H|") && hitKey.empty())
            { hitKey=entry.strId; next=entry.fValue==10. ? 20. : 10.; }
            if (next!=entry.fValue) changes.push_back({entry.eDomain,entry.strId,entry.strField,entry.fValue,next});
        }
        auto candidate=std::make_shared<CGameplayCatalog>();
        const bool prepared=enumerated && !hitKey.empty() && candidate->Load_NumericBalancePatch(catalog,changes,bytes);
        tests.Require(prepared,"Validate numeric balance patches through the real Product catalogue parser");
        if (prepared)
        {
            const bool receipt=catalog.Build_NumericBalanceReceiptHashes(beforeBytes,beforeShape,status) &&
                candidate->Build_NumericBalanceReceiptHashes(afterBytes,afterShape,status) &&
                candidate->Build_NumericBalanceSnapshot(applied,afterNumeric,status);
            tests.Require(receipt && beforeNumeric!=afterNumeric && beforeBytes!=afterBytes && beforeShape==afterShape &&
                candidate->Get_ActiveRevision()==catalog.Get_ActiveRevision() &&
                candidate->Get_ValtanPresentationGenerationId()==catalog.Get_ValtanPresentationGenerationId(),
                "Separate numeric hash changes while choreography and presentation admission identities remain exact");
            CGameplayCatalog rejected=catalog;
            auto stale=changes; stale.front().fBefore+=1.;
            tests.Require(!rejected.Load_NumericBalancePatch(catalog,stale,bytes) &&
                rejected.Get_ActiveRevision()==catalog.Get_ActiveRevision(),"Reject stale numeric CAS without replacing the catalogue");
            const auto* initialBoss=catalog.Find_Boss("BOSS_VALTAN");
            const std::vector<BALANCE_NUMERIC_CHANGE> invalidBars{{BALANCE_DOMAIN::BOSS,"BOSS_VALTAN","maximumHealthBars",double(initialBoss->iMaximumHealthBars),1.}};
            tests.Require(!rejected.Load_NumericBalancePatch(catalog,invalidBars,bytes),"Reject maximum bars incompatible with the saved raid threshold program");
            const auto h=std::find_if(entries.begin(),entries.end(),[&](const auto& e){return e.strId==hitKey;});
            const std::vector<BALANCE_NUMERIC_CHANGE> invalidPercent{{BALANCE_DOMAIN::PATTERN_DAMAGE,hitKey,"maxHpDamagePercent",h->fValue,101.}};
            tests.Require(!rejected.Load_NumericBalancePatch(catalog,invalidPercent,bytes),"Reject percentage damage above one hundred without changing the damage mode");
            auto base=std::make_shared<CGameplayCatalog>(catalog);
            auto room=std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA,base);
            const bool ready=room->Is_Ready() && room->Activate_Encounter("boss.valtan.center");
            auto* boss=ready ? room->Find_AuditionBoss() : nullptr;
            tests.Require(boss!=nullptr,"Initialize the numeric migration fixture on the published Valtan room");
            if (boss)
            {
                const auto* player=catalog.Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER);
                const auto* ghost=catalog.Find_Boss("BOSS_VALTAN_GHOST");
                for (std::uint32_t id=1u;id<=4u;++id)
                {
                    SERVER_PLAYER state; state.iPlayerId=id; state.iNetEntityId=100u+id; state.eCharacterClass=CHARACTER_CLASS_ID::LANCE_MASTER;
                    state.iMaximumHp=player->iMaximumHp; state.iCurrentHp=id==4u ? 0u : player->iMaximumHp/2u;
                    state.iMaximumResource=player->iMaximumResource; state.iCurrentResource=player->iMaximumResource/2u;
                    state.iMaximumIdentity=player->iMaximumIdentity; state.iCurrentIdentity=0u;
                    SERVER_SKILL_PROJECTILE projectile; projectile.iSkillId=34010u; projectile.iTotalDamage=100u; projectile.iAppliedTimedMask.set(0u);
                    state.Projectiles.push_back(projectile); room->m_Players.emplace(id,std::move(state));
                }
                boss->iPhase=3u; boss->bGhostPhasePatternLoopActive=true; boss->iMaximumHp=ghost->iMaximumHp;
                boss->iCurrentHp=ghost->iMaximumHp/2u; boss->iMaximumHealthBars=ghost->iMaximumHealthBars; boss->iPatternSequence=77u;
                const auto id=boss->iNetEntityId, hp=boss->iCurrentHp;
                auto objectTx=room->m_CombatObjectRuntime.Begin_Transaction();
                SERVER_COMBAT_OBJECT object; object.iCombatObjectId=17u; object.eSourceKind=SERVER_COMBAT_OBJECT_SOURCE_KIND::WORLD_ENTITY;
                object.strSourceArchetypeId="BOSS_VALTAN_GHOST"; object.fElapsedMilliseconds=321.f;
                SERVER_COMBAT_OBJECT_HIT_RUNTIME hit; hit.strNumericBalanceId=hitKey; hit.iDamagePercent=std::uint32_t(h->fValue);
                hit.iAppliedTimedCount=1u; hit.RepeatRawDamage={1u,1u}; object.Hits.push_back(hit); objectTx.Objects.push_back(object);
                const bool objectReady=room->m_CombatObjectRuntime.Commit(std::move(objectTx));
                const auto generations=room->m_GameplayCatalog.Get_GenerationCount();
                const bool staged=objectReady && room->Stage_NumericBalance(9001u,candidate,status);
                room->m_Players.at(1u).iCurrentHp-=1u; const auto liveHp=room->m_Players.at(1u).iCurrentHp;
                const bool committed=staged && room->Commit_NumericBalance(9001u);
                const auto& live=room->m_CombatObjectRuntime.Get_LiveObjects();
                tests.Require(committed && room->m_Players.size()==4u && room->m_Players.at(1u).iCurrentHp==liveHp*2u &&
                    room->m_Players.at(4u).iCurrentHp==0u && room->m_Players.at(1u).Projectiles.front().iTotalDamage>=100u &&
                    room->m_Players.at(1u).Projectiles.front().iAppliedTimedMask.test(0u),
                    "Commit four players from their current HP ratio, preserving deaths and in-flight projectile hit history");
                tests.Require(committed && boss->iNetEntityId==id && boss->iPatternSequence==77u && boss->iPhase==3u &&
                    boss->iMaximumHp==ghost->iMaximumHp/2u && std::abs(double(boss->iCurrentHp)-hp/2.)<=1. &&
                    room->m_GameplayCatalog.Get_GenerationCount()==generations && !live.empty() && live.front().fElapsedMilliseconds==321.f &&
                    live.front().Hits.front().iAppliedTimedCount==1u && live.front().Hits.front().iDamagePercent==(h->fValue==10. ? 20u : 10u),
                    "Preserve ghost identity/sequence and refresh next attack-object percent without replay or generation growth");
                const bool restaged=room->Stage_NumericBalance(9002u,base,status); room->Abort_NumericBalance(9002u);
                tests.Require(restaged && !room->Commit_NumericBalance(9002u) && room->Get_ActiveGameplayGeneration()==candidate,
                    "Abort staged room numeric values without changing the active balance");
            }
        }
    }

	{
		const BOSS_PATTERN_SEQUENCE_DEFINITION* sequence =
			catalog.Find_BossPatternSequence("ENCOUNTER_VALTAN");
		tests.Require(
			nullptr != sequence && !sequence->strSequenceId.empty() &&
			(BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE == sequence->eMode ||
			 BOSS_PATTERN_SEQUENCE_MODE::HEALTH_BAR_ROTATIONS == sequence->eMode) &&
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
		// The saved Death timeout owns its Respawn follow-up, before the next
		// explicit Play All slot and without an inter-slot pursuit delay.
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		const bool activated = room.Is_Ready() && room.Activate_Encounter("boss.valtan.center");
		const auto* patterns = room.m_GameplayCatalog.Active().Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto findPattern = [patterns](const std::string_view id) -> const BOSS_PATTERN_DEFINITION*
		{
			if (nullptr == patterns) return nullptr;
			const auto found = std::find_if(patterns->begin(), patterns->end(),
				[id](const auto& value) { return value.strPatternId == id; });
			return patterns->end() == found ? nullptr : &*found;
		};
		const auto* death = findPattern("VALTAN_GHOST_DEATH_AUDITION");
		const auto* respawn = findPattern("VALTAN_GHOST_RESPAWN_AUDITION");
		const bool deathOwnsRespawn = nullptr != death && 1u == death->Stages.size() &&
			1 == std::count_if(death->Stages.front().Branches.begin(), death->Stages.front().Branches.end(),
				[](const auto& branch) { return BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT == branch.eOutcome &&
					branch.strNextActionId.empty() && "VALTAN_GHOST_RESPAWN_AUDITION" == branch.strNextPatternId; });
		const bool respawnOwnsPhase = nullptr != respawn && 1u == respawn->Stages.size() &&
			1 == std::count_if(respawn->Stages.front().Actions.begin(), respawn->Stages.front().Actions.end(),
				[](const auto& action) { return BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == action.eTrigger &&
					BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE == action.eKind &&
					"boss.phase.gameplay" == action.strTargetId && 3u == action.iValue; });
		tests.Require(activated && deathOwnsRespawn && respawnOwnsPhase,
			"Load the saved Ghost Death TIMEOUT-to-Respawn branch and Respawn phase-3 entry action");
		if (activated && deathOwnsRespawn && respawnOwnsPhase)
		{
			SERVER_WORLD_ENTITY boss = *room.Find_AuditionBoss();
			boss.eAction = SERVER_ENTITY_ACTION::IDLE;
			boss.bIntroPatternConsumed = true;
			boss.iLastEvaluatedHealthBar = CValtanBrain::Calculate_HealthBar(boss);
			SERVER_PLAYER player{};
			player.iPlayerId = 991u; player.iNetEntityId = 1991u;
			player.iCurrentHp = player.iMaximumHp = 100000u; player.isCombatReady = true;
			player.fPositionX = boss.fPositionX; player.fPositionY = boss.fPositionY; player.fPositionZ = boss.fPositionZ;
			std::map<PLAYER_ID, SERVER_PLAYER> players{{ player.iPlayerId, player }};
			BOSS_PATTERN_SEQUENCE_DEFINITION audition{};
			audition.strEncounterId = boss.strEncounterId;
			audition.strSequenceId = "sequence.valtan.contract.death-followup";
			audition.eMode = BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE;
			audition.PatternIds = { death->strPatternId, "VALTAN_WHIRLWIND" };
			audition.iExpectedStepCount = 2u;
			audition.iInterStepPursuitTicks = 30u;
			CValtanBrain brain;
			std::vector<DAMAGE_EVENT> damage;
			const auto& active = room.m_GameplayCatalog.Active();
			const auto update = [&](SERVER_WORLD_ENTITY& value, const std::uint32_t tick)
			{ brain.Update(value, players, active, room.m_ServerNavigation, 1.f / 30.f,
				tick, {}, damage, &active, room.m_GameplayCatalog.Get_ActiveGenerationEpoch(), nullptr, &audition); };
			update(boss, 100u);
			const std::uint32_t endTick = boss.iPatternStageFirstEvaluationTick +
				(death->Stages.front().iDurationMs * 30u + 999u) / 1000u - 1u;
			update(boss, endTick - 1u);
			const bool heldUntilDeadline = death->strPatternId == boss.strPatternId && !boss.PendingPatternFollowup.Is_Pending();
			update(boss, endTick);
			const bool queuedAtDeadline = boss.strPatternId.empty() &&
				"VALTAN_GHOST_RESPAWN_AUDITION" == boss.PendingPatternFollowup.strPatternId &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == boss.PatternTerminalReceipt.eResult &&
				1u == boss.iRotationStepIndex && 30u == boss.iAutomaticPatternSequencePursuitTicksRemaining;
			tests.Require(heldUntilDeadline && queuedAtDeadline,
				"Actual Death deadline completes its Play All slot and queues the authored Respawn follow-up");
			SERVER_WORLD_ENTITY held = boss;
			held.bAutomaticPatternSequenceAuditionHold = true;
			update(held, endTick + 1u);
			tests.Require(held.bAutomaticPatternSequenceAuditionHold && held.strPatternId.empty() &&
				held.PendingPatternFollowup.Is_Pending(), "Debug hold preserves the queued Death follow-up without advancing it");
			update(boss, endTick + 1u);
			const bool started = respawn->strPatternId == boss.strPatternId &&
				1u == boss.iPatternFollowupDepth && 1u == boss.iRotationStepIndex &&
				SERVER_ENTITY_ACTION::CHASE != boss.eAction;
			const bool phaseEntered = started && room.Apply_BossPatternStageTransition(boss, {}, {},
				boss.strPatternId, boss.strActionId, active.Get_ActiveRevision(),
				boss.PinnedDefinitionRevision, endTick + 1u) && 3u == boss.iPhase;
			tests.Require(started && phaseEntered && !boss.bMechanicLedgerRequiresReset,
				"Respawn follow-up starts on the next tick before Play All pursuit and enters phase 3");
		}
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
			0u < dashGroggy->iDurationMs &&
			BOSS_PATTERN_PART_DAMAGE_POLICY::DESTROY_FIRST_ELIGIBLE ==
				dashGroggy->ePartDamagePolicy &&
			hasBossFlagAction(
				dashGroggy, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
				"boss.flag.groggy", 1u) &&
			hasBossFlagAction(
				dashGroggy, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
				"boss.flag.groggy", 0u),
			"Load Dash and its saved finite GROGGY continuation as one three-stage pattern");
		if (nullptr != dash && 3u == dash->Stages.size() && nullptr != dashGroggy)
		{
			SERVER_WORLD_ENTITY boss{};
			boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS; boss.iNetEntityId = 19910u;
			boss.strArchetypeId = "BOSS_VALTAN"; boss.strEncounterId = "ENCOUNTER_VALTAN";
			boss.iCurrentHp = boss.iMaximumHp = 100000u; boss.iMaximumHealthBars = 160u;
			boss.iLastEvaluatedHealthBar = 160u; boss.iPhase = 1u; boss.bIntroPatternConsumed = true;
			boss.bScriptedPatternPlayback = true; boss.fEngageDistance = 100.f;
			boss.PendingPatternIds.push_back(dash->strPatternId);
			SERVER_PLAYER player{};
			player.iPlayerId = 19911u; player.iNetEntityId = 19912u;
			player.iCurrentHp = player.iMaximumHp = 100000u; player.isCombatReady = true;
			player.fPositionZ = 50.f;
			std::map<PLAYER_ID, SERVER_PLAYER> players{{ player.iPlayerId, player }};
			CServerNavigation navigation;
			CValtanBrain brain;
			std::vector<DAMAGE_EVENT> damage;
			const auto update = [&](const std::uint32_t tick)
			{ brain.Update(boss, players, catalog, navigation, 1.f / 30.f, tick, {}, damage); };
			const auto ticks = [](const BOSS_PATTERN_STAGE_DEFINITION& stage)
			{ return (stage.iDurationMs * 30u + 999u) / 1000u; };
			update(100u);
			const auto chargeTick = boss.iPatternStageFirstEvaluationTick + ticks(dash->Stages[0]) - 1u;
			update(chargeTick);
			const bool chargeEntered = "CHARGE" == boss.strPatternStageId;
			const auto groggyTick = boss.iPatternStageFirstEvaluationTick + ticks(dash->Stages[1]) - 1u;
			update(groggyTick);
			const bool groggyEntered = "GROGGY" == boss.strPatternStageId &&
				boss.iPatternStageDurationMs == dashGroggy->iDurationMs;
			const auto doneTick = boss.iPatternStageFirstEvaluationTick + ticks(*dashGroggy) - 1u;
			update(doneTick - 1u);
			const bool heldBeforeDeadline = "GROGGY" == boss.strPatternStageId;
			update(doneTick);
			tests.Require(chargeEntered && groggyEntered && heldBeforeDeadline && boss.strPatternId.empty() &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == boss.PatternTerminalReceipt.eResult,
				"Dash enters and completes GROGGY on the published stage durations, preserving the last pre-deadline tick");
		}
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
			const float beforeRightError = std::abs(std::remainder(boss.fYawDegrees - expectedYaw, 360.f));
			room.Tick(1.f / 30.f);
			const float afterRightError = std::abs(std::remainder(boss.fYawDegrees - expectedYaw, 360.f));
			const bool followedSameTargetRight = room.Is_Ready() &&
				19800u == boss.iPatternTargetEntityId && afterRightError < beforeRightError && afterRightError > 0.001f;

			positionsValid = placePlayer(19700u, 15.f, 0.f) &&
				placePlayer(19701u, -5.f, 0.f) && positionsValid;
			expectedYaw = yawToward(room.m_Players.at(19701u));
			const float beforeLeftError = std::abs(std::remainder(boss.fYawDegrees - expectedYaw, 360.f));
			room.Tick(1.f / 30.f);
			float previousError = std::abs(std::remainder(boss.fYawDegrees - expectedYaw, 360.f));
			const bool switchedToNearerTargetLeft = room.Is_Ready() &&
				19801u == boss.iPatternTargetEntityId && previousError < beforeLeftError;
			const float initialConvergenceError = previousError;
			bool convergedMonotonically = true;
			for (std::uint32_t tick = 0u; tick < 30u; ++tick)
			{
				room.Tick(1.f / 30.f);
				const float error = std::abs(std::remainder(boss.fYawDegrees -
					yawToward(room.m_Players.at(19701u)), 360.f));
				convergedMonotonically = convergedMonotonically && room.Is_Ready() &&
					19801u == boss.iPatternTargetEntityId && error <= previousError + 0.001f;
				previousError = error;
			}
			tests.Require(chargeTracksNearest && positionsValid && enteredFacingLeft &&
				followedSameTargetRight && switchedToNearerTargetLeft && convergedMonotonically &&
				previousError < initialConvergenceError * 0.25f,
				"Charge switches nearest target immediately while saved slow aim converges monotonically below a quarter of its initial error");
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
		grabBoss.fPositionX = 156.03f;
		grabBoss.fPositionY = 22.97f;
		grabBoss.fPositionZ = -122.06f;
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
			const auto* profile = catalog.Find_Player(player.eCharacterClass);
			player.iCurrentResource = player.iMaximumResource =
				nullptr == profile ? 0u : profile->iMaximumResource;
			player.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
			player.isCombatReady = true;
			player.fPositionX = grabBoss.fPositionX + 1.f - 2.f * static_cast<float>(ordinal);
			player.fPositionY = grabBoss.fPositionY + 1.f + static_cast<float>(ordinal);
			player.fPositionZ = grabBoss.fPositionZ + 2.f + static_cast<float>(ordinal);
			grabRoom.m_PlayerIdByEntityId.emplace(
				player.iNetEntityId, player.iPlayerId);
			grabRoom.m_Players.emplace(player.iPlayerId, player);
		}
		const bool capturedFirst = grabRoom.Capture_PlayerAttachment(
			9300u, 9100u, PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 10u);
		const bool capturedSecond = grabRoom.Capture_PlayerAttachment(
			9301u, 9100u, PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 10u);
		SERVER_WORLD_ENTITY& movingBoss = grabRoom.m_WorldEntities.front();
		movingBoss.fPositionX += 1.f;
		movingBoss.fPositionZ += 1.f;
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
			2u == released && PLAYER_ACTION_STATE::KNOCKDOWN == first.eAction &&
			first.bPushOnlyHitReaction &&
			PLAYER_ATTACHMENT_SLOT::NONE == first.eAttachmentSlot &&
			INVALID_NET_ENTITY_ID == first.iAttachmentOwnerNetEntityId &&
			std::abs(first.fKnockbackRemainingSeconds - 0.5f) < 0.000001f &&
			std::abs(first.fKnockbackSpeed - 12.f) < 0.000001f &&
			!first.hasMoveGoal && INVALID_SKILL_ID == first.iCurrentSkillId,
			"Release every captured player outward and reject move or skill overlap during knockback");
		const std::uint32_t releaseCompletionTick = first.iKnockdownEndTick;
		for (std::uint32_t tick = 13u; tick <= releaseCompletionTick && tick < 100u; ++tick)
		{
			grabRoom.m_iServerTick = tick - 1u;
			grabRoom.Update_Players(1.f / 30.f);
		}
		const bool releaseCompleted = PLAYER_ACTION_STATE::NONE == first.eAction &&
			!first.bPushOnlyHitReaction && first.isCombatReady &&
			0.f == first.fKnockbackRemainingSeconds && 0.f == first.fKnockbackSpeed &&
			0u == first.iKnockdownEndTick && 0u == first.iHitReactionGraceEndTick;
		C2S_USE_SKILL resumedSkill = blockedSkill;
		resumedSkill.iClientSequence = 2u;
		resumedSkill.fAimX = first.fPositionX + 1.f;
		resumedSkill.fAimZ = first.fPositionZ;
		grabRoom.Handle_UseSkill(9400u, resumedSkill);
		tests.Require(releaseCompleted && PLAYER_ACTION_STATE::SKILL == first.eAction &&
			resumedSkill.iSkillId == first.iCurrentSkillId,
			"Grab release finishes its push-only recovery on room ticks and accepts skill input afterward");
		const bool recaptured = grabRoom.Capture_PlayerAttachment(
			first.iNetEntityId, 9100u,
			PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, releaseCompletionTick + 2u);
		grabRoom.m_WorldEntities.clear();
		const bool keptMissingOwner =
			grabRoom.Update_PlayerAttachment(first, releaseCompletionTick + 3u);
		tests.Require(
			recaptured && !keptMissingOwner &&
			PLAYER_ACTION_STATE::NONE == first.eAction &&
			INVALID_NET_ENTITY_ID == first.iAttachmentOwnerNetEntityId &&
			PLAYER_ATTACHMENT_SLOT::NONE == first.eAttachmentSlot,
			"Release a captured player fail-closed when its boss owner disappears");
	}
	{
		// Observe one continuous Product encounter. Only fixture player survival
		// and incoming boss HP are controlled; Room owns every stage and phase.
		auto room = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		std::vector<std::shared_ptr<CClientSession>> sessions;
		const auto drain = [&sessions]()
		{
			for (const auto& session : sessions)
			{
				session->m_OutboundFrames.clear();
				session->m_iQueuedOutboundBytes = 0u;
			}
		};
		const std::array classes{ CHARACTER_CLASS_ID::LANCE_MASTER, CHARACTER_CLASS_ID::ARTIST,
			CHARACTER_CLASS_ID::WARLORD, CHARACTER_CLASS_ID::DIMENSIONMASTER };
		bool joined = room->Is_Ready();
		for (std::size_t index = 0u; index < classes.size() && joined; ++index)
		{
			const SESSION_ID sessionId = 98701u + index;
			auto session = std::make_shared<CClientSession>(sessionId, INVALID_SOCKET,
				CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
			session->m_isSendRunning.store(true);
			room->Handle_Register(session);
			sessions.push_back(session);
			C2S_ENTER_WORLD entry{};
			entry.eWorldId = WORLD_ID::VALTAN_ARENA;
			entry.eCharacterClass = classes[index];
			entry.strNickName = "ValtanProgression" + std::to_string(index + 1u);
			joined = room->Join(sessionId, entry);
			drain();
		}
		tests.Require(joined && 4u == room->Count_HumanPlayers() && 4u == room->m_Players.size(),
			"Four real session admissions share the published Valtan room");
		const auto* trigger = room->Find_Placement("Stage_Boss");
		const auto* arrival = room->Find_Placement("Stage_Boss_ArenaEntry");
		bool entered = joined && trigger && arrival;
		if (entered)
		{
			for (auto& [id, player] : room->m_Players)
			{
				(void)id;
				player.fPositionX = trigger->fPositionX;
				player.fPositionY = trigger->fPositionY - WorldCollision::PLAYER_CENTER_OFFSET_Y;
				player.fPositionZ = trigger->fPositionZ;
				player.iInvulnerableEndTick = 200000u;
				// Real entrants walk to G; this accepted command marks admission combat-ready.
				C2S_MOVE move{}; move.iClientSequence = 1u;
				move.fGoalX = player.fPositionX; move.fGoalZ = player.fPositionZ;
				room->Handle_Move(player.iSessionId, move);
			}
			room->Tick(1.f / 30.f);
			drain();
			for (const auto& session : sessions)
			{
				C2S_INTERACT_TRIGGER interact{};
				interact.iRequestSequence = 1u;
				interact.strTriggerPlacementId = "Stage_Boss";
				room->Handle_InteractTrigger(session->Get_SessionId(), interact);
			}
			entered = room->Find_AuditionBoss() && std::all_of(room->m_Players.begin(), room->m_Players.end(),
				[](const auto& pair) { return pair.second.TriggerMove.isActive; });
		}
		tests.Require(entered, "All four typed G requests enter the same encounter through authored movement");
		if (entered)
		{
			struct WINDOW final { std::uint8_t phase; std::uint32_t bars; std::uint32_t gate; const char* mechanic; };
			const std::array windows{
				WINDOW{1,160,130,"VALTAN_FLOOR_WIPE_130"}, WINDOW{1,130,115,"VALTAN_ARENA_BREAK_109"},
				WINDOW{2,115,105,"VALTAN_SIX_PIZZA_106"}, WINDOW{2,105,80,"VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK"},
				WINDOW{2,80,65,"VALTAN_TRASH"}, WINDOW{2,65,30,"VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK"},
				WINDOW{2,30,15,"VALTAN_STRUGGLING"}, WINDOW{3,40,0,""} };
			const NET_ENTITY_ID primaryId = room->Find_AuditionBoss()->iNetEntityId;
			const auto* ghostProfile = catalog.Find_Boss("BOSS_VALTAN_GHOST");
			std::vector<std::string> entryOrder, cinematicStages, mechanics, revival;
			std::array<std::vector<std::string>, 8> loops;
			std::size_t windowIndex = 0u;
			std::uint32_t previousSequence = 0u, synchronizedTicks = 0u, terminalSnapshots = 0u;
			std::array<std::uint32_t, 4> clearPackets{}, rewardPackets{};
			bool waitingForMechanic = false, mechanicStarted = false, ghostRestored = false;
			bool stableIdentity = true, automatic = true, wireParity = true, allAlive = true;
			bool arrivalChecked = false, allArrived = false, killIssued = false, killedByPlayer = false;
			bool progressionComplete = false;
			const auto consumeSnapshots = [&]()
			{
				std::vector<std::uint8_t> baseline;
				std::size_t snapshots = 0u;
				for (std::size_t index = 0u; index < sessions.size(); ++index)
				{
					for (const auto& frame : sessions[index]->m_OutboundFrames)
					{
						if (frame.ePacketType == PACKET_TYPE::S2C_RAID_MVP_RESULT) ++clearPackets[index];
						if (frame.ePacketType == PACKET_TYPE::S2C_INVENTORY_SNAPSHOT) ++rewardPackets[index];
						if (frame.ePacketType != PACKET_TYPE::S2C_WORLD_SNAPSHOT) continue;
						++snapshots;
						if (baseline.empty()) baseline = frame.Bytes;
						else wireParity = wireParity && baseline == frame.Bytes;
						CPacketReader reader{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};
						S2C_WORLD_SNAPSHOT snapshot;
						const bool decoded = Read_Message(reader, snapshot) && !reader.Get_RemainingSize();
						wireParity = wireParity && decoded && snapshot.Players.size() == 4u;
						if (!decoded) continue;
						const auto entity = std::find_if(snapshot.Entities.begin(), snapshot.Entities.end(),
							[primaryId](const auto& value) { return value.iNetEntityId == primaryId; });
						const auto* boss = room->Find_AuditionBoss();
						if (boss)
							wireParity = wireParity && entity != snapshot.Entities.end() &&
								entity->strPatternId == boss->strPatternId && entity->iPatternSequence == boss->iPatternSequence &&
								entity->iPatternStageIndex == boss->iPatternStageIndex && entity->iPhase == boss->iPhase &&
								entity->iCurrentHp == boss->iCurrentHp && entity->iMaximumHp == boss->iMaximumHp;
						else if (killIssued && entity == snapshot.Entities.end()) ++terminalSnapshots;
					}
				}
				wireParity = wireParity && snapshots == 4u;
				if (snapshots == 4u) ++synchronizedTicks;
				drain();
			};
			drain();
			for (std::uint32_t iteration = 0u; iteration < 60000u && room->Is_Ready(); ++iteration)
			{
				room->Tick(1.f / 30.f);
				consumeSnapshots();
				auto* boss = room->Find_AuditionBoss();
				allAlive = allAlive && std::all_of(room->m_Players.begin(), room->m_Players.end(),
					[](const auto& pair) { return pair.second.iCurrentHp != 0u; });
				for (const auto& hit : room->m_TickDamageEvents)
					if (killIssued && hit.iTargetNetEntityId == primaryId && hit.iAmount > 0u) killedByPlayer = true;
				if (!boss)
				{
					progressionComplete = killIssued && room->m_bValtanRaidCleared;
					break;
				}
				stableIdentity = stableIdentity && boss->iNetEntityId == primaryId;
				automatic = automatic && !boss->bScriptedPatternPlayback && !boss->bAutomaticPatternSequenceAuditionOverride &&
					!boss->bAutomaticPatternSequenceAuditionHold && !boss->bMechanicLedgerRequiresReset;
				if (boss->strPatternId == "VALTAN_ENTRANCE_CINEMATIC" &&
					(cinematicStages.empty() || cinematicStages.back() != boss->strPatternStageId))
					cinematicStages.push_back(boss->strPatternStageId);
				if (boss->strPatternId.empty() || previousSequence == boss->iPatternSequence) continue;
				previousSequence = boss->iPatternSequence;
				std::cout << "[VALTAN_4P] tick=" << room->m_iServerTick << " pattern=" << boss->strPatternId
					<< " phase=" << unsigned(boss->iPhase) << " bars=" << CValtanBrain::Calculate_HealthBar(*boss)
					<< " sequence=" << boss->iPatternSequence << '\n';
				if (entryOrder.size() < 3u) entryOrder.push_back(boss->strPatternId);
				if (!arrivalChecked && boss->strPatternId == "VALTAN_WHIRLWIND")
				{
					arrivalChecked = true;
					allArrived = std::all_of(room->m_Players.begin(), room->m_Players.end(), [&arrival](const auto& pair)
					{ return !pair.second.TriggerMove.isActive && std::hypot(pair.second.fPositionX - arrival->fPositionX,
						pair.second.fPositionZ - arrival->fPositionZ) < .05f; });
					// Place the invulnerable observers inside the remaining central floor.
					std::size_t index = 0u;
					for (auto& [id, player] : room->m_Players)
					{
						(void)id; SERVER_NAV_POINT point{};
						const float angle = float(index++) * 1.5707963268f;
						const bool placed = room->m_ServerNavigation.Project_Point(boss->fSpawnPositionX + std::sin(angle) * 2.f,
							boss->fSpawnPositionZ + std::cos(angle) * 2.f, point);
						allArrived = allArrived && placed;
						if (placed) { player.fPositionX = point.x; player.fPositionY = point.y; player.fPositionZ = point.z; }
					}
				}
				if (waitingForMechanic)
				{
					if (boss->strPatternId == windows[windowIndex].mechanic)
					{ mechanics.push_back(boss->strPatternId); mechanicStarted = true; continue; }
					if (!mechanicStarted) continue;
					if (windowIndex == 6u && (boss->strPatternId == "VALTAN_GHOST_DEATH_AUDITION" ||
						boss->strPatternId == "VALTAN_GHOST_RESPAWN_AUDITION"))
					{ revival.push_back(boss->strPatternId); continue; }
					const auto& next = windows[windowIndex + 1u];
					const auto* rotation = catalog.Find_BossPatternRotation("ENCOUNTER_VALTAN", next.phase, next.bars);
					if (!rotation || rotation->PatternIds.empty() || boss->strPatternId != rotation->PatternIds.front()) continue;
					++windowIndex; waitingForMechanic = false; mechanicStarted = false;
					if (windowIndex == 7u)
						ghostRestored = ghostProfile && boss->iMaximumHp == ghostProfile->iMaximumHp &&
							boss->iCurrentHp == boss->iMaximumHp && 40u == CValtanBrain::Calculate_HealthBar(*boss) &&
							boss->iPhase == 3u && boss->bGhostPhasePatternLoopActive;
				}
				const auto& window = windows[windowIndex];
				const auto* rotation = catalog.Find_BossPatternRotation("ENCOUNTER_VALTAN", window.phase, window.bars);
				if (!rotation || std::find(rotation->PatternIds.begin(), rotation->PatternIds.end(), boss->strPatternId) == rotation->PatternIds.end()) continue;
				loops[windowIndex].push_back(boss->strPatternId);
				if (loops[windowIndex].size() != rotation->PatternIds.size() + 1u) continue;
				if (windowIndex < 7u)
				{
					boss->iCurrentHp = CValtanBrain::Resolve_HealthBarHp(*boss, window.gate);
					waitingForMechanic = true;
				}
				else
				{
					// End with a real approved player skill, not the debug kill command.
					boss->iCurrentHp = 1u;
					auto& player = room->m_Players.at(sessions.front()->Get_PlayerId());
					SERVER_NAV_POINT point{};
					if (room->m_ServerNavigation.Project_Point(boss->fPositionX, boss->fPositionZ + 2.f, point))
					{ player.fPositionX = point.x; player.fPositionY = point.y; player.fPositionZ = point.z; }
					C2S_USE_SKILL skill{}; skill.iClientSequence = 1u; skill.iSkillId = 34010u;
					skill.fAimX = boss->fPositionX; skill.fAimZ = boss->fPositionZ;
					room->Handle_UseSkill(player.iSessionId, skill);
					killIssued = player.iCurrentSkillId == skill.iSkillId;
				}
			}
			tests.Require(entryOrder == std::vector<std::string>{"VALTAN_ENTRANCE_CINEMATIC", "VALTAN_ENTRANCE_WHIRLWIND", "VALTAN_WHIRLWIND"} &&
				cinematicStages == std::vector<std::string>{"ESTABLISH", "ARENA_REVEAL", "HERO_HANDOFF"} && allArrived,
				"Four-player entry completes every cinematic stage, arrival and mandatory intro before health loops");
			for (std::size_t index = 0u; index < windows.size(); ++index)
			{
				const auto* rotation = catalog.Find_BossPatternRotation("ENCOUNTER_VALTAN", windows[index].phase, windows[index].bars);
				std::vector<std::string> expected = rotation ? rotation->PatternIds : std::vector<std::string>{};
				if (!expected.empty()) expected.push_back(expected.front());
				tests.Require(!expected.empty() && loops[index] == expected,
					"Continuous four-player room completes the published health-window order and wraps once");
			}
			std::vector<std::string> expectedMechanics;
			for (std::size_t index = 0u; index < 7u; ++index) expectedMechanics.emplace_back(windows[index].mechanic);
			tests.Require(mechanics == expectedMechanics && revival == std::vector<std::string>{"VALTAN_GHOST_DEATH_AUDITION", "VALTAN_GHOST_RESPAWN_AUDITION"} && ghostRestored,
				"All seven health mechanics run once in order and real Death-to-Respawn restores the primary to forty bars");
			tests.Require(room->Is_Ready() && stableIdentity && automatic && allAlive && progressionComplete && killedByPlayer,
				"The uninterrupted four-player raid reaches ghost skill death and authoritative clear without reset or audition override");
			tests.Require(wireParity && synchronizedTicks > 1000u && terminalSnapshots == 4u &&
				std::all_of(clearPackets.begin(), clearPackets.end(), [](auto count) { return count == 1u; }) &&
				std::all_of(rewardPackets.begin(), rewardPackets.end(), [](auto count) { return count >= 1u; }),
				"All four sessions decode identical authoritative snapshots and each receives one raid-clear MVP and rewards");
			std::cout << "[VALTAN_4P] complete=" << progressionComplete << " ticks=" << synchronizedTicks
				<< " window=" << windowIndex << " mechanics=" << mechanics.size() << " parity=" << wireParity
				<< " ready=" << room->Is_Ready() << " status=" << room->m_strStatus << '\n';
		}
		for (const auto& session : sessions) session->Request_Close();
	}

}

