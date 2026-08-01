#pragma once

#include "ServerIds.h"

#include "Network/PacketMessages.h"

#include <memory>

namespace LostArk::Server
{
	//server는 여러 개의 존재하는 client를 가져야 한다.
	class CClientSession;
	//Room이 정확히 어떤 단위를 의미하는 걸까?
	enum class ROOM_COMMAND_TYPE
	{
		REGISTER_SESSION,
		ENTER_WORLD,
		LEAVE
	};

	struct ROOM_COMMAND
	{
		ROOM_COMMAND_TYPE eType =
			ROOM_COMMAND_TYPE::REGISTER_SESSION;

		SESSION_ID iSessionId = INVALID_SESSION_ID;

		std::shared_ptr<CClientSession> pSession;

		LostArk::Shared::C2S_ENTER_WORLD EnterWorld;

		LostArk::Shared::PLAYER_DESPAWN_REASON eLeaveReason =
			LostArk::Shared::PLAYER_DESPAWN_REASON::DISCONNECTED;
	};
}