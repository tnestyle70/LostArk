#include "GameRoom.h"
#include "GameRoom_Internal.h"

#include "ClientSession.h"
#include "KoukuSaydonBrain.h"
#include "Gameplay/KoukuArenaReadyAreas.h"
#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"

#include <algorithm>
#include <memory>

/* Commander raid gate progress for the KoukuSaydon arena.

   The gate table is the product counterpart of the Client's F1 gate buttons: the same
   disabled boss placements of Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json and the
   same authored player positions (captured from Move Player; Server navigation still
   validates them on every move). A gate clears when its last primary boss dies; the vote
   that follows mirrors the party raid entry vote (leader / solo proposes, every member
   answers, 30 s timeout); ALL_ACCEPTED switches the arena to the next gate here, in the
   room, so a Release Server runs the same path the Debug buttons exercise by hand. */
namespace
{
	using LostArk::Shared::PLAYER_ID;

	struct KOUKU_GATE
	{
		const char* Placements[2];
		float fX, fY, fZ;
	};

	constexpr KOUKU_GATE KOUKU_GATES[] =
	{
		{ { "boss.kakulsaydon.g1.saydon", nullptr }, -2.45f, 1.32f, 740.37f },
		{ { "boss.kakulsaydon.g2.big-saydon", "boss.kakulsaydon.g2.kouku" }, 3.38f, 10.56f, 323.92f },
		{ { "boss.kakulsaydon.g3.saydon", nullptr }, -2.45f, 1.32f, 945.17f },
		{ { "boss.kakulsaydon.bingo.saydon", nullptr }, -3.4f, 0.f, 1147.44f },
	};
	constexpr std::uint8_t KOUKU_GATE_COUNT = static_cast<std::uint8_t>(std::size(KOUKU_GATES));

	/* 30 Hz ticks: the same 30 s the raid entry vote gives. */
	constexpr std::uint32_t GATE_VOTE_TIMEOUT_TICKS = 30u * 30u;
}

std::uint8_t LostArk::Server::CGameRoom::Gate_Count() const
{
	return LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId ? KOUKU_GATE_COUNT : 0u;
}

int LostArk::Server::CGameRoom::Gate_IndexOfPlacement(const std::string& placementId) const
{
	if (0u == Gate_Count() || placementId.empty())
		return -1;
	for (int iGate = 0; iGate < static_cast<int>(KOUKU_GATE_COUNT); ++iGate)
		for (const char* pPlacement : KOUKU_GATES[iGate].Placements)
			if (nullptr != pPlacement && placementId == pPlacement)
				return iGate;
	return -1;
}

void LostArk::Server::CGameRoom::Note_GatePlacementRaised(const std::string& placementId)
{
	const int iGate = Gate_IndexOfPlacement(placementId);
	if (iGate < 0)
		return;
	const std::uint8_t iRaised = static_cast<std::uint8_t>(iGate + 1);
	if (m_GateProgress.iCurrentGate == iRaised && !(m_GateProgress.iClearedMask & (1u << iGate)))
		return;
	/* A gate raised again (Debug button after a clear) fights again. */
	m_GateMvpLedger.clear();
	m_GateProgress.iCurrentGate = iRaised;
	m_GateProgress.iClearedMask &= static_cast<std::uint8_t>(~(1u << iGate));
	Broadcast_GateProgressState(false, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT::NONE);
}

void LostArk::Server::CGameRoom::Notify_GateBossDeath(const SERVER_WORLD_ENTITY& deadBoss)
{
    // A pinned raid defines its primary boss; Gate 2's supporting actor cannot delay or trigger its clear.
    if (Is_KoukuRaidRunning() && (m_KoukuRaid.State.ePhase != LostArk::Shared::KOUKUSAYDON_RAID_PHASE::PREPARING ||
        m_KoukuRaid.bClearedGate3Preparation)) return;
	const int iGate = Gate_IndexOfPlacement(deadBoss.strPlacementId);
	if (iGate < 0)
		return;
	/* Another boss of the same gate still standing: not cleared yet (gate 2 has two). */
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (entity.iNetEntityId == deadBoss.iNetEntityId ||
			WORLD_BOOTSTRAP_KIND::BOSS != entity.eKind)
			continue;
		if (Gate_IndexOfPlacement(entity.strPlacementId) == iGate)
			return;
	}
	m_GateProgress.iCurrentGate = static_cast<std::uint8_t>(iGate + 1);
	m_GateProgress.iClearedMask |= static_cast<std::uint8_t>(1u << iGate);
	Broadcast_GateProgressState(false, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT::NONE);
	Broadcast_RaidMvpResult(static_cast<std::uint8_t>(iGate + 1));
}

