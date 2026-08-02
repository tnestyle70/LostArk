#include "Network/PacketWriter.h"

#include <bit>
#include <limits>

//8Byte Buffer 기준으로 Packet을 받아서 Buffer에 8 Byte 단위로 push_back한다.
void LostArk::Shared::CPacketWriter::Write_U8(std::uint8_t value)
{
	m_Buffer.push_back(value);
}

void LostArk::Shared::CPacketWriter::Write_U16(std::uint16_t value)
{
	//16bit 정수를 1바이트씩 나눠 little-endian 순서로 기록한다.
	//0xFF = 10진수 255, 2진수 1111 1111 정확히 8비트가 1이기 때문에
	//8비트만 추출하는 마스크로 사용을 한다.
	m_Buffer.push_back(
		static_cast<std::uint8_t>(value & 0xFF));
	//8bits를 밀어서 거기에 있는 정보를 읽는다.
	m_Buffer.push_back(
		static_cast<std::uint8_t>((value >> 8) & 0xFF));
}

void LostArk::Shared::CPacketWriter::Write_U32(std::uint32_t value)
{
	//32bits 이므로 8 bits 씩 잘라서 읽는다.
	m_Buffer.push_back(
		static_cast<std::uint8_t>(value & 0xFF));
	//8bits를 밀어서 거기에 있는 정보를 읽는다.
	m_Buffer.push_back(
		static_cast<std::uint8_t>((value >> 8) & 0xFF));
	//8bits를 밀어서 거기에 있는 정보를 읽는다.
	m_Buffer.push_back(
		static_cast<std::uint8_t>((value >> 16) & 0xFF));
	//8bits를 밀어서 거기에 있는 정보를 읽는다.
	m_Buffer.push_back(
		static_cast<std::uint8_t>((value >> 24) & 0xFF));
}

void LostArk::Shared::CPacketWriter::Write_F32(float value)
{
	//float의 실제 32비트 비트 패턴을 uint32_t로 변환한뒤 기존 write u32 경로를 사용

	//회전에 대한 float의 부동 소수점 값을 그대로 유지하기 위해서 float로 받은 값을 비트에 그대로 저장
	
	static_assert(sizeof(float) == sizeof(std::uint32_t));
	
	const std::uint32_t rawBits =
		std::bit_cast<std::uint32_t>(value);

	Write_U32(rawBits);

	//uint32_t buffer = std::_Bit_cast<uint32_t>(value);

	//Write_U32(buffer);

	//return;
}

void LostArk::Shared::CPacketWriter::Write_Bytes(std::span<const std::uint8_t> bytes)
{
	//packet write 
	m_Buffer.insert(m_Buffer.end(), bytes.begin(), bytes.end());
}

bool LostArk::Shared::CPacketWriter::Write_String(std::string_view value, 
	std::size_t maxByteLength)
{
	if (value.size() > maxByteLength)
		return false;

	if (value.size() >
		(std::numeric_limits<std::uint16_t>::max)())
		return false;

	Write_U16(static_cast<std::uint16_t>(value.size()));

	for (const char character : value)
	{
		m_Buffer.push_back(
			static_cast<std::uint8_t>(character));
	}

	return true;
}
