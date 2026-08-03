#include "LevelCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <fstream>
#include <cmath>
#include <initializer_list>
#include <set>

namespace
{
	using namespace Client;

	std::vector<LEVEL_CATALOG_ENTRY> g_Entries;
	std::string g_Status = "Level catalog is not initialized.";
	bool_t g_isInitialized = false;

	bool_t ReadText(
		const std::filesystem::path& path,
		std::string& outText)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input.is_open())
			return false;
		outText.assign(
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>());
		return input.good() || input.eof();
	}

	bool_t ReadStringArray(
		const DATA_JSON_VALUE* pValue,
		std::vector<std::string>& outValues)
	{
		if (nullptr == pValue || !pValue->Is_Array())
			return false;
		for (const DATA_JSON_VALUE& value : pValue->Get_Array())
		{
			if (!value.Is_String() || value.Get_String().empty())
				return false;
			outValues.push_back(value.Get_String());
		}
		return true;
	}

	bool_t ReadFinitePair(
		const DATA_JSON_VALUE* pValue,
		f32_t& outFirst,
		f32_t& outSecond)
	{
		if (nullptr == pValue || !pValue->Is_Array() ||
			2u != pValue->Get_Array().size())
		{
			return false;
		}

		const DATA_JSON_VALUE& first = pValue->Get_Array()[0];
		const DATA_JSON_VALUE& second = pValue->Get_Array()[1];
		if (!first.Is_Number() || !second.Is_Number() ||
			!std::isfinite(first.Get_Number()) ||
			!std::isfinite(second.Get_Number()))
		{
			return false;
		}

		outFirst = static_cast<f32_t>(first.Get_Number());
		outSecond = static_cast<f32_t>(second.Get_Number());
		return true;
	}

	bool_t ReadMapLoadScope(
		const DATA_JSON_VALUE* pValue,
		MAP_LOAD_SCOPE& outScope)
	{
		if (nullptr == pValue)
			return false;
		if (pValue->Is_Null())
		{
			outScope = {};
			return true;
		}
		if (!pValue->Is_Object())
			return false;

		const DATA_JSON_VALUE* pBackground =
			pValue->Find("includeBackground");
		if (nullptr == pBackground || !pBackground->Is_Boolean() ||
			!ReadFinitePair(
				pValue->Find("minimumXZ"),
				outScope.minimumX,
				outScope.minimumZ) ||
			!ReadFinitePair(
				pValue->Find("maximumXZ"),
				outScope.maximumX,
				outScope.maximumZ) ||
			outScope.minimumX >= outScope.maximumX ||
			outScope.minimumZ >= outScope.maximumZ)
		{
			return false;
		}

		outScope.isEnabled = true;
		outScope.includeBackground = pBackground->Get_Boolean();
		return true;
	}

	CLIENT_SCENARIO ParseScenario(const std::string& id)
	{
		if ("front.lobby" == id) return CLIENT_SCENARIO::FRONT_LOBBY;
		if ("world.bern" == id) return CLIENT_SCENARIO::WORLD_BERN;
		if ("raid.valtan.arena" == id) return CLIENT_SCENARIO::RAID_VALTAN_ARENA;
		if ("dev.map.active" == id) return CLIENT_SCENARIO::DEVELOPMENT_MAP;
		if ("asset.character.lance-master" == id)
			return CLIENT_SCENARIO::DEVELOPMENT_CHARACTER;
		if ("render.hdr-readback" == id) return CLIENT_SCENARIO::DEVELOPMENT_HDR;
		if ("effect.preview" == id) return CLIENT_SCENARIO::DEVELOPMENT_EFFECT;
		if ("ui.hud.layout" == id) return CLIENT_SCENARIO::DEVELOPMENT_UI;
		return CLIENT_SCENARIO::END;
	}

	LEVEL ParseLevel(const std::string& level)
	{
		if ("LOBBY" == level) return LEVEL::LOBBY;
		if ("BERN" == level) return LEVEL::BERN;
		if ("VALTAN_ARENA" == level) return LEVEL::VALTAN_ARENA;
		if ("DEVELOPMENT" == level) return LEVEL::DEVELOPMENT;
		return LEVEL::END;
	}

	bool_t HasExecutionMetadata(
		const LEVEL_CATALOG_ENTRY& entry,
		const std::initializer_list<const char_t*> domains,
		const std::initializer_list<const char_t*> tools)
	{
		const std::vector<std::string> expectedDomains(
			domains.begin(), domains.end());
		const std::vector<std::string> expectedTools(
			tools.begin(), tools.end());
		return entry.AssetDomains == expectedDomains &&
			entry.Tools == expectedTools;
	}

	bool_t ValidateExecutionMetadata(
		const LEVEL_CATALOG_ENTRY& entry)
	{
		switch (entry.eScenario)
		{
		case CLIENT_SCENARIO::FRONT_LOBBY:
			return HasExecutionMetadata(entry, {}, {});
		case CLIENT_SCENARIO::WORLD_BERN:
			return HasExecutionMetadata(
				entry, { "Map", "Character" }, {});
		case CLIENT_SCENARIO::RAID_VALTAN_ARENA:
			return HasExecutionMetadata(
				entry,
				{ "Map", "Character", "Deploy", "Effect", "UI" },
				{});
		case CLIENT_SCENARIO::DEVELOPMENT_MAP:
			return HasExecutionMetadata(entry, { "Map" }, { "MapTool" });
		case CLIENT_SCENARIO::DEVELOPMENT_CHARACTER:
			return HasExecutionMetadata(
				entry, { "Character" }, { "AnimationTool" });
		case CLIENT_SCENARIO::DEVELOPMENT_HDR:
		case CLIENT_SCENARIO::DEVELOPMENT_EFFECT:
			return HasExecutionMetadata(entry, { "Effect" }, { "EffectTool" });
		case CLIENT_SCENARIO::DEVELOPMENT_UI:
			return HasExecutionMetadata(entry, { "UI" }, { "HUDLayoutTool" });
		default:
			return false;
		}
	}
}

