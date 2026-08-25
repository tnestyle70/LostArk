#include "ServerApp.h"

#include "ClientSession.h"

#include "Network/PacketMessages.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <WinSock2.h>
#include <bcrypt.h>

#include <cstdio>
#include <io.h>

#include <chrono>
#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <set>
#include <span>
#include <string>
#include <utility>
#include <vector>

#pragma comment(lib, "bcrypt.lib")

namespace
{
	using LostArk::Shared::GameplayDataRevision;

	enum class JSON_KIND : std::uint8_t
	{
		OBJECT, ARRAY, STRING, INTEGER, BOOLEAN, NULL_VALUE
	};

	struct JSON_VALUE final
	{
		JSON_KIND eKind = JSON_KIND::NULL_VALUE;
		std::string String;
		std::uint64_t Integer = 0u;
		bool isNegative = false;
		bool Boolean = false;
		std::vector<std::string> MemberNames;
		std::vector<JSON_VALUE> MemberValues;
		std::vector<JSON_VALUE> Elements;
	};

	bool Is_ValidUtf8(const std::string_view value)
	{
		for (std::size_t index = 0u; index < value.size();)
		{
			const unsigned char lead =
				static_cast<unsigned char>(value[index]);
			if (lead < 0x80u)
			{
				++index;
				continue;
			}
			std::size_t count = 0u;
			std::uint32_t codePoint = 0u;
			if (0xc2u <= lead && lead <= 0xdfu)
			{
				count = 2u; codePoint = lead & 0x1fu;
			}
			else if (0xe0u <= lead && lead <= 0xefu)
			{
				count = 3u; codePoint = lead & 0x0fu;
			}
			else if (0xf0u <= lead && lead <= 0xf4u)
			{
				count = 4u; codePoint = lead & 0x07u;
			}
			else
				return false;
			if (index + count > value.size())
				return false;
			for (std::size_t offset = 1u; offset < count; ++offset)
			{
				const unsigned char next =
					static_cast<unsigned char>(value[index + offset]);
				if (0x80u != (next & 0xc0u))
					return false;
				codePoint = (codePoint << 6u) | (next & 0x3fu);
			}
			if ((2u == count && codePoint < 0x80u) ||
				(3u == count && codePoint < 0x800u) ||
				(4u == count && codePoint < 0x10000u) ||
				(0xd800u <= codePoint && codePoint <= 0xdfffu) ||
				codePoint > 0x10ffffu)
				return false;
			index += count;
		}
		return true;
	}

	void Append_Utf8(const std::uint32_t codePoint, std::string& value)
	{
		if (codePoint <= 0x7fu)
			value.push_back(static_cast<char>(codePoint));
		else if (codePoint <= 0x7ffu)
		{
			value.push_back(static_cast<char>(0xc0u | (codePoint >> 6u)));
			value.push_back(static_cast<char>(0x80u | (codePoint & 0x3fu)));
		}
		else if (codePoint <= 0xffffu)
		{
			value.push_back(static_cast<char>(0xe0u | (codePoint >> 12u)));
			value.push_back(static_cast<char>(
				0x80u | ((codePoint >> 6u) & 0x3fu)));
			value.push_back(static_cast<char>(0x80u | (codePoint & 0x3fu)));
		}
		else
		{
			value.push_back(static_cast<char>(0xf0u | (codePoint >> 18u)));
			value.push_back(static_cast<char>(
				0x80u | ((codePoint >> 12u) & 0x3fu)));
			value.push_back(static_cast<char>(
				0x80u | ((codePoint >> 6u) & 0x3fu)));
			value.push_back(static_cast<char>(0x80u | (codePoint & 0x3fu)));
		}
	}

	class CBoundedJsonParser final
	{
	public:
		explicit CBoundedJsonParser(const std::string_view source)
			: m_Source(source) {}

		bool Parse(JSON_VALUE& value, std::string& status)
		{
			if (m_Source.size() >= 3u &&
				static_cast<unsigned char>(m_Source[0]) == 0xefu &&
				static_cast<unsigned char>(m_Source[1]) == 0xbbu &&
				static_cast<unsigned char>(m_Source[2]) == 0xbfu)
			{
				status = "UTF-8 BOM is forbidden in candidate JSON";
				return false;
			}
			Skip_Whitespace();
			if (!Parse_Value(value, 0u, status))
				return false;
			Skip_Whitespace();
			if (m_iCursor != m_Source.size())
			{
				status = "Candidate JSON has trailing bytes";
				return false;
			}
			return true;
		}

	private:
		static int Hex_Nibble(const char value)
		{
			if ('0' <= value && value <= '9') return value - '0';
			if ('a' <= value && value <= 'f') return value - 'a' + 10;
			if ('A' <= value && value <= 'F') return value - 'A' + 10;
			return -1;
		}

		void Skip_Whitespace()
		{
			while (m_iCursor < m_Source.size())
			{
				const char value = m_Source[m_iCursor];
				if (' ' != value && '\t' != value && '\r' != value &&
					'\n' != value)
					break;
				++m_iCursor;
			}
		}

		bool Parse_Value(
			JSON_VALUE& value, const std::size_t depth, std::string& status)
		{
			if (depth > 64u || ++m_iNodeCount > 200000u ||
				m_iCursor >= m_Source.size())
			{
				status = "Candidate JSON exceeds structural limits";
				return false;
			}
			const char lead = m_Source[m_iCursor];
			if ('{' == lead) return Parse_Object(value, depth, status);
			if ('[' == lead) return Parse_Array(value, depth, status);
			if ('\"' == lead)
			{
				value.eKind = JSON_KIND::STRING;
				return Parse_String(value.String, status);
			}
			if ('-' == lead || ('0' <= lead && lead <= '9'))
				return Parse_Integer(value, status);
			if (m_Source.substr(m_iCursor, 4u) == "true")
			{
				m_iCursor += 4u; value.eKind = JSON_KIND::BOOLEAN;
				value.Boolean = true; return true;
			}
			if (m_Source.substr(m_iCursor, 5u) == "false")
			{
				m_iCursor += 5u; value.eKind = JSON_KIND::BOOLEAN;
				value.Boolean = false; return true;
			}
			if (m_Source.substr(m_iCursor, 4u) == "null")
			{
				m_iCursor += 4u; value.eKind = JSON_KIND::NULL_VALUE;
				return true;
			}
			status = "Candidate JSON token is invalid";
			return false;
		}

		bool Parse_Object(
			JSON_VALUE& value, const std::size_t depth, std::string& status)
		{
			value.eKind = JSON_KIND::OBJECT;
			++m_iCursor;
			Skip_Whitespace();
			if (m_iCursor < m_Source.size() && '}' == m_Source[m_iCursor])
			{
				++m_iCursor; return true;
			}
			while (m_iCursor < m_Source.size())
			{
				std::string name;
				if (!Parse_String(name, status)) return false;
				if (std::find(value.MemberNames.begin(), value.MemberNames.end(),
					name) != value.MemberNames.end())
				{
					status = "Candidate JSON contains a duplicate object member";
					return false;
				}
				Skip_Whitespace();
				if (m_iCursor >= m_Source.size() ||
					':' != m_Source[m_iCursor++])
				{
					status = "Candidate JSON object separator is invalid";
					return false;
				}
				Skip_Whitespace();
				JSON_VALUE member;
				if (!Parse_Value(member, depth + 1u, status)) return false;
				value.MemberNames.push_back(std::move(name));
				value.MemberValues.push_back(std::move(member));
				Skip_Whitespace();
				if (m_iCursor >= m_Source.size()) break;
				if ('}' == m_Source[m_iCursor])
				{
					++m_iCursor; return true;
				}
				if (',' != m_Source[m_iCursor++]) break;
				Skip_Whitespace();
			}
			status = "Candidate JSON object is unterminated";
			return false;
		}

		bool Parse_Array(
			JSON_VALUE& value, const std::size_t depth, std::string& status)
		{
			value.eKind = JSON_KIND::ARRAY;
			++m_iCursor;
			Skip_Whitespace();
			if (m_iCursor < m_Source.size() && ']' == m_Source[m_iCursor])
			{
				++m_iCursor; return true;
			}
			while (m_iCursor < m_Source.size())
			{
				JSON_VALUE element;
				if (!Parse_Value(element, depth + 1u, status)) return false;
				value.Elements.push_back(std::move(element));
				Skip_Whitespace();
				if (m_iCursor >= m_Source.size()) break;
				if (']' == m_Source[m_iCursor])
				{
					++m_iCursor; return true;
				}
				if (',' != m_Source[m_iCursor++]) break;
				Skip_Whitespace();
			}
			status = "Candidate JSON array is unterminated";
			return false;
		}

		bool Parse_String(std::string& value, std::string& status)
		{
			if (m_iCursor >= m_Source.size() ||
				'\"' != m_Source[m_iCursor++])
			{
				status = "Candidate JSON string is invalid";
				return false;
			}
			value.clear();
			while (m_iCursor < m_Source.size())
			{
				const unsigned char raw =
					static_cast<unsigned char>(m_Source[m_iCursor++]);
				if ('\"' == raw)
				{
					if (value.size() > 1024u * 1024u || !Is_ValidUtf8(value))
					{
						status = "Candidate JSON string exceeds UTF-8 limits";
						return false;
					}
					return true;
				}
				if ('\\' != raw)
				{
					if (raw < 0x20u)
					{
						status = "Candidate JSON string contains a control byte";
						return false;
					}
					value.push_back(static_cast<char>(raw));
					continue;
				}
				if (m_iCursor >= m_Source.size()) break;
				const char escape = m_Source[m_iCursor++];
				switch (escape)
				{
				case '\"': value.push_back('\"'); break;
				case '\\': value.push_back('\\'); break;
				case '/': value.push_back('/'); break;
				case 'b': value.push_back('\b'); break;
				case 'f': value.push_back('\f'); break;
				case 'n': value.push_back('\n'); break;
				case 'r': value.push_back('\r'); break;
				case 't': value.push_back('\t'); break;
				case 'u':
				{
					if (m_iCursor + 4u > m_Source.size())
					{
						status = "Candidate JSON unicode escape is incomplete";
						return false;
					}
					std::uint32_t codePoint = 0u;
					for (std::size_t index = 0u; index < 4u; ++index)
					{
						const int nibble = Hex_Nibble(m_Source[m_iCursor + index]);
						if (nibble < 0)
						{
							status = "Candidate JSON unicode escape is invalid";
							return false;
						}
						codePoint = (codePoint << 4u) |
							static_cast<std::uint32_t>(nibble);
					}
					m_iCursor += 4u;
					if (0xd800u <= codePoint && codePoint <= 0xdbffu)
					{
						if (m_iCursor + 6u > m_Source.size() ||
							'\\' != m_Source[m_iCursor] ||
							'u' != m_Source[m_iCursor + 1u])
						{
							status = "Candidate JSON surrogate pair is incomplete";
							return false;
						}
						std::uint32_t low = 0u;
						for (std::size_t index = 0u; index < 4u; ++index)
						{
							const int nibble = Hex_Nibble(
								m_Source[m_iCursor + 2u + index]);
							if (nibble < 0)
							{
								status = "Candidate JSON surrogate escape is invalid";
								return false;
							}
							low = (low << 4u) |
								static_cast<std::uint32_t>(nibble);
						}
						if (low < 0xdc00u || low > 0xdfffu)
						{
							status = "Candidate JSON low surrogate is invalid";
							return false;
						}
						m_iCursor += 6u;
						codePoint = 0x10000u +
							((codePoint - 0xd800u) << 10u) + (low - 0xdc00u);
					}
					else if (0xdc00u <= codePoint && codePoint <= 0xdfffu)
					{
						status = "Candidate JSON has an unpaired low surrogate";
						return false;
					}
					Append_Utf8(codePoint, value);
					break;
				}
				default:
					status = "Candidate JSON escape is invalid";
					return false;
				}
			}
			status = "Candidate JSON string is unterminated";
			return false;
		}

		bool Parse_Integer(JSON_VALUE& value, std::string& status)
		{
			value.isNegative = '-' == m_Source[m_iCursor];
			if (value.isNegative) ++m_iCursor;
			const std::size_t digits = m_iCursor;
			if (m_iCursor >= m_Source.size() ||
				m_Source[m_iCursor] < '0' || m_Source[m_iCursor] > '9')
			{
				status = "Candidate JSON integer is invalid";
				return false;
			}
			if ('0' == m_Source[m_iCursor] && m_iCursor + 1u < m_Source.size() &&
				'0' <= m_Source[m_iCursor + 1u] &&
				m_Source[m_iCursor + 1u] <= '9')
			{
				status = "Candidate JSON integer has a leading zero";
				return false;
			}
			while (m_iCursor < m_Source.size() &&
				'0' <= m_Source[m_iCursor] && m_Source[m_iCursor] <= '9')
				++m_iCursor;
			if (m_iCursor < m_Source.size() &&
				('.' == m_Source[m_iCursor] || 'e' == m_Source[m_iCursor] ||
					'E' == m_Source[m_iCursor]))
			{
				status = "Candidate manifest forbids floating-point numbers";
				return false;
			}
			const char* first = m_Source.data() + digits;
			const char* last = m_Source.data() + m_iCursor;
			const auto parsed = std::from_chars(first, last, value.Integer);
			if (parsed.ec != std::errc{} || parsed.ptr != last ||
				(value.isNegative && 0u == value.Integer))
			{
				status = "Candidate JSON integer is out of range";
				return false;
			}
			value.eKind = JSON_KIND::INTEGER;
			return true;
		}

		std::string_view m_Source;
		std::size_t m_iCursor = 0u;
		std::size_t m_iNodeCount = 0u;
	};

	const JSON_VALUE* Find_Member(
		const JSON_VALUE& object, const std::string_view name)
	{
		if (JSON_KIND::OBJECT != object.eKind) return nullptr;
		for (std::size_t index = 0u; index < object.MemberNames.size(); ++index)
			if (object.MemberNames[index] == name)
				return &object.MemberValues[index];
		return nullptr;
	}

	JSON_VALUE* Find_Member(JSON_VALUE& object, const std::string_view name)
	{
		return const_cast<JSON_VALUE*>(Find_Member(
			static_cast<const JSON_VALUE&>(object), name));
	}

	bool Has_ExactMembers(
		const JSON_VALUE& object,
		const std::initializer_list<std::string_view> expected)
	{
		if (JSON_KIND::OBJECT != object.eKind ||
			object.MemberNames.size() != expected.size())
			return false;
		return std::all_of(expected.begin(), expected.end(),
			[&object](const std::string_view name)
			{
				return nullptr != Find_Member(object, name);
			});
	}

	bool Is_String(const JSON_VALUE* value, const std::string_view expected)
	{
		return nullptr != value && JSON_KIND::STRING == value->eKind &&
			value->String == expected;
	}

	bool Is_Integer(
		const JSON_VALUE* value, const std::uint64_t minimum,
		const std::uint64_t maximum)
	{
		return nullptr != value && JSON_KIND::INTEGER == value->eKind &&
			!value->isNegative && minimum <= value->Integer &&
			value->Integer <= maximum;
	}

	bool Is_LowerSha256(const JSON_VALUE* value)
	{
		return nullptr != value && JSON_KIND::STRING == value->eKind &&
			value->String.size() == LostArk::Shared::GAMEPLAY_DATA_REVISION_HEX_BYTES &&
			std::all_of(value->String.begin(), value->String.end(),
				[](const char character)
				{
					return ('0' <= character && character <= '9') ||
						('a' <= character && character <= 'f');
				});
	}

	bool Json_Equals(const JSON_VALUE& left, const JSON_VALUE& right)
	{
		if (left.eKind != right.eKind) return false;
		switch (left.eKind)
		{
		case JSON_KIND::STRING:
			return left.String == right.String;
		case JSON_KIND::INTEGER:
			return left.Integer == right.Integer &&
				left.isNegative == right.isNegative;
		case JSON_KIND::BOOLEAN:
			return left.Boolean == right.Boolean;
		case JSON_KIND::NULL_VALUE:
			return true;
		case JSON_KIND::ARRAY:
			if (left.Elements.size() != right.Elements.size()) return false;
			for (std::size_t index = 0u; index < left.Elements.size(); ++index)
				if (!Json_Equals(left.Elements[index], right.Elements[index]))
					return false;
			return true;
		case JSON_KIND::OBJECT:
			if (left.MemberNames.size() != right.MemberNames.size()) return false;
			for (std::size_t index = 0u; index < left.MemberNames.size(); ++index)
			{
				const JSON_VALUE* other =
					Find_Member(right, left.MemberNames[index]);
				if (nullptr == other ||
					!Json_Equals(left.MemberValues[index], *other))
					return false;
			}
			return true;
		}
		return false;
	}

	void Append_CanonicalString(const std::string& value, std::string& output)
	{
		static constexpr char HEX[] = "0123456789abcdef";
		output.push_back('\"');
		for (const unsigned char character : value)
		{
			switch (character)
			{
			case '\"': output += "\\\""; break;
			case '\\': output += "\\\\"; break;
			case '\b': output += "\\b"; break;
			case '\f': output += "\\f"; break;
			case '\n': output += "\\n"; break;
			case '\r': output += "\\r"; break;
			case '\t': output += "\\t"; break;
			default:
				if (character < 0x20u)
				{
					output += "\\u00";
					output.push_back(HEX[character >> 4u]);
					output.push_back(HEX[character & 0x0fu]);
				}
				else output.push_back(static_cast<char>(character));
				break;
			}
		}
		output.push_back('\"');
	}

