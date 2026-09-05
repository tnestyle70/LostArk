#include "KoukuSaydonCompositionDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <Windows.h>
#include <io.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr std::string_view COMPOSITION_SCHEMA =
		"lostark.kouku-saydon-composition";
	constexpr std::uint32_t FORMAT_VERSION = 2u;
	constexpr std::uint32_t FIXED_TICK_HZ = 30u;
	constexpr std::string_view COMPOSITION_ID =
		"boss.composition.kakulsaydon.gate1";
	constexpr std::string_view ENCOUNTER_ID =
		"ENCOUNTER_KAKULSAYDON_G1";
	constexpr std::string_view BOSS_ARCHETYPE_ID =
		"BOSS_KAKULSAYDON_G1_KOUKU";
	constexpr std::string_view BOSS_PLACEMENT_ID =
		"boss.kakulsaydon.g1.kouku";
	constexpr std::string_view AREA_ID = "LV_LUT_MIDNIGHTC_ED";
	constexpr std::string_view GENERATED_PATTERN_PREFIX =
		"KAKULSAYDON_G1_PATTERN_";
	constexpr std::string_view GENERATED_STAGE_PREFIX = "STAGE_";
	constexpr std::uint32_t MAX_REVISION =
		(std::numeric_limits<std::uint32_t>::max)() - 1u;
	constexpr std::uint32_t MAX_NEXT_ORDINAL = 1000000u;
	constexpr std::uint32_t MAX_TIME_MS = 600000u;
	constexpr std::size_t MAX_PATTERNS = 4096u;
	constexpr std::size_t MAX_STAGES_PER_PATTERN = 1024u;
	constexpr std::size_t MAX_PRODUCT_STAGES_PER_PATTERN = 64u;
	constexpr std::size_t MAX_OCCURRENCES_PER_STAGE = 4096u;
	constexpr std::size_t MAX_DOCUMENT_OCCURRENCES = 65536u;
	constexpr std::uintmax_t MAX_COMPOSITION_BYTES = 16u * 1024u * 1024u;
	constexpr std::uintmax_t MAX_REFERENCE_BYTES = 16u * 1024u * 1024u;

	struct ACTION_PROFILE_CONTRACT final
	{
		const char_t* pProfileId;
		const char_t* pModelAssetId;
	};

	constexpr std::array<ACTION_PROFILE_CONTRACT, 4u> ACTION_PROFILES = {{
		{ "MN_RPCT_05", "Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05" },
		{ "MN_RPCT_06", "Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06" },
		{ "MN_RPCT_07", "Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05" },
		{ "MN_RPCZ_00", "Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00" },
	}};

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char_t* const pName,
		const DATA_JSON_TYPE eType)
	{
		const DATA_JSON_VALUE* const pValue = object.Find(pName);
		return nullptr != pValue && pValue->Get_Type() == eType ?
			pValue : nullptr;
	}

	bool_t Has_ExactProperties(
		const DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> names)
	{
		if (!object.Is_Object() || object.Get_Object().size() != names.size())
			return false;
		return std::all_of(names.begin(), names.end(),
			[&object](const std::string_view name)
			{
				return nullptr != object.Find(name);
			});
	}

	bool_t Is_StableId(const std::string_view value)
	{
		if (value.empty() || value.size() > 128u || value == "." || value == "..")
			return false;
		return std::all_of(value.begin(), value.end(), [](const unsigned char value)
		{
			return (value >= 'a' && value <= 'z') ||
				(value >= 'A' && value <= 'Z') ||
				(value >= '0' && value <= '9') ||
				value == '_' || value == '-' || value == '.';
		});
	}

	bool_t Is_DisplayName(const std::string_view value)
	{
		return !value.empty() && value.size() <= 255u &&
			std::none_of(value.begin(), value.end(), [](const unsigned char value)
			{
				return value < 0x20u;
			});
	}

	bool_t Is_LowerSha256(const std::string_view value)
	{
		return 64u == value.size() &&
			std::all_of(value.begin(), value.end(), [](const unsigned char value)
			{
				return (value >= '0' && value <= '9') ||
					(value >= 'a' && value <= 'f');
			});
	}

	bool_t Try_ParseUnsigned(
		const DATA_JSON_VALUE& value,
		const std::uint32_t maximum,
		std::uint32_t& outValue)
	{
		if (!value.Is_Number() || value.Was_FloatingPointToken())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number < 0.0 ||
			number > static_cast<double>(maximum) || std::floor(number) != number)
		{
			return false;
		}
		outValue = static_cast<std::uint32_t>(number);
		return true;
	}

	bool_t Try_ParsePlayRate(
		const DATA_JSON_VALUE& value,
		f32_t& outValue)
	{
		if (!value.Is_Number())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number < 0.01 || number > 16.0)
			return false;
		outValue = static_cast<f32_t>(number);
		return std::isfinite(outValue);
	}

	bool_t Try_ParseGeneratedOrdinal(
		const std::string_view value,
		const std::string_view prefix,
		const std::uint32_t nextOrdinal)
	{
		if (!value.starts_with(prefix))
			return true;
		const std::string_view suffix = value.substr(prefix.size());
		if (suffix.empty() || suffix.front() == '0')
			return false;
		std::uint32_t ordinal = 0u;
		const std::from_chars_result parsed = std::from_chars(
			suffix.data(), suffix.data() + suffix.size(), ordinal);
		return parsed.ec == std::errc{} &&
			parsed.ptr == suffix.data() + suffix.size() &&
			ordinal > 0u && ordinal < nextOrdinal;
	}

	bool_t Is_AuthoringStatus(const std::string_view value)
	{
		return value == "DRAFT" || value == "PRODUCT";
	}

	bool_t Is_Category(const std::string_view value)
	{
		return value == "NORMAL" || value == "MECHANIC";
	}

	bool_t Is_StageKind(const std::string_view value)
	{
		return value == "WINDUP" || value == "ACTIVE" ||
			value == "RECOVERY";
	}

	bool_t Is_EndPolicy(const std::string_view value)
	{
		return value == "EXACT" || value == "HOLD_LAST_POSE" ||
			value == "LOOP_TO_WINDOW";
	}

	const ACTION_PROFILE_CONTRACT* Find_Profile(const std::string_view profileId)
	{
		const auto found = std::find_if(ACTION_PROFILES.begin(), ACTION_PROFILES.end(),
			[profileId](const ACTION_PROFILE_CONTRACT& profile)
			{
				return profileId == profile.pProfileId;
			});
		return found == ACTION_PROFILES.end() ? nullptr : &*found;
	}

	bool_t Read_Text(
		const std::filesystem::path& path,
		const std::uintmax_t maximumBytes,
		std::string& outText,
		std::string& outStatus,
		const std::string_view label)
	{
		std::error_code error;
		const std::uintmax_t size = std::filesystem::file_size(path, error);
		if (path.empty() || error || size > maximumBytes)
		{
			outStatus = std::string(label) +
				" is missing or exceeds its bounded size: " + path.string();
			return false;
		}
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			outStatus = std::string(label) + " could not be opened: " + path.string();
			return false;
		}
		std::string staged{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		if (input.bad() || staged.size() != size)
		{
			outStatus = std::string(label) + " could not be read completely: " +
				path.string();
			return false;
		}
		outText = std::move(staged);
		return true;
	}

	bool_t Is_ExpectedCompositionPath(const std::filesystem::path& path)
	{
		if (path.empty() || path.filename() != L"KoukuSaydonComposition.json")
			return false;
		for (const std::filesystem::path& component : path)
		{
			if (component == L"." || component == L"..")
				return false;
		}
		return true;
	}

	bool_t Validate_Shape(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		std::string& outStatus)
	{
		if (document.iFormatVersion != FORMAT_VERSION ||
			document.iRevision < 1u || document.iRevision > MAX_REVISION ||
			document.strCompositionId != COMPOSITION_ID ||
			document.strEncounterId != ENCOUNTER_ID ||
			document.strBossArchetypeId != BOSS_ARCHETYPE_ID ||
			document.strBossPlacementId != BOSS_PLACEMENT_ID ||
			document.strAreaId != AREA_ID ||
			document.iFixedTickHz != FIXED_TICK_HZ ||
			document.iNextPatternOrdinal < 1u ||
			document.iNextPatternOrdinal > MAX_NEXT_ORDINAL ||
			document.Patterns.size() > MAX_PATTERNS ||
			document.PlayAllPatternIds.size() > MAX_PATTERNS)
		{
			outStatus = "KoukuSaydon composition header or bounded collection size is invalid.";
			return false;
		}

		std::unordered_set<std::string> patternIds;
		std::unordered_set<std::string> actionIds;
		std::unordered_set<std::string> occurrenceIds;
		std::size_t totalOccurrences = 0u;
		for (const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern : document.Patterns)
		{
			if (!pattern.strLoadError.empty())
			{
				DATA_JSON_VALUE preserved;
				if (!Is_StableId(pattern.strPatternId) ||
					!patternIds.insert(pattern.strPatternId).second ||
					pattern.strPreservedJson.empty() ||
					!CDataJson::Parse(pattern.strPreservedJson, preserved, outStatus))
				{
					outStatus = "Isolated KoukuSaydon Pattern has lost its preserved JSON.";
					return false;
				}
				continue;
			}
			if (!Is_StableId(pattern.strPatternId) ||
				!patternIds.insert(pattern.strPatternId).second ||
				!Try_ParseGeneratedOrdinal(pattern.strPatternId,
					GENERATED_PATTERN_PREFIX, document.iNextPatternOrdinal) ||
				pattern.strActorProfileId.empty() ||
				CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(pattern.strActorProfileId) != pattern.strActorProfileId ||
				(pattern.strAuthoringStatus == "PRODUCT" && pattern.strActorProfileId != "MN_RPCZ_00") ||
				!Is_DisplayName(pattern.strDisplayName) ||
				!Is_AuthoringStatus(pattern.strAuthoringStatus) ||
				!Is_Category(pattern.strCategory) ||
				pattern.iNextStageOrdinal < 1u ||
				pattern.iNextStageOrdinal > MAX_NEXT_ORDINAL ||
				pattern.iNextAnimationOrdinal < 1u ||
				pattern.iNextAnimationOrdinal > MAX_NEXT_ORDINAL ||
				pattern.Stages.size() > MAX_STAGES_PER_PATTERN ||
				(pattern.strAuthoringStatus == "PRODUCT" &&
				 pattern.Stages.size() > MAX_PRODUCT_STAGES_PER_PATTERN))
			{
				outStatus = "KoukuSaydon Pattern identity, state, counter, or Stage count is invalid: " +
					pattern.strPatternId;
				return false;
			}

			std::uint64_t patternDurationMs = 0u;
			std::unordered_set<std::string> stageIds;
			const std::string occurrencePrefix = pattern.strPatternId + ".animation.";
			for (const KOUKU_SAYDON_COMPOSITION_STAGE& stage : pattern.Stages)
			{
				patternDurationMs += stage.iDurationMs;
				if (patternDurationMs > MAX_TIME_MS) { outStatus = "Pattern exceeds 600 seconds."; return false; }
				if (!Is_StableId(stage.strStageId) ||
					!stageIds.insert(stage.strStageId).second ||
					!Try_ParseGeneratedOrdinal(stage.strStageId,
						GENERATED_STAGE_PREFIX, pattern.iNextStageOrdinal) ||
					!Is_StableId(stage.strActionId) ||
					!actionIds.insert(stage.strActionId).second ||
					!Is_StageKind(stage.strStageKind) ||
					0u == stage.iDurationMs || stage.iDurationMs > MAX_TIME_MS ||
					stage.AnimationOccurrences.size() > MAX_OCCURRENCES_PER_STAGE ||
					stage.AnimationOccurrences.size() >
						MAX_DOCUMENT_OCCURRENCES - totalOccurrences)
				{
					outStatus = "KoukuSaydon Stage identity, clock, or animation count is invalid: " +
						stage.strStageId;
					return false;
				}
				totalOccurrences += stage.AnimationOccurrences.size();
				for (const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence :
					stage.AnimationOccurrences)
				{
					const std::uint64_t endMs =
						static_cast<std::uint64_t>(occurrence.iStartOffsetMs) +
						occurrence.iPlayMs;
					if (!Is_StableId(occurrence.strOccurrenceId) ||
						!occurrenceIds.insert(occurrence.strOccurrenceId).second ||
						!Try_ParseGeneratedOrdinal(occurrence.strOccurrenceId,
							occurrencePrefix, pattern.iNextAnimationOrdinal) ||
						CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(occurrence.strProfileId) != pattern.strActorProfileId ||
						(occurrence.strSourceStageId == "RAW" && 0u != occurrence.iSourceActionId) ||
						!Is_StableId(occurrence.strSourceStageId) ||
						!Is_StableId(occurrence.strSourceSlotId) ||
						(occurrence.strSourceStageId != "RAW" && !Is_LowerSha256(occurrence.strReferenceRevision)) ||
						!Is_StableId(occurrence.strRuntimeClip) ||
						occurrence.iStartOffsetMs > MAX_TIME_MS ||
						occurrence.iSourceStartMs > MAX_TIME_MS ||
						0u == occurrence.iPlayMs || occurrence.iPlayMs > MAX_TIME_MS ||
						!std::isfinite(occurrence.fPlayRate) ||
						occurrence.fPlayRate < 0.01f || occurrence.fPlayRate > 16.f ||
						!Is_EndPolicy(occurrence.strEndPolicy) ||
						endMs > stage.iDurationMs)
					{
						outStatus = "KoukuSaydon animation occurrence identity or timing is invalid: " +
							occurrence.strOccurrenceId;
						return false;
					}
				}
			}
		}

		outStatus = "Validated KoukuSaydon composition structure.";
		return true;
	}

	std::string Preserve_Json(const DATA_JSON_VALUE& value)
	{
		if (value.Is_Null()) return "null";
		if (value.Is_Boolean()) return value.Get_Boolean() ? "true" : "false";
		if (value.Is_String()) return "\"" + CDataJson::Escape(value.Get_String()) + "\"";
		if (value.Is_Number())
		{
			std::ostringstream out;
			out << std::setprecision(std::numeric_limits<double>::max_digits10) << value.Get_Number();
			std::string number = out.str();
			if (value.Was_FloatingPointToken() && number.find_first_of(".eE") == std::string::npos)
				number += ".0";
			return number;
		}
		std::string result = value.Is_Array() ? "[" : "{";
		bool first = true;
		if (value.Is_Array())
		{
			for (const auto& item : value.Get_Array())
			{
				if (!first) result += ",";
				first = false;
				result += Preserve_Json(item);
			}
		}
		else
		{
			for (const auto& [key, item] : value.Get_Object())
			{
				if (!first) result += ",";
				first = false;
				result += "\"" + CDataJson::Escape(key) + "\":" + Preserve_Json(item);
			}
		}
		return result + (value.Is_Array() ? "]" : "}");
	}

	void Remove_Temporary(const std::filesystem::path& path)
	{
		std::error_code ignored;
		std::filesystem::remove(path, ignored);
	}

	class COMPOSITION_WRITER_LOCK final
	{
	public:
		~COMPOSITION_WRITER_LOCK()
		{
			if (INVALID_HANDLE_VALUE != m_hFile)
				CloseHandle(m_hFile);
		}

		bool_t Acquire(
			const std::filesystem::path& destination,
			std::string& outStatus)
		{
			m_Path = destination;
			m_Path += L".writer.lock";
			const auto createLock = [this]()
			{
				return CreateFileW(
					m_Path.c_str(), GENERIC_READ | GENERIC_WRITE,
					0u, nullptr, CREATE_NEW,
					FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
					nullptr);
			};
			m_hFile = createLock();
			const DWORD firstError = INVALID_HANDLE_VALUE == m_hFile ?
				GetLastError() : ERROR_SUCCESS;
			if (INVALID_HANDLE_VALUE == m_hFile &&
				(firstError == ERROR_FILE_EXISTS ||
				 firstError == ERROR_ALREADY_EXISTS) &&
				DeleteFileW(m_Path.c_str()))
			{
				// A live writer denies delete sharing. Successful deletion therefore
				// recovers only an orphan left by an older, non-delete-on-close build.
				m_hFile = createLock();
			}
			if (INVALID_HANDLE_VALUE == m_hFile)
			{
				outStatus =
					"KoukuSaydon composition writer lock is already held or unavailable.";
				m_Path.clear();
				return false;
			}
			return true;
		}

	private:
		std::filesystem::path m_Path;
		HANDLE m_hFile = INVALID_HANDLE_VALUE;
	};
}

