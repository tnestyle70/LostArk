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

bool LostArk::Server::CGameRoom::Send_Accepted(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	S2C_ENTER_ACCEPTED message{};
	message.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	message.eWorldId = m_eWorldId;
	message.iPlayerId = player.iPlayerId;
	message.iNetEntityId = player.iNetEntityId;
	message.ActiveGameplayRevision =
		m_GameplayCatalog.Get_ActiveRevision();
	if (!Build_RequiredPinnedGameplayRevisions(
		message.RequiredPinnedGameplayRevisions))
	{
		return false;
	}
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_ENTER_ACCEPTED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_EnterRejected(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason)
{
	using namespace LostArk::Shared;
	S2C_ENTER_REJECTED message{};
	message.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	message.eWorldId = m_eWorldId;
	message.eReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_ENTER_REJECTED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_Spawned(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	S2C_PLAYER_SPAWNED message{};
	message.iPlayerId = player.iPlayerId;
	message.iNetEntityId = player.iNetEntityId;
	message.eCharacterClass = player.eCharacterClass;
	message.strNickName = player.strNickName;
	message.fPositionX = player.fPositionX;
	message.fPositionY = player.fPositionY;
	message.fPositionZ = player.fPositionZ;
	message.fYawDegrees = player.fYawDegrees;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_PLAYER_SPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_WorldEntitySpawned(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_WORLD_ENTITY& entity)
{
	std::vector<std::uint8_t> payload;
	return nullptr != session &&
		Build_WorldEntitySpawnedPayload(entity, payload) &&
		session->Send_Frame(
			LostArk::Shared::PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED, payload);
}

bool LostArk::Server::CGameRoom::Build_WorldEntitySpawnedPayload(
	const SERVER_WORLD_ENTITY& entity,
	std::vector<std::uint8_t>& outPayload)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_SPAWNED message{};
	message.iNetEntityId = entity.iNetEntityId;
	message.iOwnerBossNetEntityId = entity.iOwnerBossNetEntityId;
	message.eKind = To_NetworkKind(entity.eKind);
	message.strArchetypeId = entity.strArchetypeId;
	message.strEncounterId = entity.strEncounterId;
	message.strPlacementId = entity.strPlacementId;
	message.strActionId = entity.strActionId;
	message.fPositionX = entity.fPositionX;
	message.fPositionY = entity.fPositionY;
	message.fPositionZ = entity.fPositionZ;
	message.fYawDegrees = entity.fYawDegrees;
	message.fCollisionRadius = entity.fCollisionRadius;
	message.PinnedDefinitionRevision = entity.PinnedDefinitionRevision;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return false;
	outPayload = writer.Get_Buffer();
	return true;
}

bool LostArk::Server::CGameRoom::Send_WorldEntityDespawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_DESPAWNED message{};
	message.iNetEntityId = netEntityId;
	message.eReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_ENTITY_DESPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_CombatObjectSpawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned)
{
	using namespace LostArk::Shared;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, spawned) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_Despawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	using namespace LostArk::Shared;
	S2C_PLAYER_DESPAWNED message{};
	message.iNetEntityId = netEntityId;
	message.eReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_PLAYER_DESPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_WorldDestructionFullSync(
	const std::shared_ptr<CClientSession>& session)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId)
		return true;
	if (nullptr == session || !m_WorldDestructionRuntime.Is_Initialized())
		return false;

	S2C_WORLD_DESTRUCTION_FULL_SYNC message{};
	message.strCombatRuntimeRevision =
		m_WorldDestructionBootstrap.Get_CombatRuntimeRevision();
	message.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
	message.iEncounterEpoch = m_WorldDestructionRuntime.Get_EncounterEpoch();
	for (const WORLD_DESTRUCTION_GROUP_STATE& state :
		m_WorldDestructionRuntime.Get_GroupStates())
	{
		message.GroupStates.push_back(To_NetworkDestructionState(state));
	}
	message.Diagnostics = Build_WorldDestructionDiagnostics();
	CPacketWriter writer;
	return Write_Message(writer, message) && session->Send_Frame(
		PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC, writer.Get_Buffer());
}

LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
LostArk::Server::CGameRoom::Build_WorldDestructionDiagnostics() const
{
	LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS diagnostics{};
	diagnostics.iActiveWallCollisionCount = static_cast<std::uint32_t>(
		m_ServerCollisionSystem.Get_ActivePlayerBlockingCount());
	diagnostics.iActiveNavBlockerRegionCount = static_cast<std::uint32_t>(
		m_ServerNavigation.Get_ActiveBlockerRegionCount());
	diagnostics.iNavigationRevision = m_ServerNavigation.Get_Revision();
	diagnostics.iLastEventSequence =
		0u == m_iNextWorldDestructionEventSequence ?
		0u : m_iNextWorldDestructionEventSequence - 1u;
	return diagnostics;
}

void LostArk::Server::CGameRoom::Broadcast_Spawned(
	const SERVER_PLAYER& player,
	const SESSION_ID exceptSessionId)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		if (sessionId == exceptSessionId)
			continue;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_Spawned(session, player))
			session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Broadcast_Despawned(
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_Despawned(session, netEntityId, reason))
			session->Request_Close();
	}
}

bool LostArk::Server::CGameRoom::Send_WorldEntitySpawnResult(
	const std::shared_ptr<CClientSession>& session,
	const std::string& placementId,
	const LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT result,
	const LostArk::Shared::NET_ENTITY_ID netEntityId)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_SPAWN_RESULT message{};
	message.strPlacementId = placementId;
	message.eResult = result;
	message.iNetEntityId = netEntityId;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_ENTITY_SPAWN_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_InventorySnapshot(
	const std::shared_ptr<CClientSession>& session,
	const std::uint32_t requestSequence,
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& inventory)
{
	using namespace LostArk::Shared;
	S2C_INVENTORY_SNAPSHOT message{};
	message.iRequestSequence = requestSequence;
	message.Items = inventory;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_INVENTORY_SNAPSHOT, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_ValtanAuditionResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
	const LostArk::Shared::VALTAN_AUDITION_RESULT result,
	const std::uint32_t currentHealthBar)
{
	using namespace LostArk::Shared;
	S2C_VALTAN_AUDITION_RESULT message{};
	message.iRequestSequence = request.iRequestSequence;
	message.eOperation = request.eOperation;
	message.iTargetHealthBar = request.iTargetHealthBar;
	message.eResult = result;
	message.iCurrentHealthBar = currentHealthBar;
	message.strBossPlacementId = request.strBossPlacementId;
	message.strPatternId = request.strPatternId;
	message.iPredecessorRoomAuditionEpoch = request.iPredecessorRoomAuditionEpoch;
	message.iPredecessorPatternSequence = request.iPredecessorPatternSequence;
	message.iExpectedNextRequestSequence = request.iExpectedNextRequestSequence;
	message.ExpectedDefinitionRevision = request.ExpectedDefinitionRevision;
	message.ReplacementDefinitionRevision =
		request.ReplacementDefinitionRevision;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_VALTAN_AUDITION_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_KoukuSaydonPatternAuditionResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::
		S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& message)
{
	using namespace LostArk::Shared;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_ValtanPatternFlowResult(
	const std::shared_ptr<CClientSession>& session,
	const std::uint32_t commandSequence,
	const LostArk::Shared::VALTAN_PATTERN_FLOW_COMMAND command,
	const LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT result,
	const std::string& flowId,
	const std::string& flowRevision,
	const std::uint32_t roomFlowEpoch,
	const LostArk::Shared::GameplayDataRevision& pinnedRevision,
	const std::string& reason)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT message{};
	message.iCommandSequence = commandSequence;
	message.eCommand = command;
	message.eResult = result;
	message.strFlowId = flowId;
	message.strFlowRevision = flowRevision;
	message.iRoomFlowEpoch = roomFlowEpoch;
	message.PinnedDefinitionRevision = pinnedRevision;
	message.strReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_CharacterClassChangeResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request,
	const LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT result,
	const LostArk::Shared::CHARACTER_CLASS_ID activeClass)
{
	using namespace LostArk::Shared;
	S2C_CHARACTER_CLASS_CHANGE_RESULT message{};
	message.iClientSequence = request.iClientSequence;
	message.eResult = result;
	message.eRequestedClass = request.eCharacterClass;
	message.eActiveClass = activeClass;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_CHARACTER_CLASS_CHANGE_RESULT,
			writer.Get_Buffer());
}

