#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <map>
#include <string>
#include <string_view>
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
	static DATA_JSON_VALUE Null();
	static DATA_JSON_VALUE Boolean(bool_t value);
	static DATA_JSON_VALUE Number(double value);
	static DATA_JSON_VALUE String(string value);
	static DATA_JSON_VALUE Array(ARRAY value);
	static DATA_JSON_VALUE Object(OBJECT value);

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
	const string& Get_String() const { return m_String; }
	const ARRAY& Get_Array() const { return m_Array; }
	const OBJECT& Get_Object() const { return m_Object; }
	const DATA_JSON_VALUE* Find(string_view key) const;

private:
	DATA_JSON_TYPE m_eType = DATA_JSON_TYPE::NULL_VALUE;
	bool_t m_Boolean = false;
	double m_Number = {};
	string m_String;
	ARRAY m_Array;
	OBJECT m_Object;
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
