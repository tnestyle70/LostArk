#pragma once

#include "ServerIds.h"
#include "Network/PacketMessages.h"

#include <memory>

namespace LostArk::Server
{
	//server는 여러 개의 존재하는 client를 가져야 한다.
	class CClientSession;
	// Room은 같은 World 상태와 Tick을 공유하고 서로의 Event를 Broadcast 받는 세션 집합이다.
	// 현재는 베른성 하나이므로 Server에 CGameRoom 하나만 존재한다.
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
