// 메시지 serializer가 사용하는 byte 소유 버퍼와 쓰기 API의 본문을 구현한다.
#include "Network/PacketWriter.h"

// Write_F32가 수치 변환 없이 float 비트를 uint32_t로 옮긴다.
#include <bit>
// 문자열 byte 길이가 wire의 U16 길이 prefix에 들어가는지 검사한다.
#include <limits>

//8Byte Buffer 기준으로 Packet을 받아서 Buffer에 8 Byte 단위로 push_back한다.
// 8비트 값 하나는 1바이트다. 메시지 필드의 순서대로 소유 vector 끝에 붙인다.
// Write_Message 계열이 호출하며 함수들은 기존 buffer 뒤에 추가한다. 다중 byte/문자열 기록 중 할당 예외가 나면 이미 붙인 prefix를 자동 rollback하지 않는다.
void LostArk::Shared::CPacketWriter::Write_U8(std::uint8_t value)
{
	m_Buffer.push_back(value);
}

// 0xFF로 낮은 8비트를 고르고 다음 바이트를 오른쪽으로 8비트 내려 기록한다. 결과는 2바이트다.
void LostArk::Shared::CPacketWriter::Write_U16(std::uint16_t value)
{
	//16bit 정수를 1바이트씩 나눠 little-endian 순서로 기록한다.
	//0xFF = 10진수 255, 2진수 1111 1111 정확히 8비트가 1이기 때문에
	//8비트만 추출하는 마스크로 사용을 한다.
	// 첫 push_back은 0xFF 마스크로 낮은 8비트를 골라 쓴다. 다음 push_back이 높은 8비트를 기록한다.
	m_Buffer.push_back(static_cast<std::uint8_t>(value & 0xFF));
	//8bits를 밀어서 거기에 있는 정보를 읽는다.
	// 원본 value를 오른쪽으로 8비트 이동한 뒤 0xFF로 그 위치의 한 바이트를 고른다. value 자체는 바꾸지 않는다.
	m_Buffer.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
}

// 낮은 바이트부터 4개를 기록한다. 호스트 구조체의 padding이나 메모리 byte 순서를 전송하지 않는다.
void LostArk::Shared::CPacketWriter::Write_U32(std::uint32_t value)
{
	//32bits 이므로 8 bits 씩 잘라서 읽는다.
	// 이 push_back은 네 바이트 중 가장 낮은 8비트부터 기록한다.
	m_Buffer.push_back(static_cast<std::uint8_t>(value & 0xFF));
	//8bits를 밀어서 거기에 있는 정보를 읽는다.
	// 원본 value를 오른쪽으로 8비트 이동한 뒤 0xFF로 그 위치의 한 바이트를 고른다. value 자체는 바꾸지 않는다.
	m_Buffer.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
	//8bits를 밀어서 거기에 있는 정보를 읽는다.
	// 원본 value를 오른쪽으로 16비트 이동한 뒤 0xFF로 그 위치의 한 바이트를 고른다. value 자체는 바꾸지 않는다.
	m_Buffer.push_back(static_cast<std::uint8_t>((value >> 16) & 0xFF));
	//8bits를 밀어서 거기에 있는 정보를 읽는다.
	// 원본 value를 오른쪽으로 24비트 이동한 뒤 0xFF로 그 위치의 한 바이트를 고른다. value 자체는 바꾸지 않는다.
	m_Buffer.push_back(static_cast<std::uint8_t>((value >> 24) & 0xFF));
}

// float의 비트를 uint32_t로 옮겨 정수와 같은 little-endian wire 순서로 쓴다. NaN도 그대로 보존한다.
void LostArk::Shared::CPacketWriter::Write_F32(float value)
{

	//float의 실제 32비트 비트 패턴을 uint32_t로 변환한뒤 기존 write u32 경로를 사용
	// std::bit_cast는 수치 변환 없이 같은 비트를 uint32_t에 옮긴다. Write_U32가 그 비트를 바이트 순서로 기록한다.
	//회전에 대한 float의 부동 소수점 값을 그대로 유지하기 위해서 float로 받은 값을 비트에 그대로 저장
	// 아래 static_assert는 float와 uint32_t의 크기가 같은지 컴파일 때 확인한다. bit_cast 결과는 rawBits로 보관한다.
	static_assert(sizeof(float) == sizeof(std::uint32_t));

	const std::uint32_t rawBits = std::bit_cast<std::uint32_t>(value);

	Write_U32(rawBits);
}

// Build_Packet_Frame이 완성된 payload를 header 뒤에 복사할 때 사용한다. 별도 길이 정보는 추가하지 않는다.
void LostArk::Shared::CPacketWriter::Write_Bytes(std::span<const std::uint8_t> bytes)
{
	m_Buffer.insert(m_Buffer.end(), bytes.begin(), bytes.end());
}

// 메시지별 최대 길이와 U16이 표현할 65535바이트를 모두 확인한 뒤 길이와 본문을 쓴다.
// byte 길이를 저장하므로 한글 문자 수와 다르다. UTF-8 형식/내용 정책은 메시지 계층이 검사한다.
bool LostArk::Shared::CPacketWriter::Write_String(std::string_view value, std::size_t maxByteLength)
{
	if (value.size() > maxByteLength)
		return false;

	if (value.size() > (std::numeric_limits<std::uint16_t>::max)())
		return false;

	// 두 명시적 거부 검사를 모두 통과한 뒤에만 buffer를 변경한다. 이후 길이와 본문 기록의 할당 예외는 bool false로 변환하지 않는다.
	Write_U16(static_cast<std::uint16_t>(value.size()));

	for (const char character : value)
	{
		m_Buffer.push_back(static_cast<std::uint8_t>(character));
	}

	return true;
}