	void Append_CanonicalJson(const JSON_VALUE& value, std::string& output)
	{
		switch (value.eKind)
		{
		case JSON_KIND::STRING:
			Append_CanonicalString(value.String, output); break;
		case JSON_KIND::INTEGER:
			if (value.isNegative) output.push_back('-');
			output += std::to_string(value.Integer); break;
		case JSON_KIND::BOOLEAN:
			output += value.Boolean ? "true" : "false"; break;
		case JSON_KIND::NULL_VALUE:
			output += "null"; break;
		case JSON_KIND::ARRAY:
			output.push_back('[');
			for (std::size_t index = 0u; index < value.Elements.size(); ++index)
			{
				if (0u != index) output.push_back(',');
				Append_CanonicalJson(value.Elements[index], output);
			}
			output.push_back(']');
			break;
		case JSON_KIND::OBJECT:
		{
			std::vector<std::size_t> order(value.MemberNames.size());
			for (std::size_t index = 0u; index < order.size(); ++index)
				order[index] = index;
			std::sort(order.begin(), order.end(),
				[&value](const std::size_t left, const std::size_t right)
				{
					return value.MemberNames[left] < value.MemberNames[right];
				});
			output.push_back('{');
			for (std::size_t ordinal = 0u; ordinal < order.size(); ++ordinal)
			{
				if (0u != ordinal) output.push_back(',');
				const std::size_t index = order[ordinal];
				Append_CanonicalString(value.MemberNames[index], output);
				output.push_back(':');
				Append_CanonicalJson(value.MemberValues[index], output);
			}
			output.push_back('}');
			break;
		}
		}
	}

