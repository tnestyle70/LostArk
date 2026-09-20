#include "NpcActionEffectCueDocument.h"

#include "DataJson.h"
#include "Effect_Catalog.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <unordered_map>

void Client::NPC_ACTION_EFFECT_PLAYBACK_STATE::Reset()
{
	strClip.clear();
	fElapsedSeconds = 0.f;
	iNextCue = 0u;
	bActive = false;
}

namespace
{
	using namespace Client;

	std::unordered_map<std::string, std::vector<NPC_ACTION_EFFECT_CUE>> g_Cues;
	const std::vector<NPC_ACTION_EFFECT_CUE> g_Empty;

	bool_t IsStableId(const std::string& value)
	{
		if (value.empty() || value.size() > 128u)
			return false;
		for (const unsigned char character : value)
		{
			if (!std::isalnum(character) && '_' != character &&
				'-' != character && '.' != character)
			{
				return false;
			}
		}
		return true;
	}

	bool_t ReadString(const DATA_JSON_VALUE& object, const char_t* key,
		std::string& output)
	{
		const DATA_JSON_VALUE* value = object.Find(key);
		if (nullptr == value || !value->Is_String())
			return false;
		output = value->Get_String();
		return true;
	}

	bool_t ReadUInt(const DATA_JSON_VALUE& object, const char_t* key,
		const std::uint32_t maximum, std::uint32_t& output)
	{
		const DATA_JSON_VALUE* value = object.Find(key);
		if (nullptr == value || !value->Is_Number())
			return false;
		const f64_t raw = value->Get_Number();
		if (!std::isfinite(raw) || raw < 0.0 || raw > static_cast<f64_t>(maximum))
			return false;
		output = static_cast<std::uint32_t>(raw);
		return true;
	}
}

bool_t Client::CNpcActionEffectCueDocument::Load(
	const std::string& strArchetypeId, std::string& strOutStatus)
{
	if (g_Cues.contains(strArchetypeId))
		return true;
	if (!IsStableId(strArchetypeId))
	{
		strOutStatus = "archetype id is not a stable id";
		return false;
	}
	/* An archetype with no document is normal: it keeps the existing binding
	   path. Only a present but malformed document is an error. */
	g_Cues.emplace(strArchetypeId, std::vector<NPC_ACTION_EFFECT_CUE>{});
	const std::filesystem::path path = CProjectDataRoot::Resolve(
		std::filesystem::path("Effects/NpcActionCues") /
		(strArchetypeId + ".npcactioncues.json"));
	std::error_code error;
	if (!std::filesystem::is_regular_file(path, error))
		return true;

	std::ifstream input(path, std::ios::binary);
	const std::string text{ std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	DATA_JSON_VALUE root;
	std::string status;
	if (!CDataJson::Parse(text, root, status) || !root.Is_Object())
	{
		strOutStatus = "npc action cue document is not valid JSON: " + status;
		return false;
	}
	std::string schema, archetype;
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* rows = root.Find("cues");
	if (!ReadString(root, "schema", schema) ||
		schema != "lostark.npc-action-effect-cues" ||
		nullptr == version || !version->Is_Number() ||
		1.0 != version->Get_Number() ||
		!ReadString(root, "archetypeId", archetype) ||
		archetype != strArchetypeId ||
		nullptr == rows || !rows->Is_Array())
	{
		strOutStatus = "npc action cue header is invalid";
		return false;
	}

	std::vector<NPC_ACTION_EFFECT_CUE> staged;
	for (const DATA_JSON_VALUE& row : rows->Get_Array())
	{
		NPC_ACTION_EFFECT_CUE cue;
		if (!row.Is_Object() ||
			!ReadString(row, "cueId", cue.strCueId) ||
			!IsStableId(cue.strCueId) ||
			!ReadString(row, "effectAssetId", cue.strEffectAssetId) ||
			!IsStableId(cue.strEffectAssetId) ||
			!ReadString(row, "clip", cue.strClip) || cue.strClip.empty() ||
			!ReadUInt(row, "startMs", 600000u, cue.iStartMs) ||
			!ReadUInt(row, "durationMs", 600000u, cue.iDurationMs) ||
			!ReadString(row, "bone", cue.strBone))
		{
			strOutStatus = "npc action cue row is invalid";
			return false;
		}
		if (const DATA_JSON_VALUE* follow = row.Find("followBone"))
		{
			if (!follow->Is_Boolean())
			{
				strOutStatus = "npc action cue followBone is invalid";
				return false;
			}
			cue.bFollowBone = follow->Get_Boolean();
		}
		/* A cue whose document is absent from the catalog is dropped rather
		   than failing the archetype: a partially restored library still
		   plays what it has. */
		if (!CEffectCatalog::Contains(cue.strEffectAssetId))
			continue;
		staged.push_back(std::move(cue));
	}
	std::stable_sort(staged.begin(), staged.end(),
		[](const NPC_ACTION_EFFECT_CUE& left, const NPC_ACTION_EFFECT_CUE& right)
		{
			return left.iStartMs < right.iStartMs;
		});
	g_Cues[strArchetypeId] = std::move(staged);
	return true;
}

const std::vector<Client::NPC_ACTION_EFFECT_CUE>&
Client::CNpcActionEffectCueDocument::Get_Cues(const std::string& strArchetypeId)
{
	const auto iterator = g_Cues.find(strArchetypeId);
	return g_Cues.end() == iterator ? g_Empty : iterator->second;
}

bool_t Client::CNpcActionEffectCueDocument::Has_Clip(
	const std::string& strArchetypeId, const std::string& strClip)
{
	for (const NPC_ACTION_EFFECT_CUE& cue : Get_Cues(strArchetypeId))
	{
		if (cue.strClip == strClip)
			return true;
	}
	return false;
}
