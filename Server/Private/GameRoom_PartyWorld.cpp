#include "GameRoom.h"
#include "Gameplay/MaharakaWaterpangContract.h"

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

	/* Valtan after its clear; KoukuSaydon after its last gate cleared (the gate progress
	   widget's exit button). */
	if (WORLD_ID::VALTAN_ARENA == m_eWorldId)
	{
		if (!m_bValtanRaidCleared && !Is_RaidClearTestModeEnabled())
			return;
	}
	else if (WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId)
	{
		if (0u == Gate_Count() || 0u == (m_GateProgress.iClearedMask & (1u << (Gate_Count() - 1u))))
			return;
	}
	else
		return;

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	// Solo only -- unlike Handle_ConfirmNpcEntry, returning is never batched
	// across a party. Each player presses their own button independently.
	(void)Stage_ReturnToBern(sessionIter->second, request.iRequestSequence);
}

bool LostArk::Server::CGameRoom::Stage_ReturnToBern(
	const LostArk::Shared::PLAYER_ID playerId,
	const std::uint32_t requestSequence)
{
	using namespace LostArk::Shared;
	// Direct Lobby/debug entries have no source NPC; retain their established exit.
	constexpr const char* BERN_RETURN_PLACEMENT_ID = "npc.bern.beda.guide";

	const auto playerIter = m_Players.find(playerId);
	if (playerIter == m_Players.end())
		return false;
	const SERVER_PLAYER& player = playerIter->second;
	if (INVALID_SESSION_ID == player.iSessionId ||
		CHARACTER_CLASS_ID::END == player.eCharacterClass ||
		player.strNickName.empty())
	{
		return false;
	}

	const SESSION_ID sessionId = player.iSessionId;
	const bool isAlreadyStaged = std::any_of(
		m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
		[sessionId](const SERVER_WORLD_TRANSFER_REQUEST& pending)
		{
			return pending.iSessionId == sessionId;
		});
	if (isAlreadyStaged)
		return false;

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	transfer.iSessionId = player.iSessionId;
	transfer.eTargetWorldId = WORLD_ID::BERN;
	transfer.eCharacterClass = player.eCharacterClass;
	transfer.strNickName = player.strNickName;
	transfer.iVoiceType = player.iVoiceType;
	transfer.strAppearanceJson = player.strAppearanceJson;
	transfer.CarriedDurability = player.Get_DurabilityState();
	transfer.iHonorTitleId = player.iHonorTitleId;
	transfer.iPartyRequestSequence = requestSequence;
	transfer.strSpawnPlacementOverrideId = player.strRaidReturnNpcPlacementId.empty() ?
		BERN_RETURN_PLACEMENT_ID : player.strRaidReturnNpcPlacementId;
	// Carries Valtan clear rewards (and anything else still held) across the
	// trip -- without this, Stage_PlayerEntry's default fresh-entry grant would
	// silently reset the player back to just 3 starting potions.
	transfer.CarriedInventory = player.Inventory;
	transfer.CarriedPurse = player.Purse;
    if (const auto party = m_PartyIdByPlayerId.find(player.iPlayerId); party != m_PartyIdByPlayerId.end())
        if (const auto companion = m_Guides.find(party->second); companion != m_Guides.end() &&
            (companion->second.AnchorId == player.iPlayerId || m_PartyMembersByPartyId.at(party->second).size() == 1u))
            transfer.PartyBatchSessionIds.push_back(sessionId);
	m_PendingWorldTransfers.push_back(std::move(transfer));
	return true;
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
	if (Invite_Guide(inviter, request.iTargetNetEntityId)) return;

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
	if (const auto companion = m_Guides.find(partyId); companion != m_Guides.end())
	{
		const auto actor = m_Players.find(companion->second.PlayerId);
		if (actor != m_Players.end()) message.GuideCompanion = PARTY_ROSTER_MEMBER{actor->second.iNetEntityId, actor->second.strNickName, actor->second.eCharacterClass, actor->second.eControlKind};
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

bool LostArk::Server::CGameRoom::Activate_SpawnGroupFromTrigger(
	const std::string& spawnGroupId)
{
	return m_SpawnGroupRuntime.Activate_Repeat(
		spawnGroupId,
		[this](const std::string& id)
		{
			return Count_SpawnGroupEntities(id);
		});
}

void LostArk::Server::CGameRoom::Handle_DebugResummonWaveMonsters(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_RESUMMON_WAVE_MONSTERS& request)
{
#ifdef _DEBUG
	using namespace LostArk::Shared;
	const auto report = [this](std::string line)
	{
		m_strStatus = std::move(line);
		std::cout << "[WaveMonsters] " << m_strStatus << '\n';
	};
	/* The request names this room's own world, that world has a button for it, and
	the session owns a player here. The Valtan pattern audition owns its arena while
	it runs (the trigger boxes are not evaluated then either), so the wave waits. */
	const WAVE_MONSTER_BUTTON_ROW* row =
		CServerTriggerSystem::Find_WaveMonsterButton(m_eWorldId, request.eButton);
	if (request.eWorldId != m_eWorldId || nullptr == row ||
		!m_PlayerIdBySessionId.contains(sessionId))
	{
		report("Wave monster re-summon refused: wrong world, no such button or no player in this room");
		return;
	}
	if (WORLD_ID::VALTAN_ARENA == m_eWorldId &&
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE != m_ValtanTimelineAudition.ePhase)
	{
		report("Wave monster re-summon refused: the Valtan pattern audition is running");
		return;
	}
	const std::string groupId = row->pSpawnGroupId;
	const auto& groups = m_SpawnGroupBootstrap.Get_Groups();
	if (std::none_of(groups.begin(), groups.end(),
		[&groupId](const SPAWN_GROUP_DEFINITION& definition)
		{
			return definition.strSpawnGroupId == groupId;
		}))
	{
		report("Wave monster re-summon refused: spawn group is missing: " + groupId);
		return;
	}
	/* Remove what is still alive, then start the group over from its first wave. The
	monsters appear on the next spawn-group update at the group's authored anchors,
	wherever the player stands. */
	for (auto entity = m_WorldEntities.begin(); entity != m_WorldEntities.end();)
	{
		if (WORLD_BOOTSTRAP_KIND::MONSTER != entity->eKind ||
			entity->strSpawnGroupId != groupId)
		{
			++entity;
			continue;
		}
		m_CombatObjectRuntime.Cancel_Source(entity->iNetEntityId);
		Broadcast_WorldEntityDespawned(entity->iNetEntityId);
		entity = m_WorldEntities.erase(entity);
	}
	if (!Broadcast_CombatObjectLifecycle())
	{
		Mark_RuntimeFailure("wave-resummon.combat-object-lifecycle");
		return;
	}
	if (!m_SpawnGroupRuntime.Reset_Group(groupId) ||
		!m_SpawnGroupRuntime.Activate(groupId))
	{
		report("Wave monster re-summon failed to restart the group: " + groupId);
		return;
	}
	report("Wave monsters re-summoned: " + groupId);
#else
	(void)sessionId;
	(void)request;
#endif
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
	const auto activateTarget = [this](const WORLD_TRIGGER_ACTION_KIND kind,
		const std::string& targetId)
	{
		return Activate_TriggerTarget(kind, targetId);
	};
	/* Mario crossings and exits wait for G now, so the request must reach the same
	   room-owned admission the tick uses for a stepped-in entry (stage, authority
	   locks, contact interruption, the terminal exit's return destination). */
	const SERVER_TRIGGER_MOVE_ENTRY_HANDLER moveEntry =
		[this](const WORLD_BOOTSTRAP_PLACEMENT& trigger, SERVER_PLAYER& player,
			const std::uint32_t actionStartTick)
	{
		return Begin_MarioTriggerMove(trigger, player, actionStartTick);
	};
	/* G with no box on offer names none (INTERACT_TRIGGER_HERE_ID): the Server
	   decides which boxes the player is standing in and runs those. */
	const bool answersHere =
		LostArk::Shared::INTERACT_TRIGGER_HERE_ID == request.strTriggerPlacementId;
	if (answersHere
		? 0u == m_ServerTriggerSystem.Activate_Here(
			playerId->second, m_Players, actionTick, transfers, activateTarget, moveEntry)
		: !m_ServerTriggerSystem.Activate_Interact(
			playerId->second,
			request.strTriggerPlacementId,
			m_Players,
			actionTick,
			transfers,
			activateTarget,
			moveEntry))
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
	/* The offer stays: every box is repeatable and the player is still inside
	   it. Walking out withdraws it (Evaluate_Entries). */
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
		MAHARAKA_WATERPANG_EVENT_KIND waterpangKind{};
		const bool waterpangDebug = m_eWorldId == WORLD_ID::MAHARAKA &&
			Find_MaharakaWaterpangDebugKind(request.strTargetId, waterpangKind);
		if (request.eWorldId != m_eWorldId || (!waterpangDebug &&
			m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA && m_eWorldId != WORLD_ID::VALTAN_ARENA))
			return Result::WRONG_WORLD;
		const auto playerId = m_PlayerIdBySessionId.find(sessionId);
		if (playerId == m_PlayerIdBySessionId.end()) return Result::INVALID_PLAYER;
		const auto player = m_Players.find(playerId->second);
		if (player == m_Players.end() || !player->second.iCurrentHp) return Result::INVALID_PLAYER;
		auto& last = m_WorldPlaybackRequestSequences[sessionId];
		if (request.iRequestSequence <= last) return Result::STALE_REQUEST;
		last = request.iRequestSequence;
		if (waterpangDebug)
			return request.eOperation == Op::PLAY_SEQUENCE ?
				Start_MaharakaWaterpangDebugEvent(request.strTargetId) : Result::INVALID_TARGET;
		if (request.eOperation == Op::PLACE_ROOM_PLAYER) return Apply_DebugRoomPlayerArrival(sessionId, request);
		const bool replay = request.eOperation == Op::REPLAY_TRIGGER || request.eOperation == Op::REPLAY_SEQUENCE;
		const auto play = [&](const std::string& id)
		{
			const auto& ids = m_WorldBootstrap.Get_SequenceInstanceIds();
			if (std::find(ids.begin(), ids.end(), id) == ids.end()) return false;
			return Broadcast_WorldSequencePlay(id, 1.f, 0.f, 0.f, 0.f, 0u, {},
				replay ? WORLD_SEQUENCE_OPERATION::REPLAY : WORLD_SEQUENCE_OPERATION::PLAY);
		};
		if (request.eOperation == Op::PLAY_TRIGGER || request.eOperation == Op::REPLAY_TRIGGER)
		{
			std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
			const auto verdict = m_ServerTriggerSystem.Debug_Activate(playerId->second, request.strTargetId,
				replay, m_Players, m_iServerTick ? m_iServerTick : 1u, transfers,
				[&](WORLD_TRIGGER_ACTION_KIND kind, const std::string& id)
				{
					if (kind == WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE) return play(id);
					if (kind == WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP) return Activate_SpawnGroupFromTrigger(id);
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

LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT LostArk::Server::CGameRoom::Apply_DebugRoomPlayerArrival(
	const SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request)
{
	using namespace LostArk::Shared;
	using Result = DEBUG_WORLD_PLAYBACK_RESULT;
#ifndef _DEBUG
	(void)sessionId; (void)request;
	return Result::DISABLED;
#else
	if (m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA || request.eWorldId != m_eWorldId)
		return Result::WRONG_WORLD;
	CPacketWriter validation;
	if (request.eOperation != DEBUG_WORLD_PLAYBACK_OPERATION::PLACE_ROOM_PLAYER || !Write_Message(validation, request) ||
		!request.strTargetId.starts_with("KAKULSAYDON_G1_PATTERN_") ||
		!request.strOccurrenceId.starts_with(request.strTargetId + ".")) return Result::INVALID_TARGET;
	const auto requesterId = m_PlayerIdBySessionId.find(sessionId);
	const auto requester = requesterId == m_PlayerIdBySessionId.end() ? m_Players.end() : m_Players.find(requesterId->second);
	const auto requesterSession = Find_Session(sessionId);
	if (requester == m_Players.end() || requester->second.iSessionId != sessionId || !requester->second.iCurrentHp ||
		!requesterSession || !requesterSession->Is_Open() || requesterSession->Is_Closing()) return Result::INVALID_PLAYER;
	auto& run = m_RoomPlayerArrivalRuns[sessionId];
	if (request.iRunEpoch < run.iEpoch) return Result::STALE_REQUEST;
	if (request.iRunEpoch == run.iEpoch && request.strTargetId != run.strRootPatternId) return Result::INVALID_TARGET;
	if (request.iRunEpoch > run.iEpoch)
	{
		ROOM_PLAYER_ARRIVAL_RUN staged;
		staged.iEpoch = request.iRunEpoch; staged.strRootPatternId = request.strTargetId;
		// m_Players is ordered by stable PlayerId. Never resolve a slot again after leave/join.
		for (const auto& [id, player] : m_Players)
		{
			const auto binding = m_PlayerIdBySessionId.find(player.iSessionId);
			const auto connection = Find_Session(player.iSessionId);
			if (binding != m_PlayerIdBySessionId.end() && binding->second == id && connection && connection->Is_Open() && !connection->Is_Closing())
				staged.Players.emplace_back(id, player.iSessionId);
			if (staged.Players.size() == 4u) break;
		}
		run = std::move(staged);
	}
	if (const auto done = run.Occurrences.find(request.strOccurrenceId); done != run.Occurrences.end())
		return done->second == Result::ACCEPTED ? Result::ALREADY_USED : done->second;
	if (run.Occurrences.size() >= 128u) return Result::INVALID_TARGET;
	const auto finish = [&](Result result) { run.Occurrences.emplace(request.strOccurrenceId, result); return result; };
	if (request.iRoomPlayerSlot >= run.Players.size()) return finish(Result::SKIPPED_PLAYER);
	const auto [targetId, targetSessionId] = run.Players[request.iRoomPlayerSlot];
	const auto target = m_Players.find(targetId);
	const auto binding = m_PlayerIdBySessionId.find(targetSessionId);
	const auto connection = Find_Session(targetSessionId);
	if (target == m_Players.end() || target->second.iSessionId != targetSessionId ||
		binding == m_PlayerIdBySessionId.end() || binding->second != targetId || !connection || !connection->Is_Open() || connection->Is_Closing())
		return finish(Result::SKIPPED_PLAYER);
	C2S_DEBUG_TELEPORT_TO_POSITION position{};
	position.eWorldId = m_eWorldId;
	position.fPositionX = request.fPositionX; position.fPositionY = request.fPositionY; position.fPositionZ = request.fPositionZ;
	SERVER_NAV_POINT ground{};
	const auto verdict = Validate_DebugTeleportDestination(target->second, position, ground);
	if (verdict != DEBUG_TELEPORT_RESULT::ACCEPTED)
	{
		m_strStatus = "Sequence room player arrival rejected by teleport validation: " + std::to_string(static_cast<unsigned>(verdict));
		return finish(verdict == DEBUG_TELEPORT_RESULT::REJECTED_PLAYER_STATE ? Result::INVALID_PLAYER : Result::ACTION_REJECTED);
	}
	// The same teleport validator accepted this participant and destination before live state changes.
	Reset_PlayerForDebugTeleport(target->second);
	target->second.fPositionX = ground.x; target->second.fPositionY = ground.y; target->second.fPositionZ = ground.z;
	Update_MarioControlState(target->second);
	m_strStatus = "Sequence room player arrival committed for slot " + std::to_string(request.iRoomPlayerSlot + 1u);
	return finish(Result::ACCEPTED);
#endif
}

bool LostArk::Server::CGameRoom::Broadcast_WorldSequencePlay(
	const std::string& instanceId,
	const float playbackSpeed, const float positionOffsetX,
	const float positionOffsetY, const float positionOffsetZ, const std::uint32_t durationMs,
	const std::string& targetSequenceInstanceId,
	const LostArk::Shared::WORLD_SEQUENCE_OPERATION operation)
{
	using namespace LostArk::Shared;

	// A published raid entry replaces its first World-only cutscene with the
	// complete authored Sequence. All other world playback keeps its own path.
	if (m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA &&
		(operation == WORLD_SEQUENCE_OPERATION::PLAY || operation == WORLD_SEQUENCE_OPERATION::REPLAY))
	{
		const auto* gate = m_GameplayCatalog.Active().Find_KoukuRaidGate("GATE1");
		if (gate && !gate->strEntrySequenceInstanceId.empty() && gate->strEntrySequenceInstanceId == instanceId)
		{
			if (Is_KoukuRaidRunning() || m_Players.empty()) return false;
			const SESSION_ID owner = m_Players.begin()->second.iSessionId;
			C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request;
			request.eWorldId = m_eWorldId; request.eOperation = KOUKUSAYDON_RAID_OPERATION::START;
			const auto previous = m_KoukuRaidReceipts.find(owner);
			const auto priorSequence = previous == m_KoukuRaidReceipts.end() ? 0u : previous->second.first.iRequestSequence;
			if (priorSequence == (std::numeric_limits<std::uint32_t>::max)()) return false;
			request.iRequestSequence = priorSequence + 1u;
			request.strStartGateId = gate->strGateId;
			request.ExpectedGameplayRevision = m_GameplayCatalog.Get_ActiveRevision();
			request.iActionSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(m_GameplayCatalog.Active());
			request.iSequenceSourceRevision = gate->iSequenceRevision;
			std::string reason;
			if (!Begin_KoukuRaidPreparation(owner, request, reason)) { m_strStatus = reason; return false; }
			m_KoukuRaid.strEntryTriggerSequenceId = instanceId;
			return true;
		}
	}

	// The authored Pattern owns Saydon. Reject the retired trigger before any
	// player mutation or broadcast; STOP remains valid for stale-client cleanup.
	if ((operation == WORLD_SEQUENCE_OPERATION::PLAY || operation == WORLD_SEQUENCE_OPERATION::REPLAY) &&
		(instanceId == "world.sequence.instance.original_kouku" ||
		 targetSequenceInstanceId == "world.sequence.instance.original_kouku"))
	{
		m_strStatus = "Legacy Saydon cutscene is retired; use the authored Sequence Pattern";
		return false;
	}

	S2C_WORLD_SEQUENCE_PLAY message{};
	const bool waterpang = m_eWorldId == WORLD_ID::MAHARAKA && instanceId == MAHARAKA_WATERPANG_INTRO_INSTANCE;
	if (waterpang)
	{
		// Re-entry and debug replay cannot restart an in-progress arena session.
		if (operation != WORLD_SEQUENCE_OPERATION::PLAY) return false;
		if (m_MaharakaWaterpangIntro) return true;
		message.iServerTick = m_iServerTick;
		message.iStartTick = m_iServerTick + MAHARAKA_WATERPANG_COUNTDOWN_TICKS;
		if (!message.iStartTick) ++message.iStartTick;
	}
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
		return false;
	if (waterpang)
	{
		m_MaharakaWaterpangIntro = message;
		// The match takes the cast from here; a Debug forced event ends with the reservation.
		m_MaharakaWaterpangDebugEvent.reset();
	}
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
	return true;
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
		Remove_Guide(partyId);
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
	const std::uint32_t requestSequence, const std::string& raidReturnNpcPlacementId)
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
	transfer.strRaidReturnNpcPlacementId = raidReturnNpcPlacementId;
	transfer.eCharacterClass = leader.eCharacterClass;
	transfer.strNickName = leader.strNickName;
	transfer.iVoiceType = leader.iVoiceType;
	transfer.strAppearanceJson = leader.strAppearanceJson;
	transfer.CarriedDurability = leader.Get_DurabilityState();
	transfer.iHonorTitleId = leader.iHonorTitleId;
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
		if (batchMemberIds.size() > 1u || (m_PartyIdByPlayerId.contains(leader.iPlayerId) && m_Guides.contains(m_PartyIdByPlayerId.at(leader.iPlayerId))))
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
				proposal.Voters, targetWorld, proposal.iRequestSequence,
				proposal.strNpcPlacementId))
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
	LostArk::Shared::PARTY_TRANSFER_RESULT& outResult, std::string& status,
	const std::string& raidReturnNpcPlacementId, const std::string& spawnPlacementOverrideId)
{
	using namespace LostArk::Shared;
	outResult = PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE;
	const auto reject = [&outResult, &status](const PARTY_TRANSFER_RESULT reason, const char* detail)
	{
		outResult = reason;
		status = detail;
		return false;
	};
    const bool returning = target.m_eWorldId == WORLD_ID::BERN &&
        (m_eWorldId == WORLD_ID::VALTAN_ARENA || m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA);
    const bool entering = m_eWorldId == WORLD_ID::BERN &&
        (target.m_eWorldId == WORLD_ID::VALTAN_ARENA || target.m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA);
	if (!m_isReady || !target.m_isReady || (!entering && !returning) ||
		leaderFirstSessionIds.empty() || leaderFirstSessionIds.size() > MAX_PARTY_MEMBERS || (returning && leaderFirstSessionIds.size() != 1u))
		return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "invalid party transfer world/batch");
	const auto leader = m_PlayerIdBySessionId.find(leaderFirstSessionIds.front());
	if (leader == m_PlayerIdBySessionId.end())
		return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "party leader is no longer present");
	const auto sourceParty = m_PartyIdByPlayerId.find(leader->second);
	if (sourceParty == m_PartyIdByPlayerId.end())
		return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "source party no longer exists");
	const auto sourceMembers = m_PartyMembersByPartyId.find(sourceParty->second);
	if (sourceMembers == m_PartyMembersByPartyId.end() ||
		(!returning && (sourceMembers->second.size() != leaderFirstSessionIds.size() ||
		sourceMembers->second.front() != leader->second)))
		return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "source party changed before transfer");
	if (0u == target.m_iNextPartyId || target.m_PartyMembersByPartyId.contains(target.m_iNextPartyId))
		return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "target party identity is exhausted");

    const std::uint32_t sourcePartyId = sourceParty->second;
    const std::vector<PLAYER_ID> departingMembers = returning ? std::vector<PLAYER_ID>{leader->second} : sourceMembers->second;
    const auto sourceGuide = m_Guides.find(sourcePartyId);
    const bool carryGuide = sourceGuide != m_Guides.end() &&
        (!returning || sourceGuide->second.AnchorId == leader->second || sourceMembers->second.size() == 1u);
    std::optional<SERVER_PLAYER> guideEntry;
    GUIDE_RUNTIME guideRuntime;
    std::vector<PACKET_FRAME> guideFrames;
	std::vector<STAGED_PLAYER_ENTRY> entries;
	std::vector<NET_ENTITY_ID> departingEntities;
	entries.reserve(leaderFirstSessionIds.size());
	departingEntities.reserve(leaderFirstSessionIds.size());
	for (std::size_t index = 0; index < leaderFirstSessionIds.size(); ++index)
	{
		const auto member = m_Players.find(departingMembers[index]);
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
		enter.iVoiceType = member->second.iVoiceType;
		enter.strAppearanceJson = member->second.strAppearanceJson;
		STAGED_PLAYER_ENTRY entry{};
		SESSION_DIAGNOSTIC_REASON reason{};
		if (!target.Stage_PlayerEntry(session, enter, entries, entry, reason, status,
			spawnPlacementOverrideId, member->second.Inventory, member->second.iHonorTitleId, raidReturnNpcPlacementId,
			member->second.Purse, member->second.Get_DurabilityState()))
		{
			outResult = SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL == reason ?
				PARTY_TRANSFER_RESULT::REJECTED_ROOM_FULL : PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED;
			return false;
		}
		entries.push_back(std::move(entry));
		departingEntities.push_back(member->second.iNetEntityId);
	}
    if (carryGuide)
    {
        if (!target.m_GuideCatalog.Loaded || target.m_GuideCatalog.Revision != m_GuideCatalog.Revision ||
            target.m_Players.size() + entries.size() + 1u > MAX_WORLD_SNAPSHOT_PLAYERS)
            return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "guide generation or destination capacity is unavailable");
        const auto anchor = std::find(departingMembers.begin(), departingMembers.end(), sourceGuide->second.AnchorId);
        const auto index = anchor == departingMembers.end() ? 0u : static_cast<std::size_t>(anchor - departingMembers.begin());
        const auto& owner = entries[index].Player;
        SERVER_PLAYER companion;
        bool guideAdmitted = false;
        for (unsigned candidate = 0; candidate < 16 && !guideAdmitted; ++candidate)
        {
            const float angle = static_cast<float>(candidate) * 3.14159265359f / 8.f;
            if (!target.Build_GuidePlayer(target.m_iNextGuidePlayerId,
                target.m_iNextNetEntityId + static_cast<NET_ENTITY_ID>(entries.size()),
                owner.fPositionX + std::sin(angle) * target.m_GuideCatalog.DesiredDistance,
                owner.fPositionY,
                owner.fPositionZ + std::cos(angle) * target.m_GuideCatalog.DesiredDistance, companion)) continue;
            guideAdmitted = std::none_of(entries.begin(), entries.end(), [&](const auto& entry) {
                return std::abs(entry.Player.fPositionY - companion.fPositionY) < 1.5f &&
                    std::hypot(entry.Player.fPositionX - companion.fPositionX,
                        entry.Player.fPositionZ - companion.fPositionZ) < .75f;
            });
        }
        if (!guideAdmitted)
            return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "guide destination failed navigation or collision admission");
        guideRuntime.PlayerId = companion.iPlayerId; guideRuntime.AnchorId = owner.iPlayerId;
        guideRuntime.EventSequence = sourceGuide->second.EventSequence;
        guideEntry = companion;
        const auto oldActor = m_Players.find(sourceGuide->second.PlayerId);
        if (oldActor == m_Players.end()) return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "guide actor disappeared before transfer");
        departingEntities.push_back(oldActor->second.iNetEntityId);
        S2C_PLAYER_SPAWNED spawn;
        spawn.iPlayerId=companion.iPlayerId;spawn.iNetEntityId=companion.iNetEntityId;spawn.eCharacterClass=companion.eCharacterClass;
        spawn.eControlKind=companion.eControlKind;spawn.strNickName=companion.strNickName;spawn.iVoiceType=companion.iVoiceType;
        spawn.fPositionX=companion.fPositionX;spawn.fPositionY=companion.fPositionY;spawn.fPositionZ=companion.fPositionZ;spawn.fYawDegrees=companion.fYawDegrees;
        CPacketWriter writer;if(!Write_Message(writer,spawn))return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED,"guide spawn encoding failed");
        guideFrames.push_back({PACKET_TYPE::S2C_PLAYER_SPAWNED,writer.Get_Buffer()});
    }
	std::vector<CLIENT_SESSION_RELIABLE_BATCH> outboundBatches;
	S2C_PARTY_ROSTER roster{};
	for (const auto& entry : entries)
		roster.Members.push_back({ entry.Player.iNetEntityId, entry.Player.strNickName,
			entry.Player.eCharacterClass });
    if (guideEntry) roster.GuideCompanion = PARTY_ROSTER_MEMBER{guideEntry->iNetEntityId,guideEntry->strNickName,guideEntry->eCharacterClass,guideEntry->eControlKind};
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
		entry.Frames.insert(entry.Frames.end(),guideFrames.begin(),guideFrames.end());
		entry.Frames.push_back({ PACKET_TYPE::S2C_PARTY_ROSTER, rosterWriter.Get_Buffer() });
		outboundBatches.push_back({ entry.pSession, entry.Frames });
	}
	// Include observer notifications in the same bounded FIFO reservation;
	// neither a slow member nor a slow spectator can cause a partial commit.
	for (const auto& [id, player] : m_Players)
	{
		(void)id;
		if (!player.Is_Human()) continue;
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
		if (!player.Is_Human()) continue;
		CLIENT_SESSION_RELIABLE_BATCH observer{ target.Find_Session(player.iSessionId), {} };
		for (const auto& entry : entries)
		{
			S2C_PLAYER_SPAWNED message{};
			message.iPlayerId = entry.Player.iPlayerId;
			message.iNetEntityId = entry.Player.iNetEntityId;
			message.eCharacterClass = entry.Player.eCharacterClass;
			message.strNickName = entry.Player.strNickName;
			message.iVoiceType = entry.Player.iVoiceType;
			message.strAppearanceJson = entry.Player.strAppearanceJson;
			message.fPositionX = entry.Player.fPositionX;
			message.fPositionY = entry.Player.fPositionY;
			message.fPositionZ = entry.Player.fPositionZ;
			message.fYawDegrees = entry.Player.fYawDegrees;
			CPacketWriter writer;
			if (!Write_Message(writer, message))
				return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "target spawn payload failed encoding");
			observer.Frames.push_back({ PACKET_TYPE::S2C_PLAYER_SPAWNED, writer.Get_Buffer() });
		}
        observer.Frames.insert(observer.Frames.end(),guideFrames.begin(),guideFrames.end());
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
    auto targetGuides = target.m_Guides;
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
    if(guideEntry){targetPlayers.emplace(guideEntry->iPlayerId,*guideEntry);targetEntityPlayers.emplace(guideEntry->iNetEntityId,guideEntry->iPlayerId);targetGuides.emplace(target.m_iNextPartyId,guideRuntime);}
	targetParties.emplace(target.m_iNextPartyId, std::move(targetMembers));
	CClientSession::RELIABLE_BATCH_TRANSACTION outbound;
	if (!outbound.Prepare(outboundBatches, status))
	{
		outResult = PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY;
		return false;
	}
	// No callback here may send to the locked queues. Whole-party removal has
	// no intermediate roster; all departures/arrivals were staged above.
    for (const PLAYER_ID memberId : departingMembers) m_PartyIdByPlayerId.erase(memberId);
    for (const auto id : departingMembers) std::erase(sourceMembers->second,id);
    const bool sourcePartyRemains = !sourceMembers->second.empty();
    if(!sourcePartyRemains) m_PartyMembersByPartyId.erase(sourceMembers);
    if(carryGuide) Remove_Guide(sourcePartyId,false);
	for (const SESSION_ID sessionId : leaderFirstSessionIds)
		Leave(sessionId, PLAYER_DESPAWN_REASON::LEVEL_CHANGED, false);
	target.m_Players.swap(targetPlayers);
	target.m_PlayerIdBySessionId.swap(targetSessionPlayers);
	target.m_PlayerIdByEntityId.swap(targetEntityPlayers);
	target.m_Sessions.swap(targetSessions);
	target.m_PartyIdByPlayerId.swap(targetPartyIds);
	target.m_PartyMembersByPartyId.swap(targetParties);
    target.m_Guides.swap(targetGuides);
	target.m_iNextPlayerId += static_cast<PLAYER_ID>(entries.size());
    if(guideEntry) ++target.m_iNextGuidePlayerId;
	target.m_iNextNetEntityId += static_cast<NET_ENTITY_ID>(entries.size() + (guideEntry ? 1u : 0u));
	++target.m_iNextPartyId;
	for (const auto& entry : entries)
		entry.pSession->Bind_PlayerId(entry.Player.iPlayerId);
	outbound.Commit();
    if(sourcePartyRemains) Broadcast_PartyRoster(sourcePartyId);
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