	bool Hash_BytesSha256(
		const std::span<const std::uint8_t> bytes,
		GameplayDataRevision& revision)
	{
		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD objectBytes = 0u;
		DWORD resultBytes = 0u;
		std::vector<std::uint8_t> object;
		GameplayDataRevision staged{};
		bool succeeded = false;
		if (0 <= BCryptOpenAlgorithmProvider(
			&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) &&
			0 <= BCryptGetProperty(
				algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&objectBytes), sizeof(objectBytes),
				&resultBytes, 0u))
		{
			object.resize(objectBytes);
			if (0 <= BCryptCreateHash(
				algorithm, &hash, object.data(), objectBytes,
				nullptr, 0u, 0u))
			{
				std::size_t offset = 0u;
				succeeded = true;
				while (offset < bytes.size())
				{
					const ULONG chunk = static_cast<ULONG>((std::min)(
						bytes.size() - offset,
						static_cast<std::size_t>((std::numeric_limits<ULONG>::max)())));
					if (0 > BCryptHashData(hash,
						const_cast<PUCHAR>(bytes.data() + offset), chunk, 0u))
					{
						succeeded = false; break;
					}
					offset += chunk;
				}
				if (succeeded && 0 > BCryptFinishHash(hash,
					staged.Bytes.data(),
					static_cast<ULONG>(staged.Bytes.size()), 0u))
					succeeded = false;
			}
		}
		if (nullptr != hash) BCryptDestroyHash(hash);
		if (nullptr != algorithm) BCryptCloseAlgorithmProvider(algorithm, 0u);
		if (!succeeded || !staged.Is_Valid()) return false;
		revision = staged;
		return true;
	}

	bool Read_BoundedFile(
		const std::filesystem::path& path, const std::uintmax_t maximumBytes,
		std::vector<std::uint8_t>& bytes, std::string& status)
	{
		std::error_code error;
		const std::uintmax_t size = std::filesystem::file_size(path, error);
		if (error || size > maximumBytes ||
			size > static_cast<std::uintmax_t>((std::numeric_limits<std::size_t>::max)()))
		{
			status = "Candidate artifact exceeds its bounded file size";
			return false;
		}
		std::ifstream stream(path, std::ios::binary);
		if (!stream)
		{
			status = "Candidate artifact could not be opened";
			return false;
		}
		bytes.resize(static_cast<std::size_t>(size));
		if (!bytes.empty())
			stream.read(reinterpret_cast<char*>(bytes.data()),
				static_cast<std::streamsize>(bytes.size()));
		if (!stream || stream.peek() != std::ifstream::traits_type::eof())
		{
			status = "Candidate artifact read was incomplete";
			return false;
		}
		return true;
	}

	bool Hash_FileSha256(
		const std::filesystem::path& path, GameplayDataRevision& revision,
		std::string& status)
	{
		std::ifstream stream(path, std::ios::binary);
		if (!stream)
		{
			status = "Candidate artifact could not be opened for hashing";
			return false;
		}
		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD objectBytes = 0u;
		DWORD resultBytes = 0u;
		std::vector<std::uint8_t> object;
		GameplayDataRevision staged{};
		bool succeeded = false;
		if (0 <= BCryptOpenAlgorithmProvider(
			&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) &&
			0 <= BCryptGetProperty(
				algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&objectBytes), sizeof(objectBytes),
				&resultBytes, 0u))
		{
			object.resize(objectBytes);
			succeeded = 0 <= BCryptCreateHash(
				algorithm, &hash, object.data(), objectBytes,
				nullptr, 0u, 0u);
		}
		std::array<std::uint8_t, 1024u * 1024u> buffer{};
		while (succeeded && stream)
		{
			stream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
			const std::streamsize count = stream.gcount();
			if (count > 0 && 0 > BCryptHashData(
				hash, buffer.data(), static_cast<ULONG>(count), 0u))
				succeeded = false;
		}
		if (!stream.eof()) succeeded = false;
		if (succeeded && 0 > BCryptFinishHash(hash, staged.Bytes.data(),
			static_cast<ULONG>(staged.Bytes.size()), 0u))
			succeeded = false;
		if (nullptr != hash) BCryptDestroyHash(hash);
		if (nullptr != algorithm) BCryptCloseAlgorithmProvider(algorithm, 0u);
		if (!succeeded || !staged.Is_Valid())
		{
			status = "Candidate artifact SHA-256 failed";
			return false;
		}
		revision = staged;
		return true;
	}

	bool Is_SafeRelativePath(const std::string& relative)
	{
		if (relative.empty() || '/' == relative.front() ||
			relative.find('\\') != std::string::npos ||
			relative.find(':') != std::string::npos)
			return false;
		std::size_t cursor = 0u;
		while (cursor <= relative.size())
		{
			const std::size_t slash = relative.find('/', cursor);
			const std::string_view segment(relative.data() + cursor,
				(std::string::npos == slash ? relative.size() : slash) - cursor);
			if (segment.empty() || "." == segment || ".." == segment)
				return false;
			if (std::string::npos == slash) break;
			cursor = slash + 1u;
		}
		return true;
	}

	bool Resolve_ExactRegularFile(
		const std::filesystem::path& candidateDirectory,
		const std::string& relative, std::filesystem::path& path,
		std::string& status)
	{
		if (!Is_SafeRelativePath(relative))
		{
			status = "Candidate artifact relative path is invalid";
			return false;
		}
		std::error_code error;
		const std::filesystem::path relativePath{ relative };
		if (relativePath.is_absolute() || relativePath.has_root_name() ||
			relativePath.has_root_directory())
		{
			status = "Candidate artifact path is drive-qualified or absolute";
			return false;
		}
		const std::filesystem::path expected =
			(candidateDirectory / relativePath).lexically_normal();
		const std::filesystem::file_status linkStatus =
			std::filesystem::symlink_status(expected, error);
		if (error || !std::filesystem::is_regular_file(linkStatus) ||
			std::filesystem::is_symlink(linkStatus))
		{
			status = "Candidate artifact is not a regular non-symlink file";
			return false;
		}
		const std::filesystem::path canonical =
			std::filesystem::canonical(expected, error);
		const std::filesystem::path relativeCanonical =
			canonical.lexically_relative(candidateDirectory);
		if (error || canonical != expected || relativeCanonical.empty() ||
			relativeCanonical.is_absolute() || relativeCanonical.has_root_name() ||
			relativeCanonical.has_root_directory() ||
			relativeCanonical.begin() == relativeCanonical.end() ||
			*relativeCanonical.begin() == L"..")
		{
			status = "Candidate artifact escaped its canonical revision directory";
			return false;
		}
		path = canonical;
		return true;
	}

	std::filesystem::path Resolve_RepositoryRoot()
	{
		std::vector<std::filesystem::path> starts;
		std::vector<wchar_t> pathBuffer(32768u);
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != configuredLength && configuredLength < pathBuffer.size())
			starts.emplace_back(pathBuffer.data());
		const DWORD moduleLength = GetModuleFileNameW(nullptr, pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != moduleLength && moduleLength < pathBuffer.size())
			starts.emplace_back(std::filesystem::path(pathBuffer.data()).parent_path());
		std::error_code error;
		starts.push_back(std::filesystem::current_path(error));
		for (std::filesystem::path start : starts)
		{
			for (std::size_t depth = 0u; depth < 10u && !start.empty(); ++depth)
			{
				error.clear();
				if (std::filesystem::is_directory(start / L"Data", error) &&
					std::filesystem::is_directory(
						start / L"Tools" / L"ValtanPipeline", error))
					return std::filesystem::canonical(start, error);
				const std::filesystem::path parent = start.parent_path();
				if (parent == start) break;
				start = parent;
			}
		}
		return {};
	}

	std::filesystem::path Resolve_RuntimeGameplayBootstrap()
	{
		namespace fs = std::filesystem;
		std::vector<wchar_t> pathBuffer(32768u);
		fs::path dataRoot;
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != configuredLength && configuredLength < pathBuffer.size())
		{
			dataRoot = fs::path(pathBuffer.data()).lexically_normal();
		}
		else
		{
			const DWORD moduleLength = GetModuleFileNameW(
				nullptr, pathBuffer.data(), static_cast<DWORD>(pathBuffer.size()));
			if (0u == moduleLength || moduleLength >= pathBuffer.size())
				return {};
			dataRoot = fs::path(pathBuffer.data()).parent_path().parent_path() /
				L"DataFiles";
		}
		const fs::path expected =
			(dataRoot / L"Gameplay" / L"Gameplay.bootstrap").lexically_normal();
		std::error_code error;
		const fs::file_status linkStatus = fs::symlink_status(expected, error);
		const fs::path canonical = fs::canonical(expected, error);
		if (error || !fs::is_regular_file(linkStatus) ||
			fs::is_symlink(linkStatus) || canonical != expected)
		{
			return {};
		}
		return canonical;
	}

	std::vector<std::string_view> Split_Tabs(const std::string_view line)
	{
		std::vector<std::string_view> fields;
		std::size_t start = 0u;
		while (true)
		{
			const std::size_t tab = line.find('\t', start);
			fields.push_back(line.substr(start,
				std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab) break;
			start = tab + 1u;
		}
		return fields;
	}

	bool Is_ValtanOwnedGameplayRow(
		const std::vector<std::string_view>& fields)
	{
		if (fields.size() < 2u) return false;
		const std::string_view kind = fields[0];
		const std::string_view owner = fields[1];
		if (("BOSS" == kind || "BOSSARMOR" == kind ||
			"BOSSPART" == kind) && "BOSS_VALTAN" == owner)
		{
			return true;
		}
		if ("DAMAGE" == kind)
			return owner.starts_with("damage.valtan.");
		const bool encounterOwned = "ENCOUNTERINTRO" == kind ||
			"BOSSCOMBATOBJECT" == kind ||
			"BOSSCOMBATOBJECTHIT" == kind ||
			kind.starts_with("PATTERN") || kind.starts_with("VALTANTIMELINE");
		return encounterOwned && "ENCOUNTER_VALTAN" == owner;
	}

	bool Build_NonValtanGameplayRevision(
		const std::filesystem::path& bootstrapPath,
		GameplayDataRevision& revision,
		std::string& status)
	{
		revision = {};
		std::vector<std::uint8_t> bytes;
		if (!Read_BoundedFile(
			bootstrapPath, 64u * 1024u * 1024u, bytes, status))
		{
			return false;
		}
		const std::string_view content(
			reinterpret_cast<const char*>(bytes.data()), bytes.size());
		if (!Is_ValidUtf8(content) ||
			std::string_view::npos != content.find('\0'))
		{
			status = "Gameplay bootstrap is not bounded valid UTF-8 text";
			return false;
		}
		std::string normalized;
		normalized.reserve(bytes.size());
		std::uint64_t declaredRowCount = 0u;
		std::uint64_t actualRowCount = 0u;
		bool hasHeader = false;
		std::size_t start = 0u;
		while (start < content.size())
		{
			const std::size_t newline = content.find('\n', start);
			std::string_view line = content.substr(start,
				std::string_view::npos == newline ? newline : newline - start);
			if (!line.empty() && '\r' == line.back()) line.remove_suffix(1u);
			if (line.empty())
			{
				status = "Gameplay bootstrap contains an empty row";
				return false;
			}
			const std::vector<std::string_view> fields = Split_Tabs(line);
			if (!hasHeader)
			{
				std::uint32_t version = 0u;
				const auto parsedVersion = fields.size() < 2u ?
					std::from_chars(line.data(), line.data(), version) :
					std::from_chars(fields[1].data(),
						fields[1].data() + fields[1].size(), version);
				const auto parsedCount = fields.size() < 3u ?
					std::from_chars(line.data(), line.data(), declaredRowCount) :
					std::from_chars(fields[2].data(),
						fields[2].data() + fields[2].size(), declaredRowCount);
				if (3u != fields.size() ||
					"LOSTARK_GAMEPLAY_BOOTSTRAP" != fields[0] ||
					std::errc{} != parsedVersion.ec ||
					parsedVersion.ptr != fields[1].data() + fields[1].size() ||
					LostArk::Server::GAMEPLAY_BOOTSTRAP_VERSION != version ||
					std::errc{} != parsedCount.ec ||
					parsedCount.ptr != fields[2].data() + fields[2].size())
				{
					status = "Gameplay bootstrap header is invalid for domain hashing";
					return false;
				}
				normalized.append("LOSTARK_NON_VALTAN_GAMEPLAY\t");
				normalized.append(fields[1]);
				normalized.push_back('\n');
				hasHeader = true;
			}
			else
			{
				++actualRowCount;
				if (!Is_ValtanOwnedGameplayRow(fields))
				{
					normalized.append(line);
					normalized.push_back('\n');
				}
			}
			if (std::string_view::npos == newline) break;
			start = newline + 1u;
		}
		if (!hasHeader || actualRowCount != declaredRowCount ||
			!Hash_BytesSha256(std::span<const std::uint8_t>(
				reinterpret_cast<const std::uint8_t*>(normalized.data()),
				normalized.size()), revision))
		{
			status = "Gameplay bootstrap row count or non-Valtan hash is invalid";
			return false;
		}
		status.clear();
		return true;
	}

	bool Parse_JsonBytes(const std::vector<std::uint8_t>& bytes,
		JSON_VALUE& value, std::string& status)
	{
		return CBoundedJsonParser(std::string_view(
			reinterpret_cast<const char*>(bytes.data()), bytes.size())).Parse(
			value, status);
	}

	bool Validate_ExactStringArray(const JSON_VALUE* value,
		const std::initializer_list<std::string_view> expected)
	{
		if (nullptr == value || JSON_KIND::ARRAY != value->eKind ||
			value->Elements.size() != expected.size())
			return false;
		std::size_t index = 0u;
		for (const std::string_view item : expected)
			if (!Is_String(&value->Elements[index++], item)) return false;
		return true;
	}

	bool Admit_ValtanCandidateGeneration(
		const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request,
		const GameplayDataRevision& activeNonValtanGameplayRevision,
		std::shared_ptr<const LostArk::Server::CGameplayCatalog>& candidate,
		GameplayDataRevision& candidateBootstrapContentRevision,
		GameplayDataRevision& candidateNonValtanGameplayRevision,
		std::string& status)
	{
		using namespace LostArk::Shared;
		namespace fs = std::filesystem;
		candidate.reset();
		candidateBootstrapContentRevision = {};
		candidateNonValtanGameplayRevision = {};
		if (request.iRequiredPresentationLaneMask !=
			GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK ||
			!activeNonValtanGameplayRevision.Is_Valid())
		{
			status = "Candidate request or active non-Valtan identity is invalid";
			return false;
		}
		const std::string revisionHex =
			Format_GameplayDataRevision(request.CandidateRevision);
		const fs::path repositoryRoot = Resolve_RepositoryRoot();
		if (repositoryRoot.empty() || revisionHex.empty())
		{
			status = "Repository or candidate revision identity is unavailable";
			return false;
		}
		const fs::path expectedDirectory = (repositoryRoot / L"Intermediate" /
			L"ValtanTuningCandidates" / L"revisions" /
			fs::path(revisionHex)).lexically_normal();
		std::error_code error;
		const fs::file_status directoryStatus =
			fs::symlink_status(expectedDirectory, error);
		const fs::path candidateDirectory =
			fs::canonical(expectedDirectory, error);
		if (error || !fs::is_directory(directoryStatus) ||
			fs::is_symlink(directoryStatus) || candidateDirectory != expectedDirectory)
		{
			status = "Fixed candidate revision directory is unavailable or non-canonical";
			return false;
		}

		fs::path manifestPath;
		fs::path identityPath;
		std::vector<std::uint8_t> manifestBytes;
		std::vector<std::uint8_t> identityBytes;
		if (!Resolve_ExactRegularFile(candidateDirectory,
			"revision-manifest.json", manifestPath, status) ||
			!Resolve_ExactRegularFile(candidateDirectory,
				"revision-identity.json", identityPath, status) ||
			!Read_BoundedFile(manifestPath, 8u * 1024u * 1024u,
				manifestBytes, status) ||
			!Read_BoundedFile(identityPath, 8u * 1024u * 1024u,
				identityBytes, status))
			return false;
		GameplayDataRevision identityRevision{};
		if (!Hash_BytesSha256(identityBytes, identityRevision) ||
			identityRevision != request.CandidateRevision)
		{
			status = "Raw revision identity SHA-256 does not match the request";
			return false;
		}

		JSON_VALUE manifest;
		JSON_VALUE identity;
		if (!Parse_JsonBytes(manifestBytes, manifest, status) ||
			!Parse_JsonBytes(identityBytes, identity, status))
			return false;
		std::string canonicalIdentity;
		canonicalIdentity.reserve(identityBytes.size());
		Append_CanonicalJson(identity, canonicalIdentity);
		if (canonicalIdentity.size() != identityBytes.size() ||
			!std::equal(canonicalIdentity.begin(), canonicalIdentity.end(),
				identityBytes.begin()))
		{
			status = "Revision identity bytes are not canonical JSON";
			return false;
		}
		if (!Has_ExactMembers(manifest,
			{ "schema", "formatVersion", "revisionId", "revisionIdentity",
			  "sourceManifestId", "authoringBaseRevision", "artifactSetId",
			  "draftPatchOperationCount", "allowedDomains",
			  "requiredPresentationLanes", "clientPresentationCompatibility",
			  "serverGameplayBootstrap", "applyClass", "runtimeActivation",
			  "serverSubmanifestSha256", "clientSubmanifestSha256",
			  "authoringSubmanifestSha256", "artifacts" }))
		{
			status = "Candidate revision manifest fields are not exact";
			return false;
		}
		JSON_VALUE manifestIdentity = manifest;
		JSON_VALUE* blankRevision = Find_Member(manifestIdentity, "revisionId");
		if (nullptr == blankRevision || JSON_KIND::STRING != blankRevision->eKind)
		{
			status = "Candidate revisionId is not a string";
			return false;
		}
		blankRevision->String.clear();
		if (!Json_Equals(manifestIdentity, identity))
		{
			status = "Manifest and canonical parent identity differ";
			return false;
		}
		if (!Is_String(Find_Member(manifest, "schema"),
				"lostark.valtan-tuning-revision-manifest") ||
			!Is_Integer(Find_Member(manifest, "formatVersion"), 1u, 1u) ||
			!Is_String(Find_Member(manifest, "revisionId"), revisionHex) ||
			!Is_String(Find_Member(manifest, "applyClass"), "HOT_RELOAD") ||
			!Is_String(Find_Member(manifest, "runtimeActivation"),
				"SERVER_2PC_TICK_BOUNDARY") ||
			!Validate_ExactStringArray(Find_Member(manifest, "allowedDomains"),
				{ "VALTAN_BOSS" }) ||
			!Validate_ExactStringArray(
				Find_Member(manifest, "requiredPresentationLanes"),
				{ "ANIMATION", "EFFECT", "COMBAT_VISUAL", "CAMERA",
				  "WORLD_EVENT_SET" }) ||
			!Is_Integer(Find_Member(manifest, "draftPatchOperationCount"),
				0u, 100000u))
		{
			status = "Candidate HOT_RELOAD activation contract is invalid";
			return false;
		}
		for (const std::string_view field :
			{ "revisionId", "sourceManifestId", "authoringBaseRevision",
			  "artifactSetId", "serverSubmanifestSha256",
			  "clientSubmanifestSha256", "authoringSubmanifestSha256" })
		{
			if (!Is_LowerSha256(Find_Member(manifest, field)))
			{
				status = "Candidate manifest contains a malformed SHA-256 field";
				return false;
			}
		}

		const JSON_VALUE* revisionIdentity =
			Find_Member(manifest, "revisionIdentity");
		if (nullptr == revisionIdentity || !Has_ExactMembers(*revisionIdentity,
			{ "kind", "algorithm", "identityPayloadPath",
			  "serverBootstrapContentRevision" }) ||
			!Is_String(Find_Member(*revisionIdentity, "kind"),
				"PARENT_MANIFEST") ||
			!Is_String(Find_Member(*revisionIdentity, "algorithm"),
				"SHA256_CANONICAL_JSON_WITH_EMPTY_REVISION_ID") ||
			!Is_String(Find_Member(*revisionIdentity, "identityPayloadPath"),
				"revision-identity.json") ||
			!Is_LowerSha256(Find_Member(
				*revisionIdentity, "serverBootstrapContentRevision")))
		{
			status = "Candidate parent revision identity contract is invalid";
			return false;
		}

		const JSON_VALUE* compatibility =
			Find_Member(manifest, "clientPresentationCompatibility");
		if (nullptr == compatibility || !Has_ExactMembers(*compatibility,
			{ "mode", "requiredLanes", "artifacts" }) ||
			!Is_String(Find_Member(*compatibility, "mode"),
				"BYTE_IDENTICAL_TO_ACTIVE") ||
			!Validate_ExactStringArray(Find_Member(*compatibility, "requiredLanes"),
				{ "ANIMATION", "EFFECT", "COMBAT_VISUAL", "CAMERA",
				  "WORLD_EVENT_SET" }))
		{
			status = "Candidate presentation alias contract is invalid";
			return false;
		}
		const JSON_VALUE* aliasArtifacts =
			Find_Member(*compatibility, "artifacts");
		std::set<std::string> coveredLanes;
		if (nullptr == aliasArtifacts || JSON_KIND::ARRAY != aliasArtifacts->eKind)
		{
			status = "Candidate presentation alias artifact list is invalid";
			return false;
		}
		for (const JSON_VALUE& row : aliasArtifacts->Elements)
		{
			if (!Has_ExactMembers(row,
				{ "lane", "path", "sha256", "bytes",
				  "repositorySourceSha256" }))
			{
				status = "Candidate presentation alias row fields are invalid";
				return false;
			}
			const JSON_VALUE* lane = Find_Member(row, "lane");
			const JSON_VALUE* path = Find_Member(row, "path");
			const JSON_VALUE* hash = Find_Member(row, "sha256");
			const JSON_VALUE* sourceHash =
				Find_Member(row, "repositorySourceSha256");
			if (nullptr == lane || JSON_KIND::STRING != lane->eKind ||
				nullptr == path || JSON_KIND::STRING != path->eKind ||
				!Is_SafeRelativePath(path->String) || !Is_LowerSha256(hash) ||
				!Is_LowerSha256(sourceHash) || hash->String != sourceHash->String ||
				!Is_Integer(Find_Member(row, "bytes"), 0u,
					(std::numeric_limits<std::uint64_t>::max)()))
			{
				status = "Candidate presentation alias identity is invalid";
				return false;
			}
			if (lane->String != "ANIMATION" && lane->String != "EFFECT" &&
				lane->String != "COMBAT_VISUAL" && lane->String != "CAMERA" &&
				lane->String != "WORLD_EVENT_SET")
			{
				status = "Candidate presentation alias lane is unknown";
				return false;
			}
			coveredLanes.insert(lane->String);
		}
		if (coveredLanes != std::set<std::string>{
			"ANIMATION", "EFFECT", "COMBAT_VISUAL", "CAMERA",
			"WORLD_EVENT_SET" })
		{
			status = "Candidate presentation alias lane coverage is incomplete";
			return false;
		}

		const JSON_VALUE* bootstrap =
			Find_Member(manifest, "serverGameplayBootstrap");
		if (nullptr == bootstrap || !Has_ExactMembers(*bootstrap,
			{ "path", "formatVersion", "baselineRowCount", "candidateRowCount",
			  "removedValtanRows", "addedValtanRows", "baselineSha256",
			  "candidateSha256" }) ||
			!Is_String(Find_Member(*bootstrap, "path"),
				"Runtime/Gameplay/Gameplay.bootstrap") ||
			!Is_Integer(Find_Member(*bootstrap, "formatVersion"),
				LostArk::Server::GAMEPLAY_BOOTSTRAP_VERSION,
				LostArk::Server::GAMEPLAY_BOOTSTRAP_VERSION) ||
			!Is_LowerSha256(Find_Member(*bootstrap, "baselineSha256")) ||
			!Is_LowerSha256(Find_Member(*bootstrap, "candidateSha256")) ||
			Find_Member(*bootstrap, "candidateSha256")->String !=
				Find_Member(*revisionIdentity,
					"serverBootstrapContentRevision")->String)
		{
			status = "Candidate Server gameplay bootstrap contract is invalid";
			return false;
		}
		for (const std::string_view field :
			{ "baselineRowCount", "candidateRowCount", "removedValtanRows",
			  "addedValtanRows" })
		{
			if (!Is_Integer(Find_Member(*bootstrap, field), 0u, 4096u))
			{
				status = "Candidate Server gameplay row count is invalid";
				return false;
			}
		}
		GameplayDataRevision manifestBaselineRevision{};
		if (!Try_Parse_GameplayDataRevision(
			Find_Member(*bootstrap, "baselineSha256")->String,
			manifestBaselineRevision))
		{
			status = "Candidate bootstrap baseline identity is malformed";
			return false;
		}

		const JSON_VALUE* artifacts = Find_Member(manifest, "artifacts");
		if (nullptr == artifacts || JSON_KIND::ARRAY != artifacts->eKind ||
			artifacts->Elements.empty())
		{
			status = "Candidate artifact manifest is empty";
			return false;
		}
		std::set<std::string> artifactPaths;
		for (const JSON_VALUE& row : artifacts->Elements)
		{
			if (!Has_ExactMembers(row, { "path", "sha256", "bytes" }))
			{
				status = "Candidate artifact row fields are invalid";
				return false;
			}
			const JSON_VALUE* relative = Find_Member(row, "path");
			const JSON_VALUE* hash = Find_Member(row, "sha256");
			const JSON_VALUE* bytes = Find_Member(row, "bytes");
			if (nullptr == relative || JSON_KIND::STRING != relative->eKind ||
				!Is_SafeRelativePath(relative->String) ||
				relative->String == "revision-manifest.json" ||
				relative->String == "revision-identity.json" ||
				!artifactPaths.insert(relative->String).second ||
				!Is_LowerSha256(hash) || !Is_Integer(bytes, 0u,
					(std::numeric_limits<std::uint64_t>::max)()))
			{
				status = "Candidate artifact identity is invalid";
				return false;
			}
			fs::path artifactPath;
			GameplayDataRevision artifactRevision{};
			error.clear();
			if (!Resolve_ExactRegularFile(candidateDirectory,
				relative->String, artifactPath, status) ||
				fs::file_size(artifactPath, error) != bytes->Integer || error ||
				!Hash_FileSha256(artifactPath, artifactRevision, status) ||
				Format_GameplayDataRevision(artifactRevision) != hash->String)
			{
				if (status.empty())
					status = "Candidate artifact hash or byte count is invalid";
				return false;
			}
		}
		std::string canonicalArtifacts;
		Append_CanonicalJson(*artifacts, canonicalArtifacts);
		GameplayDataRevision artifactSetRevision{};
		if (!Hash_BytesSha256(std::span<const std::uint8_t>(
			reinterpret_cast<const std::uint8_t*>(canonicalArtifacts.data()),
			canonicalArtifacts.size()), artifactSetRevision) ||
			Format_GameplayDataRevision(artifactSetRevision) !=
				Find_Member(manifest, "artifactSetId")->String)
		{
			status = "Candidate artifactSetId is invalid";
			return false;
		}
		for (const auto& pair :
			{ std::pair<std::string_view, std::string_view>{
				"serverSubmanifestSha256", "_manifest/server.json" },
			  std::pair<std::string_view, std::string_view>{
				"clientSubmanifestSha256", "_manifest/client.json" },
			  std::pair<std::string_view, std::string_view>{
				"authoringSubmanifestSha256", "_manifest/authoring.json" } })
		{
			fs::path submanifestPath;
			GameplayDataRevision submanifestRevision{};
			if (!Resolve_ExactRegularFile(candidateDirectory,
				std::string(pair.second), submanifestPath, status) ||
				!Hash_FileSha256(submanifestPath, submanifestRevision, status) ||
				Format_GameplayDataRevision(submanifestRevision) !=
					Find_Member(manifest, pair.first)->String)
			{
				status = "Candidate submanifest SHA-256 is invalid";
				return false;
			}
		}

		const std::string bootstrapRelative =
			Find_Member(*bootstrap, "path")->String;
		fs::path bootstrapPath;
		GameplayDataRevision bootstrapRevision{};
		error.clear();
		if (!Resolve_ExactRegularFile(candidateDirectory,
			bootstrapRelative, bootstrapPath, status) ||
			fs::file_size(bootstrapPath, error) > 64u * 1024u * 1024u || error ||
			!Hash_FileSha256(bootstrapPath, bootstrapRevision, status) ||
			Format_GameplayDataRevision(bootstrapRevision) !=
				Find_Member(*bootstrap, "candidateSha256")->String)
		{
			status = "Candidate gameplay bootstrap raw SHA-256 is invalid";
			return false;
		}
		if (!Build_NonValtanGameplayRevision(
			bootstrapPath, candidateNonValtanGameplayRevision, status) ||
			candidateNonValtanGameplayRevision !=
				activeNonValtanGameplayRevision)
		{
			if (status.empty())
				status = "Candidate changes gameplay outside allowedDomains VALTAN_BOSS";
			return false;
		}
		auto staged = std::make_shared<LostArk::Server::CGameplayCatalog>();
		if (nullptr == staged || !staged->Load_FromBootstrap(
			bootstrapPath, bootstrapRevision, request.CandidateRevision))
		{
			status = nullptr == staged ?
				"Candidate gameplay catalog allocation failed" : staged->Get_Status();
			return false;
		}
		candidate = std::move(staged);
		candidateBootstrapContentRevision = bootstrapRevision;
		status.clear();
		return true;
	}

	struct RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL final
	{
		std::uint32_t iTransactionSequence = 0u;
		LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION Base;
		LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION Candidate;
	};

	constexpr std::wstring_view RUNTIME_ACTIVE_POINTER_NAME =
		L"active-generation.json";
	constexpr std::wstring_view RUNTIME_ACTIVATION_JOURNAL_NAME =
		L"active-generation.journal.json";

	bool RuntimeGenerationEquals(
		const LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& left,
		const LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& right)
	{
		return left.eSource == right.eSource &&
			left.Revision == right.Revision &&
			left.BootstrapContentRevision == right.BootstrapContentRevision &&
			left.NonValtanGameplayRevision == right.NonValtanGameplayRevision;
	}

	std::string RuntimeSourceName(
		const LostArk::Server::RUNTIME_GAMEPLAY_GENERATION_SOURCE source)
	{
		using SOURCE = LostArk::Server::RUNTIME_GAMEPLAY_GENERATION_SOURCE;
		return SOURCE::CANDIDATE == source ?
			"CANDIDATE" : "PACKAGED_BASELINE";
	}

	void AppendRuntimeGenerationRecord(
		const LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& record,
		std::string& output)
	{
		output += "{\"nonValtanGameplayRevision\":";
		Append_CanonicalString(
			Format_GameplayDataRevision(record.NonValtanGameplayRevision), output);
		output += ",\"revisionId\":";
		Append_CanonicalString(
			Format_GameplayDataRevision(record.Revision), output);
		output += ",\"serverBootstrapContentRevision\":";
		Append_CanonicalString(
			Format_GameplayDataRevision(record.BootstrapContentRevision), output);
		output += ",\"sourceKind\":";
		Append_CanonicalString(RuntimeSourceName(record.eSource), output);
		output.push_back('}');
	}

	std::string BuildRuntimeActivePointerBytes(
		const LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& record)
	{
		std::string output = "{\"formatVersion\":1,\"generation\":";
		AppendRuntimeGenerationRecord(record, output);
		output +=
			",\"schema\":\"lostark.server-runtime-active-gameplay-generation\"}";
		return output;
	}

	std::string BuildRuntimeActivationJournalBytes(
		const RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL& journal)
	{
		std::string output = "{\"base\":";
		AppendRuntimeGenerationRecord(journal.Base, output);
		output += ",\"candidate\":";
		AppendRuntimeGenerationRecord(journal.Candidate, output);
		output +=
			",\"formatVersion\":1,\"schema\":\"lostark.server-runtime-gameplay-activation-journal\",\"transactionSequence\":";
		output += std::to_string(journal.iTransactionSequence);
		output.push_back('}');
		return output;
	}

	bool ParseRuntimeGenerationRecord(
		const JSON_VALUE* value,
		LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& record)
	{
		using SOURCE = LostArk::Server::RUNTIME_GAMEPLAY_GENERATION_SOURCE;
		if (nullptr == value || !Has_ExactMembers(*value,
			{ "nonValtanGameplayRevision", "revisionId",
			  "serverBootstrapContentRevision", "sourceKind" }) ||
			!Is_LowerSha256(Find_Member(*value, "nonValtanGameplayRevision")) ||
			!Is_LowerSha256(Find_Member(*value, "revisionId")) ||
			!Is_LowerSha256(
				Find_Member(*value, "serverBootstrapContentRevision")))
		{
			return false;
		}
		const JSON_VALUE* source = Find_Member(*value, "sourceKind");
		if (Is_String(source, "PACKAGED_BASELINE"))
			record.eSource = SOURCE::PACKAGED_BASELINE;
		else if (Is_String(source, "CANDIDATE"))
			record.eSource = SOURCE::CANDIDATE;
		else
			return false;
		return Try_Parse_GameplayDataRevision(
			Find_Member(*value, "revisionId")->String, record.Revision) &&
			Try_Parse_GameplayDataRevision(
				Find_Member(*value,
					"serverBootstrapContentRevision")->String,
				record.BootstrapContentRevision) &&
			Try_Parse_GameplayDataRevision(
				Find_Member(*value, "nonValtanGameplayRevision")->String,
				record.NonValtanGameplayRevision) && record.Is_Valid();
	}

	bool ParseCanonicalRuntimeJson(
		const std::vector<std::uint8_t>& bytes,
		JSON_VALUE& value,
		std::string& status)
	{
		if (!Parse_JsonBytes(bytes, value, status))
			return false;
		std::string canonical;
		canonical.reserve(bytes.size());
		Append_CanonicalJson(value, canonical);
		if (canonical.size() != bytes.size() ||
			!std::equal(canonical.begin(), canonical.end(), bytes.begin()))
		{
			status = "Runtime gameplay activation JSON is not canonical";
			return false;
		}
		return true;
	}

	bool ParseRuntimeActivePointer(
		const std::vector<std::uint8_t>& bytes,
		LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& record,
		std::string& status)
	{
		JSON_VALUE root;
		if (!ParseCanonicalRuntimeJson(bytes, root, status) ||
			!Has_ExactMembers(root,
				{ "formatVersion", "generation", "schema" }) ||
			!Is_Integer(Find_Member(root, "formatVersion"), 1u, 1u) ||
			!Is_String(Find_Member(root, "schema"),
				"lostark.server-runtime-active-gameplay-generation") ||
			!ParseRuntimeGenerationRecord(
				Find_Member(root, "generation"), record))
		{
			if (status.empty()) status = "Runtime active gameplay pointer is invalid";
			return false;
		}
		status.clear();
		return true;
	}

	bool ParseRuntimeActivationJournal(
		const std::vector<std::uint8_t>& bytes,
		RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL& journal,
		std::string& status)
	{
		JSON_VALUE root;
		if (!ParseCanonicalRuntimeJson(bytes, root, status) ||
			!Has_ExactMembers(root,
				{ "base", "candidate", "formatVersion", "schema",
				  "transactionSequence" }) ||
			!Is_Integer(Find_Member(root, "formatVersion"), 1u, 1u) ||
			!Is_Integer(Find_Member(root, "transactionSequence"), 1u,
				(std::numeric_limits<std::uint32_t>::max)()) ||
			!Is_String(Find_Member(root, "schema"),
				"lostark.server-runtime-gameplay-activation-journal") ||
			!ParseRuntimeGenerationRecord(
				Find_Member(root, "base"), journal.Base) ||
			!ParseRuntimeGenerationRecord(
				Find_Member(root, "candidate"), journal.Candidate) ||
			LostArk::Server::RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE !=
				journal.Candidate.eSource ||
			RuntimeGenerationEquals(journal.Base, journal.Candidate))
		{
			if (status.empty()) status = "Runtime gameplay activation journal is invalid";
			return false;
		}
		journal.iTransactionSequence = static_cast<std::uint32_t>(
			Find_Member(root, "transactionSequence")->Integer);
		status.clear();
		return true;
	}

	bool PrepareRuntimeGameplayRoot(
		const std::filesystem::path& requested,
		const bool create,
		std::filesystem::path& root,
		std::string& status)
	{
		namespace fs = std::filesystem;
		if (requested.empty() || !requested.is_absolute())
		{
			status = "Runtime gameplay activation root is not absolute";
			return false;
		}
		std::error_code error;
		const fs::path expected = requested.lexically_normal();
		if (create && !fs::create_directories(expected, error) && error)
		{
			status = "Runtime gameplay activation root cannot be created";
			return false;
		}
		if (!fs::exists(expected, error) || error)
		{
			status = create ?
				"Runtime gameplay activation root is unavailable" :
				"Runtime gameplay activation root is absent";
			return false;
		}
		const fs::file_status linkStatus = fs::symlink_status(expected, error);
		const fs::path canonical = fs::canonical(expected, error);
		const DWORD attributes = ::GetFileAttributesW(expected.c_str());
		if (error || !fs::is_directory(linkStatus) || fs::is_symlink(linkStatus) ||
			INVALID_FILE_ATTRIBUTES == attributes ||
			0u != (attributes & FILE_ATTRIBUTE_REPARSE_POINT) ||
			canonical != expected)
		{
			status = "Runtime gameplay activation root is non-canonical or reparsed";
			return false;
		}
		root = canonical;
		status.clear();
		return true;
	}

	bool InspectRuntimeGameplayFile(
		const std::filesystem::path& root,
		const std::wstring_view name,
		std::filesystem::path& path,
		bool& exists,
		std::string& status)
	{
		namespace fs = std::filesystem;
		path = (root / fs::path(name)).lexically_normal();
		if (path.parent_path() != root)
		{
			status = "Runtime gameplay activation file escaped its root";
			return false;
		}
		std::error_code error;
		const fs::file_status linkStatus = fs::symlink_status(path, error);
		if (error == std::errc::no_such_file_or_directory)
		{
			exists = false;
			status.clear();
			return true;
		}
		if (error)
		{
			status = "Runtime gameplay activation file status failed";
			return false;
		}
		exists = fs::exists(linkStatus);
		if (!exists)
		{
			status.clear();
			return true;
		}
		const DWORD attributes = ::GetFileAttributesW(path.c_str());
		if (!fs::is_regular_file(linkStatus) || fs::is_symlink(linkStatus) ||
			INVALID_FILE_ATTRIBUTES == attributes ||
			0u != (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
		{
			status = "Runtime gameplay activation file is not an exact regular file";
			return false;
		}
		status.clear();
		return true;
	}

	bool ReadOptionalRuntimeGameplayFile(
		const std::filesystem::path& root,
		const std::wstring_view name,
		bool& exists,
		std::vector<std::uint8_t>& bytes,
		std::string& status)
	{
		std::filesystem::path path;
		if (!InspectRuntimeGameplayFile(root, name, path, exists, status))
			return false;
		bytes.clear();
		return !exists || Read_BoundedFile(path, 64u * 1024u, bytes, status);
	}

	bool AtomicWriteRuntimeGameplayFile(
		const std::filesystem::path& root,
		const std::wstring_view name,
		const std::string_view bytes,
		std::string& status)
	{
		std::filesystem::path finalPath;
		bool finalExists = false;
		if (!InspectRuntimeGameplayFile(
			root, name, finalPath, finalExists, status))
			return false;
		const std::filesystem::path stagePath = finalPath.wstring() +
			L".stage." + std::to_wstring(::GetCurrentProcessId()) + L"." +
			std::to_wstring(::GetTickCount64());
		HANDLE file = ::CreateFileW(
			stagePath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, nullptr);
		if (INVALID_HANDLE_VALUE == file)
		{
			status = "Runtime gameplay activation stage file cannot be created";
			return false;
		}
		bool succeeded = true;
		std::size_t written = 0u;
		while (written < bytes.size())
		{
			const DWORD requestBytes = static_cast<DWORD>((std::min)(
				bytes.size() - written,
				static_cast<std::size_t>((std::numeric_limits<DWORD>::max)())));
			DWORD chunkBytes = 0u;
			if (!::WriteFile(file, bytes.data() + written, requestBytes,
				&chunkBytes, nullptr) || chunkBytes != requestBytes)
			{
				succeeded = false;
				break;
			}
			written += chunkBytes;
		}
		if (succeeded) succeeded = FALSE != ::FlushFileBuffers(file);
		if (!::CloseHandle(file)) succeeded = false;
		if (succeeded)
		{
			succeeded = FALSE != ::MoveFileExW(
				stagePath.c_str(), finalPath.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		}
		if (!succeeded)
		{
			(void)::DeleteFileW(stagePath.c_str());
			status = "Runtime gameplay activation atomic write failed";
			return false;
		}
		status.clear();
		return true;
	}

	void DeleteRuntimeGameplayJournal(
		const std::filesystem::path& root) noexcept
	{
		std::filesystem::path path;
		bool exists = false;
		std::string ignored;
		if (InspectRuntimeGameplayFile(root, RUNTIME_ACTIVATION_JOURNAL_NAME,
			path, exists, ignored) && exists)
		{
			(void)::DeleteFileW(path.c_str());
		}
	}

	bool DeleteRuntimeGameplayJournalExact(
		const std::filesystem::path& root,
		std::string& status)
	{
		std::filesystem::path path;
		bool exists = false;
		if (!InspectRuntimeGameplayFile(root, RUNTIME_ACTIVATION_JOURNAL_NAME,
			path, exists, status))
			return false;
		if (exists && !::DeleteFileW(path.c_str()))
		{
			status = "Runtime gameplay activation journal cannot be removed";
			return false;
		}
		if (!InspectRuntimeGameplayFile(root, RUNTIME_ACTIVATION_JOURNAL_NAME,
			path, exists, status) || exists)
		{
			if (status.empty())
				status = "Runtime gameplay activation journal removal was not durable";
			return false;
		}
		status.clear();
		return true;
	}
}

bool LostArk::Server::CServerApp::Acquire_RuntimeGameplayProcessMutex(
	void*& handle,
	std::string& status)
{
	handle = nullptr;
#ifdef _DEBUG
	constexpr const wchar_t* MUTEX_NAME =
		L"Local\\LostArk.Server.ValtanRuntimeActivation.Debug";
#else
	constexpr const wchar_t* MUTEX_NAME =
		L"Local\\LostArk.Server.ValtanRuntimeActivation.Release";
#endif
	HANDLE mutex = ::CreateMutexW(nullptr, FALSE, MUTEX_NAME);
	if (nullptr == mutex)
	{
		status = "Runtime gameplay process mutex cannot be created";
		return false;
	}
	const DWORD wait = ::WaitForSingleObject(mutex, 0u);
	if (WAIT_OBJECT_0 != wait && WAIT_ABANDONED != wait)
	{
		::CloseHandle(mutex);
		status = WAIT_TIMEOUT == wait ?
			"Another Server process owns the runtime gameplay activation lock" :
			"Runtime gameplay process mutex wait failed";
		return false;
	}
	handle = mutex;
	status.clear();
	return true;
}

void LostArk::Server::CServerApp::Release_RuntimeGameplayProcessMutex(
	void*& handle) noexcept
{
	if (nullptr == handle) return;
	HANDLE mutex = static_cast<HANDLE>(handle);
	(void)::ReleaseMutex(mutex);
	(void)::CloseHandle(mutex);
	handle = nullptr;
}

bool LostArk::Server::CServerApp::Persist_RuntimeGameplayActivation(
	const std::filesystem::path& runtimeRoot,
	const std::uint32_t transactionSequence,
	const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& base,
	const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& candidate,
	std::string& status)
{
	if (0u == transactionSequence || !base.Is_Valid() ||
		!candidate.Is_Valid() ||
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE != candidate.eSource ||
		RuntimeGenerationEquals(base, candidate))
	{
		status = "Runtime gameplay activation identity is invalid";
		return false;
	}
	std::filesystem::path root;
	if (!PrepareRuntimeGameplayRoot(runtimeRoot, true, root, status))
		return false;
	bool journalExists = false;
	std::vector<std::uint8_t> journalBytes;
	if (!ReadOptionalRuntimeGameplayFile(root,
		RUNTIME_ACTIVATION_JOURNAL_NAME, journalExists, journalBytes, status) ||
		journalExists)
	{
		if (status.empty())
			status = "An unfinished runtime gameplay activation journal exists";
		return false;
	}
	bool pointerExists = false;
	std::vector<std::uint8_t> pointerBytes;
	if (!ReadOptionalRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
		pointerExists, pointerBytes, status))
		return false;
	if (pointerExists)
	{
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION diskBase{};
		if (!ParseRuntimeActivePointer(pointerBytes, diskBase, status) ||
			!RuntimeGenerationEquals(diskBase, base))
		{
			if (status.empty())
				status = "Runtime active pointer does not match process active generation";
			return false;
		}
	}
	else if (RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE == base.eSource)
	{
		status = "Candidate process active generation has no durable pointer";
		return false;
	}

	RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL journal{};
	journal.iTransactionSequence = transactionSequence;
	journal.Base = base;
	journal.Candidate = candidate;
	if (!AtomicWriteRuntimeGameplayFile(root,
		RUNTIME_ACTIVATION_JOURNAL_NAME,
		BuildRuntimeActivationJournalBytes(journal), status))
		return false;
	if (!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
		BuildRuntimeActivePointerBytes(candidate), status))
	{
		std::string rollbackStatus;
		if (!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
			BuildRuntimeActivePointerBytes(base), rollbackStatus))
		{
			status += "; durable base-pointer rollback also failed: " +
				rollbackStatus;
			return false;
		}
		DeleteRuntimeGameplayJournal(root);
		return false;
	}
	status.clear();
	return true;
}

