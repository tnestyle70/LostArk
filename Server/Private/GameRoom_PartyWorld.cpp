#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

void LostArk::Server::CGameRoom::Handle_ReturnToBern(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RETURN_TO_BERN& request)
{
	using namespace LostArk::Shared;
	// Same guide NPC placement Handle_ConfirmNpcEntry's own
	// VALTAN_ENTRY_GUIDE_NPCS[0] leads out from -- either would do (both lead to
	// the same target), this just needs to be a real BERN placement id.
	constexpr const char* BERN_RETURN_PLACEMENT_ID = "npc.bern.beda.guide";

	if (WORLD_ID::VALTAN_ARENA != m_eWorldId)
		return;
	if (!m_bValtanRaidCleared && !Is_RaidClearTestModeEnabled())
		return;

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;
	const SERVER_PLAYER& player = playerIter->second;
	if (INVALID_SESSION_ID == player.iSessionId ||
		CHARACTER_CLASS_ID::END == player.eCharacterClass ||
		player.strNickName.empty())
	{
		return;
	}

	// Solo only -- unlike Handle_ConfirmNpcEntry, returning is never batched
	// across a party. Each player presses their own button independently.
	const bool isAlreadyStaged = std::any_of(
		m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
		[sessionId](const SERVER_WORLD_TRANSFER_REQUEST& pending)
		{
			return pending.iSessionId == sessionId;
		});
	if (isAlreadyStaged)
		return;

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	transfer.iSessionId = player.iSessionId;
	transfer.eTargetWorldId = WORLD_ID::BERN;
	transfer.eCharacterClass = player.eCharacterClass;
	transfer.strNickName = player.strNickName;
	transfer.iPartyRequestSequence = request.iRequestSequence;
	transfer.strSpawnPlacementOverrideId = BERN_RETURN_PLACEMENT_ID;
	// Carries Valtan clear rewards (and anything else still held) across the
	// trip -- without this, Stage_PlayerEntry's default fresh-entry grant would
	// silently reset the player back to just 3 starting potions.
	transfer.CarriedInventory = player.Inventory;
	m_PendingWorldTransfers.push_back(std::move(transfer));
}