void LostArk::Server::CGameRoom::Handle_RoomPing(
	const SESSION_ID sessionId, const LostArk::Shared::C2S_ROOM_PING& request)
{
	using namespace LostArk::Shared;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	if (binding == m_PlayerIdBySessionId.end() || request.eWorldId != m_eWorldId) return;
	const auto sender = m_Players.find(binding->second);
	if (sender == m_Players.end() || sender->second.iSessionId != sessionId) return;
	auto& player = sender->second;
	CPacketWriter validated;
	if (!Write_Message(validated, request) || !Is_NewerSequence(request.iClientSequence, player.iLastRoomPingSequence) ||
		!player.iCurrentHp || !player.isCombatReady || player.eAction == PLAYER_ACTION_STATE::DEAD) return;
	player.iLastRoomPingSequence = request.iClientSequence;
	const auto tick = m_iServerTick ? m_iServerTick : 1u;
	if (player.iLastRoomPingTick && Elapsed_ServerTicksSkippingReservedZero(player.iLastRoomPingTick, tick) < 8u) return;
	SERVER_NAV_POINT ground;
	if (!m_ServerNavigation.Sample_Position(request.fPositionX, request.fPositionZ, ground, request.fPositionY)) return;
	S2C_ROOM_PING message;
	message.eWorldId = m_eWorldId; message.iFromNetEntityId = player.iNetEntityId;
	message.iClientSequence = request.iClientSequence;
	message.fPositionX = ground.x; message.fPositionY = ground.y; message.fPositionZ = ground.z;
	CPacketWriter writer;
	if (!Write_Message(writer, message)) return;
	player.iLastRoomPingTick = tick;
	for (const auto& [id, recipient] : m_Players)
		if (const auto session = Find_Session(recipient.iSessionId); session &&
			!session->Send_Frame(PACKET_TYPE::S2C_ROOM_PING, writer.Get_Buffer())) session->Request_Close();
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
	Guide_ChatCommand(senderIter->second, request.strText);

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
	/* Arena F1 controls share the same disabled boss placement allowlist in
	Debug and Release. Character Select keeps its private-room admission. */
	if (Is_KoukuRaidRunning())
	{
		Send_WorldEntitySpawnResult(session, request.strPlacementId, WORLD_ENTITY_SPAWN_RESULT::REJECTED, INVALID_NET_ENTITY_ID);
		return;
	}
	const bool koukuGateWorld = WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId;
    const bool valtanWorld = WORLD_ID::VALTAN_ARENA == m_eWorldId;
	const bool debugSpawnWorld =
		WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId || koukuGateWorld || valtanWorld;
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

	const bool admittedPlacement = valtanWorld ?
        (!placement->isEnabled && placement->strPlacementId == "boss.valtan.center" &&
            placement->eKind == WORLD_BOOTSTRAP_KIND::BOSS && placement->strArchetypeId == "BOSS_VALTAN" &&
            placement->strEncounterId == "ENCOUNTER_VALTAN") :
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

#ifdef _DEBUG
    if (valtanWorld)
    {
        staged.bIntroPatternConsumed = true;
        staged.bAutomaticPatternSequenceAuditionOverride = true;
        staged.bAutomaticPatternSequenceAuditionHold = true;
    }
#endif
	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	if (auto player = m_Players.find(m_PlayerIdBySessionId.at(sessionId)); player != m_Players.end())
		Apply_KoukuGateEntryCard(player->second, m_WorldEntities.back());
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	Note_GatePlacementRaised(m_WorldEntities.back().strPlacementId);
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


void LostArk::Server::CGameRoom::Update_MaharakaWaterpangHazards(
	SERVER_PLAYER& player, const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if (m_eWorldId != WORLD_ID::MAHARAKA || !updateTick ||
		!player.iCurrentHp || !player.isCombatReady || player.TriggerMove.isActive || player.bWaterpangLaunch ||
		player.eAction == PLAYER_ACTION_STATE::DEAD || player.eAction == PLAYER_ACTION_STATE::FALLING)
		return;
	MAHARAKA_WATERPANG_EVENT_SAMPLE event{};
	if (!Sample_MaharakaWaterpangNow(updateTick, event))
		return;
	// Only a body on the arena deck or its pier takes part; the pool and shore do not.
	if (!m_ServerNavigation.Is_Loaded() ||
		!Is_MaharakaWaterpangArenaFootprint(player.fPositionX, player.fPositionZ) ||
		!m_ServerNavigation.Is_PointWalkableInRegion(MAHARAKA_WATERPANG_REGION_ID,
			player.fPositionX, player.fPositionZ, player.fPositionY))
		return;
	const float bodyRadius = WorldCollision::PLAYER_HALF_EXTENT_X;
	SERVER_WORLD_TO_PLAYER_HIT hit{};
	hit.iServerTick = updateTick;
	hit.bIgnoreCounter = true;
	hit.bForcePush = true;
	hit.bUsePushDirection = true;
	hit.bKnockdown = true;
	if (MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL != event.eKind)
	{
		if (!event.bFiring || (player.iWaterpangCannonHitTick &&
			static_cast<std::int32_t>(updateTick - player.iWaterpangCannonHitTick) <
				static_cast<std::int32_t>(MAHARAKA_WATERPANG_CANNON_HIT_INTERVAL_TICKS)))
			return;
		constexpr float degreesToRadians = 3.14159265358979f / 180.f;
		const float jetX = std::sin(event.fCannonYawDegrees * degreesToRadians);
		const float jetZ = std::cos(event.fCannonYawDegrees * degreesToRadians);
		const float x = player.fPositionX - MAHARAKA_WATERPANG_CANNON_X;
		const float z = player.fPositionZ - MAHARAKA_WATERPANG_CANNON_Z;
		const float along = x * jetX + z * jetZ;
		const float across = x * jetZ - z * jetX;
		if (std::abs(along) > MAHARAKA_WATERPANG_CANNON_HALF_LENGTH_M + bodyRadius ||
			std::abs(across) > MAHARAKA_WATERPANG_CANNON_HALF_WIDTH_M + bodyRadius)
			return;
		// The jet carries the body away from the cannon along its own line.
		const float sign = along < 0.f ? -1.f : 1.f;
		hit.fSourceX = MAHARAKA_WATERPANG_CANNON_X;
		hit.fSourceZ = MAHARAKA_WATERPANG_CANNON_Z;
		hit.fPushDirectionX = jetX * sign;
		hit.fPushDirectionZ = jetZ * sign;
		hit.fPushRangeM = MAHARAKA_WATERPANG_CANNON_PUSH_M;
		hit.iPushMs = MAHARAKA_WATERPANG_CANNON_PUSH_MS;
		player.iWaterpangCannonHitTick = updateTick;
	}
	else
	{
		if (event.iElapsedTicks != MAHARAKA_WATERPANG_WATERFALL_HIT_TICK &&
			event.iElapsedTicks != MAHARAKA_WATERPANG_WATERFALL_SECOND_HIT_TICK) return;
		// The waterfall covers only the yellow centre disc; the ring of purple blocks, the pool, the shore and the piers are clear.
		const float x = player.fPositionX - MAHARAKA_WATERPANG_CANNON_X;
		const float z = player.fPositionZ - MAHARAKA_WATERPANG_CANNON_Z;
		const float radius = std::sqrt(x * x + z * z);
		if (radius > MAHARAKA_WATERPANG_WATERFALL_HIT_RADIUS_M || player.fPositionY < MAHARAKA_WATERPANG_DECK_MIN_Y_M)
			return;
		// Source waves: within 2.5 m of the origin 3.2 m ahead of the mokoko at 2.30 s, the rest at 2.46 s.
		float faceX = MAHARAKA_WATERPANG_CANNON_X - MAHARAKA_WATERPANG_MOKOMOKO_X;
		float faceZ = MAHARAKA_WATERPANG_CANNON_Z - MAHARAKA_WATERPANG_MOKOMOKO_Z;
		const float faceLength = std::sqrt(faceX * faceX + faceZ * faceZ);
		faceX /= faceLength; faceZ /= faceLength;
		const float originX = MAHARAKA_WATERPANG_MOKOMOKO_X + faceX * MAHARAKA_WATERPANG_WATERFALL_ORIGIN_FORWARD_M;
		const float originZ = MAHARAKA_WATERPANG_MOKOMOKO_Z + faceZ * MAHARAKA_WATERPANG_WATERFALL_ORIGIN_FORWARD_M;
		const bool innerWave = std::hypot(player.fPositionX - originX, player.fPositionZ - originZ) <=
			MAHARAKA_WATERPANG_WATERFALL_INNER_WAVE_M;
		if (innerWave != (event.iElapsedTicks == MAHARAKA_WATERPANG_WATERFALL_HIT_TICK)) return;
		// Away from the arena centre (on the cannon itself, along the mokoko's facing).
		float directionX = radius < 0.3f ? faceX : x / radius;
		float directionZ = radius < 0.3f ? faceZ : z / radius;
		constexpr float degreesToRadians = 3.14159265358979f / 180.f;
		float pierDegrees[2]{};
		std::uint32_t pierCount = 0u;
		for (const char* pierId : { "jump1", "jump2" })
			if (const auto* pier = Find_Placement(pierId))
				pierDegrees[pierCount++] = std::atan2(pier->fPositionX - MAHARAKA_WATERPANG_CANNON_X,
					pier->fPositionZ - MAHARAKA_WATERPANG_CANNON_Z) / degreesToRadians;
		/* The flight ends past the rim (|p + d * direction| >= exit radius) and never
		   over a jump pier: while its end bearing is within a pier's half angle, the
		   direction turns 2 degrees further away from that pier. */
		float distance = MAHARAKA_WATERPANG_WATERFALL_LAUNCH_M;
		for (std::uint32_t attempt = 0u; attempt < 60u; ++attempt)
		{
			const float along = x * directionX + z * directionZ;
			const float exitRadius = MAHARAKA_WATERPANG_WATERFALL_EXIT_RADIUS_M;
			distance = (std::max)(MAHARAKA_WATERPANG_WATERFALL_LAUNCH_M, -along + std::sqrt(
				(std::max)(0.f, along * along + exitRadius * exitRadius - radius * radius)));
			const float endDegrees = std::atan2(x + directionX * distance, z + directionZ * distance) / degreesToRadians;
			float turnDegrees = 0.f;
			for (std::uint32_t pier = 0u; pier < pierCount; ++pier)
			{
				const float offDegrees = std::remainder(endDegrees - pierDegrees[pier], 360.f);
				if (std::abs(offDegrees) < MAHARAKA_WATERPANG_PIER_HALF_ANGLE_DEGREES)
					turnDegrees = offDegrees < 0.f ? -2.f : 2.f;
			}
			if (0.f == turnDegrees) break;
			const float turned = std::atan2(directionX, directionZ) + turnDegrees * degreesToRadians;
			directionX = std::sin(turned);
			directionZ = std::cos(turned);
		}
		hit.fSourceX = MAHARAKA_WATERPANG_CANNON_X;
		hit.fSourceZ = MAHARAKA_WATERPANG_CANNON_Z;
		hit.fPushDirectionX = directionX;
		hit.fPushDirectionZ = directionZ;
		hit.fPushRangeM = distance;
		hit.iPushMs = MAHARAKA_WATERPANG_WATERFALL_LAUNCH_MS;
		hit.bPushBallistic = true;
		hit.bPushCanLeaveArena = true;
		hit.fPushHeightM = MAHARAKA_WATERPANG_WATERFALL_LAUNCH_HEIGHT_M;
	}
	// Both attacks only displace; the source rows carry no damage for this match.
	const auto result = CServerCombatHitRuntime::Apply_WorldToPlayer(player, hit, m_GameplayCatalog, m_TickDamageEvents);
	// Only a waterfall launch that armed its flight owns the forced Waterpang fall.
	if (MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL == event.eKind && SERVER_COMBAT_HIT_RESULT::LANDED == result &&
		player.bKnockbackBallistic && player.fKnockbackRemainingSeconds > 0.f)
		player.bWaterpangLaunch = true;
}

bool LostArk::Server::CGameRoom::Sample_MaharakaWaterpangNow(
	const std::uint32_t tick, LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& out) const
{
	using namespace LostArk::Shared;
	if (m_eWorldId != WORLD_ID::MAHARAKA) return false;
	// A running Debug forced event replaces the schedule; the Client samples the same way.
	MAHARAKA_WATERPANG_EVENT_KIND kind{};
	if (m_MaharakaWaterpangDebugEvent &&
		Find_MaharakaWaterpangDebugKind(m_MaharakaWaterpangDebugEvent->strSequenceInstanceId, kind) &&
		Sample_MaharakaWaterpangDebugEvent(kind, m_MaharakaWaterpangDebugEvent->iStartTick, tick, out))
		return true;
	return m_MaharakaWaterpangIntro && Sample_MaharakaWaterpangEvent(
		static_cast<std::int32_t>(tick - m_MaharakaWaterpangIntro->iStartTick),
		m_MaharakaWaterpangIntro->iStartTick, out);
}

bool LostArk::Server::CGameRoom::Is_MaharakaWaterpangDebugEventLive(const std::uint32_t tick) const
{
	using namespace LostArk::Shared;
	MAHARAKA_WATERPANG_EVENT_KIND kind{};
	if (!m_MaharakaWaterpangDebugEvent ||
		!Find_MaharakaWaterpangDebugKind(m_MaharakaWaterpangDebugEvent->strSequenceInstanceId, kind))
		return false;
	// Two seconds past the event cover a launch or push still in flight.
	const std::uint32_t liveTicks =
		Get_MaharakaWaterpangEventTicks(Make_MaharakaWaterpangDebugEvent(kind)) + 2u * MAHARAKA_WATERPANG_TICK_HZ;
	const auto elapsed = static_cast<std::int32_t>(tick - m_MaharakaWaterpangDebugEvent->iStartTick);
	return elapsed >= 0 && static_cast<std::uint32_t>(elapsed) < liveTicks;
}

LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT LostArk::Server::CGameRoom::Start_MaharakaWaterpangDebugEvent(
	const std::string& instanceId)
{
	using namespace LostArk::Shared;
	using Result = DEBUG_WORLD_PLAYBACK_RESULT;
#ifndef _DEBUG
	(void)instanceId;
	return Result::DISABLED;
#else
	MAHARAKA_WATERPANG_EVENT_KIND kind{};
	if (m_eWorldId != WORLD_ID::MAHARAKA || !Find_MaharakaWaterpangDebugKind(instanceId, kind))
		return Result::INVALID_TARGET;
	/* The countdown and the intro cutscene own the cast until the first scheduled
	   event; a forced event there would fight the intro poses and camera. */
	if (m_MaharakaWaterpangIntro && !Has_ReachedServerTick(m_iServerTick, Add_ServerTicksSkippingReservedZero(
		m_MaharakaWaterpangIntro->iStartTick, MAHARAKA_WATERPANG_SCHEDULE.front().iStartSeconds * MAHARAKA_WATERPANG_TICK_HZ)))
		return Result::ACTION_REJECTED;
	S2C_WORLD_SEQUENCE_PLAY message{};
	message.eOperation = WORLD_SEQUENCE_OPERATION::PLAY;
	message.strSequenceInstanceId = instanceId;
	message.iServerTick = m_iServerTick;
	message.iStartTick = Add_ServerTicksSkippingReservedZero(m_iServerTick, MAHARAKA_WATERPANG_DEBUG_LEAD_TICKS);
	CPacketWriter writer;
	if (!Write_Message(writer, message)) return Result::ACTION_REJECTED;
	// A new press replaces the running forced event; every Client restarts on the new occurrence.
	m_MaharakaWaterpangDebugEvent = message;
	for (const auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(player.iSessionId);
		if (nullptr != session && !session->Send_Frame(PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, writer.Get_Buffer()))
			session->Request_Close();
	}
	return Result::ACCEPTED;
#endif
}

bool LostArk::Server::CGameRoom::Is_MaharakaWaterpangArmed(const SERVER_PLAYER& player) const
{
	using namespace LostArk::Shared;
	// Armed from the intro start tick while the body stands on the Waterpang arena region; an arena
	// push-off or knockdown keeps it until the fall resolves. The empty-room reset clears the match.
	return WORLD_ID::MAHARAKA == m_eWorldId && m_MaharakaWaterpangIntro &&
		Has_ReachedServerTick(m_iServerTick, m_MaharakaWaterpangIntro->iStartTick) &&
		(player.bWaterpangFall || PLAYER_ACTION_STATE::KNOCKDOWN == player.eAction ||
		 (m_ServerNavigation.Is_Loaded() &&
		  Is_MaharakaWaterpangArenaFootprint(player.fPositionX, player.fPositionZ) &&
		  m_ServerNavigation.Is_PointWalkableInRegion(
			MAHARAKA_WATERPANG_REGION_ID, player.fPositionX, player.fPositionZ, player.fPositionY)));
}

bool LostArk::Server::CGameRoom::Try_StartMaharakaWaterGunSkill(
	SERVER_PLAYER& player, const LostArk::Shared::C2S_USE_SKILL& command)
{
	using namespace LostArk::Shared;
	const MAHARAKA_WATERGUN_SKILL* const skill = Find_MaharakaWaterGunSkill(command.iSkillId);
	// Movement is never locked, so only states that own the body's action refuse a cast.
	if (nullptr == skill || !Is_MaharakaWaterpangArmed(player) || 0u == player.iCurrentHp ||
		PLAYER_ACTION_STATE::NONE != player.eAction || player.fKnockbackRemainingSeconds > 0.f ||
		player.bWaterpangFall || player.bPatternBound || player.TriggerMove.isActive ||
		!Is_NewerSequence(command.iClientSequence, player.iLastSkillSequence) ||
		(0u != player.iSilenceEndTick && !Has_ReachedServerTick(m_iServerTick, player.iSilenceEndTick)))
		return false;
	const std::uint32_t startTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
	// One action at a time: the previous cast's clip must have run out.
	if (0u != player.iWaterGunCastEndTick &&
		static_cast<std::int32_t>(player.iWaterGunCastEndTick - startTick) > 0)
		return false;
	const auto cooldown = player.CooldownEndTickBySkillId.find(skill->iSkillId);
	if (cooldown != player.CooldownEndTickBySkillId.end() &&
		static_cast<std::int32_t>(cooldown->second - startTick) > 0)
		return false;

	player.iLastSkillSequence = command.iClientSequence;
	player.iWaterGunSkillId = skill->iSkillId;
	player.iWaterGunCastTick = startTick;
	player.iWaterGunCastEndTick = Add_ServerTicksSkippingReservedZero(
		startTick, Get_MaharakaWaterGunTicks(skill->iActionMs));
	if (0u != skill->iCooldownMs)
		player.CooldownEndTickBySkillId.insert_or_assign(skill->iSkillId,
			Add_ServerTicksSkippingReservedZero(startTick, Get_MaharakaWaterGunTicks(skill->iCooldownMs)));
	if (MAHARAKA_WATERGUN_KIND::SPEED_BUFF == skill->eKind)
	{
		player.iWaterGunSpeedEndTick = Add_ServerTicksSkippingReservedZero(
			startTick, Get_MaharakaWaterGunTicks(skill->iBuffMs));
		player.fWaterGunSpeedScale = skill->fBuffSpeedScale;
		return true;
	}
	// A standing body turns to the aim; a walking one keeps the facing of its path.
	const float aimX = command.fAimX - player.fPositionX;
	const float aimZ = command.fAimZ - player.fPositionZ;
	const float aimDistance = std::sqrt(aimX * aimX + aimZ * aimZ);
	if (!player.hasMoveGoal && aimDistance > 0.1f)
		player.fYawDegrees = std::atan2(aimX, aimZ) * RADIANS_TO_DEGREES;
	MAHARAKA_WATERGUN_SHOT shot{};
	shot.iOwnerId = player.iPlayerId;
	shot.iSkillId = skill->iSkillId;
	shot.iSourceNetEntityId = player.iNetEntityId;
	shot.iSpawnTick = Add_ServerTicksSkippingReservedZero(startTick, Get_MaharakaWaterGunTicks(skill->iSpawnMs));
	shot.fAimDistanceM = (std::min)(skill->fMaxRangeM, (std::max)(aimDistance, 1.f));
	m_MaharakaWaterGunShots.push_back(std::move(shot));
	return true;
}

void LostArk::Server::CGameRoom::Update_MaharakaWaterGunShots(const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if (m_MaharakaWaterGunShots.empty())
		return;
	if (WORLD_ID::MAHARAKA != m_eWorldId)
	{
		m_MaharakaWaterGunShots.clear();
		return;
	}
	constexpr float fixedDeltaSeconds = 1.f / static_cast<float>(MAHARAKA_WATERPANG_TICK_HZ);
	const float bodyRadius = WorldCollision::PLAYER_HALF_EXTENT_X;
	for (MAHARAKA_WATERGUN_SHOT& shot : m_MaharakaWaterGunShots)
	{
		const MAHARAKA_WATERGUN_SKILL* const skill = Find_MaharakaWaterGunSkill(shot.iSkillId);
		const auto ownerIter = m_Players.find(shot.iOwnerId);
		if (nullptr == skill || m_Players.end() == ownerIter)
		{
			shot.bSpent = true;
			continue;
		}
		if (!shot.bLaunched)
		{
			if (!Has_ReachedServerTick(updateTick, shot.iSpawnTick))
				continue;
			const SERVER_PLAYER& owner = ownerIter->second;
			// A shooter who fell or died before the muzzle notify never fires.
			if (0u == owner.iCurrentHp || PLAYER_ACTION_STATE::DEAD == owner.eAction ||
				PLAYER_ACTION_STATE::FALLING == owner.eAction || owner.bWaterpangFall)
			{
				shot.bSpent = true;
				continue;
			}
			const float yawRadians = owner.fYawDegrees / RADIANS_TO_DEGREES;
			shot.bLaunched = true;
			shot.fDirX = std::sin(yawRadians);
			shot.fDirZ = std::cos(yawRadians);
			shot.fX = owner.fPositionX;
			shot.fY = owner.fPositionY;
			shot.fZ = owner.fPositionZ;
            if (MAHARAKA_WATERGUN_KIND::GRENADE != skill->eKind)
            {
                shot.fX += shot.fDirX * .70f + shot.fDirZ * .11f;
                shot.fZ += shot.fDirZ * .70f - shot.fDirX * .11f;
            }
			shot.fReachM = MAHARAKA_WATERGUN_KIND::GRENADE == skill->eKind ?
				shot.fAimDistanceM : skill->fSpeedMps * skill->fLifeSeconds;
            auto transaction = m_CombatObjectRuntime.Begin_Transaction();
            std::string status;
            if (m_CombatObjectRuntime.Stage_WaterGunPresentation(transaction, owner,
                shot.iSkillId, m_GameplayCatalog, shot.iSpawnTick, status))
            {
                const auto visualId = transaction.Objects.back().iCombatObjectId;
                if (m_CombatObjectRuntime.Commit(std::move(transaction))) shot.iVisualObjectId = visualId;
            }
		}
		const float step = (std::min)(skill->fSpeedMps * fixedDeltaSeconds, shot.fReachM - shot.fTravelM);
		if (step > 0.f)
		{
			shot.fX += shot.fDirX * step;
			shot.fZ += shot.fDirZ * step;
			shot.fTravelM += step;
		}
        if (shot.iVisualObjectId)
        {
            float visualY = shot.fY + .75f;
            if (MAHARAKA_WATERGUN_KIND::GRENADE == skill->eKind)
            {
                // Source grenade height is 60..90 cm, apex ratio .4. The room
                // supplies a bounded piecewise parabola; collision remains at ground arrival.
                const float progress = shot.fReachM > .0001f ? std::clamp(shot.fTravelM / shot.fReachM, 0.f, 1.f) : 1.f;
                const float u = progress <= .4f ? progress / .4f : (1.f - progress) / .6f;
                visualY = shot.fY + .75f * (2.f * u - u * u);
            }
            (void)m_CombatObjectRuntime.Set_OwnedVisualPosition(shot.iVisualObjectId,
                shot.iSourceNetEntityId, shot.iSpawnTick, shot.fX, visualY, shot.fZ);
        }
		const bool grenade = MAHARAKA_WATERGUN_KIND::GRENADE == skill->eKind;
		const bool landed = shot.fTravelM + 0.0001f >= shot.fReachM;
		// A thrown bomb only bursts where it lands; a missile touches bodies along the way.
		if (!grenade || landed)
		{
			const float reach = skill->fHitRadiusM + bodyRadius;
			for (auto& [targetId, target] : m_Players)
			{
				if (targetId == shot.iOwnerId || 0u == target.iCurrentHp || !target.isCombatReady ||
					!Is_MaharakaWaterpangArmed(target) ||
					std::find(shot.Struck.begin(), shot.Struck.end(), targetId) != shot.Struck.end() ||
					std::abs(target.fPositionY - shot.fY) > MAHARAKA_WATERGUN_HIT_HEIGHT_M)
					continue;
				const float deltaX = target.fPositionX - shot.fX;
				const float deltaZ = target.fPositionZ - shot.fZ;
				if (deltaX * deltaX + deltaZ * deltaZ > reach * reach)
					continue;
				SERVER_WORLD_TO_PLAYER_HIT hit{};
				hit.iServerTick = updateTick;
				hit.bIgnoreCounter = true;
				hit.fSourceX = shot.fX;
				hit.fSourceZ = shot.fZ;
                // User-selected Kouku laser knockback, shared by humans and bots.
                hit.fPushRangeM = m_MaharakaAITuning.fKnockbackRangeM;
                hit.iPushMs = m_MaharakaAITuning.iKnockbackMs;
                hit.bForcePush = true;
				// A missile carries the body along its own line; a burst pushes away from its centre.
				hit.bUsePushDirection = !grenade;
				hit.fPushDirectionX = shot.fDirX;
				hit.fPushDirectionZ = shot.fDirZ;
				// The source rows carry no usable HP damage for this match (see the contract).
				(void)CServerCombatHitRuntime::Apply_WorldToPlayer(target, hit, m_GameplayCatalog, m_TickDamageEvents);
				shot.Struck.push_back(targetId);
				if (!grenade)
				{
					shot.bSpent = true;
					break;
				}
			}
		}
		if (landed)
			shot.bSpent = true;
        if (shot.bSpent && shot.iVisualObjectId)
        {
            const bool impact = grenade ? landed : !shot.Struck.empty();
            (void)m_CombatObjectRuntime.Finish_WaterGunPresentation(shot.iVisualObjectId,
                shot.iSourceNetEntityId, shot.iSpawnTick, updateTick, impact);
            shot.iVisualObjectId = INVALID_COMBAT_OBJECT_ID;
        }
	}
    for (const auto& shot : m_MaharakaWaterGunShots)
        if (shot.bSpent && shot.iVisualObjectId)
            (void)m_CombatObjectRuntime.Cancel_OwnedVisualObject(shot.iVisualObjectId,
                shot.iSourceNetEntityId, shot.iSpawnTick);
	m_MaharakaWaterGunShots.erase(
		std::remove_if(m_MaharakaWaterGunShots.begin(), m_MaharakaWaterGunShots.end(),
			[](const MAHARAKA_WATERGUN_SHOT& shot) { return shot.bSpent; }),
		m_MaharakaWaterGunShots.end());
}
