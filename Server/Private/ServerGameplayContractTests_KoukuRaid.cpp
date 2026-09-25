#include "ServerGameplayContractTests_Runner.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include "KoukuSaydonBrain.h"
#include "ServerApp.h"
#include <Windows.h>
#include <array>
#include <filesystem>
#include <fstream>
#include "Gameplay/KoukuArenaReadyAreas.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>

using namespace LostArk::Server;
using namespace LostArk::Shared;

void CServerGameplayContractRunner::Run_KoukuGate3Entry(TESTS& tests)
{
#ifdef _DEBUG
    tests.Require(Is_KoukuGate3EntryAura(-22.20617676f, 25.59f, 954.5942993f) &&
        !Is_KoukuGate3EntryAura(-11.9999292f, 25.59f, 964.54328125f) &&
        !Is_KoukuGate3EntryAura(-22.20617676f, 1.32f, 954.5942993f) &&
        !Is_KoukuGate3EntryAura(std::numeric_limits<float>::quiet_NaN(), 25.59f, 954.5942993f),
        "Gate 3 entry aura admits its source square and excludes the respawn aura, lower floor and nonfinite input");
    tests.Require(Is_KoukuGate3EntryTerrace(-17.509552f, 25.6f, 960.53015f) &&
        !Is_KoukuGate3EntryTerrace(-17.509552f, 1.32f, 960.53015f) &&
        !Is_KoukuGate3EntryTerrace(-31.f, 25.6f, 960.53015f) &&
        !Is_KoukuGate3EntryTerrace(std::numeric_limits<float>::quiet_NaN(), 25.6f, 960.53015f),
        "Gate 3 entry deck predicate admits the authored arrival and excludes the lower floor, outside and nonfinite input");
    C2S_GATE_PROGRESS_PROPOSE wire; wire.iRequestSequence = 1u;
    wire.eWorldId = WORLD_ID::KAKULSAYDON_ARENA; wire.eKind = GATE_PROGRESS_KIND::ENTER_GATE3;
    CPacketWriter writer; tests.Require(Write_Message(writer, wire), "Explicit Gate 3 entry writes the existing gate proposal packet");
    CPacketReader reader{writer.Get_Buffer()}; C2S_GATE_PROGRESS_PROPOSE decoded;
    tests.Require(Read_Message(reader, decoded) && decoded.eKind == GATE_PROGRESS_KIND::ENTER_GATE3,
        "Explicit Gate 3 entry round-trips through the production packet codec");
    auto invalidBytes = writer.Get_Buffer(); invalidBytes.back() = static_cast<std::uint8_t>(GATE_PROGRESS_KIND::END);
    CPacketReader invalidReader{invalidBytes};
    tests.Require(!Read_Message(invalidReader, decoded), "Unknown gate proposal kind remains rejected");
    S2C_GATE_PROGRESS_STATE state; state.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
    state.iGateCount = 4u; state.eKind = GATE_PROGRESS_KIND::ENTER_GATE3;
    CPacketWriter stateWriter; tests.Require(Write_Message(stateWriter, state), "Gate 3 entry vote state encodes");
    CPacketReader stateReader{stateWriter.Get_Buffer()}; S2C_GATE_PROGRESS_STATE decodedState;
    tests.Require(Read_Message(stateReader, decodedState) && decodedState.eKind == GATE_PROGRESS_KIND::ENTER_GATE3,
        "Gate 3 entry vote intent survives the state packet");

    for (unsigned count = 1u; count <= 4u; ++count)
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Gate 3 entry fixture loads the actual catalog, world and navigation");
        if (!room->Is_Ready()) continue;
        std::vector<std::shared_ptr<CClientSession>> sessions;
        for (unsigned id = 1u; id <= count + 1u; ++id)
        {
            auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = 500u + id;
            player.iNetEntityId = 100u + id; player.iCurrentHp = player.iMaximumHp = 100u;
            player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; player.isCombatReady = true;
            player.fPositionX = -17.5f - .3f * id; player.fPositionY = 25.6f; player.fPositionZ = 960.5f;
            if (id > count) { player.fPositionX = 20.f; player.fPositionZ = 970.f; }
            room->m_PlayerIdBySessionId[player.iSessionId] = id;
            auto connection = std::make_shared<CClientSession>(player.iSessionId, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            connection->m_isSendRunning.store(true); room->m_Sessions.emplace(player.iSessionId, connection);
            sessions.push_back(std::move(connection));
            if (id <= count) { room->m_PartyIdByPlayerId[id] = 77u; room->m_PartyMembersByPartyId[77u].push_back(id); }
        }
        tests.Require(room->Spawn_GatePlacement("boss.kakulsaydon.g2.big-saydon"), "Preservation fixture has an existing disabled-placement boss");
        room->m_GateProgress.iCurrentGate = count % 2u;
        const auto previousGate = room->m_GateProgress.iCurrentGate;
        const auto previousPlayers = room->m_Players;
        const auto previousEntities = room->m_WorldEntities;
        unsigned sequence = 1u;
        const auto propose = [&](unsigned id = 1u) {
            auto request = wire; request.iRequestSequence = sequence++;
            room->Handle_GateProgressPropose(500u + id, request);
        };
        const auto respond = [&](unsigned id, bool accepted) {
            C2S_GATE_PROGRESS_RESPOND response; response.iRequestSequence = sequence++;
            response.iProposalId = room->m_GateProgress.iProposalId; response.bAccepted = accepted;
            room->Handle_GateProgressRespond(500u + id, response);
        };
        const auto acceptAll = [&]() { for (unsigned id = 2u; id <= count; ++id) respond(id, true); };
        const auto preserved = [&]() {
            tests.Require(!room->m_GateProgress.iProposalId && room->m_GateProgress.iCurrentGate == previousGate &&
                room->m_WorldEntities.size() == previousEntities.size(), "Rejected entry preserves current gate and existing boss count");
            for (std::size_t index = 0; index < previousEntities.size() && index < room->m_WorldEntities.size(); ++index)
                tests.Require(room->m_WorldEntities[index].iNetEntityId == previousEntities[index].iNetEntityId &&
                    room->m_WorldEntities[index].iCurrentHp == previousEntities[index].iCurrentHp &&
                    room->m_WorldEntities[index].fPositionX == previousEntities[index].fPositionX,
                    "Rejected entry preserves the exact previous boss identity, HP and position");
            for (const auto& [id, player] : room->m_Players)
            {
                const auto& old = previousPlayers.at(id);
                tests.Require(player.fPositionX == old.fPositionX && player.fPositionY == old.fPositionY && player.fPositionZ == old.fPositionZ,
                    "Rejected entry preserves participant and observer positions");
            }
        };
        if (count > 1u) { propose(count); preserved(); }
        room->m_Players.at(1u).fPositionY = 1.32f; propose();
        room->m_Players.at(1u).fPositionY = 25.6f; preserved();
        room->m_Players.at(count).bPatternBound = true; propose(); acceptAll();
        room->m_Players.at(count).bPatternBound = false; preserved();
        const auto nextId = room->m_iNextNetEntityId; room->m_iNextNetEntityId = INVALID_NET_ENTITY_ID;
        propose(); acceptAll(); room->m_iNextNetEntityId = nextId; preserved();
        const auto navigation = room->m_ServerNavigation; room->m_ServerNavigation = {};
        propose(); acceptAll(); room->m_ServerNavigation = navigation; preserved();
        auto* gatePlacement = const_cast<WORLD_BOOTSTRAP_PLACEMENT*>(room->Find_Placement("boss.kakulsaydon.g3.saydon"));
        tests.Require(gatePlacement != nullptr, "Gate 3 staged boss has an authored placement");
        if (gatePlacement)
        {
            const auto archetype = gatePlacement->strArchetypeId; gatePlacement->strArchetypeId = "invalid.gate3.contract";
            propose(); acceptAll(); gatePlacement->strArchetypeId = archetype; preserved();
        }
        if (count > 1u)
        {
            propose(); tests.Require(room->m_GateProgress.Voters.size() == count, "Gate 3 vote contains the existing party only");
            respond(2u, false); preserved();
            propose(); room->m_iServerTick = room->m_GateProgress.iDeadlineTick; room->Expire_GateProgressVote(); preserved();
            propose(); room->m_Players.at(1u).fPositionY = 1.32f; acceptAll();
            room->m_Players.at(1u).fPositionY = 25.6f; preserved();
            propose(); std::swap(room->m_PartyMembersByPartyId[77u].front(), room->m_PartyMembersByPartyId[77u].back());
            acceptAll(); std::swap(room->m_PartyMembersByPartyId[77u].front(), room->m_PartyMembersByPartyId[77u].back()); preserved();
            propose(); room->m_PartyMembersByPartyId[77u].push_back(count + 1u); acceptAll();
            room->m_PartyMembersByPartyId[77u].pop_back(); preserved();
        }
        propose();
        if (count > 1u)
        {
            tests.Require(room->m_GateProgress.iCurrentGate == previousGate && room->m_WorldEntities.size() == previousEntities.size(),
                "Partial approval cannot despawn or teleport the party");
            acceptAll();
        }
        tests.Require(!room->m_GateProgress.iProposalId && room->m_GateProgress.iCurrentGate == 3u && !room->Is_KoukuRaidRunning(),
            "Explicit non-raid entry targets Gate 3 regardless of prior Gate 0 or Gate 1");
        tests.Require(std::any_of(room->m_WorldEntities.begin(), room->m_WorldEntities.end(), [](const auto& entity) {
            return entity.strPlacementId == "boss.kakulsaydon.g3.saydon"; }), "Prebuilt Gate 3 boss commits through the existing spawn consumer");
        for (unsigned id = 1u; id <= count; ++id)
        {
            const auto& player = room->m_Players.at(id);
            tests.Require(std::abs(player.fPositionX + 2.45f) < .01f && std::abs(player.fPositionZ - 945.17f) < .01f,
                "Every consenting party participant reaches the original Gate 3 combat spawn");
        }
        const auto& observer = room->m_Players.at(count + 1u);
        tests.Require(observer.fPositionX == previousPlayers.at(count + 1u).fPositionX &&
            observer.fPositionZ == previousPlayers.at(count + 1u).fPositionZ, "Unrelated room observer is never moved by the party vote");
        const auto committedId = room->m_iNextNetEntityId; propose();
        tests.Require(!room->m_GateProgress.iProposalId && room->m_iNextNetEntityId == committedId,
            "A duplicate entry command from the combat floor cannot restart the boss");
    }
#endif
}