void LostArk::Server::CGameRoom::Handle_PartyInvite(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_PARTY_INVITE& request)
{
	using namespace LostArk::Shared;

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID inviterId = sessionIter->second;
	const auto inviterIter = m_Players.find(inviterId);
	if (inviterIter == m_Players.end())
		return;
	const SERVER_PLAYER& inviter = inviterIter->second;

	const auto targetPlayerIdIter =
		m_PlayerIdByEntityId.find(request.iTargetNetEntityId);
	if (targetPlayerIdIter == m_PlayerIdByEntityId.end() ||
		targetPlayerIdIter->second == inviterId)
	{
		return;
	}
	const PLAYER_ID targetId = targetPlayerIdIter->second;
	const auto targetIter = m_Players.find(targetId);
	if (targetIter == m_Players.end())
		return;
	const SERVER_PLAYER& target = targetIter->second;

	const auto inviterPartyIter = m_PartyIdByPlayerId.find(inviterId);
	const std::uint32_t inviterPartyId = inviterPartyIter != m_PartyIdByPlayerId.end() ?
		inviterPartyIter->second : 0u;
	const auto targetPartyIter = m_PartyIdByPlayerId.find(targetId);
	if (targetPartyIter != m_PartyIdByPlayerId.end())
	{
		// Already partied together, or target belongs to a different party --
		// merging two existing parties is not supported yet either way.
		return;
	}
	if (0u != inviterPartyId)
	{
		const auto membersIter = m_PartyMembersByPartyId.find(inviterPartyId);
		if (membersIter != m_PartyMembersByPartyId.end() &&
			membersIter->second.size() >= MAX_PARTY_MEMBERS)
		{
			return;
		}
	}

	// A new invite silently replaces whatever this target's last unanswered
	// invite was -- only one can ever be outstanding per target.
	m_PendingPartyInviteByTargetPlayerId[targetId] = inviterId;

	const std::shared_ptr<CClientSession> targetSession =
		Find_Session(target.iSessionId);
	if (nullptr == targetSession)
		return;
	S2C_PARTY_INVITE_RECEIVED message{};
	message.iFromNetEntityId = inviter.iNetEntityId;
	message.strFromNickname = inviter.strNickName;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	if (!targetSession->Send_Frame(
			PACKET_TYPE::S2C_PARTY_INVITE_RECEIVED, writer.Get_Buffer()))
	{
		targetSession->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_PartyInviteRespond(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_PARTY_INVITE_RESPOND& request)
{
	using namespace LostArk::Shared;

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID responderId = sessionIter->second;

	const auto pendingIter =
		m_PendingPartyInviteByTargetPlayerId.find(responderId);
	if (pendingIter == m_PendingPartyInviteByTargetPlayerId.end())
		return;
	const PLAYER_ID inviterId = pendingIter->second;
	const auto inviterIter = m_Players.find(inviterId);
	if (inviterIter == m_Players.end())
		return;
	if (inviterIter->second.iNetEntityId != request.iFromNetEntityId)
		return;
	// A response to a replaced invite must not consume the current invite,
	// regardless of whether that stale response accepts or declines.
	m_PendingPartyInviteByTargetPlayerId.erase(pendingIter);
	if (!request.bAccepted)
		return;
	if (m_Players.find(responderId) == m_Players.end())
		return;
	// Re-check both invariants Handle_PartyInvite validated -- state may have
	// changed while this invite was outstanding.
	if (m_PartyIdByPlayerId.find(responderId) != m_PartyIdByPlayerId.end())
		return;

	auto inviterPartyIter = m_PartyIdByPlayerId.find(inviterId);
	std::uint32_t partyId = inviterPartyIter != m_PartyIdByPlayerId.end() ?
		inviterPartyIter->second : 0u;
	if (0u == partyId)
	{
		partyId = m_iNextPartyId++;
		m_PartyMembersByPartyId[partyId] = { inviterId };
		m_PartyIdByPlayerId[inviterId] = partyId;
	}
	else if (m_PartyMembersByPartyId[partyId].size() >= MAX_PARTY_MEMBERS)
	{
		return;
	}
	m_PartyMembersByPartyId[partyId].push_back(responderId);
	m_PartyIdByPlayerId[responderId] = partyId;

	Broadcast_PartyRoster(partyId);
}

void LostArk::Server::CGameRoom::Broadcast_PartyRoster(
	const std::uint32_t partyId)
{
	using namespace LostArk::Shared;

	const auto membersIter = m_PartyMembersByPartyId.find(partyId);
	if (membersIter == m_PartyMembersByPartyId.end())
		return;

	S2C_PARTY_ROSTER message{};
	for (const PLAYER_ID memberId : membersIter->second)
	{
		const auto playerIter = m_Players.find(memberId);
		if (playerIter == m_Players.end())
			continue;
		PARTY_ROSTER_MEMBER member{};
		member.iNetEntityId = playerIter->second.iNetEntityId;
		member.strNickname = playerIter->second.strNickName;
		member.eCharacterClass = playerIter->second.eCharacterClass;
		message.Members.push_back(std::move(member));
	}
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	for (const PLAYER_ID memberId : membersIter->second)
	{
		const auto playerIter = m_Players.find(memberId);
		if (playerIter == m_Players.end())
			continue;
		const std::shared_ptr<CClientSession> session =
			Find_Session(playerIter->second.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(
				PACKET_TYPE::S2C_PARTY_ROSTER, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}

std::uint32_t LostArk::Server::CGameRoom::Place_PartyForCutscene(
	const std::string& instanceId)
{
	using namespace LostArk::Shared;
	/* Only the pop-up book cutscene stages the party; every other sequence is
	   presentation the players watch from where they already stand. */
	if ("world.sequence.instance.original_kouku" != instanceId)
		return 0u;

	/* Authored in front of the boss at (-0.29, 1.33, 737.63), three metres
	   apart, each checked walkable against the Area navigation. */
	struct CUTSCENE_STAND_SPOT final
	{
		float fX;
		float fY;
		float fZ;
	};
	static constexpr CUTSCENE_STAND_SPOT SPOTS[] = {
		{ -1.166f, 1.31f, 745.078f },
		{ -3.339f, 1.32f, 743.009f },
		{ -5.512f, 1.30f, 740.941f },
		{ -7.685f, 1.32f, 738.872f },
	};
	static constexpr float BOSS_X = -0.2883f;
	static constexpr float BOSS_Z = 737.6292f;

	std::uint32_t placed = 0u;
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (0u == player.iCurrentHp)
			continue;
		const CUTSCENE_STAND_SPOT& spot =
			SPOTS[placed % (sizeof(SPOTS) / sizeof(SPOTS[0]))];
		/* A cutscene entrance is a placement, not a move: clear whatever the
		   player was doing so no queued path or skill drags them back off. */
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.Clear_SkillTarget();
		player.fActionElapsedSeconds = 0.f;
		player.iComboStage = 0;
		player.hasBufferedComboInput = false;
		player.PendingCommand.Clear();
		player.eAction = PLAYER_ACTION_STATE::NONE;
		/* Party staging replaces the Mario location, not just its camera.
		Restore the previous avatar and retire any in-flight Mario movement. */
		player.Clear_MarioControl();
		player.TriggerMove = {};
		player.fPositionX = spot.fX;
		player.fPositionY = spot.fY;
		player.fPositionZ = spot.fZ;
		/* Face the boss so the party watches the show. */
		player.fYawDegrees = std::atan2(BOSS_X - spot.fX, BOSS_Z - spot.fZ) *
			RADIANS_TO_DEGREES;
		++placed;
	}
	return placed;
}

void LostArk::Server::CGameRoom::Send_InteractPrompt(
	const SERVER_INTERACT_PROMPT_EDGE& edge)
{
	using namespace LostArk::Shared;

	const auto player = m_Players.find(edge.iPlayerId);
	if (m_Players.end() == player)
		return;
	const std::shared_ptr<CClientSession> session =
		Find_Session(player->second.iSessionId);
	if (nullptr == session)
		return;
	S2C_INTERACT_PROMPT message{};
	message.strTriggerPlacementId = edge.strTriggerPlacementId;
	message.bAvailable = edge.bAvailable;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	if (!session->Send_Frame(
		PACKET_TYPE::S2C_INTERACT_PROMPT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_InteractTrigger(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_INTERACT_TRIGGER& request)
{
	using namespace LostArk::Shared;

	const auto playerId = m_PlayerIdBySessionId.find(sessionId);
	if (m_PlayerIdBySessionId.end() == playerId)
		return;
	std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
	const std::uint32_t actionTick = 0u == m_iServerTick ? 1u : m_iServerTick;
	if (!m_ServerTriggerSystem.Activate_Interact(
		playerId->second,
		request.strTriggerPlacementId,
		m_Players,
		actionTick,
		transfers,
		[this](const WORLD_TRIGGER_ACTION_KIND kind,
			const std::string& targetId)
		{
			if (WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE == kind)
			{
				Broadcast_WorldSequencePlay(targetId);
				return true;
			}
			if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP == kind)
				return m_SpawnGroupRuntime.Activate(targetId);
			if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER == kind)
				return Activate_Encounter(targetId);
			return false;
		}))
	{
		return;
	}
	/* A gated box can move worlds like any other, so its transfer is staged
	   through the same pending list the tick uses. */
	for (SERVER_WORLD_TRANSFER_REQUEST& transfer : transfers)
	{
		if (!m_PlayerIdBySessionId.contains(transfer.iSessionId))
			continue;
		const bool alreadyStaged = std::any_of(
			m_PendingWorldTransfers.begin(),
			m_PendingWorldTransfers.end(),
			[staged = transfer.iSessionId](
				const SERVER_WORLD_TRANSFER_REQUEST& pending)
			{
				return pending.iSessionId == staged;
			});
		if (!alreadyStaged)
			m_PendingWorldTransfers.push_back(std::move(transfer));
	}
	/* The offer is spent: withdraw it so the Client stops drawing the key.
	   A repeatable box re-offers on the next entry edge. */
	Send_InteractPrompt({ playerId->second,
		request.strTriggerPlacementId, false });
}

void LostArk::Server::CGameRoom::Handle_DebugWorldPlayback(
	const SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_WORLD_PLAYBACK_RESULT result{ request.iRequestSequence, request.eWorldId,
		request.eOperation, DEBUG_WORLD_PLAYBACK_RESULT::DISABLED, request.strTargetId };
#ifdef _DEBUG
	const auto execute = [&]() -> DEBUG_WORLD_PLAYBACK_RESULT
	{
		using Result = DEBUG_WORLD_PLAYBACK_RESULT;
		using Op = DEBUG_WORLD_PLAYBACK_OPERATION;
		if (request.eWorldId != m_eWorldId ||
			(m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA && m_eWorldId != WORLD_ID::VALTAN_ARENA))
			return Result::WRONG_WORLD;
		const auto playerId = m_PlayerIdBySessionId.find(sessionId);
		if (playerId == m_PlayerIdBySessionId.end()) return Result::INVALID_PLAYER;
		const auto player = m_Players.find(playerId->second);
		if (player == m_Players.end() || !player->second.iCurrentHp) return Result::INVALID_PLAYER;
		auto& last = m_WorldPlaybackRequestSequences[sessionId];
		if (request.iRequestSequence <= last) return Result::STALE_REQUEST;
		last = request.iRequestSequence;
		const bool replay = request.eOperation == Op::REPLAY_TRIGGER || request.eOperation == Op::REPLAY_SEQUENCE;
		const auto play = [&](const std::string& id)
		{
			const auto& ids = m_WorldBootstrap.Get_SequenceInstanceIds();
			if (std::find(ids.begin(), ids.end(), id) == ids.end()) return false;
			Broadcast_WorldSequencePlay(id, 1.f, 0.f, 0.f, 0.f, 0u, {},
				replay ? WORLD_SEQUENCE_OPERATION::REPLAY : WORLD_SEQUENCE_OPERATION::PLAY);
			return true;
		};
		if (request.eOperation == Op::PLAY_TRIGGER || request.eOperation == Op::REPLAY_TRIGGER)
		{
			std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
			const auto verdict = m_ServerTriggerSystem.Debug_Activate(playerId->second, request.strTargetId,
				replay, m_Players, m_iServerTick ? m_iServerTick : 1u, transfers,
				[&](WORLD_TRIGGER_ACTION_KIND kind, const std::string& id)
				{
					if (kind == WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE) return play(id);
					if (kind == WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP) return m_SpawnGroupRuntime.Activate(id);
					if (kind == WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER) return Activate_Encounter(id);
					if (kind == WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE) return Begin_CardMaze(playerId->second);
					return false;
				});
			for (auto& transfer : transfers)
			{
				if (std::none_of(m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
					[&](const auto& pending) { return pending.iSessionId == transfer.iSessionId; }))
					m_PendingWorldTransfers.push_back(std::move(transfer));
			}
			return verdict;
		}
		const auto& ids = m_WorldBootstrap.Get_SequenceInstanceIds();
		if (std::find(ids.begin(), ids.end(), request.strTargetId) == ids.end()) return Result::INVALID_TARGET;
		if (request.eOperation == Op::STOP_SEQUENCE)
			Broadcast_WorldSequencePlay(request.strTargetId, 1.f, 0.f, 0.f, 0.f, 0u, {}, WORLD_SEQUENCE_OPERATION::STOP);
		else if (!play(request.strTargetId)) return Result::INVALID_TARGET;
		return Result::ACCEPTED;
	};
	result.eResult = execute();
#endif
	const auto session = Find_Session(sessionId);
	CPacketWriter writer;
	if (session && Write_Message(writer, result) &&
		!session->Send_Frame(PACKET_TYPE::S2C_DEBUG_WORLD_PLAYBACK_RESULT, writer.Get_Buffer()))
		session->Request_Close();
}

void LostArk::Server::CGameRoom::Broadcast_WorldSequencePlay(
	const std::string& instanceId,
	const float playbackSpeed, const float positionOffsetX,
	const float positionOffsetY, const float positionOffsetZ, const std::uint32_t durationMs,
	const std::string& targetSequenceInstanceId,
	const LostArk::Shared::WORLD_SEQUENCE_OPERATION operation)
{
	using namespace LostArk::Shared;

	S2C_WORLD_SEQUENCE_PLAY message{};
	message.eOperation = operation;
	message.strSequenceInstanceId = instanceId;
	message.strTargetSequenceInstanceId = targetSequenceInstanceId;
	message.iDurationMs = durationMs;
	message.fPlaybackSpeed = playbackSpeed;
	message.fPositionOffsetX = positionOffsetX;
	message.fPositionOffsetY = positionOffsetY;
	message.fPositionOffsetZ = positionOffsetZ;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	// Saved-instance PLAY/REPLAY stage the party; STOP and exact motion do not.
	if ((operation == WORLD_SEQUENCE_OPERATION::PLAY || operation == WORLD_SEQUENCE_OPERATION::REPLAY) &&
		targetSequenceInstanceId.empty())
		(void)Place_PartyForCutscene(instanceId);
	for (const auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session =
			Find_Session(player.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(
				PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}

void LostArk::Server::CGameRoom::Remove_FromParty(
	const LostArk::Shared::PLAYER_ID playerId)
{
	const auto partyIdIter = m_PartyIdByPlayerId.find(playerId);
	if (partyIdIter == m_PartyIdByPlayerId.end())
		return;
	const std::uint32_t partyId = partyIdIter->second;
	m_PartyIdByPlayerId.erase(partyIdIter);

	const auto membersIter = m_PartyMembersByPartyId.find(partyId);
	if (membersIter == m_PartyMembersByPartyId.end())
		return;
	std::vector<LostArk::Shared::PLAYER_ID>& members = membersIter->second;
	members.erase(
		std::remove(members.begin(), members.end(), playerId),
		members.end());
	if (members.empty())
	{
		m_PartyMembersByPartyId.erase(membersIter);
		return;
	}
	Broadcast_PartyRoster(partyId);
}

bool LostArk::Server::CGameRoom::Is_PlayerNearValtanEntryNpc(
	const SERVER_PLAYER& player, const std::string& npcPlacementId) const
{
	using namespace LostArk::Shared;
	// Handle_ConfirmNpcEntry의 VALTAN_ENTRY_GUIDE_NPCS와 같은 placement 집합. 여기서는
	// proximity만 검증하고, target world는 NPC가 아니라 propose의 eTarget이 소유한다.
	static constexpr const char* GUIDE_NPC_PLACEMENT_IDS[] = {
		"npc.bern.beda.guide", "npc.bern.aylara" };
	constexpr float INTERACTION_RADIUS = 3.f;
	const bool isGuide = std::any_of(
		std::begin(GUIDE_NPC_PLACEMENT_IDS), std::end(GUIDE_NPC_PLACEMENT_IDS),
		[&npcPlacementId](const char* id) { return npcPlacementId == id; });
	if (!isGuide)
		return false;
	const auto entityIter = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[&npcPlacementId](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::NPC == entity.eKind &&
				entity.strPlacementId == npcPlacementId;
		});
	if (m_WorldEntities.end() == entityIter)
		return false;
	const float deltaX = player.fPositionX - entityIter->fPositionX;
	const float deltaZ = player.fPositionZ - entityIter->fPositionZ;
	return deltaX * deltaX + deltaZ * deltaZ <=
		INTERACTION_RADIUS * INTERACTION_RADIUS;
}

bool LostArk::Server::CGameRoom::Stage_PartyWorldTransfer(
	const std::vector<LostArk::Shared::PLAYER_ID>& batchMemberIds,
	const LostArk::Shared::WORLD_ID targetWorldId,
	const std::uint32_t requestSequence)
{
	using namespace LostArk::Shared;
	if (batchMemberIds.empty())
		return false;
	const auto leaderIter = m_Players.find(batchMemberIds.front());
	if (leaderIter == m_Players.end())
		return false;
	const SERVER_PLAYER& leader = leaderIter->second;

	const auto isAlreadyStaged = [this](SESSION_ID sid)
	{
		return std::any_of(
			m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
			[sid](const SERVER_WORLD_TRANSFER_REQUEST& pending)
			{
				return pending.iSessionId == sid ||
					std::find(pending.PartyBatchSessionIds.begin(),
						pending.PartyBatchSessionIds.end(), sid) !=
						pending.PartyBatchSessionIds.end();
			});
	};

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	transfer.iSessionId = leader.iSessionId;
	transfer.eTargetWorldId = targetWorldId;
	transfer.eCharacterClass = leader.eCharacterClass;
	transfer.strNickName = leader.strNickName;
	transfer.iPartyRequestSequence = requestSequence;
	for (const PLAYER_ID memberId : batchMemberIds)
	{
		const auto memberIter = m_Players.find(memberId);
		if (memberIter == m_Players.end() ||
			CHARACTER_CLASS_ID::END == memberIter->second.eCharacterClass ||
			memberIter->second.strNickName.empty() ||
			isAlreadyStaged(memberIter->second.iSessionId))
		{
			return false;
		}
		if (batchMemberIds.size() > 1u)
			transfer.PartyBatchSessionIds.push_back(memberIter->second.iSessionId);
	}
	m_PendingWorldTransfers.push_back(std::move(transfer));
	return true;
}

void LostArk::Server::CGameRoom::Broadcast_RaidEntryVote(
	const RAID_ENTRY_PROPOSAL& proposal, const bool bClosed,
	const LostArk::Shared::RAID_ENTRY_VOTE_RESULT result)
{
	using namespace LostArk::Shared;
	S2C_RAID_ENTRY_VOTE message{};
	message.iProposalId = proposal.iProposalId;
	message.iAccepted = static_cast<std::uint8_t>(proposal.Accepted.size());
	message.iTotal = static_cast<std::uint8_t>(proposal.Voters.size());
	message.bClosed = bClosed;
	message.eResult = bClosed ? result : RAID_ENTRY_VOTE_RESULT::END;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	for (const PLAYER_ID memberId : proposal.Voters)
	{
		const auto playerIter = m_Players.find(memberId);
		if (playerIter == m_Players.end())
			continue;
		const std::shared_ptr<CClientSession> session =
			Find_Session(playerIter->second.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(
				PACKET_TYPE::S2C_RAID_ENTRY_VOTE, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}

void LostArk::Server::CGameRoom::Handle_RaidEntryPropose(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RAID_ENTRY_PROPOSE& request)
{
	using namespace LostArk::Shared;
	// 30Hz 기준 30초 미응답이면 tick 루프가 TIMEOUT으로 닫는다.
	constexpr std::uint32_t VOTE_TIMEOUT_TICKS = 30u * 30u;

	if (WORLD_ID::BERN != m_eWorldId || request.eTarget >= RAID_ENTRY_TARGET::END)
		return;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID proposerId = sessionIter->second;
	const auto playerIter = m_Players.find(proposerId);
	if (playerIter == m_Players.end())
		return;
	const SERVER_PLAYER& proposer = playerIter->second;
	if (0u == proposer.iCurrentHp || PLAYER_ACTION_STATE::NONE != proposer.eAction ||
		INVALID_SESSION_ID == proposer.iSessionId ||
		CHARACTER_CLASS_ID::END == proposer.eCharacterClass ||
		proposer.strNickName.empty())
	{
		return;
	}
	if (!Is_PlayerNearValtanEntryNpc(proposer, request.strNpcPlacementId))
		return;

	// 한 플레이어는 동시에 하나의 열린 proposal에만 속한다.
	const bool alreadyInVote = std::any_of(
		m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
		[proposerId](const RAID_ENTRY_PROPOSAL& p)
		{
			return std::find(p.Voters.begin(), p.Voters.end(), proposerId) !=
				p.Voters.end();
		});
	if (alreadyInVote)
		return;

	std::uint32_t partyId = 0u;
	std::vector<PLAYER_ID> voters{ proposerId };
	const auto partyIdIter = m_PartyIdByPlayerId.find(proposerId);
	if (partyIdIter != m_PartyIdByPlayerId.end())
	{
		const auto membersIter = m_PartyMembersByPartyId.find(partyIdIter->second);
		if (membersIter != m_PartyMembersByPartyId.end() &&
			membersIter->second.size() > 1u)
		{
			// 파티 발의는 리더(members.front())만 가능. 비리더는 조용히 거절한다
			// (Client UI가 입장하기를 리더에게만 노출하므로 정상 경로에서 오지 않는다).
			if (membersIter->second.front() != proposerId)
				return;
			partyId = partyIdIter->second;
			voters = membersIter->second;
		}
	}

	RAID_ENTRY_PROPOSAL proposal{};
	proposal.iProposalId = m_iNextRaidEntryProposalId++;
	if (0u == m_iNextRaidEntryProposalId)
		m_iNextRaidEntryProposalId = 1u;
	proposal.iPartyId = partyId;
	proposal.iRequestSequence = request.iRequestSequence;
	proposal.eTarget = request.eTarget;
	proposal.strNpcPlacementId = request.strNpcPlacementId;
	proposal.Voters = voters;
	proposal.iDeadlineTick = m_iServerTick + VOTE_TIMEOUT_TICKS;

	S2C_RAID_ENTRY_PROMPT prompt{};
	prompt.iProposalId = proposal.iProposalId;
	prompt.iProposerNetEntityId = proposer.iNetEntityId;
	prompt.eTarget = proposal.eTarget;
	prompt.strProposerNickname = proposer.strNickName;
	CPacketWriter promptWriter;
	if (!Write_Message(promptWriter, prompt))
		return;
	for (const PLAYER_ID memberId : proposal.Voters)
	{
		const auto memberPlayerIter = m_Players.find(memberId);
		if (memberPlayerIter == m_Players.end())
			continue;
		const std::shared_ptr<CClientSession> session =
			Find_Session(memberPlayerIter->second.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(
				PACKET_TYPE::S2C_RAID_ENTRY_PROMPT, promptWriter.Get_Buffer()))
		{
			session->Request_Close();
		}
	}

	m_RaidEntryProposals.push_back(std::move(proposal));
	Broadcast_RaidEntryVote(
		m_RaidEntryProposals.back(), false, RAID_ENTRY_VOTE_RESULT::END);
}

void LostArk::Server::CGameRoom::Handle_RaidEntryRespond(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RAID_ENTRY_RESPOND& request)
{
	using namespace LostArk::Shared;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID responderId = sessionIter->second;

	const auto proposalIter = std::find_if(
		m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
		[&request](const RAID_ENTRY_PROPOSAL& p)
		{
			return p.iProposalId == request.iProposalId;
		});
	if (proposalIter == m_RaidEntryProposals.end())
		return;
	if (std::find(proposalIter->Voters.begin(), proposalIter->Voters.end(),
			responderId) == proposalIter->Voters.end())
	{
		return;
	}
	if (!request.bAccepted)
	{
		Close_RaidEntryVote(*proposalIter, RAID_ENTRY_VOTE_RESULT::DECLINED);
		return;
	}
	if (std::find(proposalIter->Accepted.begin(), proposalIter->Accepted.end(),
			responderId) == proposalIter->Accepted.end())
	{
		proposalIter->Accepted.push_back(responderId);
	}
	if (proposalIter->Accepted.size() >= proposalIter->Voters.size())
		Close_RaidEntryVote(*proposalIter, RAID_ENTRY_VOTE_RESULT::ALL_ACCEPTED);
	else
		Broadcast_RaidEntryVote(*proposalIter, false, RAID_ENTRY_VOTE_RESULT::END);
}

void LostArk::Server::CGameRoom::Close_RaidEntryVote(
	RAID_ENTRY_PROPOSAL& proposal,
	const LostArk::Shared::RAID_ENTRY_VOTE_RESULT result)
{
	using namespace LostArk::Shared;
	RAID_ENTRY_VOTE_RESULT finalResult = result;
	if (RAID_ENTRY_VOTE_RESULT::ALL_ACCEPTED == result)
	{
		const WORLD_ID targetWorld =
			(RAID_ENTRY_TARGET::KAKULSAYDON == proposal.eTarget)
			? WORLD_ID::KAKULSAYDON_ARENA : WORLD_ID::VALTAN_ARENA;
		// 수락 완료와 실제 stage 사이에 멤버가 unavailable해졌으면 전송하지 않고
		// CANCELLED로 낮춰 전원이 Bern에 남게 한다(부분 이동 금지).
		if (!Stage_PartyWorldTransfer(
				proposal.Voters, targetWorld, proposal.iRequestSequence))
		{
			finalResult = RAID_ENTRY_VOTE_RESULT::CANCELLED;
		}
	}
	Broadcast_RaidEntryVote(proposal, true, finalResult);
	const std::uint32_t closedId = proposal.iProposalId;
	m_RaidEntryProposals.erase(
		std::remove_if(m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
			[closedId](const RAID_ENTRY_PROPOSAL& p)
			{
				return p.iProposalId == closedId;
			}),
		m_RaidEntryProposals.end());
}

void LostArk::Server::CGameRoom::Expire_RaidEntryProposals()
{
	using namespace LostArk::Shared;
	// Close_RaidEntryVote가 벡터를 수정하므로 만료 id를 먼저 모은 뒤 닫는다.
	std::vector<std::uint32_t> expiredIds;
	for (const RAID_ENTRY_PROPOSAL& p : m_RaidEntryProposals)
	{
		if (m_iServerTick >= p.iDeadlineTick)
			expiredIds.push_back(p.iProposalId);
	}
	for (const std::uint32_t id : expiredIds)
	{
		const auto it = std::find_if(
			m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
			[id](const RAID_ENTRY_PROPOSAL& p) { return p.iProposalId == id; });
		if (it != m_RaidEntryProposals.end())
			Close_RaidEntryVote(*it, RAID_ENTRY_VOTE_RESULT::TIMEOUT);
	}
}

void LostArk::Server::CGameRoom::Cancel_RaidEntryProposalsInvolving(
	const LostArk::Shared::PLAYER_ID playerId)
{
	using namespace LostArk::Shared;
	std::vector<std::uint32_t> ids;
	for (const RAID_ENTRY_PROPOSAL& p : m_RaidEntryProposals)
	{
		if (std::find(p.Voters.begin(), p.Voters.end(), playerId) != p.Voters.end())
			ids.push_back(p.iProposalId);
	}
	for (const std::uint32_t id : ids)
	{
		const auto it = std::find_if(
			m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
			[id](const RAID_ENTRY_PROPOSAL& p) { return p.iProposalId == id; });
		if (it != m_RaidEntryProposals.end())
			Close_RaidEntryVote(*it, RAID_ENTRY_VOTE_RESULT::CANCELLED);
	}
}

bool LostArk::Server::CGameRoom::Transfer_PartyTo(
	CGameRoom& target, const std::vector<SESSION_ID>& leaderFirstSessionIds,
	LostArk::Shared::PARTY_TRANSFER_RESULT& outResult, std::string& status)
{
	using namespace LostArk::Shared;
	outResult = PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE;
	const auto reject = [&outResult, &status](const PARTY_TRANSFER_RESULT reason, const char* detail)
	{
		outResult = reason;
		status = detail;
		return false;
	};
	if (!m_isReady || !target.m_isReady || WORLD_ID::BERN != m_eWorldId ||
		WORLD_ID::VALTAN_ARENA != target.m_eWorldId ||
		leaderFirstSessionIds.size() < 2u || leaderFirstSessionIds.size() > MAX_PARTY_MEMBERS)
		return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "invalid party transfer world/batch");
	const auto leader = m_PlayerIdBySessionId.find(leaderFirstSessionIds.front());
	if (leader == m_PlayerIdBySessionId.end())
		return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "party leader is no longer present");
	const auto sourceParty = m_PartyIdByPlayerId.find(leader->second);
	if (sourceParty == m_PartyIdByPlayerId.end())
		return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "source party no longer exists");
	const auto sourceMembers = m_PartyMembersByPartyId.find(sourceParty->second);
	if (sourceMembers == m_PartyMembersByPartyId.end() ||
		sourceMembers->second.size() != leaderFirstSessionIds.size() ||
		sourceMembers->second.front() != leader->second)
		return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "source party changed before transfer");
	if (0u == target.m_iNextPartyId || target.m_PartyMembersByPartyId.contains(target.m_iNextPartyId))
		return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "target party identity is exhausted");

	std::vector<STAGED_PLAYER_ENTRY> entries;
	std::vector<NET_ENTITY_ID> departingEntities;
	entries.reserve(leaderFirstSessionIds.size());
	departingEntities.reserve(leaderFirstSessionIds.size());
	for (std::size_t index = 0; index < leaderFirstSessionIds.size(); ++index)
	{
		const auto member = m_Players.find(sourceMembers->second[index]);
		if (member == m_Players.end() ||
			member->second.iSessionId != leaderFirstSessionIds[index] ||
			std::find(leaderFirstSessionIds.begin(), leaderFirstSessionIds.begin() + index,
				leaderFirstSessionIds[index]) != leaderFirstSessionIds.begin() + index)
			return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "source party member identity changed");
		const auto session = Find_Session(member->second.iSessionId);
		if (nullptr == session || session->Is_Closing())
			return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "party member session is terminal");
		C2S_ENTER_WORLD enter{};
		enter.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		enter.eWorldId = target.m_eWorldId;
		enter.eCharacterClass = member->second.eCharacterClass;
		enter.strNickName = member->second.strNickName;
		STAGED_PLAYER_ENTRY entry{};
		SESSION_DIAGNOSTIC_REASON reason{};
		if (!target.Stage_PlayerEntry(session, enter, entries, entry, reason, status))
		{
			outResult = SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL == reason ?
				PARTY_TRANSFER_RESULT::REJECTED_ROOM_FULL : PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED;
			return false;
		}
		entries.push_back(std::move(entry));
		departingEntities.push_back(member->second.iNetEntityId);
	}
	std::vector<CLIENT_SESSION_RELIABLE_BATCH> outboundBatches;
	S2C_PARTY_ROSTER roster{};
	for (const auto& entry : entries)
		roster.Members.push_back({ entry.Player.iNetEntityId, entry.Player.strNickName,
			entry.Player.eCharacterClass });
	CPacketWriter rosterWriter;
	if (!Write_Message(rosterWriter, roster))
		return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "target party roster failed encoding");
	for (auto& entry : entries)
	{
		if (!target.Build_PlayerEntryFrames(entry, entries, status))
		{
			outResult = PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED;
			return false;
		}
		entry.Frames.push_back({ PACKET_TYPE::S2C_PARTY_ROSTER, rosterWriter.Get_Buffer() });
		outboundBatches.push_back({ entry.pSession, entry.Frames });
	}
	// Include observer notifications in the same bounded FIFO reservation;
	// neither a slow member nor a slow spectator can cause a partial commit.
	for (const auto& [id, player] : m_Players)
	{
		(void)id;
		if (std::find(leaderFirstSessionIds.begin(), leaderFirstSessionIds.end(),
			player.iSessionId) != leaderFirstSessionIds.end()) continue;
		CLIENT_SESSION_RELIABLE_BATCH observer{ Find_Session(player.iSessionId), {} };
		for (const NET_ENTITY_ID entityId : departingEntities)
		{
			S2C_PLAYER_DESPAWNED message{};
			message.iNetEntityId = entityId;
			message.eReason = PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
			CPacketWriter writer;
			if (!Write_Message(writer, message))
				return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "source departure payload failed encoding");
			observer.Frames.push_back({ PACKET_TYPE::S2C_PLAYER_DESPAWNED, writer.Get_Buffer() });
		}
		outboundBatches.push_back(std::move(observer));
	}
	for (const auto& [id, player] : target.m_Players)
	{
		(void)id;
		CLIENT_SESSION_RELIABLE_BATCH observer{ target.Find_Session(player.iSessionId), {} };
		for (const auto& entry : entries)
		{
			S2C_PLAYER_SPAWNED message{};
			message.iPlayerId = entry.Player.iPlayerId;
			message.iNetEntityId = entry.Player.iNetEntityId;
			message.eCharacterClass = entry.Player.eCharacterClass;
			message.strNickName = entry.Player.strNickName;
			message.fPositionX = entry.Player.fPositionX;
			message.fPositionY = entry.Player.fPositionY;
			message.fPositionZ = entry.Player.fPositionZ;
			message.fYawDegrees = entry.Player.fYawDegrees;
			CPacketWriter writer;
			if (!Write_Message(writer, message))
				return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "target spawn payload failed encoding");
			observer.Frames.push_back({ PACKET_TYPE::S2C_PLAYER_SPAWNED, writer.Get_Buffer() });
		}
		outboundBatches.push_back(std::move(observer));
	}

	// All allocating membership work is staged before taking outbound locks.
	// Commit below contains only erases, swaps and atomic player-id stores.
	auto targetPlayers = target.m_Players;
	auto targetSessionPlayers = target.m_PlayerIdBySessionId;
	auto targetEntityPlayers = target.m_PlayerIdByEntityId;
	auto targetSessions = target.m_Sessions;
	auto targetPartyIds = target.m_PartyIdByPlayerId;
	auto targetParties = target.m_PartyMembersByPartyId;
	std::vector<PLAYER_ID> targetMembers;
	targetMembers.reserve(entries.size());
	for (const auto& entry : entries)
	{
		const SERVER_PLAYER& player = entry.Player;
		targetPlayers.emplace(player.iPlayerId, player);
		targetSessionPlayers.emplace(player.iSessionId, player.iPlayerId);
		targetEntityPlayers.emplace(player.iNetEntityId, player.iPlayerId);
		targetSessions.insert_or_assign(player.iSessionId, entry.pSession);
		targetPartyIds.emplace(player.iPlayerId, target.m_iNextPartyId);
		targetMembers.push_back(player.iPlayerId);
	}
	targetParties.emplace(target.m_iNextPartyId, std::move(targetMembers));
	CClientSession::RELIABLE_BATCH_TRANSACTION outbound;
	if (!outbound.Prepare(outboundBatches, status))
	{
		outResult = PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY;
		return false;
	}
	// No callback here may send to the locked queues. Whole-party removal has
	// no intermediate roster; all departures/arrivals were staged above.
	for (const PLAYER_ID memberId : sourceMembers->second)
		m_PartyIdByPlayerId.erase(memberId);
	m_PartyMembersByPartyId.erase(sourceMembers);
	for (const SESSION_ID sessionId : leaderFirstSessionIds)
		Leave(sessionId, PLAYER_DESPAWN_REASON::LEVEL_CHANGED, false);
	target.m_Players.swap(targetPlayers);
	target.m_PlayerIdBySessionId.swap(targetSessionPlayers);
	target.m_PlayerIdByEntityId.swap(targetEntityPlayers);
	target.m_Sessions.swap(targetSessions);
	target.m_PartyIdByPlayerId.swap(targetPartyIds);
	target.m_PartyMembersByPartyId.swap(targetParties);
	target.m_iNextPlayerId += static_cast<PLAYER_ID>(entries.size());
	target.m_iNextNetEntityId += static_cast<NET_ENTITY_ID>(entries.size());
	++target.m_iNextPartyId;
	for (const auto& entry : entries)
		entry.pSession->Bind_PlayerId(entry.Player.iPlayerId);
	outbound.Commit();
	status = "party transfer committed";
	return true;
}

