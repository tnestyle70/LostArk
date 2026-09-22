#include "ServerGameplayContractTests_Runner.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include "KoukuSaydonBrain.h"
#include "Gameplay/KoukuArenaReadyAreas.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include <algorithm>
#include <cmath>
#include <iostream>
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
    Run_KoukuGate3Entry(tests);
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
            "Duplicate G3 death cannot create a new epoch or restart the eight-second clock");
        CPacketWriter preparationWire;
        tests.Require(Write_Message(preparationWire, run.State), "Legacy clear PREPARING uses the existing Shared packet schema");
        room->m_WorldEntities.erase(boss);
        C2S_GATE_PROGRESS_PROPOSE bypass; bypass.iRequestSequence = 90u;
        bypass.eWorldId = WORLD_ID::KAKULSAYDON_ARENA; bypass.eKind = GATE_PROGRESS_KIND::ADVANCE;
        room->Handle_GateProgressPropose(501u, bypass);
        tests.Require(!room->m_GateProgress.iProposalId && !room->Enter_KoukuRaidCombat(4u),
            "Manual advance and direct Bingo entry cannot bypass a standalone clear's resource preparation");
        // Release preparation expires after ten seconds; exercise timer-first before that deadline.
        const auto readyTick = count % 2u ? 120u : 390u;
        room->m_iServerTick = readyTick; room->Update_KoukuRaid(readyTick);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING,
            "Eight seconds alone never starts unprepared Encore resources");
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
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && run.State.iEndTick == 340u,
            "Prepared legacy clear preserves the original death plus eight-second deadline");
        if (readyTick < 340u) { room->Update_KoukuRaid(339u); tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
            "Fast resource preparation still waits the complete eight-second clear beat"); }
        room->m_iServerTick = (std::max)(340u, readyTick + 1u); room->Update_KoukuRaid(room->m_iServerTick);
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
#ifdef _DEBUG
        constexpr unsigned preparationTicks = 20u * 60u * 30u;
#else
        constexpr unsigned preparationTicks = 300u;
#endif
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.State.iStartTick == 0u &&
            run.State.iEndTick == 10u + preparationTicks && run.State.ParticipantPlayerIds.size() == count,
            "Real START command pins the roster and enters bounded Debug resource preparation without starting the cinematic");
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
                    run.State.iEndTick == end + 242u && room->Is_KoukuRaidInputBlocked(),
                    "Gate 3 false clear reserves exactly eight Server seconds and blocks gameplay");
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
            tests.Require(bingo->strIntroPatternId == "KAKULSAYDON_G1_PATTERN_10" && bingo->iIntroDurationMs >= 23333u && bingo->Entries.size() == 1u,
                "Published Bingo pins the complete original encore before its repeating parent");
            if (bingo->Entries.size() != 1u) continue;
            const auto encoreStartTick = run.State.iEndTick;
            room->m_iServerTick = encoreStartTick - 1u; room->Update_KoukuRaid(room->m_iServerTick);
            tests.Require(run.State.strGateId == "GATE3" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
                "One tick before eight seconds the false-clear scene is retained");
            room->m_iServerTick = encoreStartTick; room->Update_KoukuRaid(room->m_iServerTick);
            tests.Require(run.State.strGateId == "BINGO" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC &&
                !run.bClearCinematic && run.State.strSequencePatternId == bingo->strIntroPatternId && room->Is_KoukuRaidInputBlocked(),
                "The eight-second deadline starts one server-clock encore without a vote");
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
            std::string patternStatus;
            const auto* parent = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*run.pCatalog, bingo->Entries.front().strTargetId, patternStatus);
            tests.Require(parent && parent->ParentChildren.size() == 38u &&
                parent->strParentLoopStartOccurrenceId == "KAKULSAYDON_G1_PATTERN_96.pattern.30",
                "Bingo retains one Parent with 24 introduction children and a 14-child loop");
            const auto previousRequest = run.iAuditionRequestSequence;
            std::vector<std::string> played;
            unsigned previousSequence = 0u;
            bool boardRetained = true;
            const auto firstTick = room->m_iServerTick + 1u;
            // Exercise two tail cycles using the production actor/Logic scheduler; fixture HP isolates progression.
            for (unsigned tick = firstTick; tick < firstTick + 24000u && played.size() < 52u; ++tick)
            {
                room->m_iServerTick = tick;
                for (auto& [id, player] : room->m_Players) player.iCurrentHp = player.iMaximumHp = 100000000u;
                room->Prepare_KoukuAuditionTick(tick);
                room->Update_KoukuPatternTails(tick);
                auto* boss = room->Find_KoukuSaydonArenaBoss("boss.kakulsaydon.bingo.saydon", "BOSS_KAKULSAYDON_BINGO_SAYDON");
                if (!boss || room->m_KoukuSaydonPatternAudition.Members.empty()) break;
                if (!boss->strPatternId.empty() && boss->strPatternId != parent->strPatternId && boss->iPatternSequence != previousSequence)
                { played.push_back(boss->strPatternId); previousSequence = boss->iPatternSequence; }
                room->Update_KoukuSaydonBoss(*boss, tick);
                room->Commit_KoukuMechanicTriggers(tick);
                room->Update_KoukuBingo(tick);
                if (!played.empty()) boardRetained = boardRetained && room->m_KoukuBingoDuration.iOwnerId != 0u && room->m_KoukuBingoDuration.iEndTick == 0u;
            }
            std::vector<std::string> expected;
            if (parent)
            {
                for (const auto& child : parent->ParentChildren) expected.push_back(child.strPatternId);
                for (std::size_t i = 24u; i < parent->ParentChildren.size(); ++i) expected.push_back(parent->ParentChildren[i].strPatternId);
            }
            tests.Require(played == expected && boardRetained && run.iAuditionRequestSequence == previousRequest,
                "Actual Bingo scheduler plays its prefix once and repeats the full tail without restarting the board or outer Flow");
            if (played != expected) std::cout << "[RAID PARENT] children=" << played.size() << " status=" << room->m_strStatus << '\n';
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