void CServerGameplayContractRunner::Run_KoukuRaidIntegration(TESTS& tests)
{
    const auto begin = [&](CGameRoom& room, const SESSION_ID session, const C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request) {
#ifdef _DEBUG
        room.Handle_KoukuRaidRequest(session, request);
#else
        // The Release entry collider calls this same admission consumer. Debug START stays rejected.
        std::string reason;
        tests.Require(room.Begin_KoukuRaidPreparation(session, request, reason), "Release entry admission pins the actual published raid");
#endif
    };
    const auto verifyBingoFlow = [&](CGameRoom& room, unsigned firstTick, bool verifySpecial) {
        auto& run = room.m_KoukuRaid;
        const auto* gate = run.pCatalog ? run.pCatalog->Find_KoukuRaidGate("BINGO") : nullptr;
        if (!gate || gate->Entries.empty()) { tests.Require(false, "Bingo has its saved independent entries"); return; }
        const auto loop = std::find_if(gate->Entries.begin(), gate->Entries.end(),
            [&](const auto& entry) { return entry.strEntryId == gate->strLoopStartEntryId; });
        const bool savedFlowValid = gate->Entries.size() == 35u && loop != gate->Entries.end();
        tests.Require(savedFlowValid, "Bingo publishes 35 independent entries with its exact stable loop start");
        if (!savedFlowValid) return;
        std::vector<std::string> expected;
        bool independent = true;
        for (const auto& entry : gate->Entries)
        {
            std::string status;
            const auto* pattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*run.pCatalog, entry.strTargetId, status);
            independent = independent && !entry.bBundle && pattern && pattern->ParentChildren.empty();
            expected.push_back(entry.strEntryId);
        }
        for (auto entry = loop; entry != gate->Entries.end(); ++entry) expected.push_back(entry->strEntryId);
        const auto laser = [](const auto& entry) { return entry.strTargetId == "KAKULSAYDON_G1_PATTERN_123"; };
        tests.Require(independent && std::count_if(gate->Entries.begin(), loop, laser) == 1 &&
            std::count_if(loop, gate->Entries.end(), laser) == 1,
            "Bingo normal flow has independent patterns and one laser in both its prefix and repeated tail");
        const auto raidEpoch = run.State.iRunEpoch;
        NET_ENTITY_ID boardOwner = INVALID_NET_ENTITY_ID;
        unsigned boardEpoch = 0u, previousRequest = 0u;
        bool stableOwner = true, exactRequests = true;
        std::vector<std::string> completed;
        unsigned tick = firstTick, failedTick = 0u;
        std::string advanceFailure;
        const auto rejectAdvance = [&](const char* reason) {
            failedTick = tick; advanceFailure = reason; return false; };
        const auto advance = [&](bool normalOnly) {
            room.m_iServerTick = tick;
            room.m_TickDamageEvents.clear();
            // These scheduler fixtures isolate progression from ordinary hit/hammer attrition.
            // Failed Bingo judgement still bypasses this protection and must fail the fixture.
            for (auto& [id, player] : room.m_Players)
            {
                if (!player.iCurrentHp || player.eAction == PLAYER_ACTION_STATE::DEAD) return rejectAdvance("player already dead");
                player.iCurrentHp = player.iMaximumHp = 100000000u;
                player.iInvulnerableEndTick = tick + 60000u;
            }
            if (room.m_KoukuBingoDuration.iOwnerId)
            {
                room.m_KoukuBingoDuration.iNextHammerTick = tick + 60000u;
                if (normalOnly) room.m_KoukuBingoDuration.iNextBombTick = tick + 60000u;
            }
            room.Prepare_KoukuAuditionTick(tick);
            room.Update_KoukuPatternTails(tick);
            room.Update_KoukuBingo(tick);
            auto* boss = room.Find_KoukuSaydonArenaBoss("boss.kakulsaydon.bingo.saydon", "BOSS_KAKULSAYDON_BINGO_SAYDON");
            if (!boss) return rejectAdvance("Bingo boss missing");
            boss->iCurrentHp = boss->iMaximumHp;
            if (!room.Update_KoukuSaydonBoss(*boss, tick)) return rejectAdvance("boss update rejected");
            room.Commit_KoukuMechanicTriggers(tick);
            if (normalOnly && run.bEntryRunning &&
                room.m_KoukuSaydonPatternAudition.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE)
            {
                const auto receipt = room.m_KoukuSaydonPatternAuditionReceiptBySessionId.find(run.iOwnerSessionId);
                if (receipt == room.m_KoukuSaydonPatternAuditionReceiptBySessionId.end() || !receipt->second.LastLifecycle ||
                    receipt->second.LastLifecycle->eState != KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED)
                    return rejectAdvance("normal entry has no completed receipt");
                completed.push_back(run.State.strFlowEntryId);
            }
            room.Update_KoukuRaid(tick);
            if (run.State.iRunEpoch != raidEpoch || run.State.strGateId != "BINGO" ||
                run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT) return rejectAdvance("raid epoch, gate or phase changed");
            if (!boardOwner && room.m_KoukuBingoDuration.iOwnerId)
            { boardOwner = room.m_KoukuBingoDuration.iOwnerId; boardEpoch = room.m_iKoukuBingoBoardEpoch; }
            stableOwner = stableOwner && boardOwner && room.m_KoukuBingoDuration.iOwnerId == boardOwner &&
                room.m_iKoukuBingoBoardEpoch == boardEpoch && boardEpoch == raidEpoch &&
                room.m_KoukuBingoDuration.bEncounterOwned && !room.m_KoukuBingoDuration.iEndTick;
            if (run.iAuditionRequestSequence != previousRequest)
            {
                exactRequests = exactRequests && run.iAuditionRequestSequence > previousRequest;
                previousRequest = run.iAuditionRequestSequence;
                if (!run.bBingoSpecialRunning)
                    exactRequests = exactRequests && run.State.iFlowEntryIndex < gate->Entries.size() &&
                        run.State.strFlowEntryId == gate->Entries[run.State.iFlowEntryIndex].strEntryId &&
                        room.m_KoukuSaydonPatternAudition.Request.strPatternId == gate->Entries[run.State.iFlowEntryIndex].strTargetId;
            }
            // Fake connections have no send worker; drain their frames like an acknowledging client.
            for (auto& [id, weakSession] : room.m_Sessions)
                if (const auto session = weakSession.lock())
                { session->m_OutboundFrames.clear(); session->m_iQueuedOutboundBytes = 0u; }
            if (!room.Is_Ready() || !std::all_of(room.m_Players.begin(), room.m_Players.end(),
                [](const auto& pair) { return pair.second.iCurrentHp && pair.second.eAction != PLAYER_ACTION_STATE::DEAD; }))
                return rejectAdvance("room failed or a player died during this tick");
            return true;
        };
        bool advanced = true;
        for (; tick < firstTick + 24000u && completed.size() < expected.size() && advanced; ++tick)
            advanced = advance(true);
        tests.Require(advanced && completed == expected && stableOwner && exactRequests,
            "Actual Bingo flow completes its saved independent prefix and one full repeated tail while retaining raid and board ownership");
        if (!advanced || completed != expected || !stableOwner || !exactRequests)
            std::cout << "[RAID BINGO FLOW] completed=" << completed.size() << "/" << expected.size()
                << " entry=" << run.State.strFlowEntryId << " phase=" << unsigned(run.State.ePhase)
                << " failedTick=" << failedTick << " cause=" << advanceFailure
                << " reason=" << run.State.strReason << " status=" << room.m_strStatus << '\n';
        if (!verifySpecial || !advanced || completed != expected) return;

        std::string status;
        const auto* special = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*run.pCatalog,
            gate->strBingoSpecialPatternId, status);
        // This authored Parent uses playChildrenSequentially=false. Publication flattens
        // its children into ten stages sharing one clock; runtime ParentChildren is empty.
        const std::array<unsigned, 10u> expectedDurations{5400u, 867u, 2000u, 2167u, 1394u,
            13000u, 200u, 1167u, 5100u, 1333u};
        bool flattened = special && special->ParentChildren.empty() && special->bFixedTimelineClock &&
            special->Stages.size() == expectedDurations.size();
        unsigned stageDurationMs = 0u;
        if (flattened)
            for (unsigned i = 0u; i < expectedDurations.size(); ++i)
            {
                stageDurationMs += special->Stages[i].iDurationMs;
                flattened = flattened && special->Stages[i].iDurationMs == expectedDurations[i];
            }
        // An absent optional tail duration uses the stage sum in the published ABI.
        flattened = flattened && (std::max)(stageDurationMs, special->iTimelineDurationMs) == 32628u;
        const auto detonates = [](const auto& trigger) {
            return trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_DETONATION; };
        flattened = flattened && std::count_if(special->MechanicTriggers.begin(), special->MechanicTriggers.end(), detonates) == 1 &&
            std::any_of(special->MechanicTriggers.begin(), special->MechanicTriggers.end(), [&](const auto& trigger) {
                return detonates(trigger) && trigger.iStartMs == 24828u && trigger.iDurationMs == 34u; }) &&
            std::any_of(special->MechanicTriggers.begin(), special->MechanicTriggers.end(), [](const auto& trigger) {
                return trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER && !trigger.iStartMs; }) &&
            std::any_of(special->LogicWindows.begin(), special->LogicWindows.end(), [](const auto& window) {
                return window.eKind == BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS && window.iStartMs == 8567u && window.iDurationMs == 500u; }) &&
            std::any_of(special->LogicWindows.begin(), special->LogicWindows.end(), [](const auto& window) {
                return window.eKind == BOSS_PATTERN_LOGIC_KIND::BINGO_COMPLETED_LINES && !window.iStartMs &&
                    window.iDurationMs == 32628u && window.iThreshold == 3u &&
                    std::any_of(window.OnSuccess.begin(), window.OnSuccess.end(), [](const auto& result) {
                        return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY && result.iDurationMs == 30000u; }); });
        tests.Require(flattened,
            "The explicit Bingo special publishes flattened preparation/Medusa/13s hold stages, detonation at 24828ms and a 32628ms line window");
        if (!flattened) return;
        // Exercise rejected special admission while the next normal pattern is active.
        // Only the request's expected revision is invalid; every live owner must survive.
        advanced = advance(false); ++tick;
        tests.Require(advanced && room.m_KoukuSaydonPatternAudition.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE,
            "The special preflight fixture retains an active normal Bingo entry");
        if (!advanced) return;
        const auto liveBefore = room.m_WorldEntities;
        const auto boardWhite = room.m_KoukuBingo.Get_WhiteMask(), boardRed = room.m_KoukuBingo.Get_RedMask();
        const auto durationBefore = room.m_KoukuBingoDuration;
        const auto auditionEpochBefore = run.iAuditionEpoch, requestBefore = run.iAuditionRequestSequence;
        const auto flowIndexBefore = run.State.iFlowEntryIndex;
        const auto flowEntryBefore = run.State.strFlowEntryId;
        const auto normalPatternBefore = room.m_KoukuSaydonPatternAudition.Request.strPatternId;
        const auto nextEpochBefore = room.m_iNextKoukuSaydonPatternAuditionEpoch;
        const auto receiptBefore = room.m_KoukuSaydonPatternAuditionReceiptBySessionId.at(run.iOwnerSessionId);
        const auto sourceBefore = run.State.iActionSourceRevision;
        ++run.State.iActionSourceRevision;
        const bool rejected = !room.Start_KoukuBingoSpecialPattern(tick);
        run.State.iActionSourceRevision = sourceBefore;
        const bool livePreserved = room.m_WorldEntities.size() == liveBefore.size() &&
            std::all_of(liveBefore.begin(), liveBefore.end(), [&](const auto& old) {
                const auto current = std::find_if(room.m_WorldEntities.begin(), room.m_WorldEntities.end(),
                    [&](const auto& entity) { return entity.iNetEntityId == old.iNetEntityId; });
                return current != room.m_WorldEntities.end() && current->strPatternId == old.strPatternId &&
                    current->iPatternSequence == old.iPatternSequence && current->iPatternStageIndex == old.iPatternStageIndex &&
                    current->iPatternStartTick == old.iPatternStartTick && current->iActionStartTick == old.iActionStartTick &&
                    current->iCurrentHp == old.iCurrentHp && current->eAction == old.eAction &&
                    current->fPositionX == old.fPositionX && current->fPositionY == old.fPositionY &&
                    current->fPositionZ == old.fPositionZ && current->fYawDegrees == old.fYawDegrees;
            });
        const auto& receiptAfter = room.m_KoukuSaydonPatternAuditionReceiptBySessionId.at(run.iOwnerSessionId);
        const bool lifecyclePreserved = receiptAfter.LastLifecycle.has_value() == receiptBefore.LastLifecycle.has_value() &&
            (!receiptBefore.LastLifecycle || (receiptAfter.LastLifecycle->iRoomAuditionEpoch == receiptBefore.LastLifecycle->iRoomAuditionEpoch &&
                receiptAfter.LastLifecycle->iPatternSequence == receiptBefore.LastLifecycle->iPatternSequence &&
                receiptAfter.LastLifecycle->eState == receiptBefore.LastLifecycle->eState));
        tests.Require(rejected && livePreserved && !run.bBingoSpecialRunning && run.bEntryRunning &&
            run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && run.State.iRunEpoch == raidEpoch &&
            run.iAuditionEpoch == auditionEpochBefore && run.iAuditionRequestSequence == requestBefore &&
            run.State.iFlowEntryIndex == flowIndexBefore && run.State.strFlowEntryId == flowEntryBefore &&
            room.m_KoukuSaydonPatternAudition.Request.strPatternId == normalPatternBefore && lifecyclePreserved &&
            room.m_iNextKoukuSaydonPatternAuditionEpoch == nextEpochBefore &&
            receiptAfter.Request.iRequestSequence == receiptBefore.Request.iRequestSequence &&
            receiptAfter.Result.eResult == receiptBefore.Result.eResult &&
            room.m_KoukuBingo.Get_WhiteMask() == boardWhite && room.m_KoukuBingo.Get_RedMask() == boardRed &&
            room.m_iKoukuBingoBoardEpoch == boardEpoch &&
            room.m_KoukuBingoDuration.iOwnerId == durationBefore.iOwnerId &&
            room.m_KoukuBingoDuration.iNextBombTick == durationBefore.iNextBombTick &&
            room.m_KoukuBingoDuration.iMarkedBombCount == durationBefore.iMarkedBombCount,
            "Rejected special preflight preserves every live actor, normal occurrence, receipt and encounter-owned board without consuming an epoch");
        // Use the real 30s first mark / 20s repeat clock twice. The board's three
        // completed rows provide the actual success input for the detonation, not a forced verdict.
        room.m_KoukuBingo.Fill(0x7fffu);
        room.m_KoukuBingoDuration.iNextBombTick = tick + CKoukuSaydonLogicRuntime::Ticks_FromMs(
            KOUKU_BINGO_BOMB_INITIAL_DELAY_MS + KOUKU_BINGO_BOMB_INTERVAL_MS);
        const auto firstMarkCount = room.m_KoukuBingoDuration.iMarkedBombCount;
        unsigned starts = 0u, resumes = 0u, interruptedIndex = 0u;
        std::string interruptedEntry;
        bool waitingForResume = false, specialValid = firstMarkCount == 0u;
        std::string specialFailure = specialValid ? "" : "nonzero initial mark count";
        const auto noteSpecialFailure = [&](const char* reason) {
            if (!specialValid && specialFailure.empty()) { specialFailure = reason; failedTick = tick; } };
        std::vector<unsigned> observedStages;
        unsigned specialStartTick = 0u, detonations = 0u;
        const auto specialDeadline = tick + 6000u;
        for (; tick < specialDeadline && resumes < 2u && advanced; ++tick)
        {
            const bool wasSpecial = run.bBingoSpecialRunning;
            const auto beforeEntry = run.State.strFlowEntryId;
            const auto beforeIndex = run.State.iFlowEntryIndex;
            advanced = advance(false);
            if (!wasSpecial && run.bBingoSpecialRunning)
            {
                ++starts;
                specialValid = specialValid && !waitingForResume && room.m_KoukuBingoDuration.iMarkedBombCount == starts * 3u &&
                    run.State.iFlowEntryIndex == beforeIndex && run.State.strFlowEntryId == beforeEntry &&
                    room.m_KoukuSaydonPatternAudition.Request.strPatternId == special->strPatternId;
                noteSpecialFailure("third-mark insertion changed the cursor or selected the wrong request");
                interruptedIndex = beforeIndex; interruptedEntry = beforeEntry;
                observedStages.clear(); specialStartTick = 0u; detonations = 0u;
            }
            else if (wasSpecial && !run.bBingoSpecialRunning)
            {
                specialValid = specialValid && specialStartTick && observedStages.size() == expectedDurations.size() &&
                    detonations == 1u && tick - specialStartTick == CKoukuSaydonLogicRuntime::Ticks_FromMs(32628u) &&
                    run.State.iFlowEntryIndex == interruptedIndex && run.State.strFlowEntryId == interruptedEntry;
                noteSpecialFailure("special completion duration, stage count, detonation count or cursor mismatch");
                waitingForResume = true;
            }
            if (run.bBingoSpecialRunning)
            {
                const auto* boss = room.Find_KoukuSaydonArenaBoss("boss.kakulsaydon.bingo.saydon", "BOSS_KAKULSAYDON_BINGO_SAYDON");
                if (boss && boss->strPatternId == special->strPatternId)
                {
                    if (!specialStartTick) specialStartTick = boss->iPatternStartTick;
                    const auto elapsed = tick - specialStartTick;
                    unsigned selected = 0u, startMs = 0u;
                    while (selected < expectedDurations.size() &&
                        elapsed >= CKoukuSaydonLogicRuntime::Ticks_FromMs(startMs + expectedDurations[selected]))
                        startMs += expectedDurations[selected++];
                    specialValid = specialValid && selected < expectedDurations.size() &&
                        boss->iPatternStartTick == specialStartTick && boss->iPatternStageIndex == selected &&
                        boss->iActionStartTick == specialStartTick + CKoukuSaydonLogicRuntime::Ticks_FromMs(startMs);
                    noteSpecialFailure("flattened stage index or action clock mismatch");
                    if (observedStages.empty() || observedStages.back() != selected)
                    {
                        specialValid = specialValid && selected == observedStages.size() &&
                            elapsed == CKoukuSaydonLogicRuntime::Ticks_FromMs(startMs);
                        noteSpecialFailure("flattened stage boundary order or tick mismatch");
                        observedStages.push_back(selected);
                    }
                    const auto damage = std::count_if(room.m_TickDamageEvents.begin(), room.m_TickDamageEvents.end(), [&](const auto& event) {
                        return boss->iMaximumHealthBars && event.isOutgoing && event.iTargetNetEntityId == boss->iNetEntityId &&
                            event.iAmount == std::uint64_t(boss->iMaximumHp) * 13u / boss->iMaximumHealthBars; });
                    const bool explosionTick = elapsed == CKoukuSaydonLogicRuntime::Ticks_FromMs(24828u);
                    specialValid = specialValid && damage == (explosionTick ? 1 : 0);
                    noteSpecialFailure("detonation tick or thirteen-bar damage count mismatch");
                    detonations += static_cast<unsigned>(damage);
                }
            }
            if (waitingForResume && !run.bBingoSpecialRunning && run.bEntryRunning)
            {
                specialValid = specialValid && run.State.iFlowEntryIndex == interruptedIndex &&
                    run.State.strFlowEntryId == interruptedEntry &&
                    room.m_KoukuSaydonPatternAudition.Request.strPatternId == gate->Entries[interruptedIndex].strTargetId;
                noteSpecialFailure("normal flow did not resume its interrupted stable entry");
                ++resumes; waitingForResume = false;
            }
        }
        tests.Require(advanced && specialValid && starts == 2u && resumes == 2u && stableOwner && exactRequests,
            "Every third real Bingo mark executes the flattened special on exact stage/detonation clocks and resumes the interrupted stable entry without resetting the raid or board");
        if (!advanced || !specialValid || starts != 2u || resumes != 2u || !stableOwner || !exactRequests)
            std::cout << "[RAID BINGO SPECIAL] marks=" << room.m_KoukuBingoDuration.iMarkedBombCount
                << " starts=" << starts << " resumes=" << resumes << " advanced=" << advanced << " valid=" << specialValid
                << " stableOwner=" << stableOwner << " exactRequests=" << exactRequests << " phase=" << unsigned(run.State.ePhase)
                << " tick=" << tick << " failedTick=" << failedTick << " specialStart=" << specialStartTick
                << " stages=" << observedStages.size() << " detonations=" << detonations
                << " advanceCause=" << advanceFailure << " specialCause=" << specialFailure
                << " reason=" << run.State.strReason << " status=" << room.m_strStatus << '\n';
    };
    Run_KoukuGate3Entry(tests);
    CGameplayCatalog catalog;
    tests.Require(catalog.Load(), "Raid integration loads the published candidate catalog");
    // Use the actual Release trigger and gate-vote consumers, not a Debug START shortcut.
    for (unsigned participantCount = 2u; participantCount <= 4u; ++participantCount)
    for (unsigned scenario = 0u; scenario < 2u; ++scenario)
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Party product admission loads the published world and raid");
        if (!room->Is_Ready()) continue;
        std::vector<std::shared_ptr<CClientSession>> sessions;
        for (unsigned id = 1u; id <= participantCount; ++id)
        {
            auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = 500u + id;
            player.iNetEntityId = 100u + id; player.iCurrentHp = player.iMaximumHp = 100u;
            player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; player.isCombatReady = true;
            player.fPositionX = id == 1u ? 27.8f : 3.29f;
            player.fPositionY = id == 1u ? .45f : 8.64f;
            player.fPositionZ = id == 1u ? -67.7f : -10.69f;
            room->m_PlayerIdBySessionId[player.iSessionId] = id;
            room->m_PartyIdByPlayerId[id] = 77u; room->m_PartyMembersByPartyId[77u].push_back(id);
            auto connection = std::make_shared<CClientSession>(player.iSessionId, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            connection->m_isSendRunning.store(true); room->m_Sessions.emplace(player.iSessionId, connection);
            sessions.push_back(std::move(connection));
        }
        auto& run = room->m_KoukuRaid;
        if (participantCount == 2u && scenario == 0u)
        {
            const auto* publishedGate = catalog.Find_KoukuRaidGate("GATE1");
            tests.Require(publishedGate != nullptr, "Revision rejection fixture uses the actual published G1 raid");
            if (publishedGate)
            {
                const auto source = CKoukuSaydonBrain::Resolve_ProductSourceRevision(catalog);
                C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST stale;
                stale.iRequestSequence = 1u; stale.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
                stale.eOperation = KOUKUSAYDON_RAID_OPERATION::START; stale.strStartGateId = "GATE1";
                stale.ExpectedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
                stale.iActionSourceRevision = source + 1u; stale.iSequenceSourceRevision = publishedGate->iSequenceRevision;
                const auto nextEpoch = room->m_iNextKoukuRaidEpoch;
                const auto priorProduct = room->m_pKoukuPublishedProductGeneration;
                const auto priorBossCount = room->m_WorldEntities.size();
                std::string reason;
                tests.Require(!room->Begin_KoukuRaidPreparation(501u, stale, reason) &&
                    reason.find("Action revision mismatch") != std::string::npos &&
                    reason.find("requested " + std::to_string(source + 1u)) != std::string::npos &&
                    reason.find("published " + std::to_string(source)) != std::string::npos,
                    "Raid rejects an unpublished Action revision with its exact requested and published values");
                stale.iActionSourceRevision = source; ++stale.iSequenceSourceRevision;
                tests.Require(!room->Begin_KoukuRaidPreparation(501u, stale, reason) &&
                    reason.find("Sequence revision mismatch") != std::string::npos &&
                    reason.find("requested " + std::to_string(stale.iSequenceSourceRevision)) != std::string::npos &&
                    reason.find("published " + std::to_string(publishedGate->iSequenceRevision)) != std::string::npos,
                    "Raid rejects an unpublished Sequence revision with its exact requested and published values");
                tests.Require(!run.State.iRunEpoch && room->m_iNextKoukuRaidEpoch == nextEpoch &&
                    room->m_pKoukuPublishedProductGeneration == priorProduct && room->m_WorldEntities.size() == priorBossCount &&
                    room->m_Players.at(1u).fPositionX == 27.8f && room->m_Players.at(1u).iCurrentHp == 100u,
                    "Revision rejection preserves the party, bosses, catalog and next epoch for the following exact-source retry");
            }
        }
        if (scenario == 0u)
        {
            std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
            std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
            const auto enter = [&](unsigned tick) {
                room->m_iServerTick = tick;
                room->m_ServerTriggerSystem.Evaluate_Entries(room->m_Players, tick, transfers,
                    [&](WORLD_TRIGGER_ACTION_KIND kind, const std::string& target) {
                        return room->Activate_TriggerTarget(kind, target);
                    }, prompts);
            };
            room->m_Players.at(participantCount).TriggerMove.isActive = true;
            const auto unusedEpoch = room->m_iNextKoukuRaidEpoch;
            enter(100u);
            tests.Require(room->m_iNextKoukuRaidEpoch == unusedEpoch, "Temporary contact rejection preserves the next raid epoch");
            tests.Require(!run.State.iRunEpoch && room->m_strStatus == "All room participants must be alive and outside a transfer",
                "A companion still in a G transfer temporarily rejects the actual G1 entry trigger");
            room->m_Players.at(participantCount).TriggerMove.isActive = false;
            enter(101u);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.PlayerIds.size() == participantCount &&
                run.strEntryTriggerSequenceId == "world.sequence.instance.circusfinale",
                "G1 retries the same contact after the companion lands without leaving and re-entering");
            const auto epoch = run.State.iRunEpoch;
            enter(102u);
            tests.Require(run.State.iRunEpoch == epoch, "A successful entry contact cannot start a second raid epoch");
        }
        else
        {
            room->m_GateProgress.iCurrentGate = 3u; room->m_GateProgress.iClearedMask = 7u;
            C2S_GATE_PROGRESS_PROPOSE proposal; proposal.iRequestSequence = 1u;
            proposal.eWorldId = WORLD_ID::KAKULSAYDON_ARENA; proposal.eKind = GATE_PROGRESS_KIND::ADVANCE;
            room->Handle_GateProgressPropose(501u, proposal);
            tests.Require(run.State.iRunEpoch == 0u && room->m_GateProgress.Voters.size() == participantCount,
                "The legacy Bingo UI waits for all party voters before resource admission");
            C2S_GATE_PROGRESS_RESPOND response; response.iRequestSequence = 1u;
            response.iProposalId = room->m_GateProgress.iProposalId; response.bAccepted = true;
            for (unsigned id = 2u; id <= participantCount; ++id)
            {
                room->Handle_GateProgressRespond(500u + id, response);
                if (id < participantCount) tests.Require(run.State.iRunEpoch == 0u,
                    "Partial Release UI approval cannot admit Bingo");
            }
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.bGateVoteEntry &&
                run.State.strGateId == "BINGO" && room->m_GateProgress.iCurrentGate == 3u &&
                !room->Find_KoukuSaydonArenaBoss("boss.kakulsaydon.bingo.saydon", "BOSS_KAKULSAYDON_BINGO_SAYDON"),
                "Release Bingo UI admits the saved intro/Flow owner instead of an idle spawned boss");
        }
        if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::PREPARING) continue;
        auto ack = run.Request; ack.eOperation = KOUKUSAYDON_RAID_OPERATION::READY;
        ack.iExpectedRunEpoch = run.State.iRunEpoch; ack.iRequestSequence += 1u;
        std::string reason;
        tests.Require(room->Apply_KoukuRaidReadiness(501u, ack, reason), "First participant pins the current Action/Sequence product");
        room->m_iServerTick = 103u; room->Update_KoukuRaid(103u);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.State.iReadyMask == 1u,
            "One ready client cannot start the party cinematic");
        for (unsigned id = 2u; id <= participantCount; ++id)
        {
            tests.Require(room->Apply_KoukuRaidReadiness(500u + id, ack, reason), "Each participant pins the same Action/Sequence product");
            if (id < participantCount)
            {
                room->Update_KoukuRaid(103u);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING,
                    "The complete two-to-four-client ready mask is required before a cinematic");
            }
        }
        room->m_iServerTick = 104u; room->Update_KoukuRaid(104u);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.State.iReadyMask == (1u << participantCount) - 1u &&
            !run.State.strSequencePatternId.empty(), "All Release clients enter one authoritative cinematic after READY");
        if (scenario == 1u)
        {
            room->m_iServerTick = run.State.iEndTick; room->Update_KoukuRaid(room->m_iServerTick);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && run.bEntryRunning &&
                run.State.strGateId == "BINGO" && run.iAuditionEpoch != 0u,
                "Bingo UI intro completion immediately queues its first published independent entry");
            const auto common = room->m_KoukuSaydonPatternAudition.iCommonStartTick;
            room->m_iServerTick = common; room->Prepare_KoukuAuditionTick(common);
            const auto* boss = room->Find_KoukuSaydonArenaBoss("boss.kakulsaydon.bingo.saydon", "BOSS_KAKULSAYDON_BINGO_SAYDON");
            tests.Require(boss && !boss->strPatternId.empty() &&
                room->m_KoukuSaydonPatternAudition.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE,
                "The real Bingo scheduler starts its first independent pattern at the common Server tick in Release");
            if (participantCount == 4u)
                verifyBingoFlow(*room, common, true);

        }
    }

    // A standalone boss clear/restart must enter the same pinned Sequence owner as the entry collider.
    // Resource preparation is transactional: neither the unanimous vote nor partial READY moves a player.
    for (unsigned participantCount = 2u; participantCount <= 4u; ++participantCount)
    for (unsigned transition = 0u; transition < 5u; ++transition)
    for (const bool failPreparation : { false, true })
    {
        const bool advance = transition < 2u;
        const std::uint8_t sourceGate = static_cast<std::uint8_t>(advance ? transition + 1u : transition - 1u);
        const std::string sourceGateId = "GATE" + std::to_string(sourceGate);
        const std::string targetGateId = "GATE" + std::to_string(advance ? sourceGate + 1u : sourceGate);
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Standalone gate transition loads the actual published world and raid");
        if (!room->Is_Ready()) continue;
        std::vector<std::shared_ptr<CClientSession>> sessions;
        for (unsigned id = 1u; id <= participantCount; ++id)
        {
            auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = 500u + id;
            player.iNetEntityId = 100u + id; player.iCurrentHp = player.iMaximumHp = 100u;
            player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; player.isCombatReady = true;
            player.fPositionX = 27.8f + .2f * id; player.fPositionY = .45f; player.fPositionZ = -67.7f;
            room->m_PlayerIdBySessionId[player.iSessionId] = id;
            room->m_PartyIdByPlayerId[id] = 77u; room->m_PartyMembersByPartyId[77u].push_back(id);
            auto connection = std::make_shared<CClientSession>(player.iSessionId, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            connection->m_isSendRunning.store(true); room->m_Sessions.emplace(player.iSessionId, connection);
            sessions.push_back(std::move(connection));
        }
        const auto* source = catalog.Find_KoukuRaidGate(sourceGateId);
        const auto* target = catalog.Find_KoukuRaidGate(targetGateId);
        tests.Require(source && target && !target->strIntroPatternId.empty(),
            "Standalone gate transition resolves its authored source boss and destination intro");
        if (!source || !target) continue;
        tests.Require(room->Spawn_GatePlacement(source->strPrimaryBossPlacementId),
            "Standalone transition begins with the actual previous gate boss");
        auto boss = std::find_if(room->m_WorldEntities.begin(), room->m_WorldEntities.end(), [&](const auto& entity) {
            return entity.strPlacementId == source->strPrimaryBossPlacementId;
        });
        if (boss == room->m_WorldEntities.end()) continue;
        // The public Debug spawn path commits the actor, then records its gate.
        room->Note_GatePlacementRaised(source->strPrimaryBossPlacementId);
        tests.Require(room->m_GateProgress.iCurrentGate == sourceGate &&
            !(room->m_GateProgress.iClearedMask & (1u << (sourceGate - 1u))),
            "Standalone source activation records its actual gate with no stale clear bit");
        if (advance)
        {
            boss->iCurrentHp = 0u; boss->eAction = SERVER_ENTITY_ACTION::DEAD;
            const auto dead = *boss;
            room->Notify_KoukuRaidBossDeath(dead, 100u); room->Notify_GateBossDeath(dead);
            tests.Require(room->m_GateProgress.iCurrentGate == sourceGate &&
                (room->m_GateProgress.iClearedMask & (1u << (sourceGate - 1u))) && !room->Is_KoukuRaidRunning(),
                "Standalone G1/G2 defeat records the clear before the next-gate proposal");
        }
        else
        {
            auto& deadPlayer = room->m_Players.at(participantCount);
            deadPlayer.iCurrentHp = 0u; deadPlayer.eAction = PLAYER_ACTION_STATE::DEAD;
        }
        // An in-progress standalone G3 restart keeps its Mario cursor until every client is ready.
        if (!advance && sourceGate == 3u) room->m_iNextMarioEntryStage = 3u;
        const auto previousMarioStage = room->m_iNextMarioEntryStage;
        const auto previousPlayers = room->m_Players;
        const auto previousEntities = room->m_WorldEntities;
        const auto previousGate = room->m_GateProgress.iCurrentGate;
        const auto previousMask = room->m_GateProgress.iClearedMask;
        const auto previousNextId = room->m_iNextNetEntityId;
        const auto preserved = [&]() {
            if (room->m_GateProgress.iCurrentGate != previousGate || room->m_GateProgress.iClearedMask != previousMask ||
                room->m_iNextMarioEntryStage != previousMarioStage ||
                room->m_iNextNetEntityId != previousNextId || room->m_WorldEntities.size() != previousEntities.size() ||
                room->m_Players.size() != previousPlayers.size()) return false;
            for (const auto& [id, old] : previousPlayers)
            {
                const auto found = room->m_Players.find(id);
                if (found == room->m_Players.end()) return false;
                const auto& player = found->second;
                if (player.fPositionX != old.fPositionX || player.fPositionY != old.fPositionY || player.fPositionZ != old.fPositionZ ||
                    player.iCurrentHp != old.iCurrentHp || player.eAction != old.eAction) return false;
            }
            for (std::size_t i = 0; i < previousEntities.size(); ++i)
            {
                const auto& old = previousEntities[i]; const auto& entity = room->m_WorldEntities[i];
                if (entity.iNetEntityId != old.iNetEntityId || entity.strPlacementId != old.strPlacementId ||
                    entity.iCurrentHp != old.iCurrentHp || entity.eAction != old.eAction ||
                    entity.fPositionX != old.fPositionX || entity.fPositionY != old.fPositionY || entity.fPositionZ != old.fPositionZ) return false;
            }
            return true;
        };
        room->m_iServerTick = 100u;
        C2S_GATE_PROGRESS_PROPOSE proposal; proposal.iRequestSequence = 1u;
        proposal.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
        proposal.eKind = advance ? GATE_PROGRESS_KIND::ADVANCE : GATE_PROGRESS_KIND::RESTART;
        room->Handle_GateProgressPropose(501u, proposal);
        auto& run = room->m_KoukuRaid;
        tests.Require(run.State.iRunEpoch == 0u && room->m_GateProgress.Voters.size() == participantCount && preserved(),
            "A standalone gate proposal preserves the complete party and boss until unanimous approval");
        C2S_GATE_PROGRESS_RESPOND response; response.iRequestSequence = 1u;
        response.iProposalId = room->m_GateProgress.iProposalId; response.bAccepted = true;
        for (unsigned id = 2u; id <= participantCount; ++id)
        {
            room->Handle_GateProgressRespond(500u + id, response);
            if (id < participantCount) tests.Require(run.State.iRunEpoch == 0u && preserved(),
                "Partial standalone gate approval neither moves players nor creates or removes bosses");
        }
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.bGateVoteEntry &&
            run.State.strGateId == targetGateId && preserved(),
            "Unanimous standalone advance or restart prepares the destination Sequence while preserving the old world");
        if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::PREPARING) continue;
        auto ack = run.Request; ack.eOperation = KOUKUSAYDON_RAID_OPERATION::READY;
        ack.iExpectedRunEpoch = run.State.iRunEpoch; ++ack.iRequestSequence;
        std::string reason;
        for (unsigned id = 1u; id < participantCount; ++id)
        {
            tests.Require(room->Apply_KoukuRaidReadiness(500u + id, ack, reason),
                "Standalone gate participant acknowledges its exact published resource pin");
            room->m_iServerTick = 101u; room->Update_KoukuRaid(101u);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && preserved(),
                "Partial standalone gate READY preserves positions, HP, dead members and exact boss identities");
        }
        if (failPreparation)
        {
            ack.eOperation = KOUKUSAYDON_RAID_OPERATION::FAILED;
            ack.strReason = "contract: destination preparation failed";
            tests.Require(room->Apply_KoukuRaidReadiness(500u + participantCount, ack, reason) &&
                run.State.ePhase == KOUKUSAYDON_RAID_PHASE::ABORTED && preserved() &&
                run.State.strReason.find(ack.strReason) != std::string::npos,
                "A failed destination client preserves the previous gate, clear mask, player state and boss scene");
            room->m_iServerTick = 102u; room->Update_KoukuRaid(102u);
            tests.Require(preserved(), "Aborted standalone preparation does not perform deferred teleport or spawn");
            continue;
        }
        tests.Require(room->Apply_KoukuRaidReadiness(500u + participantCount, ack, reason) && preserved(),
            "Final READY only records readiness until the authoritative transition tick");
        room->m_iServerTick = 102u; room->Update_KoukuRaid(102u);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.State.strGateId == targetGateId &&
            run.State.strSequencePatternId == target->strIntroPatternId && !run.bEntryRunning &&
            run.State.iReadyMask == (1u << participantCount) - 1u,
            "Standalone G1/G2 advance and G1/G2/G3 restart begin the published intro before combat");
        if (!advance && sourceGate == 3u)
            tests.Require(room->m_iNextMarioEntryStage == 1u,
                "G3 restart resets its prior Mario cursor only after all READY commit the cinematic");
        for (const auto& [id, old] : previousPlayers)
        {
            const auto& player = room->m_Players.at(id);
            tests.Require(player.fPositionX == old.fPositionX && player.fPositionY == old.fPositionY && player.fPositionZ == old.fPositionZ,
                "The first cinematic tick cannot use the legacy direct combat teleport");
        }
    }

    for (unsigned count = 1; count <= 4; ++count)
    {
        BOSS_PATTERN_DEFINITION pattern;
        pattern.strPatternId = "dice.contract";
        BOSS_PATTERN_LOGIC_WINDOW window;
        window.strWindowId = "dice.bind";
        window.eKind = BOSS_PATTERN_LOGIC_KIND::CARD_DICE_BIND;
        window.iStartMs = 1000u; window.iDurationMs = 2000u;
        pattern.LogicWindows.push_back(window);
        SERVER_WORLD_ENTITY boss;
        boss.iNetEntityId = 90u; boss.iPatternSequence = 7u;
        boss.strPatternId = pattern.strPatternId; boss.iCurrentHp = 100u;
        std::map<PLAYER_ID, SERVER_PLAYER> players;
        for (unsigned id = 1; id <= count; ++id)
        {
            auto& player = players[id]; player.iPlayerId = id;
            player.iNetEntityId = id + 100u; player.iCurrentHp = player.iMaximumHp = 100u;
            player.isCombatReady = true;
        }
        KOUKUSAYDON_LOGIC_LEDGER ledger;
        CKoukuSaydonLogicRuntime::Build(pattern, boss, 100u, ledger);
        std::vector<DAMAGE_EVENT> damage;
        KOUKUSAYDON_LOGIC_OUTPUT output;
        const auto update = [&](unsigned tick) { CKoukuSaydonLogicRuntime::Update(boss, pattern,
            ledger, players, catalog, nullptr, tick, damage, output); };
        const auto bound = [&]() { return std::count_if(players.begin(), players.end(),
            [](const auto& row) { return row.second.bPatternBound; }); };
        update(129u);
        tests.Require(bound() == 0, "Dice bind does not begin before its authored window");
        update(130u);
        tests.Require(bound() == count - 1u && ledger.Windows.front().iFreePlayerNetEntityId != INVALID_NET_ENTITY_ID,
            "Dice selects exactly one free player for one to four participants");
        const auto target = boss.iTargetEntityId;
        update(170u);
        tests.Require(boss.iTargetEntityId == target && bound() == count - 1u,
            "Dice keeps the same free target for the complete bind lifetime");
        update(190u);
        tests.Require(bound() == 0, "Dice releases every owned bind at the authored end tick");
        CKoukuSaydonLogicRuntime::Build(pattern, boss, 200u, ledger);
        update(230u);
        auto& foreign = players.begin()->second;
        foreign.bPatternBound = true; foreign.iPatternBindOwnerNetEntityId = 999u;
        CKoukuSaydonLogicRuntime::Discard(ledger, players, &boss);
        tests.Require(bound() == 1 && foreign.iPatternBindOwnerNetEntityId == 999u,
            "Dice cancellation releases its own binds and preserves a foreign owner");
    }
    // Exercise the production phase-2 placement and return-pin helper against the published cage anchor.
    for (unsigned count = 1u; count <= 4u; ++count)
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Legacy G3 clear fixture loads the published raid admission");
        if (!room->Is_Ready()) continue;
        std::vector<std::shared_ptr<CClientSession>> sessions;
        for (unsigned id = 1u; id <= count; ++id)
        {
            auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = 500u + id;
            player.iNetEntityId = 100u + id; player.iCurrentHp = player.iMaximumHp = 100u;
            player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; player.isCombatReady = true;
            player.fPositionX = -2.45f; player.fPositionY = 1.32f; player.fPositionZ = 945.17f;
            room->m_PlayerIdBySessionId[player.iSessionId] = id;
            auto connection = std::make_shared<CClientSession>(player.iSessionId, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            connection->m_isSendRunning.store(true); room->m_Sessions.emplace(player.iSessionId, connection);
            sessions.push_back(std::move(connection));
        }
        tests.Require(room->Spawn_GatePlacement("boss.kakulsaydon.g3.saydon"),
            "F1 and legacy in-game Gate 3 use the authored primary boss placement");
        auto boss = std::find_if(room->m_WorldEntities.begin(), room->m_WorldEntities.end(), [](const auto& e) {
            return e.strPlacementId == "boss.kakulsaydon.g3.saydon"; });
        if (boss == room->m_WorldEntities.end()) continue;
        auto dead = *boss; dead.iCurrentHp = 0u; dead.eAction = SERVER_ENTITY_ACTION::DEAD;
        room->m_iServerTick = 100u;
        // A prior manual vote and a dead member must not bypass or block the clear's resource admission.
        room->m_GateProgress.iProposalId = 999u; room->m_GateProgress.Voters = {1u};
        if (count > 1u) { room->m_Players.at(count).iCurrentHp = 0u; room->m_Players.at(count).eAction = PLAYER_ACTION_STATE::DEAD; }
        room->Notify_KoukuRaidBossDeath(dead, 100u); room->Notify_GateBossDeath(dead);
        auto& run = room->m_KoukuRaid;
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.bClearedGate3Preparation &&
            room->Is_KoukuRaidInputBlocked() && run.iGate3ClearTick == 100u && !room->m_GateProgress.iProposalId &&
            (room->m_GateProgress.iClearedMask & 4u),
            "A standalone G3 kill admits one pinned run, cancels an old vote and shows false clear while resources prepare");
        if (!run.bClearedGate3Preparation) { std::cout << "[LEGACY ADMISSION] " << room->m_strStatus << '\n'; continue; }
        const auto epoch = run.State.iRunEpoch;
        room->Notify_KoukuRaidBossDeath(dead, 101u);
        tests.Require(run.State.iRunEpoch == epoch && run.iGate3ClearTick == 100u,
            "Duplicate G3 death cannot create a new epoch or restart the five-second clock");
        CPacketWriter preparationWire;
        tests.Require(Write_Message(preparationWire, run.State), "Legacy clear PREPARING uses the existing Shared packet schema");
        room->m_WorldEntities.erase(boss);
        C2S_GATE_PROGRESS_PROPOSE bypass; bypass.iRequestSequence = 90u;
        bypass.eWorldId = WORLD_ID::KAKULSAYDON_ARENA; bypass.eKind = GATE_PROGRESS_KIND::ADVANCE;
        room->Handle_GateProgressPropose(501u, bypass);
        tests.Require(!room->m_GateProgress.iProposalId && !room->Enter_KoukuRaidCombat(4u),
            "Manual advance and direct Bingo entry cannot bypass a standalone clear's resource preparation");
        // Exercise timer-first while the shared bounded resource preparation is pending.
        const auto readyTick = count % 2u ? 120u : 390u;
        room->m_iServerTick = readyTick; room->Update_KoukuRaid(readyTick);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING,
            "Five seconds alone never starts unprepared Encore resources");
        auto ack = run.Request; ack.eOperation = KOUKUSAYDON_RAID_OPERATION::READY;
        ack.iExpectedRunEpoch = epoch; ack.iRequestSequence += 1u;
        for (unsigned id = 1u; id <= count; ++id)
        {
            std::string reason;
            tests.Require(room->Apply_KoukuRaidReadiness(500u + id, ack, reason), "Each legacy clear participant acknowledges the exact resource generation");
            if (id < count) { room->Update_KoukuRaid(readyTick); tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING,
                "Partial READY cannot start standalone Encore"); }
        }
        room->Update_KoukuRaid(readyTick);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && run.State.iEndTick == 250u,
            "Prepared legacy clear preserves the original death plus five-second deadline");
        if (readyTick < 250u) { room->Update_KoukuRaid(249u); tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
            "Fast resource preparation still waits the complete five-second clear beat"); }
        room->m_iServerTick = (std::max)(250u, readyTick + 1u); room->Update_KoukuRaid(room->m_iServerTick);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.State.strGateId == "BINGO" &&
            run.State.strSequencePatternId == run.pCatalog->Find_KoukuRaidGate("BINGO")->strIntroPatternId,
            "Both ready-first and timer-first races converge to the same pinned Encore cinematic");
        const auto start = run.State.iStartTick; room->Update_KoukuRaid(room->m_iServerTick);
        tests.Require(run.State.iStartTick == start, "Repeated safe ticks do not restart Encore playback");
        room->m_iServerTick = run.State.iEndTick; room->Update_KoukuRaid(room->m_iServerTick);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && run.State.strGateId == "BINGO" &&
            std::all_of(room->m_Players.begin(), room->m_Players.end(), [](const auto& p) { return p.second.iCurrentHp > 0u; }),
            "Standalone Encore completion enters normal Bingo authority and revives its pinned participants");
    }
    for (unsigned count = 1; count <= 4; ++count)
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Mario formation loads actual Gate 3 navigation");
        if (!room->Is_Ready()) continue;
        auto& audition = room->m_KoukuSaydonPatternAudition;
        audition.ePhase = CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
        audition.pProductGeneration = room->m_GameplayCatalog.Get_ActiveGeneration();
        audition.PinnedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        audition.iPinnedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(*audition.pProductGeneration);
        std::string status;
        const auto* phase = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*audition.pProductGeneration, "KAKULSAYDON_G1_PATTERN_33", status);
        const BOSS_PATTERN_MECHANIC_TRIGGER* formation = nullptr;
        if (phase) for (const auto& trigger : phase->MechanicTriggers)
            if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::MARIO_PHASE2_PLAYERS) formation = &trigger;
        tests.Require(formation && std::abs(formation->fTeleportX + 7.07f) < .02f &&
            std::abs(formation->fTeleportY - 1.32f) < .02f && std::abs(formation->fTeleportZ - 934.43f) < .02f,
            "Phase 2 uses the saved Iron Maiden effect anchor as its formation and return point");
        if (!formation) continue;
        SERVER_WORLD_ENTITY boss;
        boss.iNetEntityId = 90u; boss.iPatternSequence = 7u; boss.iPatternStartTick = 100u;
        boss.strPatternId = phase->strPatternId; boss.iCurrentHp = 10000u;
        unsigned durationMs = 0u;
        for (const auto& stage : phase->Stages) durationMs += stage.iDurationMs;
        const auto deadline = CKoukuSaydonLogicRuntime::Add_Ticks(boss.iPatternStartTick, CKoukuSaydonLogicRuntime::Ticks_FromMs(durationMs));
        bool nonFirstCaptive = false;
        for (unsigned seed = 1u; seed <= (count >= 3u ? 16u : 1u); ++seed)
        {
            room->m_Players.clear(); room->m_MarioLayoutRandom.seed(seed);
            for (unsigned id = 1; id <= count; ++id)
            {
                auto& player = room->m_Players[id]; player.iPlayerId = id; player.iNetEntityId = 100u + id;
                player.iCurrentHp = player.iMaximumHp = 100u; player.isCombatReady = true;
                player.fPositionX = 20.f + id; player.fPositionY = 1.32f; player.fPositionZ = 950.f;
                if (count >= 2u && id == count) player.iMarioStage = 1u;
            }
            tests.Require(room->Commit_KoukuMarioPhasePlayers(boss, *formation, 130u),
                "The authored Mario phase-2 formation commits for one to four participants");
            unsigned bound = 0u; std::vector<float> positions;
            for (const auto& [id, player] : room->m_Players)
            {
                if (player.iMarioStage)
                {
                    SERVER_NAV_POINT landing;
                    tests.Require(player.fPositionX == 20.f + id && player.fPositionZ == 950.f && player.MarioReturnPosition &&
                        room->Resolve_MarioReturnDestination(player, landing) &&
                        std::abs(landing.x - formation->fTeleportX) < .01f && std::abs(landing.z - formation->fTeleportZ) < .01f,
                        "The Mario entrant stays in its lane and pins a navigation-valid return to the same cage anchor");
                    auto restored = player; restored.Clear_MarioControl(true);
                    tests.Require(restored.MarioReturnPosition && room->Resolve_MarioReturnDestination(restored, landing) &&
                        std::abs(landing.x - formation->fTeleportX) < .01f && std::abs(landing.z - formation->fTeleportZ) < .01f,
                        "Terminal form restoration preserves the same phase-2 return point");
                    continue;
                }
                positions.push_back(player.fPositionX);
                tests.Require(std::abs(player.fPositionZ - formation->fTeleportZ) < .01f,
                    "Each non-entrant uses the authored Iron Maiden row");
                if (!player.bPatternBound) continue;
                ++bound; nonFirstCaptive = nonFirstCaptive || id != 1u;
                tests.Require(std::abs(player.fPositionX - formation->fTeleportX) < .01f &&
                    std::abs(player.fPatternBindRestoreX - formation->fTeleportX) < .01f &&
                    std::abs(player.fPatternBindRestoreZ - formation->fTeleportZ) < .01f &&
                    player.iPatternBindOwnerNetEntityId == boss.iNetEntityId &&
                    player.iPatternBindSequence == boss.iPatternSequence && player.iPatternBindEndTick == deadline,
                    "The randomly chosen captive owns slot zero at the cage until the phase deadline");
            }
            std::sort(positions.begin(), positions.end());
            for (unsigned slot = 0; slot < positions.size(); ++slot)
                tests.Require(std::abs(positions[slot] - (formation->fTeleportX + 1.25f * slot)) < .01f,
                    "Other non-entrants retain distinct 1.25-metre offsets after the captive swap");
            tests.Require(bound == (count >= 3u ? 1u : 0u),
                "Solo and duo formations have no captive; three or four participants bind one non-entrant");
        }
        if (count >= 3u) tests.Require(nonFirstCaptive, "Formation regression exercises a random captive other than the first player");
        // Formation cancels the previous action and clears combat readiness for that tick.
        // Admit the next formation explicitly so this tests its blocked destination path.
        for (auto& [id, player] : room->m_Players)
            if (player.iCurrentHp && !player.iMarioStage) player.isCombatReady = true;
        const auto saved = room->m_Players;
        auto blocked = *formation; blocked.fTeleportX = 1000000.f;
        tests.Require(!room->Commit_KoukuMarioPhasePlayers(boss, blocked, 131u), "An invalid cage destination rejects the formation");
        for (const auto& [id, player] : room->m_Players)
        {
            const auto& prior = saved.at(id);
            tests.Require(player.fPositionX == prior.fPositionX && player.fPositionY == prior.fPositionY &&
                player.fPositionZ == prior.fPositionZ && player.bPatternBound == prior.bPatternBound &&
                player.MarioReturnPosition == prior.MarioReturnPosition,
                "Rejected formation preserves every participant's position, binding and return pin");
        }
    }
    const bool raidPublished = catalog.Find_KoukuRaidGate("GATE1") &&
        catalog.Find_KoukuRaidGate("GATE2") && catalog.Find_KoukuRaidGate("GATE3");
    tests.Require(raidPublished, "All three gate flows and sequence clocks are published");
    if (!raidPublished) return;
