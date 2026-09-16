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
	constexpr std::uint32_t FORMAT_VERSION = 3u;
	constexpr std::uint32_t FIXED_TICK_HZ = 30u;
	constexpr std::string_view COMPOSITION_ID =
		"boss.composition.kakulsaydon.gate1";
	constexpr std::string_view SEQUENCE_COMPOSITION_ID =
		"boss.composition.kakulsaydon.sequencer";
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
    constexpr std::size_t MAX_SUMMON_PATTERN_SPAWNS = 4u;
	constexpr std::string_view GENERATED_WORLD_PREFIX = "kakulsaydon.g1.world.";
	constexpr std::size_t MAX_WORLDS = 4096u;
	constexpr std::size_t MAX_WORLD_OCCURRENCES_PER_PATTERN = 128u;
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
	constexpr std::size_t MAX_PATTERN_OCCURRENCES = 128u;
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
		const bool airborne = logic.strLogicType == "TRIGGER" && logic.strTriggerKind == "ALBION_AIRBORNE";
		const bool hasAirborneValues = !logic.strAirbornePhase.empty() || logic.fAirborneHeightM != 0.0 || logic.iAirborneDurationMs != 0u;
		if (hasAirborneValues && !airborne)
		{ outStatus = "Airborne phase values belong to ALBION_AIRBORNE."; return false; }
		const bool_t hasRandomVolleys = !logic.RandomVolleyOccurrenceSets.empty();
		const bool_t hasRandomValues = hasRandomVolleys || logic.iRandomSpawnIntervalMs != 0u ||
			logic.fRandomArenaRadiusM != 0.0 || logic.fRandomArenaHeightToleranceM != 0.0;
		if (hasRandomValues)
		{
			if (!hasRandomVolleys || (logic.strFixedSelectionGroupId.empty() && logic.strTrackingPresentationOccurrenceId.empty()) ||
				logic.RandomVolleyOccurrenceSets.size() > 32u ||
				logic.iRandomSpawnIntervalMs < 1u || logic.iRandomSpawnIntervalMs > MAX_TIME_MS ||
				!std::isfinite(logic.fRandomArenaRadiusM) || logic.fRandomArenaRadiusM <= 0.0 || logic.fRandomArenaRadiusM > 1000.0 ||
				!std::isfinite(logic.fRandomArenaHeightToleranceM) || logic.fRandomArenaHeightToleranceM <= 0.0 || logic.fRandomArenaHeightToleranceM > 10.0)
			{ outStatus = "SHOWTIME random volleys require 1..32 sets, interval 1..600000 ms, radius (0,1000] and height tolerance (0,10]."; return false; }
			for (const auto& volley : logic.RandomVolleyOccurrenceSets)
			{
				std::unordered_set<std::string> ids;
				if (volley.empty() || volley.size() > 64u)
				{ outStatus = "SHOWTIME random volley requires 1..64 Effect occurrence IDs."; return false; }
				for (const auto& id : volley) if (!Is_StableId(id) || !ids.insert(id).second)
				{ outStatus = "SHOWTIME random volley has an invalid or duplicate occurrence ID."; return false; }
			}
		}
		const bool_t hasShowtimeValues = hasRandomValues || !logic.strFixedSelectionGroupId.empty() ||
			!logic.strTrackingPresentationOccurrenceId.empty() || logic.iSpawnIntervalMs != 0u || logic.fFollowSpeedScale != 0.0;
		const bool_t showtime = logic.strLogicType == "DURATION" && logic.strJudgementKind == "SHOWTIME_PLAYER_TARGETS";
		if ((hasShowtimeValues && !showtime) || (showtime &&
			((!logic.strFixedSelectionGroupId.empty() && !Is_StableId(logic.strFixedSelectionGroupId)) ||
			 (!logic.strTrackingPresentationOccurrenceId.empty() && !Is_StableId(logic.strTrackingPresentationOccurrenceId)) ||
			 logic.iSpawnIntervalMs < 1u || logic.iSpawnIntervalMs > MAX_TIME_MS ||
			 !std::isfinite(logic.fFollowSpeedScale) || logic.fFollowSpeedScale < .01 || logic.fFollowSpeedScale > 10.0)))
		{ outStatus = "SHOWTIME_PLAYER_TARGETS requires stable template references, interval 1..600000 ms and speed scale .01..10."; return false; }
		const bool cross = logic.strLogicType == "DURATION" && logic.strJudgementKind == "CROSS_DIRECTION_CLONES";
        std::unordered_set<std::string> directionIds;
        if ((cross && (logic.DirectionPatternIds.size() != 4u || !Is_StableId(logic.strCloneEndStageId) || !Is_StableId(logic.strSummonOccurrenceId) ||
            !std::all_of(logic.DirectionPatternIds.begin(), logic.DirectionPatternIds.end(),
                [&](const auto& id) { return Is_StableId(id) && directionIds.insert(id).second; }))) ||
            (!cross && (!logic.DirectionPatternIds.empty() || !logic.strCloneEndStageId.empty() || !logic.strSummonOccurrenceId.empty())))
        { outStatus = "Cross direction Logic needs four unique Pattern IDs, a clone end Stage ID and its Summon occurrence."; return false; }
		const bool_t hasDurationValues = !logic.DirectionPatternIds.empty() || !logic.strCloneEndStageId.empty() || !logic.strSummonOccurrenceId.empty() || hasShowtimeValues || !logic.PatternIds.empty() || logic.iCompletionCount != 0u || !logic.strJudgementKind.empty() ||
			0u != logic.iSectorCount || !logic.SectorSymbols.empty() || !logic.RegionIds.empty() ||
			0.0 != logic.fCenterX || 0.0 != logic.fCenterZ || 0.0 != logic.fOuterRadiusM ||
			!logic.strWorldSequenceInstanceId.empty() || 0.0 != logic.fHalfAngleDegrees ||
			0.0 != logic.fMaxDistanceM || 0u != logic.iPoseIndex || 0u != logic.iThreshold ||
			0.0 != logic.fShieldArcDegrees || logic.bEndsPatternOnSuccess || logic.fNormalYawOffsetDegrees != 0.0 || logic.strInsideOutcome != "SUCCESS" || (logic.fTargetRadiusM != 0.0 && logic.strTriggerKind != "OBJECT_CONTACT");
		if ((logic.strInsideOutcome != "SUCCESS" && logic.strInsideOutcome != "FAIL") ||
			(logic.strInsideOutcome != "SUCCESS" && (logic.strLogicType != "DURATION" || (logic.strJudgementKind != "AREA_OVERLAP" && logic.strJudgementKind != "OBJECT_OVERLAP" && logic.strJudgementKind != "GAZE_REAL_BOSS"))))
		{ outStatus = "Only AREA_OVERLAP/OBJECT_OVERLAP/GAZE_REAL_BOSS takes an inside outcome of SUCCESS or FAIL: " + logic.strLogicId; return false; }
		const bool_t hasResultValues = !logic.strOutcomeKind.empty() ||
			0u != logic.iPercent || 0u != logic.iDurationMs || logic.fPushRangeM != 0.0 || logic.iPushMs != 0u || !logic.strFollowupPatternId.empty() ||
			(!logic.strTargetWorldInstanceId.empty() && logic.strJudgementKind != "OBJECT_OVERLAP") || !logic.strMotionInstanceId.empty() ||
			!logic.strSceneProfileId.empty() || !logic.strEffectResourceId.empty() || !logic.strLightResourceId.empty() || logic.iEffectDelayMs != 0u ||
			!logic.strAttachmentSlot.empty() || logic.GripLocalOffset != std::array<double, 3u>{} ||
			!logic.ContactMotions.empty() || !logic.strTargetLogicOccurrenceId.empty() || !logic.strContactTargetWorldOccurrenceId.empty();
		const bool_t hasContactValues = !logic.TargetWorldOccurrenceIds.empty() || !logic.strContactGroupId.empty() || logic.iContactPriority != 0u;
		if (hasContactValues && (logic.strLogicType != "TRIGGER" || logic.strTriggerKind != "OBJECT_CONTACT"))
		{ outStatus = "Only OBJECT_CONTACT carries target placements and contact priority."; return false; }
        if (!std::isfinite(logic.fBossChargeDistanceM) || logic.fBossChargeDistanceM < 0.0 || logic.fBossChargeDistanceM > 1000.0 ||
            !std::isfinite(logic.fChargeYawOffsetDegrees) || std::abs(logic.fChargeYawOffsetDegrees) > 360.0 ||
            (logic.fChargeYawOffsetDegrees != 0.0 && logic.fBossChargeDistanceM <= 0.0) ||
            (logic.fBossChargeDistanceM != 0.0 && (logic.strLogicType != "TRIGGER" || logic.strTriggerKind != "ENTER_AREA")))
        { outStatus = "Only ENTER_AREA accepts a boss charge distance of 0..1000 m and charge yaw of -360..360 degrees; nonzero yaw requires positive distance."; return false; }
		if (((logic.bRearmOnExit || logic.bRepeatAfterKnockback) && (logic.strLogicType != "TRIGGER" || logic.strTriggerKind != "ENTER_AREA")) ||
			(logic.bRearmOnExit && logic.bRepeatAfterKnockback))
		{ outStatus = "ENTER_AREA accepts one contact repeat policy."; return false; }
		if (!std::isfinite(logic.fPushRangeM) || logic.fPushRangeM < 0.0 || logic.fPushRangeM > 20.0 ||
			(logic.strPushDirection != "AWAY_FROM_BOSS" && logic.strPushDirection != "BOSS_FORWARD") ||
			(logic.strPushDirection == "BOSS_FORWARD" && logic.fPushRangeM <= 0.0) ||
			logic.iPushMs > MAX_TIME_MS || ((logic.fPushRangeM == 0.0) != (logic.iPushMs == 0u)) ||
			((logic.fPushRangeM != 0.0 || logic.iPushMs != 0u) &&
			 (logic.strLogicType != "RESULT" || logic.strOutcomeKind != "MAX_HP_PERCENT_DAMAGE")))
		{ outStatus = "Damage knockback needs both a distance of 0..20 m and time of 0..600000 ms; zero disables both."; return false; }
		const bool_t hasPlayerEffectValues = logic.iCountPerPlayer != 0u || logic.fPlayerEffectRadiusM != 0.0 || logic.iEffectLifetimeMs != 0u ||
			logic.iArenaRandomCount != 0u || logic.fArenaRandomRadiusM != 0.0 || logic.fArenaHeightToleranceM != 0.0 ||
			logic.fArenaMinimumSpacingM != 0.0 || logic.bRandomPlayerOnly;
		if (hasPlayerEffectValues && (logic.strLogicType != "TRIGGER" || logic.strTriggerKind != "ALBION_BLUE_CIRCLE"))
		{ outStatus = "Player effect layout belongs to ALBION_BLUE_CIRCLE."; return false; }
		const bool_t hasTriggerValues = hasPlayerEffectValues || logic.bRearmOnExit || logic.bRepeatAfterKnockback || logic.fBossChargeDistanceM != 0.0 || hasContactValues || !logic.strTriggerKind.empty() || !logic.strHudMode.empty() ||
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
			const bool_t chain = kind == "PATTERN_COMPLETION_COUNT";
			std::unordered_set<std::string> chainIds;
			if ((chain && (logic.PatternIds.empty() || logic.PatternIds.size() > 16u || logic.iCompletionCount < 1u ||
				logic.iCompletionCount > logic.PatternIds.size() || !std::all_of(logic.PatternIds.begin(), logic.PatternIds.end(),
				[&](const auto& id) { return Is_StableId(id) && chainIds.insert(id).second; }))) ||
				(!chain && (!logic.PatternIds.empty() || logic.iCompletionCount)))
			{ outStatus = "Pattern completion count requires 1..16 unique candidate IDs and count within the pool."; return false; }
			const bool_t roulette = "ROULETTE_CARD_MATCH" == kind;
			const bool_t gaze = "GAZE_REAL_BOSS" == kind;
			const bool_t pose = "POSE_INPUT" == kind;
			const bool_t stagger = "STAGGER_WINDOW" == kind;
			const bool_t objectOverlap = "OBJECT_OVERLAP" == kind;
			if ((objectOverlap && (!Is_StableId(logic.strTargetWorldInstanceId) || !std::isfinite(logic.fTargetRadiusM) ||
				logic.fTargetRadiusM < .01 || logic.fTargetRadiusM > 1000.0)) || (!objectOverlap && logic.fTargetRadiusM != 0.0))
			{ outStatus = "OBJECT_OVERLAP needs a saved target instance and a radius of .01..1000 m."; return false; }
			const bool_t foreignValues =
				(!roulette && (0u != logic.iSectorCount || !logic.SectorSymbols.empty() || !logic.RegionIds.empty() ||
					0.0 != logic.fCenterX || 0.0 != logic.fCenterZ || 0.0 != logic.fOuterRadiusM ||
					!logic.strWorldSequenceInstanceId.empty())) ||
				(!gaze && (0.0 != logic.fHalfAngleDegrees || 0.0 != logic.fMaxDistanceM)) ||
				(!pose && 0u != logic.iPoseIndex) ||
				(!stagger && (0u != logic.iThreshold || 0.0 != logic.fShieldArcDegrees ||
					(logic.bEndsPatternOnSuccess && kind != "EXTERNAL_SIGNAL" && kind != "COUNTER_WINDOW") || logic.fNormalYawOffsetDegrees != 0.0));
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
			const bool_t capture = kind == "CAPTURE_PLAYER";
			if ((capture && (logic.strAttachmentSlot != "BOSS_LEFT_HAND" ||
				!std::all_of(logic.GripLocalOffset.begin(), logic.GripLocalOffset.end(),
					[](double value) { return std::isfinite(value) && std::abs(value) <= 10.0; }))) ||
				(!capture && (!logic.strAttachmentSlot.empty() || logic.GripLocalOffset != std::array<double, 3u>{})))
			{ outStatus = "CAPTURE_PLAYER owns BOSS_LEFT_HAND and finite grip offsets of -10..10 m."; return false; }
			const bool_t percentKind = "MAX_HP_PERCENT_DAMAGE" == kind ||
				"MADNESS_GAUGE_ADD_PERCENT" == kind;
			const bool_t followup = "FOLLOWUP_PATTERN" == kind;
			const bool_t worldMotion = "PLAY_WORLD_OBJECT_MOTION" == kind;
			if ((worldMotion && (!Is_StableId(logic.strTargetWorldInstanceId) || !Is_StableId(logic.strMotionInstanceId))) ||
				(!worldMotion && (!logic.strTargetWorldInstanceId.empty() || !logic.strMotionInstanceId.empty())))
			{
				outStatus = "A World Object motion outcome needs its target and saved motion instance IDs: " + logic.strLogicId;
				return false;
			}
			const bool contactMotion = kind == "PLAY_CONTACT_WORLD_OBJECT_MOTION";
			const bool completeWindow = kind == "COMPLETE_LOGIC_WINDOW";
			std::unordered_set<std::string> contactIds;
			if ((contactMotion && (logic.ContactMotions.empty() || logic.ContactMotions.size() > 64u ||
				!std::all_of(logic.ContactMotions.begin(), logic.ContactMotions.end(), [&](const auto& row) {
					return Is_StableId(row.strTargetWorldOccurrenceId) && Is_StableId(row.strMotionInstanceId) &&
						contactIds.insert(row.strTargetWorldOccurrenceId).second; }))) ||
				(!contactMotion && !logic.ContactMotions.empty()))
			{ outStatus = "Contact motion Result needs 1..64 unique target placement to saved motion mappings."; return false; }
			if ((completeWindow && (!Is_StableId(logic.strTargetLogicOccurrenceId) ||
				(!logic.strContactTargetWorldOccurrenceId.empty() && !Is_StableId(logic.strContactTargetWorldOccurrenceId)))) ||
				(!completeWindow && (!logic.strTargetLogicOccurrenceId.empty() || !logic.strContactTargetWorldOccurrenceId.empty())))
			{ outStatus = "Complete window Result needs a target Logic window and an optional contact target placement."; return false; }
			if (logic.iPercent > 100u || (percentKind && 0u == logic.iPercent) ||
				(!percentKind && 0u != logic.iPercent))
			{
				outStatus = "A percent outcome needs 1..100 percent and no other outcome takes one: " + logic.strLogicId;
				return false;
			}
			if (("CLOWN_TRANSFORM" != kind && "FEAR" != kind && 0u != logic.iDurationMs) ||
				logic.iDurationMs > MAX_TIME_MS)
			{
				outStatus = "Only clown transform and fear take a duration: " + logic.strLogicId;
				return false;
			}
			if ((kind == "FEAR" && (logic.iDurationMs == 0u || logic.iEffectDelayMs >= logic.iDurationMs ||
                (!logic.strSceneProfileId.empty() && !Is_StableId(logic.strSceneProfileId)) ||
                (!logic.strEffectResourceId.empty() && !Is_StableId(logic.strEffectResourceId)) ||
                (!logic.strLightResourceId.empty() && !Is_StableId(logic.strLightResourceId)) ||
                (logic.strEffectResourceId.empty() && logic.iEffectDelayMs != 0u))) ||
                (kind != "FEAR" && (!logic.strSceneProfileId.empty() || !logic.strEffectResourceId.empty() || !logic.strLightResourceId.empty() || logic.iEffectDelayMs != 0u)))
            { outStatus = "FEAR requires a duration and valid optional Scene Profile/Effect references and delay."; return false; }
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
		else if (logic.strTriggerKind == "ANIMATION_BLEND" || logic.strTriggerKind == "ROOM_PLAYER_ARRIVAL")
		{
			if (hasPlayerEffectValues || hasContactValues || logic.bRearmOnExit || logic.bRepeatAfterKnockback ||
				logic.fBossChargeDistanceM != 0.0 || !logic.strHudMode.empty() || !logic.strClonePatternId.empty() ||
				!logic.ClockHours.empty() || logic.fFaceCenterYawOffsetDegrees != 0.0 ||
				logic.TeleportPosition != std::array<double, 3>{})
			{ outStatus = "This Trigger carries only its triggerKind; its Logic box owns the values."; return false; }
		}
		else if (airborne)
		{
			const auto& phase = logic.strAirbornePhase;
			if ((phase != "SELECT_PLAYER" && phase != "JUMP" && phase != "APPEAR_PLAYER" &&
				 phase != "DISAPPEAR" && phase != "CENTER" && phase != "SLAM") ||
				!std::isfinite(logic.fAirborneHeightM) || logic.fAirborneHeightM < 0.0 || logic.fAirborneHeightM > 100000.0 ||
				logic.iAirborneDurationMs > MAX_TIME_MS ||
				(phase == "JUMP" && (logic.fAirborneHeightM <= 0.0 || logic.iAirborneDurationMs == 0u)) ||
				(phase == "APPEAR_PLAYER" && logic.fAirborneHeightM <= 0.0) ||
				(phase != "JUMP" && logic.iAirborneDurationMs != 0u) ||
				(phase != "JUMP" && phase != "APPEAR_PLAYER" && logic.fAirborneHeightM != 0.0) ||
				!std::all_of(logic.TeleportPosition.begin(), logic.TeleportPosition.end(), [](double v) { return std::isfinite(v) && std::abs(v) <= 100000.0; }) ||
				(phase != "CENTER" && logic.TeleportPosition != std::array<double, 3>{}) ||
				hasPlayerEffectValues || hasContactValues || logic.bRearmOnExit || logic.bRepeatAfterKnockback ||
				logic.fBossChargeDistanceM != 0.0 || !logic.strHudMode.empty() || !logic.strClonePatternId.empty() ||
				!logic.ClockHours.empty() || logic.fFaceCenterYawOffsetDegrees != 0.0)
			{ outStatus = "ALBION_AIRBORNE needs a valid phase, phase-specific height/duration, and CENTER-only finite XYZ."; return false; }
		}
		else if (logic.strTriggerKind == "ALBION_BLUE_CIRCLE")
		{
			if (logic.iCountPerPlayer < 1u || logic.iCountPerPlayer > 8u ||
				!std::isfinite(logic.fPlayerEffectRadiusM) || logic.fPlayerEffectRadiusM < 0.0 || logic.fPlayerEffectRadiusM > 20.0 ||
				((logic.iCountPerPlayer == 1u) != (logic.fPlayerEffectRadiusM == 0.0)) ||
				logic.iEffectLifetimeMs < 1u || logic.iEffectLifetimeMs > MAX_TIME_MS ||
				!logic.strHudMode.empty() || !logic.strClonePatternId.empty() || !logic.ClockHours.empty() ||
				logic.fFaceCenterYawOffsetDegrees != 0.0 || logic.TeleportPosition != std::array<double, 3>{})
			{ outStatus = "Albion needs 1..8 per player, radius 0 for one or (0,20] for a ring, and effect lifetime 1..600000 ms."; return false; }
			if (logic.iArenaRandomCount > 32u ||
				!std::isfinite(logic.fArenaRandomRadiusM) || logic.fArenaRandomRadiusM < 0.0 || logic.fArenaRandomRadiusM > 100.0 ||
				!std::isfinite(logic.fArenaHeightToleranceM) || logic.fArenaHeightToleranceM < 0.0 || logic.fArenaHeightToleranceM > 10.0 ||
				!std::isfinite(logic.fArenaMinimumSpacingM) || logic.fArenaMinimumSpacingM < 0.0 || logic.fArenaMinimumSpacingM > 20.0 ||
				(logic.iArenaRandomCount == 0u && (logic.fArenaRandomRadiusM != 0.0 || logic.fArenaHeightToleranceM != 0.0 || logic.fArenaMinimumSpacingM != 0.0)) ||
				(logic.iArenaRandomCount > 0u && (logic.fArenaRandomRadiusM <= 0.0 || logic.fArenaHeightToleranceM <= 0.0 || logic.fArenaMinimumSpacingM <= 0.0)) ||
				(logic.bRandomPlayerOnly && (logic.iCountPerPlayer != 1u || logic.fPlayerEffectRadiusM != 0.0)))
			{ outStatus = "Albion accepts 0..32 arena circles; enabled placement needs radius (0,100], height tolerance (0,10] and spacing (0,20] m. Disabled placement uses zeros. One random player requires one circle at that player's position."; return false; }
		}
		else if (logic.strTriggerKind == "OBJECT_CONTACT")
		{
			std::unordered_set<std::string> ids;
			if (logic.TargetWorldOccurrenceIds.empty() || logic.TargetWorldOccurrenceIds.size() > 64u ||
				!std::all_of(logic.TargetWorldOccurrenceIds.begin(), logic.TargetWorldOccurrenceIds.end(),
					[&](const auto& id) { return Is_StableId(id) && ids.insert(id).second; }) ||
				!std::isfinite(logic.fTargetRadiusM) || logic.fTargetRadiusM < .01 || logic.fTargetRadiusM > 1000.0 ||
				(!logic.strContactGroupId.empty() && !Is_StableId(logic.strContactGroupId)) || logic.iContactPriority > 1000u ||
				!logic.strHudMode.empty() || !logic.strClonePatternId.empty() || !logic.ClockHours.empty() ||
				logic.fFaceCenterYawOffsetDegrees != 0.0 ||
				std::any_of(logic.TeleportPosition.begin(), logic.TeleportPosition.end(), [](double x) { return x != 0.0; }))
			{ outStatus = "OBJECT_CONTACT needs 1..64 unique WORLD placements, radius .01..1000 and priority 0..1000."; return false; }
		}
		else if (logic.strTriggerKind == "ENTER_AREA")
		{
			if (!logic.strHudMode.empty() || !logic.strClonePatternId.empty() || !logic.ClockHours.empty() ||
				logic.fFaceCenterYawOffsetDegrees != 0.0 ||
				std::any_of(logic.TeleportPosition.begin(), logic.TeleportPosition.end(), [](double x) { return x != 0.0; }))
			{ outStatus = "ENTER_AREA geometry belongs to its linked Collider occurrence."; return false; }
		}
		else if (logic.strTriggerKind == "CARD_MAZE_HIDE_NEXT" || logic.strTriggerKind == "CARD_MAZE_ENTER" || logic.strTriggerKind == "BOSS_TELEPORT_XZ")
		{
			if (!logic.strHudMode.empty() || !logic.strClonePatternId.empty() || !logic.ClockHours.empty() ||
				logic.fFaceCenterYawOffsetDegrees != 0.0 ||
				!std::all_of(logic.TeleportPosition.begin(), logic.TeleportPosition.end(), [](double x) { return std::isfinite(x) && std::abs(x) <= 100000.0; }) ||
				(logic.strTriggerKind == "CARD_MAZE_HIDE_NEXT" &&
				 std::any_of(logic.TeleportPosition.begin(), logic.TeleportPosition.end(), [](double x) { return x != 0.0; })))
			{ outStatus = "Hide has no values; maze entry and boss XZ teleport accept only finite bounded teleportPosition."; return false; }
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
		if (path.empty() || (path.filename() != L"KoukuSaydonComposition.json" &&
			path.filename() != L"KoukuSaydonSequenceComposition.json"))
			return false;
		for (const std::filesystem::path& component : path)
		{
			if (component == L"." || component == L"..")
				return false;
		}
		return true;
	}

	bool_t Matches_CompositionPath(const std::filesystem::path& path,
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, std::string& outStatus)
	{
		const auto expectedId = path.filename() == L"KoukuSaydonSequenceComposition.json" ?
			SEQUENCE_COMPOSITION_ID : COMPOSITION_ID;
		if (!Is_ExpectedCompositionPath(path) || document.strCompositionId != expectedId)
		{
			outStatus = "Composition identity does not match its Action or Sequence authoring path.";
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

	bool_t Read_PresentationIndex(const DATA_JSON_VALUE& object, const char* key,
		std::uint32_t& value, const std::uint32_t maximum)
	{
		const auto* field = object.Find(key);
		return nullptr == field || Try_ParseUnsigned(*field, maximum, value);
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
			{ "resourceKind", "elementId", "durationMs", "shape", "halfExtents", "radiusM", "halfAngleDegrees", "colliderKind", "defaultAnchorKind" }))
			return false;
		std::string kind;
		if (!Read_PresentationText(value, "resourceId", row.strResourceId, true) ||
			!Read_PresentationText(value, "displayName", row.strDisplayName, true) ||
			!Read_PresentationText(value, "kind", kind, true) ||
			!Read_PresentationText(value, "assetId", row.strAssetId, true) ||
			!Read_PresentationText(value, "resourceKind", row.strResourceKind) ||
			!Read_PresentationText(value, "elementId", row.strElementId) ||
			!Read_PresentationText(value, "defaultAnchorKind", row.strDefaultAnchorKind) ||
			!Read_PresentationText(value, "shape", row.strShape) ||
			!Read_PresentationText(value, "colliderKind", row.strColliderKind) ||
			!Read_PresentationTime(value, "durationMs", row.iDurationMs) ||
			!Read_PresentationVector(value, "halfExtents", row.HalfExtents, 0.001, 10000.0) ||
			!Read_PresentationNumber(value, "radiusM", row.fRadiusM, 0.001, 10000.0) ||
			!Read_PresentationNumber(value, "halfAngleDegrees", row.fHalfAngleDegrees, row.strShape == "REVERSE_SECTOR" ? 0.0 : 0.001, 180.0)) return false;
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
			  "dissolveStart", "dissolveEnd", "volume", "followBoss", "bone", "boneTarget",
			  "regionId", "cardSymbol", "cardColor", "anchorKind", "worldId", "logicOccurrenceId", "debugRender", "worldOccurrenceId", "brightnessMultiplier",
			  "worldEmissionIndex", "selectionGroupId", "fitEffectToDuration" })) return false;
		if (const auto* fit = value.Find("fitEffectToDuration"))
		{
			if (!fit->Is_Boolean()) return false;
			row.bFitEffectToDuration = fit->Get_Boolean();
		}
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
			Read_PresentationText(value, "selectionGroupId", row.strSelectionGroupId) &&
			Read_PresentationText(value, "bone", row.strBone) &&
			Read_PresentationText(value, "boneTarget", row.strBoneTarget) &&
			Read_PresentationText(value, "regionId", row.strRegionId) &&
			Read_PresentationText(value, "cardSymbol", row.strCardSymbol) &&
			Read_PresentationText(value, "cardColor", row.strCardColor) &&
			Read_PresentationText(value, "anchorKind", row.strAnchorKind) &&
			Read_PresentationText(value, "worldId", row.strWorldId) &&
			Read_PresentationText(value, "logicOccurrenceId", row.strLogicOccurrenceId) &&
			Read_PresentationText(value, "worldOccurrenceId", row.strWorldOccurrenceId) &&
			Read_PresentationIndex(value, "worldEmissionIndex", row.iWorldEmissionIndex, 127u) &&
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


    bool_t Validate_Folder(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_FOLDER& folder, std::string& status)
    {
        if (!folder.strLoadError.empty())
        {
            DATA_JSON_VALUE preserved;
            return Is_StableId(folder.strFolderId) && !folder.strPreservedJson.empty() &&
                CDataJson::Parse(folder.strPreservedJson, preserved, status);
        }
        if (!Is_StableId(folder.strFolderId) || !Try_ParseGeneratedOrdinal(folder.strFolderId,
            "kakulsaydon.folder.", document.iNextFolderOrdinal) ||
            !CKoukuSaydonCompositionDocument::Is_KnownGate(folder.strGateId) || !Is_DisplayName(folder.strDisplayName))
        { status = "Invalid folder identity, Gate or display name: " + folder.strFolderId; return false; }
        if (!folder.strTimelinePatternId.empty())
        {
            const auto target = std::find_if(document.Patterns.begin(), document.Patterns.end(),
                [&](const auto& row) { return row.strPatternId == folder.strTimelinePatternId; });
            if (!Is_StableId(folder.strTimelinePatternId) || target == document.Patterns.end() ||
                target->strGateId != folder.strGateId || target->strFolderId != folder.strFolderId)
            { status = "Parent timeline requires a same-folder, same-Gate Pattern: " + folder.strFolderId; return false; }
        }
        return true;
    }

    bool_t Validate_PatternFolder(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, std::string& status)
    {
        if (!pattern.strLoadError.empty() || pattern.strFolderId.empty()) return true;
        const auto folder = std::find_if(document.Folders.begin(), document.Folders.end(),
            [&](const auto& row) { return row.strFolderId == pattern.strFolderId; });
        if (!Is_StableId(pattern.strFolderId) || folder == document.Folders.end() ||
            !folder->strLoadError.empty() || folder->strGateId != pattern.strGateId)
        {
            status = "Pattern " + pattern.strPatternId + " requires a valid same-Gate Parent: " + pattern.strFolderId;
            return false;
        }
        return true;
    }

    std::uint64_t Pattern_Lifetime(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
    {
        if (pattern.iDurationMs) return pattern.iDurationMs;
        std::uint64_t duration = 0u;
        for (const auto& stage : pattern.Stages) duration += stage.iDurationMs;
        return duration;
    }

    bool_t Validate_SummonPolicy(const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION& summon,
        std::string& status)
    {
        const bool cross = summon.strSummonKind == "CROSS_DIRECTION_CLONES";
        std::unordered_set<std::string> ids;
        if ((!summon.strSummonKind.empty() && !cross) ||
            (cross && (summon.DirectionPatternIds.size() != 4u || !Is_StableId(summon.strCloneEndStageId) ||
                !std::all_of(summon.DirectionPatternIds.begin(), summon.DirectionPatternIds.end(),
                    [&](const auto& id) { return Is_StableId(id) && ids.insert(id).second; }))) ||
            (!cross && (!summon.DirectionPatternIds.empty() || !summon.strCloneEndStageId.empty())))
        {
            status = "Cross direction Summon needs four unique Pattern IDs and a clone end Stage ID: " + summon.strSummonId;
            return false;
        }
        return true;
    }

    bool_t Resolve_CrossDirectionWindows(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
        std::vector<KOUKU_SAYDON_COMPOSITION_CROSS_DIRECTION_WINDOW>& windows, std::string& status)
    {
        std::vector<KOUKU_SAYDON_COMPOSITION_CROSS_DIRECTION_WINDOW> staged;
        const auto findSummon = [&](const std::string& id) {
            return std::find_if(document.Summons.begin(), document.Summons.end(),
                [&](const auto& row) { return row.strSummonId == id; });
        };
        for (const auto& box : pattern.SummonOccurrences)
        {
            const auto summon = findSummon(box.strSummonId);
            if (summon == document.Summons.end())
            { status = "Summon definition is missing: " + box.strSummonId; return false; }
            if (!Validate_SummonPolicy(*summon, status)) return false;
            if (summon->strSummonKind != "CROSS_DIRECTION_CLONES") continue;
            if (!box.PatternSpawns.empty())
            { status = "A cross direction Summon cannot also own independent Pattern spawns: " + box.strOccurrenceId; return false; }
            staged.push_back({ box.strOccurrenceId, box.iStartMs, box.iDurationMs,
                summon->DirectionPatternIds, summon->strCloneEndStageId });
        }
        for (const auto& box : pattern.LogicOccurrences)
        {
            if (!box.bEnabled) continue;
            const auto logic = std::find_if(document.Logics.begin(), document.Logics.end(),
                [&](const auto& row) { return row.strLogicId == box.strLogicId; });
            if (logic == document.Logics.end() || logic->strJudgementKind != "CROSS_DIRECTION_CLONES") continue;
            if (!Validate_LogicDefinitionValues(*logic, status)) return false;
            const auto summon = std::find_if(pattern.SummonOccurrences.begin(), pattern.SummonOccurrences.end(),
                [&](const auto& row) { return row.strOccurrenceId == logic->strSummonOccurrenceId; });
            if (summon == pattern.SummonOccurrences.end() || !summon->PatternSpawns.empty() ||
                summon->iStartMs != box.iStartMs ||
                std::uint64_t(summon->iStartMs) + summon->iDurationMs < std::uint64_t(box.iStartMs) + box.iDurationMs)
            { status = "Cross direction Logic needs a matching named Summon covering its duration."; return false; }
            const auto definition = findSummon(summon->strSummonId);
            if (definition == document.Summons.end() || !definition->strSummonKind.empty())
            { status = "Cross direction Logic cannot also control a typed Summon."; return false; }
            staged.push_back({ box.strOccurrenceId, box.iStartMs, box.iDurationMs,
                logic->DirectionPatternIds, logic->strCloneEndStageId });
        }
        windows = std::move(staged);
        return true;
    }

    bool_t Validate_CrossDirectionWindows(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, std::string& status)
    {
        const auto fail = [&](const std::string& reason) {
            status = "Cross direction Summon/Logic in " + pattern.strPatternId + ": " + reason; return false; };
        const auto findLogic = [&](const std::string& id) -> const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION* {
            const auto row = std::find_if(document.Logics.begin(), document.Logics.end(),
                [&](const auto& value) { return value.strLogicId == id; });
            return row == document.Logics.end() ? nullptr : &*row;
        };
        std::vector<KOUKU_SAYDON_COMPOSITION_CROSS_DIRECTION_WINDOW> resolved;
        if (!Resolve_CrossDirectionWindows(document, pattern, resolved, status)) return false;
        std::vector<std::pair<std::uint64_t, std::uint64_t>> windows;
        for (const auto& box : resolved)
        {
            const std::uint64_t end = std::uint64_t(box.iStartMs) + box.iDurationMs;
            if (!box.iDurationMs || end > Pattern_Lifetime(pattern) || pattern.BossMotion)
                return fail("its window must fit the Parent and cannot share absolute Boss Motion.");
            for (const auto& id : box.DirectionPatternIds)
            {
                const auto child = std::find_if(document.Patterns.begin(), document.Patterns.end(),
                    [&](const auto& row) { return row.strPatternId == id; });
                if (child == document.Patterns.end() || !child->strLoadError.empty() || id == pattern.strPatternId ||
                    child->strGateId != pattern.strGateId || child->strActorProfileId != pattern.strActorProfileId ||
                    child->strTargetBossPlacementId != pattern.strTargetBossPlacementId || child->strCategory != "MECHANIC" ||
                    !child->PatternOccurrences.empty() || !child->LogicOccurrences.empty() ||
                    !child->SummonOccurrences.empty() || !child->WorldOccurrences.empty() ||
                    !child->SceneProfileOccurrences.empty() || child->BossMotion || child->bResetBossToSpawn ||
                    child->ResetBossYawDegrees || child->bEnterCombatOnFinish ||
                    std::any_of(document.Folders.begin(), document.Folders.end(), [&](const auto& folder) {
                        return folder.strTimelinePatternId == id; }))
                    return fail("each direction must be a leaf animation/effect Pattern for the same actor, Gate and boss: " + id);
                if (!child->iDurationMs || Pattern_Lifetime(*child) > box.iDurationMs)
                    return fail("each direction needs an explicit Duration within the Summon/Logic window: " + id);
                const auto cutoff = std::find_if(child->Stages.begin(), child->Stages.end(),
                    [&](const auto& stage) { return stage.strStageId == box.strCloneEndStageId; });
                if (cutoff == child->Stages.end() || std::next(cutoff) == child->Stages.end() || cutoff->AnimationOccurrences.empty() ||
                    std::any_of(child->Stages.begin(), child->Stages.end(), [](const auto& stage) { return stage.bRetargetOnEnter; }))
                    return fail("each direction needs the clone end Stage and cannot retarget: " + id);
                for (const auto& occurrence : child->PresentationOccurrences)
                {
                    const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
                        [&](const auto& row) { return row.strResourceId == occurrence.strResourceId; });
                    if (resource == document.PresentationResources.end() || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
                        return fail("direction children support Animation and Effect only: " + id);
                }
            }
            for (const auto& row : pattern.PatternOccurrences)
                if (row.iStartMs < end && box.iStartMs < std::uint64_t(row.iStartMs) + row.iDurationMs)
                    return fail("a Pattern row overlaps the selected body's action window.");
            std::uint64_t origin = 0u;
            for (const auto& stage : pattern.Stages)
            {
                if (stage.bRetargetOnEnter && origin >= box.iStartMs && origin < end)
                    return fail("the Parent cannot retarget during the body's action window.");
                for (const auto& animation : stage.AnimationOccurrences)
                    if (origin + animation.iStartOffsetMs < end &&
                        origin + animation.iStartOffsetMs + animation.iPlayMs > box.iStartMs)
                        return fail("a Parent Animation overlaps the selected body's action window.");
                origin += stage.iDurationMs;
            }
            for (const auto& other : pattern.LogicOccurrences)
            {
                if (!other.bEnabled || other.strOccurrenceId == box.strOccurrenceId) continue;
                const auto* otherLogic = findLogic(other.strLogicId);
                if (!otherLogic || other.iStartMs >= end ||
                    std::uint64_t(other.iStartMs) + (std::max)(1u, other.iDurationMs) <= box.iStartMs) continue;
                if (otherLogic->strJudgementKind == "BOSS_TRACK_TARGET" || otherLogic->strJudgementKind == "PATTERN_COMPLETION_COUNT" ||
                    otherLogic->strTriggerKind == "REAL_GAZE_TELEPORT" || otherLogic->strTriggerKind == "BOSS_TELEPORT_XZ" ||
                    otherLogic->strTriggerKind == "ALBION_AIRBORNE" || otherLogic->strTriggerKind == "ANIMATION_BLEND" || otherLogic->fBossChargeDistanceM > 0.0)
                    return fail("another Logic controls the body during this window.");
            }
            windows.emplace_back(box.iStartMs, end);
        }
        std::sort(windows.begin(), windows.end());
        for (std::size_t index = 1u; index < windows.size(); ++index)
            if (windows[index].first < windows[index - 1u].second)
                return fail("direction windows overlap; the boss has one action owner.");
        return true;
    }

    bool_t Validate_PatternChildren(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, std::string& status)
    {
        if (!pattern.strLoadError.empty()) return true;
        if (!Validate_CrossDirectionWindows(document, pattern, status)) return false;
        if (pattern.PatternOccurrences.empty()) return true;
        const auto fail = [&](const std::string& reason) { status = "Parent " + pattern.strPatternId + ": " + reason; return false; };
        if (std::none_of(document.Folders.begin(), document.Folders.end(), [&](const auto& folder) {
            return folder.strTimelinePatternId == pattern.strPatternId; }))
            return fail("Pattern row requires an executable Parent timeline.");
        const auto lifetime = Pattern_Lifetime(pattern);
        if (pattern.BossMotion) return fail("Parent Boss Motion cannot share a child actor.");
        std::vector<std::pair<std::uint64_t, std::uint64_t>> windows;
        for (const auto& row : pattern.PatternOccurrences)
        {
            const auto child = std::find_if(document.Patterns.begin(), document.Patterns.end(),
                [&](const auto& candidate) { return candidate.strPatternId == row.strPatternId; });
            if (child == document.Patterns.end() || !child->strLoadError.empty())
                return fail("Missing or invalid child Pattern: " + row.strPatternId);
            if (child->strPatternId == pattern.strPatternId || !child->PatternOccurrences.empty() ||
                std::any_of(document.Folders.begin(), document.Folders.end(), [&](const auto& folder) {
                    return folder.strTimelinePatternId == child->strPatternId; }))
                return fail("Nested Parent or cyclic Pattern reference: " + row.strPatternId);
            if (child->strGateId != pattern.strGateId || child->strActorProfileId != pattern.strActorProfileId ||
                child->strTargetBossPlacementId != pattern.strTargetBossPlacementId)
                return fail("Child requires the same Gate, actor and target boss: " + row.strPatternId);
            if (!Pattern_Lifetime(*child) || Pattern_Lifetime(*child) > MAX_TIME_MS ||
                !row.iDurationMs || std::uint64_t(row.iStartMs) + row.iDurationMs > lifetime)
                return fail("Child or occurrence duration is invalid: " + row.strOccurrenceId);
            if (child->BossMotion)
            {
                if (pattern.PatternOccurrences.size() != 1u || row.bRepeat ||
                    row.iDurationMs != Pattern_Lifetime(*child) || pattern.bResetBossToSpawn || pattern.ResetBossYawDegrees ||
                    std::any_of(pattern.Stages.begin(), pattern.Stages.end(), [](const auto& stage) {
                        return !stage.AnimationOccurrences.empty() || stage.bRetargetOnEnter; }))
                    return fail("Moving child needs one complete non-repeated Pattern and no Parent animation, retarget or spawn reset.");
                for (const auto& box : pattern.LogicOccurrences)
                {
                    if (!box.bEnabled) continue;
                    const auto definition = std::find_if(document.Logics.begin(), document.Logics.end(),
                        [&](const auto& logic) { return logic.strLogicId == box.strLogicId; });
                    if (definition != document.Logics.end() && (definition->strTriggerKind == "REAL_GAZE_TELEPORT" || definition->strTriggerKind == "BOSS_TELEPORT_XZ" ||
                        (definition->strTriggerKind == "ENTER_AREA" && definition->fBossChargeDistanceM > 0.0)))
                        return fail("Moving child cannot share Parent teleport or charge Logic.");
                }
            }
            windows.emplace_back(row.iStartMs, std::uint64_t(row.iStartMs) + row.iDurationMs);
        }
        std::sort(windows.begin(), windows.end());
        for (size_t i = 1; i < windows.size(); ++i)
            if (windows[i].first < windows[i - 1].second)
                return fail("Pattern windows overlap; the boss has one action owner.");
        std::uint64_t origin = 0u;
        for (const auto& stage : pattern.Stages)
        {
            for (const auto& animation : stage.AnimationOccurrences)
                for (const auto& window : windows)
                    if (origin + animation.iStartOffsetMs < window.second &&
                        origin + animation.iStartOffsetMs + animation.iPlayMs > window.first)
                        return fail("Own Animation overlaps a child Pattern window: " + animation.strOccurrenceId);
            origin += stage.iDurationMs;
        }
        return true;
    }

    bool_t Validate_Bundle(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_BUNDLE& bundle, std::string& status)
    {
        if (!bundle.strLoadError.empty())
        {
            DATA_JSON_VALUE preserved;
            return Is_StableId(bundle.strBundleId) && !bundle.strPreservedJson.empty() &&
                CDataJson::Parse(bundle.strPreservedJson, preserved, status);
        }
        const auto fail = [&](const std::string& reason) { status = "Bundle " + bundle.strBundleId + ": " + reason; return false; };
        const auto folder = std::find_if(document.Folders.begin(), document.Folders.end(),
            [&](const auto& value) { return value.strFolderId == bundle.strFolderId; });
        if (!Is_StableId(bundle.strBundleId) || !Try_ParseGeneratedOrdinal(bundle.strBundleId,
            "kakulsaydon.bundle.", document.iNextBundleOrdinal) || !Is_DisplayName(bundle.strDisplayName) ||
            !CKoukuSaydonCompositionDocument::Is_KnownGate(bundle.strGateId) ||
            !Is_AuthoringStatus(bundle.strAuthoringStatus) || folder == document.Folders.end() ||
            !folder->strLoadError.empty() || folder->strGateId != bundle.strGateId)
            return fail("invalid identity, Gate, folder or status");
        if (!bundle.iNextMemberOrdinal || bundle.iNextMemberOrdinal > MAX_NEXT_ORDINAL ||
            !bundle.iNextSceneProfileOccurrenceOrdinal || bundle.iNextSceneProfileOccurrenceOrdinal > MAX_NEXT_ORDINAL ||
            !bundle.iNextPresentationOccurrenceOrdinal || bundle.iNextPresentationOccurrenceOrdinal > MAX_NEXT_ORDINAL ||
            bundle.Members.size() > 8u || bundle.SceneProfileOccurrences.size() > 16u || bundle.PresentationOccurrences.size() > 16u)
            return fail("invalid bounded counters or rows");
        const bool product = bundle.strAuthoringStatus == "PRODUCT";
        if (product && bundle.Members.empty()) return fail("PRODUCT needs at least one member");
        std::unordered_set<std::string> memberIds, patternIds, placements, occurrenceIds;
        std::unordered_set<std::string> statefulMembers;
        std::unordered_map<std::string, std::string> worldOwners;
        struct GLOBAL_WINDOW { std::string kind, owner; uint64_t start, end; };
        std::vector<GLOBAL_WINDOW> windows;
        std::unordered_map<std::string, std::unordered_set<std::string>> globalOwners, followupGlobalOwners;
        const auto addGlobalOwners = [&](const auto& scenes, const auto& presentation, const std::string& owner, bool followup)
        {
            const auto add = [&](const std::string& kind) { globalOwners[kind].insert(owner); if (followup) followupGlobalOwners[kind].insert(owner); };
            if (!scenes.empty()) add("SCENE_PROFILE");
            for (const auto& row : presentation)
            {
                const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
                    [&](const auto& value) { return value.strResourceId == row.strResourceId; });
                if (resource != document.PresentationResources.end() && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA) add("CAMERA");
            }
        };
        uint64_t span = 0;
        const auto addWindows = [&](const auto& scenes, const auto& presentation, uint64_t offset, const std::string& owner)
        {
            // Offset alone uses the Server's ceil-to-30-Hz start; presentation rows retain authored milliseconds.
            // Compare in ms * 30 units to preserve exact half-open boundaries without floating-point rounding.
            const uint64_t offsetUnits = ((offset * 30u + 999u) / 1000u) * 1000u;
            for (const auto& row : scenes) windows.push_back({"SCENE_PROFILE", owner, offsetUnits + uint64_t(row.iStartMs) * 30u, offsetUnits + (uint64_t(row.iStartMs) + row.iDurationMs) * 30u});
            for (const auto& row : presentation)
            {
                const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
                    [&](const auto& value) { return value.strResourceId == row.strResourceId; });
                if (resource != document.PresentationResources.end() && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
                    windows.push_back({"CAMERA", owner, offsetUnits + uint64_t(row.iStartMs) * 30u, offsetUnits + (uint64_t(row.iStartMs) + row.iDurationMs) * 30u});
            }
        };
        for (const auto& member : bundle.Members)
        {
            const auto pattern = std::find_if(document.Patterns.begin(), document.Patterns.end(),
                [&](const auto& value) { return value.strPatternId == member.strPatternId; });
            if (!Is_StableId(member.strMemberId) || !Try_ParseGeneratedOrdinal(member.strMemberId,
                bundle.strBundleId + ".member.", bundle.iNextMemberOrdinal) ||
                !memberIds.insert(member.strMemberId).second || !patternIds.insert(member.strPatternId).second ||
                member.iStartOffsetMs > MAX_TIME_MS || pattern == document.Patterns.end() ||
                !pattern->strLoadError.empty() || pattern->strGateId != bundle.strGateId ||
                !placements.insert(pattern->strTargetBossPlacementId).second || (product && pattern->strAuthoringStatus != "PRODUCT"))
                return fail("member is missing, duplicated, cross-Gate, corrupt, or not PRODUCT: " + member.strMemberId);
            uint64_t duration = 0; for (const auto& stage : pattern->Stages) duration += stage.iDurationMs;
            if (pattern->iDurationMs) duration = pattern->iDurationMs;
            span = (std::max)(span, duration + member.iStartOffsetMs);
            addWindows(pattern->SceneProfileOccurrences, pattern->PresentationOccurrences, member.iStartOffsetMs, member.strMemberId);
            addGlobalOwners(pattern->SceneProfileOccurrences, pattern->PresentationOccurrences, member.strMemberId, false);
            std::vector<const KOUKU_SAYDON_COMPOSITION_PATTERN*> pending{&*pattern};
            std::unordered_set<std::string> visited;
            const auto findLogic = [&](const std::string& id) -> const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION*
            {
                const auto found = std::find_if(document.Logics.begin(), document.Logics.end(), [&](const auto& row) { return row.strLogicId == id; });
                return found == document.Logics.end() ? nullptr : &*found;
            };
            while (!pending.empty())
            {
                const auto* current = pending.back(); pending.pop_back();
                if (!visited.insert(current->strPatternId).second) continue;
                if (!current->strLoadError.empty() || current->strGateId != bundle.strGateId ||
                    current->strTargetBossPlacementId != pattern->strTargetBossPlacementId ||
                    (product && current->strAuthoringStatus != "PRODUCT")) return fail("follow-up must retain Gate, target and Product status");
                for (const auto& box : current->LogicOccurrences)
                {
                    if (!box.bEnabled) continue;
                    const auto* logic = findLogic(box.strLogicId);
                    if (!logic) return fail("member Logic reference is missing");
                    if (logic->strJudgementKind == "POSE_INPUT" || logic->strJudgementKind == "ROULETTE_CARD_MATCH" ||
                        (logic->strTriggerKind == "HUD_ENTER" || logic->strTriggerKind == "CARD_MAZE_HIDE_NEXT" || logic->strTriggerKind == "CARD_MAZE_ENTER")) statefulMembers.insert(member.strMemberId);
                    for (const auto* outcomes : {&box.OnSuccessLogicIds, &box.OnFailLogicIds, &box.OnTimeoutLogicIds})
                        for (const auto& id : *outcomes)
                        {
                            const auto* outcome = findLogic(id);
                            if (!outcome) return fail("member outcome reference is missing");
                            if (outcome->strOutcomeKind == "CLOWN_TRANSFORM") statefulMembers.insert(member.strMemberId);
                            if (!outcome->strFollowupPatternId.empty())
                            {
                                const auto followup = std::find_if(document.Patterns.begin(), document.Patterns.end(),
                                    [&](const auto& row) { return row.strPatternId == outcome->strFollowupPatternId; });
                                if (followup == document.Patterns.end()) return fail("member follow-up Pattern is missing");
                                addGlobalOwners(followup->SceneProfileOccurrences, followup->PresentationOccurrences, member.strMemberId, true);
                                pending.push_back(&*followup);
                            }
                        }
                }
                for (const auto& box : current->WorldOccurrences)
                {
                    const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
                        [&](const auto& row) { return row.strWorldId == box.strWorldId; });
                    if (world == document.Worlds.end()) return fail("member WORLD reference is missing");
                    const auto owner = worldOwners.find(world->strSequenceInstanceId);
                    if (owner != worldOwners.end() && owner->second != member.strMemberId)
                        return fail("members share a live WORLD sequence instance");
                    worldOwners[world->strSequenceInstanceId] = member.strMemberId;
                }
            }

        }
        if (statefulMembers.size() > 1u) return fail("multiple owners of shared player mode (POSE/ROULETTE/HUD/CLOWN)");
        for (const auto& row : bundle.SceneProfileOccurrences)
        {
            const auto profile = std::find_if(document.SceneProfiles.begin(), document.SceneProfiles.end(),
                [&](const auto& value) { return value.strSceneProfileId == row.strSceneProfileId; });
            const uint64_t end = uint64_t(row.iStartMs) + row.iDurationMs;
            if (!Is_StableId(row.strOccurrenceId) || !Try_ParseGeneratedOrdinal(row.strOccurrenceId,
                bundle.strBundleId + ".sceneprofile.", bundle.iNextSceneProfileOccurrenceOrdinal) ||
                !occurrenceIds.insert(row.strOccurrenceId).second || profile == document.SceneProfiles.end() ||
                !row.iDurationMs || end > MAX_TIME_MS || row.iBlendMs > MAX_TIME_MS)
                return fail("invalid common Scene Profile: " + row.strOccurrenceId);
            span = (std::max)(span, end);
        }
        for (const auto& row : bundle.PresentationOccurrences)
        {
            const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
                [&](const auto& value) { return value.strResourceId == row.strResourceId; });
            const uint64_t end = uint64_t(row.iStartMs) + row.iDurationMs;
            if (!Is_StableId(row.strOccurrenceId) || !Try_ParseGeneratedOrdinal(row.strOccurrenceId,
                bundle.strBundleId + ".presentation.", bundle.iNextPresentationOccurrenceOrdinal) ||
                !occurrenceIds.insert(row.strOccurrenceId).second || resource == document.PresentationResources.end() ||
                resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::CAMERA || !row.iDurationMs || end > MAX_TIME_MS ||
                uint64_t(row.iFadeInMs) + row.iFadeOutMs > row.iDurationMs ||
                !Valid_PresentationVector(row.PositionOffset, -100000., 100000.) ||
                !Valid_PresentationVector(row.RotationDegrees, -36000., 36000.) ||
                !Valid_PresentationVector(row.Scale, .001, 10000.) ||
                !std::isfinite(row.fVolume) || row.fVolume < 0. || row.fVolume > 1. ||
                !std::isfinite(row.fDissolveStart) || !std::isfinite(row.fDissolveEnd) ||
                row.fDissolveStart < 0. || row.fDissolveEnd > 1. || row.fDissolveStart > row.fDissolveEnd ||
                row.fBrightnessMultiplier != 1. || row.strAnchorKind != "BOSS" || !row.strBone.empty() || row.strBoneTarget != "BODY" ||
                !row.strWorldId.empty() || !row.strRegionId.empty() || !row.strLogicOccurrenceId.empty() ||
                !row.strWorldOccurrenceId.empty() || !row.strSelectionGroupId.empty() || row.strCardSymbol != "NONE" || row.strCardColor != "NONE")
                return fail("common presentation supports unbound Camera only: " + row.strOccurrenceId);
            span = (std::max)(span, end);
        }
        if (span > MAX_TIME_MS) return fail("offset or common track exceeds 600 seconds");
        addWindows(bundle.SceneProfileOccurrences, bundle.PresentationOccurrences, 0, bundle.strBundleId);
        addGlobalOwners(bundle.SceneProfileOccurrences, bundle.PresentationOccurrences, bundle.strBundleId, false);
        for (const auto& [kind, owners] : followupGlobalOwners)
            if (!owners.empty() && globalOwners[kind].size() > 1u)
                return fail("conditional follow-up global " + kind + " conflicts with another owner; its start time is not fixed");
        for (size_t a = 0; a < windows.size(); ++a) for (size_t b = a + 1; b < windows.size(); ++b)
            if (windows[a].kind == windows[b].kind && windows[a].owner != windows[b].owner &&
                windows[a].start < windows[b].end && windows[b].start < windows[a].end)
                return fail("overlapping global " + windows[a].kind + " owners: " + windows[a].owner + " / " + windows[b].owner);
        return true;
    }

	bool_t Validate_Shape(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		std::string& outStatus,
		const bool_t validatePatternLinks = true,
        const bool_t expandedIds = false)
	{
		if (document.iFormatVersion != FORMAT_VERSION ||
			document.iRevision < 1u || document.iRevision > MAX_REVISION ||
			(document.strCompositionId != COMPOSITION_ID &&
			 document.strCompositionId != SEQUENCE_COMPOSITION_ID) ||
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
			document.iNextFolderOrdinal < 1u || document.iNextFolderOrdinal > MAX_NEXT_ORDINAL ||
            document.iNextBundleOrdinal < 1u || document.iNextBundleOrdinal > MAX_NEXT_ORDINAL ||
            document.Folders.size() > 4096u || document.Bundles.size() > 4096u || document.PatternFlows.size() > 4u ||
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
				(!expandedIds && !logic.strLogicId.starts_with(GENERATED_LOGIC_PREFIX)) ||
				(!expandedIds && !Try_ParseGeneratedOrdinal(logic.strLogicId,
					GENERATED_LOGIC_PREFIX, document.iNextLogicOrdinal)) ||
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
				(!expandedIds && !Try_ParseGeneratedOrdinal(summon.strSummonId,
					GENERATED_SUMMON_PREFIX, document.iNextSummonOrdinal)) ||
				!summonIds.insert(summon.strSummonId).second ||
				!Is_DisplayName(summon.strDisplayName))
			{
				outStatus = "KoukuSaydon Summon definition identity or name is invalid: " +
					summon.strSummonId;
				return false;
			}
            if (!Validate_SummonPolicy(summon, outStatus)) return false;
		}

		std::unordered_set<std::string> worldIds;
		std::unordered_map<std::string, std::string> worldInstanceIds;
		for (const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world : document.Worlds)
		{
			// The Gate 2 source importer keeps the actor's semantic identity;
			// its World resource and installed instance must share the exact suffix.
			constexpr std::string_view sourceWorldPrefix = "world.kouku.gate2.intro.";
			const bool sourceWorld = world.strWorldId.starts_with(sourceWorldPrefix) &&
				world.strWorldId.size() > sourceWorldPrefix.size() &&
				world.strSequenceInstanceId == "world.sequence.instance.kouku.gate2.intro." +
					world.strWorldId.substr(sourceWorldPrefix.size());
			if (!Is_StableId(world.strWorldId) ||
				(!sourceWorld && (!world.strWorldId.starts_with(GENERATED_WORLD_PREFIX) ||
				 (!expandedIds && !Try_ParseGeneratedOrdinal(world.strWorldId,
					GENERATED_WORLD_PREFIX, document.iNextWorldOrdinal)))) ||
				!worldIds.insert(world.strWorldId).second ||
				!Is_DisplayName(world.strDisplayName) ||
				!Is_StableId(world.strSequenceInstanceId) ||
				(!world.strObjectResourceId.empty() && !Is_StableId(world.strObjectResourceId)) ||
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
				(!expandedIds && !Try_ParseGeneratedOrdinal(profile.strSceneProfileId,
					GENERATED_SCENE_PROFILE_PREFIX, document.iNextSceneProfileOrdinal)) ||
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
					(row.strResourceKind == "GROUP" || row.strResourceKind == "LEAF" || row.strResourceKind == "V1_EFFECT" || row.strResourceKind == "V1_ELEMENT") &&
					(row.strResourceKind == "V1_ELEMENT" ? Is_StableId(row.strElementId) : row.strElementId.empty()); break;
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
				validAsset = row.strAssetId.empty() && (row.strShape == "BOX" || row.strShape == "SECTOR" || row.strShape == "REVERSE_SECTOR" || row.strShape == "CIRCLE"); break;
			default: break;
			}
			if (!Is_StableId(row.strResourceId) || (!expandedIds && !Try_ParseGeneratedOrdinal(row.strResourceId,
				"kakulsaydon.g1.presentation.", document.iNextPresentationResourceOrdinal)) ||
				!presentationResources.emplace(row.strResourceId, &row).second ||
				!Is_DisplayName(row.strDisplayName) || !validAsset ||
				(row.strColliderKind != "GEOMETRY" && row.strColliderKind != "ROULETTE_CARD_REGION") ||
				(row.eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER && row.strColliderKind != "GEOMETRY") ||
				row.iDurationMs == 0u || row.iDurationMs > MAX_TIME_MS ||
				!Valid_PresentationVector(row.HalfExtents, 0.001, 10000.0) ||
				!std::isfinite(row.fRadiusM) || row.fRadiusM <= 0.0 || row.fRadiusM > 10000.0 ||
				!std::isfinite(row.fHalfAngleDegrees) || (row.strShape == "REVERSE_SECTOR" ? row.fHalfAngleDegrees < 0.0 : row.fHalfAngleDegrees <= 0.0) || row.fHalfAngleDegrees > 180.0)
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
        for (const auto& logic : document.Logics)
        {
            if (logic.strOutcomeKind != "FEAR") continue;
            const auto effect = presentationResources.find(logic.strEffectResourceId);
            const auto light = presentationResources.find(logic.strLightResourceId);
            if ((!logic.strSceneProfileId.empty() && !sceneProfileIds.contains(logic.strSceneProfileId)) ||
                (!logic.strEffectResourceId.empty() && (effect == presentationResources.end() ||
                 effect->second->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)))
            { outStatus = "FEAR references a missing Scene Profile or Effect resource: " + logic.strLogicId; return false; }
            if (!logic.strLightResourceId.empty() && (light == presentationResources.end() ||
                light->second->eKind != KOUKU_SAYDON_PRESENTATION_KIND::LIGHT || light->second->strDefaultAnchorKind != "PLAYER"))
            { outStatus = "FEAR requires a PLAYER Light resource: " + logic.strLogicId; return false; }
        }
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
				(!expandedIds && !Try_ParseGeneratedOrdinal(pattern.strPatternId,
					GENERATED_PATTERN_PREFIX, document.iNextPatternOrdinal)) ||
				!CKoukuSaydonCompositionDocument::Is_KnownGate(pattern.strGateId) ||
                CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(pattern.strGateId, pattern.strActorProfileId) != pattern.strTargetBossPlacementId ||
                pattern.strTargetBossPlacementId.empty() ||
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
				pattern.iNextPatternOccurrenceOrdinal == 0u ||
				pattern.iNextPatternOccurrenceOrdinal > MAX_NEXT_ORDINAL ||
				pattern.iDurationMs > MAX_TIME_MS ||
				pattern.PatternOccurrences.size() > MAX_PATTERN_OCCURRENCES ||
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

			if (pattern.bEnterCombatOnFinish && pattern.strGateId != "GATE1" &&
				pattern.strGateId != "GATE2" && pattern.strGateId != "GATE3")
			{ outStatus = "Combat entry sequences require GATE1, GATE2, or GATE3."; return false; }

			if (!std::isfinite(pattern.fAnimationRootVerticalScale) ||
				pattern.fAnimationRootVerticalScale < 0.0 || pattern.fAnimationRootVerticalScale > 1.0)
			{ outStatus = "Animation root vertical scale must be finite 0..1."; return false; }
			if (pattern.ResetBossYawDegrees && (!pattern.bResetBossToSpawn ||
				!std::isfinite(*pattern.ResetBossYawDegrees) || std::abs(*pattern.ResetBossYawDegrees) > 360.0))
			{
				outStatus = "Pattern reset yaw needs spawn reset and finite -360..360 degrees.";
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
					(!expandedIds && !Try_ParseGeneratedOrdinal(stage.strStageId,
						GENERATED_STAGE_PREFIX, pattern.iNextStageOrdinal)) ||
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
						(!expandedIds && !Try_ParseGeneratedOrdinal(occurrence.strOccurrenceId,
							occurrencePrefix, pattern.iNextAnimationOrdinal)) ||
						CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(occurrence.strProfileId) != pattern.strActorProfileId ||
						(occurrence.strSourceStageId == "RAW" && 0u != occurrence.iSourceActionId) ||
						!Is_StableId(occurrence.strSourceStageId) ||
						!Is_StableId(occurrence.strSourceSlotId) ||
						(occurrence.strSourceStageId != "RAW" && !Is_LowerSha256(occurrence.strReferenceRevision)) ||
						!Is_StableId(occurrence.strRuntimeClip) ||
						occurrence.iStartOffsetMs > MAX_TIME_MS ||
						occurrence.iSourceStartMs > MAX_TIME_MS ||
                        occurrence.iSourceEndMs > MAX_TIME_MS ||
                        (occurrence.iSourceEndMs && occurrence.iSourceEndMs <= occurrence.iSourceStartMs) ||
						0u == occurrence.iPlayMs || occurrence.iPlayMs > MAX_TIME_MS ||
						occurrence.iBlendInMs > 1000u || occurrence.iBlendInMs > occurrence.iPlayMs ||
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
            if (pattern.iDurationMs && patternDurationMs > pattern.iDurationMs)
            { outStatus = "Explicit Pattern duration is shorter than its Stages: " + pattern.strPatternId; return false; }
            const auto lifetime = pattern.iDurationMs ? pattern.iDurationMs : patternDurationMs;
            for (const auto& row : pattern.PatternOccurrences)
            {
                if (!Is_StableId(row.strOccurrenceId) || (!expandedIds && !Try_ParseGeneratedOrdinal(row.strOccurrenceId,
                    pattern.strPatternId + ".pattern.", pattern.iNextPatternOccurrenceOrdinal)) ||
                    !occurrenceIds.insert(row.strOccurrenceId).second || !Is_StableId(row.strPatternId) ||
                    !row.iDurationMs || std::uint64_t(row.iStartMs) + row.iDurationMs > lifetime)
                { outStatus = "Invalid Pattern occurrence identity or window: " + row.strOccurrenceId; return false; }
            }
            std::vector<std::pair<std::uint64_t, const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE*>> animationOrder;
            std::uint64_t stageOrigin = 0u;
            for (const auto& stage : pattern.Stages)
            {
                for (const auto& row : stage.AnimationOccurrences)
                    animationOrder.emplace_back(stageOrigin + row.iStartOffsetMs, &row);
                stageOrigin += stage.iDurationMs;
            }
            std::sort(animationOrder.begin(), animationOrder.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
            for (size_t i = 0; i < animationOrder.size(); ++i)
            {
                if (!animationOrder[i].second->iBlendInMs) continue;
                if (i == 0 ||
                    animationOrder[i-1].first + animationOrder[i-1].second->iPlayMs != animationOrder[i].first)
                { outStatus = "Animation blend requires an adjacent previous occurrence: " + animationOrder[i].second->strOccurrenceId; return false; }
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
			if (pattern.iDurationMs) lifetimeMs = pattern.iDurationMs;
			if (pattern.BossMotion)
			{
				const auto& motion = *pattern.BossMotion;
				if (std::any_of(pattern.Stages.begin(), pattern.Stages.end(),
					[](const auto& stage) { return stage.bRetargetOnEnter; }))
				{ outStatus = "Stage retargetOnEnter cannot share a Pattern with fixed-yaw Boss Motion."; return false; }
				if (pattern.bResetBossToSpawn || pattern.ResetBossYawDegrees ||
					motion.iStartMs >= motion.iEndMs || motion.iEndMs > lifetimeMs ||
					!Valid_PresentationVector(motion.StartPosition, -100000.0, 100000.0) ||
					!Valid_PresentationVector(motion.EndPosition, -100000.0, 100000.0) ||
					motion.StartPosition[1] != motion.EndPosition[1] ||
					!std::isfinite(motion.fYawDegrees) || std::abs(motion.fYawDegrees) > 360.0)
				{ outStatus = "Boss Motion needs a valid pattern interval, equal base Y and no spawn reset."; return false; }
			}
			if (pattern.PatternOccurrences.empty())
			{
				std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> blendWindows;
				if (!CKoukuSaydonCompositionDocument::Try_ResolveAnimationBlendWindows(
					document, pattern, blendWindows, outStatus)) return false;
			}
			std::unordered_set<std::string> regionIds;
			std::unordered_set<std::string> companionWorldBoxes;
			std::unordered_map<std::string, std::pair<
				const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE*, std::size_t>> selectionGroups;
			for (const auto& row : pattern.PresentationOccurrences)
			{
				const std::uint64_t endMs = static_cast<std::uint64_t>(row.iStartMs) + row.iDurationMs;
				if (!Is_StableId(row.strOccurrenceId) || (!expandedIds && !Try_ParseGeneratedOrdinal(row.strOccurrenceId,
					pattern.strPatternId + ".presentation.", pattern.iNextPresentationOccurrenceOrdinal)) ||
					!occurrenceIds.insert(row.strOccurrenceId).second ||
					!presentationResources.contains(row.strResourceId) || row.iDurationMs == 0u ||
					endMs > lifetimeMs || endMs > MAX_TIME_MS ||
					static_cast<std::uint64_t>(row.iFadeInMs) + row.iFadeOutMs > row.iDurationMs ||
					!Valid_PresentationVector(row.PositionOffset, -100000.0, 100000.0) ||
					!Valid_PresentationVector(row.RotationDegrees, -36000.0, 36000.0) ||
					!Valid_PresentationVector(row.Scale, 0.001, 10000.0) ||
					!std::isfinite(row.fDissolveStart) || row.fDissolveStart < 0.0 || row.fDissolveStart > 1.0 ||
					!std::isfinite(row.fDissolveEnd) || row.fDissolveEnd < row.fDissolveStart || row.fDissolveEnd > 1.0 ||
					!std::isfinite(row.fVolume) || row.fVolume < 0.0 || row.fVolume > 1.0 ||
					!std::isfinite(row.fBrightnessMultiplier) || row.fBrightnessMultiplier < 0.0 || row.fBrightnessMultiplier > 16.0 ||
					(row.strBoneTarget != "BODY" && row.strBoneTarget != "WEAPON") ||
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
                // Camera shots own their world coordinates; Sound does not bind an Object.
				if (row.bFitEffectToDuration && (resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
					(resource->strResourceKind != "V1_EFFECT" && resource->strResourceKind != "V1_ELEMENT")))
				{ outStatus = "Fit Effect lifetime requires a V1 Effect resource."; return false; }
                const bool objectAnchored = resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
                    resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT || resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER;
                if (objectAnchored && row.strAnchorKind == "WORLD" && row.strWorldId.empty())
                { outStatus = "World Object anchor requires a worldId; fixed world coordinates use MAP: " + row.strOccurrenceId; return false; }
				if (!row.strSelectionGroupId.empty())
				{
					if ((resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER &&
						resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT) || !Is_StableId(row.strSelectionGroupId))
					{ outStatus = "Selection Group requires a stable ID on an Effect or Collider: " + row.strOccurrenceId; return false; }
					if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER && row.strAnchorKind != "BOSS")
					{ outStatus = "Selection Group requires BOSS anchors; WORLD, MAP and PLAYER are unsupported."; return false; }
					auto& group = selectionGroups[row.strSelectionGroupId];
					if (nullptr == group.first) group.first = &row;
					const auto& first = *group.first;
					const auto* firstResource = presentationResources.at(first.strResourceId);
					if (!firstResource || firstResource->eKind != resource->eKind)
					{ outStatus = "Selection Group cannot mix Effect and Collider boxes: " + row.strSelectionGroupId; return false; }
					if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER && (row.strAnchorKind != first.strAnchorKind || row.bFollowBoss != first.bFollowBoss ||
						row.strBone != first.strBone || row.strBoneTarget != first.strBoneTarget ||
						row.strWorldId != first.strWorldId || row.strWorldOccurrenceId != first.strWorldOccurrenceId ||
						row.iWorldEmissionIndex != first.iWorldEmissionIndex ||
						(!row.bFollowBoss && row.iStartMs != first.iStartMs)))
					{ outStatus = "Selection Group Colliders must share an anchor frame: " + row.strSelectionGroupId; return false; }
					++group.second;
				}
				if (row.strBoneTarget != "BODY" && ((resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER && resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT) ||
					row.strAnchorKind != "BOSS" || row.strBone.empty()))
				{ outStatus = "WEAPON Bone anchors require a boss Collider and an explicit weapon bone."; return false; }
				if ((resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::LIGHT &&
					(row.strAnchorKind == "PLAYER" || (row.strAnchorKind == "MAP" && resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT) || row.fBrightnessMultiplier != 1.0)) ||
					(resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT &&
					(row.Scale != std::array<double, 3u>{1.0, 1.0, 1.0} ||
					 (row.strAnchorKind != "BOSS" && !row.strBone.empty()) ||
					 (row.strAnchorKind == "PLAYER" && !row.bFollowBoss))))
				{ outStatus = "Invalid Light anchor, scale, bone or brightness: " + row.strOccurrenceId; return false; }
				if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT && row.strAnchorKind == "MAP" &&
					(row.bFollowBoss || !row.strBone.empty() || row.strBoneTarget != "BODY" || !row.strWorldOccurrenceId.empty()))
				{ outStatus = "MAP Effect requires a fixed position without a bone or World occurrence."; return false; }
				if (!row.strWorldOccurrenceId.empty())
				{
					const auto owner = std::find_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
						[&](const auto& box) { return box.strOccurrenceId == row.strWorldOccurrenceId; });
					const auto world = owner == pattern.WorldOccurrences.end() ? document.Worlds.end() :
						std::find_if(document.Worlds.begin(), document.Worlds.end(), [&](const auto& value) { return value.strWorldId == owner->strWorldId; });
					if (!Is_StableId(row.strWorldOccurrenceId) || world == document.Worlds.end())
					{ outStatus = "Presentation needs an existing World occurrence in the same Pattern: " + row.strOccurrenceId; return false; }
					if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT && row.strAnchorKind != "WORLD")
					{
						if (world->strCompanionEffectResourceId != row.strResourceId || !companionWorldBoxes.insert(row.strWorldOccurrenceId).second)
						{ outStatus = "Effect companion needs one matching World box/resource in the same Pattern: " + row.strOccurrenceId; return false; }
					}
					else if ((resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER &&
						resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::LIGHT && resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT) ||
						row.strAnchorKind != "WORLD" || row.strWorldId != owner->strWorldId)
					{ outStatus = "WORLD presentation occurrence must match its World definition and anchor."; return false; }
				}
				if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT && row.strAnchorKind == "WORLD")
				{
					const auto matches = std::count_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
						[&](const auto& worldBox) { return worldBox.strWorldId == row.strWorldId &&
							(row.strWorldOccurrenceId.empty() || worldBox.strOccurrenceId == row.strWorldOccurrenceId); });
					if (row.strWorldId.empty() || matches != 1)
					{ outStatus = "WORLD Light needs one exact World occurrence in this Pattern."; return false; }
				}
				if (!row.strLogicOccurrenceId.empty())
				{
					const auto linked = std::find_if(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(),
						[&](const auto& box) { return box.strOccurrenceId == row.strLogicOccurrenceId; });
					const auto* owner = linked == pattern.LogicOccurrences.end() ? nullptr : findLogic(linked->strLogicId);
					if (resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER || nullptr == owner ||
						!Kouku_LogicAcceptsColliders(*owner) ||
						linked->iStartMs != row.iStartMs || linked->iDurationMs != row.iDurationMs)
					{ outStatus = "Collider link needs an existing Duration/Trigger window with identical timing: " + row.strOccurrenceId; return false; }
					if (owner->strTriggerKind == "OBJECT_CONTACT" &&
						((row.strAnchorKind == "WORLD" && (!row.strBone.empty() || row.strBoneTarget != "BODY")) || (row.strAnchorKind == "BOSS" && !row.bFollowBoss)))
					{ outStatus = "OBJECT_CONTACT Collider must follow its boss Bone/pivot or use a WORLD track without a Bone."; return false; }
				}
				const bool_t rouletteRegion = resource->strColliderKind == "ROULETTE_CARD_REGION";
				if (product && ((objectAnchored && row.strAnchorKind == "WORLD" && row.strWorldId.empty()) ||
					(rouletteRegion && (row.strRegionId.empty() || row.strCardSymbol == "NONE" ||
						row.strCardColor == "NONE" || row.strAnchorKind != "WORLD" || row.strWorldId.empty()))))
				{ outStatus = "PRODUCT roulette region needs an explicit World, region ID, symbol and color: " + row.strOccurrenceId; return false; }
			}
			for (const auto& [groupId, group] : selectionGroups)
				if (group.second < 2u)
				{ outStatus = "Selection Group requires at least two boxes of one kind in one Pattern: " + groupId; return false; }
			std::unordered_set<std::string> logicBoxIds;
			std::optional<std::array<double, 3u>> captureGrip;
			std::vector<std::string> rouletteInstances;
            std::vector<std::pair<std::uint64_t, std::uint64_t>> chargeWindows;
			const std::string logicPrefix = pattern.strPatternId + ".logic.";
			for (const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE& box : pattern.LogicOccurrences)
			{
				const std::uint64_t boxEndMs =
					static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs;
				if (!Is_StableId(box.strOccurrenceId) ||
					(!expandedIds && !box.strOccurrenceId.starts_with(logicPrefix)) ||
					(!expandedIds && !Try_ParseGeneratedOrdinal(box.strOccurrenceId, logicPrefix,
						pattern.iNextLogicOccurrenceOrdinal)) ||
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
				for (const auto& volley : owner.RandomVolleyOccurrenceSets)
				{
					bool hasMap = false;
					for (const auto& id : volley)
					{
						const auto row = std::find_if(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
							[&](const auto& value) { return value.strOccurrenceId == id; });
						if (row == pattern.PresentationOccurrences.end() || !presentationResources.contains(row->strResourceId) ||
							presentationResources.at(row->strResourceId)->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
							!row->strBone.empty() || row->strBoneTarget != "BODY" || !row->strWorldId.empty() ||
							!row->strWorldOccurrenceId.empty() || row->iWorldEmissionIndex != 0u || !row->strLogicOccurrenceId.empty() ||
							!((row->strAnchorKind == "MAP" && !row->bFollowBoss) || (row->strAnchorKind == "BOSS" && row->bFollowBoss)))
						{ outStatus = "SHOWTIME random volley requires same-pattern Effect rows with MAP or following BOSS anchors: " + id; return false; }
						hasMap |= row->strAnchorKind == "MAP";
					}
					if (!hasMap) { outStatus = "SHOWTIME random volley needs a MAP anchor marker."; return false; }
				}
				const bool roomArrival = owner.strTriggerKind == "ROOM_PLAYER_ARRIVAL";
				if (roomArrival != box.RoomPlayerArrival.has_value() ||
					(roomArrival && (document.strCompositionId != SEQUENCE_COMPOSITION_ID ||
					 box.RoomPlayerArrival->iPlayerSlot > 3u ||
					 !std::all_of(box.RoomPlayerArrival->Position.begin(), box.RoomPlayerArrival->Position.end(),
						 [](double v) { return std::isfinite(v) && std::abs(v) <= 100000.0; }))))
				{ outStatus = "ROOM_PLAYER_ARRIVAL requires a Sequence box with slot 0..3 and finite XYZ."; return false; }
				if (!box.strHoldLogicOccurrenceId.empty())
				{
					const auto hold = std::find_if(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(),
						[&](const auto& row) { return row.strOccurrenceId == box.strHoldLogicOccurrenceId; });
					const auto* holdLogic = hold == pattern.LogicOccurrences.end() ? nullptr : findLogic(hold->strLogicId);
					if (owner.strTriggerKind != "ENTER_AREA" || !holdLogic || holdLogic->strLogicType != "DURATION" ||
						holdLogic->strJudgementKind != "ATTACHMENT_HOLD")
					{ outStatus = "ENTER_AREA Hold must reference an ATTACHMENT_HOLD window in this Pattern."; return false; }
					if (product && box.bEnabled && (!hold->bEnabled || hold->iStartMs > box.iStartMs ||
						std::uint64_t(hold->iStartMs) + hold->iDurationMs < boxEndMs))
					{ outStatus = "Capture Hold must be enabled and cover the complete Trigger window."; return false; }
				}
				if (std::count_if(box.OnSuccessLogicIds.begin(), box.OnSuccessLogicIds.end(), [&](const auto& id) {
					const auto* result = findLogic(id); return result && result->strOutcomeKind == "CAPTURE_PLAYER"; }) > 1)
				{ outStatus = "One Trigger may apply at most one CAPTURE_PLAYER Result."; return false; }
				if (pattern.BossMotion && box.bEnabled && (owner.strTriggerKind == "REAL_GAZE_TELEPORT" || owner.strTriggerKind == "BOSS_TELEPORT_XZ" || owner.strTriggerKind == "ALBION_AIRBORNE"))
				{ outStatus = "Boss Motion cannot also teleport the boss."; return false; }
                if (box.bEnabled && owner.fBossChargeDistanceM > 0.0)
                {
                    if (pattern.BossMotion || std::any_of(chargeWindows.begin(), chargeWindows.end(), [&](const auto& interval) {
                        return interval.first < boxEndMs && box.iStartMs < interval.second; }))
                    { outStatus = "Boss charge windows cannot overlap or share absolute Boss Motion."; return false; }
                    std::uint64_t stageStartMs = 0u;
                    for (const auto& stage : pattern.Stages)
                    {
                        if (stage.bRetargetOnEnter && box.iStartMs <= stageStartMs && stageStartMs < boxEndMs)
                        { outStatus = "Boss charge direction cannot retarget during its window."; return false; }
                        stageStartMs += stage.iDurationMs;
                    }
                    chargeWindows.emplace_back(box.iStartMs, boxEndMs);
                }
				if (owner.strJudgementKind == "OBJECT_OVERLAP")
				{
					const auto count = std::count_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(), [&](const auto& worldBox) {
						const auto found = worldInstanceIds.find(worldBox.strWorldId);
						return found != worldInstanceIds.end() && found->second == owner.strTargetWorldInstanceId; });
					if (count > 1 || (product && count != 1))
					{ outStatus = "OBJECT_OVERLAP uses one target WORLD box; use OBJECT_CONTACT to distinguish multiple placements."; return false; }
				}
				if (owner.strTriggerKind == "OBJECT_CONTACT")
				{
					if (!box.OnFailLogicIds.empty() || !box.OnTimeoutLogicIds.empty())
					{ outStatus = "OBJECT_CONTACT owns only contact Success outcomes; its expiry does not fail the search."; return false; }
					if (std::count_if(box.OnSuccessLogicIds.begin(), box.OnSuccessLogicIds.end(), [&](const auto& id) {
						const auto* result = findLogic(id); return result && result->strOutcomeKind == "PLAY_CONTACT_WORLD_OBJECT_MOTION"; }) > 1)
					{ outStatus = "One contact Trigger may apply at most one card Motion Result."; return false; }
					for (const auto& id : owner.TargetWorldOccurrenceIds)
					{
						const auto target = std::find_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
							[&](const auto& item) { return item.strOccurrenceId == id; });
						if (target == pattern.WorldOccurrences.end() || target->iStartMs > box.iStartMs ||
							static_cast<std::uint64_t>(target->iStartMs) + target->iDurationMs < boxEndMs)
						{ outStatus = "Contact target WORLD placement must exist in this Pattern and cover its whole Trigger window: " + id; return false; }
					}
					if (!owner.strContactGroupId.empty())
						for (const auto& other : pattern.LogicOccurrences)
						{
							if (other.strOccurrenceId == box.strOccurrenceId || !other.bEnabled || !box.bEnabled) continue;
							const auto* otherLogic = findLogic(other.strLogicId);
							if (otherLogic && otherLogic->strTriggerKind == "OBJECT_CONTACT" &&
								otherLogic->strContactGroupId == owner.strContactGroupId &&
								(static_cast<std::uint64_t>(other.iStartMs) * 30u + 999u) / 1000u == (static_cast<std::uint64_t>(box.iStartMs) * 30u + 999u) / 1000u &&
								(other.iStartMs != box.iStartMs || other.iDurationMs != box.iDurationMs || otherLogic->iContactPriority == owner.iContactPriority))
							{ outStatus = "Contact windows with the same strike group and Server start tick need identical source times and distinct priorities."; return false; }
						}
				}
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
						if (result.strOutcomeKind == "GRAB_TO_WORLD_OBJECT" &&
							(owner.strTriggerKind != "ENTER_AREA" || slot != KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS))
						{ outStatus = "GRAB_TO_WORLD_OBJECT belongs to ENTER_AREA Success only."; return false; }
						if (result.strOutcomeKind == "CAPTURE_PLAYER")
						{
							if (owner.strTriggerKind != "ENTER_AREA" || slot != KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS)
							{ outStatus = "CAPTURE_PLAYER belongs to ENTER_AREA Success only."; return false; }
							if (box.OnSuccessLogicIds.size() != 1u)
							{ outStatus = "CAPTURE_PLAYER must be the only Success Result so capture can commit atomically."; return false; }
							if (product && box.bEnabled)
							{
								if (box.strHoldLogicOccurrenceId.empty())
								{ outStatus = "Capture Trigger requires an ATTACHMENT_HOLD window."; return false; }
								if (captureGrip && *captureGrip != result.GripLocalOffset)
								{ outStatus = "A Pattern must use one consistent capture grip offset."; return false; }
								captureGrip = result.GripLocalOffset;
							}
						}
						if (owner.strJudgementKind == "OBJECT_OVERLAP" &&
							(result.strOutcomeKind != "PLAY_WORLD_OBJECT_MOTION" || result.strTargetWorldInstanceId != owner.strTargetWorldInstanceId))
						{ outStatus = "OBJECT_OVERLAP Result must apply a motion to the same target World Object."; return false; }
						const bool contactMotion = result.strOutcomeKind == "PLAY_CONTACT_WORLD_OBJECT_MOTION";
						const bool signal = result.strOutcomeKind == "COMPLETE_LOGIC_WINDOW";
						if ((contactMotion || signal) && (owner.strTriggerKind != "OBJECT_CONTACT" || slot != KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS))
						{ outStatus = "Contact motion and completion Results belong to OBJECT_CONTACT Success only."; return false; }
						if (owner.strTriggerKind == "OBJECT_CONTACT" && !result.strOutcomeKind.empty() && !contactMotion && !signal)
						{ outStatus = "OBJECT_CONTACT Success accepts card motion or completion Results only."; return false; }
						if (contactMotion && (result.ContactMotions.size() != owner.TargetWorldOccurrenceIds.size() ||
							!std::all_of(owner.TargetWorldOccurrenceIds.begin(), owner.TargetWorldOccurrenceIds.end(), [&](const auto& id) {
								return std::any_of(result.ContactMotions.begin(), result.ContactMotions.end(), [&](const auto& row) { return row.strTargetWorldOccurrenceId == id; }); })))
						{ outStatus = "Contact motion mappings must cover exactly this Trigger's target WORLD placements."; return false; }
						if (signal)
						{
							const auto receiver = std::find_if(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(),
								[&](const auto& item) { return item.strOccurrenceId == result.strTargetLogicOccurrenceId; });
							const auto* receiverLogic = receiver == pattern.LogicOccurrences.end() ? nullptr : findLogic(receiver->strLogicId);
							if (!receiverLogic || receiverLogic->strLogicType != "DURATION" || receiverLogic->strJudgementKind != "EXTERNAL_SIGNAL" ||
								!receiver->bEnabled || receiver->iStartMs > box.iStartMs ||
								static_cast<std::uint64_t>(receiver->iStartMs) + receiver->iDurationMs < boxEndMs ||
								(!result.strContactTargetWorldOccurrenceId.empty() &&
									std::find(owner.TargetWorldOccurrenceIds.begin(), owner.TargetWorldOccurrenceIds.end(), result.strContactTargetWorldOccurrenceId) == owner.TargetWorldOccurrenceIds.end()))
							{ outStatus = "Complete window Result needs an enabled EXTERNAL_SIGNAL window covering the Trigger and an optional selected contact target."; return false; }
						}
						if ("FOLLOWUP_PATTERN" == result.strOutcomeKind)
						{
							if ("STAGGER_WINDOW" != owner.strJudgementKind && "COUNTER_WINDOW" != owner.strJudgementKind && "EXTERNAL_SIGNAL" != owner.strJudgementKind && "PATTERN_COMPLETION_COUNT" != owner.strJudgementKind)
							{
								outStatus = "A follow-up Pattern outcome belongs to a stagger or external-signal window: " +
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

            // An occurrence owns its optional actor plays; the reusable name remains shared.
			std::unordered_set<std::string> summonBoxIds;
			const std::string summonPrefix = pattern.strPatternId + ".summon.";
			for (const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE& box : pattern.SummonOccurrences)
			{
				const std::uint64_t boxEndMs =
					static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs;
				if (!Is_StableId(box.strOccurrenceId) ||
					(!expandedIds && !box.strOccurrenceId.starts_with(summonPrefix)) ||
					(!expandedIds && !Try_ParseGeneratedOrdinal(box.strOccurrenceId, summonPrefix,
						pattern.iNextSummonOccurrenceOrdinal)) ||
					!summonBoxIds.insert(box.strOccurrenceId).second ||
					!summonIds.contains(box.strSummonId) ||
					box.iStartMs > MAX_TIME_MS || 0u == box.iDurationMs ||
					boxEndMs > MAX_TIME_MS || boxEndMs > lifetimeMs)
				{
					outStatus = "KoukuSaydon Summon box identity, Summon reference, or window exceeds the Pattern lifetime: " +
	                    box.strOccurrenceId;
                    return false;
                }
                if (box.PatternSpawns.size() > MAX_SUMMON_PATTERN_SPAWNS)
                { outStatus = "A Summon box supports at most four Pattern spawns: " + box.strOccurrenceId; return false; }
                std::unordered_set<std::string> spawnIds;
                for (const auto& spawn : box.PatternSpawns)
                    if (!Is_StableId(spawn.strSpawnId) || !spawnIds.insert(spawn.strSpawnId).second ||
                        !Is_StableId(spawn.strPatternId) || !Valid_PresentationVector(spawn.PositionOffset, -1000.0, 1000.0) ||
                        !std::isfinite(spawn.fYawOffsetDegrees) || spawn.fYawOffsetDegrees < -360.0 || spawn.fYawOffsetDegrees > 360.0)
                    { outStatus = "Invalid Summon Pattern spawn identity or transform: " + box.strOccurrenceId; return false; }
            }

            /* A World box starts its sequence on the pattern clock; the sequence
			   runs to its own end, so only the start must sit inside the Pattern. */
			std::unordered_set<std::string> worldBoxIds;
			const std::string worldPrefix = pattern.strPatternId + ".world.";
			std::unordered_set<std::string> playedInstances;
			for (const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& box : pattern.WorldOccurrences)
			{
				const std::uint64_t boxEndMs =
					static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs;
				if (!Is_StableId(box.strOccurrenceId) ||
					(!expandedIds && !box.strOccurrenceId.starts_with(worldPrefix)) ||
					(!expandedIds && !Try_ParseGeneratedOrdinal(box.strOccurrenceId, worldPrefix,
						pattern.iNextWorldOccurrenceOrdinal)) ||
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
				if (box.Placement &&
					(!Valid_PresentationVector(box.Placement->Position, -100000.0, 100000.0) ||
					 !Valid_PresentationVector(box.Placement->RotationDegrees, -36000.0, 36000.0) ||
					 !Valid_PresentationVector(box.Placement->Scale, 0.001, 1000.0)))
				{ outStatus = "World occurrence placement must contain finite position, rotation and positive scale: " + box.strOccurrenceId; return false; }
				playedInstances.insert(worldInstanceIds.at(box.strWorldId));
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
					(!expandedIds && !box.strOccurrenceId.starts_with(scenePrefix)) ||
					(!expandedIds && !Try_ParseGeneratedOrdinal(box.strOccurrenceId, scenePrefix,
						pattern.iNextSceneProfileOccurrenceOrdinal)) ||
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

        if (validatePatternLinks)
        {
            std::unordered_set<std::string> folders, bundles;
            for (const auto& folder : document.Folders)
                if (!folders.insert(folder.strFolderId).second || !Validate_Folder(document, folder, outStatus)) return false;
            for (const auto& pattern : document.Patterns)
            {
                const auto chainCount = std::count_if(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(), [&](const auto& box) {
                    const auto* logic = findLogic(box.strLogicId);
                    return box.bEnabled && logic && logic->strJudgementKind == "PATTERN_COMPLETION_COUNT";
                });
                if (chainCount > 1) { outStatus = "One Pattern may own only one completion chain."; return false; }
                for (const auto& box : pattern.LogicOccurrences)
                {
                    if (!box.bEnabled) continue;
                    const auto* owner = findLogic(box.strLogicId);
                    if (!owner) continue;
                    if (owner->strJudgementKind == "PATTERN_COMPLETION_COUNT")
                    {
                        const auto* result = box.OnSuccessLogicIds.size() == 1u ? findLogic(box.OnSuccessLogicIds.front()) : nullptr;
                        if (!result || result->strOutcomeKind != "FOLLOWUP_PATTERN" || !box.OnFailLogicIds.empty() || !box.OnTimeoutLogicIds.empty())
                        { outStatus = "Completion count requires exactly one Success followup and no Fail/Timeout."; return false; }
                        for (const auto& id : owner->PatternIds)
                        {
                            const auto child = std::find_if(document.Patterns.begin(), document.Patterns.end(), [&](const auto& row) { return row.strPatternId == id; });
                            if (child != document.Patterns.end() && !child->strLoadError.empty())
                            { outStatus = "Completion candidate " + id + " is unavailable: " + child->strLoadError; return false; }
                            if (child == document.Patterns.end() || id == pattern.strPatternId || child->Stages.empty() ||
                                child->strGateId != pattern.strGateId || child->strTargetBossPlacementId != pattern.strTargetBossPlacementId || child->strActorProfileId != pattern.strActorProfileId)
                            { outStatus = "Completion candidate must be a distinct playable Pattern for the same Gate and boss."; return false; }
                            for (const auto& childBox : child->LogicOccurrences)
                            {
                                if (!childBox.bEnabled) continue;
                                const auto* childLogic = findLogic(childBox.strLogicId);
                                if (childLogic && childLogic->strJudgementKind == "PATTERN_COMPLETION_COUNT")
                                { outStatus = "Completion candidates cannot contain another chain."; return false; }
                                for (const auto slot : {KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS, KOUKU_SAYDON_OUTCOME_SLOT::FAIL, KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT})
                                    for (const auto& outcomeId : childBox.Outcomes(slot))
                                    {
                                        const auto* outcome = findLogic(outcomeId);
                                        if (outcome && (outcome->strOutcomeKind == "FOLLOWUP_PATTERN" || outcome->strOutcomeKind == "MARIO_ENTER"))
                                        { outStatus = "Completion candidates cannot insert another Pattern or Mario entry."; return false; }
                                    }
                            }
                        }
                    }
                    for (const auto slot : {KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS, KOUKU_SAYDON_OUTCOME_SLOT::FAIL, KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT})
                        for (const auto& id : box.Outcomes(slot))
                        {
                            const auto* result = findLogic(id);
                            if (result && result->strOutcomeKind == "MARIO_ENTER" &&
                                (chainCount != 1 || pattern.strGateId != "GATE3" || owner->strTriggerKind != "ENTER_AREA" ||
                                 slot != KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS || box.OnSuccessLogicIds.size() != 1u))
                            { outStatus = "Mario entry is the sole ENTER_AREA Success owned by a Gate 3 completion chain."; return false; }
                        }
                }
            }
            for (const auto& pattern : document.Patterns)
                if (!Validate_PatternFolder(document, pattern, outStatus) ||
                    !Validate_PatternChildren(document, pattern, outStatus)) return false;
            for (const auto& pattern : document.Patterns)
            {
                if (!pattern.strLoadError.empty()) continue;
                for (const auto& box : pattern.SummonOccurrences)
                    for (const auto& spawn : box.PatternSpawns)
                        if (!CKoukuSaydonCompositionDocument::Validate_SummonPatternTarget(document, pattern,
                            spawn.strPatternId, outStatus)) return false;
            }
            for (const auto& bundle : document.Bundles)
                if (!bundles.insert(bundle.strBundleId).second || !Validate_Bundle(document, bundle, outStatus)) return false;
        }
        std::unordered_set<std::string> flowIds, flowGates;
        for (const auto& flow : document.PatternFlows)
        {
            if (!Is_StableId(flow.strFlowId) || !flowIds.insert(flow.strFlowId).second ||
                !flowGates.insert(flow.strGateId).second || !Is_DisplayName(flow.strDisplayName) ||
                (flow.strGateId != "GATE1" && flow.strGateId != "GATE2" && flow.strGateId != "GATE3" && flow.strGateId != "BINGO") ||
                flow.Entries.size() > 256u)
            { outStatus = "Pattern Flow requires one bounded, named stable flow per Gate."; return false; }
            std::unordered_set<std::string> entries;
            for (const auto& entry : flow.Entries)
            {
                if (!Is_StableId(entry.strEntryId) || !entries.insert(entry.strEntryId).second ||
                    !Is_StableId(entry.strTargetId) || entry.iWaitAfterMs > MAX_TIME_MS ||
                    (entry.strKind != "PATTERN" && entry.strKind != "BUNDLE"))
                { outStatus = "Pattern Flow entry identity, kind, target or wait is invalid: " + entry.strEntryId; return false; }
                // Incomplete referenced rows remain editable; playback validates published availability.
                if (validatePatternLinks)
                {
                    const bool exists = entry.strKind == "PATTERN" ?
                        std::any_of(document.Patterns.begin(), document.Patterns.end(), [&](const auto& row) {
                            return row.strPatternId == entry.strTargetId && row.strGateId == flow.strGateId; }) :
                        std::any_of(document.Bundles.begin(), document.Bundles.end(), [&](const auto& row) {
                            return row.strBundleId == entry.strTargetId && row.strGateId == flow.strGateId; });
                    if (!exists) { outStatus = "Pattern Flow target is missing or belongs to another Gate: " + entry.strTargetId; return false; }
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

std::filesystem::path Client::CKoukuSaydonCompositionDocument::Resolve_SequencePath()
{
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Compositions/Sequences/KoukuSaydonSequenceComposition.json"));
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


namespace
{
    struct BOSS_PLACEMENT_CONTRACT { std::string_view gate, actor, placement, archetype; };
    constexpr std::array<BOSS_PLACEMENT_CONTRACT, 6u> BOSS_PLACEMENTS = {{
        {"GATE1", "MN_RPCZ_00", "boss.kakulsaydon.g1.kouku", "BOSS_KAKULSAYDON_G1_KOUKU"},
        {"GATE1", "MN_RPCT_05", "boss.kakulsaydon.g1.saydon", "BOSS_KAKULSAYDON_G1_SAYDON"},
        {"GATE2", "MN_RPCZ_00", "boss.kakulsaydon.g2.kouku", "BOSS_KAKULSAYDON_G2_KOUKU"},
        {"GATE2", "MN_RPCT_06", "boss.kakulsaydon.g2.big-saydon", "BOSS_KAKULSAYDON_G2_BIG_SAYDON"},
        {"GATE3", "MN_RPCT_05", "boss.kakulsaydon.g3.saydon", "BOSS_KAKULSAYDON_G3_SAYDON"},
        {"BINGO", "MN_RPCT_05", "boss.kakulsaydon.bingo.saydon", "BOSS_KAKULSAYDON_BINGO_SAYDON"},
    }};
}

bool_t Client::CKoukuSaydonCompositionDocument::Is_KnownGate(const std::string_view gateId)
{ return gateId == "GATE1" || gateId == "GATE2" || gateId == "GATE3" || gateId == "BINGO"; }

std::string_view Client::CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(
    const std::string_view gateId, const std::string_view actorProfileId)
{
    for (const auto& value : BOSS_PLACEMENTS)
        if (value.gate == gateId && value.actor == actorProfileId) return value.placement;
    return {};
}

std::string_view Client::CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(const std::string_view placementId)
{
    for (const auto& value : BOSS_PLACEMENTS) if (value.placement == placementId) return value.archetype;
    return {};
}

std::string_view Client::CKoukuSaydonCompositionDocument::Resolve_ActorProfileForPlacement(const std::string_view placementId)
{
    for (const auto& value : BOSS_PLACEMENTS) if (value.placement == placementId) return value.actor;
    return {};
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
			  "madnessPolicy", "nextFolderOrdinal", "nextBundleOrdinal", "folders", "bundles", "patternFlows" }))
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
		(staged.iFormatVersion < 1u || staged.iFormatVersion > FORMAT_VERSION) ||
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
	const bool_t legacyHierarchy = staged.iFormatVersion < 3u;
	staged.iFormatVersion = FORMAT_VERSION;
	staged.strCompositionId = compositionId->Get_String();
	staged.strEncounterId = encounterId->Get_String();
	staged.strBossArchetypeId = archetypeId->Get_String();
	staged.strBossPlacementId = placementId->Get_String();
	staged.strAreaId = areaId->Get_String();
    for (const auto& [name, destination] : {
        std::pair<const char*, uint32_t*>{"nextFolderOrdinal", &staged.iNextFolderOrdinal},
        {"nextBundleOrdinal", &staged.iNextBundleOrdinal}})
    {
        const auto* value = root.Find(name);
        if ((!value && !legacyHierarchy) || (value && (!Try_ParseUnsigned(*value, MAX_NEXT_ORDINAL, *destination) || !*destination)))
        { outStatus = "Invalid hierarchy counter: " + std::string(name); return false; }
    }

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
					{ "judgementKind", "fixedSelectionGroupId", "trackingPresentationOccurrenceId", "spawnIntervalMs", "followSpeedScale",
					  "randomVolleyOccurrenceSets", "randomSpawnIntervalMs", "randomArenaRadiusM", "randomArenaHeightToleranceM", "insideOutcome", "sectorCount", "sectorSymbols", "regionIds", "centerX", "centerZ",
					  "outerRadiusM", "worldSequenceInstanceId", "halfAngleDegrees",
					  "maxDistanceM", "poseIndex", "threshold", "shieldArcDegrees",
					  "endsPatternOnSuccess", "normalYawOffsetDegrees", "faceCenterYawOffsetDegrees", "outcomeKind", "percent", "durationMs", "pushRangeM", "pushMs", "pushDirection", "targetWorldInstanceId", "motionInstanceId", "targetRadiusM",
					  "directionPatternIds", "cloneEndStageId", "summonOccurrenceId", "airbornePhase", "airborneHeightM", "airborneDurationMs", "patternIds", "completionCount", "followupPatternId", "triggerKind", "countPerPlayer", "radiusM", "effectLifetimeMs",
					  "arenaRandomCount", "arenaRandomRadiusM", "arenaHeightToleranceM", "arenaMinimumSpacingM", "randomPlayerOnly",
					  "rearmOnExit", "repeatAfterKnockback", "bossChargeDistanceM", "chargeYawOffsetDegrees", "hudMode", "teleportPosition", "clonePatternId", "clockHours",
					  "targetWorldOccurrenceIds", "contactGroupId", "contactPriority", "contactMotions", "targetLogicOccurrenceId", "contactTargetWorldOccurrenceId", "sceneProfileId", "effectResourceId", "lightResourceId", "effectDelayMs", "attachmentSlot", "gripLocalOffset" }))
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
			const auto* randomSets = logicValue.Find("randomVolleyOccurrenceSets");
			const bool hasRandomFields = randomSets || logicValue.Find("randomSpawnIntervalMs") ||
				logicValue.Find("randomArenaRadiusM") || logicValue.Find("randomArenaHeightToleranceM");
			if (hasRandomFields)
			{
				if (!randomSets || !randomSets->Is_Array() || randomSets->Get_Array().empty() || randomSets->Get_Array().size() > 32u ||
					!logicValue.Find("randomSpawnIntervalMs") || !logicValue.Find("randomArenaRadiusM") || !logicValue.Find("randomArenaHeightToleranceM"))
				{ outStatus = "SHOWTIME random volley fields must be supplied together with 1..32 ordered sets."; return false; }
				for (const auto& set : randomSets->Get_Array())
				{
					if (!set.Is_Array() || set.Get_Array().empty() || set.Get_Array().size() > 64u)
					{ outStatus = "SHOWTIME random volley requires 1..64 occurrence IDs."; return false; }
					std::vector<std::string> ids;
					for (const auto& id : set.Get_Array())
					{
						if (!id.Is_String()) { outStatus = "SHOWTIME random occurrence ID must be text."; return false; }
						ids.push_back(id.Get_String());
					}
					stagedLogic.RandomVolleyOccurrenceSets.push_back(std::move(ids));
				}
			}
			const DATA_JSON_VALUE* const endsPattern = logicValue.Find("endsPatternOnSuccess");
			const DATA_JSON_VALUE* const rearmOnExit = logicValue.Find("rearmOnExit");
			const DATA_JSON_VALUE* const repeatAfterKnockback = logicValue.Find("repeatAfterKnockback");
			const DATA_JSON_VALUE* const randomPlayerOnly = logicValue.Find("randomPlayerOnly");
			if (!optionalText("judgementKind", stagedLogic.strJudgementKind) ||
				!optionalText("fixedSelectionGroupId", stagedLogic.strFixedSelectionGroupId) ||
				!optionalText("trackingPresentationOccurrenceId", stagedLogic.strTrackingPresentationOccurrenceId) ||
				!optionalUnsigned("spawnIntervalMs", MAX_TIME_MS, stagedLogic.iSpawnIntervalMs) ||
				!optionalFinite("followSpeedScale", .01, 10.0, stagedLogic.fFollowSpeedScale) ||
				!optionalUnsigned("randomSpawnIntervalMs", MAX_TIME_MS, stagedLogic.iRandomSpawnIntervalMs) ||
				!optionalFinite("randomArenaRadiusM", 0.0, 1000.0, stagedLogic.fRandomArenaRadiusM) ||
				!optionalFinite("randomArenaHeightToleranceM", 0.0, 10.0, stagedLogic.fRandomArenaHeightToleranceM) ||
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
				!Try_ParseTextList(logicValue.Find("directionPatternIds"), 4u, stagedLogic.DirectionPatternIds) ||
				!optionalText("cloneEndStageId", stagedLogic.strCloneEndStageId) ||
                !optionalText("summonOccurrenceId", stagedLogic.strSummonOccurrenceId) ||
				!Try_ParseTextList(logicValue.Find("patternIds"), 16u, stagedLogic.PatternIds) ||
				!optionalUnsigned("completionCount", 16u, stagedLogic.iCompletionCount) ||
				!optionalUnsigned("poseIndex", 7u, stagedLogic.iPoseIndex) ||
				!optionalUnsigned("threshold", (std::numeric_limits<std::uint32_t>::max)(), stagedLogic.iThreshold) ||
				!optionalFinite("shieldArcDegrees", 0.0, 360.0, stagedLogic.fShieldArcDegrees) ||
				!optionalFinite("normalYawOffsetDegrees", -360.0, 360.0, stagedLogic.fNormalYawOffsetDegrees) ||
				!optionalFinite("faceCenterYawOffsetDegrees", -360.0, 360.0, stagedLogic.fFaceCenterYawOffsetDegrees) ||
				(nullptr != endsPattern && !endsPattern->Is_Boolean()) ||
				!optionalText("outcomeKind", stagedLogic.strOutcomeKind) ||
				!optionalFinite("targetRadiusM", .01, 1000.0, stagedLogic.fTargetRadiusM) ||
				!optionalText("targetWorldInstanceId", stagedLogic.strTargetWorldInstanceId) ||
				!optionalText("motionInstanceId", stagedLogic.strMotionInstanceId) ||
				!optionalUnsigned("percent", 100u, stagedLogic.iPercent) ||
				!optionalUnsigned("durationMs", MAX_TIME_MS, stagedLogic.iDurationMs) ||
				!optionalText("pushDirection", stagedLogic.strPushDirection) ||
				!optionalFinite("pushRangeM", 0.0, 20.0, stagedLogic.fPushRangeM) ||
				!optionalUnsigned("pushMs", MAX_TIME_MS, stagedLogic.iPushMs) ||
				(nullptr != rearmOnExit && !rearmOnExit->Is_Boolean()) ||
				(nullptr != repeatAfterKnockback && !repeatAfterKnockback->Is_Boolean()) ||
				!optionalText("followupPatternId", stagedLogic.strFollowupPatternId) ||
				!optionalText("sceneProfileId", stagedLogic.strSceneProfileId) ||
				!optionalText("effectResourceId", stagedLogic.strEffectResourceId) ||
				!optionalText("lightResourceId", stagedLogic.strLightResourceId) ||
				!optionalText("attachmentSlot", stagedLogic.strAttachmentSlot) ||
				!optionalUnsigned("effectDelayMs", MAX_TIME_MS, stagedLogic.iEffectDelayMs) ||
				!optionalText("triggerKind", stagedLogic.strTriggerKind) ||
				!optionalText("airbornePhase", stagedLogic.strAirbornePhase) ||
				!optionalFinite("airborneHeightM", 0.0, 100000.0, stagedLogic.fAirborneHeightM) ||
				!optionalUnsigned("airborneDurationMs", MAX_TIME_MS, stagedLogic.iAirborneDurationMs) ||
				!optionalUnsigned("countPerPlayer", 8u, stagedLogic.iCountPerPlayer) ||
				!optionalFinite("radiusM", 0.0, 20.0, stagedLogic.fPlayerEffectRadiusM) ||
				!optionalUnsigned("effectLifetimeMs", MAX_TIME_MS, stagedLogic.iEffectLifetimeMs) ||
				!optionalUnsigned("arenaRandomCount", 32u, stagedLogic.iArenaRandomCount) ||
				!optionalFinite("arenaRandomRadiusM", 0.0, 100.0, stagedLogic.fArenaRandomRadiusM) ||
				!optionalFinite("arenaHeightToleranceM", 0.0, 10.0, stagedLogic.fArenaHeightToleranceM) ||
				!optionalFinite("arenaMinimumSpacingM", 0.0, 20.0, stagedLogic.fArenaMinimumSpacingM) ||
				(nullptr != randomPlayerOnly && !randomPlayerOnly->Is_Boolean()) ||
				!optionalFinite("bossChargeDistanceM", 0.0, 1000.0, stagedLogic.fBossChargeDistanceM) ||
                !optionalFinite("chargeYawOffsetDegrees", -360.0, 360.0, stagedLogic.fChargeYawOffsetDegrees) ||
				!optionalText("hudMode", stagedLogic.strHudMode) ||
				!optionalText("clonePatternId", stagedLogic.strClonePatternId) ||
				!Try_ParseTextList(logicValue.Find("targetWorldOccurrenceIds"), 64u, stagedLogic.TargetWorldOccurrenceIds) ||
				!optionalText("contactGroupId", stagedLogic.strContactGroupId) ||
				!optionalUnsigned("contactPriority", 1000u, stagedLogic.iContactPriority) ||
				!optionalText("targetLogicOccurrenceId", stagedLogic.strTargetLogicOccurrenceId) ||
				!optionalText("contactTargetWorldOccurrenceId", stagedLogic.strContactTargetWorldOccurrenceId))
			{
				outStatus = "KoukuSaydon Logic definition typed value is invalid: " + stagedLogic.strLogicId;
				return false;
			}
            const bool cross = stagedLogic.strLogicType == "DURATION" && stagedLogic.strJudgementKind == "CROSS_DIRECTION_CLONES";
            if ((cross != (logicValue.Find("directionPatternIds") != nullptr)) ||
                (cross != (logicValue.Find("cloneEndStageId") != nullptr)) ||
                (cross != (logicValue.Find("summonOccurrenceId") != nullptr)))
            { outStatus = "CROSS_DIRECTION_CLONES requires directionPatternIds, cloneEndStageId and summonOccurrenceId together."; return false; }
			const bool airborne = stagedLogic.strLogicType == "TRIGGER" && stagedLogic.strTriggerKind == "ALBION_AIRBORNE";
			if ((airborne && (!logicValue.Find("airbornePhase") || !logicValue.Find("airborneHeightM") ||
				!logicValue.Find("airborneDurationMs") || !logicValue.Find("teleportPosition"))) ||
				(!airborne && (logicValue.Find("airbornePhase") || logicValue.Find("airborneHeightM") || logicValue.Find("airborneDurationMs"))))
			{ outStatus = "ALBION_AIRBORNE requires phase, height, duration and teleportPosition together."; return false; }
			if (rearmOnExit) stagedLogic.bRearmOnExit = rearmOnExit->Get_Boolean();
			if (repeatAfterKnockback) stagedLogic.bRepeatAfterKnockback = repeatAfterKnockback->Get_Boolean();
			if (randomPlayerOnly) stagedLogic.bRandomPlayerOnly = randomPlayerOnly->Get_Boolean();
			if ((logicValue.Find("arenaRandomCount") || logicValue.Find("arenaRandomRadiusM") ||
				logicValue.Find("arenaHeightToleranceM") || logicValue.Find("arenaMinimumSpacingM") || randomPlayerOnly) &&
				(stagedLogic.strLogicType != "TRIGGER" || stagedLogic.strTriggerKind != "ALBION_BLUE_CIRCLE"))
			{ outStatus = "Arena circle placement and randomPlayerOnly belong to ALBION_BLUE_CIRCLE."; return false; }
			if ((logicValue.Find("pushDirection") && stagedLogic.strOutcomeKind != "MAX_HP_PERCENT_DAMAGE") ||
				(logicValue.Find("pushRangeM") != nullptr) != (logicValue.Find("pushMs") != nullptr) ||
				((logicValue.Find("pushRangeM") || logicValue.Find("pushMs")) && stagedLogic.strOutcomeKind != "MAX_HP_PERCENT_DAMAGE") ||
				((rearmOnExit || repeatAfterKnockback) && (stagedLogic.strLogicType != "TRIGGER" || stagedLogic.strTriggerKind != "ENTER_AREA")))
			{ outStatus = "Knockback fields belong together on a damage Result; rearmOnExit belongs to ENTER_AREA."; return false; }
			const auto* grip = logicValue.Find("gripLocalOffset");
			if ((stagedLogic.strOutcomeKind == "CAPTURE_PLAYER") != (grip != nullptr) ||
				(stagedLogic.strOutcomeKind == "CAPTURE_PLAYER") != (logicValue.Find("attachmentSlot") != nullptr))
			{ outStatus = "CAPTURE_PLAYER requires attachmentSlot and gripLocalOffset together."; return false; }
			if (grip)
			{
				if (!Has_Properties(*grip, {"forwardM", "upM", "rightM"}, {}))
				{ outStatus = "Capture gripLocalOffset requires forwardM/upM/rightM."; return false; }
				std::size_t axis = 0u;
				for (const auto* field : {"forwardM", "upM", "rightM"})
				{
					const auto* value = grip->Find(field);
					if (!value || !Try_ParseFinite(*value, -10.0, 10.0, stagedLogic.GripLocalOffset[axis++]))
					{ outStatus = "Capture grip offset must be finite -10..10 m."; return false; }
				}
			}
			if (const auto* targets = logicValue.Find("targetWorldOccurrenceIds"); targets &&
				targets->Get_Array().size() != stagedLogic.TargetWorldOccurrenceIds.size())
			{ outStatus = "Contact target placement IDs must not be empty."; return false; }
			if (const auto* mappings = logicValue.Find("contactMotions"))
			{
				if (!mappings->Is_Array() || mappings->Get_Array().size() > 64u)
				{ outStatus = "contactMotions must contain up to 64 target/motion mappings."; return false; }
				for (const auto& row : mappings->Get_Array())
				{
					if (!Has_Properties(row, { "targetWorldOccurrenceId", "motionInstanceId" }, {}))
					{ outStatus = "Contact motion mapping has unexpected properties."; return false; }
					const auto* target = Required(row, "targetWorldOccurrenceId", DATA_JSON_TYPE::STRING);
					const auto* motion = Required(row, "motionInstanceId", DATA_JSON_TYPE::STRING);
					if (!target || !motion) { outStatus = "Contact motion mapping must contain stable IDs."; return false; }
					stagedLogic.ContactMotions.push_back({target->Get_String(), motion->Get_String()});
				}
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
			if (!Has_Properties(summonValue, { "summonId", "displayName" },
                { "summonKind", "directionPatternIds", "cloneEndStageId" }))
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
            const auto* kind = summonValue.Find("summonKind");
            const auto* directions = summonValue.Find("directionPatternIds");
            const auto* cutoff = summonValue.Find("cloneEndStageId");
            if (kind || directions || cutoff)
            {
                if (!kind || !kind->Is_String() || kind->Get_String() != "CROSS_DIRECTION_CLONES" ||
                    !directions || !cutoff || !cutoff->Is_String() ||
                    !Try_ParseTextList(directions, 4u, stagedSummon.DirectionPatternIds))
                { outStatus = "Cross direction Summon requires summonKind, directionPatternIds and cloneEndStageId together."; return false; }
                stagedSummon.strSummonKind = kind->Get_String();
                stagedSummon.strCloneEndStageId = cutoff->Get_String();
            }
            if (!Validate_SummonPolicy(stagedSummon, outStatus)) return false;
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
			if (!Has_Properties(worldValue, { "worldId", "displayName", "sequenceInstanceId" }, { "positionOffset", "anchorKind", "anchorPosition", "companionEffectResourceId", "objectResourceId" }))
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
			if (!Read_PresentationText(worldValue, "objectResourceId", stagedWorld.strObjectResourceId) ||
				!Read_PresentationText(worldValue, "anchorKind", stagedWorld.strAnchorKind) ||
				!Read_PresentationText(worldValue, "companionEffectResourceId", stagedWorld.strCompanionEffectResourceId) ||
				!Read_PresentationVector(worldValue, "anchorPosition", stagedWorld.AnchorPosition, -100000.0, 100000.0))
			{ outStatus = "Invalid World anchor."; return false; }
			if (worldValue.Find("objectResourceId") && stagedWorld.strObjectResourceId.empty())
			{ outStatus = "World objectResourceId must be a nonempty Object ID when present."; return false; }
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
				  "nextPresentationOccurrenceOrdinal", "presentationOccurrences", "enterCombatOnFinish", "resetBossToSpawn", "resetBossYawDegrees", "bossMotion", "animationRootVerticalScale", "gateId", "targetBossPlacementId", "folderId", "durationMs", "nextPatternOccurrenceOrdinal", "patternOccurrences" }) :
			Has_Properties(patternValue,
				{ "patternId", "actorProfileId", "displayName", "authoringStatus", "category",
				  "nextStageOrdinal", "nextAnimationOrdinal", "stages" },
				{ "nextLogicOccurrenceOrdinal", "logicOccurrences",
				  "nextSummonOccurrenceOrdinal", "summonOccurrences",
				  "nextWorldOccurrenceOrdinal", "worldOccurrences",
				  "nextSceneProfileOccurrenceOrdinal", "sceneProfileOccurrences",
				  "nextPresentationOccurrenceOrdinal", "presentationOccurrences", "enterCombatOnFinish", "resetBossToSpawn", "resetBossYawDegrees", "bossMotion", "animationRootVerticalScale", "gateId", "targetBossPlacementId", "folderId", "durationMs", "nextPatternOccurrenceOrdinal", "patternOccurrences" });
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
			if (!Has_Properties(stageValue,
					{ "stageId", "actionId", "stageKind", "durationMs",
					  "animationOccurrences" }, { "retargetOnEnter" }))
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
			if (const DATA_JSON_VALUE* retarget = stageValue.Find("retargetOnEnter"))
			{
				if (!retarget->Is_Boolean())
				{ outStatus = "KoukuSaydon Stage retargetOnEnter must be boolean."; return false; }
				stagedStage.bRetargetOnEnter = retarget->Get_Boolean();
			}
			parsedOccurrences += occurrences->Get_Array().size();
			stagedStage.AnimationOccurrences.reserve(occurrences->Get_Array().size());

			for (const DATA_JSON_VALUE& occurrenceValue : occurrences->Get_Array())
			{
				if (!Has_Properties(occurrenceValue,
						{ "occurrenceId", "profileId", "sourceActionId",
						  "sourceStageId", "sourceSlotId", "referenceRevision",
						  "runtimeClip", "startOffsetMs", "sourceStartMs",
						  "playMs", "playRate", "endPolicy" }, { "blendInMs", "sourceEndMs" }))
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
				const auto* blendInMs = occurrenceValue.Find("blendInMs");
                const auto* sourceEndMs = occurrenceValue.Find("sourceEndMs");
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
                    (sourceEndMs && !Try_ParseUnsigned(*sourceEndMs, MAX_TIME_MS, stagedOccurrence.iSourceEndMs)) ||
					nullptr == playMs ||
					!Try_ParseUnsigned(*playMs, MAX_TIME_MS,
						stagedOccurrence.iPlayMs) || 0u == stagedOccurrence.iPlayMs ||
					nullptr == playRate ||
					!Try_ParsePlayRate(*playRate, stagedOccurrence.fPlayRate) ||
					nullptr == endPolicy || (blendInMs &&
					!Try_ParseUnsigned(*blendInMs, 1000u, stagedOccurrence.iBlendInMs)))
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
						  "onSuccessLogicIds", "onFailLogicIds", "onTimeoutLogicIds", "enabled", "holdLogicOccurrenceId", "roomPlayerArrival" }))
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
				if (const auto* arrival = boxValue.Find("roomPlayerArrival"))
				{
					KOUKU_SAYDON_ROOM_PLAYER_ARRIVAL value;
					const auto* slot = Required(*arrival, "playerSlot", DATA_JSON_TYPE::NUMBER);
					const auto* position = Required(*arrival, "position", DATA_JSON_TYPE::ARRAY);
					if (!Has_Properties(*arrival, { "playerSlot", "position" }, {}) || !slot || !position ||
						!Try_ParseUnsigned(*slot, 3u, value.iPlayerSlot) || position->Get_Array().size() != 3u)
					{ outStatus = "Room player arrival needs slot 0..3 and XYZ."; return false; }
					for (std::size_t axis = 0u; axis < 3u; ++axis)
						if (!Try_ParseFinite(position->Get_Array()[axis], -100000.0, 100000.0, value.Position[axis]))
						{ outStatus = "Room player arrival position must be finite."; return false; }
					stagedBox.RoomPlayerArrival = value;
				}
				if (const auto* hold = boxValue.Find("holdLogicOccurrenceId"))
				{
					if (!hold->Is_String()) { outStatus = "Hold Logic occurrence ID must be text."; return false; }
					stagedBox.strHoldLogicOccurrenceId = hold->Get_String();
				}
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
				if (!Has_Properties(boxValue,
						{ "occurrenceId", "summonId", "startMs", "durationMs" }, { "patternSpawns" }))
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
                if (const auto* spawns = boxValue.Find("patternSpawns"))
                {
                    if (!spawns->Is_Array() || spawns->Get_Array().size() > MAX_SUMMON_PATTERN_SPAWNS)
                    { outStatus = "Summon patternSpawns requires at most four rows."; return false; }
                    for (const auto& value : spawns->Get_Array())
                    {
                        KOUKU_SAYDON_COMPOSITION_SUMMON_PATTERN_SPAWN spawn;
                        const auto* id = Required(value, "spawnId", DATA_JSON_TYPE::STRING);
                        const auto* target = Required(value, "patternId", DATA_JSON_TYPE::STRING);
                        const auto* yaw = Required(value, "yawOffsetDegrees", DATA_JSON_TYPE::NUMBER);
                        if (!Has_ExactProperties(value, { "spawnId", "patternId", "positionOffset", "yawOffsetDegrees" }) ||
                            !id || !target || !yaw || !value.Find("positionOffset") ||
                            !Read_PresentationVector(value, "positionOffset", spawn.PositionOffset, -1000.0, 1000.0) ||
                            !Try_ParseFinite(*yaw, -360.0, 360.0, spawn.fYawOffsetDegrees))
                        { outStatus = "Summon Pattern spawn fields or transform are invalid."; return false; }
                        spawn.strSpawnId = id->Get_String(); spawn.strPatternId = target->Get_String();
                        stagedBox.PatternSpawns.push_back(std::move(spawn));
                    }
                }
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
				if (!Has_Properties(boxValue,
						{ "occurrenceId", "worldId", "startMs", "durationMs", "playbackSpeed" }, { "placement" }))
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
				if (const auto* placementValue = boxValue.Find("placement"))
				{
					KOUKU_SAYDON_WORLD_PLACEMENT placement;
					if (!Has_ExactProperties(*placementValue, { "position", "rotationDegrees", "scale" }) ||
						!Read_PresentationVector(*placementValue, "position", placement.Position, -100000.0, 100000.0) ||
						!Read_PresentationVector(*placementValue, "rotationDegrees", placement.RotationDegrees, -36000.0, 36000.0) ||
						!Read_PresentationVector(*placementValue, "scale", placement.Scale, 0.001, 1000.0))
					{ outStatus = "World occurrence placement requires position, rotationDegrees and positive scale vectors."; return false; }
					stagedBox.Placement = placement;
				}
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
		if (const auto* completion = patternValue.Find("enterCombatOnFinish"); nullptr != completion)
		{
			if (!completion->Is_Boolean()) { outStatus = "Invalid enterCombatOnFinish."; return false; }
			stagedPattern.bEnterCombatOnFinish = completion->Get_Boolean();
		}
		if (const auto* reset = patternValue.Find("resetBossToSpawn"); nullptr != reset)
		{
			if (!reset->Is_Boolean()) { outStatus = "Invalid resetBossToSpawn."; return false; }
			stagedPattern.bResetBossToSpawn = reset->Get_Boolean();
		}
		if (const auto* yaw = patternValue.Find("resetBossYawDegrees"); nullptr != yaw)
		{
			double degrees = 0.0;
			if (!stagedPattern.bResetBossToSpawn || !Try_ParseFinite(*yaw, -360.0, 360.0, degrees))
			{ outStatus = "Invalid resetBossYawDegrees: enable spawn reset and use -360..360 degrees."; return false; }
			stagedPattern.ResetBossYawDegrees = degrees;
		}
		if (const auto* scale = patternValue.Find("animationRootVerticalScale"); nullptr != scale)
			if (!Try_ParseFinite(*scale, 0.0, 1.0, stagedPattern.fAnimationRootVerticalScale))
			{ outStatus = "Invalid animationRootVerticalScale: use 0..1."; return false; }
		if (const auto* value = patternValue.Find("bossMotion"); nullptr != value)
		{
			KOUKU_SAYDON_BOSS_MOTION motion;
			if (!Has_ExactProperties(*value, { "startMs", "endMs", "startPosition", "endPosition", "yawDegrees" }) ||
				!Try_ParseUnsigned(*value->Find("startMs"), MAX_TIME_MS, motion.iStartMs) ||
				!Try_ParseUnsigned(*value->Find("endMs"), MAX_TIME_MS, motion.iEndMs) ||
				!Read_PresentationVector(*value, "startPosition", motion.StartPosition, -100000.0, 100000.0) ||
				!Read_PresentationVector(*value, "endPosition", motion.EndPosition, -100000.0, 100000.0) ||
				!Try_ParseFinite(*value->Find("yawDegrees"), -360.0, 360.0, motion.fYawDegrees))
			{ outStatus = "Invalid Boss Motion fields."; return false; }
			stagedPattern.BossMotion = motion;
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
		if (legacyHierarchy)
        {
            stagedPattern.strGateId = (stagedPattern.strPatternId == "KAKULSAYDON_G1_PATTERN_8" ||
                stagedPattern.strPatternId == "KAKULSAYDON_G1_PATTERN_9") ? "GATE2" : "GATE1";
            stagedPattern.strTargetBossPlacementId = Resolve_DefaultPlacementId(stagedPattern.strGateId, stagedPattern.strActorProfileId);
        }
        else
        {
            const auto* gate = Required(patternValue, "gateId", DATA_JSON_TYPE::STRING);
            const auto* target = Required(patternValue, "targetBossPlacementId", DATA_JSON_TYPE::STRING);
            if (!gate || !target) { outStatus = "Pattern Gate and target boss are required."; return false; }
            stagedPattern.strGateId = gate->Get_String();
            stagedPattern.strTargetBossPlacementId = target->Get_String();
        }
        if (const auto* folder = patternValue.Find("folderId"))
        {
            if (!folder->Is_String() || !Is_StableId(folder->Get_String()))
            { outStatus = "Pattern folderId must be a nonempty stable Parent ID."; return false; }
            stagedPattern.strFolderId = folder->Get_String();
        }
        if (const auto* value = patternValue.Find("durationMs"))
            if (!Try_ParseUnsigned(*value, MAX_TIME_MS, stagedPattern.iDurationMs))
            { outStatus = "Invalid Pattern durationMs."; return false; }
        if (const auto* value = patternValue.Find("nextPatternOccurrenceOrdinal"))
            if (!Try_ParseUnsigned(*value, MAX_NEXT_ORDINAL, stagedPattern.iNextPatternOccurrenceOrdinal) ||
                !stagedPattern.iNextPatternOccurrenceOrdinal)
            { outStatus = "Invalid nextPatternOccurrenceOrdinal."; return false; }
        if (const auto* rows = patternValue.Find("patternOccurrences"))
        {
            if (!rows->Is_Array() || rows->Get_Array().size() > MAX_PATTERN_OCCURRENCES)
            { outStatus = "Invalid Pattern occurrence collection."; return false; }
            for (const auto& value : rows->Get_Array())
            {
                KOUKU_SAYDON_COMPOSITION_PATTERN_OCCURRENCE row;
                const auto* id = Required(value, "occurrenceId", DATA_JSON_TYPE::STRING);
                const auto* child = Required(value, "patternId", DATA_JSON_TYPE::STRING);
                const auto* start = value.Find("startMs");
                const auto* duration = value.Find("durationMs");
                const auto* repeat = value.Find("repeat");
                if (!Has_Properties(value, {"occurrenceId", "patternId", "startMs", "durationMs"}, {"repeat"}) ||
                    !id || !child || !start || !duration ||
                    !Try_ParseUnsigned(*start, MAX_TIME_MS, row.iStartMs) ||
                    !Try_ParseUnsigned(*duration, MAX_TIME_MS, row.iDurationMs) || !row.iDurationMs ||
                    (repeat && !repeat->Is_Boolean()))
                { outStatus = "Invalid Pattern occurrence fields."; return false; }
                row.strOccurrenceId = id->Get_String(); row.strPatternId = child->Get_String();
                row.bRepeat = repeat && repeat->Get_Boolean();
                stagedPattern.PatternOccurrences.push_back(std::move(row));
            }
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
			const auto* gate = Required(patternValue, "gateId", DATA_JSON_TYPE::STRING);
            if (gate) stagedPattern.strGateId = gate->Get_String();
            const auto* folder = Required(patternValue, "folderId", DATA_JSON_TYPE::STRING);
            if (folder) stagedPattern.strFolderId = folder->Get_String();
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


    const auto readText = [](const DATA_JSON_VALUE& source, const char* name, std::string& value)
    { const auto* field = Required(source, name, DATA_JSON_TYPE::STRING); if (!field) return false; value = field->Get_String(); return true; };
    const auto readOrdinal = [](const DATA_JSON_VALUE& source, const char* name, uint32_t& value)
    { const auto* field = source.Find(name); return field && Try_ParseUnsigned(*field, MAX_NEXT_ORDINAL, value) && value > 0; };
    const auto preserve = [&](const auto& source, auto& item, std::string& identity, const char* key, const auto& previous)
    {
        item.strLoadError = outStatus.empty() ? "Invalid hierarchy entry" : outStatus;
        item.strPreservedJson = Preserve_Json(source);
        readText(source, "displayName", item.strDisplayName);
        readText(source, "gateId", item.strGateId);
        readText(source, key, identity);
        const auto used = [&](const std::string& id) { return std::any_of(previous.begin(), previous.end(), [&](const auto& row) {
            if constexpr (requires { row.strFolderId; row.strBundleId; }) return row.strBundleId == id;
            else return row.strFolderId == id;
        }); };
        if (!Is_StableId(identity) || used(identity))
        {
            uint64_t hash = 14695981039346656037ull;
            for (unsigned char c : item.strPreservedJson) hash = (hash ^ c) * 1099511628211ull;
            identity = "invalid." + std::to_string(hash);
            while (used(identity)) identity += ".duplicate";
        }
        if (item.strDisplayName.empty()) item.strDisplayName = identity;
    };
    const auto* folders = root.Find("folders");
    const auto* bundles = root.Find("bundles");
    if ((!legacyHierarchy && (!folders || !bundles)) ||
        (folders && (!folders->Is_Array() || folders->Get_Array().size() > 4096u)) ||
        (bundles && (!bundles->Is_Array() || bundles->Get_Array().size() > 4096u)))
    { outStatus = "Invalid folder/bundle collections."; return false; }
    if (folders) for (const auto& value : folders->Get_Array())
    {
        KOUKU_SAYDON_COMPOSITION_FOLDER folder;
        outStatus.clear();
        if (!Has_Properties(value, {"folderId", "gateId", "displayName"}, {"timelinePatternId"}) ||
            (value.Find("timelinePatternId") && !readText(value, "timelinePatternId", folder.strTimelinePatternId)) ||
            !readText(value, "folderId", folder.strFolderId) || !readText(value, "gateId", folder.strGateId) ||
            !readText(value, "displayName", folder.strDisplayName) || !Validate_Folder(staged, folder, outStatus) ||
            std::any_of(staged.Folders.begin(), staged.Folders.end(), [&](const auto& row) { return row.strFolderId == folder.strFolderId; }))
            preserve(value, folder, folder.strFolderId, "folderId", staged.Folders);
        staged.Folders.push_back(std::move(folder));
    }
    // Parents are parsed after Patterns. Quarantine a broken folder reference
    // only after the complete Parent inventory is available.
    for (size_t index = 0; index < staged.Patterns.size(); ++index)
    {
        auto& pattern = staged.Patterns[index];
        if (Validate_PatternFolder(staged, pattern, outStatus)) continue;
        pattern.strLoadError = outStatus;
        pattern.strPreservedJson = Preserve_Json(patterns->Get_Array()[index]);
        pattern.strAuthoringStatus = "DRAFT";
    }
    for (size_t index = 0; index < staged.Patterns.size(); ++index)
    {
        auto& pattern = staged.Patterns[index];
        if (!pattern.strLoadError.empty()) continue;
        bool valid = true;
        for (const auto& box : pattern.SummonOccurrences)
            for (const auto& spawn : box.PatternSpawns)
                if (!Validate_SummonPatternTarget(staged, pattern, spawn.strPatternId, outStatus)) { valid = false; break; }
        if (valid) continue;
        pattern.strLoadError = outStatus;
        pattern.strPreservedJson = Preserve_Json(patterns->Get_Array()[index]);
        pattern.strAuthoringStatus = "DRAFT";
    }
    for (size_t index = 0; index < staged.Patterns.size(); ++index)
    {
        auto& pattern = staged.Patterns[index];
        if (Validate_PatternChildren(staged, pattern, outStatus)) continue;
        pattern.strLoadError = outStatus;
        pattern.strPreservedJson = Preserve_Json(patterns->Get_Array()[index]);
        pattern.strAuthoringStatus = "DRAFT";
    }
    if (bundles) for (const auto& value : bundles->Get_Array())
    {
        KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
        outStatus.clear();
        const auto parse = [&]() -> bool
        {
            if (!Has_ExactProperties(value, {"bundleId", "gateId", "folderId", "displayName", "authoringStatus",
                "nextMemberOrdinal", "nextSceneProfileOccurrenceOrdinal", "nextPresentationOccurrenceOrdinal",
                "members", "sceneProfileOccurrences", "presentationOccurrences"}) ||
                !readText(value, "bundleId", bundle.strBundleId) || !readText(value, "gateId", bundle.strGateId) ||
                !readText(value, "folderId", bundle.strFolderId) || !readText(value, "displayName", bundle.strDisplayName) ||
                !readText(value, "authoringStatus", bundle.strAuthoringStatus) ||
                !readOrdinal(value, "nextMemberOrdinal", bundle.iNextMemberOrdinal) ||
                !readOrdinal(value, "nextSceneProfileOccurrenceOrdinal", bundle.iNextSceneProfileOccurrenceOrdinal) ||
                !readOrdinal(value, "nextPresentationOccurrenceOrdinal", bundle.iNextPresentationOccurrenceOrdinal)) return false;
            const auto* members = Required(value, "members", DATA_JSON_TYPE::ARRAY);
            const auto* scenes = Required(value, "sceneProfileOccurrences", DATA_JSON_TYPE::ARRAY);
            const auto* presentation = Required(value, "presentationOccurrences", DATA_JSON_TYPE::ARRAY);
            if (!members || members->Get_Array().size() > 8u || !scenes || scenes->Get_Array().size() > 16u ||
                !presentation || presentation->Get_Array().size() > 16u) return false;
            for (const auto& row : members->Get_Array())
            {
                KOUKU_SAYDON_COMPOSITION_BUNDLE_MEMBER member;
                const auto* offset = row.Find("startOffsetMs");
                if (!Has_ExactProperties(row, {"memberId", "patternId", "startOffsetMs"}) ||
                    !readText(row, "memberId", member.strMemberId) || !readText(row, "patternId", member.strPatternId) ||
                    !offset || !Try_ParseUnsigned(*offset, MAX_TIME_MS, member.iStartOffsetMs)) return false;
                bundle.Members.push_back(std::move(member));
            }
            for (const auto& row : scenes->Get_Array())
            {
                KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE scene;
                if (!Has_ExactProperties(row, {"occurrenceId", "sceneProfileId", "startMs", "durationMs", "blendMs"}) ||
                    !readText(row, "occurrenceId", scene.strOccurrenceId) || !readText(row, "sceneProfileId", scene.strSceneProfileId)) return false;
                for (const auto& [name, destination] : {
                    std::pair<const char*, uint32_t*>{"startMs", &scene.iStartMs}, {"durationMs", &scene.iDurationMs}, {"blendMs", &scene.iBlendMs}})
                { const auto* field = row.Find(name); if (!field || !Try_ParseUnsigned(*field, MAX_TIME_MS, *destination)) return false; }
                bundle.SceneProfileOccurrences.push_back(std::move(scene));
            }
            for (const auto& row : presentation->Get_Array())
            {
                KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE occurrence;
                if (!Read_PresentationOccurrence(row, occurrence)) return false;
                bundle.PresentationOccurrences.push_back(std::move(occurrence));
            }
            return Validate_Bundle(staged, bundle, outStatus) &&
                std::none_of(staged.Bundles.begin(), staged.Bundles.end(), [&](const auto& row) { return row.strBundleId == bundle.strBundleId; });
        };
        if (!parse()) { preserve(value, bundle, bundle.strBundleId, "bundleId", staged.Bundles); bundle.strAuthoringStatus = "DRAFT"; }
        staged.Bundles.push_back(std::move(bundle));
    }

	if (const auto* flows = root.Find("patternFlows"))
	{
		if (!flows->Is_Array() || flows->Get_Array().size() > 4u)
		{ outStatus = "Pattern Flows must be a bounded array."; return false; }
		for (const auto& value : flows->Get_Array())
		{
			KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW flow;
			const auto* entries = Required(value, "entries", DATA_JSON_TYPE::ARRAY);
			if (!Has_ExactProperties(value, {"flowId", "gateId", "displayName", "entries"}) ||
				!readText(value, "flowId", flow.strFlowId) || !readText(value, "gateId", flow.strGateId) ||
				!readText(value, "displayName", flow.strDisplayName) || !entries || entries->Get_Array().size() > 256u)
			{ outStatus = "Pattern Flow properties are invalid."; return false; }
			for (const auto& row : entries->Get_Array())
			{
				KOUKU_SAYDON_COMPOSITION_FLOW_ENTRY entry;
				const auto* wait = row.Find("waitAfterMs");
				if (!Has_ExactProperties(row, {"entryId", "kind", "targetId", "waitAfterMs"}) ||
					!readText(row, "entryId", entry.strEntryId) || !readText(row, "kind", entry.strKind) ||
					!readText(row, "targetId", entry.strTargetId) || !wait ||
					!Try_ParseUnsigned(*wait, MAX_TIME_MS, entry.iWaitAfterMs))
				{ outStatus = "Pattern Flow entry properties are invalid."; return false; }
				flow.Entries.push_back(std::move(entry));
			}
			staged.PatternFlows.push_back(std::move(flow));
		}
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

bool_t Client::CKoukuSaydonCompositionDocument::Try_ResolveCrossDirectionWindows(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
    std::vector<KOUKU_SAYDON_COMPOSITION_CROSS_DIRECTION_WINDOW>& outWindows,
    std::string& outStatus)
{
    if (!Validate_CrossDirectionWindows(document, pattern, outStatus)) return false;
    if (!Resolve_CrossDirectionWindows(document, pattern, outWindows, outStatus)) return false;
    outStatus.clear();
    return true;
}

bool_t Client::CKoukuSaydonCompositionDocument::Validate_SummonPatternTarget(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& owner,
    const std::string_view targetPatternId, std::string& outStatus)
{
    const auto target = std::find_if(document.Patterns.begin(), document.Patterns.end(),
        [&](const auto& row) { return row.strPatternId == targetPatternId; });
    const auto fail = [&](const char* reason) {
        outStatus = "Summon Pattern " + std::string(targetPatternId) + ": " + reason; return false;
    };
    if (target == document.Patterns.end() || !target->strLoadError.empty())
        return fail("target is missing or invalid.");
    if (target->strPatternId == owner.strPatternId || target->strGateId != owner.strGateId ||
        target->strActorProfileId != owner.strActorProfileId || target->strCategory != "MECHANIC")
        return fail("choose another MECHANIC Pattern with the same Gate and actor.");
    if (!target->PatternOccurrences.empty() || !target->LogicOccurrences.empty() || !target->SummonOccurrences.empty() ||
        !target->WorldOccurrences.empty() || !target->SceneProfileOccurrences.empty() || !target->PresentationOccurrences.empty() ||
        target->BossMotion || target->bEnterCombatOnFinish || target->bResetBossToSpawn || target->ResetBossYawDegrees ||
        std::any_of(document.Folders.begin(), document.Folders.end(), [&](const auto& folder) {
            return folder.strTimelinePatternId == target->strPatternId;
        }) || std::any_of(target->Stages.begin(), target->Stages.end(), [](const auto& stage) { return stage.bRetargetOnEnter; }))
        return fail("only Animation rows are supported; nested Summon, Parent, Logic, other lanes and boss movement are unavailable.");
    if (!Pattern_Lifetime(*target) || std::none_of(target->Stages.begin(), target->Stages.end(),
        [](const auto& stage) { return !stage.AnimationOccurrences.empty(); }))
        return fail("target needs a nonempty Animation timeline.");
    outStatus.clear();
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
        << "  \"nextFolderOrdinal\": " << document.iNextFolderOrdinal << ",\n"
        << "  \"nextBundleOrdinal\": " << document.iNextBundleOrdinal << ",\n"
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
			if ("SHOWTIME_PLAYER_TARGETS" == logic.strJudgementKind)
			{
				output << ",\n      \"fixedSelectionGroupId\": \"" << CDataJson::Escape(logic.strFixedSelectionGroupId)
					<< "\",\n      \"trackingPresentationOccurrenceId\": \"" << CDataJson::Escape(logic.strTrackingPresentationOccurrenceId)
					<< "\",\n      \"spawnIntervalMs\": " << logic.iSpawnIntervalMs
					<< ",\n      \"followSpeedScale\": " << logic.fFollowSpeedScale;
				if (!logic.RandomVolleyOccurrenceSets.empty())
				{
					output << ",\n      \"randomVolleyOccurrenceSets\": [";
					for (std::size_t set = 0; set < logic.RandomVolleyOccurrenceSets.size(); ++set)
					{
						output << (set ? ", [" : "[");
						const auto& ids = logic.RandomVolleyOccurrenceSets[set];
						for (std::size_t i = 0; i < ids.size(); ++i)
							output << (i ? ", \"" : "\"") << CDataJson::Escape(ids[i]) << "\"";
						output << "]";
					}
					output << "],\n      \"randomSpawnIntervalMs\": " << logic.iRandomSpawnIntervalMs
						<< ",\n      \"randomArenaRadiusM\": " << logic.fRandomArenaRadiusM
						<< ",\n      \"randomArenaHeightToleranceM\": " << logic.fRandomArenaHeightToleranceM;
				}
			}
			else if ("CROSS_DIRECTION_CLONES" == logic.strJudgementKind)
            {
                output << ",\n      \"cloneEndStageId\": \"" << CDataJson::Escape(logic.strCloneEndStageId) << "\",\n";
                output << "      \"summonOccurrenceId\": \"" << CDataJson::Escape(logic.strSummonOccurrenceId) << "\",\n";
                textList("directionPatternIds", logic.DirectionPatternIds, "      ", true);
            }
			else if ("PATTERN_COMPLETION_COUNT" == logic.strJudgementKind)
			{
				output << ",\n      \"completionCount\": " << logic.iCompletionCount << ",\n";
				textList("patternIds", logic.PatternIds, "      ", true);
			}
			else if ("ROULETTE_CARD_MATCH" == logic.strJudgementKind)
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
			else if ("EXTERNAL_SIGNAL" == logic.strJudgementKind || "COUNTER_WINDOW" == logic.strJudgementKind)
				output << ",\n      \"endsPatternOnSuccess\": " << (logic.bEndsPatternOnSuccess ? "true" : "false");
			else if ("OBJECT_OVERLAP" == logic.strJudgementKind)
				output << ",\n      \"targetWorldInstanceId\": \"" << CDataJson::Escape(logic.strTargetWorldInstanceId)
					<< "\",\n      \"targetRadiusM\": " << logic.fTargetRadiusM
					<< ",\n      \"insideOutcome\": \"" << logic.strInsideOutcome << "\"";
			else if ("AREA_OVERLAP" == logic.strJudgementKind)
			{
				output << ",\n      \"insideOutcome\": \"" << logic.strInsideOutcome << "\"";
			}
			else if ("GAZE_REAL_BOSS" == logic.strJudgementKind)
			{
				output << ",\n      \"halfAngleDegrees\": " << logic.fHalfAngleDegrees
					<< ",\n      \"maxDistanceM\": " << logic.fMaxDistanceM
					<< ",\n      \"insideOutcome\": \"" << logic.strInsideOutcome << "\"";
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
			if (logic.strOutcomeKind == "MAX_HP_PERCENT_DAMAGE" && (logic.fPushRangeM != 0.0 || logic.iPushMs != 0u))
				output << ",\n      \"pushRangeM\": " << logic.fPushRangeM
					<< ",\n      \"pushMs\": " << logic.iPushMs;
			if (logic.strPushDirection != "AWAY_FROM_BOSS")
				output << ",\n      \"pushDirection\": \"" << logic.strPushDirection << "\"";
			if (logic.strOutcomeKind == "FEAR")
                output << ",\n      \"sceneProfileId\": \"" << CDataJson::Escape(logic.strSceneProfileId)
                    << "\",\n      \"effectResourceId\": \"" << CDataJson::Escape(logic.strEffectResourceId)
                    << "\",\n      \"lightResourceId\": \"" << CDataJson::Escape(logic.strLightResourceId)
                    << "\",\n      \"effectDelayMs\": " << logic.iEffectDelayMs;
			if (logic.strOutcomeKind == "CAPTURE_PLAYER")
				output << ",\n      \"attachmentSlot\": \"" << CDataJson::Escape(logic.strAttachmentSlot)
					<< "\",\n      \"gripLocalOffset\": {\"forwardM\": " << logic.GripLocalOffset[0]
					<< ", \"upM\": " << logic.GripLocalOffset[1] << ", \"rightM\": " << logic.GripLocalOffset[2] << "}";
			if (logic.strOutcomeKind == "PLAY_WORLD_OBJECT_MOTION")
				output << ",\n      \"targetWorldInstanceId\": \"" << CDataJson::Escape(logic.strTargetWorldInstanceId)
					<< "\",\n      \"motionInstanceId\": \"" << CDataJson::Escape(logic.strMotionInstanceId) << "\"";
			if (logic.strOutcomeKind == "PLAY_CONTACT_WORLD_OBJECT_MOTION")
			{
				output << ",\n      \"contactMotions\": [";
				for (std::size_t i = 0; i < logic.ContactMotions.size(); ++i)
				{
					const auto& row = logic.ContactMotions[i];
					output << (i ? ", " : "") << "{\"targetWorldOccurrenceId\": \"" << CDataJson::Escape(row.strTargetWorldOccurrenceId)
						<< "\", \"motionInstanceId\": \"" << CDataJson::Escape(row.strMotionInstanceId) << "\"}";
				}
				output << "]";
			}
			else if (logic.strOutcomeKind == "COMPLETE_LOGIC_WINDOW")
				output << ",\n      \"targetLogicOccurrenceId\": \"" << CDataJson::Escape(logic.strTargetLogicOccurrenceId)
					<< "\",\n      \"contactTargetWorldOccurrenceId\": \"" << CDataJson::Escape(logic.strContactTargetWorldOccurrenceId) << "\"";
		}
		if (logic.strLogicType == "TRIGGER" && !logic.strTriggerKind.empty())
		{
			output << ",\n      \"triggerKind\": \"" << logic.strTriggerKind << "\"";
			if (logic.strTriggerKind == "ENTER_AREA" && logic.bRearmOnExit)
				output << ",\n      \"rearmOnExit\": true";
			if (logic.strTriggerKind == "ENTER_AREA" && logic.bRepeatAfterKnockback)
				output << ",\n      \"repeatAfterKnockback\": true";
            if (logic.strTriggerKind == "ENTER_AREA" && logic.fBossChargeDistanceM != 0.0)
                output << ",\n      \"bossChargeDistanceM\": " << logic.fBossChargeDistanceM;
            if (logic.strTriggerKind == "ENTER_AREA" && logic.fChargeYawOffsetDegrees != 0.0)
                output << ",\n      \"chargeYawOffsetDegrees\": " << logic.fChargeYawOffsetDegrees;
			if (logic.strTriggerKind == "ALBION_AIRBORNE")
			{
				output << ",\n      \"airbornePhase\": \"" << logic.strAirbornePhase << "\""
					<< ",\n      \"airborneHeightM\": " << logic.fAirborneHeightM
					<< ",\n      \"airborneDurationMs\": " << logic.iAirborneDurationMs
					<< ",\n      \"teleportPosition\": [" << logic.TeleportPosition[0] << ", " << logic.TeleportPosition[1] << ", " << logic.TeleportPosition[2] << "]";
			}
			else if (logic.strTriggerKind == "ALBION_BLUE_CIRCLE")
			{
				output << ",\n      \"countPerPlayer\": " << logic.iCountPerPlayer
					<< ",\n      \"radiusM\": " << logic.fPlayerEffectRadiusM
					<< ",\n      \"effectLifetimeMs\": " << logic.iEffectLifetimeMs;
				if (logic.iArenaRandomCount != 0u)
					output << ",\n      \"arenaRandomCount\": " << logic.iArenaRandomCount
						<< ",\n      \"arenaRandomRadiusM\": " << logic.fArenaRandomRadiusM
						<< ",\n      \"arenaHeightToleranceM\": " << logic.fArenaHeightToleranceM
						<< ",\n      \"arenaMinimumSpacingM\": " << logic.fArenaMinimumSpacingM;
				if (logic.bRandomPlayerOnly)
					output << ",\n      \"randomPlayerOnly\": true";
			}
			else if (logic.strTriggerKind == "OBJECT_CONTACT")
			{
				output << ",\n";
				textList("targetWorldOccurrenceIds", logic.TargetWorldOccurrenceIds, "      ", true);
				output << "      ,\"targetRadiusM\": " << logic.fTargetRadiusM
					<< ",\n      \"contactPriority\": " << logic.iContactPriority;
				if (!logic.strContactGroupId.empty())
					output << ",\n      \"contactGroupId\": \"" << CDataJson::Escape(logic.strContactGroupId) << "\"";
			}
			else if (logic.strTriggerKind == "HUD_ENTER")
				output << ",\n      \"hudMode\": \"" << logic.strHudMode << "\"";
			else if (logic.strTriggerKind == "CARD_MAZE_ENTER" || logic.strTriggerKind == "BOSS_TELEPORT_XZ")
				output << ",\n      \"teleportPosition\": [" << logic.TeleportPosition[0] << ", " << logic.TeleportPosition[1] << ", " << logic.TeleportPosition[2] << "]";
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
            << "      \"displayName\": \"" << CDataJson::Escape(summon.strDisplayName) << "\"";
        if (!summon.strSummonKind.empty())
        {
            output << ",\n      \"summonKind\": \"" << CDataJson::Escape(summon.strSummonKind)
                << "\",\n      \"cloneEndStageId\": \"" << CDataJson::Escape(summon.strCloneEndStageId) << "\",\n";
            textList("directionPatternIds", summon.DirectionPatternIds, "      ", true);
        }
        output << "\n    }" << (summonIndex + 1u < document.Summons.size() ? "," : "") << "\n";
	}
	output << "  ],\n  \"worlds\": [\n";
	for (std::size_t worldIndex = 0u; worldIndex < document.Worlds.size(); ++worldIndex)
	{
		const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world = document.Worlds[worldIndex];
		output << "    {\n"
			<< "      \"worldId\": \"" << CDataJson::Escape(world.strWorldId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(world.strDisplayName) << "\",\n"
			<< "      \"sequenceInstanceId\": \"" << CDataJson::Escape(world.strSequenceInstanceId) << "\",\n"
			<< (world.strObjectResourceId.empty() ? std::string{} :
				"      \"objectResourceId\": \"" + CDataJson::Escape(world.strObjectResourceId) + "\",\n")
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
			<< "\", \"elementId\": \"" << CDataJson::Escape(row.strElementId)
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
            << "      \"gateId\": \"" << CDataJson::Escape(pattern.strGateId) << "\",\n"
            << "      \"targetBossPlacementId\": \"" << CDataJson::Escape(pattern.strTargetBossPlacementId) << "\",\n";
        if (!pattern.strFolderId.empty())
            output << "      \"folderId\": \"" << CDataJson::Escape(pattern.strFolderId) << "\",\n";
        if (pattern.iDurationMs) output << "      \"durationMs\": " << pattern.iDurationMs << ",\n";
        if (!pattern.PatternOccurrences.empty() || pattern.iNextPatternOccurrenceOrdinal != 1u)
        {
            output << "      \"nextPatternOccurrenceOrdinal\": " << pattern.iNextPatternOccurrenceOrdinal
                << ",\n      \"patternOccurrences\": [";
            for (size_t i = 0; i < pattern.PatternOccurrences.size(); ++i)
            {
                const auto& row = pattern.PatternOccurrences[i];
                output << "{\"occurrenceId\": \"" << CDataJson::Escape(row.strOccurrenceId)
                    << "\", \"patternId\": \"" << CDataJson::Escape(row.strPatternId)
                    << "\", \"startMs\": " << row.iStartMs << ", \"durationMs\": " << row.iDurationMs
                    << ", \"repeat\": " << (row.bRepeat ? "true" : "false") << "}"
                    << (i + 1 < pattern.PatternOccurrences.size() ? ", " : "");
            }
            output << "],\n";
        }
        output
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
				<< "          \"durationMs\": " << stage.iDurationMs << ",\n";
			if (stage.bRetargetOnEnter) output << "          \"retargetOnEnter\": true,\n";
			output << "          \"animationOccurrences\": [\n";
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
					<< (occurrence.iSourceEndMs ? "              \"sourceEndMs\": " + std::to_string(occurrence.iSourceEndMs) + ",\n" : "")
					<< "              \"playMs\": " << occurrence.iPlayMs << ",\n"
					<< (occurrence.iBlendInMs ? "              \"blendInMs\": " + std::to_string(occurrence.iBlendInMs) + ",\n" : "")
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
			if (box.RoomPlayerArrival)
			{
				output << "          \"roomPlayerArrival\": { \"playerSlot\": " << box.RoomPlayerArrival->iPlayerSlot << ", \"position\": ";
				Write_PresentationVector(output, box.RoomPlayerArrival->Position);
				output << " },\n";
			}
			if (!box.strHoldLogicOccurrenceId.empty())
				output << "          \"holdLogicOccurrenceId\": \"" << CDataJson::Escape(box.strHoldLogicOccurrenceId) << "\",\n";
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
                << "          \"durationMs\": " << box.iDurationMs;
            if (!box.PatternSpawns.empty())
            {
                output << ",\n          \"patternSpawns\": [";
                for (std::size_t spawnIndex = 0u; spawnIndex < box.PatternSpawns.size(); ++spawnIndex)
                {
                    const auto& spawn = box.PatternSpawns[spawnIndex];
                    if (spawnIndex) output << ",";
                    output << "\n            { \"spawnId\": \"" << CDataJson::Escape(spawn.strSpawnId)
                        << "\", \"patternId\": \"" << CDataJson::Escape(spawn.strPatternId) << "\", \"positionOffset\": ";
                    Write_PresentationVector(output, spawn.PositionOffset);
                    output << ", \"yawOffsetDegrees\": " << spawn.fYawOffsetDegrees << " }";
                }
                output << "\n          ]";
            }
            output << "\n        }" << (boxIndex + 1u < pattern.SummonOccurrences.size() ? "," : "") << "\n";
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
				<< "          \"playbackSpeed\": " << static_cast<double>(box.fPlaybackSpeed);
			if (box.Placement)
			{
				output << ",\n          \"placement\": { \"position\": "; Write_PresentationVector(output, box.Placement->Position);
				output << ", \"rotationDegrees\": "; Write_PresentationVector(output, box.Placement->RotationDegrees);
				output << ", \"scale\": "; Write_PresentationVector(output, box.Placement->Scale);
				output << " }";
			}
			output << "\n        }" << (boxIndex + 1u < pattern.WorldOccurrences.size() ? "," : "") << "\n";
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
		output << "      ],\n      \"resetBossToSpawn\": " << (pattern.bResetBossToSpawn ? "true" : "false");
		if (pattern.bEnterCombatOnFinish)
			output << ",\n      \"enterCombatOnFinish\": true";
		if (pattern.fAnimationRootVerticalScale != 1.0)
			output << ",\n      \"animationRootVerticalScale\": " << pattern.fAnimationRootVerticalScale;
		if (pattern.BossMotion)
		{
			const auto& motion = *pattern.BossMotion;
			output << ",\n      \"bossMotion\": {\"startMs\": " << motion.iStartMs
				<< ", \"endMs\": " << motion.iEndMs << ", \"startPosition\": ";
			Write_PresentationVector(output, motion.StartPosition);
			output << ", \"endPosition\": ";
			Write_PresentationVector(output, motion.EndPosition);
			output << ", \"yawDegrees\": " << motion.fYawDegrees << "}";
		}
		if (pattern.ResetBossYawDegrees)
			output << ",\n      \"resetBossYawDegrees\": " << *pattern.ResetBossYawDegrees;
		output << ",\n      \"nextPresentationOccurrenceOrdinal\": " << pattern.iNextPresentationOccurrenceOrdinal
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
			if (row.bFitEffectToDuration) output << ", \"fitEffectToDuration\": true";
			output << ", \"fadeInMs\": " << row.iFadeInMs << ", \"fadeOutMs\": " << row.iFadeOutMs
				<< ", \"dissolveStart\": " << row.fDissolveStart << ", \"dissolveEnd\": " << row.fDissolveEnd
				<< ", \"brightnessMultiplier\": " << row.fBrightnessMultiplier
				<< ", \"volume\": " << row.fVolume << ", \"followBoss\": " << (row.bFollowBoss ? "true" : "false")
				<< ", \"debugRender\": " << (row.bDebugRender ? "true" : "false")
				<< ", \"bone\": \"" << CDataJson::Escape(row.strBone) << "\", \"boneTarget\": \"" << row.strBoneTarget << "\", \"regionId\": \"" << CDataJson::Escape(row.strRegionId)
				<< "\", \"cardSymbol\": \"" << CDataJson::Escape(row.strCardSymbol)
				<< "\", \"cardColor\": \"" << CDataJson::Escape(row.strCardColor)
				<< "\", \"anchorKind\": \"" << CDataJson::Escape(row.strAnchorKind)
				<< "\", \"worldId\": \"" << CDataJson::Escape(row.strWorldId) << "\", \"logicOccurrenceId\": \"" << CDataJson::Escape(row.strLogicOccurrenceId)
				<< "\", \"worldOccurrenceId\": \"" << CDataJson::Escape(row.strWorldOccurrenceId) << "\""
				<< (0u != row.iWorldEmissionIndex ? ", \"worldEmissionIndex\": " + std::to_string(row.iWorldEmissionIndex) : std::string{})
				<< (!row.strSelectionGroupId.empty() ? ", \"selectionGroupId\": \"" + CDataJson::Escape(row.strSelectionGroupId) + "\"" : std::string{}) << "}"
				<< (i + 1u < pattern.PresentationOccurrences.size() ? "," : "") << '\n';
		}
		output << "      ]\n    }"
			<< (patternIndex + 1u < document.Patterns.size() ? "," : "") << "\n";
	}

    output << "  ],\n  \"folders\": [\n";
    for (size_t i = 0; i < document.Folders.size(); ++i)
    {
        const auto& folder = document.Folders[i];
        if (!folder.strLoadError.empty()) output << folder.strPreservedJson;
        else
        {
            output << "    {\"folderId\": \"" << CDataJson::Escape(folder.strFolderId)
                << "\", \"gateId\": \"" << CDataJson::Escape(folder.strGateId)
                << "\", \"displayName\": \"" << CDataJson::Escape(folder.strDisplayName) << "\"";
            if (!folder.strTimelinePatternId.empty())
                output << ", \"timelinePatternId\": \"" << CDataJson::Escape(folder.strTimelinePatternId) << "\"";
            output << "}";
        }
        output << (i + 1 < document.Folders.size() ? "," : "") << '\n';
    }
    output << "  ],\n  \"bundles\": [\n";
    for (size_t i = 0; i < document.Bundles.size(); ++i)
    {
        const auto& bundle = document.Bundles[i];
        if (!bundle.strLoadError.empty()) output << bundle.strPreservedJson;
        else
        {
            output << "    {\"bundleId\": \"" << CDataJson::Escape(bundle.strBundleId)
                << "\", \"gateId\": \"" << CDataJson::Escape(bundle.strGateId)
                << "\", \"folderId\": \"" << CDataJson::Escape(bundle.strFolderId)
                << "\", \"displayName\": \"" << CDataJson::Escape(bundle.strDisplayName)
                << "\", \"authoringStatus\": \"" << CDataJson::Escape(bundle.strAuthoringStatus)
                << "\", \"nextMemberOrdinal\": " << bundle.iNextMemberOrdinal
                << ", \"nextSceneProfileOccurrenceOrdinal\": " << bundle.iNextSceneProfileOccurrenceOrdinal
                << ", \"nextPresentationOccurrenceOrdinal\": " << bundle.iNextPresentationOccurrenceOrdinal << ", \"members\": [";
            for (size_t j = 0; j < bundle.Members.size(); ++j)
            {
                const auto& member = bundle.Members[j];
                output << (j ? "," : "") << "{\"memberId\": \"" << CDataJson::Escape(member.strMemberId)
                    << "\", \"patternId\": \"" << CDataJson::Escape(member.strPatternId) << "\", \"startOffsetMs\": " << member.iStartOffsetMs << '}';
            }
            output << "], \"sceneProfileOccurrences\": [";
            for (size_t j = 0; j < bundle.SceneProfileOccurrences.size(); ++j)
            {
                const auto& row = bundle.SceneProfileOccurrences[j];
                output << (j ? "," : "") << "{\"occurrenceId\": \"" << CDataJson::Escape(row.strOccurrenceId)
                    << "\", \"sceneProfileId\": \"" << CDataJson::Escape(row.strSceneProfileId) << "\", \"startMs\": " << row.iStartMs
                    << ", \"durationMs\": " << row.iDurationMs << ", \"blendMs\": " << row.iBlendMs << '}';
            }
            output << "], \"presentationOccurrences\": [\n";
            for (size_t j = 0; j < bundle.PresentationOccurrences.size(); ++j)
            {
                const auto& row = bundle.PresentationOccurrences[j];
                output << "{\"occurrenceId\": \"" << CDataJson::Escape(row.strOccurrenceId)
                    << "\", \"resourceId\": \"" << CDataJson::Escape(row.strResourceId) << "\", \"startMs\": " << row.iStartMs
                    << ", \"durationMs\": " << row.iDurationMs << ", \"positionOffset\": "; Write_PresentationVector(output, row.PositionOffset);
                output << ", \"rotationDegrees\": "; Write_PresentationVector(output, row.RotationDegrees);
                output << ", \"scale\": "; Write_PresentationVector(output, row.Scale);
                if (row.bFitEffectToDuration) output << ", \"fitEffectToDuration\": true";
                output << ", \"fadeInMs\": " << row.iFadeInMs << ", \"fadeOutMs\": " << row.iFadeOutMs
                    << ", \"dissolveStart\": " << row.fDissolveStart << ", \"dissolveEnd\": " << row.fDissolveEnd
                    << ", \"brightnessMultiplier\": " << row.fBrightnessMultiplier << ", \"volume\": " << row.fVolume
                    << ", \"followBoss\": " << (row.bFollowBoss ? "true" : "false") << ", \"debugRender\": " << (row.bDebugRender ? "true" : "false")
                    << ", \"bone\": \"" << CDataJson::Escape(row.strBone) << "\", \"boneTarget\": \"" << row.strBoneTarget << "\", \"regionId\": \"" << CDataJson::Escape(row.strRegionId)
                    << "\", \"cardSymbol\": \"" << CDataJson::Escape(row.strCardSymbol) << "\", \"cardColor\": \"" << CDataJson::Escape(row.strCardColor)
                    << "\", \"anchorKind\": \"" << CDataJson::Escape(row.strAnchorKind) << "\", \"worldId\": \"" << CDataJson::Escape(row.strWorldId)
                    << "\", \"logicOccurrenceId\": \"" << CDataJson::Escape(row.strLogicOccurrenceId)
                    << "\", \"worldOccurrenceId\": \"" << CDataJson::Escape(row.strWorldOccurrenceId) << "\""
                    << (0u != row.iWorldEmissionIndex ? ", \"worldEmissionIndex\": " + std::to_string(row.iWorldEmissionIndex) : std::string{}) << "}"
                    << (j + 1 < bundle.PresentationOccurrences.size() ? "," : "") << '\n';
            }
            output << "]}";
        }
        output << (i + 1 < document.Bundles.size() ? "," : "") << '\n';
    }
    output << "  ],\n  \"patternFlows\": [\n";
    for (size_t i = 0; i < document.PatternFlows.size(); ++i)
    {
        const auto& flow = document.PatternFlows[i];
        output << "    {\"flowId\": \"" << CDataJson::Escape(flow.strFlowId)
            << "\", \"gateId\": \"" << CDataJson::Escape(flow.strGateId)
            << "\", \"displayName\": \"" << CDataJson::Escape(flow.strDisplayName) << "\", \"entries\": [";
        for (size_t j = 0; j < flow.Entries.size(); ++j)
        {
            const auto& entry = flow.Entries[j];
            output << (j ? "," : "") << "{\"entryId\": \"" << CDataJson::Escape(entry.strEntryId)
                << "\", \"kind\": \"" << CDataJson::Escape(entry.strKind)
                << "\", \"targetId\": \"" << CDataJson::Escape(entry.strTargetId)
                << "\", \"waitAfterMs\": " << entry.iWaitAfterMs << '}';
        }
        output << "]}" << (i + 1 < document.PatternFlows.size() ? "," : "") << '\n';
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
		!Matches_CompositionPath(path, stagedDocument, status) ||
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
	if (!m_bHasLastGood || m_Path.empty())
	{
		outStatus = "KoukuSaydon composition Save requires a last-good baseline.";
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
	if (!Matches_CompositionPath(m_Path, candidate, status) ||
		!Validate(candidate, m_References, status))
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

	KOUKU_SAYDON_COMPOSITION_DOCUMENT staged = candidate;
	std::string currentBytes;
	if (!Read_Text(m_Path, MAX_COMPOSITION_BYTES, currentBytes, status,
			"Current KoukuSaydon composition"))
	{
		outStatus = m_strStatus = "Could not read the current Composition before Save. Draft preserved. " + status;
		m_bFresh = false;
		return false;
	}
	if (currentBytes != m_strBaselineSourceBytes)
	{
		// Library installation may append independent resources while the user
		// authors a Pattern. Rebase only that proven change; never merge edited
		// Patterns, reordered resources, or two different values for one ID.
		KOUKU_SAYDON_COMPOSITION_DOCUMENT current;
		const auto reject = [&](const std::string& reason) {
			outStatus = m_strStatus = "Composition changed before Save: " + reason +
				" Current file and unsaved draft are preserved.";
			m_bFresh = false;
			return false;
		};
		if (!Parse_Text(currentBytes, current, status) || !Validate(current, m_References, status))
			return reject(status);
		if (current.iRevision <= m_LastGood.iRevision || current.iRevision >= MAX_REVISION ||
			current.PresentationResources.size() <= m_LastGood.PresentationResources.size() ||
			!std::equal(m_LastGood.PresentationResources.begin(), m_LastGood.PresentationResources.end(),
				current.PresentationResources.begin()))
			return reject("external changes are not compatible resource additions");
		auto unchanged = current;
		unchanged.iRevision = m_LastGood.iRevision;
		unchanged.PresentationResources = m_LastGood.PresentationResources;
		if (unchanged != m_LastGood)
			return reject("external Pattern or other authored values also changed");
		for (std::size_t index = m_LastGood.PresentationResources.size();
			index < current.PresentationResources.size(); ++index)
		{
			const auto& added = current.PresentationResources[index];
			const auto existing = std::find_if(staged.PresentationResources.begin(), staged.PresentationResources.end(),
				[&](const auto& row) { return row.strResourceId == added.strResourceId; });
			if (existing == staged.PresentationResources.end()) staged.PresentationResources.push_back(added);
			else if (*existing != added) return reject("resource ID conflicts: " + added.strResourceId);
		}
		staged.iRevision = current.iRevision;
		if (!Validate(staged, m_References, status)) return reject(status);
	}
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
		!Matches_CompositionPath(m_Path, reparsed, status) ||
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
		!Matches_CompositionPath(m_Path, reopened, status) ||
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


bool Client::Sample_KoukuSaydonBossMotion(
	const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const double patternTimeMs,
	std::array<double, 3u>& outPosition, double& outYawDegrees) noexcept
{
	if (!pattern.BossMotion || !std::isfinite(patternTimeMs)) return false;
	const auto& motion = *pattern.BossMotion;
	if (motion.iEndMs <= motion.iStartMs) return false;
	const double alpha = std::clamp((patternTimeMs - motion.iStartMs) /
		static_cast<double>(motion.iEndMs - motion.iStartMs), 0.0, 1.0);
	for (std::size_t axis = 0u; axis < outPosition.size(); ++axis)
		outPosition[axis] = motion.StartPosition[axis] +
			(motion.EndPosition[axis] - motion.StartPosition[axis]) * alpha;
	outYawDegrees = motion.fYawDegrees;
	return true;
}


bool_t Client::CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(
    const std::uint32_t sourceStartMs, const std::uint32_t sourceEndMs,
    const double elapsedMs, const double playRate, const double nativeDurationMs,
    const bool_t loop, double& outSourceMs)
{
    if (!std::isfinite(elapsedMs) || !std::isfinite(playRate) || playRate <= 0.0 ||
        !std::isfinite(nativeDurationMs) || nativeDurationMs <= 0.0 ||
        (sourceEndMs && (sourceEndMs <= sourceStartMs || sourceEndMs > nativeDurationMs + 1.0)))
        return false;
    const double start = (std::min)(double(sourceStartMs), nativeDurationMs);
    const double end = sourceEndMs ? (std::min)(double(sourceEndMs), nativeDurationMs) : nativeDurationMs;
    const double age = (std::max)(0.0, elapsedMs) * playRate;
    if (!std::isfinite(age) || end < start || (loop && end <= start)) return false;
    outSourceMs = loop ? start + std::fmod(age, end - start) : (std::min)(end, start + age);
    return true;
}

bool_t Client::CKoukuSaydonCompositionDocument::Trim_AnimationWindow(
    KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence,
    const std::int64_t deltaMs, const bool_t front, const std::uint32_t stageDurationMs,
    const std::uint32_t nativeDurationMs)
{
    if (!occurrence.iPlayMs || !std::isfinite(occurrence.fPlayRate) || occurrence.fPlayRate <= 0.f ||
        std::uint64_t(occurrence.iStartOffsetMs) + occurrence.iPlayMs > stageDurationMs) return false;
    const double rate = occurrence.fPlayRate;
    auto staged = occurrence;
    const bool loop = occurrence.strEndPolicy == "LOOP_TO_WINDOW";
    const double previousEnd = occurrence.iSourceEndMs ? occurrence.iSourceEndMs :
        (loop ? double(nativeDurationMs) : occurrence.iSourceStartMs + occurrence.iPlayMs * rate);
    if (front && loop)
    {
        const auto trim = std::clamp(deltaMs, -std::int64_t(occurrence.iStartOffsetMs), std::int64_t(occurrence.iPlayMs) - 1);
        staged.iStartOffsetMs = std::uint32_t(std::int64_t(occurrence.iStartOffsetMs) + trim);
        staged.iPlayMs = std::uint32_t(std::int64_t(occurrence.iPlayMs) - trim);
    }
    else if (front)
    {
        const auto minDelta = (std::max)(-std::int64_t(occurrence.iStartOffsetMs),
            -std::int64_t(std::floor(occurrence.iSourceStartMs / rate)));
        const double sourceLimit = previousEnd > 0.0 ? previousEnd - 1.0 : 600000.0;
        const auto maxDelta = (std::min)(std::int64_t(occurrence.iPlayMs) - 1,
            std::int64_t(std::floor((sourceLimit - occurrence.iSourceStartMs) / rate)));
        if (maxDelta < minDelta) return false;
        const auto trim = std::clamp(deltaMs, minDelta, maxDelta);
        staged.iStartOffsetMs = std::uint32_t(std::int64_t(occurrence.iStartOffsetMs) + trim);
        staged.iSourceStartMs = std::uint32_t(std::int64_t(occurrence.iSourceStartMs) + std::llround(trim * rate));
        staged.iPlayMs = std::uint32_t(std::int64_t(occurrence.iPlayMs) - trim);
        if (!loop && previousEnd <= 600000.0)
            staged.iSourceEndMs = std::uint32_t(std::llround(nativeDurationMs ?
                (std::min)(previousEnd, double(nativeDurationMs)) : previousEnd));
    }
    else
    {
        staged.iPlayMs = std::uint32_t(std::clamp(std::int64_t(occurrence.iPlayMs) + deltaMs,
            std::int64_t(1), std::int64_t(stageDurationMs) - occurrence.iStartOffsetMs));
        if (!loop)
        {
            double end = occurrence.iSourceStartMs + staged.iPlayMs * rate;
            if (nativeDurationMs) end = (std::min)(end, double(nativeDurationMs));
            if (end > 600000.0) return false;
            staged.iSourceEndMs = std::uint32_t(std::llround(end));
        }
    }
    staged.iBlendInMs = (std::min)(staged.iBlendInMs, staged.iPlayMs);
    if (staged.iSourceEndMs && staged.iSourceEndMs <= staged.iSourceStartMs)
    {
        if (staged.strEndPolicy == "HOLD_LAST_POSE") staged.iSourceEndMs = 0u;
        else return false;
    }
    occurrence = std::move(staged);
    return true;
}

bool_t Client::CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& source, const std::string_view patternId,
    KOUKU_SAYDON_COMPOSITION_DOCUMENT& outDocument, std::string& outStatus)
{
    const auto selected = std::find_if(source.Patterns.begin(), source.Patterns.end(),
        [&](const auto& row) { return row.strPatternId == patternId; });
    const auto fail = [&](const std::string& reason) {
        outStatus = "Parent expansion " + std::string(patternId) + ": " + reason;
        return false;
    };
    if (selected == source.Patterns.end()) return fail("Pattern is missing.");
    if (!selected->strLoadError.empty()) return fail(selected->strLoadError);
    if (!Validate_Shape(source, outStatus)) return false;
    if (!Validate_PatternChildren(source, *selected, outStatus)) return false;
    const auto lifetime = Pattern_Lifetime(*selected);
    if (lifetime > MAX_TIME_MS) return fail("Timeline exceeds 600 seconds.");
    if (selected->PatternOccurrences.empty())
    {
        auto staged = source;
        auto& pattern = staged.Patterns[static_cast<size_t>(selected - source.Patterns.begin())];
        std::uint64_t stageDurationMs = 0u;
        for (const auto& stage : pattern.Stages) stageDurationMs += stage.iDurationMs;
        // A leaf's late authored lanes keep the final pose through the existing
        // Stage playback clamp. Only this preview copy receives the held tail.
        if (lifetime > stageDurationMs && !pattern.Stages.empty())
            pattern.Stages.back().iDurationMs += static_cast<std::uint32_t>(lifetime - stageDurationMs);
        if (!Try_ResolveAnimationBlendWindows(staged, pattern, pattern.AnimationBlendWindows, outStatus)) return false;
        outDocument = std::move(staged); outStatus.clear(); return true;
    }
    if (!lifetime) return fail("Timeline needs a positive duration.");

    KOUKU_SAYDON_COMPOSITION_DOCUMENT staged = source;
    auto& result = staged.Patterns[static_cast<size_t>(selected - source.Patterns.begin())];
    const auto showtimeGroups = [&](const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern) {
        std::unordered_set<std::string> groups;
        for (const auto& box : pattern.LogicOccurrences)
            for (const auto& definition : source.Logics)
                if (definition.strLogicId == box.strLogicId && definition.strJudgementKind == "SHOWTIME_PLAYER_TARGETS" &&
                    !definition.strFixedSelectionGroupId.empty()) groups.insert(definition.strFixedSelectionGroupId);
        return groups;
    };
    const auto parentShowtimeGroups = showtimeGroups(result);
    for (auto& row : result.PresentationOccurrences)
        if (!parentShowtimeGroups.contains(row.strSelectionGroupId)) row.strSelectionGroupId.clear();
    result.PatternOccurrences.clear();
    result.iNextPatternOccurrenceOrdinal = 1u;
    result.iDurationMs = static_cast<std::uint32_t>(lifetime);
    result.Stages.clear();

    struct TimedStage {
        std::uint32_t start = 0u;
        KOUKU_SAYDON_COMPOSITION_STAGE stage;
    };
    std::vector<TimedStage> stages;
    std::vector<KOUKU_SAYDON_COMPOSITION_PATTERN_OCCURRENCE> slots = selected->PatternOccurrences;
    std::sort(slots.begin(), slots.end(), [](const auto& a, const auto& b) {
        return a.iStartMs < b.iStartMs;
    });
    const auto addStage = [&](const KOUKU_SAYDON_COMPOSITION_STAGE& original,
        const std::uint32_t origin, const std::uint32_t begin, const std::uint32_t end,
        const std::uint32_t destination, const std::string& animationScope, std::size_t& animationIndex) -> bool {
        if (begin >= end) return true;
        if (stages.size() >= MAX_PRODUCT_STAGES_PER_PATTERN)
            return fail("Expanded timeline exceeds 64 Stages.");
        TimedStage item{destination, original};
        item.stage.iDurationMs = end - begin;
        item.stage.bRetargetOnEnter = original.bRetargetOnEnter && begin == origin;
        item.stage.AnimationOccurrences.clear();
        for (const auto& originalAnimation : original.AnimationOccurrences)
        {
            const auto index = ++animationIndex;
            const auto start = std::uint64_t(origin) + originalAnimation.iStartOffsetMs;
            const auto finish = start + originalAnimation.iPlayMs;
            if (start >= end || finish <= begin) continue;
            if (start < begin) return fail("Animation starts before its retained Stage segment.");
            auto animation = originalAnimation;
            animation.iStartOffsetMs = static_cast<std::uint32_t>(start - begin);
            animation.iPlayMs = static_cast<std::uint32_t>((std::min)(finish, std::uint64_t(end)) - start);
            animation.iBlendInMs = (std::min)(animation.iBlendInMs, animation.iPlayMs);
            animation.iPoseStartMs = UINT32_MAX;
            if (!animationScope.empty())
                animation.strOccurrenceId = animationScope + ".animation." + std::to_string(index);
            item.stage.AnimationOccurrences.push_back(std::move(animation));
        }
        stages.push_back(std::move(item));
        return true;
    };

    // A child owns the boss for its entire placement window. Parent lanes remain
    // untouched; only animation-free portions of its Stage clock are replaced.
    std::uint32_t origin = 0u;
    for (const auto& stage : selected->Stages)
    {
        if (!stage.iDurationMs || std::uint64_t(origin) + stage.iDurationMs > lifetime)
            return fail("Own Stage duration is outside the timeline.");
        const auto end = origin + stage.iDurationMs;
        auto cursor = origin;
        for (const auto& slot : slots)
        {
            const auto slotEnd = slot.iStartMs + slot.iDurationMs;
            if (slotEnd <= cursor || slot.iStartMs >= end) continue;
            std::size_t index = 0u;
            if (!addStage(stage, origin, cursor, (std::min)(end, slot.iStartMs), cursor, {}, index)) return false;
            cursor = (std::min)(end, slotEnd);
        }
        std::size_t index = 0u;
        if (!addStage(stage, origin, cursor, end, cursor, {}, index)) return false;
        origin = end;
    }

    for (const auto& slot : slots)
    {
        const auto childIt = std::find_if(source.Patterns.begin(), source.Patterns.end(),
            [&](const auto& row) { return row.strPatternId == slot.strPatternId; });
        const auto& child = *childIt;
        if (child.bResetBossToSpawn || child.ResetBossYawDegrees || child.bEnterCombatOnFinish)
            return fail("Child spawn reset or combat-entry completion is unsupported: " + child.strPatternId);
        if (child.BossMotion)
        {
            // Validation admits a single complete non-repeated actor owner. Its
            // absolute path keeps source positions and uses the Parent clock.
            auto motion = *child.BossMotion;
            motion.iStartMs += slot.iStartMs;
            motion.iEndMs += slot.iStartMs;
            result.BossMotion = motion;
        }
        if (child.fAnimationRootVerticalScale != selected->fAnimationRootVerticalScale)
            return fail("Child animation root vertical scale differs from Parent: " + child.strPatternId);
        const auto childDuration = static_cast<std::uint32_t>(Pattern_Lifetime(child));
        const auto slotEnd = slot.iStartMs + slot.iDurationMs;
        std::uint32_t cycleStart = slot.iStartMs;
        std::uint32_t repeatIndex = 0u;
        while (cycleStart < slotEnd)
        {
            if (repeatIndex >= MAX_PRODUCT_STAGES_PER_PATTERN)
                return fail("Expanded timeline exceeds 64 child repetitions.");
            const auto available = (std::min)(childDuration, slotEnd - cycleStart);
            const std::string scope = slot.strOccurrenceId + ".r" + std::to_string(repeatIndex);
            if (scope.size() > 96u) return fail("Child occurrence namespace exceeds the stable-ID limit.");
            std::unordered_map<std::string, std::string> ids;
            const auto mapRows = [&](const auto& rows, const std::string& kind) {
                for (size_t i = 0u; i < rows.size(); ++i)
                    ids.emplace(rows[i].strOccurrenceId, scope + "." + kind + "." + std::to_string(i + 1u));
            };
            mapRows(child.LogicOccurrences, "logic");
            mapRows(child.SummonOccurrences, "summon");
            mapRows(child.WorldOccurrences, "world");
            mapRows(child.SceneProfileOccurrences, "sceneprofile");
            mapRows(child.PresentationOccurrences, "presentation");
            const auto childShowtimeGroups = showtimeGroups(child);
            std::unordered_set<std::string> childShowtimeTemplateIds;
            for (const auto& box : child.LogicOccurrences)
            {
                if (!box.bEnabled) continue;
                const auto definition = std::find_if(source.Logics.begin(), source.Logics.end(),
                    [&](const auto& row) { return row.strLogicId == box.strLogicId; });
                if (definition == source.Logics.end() || definition->strJudgementKind != "SHOWTIME_PLAYER_TARGETS") continue;
                for (const auto& volley : definition->RandomVolleyOccurrenceSets)
                    childShowtimeTemplateIds.insert(volley.begin(), volley.end());
                if (!definition->strTrackingPresentationOccurrenceId.empty())
                    childShowtimeTemplateIds.insert(definition->strTrackingPresentationOccurrenceId);
                for (const auto& row : child.PresentationOccurrences)
                    if (!definition->strFixedSelectionGroupId.empty() && row.strSelectionGroupId == definition->strFixedSelectionGroupId)
                        childShowtimeTemplateIds.insert(row.strOccurrenceId);
            }
            for (const auto& row : child.PresentationOccurrences)
                if (childShowtimeTemplateIds.contains(row.strOccurrenceId) && std::uint64_t(row.iStartMs) + row.iDurationMs > available)
                    return fail("Parent slot would truncate a SHOWTIME Effect template; extend its child window: " + row.strOccurrenceId);
            for (size_t i = 0u; i < child.PresentationOccurrences.size(); ++i)
                if (const auto& group = child.PresentationOccurrences[i].strSelectionGroupId; childShowtimeGroups.contains(group))
                    ids.emplace(group, scope + ".effectgroup." + std::to_string(i + 1u));
            for (size_t i = 0u; i < child.PresentationOccurrences.size(); ++i)
                if (!child.PresentationOccurrences[i].strRegionId.empty())
                    ids.emplace(child.PresentationOccurrences[i].strRegionId, scope + ".region." + std::to_string(i + 1u));
            const auto remap = [&](std::string& id) {
                const auto found = ids.find(id);
                if (found != ids.end()) id = found->second;
            };
            const auto remapRequired = [&](std::string& id) -> bool {
                if (id.empty()) return true;
                const auto found = ids.find(id);
                if (found == ids.end()) return fail("Child refers outside its own occurrences: " + id);
                id = found->second; return true;
            };

            std::unordered_set<std::string> usedLogic;
            for (const auto& row : child.LogicOccurrences)
            {
                usedLogic.insert(row.strLogicId);
                for (const auto* list : {&row.OnSuccessLogicIds, &row.OnFailLogicIds, &row.OnTimeoutLogicIds})
                    usedLogic.insert(list->begin(), list->end());
            }
            for (size_t i = 0; i < source.Logics.size(); ++i)
                if (usedLogic.contains(source.Logics[i].strLogicId))
                    ids.emplace(source.Logics[i].strLogicId, scope + ".definition." + std::to_string(i + 1u));
            for (const auto& id : usedLogic)
                if (!ids.contains(id)) return fail("Child Logic definition is missing: " + id);
            for (const auto& sourceLogic : source.Logics)
            {
                if (!usedLogic.contains(sourceLogic.strLogicId)) continue;
                if (sourceLogic.bEndsPatternOnSuccess || sourceLogic.strOutcomeKind == "FOLLOWUP_PATTERN" ||
                    sourceLogic.strTriggerKind == "HUD_ENTER" || sourceLogic.strTriggerKind == "CARD_MAZE_HIDE_NEXT" ||
                    sourceLogic.strTriggerKind == "CARD_MAZE_ENTER")
                    return fail("Child Logic changes the whole Pattern or persistent HUD: " + sourceLogic.strLogicId);
                auto definition = sourceLogic;
                remap(definition.strLogicId);
                if (!remapRequired(definition.strFixedSelectionGroupId) ||
                    !remapRequired(definition.strTrackingPresentationOccurrenceId) ||
                    !remapRequired(definition.strSummonOccurrenceId)) return false;
                for (auto& volley : definition.RandomVolleyOccurrenceSets)
                    for (auto& id : volley) if (!remapRequired(id)) return false;
                for (auto& id : definition.RegionIds) if (!remapRequired(id)) return false;
                for (auto& id : definition.TargetWorldOccurrenceIds) if (!remapRequired(id)) return false;
                for (auto& motion : definition.ContactMotions)
                    if (!remapRequired(motion.strTargetWorldOccurrenceId)) return false;
                if (!remapRequired(definition.strTargetLogicOccurrenceId) ||
                    !remapRequired(definition.strContactTargetWorldOccurrenceId)) return false;
                remap(definition.strTargetWorldInstanceId);
                if (!definition.strContactGroupId.empty())
                {
                    const auto [contact, inserted] = ids.emplace(definition.strContactGroupId,
                        scope + ".contact." + std::to_string(static_cast<size_t>(&sourceLogic - source.Logics.data()) + 1u));
                    definition.strContactGroupId = contact->second;
                }
                staged.Logics.push_back(std::move(definition));
                if (staged.Logics.size() > MAX_LOGICS) return fail("Expanded Logic definitions exceed 4096.");
            }

            std::uint32_t childOrigin = 0u;
            std::size_t animationIndex = 0u;
            for (const auto& stage : child.Stages)
            {
                if (!stage.iDurationMs || std::uint64_t(childOrigin) + stage.iDurationMs > childDuration)
                    return fail("Invalid child Stage duration: " + child.strPatternId);
                if (childOrigin >= available) break;
                if (!addStage(stage, childOrigin, childOrigin, (std::min)(childOrigin + stage.iDurationMs, available),
                    cycleStart + childOrigin, scope, animationIndex)) return false;
                childOrigin += stage.iDurationMs;
            }
            const auto copyLane = [&](const auto& from, auto& destination, const auto& adjust) -> bool {
                for (const auto& sourceRow : from)
                {
                    if (sourceRow.iStartMs >= available) continue;
                    auto row = sourceRow;
                    const bool clipped = std::uint64_t(row.iStartMs) + row.iDurationMs > available;
                    row.iDurationMs = (std::min)(row.iDurationMs, available - row.iStartMs);
                    row.iStartMs += cycleStart;
                    remap(row.strOccurrenceId);
                    if (!adjust(row, clipped)) return false;
                    destination.push_back(std::move(row));
                }
                return true;
            };
            if (!copyLane(child.LogicOccurrences, result.LogicOccurrences, [&](auto& row, const bool clipped) {
                const auto definition = std::find_if(source.Logics.begin(), source.Logics.end(),
                    [&](const auto& item) { return item.strLogicId == row.strLogicId; });
                if (clipped && definition != source.Logics.end() && definition->fBossChargeDistanceM > 0.0)
                    return fail("Child charge cannot be truncated; extend its Pattern window: " + row.strOccurrenceId);
                remap(row.strLogicId);
                for (auto* list : {&row.OnSuccessLogicIds, &row.OnFailLogicIds, &row.OnTimeoutLogicIds})
                    for (auto& id : *list) remap(id);
                row.bCancelAtEnd = clipped;
                return remapRequired(row.strHoldLogicOccurrenceId);
            })) return false;
            if (!copyLane(child.SummonOccurrences, result.SummonOccurrences, [](auto&, bool) { return true; })) return false;
            if (!copyLane(child.WorldOccurrences, result.WorldOccurrences, [](auto&, bool) { return true; })) return false;
            if (!copyLane(child.SceneProfileOccurrences, result.SceneProfileOccurrences, [](auto& row, bool) {
                row.iBlendMs = (std::min)(row.iBlendMs, row.iDurationMs); return true;
            })) return false;
            if (!copyLane(child.PresentationOccurrences, result.PresentationOccurrences, [&](auto& row, bool) {
                // Server visual templates retain their scoped group references.
                if (childShowtimeGroups.contains(row.strSelectionGroupId)) remap(row.strSelectionGroupId);
                else row.strSelectionGroupId.clear();
                remap(row.strRegionId);
                row.iFadeInMs = (std::min)(row.iFadeInMs, row.iDurationMs);
                row.iFadeOutMs = (std::min)(row.iFadeOutMs, row.iDurationMs - row.iFadeInMs);
                return remapRequired(row.strLogicOccurrenceId) && remapRequired(row.strWorldOccurrenceId);
            })) return false;
            if (result.LogicOccurrences.size() > MAX_LOGIC_OCCURRENCES_PER_PATTERN ||
                result.SummonOccurrences.size() > MAX_SUMMON_OCCURRENCES_PER_PATTERN ||
                result.WorldOccurrences.size() > MAX_WORLD_OCCURRENCES_PER_PATTERN ||
                result.SceneProfileOccurrences.size() > MAX_SCENE_PROFILE_OCCURRENCES_PER_PATTERN ||
                result.PresentationOccurrences.size() > 1024u)
                return fail("Expanded child lanes exceed the existing Pattern capacity.");
            if (!slot.bRepeat) break;
            cycleStart += available;
            ++repeatIndex;
        }
    }

    std::sort(stages.begin(), stages.end(), [](const auto& a, const auto& b) { return a.start < b.start; });
    const std::string idleClip = selected->strActorProfileId == "MN_RPCT_05" ? "rpct00_idle_battle_1" :
        selected->strActorProfileId == "MN_RPCT_06" ? "mn_rpct_06_sk.ao_idle_battle_1" :
        selected->strActorProfileId == "MN_RPCZ_00" ? "rpcz00_idle_battle_1" : "";
    if (idleClip.empty()) return fail("Actor has no supported idle clip.");
    std::uint32_t stageOrdinal = 0u;
    std::uint64_t emittedDuration = 0u;
    std::uint64_t emittedEndTick = 0u;
    const auto emitStage = [&](KOUKU_SAYDON_COMPOSITION_STAGE stage) -> bool {
        if (stageOrdinal >= MAX_PRODUCT_STAGES_PER_PATTERN) return fail("Expanded timeline exceeds 64 Stages.");
        const auto endTick = ((emittedDuration + stage.iDurationMs) * FIXED_TICK_HZ + 999u) / 1000u;
        if (endTick <= emittedEndTick)
            return fail("Parent Stage boundaries must occupy distinct 30 Hz Server ticks; extend the short gap.");
        emittedDuration += stage.iDurationMs;
        emittedEndTick = endTick;
        ++stageOrdinal;
        stage.strStageId = "STAGE_" + std::to_string(stageOrdinal);
        stage.strActionId = selected->strPatternId + "_ACTION_" + std::to_string(stageOrdinal);
        if (stage.AnimationOccurrences.empty())
        {
            KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE idle;
            idle.strOccurrenceId = selected->strPatternId + ".idle." + std::to_string(stageOrdinal);
            idle.strProfileId = selected->strActorProfileId;
            idle.strSourceStageId = "RAW";
            idle.strSourceSlotId = idleClip;
            idle.strRuntimeClip = idleClip;
            idle.iPlayMs = stage.iDurationMs;
            idle.strEndPolicy = "LOOP_TO_WINDOW";
            stage.AnimationOccurrences.push_back(std::move(idle));
        }
        // A blend is only meaningful when its previous clip survives the
        // placement boundary. No implicit blend reaches across an idle gap.
        for (auto& animation : stage.AnimationOccurrences)
        {
            if (!animation.iBlendInMs) continue;
            if (animation.iStartOffsetMs || result.Stages.empty()) { animation.iBlendInMs = 0u; continue; }
            const auto& previous = result.Stages.back();
            const bool adjacent = std::any_of(previous.AnimationOccurrences.begin(), previous.AnimationOccurrences.end(),
                [&](const auto& row) { return std::uint64_t(row.iStartOffsetMs) + row.iPlayMs == previous.iDurationMs; });
            if (!adjacent) animation.iBlendInMs = 0u;
        }
        result.Stages.push_back(std::move(stage));
        return true;
    };
    const auto emitGap = [&](const std::uint32_t duration) -> bool {
        if (!duration) return true;
        KOUKU_SAYDON_COMPOSITION_STAGE idle;
        idle.strStageKind = "ACTIVE"; idle.iDurationMs = duration;
        return emitStage(std::move(idle));
    };
    std::uint32_t cursor = 0u;
    for (auto& item : stages)
    {
        if (item.start < cursor) return fail("Expanded Stage intervals overlap.");
        if (!emitGap(item.start - cursor) || !emitStage(std::move(item.stage))) return false;
        cursor = item.start + result.Stages.back().iDurationMs;
    }
    if (!emitGap(static_cast<std::uint32_t>(lifetime) - cursor)) return false;
    result.iNextStageOrdinal = stageOrdinal + 1u;
    if (!Validate_Shape(staged, outStatus, true, true)) return false;
    // These consumers own one boss/HUD/camera state rather than one state per
    // occurrence. Reject conflicting owners while independent lanes coexist.
    struct ExclusiveWindow { std::uint32_t start, end; std::string owner; };
    std::unordered_map<std::string, std::vector<ExclusiveWindow>> exclusiveWindows;
    const auto admitExclusive = [&](const std::string& kind, const auto& row) -> bool {
        auto& windows = exclusiveWindows[kind];
        const auto end = row.iStartMs + row.iDurationMs;
        auto ownerEnd = std::string::npos;
        for (const auto* family : { ".logic.", ".sceneprofile.", ".presentation." })
        {
            const auto found = row.strOccurrenceId.rfind(family);
            if (found != std::string::npos) ownerEnd = found;
        }
        const auto owner = row.strOccurrenceId.substr(0, ownerEnd);
        if (std::any_of(windows.begin(), windows.end(), [&](const auto& previous) {
            return previous.owner != owner && previous.start < end && row.iStartMs < previous.end;
        })) return fail("Overlapping " + kind + " windows would share one runtime owner: " + row.strOccurrenceId);
        windows.push_back({row.iStartMs, end, owner});
        return true;
    };
    for (const auto& row : result.LogicOccurrences)
    {
        if (!row.bEnabled) continue;
        const auto definition = std::find_if(staged.Logics.begin(), staged.Logics.end(),
            [&](const auto& item) { return item.strLogicId == row.strLogicId; });
        if (definition == staged.Logics.end()) return fail("Expanded Logic definition is missing.");
        const auto& kind = definition->strJudgementKind;
        if ((kind == "STAGGER_WINDOW" || kind == "COUNTER_WINDOW" || kind == "POSE_INPUT") &&
            !admitExclusive(kind, row)) return false;
    }
    for (const auto& row : result.SceneProfileOccurrences)
        if (!admitExclusive("SCENE_PROFILE", row)) return false;
    for (const auto& row : result.PresentationOccurrences)
    {
        const auto resource = std::find_if(staged.PresentationResources.begin(), staged.PresentationResources.end(),
            [&](const auto& item) { return item.strResourceId == row.strResourceId; });
        if (resource != staged.PresentationResources.end() && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA &&
            !admitExclusive("CAMERA", row)) return false;
    }
    if (!Try_ResolveAnimationBlendWindows(staged, result, result.AnimationBlendWindows, outStatus)) return false;
    outDocument = std::move(staged);
    outStatus.clear();
    return true;
}

bool_t Client::CKoukuSaydonCompositionDocument::Try_ResolveAnimationBlendWindows(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
    std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW>& outWindows, std::string& status)
{
    std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> staged;
    std::vector<KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE> animations;
    std::uint64_t origin = 0u;
    for (const auto& stage : pattern.Stages)
    {
        const auto first = std::min_element(stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
            [](const auto& a, const auto& b) { return a.iStartOffsetMs != b.iStartOffsetMs ?
                a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; });
        for (auto row : stage.AnimationOccurrences)
        {
            if (origin + row.iStartOffsetMs > MAX_TIME_MS) { status = "Animation blend timeline exceeds 600 seconds."; return false; }
            row.iPoseStartMs = uint32_t(origin) + (row.strOccurrenceId == first->strOccurrenceId ? 0u : row.iStartOffsetMs);
            row.iStartOffsetMs += uint32_t(origin);
            animations.push_back(std::move(row));
        }
        origin += stage.iDurationMs;
    }
    const auto lifetime = pattern.iDurationMs ? pattern.iDurationMs : origin;
    std::sort(animations.begin(), animations.end(), [](const auto& a, const auto& b) {
        return a.iPoseStartMs != b.iPoseStartMs ? a.iPoseStartMs < b.iPoseStartMs : a.strOccurrenceId < b.strOccurrenceId; });
    for (const auto& box : pattern.LogicOccurrences)
    {
        if (!box.bEnabled) continue;
        const auto logic = std::find_if(document.Logics.begin(), document.Logics.end(),
            [&](const auto& value) { return value.strLogicId == box.strLogicId; });
        if (logic == document.Logics.end() || logic->strTriggerKind != "ANIMATION_BLEND") continue;
        const auto fail = [&](const std::string& reason) { status = "Animation blend " + box.strOccurrenceId + ": " + reason; return false; };
        const uint64_t end = uint64_t(box.iStartMs) + box.iDurationMs;
        if (!box.iDurationMs || box.iDurationMs > 1000u || end > lifetime)
            return fail("the existing clip transition requires a 1..1000 ms box inside this Pattern.");
        size_t target = animations.size(), count = 0u;
        for (size_t i = 1u; i < animations.size(); ++i)
            if (box.iStartMs <= animations[i].iPoseStartMs && animations[i].iPoseStartMs <= end)
            { target = i; ++count; }
        if (count != 1u) return fail("the box must contain exactly one transition between consecutive pose owners.");
        const auto& source = animations[target - 1u];
        const auto& destination = animations[target];
        if (source.iPoseStartMs >= destination.iPoseStartMs || box.iStartMs < source.iPoseStartMs ||
            (target + 1u < animations.size() && end >= animations[target + 1u].iPoseStartMs))
            return fail("the source/target pair is ambiguous or another pose owner enters this window.");
        if (Resolve_ActorProfileId(source.strProfileId) != Resolve_ActorProfileId(destination.strProfileId))
            return fail("both clips must belong to the same actor model.");
        for (const auto& row : animations)
            if (row.iBlendInMs && uint64_t(row.iStartOffsetMs) < end &&
                uint64_t(box.iStartMs) < uint64_t(row.iStartOffsetMs) + row.iBlendInMs)
                return fail("the Logic window overlaps an existing clip blendInMs.");
        if (std::any_of(staged.begin(), staged.end(), [&](const auto& previous) {
            return previous.iStartMs < end && box.iStartMs < uint64_t(previous.iStartMs) + previous.iDurationMs; }))
            return fail("two Logic blend windows overlap.");
        staged.push_back({box.strOccurrenceId, box.iStartMs, box.iDurationMs, source, destination});
    }
    std::sort(staged.begin(), staged.end(), [](const auto& a, const auto& b) { return a.iStartMs < b.iStartMs; });
    outWindows = std::move(staged);
    status.clear();
    return true;
}
