#include "EncounterPatternReference.h"

#include "DataJson.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <unordered_map>
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

	bool_t Validate_FiniteStageGraph(const DATA_JSON_VALUE& pattern)
	{
		const DATA_JSON_VALUE* stages = pattern.Find("stages");
		if (nullptr == stages || !stages->Is_Array() || stages->Get_Array().empty())
			return false;
		const auto& rows = stages->Get_Array();
		std::unordered_map<std::string, size_t> stageByAction;
		for (size_t index = 0u; index < rows.size(); ++index)
		{
			std::string action;
			if (!Read_String(rows[index], "actionId", false, action) ||
				!stageByAction.emplace(action, index).second)
				return false;
		}
		std::vector<std::vector<size_t>> successors(rows.size());
		std::vector<size_t> incoming(rows.size(), 0u);
		for (size_t index = 0u; index < rows.size(); ++index)
		{
			bool hasTimeout = false;
			const DATA_JSON_VALUE* branches = rows[index].Find("branches");
			if (nullptr != branches)
			{
				if (!branches->Is_Array())
					return false;
				for (const DATA_JSON_VALUE& branch : branches->Get_Array())
				{
					std::string outcome;
					const DATA_JSON_VALUE* target = branch.Find("nextActionId");
					if (!Read_String(branch, "outcome", false, outcome) || nullptr == target)
						return false;
					hasTimeout = hasTimeout || outcome == "TIMEOUT";
					if (target->Is_Null())
						continue;
					if (!target->Is_String())
						return false;
					const auto next = stageByAction.find(target->Get_String());
					if (stageByAction.end() == next)
						return false;
					successors[index].push_back(next->second);
					++incoming[next->second];
				}
			}
			if (!hasTimeout && index + 1u < rows.size())
			{
				successors[index].push_back(index + 1u);
				++incoming[index + 1u];
			}
		}
		std::vector<size_t> ready;
		for (size_t index = 0u; index < incoming.size(); ++index)
			if (0u == incoming[index])
				ready.push_back(index);
		for (size_t index = 0u; index < ready.size(); ++index)
			for (const size_t next : successors[ready[index]])
				if (0u == --incoming[next])
					ready.push_back(next);
		return ready.size() == rows.size();
	}

	bool_t Validate_PatternServerMotion(const DATA_JSON_VALUE& pattern)
	{
		const DATA_JSON_VALUE* motion = pattern.Find("serverMotion");
		if (nullptr == motion || motion->Is_Null())
			return true;
		if (!Is_ExactObjectWithOptional(*motion,
			{ "kind", "anchorId", "landingPosition", "apexHeight", "travelStageId",
			  "takeoffStartMs", "takeoffEndMs", "travelStartMs", "travelEndMs" },
			{ "moveToAnchorBeforeTakeoff" }))
			return false;
		std::string kind, anchorId, travelStageId;
		uint32_t takeoffStart = 0u, takeoffEnd = 0u, travelStart = 0u, travelEnd = 0u;
		if (!Read_String(*motion, "kind", false, kind) ||
			(kind != "LEAP_TO_ANCHOR" && kind != "LEAP_TO_TARGET") ||
			!Read_String(*motion, "anchorId", false, anchorId) || !Is_StableId(anchorId) ||
			!Read_String(*motion, "travelStageId", false, travelStageId) || !Is_StableId(travelStageId) ||
			!Is_FiniteNumber(*motion, "apexHeight") ||
			motion->Find("apexHeight")->Get_Number() <= 0.0 ||
			motion->Find("apexHeight")->Get_Number() > 200.0 ||
			!Read_Unsigned(*motion, "takeoffStartMs", UINT32_MAX, takeoffStart) ||
			!Read_Unsigned(*motion, "takeoffEndMs", UINT32_MAX, takeoffEnd) ||
			!Read_Unsigned(*motion, "travelStartMs", UINT32_MAX, travelStart) ||
			!Read_Unsigned(*motion, "travelEndMs", UINT32_MAX, travelEnd) ||
			takeoffStart >= takeoffEnd || travelStart >= travelEnd)
			return false;
		const DATA_JSON_VALUE* approach = motion->Find("moveToAnchorBeforeTakeoff");
		if (nullptr != approach && (!approach->Is_Boolean() ||
			(approach->Get_Boolean() && 0u == takeoffStart)))
			return false;
		const DATA_JSON_VALUE* landing = motion->Find("landingPosition");
		if (nullptr == landing || !landing->Is_Array() || landing->Get_Array().size() != 3u ||
			!std::all_of(landing->Get_Array().begin(), landing->Get_Array().end(),
				[](const DATA_JSON_VALUE& value)
				{ return value.Is_Number() && std::isfinite(value.Get_Number()) &&
					std::abs(value.Get_Number()) <= 100000.0; }))
			return false;
		const DATA_JSON_VALUE* stages = pattern.Find("stages");
		if (nullptr == stages || !stages->Is_Array() || stages->Get_Array().size() < 2u)
			return false;
		uint32_t firstDuration = 0u;
		if (!Read_Unsigned(stages->Get_Array().front(), "durationMs", UINT32_MAX, firstDuration) ||
			takeoffEnd > firstDuration)
			return false;
		const auto travel = std::find_if(stages->Get_Array().begin() + 1u,
			stages->Get_Array().end(), [&travelStageId](const DATA_JSON_VALUE& stage)
			{
				const DATA_JSON_VALUE* id = stage.Find("stageId");
				return nullptr != id && id->Is_String() && id->Get_String() == travelStageId;
			});
		uint32_t travelDuration = 0u;
		return stages->Get_Array().end() != travel &&
			Read_Unsigned(*travel, "durationMs", UINT32_MAX, travelDuration) &&
			travelEnd <= travelDuration;
	}

	bool_t Validate_PatternFinale(
		const DATA_JSON_VALUE& pattern, const DATA_JSON_VALUE::ARRAY& patterns,
		const std::string& bossArchetypeId)
	{
		const DATA_JSON_VALUE* finale = pattern.Find("finale");
		if (nullptr == finale)
			return true;
		if (!Is_ExactObject(*finale, { "kind", "ghostArchetypeId", "ghostPatternIds",
			"spawnHalfExtentsM", "maximumActiveGhosts" }))
			return false;
		std::string kind, archetype;
		uint32_t maximumActive = 0u;
		if (!Read_String(*finale, "kind", false, kind) || kind != "GHOST_PORTAL_LOOP" ||
			!Read_String(*finale, "ghostArchetypeId", false, archetype) ||
			!Is_StableId(archetype) || archetype == bossArchetypeId ||
			!Read_Unsigned(*finale, "maximumActiveGhosts", 64u, maximumActive) ||
			0u == maximumActive ||
			!Is_Boolean(pattern, "invulnerableWhileRunning") ||
			pattern.Find("invulnerableWhileRunning")->Get_Boolean())
			return false;
		const DATA_JSON_VALUE* extents = finale->Find("spawnHalfExtentsM");
		const DATA_JSON_VALUE* children = finale->Find("ghostPatternIds");
		if (nullptr == extents || !extents->Is_Array() || extents->Get_Array().size() != 2u ||
			nullptr == children || !children->Is_Array() || children->Get_Array().empty() ||
			children->Get_Array().size() > 64u ||
			!std::all_of(extents->Get_Array().begin(), extents->Get_Array().end(),
				[](const DATA_JSON_VALUE& value)
				{ return value.Is_Number() && std::isfinite(value.Get_Number()) &&
					value.Get_Number() >= 1.0 && value.Get_Number() <= 100.0; }))
			return false;
		const DATA_JSON_VALUE* ownerId = pattern.Find("patternId");
		std::unordered_set<std::string> childIds;
		for (const DATA_JSON_VALUE& childId : children->Get_Array())
		{
			if (!childId.Is_String() || !Is_StableId(childId.Get_String()) ||
				!childIds.insert(childId.Get_String()).second ||
				(nullptr != ownerId && ownerId->Is_String() &&
				 ownerId->Get_String() == childId.Get_String()))
				return false;
			const auto child = std::find_if(patterns.begin(), patterns.end(),
				[&childId](const DATA_JSON_VALUE& candidate)
				{
					const DATA_JSON_VALUE* id = candidate.Find("patternId");
					return nullptr != id && id->Is_String() && id->Get_String() == childId.Get_String();
				});
			if (patterns.end() == child || nullptr != child->Find("finale") ||
				!Validate_FiniteStageGraph(*child))
				return false;
		}
		return Validate_FiniteStageGraph(pattern);
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
		if (!Read_String(*motion, "kind", false, kind))
			return false;
		if (kind == "PORTAL_TARGET_RUSH")
		{
			uint32_t stageDurationMs = 0u;
			uint32_t retargetDelayMs = 0u;
			if (!Is_ExactObject(*motion,
					{ "kind", "retargetDelayMs", "speedMps", "distanceM" }) ||
				!Read_Unsigned(stage, "durationMs",
					MAX_REFERENCE_STAGE_DURATION_MS, stageDurationMs) ||
				0u == stageDurationMs ||
				!Read_Unsigned(*motion, "retargetDelayMs", stageDurationMs,
					retargetDelayMs) ||
				!Is_FiniteNumber(*motion, "speedMps") ||
				!Is_FiniteNumber(*motion, "distanceM"))
			{
				return false;
			}
			const double speedMps = motion->Find("speedMps")->Get_Number();
			const double distanceM = motion->Find("distanceM")->Get_Number();
			if (speedMps <= 0.0 || speedMps > 1000.0 ||
				distanceM <= 0.0 || distanceM > 1000.0)
			{
				return false;
			}
			const double travelEndMs = static_cast<double>(retargetDelayMs) +
				distanceM / speedMps * 1000.0;
			outHasForwardMotion =
				travelEndMs <= static_cast<double>(stageDurationMs) + 0.000001;
			return outHasForwardMotion;
		}
		if (kind == "PORTAL_CROSS_ARENA")
		{
			uint32_t cornerIndex = 0u;
			const DATA_JSON_VALUE* extents = motion->Find("halfExtentsM");
			if (!Is_ExactObject(*motion, { "kind", "cornerIndex", "halfExtentsM" }) ||
				!Read_Unsigned(*motion, "cornerIndex", 3u, cornerIndex) ||
				nullptr == extents || !extents->Is_Array() || extents->Get_Array().size() != 2u)
				return false;
			return std::all_of(extents->Get_Array().begin(), extents->Get_Array().end(),
				[](const DATA_JSON_VALUE& value)
				{ return value.Is_Number() && std::isfinite(value.Get_Number()) &&
					value.Get_Number() >= 1.0 && value.Get_Number() <= 100.0; });
		}
		if (!Is_ExactObject(*motion, { "kind", "distance" }) || kind != "FORWARD" ||
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
		const std::string& currentPatternId,
		const bool_t isTerminalStage,
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
				uint32_t firstSpawnOffsetMs = 0u;
				uint32_t spawnIntervalMs = 0u;
				uint32_t arenaRandomCount = 0u;
				if (!Is_ExactObject(action,
						{ "trigger", "kind", "targetId", "targetingPolicy",
						  "countPerResolvedTarget", "layout", "radiusM",
						  "startAngleDegrees", "angleStepDegrees", "allowOverlap",
						  "maximumTotalObjects", "spawnCount", "firstSpawnOffsetMs",
						  "spawnIntervalMs",
						  "arenaRandomCount", "arenaRandomRadiusM",
						  "arenaHeightToleranceM", "arenaAnchorPolicy" }) ||
					!Read_String(action, "trigger", false, trigger) ||
					trigger != "ENTER" ||
					!Read_String(action, "targetId", false, targetId) ||
					!Is_StableId(targetId) ||
					!Read_String(action, "targetingPolicy", false,
						targetingPolicy) ||
					(targetingPolicy != "PER_ALIVE_PLAYER" &&
					 targetingPolicy != "BOSS_RELATIVE" &&
					 targetingPolicy != "ARENA_CENTER") ||
					!Read_String(action, "layout", false, layout) ||
					!Read_Unsigned(action, "countPerResolvedTarget", 8u,
						countPerResolvedTarget) ||
					0u == countPerResolvedTarget ||
					!Read_Unsigned(action, "maximumTotalObjects", 64u,
						maximumTotalObjects) ||
					!Read_Unsigned(action, "spawnCount", 64u, spawnCount) ||
					0u == spawnCount ||
					!Read_Unsigned(action, "firstSpawnOffsetMs",
						MAX_REFERENCE_STAGE_DURATION_MS, firstSpawnOffsetMs) ||
					firstSpawnOffsetMs >= stageDurationMs ||
					!Read_Unsigned(action, "spawnIntervalMs",
						MAX_REFERENCE_STAGE_DURATION_MS, spawnIntervalMs) ||
					(spawnCount > 1u && 0u == spawnIntervalMs) ||
					(1u == spawnCount && 0u != spawnIntervalMs) ||
					static_cast<uint64_t>(firstSpawnOffsetMs) +
						static_cast<uint64_t>(spawnCount - 1u) * spawnIntervalMs >=
						static_cast<uint64_t>(stageDurationMs) ||
					!Read_Unsigned(action, "arenaRandomCount", 32u,
						arenaRandomCount) ||
					!Is_FiniteNumber(action, "radiusM") ||
					!Is_FiniteNumber(action, "startAngleDegrees") ||
					!Is_FiniteNumber(action, "angleStepDegrees") ||
					!Is_Boolean(action, "allowOverlap") ||
					!Is_FiniteNumber(action, "arenaRandomRadiusM") ||
					action.Find("arenaRandomRadiusM")->Get_Number() < 0.0 ||
					action.Find("arenaRandomRadiusM")->Get_Number() > 1000.0 ||
					!Is_FiniteNumber(action, "arenaHeightToleranceM") ||
					action.Find("arenaHeightToleranceM")->Get_Number() < 0.0 ||
					action.Find("arenaHeightToleranceM")->Get_Number() > 1000.0 ||
					!Read_String(action, "arenaAnchorPolicy", false,
						arenaAnchorPolicy) ||
					maximumTotalObjects <
						countPerResolvedTarget + arenaRandomCount)
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
				const bool_t hasNoArenaSupplement =
					0u == arenaRandomCount &&
					0.0 == action.Find("arenaRandomRadiusM")->Get_Number() &&
					0.0 == action.Find("arenaHeightToleranceM")->Get_Number() &&
					arenaAnchorPolicy == "NONE";
				const bool_t hasArenaSupplement =
					arenaRandomCount > 0u &&
					action.Find("arenaRandomRadiusM")->Get_Number() > 0.0 &&
					action.Find("arenaHeightToleranceM")->Get_Number() > 0.0 &&
					arenaAnchorPolicy == "BOSS_SPAWN_POSITION";
				const bool_t isPerAliveArenaContract =
					targetingPolicy == "PER_ALIVE_PLAYER" &&
					(hasNoArenaSupplement || hasArenaSupplement);
				const bool_t isBossRelativeContract =
					(targetingPolicy == "BOSS_RELATIVE" ||
					 targetingPolicy == "ARENA_CENTER") &&
					countPerResolvedTarget >= 2u &&
					maximumTotalObjects >= countPerResolvedTarget &&
					1u == spawnCount && 0u == spawnIntervalMs &&
					0u == arenaRandomCount &&
					0.0 == action.Find("arenaRandomRadiusM")->Get_Number() &&
					0.0 == action.Find("arenaHeightToleranceM")->Get_Number() &&
					arenaAnchorPolicy == "NONE" && layout == "RADIAL" &&
					radius > 0.0 && angleStep > 0.0 &&
					angleStep * static_cast<double>(countPerResolvedTarget) <=
						360.000001 && !allowOverlap;
				if (radius < 0.0 ||
					(isSingle && (layout != "SINGLE" || 0.0 != radius ||
						0.0 != startAngle || 0.0 != angleStep || allowOverlap)) ||
					(!isSingle && (layout != "RADIAL" || radius <= 0.0 ||
						angleStep <= 0.0 ||
						angleStep * static_cast<double>(countPerResolvedTarget) >
							360.000001 || allowOverlap)) ||
					(!isPerAliveArenaContract && !isBossRelativeContract))
				{
					return false;
				}
				if (currentPatternId == "VALTAN_GHOST_PORTAL_ONCE")
				{
					const DATA_JSON_VALUE* stageId = stage.Find("stageId");
					const DATA_JSON_VALUE* actionId = stage.Find("actionId");
					constexpr double PORTAL_TRIANGLE_RADIUS_M = 7.5;
					if (nullptr == stageId || !stageId->Is_String() ||
						stageId->Get_String() != "ACTIVE" ||
						nullptr == actionId || !actionId->Is_String() ||
						actionId->Get_String() != "valtan.ghost.portal-once.active" ||
						actions->Get_Array().size() != 1u ||
						targetId != "combatobject.valtan.ghost.portal-charge" ||
						targetingPolicy != "BOSS_RELATIVE" ||
						3u != countPerResolvedTarget || layout != "RADIAL" ||
						std::fabs(radius - PORTAL_TRIANGLE_RADIUS_M) > 0.000001 ||
						30.0 != startAngle || 120.0 != angleStep || allowOverlap ||
						3u != maximumTotalObjects || 1u != spawnCount ||
						0u != firstSpawnOffsetMs || 0u != spawnIntervalMs ||
						0u != arenaRandomCount || !hasNoArenaSupplement)
					{
						return false;
					}
				}

				const std::string actionKey = trigger + "\n" + targetId;
				if (!actionKeys.insert(actionKey).second)
					return false;
				/* A scheduled volley is still owned by one Server stage action.
				   Its repeated spawns do not open a cross-stage lifetime here. */
				continue;
			}
			if (actionKind->Get_String() == "RELEASE_GRABBED_PLAYERS")
			{
				std::string trigger;
				std::string targetId;
				std::string releaseMode;
				uint32_t durationMs = 0u;
				if (!Is_ExactObject(action,
						{ "trigger", "kind", "targetId", "releaseMode",
						  "speedMps", "durationMs", "yawOffsetDegrees" }) ||
					!Read_String(action, "trigger", false, trigger) ||
					(trigger != "ENTER" && trigger != "EXIT") ||
					!Read_String(action, "targetId", false, targetId) ||
					targetId != "boss.attachment.left-hand" ||
					!Read_String(action, "releaseMode", false, releaseMode) ||
					!Is_FiniteNumber(action, "speedMps") ||
					!Is_FiniteNumber(action, "yawOffsetDegrees") ||
					!Read_Unsigned(action, "durationMs", 5000u, durationMs))
				{
					return false;
				}

				const double speedMps = action.Find("speedMps")->Get_Number();
				const double yawOffsetDegrees =
					action.Find("yawOffsetDegrees")->Get_Number();
				const bool_t isHold = releaseMode == "HOLD" &&
					0.0 == speedMps && 0u == durationMs &&
					0.0 == yawOffsetDegrees;
				const bool_t isKnockback =
					(releaseMode == "OPPOSITE_KNOCKBACK" || releaseMode == "ARENA_EJECTION") &&
					speedMps > 0.0 && speedMps <= 50.0 && durationMs > 0u &&
					(releaseMode == "ARENA_EJECTION" || 0.0 == yawOffsetDegrees);
				if ((!isHold && !isKnockback) || speedMps < 0.0 ||
					speedMps > 50.0 || std::abs(yawOffsetDegrees) > 180.0)
				{
					return false;
				}

				const std::string actionKey = trigger + "\n" + targetId;
				if (!actionKeys.insert(actionKey).second)
					return false;
				/* Releasing an attachment is an instantaneous Server command. */
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
				(trigger != "ENTER" && trigger != "EXIT"))
			{
				return false;
			}

			if (kind == "DAMAGE_GRABBED_PLAYERS" ||
				kind == "EXECUTE_GRABBED_PLAYERS")
			{
				const bool_t validTarget = kind == "EXECUTE_GRABBED_PLAYERS" ?
					targetId == "boss.attachment.left-hand" :
					0u == targetId.find("damage.");
				const DATA_JSON_VALUE* hitShape = stage.Find("hitShape");
				if (trigger != "ENTER" || 0u != value || 0u != durationMs ||
					!validTarget ||
					actions->Get_Array().size() != 1u || nullptr == hitShape ||
					!hitShape->Is_String() || hitShape->Get_String() != "NONE")
				{
					return false;
				}
				/* These Server-owned impacts are instantaneous, not paired
				   ENTER/EXIT state. Damage-profile membership stays with the
				   publisher and Server catalog, as for ordinary stage damage. */
				continue;
			}

			bool_t validKind = false;
			if (kind == "SET_BOSS_FLAG")
			{
				validKind = (targetId == "boss.flag.groggy" ||
					targetId == "boss.flag.invulnerable" ||
					targetId == "boss.flag.counterable") && value <= 1u &&
					0u == durationMs;
			}
			else if (kind == "SET_STAGGER_GAUGE")
				validKind = targetId == "boss.gauge.stagger" && 0u == durationMs;
			else if (kind == "SET_SHIELD")
				validKind = targetId == "boss.gauge.shield" && 0u == durationMs;
			else if (kind == "SET_PLAYER_BIND")
			{
				validKind = targetId == "player.status.bind" &&
					((trigger == "ENTER" && 5000u == value &&
					  durationMs >= 100u && durationMs <= 120000u) ||
					 (trigger == "EXIT" && 0u == value && 0u == durationMs));
			}
			else if (kind == "SET_PLAYER_SILENCE")
			{
				validKind = targetId == "player.status.silence" &&
					trigger == "ENTER" && 1u == value &&
					durationMs >= stageDurationMs &&
					durationMs >= 100u && durationMs <= 120000u;
			}
			else if (kind == "SPAWN_COMBAT_OBJECT")
				validKind = trigger == "ENTER" && 1u == value && 0u == durationMs;
			else if (kind == "SET_GAMEPLAY_PHASE")
				validKind = trigger == "ENTER" &&
					targetId == "boss.phase.gameplay" &&
					(2u == value || 3u == value) &&
					0u == durationMs;
			else if (kind == "RETARGET_RANDOM_ALIVE")
				validKind = trigger == "ENTER" &&
					targetId == "boss.target.pattern" && 1u == value &&
					0u == durationMs;
			else if (kind == "RETURN_TO_ARENA_CENTER")
				validKind = trigger == "ENTER" &&
					targetId == "boss.arena.center" && 1u == value &&
					0u == durationMs;
			else if (kind == "SUPPRESS_INTER_STEP_PURSUIT")
			{
				const DATA_JSON_VALUE* stageId = stage.Find("stageId");
				const DATA_JSON_VALUE* actionId = stage.Find("actionId");
				const DATA_JSON_VALUE* branches = stage.Find("branches");
				const bool_t hasNoBranches = nullptr == branches ||
					(branches->Is_Array() && branches->Get_Array().empty());
				validKind = trigger == "EXIT" &&
					targetId == "boss.sequence.inter-step-pursuit" &&
					0u == value && 0u == durationMs && isTerminalStage &&
					currentPatternId == "VALTAN_GHOST_DEATH_AUDITION" &&
					nullptr != stageId && stageId->Is_String() &&
					stageId->Get_String() == "STEP_01" &&
					nullptr != actionId && actionId->Is_String() &&
					actionId->Get_String() == "valtan.sequence.dead.step-01" &&
					hasNoBranches;
			}
			if (!validKind || (trigger == "ENTER" ? 0u == value : 0u != value))
				return false;

			const std::string actionKey = trigger + "\n" + targetId;
			if (!actionKeys.insert(actionKey).second)
				return false;
			if (kind == "SPAWN_COMBAT_OBJECT" ||
				kind == "SET_GAMEPLAY_PHASE" ||
				kind == "RETARGET_RANDOM_ALIVE" ||
				kind == "RETURN_TO_ARENA_CENTER" ||
				kind == "SUPPRESS_INTER_STEP_PURSUIT" ||
				/* Silence is an ENTER-only deadline-latched status. The Server
				   owns expiry, so it intentionally has no paired EXIT action. */
				kind == "SET_PLAYER_SILENCE")
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
			outcome == "SUMMON_DEAD" || outcome == "ALL_PLAYERS_GRABBED" ||
			outcome == "ANY_PLAYER_GRABBED" || outcome == "NAVIGATION_BLOCKED" ||
			outcome == "HEALTH_DAMAGE_THRESHOLD_REACHED";
	}

	bool_t Validate_StageBranches(
		const DATA_JSON_VALUE& stage,
		const std::unordered_set<std::string>& stageActionIds,
		const std::unordered_set<std::string>& patternIds,
		const std::string& currentPatternId,
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
		bool_t hasNavigationBlocked = false;
		bool_t hasHealthDamageThreshold = false;
		for (const DATA_JSON_VALUE& branch : branches->Get_Array())
		{
			std::string outcome;
			if (!Is_ExactObjectWithOptional(branch,
					{ "outcome", "nextActionId" }, { "nextPatternId" }) ||
				!Read_String(branch, "outcome", false, outcome) ||
				!Is_KnownBranchOutcome(outcome) ||
				!outcomes.insert(outcome).second)
			{
				return false;
			}
			const DATA_JSON_VALUE* nextActionId = branch.Find("nextActionId");
			const DATA_JSON_VALUE* nextPatternId = branch.Find("nextPatternId");
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
			if (nullptr != nextPatternId)
			{
				if (!nextActionId->Is_Null() || !nextPatternId->Is_String() ||
					!Is_StableId(nextPatternId->Get_String()) ||
					nextPatternId->Get_String() == currentPatternId ||
					0u == patternIds.count(nextPatternId->Get_String()))
				{
					return false;
				}
			}
			hasTimeout = hasTimeout || outcome == "TIMEOUT";
			hasWallContact = hasWallContact || outcome == "WALL_CONTACT";
			hasCounterHit = hasCounterHit || outcome == "COUNTER_HIT";
			hasStaggerBroken = hasStaggerBroken || outcome == "STAGGER_BROKEN";
			hasNavigationBlocked = hasNavigationBlocked || outcome == "NAVIGATION_BLOCKED";
			hasHealthDamageThreshold = hasHealthDamageThreshold ||
				outcome == "HEALTH_DAMAGE_THRESHOLD_REACHED";
		}
		const DATA_JSON_VALUE* response = stage.Find("playerResponse");
		const DATA_JSON_VALUE* bossResponse = stage.Find("bossResponse");
		if (nullptr == bossResponse)
		{
			if (hasHealthDamageThreshold)
				return false;
		}
		else
		{
			std::string kind;
			uint32_t threshold = 0u;
			const DATA_JSON_VALUE* stageKind = stage.Find("stageKind");
			if (!Is_ExactObject(*bossResponse, { "kind", "threshold" }) ||
				!Read_String(*bossResponse, "kind", false, kind) ||
				kind != "ACCUMULATED_HEALTH_DAMAGE" ||
				!Read_Unsigned(*bossResponse, "threshold", UINT32_MAX,
					threshold) ||
				0u == threshold || !hasHealthDamageThreshold ||
				nullptr == stageKind || !stageKind->Is_String() ||
				stageKind->Get_String() != "ACTIVE")
			{
				return false;
			}
		}
		return (!hasNavigationBlocked || (nullptr != response && response->Is_String() &&
			response->Get_String() == "CAPTURE")) &&
			hasTimeout && (!hasWallContact || hasForwardMotion) &&
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
	std::unordered_set<std::string> allPatternIds;
	for (const DATA_JSON_VALUE& entry : patterns->Get_Array())
	{
		std::string patternId;
		if (!Read_String(entry, "patternId", false, patternId) ||
			!Is_StableId(patternId) || !allPatternIds.insert(patternId).second)
		{
			outStatus = "Encounter pattern identity is invalid or duplicated";
			return false;
		}
	}

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
				{ "serverMotion", "finale", "verticalOffsetM" }))
		{
			outStatus = "Encounter pattern has unexpected properties";
			return false;
		}

		ENCOUNTER_PATTERN_REFERENCE pattern;
		std::string category;
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
			!Read_String(entry, "targetPolicy", false, pattern.targetPolicy) ||
			(pattern.targetPolicy != "NONE" &&
				pattern.targetPolicy != "NEAREST_EACH_TICK" &&
				pattern.targetPolicy != "LOCK_NEAREST_ON_START" &&
				pattern.targetPolicy != "LOCK_RANDOM_ALIVE_ON_START" &&
				pattern.targetPolicy != "LOCK_RANDOM_ALIVE_BEHIND_ON_START") ||
			!Read_String(entry, "aimPolicy", false, pattern.aimPolicy) ||
			(pattern.aimPolicy != "NONE" &&
				pattern.aimPolicy != "TRACK_TARGET_EACH_TICK" &&
				pattern.aimPolicy != "LOCK_FACING_ON_START" &&
				pattern.aimPolicy != "FACE_MOTION_ANCHOR") ||
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
			!Is_FiniteNumberArray(entry, "sourceActionIds") ||
			(nullptr != entry.Find("verticalOffsetM") &&
			 (!Is_FiniteNumber(entry, "verticalOffsetM") ||
			  0.0 == entry.Find("verticalOffsetM")->Get_Number() ||
			  std::fabs(entry.Find("verticalOffsetM")->Get_Number()) > 100.0)))
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
					{ "hitOffsetsMs", "motion", "actions", "branches",
					  "playerResponse", "attachmentSlot", "partDamagePolicy",
					  "gripLocalOffset", "counterProxy", "bossResponse",
					  "hitAnchor", "hitActivation", "verticalOffsetM" }))
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
			const DATA_JSON_VALUE* partDamagePolicy =
				stageEntry.Find("partDamagePolicy");
			if (nullptr != partDamagePolicy)
			{
				if (!partDamagePolicy->Is_String() ||
					"DESTROY_FIRST_ELIGIBLE" !=
						partDamagePolicy->Get_String())
				{
					outStatus = "Encounter part-damage policy is invalid: " +
						pattern.patternId + "/" + stage.stageId;
					return false;
				}
				stage.partDamagePolicy = partDamagePolicy->Get_String();
			}
			const DATA_JSON_VALUE* stageVerticalOffset =
				stageEntry.Find("verticalOffsetM");
			if (nullptr != stageVerticalOffset)
			{
				const DATA_JSON_VALUE* stageMotion = stageEntry.Find("motion");
				const DATA_JSON_VALUE* patternMotion = entry.Find("serverMotion");
				if (!stageVerticalOffset->Is_Number() ||
					!std::isfinite(stageVerticalOffset->Get_Number()) ||
					0.0 == stageVerticalOffset->Get_Number() ||
					std::fabs(stageVerticalOffset->Get_Number()) > 100.0 ||
					nullptr == stageEntry.Find("bossResponse") ||
					(nullptr != stageMotion && !stageMotion->Is_Null()) ||
					(nullptr != patternMotion && !patternMotion->Is_Null()))
				{
					outStatus = "Encounter stage v4 field is invalid: " +
						pattern.patternId + "/" + stage.stageId;
					return false;
				}
				stage.fVerticalOffsetM = static_cast<f32_t>(
					stageVerticalOffset->Get_Number());
			}
			const DATA_JSON_VALUE* counterProxy = stageEntry.Find("counterProxy");
			if (nullptr != counterProxy)
			{
				std::string kind;
				if (!Is_ExactObject(*counterProxy,
						{ "kind", "forwardOffsetM", "rightOffsetM", "radiusM",
						  "arcDegrees" }) ||
					!Read_String(*counterProxy, "kind", false, kind) ||
					(kind != "BOSS_LOCAL_CIRCLE" &&
					 kind != "BOSS_FORWARD_ARC") ||
					!Read_Float(*counterProxy, "forwardOffsetM",
						stage.fCounterProxyForwardOffsetM) ||
					!Read_Float(*counterProxy, "rightOffsetM",
						stage.fCounterProxyRightOffsetM) ||
					!Read_Float(*counterProxy, "radiusM",
						stage.fCounterProxyRadiusM) ||
					!Read_Float(*counterProxy, "arcDegrees",
						stage.fCounterProxyArcDegrees) ||
					std::fabs(stage.fCounterProxyForwardOffsetM) > 20.f ||
					std::fabs(stage.fCounterProxyRightOffsetM) > 20.f ||
					(kind == "BOSS_LOCAL_CIRCLE" &&
					 (stage.fCounterProxyRadiusM <= 0.f ||
					  stage.fCounterProxyRadiusM > 20.f ||
					  0.f != stage.fCounterProxyArcDegrees)) ||
					(kind == "BOSS_FORWARD_ARC" &&
					 (0.f != stage.fCounterProxyForwardOffsetM ||
					  0.f != stage.fCounterProxyRightOffsetM ||
					  0.f != stage.fCounterProxyRadiusM ||
					  180.f != stage.fCounterProxyArcDegrees)))
				{
					outStatus = "Encounter counter proxy is invalid: " +
						pattern.patternId + "/" + stage.stageId;
					return false;
				}
				stage.counterProxyKind = std::move(kind);
				stage.bHasCounterProxy = true;
			}
			const DATA_JSON_VALUE* hitAnchor = stageEntry.Find("hitAnchor");
			if (nullptr != hitAnchor)
			{
				if (!Is_ExactObject(*hitAnchor,
						{ "kind", "forwardOffsetM", "rightOffsetM",
						  "yawOffsetDegrees" }) ||
					!Read_String(*hitAnchor, "kind", false,
						stage.hitAnchorKind) ||
					("BOSS_CURRENT" != stage.hitAnchorKind &&
					 "STAGE_ORIGIN" != stage.hitAnchorKind) ||
					!Read_Float(*hitAnchor, "forwardOffsetM",
						stage.fHitAnchorForwardOffsetM) ||
					!Read_Float(*hitAnchor, "rightOffsetM",
						stage.fHitAnchorRightOffsetM) ||
					!Read_Float(*hitAnchor, "yawOffsetDegrees",
						stage.fHitAnchorYawOffsetDegrees) ||
					std::fabs(stage.fHitAnchorForwardOffsetM) > 1000.f ||
					std::fabs(stage.fHitAnchorRightOffsetM) > 1000.f ||
					std::fabs(stage.fHitAnchorYawOffsetDegrees) > 360.f)
				{
					outStatus = "Encounter stage hit anchor is invalid: " +
						pattern.patternId + "/" + stage.stageId;
					return false;
				}
				stage.bHasHitAnchor = true;
			}
			const DATA_JSON_VALUE* hitActivation =
				stageEntry.Find("hitActivation");
			if (nullptr != hitActivation)
			{
				std::string kind;
				std::string perTargetPolicy;
				if (!Is_ExactObject(*hitActivation,
						{ "kind", "startMs", "lifetimeMs",
						  "perTargetPolicy" }) ||
					!Read_String(*hitActivation, "kind", false, kind) ||
					"ACTIVE_WINDOW" != kind ||
					!Read_String(*hitActivation, "perTargetPolicy", false,
						perTargetPolicy) || "ONCE" != perTargetPolicy ||
					!Read_Unsigned(*hitActivation, "startMs",
						MAX_STAGE_DURATION_MS, stage.iHitActivationStartMs) ||
					!Read_Unsigned(*hitActivation, "lifetimeMs",
						MAX_STAGE_DURATION_MS, stage.iHitActivationLifetimeMs) ||
					0u == stage.iHitActivationLifetimeMs ||
					static_cast<uint64_t>(stage.iHitActivationStartMs) +
						stage.iHitActivationLifetimeMs > stage.iDurationMs)
				{
					outStatus = "Encounter stage hit activation is invalid: " +
						pattern.patternId + "/" + stage.stageId;
					return false;
				}
				stage.bHasHitActivation = true;
			}
			const DATA_JSON_VALUE* playerResponse =
				stageEntry.Find("playerResponse");
			const DATA_JSON_VALUE* attachmentSlot =
				stageEntry.Find("attachmentSlot");
			const DATA_JSON_VALUE* gripLocalOffset =
				stageEntry.Find("gripLocalOffset");
			if ((nullptr == playerResponse) != (nullptr == attachmentSlot) ||
				(nullptr == playerResponse) != (nullptr == gripLocalOffset))
			{
				outStatus = "Encounter capture hit fields are incomplete: " +
					pattern.patternId + "/" + stage.stageId;
				return false;
			}
			if (nullptr != playerResponse)
			{
				uint32_t pushMs = 0u;
				uint32_t downMs = 0u;
				const DATA_JSON_VALUE* knockdown = stageEntry.Find("knockdown");
				PLAYER_HAND_GRIP_LOCAL_OFFSET gripOffset;
				if (!playerResponse->Is_String() ||
					!attachmentSlot->Is_String() ||
					!Is_ExactObject(*gripLocalOffset,
						{ "forwardM", "upM", "rightM" }) ||
					!Read_Float(*gripLocalOffset, "forwardM",
						gripOffset.fForwardM) ||
					!Read_Float(*gripLocalOffset, "upM", gripOffset.fUpM) ||
					!Read_Float(*gripLocalOffset, "rightM",
						gripOffset.fRightM) ||
					!CPlayerHandGripTransform::Is_ValidGripLocalOffset(gripOffset) ||
					"CAPTURE" != playerResponse->Get_String() ||
					"BOSS_LEFT_HAND" != attachmentSlot->Get_String() ||
					"NONE" == stage.hitShape ||
					!Is_FiniteNumber(stageEntry, "pushRangeM") ||
					0.0 != stageEntry.Find("pushRangeM")->Get_Number() ||
					!Read_Unsigned(stageEntry, "pushMs",
						MAX_STAGE_DURATION_MS, pushMs) || 0u != pushMs ||
					!Read_Unsigned(stageEntry, "downMs",
						MAX_STAGE_DURATION_MS, downMs) || 0u != downMs ||
					nullptr == knockdown || !knockdown->Is_Boolean() ||
					knockdown->Get_Boolean())
				{
					outStatus = "Encounter capture hit contract is invalid: " +
						pattern.patternId + "/" + stage.stageId;
					return false;
				}
				stage.gripLocalOffset = gripOffset;
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
			const bool_t validActiveWindow = stage.bHasHitActivation &&
				validEmptyHitSchedule && "NONE" != stage.hitShape;
			if ((!stage.bHasHitActivation && !validExplicitHitSchedule &&
				 !validLegacyHitSchedule && !validEmptyHitSchedule) ||
				(stage.bHasHitActivation && !validActiveWindow) ||
				("NONE" == stage.hitShape &&
				 (stage.bHasHitAnchor || stage.bHasHitActivation)))
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
			if (!Validate_StageMotion(stageEntry, hasForwardMotion))
			{
				outStatus = "Encounter stage v4 motion is invalid: " +
					pattern.patternId + "/" + stage.stageId;
				return false;
			}
			if (!Validate_StageActions(stageEntry, pattern.patternId,
					&stageEntry == &stages->Get_Array().back(),
					activeStageActionLifetimes,
					hasCounterableEnter, hasCounterableExit,
					hasStaggerEnter, hasStaggerExit))
			{
				outStatus = "Encounter stage v4 actions are invalid: " +
					pattern.patternId + "/" + stage.stageId;
				return false;
			}
			if (!Validate_StageBranches(stageEntry, stageActionIds, allPatternIds,
					pattern.patternId, stage.actionId,
					hasForwardMotion, hasCounterableEnter, hasCounterableExit,
					hasStaggerEnter, hasStaggerExit))
			{
				outStatus = "Encounter stage v4 branches are invalid: " +
					pattern.patternId + "/" + stage.stageId;
				return false;
			}
			const DATA_JSON_VALUE* branches = stageEntry.Find("branches");
			if (nullptr != branches)
			{
				stage.bHasCounterHitBranch = std::any_of(
					branches->Get_Array().begin(), branches->Get_Array().end(),
					[](const DATA_JSON_VALUE& branch)
					{
						const DATA_JSON_VALUE* outcome = branch.Find("outcome");
						return nullptr != outcome && outcome->Is_String() &&
							"COUNTER_HIT" == outcome->Get_String();
					});
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

		const bool_t isTrash = pattern.patternId == "VALTAN_TRASH" ||
			pattern.patternId == "VALTAN_TRASH_CATCH_IF" ||
			pattern.patternId == "VALTAN_TRASH_CATCH_SUCCESS" ||
			pattern.patternId == "VALTAN_TRASH_CATCH_FAIL";
		if (!Validate_PatternServerMotion(entry) ||
			!Validate_PatternFinale(entry, patterns->Get_Array(), bossArchetypeId) ||
			(isTrash && !Validate_FiniteStageGraph(entry)))
		{
			outStatus = "Encounter pattern extensions are invalid: " + pattern.patternId;
			return false;
		}
		const DATA_JSON_VALUE* serverMotion = entry.Find("serverMotion");
		if (nullptr != serverMotion && serverMotion->Is_Object())
		{
			ENCOUNTER_PATTERN_SERVER_MOTION_REFERENCE projectedMotion;
			projectedMotion.kind =
				serverMotion->Find("kind")->Get_String();
			projectedMotion.anchorId =
				serverMotion->Find("anchorId")->Get_String();
			const DATA_JSON_VALUE::ARRAY& landing =
				serverMotion->Find("landingPosition")->Get_Array();
			for (size_t coordinate = 0u;
				coordinate < projectedMotion.landingPosition.size(); ++coordinate)
			{
				projectedMotion.landingPosition[coordinate] =
					static_cast<f32_t>(landing[coordinate].Get_Number());
			}
			const DATA_JSON_VALUE* moveToAnchor =
				serverMotion->Find("moveToAnchorBeforeTakeoff");
			projectedMotion.bMoveToAnchorBeforeTakeoff =
				nullptr != moveToAnchor && moveToAnchor->Get_Boolean();
			pattern.serverMotion = std::move(projectedMotion);
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
