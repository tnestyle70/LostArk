#include "NpcPlacementPresentationService.h"

#include "DataJson.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <mutex>
#include <utility>

namespace
{
	std::mutex g_NpcPlacementMutex;
	std::map<uint32_t, std::map<std::string, std::string, std::less<>>>
		g_IdleClipsByLevel;
	std::string g_strStatus = "NPC placement presentation is not loaded.";

	std::filesystem::path Get_ModuleDirectory()
	{
		wchar_t buffer[32768]{};
		const DWORD length = GetModuleFileNameW(
			nullptr, buffer, static_cast<DWORD>(std::size(buffer)));
		if (0u == length || length >= std::size(buffer))
			return {};
		return std::filesystem::path(buffer).parent_path();
	}

	std::filesystem::path Find_Document(const char_t* pWorldId)
	{
		const std::wstring fileName =
			std::filesystem::path(std::string(pWorldId) +
				".npcpresentation.json").wstring();
		const std::filesystem::path module = Get_ModuleDirectory();
		const std::filesystem::path adjacent =
			module / L"DataFiles" / L"World" / fileName;
		if (std::filesystem::is_regular_file(adjacent))
			return adjacent;
		const std::filesystem::path parent = module.parent_path() /
			L"DataFiles" / L"World" / fileName;
		return std::filesystem::is_regular_file(parent) ? parent : adjacent;
	}
}

void Client::CNpcPlacementPresentationService::Begin_LevelLoad(
	const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_NpcPlacementMutex };
	g_IdleClipsByLevel.erase(iLevelIndex);
}

HRESULT Client::CNpcPlacementPresentationService::Load(
	const uint32_t iLevelIndex,
	const char_t* pWorldId)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || nullptr == pWorldId ||
		'\0' == pWorldId[0])
	{
		return E_INVALIDARG;
	}

	const std::filesystem::path path = Find_Document(pWorldId);
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		std::scoped_lock lock{ g_NpcPlacementMutex };
		g_strStatus = "NPC placement presentation document is missing: " +
			path.string();
		return E_FAIL;
	}
	const std::string text(
		(std::istreambuf_iterator<char_t>(input)),
		std::istreambuf_iterator<char_t>());

	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError))
	{
		std::scoped_lock lock{ g_NpcPlacementMutex };
		g_strStatus = "NPC placement presentation parse failed: " + parseError;
		return E_FAIL;
	}
	const DATA_JSON_VALUE* pSchema = root.Find("schema");
	const DATA_JSON_VALUE* pVersion = root.Find("formatVersion");
	const DATA_JSON_VALUE* pWorld = root.Find("worldId");
	const DATA_JSON_VALUE* pEntries = root.Find("entries");
	if (nullptr == pSchema || !pSchema->Is_String() ||
		pSchema->Get_String() != "lostark.npc-placement-presentation" ||
		nullptr == pVersion || !pVersion->Is_Number() ||
		pVersion->Get_Number() != 1.0 ||
		nullptr == pWorld || !pWorld->Is_String() ||
		pWorld->Get_String() != pWorldId ||
		nullptr == pEntries || !pEntries->Is_Array())
	{
		std::scoped_lock lock{ g_NpcPlacementMutex };
		g_strStatus = "NPC placement presentation contract mismatch.";
		return E_FAIL;
	}

	std::map<std::string, std::string, std::less<>> staged;
	for (const DATA_JSON_VALUE& value : pEntries->Get_Array())
	{
		if (!value.Is_Object() || 2u != value.Get_Object().size())
		{
			std::scoped_lock lock{ g_NpcPlacementMutex };
			g_strStatus = "NPC placement presentation entry shape is invalid.";
			return E_FAIL;
		}
		const DATA_JSON_VALUE* pPlacementId = value.Find("placementId");
		const DATA_JSON_VALUE* pIdleClip = value.Find("idleClip");
		if (nullptr == pPlacementId || !pPlacementId->Is_String() ||
			pPlacementId->Get_String().empty() ||
			nullptr == pIdleClip || !pIdleClip->Is_String() ||
			pIdleClip->Get_String().empty() ||
			pIdleClip->Get_String().size() > 64u ||
			!staged.emplace(
				pPlacementId->Get_String(), pIdleClip->Get_String()).second)
		{
			std::scoped_lock lock{ g_NpcPlacementMutex };
			g_strStatus = "NPC placement presentation entry is invalid.";
			return E_FAIL;
		}
	}

	std::scoped_lock lock{ g_NpcPlacementMutex };
	g_IdleClipsByLevel[iLevelIndex] = std::move(staged);
	g_strStatus = "NPC placement presentation ready.";
	return S_OK;
}

const std::string* Client::CNpcPlacementPresentationService::Find_IdleClip(
	const uint32_t iLevelIndex,
	const std::string_view placementId)
{
	if (placementId.empty())
		return nullptr;
	std::scoped_lock lock{ g_NpcPlacementMutex };
	const auto level = g_IdleClipsByLevel.find(iLevelIndex);
	if (level == g_IdleClipsByLevel.end())
		return nullptr;
	const auto entry = level->second.find(placementId);
	return entry != level->second.end() ? &entry->second : nullptr;
}

const std::string& Client::CNpcPlacementPresentationService::Get_Status()
{
	std::scoped_lock lock{ g_NpcPlacementMutex };
	return g_strStatus;
}