void LostArk::Server::CGameRoom::Broadcast_WorldEntitySpawned(
	const SERVER_WORLD_ENTITY& entity)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_WorldEntitySpawned(session, entity))
			session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Broadcast_WorldEntityDespawned(
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session &&
			!Send_WorldEntityDespawned(session, netEntityId, reason))
		{
			session->Request_Close();
		}
	}
}

bool LostArk::Server::CGameRoom::Broadcast_CombatObjectLifecycle()
{
	using namespace LostArk::Shared;
	std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
	std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> presentationEvents;
	std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
	m_CombatObjectRuntime.Drain_Lifecycle(
		spawned, presentationEvents, despawned);
	for (const S2C_COMBAT_OBJECT_SPAWNED& message : spawned)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
		{
			(void)playerId;
			const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
			if (nullptr != session && !session->Send_Frame(
				PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED, writer.Get_Buffer()))
			{
				session->Request_Close();
			}
		}
	}
	for (const S2C_COMBAT_OBJECT_PRESENTATION_EVENT& message :
		presentationEvents)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
		{
			(void)playerId;
			const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
			if (nullptr != session && !session->Send_Frame(
				PACKET_TYPE::S2C_COMBAT_OBJECT_PRESENTATION_EVENT,
				writer.Get_Buffer()))
			{
				session->Request_Close();
			}
		}
	}
	for (const S2C_COMBAT_OBJECT_DESPAWNED& message : despawned)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
		{
			(void)playerId;
			const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
			if (nullptr != session && !session->Send_Frame(
				PACKET_TYPE::S2C_COMBAT_OBJECT_DESPAWNED, writer.Get_Buffer()))
			{
				session->Request_Close();
			}
		}
	}
	return true;
}

bool LostArk::Server::CGameRoom::Broadcast_WorldDestructionDelta(
	const std::vector<WORLD_DESTRUCTION_STATE_TRANSITION>& transitions,
	const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>& liveEvents,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (transitions.empty() && liveEvents.empty())
		return true;

	S2C_WORLD_DESTRUCTION_DELTA message{};
	message.strCombatRuntimeRevision =
		m_WorldDestructionBootstrap.Get_CombatRuntimeRevision();
	message.iServerTick = serverTick;
	message.iEncounterEpoch = m_WorldDestructionRuntime.Get_EncounterEpoch();
	for (const WORLD_DESTRUCTION_STATE_TRANSITION& transition : transitions)
	{
		WORLD_DESTRUCTION_GROUP_STATE state{};
		if (!m_WorldDestructionRuntime.Find_GroupState(
			transition.strGroupId, state))
		{
			return false;
		}
		message.ChangedStates.push_back(To_NetworkDestructionState(state));
	}
	std::sort(message.ChangedStates.begin(), message.ChangedStates.end(),
		[](const WORLD_DESTRUCTION_STATE_WIRE& left,
			const WORLD_DESTRUCTION_STATE_WIRE& right)
		{
			return left.strGroupId < right.strGroupId;
		});
	message.LiveEvents = liveEvents;
	message.Diagnostics = Build_WorldDestructionDiagnostics();

	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return false;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_DESTRUCTION_DELTA, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
	return true;
}

