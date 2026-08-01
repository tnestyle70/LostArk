#pragma once

#include "ServerIds.h"

#include "Network/NetworkIds.h"
#include "Network/PacketType.h"

#include <string>

namespace LostArk::Server
{
	//server 단에서 가지고 있는 플레이어의 정보들을 구조체로 저장하고, 
	//그 정보를 client의 입력을 통해서 서버 내부에 tick으로 돌려서, transform을 계산하고,
	//다시 client 쪽으로 broadcast를 한다.
	struct SERVER_PLAYER
	{
		//session id
		SESSION_ID iSessionId = INVALID_SESSION_ID;
		//player id
		LostArk::Shared::PLAYER_ID iPlayerId =
			LostArk::Shared::INVALID_PLAYER_ID;
		//net entity id
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		//character class
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		//nick name
		std::string strNickName;

		//pos
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;

		//yaw
		float fYawDegrees = 0.f;
	};
}