#ifdef _DEBUG
    for (unsigned failure = 0u; failure < 4u; ++failure)
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        if (!room->Is_Ready()) continue;
        auto connection = std::make_shared<CClientSession>(501u, INVALID_SOCKET,
            CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
        connection->m_isSendRunning.store(true); room->m_Sessions.emplace(501u, connection);
        auto& player = room->m_Players[1u]; player.iPlayerId = 1u; player.iSessionId = 501u; player.iNetEntityId = 101u;
        player.iCurrentHp = player.iMaximumHp = 100u; player.isCombatReady = true;
        player.fPositionX = 9.f; player.fPositionZ = -61.f; player.bPatternBound = true;
        room->m_PlayerIdBySessionId[501u] = 1u;
        SERVER_WORLD_ENTITY existing;
        const auto* placement = room->Find_Placement("boss.kakulsaydon.g1.saydon");
        if (placement && room->Build_WorldEntity(*placement, 987u, existing)) room->m_WorldEntities.push_back(std::move(existing));
        const auto beforeEntities = room->m_WorldEntities.size();
        C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request;
        request.iRequestSequence = 1u; request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA; request.strStartGateId = "GATE1";
        request.ExpectedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        request.iActionSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(room->m_GameplayCatalog.Active());
        request.iSequenceSourceRevision = room->m_GameplayCatalog.Active().Find_KoukuRaidGate("GATE1")->iSequenceRevision;
        room->m_iServerTick = 20u; room->Handle_KoukuRaidRequest(501u, request);
        tests.Require(room->m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING, "Abort fixture enters preparation through START");
        request.iRequestSequence = 2u; request.iExpectedRunEpoch = room->m_KoukuRaid.State.iRunEpoch;
        if (failure == 0u) { request.eOperation = KOUKUSAYDON_RAID_OPERATION::FAILED; request.strReason = "Sequence revision unavailable"; room->Handle_KoukuRaidRequest(501u, request); }
        else if (failure == 1u)
        {
            room->m_iServerTick = 320u; room->Update_KoukuRaid(320u);
            tests.Require(room->m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING &&
                !room->m_KoukuRaid.State.iReadyMask && !room->m_KoukuRaid.State.iStartTick &&
                room->m_WorldEntities.size() == beforeEntities && player.fPositionX == 9.f,
                "Debug resource loading may exceed ten seconds without partially starting actors or the cinematic");
            room->m_iServerTick = room->m_KoukuRaid.State.iEndTick;
            room->Update_KoukuRaid(room->m_iServerTick);
        }
        else if (failure == 2u) { request.eOperation = KOUKUSAYDON_RAID_OPERATION::STOP; room->Handle_KoukuRaidRequest(501u, request); }
        else { room->m_PlayerIdBySessionId.erase(501u); room->Update_KoukuRaid(21u); }
        tests.Require(room->m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::ABORTED &&
            room->m_WorldEntities.size() == beforeEntities && player.fPositionX == 9.f && player.fPositionZ == -61.f && player.bPatternBound,
            "FAILED, timeout, owner STOP and participant loss preserve pre-preparation actors, position and gameplay state");
        const auto oldEpoch = request.iExpectedRunEpoch;
        room->m_PlayerIdBySessionId[501u] = 1u;
        auto restart = request; restart.eOperation = KOUKUSAYDON_RAID_OPERATION::START;
        restart.iRequestSequence = 3u; restart.iExpectedRunEpoch = 0u; restart.strReason.clear();
        room->Handle_KoukuRaidRequest(501u, restart);
        auto oldAck = restart; oldAck.eOperation = KOUKUSAYDON_RAID_OPERATION::READY;
        oldAck.iRequestSequence = 4u; oldAck.iExpectedRunEpoch = oldEpoch;
        room->Handle_KoukuRaidRequest(501u, oldAck);
        tests.Require(room->m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING &&
            room->m_KoukuRaid.State.iRunEpoch != oldEpoch && room->m_KoukuRaid.State.iReadyMask == 0u,
            "An old READY cannot acknowledge or abort the next run after preparation failure");
    }