bool LostArk::Server::CServerApp::Rollback_RuntimeGameplayActivation(
	const std::filesystem::path& runtimeRoot,
	const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& base,
	std::string& status)
{
	if (!base.Is_Valid())
	{
		status = "Runtime gameplay rollback base is invalid";
		return false;
	}
	std::filesystem::path root;
	if (!PrepareRuntimeGameplayRoot(runtimeRoot, false, root, status) ||
		!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
			BuildRuntimeActivePointerBytes(base), status))
		return false;
	DeleteRuntimeGameplayJournal(root);
	status.clear();
	return true;
}

void LostArk::Server::CServerApp::Complete_RuntimeGameplayActivation(
	const std::filesystem::path& runtimeRoot) noexcept
{
	std::filesystem::path root;
	std::string ignored;
	if (PrepareRuntimeGameplayRoot(runtimeRoot, false, root, ignored))
		DeleteRuntimeGameplayJournal(root);
}

bool LostArk::Server::CServerApp::Recover_RuntimeActiveGameplayPointer(
	const std::filesystem::path& runtimeRoot,
	const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION& active,
	bool& hasPersistedPointer,
	std::string& status,
	const bool allowPackagedIdentityDrift)
{
	active = packaged;
	hasPersistedPointer = false;
	if (!packaged.Is_Valid() ||
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE != packaged.eSource)
	{
		status = "Packaged runtime gameplay identity is invalid";
		return false;
	}
	std::error_code existsError;
	if (!std::filesystem::exists(runtimeRoot, existsError))
	{
		if (existsError)
		{
			status = "Runtime gameplay activation root status failed";
			return false;
		}
		status.clear();
		return true;
	}
	std::filesystem::path root;
	if (!PrepareRuntimeGameplayRoot(runtimeRoot, false, root, status))
		return false;
	bool pointerExists = false;
	bool journalExists = false;
	std::vector<std::uint8_t> pointerBytes;
	std::vector<std::uint8_t> journalBytes;
	if (!ReadOptionalRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
		pointerExists, pointerBytes, status) ||
		!ReadOptionalRuntimeGameplayFile(root,
			RUNTIME_ACTIVATION_JOURNAL_NAME,
			journalExists, journalBytes, status))
		return false;

	RUNTIME_ACTIVE_GAMEPLAY_GENERATION pointer{};
	if (pointerExists && !ParseRuntimeActivePointer(
		pointerBytes, pointer, status))
		return false;
	if (!journalExists)
	{
		if (pointerExists) active = pointer;
		hasPersistedPointer = pointerExists;
		if (RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE ==
			active.eSource && !allowPackagedIdentityDrift &&
			!RuntimeGenerationEquals(active, packaged))
		{
			status = "Durable packaged pointer does not match packaged gameplay";
			return false;
		}
		status.clear();
		return true;
	}

	RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL journal{};
	if (!ParseRuntimeActivationJournal(journalBytes, journal, status))
		return false;
	if (pointerExists && RuntimeGenerationEquals(pointer, journal.Candidate))
		active = journal.Candidate;
	else if (pointerExists && RuntimeGenerationEquals(pointer, journal.Base))
		active = journal.Base;
	else if (!pointerExists &&
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE ==
			journal.Base.eSource && (allowPackagedIdentityDrift ||
			RuntimeGenerationEquals(journal.Base, packaged)))
		active = journal.Base;
	else
	{
		status = "Activation journal cannot recover to an exact old or new generation";
		return false;
	}
	hasPersistedPointer = pointerExists;
	if (RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE ==
		active.eSource && !allowPackagedIdentityDrift &&
		!RuntimeGenerationEquals(active, packaged))
	{
		status = "Recovered packaged generation does not match packaged gameplay";
		return false;
	}
	status.clear();
	return true;
}

bool LostArk::Server::CServerApp::Reset_RuntimeGameplayActivationToPackaged(
	const std::filesystem::path& runtimeRoot,
	const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
	std::string& status)
{
	if (!packaged.Is_Valid() ||
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE != packaged.eSource)
	{
		status = "Packaged runtime gameplay identity is invalid for reset";
		return false;
	}
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION recovered{};
	bool hadPointer = false;
	if (!Recover_RuntimeActiveGameplayPointer(
		runtimeRoot, packaged, recovered, hadPointer, status, true))
		return false;
	(void)hadPointer;

	/* This is the offline schema-upgrade escape hatch. Recovery above validates
	the durable pointer/journal structure and proves that the pointer names one
	of the journal's exact old/new identities. The discarded candidate artifacts
	must not be re-admitted: they may be absent, corrupt, or use an intentionally
	retired bootstrap schema. Only the newly validated packaged generation is
	selected below. Malformed or ambiguous pointer/journal bytes still fail
	closed before any durable write. */

	std::filesystem::path root;
	if (!PrepareRuntimeGameplayRoot(runtimeRoot, true, root, status))
		return false;
	/* Finish any exact old/new activation journal first. A crash before or after
	this write still leaves one fully named generation. Only then linearize the
	offline reset with one atomic packaged pointer replacement. */
	if (!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
		BuildRuntimeActivePointerBytes(recovered), status) ||
		!DeleteRuntimeGameplayJournalExact(root, status))
		return false;
	if (!RuntimeGenerationEquals(recovered, packaged) &&
		!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
			BuildRuntimeActivePointerBytes(packaged), status))
		return false;
	status.clear();
	return true;
}

bool LostArk::Server::CServerApp::Load_RuntimeActiveGameplayGeneration(
	const std::filesystem::path& runtimeRoot,
	const std::shared_ptr<const CGameplayCatalog>& packagedGeneration,
	const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
	std::shared_ptr<const CGameplayCatalog>& activeGeneration,
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION& active,
	bool& isCandidate,
	std::string& status)
{
	activeGeneration.reset();
	isCandidate = false;
	bool hasPersistedPointer = false;
	if (nullptr == packagedGeneration ||
		!Recover_RuntimeActiveGameplayPointer(runtimeRoot, packaged, active,
			hasPersistedPointer, status))
		return false;
	if (RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE == active.eSource)
	{
		activeGeneration = packagedGeneration;
		Complete_RuntimeGameplayActivation(runtimeRoot);
		status.clear();
		return true;
	}

	LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST request{};
	request.iTransactionSequence = 1u;
	request.BaseRevision = packaged.Revision;
	request.CandidateRevision = active.Revision;
	request.iRequiredPresentationLaneMask =
		LostArk::Shared::GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK;
	std::shared_ptr<const CGameplayCatalog> candidateGeneration;
	LostArk::Shared::GameplayDataRevision bootstrapRevision{};
	LostArk::Shared::GameplayDataRevision nonValtanRevision{};
	if (!Admit_ValtanCandidateGeneration(
		request, packaged.NonValtanGameplayRevision, candidateGeneration,
		bootstrapRevision, nonValtanRevision, status) ||
		bootstrapRevision != active.BootstrapContentRevision ||
		nonValtanRevision != active.NonValtanGameplayRevision ||
		!Validate_ValtanHotReloadBaseProfile(
			packagedGeneration->Find_Boss("BOSS_VALTAN"),
			nullptr == candidateGeneration ? nullptr :
				candidateGeneration->Find_Boss("BOSS_VALTAN"), status))
	{
		if (status.empty())
			status = "Durable runtime candidate failed startup admission";
		return false;
	}
	activeGeneration = std::move(candidateGeneration);
	isCandidate = true;
	Complete_RuntimeGameplayActivation(runtimeRoot);
	status.clear();
	return true;
}

