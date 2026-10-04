# Debug 이펙트 로딩의 JSON 객체 구성·이동 비용 최적화 계획

## G00. 최종 반영 경계

최종 구현은 `DATA_JSON_VALUE`가 OBJECT payload를 단독 owner로 소유하고, parser가 그 최종 owner 안에서 map과 입력 순서를 직접 구성하는 방식이다. 09-21에 반영한 활성 payload variant를 유지하면서 JSON 객체의 후속 이동과 parser의 임시 map 이동에서 발생하던 sentinel/proxy 재생성을 줄인다. 실험 후보의 비교와 실행 결과는 같은 날짜의 대응 RESULT에 기록하며, 이 문서는 최종 제품 코드의 정본만 보존한다.

`DataJson.cpp`와 effect codec 13개 TU에 이미 적용된 Debug `/O2`를 유지한다. `/MDd`, `_DEBUG`, `_ITERATOR_DEBUG_LEVEL=2`, parser·codec 검증과 실패 보존을 끄지 않는다. 외부 factory의 map 인자 구성과 STRING/ARRAY의 Debug STL 비용까지 사라진다고 설명하지 않는다. 실제 성능 주장은 고정된 입력과 측정 범위, 별도 RESULT의 실행 증거를 따른다.

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | C:/Users/tnest/Desktop/LostArk/Client/Public/DataJson.h | 객체 payload 단독 소유와 공개 값의 copy/move 계약 |
| 수정 | C:/Users/tnest/Desktop/LostArk/Client/Private/DataJson.cpp | private parser의 직접 객체 구성, deep copy, 실패 시 commit 경계 |
| 추가 | C:/Users/tnest/Desktop/LostArk/Tools/DataJsonLoadingBenchmark/DataJsonLoadingBenchmark.cpp | 실제 parser의 고정 입력·단계별 CPU 측정용 독립 console main |
| 추가 | C:/Users/tnest/Desktop/LostArk/Tools/DataJsonLoadingBenchmark/ValueContracts.cpp | 값 소유권·이동 상태·복사 실패 보존의 독립 console main |
| 추가 | C:/Users/tnest/Desktop/LostArk/Tools/DataJsonLoadingBenchmark/Build-Benchmark.ps1 | 선택한 실제 h/cpp의 snapshot과 공통 도구체인 빌드 |
| 추가 | C:/Users/tnest/Desktop/LostArk/Tools/DataJsonLoadingBenchmark/Prepare-Corpus.ps1 | 현재 제품 V1 효과 목록의 입력 snapshot과 무결성 기록 |
| 추가 | C:/Users/tnest/Desktop/LostArk/Tools/DataJsonLoadingBenchmark/Run-Benchmark.ps1 | 준비한 native benchmark 실행과 측정 metadata 보존 |
| 추가 | C:/Users/tnest/Desktop/LostArk/Tools/DataJsonLoadingBenchmark/Run-ValueContracts.ps1 | 현재 제품 또는 지정된 실제 h/cpp의 별도 계약 실행 |

## G01. DataJson.h의 객체 소유권과 값 계약

변경 종류는 기존 헤더 전체 교체다. 파일은 원본의 UTF-8 BOM 없음과 CRLF를 유지한다. `<memory>`는 payload의 단독 owner를 제공하고, `<type_traits>`는 실제 variant의 이동이 nothrow라는 사실을 컴파일로 확인한다. 기존 `ARRAY`, `OBJECT`, 모든 factory와 const getter의 공개 서명을 유지하며 mutable getter는 추가하지 않는다.

`OBJECT_PAYLOAD::values`는 key와 자식 값의 소유 map이고 `insertionOrder`는 원본 key 입력 순서를 보존한다. `OBJECT_STORAGE`는 두 컨테이너를 가진 payload의 `unique_ptr` owner다. scalar는 컨테이너나 payload를 할당하지 않는다. payload 공유나 copy-on-write는 도입하지 않는다.

명시적 복사 생성자는 자식 값까지 deep copy한다. 복사 대입은 임시 복사가 완성된 뒤에만 noexcept move로 교체한다. 이동 생성·대입은 variant 대안 전체의 nothrow trait가 참이어야 컴파일되며 map 이동에 noexcept를 강제로 붙이지 않는다. 이동된 원본은 기존처럼 타입을 유지하고 OBJECT owner가 null일 수 있다. 이 상태의 객체 getter와 Find는 빈 객체처럼 읽히며 다시 복사·이동·대입·파괴할 수 있다.

`friend class DATA_JSON_READER`는 CPP 안의 단일 reader만 private staging payload를 구성하게 한다. 외부 소비자는 기존 const API로만 접근한다. `DATA_JSON_VALUE` 내부 배치가 바뀌므로 제품 반영 시 헤더 의존 Client TU 전체를 같은 헤더로 다시 컴파일한다. `Tools/Build/CppStandardPch.h`는 표준 라이브러리만 포함하므로 DataJson 자체가 PCH에 포함된 것으로 가정하지 않는다.

### C:/Users/tnest/Desktop/LostArk/Client/Public/DataJson.h 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

NS_BEGIN(Client)

enum class DATA_JSON_TYPE
{
	NULL_VALUE,
	BOOLEAN,
	NUMBER,
	STRING,
	ARRAY,
	OBJECT
};

class DATA_JSON_VALUE final
{
public:
	using ARRAY = vector<DATA_JSON_VALUE>;
	using OBJECT = map<string, DATA_JSON_VALUE, less<>>;

public:
	DATA_JSON_VALUE() = default;
	DATA_JSON_VALUE(const DATA_JSON_VALUE& other);
	DATA_JSON_VALUE& operator=(const DATA_JSON_VALUE& other);
	DATA_JSON_VALUE(DATA_JSON_VALUE&& other) noexcept = default;
	DATA_JSON_VALUE& operator=(DATA_JSON_VALUE&& other) noexcept = default;
	~DATA_JSON_VALUE() = default;

	static DATA_JSON_VALUE Null();
	static DATA_JSON_VALUE Boolean(bool_t value);
	static DATA_JSON_VALUE Number(
		double value, bool_t bFloatingPointToken = false);
	static DATA_JSON_VALUE String(string value);
	static DATA_JSON_VALUE Array(ARRAY value);
	static DATA_JSON_VALUE Object(
		OBJECT value, vector<string> insertionOrder = {});

	DATA_JSON_TYPE Get_Type() const { return m_eType; }
	bool_t Is_Null() const {
		return DATA_JSON_TYPE::NULL_VALUE == m_eType;
	}
	bool_t Is_Boolean() const {
		return DATA_JSON_TYPE::BOOLEAN == m_eType;
	}
	bool_t Is_Number() const {
		return DATA_JSON_TYPE::NUMBER == m_eType;
	}
	bool_t Is_String() const {
		return DATA_JSON_TYPE::STRING == m_eType;
	}
	bool_t Is_Array() const {
		return DATA_JSON_TYPE::ARRAY == m_eType;
	}
	bool_t Is_Object() const {
		return DATA_JSON_TYPE::OBJECT == m_eType;
	}

	bool_t Get_Boolean() const { return m_Boolean; }
	double Get_Number() const { return m_Number; }
	bool_t Was_FloatingPointToken() const {
		return m_bFloatingPointToken;
	}
	const string& Get_String() const;
	const ARRAY& Get_Array() const;
	const OBJECT& Get_Object() const;
	const vector<string>& Get_ObjectInsertionOrder() const;
	const DATA_JSON_VALUE* Find(string_view key) const;

private:
	friend class DATA_JSON_READER;

	struct OBJECT_PAYLOAD final
	{
		OBJECT values;
		vector<string> insertionOrder;
	};
	using OBJECT_STORAGE = std::unique_ptr<OBJECT_PAYLOAD>;

	DATA_JSON_TYPE m_eType = DATA_JSON_TYPE::NULL_VALUE;
	bool_t m_Boolean = false;
	double m_Number = {};
	bool_t m_bFloatingPointToken = false;
	// Scalars must not construct empty Debug STL containers and their proxies.
	// Objects transfer one owner instead of moving their map sentinel and proxy.
	// A moved-from object may have no payload and is read as an empty object.
	std::variant<std::monostate, string, ARRAY, OBJECT_STORAGE> m_Payload;
	static_assert(std::is_nothrow_move_constructible_v<decltype(m_Payload)>);
	static_assert(std::is_nothrow_move_assignable_v<decltype(m_Payload)>);
};

struct DATA_JSON_PARSE_LIMITS final
{
	size_t iMaximumBytes = 16u * 1024u * 1024u;
	size_t iMaximumDepth = 64u;
	size_t iMaximumValues = 1'000'000u;
};

class CDataJson final
{
public:
	static bool_t Parse(
		string_view text,
		DATA_JSON_VALUE& outValue,
		string& outError);
	static bool_t Parse(
		string_view text,
		DATA_JSON_VALUE& outValue,
		string& outError,
		const DATA_JSON_PARSE_LIMITS& limits);
	static string Escape(string_view value);
};