bool_t Client::CLevelCatalog::Initialize()
{
	if (g_isInitialized)
		return true;

	const std::filesystem::path path =
		CProjectDataRoot::Resolve(L"Levels/LevelCatalog.json");
	std::string text;
	if (path.empty() || !ReadText(path, text))
	{
		g_Status = "Could not read Data/Levels/LevelCatalog.json.";
		return false;
	}

	DATA_JSON_VALUE root;
	std::string error;
	if (!CDataJson::Parse(text, root, error) || !root.Is_Object())
	{
		g_Status = "Invalid level catalog JSON: " + error;
		return false;
	}

	const DATA_JSON_VALUE* pSchema = root.Find("schema");
	const DATA_JSON_VALUE* pVersion = root.Find("formatVersion");
	const DATA_JSON_VALUE* pLevels = root.Find("levels");
	if (nullptr == pSchema || !pSchema->Is_String() ||
		"lostark.level-catalog" != pSchema->Get_String() ||
		nullptr == pVersion || !pVersion->Is_Number() ||
		1.0 != pVersion->Get_Number() ||
		nullptr == pLevels || !pLevels->Is_Array())
	{
		g_Status = "Level catalog root contract mismatch.";
		return false;
	}

	std::vector<LEVEL_CATALOG_ENTRY> staged;
	std::set<std::string> ids;
	std::set<CLIENT_SCENARIO> scenarios;
	for (const DATA_JSON_VALUE& value : pLevels->Get_Array())
	{
		if (!value.Is_Object())
		{
			g_Status = "Level catalog entry must be an object.";
			return false;
		}

		const DATA_JSON_VALUE* pId = value.Find("id");
		const DATA_JSON_VALUE* pKind = value.Find("kind");
		const DATA_JSON_VALUE* pLevel = value.Find("level");
		const DATA_JSON_VALUE* pMapArea = value.Find("mapAreaId");
		const DATA_JSON_VALUE* pMapLoadBounds =
			value.Find("mapLoadBounds");
		if (nullptr == pId || !pId->Is_String() ||
			nullptr == pKind || !pKind->Is_String() ||
			nullptr == pLevel || !pLevel->Is_String() ||
			nullptr == pMapArea ||
			nullptr == pMapLoadBounds ||
			(!pMapArea->Is_Null() && !pMapArea->Is_String()))
		{
			g_Status = "Level catalog entry field mismatch.";
			return false;
		}

		LEVEL_CATALOG_ENTRY entry;
		entry.strStableId = pId->Get_String();
		entry.strKind = pKind->Get_String();
		entry.eScenario = ParseScenario(entry.strStableId);
		entry.eLevel = ParseLevel(pLevel->Get_String());
		if (pMapArea->Is_String())
			entry.strMapAreaId = pMapArea->Get_String();
		if (CLIENT_SCENARIO::END == entry.eScenario ||
			LEVEL::END == entry.eLevel ||
			("product" != entry.strKind &&
			 "development" != entry.strKind) ||
			!ReadStringArray(value.Find("assetDomains"), entry.AssetDomains) ||
			!ReadStringArray(value.Find("tools"), entry.Tools) ||
			!ValidateExecutionMetadata(entry) ||
			!ReadMapLoadScope(pMapLoadBounds, entry.MapLoadScope) ||
			(entry.MapLoadScope.isEnabled && entry.strMapAreaId.empty()) ||
			!ids.insert(entry.strStableId).second ||
			!scenarios.insert(entry.eScenario).second)
		{
			g_Status = "Invalid or duplicate level catalog entry: " +
				entry.strStableId;
			return false;
		}
		staged.push_back(std::move(entry));
	}

	if (staged.size() != static_cast<size_t>(CLIENT_SCENARIO::END))
	{
		g_Status = "Level catalog does not cover every CLIENT_SCENARIO.";
		return false;
	}

	g_Entries = std::move(staged);
	g_Status = "Level catalog ready: " +
		std::to_string(g_Entries.size()) + " scenarios.";
	g_isInitialized = true;
	return true;
}

const Client::LEVEL_CATALOG_ENTRY* Client::CLevelCatalog::Find(
	const CLIENT_SCENARIO eScenario)
{
	if (!Initialize())
		return nullptr;
	for (const LEVEL_CATALOG_ENTRY& entry : g_Entries)
		if (entry.eScenario == eScenario)
			return &entry;
	return nullptr;
}

const Client::LEVEL_CATALOG_ENTRY* Client::CLevelCatalog::Find(
	const std::string& stableId)
{
	if (!Initialize())
		return nullptr;
	for (const LEVEL_CATALOG_ENTRY& entry : g_Entries)
		if (entry.strStableId == stableId)
			return &entry;
	return nullptr;
}

const std::vector<Client::LEVEL_CATALOG_ENTRY>&
Client::CLevelCatalog::Get_Entries()
{
	Initialize();
	return g_Entries;
}

const std::string& Client::CLevelCatalog::Get_Status()
{
	return g_Status;
}
