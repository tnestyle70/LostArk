#pragma once

#include "Network/PacketFrame.h"

#include <cstddef>
#include <cstdint>
#include <span>	
#include <vector>

namespace LostArk::Shared
{
	//왜 packet parser의 결과가 need mode data, frame ready, invalid frame으로 나올까?
	enum class PACKET_PARSE_RESULT
	{
		NEED_MORE_DATA,
		FRAME_READY,
		INVALID_FRAME
	};

	class CPacketStreamParser final
	{
	public:
		bool Append(std::span<const std::uint8_t> bytes);

		PACKET_PARSE_RESULT Try_Pop(
			PACKET_FRAME& frame);

		void Reset();

		std::size_t Get_BufferedByteCount() const;

	private:
		void Compact();

	private:
		std::vector<std::uint8_t> m_Buffer;
		std::size_t m_iReadOffset = {};
	};
}