#endif
    for (unsigned count = 1; count <= 4; ++count)
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Raid room loads navigation and world templates");
        if (!room->Is_Ready()) continue;
        auto& run = room->m_KoukuRaid;
        std::vector<std::shared_ptr<CClientSession>> sessions;
        for (unsigned id = 1; id <= count; ++id)
        {
            auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = 500u + id;
            player.iNetEntityId = 100u + id; player.iCurrentHp = player.iMaximumHp = 100u; player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
            player.isCombatReady = true; player.fPositionX = 10.f + id; player.fPositionZ = -60.f;
            room->m_PlayerIdBySessionId[player.iSessionId] = id;
            auto connection = std::make_shared<CClientSession>(player.iSessionId, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            connection->m_isSendRunning.store(true); room->m_Sessions.emplace(player.iSessionId, connection);
            sessions.push_back(std::move(connection));
        }
        C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST start;
        start.iRequestSequence = 1u; start.eWorldId = WORLD_ID::KAKULSAYDON_ARENA; start.strStartGateId = "GATE1";
        start.ExpectedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        start.iActionSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(room->m_GameplayCatalog.Active());
        start.iSequenceSourceRevision = room->m_GameplayCatalog.Active().Find_KoukuRaidGate("GATE1")->iSequenceRevision;
        room->m_iServerTick = 10u;
        const auto originalEntityCount = room->m_WorldEntities.size();
        begin(*room, 501u, start);
        constexpr unsigned preparationTicks = 20u * 60u * 30u;
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.State.iStartTick == 0u &&
            run.State.iEndTick == 10u + preparationTicks && run.State.ParticipantPlayerIds.size() == count,
            "Real START command pins the roster and enters bounded resource preparation without starting the cinematic");
        if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::PREPARING)
        { std::cout << "[RAID START] " << room->m_strStatus << '\n'; continue; }
        auto ack = start; ack.iRequestSequence = 2u; ack.iExpectedRunEpoch = run.State.iRunEpoch;
        ack.eOperation = KOUKUSAYDON_RAID_OPERATION::READY;
        if (count > 1u)
        {
            auto foreignStop = ack; foreignStop.eOperation = KOUKUSAYDON_RAID_OPERATION::STOP;
            room->Handle_KoukuRaidRequest(500u + count, foreignStop);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING,
                "A non-owner cannot cancel preparation");
        }
        std::string reason;
        auto stale = ack; ++stale.iSequenceSourceRevision;
        tests.Require(!room->Apply_KoukuRaidReadiness(501u, stale, reason) && !run.State.iReadyMask,
            "A mismatched Sequence revision cannot acknowledge preparation");
        stale = ack; ++stale.iActionSourceRevision;
        tests.Require(!room->Apply_KoukuRaidReadiness(501u, stale, reason) && !run.State.iReadyMask,
            "A mismatched Action revision cannot acknowledge preparation");
        stale = ack; ++stale.iExpectedRunEpoch;
        tests.Require(!room->Apply_KoukuRaidReadiness(501u, stale, reason) && !run.State.iReadyMask,
            "An ACK from a different raid epoch is isolated");
        room->m_PlayerIdBySessionId[999u] = 999u;
        tests.Require(!room->Apply_KoukuRaidReadiness(999u, ack, reason) && !run.State.iReadyMask,
            "Late-join observers cannot acknowledge the fixed START roster");
        room->m_PlayerIdBySessionId.erase(999u);
        for (unsigned id = 1; id <= count; ++id)
        {
            room->Handle_KoukuRaidRequest(500u + id, ack);
            const auto mask = run.State.iReadyMask;
            room->Handle_KoukuRaidRequest(500u + id, ack);
            tests.Require(run.State.iReadyMask == mask, "Duplicate READY is idempotent through the real session command handler");
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING &&
                room->m_WorldEntities.size() == originalEntityCount && room->m_Players.at(id).fPositionX == 10.f + id,
                "Every READY preserves existing actors and positions until the shared fixed-tick commit");
            if (id < count) { room->Update_KoukuRaid(11u); tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING,
                "One missing participant prevents cinematic start"); }
        }
        room->m_iServerTick = 12u; room->Update_KoukuRaid(12u);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.State.iStartTick == 13u,
            "All participants READY commits one common future cinematic tick");
        unsigned proposalSequence = 1u;
        const auto propose = [&](GATE_PROGRESS_KIND kind) {
            C2S_GATE_PROGRESS_PROPOSE request; request.iRequestSequence = proposalSequence++;
            request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA; request.eKind = kind;
            room->Handle_GateProgressPropose(501u, request);
        };
        const auto respond = [&](unsigned id, bool accepted) {
            C2S_GATE_PROGRESS_RESPOND request; request.iRequestSequence = proposalSequence++;
            request.iProposalId = room->m_GateProgress.iProposalId; request.bAccepted = accepted;
            room->Handle_GateProgressRespond(500u + id, request);
        };
        for (const auto* gateId : {"GATE1", "GATE2", "GATE3"})
        {
            const auto* gate = run.pCatalog->Find_KoukuRaidGate(gateId);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.State.strGateId == gateId,
                "The next gate remains cinematic until its authored end");
            auto end = run.State.iEndTick;
            room->m_iServerTick = end - 1u; room->Update_KoukuRaid(end - 1u);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.iPrimaryBossId == INVALID_NET_ENTITY_ID,
                "Combat actor admission cannot run one tick before the sequence ends");
            room->m_iServerTick = end; room->Update_KoukuRaid(end);
            for (const auto& arrival : gate->Arrivals)
            {
                if (arrival.bClear || arrival.iSlot >= count) continue;
                const auto& player = room->m_Players.at(run.PlayerIds[arrival.iSlot]);
                tests.Require(std::abs(player.fPositionX - arrival.Position[0]) < .01f &&
                    std::abs(player.fPositionZ - arrival.Position[2]) < .01f,
                    "Each participating player arrives at its authored sequence slot");
            }
            if (std::string(gateId) == "GATE3")
            {
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY && !run.bEntryRunning &&
                    run.iPrimaryBossId == INVALID_NET_ENTITY_ID && room->Is_KoukuRaidInputBlocked(),
                    "Gate 3 intro holds the authored arrival positions with gameplay blocked until entry approval");
                CPacketWriter writer;
                tests.Require(Write_Message(writer, run.State), "Gate 3 entry wait has a valid replicated packet");
                CPacketReader reader{writer.Get_Buffer()}; S2C_KOUKUSAYDON_RAID_STATE decoded;
                tests.Require(Read_Message(reader, decoded) && decoded.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY &&
                    decoded.strGateId == "GATE3", "Gate 3 entry wait survives the Shared writer and reader");
                auto invalid = run.State; invalid.strGateId = "GATE2"; CPacketWriter invalidWriter;
                tests.Require(!Write_Message(invalidWriter, invalid), "Other gates cannot claim Gate 3 entry wait");
                room->m_iServerTick = end + 900u; room->Update_KoukuRaid(end + 900u);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY && !run.bEntryRunning,
                    "Elapsed time cannot start Gate 3 combat without the entry button vote");
                const auto firstX = room->m_Players.at(1u).fPositionX;
                room->m_Players.at(count).bPatternBound = true;
                tests.Require(!room->Enter_KoukuRaidCombat(3u) && room->m_Players.at(1u).fPositionX == firstX &&
                    run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY && !run.bEntryRunning,
                    "A rejected participant keeps the entire arrival formation and flow unchanged");
                room->m_Players.at(count).bPatternBound = false;
                const auto beforeAdmissionPlayers = room->m_Players;
                const auto beforeAdmissionEntities = room->m_WorldEntities.size();
                const auto beforeAdmissionGate = room->m_GateProgress;
                const auto gameplayPin = run.State.PinnedGameplayRevision;
                run.State.PinnedGameplayRevision = {};
                tests.Require(!room->Enter_KoukuRaidCombat(3u) && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY &&
                    room->m_WorldEntities.size() == beforeAdmissionEntities && room->m_GateProgress.iCurrentGate == beforeAdmissionGate.iCurrentGate &&
                    room->m_GateProgress.iClearedMask == beforeAdmissionGate.iClearedMask,
                    "Rejected first-flow admission preserves Gate 3 arrival state before any actor or GateProgress commit");
                for (const auto& [id, player] : room->m_Players)
                {
                    const auto& previous = beforeAdmissionPlayers.at(id);
                    tests.Require(player.fPositionX == previous.fPositionX && player.fPositionY == previous.fPositionY &&
                        player.fPositionZ == previous.fPositionZ && player.iCurrentHp == previous.iCurrentHp,
                        "Rejected first-flow admission preserves every Gate 3 player");
                }
                run.State.PinnedGameplayRevision = gameplayPin;
                propose(GATE_PROGRESS_KIND::ENTER_GATE3);
                for (unsigned id = 2u; id <= count; ++id)
                {
                    tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY,
                        "Partial Gate 3 entry approval cannot teleport participants or spawn combat");
                    respond(id, true);
                }
                for (const auto id : run.PlayerIds)
                    tests.Require(std::abs(room->m_Players.at(id).fPositionX + 2.45f) < .01f &&
                        std::abs(room->m_Players.at(id).fPositionZ - 945.17f) < .01f,
                        "Approved Gate 3 entry moves every player to the existing combat spawn");
                tests.Require(run.bGate3CombatEntered && !room->Is_KoukuRaidInputBlocked(),
                    "First Gate 3 entry enables combat and future restart semantics");
            }
            if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT)
                std::cout << "[RAID STATUS] " << gateId << ": " << run.State.strReason << '\n';
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && run.bEntryRunning,
                "Sequence completion starts the saved Server pattern flow");
            if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT) break;
            if (std::string(gateId) == "GATE1" || std::string(gateId) == "GATE3")
            {
                if (count > 1u)
                {
                    C2S_GATE_PROGRESS_PROPOSE unauthorized; unauthorized.iRequestSequence = proposalSequence++;
                    unauthorized.eWorldId = WORLD_ID::KAKULSAYDON_ARENA; unauthorized.eKind = GATE_PROGRESS_KIND::RESTART;
                    room->Handle_GateProgressPropose(500u + count, unauthorized);
                    tests.Require(!room->m_GateProgress.iProposalId, "Only the raid owner can propose a gate vote");
                }
                room->m_Players.at(1u).iCurrentHp = 0u; room->m_Players.at(1u).eAction = PLAYER_ACTION_STATE::DEAD;
                propose(GATE_PROGRESS_KIND::RESTART);
                for (unsigned id = 2u; id <= count; ++id) respond(id, true);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.State.strGateId == gateId &&
                    run.State.strSequencePatternId == gate->strIntroPatternId && room->m_Players.at(1u).iCurrentHp == 100u,
                    "An approved restart revives participants and replays the same gate intro before combat");
                end = run.State.iEndTick; room->m_iServerTick = end; room->Update_KoukuRaid(end);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && run.bEntryRunning,
                    "Restarted intro resumes the first authored flow entry");
            }
            SERVER_WORLD_ENTITY dead;
            dead.iNetEntityId = run.iPrimaryBossId; dead.iCurrentHp = 100u;
            room->Notify_KoukuRaidBossDeath(dead, end + 1u);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT,
                "Living boss cannot trigger a gate transition");
            if (std::string(gateId) == "GATE2")
            {
                auto supporting = dead; ++supporting.iNetEntityId; supporting.iCurrentHp = 0u;
                supporting.strPlacementId = "boss.kakulsaydon.g2.big-saydon";
                room->Notify_KoukuRaidBossDeath(supporting, end + 1u); room->Notify_GateBossDeath(supporting);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && !(room->m_GateProgress.iClearedMask & 2u),
                    "Gate 2 supporting actor cannot clear the pinned primary boss encounter");
            }
            dead.iCurrentHp = 0u; room->Notify_KoukuRaidBossDeath(dead, end + 2u);
            const auto currentGate = room->m_GateProgress.iCurrentGate;
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE &&
                (room->m_GateProgress.iClearedMask & (1u << (currentGate - 1u))),
                "Actual primary boss death opens the existing clear UI and waits without a deadline");
            if (currentGate == 3u)
            {
                const auto* encore = run.pCatalog->Find_KoukuRaidGate("BINGO");
                tests.Require(encore && !encore->strIntroPatternId.empty() &&
                    run.State.iEndTick == end + 152u && room->Is_KoukuRaidInputBlocked(),
                    "Gate 3 false clear reserves exactly five Server seconds and blocks gameplay");
                propose(GATE_PROGRESS_KIND::ADVANCE);
                tests.Require(!room->m_GateProgress.iProposalId && !room->Enter_KoukuRaidCombat(4u),
                    "A vote or direct entry cannot bypass the pending encore");
                continue;
            }
            room->m_iServerTick = end + 9000u; room->Update_KoukuRaid(end + 9000u);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && run.State.strGateId == gateId,
                "Other cleared gates retain their explicit next-gate vote");
            if (count > 1u)
            {
                propose(GATE_PROGRESS_KIND::ADVANCE);
                tests.Require(room->m_GateProgress.Voters.size() == count && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
                    "Every pinned participant votes even when the room players have no party object");
                room->m_PlayerIdBySessionId[999u] = 999u;
                C2S_GATE_PROGRESS_RESPOND observer; observer.iRequestSequence = proposalSequence++;
                observer.iProposalId = room->m_GateProgress.iProposalId; observer.bAccepted = true;
                room->Handle_GateProgressRespond(999u, observer);
                room->m_PlayerIdBySessionId.erase(999u);
                tests.Require(room->m_GateProgress.Accepted.size() == 1u, "A late join cannot approve a fixed-roster gate vote");
                respond(2u, false);
                tests.Require(!room->m_GateProgress.iProposalId && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
                    "Declining gate entry preserves the cleared gate and its presentation pin");
                propose(GATE_PROGRESS_KIND::ADVANCE);
                room->m_iServerTick = room->m_GateProgress.iDeadlineTick; room->Expire_GateProgressVote();
                tests.Require(!room->m_GateProgress.iProposalId && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
                    "A gate vote timeout does not start a sequence or despawn the arena");
            }
            propose(GATE_PROGRESS_KIND::ADVANCE);
            for (unsigned id = 2u; id <= count; ++id)
            {
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
                    "Partial gate approval cannot start the next sequence");
                respond(id, true);
            }
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC,
                "Only unanimous gate entry approval starts the next cinematic");
            if (std::string(gateId) == "GATE2")
            {
                tests.Require(!run.bClearCinematic && run.State.strGateId == "GATE3" &&
                    run.State.strSequencePatternId == run.pCatalog->Find_KoukuRaidGate("GATE3")->strIntroPatternId,
                    "Approved Gate 2 exit starts only the Gate 3 intro without an extra clear movie");
            }
        }
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && room->m_GateProgress.iClearedMask == 7u,
            "One to four participants can clear every gate with explicit UI approvals");
        if (const auto* bingo = run.pCatalog->Find_KoukuRaidGate("BINGO"))
        {
            // The saved encore removes 5000 ms of lead-in while retaining its audio tail.
            tests.Require(bingo->strIntroPatternId == "KAKULSAYDON_G1_PATTERN_10" && bingo->iIntroDurationMs == 21322u &&
                bingo->Entries.size() == 35u && !bingo->strLoopStartEntryId.empty(),
                "Published Bingo pins the trimmed 21322 ms encore before its saved independent combat flow");
            if (bingo->Entries.empty()) continue;
            const auto encoreStartTick = run.State.iEndTick;
            room->m_iServerTick = encoreStartTick - 1u; room->Update_KoukuRaid(room->m_iServerTick);
            tests.Require(run.State.strGateId == "GATE3" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
                "One tick before five seconds the false-clear scene is retained");
            room->m_iServerTick = encoreStartTick; room->Update_KoukuRaid(room->m_iServerTick);
            tests.Require(run.State.strGateId == "BINGO" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC &&
                !run.bClearCinematic && run.State.strSequencePatternId == bingo->strIntroPatternId && run.State.iStartTick == encoreStartTick && room->Is_KoukuRaidInputBlocked(),
                "The five-second deadline starts one server-clock encore without a vote");
            CPacketWriter encoreWriter;
            tests.Require(Write_Message(encoreWriter, run.State), "Encore intro is valid on the Shared wire");
            room->m_iServerTick = run.State.iEndTick - 1u; room->Update_KoukuRaid(room->m_iServerTick);
            tests.Require(!room->Enter_KoukuRaidCombat(4u) && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC,
                "Bingo combat cannot cut the encore short by one tick");
            room->m_iServerTick = run.State.iEndTick;
            const auto originalPlayers = room->m_Players;
            const auto originalWorldCount = room->m_WorldEntities.size();
            const auto originalGate = room->m_GateProgress;
            const auto gameplayPin = run.State.PinnedGameplayRevision;
            run.State.PinnedGameplayRevision = {};
            tests.Require(!room->Enter_KoukuRaidCombat(4u) && run.State.strGateId == "BINGO" &&
                run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && room->m_WorldEntities.size() == originalWorldCount &&
                room->m_GateProgress.iCurrentGate == originalGate.iCurrentGate && room->m_GateProgress.iClearedMask == originalGate.iClearedMask,
                "Rejected Bingo audition admission preserves old actors, gate progress and the cleared flow");
            for (const auto& [id, player] : room->m_Players)
            {
                const auto& previous = originalPlayers.at(id);
                tests.Require(player.fPositionX == previous.fPositionX && player.fPositionY == previous.fPositionY &&
                    player.fPositionZ == previous.fPositionZ && player.iCurrentHp == previous.iCurrentHp,
                    "Rejected Bingo audition admission preserves the full formation");
            }
            run.State.PinnedGameplayRevision = gameplayPin;
            const auto originalSupport = room->m_ServerNavigation.Get_RuntimeSupportSurfaces();
            auto blockedSupport = originalSupport;
            blockedSupport.push_back({"contract.bingo.entry.height", -3.4f, 1147.44f, 2.f, 5.f});
            std::string supportStatus;
            tests.Require(room->m_ServerNavigation.Set_RuntimeSupportSurfaces(blockedSupport, supportStatus),
                "Bingo rejection fixture stages an incompatible destination deck");
            tests.Require(!room->Enter_KoukuRaidCombat(4u) && run.State.strGateId == "BINGO" &&
                run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && room->m_WorldEntities.size() == originalWorldCount,
                "Rejected Bingo navigation preserves the cleared gate, flow and existing world");
            for (const auto& [id, player] : room->m_Players)
            {
                const auto& previous = originalPlayers.at(id);
                tests.Require(player.fPositionX == previous.fPositionX && player.fPositionY == previous.fPositionY &&
                    player.fPositionZ == previous.fPositionZ && player.iCurrentHp == previous.iCurrentHp,
                    "Rejected Bingo entry does not partially move or revive any participant");
            }
            tests.Require(room->m_ServerNavigation.Set_RuntimeSupportSurfaces(originalSupport, supportStatus),
                "Bingo rejection fixture restores the original destination deck");
            room->m_Players.at(count).iCurrentHp = 0u;
            room->m_Players.at(count).eAction = PLAYER_ACTION_STATE::DEAD;
            room->Update_KoukuRaid(room->m_iServerTick);
            tests.Require(run.State.strGateId == "BINGO" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT &&
                run.bEntryRunning && room->m_GateProgress.iCurrentGate == 4u && !room->Is_KoukuRaidInputBlocked(),
                "Encore completion revives the full roster and starts the saved Bingo combat flow");
            if (run.State.strGateId != "BINGO" || run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT) continue;
            for (const auto& [id, player] : room->m_Players)
                tests.Require(player.iCurrentHp == player.iMaximumHp && std::abs(player.fPositionX + 3.4f) < .01f &&
                    std::abs(player.fPositionZ - 1147.44f) < .01f,
                    "Bingo entry revives and moves the complete fixed roster to its authored combat spawn");
            verifyBingoFlow(*room, room->m_iServerTick + 1u, false);
            for (auto& [id, player] : room->m_Players) player.iCurrentHp = player.iMaximumHp = 100u;
            for (auto& entity : room->m_WorldEntities)
                if (entity.iNetEntityId == run.iPrimaryBossId)
                { entity.fPositionX = -3.4f; entity.fPositionY = 0.f; entity.fPositionZ = 1147.44f; }
            room->m_Players.at(count).iCurrentHp = 0u;
            room->m_Players.at(count).eAction = PLAYER_ACTION_STATE::DEAD;
            propose(GATE_PROGRESS_KIND::RESTART);
            for (unsigned id = 2u; id <= count; ++id) respond(id, true);
            tests.Require(run.State.strGateId == "BINGO" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT &&
                run.State.iFlowEntryIndex == 0u && run.bEntryRunning && room->m_Players.at(count).iCurrentHp == 100u,
                "Bingo restart revives the roster and ignores its soon-despawned boss blocking the original spawn");
            room->Stop_KoukuRaid("Standalone Bingo START contract");
            auto bingoStart = start; bingoStart.iRequestSequence = 100u; bingoStart.strStartGateId = "BINGO";
            CPacketWriter requestWriter;
            tests.Require(Write_Message(requestWriter, bingoStart), "Standalone Bingo START has a valid typed request");
            begin(*room, 501u, bingoStart);
            tests.Require(run.State.strGateId == "BINGO" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING &&
                run.State.strSequencePatternId.empty() && !run.bEntryRunning,
                "Standalone Bingo START pins its cinematic resources while retaining direct combat entry");
            CPacketWriter prepareWriter;
            tests.Require(Write_Message(prepareWriter, run.State), "No-intro Bingo preparation can be broadcast to real clients");
            CPacketReader prepareReader{prepareWriter.Get_Buffer()}; S2C_KOUKUSAYDON_RAID_STATE preparation;
            tests.Require(Read_Message(prepareReader, preparation) && preparation.strGateId == "BINGO" &&
                preparation.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && preparation.strSequencePatternId.empty(),
                "No-intro Bingo preparation survives the Shared writer and reader");
            auto bingoReady = bingoStart; bingoReady.iRequestSequence = 101u;
            bingoReady.eOperation = KOUKUSAYDON_RAID_OPERATION::READY; bingoReady.iExpectedRunEpoch = run.State.iRunEpoch;
            for (unsigned id = 1u; id <= count; ++id) room->Handle_KoukuRaidRequest(500u + id, bingoReady);
            ++room->m_iServerTick; room->Update_KoukuRaid(room->m_iServerTick);
            tests.Require(run.State.strGateId == "BINGO" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT &&
                run.bEntryRunning, "Standalone Bingo READY starts the same saved combat loop without a cinematic phase");
            if (!bingo->strClearPatternId.empty())
            {
                tests.Require(bingo->strClearPatternId == "KAKULSAYDON_G1_PATTERN_9" && bingo->iClearDurationMs >= 49083u,
                    "Published Bingo clear reuses its original ending and finite sound tail clock");
                SERVER_WORLD_ENTITY endingBoss;
                endingBoss.iNetEntityId = run.iPrimaryBossId; endingBoss.iCurrentHp = 0u;
                endingBoss.eAction = SERVER_ENTITY_ACTION::DEAD;
                endingBoss.strPlacementId = bingo->strPrimaryBossPlacementId;
                const auto primaryBeforeDeath = run.iPrimaryBossId;
                room->Notify_KoukuRaidBossDeath(endingBoss, ++room->m_iServerTick);
                room->Notify_GateBossDeath(endingBoss);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE &&
                    !(room->m_GateProgress.iClearedMask & 8u) && run.iPrimaryBossId == primaryBeforeDeath,
                    "Bingo death delays clear confirmation and defers actor cleanup beyond the boss iteration");
                room->Update_KoukuRaid(++room->m_iServerTick);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.bClearCinematic &&
                    run.State.strGateId == "BINGO" && run.State.strSequencePatternId == bingo->strClearPatternId &&
                    room->Is_KoukuRaidInputBlocked() && !run.bEntryRunning && !(room->m_GateProgress.iClearedMask & 8u),
                    "The next safe tick starts the pinned Bingo ending and blocks combat until it finishes");
                CPacketWriter endingWriter;
                tests.Require(Write_Message(endingWriter, run.State), "Bingo ending state has a valid Shared wire contract");
                const auto endingTick = run.State.iEndTick;
                room->m_iServerTick = endingTick - 1u; room->Update_KoukuRaid(room->m_iServerTick);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && !(room->m_GateProgress.iClearedMask & 8u),
                    "Bingo ending keeps the clear flag pending until the last presentation tick");
                room->m_iServerTick = endingTick; room->Update_KoukuRaid(endingTick);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && run.State.strSequencePatternId.empty() &&
                    run.State.iEndTick == 0u && (room->m_GateProgress.iClearedMask & 8u) && run.bClearCinematic,
                    "Bingo ending completes once and commits its clear flag without restarting combat");
                room->m_iServerTick += 9000u; room->Update_KoukuRaid(room->m_iServerTick);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && run.State.strSequencePatternId.empty() && !run.bEntryRunning,
                    "Finished Bingo ending remains idle and cannot replay on subsequent ticks");
            }

        }
        else std::cout << "[RAID BINGO] Published Bingo gate not present; the optional transition contract was not exercised.\n";
    }
}