void LostArk::Server::CGameRoom::Notify_PartyTransferFailure(
	const SESSION_ID sessionId, const std::uint32_t requestSequence,
	const LostArk::Shared::WORLD_ID targetWorldId,
	const LostArk::Shared::PARTY_TRANSFER_RESULT result)
{
	if (0u == requestSequence || !m_PlayerIdBySessionId.contains(sessionId)) return;
	LostArk::Shared::S2C_PARTY_TRANSFER_RESULT message{};
	message.iRequestSequence = requestSequence;
	message.eTargetWorldId = targetWorldId;
	message.eResult = result;
	m_PendingPartyTransferResults.insert_or_assign(sessionId, message);
	Flush_PartyTransferResults();
}

void LostArk::Server::CGameRoom::Flush_PartyTransferResults()
{
	using namespace LostArk::Shared;
	for (auto iter = m_PendingPartyTransferResults.begin(); iter != m_PendingPartyTransferResults.end();)
	{
		const auto session = Find_Session(iter->first);
		if (nullptr == session || session->Is_Closing())
		{
			iter = m_PendingPartyTransferResults.erase(iter);
			continue;
		}
		CPacketWriter writer;
		if (!Write_Message(writer, iter->second))
		{
			m_strStatus = "party transfer failure notice failed validation";
			iter = m_PendingPartyTransferResults.erase(iter);
			continue;
		}
		CClientSession::RELIABLE_BATCH_TRANSACTION outbound;
		std::string status;
		if (!outbound.Prepare({ { session, {
			{ PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT, writer.Get_Buffer() } } } }, status))
		{
			++iter;
			continue;
		}
		outbound.Commit();
		iter = m_PendingPartyTransferResults.erase(iter);
	}
}