Client::CKoukuSaydonCompositionDocument::CKoukuSaydonCompositionDocument(
	std::filesystem::path path)
	: m_Path(std::move(path))
{
}

std::filesystem::path Client::CKoukuSaydonCompositionDocument::Resolve_Path()
{
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"KoukuSaydon/Gate1/KoukuSaydonComposition.json"));
}

bool_t Client::CKoukuSaydonCompositionDocument::Is_KnownProfile(
	const std::string_view profileId)
{
	return nullptr != Find_Profile(profileId);
}

std::string_view Client::CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(
	const std::string_view sourceProfileId)
{
	const auto* const profile = Find_Profile(sourceProfileId);
	if (nullptr == profile)
		return {};
	return sourceProfileId == "MN_RPCT_07" ? "MN_RPCT_05" : profile->pProfileId;
}

bool_t Client::CKoukuSaydonCompositionDocument::Parse_Text(
	const std::string_view text,
	KOUKU_SAYDON_COMPOSITION_DOCUMENT& outDocument,
	std::string& outStatus)
{
	if (text.empty() || text.size() > MAX_COMPOSITION_BYTES)
	{
		outStatus = "KoukuSaydon composition text is empty or exceeds its bounded size.";
		return false;
	}

	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError) ||
		!Has_ExactProperties(root,
			{ "schema", "formatVersion", "revision", "compositionId",
			  "encounterId", "bossArchetypeId", "bossPlacementId", "areaId",
			  "fixedTickHz", "nextPatternOrdinal", "playAllPatternIds", "patterns" }))
	{
		outStatus = "KoukuSaydon composition JSON is malformed or has unexpected root properties: " +
			parseError;
		return false;
	}

	const DATA_JSON_VALUE* const schema = Required(root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* const version = Required(root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* const revision = Required(root, "revision", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* const compositionId = Required(root, "compositionId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* const encounterId = Required(root, "encounterId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* const archetypeId = Required(root, "bossArchetypeId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* const placementId = Required(root, "bossPlacementId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* const areaId = Required(root, "areaId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* const fixedTickHz = Required(root, "fixedTickHz", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* const nextPatternOrdinal = Required(root, "nextPatternOrdinal", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* const playAllPatternIds = Required(root, "playAllPatternIds", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* const patterns = Required(root, "patterns", DATA_JSON_TYPE::ARRAY);

	KOUKU_SAYDON_COMPOSITION_DOCUMENT staged;
	if (nullptr == schema || schema->Get_String() != COMPOSITION_SCHEMA ||
		nullptr == version ||
		!Try_ParseUnsigned(*version, FORMAT_VERSION, staged.iFormatVersion) ||
		(staged.iFormatVersion != 1u && staged.iFormatVersion != FORMAT_VERSION) ||
		nullptr == revision || !Try_ParseUnsigned(*revision, MAX_REVISION, staged.iRevision) ||
		0u == staged.iRevision ||
		nullptr == compositionId || nullptr == encounterId || nullptr == archetypeId ||
		nullptr == placementId || nullptr == areaId ||
		nullptr == fixedTickHz ||
		!Try_ParseUnsigned(*fixedTickHz, FIXED_TICK_HZ, staged.iFixedTickHz) ||
		staged.iFixedTickHz != FIXED_TICK_HZ ||
		nullptr == nextPatternOrdinal ||
		!Try_ParseUnsigned(*nextPatternOrdinal, MAX_NEXT_ORDINAL,
			staged.iNextPatternOrdinal) || 0u == staged.iNextPatternOrdinal ||
		nullptr == playAllPatternIds || nullptr == patterns ||
		patterns->Get_Array().size() > MAX_PATTERNS ||
		playAllPatternIds->Get_Array().size() > patterns->Get_Array().size())
	{
		outStatus = "KoukuSaydon composition root value or type is invalid.";
		return false;
	}
	const bool_t legacyFormat = 1u == staged.iFormatVersion;
	staged.iFormatVersion = FORMAT_VERSION;
	staged.strCompositionId = compositionId->Get_String();
	staged.strEncounterId = encounterId->Get_String();
	staged.strBossArchetypeId = archetypeId->Get_String();
	staged.strBossPlacementId = placementId->Get_String();
	staged.strAreaId = areaId->Get_String();

	for (const DATA_JSON_VALUE& value : playAllPatternIds->Get_Array())
	{
		if (!value.Is_String())
		{
			outStatus = "KoukuSaydon playAllPatternIds must contain only strings.";
			return false;
		}
		staged.PlayAllPatternIds.push_back(value.Get_String());
	}

	staged.Patterns.reserve(patterns->Get_Array().size());
	std::size_t parsedOccurrences = 0u;
	const auto validationHeader = staged;
	std::unordered_set<std::string> parsedPatternIds, parsedActionIds, parsedOccurrenceIds;

	for (const DATA_JSON_VALUE& patternValue : patterns->Get_Array())
	{
		const auto previousOccurrences = parsedOccurrences;
		KOUKU_SAYDON_COMPOSITION_PATTERN stagedPattern;
		const auto parsePattern = [&]() -> bool_t
		{
		const bool_t validProperties = legacyFormat ?
			Has_ExactProperties(patternValue,
				{ "patternId", "displayName", "authoringStatus", "category",
				  "nextStageOrdinal", "nextAnimationOrdinal", "stages" }) :
			Has_ExactProperties(patternValue,
				{ "patternId", "actorProfileId", "displayName", "authoringStatus", "category",
				  "nextStageOrdinal", "nextAnimationOrdinal", "stages" });
		if (!validProperties)
		{
			outStatus = "KoukuSaydon Pattern has unexpected properties.";
			return false;
		}
		const DATA_JSON_VALUE* const patternId = Required(patternValue, "patternId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* const actorProfileId = Required(patternValue, "actorProfileId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* const displayName = Required(patternValue, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* const authoringStatus = Required(patternValue, "authoringStatus", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* const category = Required(patternValue, "category", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* const nextStageOrdinal = Required(patternValue, "nextStageOrdinal", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* const nextAnimationOrdinal = Required(patternValue, "nextAnimationOrdinal", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* const stages = Required(patternValue, "stages", DATA_JSON_TYPE::ARRAY);
		if (nullptr == patternId || (!legacyFormat && nullptr == actorProfileId) || nullptr == displayName ||
			nullptr == authoringStatus || nullptr == category ||
			nullptr == nextStageOrdinal ||
			!Try_ParseUnsigned(*nextStageOrdinal, MAX_NEXT_ORDINAL,
				stagedPattern.iNextStageOrdinal) || 0u == stagedPattern.iNextStageOrdinal ||
			nullptr == nextAnimationOrdinal ||
			!Try_ParseUnsigned(*nextAnimationOrdinal, MAX_NEXT_ORDINAL,
				stagedPattern.iNextAnimationOrdinal) ||
			0u == stagedPattern.iNextAnimationOrdinal ||
			nullptr == stages || stages->Get_Array().size() > MAX_STAGES_PER_PATTERN)
		{
			outStatus = "KoukuSaydon Pattern value or type is invalid.";
			return false;
		}
		stagedPattern.strPatternId = patternId->Get_String();
		if (!legacyFormat)
			stagedPattern.strActorProfileId = actorProfileId->Get_String();
		stagedPattern.strDisplayName = displayName->Get_String();
		stagedPattern.strAuthoringStatus = authoringStatus->Get_String();
		stagedPattern.strCategory = category->Get_String();
		stagedPattern.Stages.reserve(stages->Get_Array().size());

		for (const DATA_JSON_VALUE& stageValue : stages->Get_Array())
		{
			if (!Has_ExactProperties(stageValue,
					{ "stageId", "actionId", "stageKind", "durationMs",
					  "animationOccurrences" }))
			{
				outStatus = "KoukuSaydon Stage has unexpected properties.";
				return false;
			}
			const DATA_JSON_VALUE* const stageId = Required(stageValue, "stageId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* const actionId = Required(stageValue, "actionId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* const stageKind = Required(stageValue, "stageKind", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* const durationMs = Required(stageValue, "durationMs", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* const occurrences = Required(stageValue, "animationOccurrences", DATA_JSON_TYPE::ARRAY);
			KOUKU_SAYDON_COMPOSITION_STAGE stagedStage;
			if (nullptr == stageId || nullptr == actionId || nullptr == stageKind ||
				nullptr == durationMs ||
				!Try_ParseUnsigned(*durationMs, MAX_TIME_MS, stagedStage.iDurationMs) ||
				0u == stagedStage.iDurationMs || nullptr == occurrences ||
				occurrences->Get_Array().size() > MAX_OCCURRENCES_PER_STAGE ||
				occurrences->Get_Array().size() >
					MAX_DOCUMENT_OCCURRENCES - parsedOccurrences)
			{
				outStatus = "KoukuSaydon Stage value or type is invalid.";
				return false;
			}
			stagedStage.strStageId = stageId->Get_String();
			stagedStage.strActionId = actionId->Get_String();
			stagedStage.strStageKind = stageKind->Get_String();
			parsedOccurrences += occurrences->Get_Array().size();
			stagedStage.AnimationOccurrences.reserve(occurrences->Get_Array().size());

			for (const DATA_JSON_VALUE& occurrenceValue : occurrences->Get_Array())
			{
				if (!Has_ExactProperties(occurrenceValue,
						{ "occurrenceId", "profileId", "sourceActionId",
						  "sourceStageId", "sourceSlotId", "referenceRevision",
						  "runtimeClip", "startOffsetMs", "sourceStartMs",
						  "playMs", "playRate", "endPolicy" }))
				{
					outStatus = "KoukuSaydon animation occurrence has unexpected properties.";
					return false;
				}
				const DATA_JSON_VALUE* const occurrenceId = Required(occurrenceValue, "occurrenceId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const profileId = Required(occurrenceValue, "profileId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const sourceActionId = Required(occurrenceValue, "sourceActionId", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* const sourceStageId = Required(occurrenceValue, "sourceStageId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const sourceSlotId = Required(occurrenceValue, "sourceSlotId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const referenceRevision = Required(occurrenceValue, "referenceRevision", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const runtimeClip = Required(occurrenceValue, "runtimeClip", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const startOffsetMs = Required(occurrenceValue, "startOffsetMs", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* const sourceStartMs = Required(occurrenceValue, "sourceStartMs", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* const playMs = Required(occurrenceValue, "playMs", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* const playRate = Required(occurrenceValue, "playRate", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* const endPolicy = Required(occurrenceValue, "endPolicy", DATA_JSON_TYPE::STRING);
				KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE stagedOccurrence;
				if (nullptr == occurrenceId || nullptr == profileId ||
					nullptr == sourceActionId ||
					!Try_ParseUnsigned(*sourceActionId,
						(std::numeric_limits<std::uint32_t>::max)(),
						stagedOccurrence.iSourceActionId) ||
					nullptr == sourceStageId || nullptr == sourceSlotId ||
					nullptr == referenceRevision || nullptr == runtimeClip ||
					nullptr == startOffsetMs ||
					!Try_ParseUnsigned(*startOffsetMs, MAX_TIME_MS,
						stagedOccurrence.iStartOffsetMs) ||
					nullptr == sourceStartMs ||
					!Try_ParseUnsigned(*sourceStartMs, MAX_TIME_MS,
						stagedOccurrence.iSourceStartMs) ||
					nullptr == playMs ||
					!Try_ParseUnsigned(*playMs, MAX_TIME_MS,
						stagedOccurrence.iPlayMs) || 0u == stagedOccurrence.iPlayMs ||
					nullptr == playRate ||
					!Try_ParsePlayRate(*playRate, stagedOccurrence.fPlayRate) ||
					nullptr == endPolicy)
				{
					outStatus = "KoukuSaydon animation occurrence value or type is invalid.";
					return false;
				}
				stagedOccurrence.strOccurrenceId = occurrenceId->Get_String();
				stagedOccurrence.strProfileId = profileId->Get_String();
				stagedOccurrence.strSourceStageId = sourceStageId->Get_String();
				stagedOccurrence.strSourceSlotId = sourceSlotId->Get_String();
				stagedOccurrence.strReferenceRevision = referenceRevision->Get_String();
				stagedOccurrence.strRuntimeClip = runtimeClip->Get_String();
				stagedOccurrence.strEndPolicy = endPolicy->Get_String();
				stagedStage.AnimationOccurrences.push_back(std::move(stagedOccurrence));
			}
			stagedPattern.Stages.push_back(std::move(stagedStage));
		}
		if (legacyFormat)
		{
			for (const auto& stage : stagedPattern.Stages)
				for (const auto& row : stage.AnimationOccurrences)
				{
					const auto actor = Resolve_ActorProfileId(row.strProfileId);
					if (actor.empty() || (!stagedPattern.strActorProfileId.empty() &&
						stagedPattern.strActorProfileId != actor))
					{
						outStatus = "Legacy KoukuSaydon Pattern has unknown or mixed actor profiles.";
						return false;
					}
					stagedPattern.strActorProfileId = actor;
				}
			if (stagedPattern.strActorProfileId.empty())
				stagedPattern.strActorProfileId = "MN_RPCZ_00";
		}
		KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = validationHeader;
		candidate.Patterns.push_back(stagedPattern);
		if (!Validate_Shape(candidate, outStatus)) return false;
		if (parsedPatternIds.contains(stagedPattern.strPatternId))
		{ outStatus = "Duplicate pattern ID."; return false; }
		for (const auto& stage : stagedPattern.Stages)
		{
			if (parsedActionIds.contains(stage.strActionId))
			{ outStatus = "Duplicate action ID: " + stage.strActionId; return false; }
			for (const auto& row : stage.AnimationOccurrences)
				if (parsedOccurrenceIds.contains(row.strOccurrenceId))
				{ outStatus = "Duplicate animation ID: " + row.strOccurrenceId; return false; }
		}
		return true;
		};
		if (!parsePattern())
		{
			parsedOccurrences = previousOccurrences;
			stagedPattern = {};
			stagedPattern.strLoadError = outStatus;
			stagedPattern.strPreservedJson = Preserve_Json(patternValue);
			const auto* id = Required(patternValue, "patternId", DATA_JSON_TYPE::STRING);
			const auto* name = Required(patternValue, "displayName", DATA_JSON_TYPE::STRING);
			stagedPattern.strPatternId = nullptr != id ? id->Get_String() : "invalid";
			const auto used = [&](const std::string& token) {
				return std::any_of(staged.Patterns.begin(), staged.Patterns.end(),
					[&](const auto& p) { return p.strPatternId == token; });
			};
			if (!Is_StableId(stagedPattern.strPatternId) || used(stagedPattern.strPatternId))
			{
				// Diagnostic identity only; Serialize retains the original JSON identity.
				std::uint64_t hash = 14695981039346656037ull;
				for (unsigned char c : stagedPattern.strPreservedJson) hash = (hash ^ c) * 1099511628211ull;
				stagedPattern.strPatternId = "invalid." + std::to_string(hash);
				while (used(stagedPattern.strPatternId)) stagedPattern.strPatternId += ".duplicate";
			}
			stagedPattern.strDisplayName = nullptr != name && !name->Get_String().empty() ?
				name->Get_String() : stagedPattern.strPatternId;
			stagedPattern.strAuthoringStatus = "DRAFT";
			stagedPattern.strCategory = "MECHANIC";
		}
		parsedPatternIds.insert(stagedPattern.strPatternId);
		for (const auto& stage : stagedPattern.Stages)
		{
			parsedActionIds.insert(stage.strActionId);
			for (const auto& row : stage.AnimationOccurrences) parsedOccurrenceIds.insert(row.strOccurrenceId);
		}
		staged.Patterns.push_back(std::move(stagedPattern));
	}

	if (!Validate_Shape(staged, outStatus))
		return false;
	outDocument = std::move(staged);
	outStatus = "Parsed KoukuSaydon composition document.";
	return true;
}

bool_t Client::CKoukuSaydonCompositionDocument::Load_ImmutableActionReferences(
	KOUKU_SAYDON_ACTION_REFERENCE_SET& outReferences,
	std::string& outStatus)
{
	KOUKU_SAYDON_ACTION_REFERENCE_SET staged;
	std::string errors;
	for (std::size_t index = 0u; index < ACTION_PROFILES.size(); ++index)
	{
		const auto& profile = ACTION_PROFILES[index];
		std::string bytes, status;
		auto& reference = staged.Documents[index];
		if (!Read_Text(CKoukuSaydonAnimationActionDocument::Resolve_ReferencePath(profile.pProfileId),
			MAX_REFERENCE_BYTES, bytes, status, "KoukuSaydon action reference") ||
			!CKoukuSaydonAnimationActionDocument::Parse_ReferenceText(bytes, reference, status, true) ||
			reference.strProfileId != profile.pProfileId || reference.strModelAssetId != profile.pModelAssetId)
		{
			reference = {};
			errors += std::string(profile.pProfileId) + ": " + status + "\n";
			continue;
		}
		staged.SourceBytes[index] = std::move(bytes);
	}
	// Resource failures never revoke composition editing or other profiles.
	outReferences = std::move(staged);
	outStatus = errors.empty() ? "Loaded KoukuSaydon resources." : errors;
	return true;
}

bool_t Client::CKoukuSaydonCompositionDocument::Validate(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
	const KOUKU_SAYDON_ACTION_REFERENCE_SET& references,
	std::string& outStatus)
{
	if (!Validate_Shape(document, outStatus))
		return false;

	(void)references;
	outStatus = "Validated KoukuSaydon composition editing bounds.";
	return true;
}

std::string Client::CKoukuSaydonCompositionDocument::Serialize(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document)
{
	std::ostringstream output;
	output << std::setprecision(std::numeric_limits<f32_t>::max_digits10);
	output << "{\n"
		<< "  \"schema\": \"" << COMPOSITION_SCHEMA << "\",\n"
		<< "  \"formatVersion\": " << document.iFormatVersion << ",\n"
		<< "  \"revision\": " << document.iRevision << ",\n"
		<< "  \"compositionId\": \"" << CDataJson::Escape(document.strCompositionId) << "\",\n"
		<< "  \"encounterId\": \"" << CDataJson::Escape(document.strEncounterId) << "\",\n"
		<< "  \"bossArchetypeId\": \"" << CDataJson::Escape(document.strBossArchetypeId) << "\",\n"
		<< "  \"bossPlacementId\": \"" << CDataJson::Escape(document.strBossPlacementId) << "\",\n"
		<< "  \"areaId\": \"" << CDataJson::Escape(document.strAreaId) << "\",\n"
		<< "  \"fixedTickHz\": " << document.iFixedTickHz << ",\n"
		<< "  \"nextPatternOrdinal\": " << document.iNextPatternOrdinal << ",\n"
		<< "  \"playAllPatternIds\": [";
	for (std::size_t index = 0u; index < document.PlayAllPatternIds.size(); ++index)
	{
		output << (0u == index ? "" : ", ") << "\""
			<< CDataJson::Escape(document.PlayAllPatternIds[index]) << "\"";
	}
	output << "],\n  \"patterns\": [\n";
	for (std::size_t patternIndex = 0u; patternIndex < document.Patterns.size(); ++patternIndex)
	{
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern = document.Patterns[patternIndex];
		if (!pattern.strLoadError.empty())
		{
			output << "    " << pattern.strPreservedJson
				<< (patternIndex + 1u < document.Patterns.size() ? "," : "") << "\n";
			continue;
		}
		output << "    {\n"
			<< "      \"patternId\": \"" << CDataJson::Escape(pattern.strPatternId) << "\",\n"
			<< "      \"actorProfileId\": \"" << CDataJson::Escape(pattern.strActorProfileId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(pattern.strDisplayName) << "\",\n"
			<< "      \"authoringStatus\": \"" << CDataJson::Escape(pattern.strAuthoringStatus) << "\",\n"
			<< "      \"category\": \"" << CDataJson::Escape(pattern.strCategory) << "\",\n"
			<< "      \"nextStageOrdinal\": " << pattern.iNextStageOrdinal << ",\n"
			<< "      \"nextAnimationOrdinal\": " << pattern.iNextAnimationOrdinal << ",\n"
			<< "      \"stages\": [\n";
		for (std::size_t stageIndex = 0u; stageIndex < pattern.Stages.size(); ++stageIndex)
		{
			const KOUKU_SAYDON_COMPOSITION_STAGE& stage = pattern.Stages[stageIndex];
			output << "        {\n"
				<< "          \"stageId\": \"" << CDataJson::Escape(stage.strStageId) << "\",\n"
				<< "          \"actionId\": \"" << CDataJson::Escape(stage.strActionId) << "\",\n"
				<< "          \"stageKind\": \"" << CDataJson::Escape(stage.strStageKind) << "\",\n"
				<< "          \"durationMs\": " << stage.iDurationMs << ",\n"
				<< "          \"animationOccurrences\": [\n";
			for (std::size_t occurrenceIndex = 0u;
				occurrenceIndex < stage.AnimationOccurrences.size(); ++occurrenceIndex)
			{
				const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence =
					stage.AnimationOccurrences[occurrenceIndex];
				output << "            {\n"
					<< "              \"occurrenceId\": \"" << CDataJson::Escape(occurrence.strOccurrenceId) << "\",\n"
					<< "              \"profileId\": \"" << CDataJson::Escape(occurrence.strProfileId) << "\",\n"
					<< "              \"sourceActionId\": " << occurrence.iSourceActionId << ",\n"
					<< "              \"sourceStageId\": \"" << CDataJson::Escape(occurrence.strSourceStageId) << "\",\n"
					<< "              \"sourceSlotId\": \"" << CDataJson::Escape(occurrence.strSourceSlotId) << "\",\n"
					<< "              \"referenceRevision\": \"" << CDataJson::Escape(occurrence.strReferenceRevision) << "\",\n"
					<< "              \"runtimeClip\": \"" << CDataJson::Escape(occurrence.strRuntimeClip) << "\",\n"
					<< "              \"startOffsetMs\": " << occurrence.iStartOffsetMs << ",\n"
					<< "              \"sourceStartMs\": " << occurrence.iSourceStartMs << ",\n"
					<< "              \"playMs\": " << occurrence.iPlayMs << ",\n"
					<< "              \"playRate\": " << occurrence.fPlayRate << ",\n"
					<< "              \"endPolicy\": \"" << CDataJson::Escape(occurrence.strEndPolicy) << "\"\n"
					<< "            }" << (occurrenceIndex + 1u < stage.AnimationOccurrences.size() ? "," : "") << "\n";
			}
			output << "          ]\n        }"
				<< (stageIndex + 1u < pattern.Stages.size() ? "," : "") << "\n";
		}
		output << "      ]\n    }"
			<< (patternIndex + 1u < document.Patterns.size() ? "," : "") << "\n";
	}
	output << "  ]\n}\n";
	return output.str();
}

bool_t Client::CKoukuSaydonCompositionDocument::Reload(std::string& outStatus)
{
	const std::filesystem::path path = m_Path.empty() ? Resolve_Path() : m_Path;
	return Reload_FromPath(path, outStatus);
}

bool_t Client::CKoukuSaydonCompositionDocument::Reload_FromPath(
	const std::filesystem::path& path,
	std::string& outStatus)
{
	std::string bytes;
	KOUKU_SAYDON_COMPOSITION_DOCUMENT stagedDocument;
	KOUKU_SAYDON_ACTION_REFERENCE_SET stagedReferences;
	std::string status;
	if (!Is_ExpectedCompositionPath(path) ||
		!Read_Text(path, MAX_COMPOSITION_BYTES, bytes, status,
			"KoukuSaydon composition") ||
		!Parse_Text(bytes, stagedDocument, status) ||
		!Validate(stagedDocument, stagedReferences, status))
	{
		m_bFresh = false;
		m_strStatus = m_bHasLastGood ?
			"Reload rejected; last-good KoukuSaydon composition preserved: " + status :
			"KoukuSaydon composition is not admitted yet: " + status;
		outStatus = m_strStatus;
		return false;
	}

	std::string referenceStatus;
	Load_ImmutableActionReferences(stagedReferences, referenceStatus);
	m_Path = path;
	m_LastGood = std::move(stagedDocument);
	m_References = std::move(stagedReferences);
	m_strBaselineSourceBytes = std::move(bytes);
	m_bHasLastGood = true;
	m_bFresh = true;
	++m_iGeneration;
	m_strStatus = "Loaded KoukuSaydon composition revision " +
		std::to_string(m_LastGood.iRevision) + ". " + referenceStatus;
	outStatus = m_strStatus;
	return true;
}

bool_t Client::CKoukuSaydonCompositionDocument::Save_Atomic(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT& candidate,
	std::string& outStatus)
{
	if (!m_bHasLastGood || !m_bFresh || m_Path.empty())
	{
		outStatus = "KoukuSaydon composition Save requires a fresh last-good baseline.";
		m_strStatus = outStatus;
		return false;
	}
	if (candidate.iRevision != m_LastGood.iRevision ||
		candidate.iRevision >= MAX_REVISION)
	{
		outStatus = "KoukuSaydon composition candidate revision is stale or exhausted.";
		m_strStatus = outStatus;
		return false;
	}
	std::string status;
	if (!Validate(candidate, m_References, status))
	{
		outStatus = "KoukuSaydon composition Save validation rejected the candidate: " + status;
		m_strStatus = outStatus;
		return false;
	}

	COMPOSITION_WRITER_LOCK writerLock;
	if (!writerLock.Acquire(m_Path, status))
	{
		outStatus = status;
		m_strStatus = outStatus;
		return false;
	}

	std::string currentBytes;
	if (!Read_Text(m_Path, MAX_COMPOSITION_BYTES, currentBytes, status,
			"Current KoukuSaydon composition") ||
		currentBytes != m_strBaselineSourceBytes)
	{
		outStatus = "KoukuSaydon composition changed before Save; reload required. " + status;
		m_strStatus = outStatus;
		m_bFresh = false;
		return false;
	}

	KOUKU_SAYDON_COMPOSITION_DOCUMENT staged = candidate;
	++staged.iRevision;
	const std::string serialized = Serialize(staged);
	std::filesystem::path temporary = m_Path;
	temporary += L".tmp." + std::to_wstring(GetCurrentProcessId()) + L"." +
		std::to_wstring(GetCurrentThreadId()) + L"." +
		std::to_wstring(GetTickCount64());
	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, temporary.c_str(), L"wb") || nullptr == file)
	{
		outStatus = "Could not open the temporary KoukuSaydon composition.";
		m_strStatus = outStatus;
		return false;
	}
	const bool_t wrote = serialized.size() ==
		fwrite(serialized.data(), 1u, serialized.size(), file);
	const bool_t flushed = 0 == fflush(file) && 0 == _commit(_fileno(file));
	const bool_t closed = 0 == fclose(file);
	if (!wrote || !flushed || !closed)
	{
		Remove_Temporary(temporary);
		outStatus = "Could not durably write the temporary KoukuSaydon composition.";
		m_strStatus = outStatus;
		return false;
	}

	std::string verificationBytes;
	KOUKU_SAYDON_COMPOSITION_DOCUMENT reparsed;
	if (!Read_Text(temporary, MAX_COMPOSITION_BYTES, verificationBytes, status,
			"Temporary KoukuSaydon composition") ||
		verificationBytes != serialized ||
		!Parse_Text(verificationBytes, reparsed, status) ||
		!Validate(reparsed, m_References, status) || Serialize(reparsed) != serialized)
	{
		Remove_Temporary(temporary);
		outStatus = "KoukuSaydon composition temp verification failed: " + status;
		m_strStatus = outStatus;
		return false;
	}

	std::string finalCurrentBytes;
	if (!Read_Text(m_Path, MAX_COMPOSITION_BYTES, finalCurrentBytes, status,
			"Current KoukuSaydon composition") ||
		finalCurrentBytes != currentBytes)
	{
		Remove_Temporary(temporary);
		outStatus = "KoukuSaydon composition CAS changed during Save; destination preserved. " + status;
		m_strStatus = outStatus;
		m_bFresh = false;
		return false;
	}

	if (!MoveFileExW(temporary.c_str(), m_Path.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		Remove_Temporary(temporary);
		outStatus = "Could not atomically replace the KoukuSaydon composition.";
		m_strStatus = outStatus;
		return false;
	}

	std::string reopenedBytes;
	KOUKU_SAYDON_COMPOSITION_DOCUMENT reopened;
	if (!Read_Text(m_Path, MAX_COMPOSITION_BYTES, reopenedBytes, status,
			"Committed KoukuSaydon composition") ||
		reopenedBytes != serialized ||
		!Parse_Text(reopenedBytes, reopened, status) ||
		!Validate(reopened, m_References, status) || Serialize(reopened) != serialized)
	{
		m_bFresh = false;
		outStatus =
			"KoukuSaydon composition COMMIT_SUCCEEDED_REOPEN_FAILED; source was not rewritten: " +
			status;
		m_strStatus = outStatus;
		return false;
	}

	m_LastGood = std::move(reopened);
	m_strBaselineSourceBytes = std::move(reopenedBytes);
	m_bFresh = true;
	++m_iGeneration;
	m_strStatus = "Saved and reopened KoukuSaydon composition revision " +
		std::to_string(m_LastGood.iRevision) + ".";
	outStatus = m_strStatus;
	return true;
}
