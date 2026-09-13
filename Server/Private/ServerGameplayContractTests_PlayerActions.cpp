#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "BossCombatRuntime.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "PlayerSkillSystem.h"
#include "Network/PacketWriter.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_PlayerActions(TESTS& tests, CGameplayCatalog& catalog, CServerNavigation& navigation, const float& navCellSize, const float& boundaryProbeZ, float& lastWalkableX, float& firstBlockedX, SERVER_WORLD_ENTITY& boss, const PLAYER_SKILL_DEFINITION*& talonStrike, LostArk::Shared::C2S_USE_SKILL& useSkill)
{

	{
		/* Armour is server state. It mitigates every incoming hit while a plate is
		intact and only loses durability inside a GROGGY stage, so the numbers here
		come from the published bootstrap and the same skill path the arena runs. */
		const BOSS_RUNTIME_PROFILE* armorProfile = catalog.Find_Boss("BOSS_VALTAN");
		tests.Require(
			nullptr != armorProfile &&
			2u == armorProfile->ArmorPlates.size() &&
			0u == armorProfile->ArmorPlates[0].iPlateIndex &&
			1u == armorProfile->ArmorPlates[1].iPlateIndex &&
			20u == armorProfile->ArmorPlates[0].iDurability &&
			20u == armorProfile->ArmorPlates[1].iDurability &&
			50u == armorProfile->ArmorPlates[0].iDefense &&
			50u == armorProfile->ArmorPlates[1].iDefense,
			"Load Valtan's two authored armour plates from the gameplay bootstrap");

		std::uint32_t armorSequence = 10u;
		const auto strikeArmoredBoss =
			[&](const bool groggy,
				const std::uint32_t firstDurability,
				const std::uint32_t secondDurability,
				SERVER_WORLD_ENTITY& outBoss) -> std::uint32_t
		{
			SERVER_PLAYER armorPlayer{};
			armorPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			armorPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
			armorPlayer.iCurrentResource = 1000;
			armorPlayer.iMaximumResource = 1000;
			armorPlayer.fPositionX = 151.f;
			armorPlayer.fPositionY = 22.97f;
			armorPlayer.fPositionZ = -129.f;
			SERVER_WORLD_ENTITY armored = boss;
			armored.bPatternGroggy = groggy;
			armored.ArmorPlates.clear();
			for (const BOSS_ARMOR_PLATE& plate : armorProfile->ArmorPlates)
			{
				SERVER_BOSS_ARMOR_PLATE_STATE state{};
				state.iPlateIndex = plate.iPlateIndex;
				state.iDefense = plate.iDefense;
				state.iRemainingDurability = 0u == plate.iPlateIndex ?
					firstDurability : secondDurability;
				armored.ArmorPlates.push_back(state);
			}
			std::vector<SERVER_WORLD_ENTITY> armorEntities{ armored };
			C2S_USE_SKILL armorSkill = useSkill;
			armorSkill.iClientSequence = ++armorSequence;
			CPlayerSkillSystem armorSkills;
			armorSkills.Try_Start(armorPlayer, armorSkill, catalog, 10);
			std::vector<DAMAGE_EVENT> armorEvents;
			for (std::uint32_t tick = 11; tick < 70; ++tick)
			{
				armorSkills.Update(armorPlayer, armorEntities, catalog,
					&navigation, nullptr, 1.f / 30.f, tick, armorEvents);
			}
			outBoss = armorEntities[0];
			return armored.iCurrentHp - armorEntities[0].iCurrentHp;
		};

		SERVER_WORLD_ENTITY strippedBoss{};
		const std::uint32_t strippedDamage =
			strikeArmoredBoss(false, 0u, 0u, strippedBoss);
		SERVER_WORLD_ENTITY halfArmoredBoss{};
		const std::uint32_t halfArmoredDamage =
			strikeArmoredBoss(false, 0u, 4000u, halfArmoredBoss);
		SERVER_WORLD_ENTITY fullyArmoredBoss{};
		const std::uint32_t fullyArmoredDamage =
			strikeArmoredBoss(false, 4000u, 4000u, fullyArmoredBoss);
		tests.Require(
			3610u == strippedDamage &&
			fullyArmoredDamage < halfArmoredDamage &&
			halfArmoredDamage < strippedDamage,
			"Mitigate a boss hit by each intact plate and by nothing once all break");
		{
			/* An invulnerable pattern absorbs the hit whole: no HP moves and no
			damage event reaches the snapshot, so the raid can see the mechanic
			has to be answered rather than outraced. */
			SERVER_PLAYER immunePlayer{};
			immunePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			immunePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
			immunePlayer.iCurrentResource = 1000;
			immunePlayer.iMaximumResource = 1000;
			immunePlayer.fPositionX = 151.f;
			immunePlayer.fPositionY = 22.97f;
			immunePlayer.fPositionZ = -129.f;
			SERVER_WORLD_ENTITY immuneBoss = boss;
			immuneBoss.bPatternInvulnerable = true;
			std::vector<SERVER_WORLD_ENTITY> immuneEntities{ immuneBoss };
			C2S_USE_SKILL immuneSkill = useSkill;
			immuneSkill.iClientSequence = 41;
			CPlayerSkillSystem immuneSkills;
			immuneSkills.Try_Start(immunePlayer, immuneSkill, catalog, 10);
			std::vector<DAMAGE_EVENT> immuneEvents;
			for (std::uint32_t tick = 11; tick < 70; ++tick)
			{
				immuneSkills.Update(immunePlayer, immuneEntities, catalog,
					&navigation, nullptr, 1.f / 30.f, tick, immuneEvents);
			}
			tests.Require(
				immuneBoss.iCurrentHp == immuneEntities[0].iCurrentHp &&
				immuneEvents.empty(),
				"Absorb every player hit while an invulnerable pattern runs");
		}
		tests.Require(
			4000u == fullyArmoredBoss.ArmorPlates[0].iRemainingDurability &&
			4000u == fullyArmoredBoss.ArmorPlates[1].iRemainingDurability,
			"Leave armour durability untouched by damage outside a groggy stage");

		SERVER_WORLD_ENTITY groggyBoss{};
		const std::uint32_t groggyDamage =
			strikeArmoredBoss(true, 4000u, 4000u, groggyBoss);
		tests.Require(
			groggyDamage == fullyArmoredDamage &&
			4000u - groggyDamage ==
				groggyBoss.ArmorPlates[0].iRemainingDurability &&
			4000u == groggyBoss.ArmorPlates[1].iRemainingDurability,
			"Spend groggy damage on the front plate only, at the mitigated amount");

		SERVER_WORLD_ENTITY brokenBoss{};
		(void)strikeArmoredBoss(true, 1u, 4000u, brokenBoss);
		tests.Require(
			0u == brokenBoss.ArmorPlates[0].iRemainingDurability &&
			4000u == brokenBoss.ArmorPlates[1].iRemainingDurability &&
			brokenBoss.bPendingArmorBreakReaction &&
			!brokenBoss.bPatternGroggy,
			"Break one plate per groggy window, close it, and queue the part-break reaction");
	}
	{
		/* Q/W/E/R counter power is attached to a real damaging hit, unlike the
		damage-less COUNTER guard bridge. A live boss counter window publishes one
		typed edge and closes only after this Q hit overlaps its front proxy. */
		SERVER_WORLD_ENTITY counterBoss = boss;
		counterBoss.iNetEntityId = 88101u;
		counterBoss.fPositionX = 1.f;
		counterBoss.fPositionY = 0.f;
		counterBoss.fPositionZ = 0.f;
		counterBoss.fYawDegrees = -90.f;
		counterBoss.strPatternId = "VALTAN_TRIPLE_COUNTER";
		counterBoss.strPatternStageId = "COUNTER_1";
		counterBoss.strActionId = "valtan.reactive.triple-counter.first";
		counterBoss.iPatternSequence = 902u;
		counterBoss.bPatternHasCounterProxy = true;
		counterBoss.ePatternCounterProxyKind =
			BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC;
		counterBoss.fPatternCounterProxyArcDegrees = 180.f;
		counterBoss.ArmorPlates.clear();
		std::string counterBossStatus;
		const std::vector<BOSS_PART_DEFINITION> noCounterParts;
		const bool initializedCounterBoss = CBossCombatRuntime::Initialize(
			counterBoss.BossCombat, noCounterParts, counterBossStatus);
		(void)CBossCombatRuntime::Set_Flag(
			counterBoss.BossCombat,
			SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, true);
		const std::uint32_t hpBeforeBossCounter = counterBoss.iCurrentHp;
		std::vector<SERVER_WORLD_ENTITY> counterBossEntities{ counterBoss };

		SERVER_PLAYER bossCounterPlayer{};
		bossCounterPlayer.iPlayerId = 88102u;
		bossCounterPlayer.iNetEntityId = 88103u;
		bossCounterPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		bossCounterPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		bossCounterPlayer.iCurrentHp = 1000u;
		bossCounterPlayer.iMaximumHp = 1000u;
		bossCounterPlayer.iCurrentResource = 1000u;
		bossCounterPlayer.iMaximumResource = 1000u;
		bossCounterPlayer.fPositionX = 0.f;
		bossCounterPlayer.fPositionY = 0.f;
		bossCounterPlayer.fPositionZ = 0.f;
		C2S_USE_SKILL bossCounterSkill{};
		bossCounterSkill.iClientSequence = 76u;
		bossCounterSkill.iSkillId = 34040u;
		bossCounterSkill.fAimX = counterBoss.fPositionX;
		bossCounterSkill.fAimZ = counterBoss.fPositionZ;
		CPlayerSkillSystem bossCounterSkills;
		const bool startedBossCounter = bossCounterSkills.Try_Start(
			bossCounterPlayer, bossCounterSkill, catalog, 10u);
		std::vector<DAMAGE_EVENT> bossCounterEvents;
		for (std::uint32_t tick = 11u; tick < 35u; ++tick)
		{
			bossCounterSkills.Update(
				bossCounterPlayer, counterBossEntities, catalog, nullptr,
				nullptr, 1.f / 30.f, tick, bossCounterEvents);
		}
		SERVER_WORLD_ENTITY& resolvedCounterBoss =
			counterBossEntities.front();
		std::uint32_t bossCounterEventDamage = 0u;
		for (const DAMAGE_EVENT& event : bossCounterEvents)
			bossCounterEventDamage += event.iAmount;
		const bool publishedCounterHit =
			CBossCombatRuntime::Consume_PatternOutcome(
				resolvedCounterBoss, "valtan.reactive.triple-counter.first",
				BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT);
		tests.Require(
			initializedCounterBoss && startedBossCounter &&
			publishedCounterHit &&
			!CBossCombatRuntime::Has_Flag(
				resolvedCounterBoss.BossCombat,
				SERVER_BOSS_COMBAT_FLAG::COUNTERABLE) &&
			!bossCounterEvents.empty() &&
			hpBeforeBossCounter - resolvedCounterBoss.iCurrentHp ==
				bossCounterEventDamage,
			"Publish and close a front-proxy boss counter window from a real Q skill hit");
	}
	{
		/* Make one otherwise-eligible row the only row not on cooldown, then
		mark it at its repeat limit. The same weighted evaluator must relax only
		the soft repeat reason and expose the original block plus the relaxation. */
		const auto* repeatPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const BOSS_PATTERN_DEFINITION* repeatPattern = nullptr;
		if (nullptr != repeatPatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& candidate : *repeatPatterns)
			{
				if (BOSS_PATTERN_SELECTION::NORMAL != candidate.eSelection ||
					"VALTAN_ENTRANCE_WHIRLWIND" == candidate.strPatternId ||
					candidate.iMinimumPhase > 1u || candidate.iMaximumPhase < 1u ||
					candidate.iMinimumHealthBar > 160u ||
					candidate.iMaximumHealthBar < 160u ||
					BOSS_PATTERN_ARMOR_REQUIREMENT::ARMORED ==
						candidate.eArmorRequirement ||
					BOSS_PATTERN_PHASE_REQUIREMENT::PHASE_TWO ==
						candidate.ePhaseRequirement ||
					0u == candidate.iMaximumConsecutiveUses)
				{
					continue;
				}
				const std::size_t sourceFamilyCount =
					static_cast<std::size_t>(std::count_if(
						repeatPatterns->begin(), repeatPatterns->end(),
						[&candidate](const BOSS_PATTERN_DEFINITION& other)
						{
							return BOSS_PATTERN_SELECTION::NORMAL ==
								other.eSelection &&
								other.iSourcePrimaryActionId ==
									candidate.iSourcePrimaryActionId;
						}));
				if (1u == sourceFamilyCount)
				{
					repeatPattern = &candidate;
					break;
				}
			}
		}
		std::map<PLAYER_ID, SERVER_PLAYER> repeatPlayers;
		SERVER_PLAYER repeatTarget{};
		repeatTarget.iPlayerId = 7001u;
		repeatTarget.iNetEntityId = 7002u;
		repeatTarget.iCurrentHp = 1000000u;
		repeatTarget.iMaximumHp = 1000000u;
		repeatTarget.isCombatReady = true;
		if (nullptr != repeatPattern)
			repeatTarget.fPositionX = (repeatPattern->fMinimumRange +
				repeatPattern->fMaximumRange) * 0.5f;
		repeatPlayers.emplace(repeatTarget.iPlayerId, repeatTarget);
		SERVER_WORLD_ENTITY repeatBoss{};
		repeatBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		repeatBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		repeatBoss.strArchetypeId = "BOSS_VALTAN";
		repeatBoss.strEncounterId = "ENCOUNTER_VALTAN";
		repeatBoss.iCurrentHp = 60000u;
		repeatBoss.iMaximumHp = 60000u;
		repeatBoss.iMaximumHealthBars = 160u;
		repeatBoss.iLastEvaluatedHealthBar = 160u;
		repeatBoss.iPhase = 1u;
		repeatBoss.fEngageDistance = 1000.f;
		repeatBoss.bIntroPatternConsumed = true;
		repeatBoss.bScriptedPatternPlayback = true;
		if (nullptr != repeatPattern && nullptr != repeatPatterns)
		{
			repeatBoss.strLastPatternId = repeatPattern->strPatternId;
			repeatBoss.iConsecutivePatternUses =
				repeatPattern->iMaximumConsecutiveUses;
			for (const BOSS_PATTERN_DEFINITION& candidate : *repeatPatterns)
			{
				if (BOSS_PATTERN_SELECTION::NORMAL != candidate.eSelection ||
					candidate.strPatternId == repeatPattern->strPatternId)
				{
					continue;
				}
				SERVER_BOSS_PATTERN_COOLDOWN cooldown{};
				cooldown.iSourcePrimaryActionId =
					candidate.iSourcePrimaryActionId;
				cooldown.iReadyTick = 22000u;
				repeatBoss.PatternCooldowns.push_back(std::move(cooldown));
			}
		}
		std::vector<DAMAGE_EVENT> repeatDamageEvents;
		auto repeatBrainStorage = std::make_unique<CValtanBrain>();
		CValtanBrain& repeatBrain = *repeatBrainStorage;
		repeatBrain.Update(
			repeatBoss, repeatPlayers, catalog, navigation,
			1.f / 30.f, 21000u, {}, repeatDamageEvents);
		const VALTAN_DECISION_TRACE* repeatTrace =
			repeatBrain.Get_LatestDecisionTrace();
		const auto repeatCandidate = nullptr == repeatTrace ||
			nullptr == repeatPattern ?
				VALTAN_DECISION_CANDIDATE_TRACE{} :
				[repeatTrace, repeatPattern]()
				{
					const auto found = std::find_if(
						repeatTrace->Candidates.begin(),
						repeatTrace->Candidates.end(),
						[repeatPattern](
							const VALTAN_DECISION_CANDIDATE_TRACE& candidate)
						{
							return candidate.strPatternId ==
								repeatPattern->strPatternId;
						});
					return repeatTrace->Candidates.end() == found ?
						VALTAN_DECISION_CANDIDATE_TRACE{} : *found;
				}();
		bool weightIntervalsPartition = nullptr != repeatTrace;
		std::uint64_t nextWeightBegin = 0u;
		std::size_t effectiveCandidateCount = 0u;
		if (nullptr != repeatTrace)
		{
			for (const VALTAN_DECISION_CANDIDATE_TRACE& candidate :
				repeatTrace->Candidates)
			{
				if (0u == candidate.iEffectiveWeight) continue;
				++effectiveCandidateCount;
				weightIntervalsPartition = weightIntervalsPartition &&
					candidate.iWeightBeginInclusive == nextWeightBegin &&
					candidate.iWeightEndExclusive >
						candidate.iWeightBeginInclusive &&
					candidate.iWeightEndExclusive -
						candidate.iWeightBeginInclusive ==
						candidate.iEffectiveWeight &&
					candidate.iWeightEndExclusive <= repeatTrace->iTotalWeight &&
					(!candidate.bSelected ||
						(repeatTrace->iRandomTicket >=
							candidate.iWeightBeginInclusive &&
						 repeatTrace->iRandomTicket <
							candidate.iWeightEndExclusive));
				nextWeightBegin = candidate.iWeightEndExclusive;
			}
			weightIntervalsPartition = weightIntervalsPartition &&
				0u != effectiveCandidateCount &&
				nextWeightBegin == repeatTrace->iTotalWeight;
		}
		tests.Require(
			nullptr != repeatPattern && nullptr != repeatTrace &&
			weightIntervalsPartition &&
			repeatTrace->bMaximumConsecutiveRelaxed &&
			VALTAN_DECISION_RESULT::SELECTED == repeatTrace->eResult &&
			repeatPattern->strPatternId == repeatTrace->strSelectedPatternId &&
			repeatCandidate.bSoftRepeatBlocked &&
			repeatCandidate.bSoftRepeatRelaxed && repeatCandidate.bSelected &&
			0u != (repeatCandidate.iExclusionMask &
				VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED) &&
			0u != (repeatCandidate.iExclusionMask &
				VALTAN_EXCLUDE_SOFT_REPEAT_RELAXED),
			"Relax soft repeat avoidance and partition every effective decision weight interval before selection");

		auto traceRoomStorage =
			std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& traceRoom = *traceRoomStorage;
		SERVER_WORLD_ENTITY traceBoss = repeatBoss;
		traceBoss.iNetEntityId = 79991u;
		traceBoss.strPlacementId = "boss.valtan.decision-trace-contract";
		traceBoss.PinnedDefinitionRevision =
			traceRoom.m_GameplayCatalog.Get_ActiveRevision();
		traceRoom.m_WorldEntities.push_back(traceBoss);
		traceRoom.m_ValtanBrain = repeatBrain;
		if (nullptr != repeatTrace)
		{
			traceRoom.m_ValtanDecisionTraceRevision.iBossEntityId =
				traceBoss.iNetEntityId;
			traceRoom.m_ValtanDecisionTraceRevision.strBossPlacementId =
				traceBoss.strPlacementId;
			traceRoom.m_ValtanDecisionTraceRevision.iTraceSequence =
				repeatTrace->iTraceSequence;
			traceRoom.m_ValtanDecisionTraceRevision.DefinitionRevision =
				traceBoss.PinnedDefinitionRevision;
		}
		C2S_VALTAN_DECISION_TRACE_QUERY traceQuery{};
		traceQuery.iRequestSequence = 91u;
		traceQuery.strBossPlacementId = traceBoss.strPlacementId;
		S2C_VALTAN_DECISION_TRACE_RESPONSE traceResponse{};
		std::string traceStatus;
		CPacketWriter traceWriter;
		const bool mappedTrace = nullptr != repeatTrace &&
			traceRoom.Build_ValtanDecisionTraceResponse(
				traceQuery, traceResponse, traceStatus) &&
			VALTAN_DECISION_TRACE_QUERY_RESULT::TRACE == traceResponse.eResult &&
			traceBoss.PinnedDefinitionRevision ==
				traceResponse.DefinitionRevision &&
			repeatTrace->iTraceSequence == traceResponse.Trace.iTraceSequence &&
			repeatTrace->Candidates.size() == traceResponse.Trace.Candidates.size() &&
			Write_Message(traceWriter, traceResponse);
		C2S_VALTAN_DECISION_TRACE_QUERY unchangedQuery = traceQuery;
		unchangedQuery.iRequestSequence = 92u;
		unchangedQuery.iAfterTraceSequence = nullptr == repeatTrace ? 0u :
			repeatTrace->iTraceSequence;
		S2C_VALTAN_DECISION_TRACE_RESPONSE unchangedResponse{};
		const bool mappedUnchanged =
			traceRoom.Build_ValtanDecisionTraceResponse(
				unchangedQuery, unchangedResponse, traceStatus) &&
			VALTAN_DECISION_TRACE_QUERY_RESULT::UNCHANGED ==
				unchangedResponse.eResult &&
			!unchangedResponse.DefinitionRevision.Is_Valid() &&
			0u == unchangedResponse.Trace.iTraceSequence;
		GameplayDataRevision collectedSelectorRevision =
			traceBoss.PinnedDefinitionRevision;
		collectedSelectorRevision.Bytes[7] ^= 0x80u;
		if (!collectedSelectorRevision.Is_Valid() ||
			collectedSelectorRevision == traceBoss.PinnedDefinitionRevision)
		{
			collectedSelectorRevision.Bytes[8] ^= 1u;
		}
		traceRoom.m_ValtanDecisionTraceRevision.DefinitionRevision =
			collectedSelectorRevision;
		S2C_VALTAN_DECISION_TRACE_RESPONSE collectedTraceResponse{};
		const bool mappedCollectedTrace =
			nullptr == traceRoom.Resolve_GameplayGeneration(
				collectedSelectorRevision) &&
			traceRoom.Build_ValtanDecisionTraceResponse(
				traceQuery, collectedTraceResponse, traceStatus) &&
			VALTAN_DECISION_TRACE_QUERY_RESULT::TRACE ==
				collectedTraceResponse.eResult &&
			collectedSelectorRevision ==
				collectedTraceResponse.DefinitionRevision;
		traceRoom.m_ValtanDecisionTraceRevision.DefinitionRevision =
			traceBoss.PinnedDefinitionRevision;
		bool rejectedUnknownExclusion = true;
		VALTAN_DECISION_TRACE* mutableTrace = const_cast<VALTAN_DECISION_TRACE*>(
			traceRoom.m_ValtanBrain.Get_LatestDecisionTrace());
		if (nullptr != mutableTrace && !mutableTrace->Candidates.empty())
		{
			const std::uint32_t originalMask =
				mutableTrace->Candidates.front().iExclusionMask;
			mutableTrace->Candidates.front().iExclusionMask |= 0x80000000u;
			S2C_VALTAN_DECISION_TRACE_RESPONSE invalidResponse{};
			rejectedUnknownExclusion =
				!traceRoom.Build_ValtanDecisionTraceResponse(
					traceQuery, invalidResponse, traceStatus);
			mutableTrace->Candidates.front().iExclusionMask = originalMask;
		}
		tests.Require(
			mappedTrace && mappedUnchanged && mappedCollectedTrace &&
			rejectedUnknownExclusion,
			"Map the authoritative Valtan decision trace after selector-generation GC and fail-close unknown exclusion bits");
	}

	{
		/* Both charges use the same typed impact marker. Dash keeps the exact
		user-saved RECOVERY occurrence identity, but that animation span is now a
		Server-owned GROGGY part-damage window. */
		const auto* chargePatterns = catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto findChargeStage =
			[&](const char* patternId)
				-> const BOSS_PATTERN_STAGE_DEFINITION*
		{
			if (nullptr == chargePatterns)
				return nullptr;
			for (const BOSS_PATTERN_DEFINITION& pattern : *chargePatterns)
			{
				if (pattern.strPatternId != patternId)
					continue;
				for (std::size_t index = 0; index < pattern.Stages.size(); ++index)
				{
					if (!pattern.Stages[index].bChargeImpact)
						continue;
					return &pattern.Stages[index];
				}
			}
			return nullptr;
		};
		const BOSS_PATTERN_STAGE_DEFINITION* openingCharge =
			findChargeStage("VALTAN_ARMOR_BREAK_OPENING");
		const BOSS_PATTERN_STAGE_DEFINITION* dashCharge =
			findChargeStage("VALTAN_DASH_CHARGE");
		tests.Require(
			nullptr != openingCharge && nullptr != dashCharge &&
			openingCharge->strStageId == "WALL_CHARGE" &&
			dashCharge->strStageId == "CHARGE",
			"Load each wall-contact charge graph into its immediate typed target");

		std::uint32_t chargeStageCount = 0u;
		std::uint32_t groggyStageCount = 0u;
		if (nullptr != chargePatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *chargePatterns)
			{
				if (pattern.strPatternId != "VALTAN_ARMOR_BREAK_OPENING" &&
					pattern.strPatternId != "VALTAN_DASH_CHARGE")
					continue;
				for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
				{
					if (stage.bChargeImpact)
						++chargeStageCount;
					if (BOSS_PATTERN_STAGE_KIND::GROGGY == stage.eStageKind)
						++groggyStageCount;
				}
			}
		}
		tests.Require(
			2u == chargeStageCount && 2u == groggyStageCount,
			"Open a typed GROGGY window after each Valtan charge graph");

		/* Dash remains a repeatable weighted attack. Its saved start/loop/end
		composition owns the damage window and the event-entered GROGGY stage. */
		const BOSS_PATTERN_DEFINITION* dashPattern = nullptr;
		const BOSS_PATTERN_STAGE_DEFINITION* dashGroggy = nullptr;
		const BOSS_PATTERN_DEFINITION* partBreakPattern = nullptr;
		if (nullptr != chargePatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *chargePatterns)
			{
				if (pattern.strPatternId == "VALTAN_DASH_CHARGE")
				{
					dashPattern = &pattern;
					if (pattern.Stages.size() > 2u)
						dashGroggy = &pattern.Stages[2];
				}
				else if (pattern.strPatternId == "VALTAN_PART_BREAK")
					partBreakPattern = &pattern;
			}
		}
		const auto hasDashBranch = [](
			const BOSS_PATTERN_STAGE_DEFINITION& stage,
			const BOSS_PATTERN_STAGE_OUTCOME outcome,
			const std::string_view nextActionId)
		{
			return std::any_of(
				stage.Branches.begin(), stage.Branches.end(),
				[outcome, nextActionId](const BOSS_PATTERN_STAGE_BRANCH& branch)
				{
					return branch.eOutcome == outcome &&
						branch.strNextActionId == nextActionId;
				});
		};
		const auto hasPatternFollowup = [](
			const BOSS_PATTERN_STAGE_DEFINITION& stage,
			const BOSS_PATTERN_STAGE_OUTCOME outcome,
			const std::string_view nextPatternId)
		{
			return std::any_of(
				stage.Branches.begin(), stage.Branches.end(),
				[outcome, nextPatternId](const BOSS_PATTERN_STAGE_BRANCH& branch)
				{
					return branch.eOutcome == outcome &&
						branch.strNextActionId.empty() &&
						branch.strNextPatternId == nextPatternId;
				});
		};
		const auto hasDashGroggyFlag = [](
			const BOSS_PATTERN_STAGE_DEFINITION& stage,
			const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			const std::uint32_t value)
		{
			return std::any_of(
				stage.Actions.begin(), stage.Actions.end(),
				[trigger, value](const BOSS_PATTERN_STAGE_ACTION& action)
				{
					return action.eTrigger == trigger &&
						action.eKind ==
							BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG &&
						action.strTargetId == "boss.flag.groggy" &&
						action.iValue == value;
				});
		};
		tests.Require(
			nullptr != dashPattern &&
			BOSS_PATTERN_SELECTION::NORMAL == dashPattern->eSelection &&
			0u != dashPattern->iSelectionWeight &&
			dashPattern->fMaximumRange > 0.f,
			"Keep Dash Charge a repeatable weighted pattern with travel distance");
		tests.Require(
			nullptr != dashPattern && 3u == dashPattern->Stages.size() &&
			nullptr != dashCharge && &dashPattern->Stages[1] == dashCharge &&
			nullptr != dashGroggy && &dashPattern->Stages[2] == dashGroggy &&
			nullptr != partBreakPattern && 2u == partBreakPattern->Stages.size() &&
			BOSS_PATTERN_SELECTION::AUDITION_ONLY == partBreakPattern->eSelection &&
			BOSS_PATTERN_TARGET_POLICY::NONE == partBreakPattern->eTargetPolicy &&
			BOSS_PATTERN_AIM_POLICY::NONE == partBreakPattern->eAimPolicy &&
			420604u == dashPattern->iSourcePrimaryActionId &&
			BOSS_PATTERN_TARGET_POLICY::LOCK_NEAREST_ON_START ==
				dashPattern->eTargetPolicy &&
			BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START ==
				dashPattern->eAimPolicy &&
			BOSS_PATTERN_STAGE_KIND::WINDUP ==
				dashPattern->Stages[0].eStageKind &&
			3650u == dashPattern->Stages[0].iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::NONE ==
				dashPattern->Stages[0].eHitShape &&
			0u == dashPattern->Stages[0].iHitCount &&
			BOSS_PATTERN_STAGE_MOTION_KIND::NONE ==
				dashPattern->Stages[0].Motion.eKind &&
			1500u == dashCharge->iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::BOX == dashCharge->eHitShape &&
			std::abs(dashCharge->fHitLength - 10.f) < 1.0e-6f &&
			std::abs(dashCharge->fHitHalfWidth - 2.5f) < 1.0e-6f &&
			1u == dashCharge->iHitCount && 0u == dashCharge->iHitDelayMs &&
			dashCharge->HitOffsetsMs.empty() &&
			"damage.valtan.dash-charge" == dashCharge->strDamageProfileId &&
			std::abs(dashCharge->fPushRangeM - 2.f) < 1.0e-6f &&
			150u == dashCharge->iPushMs && dashCharge->bKnockdown &&
			1000u == dashCharge->iDownMs && dashCharge->bChargeImpact &&
			BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD ==
				dashCharge->Motion.eKind &&
			std::abs(dashCharge->Motion.fDistance - 20.f) < 1.0e-6f &&
			BOSS_PATTERN_STAGE_KIND::GROGGY ==
				dashGroggy->eStageKind &&
			6833u == dashGroggy->iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::NONE ==
				dashGroggy->eHitShape &&
			0u == dashGroggy->iHitCount &&
			BOSS_PATTERN_STAGE_MOTION_KIND::NONE ==
				dashGroggy->Motion.eKind &&
			BOSS_PATTERN_PART_DAMAGE_POLICY::DESTROY_FIRST_ELIGIBLE ==
				dashGroggy->ePartDamagePolicy &&
			2u == dashGroggy->Actions.size() &&
			hasDashGroggyFlag(*dashGroggy,
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1u) &&
			hasDashGroggyFlag(*dashGroggy,
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT, 0u) &&
			BOSS_PATTERN_STAGE_KIND::PART_BREAK ==
				partBreakPattern->Stages[0].eStageKind &&
			1800u == partBreakPattern->Stages[0].iDurationMs &&
			BOSS_PATTERN_STAGE_KIND::RECOVERY ==
				partBreakPattern->Stages[1].eStageKind &&
			5183u == partBreakPattern->Stages[1].iDurationMs,
			"Keep Dash Charge and saved GROGGY in one graph, then play the complete 6983ms Part Break reaction and recovery chain");
		tests.Require(
			nullptr != dashPattern &&
			2u == dashPattern->Stages[1].Branches.size() &&
			hasDashBranch(dashPattern->Stages[1],
				BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT,
				"valtan.attack.dash-charge.recovery") &&
			hasDashBranch(dashPattern->Stages[1],
				BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.attack.dash-charge.recovery") &&
			nullptr != dashGroggy && 2u == dashGroggy->Branches.size() &&
			hasPatternFollowup(*dashGroggy,
				BOSS_PATTERN_STAGE_OUTCOME::PART_DESTROYED,
				"VALTAN_PART_BREAK") &&
			hasDashBranch(*dashGroggy,
				BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT, "") &&
			/* The publisher writes the ordered fallthrough of a stage without
			   authored branches as one explicit TIMEOUT row: PART_BREAK -> its
			   recovery, and the recovery -> terminal. */
			1u == partBreakPattern->Stages[0].Branches.size() &&
			hasDashBranch(partBreakPattern->Stages[0],
				BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reaction.part-break.recovery") &&
			1u == partBreakPattern->Stages[1].Branches.size() &&
			hasDashBranch(partBreakPattern->Stages[1],
				BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT, ""),
			"Enter same-pattern Dash GROGGY and interrupt it only on a typed part-destroyed edge");
	}

	{
		/* A plate coming off is its own beat: the boss plays the source's
		part-destruction reaction instead of the ordinary recovery. The clock must
		never reach that stage, or a charge that hit nothing would play it too. */
		const auto* reactionPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		std::uint32_t partBreakStageCount = 0u;
		bool everyPartBreakHasExpectedTail = true;
		bool everyChargePatternHasExpectedReaction = true;
		if (nullptr != reactionPatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *reactionPatterns)
			{
				std::uint32_t patternPartBreaks = 0u;
				bool hasCharge = false;
				for (std::size_t index = 0; index < pattern.Stages.size(); ++index)
				{
					if (pattern.Stages[index].bChargeImpact)
						hasCharge = true;
					if (BOSS_PATTERN_STAGE_KIND::PART_BREAK !=
						pattern.Stages[index].eStageKind)
					{
						continue;
					}
					++patternPartBreaks;
					++partBreakStageCount;
					const bool exactSplitReaction =
						"VALTAN_PART_BREAK" == pattern.strPatternId &&
						index + 2u == pattern.Stages.size() &&
						BOSS_PATTERN_STAGE_KIND::RECOVERY ==
							pattern.Stages[index + 1u].eStageKind &&
						"PART_BREAK_RECOVERY" ==
							pattern.Stages[index + 1u].strStageId &&
						5183u == pattern.Stages[index + 1u].iDurationMs;
					if (!exactSplitReaction && index + 1u != pattern.Stages.size())
						everyPartBreakHasExpectedTail = false;
				}
				if (hasCharge)
				{
					const std::uint32_t expected =
						"VALTAN_ARMOR_BREAK_OPENING" == pattern.strPatternId ?
							1u : 0u;
					if (expected != patternPartBreaks)
						everyChargePatternHasExpectedReaction = false;
				}
			}
		}
		tests.Require(
			2u == partBreakStageCount && everyPartBreakHasExpectedTail &&
				everyChargePatternHasExpectedReaction,
			"Keep the opening reaction local and give the Dash reaction one exact recovery tail");
		tests.Require(
			Is_EventEnteredStage(BOSS_PATTERN_STAGE_KIND::GROGGY) &&
			Is_EventEnteredStage(BOSS_PATTERN_STAGE_KIND::PART_BREAK) &&
			!Is_EventEnteredStage(BOSS_PATTERN_STAGE_KIND::WINDUP) &&
			!Is_EventEnteredStage(BOSS_PATTERN_STAGE_KIND::ACTIVE) &&
			!Is_EventEnteredStage(BOSS_PATTERN_STAGE_KIND::RECOVERY),
			"Keep only stun and part-break reaction stages event-entered; recovery advances on the pattern clock");
	}

	{
		/* A weighted pattern can require an armour state, and only a weighted one:
		a scripted health-bar mechanic gated on armour would silently vanish from a
		fight that depends on it. */
		const auto* gatePatterns = catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const BOSS_PATTERN_DEFINITION* dashPattern = nullptr;
		bool everyScriptedPatternIsUngated = true;
		std::uint32_t gatedCount = 0u;
		if (nullptr != gatePatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *gatePatterns)
			{
				if (BOSS_PATTERN_ARMOR_REQUIREMENT::ANY !=
					pattern.eArmorRequirement)
				{
					++gatedCount;
					if (BOSS_PATTERN_SELECTION::NORMAL != pattern.eSelection)
						everyScriptedPatternIsUngated = false;
				}
				if (pattern.strPatternId == "VALTAN_DASH_CHARGE")
					dashPattern = &pattern;
			}
		}
		tests.Require(
			nullptr != dashPattern &&
			BOSS_PATTERN_ARMOR_REQUIREMENT::ARMORED ==
				dashPattern->eArmorRequirement &&
			1u == gatedCount && everyScriptedPatternIsUngated,
			"Offer the armour-opening dash only while a plate is still on");

		/* The gate reads live plate state, so the same boss stops meeting an
		ARMORED requirement the moment its last plate breaks, and a boss that
		wears no plates at all reads as stripped rather than as armoured. */
		SERVER_WORLD_ENTITY gateBoss{};
		gateBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		gateBoss.strArchetypeId = "BOSS_VALTAN";
		gateBoss.strEncounterId = "ENCOUNTER_VALTAN";
		const BOSS_RUNTIME_PROFILE* gateProfile =
			catalog.Find_Boss("BOSS_VALTAN");
		if (nullptr != gateProfile)
		{
			for (const BOSS_ARMOR_PLATE& plate : gateProfile->ArmorPlates)
			{
				SERVER_BOSS_ARMOR_PLATE_STATE state{};
				state.iPlateIndex = plate.iPlateIndex;
				state.iDefense = plate.iDefense;
				state.iRemainingDurability = plate.iDurability;
				gateBoss.ArmorPlates.push_back(state);
			}
		}
		SERVER_WORLD_ENTITY halfGateBoss = gateBoss;
		halfGateBoss.ArmorPlates[0].iRemainingDurability = 0u;
		SERVER_WORLD_ENTITY strippedGateBoss = gateBoss;
		for (SERVER_BOSS_ARMOR_PLATE_STATE& plate : strippedGateBoss.ArmorPlates)
			plate.iRemainingDurability = 0u;
		SERVER_WORLD_ENTITY plainGateBoss = gateBoss;
		plainGateBoss.ArmorPlates.clear();
		using ARMOR_REQUIREMENT = BOSS_PATTERN_ARMOR_REQUIREMENT;
		tests.Require(
			2u == gateBoss.ArmorPlates.size() &&
			CValtanBrain::Is_ArmorRequirementMet(
				gateBoss, ARMOR_REQUIREMENT::ARMORED) &&
			CValtanBrain::Is_ArmorRequirementMet(
				halfGateBoss, ARMOR_REQUIREMENT::ARMORED) &&
			!CValtanBrain::Is_ArmorRequirementMet(
				strippedGateBoss, ARMOR_REQUIREMENT::ARMORED) &&
			!CValtanBrain::Is_ArmorRequirementMet(
				plainGateBoss, ARMOR_REQUIREMENT::ARMORED),
			"Meet an ARMORED requirement only while some plate is still on");
		tests.Require(
			!CValtanBrain::Is_ArmorRequirementMet(
				gateBoss, ARMOR_REQUIREMENT::STRIPPED) &&
			CValtanBrain::Is_ArmorRequirementMet(
				strippedGateBoss, ARMOR_REQUIREMENT::STRIPPED) &&
			CValtanBrain::Is_ArmorRequirementMet(
				plainGateBoss, ARMOR_REQUIREMENT::STRIPPED) &&
			CValtanBrain::Is_ArmorRequirementMet(
				gateBoss, ARMOR_REQUIREMENT::ANY) &&
			CValtanBrain::Is_ArmorRequirementMet(
				strippedGateBoss, ARMOR_REQUIREMENT::ANY),
			"Read a boss with no plate left, and one with none authored, as stripped");
		SERVER_WORLD_ENTITY phaseOneBoss{};
		phaseOneBoss.iPhase = 1u;
		SERVER_WORLD_ENTITY phaseTwoBoss{};
		phaseTwoBoss.iPhase = 2u;
		using PHASE_REQUIREMENT = BOSS_PATTERN_PHASE_REQUIREMENT;
		tests.Require(
			CValtanBrain::Is_PhaseRequirementMet(
				phaseOneBoss, PHASE_REQUIREMENT::PHASE_ONE) &&
			!CValtanBrain::Is_PhaseRequirementMet(
				phaseOneBoss, PHASE_REQUIREMENT::PHASE_TWO) &&
			!CValtanBrain::Is_PhaseRequirementMet(
				phaseTwoBoss, PHASE_REQUIREMENT::PHASE_ONE) &&
			CValtanBrain::Is_PhaseRequirementMet(
				phaseTwoBoss, PHASE_REQUIREMENT::PHASE_TWO) &&
			CValtanBrain::Is_PhaseRequirementMet(
				phaseOneBoss, PHASE_REQUIREMENT::ANY) &&
			CValtanBrain::Is_PhaseRequirementMet(
				phaseTwoBoss, PHASE_REQUIREMENT::ANY),
			"Offer a phase-gated pattern only in the phase it names");

		/* Valtan carries no duplicate percent threshold. Exactly one typed edge
		on the 109 central IMPACT owns the phase-two transition. */
		const BOSS_RUNTIME_PROFILE* thresholdProfile =
			catalog.Find_Boss("BOSS_VALTAN");
		const auto* thresholdPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		std::uint32_t transitionBar = 0u;
		std::size_t phaseActionCount = 0u;
		bool exactPhaseAction = false;
		if (nullptr != thresholdPatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *thresholdPatterns)
			{
				if (pattern.strPatternId == "VALTAN_ARENA_BREAK_109")
				{
					transitionBar = pattern.iTriggerHealthBar;
					for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
					{
						for (const BOSS_PATTERN_STAGE_ACTION& action : stage.Actions)
						{
							if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE !=
								action.eKind)
							{
								continue;
							}
							++phaseActionCount;
							exactPhaseAction = "IMPACT" == stage.strStageId &&
								"valtan.mechanic.arena-break-109.impact" ==
									stage.strActionId &&
								BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
									action.eTrigger &&
								"boss.phase.gameplay" == action.strTargetId &&
								2u == action.iValue && 0u == action.iDurationMs;
						}
					}
				}
			}
		}
		tests.Require(
			nullptr != thresholdProfile &&
			BOSS_PHASE_POLICY_KIND::AUTHORED_PATTERN_EVENT ==
				thresholdProfile->PhasePolicy.eKind &&
			0u == thresholdProfile->PhasePolicy.iThresholdPercent &&
			109u == transitionBar && 1u == phaseActionCount && exactPhaseAction,
			"Advance Valtan to phase two only on 109 IMPACT ENTER");

		/* The wipe and both authored terrain-destruction auditions are protected
		while they run. The wipe still fires on the bar its own name carries. */
		const BOSS_PATTERN_DEFINITION* wipePattern = nullptr;
		const BOSS_PATTERN_DEFINITION* terrainThreePattern = nullptr;
		const BOSS_PATTERN_DEFINITION* terrainNinePattern = nullptr;
		std::uint32_t invulnerableCount = 0u;
		if (nullptr != thresholdPatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *thresholdPatterns)
			{
				if (pattern.bInvulnerableWhileRunning)
					++invulnerableCount;
				if (pattern.strPatternId == "VALTAN_FLOOR_WIPE_130")
					wipePattern = &pattern;
				else if (pattern.strPatternId ==
					"VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK")
					terrainThreePattern = &pattern;
				else if (pattern.strPatternId ==
					"VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK")
					terrainNinePattern = &pattern;
			}
		}
		tests.Require(
			nullptr != wipePattern && wipePattern->bInvulnerableWhileRunning &&
			nullptr != terrainThreePattern &&
			terrainThreePattern->bInvulnerableWhileRunning &&
			nullptr != terrainNinePattern &&
			terrainNinePattern->bInvulnerableWhileRunning &&
			130u == wipePattern->iTriggerHealthBar && 4u == invulnerableCount,
			"Protect the entrance camera gate, 130-bar wipe, and both terrain-destruction auditions");

		/* Its second smash is arena wide and lethal to any authored player pool,
		which is what makes it a wipe rather than a large hit. */
		const std::uint32_t wipeRate = catalog.Find_DamageRatePercent(
			"damage.valtan.omnidirectional-wipe-130");
		const BOSS_RUNTIME_PROFILE* wipeBoss = catalog.Find_Boss("BOSS_VALTAN");
		std::uint32_t toughestPlayerHp = 0u;
		for (const CHARACTER_CLASS_ID characterClass : {
				CHARACTER_CLASS_ID::LANCE_MASTER,
				CHARACTER_CLASS_ID::GUNSLINGER,
				CHARACTER_CLASS_ID::SLAYER,
				CHARACTER_CLASS_ID::ARTIST,
				CHARACTER_CLASS_ID::DIMENSIONMASTER })
		{
			const PLAYER_RUNTIME_PROFILE* playerProfile =
				catalog.Find_Player(characterClass);
			if (nullptr != playerProfile)
			{
				toughestPlayerHp = (std::max)(
					toughestPlayerHp, playerProfile->iMaximumHp);
			}
		}
		const BOSS_PATTERN_STAGE_DEFINITION* wipeSmash = nullptr;
		if (nullptr != wipePattern)
		{
			for (const BOSS_PATTERN_STAGE_DEFINITION& stage : wipePattern->Stages)
			{
				if (stage.strDamageProfileId ==
					"damage.valtan.omnidirectional-wipe-130")
				{
					wipeSmash = &stage;
				}
			}
		}
		tests.Require(
			nullptr != wipeBoss && nullptr != wipeSmash && 0u != wipeRate &&
			0u != toughestPlayerHp &&
			BOSS_PATTERN_HIT_SHAPE::CIRCLE == wipeSmash->eHitShape &&
			wipeSmash->fHitOuterRadius >= 100.f &&
			CGameplayCatalog::Resolve_Damage(wipeBoss->iAttackPower, wipeRate) >
				toughestPlayerHp,
			"Reach the whole arena with a wipe hit that outdamages the toughest player pool");
	}
	{
		SERVER_PLAYER missPlayer{};
		missPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		missPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		missPlayer.iCurrentResource = 1000;
		missPlayer.iMaximumResource = 1000;
		missPlayer.fPositionX = 151.f;
		missPlayer.fPositionY = 22.97f;
		missPlayer.fPositionZ = -129.f;
		std::vector<SERVER_WORLD_ENTITY> behindEntities{ boss };
		behindEntities[0].fPositionZ = missPlayer.fPositionZ - 3.5f;
		C2S_USE_SKILL forwardSkill = useSkill;
		forwardSkill.iClientSequence = 3;
		CPlayerSkillSystem missSkills;
		std::vector<DAMAGE_EVENT> missEvents;
		tests.Require(missSkills.Try_Start(missPlayer, forwardSkill, catalog, 100),
			"Approve skill aimed away from the target behind the caster");
		for (std::uint32_t tick = 101; tick < 160; ++tick)
			missSkills.Update(missPlayer, behindEntities, catalog, &navigation, nullptr,
				1.f / 30.f, tick, missEvents);
		tests.Require(10000u == behindEntities[0].iCurrentHp && missEvents.empty(),
			"Leave a target outside the authored hit shape untouched");
	}
	{
		/* Knockback: only hits the source authored with a push move a monster,
		scaled by its profile; the slide runs over the hit's own push window,
		a zero-scale monster never moves, and a wall ends the slide early. */
		const auto makeMonster = [&boss](const float knockbackScale)
		{
			SERVER_WORLD_ENTITY monster{};
			monster.iNetEntityId = 910u;
			monster.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
			monster.eAction = SERVER_ENTITY_ACTION::IDLE;
			monster.strArchetypeId = "MONSTER_VALTAN_PADD_01";
			monster.iCurrentHp = 100000;
			monster.iMaximumHp = 100000;
			/* 1.5 m in front of the attacker: inside 34120's 2.2 m and 34540's
			3.2 m shapes even with a small monster body. */
			monster.fPositionX = 151.f;
			monster.fPositionY = boss.fPositionY;
			monster.fPositionZ = -127.5f;
			monster.fCollisionRadius = 0.6f;
			monster.fHitKnockbackScale = knockbackScale;
			return monster;
		};
		const auto hitWith = [&](const SKILL_ID skillId,
			const PLAYER_STANCE_ID stance,
			std::vector<SERVER_WORLD_ENTITY>& targets,
			std::vector<DAMAGE_EVENT>& events)
		{
			SERVER_PLAYER attacker{};
			attacker.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			attacker.eStance = stance;
			attacker.iCurrentResource = 1000;
			attacker.iMaximumResource = 1000;
			attacker.fPositionX = 151.f;
			attacker.fPositionY = 22.97f;
			attacker.fPositionZ = -129.f;
			C2S_USE_SKILL knockSkill = useSkill;
			knockSkill.iSkillId = skillId;
			knockSkill.iClientSequence = 5;
			CPlayerSkillSystem knockSkills;
			if (!knockSkills.Try_Start(attacker, knockSkill, catalog, 200))
				return false;
			for (std::uint32_t tick = 201; tick < 260; ++tick)
				knockSkills.Update(attacker, targets, catalog, &navigation, nullptr,
					1.f / 30.f, tick, events);
			return true;
		};
		/* 34540 (short spear Q) is authored push=130 ms, pushr=1.0 m on its one
		hit; 34120's three hits are authored push=0. */
		const PLAYER_SKILL_DEFINITION* pushSkill = catalog.Find_Skill(34540);
		tests.Require(
			nullptr != pushSkill && 1u == pushSkill->Hits.size() &&
			130u == pushSkill->Hits[0].iPushMs &&
			std::fabs(pushSkill->Hits[0].fPushRange - 1.f) < 0.0001f &&
			nullptr != talonStrike && 0u == talonStrike->Hits[0].iPushMs &&
			0.f == talonStrike->Hits[0].fPushRange,
			"Load the authored push window and range per hit shape from the gameplay bootstrap");

		std::vector<SERVER_WORLD_ENTITY> unpushed{ makeMonster(1.f) };
		std::vector<DAMAGE_EVENT> unpushedEvents;
		const bool unpushedStarted = hitWith(34120,
			PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR, unpushed, unpushedEvents);
		tests.Require(
			unpushedStarted && !unpushedEvents.empty() &&
			0.f == unpushed[0].fKnockbackRemainingSeconds,
			"Leave a monster unmoved by hits the source authored without a push");

		std::vector<SERVER_WORLD_ENTITY> pushed{ makeMonster(1.f) };
		std::vector<DAMAGE_EVENT> pushedEvents;
		const bool pushedStarted = hitWith(34540,
			PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR, pushed, pushedEvents);
		tests.Require(
			pushedStarted && !pushedEvents.empty() &&
			std::fabs(pushed[0].fKnockbackRemainingSeconds - 0.13f) < 0.0001f &&
			std::fabs(pushed[0].fKnockbackDirectionX) < 0.001f &&
			pushed[0].fKnockbackDirectionZ > 0.999f,
			"Arm a knockback away from the attacker on an authored push hit");
		const float startX = pushed[0].fPositionX;
		const float startZ = pushed[0].fPositionZ;
		std::uint32_t knockbackTicks = 0;
		while (CMonsterBrain::Advance_Knockback(pushed[0], navigation, 1.f / 30.f))
		{
			++knockbackTicks;
			if (knockbackTicks > 30u)
				break;
		}
		tests.Require(
			4u == knockbackTicks &&
			0.f == pushed[0].fKnockbackRemainingSeconds &&
			std::fabs(pushed[0].fPositionX - startX) < 0.01f &&
			std::fabs((pushed[0].fPositionZ - startZ) - 1.f) < 0.01f &&
			SERVER_ENTITY_ACTION::IDLE == pushed[0].eAction,
			"Slide the monster the authored push range over the push window and stop");

		std::vector<SERVER_WORLD_ENTITY> immune{ makeMonster(0.f) };
		std::vector<DAMAGE_EVENT> immuneEvents;
		const bool immuneStarted = hitWith(34540,
			PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR, immune, immuneEvents);
		tests.Require(
			immuneStarted && !immuneEvents.empty() &&
			0.f == immune[0].fKnockbackRemainingSeconds &&
			!CMonsterBrain::Advance_Knockback(immune[0], navigation, 1.f / 30.f) &&
			151.f == immune[0].fPositionX &&
			-127.5f == immune[0].fPositionZ,
			"Leave a zero-scale monster in place when an authored push hit lands");

		SERVER_WORLD_ENTITY walled = makeMonster(1.f);
		walled.fPositionX = lastWalkableX - navCellSize * 0.5f;
		walled.fPositionZ = boundaryProbeZ;
		walled.fKnockbackDirectionX = 1.f;
		walled.fKnockbackDirectionZ = 0.f;
		walled.fKnockbackSpeed = 4.f / 0.13f;
		walled.fKnockbackRemainingSeconds = 0.13f;
		std::uint32_t walledTicks = 0;
		while (CMonsterBrain::Advance_Knockback(walled, navigation, 1.f / 30.f))
		{
			++walledTicks;
			if (walledTicks > 30u)
				break;
		}
		SERVER_NAV_POINT walledPoint{};
		tests.Require(
			walledTicks >= 1u && walledTicks < 5u &&
			0.f == walled.fKnockbackRemainingSeconds &&
			walled.fPositionX < firstBlockedX &&
			navigation.Sample_Position(walled.fPositionX, walled.fPositionZ, walledPoint),
			"Stop a knockback at the non-walkable boundary and end the window");
	}
	{
		/* Player hit reaction: an authored boss/monster push arms away from the
		hit source over its own window, a negative range pulls toward it, a
		FallDown hit holds KNOCKDOWN until downMs, a running window never
		re-arms, and no command passes while the player is down. */
		SERVER_PLAYER victim{};
		victim.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		victim.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		victim.iCurrentHp = 5000;
		victim.iMaximumHp = 5000;
		victim.iCurrentResource = 1000;
		victim.iMaximumResource = 1000;
		victim.fPositionX = 10.f;
		victim.fPositionZ = 5.f;
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			victim, 10.f, 3.f, 2.f, 242u, false, 0u, 300u);
		tests.Require(
			victim.fKnockbackDirectionZ > 0.999f &&
			std::fabs(victim.fKnockbackDirectionX) < 0.001f &&
			std::fabs(victim.fKnockbackRemainingSeconds - 0.242f) < 0.0001f &&
			std::fabs(victim.fKnockbackSpeed - 2.f / 0.242f) < 0.001f &&
			PLAYER_ACTION_STATE::NONE == victim.eAction,
			"Arm a player push away from the hit source without a knockdown");
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			victim, 10.f, 7.f, 2.f, 242u, true, 2000u, 301u);
		tests.Require(
			victim.fKnockbackDirectionZ > 0.999f &&
			PLAYER_ACTION_STATE::NONE == victim.eAction,
			"Keep a running push window from being re-armed by a second hit");
		victim.fKnockbackRemainingSeconds = 0.f;
		victim.fKnockbackSpeed = 0.f;
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			victim, 10.f, 3.f, -2.4f, 97u, true, 2000u, 310u);
		tests.Require(
			victim.fKnockbackDirectionZ < -0.999f &&
			std::fabs(victim.fKnockbackSpeed - 2.4f / 0.097f) < 0.001f &&
			PLAYER_ACTION_STATE::KNOCKDOWN == victim.eAction &&
			310u == victim.iActionStartTick &&
			370u == victim.iKnockdownEndTick &&
			LostArk::Shared::INVALID_SKILL_ID == victim.iCurrentSkillId,
			"Pull the player toward the hit source and hold the authored knockdown");
		C2S_USE_SKILL downSkill{};
		downSkill.iClientSequence = 9;
		downSkill.iSkillId = 34120;
		downSkill.fAimX = 10.f;
		downSkill.fAimZ = 50.f;
		CPlayerSkillSystem downSkills;
		tests.Require(
			!downSkills.Try_Start(victim, downSkill, catalog, 315u),
			"Reject a skill command while the player is knocked down");

		const PLAYER_SKILL_DEFINITION* standup = catalog.Find_Skill(34030);
		tests.Require(
			nullptr != standup &&
			PLAYER_SKILL_KIND::STANDUP == standup->eSkillKind &&
			3000u == standup->iCooldownMs &&
			!standup->RootMotion.empty() &&
			standup->RootMotion.back().fForward > 3.5f,
			"Load the official stand-up skill with its rolling root motion");
		C2S_USE_SKILL standupCommand{};
		standupCommand.iClientSequence = 10;
		standupCommand.iSkillId = 34030;
		standupCommand.fAimX = victim.fPositionX;
		standupCommand.fAimZ = victim.fPositionZ - 5.f;
		tests.Require(
			downSkills.Try_Start(victim, standupCommand, catalog, 320u) &&
			PLAYER_ACTION_STATE::SKILL == victim.eAction &&
			34030u == victim.iCurrentSkillId &&
			0u == victim.iComboStage &&
			0u == victim.iKnockdownEndTick &&
			0.f == victim.fKnockbackRemainingSeconds,
			"Stand the knocked-down player up through the STANDUP skill");

		SERVER_PLAYER standing{};
		standing.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		standing.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		standing.iCurrentHp = 5000;
		standing.iMaximumHp = 5000;
		standing.iCurrentResource = 1000;
		standing.iMaximumResource = 1000;
		C2S_USE_SKILL standingStandup = standupCommand;
		standingStandup.iClientSequence = 1;
		CPlayerSkillSystem standingSkills;
		tests.Require(
			!standingSkills.Try_Start(standing, standingStandup, catalog, 330u),
			"Reject the STANDUP skill while the player is on its feet");

		victim.eAction = PLAYER_ACTION_STATE::NONE;
		victim.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			victim, 10.f, 3.f, 2.f, 242u, true, 2000u, 330u);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == victim.eAction &&
			0.f == victim.fKnockbackRemainingSeconds &&
			380u == victim.iHitReactionGraceEndTick,
			"Hold the get-up grace so a hit cannot chain a second knockdown");
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			victim, 10.f, 3.f, 2.f, 242u, true, 2000u, 385u);
		tests.Require(
			PLAYER_ACTION_STATE::KNOCKDOWN == victim.eAction &&
			victim.fKnockbackRemainingSeconds > 0.f,
			"Arm the hit reaction again once the get-up grace has passed");

		const auto* valtanPatterns = catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const BOSS_PATTERN_DEFINITION* swing = nullptr;
		const BOSS_PATTERN_DEFINITION* roar = nullptr;
		if (nullptr != valtanPatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *valtanPatterns)
			{
				if ("VALTAN_SWING" == pattern.strPatternId)
					swing = &pattern;
				if ("VALTAN_IMPRISON_ROAR" == pattern.strPatternId)
					roar = &pattern;
			}
		}
		const auto findActivePush = [](const BOSS_PATTERN_DEFINITION* pattern)
			-> const BOSS_PATTERN_STAGE_DEFINITION*
		{
			if (nullptr == pattern)
				return nullptr;
			for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern->Stages)
			{
				if (!stage.strDamageProfileId.empty())
					return &stage;
			}
			return nullptr;
		};
		const BOSS_PATTERN_STAGE_DEFINITION* swingHit = findActivePush(swing);
		const BOSS_PATTERN_STAGE_DEFINITION* roarHit = findActivePush(roar);
		tests.Require(
			nullptr != swingHit && nullptr != roarHit &&
			std::fabs(swingHit->fPushRangeM - 2.f) < 0.0001f &&
			242u == swingHit->iPushMs && swingHit->bKnockdown &&
			2000u == swingHit->iDownMs &&
			roarHit->fPushRangeM < 0.f && roarHit->bKnockdown,
			"Load the official swing push and the imprison-roar pull from the encounter");
	}
	{
		/* Projectiles: the Artist tiger (31490, official 314900) is a missile
		that leaves the caster 1147 ms in, runs 11 m at 6.5 m/s and carries a
		1.5 x 2.5 m box; the DimensionMaster nail (2050100, 20501001) is a
		fixed area that stands 0.8 m in front of the caster (AreaOrigin 0,
		AreaOffsetX 80) and fires its 1 m circle the moment it appears. */
		const PLAYER_SKILL_DEFINITION* tigerSkill = catalog.Find_Skill(31490);
		tests.Require(
			nullptr != tigerSkill && 1u == tigerSkill->Projectiles.size() &&
			PLAYER_PROJECTILE_KIND::MISSILE == tigerSkill->Projectiles[0].eKind &&
			1147u == tigerSkill->Projectiles[0].iTimeMs &&
			std::fabs(tigerSkill->Projectiles[0].fSpeed - 6.5f) < 0.001f &&
			std::fabs(tigerSkill->Projectiles[0].fMaxDistance - 11.f) < 0.001f &&
			3000u == tigerSkill->Projectiles[0].iLifeMs &&
			1u == tigerSkill->Projectiles[0].Hits.size() &&
			tigerSkill->Projectiles[0].Hits[0].isContact &&
			2u == tigerSkill->Projectiles[0].Hits[0].Hit.iAreaType,
			"Load the authored missile definition of a skill from the gameplay bootstrap");

		const auto makeArtist = []()
		{
			SERVER_PLAYER artist{};
			artist.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
			artist.iCurrentHp = 1000;
			artist.iMaximumHp = 1000;
			artist.iCurrentResource = 1000;
			artist.iMaximumResource = 1000;
			artist.iCurrentIdentity = 1000;
			artist.iMaximumIdentity = 1000;
			artist.fPositionX = 151.f;
			artist.fPositionY = 22.97f;
			artist.fPositionZ = -129.f;
			return artist;
		};
		const auto makeTargetAt = [&boss](const float distanceAhead)
		{
			SERVER_WORLD_ENTITY target = boss;
			target.iNetEntityId = 930u;
			target.iCurrentHp = 100000;
			target.iMaximumHp = 100000;
			target.fPositionZ = -129.f + distanceAhead;
			return target;
		};

		/* The missile reaches a target eight metres away when its authored
		forward box first overlaps the current profile body, not a retired radius. */
		SERVER_PLAYER tigerCaster = makeArtist();
		std::vector<SERVER_WORLD_ENTITY> tigerTargets{ makeTargetAt(8.f) };
		C2S_USE_SKILL tigerCommand{};
		tigerCommand.iClientSequence = 1;
		tigerCommand.iSkillId = 31490;
		tigerCommand.fAimX = tigerCaster.fPositionX;
		tigerCommand.fAimZ = tigerCaster.fPositionZ + 8.f;
		CPlayerSkillSystem tigerSkills;
		std::vector<DAMAGE_EVENT> tigerEvents;
		tests.Require(tigerSkills.Try_Start(tigerCaster, tigerCommand, catalog, 10),
			"Approve the tiger skill aimed past a distant target");
		std::uint32_t firstHitTick = 0;
		for (std::uint32_t tick = 11; tick < 200; ++tick)
		{
			tigerSkills.Update(tigerCaster, tigerTargets, catalog, &navigation, nullptr,
				1.f / 30.f, tick, tigerEvents);
			if (0u == firstHitTick && !tigerEvents.empty())
				firstHitTick = tick;
			if (tick == 50u)
			{
				tests.Require(
					tigerEvents.empty() && 1u == tigerCaster.Projectiles.size(),
					"Spawn the missile at its authored time and land nothing before it arrives");
			}
		}
		const PLAYER_SKILL_PROJECTILE& tigerProjectile = tigerSkill->Projectiles[0];
		constexpr std::uint32_t tigerTickRate = 30u;
		const float tigerContactDistance = 8.f - boss.fCollisionRadius -
			tigerProjectile.fOffsetForward - tigerProjectile.Hits[0].Hit.fOffset -
			tigerProjectile.Hits[0].Hit.fRange;
		const std::uint32_t expectedTigerContactTick = 10u +
			(tigerProjectile.iTimeMs * tigerTickRate + 999u) / 1000u +
			static_cast<std::uint32_t>(std::ceil(tigerContactDistance *
				static_cast<float>(tigerTickRate) / tigerProjectile.fSpeed));
		// Allow one tick for float accumulation at the contact boundary.
		tests.Require(
			1u == tigerEvents.size() && firstHitTick + 1u >= expectedTigerContactTick &&
			firstHitTick <= expectedTigerContactTick + 1u &&
			tigerTargets[0].iNetEntityId == tigerEvents[0].iTargetNetEntityId,
			"Land the missile's contact hit once when its box reaches the target");
		const std::uint32_t tigerRate = catalog.Find_DamageRatePercent(
			tigerSkill->strDamageProfileId);
		const PLAYER_RUNTIME_PROFILE* artistProfile =
			catalog.Find_Player(CHARACTER_CLASS_ID::ARTIST);
		const std::uint32_t tigerTotal = CGameplayCatalog::Resolve_Damage(
			nullptr == artistProfile ? 0u : artistProfile->iAttackPower, tigerRate);
		/* One caster hit and one missile hit share the rate: the missile lands
		the second half. */
		tests.Require(
			1u == tigerEvents.size() && tigerEvents[0].iAmount == tigerTotal - tigerTotal / 2u &&
			100000u - tigerEvents[0].iAmount == tigerTargets[0].iCurrentHp,
			"Split the skill rate between the caster hit and the missile hit");
		tests.Require(
			tigerCaster.Projectiles.empty() &&
			PLAYER_ACTION_STATE::NONE == tigerCaster.eAction,
			"Retire the missile after its authored distance while the caster is already free");

		/* Behind the caster the tiger never turns: no hit, and the target keeps
		its HP after the whole flight. */
		SERVER_PLAYER missCaster = makeArtist();
		std::vector<SERVER_WORLD_ENTITY> missTargets{ makeTargetAt(-8.f) };
		CPlayerSkillSystem missSkills;
		std::vector<DAMAGE_EVENT> missEvents;
		tests.Require(missSkills.Try_Start(missCaster, tigerCommand, catalog, 10),
			"Approve the tiger skill aimed away from a target behind the caster");
		for (std::uint32_t tick = 11; tick < 200; ++tick)
			missSkills.Update(missCaster, missTargets, catalog, &navigation, nullptr,
				1.f / 30.f, tick, missEvents);
		tests.Require(missEvents.empty() && 100000u == missTargets[0].iCurrentHp,
			"Leave a target outside the missile's path untouched");

		const PLAYER_SKILL_DEFINITION* nailSkill = catalog.Find_Skill(2050100);
		tests.Require(
			nullptr != nailSkill && 1u == nailSkill->Projectiles.size() &&
			PLAYER_PROJECTILE_KIND::FIXAREA == nailSkill->Projectiles[0].eKind &&
			PLAYER_PROJECTILE_ORIGIN::CASTER == nailSkill->Projectiles[0].eOrigin &&
			std::fabs(nailSkill->Projectiles[0].fOffsetForward - 0.8f) < 0.001f &&
			280u == nailSkill->Projectiles[0].iTimeMs &&
			!nailSkill->Projectiles[0].Hits[0].isContact &&
			0u == nailSkill->Projectiles[0].Hits[0].Hit.iTimeMs,
			"Load the authored fixed-area definition of a skill from the gameplay bootstrap");
		SERVER_PLAYER nailCaster = makeArtist();
		nailCaster.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
		/* Aim twelve metres out: the area stays at its authored caster offset.
		One current-profile body overlaps its circle; the distant body cannot. */
		const float nailContactDistance = nailSkill->Projectiles[0].fOffsetForward +
			boss.fCollisionRadius + nailSkill->Projectiles[0].Hits[0].Hit.fRange - 0.25f;
		std::vector<SERVER_WORLD_ENTITY> nailTargets{
			makeTargetAt(nailContactDistance), makeTargetAt(9.5f) };
		nailTargets[1].iNetEntityId = 931u;
		C2S_USE_SKILL nailCommand{};
		nailCommand.iClientSequence = 1;
		nailCommand.iSkillId = 2050100;
		nailCommand.fAimX = nailCaster.fPositionX;
		nailCommand.fAimZ = nailCaster.fPositionZ + 12.f;
		CPlayerSkillSystem nailSkills;
		std::vector<DAMAGE_EVENT> nailEvents;
		tests.Require(nailSkills.Try_Start(nailCaster, nailCommand, catalog, 10),
			"Approve the nail skill aimed beyond the area's reach");
		for (std::uint32_t tick = 11; tick < 40; ++tick)
			nailSkills.Update(nailCaster, nailTargets, catalog, &navigation, nullptr,
				1.f / 30.f, tick, nailEvents);
		tests.Require(
			1u == nailEvents.size() &&
			nailTargets[0].iNetEntityId == nailEvents[0].iTargetNetEntityId &&
			100000u == nailTargets[1].iCurrentHp,
			"Drop the fixed area at the caster's authored offset and fire its timed hit there");
	}
}

