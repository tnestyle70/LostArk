#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "ClientSession.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "KoukuSaydonBrain.h"
#include "Gameplay/WorldCollisionContract.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include "ServerApp.h"
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
#include <random>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;

void LostArk::Server::CServerGameplayContractRunner::Run_KoukuMarioEntryContact(TESTS& tests)
{
#ifdef _DEBUG
    BOSS_PATTERN_DEFINITION root{};
    BOSS_PATTERN_LOGIC_WINDOW entry{};
    entry.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA;
    entry.iStartMs = 2998u;
    BOSS_PATTERN_LOGIC_RESULT result{};
    result.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER;
    entry.OnSuccess.push_back(result);
    BOSS_LOGIC_REGION region{};
    region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
    region.fHalfX = region.fHalfY = region.fHalfZ = 1.f;
    entry.CardRegions.push_back(region);
    root.LogicWindows.push_back(entry);
    SERVER_WORLD_ENTITY anchor{};
    anchor.fPositionX = -.07f; anchor.fPositionY = 1.32f; anchor.fPositionZ = 942.33f;
    const auto contactTick = CKoukuSaydonLogicRuntime::Ticks_FromMs(entry.iStartMs);
    CServerCollisionSystem collision;
    std::string status;
    tests.Require(collision.Initialize({}, status), "Initialize Mario contact movement collision");
    collision.Set_BlockingBodies({{anchor.fPositionX, anchor.fPositionZ, 1.f,
        anchor.fPositionY + 1.f, 1.f, 990u}});
    const std::array interruptible{PLAYER_ACTION_STATE::NONE, PLAYER_ACTION_STATE::SKILL,
        PLAYER_ACTION_STATE::INTERACTION, PLAYER_ACTION_STATE::KNOCKDOWN,
        PLAYER_ACTION_STATE::FEAR, PLAYER_ACTION_STATE::ESTHER_CAST};
    for (const float yaw : {0.f, 43.f, 90.f}) {
        anchor.fYawDegrees = yaw;
        const float radians = yaw * .017453292519943295f;
        const float rightX = std::cos(radians), rightZ = -std::sin(radians);
        SERVER_PLAYER player{};
        player.iPlayerId = 991u; player.iNetEntityId = 992u; player.iCurrentHp = 100u; player.isCombatReady = true;
        player.fPositionX = anchor.fPositionX + rightX * 1.5f;
        player.fPositionY = anchor.fPositionY; player.fPositionZ = anchor.fPositionZ + rightZ * 1.5f;
        const float proposedX = anchor.fPositionX + rightX * 1.4f;
        const float proposedZ = anchor.fPositionZ + rightZ * 1.4f;
        float x = 0.f, y = 0.f, z = 0.f; bool blocked = false;
        const bool moved = collision.Resolve_PlayerMove(player, proposedX, anchor.fPositionY,
            proposedZ, x, y, z, blocked);
        player.fPositionX = x; player.fPositionY = y; player.fPositionZ = z;
        const bool constrained = std::hypot(x - proposedX, z - proposedZ) > 0.001f;
        const bool beforeEntry = CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(root, anchor, player, contactTick - 1u);
        const bool touchingEntry = CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(root, anchor, player, contactTick);
        const auto oldPrecision = std::cout.precision(9);
        std::cout << "[MarioContactProbe] yaw=" << yaw << " proposed=(" << proposedX << ',' << proposedZ
            << ") actual=(" << x << ',' << z << ") moved=" << moved << " blocked=" << blocked
            << " constrained=" << constrained << " before=" << beforeEntry << " touching=" << touchingEntry << '\n';
        std::cout.precision(oldPrecision);
        // A successful tangent slide clears blocked; the clipped destination
        // proves the body's collision was consumed during this short tick.
        tests.Require(moved && constrained && !beforeEntry && touchingEntry,
            "A real boss-body stop point touches the Mario entry on its first active tick, including movement skin");
        auto outside = player; outside.fPositionX += rightX * .01f; outside.fPositionZ += rightZ * .01f;
        tests.Require(!CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(root, anchor, outside, contactTick),
            "Mario body contact does not admit a player still outside the contact margin");
        for (const auto action : interruptible) {
            player.eAction = action;
            tests.Require(CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(root, anchor, player, contactTick),
                "Mario body contact does not wait for ordinary actions, knockdown or fear to finish");
        }
        for (int invalid = 0; invalid < 8; ++invalid) {
            auto rejected = player;
            switch (invalid) {
            case 0: rejected.iCurrentHp = 0u; break;
            case 1: rejected.eAction = PLAYER_ACTION_STATE::FALLING; break;
            case 2: rejected.eAction = PLAYER_ACTION_STATE::GRABBED; break;
            case 3: rejected.bPatternBound = true; break;
            case 4: rejected.iAttachmentOwnerNetEntityId = 7u; break;
            case 5: rejected.bArenaEjectionActive = true; break;
            case 6: rejected.TriggerMove.isActive = true; break;
            case 7: rejected.iMarioStage = 1u; break;
            }
            tests.Require(!CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(root, anchor, rejected, contactTick),
                "Mario entry preserves death, fall, attachment, binding and existing-transfer ownership");
        }
        // A non-chain entry honours the window end; a chain root (honorWindowEnd false) stays open past it.
        {
            auto ended = root; ended.LogicWindows.front().iDurationMs = 1000u;
            const auto afterEnd = CKoukuSaydonLogicRuntime::Ticks_FromMs(entry.iStartMs + 1000u);
            tests.Require(CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(ended, anchor, player, afterEnd) &&
                !CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(ended, anchor, player, afterEnd, true) &&
                CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(ended, anchor, player, afterEnd - 1u, true),
                "Only a non-chain Mario entry closes with its ENTER_AREA window end");
        }
    }
    {
        BOSS_PATTERN_DEFINITION chainOnly{}, none{};
        BOSS_PATTERN_LOGIC_WINDOW chain{}; chain.eKind = BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT;
        chainOnly.LogicWindows.push_back(chain);
        tests.Require(CKoukuSaydonLogicRuntime::Find_MarioEntryResult(root) == &root.LogicWindows.front().OnSuccess.front() &&
            CKoukuSaydonLogicRuntime::Has_MarioEntry(root) && CKoukuSaydonLogicRuntime::Has_MarioEntry(chainOnly) &&
            !CKoukuSaydonLogicRuntime::Find_MarioEntryResult(chainOnly) && !CKoukuSaydonLogicRuntime::Has_MarioEntry(none),
            "A Mario entry pattern is a chain root or an ENTER_AREA window whose sole Success is MARIO_ENTER with regions");
    }
    const auto seedOwnedObject = [&](CGameRoom& room, const SERVER_PLAYER& player) {
        auto transaction = room.m_CombatObjectRuntime.Begin_Transaction();
        SERVER_COMBAT_OBJECT object{};
        object.iCombatObjectId = 993u; object.iSourceNetEntityId = player.iNetEntityId;
        object.iSourcePlayerId = player.iPlayerId; object.fRemainingMilliseconds = 1000.f;
        transaction.Objects.push_back(object);
        return room.m_CombatObjectRuntime.Commit(std::move(transaction));
    };
    for (std::uint8_t stage = 1u; stage <= 4u; ++stage) {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Load actual Mario stage navigation for contact admission");
        for (const auto action : interruptible) {
            SERVER_PLAYER player{};
            player.iPlayerId = 994u; player.iNetEntityId = 995u; player.iCurrentHp = player.iMaximumHp = 100u;
            player.isCombatReady = true; player.eAction = action; player.iCurrentSkillId = 34010u;
            player.iActionStartTick = 1u; player.fActionElapsedSeconds = .1f; player.iComboStage = 2u;
            player.Projectiles.emplace_back(); player.hasMoveGoal = true; player.fKnockbackRemainingSeconds = 1.f;
            player.iKnockdownEndTick = player.iFearEndTick = 60u; player.strFearPresentationId = "contact-test";
            player.CooldownEndTickBySkillId.emplace(34010u, 600u);
            tests.Require(seedOwnedObject(*room, player), "Stage the entry action's delayed combat object");
            const bool entered = room->Enter_MarioFromPattern(player, stage);
            tests.Require(entered && player.iMarioStage == stage && player.eAction == PLAYER_ACTION_STATE::NONE &&
                player.iCurrentSkillId == INVALID_SKILL_ID && player.Projectiles.empty() && !player.hasMoveGoal &&
                player.fKnockbackRemainingSeconds == 0.f && !player.iKnockdownEndTick && !player.iFearEndTick &&
                player.strFearPresentationId.empty() && player.isCombatReady && player.iCurrentHp == 100u &&
                player.CooldownEndTickBySkillId.at(34010u) == 600u && room->m_CombatObjectRuntime.Get_LiveObjects().empty(),
                "All four Mario stages interrupt the contacting action and cancel its objects only after admission");
            tests.Require(!room->Enter_MarioFromPattern(player, stage), "Committed Mario entry remains one-shot");
        }
    }
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        SERVER_PLAYER player{};
        player.iPlayerId = 996u; player.iNetEntityId = 997u; player.iCurrentHp = 100u; player.isCombatReady = true;
        player.eAction = PLAYER_ACTION_STATE::SKILL; player.iCurrentSkillId = 34010u;
        player.Projectiles.emplace_back(); player.fPositionX = 5.f; player.fPositionZ = 942.f;
        tests.Require(seedOwnedObject(*room, player), "Stage a live action before a rejected Mario layout");
        auto& groups = const_cast<std::vector<SPAWN_GROUP_DEFINITION>&>(room->m_SpawnGroupBootstrap.Get_Groups());
        const auto group = std::find_if(groups.begin(), groups.end(), [](const auto& row) { return row.strSpawnGroupId == "spawn.mario1.source"; });
        if (group == groups.end() || group->Waves.empty() || group->Waves.front().Entries.empty()) {
            tests.Require(false, "Resolve the actual Mario source anchor for failed-admission preservation");
        } else group->Waves.front().Entries.front().strAnchorId = "missing.mario.contact.test.anchor";
        tests.Require(!room->Enter_MarioFromPattern(player, 1u) && !player.iMarioStage &&
            player.eAction == PLAYER_ACTION_STATE::SKILL && player.iCurrentSkillId == 34010u &&
            player.Projectiles.size() == 1u && player.fPositionX == 5.f && player.fPositionZ == 942.f &&
            room->m_CombatObjectRuntime.Get_LiveObjects().size() == 1u,
            "A post-destination Mario layout failure preserves the live action, position and delayed combat object");
    }
    for (const auto& [id, stage] : std::array<std::pair<const char*, std::uint8_t>, 2u>{
        { {"Mario1_Trigger_1", 1u}, {"Mario4_Tigger_3", 4u} }}) {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        const auto* trigger = room->Find_Placement(id);
        if (!trigger) { tests.Require(false, "Resolve the exact authored Mario lane trigger ID"); continue; }
        SERVER_PLAYER player{};
        player.iPlayerId = 998u; player.iNetEntityId = 999u; player.iCurrentHp = 100u; player.isCombatReady = true;
        player.iMarioStage = stage; player.bMarioRailReady = true; player.strMarioRailArrivalId = "preserved";
        player.fPositionX = trigger->fPositionX; player.fPositionY = trigger->fPositionY; player.fPositionZ = trigger->fPositionZ;
        for (const auto action : {PLAYER_ACTION_STATE::SKILL, PLAYER_ACTION_STATE::INTERACTION, PLAYER_ACTION_STATE::TRIGGER_MOVE}) {
            auto contacting = player; contacting.eAction = action; contacting.iCurrentSkillId = 34010u;
            contacting.TriggerMove.isActive = action == PLAYER_ACTION_STATE::TRIGGER_MOVE;
            contacting.TriggerMove.strSourcePlacementId.clear(); // Debug Mario jump has no trigger owner.
            tests.Require(room->Begin_MarioTriggerMove(*trigger, contacting, 100u) == SERVER_TRIGGER_MOVE_ENTRY_RESULT::STARTED &&
                contacting.TriggerMove.isActive && contacting.iCurrentSkillId == INVALID_SKILL_ID &&
                contacting.iMarioStage == stage && contacting.bMarioRailReady && contacting.strMarioRailArrivalId == "preserved",
                "Actual Mario lane contacts interrupt skill, interaction and jump while preserving the current rail");
            contacting.TriggerMove.strSourcePlacementId = id;
            contacting.TriggerMove.fElapsedSeconds = .1f;
            tests.Require(room->Begin_MarioTriggerMove(*trigger, contacting, 101u) == SERVER_TRIGGER_MOVE_ENTRY_RESULT::STARTED &&
                contacting.TriggerMove.fElapsedSeconds == .1f, "The same active Mario trigger cannot restart its transfer");
        }
        CServerTriggerSystem triggers;
        triggers.Set_WorldId(WORLD_ID::KAKULSAYDON_ARENA);
        // Exercise automatic contact policy independently of the installed interaction flag.
        auto contactTrigger = *trigger;
        contactTrigger.requiresInteract = false;
        contactTrigger.isTriggerOnce = false;
        tests.Require(triggers.Initialize({contactTrigger}, status), "Load actual Mario geometry with repeatable automatic contact policy");
        player.bPatternBound = true;
        std::map<PLAYER_ID, SERVER_PLAYER> players{{player.iPlayerId, player}};
        std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
        std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
        const auto move = [&](const auto& box, auto& target, auto tick) { return room->Begin_MarioTriggerMove(box, target, tick); };
        // Mario crossings and exits are G boxes; the G press goes through the same room-owned admission.
        (void)triggers.Activate_Interact(player.iPlayerId, contactTrigger.strPlacementId, players, 102u, transfers, {}, move);
        tests.Require(!players.at(player.iPlayerId).TriggerMove.isActive, "An owned Mario G press preserves a temporary authority lock");
        players.at(player.iPlayerId).bPatternBound = false;
        (void)triggers.Activate_Interact(player.iPlayerId, contactTrigger.strPlacementId, players, 103u, transfers, {}, move);
        tests.Require(players.at(player.iPlayerId).TriggerMove.isActive,
            "A rejected Mario G press succeeds on the next press after unlock without leaving the box");
        auto unrelated = *trigger; unrelated.strPlacementId = "ordinary.movePlayer";
        tests.Require(room->Begin_MarioTriggerMove(unrelated, player, 104u) == SERVER_TRIGGER_MOVE_ENTRY_RESULT::USE_DEFAULT,
            "Mario action interruption does not change unrelated world movePlayer contracts");
    }