NS_END
```

## G02. DataJson.cpp의 private 구성·복사·commit 흐름

변경 종류는 기존 CPP 전체 교체다. `AppendUtf8`는 익명 namespace의 보조 함수로 유지하고, `DATA_JSON_READER`는 Client namespace에 정의한다. 별도 parser나 공개 mutation 경로를 만들지 않는다.

| 함수 | 한 줄 책임과 내부 흐름 |
|---|---|
| `DATA_JSON_READER::ReadObject` | private staging 노드를 null로 초기화하고 최종 OBJECT owner 안에 map/order를 생성한 뒤 각 key와 자식을 직접 채운다. |
| `DATA_JSON_VALUE(const DATA_JSON_VALUE&)` | scalar 상태와 활성 STRING/ARRAY를 복사하며 OBJECT는 새 payload와 모든 자식 값을 재귀 복사한다. |
| `operator=(const DATA_JSON_VALUE&)` | 자기 대입은 건너뛰고 전체 임시 복사가 성공한 경우만 교체하여 할당·복사 예외에서 기존 값을 보존한다. |
| `Object` | 외부 호출자의 기존 factory 계약과 입력 순서 크기 보정을 유지하고 단독 owner를 만든다. parser는 이미 직접 구성한 객체를 이 factory로 다시 옮기지 않는다. |
| `Get_Object`, `Get_ObjectInsertionOrder` | 활성 owner의 const 참조를 반환하고 잘못된 타입이나 이동된 빈 owner에서는 기존 방식의 정적 빈 컨테이너를 반환한다. |
| `CDataJson::Parse` | 문서 전체와 trailing data 검증을 끝낸 private root만 호출자의 출력에 최종 commit한다. |

실제 effect codec은 기존 `CDataJson::Parse`를 호출한다. reader가 root와 자식의 private 노드를 구성하고 `ReadObject`는 payload 생성 성공 후 OBJECT 타입을 설정한다. 실패한 불완전 객체는 임시 트리와 함께 파괴되며 외부로 반환하지 않는다. 성공한 객체의 map/order를 factory를 거쳐 다시 이동하지 않고, 부모나 최종 출력으로의 이동은 owner를 넘긴다.

문법, maximum bytes/depth/value 수, number token, Unicode, object 입력 순서, error text와 byte 위치를 유지한다. duplicate key는 기존처럼 그 value를 먼저 파싱하고 입력 순서 추가 뒤 map 삽입에서 거부한다. 성공한 parser 객체의 map/order 크기는 항상 같으므로 factory의 크기 보정은 필요하지 않다. 외부 factory 보정은 유지한다. `ReadArray`의 private staging 확장 방식과 최종 `Parse` 실패 보존 경계도 유지한다.

### C:/Users/tnest/Desktop/LostArk/Client/Private/DataJson.cpp 전체 코드

```cpp
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

}

