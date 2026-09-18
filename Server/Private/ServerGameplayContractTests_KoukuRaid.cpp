#include "ServerGameplayContractTests_Runner.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include "KoukuSaydonBrain.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>

using namespace LostArk::Server;
using namespace LostArk::Shared;

void CServerGameplayContractRunner::Run_KoukuRaidIntegration(TESTS& tests)
{
#ifdef _DEBUG
    CGameplayCatalog catalog;
    tests.Require(catalog.Load(), "Raid integration loads the published candidate catalog");
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
        else if (failure == 1u) { room->m_iServerTick = 320u; room->Update_KoukuRaid(320u); }
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
        room->Handle_KoukuRaidRequest(501u, start);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.State.iStartTick == 0u &&
            run.State.iEndTick == 310u && run.State.ParticipantPlayerIds.size() == count,
            "Real START command pins the roster and enters a ten-second preparation without starting the cinematic");
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
            if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT)
                std::cout << "[RAID STATUS] " << gateId << ": " << run.State.strReason << '\n';
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && run.bEntryRunning,
                "Sequence completion starts the saved Server pattern flow");
            if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT) break;
            for (const auto& arrival : gate->Arrivals)
            {
                if (arrival.bClear || arrival.iSlot >= count) continue;
                const auto& player = room->m_Players.at(run.PlayerIds[arrival.iSlot]);
                tests.Require(std::abs(player.fPositionX - arrival.Position[0]) < .01f &&
                    std::abs(player.fPositionZ - arrival.Position[2]) < .01f,
                    "Each participating player arrives at its authored sequence slot");
            }
            if (std::string(gateId) == "GATE1")
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
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && run.State.iEndTick == 0u &&
                (room->m_GateProgress.iClearedMask & (1u << (currentGate - 1u))),
                "Actual primary boss death opens the existing clear UI and waits without a deadline");
            room->m_iServerTick = end + 9000u; room->Update_KoukuRaid(end + 9000u);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && run.State.strGateId == gateId,
                "Five elapsed minutes cannot automatically advance a cleared gate");
            if (currentGate == 3u)
            {
                propose(GATE_PROGRESS_KIND::ADVANCE);
                tests.Require(!room->m_GateProgress.iProposalId && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
                    "The final cleared gate remains available to the existing exit or restart UI");
                continue;
            }
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
                tests.Require(run.bClearCinematic && run.State.strSequencePatternId == gate->strClearPatternId,
                    "Approved Gate 2 exit plays its clear sequence before Gate 3 entry");
                room->m_iServerTick = run.State.iEndTick; room->Update_KoukuRaid(run.State.iEndTick);
            }
        }
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && room->m_GateProgress.iClearedMask == 7u,
            "One to four participants can clear every gate with explicit UI approvals");
    }
#endif
}
