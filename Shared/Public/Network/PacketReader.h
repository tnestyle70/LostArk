#pragma once

#include <cstddef>
#include <cstdint>
#include <span>	
#include <string>

//packetreader에서 수행해야 하는 동작 - Client로부터 온 packet을 8bytes 단위로
//읽어서 buffer에 어디까지 읽었는지를 담는다.

namespace LostArk::Shared
{
	//8bytes 단위로 packet을 끊어서 읽는다.
	class CPacketReader final
	{
	public:
		explicit CPacketReader(
			std::span<const std::uint8_t> buffer);

	public:
		bool Read_U8(std::uint8_t& value);
		bool Read_U16(std::uint16_t& value);
		bool Read_U32(std::uint32_t& value);
		//Writer가 기록한 위치와 회전 float를 복원하기 위해서 필요
		bool Read_F32(float& value);

		bool Read_String(
			std::string& value,
			std::size_t maxByteLength);

		std::size_t Get_RemainingSize() const;

	private:
		bool Can_Read(std::size_t byteCount) const;
		
	private:
		//span은 데이터를 소유하지 않는 연속 메모리 보기이다.
		//개념적으로 다음 두 값을 묶은 것이다.
		//const uint8_t* data, size_t size
		//원본 vector가 span보다 오래 살아있어야 한다. 
		std::span<const std::uint8_t> m_Buffer;
		//offset 기준으로 읽는다.
		std::size_t m_iOffset = {};
	};
}