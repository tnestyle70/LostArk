// 32 byte revision의 검사·텍스트 변환·wire 읽기/쓰기를 구현한다.
#include "GameplayDataRevision.h"

// 고정 길이 revision을 한 byte씩 지역 값에 읽는다.
#include "Network/PacketReader.h"
// 길이 prefix 없이 revision 32 byte를 writer 저장소에 복사한다.
#include "Network/PacketWriter.h"

// Is_Valid가 any_of로 0이 아닌 byte의 존재만 검사한다.
#include <algorithm>
// revision 배열의 연속 범위를 소유하지 않고 Write_Bytes에 전달한다.
#include <span>

namespace
{
	// 한 문자를 4비트 값으로 변환한다. 허용하지 않는 문자는 -1이며 로케일에 의존하지 않는다.
	[[nodiscard]] int Hex_Nibble(const char value) noexcept
	{
		if ('0' <= value && value <= '9')
			return value - '0';
		if ('a' <= value && value <= 'f')
			return value - 'a' + 10;
		if ('A' <= value && value <= 'F')
			return value - 'A' + 10;
		return -1;
	}
}

// 예약된 all-zero 값과 실제 식별값을 구분한다. 무결성·서명·현재 Catalog 존재를 검사하는 함수는 아니다.
bool LostArk::Shared::GameplayDataRevision::Is_Valid() const noexcept
{
	return std::any_of(Bytes.begin(), Bytes.end(),
		[](const std::uint8_t value)
		{
			return 0u != value;
		});
}

// loader의 텍스트 입력을 지역 decoded에 모은 뒤 성공할 때만 호출자의 revision에 대입한다.
bool LostArk::Shared::Try_Parse_GameplayDataRevision(
	const std::string_view hex, GameplayDataRevision& revision) noexcept
{
	if (GAMEPLAY_DATA_REVISION_HEX_BYTES != hex.size())
		return false;

	GameplayDataRevision decoded{};
	for (std::size_t index = 0; index < decoded.Bytes.size(); ++index)
	{
		const int high = Hex_Nibble(hex[index * 2u]);
		const int low = Hex_Nibble(hex[index * 2u + 1u]);
		if (0 > high || 0 > low)
			return false;
		// 앞 문자는 상위 4비트, 뒤 문자는 하위 4비트다. 두 문자가 한 바이트가 된다.
		decoded.Bytes[index] = static_cast<std::uint8_t>((high << 4) | low);
	}
	if (!decoded.Is_Valid())
		return false;
	revision = decoded;
	return true;
}

// 값의 소유 복사본인 문자열을 반환한다. 호출자가 보관해도 입력 revision의 수명에 묶이지 않는다.
std::string LostArk::Shared::Format_GameplayDataRevision(const GameplayDataRevision& revision)
{
	if (!revision.Is_Valid())
		return {};

	// 각 byte의 상위/하위 4비트를 두 문자로 풀어 원래 순서를 유지한다. 문자열 할당 예외는 이 함수 밖으로 전달된다.
	static constexpr char DIGITS[] = "0123456789abcdef";
	std::string result(GAMEPLAY_DATA_REVISION_HEX_BYTES, '0');
	for (std::size_t index = 0; index < revision.Bytes.size(); ++index)
	{
		result[index * 2u] = DIGITS[revision.Bytes[index] >> 4u];
		result[index * 2u + 1u] = DIGITS[revision.Bytes[index] & 0x0fu];
	}
	return result;
}

// 메시지 serializer가 유효한 revision의 원시 32바이트를 writer 소유 버퍼에 추가한다.
bool LostArk::Shared::Write_GameplayDataRevision(CPacketWriter& writer, const GameplayDataRevision& revision)
{
	if (!revision.Is_Valid())
		return false;
	writer.Write_Bytes(std::span<const std::uint8_t>{revision.Bytes});
	return true;
}

// 메시지 parser가 사용하는 고정 길이 reader다. 출력 보존과 reader 위치 복원은 서로 다른 계약이다.
// Read_U8 실패나 all-zero 검사 실패 전에 소비한 바이트를 되감지 않으므로 호출자는 실패한 메시지를 버린다.
bool LostArk::Shared::Read_GameplayDataRevision(CPacketReader& reader, GameplayDataRevision& revision)
{
	GameplayDataRevision decoded{};
	for (std::uint8_t& value : decoded.Bytes)
	{
		if (!reader.Read_U8(value))
			return false;
	}
	if (!decoded.Is_Valid())
		return false;
	revision = decoded;
	return true;
}
