#pragma once

#include "Network/PacketType.h"

#include <cstdint>
#include <span>
#include <vector>

namespace LostArk::Shared
{
	//packet의 전체 사이즈 기준으로 packet type과 total size에 대한 정보를 가지고 있다.
	//tcp를 경계가 없기 때문에, 이런 식으로 header에 정보를 담아서 읽어야 한다.
	struct PACKET_HEADER
	{
		std::uint32_t iTotalSize = {};

		PACKET_TYPE ePacketType =
			PACKET_TYPE::INVALID;
	};

	//packet header가 tcp 사이의 경계를 나눠주는 거는 알겠는데, 
	//frame이 하는 역할은 뭐지? 이 구조체의 역할이 뭐야?
	//header 즉 tcp의 packet을 구분하는 정보와 frame, 즉 packet의 type과 실질적인 payload
	//를 나눠서 담는 역할을 하는 거 맞나?
	struct PACKET_FRAME
	{
		PACKET_TYPE ePacketType = PACKET_TYPE::INVALID;

		std::vector<std::uint8_t> Payload;
	};

	bool Build_Packet_Frame(
		PACKET_TYPE packetType,
		std::span<const std::uint8_t> payload,
		std::vector<std::uint8_t>& frameBytes);

	bool Read_Packet_Header(
		std::span<const std::uint8_t> bytes,
		PACKET_HEADER& header);
}