#endif
}

void LostArk::Server::CServerGameplayContractRunner::Run_KoukuProduct(TESTS& tests, CGameplayCatalog& catalog)
{
#ifdef _DEBUG
    // These use the published composition, real room and the same command
    // admission/terminal receipts consumed by Complete Play and its F1 test.
    enum class MARIO_SCENARIO { NO_ENTRY, RETURN_AFTER_CHAIN, RETURN_EARLY, PARTY, DEATH, DISCONNECT, LAST_TICK_ENTRY, NO_FOLLOWUP, CANCEL };
    std::uint32_t noEntryCompletionTick = 0u;
    const std::set<std::string> marioCandidates{
        "KAKULSAYDON_G1_PATTERN_38", "KAKULSAYDON_G1_PATTERN_39", "KAKULSAYDON_G1_PATTERN_40",
        "KAKULSAYDON_G1_PATTERN_43", "KAKULSAYDON_G1_PATTERN_52", "KAKULSAYDON_G1_PATTERN_46"};
    const auto marioScenario = [&](const MARIO_SCENARIO scenario, const std::uint32_t seed = 71023u) {
        const bool noFollowup = scenario == MARIO_SCENARIO::NO_FOLLOWUP || scenario == MARIO_SCENARIO::CANCEL;
        const bool enterPortal = scenario != MARIO_SCENARIO::NO_ENTRY && !noFollowup;
        const bool abortScenario = scenario == MARIO_SCENARIO::DEATH || scenario == MARIO_SCENARIO::DISCONNECT;
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        const auto* placement = room->Find_Placement("boss.kakulsaydon.g3.saydon");
        SERVER_WORLD_ENTITY boss{};
        if (!room->Is_Ready() || !placement || !room->Build_WorldEntity(*placement, room->m_iNextNetEntityId++, boss)) {
            tests.Require(false, "Load Mario scenario from the published Gate 3 placement"); return std::vector<std::string>{};
        }
        // Fix the test's two-completion/six-candidate scenario explicitly. Live authoring may
        // select a different count or pool without changing this optional runtime contract.
        {
            auto generation = std::make_shared<CGameplayCatalog>(room->m_GameplayCatalog.Active());
            auto* definitions = const_cast<std::vector<BOSS_PATTERN_DEFINITION>*>(generation->Find_BossPatterns("ENCOUNTER_KAKULSAYDON_G1"));
            if (definitions) for (auto& definition : *definitions) if (definition.strPatternId == "KAKULSAYDON_G1_PATTERN_34")
                for (auto& window : definition.LogicWindows) if (window.eKind == BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT) {
                    window.iCompletionCount = 2u;
                    window.PatternIds = {"KAKULSAYDON_G1_PATTERN_38", "KAKULSAYDON_G1_PATTERN_39", "KAKULSAYDON_G1_PATTERN_40",
                        "KAKULSAYDON_G1_PATTERN_43", "KAKULSAYDON_G1_PATTERN_52", "KAKULSAYDON_G1_PATTERN_46"};
                    window.OnSuccess.clear();
                    if (!noFollowup) {
                        BOSS_PATTERN_LOGIC_RESULT followup{};
                        followup.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN;
                        followup.strPatternId = "KAKULSAYDON_G1_PATTERN_33";
                        window.OnSuccess = {followup};
                    }
                }
            // This fixture exercises optional success with no entrant, independently of
            // a designer-authored failure policy on the current product's entry window.
            if (definitions) for (auto& definition : *definitions) if (definition.strPatternId == "KAKULSAYDON_G1_PATTERN_34")
                for (auto& window : definition.LogicWindows) if (window.eKind == BOSS_PATTERN_LOGIC_KIND::ENTER_AREA)
                    window.OnTimeout.clear();
            room->m_pKoukuPublishedProductGeneration = std::move(generation);
        }
        const auto bossId = boss.iNetEntityId;
        room->m_WorldEntities.push_back(boss);
        SERVER_PLAYER player{};
        player.iPlayerId = 923u; player.iNetEntityId = 924u; player.iSessionId = 925u;
        player.iCurrentHp = player.iMaximumHp = 100000u; player.isCombatReady = true;
        player.fPositionX = boss.fPositionX; player.fPositionY = boss.fPositionY; player.fPositionZ = boss.fPositionZ;
        if (!enterPortal || scenario == MARIO_SCENARIO::LAST_TICK_ENTRY) player.fPositionX += 25.f;
        room->m_Players.emplace(player.iPlayerId, player);
        if (scenario == MARIO_SCENARIO::DISCONNECT) {
            // A disconnected transport may already be expired; the room bindings still own departure cleanup.
            room->m_Sessions.emplace(player.iSessionId, std::weak_ptr<CClientSession>{});
            room->m_PlayerIdBySessionId.emplace(player.iSessionId, player.iPlayerId);
            room->m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
        }
        if (scenario == MARIO_SCENARIO::PARTY) {
            auto other = player; other.iPlayerId = 926u; other.iNetEntityId = 927u; other.iSessionId = 928u;
            other.fPositionX += 25.f; room->m_Players.emplace(other.iPlayerId, other);
        }
        C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request{};
        request.iRequestSequence = 1u;
        request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
        request.Scope.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
        request.Scope.strGateId = "GATE3"; request.Scope.strEncounterId = "ENCOUNTER_KAKULSAYDON_G1";
        request.Scope.strBossPlacementId = "boss.kakulsaydon.g3.saydon";
        request.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G3_SAYDON";
        request.Scope.ExpectedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        request.Scope.iExpectedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(room->m_GameplayCatalog.Active());
        request.strPatternId = "KAKULSAYDON_G1_PATTERN_34";
        request.iMarioTestStartStage = 3u; request.iMarioTestSeed = seed;
        CPacketWriter requestWriter;
        C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST decoded{};
        bool codec = Write_Message(requestWriter, request);
        CPacketReader requestReader(requestWriter.Get_Buffer());
        codec = codec && Read_Message(requestReader, decoded) && decoded.iMarioTestStartStage == 3u && decoded.iMarioTestSeed == seed;
        auto oldPayload = requestWriter.Get_Buffer(); oldPayload.erase(oldPayload.begin(), oldPayload.begin() + 5u);
        CPacketReader oldReader(oldPayload);
        codec = codec && !Read_Message(oldReader, decoded) && decoded.iMarioTestStartStage == 3u;
        auto invalid = request; invalid.iMarioTestStartStage = 5u; CPacketWriter invalidWriter;
        tests.Require(codec && !Write_Message(invalidWriter, invalid), "Round-trip Mario test state and reject old or invalid request payload transactionally");
        S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result{};
        const bool queued = KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED == room->Evaluate_KoukuSaydonPatternAudition(925u, request, result);
        if (!queued) { std::cerr << "Mario admission: " << result.strReason << '\n'; tests.Require(false, "Admit actual Mario phase 1 with its dependency closure"); return std::vector<std::string>{}; }
        std::vector<std::string> selected;
        std::uint32_t completed = 0u, phase2Tick = 0u, completionTick = 0u, returnTick = 0u;
        bool bounded = true, retained = false, wire = false, returnStarted = false, aborted = false, waited = false;
        bool entryCommitted = false, landedBeforeCount = false, terminalCompleted = false;
        std::uint32_t finalChildStartTick = 0u, finalChildDurationTicks = 0u;
        for (std::uint32_t tick = 1u; tick < 2400u; ++tick) {
            room->m_iServerTick = tick;
            if (returnStarted && room->m_Players.contains(player.iPlayerId) && room->m_Players.at(player.iPlayerId).TriggerMove.isActive)
                room->Update_Players(1.f / 30.f);
            if (scenario == MARIO_SCENARIO::LAST_TICK_ENTRY && tick == noEntryCompletionTick &&
                !room->m_KoukuSaydonPatternAudition.Members.empty()) {
                const auto& anchor = room->m_KoukuSaydonPatternAudition.Members.front().MarioEntryAnchor;
                if (anchor) {
                    auto& entrant = room->m_Players.at(player.iPlayerId);
                    entrant.fPositionX = anchor->fPositionX; entrant.fPositionY = anchor->fPositionY; entrant.fPositionZ = anchor->fPositionZ;
                }
            }
            room->Prepare_KoukuAuditionTick(tick);
            auto* live = room->Find_KoukuSaydonArenaBoss(request.Scope.strBossPlacementId, request.Scope.strBossArchetypeId);
            if (room->m_KoukuSaydonPatternAudition.Members.empty()) {
                const auto receipt = room->m_KoukuSaydonPatternAuditionReceiptBySessionId.find(player.iSessionId);
                if (scenario == MARIO_SCENARIO::DISCONNECT) {
                    aborted = room->Is_Ready() && room->m_Players.empty() && room->m_Sessions.empty() &&
                        room->m_PlayerIdBySessionId.empty() && room->m_PlayerIdByEntityId.empty() && !live &&
                        room->m_iNextMarioEntryStage == 1u && room->m_PendingKoukuSaydonPatternAuditionLifecycle.empty() &&
                        receipt == room->m_KoukuSaydonPatternAuditionReceiptBySessionId.end() &&
                        room->Get_Status() == "Replayable arena reset after the room became empty";
                } else {
                    aborted = scenario == MARIO_SCENARIO::DEATH && room->Get_Status().find("entrant died") != std::string::npos &&
                        receipt != room->m_KoukuSaydonPatternAuditionReceiptBySessionId.end() && receipt->second.LastLifecycle &&
                        receipt->second.LastLifecycle->eState == KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED &&
                        receipt->second.LastLifecycle->strReason == room->Get_Status();
                }
                bounded = bounded && aborted; break;
            }
            if (!live || !room->Update_KoukuSaydonBoss(*live, tick)) { bounded = false; break; }
            if (room->m_KoukuSaydonPatternAudition.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE) {
                const auto receipt = room->m_KoukuSaydonPatternAuditionReceiptBySessionId.find(player.iSessionId);
                terminalCompleted = noFollowup && receipt != room->m_KoukuSaydonPatternAuditionReceiptBySessionId.end() &&
                    receipt->second.LastLifecycle && receipt->second.LastLifecycle->eState == KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED;
                bounded = bounded && terminalCompleted && completed == 1u && finalChildDurationTicks &&
                    tick + 1u >= finalChildStartTick + finalChildDurationTicks && live->strPatternId.empty() && room->m_PendingKoukuMarioEntries.empty();
                if (terminalCompleted) { completed = 2u; completionTick = tick; }
                break;
            }
            room->Commit_KoukuMarioEntries();
            // Committing the entry can append Mario objects and reallocate world entities.
            live = room->Find_KoukuSaydonArenaBoss(request.Scope.strBossPlacementId, request.Scope.strBossArchetypeId);
            if (!live) { bounded = false; break; }
            const auto& member = room->m_KoukuSaydonPatternAudition.Members.front();
            if (noFollowup && member.bCompletionChainStarted && member.iCompletionChainCompleted == 1u &&
                member.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE && !finalChildStartTick) {
                finalChildStartTick = live->iPatternStartTick;
                std::string childStatus;
                const auto* child = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*room->Resolve_KoukuProductCatalog(), live->strPatternId, childStatus);
                if (child) {
                    std::uint32_t totalMs = 0u;
                    for (const auto& stage : child->Stages) {
                        totalMs += stage.iDurationMs;
                        finalChildDurationTicks += CKoukuSaydonLogicRuntime::Ticks_FromMs(stage.iDurationMs);
                    }
                    if (child->bFixedTimelineClock) finalChildDurationTicks = CKoukuSaydonLogicRuntime::Ticks_FromMs(totalMs);
                }
            }
            entryCommitted = entryCommitted || member.bMarioEntryConsumed;
            if (member.bCompletionChainStarted && selected.empty())
                selected.assign(member.PatternIds.begin() + member.iCompletionChainFirstIndex,
                    member.PatternIds.begin() + member.iCompletionChainFirstIndex + member.iCompletionChainCount);
            bounded = bounded && member.iCompletionChainCompleted >= completed && member.iCompletionChainCompleted <= completed + 1u;
            completed = member.iCompletionChainCompleted;
            if (completed == 2u && !completionTick) completionTick = tick;
            if (member.bMarioReturnCompleted && !returnTick) { returnTick = tick; landedBeforeCount = completed < 2u; }
            if (completed < 2u && member.bCompletionChainStarted) {
                bounded = bounded && std::find(member.PatternIds.begin(), member.PatternIds.end(), "KAKULSAYDON_G1_PATTERN_33") == member.PatternIds.end();
                retained = retained || member.MarioEntryAnchor.has_value();
                S2C_KOUKUSAYDON_BUNDLE_STATE state{}, decodedState{};
                CPacketWriter writer;
                if (room->Build_KoukuBundleState(state) && Write_Message(writer, state)) {
                    CPacketReader reader(writer.Get_Buffer());
                    wire = Read_Message(reader, decodedState) && decodedState.Members.front().iBossNetEntityId == bossId &&
                        (member.ePhase != CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE ||
                         decodedState.Members.front().iStartTick == live->iPatternStartTick) &&
                        (member.bMarioEntryConsumed ? decodedState.Members.front().strMarioEntryPatternId.empty() :
                            decodedState.Members.front().strMarioEntryPatternId == request.strPatternId && decodedState.Members.front().iMarioEntryStage == 3u);
                    auto truncated = writer.Get_Buffer(); truncated.pop_back(); CPacketReader shortReader(truncated);
                    wire = wire && !Read_Message(shortReader, decodedState) && decodedState.iRunEpoch == state.iRunEpoch;
                    auto oldState = writer.Get_Buffer();
                    oldState.resize(oldState.size() - (27u + state.Members.front().strMarioEntryPatternId.size()));
                    CPacketReader oldStateReader(oldState);
                    auto invalidState = state; invalidState.Members.front().iMarioEntryStage = 5u;
                    CPacketWriter invalidStateWriter;
                    wire = wire && !Read_Message(oldStateReader, decodedState) && !Write_Message(invalidStateWriter, invalidState);
                }
            }
            if (scenario == MARIO_SCENARIO::CANCEL && member.bCompletionChainStarted && !completed) {
                room->Clear_KoukuSaydonPatternAudition(false, "Mario chain contract cancellation");
                const auto receipt = room->m_KoukuSaydonPatternAuditionReceiptBySessionId.find(player.iSessionId);
                tests.Require(room->m_KoukuSaydonPatternAudition.Members.empty() && receipt != room->m_KoukuSaydonPatternAuditionReceiptBySessionId.end() &&
                    receipt->second.LastLifecycle && receipt->second.LastLifecycle->eState == KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED,
                    "Cancellation before child completion aborts rather than completing the two-pattern chain");
                return selected;
            }
            if (member.bCompletionChainAwaitingReturn && member.bMarioSoloReturnRequired && !member.bMarioReturnCompleted) {
                waited = waited || tick > completionTick;
                bounded = bounded && live->strPatternId.empty() && !member.bCompletionChainSuccessQueued &&
                    std::find(member.PatternIds.begin(), member.PatternIds.end(), "KAKULSAYDON_G1_PATTERN_33") == member.PatternIds.end();
                if (abortScenario && tick == completionTick + 5u) {
                    if (scenario == MARIO_SCENARIO::DEATH) { auto& entrant = room->m_Players.at(player.iPlayerId); entrant.iCurrentHp = 0u; entrant.eAction = PLAYER_ACTION_STATE::DEAD; }
                    else room->Leave(player.iSessionId, PLAYER_DESPAWN_REASON::DISCONNECTED, false);
                    continue;
                }
            }
            const bool returnEarly = scenario == MARIO_SCENARIO::RETURN_EARLY && member.bMarioEntryConsumed;
            const bool returnAfter = (scenario == MARIO_SCENARIO::RETURN_AFTER_CHAIN || scenario == MARIO_SCENARIO::LAST_TICK_ENTRY) &&
                completionTick && tick >= completionTick + 60u;
            if (!returnStarted && (returnEarly || returnAfter)) {
                auto& entrant = room->m_Players.at(player.iPlayerId);
                bool began = false;
                if (returnEarly) {
                    C2S_MARIO_RETURN command{}; command.iClientSequence = 1u; command.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
                    began = room->Apply_MarioReturn(entrant, command).eResult == MARIO_RETURN_RESULT::ACCEPTED;
                    const auto motion = entrant.TriggerMove;
                    began = began && room->Apply_MarioReturn(entrant, command).eResult == MARIO_RETURN_RESULT::ACCEPTED &&
                        entrant.TriggerMove.fElapsedSeconds == motion.fElapsedSeconds;
                } else {
                    // The current product pins a pattern-owned landing. The typed Return
                    // command consumes it; replaying the static exit action would bypass it.
                    C2S_MARIO_RETURN command{}; command.iClientSequence = 1u; command.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
                    began = room->Apply_MarioReturn(entrant, command).eResult == MARIO_RETURN_RESULT::ACCEPTED;
                }
                tests.Require(began && entrant.TriggerMove.isActive && !member.bMarioReturnCompleted,
                    "Start terminal return through authored motion and keep the solo gate closed before landing");
                if (!began) { bounded = false; break; }
                room->Complete_KoukuMarioReturn(entrant, "Mario3_Trigger_12", tick);
                bounded = bounded && !member.bMarioReturnCompleted;
                returnStarted = true;
            }
            if (member.PatternIds[member.iPatternIndex] == "KAKULSAYDON_G1_PATTERN_33") {
                phase2Tick = tick;
                if (member.bMarioReturnCompleted) {
                    room->Complete_KoukuMarioReturn(room->m_Players.at(player.iPlayerId), "Mario3_Trigger_12", tick);
                    room->Prepare_KoukuAuditionTick(tick);
                }
                bounded = bounded && std::count(member.PatternIds.begin(), member.PatternIds.end(), "KAKULSAYDON_G1_PATTERN_33") == 1;
                break;
            }
        }
        if (scenario == MARIO_SCENARIO::NO_ENTRY) noEntryCompletionTick = completionTick;

        tests.Require(bounded && retained && wire && selected.size() == 2u && std::set<std::string>(selected.begin(), selected.end()).size() == 2u &&
            std::all_of(selected.begin(), selected.end(), [&](const auto& id) { return marioCandidates.contains(id); }) &&
            completed == 2u && (noFollowup ? terminalCompleted && !phase2Tick : (abortScenario ? aborted && !phase2Tick : phase2Tick > completionTick)),
            "Count two distinct Server-selected real pattern completions and preserve optional terminal or followup behavior");
        const auto expectedNextStage = scenario == MARIO_SCENARIO::DISCONNECT ? 1u : (enterPortal ? 4u : 3u);
        tests.Require(entryCommitted == enterPortal && room->m_iNextMarioEntryStage == expectedNextStage,
            "Advance the successful-entry count once and reset it after the last real session leaves");
        if (scenario == MARIO_SCENARIO::RETURN_AFTER_CHAIN || scenario == MARIO_SCENARIO::LAST_TICK_ENTRY)
            tests.Require(waited && returnTick > completionTick && phase2Tick >= returnTick && !room->m_Players.at(player.iPlayerId).iMarioStage,
                "Wait for the solo entrant's actual terminal landing even when entry commits on the last child tick");
        if (scenario == MARIO_SCENARIO::RETURN_EARLY)
            tests.Require(landedBeforeCount && phase2Tick == completionTick + 1u,
                "A solo return completed first still waits for two patterns and queues phase 2 once");
        if (scenario == MARIO_SCENARIO::NO_ENTRY || scenario == MARIO_SCENARIO::PARTY)
            tests.Require(!waited && !returnStarted && phase2Tick == completionTick + 1u,
                "No entrant and a multiplayer entry both keep the existing immediate two-pattern success path");
        if (abortScenario)
            tests.Require(waited && aborted && !phase2Tick, "A solo entrant death or disconnect aborts explicitly without phase-2 success");
        return selected;
    };
    (void)marioScenario(MARIO_SCENARIO::NO_FOLLOWUP);
    (void)marioScenario(MARIO_SCENARIO::CANCEL);
    // Select a reproducible Server seed that includes the summon Parent; the
    // actual room still performs the selection and runs its published duration.
    std::uint32_t summonSeed = 0u;
    for (; summonSeed < 1000u; ++summonSeed) {
        std::vector<std::string> candidates{
            "KAKULSAYDON_G1_PATTERN_38", "KAKULSAYDON_G1_PATTERN_39", "KAKULSAYDON_G1_PATTERN_40",
            "KAKULSAYDON_G1_PATTERN_43", "KAKULSAYDON_G1_PATTERN_52", "KAKULSAYDON_G1_PATTERN_46"};
        std::mt19937 random(summonSeed);
        for (std::size_t i = candidates.size(); i > 1u; --i) std::swap(candidates[i - 1u], candidates[random() % i]);
        if (candidates[0] == "KAKULSAYDON_G1_PATTERN_52" || candidates[1] == "KAKULSAYDON_G1_PATTERN_52") break;
    }
    const auto summonOrder = marioScenario(MARIO_SCENARIO::NO_FOLLOWUP, summonSeed);
    tests.Require(std::find(summonOrder.begin(), summonOrder.end(), "KAKULSAYDON_G1_PATTERN_52") != summonOrder.end(),
        "The actual two-pattern chain can select and complete the timed summon Parent");
    const auto firstMarioOrder = marioScenario(MARIO_SCENARIO::NO_ENTRY);
    const auto secondMarioOrder = marioScenario(MARIO_SCENARIO::RETURN_AFTER_CHAIN);
    tests.Require(!firstMarioOrder.empty() && firstMarioOrder == secondMarioOrder, "Replay the same Mario test seed through the same Server completion path");
    (void)marioScenario(MARIO_SCENARIO::RETURN_EARLY);
    (void)marioScenario(MARIO_SCENARIO::PARTY);
    (void)marioScenario(MARIO_SCENARIO::DEATH);
    (void)marioScenario(MARIO_SCENARIO::DISCONNECT);
    (void)marioScenario(MARIO_SCENARIO::LAST_TICK_ENTRY);
    // The Parent P88 (KAKULSAYDON_G1_PATTERN_88) owns ENTER_AREA -> MARIO_ENTER without a completion chain;
    // its portal is the pattern's own window and it must not publish a Client hold.
    // LAST_TICK ends P88 early through a stagger success on the very tick the entrant is judged inside the
    // Mario window, so the queued entry and the pattern completion race within one boss update.
    enum class PARENT_ENTRY { AUTHORED_STAGE, LIVE_COUNTER, REQUEST_STAGE, AFTER_WINDOW, LAST_TICK, LAST_TICK_SINGLE, MULTI_WINDOW };
    const auto parentMarioEntry = [&](const PARENT_ENTRY scenario) {
        const std::string parentId = "KAKULSAYDON_G1_PATTERN_88";
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        const auto* placement = room->Find_Placement("boss.kakulsaydon.g3.saydon");
        SERVER_WORLD_ENTITY boss{};
        if (!room->Is_Ready() || !placement || !room->Build_WorldEntity(*placement, room->m_iNextNetEntityId++, boss)) {
            tests.Require(false, "Load parent Mario entry scenario from the published Gate 3 placement"); return;
        }
        const std::uint8_t authoredStage = scenario == PARENT_ENTRY::LIVE_COUNTER ? 0u : 1u;
        const std::uint8_t expectedStage = scenario == PARENT_ENTRY::LIVE_COUNTER ? 3u : scenario == PARENT_ENTRY::REQUEST_STAGE ? 2u : scenario == PARENT_ENTRY::MULTI_WINDOW ? 4u : 1u;
        const bool finalTick = scenario == PARENT_ENTRY::LAST_TICK || scenario == PARENT_ENTRY::LAST_TICK_SINGLE;
        auto generation = std::make_shared<CGameplayCatalog>(room->m_GameplayCatalog.Active());
        auto* definitions = const_cast<std::vector<BOSS_PATTERN_DEFINITION>*>(generation->Find_BossPatterns("ENCOUNTER_KAKULSAYDON_G1"));
        BOSS_LOGIC_REGION region{}; std::uint32_t windowStartMs = 0u, windowEndMs = 0u; bool authored = false, chainFree = false;
        if (definitions) for (auto& definition : *definitions) if (definition.strPatternId == parentId) {
            chainFree = std::none_of(definition.LogicWindows.begin(), definition.LogicWindows.end(),
                [](const auto& window) { return window.eKind == BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT; });
            for (auto& window : definition.LogicWindows) if (window.strWindowId == parentId + ".logic.1" &&
                window.eKind == BOSS_PATTERN_LOGIC_KIND::ENTER_AREA && !window.CardRegions.empty()) {
                BOSS_PATTERN_LOGIC_RESULT entry{};
                entry.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER; entry.iMarioEntryStage = authoredStage;
                window.OnSuccess = {entry}; region = window.CardRegions.front();
                windowStartMs = window.iStartMs; windowEndMs = window.iStartMs + window.iDurationMs; authored = true;
            }
            if (scenario == PARENT_ENTRY::MULTI_WINDOW && authored) {
                const auto first = std::find_if(definition.LogicWindows.begin(), definition.LogicWindows.end(),
                    [&](const auto& window) { return window.strWindowId == parentId + ".logic.1"; });
                auto second = *first; second.strWindowId = parentId + ".logic.second-entry";
                second.OnSuccess.front().iMarioEntryStage = 4u;
                for (auto& firstRegion : first->CardRegions) firstRegion.fCenterX += 50.f;
                definition.LogicWindows.push_back(std::move(second));
            }
            if (finalTick && authored) {
                BOSS_PATTERN_LOGIC_WINDOW stagger{};
                stagger.strWindowId = parentId + ".logic.lasttick"; stagger.eKind = BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW;
                stagger.iStartMs = windowStartMs; stagger.iDurationMs = windowEndMs - windowStartMs;
                stagger.iThreshold = 1000u; stagger.bEndsPatternOnSuccess = true;
                definition.LogicWindows.push_back(stagger);
            }
        }
        std::string status;
        const auto* parent = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, parentId, status);
        tests.Require(authored && chainFree && parent && CKoukuSaydonLogicRuntime::Has_MarioEntry(*parent) &&
            CKoukuSaydonLogicRuntime::Find_MarioEntryResult(*parent) && CKoukuSaydonLogicRuntime::Find_MarioEntryResult(*parent)->iMarioEntryStage == authoredStage,
            "The published Parent P88 owns a chain-free ENTER_AREA window that can carry the authored Mario entry");
        if (!authored) return;
        room->m_pKoukuPublishedProductGeneration = std::move(generation);
        room->m_WorldEntities.push_back(boss);
        SERVER_PLAYER player{};
        player.iPlayerId = 940u; player.iNetEntityId = 941u; player.iSessionId = 942u;
        player.iCurrentHp = player.iMaximumHp = 100000u; player.isCombatReady = true;
        player.fPositionX = boss.fPositionX + 25.f; player.fPositionY = boss.fPositionY; player.fPositionZ = boss.fPositionZ;
        room->m_Players.emplace(player.iPlayerId, player);
        if (scenario == PARENT_ENTRY::LIVE_COUNTER) room->m_iNextMarioEntryStage = 3u;
        C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request{};
        request.iRequestSequence = 1u;
        request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
        request.Scope.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
        request.Scope.strGateId = "GATE3"; request.Scope.strEncounterId = "ENCOUNTER_KAKULSAYDON_G1";
        request.Scope.strBossPlacementId = "boss.kakulsaydon.g3.saydon";
        request.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G3_SAYDON";
        request.Scope.ExpectedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        request.Scope.iExpectedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(room->m_GameplayCatalog.Active());
        request.strPatternId = parentId;
        if (scenario == PARENT_ENTRY::REQUEST_STAGE) { request.iMarioTestStartStage = 2u; request.iMarioTestSeed = 7u; }
        S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result{};
        const bool queued = KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED == room->Evaluate_KoukuSaydonPatternAudition(player.iSessionId, request, result);
        if (!queued) std::cerr << "Parent Mario admission: " << result.strReason << '\n';
        tests.Require(queued, "Admit a chain-free Gate 3 Parent as a Mario entry pattern, including the request's test stage");
        if (!queued) return;
        const std::uint32_t enterMs = scenario == PARENT_ENTRY::AFTER_WINDOW ? windowEndMs + 1000u : windowStartMs + 500u;
        bool bounded = true, spliced = false, moved = false, stageOk = false, wire = false, anchorCleared = false, pendingClear = false, secondReached = false;
        bool singleCompletionQueued = false;
        std::uint32_t memberStart = 0u, entryTick = 0u, completionTick = 0u;
        for (std::uint32_t tick = 1u; tick < 6000u; ++tick) {
            room->m_iServerTick = tick;
            room->Prepare_KoukuAuditionTick(tick);
            auto* live = room->Find_KoukuSaydonArenaBoss(request.Scope.strBossPlacementId, request.Scope.strBossArchetypeId);
            if (room->m_KoukuSaydonPatternAudition.Members.empty()) { bounded = false; break; }
            if (!live || !room->Update_KoukuSaydonBoss(*live, tick)) { bounded = false; break; }
            if (room->m_KoukuSaydonPatternAudition.Members.empty()) { bounded = false; break; }
            if (scenario == PARENT_ENTRY::LAST_TICK_SINGLE && room->m_KoukuSaydonPatternAudition.Members.front().bCompleted) {
                singleCompletionQueued = room->m_KoukuSaydonPatternAudition.Members.size() == 1u &&
                    room->m_PendingKoukuMarioEntries.size() == 1u && room->m_KoukuSaydonPatternAudition.Members.front().MarioEntryAnchor.has_value();
                completionTick = tick;
            }
            room->Commit_KoukuMarioEntries();
            if (room->m_KoukuSaydonPatternAudition.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE) {
                if (scenario == PARENT_ENTRY::LAST_TICK_SINGLE && singleCompletionQueued) {
                    entryTick = room->m_Players.at(player.iPlayerId).iMarioStage ? tick : 0u;
                    secondReached = true; anchorCleared = true; pendingClear = room->m_PendingKoukuMarioEntries.empty();
                }
                else bounded = false;
                break;
            }
            auto& member = room->m_KoukuSaydonPatternAudition.Members.front();
            if (!spliced && member.MarioEntryAnchor && member.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE) {
                // The single-member regression deliberately ends the whole run on the entry tick.
                if (scenario != PARENT_ENTRY::LAST_TICK_SINGLE) {
                    member.PatternIds.push_back("KAKULSAYDON_G1_PATTERN_38");
                    member.TransitionTicks.resize(member.PatternIds.size() - 1u, 1u);
                }
                spliced = true; memberStart = member.iMarioEntryStartTick;
                stageOk = member.iMarioEntryStage == (scenario == PARENT_ENTRY::MULTI_WINDOW ? authoredStage : expectedStage) && member.strMarioEntryPatternId == parentId && !member.bMarioEntryConsumed;
                S2C_KOUKUSAYDON_BUNDLE_STATE state{}, decoded{}; CPacketWriter writer;
                wire = room->Build_KoukuBundleState(state) && Write_Message(writer, state);
                if (wire) { CPacketReader reader(writer.Get_Buffer()); wire = Read_Message(reader, decoded) && !decoded.Members.empty() &&
                    decoded.Members.front().strMarioEntryPatternId.empty() && decoded.Members.front().iMarioEntryHoldMs == 0u; }
            }
            if (spliced && !moved && tick - memberStart >= CKoukuSaydonLogicRuntime::Ticks_FromMs(enterMs)) {
                auto& entrant = room->m_Players.at(player.iPlayerId);
                entrant.fPositionX = region.fCenterX; entrant.fPositionY = region.fCenterY; entrant.fPositionZ = region.fCenterZ;
                // The stagger threshold is crossed on the same next tick that judges the entrant inside the region.
                if (finalTick) live->iCurrentHp -= 1000u;
                moved = true;
            }
            if (member.bMarioEntryConsumed && !entryTick) entryTick = tick;
            if (member.iPatternIndex == 1u) {
                secondReached = true; completionTick = tick;
                anchorCleared = !member.MarioEntryAnchor.has_value(); pendingClear = room->m_PendingKoukuMarioEntries.empty();
                room->Clear_KoukuSaydonPatternAudition(false, "Parent Mario entry contract end");
                break;
            }
        }
        const auto& entrant = room->m_Players.at(player.iPlayerId);
        tests.Require(bounded && spliced && stageOk && wire && secondReached && anchorCleared && pendingClear,
            "A chain-free entry Parent pins its own anchor without a Client hold and drops the portal when the pattern completes");
        if (scenario == PARENT_ENTRY::AFTER_WINDOW)
            tests.Require(moved && !entryTick && entrant.iMarioStage == 0u && room->m_iNextMarioEntryStage == 1u,
                "A chain-free Mario entry closes when its ENTER_AREA window ends");
        else if (finalTick)
            tests.Require(moved && entryTick && entryTick == completionTick &&
                (scenario != PARENT_ENTRY::LAST_TICK_SINGLE || singleCompletionQueued) &&
                entrant.iMarioStage == expectedStage && room->m_iNextMarioEntryStage == static_cast<std::uint8_t>(expectedStage + 1u),
                "An entry queued on the pattern's final tick still commits before the portal closes");
        else
            tests.Require(moved && entryTick && entryTick - memberStart >= CKoukuSaydonLogicRuntime::Ticks_FromMs(windowStartMs) &&
                entrant.iMarioStage == expectedStage && room->m_iNextMarioEntryStage == static_cast<std::uint8_t>(expectedStage + 1u),
                "Authored stage, live counter and request test stage each enter the expected Mario stage and advance the counter once");
    };
    parentMarioEntry(PARENT_ENTRY::AUTHORED_STAGE);
    parentMarioEntry(PARENT_ENTRY::LIVE_COUNTER);
    parentMarioEntry(PARENT_ENTRY::REQUEST_STAGE);
    parentMarioEntry(PARENT_ENTRY::AFTER_WINDOW);
    parentMarioEntry(PARENT_ENTRY::LAST_TICK);
    parentMarioEntry(PARENT_ENTRY::LAST_TICK_SINGLE);
    parentMarioEntry(PARENT_ENTRY::MULTI_WINDOW);
    {
        // PATTERNLOGICOUTCOME: an 11th field on MARIO_ENTER is the authored stage 1..4; other kinds keep the fear-only rule.
        namespace fs = std::filesystem;
        std::vector<wchar_t> buffer(32768u); fs::path dataRoot;
        const DWORD configured = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", buffer.data(), static_cast<DWORD>(buffer.size()));
        if (configured && configured < buffer.size()) dataRoot = buffer.data();
        else { GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size())); dataRoot = fs::path(buffer.data()).parent_path().parent_path() / L"DataFiles"; }
        std::ifstream input(dataRoot / L"Gameplay" / L"Gameplay.bootstrap", std::ios::binary);
        std::string bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        if (!bytes.empty() && bytes.back() != '\n') bytes += '\n';
        const std::string row = "PATTERNLOGICOUTCOME\tENCOUNTER_KAKULSAYDON_G1\tKAKULSAYDON_G1_PATTERN_88\tKAKULSAYDON_G1_PATTERN_88.logic.1\tSUCCESS\t0\t";
        // Replace this fixture's exact outcome ordinal; the saved product may already own it.
        for (auto at = bytes.find(row); at != std::string::npos; at = bytes.find(row)) {
            const auto end = bytes.find('\n', at); bytes.erase(at, (end == std::string::npos ? bytes.size() : end + 1u) - at);
        }
        unsigned fixture = 0u;
        const auto load = [&](const std::string& extra, std::uint8_t& outStage) {
            std::string copy = bytes + extra;
            const auto headerEnd = copy.find('\n');
            const auto headerCount = copy.rfind('\t', headerEnd);
            copy.replace(headerCount + 1u, headerEnd - headerCount - 1u, std::to_string(std::count(copy.begin(), copy.end(), '\n') - 1u));
            const fs::path directory = fs::temp_directory_path() / (L"LostArkMarioStageContract-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(++fixture));
            std::error_code error; fs::create_directories(directory, error); const fs::path path = directory / L"Gameplay.bootstrap";
            { std::ofstream output(path, std::ios::binary | std::ios::trunc); output.write(copy.data(), static_cast<std::streamsize>(copy.size())); }
            GameplayDataRevision revision; std::string status; CGameplayCatalog generation;
            const bool loaded = !error && CServerApp::Hash_GameplayFileForAdmission(path, revision, status) && generation.Load_FromBootstrap(fs::canonical(path), revision, revision);
            outStage = 255u;
            const auto* pattern = loaded ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(generation, "KAKULSAYDON_G1_PATTERN_88", status) : nullptr;
            const auto* entry = pattern ? CKoukuSaydonLogicRuntime::Find_MarioEntryResult(*pattern) : nullptr;
            if (entry) outStage = entry->iMarioEntryStage;
            if (!loaded) std::cout << "[STATUS] Mario stage fixture " << fixture << ": " << generation.Get_Status() << '\n';
            fs::remove_all(directory, error);
            return loaded;
        };
        std::uint8_t stage = 255u;
        tests.Require(!bytes.empty() && load(row + "MARIO_ENTER\t0\t0\t-\t2\n", stage) && stage == 2u,
            "An 11-field MARIO_ENTER outcome row carries the authored Mario stage");
        tests.Require(load(row + "MARIO_ENTER\t0\t0\t-\n", stage) && stage == 0u,
            "A 10-field MARIO_ENTER outcome row keeps the live room counter");
        tests.Require(!load(row + "MARIO_ENTER\t0\t0\t-\t5\n", stage) && !load(row + "MARIO_ENTER\t0\t0\t-\t0\n", stage) &&
            !load(row + "INSTANT_DEATH\t0\t0\t-\t2\n", stage),
            "Mario stages outside 1..4 and an 11th field on a non-fear, non-Mario outcome are rejected");
    }
    for (std::uint8_t stage = 1u; stage <= 4u; ++stage) {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        SERVER_PLAYER player{}; player.iPlayerId = 930u; player.iNetEntityId = 931u;
        player.iCurrentHp = player.iMaximumHp = 100000u; player.isCombatReady = true;
        const bool entered = room->Enter_MarioFromPattern(player, stage);
        const auto after = player;
        const bool duplicateRejected = !room->Enter_MarioFromPattern(player, stage) && player.iMarioStage == after.iMarioStage && player.fPositionX == after.fPositionX;
        if (!entered) std::cerr << "Mario stage " << unsigned(stage) << ": " << room->Get_Status() << '\n';
        tests.Require(entered && duplicateRejected && player.iMarioStage == stage,
            "Enter each published Mario Intro through existing mode/layout setup and reject duplicate entry");
    }
