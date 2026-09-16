#include "GameRoom.h"
#include "GameRoom_Internal.h"

#include "ClientSession.h"
#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"

#include <memory>

using namespace GameRoomDetail;

/* The honor title a player wears over their head. Cosmetic: the Server owns which id the
player has on (so every Client draws the same one and it survives a world transfer through
ROOM_COMMAND::iCarriedHonorTitleId) and admits only ids the published bootstrap lists; the
snapshot carries the id, this verdict only reports why a request did nothing. */

void LostArk::Server::CGameRoom::Handle_SetHonorTitle(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_SET_HONOR_TITLE& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	S2C_SET_HONOR_TITLE_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.eResult = HONOR_TITLE_RESULT::REJECTED_SESSION;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
		result = Apply_SetHonorTitle(player->second, request);
	CPacketWriter writer;
	if (!Write_Message(writer, result) || !session->Send_Frame(
		PACKET_TYPE::S2C_SET_HONOR_TITLE_RESULT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

LostArk::Shared::S2C_SET_HONOR_TITLE_RESULT
LostArk::Server::CGameRoom::Apply_SetHonorTitle(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_SET_HONOR_TITLE& request)
{
	using namespace LostArk::Shared;
	S2C_SET_HONOR_TITLE_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.iActiveHonorTitleId = player.iHonorTitleId;
	if (request.eWorldId != m_eWorldId)
	{
		result.eResult = HONOR_TITLE_RESULT::REJECTED_WRONG_WORLD;
		return result;
	}
	const S2C_SET_HONOR_TITLE_RESULT& previous = player.LastHonorTitleResult;
	if (0u != request.iRequestSequence &&
		request.iRequestSequence == previous.iRequestSequence)
	{
		return previous;
	}
	if (!Is_NewerSequence(request.iRequestSequence, previous.iRequestSequence))
	{
		result.eResult = HONOR_TITLE_RESULT::REJECTED_STALE_SEQUENCE;
		return result;
	}
	const auto commit = [&player, &result](const HONOR_TITLE_RESULT reason)
	{
		result.eResult = reason;
		result.iActiveHonorTitleId = player.iHonorTitleId;
		player.LastHonorTitleResult = result;
		return result;
	};
	if (request.iHonorTitleId == player.iHonorTitleId)
		return commit(HONOR_TITLE_RESULT::REJECTED_SAME_STATE);
	if (INVALID_HONOR_TITLE_ID != request.iHonorTitleId &&
		!m_HonorTitleCatalog.Has_Title(request.iHonorTitleId))
	{
		return commit(HONOR_TITLE_RESULT::REJECTED_UNKNOWN_TITLE);
	}
	player.iHonorTitleId = request.iHonorTitleId;
	return commit(HONOR_TITLE_RESULT::ACCEPTED);
}