std::filesystem::path
LostArk::Server::CServerApp::Resolve_RuntimeActiveGameplayRoot() const
{
	if (!m_RuntimeActiveGameplayRootOverride.empty())
		return m_RuntimeActiveGameplayRootOverride;
	const std::filesystem::path repositoryRoot = Resolve_RepositoryRoot();
	if (repositoryRoot.empty()) return {};
	#ifdef _DEBUG
	constexpr std::wstring_view configuration = L"Debug";
	#else
	constexpr std::wstring_view configuration = L"Release";
	#endif
	return (repositoryRoot / L"Intermediate" / L"ValtanTuningRuntime" /
		L"Server" / configuration).lexically_normal();
}

int LostArk::Server::CServerApp::Reset_ValtanRuntimeToPackaged()
{
#ifndef _DEBUG
	std::cerr << "Valtan runtime reset is unavailable in Release builds.\n";
	return 2;
#else
	void* processMutex = nullptr;
	std::string status;
	if (!Acquire_RuntimeGameplayProcessMutex(processMutex, status))
	{
		std::cerr << "Valtan runtime reset refused. Status=" << status << '\n';
		return 3;
	}
	const auto finish = [&processMutex](const int code)
	{
		Release_RuntimeGameplayProcessMutex(processMutex);
		return code;
	};
	auto packagedCatalog = std::make_shared<CGameplayCatalog>();
	if (nullptr == packagedCatalog || !packagedCatalog->Load())
	{
		std::cerr << "Packaged gameplay failed to load for Valtan reset. Status="
			<< (nullptr == packagedCatalog ? "allocation failed" :
				packagedCatalog->Get_Status()) << '\n';
		return finish(1);
	}
	const std::filesystem::path bootstrapPath =
		Resolve_RuntimeGameplayBootstrap();
	GameplayDataRevision nonValtanRevision{};
	if (bootstrapPath.empty() || !Build_NonValtanGameplayRevision(
		bootstrapPath, nonValtanRevision, status))
	{
		std::cerr << "Packaged gameplay identity failed for Valtan reset. Status="
			<< status << '\n';
		return finish(1);
	}
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION packaged{};
	packaged.eSource =
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
	packaged.Revision = packagedCatalog->Get_ActiveRevision();
	packaged.BootstrapContentRevision = packaged.Revision;
	packaged.NonValtanGameplayRevision = nonValtanRevision;
	const std::filesystem::path repositoryRoot = Resolve_RepositoryRoot();
	const std::filesystem::path runtimeRoot = repositoryRoot.empty() ?
		std::filesystem::path{} :
		(repositoryRoot / L"Intermediate" / L"ValtanTuningRuntime" /
			L"Server" / L"Debug").lexically_normal();
	if (runtimeRoot.empty() ||
		!Reset_RuntimeGameplayActivationToPackaged(
			runtimeRoot, packaged, status))
	{
		std::cerr << "Valtan runtime reset failed closed. Status="
			<< (status.empty() ? "runtime root unavailable" : status) << '\n';
		return finish(1);
	}
	std::cout << "Valtan runtime reset selected packaged gameplay revision "
		<< LostArk::Shared::Format_GameplayDataRevision(packaged.Revision)
		<< ". Restart Server normally.\n";
	return finish(0);
#endif
}

LostArk::Server::CServerApp::~CServerApp()
{
	Shutdown();
}

int LostArk::Server::CServerApp::Run(
	const std::uint32_t automaticShutdownMilliseconds,
	const std::string_view bindAddress,
	const std::uint16_t port,
	const bool headless)
{
	using LostArk::Shared::WORLD_ID;
#ifdef _DEBUG
	std::string processMutexStatus;
	if (!Acquire_RuntimeGameplayProcessMutex(
		m_hRuntimeGameplayProcessMutex, processMutexStatus))
	{
		std::cerr << "Server runtime gameplay lock acquisition failed. Status="
			<< processMutexStatus << '\n';
		return 1;
	}
#endif
	auto packagedGameplayCatalog = std::make_shared<CGameplayCatalog>();
	if (nullptr == packagedGameplayCatalog ||
		!packagedGameplayCatalog->Load())
	{
		std::cerr << "Process gameplay generation failed to initialize. Status="
			<< (nullptr == packagedGameplayCatalog ?
				"Gameplay catalog allocation failed" :
				packagedGameplayCatalog->Get_Status()) << '\n';
		return 1;
	}
	std::shared_ptr<const CGameplayCatalog> packagedGameplayGeneration =
		std::move(packagedGameplayCatalog);
	std::shared_ptr<const CGameplayCatalog> initialGameplayGeneration =
		packagedGameplayGeneration;
	GameplayDataRevision initialNonValtanGameplayRevision{};
	std::string nonValtanStatus;
	const std::filesystem::path runtimeGameplayBootstrap =
		Resolve_RuntimeGameplayBootstrap();
	if (runtimeGameplayBootstrap.empty() ||
		!Build_NonValtanGameplayRevision(
			runtimeGameplayBootstrap, initialNonValtanGameplayRevision,
			nonValtanStatus))
	{
		std::cerr << "Process non-Valtan gameplay identity failed to initialize. "
			"Status=" << (nonValtanStatus.empty() ?
				"Runtime gameplay bootstrap is unavailable" : nonValtanStatus)
			<< '\n';
		return 1;
	}
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION packagedRuntimeGeneration{};
	packagedRuntimeGeneration.eSource =
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
	packagedRuntimeGeneration.Revision =
		packagedGameplayGeneration->Get_ActiveRevision();
	packagedRuntimeGeneration.BootstrapContentRevision =
		packagedGameplayGeneration->Get_ActiveRevision();
	packagedRuntimeGeneration.NonValtanGameplayRevision =
		initialNonValtanGameplayRevision;
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION initialRuntimeGeneration =
		packagedRuntimeGeneration;
	bool initialGenerationIsCandidate = false;
#ifdef _DEBUG
	std::string runtimeRecoveryStatus;
	if (!Load_RuntimeActiveGameplayGeneration(
		Resolve_RuntimeActiveGameplayRoot(), packagedGameplayGeneration,
		packagedRuntimeGeneration, initialGameplayGeneration,
		initialRuntimeGeneration, initialGenerationIsCandidate,
		runtimeRecoveryStatus))
	{
		std::cerr << "Runtime active gameplay recovery failed closed. Status="
			<< runtimeRecoveryStatus << '\n';
		return 1;
	}
#endif

	std::map<WORLD_ID, std::shared_ptr<CGameRoom>> stagedSharedSimulations;
	const auto stageSharedSimulation =
		[&stagedSharedSimulations, &initialGameplayGeneration](
			const WORLD_ID worldId)
		{
			auto simulation = std::make_shared<CGameRoom>(
				worldId, initialGameplayGeneration);
			if (nullptr == simulation || !simulation->Is_Ready())
			{
				std::cerr << "World simulation failed to initialize. World="
					<< static_cast<unsigned>(worldId) << ", Status="
					<< (nullptr == simulation ?
						"Simulation allocation failed" : simulation->Get_Status())
					<< '\n';
				return false;
			}
			return stagedSharedSimulations.emplace(
				worldId, std::move(simulation)).second;
		};

	if (!stageSharedSimulation(WORLD_ID::BERN) ||
		!stageSharedSimulation(WORLD_ID::VALTAN_ARENA) ||
		!stageSharedSimulation(WORLD_ID::TRAINING_GROUND))
	{
		return 1;
	}

	// Character Select uses the same Server gameplay runtime as every other
	// world, but each admitted session receives a private simulation instance.
	// Construct one temporary instance here to retain startup fail-fast validation.
	{
		const auto validationSimulation =
			std::make_shared<CGameRoom>(
				WORLD_ID::CHARACTER_SELECT_ARENA, initialGameplayGeneration);
		if (nullptr == validationSimulation || !validationSimulation->Is_Ready())
		{
			std::cerr << "Character Select simulation failed to initialize. Status="
				<< (nullptr == validationSimulation ?
					"Simulation allocation failed" :
					validationSimulation->Get_Status())
				<< '\n';
			return 1;
		}
	}

	{
		std::scoped_lock lock{ m_SessionsMutex };
		m_SharedGameRooms = std::move(stagedSharedSimulations);
		m_pActiveGameplayGeneration = std::move(initialGameplayGeneration);
		m_ActiveGameplayBootstrapContentRevision =
			initialRuntimeGeneration.BootstrapContentRevision;
		m_ActiveNonValtanGameplayRevision =
			initialRuntimeGeneration.NonValtanGameplayRevision;
		m_isActiveGameplayGenerationFromCandidate =
			initialGenerationIsCandidate;
#ifdef _DEBUG
		m_isRuntimeActivePersistenceEnabled = true;
#else
		m_isRuntimeActivePersistenceEnabled = false;
#endif
		m_CharacterSelectArenas.clear();
		m_GameplayBindingBySessionId.clear();
	}

	if (!m_WinSockContext.Initialize())
	{
		std::cerr << "Failed to initialize WinSock 2.2\n";
		return 1;
	}
	if (0u == port || !m_TcpListener.Open(bindAddress, port))
	{
		std::cerr << "Failed to open TCP listener. Address="
			<< bindAddress << ':' << port << ", Error="
			<< m_TcpListener.Get_LastErrorCode() << '\n';
		return 1;
	}

	m_isRunning.store(true);
	m_RoomThread = std::thread(&CServerApp::Room_Loop, this);
	m_AcceptThread = std::thread(&CServerApp::Accept_Loop, this);
	const bool useHeadlessMode = headless ||
		0 == ::_isatty(::_fileno(stdin));
	std::cout << "Listening on " << bindAddress << ':' << port
		<< " with shared BERN, VALTAN_ARENA, TRAINING_GROUND and "
		<< "session-private CHARACTER_SELECT_ARENA simulations.";
	if (0u == automaticShutdownMilliseconds && useHeadlessMode)
	{
		std::cout << " Headless mode; terminate the process to stop.\n";
		while (m_isRunning.load())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
	}
	else if (0u == automaticShutdownMilliseconds)
	{
		std::cout << " Press Enter to stop.\n";
		std::cin.get();
	}
	else
	{
		std::cout << " Smoke timeout=" << automaticShutdownMilliseconds
			<< "ms.\n";
		std::this_thread::sleep_for(
			std::chrono::milliseconds(automaticShutdownMilliseconds));
	}
	Shutdown();
	return 0;
}

void LostArk::Server::CServerApp::Accept_Loop()
{
	while (m_isRunning.load())
	{
		const SOCKET clientSocket = m_TcpListener.Accept();
		if (INVALID_SOCKET == clientSocket)
		{
			if (m_isRunning.load())
			{
				std::cerr << "Accept failed. Error="
					<< m_TcpListener.Get_LastErrorCode() << '\n';
			}
			break;
		}

		const SESSION_ID sessionId = m_iNextSessionId.fetch_add(1);
		if (sessionId == INVALID_SESSION_ID)
		{
			::closesocket(clientSocket);
			continue;
		}
		auto session = std::make_shared<CClientSession>(
			sessionId,
			clientSocket,
			[this](const SESSION_ID id, const LostArk::Shared::PACKET_FRAME& frame)
			{
				On_SessionFrame(id, frame);
			},
			[this](const SESSION_ID id)
			{
				On_SessionClosed(id);
			});
		{
			std::scoped_lock lock{ m_SessionsMutex };
			m_Sessions.emplace(sessionId, session);
		}
		if (!session->Start())
		{
			session->Request_Close();
			On_SessionClosed(sessionId);
			continue;
		}
		std::cout << "Client connected. SessionId=" << sessionId << '\n';
	}
}

void LostArk::Server::CServerApp::Room_Loop()
{
	using namespace std::chrono;
	constexpr duration<double> FIXED_STEP_SECONDS{ 1.0 / 30.0 };
	constexpr float FIXED_DELTA_SECONDS =
		static_cast<float>(FIXED_STEP_SECONDS.count());
	const steady_clock::duration fixedStep =
		duration_cast<steady_clock::duration>(FIXED_STEP_SECONDS);
	steady_clock::time_point nextTick = steady_clock::now();

	while (m_isRunning.load())
	{
		nextTick += fixedStep;
		Tick_GameplaySimulations(FIXED_DELTA_SECONDS);
		Reap_ClosedSessions();
		std::this_thread::sleep_until(nextTick);
		if (steady_clock::now() > nextTick + fixedStep)
			nextTick = steady_clock::now();
	}
	Tick_GameplaySimulations(FIXED_DELTA_SECONDS);
	Reap_ClosedSessions();
}

