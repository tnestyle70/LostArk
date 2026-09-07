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
#include <unordered_map>
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
	constexpr std::string_view GENERATED_LOGIC_PREFIX = "kakulsaydon.g1.logic.";
	constexpr std::size_t MAX_LOGICS = 4096u;
	constexpr std::size_t MAX_LOGIC_OCCURRENCES_PER_PATTERN = 1024u;
	constexpr std::string_view GENERATED_SUMMON_PREFIX = "kakulsaydon.g1.summon.";
	constexpr std::size_t MAX_SUMMONS = 4096u;
	constexpr std::size_t MAX_SUMMON_OCCURRENCES_PER_PATTERN = 1024u;
	constexpr std::string_view GENERATED_WORLD_PREFIX = "kakulsaydon.g1.world.";
	constexpr std::size_t MAX_WORLDS = 4096u;
	constexpr std::size_t MAX_WORLD_OCCURRENCES_PER_PATTERN = 16u;
	constexpr std::string_view GENERATED_SCENE_PROFILE_PREFIX = "kakulsaydon.g1.sceneprofile.";
	constexpr std::size_t MAX_SCENE_PROFILES = 4096u;
	constexpr std::size_t MAX_SCENE_PROFILE_OCCURRENCES_PER_PATTERN = 16u;
	constexpr std::uint32_t MAX_MADNESS_MAXIMUM = 1000000u;
	constexpr double MIN_WORLD_PLAYBACK_SPEED = 0.05;
	constexpr double MAX_WORLD_PLAYBACK_SPEED = 16.0;
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

	/* Physical bodies that a KoukuSaydon arena boss (Data/Actors/BossCatalog.json
	   BOSS_KAKULSAYDON_* rows) presents on. Only a pattern authored on one of
	   them can become PRODUCT, because the Server audition plays it on the
	   live arena boss with that body. The projector re-derives this join from
	   the catalog; this list is the Workbench pre-check. */
	bool_t Is_ArenaBossBodyProfile(const std::string_view actorProfileId)
	{
		return actorProfileId == "MN_RPCZ_00" ||
			actorProfileId == "MN_RPCT_05" ||
			actorProfileId == "MN_RPCT_06";
	}

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

	/* Optional properties may be absent in an older file but never unknown, so
	   a document written before the Logic catalog existed still opens. */
	bool_t Has_Properties(
		const DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> required,
		const std::initializer_list<std::string_view> optional)
	{
		if (!object.Is_Object())
			return false;
		for (const std::string_view name : required)
		{
			if (nullptr == object.Find(name))
				return false;
		}
		for (const auto& [key, value] : object.Get_Object())
		{
			(void)value;
			const std::string_view name = key;
			const bool_t known =
				std::find(required.begin(), required.end(), name) != required.end() ||
				std::find(optional.begin(), optional.end(), name) != optional.end();
			if (!known)
				return false;
		}
		return true;
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

	bool_t Is_LogicType(const std::string_view value)
	{
		return std::any_of(KOUKU_SAYDON_LOGIC_TYPES.begin(), KOUKU_SAYDON_LOGIC_TYPES.end(),
			[value](const char_t* const type)
			{
				return value == type;
			});
	}

	bool_t Is_JudgementKind(const std::string_view value)
	{
		return std::any_of(KOUKU_SAYDON_JUDGEMENT_KINDS.begin(), KOUKU_SAYDON_JUDGEMENT_KINDS.end(),
			[value](const char_t* const kind) { return value == kind; });
	}

	bool_t Is_OutcomeKind(const std::string_view value)
	{
		return std::any_of(KOUKU_SAYDON_OUTCOME_KINDS.begin(), KOUKU_SAYDON_OUTCOME_KINDS.end(),
			[value](const char_t* const kind) { return value == kind; });
	}

	bool_t Is_CardSymbol(const std::string_view value)
	{
		return std::any_of(KOUKU_SAYDON_CARD_SYMBOLS.begin(), KOUKU_SAYDON_CARD_SYMBOLS.end(),
			[value](const char_t* const symbol) { return value == symbol; });
	}

	bool_t Try_ParseFinite(
		const DATA_JSON_VALUE& value,
		const double minimum,
		const double maximum,
		double& outValue)
	{
		if (!value.Is_Number())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number < minimum || number > maximum)
			return false;
		outValue = number;
		return true;
	}

	/* A list of RESULT Logic IDs on one outcome slot; empty entries are dropped
	   so a cleared combo never leaves a hole. */
	bool_t Try_ParseTextList(
		const DATA_JSON_VALUE* const value,
		const std::size_t maximum,
		std::vector<std::string>& outList)
	{
		outList.clear();
		if (nullptr == value)
			return true;
		if (!value->Is_Array() || value->Get_Array().size() > maximum)
			return false;
		for (const DATA_JSON_VALUE& item : value->Get_Array())
		{
			if (!item.Is_String())
				return false;
			if (!item.Get_String().empty())
				outList.push_back(item.Get_String());
		}
		return true;
	}

	/* The typed values a Logic definition may carry, by type and kind. This is
	   the same table the projector enforces so a Save is never refused later. */
	bool_t Validate_LogicDefinitionValues(
		const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic,
		std::string& outStatus)
	{
		const bool_t hasDurationValues = !logic.strJudgementKind.empty() ||
			0u != logic.iSectorCount || !logic.SectorSymbols.empty() || !logic.RegionIds.empty() ||
			0.0 != logic.fCenterX || 0.0 != logic.fCenterZ || 0.0 != logic.fOuterRadiusM ||
			!logic.strWorldSequenceInstanceId.empty() || 0.0 != logic.fHalfAngleDegrees ||
			0.0 != logic.fMaxDistanceM || 0u != logic.iPoseIndex || 0u != logic.iThreshold ||
			0.0 != logic.fShieldArcDegrees || logic.bEndsPatternOnSuccess || logic.fNormalYawOffsetDegrees != 0.0 || logic.strInsideOutcome != "SUCCESS";
		if ((logic.strInsideOutcome != "SUCCESS" && logic.strInsideOutcome != "FAIL") ||
			(logic.strInsideOutcome != "SUCCESS" && (logic.strLogicType != "DURATION" || logic.strJudgementKind != "AREA_OVERLAP")))
		{ outStatus = "Only AREA_OVERLAP takes an inside outcome of SUCCESS or FAIL: " + logic.strLogicId; return false; }
		const bool_t hasResultValues = !logic.strOutcomeKind.empty() ||
			0u != logic.iPercent || 0u != logic.iDurationMs || !logic.strFollowupPatternId.empty();
		const bool_t hasTriggerValues = !logic.strTriggerKind.empty() || !logic.strHudMode.empty() ||
			!logic.strClonePatternId.empty() || !logic.ClockHours.empty() || logic.fFaceCenterYawOffsetDegrees != 0.0 ||
			std::any_of(logic.TeleportPosition.begin(), logic.TeleportPosition.end(), [](double x) { return x != 0.0; });
		if (!std::isfinite(logic.fNormalYawOffsetDegrees) || std::abs(logic.fNormalYawOffsetDegrees) > 360.0 ||
			!std::isfinite(logic.fFaceCenterYawOffsetDegrees) || std::abs(logic.fFaceCenterYawOffsetDegrees) > 360.0)
		{ outStatus = "Logic yaw offsets must be finite -360..360 degrees."; return false; }
		if (logic.strLogicType != "TRIGGER" && hasTriggerValues)
		{ outStatus = "Non-trigger Logic carries trigger values: " + logic.strLogicId; return false; }
		if ("DURATION" == logic.strLogicType)
		{
			if (hasResultValues)
			{
				outStatus = "A DURATION Logic carries RESULT values: " + logic.strLogicId;
				return false;
			}
			if (logic.strJudgementKind.empty())
			{
				if (hasDurationValues)
				{
					outStatus = "A DURATION Logic carries judgement values without a judgement kind: " + logic.strLogicId;
					return false;
				}
				return true;
			}
			if (!Is_JudgementKind(logic.strJudgementKind))
			{
				outStatus = "Unknown judgement kind on " + logic.strLogicId + ": " + logic.strJudgementKind;
				return false;
			}
			const std::string_view kind = logic.strJudgementKind;
			const bool_t roulette = "ROULETTE_CARD_MATCH" == kind;
			const bool_t gaze = "GAZE_REAL_BOSS" == kind;
			const bool_t pose = "POSE_INPUT" == kind;
			const bool_t stagger = "STAGGER_WINDOW" == kind;
			const bool_t foreignValues =
				(!roulette && (0u != logic.iSectorCount || !logic.SectorSymbols.empty() || !logic.RegionIds.empty() ||
					0.0 != logic.fCenterX || 0.0 != logic.fCenterZ || 0.0 != logic.fOuterRadiusM ||
					!logic.strWorldSequenceInstanceId.empty())) ||
				(!gaze && (0.0 != logic.fHalfAngleDegrees || 0.0 != logic.fMaxDistanceM)) ||
				(!pose && 0u != logic.iPoseIndex) ||
				(!stagger && (0u != logic.iThreshold || 0.0 != logic.fShieldArcDegrees ||
					logic.bEndsPatternOnSuccess || logic.fNormalYawOffsetDegrees != 0.0));
			if (foreignValues)
			{
				outStatus = "Logic carries values of another judgement kind: " + logic.strLogicId;
				return false;
			}
			if (roulette && (logic.iSectorCount != 0u || !logic.SectorSymbols.empty() ||
				!logic.strWorldSequenceInstanceId.empty()) && (logic.iSectorCount < 2u || logic.iSectorCount > 64u ||
				logic.SectorSymbols.size() != logic.iSectorCount ||
				!std::all_of(logic.SectorSymbols.begin(), logic.SectorSymbols.end(),
					[](const std::string& symbol) { return Is_CardSymbol(symbol); }) ||
				logic.fOuterRadiusM < 0.1 || logic.fOuterRadiusM > 1000.0 ||
				std::fabs(logic.fCenterX) > 100000.0 || std::fabs(logic.fCenterZ) > 100000.0 ||
				!Is_StableId(logic.strWorldSequenceInstanceId)))
			{
				outStatus = "Roulette Logic needs 2..64 card sectors, a centre, a radius and its world sequence instance: " + logic.strLogicId;
				return false;
			}
			if (roulette)
			{
				std::unordered_set<std::string> ids;
				if (logic.RegionIds.size() > 8u || !std::all_of(logic.RegionIds.begin(), logic.RegionIds.end(),
					[&](const auto& id) { return Is_StableId(id) && ids.insert(id).second; }))
				{ outStatus = "Roulette region references must be up to eight unique stable IDs: " + logic.strLogicId; return false; }
			}
			if (gaze && (logic.fHalfAngleDegrees < 1.0 || logic.fHalfAngleDegrees > 180.0 ||
				logic.fMaxDistanceM < 0.0 || logic.fMaxDistanceM > 1000.0))
			{
				outStatus = "Gaze Logic needs a half angle of 1..180 degrees and a distance of 0..1000 m: " + logic.strLogicId;
				return false;
			}
			if (pose && logic.iPoseIndex >= KOUKU_SAYDON_DANCE_POSE_COUNT)
			{
				outStatus = "Pose Logic needs a pose index of 0..3: " + logic.strLogicId;
				return false;
			}
			if (stagger && (0u == logic.iThreshold ||
				logic.fShieldArcDegrees < 0.0 || logic.fShieldArcDegrees > 360.0))
			{
				outStatus = "Stagger Logic needs a damage threshold and a shield arc of 0..360 degrees: " + logic.strLogicId;
				return false;
			}
			return true;
		}
		if ("RESULT" == logic.strLogicType)
		{
			if (hasDurationValues)
			{
				outStatus = "A RESULT Logic carries judgement values: " + logic.strLogicId;
				return false;
			}
			if (logic.strOutcomeKind.empty())
			{
				if (hasResultValues)
				{
					outStatus = "A RESULT Logic carries outcome values without an outcome kind: " + logic.strLogicId;
					return false;
				}
				return true;
			}
			if (!Is_OutcomeKind(logic.strOutcomeKind))
			{
				outStatus = "Unknown outcome kind on " + logic.strLogicId + ": " + logic.strOutcomeKind;
				return false;
			}
			const std::string_view kind = logic.strOutcomeKind;
			const bool_t percentKind = "MAX_HP_PERCENT_DAMAGE" == kind ||
				"MADNESS_GAUGE_ADD_PERCENT" == kind;
			const bool_t followup = "FOLLOWUP_PATTERN" == kind;
			if (logic.iPercent > 100u || (percentKind && 0u == logic.iPercent) ||
				(!percentKind && 0u != logic.iPercent))
			{
				outStatus = "A percent outcome needs 1..100 percent and no other outcome takes one: " + logic.strLogicId;
				return false;
			}
			if (("CLOWN_TRANSFORM" != kind && 0u != logic.iDurationMs) ||
				logic.iDurationMs > MAX_TIME_MS)
			{
				outStatus = "Only the clown transform takes a duration: " + logic.strLogicId;
				return false;
			}
			if (followup != !logic.strFollowupPatternId.empty() ||
				(followup && !Is_StableId(logic.strFollowupPatternId)))
			{
				outStatus = "A follow-up outcome names exactly one Pattern: " + logic.strLogicId;
				return false;
			}
			return true;
		}
		if (hasDurationValues || hasResultValues)
		{
			outStatus = "A TRIGGER Logic carries duration/result values: " + logic.strLogicId;
			return false;
		}
		if (logic.strTriggerKind.empty())
		{
			if (hasTriggerValues) { outStatus = "Trigger values need a triggerKind."; return false; }
		}
		else if (logic.strTriggerKind == "ENTER_AREA")
		{
			if (!logic.strHudMode.empty() || !logic.strClonePatternId.empty() || !logic.ClockHours.empty() ||
				logic.fFaceCenterYawOffsetDegrees != 0.0 ||
				std::any_of(logic.TeleportPosition.begin(), logic.TeleportPosition.end(), [](double x) { return x != 0.0; }))
			{ outStatus = "ENTER_AREA geometry belongs to its linked Collider occurrence."; return false; }
		}
		else if (logic.strTriggerKind == "HUD_ENTER")
		{
			const auto& mode = logic.strHudMode;
			if ((mode != "NONE" && mode != "POLYMORPH" && mode != "MARIO" && mode != "DANCE" && mode != "MAZE") ||
				!logic.strClonePatternId.empty() || !logic.ClockHours.empty() || logic.fFaceCenterYawOffsetDegrees != 0.0 ||
				std::any_of(logic.TeleportPosition.begin(), logic.TeleportPosition.end(), [](double x) { return x != 0.0; }))
			{ outStatus = "HUD_ENTER has invalid or foreign values."; return false; }
		}
		else if (logic.strTriggerKind == "REAL_GAZE_TELEPORT")
		{
			std::unordered_set<std::uint32_t> hours;
			if (!logic.strHudMode.empty() || !Is_StableId(logic.strClonePatternId) || logic.ClockHours.size() != 3u ||
				!std::all_of(logic.TeleportPosition.begin(), logic.TeleportPosition.end(), [](double x) { return std::isfinite(x) && std::abs(x) <= 100000.0; }))
			{ outStatus = "Real Saydon teleport needs a position and three clone clock positions."; return false; }
			for (auto hour : logic.ClockHours)
				if (hour < 1u || hour > 12u || hour == 1u || !hours.insert(hour).second)
				{ outStatus = "Clone clock positions must be unique and exclude 1 o'clock."; return false; }
		}
		else { outStatus = "Unknown trigger kind: " + logic.strTriggerKind; return false; }
		return true;
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

	const char* Presentation_KindName(const KOUKU_SAYDON_PRESENTATION_KIND kind)
	{
		switch (kind)
		{
		case KOUKU_SAYDON_PRESENTATION_KIND::EFFECT: return "EFFECT";
		case KOUKU_SAYDON_PRESENTATION_KIND::SOUND: return "SOUND";
		case KOUKU_SAYDON_PRESENTATION_KIND::CAMERA: return "CAMERA";
		case KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER: return "COLLIDER";
		case KOUKU_SAYDON_PRESENTATION_KIND::LIGHT: return "LIGHT";
		default: return "";
		}
	}

	bool_t Read_PresentationText(const DATA_JSON_VALUE& object, const char* key,
		std::string& value, const bool_t required = false)
	{
		const auto* field = object.Find(key);
		if (nullptr == field) return !required;
		if (!field->Is_String()) return false;
		value = field->Get_String();
		return true;
	}

	bool_t Read_PresentationTime(const DATA_JSON_VALUE& object, const char* key,
		std::uint32_t& value)
	{
		const auto* field = object.Find(key);
		return nullptr == field || Try_ParseUnsigned(*field, MAX_TIME_MS, value);
	}

	bool_t Read_PresentationNumber(const DATA_JSON_VALUE& object, const char* key,
		double& value, const double minimum, const double maximum)
	{
		const auto* field = object.Find(key);
		return nullptr == field || Try_ParseFinite(*field, minimum, maximum, value);
	}

	bool_t Read_PresentationVector(const DATA_JSON_VALUE& object, const char* key,
		std::array<double, 3u>& value, const double minimum, const double maximum)
	{
		const auto* field = object.Find(key);
		if (nullptr == field) return true;
		if (!field->Is_Array() || field->Get_Array().size() != 3u) return false;
		for (std::size_t i = 0u; i < 3u; ++i)
			if (!Try_ParseFinite(field->Get_Array()[i], minimum, maximum, value[i])) return false;
		return true;
	}

	bool_t Read_PresentationResource(const DATA_JSON_VALUE& value,
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& row)
	{
		if (!Has_Properties(value, { "resourceId", "displayName", "kind", "assetId" },
			{ "resourceKind", "durationMs", "shape", "halfExtents", "radiusM", "halfAngleDegrees", "colliderKind", "defaultAnchorKind" }))
			return false;
		std::string kind;
		if (!Read_PresentationText(value, "resourceId", row.strResourceId, true) ||
			!Read_PresentationText(value, "displayName", row.strDisplayName, true) ||
			!Read_PresentationText(value, "kind", kind, true) ||
			!Read_PresentationText(value, "assetId", row.strAssetId, true) ||
			!Read_PresentationText(value, "resourceKind", row.strResourceKind) ||
			!Read_PresentationText(value, "defaultAnchorKind", row.strDefaultAnchorKind) ||
			!Read_PresentationText(value, "shape", row.strShape) ||
			!Read_PresentationText(value, "colliderKind", row.strColliderKind) ||
			!Read_PresentationTime(value, "durationMs", row.iDurationMs) ||
			!Read_PresentationVector(value, "halfExtents", row.HalfExtents, 0.001, 10000.0) ||
			!Read_PresentationNumber(value, "radiusM", row.fRadiusM, 0.001, 10000.0) ||
			!Read_PresentationNumber(value, "halfAngleDegrees", row.fHalfAngleDegrees, 0.001, 180.0)) return false;
		for (const auto candidate : { KOUKU_SAYDON_PRESENTATION_KIND::EFFECT,
			KOUKU_SAYDON_PRESENTATION_KIND::SOUND, KOUKU_SAYDON_PRESENTATION_KIND::CAMERA,
			KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER, KOUKU_SAYDON_PRESENTATION_KIND::LIGHT })
			if (kind == Presentation_KindName(candidate)) { row.eKind = candidate; return true; }
		return false;
	}

	bool_t Read_PresentationOccurrence(const DATA_JSON_VALUE& value,
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& row)
	{
		if (!Has_Properties(value, { "occurrenceId", "resourceId", "startMs", "durationMs" },
			{ "positionOffset", "rotationDegrees", "scale", "fadeInMs", "fadeOutMs",
			  "dissolveStart", "dissolveEnd", "volume", "followBoss", "bone",
			  "regionId", "cardSymbol", "cardColor", "anchorKind", "worldId", "logicOccurrenceId", "debugRender", "worldOccurrenceId", "brightnessMultiplier" })) return false;
		const auto* debugRender = value.Find("debugRender");
		if (nullptr != debugRender)
		{
			if (!debugRender->Is_Boolean()) return false;
			row.bDebugRender = debugRender->Get_Boolean();
		}
		const auto* follow = value.Find("followBoss");
		if (nullptr != follow)
		{
			if (!follow->Is_Boolean()) return false;
			row.bFollowBoss = follow->Get_Boolean();
		}
		return Read_PresentationText(value, "occurrenceId", row.strOccurrenceId, true) &&
			Read_PresentationText(value, "resourceId", row.strResourceId, true) &&
			Read_PresentationText(value, "bone", row.strBone) &&
			Read_PresentationText(value, "regionId", row.strRegionId) &&
			Read_PresentationText(value, "cardSymbol", row.strCardSymbol) &&
			Read_PresentationText(value, "cardColor", row.strCardColor) &&
			Read_PresentationText(value, "anchorKind", row.strAnchorKind) &&
			Read_PresentationText(value, "worldId", row.strWorldId) &&
			Read_PresentationText(value, "logicOccurrenceId", row.strLogicOccurrenceId) &&
			Read_PresentationText(value, "worldOccurrenceId", row.strWorldOccurrenceId) &&
			Read_PresentationTime(value, "startMs", row.iStartMs) &&
			Read_PresentationTime(value, "durationMs", row.iDurationMs) &&
			Read_PresentationTime(value, "fadeInMs", row.iFadeInMs) &&
			Read_PresentationTime(value, "fadeOutMs", row.iFadeOutMs) &&
			Read_PresentationVector(value, "positionOffset", row.PositionOffset, -100000.0, 100000.0) &&
			Read_PresentationVector(value, "rotationDegrees", row.RotationDegrees, -36000.0, 36000.0) &&
			Read_PresentationVector(value, "scale", row.Scale, 0.001, 10000.0) &&
			Read_PresentationNumber(value, "dissolveStart", row.fDissolveStart, 0.0, 1.0) &&
			Read_PresentationNumber(value, "dissolveEnd", row.fDissolveEnd, 0.0, 1.0) &&
			Read_PresentationNumber(value, "volume", row.fVolume, 0.0, 1.0) &&
			Read_PresentationNumber(value, "brightnessMultiplier", row.fBrightnessMultiplier, 0.0, 16.0);
	}

	bool_t Valid_PresentationVector(const std::array<double, 3u>& value,
		const double minimum, const double maximum)
	{
		return std::all_of(value.begin(), value.end(), [=](const double number) {
			return std::isfinite(number) && number >= minimum && number <= maximum; });
	}

	void Write_PresentationVector(std::ostringstream& out, const std::array<double, 3u>& value)
	{
		out << '[' << value[0] << ", " << value[1] << ", " << value[2] << ']';
	}

	bool_t Validate_Shape(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		std::string& outStatus,
		const bool_t validatePatternLinks = true)
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
			document.iNextLogicOrdinal < 1u ||
			document.iNextLogicOrdinal > MAX_NEXT_ORDINAL ||
			document.Logics.size() > MAX_LOGICS ||
			document.iNextSummonOrdinal < 1u ||
			document.iNextSummonOrdinal > MAX_NEXT_ORDINAL ||
			document.Summons.size() > MAX_SUMMONS ||
			document.iNextWorldOrdinal < 1u ||
			document.iNextWorldOrdinal > MAX_NEXT_ORDINAL ||
			document.Worlds.size() > MAX_WORLDS ||
			document.iNextSceneProfileOrdinal < 1u ||
			document.iNextSceneProfileOrdinal > MAX_NEXT_ORDINAL ||
			document.SceneProfiles.size() > MAX_SCENE_PROFILES ||
			0u == document.MadnessPolicy.iMaximum ||
			document.MadnessPolicy.iMaximum > MAX_MADNESS_MAXIMUM ||
			document.MadnessPolicy.iClownHoldMs > MAX_TIME_MS ||
			document.Patterns.size() > MAX_PATTERNS ||
			document.PlayAllPatternIds.size() > MAX_PATTERNS)
		{
			outStatus = "KoukuSaydon composition header or bounded collection size is invalid.";
			return false;
		}

		std::unordered_set<std::string> logicIds;
		std::unordered_map<std::string, const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION*> logicById;
		for (const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic : document.Logics)
		{
			if (!Is_StableId(logic.strLogicId) ||
				!logic.strLogicId.starts_with(GENERATED_LOGIC_PREFIX) ||
				!Try_ParseGeneratedOrdinal(logic.strLogicId,
					GENERATED_LOGIC_PREFIX, document.iNextLogicOrdinal) ||
				!logicIds.insert(logic.strLogicId).second ||
				!Is_DisplayName(logic.strDisplayName) ||
				!Is_LogicType(logic.strLogicType))
			{
				outStatus = "KoukuSaydon Logic definition identity, name, or type is invalid: " +
					logic.strLogicId;
				return false;
			}
			if (!Validate_LogicDefinitionValues(logic, outStatus))
				return false;
			logicById.emplace(logic.strLogicId, &logic);
		}
		/* An outcome slot lists RESULT Logics only, and only a DURATION box may
		   own outcomes: a judgement window is the only thing that ends in
		   success, failure or timeout. */
		const auto findLogic = [&logicById](const std::string& logicId)
			-> const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION*
		{
			const auto found = logicById.find(logicId);
			return found == logicById.end() ? nullptr : found->second;
		};
		const auto isResultReference = [&findLogic](const std::string& logicId)
		{
			const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION* const logic = findLogic(logicId);
			return nullptr != logic && "RESULT" == logic->strLogicType;
		};

		std::unordered_set<std::string> summonIds;
		for (const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION& summon : document.Summons)
		{
			if (!Is_StableId(summon.strSummonId) ||
				!summon.strSummonId.starts_with(GENERATED_SUMMON_PREFIX) ||
				!Try_ParseGeneratedOrdinal(summon.strSummonId,
					GENERATED_SUMMON_PREFIX, document.iNextSummonOrdinal) ||
				!summonIds.insert(summon.strSummonId).second ||
				!Is_DisplayName(summon.strDisplayName))
			{
				outStatus = "KoukuSaydon Summon definition identity or name is invalid: " +
					summon.strSummonId;
				return false;
			}
		}

		std::unordered_set<std::string> worldIds;
		std::unordered_map<std::string, std::string> worldInstanceIds;
		for (const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world : document.Worlds)
		{
			if (!Is_StableId(world.strWorldId) ||
				!world.strWorldId.starts_with(GENERATED_WORLD_PREFIX) ||
				!Try_ParseGeneratedOrdinal(world.strWorldId,
					GENERATED_WORLD_PREFIX, document.iNextWorldOrdinal) ||
				!worldIds.insert(world.strWorldId).second ||
				!Is_DisplayName(world.strDisplayName) ||
				!Is_StableId(world.strSequenceInstanceId) ||
				(world.strAnchorKind != "NONE" && world.strAnchorKind != "BOSS_SPAWN") ||
				!Valid_PresentationVector(world.AnchorPosition, -100000.0, 100000.0) ||
				!std::all_of(world.PositionOffset.begin(), world.PositionOffset.end(),
					[](const double value) { return std::isfinite(value) && std::abs(value) <= 100000.0; }))
			{
				outStatus = "KoukuSaydon World definition identity, name, or sequence instance is invalid: " +
					world.strWorldId;
				return false;
			}
			worldInstanceIds.emplace(world.strWorldId, world.strSequenceInstanceId);
		}
		std::unordered_set<std::string> sceneProfileIds;
		for (const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION& profile : document.SceneProfiles)
		{
			if (!Is_StableId(profile.strSceneProfileId) ||
				!profile.strSceneProfileId.starts_with(GENERATED_SCENE_PROFILE_PREFIX) ||
				!Try_ParseGeneratedOrdinal(profile.strSceneProfileId,
					GENERATED_SCENE_PROFILE_PREFIX, document.iNextSceneProfileOrdinal) ||
				!sceneProfileIds.insert(profile.strSceneProfileId).second ||
				!Is_DisplayName(profile.strDisplayName) ||
				!Is_StableId(profile.strRenderingProfileId))
			{
				outStatus = "KoukuSaydon Scene Profile definition identity, name, or rendering profile is invalid: " +
					profile.strSceneProfileId;
				return false;
			}
		}

		std::unordered_map<std::string, const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE*> presentationResources;
		if (document.iNextPresentationResourceOrdinal == 0u ||
			document.iNextPresentationResourceOrdinal > MAX_NEXT_ORDINAL || document.PresentationResources.size() > 4096u)
		{ outStatus = "Invalid presentation resource count."; return false; }
		for (const auto& row : document.PresentationResources)
		{
			bool_t validAsset = false;
			switch (row.eKind)
			{
			case KOUKU_SAYDON_PRESENTATION_KIND::EFFECT:
				validAsset = Is_StableId(row.strAssetId) &&
					(row.strResourceKind == "GROUP" || row.strResourceKind == "LEAF"); break;
			case KOUKU_SAYDON_PRESENTATION_KIND::LIGHT:
				validAsset = Is_StableId(row.strAssetId) && row.strResourceKind.empty() &&
					(row.strDefaultAnchorKind == "MAP" || row.strDefaultAnchorKind == "PLAYER" || row.strDefaultAnchorKind == "BOSS"); break;
			case KOUKU_SAYDON_PRESENTATION_KIND::CAMERA:
				validAsset = Is_StableId(row.strAssetId); break;
			case KOUKU_SAYDON_PRESENTATION_KIND::SOUND:
			{
				const std::filesystem::path assetPath(row.strAssetId);
				validAsset = row.strAssetId.starts_with("Sound/") &&
					!assetPath.is_absolute() && row.strAssetId.find(':') == std::string::npos &&
					row.strAssetId.find('\\') == std::string::npos &&
					std::none_of(assetPath.begin(), assetPath.end(), [](const auto& part) { return part == ".."; });
				break;
			}
			case KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER:
				validAsset = row.strAssetId.empty() && (row.strShape == "BOX" || row.strShape == "SECTOR" || row.strShape == "CIRCLE"); break;
			default: break;
			}
			if (!Is_StableId(row.strResourceId) || !Try_ParseGeneratedOrdinal(row.strResourceId,
				"kakulsaydon.g1.presentation.", document.iNextPresentationResourceOrdinal) ||
				!presentationResources.emplace(row.strResourceId, &row).second ||
				!Is_DisplayName(row.strDisplayName) || !validAsset ||
				(row.strColliderKind != "GEOMETRY" && row.strColliderKind != "ROULETTE_CARD_REGION") ||
				(row.eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER && row.strColliderKind != "GEOMETRY") ||
				row.iDurationMs == 0u || row.iDurationMs > MAX_TIME_MS ||
				!Valid_PresentationVector(row.HalfExtents, 0.001, 10000.0) ||
				!std::isfinite(row.fRadiusM) || row.fRadiusM <= 0.0 || row.fRadiusM > 10000.0 ||
				!std::isfinite(row.fHalfAngleDegrees) || row.fHalfAngleDegrees <= 0.0 || row.fHalfAngleDegrees > 180.0)
			{ outStatus = "Invalid presentation resource: " + row.strResourceId; return false; }
		}

		for (const auto& world : document.Worlds)
			if (!world.strCompanionEffectResourceId.empty())
			{
				const auto effect = presentationResources.find(world.strCompanionEffectResourceId);
				if (effect == presentationResources.end() || effect->second->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
				{ outStatus = "World companion must name an EFFECT resource: " + world.strWorldId; return false; }
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
				(pattern.strAuthoringStatus == "PRODUCT" &&
					!Is_ArenaBossBodyProfile(pattern.strActorProfileId)) ||
				!Is_DisplayName(pattern.strDisplayName) ||
				!Is_AuthoringStatus(pattern.strAuthoringStatus) ||
				!Is_Category(pattern.strCategory) ||
				pattern.iNextStageOrdinal < 1u ||
				pattern.iNextStageOrdinal > MAX_NEXT_ORDINAL ||
				pattern.iNextAnimationOrdinal < 1u ||
				pattern.iNextAnimationOrdinal > MAX_NEXT_ORDINAL ||
				pattern.iNextLogicOccurrenceOrdinal < 1u ||
				pattern.iNextLogicOccurrenceOrdinal > MAX_NEXT_ORDINAL ||
				pattern.LogicOccurrences.size() > MAX_LOGIC_OCCURRENCES_PER_PATTERN ||
				pattern.iNextSummonOccurrenceOrdinal < 1u ||
				pattern.iNextSummonOccurrenceOrdinal > MAX_NEXT_ORDINAL ||
				pattern.SummonOccurrences.size() > MAX_SUMMON_OCCURRENCES_PER_PATTERN ||
				pattern.iNextWorldOccurrenceOrdinal < 1u ||
				pattern.iNextWorldOccurrenceOrdinal > MAX_NEXT_ORDINAL ||
				pattern.WorldOccurrences.size() > MAX_WORLD_OCCURRENCES_PER_PATTERN ||
				pattern.iNextSceneProfileOccurrenceOrdinal < 1u ||
				pattern.iNextSceneProfileOccurrenceOrdinal > MAX_NEXT_ORDINAL ||
				pattern.SceneProfileOccurrences.size() > MAX_SCENE_PROFILE_OCCURRENCES_PER_PATTERN ||
				pattern.iNextPresentationOccurrenceOrdinal == 0u ||
				pattern.iNextPresentationOccurrenceOrdinal > MAX_NEXT_ORDINAL ||
				pattern.PresentationOccurrences.size() > 1024u ||
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

		/* Logic boxes are pattern-relative, so their window is checked against the
		   whole Stage clock sum. A PRODUCT Pattern may own typed judgement boxes;
		   a box whose Logic is still only a name keeps the Pattern in DRAFT. */
		std::vector<std::pair<std::string, std::string>> followupTargets;
		for (const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern : document.Patterns)
		{
			if (!pattern.strLoadError.empty())
				continue;
			const bool_t product = "PRODUCT" == pattern.strAuthoringStatus;
			std::uint64_t lifetimeMs = 0u;
			for (const KOUKU_SAYDON_COMPOSITION_STAGE& stage : pattern.Stages)
				lifetimeMs += stage.iDurationMs;
			std::unordered_set<std::string> regionIds;
			std::unordered_set<std::string> companionWorldBoxes;
			for (const auto& row : pattern.PresentationOccurrences)
			{
				const std::uint64_t endMs = static_cast<std::uint64_t>(row.iStartMs) + row.iDurationMs;
				if (!Is_StableId(row.strOccurrenceId) || !Try_ParseGeneratedOrdinal(row.strOccurrenceId,
					pattern.strPatternId + ".presentation.", pattern.iNextPresentationOccurrenceOrdinal) ||
					!occurrenceIds.insert(row.strOccurrenceId).second ||
					!presentationResources.contains(row.strResourceId) || row.iDurationMs == 0u ||
					endMs > lifetimeMs || endMs > MAX_TIME_MS ||
					static_cast<std::uint64_t>(row.iFadeInMs) + row.iFadeOutMs > row.iDurationMs ||
					!Valid_PresentationVector(row.PositionOffset, -100000.0, 100000.0) ||
					!Valid_PresentationVector(row.RotationDegrees, -36000.0, 36000.0) ||
					!Valid_PresentationVector(row.Scale, 0.001, 10000.0) ||
					!std::isfinite(row.fDissolveStart) || row.fDissolveStart < 0.0 || row.fDissolveStart > 1.0 ||
					!std::isfinite(row.fDissolveEnd) || row.fDissolveEnd <= row.fDissolveStart || row.fDissolveEnd > 1.0 ||
					!std::isfinite(row.fVolume) || row.fVolume < 0.0 || row.fVolume > 1.0 ||
					!std::isfinite(row.fBrightnessMultiplier) || row.fBrightnessMultiplier < 0.0 || row.fBrightnessMultiplier > 16.0 ||
					(!row.strBone.empty() && !Is_StableId(row.strBone)))
				{ outStatus = "Invalid presentation occurrence or window: " + row.strOccurrenceId; return false; }
				const bool_t knownSymbol = row.strCardSymbol == "NONE" ||
					std::any_of(KOUKU_SAYDON_CARD_SYMBOLS.begin(), KOUKU_SAYDON_CARD_SYMBOLS.end(),
						[&](const char* symbol) { return row.strCardSymbol == symbol; });
				if ((!row.strRegionId.empty() && (!Is_StableId(row.strRegionId) || !regionIds.insert(row.strRegionId).second)) ||
					!knownSymbol || (row.strCardColor != "NONE" && row.strCardColor != "RED" && row.strCardColor != "BLACK") ||
					(row.strAnchorKind != "BOSS" && row.strAnchorKind != "WORLD" &&
						row.strAnchorKind != "PLAYER" && row.strAnchorKind != "MAP") ||
					(!row.strWorldId.empty() && !worldIds.contains(row.strWorldId)) ||
					(row.strAnchorKind != "WORLD" && !row.strWorldId.empty()))
				{ outStatus = "Invalid region identity, card or anchor: " + row.strOccurrenceId; return false; }
				const auto* resource = presentationResources.at(row.strResourceId);
				if ((resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::LIGHT &&
					(row.strAnchorKind == "PLAYER" || row.strAnchorKind == "MAP" || row.fBrightnessMultiplier != 1.0)) ||
					(resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT &&
					(row.strAnchorKind == "WORLD" || row.Scale != std::array<double, 3u>{1.0, 1.0, 1.0} ||
					 (row.strAnchorKind != "BOSS" && !row.strBone.empty()) ||
					 (row.strAnchorKind == "PLAYER" && !row.bFollowBoss))))
				{ outStatus = "Invalid Light anchor, scale, bone or brightness: " + row.strOccurrenceId; return false; }
				if (!row.strWorldOccurrenceId.empty())
				{
					const auto owner = std::find_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
						[&](const auto& box) { return box.strOccurrenceId == row.strWorldOccurrenceId; });
					const auto world = owner == pattern.WorldOccurrences.end() ? document.Worlds.end() :
						std::find_if(document.Worlds.begin(), document.Worlds.end(), [&](const auto& value) { return value.strWorldId == owner->strWorldId; });
					if (resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT || !Is_StableId(row.strWorldOccurrenceId) ||
						world == document.Worlds.end() || world->strCompanionEffectResourceId != row.strResourceId ||
						!companionWorldBoxes.insert(row.strWorldOccurrenceId).second)
					{ outStatus = "Effect companion needs one matching World box/resource in the same Pattern: " + row.strOccurrenceId; return false; }
				}
				if (!row.strLogicOccurrenceId.empty())
				{
					const auto linked = std::find_if(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(),
						[&](const auto& box) { return box.strOccurrenceId == row.strLogicOccurrenceId; });
					const auto* owner = linked == pattern.LogicOccurrences.end() ? nullptr : findLogic(linked->strLogicId);
					if (resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER || nullptr == owner ||
						(owner->strLogicType != "DURATION" && owner->strLogicType != "TRIGGER") ||
						linked->iStartMs != row.iStartMs || linked->iDurationMs != row.iDurationMs)
					{ outStatus = "Collider link needs an existing Duration/Trigger window with identical timing: " + row.strOccurrenceId; return false; }
				}
				const bool_t rouletteRegion = resource->strColliderKind == "ROULETTE_CARD_REGION";
				if (product && ((row.strAnchorKind == "WORLD" && row.strWorldId.empty()) ||
					(rouletteRegion && (row.strRegionId.empty() || row.strCardSymbol == "NONE" ||
						row.strCardColor == "NONE" || row.strAnchorKind != "WORLD" || row.strWorldId.empty()))))
				{ outStatus = "PRODUCT roulette region needs an explicit World, region ID, symbol and color: " + row.strOccurrenceId; return false; }
			}
			std::unordered_set<std::string> logicBoxIds;
			std::vector<std::string> rouletteInstances;
			const std::string logicPrefix = pattern.strPatternId + ".logic.";
			for (const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE& box : pattern.LogicOccurrences)
			{
				const std::uint64_t boxEndMs =
					static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs;
				if (!Is_StableId(box.strOccurrenceId) ||
					!box.strOccurrenceId.starts_with(logicPrefix) ||
					!Try_ParseGeneratedOrdinal(box.strOccurrenceId, logicPrefix,
						pattern.iNextLogicOccurrenceOrdinal) ||
					!logicBoxIds.insert(box.strOccurrenceId).second ||
					!logicIds.contains(box.strLogicId) ||
					box.iStartMs > MAX_TIME_MS || 0u == box.iDurationMs ||
					boxEndMs > MAX_TIME_MS || boxEndMs > lifetimeMs)
				{
					outStatus = "KoukuSaydon Logic box identity, Logic reference, or window exceeds the Pattern lifetime: " +
						box.strOccurrenceId;
					return false;
				}
				const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& owner = *findLogic(box.strLogicId);
				const bool_t hasOutcome = !box.OnSuccessLogicIds.empty() ||
					!box.OnFailLogicIds.empty() || !box.OnTimeoutLogicIds.empty();
				if (hasOutcome && !Kouku_LogicOwnsOutcomes(owner))
				{
					outStatus = "KoukuSaydon Logic box outcomes belong to a DURATION or ENTER_AREA box: " +
						box.strOccurrenceId;
					return false;
				}
				for (const KOUKU_SAYDON_OUTCOME_SLOT slot : {
					KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS, KOUKU_SAYDON_OUTCOME_SLOT::FAIL,
					KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT })
				{
					const std::vector<std::string>& targets = box.Outcomes(slot);
					if (targets.size() > KOUKU_SAYDON_MAX_OUTCOMES_PER_SLOT)
					{
						outStatus = "KoukuSaydon Logic box outcome slot holds more than four RESULT Logics: " +
							box.strOccurrenceId;
						return false;
					}
					if (!targets.empty() && !Kouku_LogicOutcomeKind(owner).empty() &&
						!Kouku_IsOutcomeSlotAllowed(Kouku_LogicOutcomeKind(owner), slot))
					{
						outStatus = "KoukuSaydon Logic box wires an outcome slot its judgement kind never ends in: " +
							box.strOccurrenceId;
						return false;
					}
					for (const std::string& target : targets)
					{
						if (!isResultReference(target))
						{
							outStatus = "KoukuSaydon Logic box outcomes must name RESULT Logics: " +
								box.strOccurrenceId;
							return false;
						}
						const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& result = *findLogic(target);
						if ("FOLLOWUP_PATTERN" == result.strOutcomeKind)
						{
							if ("STAGGER_WINDOW" != owner.strJudgementKind)
							{
								outStatus = "A follow-up Pattern outcome belongs to a stagger window only: " +
									box.strOccurrenceId;
								return false;
							}
							followupTargets.emplace_back(box.strOccurrenceId, result.strFollowupPatternId);
						}
						if (product && result.strOutcomeKind.empty())
						{
							outStatus = "KoukuSaydon PRODUCT Pattern wires a RESULT Logic without an outcome kind: " +
								box.strOccurrenceId + " -> " + target;
							return false;
						}
					}
				}
				if (product && "DURATION" == owner.strLogicType && owner.strJudgementKind.empty())
				{
					outStatus = "KoukuSaydon PRODUCT Pattern owns a DURATION box without a judgement kind: " +
						box.strOccurrenceId;
					return false;
				}
				if (product && box.bEnabled && "ROULETTE_CARD_MATCH" == owner.strJudgementKind)
				{
					if (owner.RegionIds.size() != 8u)
					{ outStatus = "PRODUCT roulette needs eight explicitly mapped region IDs: " + box.strOccurrenceId; return false; }
					for (const auto& regionId : owner.RegionIds)
					{
						const auto region = std::find_if(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
							[&](const auto& occurrence) { return occurrence.strRegionId == regionId; });
						if (region == pattern.PresentationOccurrences.end() || region->strAnchorKind != "WORLD" ||
							region->strWorldId.empty() || presentationResources.at(region->strResourceId)->strColliderKind != "ROULETTE_CARD_REGION")
						{ outStatus = "PRODUCT roulette region is missing or not bound to a World: " + regionId; return false; }
						rouletteInstances.push_back(worldInstanceIds.at(region->strWorldId));
					}
				}
			}

			/* Summon boxes stay authoring-only: the Server consumer is a later
			   slice, so a PRODUCT Pattern may keep them without projecting them. */
			std::unordered_set<std::string> summonBoxIds;
			const std::string summonPrefix = pattern.strPatternId + ".summon.";
			for (const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE& box : pattern.SummonOccurrences)
			{
				const std::uint64_t boxEndMs =
					static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs;
				if (!Is_StableId(box.strOccurrenceId) ||
					!box.strOccurrenceId.starts_with(summonPrefix) ||
					!Try_ParseGeneratedOrdinal(box.strOccurrenceId, summonPrefix,
						pattern.iNextSummonOccurrenceOrdinal) ||
					!summonBoxIds.insert(box.strOccurrenceId).second ||
					!summonIds.contains(box.strSummonId) ||
					box.iStartMs > MAX_TIME_MS || 0u == box.iDurationMs ||
					boxEndMs > MAX_TIME_MS || boxEndMs > lifetimeMs)
				{
					outStatus = "KoukuSaydon Summon box identity, Summon reference, or window exceeds the Pattern lifetime: " +
						box.strOccurrenceId;
					return false;
				}
			}

			/* A World box starts its sequence on the pattern clock; the sequence
			   runs to its own end, so only the start must sit inside the Pattern. */
			std::unordered_set<std::string> worldBoxIds;
			std::unordered_set<std::string> playedInstances;
			const std::string worldPrefix = pattern.strPatternId + ".world.";
			for (const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& box : pattern.WorldOccurrences)
			{
				const std::uint64_t boxEndMs =
					static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs;
				if (!Is_StableId(box.strOccurrenceId) ||
					!box.strOccurrenceId.starts_with(worldPrefix) ||
					!Try_ParseGeneratedOrdinal(box.strOccurrenceId, worldPrefix,
						pattern.iNextWorldOccurrenceOrdinal) ||
					!worldBoxIds.insert(box.strOccurrenceId).second ||
					!worldIds.contains(box.strWorldId) ||
					box.iStartMs > MAX_TIME_MS || 0u == box.iDurationMs ||
					boxEndMs > MAX_TIME_MS ||
					static_cast<std::uint64_t>(box.iStartMs) > lifetimeMs ||
					!std::isfinite(box.fPlaybackSpeed) ||
					box.fPlaybackSpeed < static_cast<f32_t>(MIN_WORLD_PLAYBACK_SPEED) ||
					box.fPlaybackSpeed > static_cast<f32_t>(MAX_WORLD_PLAYBACK_SPEED))
				{
					outStatus = "KoukuSaydon World box identity, World reference, start, or playback speed is invalid: " +
						box.strOccurrenceId;
					return false;
				}
				if (!playedInstances.insert(worldInstanceIds.at(box.strWorldId)).second)
				{
					outStatus = "KoukuSaydon Pattern plays the same world sequence instance from two boxes: " +
						box.strOccurrenceId;
					return false;
				}
			}
			for (const std::string& instanceId : rouletteInstances)
			{
				if (!playedInstances.contains(instanceId))
				{
					outStatus = "KoukuSaydon PRODUCT roulette window needs a World box playing " +
						instanceId + ": " + pattern.strPatternId;
					return false;
				}
			}

			std::unordered_set<std::string> sceneBoxIds;
			const std::string scenePrefix = pattern.strPatternId + ".sceneprofile.";
			for (const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE& box : pattern.SceneProfileOccurrences)
			{
				const std::uint64_t boxEndMs =
					static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs;
				if (!Is_StableId(box.strOccurrenceId) ||
					!box.strOccurrenceId.starts_with(scenePrefix) ||
					!Try_ParseGeneratedOrdinal(box.strOccurrenceId, scenePrefix,
						pattern.iNextSceneProfileOccurrenceOrdinal) ||
					!sceneBoxIds.insert(box.strOccurrenceId).second ||
					!sceneProfileIds.contains(box.strSceneProfileId) ||
					box.iStartMs > MAX_TIME_MS || 0u == box.iDurationMs ||
					boxEndMs > MAX_TIME_MS || boxEndMs > lifetimeMs ||
					box.iBlendMs > MAX_TIME_MS)
				{
					outStatus = "KoukuSaydon Scene Profile box identity, reference, window, or blend is invalid: " +
						box.strOccurrenceId;
					return false;
				}
			}
		}
		for (const auto& [boxId, target] : followupTargets)
		{
			if (validatePatternLinks && !patternIds.contains(target))
			{
				outStatus = "KoukuSaydon follow-up outcome names an unknown Pattern: " + boxId + " -> " + target;
				return false;
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
		!Has_Properties(root,
			{ "schema", "formatVersion", "revision", "compositionId",
			  "encounterId", "bossArchetypeId", "bossPlacementId", "areaId",
			  "fixedTickHz", "nextPatternOrdinal", "playAllPatternIds", "patterns" },
			{ "nextLogicOrdinal", "logics", "nextSummonOrdinal", "summons",
			  "nextWorldOrdinal", "worlds", "nextSceneProfileOrdinal", "sceneProfiles",
			  "nextPresentationResourceOrdinal", "presentationResources",
			  "madnessPolicy" }))
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

	/* The Logic catalog is parsed before any Pattern so a per-Pattern candidate
	   can resolve the logicId a box references. */
	const DATA_JSON_VALUE* const nextLogicOrdinal = root.Find("nextLogicOrdinal");
	const DATA_JSON_VALUE* const logics = root.Find("logics");
	if ((nullptr != nextLogicOrdinal &&
		 (!Try_ParseUnsigned(*nextLogicOrdinal, MAX_NEXT_ORDINAL, staged.iNextLogicOrdinal) ||
		  0u == staged.iNextLogicOrdinal)) ||
		(nullptr != logics &&
		 (!logics->Is_Array() || logics->Get_Array().size() > MAX_LOGICS)))
	{
		outStatus = "KoukuSaydon composition Logic catalog header is invalid.";
		return false;
	}
	if (nullptr != logics)
	{
		staged.Logics.reserve(logics->Get_Array().size());
		for (const DATA_JSON_VALUE& logicValue : logics->Get_Array())
		{
			if (!Has_Properties(logicValue, { "logicId", "displayName", "logicType" },
					{ "judgementKind", "insideOutcome", "sectorCount", "sectorSymbols", "regionIds", "centerX", "centerZ",
					  "outerRadiusM", "worldSequenceInstanceId", "halfAngleDegrees",
					  "maxDistanceM", "poseIndex", "threshold", "shieldArcDegrees",
					  "endsPatternOnSuccess", "normalYawOffsetDegrees", "faceCenterYawOffsetDegrees", "outcomeKind", "percent", "durationMs",
					  "followupPatternId", "triggerKind", "hudMode", "teleportPosition", "clonePatternId", "clockHours" }))
			{
				outStatus = "KoukuSaydon Logic definition has unexpected properties.";
				return false;
			}
			const DATA_JSON_VALUE* const logicId = Required(logicValue, "logicId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* const logicName = Required(logicValue, "displayName", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* const logicType = Required(logicValue, "logicType", DATA_JSON_TYPE::STRING);
			if (nullptr == logicId || nullptr == logicName || nullptr == logicType)
			{
				outStatus = "KoukuSaydon Logic definition value or type is invalid.";
				return false;
			}
			KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION stagedLogic;
			stagedLogic.strLogicId = logicId->Get_String();
			stagedLogic.strDisplayName = logicName->Get_String();
			stagedLogic.strLogicType = logicType->Get_String();
			/* Typed values are optional; each present key must parse in range. */
			const auto optionalText = [&logicValue](const char_t* const name, std::string& out)
			{
				const DATA_JSON_VALUE* const value = logicValue.Find(name);
				if (nullptr == value)
					return true;
				if (!value->Is_String())
					return false;
				out = value->Get_String();
				return true;
			};
			const auto optionalUnsigned = [&logicValue](const char_t* const name,
				const std::uint32_t maximum, std::uint32_t& out)
			{
				const DATA_JSON_VALUE* const value = logicValue.Find(name);
				return nullptr == value || Try_ParseUnsigned(*value, maximum, out);
			};
			const auto optionalFinite = [&logicValue](const char_t* const name,
				const double minimum, const double maximum, double& out)
			{
				const DATA_JSON_VALUE* const value = logicValue.Find(name);
				return nullptr == value || Try_ParseFinite(*value, minimum, maximum, out);
			};
			const DATA_JSON_VALUE* const endsPattern = logicValue.Find("endsPatternOnSuccess");
			if (!optionalText("judgementKind", stagedLogic.strJudgementKind) ||
				!optionalText("insideOutcome", stagedLogic.strInsideOutcome) ||
				!optionalUnsigned("sectorCount", 64u, stagedLogic.iSectorCount) ||
				!Try_ParseTextList(logicValue.Find("sectorSymbols"), 64u, stagedLogic.SectorSymbols) ||
				!Try_ParseTextList(logicValue.Find("regionIds"), 8u, stagedLogic.RegionIds) ||
				!optionalFinite("centerX", -100000.0, 100000.0, stagedLogic.fCenterX) ||
				!optionalFinite("centerZ", -100000.0, 100000.0, stagedLogic.fCenterZ) ||
				!optionalFinite("outerRadiusM", 0.0, 1000.0, stagedLogic.fOuterRadiusM) ||
				!optionalText("worldSequenceInstanceId", stagedLogic.strWorldSequenceInstanceId) ||
				!optionalFinite("halfAngleDegrees", 0.0, 180.0, stagedLogic.fHalfAngleDegrees) ||
				!optionalFinite("maxDistanceM", 0.0, 1000.0, stagedLogic.fMaxDistanceM) ||
				!optionalUnsigned("poseIndex", 7u, stagedLogic.iPoseIndex) ||
				!optionalUnsigned("threshold", (std::numeric_limits<std::uint32_t>::max)(), stagedLogic.iThreshold) ||
				!optionalFinite("shieldArcDegrees", 0.0, 360.0, stagedLogic.fShieldArcDegrees) ||
				!optionalFinite("normalYawOffsetDegrees", -360.0, 360.0, stagedLogic.fNormalYawOffsetDegrees) ||
				!optionalFinite("faceCenterYawOffsetDegrees", -360.0, 360.0, stagedLogic.fFaceCenterYawOffsetDegrees) ||
				(nullptr != endsPattern && !endsPattern->Is_Boolean()) ||
				!optionalText("outcomeKind", stagedLogic.strOutcomeKind) ||
				!optionalUnsigned("percent", 100u, stagedLogic.iPercent) ||
				!optionalUnsigned("durationMs", MAX_TIME_MS, stagedLogic.iDurationMs) ||
				!optionalText("followupPatternId", stagedLogic.strFollowupPatternId) ||
				!optionalText("triggerKind", stagedLogic.strTriggerKind) ||
				!optionalText("hudMode", stagedLogic.strHudMode) ||
				!optionalText("clonePatternId", stagedLogic.strClonePatternId))
			{
				outStatus = "KoukuSaydon Logic definition typed value is invalid: " + stagedLogic.strLogicId;
				return false;
			}
			if (const auto* pos = logicValue.Find("teleportPosition"))
			{
				if (!pos->Is_Array() || pos->Get_Array().size() != 3u)
				{ outStatus = "teleportPosition must contain XYZ."; return false; }
				for (std::size_t i = 0; i < 3u; ++i)
					if (!Try_ParseFinite(pos->Get_Array()[i], -100000.0, 100000.0, stagedLogic.TeleportPosition[i]))
					{ outStatus = "teleportPosition must be finite."; return false; }
			}
			if (const auto* hours = logicValue.Find("clockHours"))
			{
				if (!hours->Is_Array() || hours->Get_Array().size() != 3u)
				{ outStatus = "clockHours must contain three hours."; return false; }
				for (const auto& value : hours->Get_Array())
				{
					std::uint32_t hour = 0u;
					if (!Try_ParseUnsigned(value, 12u, hour) || hour == 0u)
					{ outStatus = "clockHours must be 1..12."; return false; }
					stagedLogic.ClockHours.push_back(hour);
				}
			}
			if (nullptr != endsPattern)
				stagedLogic.bEndsPatternOnSuccess = endsPattern->Get_Boolean();
			staged.Logics.push_back(std::move(stagedLogic));
		}
	}

	const DATA_JSON_VALUE* const nextSummonOrdinal = root.Find("nextSummonOrdinal");
	const DATA_JSON_VALUE* const summons = root.Find("summons");
	if ((nullptr != nextSummonOrdinal &&
		 (!Try_ParseUnsigned(*nextSummonOrdinal, MAX_NEXT_ORDINAL, staged.iNextSummonOrdinal) ||
		  0u == staged.iNextSummonOrdinal)) ||
		(nullptr != summons &&
		 (!summons->Is_Array() || summons->Get_Array().size() > MAX_SUMMONS)))
	{
		outStatus = "KoukuSaydon composition Summon catalog header is invalid.";
		return false;
	}
	if (nullptr != summons)
	{
		staged.Summons.reserve(summons->Get_Array().size());
		for (const DATA_JSON_VALUE& summonValue : summons->Get_Array())
		{
			if (!Has_ExactProperties(summonValue, { "summonId", "displayName" }))
			{
				outStatus = "KoukuSaydon Summon definition has unexpected properties.";
				return false;
			}
			const DATA_JSON_VALUE* const summonId = Required(summonValue, "summonId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* const summonName = Required(summonValue, "displayName", DATA_JSON_TYPE::STRING);
			if (nullptr == summonId || nullptr == summonName)
			{
				outStatus = "KoukuSaydon Summon definition value or type is invalid.";
				return false;
			}
			KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION stagedSummon;
			stagedSummon.strSummonId = summonId->Get_String();
			stagedSummon.strDisplayName = summonName->Get_String();
			staged.Summons.push_back(std::move(stagedSummon));
		}
	}

	const DATA_JSON_VALUE* const nextWorldOrdinal = root.Find("nextWorldOrdinal");
	const DATA_JSON_VALUE* const worlds = root.Find("worlds");
	if ((nullptr != nextWorldOrdinal &&
		 (!Try_ParseUnsigned(*nextWorldOrdinal, MAX_NEXT_ORDINAL, staged.iNextWorldOrdinal) ||
		  0u == staged.iNextWorldOrdinal)) ||
		(nullptr != worlds &&
		 (!worlds->Is_Array() || worlds->Get_Array().size() > MAX_WORLDS)))
	{
		outStatus = "KoukuSaydon composition World catalog header is invalid.";
		return false;
	}
	if (nullptr != worlds)
	{
		staged.Worlds.reserve(worlds->Get_Array().size());
		for (const DATA_JSON_VALUE& worldValue : worlds->Get_Array())
		{
			if (!Has_Properties(worldValue, { "worldId", "displayName", "sequenceInstanceId" }, { "positionOffset", "anchorKind", "anchorPosition", "companionEffectResourceId" }))
			{
				outStatus = "KoukuSaydon World definition has unexpected properties.";
				return false;
			}
			const DATA_JSON_VALUE* const worldId = Required(worldValue, "worldId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* const worldName = Required(worldValue, "displayName", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* const instanceId = Required(worldValue, "sequenceInstanceId", DATA_JSON_TYPE::STRING);
			if (nullptr == worldId || nullptr == worldName || nullptr == instanceId)
			{
				outStatus = "KoukuSaydon World definition value or type is invalid.";
				return false;
			}
			KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION stagedWorld;
			stagedWorld.strWorldId = worldId->Get_String();
			stagedWorld.strDisplayName = worldName->Get_String();
			stagedWorld.strSequenceInstanceId = instanceId->Get_String();
			if (const auto* position = worldValue.Find("positionOffset"))
			{
				if (!position->Is_Array() || position->Get_Array().size() != 3u)
				{
					outStatus = "KoukuSaydon World positionOffset requires three numbers.";
					return false;
				}
				for (size_t i = 0u; i < 3u; ++i)
					if (!Try_ParseFinite(position->Get_Array()[i], -100000.0, 100000.0, stagedWorld.PositionOffset[i]))
					{
						outStatus = "KoukuSaydon World positionOffset is not finite or exceeds 100000 metres.";
						return false;
					}
			}
			if (!Read_PresentationText(worldValue, "anchorKind", stagedWorld.strAnchorKind) ||
				!Read_PresentationText(worldValue, "companionEffectResourceId", stagedWorld.strCompanionEffectResourceId) ||
				!Read_PresentationVector(worldValue, "anchorPosition", stagedWorld.AnchorPosition, -100000.0, 100000.0))
			{ outStatus = "Invalid World anchor."; return false; }
			staged.Worlds.push_back(std::move(stagedWorld));
		}
	}

	const DATA_JSON_VALUE* const nextSceneProfileOrdinal = root.Find("nextSceneProfileOrdinal");
	const DATA_JSON_VALUE* const sceneProfiles = root.Find("sceneProfiles");
	if ((nullptr != nextSceneProfileOrdinal &&
		 (!Try_ParseUnsigned(*nextSceneProfileOrdinal, MAX_NEXT_ORDINAL, staged.iNextSceneProfileOrdinal) ||
		  0u == staged.iNextSceneProfileOrdinal)) ||
		(nullptr != sceneProfiles &&
		 (!sceneProfiles->Is_Array() || sceneProfiles->Get_Array().size() > MAX_SCENE_PROFILES)))
	{
		outStatus = "KoukuSaydon composition Scene Profile catalog header is invalid.";
		return false;
	}
	if (nullptr != sceneProfiles)
	{
		staged.SceneProfiles.reserve(sceneProfiles->Get_Array().size());
		for (const DATA_JSON_VALUE& profileValue : sceneProfiles->Get_Array())
		{
			if (!Has_ExactProperties(profileValue, { "sceneProfileId", "displayName", "renderingProfileId" }))
			{
				outStatus = "KoukuSaydon Scene Profile definition has unexpected properties.";
				return false;
			}
			const DATA_JSON_VALUE* const profileId = Required(profileValue, "sceneProfileId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* const profileName = Required(profileValue, "displayName", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* const renderingId = Required(profileValue, "renderingProfileId", DATA_JSON_TYPE::STRING);
			if (nullptr == profileId || nullptr == profileName || nullptr == renderingId)
			{
				outStatus = "KoukuSaydon Scene Profile definition value or type is invalid.";
				return false;
			}
			KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION stagedProfile;
			stagedProfile.strSceneProfileId = profileId->Get_String();
			stagedProfile.strDisplayName = profileName->Get_String();
			stagedProfile.strRenderingProfileId = renderingId->Get_String();
			staged.SceneProfiles.push_back(std::move(stagedProfile));
		}
	}

	if (const auto* ordinal = root.Find("nextPresentationResourceOrdinal"); nullptr != ordinal)
		if (!Try_ParseUnsigned(*ordinal, MAX_NEXT_ORDINAL, staged.iNextPresentationResourceOrdinal) ||
			staged.iNextPresentationResourceOrdinal == 0u) { outStatus = "Invalid presentation resource counter."; return false; }
	if (const auto* rows = root.Find("presentationResources"); nullptr != rows)
	{
		if (!rows->Is_Array() || rows->Get_Array().size() > 4096u)
		{ outStatus = "Invalid presentation resource list."; return false; }
		for (const auto& value : rows->Get_Array())
		{
			KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE row;
			if (!Read_PresentationResource(value, row))
			{ outStatus = "Invalid presentation resource value or property."; return false; }
			staged.PresentationResources.push_back(std::move(row));
		}
	}
	if (const DATA_JSON_VALUE* const madnessPolicy = root.Find("madnessPolicy"); nullptr != madnessPolicy)
	{
		const DATA_JSON_VALUE* const maximum = Required(*madnessPolicy, "maximum", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* const clownHoldMs = Required(*madnessPolicy, "clownHoldMs", DATA_JSON_TYPE::NUMBER);
		if (!Has_ExactProperties(*madnessPolicy, { "maximum", "clownHoldMs" }) ||
			nullptr == maximum || nullptr == clownHoldMs ||
			!Try_ParseUnsigned(*maximum, MAX_MADNESS_MAXIMUM, staged.MadnessPolicy.iMaximum) ||
			0u == staged.MadnessPolicy.iMaximum ||
			!Try_ParseUnsigned(*clownHoldMs, MAX_TIME_MS, staged.MadnessPolicy.iClownHoldMs))
		{
			outStatus = "KoukuSaydon composition madness policy is invalid.";
			return false;
		}
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
			Has_Properties(patternValue,
				{ "patternId", "displayName", "authoringStatus", "category",
				  "nextStageOrdinal", "nextAnimationOrdinal", "stages" },
				{ "nextLogicOccurrenceOrdinal", "logicOccurrences",
				  "nextSummonOccurrenceOrdinal", "summonOccurrences",
				  "nextWorldOccurrenceOrdinal", "worldOccurrences",
				  "nextSceneProfileOccurrenceOrdinal", "sceneProfileOccurrences",
				  "nextPresentationOccurrenceOrdinal", "presentationOccurrences", "resetBossToSpawn" }) :
			Has_Properties(patternValue,
				{ "patternId", "actorProfileId", "displayName", "authoringStatus", "category",
				  "nextStageOrdinal", "nextAnimationOrdinal", "stages" },
				{ "nextLogicOccurrenceOrdinal", "logicOccurrences",
				  "nextSummonOccurrenceOrdinal", "summonOccurrences",
				  "nextWorldOccurrenceOrdinal", "worldOccurrences",
				  "nextSceneProfileOccurrenceOrdinal", "sceneProfileOccurrences",
				  "nextPresentationOccurrenceOrdinal", "presentationOccurrences", "resetBossToSpawn" });
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
		const DATA_JSON_VALUE* const nextLogicOccurrenceOrdinal =
			patternValue.Find("nextLogicOccurrenceOrdinal");
		const DATA_JSON_VALUE* const logicOccurrences = patternValue.Find("logicOccurrences");
		if ((nullptr != nextLogicOccurrenceOrdinal &&
			 (!Try_ParseUnsigned(*nextLogicOccurrenceOrdinal, MAX_NEXT_ORDINAL,
				stagedPattern.iNextLogicOccurrenceOrdinal) ||
			  0u == stagedPattern.iNextLogicOccurrenceOrdinal)) ||
			(nullptr != logicOccurrences &&
			 (!logicOccurrences->Is_Array() ||
			  logicOccurrences->Get_Array().size() > MAX_LOGIC_OCCURRENCES_PER_PATTERN)))
		{
			outStatus = "KoukuSaydon Pattern Logic counter or box list is invalid.";
			return false;
		}
		if (nullptr != logicOccurrences)
		{
			stagedPattern.LogicOccurrences.reserve(logicOccurrences->Get_Array().size());
			for (const DATA_JSON_VALUE& boxValue : logicOccurrences->Get_Array())
			{
				if (!Has_Properties(boxValue,
						{ "occurrenceId", "logicId", "startMs", "durationMs" },
						{ "onSuccessLogicId", "onTimeoutLogicId",
						  "onSuccessLogicIds", "onFailLogicIds", "onTimeoutLogicIds", "enabled" }))
				{
					outStatus = "KoukuSaydon Logic box has unexpected properties.";
					return false;
				}
				const DATA_JSON_VALUE* const boxId = Required(boxValue, "occurrenceId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const boxLogicId = Required(boxValue, "logicId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const boxStart = Required(boxValue, "startMs", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* const boxDuration = Required(boxValue, "durationMs", DATA_JSON_TYPE::NUMBER);
				/* The singular keys are the pre-list form and read as a one-entry
				   list; the list keys win when both are present. */
				const DATA_JSON_VALUE* const onSuccess = boxValue.Find("onSuccessLogicId");
				const DATA_JSON_VALUE* const onTimeout = boxValue.Find("onTimeoutLogicId");
				KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE stagedBox;
				if (nullptr == boxId || nullptr == boxLogicId ||
					nullptr == boxStart ||
					!Try_ParseUnsigned(*boxStart, MAX_TIME_MS, stagedBox.iStartMs) ||
					nullptr == boxDuration ||
					!Try_ParseUnsigned(*boxDuration, MAX_TIME_MS, stagedBox.iDurationMs) ||
					0u == stagedBox.iDurationMs ||
					(nullptr != onSuccess && !onSuccess->Is_String()) ||
					(nullptr != onTimeout && !onTimeout->Is_String()) ||
					!Try_ParseTextList(boxValue.Find("onSuccessLogicIds"),
						KOUKU_SAYDON_MAX_OUTCOMES_PER_SLOT, stagedBox.OnSuccessLogicIds) ||
					!Try_ParseTextList(boxValue.Find("onFailLogicIds"),
						KOUKU_SAYDON_MAX_OUTCOMES_PER_SLOT, stagedBox.OnFailLogicIds) ||
					!Try_ParseTextList(boxValue.Find("onTimeoutLogicIds"),
						KOUKU_SAYDON_MAX_OUTCOMES_PER_SLOT, stagedBox.OnTimeoutLogicIds))
				{
					outStatus = "KoukuSaydon Logic box value or type is invalid.";
					return false;
				}
				stagedBox.strOccurrenceId = boxId->Get_String();
				stagedBox.strLogicId = boxLogicId->Get_String();
				if (const auto* enabled = boxValue.Find("enabled"))
				{
					if (!enabled->Is_Boolean()) { outStatus = "Logic enabled must be boolean."; return false; }
					stagedBox.bEnabled = enabled->Get_Boolean();
				}
				if (nullptr != onSuccess && nullptr == boxValue.Find("onSuccessLogicIds") &&
					!onSuccess->Get_String().empty())
					stagedBox.OnSuccessLogicIds.push_back(onSuccess->Get_String());
				if (nullptr != onTimeout && nullptr == boxValue.Find("onTimeoutLogicIds") &&
					!onTimeout->Get_String().empty())
					stagedBox.OnTimeoutLogicIds.push_back(onTimeout->Get_String());
				stagedPattern.LogicOccurrences.push_back(std::move(stagedBox));
			}
		}
		const DATA_JSON_VALUE* const nextSummonOccurrenceOrdinal =
			patternValue.Find("nextSummonOccurrenceOrdinal");
		const DATA_JSON_VALUE* const summonOccurrences = patternValue.Find("summonOccurrences");
		if ((nullptr != nextSummonOccurrenceOrdinal &&
			 (!Try_ParseUnsigned(*nextSummonOccurrenceOrdinal, MAX_NEXT_ORDINAL,
				stagedPattern.iNextSummonOccurrenceOrdinal) ||
			  0u == stagedPattern.iNextSummonOccurrenceOrdinal)) ||
			(nullptr != summonOccurrences &&
			 (!summonOccurrences->Is_Array() ||
			  summonOccurrences->Get_Array().size() > MAX_SUMMON_OCCURRENCES_PER_PATTERN)))
		{
			outStatus = "KoukuSaydon Pattern Summon counter or box list is invalid.";
			return false;
		}
		if (nullptr != summonOccurrences)
		{
			stagedPattern.SummonOccurrences.reserve(summonOccurrences->Get_Array().size());
			for (const DATA_JSON_VALUE& boxValue : summonOccurrences->Get_Array())
			{
				if (!Has_ExactProperties(boxValue,
						{ "occurrenceId", "summonId", "startMs", "durationMs" }))
				{
					outStatus = "KoukuSaydon Summon box has unexpected properties.";
					return false;
				}
				const DATA_JSON_VALUE* const boxId = Required(boxValue, "occurrenceId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const boxSummonId = Required(boxValue, "summonId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const boxStart = Required(boxValue, "startMs", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* const boxDuration = Required(boxValue, "durationMs", DATA_JSON_TYPE::NUMBER);
				KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE stagedBox;
				if (nullptr == boxId || nullptr == boxSummonId ||
					nullptr == boxStart ||
					!Try_ParseUnsigned(*boxStart, MAX_TIME_MS, stagedBox.iStartMs) ||
					nullptr == boxDuration ||
					!Try_ParseUnsigned(*boxDuration, MAX_TIME_MS, stagedBox.iDurationMs) ||
					0u == stagedBox.iDurationMs)
				{
					outStatus = "KoukuSaydon Summon box value or type is invalid.";
					return false;
				}
				stagedBox.strOccurrenceId = boxId->Get_String();
				stagedBox.strSummonId = boxSummonId->Get_String();
				stagedPattern.SummonOccurrences.push_back(std::move(stagedBox));
			}
		}
		const DATA_JSON_VALUE* const nextWorldOccurrenceOrdinal =
			patternValue.Find("nextWorldOccurrenceOrdinal");
		const DATA_JSON_VALUE* const worldOccurrences = patternValue.Find("worldOccurrences");
		if ((nullptr != nextWorldOccurrenceOrdinal &&
			 (!Try_ParseUnsigned(*nextWorldOccurrenceOrdinal, MAX_NEXT_ORDINAL,
				stagedPattern.iNextWorldOccurrenceOrdinal) ||
			  0u == stagedPattern.iNextWorldOccurrenceOrdinal)) ||
			(nullptr != worldOccurrences &&
			 (!worldOccurrences->Is_Array() ||
			  worldOccurrences->Get_Array().size() > MAX_WORLD_OCCURRENCES_PER_PATTERN)))
		{
			outStatus = "KoukuSaydon Pattern World counter or box list is invalid.";
			return false;
		}
		if (nullptr != worldOccurrences)
		{
			for (const DATA_JSON_VALUE& boxValue : worldOccurrences->Get_Array())
			{
				if (!Has_ExactProperties(boxValue,
						{ "occurrenceId", "worldId", "startMs", "durationMs", "playbackSpeed" }))
				{
					outStatus = "KoukuSaydon World box has unexpected properties.";
					return false;
				}
				const DATA_JSON_VALUE* const boxId = Required(boxValue, "occurrenceId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const boxWorldId = Required(boxValue, "worldId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const boxStart = Required(boxValue, "startMs", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* const boxDuration = Required(boxValue, "durationMs", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* const boxSpeed = Required(boxValue, "playbackSpeed", DATA_JSON_TYPE::NUMBER);
				KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE stagedBox;
				double speed = 1.0;
				if (nullptr == boxId || nullptr == boxWorldId ||
					nullptr == boxStart ||
					!Try_ParseUnsigned(*boxStart, MAX_TIME_MS, stagedBox.iStartMs) ||
					nullptr == boxDuration ||
					!Try_ParseUnsigned(*boxDuration, MAX_TIME_MS, stagedBox.iDurationMs) ||
					0u == stagedBox.iDurationMs || nullptr == boxSpeed ||
					!Try_ParseFinite(*boxSpeed, MIN_WORLD_PLAYBACK_SPEED, MAX_WORLD_PLAYBACK_SPEED, speed))
				{
					outStatus = "KoukuSaydon World box value or type is invalid.";
					return false;
				}
				stagedBox.strOccurrenceId = boxId->Get_String();
				stagedBox.strWorldId = boxWorldId->Get_String();
				stagedBox.fPlaybackSpeed = static_cast<f32_t>(speed);
				stagedPattern.WorldOccurrences.push_back(std::move(stagedBox));
			}
		}
		const DATA_JSON_VALUE* const nextSceneProfileOccurrenceOrdinal =
			patternValue.Find("nextSceneProfileOccurrenceOrdinal");
		const DATA_JSON_VALUE* const sceneProfileOccurrences = patternValue.Find("sceneProfileOccurrences");
		if ((nullptr != nextSceneProfileOccurrenceOrdinal &&
			 (!Try_ParseUnsigned(*nextSceneProfileOccurrenceOrdinal, MAX_NEXT_ORDINAL,
				stagedPattern.iNextSceneProfileOccurrenceOrdinal) ||
			  0u == stagedPattern.iNextSceneProfileOccurrenceOrdinal)) ||
			(nullptr != sceneProfileOccurrences &&
			 (!sceneProfileOccurrences->Is_Array() ||
			  sceneProfileOccurrences->Get_Array().size() > MAX_SCENE_PROFILE_OCCURRENCES_PER_PATTERN)))
		{
			outStatus = "KoukuSaydon Pattern Scene Profile counter or box list is invalid.";
			return false;
		}
		if (nullptr != sceneProfileOccurrences)
		{
			for (const DATA_JSON_VALUE& boxValue : sceneProfileOccurrences->Get_Array())
			{
				if (!Has_ExactProperties(boxValue,
						{ "occurrenceId", "sceneProfileId", "startMs", "durationMs", "blendMs" }))
				{
					outStatus = "KoukuSaydon Scene Profile box has unexpected properties.";
					return false;
				}
				const DATA_JSON_VALUE* const boxId = Required(boxValue, "occurrenceId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const boxProfileId = Required(boxValue, "sceneProfileId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* const boxStart = Required(boxValue, "startMs", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* const boxDuration = Required(boxValue, "durationMs", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* const boxBlend = Required(boxValue, "blendMs", DATA_JSON_TYPE::NUMBER);
				KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE stagedBox;
				if (nullptr == boxId || nullptr == boxProfileId ||
					nullptr == boxStart ||
					!Try_ParseUnsigned(*boxStart, MAX_TIME_MS, stagedBox.iStartMs) ||
					nullptr == boxDuration ||
					!Try_ParseUnsigned(*boxDuration, MAX_TIME_MS, stagedBox.iDurationMs) ||
					0u == stagedBox.iDurationMs || nullptr == boxBlend ||
					!Try_ParseUnsigned(*boxBlend, MAX_TIME_MS, stagedBox.iBlendMs))
				{
					outStatus = "KoukuSaydon Scene Profile box value or type is invalid.";
					return false;
				}
				stagedBox.strOccurrenceId = boxId->Get_String();
				stagedBox.strSceneProfileId = boxProfileId->Get_String();
				stagedPattern.SceneProfileOccurrences.push_back(std::move(stagedBox));
			}
		}
		if (const auto* reset = patternValue.Find("resetBossToSpawn"); nullptr != reset)
		{
			if (!reset->Is_Boolean()) { outStatus = "Invalid resetBossToSpawn."; return false; }
			stagedPattern.bResetBossToSpawn = reset->Get_Boolean();
		}
		if (const auto* ordinal = patternValue.Find("nextPresentationOccurrenceOrdinal"); nullptr != ordinal)
			if (!Try_ParseUnsigned(*ordinal, MAX_NEXT_ORDINAL, stagedPattern.iNextPresentationOccurrenceOrdinal) ||
				stagedPattern.iNextPresentationOccurrenceOrdinal == 0u)
			{ outStatus = "Invalid presentation occurrence counter."; return false; }
		if (const auto* rows = patternValue.Find("presentationOccurrences"); nullptr != rows)
		{
			if (!rows->Is_Array() || rows->Get_Array().size() > 1024u)
			{ outStatus = "Invalid presentation occurrence list."; return false; }
			for (const auto& value : rows->Get_Array())
			{
				KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE row;
				if (!Read_PresentationOccurrence(value, row))
				{ outStatus = "Invalid presentation occurrence value or property."; return false; }
				stagedPattern.PresentationOccurrences.push_back(std::move(row));
			}
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
		// The candidate contains one row. Cross-pattern links are checked once
		// all rows have been parsed, including forward references.
		if (!Validate_Shape(candidate, outStatus, false)) return false;
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
		for (const auto& box : stagedPattern.LogicOccurrences)
			if (parsedOccurrenceIds.contains(box.strOccurrenceId))
			{ outStatus = "Duplicate Logic box ID: " + box.strOccurrenceId; return false; }
		for (const auto& box : stagedPattern.SummonOccurrences)
			if (parsedOccurrenceIds.contains(box.strOccurrenceId))
			{ outStatus = "Duplicate Summon box ID: " + box.strOccurrenceId; return false; }
		for (const auto& box : stagedPattern.WorldOccurrences)
			if (parsedOccurrenceIds.contains(box.strOccurrenceId))
			{ outStatus = "Duplicate World box ID: " + box.strOccurrenceId; return false; }
		for (const auto& box : stagedPattern.SceneProfileOccurrences)
			if (parsedOccurrenceIds.contains(box.strOccurrenceId))
			{ outStatus = "Duplicate Scene Profile box ID: " + box.strOccurrenceId; return false; }
		for (const auto& box : stagedPattern.PresentationOccurrences)
			if (parsedOccurrenceIds.contains(box.strOccurrenceId))
			{ outStatus = "Duplicate presentation box ID: " + box.strOccurrenceId; return false; }
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
		for (const auto& box : stagedPattern.LogicOccurrences)
			parsedOccurrenceIds.insert(box.strOccurrenceId);
		for (const auto& box : stagedPattern.SummonOccurrences)
			parsedOccurrenceIds.insert(box.strOccurrenceId);
		for (const auto& box : stagedPattern.WorldOccurrences)
			parsedOccurrenceIds.insert(box.strOccurrenceId);
		for (const auto& box : stagedPattern.SceneProfileOccurrences)
			parsedOccurrenceIds.insert(box.strOccurrenceId);
		for (const auto& box : stagedPattern.PresentationOccurrences)
			parsedOccurrenceIds.insert(box.strOccurrenceId);
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
	output << std::setprecision(std::numeric_limits<double>::max_digits10);
	const auto textList = [&output](const char_t* const key, const std::vector<std::string>& list,
		const char_t* const indent, const bool_t last)
	{
		output << indent << "\"" << key << "\": [";
		for (std::size_t index = 0u; index < list.size(); ++index)
			output << (0u == index ? "" : ", ") << "\"" << CDataJson::Escape(list[index]) << "\"";
		output << "]" << (last ? "" : ",") << "\n";
	};
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
		<< "  \"nextLogicOrdinal\": " << document.iNextLogicOrdinal << ",\n"
		<< "  \"nextSummonOrdinal\": " << document.iNextSummonOrdinal << ",\n"
		<< "  \"nextWorldOrdinal\": " << document.iNextWorldOrdinal << ",\n"
		<< "  \"nextSceneProfileOrdinal\": " << document.iNextSceneProfileOrdinal << ",\n"
		<< "  \"madnessPolicy\": {\n"
		<< "    \"maximum\": " << document.MadnessPolicy.iMaximum << ",\n"
		<< "    \"clownHoldMs\": " << document.MadnessPolicy.iClownHoldMs << "\n"
		<< "  },\n";
	textList("playAllPatternIds", document.PlayAllPatternIds, "  ", false);
	output << "  \"logics\": [\n";
	for (std::size_t logicIndex = 0u; logicIndex < document.Logics.size(); ++logicIndex)
	{
		const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic = document.Logics[logicIndex];
		output << "    {\n"
			<< "      \"logicId\": \"" << CDataJson::Escape(logic.strLogicId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(logic.strDisplayName) << "\",\n"
			<< "      \"logicType\": \"" << CDataJson::Escape(logic.strLogicType) << "\"";
		/* Only the keys of the definition's kind are written, so a typed file
		   stays exactly what the projector accepts. */
		if ("DURATION" == logic.strLogicType && !logic.strJudgementKind.empty())
		{
			output << ",\n      \"judgementKind\": \"" << CDataJson::Escape(logic.strJudgementKind) << "\"";
			if ("ROULETTE_CARD_MATCH" == logic.strJudgementKind)
			{
				output << ",\n";
				textList("regionIds", logic.RegionIds, "      ", true);
				if (logic.iSectorCount != 0u || !logic.SectorSymbols.empty() || !logic.strWorldSequenceInstanceId.empty())
				{
					output << "      ,\"sectorCount\": " << logic.iSectorCount << ",\n";
					textList("sectorSymbols", logic.SectorSymbols, "      ", true);
					output << "      ,\"centerX\": " << logic.fCenterX
						<< ",\n      \"centerZ\": " << logic.fCenterZ
						<< ",\n      \"outerRadiusM\": " << logic.fOuterRadiusM
						<< ",\n      \"worldSequenceInstanceId\": \""
						<< CDataJson::Escape(logic.strWorldSequenceInstanceId) << "\"";
				}
			}
			else if ("AREA_OVERLAP" == logic.strJudgementKind)
			{
				output << ",\n      \"insideOutcome\": \"" << logic.strInsideOutcome << "\"";
			}
			else if ("GAZE_REAL_BOSS" == logic.strJudgementKind)
			{
				output << ",\n      \"halfAngleDegrees\": " << logic.fHalfAngleDegrees
					<< ",\n      \"maxDistanceM\": " << logic.fMaxDistanceM;
			}
			else if ("POSE_INPUT" == logic.strJudgementKind)
			{
				output << ",\n      \"poseIndex\": " << logic.iPoseIndex;
			}
			else if ("STAGGER_WINDOW" == logic.strJudgementKind)
			{
				output << ",\n      \"threshold\": " << logic.iThreshold
					<< ",\n      \"shieldArcDegrees\": " << logic.fShieldArcDegrees
					<< ",\n      \"endsPatternOnSuccess\": "
					<< (logic.bEndsPatternOnSuccess ? "true" : "false")
					<< ",\n      \"normalYawOffsetDegrees\": " << logic.fNormalYawOffsetDegrees;
			}
		}
		else if ("RESULT" == logic.strLogicType && !logic.strOutcomeKind.empty())
		{
			output << ",\n      \"outcomeKind\": \"" << CDataJson::Escape(logic.strOutcomeKind) << "\""
				<< ",\n      \"percent\": " << logic.iPercent
				<< ",\n      \"durationMs\": " << logic.iDurationMs
				<< ",\n      \"followupPatternId\": \"" << CDataJson::Escape(logic.strFollowupPatternId) << "\"";
		}
		if (logic.strLogicType == "TRIGGER" && !logic.strTriggerKind.empty())
		{
			output << ",\n      \"triggerKind\": \"" << logic.strTriggerKind << "\"";
			if (logic.strTriggerKind == "HUD_ENTER")
				output << ",\n      \"hudMode\": \"" << logic.strHudMode << "\"";
			else if (logic.strTriggerKind == "REAL_GAZE_TELEPORT")
			{
				output << ",\n      \"faceCenterYawOffsetDegrees\": " << logic.fFaceCenterYawOffsetDegrees;
				output << ",\n      \"teleportPosition\": [" << logic.TeleportPosition[0] << ", " << logic.TeleportPosition[1] << ", " << logic.TeleportPosition[2] << "]"
					<< ",\n      \"clonePatternId\": \"" << CDataJson::Escape(logic.strClonePatternId) << "\",\n      \"clockHours\": [";
				for (std::size_t i = 0; i < logic.ClockHours.size(); ++i)
					output << (i ? ", " : "") << logic.ClockHours[i];
				output << "]";
			}
		}
		output << "\n    }" << (logicIndex + 1u < document.Logics.size() ? "," : "") << "\n";
	}
	output << "  ],\n  \"summons\": [\n";
	for (std::size_t summonIndex = 0u; summonIndex < document.Summons.size(); ++summonIndex)
	{
		const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION& summon = document.Summons[summonIndex];
		output << "    {\n"
			<< "      \"summonId\": \"" << CDataJson::Escape(summon.strSummonId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(summon.strDisplayName) << "\"\n"
			<< "    }" << (summonIndex + 1u < document.Summons.size() ? "," : "") << "\n";
	}
	output << "  ],\n  \"worlds\": [\n";
	for (std::size_t worldIndex = 0u; worldIndex < document.Worlds.size(); ++worldIndex)
	{
		const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world = document.Worlds[worldIndex];
		output << "    {\n"
			<< "      \"worldId\": \"" << CDataJson::Escape(world.strWorldId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(world.strDisplayName) << "\",\n"
			<< "      \"sequenceInstanceId\": \"" << CDataJson::Escape(world.strSequenceInstanceId) << "\",\n"
			<< "      \"positionOffset\": [" << world.PositionOffset[0] << ", " << world.PositionOffset[1] << ", " << world.PositionOffset[2] << "],\n"
			<< "      \"anchorKind\": \"" << CDataJson::Escape(world.strAnchorKind) << "\",\n"
			<< "      \"anchorPosition\": [" << world.AnchorPosition[0] << ", " << world.AnchorPosition[1] << ", " << world.AnchorPosition[2] << "],\n"
			<< "      \"companionEffectResourceId\": \"" << CDataJson::Escape(world.strCompanionEffectResourceId) << "\"\n"
			<< "    }" << (worldIndex + 1u < document.Worlds.size() ? "," : "") << "\n";
	}
	output << "  ],\n  \"sceneProfiles\": [\n";
	for (std::size_t profileIndex = 0u; profileIndex < document.SceneProfiles.size(); ++profileIndex)
	{
		const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION& profile = document.SceneProfiles[profileIndex];
		output << "    {\n"
			<< "      \"sceneProfileId\": \"" << CDataJson::Escape(profile.strSceneProfileId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(profile.strDisplayName) << "\",\n"
			<< "      \"renderingProfileId\": \"" << CDataJson::Escape(profile.strRenderingProfileId) << "\"\n"
			<< "    }" << (profileIndex + 1u < document.SceneProfiles.size() ? "," : "") << "\n";
	}
	output << "  ],\n  \"nextPresentationResourceOrdinal\": " << document.iNextPresentationResourceOrdinal
		<< ",\n  \"presentationResources\": [\n";
	for (std::size_t i = 0u; i < document.PresentationResources.size(); ++i)
	{
		const auto& row = document.PresentationResources[i];
		output << "    {\"resourceId\": \"" << CDataJson::Escape(row.strResourceId)
			<< "\", \"displayName\": \"" << CDataJson::Escape(row.strDisplayName)
			<< "\", \"defaultAnchorKind\": \"" << CDataJson::Escape(row.strDefaultAnchorKind)
			<< "\", \"kind\": \"" << Presentation_KindName(row.eKind)
			<< "\", \"assetId\": \"" << CDataJson::Escape(row.strAssetId)
			<< "\", \"resourceKind\": \"" << CDataJson::Escape(row.strResourceKind)
			<< "\", \"durationMs\": " << row.iDurationMs
			<< ", \"shape\": \"" << CDataJson::Escape(row.strShape) << "\", \"colliderKind\": \"" << CDataJson::Escape(row.strColliderKind) << "\", \"halfExtents\": ";
		Write_PresentationVector(output, row.HalfExtents);
		output << ", \"radiusM\": " << row.fRadiusM << ", \"halfAngleDegrees\": " << row.fHalfAngleDegrees
			<< '}' << (i + 1u < document.PresentationResources.size() ? "," : "") << '\n';
	}
	output << "  ],\n  \"patterns\": [\n";
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
			<< "      \"nextLogicOccurrenceOrdinal\": " << pattern.iNextLogicOccurrenceOrdinal << ",\n"
			<< "      \"nextSummonOccurrenceOrdinal\": " << pattern.iNextSummonOccurrenceOrdinal << ",\n"
			<< "      \"nextWorldOccurrenceOrdinal\": " << pattern.iNextWorldOccurrenceOrdinal << ",\n"
			<< "      \"nextSceneProfileOccurrenceOrdinal\": " << pattern.iNextSceneProfileOccurrenceOrdinal << ",\n"
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
					<< "              \"playRate\": " << static_cast<double>(occurrence.fPlayRate) << ",\n"
					<< "              \"endPolicy\": \"" << CDataJson::Escape(occurrence.strEndPolicy) << "\"\n"
					<< "            }" << (occurrenceIndex + 1u < stage.AnimationOccurrences.size() ? "," : "") << "\n";
			}
			output << "          ]\n        }"
				<< (stageIndex + 1u < pattern.Stages.size() ? "," : "") << "\n";
		}
		output << "      ],\n      \"logicOccurrences\": [\n";
		for (std::size_t boxIndex = 0u; boxIndex < pattern.LogicOccurrences.size(); ++boxIndex)
		{
			const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE& box = pattern.LogicOccurrences[boxIndex];
			output << "        {\n"
				<< "          \"occurrenceId\": \"" << CDataJson::Escape(box.strOccurrenceId) << "\",\n"
				<< "          \"logicId\": \"" << CDataJson::Escape(box.strLogicId) << "\",\n"
				<< "          \"startMs\": " << box.iStartMs << ",\n"
				<< "          \"durationMs\": " << box.iDurationMs << ",\n";
			output << "          \"enabled\": " << (box.bEnabled ? "true" : "false") << ",\n";
			textList("onSuccessLogicIds", box.OnSuccessLogicIds, "          ", false);
			textList("onFailLogicIds", box.OnFailLogicIds, "          ", false);
			textList("onTimeoutLogicIds", box.OnTimeoutLogicIds, "          ", true);
			output << "        }" << (boxIndex + 1u < pattern.LogicOccurrences.size() ? "," : "") << "\n";
		}
		output << "      ],\n      \"summonOccurrences\": [\n";
		for (std::size_t boxIndex = 0u; boxIndex < pattern.SummonOccurrences.size(); ++boxIndex)
		{
			const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE& box = pattern.SummonOccurrences[boxIndex];
			output << "        {\n"
				<< "          \"occurrenceId\": \"" << CDataJson::Escape(box.strOccurrenceId) << "\",\n"
				<< "          \"summonId\": \"" << CDataJson::Escape(box.strSummonId) << "\",\n"
				<< "          \"startMs\": " << box.iStartMs << ",\n"
				<< "          \"durationMs\": " << box.iDurationMs << "\n"
				<< "        }" << (boxIndex + 1u < pattern.SummonOccurrences.size() ? "," : "") << "\n";
		}
		output << "      ],\n      \"worldOccurrences\": [\n";
		for (std::size_t boxIndex = 0u; boxIndex < pattern.WorldOccurrences.size(); ++boxIndex)
		{
			const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& box = pattern.WorldOccurrences[boxIndex];
			output << "        {\n"
				<< "          \"occurrenceId\": \"" << CDataJson::Escape(box.strOccurrenceId) << "\",\n"
				<< "          \"worldId\": \"" << CDataJson::Escape(box.strWorldId) << "\",\n"
				<< "          \"startMs\": " << box.iStartMs << ",\n"
				<< "          \"durationMs\": " << box.iDurationMs << ",\n"
				<< "          \"playbackSpeed\": " << static_cast<double>(box.fPlaybackSpeed) << "\n"
				<< "        }" << (boxIndex + 1u < pattern.WorldOccurrences.size() ? "," : "") << "\n";
		}
		output << "      ],\n      \"sceneProfileOccurrences\": [\n";
		for (std::size_t boxIndex = 0u; boxIndex < pattern.SceneProfileOccurrences.size(); ++boxIndex)
		{
			const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE& box = pattern.SceneProfileOccurrences[boxIndex];
			output << "        {\n"
				<< "          \"occurrenceId\": \"" << CDataJson::Escape(box.strOccurrenceId) << "\",\n"
				<< "          \"sceneProfileId\": \"" << CDataJson::Escape(box.strSceneProfileId) << "\",\n"
				<< "          \"startMs\": " << box.iStartMs << ",\n"
				<< "          \"durationMs\": " << box.iDurationMs << ",\n"
				<< "          \"blendMs\": " << box.iBlendMs << "\n"
				<< "        }" << (boxIndex + 1u < pattern.SceneProfileOccurrences.size() ? "," : "") << "\n";
		}
		output << "      ],\n      \"resetBossToSpawn\": " << (pattern.bResetBossToSpawn ? "true" : "false")
			<< ",\n      \"nextPresentationOccurrenceOrdinal\": " << pattern.iNextPresentationOccurrenceOrdinal
			<< ",\n      \"presentationOccurrences\": [\n";
		for (std::size_t i = 0u; i < pattern.PresentationOccurrences.size(); ++i)
		{
			const auto& row = pattern.PresentationOccurrences[i];
			output << "        {\"occurrenceId\": \"" << CDataJson::Escape(row.strOccurrenceId)
				<< "\", \"resourceId\": \"" << CDataJson::Escape(row.strResourceId)
				<< "\", \"startMs\": " << row.iStartMs << ", \"durationMs\": " << row.iDurationMs
				<< ", \"positionOffset\": ";
			Write_PresentationVector(output, row.PositionOffset);
			output << ", \"rotationDegrees\": "; Write_PresentationVector(output, row.RotationDegrees);
			output << ", \"scale\": "; Write_PresentationVector(output, row.Scale);
			output << ", \"fadeInMs\": " << row.iFadeInMs << ", \"fadeOutMs\": " << row.iFadeOutMs
				<< ", \"dissolveStart\": " << row.fDissolveStart << ", \"dissolveEnd\": " << row.fDissolveEnd
				<< ", \"brightnessMultiplier\": " << row.fBrightnessMultiplier
				<< ", \"volume\": " << row.fVolume << ", \"followBoss\": " << (row.bFollowBoss ? "true" : "false")
				<< ", \"debugRender\": " << (row.bDebugRender ? "true" : "false")
				<< ", \"bone\": \"" << CDataJson::Escape(row.strBone) << "\", \"regionId\": \"" << CDataJson::Escape(row.strRegionId)
				<< "\", \"cardSymbol\": \"" << CDataJson::Escape(row.strCardSymbol)
				<< "\", \"cardColor\": \"" << CDataJson::Escape(row.strCardColor)
				<< "\", \"anchorKind\": \"" << CDataJson::Escape(row.strAnchorKind)
				<< "\", \"worldId\": \"" << CDataJson::Escape(row.strWorldId) << "\", \"logicOccurrenceId\": \"" << CDataJson::Escape(row.strLogicOccurrenceId)
				<< "\", \"worldOccurrenceId\": \"" << CDataJson::Escape(row.strWorldOccurrenceId) << "\"}"
				<< (i + 1u < pattern.PresentationOccurrences.size() ? "," : "") << '\n';
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
