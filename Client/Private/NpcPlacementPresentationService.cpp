#include "NpcPlacementPresentationService.h"

#include "DataJson.h"

#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <mutex>
#include <utility>

namespace
{
	std::mutex g_NpcPlacementMutex;
	using PRESENTATION_MAP = std::map<
		std::string,
		Client::NPC_PLACEMENT_PRESENTATION_ENTRY,
		std::less<>>;
	std::map<uint32_t, PRESENTATION_MAP> g_PresentationsByLevel;
	std::string g_strStatus = "NPC placement presentation is not loaded.";

	bool Is_StableId(const std::string_view value)
	{
		if (value.empty() || value.size() > 128u)
			return false;
		for (const unsigned char character : value)
		{
			if (0 == std::isalnum(character) && character != '_' &&
				character != '.' && character != '-')
			{
				return false;
			}
		}
		return true;
	}

	bool Is_StableClip(const std::string_view value)
	{
		if (value.empty() || value.size() > 64u)
			return false;
		for (const unsigned char character : value)
		{
			if (0 == std::isalnum(character) && character != '_' &&
				character != '~' && character != '.' && character != '-')
			{
				return false;
			}
		}
		return true;
	}

	bool Read_NullableClip(
		const Client::DATA_JSON_VALUE* pValue,
		std::string& outClip)
	{
		outClip.clear();
		if (nullptr == pValue)
			return false;
		if (pValue->Is_Null())
			return true;
		if (!pValue->Is_String() ||
			!Is_StableClip(pValue->Get_String()))
		{
			return false;
		}
		outClip = pValue->Get_String();
		return true;
	}

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
	if (iLevelIndex >= ETOUI(LEVEL::END))
		return;

