#include "EncounterPatternReference.h"

#include "DataJson.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.encounter-profile";
	constexpr uint32_t FORMAT_VERSION = 3u;

	bool_t Read_TextFile(
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

	/* Rejects documents that carry properties this reader does not understand,
	   so a schema change fails loudly instead of being silently dropped. */
	bool_t Is_ExactObject(
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

	/* Server-only fields the Client reference deliberately does not model. It
	   still refuses anything it has not been told about, so a typo is rejected
	   rather than silently ignored. */
	bool_t Is_ExactObjectWithOptional(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> requiredKeys,
		const std::initializer_list<const char_t*> optionalKeys)
	{
		if (!value.Is_Object())
			return false;
		size_t present = requiredKeys.size();
		for (const char_t* key : optionalKeys)
		{
			if (nullptr != value.Find(key))
				++present;
		}
		if (value.Get_Object().size() != present)
			return false;
		return std::all_of(requiredKeys.begin(), requiredKeys.end(),
			[&value](const char_t* key)
			{
				return nullptr != value.Find(key);
			});
	}

	bool_t Read_String(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const bool_t allowEmpty,
		std::string& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_String() ||
			(!allowEmpty && value->Get_String().empty()))
		{
			return false;
		}
		outValue = value->Get_String();
		return true;
	}

	bool_t Read_Unsigned(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const uint32_t maximum,
		uint32_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Number())
			return false;
		const double number = value->Get_Number();
		if (!std::isfinite(number) || std::floor(number) != number ||
			number < 0.0 || number > static_cast<double>(maximum))
		{
			return false;
		}
		outValue = static_cast<uint32_t>(number);
		return true;
	}

	bool_t Is_FiniteNumber(
		const DATA_JSON_VALUE& parent,
		const char_t* key)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		return nullptr != value && value->Is_Number() &&
			std::isfinite(value->Get_Number());
	}

	bool_t Is_Boolean(
		const DATA_JSON_VALUE& parent,
		const char_t* key)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		return nullptr != value && value->Is_Boolean();
	}

	bool_t Read_Float(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		f32_t& outValue)
	{
		if (!Is_FiniteNumber(parent, key))
			return false;
		outValue = static_cast<f32_t>(parent.Find(key)->Get_Number());
		return true;
	}

	bool_t Is_FiniteNumberArray(
		const DATA_JSON_VALUE& parent,
		const char_t* key)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Array())
			return false;
		return std::all_of(value->Get_Array().begin(),
			value->Get_Array().end(),
			[](const DATA_JSON_VALUE& element)
			{
				return element.Is_Number() &&
					std::isfinite(element.Get_Number());
			});
	}
}

