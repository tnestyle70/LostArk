#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>
//packetwriter에 raw bytes 기록 기능 추가, header 뒤에 완성된 payload를 붙이기 위해서 필요하다
#include <span>

namespace LostArk::Shared
{
	class CPacketWriter final
	{
	public:
		void Write_U8(std::uint8_t value);
		void Write_U16(std::uint16_t value);
		void Write_U32(std::uint32_t value);
		//player spawn과 이후 worldsnapshot에서 위치와 회전을 기록하기 위해서 필요하다.
		void Write_F32(float value);

		void Write_Bytes(
			std::span<const std::uint8_t>bytes);

		bool Write_String(
			std::string_view value,
			std::size_t maxByteLength);

		const std::vector<std::uint8_t>& Get_Buffer() const
		{
			return m_Buffer;
		}

	private:
		std::vector<std::uint8_t> m_Buffer;
	};
}