NS_BEGIN(Client)

	class DATA_JSON_READER final
	{
	public:
		explicit DATA_JSON_READER(const string_view text,
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
				// Keep bounded growth in the private staging array; the caller's
				// outValue is still committed only after complete parsing.
				if (values.size() == values.capacity())
				{
					DATA_JSON_VALUE::ARRAY expanded;
					expanded.reserve(values.empty() ? 4u : values.size() * 2u);
					for (DATA_JSON_VALUE& existing : values)
						expanded.push_back(move(existing));
					values.swap(expanded);
				}
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
			// Reader outputs are private staging nodes until Parse commits the root.
			// Construct the map in its final owner instead of moving a staging map.
			outValue = DATA_JSON_VALUE::Null();
			outValue.m_Payload.emplace<DATA_JSON_VALUE::OBJECT_STORAGE>(
				std::make_unique<DATA_JSON_VALUE::OBJECT_PAYLOAD>());
			outValue.m_eType = DATA_JSON_TYPE::OBJECT;
			auto& payload = *std::get<DATA_JSON_VALUE::OBJECT_STORAGE>(
				outValue.m_Payload);
			auto& values = payload.values;
			auto& insertionOrder = payload.insertionOrder;
			SkipWhitespace();
			if (Consume('}'))
			{
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
				insertionOrder.push_back(key);
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
			const string_view token = m_Text.substr(start, m_Position - start);
			const bool_t bFloatingPointToken =
				token.find_first_of(".eE") != string_view::npos;
			outValue = DATA_JSON_VALUE::Number(number, bFloatingPointToken);
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

NS_END

DATA_JSON_VALUE::DATA_JSON_VALUE(const DATA_JSON_VALUE& other)
	: m_eType(other.m_eType)
	, m_Boolean(other.m_Boolean)
	, m_Number(other.m_Number)
	, m_bFloatingPointToken(other.m_bFloatingPointToken)
{
	switch (m_eType)
	{
	case DATA_JSON_TYPE::STRING:
		m_Payload.emplace<string>(other.Get_String());
		break;
	case DATA_JSON_TYPE::ARRAY:
		m_Payload.emplace<ARRAY>(other.Get_Array());
		break;
	case DATA_JSON_TYPE::OBJECT:
	{
		const OBJECT_STORAGE& source = std::get<OBJECT_STORAGE>(other.m_Payload);
		if (source)
		{
			m_Payload.emplace<OBJECT_STORAGE>(
				std::make_unique<OBJECT_PAYLOAD>(*source));
		}
		else
		{
			m_Payload.emplace<OBJECT_STORAGE>();
		}
		break;
	}
	default:
		break;
	}
}

DATA_JSON_VALUE& DATA_JSON_VALUE::operator=(const DATA_JSON_VALUE& other)
{
	if (this != &other)
	{
		DATA_JSON_VALUE staged(other);
		*this = move(staged);
	}
	return *this;
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

DATA_JSON_VALUE DATA_JSON_VALUE::Number(
	const double value, const bool_t bFloatingPointToken)
{
	DATA_JSON_VALUE result;
	result.m_eType = DATA_JSON_TYPE::NUMBER;
	result.m_Number = value;
	result.m_bFloatingPointToken = bFloatingPointToken;
	return result;
}

DATA_JSON_VALUE DATA_JSON_VALUE::String(string value)
{
	DATA_JSON_VALUE result;
	result.m_eType = DATA_JSON_TYPE::STRING;
	result.m_Payload.emplace<string>(move(value));
	return result;
}

DATA_JSON_VALUE DATA_JSON_VALUE::Array(ARRAY value)
{
	DATA_JSON_VALUE result;
	result.m_eType = DATA_JSON_TYPE::ARRAY;
	result.m_Payload.emplace<ARRAY>(move(value));
	return result;
}

DATA_JSON_VALUE DATA_JSON_VALUE::Object(
	OBJECT value, vector<string> insertionOrder)
{
	DATA_JSON_VALUE result;
	result.m_eType = DATA_JSON_TYPE::OBJECT;
	if (insertionOrder.size() != value.size())
	{
		insertionOrder.clear();
		insertionOrder.reserve(value.size());
		for (const auto& [key, child] : value)
			insertionOrder.push_back(key);
	}
	result.m_Payload.emplace<OBJECT_STORAGE>(
		std::make_unique<OBJECT_PAYLOAD>(move(value), move(insertionOrder)));
	return result;
}

const string& DATA_JSON_VALUE::Get_String() const
{
	if (const auto* value = std::get_if<string>(&m_Payload))
		return *value;
	static const string empty;
	return empty;
}

const DATA_JSON_VALUE::ARRAY& DATA_JSON_VALUE::Get_Array() const
{
	if (const auto* value = std::get_if<ARRAY>(&m_Payload))
		return *value;
	static const ARRAY empty;
	return empty;
}

const DATA_JSON_VALUE::OBJECT& DATA_JSON_VALUE::Get_Object() const
{
	if (const auto* value = std::get_if<OBJECT_STORAGE>(&m_Payload);
		value && *value)
	{
		return (*value)->values;
	}
	static const OBJECT empty;
	return empty;
}

const vector<string>& DATA_JSON_VALUE::Get_ObjectInsertionOrder() const
{
	if (const auto* value = std::get_if<OBJECT_STORAGE>(&m_Payload);
		value && *value)
	{
		return (*value)->insertionOrder;
	}
	static const vector<string> empty;
	return empty;
}

const DATA_JSON_VALUE* DATA_JSON_VALUE::Find(const string_view key) const
{
	if (!Is_Object())
		return nullptr;
	const OBJECT& values = Get_Object();
	const auto iterator = values.find(key);
	return iterator == values.end() ? nullptr : &iterator->second;
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
	if (!DATA_JSON_READER(text, limits).Read(staged, outError))
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
```

## G03. DataJsonLoadingBenchmark.cpp의 실제 parser 측정

이 새 파일은 제품 Client와 별개의 console main이다. 실제 `DataJson.h/.cpp`, Client 정의와 EngineSDK 헤더를 사용하고 parser 내부에 계측을 삽입하지 않는다. 모든 JSON byte를 먼저 메모리에 읽은 다음 parse, semantic traversal, destruction을 별도 phase로 실행한다. worker별 DOM 하나를 유지하고 1명 또는 3명씩 wave로 처리한다. parse wall에는 phase 동기화와 root 생성이 포함되며 semantic 순회는 제외된다. worker 시간의 합을 wall time으로 바꾸지 않는다.

`Digest`는 값 종류, 숫자 bit/token, 문자열, array 순서, object key/value 및 입력 순서를 순회한다. FNV1a64는 의미 비교용 진단 값이며 입력 byte 무결성은 script의 SHA256으로 기록한다. 기본 `Contracts`는 malformed 입력과 제한·출력 보존·copy/move/order를 확인한다. Debug allocation pass는 별도 실행에서 parse 중 `_HOOK_ALLOC` 요청 수와 누적 요청 byte만 집계하고, 그 실행 시간은 속도 표에 사용하지 않는다.

이 도구의 경계는 in-memory parser다. 파일 읽기, effect codec decode, renderer/device-resource, GPU 실행, 전체 맵 진입 또는 첫 화면 시간을 포함하지 않는다. source 선택을 바꿀 수 있지만 각 executable은 하나의 일관된 header/cpp snapshot만 사용한다.

### C:/Users/tnest/Desktop/LostArk/Tools/DataJsonLoadingBenchmark/DataJsonLoadingBenchmark.cpp 전체 코드

```cpp
#include <algorithm>
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "DataJson.h"
#include <Psapi.h>

using Clock = std::chrono::steady_clock;
using Client::CDataJson;
using Client::DATA_JSON_VALUE;
using Client::DATA_JSON_TYPE;
using Client::DATA_JSON_PARSE_LIMITS;
static double Ms(Clock::time_point a, Clock::time_point b) { return std::chrono::duration<double, std::milli>(b-a).count(); }
static std::atomic<bool> trackAlloc{false};
static std::atomic<uint64_t> allocations{0}, allocationBytes{0};
#ifdef _DEBUG
static int AllocHook(int kind, void*, size_t size, int, long, const unsigned char*, int) {
    if(kind == _HOOK_ALLOC && trackAlloc.load(std::memory_order_relaxed)) {
        allocations.fetch_add(1, std::memory_order_relaxed);
        allocationBytes.fetch_add(size, std::memory_order_relaxed);
    }
    return TRUE;
}
#endif
struct Digest {
    uint64_t hash=14695981039346656037ull, nodes=0;
    void Byte(uint8_t b){hash=(hash^b)*1099511628211ull;}
    void U64(uint64_t n){for(int i=0;i<8;i++) Byte(static_cast<uint8_t>(n>>(i*8)));}
    void Text(std::string_view s){U64(s.size());for(unsigned char c:s)Byte(c);}
    void Value(const DATA_JSON_VALUE& v){
        ++nodes; Byte(static_cast<uint8_t>(v.Get_Type()));
        switch(v.Get_Type()){
        case DATA_JSON_TYPE::BOOLEAN: Byte(v.Get_Boolean()); break;
        case DATA_JSON_TYPE::NUMBER: U64(std::bit_cast<uint64_t>(v.Get_Number())); Byte(v.Was_FloatingPointToken()); break;
        case DATA_JSON_TYPE::STRING: Text(v.Get_String());break;
        case DATA_JSON_TYPE::ARRAY: U64(v.Get_Array().size());for(const auto& x:v.Get_Array())Value(x);break;
        case DATA_JSON_TYPE::OBJECT:
            U64(v.Get_Object().size());for(const auto& [k,x]:v.Get_Object()){Text(k);Value(x);}
            U64(v.Get_ObjectInsertionOrder().size());for(const auto& k:v.Get_ObjectInsertionOrder())Text(k);break;
        default:break;
        }
    }
};
static std::string Hex(uint64_t v){std::ostringstream s;s<<std::hex<<std::setw(16)<<std::setfill('0')<<v;return s.str();}
static std::string Quote(std::string_view v){return "\""+CDataJson::Escape(v)+"\"";}
struct Input {std::string id, path, text;};
struct Row {bool ok=false;std::string error;uint64_t hash=0,nodes=0;double parseMs=0,digestMs=0,destroyMs=0;};
static uint64_t FT(FILETIME v){return (uint64_t(v.dwHighDateTime)<<32)|v.dwLowDateTime;}
struct Usage {uint64_t idle=0,kernel=0,user=0,process=0;};
static Usage UsageNow(){FILETIME i{},k{},u{},c{},e{},pk{},pu{};GetSystemTimes(&i,&k,&u);GetProcessTimes(GetCurrentProcess(),&c,&e,&pk,&pu);return {FT(i),FT(k),FT(u),FT(pk)+FT(pu)};}
static void Require(bool yes, const char* message){if(!yes)throw std::runtime_error(message);}
static unsigned Contracts(){
    unsigned n=0;std::string e;DATA_JSON_VALUE v;
    const std::string good=R"({"z":[null,true,false,1,1.0,-0,2e1,"a\n\uD83D\uDE00"],"a":{"q":2}})";
    Require(CDataJson::Parse(good,v,e),"valid mixed input");++n;Digest d;d.Value(v);
    Require(v.Get_ObjectInsertionOrder()==std::vector<std::string>({"z","a"}),"insertion order");++n;
    Require(v.Get_Object().begin()->first=="a","sorted object keys");++n;
    Require(!v.Find("z")->Get_Array()[3].Was_FloatingPointToken() && v.Find("z")->Get_Array()[4].Was_FloatingPointToken(),"numeric token flags");++n;
    auto copy=v;Digest dc;dc.Value(copy);Require(dc.hash==d.hash,"copy value");++n;
    Require(&copy.Get_Object()!=&v.Get_Object() && &copy.Find("z")->Get_Array()!=&v.Find("z")->Get_Array(),"deep copy owns subtree");++n;
    auto moved=std::move(copy);Digest dm;dm.Value(moved);Require(dm.hash==d.hash,"move destination");++n;
    moved=moved;Digest ds;ds.Value(moved);Require(ds.hash==d.hash,"self-copy assignment");++n;
    for(const std::string bad:{"", "{", "[1,]", "{\"a\":1,\"a\":2}", "[01]", "[1e]", "[NaN]", "true false", "\"\\uD800\"", "\"\\uDC00\""}) {
        auto keep=v;Require(!CDataJson::Parse(bad,keep,e),"malformed input accepted");Digest k;k.Value(keep);Require(k.hash==d.hash,"failure changed output");++n;
    }
    DATA_JSON_PARSE_LIMITS l;l.iMaximumBytes=2;Require(CDataJson::Parse("[]",v,e,l),"byte boundary");++n;
    Require(!CDataJson::Parse("[0]",v,e,l),"byte limit");++n;
    l={};l.iMaximumValues=2;Require(CDataJson::Parse("[0]",v,e,l),"value boundary");++n;
    Require(!CDataJson::Parse("[0,1]",v,e,l),"value limit");++n;
    l={};l.iMaximumDepth=1;Require(CDataJson::Parse("[0]",v,e,l),"depth boundary");++n;
    Require(!CDataJson::Parse("[[0]]",v,e,l),"depth limit");++n;
    l={};l.iMaximumValues=0;Require(!CDataJson::Parse("null",v,e,l),"zero limits");++n;
    return n;
}
int main(int argc,char** argv){try{
    if(argc<4)throw std::runtime_error("usage: exe corpus.tsv workers output.json [alloc]");
    const size_t workers=std::stoull(argv[2]);Require(workers==1||workers==3,"workers must be 1 or 3");
    const bool allocationRun=argc>4 && std::string(argv[4])=="alloc";
    Require(!allocationRun || workers==1,"allocation pass must have one worker");
    const auto contracts=Contracts();
    std::vector<Input> inputs;uint64_t bytes=0;std::ifstream list(argv[1]);std::string line;
    while(std::getline(list,line)){if(!line.empty()&&line.back()=='\r')line.pop_back();if(line.empty())continue;const auto tab=line.find('\t');Require(tab!=std::string::npos,"invalid corpus row");Input in{line.substr(0,tab),line.substr(tab+1),{}};std::ifstream f(in.path,std::ios::binary);Require(bool(f),"missing corpus file");in.text.assign(std::istreambuf_iterator<char>(f),{});Require(!f.bad(),"read failed");bytes+=in.text.size();inputs.push_back(std::move(in));}
    Require(!inputs.empty(),"empty corpus");
    std::vector<Row> rows(inputs.size());std::vector<std::unique_ptr<DATA_JSON_VALUE>> roots(workers);
    std::barrier gate(static_cast<std::ptrdiff_t>(workers+1));std::atomic<int> phase{0};size_t wave=0;
    DATA_JSON_PARSE_LIMITS limits;limits.iMaximumBytes=64u*1024u*1024u;limits.iMaximumDepth=64;limits.iMaximumValues=3'000'000;
    std::vector<std::thread> threads;
    for(size_t slot=0;slot<workers;slot++)threads.emplace_back([&,slot]{for(;;){gate.arrive_and_wait();const int p=phase.load();if(p==4){gate.arrive_and_wait();break;}const size_t index=wave+slot;if(index<inputs.size()){
        Row& r=rows[index];auto a=Clock::now();
        if(p==1){roots[slot]=std::make_unique<DATA_JSON_VALUE>();r.ok=CDataJson::Parse(inputs[index].text,*roots[slot],r.error,limits);r.parseMs=Ms(a,Clock::now());}
        if(p==2){Digest d;if(r.ok)d.Value(*roots[slot]);r.hash=d.hash;r.nodes=d.nodes;r.digestMs=Ms(a,Clock::now());}
        if(p==3){roots[slot].reset();r.destroyMs=Ms(a,Clock::now());}
    }gate.arrive_and_wait();}});
#ifdef _DEBUG
    const auto previousHook=allocationRun?_CrtSetAllocHook(AllocHook):nullptr;
#endif
    const Usage u0=UsageNow();const auto totalStart=Clock::now();double parseWall=0,digestWall=0,destroyWall=0;
    for(wave=0;wave<inputs.size();wave+=workers)for(int p=1;p<=3;p++){
        phase.store(p);trackAlloc.store(allocationRun && p==1);const auto a=Clock::now();gate.arrive_and_wait();gate.arrive_and_wait();const auto elapsed=Ms(a,Clock::now());trackAlloc.store(false);if(p==1)parseWall+=elapsed;if(p==2)digestWall+=elapsed;if(p==3)destroyWall+=elapsed;
    }
    const auto totalEnd=Clock::now();const Usage u1=UsageNow();phase.store(4);gate.arrive_and_wait();gate.arrive_and_wait();for(auto& t:threads)t.join();
#ifdef _DEBUG
    if(allocationRun)_CrtSetAllocHook(previousHook);
#endif
    PROCESS_MEMORY_COUNTERS_EX memory{};GetProcessMemoryInfo(GetCurrentProcess(),reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memory),sizeof(memory));
    uint64_t nodes=0,failed=0;Digest aggregate;double parseSum=0,destroySum=0;for(size_t i=0;i<rows.size();i++){nodes+=rows[i].nodes;failed+=!rows[i].ok;aggregate.Text(inputs[i].id);aggregate.U64(rows[i].hash);parseSum+=rows[i].parseMs;destroySum+=rows[i].destroyMs;}
    const double busy=double((u1.kernel-u0.kernel)+(u1.user-u0.user)-(u1.idle-u0.idle));
    const double all=double((u1.kernel-u0.kernel)+(u1.user-u0.user));
    std::ofstream out(argv[3]);out<<std::setprecision(12)<<"{\n\"workers\":"<<workers<<",\n\"allocationInstrumentation\":"<<(allocationRun?"true":"false")<<",\n\"debug\":"
#ifdef _DEBUG
    <<"true"
#else
    <<"false"
#endif
    <<",\n\"iteratorDebugLevel\":"<<_ITERATOR_DEBUG_LEVEL<<",\n\"sizeofValue\":"<<sizeof(DATA_JSON_VALUE)<<",\n\"contractCases\":"<<contracts<<",\n\"fileCount\":"<<inputs.size()<<",\n\"inputBytes\":"<<bytes<<",\n\"nodes\":"<<nodes<<",\n\"failed\":"<<failed<<",\n\"semanticDigestFnv1a64\":"<<Quote(Hex(aggregate.hash))<<",\n\"parseWallMs\":"<<parseWall<<",\n\"digestWallMs\":"<<digestWall<<",\n\"destructionWallMs\":"<<destroyWall<<",\n\"totalWallMs\":"<<Ms(totalStart,totalEnd)<<",\n\"parseSummedWorkerMs\":"<<parseSum<<",\n\"destructionSummedWorkerMs\":"<<destroySum<<",\n\"processCpuMs\":"<<double(u1.process-u0.process)/10000.<<",\n\"systemCpuBusyPercent\":"<<(all?busy/all*100.:0.)<<",\n\"peakWorkingSetBytes\":"<<memory.PeakWorkingSetSize<<",\n\"privateBytesAtEnd\":"<<memory.PrivateUsage<<",\n\"parseAllocations\":"<<allocations.load()<<",\n\"parseAllocatedBytes\":"<<allocationBytes.load()<<",\n\"boundary\":\"Preloaded memory; actual unmodified CDataJson parser and Engine headers; no codec, GPU, Client or UI. Phases measured in waves; worker sums are not wall time. FNV semantic traversal is outside parse timing. Allocation passes are excluded from speed comparisons.\",\n\"files\":[\n";
    for(size_t i=0;i<rows.size();i++){const auto&r=rows[i];if(i)out<<",\n";out<<"{\"id\":"<<Quote(inputs[i].id)<<",\"bytes\":"<<inputs[i].text.size()<<",\"ok\":"<<(r.ok?"true":"false")<<",\"error\":"<<Quote(r.error)<<",\"nodes\":"<<r.nodes<<",\"digest\":"<<Quote(Hex(r.hash))<<",\"parseMs\":"<<r.parseMs<<",\"digestMs\":"<<r.digestMs<<",\"destroyMs\":"<<r.destroyMs<<"}";}
    out<<"\n]}\n";out.close();std::cout<<"files="<<inputs.size()<<" failed="<<failed<<" parse_ms="<<parseWall<<" destroy_ms="<<destroyWall<<" total_ms="<<Ms(totalStart,totalEnd)<<" digest="<<Hex(aggregate.hash)<<"\n";return failed?2:0;
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}
```

## G04. ValueContracts.cpp의 소유권과 실패 원자성 검사

이 새 파일도 제품과 별개의 console main이며 실제 검증한 계약 소스를 byte 그대로 보존한다. 모든 타입 간 copy/move 대입, factory와 잘못된 타입 getter, 입력 순서, 중첩 deep copy 독립성·수명, moved-from 객체의 읽기와 재사용을 검사한다. self-move 뒤에는 유효하게 읽고 재대입할 수 있어야 하며 원래 내용 보존까지 요구하지 않는다.

`--require-strong-copy`는 자기 소유 자식에서의 복사 대입을 검사하고, Debug에서는 복사 과정의 각 할당 지점에 한 번씩 실패를 주입해 `bad_alloc`과 이전 목적지 보존을 확인한다. hook은 독립 process에서만 설치하고 scope 종료 시 이전 hook으로 복원한다. 이 강화된 실패 보장을 제공하지 않는 과거 baseline에 자동 적용하지 않는다. Release에서는 Debug CRT 실패주입을 실행하지 않는다.

### C:/Users/tnest/Desktop/LostArk/Tools/DataJsonLoadingBenchmark/ValueContracts.cpp 전체 코드

```cpp
#include "DataJson.h"

#include <bit>
#include <cstdint>
#include <iostream>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

using Client::CDataJson;
using Client::DATA_JSON_TYPE;
using Client::DATA_JSON_VALUE;

namespace
{
	unsigned g_Checks = 0;

	void Require(const bool result, const char* message)
	{
		++g_Checks;
		if (!result)
			throw std::runtime_error(message);
	}

	bool Equal(const DATA_JSON_VALUE& left, const DATA_JSON_VALUE& right)
	{
		if (left.Get_Type() != right.Get_Type())
			return false;
		switch (left.Get_Type())
		{
		case DATA_JSON_TYPE::BOOLEAN:
			return left.Get_Boolean() == right.Get_Boolean();
		case DATA_JSON_TYPE::NUMBER:
			return std::bit_cast<uint64_t>(left.Get_Number()) ==
				std::bit_cast<uint64_t>(right.Get_Number()) &&
				left.Was_FloatingPointToken() == right.Was_FloatingPointToken();
		case DATA_JSON_TYPE::STRING:
			return left.Get_String() == right.Get_String();
		case DATA_JSON_TYPE::ARRAY:
		{
			const auto& a = left.Get_Array();
			const auto& b = right.Get_Array();
			if (a.size() != b.size())
				return false;
			for (size_t index = 0; index < a.size(); ++index)
				if (!Equal(a[index], b[index]))
					return false;
			return true;
		}
		case DATA_JSON_TYPE::OBJECT:
		{
			const auto& a = left.Get_Object();
			const auto& b = right.Get_Object();
			if (a.size() != b.size() || left.Get_ObjectInsertionOrder() !=
				right.Get_ObjectInsertionOrder())
				return false;
			auto other = b.begin();
			for (const auto& [key, value] : a)
			{
				if (key != other->first || !Equal(value, other->second))
					return false;
				++other;
			}
			return true;
		}
		default:
			return true;
		}
	}

	DATA_JSON_VALUE Parse(const char* text)
	{
		DATA_JSON_VALUE value;
		std::string error;
		Require(CDataJson::Parse(text, value, error), "fixture parse");
		return value;
	}

	void CheckValueContracts()
	{
		DATA_JSON_VALUE::OBJECT fields;
		fields.emplace("z", DATA_JSON_VALUE::Boolean(true));
		fields.emplace("a", DATA_JSON_VALUE::Number(-0.0, true));
		auto sorted = DATA_JSON_VALUE::Object(fields);
		Require(sorted.Get_ObjectInsertionOrder() ==
			std::vector<std::string>({ "a", "z" }), "factory fallback order");
		auto ordered = DATA_JSON_VALUE::Object(fields, { "z", "a" });
		Require(ordered.Get_ObjectInsertionOrder() ==
			std::vector<std::string>({ "z", "a" }), "factory explicit order");
		auto repaired = DATA_JSON_VALUE::Object(fields, { "z" });
		Require(Equal(repaired, sorted), "factory count mismatch order repair");
		Require(ordered.Find("missing") == nullptr, "missing object member");
		Require(ordered.Find("z")->Get_Boolean(), "factory object child");

		DATA_JSON_VALUE::ARRAY elements;
		elements.push_back(DATA_JSON_VALUE::Number(1));
		elements.push_back(DATA_JSON_VALUE::String(std::string("x\0y", 3)));
		const DATA_JSON_VALUE values[] = {
			DATA_JSON_VALUE::Null(), DATA_JSON_VALUE::Boolean(true),
			DATA_JSON_VALUE::Number(-0.0, true),
			DATA_JSON_VALUE::String(std::string("x\0y", 3)),
			DATA_JSON_VALUE::Array(std::move(elements)), ordered
		};
		for (const auto& value : values)
		{
			if (!value.Is_String())
				Require(value.Get_String().empty(), "wrong string getter");
			if (!value.Is_Array())
				Require(value.Get_Array().empty(), "wrong array getter");
			if (!value.Is_Object())
			{
				Require(value.Get_Object().empty(), "wrong object getter");
				Require(value.Get_ObjectInsertionOrder().empty(), "wrong order getter");
				Require(value.Find("z") == nullptr, "wrong type Find");
			}

			for (const auto& previous : values)
			{
				auto assigned = previous;
				assigned = value;
				Require(Equal(assigned, value), "copy assignment type transition");
				assigned = assigned;
				Require(Equal(assigned, value), "self copy");
				auto input = value;
				assigned = std::move(input);
				Require(Equal(assigned, value), "move assignment type transition");
				input = DATA_JSON_VALUE::Boolean(false);
				Require(input.Is_Boolean() && !input.Get_Boolean(), "moved source reuse");
			}
			auto selfMoved = value;
			selfMoved = std::move(selfMoved);
			// Self-move need not preserve content; the value must remain reusable.
			(void)selfMoved.Get_String();
			(void)selfMoved.Get_Array();
			(void)selfMoved.Get_Object();
			(void)selfMoved.Get_ObjectInsertionOrder();
			(void)selfMoved.Find("missing");
			selfMoved = value;
			Require(Equal(selfMoved, value), "self moved value reuse");
		}

		auto source = Parse(R"({"b":{"inner":[1,2,{"text":"long-owned-string-0123456789"}]},"a":true})");
		auto snapshot = source;
		auto destination = std::move(source);
		Require(Equal(destination, snapshot), "object move destination");
		Require(source.Is_Object(), "moved object type retained");
		Require(source.Get_Object().empty(), "moved object map empty");
		Require(source.Get_ObjectInsertionOrder().empty(), "moved object order empty");
		Require(source.Find("b") == nullptr, "moved object Find");
		auto emptyCopy = source;
		Require(emptyCopy.Is_Object() && emptyCopy.Get_Object().empty(), "moved object copy");
		auto assignedEmpty = DATA_JSON_VALUE::String("replace");
		assignedEmpty = source;
		Require(assignedEmpty.Is_Object() && assignedEmpty.Get_Object().empty(), "moved object assignment");
		source = snapshot;
		Require(Equal(source, snapshot), "moved object deep copy reuse");
		Require(source.Find("b") != snapshot.Find("b"), "deep copy node independence");
		Require(&source.Find("b")->Get_Object() != &snapshot.Find("b")->Get_Object(),
			"deep copy nested object independence");
		snapshot = DATA_JSON_VALUE::Null();
		Require(source.Find("b")->Find("inner")->Get_Array().size() == 3,
			"copy survives original replacement");
	}

	void CheckStrongCopyAlias()
	{
		auto value = Parse(R"({"nested":{"x":[1,{"y":"owned-text"}]},"other":false})");
		auto expected = *value.Find("nested");
		value = *value.Find("nested");
		Require(Equal(value, expected), "copy assignment from owned descendant");
	}

#ifdef _DEBUG
	long g_AllocationsUntilFailure = -1;
	bool g_DeniedAllocation = false;

	int AllocationFailureHook(int kind, void*, size_t, int blockType,
		long, const unsigned char*, int)
	{
		if (kind == _HOOK_ALLOC && blockType != _CRT_BLOCK &&
			g_AllocationsUntilFailure >= 0)
		{
			if (g_AllocationsUntilFailure-- == 0)
			{
				g_DeniedAllocation = true;
				return FALSE;
			}
		}
		return TRUE;
	}

	class ScopedAllocationFailure final
	{
	public:
		explicit ScopedAllocationFailure(const long index)
		{
			g_AllocationsUntilFailure = index;
			g_DeniedAllocation = false;
			m_Previous = _CrtSetAllocHook(AllocationFailureHook);
		}
		~ScopedAllocationFailure()
		{
			_CrtSetAllocHook(m_Previous);
			g_AllocationsUntilFailure = -1;
		}
	private:
		_CRT_ALLOC_HOOK m_Previous = nullptr;
	};

	unsigned CheckCopyFailureRollback()
	{
		const auto source = Parse(R"({"z":[1,2,{"long":"abcdefghijklmnopqrstuvwxyz0123456789"}],"a":{"nested":true}})");
		const auto original = Parse(R"({"keep":{"saved":[false,99.0]},"name":"previous-value"})");
		unsigned deniedCases = 0;
		for (long index = 0; index < 256; ++index)
		{
			auto target = original;
			bool threw = false;
			bool denied = false;
			{
				ScopedAllocationFailure failure(index);
				try
				{
					target = source;
				}
				catch (const std::bad_alloc&)
				{
					threw = true;
				}
				denied = g_DeniedAllocation;
			}
			if (!denied)
			{
				Require(!threw && Equal(target, source), "copy succeeds after failure sweep");
				Require(deniedCases != 0, "allocation fault injection exercised");
				return deniedCases;
			}
			Require(threw, "denied allocation throws bad_alloc");
			Require(Equal(target, original), "failed copy assignment preserves destination");
			++deniedCases;
		}
		throw std::runtime_error("allocation failure sweep did not reach success");
	}
#endif
}

int main(int argc, char** argv)
{
	try
	{
		const bool requireStrongCopy = argc == 2 &&
			std::string(argv[1]) == "--require-strong-copy";
		CheckValueContracts();
		unsigned allocationFailures = 0;
		if (requireStrongCopy)
		{
			CheckStrongCopyAlias();
#ifdef _DEBUG
			allocationFailures = CheckCopyFailureRollback();
#endif
		}
		std::cout << "checks=" << g_Checks
			<< " allocationFailures=" << allocationFailures
			<< " iteratorDebugLevel=" << _ITERATOR_DEBUG_LEVEL
			<< " strongCopy=" << requireStrongCopy << " failures=0\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "checks=" << g_Checks << " error=" << error.what() << '\n';
		return 1;
	}
}
```

## G05. 프로젝트 등록과 재현 절차

제품의 C++ 파일 추가는 없다. 기존 `Client/Default/Client.vcxproj`와 `.vcxproj.filters`의 `DataJson.h/.cpp` 항목을 그대로 사용한다. 두 새 Tools C++는 각각 자기 `main`을 가진 독립 console probe로 공용 script가 선택한 하나만 직접 `cl.exe`로 컴파일한다. Client/Engine 빌드 대상이나 제품 filter에 추가하면 제품에 불필요한 진입점을 섞으므로 `.vcxproj`와 `.filters` 등록은 필요하지 않다. out source snapshot과 EXE/OBJ/PDB/로그는 제품 소스나 commit 대상에 넣지 않는다.

`Build-Benchmark.ps1`의 `-Probe Benchmark`는 `DataJsonLoadingBenchmark.cpp`와 `benchmark.exe`, `-Probe ValueContracts`는 `ValueContracts.cpp`와 `value-contracts.exe`를 선택한다. 나머지 source snapshot, compiler 선택, flags와 receipt는 공통이다. 기본 MSVC 14.44 / SDK 10.0.26100.0을 `vswhere`로 찾고 `-VcVarsPath`, `-ToolsetVersion`, `-WindowsSdkVersion`, `-RepoRoot`로 명시할 수도 있다. RepoRoot 기본 계산은 script 본문에서 수행하여 PowerShell param 초기화 중 빈 `$PSScriptRoot`에 의존하지 않는다. 기존 출력 디렉터리와 결과를 덮어쓰지 않는다.

저장소 루트 PowerShell에서 현재 제품의 Debug/Release 계약을 독립적으로 실행한다. 두 실행의 출력 폴더는 새 경로여야 한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/DataJsonLoadingBenchmark/Run-ValueContracts.ps1 -OutputDirectory out/DataJsonContracts/current-Debug -Configuration Debug -RequireStrongCopy
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/DataJsonLoadingBenchmark/Run-ValueContracts.ps1 -OutputDirectory out/DataJsonContracts/current-Release -Configuration Release -RequireStrongCopy
```

특정 source를 검사할 때는 `-SourceHeader`와 `-SourceCpp`를 반드시 함께 지정한다. 선택한 값은 직접 실행 입력이 아니라 독립 빌드용 실제 C++ 파일이다. 다음 예는 보존된 현재 작업의 최종 후보를 사용한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/DataJsonLoadingBenchmark/Run-ValueContracts.ps1 -SourceHeader out/DebugEffectLoading20261004/candidate-v2/Client/Public/DataJson.h -SourceCpp out/DebugEffectLoading20261004/candidate-v2/Client/Private/DataJson.cpp -OutputDirectory out/DataJsonContracts/snapshot-Debug -Configuration Debug -Variant snapshot -RequireStrongCopy
```

성능 비교용 입력은 한 번 준비하고 같은 snapshot과 compiler flags를 사용한다. Prepare의 현재 제품 V1 사용 목록과 baseline DataJson snapshot을 만든 뒤 빌드한다. 아래 명령은 입력 수를 고정하지 않는다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/DataJsonLoadingBenchmark/Prepare-Corpus.ps1 -OutputDirectory out/DataJsonBench/input
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/DataJsonLoadingBenchmark/Build-Benchmark.ps1 -SourceHeader out/DataJsonBench/input/baseline/DataJson.h -SourceCpp out/DataJsonBench/input/baseline/DataJson.cpp -OutputDirectory out/DataJsonBench/baseline-Debug -Configuration Debug -Variant baseline
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/DataJsonLoadingBenchmark/Run-Benchmark.ps1 -Executable out/DataJsonBench/baseline-Debug/benchmark.exe -Corpus out/DataJsonBench/input/corpus.tsv -Workers 3 -Result out/DataJsonBench/results/baseline-debug3-a1.json
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/DataJsonLoadingBenchmark/Run-Benchmark.ps1 -Executable out/DataJsonBench/baseline-Debug/benchmark.exe -Corpus out/DataJsonBench/input/corpus.tsv -Workers 1 -Allocation -Result out/DataJsonBench/results/baseline-allocation.json
```

비교할 source를 별도 Build 출력 디렉터리에 준비하고 동일 corpus로 제한된 A/B/A 반복을 수행한다. 모든 컴파일과 값 계약 실패주입 실행을 성능 측정과 겹치지 않는다. 실패나 의미 digest 차이가 있으면 속도보다 원인을 먼저 확인한다. 실제 codec canonical 결과·resource 목록과 잘못된 문서의 출력 보존은 parser probe와 별도 실제 codec 검사로 확인한다.

이번 변경의 제품 검증 범위는 필요한 Client C++ 컴파일·링크와 DataJson 헤더 의존 TU의 일관성 확인이다. 이미 준비된 Engine·Shared·Server 의존 결과를 사용하여 같은 Client 프로젝트의 아래 target을 실행하고 compile log와 dependency/output 기록을 확인한다. 단일 DataJson OBJ만 기존 제품 OBJ에 섞지 않는다. 변경하지 않은 전체 Shader/FxCompile 재생성을 이번 코드 검증의 필수 조건으로 삼지 않으며, 이 제한된 target의 성공을 전체 Product 빌드·CSO 최신성·화면 성공으로 설명하지 않는다. 실제 완료 및 중단 상태는 대응 RESULT에 기록한다.

```powershell
& 'C:/Program Files/Microsoft Visual Studio/18/Insiders/MSBuild/Current/Bin/amd64/MSBuild.exe' `
  Client/Default/Client.vcxproj `
  '/t:PrepareForBuild;ResolveReferences;_ClCompile;_ResourceCompile;_Link;_Manifest;DeployClientRuntimeDependencies' `
  /m:1 /nodeReuse:false /p:Configuration=Debug /p:Platform=x64 `
  /p:PreferredToolArchitecture=x64 /p:WindowsSDKToolArchitecture=Native64Bit `
  /p:BuildProjectReferences=false /p:CL_MPCount=3 /v:minimal `
  '/bl:out/DebugEffectLoading20261004/client-cpp.binlog' `
  '/flp:LogFile=out/DebugEffectLoading20261004/client-cpp-diagnostic.log;Verbosity=diagnostic;Encoding=UTF-8'
git diff --check
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj' -Raw) | Out-Null
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj.filters' -Raw) | Out-Null
```

Data, Resources, publisher, Shared/Server wire 계약과 팀 public 인터페이스는 바꾸지 않는다. 자동 검증은 독립 native process와 제품 compile/link까지다. Client/UI 실행·종료·화면 조작은 하지 않는다. 사용자가 실제 Lobby에서 KoukuSaydon에 입장해 F1 Profiler Capture를 사용할 경우 기존 `Effect.Prepare.Document`, `Metadata`, `Renderer`, `Commit` CPU 구간과 사용자 화면을 별도로 기록한다. parser 성능을 GPU 시간이나 전체 진입 시간으로 대체하지 않는다.

## G06. 재현 PowerShell의 최종 전체 코드

네 script는 제품 프로젝트에 등록하는 C++ 파일이 아니라 저장소 루트에서 호출하는 독립 도구다. `Build-Benchmark`는 실제 source snapshot·compiler·flags·receipt를 소유하고, `Prepare-Corpus`는 현재 저장된 V1 효과 목록의 고정 입력을 만든다. `Run-Benchmark`는 측정 실행과 환경 metadata를, `Run-ValueContracts`는 현재 제품 또는 지정 source의 계약 빌드·실행을 담당한다. 공통 사용법은 [도구 README](C:/Users/tnest/Desktop/LostArk/Tools/DataJsonLoadingBenchmark/README.md)를 따른다.

`Prepare-Corpus`의 파일별 행은 `[pscustomobject]`로 만들어 Windows PowerShell 5의 `Measure-Object -Property bytes`가 실제 속성을 합산하도록 한다. source와 입력 snapshot의 전후 SHA 검사를 유지하며, 출력 디렉터리와 결과 덮어쓰기를 거부한다. 아래 전문은 현재 최종 script와 byte 단위로 일치한다.

### C:/Users/tnest/Desktop/LostArk/Tools/DataJsonLoadingBenchmark/Build-Benchmark.ps1 전체 코드

```powershell
param(
    [Parameter(Mandatory=$true)][string]$SourceHeader,
    [Parameter(Mandatory=$true)][string]$SourceCpp,
    [Parameter(Mandatory=$true)][string]$OutputDirectory,
    [ValidateSet('Debug','Release')][string]$Configuration='Debug',
    [ValidateSet('Benchmark','ValueContracts')][string]$Probe='Benchmark',
    [string]$Variant='snapshot',
    [string]$RepoRoot,
    [string]$VcVarsPath,
    [ValidatePattern('^[0-9.]+$')][string]$ToolsetVersion='14.44',
    [ValidatePattern('^[0-9.]+$')][string]$WindowsSdkVersion='10.0.26100.0'
)
$ErrorActionPreference='Stop'
if(!$RepoRoot){$RepoRoot=Split-Path -Parent (Split-Path -Parent $PSScriptRoot)}
$repo=(Resolve-Path -LiteralPath $RepoRoot).Path
$header=(Resolve-Path -LiteralPath $SourceHeader).Path
$cpp=(Resolve-Path -LiteralPath $SourceCpp).Path
$output=$ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputDirectory)
if(Test-Path -LiteralPath $output){throw "Output directory already exists: $output"}
if(!$VcVarsPath){
    $vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    if(!(Test-Path -LiteralPath $vswhere)){throw 'vswhere unavailable; specify -VcVarsPath.'}
    $installation=@(& $vswhere -latest -prerelease -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath)
    if($LASTEXITCODE -or !$installation.Count){throw 'Visual C++ installation unavailable; specify -VcVarsPath.'}
    $VcVarsPath=Join-Path $installation[0] 'VC/Auxiliary/Build/vcvars64.bat'
}
$vcvars=(Resolve-Path -LiteralPath $VcVarsPath).Path
$nativeName=if($Probe -eq 'ValueContracts'){'ValueContracts.cpp'}else{'DataJsonLoadingBenchmark.cpp'}
$executableName=if($Probe -eq 'ValueContracts'){'value-contracts.exe'}else{'benchmark.exe'}
$native=Join-Path $PSScriptRoot $nativeName
$defines=Join-Path $repo 'Client/Public/Client_Defines.h'
$engine=Join-Path $repo 'EngineSDK/Inc'
if(!(Test-Path -LiteralPath $defines) -or !(Test-Path -LiteralPath $engine)){throw 'Actual Client/EngineSDK headers are required.'}
foreach($path in @($repo,$header,$cpp,$output,$vcvars,$native)){
    if($path.IndexOfAny([char[]]@('"',"`r","`n",'%')) -ge 0){throw "Unsupported command path: $path"}
}
$sourceDirectory=Join-Path $output 'sources'
New-Item -ItemType Directory -Path $sourceDirectory -Force|Out-Null
$inputs=@(@{original=$header;name='DataJson.h'},@{original=$cpp;name='DataJson.cpp'},@{original=$defines;name='Client_Defines.h'})
$sourceRecords=@(foreach($inputFile in $inputs){
    $before=(Get-FileHash -LiteralPath $inputFile.original -Algorithm SHA256).Hash.ToLowerInvariant()
    $snapshot=Join-Path $sourceDirectory $inputFile.name
    Copy-Item -LiteralPath $inputFile.original -Destination $snapshot
    if($before -ne (Get-FileHash -LiteralPath $inputFile.original).Hash.ToLowerInvariant() -or $before -ne (Get-FileHash -LiteralPath $snapshot).Hash.ToLowerInvariant()){throw 'Source changed while snapshotting.'}
    [ordered]@{path=$inputFile.original;snapshot=$snapshot;sha256=$before}
})
$sourceRecords+=@([ordered]@{path=$native;sha256=(Get-FileHash -LiteralPath $native).Hash.ToLowerInvariant()})
$engineRecords=@(Get-ChildItem -LiteralPath $engine -Filter '*.h'|ForEach-Object{[ordered]@{path=$_.FullName;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}})
$flags=@('/nologo','/EHsc','/std:c++20','/utf-8','/wd4828','/W3','/O2','/sdl','/permissive-','/DNOMINMAX','/D_UNICODE','/DUNICODE','/D_WINDOWS')
if($Configuration -eq 'Debug'){$flags+=@('/MDd','/D_DEBUG','/D_ITERATOR_DEBUG_LEVEL=2')}else{$flags+=@('/MD','/DNDEBUG','/D_ITERATOR_DEBUG_LEVEL=0')}
$compileArgs=$flags+@('/I"'+$sourceDirectory+'"','/I"'+(Join-Path $repo 'Client/Public')+'"','/I"'+$engine+'"','/Fo"'+$output+'/"','/Fe"'+(Join-Path $output $executableName)+'"','"'+$native+'"','"'+(Join-Path $sourceDirectory 'DataJson.cpp')+'"','/link','/INCREMENTAL:NO','Psapi.lib')
$utf8=[Text.UTF8Encoding]::new($false)
$rsp=Join-Path $output 'compile.rsp'
[IO.File]::WriteAllText($rsp,($compileArgs -join "`r`n")+"`r`n",$utf8)
$command='call "'+$vcvars+'" '+$WindowsSdkVersion+' -vcvars_ver='+$ToolsetVersion+' >nul && cl.exe @"'+$rsp+'"'
$previousPreference=$ErrorActionPreference;$ErrorActionPreference='Continue'
& $env:COMSPEC /d /s /c $command *> (Join-Path $output 'compile.log')
$compileExit=$LASTEXITCODE;$ErrorActionPreference=$previousPreference
$receipt=[ordered]@{variant=$Variant;configuration=$Configuration;probe=$Probe;gitRef=(& git -C $repo rev-parse HEAD);utc=[DateTime]::UtcNow.ToString('o');command=$command;toolsetVersion=$ToolsetVersion;windowsSdkVersion=$WindowsSdkVersion;flags=$flags;compilerExitCode=$compileExit;sources=$sourceRecords;engineHeaders=$engineRecords}
[IO.File]::WriteAllText((Join-Path $output 'build-receipt.json'),($receipt|ConvertTo-Json -Depth 7),$utf8)
Get-Content -LiteralPath (Join-Path $output 'compile.log') -Tail 18
if($compileExit){throw "Native compilation failed: $compileExit"}
Write-Output (Join-Path $output $executableName)
```

### C:/Users/tnest/Desktop/LostArk/Tools/DataJsonLoadingBenchmark/Prepare-Corpus.ps1 전체 코드

```powershell
param(
    [Parameter(Mandatory=$true)][string]$OutputDirectory,
    [string]$RepoRoot
)
$ErrorActionPreference='Stop'
if(!$RepoRoot){$RepoRoot=Split-Path -Parent (Split-Path -Parent $PSScriptRoot)}
$repo=(Resolve-Path -LiteralPath $RepoRoot).Path
$destination=$ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputDirectory)
if(Test-Path -LiteralPath $destination){throw "Corpus output already exists; preserve frozen input: $destination"}
$utf8=[Text.UTF8Encoding]::new($false)
$baseline=Join-Path $destination 'baseline'
New-Item -ItemType Directory -Force $baseline,(Join-Path $destination 'corpus')|Out-Null
$sources=@(foreach($relative in @('Client/Public/DataJson.h','Client/Private/DataJson.cpp','Client/Public/Client_Defines.h')){
    $original=Join-Path $repo $relative
    $snapshot=Join-Path $baseline ([IO.Path]::GetFileName($relative))
    $hash=(Get-FileHash -LiteralPath $original).Hash.ToLowerInvariant()
    Copy-Item -LiteralPath $original -Destination $snapshot
    if($hash -ne (Get-FileHash -LiteralPath $snapshot).Hash.ToLowerInvariant() -or $hash -ne (Get-FileHash -LiteralPath $original).Hash.ToLowerInvariant()){throw 'Source changed during snapshot.'}
    [ordered]@{path=$relative;sha256=$hash}
})
$inputs=@('Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json','Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json','Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json','Data/Effects/EffectCatalog.json')
$dependencies=@($inputs|ForEach-Object{[ordered]@{path=$_;sha256=(Get-FileHash -LiteralPath (Join-Path $repo $_)).Hash.ToLowerInvariant()}})
$docs=@($inputs|ForEach-Object{Get-Content -LiteralPath (Join-Path $repo $_) -Encoding UTF8 -Raw|ConvertFrom-Json})
$ids=[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach($id in @('effect.kouku.card.match.bind.floor','effect.kouku.card.match.bind.release','effect.kouku.mario.marker.red','effect.kouku.mario.marker.blue','effect.kouku.mario.marker.yellow','effect.kouku.mario.ball.pop.red','effect.kouku.mario.ball.pop.blue','effect.kouku.mario.ball.pop.yellow','effect.kouku.mario.flyingball.hit')){$null=$ids.Add($id)}
function Add-Resource($row){if($row.kind -eq 'EFFECT' -and $row.resourceKind -in @('V1_EFFECT','V1_ELEMENT')){$null=$ids.Add([string]$row.assetId)}}
foreach($pattern in $docs[0].patterns){foreach($resource in $pattern.presentationOccurrences){Add-Resource $resource}}
foreach($fear in $docs[0].fearPresentations){if($fear.effectResource){Add-Resource $fear.effectResource}}
foreach($visual in $docs[0].targetedCombatVisuals){foreach($resource in $visual.resources){Add-Resource $resource};if($visual.contactEffectAssetId){$null=$ids.Add([string]$visual.contactEffectAssetId)}}
$used=[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach($pattern in $docs[1].patterns){foreach($resource in $pattern.presentationOccurrences){$null=$used.Add([string]$resource.resourceId)}}
foreach($resource in $docs[1].presentationResources){if($used.Remove([string]$resource.resourceId)){Add-Resource $resource}}
if($used.Count){throw 'Unresolved sequence resource.'}
$templates=[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach($instance in $docs[2].instances){if($instance.enabled){$null=$templates.Add([string]$instance.templateId)}}
foreach($template in $docs[2].templates){if($templates.Remove([string]$template.sequenceId)){foreach($effect in $template.effectTracks){if($effect.resourceKind -in @('V1_EFFECT','V1_ELEMENT')){$null=$ids.Add([string]$effect.resourceId)}}}}
if($templates.Count){throw 'Unresolved world sequence template.'}
$catalog=@{}
foreach($entry in $docs[3].effects){$catalog[[string]$entry.effectAssetId]=$entry}
$files=@();$lines=@();$index=0
$dataRoot=[IO.Path]::GetFullPath((Join-Path $repo 'Data'))+[IO.Path]::DirectorySeparatorChar
foreach($id in @($ids|Sort-Object)){
    if(!$catalog.ContainsKey($id)){throw "Missing effect catalog ID: $id"}
    $entry=$catalog[$id]
    if($entry.payloadKind -ne 'DIRECT_AUTHORED_DOCUMENT'){throw "Unexpected payload kind: $id"}
    $source=[IO.Path]::GetFullPath((Join-Path $dataRoot $entry.authoringPath))
    if(!$source.StartsWith($dataRoot,[StringComparison]::OrdinalIgnoreCase)){throw "Authoring path outside Data: $id"}
    $relative='corpus/{0:D3}.json' -f $index++
    $snapshot=Join-Path $destination $relative
    $before=(Get-FileHash -LiteralPath $source).Hash.ToLowerInvariant()
    Copy-Item -LiteralPath $source -Destination $snapshot
    $after=(Get-FileHash -LiteralPath $source).Hash.ToLowerInvariant()
    $copy=(Get-FileHash -LiteralPath $snapshot).Hash.ToLowerInvariant()
    if($before -ne $after -or $copy -ne $before){throw "Concurrent source mutation: $id"}
    $files += [pscustomobject][ordered]@{effectAssetId=$id;sourcePath=$source;snapshotPath=$relative;bytes=(Get-Item -LiteralPath $snapshot).Length;sha256=$copy}
    $lines += "$id`t$relative"
}
foreach($dependency in $dependencies){if($dependency.sha256 -ne (Get-FileHash -LiteralPath (Join-Path $repo $dependency.path)).Hash.ToLowerInvariant()){throw 'Dependency changed while preparing corpus.'}}
$manifest=[ordered]@{gitRef=(& git -C $repo rev-parse HEAD);utc=[DateTime]::UtcNow.ToString('o');corpusBoundary='KoukuSaydon saved V1 closure matching Collect_ProductEffectTargets branches; excludes Valtan, V2, actor/GPU preparation and live unsaved draft';sourceHashes=$sources;dependencyInputs=$dependencies;count=$files.Count;bytes=($files|Measure-Object -Property bytes -Sum).Sum;files=$files}
[IO.File]::WriteAllText((Join-Path $destination 'corpus-manifest.json'),($manifest|ConvertTo-Json -Depth 8),$utf8)
[IO.File]::WriteAllText((Join-Path $destination 'corpus.tsv'),($lines -join "`n")+"`n",$utf8)
[pscustomobject]$manifest|Select-Object gitRef,count,bytes|ConvertTo-Json
```

### C:/Users/tnest/Desktop/LostArk/Tools/DataJsonLoadingBenchmark/Run-Benchmark.ps1 전체 코드

```powershell
param(
    [Parameter(Mandatory=$true)][string]$Executable,
    [Parameter(Mandatory=$true)][string]$Corpus,
    [Parameter(Mandatory=$true)][string]$Result,
    [ValidateSet(1,3)][int]$Workers=1,
    [switch]$Allocation,
    [string]$CorpusManifest,
    [string]$BuildReceipt
)
$ErrorActionPreference='Stop'
$exe=(Resolve-Path -LiteralPath $Executable).Path
$corpusPath=(Resolve-Path -LiteralPath $Corpus).Path
$resultPath=$ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Result)
if(Test-Path -LiteralPath $resultPath){throw "Result already exists: $resultPath"}
if(Test-Path -LiteralPath ($resultPath+'.meta.json')){throw 'Result metadata already exists.'}
if($Allocation -and $Workers -ne 1){throw 'Allocation instrumentation requires one worker.'}
if(!$CorpusManifest){$CorpusManifest=Join-Path (Split-Path -Parent $corpusPath) 'corpus-manifest.json'}
if(!$BuildReceipt){$BuildReceipt=Join-Path (Split-Path -Parent $exe) 'build-receipt.json'}
$receipt=$null
if(Test-Path -LiteralPath $BuildReceipt){$receipt=Get-Content -LiteralPath $BuildReceipt -Encoding UTF8 -Raw|ConvertFrom-Json}
if($Allocation -and (!$receipt -or $receipt.configuration -ne 'Debug')){throw 'Allocation instrumentation requires a Debug build receipt.'}
New-Item -ItemType Directory -Force (Split-Path -Parent $resultPath)|Out-Null
$nativeArgs=@($corpusPath,[string]$Workers,$resultPath)
if($Allocation){$nativeArgs+='alloc'}
$before=[DateTime]::UtcNow.ToString('o')
# Relative corpus paths, when supplied, are resolved relative to the manifest directory.
Push-Location -LiteralPath (Split-Path -Parent $corpusPath)
try{& $exe @nativeArgs;$nativeExit=$LASTEXITCODE}finally{Pop-Location}
if($nativeExit){throw "Native benchmark failed: $nativeExit"}
$value=Get-Content -LiteralPath $resultPath -Encoding UTF8 -Raw|ConvertFrom-Json
$meta=[ordered]@{runId=[IO.Path]::GetFileNameWithoutExtension($resultPath);variant=$receipt.variant;configuration=$receipt.configuration;utcStart=$before;utcEnd=[DateTime]::UtcNow.ToString('o');executable=$exe;executableSha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();arguments=$nativeArgs;buildReceipt=$BuildReceipt;corpusSha256=(Get-FileHash -LiteralPath $corpusPath).Hash.ToLowerInvariant();corpusManifestSha256=$(if(Test-Path -LiteralPath $CorpusManifest){(Get-FileHash -LiteralPath $CorpusManifest).Hash.ToLowerInvariant()}else{$null});processorCount=[Environment]::ProcessorCount;os=[Environment]::OSVersion.VersionString}
[IO.File]::WriteAllText(($resultPath+'.meta.json'),($meta|ConvertTo-Json -Depth 5),[Text.UTF8Encoding]::new($false))
$value|Select-Object workers,debug,sizeofValue,contractCases,fileCount,inputBytes,failed,parseWallMs,digestWallMs,destructionWallMs,totalWallMs,processCpuMs,systemCpuBusyPercent,semanticDigestFnv1a64,parseAllocations|ConvertTo-Json
```

### C:/Users/tnest/Desktop/LostArk/Tools/DataJsonLoadingBenchmark/Run-ValueContracts.ps1 전체 코드

```powershell
param(
    [Parameter(Mandatory=$true)][string]$OutputDirectory,
    [ValidateSet('Debug','Release')][string]$Configuration='Debug',
    [string]$SourceHeader,
    [string]$SourceCpp,
    [switch]$RequireStrongCopy,
    [string]$Variant,
    [string]$RepoRoot,
    [string]$VcVarsPath,
    [ValidatePattern('^[0-9.]+$')][string]$ToolsetVersion='14.44',
    [ValidatePattern('^[0-9.]+$')][string]$WindowsSdkVersion='10.0.26100.0'
)

$ErrorActionPreference='Stop'
if(!$RepoRoot){$RepoRoot=Split-Path -Parent (Split-Path -Parent $PSScriptRoot)}
$repo=(Resolve-Path -LiteralPath $RepoRoot).Path
if([bool]$SourceHeader -ne [bool]$SourceCpp){
    throw 'Specify both -SourceHeader and -SourceCpp, or omit both for current product sources.'
}
$customSources=[bool]$SourceHeader
if(!$customSources){
    $SourceHeader=Join-Path $repo 'Client/Public/DataJson.h'
    $SourceCpp=Join-Path $repo 'Client/Private/DataJson.cpp'
}
if(!$Variant){$Variant=if($customSources){'custom'}else{'current'}}
$output=$ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputDirectory)
$buildArguments=@{
    SourceHeader=$SourceHeader
    SourceCpp=$SourceCpp
    OutputDirectory=$output
    Configuration=$Configuration
    Variant=$Variant
    Probe='ValueContracts'
    RepoRoot=$repo
    ToolsetVersion=$ToolsetVersion
    WindowsSdkVersion=$WindowsSdkVersion
}
if($VcVarsPath){$buildArguments.VcVarsPath=$VcVarsPath}

# Reuse the benchmark's source snapshot, compiler selection, flags and receipt.
# The shared builder requires a fresh output directory and preserves old results.
& (Join-Path $PSScriptRoot 'Build-Benchmark.ps1') @buildArguments
$executable=Join-Path $output 'value-contracts.exe'
$nativeArguments=@()
if($RequireStrongCopy){$nativeArguments+='--require-strong-copy'}
$resultText=Join-Path $output 'results.txt'
$utcStart=[DateTime]::UtcNow.ToString('o')
$previousPreference=$ErrorActionPreference
$ErrorActionPreference='Continue'
try{
    & $executable @nativeArguments *> $resultText
    $nativeExit=$LASTEXITCODE
}finally{
    $ErrorActionPreference=$previousPreference
}
$utcEnd=[DateTime]::UtcNow.ToString('o')
$text=(Get-Content -LiteralPath $resultText -Raw).Trim()
$parsed=[regex]::Match($text,
    '^checks=(\d+) allocationFailures=(\d+) iteratorDebugLevel=(\d+) strongCopy=([01]) failures=(\d+)$')
$result=[ordered]@{
    variant=$Variant
    configuration=$Configuration
    utcStart=$utcStart
    utcEnd=$utcEnd
    executable=$executable
    executableSha256=(Get-FileHash -LiteralPath $executable -Algorithm SHA256).Hash.ToLowerInvariant()
    arguments=$nativeArguments
    exitCode=$nativeExit
    buildReceipt=(Join-Path $output 'build-receipt.json')
    textOutput=$resultText
    requireStrongCopy=[bool]$RequireStrongCopy
    parsed=$parsed.Success
    boundary='Isolated actual DataJson value contracts; no performance measurement, Client, UI or GPU execution.'
}
if($parsed.Success){
    $result.checks=[int]$parsed.Groups[1].Value
    $result.injectedAllocationFailures=[int]$parsed.Groups[2].Value
    $result.iteratorDebugLevel=[int]$parsed.Groups[3].Value
    $result.strongCopy=($parsed.Groups[4].Value -eq '1')
    $result.failures=[int]$parsed.Groups[5].Value
}
[IO.File]::WriteAllText((Join-Path $output 'results.json'),
    ($result|ConvertTo-Json -Depth 5),[Text.UTF8Encoding]::new($false))
Write-Output $text
if($nativeExit -or !$parsed.Success -or $result.failures -ne 0){
    throw "DataJson value contracts failed; inspect $resultText"
}
```