void LostArk::Server::CGameRoom::Tick_MvpLedgers()
{
	for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		/* The clock starts with the first recorded hit and stops with the boss. */
		if (WORLD_BOOTSTRAP_KIND::BOSS != entity.eKind || entity.MvpLedger.empty() ||
			LostArk::Shared::INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId ||
			0u == entity.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == entity.eAction)
			continue;
		for (const auto& [playerId, player] : m_Players)
		{
			SERVER_MVP_LEDGER_ROW& row = Find_Or_Add_MvpLedgerRow(entity.MvpLedger, playerId);
			++row.iFightTicks;
			if (0u != player.iCurrentHp && LostArk::Shared::PLAYER_ACTION_STATE::DEAD != player.eAction)
				++row.iAliveTicks;
			if (row.bHpSeen && player.iCurrentHp < row.iLastHp)
				++row.iDamagingHitsTaken;
			row.iLastHp = player.iCurrentHp;
			row.bHpSeen = true;
			const bool knockedDown = LostArk::Shared::PLAYER_ACTION_STATE::KNOCKDOWN == player.eAction;
			if (knockedDown && !row.bWasKnockedDown)
				++row.iKnockdowns;
			row.bWasKnockedDown = knockedDown;
			if (0u != player.iMaximumHp)
			{
				const std::uint64_t permille =
					static_cast<std::uint64_t>(player.iCurrentHp) * 1000u / player.iMaximumHp;
				row.iLowestHpPermille = (std::min)(row.iLowestHpPermille,
					static_cast<std::uint32_t>((std::min)(permille, std::uint64_t(1000u))));
			}
		}
	}
}

void LostArk::Server::CGameRoom::Merge_MvpLedger(SERVER_WORLD_ENTITY& boss)
{
	for (const SERVER_MVP_LEDGER_ROW& source : boss.MvpLedger)
	{
		SERVER_MVP_LEDGER_ROW& row = Find_Or_Add_MvpLedgerRow(m_GateMvpLedger, source.iPlayerId);
		row.iDamage += source.iDamage;
		row.iStagger += source.iStagger;
		row.iCounterCount += source.iCounterCount;
		row.iPartDamage += source.iPartDamage;
		row.iFinishingBlows += source.iFinishingBlows;
		row.iDamagingHitsTaken += source.iDamagingHitsTaken;
		row.iKnockdowns += source.iKnockdowns;
		row.iLowestHpPermille = (std::min)(row.iLowestHpPermille, source.iLowestHpPermille);
		if (0u != source.iMinCounterGapTicks &&
			(0u == row.iMinCounterGapTicks || source.iMinCounterGapTicks < row.iMinCounterGapTicks))
			row.iMinCounterGapTicks = source.iMinCounterGapTicks;
		/* Two bosses fought at once share one clock: keep the longer one. */
		if (source.iFightTicks > row.iFightTicks)
		{
			row.iFightTicks = source.iFightTicks;
			row.iAliveTicks = source.iAliveTicks;
		}
	}
	boss.MvpLedger.clear();
}

