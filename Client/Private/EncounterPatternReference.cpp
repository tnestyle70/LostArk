#include "EncounterPatternReference.h"

#include "DataJson.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.encounter-profile";
	constexpr uint32_t FORMAT_VERSION = 4u;
	constexpr size_t MAX_STAGE_AUXILIARY_COUNT = 8u;
	constexpr uint32_t MAX_REFERENCE_STAGE_DURATION_MS = 60000u;

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

	bool_t Read_OptionalOrderedUnsignedArray(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const size_t maximumCount,
		const uint32_t maximumValue,
		std::vector<uint32_t>& outValues)
	{
		outValues.clear();
		const DATA_JSON_VALUE* values = parent.Find(key);
		if (nullptr == values)
			return true;
		if (!values->Is_Array() || values->Get_Array().empty() ||
			values->Get_Array().size() > maximumCount)
		{
			return false;
		}

		uint32_t previous = 0u;
		for (size_t index = 0u; index < values->Get_Array().size(); ++index)
		{
			const DATA_JSON_VALUE& value = values->Get_Array()[index];
			if (!value.Is_Number())
				return false;
			const double number = value.Get_Number();
			if (!std::isfinite(number) || std::floor(number) != number ||
				number < 0.0 || number > static_cast<double>(maximumValue))
			{
				return false;
			}
			const uint32_t current = static_cast<uint32_t>(number);
			if (0u != index && current <= previous)
				return false;
			outValues.push_back(current);
			previous = current;
		}
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

	bool_t Is_StableId(const std::string& value)
	{
		return !value.empty() && value.size() <= 128u &&
			std::all_of(value.begin(), value.end(),
				[](const unsigned char character)
				{
					return 0 != std::isalnum(character) || character == '_' ||
						character == '-' || character == '.';
				});
	}

	bool_t Validate_StageMotion(
		const DATA_JSON_VALUE& stage,
		bool_t& outHasForwardMotion)
	{
		outHasForwardMotion = false;
		const DATA_JSON_VALUE* motion = stage.Find("motion");
		if (nullptr == motion)
			return true;
		std::string kind;
		if (!Is_ExactObject(*motion, { "kind", "distance" }) ||
			!Read_String(*motion, "kind", false, kind) || kind != "FORWARD" ||
			!Is_FiniteNumber(*motion, "distance"))
		{
			return false;
		}
		const double distance = motion->Find("distance")->Get_Number();
		outHasForwardMotion = distance > 0.0 && distance <= 1000.0;
		return outHasForwardMotion;
	}

	bool_t Validate_StageActions(
		const DATA_JSON_VALUE& stage,
		std::unordered_set<std::string>& activeLifetimes,
		bool_t& outHasCounterableEnter,
		bool_t& outHasCounterableExit,
		bool_t& outHasStaggerEnter,
		bool_t& outHasStaggerExit)
	{
		outHasCounterableEnter = false;
		outHasCounterableExit = false;
		outHasStaggerEnter = false;
		outHasStaggerExit = false;
		const DATA_JSON_VALUE* actions = stage.Find("actions");
		if (nullptr == actions)
			return true;
		if (!actions->Is_Array() || actions->Get_Array().empty() ||
			actions->Get_Array().size() > MAX_STAGE_AUXILIARY_COUNT)
		{
			return false;
		}
		uint32_t stageDurationMs = 0u;
		if (!Read_Unsigned(stage, "durationMs", MAX_REFERENCE_STAGE_DURATION_MS,
				stageDurationMs) || 0u == stageDurationMs)
		{
			return false;
		}

		std::unordered_set<std::string> actionKeys;
		for (const DATA_JSON_VALUE& action : actions->Get_Array())
		{
			const DATA_JSON_VALUE* actionKind = action.Find("kind");
			if (nullptr == actionKind || !actionKind->Is_String())
				return false;
			if (actionKind->Get_String() == "SPAWN_COMBAT_OBJECT_VOLLEY")
			{
				std::string trigger;
				std::string targetId;
				std::string targetingPolicy;
				std::string layout;
				std::string arenaAnchorPolicy;
				uint32_t countPerResolvedTarget = 0u;
				uint32_t maximumTotalObjects = 0u;
				uint32_t spawnCount = 0u;
				uint32_t spawnIntervalMs = 0u;
				uint32_t arenaRandomCount = 0u;
				if (!Is_ExactObject(action,
						{ "trigger", "kind", "targetId", "targetingPolicy",
						  "countPerResolvedTarget", "layout", "radiusM",
						  "startAngleDegrees", "angleStepDegrees", "allowOverlap",
						  "maximumTotalObjects", "spawnCount", "spawnIntervalMs",
						  "arenaRandomCount", "arenaRandomRadiusM",
						  "arenaHeightToleranceM", "arenaAnchorPolicy" }) ||
					!Read_String(action, "trigger", false, trigger) ||
					trigger != "ENTER" ||
					!Read_String(action, "targetId", false, targetId) ||
					!Is_StableId(targetId) ||
					!Read_String(action, "targetingPolicy", false,
						targetingPolicy) ||
					targetingPolicy != "PER_ALIVE_PLAYER" ||
					!Read_String(action, "layout", false, layout) ||
					!Read_Unsigned(action, "countPerResolvedTarget", 8u,
						countPerResolvedTarget) ||
					0u == countPerResolvedTarget ||
					!Read_Unsigned(action, "maximumTotalObjects", 64u,
						maximumTotalObjects) ||
					!Read_Unsigned(action, "spawnCount", 64u, spawnCount) ||
					0u == spawnCount ||
					!Read_Unsigned(action, "spawnIntervalMs",
						MAX_REFERENCE_STAGE_DURATION_MS, spawnIntervalMs) ||
					(spawnCount > 1u && 0u == spawnIntervalMs) ||
					(1u == spawnCount && 0u != spawnIntervalMs) ||
					static_cast<uint64_t>(spawnCount - 1u) * spawnIntervalMs >=
						static_cast<uint64_t>(stageDurationMs) ||
					!Read_Unsigned(action, "arenaRandomCount", 32u,
						arenaRandomCount) ||
					0u == arenaRandomCount ||
					maximumTotalObjects <
						countPerResolvedTarget + arenaRandomCount ||
					!Is_FiniteNumber(action, "radiusM") ||
					!Is_FiniteNumber(action, "startAngleDegrees") ||
					!Is_FiniteNumber(action, "angleStepDegrees") ||
					!Is_Boolean(action, "allowOverlap") ||
					!Is_FiniteNumber(action, "arenaRandomRadiusM") ||
					action.Find("arenaRandomRadiusM")->Get_Number() <= 0.0 ||
					action.Find("arenaRandomRadiusM")->Get_Number() > 1000.0 ||
					!Is_FiniteNumber(action, "arenaHeightToleranceM") ||
					action.Find("arenaHeightToleranceM")->Get_Number() < 0.0 ||
					action.Find("arenaHeightToleranceM")->Get_Number() > 1000.0 ||
					!Read_String(action, "arenaAnchorPolicy", false,
						arenaAnchorPolicy) ||
					arenaAnchorPolicy != "BOSS_SPAWN_POSITION" ||
					3u != spawnCount || 1333u != spawnIntervalMs ||
					4u != arenaRandomCount ||
					14.0 != action.Find("arenaRandomRadiusM")->Get_Number() ||
					1.0 != action.Find("arenaHeightToleranceM")->Get_Number())
				{
					return false;
				}

				const double radius = action.Find("radiusM")->Get_Number();
				const double startAngle =
					action.Find("startAngleDegrees")->Get_Number();
				const double angleStep =
					action.Find("angleStepDegrees")->Get_Number();
				const bool_t allowOverlap =
					action.Find("allowOverlap")->Get_Boolean();
				const bool_t isSingle = 1u == countPerResolvedTarget;
				if (radius < 0.0 ||
					(isSingle && (layout != "SINGLE" || 0.0 != radius ||
						0.0 != startAngle || 0.0 != angleStep || allowOverlap)) ||
					(!isSingle && (layout != "RADIAL" || radius <= 0.0 ||
						0.0 == angleStep || allowOverlap)))
				{
					return false;
				}

				const std::string actionKey = trigger + "\n" + targetId;
				if (!actionKeys.insert(actionKey).second)
					return false;
				/* A scheduled volley is still owned by one Server stage action.
				   Its repeated spawns do not open a cross-stage lifetime here. */
				continue;
			}

			std::string trigger;
			std::string kind;
			std::string targetId;
			uint32_t value = 0u;
			uint32_t durationMs = 0u;
			if (!Is_ExactObject(action,
					{ "trigger", "kind", "targetId", "value", "durationMs" }) ||
				!Read_String(action, "trigger", false, trigger) ||
				!Read_String(action, "kind", false, kind) ||
				!Read_String(action, "targetId", false, targetId) ||
				!Is_StableId(targetId) ||
				!Read_Unsigned(action, "value", UINT32_MAX, value) ||
				!Read_Unsigned(action, "durationMs", UINT32_MAX, durationMs) ||
				0u != durationMs ||
				(trigger != "ENTER" && trigger != "EXIT"))
			{
				return false;
			}

			bool_t validKind = false;
			if (kind == "SET_BOSS_FLAG")
			{
				validKind = (targetId == "boss.flag.groggy" ||
					targetId == "boss.flag.invulnerable" ||
					targetId == "boss.flag.counterable") && value <= 1u;
			}
			else if (kind == "SET_STAGGER_GAUGE")
				validKind = targetId == "boss.gauge.stagger";
			else if (kind == "SET_SHIELD")
				validKind = targetId == "boss.gauge.shield";
			else if (kind == "SPAWN_COMBAT_OBJECT")
				validKind = trigger == "ENTER" && 1u == value;
			else if (kind == "SET_GAMEPLAY_PHASE")
				validKind = trigger == "ENTER" &&
					targetId == "boss.phase.gameplay" && 2u == value;
			if (!validKind || (trigger == "ENTER" ? 0u == value : 0u != value))
				return false;

			const std::string actionKey = trigger + "\n" + targetId;
			if (!actionKeys.insert(actionKey).second)
				return false;
			if (kind == "SPAWN_COMBAT_OBJECT" ||
				kind == "SET_GAMEPLAY_PHASE")
				continue;

			const std::string lifetimeKey = kind + "\n" + targetId;
			if (trigger == "ENTER")
			{
				if (!activeLifetimes.insert(lifetimeKey).second)
					return false;
			}
			else if (0u == activeLifetimes.erase(lifetimeKey))
				return false;

			const bool_t isCounterable = kind == "SET_BOSS_FLAG" &&
				targetId == "boss.flag.counterable";
			const bool_t isStagger = kind == "SET_STAGGER_GAUGE" &&
				targetId == "boss.gauge.stagger";
			outHasCounterableEnter = outHasCounterableEnter ||
				(isCounterable && trigger == "ENTER");
			outHasCounterableExit = outHasCounterableExit ||
				(isCounterable && trigger == "EXIT");
			outHasStaggerEnter = outHasStaggerEnter ||
				(isStagger && trigger == "ENTER");
			outHasStaggerExit = outHasStaggerExit ||
				(isStagger && trigger == "EXIT");
		}
		return true;
	}

	bool_t Is_KnownBranchOutcome(const std::string& outcome)
	{
		return outcome == "TIMEOUT" || outcome == "COUNTER_HIT" ||
			outcome == "STAGGER_BROKEN" || outcome == "WALL_CONTACT" ||
			outcome == "PART_DESTROYED" || outcome == "PROP_DESTROYED" ||
			outcome == "SUMMON_DEAD" || outcome == "ALL_PLAYERS_GRABBED";
	}

	bool_t Validate_StageBranches(
		const DATA_JSON_VALUE& stage,
		const std::unordered_set<std::string>& stageActionIds,
		const std::string& currentActionId,
		const bool_t hasForwardMotion,
		const bool_t hasCounterableEnter,
		const bool_t hasCounterableExit,
		const bool_t hasStaggerEnter,
		const bool_t hasStaggerExit)
	{
		const DATA_JSON_VALUE* branches = stage.Find("branches");
		if (nullptr == branches)
			return true;
		if (!branches->Is_Array() || branches->Get_Array().empty() ||
			branches->Get_Array().size() > MAX_STAGE_AUXILIARY_COUNT)
		{
			return false;
		}

		std::unordered_set<std::string> outcomes;
		bool_t hasTimeout = false;
		bool_t hasWallContact = false;
		bool_t hasCounterHit = false;
		bool_t hasStaggerBroken = false;
		for (const DATA_JSON_VALUE& branch : branches->Get_Array())
		{
			std::string outcome;
			if (!Is_ExactObject(branch, { "outcome", "nextActionId" }) ||
				!Read_String(branch, "outcome", false, outcome) ||
				!Is_KnownBranchOutcome(outcome) ||
				!outcomes.insert(outcome).second)
			{
				return false;
			}
			const DATA_JSON_VALUE* nextActionId = branch.Find("nextActionId");
			if (nullptr == nextActionId)
				return false;
			if (!nextActionId->Is_Null())
			{
				if (!nextActionId->Is_String() ||
					!Is_StableId(nextActionId->Get_String()) ||
					0u == stageActionIds.count(nextActionId->Get_String()) ||
					nextActionId->Get_String() == currentActionId)
				{
					return false;
				}
			}
			hasTimeout = hasTimeout || outcome == "TIMEOUT";
			hasWallContact = hasWallContact || outcome == "WALL_CONTACT";
			hasCounterHit = hasCounterHit || outcome == "COUNTER_HIT";
			hasStaggerBroken = hasStaggerBroken || outcome == "STAGGER_BROKEN";
		}
		return hasTimeout && (!hasWallContact || hasForwardMotion) &&
			(!hasCounterHit || (hasCounterableEnter && hasCounterableExit)) &&
			(!hasStaggerBroken || (hasStaggerEnter && hasStaggerExit));
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
				"selectionMode", "category", "minimumPhase", "maximumPhase",
				"targetPolicy", "aimPolicy", "minimumHealthBar",
				"maximumHealthBar", "triggerHealthBar", "triggerOrder",
				"armorRequirement", "phaseRequirement",
				"invulnerableWhileRunning", "selectionWeight",
				"maximumConsecutiveUses", "minimumRange", "maximumRange",
				"stages" },
				{ "serverMotion" }))
		{
			outStatus = "Encounter pattern has unexpected properties";
			return false;
		}

		ENCOUNTER_PATTERN_REFERENCE pattern;
		std::string category;
		std::string targetPolicy;
		std::string aimPolicy;
		uint32_t minimumPhase = 0u;
		uint32_t maximumPhase = 0u;
		uint32_t ignored = 0u;
		std::string armorRequirement;
		std::string phaseRequirement;
		if (!Read_String(entry, "patternId", false, pattern.patternId) ||
			!Read_String(entry, "displayName", false, pattern.displayName) ||
			!Read_String(entry, "actionId", false, pattern.actionId) ||
			!Read_String(entry, "selectionMode", false,
				pattern.selectionMode) ||
			!Read_String(entry, "category", false, category) ||
			(category != "NORMAL" && category != "IMPORTANT" &&
				category != "MECHANIC") ||
			!Read_Unsigned(entry, "minimumPhase", 3u, minimumPhase) ||
			!Read_Unsigned(entry, "maximumPhase", 3u, maximumPhase) ||
			0u == minimumPhase || minimumPhase > maximumPhase ||
			!Read_String(entry, "targetPolicy", false, targetPolicy) ||
			(targetPolicy != "NONE" && targetPolicy != "NEAREST_EACH_TICK" &&
				targetPolicy != "LOCK_NEAREST_ON_START" &&
				targetPolicy != "LOCK_RANDOM_ALIVE_ON_START") ||
			!Read_String(entry, "aimPolicy", false, aimPolicy) ||
			(aimPolicy != "NONE" && aimPolicy != "TRACK_TARGET_EACH_TICK" &&
				aimPolicy != "LOCK_FACING_ON_START" &&
				aimPolicy != "FACE_MOTION_ANCHOR") ||
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
		std::unordered_set<std::string> stageActionIds;
		for (const DATA_JSON_VALUE& stageEntry : stages->Get_Array())
		{
			std::string stageActionId;
			if (!Read_String(stageEntry, "actionId", false, stageActionId) ||
				!Is_StableId(stageActionId) ||
				!stageActionIds.insert(stageActionId).second)
			{
				outStatus = "Encounter pattern stage action identity is invalid: " +
					pattern.patternId;
				return false;
			}
		}
		std::unordered_set<std::string> activeStageActionLifetimes;
		for (const DATA_JSON_VALUE& stageEntry : stages->Get_Array())
		{
			if (!Is_ExactObjectWithOptional(stageEntry, {
					"stageId", "actionId", "stageKind", "durationMs",
					"hitShape", "hitOuterRadius", "hitInnerRadius",
					"hitAngleDegrees", "hitLength", "hitHalfWidth",
					"hitCount", "hitIntervalMs", "hitDelayMs",
					"serverDamageProfileId",
					"pushRangeM", "pushMs", "knockdown", "downMs" },
					{ "hitOffsetsMs", "motion", "actions", "branches" }))
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
				!Read_Unsigned(stageEntry, "hitDelayMs",
					MAX_STAGE_DURATION_MS, stage.iHitDelayMs) ||
				!Read_OptionalOrderedUnsignedArray(stageEntry, "hitOffsetsMs",
					1000u, MAX_STAGE_DURATION_MS, stage.hitOffsetsMs) ||
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
			const bool_t hasExplicitHitOffsets = !stage.hitOffsetsMs.empty();
			const bool_t validExplicitHitSchedule =
				hasExplicitHitOffsets &&
				stage.hitOffsetsMs.size() == stage.iHitCount &&
				0u == stage.iHitIntervalMs && 0u == stage.iHitDelayMs &&
				stage.hitOffsetsMs.back() < stage.iDurationMs;
			const bool_t validLegacyHitSchedule =
				!hasExplicitHitOffsets && stage.iHitCount > 0u &&
				(1u == stage.iHitCount ? 0u == stage.iHitIntervalMs :
					stage.iHitIntervalMs > 0u) &&
				static_cast<uint64_t>(stage.iHitDelayMs) +
					static_cast<uint64_t>(stage.iHitCount - 1u) *
						stage.iHitIntervalMs < stage.iDurationMs;
			const bool_t validEmptyHitSchedule = 0u == stage.iHitCount &&
				!hasExplicitHitOffsets && 0u == stage.iHitIntervalMs &&
				0u == stage.iHitDelayMs;
			if (!validExplicitHitSchedule && !validLegacyHitSchedule &&
				!validEmptyHitSchedule)
			{
				outStatus = "Encounter stage hit schedule is invalid: " +
					pattern.patternId + "/" + stage.stageId;
				return false;
			}
			if (!stageIds.insert(stage.stageId).second)
			{
				outStatus = "Duplicate encounter stage: " +
					pattern.patternId + "/" + stage.stageId;
				return false;
			}

			bool_t hasForwardMotion = false;
			bool_t hasCounterableEnter = false;
			bool_t hasCounterableExit = false;
			bool_t hasStaggerEnter = false;
			bool_t hasStaggerExit = false;
			if (!Validate_StageMotion(stageEntry, hasForwardMotion) ||
				!Validate_StageActions(stageEntry, activeStageActionLifetimes,
					hasCounterableEnter, hasCounterableExit,
					hasStaggerEnter, hasStaggerExit) ||
				!Validate_StageBranches(stageEntry, stageActionIds, stage.actionId,
					hasForwardMotion, hasCounterableEnter, hasCounterableExit,
					hasStaggerEnter, hasStaggerExit))
			{
				outStatus = "Encounter stage v4 field is invalid: " +
					pattern.patternId + "/" + stage.stageId;
				return false;
			}

			stage.iStartOffsetMs = pattern.iTotalDurationMs;
			pattern.iTotalDurationMs += stage.iDurationMs;
			pattern.stages.push_back(std::move(stage));
		}
		if (!activeStageActionLifetimes.empty())
		{
			outStatus = "Encounter stage action lifetime is not closed: " +
				pattern.patternId;
			return false;
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