void LostArk::Server::CGameRoom::Handle_Chat(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_CHAT& request)
{
	using namespace LostArk::Shared;

	const auto senderPlayerIdIter = m_PlayerIdBySessionId.find(sessionId);
	if (senderPlayerIdIter == m_PlayerIdBySessionId.end())
		return;
	const auto senderIter = m_Players.find(senderPlayerIdIter->second);
	if (senderIter == m_Players.end())
		return;

	S2C_CHAT message{};
	message.iFromNetEntityId = senderIter->second.iNetEntityId;
	message.strFromNickname = senderIter->second.strNickName;
	message.strText = request.strText;

	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;

	// Every current room member, sender included -- see Handle_Chat's own
	// header comment for why the sender reads its own bubble off this same
	// broadcast instead of a second local-only path.
	for (const auto& [playerId, player] : m_Players)
	{
		const std::shared_ptr<CClientSession> session =
			Find_Session(player.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(PACKET_TYPE::S2C_CHAT, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}

void LostArk::Server::CGameRoom::Handle_SpawnWorldEntity(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_SPAWN_WORLD_ENTITY& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	/* Character Select owns the monster/mini-boss/Valtan audition buttons; the
	KoukuSaydon arena owns the Debug gate buttons that raise its disabled gate
	bosses. The gate buttons share the Debug-only boundary of the Debug
	teleport they pair with, so a Release Server refuses them the same way.
	Product worlds never accept a client-chosen spawn. */
#ifdef _DEBUG
	const bool koukuGateWorld = WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId;
#else
	const bool koukuGateWorld = false;
#endif
	const bool debugSpawnWorld =
		WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId || koukuGateWorld;
	if (!debugSpawnWorld ||
		!m_PlayerIdBySessionId.contains(sessionId) || nullptr == session)
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	const WORLD_BOOTSTRAP_PLACEMENT* placement =
		Find_Placement(request.strPlacementId);
	if (nullptr == placement)
	{
		if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId)
		{
			Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::REJECTED,
				INVALID_NET_ENTITY_ID);
			return;
		}
		const auto group = std::find_if(
			m_SpawnGroupBootstrap.Get_Groups().begin(),
			m_SpawnGroupBootstrap.Get_Groups().end(),
			[&request](const SPAWN_GROUP_DEFINITION& definition)
			{
				return definition.strSpawnGroupId == request.strPlacementId;
			});
		if (m_SpawnGroupBootstrap.Get_Groups().end() == group)
		{
			Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::REJECTED,
				INVALID_NET_ENTITY_ID);
			return;
		}
		if (!m_SpawnGroupRuntime.Is_ActiveOrCompleted(request.strPlacementId) &&
			!m_SpawnGroupRuntime.Activate_Immediate(
				request.strPlacementId,
				m_SpawnGroupBootstrap,
				[this](const std::string& spawnGroupId,
					const SPAWN_GROUP_ENTRY& entry,
					const SPAWN_GROUP_ANCHOR& anchor,
					const MONSTER_RUNTIME_PROFILE& profile,
					const std::uint32_t ordinal)
				{
					return Spawn_Monster(
						spawnGroupId, entry, anchor, profile, ordinal);
				}))
		{
			Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::REJECTED,
				INVALID_NET_ENTITY_ID);
			return;
		}

		if (!Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::ACTIVATED,
			INVALID_NET_ENTITY_ID))
		{
			session->Request_Close();
		}
		return;
	}

	const bool admittedPlacement =
		WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId ?
			(!placement->isEnabled &&
			 WORLD_BOOTSTRAP_KIND::BOSS == placement->eKind &&
			 placement->strArchetypeId == "BOSS_VALTAN") :
			CKoukuSaydonBrain::Is_ArenaBossPlacement(m_eWorldId, *placement);
	if (!admittedPlacement)
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}
	const auto existing = std::find_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[&request](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.strPlacementId == request.strPlacementId;
		});
	if (m_WorldEntities.end() != existing)
	{
		if (!Send_WorldEntitySpawned(session, *existing) ||
			!Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS,
				existing->iNetEntityId))
		{
			session->Request_Close();
		}
		if (auto player = m_Players.find(m_PlayerIdBySessionId.at(sessionId)); player != m_Players.end())
			Apply_KoukuGateEntryCard(player->second, *existing);
		return;
	}
	if (m_iNextNetEntityId == INVALID_NET_ENTITY_ID)
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	SERVER_WORLD_ENTITY staged{};
	if (!Build_WorldEntity(*placement, m_iNextNetEntityId, staged))
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	if (auto player = m_Players.find(m_PlayerIdBySessionId.at(sessionId)); player != m_Players.end())
		Apply_KoukuGateEntryCard(player->second, m_WorldEntities.back());
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	if (!Send_WorldEntitySpawnResult(
		session,
		request.strPlacementId,
		WORLD_ENTITY_SPAWN_RESULT::SPAWNED,
		m_WorldEntities.back().iNetEntityId))
	{
		session->Request_Close();
	}
}

LostArk::Server::SERVER_WORLD_ENTITY*
LostArk::Server::CGameRoom::Find_KoukuSaydonAuditionBoss()
{
	const auto found = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[this](const SERVER_WORLD_ENTITY& entity)
		{
			return CKoukuSaydonBrain::Is_GateOneBoss(m_eWorldId, entity);
		});
	return m_WorldEntities.end() == found ? nullptr : &*found;
}