int CServerGameplayContractRunner::Run_KoukuRaid()
{
    int result = 1;
    const auto execute = [](void* opaque) {
        TESTS tests;
        Run_KoukuRaidIntegration(tests);
        CGameplayCatalog catalog;
        if (catalog.Load()) Run_KoukuFearAndCounterContracts(tests, catalog);
        else tests.Require(false, "Load fear/counter catalog");
        std::cout << "failures : " << tests.failures << '\n';
        *static_cast<int*>(opaque) = tests.failures ? 1 : 0;
    };
    if (!Run_WithContractWorkerStack(execute, &result)) return 1;
    return result;
}


int CServerGameplayContractRunner::Run_KoukuDiceDamageContracts()
{
    int result = 1;
    const auto execute = [](void* opaque) {
        TESTS tests;
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Dice damage fixture loads authoritative gameplay");
        if (!room->Is_Ready()) { std::cout << "Dice room: " << room->Get_Status() << '\n'; *static_cast<int*>(opaque) = 1; return; }
        auto& catalog = room->m_GameplayCatalog;
        // Release retains disabled gate templates until admission; this focused fixture owns its actor.
        room->m_WorldEntities.clear(); room->m_WorldEntities.emplace_back();
        auto& boss = room->m_WorldEntities.front();
        boss.iNetEntityId = 700u; boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        boss.strEncounterId = "ENCOUNTER_KAKULSAYDON_G1"; boss.strArchetypeId = "BOSS_KAKULSAYDON_G1_SAYDON";
        boss.iCurrentHp = 100u; boss.eAction = SERVER_ENTITY_ACTION::IDLE;
        boss.strPatternId = "dice.hit.contract"; boss.iPatternSequence = 700u;
        boss.PinnedDefinitionRevision = catalog.Get_ActiveRevision();
        boss.fPositionX = boss.fPositionY = boss.fPositionZ = 0.f;
        boss.iPatternTargetEntityId = 101u;
        const auto seedPlayers = [&](MECHANIC_CARD_COLOR color) {
            room->m_Players.clear();
            for (PLAYER_ID id = 1u; id <= 6u; ++id) {
                auto& p = room->m_Players[id]; p.iPlayerId = id; p.iNetEntityId = 100u + id;
                p.iCurrentHp = p.iMaximumHp = 1000u * id; p.isCombatReady = true;
                p.eMechanicCardSymbol = id <= 4u ? static_cast<MECHANIC_CARD_SYMBOL>(id) : MECHANIC_CARD_SYMBOL::NONE;
                p.eMechanicCardColor = color;
                p.bPatternBound = id == 2u || id == 3u;
                p.iPatternBindOwnerNetEntityId = p.bPatternBound ? boss.iNetEntityId : INVALID_NET_ENTITY_ID;
                if (id == 5u) p.fPositionX = 100.f;
            }
        };
        for (const char* hitKind : {"CONTACT", "TIMED"})
        for (unsigned symbol = 1u; symbol <= 4u; ++symbol)
        for (const auto color : {MECHANIC_CARD_COLOR::RED, MECHANIC_CARD_COLOR::BLACK}) {
            seedPlayers(color); room->m_CombatObjectRuntime.Reset(); room->m_CombatObjectRuntime.Discard_PendingLifecycle();
            BOSS_PATTERN_MECHANIC_TRIGGER trigger{};
            trigger.strTriggerId = "dice.card"; trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES;
            trigger.iDurationMs = 1000u; trigger.ProjectileVisualIds = {"test.card"};
            trigger.ProjectileCardSymbols = {static_cast<MECHANIC_CARD_SYMBOL>(symbol)};
            trigger.strContactVisualId = "test.card.burst"; trigger.fProjectileSpeedMps = 1.f;
            trigger.fProjectileContactRadiusM = 1.f; trigger.iProjectileLifetimeMs = 1000u;
            trigger.iProjectileCountPerWave = 1u; trigger.bProjectileHoming = true;
            ATTACK_HIT_TEMPLATE hit{}; hit.strHitId = "card.contact"; hit.strTrigger = hitKind;
            hit.strShape = "CIRCLE"; hit.strDamageKind = "MAX_HP_PERCENT"; hit.iDamagePercent = 50u;
            hit.fRadiusM = 1.; hit.iRepeatCount = 1u;
            if (hit.strTrigger == "CONTACT") hit.iEndMs = 1000u;
            else hit.iAtMs = 33u;
            trigger.ProjectileHits = {hit};
            BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = boss.strPatternId; pattern.MechanicTriggers = {trigger};
            KOUKUSAYDON_LOGIC_LEDGER ledger{};
            CKoukuSaydonLogicRuntime::Build(pattern, boss, 100u, ledger);
            room->Update_KoukuPlayerTargets(boss, pattern, ledger, catalog, 100u);
            const auto& objects = room->m_CombatObjectRuntime.Get_LiveObjects();
            tests.Require(objects.size() == 1u && objects.front().eDamageImmuneCardSymbol == static_cast<MECHANIC_CARD_SYMBOL>(symbol),
                "The actual pursuit spawn pins its explicit visual-slot symbol on the Server object");
            std::vector<DAMAGE_EVENT> events;
            room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, catalog, .034f, 101u, events);
            bool correct = events.size() == 4u;
            for (const auto& [id, p] : room->m_Players) {
                const auto expected = (id == symbol || id == 5u) ? p.iMaximumHp : p.iMaximumHp / 2u;
                correct = correct && p.iCurrentHp == expected;
            }
            if (!correct) {
                std::cout << "Dice case " << hitKind << " symbol=" << symbol << " color=" << static_cast<unsigned>(color)
                    << " objects=" << objects.size() << " events=" << events.size() << " room=" << room->Get_Status();
                for (const auto& [id, player] : room->m_Players) std::cout << " hp" << id << '=' << player.iCurrentHp;
                std::cout << '\n';
            }
            tests.Require(correct, hit.strTrigger == "CONTACT" ?
                "Contact card: each of four suits ignores color, protects only matching players, and deals 50% maximum HP to different or unassigned suits including bound players" :
                "Timed card: each of four suits uses the same per-player symbol immunity and exact 50% maximum HP without hitting distant players");
            const auto eventCount = events.size();
            room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, catalog, .034f, 102u, events);
            tests.Require(events.size() == eventCount, "A completed card contact does not deal a duplicate hit on the next tick");
        }
        // An ordinary pursuit with no card symbols remains an ordinary authored attack.
        seedPlayers(MECHANIC_CARD_COLOR::RED); room->m_CombatObjectRuntime.Reset();
        BOSS_COMBAT_OBJECT_DEFINITION generic{};
        generic.strEncounterId = boss.strEncounterId; generic.strOwnerPatternId = boss.strPatternId;
        generic.strOwnerStageActionId = "generic.card"; generic.strCombatObjectArchetypeId = "combatobject.kouku.pursuit";
        generic.strClientVisualId = "test.generic"; generic.iLifeMs = 1000u;
        ATTACK_HIT_TEMPLATE ordinary{}; ordinary.strHitId = "ordinary"; ordinary.strTrigger = "CONTACT";
        ordinary.strShape = "CIRCLE"; ordinary.strDamageKind = "MAX_HP_PERCENT"; ordinary.iDamagePercent = 50u;
        ordinary.fRadiusM = 1.; ordinary.iEndMs = 1000u; ordinary.iRepeatCount = 1u; generic.AttackTemplates = {ordinary};
        auto transaction = room->m_CombatObjectRuntime.Begin_Transaction(); std::string status;
        const bool staged = room->m_CombatObjectRuntime.Stage_BossCombatObject(transaction, boss, nullptr, generic, nullptr, catalog, 1u, 200u, status);
        tests.Require(staged && room->m_CombatObjectRuntime.Commit(std::move(transaction)), "Generic pursuit stages without optional symbol data");
        std::vector<DAMAGE_EVENT> events;
        room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, catalog, .034f, 201u, events);
        tests.Require(room->m_Players.at(1u).iCurrentHp == 500u && events.size() == 5u,
            "Absent cardSymbols preserves ordinary damage even when players have card assignments");

        // Fixed damage is independent of maximum HP and shares the existing contact clock and safe-zone gate.
        BOSS_PATTERN_LOGIC_RESULT fixed{}; fixed.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE; fixed.iDamageAmount = 500u;
        BOSS_PATTERN_LOGIC_WINDOW contact{}; contact.strWindowId = "fixed.tick";
        contact.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA; contact.iDurationMs = 1000u; contact.iRepeatIntervalMs = 500u;
        contact.OnSuccess = {fixed}; BOSS_LOGIC_REGION circle{}; circle.strRegionId = "fixed.region";
        circle.bCircle = true; circle.fRadiusM = 5.f; circle.eAnchor = BOSS_LOGIC_REGION_ANCHOR::WORLD;
        contact.CardRegions = {circle}; BOSS_PATTERN_DEFINITION fixedPattern{};
        fixedPattern.strPatternId = boss.strPatternId; fixedPattern.LogicWindows = {contact};
        seedPlayers(MECHANIC_CARD_COLOR::RED); room->m_Players.erase(1u); room->m_Players.erase(2u);
        KOUKUSAYDON_LOGIC_LEDGER fixedLedger{}; KOUKUSAYDON_LOGIC_OUTPUT output{}; events.clear();
        CKoukuSaydonLogicRuntime::Build(fixedPattern, boss, 300u, fixedLedger);
        for (unsigned tick : {300u, 300u, 314u, 315u, 330u})
            CKoukuSaydonLogicRuntime::Update(boss, fixedPattern, fixedLedger, room->m_Players, catalog, nullptr, tick, events, output);
        tests.Require(room->m_Players.at(3u).iCurrentHp == 2000u && room->m_Players.at(4u).iCurrentHp == 3000u &&
            room->m_Players.at(6u).iCurrentHp == 5000u && events.size() == 6u,
            "Fixed fire tick deals exactly 500 at entry and each 500 ms, independent of maximum HP, with no duplicate or end tick");
        auto zone = contact; zone.strWindowId = "fixed.safe"; zone.eKind = BOSS_PATTERN_LOGIC_KIND::INVULNERABILITY_ZONE;
        zone.iRepeatIntervalMs = 0u; zone.OnSuccess.clear(); fixedPattern.LogicWindows.push_back(zone);
        seedPlayers(MECHANIC_CARD_COLOR::RED); events.clear(); output = {};
        CKoukuSaydonLogicRuntime::Build(fixedPattern, boss, 400u, fixedLedger);
        CKoukuSaydonLogicRuntime::Update(boss, fixedPattern, fixedLedger, room->m_Players, catalog, nullptr, 400u, events, output);
        tests.Require(events.empty() && room->m_Players.at(1u).iCurrentHp == 1000u,
            "Same-pattern safe zones suppress fixed damage through the common result gate");

        std::array<wchar_t, 32768u> candidatePath{};
        const auto length = GetEnvironmentVariableW(L"LOSTARK_KOUKU_DICE_TEST_ROWS", candidatePath.data(), static_cast<DWORD>(candidatePath.size()));
        if (length && length < candidatePath.size()) {
            const std::filesystem::path path(candidatePath.data()); std::ifstream input(path, std::ios::binary);
            const std::string rows((std::istreambuf_iterator<char>(input)), {});
            GameplayDataRevision hash{}; CGameplayCatalog draft;
            const bool admitted = !rows.empty() && CServerApp::Hash_GameplayFileForAdmission(path, hash, status) && draft.Load_DraftKoukuProduct(catalog, rows, hash);
            if (!admitted) std::cout << "Dice draft: " << draft.Get_Status() << '\n';
            tests.Require(admitted, "Actual prepared Dice rows pass the native transactional draft parser");
            if (admitted) {
                const auto* patterns = draft.Find_BossPatterns("ENCOUNTER_KAKULSAYDON_G1");
                bool exact = false;
                if (patterns) for (const auto& pattern : *patterns) if (pattern.strPatternId == "KAKULSAYDON_G1_PATTERN_78")
                    for (const auto& trigger : pattern.MechanicTriggers) if (trigger.strTriggerId == "KAKULSAYDON_G1_PATTERN_78.logic.1")
                        exact = trigger.ProjectileCardSymbols == std::vector<MECHANIC_CARD_SYMBOL>{MECHANIC_CARD_SYMBOL::HEART, MECHANIC_CARD_SYMBOL::SPADE, MECHANIC_CARD_SYMBOL::CLUB, MECHANIC_CARD_SYMBOL::DIAMOND} &&
                            trigger.ProjectileHits.size() == 1u && trigger.ProjectileHits.front().iDamagePercent == 50u;
                tests.Require(exact, "Native P78 parser retains the visual-slot symbols and 50% maximum-HP contact damage");
            }
        }
        std::cout << "failures : " << tests.failures << '\n';
        *static_cast<int*>(opaque) = tests.failures ? 1 : 0;
    };
    if (!Run_WithContractWorkerStack(execute, &result)) return 1;
    return result;
}
