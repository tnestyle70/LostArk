#pragma once

#include <cstdint>
//playerid와 netentity id의 의미를 고정
//invalid 값은 0
//shared 소유
//client와 server가 동일하게 사용

//payload를 8byte로 통일 시킨다
namespace LostArk::Shared
{
	//네트워크에서 사용하는 ID 타입들의 정본
	using PLAYER_ID =
		std::uint32_t;

	using NET_ENTITY_ID =
		std::uint32_t;

	// Room-owned combat objects use their own monotonic identity space.  They
	// are not actors and must never be interpreted as NET_ENTITY_ID values.
	using COMBAT_OBJECT_ID =
		std::uint64_t;

	inline constexpr PLAYER_ID
		INVALID_PLAYER_ID = 0;

	inline constexpr NET_ENTITY_ID
		INVALID_NET_ENTITY_ID = 0;

	inline constexpr COMBAT_OBJECT_ID
		INVALID_COMBAT_OBJECT_ID = 0;
}
