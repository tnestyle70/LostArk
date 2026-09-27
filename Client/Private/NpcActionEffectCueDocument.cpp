#include "NpcActionEffectCueDocument.h"

#include "DataJson.h"
#include "Effect_Catalog.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
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
	/* A document that failed stays failed with its reason. Without this a
	   rejected parse would be cached as an empty success on the next call. */
	std::unordered_map<std::string, std::string> g_Failures;
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
		/* Milliseconds are whole. Truncating instead would silently move a
		   source time such as 2299.9999523 to 2299, so round to the nearest
		   millisecond and refuse a value that is not one. */
		const f64_t rounded = static_cast<f64_t>(std::llround(raw));
		if (std::abs(rounded - raw) > 0.001 ||
			rounded < 0.0 || rounded > static_cast<f64_t>(maximum))
		{
			return false;
		}
		output = static_cast<std::uint32_t>(rounded);
		return true;
	}
}

bool_t Client::CNpcActionEffectCueDocument::Load(
	const std::string& strArchetypeId, std::string& strOutStatus)
{
	if (g_Cues.contains(strArchetypeId))
		return true;
	if (const auto failed = g_Failures.find(strArchetypeId);
		g_Failures.end() != failed)
	{
		strOutStatus = failed->second;
		return false;
	}
	if (!IsStableId(strArchetypeId))
	{
		strOutStatus = "archetype id is not a stable id";
		return false;
	}
	/* Nothing is cached before the document parses. A malformed document
	   commits its reason to g_Failures instead of an empty cue list, so the
	   next call repeats the same failure rather than reporting success. */
	const auto reject = [&strArchetypeId, &strOutStatus](std::string reason)
	{
		strOutStatus = std::move(reason);
		g_Failures[strArchetypeId] = strOutStatus;
		return false;
	};
	const std::filesystem::path path = CProjectDataRoot::Resolve(
		std::filesystem::path("Effects/NpcActionCues") /
		(strArchetypeId + ".npcactioncues.json"));
	std::error_code error;
	if (!std::filesystem::is_regular_file(path, error))
	{
		/* An archetype with no document is normal: it keeps the existing
		   binding path. */
		g_Cues.emplace(strArchetypeId, std::vector<NPC_ACTION_EFFECT_CUE>{});
		return true;
	}

	std::ifstream input(path, std::ios::binary);
	const std::string text{ std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	DATA_JSON_VALUE root;
	std::string status;
	if (!CDataJson::Parse(text, root, status) || !root.Is_Object())
		return reject("npc action cue document is not valid JSON: " + status);
	std::string schema, archetype;
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* rows = root.Find("cues");
	/* Version 2 adds the source sound notify. Version 1 documents keep their
	   exact meaning; any other version is refused rather than guessed. */
	const bool_t bVersionSupported = nullptr != version && version->Is_Number() &&
		(1.0 == version->Get_Number() || 2.0 == version->Get_Number());
	if (!ReadString(root, "schema", schema) ||
		schema != "lostark.npc-action-effect-cues" ||
		!bVersionSupported ||
		!ReadString(root, "archetypeId", archetype) ||
		archetype != strArchetypeId ||
		nullptr == rows || !rows->Is_Array())
	{
		return reject("npc action cue header is invalid");
	}
	const bool_t bSoundSupported = 2.0 == version->Get_Number();

	std::vector<NPC_ACTION_EFFECT_CUE> staged;
	std::vector<std::string> seenCueIds;
	for (const DATA_JSON_VALUE& row : rows->Get_Array())
	{
		NPC_ACTION_EFFECT_CUE cue;
		if (!row.Is_Object() ||
			!ReadString(row, "cueId", cue.strCueId) ||
			!IsStableId(cue.strCueId) ||
			!ReadString(row, "effectAssetId", cue.strEffectAssetId) ||
			(!cue.strEffectAssetId.empty() && !IsStableId(cue.strEffectAssetId)) ||
			!ReadString(row, "clip", cue.strClip) || cue.strClip.empty() ||
			!ReadUInt(row, "startMs", 600000u, cue.iStartMs) ||
			!ReadUInt(row, "durationMs", 600000u, cue.iDurationMs) ||
			!ReadString(row, "bone", cue.strBone))
		{
			return reject("npc action cue row is invalid");
		}
		/* A repeated cue id would make the spawned placement id ambiguous,
		   which the live-cue bookkeeping relies on being unique. */
		if (seenCueIds.end() != std::find(seenCueIds.begin(), seenCueIds.end(),
			cue.strCueId))
		{
			return reject("npc action cue id is repeated: " + cue.strCueId);
		}
		seenCueIds.push_back(cue.strCueId);
		if (const DATA_JSON_VALUE* follow = row.Find("followBone"))
		{
			if (!follow->Is_Boolean())
				return reject("npc action cue followBone is invalid");
			cue.bFollowBone = follow->Get_Boolean();
		}
		if (const DATA_JSON_VALUE* sound = row.Find("sound"))
		{
			/* The sound block is the version 2 addition. A version 1 document
			   carrying one is a generation mismatch, not a field to ignore. */
			if (!bSoundSupported)
				return reject("npc action cue sound needs formatVersion 2");
			if (!sound->Is_Object() ||
				!ReadString(*sound, "soundClass", cue.strSoundClass) ||
				cue.strSoundClass.empty() ||
				!ReadString(*sound, "event", cue.strSoundEvent) ||
				cue.strSoundEvent.empty())
			{
				return reject("npc action cue sound is invalid");
			}
			const DATA_JSON_VALUE* alternate = sound->Find("alternateEvent");
			const DATA_JSON_VALUE* suffix =
				sound->Find("alternateModelTagSuffix");
			/* The original condition needs both halves: which event and what
			   selects it. One without the other cannot be evaluated. */
			if ((nullptr == alternate) != (nullptr == suffix))
				return reject("npc action cue sound alternate is incomplete");
			if (nullptr != alternate &&
				(!ReadString(*sound, "alternateEvent",
					cue.strSoundEventAlternate) ||
				cue.strSoundEventAlternate.empty() ||
				!ReadString(*sound, "alternateModelTagSuffix",
					cue.strSoundAlternateModelTagSuffix) ||
				cue.strSoundAlternateModelTagSuffix.empty()))
			{
				return reject("npc action cue sound alternate is invalid");
			}
		}
		/* A row with neither a visual nor a sound carries nothing. An empty
		   asset is only meaningful for the sound-only notifies that the source
		   schedules where no particle exists. */
		if (cue.strEffectAssetId.empty() && cue.strSoundEvent.empty())
			return reject("npc action cue row has no effect and no sound");
		/* A cue whose document is absent from the catalog is dropped rather
		   than failing the archetype: a partially restored library still
		   plays what it has. A sound-only row has no catalog entry to match. */
		if (!cue.strEffectAssetId.empty() &&
			!CEffectCatalog::Contains(cue.strEffectAssetId))
		{
			continue;
		}
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
