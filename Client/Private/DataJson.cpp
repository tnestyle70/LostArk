#include "DataJson.h"

#include <charconv>
#include <cmath>
#include <cstdio>

namespace
{
	using namespace Client;

	void AppendUtf8(string& output, const uint32_t codePoint)
	{
		if (codePoint <= 0x7fu)
		{
			output.push_back(static_cast<char>(codePoint));
		}
		else if (codePoint <= 0x7ffu)
		{
			output.push_back(static_cast<char>(0xc0u | (codePoint >> 6u)));
			output.push_back(static_cast<char>(0x80u | (codePoint & 0x3fu)));
		}
		else if (codePoint <= 0xffffu)
		{
			output.push_back(static_cast<char>(0xe0u | (codePoint >> 12u)));
			output.push_back(static_cast<char>(
				0x80u | ((codePoint >> 6u) & 0x3fu)));
			output.push_back(static_cast<char>(0x80u | (codePoint & 0x3fu)));
		}
		else
		{
			output.push_back(static_cast<char>(0xf0u | (codePoint >> 18u)));
			output.push_back(static_cast<char>(
				0x80u | ((codePoint >> 12u) & 0x3fu)));
			output.push_back(static_cast<char>(
				0x80u | ((codePoint >> 6u) & 0x3fu)));
			output.push_back(static_cast<char>(0x80u | (codePoint & 0x3fu)));
		}
	}

	class JSON_READER final
	{
	public:
		explicit JSON_READER(const string_view text,
			const DATA_JSON_PARSE_LIMITS& limits)
			: m_Text(text), m_Limits(limits)
		{
		}

		bool_t Read(DATA_JSON_VALUE& outValue, string& outError)
		{
			SkipWhitespace();
			if (!ReadValue(0u, outValue))
			{
				outError = FormatError();
				return false;
			}
			SkipWhitespace();
			if (m_Position != m_Text.size())
			{
				SetError("Trailing data after the root value");
				outError = FormatError();
				return false;
			}
			outError.clear();
			return true;
		}

	private:
		bool_t ReadValue(
			const size_t depth,
			DATA_JSON_VALUE& outValue)
		{
			if (depth > m_Limits.iMaximumDepth)
				return SetError("Maximum nesting depth exceeded");
			if (++m_ValueCount > m_Limits.iMaximumValues)
				return SetError("Maximum value count exceeded");

			SkipWhitespace();
			if (m_Position >= m_Text.size())
				return SetError("Unexpected end of input");

			switch (m_Text[m_Position])
			{
			case 'n':
				if (!ReadLiteral("null"))
					return false;
				outValue = DATA_JSON_VALUE::Null();
				return true;
			case 't':
				if (!ReadLiteral("true"))
					return false;
				outValue = DATA_JSON_VALUE::Boolean(true);
				return true;
			case 'f':
				if (!ReadLiteral("false"))
					return false;
				outValue = DATA_JSON_VALUE::Boolean(false);
				return true;
			case '"':
			{
				string value;
				if (!ReadString(value))
					return false;
				outValue = DATA_JSON_VALUE::String(move(value));
				return true;
			}
			case '[':
				return ReadArray(depth, outValue);
			case '{':
				return ReadObject(depth, outValue);
			default:
				return ReadNumber(outValue);
			}
		}

		bool_t ReadArray(
			const size_t depth,
			DATA_JSON_VALUE& outValue)
		{
			++m_Position;
			DATA_JSON_VALUE::ARRAY values;
			SkipWhitespace();
			if (Consume(']'))
			{
				outValue = DATA_JSON_VALUE::Array(move(values));
				return true;
			}

			while (true)
			{
				DATA_JSON_VALUE value;
				if (!ReadValue(depth + 1u, value))
					return false;
				values.push_back(move(value));
				SkipWhitespace();
				if (Consume(']'))
					break;
				if (!Consume(','))
					return SetError("Expected ',' or ']' in array");
				SkipWhitespace();
				if (Peek(']'))
					return SetError("Trailing comma in array");
			}

			outValue = DATA_JSON_VALUE::Array(move(values));
			return true;
		}

		bool_t ReadObject(
			const size_t depth,
			DATA_JSON_VALUE& outValue)
		{
			++m_Position;
			DATA_JSON_VALUE::OBJECT values;
			SkipWhitespace();
			if (Consume('}'))
			{
				outValue = DATA_JSON_VALUE::Object(move(values));
				return true;
			}

			while (true)
			{
				string key;
				if (!ReadString(key))
					return false;
				SkipWhitespace();
				if (!Consume(':'))
					return SetError("Expected ':' after object key");

				DATA_JSON_VALUE value;
				if (!ReadValue(depth + 1u, value))
					return false;
				if (!values.emplace(move(key), move(value)).second)
					return SetError("Duplicate object key");

				SkipWhitespace();
				if (Consume('}'))
					break;
				if (!Consume(','))
					return SetError("Expected ',' or '}' in object");
				SkipWhitespace();
				if (Peek('}'))
					return SetError("Trailing comma in object");
			}

			outValue = DATA_JSON_VALUE::Object(move(values));
			return true;
		}

