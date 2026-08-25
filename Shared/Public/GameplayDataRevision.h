#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace LostArk::Shared
{
	class CPacketReader;
	class CPacketWriter;

	inline constexpr std::size_t GAMEPLAY_DATA_REVISION_BYTES = 32u;
	inline constexpr std::size_t GAMEPLAY_DATA_REVISION_HEX_BYTES =
		GAMEPLAY_DATA_REVISION_BYTES * 2u;

	// SHA-256 content identity for one immutable gameplay/presentation bundle.
	// The all-zero value is reserved as "no revision" and is never valid on wire.
	struct GameplayDataRevision final
	{
		std::array<std::uint8_t, GAMEPLAY_DATA_REVISION_BYTES> Bytes{};

		[[nodiscard]] bool Is_Valid() const noexcept;

		friend bool operator==(
			const GameplayDataRevision&,
			const GameplayDataRevision&) = default;
	};

	// Parses exactly 64 hexadecimal characters. The destination is unchanged on
	// malformed input, including the reserved all-zero revision.
	[[nodiscard]] bool Try_Parse_GameplayDataRevision(
		std::string_view hex,
		GameplayDataRevision& revision) noexcept;

	// Returns lowercase hexadecimal for a valid revision, or an empty string for
	// the reserved all-zero value.
	[[nodiscard]] std::string Format_GameplayDataRevision(
		const GameplayDataRevision& revision);

	// Fixed-width 32-byte wire encoding. Read failure never mutates destination.
	[[nodiscard]] bool Write_GameplayDataRevision(
		CPacketWriter& writer,
		const GameplayDataRevision& revision);
	[[nodiscard]] bool Read_GameplayDataRevision(
		CPacketReader& reader,
		GameplayDataRevision& revision);
}
