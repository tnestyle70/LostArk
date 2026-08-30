#include "EstherActionSoundCueDocument.h"

#include "DataJson.h"
#include "GameInstance.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "SoundCueCatalog.h"

#include <cmath>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <functional>
#include <limits>
#include <set>
#include <system_error>

std::vector<Client::ESTHER_ACTION_SOUND_CUE>
	Client::CEstherActionSoundCueDocument::s_Cues;
std::uint32_t Client::CEstherActionSoundCueDocument::s_iFixedTickHz = 30u;
bool_t Client::CEstherActionSoundCueDocument::s_bLoaded = false;

namespace
{
	using namespace Client;

	bool_t IsStableId(const std::string& value)
	{
		if (value.empty() || value.size() > 96u)
			return false;
		for (const unsigned char character : value)
		{
			if (!std::isalnum(character) && '_' != character && '-' != character &&
				'.' != character)
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
		if (nullptr == value || !value->Is_String() ||
			!IsStableId(value->Get_String()))
		{
			return false;
		}
		output = value->Get_String();
		return true;
	}

	bool_t ReadU32(const DATA_JSON_VALUE& object, const char_t* key,
		std::uint32_t& output)
	{
		const DATA_JSON_VALUE* value = object.Find(key);
		if (nullptr == value || !value->Is_Number() ||
			!std::isfinite(value->Get_Number()) || value->Get_Number() < 0.0 ||
			value->Get_Number() >
				static_cast<double>((std::numeric_limits<std::uint32_t>::max)()) ||
			std::floor(value->Get_Number()) != value->Get_Number())
		{
			return false;
		}
		output = static_cast<std::uint32_t>(value->Get_Number());
		return true;
	}

	bool_t ParseOwnerKind(const std::string& value,
		ESTHER_ACTION_SOUND_OWNER_KIND& output)
	{
		if ("PLAYER_ACTION" == value)
			output = ESTHER_ACTION_SOUND_OWNER_KIND::PLAYER_ACTION;
		else if ("NPC_ACTION" == value)
			output = ESTHER_ACTION_SOUND_OWNER_KIND::NPC_ACTION;
		else
			return false;
		return true;
	}

	bool_t IsResourceSoundId(const std::string& value)
	{
		if (value.rfind("Sound/", 0u) != 0u ||
			value.size() < 5u || value.substr(value.size() - 4u) != ".wav" ||
			value.find("..") != std::string::npos ||
			std::filesystem::path(value).is_absolute())
		{
			return false;
		}
		return true;
	}
}

void Client::ESTHER_ACTION_SOUND_PLAYBACK_STATE::Reset()
{
	eOwnerKind = ESTHER_ACTION_SOUND_OWNER_KIND::END;
	strOwnerId.clear();
	strActionId.clear();
	iActionStartTick = 0u;
	AttemptedCueIds.clear();
}

bool_t Client::CEstherActionSoundCueDocument::Load(
	std::string& strOutStatus)
{
	const std::filesystem::path path =
		CProjectDataRoot::Resolve("Sound/EstherActionSoundCues.json");
	std::ifstream input(path, std::ios::binary);
	if (path.empty() || !input)
	{
		strOutStatus = "EstherActionSoundCues.json not found.";
		return false;
	}
	const std::string text{
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError) || !root.Is_Object() ||
		4u != root.Get_Object().size())
	{
		strOutStatus = parseError.empty() ?
			"Esther action Sound root is not an exact JSON object." : parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = root.Find("schema");
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* cues = root.Find("cues");
	std::uint32_t fixedTickHz = 0u;
	if (nullptr == schema || !schema->Is_String() ||
		"lostark.esther-action-sound-cues" != schema->Get_String() ||
		nullptr == version || !version->Is_Number() ||
		1.0 != version->Get_Number() ||
		!ReadU32(root, "fixedTickHz", fixedTickHz) || 30u != fixedTickHz ||
		nullptr == cues || !cues->Is_Array() || cues->Get_Array().empty())
	{
		strOutStatus = "Esther action Sound schema/version/tick contract is invalid.";
		return false;
	}

	std::set<std::string> cueIds;
	std::vector<ESTHER_ACTION_SOUND_CUE> staged;
	staged.reserve(cues->Get_Array().size());
	for (const DATA_JSON_VALUE& value : cues->Get_Array())
	{
		if (!value.Is_Object() || 11u != value.Get_Object().size())
		{
			strOutStatus = "Esther action Sound cue fields are not exact.";
			return false;
		}
		ESTHER_ACTION_SOUND_CUE cue;
		std::string ownerKind;
		const DATA_JSON_VALUE* volume = value.Find("volume");
		const DATA_JSON_VALUE* once = value.Find("once");
		if (!ReadString(value, "cueId", cue.strCueId) ||
			!ReadString(value, "ownerKind", ownerKind) ||
			!ParseOwnerKind(ownerKind, cue.eOwnerKind) ||
			!ReadString(value, "ownerId", cue.strOwnerId) ||
			!ReadString(value, "actionId", cue.strActionId) ||
			!ReadString(value, "catalogOwnerId", cue.strCatalogOwnerId) ||
			!ReadString(value, "soundEvent", cue.strSoundEvent) ||
			!ReadU32(value, "startMs", cue.iStartMs) ||
			!ReadU32(value, "lateToleranceMs", cue.iLateToleranceMs) ||
			nullptr == volume || !volume->Is_Number() ||
			!std::isfinite(volume->Get_Number()) ||
			volume->Get_Number() <= 0.0 || volume->Get_Number() > 1.0 ||
			nullptr == once || !once->Is_Boolean() || !once->Get_Boolean() ||
			!ReadString(value, "timingBasis", cue.strTimingBasis) ||
			"PROJECT_TUNED_EDGE" != cue.strTimingBasis ||
			cue.iStartMs > 600000u || cue.iLateToleranceMs > 10000u ||
			!cueIds.insert(cue.strCueId).second)
		{
			strOutStatus = "Esther action Sound cue is invalid or duplicated.";
			return false;
		}
		cue.fVolume = static_cast<f32_t>(volume->Get_Number());
		cue.bOnce = true;
		if ((ESTHER_ACTION_SOUND_OWNER_KIND::PLAYER_ACTION == cue.eOwnerKind &&
			 ("PLAYER" != cue.strOwnerId || "ESTHER_CAST" != cue.strActionId)) ||
			(ESTHER_ACTION_SOUND_OWNER_KIND::NPC_ACTION == cue.eOwnerKind &&
			 (cue.strOwnerId.rfind("NPC_", 0u) != 0u ||
			  "esther.strike" != cue.strActionId)))
		{
			strOutStatus = "Esther action Sound owner/action tuple is invalid.";
			return false;
		}
		const std::vector<std::string>& variants =
			CSoundCueCatalog::Find_Variants(
				cue.strCatalogOwnerId, cue.strSoundEvent);
		if (variants.empty())
		{
			strOutStatus = "Esther action Sound event has no catalog variants: " +
				cue.strSoundEvent;
			return false;
		}
		for (const std::string& variant : variants)
		{
			if (!IsResourceSoundId(variant))
			{
				strOutStatus = "Esther action Sound variant is not a Resources-relative WAV: " +
					variant;
				return false;
			}
			const std::filesystem::path soundPath =
				CRuntimeAssetRoot::Resolve(variant);
			std::error_code assetError;
			if (soundPath.empty() ||
				!std::filesystem::is_regular_file(soundPath, assetError) ||
				assetError)
			{
				strOutStatus =
					"Esther action Sound variant is missing from Resources: " +
					variant;
				return false;
			}
		}
		staged.push_back(std::move(cue));
	}

	s_Cues = std::move(staged);
	s_iFixedTickHz = fixedTickHz;
	s_bLoaded = true;
	strOutStatus = "Loaded " + std::to_string(s_Cues.size()) +
		" Esther replicated action Sound cue(s).";
	return true;
}

bool_t Client::CEstherActionSoundCueDocument::Play_Due(
	const ESTHER_ACTION_SOUND_OWNER_KIND eOwnerKind,
	const std::string& strOwnerId,
	const std::string& strActionId,
	const std::uint32_t iServerTick,
	const std::uint32_t iActionStartTick,
	ESTHER_ACTION_SOUND_PLAYBACK_STATE& State,
	std::string& strOutStatus)
{
	strOutStatus.clear();
	if (!s_bLoaded || ESTHER_ACTION_SOUND_OWNER_KIND::END == eOwnerKind ||
		!IsStableId(strOwnerId) || !IsStableId(strActionId) ||
		0u == iServerTick || 0u == iActionStartTick || 0u == s_iFixedTickHz)
	{
		strOutStatus = "Esther action Sound runtime input is invalid or not loaded.";
		return false;
	}
	const std::uint32_t ageTicks = iServerTick - iActionStartTick;
	if (iServerTick != iActionStartTick &&
		ageTicks > static_cast<std::uint32_t>((std::numeric_limits<std::int32_t>::max)()))
	{
		strOutStatus = "Esther action Sound start tick is in the future.";
		return false;
	}
	if (State.eOwnerKind != eOwnerKind || State.strOwnerId != strOwnerId ||
		State.strActionId != strActionId ||
		State.iActionStartTick != iActionStartTick)
	{
		State.Reset();
		State.eOwnerKind = eOwnerKind;
		State.strOwnerId = strOwnerId;
		State.strActionId = strActionId;
		State.iActionStartTick = iActionStartTick;
	}
	const std::uint64_t ageMs =
		static_cast<std::uint64_t>(ageTicks) * 1000u / s_iFixedTickHz;
	std::size_t played = 0u;
	std::size_t dropped = 0u;
	for (const ESTHER_ACTION_SOUND_CUE& cue : s_Cues)
	{
		if (cue.eOwnerKind != eOwnerKind || cue.strOwnerId != strOwnerId ||
			cue.strActionId != strActionId ||
			State.AttemptedCueIds.end() !=
				State.AttemptedCueIds.find(cue.strCueId) || ageMs < cue.iStartMs)
		{
			continue;
		}
		State.AttemptedCueIds.insert(cue.strCueId);
		if (ageMs > static_cast<std::uint64_t>(cue.iStartMs) +
			cue.iLateToleranceMs)
		{
			++dropped;
			continue;
		}
		const std::vector<std::string>& variants =
			CSoundCueCatalog::Find_Variants(
				cue.strCatalogOwnerId, cue.strSoundEvent);
		if (variants.empty())
		{
			++dropped;
			continue;
		}
		const std::size_t variantIndex = variants.size() == 1u ? 0u :
			(static_cast<std::size_t>(iActionStartTick) +
			 std::hash<std::string>{}(cue.strCueId)) % variants.size();
		const std::filesystem::path soundPath =
			CRuntimeAssetRoot::Resolve(variants[variantIndex]);
		std::error_code assetError;
		if (soundPath.empty() ||
			!std::filesystem::is_regular_file(soundPath, assetError) || assetError ||
			FAILED(CGameInstance::Get().Play_Sound(
				soundPath.wstring(), cue.fVolume)))
		{
			++dropped;
			continue;
		}
		++played;
	}
	strOutStatus = "Esther action Sound occurrence played " +
		std::to_string(played) + ", dropped " + std::to_string(dropped) + ".";
	return true;
}
