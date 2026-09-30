#include "ServerGameplayContractTests_Runner.h"
#include "ServerApp.h"
#include "ClientSession.h"
#include "ColosseumCombatPolicy.h"
#include "Network/PacketWriter.h"
#include "Network/PacketReader.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
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
    for (unsigned count = 1u; count <= 3u; ++count)
    {
        auto smallSource = std::make_unique<CGameRoom>(WORLD_ID::BERN, app->m_pActiveGameplayGeneration);
        auto smallTarget = std::make_unique<CGameRoom>(WORLD_ID::COLOSSEUM, app->m_pActiveGameplayGeneration);
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
        tests.Require(committed, "One, two and three humans each commit through the real atomic Colosseum admission");
        if (!committed) std::cout << "Flexible admission count=" << count << " status=" << status << '\n';
        if (committed)
        {
            tests.Require(smallTarget->Count_HumanPlayers() == count && smallTarget->m_Players.size() == count + 10u,
                "Flexible admission keeps exactly the admitted humans and ten sessionless candidates");
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
                "Only the team without a human receives exactly four automatically selected mercenaries");
            smallTarget->Update_ColosseumMatch(1u);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::RECRUITING,
                "Small matches reach recruitment without waiting for absent humans");
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
                state.iExpectedPlayers == 8u && Write_Message(writer, state),
                "Small matches fill four members per team and publish an encodable eight-person entry countdown");
            smallTarget->Update_ColosseumMatch(smallTarget->m_iColosseumPhaseEnd);
            smallTarget->Update_ColosseumMatch(smallTarget->m_iColosseumPhaseEnd);
            smallTarget->Update_ColosseumMatch(smallTarget->m_iColosseumPhaseEnd);
            tests.Require(smallTarget->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ACTIVE &&
                std::count_if(smallTarget->m_Players.begin(), smallTarget->m_Players.end(), [](const auto& p) { return p.second.bColosseumCombatActive; }) == 8,
                "A solo, duo or trio can finish all entry phases and activate eight combatants");
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
    const auto expectedReferenceHp = source->m_GameplayCatalog.Find_Boss("BOSS_VALTAN")->iMaximumHp;
    const auto expectedMatchHp = expectedReferenceHp / 4u + (expectedReferenceHp % 4u ? 1u : 0u);
    tests.Require(std::all_of(room->m_Players.begin(), room->m_Players.end(), [expectedMatchHp, expectedReferenceHp](const auto& value) {
        return value.second.iMaximumHp == expectedMatchHp && value.second.iColosseumDamageReferenceHp == expectedReferenceHp && !value.second.isCombatReady &&
            (!value.second.Is_Human() || (value.second.Inventory.empty() && value.second.Purse.iSilver >= 77u));
    }), "Match has 40 Valtan HP bars with the full immutable damage reference and preserves empty inventory/purse");
    for (const auto& [id, ai] : room->m_ColosseumMercenaries)
    {
        const auto& merc = room->m_Players.at(id);
        std::vector<std::string> slots;
        bool valid = !ai.ComboSkills.empty();
        for (const auto skillId : ai.ComboSkills)
        {
            const auto* skill = room->m_GameplayCatalog.Find_Skill(skillId);
            valid = valid && skill &&
                (merc.eCharacterClass != CHARACTER_CLASS_ID::DIMENSIONMASTER || (skill->strInputSlot != "LMB" && skill->strInputSlot != "SPACE")) &&
                skill->eCharacterClass == merc.eCharacterClass &&
                (skill->eRequiredStance == PLAYER_STANCE_ID::NONE || skill->eRequiredStance == merc.eStance);
            if (skill) slots.push_back(skill->strInputSlot);
        }
        tests.Require(valid, "Staged skills resolve actual class/stance bindings; only DimensionMaster excludes basic attacks");
        if (merc.eCharacterClass == CHARACTER_CLASS_ID::DIMENSIONMASTER)
            tests.Require(!room->m_GuideCatalog.Combos.empty() && slots == room->m_GuideCatalog.Combos.front().Slots,
                "DimensionMaster rotation reuses the first published Guide combo in authored order");
        if (merc.eCharacterClass != CHARACTER_CLASS_ID::DIMENSIONMASTER)
        {
            std::vector<SKILL_ID> allAvailable;
            for (const auto& [skillId, skill] : room->m_GameplayCatalog.Active().Get_Skills())
                if (skill.eCharacterClass == merc.eCharacterClass &&
                    (skill.eRequiredStance == PLAYER_STANCE_ID::NONE || skill.eRequiredStance == merc.eStance)) allAvailable.push_back(skillId);
            std::sort(allAvailable.begin(), allAvailable.end());
            auto admittedSkills = ai.ComboSkills;
            std::sort(admittedSkills.begin(), admittedSkills.end());
            tests.Require(admittedSkills == allAvailable && !slots.empty() && slots.front() == "LMB",
                "Other mercenary classes include every current published skill and LMB");
        }
        std::cout << "Mercenary combo class=" << static_cast<unsigned>(merc.eCharacterClass) << " slots=";
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
        std::count_if(room->m_Players.begin(), room->m_Players.end(), [](const auto& p) { return p.second.bColosseumParticipant; }) == 8,
        "Two selected mercenaries per team prepare eight participants and a three-second entry countdown");
    const auto entryDeadline = room->m_iColosseumPhaseEnd;
    room->Update_ColosseumMatch(entryDeadline - 1u);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::ENTRY_COUNTDOWN &&
        std::none_of(room->m_Players.begin(), room->m_Players.end(), [](const auto& p) { return p.second.bColosseumCombatActive; }),
        "Entry countdown blocks all damage before the deadline");
    room->Update_ColosseumMatch(entryDeadline);
    tests.Require(room->m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::INTRO && room->m_iColosseumPhaseEnd == entryDeadline + 258u,
        "Three-second entry transitions to one shared 8.6-second lineup clock");
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
            std::hypot(evading.fMoveGoalX - landing.x, evading.fMoveGoalZ - landing.z) > 2.f,
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
            auto& ai = hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId);
            const auto admitted = std::find_if(room->m_ColosseumMercenaries.begin(), room->m_ColosseumMercenaries.end(),
                [&](const auto& value) { return room->m_Players.at(value.first).eCharacterClass == CHARACTER_CLASS_ID::DIMENSIONMASTER; });
            tests.Require(admitted != room->m_ColosseumMercenaries.end(), "Fixed rotation fixture finds the admitted DimensionMaster combo");
            if (admitted != room->m_ColosseumMercenaries.end() && admitted->second.ComboSkills.size() >= 2u)
            {
                ai = admitted->second; ai.iSkillCursor = 0u; ai.iSequence = 0u;
                ai.fComboElapsed = ai.fStepWaitElapsed = ai.fThinkElapsed = 0.f;
                auto& controlled = hazardRoom->m_Players.at(merc.iPlayerId);
                auto& target = hazardRoom->m_Players.at(caster.iPlayerId);
                controlled = merc;
                const auto* fixedProfile = catalog->Find_Player(CHARACTER_CLASS_ID::DIMENSIONMASTER);
                controlled.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER; controlled.eStance = fixedProfile->eDefaultStance;
                controlled.iCurrentResource = controlled.iMaximumResource = fixedProfile->iMaximumResource;
                controlled.iMaximumIdentity = fixedProfile->iMaximumIdentity;
                CPlayerSkillSystem::Reset_Gauges(controlled, hazardRoom->m_GameplayCatalog);
                target.eAction = PLAYER_ACTION_STATE::NONE; target.iCurrentSkillId = INVALID_SKILL_ID;
                target.fPositionZ = landing.z + 1.f;
                const auto first = ai.ComboSkills[0], second = ai.ComboSkills[1];
                controlled.CooldownEndTickBySkillId[first] = 1000u;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.eAction == PLAYER_ACTION_STATE::NONE && ai.iSkillCursor == 0u &&
                    std::string(ai.pReason) == "Waiting for the ordered skill cooldown",
                    "A blocked first skill waits without selecting another ready skill or LMB");
                controlled.CooldownEndTickBySkillId.clear();
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.eAction == PLAYER_ACTION_STATE::SKILL && controlled.iCurrentSkillId == first && ai.iSkillCursor == 1u,
                    "The real AI admits the first ordered skill once its cooldown clears");
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.iCurrentSkillId == first && ai.iSkillCursor == 1u && controlled.PendingCommand.eKind == PLAYER_PENDING_COMMAND_KIND::NONE,
                    "The second ordered skill is not queued before the first action finishes");
                const auto finishAction = [&]()
                {
                    std::vector<SERVER_WORLD_ENTITY> world;
                    std::vector<DAMAGE_EVENT> damage;
                    for (unsigned tick = 0u; tick < 900u && controlled.eAction != PLAYER_ACTION_STATE::NONE; ++tick)
                        hazardRoom->m_PlayerSkillSystem.Update(controlled, world, hazardRoom->m_GameplayCatalog,
                            &hazardRoom->m_ServerNavigation, nullptr, 1.f / 30.f, ++hazardRoom->m_iServerTick, damage);
                };
                finishAction();
                tests.Require(controlled.eAction == PLAYER_ACTION_STATE::NONE, "The admitted action completes through the actual shared skill runtime");
                controlled.fPositionX = landing.x; controlled.fPositionY = landing.y; controlled.fPositionZ = landing.z;
                controlled.bPatternBound = true;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(ai.iSkillCursor == 1u && controlled.eAction == PLAYER_ACTION_STATE::NONE,
                    "Crowd control pauses the fixed combo without losing its next step");
                controlled.bPatternBound = false;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.iCurrentSkillId == second && controlled.eAction == PLAYER_ACTION_STATE::SKILL && ai.iSkillCursor == 2u,
                    "The ordered second skill resumes after crowd control ends");
                finishAction();
                controlled.fPositionX = landing.x; controlled.fPositionY = landing.y; controlled.fPositionZ = landing.z;
                ai.iSkillCursor = 1u; ai.fStepWaitElapsed = 0.f; ai.fStepWaitTimeout = .4f;
                controlled.CooldownEndTickBySkillId[second] = hazardRoom->m_iServerTick + 1000u;
                hazardRoom->Update_Colosseum(.25f); hazardRoom->Update_Colosseum(.25f);
                tests.Require(ai.iSkillCursor == 0u && controlled.eAction == PLAYER_ACTION_STATE::NONE &&
                    std::string(ai.pReason) == "Unavailable skill exceeded its wait deadline",
                    "An unavailable ordered step aborts at its wait deadline without fallback attacks");
                ai.iSkillCursor = 1u; ai.fComboElapsed = ai.fComboTimeout - .1f;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(ai.iSkillCursor == 0u && controlled.eAction == PLAYER_ACTION_STATE::NONE &&
                    std::string(ai.pReason) == "Combo total deadline reached",
                    "The full combo deadline resets its order without injecting a skill");
            }
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
                tests.Require(admittedChain != room->m_ColosseumMercenaries.end() && !admittedChain->second.ComboSkills.empty(),
                    "Native continuation class has an admitted next-slot rotation");
                if (admittedChain == room->m_ColosseumMercenaries.end() || admittedChain->second.ComboSkills.empty()) continue;
                SKILL_ID nextSkill = INVALID_SKILL_ID;
                for (const auto& [id, definition] : catalog->Get_Skills())
                    if (definition.eCharacterClass == actor.eCharacterClass)
                    {
                        controlled.CooldownEndTickBySkillId[id] = hazardRoom->m_iServerTick + 100000u;
                        if (definition.strInputSlot == "Q" &&
                            (definition.eRequiredStance == PLAYER_STANCE_ID::NONE || definition.eRequiredStance == actor.eStance)) nextSkill = id;
                    }
                controlled.CooldownEndTickBySkillId.erase(chain->iSkillId);
                chainAi.ComboSkills = admittedChain->second.ComboSkills;
                hazardRoom->Update_Colosseum(.25f);
                const auto cursorAfterStart = chainAi.iSkillCursor;
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
                    ordered = ordered && chainAi.iSkillCursor == cursorAfterStart && controlled.iCurrentSkillId == chain->iSkillId &&
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
                controlled.CooldownEndTickBySkillId.erase(nextSkill);
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.eAction == PLAYER_ACTION_STATE::SKILL && controlled.iCurrentSkillId == nextSkill,
                    "Only the completed native COMBO opens another available skill on the following decision");
                chain->ComboStages.front().iInputOpenMs = originalOpen;
                chain->ComboStages.front().iInputCloseMs = originalClose;
            }
            // With every skill initially ready, the actual AI must advance the
            // explicit rotation after one native LMB cycle, not restart LMB or sort IDs.
            for (const auto classId : { CHARACTER_CLASS_ID::LANCE_MASTER, CHARACTER_CLASS_ID::WARLORD,
                CHARACTER_CLASS_ID::GUARDIANKNIGHT, CHARACTER_CLASS_ID::ARTIST })
            {
                auto& actor = hazardRoom->m_Players.at(merc.iPlayerId);
                actor = merc;
                const auto* actorProfile = catalog->Find_Player(classId);
                actor.eCharacterClass = classId; actor.eStance = actorProfile->eDefaultStance;
                actor.iCurrentResource = actor.iMaximumResource = actorProfile->iMaximumResource;
                actor.iMaximumIdentity = actorProfile->iMaximumIdentity;
                actor.fMoveSpeed = actorProfile->fMoveSpeed;
                actor.CooldownEndTickBySkillId.clear();
                CPlayerSkillSystem::Reset_Gauges(actor, hazardRoom->m_GameplayCatalog);
                auto& target = hazardRoom->m_Players.at(caster.iPlayerId);
                target.eAction = PLAYER_ACTION_STATE::NONE; target.iCurrentSkillId = INVALID_SKILL_ID;
                target.fPositionX = landing.x; target.fPositionY = landing.y; target.fPositionZ = landing.z + 1.f;
                auto& rotation = hazardRoom->m_ColosseumMercenaries.at(merc.iPlayerId);
                rotation = {};
                bool ordered = true;
                std::vector<SERVER_WORLD_ENTITY> emptyWorld;
                std::vector<DAMAGE_EVENT> damage;
                for (const auto* slot : { "LMB", "Q", "W" })
                {
                    actor.fPositionX = landing.x; actor.fPositionY = landing.y; actor.fPositionZ = landing.z;
                    hazardRoom->Update_Colosseum(.25f);
                    const auto* running = catalog->Find_Skill(actor.iCurrentSkillId);
                    ordered = ordered && actor.eAction == PLAYER_ACTION_STATE::SKILL && running && running->strInputSlot == slot;
                    for (unsigned tick = 0u; tick < 900u && actor.eAction != PLAYER_ACTION_STATE::NONE; ++tick)
                    {
                        hazardRoom->Update_Colosseum(1.f / 30.f);
                        hazardRoom->m_PlayerSkillSystem.Update(actor, emptyWorld, hazardRoom->m_GameplayCatalog,
                            &hazardRoom->m_ServerNavigation, nullptr, 1.f / 30.f, ++hazardRoom->m_iServerTick, damage);
                    }
                    ordered = ordered && actor.eAction == PLAYER_ACTION_STATE::NONE;
                }
                tests.Require(ordered, "Every other class completes native LMB then Q then W in the explicit rotation with every skill ready");
                std::cout << "Mercenary ready rotation class=" << static_cast<unsigned>(classId) << " LMB,Q,W=" << ordered << '\n';
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
                controlled.CooldownEndTickBySkillId.erase(34540u);
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.iCurrentSkillId == 34510u,
                    "A shorter stance skill list preserves the next input slot and wraps from Z to the new LMB");
                for (unsigned tick = 0u; tick < 900u && controlled.eAction != PLAYER_ACTION_STATE::NONE; ++tick)
                {
                    hazardRoom->Update_Colosseum(1.f / 30.f);
                    hazardRoom->m_PlayerSkillSystem.Update(controlled, emptyWorld, hazardRoom->m_GameplayCatalog,
                        &hazardRoom->m_ServerNavigation, nullptr, 1.f / 30.f, ++hazardRoom->m_iServerTick, damage);
                }
                controlled.fPositionX = landing.x; controlled.fPositionY = landing.y; controlled.fPositionZ = landing.z;
                hazardRoom->Update_Colosseum(.25f);
                tests.Require(controlled.iCurrentSkillId == 34540u &&
                    std::find(selection.ComboSkills.begin(), selection.ComboSkills.end(), 34540u) != selection.ComboSkills.end() &&
                    std::find(selection.ComboSkills.begin(), selection.ComboSkills.end(), 34040u) == selection.ComboSkills.end(),
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