		bool_t ReadString(string& outValue)
		{
			if (!Consume('"'))
				return SetError("Expected string");

			outValue.clear();
			while (m_Position < m_Text.size())
			{
				const unsigned char value = static_cast<unsigned char>(
					m_Text[m_Position++]);
				if ('"' == value)
					return true;
				if ('\\' == value)
				{
					if (!ReadEscape(outValue))
						return false;
					continue;
				}
				if (value < 0x20u)
					return SetError("Control character in string");
				outValue.push_back(static_cast<char>(value));
			}
			return SetError("Unterminated string");
		}

		bool_t ReadEscape(string& outValue)
		{
			if (m_Position >= m_Text.size())
				return SetError("Unterminated escape sequence");

			const char escape = m_Text[m_Position++];
			switch (escape)
			{
			case '"': outValue.push_back('"'); return true;
			case '\\': outValue.push_back('\\'); return true;
			case '/': outValue.push_back('/'); return true;
			case 'b': outValue.push_back('\b'); return true;
			case 'f': outValue.push_back('\f'); return true;
			case 'n': outValue.push_back('\n'); return true;
			case 'r': outValue.push_back('\r'); return true;
			case 't': outValue.push_back('\t'); return true;
			case 'u':
				break;
			default:
				return SetError("Invalid escape sequence");
			}

			uint32_t codePoint = {};
			if (!ReadHexQuad(codePoint))
				return false;
			if (codePoint >= 0xd800u && codePoint <= 0xdbffu)
			{
				if (m_Position + 2u > m_Text.size() ||
					'\\' != m_Text[m_Position] ||
					'u' != m_Text[m_Position + 1u])
				{
					return SetError("Missing low surrogate");
				}
				m_Position += 2u;
				uint32_t low = {};
				if (!ReadHexQuad(low) || low < 0xdc00u || low > 0xdfffu)
					return SetError("Invalid low surrogate");
				codePoint = 0x10000u +
					((codePoint - 0xd800u) << 10u) +
					(low - 0xdc00u);
			}
			else if (codePoint >= 0xdc00u && codePoint <= 0xdfffu)
			{
				return SetError("Unexpected low surrogate");
			}

			AppendUtf8(outValue, codePoint);
			return true;
		}

		bool_t ReadHexQuad(uint32_t& outValue)
		{
			if (m_Position + 4u > m_Text.size())
				return SetError("Incomplete unicode escape");

			outValue = {};
			for (size_t index = 0; index < 4u; ++index)
			{
				const char value = m_Text[m_Position++];
				uint32_t digit = {};
				if (value >= '0' && value <= '9')
					digit = static_cast<uint32_t>(value - '0');
				else if (value >= 'a' && value <= 'f')
					digit = 10u + static_cast<uint32_t>(value - 'a');
				else if (value >= 'A' && value <= 'F')
					digit = 10u + static_cast<uint32_t>(value - 'A');
				else
					return SetError("Invalid unicode escape");
				outValue = (outValue << 4u) | digit;
			}
			return true;
		}

		bool_t ReadNumber(DATA_JSON_VALUE& outValue)
		{
			const size_t start = m_Position;
			Consume('-');
			if (Consume('0'))
			{
				if (m_Position < m_Text.size() &&
					m_Text[m_Position] >= '0' && m_Text[m_Position] <= '9')
				{
					return SetError("Leading zero in number");
				}
			}
			else
			{
				if (!ReadDigits())
					return SetError("Invalid number");
			}

			if (Consume('.'))
			{
				if (!ReadDigits())
					return SetError("Missing fraction digits");
			}
			if (Consume('e') || Consume('E'))
			{
				Consume('+') || Consume('-');
				if (!ReadDigits())
					return SetError("Missing exponent digits");
			}

			double number = {};
			const char* begin = m_Text.data() + start;
			const char* end = m_Text.data() + m_Position;
			const auto result = from_chars(begin, end, number);
			if (result.ec != errc{} || result.ptr != end ||
				!isfinite(number))
			{
				return SetError("Invalid or non-finite number");
			}
			outValue = DATA_JSON_VALUE::Number(number);
			return true;
		}

