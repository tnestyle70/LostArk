#include "GameRoom.h"
#include "ClientSession.h"
#include "Network/PacketWriter.h"
#include "GameRoom_Internal.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

using namespace LostArk::Shared;
using namespace GameRoomDetail;

namespace
{
    constexpr std::uint32_t KOUKU_GATE3_CLEAR_HOLD_MS = 5000u;
}

bool LostArk::Server::CGameRoom::Is_KoukuRaidInputBlocked() const
{
    return Is_KoukuRaidRunning() && ((m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING &&
        m_KoukuRaid.bClearedGate3Preparation) || m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC ||
        m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY ||
        (m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE &&
            m_KoukuRaid.State.strGateId == "GATE3" && m_KoukuRaid.State.iEndTick != 0u));
}

void LostArk::Server::CGameRoom::Handle_KoukuRaidRequest(const SESSION_ID sessionId,
    const C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request)
{
    const auto connection = Find_Session(sessionId);
    if (!connection || !m_PlayerIdBySessionId.contains(sessionId)) return;
    const auto send = [&](const S2C_KOUKUSAYDON_RAID_STATE& state) {
        CPacketWriter writer;
        if (!Write_Message(writer, state) || !connection->Send_Frame(PACKET_TYPE::S2C_KOUKUSAYDON_RAID_STATE, writer.Get_Buffer())) connection->Request_Close();
    };
    S2C_KOUKUSAYDON_RAID_STATE rejection;
    rejection.eWorldId = request.eWorldId; rejection.iRequestSequence = request.iRequestSequence;
    rejection.strGateId = request.strStartGateId; rejection.ePhase = KOUKUSAYDON_RAID_PHASE::ABORTED;
    rejection.PinnedGameplayRevision = request.ExpectedGameplayRevision;
    rejection.iActionSourceRevision = request.iActionSourceRevision; rejection.iSequenceSourceRevision = request.iSequenceSourceRevision;
    rejection.iServerTick = m_iServerTick;
    const auto reject = [&](const char* reason) { rejection.strReason = reason; send(rejection); };
    CPacketWriter shape;
    if (m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA || !Write_Message(shape, request)) { reject("Invalid raid request scope"); return; }
    const auto previous = m_KoukuRaidReceipts.find(sessionId);
    if (previous != m_KoukuRaidReceipts.end() && request.iRequestSequence <= previous->second.first.iRequestSequence)
    {
        CPacketWriter old;
        if (Write_Message(old, previous->second.first) && old.Get_Buffer() == shape.Get_Buffer())
        {
            auto state = previous->second.second;
            if (state.iRunEpoch && state.iRunEpoch == m_KoukuRaid.State.iRunEpoch) state = m_KoukuRaid.State;
            state.iServerTick = m_iServerTick; send(state);
        }
        else reject("Raid request sequence was reused or is stale");
        return;
    }
    if (request.eOperation == KOUKUSAYDON_RAID_OPERATION::STOP)
    {
        if (!Is_KoukuRaidRunning() || sessionId != m_KoukuRaid.iOwnerSessionId || request.iExpectedRunEpoch != m_KoukuRaid.State.iRunEpoch)
        { reject("STOP does not own the current raid epoch"); return; }
        m_KoukuRaid.State.iRequestSequence = request.iRequestSequence;
        Stop_KoukuRaid("Complete Play stopped by its owner");
        m_KoukuRaidReceipts[sessionId] = {request, m_KoukuRaid.State}; return;
    }
    if (request.eOperation == KOUKUSAYDON_RAID_OPERATION::READY || request.eOperation == KOUKUSAYDON_RAID_OPERATION::FAILED)
    {
        std::string reason;
        if (!Apply_KoukuRaidReadiness(sessionId, request, reason)) { reject(reason.c_str()); return; }
        m_KoukuRaidReceipts[sessionId] = {request, m_KoukuRaid.State};
        send(m_KoukuRaid.State); return;
    }
    std::string reason;
    if (!Begin_KoukuRaidPreparation(sessionId, request, reason)) reject(reason.c_str());
}
bool LostArk::Server::CGameRoom::Begin_KoukuRaidPreparation(const SESSION_ID sessionId,
    const C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason, const bool clearedGate3, const bool bingoGateVoteEntry)
{
    reason.clear(); CPacketWriter validation;
    if (!Write_Message(validation, request)) { reason = "Invalid raid request shape"; return false; }
    if (request.eOperation != KOUKUSAYDON_RAID_OPERATION::START || m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA ||
        !m_PlayerIdBySessionId.contains(sessionId)) { reason = "Invalid raid START owner or scope"; return false; }
    if (bingoGateVoteEntry && (request.strStartGateId != "BINGO" || !m_GateProgress.iProposalId ||
        m_GateProgress.iProposerId != m_PlayerIdBySessionId.at(sessionId) ||
        m_GateProgress.Voters.empty() || m_GateProgress.Accepted.size() != m_GateProgress.Voters.size()))
    { reason = "Bingo entry requires its unanimous gate vote"; return false; }
    if (Is_KoukuRaidRunning() || (!clearedGate3 && ((!bingoGateVoteEntry && m_GateProgress.iProposalId) ||
        m_KoukuSaydonPatternAudition.ePhase != KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE)) ||
        (clearedGate3 && request.strStartGateId != "GATE3"))
    { reason = "A raid or pattern run already owns this room"; return false; }
    // An entry can retry while a companion finishes a transfer. Reject that
    // temporary condition before the full Product/actor preflight or epoch commit.
    if (!clearedGate3 && !bingoGateVoteEntry && std::any_of(m_Players.begin(), m_Players.end(), [](const auto& entry) {
        const auto& player = entry.second;
        return !player.iCurrentHp || player.iMarioStage || player.TriggerMove.isActive;
    })) { reason = "All room participants must be alive and outside a transfer"; return false; }
    auto product = m_GameplayCatalog.Get_ActiveGeneration();
    if (m_pKoukuPublishedProductGeneration && m_pKoukuPublishedProductGeneration->Has_SameNonKoukuGameplay(m_GameplayCatalog.Active())) product = m_pKoukuPublishedProductGeneration;
    if (!product || request.ExpectedGameplayRevision != m_GameplayCatalog.Get_ActiveRevision())
    { reason = "Gameplay generation changed; reload before Complete Play"; return false; }
    const auto currentSource = CKoukuSaydonBrain::Resolve_ProductSourceRevision(*product);
    const auto* currentGate = product->Find_KoukuRaidGate(request.strStartGateId);
    if (request.iActionSourceRevision != currentSource || !currentGate ||
        currentGate->iSequenceRevision != request.iSequenceSourceRevision)
    {
        auto candidate = std::make_shared<CGameplayCatalog>();
        if (!candidate->Load_PublishedKoukuProduct(m_GameplayCatalog.Active()) ||
            !candidate->Has_SameNonKoukuGameplay(m_GameplayCatalog.Active()) ||
            CKoukuSaydonBrain::Resolve_ProductSourceRevision(*candidate) != request.iActionSourceRevision ||
            request.iActionSourceRevision < currentSource)
        { reason = "Exact published raid Product could not be admitted; publish saved Action and Sequence data"; return false; }
        product = std::move(candidate);
    }
    for (const auto* gateId : {"GATE1", "GATE2", "GATE3", "BINGO"})
    {
        if (request.strStartGateId == "BINGO" ? std::string(gateId) != "BINGO" :
            (std::string(gateId) != "BINGO" && std::string(gateId) < request.strStartGateId)) continue;
        const auto* gate = product->Find_KoukuRaidGate(gateId);
        if (!gate && std::string(gateId) == "BINGO" && request.strStartGateId != "BINGO") continue;
        if (!gate || gate->iSequenceRevision != request.iSequenceSourceRevision || gate->Entries.empty() || !Find_Placement(gate->strPrimaryBossPlacementId))
        { reason = "Published raid flow or Sequence revision is missing or stale"; return false; }
        const auto* primaryProfile = product->Find_Boss(Find_Placement(gate->strPrimaryBossPlacementId)->strArchetypeId);
        if (!primaryProfile || std::any_of(gate->EntryGroups.begin(), gate->EntryGroups.end(), [&](const auto& group) {
            return group.RepeatUntilHealthBars && *group.RepeatUntilHealthBars > primaryProfile->iMaximumHealthBars; }))
        { reason = "Published HP repeat exceeds the primary boss health bars"; return false; }
        if (gate->strGateId != "BINGO" && std::count_if(gate->Arrivals.begin(), gate->Arrivals.end(), [](const auto& arrival) { return !arrival.bClear; }) != 4)
        { reason = "Published gate intro must define all four room arrival slots"; return false; }
        for (const auto& arrival : gate->Arrivals)
        {
            SERVER_NAV_POINT floor;
            if (!m_ServerNavigation.Is_PointWalkableExact(arrival.Position[0], arrival.Position[2]) ||
                !m_ServerNavigation.Sample_Position(arrival.Position[0], arrival.Position[2], floor) || std::abs(floor.y - arrival.Position[1]) > .25f)
            { reason = "Published Sequence arrival is outside the Server navigation floor"; return false; }
        }
        for (const auto& entry : gate->Entries)
        {
            std::vector<std::string> patterns;
            if (entry.bBundle) { const auto* bundle = product->Find_BossPatternBundle(entry.strTargetId); if (bundle) for (const auto& member : bundle->Members) patterns.push_back(member.strPatternId); }
            else patterns.push_back(entry.strTargetId);
            if (patterns.empty()) { reason = "Published flow has an empty bundle"; return false; }
            for (const auto& id : patterns)
            {
                std::string status;
                const auto* definition = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*product, id, status);
                const auto* placement = definition ? Find_Placement(definition->strTargetBossPlacementId) : nullptr;
                SERVER_WORLD_ENTITY candidate;
                if (!definition || !placement || !Build_WorldEntity(*placement, m_iNextNetEntityId, candidate))
                { reason = "Flow actor or pattern preflight failed"; return false; }
            }
        }
    }
    if (!m_iNextKoukuRaidEpoch || m_Players.empty() || m_Players.size() > 4u)
    { reason = "Complete Play requires one to four room participants and an available epoch"; return false; }
    KOUKU_RAID_RUN staged;
    staged.iOwnerSessionId = sessionId; staged.Request = request; staged.pCatalog = product; staged.State.eWorldId = m_eWorldId;
    staged.State.iRequestSequence = request.iRequestSequence; staged.State.strGateId = request.strStartGateId;
    staged.State.PinnedGameplayRevision = request.ExpectedGameplayRevision;
    staged.State.iActionSourceRevision = request.iActionSourceRevision; staged.State.iSequenceSourceRevision = request.iSequenceSourceRevision;
    staged.State.strReason.clear(); staged.State.iRunEpoch = m_iNextKoukuRaidEpoch;
    staged.State.iOwnerPlayerId = m_PlayerIdBySessionId.at(sessionId);
    for (const auto& [id, player] : m_Players) staged.PlayerIds.push_back(id);
    const auto receipt = m_KoukuSaydonPatternAuditionReceiptBySessionId.find(sessionId);
    if (receipt != m_KoukuSaydonPatternAuditionReceiptBySessionId.end())
    {
        staged.PriorAuditionRequest = receipt->second.Request; staged.PriorAuditionResult = receipt->second.Result;
        staged.PriorAuditionLifecycle = receipt->second.LastLifecycle;
    }
    staged.bClearedGate3Preparation = clearedGate3;
    staged.bBingoGateVoteEntry = bingoGateVoteEntry;
    ++m_iNextKoukuRaidEpoch;
    m_KoukuRaid = std::move(staged);
    const auto* gate = product->Find_KoukuRaidGate(request.strStartGateId);
    auto& state = m_KoukuRaid.State;
    state.ePhase = KOUKUSAYDON_RAID_PHASE::PREPARING;
    state.strSequenceCompositionId = gate->strSequenceCompositionId; state.strSequencePatternId = request.strStartGateId == "BINGO" && !bingoGateVoteEntry ? "" : gate->strIntroPatternId;
    state.ParticipantPlayerIds = m_KoukuRaid.PlayerIds;
    // F1 Complete Play and world entry share bounded resource readiness in both builds.
    constexpr uint32_t preparationTicks = 20u * 60u * 30u;
    state.iStartTick = 0u; state.iEndTick = Add_ServerTicksSkippingReservedZero(m_iServerTick, preparationTicks); state.iServerTick = m_iServerTick;
    m_KoukuRaidReceipts[sessionId] = {request, state};
    Broadcast_KoukuRaidState(); return true;
}
bool LostArk::Server::CGameRoom::Apply_KoukuRaidReadiness(const SESSION_ID sessionId,
    const C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason)
{
    reason.clear(); auto& run = m_KoukuRaid;
    const auto player = m_PlayerIdBySessionId.find(sessionId);
    if (player == m_PlayerIdBySessionId.end() || request.eWorldId != m_eWorldId ||
        request.iExpectedRunEpoch != run.State.iRunEpoch || !request.iExpectedRunEpoch ||
        request.ExpectedGameplayRevision != run.State.PinnedGameplayRevision ||
        request.iActionSourceRevision != run.State.iActionSourceRevision || request.iSequenceSourceRevision != run.State.iSequenceSourceRevision ||
        request.strStartGateId != run.Request.strStartGateId)
    { reason = "Raid preparation ACK has a stale epoch or revision"; return false; }
    const auto member = std::find(run.PlayerIds.begin(), run.PlayerIds.end(), player->second);
    if (member == run.PlayerIds.end()) { reason = "Late joins cannot acknowledge a pinned raid roster"; return false; }
    const auto bit = static_cast<std::uint8_t>(1u << std::distance(run.PlayerIds.begin(), member));
    if (request.eOperation == KOUKUSAYDON_RAID_OPERATION::READY && (run.State.iReadyMask & bit)) return true;
    if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::PREPARING)
    { reason = "This raid no longer accepts preparation ACKs"; return false; }
    if (Has_ReachedServerTick(m_iServerTick, run.State.iEndTick))
    { Stop_KoukuRaid("Raid preparation deadline expired"); reason = "Raid preparation timed out"; return false; }
    if (request.eOperation == KOUKUSAYDON_RAID_OPERATION::FAILED)
    {
        Stop_KoukuRaid("Player " + std::to_string(player->second) + " preparation failed: " + request.strReason); return true;
    }
    if (request.eOperation != KOUKUSAYDON_RAID_OPERATION::READY)
    { reason = "Invalid preparation operation"; return false; }
    run.State.iReadyMask |= bit;
    // The fixed tick revalidates the roster immediately before any destructive transition.
    run.State.iServerTick = m_iServerTick; Broadcast_KoukuRaidState(); return true;
}

