#include "ServerGameplayContractTests_Runner.h"
#include "ServerApp.h"
#include "ClientSession.h"
#include "ColosseumCombatPolicy.h"
#include "ColosseumThreatAssessment.h"
#include "Network/PacketWriter.h"
#include "Network/PacketReader.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace LostArk::Server;
using namespace LostArk::Shared;

int CServerGameplayContractRunner::Run_ColosseumMatchContracts()
{
    TESTS tests;
    auto app = std::make_unique<CServerApp>();
    auto source = std::make_shared<CGameRoom>(WORLD_ID::BERN);
    tests.Require(source->Is_Ready(), "Colosseum match fixture loads real Bern generation");
    if (!source->Is_Ready()) return 1;
    app->m_pActiveGameplayGeneration = source->Get_ActiveGameplayGeneration();
    app->m_SharedGameRooms.emplace(WORLD_ID::BERN, source);
    std::vector<std::shared_ptr<CClientSession>> sessions;
    const auto drain = [&]()
    {
        for (const auto& session : sessions)
        {
            session->m_OutboundFrames.clear(); session->m_iQueuedOutboundBytes = 0u;
            session->m_OutboundMetrics.iCurrentQueuedByteCount = 0u;
            session->m_OutboundMetrics.iCurrentQueuedFrameCount = 0u;
        }
    };
    for (unsigned i = 0; i < 8u; ++i)
    {
        const SESSION_ID id = 991000u + i;
        auto session = std::make_shared<CClientSession>(id, INVALID_SOCKET,
            CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
        session->m_isSendRunning.store(true);
        source->Handle_Register(session);
        C2S_ENTER_WORLD enter;
        enter.eWorldId = WORLD_ID::BERN;
        enter.eCharacterClass = i < 4u ? CHARACTER_CLASS_ID::GUARDIANKNIGHT : CHARACTER_CLASS_ID::LANCE_MASTER;
        enter.strNickName = "Colosseum" + std::to_string(i);
        const bool joined = source->Join(id, enter, "npc.bern.25184_1.2");
        tests.Require(joined, "Human session admitted through existing Bern admission");
        if (!joined) return 1;
        sessions.push_back(session);
        app->m_Sessions.emplace(id, session);
        app->m_GameplayBindingBySessionId.emplace(id,
            CServerApp::SESSION_GAMEPLAY_BINDING{ WORLD_ID::BERN, INVALID_SESSION_ID, source });
        auto& player = source->m_Players.at(session->Get_PlayerId());
        player.Inventory.clear(); player.Purse.iSilver = 77u + i;
        drain();
    }
    for (unsigned count = 1u; count <= 4u; ++count)
    {
        source->m_iServerTick = 1000u; source->m_iColosseumQueueDeadline = 0u;
        source->m_ColosseumQueue.clear(); source->m_PendingWorldTransfers.clear(); source->m_bColosseumTransferPending = false;
        for (unsigned i = 0u; i < count; ++i) source->m_ColosseumQueue.push_back({ sessions[i]->Get_SessionId(), i + 1u });
        source->Try_FormColosseumMatch();
        tests.Require(source->m_iColosseumQueueDeadline == 1300u && source->m_PendingWorldTransfers.empty(),
            "One through four humans open exactly a ten-second collection window");
        source->m_iServerTick = 1299u; source->Try_FormColosseumMatch();
        tests.Require(source->m_PendingWorldTransfers.empty(), "No early match before the shared collection deadline");
        source->m_iServerTick = 1300u; source->Try_FormColosseumMatch();
        tests.Require(source->m_PendingWorldTransfers.size() == 1u &&
            source->m_PendingWorldTransfers.front().PartyBatchSessionIds.size() == count && source->m_ColosseumQueue.size() == count,
            "One through four humans reserve the exact present count transactionally at the deadline");
    }
    source->m_ColosseumQueue.clear(); source->m_PendingWorldTransfers.clear(); source->m_bColosseumTransferPending = false;
    source->m_iServerTick = 0u; source->m_iColosseumQueueDeadline = 0u;
    drain();
    for (unsigned count = 1u; count <= 4u; ++count)
    {
        auto smallSource = std::make_shared<CGameRoom>(WORLD_ID::BERN, app->m_pActiveGameplayGeneration);
        auto smallTarget = std::make_shared<CGameRoom>(WORLD_ID::COLOSSEUM, app->m_pActiveGameplayGeneration);
        std::vector<std::shared_ptr<CClientSession>> peers;
        std::vector<SESSION_ID> ordered;
        bool admitted = smallSource->Is_Ready() && smallTarget->Is_Ready();
        for (unsigned i = 0u; i < count && admitted; ++i)
        {
            const SESSION_ID id = 998000u + count * 10u + i;
            auto peer = std::make_shared<CClientSession>(id, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            peer->m_isSendRunning.store(true); smallSource->Handle_Register(peer);
            C2S_ENTER_WORLD enter;
            enter.eWorldId = WORLD_ID::BERN; enter.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
            enter.strNickName = "Flexible" + std::to_string(i);
            admitted = smallSource->Join(id, enter, "npc.bern.25184_1.2");
            if (admitted) smallSource->m_ColosseumQueue.push_back({id, i + 1u});
            peers.push_back(peer); ordered.push_back(id);
        }
        std::string status;
        const bool committed = admitted && smallSource->Transfer_ColosseumMatchTo(*smallTarget, ordered, 9000u + count, status);
        tests.Require(committed, "One through four humans each commit through the real atomic Colosseum admission");
        if (!committed) std::cout << "Flexible admission count=" << count << " status=" << status << '\n';
        if (committed)
        {
            tests.Require(smallTarget->Count_HumanPlayers() == count && smallTarget->m_Players.size() == count + 10u,
                "Flexible admission keeps exactly the admitted humans and ten sessionless mercenaries");
            for (std::size_t i = 0; i < ordered.size(); ++i)
            {
                const auto& p = smallTarget->m_Players.at(smallTarget->m_PlayerIdBySessionId.at(ordered[i]));
                tests.Require(p.iColosseumTeam == i % 2u && p.iColosseumArrivalIndex == i,
                    "Flexible admission preserves the accepted order and parity team");
                smallTarget->Handle_ColosseumLoadReady(ordered[i], {smallTarget->m_iColosseumMatchId});
            }
            const auto autoSelected = std::count_if(smallTarget->m_Players.begin(), smallTarget->m_Players.end(),
                [](const auto& p) { return p.second.Is_ColosseumMercenary() && p.second.bColosseumParticipant; });
            tests.Require(autoSelected == (count == 1u ? 4 : 0),
                "Only solo admission automatically selects four opposite-team mercenaries");
            tests.Require(std::all_of(smallTarget->m_Players.begin(), smallTarget->m_Players.end(), [&](const auto& entry) {
                const auto& p = entry.second;
                if (!p.Is_ColosseumMercenary()) return true;
                if (p.iSessionId != INVALID_SESSION_ID) return false;
                if (!p.bColosseumParticipant) return !smallTarget->m_PartyIdByPlayerId.contains(p.iPlayerId);
                const auto party = smallTarget->m_PartyIdByPlayerId.find(p.iPlayerId);
                return count == 1u && p.iColosseumTeam == 1u && p.iColosseumArrivalIndex < 8u &&
                    p.iColosseumArrivalIndex % 2u == p.iColosseumTeam && party != smallTarget->m_PartyIdByPlayerId.end() &&
                    party->second == smallTarget->m_ColosseumTeamPartyIds[1];
            }), "Automatic opponents use real stable combat seats and party membership without fake sessions");
            const auto initialState = smallTarget->Build_ColosseumState();
            CPacketWriter initialWriter;
            tests.Require(initialState.Participants.size() == count + (count == 1u ? 4u : 0u) &&
                Write_Message(initialWriter, initialState),
                "Admission publishes only human participants and the explicit solo opponent team");
            const auto advance = [&](const std::uint32_t tick) {
                smallTarget->m_iServerTick = tick;
                smallTarget->Update_ColosseumMatch(tick);
            };
            advance(1u);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::RECRUITING,
                "Small matches wait for manual recruitment on each human-owned team");
            std::array<SESSION_ID, 2> recruiter{};
            for (const auto id : ordered)
            {
                const auto& player = smallTarget->m_Players.at(smallTarget->m_PlayerIdBySessionId.at(id));
                recruiter[player.iColosseumTeam] = id;
            }
            for (std::uint8_t team = 0u; team < 2u; ++team)
            {
                if (!recruiter[team]) continue;
                std::uint32_t sequence = 1u;
                for (const auto& [id, candidate] : smallTarget->m_Players)
                {
                    if (!candidate.Is_ColosseumMercenary() || candidate.iColosseumTeam != team) continue;
                    smallTarget->Handle_ColosseumRecruit(recruiter[team], {sequence++, smallTarget->m_iColosseumMatchId, candidate.iNetEntityId});
                }
            }
            const auto state = smallTarget->Build_ColosseumState();
            CPacketWriter writer;
            tests.Require(state.ePhase == COLOSSEUM_MATCH_PHASE::ENTRY_COUNTDOWN && state.Participants.size() == 8u &&
                state.iExpectedPlayers == 8u && state.iPhaseEndTick - state.iPhaseStartTick == 300u && Write_Message(writer, state) &&
                smallTarget->m_PartyMembersByPartyId.at(smallTarget->m_ColosseumTeamPartyIds[0]).size() == 4u &&
                smallTarget->m_PartyMembersByPartyId.at(smallTarget->m_ColosseumTeamPartyIds[1]).size() == 4u,
                "One through four humans reach two real four-person parties after explicit allied recruitment");
            const auto entryDeadline = smallTarget->m_iColosseumPhaseEnd;
            advance(entryDeadline - 1u);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ENTRY_COUNTDOWN &&
                std::none_of(smallTarget->m_Players.begin(), smallTarget->m_Players.end(), [](const auto& p) { return p.second.bColosseumCombatActive; }),
                "Small matches cannot activate combat before the full ten-second entry countdown");
            advance(entryDeadline);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::INTRO &&
                smallTarget->m_iColosseumPhaseEnd == entryDeadline + 258u,
                "Small matches enter the real shared 8.6-second introduction");
            advance(smallTarget->m_iColosseumPhaseEnd);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::COUNTDOWN &&
                smallTarget->m_iColosseumPhaseEnd - smallTarget->m_iColosseumPhaseStart == 300u,
                "Small introductions advance to the real ten-second gate countdown");
            advance(smallTarget->m_iColosseumPhaseEnd);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ACTIVE &&
                smallTarget->m_iColosseumPhaseEnd - smallTarget->m_iColosseumPhaseStart == 3600u &&
                std::count_if(smallTarget->m_Players.begin(), smallTarget->m_Players.end(), [](const auto& p) { return p.second.bColosseumCombatActive; }) == 8,
                "One through four humans activate eight combatants for exactly 120 seconds");
            smallTarget->Handle_ColosseumReturn(ordered.front(), {smallTarget->m_iColosseumMatchId});
            tests.Require(smallTarget->m_PendingWorldTransfers.empty(), "The typed result return rejects an active small match");
            const auto combatDeadline = smallTarget->m_iColosseumPhaseEnd;
            const std::uint8_t winningTeam = count == 2u ? 1u : 0u;
            smallTarget->m_iColosseumScores[winningTeam] = 2u;
            smallTarget->m_iColosseumScores[1u - winningTeam] = 1u;
            advance(combatDeadline - 1u);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ACTIVE,
                "Small matches preserve combat until the full match deadline");
            advance(combatDeadline);
            const auto finished = smallTarget->Build_ColosseumState();
            CPacketWriter resultWriter;
            tests.Require(finished.ePhase == COLOSSEUM_MATCH_PHASE::FINISHED && finished.iWinningTeam == winningTeam &&
                std::count_if(finished.Participants.begin(), finished.Participants.end(), [winningTeam](const auto& row) { return row.iTeam == winningTeam; }) == 4 &&
                std::none_of(smallTarget->m_Players.begin(), smallTarget->m_Players.end(), [](const auto& p) { return p.second.isCombatReady; }) &&
                Write_Message(resultWriter, finished),
                "Small matches finish with an encodable four-person winning roster and no combat authority");

            // The fixture owns an isolated app binding so the result button's typed
            // command reaches the same atomic transfer used by the production Server.
            auto returnApp = std::make_unique<CServerApp>();
            returnApp->m_pActiveGameplayGeneration = app->m_pActiveGameplayGeneration;
            returnApp->m_SharedGameRooms.emplace(WORLD_ID::BERN, smallSource);
            returnApp->m_ColosseumMatches.emplace(smallTarget->m_iColosseumMatchId, smallTarget);
            for (const auto& peer : peers)
            {
                returnApp->m_Sessions.emplace(peer->Get_SessionId(), peer);
                returnApp->m_GameplayBindingBySessionId.emplace(peer->Get_SessionId(),
                    CServerApp::SESSION_GAMEPLAY_BINDING{WORLD_ID::COLOSSEUM, INVALID_SESSION_ID, smallTarget});
            }
            smallTarget->Handle_ColosseumReturn(ordered.front(), {smallTarget->m_iColosseumMatchId + 1u});
            tests.Require(smallTarget->m_PendingWorldTransfers.empty(), "The typed result return rejects another match identity");
            for (std::size_t index = 0; index < ordered.size(); ++index)
            {
                for (const auto& peer : peers)
                {
                    peer->m_OutboundFrames.clear(); peer->m_iQueuedOutboundBytes = 0u;
                    peer->m_OutboundMetrics.iCurrentQueuedByteCount = 0u;
                    peer->m_OutboundMetrics.iCurrentQueuedFrameCount = 0u;
                }
                const auto sessionId = ordered[index];
                smallTarget->Handle_ColosseumReturn(sessionId, {smallTarget->m_iColosseumMatchId});
                SERVER_WORLD_TRANSFER_REQUEST back;
                CServerApp::SESSION_WORLD_TRANSFER_FAILURE failure;
                const bool returned = smallTarget->Try_DequeueWorldTransfer(back) &&
                    returnApp->Transfer_SessionWorld(smallTarget, back, failure);
                tests.Require(returned, "Every small-match human completes the typed result return to Bern");
                if (!returned) { std::cout << "Flexible return count=" << count << " status=" << failure.strContext << '\n'; continue; }
                const auto& restored = smallSource->m_Players.at(smallSource->m_PlayerIdBySessionId.at(sessionId));
                const auto* profile = smallSource->m_GameplayCatalog.Find_Player(restored.eCharacterClass);
                tests.Require(profile && restored.eCharacterClass == CHARACTER_CLASS_ID::LANCE_MASTER &&
                    restored.strNickName == "Flexible" + std::to_string(index) &&
                    restored.iCurrentHp == profile->iMaximumHp && restored.iMaximumHp == profile->iMaximumHp &&
                    restored.iColosseumMatchId == 0u && !restored.bColosseumParticipant &&
                    !smallSource->m_PartyIdByPlayerId.contains(restored.iPlayerId) &&
                    returnApp->m_GameplayBindingBySessionId.at(sessionId).pSimulation == smallSource,
                    "Typed return preserves identity, restores normal health and clears match-party authority");
            }
            tests.Require(smallSource->Count_HumanPlayers() == count && smallTarget->Count_HumanPlayers() == 0u &&
                std::none_of(smallSource->m_Players.begin(), smallSource->m_Players.end(), [](const auto& p) { return p.second.Is_ColosseumMercenary(); }),
                "All small-match humans return while mercenaries stay out of Bern");
            std::cout << "Flexible lifecycle count=" << count << " autoOpponents=" << autoSelected << " returned=" << smallSource->Count_HumanPlayers() << '\n';
        }
        for (const auto& peer : peers) peer->m_isSendRunning.store(false);
    }
    for (unsigned i = 0; i < 3u; ++i) source->m_ColosseumQueue.push_back({ sessions[i]->Get_SessionId(), i + 1u });
    source->Try_FormColosseumMatch();
    tests.Require(source->m_PendingWorldTransfers.empty() && source->m_ColosseumQueue.size() == 3u,
        "Both configurations keep three humans waiting");
    source->m_ColosseumQueue.push_back({ sessions[3]->Get_SessionId(), 4u });
    source->Try_FormColosseumMatch();
    tests.Require(source->m_PendingWorldTransfers.empty(), "Even four humans wait for the same ten-second collection deadline");
    source->m_iServerTick = source->m_iColosseumQueueDeadline;
    source->Try_FormColosseumMatch();
    SERVER_WORLD_TRANSFER_REQUEST transfer;
    const bool staged = source->Try_DequeueWorldTransfer(transfer);
    tests.Require(staged && transfer.bColosseumMatch && transfer.PartyBatchSessionIds.size() == 4u &&
        source->m_ColosseumQueue.size() == 4u, "Deadline reserves one ordered immutable batch without erasing the queue");
    if (!staged) return 1;
    std::atomic_bool cancelBeforeLoad{ true };
    auto cancelledRoom = std::make_unique<CGameRoom>(WORLD_ID::COLOSSEUM, app->m_pActiveGameplayGeneration, &cancelBeforeLoad);
    tests.Require(!cancelledRoom->Is_Ready() && cancelledRoom->Get_Status() == "Room preparation cancelled",
        "Cooperative cancellation rejects construction before the first loader stage");
    // Synchronous preparation is explicit test setup only. Product admission must
    // consume a completed worker result and never load a room inside the transfer.
    app->m_PreparedColosseumRoom = std::make_shared<CGameRoom>(WORLD_ID::COLOSSEUM, app->m_pActiveGameplayGeneration);
    tests.Require(app->m_PreparedColosseumRoom->Is_Ready(), "Transfer fixture owns an already prepared room");
    // Exhaust one reliable FIFO after valid sessions and source bindings were staged.
    sessions[2]->m_iQueuedOutboundBytes = CClientSession::MAX_OUTBOUND_BYTE_COUNT;
    CServerApp::SESSION_WORLD_TRANSFER_FAILURE failure;
    tests.Require(!app->Transfer_SessionWorld(source, transfer, failure) &&
        failure.strContext.find("byte capacity is exhausted") != std::string::npos && source->Count_HumanPlayers() == 8u &&
        app->m_ColosseumMatches.empty() && source->m_ColosseumQueue.size() == 4u &&
        std::all_of(sessions.begin(), sessions.end(), [&](const auto& session) {
            return app->m_GameplayBindingBySessionId.at(session->Get_SessionId()).pSimulation == source;
        }), "Reliable capacity rejection preserves all four sources, queue and bindings");
    drain();
    const bool committed = app->Transfer_SessionWorld(source, transfer, failure);
    tests.Require(committed, "Real four-human navigation/profile/reliable transaction commits");
    if (!committed) { std::cout << failure.strContext << '\n'; return 1; }
    source->Notify_ColosseumTransferResult(true);
    app->m_PreparedColosseumRoom.reset();
    auto room = app->m_ColosseumMatches.begin()->second;
    tests.Require(source->Count_HumanPlayers() == 4u && source->m_ColosseumQueue.empty() &&
        room->m_Players.size() == 14u && room->Count_HumanPlayers() == 4u && room->m_ColosseumMercenaries.size() == 10u,
        "Commit creates one private four-human room and ten sessionless class candidates");
    for (const auto& [id, merc] : room->m_Players)
        if (merc.Is_ColosseumMercenary())
            std::cout << "Mercenary admitted team=" << static_cast<unsigned>(merc.iColosseumTeam)
                << " class=" << static_cast<unsigned>(merc.eCharacterClass) << " position="
                << merc.fPositionX << ',' << merc.fPositionY << ',' << merc.fPositionZ << '\n';
    const auto state = room->Build_ColosseumState();
    tests.Require(state.Players.size() == 14u && state.ePhase == COLOSSEUM_MATCH_PHASE::LOADING &&
        std::count_if(state.Players.begin(), state.Players.end(), [](const auto& p) { return p.bParticipant; }) == 4,
        "Persistent initial state marks only four humans as participants");
    const auto expectedReferenceHp = source->m_GameplayCatalog.Find_Boss("BOSS_VALTAN")->Get_DamageReferenceHp();
    const auto expectedMatchHp = expectedReferenceHp / 8u + (expectedReferenceHp % 8u ? 1u : 0u);
    tests.Require(expectedReferenceHp == 741285439u && expectedMatchHp == 92660680u &&
        source->m_GameplayCatalog.Find_Boss("BOSS_VALTAN")->iMaximumHp == 2100000000u,
        "New Colosseum admission preserves its original HP and damage scale when raid Valtan HP increases to 2.1 billion");
    tests.Require(std::all_of(room->m_Players.begin(), room->m_Players.end(), [expectedMatchHp, expectedReferenceHp](const auto& value) {
        return value.second.iMaximumHp == expectedMatchHp && value.second.iColosseumDamageReferenceHp == expectedReferenceHp && !value.second.isCombatReady &&
            (!value.second.Is_Human() || (value.second.Inventory.empty() && value.second.Purse.iSilver >= 77u));
    }), "Match has 20 Valtan HP bars with the full immutable damage reference and preserves empty inventory/purse");
    for (const auto& [id, ai] : room->m_ColosseumMercenaries)
    {
        const auto& merc = room->m_Players.at(id);
        std::vector<std::string> slots;
        bool valid = !ai.AvailableSkills.empty();
        for (const auto skillId : ai.AvailableSkills)
        {
            const auto* skill = room->m_GameplayCatalog.Find_Skill(skillId);
            valid = valid && skill &&
                skill->eCharacterClass == merc.eCharacterClass &&
                (skill->eRequiredStance == PLAYER_STANCE_ID::NONE || skill->eRequiredStance == merc.eStance);
            if (skill) slots.push_back(skill->strInputSlot);
        }
        tests.Require(valid, "Every mercenary stages actual class/stance bindings without a fixed Guide combo");
        {
            std::vector<SKILL_ID> allAvailable;
            for (const auto& [skillId, skill] : room->m_GameplayCatalog.Active().Get_Skills())
                if (skill.eCharacterClass == merc.eCharacterClass &&
                    (skill.eRequiredStance == PLAYER_STANCE_ID::NONE || skill.eRequiredStance == merc.eStance)) allAvailable.push_back(skillId);
            std::sort(allAvailable.begin(), allAvailable.end());
            auto admittedSkills = ai.AvailableSkills;
            std::sort(admittedSkills.begin(), admittedSkills.end());
            tests.Require(admittedSkills == allAvailable &&
                std::find(slots.begin(), slots.end(), "LMB") != slots.end() &&
                std::find(slots.begin(), slots.end(), "SPACE") != slots.end(),
                "All classes, including DimensionMaster, include every current published skill, LMB and SPACE");
        }
        std::cout << "Mercenary available class=" << static_cast<unsigned>(merc.eCharacterClass) << " slots=";
        for (const auto& slot : slots) std::cout << slot << ' ';
        std::cout << '\n';
    }
    tests.Require(room->m_Players.at(room->m_PlayerIdBySessionId.at(sessions[0]->Get_SessionId())).iColosseumTeam == 0u &&
        room->m_Players.at(room->m_PlayerIdBySessionId.at(sessions[1]->Get_SessionId())).iColosseumTeam == 1u &&
        room->m_Players.at(room->m_PlayerIdBySessionId.at(sessions[2]->Get_SessionId())).iColosseumTeam == 0u,
        "Acceptance order fixes teams 1/3 versus 2/4");
    for (unsigned i = 0u; i < 4u; ++i) room->Handle_ColosseumLoadReady(sessions[i]->Get_SessionId(), { room->m_iColosseumMatchId });
    room->Update_ColosseumMatch(1u);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::RECRUITING,
        "All human presentation readiness opens mercenary selection without combat authority");
    std::array<SESSION_ID, 2> recruiters{};
    std::array<std::vector<NET_ENTITY_ID>, 2> candidates;
    for (auto& [id, player] : room->m_Players)
        if (player.Is_Human()) recruiters[player.iColosseumTeam] = player.iSessionId;
        else candidates[player.iColosseumTeam].push_back(player.iNetEntityId);
    C2S_COLOSSEUM_RECRUIT request{ 1u, room->m_iColosseumMatchId, candidates[1][0] };
    room->Handle_ColosseumRecruit(recruiters[0], request);
    tests.Require(!room->m_Players.at(room->m_PlayerIdByEntityId.at(candidates[1][0])).bColosseumParticipant,
        "Recruit rejects the other team's candidate");
    for (std::uint8_t team = 0; team < 2u; ++team)
    {
        request.iRequestSequence = 2u; request.iMercenaryNetEntityId = candidates[team][0];
        room->Handle_ColosseumRecruit(recruiters[team], request);
        room->Handle_ColosseumRecruit(recruiters[team], request);
        tests.Require(room->m_PartyMembersByPartyId.at(room->m_ColosseumTeamPartyIds[team]).size() == 3u,
            "Duplicate recruit selects one mercenary exactly once");
        request.iRequestSequence = 3u; request.iMercenaryNetEntityId = candidates[team][1];
        room->Handle_ColosseumRecruit(recruiters[team], request);
    }
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ENTRY_COUNTDOWN &&
        room->m_iColosseumPhaseEnd - room->m_iColosseumPhaseStart == 300u &&
        std::count_if(room->m_Players.begin(), room->m_Players.end(), [](const auto& p) { return p.second.bColosseumParticipant; }) == 8,
        "Two selected mercenaries per team prepare eight participants and a ten-second entry countdown");
    const auto entryDeadline = room->m_iColosseumPhaseEnd;
    room->Update_ColosseumMatch(entryDeadline - 1u);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ENTRY_COUNTDOWN &&
        std::none_of(room->m_Players.begin(), room->m_Players.end(), [](const auto& p) { return p.second.bColosseumCombatActive; }),
        "Entry countdown blocks all damage before the deadline");
    room->Update_ColosseumMatch(entryDeadline);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::INTRO && room->m_iColosseumPhaseEnd == entryDeadline + 258u,
        "Ten-second entry transitions to one shared 8.6-second lineup clock");
    room->Update_ColosseumMatch(room->m_iColosseumPhaseEnd);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::COUNTDOWN,
        "Lineup completion starts the original gate countdown");
    room->Update_ColosseumMatch(room->m_iColosseumPhaseEnd);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ACTIVE,
        "Gate countdown completion enables eight participant combat");
    request.iRequestSequence = 4u; request.iMercenaryNetEntityId = candidates[0][2];
    room->Handle_ColosseumRecruit(recruiters[0], request);
    tests.Require(!room->m_Players.at(room->m_PlayerIdByEntityId.at(candidates[0][2])).bColosseumParticipant,
        "A third mercenary remains inert after the match starts");
    const auto before = room->m_Players.at(room->m_PlayerIdByEntityId.at(candidates[0][2]));
    room->Update_Colosseum(.25f);
    const auto& after = room->m_Players.at(before.iPlayerId);
    tests.Require(!after.hasMoveGoal && after.eAction == PLAYER_ACTION_STATE::NONE &&
        after.fPositionX == before.fPositionX && after.fPositionZ == before.fPositionZ,
        "Unselected candidate cannot acquire movement or a skill");
    tests.Require(std::any_of(room->m_Players.begin(), room->m_Players.end(), [](const auto& p) {
        return p.second.Is_ColosseumMercenary() && p.second.bColosseumParticipant &&
            (p.second.hasMoveGoal || p.second.eAction == PLAYER_ACTION_STATE::SKILL);
    }), "Selected mercenary targets the opposing team through the shared executor");
    for (unsigned i = 4; i < 8u; ++i) source->m_ColosseumQueue.push_back({ sessions[i]->Get_SessionId(), i + 1u });
    source->Try_FormColosseumMatch();
    source->m_iServerTick = source->m_iColosseumQueueDeadline;
    source->Try_FormColosseumMatch();
    SERVER_WORLD_TRANSFER_REQUEST second;
    const bool dequeued = source->Try_DequeueWorldTransfer(second);
    const auto beginTime = std::chrono::steady_clock::now();
    const bool preparing = dequeued && app->Begin_ColosseumPreparation(source, second);
    const auto launchMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - beginTime).count();
    tests.Require(preparing && app->m_ColosseumPreparationWorker.valid() && source->Count_HumanPlayers() == 4u &&
        app->m_ColosseumMatches.size() == 1u && source->m_ColosseumQueue.size() == 4u,
        "Async preparation retains four source humans, queue and authority until ready");
    tests.Require(!app->Begin_ColosseumPreparation(source, second), "Only one preparation worker can own the queued batch");
    std::cout << "Colosseum preparation launchMs=" << launchMs << '\n';
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::minutes(2);
    while (app->m_ColosseumPreparationWorker.valid() && std::chrono::steady_clock::now() < deadline)
    {
        app->Advance_ColosseumPreparation();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    const bool next = !app->m_ColosseumPreparationWorker.valid() && app->m_ColosseumMatches.size() == 2u;
    tests.Require(next && app->m_ColosseumMatches.size() == 2u,
        "The next four humans receive another private simulation");
    if (next)
    {
        const auto other = app->m_ColosseumMatches.rbegin()->second;
        tests.Require(other != room && other->m_iColosseumMatchId != room->m_iColosseumMatchId &&
            other->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::LOADING,
            "Match authority and recruitment state are isolated despite reused room-local entity IDs");
    }
    // An unresolved result must never block the tick or publish prepared authority.
    if (!app->m_ColosseumPreparationWorker.valid())
    {
        std::promise<CServerApp::COLOSSEUM_PREPARATION_RESULT> delayed;
        app->m_ColosseumPreparationWorker = delayed.get_future();
        app->m_ColosseumPreparationSource = source;
        app->m_ColosseumPreparationTransfer = second;
        app->m_ColosseumPreparationCancelled = std::make_shared<std::atomic_bool>(false);
        app->Advance_ColosseumPreparation();
        tests.Require(app->m_ColosseumPreparationWorker.valid() && app->m_ColosseumPreparationCancelled->load() &&
            app->m_ColosseumMatches.size() == 2u,
            "Pending future poll returns and cancels when its source binding has changed");
        delayed.set_value({});
        app->Advance_ColosseumPreparation();
        tests.Require(!app->m_ColosseumPreparationWorker.valid() && app->m_ColosseumMatches.size() == 2u,
            "A cancelled completion cannot create or rebind a match");
    }
    const auto defeatedSession = recruiters[1];
    auto& defeated = room->m_Players.at(room->m_PlayerIdBySessionId.at(defeatedSession));
    defeated.iCurrentHp = 0u; defeated.eAction = PLAYER_ACTION_STATE::DEAD;
    room->Handle_UseSquareHole(defeatedSession, C2S_USE_SQUAREHOLE{ 90u, 1u });
    tests.Require(room->m_PendingWorldTransfers.empty(), "An ACTIVE defeated human cannot bypass the arena through a square hole");
    for (auto& [id, player] : room->m_Players)
        if (player.iColosseumTeam == 1u && player.bColosseumParticipant) player.iCurrentHp = 0u;
    room->Update_Colosseum(.1f);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ACTIVE, "A team death does not truncate the timed respawning match");
    room->m_iColosseumScores[0] = 1u;
    room->Update_ColosseumMatch(room->m_iColosseumPhaseEnd);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::FINISHED && room->m_iColosseumWinnerTeam == 0u &&
        std::none_of(room->m_Players.begin(), room->m_Players.end(), [](const auto& p) { return p.second.isCombatReady; }),
        "The 120-second deadline determines the score winner and closes every damage authority");
    drain();
    room->Handle_UseSquareHole(defeatedSession, C2S_USE_SQUAREHOLE{ 91u, 1u });
    SERVER_WORLD_TRANSFER_REQUEST returnToBern;
    const bool returned = room->Try_DequeueWorldTransfer(returnToBern) &&
        app->Transfer_SessionWorld(room, returnToBern, failure);
    tests.Require(returned, "A FINISHED defeated human returns through the normal atomic Bern transfer");
    if (returned)
    {
        const auto& human = source->m_Players.at(source->m_PlayerIdBySessionId.at(defeatedSession));
        const auto* profile = source->m_GameplayCatalog.Find_Player(human.eCharacterClass);
        std::cout << "Returned Guardian normal HP=" << human.iMaximumHp << " activeProfile=" << (profile ? profile->iMaximumHp : 0u) << '\n';
        tests.Require(profile && human.eCharacterClass == CHARACTER_CLASS_ID::GUARDIANKNIGHT &&
            human.iCurrentHp == profile->iMaximumHp && human.iMaximumHp == profile->iMaximumHp &&
            human.eAction == PLAYER_ACTION_STATE::NONE && human.iColosseumMatchId == 0u && human.iColosseumDamageReferenceHp == 0u && !human.bColosseumParticipant &&
            !source->m_PartyIdByPlayerId.contains(human.iPlayerId) &&
            std::none_of(source->m_Players.begin(), source->m_Players.end(), [](const auto& value) { return value.second.Is_ColosseumMercenary(); }),
            "Bern restores the actual normal Guardian profile HP and carries no match, mercenary or automatic team party authority");
    }
    // Drive the real AI decision and shared movement executor; malformed geometry
    // would take the attack branch instead of choosing a lower-risk navigation goal.
    for (unsigned shapeKind = 0; shapeKind < 5u; ++shapeKind)
    {
        auto catalog = std::make_shared<CGameplayCatalog>(source->m_GameplayCatalog.Active());
        auto* skill = const_cast<PLAYER_SKILL_DEFINITION*>(catalog->Find_Skill(34040u));
        tests.Require(skill != nullptr, "Evade fixture owns a mutable copy of a published caster skill");
        if (!skill) continue;
        skill->Hits.clear(); skill->ComboStages.clear();
        PLAYER_SKILL_HIT hit;
        hit.iAreaType = shapeKind < 2u ? 1u : shapeKind == 2u ? 2u : 3u;
        hit.fRange = 1.5f; hit.fWidth = shapeKind == 2u ? 3.f : 0.f;
        hit.fInner = shapeKind == 1u ? .2f : 0.f;
        hit.fAngleDegrees = shapeKind == 3u ? 60.f : 0.f;
        hit.iTimeMs = 100u;
        skill->Hits.push_back(hit);
        auto hazardRoom = std::make_unique<CGameRoom>(WORLD_ID::COLOSSEUM, catalog);
        SERVER_NAV_POINT landing;
        const bool standable = hazardRoom->Is_Ready() &&
            hazardRoom->m_ServerNavigation.Sample_Position(-20.4f, -2.6f, landing, 12.22f);
        tests.Require(standable, "Evade fixture uses the actual Colosseum navigation surface");
        if (!standable) continue;
        hazardRoom->m_iColosseumMatchId = 900u;
        hazardRoom->m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::ACTIVE;
        const auto* profile = catalog->Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER);
        SERVER_PLAYER merc;
        merc.iPlayerId = 1u; merc.iNetEntityId = 101u;
        merc.eControlKind = PLAYER_CONTROL_KIND::COLOSSEUM_MERCENARY_AI;
        merc.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
        merc.eStance = profile->eDefaultStance;
        merc.iCurrentHp = merc.iMaximumHp = expectedMatchHp;
        merc.iColosseumDamageReferenceHp = expectedReferenceHp;
        merc.iCurrentResource = merc.iMaximumResource = profile->iMaximumResource;
        merc.fMoveSpeed = profile->fMoveSpeed;
        merc.iColosseumMatchId = 900u; merc.iColosseumTeam = 0u;
        merc.bColosseumParticipant = true; merc.isCombatReady = true; merc.bColosseumCombatActive = true;
        merc.CooldownEndTickBySkillId[34020u] = 100000u; // Existing navigation fallback when SPACE is unavailable.
        merc.fPositionX = landing.x; merc.fPositionY = landing.y; merc.fPositionZ = landing.z;
        auto caster = merc;
        caster.iPlayerId = 2u; caster.iNetEntityId = 102u; caster.iColosseumTeam = 1u;
        caster.eControlKind = PLAYER_CONTROL_KIND::HUMAN;
        caster.eAction = PLAYER_ACTION_STATE::SKILL; caster.iCurrentSkillId = skill->iSkillId;
        caster.fYawDegrees = 0.f;
        hazardRoom->m_Players.emplace(merc.iPlayerId, merc);
        hazardRoom->m_Players.emplace(caster.iPlayerId, caster);
        hazardRoom->m_ColosseumMercenaries.emplace(merc.iPlayerId, CGameRoom::COLOSSEUM_MERCENARY_RUNTIME{});
        hazardRoom->Update_Colosseum(.25f);
        const auto& evading = hazardRoom->m_Players.at(merc.iPlayerId);
        tests.Require(evading.hasMoveGoal && evading.eAction == PLAYER_ACTION_STATE::NONE &&
            std::hypot(evading.fMoveGoalX - landing.x, evading.fMoveGoalZ - landing.z) > .5f &&
            hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId).Threats.At(evading.fMoveGoalX, evading.fPositionY, evading.fMoveGoalZ).risk == 0.f,
            "Circle/ring/box/cone/full-cone future hit selects an actual navigation escape instead of attacking");
        if (shapeKind == 0u)
        {
            auto& dodgeActor = hazardRoom->m_Players.at(merc.iPlayerId);
            dodgeActor = merc; dodgeActor.CooldownEndTickBySkillId.erase(34020u);
            hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId) = {};
            hazardRoom->Update_Colosseum(.25f);
            const auto* admittedDodge = catalog->Find_Skill(dodgeActor.iCurrentSkillId);
            tests.Require(admittedDodge && Is_DodgeSkill(*admittedDodge) && dodgeActor.eAction == PLAYER_ACTION_STATE::SKILL,
                "A non-DimensionMaster uses available SPACE toward the validated escape instead of suppressing dodge skills");
            // Real published manual/automatic chains, through the actual room AI
            // input and native skill Update. No stage is advanced by the test.
            const std::array<SKILL_ID, 10> chainSkills{ 17080u, 34160u, 34140u, 31210u, 49110u, 17080u, 17000u, 34010u, 31000u, 49000u };
            for (std::size_t chainIndex = 0u; chainIndex < chainSkills.size(); ++chainIndex)
            {
                auto* chain = const_cast<PLAYER_SKILL_DEFINITION*>(catalog->Find_Skill(chainSkills[chainIndex]));
                const auto* chainProfile = chain ? catalog->Find_Player(chain->eCharacterClass) : nullptr;
                tests.Require(chain && chainProfile && chain->ComboStages.size() >= 2u,
                    "Native continuation fixture owns the published COMBO stage contract");
                if (!chain || !chainProfile || chain->ComboStages.size() < 2u) continue;
                const auto originalOpen = chain->ComboStages.front().iInputOpenMs;
                const auto originalClose = chain->ComboStages.front().iInputCloseMs;
                if (chainIndex == 5u)
                {
                    // 260..290ms is missed by the 200ms tactical clock but contains
                    // a real 30 Hz fixed tick. Keep the native executor unchanged.
                    chain->ComboStages.front().iInputOpenMs = 260u;
                    chain->ComboStages.front().iInputCloseMs = 290u;
                }
                SERVER_PLAYER actor = merc;
                actor.eCharacterClass = chain->eCharacterClass; actor.eStance = chainProfile->eDefaultStance;
                actor.iCurrentResource = actor.iMaximumResource = chainProfile->iMaximumResource;
                actor.iMaximumIdentity = chainProfile->iMaximumIdentity;
                actor.fMoveSpeed = chainProfile->fMoveSpeed;
                CPlayerSkillSystem::Reset_Gauges(actor, hazardRoom->m_GameplayCatalog);
                auto& controlled = hazardRoom->m_Players.at(merc.iPlayerId);
                controlled = actor;
                auto& target = hazardRoom->m_Players.at(caster.iPlayerId);
                target.eAction = PLAYER_ACTION_STATE::NONE; target.iCurrentSkillId = INVALID_SKILL_ID;
                target.fPositionX = landing.x; target.fPositionY = landing.y; target.fPositionZ = landing.z + 1.f;
                auto& chainAi = hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId);
                chainAi = {};
                const auto admittedChain = std::find_if(room->m_ColosseumMercenaries.begin(), room->m_ColosseumMercenaries.end(),
                    [&](const auto& value) { return room->m_Players.at(value.first).eCharacterClass == chain->eCharacterClass; });
                tests.Require(admittedChain != room->m_ColosseumMercenaries.end() && !admittedChain->second.AvailableSkills.empty(),
                    "Native continuation class has admitted available skill bindings");
                if (admittedChain == room->m_ColosseumMercenaries.end() || admittedChain->second.AvailableSkills.empty()) continue;
                SKILL_ID nextSkill = INVALID_SKILL_ID;
                for (const auto& [id, definition] : catalog->Get_Skills())
                    if (definition.eCharacterClass == actor.eCharacterClass)
                    {
                        controlled.CooldownEndTickBySkillId[id] = hazardRoom->m_iServerTick + 100000u;
                        if (definition.strInputSlot == "Q" &&
                            (definition.eRequiredStance == PLAYER_STANCE_ID::NONE || definition.eRequiredStance == actor.eStance)) nextSkill = id;
                    }
                controlled.CooldownEndTickBySkillId.erase(chain->iSkillId);
                chainAi.AvailableSkills = admittedChain->second.AvailableSkills;
                hazardRoom->Update_Colosseum(.25f);
                const auto lastAfterStart = chainAi.iLastSkillId;
                const auto recentAfterStart = chainAi.RecentSkills;
                const auto recentCursorAfterStart = chainAi.iRecentSkillCursor;
                const bool chainStarted = controlled.eAction == PLAYER_ACTION_STATE::SKILL && controlled.iCurrentSkillId == chain->iSkillId;
                const auto paidResource = controlled.iCurrentResource;
                const auto paidCooldown = controlled.CooldownEndTickBySkillId[chain->iSkillId];
                std::size_t maximumStage = controlled.iComboStage;
                unsigned buffered = 0u;
                bool ordered = chainStarted, inputWindowsOnly = true;
                std::vector<SERVER_WORLD_ENTITY> emptyWorld;
                std::vector<DAMAGE_EVENT> damage;
                for (unsigned tick = 0u; chainStarted && tick < 900u && controlled.eAction != PLAYER_ACTION_STATE::NONE; ++tick)
                {
                    const auto previousSequence = controlled.iLastSkillSequence;
                    const auto previousStage = controlled.iComboStage;
                    const float beforeMs = controlled.fActionElapsedSeconds * 1000.f;
                    hazardRoom->Update_Colosseum(1.f / 30.f);
                    if (controlled.iLastSkillSequence != previousSequence)
                    {
                        ++buffered;
                        const auto& stage = chain->ComboStages[previousStage - 1u];
                        inputWindowsOnly = inputWindowsOnly && stage.iInputCloseMs > 0u &&
                            beforeMs >= stage.iInputOpenMs && beforeMs <= stage.iInputCloseMs && controlled.hasBufferedComboInput;
                    }
                    ordered = ordered && chainAi.iLastSkillId == lastAfterStart && chainAi.RecentSkills == recentAfterStart &&
                        chainAi.iRecentSkillCursor == recentCursorAfterStart && controlled.iCurrentSkillId == chain->iSkillId &&
                        controlled.PendingCommand.eKind == PLAYER_PENDING_COMMAND_KIND::NONE;
                    hazardRoom->m_PlayerSkillSystem.Update(controlled, emptyWorld, hazardRoom->m_GameplayCatalog,
                        &hazardRoom->m_ServerNavigation, nullptr, 1.f / 30.f, ++hazardRoom->m_iServerTick, damage);
                    maximumStage = (std::max)(maximumStage, static_cast<std::size_t>(controlled.iComboStage));
                }
                const auto expectedBuffers = std::count_if(chain->ComboStages.begin(), chain->ComboStages.end() - 1,
                    [](const auto& stage) { return stage.iInputCloseMs > 0u; });
                tests.Require(chainStarted && ordered && inputWindowsOnly && controlled.eAction == PLAYER_ACTION_STATE::NONE &&
                    maximumStage == chain->ComboStages.size() && buffered == expectedBuffers &&
                    controlled.iCurrentResource == paidResource && controlled.CooldownEndTickBySkillId[chain->iSkillId] == paidCooldown,
                    "Every native stage completes; only manual windows buffer once, with no extra cost, cooldown or next-slot approval");
                std::cout << "Mercenary native chain skill=" << chain->iSkillId << " narrow=" << (chainIndex == 5u)
                    << " stages=" << maximumStage << '/' << chain->ComboStages.size() << " buffers=" << buffered << '/' << expectedBuffers << '\n';
                controlled.fPositionX = landing.x; controlled.fPositionY = landing.y; controlled.fPositionZ = landing.z;
                controlled.CooldownEndTickBySkillId[chain->iSkillId] = hazardRoom->m_iServerTick + 100000u;
                controlled.CooldownEndTickBySkillId.erase(nextSkill);
                chainAi.fThinkElapsed = 1.f;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.eAction == PLAYER_ACTION_STATE::SKILL && controlled.iCurrentSkillId == nextSkill,
                    "Only the completed native COMBO opens another available skill on the following decision");
                chain->ComboStages.front().iInputOpenMs = originalOpen;
                chain->ComboStages.front().iInputCloseMs = originalClose;
            }
            // Fresh actors isolate new-cast admission while preserving each random
            // stream and recent-skill memory; normal simulation advances these clocks.
            for (const auto classId : { CHARACTER_CLASS_ID::DIMENSIONMASTER, CHARACTER_CLASS_ID::LANCE_MASTER,
                CHARACTER_CLASS_ID::WARLORD, CHARACTER_CLASS_ID::GUARDIANKNIGHT, CHARACTER_CLASS_ID::ARTIST })
            {
                auto& actor = hazardRoom->m_Players.at(merc.iPlayerId);
                auto& selection = hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId);
                const auto* actorProfile = catalog->Find_Player(classId);
                auto fresh = merc;
                fresh.eCharacterClass = classId; fresh.eStance = actorProfile->eDefaultStance;
                fresh.iCurrentResource = fresh.iMaximumResource = actorProfile->iMaximumResource;
                fresh.iMaximumIdentity = actorProfile->iMaximumIdentity;
                fresh.fMoveSpeed = actorProfile->fMoveSpeed; fresh.CooldownEndTickBySkillId.clear();
                CPlayerSkillSystem::Reset_Gauges(fresh, hazardRoom->m_GameplayCatalog);
                auto& target = hazardRoom->m_Players.at(caster.iPlayerId);
                target = caster; target.eAction = PLAYER_ACTION_STATE::NONE; target.iCurrentSkillId = INVALID_SKILL_ID;
                target.fPositionZ = landing.z + 1.f;
                const auto trace = [&](const std::uint64_t seed)
                {
                    selection = {}; selection.iRandomState = seed;
                    std::vector<SKILL_ID> result;
                    bool valid = true;
                    for (unsigned sample = 0u; sample < 32u; ++sample)
                    {
                        hazardRoom->m_iServerTick = 10000u + sample * 120u;
                        actor = fresh; selection.iNextAttackTick = 0u; selection.fThinkElapsed = 1.f;
                        hazardRoom->Update_Colosseum(.25f);
                        const auto* running = catalog->Find_Skill(actor.iCurrentSkillId);
                        valid = valid && actor.eAction == PLAYER_ACTION_STATE::SKILL && running &&
                            running->eCharacterClass == classId && !Is_DodgeSkill(*running) &&
                            running->eSkillKind != PLAYER_SKILL_KIND::STANDUP && running->strInputSlot != "ALT_V" &&
                            (running->eRequiredStance == PLAYER_STANCE_ID::NONE || running->eRequiredStance == fresh.eStance) &&
                            selection.iLastSkillId == actor.iCurrentSkillId;
                        result.push_back(actor.iCurrentSkillId);
                    }
                    tests.Require(valid, "Weighted draws admit only actual ready class/stance attacks and commit successful-cast memory");
                    return result;
                };
                const auto first = trace(0x123456789abcdefu);
                const auto replay = trace(0x123456789abcdefu);
                const auto different = trace(0x987654321abcdefu);
                tests.Require(first == replay && first != different && std::set<SKILL_ID>(first.begin(), first.end()).size() >= 3u,
                    "Every class has reproducible seeded choices and multiple different ready skills instead of a fixed rotation");
                actor = fresh; actor.bPatternBound = true;
                selection.iNextAttackTick = 0u; selection.fThinkElapsed = 1.f;
                const auto previousLast = selection.iLastSkillId;
                const auto previousRecent = selection.RecentSkills;
                const auto previousCursor = selection.iRecentSkillCursor;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE && selection.iLastSkillId == previousLast &&
                    selection.RecentSkills == previousRecent && selection.iRecentSkillCursor == previousCursor,
                    "Crowd control admits no new skill and preserves the successful-cast history");
                actor = fresh;
                for (const auto& [id, definition] : catalog->Get_Skills())
                    if (definition.eCharacterClass == classId) actor.CooldownEndTickBySkillId[id] = hazardRoom->m_iServerTick + 10000u;
                selection.iNextAttackTick = 0u; selection.fThinkElapsed = 1.f;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE && selection.iLastSkillId == previousLast &&
                    selection.RecentSkills == previousRecent && selection.iRecentSkillCursor == previousCursor,
                    "Unavailable skills do not consume successful-cast memory or fabricate an attack");
                if (classId == CHARACTER_CLASS_ID::GUARDIANKNIGHT)
                {
                    actor = fresh; actor.iEmberOrbs = 0u; selection = {};
                    const PLAYER_SKILL_DEFINITION* expression = nullptr;
                    for (const auto& [id, definition] : catalog->Get_Skills())
                        if (definition.eCharacterClass == classId && definition.iEmberCost > 0u &&
                            (definition.eRequiredStance == PLAYER_STANCE_ID::NONE || definition.eRequiredStance == actor.eStance))
                        { expression = &definition; break; }
                    tests.Require(expression != nullptr, "Guardian fixture resolves a native human-form ember expression skill");
                    if (expression)
                    {
                        for (const auto& [id, definition] : catalog->Get_Skills())
                            if (definition.eCharacterClass == classId && id != expression->iSkillId)
                                actor.CooldownEndTickBySkillId[id] = hazardRoom->m_iServerTick + 10000u;
                        auto native = actor;
                        C2S_USE_SKILL request; request.iClientSequence = 1u; request.iSkillId = expression->iSkillId;
                        request.eTargetIntent = expression->eTargetIntent; request.fAimX = target.fPositionX; request.fAimZ = target.fPositionZ;
                        const bool nativeAccepted = hazardRoom->m_PlayerSkillSystem.Try_Start(native, request, *catalog,
                            hazardRoom->m_iServerTick + 1u, &hazardRoom->m_ServerNavigation);
                        hazardRoom->Update_Colosseum(.25f);
                        tests.Require(nativeAccepted && actor.eAction == PLAYER_ACTION_STATE::SKILL &&
                            actor.iCurrentSkillId == expression->iSkillId && actor.iEmberOrbs == 0u &&
                            selection.iLastSkillId == expression->iSkillId,
                            "Guardian AI preserves native zero-ember expression admission instead of adding a stricter resource gate");
                    }
                }
                actor = fresh; actor.eAction = PLAYER_ACTION_STATE::KNOCKDOWN;
                actor.iKnockdownEndTick = hazardRoom->m_iServerTick + 300u; selection = {};
                hazardRoom->Update_Colosseum(.25f);
                const auto* standup = catalog->Find_Skill(actor.iCurrentSkillId);
                const bool hasPublishedStandup = std::any_of(catalog->Get_Skills().begin(), catalog->Get_Skills().end(),
                    [&](const auto& entry) { return entry.second.eCharacterClass == classId &&
                        entry.second.eSkillKind == PLAYER_SKILL_KIND::STANDUP; });
                if (classId == CHARACTER_CLASS_ID::GUARDIANKNIGHT)
                    tests.Require(!hasPublishedStandup && !standup && actor.eAction == PLAYER_ACTION_STATE::KNOCKDOWN &&
                        actor.iKnockdownEndTick == hazardRoom->m_iServerTick + 300u &&
                        actor.PendingCommand.eKind == PLAYER_PENDING_COMMAND_KIND::NONE,
                        "Guardian has no published stand-up and waits for native knockdown recovery without fabricating a skill");
                else
                    tests.Require(hasPublishedStandup && standup && standup->eSkillKind == PLAYER_SKILL_KIND::STANDUP &&
                        actor.eAction == PLAYER_ACTION_STATE::SKILL && actor.iKnockdownEndTick == 0u &&
                        selection.eTactic == CGameRoom::COLOSSEUM_TACTIC::RECOVER,
                        "All four classes with a published stand-up, including DimensionMaster, use native knockdown recovery admission");
                actor = fresh; target = caster; selection = {};
                hazardRoom->Update_Colosseum(.25f);
                const auto* dodge = catalog->Find_Skill(actor.iCurrentSkillId);
                tests.Require(dodge && Is_DodgeSkill(*dodge) && actor.eAction == PLAYER_ACTION_STATE::SKILL &&
                    selection.eTactic == CGameRoom::COLOSSEUM_TACTIC::EVADE,
                    "Every class including DimensionMaster uses native SPACE against an imminent enemy hit");
                std::vector<SERVER_WORLD_ENTITY> dodgeWorld;
                std::vector<DAMAGE_EVENT> dodgeDamage;
                for (unsigned tick = 0u; tick < 300u && actor.eAction != PLAYER_ACTION_STATE::NONE; ++tick)
                    hazardRoom->m_PlayerSkillSystem.Update(actor, dodgeWorld, hazardRoom->m_GameplayCatalog,
                        &hazardRoom->m_ServerNavigation, nullptr, 1.f / 30.f, ++hazardRoom->m_iServerTick, dodgeDamage);
                tests.Require(dodge && actor.eAction == PLAYER_ACTION_STATE::NONE &&
                    selection.Threats.At(actor.fPositionX, actor.fPositionY, actor.fPositionZ).risk == 0.f &&
                    std::hypot(actor.fPositionX - landing.x, actor.fPositionZ - landing.z) > 1.5f,
                    "Native SPACE root motion completes at an actually safe position for every class, including abrupt and backward-tail curves");
            }
            // A soft repetition penalty lowers likelihood; it deliberately permits
            // a repeated cast when it remains the only usable option.
            {
                auto& actor = hazardRoom->m_Players.at(merc.iPlayerId);
                auto& selection = hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId);
                auto& target = hazardRoom->m_Players.at(caster.iPlayerId);
                target = caster; target.eAction = PLAYER_ACTION_STATE::NONE; target.iCurrentSkillId = INVALID_SKILL_ID;
                target.fPositionZ = landing.z + 1.f;
                const auto countQ = [&](const bool penalized)
                {
                    unsigned count = 0u;
                    for (unsigned sample = 1u; sample <= 128u; ++sample)
                    {
                        hazardRoom->m_iServerTick = 20000u;
                        actor = merc; selection = {}; selection.iRandomState = 0x123456789abcdefu * sample;
                        for (const auto& [id, definition] : catalog->Get_Skills())
                            if (definition.eCharacterClass == actor.eCharacterClass && id != 34040u && id != 34090u)
                                actor.CooldownEndTickBySkillId[id] = 30000u;
                        if (penalized) { selection.iLastSkillId = 34040u; selection.RecentSkills.fill(34040u); }
                        hazardRoom->Update_Colosseum(.25f);
                        if (actor.iCurrentSkillId == 34040u) ++count;
                    }
                    return count;
                };
                const auto unpenalized = countQ(false), penalized = countQ(true);
                tests.Require(unpenalized > 20u && penalized < unpenalized / 2u,
                    "Recent successful skills receive a measurable soft penalty in otherwise identical weighted draws");
                actor = merc; selection = {}; selection.iLastSkillId = 34040u; selection.RecentSkills.fill(34040u);
                for (const auto& [id, definition] : catalog->Get_Skills())
                    if (definition.eCharacterClass == actor.eCharacterClass && id != 34040u)
                        actor.CooldownEndTickBySkillId[id] = 30000u;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(actor.iCurrentSkillId == 34040u && actor.eAction == PLAYER_ACTION_STATE::SKILL,
                    "A recent skill remains usable when all alternatives are unavailable");
            }
            // Drive both real AI selectors against the published ALT_V binding.
            // Player resets below isolate command admission; the production
            // death/respawn call must preserve the match-owned interval.
            for (const auto classId : { CHARACTER_CLASS_ID::DIMENSIONMASTER, CHARACTER_CLASS_ID::LANCE_MASTER,
                CHARACTER_CLASS_ID::WARLORD, CHARACTER_CLASS_ID::GUARDIANKNIGHT, CHARACTER_CLASS_ID::ARTIST })
            {
                const auto* actorProfile = catalog->Find_Player(classId);
                const PLAYER_SKILL_DEFINITION* awakening = nullptr;
                for (const auto& [id, definition] : catalog->Get_Skills())
                    if (definition.eCharacterClass == classId && definition.strInputSlot == "ALT_V") awakening = &definition;
                tests.Require(actorProfile && awakening && awakening->eSkillKind == PLAYER_SKILL_KIND::ACTIVE,
                    "Every Colosseum mercenary resolves its actual published ALT_V active skill");
                if (!actorProfile || !awakening) continue;
                auto& actor = hazardRoom->m_Players.at(merc.iPlayerId);
                auto& interval = hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId);
                auto& target = hazardRoom->m_Players.at(caster.iPlayerId);
                target.eAction = PLAYER_ACTION_STATE::NONE; target.iCurrentSkillId = INVALID_SKILL_ID;
                target.fPositionX = landing.x; target.fPositionY = landing.y; target.fPositionZ = landing.z + 1.f;
                const auto blockOtherSkills = [&](SERVER_PLAYER& player)
                {
                    for (const auto& [id, definition] : catalog->Get_Skills())
                        if (definition.eCharacterClass == classId && id != awakening->iSkillId)
                            player.CooldownEndTickBySkillId[id] = hazardRoom->m_iServerTick + 100000u;
                };
                const auto freshActor = [&]()
                {
                    auto player = merc;
                    player.eCharacterClass = classId; player.eStance = actorProfile->eDefaultStance;
                    player.iCurrentResource = player.iMaximumResource = actorProfile->iMaximumResource;
                    player.iMaximumIdentity = actorProfile->iMaximumIdentity; player.fMoveSpeed = actorProfile->fMoveSpeed;
                    player.fColosseumSpawnX = landing.x; player.fColosseumSpawnY = landing.y;
                    player.fColosseumSpawnZ = landing.z; player.fColosseumSpawnYaw = player.fYawDegrees;
                    player.CooldownEndTickBySkillId.clear();
                    CPlayerSkillSystem::Reset_Gauges(player, hazardRoom->m_GameplayCatalog);
                    blockOtherSkills(player);
                    return player;
                };
                const auto decide = [&]()
                {
                    interval.fThinkElapsed = 1.f; interval.iNextAttackTick = 0u;
                    hazardRoom->Update_Colosseum(.25f);
                };
                constexpr std::uint32_t firstTick = 200000u;
                hazardRoom->m_iServerTick = firstTick;
                hazardRoom->m_iColosseumPhaseEnd = firstTick + 3600u;
                hazardRoom->m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::ACTIVE;
                actor = freshActor(); interval = {}; interval.AvailableSkills = { awakening->iSkillId };
                actor.CooldownEndTickBySkillId[awakening->iSkillId] = firstTick + 1200u;
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE && !interval.iLastAltVAdmissionTick,
                    "A native cooldown rejection does not consume the mercenary ALT_V interval");
                actor.CooldownEndTickBySkillId.erase(awakening->iSkillId);
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::SKILL && actor.iCurrentSkillId == awakening->iSkillId &&
                    interval.iLastAltVAdmissionTick == firstTick,
                    "An accepted ALT_V starts that mercenary's thirty-second admission interval");

                actor.iCurrentHp = 0u; actor.eAction = PLAYER_ACTION_STATE::DEAD;
                actor.iColosseumRespawnTick = firstTick + 90u;
                hazardRoom->m_iServerTick = firstTick + 90u;
                hazardRoom->Update_ColosseumMatch(hazardRoom->m_iServerTick);
                tests.Require(actor.iCurrentHp == expectedMatchHp && actor.iMaximumHp == expectedMatchHp &&
                    actor.CooldownEndTickBySkillId.empty() && interval.iLastAltVAdmissionTick == firstTick,
                    "Actual arena respawn restores twenty-bar HP and clears native cooldowns while preserving the ALT_V interval");
                blockOtherSkills(actor);
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE && interval.iLastAltVAdmissionTick == firstTick,
                    "Respawn cannot authorize another mercenary ALT_V before thirty seconds");
                hazardRoom->m_iServerTick = firstTick + 899u; actor = freshActor();
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE && interval.iLastAltVAdmissionTick == firstTick,
                    "The same mercenary ALT_V stays blocked at the 899-tick boundary");
                hazardRoom->m_iServerTick = firstTick + 900u; actor = freshActor();
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::SKILL && actor.iCurrentSkillId == awakening->iSkillId &&
                    interval.iLastAltVAdmissionTick == firstTick + 900u,
                    "The same mercenary ALT_V is admitted at 900 ticks when its native cooldown permits");

                hazardRoom->m_iServerTick = firstTick + 1800u; actor = freshActor();
                actor.CooldownEndTickBySkillId[awakening->iSkillId] = firstTick + 2700u;
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE &&
                    actor.CooldownEndTickBySkillId.at(awakening->iSkillId) == firstTick + 2700u &&
                    interval.iLastAltVAdmissionTick == firstTick + 900u,
                    "An existing longer native cooldown remains unchanged after the thirty-second minimum expires");
                auto peer = freshActor(); peer.iPlayerId = 3u; peer.iNetEntityId = 103u;
                hazardRoom->m_Players.emplace(peer.iPlayerId, peer);
                auto& peerInterval = hazardRoom->m_ColosseumMercenaries[peer.iPlayerId];
                peerInterval.AvailableSkills = { awakening->iSkillId };
                decide();
                tests.Require(hazardRoom->m_Players.at(peer.iPlayerId).iCurrentSkillId == awakening->iSkillId &&
                    peerInterval.iLastAltVAdmissionTick == firstTick + 1800u && interval.iLastAltVAdmissionTick == firstTick + 900u,
                    "A different mercenary owns an independent ALT_V interval");
                hazardRoom->m_ColosseumMercenaries.erase(peer.iPlayerId); hazardRoom->m_Players.erase(peer.iPlayerId);
                for (const auto phase : { COLOSSEUM_MATCH_PHASE::RECRUITING, COLOSSEUM_MATCH_PHASE::FINISHED })
                {
                    hazardRoom->m_eColosseumPhase = phase;
                    decide();
                    tests.Require(interval.iLastAltVAdmissionTick == firstTick + 900u,
                        "Recruitment and match completion do not erase a mercenary's accepted ALT_V clock");
                }
                hazardRoom->m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::ACTIVE;

                const auto beforeWrap = (std::numeric_limits<std::uint32_t>::max)() - 400u;
                hazardRoom->m_iServerTick = beforeWrap; actor = freshActor();
                interval = {}; interval.AvailableSkills = { awakening->iSkillId };
                decide();
                tests.Require(interval.iLastAltVAdmissionTick == beforeWrap && actor.iCurrentSkillId == awakening->iSkillId,
                    "The mercenary ALT_V interval records an accepted cast near server tick wrap");
                hazardRoom->m_iServerTick = beforeWrap + 899u; actor = freshActor();
                decide();
                tests.Require(actor.eAction == PLAYER_ACTION_STATE::NONE && interval.iLastAltVAdmissionTick == beforeWrap,
                    "Unsigned elapsed ticks keep ALT_V blocked across wrap before thirty seconds");
                hazardRoom->m_iServerTick = beforeWrap + 900u; actor = freshActor();
                decide();
                tests.Require(actor.iCurrentSkillId == awakening->iSkillId &&
                    interval.iLastAltVAdmissionTick == beforeWrap + 900u,
                    "ALT_V becomes eligible exactly at the thirty-second interval across tick wrap");
            }
            // The normal skill executor owns stance changes and stand-up too.
            // Neither a second stance runtime nor a synthetic direct swap is used.
            {
                auto& controlled = hazardRoom->m_Players.at(merc.iPlayerId);
                auto& selection = hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId);
                controlled = merc; selection = {};
                CPlayerSkillSystem::Reset_Gauges(controlled, hazardRoom->m_GameplayCatalog);
                for (const auto& [id, definition] : catalog->Get_Skills())
                    if (definition.eCharacterClass == CHARACTER_CLASS_ID::LANCE_MASTER)
                        controlled.CooldownEndTickBySkillId[id] = hazardRoom->m_iServerTick + 100000u;
                controlled.CooldownEndTickBySkillId.erase(34000u);
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.iCurrentSkillId == 34000u, "Available-skill selection admits the real published LanceMaster Z stance skill");
                std::vector<SERVER_WORLD_ENTITY> emptyWorld;
                std::vector<DAMAGE_EVENT> damage;
                for (unsigned tick = 0u; tick < 150u && controlled.eAction != PLAYER_ACTION_STATE::NONE; ++tick)
                    hazardRoom->m_PlayerSkillSystem.Update(controlled, emptyWorld, hazardRoom->m_GameplayCatalog,
                        &hazardRoom->m_ServerNavigation, nullptr, 1.f / 30.f, ++hazardRoom->m_iServerTick, damage);
                tests.Require(controlled.eStance == PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR,
                    "The native action commits the stance before the next AI decision");
                controlled.CooldownEndTickBySkillId.erase(34510u);
                selection.iNextAttackTick = 0u; selection.fThinkElapsed = 1.f;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.iCurrentSkillId == 34510u,
                    "The native stance exposes its ready LMB through the refreshed class bindings");
                for (unsigned tick = 0u; tick < 900u && controlled.eAction != PLAYER_ACTION_STATE::NONE; ++tick)
                {
                    hazardRoom->Update_Colosseum(1.f / 30.f);
                    hazardRoom->m_PlayerSkillSystem.Update(controlled, emptyWorld, hazardRoom->m_GameplayCatalog,
                        &hazardRoom->m_ServerNavigation, nullptr, 1.f / 30.f, ++hazardRoom->m_iServerTick, damage);
                }
                controlled.fPositionX = landing.x; controlled.fPositionY = landing.y; controlled.fPositionZ = landing.z;
                controlled.CooldownEndTickBySkillId[34510u] = hazardRoom->m_iServerTick + 100000u;
                controlled.CooldownEndTickBySkillId.erase(34540u);
                selection.iNextAttackTick = 0u; selection.fThinkElapsed = 1.f;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.iCurrentSkillId == 34540u &&
                    std::find(selection.AvailableSkills.begin(), selection.AvailableSkills.end(), 34540u) != selection.AvailableSkills.end() &&
                    std::find(selection.AvailableSkills.begin(), selection.AvailableSkills.end(), 34040u) == selection.AvailableSkills.end(),
                    "Selection refreshes actual active-catalog bindings after a native stance change");
                controlled = merc; controlled.eAction = PLAYER_ACTION_STATE::KNOCKDOWN;
                controlled.iKnockdownEndTick = hazardRoom->m_iServerTick + 300u; selection = {};
                hazardRoom->Update_Colosseum(.25f);
                const auto* standup = catalog->Find_Skill(controlled.iCurrentSkillId);
                tests.Require(standup && standup->eSkillKind == PLAYER_SKILL_KIND::STANDUP &&
                    controlled.eAction == PLAYER_ACTION_STATE::SKILL && controlled.iKnockdownEndTick == 0u,
                    "A non-DimensionMaster mercenary uses the existing stand-up executor only while knocked down");
            }
        }
    }
    // Exercise the observation against state committed by the real skill executor,
    // then drive the room selector with that same lingering ground hazard.
    {
        auto catalog = std::make_shared<CGameplayCatalog>(source->m_GameplayCatalog.Active());
        auto tactical = std::make_unique<CGameRoom>(WORLD_ID::COLOSSEUM, catalog);
        SERVER_NAV_POINT landing;
        const bool ready = tactical->Is_Ready() && tactical->m_ServerNavigation.Sample_Position(-20.4f, -2.6f, landing, 12.22f);
        tests.Require(ready, "Tactical fixtures load the actual Colosseum navigation and native skill catalog");
        if (ready)
        {
            tactical->m_iColosseumMatchId = 901u; tactical->m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::ACTIVE;
            tactical->m_iServerTick = 100u;
            const auto* profile = catalog->Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER);
            SERVER_PLAYER fresh;
            fresh.iPlayerId = 1u; fresh.iNetEntityId = 101u;
            fresh.eControlKind = PLAYER_CONTROL_KIND::COLOSSEUM_MERCENARY_AI;
            fresh.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; fresh.eStance = profile->eDefaultStance;
            fresh.iCurrentHp = fresh.iMaximumHp = expectedMatchHp; fresh.iColosseumDamageReferenceHp = expectedReferenceHp;
            fresh.iCurrentResource = fresh.iMaximumResource = profile->iMaximumResource;
            fresh.fMoveSpeed = profile->fMoveSpeed; fresh.iColosseumMatchId = 901u;
            fresh.iColosseumTeam = 0u; fresh.bColosseumParticipant = true;
            fresh.isCombatReady = fresh.bColosseumCombatActive = true;
            fresh.fPositionX = landing.x; fresh.fPositionY = landing.y; fresh.fPositionZ = landing.z;
            auto enemy = fresh;
            enemy.iPlayerId = 2u; enemy.iNetEntityId = 102u; enemy.iColosseumTeam = 1u;
            enemy.eControlKind = PLAYER_CONTROL_KIND::HUMAN; enemy.fPositionZ += 4.f;
            tactical->m_Players.emplace(1u, fresh); tactical->m_Players.emplace(2u, enemy);
            auto& actor = tactical->m_Players.at(1u);
            auto& caster = tactical->m_Players.at(2u);
            auto& ai = tactical->m_ColosseumMercenaries[1u];
            auto* skill = const_cast<PLAYER_SKILL_DEFINITION*>(catalog->Find_Skill(34040u));
            skill->eSkillKind = PLAYER_SKILL_KIND::ACTIVE; skill->ComboStages.clear(); skill->RootMotion.clear();
            skill->Hits.clear(); skill->Projectiles.clear(); skill->iActionDurationMs = 100u;
            skill->iCooldownMs = 0u; skill->iResourceCost = 0u;
            skill->eTargetIntent = SKILL_TARGET_INTENT_KIND::GROUND_POINT;
            skill->fTargetMaximumRange = skill->fMaximumRange = 12.f; skill->requiresWalkableTarget = true;
            PLAYER_SKILL_HIT hit; hit.iResultKind = 1u; hit.iTimeMs = 80u; hit.iAreaType = 1u; hit.fRange = .8f;
            skill->Hits.push_back(hit);
            C2S_USE_SKILL command; command.iClientSequence = 1u; command.iSkillId = skill->iSkillId;
            command.eTargetIntent = SKILL_TARGET_INTENT_KIND::GROUND_POINT; command.fAimX = landing.x; command.fAimZ = landing.z;
            const bool groundCast = tactical->m_PlayerSkillSystem.Try_Start(caster, command, *catalog, tactical->m_iServerTick,
                &tactical->m_ServerNavigation);
            CColosseumThreatAssessment observed;
            const std::vector<SERVER_COMBAT_OBJECT> noObjects;
            observed.Observe(actor, tactical->m_Players, noObjects, *catalog, tactical->m_iServerTick);
            tests.Require(groundCast && caster.hasSkillTarget && observed.At(landing.x, landing.y, landing.z).risk > 0.f &&
                observed.At(caster.fPositionX, caster.fPositionY, caster.fPositionZ).risk == 0.f &&
                observed.At(landing.x, landing.y + 5.f, landing.z).risk == 0.f,
                "Native ground-target admission places predicted damage at the committed aim point with the actual height guard");
            caster = enemy; skill->Hits.clear();
            PLAYER_SKILL_PROJECTILE area; area.eKind = PLAYER_PROJECTILE_KIND::FIXAREA;
            area.eOrigin = PLAYER_PROJECTILE_ORIGIN::AIM; area.fMaxDistance = 12.f; area.iLifeMs = 1000u;
            PLAYER_PROJECTILE_HIT contact; contact.isContact = true; contact.Hit = hit; contact.Hit.iTimeMs = 0u;
            area.Hits.push_back(contact); skill->Projectiles.push_back(area);
            const bool areaCast = tactical->m_PlayerSkillSystem.Try_Start(caster, command, *catalog, tactical->m_iServerTick,
                &tactical->m_ServerNavigation);
            std::vector<SERVER_WORLD_ENTITY> emptyWorld; std::vector<DAMAGE_EVENT> damage;
            for (unsigned tick = 0u; tick < 5u; ++tick)
                tactical->m_PlayerSkillSystem.Update(caster, emptyWorld, tactical->m_GameplayCatalog,
                    &tactical->m_ServerNavigation, nullptr, 1.f / 30.f, ++tactical->m_iServerTick, damage);
            observed.Observe(actor, tactical->m_Players, noObjects, *catalog, tactical->m_iServerTick);
            tests.Require(areaCast && caster.eAction == PLAYER_ACTION_STATE::NONE && !caster.Projectiles.empty() &&
                observed.At(landing.x, landing.y, landing.z).risk > 0.f,
                "A projectile spawned by the native executor remains a threat after its caster action finishes");
            tests.Require(observed.At(landing.x - 3.f, landing.y, landing.z).risk == 0.f &&
                observed.At(landing.x + 3.f, landing.y, landing.z).risk == 0.f &&
                observed.Along(landing.x - 3.f, landing.y, landing.z, landing.x + 3.f, landing.y, landing.z, .6f) > 0.f &&
                observed.Along(landing.x - 3.f, landing.y, landing.z + 3.f, landing.x + 3.f, landing.y, landing.z + 3.f, .6f) == 0.f,
                "Route assessment detects crossing a lingering hit even when both endpoints are safe and accepts a clear detour");
            actor.CooldownEndTickBySkillId[34020u] = 10000u;
            tactical->Update_Colosseum(.25f);
            tests.Require(actor.hasMoveGoal && actor.eAction == PLAYER_ACTION_STATE::NONE &&
                ai.eTactic == CGameRoom::COLOSSEUM_TACTIC::EVADE &&
                observed.At(actor.fMoveGoalX, actor.fPositionY, actor.fMoveGoalZ, .4f).risk == 0.f,
                "The actual AI walks to a safe navigation goal for an active lingering area when SPACE is cooling down");
            if (!caster.Projectiles.empty())
            {
                caster.Projectiles.front().ContactMarks.push_back({actor.iNetEntityId, 0u, 1u, 0.f});
                observed.Observe(actor, tactical->m_Players, noObjects, *catalog, tactical->m_iServerTick);
                tests.Require(observed.At(landing.x, landing.y, landing.z).risk == 0.f,
                    "Consumed per-target contact hits are not predicted as fresh damage");
                caster.Projectiles.front().ContactMarks.clear();
            }
            caster.iCurrentHp = 0u;
            observed.Observe(actor, tactical->m_Players, noObjects, *catalog, tactical->m_iServerTick);
            tests.Require(observed.ThreatCount() == 0u, "A dead projectile owner is excluded by the same PvP source guard as damage");
            caster = enemy; caster.fPositionZ = landing.z + 1.f; actor = fresh; ai = {};
            auto challenger = caster; challenger.iPlayerId = 3u; challenger.iNetEntityId = 103u; challenger.fPositionZ += .1f;
            tactical->m_Players.emplace(3u, challenger);
            const auto decide = [&]()
            {
                ai.fThinkElapsed = 1.f; ai.iNextAttackTick = 0u;
                for (const auto& [id, definition] : catalog->Get_Skills())
                    if (definition.eCharacterClass == actor.eCharacterClass)
                        actor.CooldownEndTickBySkillId[id] = tactical->m_iServerTick + 10000u;
                tactical->Update_Colosseum(.25f);
            };
            decide();
            tests.Require(ai.iTargetEntityId == caster.iNetEntityId, "Target scoring selects the closest comparable living opponent");
            tactical->m_Players.at(3u).fPositionZ = landing.z + .9f; tactical->m_iServerTick += 6u;
            decide();
            tests.Require(ai.iTargetEntityId == caster.iNetEntityId, "Target hysteresis retains an opponent through a small short-lived score change");
            caster.iCurrentHp = 0u; tactical->m_iServerTick += 6u; decide();
            tests.Require(ai.iTargetEntityId == challenger.iNetEntityId, "A dead target is replaced immediately by another eligible opponent");
            tactical->m_Players.at(3u).iColosseumMatchId = 999u; decide();
            tests.Require(ai.iTargetEntityId == INVALID_NET_ENTITY_ID && ai.eTactic == CGameRoom::COLOSSEUM_TACTIC::WAIT,
                "Other matches are excluded from target selection even when their players are nearby");
            tactical->m_Players.erase(3u); caster = enemy; caster.fPositionZ = landing.z + 2.f;
            actor = fresh; actor.iCurrentHp = actor.iMaximumHp / 5u; ai = {}; ai.iObservedHp = fresh.iMaximumHp;
            tactical->m_iServerTick = 1000u; decide();
            const auto retreatUntil = ai.iTacticUntilTick;
            tests.Require(ai.eTactic == CGameRoom::COLOSSEUM_TACTIC::RETREAT && actor.hasMoveGoal && retreatUntil > 1000u,
                "Low health under nearby pressure enters a bounded retreat through the real movement executor");
            caster.fPositionZ = landing.z + 10.f; tactical->m_iServerTick = 1006u; decide();
            tests.Require(ai.eTactic == CGameRoom::COLOSSEUM_TACTIC::RETREAT && ai.iTacticUntilTick == retreatUntil,
                "Retreat hysteresis holds its existing deadline when immediate pressure disappears");
            tactical->m_iServerTick = retreatUntil + 1u; decide();
            tests.Require(ai.eTactic != CGameRoom::COLOSSEUM_TACTIC::RETREAT && actor.iCurrentHp == actor.iMaximumHp / 5u &&
                ai.iRetreatAllowedTick > tactical->m_iServerTick,
                "Retreat re-engages after its deadline without waiting for unavailable passive healing and cannot immediately retrigger");
            // Full room ticks exercise simultaneous decisions, movement, native
            // skill damage, hit reactions and score/respawn guards together.
            auto battle = std::make_unique<CGameRoom>(WORLD_ID::COLOSSEUM,
                std::make_shared<CGameplayCatalog>(source->m_GameplayCatalog.Active()));
            battle->m_iColosseumMatchId = 902u; battle->m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::ACTIVE;
            battle->m_iServerTick = 100u; battle->m_iColosseumPhaseEnd = 10000u;
            const std::array<CHARACTER_CLASS_ID, 6> classes{ CHARACTER_CLASS_ID::DIMENSIONMASTER,
                CHARACTER_CLASS_ID::LANCE_MASTER, CHARACTER_CLASS_ID::WARLORD,
                CHARACTER_CLASS_ID::GUARDIANKNIGHT, CHARACTER_CLASS_ID::ARTIST, CHARACTER_CLASS_ID::LANCE_MASTER };
            std::map<PLAYER_ID, std::array<float, 2>> initialPositions;
            std::map<PLAYER_ID, std::set<SKILL_ID>> casts;
            for (std::size_t i = 0u; i < classes.size(); ++i)
            {
                auto player = fresh;
                player.iPlayerId = static_cast<PLAYER_ID>(i + 1u); player.iNetEntityId = static_cast<NET_ENTITY_ID>(201u + i);
                player.iColosseumMatchId = 902u; player.iColosseumTeam = static_cast<std::uint8_t>(i % 2u);
                const auto* memberProfile = battle->m_GameplayCatalog.Find_Player(classes[i]);
                player.eCharacterClass = classes[i]; player.eStance = memberProfile->eDefaultStance;
                player.iCurrentResource = player.iMaximumResource = memberProfile->iMaximumResource;
                player.iMaximumIdentity = memberProfile->iMaximumIdentity; player.fMoveSpeed = memberProfile->fMoveSpeed;
                CPlayerSkillSystem::Reset_Gauges(player, battle->m_GameplayCatalog);
                SERVER_NAV_POINT position;
                const float angle = static_cast<float>(i) * 1.04719755f;
                if (battle->m_ServerNavigation.Sample_Position(landing.x + std::cos(angle) * 1.5f,
                    landing.z + std::sin(angle) * 1.5f, position, landing.y))
                { player.fPositionX = position.x; player.fPositionY = position.y; player.fPositionZ = position.z; }
                player.fColosseumSpawnX = player.fPositionX; player.fColosseumSpawnY = player.fPositionY;
                player.fColosseumSpawnZ = player.fPositionZ;
                initialPositions[player.iPlayerId] = { player.fPositionX, player.fPositionZ };
                battle->m_PlayerIdByEntityId[player.iNetEntityId] = player.iPlayerId;
                battle->m_Players.emplace(player.iPlayerId, player);
                battle->m_ColosseumMercenaries[player.iPlayerId].iRandomState = 0x123456789abcdefu * (i + 1u);
            }
            std::size_t actualDamageEvents = 0u;
            bool moved = false, lostHp = false;
            for (unsigned tick = 0u; tick < 360u; ++tick)
            {
                battle->Tick(1.f / 30.f);
                actualDamageEvents += battle->m_TickDamageEvents.size();
                for (const auto& [id, player] : battle->m_Players)
                {
                    const auto& initial = initialPositions.at(id);
                    moved = moved || std::hypot(player.fPositionX - initial[0], player.fPositionZ - initial[1]) > .5f;
                    lostHp = lostHp || player.iCurrentHp < player.iMaximumHp;
                    if (player.eAction == PLAYER_ACTION_STATE::SKILL) casts[id].insert(player.iCurrentSkillId);
                }
            }
            const auto variedActors = std::count_if(casts.begin(), casts.end(), [](const auto& entry) { return entry.second.size() >= 2u; });
            tests.Require(battle->m_iServerTick == 460u && actualDamageEvents > 0u && moved && lostHp && variedActors >= 2,
                "Twelve seconds of full room ticks produce real PvP damage, HP loss, movement and varied skill use by multiple mercenaries");
            std::cout << "Mercenary battle ticks=" << (battle->m_iServerTick - 100u) << " damage=" << actualDamageEvents
                << " moved=" << moved << " lostHp=" << lostHp << " variedActors=" << variedActors << '\n';
        }
    }
    auto preview = std::make_unique<CGameRoom>(WORLD_ID::COLOSSEUM);
    preview->Update_Colosseum(.5f);
    tests.Require(preview->m_iColosseumMatchId == 0u && preview->m_ColosseumMercenaries.empty(),
        "Direct Colosseum preview gains no PvP authority or candidates");
    // Numeric balance uses its real parser and room stage/commit. A running
    // match pins health at admission; ordinary raid players still migrate by ratio.
    std::map<PLAYER_ID, std::array<std::uint32_t, 3>> pinnedHealth;
    for (auto& [id, player] : room->m_Players)
    {
        if (player.iCurrentHp) player.iCurrentHp = player.iMaximumHp / 2u;
        pinnedHealth.emplace(id, std::array<std::uint32_t, 3>{ player.iCurrentHp, player.iMaximumHp, player.iColosseumDamageReferenceHp });
    }
    std::uint32_t numericSequence = 1800u;
    for (const auto phase : { COLOSSEUM_MATCH_PHASE::RECRUITING, COLOSSEUM_MATCH_PHASE::ACTIVE, COLOSSEUM_MATCH_PHASE::FINISHED })
    {
        room->m_eColosseumPhase = phase;
        const auto base = room->Get_ActiveGameplayGeneration();
        const auto* classProfile = base->Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER);
        const auto* bossProfile = base->Find_Boss("BOSS_VALTAN");
        const std::vector<BALANCE_NUMERIC_CHANGE> changes{
            { BALANCE_DOMAIN::PLAYER, "LANCE_MASTER", "maximumHp", double(classProfile->iMaximumHp), double(classProfile->iMaximumHp + 37u) },
            { BALANCE_DOMAIN::BOSS, "BOSS_VALTAN", "maximumHp", double(bossProfile->iMaximumHp), double(bossProfile->iMaximumHp + 1000u) }
        };
        auto candidate = std::make_shared<CGameplayCatalog>();
        std::string bytes, status;
        const bool parsed = candidate->Load_NumericBalancePatch(*base, changes, bytes);
        const bool committed = parsed && room->Stage_NumericBalance(++numericSequence, candidate, status) &&
            room->Commit_NumericBalance(numericSequence);
        tests.Require(committed && std::all_of(room->m_Players.begin(), room->m_Players.end(), [&](const auto& value) {
            const auto health = pinnedHealth.at(value.first);
            return value.second.iCurrentHp == health[0] && value.second.iMaximumHp == health[1] &&
                value.second.iColosseumDamageReferenceHp == health[2];
        }), "Numeric reload pins arena HP and full damage reference for humans, selected mercenaries and idle candidates in every phase");
        if (phase == COLOSSEUM_MATCH_PHASE::RECRUITING && parsed)
        {
            auto raid = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA, base);
            SERVER_PLAYER player;
            player.iPlayerId = 9001u; player.iNetEntityId = 9002u;
            player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
            player.iMaximumHp = classProfile->iMaximumHp; player.iCurrentHp = classProfile->iMaximumHp / 2u;
            const auto oldHp = player.iCurrentHp;
            raid->m_Players.emplace(player.iPlayerId, player);
            const auto newMaximum = candidate->Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER)->iMaximumHp;
            const auto newCurrent = static_cast<std::uint32_t>((static_cast<std::uint64_t>(oldHp) * newMaximum + classProfile->iMaximumHp / 2u) / classProfile->iMaximumHp);
            const bool raidCommitted = raid->Is_Ready() && raid->Stage_NumericBalance(1900u, candidate, status) && raid->Commit_NumericBalance(1900u);
            tests.Require(raidCommitted && raid->m_Players.at(player.iPlayerId).iMaximumHp == newMaximum &&
                raid->m_Players.at(player.iPlayerId).iCurrentHp == newCurrent,
                "The real Valtan room keeps normal class-HP ratio migration after the same numeric reload");
        }
    }
    for (const auto& session : sessions) session->m_isSendRunning.store(false);
    std::cout << "Colosseum match contract failures=" << tests.failures << '\n';
    return tests.failures ? 1 : 0;
}

int CServerGameplayContractRunner::Run_ColosseumMatch()
{
    int result = 1;
    const auto execute = [](void* output)
    {
        std::cout << std::unitbuf;
        *static_cast<int*>(output) = Run_ColosseumMatchContracts();
    };
    return ServerGameplayContractDetail::Run_WithContractWorkerStack(execute, &result) ? result : 1;
}