void LostArk::Server::CServerApp::On_SessionFrame(
	const SESSION_ID sessionId,
	const LostArk::Shared::PACKET_FRAME& frame)
{
	using namespace LostArk::Shared;

	CPacketReader reader{ frame.Payload };
	if (frame.ePacketType == PACKET_TYPE::C2S_ENTER_WORLD)
	{
		C2S_ENTER_WORLD enterWorld{};
		if (!Read_Message(reader, enterWorld) ||
			0u != reader.Get_RemainingSize() ||
			enterWorld.iProtocolVersion != NETWORK_PROTOCOL_VERSION ||
			!Is_Known_World_Id(enterWorld.eWorldId))
		{
			Request_SessionClose(sessionId);
			return;
		}

		const std::shared_ptr<CGameRoom> targetSimulation =
			Acquire_EntrySimulation(sessionId, enterWorld.eWorldId);
		if (nullptr == targetSimulation ||
			!Bind_SessionSimulation(
				sessionId, enterWorld.eWorldId, targetSimulation))
		{
			Request_SessionClose(sessionId);
			return;
		}

		std::shared_ptr<CClientSession> session;
		{
			std::scoped_lock lock{ m_SessionsMutex };
			const auto iter = m_Sessions.find(sessionId);
			if (iter != m_Sessions.end())
				session = iter->second;
		}

		ROOM_COMMAND registerCommand{};
		registerCommand.eType = ROOM_COMMAND_TYPE::REGISTER_SESSION;
		registerCommand.iSessionId = sessionId;
		registerCommand.pSession = session;

		ROOM_COMMAND enterCommand{};
		enterCommand.eType = ROOM_COMMAND_TYPE::ENTER_WORLD;
		enterCommand.iSessionId = sessionId;
		enterCommand.EnterWorld = std::move(enterWorld);

		if (nullptr == session ||
			!targetSimulation->Enqueue(std::move(registerCommand)) ||
			!targetSimulation->Enqueue(std::move(enterCommand)))
		{
			// REGISTER may already be queued. LEAVE is ordered after it and
			// removes the weak session before a private arena can retire.
			ROOM_COMMAND rollbackCommand{};
			rollbackCommand.eType = ROOM_COMMAND_TYPE::LEAVE;
			rollbackCommand.iSessionId = sessionId;
			rollbackCommand.eLeaveReason =
				PLAYER_DESPAWN_REASON::DISCONNECTED;
			(void)targetSimulation->Enqueue(std::move(rollbackCommand));
			Unbind_SessionSimulation(sessionId, targetSimulation);
			Request_SessionClose(sessionId);
		}
		return;
	}

	ROOM_COMMAND command{};
	command.iSessionId = sessionId;
	if (frame.ePacketType == PACKET_TYPE::C2S_MOVE)
	{
		C2S_MOVE move{};
		if (!Read_Message(reader, move) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::MOVE;
		command.Move = move;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_USE_SKILL)
	{
		C2S_USE_SKILL useSkill{};
		if (!Read_Message(reader, useSkill) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::USE_SKILL;
		command.UseSkill = useSkill;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_RELEASE_SKILL)
	{
		C2S_RELEASE_SKILL releaseSkill{};
		if (!Read_Message(reader, releaseSkill) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::RELEASE_SKILL;
		command.ReleaseSkill = releaseSkill;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_UPDATE_SKILL_AIM)
	{
		C2S_UPDATE_SKILL_AIM updateSkillAim{};
		if (!Read_Message(reader, updateSkillAim) ||
			0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM;
		command.UpdateSkillAim = updateSkillAim;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_USE_ESTHER_SKILL)
	{
		C2S_USE_ESTHER_SKILL useEstherSkill{};
		if (!Read_Message(reader, useEstherSkill) ||
			0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::USE_ESTHER_SKILL;
		command.UseEstherSkill = useEstherSkill;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_REVIVE_PLAYER)
	{
		C2S_REVIVE_PLAYER revivePlayer{};
		if (!Read_Message(reader, revivePlayer) ||
			0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::REVIVE_PLAYER;
		command.RevivePlayer = revivePlayer;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_CHANGE_CHARACTER_CLASS)
	{
		C2S_CHANGE_CHARACTER_CLASS request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::CHANGE_CHARACTER_CLASS;
		command.ChangeCharacterClass = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_SPAWN_WORLD_ENTITY)
	{
		C2S_SPAWN_WORLD_ENTITY request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::SPAWN_WORLD_ENTITY;
		command.SpawnWorldEntity = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_VALTAN_AUDITION_REQUEST)
	{
		C2S_VALTAN_AUDITION_REQUEST request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::VALTAN_AUDITION;
		command.ValtanAudition = request;
	}
	else if (frame.ePacketType ==
		PACKET_TYPE::C2S_DATA_REVISION_PREPARE_REQUEST)
	{
		C2S_DATA_REVISION_PREPARE_REQUEST request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}

		std::shared_ptr<CClientSession> session;
		std::shared_ptr<CGameRoom> simulation;
		std::shared_ptr<const CGameplayCatalog> activeGeneration;
		GameplayDataRevision activeBootstrapContentRevision{};
		GameplayDataRevision activeNonValtanGameplayRevision{};
		{
			std::scoped_lock lock{ m_SessionsMutex };
			const auto sessionIter = m_Sessions.find(sessionId);
			const auto bindingIter =
				m_GameplayBindingBySessionId.find(sessionId);
			if (m_Sessions.end() != sessionIter)
				session = sessionIter->second;
			if (m_GameplayBindingBySessionId.end() != bindingIter)
				simulation = bindingIter->second.pSimulation;
			activeGeneration = m_pActiveGameplayGeneration;
			activeBootstrapContentRevision =
				m_ActiveGameplayBootstrapContentRevision;
			activeNonValtanGameplayRevision =
				m_ActiveNonValtanGameplayRevision;
		}
		if (nullptr == session || nullptr == simulation ||
			nullptr == activeGeneration)
		{
			Request_SessionClose(sessionId);
			return;
		}
		if (request.CandidateRevision == activeGeneration->Get_ActiveRevision())
		{
			/* A retried request may carry the old base after the original 2PC
			   committed. The wire forbids ABORTED when candidate==active, so the
			   only truthful typed terminal result is idempotent COMMITTED. */
			if (!Send_DataRevisionResult(session, request,
				DATA_REVISION_RESULT::COMMITTED,
				activeGeneration->Get_ActiveRevision(), {}))
				Request_SessionClose(sessionId);
			return;
		}
#ifndef _DEBUG
		if (!Send_DataRevisionResult(session, request,
			DATA_REVISION_RESULT::ABORTED,
			activeGeneration->Get_ActiveRevision(),
			"Release Server rejects gameplay data revision activation"))
		{
			Request_SessionClose(sessionId);
		}
		return;
#else
		if (WORLD_ID::VALTAN_ARENA != simulation->Get_WorldId() ||
			request.BaseRevision != activeGeneration->Get_ActiveRevision())
		{
			if (!Send_DataRevisionResult(session, request,
				DATA_REVISION_RESULT::ABORTED,
				activeGeneration->Get_ActiveRevision(),
				WORLD_ID::VALTAN_ARENA != simulation->Get_WorldId() ?
					"Data revision requester is not bound to Valtan Arena" :
					"Data revision base does not match process active generation"))
				Request_SessionClose(sessionId);
			return;
		}
		std::shared_ptr<const CGameplayCatalog> candidateGeneration;
		GameplayDataRevision candidateBootstrapContentRevision{};
		GameplayDataRevision candidateNonValtanGameplayRevision{};
		std::string admissionStatus;
		{
			std::scoped_lock admissionLock{ m_DataRevisionAdmissionMutex };
			if (!Admit_ValtanCandidateGeneration(
				request, activeNonValtanGameplayRevision, candidateGeneration,
				candidateBootstrapContentRevision,
				candidateNonValtanGameplayRevision, admissionStatus) ||
				!Validate_ValtanHotReloadBaseProfile(
					activeGeneration->Find_Boss("BOSS_VALTAN"),
					nullptr == candidateGeneration ? nullptr :
						candidateGeneration->Find_Boss("BOSS_VALTAN"),
					admissionStatus))
			{
				candidateGeneration.reset();
			}
		}
		if (nullptr == candidateGeneration)
		{
			if (!Send_DataRevisionResult(session, request,
				DATA_REVISION_RESULT::ABORTED,
				activeGeneration->Get_ActiveRevision(),
				admissionStatus.empty() ?
					"Candidate admission failed closed" : admissionStatus))
				Request_SessionClose(sessionId);
			return;
		}
		SERVER_CONTROL_EVENT event{};
		event.eKind = SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST;
		event.iSessionId = sessionId;
		event.RevisionRequest = request;
		event.pCandidateGeneration = std::move(candidateGeneration);
		event.BaseBootstrapContentRevision =
			activeBootstrapContentRevision;
		event.CandidateBootstrapContentRevision =
			candidateBootstrapContentRevision;
		event.BaseNonValtanGameplayRevision =
			activeNonValtanGameplayRevision;
		event.CandidateNonValtanGameplayRevision =
			candidateNonValtanGameplayRevision;
		if (!Queue_ServerControlEvent(std::move(event)))
		{
			if (!Send_DataRevisionResult(session, request,
				DATA_REVISION_RESULT::ABORTED,
				activeGeneration->Get_ActiveRevision(),
				"Server control ingress is full"))
				Request_SessionClose(sessionId);
		}
		return;
#endif
	}
	else if (frame.ePacketType ==
		PACKET_TYPE::C2S_DATA_REVISION_PREPARE_RESPONSE)
	{
		C2S_DATA_REVISION_PREPARE_RESPONSE response{};
		if (!Read_Message(reader, response) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
#ifdef _DEBUG
		SERVER_CONTROL_EVENT event{};
		event.eKind = SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE;
		event.iSessionId = sessionId;
		event.RevisionResponse = std::move(response);
		if (!Queue_ServerControlEvent(std::move(event)))
			Request_SessionClose(sessionId);
#else
		Request_SessionClose(sessionId);
#endif
		return;
	}
	else if (frame.ePacketType ==
		PACKET_TYPE::C2S_VALTAN_DECISION_TRACE_QUERY)
	{
		C2S_VALTAN_DECISION_TRACE_QUERY request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
#ifdef _DEBUG
		SERVER_CONTROL_EVENT event{};
		event.eKind = SERVER_CONTROL_EVENT_KIND::VALTAN_DECISION_TRACE_QUERY;
		event.iSessionId = sessionId;
		event.DecisionTraceQuery = std::move(request);
		if (!Queue_ServerControlEvent(std::move(event)))
			Request_SessionClose(sessionId);
#else
		std::shared_ptr<CClientSession> session;
		{
			std::scoped_lock lock{ m_SessionsMutex };
			const auto iter = m_Sessions.find(sessionId);
			if (m_Sessions.end() != iter) session = iter->second;
		}
		S2C_VALTAN_DECISION_TRACE_RESPONSE response{};
		response.iRequestSequence = request.iRequestSequence;
		response.strBossPlacementId = request.strBossPlacementId;
		response.eResult =
			VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_RELEASE_BUILD;
		CPacketWriter writer;
		if (nullptr == session || !Write_Message(writer, response) ||
			!session->Send_Frame(PACKET_TYPE::S2C_VALTAN_DECISION_TRACE_RESPONSE,
				writer.Get_Buffer()))
			Request_SessionClose(sessionId);
#endif
		return;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_GIVE_ITEM)
	{
		C2S_DEBUG_GIVE_ITEM request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_GIVE_ITEM;
		command.DebugGiveItem = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_USE_ITEM)
	{
		C2S_USE_ITEM request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::USE_ITEM;
		command.UseItem = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DESPAWN_ALL_WORLD_ENTITIES)
	{
		C2S_DESPAWN_ALL_WORLD_ENTITIES request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DESPAWN_ALL_WORLD_ENTITIES;
		command.DespawnAllWorldEntities = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_CONFIRM_NPC_ENTRY)
	{
		C2S_CONFIRM_NPC_ENTRY request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::CONFIRM_NPC_ENTRY;
		command.ConfirmNpcEntry = request;
	}
	else
	{
		Request_SessionClose(sessionId);
		return;
	}

	if (!Enqueue_AssignedCommand(sessionId, std::move(command)))
		Request_SessionClose(sessionId);
}

void LostArk::Server::CServerApp::On_SessionClosed(const SESSION_ID sessionId)
{
	{
		std::scoped_lock lock{ m_SessionsMutex };
		const auto bindingIter =
			m_GameplayBindingBySessionId.find(sessionId);
		if (bindingIter != m_GameplayBindingBySessionId.end())
		{
			if (nullptr != bindingIter->second.pSimulation)
			{
				ROOM_COMMAND command{};
				command.eType = ROOM_COMMAND_TYPE::LEAVE;
				command.iSessionId = sessionId;
				command.eLeaveReason =
					LostArk::Shared::PLAYER_DESPAWN_REASON::DISCONNECTED;
				(void)bindingIter->second.pSimulation->Enqueue(
					std::move(command));
			}
			m_GameplayBindingBySessionId.erase(bindingIter);
		}
	}

	// A private Character Select simulation owns the queued LEAVE until the
	// room thread consumes it, seals the empty queue, and retires the arena.
	{
		std::scoped_lock lock{ m_ClosedSessionMutex };
		m_ClosedSessionIds.push_back(sessionId);
	}
	SERVER_CONTROL_EVENT controlEvent{};
	controlEvent.eKind = SERVER_CONTROL_EVENT_KIND::SESSION_DISCONNECTED;
	controlEvent.iSessionId = sessionId;
	(void)Queue_ServerControlEvent(std::move(controlEvent));
}

void LostArk::Server::CServerApp::Request_SessionClose(const SESSION_ID sessionId)
{
	std::shared_ptr<CClientSession> session;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		const auto iter = m_Sessions.find(sessionId);
		if (iter != m_Sessions.end())
			session = iter->second;
	}
	if (nullptr != session)
		session->Request_Close();
}

void LostArk::Server::CServerApp::Reap_ClosedSessions()
{
	std::deque<SESSION_ID> closedIds;
	{
		std::scoped_lock lock{ m_ClosedSessionMutex };
		closedIds.swap(m_ClosedSessionIds);
	}
	for (const SESSION_ID sessionId : closedIds)
	{
		std::shared_ptr<CClientSession> session;
		{
			std::scoped_lock lock{ m_SessionsMutex };
			const auto iter = m_Sessions.find(sessionId);
			if (iter == m_Sessions.end())
				continue;
			session = std::move(iter->second);
			m_Sessions.erase(iter);
		}
		if (nullptr != session)
			session->Stop();
	}
}

std::shared_ptr<LostArk::Server::CGameRoom>
LostArk::Server::CServerApp::Acquire_EntrySimulation(
	const SESSION_ID sessionId,
	const LostArk::Shared::WORLD_ID worldId)
{
	using LostArk::Shared::WORLD_ID;

	if (!LostArk::Shared::Is_Known_World_Id(worldId))
		return nullptr;
	if (WORLD_ID::CHARACTER_SELECT_ARENA != worldId)
		return Find_SharedSimulation(worldId);

	std::shared_ptr<const CGameplayCatalog> activeGameplayGeneration;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		if (!m_Sessions.contains(sessionId))
			return nullptr;

		const auto bindingIter =
			m_GameplayBindingBySessionId.find(sessionId);
		if (bindingIter != m_GameplayBindingBySessionId.end())
		{
			const SESSION_GAMEPLAY_BINDING& binding = bindingIter->second;
			return binding.eWorldId == worldId &&
				binding.iPrivateArenaOwnerSessionId == sessionId ?
					binding.pSimulation : nullptr;
		}
		activeGameplayGeneration = m_pActiveGameplayGeneration;
	}

	if (nullptr == activeGameplayGeneration)
		return nullptr;
	auto simulation = std::make_shared<CGameRoom>(
		worldId, activeGameplayGeneration);
	if (nullptr == simulation || !simulation->Is_Ready())
	{
		std::cerr << "Private Character Select simulation failed. SessionId="
			<< sessionId << ", Status="
			<< (nullptr == simulation ?
				"Simulation allocation failed" : simulation->Get_Status())
			<< '\n';
		return nullptr;
	}
	return simulation;
}

std::shared_ptr<LostArk::Server::CGameRoom>
LostArk::Server::CServerApp::Find_SharedSimulation(
	const LostArk::Shared::WORLD_ID worldId)
{
	std::scoped_lock lock{ m_SessionsMutex };
	const auto iter = m_SharedGameRooms.find(worldId);
	return iter == m_SharedGameRooms.end() ? nullptr : iter->second;
}

bool LostArk::Server::CServerApp::Bind_SessionSimulation(
	const SESSION_ID sessionId,
	const LostArk::Shared::WORLD_ID worldId,
	const std::shared_ptr<CGameRoom>& simulation)
{
	using LostArk::Shared::WORLD_ID;

	if (nullptr == simulation ||
		!LostArk::Shared::Is_Known_World_Id(worldId) ||
		simulation->Get_WorldId() != worldId)
	{
		return false;
	}

	std::scoped_lock lock{ m_SessionsMutex };
	if (!m_Sessions.contains(sessionId))
		return false;
	if (nullptr == m_pActiveGameplayGeneration ||
		nullptr == simulation->Get_ActiveGameplayGeneration() ||
		simulation->Get_ActiveGameplayGeneration()->Get_ActiveRevision() !=
			m_pActiveGameplayGeneration->Get_ActiveRevision())
	{
		return false;
	}

	const SESSION_ID privateOwnerSessionId =
		WORLD_ID::CHARACTER_SELECT_ARENA == worldId ?
			sessionId : INVALID_SESSION_ID;
	const auto existingBinding =
		m_GameplayBindingBySessionId.find(sessionId);
	if (existingBinding != m_GameplayBindingBySessionId.end())
	{
		return existingBinding->second.eWorldId == worldId &&
			existingBinding->second.iPrivateArenaOwnerSessionId ==
				privateOwnerSessionId &&
			existingBinding->second.pSimulation == simulation;
	}

	bool insertedPrivateArena = false;
	if (WORLD_ID::CHARACTER_SELECT_ARENA == worldId)
	{
		const auto [arenaIter, inserted] =
			m_CharacterSelectArenas.emplace(sessionId, simulation);
		if (!inserted && arenaIter->second != simulation)
			return false;
		insertedPrivateArena = inserted;
	}
	else
	{
		const auto sharedIter = m_SharedGameRooms.find(worldId);
		if (sharedIter == m_SharedGameRooms.end() ||
			sharedIter->second != simulation)
		{
			return false;
		}
	}

	SESSION_GAMEPLAY_BINDING binding{};
	binding.eWorldId = worldId;
	binding.iPrivateArenaOwnerSessionId = privateOwnerSessionId;
	binding.pSimulation = simulation;
	const auto [bindingIter, insertedBinding] =
		m_GameplayBindingBySessionId.emplace(sessionId, std::move(binding));
	(void)bindingIter;
	if (!insertedBinding)
	{
		if (insertedPrivateArena)
			m_CharacterSelectArenas.erase(sessionId);
		return false;
	}
	return true;
}

bool LostArk::Server::CServerApp::Enqueue_AssignedCommand(
	const SESSION_ID sessionId,
	ROOM_COMMAND command)
{
	if (command.iSessionId != sessionId)
		return false;

	// Keep lookup plus Enqueue atomic with Transfer_SessionWorld. Both paths
	// use m_SessionsMutex -> CGameRoom::m_CommandMutex ordering.
	std::scoped_lock lock{ m_SessionsMutex };
	const auto iter = m_GameplayBindingBySessionId.find(sessionId);
	return iter != m_GameplayBindingBySessionId.end() &&
		nullptr != iter->second.pSimulation &&
		iter->second.pSimulation->Enqueue(std::move(command));
}

bool LostArk::Server::CServerApp::Queue_ServerControlEvent(
	SERVER_CONTROL_EVENT&& event)
{
	std::scoped_lock lock{ m_ServerControlMutex };
	if (m_ServerControlEvents.size() >= MAX_SERVER_CONTROL_EVENTS)
		return false;
	m_ServerControlEvents.push_back(std::move(event));
	return true;
}

bool LostArk::Server::CServerApp::Resolve_CandidateArtifactForAdmission(
	const std::filesystem::path& candidateDirectory,
	const std::string& relativePath,
	std::filesystem::path& resolvedPath,
	std::string& status)
{
	return Resolve_ExactRegularFile(
		candidateDirectory, relativePath, resolvedPath, status);
}

bool LostArk::Server::CServerApp::
Build_NonValtanGameplayRevisionForAdmission(
	const std::filesystem::path& bootstrapPath,
	LostArk::Shared::GameplayDataRevision& revision,
	std::string& status)
{
	return Build_NonValtanGameplayRevision(bootstrapPath, revision, status);
}

bool LostArk::Server::CServerApp::Hash_GameplayFileForAdmission(
	const std::filesystem::path& path,
	LostArk::Shared::GameplayDataRevision& revision,
	std::string& status)
{
	return Hash_FileSha256(path, revision, status);
}

bool LostArk::Server::CServerApp::Validate_ValtanHotReloadBaseProfile(
	const BOSS_RUNTIME_PROFILE* activeProfile,
	const BOSS_RUNTIME_PROFILE* candidateProfile,
	std::string& status)
{
	/* These values are copied into entity health, combat, and movement state at
	   spawn, so swapping only the immutable definition cannot apply them. */
	if (nullptr == activeProfile || nullptr == candidateProfile)
	{
		status = "Valtan HOT_RELOAD base profile is unavailable; "
			"ENCOUNTER_RESET required";
		return false;
	}
	if (activeProfile->iMaximumHp != candidateProfile->iMaximumHp ||
		activeProfile->iMaximumHealthBars !=
			candidateProfile->iMaximumHealthBars ||
		activeProfile->iAttackPower != candidateProfile->iAttackPower ||
		activeProfile->fCollisionRadius != candidateProfile->fCollisionRadius ||
		activeProfile->fEngageDistance != candidateProfile->fEngageDistance ||
		activeProfile->fMoveSpeed != candidateProfile->fMoveSpeed)
	{
		status = "Candidate changes live Valtan base fields; "
			"ENCOUNTER_RESET required";
		return false;
	}
	status.clear();
	return true;
}

bool LostArk::Server::CServerApp::Send_DataRevisionPrepare(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request)
{
	using namespace LostArk::Shared;
	if (nullptr == session) return false;
	S2C_DATA_REVISION_PREPARE prepare{};
	prepare.iTransactionSequence = request.iTransactionSequence;
	prepare.BaseRevision = request.BaseRevision;
	prepare.CandidateRevision = request.CandidateRevision;
	prepare.iRequiredPresentationLaneMask =
		request.iRequiredPresentationLaneMask;
	CPacketWriter writer;
	return Write_Message(writer, prepare) && session->Send_Frame(
		PACKET_TYPE::S2C_DATA_REVISION_PREPARE, writer.Get_Buffer());
}

bool LostArk::Server::CServerApp::Send_DataRevisionResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request,
	const LostArk::Shared::DATA_REVISION_RESULT result,
	const LostArk::Shared::GameplayDataRevision& activeRevision,
	std::string reason)
{
	using namespace LostArk::Shared;
	if (nullptr == session) return false;
	if (DATA_REVISION_RESULT::COMMITTED == result)
		reason.clear();
	else
	{
		if (reason.empty()) reason = "Data revision transaction aborted";
		if (reason.size() > MAX_DATA_REVISION_REASON_BYTES)
			reason.resize(MAX_DATA_REVISION_REASON_BYTES);
	}
	S2C_DATA_REVISION_RESULT message{};
	message.iTransactionSequence = request.iTransactionSequence;
	message.CandidateRevision = request.CandidateRevision;
	message.ActiveRevision = activeRevision;
	message.eResult = result;
	message.strReason = std::move(reason);
	CPacketWriter writer;
	return Write_Message(writer, message) && session->Send_Frame(
		PACKET_TYPE::S2C_DATA_REVISION_RESULT, writer.Get_Buffer());
}

void LostArk::Server::CServerApp::Process_ValtanDecisionTraceQuery(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request)
{
	using namespace LostArk::Shared;
	std::shared_ptr<CClientSession> session;
	std::shared_ptr<CGameRoom> simulation;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		const auto sessionIter = m_Sessions.find(sessionId);
		const auto bindingIter = m_GameplayBindingBySessionId.find(sessionId);
		if (m_Sessions.end() != sessionIter) session = sessionIter->second;
		if (m_GameplayBindingBySessionId.end() != bindingIter)
			simulation = bindingIter->second.pSimulation;
	}
	if (nullptr == session) return;
	S2C_VALTAN_DECISION_TRACE_RESPONSE response{};
	response.iRequestSequence = request.iRequestSequence;
	response.strBossPlacementId = request.strBossPlacementId;
	std::string status;
	if (nullptr == simulation)
		response.eResult =
			VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_WRONG_WORLD;
	else if (!simulation->Build_ValtanDecisionTraceResponse(
		request, response, status))
	{
		std::cerr << "Valtan decision trace query failed closed. SessionId="
			<< sessionId << ", Reason=" << status << '\n';
		Request_SessionClose(sessionId);
		return;
	}
	CPacketWriter writer;
	if (!Write_Message(writer, response) || !session->Send_Frame(
		PACKET_TYPE::S2C_VALTAN_DECISION_TRACE_RESPONSE, writer.Get_Buffer()))
		Request_SessionClose(sessionId);
}

bool LostArk::Server::CServerApp::Validate_DataRevisionTransactionMembership(
	std::string& status)
{
	if (!m_DataRevisionTransaction.Is_Active())
	{
		status = "Data revision transaction is inactive";
		return false;
	}
	std::scoped_lock lock{ m_SessionsMutex };
	if (nullptr == m_pActiveGameplayGeneration ||
		m_pActiveGameplayGeneration->Get_ActiveRevision() !=
			m_DataRevisionTransaction.Request.BaseRevision ||
		m_ActiveGameplayBootstrapContentRevision !=
			m_DataRevisionTransaction.BaseBootstrapContentRevision ||
		m_ActiveNonValtanGameplayRevision !=
			m_DataRevisionTransaction.BaseNonValtanGameplayRevision ||
		m_ActiveNonValtanGameplayRevision !=
			m_DataRevisionTransaction.CandidateNonValtanGameplayRevision)
	{
		status = "Process active gameplay generation changed during prepare";
		return false;
	}
	std::size_t boundParticipantCount = 0u;
	for (const auto& [sessionId, binding] : m_GameplayBindingBySessionId)
	{
		if (m_Sessions.contains(sessionId) && nullptr != binding.pSimulation)
			++boundParticipantCount;
	}
	if (boundParticipantCount !=
		m_DataRevisionTransaction.Participants.size())
	{
		status = "Bound data revision participant set changed during prepare";
		return false;
	}
	for (const DATA_REVISION_PARTICIPANT& participant :
		m_DataRevisionTransaction.Participants)
	{
		const auto sessionIter = m_Sessions.find(participant.iSessionId);
		const auto bindingIter =
			m_GameplayBindingBySessionId.find(participant.iSessionId);
		if (m_Sessions.end() == sessionIter ||
			sessionIter->second != participant.pSession ||
			m_GameplayBindingBySessionId.end() == bindingIter ||
			bindingIter->second.eWorldId != participant.eWorldId ||
			bindingIter->second.pSimulation != participant.pSimulation)
		{
			status = "Data revision participant disconnected or changed rooms";
			return false;
		}
	}
	std::set<const CGameRoom*> currentRooms;
	for (const auto& [worldId, room] : m_SharedGameRooms)
	{
		(void)worldId;
		if (nullptr != room) currentRooms.insert(room.get());
	}
	for (const auto& [sessionId, room] : m_CharacterSelectArenas)
	{
		(void)sessionId;
		if (nullptr != room) currentRooms.insert(room.get());
	}
	if (currentRooms.size() != m_DataRevisionTransaction.Simulations.size())
	{
		status = "Process gameplay room set changed during prepare";
		return false;
	}
	for (const auto& room : m_DataRevisionTransaction.Simulations)
	{
		if (nullptr == room || !currentRooms.contains(room.get()))
		{
			status = "Prepared gameplay room is no longer process-owned";
			return false;
		}
	}
	status.clear();
	return true;
}

void LostArk::Server::CServerApp::Abort_DataRevisionTransaction(
	std::string reason)
{
	using namespace LostArk::Shared;
	if (!m_DataRevisionTransaction.Is_Active()) return;
	DATA_REVISION_TRANSACTION transaction =
		std::move(m_DataRevisionTransaction);
	m_DataRevisionTransaction = {};
	const auto tombstoneExpiry = std::chrono::steady_clock::now() +
		std::chrono::seconds(10);
	for (const DATA_REVISION_PARTICIPANT& participant :
		transaction.Participants)
	{
		if (participant.hasResponded)
			continue;
		const auto existing = std::find_if(
			m_DataRevisionResponseTombstones.begin(),
			m_DataRevisionResponseTombstones.end(),
			[&transaction, &participant](
				const DATA_REVISION_RESPONSE_TOMBSTONE& tombstone)
			{
				return tombstone.iSessionId == participant.iSessionId &&
					tombstone.iTransactionSequence ==
						transaction.Request.iTransactionSequence &&
					tombstone.CandidateRevision ==
						transaction.Request.CandidateRevision;
			});
		if (m_DataRevisionResponseTombstones.end() != existing)
		{
			existing->ExpiresAt = tombstoneExpiry;
			continue;
		}
		if (m_DataRevisionResponseTombstones.size() >=
			MAX_DATA_REVISION_RESPONSE_TOMBSTONES)
		{
			m_DataRevisionResponseTombstones.pop_front();
		}
		m_DataRevisionResponseTombstones.push_back({
			participant.iSessionId,
			transaction.Request.iTransactionSequence,
			transaction.Request.CandidateRevision,
			transaction.Request.iRequiredPresentationLaneMask,
			tombstoneExpiry });
	}
	for (const auto& simulation : transaction.Simulations)
		if (nullptr != simulation)
			simulation->Abort_GameplayGeneration(
				transaction.Request.iTransactionSequence);
	GameplayDataRevision activeRevision = transaction.Request.BaseRevision;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		if (nullptr != m_pActiveGameplayGeneration)
			activeRevision = m_pActiveGameplayGeneration->Get_ActiveRevision();
	}
	if (reason.empty()) reason = "Data revision transaction aborted";
	for (const DATA_REVISION_PARTICIPANT& participant :
		transaction.Participants)
	{
		if (!Send_DataRevisionResult(participant.pSession, transaction.Request,
			DATA_REVISION_RESULT::ABORTED, activeRevision, reason))
			Request_SessionClose(participant.iSessionId);
	}
}

bool LostArk::Server::CServerApp::Commit_DataRevisionTransaction()
{
	using namespace LostArk::Shared;
	if (!m_DataRevisionTransaction.Is_Active()) return false;
	std::string status;
	if (!Validate_DataRevisionTransactionMembership(status))
	{
		Abort_DataRevisionTransaction(std::move(status));
		return false;
	}

	RUNTIME_ACTIVE_GAMEPLAY_GENERATION baseRuntime{};
	baseRuntime.eSource = m_isActiveGameplayGenerationFromCandidate ?
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE :
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
	baseRuntime.Revision = m_DataRevisionTransaction.Request.BaseRevision;
	baseRuntime.BootstrapContentRevision =
		m_DataRevisionTransaction.BaseBootstrapContentRevision;
	baseRuntime.NonValtanGameplayRevision =
		m_DataRevisionTransaction.BaseNonValtanGameplayRevision;
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION candidateRuntime{};
	candidateRuntime.eSource =
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE;
	candidateRuntime.Revision =
		m_DataRevisionTransaction.Request.CandidateRevision;
	candidateRuntime.BootstrapContentRevision =
		m_DataRevisionTransaction.CandidateBootstrapContentRevision;
	candidateRuntime.NonValtanGameplayRevision =
		m_DataRevisionTransaction.CandidateNonValtanGameplayRevision;

	bool stageSucceeded = true;
	{
		/* Freeze registration only while every room performs its allocation-free
		preflight. Durable pointer I/O happens after this lock is released. */
		std::scoped_lock lock{ m_SessionsMutex };
		if (nullptr == m_pActiveGameplayGeneration ||
			m_pActiveGameplayGeneration->Get_ActiveRevision() !=
				m_DataRevisionTransaction.Request.BaseRevision)
		{
			status = "Process active generation changed before room commit";
			stageSucceeded = false;
		}
		for (const auto& simulation : m_DataRevisionTransaction.Simulations)
		{
			if (!stageSucceeded) break;
			std::string roomStatus;
			if (nullptr == simulation ||
				!simulation->Stage_GameplayGeneration(
					m_DataRevisionTransaction.Request.iTransactionSequence,
					m_DataRevisionTransaction.Request.BaseRevision,
					m_DataRevisionTransaction.pCandidateGeneration,
					roomStatus))
			{
				status = roomStatus.empty() ?
					"A process gameplay room rejected generation stage" :
					roomStatus;
				stageSucceeded = false;
			}
		}
		if (!stageSucceeded)
			for (const auto& simulation : m_DataRevisionTransaction.Simulations)
				if (nullptr != simulation)
					simulation->Abort_GameplayGeneration(
						m_DataRevisionTransaction.Request.iTransactionSequence);
	}
	if (!stageSucceeded)
	{
		Abort_DataRevisionTransaction(std::move(status));
		return false;
	}

	const std::filesystem::path runtimeRoot =
		Resolve_RuntimeActiveGameplayRoot();
	bool durablePointerPromoted = false;
	if (m_isRuntimeActivePersistenceEnabled)
	{
		if (!Persist_RuntimeGameplayActivation(
			runtimeRoot,
			m_DataRevisionTransaction.Request.iTransactionSequence,
			baseRuntime, candidateRuntime, status))
		{
			for (const auto& simulation : m_DataRevisionTransaction.Simulations)
				if (nullptr != simulation)
					simulation->Abort_GameplayGeneration(
						m_DataRevisionTransaction.Request.iTransactionSequence);
			Abort_DataRevisionTransaction(status.empty() ?
				"Runtime active pointer promotion failed" : std::move(status));
			return false;
		}
		durablePointerPromoted = true;
	}

	bool cohortStillExact = true;
	{
		/* This is the process-wide room-tick publication boundary. Revalidate
		the full cohort after disk I/O while registration is frozen, then perform
		only bounded pointer swaps. No file or socket I/O occurs under this lock. */
		std::scoped_lock lock{ m_SessionsMutex };
		if (nullptr == m_pActiveGameplayGeneration ||
			m_pActiveGameplayGeneration->Get_ActiveRevision() !=
				m_DataRevisionTransaction.Request.BaseRevision ||
			m_ActiveGameplayBootstrapContentRevision !=
				m_DataRevisionTransaction.BaseBootstrapContentRevision ||
			m_ActiveNonValtanGameplayRevision !=
				m_DataRevisionTransaction.BaseNonValtanGameplayRevision)
		{
			cohortStillExact = false;
			status = "Process active generation changed during durable promotion";
		}
		std::size_t boundCount = 0u;
		for (const auto& [sessionId, binding] : m_GameplayBindingBySessionId)
			if (m_Sessions.contains(sessionId) && nullptr != binding.pSimulation)
				++boundCount;
		if (boundCount != m_DataRevisionTransaction.Participants.size())
		{
			cohortStillExact = false;
			status = "Bound participant set changed during durable promotion";
		}
		for (const DATA_REVISION_PARTICIPANT& participant :
			m_DataRevisionTransaction.Participants)
		{
			const auto session = m_Sessions.find(participant.iSessionId);
			const auto binding =
				m_GameplayBindingBySessionId.find(participant.iSessionId);
			if (m_Sessions.end() == session ||
				session->second != participant.pSession ||
				m_GameplayBindingBySessionId.end() == binding ||
				binding->second.eWorldId != participant.eWorldId ||
				binding->second.pSimulation != participant.pSimulation)
			{
				cohortStillExact = false;
				status = "Prepared participant changed during durable promotion";
				break;
			}
		}
		std::set<const CGameRoom*> currentRooms;
		for (const auto& [worldId, room] : m_SharedGameRooms)
		{
			(void)worldId;
			if (nullptr != room) currentRooms.insert(room.get());
		}
		for (const auto& [sessionId, room] : m_CharacterSelectArenas)
		{
			(void)sessionId;
			if (nullptr != room) currentRooms.insert(room.get());
		}
		if (currentRooms.size() !=
			m_DataRevisionTransaction.Simulations.size())
		{
			cohortStillExact = false;
			status = "Process room set changed during durable promotion";
		}
		for (const auto& simulation : m_DataRevisionTransaction.Simulations)
			if (nullptr == simulation || !currentRooms.contains(simulation.get()))
			{
				cohortStillExact = false;
				status = "Prepared room left during durable promotion";
				break;
			}

		if (!cohortStillExact)
		{
			for (const auto& simulation :
				m_DataRevisionTransaction.Simulations)
				if (nullptr != simulation)
					simulation->Abort_GameplayGeneration(
						m_DataRevisionTransaction.Request.iTransactionSequence);
		}
		else
		{
			for (const auto& simulation :
				m_DataRevisionTransaction.Simulations)
			{
				if (!simulation->Commit_GameplayGeneration(
					m_DataRevisionTransaction.Request.iTransactionSequence))
				{
					/* Durable new already exists. A split in-memory generation can
					never be hidden as ABORT; restart recovery will select new. */
					std::cerr << "FATAL: partial gameplay generation commit" << '\n';
					std::terminate();
				}
			}
			m_pActiveGameplayGeneration =
				m_DataRevisionTransaction.pCandidateGeneration;
			m_ActiveGameplayBootstrapContentRevision =
				m_DataRevisionTransaction.CandidateBootstrapContentRevision;
			m_ActiveNonValtanGameplayRevision =
				m_DataRevisionTransaction.CandidateNonValtanGameplayRevision;
			m_isActiveGameplayGenerationFromCandidate = true;
		}
	}
	if (!cohortStillExact)
	{
		if (durablePointerPromoted)
		{
			std::string rollbackStatus;
			if (!Rollback_RuntimeGameplayActivation(
				runtimeRoot, baseRuntime, rollbackStatus))
			{
				std::cerr << "FATAL: runtime gameplay pointer rollback failed: "
					<< rollbackStatus << '\n';
				std::terminate();
			}
		}
		Abort_DataRevisionTransaction(std::move(status));
		return false;
	}
	if (durablePointerPromoted)
		Complete_RuntimeGameplayActivation(runtimeRoot);

	DATA_REVISION_TRANSACTION transaction =
		std::move(m_DataRevisionTransaction);
	m_DataRevisionTransaction = {};
	const GameplayDataRevision committedRevision =
		transaction.pCandidateGeneration->Get_ActiveRevision();
	for (const DATA_REVISION_PARTICIPANT& participant :
		transaction.Participants)
	{
		if (!Send_DataRevisionResult(participant.pSession, transaction.Request,
			DATA_REVISION_RESULT::COMMITTED, committedRevision, {}))
			Request_SessionClose(participant.iSessionId);
	}
	return true;
}

void LostArk::Server::CServerApp::Advance_ServerControlTransactions()
{
	using namespace LostArk::Shared;
	std::deque<SERVER_CONTROL_EVENT> events;
	{
		std::scoped_lock lock{ m_ServerControlMutex };
		events.swap(m_ServerControlEvents);
	}
	const auto now = std::chrono::steady_clock::now();
	m_DataRevisionResponseTombstones.erase(
		std::remove_if(
			m_DataRevisionResponseTombstones.begin(),
			m_DataRevisionResponseTombstones.end(),
			[now](const DATA_REVISION_RESPONSE_TOMBSTONE& tombstone)
			{
				return tombstone.ExpiresAt <= now;
			}),
		m_DataRevisionResponseTombstones.end());
	const auto abortCurrent =
		[this](std::string reason)
		{
			Abort_DataRevisionTransaction(std::move(reason));
		};
	for (SERVER_CONTROL_EVENT& event : events)
	{
		if (SERVER_CONTROL_EVENT_KIND::VALTAN_DECISION_TRACE_QUERY ==
			event.eKind)
		{
			Process_ValtanDecisionTraceQuery(
				event.iSessionId, event.DecisionTraceQuery);
			continue;
		}
		if (SERVER_CONTROL_EVENT_KIND::SESSION_DISCONNECTED == event.eKind)
		{
			if (m_DataRevisionTransaction.Is_Active() &&
				std::any_of(
					m_DataRevisionTransaction.Participants.begin(),
					m_DataRevisionTransaction.Participants.end(),
					[&event](const DATA_REVISION_PARTICIPANT& participant)
					{
						return participant.iSessionId == event.iSessionId;
					}))
			{
				abortCurrent(
					"A prepared client disconnected before commit");
			}
			continue;
		}
		if (SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE == event.eKind)
		{
			const C2S_DATA_REVISION_PREPARE_RESPONSE& response =
				event.RevisionResponse;
			if (!m_DataRevisionTransaction.Is_Active())
			{
				const auto answeredAbortedTransaction = std::find_if(
					m_DataRevisionResponseTombstones.begin(),
					m_DataRevisionResponseTombstones.end(),
					[&event, &response](
						const DATA_REVISION_RESPONSE_TOMBSTONE& identity)
					{
						const bool validReady =
							DATA_REVISION_PREPARE_STATUS::READY ==
								response.eStatus &&
							(response.iPreparedPresentationLaneMask &
								identity.iRequiredPresentationLaneMask) ==
								identity.iRequiredPresentationLaneMask &&
							0u == (response.iFailedPresentationLaneMask &
								identity.iRequiredPresentationLaneMask);
						const bool validNack =
							DATA_REVISION_PREPARE_STATUS::NACK ==
								response.eStatus;
						return (validReady || validNack) &&
							identity.iSessionId == event.iSessionId &&
							identity.iTransactionSequence ==
								response.iTransactionSequence &&
							identity.CandidateRevision ==
								response.CandidateRevision &&
							identity.iRequiredPresentationLaneMask ==
								response.iRequiredPresentationLaneMask;
					});
				if (m_DataRevisionResponseTombstones.end() ==
					answeredAbortedTransaction)
				{
					Request_SessionClose(event.iSessionId);
				}
				else
				{
					/* Consume the one response that was valid when sent. A later
					   duplicate is unsolicited and keeps the normal fail-close policy. */
					m_DataRevisionResponseTombstones.erase(
						answeredAbortedTransaction);
				}
				continue;
			}
			auto participant = std::find_if(
				m_DataRevisionTransaction.Participants.begin(),
				m_DataRevisionTransaction.Participants.end(),
				[&event](const DATA_REVISION_PARTICIPANT& value)
				{
					return value.iSessionId == event.iSessionId;
				});
			if (m_DataRevisionTransaction.Participants.end() == participant ||
				participant->hasResponded ||
				response.iTransactionSequence !=
					m_DataRevisionTransaction.Request.iTransactionSequence ||
				response.CandidateRevision !=
					m_DataRevisionTransaction.Request.CandidateRevision ||
				response.iRequiredPresentationLaneMask !=
					m_DataRevisionTransaction.Request.iRequiredPresentationLaneMask)
			{
				abortCurrent(
					"Duplicate, stale, or mismatched client prepare response");
				Request_SessionClose(event.iSessionId);
				continue;
			}
			participant->hasResponded = true;
			if (DATA_REVISION_PREPARE_STATUS::NACK == response.eStatus)
			{
				abortCurrent(response.strReason.empty() ?
					"A client rejected the candidate presentation generation" :
					response.strReason);
				continue;
			}
			const std::uint32_t required =
				m_DataRevisionTransaction.Request.iRequiredPresentationLaneMask;
			if ((response.iPreparedPresentationLaneMask & required) != required ||
				0u != (response.iFailedPresentationLaneMask & required))
			{
				abortCurrent(
					"Client READY did not prepare every required presentation lane");
				Request_SessionClose(event.iSessionId);
				continue;
			}
			participant->isReady = true;
			continue;
		}
		if (SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST != event.eKind)
			continue;

		if (m_DataRevisionTransaction.Is_Active())
		{
			const bool exactDuplicate =
				m_DataRevisionTransaction.iRequesterSessionId == event.iSessionId &&
				m_DataRevisionTransaction.Request.iTransactionSequence ==
					event.RevisionRequest.iTransactionSequence &&
				m_DataRevisionTransaction.Request.CandidateRevision ==
					event.RevisionRequest.CandidateRevision;
			if (exactDuplicate) continue;
			std::shared_ptr<CClientSession> requester;
			GameplayDataRevision active{};
			{
				std::scoped_lock lock{ m_SessionsMutex };
				const auto iter = m_Sessions.find(event.iSessionId);
				if (m_Sessions.end() != iter) requester = iter->second;
				if (nullptr != m_pActiveGameplayGeneration)
					active = m_pActiveGameplayGeneration->Get_ActiveRevision();
			}
			if (active.Is_Valid() &&
				event.RevisionRequest.CandidateRevision != active &&
				!Send_DataRevisionResult(requester, event.RevisionRequest,
					DATA_REVISION_RESULT::ABORTED, active,
					"Another gameplay data revision transaction is active"))
				Request_SessionClose(event.iSessionId);
			continue;
		}

		DATA_REVISION_TRANSACTION staged{};
		staged.iRequesterSessionId = event.iSessionId;
		staged.Request = event.RevisionRequest;
		staged.pCandidateGeneration = std::move(event.pCandidateGeneration);
		staged.BaseBootstrapContentRevision =
			event.BaseBootstrapContentRevision;
		staged.CandidateBootstrapContentRevision =
			event.CandidateBootstrapContentRevision;
		staged.BaseNonValtanGameplayRevision =
			event.BaseNonValtanGameplayRevision;
		staged.CandidateNonValtanGameplayRevision =
			event.CandidateNonValtanGameplayRevision;
		std::string beginStatus;
		{
			std::scoped_lock lock{ m_SessionsMutex };
			const auto requesterSession = m_Sessions.find(event.iSessionId);
			const auto requesterBinding =
				m_GameplayBindingBySessionId.find(event.iSessionId);
			if (m_Sessions.end() == requesterSession ||
				m_GameplayBindingBySessionId.end() == requesterBinding ||
				WORLD_ID::VALTAN_ARENA != requesterBinding->second.eWorldId ||
				nullptr == requesterBinding->second.pSimulation)
				beginStatus = "Data revision requester left Valtan Arena";
			else if (nullptr == m_pActiveGameplayGeneration ||
				m_pActiveGameplayGeneration->Get_ActiveRevision() !=
					staged.Request.BaseRevision)
				beginStatus = "Data revision base is no longer process active";
			else if (!staged.BaseBootstrapContentRevision.Is_Valid() ||
				!staged.CandidateBootstrapContentRevision.Is_Valid() ||
				m_ActiveGameplayBootstrapContentRevision !=
					staged.BaseBootstrapContentRevision)
				beginStatus = "Candidate bootstrap baseline is no longer process active";
			else if (!staged.BaseNonValtanGameplayRevision.Is_Valid() ||
				!staged.CandidateNonValtanGameplayRevision.Is_Valid() ||
				m_ActiveNonValtanGameplayRevision !=
					staged.BaseNonValtanGameplayRevision ||
				m_ActiveNonValtanGameplayRevision !=
					staged.CandidateNonValtanGameplayRevision)
				beginStatus = "Candidate changes gameplay outside allowedDomains VALTAN_BOSS";
			else if (nullptr == staged.pCandidateGeneration ||
				staged.pCandidateGeneration->Get_ActiveRevision() !=
					staged.Request.CandidateRevision)
				beginStatus = "Admitted candidate parent revision is inconsistent";
			else
			{
				staged.Participants.reserve(m_GameplayBindingBySessionId.size());
				for (const auto& [participantId, binding] :
					m_GameplayBindingBySessionId)
				{
					const auto sessionIter = m_Sessions.find(participantId);
					if (m_Sessions.end() == sessionIter ||
						nullptr == binding.pSimulation)
						continue;
					DATA_REVISION_PARTICIPANT participant{};
					participant.iSessionId = participantId;
					participant.eWorldId = binding.eWorldId;
					participant.pSession = sessionIter->second;
					participant.pSimulation = binding.pSimulation;
					staged.Participants.push_back(std::move(participant));
				}
				std::sort(staged.Participants.begin(), staged.Participants.end(),
					[](const DATA_REVISION_PARTICIPANT& left,
						const DATA_REVISION_PARTICIPANT& right)
					{
						return left.iSessionId < right.iSessionId;
					});
				for (const auto& [worldId, simulation] : m_SharedGameRooms)
				{
					(void)worldId;
					if (nullptr != simulation)
						staged.Simulations.push_back(simulation);
				}
				for (const auto& [ownerSessionId, simulation] :
					m_CharacterSelectArenas)
				{
					(void)ownerSessionId;
					if (nullptr != simulation)
						staged.Simulations.push_back(simulation);
				}
				const bool requesterParticipates = std::any_of(
					staged.Participants.begin(), staged.Participants.end(),
					[&event](const DATA_REVISION_PARTICIPANT& participant)
					{
						return participant.iSessionId == event.iSessionId;
					});
				if (!requesterParticipates || staged.Simulations.empty())
					beginStatus = "Data revision participant or room set is empty";
			}
		}
		if (!beginStatus.empty())
		{
			std::shared_ptr<CClientSession> requester;
			GameplayDataRevision active = staged.Request.BaseRevision;
			{
				std::scoped_lock lock{ m_SessionsMutex };
				const auto iter = m_Sessions.find(event.iSessionId);
				if (m_Sessions.end() != iter) requester = iter->second;
				if (nullptr != m_pActiveGameplayGeneration)
					active = m_pActiveGameplayGeneration->Get_ActiveRevision();
			}
			if (active == staged.Request.CandidateRevision ||
				!Send_DataRevisionResult(requester, staged.Request,
					DATA_REVISION_RESULT::ABORTED, active, beginStatus))
				Request_SessionClose(event.iSessionId);
			continue;
		}
		staged.Deadline = std::chrono::steady_clock::now() +
			std::chrono::seconds(3);
		m_DataRevisionTransaction = std::move(staged);
		bool prepareSent = true;
		for (const DATA_REVISION_PARTICIPANT& participant :
			m_DataRevisionTransaction.Participants)
		{
			if (!Send_DataRevisionPrepare(
				participant.pSession, m_DataRevisionTransaction.Request))
			{
				prepareSent = false;
				Request_SessionClose(participant.iSessionId);
				break;
			}
		}
		if (!prepareSent)
			abortCurrent(
				"A participant could not receive the prepare message");
	}

	if (!m_DataRevisionTransaction.Is_Active()) return;
	std::string membershipStatus;
	if (!Validate_DataRevisionTransactionMembership(membershipStatus))
	{
		abortCurrent(std::move(membershipStatus));
		return;
	}
	if (std::chrono::steady_clock::now() >=
		m_DataRevisionTransaction.Deadline)
	{
		abortCurrent(
			"Data revision prepare timed out before every client was ready");
		return;
	}
	const bool allReady = std::all_of(
		m_DataRevisionTransaction.Participants.begin(),
		m_DataRevisionTransaction.Participants.end(),
		[](const DATA_REVISION_PARTICIPANT& participant)
		{
			return participant.isReady;
		});
	if (allReady)
		(void)Commit_DataRevisionTransaction();
}

void LostArk::Server::CServerApp::Tick_GameplaySimulations(
	const float fixedDeltaSeconds)
{
	/* Control transactions commit before any room consumes this fixed step, so
	   every process room observes one generation on this tick boundary. */
	Advance_ServerControlTransactions();
	std::vector<std::shared_ptr<CGameRoom>> simulations;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		simulations.reserve(
			m_SharedGameRooms.size() + m_CharacterSelectArenas.size());
		for (const auto& [worldId, simulation] : m_SharedGameRooms)
		{
			(void)worldId;
			if (nullptr != simulation)
				simulations.push_back(simulation);
		}
		for (const auto& [sessionId, simulation] : m_CharacterSelectArenas)
		{
			(void)sessionId;
			if (nullptr != simulation)
				simulations.push_back(simulation);
		}
	}

	// The room thread is the only writer of gameplay state. The mutex is
	// released before Tick so receive and session threads never wait on a tick.
	for (const std::shared_ptr<CGameRoom>& simulation : simulations)
	{
		simulation->Tick(fixedDeltaSeconds);
		Handle_WorldTransfers(simulation);
	}
	Retire_QuiescentCharacterSelectArenas();
}

