#pragma once

#include "Network/PacketMessages.h"

//서버 패킷을 engine에 바로 전달하지 않고, client 내부에서
//처리할 의미 있는 이벤트로 한 번 변환하기 위해 존재

//S2C_PLAYER_SPAWNED -> PLAYER_SPAWNED event
//S2C_PLAYER_DESPAWNED -> PLAYER_DESPAWNED event

namespace Client
{
	//이벤트가 생성인지 삭제인지를 나타낸다.
	//추후에 이동과 스킬 등등 동기화
	enum class CLIENT_REPLICATION_EVENT_TYPE
	{
		PLAYER_SPAWNED,
		PLAYER_DESPAWNED
	};
	//하나의 ordered_queue에 서로 다른 종류의 이벤트를 저장하기 위한 봉투
	struct CLIENT_REPLICATION_EVENT
	{
		CLIENT_REPLICATION_EVENT_TYPE eType =
			CLIENT_REPLICATION_EVENT_TYPE::PLAYER_SPAWNED;

		LostArk::Shared::S2C_PLAYER_SPAWNED PlayerSpawned;
		LostArk::Shared::S2C_PLAYER_DESPAWNED PlayerDespawned;
	};
}