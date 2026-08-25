#include "GameplayDataRevision.h"

#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <algorithm>
#include <span>

namespace
{
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

bool LostArk::Shared::GameplayDataRevision::Is_Valid() const noexcept
{
	return std::any_of(Bytes.begin(), Bytes.end(), [](const std::uint8_t value)
		{
			return 0u != value;
		});
}

bool LostArk::Shared::Try_Parse_GameplayDataRevision(
	const std::string_view hex,
	GameplayDataRevision& revision) noexcept
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
		decoded.Bytes[index] = static_cast<std::uint8_t>((high << 4) | low);
	}
	if (!decoded.Is_Valid())
		return false;
	revision = decoded;
	return true;
}

std::string LostArk::Shared::Format_GameplayDataRevision(
	const GameplayDataRevision& revision)
{
	if (!revision.Is_Valid())
		return {};

	static constexpr char DIGITS[] = "0123456789abcdef";
	std::string result(GAMEPLAY_DATA_REVISION_HEX_BYTES, '0');
	for (std::size_t index = 0; index < revision.Bytes.size(); ++index)
	{
		result[index * 2u] = DIGITS[revision.Bytes[index] >> 4u];
		result[index * 2u + 1u] = DIGITS[revision.Bytes[index] & 0x0fu];
	}
	return result;
}

bool LostArk::Shared::Write_GameplayDataRevision(
	CPacketWriter& writer,
	const GameplayDataRevision& revision)
{
	if (!revision.Is_Valid())
		return false;
	writer.Write_Bytes(std::span<const std::uint8_t>{ revision.Bytes });
	return true;
}

bool LostArk::Shared::Read_GameplayDataRevision(
	CPacketReader& reader,
	GameplayDataRevision& revision)
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
