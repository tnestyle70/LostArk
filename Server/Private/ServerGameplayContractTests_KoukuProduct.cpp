#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "ClientSession.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "KoukuSaydonBrain.h"
#include "Network/PacketReader.h"
#include "ServerNavigation.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_KoukuProduct(TESTS& tests, CGameplayCatalog& catalog)
{


	{
		const auto* koukuParts =
			catalog.Find_BossParts("BOSS_KAKULSAYDON_G1_KOUKU");
		const auto* valtanParts = catalog.Find_BossParts("BOSS_VALTAN");
		const auto* patterns =
			catalog.Find_BossPatterns("ENCOUNTER_KAKULSAYDON_G1");
		const auto* sequence =
			catalog.Find_BossPatternSequence("ENCOUNTER_KAKULSAYDON_G1");
		const std::uint32_t productSourceRevision =
			CKoukuSaydonBrain::Resolve_ProductSourceRevision(catalog);
		/* The Gate 1 Saydon owns every Product pattern; the first one in the
		authored Play All order is the reference occurrence these tests run. */
		const std::string firstProductId =
			nullptr != sequence && !sequence->PatternIds.empty() ?
				sequence->PatternIds.front() : std::string{};
		const auto firstProduct = nullptr == patterns ?
			std::vector<BOSS_PATTERN_DEFINITION>::const_iterator{} :
			std::find_if(patterns->begin(), patterns->end(),
				[&firstProductId](const BOSS_PATTERN_DEFINITION& pattern)
				{
					return firstProductId == pattern.strPatternId;
				});
		const std::size_t firstProductStageCount =
			nullptr != patterns && patterns->end() != firstProduct ?
				firstProduct->Stages.size() : 0u;
		const std::uint32_t firstProductLastStage = 0u == firstProductStageCount ?
			0u : static_cast<std::uint32_t>(firstProductStageCount - 1u);
		std::string animationStatus;
		const bool exactProduct = 0u != productSourceRevision && nullptr != patterns &&
			patterns->end() != firstProduct && nullptr != sequence &&
			"KAKULSAYDON_G1_PLAY_ALL" == sequence->strSequenceId &&
			!sequence->PatternIds.empty() && !firstProductId.empty() &&
			CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
				*firstProduct, animationStatus);
		tests.Require(
			(nullptr == koukuParts || koukuParts->empty()) &&
			nullptr != valtanParts && !valtanParts->empty(),
			"Admit an unarmoured KoukuSaydon boss while retaining Valtan exact parts");
		tests.Require(exactProduct,
			"Admit the exact KoukuSaydon Saydon animation-only Product sequence");

		if (exactProduct)
		{
			SERVER_WORLD_ENTITY boss{};
			boss.iNetEntityId = 777u;
			boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			boss.eAction = SERVER_ENTITY_ACTION::IDLE;
			boss.strEncounterId = "ENCOUNTER_KAKULSAYDON_G1";
			boss.strArchetypeId = "BOSS_KAKULSAYDON_G1_KOUKU";
			boss.strPlacementId = "boss.kakulsaydon.g1.kouku";
			boss.iCurrentHp = 1000000u;
			boss.iMaximumHp = 1000000u;
			boss.PinnedDefinitionRevision = catalog.Get_ActiveRevision();
			CKoukuSaydonBrain brain;
			std::string status;
			const bool began = brain.Begin_Pattern(
				boss, *firstProduct, catalog.Get_ActiveRevision(), 1u, status);
			std::vector<std::string> visitedActions;
			if (began)
				visitedActions.push_back(boss.strActionId);
			KOUKUSAYDON_BRAIN_UPDATE_RESULT terminal =
				KOUKUSAYDON_BRAIN_UPDATE_RESULT::IDLE;
			for (std::uint32_t tick = 1u; began && tick < 700u; ++tick)
			{
				const std::string previousAction = boss.strActionId;
				terminal = brain.Update(boss, catalog, tick, status);
				if (KOUKUSAYDON_BRAIN_UPDATE_RESULT::STAGE_CHANGED == terminal &&
					boss.strActionId != previousAction)
				{
					visitedActions.push_back(boss.strActionId);
				}
				if (KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED == terminal ||
					KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_INVALID_DEFINITION ==
						terminal)
				{
					break;
				}
			}
			tests.Require(began && firstProductStageCount == visitedActions.size() &&
				KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED == terminal &&
				boss.strPatternId.empty() &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
					boss.PatternTerminalReceipt.eResult &&
				boss.PatternTerminalReceipt.iPatternSequence == boss.iPatternSequence,
				"Run every stage of the first KoukuSaydon Product on the fixed Server clock");

			BOSS_PATTERN_DEFINITION unsupported = *firstProduct;
			unsupported.Stages.front().Actions.push_back({});
			const bool rejectsAction =
				!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
					unsupported, status);
			unsupported = *firstProduct;
			unsupported.Stages.front().eHitShape = BOSS_PATTERN_HIT_SHAPE::CIRCLE;
			const bool rejectsHit =
				!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
					unsupported, status);
			unsupported = *firstProduct;
			unsupported.Stages.front().eHitActivationKind =
				BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW;
			const bool rejectsHitWindow =
				!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
					unsupported, status);
			unsupported = *firstProduct;
			unsupported.Stages.front().Motion.eKind =
				BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD;
			const bool rejectsMotion =
				!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
					unsupported, status);
			unsupported = *firstProduct;
			unsupported.Stages.front().Branches.push_back({
				BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT,
				unsupported.Stages[1u].strActionId, {} });
			const bool rejectsBranch =
				!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
					unsupported, status);
			tests.Require(rejectsAction && rejectsHit && rejectsHitWindow && rejectsMotion &&
				rejectsBranch,
				"Fail closed on KoukuSaydon action hit motion and non-timeout branch data");

			{
				/* Different boss bodies share one authored Product order. Selection
				must retain that order and each retained pattern's outgoing pause. */
				std::vector<BOSS_PATTERN_DEFINITION> mixedPatterns(3u, patterns->front());
				mixedPatterns[0].strPatternId = "KAKULSAYDON_TEST_KOUKU_FIRST";
				mixedPatterns[1].strPatternId = "KAKULSAYDON_TEST_SAYDON";
				mixedPatterns[2].strPatternId = "KAKULSAYDON_TEST_KOUKU_LAST";
				mixedPatterns[0].AuditionBossArchetypeIds = { "BOSS_KAKULSAYDON_G2_KOUKU" };
				mixedPatterns[1].AuditionBossArchetypeIds = { "BOSS_KAKULSAYDON_G1_SAYDON" };
				mixedPatterns[2].AuditionBossArchetypeIds = { "BOSS_KAKULSAYDON_G2_KOUKU" };
				BOSS_PATTERN_SEQUENCE_DEFINITION mixedSequence = *sequence;
				mixedSequence.PatternIds = { mixedPatterns[0].strPatternId,
					mixedPatterns[1].strPatternId, mixedPatterns[2].strPatternId };
				mixedSequence.iExpectedStepCount = 3u;
				mixedSequence.TransitionPursuitTicks = { 7u, 19u };
				std::vector<std::string> selectedIds;
				std::vector<std::uint32_t> selectedTransitions;
				std::string selectionStatus;
				const bool koukuSelected = CKoukuSaydonBrain::Select_AnimationOnlySequence(
					mixedPatterns, mixedSequence, "BOSS_KAKULSAYDON_G2_KOUKU",
					selectedIds, selectedTransitions, selectionStatus) &&
					selectedIds == std::vector<std::string>{ mixedPatterns[0].strPatternId,
						mixedPatterns[2].strPatternId } &&
					selectedTransitions == std::vector<std::uint32_t>{ 7u };
				const bool saydonSelected = CKoukuSaydonBrain::Select_AnimationOnlySequence(
					mixedPatterns, mixedSequence, "BOSS_KAKULSAYDON_G1_SAYDON",
					selectedIds, selectedTransitions, selectionStatus) &&
					selectedIds == std::vector<std::string>{ mixedPatterns[1].strPatternId } &&
					selectedTransitions.empty();
				const bool emptyPreserved = !CKoukuSaydonBrain::Select_AnimationOnlySequence(
					mixedPatterns, mixedSequence, "BOSS_KAKULSAYDON_G2_BIG_SAYDON",
					selectedIds, selectedTransitions, selectionStatus) &&
					!selectionStatus.empty() &&
					selectedIds == std::vector<std::string>{ mixedPatterns[1].strPatternId } &&
					selectedTransitions.empty();
				tests.Require(koukuSelected && saydonSelected && emptyPreserved,
					"Select Play All by boss body in authored order and preserve selection on no compatible Product");
			}

			{
				/* G2 Big Saydon is tuned above the floor. A saved Y must survive
				the same entity build and fixed ticks that consume the world bootstrap. */
				auto heightRoom = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
				const auto* bigPlacement = heightRoom->Find_Placement("boss.kakulsaydon.g2.big-saydon");
				const auto* regularPlacement = heightRoom->Find_Placement("boss.kakulsaydon.g1.saydon");
				const bool heightFixtureReady = heightRoom->Is_Ready() &&
					nullptr != bigPlacement && nullptr != regularPlacement &&
					heightRoom->m_ServerNavigation.Is_Loaded();
				tests.Require(heightFixtureReady, "Load the saved Big Saydon height contract placements and navigation");
				if (heightFixtureReady)
				{
					constexpr float authoredHeight = 8.63f;
					WORLD_BOOTSTRAP_PLACEMENT raisedPlacement = *bigPlacement;
					raisedPlacement.fPositionY = authoredHeight;
					auto raisedBoss = std::make_unique<SERVER_WORLD_ENTITY>();
					SERVER_NAV_POINT bigGround{};
					const bool raisedBuilt = heightRoom->m_ServerNavigation.Is_PointWalkableExact(
							raisedPlacement.fPositionX, raisedPlacement.fPositionZ) &&
						heightRoom->m_ServerNavigation.Sample_Position(
							raisedPlacement.fPositionX, raisedPlacement.fPositionZ, bigGround) &&
						heightRoom->Build_WorldEntity(raisedPlacement, heightRoom->m_iNextNetEntityId, *raisedBoss);
					tests.Require(raisedBuilt && std::abs(bigGround.y - authoredHeight) > 0.1f &&
						std::abs(raisedBoss->fPositionX - raisedPlacement.fPositionX) < 0.001f &&
						std::abs(raisedBoss->fPositionZ - raisedPlacement.fPositionZ) < 0.001f &&
						std::abs(raisedBoss->fPositionY - authoredHeight) < 0.001f &&
						std::abs(raisedBoss->fSpawnPositionY - authoredHeight) < 0.001f,
						"Keep Big Saydon saved Y 8.63 in current and spawn transforms without shifting walkable XZ");
					bool heightPersisted = raisedBuilt;
					if (raisedBuilt)
					{
						const NET_ENTITY_ID raisedId = raisedBoss->iNetEntityId;
						++heightRoom->m_iNextNetEntityId;
						heightRoom->m_WorldEntities.push_back(std::move(*raisedBoss));
						for (std::uint32_t tick = 0u; heightPersisted && tick < 90u; ++tick)
						{
							++heightRoom->m_iServerTick;
							heightRoom->Update_WorldEntities(1.f / 30.f);
							const auto live = std::find_if(heightRoom->m_WorldEntities.begin(),
								heightRoom->m_WorldEntities.end(), [raisedId](const SERVER_WORLD_ENTITY& candidate)
								{ return candidate.iNetEntityId == raisedId; });
							heightPersisted = heightRoom->m_WorldEntities.end() != live &&
								SERVER_ENTITY_ACTION::IDLE == live->eAction && live->strPatternId.empty() &&
								std::abs(live->fPositionY - authoredHeight) < 0.001f &&
								std::abs(live->fSpawnPositionY - authoredHeight) < 0.001f;
						}
					}
					tests.Require(heightPersisted, "Keep the spawned Big Saydon authored height through 90 arena fixed ticks");
					WORLD_BOOTSTRAP_PLACEMENT regularRaised = *regularPlacement;
					regularRaised.fPositionY = authoredHeight;
					auto regularBoss = std::make_unique<SERVER_WORLD_ENTITY>();
					SERVER_NAV_POINT regularGround{};
					const bool regularBuilt = heightRoom->m_ServerNavigation.Sample_Position(
							regularRaised.fPositionX, regularRaised.fPositionZ, regularGround) &&
						heightRoom->Build_WorldEntity(regularRaised, heightRoom->m_iNextNetEntityId, *regularBoss);
					tests.Require(regularBuilt && std::abs(regularGround.y - authoredHeight) > 0.1f &&
						std::abs(regularBoss->fPositionY - regularGround.y) < 0.001f &&
						std::abs(regularBoss->fSpawnPositionY - regularGround.y) < 0.001f,
						"Keep ordinary arena bosses on navigation height despite a different authored Y");
				}
			}

			{
				/* The F1 gate buttons raise disabled arena boss placements. Such a
				boss shares the Gate 1 encounter but never owns a brain: it must
				idle through fixed ticks until an audition names it, and the Debug
				revert must remove it while keeping the enabled Gate 1 Kouku. */
				auto arenaRoom = std::make_unique<CGameRoom>(
					WORLD_ID::KAKULSAYDON_ARENA);
				const WORLD_BOOTSTRAP_PLACEMENT* gateSaydon =
					arenaRoom->Find_Placement("boss.kakulsaydon.g1.saydon");
				const WORLD_BOOTSTRAP_PLACEMENT* gateKouku =
					arenaRoom->Find_Placement("boss.kakulsaydon.g1.kouku");
				const bool placementsAdmitted = arenaRoom->Is_Ready() &&
					nullptr != gateSaydon && nullptr != gateKouku &&
					CKoukuSaydonBrain::Is_ArenaBossPlacement(
						WORLD_ID::KAKULSAYDON_ARENA, *gateSaydon) &&
					!CKoukuSaydonBrain::Is_ArenaBossPlacement(
						WORLD_ID::KAKULSAYDON_ARENA, *gateKouku) &&
					!CKoukuSaydonBrain::Is_ArenaBossPlacement(
						WORLD_ID::CHARACTER_SELECT_ARENA, *gateSaydon);
				tests.Require(placementsAdmitted,
					"Admit only disabled KoukuSaydon gate boss placements for the arena Debug spawn");

				SERVER_WORLD_ENTITY gateBoss{};
				const bool built = placementsAdmitted &&
					arenaRoom->Build_WorldEntity(
						*gateSaydon, arenaRoom->m_iNextNetEntityId, gateBoss);
				bool idle = built;
				NET_ENTITY_ID gateId = INVALID_NET_ENTITY_ID;
				if (built)
				{
					gateId = gateBoss.iNetEntityId;
					++arenaRoom->m_iNextNetEntityId;
					arenaRoom->m_WorldEntities.push_back(gateBoss);
					for (std::uint32_t tick = 0u; idle && tick < 90u; ++tick)
					{
						/* Advance the room clock the way the fixed tick does, so
						the brain sees 90 distinct ticks rather than one. */
						++arenaRoom->m_iServerTick;
						arenaRoom->Update_WorldEntities(1.f / 30.f);
						const auto live = std::find_if(
							arenaRoom->m_WorldEntities.begin(),
							arenaRoom->m_WorldEntities.end(),
							[gateId](const SERVER_WORLD_ENTITY& candidate)
							{
								return candidate.iNetEntityId == gateId;
							});
						const SERVER_WORLD_ENTITY* auditionBoss =
							arenaRoom->Find_KoukuSaydonAuditionBoss();
						idle = arenaRoom->m_WorldEntities.end() != live &&
							live->strPatternId.empty() &&
							SERVER_ENTITY_ACTION::IDLE == live->eAction &&
							live->iCurrentHp == live->iMaximumHp &&
							CKoukuSaydonBrain::Is_ArenaBoss(
								WORLD_ID::KAKULSAYDON_ARENA, *live) &&
							!CKoukuSaydonBrain::Is_GateOneBoss(
								WORLD_ID::KAKULSAYDON_ARENA, *live) &&
							nullptr != auditionBoss &&
							auditionBoss->iNetEntityId != gateId;
					}
				}
				tests.Require(built && idle,
					"Keep a Debug-activated KoukuSaydon gate boss idle without a brain or pattern");

				/* The arena navgrid has 4 m cells. Snapping the raised boss to
				its cell centre moved it 1.29 m from the fixed Gate 1 player
				position, so the gate teleport was refused as a collision. */
				arenaRoom->Refresh_PlayerBlockingBodies();
				tests.Require(built &&
					std::abs(gateBoss.fPositionX - gateSaydon->fPositionX) < 0.001f &&
					std::abs(gateBoss.fPositionZ - gateSaydon->fPositionZ) < 0.001f &&
					arenaRoom->m_ServerCollisionSystem.Is_PlayerPositionClear(
						-2.84f, 1.32f, 941.02f, INVALID_NET_ENTITY_ID),
					"Keep a raised KoukuSaydon gate boss on its authored XZ so the Gate 1 player position stays clear");

				{
					/* The Debug clown toggle swaps only the replicated madness
					form; the gauge, HP and class stay untouched, and a dead
					player keeps the body its death was authored on. */
					auto formStorage = std::make_unique<SERVER_PLAYER>();
					SERVER_PLAYER& formPlayer = *formStorage;
					formPlayer.iPlayerId = 321u;
					formPlayer.iNetEntityId = 654u;
					formPlayer.iCurrentHp = formPlayer.iMaximumHp = 100u;
					C2S_DEBUG_SET_MADNESS_FORM formRequest{};
					formRequest.iRequestSequence = 1u;
					formRequest.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
					formRequest.eForm = PLAYER_MADNESS_FORM::CLOWN;
					auto formVerdict = arenaRoom->Apply_DebugMadnessForm(formPlayer, formRequest);
#ifndef _DEBUG
					tests.Require(
						DEBUG_MADNESS_FORM_RESULT::REJECTED_DISABLED == formVerdict.eResult &&
						PLAYER_MADNESS_FORM::NORMAL == formPlayer.eMadnessForm,
						"Release refuses the Debug madness form toggle");
#else
					tests.Require(
						DEBUG_MADNESS_FORM_RESULT::ACCEPTED == formVerdict.eResult &&
						PLAYER_MADNESS_FORM::CLOWN == formVerdict.eActiveForm &&
						PLAYER_MADNESS_FORM::CLOWN == formPlayer.eMadnessForm &&
						0u == formPlayer.iCurrentMadness && 100u == formPlayer.iCurrentHp,
						"Accept the Debug clown form without touching gauge or HP");
					formVerdict = arenaRoom->Apply_DebugMadnessForm(formPlayer, formRequest);
					tests.Require(
						DEBUG_MADNESS_FORM_RESULT::ACCEPTED == formVerdict.eResult,
						"Replay of the same madness form sequence returns the stored verdict");
					formRequest.iRequestSequence = 2u;
					formVerdict = arenaRoom->Apply_DebugMadnessForm(formPlayer, formRequest);
					tests.Require(
						DEBUG_MADNESS_FORM_RESULT::REJECTED_SAME_FORM == formVerdict.eResult &&
						PLAYER_MADNESS_FORM::CLOWN == formPlayer.eMadnessForm,
						"Reject a madness form the player already presents");
					formRequest.iRequestSequence = 3u;
					formRequest.eForm = PLAYER_MADNESS_FORM::NORMAL;
					formPlayer.iCurrentHp = 0u;
					formPlayer.eAction = PLAYER_ACTION_STATE::DEAD;
					formVerdict = arenaRoom->Apply_DebugMadnessForm(formPlayer, formRequest);
					tests.Require(
						DEBUG_MADNESS_FORM_RESULT::REJECTED_PLAYER_STATE == formVerdict.eResult &&
						PLAYER_MADNESS_FORM::CLOWN == formPlayer.eMadnessForm,
						"Reject a madness form change on a dead player");
					formPlayer.iCurrentHp = 100u;
					formPlayer.eAction = PLAYER_ACTION_STATE::NONE;
					formRequest.iRequestSequence = 4u;
					formVerdict = arenaRoom->Apply_DebugMadnessForm(formPlayer, formRequest);
					tests.Require(
						DEBUG_MADNESS_FORM_RESULT::ACCEPTED == formVerdict.eResult &&
						PLAYER_MADNESS_FORM::NORMAL == formPlayer.eMadnessForm,
						"Return the player body through the same Debug toggle");
#endif
				}

#ifdef _DEBUG
				{
					/* The audition scope may name the enabled Gate 1 Kouku, but a
					Saydon-body pattern must not play on the Kouku body. */
					C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST saydonRequest{};
					saydonRequest.iRequestSequence = 1u;
					saydonRequest.eOperation =
						KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
					saydonRequest.Scope.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
					saydonRequest.Scope.strEncounterId = "ENCOUNTER_KAKULSAYDON_G1";
					saydonRequest.Scope.strBossPlacementId = "boss.kakulsaydon.g1.kouku";
					saydonRequest.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G1_KOUKU";
					saydonRequest.Scope.ExpectedGameplayRevision =
						arenaRoom->m_GameplayCatalog.Get_ActiveRevision();
					saydonRequest.Scope.iExpectedSourceRevision =
						CKoukuSaydonBrain::Resolve_ProductSourceRevision(
							arenaRoom->m_GameplayCatalog.Active());
					saydonRequest.strPatternId = firstProductId;
					S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT saydonResult{};
					const KOUKUSAYDON_PATTERN_AUDITION_RESULT saydonVerdict =
						built && idle ?
							arenaRoom->Evaluate_KoukuSaydonPatternAudition(
								8102u, saydonRequest, saydonResult) :
							KOUKUSAYDON_PATTERN_AUDITION_RESULT::END;
					tests.Require(
						KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_UNSUPPORTED_PATTERN ==
							saydonVerdict &&
						CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
							arenaRoom->m_KoukuSaydonPatternAudition.ePhase,
						"Reject a Saydon-body pattern audition on the Gate 1 Kouku boss");
				}

				/* Exercise the room dispatch with the original Kouku still first
				in its entity list. Updating that idle actor must never abort an
				accepted occurrence owned by a later, Debug-activated boss. */
				const WORLD_BOOTSTRAP_PLACEMENT* secondKoukuPlacement =
					arenaRoom->Find_Placement("boss.kakulsaydon.g3.saydon");
				auto secondKouku = std::make_unique<SERVER_WORLD_ENTITY>();
				const bool secondBuilt = nullptr != secondKoukuPlacement &&
					arenaRoom->Build_WorldEntity(*secondKoukuPlacement,
						arenaRoom->m_iNextNetEntityId, *secondKouku);
				const NET_ENTITY_ID secondKoukuId = secondKouku->iNetEntityId;
				if (secondBuilt)
				{
					++arenaRoom->m_iNextNetEntityId;
					arenaRoom->m_WorldEntities.push_back(std::move(*secondKouku));
				}
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST secondRequest{};
				secondRequest.iRequestSequence = 1u;
				secondRequest.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				secondRequest.Scope.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
				secondRequest.Scope.strEncounterId = "ENCOUNTER_KAKULSAYDON_G1";
				secondRequest.Scope.strBossPlacementId = "boss.kakulsaydon.g3.saydon";
				secondRequest.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G3_SAYDON";
				secondRequest.Scope.ExpectedGameplayRevision =
					arenaRoom->m_GameplayCatalog.Get_ActiveRevision();
				secondRequest.Scope.iExpectedSourceRevision =
					CKoukuSaydonBrain::Resolve_ProductSourceRevision(arenaRoom->m_GameplayCatalog.Active());
				secondRequest.strPatternId = firstProductId;
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT secondResult{};
				const bool secondQueued = secondBuilt &&
					KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
						arenaRoom->Evaluate_KoukuSaydonPatternAudition(8103u, secondRequest, secondResult);
				bool originalStayedIdle = secondQueued;
				for (std::uint32_t tick = 0u; originalStayedIdle && tick < 700u; ++tick)
				{
					arenaRoom->Update_WorldEntities(1.f / 30.f);
					++arenaRoom->m_iServerTick;
					const SERVER_WORLD_ENTITY* original = arenaRoom->Find_KoukuSaydonAuditionBoss();
					originalStayedIdle = nullptr != original && original->strPatternId.empty() &&
						SERVER_ENTITY_ACTION::IDLE == original->eAction;
					if (CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
						arenaRoom->m_KoukuSaydonPatternAudition.ePhase)
						break;
				}
				const SERVER_WORLD_ENTITY* completedKouku = arenaRoom->Find_KoukuSaydonArenaBoss(
					secondRequest.Scope.strBossPlacementId, secondRequest.Scope.strBossArchetypeId);
				const auto& lifecycle = arenaRoom->m_PendingKoukuSaydonPatternAuditionLifecycle;
				const bool completed = secondQueued && originalStayedIdle && nullptr != completedKouku &&
					completedKouku->strPatternId.empty() &&
					SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
						completedKouku->PatternTerminalReceipt.eResult &&
					std::any_of(lifecycle.begin(), lifecycle.end(),
						[secondKoukuId](const auto& entry)
						{
							return secondKoukuId == entry.Message.iBossNetEntityId &&
								KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED == entry.Message.eState;
						}) &&
					std::none_of(lifecycle.begin(), lifecycle.end(), [](const auto& entry)
						{ return KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED == entry.Message.eState; });
				tests.Require(completed,
					"Complete a raised Gate 3 Saydon pattern through room ticks while the original Kouku stays idle");

				secondRequest.iRequestSequence = 2u;
				secondRequest.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_ALL;
				secondRequest.strPatternId.clear();
				const bool allQueued = completed &&
					KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
						arenaRoom->Evaluate_KoukuSaydonPatternAudition(8103u, secondRequest, secondResult);
				std::uint32_t stageBeforeDespawn = 0u;
				for (std::uint32_t tick = 0u; allQueued && tick < 200u; ++tick)
				{
					arenaRoom->Update_WorldEntities(1.f / 30.f);
					++arenaRoom->m_iServerTick;
					const SERVER_WORLD_ENTITY* playing = arenaRoom->Find_KoukuSaydonArenaBoss(
						secondRequest.Scope.strBossPlacementId, secondRequest.Scope.strBossArchetypeId);
					if (nullptr != playing)
						stageBeforeDespawn = playing->iPatternStageIndex;
					if (stageBeforeDespawn > 0u)
						break;
				}
				const bool runningBeforeDespawn = allQueued && stageBeforeDespawn > 0u &&
					CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE ==
						arenaRoom->m_KoukuSaydonPatternAudition.ePhase;
				const bool removedActive = runningBeforeDespawn &&
					arenaRoom->Despawn_KoukuSaydonArenaDebugEntities() &&
					CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
						arenaRoom->m_KoukuSaydonPatternAudition.ePhase &&
					nullptr == arenaRoom->Find_KoukuSaydonArenaBoss(
						secondRequest.Scope.strBossPlacementId, secondRequest.Scope.strBossArchetypeId) &&
					std::any_of(lifecycle.begin(), lifecycle.end(),
						[secondKoukuId, stageBeforeDespawn](const auto& entry)
						{
							return secondKoukuId == entry.Message.iBossNetEntityId &&
								stageBeforeDespawn == entry.Message.iStageIndex &&
								2u == entry.Message.iRequestSequence &&
								KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED == entry.Message.eState;
						});
				tests.Require(removedActive,
					"Abort the active raised-boss Play All occurrence when Debug gate despawn removes its owner");
#endif

				const bool reverted = built && idle &&
					arenaRoom->Despawn_KoukuSaydonArenaDebugEntities() &&
					arenaRoom->m_WorldEntities.end() == std::find_if(
						arenaRoom->m_WorldEntities.begin(),
						arenaRoom->m_WorldEntities.end(),
						[gateId](const SERVER_WORLD_ENTITY& candidate)
						{
							return candidate.iNetEntityId == gateId;
						}) &&
					nullptr != arenaRoom->Find_KoukuSaydonAuditionBoss();
				tests.Require(reverted,
					"Despawn only the Debug-activated gate boss and keep the enabled Gate 1 Kouku");
			}

