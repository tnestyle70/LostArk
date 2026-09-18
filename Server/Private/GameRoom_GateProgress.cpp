#include "GameRoom.h"

#include "ClientSession.h"
#include "KoukuSaydonBrain.h"
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
	m_GateProgress.iCurrentGate = iRaised;
	m_GateProgress.iClearedMask &= static_cast<std::uint8_t>(~(1u << iGate));
	Broadcast_GateProgressState(false, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT::NONE);
}

void LostArk::Server::CGameRoom::Notify_GateBossDeath(const SERVER_WORLD_ENTITY& deadBoss)
{
    // A pinned raid defines its primary boss; Gate 2's supporting actor cannot delay or trigger its clear.
    if (Is_KoukuRaidRunning() && m_KoukuRaid.State.ePhase != LostArk::Shared::KOUKUSAYDON_RAID_PHASE::PREPARING) return;
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
		const std::uint8_t iTarget = GATE_PROGRESS_KIND::RESTART == m_GateProgress.eKind ?
			(std::max<std::uint8_t>)(m_GateProgress.iCurrentGate, 1u) :
			static_cast<std::uint8_t>(m_GateProgress.iCurrentGate + 1u);
		if ((m_GateProgress.iRaidEpoch && (!Is_KoukuRaidRunning() || m_GateProgress.iRaidEpoch != m_KoukuRaid.State.iRunEpoch)) || !Advance_Gate(iTarget))
			finalResult = GATE_PROGRESS_VOTE_RESULT::CANCELLED;
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
	if (!Build_WorldEntity(*placement, m_iNextNetEntityId, staged))
		return false;
	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	for (auto& [playerId, player] : m_Players)
		Apply_KoukuGateEntryCard(player, m_WorldEntities.back());
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	return true;
}

bool LostArk::Server::CGameRoom::Advance_Gate(const std::uint8_t nextGate)
{
	using namespace LostArk::Shared;
	if (0u == nextGate || nextGate > Gate_Count())
		return false;
    if (Is_KoukuRaidRunning())
        return Advance_KoukuRaidGate(nextGate, m_GateProgress.eKind == GATE_PROGRESS_KIND::RESTART);
	const KOUKU_GATE& gate = KOUKU_GATES[nextGate - 1u];

	/* Same order as the Debug gate button: clear the arena, raise the placements, move
	   the players -- here for every player in the room, on the validated navigation path. */
	Reset_CardMaze();
	if (!Despawn_KoukuSaydonArenaDebugEntities())
	{
		Mark_RuntimeFailure("gate-progress.despawn");
		return false;
	}
	for (auto& [playerId, player] : m_Players)
		player.Clear_KoukuAssignedCard();
	for (const char* pPlacement : gate.Placements)
	{
		if (nullptr != pPlacement && !Spawn_GatePlacement(pPlacement))
			return false;
	}
	C2S_DEBUG_TELEPORT_TO_POSITION move{};
	move.iRequestSequence = 1u;
	move.eWorldId = m_eWorldId;
	move.fPositionX = gate.fX;
	move.fPositionY = gate.fY;
	move.fPositionZ = gate.fZ;
	for (auto& [playerId, player] : m_Players)
	{
		SERVER_NAV_POINT ground{};
		/* A dead or bound player stays where it is; the living party moves. */
		if (DEBUG_TELEPORT_RESULT::ACCEPTED != Validate_DebugTeleportDestination(player, move, ground))
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