void LostArk::Server::CServerApp::Retire_QuiescentCharacterSelectArenas()
{
	// Lock order is ServerApp sessions mutex -> room command mutex. Try_Seal
	// serializes the empty decision with each receive-thread Enqueue.
	std::scoped_lock lock{ m_SessionsMutex };
	for (auto iter = m_CharacterSelectArenas.begin();
		iter != m_CharacterSelectArenas.end();)
	{
		const SESSION_ID ownerSessionId = iter->first;
		const std::shared_ptr<CGameRoom>& simulation = iter->second;
		const auto bindingIter =
			m_GameplayBindingBySessionId.find(ownerSessionId);
		const bool isStillBound =
			bindingIter != m_GameplayBindingBySessionId.end() &&
			bindingIter->second.pSimulation == simulation;

		if (isStillBound || nullptr == simulation ||
			!simulation->Try_SealPrivateArenaForRetirement())
		{
			++iter;
			continue;
		}
		iter = m_CharacterSelectArenas.erase(iter);
	}
}

void LostArk::Server::CServerApp::Handle_WorldTransfers(
	const std::shared_ptr<CGameRoom>& sourceSimulation)
{
	if (nullptr == sourceSimulation)
		return;

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	while (sourceSimulation->Try_DequeueWorldTransfer(transfer))
	{
		if (!Transfer_SessionWorld(sourceSimulation, transfer))
			Request_SessionClose(transfer.iSessionId);
	}
}