#endif



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
				// Product admission now pins the exact Gate and placement as well as the body.
				secondRequest.Scope.strGateId = "GATE3";
				const BOSS_PATTERN_DEFINITION* gateThreePattern = nullptr;
				for (const auto& id : sequence->PatternIds)
				{
					const auto found = std::find_if(patterns->begin(), patterns->end(),
						[&](const auto& pattern) { return pattern.strPatternId == id &&
							pattern.strGateId == secondRequest.Scope.strGateId &&
							pattern.strTargetBossPlacementId == secondRequest.Scope.strBossPlacementId; });
					if (found != patterns->end()) { gateThreePattern = &*found; break; }
				}
				secondRequest.strPatternId = gateThreePattern ? gateThreePattern->strPatternId : std::string{};
				std::uint32_t gateThreeCompletionTicks = 30u;
				if (gateThreePattern)
					for (const auto& stage : gateThreePattern->Stages)
						gateThreeCompletionTicks += CKoukuSaydonLogicRuntime::Ticks_FromMs(stage.iDurationMs);
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT secondResult{};
				const bool secondQueued = secondBuilt && gateThreePattern &&
					KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
						arenaRoom->Evaluate_KoukuSaydonPatternAudition(8103u, secondRequest, secondResult);
				bool originalStayedIdle = secondQueued;
				for (std::uint32_t tick = 0u; originalStayedIdle && tick < gateThreeCompletionTicks; ++tick)
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
				std::uint32_t sequenceBeforeDespawn = 0u;
				const std::uint32_t firstStageTicks = gateThreePattern && !gateThreePattern->Stages.empty() ?
					CKoukuSaydonLogicRuntime::Ticks_FromMs(gateThreePattern->Stages.front().iDurationMs) + 30u : 0u;
				for (std::uint32_t tick = 0u; allQueued && tick < firstStageTicks; ++tick)
				{
					arenaRoom->Update_WorldEntities(1.f / 30.f);
					++arenaRoom->m_iServerTick;
					const SERVER_WORLD_ENTITY* playing = arenaRoom->Find_KoukuSaydonArenaBoss(
						secondRequest.Scope.strBossPlacementId, secondRequest.Scope.strBossArchetypeId);
					if (nullptr != playing)
					{
						stageBeforeDespawn = playing->iPatternStageIndex;
						sequenceBeforeDespawn = playing->iPatternSequence;
					}
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
						[secondKoukuId, sequenceBeforeDespawn](const auto& entry)
						{
							return secondKoukuId == entry.Message.iBossNetEntityId &&
								// Run termination identifies its owner occurrence; it has no member stage.
								sequenceBeforeDespawn == entry.Message.iPatternSequence &&
								0u == entry.Message.iStageIndex && entry.Message.strMemberId.empty() &&
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
				const auto gazeDefinition = std::find_if(patterns->begin(), patterns->end(),
					[](const auto& pattern) { return pattern.strPatternId == "KAKULSAYDON_G1_PATTERN_2"; });
				const BOSS_PATTERN_MECHANIC_TRIGGER* expectedTeleport = nullptr;
				if (gazeDefinition != patterns->end())
					for (const auto& trigger : gazeDefinition->MechanicTriggers)
						if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT)
							expectedTeleport = &trigger;
				std::vector<std::uint8_t> ownerPayload;
				S2C_WORLD_ENTITY_SPAWNED ownerMessage{};
				const bool ownerWritten = gazeRoom->Build_WorldEntitySpawnedPayload(*owner, ownerPayload);
				CPacketReader ownerReader{ ownerPayload };
				admitted = admitted && ownerWritten && Read_Message(ownerReader, ownerMessage) &&
					expectedTeleport &&
					std::abs(owner->fPositionX - expectedTeleport->fTeleportX) < 0.001f &&
					std::abs(owner->fPositionY - expectedTeleport->fTeleportY) < 0.001f &&
					std::abs(owner->fPositionZ - expectedTeleport->fTeleportZ) < 0.001f &&
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
				// Spawn reset commits before the first stage's root motion advances in this tick.
				centeredRoom->Prepare_KoukuAuditionTick(centeredRoom->m_iServerTick + 1u);
				const bool resetAtStartCommit = centeredBoss->fPositionX == centerX &&
					centeredBoss->fPositionY == centerY && centeredBoss->fPositionZ == centerZ &&
					centeredBoss->iPatternStartTick == centeredRoom->m_iServerTick + 1u;
				centeredRoom->Update_WorldEntities(1.f / 30.f);
				++centeredRoom->m_iServerTick;
				centeredBoss = centeredRoom->Find_KoukuSaydonArenaBoss(request.Scope.strBossPlacementId, request.Scope.strBossArchetypeId);
				const bool rootStartedAtSpawn = centeredBoss &&
					(centeredBoss->PatternStageRootMotion.empty() ||
						(centeredBoss->bPatternStageRootOriginCaptured &&
							centeredBoss->fPatternStageOriginX == centerX &&
							centeredBoss->fPatternStageOriginY == centerY &&
							centeredBoss->fPatternStageOriginZ == centerZ));
				tests.Require(queued && resetAtStartCommit && rootStartedAtSpawn &&
					centeredBoss->iPatternStartTick == centeredRoom->m_iServerTick &&
					MECHANIC_CARD_SYMBOL::NONE != assignedSuit && MECHANIC_CARD_COLOR::NONE != assignedColor,
					"Reset the production Dance or Roulette boss to spawn before root motion and publish its fixed start clock with an encounter card");
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
			selected.Scope.strGateId = "GATE1";
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
				// The room prepares all due members before any per-boss update.
				room->Prepare_KoukuAuditionTick(tick);
				runtimeAdvanced = room->Update_KoukuSaydonBoss(*liveBoss, tick);
				room->m_iServerTick = tick;
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
					// PATTERN_COMPLETED above carries the last stage; COMPLETED closes the run.
					exactLifecycleIdentity = exactLifecycleIdentity &&
						occurrenceSequence == lifecycle.iPatternSequence &&
						0u == lifecycle.iStageIndex && lifecycle.strMemberId.empty();
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
				// Duplicate verdicts reconcile to the last run-terminal lifecycle edge.
				0u == duplicateResult.iStageIndex;

			auto playAll = selected;
			playAll.iRequestSequence = 3u;
			playAll.eOperation =
				KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_ALL;
			playAll.strPatternId.clear();
			S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT playAllResult{};
			std::vector<std::string> expectedGateOneOrder;
			std::vector<std::uint32_t> expectedGateOneTransitions;
			std::vector<std::size_t> expectedGateOneIndices;
			for (std::size_t index = 0; index < sequence->PatternIds.size(); ++index)
			{
				const auto definition = std::find_if(patterns->begin(), patterns->end(),
					[&](const auto& pattern) { return pattern.strPatternId == sequence->PatternIds[index]; });
				if (definition != patterns->end() && definition->strGateId == playAll.Scope.strGateId &&
					definition->strTargetBossPlacementId == playAll.Scope.strBossPlacementId)
				{
					expectedGateOneOrder.push_back(definition->strPatternId);
					expectedGateOneIndices.push_back(index);
				}
			}
			// Only selected adjacent occurrences have a transition; the final one has none.
			for (std::size_t index = 0; index + 1 < expectedGateOneIndices.size(); ++index)
				expectedGateOneTransitions.push_back(sequence->TransitionPursuitTicks[expectedGateOneIndices[index]]);
			const bool playAllQueued = duplicateReconciled && !expectedGateOneOrder.empty() &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, playAll, playAllResult) &&
				expectedGateOneOrder ==
					room->m_KoukuSaydonPatternAudition.Members.front().PatternIds &&
				expectedGateOneTransitions ==
					room->m_KoukuSaydonPatternAudition.Members.front().TransitionTicks &&
				playAllResult.Scope.ExpectedGameplayRevision ==
					playAll.Scope.ExpectedGameplayRevision &&
				playAllResult.Scope.iExpectedSourceRevision ==
					playAll.Scope.iExpectedSourceRevision &&
				playAllResult.PinnedGameplayRevision ==
					playAll.Scope.ExpectedGameplayRevision &&
				playAllResult.iPinnedSourceRevision ==
					playAll.Scope.iExpectedSourceRevision &&
				expectedGateOneOrder.front() ==
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