	/* The level loader may still be presenting the currently committed level
	while the next document is parsed on its worker thread.  Keep that committed
	map alive until Load has validated a complete replacement.  Erasing here
	turned a missing/corrupt v2 document into a partial commit and also made a
	failed level transition discard the presentation that was still in use. */
	std::scoped_lock lock{ g_NpcPlacementMutex };
	g_strStatus = "NPC placement presentation load staged; committed data retained.";
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
		pVersion->Get_Number() != 2.0 ||
		nullptr == pWorld || !pWorld->Is_String() ||
		pWorld->Get_String() != pWorldId ||
		nullptr == pEntries || !pEntries->Is_Array() ||
		4u != root.Get_Object().size())
	{
		std::scoped_lock lock{ g_NpcPlacementMutex };
		g_strStatus = "NPC placement presentation contract mismatch.";
		return E_FAIL;
	}

	PRESENTATION_MAP staged;
	for (const DATA_JSON_VALUE& value : pEntries->Get_Array())
	{
		if (!value.Is_Object() || 4u != value.Get_Object().size())
		{
			std::scoped_lock lock{ g_NpcPlacementMutex };
			g_strStatus = "NPC placement presentation entry shape is invalid.";
			return E_FAIL;
		}
		const DATA_JSON_VALUE* pPlacementId = value.Find("placementId");
		const DATA_JSON_VALUE* pIdleClip = value.Find("idleClip");
		const DATA_JSON_VALUE* pWalkClip = value.Find("walkClip");
		const DATA_JSON_VALUE* pActions = value.Find("actions");
		NPC_PLACEMENT_PRESENTATION_ENTRY entry;
		if (nullptr == pPlacementId || !pPlacementId->Is_String() ||
			!Is_StableId(pPlacementId->Get_String()) ||
			!Read_NullableClip(pIdleClip, entry.strIdleClip) ||
			!Read_NullableClip(pWalkClip, entry.strWalkClip) ||
			nullptr == pActions || !pActions->Is_Array() ||
			pActions->Get_Array().size() > 32u)
		{
			std::scoped_lock lock{ g_NpcPlacementMutex };
			g_strStatus = "NPC placement presentation entry is invalid.";
			return E_FAIL;
		}
		entry.strPlacementId = pPlacementId->Get_String();
		for (const DATA_JSON_VALUE& actionValue : pActions->Get_Array())
		{
			if (!actionValue.Is_Object() ||
				5u != actionValue.Get_Object().size())
			{
				std::scoped_lock lock{ g_NpcPlacementMutex };
				g_strStatus = "NPC placement action shape is invalid.";
				return E_FAIL;
			}
			const DATA_JSON_VALUE* pActionId = actionValue.Find("actionId");
			const DATA_JSON_VALUE* pClipName = actionValue.Find("clipName");
			const DATA_JSON_VALUE* pLoop = actionValue.Find("loop");
			const DATA_JSON_VALUE* pPlaybackRate =
				actionValue.Find("playbackRate");
			const DATA_JSON_VALUE* pBlendSeconds =
				actionValue.Find("blendSeconds");
			if (nullptr == pActionId || !pActionId->Is_String() ||
				!Is_StableId(pActionId->Get_String()) ||
				pActionId->Get_String() == "npc.idle" ||
				pActionId->Get_String() == "npc.move.walk" ||
				nullptr == pClipName || !pClipName->Is_String() ||
				!Is_StableClip(pClipName->Get_String()) ||
				nullptr == pLoop || !pLoop->Is_Boolean() ||
				nullptr == pPlaybackRate || !pPlaybackRate->Is_Number() ||
				nullptr == pBlendSeconds || !pBlendSeconds->Is_Number() ||
				!std::isfinite(pPlaybackRate->Get_Number()) ||
				pPlaybackRate->Get_Number() < 0.1 ||
				pPlaybackRate->Get_Number() > 4.0 ||
				!std::isfinite(pBlendSeconds->Get_Number()) ||
				pBlendSeconds->Get_Number() < 0.0 ||
				pBlendSeconds->Get_Number() > 2.0)
			{
				std::scoped_lock lock{ g_NpcPlacementMutex };
				g_strStatus = "NPC placement action is invalid.";
				return E_FAIL;
			}
			NPC_PLACEMENT_ACTION_BINDING action;
			action.strActionId = pActionId->Get_String();
			action.strClipName = pClipName->Get_String();
			action.isLoop = pLoop->Get_Boolean();
			action.fPlaybackRate = static_cast<f32_t>(
				pPlaybackRate->Get_Number());
			action.fBlendSeconds = static_cast<f32_t>(
				pBlendSeconds->Get_Number());
			const std::string actionId = action.strActionId;
			if (!entry.ActionBindings.emplace(
					actionId, std::move(action)).second)
			{
				std::scoped_lock lock{ g_NpcPlacementMutex };
				g_strStatus = "NPC placement action ID is duplicated.";
				return E_FAIL;
			}
		}
		const std::string placementId = entry.strPlacementId;
		if (!staged.emplace(placementId, std::move(entry)).second)
		{
			std::scoped_lock lock{ g_NpcPlacementMutex };
			g_strStatus = "NPC placement ID is duplicated.";
			return E_FAIL;
		}
	}

	std::scoped_lock lock{ g_NpcPlacementMutex };
	g_PresentationsByLevel[iLevelIndex] = std::move(staged);
	g_strStatus = "NPC placement presentation ready.";
	return S_OK;
}

bool_t Client::CNpcPlacementPresentationService::Try_Get_Presentation(
	const uint32_t iLevelIndex,
	const std::string_view placementId,
	NPC_PLACEMENT_PRESENTATION_ENTRY& outEntry)
{
	if (placementId.empty())
		return false;
	std::scoped_lock lock{ g_NpcPlacementMutex };
	const auto level = g_PresentationsByLevel.find(iLevelIndex);
	if (level == g_PresentationsByLevel.end())
		return false;
	const auto entry = level->second.find(placementId);
	if (entry == level->second.end())
		return false;
	outEntry = entry->second;
	return true;
}

const std::string& Client::CNpcPlacementPresentationService::Get_Status()
{
	std::scoped_lock lock{ g_NpcPlacementMutex };
	return g_strStatus;
}
