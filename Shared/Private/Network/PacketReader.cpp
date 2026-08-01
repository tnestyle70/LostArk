#include "Network/PacketReader.h"

#include <bit>

LostArk::Shared::CPacketReader::CPacketReader(std::span<const std::uint8_t> buffer)
	: m_Buffer{buffer}
{}

bool LostArk::Shared::CPacketReader::Read_U8(std::uint8_t & value)
{
	if (!Can_Read(1))
		return false;

	value = m_Buffer[m_iOffset];
	++m_iOffset;

	return true;
}

bool LostArk::Shared::CPacketReader::Read_U16(std::uint16_t& value)
{
	//offset만큼 전부 정보를 다 읽었다면 return false로 정보를 더 읽지 않고
	//나온다.
	if (!Can_Read(2))
		return false;
	//16bytes를 | 연산과 << 8 bit shift로 옮겨서 value에 저장
	value =
		static_cast<std::uint16_t>(m_Buffer[m_iOffset]) |
		(static_cast<std::uint16_t>(m_Buffer[m_iOffset + 1]) << 8);

	m_iOffset += 2;

	return true;
}

bool LostArk::Shared::CPacketReader::Read_U32(std::uint32_t& value)
{
	//bytes count에 대한 검사 이후에 실질적인 value 값을 Read_String 쪽으로
	//채워서 넘겨준다. 8 16 32 bytes로 읽을 수 있는지를 체크하고,
	//or 연산과 bit shift로 실질적인 value 값을 채워서 read_string의 value 값으로 넘겨준다.
	if (!Can_Read(4))
		return false;

	//32bytes를 | 연산과 << 8 bytes shift로 옮겨서 value를 저장
	value =
		static_cast<std::uint32_t>(
			m_Buffer[m_iOffset]) |
		(static_cast<std::uint32_t>(
			m_Buffer[m_iOffset + 1]) << 8) |
		(static_cast<std::uint32_t>(
			m_Buffer[m_iOffset + 2]) << 16) |
		(static_cast<std::uint32_t>(
			m_Buffer[m_iOffset + 3]) << 24);

	m_iOffset += 4;

	return true;
}

bool LostArk::Shared::CPacketReader::Read_F32(float& value)
{
	std::uint32_t rawBits = 0;

	if (!Read_U32(rawBits))
		return false;

	value = std::bit_cast<float>(rawBits);

	return true;
}

//bytes별 read 함수에서 채운 value 값과 length를 바탕으로 채워넣는다.
bool LostArk::Shared::CPacketReader::Read_String(std::string& value, 
	std::size_t maxByteLength)
{
	//string 읽기 왜 16bytes 지? 32bytes도 있는데 왜 16bytes인가? 

	//value의 size에 따른 Read 8 16 32로 나눠서 read를 해야 하는 거 아닌가?
	std::uint16_t stringByteLength = {};
	
	if (!Read_U16(stringByteLength))
		return false;
	
	if (stringByteLength > maxByteLength)
		return false;

	if (!Can_Read(stringByteLength))
		return false;
	//이 string을 char*로 캐스팅해서 buffer의 size랑 offset을 통해서
	//begin이 되는 string을 설정한다. m_Buffer size랑 왜 offset을 더하지? 
	//buffer.data()는 원소의 주소이고, m_iOffset은 소비한 주소이다.
	const char* stringBegin =
		reinterpret_cast<const char*>(
			m_Buffer.data() + m_iOffset);

	value.assign(stringBegin, stringByteLength);

	m_iOffset += stringByteLength;

	return true;
}

std::size_t LostArk::Shared::CPacketReader::Get_RemainingSize() const
{
	//이 함수를 통해서 buffer와
	return m_Buffer.size() - m_iOffset;
}

bool LostArk::Shared::CPacketReader::Can_Read(std::size_t byteCount) const
{
	//뺄샘 underflow를 막기 위해서 다음처럼 설정한다.
	if (m_iOffset > m_Buffer.size())
		return false;

	return byteCount <= m_Buffer.size() - m_iOffset;
}
