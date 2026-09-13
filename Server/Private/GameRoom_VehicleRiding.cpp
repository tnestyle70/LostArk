#include "GameRoom.h"

#include "ClientSession.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"

#include <memory>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

namespace
{
	bool Is_VehicleRidingWorld(const LostArk::Shared::WORLD_ID worldId)
	{
		return LostArk::Shared::WORLD_ID::BERN == worldId ||
			LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == worldId;
	}
}

float LostArk::Server::CGameRoom::Resolve_PlayerMoveSpeed(
	const SERVER_PLAYER& player) const
{
	if (LostArk::Shared::INVALID_VEHICLE_ID != player.iVehicleId)
	{
		if (const SERVER_VEHICLE_DEFINITION* vehicle =
			m_VehicleCatalog.Find_Vehicle(player.iVehicleId))
		{
			return vehicle->fMoveSpeed;
		}
	}
	return player.fMoveSpeed * Resolve_StanceMoveSpeedScale(player);
}

bool LostArk::Server::CGameRoom::Can_RideVehicle(
	const SERVER_PLAYER& player) const
{
	using namespace LostArk::Shared;
	return 0u != player.iCurrentHp &&
		PLAYER_ACTION_STATE::NONE == player.eAction &&
		!player.bPatternBound &&
		0u == player.iMarioStage &&
		PLAYER_MADNESS_FORM::NORMAL == player.eMadnessForm &&
		KOUKU_HUD_MODE::NONE == player.eKoukuHudMode &&
		INVALID_NET_ENTITY_ID == player.iAttachmentOwnerNetEntityId &&
		player.fKnockbackRemainingSeconds <= 0.f &&
		!player.TriggerMove.isActive &&
		0u == player.CardMaze.flags &&
		0u == player.CardMaze.transferStartTick;
}

void LostArk::Server::CGameRoom::Handle_SetVehicleRiding(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	S2C_SET_VEHICLE_RIDING_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.eResult = VEHICLE_RIDING_RESULT::REJECTED_SESSION;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
		result = Apply_SetVehicleRiding(player->second, request);
	CPacketWriter writer;
	if (!Write_Message(writer, result) || !session->Send_Frame(
		PACKET_TYPE::S2C_SET_VEHICLE_RIDING_RESULT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT
LostArk::Server::CGameRoom::Apply_SetVehicleRiding(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request)
{
	using namespace LostArk::Shared;
	S2C_SET_VEHICLE_RIDING_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.iActiveVehicleId = player.iVehicleId;
	if (request.eWorldId != m_eWorldId)
	{
		result.eResult = VEHICLE_RIDING_RESULT::REJECTED_WRONG_WORLD;
		return result;
	}
	const S2C_SET_VEHICLE_RIDING_RESULT& previous = player.LastVehicleRidingResult;
	if (0u != request.iRequestSequence &&
		request.iRequestSequence == previous.iRequestSequence)
	{
		return previous;
	}
	if (!Is_NewerSequence(request.iRequestSequence, previous.iRequestSequence))
	{
		result.eResult = VEHICLE_RIDING_RESULT::REJECTED_STALE_SEQUENCE;
		return result;
	}
	const auto commit = [&player, &result](const VEHICLE_RIDING_RESULT reason)
	{
		result.eResult = reason;
		result.iActiveVehicleId = player.iVehicleId;
		player.LastVehicleRidingResult = result;
		return result;
	};
	if (request.iVehicleId == player.iVehicleId)
		return commit(VEHICLE_RIDING_RESULT::REJECTED_SAME_STATE);
	if (INVALID_VEHICLE_ID == request.iVehicleId)
	{
		player.iVehicleId = INVALID_VEHICLE_ID;
		return commit(VEHICLE_RIDING_RESULT::ACCEPTED);
	}
	if (!Is_VehicleRidingWorld(m_eWorldId))
		return commit(VEHICLE_RIDING_RESULT::REJECTED_WORLD_NOT_ALLOWED);
	if (nullptr == m_VehicleCatalog.Find_Vehicle(request.iVehicleId))
		return commit(VEHICLE_RIDING_RESULT::REJECTED_UNKNOWN_VEHICLE);
	if (!Can_RideVehicle(player))
		return commit(VEHICLE_RIDING_RESULT::REJECTED_PLAYER_STATE);
	player.iVehicleId = request.iVehicleId;
	return commit(VEHICLE_RIDING_RESULT::ACCEPTED);
}

void LostArk::Server::CGameRoom::Enforce_VehicleRidingState()
{
	const bool ridingWorld = Is_VehicleRidingWorld(m_eWorldId);
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (LostArk::Shared::INVALID_VEHICLE_ID == player.iVehicleId)
			continue;
		if (!ridingWorld ||
			nullptr == m_VehicleCatalog.Find_Vehicle(player.iVehicleId) ||
			!Can_RideVehicle(player))
		{
			player.iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
		}
	}
}