		bool_t ReadDigits()
		{
			const size_t start = m_Position;
			while (m_Position < m_Text.size() &&
				m_Text[m_Position] >= '0' && m_Text[m_Position] <= '9')
			{
				++m_Position;
			}
			return m_Position != start;
		}

		bool_t ReadLiteral(const string_view literal)
		{
			if (m_Text.substr(m_Position, literal.size()) != literal)
				return SetError("Invalid literal");
			m_Position += literal.size();
			return true;
		}

		void SkipWhitespace()
		{
			while (m_Position < m_Text.size())
			{
				const char value = m_Text[m_Position];
				if (' ' != value && '\t' != value &&
					'\r' != value && '\n' != value)
				{
					break;
				}
				++m_Position;
			}
		}

		bool_t Peek(const char value) const
		{
			return m_Position < m_Text.size() &&
				m_Text[m_Position] == value;
		}

		bool_t Consume(const char value)
		{
			if (!Peek(value))
				return false;
			++m_Position;
			return true;
		}

		bool_t SetError(const char* pMessage)
		{
			if (m_Error.empty())
				m_Error = pMessage;
			return false;
		}

		string FormatError() const
		{
			return m_Error + " at byte " + to_string(m_Position);
		}

	private:
		string_view m_Text;
		DATA_JSON_PARSE_LIMITS m_Limits;
		size_t m_Position = {};
		size_t m_ValueCount = {};
		string m_Error;
	};
}

DATA_JSON_VALUE DATA_JSON_VALUE::Null()
{
	return {};
}

DATA_JSON_VALUE DATA_JSON_VALUE::Boolean(const bool_t value)
{
	DATA_JSON_VALUE result;
	result.m_eType = DATA_JSON_TYPE::BOOLEAN;
	result.m_Boolean = value;
	return result;
}

DATA_JSON_VALUE DATA_JSON_VALUE::Number(const double value)
{
	DATA_JSON_VALUE result;
	result.m_eType = DATA_JSON_TYPE::NUMBER;
	result.m_Number = value;
	return result;
}

DATA_JSON_VALUE DATA_JSON_VALUE::String(string value)
{
	DATA_JSON_VALUE result;
	result.m_eType = DATA_JSON_TYPE::STRING;
	result.m_String = move(value);
	return result;
}

DATA_JSON_VALUE DATA_JSON_VALUE::Array(ARRAY value)
{
	DATA_JSON_VALUE result;
	result.m_eType = DATA_JSON_TYPE::ARRAY;
	result.m_Array = move(value);
	return result;
}

DATA_JSON_VALUE DATA_JSON_VALUE::Object(OBJECT value)
{
	DATA_JSON_VALUE result;
	result.m_eType = DATA_JSON_TYPE::OBJECT;
	result.m_Object = move(value);
	return result;
}

const DATA_JSON_VALUE* DATA_JSON_VALUE::Find(const string_view key) const
{
	if (!Is_Object())
		return nullptr;
	const auto iterator = m_Object.find(key);
	return iterator == m_Object.end() ? nullptr : &iterator->second;
}

bool_t CDataJson::Parse(
	const string_view text,
	DATA_JSON_VALUE& outValue,
	string& outError)
{
	return Parse(text, outValue, outError, DATA_JSON_PARSE_LIMITS{});
}

bool_t CDataJson::Parse(
	const string_view text,
	DATA_JSON_VALUE& outValue,
	string& outError,
	const DATA_JSON_PARSE_LIMITS& limits)
{
	if (text.empty())
	{
		outError = "JSON document is empty";
		return false;
	}
	if (0u == limits.iMaximumBytes || 0u == limits.iMaximumDepth ||
		0u == limits.iMaximumValues)
	{
		outError = "JSON parse limits must be positive";
		return false;
	}
	if (text.size() > limits.iMaximumBytes)
	{
		outError = "JSON document exceeds its byte limit";
		return false;
	}

	DATA_JSON_VALUE staged;
	if (!JSON_READER(text, limits).Read(staged, outError))
		return false;
	outValue = move(staged);
	return true;
}

string CDataJson::Escape(const string_view value)
{
	string output;
	output.reserve(value.size() + 2u);
	for (const unsigned char character : value)
	{
		switch (character)
		{
		case '"': output += "\\\""; break;
		case '\\': output += "\\\\"; break;
		case '\b': output += "\\b"; break;
		case '\f': output += "\\f"; break;
		case '\n': output += "\\n"; break;
		case '\r': output += "\\r"; break;
		case '\t': output += "\\t"; break;
		default:
			if (character < 0x20u)
			{
				char buffer[7]{};
				sprintf_s(buffer, "\\u%04x", character);
				output += buffer;
			}
			else
			{
				output.push_back(static_cast<char>(character));
			}
			break;
		}
	}
	return output;
}