void LostArk::Server::CGameRoom::Broadcast_RaidMvpResult(const std::uint8_t iGate)
{
	using namespace LostArk::Shared;
	S2C_RAID_MVP_RESULT message{};
	message.eWorldId = m_eWorldId;
	message.iGate = iGate;
	/* Only players still in the room have an identity to show. */
	for (const SERVER_MVP_LEDGER_ROW& row : m_GateMvpLedger)
	{
		const auto playerIter = m_Players.find(row.iPlayerId);
		if (playerIter == m_Players.end() || message.Participants.size() >= MAX_RAID_MVP_PARTICIPANTS)
			continue;
		RAID_MVP_PARTICIPANT participant{};
		participant.iPlayerId = row.iPlayerId;
		participant.iNetEntityId = playerIter->second.iNetEntityId;
		participant.eCharacterClass = playerIter->second.eCharacterClass;
		participant.strNickname = playerIter->second.strNickName;
		participant.iDamage = row.iDamage;
		participant.iStagger = row.iStagger;
		participant.iCounterCount = row.iCounterCount;
		participant.iFightTicks = row.iFightTicks;
		participant.iAliveTicks = (std::min)(row.iAliveTicks, row.iFightTicks);
		participant.iPartDamage = row.iPartDamage;
		participant.iFinishingBlows = row.iFinishingBlows;
		participant.iDamagingHitsTaken = row.iDamagingHitsTaken;
		participant.iLowestHpPermille = static_cast<std::uint16_t>((std::min)(row.iLowestHpPermille, 1000u));
		participant.bAliveAtClear = 0u != playerIter->second.iCurrentHp &&
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD != playerIter->second.eAction;
		participant.iMinCounterGapMs = static_cast<std::uint32_t>(
			static_cast<std::uint64_t>(row.iMinCounterGapTicks) * 1000u / GameRoomDetail::SERVER_TICK_HZ);
		participant.iKnockdowns = row.iKnockdowns;
		participant.iFightMs = static_cast<std::uint32_t>(
			static_cast<std::uint64_t>(row.iFightTicks) * 1000u / GameRoomDetail::SERVER_TICK_HZ);
		message.Participants.push_back(std::move(participant));
	}
	m_GateMvpLedger.clear();
	if (message.Participants.empty())
		return;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	for (const auto& [playerId, player] : m_Players)
	{
		const std::shared_ptr<CClientSession> session = Find_Session(player.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(PACKET_TYPE::S2C_RAID_MVP_RESULT, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}

void LostArk::Server::CGameRoom::Handle_GateProgressPropose(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_GATE_PROGRESS_PROPOSE& request)
{
	using namespace LostArk::Shared;
	if (0u == Gate_Count() || request.eWorldId != m_eWorldId)
		return;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID proposerId = sessionIter->second;
	const auto playerIter = m_Players.find(proposerId);
	if (playerIter == m_Players.end() || playerIter->second.iSessionId != sessionId)
		return;
	/* One vote at a time. ADVANCE needs a raised gate that is cleared and a next gate to
	   exist; RESTART re-raises the current gate whether it fell or not, and with no gate
	   raised yet (fresh room) it raises the first one. */
	const std::uint8_t iCurrent = m_GateProgress.iCurrentGate;
	if (0u != m_GateProgress.iProposalId || request.eKind >= GATE_PROGRESS_KIND::END)
		return;
	if (GATE_PROGRESS_KIND::ADVANCE == request.eKind &&
		(0u == iCurrent || iCurrent >= Gate_Count() || 0u == (m_GateProgress.iClearedMask & (1u << (iCurrent - 1u)))))
		return;

    const bool raid = Is_KoukuRaidRunning();
    if (raid && m_KoukuRaid.State.strGateId == "GATE3" &&
        m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && m_KoukuRaid.State.iEndTick) return;
    if (request.eKind == GATE_PROGRESS_KIND::ENTER_GATE3 &&
        (raid ? (m_KoukuRaid.State.strGateId != "GATE3" ||
            m_KoukuRaid.State.ePhase != KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY) :
            !Is_KoukuGate3EntryTerrace(playerIter->second.fPositionX,
                playerIter->second.fPositionY, playerIter->second.fPositionZ))) return;
    if (raid && (proposerId != m_KoukuRaid.State.iOwnerPlayerId ||
        m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING ||
        m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC ||
        (request.eKind == GATE_PROGRESS_KIND::ADVANCE && m_KoukuRaid.State.ePhase != KOUKUSAYDON_RAID_PHASE::WAIT_GATE))) return;
	std::vector<PLAYER_ID> voters = raid ? m_KoukuRaid.PlayerIds : std::vector<PLAYER_ID>{proposerId};
	const auto partyIdIter = m_PartyIdByPlayerId.find(proposerId);
	if (!raid && partyIdIter != m_PartyIdByPlayerId.end())
	{
		const auto membersIter = m_PartyMembersByPartyId.find(partyIdIter->second);
		if (membersIter != m_PartyMembersByPartyId.end() && membersIter->second.size() > 1u)
		{
			/* The leader (members.front()) proposes; the Client only offers the button to the
			   leader, so a non-leader request is silently dropped like the raid entry vote. */
			if (membersIter->second.front() != proposerId)
				return;
			voters = membersIter->second;
		}
	}
    if (raid && std::any_of(voters.begin(), voters.end(), [&](const auto id) { return !m_Players.contains(id); })) return;
	/* Only voters still in this room count. */
	voters.erase(std::remove_if(voters.begin(), voters.end(),
		[this](const PLAYER_ID id) { return !m_Players.contains(id); }), voters.end());
	if (voters.empty())
		return;

	m_GateProgress.iProposalId = m_iNextGateProposalId++;
	if (0u == m_iNextGateProposalId)
		m_iNextGateProposalId = 1u;
	m_GateProgress.iRaidEpoch = raid ? m_KoukuRaid.State.iRunEpoch : 0u;
	m_GateProgress.eKind = request.eKind;
	m_GateProgress.iRequestSequence = request.iRequestSequence;
	m_GateProgress.iProposerId = proposerId;
	m_GateProgress.Voters = voters;
	m_GateProgress.Accepted = { proposerId };
	m_GateProgress.iDeadlineTick = m_iServerTick + GATE_VOTE_TIMEOUT_TICKS;
	if (m_GateProgress.Accepted.size() >= m_GateProgress.Voters.size())
	{
		/* Solo: the proposer's own answer is the whole vote. */
		Close_GateProgressVote(GATE_PROGRESS_VOTE_RESULT::ALL_ACCEPTED);
		return;
	}
	Broadcast_GateProgressState(false, GATE_PROGRESS_VOTE_RESULT::NONE);
}

void LostArk::Server::CGameRoom::Handle_GateProgressRespond(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_GATE_PROGRESS_RESPOND& request)
{
	using namespace LostArk::Shared;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID responderId = sessionIter->second;
	if (0u == m_GateProgress.iProposalId || request.iProposalId != m_GateProgress.iProposalId)
		return;
	if (std::find(m_GateProgress.Voters.begin(), m_GateProgress.Voters.end(), responderId) ==
		m_GateProgress.Voters.end())
		return;
	if (!request.bAccepted)
	{
		Close_GateProgressVote(GATE_PROGRESS_VOTE_RESULT::DECLINED);
		return;
	}
	if (std::find(m_GateProgress.Accepted.begin(), m_GateProgress.Accepted.end(), responderId) ==
		m_GateProgress.Accepted.end())
		m_GateProgress.Accepted.push_back(responderId);
	if (m_GateProgress.Accepted.size() >= m_GateProgress.Voters.size())
		Close_GateProgressVote(GATE_PROGRESS_VOTE_RESULT::ALL_ACCEPTED);
	else
		Broadcast_GateProgressState(false, GATE_PROGRESS_VOTE_RESULT::NONE);
}

void LostArk::Server::CGameRoom::Close_GateProgressVote(
	const LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result)
{
	using namespace LostArk::Shared;
	GATE_PROGRESS_VOTE_RESULT finalResult = result;
	if (GATE_PROGRESS_VOTE_RESULT::ALL_ACCEPTED == result)
	{
		bool entered = false;
		if (m_GateProgress.eKind == GATE_PROGRESS_KIND::ENTER_GATE3)
		{
			if (m_GateProgress.iRaidEpoch)
			{
				entered = Is_KoukuRaidRunning() && m_GateProgress.iRaidEpoch == m_KoukuRaid.State.iRunEpoch &&
					m_GateProgress.iProposerId == m_KoukuRaid.State.iOwnerPlayerId &&
					m_KoukuRaid.State.strGateId == "GATE3" &&
					m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY && Enter_KoukuRaidCombat(3u);
			}
			else if (!Is_KoukuRaidRunning())
			{
				// Consent belongs to this exact party. Moving off the deck or changing
				// leadership/membership during the vote must not move the old roster.
				std::vector<PLAYER_ID> currentVoters{m_GateProgress.iProposerId};
				const auto party = m_PartyIdByPlayerId.find(m_GateProgress.iProposerId);
				if (party != m_PartyIdByPlayerId.end())
				{
					const auto members = m_PartyMembersByPartyId.find(party->second);
					if (members != m_PartyMembersByPartyId.end()) currentVoters = members->second;
				}
				const auto proposer = m_Players.find(m_GateProgress.iProposerId);
				entered = !currentVoters.empty() && currentVoters.front() == m_GateProgress.iProposerId &&
					currentVoters == m_GateProgress.Voters && proposer != m_Players.end() &&
					Is_KoukuGate3EntryTerrace(proposer->second.fPositionX, proposer->second.fPositionY, proposer->second.fPositionZ) &&
					Advance_Gate(3u, &m_GateProgress.Voters);
			}
		}
		else
		{
			const std::uint8_t iTarget = GATE_PROGRESS_KIND::RESTART == m_GateProgress.eKind ?
				(std::max<std::uint8_t>)(m_GateProgress.iCurrentGate, 1u) :
				static_cast<std::uint8_t>(m_GateProgress.iCurrentGate + 1u);
			entered = (!m_GateProgress.iRaidEpoch || (Is_KoukuRaidRunning() &&
				m_GateProgress.iRaidEpoch == m_KoukuRaid.State.iRunEpoch)) && Advance_Gate(iTarget);
		}
		if (!entered) finalResult = GATE_PROGRESS_VOTE_RESULT::CANCELLED;
	}
	/* The closing message still names the proposal, then the vote is gone. */
	Broadcast_GateProgressState(true, finalResult);
	m_GateProgress.iProposalId = 0u;
	m_GateProgress.iRaidEpoch = 0u;
	m_GateProgress.eKind = GATE_PROGRESS_KIND::ADVANCE;
	m_GateProgress.iRequestSequence = 0u;
	m_GateProgress.iProposerId = INVALID_PLAYER_ID;
	m_GateProgress.Voters.clear();
	m_GateProgress.Accepted.clear();
	m_GateProgress.iDeadlineTick = 0u;
}

void LostArk::Server::CGameRoom::Expire_GateProgressVote()
{
	if (0u != m_GateProgress.iProposalId && m_iServerTick >= m_GateProgress.iDeadlineTick)
		Close_GateProgressVote(LostArk::Shared::GATE_PROGRESS_VOTE_RESULT::TIMEOUT);
}

bool LostArk::Server::CGameRoom::Spawn_GatePlacement(const std::string& placementId)
{
	return Spawn_GatePlacement(placementId, nullptr);
}

bool LostArk::Server::CGameRoom::Spawn_GatePlacement(const std::string& placementId, SERVER_WORLD_ENTITY* prepared)
{
	using namespace LostArk::Shared;
	const WORLD_BOOTSTRAP_PLACEMENT* placement = Find_Placement(placementId);
	if (nullptr == placement || !CKoukuSaydonBrain::Is_ArenaBossPlacement(m_eWorldId, *placement))
		return false;
	const bool alreadyUp = std::any_of(m_WorldEntities.begin(), m_WorldEntities.end(),
		[&placementId](const SERVER_WORLD_ENTITY& entity) { return entity.strPlacementId == placementId; });
	if (alreadyUp)
		return true;
	if (m_iNextNetEntityId == INVALID_NET_ENTITY_ID)
		return false;
	SERVER_WORLD_ENTITY staged{};
	if (prepared) staged = std::move(*prepared);
	else if (!Build_WorldEntity(*placement, m_iNextNetEntityId, staged))
		return false;
	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	for (auto& [playerId, player] : m_Players)
		Apply_KoukuGateEntryCard(player, m_WorldEntities.back());
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	return true;
}

bool LostArk::Server::CGameRoom::Enter_KoukuRaidCombat(const std::uint8_t gateIndex)
{
    using namespace LostArk::Shared;
    auto& run = m_KoukuRaid;
    const bool gateThreeEntry = gateIndex == 3u && run.State.strGateId == "GATE3" &&
        (run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY ||
            (run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.bGate3CombatEntered));
    const bool bingoEntry = gateIndex == 4u &&
        ((run.State.strGateId == "GATE3" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && !run.State.iEndTick) ||
            (run.State.strGateId == "BINGO" && (run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING ||
                run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE || run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT ||
                (run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && !run.bClearCinematic &&
                    GameRoomDetail::Has_ReachedServerTick(m_iServerTick, run.State.iEndTick)))));
    if ((!gateThreeEntry && !bingoEntry) || run.PlayerIds.empty() || !run.pCatalog ||
        !run.pCatalog->Find_KoukuRaidGate(bingoEntry ? "BINGO" : "GATE3")) return false;
    const auto& gate = KOUKU_GATES[gateIndex - 1u];
    C2S_DEBUG_TELEPORT_TO_POSITION move; move.eWorldId = m_eWorldId;
    move.fPositionX = gate.fX; move.fPositionY = gate.fY; move.fPositionZ = gate.fZ;
    std::vector<std::pair<PLAYER_ID, SERVER_NAV_POINT>> destinations;
    std::vector<std::pair<PLAYER_ID, SERVER_PLAYER>> previousPlayers;
    for (const auto id : run.PlayerIds)
    {
        const auto player = m_Players.find(id);
        if (player == m_Players.end()) return false;
        const auto session = m_PlayerIdBySessionId.find(player->second.iSessionId);
        if (session == m_PlayerIdBySessionId.end() || session->second != id) return false;
        if (!m_GameplayCatalog.Find_Player(player->second.eCharacterClass)) return false;
        // The raid-owned encore handoff or unanimous Bingo restart revives participants. Validate that future
        // state without touching the live player or cancelling the existing encounter.
        auto candidate = player->second;
        if (bingoEntry)
        {
            candidate.iCurrentHp = candidate.iMaximumHp;
            candidate.eAction = PLAYER_ACTION_STATE::NONE;
            candidate.Clear_PatternBindStatus(); candidate.Clear_Attachment();
        }
        SERVER_NAV_POINT ground;
        // The previous primary actor is removed by the approved Bingo reset.
        if (Validate_DebugTeleportDestination(candidate, move, ground,
            bingoEntry ? run.iPrimaryBossId : INVALID_NET_ENTITY_ID) != DEBUG_TELEPORT_RESULT::ACCEPTED)
        { m_strStatus = "Raid combat entry destination is unavailable."; return false; }
        destinations.emplace_back(id, ground);
        previousPlayers.emplace_back(id, player->second);
    }
    if (!Start_KoukuRaidCombat(m_iServerTick, bingoEntry ? "BINGO" : "GATE3")) return false;
    const auto previousState = run.State;
    const auto previousGateProgress = m_GateProgress;
    if (bingoEntry)
    {
        // Every session, profile and destination has passed before the old flow changes.
        if (m_KoukuSaydonPatternAudition.iRoomAuditionEpoch)
            Clear_KoukuSaydonPatternAudition(false, "Bingo entry approved");
        Stop_KoukuBingoDuration(true); Reset_CardMaze();
        if (!Despawn_KoukuSaydonArenaDebugEntities(true)) return false;
        run.State.strGateId = "BINGO";
        run.State.strFlowEntryId.clear(); run.State.iFlowEntryIndex = 0u;
        run.bClearCinematic = false; run.CompletedArrivals.clear();
        run.iPrimaryBossId = INVALID_NET_ENTITY_ID; run.bEntryRunning = false; run.iNextEntryTick = 0u;
        run.iAuditionEpoch = 0u; run.iAuditionRequestSequence = 0u;
        m_iNextMarioEntryStage = 1u;
    }
    for (const auto& [id, ground] : destinations)
    {
        auto& player = m_Players.at(id);
        if (bingoEntry)
        {
            const auto* profile = m_GameplayCatalog.Find_Player(player.eCharacterClass);
            player.Clear_KoukuInteractionState(); player.Clear_KoukuAssignedCard();
            player.iCurrentHp = player.iMaximumHp; player.iCurrentResource = player.iMaximumResource;
            player.iResourceAccumulator = 0u; CPlayerSkillSystem::Reset_Gauges(player, m_GameplayCatalog);
            player.iCurrentMadness = 0u; player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
            player.eStance = profile->eDefaultStance; player.CooldownEndTickBySkillId.clear();
        }
        Reset_PlayerForDebugTeleport(player);
        player.fPositionX = ground.x; player.fPositionY = ground.y; player.fPositionZ = ground.z;
    }
    if (!Start_KoukuRaidCombat(m_iServerTick))
    {
        // No snapshot can observe a partial formation within this room tick.
        if (m_KoukuSaydonPatternAudition.iRoomAuditionEpoch)
            Clear_KoukuSaydonPatternAudition(false, "Raid combat entry failed");
        (void)Despawn_KoukuSaydonArenaDebugEntities(true);
        for (auto& [id, previous] : previousPlayers) m_Players.at(id) = std::move(previous);
        run.State = previousState; run.iPrimaryBossId = INVALID_NET_ENTITY_ID; run.bEntryRunning = false;
        m_GateProgress = previousGateProgress;
        Broadcast_KoukuRaidState(); Broadcast_GateProgressState(false, GATE_PROGRESS_VOTE_RESULT::NONE);
        return false;
    }
    if (gateIndex == 3u) run.bGate3CombatEntered = true;
    return true;
}

bool LostArk::Server::CGameRoom::Advance_Gate(const std::uint8_t nextGate)
{
	return Advance_Gate(nextGate, nullptr);
}

bool LostArk::Server::CGameRoom::Advance_Gate(const std::uint8_t nextGate,
    const std::vector<LostArk::Shared::PLAYER_ID>* participants)
{
	using namespace LostArk::Shared;
	if (0u == nextGate || nextGate > Gate_Count())
		return false;
    if (Is_KoukuRaidRunning())
        return Advance_KoukuRaidGate(nextGate, m_GateProgress.eKind == GATE_PROGRESS_KIND::RESTART);
	const KOUKU_GATE& gate = KOUKU_GATES[nextGate - 1u];
	C2S_DEBUG_TELEPORT_TO_POSITION move{};
	move.iRequestSequence = 1u; move.eWorldId = m_eWorldId;
	move.fPositionX = gate.fX; move.fPositionY = gate.fY; move.fPositionZ = gate.fZ;
	std::vector<std::pair<PLAYER_ID, SERVER_NAV_POINT>> destinations;
	std::vector<SERVER_WORLD_ENTITY> prepared;
	if (participants)
	{
		if (participants->empty()) return false;
		for (const auto id : *participants)
		{
			const auto player = m_Players.find(id);
			if (player == m_Players.end()) return false;
			const auto session = m_PlayerIdBySessionId.find(player->second.iSessionId);
			SERVER_NAV_POINT ground;
			if (session == m_PlayerIdBySessionId.end() || session->second != id ||
				!m_GameplayCatalog.Find_Player(player->second.eCharacterClass) ||
				Validate_DebugTeleportDestination(player->second, move, ground) != DEBUG_TELEPORT_RESULT::ACCEPTED)
				return false;
			destinations.emplace_back(id, ground);
		}
		auto nextId = m_iNextNetEntityId;
		for (const char* id : gate.Placements)
		{
			if (!id) continue;
			const auto* placement = Find_Placement(id); SERVER_WORLD_ENTITY staged;
			if (!nextId || !placement || !CKoukuSaydonBrain::Is_ArenaBossPlacement(m_eWorldId, *placement) ||
				!Build_WorldEntity(*placement, nextId++, staged)) return false;
			prepared.push_back(std::move(staged));
		}
		if (!Despawn_KoukuSaydonArenaDebugEntities(false, true)) return false;
	}

	/* Same order as the Debug gate button: clear the arena, raise the placements, move
	   the players -- here for every player in the room, on the validated navigation path. */
	Reset_CardMaze();
	if (!Despawn_KoukuSaydonArenaDebugEntities())
	{
		Mark_RuntimeFailure("gate-progress.despawn");
		return false;
	}
	for (auto& [playerId, player] : m_Players)
		if (!participants || std::find(participants->begin(), participants->end(), playerId) != participants->end())
			player.Clear_KoukuAssignedCard();
	std::size_t preparedIndex = 0u;
	for (const char* pPlacement : gate.Placements)
	{
		if (nullptr != pPlacement && !Spawn_GatePlacement(pPlacement, participants ? &prepared[preparedIndex++] : nullptr))
			return false;
	}
	for (auto& [playerId, player] : m_Players)
	{
		SERVER_NAV_POINT ground{};
		if (participants)
		{
			const auto destination = std::find_if(destinations.begin(), destinations.end(),
				[playerId](const auto& row) { return row.first == playerId; });
			if (destination == destinations.end()) continue;
			ground = destination->second;
		}
		/* A dead or bound player stays where it is; the living party moves. */
		else if (DEBUG_TELEPORT_RESULT::ACCEPTED != Validate_DebugTeleportDestination(player, move, ground))
			continue;
		Reset_PlayerForDebugTeleport(player);
		player.fPositionX = ground.x;
		player.fPositionY = ground.y;
		player.fPositionZ = ground.z;
		Update_MarioControlState(player);
	}
	/* The gate is up again: fought from the start, whether it is the next one or a restart. */
	m_GateProgress.iCurrentGate = nextGate;
	m_GateProgress.iClearedMask &= static_cast<std::uint8_t>(~(1u << (nextGate - 1u)));
	return true;
}

bool LostArk::Server::CGameRoom::Build_GateProgressState(
	LostArk::Shared::S2C_GATE_PROGRESS_STATE& message, const bool bClosed,
	const LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result) const
{
	using namespace LostArk::Shared;
	if (0u == Gate_Count())
		return false;
	message = {};
	message.eWorldId = m_eWorldId;
	message.iGateCount = Gate_Count();
	message.iCurrentGate = m_GateProgress.iCurrentGate;
	message.iClearedMask = m_GateProgress.iClearedMask;
	message.iProposalId = m_GateProgress.iProposalId;
	message.eKind = m_GateProgress.eKind;
	if (const auto proposer = m_Players.find(m_GateProgress.iProposerId); proposer != m_Players.end())
		message.iProposerNetEntityId = proposer->second.iNetEntityId;
	message.iAccepted = static_cast<std::uint8_t>((std::min)(m_GateProgress.Accepted.size(), std::size_t(255)));
	message.iTotal = static_cast<std::uint8_t>((std::min)(m_GateProgress.Voters.size(), std::size_t(255)));
	message.bClosed = bClosed;
	message.eResult = bClosed ? result : GATE_PROGRESS_VOTE_RESULT::NONE;
	return true;
}

void LostArk::Server::CGameRoom::Broadcast_GateProgressState(
	const bool bClosed, const LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result)
{
	using namespace LostArk::Shared;
	S2C_GATE_PROGRESS_STATE message{};
	if (!Build_GateProgressState(message, bClosed, result))
		return;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	for (const auto& [playerId, player] : m_Players)
	{
		const std::shared_ptr<CClientSession> session = Find_Session(player.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(PACKET_TYPE::S2C_GATE_PROGRESS_STATE, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}