bool LostArk::Server::CServerApp::Transfer_SessionWorld(
	const std::shared_ptr<CGameRoom>& sourceSimulation,
	const SERVER_WORLD_TRANSFER_REQUEST& transfer)
{
	using namespace LostArk::Shared;

	if (nullptr == sourceSimulation ||
		CHARACTER_CLASS_ID::END == transfer.eCharacterClass ||
		transfer.strNickName.empty())
	{
		return false;
	}

	const WORLD_ID sourceWorldId = sourceSimulation->Get_WorldId();
	const std::shared_ptr<CGameRoom> targetSimulation =
		Find_SharedSimulation(transfer.eTargetWorldId);
	if (nullptr == targetSimulation ||
		targetSimulation == sourceSimulation ||
		sourceWorldId == transfer.eTargetWorldId)
	{
		return false;
	}

	// Enqueue_AssignedCommand and On_SessionClosed use the same mutex. Route
	// lookup, queue ordering, and the binding commit therefore have one order.
	std::scoped_lock lock{ m_SessionsMutex };
	const auto sessionIter = m_Sessions.find(transfer.iSessionId);
	const auto bindingIter =
		m_GameplayBindingBySessionId.find(transfer.iSessionId);
	if (sessionIter == m_Sessions.end() ||
		bindingIter == m_GameplayBindingBySessionId.end() ||
		bindingIter->second.eWorldId != sourceWorldId ||
		bindingIter->second.pSimulation != sourceSimulation ||
		(WORLD_ID::CHARACTER_SELECT_ARENA == sourceWorldId &&
			bindingIter->second.iPrivateArenaOwnerSessionId !=
				transfer.iSessionId))
	{
		return false;
	}

	ROOM_COMMAND registerCommand{};
	registerCommand.eType = ROOM_COMMAND_TYPE::REGISTER_SESSION;
	registerCommand.iSessionId = transfer.iSessionId;
	registerCommand.pSession = sessionIter->second;
	if (!targetSimulation->Enqueue(std::move(registerCommand)))
		return false;

	C2S_ENTER_WORLD enterWorld{};
	enterWorld.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	enterWorld.eWorldId = transfer.eTargetWorldId;
	enterWorld.eCharacterClass = transfer.eCharacterClass;
	enterWorld.strNickName = transfer.strNickName;
	ROOM_COMMAND enterCommand{};
	enterCommand.eType = ROOM_COMMAND_TYPE::ENTER_WORLD;
	enterCommand.iSessionId = transfer.iSessionId;
	enterCommand.EnterWorld = std::move(enterWorld);
	if (!targetSimulation->Enqueue(std::move(enterCommand)))
	{
		ROOM_COMMAND targetRollback{};
		targetRollback.eType = ROOM_COMMAND_TYPE::LEAVE;
		targetRollback.iSessionId = transfer.iSessionId;
		targetRollback.eLeaveReason =
			PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
		(void)targetSimulation->Enqueue(std::move(targetRollback));
		return false;
	}

	if (!sourceSimulation->Commit_WorldTransferDeparture(
		transfer.iSessionId))
	{
		ROOM_COMMAND targetRollback{};
		targetRollback.eType = ROOM_COMMAND_TYPE::LEAVE;
		targetRollback.iSessionId = transfer.iSessionId;
		targetRollback.eLeaveReason =
			PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
		(void)targetSimulation->Enqueue(std::move(targetRollback));
		return false;
	}

	bindingIter->second.eWorldId = transfer.eTargetWorldId;
	bindingIter->second.iPrivateArenaOwnerSessionId = INVALID_SESSION_ID;
	bindingIter->second.pSimulation = targetSimulation;
	return true;
}

void LostArk::Server::CServerApp::Unbind_SessionSimulation(
	const SESSION_ID sessionId,
	const std::shared_ptr<CGameRoom>& expectedSimulation)
{
	std::scoped_lock lock{ m_SessionsMutex };
	const auto iter = m_GameplayBindingBySessionId.find(sessionId);
	if (iter == m_GameplayBindingBySessionId.end())
		return;
	if (nullptr != expectedSimulation &&
		iter->second.pSimulation != expectedSimulation)
	{
		return;
	}
	m_GameplayBindingBySessionId.erase(iter);
}

void LostArk::Server::CServerApp::Shutdown()
{
	m_isRunning.store(false);
	m_TcpListener.Close();
	if (m_AcceptThread.joinable())
		m_AcceptThread.join();
	if (m_RoomThread.joinable())
		m_RoomThread.join();
	Abort_DataRevisionTransaction("Server shutdown aborted data revision prepare");

	std::vector<std::shared_ptr<CClientSession>> sessions;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		for (auto& [sessionId, session] : m_Sessions)
		{
			(void)sessionId;
			sessions.push_back(std::move(session));
		}
		m_Sessions.clear();
		m_GameplayBindingBySessionId.clear();
	}
	for (const auto& session : sessions)
	{
		if (nullptr != session)
			session->Request_Close();
	}
	for (const auto& session : sessions)
	{
		if (nullptr != session)
			session->Stop();
	}

	// Receive callbacks and the room thread are stopped before simulations are
	// destroyed, so no producer can enqueue into an unowned room.
	{
		std::scoped_lock lock{ m_SessionsMutex };
		m_CharacterSelectArenas.clear();
		m_SharedGameRooms.clear();
		m_pActiveGameplayGeneration.reset();
		m_ActiveGameplayBootstrapContentRevision = {};
		m_ActiveNonValtanGameplayRevision = {};
		m_isActiveGameplayGenerationFromCandidate = false;
		m_isRuntimeActivePersistenceEnabled = false;
	}
	m_DataRevisionResponseTombstones.clear();
	{
		std::scoped_lock lock{ m_ClosedSessionMutex };
		m_ClosedSessionIds.clear();
	}
	{
		std::scoped_lock lock{ m_ServerControlMutex };
		m_ServerControlEvents.clear();
	}
	m_WinSockContext.Shutdown();
	Release_RuntimeGameplayProcessMutex(m_hRuntimeGameplayProcessMutex);
}
