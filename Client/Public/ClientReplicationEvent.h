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
		WORLD_ENTITY_SPAWNED,
		COMBAT_OBJECT_SPAWNED,
		WORLD_ENTITY_DESPAWNED,
		COMBAT_OBJECT_DESPAWNED,
		PLAYER_DESPAWNED,
		WORLD_SNAPSHOT,
		WORLD_DESTRUCTION_FULL_SYNC,
		WORLD_DESTRUCTION_DELTA,
		ENCOUNTER_PROP_SYNC,
		INVENTORY_SNAPSHOT
	};

	/* Only adjacent full snapshots are replaceable.  Every reliable lifecycle
	   event is an ordering barrier at both the raw-frame and parsed-event
	   queues, so spawn -> snapshot -> despawn cannot collapse. */
	constexpr bool Can_CoalesceAdjacentReplicationEvents(
		const CLIENT_REPLICATION_EVENT_TYPE previous,
		const CLIENT_REPLICATION_EVENT_TYPE incoming)
	{
		return CLIENT_REPLICATION_EVENT_TYPE::WORLD_SNAPSHOT == previous &&
			CLIENT_REPLICATION_EVENT_TYPE::WORLD_SNAPSHOT == incoming;
	}

	constexpr bool Can_CoalesceAdjacentInboundFrames(
		const LostArk::Shared::PACKET_TYPE previous,
		const LostArk::Shared::PACKET_TYPE incoming)
	{
		return LostArk::Shared::PACKET_TYPE::S2C_WORLD_SNAPSHOT == previous &&
			LostArk::Shared::PACKET_TYPE::S2C_WORLD_SNAPSHOT == incoming;
	}
	//하나의 ordered_queue에 서로 다른 종류의 이벤트를 저장하기 위한 봉투
	//spawn, despawn, snapshot을 서로 다른 queue에 넣지 않고, 하나의 queue가 tcp
	//도착 순서를 보존해야, spawn->snapshot->despawn의 의미가 사라지지 않음
	struct CLIENT_REPLICATION_EVENT
	{
		CLIENT_REPLICATION_EVENT_TYPE eType =
			CLIENT_REPLICATION_EVENT_TYPE::PLAYER_SPAWNED;

		LostArk::Shared::S2C_PLAYER_SPAWNED PlayerSpawned;
		LostArk::Shared::S2C_WORLD_ENTITY_SPAWNED WorldEntitySpawned;
		LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED CombatObjectSpawned;
		LostArk::Shared::S2C_WORLD_ENTITY_DESPAWNED WorldEntityDespawned;
		LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED CombatObjectDespawned;
		LostArk::Shared::S2C_PLAYER_DESPAWNED PlayerDespawned;
		LostArk::Shared::S2C_WORLD_SNAPSHOT WorldSnapshot;
		LostArk::Shared::S2C_WORLD_DESTRUCTION_FULL_SYNC
			WorldDestructionFullSync;
		LostArk::Shared::S2C_WORLD_DESTRUCTION_DELTA
			WorldDestructionDelta;
		LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC EncounterPropSync;
		LostArk::Shared::S2C_INVENTORY_SNAPSHOT InventorySnapshot;
	};
}