void LostArk::Server::CGameRoom::Broadcast_WorldSnapshot()
{
	using namespace LostArk::Shared;
	S2C_WORLD_SNAPSHOT message{};
	message.iServerTick = m_iServerTick;
	message.eWorldId = m_eWorldId;
	message.iEstherGauge = m_EstherSkillSystem.Get_Gauge();
	message.iEstherGaugeMaximum = m_EstherSkillSystem.Get_GaugeMaximum();
	message.ActiveGameplayRevision =
		m_GameplayCatalog.Get_ActiveRevision();
	if (!Build_RequiredPinnedGameplayRevisions(
		message.RequiredPinnedGameplayRevisions))
	{
		m_strStatus = "Live gameplay revision pins are invalid or exceed wire bounds";
		return;
	}
	message.Players.reserve(m_Players.size());
	message.Entities.reserve(m_WorldEntities.size());
	for (const auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		PLAYER_SNAPSHOT snapshot{};
		snapshot.iNetEntityId = player.iNetEntityId;
		snapshot.eCharacterClass = player.eCharacterClass;
		snapshot.fPositionX = player.fPositionX;
		snapshot.fPositionY = player.fPositionY;
		snapshot.fPositionZ = player.fPositionZ;
		snapshot.fYawDegrees = player.fYawDegrees;
		snapshot.iLastProcessedMoveSequence = player.iLastMoveSequence;
		snapshot.fMoveSpeed = player.fMoveSpeed * Resolve_StanceMoveSpeedScale(player);
		snapshot.canPredictMove =
			PLAYER_ACTION_STATE::NONE == player.eAction &&
			player.iCurrentHp != 0u && !player.bPatternBound &&
			player.iMarioStage == 0u && !player.TriggerMove.isActive &&
			player.fKnockbackRemainingSeconds <= 0.f &&
			player.CardMaze.transferStartTick == 0u &&
			(!(player.CardMaze.flags & 1u) ||
			 m_KoukuCardMaze.Is_SoloHunter(player.iPlayerId));
#ifdef _DEBUG
		if (WORLD_ID::VALTAN_ARENA == m_eWorldId &&
			VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE != m_ValtanTimelineAudition.ePhase &&
			!(VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH ==
				m_ValtanTimelineAudition.ePhase &&
			  player.iPlayerId == m_ValtanTimelineAudition.iOwnerPlayerId))
		{
			snapshot.canPredictMove = false;
		}
#endif
		snapshot.hasMoveGoal = snapshot.canPredictMove && player.hasMoveGoal;
		if (snapshot.hasMoveGoal)
		{
			const SERVER_NAV_POINT waypoint = player.iMovePathIndex < player.MovePath.size() ?
				player.MovePath[player.iMovePathIndex] :
				SERVER_NAV_POINT{ player.fMoveGoalX, player.fPositionY, player.fMoveGoalZ };
			snapshot.fMoveWaypointX = waypoint.x;
			snapshot.fMoveWaypointY = waypoint.y;
			snapshot.fMoveWaypointZ = waypoint.z;
		}
		snapshot.eLocomotionState =
			(player.hasMoveGoal || player.TriggerMove.isActive) ?
			PLAYER_LOCOMOTION_STATE::MOVING : PLAYER_LOCOMOTION_STATE::IDLE;
		snapshot.eAction = player.eAction;
		snapshot.eStance = player.eStance;
		snapshot.iSkillId = player.iCurrentSkillId;
		snapshot.iActionStartTick = player.iActionStartTick;
		if (PLAYER_ACTION_STATE::GRABBED == player.eAction)
		{
			snapshot.iAttachmentOwnerNetEntityId =
				player.iAttachmentOwnerNetEntityId;
			snapshot.eAttachmentSlot = player.eAttachmentSlot;
			snapshot.fAttachmentLocalOffsetX =
				player.fAttachmentLocalOffsetX;
			snapshot.fAttachmentLocalOffsetY =
				player.fAttachmentLocalOffsetY;
			snapshot.fAttachmentLocalOffsetZ =
				player.fAttachmentLocalOffsetZ;
			snapshot.fAttachmentYawOffsetDegrees =
				player.fAttachmentYawOffsetDegrees;
		}
		if (PLAYER_ACTION_STATE::SKILL == player.eAction &&
			player.hasSkillTarget)
		{
			snapshot.hasSkillTarget = true;
			snapshot.fSkillTargetX = player.fSkillTargetX;
			snapshot.fSkillTargetY = player.fSkillTargetY;
			snapshot.fSkillTargetZ = player.fSkillTargetZ;
		}
		snapshot.iCurrentHp = player.iCurrentHp;
		snapshot.iMaximumHp = player.iMaximumHp;
		snapshot.iCurrentResource = player.iCurrentResource;
		snapshot.iMaximumResource = player.iMaximumResource;
		snapshot.iCurrentIdentity = player.iCurrentIdentity;
		snapshot.iMaximumIdentity = player.iMaximumIdentity;
		snapshot.iCurrentMadness = player.iCurrentMadness;
		snapshot.iMaximumMadness = player.iMaximumMadness;
		snapshot.eMadnessForm = player.eMadnessForm;
		snapshot.eMechanicCardSymbol = player.eMechanicCardSymbol;
		snapshot.eMechanicCardColor = player.eMechanicCardColor;
		snapshot.eKoukuHudMode = player.eKoukuHudMode;
		for (std::size_t slot = 0u; slot < KOUKU_HUD_SLOT_COUNT; ++slot)
			snapshot.ModeSkillIndexBySlot[slot] = player.ModeSkillIndexBySlot[slot];
		snapshot.iMarioStage = player.iMarioStage;
		snapshot.iMarioLayoutVariant = player.iMarioLayoutVariant;
		snapshot.iMarioPoppedBallMask = player.iMarioStage >= 1u && player.iMarioStage <= 4u ?
			m_MarioPoppedBalls[player.iMarioStage] : std::uint16_t{};
		snapshot.iMarioCurseReleasedMask = Mario_CurseReleasedMask(player.iMarioStage, player.iMarioLayoutVariant);
		snapshot.eCardMazeRole = player.eCardMazeRole;
		snapshot.eCardMazeSuit = player.eCardMazeSuit;
		snapshot.iCardMazeKills = player.iCardMazeKills;
		snapshot.iCardMazeKillTarget = player.iCardMazeKillTarget;
		snapshot.CardMaze = player.CardMaze;
		snapshot.isCombatReady = player.isCombatReady;
		snapshot.isPatternBound = player.bPatternBound;
		snapshot.iPatternBindEndTick = player.iPatternBindEndTick;
        if (PLAYER_ACTION_STATE::FEAR == player.eAction)
        {
            snapshot.iFearEndTick = player.iFearEndTick;
            snapshot.strFearPresentationId = player.strFearPresentationId;
        }
		snapshot.iSilenceEndTick = player.iSilenceEndTick;
		snapshot.iSilenceDurationTicks = player.iSilenceDurationTicks;
		snapshot.iComboStage = player.iComboStage;
		/* Collect, sort, then truncate: cutting during unordered_map iteration
		made the surviving cooldowns depend on hash order. Signed difference keeps
		ordering across a wrapped tick counter. */
		for (const auto& [skillId, cooldownEndTick] :
			player.CooldownEndTickBySkillId)
		{
			if (static_cast<std::int32_t>(cooldownEndTick - m_iServerTick) > 0)
				snapshot.Cooldowns.push_back({ skillId, cooldownEndTick });
		}
		std::sort(snapshot.Cooldowns.begin(), snapshot.Cooldowns.end(),
			[](const SKILL_COOLDOWN_SNAPSHOT& left,
				const SKILL_COOLDOWN_SNAPSHOT& right)
			{
				return left.iSkillId < right.iSkillId;
			});
		if (snapshot.Cooldowns.size() > MAX_PLAYER_COOLDOWNS)
			snapshot.Cooldowns.resize(MAX_PLAYER_COOLDOWNS);
		message.Players.push_back(snapshot);
	}
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		WORLD_ENTITY_SNAPSHOT snapshot{};
		snapshot.iNetEntityId = entity.iNetEntityId;
		snapshot.eAction = To_NetworkAction(entity.eAction);
		snapshot.strPatternId = entity.strPatternId;
		if (entity.eKind == WORLD_BOOTSTRAP_KIND::NPC ||
			entity.eAction == SERVER_ENTITY_ACTION::PATTERN_WINDUP ||
			entity.eAction == SERVER_ENTITY_ACTION::PATTERN_ACTIVE ||
			entity.eAction == SERVER_ENTITY_ACTION::PATTERN_RECOVERY)
		{
			snapshot.strActionId = entity.strActionId;
		}
		snapshot.fPositionX = entity.fPositionX;
		snapshot.fPositionY = entity.fPositionY;
		snapshot.fPositionZ = entity.fPositionZ;
		snapshot.fYawDegrees = entity.fYawDegrees;
		snapshot.iActionStartTick = entity.iActionStartTick;
		snapshot.iPatternSequence = entity.iPatternSequence;
		snapshot.iPatternStartTick = entity.iPatternStartTick;
		snapshot.iPatternStageIndex = entity.iPatternStageIndex;
		snapshot.iCurrentHp = entity.iCurrentHp;
		snapshot.iMaximumHp = entity.iMaximumHp;
		snapshot.iPhase = entity.iPhase;
		snapshot.PinnedDefinitionRevision =
			entity.PinnedDefinitionRevision;
		/* Presentation only needs to know which plates came off, not how much
		durability is left, so the wire carries one bit per authored plate. */
		for (const SERVER_BOSS_ARMOR_PLATE_STATE& plate : entity.ArmorPlates)
		{
			if (0u == plate.iRemainingDurability &&
				plate.iPlateIndex <
					LostArk::Shared::MAX_WORLD_ENTITY_ARMOR_PLATES)
			{
				snapshot.iBrokenArmorMask |= static_cast<std::uint8_t>(
					1u << plate.iPlateIndex);
			}
		}
		if (WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind)
		{
			snapshot.iPatternTargetNetEntityId =
				entity.iPatternTargetEntityId;
			if (entity.bPortalMotionActive &&
				entity.bPortalRushTargetLocked &&
				BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH ==
					entity.ePatternStageMotionKind)
			{
				snapshot.PortalRushRoute.isValid = true;
				snapshot.PortalRushRoute.fStartX = entity.fPortalStartX;
				snapshot.PortalRushRoute.fStartY = entity.fSpawnPositionY;
				snapshot.PortalRushRoute.fStartZ = entity.fPortalStartZ;
				snapshot.PortalRushRoute.fEndX = entity.fPortalEndX;
				snapshot.PortalRushRoute.fEndY = entity.fSpawnPositionY;
				snapshot.PortalRushRoute.fEndZ = entity.fPortalEndZ;
			}
			snapshot.hasBossCombatState = true;
			snapshot.BossCombat.iStateRevision =
				entity.BossCombat.iStateRevision;
			snapshot.BossCombat.iAlivePartMask =
				entity.BossCombat.iAlivePartMask;
			snapshot.BossCombat.iFlags = static_cast<std::uint16_t>(
				entity.BossCombat.iFlags) &
				BOSS_COMBAT_STATE_KNOWN_FLAG_MASK;
			snapshot.BossCombat.iCurrentStagger =
				entity.BossCombat.iStaggerCurrent;
			snapshot.BossCombat.iMaximumStagger =
				entity.BossCombat.iStaggerMaximum;
			snapshot.BossCombat.iCurrentShield =
				entity.BossCombat.iShieldCurrent;
			snapshot.BossCombat.iMaximumShield =
				entity.BossCombat.iShieldMaximum;
			if (BOSS_PATTERN_BOSS_RESPONSE_KIND::ACCUMULATED_HEALTH_DAMAGE ==
				entity.ePatternBossResponseKind)
			{
				snapshot.BossCombat.iResponseThreshold =
					entity.iPatternBossResponseThreshold;
				snapshot.BossCombat.iResponseProgress = (std::min)(
					entity.iPatternBossResponseAccumulatedHealthDamage,
					entity.iPatternBossResponseThreshold);
			}
			/* The existing gameplay phase remains the one authority. The boss
			payload mirrors it rather than introducing a second phase clock. */
			snapshot.BossCombat.iGameplayPhase = entity.iPhase;
		}
		message.Entities.push_back(std::move(snapshot));
	}
	message.DamageEvents = m_TickDamageEvents;
	message.Bingo.iWhiteMask = m_KoukuBingo.Get_WhiteMask();
	message.Bingo.iRedMask = m_KoukuBingo.Get_RedMask();
	message.Bingo.Hammer = m_KoukuBingo.Get_Hammer();
	message.Bingo.iBombCount = 0u;
	for (const auto& bomb : m_KoukuBingo.Get_Bombs())
	{
		if (LostArk::Shared::BINGO_BOMB_PHASE::NONE == bomb.ePhase)
			continue;
		auto& packed = message.Bingo.Bombs[message.Bingo.iBombCount++];
		packed.ePhase = bomb.ePhase;
		packed.iCarrierNetEntityId = bomb.iCarrierNetEntityId;
		packed.fPositionX = bomb.fPositionX;
		packed.fPositionZ = bomb.fPositionZ;
	}
	message.BossCombatEvents = m_TickBossCombatEvents;
	if (!m_CombatObjectRuntime.Build_Snapshots(message.CombatObjects))
		return;

	const auto encodeStart = std::chrono::steady_clock::now();
	CPacketWriter writer;
	const bool encoded = Write_Message(writer, message);
	const std::uint64_t encodeMicroseconds = To_Microseconds(
		std::chrono::steady_clock::now() - encodeStart);
	{
		std::scoped_lock lock{ m_CommandMutex };
		++m_PerformanceMetrics.iSnapshotEncodeCount;
		m_PerformanceMetrics.iLastSnapshotEncodeMicroseconds =
			encodeMicroseconds;
		m_PerformanceMetrics.iMaximumSnapshotEncodeMicroseconds = (std::max)(
			m_PerformanceMetrics.iMaximumSnapshotEncodeMicroseconds,
			encodeMicroseconds);
		if (!encoded)
			++m_PerformanceMetrics.iSnapshotEncodeFailureCount;
	}
	if (!encoded)
		return;

	const auto enqueueBatchStart = std::chrono::steady_clock::now();
	std::uint64_t maximumSessionEnqueueMicroseconds = 0u;
	std::uint64_t recipientCount = 0u;
	std::uint64_t enqueueFailureCount = 0u;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr == session)
			continue;
		++recipientCount;
		const auto sessionEnqueueStart = std::chrono::steady_clock::now();
		const bool enqueued = session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_SNAPSHOT, writer.Get_Buffer());
		const std::uint64_t sessionEnqueueMicroseconds = To_Microseconds(
			std::chrono::steady_clock::now() - sessionEnqueueStart);
		maximumSessionEnqueueMicroseconds = (std::max)(
			maximumSessionEnqueueMicroseconds, sessionEnqueueMicroseconds);
		if (!enqueued)
		{
			++enqueueFailureCount;
			session->Request_Close();
		}
	}
	const std::uint64_t enqueueBatchMicroseconds = To_Microseconds(
		std::chrono::steady_clock::now() - enqueueBatchStart);
	{
		std::scoped_lock lock{ m_CommandMutex };
		++m_PerformanceMetrics.iSnapshotEnqueueBatchCount;
		m_PerformanceMetrics.iSnapshotRecipientCount += recipientCount;
		m_PerformanceMetrics.iSnapshotEnqueueFailureCount += enqueueFailureCount;
		m_PerformanceMetrics.iLastSnapshotEnqueueMicroseconds =
			enqueueBatchMicroseconds;
		m_PerformanceMetrics.iMaximumSnapshotEnqueueMicroseconds = (std::max)(
			m_PerformanceMetrics.iMaximumSnapshotEnqueueMicroseconds,
			enqueueBatchMicroseconds);
		m_PerformanceMetrics.iLastMaximumSessionEnqueueMicroseconds =
			maximumSessionEnqueueMicroseconds;
		m_PerformanceMetrics.iMaximumSessionEnqueueMicroseconds = (std::max)(
			m_PerformanceMetrics.iMaximumSessionEnqueueMicroseconds,
			maximumSessionEnqueueMicroseconds);
	}
}

std::shared_ptr<LostArk::Server::CClientSession>
LostArk::Server::CGameRoom::Find_Session(const SESSION_ID sessionId) const
{
	const auto iter = m_Sessions.find(sessionId);
	return iter == m_Sessions.end() ? nullptr : iter->second.lock();
}
