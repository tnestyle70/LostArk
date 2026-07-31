#pragma once

#include <cstddef>
#include <cstdint>

namespace LostArk::Shared
{
	enum class CHARACTER_CLASS_ID : std::uint8_t
	{
		LANCE_MASTER, 
		GUNSLINGER,
		DESTROYER,
		ARTIST,
		END
	};

	enum class PACKET_TYPE : std::uint16_t
	{
		INVALID,

		C2S_ENTER_WORLD,
		S2C_ENTER_ACCEPTED,
		S2C_PLAYER_SPAWNED,

		C2S_MOVE,

	};
}