#include "MapLightDocument.h"

#include "DataJson.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.map-light-presentation";
	constexpr uint32_t FORMAT_VERSION = 1u;

	bool_t ReadTextFile(
		const std::filesystem::path& path,
		std::string& outText)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input.is_open())
			return false;
		std::ostringstream buffer;
		buffer << input.rdbuf();
		if (input.bad())
			return false;
		outText = buffer.str();
		return true;
	}

	bool_t IsExactObject(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key)
			{
				return nullptr != value.Find(key);
			});
	}

	bool_t ReadString(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		std::string& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_String() ||
			value->Get_String().empty())
		{
			return false;
		}
		outValue = value->Get_String();
		return true;
	}

	bool_t ReadFinite(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const double minimum,
		const double maximum,
		f32_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Number())
			return false;
		const double number = value->Get_Number();
		if (!std::isfinite(number) || number < minimum || number > maximum)
			return false;
		outValue = static_cast<f32_t>(number);
		return std::isfinite(outValue);
	}

	bool_t ReadVector(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const size_t expectedCount,
		const double minimum,
		const double maximum,
		std::vector<f32_t>& outValues)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Array() ||
			value->Get_Array().size() != expectedCount)
		{
			return false;
		}

		std::vector<f32_t> staged;
		staged.reserve(expectedCount);
		for (const DATA_JSON_VALUE& component : value->Get_Array())
		{
			if (!component.Is_Number())
				return false;
			const double number = component.Get_Number();
			if (!std::isfinite(number) || number < minimum || number > maximum)
				return false;
			const f32_t narrowed = static_cast<f32_t>(number);
			if (!std::isfinite(narrowed))
				return false;
			staged.push_back(narrowed);
		}
		outValues = std::move(staged);
		return true;
	}

	bool_t IsStableToken(
		const std::string& value,
		const size_t maximumLength,
		const bool_t allowColon)
	{
		if (value.empty() || value.size() > maximumLength)
			return false;
		return std::all_of(value.begin(), value.end(),
			[allowColon](const unsigned char character)
			{
				return std::isalnum(character) || character == '.' ||
					character == '_' || character == '-' ||
					(allowColon && character == ':');
			});
	}
}

bool_t Client::CMapLightDocument::Load(
	const std::filesystem::path& path,
	const std::string& expectedAreaId,
	std::string& outStatus)
{
	std::string text;
	if (path.empty() || expectedAreaId.empty() || !ReadTextFile(path, text))
	{
		outStatus = "Map light document is unreadable: " + path.string();
		return false;
	}

	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError))
	{
		outStatus = "Map light document parse failed: " + parseError;
		return false;
	}
	if (!IsExactObject(root, {
			"schema", "formatVersion", "areaId", "provenance", "lights" }))
	{
		outStatus = "Map light document root has unexpected properties";
		return false;
	}

	std::string schema;
	std::string areaId;
	std::string provenance;
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* lights = root.Find("lights");
	if (!ReadString(root, "schema", schema) || schema != SCHEMA ||
		nullptr == version || !version->Is_Number() ||
		!std::isfinite(version->Get_Number()) ||
		version->Get_Number() != static_cast<double>(FORMAT_VERSION) ||
		!ReadString(root, "areaId", areaId) || areaId != expectedAreaId ||
		!IsStableToken(areaId, 128u, false) ||
		!ReadString(root, "provenance", provenance) ||
		(provenance != "SOURCE_EXACT" &&
			provenance != "SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED" &&
			provenance != "PROJECT_AUTHORED") ||
		nullptr == lights || !lights->Is_Array() ||
		lights->Get_Array().empty() ||
		lights->Get_Array().size() > MAX_LIGHT_COUNT)
	{
		outStatus = "Map light document header is invalid";
		return false;
	}

	std::vector<MAP_POINT_LIGHT_RECORD> staged;
	staged.reserve(lights->Get_Array().size());
	std::unordered_set<std::string> lightIds;
	std::unordered_set<std::string> sourceObjectIds;
	for (const DATA_JSON_VALUE& entry : lights->Get_Array())
	{
		if (!IsExactObject(entry, {
				"lightId", "sourceLevel", "sourceObjectId", "position",
				"radiusMeters", "falloffExponent", "color", "brightness" }))
		{
			outStatus = "Map point light has unexpected properties";
			return false;
		}

		MAP_POINT_LIGHT_RECORD record;
		std::vector<f32_t> position;
		std::vector<f32_t> color;
		if (!ReadString(entry, "lightId", record.lightId) ||
			!IsStableToken(record.lightId, 128u, false) ||
			!ReadString(entry, "sourceLevel", record.sourceLevel) ||
			!IsStableToken(record.sourceLevel, 128u, false) ||
			!ReadString(entry, "sourceObjectId", record.sourceObjectId) ||
			!IsStableToken(record.sourceObjectId, 256u, true) ||
			!ReadVector(entry, "position", 3u, -100000.0, 100000.0,
				position) ||
			!ReadFinite(entry, "radiusMeters", 0.01, 1000.0,
				record.radiusMeters) ||
			!ReadFinite(entry, "falloffExponent", 0.01, 64.0,
				record.falloffExponent) ||
			!ReadVector(entry, "color", 4u, 0.0, 1.0, color) ||
			!ReadFinite(entry, "brightness", 0.0, 64.0,
				record.brightness))
		{
			outStatus = "Map point light field is invalid";
			return false;
		}
		if (!lightIds.insert(record.lightId).second ||
			!sourceObjectIds.insert(record.sourceObjectId).second)
		{
			outStatus = "Map point light identity is duplicated: " +
				record.lightId;
			return false;
		}

		record.position = { position[0], position[1], position[2] };
		record.color = { color[0], color[1], color[2], color[3] };
		staged.push_back(std::move(record));
	}

	m_AreaId = std::move(areaId);
	m_Provenance = std::move(provenance);
	m_Lights = std::move(staged);
	m_isReady = true;
	outStatus = "Map light presentation ready: " +
		std::to_string(m_Lights.size()) + " point lights";
	return true;
}

void Client::CMapLightDocument::Clear()
{
	m_AreaId.clear();
	m_Provenance.clear();
	m_Lights.clear();
	m_isReady = false;
}
