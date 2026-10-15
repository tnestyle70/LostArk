// Reader의 차용 span·byte offset과 필드 읽기 API를 구현한다.
#include "Network/PacketReader.h"

// Read_F32가 uint32_t의 비트 패턴을 float로 복원하는 std::bit_cast를 사용한다.
#include <bit>

// Read_Packet_Header와 Client/Server 메시지 해석이 임시 Reader를 만든다. span만 복사하며 입력 byte를 복제하거나 수명을 연장하지 않는다.
LostArk::Shared::CPacketReader::CPacketReader(std::span<const std::uint8_t> buffer) : m_Buffer{buffer}
{
}

// 필드 전체가 있을 때만 읽는다. offset은 버퍼 처음부터 소비한 바이트 수다.
bool LostArk::Shared::CPacketReader::Read_U8(std::uint8_t& value)
{
	if (!Can_Read(1))
		return false;

	value = m_Buffer[m_iOffset];
	++m_iOffset;

	return true;
}

// 16비트 = 2바이트다. 두 번째 바이트를 8비트 올려 little-endian 정수로 조합한다.
bool LostArk::Shared::CPacketReader::Read_U16(std::uint16_t& value)
{
	//offset만큼 전부 정보를 다 읽었다면 return false로 정보를 더 읽지 않고
	//나온다.
	// Can_Read(2)는 offset 이후에 2바이트가 남았는지 검사한다. 부족하면 value와 offset을 그대로 두고 false를 반환한다.
	if (!Can_Read(2))
		return false;
	//16bytes를 | 연산과 << 8 bit shift로 옮겨서 value에 저장
	// 여기서 16은 비트 수다. 두 바이트 중 뒤의 바이트를 8비트 왼쪽으로 이동해 16비트 값을 복원한다.
	value =
		static_cast<std::uint16_t>(m_Buffer[m_iOffset]) | (static_cast<std::uint16_t>(m_Buffer[m_iOffset + 1]) << 8);

	m_iOffset += 2;

	return true;
}

// 32비트 = 4바이트다. 각각 0/8/16/24비트 위치로 옮겨 원래 비트 패턴을 복원한다.
bool LostArk::Shared::CPacketReader::Read_U32(std::uint32_t& value)
{
	//bytes count에 대한 검사 이후에 실질적인 value 값을 Read_String 쪽으로
	//채워서 넘겨준다. 8 16 32 bytes로 읽을 수 있는지를 체크하고,
	//or 연산과 bit shift로 실질적인 value 값을 채워서 read_string의 value 값으로 넘겨준다.
	// Read_U32는 4바이트로 만든 32비트 정수를 호출자의 value에 쓴다. 이 파일의 Read_String은 길이 prefix를 Read_U16으로 읽는다.
	if (!Can_Read(4))
		return false;

	//32bytes를 | 연산과 << 8 bytes shift로 옮겨서 value를 저장
	// 여기서 32는 비트 수다. 네 바이트를 각각 0/8/16/24비트 왼쪽으로 이동한 뒤 OR로 조합한다.
	value = static_cast<std::uint32_t>(m_Buffer[m_iOffset]) |
		(static_cast<std::uint32_t>(m_Buffer[m_iOffset + 1]) << 8) |
		(static_cast<std::uint32_t>(m_Buffer[m_iOffset + 2]) << 16) |
		(static_cast<std::uint32_t>(m_Buffer[m_iOffset + 3]) << 24);

	m_iOffset += 4;

	return true;
}

// Read_U32로 바이트 순서를 복원한 뒤 같은 32비트를 float로 해석한다. 수치 cast가 아니다.
bool LostArk::Shared::CPacketReader::Read_F32(float& value)
{
	std::uint32_t rawBits = 0;

	if (!Read_U32(rawBits))
		return false;

	value = std::bit_cast<float>(rawBits);

	return true;
}

//bytes별 read 함수에서 채운 value 값과 length를 바탕으로 채워넣는다.
// 길이 필드가 U16인 것은 wire 계약이다. 문자열 전체 길이에 따라 U8/U32 reader를 바꾸지 않는다.
// prefix 이후의 상한/본문 부족 실패는 string 출력은 보존하지만 이미 읽은 2바이트를 되돌리지 않는다.
bool LostArk::Shared::CPacketReader::Read_String(std::string& value, std::size_t maxByteLength)
{

	//string 읽기 왜 16bytes 지? 32bytes도 있는데 왜 16bytes인가? 
	// 길이 prefix는 16비트 정수, 즉 2바이트다. 이어지는 문자열 본문 크기는 prefix가 나타내는 바이트 수다.
	//value의 size에 따른 Read 8 16 32로 나눠서 read를 해야 하는 거 아닌가?
	// 문자열 크기와 관계없이 길이 prefix 형식은 U16으로 고정된다. 본문은 아래 value.assign이 해당 길이만큼 복사한다.
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
	// data()는 시작 주소이고 offset은 이미 소비한 byte 수다. 두 값을 더하면 본문의 첫 바이트다.
	// NUL 종료에 의존하지 않고 명시한 길이만 string이 복사한다.
	const char* stringBegin = reinterpret_cast<const char*>(m_Buffer.data() + m_iOffset);

	// 문자열 할당 예외는 catch하지 않는다. 이 대입 전에는 이미 길이 prefix를 소비했고, 본문 길이만큼의 offset 증가는 대입 성공 뒤 수행한다.
	value.assign(stringBegin, stringByteLength);

	m_iOffset += stringByteLength;

	return true;
}

// 생성 후 각 읽기가 유지하는 offset <= size 전제 아래 남은 바이트 수를 계산한다.
std::size_t LostArk::Shared::CPacketReader::Get_RemainingSize() const
{
	//이 함수를 통해서 buffer와
	// 전체 buffer 크기에서 이미 소비한 offset을 빼 아직 읽지 않은 바이트 수를 반환한다.
	return m_Buffer.size() - m_iOffset;
}

// offset + byteCount를 먼저 더하지 않고 뺄셈으로 비교하여 unsigned 덧셈 overflow를 피한다.
bool LostArk::Shared::CPacketReader::Can_Read(std::size_t byteCount) const
{
	//뺄샘 underflow를 막기 위해서 다음처럼 설정한다.
	// offset이 size보다 큰 상태는 먼저 거부한다. 그 뒤의 size - offset 비교가 unsigned 뺄셈 범위를 지키게 한다.
	if (m_iOffset > m_Buffer.size())
		return false;

	return byteCount <= m_Buffer.size() - m_iOffset;
}