#ifdef _DEBUG
			auto room = std::make_unique<CGameRoom>(
				WORLD_ID::KAKULSAYDON_ARENA);
			/* Every Product plays on the Saydon body, so the audition target is
			the Gate 1 Saydon raised from its disabled placement. */
			const auto raiseGateSaydon = [](CGameRoom& target) -> SERVER_WORLD_ENTITY*
			{
				const WORLD_BOOTSTRAP_PLACEMENT* placement =
					target.Find_Placement("boss.kakulsaydon.g1.saydon");
				SERVER_WORLD_ENTITY raised{};
				if (nullptr == placement ||
					!target.Build_WorldEntity(*placement, target.m_iNextNetEntityId, raised))
					return nullptr;
				++target.m_iNextNetEntityId;
				target.m_WorldEntities.push_back(raised);
				return target.Find_KoukuSaydonArenaBoss(
					"boss.kakulsaydon.g1.saydon", "BOSS_KAKULSAYDON_G1_SAYDON");
			};
			/* Use the published room, navigation and patterns through the same
			request -> fixed tick -> factory -> wire admission as Complete Play. */
			const auto exerciseProductionGaze = [&](const bool exhaustCloneIds)
			{
				auto gazeRoom = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
				SERVER_WORLD_ENTITY* raised = raiseGateSaydon(*gazeRoom);
				const bool ready = gazeRoom->Is_Ready() && nullptr != raised;
				if (!ready)
				{
					tests.Require(false, "Build the production Saydon gaze room and owner");
					return;
				}
				const NET_ENTITY_ID ownerId = raised->iNetEntityId;
				const float centerX = raised->fSpawnPositionX;
				const float centerZ = raised->fSpawnPositionZ;
				const float beforeY = raised->fPositionY;
				const float beforeYaw = raised->fYawDegrees;
				const std::size_t beforeCount = gazeRoom->m_WorldEntities.size();
				if (exhaustCloneIds)
					gazeRoom->m_iNextNetEntityId = (std::numeric_limits<NET_ENTITY_ID>::max)() - 1u;
				const NET_ENTITY_ID beforeNextId = gazeRoom->m_iNextNetEntityId;
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request{};
				request.iRequestSequence = 1u;
				request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				request.Scope.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
				request.Scope.strEncounterId = "ENCOUNTER_KAKULSAYDON_G1";
				request.Scope.strBossPlacementId = "boss.kakulsaydon.g1.saydon";
				request.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G1_SAYDON";
				request.Scope.ExpectedGameplayRevision = gazeRoom->m_GameplayCatalog.Get_ActiveRevision();
				request.Scope.iExpectedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(
					gazeRoom->m_GameplayCatalog.Active());
				request.strPatternId = "KAKULSAYDON_G1_PATTERN_2";
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result{};
				const bool queued = KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
					gazeRoom->Evaluate_KoukuSaydonPatternAudition(8199u, request, result);
				bool triggerFired = false;
				for (std::uint32_t tick = 0u; queued && !triggerFired && tick < 120u; ++tick)
				{
					gazeRoom->Update_WorldEntities(1.f / 30.f);
					++gazeRoom->m_iServerTick;
					const auto& cues = gazeRoom->m_KoukuSaydonPatternAudition.Members.front().LogicLedger.MechanicTriggers;
					triggerFired = std::any_of(cues.begin(), cues.end(),
						[](const auto& cue) { return cue.bStarted; });
				}
				const auto findOwner = [&]() -> SERVER_WORLD_ENTITY*
				{
					return gazeRoom->Find_KoukuSaydonArenaBoss(
						"boss.kakulsaydon.g1.saydon", "BOSS_KAKULSAYDON_G1_SAYDON");
				};
				SERVER_WORLD_ENTITY* owner = findOwner();
				if (exhaustCloneIds)
				{
					/* Two valid children stage before the third would get the zero
					ID. No actor or ID counter may commit from that partial set. */
					tests.Require(queued && triggerFired && nullptr != owner &&
						gazeRoom->m_WorldEntities.size() == beforeCount &&
						gazeRoom->m_iNextNetEntityId == beforeNextId &&
						owner->fPositionX == centerX && owner->fPositionY == beforeY &&
						owner->fPositionZ == centerZ && owner->fYawDegrees == beforeYaw &&
						gazeRoom->m_PendingKoukuMechanicTriggers.empty() &&
						gazeRoom->Get_Status().find("clone admission failed") != std::string::npos,
						"Preserve the production gaze owner and IDs when the third staged clone cannot be admitted");
					return;
				}
				bool admitted = queued && triggerFired && nullptr != owner &&
					gazeRoom->m_WorldEntities.size() == beforeCount + 3u &&
					gazeRoom->m_iNextNetEntityId == beforeNextId + 3u;
				if (!admitted)
				{
					std::cerr << "Production gaze admission: " << gazeRoom->Get_Status() << '\n';
					tests.Require(false, "Commit three production gaze clones at the teleport trigger tick");
					return;
				}
				const float dx = owner->fPositionX - centerX;
				const float dz = owner->fPositionZ - centerZ;
				const float radius = std::sqrt(dx * dx + dz * dz);
				const auto looksAtCenter = [centerX, centerZ](const SERVER_WORLD_ENTITY& entity)
				{
					const float x = centerX - entity.fPositionX;
					const float z = centerZ - entity.fPositionZ;
					const float distance = std::sqrt(x * x + z * z);
					const float yaw = (entity.fYawDegrees + 90.f) * 0.017453292519943295f;
					return distance > 0.f && (std::sin(yaw) * x + std::cos(yaw) * z) / distance > 0.9999f;
				};
				std::vector<std::uint8_t> ownerPayload;
				S2C_WORLD_ENTITY_SPAWNED ownerMessage{};
				const bool ownerWritten = gazeRoom->Build_WorldEntitySpawnedPayload(*owner, ownerPayload);
				CPacketReader ownerReader{ ownerPayload };
				admitted = admitted && ownerWritten && Read_Message(ownerReader, ownerMessage) &&
					std::abs(owner->fPositionX + 6.36f) < 0.001f &&
					std::abs(owner->fPositionY - 1.3f) < 0.001f &&
					std::abs(owner->fPositionZ - 937.92f) < 0.001f &&
					gazeRoom->m_ServerNavigation.Is_PointWalkableExact(owner->fPositionX, owner->fPositionZ) &&
					looksAtCenter(*owner);
				std::size_t cloneCount = 0u;
				for (const auto& clone : gazeRoom->m_WorldEntities)
				{
					if (!clone.bKoukuGazeClone)
						continue;
					++cloneCount;
					std::vector<std::uint8_t> payload;
					S2C_WORLD_ENTITY_SPAWNED message{};
					const bool written = gazeRoom->Build_WorldEntitySpawnedPayload(clone, payload);
					CPacketReader reader{ payload };
					const float cloneX = clone.fPositionX - centerX;
					const float cloneZ = clone.fPositionZ - centerZ;
					admitted = admitted && written && Read_Message(reader, message) &&
						Is_Valid_WorldEntitySpawnOwner(message, &ownerMessage) &&
						clone.iOwnerBossNetEntityId == ownerId &&
						clone.iKoukuCloneOwnerSequence == owner->iPatternSequence &&
						clone.strPatternId == "KAKULSAYDON_G1_PATTERN_5" &&
						clone.iActionStartTick == gazeRoom->m_iServerTick &&
						clone.iPatternStartTick == gazeRoom->m_iServerTick &&
						clone.PinnedDefinitionRevision == owner->PinnedDefinitionRevision &&
						gazeRoom->m_ServerNavigation.Is_PointWalkableExact(clone.fPositionX, clone.fPositionZ) &&
						std::abs(std::sqrt(cloneX * cloneX + cloneZ * cloneZ) - radius) < 0.002f &&
						std::abs(clone.fPositionY - owner->fPositionY) < 0.001f && looksAtCenter(clone);
				}
				tests.Require(admitted && 3u == cloneCount,
					"Commit production Pattern 2 into three Pattern 5 clones on the real navgrid with inward yaw and valid spawn owners");
				gazeRoom->Update_WorldEntities(1.f / 30.f);
				++gazeRoom->m_iServerTick;
				tests.Require(gazeRoom->m_WorldEntities.size() == beforeCount + 3u &&
					gazeRoom->m_iNextNetEntityId == beforeNextId + 3u,
					"Keep the production gaze trigger one-shot across the next room tick");
				owner = findOwner();
				if (nullptr != owner)
					gazeRoom->m_KoukuSaydonBrain.Complete_Pattern(*owner, gazeRoom->m_iServerTick);
				gazeRoom->Update_WorldEntities(1.f / 30.f);
				++gazeRoom->m_iServerTick;
				tests.Require(nullptr != findOwner() && 0u == findOwner()->iPatternStartTick &&
					gazeRoom->m_WorldEntities.size() == beforeCount &&
					std::none_of(gazeRoom->m_WorldEntities.begin(), gazeRoom->m_WorldEntities.end(),
						[](const auto& entity) { return entity.bKoukuGazeClone; }),
					"Remove all three production gaze clones on the owning pattern termination edge");
			};
			exerciseProductionGaze(false);
			exerciseProductionGaze(true);
			for (const char* patternId : { "KAKULSAYDON_G1_PATTERN_6", "KAKULSAYDON_G1_PATTERN_7" })
			{
				auto centeredRoom = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
				auto* centeredBoss = raiseGateSaydon(*centeredRoom);
				if (nullptr == centeredBoss)
				{
					tests.Require(false, "Raise the production Dance/Roulette owner");
					continue;
				}
				SERVER_PLAYER cardPlayer{};
				cardPlayer.iPlayerId = 8197u;
				cardPlayer.iNetEntityId = centeredRoom->m_iNextNetEntityId++;
				cardPlayer.iCurrentHp = cardPlayer.iMaximumHp = 100u;
				centeredRoom->m_Players.emplace(cardPlayer.iPlayerId, cardPlayer);
				centeredRoom->Apply_KoukuGateEntryCard(centeredRoom->m_Players.at(cardPlayer.iPlayerId), *centeredBoss);
				const auto assignedSuit = centeredRoom->m_Players.at(cardPlayer.iPlayerId).eMechanicCardSymbol;
				const auto assignedColor = centeredRoom->m_Players.at(cardPlayer.iPlayerId).eMechanicCardColor;
				const float centerX = centeredBoss->fSpawnPositionX;
				const float centerY = centeredBoss->fSpawnPositionY;
				const float centerZ = centeredBoss->fSpawnPositionZ;
				centeredBoss->fPositionX += 5.f;
				centeredBoss->fPositionZ -= 4.f;
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request{};
				request.iRequestSequence = 1u;
				request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				request.Scope.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
				request.Scope.strEncounterId = "ENCOUNTER_KAKULSAYDON_G1";
				request.Scope.strBossPlacementId = "boss.kakulsaydon.g1.saydon";
				request.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G1_SAYDON";
				request.Scope.ExpectedGameplayRevision = centeredRoom->m_GameplayCatalog.Get_ActiveRevision();
				request.Scope.iExpectedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(centeredRoom->m_GameplayCatalog.Active());
				request.strPatternId = patternId;
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result{};
				const bool queued = KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
					centeredRoom->Evaluate_KoukuSaydonPatternAudition(8197u, request, result);
				centeredRoom->Update_WorldEntities(1.f / 30.f);
				++centeredRoom->m_iServerTick;
				centeredBoss = centeredRoom->Find_KoukuSaydonArenaBoss(request.Scope.strBossPlacementId, request.Scope.strBossArchetypeId);
				tests.Require(queued && nullptr != centeredBoss && centeredBoss->fPositionX == centerX &&
					centeredBoss->fPositionY == centerY && centeredBoss->fPositionZ == centerZ &&
					centeredBoss->iPatternStartTick == centeredRoom->m_iServerTick &&
					MECHANIC_CARD_SYMBOL::NONE != assignedSuit && MECHANIC_CARD_COLOR::NONE != assignedColor,
					"Reset the production Dance or Roulette boss to spawn and publish its fixed pattern start clock with an encounter card");
				SERVER_WORLD_ENTITY otherGate{};
				otherGate.strArchetypeId = "BOSS_KAKULSAYDON_G3_SAYDON";
				centeredRoom->Apply_KoukuGateEntryCard(centeredRoom->m_Players.at(cardPlayer.iPlayerId), otherGate);
				centeredRoom->Despawn_KoukuSaydonArenaDebugEntities();
				centeredRoom->Update_KoukuPlayerModes(2u);
				const auto& cleared = centeredRoom->m_Players.at(cardPlayer.iPlayerId);
				tests.Require(MECHANIC_CARD_SYMBOL::NONE == cleared.eMechanicCardSymbol &&
					MECHANIC_CARD_COLOR::NONE == cleared.eMechanicCardColor,
					"Remove the assigned suit and color when the Gate 1 Saydon owner leaves");
			}

			SERVER_WORLD_ENTITY* liveBoss = raiseGateSaydon(*room);
			C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST selected{};
			selected.iRequestSequence = 2u;
			selected.eOperation =
				KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
			selected.Scope.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
			selected.Scope.strEncounterId = "ENCOUNTER_KAKULSAYDON_G1";
			selected.Scope.strBossPlacementId =
				"boss.kakulsaydon.g1.saydon";
			selected.Scope.strBossArchetypeId =
				"BOSS_KAKULSAYDON_G1_SAYDON";
			selected.Scope.ExpectedGameplayRevision =
				room->m_GameplayCatalog.Get_ActiveRevision();
			selected.Scope.iExpectedSourceRevision =
				CKoukuSaydonBrain::Resolve_ProductSourceRevision(
					room->m_GameplayCatalog.Active());
			selected.strPatternId = firstProductId;

			S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT rejected{};
			auto wrongScope = selected;
			wrongScope.iRequestSequence = 1u;
			wrongScope.Scope.strBossPlacementId = "boss.valtan.center";
			const bool exactScopeRejected = room->Is_Ready() && nullptr != liveBoss &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_SCOPE_MISMATCH ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, wrongScope, rejected) &&
				CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
					room->m_KoukuSaydonPatternAudition.ePhase &&
				room->m_PendingKoukuSaydonPatternAuditionLifecycle.empty();

			auto wrongGameplayRevision = selected;
			wrongGameplayRevision.iRequestSequence = 1u;
			wrongGameplayRevision.Scope.ExpectedGameplayRevision.Bytes.front() ^=
				0xffu;
			const bool gameplayRevisionRejected = exactScopeRejected &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_REVISION_MISMATCH ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, wrongGameplayRevision, rejected) &&
				rejected.eResult == KOUKUSAYDON_PATTERN_AUDITION_RESULT::
					REJECTED_REVISION_MISMATCH &&
				rejected.Scope.ExpectedGameplayRevision ==
					wrongGameplayRevision.Scope.ExpectedGameplayRevision &&
				rejected.PinnedGameplayRevision ==
					selected.Scope.ExpectedGameplayRevision &&
				rejected.iPinnedSourceRevision ==
					selected.Scope.iExpectedSourceRevision &&
				0u == rejected.iRoomAuditionEpoch &&
				INVALID_NET_ENTITY_ID == rejected.iBossNetEntityId &&
				rejected.strResolvedPatternId.empty() &&
				CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
					room->m_KoukuSaydonPatternAudition.ePhase &&
				room->m_PendingKoukuSaydonPatternAuditionLifecycle.empty();

			auto wrongSourceRevision = selected;
			wrongSourceRevision.iRequestSequence = 1u;
			wrongSourceRevision.Scope.iExpectedSourceRevision =
				(std::numeric_limits<std::uint32_t>::max)() ==
					selected.Scope.iExpectedSourceRevision ?
					selected.Scope.iExpectedSourceRevision - 1u :
					selected.Scope.iExpectedSourceRevision + 1u;
			const bool sourceRevisionRejected = gameplayRevisionRejected &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::
					REJECTED_SOURCE_REVISION_MISMATCH ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, wrongSourceRevision, rejected) &&
				rejected.eResult == KOUKUSAYDON_PATTERN_AUDITION_RESULT::
					REJECTED_SOURCE_REVISION_MISMATCH &&
				rejected.Scope.iExpectedSourceRevision ==
					wrongSourceRevision.Scope.iExpectedSourceRevision &&
				rejected.PinnedGameplayRevision ==
					selected.Scope.ExpectedGameplayRevision &&
				rejected.iPinnedSourceRevision ==
					selected.Scope.iExpectedSourceRevision &&
				0u == rejected.iRoomAuditionEpoch &&
				INVALID_NET_ENTITY_ID == rejected.iBossNetEntityId &&
				rejected.strResolvedPatternId.empty() &&
				CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
					room->m_KoukuSaydonPatternAudition.ePhase &&
				room->m_PendingKoukuSaydonPatternAuditionLifecycle.empty();

			S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT selectedResult{};
			const bool selectedQueued = sourceRevisionRejected &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, selected, selectedResult) &&
				selectedResult.Scope.eWorldId == selected.Scope.eWorldId &&
				selectedResult.Scope.strEncounterId ==
					selected.Scope.strEncounterId &&
				selectedResult.Scope.strBossPlacementId ==
					selected.Scope.strBossPlacementId &&
				selectedResult.Scope.strBossArchetypeId ==
					selected.Scope.strBossArchetypeId &&
				selectedResult.Scope.ExpectedGameplayRevision ==
					selected.Scope.ExpectedGameplayRevision &&
				selectedResult.Scope.iExpectedSourceRevision ==
					selected.Scope.iExpectedSourceRevision &&
				selectedResult.PinnedGameplayRevision ==
					selected.Scope.ExpectedGameplayRevision &&
				selectedResult.iPinnedSourceRevision ==
					selected.Scope.iExpectedSourceRevision &&
				firstProductId ==
					selectedResult.strResolvedPatternId &&
				1u == room->m_PendingKoukuSaydonPatternAuditionLifecycle.size() &&
				KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING ==
					room->m_PendingKoukuSaydonPatternAuditionLifecycle.front().
						Message.eState;

			bool runtimeAdvanced = selectedQueued;
			for (std::uint32_t tick = 1u; runtimeAdvanced && tick < 700u; ++tick)
			{
				runtimeAdvanced = room->Update_KoukuSaydonBoss(*liveBoss, tick);
				if (CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
					room->m_KoukuSaydonPatternAudition.ePhase)
				{
					break;
				}
			}
			std::vector<std::uint32_t> liveStageIndices;
			std::size_t patternCompletedCount = 0u;
			std::size_t completedCount = 0u;
			bool exactLifecycleIdentity = selectedQueued;
			std::uint32_t occurrenceSequence = 0u;
			for (const auto& targeted :
				room->m_PendingKoukuSaydonPatternAuditionLifecycle)
			{
				const auto& lifecycle = targeted.Message;
				exactLifecycleIdentity = exactLifecycleIdentity &&
					8101u == targeted.iSessionId &&
					selected.iRequestSequence == lifecycle.iRequestSequence &&
					selected.eOperation == lifecycle.eOperation &&
					selected.Scope.eWorldId == lifecycle.Scope.eWorldId &&
					selected.Scope.strEncounterId ==
						lifecycle.Scope.strEncounterId &&
					selected.Scope.strBossPlacementId ==
						lifecycle.Scope.strBossPlacementId &&
					selected.Scope.strBossArchetypeId ==
						lifecycle.Scope.strBossArchetypeId &&
					selected.Scope.ExpectedGameplayRevision ==
						lifecycle.Scope.ExpectedGameplayRevision &&
					selected.Scope.ExpectedGameplayRevision ==
						lifecycle.PinnedGameplayRevision &&
					selected.Scope.iExpectedSourceRevision ==
						lifecycle.Scope.iExpectedSourceRevision &&
					selected.Scope.iExpectedSourceRevision ==
						lifecycle.iPinnedSourceRevision &&
					firstProductId == lifecycle.strPatternId;
				if (KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE ==
					lifecycle.eState)
				{
					liveStageIndices.push_back(lifecycle.iStageIndex);
					if (0u == occurrenceSequence)
						occurrenceSequence = lifecycle.iPatternSequence;
					exactLifecycleIdentity = exactLifecycleIdentity &&
						occurrenceSequence == lifecycle.iPatternSequence;
				}
				else if (KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::
					PATTERN_COMPLETED == lifecycle.eState)
				{
					++patternCompletedCount;
					exactLifecycleIdentity = exactLifecycleIdentity &&
						occurrenceSequence == lifecycle.iPatternSequence &&
						firstProductLastStage == lifecycle.iStageIndex;
				}
				else if (KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED ==
					lifecycle.eState)
				{
					++completedCount;
					exactLifecycleIdentity = exactLifecycleIdentity &&
						occurrenceSequence == lifecycle.iPatternSequence &&
						firstProductLastStage == lifecycle.iStageIndex;
				}
			}
			std::vector<std::uint32_t> expectedLiveStages;
			for (std::uint32_t stage = 0u; stage < firstProductStageCount; ++stage)
				expectedLiveStages.push_back(stage);
			tests.Require(runtimeAdvanced && exactLifecycleIdentity &&
				0u != occurrenceSequence &&
				expectedLiveStages == liveStageIndices &&
				1u == patternCompletedCount && 1u == completedCount &&
				CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
					room->m_KoukuSaydonPatternAudition.ePhase &&
				liveBoss->strPatternId.empty() &&
				SERVER_ENTITY_ACTION::IDLE == liveBoss->eAction,
				"Publish exact KoukuSaydon Pending Active stage Live and terminal lifecycle edges");

			S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT duplicateResult{};
			const std::size_t lifecycleCountBeforeStaleRequest =
				room->m_PendingKoukuSaydonPatternAuditionLifecycle.size();
			auto staleSourceIdentity = selected;
			staleSourceIdentity.Scope.iExpectedSourceRevision =
				wrongSourceRevision.Scope.iExpectedSourceRevision;
			S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT staleSourceResult{};
			const bool staleSourceRejected =
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_STALE_REQUEST ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, staleSourceIdentity, staleSourceResult) &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_STALE_REQUEST ==
					staleSourceResult.eResult &&
				lifecycleCountBeforeStaleRequest ==
					room->m_PendingKoukuSaydonPatternAuditionLifecycle.size();
			const bool duplicateReconciled =
				staleSourceRejected &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::DUPLICATE_IGNORED ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, selected, duplicateResult) &&
				duplicateResult.Scope.ExpectedGameplayRevision ==
					selected.Scope.ExpectedGameplayRevision &&
				duplicateResult.Scope.iExpectedSourceRevision ==
					selected.Scope.iExpectedSourceRevision &&
				duplicateResult.PinnedGameplayRevision ==
					selected.Scope.ExpectedGameplayRevision &&
				duplicateResult.iPinnedSourceRevision ==
					selected.Scope.iExpectedSourceRevision &&
				occurrenceSequence == duplicateResult.iPatternSequence &&
				firstProductLastStage == duplicateResult.iStageIndex;

			auto playAll = selected;
			playAll.iRequestSequence = 3u;
			playAll.eOperation =
				KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_ALL;
			playAll.strPatternId.clear();
			S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT playAllResult{};
			const bool playAllQueued = duplicateReconciled &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, playAll, playAllResult) &&
				sequence->PatternIds ==
					room->m_KoukuSaydonPatternAudition.Members.front().PatternIds &&
				sequence->TransitionPursuitTicks ==
					room->m_KoukuSaydonPatternAudition.Members.front().TransitionTicks &&
				playAllResult.Scope.ExpectedGameplayRevision ==
					playAll.Scope.ExpectedGameplayRevision &&
				playAllResult.Scope.iExpectedSourceRevision ==
					playAll.Scope.iExpectedSourceRevision &&
				playAllResult.PinnedGameplayRevision ==
					playAll.Scope.ExpectedGameplayRevision &&
				playAllResult.iPinnedSourceRevision ==
					playAll.Scope.iExpectedSourceRevision &&
				sequence->PatternIds.front() ==
					playAllResult.strResolvedPatternId;
			tests.Require(exactScopeRejected && gameplayRevisionRejected &&
				sourceRevisionRejected && staleSourceRejected &&
				selectedQueued &&
				duplicateReconciled && playAllQueued,
				"Scope Play Selected retries and Play All consume only the Server Product sequence");

			/* Evaluate stages PENDING locally, while Handle sends the result frame
			   immediately. The room can only flush PENDING/ACTIVE after the fixed
			   world update, so the real reliable session FIFO must expose the verdict
			   before either lifecycle edge. */
			constexpr SESSION_ID FIFO_SESSION = 8102u;
			auto fifoRoom = std::make_unique<CGameRoom>(
				WORLD_ID::KAKULSAYDON_ARENA);
			auto fifoSession = std::make_shared<CClientSession>(
				FIFO_SESSION, INVALID_SOCKET,
				CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
			fifoSession->m_isSendRunning.store(true);
			fifoRoom->m_Sessions.emplace(FIFO_SESSION, fifoSession);
			(void)raiseGateSaydon(*fifoRoom);
			auto fifoRequest = selected;
			fifoRequest.iRequestSequence = 1u;
			fifoRequest.Scope.ExpectedGameplayRevision =
				fifoRoom->m_GameplayCatalog.Get_ActiveRevision();
			fifoRequest.Scope.iExpectedSourceRevision =
				CKoukuSaydonBrain::Resolve_ProductSourceRevision(
					fifoRoom->m_GameplayCatalog.Active());
			fifoRoom->Handle_KoukuSaydonPatternAudition(
				FIFO_SESSION, fifoRequest);
			fifoRoom->Tick(1.f / 30.f);

			struct KOUKU_FIFO_EDGE final
			{
				bool isResult = false;
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT Result{};
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE Lifecycle{};
			};
			std::vector<KOUKU_FIFO_EDGE> fifoEdges;
			for (const auto& outbound : fifoSession->m_OutboundFrames)
			{
				if (PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT !=
						outbound.ePacketType &&
					PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE !=
						outbound.ePacketType)
				{
					continue;
				}
				PACKET_HEADER header{};
				if (!Read_Packet_Header(outbound.Bytes, header) ||
					outbound.Bytes.size() < PACKET_HEADER_BYTES)
				{
					continue;
				}
				CPacketReader reader{ std::span<const std::uint8_t>(
					outbound.Bytes.data() + PACKET_HEADER_BYTES,
					outbound.Bytes.size() - PACKET_HEADER_BYTES) };
				KOUKU_FIFO_EDGE edge{};
				if (PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT ==
					header.ePacketType)
				{
					edge.isResult = Read_Message(reader, edge.Result) &&
						0u == reader.Get_RemainingSize();
					if (edge.isResult)
						fifoEdges.push_back(std::move(edge));
				}
				else if (PACKET_TYPE::
					S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE ==
						header.ePacketType &&
					Read_Message(reader, edge.Lifecycle) &&
					0u == reader.Get_RemainingSize())
				{
					fifoEdges.push_back(std::move(edge));
				}
			}
			const bool resultBeforeLifecycle = fifoRoom->Is_Ready() &&
				3u == fifoEdges.size() && fifoEdges[0u].isResult &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
					fifoEdges[0u].Result.eResult &&
				!fifoEdges[1u].isResult &&
				KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING ==
					fifoEdges[1u].Lifecycle.eState &&
				!fifoEdges[2u].isResult &&
				KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE ==
					fifoEdges[2u].Lifecycle.eState &&
				0u == fifoEdges[2u].Lifecycle.iStageIndex &&
				firstProductId ==
					fifoEdges[2u].Lifecycle.strPatternId &&
				fifoRequest.iRequestSequence ==
					fifoEdges[0u].Result.iRequestSequence &&
				fifoEdges[0u].Result.iRequestSequence ==
					fifoEdges[1u].Lifecycle.iRequestSequence &&
				fifoEdges[0u].Result.iRequestSequence ==
					fifoEdges[2u].Lifecycle.iRequestSequence;
			const bool fifoExactRevisionEcho = resultBeforeLifecycle &&
				fifoRequest.Scope.ExpectedGameplayRevision ==
					fifoEdges[0u].Result.Scope.ExpectedGameplayRevision &&
				fifoRequest.Scope.ExpectedGameplayRevision ==
					fifoEdges[0u].Result.PinnedGameplayRevision &&
				fifoRequest.Scope.iExpectedSourceRevision ==
					fifoEdges[0u].Result.Scope.iExpectedSourceRevision &&
				fifoRequest.Scope.iExpectedSourceRevision ==
					fifoEdges[0u].Result.iPinnedSourceRevision &&
				fifoRequest.Scope.ExpectedGameplayRevision ==
					fifoEdges[1u].Lifecycle.Scope.ExpectedGameplayRevision &&
				fifoRequest.Scope.ExpectedGameplayRevision ==
					fifoEdges[1u].Lifecycle.PinnedGameplayRevision &&
				fifoRequest.Scope.iExpectedSourceRevision ==
					fifoEdges[1u].Lifecycle.Scope.iExpectedSourceRevision &&
				fifoRequest.Scope.iExpectedSourceRevision ==
					fifoEdges[1u].Lifecycle.iPinnedSourceRevision &&
				fifoRequest.Scope.ExpectedGameplayRevision ==
					fifoEdges[2u].Lifecycle.Scope.ExpectedGameplayRevision &&
				fifoRequest.Scope.ExpectedGameplayRevision ==
					fifoEdges[2u].Lifecycle.PinnedGameplayRevision &&
				fifoRequest.Scope.iExpectedSourceRevision ==
					fifoEdges[2u].Lifecycle.Scope.iExpectedSourceRevision &&
				fifoRequest.Scope.iExpectedSourceRevision ==
					fifoEdges[2u].Lifecycle.iPinnedSourceRevision;
			tests.Require(resultBeforeLifecycle,
				"Send KoukuSaydon QUEUED result before PENDING and first Active Live edge");
			tests.Require(fifoExactRevisionEcho,
				"Echo exact KoukuSaydon gameplay and Product source pins on every FIFO edge");
			fifoSession->Request_Close();
#endif
		}
	}
}