bool LostArk::Server::CGameRoom::Is_KoukuRaidRunning() const
{
    const auto phase = m_KoukuRaid.State.ePhase;
    return phase == KOUKUSAYDON_RAID_PHASE::PREPARING || phase == KOUKUSAYDON_RAID_PHASE::CINEMATIC || phase == KOUKUSAYDON_RAID_PHASE::COMBAT ||
        phase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE || phase == KOUKUSAYDON_RAID_PHASE::WAIT_MINIGAME || phase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY;
}
bool LostArk::Server::CGameRoom::Build_KoukuRaidState(S2C_KOUKUSAYDON_RAID_STATE& state) const
{
    if (!m_KoukuRaid.State.iRunEpoch) return false;
    state = m_KoukuRaid.State; state.iServerTick = m_iServerTick; return true;
}
void LostArk::Server::CGameRoom::Broadcast_KoukuRaidState()
{
    auto state = m_KoukuRaid.State;
    CPacketWriter writer; if (!Write_Message(writer, state)) { m_strStatus = "Raid state serialization failed"; return; }
    for (const auto& [id, player] : m_Players)
        if (const auto session = Find_Session(player.iSessionId); session && !session->Send_Frame(PACKET_TYPE::S2C_KOUKUSAYDON_RAID_STATE, writer.Get_Buffer())) session->Request_Close();
    const auto receipt = m_KoukuRaidReceipts.find(m_KoukuRaid.iOwnerSessionId);
    if (receipt != m_KoukuRaidReceipts.end()) receipt->second.second = state;
}
void LostArk::Server::CGameRoom::Stop_KoukuRaid(std::string reason, const bool completed)
{
    if (!m_KoukuRaid.State.iRunEpoch) return;
    const bool preparing = m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING;
    m_KoukuRaid.State.ePhase = completed ? KOUKUSAYDON_RAID_PHASE::COMPLETE : KOUKUSAYDON_RAID_PHASE::ABORTED;
    if (reason.size() > MAX_KOUKUSAYDON_PATTERN_AUDITION_REASON_BYTES)
    {
        auto cut = MAX_KOUKUSAYDON_PATTERN_AUDITION_REASON_BYTES;
        while (cut && (static_cast<unsigned char>(reason[cut]) & 0xc0u) == 0x80u) --cut;
        reason.resize(cut);
    }
    m_KoukuRaid.State.strReason = std::move(reason); m_KoukuRaid.State.iServerTick = m_iServerTick;
    if (m_GateProgress.iProposalId && m_GateProgress.iRaidEpoch == m_KoukuRaid.State.iRunEpoch)
        Close_GateProgressVote(GATE_PROGRESS_VOTE_RESULT::CANCELLED);
    if (preparing)
    {
        if (!m_KoukuRaid.strEntryTriggerSequenceId.empty())
            m_ServerTriggerSystem.Reset_SequenceActivation(m_KoukuRaid.strEntryTriggerSequenceId);
        Broadcast_KoukuRaidState(); return;
    }
    if (m_KoukuSaydonPatternAudition.iRoomAuditionEpoch != 0u) Clear_KoukuSaydonPatternAudition(false, "Raid stopped");
    Stop_KoukuBingoDuration(true);
    Reset_CardMaze();
    (void)Despawn_KoukuSaydonArenaDebugEntities(true);
    for (auto& [id, player] : m_Players) { player.Clear_KoukuInteractionState(); player.Clear_MarioControl(); }
    if (m_KoukuRaid.PriorAuditionRequest && m_KoukuRaid.PriorAuditionResult)
    {
        auto& previous = m_KoukuSaydonPatternAuditionReceiptBySessionId[m_KoukuRaid.iOwnerSessionId];
        previous.Request = *m_KoukuRaid.PriorAuditionRequest; previous.Result = *m_KoukuRaid.PriorAuditionResult; previous.LastLifecycle = m_KoukuRaid.PriorAuditionLifecycle;
    }
    else m_KoukuSaydonPatternAuditionReceiptBySessionId.erase(m_KoukuRaid.iOwnerSessionId);
    m_KoukuRaid.bEntryRunning = false;
    Broadcast_KoukuRaidState();
}
bool LostArk::Server::CGameRoom::Begin_KoukuRaidCinematic(const std::string& gateId, const bool clear, const std::uint32_t tick)
{
    const auto* gate = m_KoukuRaid.pCatalog->Find_KoukuRaidGate(gateId);
    if (!gate || (clear ? gate->strClearPatternId.empty() : gate->strIntroPatternId.empty())) return false;
    if (!Despawn_KoukuSaydonArenaDebugEntities(true)) return false;
    auto& run = m_KoukuRaid; run.bClearCinematic = clear; run.CompletedArrivals.clear(); run.iPrimaryBossId = INVALID_NET_ENTITY_ID;
    run.bEntryRunning = false; run.iNextEntryTick = 0u;
    run.State.strGateId = gateId; run.State.ePhase = KOUKUSAYDON_RAID_PHASE::CINEMATIC;
    run.State.strSequenceCompositionId = gate->strSequenceCompositionId;
    run.State.strSequencePatternId = clear ? gate->strClearPatternId : gate->strIntroPatternId;
    run.State.iSequenceSourceRevision = gate->iSequenceRevision;
    run.State.iStartTick = tick; run.State.iEndTick = Add_ServerTicksSkippingReservedZero(tick, CKoukuSaydonLogicRuntime::Ticks_FromMs(clear ? gate->iClearDurationMs : gate->iIntroDurationMs));
    run.State.iServerTick = m_iServerTick; run.State.iFlowEntryIndex = 0u; run.State.strFlowEntryId.clear();
    for (auto& [id, player] : m_Players)
    {
        Reset_PlayerForDebugTeleport(player);
        // A new gate cinematic ends the previous gate's persistent overhead cards.
        player.Clear_KoukuAssignedCard();
    }
    Broadcast_KoukuRaidState(); return true;
}
bool LostArk::Server::CGameRoom::Start_KoukuRaidCombat(const std::uint32_t tick, const std::string& preflightGateId)
{
    auto& run = m_KoukuRaid;
    const auto* gate = run.pCatalog ? run.pCatalog->Find_KoukuRaidGate(preflightGateId.empty() ? run.State.strGateId : preflightGateId) : nullptr;
    if (!gate) return false;
    std::set<std::string> placements{gate->strPrimaryBossPlacementId};
    for (const auto& entry : gate->Entries)
    {
        if (entry.bBundle) { const auto* bundle = run.pCatalog->Find_BossPatternBundle(entry.strTargetId); if (!bundle) return false; for (const auto& member : bundle->Members) placements.insert(member.strTargetBossPlacementId); }
        else { std::string status; const auto* pattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*run.pCatalog, entry.strTargetId, status); if (!pattern) return false; placements.insert(pattern->strTargetBossPlacementId); }
    }
    std::vector<SERVER_WORLD_ENTITY> staged;
    auto nextId = m_iNextNetEntityId;
    auto primaryBossId = INVALID_NET_ENTITY_ID;
    for (const auto& placementId : placements)
    {
        const auto* placement = Find_Placement(placementId); SERVER_WORLD_ENTITY boss;
        if (!placement || !nextId || !Build_WorldEntity(*placement, nextId++, boss)) return false;
        if (placementId == gate->strPrimaryBossPlacementId) primaryBossId = boss.iNetEntityId;
        staged.push_back(std::move(boss));
    }
    C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request;
    if (!Build_KoukuRaidEntryRequest(*gate, 0u, request)) return false;
    const auto published = m_pKoukuPublishedProductGeneration; m_pKoukuPublishedProductGeneration = run.pCatalog;
    S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result;
    const auto verdict = Evaluate_KoukuSaydonPatternAudition(run.iOwnerSessionId, request, result, false, &staged);
    m_pKoukuPublishedProductGeneration = published;
    if (verdict != KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED) { m_strStatus = result.strReason; return false; }
    if (!preflightGateId.empty()) return true;
    run.iPrimaryBossId = primaryBossId;
    for (auto& entity : staged)
    {
        m_WorldEntities.push_back(std::move(entity)); Broadcast_WorldEntitySpawned(m_WorldEntities.back());
        for (auto& [id, player] : m_Players) Apply_KoukuGateEntryCard(player, m_WorldEntities.back());
    }
    Note_GatePlacementRaised(gate->strPrimaryBossPlacementId);
    m_iNextNetEntityId = nextId; run.State.ePhase = KOUKUSAYDON_RAID_PHASE::COMBAT;
    run.State.strSequencePatternId.clear(); run.State.iStartTick = tick; run.State.iEndTick = 0u; run.State.iFlowEntryIndex = 0u;
    run.State.iServerTick = tick;
    if (!Start_KoukuRaidEntry(tick)) return false;
    Broadcast_KoukuRaidState(); return true;
}
bool LostArk::Server::CGameRoom::Build_KoukuRaidEntryRequest(const KOUKU_RAID_GATE_DEFINITION& gate,
    const std::uint32_t entryIndex, C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request)
{
    const auto& run = m_KoukuRaid;
    if (!run.pCatalog || entryIndex >= gate.Entries.size()) return false;
    const auto& entry = gate.Entries[entryIndex];
    const auto old = m_KoukuSaydonPatternAuditionReceiptBySessionId.find(run.iOwnerSessionId);
    const auto prior = old == m_KoukuSaydonPatternAuditionReceiptBySessionId.end() ? 0u : old->second.Request.iRequestSequence;
    if (prior == UINT32_MAX) { m_strStatus = "Raid audition request sequence is exhausted."; return false; }
    request = {};
    request.iRequestSequence = prior + 1u; request.eOperation = entry.bBundle ? KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_BUNDLE : KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
    request.Scope.eWorldId = m_eWorldId; request.Scope.strEncounterId = gate.strEncounterId; request.Scope.strGateId = gate.strGateId;
    request.Scope.ExpectedGameplayRevision = run.State.PinnedGameplayRevision; request.Scope.iExpectedSourceRevision = run.State.iActionSourceRevision;
    std::string placementId;
    if (entry.bBundle) { request.strBundleId = entry.strTargetId; const auto* bundle = run.pCatalog->Find_BossPatternBundle(entry.strTargetId); if (!bundle || bundle->Members.empty()) return false; placementId = bundle->Members.front().strTargetBossPlacementId; }
    else { request.strPatternId = entry.strTargetId; std::string status; const auto* pattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*run.pCatalog, entry.strTargetId, status); if (!pattern) return false; placementId = pattern->strTargetBossPlacementId; }
    const auto* placement = Find_Placement(placementId); if (!placement) return false;
    request.Scope.strBossPlacementId = placementId; request.Scope.strBossArchetypeId = placement->strArchetypeId;
    return true;
}
bool LostArk::Server::CGameRoom::Start_KoukuRaidEntry(const std::uint32_t tick)
{
    auto& run = m_KoukuRaid; const auto* gate = run.pCatalog->Find_KoukuRaidGate(run.State.strGateId);
    if (!gate || run.State.iFlowEntryIndex >= gate->Entries.size()) return true;
    const auto primary = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(), [&](const auto& boss) { return boss.iNetEntityId == run.iPrimaryBossId; });
    if (primary == m_WorldEntities.end()) { m_strStatus = "Flow primary boss is missing"; return false; }
    run.State.iFlowEntryIndex = gate->Resolve_ReadyEntry(run.State.iFlowEntryIndex,
        primary->iCurrentHp, primary->iMaximumHp, primary->iMaximumHealthBars);
    if (run.State.iFlowEntryIndex >= gate->Entries.size()) return true;
    C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request;
    if (!Build_KoukuRaidEntryRequest(*gate, run.State.iFlowEntryIndex, request)) return false;
    const auto published = m_pKoukuPublishedProductGeneration; m_pKoukuPublishedProductGeneration = run.pCatalog;
    S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result;
    const bool continueLoop = (run.State.strGateId == "GATE1" || !gate->strLoopStartEntryId.empty() ||
        std::any_of(gate->EntryGroups.begin(), gate->EntryGroups.end(), [](const auto& group) { return group.RepeatUntilHealthBars.has_value(); })) && run.iAuditionEpoch != 0u &&
        m_KoukuSaydonPatternAudition.iRoomAuditionEpoch == run.iAuditionEpoch &&
        m_KoukuSaydonPatternAudition.Request.Scope.strGateId == run.State.strGateId;
    const auto verdict = Evaluate_KoukuSaydonPatternAudition(run.iOwnerSessionId, request, result,
        (run.State.iFlowEntryIndex > 0u || run.State.strGateId == "BINGO" || continueLoop) && run.iAuditionEpoch != 0u);
    m_pKoukuPublishedProductGeneration = published;
    if (verdict != KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED) { m_strStatus = result.strReason; return false; }
    run.iAuditionRequestSequence = request.iRequestSequence; run.iAuditionEpoch = result.iRoomAuditionEpoch; run.bEntryRunning = true;
    run.State.strFlowEntryId = gate->Entries[run.State.iFlowEntryIndex].strEntryId; run.State.iServerTick = tick; Broadcast_KoukuRaidState(); return true;
}
void LostArk::Server::CGameRoom::Notify_KoukuRaidBossDeath(const SERVER_WORLD_ENTITY& boss, const std::uint32_t tick)
{
    // Legacy F1 and in-game gate entry create the same authored G3 boss without a raid epoch.
    // Admit its clear through normal pinned resource preparation before starting Encore.
    if (!Is_KoukuRaidRunning() && m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA &&
        boss.strPlacementId == "boss.kakulsaydon.g3.saydon" &&
        (boss.iCurrentHp == 0u || boss.eAction == SERVER_ENTITY_ACTION::DEAD) && !m_Players.empty())
    {
        auto product = m_GameplayCatalog.Get_ActiveGeneration();
        if (m_pKoukuPublishedProductGeneration && m_pKoukuPublishedProductGeneration->Has_SameNonKoukuGameplay(m_GameplayCatalog.Active()))
            product = m_pKoukuPublishedProductGeneration;
        const auto* gate = product ? product->Find_KoukuRaidGate("GATE3") : nullptr;
        const auto* bingo = product ? product->Find_KoukuRaidGate("BINGO") : nullptr;
        const auto* placement = Find_Placement(boss.strPlacementId);
        const auto owner = m_PlayerIdBySessionId.contains(m_KoukuSaydonPatternAudition.iOwnerSessionId) ?
            m_KoukuSaydonPatternAudition.iOwnerSessionId : m_Players.begin()->second.iSessionId;
        const auto old = m_KoukuRaidReceipts.find(owner);
        const auto sequence = old == m_KoukuRaidReceipts.end() ? 1u : old->second.first.iRequestSequence + 1u;
        if (gate && bingo && !bingo->strIntroPatternId.empty() && placement && sequence &&
            gate->strPrimaryBossPlacementId == boss.strPlacementId && placement->strArchetypeId == boss.strArchetypeId)
        {
            C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request;
            request.eWorldId = m_eWorldId; request.eOperation = KOUKUSAYDON_RAID_OPERATION::START;
            request.iRequestSequence = sequence; request.strStartGateId = "GATE3";
            request.ExpectedGameplayRevision = m_GameplayCatalog.Get_ActiveRevision();
            request.iActionSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(*product);
            request.iSequenceSourceRevision = gate->iSequenceRevision;
            std::string reason;
            if (Begin_KoukuRaidPreparation(owner, request, reason, true))
            {
                auto& prepared = m_KoukuRaid;
                prepared.iPrimaryBossId = boss.iNetEntityId; prepared.iGate3ClearTick = tick;
                prepared.bGate3CombatEntered = true;
                if (m_GateProgress.iProposalId) Close_GateProgressVote(GATE_PROGRESS_VOTE_RESULT::CANCELLED);
                if (m_KoukuSaydonPatternAudition.iRoomAuditionEpoch)
                    Clear_KoukuSaydonPatternAudition(false, "Gate 3 defeated; preparing Encore");
                m_GateProgress.iCurrentGate = 3u; m_GateProgress.iClearedMask |= 4u;
                Broadcast_GateProgressState(false, GATE_PROGRESS_VOTE_RESULT::NONE);
                return;
            }
            m_strStatus = "Gate 3 Encore admission failed: " + reason;
        }
    }
    auto& run = m_KoukuRaid;
    if (!Is_KoukuRaidRunning() || (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT && run.State.ePhase != KOUKUSAYDON_RAID_PHASE::WAIT_MINIGAME) ||
        boss.iNetEntityId != run.iPrimaryBossId || (boss.iCurrentHp && boss.eAction != SERVER_ENTITY_ACTION::DEAD)) return;
    run.State.ePhase = KOUKUSAYDON_RAID_PHASE::WAIT_GATE; run.State.iStartTick = tick;
    run.State.iEndTick = 0u; run.State.iServerTick = tick; run.bEntryRunning = false;
    if (m_KoukuSaydonPatternAudition.iRoomAuditionEpoch != 0u) Clear_KoukuSaydonPatternAudition(false, "Gate boss defeated");
    const auto* gate = run.pCatalog->Find_KoukuRaidGate(run.State.strGateId);
    const int index = gate ? Gate_IndexOfPlacement(gate->strPrimaryBossPlacementId) : -1;
    const auto* bingo = run.pCatalog->Find_KoukuRaidGate("BINGO");
    const bool pendingEncore = run.State.strGateId == "GATE3" && bingo && !bingo->strIntroPatternId.empty();
    // The false clear belongs to the raid clock, not a Client UI timer or a vote.
    if (pendingEncore) run.State.iEndTick = Add_ServerTicksSkippingReservedZero(tick, CKoukuSaydonLogicRuntime::Ticks_FromMs(KOUKU_GATE3_CLEAR_HOLD_MS));
    const bool pendingEncoreEnding = gate && run.State.strGateId == "BINGO" && !gate->strClearPatternId.empty();
    if (index >= 0 && !pendingEncoreEnding)
    {
        m_GateProgress.iCurrentGate = static_cast<std::uint8_t>(index + 1);
        m_GateProgress.iClearedMask |= static_cast<std::uint8_t>(1u << index);
        Broadcast_GateProgressState(false, GATE_PROGRESS_VOTE_RESULT::NONE);
        if (!pendingEncore) Broadcast_RaidMvpResult(static_cast<std::uint8_t>(index + 1));
    }
    Broadcast_KoukuRaidState();
}
bool LostArk::Server::CGameRoom::Advance_KoukuRaidGate(const std::uint8_t nextGate, const bool restart)
{
    auto& run = m_KoukuRaid;
    if (!Is_KoukuRaidRunning() || !run.pCatalog || run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING ||
        run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC) return false;
    if (run.State.strGateId == "GATE3" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && run.State.iEndTick) return false;
    const auto current = m_GateProgress.iCurrentGate;
    if (!current || (restart ? nextGate != current :
        (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::WAIT_GATE || nextGate != current + 1u))) return false;
    const std::string target = nextGate == 4u ? "BINGO" : "GATE" + std::to_string(nextGate);
    const auto* targetGate = run.pCatalog->Find_KoukuRaidGate(target);
    if (!targetGate || run.PlayerIds.empty()) return false;
    if (run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY)
        return restart && nextGate == 3u && Enter_KoukuRaidCombat(nextGate);
    for (const auto id : run.PlayerIds)
    {
        const auto player = m_Players.find(id);
        if (player == m_Players.end() || !m_GameplayCatalog.Find_Player(player->second.eCharacterClass)) return false;
    }
    if (nextGate == 4u) return Enter_KoukuRaidCombat(4u);
    // The existing unanimous vote authorizes a reset, including reviving dead participants.
    if (m_KoukuSaydonPatternAudition.iRoomAuditionEpoch != 0u)
        Clear_KoukuSaydonPatternAudition(false, restart ? "Gate restart approved" : "Next gate approved");
    Stop_KoukuBingoDuration(true);
    Reset_CardMaze();
    // The next gate's intro owns this transition; the G2 clear movie is not replayed here.
    if (!Begin_KoukuRaidCinematic(target, false, Add_ServerTicksSkippingReservedZero(m_iServerTick, 1u))) return false;
    m_iNextMarioEntryStage = 1u;
    for (const auto id : run.PlayerIds)
    {
        auto& player = m_Players.at(id);
        const auto* profile = m_GameplayCatalog.Find_Player(player.eCharacterClass);
        player.Clear_KoukuInteractionState(); Reset_PlayerForDebugTeleport(player);
        player.iCurrentHp = player.iMaximumHp; player.iCurrentResource = player.iMaximumResource;
        player.iResourceAccumulator = 0u; CPlayerSkillSystem::Reset_Gauges(player, m_GameplayCatalog);
        player.iCurrentMadness = 0u; player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
        player.eStance = profile->eDefaultStance; player.CooldownEndTickBySkillId.clear();
    }
    return true;
}
void LostArk::Server::CGameRoom::Update_KoukuRaid(const std::uint32_t tick)
{
    if (!Is_KoukuRaidRunning()) return;
    auto& run = m_KoukuRaid;
    if (!m_PlayerIdBySessionId.contains(run.iOwnerSessionId) || std::any_of(run.PlayerIds.begin(), run.PlayerIds.end(), [&](auto id) { return !m_Players.contains(id); }))
    { Stop_KoukuRaid("A raid participant left the room"); return; }
    if (run.State.PinnedGameplayRevision != m_GameplayCatalog.Get_ActiveRevision()) { Stop_KoukuRaid("Global gameplay generation changed"); return; }
    const auto* gate = run.pCatalog->Find_KoukuRaidGate(run.State.strGateId); if (!gate) { Stop_KoukuRaid("Pinned gate disappeared"); return; }
    if (run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING)
    {
        if (Has_ReachedServerTick(tick, run.State.iEndTick)) { Stop_KoukuRaid("Raid preparation deadline expired"); return; }
        if (run.State.iReadyMask == (1u << run.PlayerIds.size()) - 1u)
        {
            for (const auto id : run.PlayerIds)
            {
                const auto& player = m_Players.at(id);
                if (!run.bClearedGate3Preparation && !run.bBingoGateVoteEntry && (!player.iCurrentHp || player.iMarioStage || player.TriggerMove.isActive))
                { Stop_KoukuRaid("A participant became unavailable while preparing"); return; }
            }
            m_pKoukuPublishedProductGeneration = run.pCatalog;
            if (run.bClearedGate3Preparation)
            {
                run.bClearedGate3Preparation = false;
                run.State.ePhase = KOUKUSAYDON_RAID_PHASE::WAIT_GATE;
                run.State.strSequencePatternId.clear(); run.State.iStartTick = run.iGate3ClearTick;
                run.State.iEndTick = Add_ServerTicksSkippingReservedZero(run.iGate3ClearTick, CKoukuSaydonLogicRuntime::Ticks_FromMs(KOUKU_GATE3_CLEAR_HOLD_MS));
                run.State.iServerTick = tick; Broadcast_KoukuRaidState();
            }
            else if (run.Request.strStartGateId == "BINGO" && !run.bBingoGateVoteEntry ? !Enter_KoukuRaidCombat(4u) :
                !Begin_KoukuRaidCinematic(run.Request.strStartGateId, false, Add_ServerTicksSkippingReservedZero(tick, 1u)))
                Stop_KoukuRaid("Sequence admission failed after preparation");
        }
    }
    else if (run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC)
    {
        std::vector<std::pair<PLAYER_ID, SERVER_NAV_POINT>> arrivals;
        std::vector<std::string> completedArrivals;
        for (const auto& arrival : gate->Arrivals)
        {
            if (arrival.bClear != run.bClearCinematic || run.CompletedArrivals.contains(arrival.strOccurrenceId) ||
                !Has_ReachedServerTick(tick, Add_ServerTicksSkippingReservedZero(run.State.iStartTick, CKoukuSaydonLogicRuntime::Ticks_FromMs(arrival.iStartMs)))) continue;
            if (arrival.iSlot < run.PlayerIds.size())
            {
                const auto id = run.PlayerIds[arrival.iSlot]; const auto& player = m_Players.at(id); SERVER_NAV_POINT ground;
                C2S_DEBUG_TELEPORT_TO_POSITION request; request.eWorldId = m_eWorldId;
                request.fPositionX = arrival.Position[0]; request.fPositionY = arrival.Position[1]; request.fPositionZ = arrival.Position[2];
                if (Validate_DebugTeleportDestination(player, request, ground) != DEBUG_TELEPORT_RESULT::ACCEPTED)
                { Stop_KoukuRaid("Authored Sequence player arrival was rejected"); return; }
                arrivals.emplace_back(id, ground);
            }
            completedArrivals.push_back(arrival.strOccurrenceId);
        }
        for (const auto& [id, ground] : arrivals)
        {
            auto& player = m_Players.at(id); Reset_PlayerForDebugTeleport(player);
            player.fPositionX = ground.x; player.fPositionY = ground.y; player.fPositionZ = ground.z;
        }
        run.CompletedArrivals.insert(completedArrivals.begin(), completedArrivals.end());
        if (Has_ReachedServerTick(tick, run.State.iEndTick))
        {
            if (run.bClearCinematic && run.State.strGateId == "BINGO")
            {
                run.State.ePhase = KOUKUSAYDON_RAID_PHASE::WAIT_GATE;
                run.State.strSequencePatternId.clear(); run.State.iStartTick = tick; run.State.iEndTick = 0u;
                run.State.iServerTick = tick;
                m_GateProgress.iCurrentGate = 4u;
                m_GateProgress.iClearedMask |= static_cast<std::uint8_t>(1u << 3u);
                Broadcast_KoukuRaidState(); Broadcast_GateProgressState(false, GATE_PROGRESS_VOTE_RESULT::NONE);
                Broadcast_RaidMvpResult(4u);
            }
            else if (!run.bClearCinematic && run.State.strGateId == "BINGO")
            { if (!Enter_KoukuRaidCombat(4u)) Stop_KoukuRaid("Encore combat entry failed: " + m_strStatus); }
            else if (run.bClearCinematic)
            { if (!Begin_KoukuRaidCinematic("GATE3", false, Add_ServerTicksSkippingReservedZero(tick, 1u))) Stop_KoukuRaid("Gate 3 Sequence failed"); }
            else if (run.State.strGateId == "GATE3" && run.bGate3CombatEntered)
            { if (!Enter_KoukuRaidCombat(3u)) Stop_KoukuRaid("Gate 3 restart entry failed: " + m_strStatus); }
            else if (run.State.strGateId == "GATE3")
            {
                run.State.ePhase = KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY;
                run.State.strSequencePatternId.clear(); run.State.iStartTick = tick; run.State.iEndTick = 0u;
                run.State.iServerTick = tick; run.bEntryRunning = false;
                m_GateProgress.iCurrentGate = 3u;
                m_GateProgress.iClearedMask &= static_cast<std::uint8_t>(~(1u << 2u));
                Broadcast_KoukuRaidState(); Broadcast_GateProgressState(false, GATE_PROGRESS_VOTE_RESULT::NONE);
            }
            else if (!Start_KoukuRaidCombat(tick)) Stop_KoukuRaid("Gate combat admission failed: " + m_strStatus);
        }
    }
    else if (run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE || run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY)
    {
        if (run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE &&
            run.State.strGateId == "GATE3" && run.State.iEndTick && Has_ReachedServerTick(tick, run.State.iEndTick))
        {
            if (!Begin_KoukuRaidCinematic("BINGO", false, tick))
                Stop_KoukuRaid("Encore intro Sequence failed");
            return;
        }
        // Defer cleanup until outside the boss iteration that reported death.
        if (run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE &&
            run.State.strGateId == "BINGO" && !run.bClearCinematic && !gate->strClearPatternId.empty())
        {
            Stop_KoukuBingoDuration(true);
            if (!Begin_KoukuRaidCinematic("BINGO", true, Add_ServerTicksSkippingReservedZero(tick, 1u)))
                Stop_KoukuRaid("Bingo ending Sequence failed");
        }
        // Other cleared gates wait for the existing unanimous entry vote.
    }
    else
    {
        const auto primary = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(), [&](const auto& boss) { return boss.iNetEntityId == run.iPrimaryBossId; });
        if (primary == m_WorldEntities.end())
        { Stop_KoukuRaid("Gate boss was removed without a death event"); return; }
        const bool mazeActive = m_KoukuCardMaze.Get_Phase() != CKoukuCardMazeRuntime::PHASE::INACTIVE ||
            std::any_of(m_Players.begin(), m_Players.end(), [](const auto& pair) {
                const auto& player = pair.second;
                // P28 commits maze entry before the telescope assigns roles or starts HUNTING.
                // The area owner is cleared only after the return transfer (or player death).
                return player.eCardMazeRole != CARD_MAZE_ROLE::NONE ||
                    (player.iCurrentHp && player.eAction != PLAYER_ACTION_STATE::DEAD &&
                        player.eKoukuAreaHudMode == KOUKU_HUD_MODE::MAZE);
            });
        const auto desired = mazeActive ? KOUKUSAYDON_RAID_PHASE::WAIT_MINIGAME : KOUKUSAYDON_RAID_PHASE::COMBAT;
        if (run.State.ePhase != desired) { run.State.ePhase = desired; run.State.iServerTick = tick; Broadcast_KoukuRaidState(); }
        if (run.bEntryRunning && m_KoukuSaydonPatternAudition.ePhase == KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE)
        {
            const auto receipt = m_KoukuSaydonPatternAuditionReceiptBySessionId.find(run.iOwnerSessionId);
            if (receipt == m_KoukuSaydonPatternAuditionReceiptBySessionId.end() || !receipt->second.LastLifecycle ||
                receipt->second.LastLifecycle->iRoomAuditionEpoch != run.iAuditionEpoch || receipt->second.LastLifecycle->eState != KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED)
            { Stop_KoukuRaid("Flow occurrence aborted before completion"); return; }
            run.bEntryRunning = false;
            run.iNextEntryTick = Add_ServerTicksSkippingReservedZero(tick, CKoukuSaydonLogicRuntime::Ticks_FromMs(gate->Entries[run.State.iFlowEntryIndex].iWaitAfterMs));
            run.State.iFlowEntryIndex = gate->Resolve_NextCompletedEntry(run.State.iFlowEntryIndex,
                primary->iCurrentHp, primary->iMaximumHp, primary->iMaximumHealthBars);
            if (run.State.iFlowEntryIndex == gate->Entries.size())
            {
                if (!gate->strLoopStartEntryId.empty())
                {
                    const auto loopStart = std::find_if(gate->Entries.begin(), gate->Entries.end(),
                        [&](const auto& entry) { return entry.strEntryId == gate->strLoopStartEntryId; });
                    if (loopStart == gate->Entries.end()) { Stop_KoukuRaid("Published flow loop start is missing"); return; }
                    run.State.iFlowEntryIndex = static_cast<std::uint32_t>(loopStart - gate->Entries.begin());
                }
                else if ((run.State.strGateId == "BINGO" || run.State.strGateId == "GATE1") &&
                    std::none_of(gate->EntryGroups.begin(), gate->EntryGroups.end(), [](const auto& group) { return group.RepeatUntilHealthBars.has_value(); }))
                    run.State.iFlowEntryIndex = 0u;
            }
        }
        if (!run.bEntryRunning && !mazeActive && run.State.iFlowEntryIndex < gate->Entries.size() && Has_ReachedServerTick(tick, run.iNextEntryTick))
            if (!Start_KoukuRaidEntry(tick)) Stop_KoukuRaid("Next flow entry rejected: " + m_strStatus);
    }
    if (Is_KoukuRaidRunning() && tick % 30u == 0u) { run.State.iServerTick = tick; Broadcast_KoukuRaidState(); }
}