bool_t Client::CEncounterPatternReference::Load(
	const std::filesystem::path& path,
	std::string& outStatus)
{
	std::string text;
	if (path.empty() || !Read_TextFile(path, text))
	{
		outStatus = "Encounter reference is unreadable: " + path.string();
		return false;
	}

	std::string parseError;
	DATA_JSON_VALUE root;
	if (!CDataJson::Parse(text, root, parseError))
	{
		outStatus = "Encounter reference parse failed: " + parseError;
		return false;
	}
	if (!Is_ExactObjectWithOptional(root, {
			"schema", "formatVersion", "encounterId", "bossArchetypeId",
			"authority", "fixedTickHz", "states", "patterns" },
			{ "introPatternId" }))
	{
		outStatus = "Encounter reference root has unexpected properties";
		return false;
	}

	std::string schema;
	std::string encounterId;
	std::string bossArchetypeId;
	std::string authority;
	uint32_t formatVersion = 0u;
	uint32_t fixedTickHz = 0u;
	if (!Read_String(root, "schema", false, schema) || SCHEMA != schema ||
		!Read_Unsigned(root, "formatVersion", FORMAT_VERSION, formatVersion) ||
		FORMAT_VERSION != formatVersion ||
		!Read_String(root, "encounterId", false, encounterId) ||
		!Read_String(root, "bossArchetypeId", false, bossArchetypeId) ||
		!Read_String(root, "authority", false, authority) ||
		!Read_Unsigned(root, "fixedTickHz", MAX_FIXED_TICK_HZ, fixedTickHz) ||
		0u == fixedTickHz)
	{
		outStatus = "Encounter reference header is invalid";
		return false;
	}

	const DATA_JSON_VALUE* states = root.Find("states");
	const DATA_JSON_VALUE* patterns = root.Find("patterns");
	if (nullptr == states || !states->Is_Array() ||
		nullptr == patterns || !patterns->Is_Array() ||
		patterns->Get_Array().empty() ||
		patterns->Get_Array().size() > MAX_PATTERN_COUNT)
	{
		outStatus = "Encounter reference pattern array is invalid";
		return false;
	}

	std::vector<ENCOUNTER_PATTERN_REFERENCE> staged;
	staged.reserve(patterns->Get_Array().size());
	std::unordered_set<std::string> patternIds;

	for (const DATA_JSON_VALUE& entry : patterns->Get_Array())
	{
		if (!Is_ExactObjectWithOptional(entry, {
				"patternId", "displayName", "actionId", "sourceActionIds",
				"selectionMode", "minimumHealthBar", "maximumHealthBar",
				"triggerHealthBar", "triggerOrder", "armorRequirement",
				"phaseRequirement", "invulnerableWhileRunning",
				"selectionWeight", "maximumConsecutiveUses", "minimumRange",
				"maximumRange", "stages" },
				{ "serverMotion" }))
		{
			outStatus = "Encounter pattern has unexpected properties";
			return false;
		}

		ENCOUNTER_PATTERN_REFERENCE pattern;
		uint32_t ignored = 0u;
		std::string armorRequirement;
		std::string phaseRequirement;
		if (!Read_String(entry, "patternId", false, pattern.patternId) ||
			!Read_String(entry, "displayName", false, pattern.displayName) ||
			!Read_String(entry, "actionId", false, pattern.actionId) ||
			!Read_String(entry, "selectionMode", false,
				pattern.selectionMode) ||
			!Read_Unsigned(entry, "minimumHealthBar", 1000u, ignored) ||
			!Read_Unsigned(entry, "maximumHealthBar", 1000u, ignored) ||
			!Read_Unsigned(entry, "triggerHealthBar", 1000u,
				pattern.iTriggerHealthBar) ||
			!Read_Unsigned(entry, "triggerOrder", 1000u, ignored) ||
			!Read_String(entry, "armorRequirement", false,
				armorRequirement) ||
			(armorRequirement != "ANY" &&
				armorRequirement != "ARMORED" &&
				armorRequirement != "STRIPPED") ||
			!Read_String(entry, "phaseRequirement", false,
				phaseRequirement) ||
			(phaseRequirement != "ANY" &&
				phaseRequirement != "PHASE_ONE" &&
				phaseRequirement != "PHASE_TWO") ||
			!Is_Boolean(entry, "invulnerableWhileRunning") ||
			!Read_Unsigned(entry, "selectionWeight", 1000u, ignored) ||
			!Read_Unsigned(entry, "maximumConsecutiveUses", 1000u, ignored) ||
			!Is_FiniteNumber(entry, "minimumRange") ||
			!Is_FiniteNumber(entry, "maximumRange") ||
			!Is_FiniteNumberArray(entry, "sourceActionIds"))
		{
			outStatus = "Encounter pattern field is invalid";
			return false;
		}
		if (!patternIds.insert(pattern.patternId).second)
		{
			outStatus = "Duplicate encounter pattern: " + pattern.patternId;
			return false;
		}
		for (const DATA_JSON_VALUE& sourceId :
			entry.Find("sourceActionIds")->Get_Array())
		{
			pattern.sourceActionIds.push_back(
				static_cast<uint32_t>(sourceId.Get_Number()));
		}

		const DATA_JSON_VALUE* stages = entry.Find("stages");
		if (nullptr == stages || !stages->Is_Array() ||
			stages->Get_Array().empty() ||
			stages->Get_Array().size() > MAX_STAGE_COUNT)
		{
			outStatus = "Encounter pattern stage array is invalid: " +
				pattern.patternId;
			return false;
		}

		std::unordered_set<std::string> stageIds;
		for (const DATA_JSON_VALUE& stageEntry : stages->Get_Array())
		{
			if (!Is_ExactObject(stageEntry, {
					"stageId", "actionId", "stageKind", "durationMs",
					"hitShape", "hitOuterRadius", "hitInnerRadius",
					"hitAngleDegrees", "hitLength", "hitHalfWidth",
					"hitCount", "hitIntervalMs", "hitDelayMs",
					"serverDamageProfileId",
					"pushRangeM", "pushMs", "knockdown", "downMs" }))
			{
				outStatus = "Encounter stage has unexpected properties: " +
					pattern.patternId;
				return false;
			}

			ENCOUNTER_STAGE_REFERENCE stage;
			if (!Read_String(stageEntry, "stageId", false, stage.stageId) ||
				!Read_String(stageEntry, "actionId", false, stage.actionId) ||
				!Read_String(stageEntry, "stageKind", false,
					stage.stageKind) ||
				!Read_Unsigned(stageEntry, "durationMs",
					MAX_STAGE_DURATION_MS, stage.iDurationMs) ||
				!Read_String(stageEntry, "hitShape", false, stage.hitShape) ||
				!Read_Unsigned(stageEntry, "hitCount", 1000u,
					stage.iHitCount) ||
				!Read_Unsigned(stageEntry, "hitIntervalMs",
					MAX_STAGE_DURATION_MS, stage.iHitIntervalMs) ||
				!Read_String(stageEntry, "serverDamageProfileId", true,
					stage.serverDamageProfileId) ||
				!Read_Float(stageEntry, "hitOuterRadius",
					stage.fHitOuterRadius) ||
				!Read_Float(stageEntry, "hitInnerRadius",
					stage.fHitInnerRadius) ||
				!Read_Float(stageEntry, "hitAngleDegrees",
					stage.fHitAngleDegrees) ||
				!Read_Float(stageEntry, "hitLength", stage.fHitLength) ||
				!Read_Float(stageEntry, "hitHalfWidth", stage.fHitHalfWidth))
			{
				outStatus = "Encounter stage field is invalid: " +
					pattern.patternId;
				return false;
			}
			if (!stageIds.insert(stage.stageId).second)
			{
				outStatus = "Duplicate encounter stage: " +
					pattern.patternId + "/" + stage.stageId;
				return false;
			}

			stage.iStartOffsetMs = pattern.iTotalDurationMs;
			pattern.iTotalDurationMs += stage.iDurationMs;
			pattern.stages.push_back(std::move(stage));
		}

		staged.push_back(std::move(pattern));
	}

	m_EncounterId = std::move(encounterId);
	m_BossArchetypeId = std::move(bossArchetypeId);
	m_iFixedTickHz = fixedTickHz;
	m_Patterns = std::move(staged);
	m_isReady = true;
	outStatus = "Loaded encounter reference: " + m_EncounterId + " (" +
		std::to_string(m_Patterns.size()) + " patterns)";
	return true;
}

void Client::CEncounterPatternReference::Clear()
{
	m_EncounterId.clear();
	m_BossArchetypeId.clear();
	m_iFixedTickHz = 0u;
	m_Patterns.clear();
	m_isReady = false;
}

const Client::ENCOUNTER_PATTERN_REFERENCE*
Client::CEncounterPatternReference::Find_Pattern(
	const std::string& patternId) const
{
	const auto iter = std::find_if(m_Patterns.begin(), m_Patterns.end(),
		[&patternId](const ENCOUNTER_PATTERN_REFERENCE& pattern)
		{
			return pattern.patternId == patternId;
		});
	return m_Patterns.end() == iter ? nullptr : &(*iter);
}

uint32_t Client::CEncounterPatternReference::To_ServerTick(
	const uint32_t milliseconds,
	const uint32_t fixedTickHz)
{
	if (0u == fixedTickHz)
		return 0u;
	const uint64_t numerator =
		static_cast<uint64_t>(milliseconds) *
		static_cast<uint64_t>(fixedTickHz) + 999ull;
	return static_cast<uint32_t>(numerator / 1000ull);
}
