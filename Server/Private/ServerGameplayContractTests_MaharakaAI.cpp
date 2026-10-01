#include "ServerGameplayContractTests_Runner.h"
#include "GameRoom.h"
#include "ServerApp.h"
#include "ClientSession.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <set>

using namespace LostArk::Server;
using namespace LostArk::Shared;

int CServerGameplayContractRunner::Run_MaharakaAI()
{
    TESTS tests;

    // Exercise the actual G/vote/admission transaction without a listening socket.
    for (const unsigned partySize : {1u, 2u, 4u})
    {
        auto source = std::make_shared<CGameRoom>(WORLD_ID::BERN);
        auto island = std::make_shared<CGameRoom>(WORLD_ID::MAHARAKA);
        auto app = std::make_unique<CServerApp>();
        app->m_SharedGameRooms.emplace(WORLD_ID::BERN, source);
        app->m_SharedGameRooms.emplace(WORLD_ID::MAHARAKA, island);
        std::vector<std::shared_ptr<CClientSession>> peers;
        const auto clear = [&]() {
            for (auto& peer : peers) {
                peer->m_OutboundFrames.clear(); peer->m_iQueuedOutboundBytes = 0;
                peer->m_OutboundMetrics.iCurrentQueuedByteCount = 0;
                peer->m_OutboundMetrics.iCurrentQueuedFrameCount = 0;
            }
        };
        bool ready = source->Is_Ready() && island->Is_Ready();
        for (unsigned i = 0; i < partySize && ready; ++i)
        {
            auto peer = std::make_shared<CClientSession>(99701u + i, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            peer->m_isSendRunning.store(true); source->Handle_Register(peer);
            C2S_ENTER_WORLD entry; entry.eWorldId = WORLD_ID::BERN;
            entry.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
            entry.strNickName = "IslandParty" + std::to_string(i);
            ready = source->Join(peer->Get_SessionId(), entry);
            peers.push_back(peer);
            app->m_Sessions.emplace(peer->Get_SessionId(), peer);
            CServerApp::SESSION_GAMEPLAY_BINDING binding;
            binding.eWorldId = WORLD_ID::BERN; binding.pSimulation = source;
            app->m_GameplayBindingBySessionId.emplace(peer->Get_SessionId(), binding);
            clear();
        }
        tests.Require(ready, "Island party fixtures admit the complete human roster");
        if (!ready) continue;
        auto& leader = source->m_Players.at(peers.front()->Get_PlayerId());
        for (unsigned i = 1; i < partySize; ++i)
        {
            C2S_PARTY_INVITE invite;
            invite.iTargetNetEntityId = source->m_Players.at(peers[i]->Get_PlayerId()).iNetEntityId;
            source->Handle_PartyInvite(leader.iSessionId, invite);
            C2S_PARTY_INVITE_RESPOND response;
            response.iFromNetEntityId = leader.iNetEntityId; response.bAccepted = true;
            source->Handle_PartyInviteRespond(peers[i]->Get_SessionId(), response);
        }
        clear();
        const auto* dock = source->Find_Placement("island.dock.to.maharaka");
        tests.Require(dock != nullptr, "Island entry uses the published Bern G dock");
        if (!dock) continue;
        for (unsigned i = 0; i < partySize; ++i)
        {
            auto& player = source->m_Players.at(peers[i]->Get_PlayerId());
            player.fPositionX = dock->fPositionX; player.fPositionY = dock->fPositionY; player.fPositionZ = dock->fPositionZ;
            player.iVehicleId = 8200u; player.bShipDockValid = true;
            player.fShipDockX = 10.f + i; player.fShipDockY = 1.f;
            player.fShipDockZ = 20.f + i; player.fShipDockYawDegrees = 30.f;
            player.Inventory.clear(); player.Purse.iSilver = 0u; player.Purse.iGold = 0u;
        }
        C2S_INTERACT_TRIGGER interact; interact.iRequestSequence = 501u;
        interact.strTriggerPlacementId = dock->strPlacementId;
        source->Handle_InteractTrigger(leader.iSessionId, interact);
        tests.Require(source->m_RaidEntryProposals.size() == 1u && source->m_PendingWorldTransfers.empty(),
            "G opens one island confirmation for all members before any departure");
        if (source->m_RaidEntryProposals.empty()) continue;
        C2S_RAID_ENTRY_RESPOND answer;
        answer.iProposalId = source->m_RaidEntryProposals.front().iProposalId; answer.bAccepted = false;
        source->Handle_RaidEntryRespond(peers.back()->Get_SessionId(), answer);
        tests.Require(source->m_RaidEntryProposals.empty() && source->m_PendingWorldTransfers.empty() &&
            source->Count_HumanPlayers() == partySize && island->Count_HumanPlayers() == 0u,
            "An island vote decline preserves every member in Bern");
        C2S_RAID_ENTRY_PROPOSE propose; propose.iRequestSequence = 502u;
        propose.eTarget = RAID_ENTRY_TARGET::MAHARAKA; propose.strNpcPlacementId = dock->strPlacementId;
        source->Handle_RaidEntryPropose(leader.iSessionId, propose);
        if (source->m_RaidEntryProposals.empty()) { tests.Require(false, "Island confirmation can be proposed again"); continue; }
        answer.iProposalId = source->m_RaidEntryProposals.front().iProposalId; answer.bAccepted = true;
        for (const auto& peer : peers) source->Handle_RaidEntryRespond(peer->Get_SessionId(), answer);
        SERVER_WORLD_TRANSFER_REQUEST transfer;
        const bool staged = source->Try_DequeueWorldTransfer(transfer);
        tests.Require(staged && transfer.PartyBatchSessionIds.size() == partySize &&
            source->m_MaharakaShipReturnBySession.size() == partySize,
            "Every island traveler, including solo, uses the bounded batch and retains its own ship");
        if (!staged) continue;
        clear();
        const std::array<std::uint8_t, 1> payload{1u};
        for (unsigned i = 0; i < CClientSession::MAX_OUTBOUND_FRAME_COUNT; ++i)
            (void)peers.back()->Send_Frame(PACKET_TYPE::S2C_CHAT, payload);
        CServerApp::SESSION_WORLD_TRANSFER_FAILURE failure;
        tests.Require(!app->Transfer_SessionWorld(source, transfer, failure) &&
            failure.ePartyResult == PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY &&
            source->Count_HumanPlayers() == partySize && island->Count_HumanPlayers() == 0u,
            "One full outbound queue rejects island admission without partial room or party mutation");
        clear();
        const bool entered = app->Transfer_SessionWorld(source, transfer, failure);
        tests.Require(entered && source->Count_HumanPlayers() == 0u && island->Count_HumanPlayers() == partySize,
            "All island travelers commit together after reliable preparation");
        if (!entered) continue;
        tests.Require(partySize == 1u ? island->m_PartyMembersByPartyId.empty() :
            island->m_PartyMembersByPartyId.size() == 1u && island->m_PartyMembersByPartyId.begin()->second.size() == partySize,
            "Island entry preserves the complete party roster");
        for (unsigned i = 0; i < partySize; ++i)
        {
            const auto& player = island->m_Players.at(peers[i]->Get_PlayerId());
            tests.Require(player.strNickName == "IslandParty" + std::to_string(i) && player.Inventory.empty() &&
                player.Purse.iSilver == 0u && player.Purse.iGold == 0u &&
                app->m_GameplayBindingBySessionId.at(peers[i]->Get_SessionId()).pSimulation == island,
                "Nickname, intentionally empty inventory/purse and authoritative session binding survive island entry");
        }
        clear();
        auto& islandLeader = island->m_Players.at(peers.front()->Get_PlayerId());
        const auto* exit = island->Find_Placement("island.exit.to.bern");
        tests.Require(exit != nullptr, "Island return uses the published G exit");
        if (!exit) continue;
        islandLeader.fPositionX = exit->fPositionX; islandLeader.fPositionY = exit->fPositionY; islandLeader.fPositionZ = exit->fPositionZ;
        interact.iRequestSequence = 503u; interact.strTriggerPlacementId = exit->strPlacementId;
        island->Handle_InteractTrigger(islandLeader.iSessionId, interact);
        tests.Require(island->m_RaidEntryProposals.size() == 1u, "Island G return also confirms with the party");
        if (island->m_RaidEntryProposals.empty()) continue;
        answer.iProposalId = island->m_RaidEntryProposals.front().iProposalId;
        for (const auto& peer : peers) island->Handle_RaidEntryRespond(peer->Get_SessionId(), answer);
        const bool returnStaged = island->Try_DequeueWorldTransfer(transfer);
        tests.Require(returnStaged && transfer.strSpawnPlacementOverrideId == "island.return.sea.landing",
            "Party return keeps the authored sea landing");
        clear();
        const bool returned = returnStaged && app->Transfer_SessionWorld(island, transfer, failure);
        tests.Require(returned && source->Count_HumanPlayers() == partySize && island->Count_HumanPlayers() == 0u,
            "Party return commits every member back into Bern");
        if (returned) for (unsigned i = 0; i < partySize; ++i)
        {
            const auto& player = source->m_Players.at(peers[i]->Get_PlayerId());
            tests.Require(player.iVehicleId == 8200u && player.bShipDockValid &&
                player.fShipDockX == 10.f + i && player.Inventory.empty() && player.Purse.iSilver == 0u,
                "Each returning member restores its own ship and empty inventory without fresh grants");
        }
    }
    for (const auto world : {WORLD_ID::MAHARAKA, WORLD_ID::VALTAN_ARENA, WORLD_ID::KAKULSAYDON_ARENA})
    {
        auto fallRoom = std::make_unique<CGameRoom>(world);
        SERVER_PLAYER falling; falling.iCurrentHp = falling.iMaximumHp = 100u;
        falling.fPositionX = MAHARAKA_WATERPANG_CANNON_X;
        falling.fPositionZ = MAHARAKA_WATERPANG_CANNON_Z; falling.fPositionY = 22.4f;
        if (world == WORLD_ID::KAKULSAYDON_ARENA) falling.iMarioStage = 1u;
        fallRoom->Begin_PlayerFall(falling, 0.f, 100u);
        const bool water = world == WORLD_ID::MAHARAKA;
        falling.bWaterpangFall = water;
        tests.Require(std::abs(falling.fFallDeathPlaneY - (water ? 20.4f : 17.4f)) < .001f,
            "Only Maharaka changes the fall depth to two meters; both raid fall planes retain five");
        falling.fPositionY = falling.fFallDeathPlaneY + .01f;
        (void)fallRoom->Update_PlayerFall(falling, 0.f, 101u);
        tests.Require(falling.eAction == PLAYER_ACTION_STATE::FALLING && falling.iCurrentHp == 100u,
            "Above the fall plane no world resolves the fall early");
        falling.fPositionY = falling.fFallDeathPlaneY - .01f;
        (void)fallRoom->Update_PlayerFall(falling, 0.f, 102u);
        tests.Require(water ? falling.eAction == PLAYER_ACTION_STATE::NONE && falling.iCurrentHp == 100u :
            world == WORLD_ID::VALTAN_ARENA ? falling.eAction == PLAYER_ACTION_STATE::FALLING && falling.iCurrentHp == 100u :
            falling.eAction == PLAYER_ACTION_STATE::DEAD && falling.iCurrentHp == 0u,
            "Waterpang returns alive at two meters, Valtan keeps its deadline and Kouku keeps its original height death");
        if (world == WORLD_ID::VALTAN_ARENA)
        {
            (void)fallRoom->Update_PlayerFall(falling, 0.f, 145u);
            tests.Require(falling.eAction == PLAYER_ACTION_STATE::DEAD && falling.iCurrentHp == 0u,
                "Valtan still resolves falling only at its original 45-tick deadline");
        }
    }

    Run_WorldPlayback(tests);
    auto room = std::make_unique<CGameRoom>(WORLD_ID::MAHARAKA);
    tests.Require(room->Is_Ready(), "Waterpang AI fixture loads the published Maharaka room");
    if (!room->Is_Ready()) return 1;
    std::vector<std::shared_ptr<CClientSession>> sessions;
    const auto drain = [&]() { for (auto& session : sessions) { session->m_OutboundFrames.clear(); session->m_iQueuedOutboundBytes = 0; session->m_OutboundMetrics.iCurrentQueuedByteCount = 0; session->m_OutboundMetrics.iCurrentQueuedFrameCount = 0; } };
    for (unsigned i = 0; i < 4; ++i)
    {
        auto session = std::make_shared<CClientSession>(99501u + i, INVALID_SOCKET, CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
        session->m_isSendRunning.store(true); room->Handle_Register(session);
        C2S_ENTER_WORLD enter; enter.eWorldId = WORLD_ID::MAHARAKA; enter.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; enter.strNickName = "Waterpang contract";
        tests.Require(room->Join(session->Get_SessionId(), enter), "Human admission succeeds beside optional Waterpang contestants");
        sessions.push_back(session); drain();
    }
    if (room->Count_HumanPlayers() != 4) return 1;
    C2S_MAHARAKA_AI_TUNING request; request.iRequestSequence = 1;
    CPacketWriter requestWriter; C2S_MAHARAKA_AI_TUNING decoded;
    const bool written = Write_Message(requestWriter, request); CPacketReader requestReader(requestWriter.Get_Buffer());
    tests.Require(written && Read_Message(requestReader, decoded) && !requestReader.Get_RemainingSize(), "AI typed GET request round trips");
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    const auto revision = room->m_MaharakaAITuning.iRevision;
    request.eOperation = MAHARAKA_AI_OPERATION::APPLY; request.iExpectedRevision = revision;
    request.Tuning = room->m_MaharakaAITuning; request.Tuning.fMoveProbability = 1.f; request.Tuning.fAggression = 1.f;
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iRevision == revision + 1 && room->m_MaharakaAITuning.fAggression == 1.f, "AI APPLY commits the new revision and live decision values");
    request.Tuning.iBotCount = 0;
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iBotCount == 20 && room->m_MaharakaAITuning.iRevision == revision + 1, "Stale tuning CAS preserves the live roster and revision");
    request.iExpectedRevision = revision + 1; request.Tuning.fAggression = std::numeric_limits<float>::quiet_NaN();
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iRevision == revision + 1, "Nonfinite AI tuning is rejected before mutation");
    // Exercise the real SAVE path in an isolated authoring root, without touching team data.
    const auto originalCwd = std::filesystem::current_path();
    const auto isolated = originalCwd / "out/WaterpangEffects20260930/ai-save-fixture";
    std::filesystem::create_directories(isolated / "Data/AI");
    { std::ofstream marker(isolated / "AGENTS.md"); marker << "isolated test root\n"; }
    const auto source = isolated / "Data/AI/MaharakaWaterpangAI.json";
    { std::ofstream stream(source, std::ios::binary); stream << room->m_strMaharakaAISourceBytes; }
    std::filesystem::current_path(isolated);
    request.eOperation = MAHARAKA_AI_OPERATION::SAVE; request.Tuning = room->m_MaharakaAITuning;
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iRevision == revision + 2 && std::filesystem::is_regular_file(source.string() + ".backup"), "AI SAVE validates and atomically replaces source with a recoverable backup");
    { std::ofstream stream(source, std::ios::app); stream << " \n"; }
    request.iExpectedRevision = revision + 2; request.Tuning.iBotCount = 0;
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iRevision == revision + 2 && room->m_MaharakaAITuning.iBotCount == 20, "External source edit rejects SAVE and preserves active tuning");
    std::filesystem::current_path(originalCwd);

    S2C_WORLD_SEQUENCE_PLAY intro; intro.eOperation = WORLD_SEQUENCE_OPERATION::PLAY;
    intro.strSequenceInstanceId = MAHARAKA_WATERPANG_INTRO_INSTANCE; intro.iStartTick = 100;
    room->m_MaharakaWaterpangIntro = intro; room->m_iServerTick = 100;
    room->Update_MaharakaWaterpangMatch(100);
    std::set<std::string> expectedNames;
    for (unsigned slot = 1u; slot <= 20u; ++slot) expectedNames.insert("Waterpang AI " + std::to_string(slot));
    const auto numberedRoster = [&]()
    {
        std::set<std::string> names;
        for (const auto& [id, state] : room->m_MaharakaWaterpangAI)
        {
            const auto& player = room->m_Players.at(id);
            if (player.strNickName != "Waterpang AI " + std::to_string(state.iSlot + 1u) ||
                !names.insert(player.strNickName).second) return false;
        }
        return names == expectedNames;
    };
    tests.Require(numberedRoster(), "Twenty Waterpang AI have unique stable slot names 1 through 20 across avatar and NPC appearances");
    bool spawnNames = true;
    for (const auto& session : sessions)
    {
        std::set<std::string> observedNames;
        for (const auto& frame : session->m_OutboundFrames)
        {
            if (frame.ePacketType != PACKET_TYPE::S2C_PLAYER_SPAWNED) continue;
            CPacketReader reader(std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES));
            S2C_PLAYER_SPAWNED spawned;
            if (!Read_Message(reader, spawned) || reader.Get_RemainingSize()) { spawnNames = false; continue; }
            if (spawned.eControlKind == PLAYER_CONTROL_KIND::WATERPANG_AI) observedNames.insert(spawned.strNickName);
        }
        spawnNames = spawnNames && observedNames == expectedNames;
    }
    tests.Require(spawnNames, "Actual reliable spawn frames deliver all twenty AI nicknames unchanged to every human session");
    drain();
    tests.Require(room->m_MaharakaWaterpangAI.size() == 20 && room->m_Players.size() == 24 && room->Count_HumanPlayers() == 4, "Twenty AI fit the real navigation and retain all four human slots");
    if (room->m_MaharakaWaterpangAI.size() != 20) return 1;
    unsigned avatars = 0, npcs = 0;
    for (const auto& [id, state] : room->m_MaharakaWaterpangAI)
    {
        const auto& player = room->m_Players.at(id);
        tests.Require(player.Is_WaterpangAI() && !player.Is_Human() && player.iSessionId == INVALID_SESSION_ID && player.fPositionY >= MAHARAKA_WATERPANG_DECK_MIN_Y_M, "AI uses native deck support without a fake session");
        if (player.strWaterpangNpcArchetypeId.empty()) avatars += player.Inventory.size() == 2;
        else ++npcs;
    }
    tests.Require(avatars == 12 && npcs == 8, "Roster carries twelve real avatar loadouts and eight immutable NPC identities");
    room->m_iServerTick = 100 + 20 * 30;
    room->Update_MaharakaWaterpangMatch(room->m_iServerTick); drain();
    bool moved = false, usedSkill = false;
    for (const auto& [id, state] : room->m_MaharakaWaterpangAI)
    {
        const auto& player = room->m_Players.at(id);
        moved = moved || player.iLastMoveSequence != 0;
        usedSkill = usedSkill || player.iWaterGunSkillId != 0;
    }
    tests.Require(moved && usedSkill, "Actual AI decision loop submits admitted movement and Waterpang skills");
    for (unsigned i = 0; i < 15; ++i) { room->Tick(1.f / 30.f); drain(); }
    tests.Require(room->m_MaharakaWaterpangAI.size() == 20 && room->Count_HumanPlayers() == 4, "Actual room ticks preserve AI ownership and human roster");
    tests.Require(numberedRoster(), "Movement and skill updates preserve every AI slot nickname");
    std::array<SERVER_NAV_POINT, 4> admitted;
    for (unsigned i = 0; i < sessions.size(); ++i)
    {
        const auto& player = room->m_Players.at(sessions[i]->Get_PlayerId());
        admitted[i] = {player.fPositionX, player.fPositionY, player.fPositionZ};
    }
    auto& human = room->m_Players.at(sessions[0]->Get_PlayerId());
    // Isolate collapse probes from the main branch\'s independent return/snapshot regression.
    {
        const auto beforeCollapseHuman=human;
        const auto beforeCollapseTick=room->m_iServerTick;
        tests.Require(MAHARAKA_WATERPANG_MATCH_END_TICKS - MAHARAKA_WATERPANG_COLLAPSE_START_TICKS == 60u * 30u &&
            MAHARAKA_WATERPANG_COLLAPSE_START_TICKS == (20u + 120u) * 30u,
            "Original ring collapse starts with exactly one minute remaining, excluding countdown and intro");
        const auto missingTick = intro.iStartTick + MAHARAKA_WATERPANG_COLLAPSE_SUPPORT_TICKS;
        human.fPositionX = MAHARAKA_WATERPANG_CANNON_X + 6.f; human.fPositionY = 22.4f; human.fPositionZ = MAHARAKA_WATERPANG_CANNON_Z;
        human.eAction = PLAYER_ACTION_STATE::NONE; human.TriggerMove = {};
        human.bKnockbackBallistic = human.bArenaEjectionActive = false;
        tests.Require(!room->Update_PlayerFall(human, 1.f / 30.f, missingTick - 1u),
            "Ring keeps support before the collapse gameplay transition");
        tests.Require(room->Update_PlayerFall(human, 1.f / 30.f, missingTick) &&
            human.bWaterpangFall && human.eAction == PLAYER_ACTION_STATE::FALLING && human.fPositionY < 22.4f,
            "A stationary contestant on the collapsed ring begins the existing Waterpang fall");
        for (unsigned step=1;step<=60u && human.bWaterpangFall;++step)
            room->Update_PlayerFall(human, 1.f / 30.f, missingTick+step);
        tests.Require(human.iCurrentHp && !human.bWaterpangFall && human.eAction == PLAYER_ACTION_STATE::NONE,
            "Collapsed ring fall returns the contestant alive to a jump pier");
        tests.Require(!room->Update_PlayerFall(human, 1.f / 30.f, missingTick + 46u),
            "Side jump piers do not collapse with the outer ring");
        human.fPositionX = MAHARAKA_WATERPANG_CANNON_X + 4.f; human.fPositionY = 22.4f; human.fPositionZ = MAHARAKA_WATERPANG_CANNON_Z;
        tests.Require(!room->Update_PlayerFall(human, 1.f / 30.f, missingTick),
            "The yellow centre remains supported after ring collapse");
        human.fPositionX = MAHARAKA_WATERPANG_CANNON_X + 6.f; human.fPositionY = 20.48f;
        tests.Require(!room->Update_PlayerFall(human, 1.f / 30.f, missingTick),
            "Ordinary exploration below the arena is not a collapsed-deck fall");
        human.TriggerMove.isActive = true; human.TriggerMove.strSourcePlacementId = "jump1";
        human.TriggerMove.fStartX = human.fPositionX; human.TriggerMove.fStartY = human.fPositionY; human.TriggerMove.fStartZ = human.fPositionZ;
        human.TriggerMove.fTargetX = human.fPositionX; human.TriggerMove.fTargetY = 22.4f; human.TriggerMove.fTargetZ = human.fPositionZ;
        human.TriggerMove.fDurationSeconds = 1.f; human.TriggerMove.fArcHeight = 2.f;
        human.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
        room->m_iServerTick = missingTick;
        room->Update_Players(1.f / 30.f); drain();
        tests.Require(human.TriggerMove.isActive && std::hypot(
            human.TriggerMove.fTargetX - MAHARAKA_WATERPANG_CANNON_X,
            human.TriggerMove.fTargetZ - MAHARAKA_WATERPANG_CANNON_Z) < MAHARAKA_WATERPANG_WATERFALL_HIT_RADIUS_M,
            "G jump targets the remaining centre without modifying authored boxes");
        for (const auto name : MAHARAKA_WATERPANG_JUMP_TRIGGER_IDS)
        {
            const auto* box=room->Find_Placement(std::string(name));
            tests.Require(box && box->TriggerActions.size()==1u,"Collapsed match keeps each published jump action");
            if (!box || box->TriggerActions.size()!=1u) continue;
            human.TriggerMove={}; human.eAction=PLAYER_ACTION_STATE::NONE;
            human.bWaterpangFall=human.bKnockbackBallistic=human.bArenaEjectionActive=false;
            human.fKnockbackRemainingSeconds=0.f;
            human.fPositionX=box->fPositionX; human.fPositionY=box->fPositionY; human.fPositionZ=box->fPositionZ;
            CServerTriggerSystem jump; jump.Set_WorldId(WORLD_ID::MAHARAKA);
            std::string status;
            tests.Require(jump.Initialize({*box},status),"Post-collapse G trigger initializes from the real published box");
            std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
            const auto activate=[](WORLD_TRIGGER_ACTION_KIND,const std::string&){return true;};
            tests.Require(jump.Activate_Here(human.iPlayerId,room->m_Players,missingTick,transfers,activate)==1u,
                "Post-collapse G input activates the saved crossing");
            const auto& authored=box->TriggerActions.front();
            room->m_iServerTick=missingTick;
            room->Update_Players(1.f/30.f); drain();
            const auto landing=human.TriggerMove;
            const bool onMissingRing=Is_MaharakaWaterpangMissingRing(MAHARAKA_WATERPANG_COLLAPSE_SUPPORT_TICKS,
                authored.fTargetX,authored.fTargetZ);
            tests.Require(landing.isActive && (onMissingRing ? std::abs(std::hypot(
                landing.fTargetX-MAHARAKA_WATERPANG_CANNON_X,landing.fTargetZ-MAHARAKA_WATERPANG_CANNON_Z)-
                MAHARAKA_WATERPANG_COLLAPSED_LANDING_RADIUS_M)<.001f :
                landing.fTargetX==authored.fTargetX && landing.fTargetY==authored.fTargetY && landing.fTargetZ==authored.fTargetZ),
                "Collapse preserves safe edited landings and adapts only missing outer-ring destinations");
            for (unsigned tick=1;tick<45;++tick)
            {
                room->m_iServerTick=missingTick+tick;
                room->Update_Players(1.f/30.f); drain();
            }
            tests.Require(!human.TriggerMove.isActive && human.iCurrentHp && !human.bWaterpangFall &&
                human.eAction!=PLAYER_ACTION_STATE::FALLING && human.fPositionY>=MAHARAKA_WATERPANG_DECK_MIN_Y_M &&
                std::hypot(human.fPositionX-landing.fTargetX,human.fPositionZ-landing.fTargetZ)<.01f,
                "Every real G jump lands alive and stays supported after collapse");
        }
        human.TriggerMove = {}; human.eAction = PLAYER_ACTION_STATE::NONE;
        human.fPositionX = MAHARAKA_WATERPANG_CANNON_X + 6.f; human.fPositionY = 22.5f;
        human.bKnockbackBallistic = true; human.bKnockbackCanLeaveArena = true;
        human.fKnockbackRemainingSeconds = 1.f / 30.f; human.fKnockbackVelocityY = -6.f;
        human.fKnockbackLaunchY = human.fKnockbackSupportY = 22.4f; human.fKnockbackSpeed = 0.f;
        room->Advance_PlayerKnockback(human, 1.f / 30.f);
        tests.Require(human.bWaterpangFall && human.eAction == PLAYER_ACTION_STATE::FALLING,
            "Airborne knockback cannot land on the old baked ring height");
        human=beforeCollapseHuman;
        room->m_iServerTick=beforeCollapseTick;
    }
    auto& fallen = room->m_Players.at(sessions[1]->Get_PlayerId());
    auto& launched = room->m_Players.at(sessions[2]->Get_PlayerId());
    auto& visitor = room->m_Players.at(sessions[3]->Get_PlayerId());
    for (unsigned i = 0; i < 3u; ++i)
    {
        auto& player = room->m_Players.at(sessions[i]->Get_PlayerId());
        player.fPositionX = MAHARAKA_WATERPANG_CANNON_X + (i == 0u ? 4.f : i == 1u ? -4.f : 0.f);
        player.fPositionY = 22.4f; player.fPositionZ = MAHARAKA_WATERPANG_CANNON_Z + (i == 2u ? 4.f : 0.f);
    }
    C2S_USE_SKILL shot; shot.iClientSequence = human.iLastSkillSequence + 1u;
    shot.iSkillId = MAHARAKA_WATERGUN_SKILLS.front().iSkillId;
    shot.fAimX = fallen.fPositionX; shot.fAimZ = fallen.fPositionZ;
    tests.Require(room->Try_StartMaharakaWaterGunSkill(human, shot) && human.iWaterGunCastTick != 0u,
        "A real human Waterpang cast precedes the match-end snapshot regression");
    room->Update_MaharakaWaterpangMatch(room->m_iServerTick);
    tests.Require(human.bWaterpangParticipant && fallen.bWaterpangParticipant && launched.bWaterpangParticipant && !visitor.bWaterpangParticipant,
        "Match membership retains arena humans without capturing island visitors");
    // One contestant has already returned to a jump box; another is still in flight at expiry.
    fallen.fPositionX = 66.f; fallen.fPositionY = 20.48f; fallen.fPositionZ = -998.f;
    fallen.iCurrentHp = 0u; fallen.eAction = PLAYER_ACTION_STATE::DEAD;
    launched.fPositionX = 90.f; launched.fPositionY = 25.f; launched.fPositionZ = -984.f;
    launched.bWaterpangLaunch = launched.bWaterpangFall = launched.bKnockbackBallistic = launched.bKnockbackCanLeaveArena = true;
    launched.fKnockbackRemainingSeconds = 1.f; launched.fKnockbackVelocityY = 5.f;
    launched.iFallDeathTick = room->m_iServerTick + 10u; launched.eAction = PLAYER_ACTION_STATE::FALLING;
    human.fWaterGunSpeedScale = 1.8f; human.iWaterGunSpeedEndTick = room->m_iServerTick + 100u;
    human.iWaterpangCannonHitTick = room->m_iServerTick;
    for (const auto& skill : MAHARAKA_WATERGUN_SKILLS) human.CooldownEndTickBySkillId[skill.iSkillId] = room->m_iServerTick + 100u;
    const auto spawnId = fallen.strSpawnPlacementId;
    fallen.strSpawnPlacementId = "missing.waterpang.return.spawn";
    const auto shotCount = room->m_MaharakaWaterGunShots.size();
    drain();
    tests.Require(!room->Finish_MaharakaWaterpangMatch() && room->m_MaharakaWaterpangIntro &&
        room->m_MaharakaWaterpangAI.size() == 20u && room->m_MaharakaWaterGunShots.size() == shotCount &&
        human.iWaterGunCastTick != 0u && human.bWaterpangParticipant && fallen.iCurrentHp == 0u &&
        std::abs(human.fPositionX - (MAHARAKA_WATERPANG_CANNON_X + 4.f)) < .001f &&
        sessions[0]->m_OutboundFrames.empty(),
        "One invalid return destination preserves all positions, AI, casts and the sequence transaction");
    fallen.strSpawnPlacementId = spawnId;
    room->m_iServerTick = intro.iStartTick + MAHARAKA_WATERPANG_MATCH_END_TICKS - 1u;
    const auto encodeFailures = room->m_PerformanceMetrics.iSnapshotEncodeFailureCount;
    room->Tick(1.f / 30.f);
    tests.Require(!room->m_MaharakaWaterpangIntro && !room->m_MaharakaWaterpangDebugEvent &&
        room->m_MaharakaWaterpangAI.empty() && room->m_MaharakaWaterGunShots.empty() && room->Count_HumanPlayers() == 4u,
        "Actual expiry tick clears the match and all AI while retaining the connected humans");
    {
        const auto returnedHuman=human;
        human.fPositionX=MAHARAKA_WATERPANG_CANNON_X+6.f;
        human.fPositionY=22.4f; human.fPositionZ=MAHARAKA_WATERPANG_CANNON_Z;
        tests.Require(!room->Update_PlayerFall(human,1.f/30.f,room->m_iServerTick),
            "Match reset restores the ring's normal support without stale collapse state");
        human=returnedHuman;
    }
    room->Refresh_PlayerBlockingBodies();
    for (unsigned i = 0; i < 3u; ++i)
    {
        const auto& player = room->m_Players.at(sessions[i]->Get_PlayerId());
        const auto* spawn = room->Find_Placement(player.strSpawnPlacementId);
        SERVER_NAV_POINT returnGround{}, markerGround{};
        const bool returnNavigable = room->m_ServerNavigation.Sample_Position(player.fPositionX, player.fPositionZ, returnGround, player.fPositionY) &&
            std::abs(returnGround.y - player.fPositionY) < .05f;
        const bool returnClear = room->m_ServerCollisionSystem.Is_PlayerPositionClear(player.fPositionX, player.fPositionY, player.fPositionZ, player.iNetEntityId);
        const bool markerNavigable = spawn && room->m_ServerNavigation.Sample_Position(spawn->fPositionX, spawn->fPositionZ, markerGround, spawn->fPositionY) &&
            std::abs(markerGround.y - spawn->fPositionY) <= 2.f;
        const bool markerClear = spawn && room->m_ServerCollisionSystem.Is_PlayerPositionClear(spawn->fPositionX, spawn->fPositionY, spawn->fPositionZ, player.iNetEntityId);
        const float markerDistance = spawn ? std::hypot(player.fPositionX - spawn->fPositionX, player.fPositionZ - spawn->fPositionZ) : 1000.f;
        std::cout << "[WaterpangLanding] participant=" << i << " markerDistance=" << markerDistance
            << " markerNav/clear=" << markerNavigable << '/' << markerClear
            << " returnNav/clear=" << returnNavigable << '/' << returnClear << '\n';
        tests.Require(spawn && markerDistance <= 3.001f && returnNavigable && returnClear &&
            !Is_MaharakaWaterpangArenaFootprint(player.fPositionX, player.fPositionZ),
            "Each admission marker returns onto nearby authoritative navigation outside every blocking body");
        tests.Require(markerDistance < .4f || !markerNavigable || !markerClear,
            "A displaced landing is justified by the authored marker failing navigation or actual collision admission");
        std::cout << "[WaterpangReturn] participant=" << i << " spawn=" << player.strSpawnPlacementId
            << " admitted=" << admitted[i].x << ',' << admitted[i].y << ',' << admitted[i].z
            << " returned=" << player.fPositionX << ',' << player.fPositionY << ',' << player.fPositionZ
            << " hp=" << player.iCurrentHp << '/' << player.iMaximumHp << " action=" << static_cast<unsigned>(player.eAction)
            << " participant/fall/launch/ballistic/leave/trigger=" << player.bWaterpangParticipant << '/' << player.bWaterpangFall
            << '/' << player.bWaterpangLaunch << '/' << player.bKnockbackBallistic << '/' << player.bKnockbackCanLeaveArena
            << '/' << player.TriggerMove.isActive << " fallTick=" << player.iFallDeathTick
            << " cast=" << player.iWaterGunSkillId << '/' << player.iWaterGunCastTick << '/' << player.iWaterGunCastEndTick
            << " speed=" << player.iWaterGunSpeedEndTick << '/' << player.fWaterGunSpeedScale << '\n';
        tests.Require(player.iCurrentHp == player.iMaximumHp &&
            player.eAction == PLAYER_ACTION_STATE::NONE && !player.bWaterpangParticipant &&
            !player.bWaterpangFall && !player.bWaterpangLaunch && !player.bKnockbackBallistic &&
            !player.bKnockbackCanLeaveArena && !player.TriggerMove.isActive && !player.iFallDeathTick &&
            !player.iWaterGunSkillId && !player.iWaterGunCastTick && !player.iWaterGunCastEndTick &&
            !player.iWaterGunSpeedEndTick && player.fWaterGunSpeedScale == 1.f,
            "Every participant returns alive to its own admission spawn with all Waterpang motion and cast state cleared");
    }
    tests.Require(visitor.fPositionX == admitted[3].x && visitor.fPositionY == admitted[3].y && visitor.fPositionZ == admitted[3].z,
        "An island visitor remains at the existing location");
    for (const auto& skill : MAHARAKA_WATERGUN_SKILLS)
        tests.Require(!human.CooldownEndTickBySkillId.contains(skill.iSkillId), "Match return clears only the Waterpang skill cooldowns");
    bool stopped = false;
    unsigned snapshotRecipients = 0u;
    for (const auto& session : sessions)
    {
        bool snapshotReceived = false;
        for (const auto& frame : session->m_OutboundFrames)
        {
            CPacketReader reader{std::span<const std::uint8_t>{frame.Bytes}.subspan(PACKET_HEADER_BYTES)};
            if (frame.ePacketType == PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY)
            {
                S2C_WORLD_SEQUENCE_PLAY play;
                stopped = stopped || (Read_Message(reader, play) && play.eOperation == WORLD_SEQUENCE_OPERATION::STOP);
            }
            if (frame.ePacketType == PACKET_TYPE::S2C_WORLD_SNAPSHOT)
            {
                S2C_WORLD_SNAPSHOT snapshot;
                snapshotReceived = Read_Message(reader, snapshot) && !reader.Get_RemainingSize() && snapshot.Players.size() == 4u &&
                    std::all_of(snapshot.Players.begin(), snapshot.Players.end(), [](const auto& player)
                    { return player.iWaterGunSkillId == 0u && player.iWaterGunCastTick == 0u && !player.isWaterpangArmed; });
            }
        }
        snapshotRecipients += snapshotReceived ? 1u : 0u;
    }
    tests.Require(stopped && snapshotRecipients == 4u && room->m_PerformanceMetrics.iSnapshotEncodeFailureCount == encodeFailures,
        "STOP and a valid post-return world snapshot reach every client without the stale cast-tick failure");
    drain();
    std::array<SERVER_NAV_POINT, 3> beforeMove;
    for (unsigned i = 0; i < beforeMove.size(); ++i)
    {
        auto& player = room->m_Players.at(sessions[i]->Get_PlayerId());
        beforeMove[i] = {player.fPositionX, player.fPositionY, player.fPositionZ};
        bool submitted = false;
        for (unsigned direction = 0; direction < 8u && !submitted; ++direction)
        {
            const float angle = direction * 3.14159265359f / 4.f;
            SERVER_NAV_POINT goal{};
            const float x = player.fPositionX + .75f * std::cos(angle), z = player.fPositionZ + .75f * std::sin(angle);
            if (!room->m_ServerNavigation.Sample_Position(x, z, goal, player.fPositionY) ||
                !room->m_ServerNavigation.Has_LineOfSight(player.fPositionX, player.fPositionZ, goal.x, goal.z, player.fPositionY) ||
                !room->m_ServerCollisionSystem.Is_PlayerPositionClear(goal.x, goal.y, goal.z, player.iNetEntityId)) continue;
            float sweepX, sweepY, sweepZ; bool blocked = false;
            if (!room->m_ServerCollisionSystem.Resolve_PlayerMove(player, goal.x, goal.y, goal.z, sweepX, sweepY, sweepZ, blocked) || blocked) continue;
            C2S_MOVE move; move.iClientSequence = player.iLastMoveSequence + 1u;
            move.fGoalX = goal.x; move.fGoalZ = goal.z;
            room->Execute_PlayerMove(player, move);
            submitted = player.hasMoveGoal;
        }
        tests.Require(submitted, "Every returned participant admits a movement command along an actual free navigation route");
    }
    room->Tick(1.f / 30.f); drain();
    for (unsigned i = 0; i < beforeMove.size(); ++i)
    {
        const auto& player = room->m_Players.at(sessions[i]->Get_PlayerId());
        tests.Require(std::hypot(player.fPositionX - beforeMove[i].x, player.fPositionZ - beforeMove[i].z) > .005f,
            "Every returned participant advances on the next authoritative room tick");
    }
    const auto* jump = room->Find_Placement("jump3");
    tests.Require(jump != nullptr, "Published Waterpang re-entry jump exists");
    if (jump)
    {
        human.hasMoveGoal = false; human.MovePath.clear();
        human.fPositionX = jump->fPositionX; human.fPositionY = jump->fPositionY; human.fPositionZ = jump->fPositionZ;
        std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
        const auto activate = [&](WORLD_TRIGGER_ACTION_KIND kind, const std::string& target)
        { return room->Activate_TriggerTarget(kind, target); };
        const bool jumped = room->m_ServerTriggerSystem.Activate_Interact(human.iPlayerId, jump->strPlacementId,
            room->m_Players, room->m_iServerTick + 1u, transfers, activate);
        tests.Require(jumped && human.TriggerMove.isActive, "The same player can activate the real G jump after match return");
        for (unsigned i = 0; i < 45u; ++i) { room->Tick(1.f / 30.f); drain(); }
        tests.Require(room->m_MaharakaWaterpangIntro && room->m_MaharakaWaterpangIntro->iStartTick >
            intro.iStartTick + MAHARAKA_WATERPANG_MATCH_END_TICKS && room->m_MaharakaWaterpangAI.size() == 20u,
            "Re-entry landing reserves a new countdown and admits the next AI roster without leaving the room");
        tests.Require(numberedRoster(), "The next match respawns unique AI names from 1 through 20 without stale or duplicate suffixes");
    }
    for (const auto& session : sessions) room->Leave(session->Get_SessionId(), PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
    return tests.failures ? 1 : 0;
}
