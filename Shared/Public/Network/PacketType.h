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
		S2C_WORLD_SNAPSHOT,

		C2S_CHAT,
		S2C_CHAT,

		S2C_PLAYER_DESPAWNED
	};
	//TCP는 메시지 경계를 보존하지 않기 때문에, payload앞에 header를 둔다.
	//패킷별로 경계를 구분해야, packet을 하나의 의미 단위로 읽을 수 있다.
	//6 * 8 = 48 bytes를 의미한다?
	inline constexpr std::size_t PACKET_HEADER_BYTES = 6;
	//전체 크기는 어느 정도?
	inline constexpr std::uint32_t MAX_PACKET_BYTES =
		64u * 1024u;
	//max packet bytes 64 * 1024 * 4, 어떤 근거로 이 숫자가 나온 걸까?
	inline constexpr std::size_t MAX_BUFFERED_PACKET_BYTES =
		static_cast<std::size_t>(MAX_PACKET_BYTES) * 4u;
	//알려진 패킷인지를 검사하는 함수, client에서 정해진 규칙대로 보낸 packet인지를 검사 
	[[nodiscard]]
	constexpr bool Is_Known_Packet_Type(
		PACKET_TYPE packetType)
	{
		switch (packetType)
		{
		case PACKET_TYPE::C2S_ENTER_WORLD:
		case PACKET_TYPE::S2C_ENTER_ACCEPTED:
		case PACKET_TYPE::S2C_PLAYER_SPAWNED:
		case PACKET_TYPE::C2S_MOVE:
		case PACKET_TYPE::S2C_WORLD_SNAPSHOT:
		case PACKET_TYPE::C2S_CHAT:
		case PACKET_TYPE::S2C_CHAT:
		case PACKET_TYPE::S2C_PLAYER_DESPAWNED:
			return true;
		default:
			return  false;
		}
	}

	//inline constexpr인 이유
	//inline : 이 헤더를 여러 .cpp가 include해도 동일한 변수 정의로 취급한다.
	//constexpr : 컴파일 타임 시간 상수
	inline constexpr std::size_t MAX_NICKNAME_BYTES = 32;
}