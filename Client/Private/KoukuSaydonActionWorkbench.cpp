#include "imgui.h"

#include "KoukuSaydonActionWorkbench.h"
#include "CompositionTimeline.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <unordered_map>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr f32_t TIMELINE_LANE_HEIGHT = 24.f;
	constexpr f32_t TIMELINE_LABEL_WIDTH = 92.f;
	constexpr std::uint32_t MAX_EDITOR_TIME_MS = 600000u;
	constexpr std::array<const char_t*, 3u> STAGE_KINDS = {
		"WINDUP", "ACTIVE", "RECOVERY" };
	constexpr std::array<const char_t*, 2u> PATTERN_CATEGORIES = {
		"NORMAL", "MECHANIC" };
	/* All resource families route commands to the existing preview owners. */
	constexpr std::array<const char_t*, 10u> RESOURCE_CATEGORIES = {
		"Animation", "Logic", "Summon", "World", "Scene Profile", "Effect", "Collider", "Sound", "Camera", "Light" };
	constexpr ImU32 TIMELINE_LOGIC_COLOR = IM_COL32(196, 118, 64, 255);
	constexpr ImU32 TIMELINE_SUMMON_COLOR = IM_COL32(88, 156, 116, 255);
	constexpr ImU32 TIMELINE_WORLD_COLOR = IM_COL32(84, 132, 196, 255);
	constexpr ImU32 TIMELINE_SCENE_PROFILE_COLOR = IM_COL32(156, 108, 196, 255);
	constexpr const char_t* DEFAULT_BOSS_VARIANT_LABEL = "\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC - 1\xEA\xB4\x80\xEB\xAC\xB8";
	/* Pose index -> the clip the Q/W/E/R slot plays before the dance shuffle. */
	constexpr std::array<const char_t*, 4u> DANCE_POSE_LABELS = {
		"0: Q Arms folded (25_03)", "1: W Superman (25_04)",
		"2: E Arms open (25_05)", "3: R One leg (25_06)" };

	KOUKU_SAYDON_COMPOSITION_PATTERN* Find_Pattern(
		KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const std::string_view patternId)
	{
		const auto found = std::find_if(document.Patterns.begin(), document.Patterns.end(),
			[patternId](const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
			{
				return pattern.strPatternId == patternId;
			});
		return found == document.Patterns.end() ? nullptr : &*found;
	}

	const KOUKU_SAYDON_COMPOSITION_PATTERN* Find_Pattern(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const std::string_view patternId)
	{
		const auto found = std::find_if(document.Patterns.begin(), document.Patterns.end(),
			[patternId](const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
			{
				return pattern.strPatternId == patternId;
			});
		return found == document.Patterns.end() ? nullptr : &*found;
	}

	KOUKU_SAYDON_COMPOSITION_STAGE* Find_Stage(
		KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
		const std::string_view stageId)
	{
		const auto found = std::find_if(pattern.Stages.begin(), pattern.Stages.end(),
			[stageId](const KOUKU_SAYDON_COMPOSITION_STAGE& stage)
			{
				return stage.strStageId == stageId;
			});
		return found == pattern.Stages.end() ? nullptr : &*found;
	}

	const KOUKU_SAYDON_COMPOSITION_STAGE* Find_Stage(
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
		const std::string_view stageId)
	{
		const auto found = std::find_if(pattern.Stages.begin(), pattern.Stages.end(),
			[stageId](const KOUKU_SAYDON_COMPOSITION_STAGE& stage)
			{
				return stage.strStageId == stageId;
			});
		return found == pattern.Stages.end() ? nullptr : &*found;
	}

	struct MUTABLE_OCCURRENCE final
	{
		KOUKU_SAYDON_COMPOSITION_STAGE* pStage = nullptr;
		KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* pOccurrence = nullptr;
	};

	MUTABLE_OCCURRENCE Find_Occurrence(
		KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
		const std::string_view occurrenceId)
	{
		for (KOUKU_SAYDON_COMPOSITION_STAGE& stage : pattern.Stages)
		{
			const auto found = std::find_if(
				stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
				[occurrenceId](
					const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence)
				{
					return occurrence.strOccurrenceId == occurrenceId;
				});
			if (found != stage.AnimationOccurrences.end())
				return { &stage, &*found };
		}
		return {};
	}

	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* Find_Occurrence(
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
		const std::string_view occurrenceId,
		const KOUKU_SAYDON_COMPOSITION_STAGE** const ppStage = nullptr)
	{
		for (const KOUKU_SAYDON_COMPOSITION_STAGE& stage : pattern.Stages)
		{
			const auto found = std::find_if(
				stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
				[occurrenceId](
					const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence)
				{
					return occurrence.strOccurrenceId == occurrenceId;
				});
			if (found != stage.AnimationOccurrences.end())
			{
				if (nullptr != ppStage)
					*ppStage = &stage;
				return &*found;
			}
		}
		return nullptr;
	}

	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT* Find_Reference(
		const KOUKU_SAYDON_ACTION_REFERENCE_SET& references,
		const std::string_view profileId)
	{
		const auto found = std::find_if(
			references.Documents.begin(), references.Documents.end(),
			[profileId](
				const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& document)
			{
				return document.strProfileId == profileId;
			});
		return found == references.Documents.end() ? nullptr : &*found;
	}

	const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE* Find_SourceSlot(
		const KOUKU_SAYDON_ACTION_REFERENCE_SET& references,
		const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& source,
		const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT** const ppReference = nullptr)
	{
		const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT* const reference =
			Find_Reference(references, source.strProfileId);
		if (nullptr == reference)
			return nullptr;
		const auto action = std::find_if(reference->Actions.begin(), reference->Actions.end(),
			[&source](const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& value)
			{
				return value.iSourceActionId == source.iSourceActionId;
			});
		if (action == reference->Actions.end())
		{
			return nullptr;
		}
		const auto stage = std::find_if(action->Stages.begin(), action->Stages.end(),
			[&source](const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& value)
			{
				return value.strStageId == source.strSourceStageId;
			});
		if (stage == action->Stages.end())
			return nullptr;
		const auto slot = std::find_if(stage->Slots.begin(), stage->Slots.end(),
			[&source](const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& value)
			{
				return value.strSlotId == source.strSourceSlotId;
			});
		if (slot == stage->Slots.end())
			return nullptr;
		if (nullptr != ppReference)
			*ppReference = reference;
		return &*slot;
	}

	constexpr std::array<const char_t*, 3u> END_POLICIES = {
		"EXACT", "HOLD_LAST_POSE", "LOOP_TO_WINDOW" };
	constexpr std::array<const char_t*, 3u> ACTOR_PROFILES = {
		"MN_RPCZ_00", "MN_RPCT_05", "MN_RPCT_06" };

	const char_t* Actor_Label(const std::string_view actorProfileId)
	{
		if ("MN_RPCZ_00" == actorProfileId) return "Kouku";
		if ("MN_RPCT_05" == actorProfileId) return "Saydon";
		if ("MN_RPCT_06" == actorProfileId) return "Large Saydon";
		return "Unknown model";
	}

	// Create a first Pattern only inside the append candidate. Failed admission
	// must not leave an empty Pattern or consume any persistent ID ordinal.
	KOUKU_SAYDON_COMPOSITION_PATTERN* Find_AppendPattern(
		KOUKU_SAYDON_COMPOSITION_DOCUMENT& candidate, const std::string_view patternId,
		const std::string_view sourceProfileId, const std::string_view displayName,
		std::string& outStatus)
	{
		const auto actor = CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(sourceProfileId);
		if (actor.empty())
		{
			outStatus = "Append requires a known KoukuSaydon model profile.";
			return nullptr;
		}
		if (patternId.empty())
		{
			if (candidate.iNextPatternOrdinal >= 1000000u)
			{
				outStatus = "Pattern stable ID ordinals are exhausted.";
				return nullptr;
			}
			KOUKU_SAYDON_COMPOSITION_PATTERN created;
			created.strPatternId = "KAKULSAYDON_G1_PATTERN_" +
				std::to_string(candidate.iNextPatternOrdinal++);
			created.strDisplayName = std::string(displayName);
			created.strActorProfileId = std::string(actor);
			created.strAuthoringStatus = "DRAFT";
			created.strCategory = "NORMAL";
			candidate.Patterns.push_back(std::move(created));
			return &candidate.Patterns.back();
		}
		auto* pattern = Find_Pattern(candidate, patternId);
		if (nullptr == pattern || !pattern->strLoadError.empty())
		{
			outStatus = "Select an editable Pattern for this model.";
			return nullptr;
		}
		if (pattern->strActorProfileId != actor)
		{
			outStatus = "Action belongs to " + std::string(Actor_Label(actor)) +
				". Select that model's Pattern before appending.";
			return nullptr;
		}
		return pattern;
	}

	bool_t ContainsInsensitive(
		const std::string_view text,
		const std::string_view query)
	{
		if (query.empty())
			return true;
		if (query.size() > text.size())
			return false;
		for (std::size_t offset = 0u; offset + query.size() <= text.size(); ++offset)
		{
			std::size_t index = 0u;
			for (; index < query.size(); ++index)
			{
				const unsigned char left = static_cast<unsigned char>(text[offset + index]);
				const unsigned char right = static_cast<unsigned char>(query[index]);
				if (std::tolower(left) != std::tolower(right))
					break;
			}
			if (index == query.size())
				return true;
		}
		return false;
	}

	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE* Find_Action(
		const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
		const std::uint32_t sourceActionId)
	{
		const auto found = std::find_if(reference.Actions.begin(), reference.Actions.end(),
			[sourceActionId](const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& action)
			{
				return action.iSourceActionId == sourceActionId;
			});
		return found == reference.Actions.end() ? nullptr : &*found;
	}

	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE Build_ReferenceOccurrence(
		const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
		const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& action,
		const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& stage,
		const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot)
	{
		KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence;
		occurrence.strProfileId = reference.strProfileId;
		occurrence.iSourceActionId = action.iSourceActionId;
		occurrence.strSourceStageId = stage.strStageId;
		occurrence.strSourceSlotId = slot.strSlotId;
		occurrence.strReferenceRevision = reference.strReferenceRevision;
		occurrence.strRuntimeClip = slot.strRuntimeClip;
		occurrence.iSourceStartMs = slot.iSourceStartMs;
		occurrence.iPlayMs = slot.iPlayMs;
		occurrence.fPlayRate = slot.fPlayRate;
		occurrence.strEndPolicy = slot.bLoop ? "LOOP_TO_WINDOW" : "EXACT";
		return occurrence;
	}

	void Rebuild_PlayAllPatternIds(KOUKU_SAYDON_COMPOSITION_DOCUMENT& document)
	{
		document.PlayAllPatternIds.clear();
		for (const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern : document.Patterns)
		{
			if (pattern.strLoadError.empty() && "PRODUCT" == pattern.strAuthoringStatus)
				document.PlayAllPatternIds.push_back(pattern.strPatternId);
		}
	}

	void Mark_Draft(
		KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
	{
		pattern.strAuthoringStatus = "DRAFT";
		Rebuild_PlayAllPatternIds(document);
	}

	std::uint32_t Pattern_DurationMs(
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
	{
		std::uint64_t duration = 0u;
		for (const KOUKU_SAYDON_COMPOSITION_STAGE& stage : pattern.Stages)
			duration += stage.iDurationMs;
		return static_cast<std::uint32_t>((std::min)(duration,
			static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)())));
	}

	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION* Find_Logic(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const std::string_view logicId)
	{
		const auto found = std::find_if(document.Logics.begin(), document.Logics.end(),
			[logicId](const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic)
			{
				return logic.strLogicId == logicId;
			});
		return found == document.Logics.end() ? nullptr : &*found;
	}

	KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE* Find_LogicBox(
		KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
		const std::string_view occurrenceId)
	{
		const auto found = std::find_if(
			pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(),
			[occurrenceId](const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE& box)
			{
				return box.strOccurrenceId == occurrenceId;
			});
		return found == pattern.LogicOccurrences.end() ? nullptr : &*found;
	}

	const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE* Find_LogicBox(
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
		const std::string_view occurrenceId)
	{
		const auto found = std::find_if(
			pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(),
			[occurrenceId](const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE& box)
			{
				return box.strOccurrenceId == occurrenceId;
			});
		return found == pattern.LogicOccurrences.end() ? nullptr : &*found;
	}

	std::size_t Count_LogicReferences(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const std::string_view logicId,
		bool_t* const outUnresolved = nullptr)
	{
		std::size_t count = 0u;
		bool_t unresolved = false;
		for (const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern : document.Patterns)
		{
			if (pattern.strLoadError.empty())
			{
				for (const auto& box : pattern.LogicOccurrences)
				{
					if (box.strLogicId == logicId)
						++count;
					for (const KOUKU_SAYDON_OUTCOME_SLOT slot : {
						KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS, KOUKU_SAYDON_OUTCOME_SLOT::FAIL,
						KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT })
					{
						for (const std::string& target : box.Outcomes(slot))
							if (target == logicId)
								++count;
					}
				}
				continue;
			}
			// Quarantined Patterns keep their references in the preserved source.
			DATA_JSON_VALUE preserved;
			std::string error;
			if (!CDataJson::Parse(pattern.strPreservedJson, preserved, error) ||
				!preserved.Is_Object())
			{
				unresolved = true;
				continue;
			}
			const auto* boxes = preserved.Find("logicOccurrences");
			if (nullptr == boxes)
				continue;
			if (!boxes->Is_Array())
			{
				unresolved = true;
				continue;
			}
			for (const auto& box : boxes->Get_Array())
			{
				const auto* reference = box.Find("logicId");
				if (nullptr == reference || !reference->Is_String())
					unresolved = true;
				else if (reference->Get_String() == logicId)
					++count;
				// Outcome slots are optional; a present non-text slot is unreadable.
				for (const char_t* const outcomeKey : { "onSuccessLogicId", "onTimeoutLogicId" })
				{
					const auto* outcome = box.Find(outcomeKey);
					if (nullptr == outcome)
						continue;
					if (!outcome->Is_String())
						unresolved = true;
					else if (outcome->Get_String() == logicId)
						++count;
				}
				for (const char_t* const outcomeKey : {
					"onSuccessLogicIds", "onFailLogicIds", "onTimeoutLogicIds" })
				{
					const auto* outcomes = box.Find(outcomeKey);
					if (nullptr == outcomes)
						continue;
					if (!outcomes->Is_Array())
					{
						unresolved = true;
						continue;
					}
					for (const auto& outcome : outcomes->Get_Array())
					{
						if (!outcome.Is_String())
							unresolved = true;
						else if (outcome.Get_String() == logicId)
							++count;
					}
				}
			}
		}
		if (nullptr != outUnresolved)
			*outUnresolved = unresolved;
		return count;
	}

	// Latest Logic box end; the Pattern lifetime may never shrink below it.
	std::uint32_t Pattern_LogicEndMs(
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
	{
		std::uint64_t end = 0u;
		for (const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE& box : pattern.LogicOccurrences)
			end = (std::max)(end, static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs);
		return static_cast<std::uint32_t>((std::min)(end,
			static_cast<std::uint64_t>(MAX_EDITOR_TIME_MS)));
	}

	const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION* Find_Summon(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const std::string_view summonId)
	{
		const auto found = std::find_if(document.Summons.begin(), document.Summons.end(),
			[summonId](const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION& summon)
			{
				return summon.strSummonId == summonId;
			});
		return found == document.Summons.end() ? nullptr : &*found;
	}

	KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE* Find_SummonBox(
		KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
		const std::string_view occurrenceId)
	{
		const auto found = std::find_if(
			pattern.SummonOccurrences.begin(), pattern.SummonOccurrences.end(),
			[occurrenceId](const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE& box)
			{
				return box.strOccurrenceId == occurrenceId;
			});
		return found == pattern.SummonOccurrences.end() ? nullptr : &*found;
	}

	const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE* Find_SummonBox(
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
		const std::string_view occurrenceId)
	{
		const auto found = std::find_if(
			pattern.SummonOccurrences.begin(), pattern.SummonOccurrences.end(),
			[occurrenceId](const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE& box)
			{
				return box.strOccurrenceId == occurrenceId;
			});
		return found == pattern.SummonOccurrences.end() ? nullptr : &*found;
	}

	// Same reference rules as Logic: typed boxes plus quarantined Patterns' preserved source.
	std::size_t Count_SummonReferences(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const std::string_view summonId,
		bool_t* const outUnresolved = nullptr)
	{
		std::size_t count = 0u;
		bool_t unresolved = false;
		for (const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern : document.Patterns)
		{
			if (pattern.strLoadError.empty())
			{
				for (const auto& box : pattern.SummonOccurrences)
					if (box.strSummonId == summonId)
						++count;
				continue;
			}
			DATA_JSON_VALUE preserved;
			std::string error;
			if (!CDataJson::Parse(pattern.strPreservedJson, preserved, error) ||
				!preserved.Is_Object())
			{
				unresolved = true;
				continue;
			}
			const auto* boxes = preserved.Find("summonOccurrences");
			if (nullptr == boxes)
				continue;
			if (!boxes->Is_Array())
			{
				unresolved = true;
				continue;
			}
			for (const auto& box : boxes->Get_Array())
			{
				const auto* reference = box.Find("summonId");
				if (nullptr == reference || !reference->Is_String())
					unresolved = true;
				else if (reference->Get_String() == summonId)
					++count;
			}
		}
		if (nullptr != outUnresolved)
			*outUnresolved = unresolved;
		return count;
	}

	std::uint32_t Pattern_SummonEndMs(
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
	{
		std::uint64_t end = 0u;
		for (const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE& box : pattern.SummonOccurrences)
			end = (std::max)(end, static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs);
		return static_cast<std::uint32_t>((std::min)(end,
			static_cast<std::uint64_t>(MAX_EDITOR_TIME_MS)));
	}

	const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION* Find_World(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const std::string_view worldId)
	{
		const auto found = std::find_if(document.Worlds.begin(), document.Worlds.end(),
			[worldId](const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world)
			{
				return world.strWorldId == worldId;
			});
		return found == document.Worlds.end() ? nullptr : &*found;
	}

	KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE* Find_WorldBox(
		KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
		const std::string_view occurrenceId)
	{
		const auto found = std::find_if(
			pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
			[occurrenceId](const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& box)
			{
				return box.strOccurrenceId == occurrenceId;
			});
		return found == pattern.WorldOccurrences.end() ? nullptr : &*found;
	}

	const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE* Find_WorldBox(
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
		const std::string_view occurrenceId)
	{
		const auto found = std::find_if(
			pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
			[occurrenceId](const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& box)
			{
				return box.strOccurrenceId == occurrenceId;
			});
		return found == pattern.WorldOccurrences.end() ? nullptr : &*found;
	}

	const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION* Find_SceneProfile(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const std::string_view sceneProfileId)
	{
		const auto found = std::find_if(document.SceneProfiles.begin(), document.SceneProfiles.end(),
			[sceneProfileId](const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION& profile)
			{
				return profile.strSceneProfileId == sceneProfileId;
			});
		return found == document.SceneProfiles.end() ? nullptr : &*found;
	}

	KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE* Find_SceneProfileBox(
		KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
		const std::string_view occurrenceId)
	{
		const auto found = std::find_if(
			pattern.SceneProfileOccurrences.begin(), pattern.SceneProfileOccurrences.end(),
			[occurrenceId](const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE& box)
			{
				return box.strOccurrenceId == occurrenceId;
			});
		return found == pattern.SceneProfileOccurrences.end() ? nullptr : &*found;
	}

	const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE* Find_SceneProfileBox(
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
		const std::string_view occurrenceId)
	{
		const auto found = std::find_if(
			pattern.SceneProfileOccurrences.begin(), pattern.SceneProfileOccurrences.end(),
			[occurrenceId](const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE& box)
			{
				return box.strOccurrenceId == occurrenceId;
			});
		return found == pattern.SceneProfileOccurrences.end() ? nullptr : &*found;
	}

	/* Same reference rules as Summon: typed boxes plus quarantined Patterns'
	   preserved source, for one lane's box list and reference key. */
	std::size_t Count_LaneReferences(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const std::string_view definitionId,
		const char_t* const occurrencesKey,
		const char_t* const referenceKey,
		bool_t* const outUnresolved)
	{
		std::size_t count = 0u;
		bool_t unresolved = false;
		for (const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern : document.Patterns)
		{
			if (pattern.strLoadError.empty())
			{
				if (std::string_view("worldOccurrences") == occurrencesKey)
				{
					for (const auto& box : pattern.WorldOccurrences)
						if (box.strWorldId == definitionId)
							++count;
				}
				else
				{
					for (const auto& box : pattern.SceneProfileOccurrences)
						if (box.strSceneProfileId == definitionId)
							++count;
				}
				continue;
			}
			DATA_JSON_VALUE preserved;
			std::string error;
			if (!CDataJson::Parse(pattern.strPreservedJson, preserved, error) ||
				!preserved.Is_Object())
			{
				unresolved = true;
				continue;
			}
			const auto* boxes = preserved.Find(occurrencesKey);
			if (nullptr == boxes)
				continue;
			if (!boxes->Is_Array())
			{
				unresolved = true;
				continue;
			}
			for (const auto& box : boxes->Get_Array())
			{
				const auto* reference = box.Find(referenceKey);
				if (nullptr == reference || !reference->Is_String())
					unresolved = true;
				else if (reference->Get_String() == definitionId)
					++count;
			}
		}
		if (nullptr != outUnresolved)
			*outUnresolved = unresolved;
		return count;
	}

	std::size_t Count_WorldReferences(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const std::string_view worldId,
		bool_t* const outUnresolved = nullptr)
	{
		return Count_LaneReferences(document, worldId, "worldOccurrences", "worldId", outUnresolved);
	}

	std::size_t Count_SceneProfileReferences(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const std::string_view sceneProfileId,
		bool_t* const outUnresolved = nullptr)
	{
		return Count_LaneReferences(document, sceneProfileId, "sceneProfileOccurrences",
			"sceneProfileId", outUnresolved);
	}

	const char* Presentation_Label(const KOUKU_SAYDON_PRESENTATION_KIND kind)
	{
		switch (kind)
		{
		case KOUKU_SAYDON_PRESENTATION_KIND::EFFECT: return "Effect";
		case KOUKU_SAYDON_PRESENTATION_KIND::SOUND: return "Sound";
		case KOUKU_SAYDON_PRESENTATION_KIND::CAMERA: return "Camera";
		case KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER: return "Collider";
		case KOUKU_SAYDON_PRESENTATION_KIND::LIGHT: return "Light";
		case KOUKU_SAYDON_PRESENTATION_KIND::WORLD: return "World";
		case KOUKU_SAYDON_PRESENTATION_KIND::SCENE_PROFILE: return "Scene Profile";
		default: return "Unavailable";
		}
	}

	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE* Find_PresentationResource(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, const std::string_view id)
	{
		const auto found = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
			[id](const auto& row) { return row.strResourceId == id; });
		return found == document.PresentationResources.end() ? nullptr : &*found;
	}

	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE* Find_PresentationBox(
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const std::string_view id)
	{
		const auto found = std::find_if(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
			[id](const auto& row) { return row.strOccurrenceId == id; });
		return found == pattern.PresentationOccurrences.end() ? nullptr : &*found;
	}

	/* A World box only needs its start inside the Pattern; a Scene Profile box
	   needs its whole window inside it. */
	std::uint32_t Pattern_LaneEndMs(
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
	{
		std::uint64_t end = 0u;
		for (const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& box : pattern.WorldOccurrences)
			end = (std::max)(end, static_cast<std::uint64_t>(box.iStartMs) + 1u);
		for (const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE& box : pattern.SceneProfileOccurrences)
			end = (std::max)(end, static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs);
		for (const auto& box : pattern.PresentationOccurrences)
			end = (std::max)(end, static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs);
		return static_cast<std::uint32_t>((std::min)(end,
			static_cast<std::uint64_t>(MAX_EDITOR_TIME_MS)));
	}

	const char_t* Outcome_SlotLabel(const KOUKU_SAYDON_OUTCOME_SLOT slot)
	{
		return KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS == slot ? "Success" :
			(KOUKU_SAYDON_OUTCOME_SLOT::FAIL == slot ? "Fail" : "Timeout");
	}

	bool_t Copy_Text(char_t* const destination, const std::size_t capacity,
		const std::string_view source)
	{
		if (nullptr == destination || 0u == capacity || source.size() >= capacity)
			return false;
		memcpy(destination, source.data(), source.size());
		destination[source.size()] = '\0';
		return true;
	}

	bool_t Append_WorldCompanionEffect(KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
		const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world,
		const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& owner, std::string& status)
	{
		if (world.strCompanionEffectResourceId.empty()) return true;
		if (pattern.iNextPresentationOccurrenceOrdinal >= 1000000u ||
			static_cast<std::uint64_t>(owner.iStartMs) + owner.iDurationMs > Pattern_DurationMs(pattern))
		{ status = "World companion needs an available Effect box ID and a window inside the Pattern lifetime."; return false; }
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE effect;
		effect.strOccurrenceId = pattern.strPatternId + ".presentation." + std::to_string(pattern.iNextPresentationOccurrenceOrdinal++);
		effect.strResourceId = world.strCompanionEffectResourceId;
		effect.strWorldOccurrenceId = owner.strOccurrenceId;
		effect.iStartMs = owner.iStartMs; effect.iDurationMs = owner.iDurationMs;
		pattern.PresentationOccurrences.push_back(std::move(effect));
		return true;
	}

	std::string Read_PublishDiagnosticTail(const std::filesystem::path& path)
	{
		constexpr std::streamoff MAX_TAIL_BYTES = 4096;
		std::ifstream input(path, std::ios::binary);
		if (!input)
			return {};
		input.seekg(0, std::ios::end);
		const std::streamoff size = input.tellg();
		if (size > MAX_TAIL_BYTES)
			input.seekg(size - MAX_TAIL_BYTES, std::ios::beg);
		else
			input.seekg(0, std::ios::beg);
		return {
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
	}
}

void Client::CKoukuSaydonActionWorkbench::Open()
{
	m_bOpen = true;
	m_bResourcesOpen = true;
	if (m_ModelResources.empty()) m_bResourceRefreshRequested = true;
}

Client::CKoukuSaydonActionWorkbench::~CKoukuSaydonActionWorkbench()
{
	/* Closing our observation handle never terminates the transactional child.
	   It may still complete or roll back under the build-domain owner lock. */
	if (nullptr != m_hPublishProcess)
		CloseHandle(static_cast<HANDLE>(m_hPublishProcess));
}

bool_t Client::CKoukuSaydonActionWorkbench::Reload(std::string& outStatus)
{
	if (m_bDirty)
	{
		outStatus = "KoukuSaydon composition Reload requires an explicit draft discard.";
		m_strStatus = outStatus;
		return false;
	}
	if (!m_Document.Reload(outStatus))
	{
		KOUKU_SAYDON_ACTION_REFERENCE_SET references;
		std::string referenceStatus;
		if (CKoukuSaydonCompositionDocument::Load_ImmutableActionReferences(
				references, referenceStatus))
		{
			m_ResourceReferences = std::move(references);
		}
		m_bResourceTreeDirty = true;
		m_strStatus = outStatus;
		return false;
	}

	const std::string previousPatternId = m_strSelectedPatternId;
	m_Draft = m_Document.Get_LastGood();
	m_ResourceReferences = m_Document.Get_References();
	m_bResourceTreeDirty = true;
	m_bHasDraft = true;
	m_bDirty = false;
	++m_iDraftGeneration;
	m_strSelectedPatternId = previousPatternId;
	if (nullptr == Find_Pattern(m_Draft, m_strSelectedPatternId))
	{
		m_strSelectedPatternId = m_Draft.Patterns.empty() ?
			std::string{} : m_Draft.Patterns.front().strPatternId;
		m_strSelectedStageId.clear();
		m_strSelectedOccurrenceId.clear();
	}
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const selectedPattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (nullptr != selectedPattern && m_strSelectedStageId.empty() &&
		!selectedPattern->Stages.empty())
	{
		m_strSelectedStageId = selectedPattern->Stages.front().strStageId;
	}
	Normalize_Selection();
	Synchronize_EditorFields();
	m_strStatus = outStatus;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Save(std::string& outStatus)
{
	if (!m_bHasDraft)
	{
		outStatus = "No KoukuSaydon composition draft is loaded.";
		m_strStatus = outStatus;
		return false;
	}
	if (!m_bDirty)
	{
		outStatus = "KoukuSaydon composition has no unsaved changes.";
		m_strStatus = outStatus;
		return true;
	}
	if (!m_Document.Save_Atomic(m_Draft, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	m_Draft = m_Document.Get_LastGood();
	m_ResourceReferences = m_Document.Get_References();
	m_bResourceTreeDirty = true;
	m_bDirty = false;
	++m_iDraftGeneration;
	Normalize_Selection();
	Synchronize_EditorFields();
	m_strStatus = outStatus;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Publish_Product(
	std::string& outStatus)
{
	if (!m_bHasDraft || m_bDirty || !m_Document.Is_Fresh())
	{
		outStatus =
			"Publish requires one clean, freshly reopened KoukuSaydon Save.";
		m_strStatus = outStatus;
		return false;
	}
	if (m_Draft.PlayAllPatternIds.empty())
	{
		outStatus =
			"Publish requires at least one validated PRODUCT Pattern.";
		m_strStatus = outStatus;
		return false;
	}
	if (nullptr != m_hPublishProcess)
	{
		outStatus =
			"A KoukuSaydon Product publisher is already running or still observed.";
		m_strStatus = outStatus;
		return false;
	}

	std::error_code error;
	const std::filesystem::path projectRoot = std::filesystem::weakly_canonical(
		CProjectDataRoot::Get().parent_path(), error);
	const std::filesystem::path script = projectRoot / L"Tools" / L"Build" /
		L"Invoke-BuildDomainOwner.ps1";
	const std::filesystem::path diagnosticDirectory =
		projectRoot / L"out" / L"KoukuSaydon";
	if (error || projectRoot.empty() ||
		!std::filesystem::is_regular_file(script, error) || error)
	{
		outStatus =
			"The canonical KoukuSaydon Product publisher could not be resolved.";
		m_strStatus = outStatus;
		return false;
	}
	std::filesystem::create_directories(diagnosticDirectory, error);
	if (error)
	{
		outStatus =
			"The KoukuSaydon publish diagnostic directory could not be created.";
		m_strStatus = outStatus;
		return false;
	}
	m_PublishDiagnosticPath = diagnosticDirectory /
		(L"KoukuSaydonComposition." +
		 std::to_wstring(GetCurrentProcessId()) + L"." +
		 std::to_wstring(GetTickCount64()) + L".publish.log");

	SECURITY_ATTRIBUTES security{};
	security.nLength = sizeof(security);
	security.bInheritHandle = TRUE;
	const HANDLE output = CreateFileW(
		m_PublishDiagnosticPath.c_str(), GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_DELETE, &security, CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == output)
	{
		outStatus = "Could not create the preserved KoukuSaydon publish log.";
		m_strStatus = outStatus;
		return false;
	}
	const HANDLE input = CreateFileW(
		L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		&security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == input)
	{
		CloseHandle(output);
		outStatus = "Could not create the KoukuSaydon publisher input handle.";
		m_strStatus = outStatus;
		return false;
	}

	const std::wstring command =
		L"\"powershell.exe\" -NoProfile -ExecutionPolicy Bypass -File \"" +
		script.wstring() + L"\" -Owner KoukuSaydon " +
		L"-ExpectedKoukuSaydonSourceRevision " +
		std::to_wstring(m_Draft.iRevision);
	std::vector<wchar_t> mutableCommand(command.begin(), command.end());
	mutableCommand.push_back(L'\0');
	STARTUPINFOW startup{};
	startup.cb = sizeof(startup);
	startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startup.wShowWindow = SW_HIDE;
	startup.hStdInput = input;
	startup.hStdOutput = output;
	startup.hStdError = output;
	PROCESS_INFORMATION process{};
	const BOOL created = CreateProcessW(
		L"powershell.exe", mutableCommand.data(), nullptr, nullptr, TRUE,
		CREATE_NO_WINDOW, nullptr, projectRoot.c_str(), &startup, &process);
	const DWORD createError = created ? ERROR_SUCCESS : GetLastError();
	CloseHandle(input);
	CloseHandle(output);
	if (!created)
	{
		outStatus = "Could not start the KoukuSaydon Product publisher (Win32 " +
			std::to_string(createError) + ").";
		m_strStatus = outStatus;
		return false;
	}
	CloseHandle(process.hThread);
	m_hPublishProcess = process.hProcess;
	m_iPublishStartedAtMilliseconds = GetTickCount64();
	outStatus =
		"Publishing saved KoukuSaydon Product. Authoring source writes are locked until completion.";
	m_strStatus = outStatus;
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Poll_PublishProcess()
{
	if (nullptr == m_hPublishProcess)
		return;
	const HANDLE process = static_cast<HANDLE>(m_hPublishProcess);
	const DWORD wait = WaitForSingleObject(process, 0u);
	if (WAIT_TIMEOUT == wait)
	{
		if (GetTickCount64() - m_iPublishStartedAtMilliseconds >= 60000u &&
			m_strStatus.find("still running") == std::string::npos)
		{
			m_strStatus =
				"KoukuSaydon Product publish is still running. It will not be terminated during an atomic domain transaction.";
		}
		return;
	}
	if (WAIT_OBJECT_0 != wait)
	{
		m_strStatus =
			"KoukuSaydon publisher observation failed; no second publisher will start while this handle remains live.";
		return;
	}

	DWORD exitCode = 1u;
	const bool_t exitKnown = FALSE != GetExitCodeProcess(process, &exitCode);
	CloseHandle(process);
	m_hPublishProcess = nullptr;
	const std::string diagnostic =
		Read_PublishDiagnosticTail(m_PublishDiagnosticPath);
	if (exitKnown && 0u == exitCode)
	{
		m_strStatus =
			"Published all KoukuSaydon PRODUCT patterns and runtime data. Restart the Debug Server, then use Play Published Product (Server).";
		return;
	}
	m_strStatus = "KoukuSaydon Product publish failed";
	if (exitKnown)
		m_strStatus += " with exit code " + std::to_string(exitCode);
	m_strStatus += ". Preserved log: " + m_PublishDiagnosticPath.string();
	if (!diagnostic.empty())
		m_strStatus += "\n" + diagnostic;
}

bool_t Client::CKoukuSaydonActionWorkbench::Validate_Draft(
	std::string& outStatus) const
{
	if (!m_bHasDraft)
	{
		outStatus = "No KoukuSaydon composition draft is loaded.";
		return false;
	}
	return CKoukuSaydonCompositionDocument::Validate(
		m_Draft, m_ResourceReferences, outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Select_ActorProfile(
	const std::string_view actorProfileId, std::string& outStatus)
{
	const auto resolved = CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(actorProfileId);
	if (resolved.empty() || resolved != actorProfileId)
	{
		outStatus = "Select Kouku, Saydon or Large Saydon.";
		return false;
	}
	m_strSelectedActorProfileId = std::string(resolved);
	const auto* current = Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (nullptr != current && current->strActorProfileId == resolved)
		return true;
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	Clear_TimelineSelection();
	m_strSelectedPatternId.clear();
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	for (const auto& pattern : m_Draft.Patterns)
	{
		if (pattern.strActorProfileId == resolved && pattern.strLoadError.empty())
			return Select_PatternById(pattern.strPatternId, outStatus);
	}
	m_bFitRequested = true;
	Synchronize_EditorFields();
	outStatus = "No Pattern for " + std::string(Actor_Label(resolved)) +
		" yet. Append Action creates its first Pattern, or use Create Pattern.";
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Select_WorkbenchBoss(
	const COMPOSITION_WORKBENCH_BOSS boss)
{
	/* One Gate 1 composition serves every gate entry today; the entry only
	   narrows the model filter and names the gate above the pattern list. */
	const char_t* actor = "MN_RPCT_05";
	const char_t* label = "\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC - 1\xEA\xB4\x80\xEB\xAC\xB8";
	switch (boss)
	{
	case COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_GATE2:
		actor = "MN_RPCT_06";
		label = "\xEB\x8C\x80\xED\x98\x95 \xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC, \xEC\xBF\xA0\xED\x81\xAC - 2\xEA\xB4\x80\xEB\xAC\xB8";
		break;
	case COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_GATE3:
		label = "\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC - 3\xEA\xB4\x80\xEB\xAC\xB8";
		break;
	case COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_ENCORE:
		label = "\xEC\x95\xB5\xEC\xBD\x9C \xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC - \xEB\xB9\x99\xEA\xB3\xA0";
		break;
	default:
		break;
	}
	m_strBossVariantLabel = label;
	std::string status;
	(void)Select_ActorProfile(actor, status);
}

bool_t Client::CKoukuSaydonActionWorkbench::Select_PatternById(
	const std::string_view patternId,
	std::string& outStatus)
{
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern =
		Find_Pattern(m_Draft, patternId);
	if (nullptr == pattern)
	{
		outStatus = "KoukuSaydon Pattern selection is not in the authoring document.";
		return false;
	}
	Clear_TimelineSelection();
	m_strSelectedActorProfileId = pattern->strActorProfileId;
	m_bFitRequested = true;
	m_strSelectedPatternId = std::string(patternId);
	m_strSelectedStageId = pattern->Stages.empty() ?
		std::string{} : pattern->Stages.front().strStageId;
	m_strSelectedOccurrenceId.clear();
	Normalize_Selection();
	Synchronize_EditorFields();
	outStatus = "Selected KoukuSaydon Pattern " + m_strSelectedPatternId + ".";
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Consume_PresentationPreviewRequest(
	KOUKU_PRESENTATION_PREVIEW_REQUEST& outRequest)
{
	if (!m_bPresentationPreviewRequestPending) return false;
	outRequest = std::move(m_PendingPresentationPreviewRequest);
	m_PendingPresentationPreviewRequest = {};
	m_bPresentationPreviewRequestPending = false;
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Set_PresentationResources(
	std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> resources,
	std::string status)
{
	m_PresentationResourceInventory = std::move(resources);
	m_strPresentationResourceStatus = std::move(status);
}

bool_t Client::CKoukuSaydonActionWorkbench::Consume_PresentationResourceRefreshRequest()
{
	const bool_t requested = m_bPresentationResourceRefreshRequested;
	m_bPresentationResourceRefreshRequested = false;
	return requested;
}

bool_t Client::CKoukuSaydonActionWorkbench::Consume_AnimationPreviewRequest(
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& outRequest)
{
	if (!m_bPreviewRequestPending)
		return false;
	outRequest = std::move(m_PendingPreviewRequest);
	m_PendingPreviewRequest = {};
	m_bPreviewRequestPending = false;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Consume_PatternPreviewRequest(
	KOUKU_SAYDON_COMPOSITION_PATTERN& outPattern,
	std::uint32_t& outStartClockMs,
	bool_t& outStartPaused,
	std::string& outTargetAssetName)
{
	if (!m_bPatternPreviewRequestPending)
		return false;
	outPattern = std::move(m_PendingPatternPreview);
	outStartClockMs = m_iPendingPreviewStartMs;
	outStartPaused = m_bPendingPreviewStartPaused;
	outTargetAssetName = std::move(m_strPendingPreviewTargetAsset);
	m_strPendingPreviewTargetAsset.clear();
	m_PendingPatternPreview = {};
	m_bPatternPreviewRequestPending = false;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Consume_ServerPlayRequest(
	std::string& outPatternId,
	std::uint32_t& outSourceRevision)
{
	if (!m_bServerPlayRequestPending)
		return false;
	outPatternId = std::move(m_strPendingServerPlayPatternId);
	outSourceRevision = m_iPendingServerPlaySourceRevision;
	m_strPendingServerPlayPatternId.clear();
	m_iPendingServerPlaySourceRevision = 0u;
	m_bServerPlayRequestPending = false;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Commit_Candidate(
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate,
	const std::string_view successStatus,
	std::string& outStatus)
{
	for (const auto& previous : m_Draft.Patterns)
	{
		if (previous.strLoadError.empty()) continue;
		const auto* current = Find_Pattern(candidate, previous.strPatternId);
		if (nullptr != current && *current != previous)
		{
			outStatus = m_strStatus = "Repair and reload the invalid Pattern, or delete it before editing.";
			return false;
		}
	}
	std::string validationStatus;
	if (!m_bHasDraft ||
		!CKoukuSaydonCompositionDocument::Validate(
			candidate, m_ResourceReferences, validationStatus))
	{
		outStatus = "KoukuSaydon edit rejected; draft preserved: " + validationStatus;
		m_strStatus = outStatus;
		return false;
	}
	m_Draft = std::move(candidate);
	m_bDirty = true;
	++m_iDraftGeneration;
	Normalize_Selection();
	Synchronize_EditorFields();
	outStatus = std::string(successStatus);
	m_strStatus = outStatus;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Create_Pattern(
	const std::string_view displayName,
	const std::string_view category,
	std::string& outPatternId,
	std::string& outStatus)
{
	if (!m_bHasDraft || m_Draft.iNextPatternOrdinal >= 1000000u)
	{
		outStatus = "KoukuSaydon Pattern creation requires a loaded draft and available ordinal.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
	pattern.strPatternId = "KAKULSAYDON_G1_PATTERN_" +
		std::to_string(candidate.iNextPatternOrdinal++);
	pattern.strDisplayName = std::string(displayName);
	pattern.strActorProfileId = m_strSelectedActorProfileId;
	pattern.strAuthoringStatus = "DRAFT";
	pattern.strCategory = std::string(category);
	pattern.iNextStageOrdinal = 1u;
	pattern.iNextAnimationOrdinal = 1u;
	const std::string patternId = pattern.strPatternId;
	candidate.Patterns.push_back(std::move(pattern));
	Rebuild_PlayAllPatternIds(candidate);
	if (!Commit_Candidate(std::move(candidate),
			"Created KoukuSaydon draft Pattern " + patternId + ".", outStatus))
	{
		return false;
	}
	Clear_TimelineSelection();
	m_bFitRequested = true;
	m_strSelectedPatternId = patternId;
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	Normalize_Selection();
	Synchronize_EditorFields();
	outPatternId = patternId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_Pattern(
	const std::string_view patternId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	const auto found = std::find_if(candidate.Patterns.begin(), candidate.Patterns.end(),
		[patternId](const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
		{
			return pattern.strPatternId == patternId;
		});
	if (found == candidate.Patterns.end())
	{
		outStatus = "KoukuSaydon Pattern delete target is absent.";
		return false;
	}
	candidate.Patterns.erase(found);
	Rebuild_PlayAllPatternIds(candidate);
	if (!Commit_Candidate(std::move(candidate), "Deleted KoukuSaydon Pattern.", outStatus))
		return false;
	if (m_strSelectedPatternId == patternId)
	{
		m_strSelectedPatternId.clear();
		m_strSelectedStageId.clear();
		m_strSelectedOccurrenceId.clear();
		Synchronize_EditorFields();
	}
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Rename_Pattern(
	const std::string_view patternId,
	const std::string_view displayName,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = "KoukuSaydon Pattern rename target is absent.";
		return false;
	}
	pattern->strDisplayName = std::string(displayName);
	return Commit_Candidate(std::move(candidate),
		"Renamed KoukuSaydon Pattern display name.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_PatternCategory(
	const std::string_view patternId,
	const std::string_view category,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern =
		Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = "KoukuSaydon Pattern category target is absent.";
		return false;
	}
	if (pattern->strCategory == category)
	{
		outStatus = "KoukuSaydon Pattern category is unchanged.";
		return true;
	}
	pattern->strCategory = std::string(category);
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate),
		"Changed KoukuSaydon Pattern category and returned it to DRAFT.",
		outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_PatternAuthoringStatus(
	const std::string_view patternId,
	const std::string_view authoringStatus,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = "KoukuSaydon Pattern status target is absent.";
		return false;
	}
	pattern->strAuthoringStatus = std::string(authoringStatus);
	Rebuild_PlayAllPatternIds(candidate);
	return Commit_Candidate(std::move(candidate),
		"Changed KoukuSaydon Pattern authoring status.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Add_Stage(
	const std::string_view patternId,
	const std::string_view stageKind,
	const std::uint32_t durationMs,
	std::string& outStageId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || pattern->iNextStageOrdinal >= 1000000u)
	{
		outStatus = "KoukuSaydon Stage creation target or ordinal is invalid.";
		return false;
	}
	const std::uint32_t ordinal = pattern->iNextStageOrdinal++;
	KOUKU_SAYDON_COMPOSITION_STAGE stage;
	stage.strStageId = "STAGE_" + std::to_string(ordinal);
	stage.strActionId = pattern->strPatternId + ".stage." + std::to_string(ordinal);
	stage.strStageKind = std::string(stageKind);
	stage.iDurationMs = durationMs;
	const std::string stageId = stage.strStageId;
	pattern->Stages.push_back(std::move(stage));
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Added KoukuSaydon Stage " + stageId + ".", outStatus))
	{
		return false;
	}
	m_strSelectedPatternId = std::string(patternId);
	m_strSelectedStageId = stageId;
	m_strSelectedOccurrenceId.clear();
	Synchronize_EditorFields();
	outStageId = stageId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_Stage(
	const std::string_view patternId,
	const std::string_view stageId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = "KoukuSaydon Stage delete Pattern is absent.";
		return false;
	}
	const auto found = std::find_if(pattern->Stages.begin(), pattern->Stages.end(),
		[stageId](const KOUKU_SAYDON_COMPOSITION_STAGE& stage)
		{
			return stage.strStageId == stageId;
		});
	if (found == pattern->Stages.end())
	{
		outStatus = "KoukuSaydon Stage delete target is absent.";
		return false;
	}
	pattern->Stages.erase(found);
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Deleted KoukuSaydon Stage.", outStatus))
		return false;
	if (m_strSelectedStageId == stageId)
	{
		m_strSelectedStageId.clear();
		m_strSelectedOccurrenceId.clear();
		Synchronize_EditorFields();
	}
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Append_AnimationAsStage(
	const std::string_view patternId,
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& source,
	std::string& outStageId,
	std::string& outOccurrenceId,
	std::string& outStatus)
{
	if (!Is_AppendAdmitted(source, outStatus))
		return false;

	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern =
		Find_AppendPattern(candidate, patternId, source.strProfileId, source.strRuntimeClip, outStatus);
	if (nullptr == pattern || pattern->iNextStageOrdinal >= 1000000u ||
		pattern->iNextAnimationOrdinal >= 1000000u)
	{
		outStatus =
			"Append target Pattern or its stable ID ordinal is unavailable.";
		return false;
	}

	const std::string targetPatternId = pattern->strPatternId;
	const std::uint32_t stageOrdinal = pattern->iNextStageOrdinal++;
	const std::uint32_t animationOrdinal = pattern->iNextAnimationOrdinal++;
	KOUKU_SAYDON_COMPOSITION_STAGE stage;
	stage.strStageId = "STAGE_" + std::to_string(stageOrdinal);
	stage.strActionId = pattern->strPatternId + ".stage." +
		std::to_string(stageOrdinal);
	stage.strStageKind = "ACTIVE";
	stage.iDurationMs = source.iPlayMs;

	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence = source;
	occurrence.strOccurrenceId = pattern->strPatternId + ".animation." +
		std::to_string(animationOrdinal);
	occurrence.iStartOffsetMs = 0u;
	std::string policyNote;
	(void)Normalize_EndPolicyForWindow(occurrence, policyNote);

	const std::string stageId = stage.strStageId;
	const std::string occurrenceId = occurrence.strOccurrenceId;
	stage.AnimationOccurrences.push_back(std::move(occurrence));
	pattern->Stages.push_back(std::move(stage));
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Appended boss animation as Stage " + stageId + "." + policyNote, outStatus))
	{
		return false;
	}

	Clear_TimelineSelection();
	m_bFitRequested = true;
	m_strSelectedActorProfileId = std::string(CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(source.strProfileId));
	m_strSelectedPatternId = targetPatternId;
	Select_TimelineBox(stageId, occurrenceId, false);
	outStageId = stageId;
	outOccurrenceId = occurrenceId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Move_Stage(
	const std::string_view patternId,
	const std::string_view stageId,
	const int32_t direction,
	std::string& outStatus)
{
	if (0 == direction)
	{
		outStatus = "KoukuSaydon Stage move direction is zero.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = "KoukuSaydon Stage move Pattern is absent.";
		return false;
	}
	const auto found = std::find_if(pattern->Stages.begin(), pattern->Stages.end(),
		[stageId](const KOUKU_SAYDON_COMPOSITION_STAGE& stage)
		{
			return stage.strStageId == stageId;
		});
	if (found == pattern->Stages.end())
	{
		outStatus = "KoukuSaydon Stage move target is absent.";
		return false;
	}
	const std::ptrdiff_t index = std::distance(pattern->Stages.begin(), found);
	const std::ptrdiff_t target = index + (direction < 0 ? -1 : 1);
	if (target < 0 || target >= static_cast<std::ptrdiff_t>(pattern->Stages.size()))
	{
		outStatus = "KoukuSaydon Stage is already at that edge.";
		return false;
	}
	std::iter_swap(pattern->Stages.begin() + index, pattern->Stages.begin() + target);
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Reordered KoukuSaydon Stage.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Move_SelectedStage(
	const std::string_view patternId,
	const int32_t direction,
	std::string& outStatus)
{
	std::string stageId;
	std::string occurrenceId;
	if (1u == m_TimelineSelectedStageIds.size() && m_TimelineSelectedOccurrenceIds.empty())
	{
		stageId = m_TimelineSelectedStageIds.front();
	}
	else if (m_TimelineSelectedStageIds.empty() && 1u == m_TimelineSelectedOccurrenceIds.size())
	{
		occurrenceId = m_TimelineSelectedOccurrenceIds.front();
		const KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(m_Draft, patternId);
		if (nullptr != pattern)
		{
			for (const KOUKU_SAYDON_COMPOSITION_STAGE& stage : pattern->Stages)
			{
				for (const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence :
					stage.AnimationOccurrences)
				{
					if (occurrence.strOccurrenceId == occurrenceId)
						stageId = stage.strStageId;
				}
			}
		}
	}
	if (stageId.empty())
	{
		outStatus = m_strStatus =
			"Select exactly one Stage or one animation box to move it with Left/Right.";
		return false;
	}
	if (!Move_Stage(patternId, stageId, direction, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	// The commit rebuilt the draft; reselect the same stable IDs so a second
	// arrow press keeps moving the same Stage.
	Select_TimelineBox(stageId, occurrenceId, false);
	m_strStatus = direction < 0 ?
		"Moved the Stage one slot earlier (its animation boxes moved with it)." :
		"Moved the Stage one slot later (its animation boxes moved with it).";
	outStatus = m_strStatus;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_StageDuration(
	const std::string_view patternId,
	const std::string_view stageId,
	const std::uint32_t durationMs,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_STAGE* const stage = nullptr == pattern ?
		nullptr : Find_Stage(*pattern, stageId);
	if (nullptr == stage)
	{
		outStatus = "KoukuSaydon Stage duration target is absent.";
		return false;
	}
	stage->iDurationMs = durationMs;
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Changed KoukuSaydon Stage duration.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_StageKind(
	const std::string_view patternId,
	const std::string_view stageId,
	const std::string_view stageKind,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_STAGE* const stage = nullptr == pattern ?
		nullptr : Find_Stage(*pattern, stageId);
	if (nullptr == stage)
	{
		outStatus = "KoukuSaydon Stage kind target is absent.";
		return false;
	}
	stage->strStageKind = std::string(stageKind);
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Changed KoukuSaydon Stage kind.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Bind_Animation(
	const std::string_view patternId,
	const std::string_view targetStageId,
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& source,
	const std::uint32_t startOffsetMs,
	std::string& outOccurrenceId,
	std::string& outStatus)
{
	if (!Is_AppendAdmitted(source, outStatus))
		return false;

	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_STAGE* const stage = nullptr == pattern ?
		nullptr : Find_Stage(*pattern, targetStageId);
	if (nullptr == stage || pattern->iNextAnimationOrdinal >= 1000000u)
	{
		outStatus = "KoukuSaydon animation Bind target or ordinal is invalid.";
		return false;
	}
	const std::string occurrenceId = pattern->strPatternId + ".animation." +
		std::to_string(pattern->iNextAnimationOrdinal++);
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence = source;
	occurrence.strOccurrenceId = occurrenceId;
	occurrence.iStartOffsetMs = startOffsetMs;
	std::string policyNote;
	(void)Normalize_EndPolicyForWindow(occurrence, policyNote);
	stage->iDurationMs = (std::max)(stage->iDurationMs, startOffsetMs + occurrence.iPlayMs);
	stage->AnimationOccurrences.push_back(std::move(occurrence));
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Bound KoukuSaydon animation " + occurrenceId + "." + policyNote, outStatus))
	{
		return false;
	}
	m_strSelectedPatternId = std::string(patternId);
	m_strSelectedStageId = std::string(targetStageId);
	m_strSelectedOccurrenceId = occurrenceId;
	Synchronize_EditorFields();
	outOccurrenceId = occurrenceId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Move_Animation(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	const std::uint32_t startOffsetMs,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	const MUTABLE_OCCURRENCE found = nullptr == pattern ? MUTABLE_OCCURRENCE{} :
		Find_Occurrence(*pattern, occurrenceId);
	if (nullptr == found.pOccurrence)
	{
		outStatus = "KoukuSaydon animation move target is absent.";
		return false;
	}
	found.pOccurrence->iStartOffsetMs = startOffsetMs;
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Moved KoukuSaydon animation box.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Trim_Animation(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	const std::uint32_t sourceStartMs,
	const std::uint32_t playMs,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	const MUTABLE_OCCURRENCE found = nullptr == pattern ? MUTABLE_OCCURRENCE{} :
		Find_Occurrence(*pattern, occurrenceId);
	if (nullptr == found.pOccurrence)
	{
		outStatus = "KoukuSaydon animation trim target is absent.";
		return false;
	}
	found.pOccurrence->iSourceStartMs = sourceStartMs;
	found.pOccurrence->iPlayMs = playMs;
	if (!Validate_SourceStart(*found.pOccurrence, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	std::string policyNote;
	(void)Normalize_EndPolicyForWindow(*found.pOccurrence, policyNote);
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate),
		"Trimmed KoukuSaydon animation box." + policyNote, outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Duplicate_Animation(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	std::string& outOccurrenceId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	const MUTABLE_OCCURRENCE found = nullptr == pattern ? MUTABLE_OCCURRENCE{} :
		Find_Occurrence(*pattern, occurrenceId);
	if (nullptr == found.pOccurrence || pattern->iNextAnimationOrdinal >= 1000000u)
	{
		outStatus = "KoukuSaydon animation duplicate target or ordinal is invalid.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE duplicate = *found.pOccurrence;
	duplicate.strOccurrenceId = pattern->strPatternId + ".animation." +
		std::to_string(pattern->iNextAnimationOrdinal++);
	const std::string duplicateId = duplicate.strOccurrenceId;
	found.pStage->AnimationOccurrences.push_back(std::move(duplicate));
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Duplicated KoukuSaydon animation box " + duplicateId + ".", outStatus))
	{
		return false;
	}
	m_strSelectedOccurrenceId = duplicateId;
	Synchronize_EditorFields();
	outOccurrenceId = duplicateId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_Animation(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = "KoukuSaydon animation delete Pattern is absent.";
		return false;
	}
	MUTABLE_OCCURRENCE found = Find_Occurrence(*pattern, occurrenceId);
	if (nullptr == found.pOccurrence || nullptr == found.pStage)
	{
		outStatus = "KoukuSaydon animation delete target is absent.";
		return false;
	}
	const auto eraseAt = std::find_if(
		found.pStage->AnimationOccurrences.begin(),
		found.pStage->AnimationOccurrences.end(),
		[occurrenceId](
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence)
		{
			return occurrence.strOccurrenceId == occurrenceId;
		});
	found.pStage->AnimationOccurrences.erase(eraseAt);
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Deleted KoukuSaydon animation box.", outStatus))
		return false;
	if (m_strSelectedOccurrenceId == occurrenceId)
	{
		m_strSelectedOccurrenceId.clear();
		Synchronize_EditorFields();
	}
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Move_AnimationToStage(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	const std::string_view targetStageId,
	const std::uint32_t startOffsetMs,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = "KoukuSaydon animation Stage-move Pattern is absent.";
		return false;
	}
	MUTABLE_OCCURRENCE found = Find_Occurrence(*pattern, occurrenceId);
	KOUKU_SAYDON_COMPOSITION_STAGE* const target = Find_Stage(*pattern, targetStageId);
	if (nullptr == found.pOccurrence || nullptr == found.pStage || nullptr == target)
	{
		outStatus = "KoukuSaydon animation Stage-move source or target is absent.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE moved = *found.pOccurrence;
	moved.iStartOffsetMs = startOffsetMs;
	const auto eraseAt = std::find_if(
		found.pStage->AnimationOccurrences.begin(),
		found.pStage->AnimationOccurrences.end(),
		[occurrenceId](
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence)
		{
			return occurrence.strOccurrenceId == occurrenceId;
		});
	found.pStage->AnimationOccurrences.erase(eraseAt);
	target->AnimationOccurrences.push_back(std::move(moved));
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Moved KoukuSaydon animation box to Stage " +
			std::string(targetStageId) + ".", outStatus))
	{
		return false;
	}
	m_strSelectedStageId = std::string(targetStageId);
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Consume_PreviewTransportRequest(
	KOUKU_PREVIEW_TRANSPORT& outTransport,
	std::uint32_t& outSeekMs)
{
	if (KOUKU_PREVIEW_TRANSPORT::NONE == m_ePendingTransport)
		return false;
	outTransport = m_ePendingTransport;
	outSeekMs = m_iPendingSeekMs;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
	m_iPendingSeekMs = 0u;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Is_AppendAdmitted(
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& source,
	std::string& outStatus) const
{
	const bool raw = 0u == source.iSourceActionId && "RAW" == source.strSourceStageId;
	const auto* sourceSlot = Find_SourceSlot(m_ResourceReferences, source);
	const bool available = raw ? std::any_of(m_ModelResources.begin(), m_ModelResources.end(),
		[&](const auto& item) { return item.strRuntimeClip == source.strRuntimeClip &&
			item.strProfileId == source.strProfileId; }) :
		(nullptr != sourceSlot && sourceSlot->strRuntimeClip == source.strRuntimeClip);
	if (!available)
	{
		outStatus = "Select an available Animation Resource first.";
		return false;
	}
	if (!CKoukuSaydonCompositionDocument::Is_KnownProfile(source.strProfileId))
	{
		outStatus = "This clip belongs to another Workbench's model.";
		return false;
	}
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Resolve_NativeClipMs(
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence,
	std::uint32_t& outNativeMs) const
{
	/* Physical clip lengths come only from the native model metadata; a reference slot
	   carries its authored window, not the clip it was cut from. */
	const auto found = std::find_if(m_ModelResources.begin(), m_ModelResources.end(),
		[&occurrence](const COMPOSITION_ANIMATION_RESOURCE& item)
		{
			return item.strProfileId == occurrence.strProfileId &&
				item.strRuntimeClip == occurrence.strRuntimeClip;
		});
	if (found == m_ModelResources.end() || 0u == found->iDurationMs)
		return false;
	outNativeMs = found->iDurationMs;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Validate_SourceStart(
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence,
	std::string& outStatus) const
{
	std::uint32_t nativeMs = 0u;
	if ("HOLD_LAST_POSE" != occurrence.strEndPolicy &&
		Resolve_NativeClipMs(occurrence, nativeMs) && occurrence.iSourceStartMs >= nativeMs)
	{
		outStatus = "Source Start must be inside the native clip (" + std::to_string(nativeMs) +
			" ms). Use HOLD_LAST_POSE to keep only the held tail.";
		return false;
	}
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Queue_AnimationPreview(
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence)
{
	m_PendingPreviewRequest = occurrence;
	m_bPreviewRequestPending = true;
	m_bPatternPreviewRequestPending = false;
	m_strPendingPreviewTargetAsset.clear();
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
}

bool_t Client::CKoukuSaydonActionWorkbench::Normalize_EndPolicyForWindow(
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence,
	std::string& outNote) const
{
	if ("EXACT" != occurrence.strEndPolicy)
		return false;
	std::uint32_t nativeMs = 0u;
	if (!Resolve_NativeClipMs(occurrence, nativeMs))
		return false;
	const double windowEndMs = occurrence.iSourceStartMs +
		static_cast<double>(occurrence.iPlayMs) * occurrence.fPlayRate;
	if (windowEndMs <= nativeMs + 1.0)
		return false;
	occurrence.strEndPolicy = "HOLD_LAST_POSE";
	outNote += " " + occurrence.strRuntimeClip +
		" outruns its native clip; end policy became HOLD_LAST_POSE.";
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Queue_ResourcePatternPreview(
	KOUKU_SAYDON_COMPOSITION_PATTERN pattern, const std::string& targetAssetName)
{
	m_PendingPatternPreview = std::move(pattern);
	m_strPendingPreviewTargetAsset = targetAssetName;
	m_iPendingPreviewStartMs = 0u;
	m_bPendingPreviewStartPaused = false;
	m_bPatternPreviewRequestPending = true;
	m_bPreviewRequestPending = false;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
}

void Client::CKoukuSaydonActionWorkbench::Queue_ModelResourcePreview(
	const COMPOSITION_ANIMATION_RESOURCE& resource)
{
	const auto actor = CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(resource.strProfileId);
	std::string selectionStatus;
	if (!actor.empty()) (void)Select_ActorProfile(actor, selectionStatus);
	if (resource.iDurationMs == 0u)
	{
		m_strStatus = "Clip timing is unavailable. Refresh Animation Resources and retry.";
		return;
	}
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence;
	occurrence.strOccurrenceId = "resource.animation.1";
	occurrence.strProfileId = resource.strProfileId;
	occurrence.strSourceStageId = "RAW";
	occurrence.strSourceSlotId = resource.strRuntimeClip;
	occurrence.strRuntimeClip = resource.strRuntimeClip;
	occurrence.iPlayMs = resource.iDurationMs;
	occurrence.strEndPolicy = resource.strEndPolicy;
	m_SelectedResource = occurrence;
	m_bHasSelectedResource = true;
	m_strSelectedResourceTargetAsset = resource.strTargetAssetName;
	m_strSelectedResourceProfileId.clear();
	m_iSelectedResourceActionId = 0u;
	m_strSelectedSequenceResourceId.clear();
	KOUKU_SAYDON_COMPOSITION_STAGE stage;
	stage.strStageId = "RESOURCE_STAGE_1";
	stage.strStageKind = "ACTIVE";
	stage.iDurationMs = occurrence.iPlayMs;
	stage.AnimationOccurrences.push_back(std::move(occurrence));
	KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
	pattern.strPatternId = "RESOURCE_PREVIEW";
	pattern.strDisplayName = resource.strRuntimeClip;
	pattern.Stages.push_back(std::move(stage));
	Queue_ResourcePatternPreview(std::move(pattern), resource.strTargetAssetName);
	m_strStatus = "Preview requested: " + resource.strTargetAssetName + " / " + resource.strRuntimeClip;
}

void Client::CKoukuSaydonActionWorkbench::Queue_SequencePreview(
	const COMPOSITION_ANIMATION_SEQUENCE_RESOURCE& sequence)
{
	KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
	pattern.strPatternId = "RESOURCE_PREVIEW";
	pattern.strDisplayName = sequence.strDisplayName;
	std::uint32_t totalMs = 0u;
	for (const auto& resource : sequence.Clips)
	{
		if (0u == resource.iDurationMs || resource.iDurationMs > MAX_EDITOR_TIME_MS ||
			totalMs > MAX_EDITOR_TIME_MS - resource.iDurationMs)
		{
			m_strStatus = "Sequence preview requires available clip timing within 600 seconds: " + resource.strRuntimeClip;
			return;
		}
		const auto ordinal = std::to_string(pattern.Stages.size() + 1u);
		KOUKU_SAYDON_COMPOSITION_STAGE stage;
		stage.strStageId = "RESOURCE_STAGE_" + ordinal;
		stage.strStageKind = "ACTIVE";
		stage.iDurationMs = resource.iDurationMs;
		KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence;
		occurrence.strOccurrenceId = "resource.animation." + ordinal;
		occurrence.strProfileId = resource.strProfileId;
		occurrence.strSourceStageId = "RAW";
		occurrence.strSourceSlotId = resource.strRuntimeClip;
		occurrence.strRuntimeClip = resource.strRuntimeClip;
		occurrence.iPlayMs = resource.iDurationMs;
		occurrence.strEndPolicy = resource.strEndPolicy;
		stage.AnimationOccurrences.push_back(std::move(occurrence));
		pattern.Stages.push_back(std::move(stage));
		totalMs += resource.iDurationMs;
	}
	if (pattern.Stages.empty())
	{
		m_strStatus = "Sequence has no animation clips.";
		return;
	}
	m_strSelectedSequenceResourceId = sequence.strStableId;
	m_strSelectedResourceProfileId.clear();
	m_iSelectedResourceActionId = 0u;
	m_bHasSelectedResource = false;
	m_strSelectedResourceTargetAsset.clear();
	Queue_ResourcePatternPreview(std::move(pattern), sequence.strTargetAssetName);
	m_strStatus = "Sequence preview requested: " + sequence.strDisplayName;
}

void Client::CKoukuSaydonActionWorkbench::Queue_SlotPreview(
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& action,
	const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& stage,
	const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot)
{
	const auto actor = CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(reference.strProfileId);
	std::string selectionStatus;
	if (!actor.empty()) (void)Select_ActorProfile(actor, selectionStatus);
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE request =
		Build_ReferenceOccurrence(reference, action, stage, slot);
	m_strSelectedResourceProfileId = reference.strProfileId;
	m_iSelectedResourceActionId = action.iSourceActionId;
	m_strSelectedResourceTargetAsset.clear();
	m_strSelectedSequenceResourceId.clear();
	m_SelectedResource = request;
	m_bHasSelectedResource = true;
	Queue_AnimationPreview(request);
	m_strStatus = "Animation preview requested: " + slot.strRuntimeClip;
}

void Client::CKoukuSaydonActionWorkbench::Queue_ActionPreview(
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& action,
	const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE* const selectedStage)
{
	const auto actor = CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(reference.strProfileId);
	std::string selectionStatus;
	if (!actor.empty()) (void)Select_ActorProfile(actor, selectionStatus);
	m_strSelectedResourceProfileId = reference.strProfileId;
	m_iSelectedResourceActionId = action.iSourceActionId;
	KOUKU_SAYDON_COMPOSITION_PATTERN preview;
	preview.strPatternId = "KAKULSAYDON_RESOURCE_PREVIEW";
	preview.strDisplayName = action.strDisplayName;
	preview.strAuthoringStatus = "DRAFT";
	preview.strCategory = "NORMAL";
	std::uint32_t occurrenceOrdinal = 1u;
	std::uint32_t stageOrdinal = 1u;
	for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& sourceStage :
		action.Stages)
	{
		if (nullptr != selectedStage &&
			selectedStage->strStageId != sourceStage.strStageId)
		{
			continue;
		}
		if (sourceStage.Slots.empty())
			continue;
		KOUKU_SAYDON_COMPOSITION_STAGE stage;
		stage.strStageId = "STAGE_" + std::to_string(stageOrdinal);
		stage.strActionId = "kakulsaydon.preview." + reference.strProfileId + "." +
			std::to_string(action.iSourceActionId) + "." + std::to_string(stageOrdinal);
		stage.strStageKind = "ACTIVE";
		std::uint32_t offsetMs = 0u;
		for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot : sourceStage.Slots)
		{
			KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence =
				Build_ReferenceOccurrence(reference, action, sourceStage, slot);
			occurrence.strOccurrenceId = preview.strPatternId + ".animation." +
				std::to_string(occurrenceOrdinal++);
			occurrence.iStartOffsetMs = offsetMs;
			offsetMs += slot.iPlayMs;
			stage.AnimationOccurrences.push_back(std::move(occurrence));
		}
		stage.iDurationMs = (std::max)(offsetMs, 1u);
		preview.Stages.push_back(std::move(stage));
		++stageOrdinal;
	}
	preview.iNextStageOrdinal = stageOrdinal;
	preview.iNextAnimationOrdinal = occurrenceOrdinal;
	if (preview.Stages.empty())
	{
		m_strStatus = "The action has no clip slot to preview.";
		return;
	}
	m_PendingPatternPreview = std::move(preview);
	m_strPendingPreviewTargetAsset.clear();
	m_strSelectedSequenceResourceId.clear();
	m_strSelectedResourceTargetAsset.clear();
	m_iPendingPreviewStartMs = 0u;
	m_bPendingPreviewStartPaused = false;
	m_bPatternPreviewRequestPending = true;
	m_bPreviewRequestPending = false;
	m_bHasSelectedResource = false;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
	m_strStatus = nullptr == selectedStage ?
		"Action preview requested." : "Source Stage preview requested.";
}

void Client::CKoukuSaydonActionWorkbench::Set_ModelResources(
	std::vector<COMPOSITION_ANIMATION_RESOURCE> resources, std::string status)
{
	if (!resources.empty())
	{
		m_ModelResources = std::move(resources);
		// Raw selections carry a copy of timing. A refresh must not append that
		// stale window after the model file has been replaced or retimed.
		if (!m_strSelectedResourceTargetAsset.empty())
		{
			m_bHasSelectedResource = false;
			m_strSelectedResourceTargetAsset.clear();
		}
	}
	m_strResourceStatus = std::move(status);
	m_bResourceTreeDirty = true;
}

bool Client::CKoukuSaydonActionWorkbench::Can_AppendCompositionAnimationResource(
	const COMPOSITION_ANIMATION_RESOURCE& resource, const bool asNewStage,
	std::string& outStatus) const
{
	const auto actor = CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(resource.strProfileId);
	if (actor.empty() || resource.strTargetAssetName != actor)
	{
		outStatus = "Select a clip from the Kouku, Saydon or Large Saydon model.";
		return false;
	}
	const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (!m_bHasDraft || (nullptr == pattern && !asNewStage) ||
		(nullptr != pattern && (!pattern->strLoadError.empty() || pattern->strActorProfileId != actor)))
	{
		outStatus = "Select an editable KoukuSaydon Pattern to append.";
		return false;
	}
	if (!asNewStage && (nullptr == pattern || nullptr == Find_Stage(*pattern, m_strSelectedStageId)))
	{
		outStatus = "Select a Stage to add an Animation row.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE source;
	source.strProfileId = resource.strProfileId;
	source.strSourceStageId = "RAW";
	source.strSourceSlotId = resource.strRuntimeClip;
	source.strRuntimeClip = resource.strRuntimeClip;
	source.iPlayMs = resource.iDurationMs;
	source.strEndPolicy = resource.strEndPolicy;
	return Is_AppendAdmitted(source, outStatus);
}

bool Client::CKoukuSaydonActionWorkbench::Append_CompositionAnimationResource(
	const COMPOSITION_ANIMATION_RESOURCE& resource, const bool asNewStage,
	std::string& outStatus)
{
	if (!Can_AppendCompositionAnimationResource(resource, asNewStage, outStatus))
		return false;
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE source;
	source.strProfileId = resource.strProfileId;
	source.strSourceStageId = "RAW";
	source.strSourceSlotId = resource.strRuntimeClip;
	source.strRuntimeClip = resource.strRuntimeClip;
	source.iPlayMs = resource.iDurationMs;
	source.strEndPolicy = resource.strEndPolicy;
	std::string occurrenceId, stageId;
	const bool result = asNewStage ?
		Append_AnimationAsStage(m_strSelectedPatternId, source, stageId, occurrenceId, outStatus) :
		Bind_Animation(m_strSelectedPatternId, m_strSelectedStageId, source, 0u, occurrenceId, outStatus);
	m_strStatus = outStatus;
	return result;
}

void Client::CKoukuSaydonActionWorkbench::Rebuild_ResourceTree()
{
	m_strResourceTreeQuery = m_ResourceSearch;
	m_bResourceTreeDirty = false;
	m_ResourceLeaves.clear();
	m_ResourceTree = {};
	const std::string_view query = m_strResourceTreeQuery;
	for (std::size_t documentIndex = 0u;
		documentIndex < m_ResourceReferences.Documents.size(); ++documentIndex)
	{
		const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference =
			m_ResourceReferences.Documents[documentIndex];
		if (reference.strProfileId.empty())
			continue;
		for (std::size_t actionIndex = 0u; actionIndex < reference.Actions.size(); ++actionIndex)
		{
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& action = reference.Actions[actionIndex];
			bool_t hasSlot = false;
			bool_t matches = query.empty() ||
				ContainsInsensitive(action.strDisplayName, query) ||
				ContainsInsensitive(reference.strProfileId, query) ||
				ContainsInsensitive(std::to_string(action.iSourceActionId), query);
			for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& stage : action.Stages)
			{
				for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot : stage.Slots)
				{
					hasSlot = true;
					if (!matches && ContainsInsensitive(slot.strRuntimeClip, query))
						matches = true;
				}
			}
			if (!hasSlot || !matches)
				continue;
			const std::size_t leafIndex = m_ResourceLeaves.size();
			m_ResourceLeaves.push_back({ documentIndex, actionIndex });
			InsertResourceTree(m_ResourceTree,
				{ CKoukuSaydonAnimationActionDocument::Resolve_ActionCategory(
					  reference.strProfileId, action.strDisplayName),
				  reference.strProfileId },
				leafIndex);
		}
	}
	for (std::size_t index = 0u; index < m_SequenceResources.size(); ++index)
	{
		const auto& sequence = m_SequenceResources[index];
		const bool matches = query.empty() || ContainsInsensitive(sequence.strDisplayName, query) ||
			ContainsInsensitive(sequence.strProfileId, query) || ContainsInsensitive(sequence.strStableId, query) ||
			std::any_of(sequence.Clips.begin(), sequence.Clips.end(), [&](const auto& clip) {
				return ContainsInsensitive(clip.strRuntimeClip, query);
			});
		if (!matches) continue;
		const auto leafIndex = m_ResourceLeaves.size();
		RESOURCE_ACTION_LEAF leaf;
		leaf.iAction = index;
		leaf.bSequence = true;
		m_ResourceLeaves.push_back(leaf);
		InsertResourceTree(m_ResourceTree, { "Valtan", "Valtan Sequences" }, leafIndex);
	}
	(void)FinalizeResourceTree(m_ResourceTree);
	m_PhysicalResourceTree = {};
	for (std::size_t index = 0u; index < m_ModelResources.size(); ++index)
	{
		const auto& resource = m_ModelResources[index];
		if (!query.empty() && !ContainsInsensitive(resource.strRuntimeClip, query) &&
			!ContainsInsensitive(resource.strTargetAssetName, query) &&
			!ContainsInsensitive(resource.strProfileId, query)) continue;
		InsertResourceTree(m_PhysicalResourceTree, { resource.strTargetAssetName }, index);
	}
	(void)FinalizeResourceTree(m_PhysicalResourceTree);
}

bool_t Client::CKoukuSaydonActionWorkbench::Append_ActionAsStages(
	const std::string_view patternId,
	const std::string_view profileId,
	const std::uint32_t sourceActionId,
	std::string& outStatus)
{
	const auto* reference = Find_Reference(m_ResourceReferences, profileId);
	const auto* action = nullptr == reference ? nullptr : Find_Action(*reference, sourceActionId);
	if (nullptr == action)
	{
		outStatus = m_strStatus = "Select an extracted action first.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_AppendPattern(
		candidate, patternId, profileId, action->strDisplayName, outStatus);
	if (nullptr == pattern)
	{
		m_strStatus = outStatus;
		return false;
	}
	const std::string targetPatternId = pattern->strPatternId;
	std::string policyNote;
	std::size_t appendedStages = 0u;
	std::string lastStageId;
	std::string lastOccurrenceId;
	for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& sourceStage : action->Stages)
	{
		if (sourceStage.Slots.empty())
			continue;
		if (pattern->iNextStageOrdinal >= 1000000u || pattern->iNextAnimationOrdinal >= 1000000u)
		{
			outStatus = m_strStatus = "Pattern stable ID ordinals are exhausted.";
			return false;
		}
		const std::uint32_t stageOrdinal = pattern->iNextStageOrdinal++;
		KOUKU_SAYDON_COMPOSITION_STAGE stage;
		stage.strStageId = "STAGE_" + std::to_string(stageOrdinal);
		stage.strActionId = pattern->strPatternId + ".stage." + std::to_string(stageOrdinal);
		stage.strStageKind = "ACTIVE";
		std::uint32_t offsetMs = 0u;
		for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot : sourceStage.Slots)
		{
			if (pattern->iNextAnimationOrdinal >= 1000000u || slot.iPlayMs > MAX_EDITOR_TIME_MS ||
				offsetMs > MAX_EDITOR_TIME_MS - slot.iPlayMs)
			{
				outStatus = m_strStatus = "Action exceeds the Stage duration or stable ID limit; draft preserved.";
				return false;
			}
			KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence =
				Build_ReferenceOccurrence(*reference, *action, sourceStage, slot);
			occurrence.strOccurrenceId = pattern->strPatternId + ".animation." +
				std::to_string(pattern->iNextAnimationOrdinal++);
			occurrence.iStartOffsetMs = offsetMs;
			(void)Normalize_EndPolicyForWindow(occurrence, policyNote);
			offsetMs += occurrence.iPlayMs;
			lastOccurrenceId = occurrence.strOccurrenceId;
			stage.AnimationOccurrences.push_back(std::move(occurrence));
		}
		stage.iDurationMs = (std::max)(offsetMs, 1u);
		lastStageId = stage.strStageId;
		pattern->Stages.push_back(std::move(stage));
		++appendedStages;
	}
	if (0u == appendedStages)
	{
		outStatus = m_strStatus = "The action has no clip slot to append.";
		return false;
	}
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Appended " + std::to_string(appendedStages) + " Stage(s) from action " +
				std::to_string(sourceActionId) + "." + policyNote, outStatus))
	{
		return false;
	}
	Clear_TimelineSelection();
	m_bFitRequested = true;
	m_strSelectedActorProfileId = std::string(CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(profileId));
	m_strSelectedPatternId = targetPatternId;
	Select_TimelineBox(lastStageId, lastOccurrenceId, false);
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Append_ActionToStage(
	const std::string_view patternId,
	const std::string_view stageId,
	const std::string_view profileId,
	const std::uint32_t sourceActionId,
	std::string& outStatus)
{
	const auto* reference = Find_Reference(m_ResourceReferences, profileId);
	const auto* action = nullptr == reference ? nullptr : Find_Action(*reference, sourceActionId);
	if (nullptr == action)
	{
		outStatus = m_strStatus = "Select an extracted action first.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_STAGE* const stage = nullptr == pattern ?
		nullptr : Find_Stage(*pattern, stageId);
	if (nullptr == stage)
	{
		outStatus = m_strStatus = "Append target Stage is unavailable.";
		return false;
	}
	std::uint32_t offsetMs = 0u;
	for (const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& row : stage->AnimationOccurrences)
		offsetMs = (std::max)(offsetMs, row.iStartOffsetMs + row.iPlayMs);
	std::string policyNote;
	std::size_t appendedClips = 0u;
	std::string lastOccurrenceId;
	for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& sourceStage : action->Stages)
	{
		for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot : sourceStage.Slots)
		{
			if (pattern->iNextAnimationOrdinal >= 1000000u)
			{
				outStatus = m_strStatus = "Pattern stable ID ordinals are exhausted.";
				return false;
			}
			KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence =
				Build_ReferenceOccurrence(*reference, *action, sourceStage, slot);
			occurrence.strOccurrenceId = pattern->strPatternId + ".animation." +
				std::to_string(pattern->iNextAnimationOrdinal++);
			occurrence.iStartOffsetMs = offsetMs;
			(void)Normalize_EndPolicyForWindow(occurrence, policyNote);
			offsetMs += occurrence.iPlayMs;
			lastOccurrenceId = occurrence.strOccurrenceId;
			stage->AnimationOccurrences.push_back(std::move(occurrence));
			++appendedClips;
		}
	}
	if (0u == appendedClips)
	{
		outStatus = m_strStatus = "The action has no clip slot to append.";
		return false;
	}
	stage->iDurationMs = (std::max)(stage->iDurationMs, offsetMs);
	const std::string targetStageId(stageId);
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Appended " + std::to_string(appendedClips) + " clip(s) to Stage " +
				targetStageId + "." + policyNote, outStatus))
	{
		return false;
	}
	Clear_TimelineSelection();
	m_bFitRequested = true;
	m_strSelectedPatternId = std::string(patternId);
	Select_TimelineBox(targetStageId, lastOccurrenceId, false);
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_AnimationPlayback(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	const f32_t playRate,
	const std::string_view endPolicy,
	std::string& outStatus)
{
	const bool_t knownPolicy = std::any_of(END_POLICIES.begin(), END_POLICIES.end(),
		[endPolicy](const char_t* const value) { return endPolicy == value; });
	if (!knownPolicy || !std::isfinite(playRate) || playRate < 0.01f || playRate > 16.f)
	{
		outStatus = m_strStatus =
			"Play rate must be 0.01..16 and end policy EXACT, HOLD_LAST_POSE or LOOP_TO_WINDOW.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	const MUTABLE_OCCURRENCE found = nullptr == pattern ? MUTABLE_OCCURRENCE{} :
		Find_Occurrence(*pattern, occurrenceId);
	if (nullptr == found.pOccurrence)
	{
		outStatus = m_strStatus = "KoukuSaydon animation playback target is absent.";
		return false;
	}
	found.pOccurrence->fPlayRate = playRate;
	found.pOccurrence->strEndPolicy = std::string(endPolicy);
	if (!Validate_SourceStart(*found.pOccurrence, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	std::uint32_t nativeMs = 0u;
	if ("EXACT" == endPolicy && Resolve_NativeClipMs(*found.pOccurrence, nativeMs))
	{
		const double windowEndMs = found.pOccurrence->iSourceStartMs +
			static_cast<double>(found.pOccurrence->iPlayMs) * playRate;
		if (windowEndMs > nativeMs + 1.0)
		{
			outStatus = m_strStatus = "EXACT cannot outrun the native clip (" +
				std::to_string(nativeMs) +
				" ms). Shorten the box, or choose HOLD_LAST_POSE or LOOP_TO_WINDOW.";
			return false;
		}
	}
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Updated animation playback.", outStatus);
}

void Client::CKoukuSaydonActionWorkbench::Normalize_Selection()
{
	if (!m_strSelectedLogicId.empty() && nullptr == Find_Logic(m_Draft, m_strSelectedLogicId))
		m_strSelectedLogicId.clear();
	if (!m_strSelectedSummonId.empty() && nullptr == Find_Summon(m_Draft, m_strSelectedSummonId))
		m_strSelectedSummonId.clear();
	if (!m_strSelectedWorldId.empty() && nullptr == Find_World(m_Draft, m_strSelectedWorldId))
		m_strSelectedWorldId.clear();
	if (!m_strSelectedSceneProfileId.empty() &&
		nullptr == Find_SceneProfile(m_Draft, m_strSelectedSceneProfileId))
		m_strSelectedSceneProfileId.clear();
	if (!m_strSelectedPresentationResourceId.empty() &&
		nullptr == Find_PresentationResource(m_Draft, m_strSelectedPresentationResourceId))
		m_strSelectedPresentationResourceId.clear();
	if (!m_strLogicValueDraftId.empty() && nullptr == Find_Logic(m_Draft, m_strLogicValueDraftId))
		m_strLogicValueDraftId.clear();
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (nullptr == pattern)
	{
		Clear_TimelineSelection();
		m_strSelectedPatternId.clear();
		m_strSelectedStageId.clear();
		m_strSelectedOccurrenceId.clear();
		m_strSelectedLogicOccurrenceId.clear();
		m_strSelectedSummonOccurrenceId.clear();
		m_strSelectedWorldOccurrenceId.clear();
		m_strSelectedSceneProfileOccurrenceId.clear();
		m_strSelectedPresentationOccurrenceId.clear();
		return;
	}
	if (!m_strSelectedLogicOccurrenceId.empty() &&
		nullptr == Find_LogicBox(*pattern, m_strSelectedLogicOccurrenceId))
	{
		m_strSelectedLogicOccurrenceId.clear();
	}
	if (!m_strSelectedSummonOccurrenceId.empty() &&
		nullptr == Find_SummonBox(*pattern, m_strSelectedSummonOccurrenceId))
	{
		m_strSelectedSummonOccurrenceId.clear();
	}
	if (!m_strSelectedWorldOccurrenceId.empty() &&
		nullptr == Find_WorldBox(*pattern, m_strSelectedWorldOccurrenceId))
	{
		m_strSelectedWorldOccurrenceId.clear();
	}
	if (!m_strSelectedSceneProfileOccurrenceId.empty() &&
		nullptr == Find_SceneProfileBox(*pattern, m_strSelectedSceneProfileOccurrenceId))
		m_strSelectedSceneProfileOccurrenceId.clear();
	if (!m_strSelectedPresentationOccurrenceId.empty() &&
		nullptr == Find_PresentationBox(*pattern, m_strSelectedPresentationOccurrenceId))
		m_strSelectedPresentationOccurrenceId.clear();
	m_strSelectedActorProfileId = pattern->strActorProfileId;
	if (m_strTimelineSelectionPatternId != m_strSelectedPatternId)
	{
		Clear_TimelineSelection();
		m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	}
	std::erase_if(m_TimelineSelectedStageIds, [pattern](const auto& id)
		{ return nullptr == Find_Stage(*pattern, id); });
	std::erase_if(m_TimelineSelectedOccurrenceIds, [pattern](const auto& id)
		{ return nullptr == Find_Occurrence(*pattern, id); });
	if (!m_strSelectedStageId.empty() &&
		nullptr == Find_Stage(*pattern, m_strSelectedStageId))
	{
		m_strSelectedStageId.clear();
	}
	if (!m_strSelectedOccurrenceId.empty())
	{
		const KOUKU_SAYDON_COMPOSITION_STAGE* occurrenceStage = nullptr;
		if (nullptr == Find_Occurrence(
				*pattern, m_strSelectedOccurrenceId, &occurrenceStage))
		{
			m_strSelectedOccurrenceId.clear();
		}
		else if (m_strSelectedStageId.empty() && nullptr != occurrenceStage)
		{
			m_strSelectedStageId = occurrenceStage->strStageId;
		}
	}
}

void Client::CKoukuSaydonActionWorkbench::Synchronize_EditorFields()
{
	m_PatternName[0] = '\0';
	m_iPatternDurationMs = 0;
	m_iOccurrenceStartOffsetMs = 0;
	m_iOccurrenceSourceStartMs = 0;
	m_iOccurrencePlayMs = 1;
	m_fOccurrencePlayRate = 1.f;
	m_iOccurrenceEndPolicy = 0;
	m_iLogicBoxStartMs = 0;
	m_iLogicBoxDurationMs = 1000;
	m_iSummonBoxStartMs = 0;
	m_iSummonBoxDurationMs = 1000;
	m_iWorldBoxStartMs = 0;
	m_iWorldBoxDurationMs = 1000;
	m_fWorldBoxPlaybackSpeed = 1.f;
	m_iSceneProfileBoxStartMs = 0;
	m_iSceneProfileBoxDurationMs = 1000;
	m_iSceneProfileBoxBlendMs = 500;
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (nullptr == pattern)
		return;
	(void)Copy_Text(m_PatternName, std::size(m_PatternName), pattern->strDisplayName);
	m_iPatternDurationMs = static_cast<int32_t>(Pattern_DurationMs(*pattern));
	if (const auto* box = Find_PresentationBox(*pattern, m_strSelectedPresentationOccurrenceId))
		m_PresentationBoxEdit = *box;
	if (const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE* const logicBox =
			Find_LogicBox(*pattern, m_strSelectedLogicOccurrenceId);
		nullptr != logicBox)
	{
		m_iLogicBoxStartMs = static_cast<int32_t>(logicBox->iStartMs);
		m_iLogicBoxDurationMs = static_cast<int32_t>(logicBox->iDurationMs);
	}
	if (const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE* const summonBox =
			Find_SummonBox(*pattern, m_strSelectedSummonOccurrenceId);
		nullptr != summonBox)
	{
		m_iSummonBoxStartMs = static_cast<int32_t>(summonBox->iStartMs);
		m_iSummonBoxDurationMs = static_cast<int32_t>(summonBox->iDurationMs);
	}
	if (const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE* const worldBox =
			Find_WorldBox(*pattern, m_strSelectedWorldOccurrenceId);
		nullptr != worldBox)
	{
		m_iWorldBoxStartMs = static_cast<int32_t>(worldBox->iStartMs);
		m_iWorldBoxDurationMs = static_cast<int32_t>(worldBox->iDurationMs);
		m_fWorldBoxPlaybackSpeed = worldBox->fPlaybackSpeed;
	}
	if (const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE* const sceneBox =
			Find_SceneProfileBox(*pattern, m_strSelectedSceneProfileOccurrenceId);
		nullptr != sceneBox)
	{
		m_iSceneProfileBoxStartMs = static_cast<int32_t>(sceneBox->iStartMs);
		m_iSceneProfileBoxDurationMs = static_cast<int32_t>(sceneBox->iDurationMs);
		m_iSceneProfileBoxBlendMs = static_cast<int32_t>(sceneBox->iBlendMs);
	}
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* const occurrence =
		Find_Occurrence(*pattern, m_strSelectedOccurrenceId);
	if (nullptr == occurrence)
		return;
	m_iOccurrenceStartOffsetMs = static_cast<int32_t>(occurrence->iStartOffsetMs);
	m_iOccurrenceSourceStartMs = static_cast<int32_t>(occurrence->iSourceStartMs);
	m_iOccurrencePlayMs = static_cast<int32_t>(occurrence->iPlayMs);
	m_fOccurrencePlayRate = occurrence->fPlayRate;
	for (int32_t index = 0; index < static_cast<int32_t>(END_POLICIES.size()); ++index)
	{
		if (occurrence->strEndPolicy == END_POLICIES[index])
			m_iOccurrenceEndPolicy = index;
	}
}

void Client::CKoukuSaydonActionWorkbench::Render_Toolbar()
{
	const bool_t publishing = Is_PublishRunning();
	if (!m_bSharedWorkspaceActive)
	{
		ImGui::Checkbox("Resources", &m_bResourcesOpen);
		ImGui::SameLine();
	}
	ImGui::BeginDisabled(publishing);
	if (ImGui::Button("Reload"))
	{
		if (m_bDirty)
			m_bReloadConfirmationRequested = true;
		else
		{
			std::string status;
			(void)Reload(status);
		}
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(publishing || !m_bDirty || !m_Document.Is_Fresh());
	if (ImGui::Button("Save"))
	{
		std::string status;
		(void)Save(status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(
		publishing || m_bDirty || !m_Document.Is_Fresh() ||
		m_Draft.PlayAllPatternIds.empty());
	if (ImGui::Button("Publish All PRODUCT"))
	{
		std::string status;
		(void)Publish_Product(status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const selectedPattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	ImGui::BeginDisabled(
		publishing || m_bDirty || nullptr == selectedPattern ||
		selectedPattern->strAuthoringStatus != "PRODUCT");
	if (ImGui::Button("Play Published Product (Server)"))
	{
		m_strPendingServerPlayPatternId = m_strSelectedPatternId;
		m_iPendingServerPlaySourceRevision = m_Draft.iRevision;
		m_bServerPlayRequestPending = true;
		m_strStatus =
			"Routing the exact published Product to KoukuSaydon Boss Tool. The Server must have been restarted after Publish.";
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Fit"))
		m_bFitRequested = true;
	ImGui::SameLine();
	if (!m_bSharedWorkspaceActive)
	{
		if (ImGui::Button(m_bTimelineMaximized ? "Restore Panels" : "Maximize Timeline"))
			m_bTimelineMaximized = !m_bTimelineMaximized;
		ImGui::SameLine();
	}
	ImGui::SetNextItemWidth(150.f);
	ImGui::SliderFloat("Zoom##KoukuTimeline", &m_fPixelsPerSecond,
		1.f, 500.f, "%.0f px/s", ImGuiSliderFlags_Logarithmic);

	const char_t* const freshness = m_Document.Is_Fresh() ? "fresh" :
		(m_Document.Has_LastGood() ? "stale/last-good" : "cold");
	ImGui::Text("rev %u | %s%s | %s",
		m_bHasDraft ? m_Draft.iRevision : 0u,
		freshness, m_bDirty ? " | dirty" : "", m_strStatus.c_str());
}

void Client::CKoukuSaydonActionWorkbench::Render_PatternsAndResources()
{
	ImGui::SeparatorText("Patterns by Model");
	if (ImGui::BeginCombo("Model##KoukuPatternActor", Actor_Label(m_strSelectedActorProfileId)))
	{
		for (const auto* actor : ACTOR_PROFILES)
		{
			if (ImGui::Selectable(Actor_Label(actor), m_strSelectedActorProfileId == actor))
			{
				std::string status;
				(void)Select_ActorProfile(actor, status);
				m_strStatus = status;
			}
		}
		ImGui::EndCombo();
	}
	if (ImGui::BeginChild("##KoukuPatternList", ImVec2(0.f, 150.f),
		ImGuiChildFlags_Borders))
	{
		const bool_t gate2Actor = m_strSelectedActorProfileId == "MN_RPCZ_00" ||
			m_strSelectedActorProfileId == "MN_RPCT_06";
		const std::string gateLabel = (gate2Actor ?
			std::string("\xEB\x8C\x80\xED\x98\x95 \xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC, \xEC\xBF\xA0\xED\x81\xAC - 2\xEA\xB4\x80\xEB\xAC\xB8") :
			(m_strBossVariantLabel.empty() ? std::string(DEFAULT_BOSS_VARIANT_LABEL) : m_strBossVariantLabel)) + "##KoukuGate";
		if (ImGui::TreeNodeEx(gateLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (const auto& pattern : m_Draft.Patterns)
			{
				if (pattern.strActorProfileId != m_strSelectedActorProfileId && pattern.strLoadError.empty()) continue;
				std::string name = pattern.strDisplayName;
				const std::string prefix = "1\xEA\xB4\x80\xEB\xAC\xB8 / ";
				if (name.starts_with(prefix)) name.erase(0u, prefix.size());
				const std::string label = name +
					("PRODUCT" == pattern.strAuthoringStatus ? " [PRODUCT]" : "") +
					(pattern.strLoadError.empty() ? "" : " [Error]") +
					"##" + pattern.strPatternId;
				if (ImGui::Selectable(label.c_str(), m_strSelectedPatternId == pattern.strPatternId))
				{
					std::string status;
					(void)Select_PatternById(pattern.strPatternId, status);
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s | %s\n%s", pattern.strPatternId.c_str(),
						pattern.strAuthoringStatus.c_str(), pattern.strLoadError.c_str());
			}
			ImGui::TreePop();
		}
	}
	ImGui::EndChild();

	ImGui::InputTextWithHint("##NewKoukuPattern", "Pattern display name",
		m_NewPatternName, std::size(m_NewPatternName));
	static int32_t categoryIndex = 1;
	ImGui::SetNextItemWidth(110.f);
	if (ImGui::BeginCombo("##NewKoukuCategory", PATTERN_CATEGORIES[categoryIndex]))
	{
		for (int32_t index = 0; index < static_cast<int32_t>(PATTERN_CATEGORIES.size()); ++index)
		{
			if (ImGui::Selectable(PATTERN_CATEGORIES[index], categoryIndex == index))
				categoryIndex = index;
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ImGui::BeginDisabled('\0' == m_NewPatternName[0] || !m_bHasDraft);
	if (ImGui::Button("Create Pattern"))
	{
		std::string patternId;
		std::string status;
		if (Create_Pattern(m_NewPatternName, PATTERN_CATEGORIES[categoryIndex],
				patternId, status))
		{
			m_NewPatternName[0] = '\0';
		}
	}
	ImGui::EndDisabled();

}

void Client::CKoukuSaydonActionWorkbench::Render_ResourcesWindow()
{
	if (!m_bResourcesOpen || !m_bOpen) return;
	ImGui::SetNextWindowSize(ImVec2(450.f, 600.f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Composition Resources###KoukuCompositionResources", &m_bResourcesOpen))
		Render_ResourceTree();
	ImGui::End();
}

void Client::CKoukuSaydonActionWorkbench::Render_ResourceTree()
{
	/* One tab per family. Only Animation and Logic own an authoring flow today;
	   the remaining tabs name their family without pretending to work. */
	if (!ImGui::BeginTabBar("##KoukuResourceCategories"))
		return;
	for (int32_t index = 0; index < static_cast<int32_t>(RESOURCE_CATEGORIES.size()); ++index)
	{
		if (!ImGui::BeginTabItem(RESOURCE_CATEGORIES[index]))
			continue;
		m_iSelectedResourceCategory = index;
		const std::string_view category = RESOURCE_CATEGORIES[index];
		if ("Animation" == category)
			Render_AnimationResources();
		else if ("Logic" == category)
			Render_LogicResources();
		else if ("Summon" == category)
			Render_SummonResources();
		else if ("World" == category)
			Render_WorldResources();
		else if ("Scene Profile" == category)
			Render_SceneProfileResources();
		else if ("Effect" == category) Render_PresentationResources(KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
		else if ("Collider" == category) Render_PresentationResources(KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER);
		else if ("Sound" == category) Render_PresentationResources(KOUKU_SAYDON_PRESENTATION_KIND::SOUND);
		else if ("Camera" == category) Render_PresentationResources(KOUKU_SAYDON_PRESENTATION_KIND::CAMERA);
		else if ("Light" == category) Render_PresentationResources(KOUKU_SAYDON_PRESENTATION_KIND::LIGHT);
		else
			ImGui::TextDisabled(
				"%s rows are not authorable yet. Their save and playback consumer lands in a later slice.",
				RESOURCE_CATEGORIES[index]);
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();
}

void Client::CKoukuSaydonActionWorkbench::Render_AnimationResources()
{
	ImGui::SeparatorText("Animation Resources");
	if (ImGui::Button("Refresh Animation Resources")) m_bResourceRefreshRequested = true;
	ImGui::SameLine();
	ImGui::TextDisabled("%zu actions | %zu physical clips",
		m_ResourceLeaves.size(), m_ModelResources.size());
	ImGui::TextWrapped("%s", m_strResourceStatus.c_str());
	if (!m_strSequenceResourceStatus.empty()) ImGui::TextWrapped("%s", m_strSequenceResourceStatus.c_str());
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::InputTextWithHint("##KoukuResourceSearch", "Search action, profile or clip...",
			m_ResourceSearch, sizeof(m_ResourceSearch)))
	{
		m_bResourceTreeDirty = true;
	}
	if (m_bResourceTreeDirty || m_strResourceTreeQuery != m_ResourceSearch)
		Rebuild_ResourceTree();

	const KOUKU_SAYDON_COMPOSITION_PATTERN* const selectedPattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool_t patternEditable = nullptr != selectedPattern &&
		selectedPattern->strLoadError.empty();
	const bool_t stageSelected = patternEditable && !m_strSelectedStageId.empty();
	const std::string targetActorProfile = nullptr == selectedPattern ? std::string{} : selectedPattern->strActorProfileId;
	const std::string targetPatternName = nullptr == selectedPattern ? "new Pattern" : selectedPattern->strDisplayName;

	const auto selectedSequence = std::find_if(m_SequenceResources.begin(), m_SequenceResources.end(),
		[this](const auto& item) { return item.strStableId == m_strSelectedSequenceResourceId; });
	if (selectedSequence != m_SequenceResources.end())
	{
		ImGui::SeparatorText("Selected Sequence");
		ImGui::TextWrapped("%s", selectedSequence->strDisplayName.c_str());
		if (ImGui::Button("Preview Sequence")) Queue_SequencePreview(*selectedSequence);
		ImGui::TextDisabled("Valtan sequence previews on its own model; select a Kouku body clip to append here.");
	}
	/* The selected extracted action, listed the way the designer authored it. */
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT* const selectedDocument =
		m_strSelectedResourceProfileId.empty() ? nullptr :
		Find_Reference(m_ResourceReferences, m_strSelectedResourceProfileId);
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE* const selectedAction =
		nullptr == selectedDocument ? nullptr :
		Find_Action(*selectedDocument, m_iSelectedResourceActionId);
	if (nullptr != selectedAction)
	{
		ImGui::SeparatorText("Selected Action");
		ImGui::TextWrapped("%s", selectedAction->strDisplayName.c_str());
		ImGui::TextDisabled("Action %u | %s | %s", selectedAction->iSourceActionId,
			selectedDocument->strProfileId.c_str(),
			CKoukuSaydonAnimationActionDocument::Resolve_ActionCategory(
				selectedDocument->strProfileId, selectedAction->strDisplayName));
		ImGui::BeginChild("##SelectedActionSlots", ImVec2(0.f, 90.f), ImGuiChildFlags_Borders);
		std::uint32_t clipOrdinal = 0u;
		for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& stage : selectedAction->Stages)
		{
			for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot : stage.Slots)
			{
				ImGui::BulletText("%02u  %s  | %u ms%s", ++clipOrdinal,
					slot.strRuntimeClip.c_str(), slot.iPlayMs, slot.bLoop ? " loop" : "");
			}
		}
		ImGui::EndChild();
		const auto actionActor = CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(selectedDocument->strProfileId);
		const bool_t canAppendAction = m_bHasDraft && !actionActor.empty() &&
			(nullptr == selectedPattern || (patternEditable && targetActorProfile == actionActor));
		if (ImGui::Button("Preview Action"))
			Queue_ActionPreview(*selectedDocument, *selectedAction, nullptr);
		ImGui::SameLine();
		ImGui::BeginDisabled(!canAppendAction);
		if (ImGui::Button("Append Action as Stages"))
		{
			std::string status;
			(void)Append_ActionAsStages(m_strSelectedPatternId,
				selectedDocument->strProfileId, selectedAction->iSourceActionId, status);
			m_strStatus = status;
		}
		ImGui::BeginDisabled(!stageSelected);
		if (ImGui::Button("Append Action to Selected Stage"))
		{
			std::string status;
			(void)Append_ActionToStage(m_strSelectedPatternId, m_strSelectedStageId,
				selectedDocument->strProfileId, selectedAction->iSourceActionId, status);
			m_strStatus = status;
		}
		ImGui::EndDisabled();
		ImGui::EndDisabled();
		ImGui::TextWrapped("Append target: %s / %s", Actor_Label(actionActor),
			targetPatternName.c_str());
	}
	if (m_bHasSelectedResource)
	{
		ImGui::SeparatorText("Selected Clip");
		ImGui::TextDisabled("%s | %s | %u ms | %s",
			m_SelectedResource.strProfileId.c_str(),
			m_SelectedResource.strRuntimeClip.c_str(),
			m_SelectedResource.iPlayMs,
			m_SelectedResource.strEndPolicy.c_str());
		if (ImGui::Button("Play Preview"))
		{
			if (m_strSelectedResourceTargetAsset.empty())
				Queue_AnimationPreview(m_SelectedResource);
			else
			{
				COMPOSITION_ANIMATION_RESOURCE resource;
				resource.strTargetAssetName = m_strSelectedResourceTargetAsset;
				resource.strProfileId = m_SelectedResource.strProfileId;
				resource.strRuntimeClip = m_SelectedResource.strRuntimeClip;
				resource.iDurationMs = m_SelectedResource.iPlayMs;
				resource.strEndPolicy = m_SelectedResource.strEndPolicy;
				Queue_ModelResourcePreview(resource);
			}
		}
		ImGui::SameLine();
		const auto clipActor = CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(m_SelectedResource.strProfileId);
		const bool_t canAppendClip = m_bHasDraft && !clipActor.empty() &&
			(nullptr == selectedPattern || (patternEditable && targetActorProfile == clipActor));
		ImGui::BeginDisabled(!canAppendClip);
		if (ImGui::Button("Append as Stage"))
		{
			std::string stageId;
			std::string occurrenceId;
			std::string status;
			(void)Append_AnimationAsStage(
				m_strSelectedPatternId, m_SelectedResource,
				stageId, occurrenceId, status);
			m_strStatus = status;
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(!stageSelected);
		if (ImGui::Button("Add Animation Row"))
		{
			std::string occurrenceId, status;
			(void)Bind_Animation(m_strSelectedPatternId, m_strSelectedStageId,
				m_SelectedResource, 0u, occurrenceId, status);
			m_strStatus = status;
		}
		ImGui::EndDisabled();
		ImGui::EndDisabled();
	}
	if (!ImGui::BeginChild("##KoukuAnimationResources", ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::EndChild();
		return;
	}

	if (!m_bSharedWorkspaceActive && ImGui::TreeNodeEx("Physical Clips", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (m_ModelResources.empty())
			ImGui::TextDisabled("Refresh Animation Resources reads the installed model clip headers.");
		RenderResourceTree(m_PhysicalResourceTree, [this](const std::size_t index)
		{
			const auto& resource = m_ModelResources[index];
			ImGui::PushID(resource.strTargetAssetName.c_str());
			ImGui::PushID(resource.strRuntimeClip.c_str());
			if (ImGui::Selectable(resource.strRuntimeClip.c_str(), m_bHasSelectedResource &&
				m_strSelectedResourceTargetAsset == resource.strTargetAssetName &&
				m_SelectedResource.strRuntimeClip == resource.strRuntimeClip))
				Queue_ModelResourcePreview(resource);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s | %u ms\n%s\nClick to preview",
				resource.strTargetAssetName.c_str(), resource.iDurationMs, resource.strSourceAssetId.c_str());
			ImGui::PopID();
			ImGui::PopID();
		});
		ImGui::TreePop();
	}

	if (m_ResourceLeaves.empty())
		ImGui::TextDisabled("No extracted action matches. Action references load with the composition.");
	RenderResourceTree(m_ResourceTree, [this](const std::size_t leafIndex)
	{
		const RESOURCE_ACTION_LEAF& leaf = m_ResourceLeaves[leafIndex];
		if (leaf.bSequence)
		{
			const auto& sequence = m_SequenceResources[leaf.iAction];
			ImGui::PushID(sequence.strStableId.c_str());
			const std::string sequencePath = "SEQUENCE/" + sequence.strStableId;
			ImGui::SetNextItemOpen(m_strExpandedResourceActionId == sequencePath);
			const bool open = ImGui::TreeNodeEx("##Sequence", ImGuiTreeNodeFlags_SpanAvailWidth,
				"%s | %zu clips", sequence.strDisplayName.c_str(), sequence.Clips.size());
			if (ImGui::IsItemToggledOpen())
			{
				m_strExpandedResourceActionId = open ? sequencePath : std::string{};
				m_strExpandedResourceStageId.clear();
			}
			if (ImGui::IsItemClicked()) Queue_SequencePreview(sequence);
			if (open)
			{
				for (std::size_t index = 0u; index < sequence.Clips.size(); ++index)
				{
					const auto& clip = sequence.Clips[index];
					ImGui::PushID(static_cast<int>(index));
					if (ImGui::Selectable(clip.strRuntimeClip.c_str())) Queue_ModelResourcePreview(clip);
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("%u ms | %s", clip.iDurationMs, clip.strEndPolicy.c_str());
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
			return;
		}
		const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference =
			m_ResourceReferences.Documents[leaf.iDocument];
		const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& action = reference.Actions[leaf.iAction];
		ImGui::PushID(reference.strProfileId.c_str());
		ImGui::PushID(std::to_string(action.iSourceActionId).c_str());
		const bool_t actionSelected =
			m_iSelectedResourceActionId == action.iSourceActionId &&
			m_strSelectedResourceProfileId == reference.strProfileId;
		std::uint32_t totalMs = 0u;
		std::size_t clipCount = 0u;
		for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& stage : action.Stages)
		{
			for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot : stage.Slots)
			{
				totalMs += slot.iPlayMs;
				++clipCount;
			}
		}
		const std::string actionLabel = action.strDisplayName + " (" +
			std::to_string(action.iSourceActionId) + ") | " + std::to_string(clipCount) +
			" clips | " + std::to_string(totalMs) + " ms";
		const std::string actionPath = "ACTION/" + reference.strProfileId + "/" +
			std::to_string(action.iSourceActionId);
		ImGui::SetNextItemOpen(m_strExpandedResourceActionId == actionPath);
		const bool_t actionOpen = ImGui::TreeNodeEx("##ResourceAction",
			ImGuiTreeNodeFlags_SpanAvailWidth |
				(actionSelected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None),
			"%s", actionLabel.c_str());
		if (ImGui::IsItemToggledOpen())
		{
			m_strExpandedResourceActionId = actionOpen ? actionPath : std::string{};
			m_strExpandedResourceStageId.clear();
		}
		if (ImGui::IsItemClicked())
			Queue_ActionPreview(reference, action, nullptr);
		if (actionOpen)
		{
			for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& stage : action.Stages)
			{
				if (stage.Slots.empty())
					continue;
				ImGui::PushID(stage.strStageId.c_str());
				const std::string stagePath = actionPath + "/" + stage.strStageId;
				ImGui::SetNextItemOpen(m_strExpandedResourceStageId == stagePath);
				const bool_t stageOpen = ImGui::TreeNode(stage.strStageId.c_str());
				if (ImGui::IsItemToggledOpen())
					m_strExpandedResourceStageId = stageOpen ? stagePath : std::string{};
				if (ImGui::IsItemClicked())
					Queue_ActionPreview(reference, action, &stage);
				if (stageOpen)
				{
					for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot : stage.Slots)
					{
						const bool_t slotSelected = m_bHasSelectedResource &&
							m_SelectedResource.strProfileId == reference.strProfileId &&
							m_SelectedResource.iSourceActionId == action.iSourceActionId &&
							m_SelectedResource.strSourceStageId == stage.strStageId &&
							m_SelectedResource.strSourceSlotId == slot.strSlotId;
						const std::string slotLabel = slot.strRuntimeClip + "##" + slot.strSlotId;
						if (ImGui::Selectable(slotLabel.c_str(), slotSelected))
							Queue_SlotPreview(reference, action, stage, slot);
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("%u ms%s | click to preview", slot.iPlayMs, slot.bLoop ? " loop" : "");
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
		ImGui::PopID();
	});
	ImGui::EndChild();
}

bool_t Client::CKoukuSaydonActionWorkbench::Request_PatternPreview(
	const std::string_view patternId, const std::uint32_t startClockMs, std::string& outStatus)
{
	const auto* pattern = Find_Pattern(m_Draft, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0u)
	{
		outStatus = m_strStatus = "Select an editable Pattern with a positive lifetime to play.";
		return false;
	}
	const auto durationMs = Pattern_DurationMs(*pattern);
	m_PendingPatternPreview = *pattern;
	m_strPendingPreviewTargetAsset.clear();
	m_bPatternPreviewRequestPending = true;
	m_bPreviewRequestPending = false;
	m_iPendingPreviewStartMs = startClockMs < durationMs ? startClockMs : 0u;
	m_bPendingPreviewStartPaused = false;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
	m_strCursorPatternId = pattern->strPatternId;
	m_iCursorMs = m_iPendingPreviewStartMs;
	outStatus = m_strStatus = "Pattern preview requested from " + std::to_string(m_iCursorMs) + " ms.";
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Stop_Preview()
{
	m_bPatternPreviewRequestPending = false;
	m_bPreviewRequestPending = false;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::STOP;
	m_iCursorMs = 0u;
	m_strStatus = "Preview stop requested; the Pattern cursor is at zero.";
}

void Client::CKoukuSaydonActionWorkbench::Render_Transport()
{
	const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool patternReady = nullptr != pattern && pattern->strLoadError.empty();
	if (nullptr == pattern) ImGui::TextDisabled("Select a Pattern to play all authored lanes.");
	else if (!pattern->strLoadError.empty()) ImGui::TextWrapped("%s", pattern->strLoadError.c_str());
	const auto durationMs = patternReady ? Pattern_DurationMs(*pattern) : 0u;
	if (patternReady && m_strCursorPatternId != pattern->strPatternId)
	{
		m_strCursorPatternId = pattern->strPatternId;
		m_iCursorMs = 0u;
	}
	m_iCursorMs = (std::min)(m_iCursorMs, durationMs);
	const bool_t patternPreview = patternReady && m_PreviewState.bPlaying &&
		m_PreviewState.strPatternId == pattern->strPatternId;
	ImGui::BeginDisabled(!patternReady);
	if (ImGui::Button("Play Pattern"))
	{
		std::string status;
		(void)Request_PatternPreview(pattern->strPatternId, m_iCursorMs, status);
	}
	ImGui::SameLine();
	const auto* selectedOccurrence = patternReady ? Find_Occurrence(*pattern, m_strSelectedOccurrenceId) : nullptr;
	ImGui::BeginDisabled(nullptr == selectedOccurrence);
	if (ImGui::Button("Play Row"))
	{
		Queue_AnimationPreview(*selectedOccurrence);
	}
	ImGui::EndDisabled();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_PreviewState.bPlaying);
	if (ImGui::Button(m_PreviewState.bPaused ? "Resume" : "Pause"))
	{
		m_ePendingTransport = m_PreviewState.bPaused ?
			KOUKU_PREVIEW_TRANSPORT::RESUME : KOUKU_PREVIEW_TRANSPORT::PAUSE;
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop"))
		Stop_Preview();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (m_PreviewState.bPlaying)
	{
		ImGui::Text("%s: %u / %u ms%s", patternPreview ? "Pattern" : "Other preview",
			m_PreviewState.iClockMs, m_PreviewState.iDurationMs,
			m_PreviewState.bPaused ? " (paused)" : "");
	}
	else
	{
		ImGui::TextDisabled("cursor %u / %u ms | %zu stages", m_iCursorMs, durationMs,
			patternReady ? pattern->Stages.size() : 0u);
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Click the ruler to place the cursor or seek a running preview. Drag box edges to trim, the body to move, the Stage edge to resize.");

}

bool_t Client::CKoukuSaydonActionWorkbench::Set_PatternDuration(
	const std::string_view patternId, const std::uint32_t durationMs, std::string& outStatus)
{
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Full lifetime requires an editable Pattern.";
		return false;
	}
	if (pattern->Stages.empty())
	{
		std::string stageId;
		return Add_Stage(patternId, "ACTIVE", durationMs, stageId, outStatus);
	}
	auto& finalStage = pattern->Stages.back();
	const std::uint64_t earlierMs = Pattern_DurationMs(*pattern) - finalStage.iDurationMs;
	std::uint64_t occupiedEndMs = 1u;
	for (const auto& box : finalStage.AnimationOccurrences)
		occupiedEndMs = (std::max)(occupiedEndMs,
			static_cast<std::uint64_t>(box.iStartOffsetMs) + box.iPlayMs);
	const auto minimumMs = (std::max)((std::max)(earlierMs + occupiedEndMs,
		static_cast<std::uint64_t>(Pattern_LaneEndMs(*pattern))),
		(std::max)(static_cast<std::uint64_t>(Pattern_LogicEndMs(*pattern)),
			static_cast<std::uint64_t>(Pattern_SummonEndMs(*pattern))));
	if (durationMs < minimumMs || durationMs > MAX_EDITOR_TIME_MS)
	{
		outStatus = m_strStatus = "Full lifetime must be " + std::to_string(minimumMs) +
			"..600000 ms. Earlier Stage clocks, existing animation boxes and Logic boxes are preserved.";
		return false;
	}
	if (durationMs == Pattern_DurationMs(*pattern))
	{
		outStatus = m_strStatus = "Full lifetime is unchanged.";
		return true;
	}
	finalStage.iDurationMs = static_cast<std::uint32_t>(durationMs - earlierMs);
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
		"Updated full lifetime. Press Save to keep it and Play to preview the edited clock.", outStatus))
		return false;
	m_bFitRequested = true;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Duplicate_TimelineSelection(
	const std::string_view patternId, const std::vector<std::string>& stageIds,
	const std::vector<std::string>& occurrenceIds, std::string& outStatus)
{
	if (stageIds.empty() && occurrenceIds.empty())
	{
		outStatus = m_strStatus = "Select at least one Stage or animation box to duplicate.";
		return false;
	}
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Duplicate selection target Pattern is unavailable or invalid.";
		return false;
	}
	for (const auto& id : stageIds)
	{
		if (nullptr == Find_Stage(*pattern, id))
		{
			outStatus = m_strStatus = "Duplicate rejected; Stage is unavailable: " + id;
			return false;
		}
	}
	for (const auto& id : occurrenceIds)
	{
		if (nullptr == Find_Occurrence(*pattern, id).pOccurrence)
		{
			outStatus = m_strStatus = "Duplicate rejected; animation box is unavailable: " + id;
			return false;
		}
	}
	const std::string targetPatternId = pattern->strPatternId;
	const auto contains = [](const auto& ids, const auto& id)
		{ return std::find(ids.begin(), ids.end(), id) != ids.end(); };
	std::vector<KOUKU_SAYDON_COMPOSITION_STAGE> stages;
	std::vector<std::string> selectedStageIds, selectedOccurrenceIds;
	std::string lastStageId, lastOccurrenceId;
	const auto assignOccurrenceId = [&](auto& occurrence) -> bool_t
	{
		if (pattern->iNextAnimationOrdinal >= 1000000u)
		{
			outStatus = m_strStatus = "Duplicate rejected; animation stable ID ordinals are exhausted.";
			return false;
		}
		occurrence.strOccurrenceId = targetPatternId + ".animation." +
			std::to_string(pattern->iNextAnimationOrdinal++);
		return true;
	};
	for (const auto& sourceStage : pattern->Stages)
	{
		const bool selectedStage = contains(stageIds, sourceStage.strStageId);
		auto retainedStage = sourceStage;
		if (!selectedStage)
		{
			for (const auto& sourceOccurrence : sourceStage.AnimationOccurrences)
			{
				if (!contains(occurrenceIds, sourceOccurrence.strOccurrenceId)) continue;
				auto duplicate = sourceOccurrence;
				if (!assignOccurrenceId(duplicate)) return false;
				lastStageId = sourceStage.strStageId;
				lastOccurrenceId = duplicate.strOccurrenceId;
				selectedOccurrenceIds.push_back(lastOccurrenceId);
				retainedStage.AnimationOccurrences.push_back(std::move(duplicate));
			}
		}
		stages.push_back(std::move(retainedStage));
		if (!selectedStage) continue;
		if (pattern->iNextStageOrdinal >= 1000000u)
		{
			outStatus = m_strStatus = "Duplicate rejected; Stage stable ID ordinals are exhausted.";
			return false;
		}
		auto duplicate = sourceStage;
		const auto ordinal = std::to_string(pattern->iNextStageOrdinal++);
		duplicate.strStageId = "STAGE_" + ordinal;
		duplicate.strActionId = targetPatternId + ".stage." + ordinal;
		for (auto& occurrence : duplicate.AnimationOccurrences)
			if (!assignOccurrenceId(occurrence)) return false;
		lastStageId = duplicate.strStageId;
		lastOccurrenceId.clear();
		selectedStageIds.push_back(lastStageId);
		stages.push_back(std::move(duplicate));
	}
	pattern->Stages = std::move(stages);
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Duplicated selected boxes. Press Save.", outStatus))
		return false;
	Clear_TimelineSelection();
	m_strSelectedPatternId = targetPatternId;
	m_strTimelineSelectionPatternId = targetPatternId;
	m_TimelineSelectedStageIds = std::move(selectedStageIds);
	m_TimelineSelectedOccurrenceIds = std::move(selectedOccurrenceIds);
	m_strSelectedStageId = lastStageId;
	m_strSelectedOccurrenceId = lastOccurrenceId;
	Normalize_Selection();
	Synchronize_EditorFields();
	m_bFitRequested = true;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_TimelineSelection(
	const std::string_view patternId,
	const std::vector<std::string>& stageIds,
	const std::vector<std::string>& occurrenceIds,
	std::string& outStatus)
{
	if (stageIds.empty() && occurrenceIds.empty())
	{
		outStatus = m_strStatus = "Select at least one Stage or animation box.";
		return false;
	}
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = m_strStatus = "Delete selection target Pattern is unavailable.";
		return false;
	}
	for (const auto& id : stageIds)
	{
		if (nullptr == Find_Stage(*pattern, id))
		{
			outStatus = m_strStatus = "Delete rejected; Stage is unavailable: " + id;
			return false;
		}
	}
	for (const auto& id : occurrenceIds)
	{
		if (nullptr == Find_Occurrence(*pattern, id).pOccurrence)
		{
			outStatus = m_strStatus = "Delete rejected; animation box is unavailable: " + id;
			return false;
		}
	}
	// A selected Stage already owns its selected children; erase each object once.
	pattern->Stages.erase(std::remove_if(pattern->Stages.begin(), pattern->Stages.end(),
		[&stageIds](const auto& stage)
		{ return std::find(stageIds.begin(), stageIds.end(), stage.strStageId) != stageIds.end(); }),
		pattern->Stages.end());
	for (auto& stage : pattern->Stages)
	{
		stage.AnimationOccurrences.erase(std::remove_if(
			stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
			[&occurrenceIds](const auto& occurrence)
			{ return std::find(occurrenceIds.begin(), occurrenceIds.end(),
				occurrence.strOccurrenceId) != occurrenceIds.end(); }), stage.AnimationOccurrences.end());
	}
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Deleted selected boxes. Press Save.", outStatus))
		return false;
	Clear_TimelineSelection();
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Clear_TimelineSelection()
{
	m_strTimelineSelectionPatternId.clear();
	m_TimelineSelectedStageIds.clear();
	m_TimelineSelectedOccurrenceIds.clear();
	m_bTimelineMarqueeActive = false;
	m_iTimelineDragMode = -1;
}

void Client::CKoukuSaydonActionWorkbench::Select_TimelineBox(
	const std::string& stageId, const std::string& occurrenceId, const bool_t toggle)
{
	if (!toggle)
	{
		m_TimelineSelectedStageIds.clear();
		m_TimelineSelectedOccurrenceIds.clear();
	}
	auto& selected = occurrenceId.empty() ? m_TimelineSelectedStageIds : m_TimelineSelectedOccurrenceIds;
	const auto& id = occurrenceId.empty() ? stageId : occurrenceId;
	const auto found = std::find(selected.begin(), selected.end(), id);
	if (toggle && found != selected.end()) selected.erase(found);
	else if (found == selected.end()) selected.push_back(id);
	m_strSelectedStageId = stageId;
	m_strSelectedOccurrenceId = occurrenceId;
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
}

void Client::CKoukuSaydonActionWorkbench::Render_Timeline()
{
	if (m_strTimelineSelectionPatternId != m_strSelectedPatternId)
	{
		Clear_TimelineSelection();
		m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	}
	const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool_t patternReady = nullptr != pattern && pattern->strLoadError.empty();
	const std::string patternId = nullptr == pattern ? std::string{} : pattern->strPatternId;
	const auto durationMs = patternReady ? Pattern_DurationMs(*pattern) : 0u;
	if (m_strCursorPatternId != patternId)
	{
		m_strCursorPatternId = patternId;
		m_iCursorMs = 0u;
	}
	m_iCursorMs = (std::min)(m_iCursorMs, durationMs);
	const bool_t patternPreview = patternReady && m_PreviewState.bPlaying &&
		m_PreviewState.strPatternId == patternId;
	const bool_t canPlayPattern = patternReady && durationMs > 0u;
	const bool_t publishing = Is_PublishRunning();
	ImGui::BeginDisabled(publishing || !m_bHasDraft || !m_bDirty || !m_Document.Is_Fresh());
	const bool_t saveRequested = ImGui::Button("Save##KoukuSequencer");
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!canPlayPattern);
	if (ImGui::Button("Play##KoukuSequencer"))
	{
		std::string status;
		(void)Request_PatternPreview(patternId, m_iCursorMs, status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_PreviewState.bPlaying &&
		!m_bPatternPreviewRequestPending && !m_bPreviewRequestPending);
	if (ImGui::Button("Stop##KoukuSequencer")) Stop_Preview();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("%s | %u / %u ms%s", m_bDirty ? "Unsaved changes" : "Saved",
		patternPreview ? m_PreviewState.iClockMs : m_iCursorMs, durationMs,
		patternPreview && m_PreviewState.bPaused ? " (paused)" : "");

	ImGui::SetNextItemWidth(155.f);
	if (ImGui::SliderFloat("Zoom##KoukuSequencer", &m_fPixelsPerSecond, 1.f, 500.f, "%.1f px/s"))
		m_bFitRequested = false;
	ImGui::SameLine();
	const bool_t durationEditable = !publishing && patternReady;
	ImGui::BeginDisabled(!durationEditable);
	ImGui::SetNextItemWidth(145.f);
	ImGui::InputInt("Full lifetime ms##KoukuSequencer",
		&m_iPatternDurationMs, 100, 1000);
	bool_t durationRequested = durationEditable &&
		(ImGui::IsItemActive() || ImGui::IsItemDeactivated()) &&
		(ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
		 ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false));
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Sum of Stage clocks. Enter or Apply changes the final Stage only; existing animation windows are preserved.");
	ImGui::SameLine();
	durationRequested |= ImGui::Button("Apply##KoukuSequencerLifetime");
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Fit##KoukuSequencer")) m_bFitRequested = true;

	ImGui::TextUnformatted("Selected Box");
	ImGui::SameLine();
	const bool_t hasSelection = !m_TimelineSelectedStageIds.empty() ||
		!m_TimelineSelectedOccurrenceIds.empty() ||
		!m_strSelectedLogicOccurrenceId.empty() ||
		!m_strSelectedSummonOccurrenceId.empty() ||
		!m_strSelectedWorldOccurrenceId.empty() ||
		!m_strSelectedSceneProfileOccurrenceId.empty() ||
		!m_strSelectedPresentationOccurrenceId.empty();
	ImGui::BeginDisabled(publishing || !patternReady || !hasSelection);
	bool_t deleteRequested = ImGui::Button("Delete##KoukuSequencerSelection");
	ImGui::SameLine();
	ImGui::BeginDisabled(m_TimelineSelectedStageIds.empty() &&
		m_TimelineSelectedOccurrenceIds.empty());
	const bool_t duplicateRequested = ImGui::Button("Duplicate##KoukuSequencerSelection");
	ImGui::EndDisabled();
	ImGui::SameLine();
	/* Stage order: one Stage (or the Stage owning one animation box) moves one
	   slot; its animation boxes are Stage-relative and travel with it. */
	const bool_t singleBoxSelected =
		(1u == m_TimelineSelectedStageIds.size() && m_TimelineSelectedOccurrenceIds.empty()) ||
		(m_TimelineSelectedStageIds.empty() && 1u == m_TimelineSelectedOccurrenceIds.size());
	ImGui::BeginDisabled(!singleBoxSelected);
	int32_t moveRequested = 0;
	if (ImGui::Button("< Earlier##KoukuSequencerSelection")) moveRequested = -1;
	ImGui::SameLine();
	if (ImGui::Button("Later >##KoukuSequencerSelection")) moveRequested = 1;
	ImGui::EndDisabled();
	ImGui::EndDisabled();
	ImGui::SameLine();
	const char_t* const laneSelection = !m_strSelectedLogicOccurrenceId.empty() ? " + 1 logic box" :
		(!m_strSelectedSummonOccurrenceId.empty() ? " + 1 summon box" :
		(!m_strSelectedWorldOccurrenceId.empty() ? " + 1 world box" :
		(!m_strSelectedSceneProfileOccurrenceId.empty() ? " + 1 scene profile box" :
		(!m_strSelectedPresentationOccurrenceId.empty() ? " + 1 presentation box" : ""))));
	ImGui::TextDisabled("%zu Stages + %zu animation boxes%s",
		m_TimelineSelectedStageIds.size(), m_TimelineSelectedOccurrenceIds.size(), laneSelection);
	ImGui::TextDisabled("Click: select | Ctrl+click: toggle | drag empty space: box select | Ctrl+drag: add | Delete: remove selection | Left/Right: move the selected Stage");
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	if (saveRequested || deleteRequested || duplicateRequested || durationRequested ||
		0 != moveRequested)
	{
		std::string status;
		if (0 != moveRequested)
			(void)Move_SelectedStage(patternId, moveRequested, status);
		else if (deleteRequested && m_TimelineSelectedStageIds.empty() &&
			m_TimelineSelectedOccurrenceIds.empty() && !m_strSelectedLogicOccurrenceId.empty())
			(void)Delete_LogicBox(patternId, m_strSelectedLogicOccurrenceId, status);
		else if (deleteRequested && m_TimelineSelectedStageIds.empty() &&
			m_TimelineSelectedOccurrenceIds.empty() && !m_strSelectedSummonOccurrenceId.empty())
			(void)Delete_SummonBox(patternId, m_strSelectedSummonOccurrenceId, status);
		else if (deleteRequested && m_TimelineSelectedStageIds.empty() &&
			m_TimelineSelectedOccurrenceIds.empty() && !m_strSelectedWorldOccurrenceId.empty())
			(void)Delete_WorldBox(patternId, m_strSelectedWorldOccurrenceId, status);
		else if (deleteRequested && m_TimelineSelectedStageIds.empty() &&
			m_TimelineSelectedOccurrenceIds.empty() && !m_strSelectedSceneProfileOccurrenceId.empty())
			(void)Delete_SceneProfileBox(patternId, m_strSelectedSceneProfileOccurrenceId, status);
		else if (deleteRequested && m_TimelineSelectedStageIds.empty() &&
			m_TimelineSelectedOccurrenceIds.empty() && !m_strSelectedPresentationOccurrenceId.empty())
			(void)Delete_PresentationBox(patternId, m_strSelectedPresentationOccurrenceId, status);
		else if (deleteRequested)
			(void)Delete_TimelineSelection(patternId,
				m_TimelineSelectedStageIds, m_TimelineSelectedOccurrenceIds, status);
		else if (duplicateRequested)
			(void)Duplicate_TimelineSelection(patternId,
				m_TimelineSelectedStageIds, m_TimelineSelectedOccurrenceIds, status);
		else if (durationRequested)
			(void)Set_PatternDuration(patternId,
				static_cast<std::uint32_t>((std::max)(m_iPatternDurationMs, 0)), status);
		else
			(void)Save(status);
		// The candidate may replace every draft pointer; draw the new view next frame.
		return;
	}
	if (!patternReady)
	{
		if (nullptr == pattern) ImGui::TextDisabled("Select a Pattern.");
		else ImGui::TextWrapped("%s", pattern->strLoadError.c_str());
		return;
	}

	constexpr f32_t labelWidth = 180.f;
	constexpr f32_t rulerHeight = 24.f;
	const ImVec2 available = ImGui::GetContentRegionAvail();
	if (m_bFitRequested && durationMs > 0u)
	{
		m_fPixelsPerSecond = std::clamp((available.x - labelWidth - 24.f) * 1000.f /
			static_cast<f32_t>(durationMs), 1.f, 500.f);
		m_bFitRequested = false;
	}
	const f32_t scale = m_fPixelsPerSecond * 0.001f;
	const f32_t timelineWidth = (std::max)(available.x, labelWidth + durationMs * scale + 24.f);
	struct ANIMATION_INTERVAL
	{
		const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* occurrence;
		std::uint64_t startMs;
	};
	std::vector<ANIMATION_INTERVAL> intervals;
	std::uint64_t stageBaseMs = 0u;
	for (const auto& stage : pattern->Stages)
	{
		for (const auto& occurrence : stage.AnimationOccurrences)
			intervals.push_back({ &occurrence, stageBaseMs + occurrence.iStartOffsetMs });
		stageBaseMs += stage.iDurationMs;
	}
	std::stable_sort(intervals.begin(), intervals.end(), [](const auto& left, const auto& right)
	{
		if (left.startMs != right.startMs) return left.startMs < right.startMs;
		return left.occurrence->strOccurrenceId < right.occurrence->strOccurrenceId;
	});
	std::vector<std::uint64_t> rowEnds;
	std::unordered_map<std::string, std::size_t> occurrenceRows;
	for (const auto& interval : intervals)
	{
		std::size_t row = 0u;
		while (row < rowEnds.size() && rowEnds[row] > interval.startMs) ++row;
		const auto visibleMs = (std::max)(static_cast<std::uint64_t>(interval.occurrence->iPlayMs),
			static_cast<std::uint64_t>(std::ceil(8.f / scale)));
		if (row == rowEnds.size()) rowEnds.push_back(interval.startMs + visibleMs);
		else rowEnds[row] = interval.startMs + visibleMs;
		occurrenceRows.emplace(interval.occurrence->strOccurrenceId, row);
	}
	const std::size_t rowCount = (std::max)(std::size_t{ 1u }, rowEnds.size());
	std::array<std::size_t, 5u> presentationFamilyRows{ 1u, 1u, 1u, 1u, 1u };
	std::array<std::size_t, 5u> presentationFamilyOffsets{};
	std::unordered_map<std::string, std::size_t> presentationOccurrenceRows;
	for (std::size_t family = 0u; family < 5u; ++family)
	{
		std::vector<const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE*> boxes;
		for (const auto& box : pattern->PresentationOccurrences)
		{
			const auto* resource = Find_PresentationResource(m_Draft, box.strResourceId);
			if (nullptr != resource && static_cast<std::size_t>(resource->eKind) == family) boxes.push_back(&box);
		}
		std::stable_sort(boxes.begin(), boxes.end(), [](const auto* a, const auto* b) { return a->iStartMs < b->iStartMs; });
		std::vector<std::uint64_t> ends;
		for (const auto* box : boxes)
		{
			std::size_t row = 0u;
			while (row < ends.size() && ends[row] > box->iStartMs) ++row;
			const auto end = static_cast<std::uint64_t>(box->iStartMs) +
				(std::max)(static_cast<std::uint64_t>(box->iDurationMs), static_cast<std::uint64_t>(std::ceil(8.f / scale)));
			if (row == ends.size()) ends.push_back(end); else ends[row] = end;
			presentationOccurrenceRows.emplace(box->strOccurrenceId, row);
		}
		presentationFamilyRows[family] = (std::max)(std::size_t{1u}, ends.size());
		if (family > 0u) presentationFamilyOffsets[family] = presentationFamilyOffsets[family - 1u] + presentationFamilyRows[family - 1u];
	}
	const std::size_t presentationRowCount = presentationFamilyOffsets[4u] + presentationFamilyRows[4u];
	const f32_t height = rulerHeight + TIMELINE_LANE_HEIGHT * static_cast<f32_t>(5u + rowCount + presentationRowCount);
	if (!ImGui::BeginChild("##KoukuTimeline", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders,
		ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImGui::EndChild();
		return;
	}
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	ImDrawList* draw = ImGui::GetWindowDrawList();
	const bool_t canInteract = !publishing && !ImGui::GetIO().WantTextInput;
	struct TIMELINE_HIT_BOX
	{
		std::string stageId;
		std::string occurrenceId;
		ImVec2 min;
		ImVec2 max;
	};
	std::vector<TIMELINE_HIT_BOX> hitBoxes;
	const auto contains = [](const auto& ids, const std::string& id)
	{ return std::find(ids.begin(), ids.end(), id) != ids.end(); };
	CompositionTimeline::DrawRuler(draw, ImVec2(origin.x + labelWidth, origin.y),
		ImVec2(origin.x + timelineWidth, origin.y + rulerHeight), durationMs, m_fPixelsPerSecond);
	/* The ruler is the transport surface: press/drag scrubs a running preview
	   and otherwise places the cursor Play starts from. */
	ImGui::SetCursorScreenPos(ImVec2(origin.x + labelWidth, origin.y));
	ImGui::InvisibleButton("##KoukuRuler",
		ImVec2((std::max)(8.f, durationMs * scale), rulerHeight));
	if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActive() && durationMs > 0u)
	{
		const f32_t mouseMs = (ImGui::GetIO().MousePos.x - (origin.x + labelWidth)) / scale;
		const auto clockMs = static_cast<std::uint32_t>(std::clamp(
			std::llround(mouseMs), 0ll, static_cast<long long>(durationMs)));
		if (patternPreview)
		{
			m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::SEEK;
			m_iPendingSeekMs = clockMs;
		}
		m_iCursorMs = clockMs;
	}
	const std::uint32_t playheadMs = patternPreview ? m_PreviewState.iClockMs : m_iCursorMs;
	if (playheadMs <= durationMs)
	{
		const f32_t playheadX = origin.x + labelWidth + playheadMs * scale;
		draw->AddLine(ImVec2(playheadX, origin.y), ImVec2(playheadX, origin.y + height),
			patternPreview ? IM_COL32(255, 220, 72, 230) : IM_COL32(200, 200, 200, 140), 1.5f);
	}
	draw->AddText(ImVec2(origin.x + 4.f, origin.y + rulerHeight + 4.f), IM_COL32_WHITE, "Stages");
	draw->AddText(ImVec2(origin.x + 4.f, origin.y + rulerHeight + TIMELINE_LANE_HEIGHT + 4.f),
		IM_COL32(240, 188, 98, 255), "Animation");
	for (std::size_t row = 1u; row < rowCount; ++row)
	{
		const std::string label = "Overlap " + std::to_string(row + 1u);
		draw->AddText(ImVec2(origin.x + 4.f,
			origin.y + rulerHeight + TIMELINE_LANE_HEIGHT * static_cast<f32_t>(1u + row) + 4.f),
			IM_COL32(180, 180, 190, 255), label.c_str());
	}

	const f32_t logicLaneY = origin.y + rulerHeight +
		TIMELINE_LANE_HEIGHT * static_cast<f32_t>(1u + rowCount);
	draw->AddText(ImVec2(origin.x + 4.f, logicLaneY + 4.f),
		IM_COL32(236, 170, 110, 255), "Logic");

	std::string editStageId, editOccurrenceId, editLogicBoxId;
	std::uint32_t newOffset = 0u, newSourceStart = 0u, newPlayMs = 0u, newStageDuration = 0u;
	std::uint32_t newLogicStartMs = 0u, newLogicDurationMs = 0u;
	bool_t deleteLogicRequested = false;
	const f32_t summonLaneY = logicLaneY + TIMELINE_LANE_HEIGHT;
	draw->AddText(ImVec2(origin.x + 4.f, summonLaneY + 4.f),
		IM_COL32(150, 220, 180, 255), "Summon");
	std::string editSummonBoxId;
	std::uint32_t newSummonStartMs = 0u, newSummonDurationMs = 0u;
	bool_t deleteSummonRequested = false;
	const f32_t worldLaneY = summonLaneY + TIMELINE_LANE_HEIGHT;
	draw->AddText(ImVec2(origin.x + 4.f, worldLaneY + 4.f),
		IM_COL32(150, 190, 240, 255), "World");
	std::string editWorldBoxId;
	std::uint32_t newWorldStartMs = 0u, newWorldDurationMs = 0u;
	bool_t deleteWorldRequested = false;
	const f32_t sceneLaneY = worldLaneY + TIMELINE_LANE_HEIGHT;
	draw->AddText(ImVec2(origin.x + 4.f, sceneLaneY + 4.f),
		IM_COL32(200, 170, 240, 255), "Scene Profile");
	std::string editSceneBoxId;
	std::uint32_t newSceneStartMs = 0u, newSceneDurationMs = 0u;
	bool_t deleteSceneRequested = false;
	std::string editPresentationBoxId;
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE editedPresentationBox;
	bool_t deletePresentationRequested = false;
	for (int i = 0; i < 5; ++i)
		draw->AddText(ImVec2(origin.x + 4.f, sceneLaneY + static_cast<float>(presentationFamilyOffsets[i] + 1u) * TIMELINE_LANE_HEIGHT + 4.f),
			IM_COL32(180, 210, 200, 255), Presentation_Label(static_cast<KOUKU_SAYDON_PRESENTATION_KIND>(i)));
	int32_t moveStageDirection = 0;
	std::uint32_t stageStartMs = 0u;
	for (const auto& stage : pattern->Stages)
	{
		const f32_t stageX = origin.x + labelWidth + stageStartMs * scale;
		const f32_t stageY = origin.y + rulerHeight;
		const f32_t stageWidth = (std::max)(8.f, stage.iDurationMs * scale);
		ImGui::PushID(stage.strStageId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(stageX, stageY));
		ImGui::InvisibleButton("##StageBox", ImVec2(stageWidth, TIMELINE_LANE_HEIGHT - 2.f));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = stage.iDurationMs;
			m_iTimelineDragMode = ImGui::GetIO().MousePos.x > stageX + stageWidth - 7.f ? 3 : 0;
			Select_TimelineBox(stage.strStageId, {}, ImGui::GetIO().KeyCtrl);
			if (ImGui::GetIO().KeyCtrl) m_iTimelineDragMode = -1;
		}
		f32_t shownStageWidth = stageWidth;
		if (canInteract && !m_bTimelineMarqueeActive &&
			(ImGui::IsItemActive() || ImGui::IsItemDeactivated()) && 3 == m_iTimelineDragMode)
		{
			std::uint32_t occupiedEnd = 1u;
			for (const auto& box : stage.AnimationOccurrences)
				occupiedEnd = (std::max)(occupiedEnd, box.iStartOffsetMs + box.iPlayMs);
			const auto delta = static_cast<int64_t>(std::llround(ImGui::GetMouseDragDelta().x / scale));
			const auto changed = static_cast<std::uint32_t>(std::clamp(
				static_cast<int64_t>(m_iDragOriginOffsetMs) + delta,
				static_cast<int64_t>(occupiedEnd), static_cast<int64_t>(MAX_EDITOR_TIME_MS)));
			shownStageWidth = (std::max)(8.f, changed * scale);
			if (ImGui::IsItemDeactivated() && changed != stage.iDurationMs)
			{
				editStageId = stage.strStageId;
				newStageDuration = changed;
			}
		}
		CompositionTimeline::DrawBox(draw, ImVec2(stageX, stageY),
			ImVec2(stageX + shownStageWidth, stageY + 22.f), IM_COL32(96, 96, 112, 255),
			contains(m_TimelineSelectedStageIds, stage.strStageId), stage.strStageId.c_str(), false, true);
		hitBoxes.push_back({ stage.strStageId, {}, ImVec2(stageX, stageY),
			ImVec2(stageX + shownStageWidth, stageY + 22.f) });
		ImGui::PopID();

		for (const auto& occurrence : stage.AnimationOccurrences)
		{
			const f32_t y = origin.y + rulerHeight + TIMELINE_LANE_HEIGHT *
				static_cast<f32_t>(1u + occurrenceRows.at(occurrence.strOccurrenceId));
			const f32_t x = stageX + occurrence.iStartOffsetMs * scale;
			const f32_t width = (std::max)(8.f, occurrence.iPlayMs * scale);
			ImGui::PushID(occurrence.strOccurrenceId.c_str());
			ImGui::SetCursorScreenPos(ImVec2(x, y));
			ImGui::InvisibleButton("##AnimationBox", ImVec2(width, 22.f));
			if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
			{
				m_iDragOriginOffsetMs = occurrence.iStartOffsetMs;
				m_iDragOriginSourceMs = occurrence.iSourceStartMs;
				m_iDragOriginPlayMs = occurrence.iPlayMs;
				const f32_t mouse = ImGui::GetIO().MousePos.x;
				const auto gesture = CompositionTimeline::HitBoxGesture(mouse, x, x + width, 6.f, true, true);
				m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START ? 1 :
					(gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
				Select_TimelineBox(stage.strStageId, occurrence.strOccurrenceId, ImGui::GetIO().KeyCtrl);
				if (ImGui::GetIO().KeyCtrl) m_iTimelineDragMode = -1;
			}
			f32_t shownX = x, shownWidth = width;
			if (canInteract && !m_bTimelineMarqueeActive && m_iTimelineDragMode >= 0 &&
				(ImGui::IsItemActive() || ImGui::IsItemDeactivated()))
			{
				int64_t offset = m_iDragOriginOffsetMs, source = m_iDragOriginSourceMs, play = m_iDragOriginPlayMs;
				const auto delta = static_cast<int64_t>(std::llround(ImGui::GetMouseDragDelta().x / scale));
				if (1 == m_iTimelineDragMode)
				{
					const auto minDelta = (std::max)(-offset,
						-static_cast<int64_t>(std::floor(source / static_cast<double>(occurrence.fPlayRate))));
					int64_t maxSourceMs = MAX_EDITOR_TIME_MS;
					std::uint32_t nativeMs = 0u;
					if ("HOLD_LAST_POSE" != occurrence.strEndPolicy && Resolve_NativeClipMs(occurrence, nativeMs))
						maxSourceMs = (std::min)(maxSourceMs, static_cast<int64_t>(nativeMs) - 1);
					const auto maxDelta = (std::max)(minDelta, (std::min)(play - 1,
						static_cast<int64_t>(std::floor((maxSourceMs - source) / static_cast<double>(occurrence.fPlayRate)))));
					const auto trim = std::clamp(delta, minDelta, maxDelta);
					offset += trim;
					source += static_cast<int64_t>(std::llround(trim * occurrence.fPlayRate));
					play -= trim;
				}
				else if (2 == m_iTimelineDragMode)
					play = std::clamp(play + delta, int64_t{1}, static_cast<int64_t>(stage.iDurationMs) - offset);
				else
					offset = std::clamp(offset + delta, int64_t{0}, static_cast<int64_t>(stage.iDurationMs) - play);
				shownX = stageX + static_cast<f32_t>(offset) * scale;
				shownWidth = (std::max)(8.f, static_cast<f32_t>(play) * scale);
				if (ImGui::IsItemDeactivated() && (offset != occurrence.iStartOffsetMs ||
					source != occurrence.iSourceStartMs || play != occurrence.iPlayMs))
				{
					editOccurrenceId = occurrence.strOccurrenceId;
					newOffset = static_cast<std::uint32_t>(offset);
					newSourceStart = static_cast<std::uint32_t>(source);
					newPlayMs = static_cast<std::uint32_t>(play);
				}
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s\n%s | offset %u ms | source %u ms | play %u ms | x%.2f | %s",
					occurrence.strOccurrenceId.c_str(), stage.strStageId.c_str(),
					occurrence.iStartOffsetMs, occurrence.iSourceStartMs, occurrence.iPlayMs,
					occurrence.fPlayRate, occurrence.strEndPolicy.c_str());
			CompositionTimeline::DrawBox(draw, ImVec2(shownX, y),
				ImVec2(shownX + shownWidth, y + 22.f), IM_COL32(72, 128, 200, 255),
				contains(m_TimelineSelectedOccurrenceIds, occurrence.strOccurrenceId) ||
				contains(m_TimelineSelectedStageIds, stage.strStageId), occurrence.strRuntimeClip.c_str());
			hitBoxes.push_back({ stage.strStageId, occurrence.strOccurrenceId, ImVec2(shownX, y),
				ImVec2(shownX + shownWidth, y + 22.f) });
			ImGui::PopID();
		}
		stageStartMs += stage.iDurationMs;
	}
	/* Logic boxes live on one pattern-relative lane so a window can span the
	   repeated clips it judges. Marquee selection ignores them on purpose. */
	for (const auto& box : pattern->LogicOccurrences)
	{
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, logicLaneY));
		ImGui::InvisibleButton("##LogicBox", ImVec2(width, 22.f));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture = CompositionTimeline::HitBoxGesture(
				ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START ? 1 :
				(gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			m_TimelineSelectedStageIds.clear();
			m_TimelineSelectedOccurrenceIds.clear();
			m_strSelectedStageId.clear();
			m_strSelectedOccurrenceId.clear();
			m_strSelectedLogicOccurrenceId = box.strOccurrenceId;
			m_strSelectedSummonOccurrenceId.clear();
			m_strSelectedWorldOccurrenceId.clear();
			m_strSelectedSceneProfileOccurrenceId.clear();
			m_strSelectedPresentationOccurrenceId.clear();
			m_strTimelineSelectionPatternId = patternId;
			Synchronize_EditorFields();
		}
		f32_t shownX = x, shownWidth = width;
		if (canInteract && !m_bTimelineMarqueeActive && m_iTimelineDragMode >= 0 &&
			(ImGui::IsItemActive() || ImGui::IsItemDeactivated()))
		{
			int64_t start = m_iDragOriginOffsetMs, length = m_iDragOriginPlayMs;
			const auto delta = static_cast<int64_t>(std::llround(ImGui::GetMouseDragDelta().x / scale));
			const auto lifetime = static_cast<int64_t>(durationMs);
			if (1 == m_iTimelineDragMode)
			{
				const auto trim = std::clamp(delta, -start, length - 1);
				start += trim;
				length -= trim;
			}
			else if (2 == m_iTimelineDragMode)
				length = std::clamp(length + delta, int64_t{1}, (std::max)(int64_t{1}, lifetime - start));
			else
				start = std::clamp(start + delta, int64_t{0}, (std::max)(int64_t{0}, lifetime - length));
			shownX = origin.x + labelWidth + static_cast<f32_t>(start) * scale;
			shownWidth = (std::max)(8.f, static_cast<f32_t>(length) * scale);
			if (ImGui::IsItemDeactivated() &&
				(start != box.iStartMs || length != box.iDurationMs))
			{
				editLogicBoxId = box.strOccurrenceId;
				newLogicStartMs = static_cast<std::uint32_t>(start);
				newLogicDurationMs = static_cast<std::uint32_t>(length);
			}
		}
		const auto* const logic = Find_Logic(m_Draft, box.strLogicId);
		std::string label = nullptr == logic ? box.strLogicId :
			logic->strDisplayName + " [" + logic->strLogicType + "]";
		for (const KOUKU_SAYDON_OUTCOME_SLOT slot : {
			KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS, KOUKU_SAYDON_OUTCOME_SLOT::FAIL,
			KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT })
		{
			const std::vector<std::string>& targets = box.Outcomes(slot);
			if (targets.empty())
				continue;
			const auto* const first = Find_Logic(m_Draft, targets.front());
			label += std::string("  ") + Outcome_SlotLabel(slot) + ">" +
				(nullptr == first ? targets.front() : first->strDisplayName) +
				(targets.size() > 1u ? " +" + std::to_string(targets.size() - 1u) : "");
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\n%s | start %u ms | lifetime %u ms",
				box.strOccurrenceId.c_str(), label.c_str(), box.iStartMs, box.iDurationMs);
		CompositionTimeline::DrawBox(draw, ImVec2(shownX, logicLaneY),
			ImVec2(shownX + shownWidth, logicLaneY + 22.f), TIMELINE_LOGIC_COLOR,
			m_strSelectedLogicOccurrenceId == box.strOccurrenceId, label.c_str());
		ImGui::PopID();
	}
	/* Summon boxes: spawn at the left edge, despawn at the right edge. */
	for (const auto& box : pattern->SummonOccurrences)
	{
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, summonLaneY));
		ImGui::InvisibleButton("##SummonBox", ImVec2(width, 22.f));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture = CompositionTimeline::HitBoxGesture(
				ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START ? 1 :
				(gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			m_TimelineSelectedStageIds.clear();
			m_TimelineSelectedOccurrenceIds.clear();
			m_strSelectedStageId.clear();
			m_strSelectedOccurrenceId.clear();
			m_strSelectedLogicOccurrenceId.clear();
			m_strSelectedSummonOccurrenceId = box.strOccurrenceId;
			m_strSelectedWorldOccurrenceId.clear();
			m_strSelectedSceneProfileOccurrenceId.clear();
			m_strSelectedPresentationOccurrenceId.clear();
			m_strTimelineSelectionPatternId = patternId;
			Synchronize_EditorFields();
		}
		f32_t shownX = x, shownWidth = width;
		if (canInteract && !m_bTimelineMarqueeActive && m_iTimelineDragMode >= 0 &&
			(ImGui::IsItemActive() || ImGui::IsItemDeactivated()))
		{
			int64_t start = m_iDragOriginOffsetMs, length = m_iDragOriginPlayMs;
			const auto delta = static_cast<int64_t>(std::llround(ImGui::GetMouseDragDelta().x / scale));
			const auto lifetime = static_cast<int64_t>(durationMs);
			if (1 == m_iTimelineDragMode)
			{
				const auto trim = std::clamp(delta, -start, length - 1);
				start += trim;
				length -= trim;
			}
			else if (2 == m_iTimelineDragMode)
				length = std::clamp(length + delta, int64_t{1}, (std::max)(int64_t{1}, lifetime - start));
			else
				start = std::clamp(start + delta, int64_t{0}, (std::max)(int64_t{0}, lifetime - length));
			shownX = origin.x + labelWidth + static_cast<f32_t>(start) * scale;
			shownWidth = (std::max)(8.f, static_cast<f32_t>(length) * scale);
			if (ImGui::IsItemDeactivated() &&
				(start != box.iStartMs || length != box.iDurationMs))
			{
				editSummonBoxId = box.strOccurrenceId;
				newSummonStartMs = static_cast<std::uint32_t>(start);
				newSummonDurationMs = static_cast<std::uint32_t>(length);
			}
		}
		const auto* const summon = Find_Summon(m_Draft, box.strSummonId);
		const std::string label = nullptr == summon ? box.strSummonId : summon->strDisplayName;
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\n%s | spawn %u ms | lifetime %u ms",
				box.strOccurrenceId.c_str(), label.c_str(), box.iStartMs, box.iDurationMs);
		CompositionTimeline::DrawBox(draw, ImVec2(shownX, summonLaneY),
			ImVec2(shownX + shownWidth, summonLaneY + 22.f), TIMELINE_SUMMON_COLOR,
			m_strSelectedSummonOccurrenceId == box.strOccurrenceId, label.c_str());
		ImGui::PopID();
	}
	/* World boxes: the sequence starts at the left edge; the width is the
	   authored shown span. Scene Profile boxes: the profile holds for the box. */
	for (const auto& box : pattern->WorldOccurrences)
	{
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, worldLaneY));
		ImGui::InvisibleButton("##WorldBox", ImVec2(width, 22.f));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture = CompositionTimeline::HitBoxGesture(
				ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START ? 1 :
				(gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			m_TimelineSelectedStageIds.clear();
			m_TimelineSelectedOccurrenceIds.clear();
			m_strSelectedStageId.clear();
			m_strSelectedOccurrenceId.clear();
			m_strSelectedLogicOccurrenceId.clear();
			m_strSelectedSummonOccurrenceId.clear();
			m_strSelectedWorldOccurrenceId = box.strOccurrenceId;
			m_strSelectedSceneProfileOccurrenceId.clear();
			m_strSelectedPresentationOccurrenceId.clear();
			m_strTimelineSelectionPatternId = patternId;
			Synchronize_EditorFields();
		}
		f32_t shownX = x, shownWidth = width;
		if (canInteract && !m_bTimelineMarqueeActive && m_iTimelineDragMode >= 0 &&
			(ImGui::IsItemActive() || ImGui::IsItemDeactivated()))
		{
			int64_t start = m_iDragOriginOffsetMs, length = m_iDragOriginPlayMs;
			const auto delta = static_cast<int64_t>(std::llround(ImGui::GetMouseDragDelta().x / scale));
			const auto lifetime = static_cast<int64_t>(durationMs);
			if (1 == m_iTimelineDragMode)
			{
				const auto trim = std::clamp(delta, -start, length - 1);
				start += trim;
				length -= trim;
			}
			else if (2 == m_iTimelineDragMode)
				length = (std::max)(int64_t{1}, length + delta);
			else
				start = std::clamp(start + delta, int64_t{0}, lifetime);
			shownX = origin.x + labelWidth + static_cast<f32_t>(start) * scale;
			shownWidth = (std::max)(8.f, static_cast<f32_t>(length) * scale);
			if (ImGui::IsItemDeactivated() &&
				(start != box.iStartMs || length != box.iDurationMs))
			{
				editWorldBoxId = box.strOccurrenceId;
				newWorldStartMs = static_cast<std::uint32_t>(start);
				newWorldDurationMs = static_cast<std::uint32_t>(length);
			}
		}
		const auto* const world = Find_World(m_Draft, box.strWorldId);
		char speedText[32]{};
		(void)snprintf(speedText, sizeof(speedText), " x%.2f", box.fPlaybackSpeed);
		const std::string label = (nullptr == world ? box.strWorldId : world->strDisplayName) + speedText;
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\n%s | start %u ms | shown %u ms | speed x%.2f",
				box.strOccurrenceId.c_str(), label.c_str(), box.iStartMs, box.iDurationMs, box.fPlaybackSpeed);
		CompositionTimeline::DrawBox(draw, ImVec2(shownX, worldLaneY),
			ImVec2(shownX + shownWidth, worldLaneY + 22.f), TIMELINE_WORLD_COLOR,
			m_strSelectedWorldOccurrenceId == box.strOccurrenceId, label.c_str());
		ImGui::PopID();
	}
	for (const auto& box : pattern->SceneProfileOccurrences)
	{
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, sceneLaneY));
		ImGui::InvisibleButton("##SceneProfileBox", ImVec2(width, 22.f));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture = CompositionTimeline::HitBoxGesture(
				ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START ? 1 :
				(gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			m_TimelineSelectedStageIds.clear();
			m_TimelineSelectedOccurrenceIds.clear();
			m_strSelectedStageId.clear();
			m_strSelectedOccurrenceId.clear();
			m_strSelectedLogicOccurrenceId.clear();
			m_strSelectedSummonOccurrenceId.clear();
			m_strSelectedWorldOccurrenceId.clear();
			m_strSelectedSceneProfileOccurrenceId = box.strOccurrenceId;
			m_strTimelineSelectionPatternId = patternId;
			Synchronize_EditorFields();
		}
		f32_t shownX = x, shownWidth = width;
		if (canInteract && !m_bTimelineMarqueeActive && m_iTimelineDragMode >= 0 &&
			(ImGui::IsItemActive() || ImGui::IsItemDeactivated()))
		{
			int64_t start = m_iDragOriginOffsetMs, length = m_iDragOriginPlayMs;
			const auto delta = static_cast<int64_t>(std::llround(ImGui::GetMouseDragDelta().x / scale));
			const auto lifetime = static_cast<int64_t>(durationMs);
			if (1 == m_iTimelineDragMode)
			{
				const auto trim = std::clamp(delta, -start, length - 1);
				start += trim;
				length -= trim;
			}
			else if (2 == m_iTimelineDragMode)
				length = std::clamp(length + delta, int64_t{1}, (std::max)(int64_t{1}, lifetime - start));
			else
				start = std::clamp(start + delta, int64_t{0}, (std::max)(int64_t{0}, lifetime - length));
			shownX = origin.x + labelWidth + static_cast<f32_t>(start) * scale;
			shownWidth = (std::max)(8.f, static_cast<f32_t>(length) * scale);
			if (ImGui::IsItemDeactivated() &&
				(start != box.iStartMs || length != box.iDurationMs))
			{
				editSceneBoxId = box.strOccurrenceId;
				newSceneStartMs = static_cast<std::uint32_t>(start);
				newSceneDurationMs = static_cast<std::uint32_t>(length);
			}
		}
		const auto* const profile = Find_SceneProfile(m_Draft, box.strSceneProfileId);
		const std::string label = nullptr == profile ? box.strSceneProfileId : profile->strDisplayName;
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\n%s | start %u ms | lifetime %u ms | blend %u ms",
				box.strOccurrenceId.c_str(), label.c_str(), box.iStartMs, box.iDurationMs, box.iBlendMs);
		CompositionTimeline::DrawBox(draw, ImVec2(shownX, sceneLaneY),
			ImVec2(shownX + shownWidth, sceneLaneY + 22.f), TIMELINE_SCENE_PROFILE_COLOR,
			m_strSelectedSceneProfileOccurrenceId == box.strOccurrenceId, label.c_str());
		ImGui::PopID();
	}
	for (const auto& box : pattern->PresentationOccurrences)
	{
		const auto* resource = Find_PresentationResource(m_Draft, box.strResourceId);
		if (nullptr == resource) continue;
		const f32_t presentationLaneY = sceneLaneY +
			static_cast<float>(1u + presentationFamilyOffsets[static_cast<std::size_t>(resource->eKind)] +
				presentationOccurrenceRows.at(box.strOccurrenceId)) * TIMELINE_LANE_HEIGHT;
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, presentationLaneY));
		ImGui::InvisibleButton("##PresentationBox", ImVec2(width, 22.f));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture = CompositionTimeline::HitBoxGesture(
				ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START ? 1 :
				(gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			m_TimelineSelectedStageIds.clear();
			m_TimelineSelectedOccurrenceIds.clear();
			m_strSelectedStageId.clear();
			m_strSelectedOccurrenceId.clear();
			m_strSelectedLogicOccurrenceId.clear();
			m_strSelectedSummonOccurrenceId.clear();
			m_strSelectedWorldOccurrenceId.clear();
			m_strSelectedSceneProfileOccurrenceId.clear();
			m_strSelectedPresentationOccurrenceId = box.strOccurrenceId;
			m_strTimelineSelectionPatternId = patternId;
			Synchronize_EditorFields();
		}
		f32_t shownX = x, shownWidth = width;
		if (canInteract && !m_bTimelineMarqueeActive && m_iTimelineDragMode >= 0 &&
			(ImGui::IsItemActive() || ImGui::IsItemDeactivated()))
		{
			int64_t start = m_iDragOriginOffsetMs, length = m_iDragOriginPlayMs;
			const auto delta = static_cast<int64_t>(std::llround(ImGui::GetMouseDragDelta().x / scale));
			const auto lifetime = static_cast<int64_t>(durationMs);
			if (1 == m_iTimelineDragMode)
			{
				const auto trim = std::clamp(delta, -start, length - 1);
				start += trim;
				length -= trim;
			}
			else if (2 == m_iTimelineDragMode)
				length = std::clamp(length + delta, int64_t{1}, (std::max)(int64_t{1}, lifetime - start));
			else
				start = std::clamp(start + delta, int64_t{0}, (std::max)(int64_t{0}, lifetime - length));
			shownX = origin.x + labelWidth + static_cast<f32_t>(start) * scale;
			shownWidth = (std::max)(8.f, static_cast<f32_t>(length) * scale);
			if (ImGui::IsItemDeactivated() &&
				(start != box.iStartMs || length != box.iDurationMs))
			{
				editPresentationBoxId = box.strOccurrenceId;
				editedPresentationBox = box;
				editedPresentationBox.iStartMs = static_cast<std::uint32_t>(start);
				editedPresentationBox.iDurationMs = static_cast<std::uint32_t>(length);
				editedPresentationBox.iFadeInMs = (std::min)(box.iFadeInMs, editedPresentationBox.iDurationMs);
				editedPresentationBox.iFadeOutMs = (std::min)(box.iFadeOutMs,
					editedPresentationBox.iDurationMs - editedPresentationBox.iFadeInMs);
			}
		}
		const std::string label = resource->strDisplayName;
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\n%s | start %u ms | lifetime %u ms",
				box.strOccurrenceId.c_str(), label.c_str(), box.iStartMs, box.iDurationMs);
		CompositionTimeline::DrawBox(draw, ImVec2(shownX, presentationLaneY),
			ImVec2(shownX + shownWidth, presentationLaneY + 22.f), IM_COL32(94, 165, 151, 255),
			m_strSelectedPresentationOccurrenceId == box.strOccurrenceId, label.c_str());
		ImGui::PopID();
	}
	if (!canInteract)
	{
		m_bTimelineMarqueeActive = false;
		m_iTimelineDragMode = -1;
	}
	// Submit the background last: existing boxes/ruler win hit testing, while
	// an empty-space press owns an active item instead of dragging the window.
	ImGui::SetCursorScreenPos(origin);
	const f32_t canvasHeight = (std::max)(height + 48.f,
		ImGui::GetWindowSize().y + ImGui::GetScrollY());
	ImGui::InvisibleButton("##KoukuMarquee", ImVec2(timelineWidth, canvasHeight));
	if (canInteract && ImGui::IsItemActivated())
	{
		if (!ImGui::GetIO().KeyCtrl)
		{
			m_TimelineSelectedStageIds.clear();
			m_TimelineSelectedOccurrenceIds.clear();
			m_strSelectedStageId.clear();
			m_strSelectedOccurrenceId.clear();
			m_strSelectedLogicOccurrenceId.clear();
			m_strSelectedSummonOccurrenceId.clear();
			m_strSelectedWorldOccurrenceId.clear();
			m_strSelectedSceneProfileOccurrenceId.clear();
			m_strSelectedPresentationOccurrenceId.clear();
			Synchronize_EditorFields();
		}
		m_bTimelineMarqueeActive = true;
		m_fTimelineMarqueeStartX = ImGui::GetIO().MousePos.x - origin.x;
		m_fTimelineMarqueeStartY = ImGui::GetIO().MousePos.y - origin.y;
		m_iTimelineDragMode = -1;
	}
	if (m_bTimelineMarqueeActive)
	{
		const ImVec2 start(origin.x + m_fTimelineMarqueeStartX, origin.y + m_fTimelineMarqueeStartY);
		const ImVec2 cursor = ImGui::GetIO().MousePos;
		const ImVec2 marqueeMin((std::min)(start.x, cursor.x), (std::min)(start.y, cursor.y));
		const ImVec2 marqueeMax((std::max)(start.x, cursor.x), (std::max)(start.y, cursor.y));
		draw->AddRectFilled(marqueeMin, marqueeMax, IM_COL32(90, 160, 240, 35));
		draw->AddRect(marqueeMin, marqueeMax, IM_COL32(110, 185, 255, 230));
		const bool_t released = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
		for (const auto& box : hitBoxes)
		{
			if (marqueeMax.x <= box.min.x || marqueeMin.x >= box.max.x ||
				marqueeMax.y <= box.min.y || marqueeMin.y >= box.max.y)
				continue;
			draw->AddRect(box.min, box.max, IM_COL32(255, 224, 92, 255), 3.f, 0, 2.f);
			if (released)
			{
				auto& selected = box.occurrenceId.empty() ?
					m_TimelineSelectedStageIds : m_TimelineSelectedOccurrenceIds;
				const auto& id = box.occurrenceId.empty() ? box.stageId : box.occurrenceId;
				if (!contains(selected, id)) selected.push_back(id);
				m_strSelectedStageId = box.stageId;
				m_strSelectedOccurrenceId = box.occurrenceId;
			}
		}
		if (released)
		{
			m_bTimelineMarqueeActive = false;
			Synchronize_EditorFields();
		}
		else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
			m_bTimelineMarqueeActive = false;
	}
	if (canInteract && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
		!m_bTimelineMarqueeActive && !ImGui::IsAnyItemActive() &&
		ImGui::IsKeyPressed(ImGuiKey_Delete, false))
	{
		if (!m_TimelineSelectedStageIds.empty() || !m_TimelineSelectedOccurrenceIds.empty())
			deleteRequested = true;
		else if (!m_strSelectedLogicOccurrenceId.empty())
			deleteLogicRequested = true;
		else if (!m_strSelectedSummonOccurrenceId.empty())
			deleteSummonRequested = true;
		else if (!m_strSelectedWorldOccurrenceId.empty())
			deleteWorldRequested = true;
		else if (!m_strSelectedSceneProfileOccurrenceId.empty())
			deleteSceneRequested = true;
		else if (!m_strSelectedPresentationOccurrenceId.empty())
			deletePresentationRequested = true;
	}
	if (canInteract && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
		!m_bTimelineMarqueeActive && !ImGui::IsAnyItemActive() &&
		(!m_TimelineSelectedStageIds.empty() || !m_TimelineSelectedOccurrenceIds.empty()))
	{
		if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false))
			moveStageDirection = -1;
		else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, false))
			moveStageDirection = 1;
	}
	ImGui::SetCursorScreenPos(origin);
	ImGui::Dummy(ImVec2(timelineWidth, height + 48.f));
	ImGui::EndChild();
	std::string status;
	if (deleteRequested && !m_strSelectedPresentationOccurrenceId.empty())
		(void)Delete_PresentationBox(patternId, m_strSelectedPresentationOccurrenceId, status);
	else if (deleteRequested)
	{
		// No frame-local Pattern, Stage or occurrence pointer is consumed after commit.
		(void)Delete_TimelineSelection(patternId,
			m_TimelineSelectedStageIds, m_TimelineSelectedOccurrenceIds, status);
	}
	else if (0 != moveStageDirection)
		(void)Move_SelectedStage(patternId, moveStageDirection, status);
	else if (deleteLogicRequested)
		(void)Delete_LogicBox(patternId, m_strSelectedLogicOccurrenceId, status);
	else if (!editLogicBoxId.empty())
		(void)Set_LogicBoxWindow(patternId, editLogicBoxId, newLogicStartMs, newLogicDurationMs, status);
	else if (deleteSummonRequested)
		(void)Delete_SummonBox(patternId, m_strSelectedSummonOccurrenceId, status);
	else if (!editSummonBoxId.empty())
		(void)Set_SummonBoxWindow(patternId, editSummonBoxId, newSummonStartMs, newSummonDurationMs, status);
	else if (deleteWorldRequested)
		(void)Delete_WorldBox(patternId, m_strSelectedWorldOccurrenceId, status);
	else if (!editWorldBoxId.empty())
	{
		const auto* const edited = Find_WorldBox(*pattern, editWorldBoxId);
		(void)Set_WorldBoxWindow(patternId, editWorldBoxId, newWorldStartMs, newWorldDurationMs,
			nullptr == edited ? 1.f : edited->fPlaybackSpeed, status);
	}
	else if (deleteSceneRequested)
		(void)Delete_SceneProfileBox(patternId, m_strSelectedSceneProfileOccurrenceId, status);
	else if (!editSceneBoxId.empty())
	{
		const auto* const edited = Find_SceneProfileBox(*pattern, editSceneBoxId);
		(void)Set_SceneProfileBoxWindow(patternId, editSceneBoxId, newSceneStartMs, newSceneDurationMs,
			nullptr == edited ? 500u : edited->iBlendMs, status);
	}
	else if (deletePresentationRequested)
		(void)Delete_PresentationBox(patternId, m_strSelectedPresentationOccurrenceId, status);
	else if (!editPresentationBoxId.empty())
		(void)Set_PresentationBox(patternId, editedPresentationBox, status);
	else if (!editStageId.empty())
		(void)Set_StageDuration(m_strSelectedPatternId, editStageId, newStageDuration, status);
	else if (!editOccurrenceId.empty())
	{
		auto candidate = m_Draft;
		auto* editedPattern = Find_Pattern(candidate, m_strSelectedPatternId);
		const auto found = Find_Occurrence(*editedPattern, editOccurrenceId);
		found.pOccurrence->iStartOffsetMs = newOffset;
		found.pOccurrence->iSourceStartMs = newSourceStart;
		found.pOccurrence->iPlayMs = newPlayMs;
		std::string policyNote;
		(void)Normalize_EndPolicyForWindow(*found.pOccurrence, policyNote);
		Mark_Draft(candidate, *editedPattern);
		(void)Commit_Candidate(std::move(candidate), "Updated animation box timing." + policyNote, status);
	}
}

bool_t Client::CKoukuSaydonActionWorkbench::Create_Logic(
	const std::string_view displayName,
	const std::string_view logicType,
	std::string& outLogicId,
	std::string& outStatus)
{
	if (!m_bHasDraft || m_Draft.iNextLogicOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Logic creation requires a loaded draft and an available ordinal.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION logic;
	logic.strLogicId = "kakulsaydon.g1.logic." + std::to_string(candidate.iNextLogicOrdinal++);
	logic.strDisplayName = std::string(displayName);
	logic.strLogicType = std::string(logicType);
	const std::string logicId = logic.strLogicId;
	candidate.Logics.push_back(std::move(logic));
	if (!Commit_Candidate(std::move(candidate),
			"Created Logic " + logicId + ". Select it and press Append Logic at Cursor.", outStatus))
	{
		return false;
	}
	m_strSelectedLogicId = logicId;
	outLogicId = logicId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_Logic(
	const std::string_view logicId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	const auto found = std::find_if(candidate.Logics.begin(), candidate.Logics.end(),
		[logicId](const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic)
		{
			return logic.strLogicId == logicId;
		});
	if (found == candidate.Logics.end())
	{
		outStatus = m_strStatus = "Logic delete target is absent.";
		return false;
	}
	bool_t unresolvedReferences = false;
	const std::size_t references = Count_LogicReferences(candidate, logicId, &unresolvedReferences);
	if (unresolvedReferences)
	{
		outStatus = m_strStatus = "Repair the invalid Pattern Logic references before deleting a Logic.";
		return false;
	}
	if (0u != references)
	{
		outStatus = m_strStatus = "Delete its " + std::to_string(references) +
			" Logic box(es) first; a referenced Logic is preserved.";
		return false;
	}
	candidate.Logics.erase(found);
	if (!Commit_Candidate(std::move(candidate), "Deleted Logic.", outStatus))
		return false;
	if (m_strSelectedLogicId == logicId)
		m_strSelectedLogicId.clear();
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Append_LogicBox(
	const std::string_view patternId,
	const std::string_view logicId,
	const std::uint32_t startMs,
	const std::uint32_t durationMs,
	std::string& outOccurrenceId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Append Logic requires an editable Pattern selected in Patterns.";
		return false;
	}
	if (nullptr == Find_Logic(candidate, logicId))
	{
		outStatus = m_strStatus = "Append Logic target definition is absent.";
		return false;
	}
	if (pattern->iNextLogicOccurrenceOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Logic box stable ID ordinals are exhausted.";
		return false;
	}
	const std::uint32_t lifetimeMs = Pattern_DurationMs(*pattern);
	if (0u == lifetimeMs)
	{
		outStatus = m_strStatus =
			"Append Logic requires a Pattern lifetime. Append an animation action first.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE box;
	box.strOccurrenceId = pattern->strPatternId + ".logic." +
		std::to_string(pattern->iNextLogicOccurrenceOrdinal++);
	box.strLogicId = std::string(logicId);
	box.iStartMs = (std::min)(startMs, lifetimeMs - 1u);
	box.iDurationMs = std::clamp(durationMs, 1u, lifetimeMs - box.iStartMs);
	const std::string occurrenceId = box.strOccurrenceId;
	/* A typed judgement box may join a PRODUCT Pattern; a Logic that is still
	   only a name has nothing the Server could run, so it returns to DRAFT. */
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION* const owner = Find_Logic(candidate, logicId);
	const bool_t untypedDuration = nullptr != owner && "DURATION" == owner->strLogicType &&
		owner->strJudgementKind.empty();
	const bool_t returnedToDraft = untypedDuration && "PRODUCT" == pattern->strAuthoringStatus;
	pattern->LogicOccurrences.push_back(std::move(box));
	if (returnedToDraft)
		Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			returnedToDraft ?
				"Appended Logic box and returned the Pattern to DRAFT; give the Logic a judgement kind in Resources > Logic." :
				"Appended Logic box at the cursor. Drag its edges on the Logic lane or edit Lifetime in Box Detail.",
			outStatus))
	{
		return false;
	}
	m_strSelectedPatternId = std::string(patternId);
	m_TimelineSelectedStageIds.clear();
	m_TimelineSelectedOccurrenceIds.clear();
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId = occurrenceId;
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
	outOccurrenceId = occurrenceId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_LogicBoxWindow(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	const std::uint32_t startMs,
	const std::uint32_t durationMs,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE* const box =
		nullptr == pattern ? nullptr : Find_LogicBox(*pattern, occurrenceId);
	if (nullptr == box)
	{
		outStatus = m_strStatus = "Logic box window target is absent.";
		return false;
	}
	const std::uint64_t lifetimeMs = Pattern_DurationMs(*pattern);
	const std::uint64_t endMs = static_cast<std::uint64_t>(startMs) + durationMs;
	if (0u == durationMs || endMs > lifetimeMs)
	{
		outStatus = m_strStatus = "Logic box window must stay inside the Pattern lifetime of " +
			std::to_string(lifetimeMs) + " ms.";
		return false;
	}
	if (box->iStartMs == startMs && box->iDurationMs == durationMs)
	{
		outStatus = m_strStatus = "Logic box window is unchanged.";
		return true;
	}
	box->iStartMs = startMs;
	box->iDurationMs = durationMs;
	for (auto& collider : pattern->PresentationOccurrences)
		if (collider.strLogicOccurrenceId == occurrenceId)
		{
			collider.iStartMs = startMs;
			collider.iDurationMs = durationMs;
		}
	return Commit_Candidate(std::move(candidate), "Updated Logic window and its linked Collider boxes.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_LogicBox(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = m_strStatus = "Logic box delete Pattern is absent.";
		return false;
	}
	const auto found = std::find_if(
		pattern->LogicOccurrences.begin(), pattern->LogicOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE& box)
		{
			return box.strOccurrenceId == occurrenceId;
		});
	if (found == pattern->LogicOccurrences.end())
	{
		outStatus = m_strStatus = "Logic box delete target is absent.";
		return false;
	}
	pattern->LogicOccurrences.erase(found);
	if (!Commit_Candidate(std::move(candidate), "Deleted Logic box.", outStatus))
		return false;
	if (m_strSelectedLogicOccurrenceId == occurrenceId)
	{
		m_strSelectedLogicOccurrenceId.clear();
		Synchronize_EditorFields();
	}
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_LogicBoxOutcomes(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	const KOUKU_SAYDON_OUTCOME_SLOT slot,
	const std::vector<std::string>& resultLogicIds,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE* const box =
		nullptr == pattern ? nullptr : Find_LogicBox(*pattern, occurrenceId);
	if (nullptr == box)
	{
		outStatus = m_strStatus = "Logic box outcome target is absent.";
		return false;
	}
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION* const owner = Find_Logic(candidate, box->strLogicId);
	if (nullptr == owner || !Kouku_LogicOwnsOutcomes(*owner))
	{
		outStatus = m_strStatus = "Only a DURATION or ENTER_AREA Logic box owns result slots.";
		return false;
	}
	if (!Kouku_LogicOutcomeKind(*owner).empty() && !resultLogicIds.empty() &&
		!Kouku_IsOutcomeSlotAllowed(Kouku_LogicOutcomeKind(*owner), slot))
	{
		outStatus = m_strStatus = std::string(Outcome_SlotLabel(slot)) +
			" is not an outcome of " + owner->strJudgementKind + ".";
		return false;
	}
	if (resultLogicIds.size() > KOUKU_SAYDON_MAX_OUTCOMES_PER_SLOT)
	{
		outStatus = m_strStatus = "An outcome slot holds at most four RESULT Logics.";
		return false;
	}
	for (const std::string& resultLogicId : resultLogicIds)
	{
		const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION* const result = Find_Logic(candidate, resultLogicId);
		if (nullptr == result || "RESULT" != result->strLogicType)
		{
			outStatus = m_strStatus = "An outcome must name an existing RESULT Logic.";
			return false;
		}
	}
	std::vector<std::string>& current = box->Outcomes(slot);
	if (current == resultLogicIds)
	{
		outStatus = m_strStatus = "Logic box outcome is unchanged.";
		return true;
	}
	current = resultLogicIds;
	return Commit_Candidate(std::move(candidate),
		std::string("Wired the ") + Outcome_SlotLabel(slot) + " outcomes. Press Save to keep them.",
		outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_LogicDefinitionValues(
	const std::string_view logicId,
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& values,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	const auto found = std::find_if(candidate.Logics.begin(), candidate.Logics.end(),
		[logicId](const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic)
		{
			return logic.strLogicId == logicId;
		});
	if (found == candidate.Logics.end())
	{
		outStatus = m_strStatus = "Logic value target is absent.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION updated = values;
	updated.strLogicId = found->strLogicId;
	updated.strDisplayName = found->strDisplayName;
	updated.strLogicType = found->strLogicType;
	if (updated == *found)
	{
		outStatus = m_strStatus = "Logic values are unchanged.";
		return true;
	}
	*found = std::move(updated);
	return Commit_Candidate(std::move(candidate),
		"Applied the Logic values. Press Save to keep them; Publish carries them to the Server.",
		outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Create_Summon(
	const std::string_view displayName,
	std::string& outSummonId,
	std::string& outStatus)
{
	if (!m_bHasDraft || m_Draft.iNextSummonOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Summon creation requires a loaded draft and an available ordinal.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION summon;
	summon.strSummonId = "kakulsaydon.g1.summon." + std::to_string(candidate.iNextSummonOrdinal++);
	summon.strDisplayName = std::string(displayName);
	const std::string summonId = summon.strSummonId;
	candidate.Summons.push_back(std::move(summon));
	if (!Commit_Candidate(std::move(candidate),
			"Created Summon " + summonId + ". Select it and press Append Summon at Cursor.", outStatus))
	{
		return false;
	}
	m_strSelectedSummonId = summonId;
	outSummonId = summonId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_Summon(
	const std::string_view summonId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	const auto found = std::find_if(candidate.Summons.begin(), candidate.Summons.end(),
		[summonId](const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION& summon)
		{
			return summon.strSummonId == summonId;
		});
	if (found == candidate.Summons.end())
	{
		outStatus = m_strStatus = "Summon delete target is absent.";
		return false;
	}
	bool_t unresolvedReferences = false;
	const std::size_t references = Count_SummonReferences(candidate, summonId, &unresolvedReferences);
	if (unresolvedReferences)
	{
		outStatus = m_strStatus = "Repair the invalid Pattern Summon references before deleting a Summon.";
		return false;
	}
	if (0u != references)
	{
		outStatus = m_strStatus = "Delete its " + std::to_string(references) +
			" Summon box(es) first; a referenced Summon is preserved.";
		return false;
	}
	candidate.Summons.erase(found);
	if (!Commit_Candidate(std::move(candidate), "Deleted Summon.", outStatus))
		return false;
	if (m_strSelectedSummonId == summonId)
		m_strSelectedSummonId.clear();
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Append_SummonBox(
	const std::string_view patternId,
	const std::string_view summonId,
	const std::uint32_t startMs,
	const std::uint32_t durationMs,
	std::string& outOccurrenceId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Append Summon requires an editable Pattern selected in Patterns.";
		return false;
	}
	if (nullptr == Find_Summon(candidate, summonId))
	{
		outStatus = m_strStatus = "Append Summon target definition is absent.";
		return false;
	}
	if (pattern->iNextSummonOccurrenceOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Summon box stable ID ordinals are exhausted.";
		return false;
	}
	const std::uint32_t lifetimeMs = Pattern_DurationMs(*pattern);
	if (0u == lifetimeMs)
	{
		outStatus = m_strStatus =
			"Append Summon requires a Pattern lifetime. Append an animation action first.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE box;
	box.strOccurrenceId = pattern->strPatternId + ".summon." +
		std::to_string(pattern->iNextSummonOccurrenceOrdinal++);
	box.strSummonId = std::string(summonId);
	box.iStartMs = (std::min)(startMs, lifetimeMs - 1u);
	box.iDurationMs = std::clamp(durationMs, 1u, lifetimeMs - box.iStartMs);
	const std::string occurrenceId = box.strOccurrenceId;
	pattern->SummonOccurrences.push_back(std::move(box));
	if (!Commit_Candidate(std::move(candidate),
			"Appended Summon box. Its lifetime ends where the spawn despawns; drag the edges on the Summon lane. Summon stays authoring-only until its Server consumer lands.",
			outStatus))
	{
		return false;
	}
	m_strSelectedPatternId = std::string(patternId);
	m_TimelineSelectedStageIds.clear();
	m_TimelineSelectedOccurrenceIds.clear();
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId = occurrenceId;
	m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
	outOccurrenceId = occurrenceId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_SummonBoxWindow(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	const std::uint32_t startMs,
	const std::uint32_t durationMs,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE* const box =
		nullptr == pattern ? nullptr : Find_SummonBox(*pattern, occurrenceId);
	if (nullptr == box)
	{
		outStatus = m_strStatus = "Summon box window target is absent.";
		return false;
	}
	const std::uint64_t lifetimeMs = Pattern_DurationMs(*pattern);
	const std::uint64_t endMs = static_cast<std::uint64_t>(startMs) + durationMs;
	if (0u == durationMs || endMs > lifetimeMs)
	{
		outStatus = m_strStatus = "Summon box window must stay inside the Pattern lifetime of " +
			std::to_string(lifetimeMs) + " ms.";
		return false;
	}
	if (box->iStartMs == startMs && box->iDurationMs == durationMs)
	{
		outStatus = m_strStatus = "Summon box window is unchanged.";
		return true;
	}
	box->iStartMs = startMs;
	box->iDurationMs = durationMs;
	return Commit_Candidate(std::move(candidate), "Updated Summon box window.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_SummonBox(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = m_strStatus = "Summon box delete Pattern is absent.";
		return false;
	}
	const auto found = std::find_if(
		pattern->SummonOccurrences.begin(), pattern->SummonOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE& box)
		{
			return box.strOccurrenceId == occurrenceId;
		});
	if (found == pattern->SummonOccurrences.end())
	{
		outStatus = m_strStatus = "Summon box delete target is absent.";
		return false;
	}
	pattern->SummonOccurrences.erase(found);
	if (!Commit_Candidate(std::move(candidate), "Deleted Summon box.", outStatus))
		return false;
	if (m_strSelectedSummonOccurrenceId == occurrenceId)
	{
		m_strSelectedSummonOccurrenceId.clear();
		Synchronize_EditorFields();
	}
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Render_SummonResources()
{
	ImGui::SeparatorText("Summon Catalog");
	ImGui::TextDisabled("%zu summons | Create names what a Pattern spawns; Append places its spawn time and lifetime.",
		m_Draft.Summons.size());
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const selectedPattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool_t patternEditable = nullptr != selectedPattern &&
		selectedPattern->strLoadError.empty();
	const std::string targetPatternName = nullptr == selectedPattern ?
		std::string("none") : selectedPattern->strDisplayName;
	const std::uint32_t lifetimeMs = nullptr == selectedPattern ? 0u : Pattern_DurationMs(*selectedPattern);

	if (ImGui::BeginChild("##KoukuSummonList", ImVec2(0.f, 160.f), ImGuiChildFlags_Borders))
	{
		if (m_Draft.Summons.empty())
			ImGui::TextDisabled("No Summon yet. Name it and press Create Summon.");
		for (const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION& summon : m_Draft.Summons)
		{
			const std::string label = summon.strDisplayName + "##" + summon.strSummonId;
			if (ImGui::Selectable(label.c_str(), m_strSelectedSummonId == summon.strSummonId))
				m_strSelectedSummonId = summon.strSummonId;
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s | %zu box(es)", summon.strSummonId.c_str(),
					Count_SummonReferences(m_Draft, summon.strSummonId));
		}
	}
	ImGui::EndChild();

	ImGui::SeparatorText("Create Summon");
	ImGui::InputTextWithHint("##NewKoukuSummonName", "Summon display name",
		m_NewSummonName, std::size(m_NewSummonName));
	ImGui::SameLine();
	ImGui::BeginDisabled('\0' == m_NewSummonName[0] || !m_bHasDraft);
	if (ImGui::Button("Create Summon"))
	{
		std::string summonId;
		std::string status;
		if (Create_Summon(m_NewSummonName, summonId, status))
			m_NewSummonName[0] = '\0';
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();

	const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION* const selectedSummon =
		Find_Summon(m_Draft, m_strSelectedSummonId);
	if (nullptr == selectedSummon)
		return;
	const std::string summonId = selectedSummon->strSummonId;
	const std::string summonName = selectedSummon->strDisplayName;
	bool_t unresolvedReferences = false;
	const std::size_t references = Count_SummonReferences(m_Draft, summonId, &unresolvedReferences);
	ImGui::SeparatorText("Selected Summon");
	ImGui::TextWrapped("%s", summonName.c_str());
	ImGui::TextDisabled("%s | %zu box(es)", summonId.c_str(), references);
	ImGui::Checkbox("Until Pattern end##NewKoukuSummonBox", &m_bNewSummonBoxToPatternEnd);
	if (!m_bNewSummonBoxToPatternEnd)
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(120.f);
		ImGui::InputInt("Box ms##NewKoukuSummonBox", &m_iNewSummonBoxDurationMs, 100, 1000);
		m_iNewSummonBoxDurationMs = std::clamp(m_iNewSummonBoxDurationMs, 1,
			static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	}
	ImGui::BeginDisabled(!patternEditable);
	if (ImGui::Button("Append Summon at Cursor"))
	{
		const std::uint32_t durationMs = m_bNewSummonBoxToPatternEnd ?
			(lifetimeMs > m_iCursorMs ? lifetimeMs - m_iCursorMs : 1u) :
			static_cast<std::uint32_t>(m_iNewSummonBoxDurationMs);
		std::string occurrenceId;
		std::string status;
		(void)Append_SummonBox(m_strSelectedPatternId, summonId, m_iCursorMs,
			durationMs, occurrenceId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(0u != references || unresolvedReferences);
	if (ImGui::Button("Delete Summon"))
	{
		std::string status;
		(void)Delete_Summon(summonId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	if (!patternEditable)
		ImGui::TextDisabled("Select an editable Pattern in Patterns to append this Summon.");
	ImGui::TextDisabled("Append target: %s | cursor %u ms | lifetime %u ms",
		targetPatternName.c_str(), m_iCursorMs, lifetimeMs);
}

void Client::CKoukuSaydonActionWorkbench::Render_SummonBoxDetails(
	const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
{
	const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE* const box =
		Find_SummonBox(pattern, m_strSelectedSummonOccurrenceId);
	if (nullptr == box)
		return;
	const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION* const summon =
		Find_Summon(m_Draft, box->strSummonId);
	const std::string patternId = pattern.strPatternId;
	const std::string occurrenceId = box->strOccurrenceId;
	const std::uint32_t lifetimeMs = Pattern_DurationMs(pattern);
	ImGui::SeparatorText("Summon Box");
	ImGui::Text("%s", occurrenceId.c_str());
	ImGui::Text("%s", nullptr == summon ? "(missing Summon)" : summon->strDisplayName.c_str());
	ImGui::TextDisabled("%s", box->strSummonId.c_str());
	ImGui::InputInt("Spawn ms##KoukuSummonBox", &m_iSummonBoxStartMs, 10, 100);
	ImGui::InputInt("Lifetime ms##KoukuSummonBox", &m_iSummonBoxDurationMs, 10, 100);
	m_iSummonBoxStartMs = std::clamp(m_iSummonBoxStartMs, 0,
		static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_iSummonBoxDurationMs = std::clamp(m_iSummonBoxDurationMs, 1,
		static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	if (ImGui::Button("Apply Window##KoukuSummonBox"))
	{
		std::string status;
		(void)Set_SummonBoxWindow(patternId, occurrenceId,
			static_cast<std::uint32_t>(m_iSummonBoxStartMs),
			static_cast<std::uint32_t>(m_iSummonBoxDurationMs), status);
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("To Pattern End##KoukuSummonBox"))
	{
		std::string status;
		(void)Set_SummonBoxWindow(patternId, occurrenceId, box->iStartMs,
			lifetimeMs > box->iStartMs ? lifetimeMs - box->iStartMs : 1u, status);
		return;
	}
	ImGui::TextDisabled("The spawn appears at Spawn ms and despawns when Lifetime ends. Pattern lifetime %u ms.",
		lifetimeMs);
	if (ImGui::Button("Delete Summon Box"))
	{
		std::string status;
		(void)Delete_SummonBox(patternId, occurrenceId, status);
	}
}

bool_t Client::CKoukuSaydonActionWorkbench::Append_WorldResource(
	const std::string_view instanceId, std::string& outStatus)
{
	const auto source = std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(),
		[instanceId](const auto& row) { return row.strInstanceId == instanceId; });
	if (source == m_WorldSequenceResources.end())
	{ outStatus = m_strStatus = "Select a saved World Object state first."; return false; }
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, m_strSelectedPatternId);
	if (!m_bHasDraft || !pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0u ||
		pattern->iNextWorldOccurrenceOrdinal >= 1000000u)
	{ outStatus = m_strStatus = "Append needs an editable Pattern with an animation lifetime."; return false; }
	auto world = std::find_if(candidate.Worlds.begin(), candidate.Worlds.end(),
		[instanceId](const auto& row) { return row.strSequenceInstanceId == instanceId; });
	if (world == candidate.Worlds.end())
	{
		if (candidate.iNextWorldOrdinal >= 1000000u)
		{ outStatus = m_strStatus = "World resource stable ID ordinals are exhausted."; return false; }
		KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION value;
		value.strWorldId = "kakulsaydon.g1.world." + std::to_string(candidate.iNextWorldOrdinal++);
		value.strDisplayName = source->strDisplayName;
		value.strSequenceInstanceId = source->strInstanceId;
		candidate.Worlds.push_back(std::move(value));
		world = candidate.Worlds.end() - 1;
	}
	KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE box;
	box.strOccurrenceId = pattern->strPatternId + ".world." + std::to_string(pattern->iNextWorldOccurrenceOrdinal++);
	box.strWorldId = world->strWorldId;
	box.iStartMs = (std::min)(m_iCursorMs, Pattern_DurationMs(*pattern));
	box.iDurationMs = (std::max)(1u, source->iDurationMs);
	const std::string worldId = box.strWorldId;
	const std::string occurrenceId = box.strOccurrenceId;
	pattern->WorldOccurrences.push_back(box);
	if (!Append_WorldCompanionEffect(*pattern, *world, box, outStatus))
	{ m_strStatus = outStatus; return false; }
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Appended saved World Object state at the cursor. Tune lifetime in Box Detail.", outStatus)) return false;
	m_strSelectedWorldId = worldId;
	m_strSelectedWorldOccurrenceId = occurrenceId;
	m_strSelectedStageId.clear(); m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear(); m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear(); m_strSelectedPresentationOccurrenceId.clear();
	m_TimelineSelectedStageIds.clear(); m_TimelineSelectedOccurrenceIds.clear();
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Create_World(
	const std::string_view displayName,
	const std::string_view sequenceInstanceId,
	std::string& outWorldId,
	std::string& outStatus)
{
	if (!m_bHasDraft || m_Draft.iNextWorldOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "World creation requires a loaded draft and an available ordinal.";
		return false;
	}
	if (sequenceInstanceId.empty())
	{
		outStatus = m_strStatus = "Create World needs one authored world sequence instance.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION world;
	world.strWorldId = "kakulsaydon.g1.world." + std::to_string(candidate.iNextWorldOrdinal++);
	world.strDisplayName = std::string(displayName);
	world.strSequenceInstanceId = std::string(sequenceInstanceId);
	const std::string worldId = world.strWorldId;
	candidate.Worlds.push_back(std::move(world));
	if (!Commit_Candidate(std::move(candidate),
			"Created World " + worldId + ". Select it and press Append World at Cursor.", outStatus))
	{
		return false;
	}
	m_strSelectedWorldId = worldId;
	outWorldId = worldId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_WorldCompanionEffect(
	const std::string_view worldId, const std::string_view resourceId, std::string& outStatus)
{
	auto candidate = m_Draft;
	const auto world = std::find_if(candidate.Worlds.begin(), candidate.Worlds.end(),
		[worldId](const auto& row) { return row.strWorldId == worldId; });
	const auto* resource = resourceId.empty() ? nullptr : Find_PresentationResource(candidate, resourceId);
	if (world == candidate.Worlds.end() || (!resourceId.empty() &&
		(nullptr == resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)))
	{ outStatus = m_strStatus = "World companion requires an existing World and an EFFECT resource."; return false; }
	world->strCompanionEffectResourceId = std::string(resourceId);
	for (auto& pattern : candidate.Patterns)
	{
		if (!pattern.strLoadError.empty()) continue;
		bool changed = false;
		for (const auto& owner : pattern.WorldOccurrences)
		{
			if (owner.strWorldId != worldId) continue;
			const auto linked = std::find_if(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
				[&](const auto& row) { return row.strWorldOccurrenceId == owner.strOccurrenceId; });
			if (linked != pattern.PresentationOccurrences.end())
			{
				if (resourceId.empty()) linked->strWorldOccurrenceId.clear();
				else linked->strResourceId = std::string(resourceId);
				changed = true;
			}
			else if (!resourceId.empty())
			{
				if (!Append_WorldCompanionEffect(pattern, *world, owner, outStatus))
				{ m_strStatus = outStatus; return false; }
				changed = true;
			}
		}
		if (changed) Mark_Draft(candidate, pattern);
	}
	return Commit_Candidate(std::move(candidate), resourceId.empty() ?
		"Removed the World companion link; existing Effect boxes remain independent." :
		"Connected the World companion and its explicit Effect boxes.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Render_WorldCompanionSelector(
	const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world)
{
	const auto* selected = Find_PresentationResource(m_Draft, world.strCompanionEffectResourceId);
	if (ImGui::BeginCombo("Companion Effect", nullptr == selected ? "(none)" : selected->strDisplayName.c_str()))
	{
		std::string status;
		if (ImGui::Selectable("(none)", world.strCompanionEffectResourceId.empty()))
		{
			const std::string worldId = world.strWorldId;
			(void)Set_WorldCompanionEffect(worldId, "", status);
			ImGui::EndCombo(); return true;
		}
		for (const auto& resource : m_Draft.PresentationResources)
			if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT &&
				ImGui::Selectable((resource.strDisplayName + "##" + resource.strResourceId).c_str(), resource.strResourceId == world.strCompanionEffectResourceId))
			{
				const auto worldId = world.strWorldId, resourceId = resource.strResourceId;
				(void)Set_WorldCompanionEffect(worldId, resourceId, status);
				ImGui::EndCombo(); return true;
			}
		ImGui::EndCombo();
	}
	ImGui::TextDisabled("World Append adds an Effect box. World timing edits sync it; its own Effect tuning remains editable.");
	return false;
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_World(
	const std::string_view worldId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	const auto found = std::find_if(candidate.Worlds.begin(), candidate.Worlds.end(),
		[worldId](const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world)
		{
			return world.strWorldId == worldId;
		});
	if (found == candidate.Worlds.end())
	{
		outStatus = m_strStatus = "World delete target is absent.";
		return false;
	}
	bool_t unresolvedReferences = false;
	const std::size_t references = Count_WorldReferences(candidate, worldId, &unresolvedReferences);
	if (unresolvedReferences)
	{
		outStatus = m_strStatus = "Repair the invalid Pattern World references before deleting a World.";
		return false;
	}
	if (0u != references)
	{
		outStatus = m_strStatus = "Delete its " + std::to_string(references) +
			" World box(es) first; a referenced World is preserved.";
		return false;
	}
	candidate.Worlds.erase(found);
	if (!Commit_Candidate(std::move(candidate), "Deleted World.", outStatus))
		return false;
	if (m_strSelectedWorldId == worldId)
		m_strSelectedWorldId.clear();
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Append_WorldBox(
	const std::string_view patternId,
	const std::string_view worldId,
	const std::uint32_t startMs,
	const std::uint32_t durationMs,
	std::string& outOccurrenceId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Append World requires an editable Pattern selected in Patterns.";
		return false;
	}
	const auto* world = Find_World(candidate, worldId);
	if (nullptr == world)
	{
		outStatus = m_strStatus = "Append World target definition is absent.";
		return false;
	}
	if (pattern->iNextWorldOccurrenceOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "World box stable ID ordinals are exhausted.";
		return false;
	}
	const std::uint32_t lifetimeMs = Pattern_DurationMs(*pattern);
	if (0u == lifetimeMs)
	{
		outStatus = m_strStatus =
			"Append World requires a Pattern lifetime. Append an animation action first.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE box;
	box.strOccurrenceId = pattern->strPatternId + ".world." +
		std::to_string(pattern->iNextWorldOccurrenceOrdinal++);
	box.strWorldId = std::string(worldId);
	box.iStartMs = (std::min)(startMs, lifetimeMs);
	box.iDurationMs = (std::max)(1u, durationMs);
	box.fPlaybackSpeed = 1.f;
	const std::string occurrenceId = box.strOccurrenceId;
	pattern->WorldOccurrences.push_back(std::move(box));
	if (!Append_WorldCompanionEffect(*pattern, *world, pattern->WorldOccurrences.back(), outStatus))
	{ m_strStatus = outStatus; return false; }
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Appended World box. The sequence starts at the box start and runs at the box speed; edit both in Box Detail.",
			outStatus))
	{
		return false;
	}
	m_strSelectedPatternId = std::string(patternId);
	m_TimelineSelectedStageIds.clear();
	m_TimelineSelectedOccurrenceIds.clear();
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId = occurrenceId;
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
	outOccurrenceId = occurrenceId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_WorldBoxWindow(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	const std::uint32_t startMs,
	const std::uint32_t durationMs,
	const f32_t playbackSpeed,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE* const box =
		nullptr == pattern ? nullptr : Find_WorldBox(*pattern, occurrenceId);
	if (nullptr == box)
	{
		outStatus = m_strStatus = "World box window target is absent.";
		return false;
	}
	const std::uint64_t lifetimeMs = Pattern_DurationMs(*pattern);
	if (0u == durationMs || startMs > lifetimeMs ||
		!std::isfinite(playbackSpeed) || playbackSpeed < 0.05f || playbackSpeed > 16.f)
	{
		outStatus = m_strStatus = "World box must start inside the Pattern lifetime of " +
			std::to_string(lifetimeMs) + " ms and play at 0.05..16 x.";
		return false;
	}
	if (box->iStartMs == startMs && box->iDurationMs == durationMs &&
		box->fPlaybackSpeed == playbackSpeed)
	{
		outStatus = m_strStatus = "World box is unchanged.";
		return true;
	}
	const bool_t timingChanged = box->iStartMs != startMs || box->iDurationMs != durationMs;
	box->iStartMs = startMs;
	box->iDurationMs = durationMs;
	box->fPlaybackSpeed = playbackSpeed;
	for (auto& effect : pattern->PresentationOccurrences)
		if (timingChanged && effect.strWorldOccurrenceId == occurrenceId)
		{
			effect.iStartMs = startMs; effect.iDurationMs = durationMs;
			effect.iFadeInMs = (std::min)(effect.iFadeInMs, durationMs);
			effect.iFadeOutMs = (std::min)(effect.iFadeOutMs, durationMs - effect.iFadeInMs);
		}
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Updated World and companion Effect timing.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_WorldBox(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = m_strStatus = "World box delete Pattern is absent.";
		return false;
	}
	const auto found = std::find_if(
		pattern->WorldOccurrences.begin(), pattern->WorldOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& box)
		{
			return box.strOccurrenceId == occurrenceId;
		});
	if (found == pattern->WorldOccurrences.end())
	{
		outStatus = m_strStatus = "World box delete target is absent.";
		return false;
	}
	pattern->WorldOccurrences.erase(found);
	std::erase_if(pattern->PresentationOccurrences, [occurrenceId](const auto& row) { return row.strWorldOccurrenceId == occurrenceId; });
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Deleted World and its linked Effect box.", outStatus))
		return false;
	if (m_strSelectedWorldOccurrenceId == occurrenceId)
	{
		m_strSelectedWorldOccurrenceId.clear();
		Synchronize_EditorFields();
	}
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Create_SceneProfile(
	const std::string_view displayName,
	const std::string_view renderingProfileId,
	std::string& outSceneProfileId,
	std::string& outStatus)
{
	if (!m_bHasDraft || m_Draft.iNextSceneProfileOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Scene Profile creation requires a loaded draft and an available ordinal.";
		return false;
	}
	if (renderingProfileId.empty())
	{
		outStatus = m_strStatus = "Create Scene Profile needs one rendering profile.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION profile;
	profile.strSceneProfileId = "kakulsaydon.g1.sceneprofile." +
		std::to_string(candidate.iNextSceneProfileOrdinal++);
	profile.strDisplayName = std::string(displayName);
	profile.strRenderingProfileId = std::string(renderingProfileId);
	const std::string sceneProfileId = profile.strSceneProfileId;
	candidate.SceneProfiles.push_back(std::move(profile));
	if (!Commit_Candidate(std::move(candidate),
			"Created Scene Profile " + sceneProfileId + ". Select it and press Append Scene Profile at Cursor.",
			outStatus))
	{
		return false;
	}
	m_strSelectedSceneProfileId = sceneProfileId;
	outSceneProfileId = sceneProfileId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_SceneProfile(
	const std::string_view sceneProfileId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	const auto found = std::find_if(candidate.SceneProfiles.begin(), candidate.SceneProfiles.end(),
		[sceneProfileId](const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION& profile)
		{
			return profile.strSceneProfileId == sceneProfileId;
		});
	if (found == candidate.SceneProfiles.end())
	{
		outStatus = m_strStatus = "Scene Profile delete target is absent.";
		return false;
	}
	bool_t unresolvedReferences = false;
	const std::size_t references =
		Count_SceneProfileReferences(candidate, sceneProfileId, &unresolvedReferences);
	if (unresolvedReferences)
	{
		outStatus = m_strStatus = "Repair the invalid Pattern Scene Profile references before deleting one.";
		return false;
	}
	if (0u != references)
	{
		outStatus = m_strStatus = "Delete its " + std::to_string(references) +
			" Scene Profile box(es) first; a referenced Scene Profile is preserved.";
		return false;
	}
	candidate.SceneProfiles.erase(found);
	if (!Commit_Candidate(std::move(candidate), "Deleted Scene Profile.", outStatus))
		return false;
	if (m_strSelectedSceneProfileId == sceneProfileId)
		m_strSelectedSceneProfileId.clear();
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Append_SceneProfileBox(
	const std::string_view patternId,
	const std::string_view sceneProfileId,
	const std::uint32_t startMs,
	const std::uint32_t durationMs,
	std::string& outOccurrenceId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Append Scene Profile requires an editable Pattern selected in Patterns.";
		return false;
	}
	if (nullptr == Find_SceneProfile(candidate, sceneProfileId))
	{
		outStatus = m_strStatus = "Append Scene Profile target definition is absent.";
		return false;
	}
	if (pattern->iNextSceneProfileOccurrenceOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Scene Profile box stable ID ordinals are exhausted.";
		return false;
	}
	const std::uint32_t lifetimeMs = Pattern_DurationMs(*pattern);
	if (0u == lifetimeMs)
	{
		outStatus = m_strStatus =
			"Append Scene Profile requires a Pattern lifetime. Append an animation action first.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE box;
	box.strOccurrenceId = pattern->strPatternId + ".sceneprofile." +
		std::to_string(pattern->iNextSceneProfileOccurrenceOrdinal++);
	box.strSceneProfileId = std::string(sceneProfileId);
	box.iStartMs = (std::min)(startMs, lifetimeMs - 1u);
	box.iDurationMs = std::clamp(durationMs, 1u, lifetimeMs - box.iStartMs);
	box.iBlendMs = 500u;
	const std::string occurrenceId = box.strOccurrenceId;
	pattern->SceneProfileOccurrences.push_back(std::move(box));
	if (!Commit_Candidate(std::move(candidate),
			"Appended Scene Profile box. The profile applies at the box start, blends in over Blend ms and restores when the box ends.",
			outStatus))
	{
		return false;
	}
	m_strSelectedPatternId = std::string(patternId);
	m_TimelineSelectedStageIds.clear();
	m_TimelineSelectedOccurrenceIds.clear();
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId = occurrenceId;
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
	outOccurrenceId = occurrenceId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_SceneProfileBoxWindow(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	const std::uint32_t startMs,
	const std::uint32_t durationMs,
	const std::uint32_t blendMs,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE* const box =
		nullptr == pattern ? nullptr : Find_SceneProfileBox(*pattern, occurrenceId);
	if (nullptr == box)
	{
		outStatus = m_strStatus = "Scene Profile box window target is absent.";
		return false;
	}
	const std::uint64_t lifetimeMs = Pattern_DurationMs(*pattern);
	const std::uint64_t endMs = static_cast<std::uint64_t>(startMs) + durationMs;
	if (0u == durationMs || endMs > lifetimeMs || blendMs > MAX_EDITOR_TIME_MS)
	{
		outStatus = m_strStatus = "Scene Profile box window must stay inside the Pattern lifetime of " +
			std::to_string(lifetimeMs) + " ms.";
		return false;
	}
	if (box->iStartMs == startMs && box->iDurationMs == durationMs && box->iBlendMs == blendMs)
	{
		outStatus = m_strStatus = "Scene Profile box is unchanged.";
		return true;
	}
	box->iStartMs = startMs;
	box->iDurationMs = durationMs;
	box->iBlendMs = blendMs;
	return Commit_Candidate(std::move(candidate), "Updated Scene Profile box.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_SceneProfileBox(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = m_strStatus = "Scene Profile box delete Pattern is absent.";
		return false;
	}
	const auto found = std::find_if(
		pattern->SceneProfileOccurrences.begin(), pattern->SceneProfileOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE& box)
		{
			return box.strOccurrenceId == occurrenceId;
		});
	if (found == pattern->SceneProfileOccurrences.end())
	{
		outStatus = m_strStatus = "Scene Profile box delete target is absent.";
		return false;
	}
	pattern->SceneProfileOccurrences.erase(found);
	if (!Commit_Candidate(std::move(candidate), "Deleted Scene Profile box.", outStatus))
		return false;
	if (m_strSelectedSceneProfileOccurrenceId == occurrenceId)
	{
		m_strSelectedSceneProfileOccurrenceId.clear();
		m_strSelectedPresentationOccurrenceId.clear();
		Synchronize_EditorFields();
	}
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Create_PresentationResource(
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& source,
	const std::string_view displayName, std::string& outStatus)
{
	auto candidate = m_Draft;
	if (candidate.iNextPresentationResourceOrdinal >= 1000000u)
	{ outStatus = m_strStatus = "Presentation resource IDs are exhausted."; return false; }
	auto row = source;
	row.strResourceId = "kakulsaydon.g1.presentation." + std::to_string(candidate.iNextPresentationResourceOrdinal++);
	row.strDisplayName = std::string(displayName);
	const auto id = row.strResourceId;
	candidate.PresentationResources.push_back(std::move(row));
	if (!Commit_Candidate(std::move(candidate), "Created presentation resource. Append places it on the Pattern clock.", outStatus)) return false;
	m_strSelectedPresentationResourceId = id;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Append_PresentationBox(
	const std::string_view resourceId, std::string& outStatus)
{
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, m_strSelectedPatternId);
	const auto* resource = Find_PresentationResource(candidate, resourceId);
	if (nullptr == pattern || !pattern->strLoadError.empty() || nullptr == resource ||
		pattern->iNextPresentationOccurrenceOrdinal >= 1000000u || Pattern_DurationMs(*pattern) == 0u)
	{ outStatus = m_strStatus = "Append requires a valid resource and a Pattern with a lifetime."; return false; }
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE row;
	row.strOccurrenceId = pattern->strPatternId + ".presentation." + std::to_string(pattern->iNextPresentationOccurrenceOrdinal++);
	row.strResourceId = std::string(resourceId);
	row.iStartMs = (std::min)(m_iCursorMs, Pattern_DurationMs(*pattern) - 1u);
	row.iDurationMs = (std::min)(resource->iDurationMs, Pattern_DurationMs(*pattern) - row.iStartMs);
	if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT)
	{
		row.strAnchorKind = resource->strDefaultAnchorKind;
		row.bFollowBoss = row.strAnchorKind != "MAP";
	}
	if (resource->strColliderKind == "ROULETTE_CARD_REGION")
	{
		row.strRegionId = row.strOccurrenceId + ".region";
		row.strAnchorKind = "WORLD";
	}
	const auto id = row.strOccurrenceId;
	pattern->PresentationOccurrences.push_back(row);
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Appended presentation box. Select it to tune timing and presentation.", outStatus)) return false;
	Clear_TimelineSelection();
	m_strSelectedPresentationOccurrenceId = id;
	m_PresentationBoxEdit = row;
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_PresentationBox(
	const std::string_view patternId, const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& value,
	std::string& outStatus)
{
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{ outStatus = m_strStatus = "Presentation box Pattern is unavailable."; return false; }
	const auto found = std::find_if(pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(),
		[&](const auto& row) { return row.strOccurrenceId == value.strOccurrenceId; });
	if (found == pattern->PresentationOccurrences.end() || found->strResourceId != value.strResourceId)
	{ outStatus = m_strStatus = "Presentation box identity changed; previous row preserved."; return false; }
	*found = value;
	if (!value.strLogicOccurrenceId.empty())
	{
		auto* linked = Find_LogicBox(*pattern, value.strLogicOccurrenceId);
		if (nullptr == linked) { outStatus = m_strStatus = "Linked Logic window is unavailable."; return false; }
		linked->iStartMs = value.iStartMs;
		linked->iDurationMs = value.iDurationMs;
		for (auto& collider : pattern->PresentationOccurrences)
			if (collider.strLogicOccurrenceId == value.strLogicOccurrenceId)
			{
				collider.iStartMs = value.iStartMs;
				collider.iDurationMs = value.iDurationMs;
			}
	}
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Updated presentation box. Save keeps its tuning.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Connect_ColliderLogic(
	const std::string& patternId, const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence,
	const std::string& logicId, std::string& outStatus)
{
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	const auto* logic = Find_Logic(candidate, logicId);
	if (nullptr == pattern || nullptr == logic || (logic->strLogicType != "DURATION" &&
		(logic->strLogicType != "TRIGGER" || logic->strTriggerKind != "ENTER_AREA")))
	{ outStatus = m_strStatus = "Select an existing Duration or ENTER_AREA Trigger Logic definition."; return false; }
	auto existing = std::find_if(pattern->LogicOccurrences.begin(), pattern->LogicOccurrences.end(),
		[&](const auto& box) { return box.strLogicId == logicId; });
	if (existing == pattern->LogicOccurrences.end())
	{
		if (pattern->iNextLogicOccurrenceOrdinal >= 1000000u) return false;
		KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE box;
		box.strOccurrenceId = patternId + ".logic." + std::to_string(pattern->iNextLogicOccurrenceOrdinal++);
		box.strLogicId = logicId;
		box.iStartMs = occurrence.iStartMs;
		box.iDurationMs = occurrence.iDurationMs;
		pattern->LogicOccurrences.push_back(std::move(box));
		existing = std::prev(pattern->LogicOccurrences.end());
	}
	const auto collider = std::find_if(pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(),
		[&](const auto& row) { return row.strOccurrenceId == occurrence.strOccurrenceId; });
	if (collider == pattern->PresentationOccurrences.end()) return false;
	*collider = occurrence;
	collider->strLogicOccurrenceId = existing->strOccurrenceId;
	collider->iStartMs = existing->iStartMs;
	collider->iDurationMs = existing->iDurationMs;
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Connected Collider to the existing Logic window and its Result slots.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_PresentationBoxDebugRender(
	const std::string_view patternId, const std::string_view occurrenceId,
	const bool_t visible, std::string& outStatus)
{
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{ outStatus = m_strStatus = "Collider debug target Pattern is unavailable."; return false; }
	const auto box = std::find_if(pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(),
		[occurrenceId](const auto& row) { return row.strOccurrenceId == occurrenceId; });
	const auto* resource = box == pattern->PresentationOccurrences.end() ? nullptr :
		Find_PresentationResource(candidate, box->strResourceId);
	if (nullptr == resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
	{ outStatus = m_strStatus = "Debug Render requires an existing Collider box."; return false; }
	auto pendingEdit = m_PresentationBoxEdit;
	box->bDebugRender = visible;
	if (!Commit_Candidate(std::move(candidate), "Updated Collider Debug Render. Save keeps this flag.", outStatus)) return false;
	// Synchronize_EditorFields refreshes from the committed row. Restore the
	// pending size/rotation/time draft so this checkbox cannot Apply those values.
	if (pendingEdit.strOccurrenceId == occurrenceId) pendingEdit.bDebugRender = visible;
	m_PresentationBoxEdit = std::move(pendingEdit);
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_ColliderTriggerDamage(
	const std::string& patternId, const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence,
	const std::uint32_t percent, std::string& outStatus)
{
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	const auto* resource = Find_PresentationResource(candidate, occurrence.strResourceId);
	if (!m_bHasDraft || nullptr == pattern || !pattern->strLoadError.empty() || nullptr == resource ||
		resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER || resource->strColliderKind != "GEOMETRY" ||
		percent < 1u || percent > 100u)
	{ outStatus = m_strStatus = "Trigger damage requires a Geometry Collider and 1..100 percent max HP."; return false; }
	auto collider = std::find_if(pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(),
		[&](const auto& row) { return row.strOccurrenceId == occurrence.strOccurrenceId && row.strResourceId == occurrence.strResourceId; });
	if (collider == pattern->PresentationOccurrences.end())
	{ outStatus = m_strStatus = "Trigger Collider identity changed; previous draft preserved."; return false; }
	const auto oldWindowId = collider->strLogicOccurrenceId;
	auto* linked = Find_LogicBox(*pattern, occurrence.strLogicOccurrenceId);
	const auto* linkedLogic = nullptr == linked ? nullptr : Find_Logic(candidate, linked->strLogicId);
	if (!occurrence.strLogicOccurrenceId.empty() && nullptr == linked)
	{ outStatus = m_strStatus = "Selected Trigger window is unavailable."; return false; }
	if (nullptr == linkedLogic || linkedLogic->strLogicType != "TRIGGER" || linkedLogic->strTriggerKind != "ENTER_AREA")
	{
		// Reuse the typed definition, but unrelated boxes get independent windows.
		auto logic = std::find_if(candidate.Logics.begin(), candidate.Logics.end(), [](const auto& row) {
			return row.strLogicType == "TRIGGER" && row.strTriggerKind == "ENTER_AREA"; });
		if (logic == candidate.Logics.end())
		{
			if (candidate.iNextLogicOrdinal >= 1000000u)
			{ outStatus = m_strStatus = "Logic IDs are exhausted."; return false; }
			KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION created;
			created.strLogicId = "kakulsaydon.g1.logic." + std::to_string(candidate.iNextLogicOrdinal++);
			created.strDisplayName = resource->strDisplayName + " Enter Area";
			created.strLogicType = "TRIGGER"; created.strTriggerKind = "ENTER_AREA";
			candidate.Logics.push_back(std::move(created));
			logic = std::prev(candidate.Logics.end());
		}
		if (pattern->iNextLogicOccurrenceOrdinal >= 1000000u)
		{ outStatus = m_strStatus = "Logic window IDs are exhausted."; return false; }
		KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE window;
		window.strOccurrenceId = patternId + ".logic." + std::to_string(pattern->iNextLogicOccurrenceOrdinal++);
		window.strLogicId = logic->strLogicId;
		pattern->LogicOccurrences.push_back(std::move(window));
		linked = &pattern->LogicOccurrences.back();
	}
	const auto windowId = linked->strOccurrenceId;
	auto damageSlot = linked->OnSuccessLogicIds.end();
	for (auto slot = linked->OnSuccessLogicIds.begin(); slot != linked->OnSuccessLogicIds.end(); ++slot)
	{
		const auto* result = Find_Logic(candidate, *slot);
		if (nullptr == result || result->strOutcomeKind != "MAX_HP_PERCENT_DAMAGE") continue;
		if (damageSlot != linked->OnSuccessLogicIds.end())
		{ outStatus = m_strStatus = "Trigger has several damage Results; choose one in Success before editing damage."; return false; }
		damageSlot = slot;
	}
	if (damageSlot == linked->OnSuccessLogicIds.end() && linked->OnSuccessLogicIds.size() >= 4u)
	{ outStatus = m_strStatus = "Success already has four Results; remove one before adding damage."; return false; }
	auto damage = std::find_if(candidate.Logics.begin(), candidate.Logics.end(), [&](const auto& row) {
		return row.strLogicType == "RESULT" && row.strOutcomeKind == "MAX_HP_PERCENT_DAMAGE" && row.iPercent == percent; });
	if (damage == candidate.Logics.end())
	{
		if (candidate.iNextLogicOrdinal >= 1000000u)
		{ outStatus = m_strStatus = "Logic IDs are exhausted."; return false; }
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION result;
		result.strLogicId = "kakulsaydon.g1.logic." + std::to_string(candidate.iNextLogicOrdinal++);
		result.strDisplayName = resource->strDisplayName + " Damage " + std::to_string(percent) + "% max HP";
		result.strLogicType = "RESULT"; result.strOutcomeKind = "MAX_HP_PERCENT_DAMAGE"; result.iPercent = percent;
		candidate.Logics.push_back(std::move(result));
		damage = std::prev(candidate.Logics.end());
	}
	// Rewire this window instead of changing a Result used by other windows.
	if (damageSlot == linked->OnSuccessLogicIds.end()) linked->OnSuccessLogicIds.push_back(damage->strLogicId);
	else *damageSlot = damage->strLogicId;
	linked->iStartMs = occurrence.iStartMs;
	linked->iDurationMs = occurrence.iDurationMs;
	*collider = occurrence;
	collider->strLogicOccurrenceId = windowId;
	for (auto& other : pattern->PresentationOccurrences)
		if (other.strLogicOccurrenceId == windowId)
		{ other.iStartMs = occurrence.iStartMs; other.iDurationMs = occurrence.iDurationMs; }
	if (!oldWindowId.empty() && oldWindowId != windowId &&
		std::none_of(pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(),
			[&](const auto& row) { return row.strLogicOccurrenceId == oldWindowId; }))
	{
		const auto* old = Find_LogicBox(*pattern, oldWindowId);
		const auto* oldLogic = nullptr == old ? nullptr : Find_Logic(candidate, old->strLogicId);
		if (nullptr != oldLogic && oldLogic->strJudgementKind == "AREA_OVERLAP")
			std::erase_if(pattern->LogicOccurrences, [&](const auto& row) { return row.strOccurrenceId == oldWindowId; });
	}
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Saved Trigger geometry, ENTER_AREA window and max-HP damage Result.", outStatus)) return false;
	m_strColliderExecutionEditId.clear();
	m_bColliderDamageDirty = false;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_PresentationBox(
	const std::string_view patternId, const std::string_view occurrenceId, std::string& outStatus)
{
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty()) return false;
	const auto removed = std::erase_if(pattern->PresentationOccurrences,
		[occurrenceId](const auto& row) { return row.strOccurrenceId == occurrenceId; });
	if (removed == 0u) { outStatus = m_strStatus = "Presentation box was not found."; return false; }
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Deleted presentation box.", outStatus)) return false;
	m_strSelectedPresentationOccurrenceId.clear();
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Queue_PresentationPreview(
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE* occurrence)
{
	if (nullptr != occurrence)
	{
		const auto finite = [](const auto& values, const double minimum, const double maximum) {
			return std::all_of(values.begin(), values.end(), [=](const double v) {
				return std::isfinite(v) && v >= minimum && v <= maximum; }); };
		if (occurrence->iDurationMs == 0u || occurrence->iDurationMs > MAX_EDITOR_TIME_MS ||
			static_cast<std::uint64_t>(occurrence->iFadeInMs) + occurrence->iFadeOutMs > occurrence->iDurationMs ||
			!std::isfinite(occurrence->fDissolveStart) || !std::isfinite(occurrence->fDissolveEnd) ||
			occurrence->fDissolveStart < 0.0 || occurrence->fDissolveStart >= occurrence->fDissolveEnd ||
			occurrence->fDissolveEnd > 1.0 || !finite(occurrence->PositionOffset, -100000.0, 100000.0) ||
			!finite(occurrence->RotationDegrees, -36000.0, 36000.0) || !finite(occurrence->Scale, 0.001, 10000.0) ||
			!std::isfinite(occurrence->fVolume) || occurrence->fVolume < 0.0 || occurrence->fVolume > 1.0)
		{
			m_strStatus = "Preview rejected: check lifetime, fade sum, dissolve range and finite transforms.";
			return;
		}
	}
	m_PendingPresentationPreviewRequest.Resource = resource;
	m_PendingPresentationPreviewRequest.Occurrence = nullptr != occurrence ? *occurrence :
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE{};
	if (nullptr == occurrence)
	{
		m_PendingPresentationPreviewRequest.Occurrence.strResourceId = resource.strResourceId;
		m_PendingPresentationPreviewRequest.Occurrence.iDurationMs = resource.iDurationMs;
		if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT)
		{
			m_PendingPresentationPreviewRequest.Occurrence.strAnchorKind = resource.strDefaultAnchorKind;
			m_PendingPresentationPreviewRequest.Occurrence.bFollowBoss = resource.strDefaultAnchorKind != "MAP";
		}
	}
	m_bPresentationPreviewRequestPending = true;
	m_strStatus = std::string(Presentation_Label(resource.eKind)) + " preview requested: " + resource.strDisplayName;
}

void Client::CKoukuSaydonActionWorkbench::Render_PresentationResources(const KOUKU_SAYDON_PRESENTATION_KIND kind)
{
	ImGui::PushID(static_cast<int>(kind));
	ImGui::SeparatorText(Presentation_Label(kind));
	if (ImGui::Button("Refresh Resources")) m_bPresentationResourceRefreshRequested = true;
	ImGui::TextWrapped("%s", m_strPresentationResourceStatus.c_str());
	std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> sources;
	const bool lightFamily = kind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT;
	const char* lightAnchors[] = { "MAP", "PLAYER", "BOSS" };
	if (lightFamily) ImGui::Combo("Category##LightResources", &m_iLightResourceCategory, "Map\0Character\0Boss\0");
	for (const auto& item : m_PresentationResourceInventory)
		if (item.eKind == kind && (!lightFamily || item.strDefaultAnchorKind == lightAnchors[m_iLightResourceCategory])) sources.push_back(item);
	if (kind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
	{
		for (int i = 0; i < 5; ++i)
		{
			KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE preset;
			preset.eKind = kind;
			preset.strResourceId = "preset.collider." + std::to_string(i);
			preset.strDisplayName = i == 0 ? "Rectangle" : i == 1 ? "Semicircle" : i == 2 ? "Sector" : i == 3 ? "Roulette Card Region" : "Circle";
			preset.strShape = i == 0 ? "BOX" : i == 4 ? "CIRCLE" : "SECTOR";
			preset.fHalfAngleDegrees = i == 1 ? 90.0 : i == 3 ? 22.5 : 45.0;
			preset.strColliderKind = i == 3 ? "ROULETTE_CARD_REGION" : "GEOMETRY";
			preset.strResourceKind.clear();
			sources.push_back(preset);
		}
		ImGui::TextDisabled("Region authoring and visual preview. Gameplay outcomes remain in explicit Logic definitions.");
	}
	ImGui::BeginChild("##PresentationSources", ImVec2(0.f, 180.f), ImGuiChildFlags_Borders);
	for (const auto& item : sources)
	{
		const std::string id = std::to_string(static_cast<int>(item.eKind)) + ":" + item.strResourceKind + ":" +
			(item.strAssetId.empty() ? item.strResourceId : item.strAssetId);
		const std::string label = (kind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ? item.strResourceKind + "  " : "") +
			item.strDisplayName + "##" + id;
		if (ImGui::Selectable(label.c_str(), m_strSelectedPresentationSourceId == id))
		{
			m_strSelectedPresentationSourceId = id;
			(void)Copy_Text(m_NewPresentationName, std::size(m_NewPresentationName), item.strDisplayName);
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s | native %u ms", item.strAssetId.c_str(), item.iDurationMs);
	}
	ImGui::EndChild();
	const auto source = std::find_if(sources.begin(), sources.end(), [&](const auto& item) {
		return m_strSelectedPresentationSourceId == std::to_string(static_cast<int>(item.eKind)) + ":" + item.strResourceKind + ":" +
			(item.strAssetId.empty() ? item.strResourceId : item.strAssetId); });
	ImGui::InputText("Display name", m_NewPresentationName, std::size(m_NewPresentationName));
	ImGui::BeginDisabled(source == sources.end());
	if (ImGui::Button("Preview Source") && source != sources.end()) Queue_PresentationPreview(*source);
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_bHasDraft || m_NewPresentationName[0] == '\0');
	if (ImGui::Button("Create") && source != sources.end())
	{
		std::string status;
		(void)Create_PresentationResource(*source, m_NewPresentationName, status);
	}
	ImGui::EndDisabled();
	ImGui::EndDisabled();
	if (lightFamily)
	{
		const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
		ImGui::BeginDisabled(source == sources.end() || nullptr == pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0u);
		if (ImGui::Button("Append selected Light") && source != sources.end())
		{
			std::string status;
			const auto existing = std::find_if(m_Draft.PresentationResources.begin(), m_Draft.PresentationResources.end(),
				[&](const auto& item) { return item.eKind == kind && item.strAssetId == source->strAssetId; });
			if (existing != m_Draft.PresentationResources.end())
			{ const std::string id = existing->strResourceId; (void)Append_PresentationBox(id, status); }
			else if (Create_PresentationResource(*source, source->strDisplayName, status))
				(void)Append_PresentationBox(m_strSelectedPresentationResourceId, status);
		}
		ImGui::EndDisabled();
		ImGui::TextDisabled("Light shape and RGB are saved in Rendering Workbench. Boxes own timing and anchor.");
	}
	ImGui::SeparatorText("Created Resources");
	for (const auto& item : m_Draft.PresentationResources)
	{
		if (item.eKind != kind || (lightFamily && item.strDefaultAnchorKind != lightAnchors[m_iLightResourceCategory])) continue;
		const auto label = item.strDisplayName + "##" + item.strResourceId;
		if (ImGui::Selectable(label.c_str(), m_strSelectedPresentationResourceId == item.strResourceId))
			m_strSelectedPresentationResourceId = item.strResourceId;
	}
	const auto* selected = Find_PresentationResource(m_Draft, m_strSelectedPresentationResourceId);
	if (nullptr != selected && selected->eKind == kind)
	{
		const auto resource = *selected;
		if (ImGui::Button("Preview")) Queue_PresentationPreview(resource);
		ImGui::SameLine();
		const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
		ImGui::BeginDisabled(nullptr == pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0u);
		if (ImGui::Button("Append at Cursor")) { std::string status; (void)Append_PresentationBox(resource.strResourceId, status); }
		ImGui::EndDisabled();
	}
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::PopID();
}

void Client::CKoukuSaydonActionWorkbench::Render_PresentationBoxDetails(
	const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
{
	const auto* box = Find_PresentationBox(pattern, m_strSelectedPresentationOccurrenceId);
	if (nullptr == box) return;
	const auto* resource = Find_PresentationResource(m_Draft, box->strResourceId);
	if (nullptr == resource) return;
	const auto definition = *resource;
	const auto patternId = pattern.strPatternId;
	const auto occurrenceId = box->strOccurrenceId;
	ImGui::SeparatorText(Presentation_Label(definition.eKind));
	ImGui::TextWrapped("%s", definition.strDisplayName.c_str());
	ImGui::TextDisabled("%s", occurrenceId.c_str());
	auto& edit = m_PresentationBoxEdit;
	if (edit.strOccurrenceId != occurrenceId) edit = *box;
	int start = static_cast<int>(edit.iStartMs), duration = static_cast<int>(edit.iDurationMs);
	if (ImGui::InputInt("Start ms##PresentationBox", &start)) edit.iStartMs = static_cast<std::uint32_t>((std::max)(0, start));
	if (ImGui::InputInt("Lifetime ms##PresentationBox", &duration)) edit.iDurationMs = static_cast<std::uint32_t>((std::max)(1, duration));
	const auto vectorControl = [](const char* label, std::array<double, 3u>& values, const float minimum, const float maximum) {
		float v[3] = { static_cast<float>(values[0]), static_cast<float>(values[1]), static_cast<float>(values[2]) };
		if (ImGui::DragFloat3(label, v, 0.05f, minimum, maximum, "%.3f"))
			for (std::size_t i = 0u; i < 3u; ++i) values[i] = v[i];
	};
	vectorControl("Position offset (m)##PresentationBox", edit.PositionOffset, -100000.f, 100000.f);
	vectorControl("Rotation (degrees)##PresentationBox", edit.RotationDegrees, -36000.f, 36000.f);
	const bool light = definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT;
	if (!light) vectorControl("Scale / size##PresentationBox", edit.Scale, 0.001f, 10000.f);
	if (light)
	{
		const char* kinds[] = { "MAP", "PLAYER", "BOSS" };
		const char* labels[] = { "Map", "Character", "Boss" };
		int selected = edit.strAnchorKind == "MAP" ? 0 : edit.strAnchorKind == "PLAYER" ? 1 : 2;
		if (ImGui::Combo("Anchor##LightBox", &selected, labels, 3))
		{
			edit.strAnchorKind = kinds[selected]; edit.strWorldId.clear(); edit.strBone.clear();
			edit.bFollowBoss = selected != 0;
		}
		if (edit.strAnchorKind == "MAP") ImGui::TextDisabled("Map uses a fixed world position from the resource plus this offset.");
		if (edit.strAnchorKind == "PLAYER") ImGui::TextDisabled("One following light per living replicated character.");
		float brightness = static_cast<float>(edit.fBrightnessMultiplier);
		if (ImGui::DragFloat("Brightness multiplier", &brightness, 0.05f, 0.f, 16.f)) edit.fBrightnessMultiplier = brightness;
		int fadeIn = static_cast<int>(edit.iFadeInMs), fadeOut = static_cast<int>(edit.iFadeOutMs);
		if (ImGui::InputInt("Fade in ms##Light", &fadeIn)) edit.iFadeInMs = static_cast<std::uint32_t>((std::max)(0, fadeIn));
		if (ImGui::InputInt("Fade out ms##Light", &fadeOut)) edit.iFadeOutMs = static_cast<std::uint32_t>((std::max)(0, fadeOut));
		ImGui::Checkbox("Debug wire##Light", &edit.bDebugRender);
		if (edit.strAnchorKind == "BOSS")
		{
			char bone[128]{};
			(void)Copy_Text(bone, std::size(bone), edit.strBone);
			if (ImGui::InputText("Bone (empty = pivot)##Light", bone, std::size(bone))) edit.strBone = bone;
		}
	}
	if (!light || edit.strAnchorKind == "BOSS") ImGui::Checkbox("Follow anchor##PresentationBox", &edit.bFollowBoss);
	if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
	{
		int fadeIn = static_cast<int>(edit.iFadeInMs), fadeOut = static_cast<int>(edit.iFadeOutMs);
		if (ImGui::InputInt("Fade in ms", &fadeIn)) edit.iFadeInMs = static_cast<std::uint32_t>((std::max)(0, fadeIn));
		if (ImGui::InputInt("Fade out ms", &fadeOut)) edit.iFadeOutMs = static_cast<std::uint32_t>((std::max)(0, fadeOut));
		float startRatio = static_cast<float>(edit.fDissolveStart), endRatio = static_cast<float>(edit.fDissolveEnd);
		ImGui::BeginDisabled(edit.iFadeOutMs == 0u);
		if (ImGui::SliderFloat("Dissolve out start", &startRatio, 0.f, 1.f)) edit.fDissolveStart = startRatio;
		if (ImGui::SliderFloat("Dissolve out end", &endRatio, 0.f, 1.f)) edit.fDissolveEnd = endRatio;
		ImGui::EndDisabled();
		char bone[128]{};
		(void)Copy_Text(bone, std::size(bone), edit.strBone);
		if (ImGui::InputText("Bone (empty = pivot)", bone, std::size(bone))) edit.strBone = bone;
		ImGui::TextDisabled("Fade 0 keeps authored alpha/dissolve. Positive fades override; dissolve-out follows Fade Out.");
	}
	if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::SOUND)
	{
		float volume = static_cast<float>(edit.fVolume);
		if (ImGui::SliderFloat("Volume", &volume, 0.f, 1.f)) edit.fVolume = volume;
	}
	if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
	{
		const auto* linkedBox = Find_LogicBox(pattern, edit.strLogicOccurrenceId);
		const auto* linkedLogic = nullptr == linkedBox ? nullptr : Find_Logic(m_Draft, linkedBox->strLogicId);
		if (m_strColliderExecutionEditId != occurrenceId)
		{
			m_strColliderExecutionEditId = occurrenceId;
			m_strColliderExecutionType = nullptr == linkedLogic ? "DURATION" : linkedLogic->strLogicType;
			m_strColliderLogicDefinitionId = nullptr == linkedLogic ? "" : linkedLogic->strLogicId;
			m_iColliderDamagePercent = 10;
			m_bColliderDamageDirty = false;
			if (nullptr != linkedBox)
				for (const auto& id : linkedBox->OnSuccessLogicIds)
					if (const auto* result = Find_Logic(m_Draft, id); result && result->strOutcomeKind == "MAX_HP_PERCENT_DAMAGE")
					{ m_iColliderDamagePercent = static_cast<int32_t>(result->iPercent); break; }
		}
		if (ImGui::BeginCombo("Execution", m_strColliderExecutionType.c_str()))
		{
			for (const char* type : { "DURATION", "TRIGGER" })
				if (ImGui::Selectable(type, m_strColliderExecutionType == type))
				{
					m_strColliderExecutionType = type;
					m_strColliderLogicDefinitionId.clear();
				}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("Shared Logic window", nullptr == linkedBox ? "(none)" : linkedBox->strOccurrenceId.c_str()))
		{
			if (ImGui::Selectable("(none)", edit.strLogicOccurrenceId.empty())) edit.strLogicOccurrenceId.clear();
			for (const auto& window : pattern.LogicOccurrences)
			{
				const auto* logic = Find_Logic(m_Draft, window.strLogicId);
				if (nullptr == logic || logic->strLogicType != m_strColliderExecutionType ||
					(logic->strLogicType == "TRIGGER" && logic->strTriggerKind != "ENTER_AREA")) continue;
				const auto label = logic->strDisplayName + " | " + window.strOccurrenceId;
				if (ImGui::Selectable(label.c_str(), edit.strLogicOccurrenceId == window.strOccurrenceId))
				{
					edit.strLogicOccurrenceId = window.strOccurrenceId;
					edit.iStartMs = window.iStartMs;
					edit.iDurationMs = window.iDurationMs;
				}
			}
			ImGui::EndCombo();
		}
		const auto* definitionLogic = Find_Logic(m_Draft, m_strColliderLogicDefinitionId);
		if (ImGui::BeginCombo("Logic definition", nullptr == definitionLogic ? "(select Logic)" : definitionLogic->strDisplayName.c_str()))
		{
			for (const auto& logic : m_Draft.Logics)
				if (logic.strLogicType == m_strColliderExecutionType &&
					(logic.strLogicType != "TRIGGER" || logic.strTriggerKind == "ENTER_AREA") &&
					ImGui::Selectable((logic.strDisplayName + "##" + logic.strLogicId).c_str(), m_strColliderLogicDefinitionId == logic.strLogicId))
					m_strColliderLogicDefinitionId = logic.strLogicId;
			ImGui::EndCombo();
		}
		ImGui::BeginDisabled(m_strColliderLogicDefinitionId.empty());
		if (ImGui::Button("Append / reuse Logic window"))
		{
			std::string status;
			const auto value = edit;
			(void)Connect_ColliderLogic(patternId, value, m_strColliderLogicDefinitionId, status);
			ImGui::EndDisabled();
			return;
		}
		ImGui::EndDisabled();
		bool debugRender = edit.bDebugRender;
		if (ImGui::Checkbox("Debug Render##ColliderBox", &debugRender))
		{
			std::string status;
			(void)Set_PresentationBoxDebugRender(patternId, occurrenceId, debugRender, status);
			return;
		}
		ImGui::TextWrapped("A linked Collider shares its start/lifetime and Result slots with one Logic window. Eight roulette regions can share that same window.");
		if (m_strColliderExecutionType == "TRIGGER")
		{
			if (ImGui::InputInt("Damage (% max HP)", &m_iColliderDamagePercent, 1, 10))
			{ m_iColliderDamagePercent = std::clamp(m_iColliderDamagePercent, 1, 100); m_bColliderDamageDirty = true; }
			ImGui::BeginDisabled(definition.strColliderKind != "GEOMETRY");
			if (ImGui::Button("Create / update Trigger damage"))
			{
				std::string status;
				const auto value = edit;
				(void)Set_ColliderTriggerDamage(patternId, value, static_cast<std::uint32_t>(m_iColliderDamagePercent), status);
				ImGui::EndDisabled();
				return;
			}
			ImGui::EndDisabled();
			ImGui::TextWrapped("Creates or reuses ENTER_AREA and a max-HP damage Result together. First entry runs Success once; no entry runs Timeout. Other Result slots are preserved.");
		}
		if (nullptr != linkedLogic && linkedLogic->strJudgementKind == "AREA_OVERLAP" &&
			ImGui::BeginCombo("Inside outcome##ColliderBox", linkedLogic->strInsideOutcome.c_str()))
		{
			for (const char* outcome : { "SUCCESS", "FAIL" })
				if (ImGui::Selectable(outcome, linkedLogic->strInsideOutcome == outcome))
				{
					auto value = *linkedLogic;
					value.strInsideOutcome = outcome;
					std::string status;
					(void)Set_LogicDefinitionValues(value.strLogicId, value, status);
					ImGui::EndCombo();
					return;
				}
			ImGui::EndCombo();
		}
		if (!box->strLogicOccurrenceId.empty() && Render_LogicOutcomeSlots(patternId, box->strLogicOccurrenceId)) return;
		std::string colliderKind = definition.strColliderKind;
		if (ImGui::BeginCombo("Collider kind", colliderKind.c_str()))
		{
			for (const char* kind : { "GEOMETRY", "ROULETTE_CARD_REGION" })
				if (ImGui::Selectable(kind, colliderKind == kind)) colliderKind = kind;
			ImGui::EndCombo();
		}
		if (colliderKind != definition.strColliderKind)
		{
			auto candidate = m_Draft;
			for (auto& item : candidate.PresentationResources)
				if (item.strResourceId == definition.strResourceId) item.strColliderKind = colliderKind;
			for (auto& affected : candidate.Patterns)
				if (std::any_of(affected.PresentationOccurrences.begin(), affected.PresentationOccurrences.end(),
					[&](const auto& row) { return row.strResourceId == definition.strResourceId; })) Mark_Draft(candidate, affected);
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated shared Collider resource kind.", status);
			return;
		}
		if (ImGui::BeginCombo("Anchor", edit.strAnchorKind.c_str()))
		{
			for (const char* kind : { "BOSS", "WORLD" })
				if (ImGui::Selectable(kind, edit.strAnchorKind == kind))
				{
					edit.strAnchorKind = kind;
					if (edit.strAnchorKind == "BOSS") edit.strWorldId.clear();
				}
			ImGui::EndCombo();
		}
		if (edit.strAnchorKind == "WORLD" && ImGui::BeginCombo("World anchor", edit.strWorldId.empty() ? "(select World)" : edit.strWorldId.c_str()))
		{
			for (const auto& world : m_Draft.Worlds)
				if (ImGui::Selectable((world.strDisplayName + "##" + world.strWorldId).c_str(), edit.strWorldId == world.strWorldId)) edit.strWorldId = world.strWorldId;
			ImGui::EndCombo();
		}
		if (definition.strColliderKind == "ROULETTE_CARD_REGION")
		{
			char regionId[256]{};
			(void)Copy_Text(regionId, std::size(regionId), edit.strRegionId);
			if (ImGui::InputText("Region ID", regionId, std::size(regionId))) edit.strRegionId = regionId;
			if (ImGui::BeginCombo("Card symbol", edit.strCardSymbol.c_str()))
			{
				for (const char* symbol : { "NONE", "HEART", "SPADE", "CLUB", "DIAMOND" })
					if (ImGui::Selectable(symbol, edit.strCardSymbol == symbol)) edit.strCardSymbol = symbol;
				ImGui::EndCombo();
			}
			if (ImGui::BeginCombo("Card color", edit.strCardColor.c_str()))
			{
				for (const char* color : { "NONE", "RED", "BLACK" })
					if (ImGui::Selectable(color, edit.strCardColor == color)) edit.strCardColor = color;
				ImGui::EndCombo();
			}
			ImGui::TextWrapped("Map the observed floor symbol and color, then connect this Region ID in Roulette Logic. Unmapped regions stay DRAFT.");
		}
		if (definition.strShape == "BOX")
		{
			float fullSize[3]{};
			for (std::size_t i = 0u; i < 3u; ++i) fullSize[i] = static_cast<float>(2.0 * definition.HalfExtents[i] * edit.Scale[i]);
			if (ImGui::DragFloat3("Width / height / depth (m)", fullSize, 0.05f, 0.01f, 10000.f))
				for (std::size_t i = 0u; i < 3u; ++i) edit.Scale[i] = fullSize[i] / (2.0 * definition.HalfExtents[i]);
		}
		else
		{
			float radius = static_cast<float>(definition.fRadiusM * edit.Scale[0]);
			if (ImGui::DragFloat("Radius (m)", &radius, 0.05f, 0.01f, 10000.f))
				edit.Scale[0] = edit.Scale[2] = radius / definition.fRadiusM;
			if (definition.strShape == "SECTOR") ImGui::Text("Sector angle: %.1f degrees", 2.0 * definition.fHalfAngleDegrees);
		}
		ImGui::TextDisabled("Dimensions affect this box. Rotation sets the region direction.");
	}
	if (ImGui::Button("Apply##PresentationBox"))
	{
		std::string status;
		const auto value = edit;
		if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER &&
			m_strColliderExecutionType == "TRIGGER" && m_bColliderDamageDirty)
			(void)Set_ColliderTriggerDamage(patternId, value, static_cast<std::uint32_t>(m_iColliderDamagePercent), status);
		else (void)Set_PresentationBox(patternId, value, status);
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("Preview##PresentationBox")) Queue_PresentationPreview(definition, &edit);
	ImGui::SameLine();
	if (ImGui::Button("Delete##PresentationBox"))
	{
		std::string status;
		(void)Delete_PresentationBox(patternId, occurrenceId, status);
	}
	ImGui::TextWrapped("%s", m_strStatus.c_str());
}

void Client::CKoukuSaydonActionWorkbench::Render_WorldResources()
{
	ImGui::SeparatorText("World Catalog");
	ImGui::TextDisabled("%zu worlds | Create names an authored world sequence of the arena; Append starts it on the Pattern clock.",
		m_Draft.Worlds.size());
	if (!m_strWorldSequenceResourceStatus.empty())
		ImGui::TextWrapped("%s", m_strWorldSequenceResourceStatus.c_str());
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const selectedPattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool_t patternEditable = nullptr != selectedPattern &&
		selectedPattern->strLoadError.empty();
	const std::string targetPatternName = nullptr == selectedPattern ?
		std::string("none") : selectedPattern->strDisplayName;

	if (ImGui::BeginChild("##KoukuWorldList", ImVec2(0.f, 140.f), ImGuiChildFlags_Borders))
	{
		if (m_Draft.Worlds.empty())
			ImGui::TextDisabled("No World yet. Name it, pick a world sequence and press Create World.");
		for (const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world : m_Draft.Worlds)
		{
			const std::string label = world.strDisplayName + "##" + world.strWorldId;
			if (ImGui::Selectable(label.c_str(), m_strSelectedWorldId == world.strWorldId))
				m_strSelectedWorldId = world.strWorldId;
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s | %s | %zu box(es)", world.strWorldId.c_str(),
					world.strSequenceInstanceId.c_str(), Count_WorldReferences(m_Draft, world.strWorldId));
		}
	}
	ImGui::EndChild();

	ImGui::SeparatorText("Saved World Object states");
	ImGui::InputTextWithHint("##NewKoukuWorldName", "World display name (e.g. \xEC\x9B\x94\xEB\x93\x9C_\xEB\xA3\xB0\xEB\xA0\x9B)",
		m_NewWorldName, std::size(m_NewWorldName));
	const auto findResource = [this](const std::string& instanceId) -> const KOUKU_WORLD_SEQUENCE_RESOURCE*
	{
		const auto found = std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(),
			[&instanceId](const KOUKU_WORLD_SEQUENCE_RESOURCE& resource)
			{ return resource.strInstanceId == instanceId; });
		return found == m_WorldSequenceResources.end() ? nullptr : &*found;
	};
	const KOUKU_WORLD_SEQUENCE_RESOURCE* const newResource = findResource(m_strNewWorldSequenceInstanceId);
	if (ImGui::BeginCombo("Sequence##NewKoukuWorld",
			nullptr == newResource ? "(pick a world sequence)" : newResource->strDisplayName.c_str()))
	{
		if (m_WorldSequenceResources.empty())
			ImGui::TextDisabled("Enter the KoukuSaydon Arena to list its world sequences.");
		for (const KOUKU_WORLD_SEQUENCE_RESOURCE& resource : m_WorldSequenceResources)
		{
			const std::string item = resource.strDisplayName + "##" + resource.strInstanceId;
			if (ImGui::Selectable(item.c_str(), m_strNewWorldSequenceInstanceId == resource.strInstanceId))
				m_strNewWorldSequenceInstanceId = resource.strInstanceId;
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s | %u ms", resource.strInstanceId.c_str(), resource.iDurationMs);
		}
		ImGui::EndCombo();
	}
	ImGui::BeginDisabled(nullptr == newResource || !patternEditable);
	if (ImGui::Button("Append selected World Object at Cursor"))
	{
		std::string status;
		(void)Append_WorldResource(m_strNewWorldSequenceInstanceId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled("Create and tune objects/states in World Object Tool, Save, then select and Append here.");
	ImGui::BeginDisabled(nullptr == newResource);
	if (ImGui::Button("Preview Sequence##NewWorld") && nullptr != newResource)
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE preview;
		preview.eKind = KOUKU_SAYDON_PRESENTATION_KIND::WORLD;
		preview.strAssetId = newResource->strInstanceId;
		preview.strDisplayName = newResource->strDisplayName;
		preview.iDurationMs = (std::max)(1u, newResource->iDurationMs);
		Queue_PresentationPreview(preview);
	}
	ImGui::EndDisabled();
	ImGui::BeginDisabled('\0' == m_NewWorldName[0] || !m_bHasDraft || nullptr == newResource);
	if (ImGui::Button("Create World"))
	{
		std::string worldId;
		std::string status;
		if (Create_World(m_NewWorldName, m_strNewWorldSequenceInstanceId, worldId, status))
			m_NewWorldName[0] = '\0';
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();

	const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION* const selectedWorld =
		Find_World(m_Draft, m_strSelectedWorldId);
	if (nullptr == selectedWorld)
		return;
	const std::string worldId = selectedWorld->strWorldId;
	const std::string worldName = selectedWorld->strDisplayName;
	const std::string instanceId = selectedWorld->strSequenceInstanceId;
	bool_t unresolvedReferences = false;
	const std::size_t references = Count_WorldReferences(m_Draft, worldId, &unresolvedReferences);
	const KOUKU_WORLD_SEQUENCE_RESOURCE* const resource = findResource(instanceId);
	ImGui::SeparatorText("Selected World");
	ImGui::TextWrapped("%s", worldName.c_str());
	ImGui::TextDisabled("%s | %s | %zu box(es)", worldId.c_str(), instanceId.c_str(), references);
	if (nullptr != resource)
		ImGui::TextDisabled("Sequence %u ms%s", resource->iDurationMs,
			resource->bHasBoundPlacement ? " | bound to a map placement" : "");
	else
		ImGui::TextDisabled("The sequence list is empty here; enter the arena to see its length.");
	if (Render_WorldCompanionSelector(*selectedWorld)) return;
	if (ImGui::Button("Preview World"))
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE preview;
		preview.eKind = KOUKU_SAYDON_PRESENTATION_KIND::WORLD;
		preview.strResourceId = worldId;
		preview.strAssetId = instanceId;
		preview.strDisplayName = worldName;
		preview.iDurationMs = nullptr != resource ? (std::max)(1u, resource->iDurationMs) : 1000u;
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE occurrence;
		occurrence.strResourceId = worldId;
		occurrence.iDurationMs = preview.iDurationMs;
		occurrence.PositionOffset = selectedWorld->PositionOffset;
		Queue_PresentationPreview(preview, &occurrence);
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(!patternEditable);
	if (ImGui::Button("Append World at Cursor"))
	{
		std::string occurrenceId;
		std::string status;
		(void)Append_WorldBox(m_strSelectedPatternId, worldId, m_iCursorMs,
			nullptr != resource && resource->iDurationMs > 0u ? resource->iDurationMs : 1000u,
			occurrenceId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(0u != references || unresolvedReferences);
	if (ImGui::Button("Delete World"))
	{
		std::string status;
		(void)Delete_World(worldId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	if (!patternEditable)
		ImGui::TextDisabled("Select an editable Pattern in Patterns to append this World.");
	ImGui::TextDisabled("Append target: %s | cursor %u ms", targetPatternName.c_str(), m_iCursorMs);
}

void Client::CKoukuSaydonActionWorkbench::Render_WorldBoxDetails(
	const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
{
	const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE* const box =
		Find_WorldBox(pattern, m_strSelectedWorldOccurrenceId);
	if (nullptr == box)
		return;
	const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION* const world =
		Find_World(m_Draft, box->strWorldId);
	const std::string patternId = pattern.strPatternId;
	const std::string occurrenceId = box->strOccurrenceId;
	const std::uint32_t lifetimeMs = Pattern_DurationMs(pattern);
	ImGui::SeparatorText("World Box");
	ImGui::Text("%s", occurrenceId.c_str());
	ImGui::Text("%s", nullptr == world ? "(missing World)" : world->strDisplayName.c_str());
	ImGui::TextDisabled("%s -> %s", box->strWorldId.c_str(),
		nullptr == world ? "?" : world->strSequenceInstanceId.c_str());
	if (nullptr != world)
	{
		if (Render_WorldCompanionSelector(*world)) return;
		float offset[3] = { static_cast<float>(world->PositionOffset[0]),
			static_cast<float>(world->PositionOffset[1]), static_cast<float>(world->PositionOffset[2]) };
		float anchor[3] = { static_cast<float>(world->AnchorPosition[0]),
			static_cast<float>(world->AnchorPosition[1]), static_cast<float>(world->AnchorPosition[2]) };
		bool anchored = world->strAnchorKind == "BOSS_SPAWN";
		bool changed = ImGui::Checkbox("Anchor to boss spawn##KoukuWorldBox", &anchored);
		if (anchored)
			changed |= ImGui::DragFloat3("Source anchor (m)##KoukuWorldBox", anchor, 0.05f, -100000.f, 100000.f, "%.3f");
		changed |= ImGui::DragFloat3("World position offset (m)##KoukuWorldBox", offset,
			0.05f, -100000.f, 100000.f, "%.3f");
		if (changed)
		{
			const std::string worldId = world->strWorldId;
			auto candidate = m_Draft;
			const auto edited = std::find_if(candidate.Worlds.begin(), candidate.Worlds.end(),
				[&worldId](const auto& item) { return item.strWorldId == worldId; });
			if (edited != candidate.Worlds.end())
			{
				edited->PositionOffset = { offset[0], offset[1], offset[2] };
				edited->AnchorPosition = { anchor[0], anchor[1], anchor[2] };
				edited->strAnchorKind = anchored ? "BOSS_SPAWN" : "NONE";
				for (auto& affected : candidate.Patterns)
					if (std::any_of(affected.WorldOccurrences.begin(), affected.WorldOccurrences.end(),
						[&worldId](const auto& item) { return item.strWorldId == worldId; }))
						Mark_Draft(candidate, affected);
				std::string status;
				(void)Commit_Candidate(std::move(candidate), "Updated World anchor and offset. Save keeps this World tuning.", status);
			}
			return;
		}
		ImGui::TextDisabled("Spawn anchor offset = boss spawn - source anchor + position offset. Applies to this World definition.");
	}
	ImGui::InputInt("Start ms##KoukuWorldBox", &m_iWorldBoxStartMs, 10, 100);
	ImGui::InputInt("Lifetime ms##KoukuWorldBox", &m_iWorldBoxDurationMs, 10, 100);
	ImGui::InputFloat("Playback speed x##KoukuWorldBox", &m_fWorldBoxPlaybackSpeed, 0.05f, 0.25f, "%.2f");
	m_iWorldBoxStartMs = std::clamp(m_iWorldBoxStartMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_iWorldBoxDurationMs = std::clamp(m_iWorldBoxDurationMs, 1, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_fWorldBoxPlaybackSpeed = std::clamp(m_fWorldBoxPlaybackSpeed, 0.05f, 16.f);
	if (ImGui::Button("Apply##KoukuWorldBox"))
	{
		std::string status;
		(void)Set_WorldBoxWindow(patternId, occurrenceId,
			static_cast<std::uint32_t>(m_iWorldBoxStartMs),
			static_cast<std::uint32_t>(m_iWorldBoxDurationMs),
			m_fWorldBoxPlaybackSpeed, status);
		return;
	}
	ImGui::TextWrapped("Preview and Server Play start the sequence at Start ms with the World offset and playback speed. Lifetime ms limits the active World box; sequence motion uses its authored length and speed. Pattern lifetime %u ms.",
		lifetimeMs);
	if (ImGui::Button("Delete World Box"))
	{
		std::string status;
		(void)Delete_WorldBox(patternId, occurrenceId, status);
	}
}

void Client::CKoukuSaydonActionWorkbench::Render_SceneProfileResources()
{
	ImGui::SeparatorText("Scene Profile Catalog");
	ImGui::TextDisabled("%zu scene profiles | Create names a rendering profile; Append applies it for a window of the Pattern.",
		m_Draft.SceneProfiles.size());
	if (!m_strRenderingProfileResourceStatus.empty())
		ImGui::TextWrapped("%s", m_strRenderingProfileResourceStatus.c_str());
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const selectedPattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool_t patternEditable = nullptr != selectedPattern &&
		selectedPattern->strLoadError.empty();
	const std::string targetPatternName = nullptr == selectedPattern ?
		std::string("none") : selectedPattern->strDisplayName;
	const std::uint32_t lifetimeMs = nullptr == selectedPattern ? 0u : Pattern_DurationMs(*selectedPattern);

	if (ImGui::BeginChild("##KoukuSceneProfileList", ImVec2(0.f, 140.f), ImGuiChildFlags_Borders))
	{
		if (m_Draft.SceneProfiles.empty())
			ImGui::TextDisabled("No Scene Profile yet. Name it, pick a rendering profile and press Create Scene Profile.");
		for (const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION& profile : m_Draft.SceneProfiles)
		{
			const std::string label = profile.strDisplayName + "##" + profile.strSceneProfileId;
			if (ImGui::Selectable(label.c_str(), m_strSelectedSceneProfileId == profile.strSceneProfileId))
				m_strSelectedSceneProfileId = profile.strSceneProfileId;
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s | %s | %zu box(es)", profile.strSceneProfileId.c_str(),
					profile.strRenderingProfileId.c_str(),
					Count_SceneProfileReferences(m_Draft, profile.strSceneProfileId));
		}
	}
	ImGui::EndChild();

	ImGui::SeparatorText("Create Scene Profile");
	ImGui::InputTextWithHint("##NewKoukuSceneProfileName", "Scene Profile display name",
		m_NewSceneProfileName, std::size(m_NewSceneProfileName));
	if (ImGui::BeginCombo("Rendering profile##NewKoukuSceneProfile",
			m_strNewSceneProfileRenderingId.empty() ? "(pick a rendering profile)" : m_strNewSceneProfileRenderingId.c_str()))
	{
		if (m_RenderingProfileIds.empty())
			ImGui::TextDisabled("No rendering profile is loaded.");
		for (const std::string& profileId : m_RenderingProfileIds)
		{
			if (ImGui::Selectable(profileId.c_str(), m_strNewSceneProfileRenderingId == profileId))
				m_strNewSceneProfileRenderingId = profileId;
		}
		ImGui::EndCombo();
	}
	ImGui::BeginDisabled(m_strNewSceneProfileRenderingId.empty());
	if (ImGui::Button("Preview Rendering Profile##NewSceneProfile"))
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE preview;
		preview.eKind = KOUKU_SAYDON_PRESENTATION_KIND::SCENE_PROFILE;
		preview.strAssetId = m_strNewSceneProfileRenderingId;
		preview.strDisplayName = m_strNewSceneProfileRenderingId;
		preview.iDurationMs = 3000u;
		Queue_PresentationPreview(preview);
	}
	ImGui::EndDisabled();
	ImGui::BeginDisabled('\0' == m_NewSceneProfileName[0] || !m_bHasDraft ||
		m_strNewSceneProfileRenderingId.empty());
	if (ImGui::Button("Create Scene Profile"))
	{
		std::string sceneProfileId;
		std::string status;
		if (Create_SceneProfile(m_NewSceneProfileName, m_strNewSceneProfileRenderingId, sceneProfileId, status))
			m_NewSceneProfileName[0] = '\0';
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();

	const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION* const selectedProfile =
		Find_SceneProfile(m_Draft, m_strSelectedSceneProfileId);
	if (nullptr == selectedProfile)
		return;
	const std::string sceneProfileId = selectedProfile->strSceneProfileId;
	const std::string profileName = selectedProfile->strDisplayName;
	const std::string renderingId = selectedProfile->strRenderingProfileId;
	bool_t unresolvedReferences = false;
	const std::size_t references = Count_SceneProfileReferences(m_Draft, sceneProfileId, &unresolvedReferences);
	ImGui::SeparatorText("Selected Scene Profile");
	ImGui::TextWrapped("%s", profileName.c_str());
	ImGui::TextDisabled("%s | %s | %zu box(es)", sceneProfileId.c_str(), renderingId.c_str(), references);
	if (!m_RenderingProfileIds.empty() &&
		m_RenderingProfileIds.end() == std::find(m_RenderingProfileIds.begin(), m_RenderingProfileIds.end(), renderingId))
	{
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.24f, 1.f),
			"This rendering profile is not in the loaded runtime catalog; publish RenderingProfiles first.");
	}
	if (ImGui::Button("Preview Scene Profile"))
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE preview;
		preview.eKind = KOUKU_SAYDON_PRESENTATION_KIND::SCENE_PROFILE;
		preview.strResourceId = sceneProfileId;
		preview.strAssetId = renderingId;
		preview.strDisplayName = profileName;
		preview.iDurationMs = 3000u;
		Queue_PresentationPreview(preview);
	}
	ImGui::Checkbox("Until Pattern end##NewKoukuSceneProfileBox", &m_bNewSceneProfileBoxToPatternEnd);
	if (!m_bNewSceneProfileBoxToPatternEnd)
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(120.f);
		ImGui::InputInt("Box ms##NewKoukuSceneProfileBox", &m_iNewSceneProfileBoxDurationMs, 100, 1000);
		m_iNewSceneProfileBoxDurationMs = std::clamp(m_iNewSceneProfileBoxDurationMs, 1,
			static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	}
	ImGui::BeginDisabled(!patternEditable);
	if (ImGui::Button("Append Scene Profile at Cursor"))
	{
		const std::uint32_t durationMs = m_bNewSceneProfileBoxToPatternEnd ?
			(lifetimeMs > m_iCursorMs ? lifetimeMs - m_iCursorMs : 1u) :
			static_cast<std::uint32_t>(m_iNewSceneProfileBoxDurationMs);
		std::string occurrenceId;
		std::string status;
		(void)Append_SceneProfileBox(m_strSelectedPatternId, sceneProfileId, m_iCursorMs,
			durationMs, occurrenceId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(0u != references || unresolvedReferences);
	if (ImGui::Button("Delete Scene Profile"))
	{
		std::string status;
		(void)Delete_SceneProfile(sceneProfileId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	if (!patternEditable)
		ImGui::TextDisabled("Select an editable Pattern in Patterns to append this Scene Profile.");
	ImGui::TextDisabled("Append target: %s | cursor %u ms | lifetime %u ms",
		targetPatternName.c_str(), m_iCursorMs, lifetimeMs);
}

void Client::CKoukuSaydonActionWorkbench::Render_SceneProfileBoxDetails(
	const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
{
	const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE* const box =
		Find_SceneProfileBox(pattern, m_strSelectedSceneProfileOccurrenceId);
	if (nullptr == box)
		return;
	const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION* const profile =
		Find_SceneProfile(m_Draft, box->strSceneProfileId);
	const std::string patternId = pattern.strPatternId;
	const std::string occurrenceId = box->strOccurrenceId;
	const std::uint32_t lifetimeMs = Pattern_DurationMs(pattern);
	ImGui::SeparatorText("Scene Profile Box");
	ImGui::Text("%s", occurrenceId.c_str());
	ImGui::Text("%s", nullptr == profile ? "(missing Scene Profile)" : profile->strDisplayName.c_str());
	ImGui::TextDisabled("%s -> %s", box->strSceneProfileId.c_str(),
		nullptr == profile ? "?" : profile->strRenderingProfileId.c_str());
	ImGui::InputInt("Start ms##KoukuSceneProfileBox", &m_iSceneProfileBoxStartMs, 10, 100);
	ImGui::InputInt("Lifetime ms##KoukuSceneProfileBox", &m_iSceneProfileBoxDurationMs, 10, 100);
	ImGui::InputInt("Blend ms##KoukuSceneProfileBox", &m_iSceneProfileBoxBlendMs, 10, 100);
	m_iSceneProfileBoxStartMs = std::clamp(m_iSceneProfileBoxStartMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_iSceneProfileBoxDurationMs = std::clamp(m_iSceneProfileBoxDurationMs, 1, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_iSceneProfileBoxBlendMs = std::clamp(m_iSceneProfileBoxBlendMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	if (ImGui::Button("Apply##KoukuSceneProfileBox"))
	{
		std::string status;
		(void)Set_SceneProfileBoxWindow(patternId, occurrenceId,
			static_cast<std::uint32_t>(m_iSceneProfileBoxStartMs),
			static_cast<std::uint32_t>(m_iSceneProfileBoxDurationMs),
			static_cast<std::uint32_t>(m_iSceneProfileBoxBlendMs), status);
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("To Pattern End##KoukuSceneProfileBox"))
	{
		std::string status;
		(void)Set_SceneProfileBoxWindow(patternId, occurrenceId, box->iStartMs,
			lifetimeMs > box->iStartMs ? lifetimeMs - box->iStartMs : 1u, box->iBlendMs, status);
		return;
	}
	ImGui::TextWrapped("The profile blends in over Blend ms from Start ms and the previous profile returns when Lifetime ends. Pattern lifetime %u ms.",
		lifetimeMs);
	if (ImGui::Button("Delete Scene Profile Box"))
	{
		std::string status;
		(void)Delete_SceneProfileBox(patternId, occurrenceId, status);
	}
}

void Client::CKoukuSaydonActionWorkbench::Render_LogicDefinitionValues(
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic)
{
	/* The draft copies the definition once per selection so typing never
	   commits half a value; Apply Values commits the whole set. */
	if (m_strLogicValueDraftId != logic.strLogicId)
	{
		m_LogicValueDraft = logic;
		m_strLogicValueDraftId = logic.strLogicId;
	}
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft = m_LogicValueDraft;
	ImGui::SeparatorText("DURATION" == logic.strLogicType ? "Judgement" : "TRIGGER" == logic.strLogicType ? "Trigger" : "Outcome");
	if ("DURATION" == logic.strLogicType)
	{
		if (ImGui::BeginCombo("Judgement kind##KoukuLogicValue",
				draft.strJudgementKind.empty() ? "(name only)" : draft.strJudgementKind.c_str()))
		{
			if (ImGui::Selectable("(name only)", draft.strJudgementKind.empty()))
				draft = KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION{ logic.strLogicId, logic.strDisplayName, logic.strLogicType };
			for (const char_t* const kind : KOUKU_SAYDON_JUDGEMENT_KINDS)
			{
				if (ImGui::Selectable(kind, draft.strJudgementKind == kind) && draft.strJudgementKind != kind)
				{
					draft = KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION{ logic.strLogicId, logic.strDisplayName, logic.strLogicType };
					draft.strJudgementKind = kind;

					if ("GAZE_REAL_BOSS" == draft.strJudgementKind)
					{
						draft.fHalfAngleDegrees = 45.0;
						draft.fMaxDistanceM = 30.0;
					}
					else if ("STAGGER_WINDOW" == draft.strJudgementKind)
					{
						draft.iThreshold = 1000u;
						draft.fShieldArcDegrees = 90.0;
						draft.bEndsPatternOnSuccess = true;
					}
				}
			}
			ImGui::EndCombo();
		}
		if ("ROULETTE_CARD_MATCH" == draft.strJudgementKind)
		{
			ImGui::TextWrapped("Select eight authored Roulette Card Regions. Each region has a symbol and a RED/BLACK color in its Collider Box Detail.");
			const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
			std::unordered_set<std::string> shown;
			if (nullptr != pattern)
				for (const auto& region : pattern->PresentationOccurrences)
				{
					const auto* resource = Find_PresentationResource(m_Draft, region.strResourceId);
					if (nullptr == resource || resource->strColliderKind != "ROULETTE_CARD_REGION" ||
						region.strRegionId.empty() || !shown.insert(region.strRegionId).second) continue;
					bool selected = std::find(draft.RegionIds.begin(), draft.RegionIds.end(), region.strRegionId) != draft.RegionIds.end();
					const std::string label = region.strRegionId + " | " + region.strCardSymbol + " / " + region.strCardColor;
					if (ImGui::Checkbox(label.c_str(), &selected))
					{
						if (selected && draft.RegionIds.size() < 8u) draft.RegionIds.push_back(region.strRegionId);
						else if (!selected) std::erase(draft.RegionIds, region.strRegionId);
					}
				}
			for (const auto& id : draft.RegionIds)
				if (!shown.contains(id)) ImGui::TextColored(ImVec4(1.f, 0.5f, 0.3f, 1.f), "Unavailable region: %s", id.c_str());
			ImGui::Text("Mapped regions: %zu / 8", draft.RegionIds.size());
			if (ImGui::Button("Clear region references")) draft.RegionIds.clear();
			if (draft.iSectorCount != 0u || !draft.SectorSymbols.empty())
				ImGui::TextDisabled("Legacy sector data is retained for disabled/DRAFT editing only; it does not define the new eight regions.");
			ImGui::TextWrapped("Only the position at the window end is judged: matching card = Success, another card = Fail, outside all regions = Timeout.");
		}
		else if ("AREA_OVERLAP" == draft.strJudgementKind)
		{
			if (ImGui::BeginCombo("Inside outcome", draft.strInsideOutcome.c_str()))
			{
				for (const char* outcome : { "SUCCESS", "FAIL" })
					if (ImGui::Selectable(outcome, draft.strInsideOutcome == outcome)) draft.strInsideOutcome = outcome;
				ImGui::EndCombo();
			}
			ImGui::TextDisabled("At the window end, inside runs this Result slot; outside runs Timeout.");
		}
		else if ("GAZE_REAL_BOSS" == draft.strJudgementKind)
		{
			float halfAngle = static_cast<float>(draft.fHalfAngleDegrees);
			if (ImGui::InputFloat("Half angle deg##KoukuLogicValue", &halfAngle, 1.f, 5.f, "%.1f"))
				draft.fHalfAngleDegrees = std::clamp(halfAngle, 1.f, 180.f);
			float distance = static_cast<float>(draft.fMaxDistanceM);
			if (ImGui::InputFloat("Max distance m##KoukuLogicValue", &distance, 1.f, 5.f, "%.1f"))
				draft.fMaxDistanceM = std::clamp(distance, 0.f, 1000.f);
			ImGui::TextDisabled("Judged once when the window ends: the player's facing must hold the real boss inside this cone.");
		}
		else if ("POSE_INPUT" == draft.strJudgementKind)
		{
			const std::size_t poseIndex = (std::min<std::size_t>)(draft.iPoseIndex, DANCE_POSE_LABELS.size() - 1u);
			if (ImGui::BeginCombo("Pose##KoukuLogicValue", DANCE_POSE_LABELS[poseIndex]))
			{
				for (std::size_t index = 0u; index < DANCE_POSE_LABELS.size(); ++index)
					if (ImGui::Selectable(DANCE_POSE_LABELS[index], poseIndex == index))
						draft.iPoseIndex = static_cast<std::uint32_t>(index);
				ImGui::EndCombo();
			}
			ImGui::TextDisabled("The boss pose the window shows; players answer through their shuffled Q/W/E/R slots.");
		}
		else if ("STAGGER_WINDOW" == draft.strJudgementKind)
		{
			int32_t threshold = static_cast<int32_t>((std::min<std::uint32_t>)(draft.iThreshold, 2000000000u));
			if (ImGui::InputInt("Damage threshold##KoukuLogicValue", &threshold, 100, 1000))
				draft.iThreshold = static_cast<std::uint32_t>((std::max)(1, threshold));
			float arc = static_cast<float>(draft.fShieldArcDegrees);
			if (ImGui::InputFloat("Shield arc deg##KoukuLogicValue", &arc, 5.f, 15.f, "%.1f"))
				draft.fShieldArcDegrees = std::clamp(arc, 0.f, 360.f);
			float normalYaw = static_cast<float>(draft.fNormalYawOffsetDegrees);
			if (ImGui::InputFloat("Shield normal yaw offset (degrees)", &normalYaw, 1.f, 15.f))
				draft.fNormalYawOffsetDegrees = std::clamp(normalYaw, -360.f, 360.f);
			ImGui::Checkbox("Ends the Pattern on success##KoukuLogicValue", &draft.bEndsPatternOnSuccess);
			ImGui::TextDisabled("Hits inside the frontal shield arc reflect to their caster; damage taken elsewhere accumulates toward the threshold.");
		}
	}
	else if ("RESULT" == logic.strLogicType)
	{
		if (ImGui::BeginCombo("Outcome kind##KoukuLogicValue",
				draft.strOutcomeKind.empty() ? "(name only)" : draft.strOutcomeKind.c_str()))
		{
			if (ImGui::Selectable("(name only)", draft.strOutcomeKind.empty()))
				draft = KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION{ logic.strLogicId, logic.strDisplayName, logic.strLogicType };
			for (const char_t* const kind : KOUKU_SAYDON_OUTCOME_KINDS)
			{
				if (ImGui::Selectable(kind, draft.strOutcomeKind == kind) && draft.strOutcomeKind != kind)
				{
					draft = KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION{ logic.strLogicId, logic.strDisplayName, logic.strLogicType };
					draft.strOutcomeKind = kind;
					if ("MAX_HP_PERCENT_DAMAGE" == draft.strOutcomeKind || "MADNESS_GAUGE_ADD_PERCENT" == draft.strOutcomeKind)
						draft.iPercent = 10u;
				}
			}
			ImGui::EndCombo();
		}
		const bool_t percentKind = "MAX_HP_PERCENT_DAMAGE" == draft.strOutcomeKind ||
			"MADNESS_GAUGE_ADD_PERCENT" == draft.strOutcomeKind;
		if (percentKind)
		{
			int32_t percent = static_cast<int32_t>(draft.iPercent);
			if (ImGui::InputInt("Percent##KoukuLogicValue", &percent, 1, 10))
				draft.iPercent = static_cast<std::uint32_t>(std::clamp(percent, 1, 100));
		}
		if ("CLOWN_TRANSFORM" == draft.strOutcomeKind)
		{
			int32_t durationMs = static_cast<int32_t>(draft.iDurationMs);
			if (ImGui::InputInt("Hold ms (0 = encounter policy)##KoukuLogicValue", &durationMs, 100, 1000))
				draft.iDurationMs = static_cast<std::uint32_t>(std::clamp(durationMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS)));
		}
		if ("FOLLOWUP_PATTERN" == draft.strOutcomeKind)
		{
			if (ImGui::BeginCombo("Follow-up Pattern##KoukuLogicValue",
					draft.strFollowupPatternId.empty() ? "(pick a Pattern)" : draft.strFollowupPatternId.c_str()))
			{
				for (const KOUKU_SAYDON_COMPOSITION_PATTERN& candidate : m_Draft.Patterns)
				{
					if (!candidate.strLoadError.empty())
						continue;
					const std::string item = candidate.strDisplayName + "##" + candidate.strPatternId;
					if (ImGui::Selectable(item.c_str(), draft.strFollowupPatternId == candidate.strPatternId))
						draft.strFollowupPatternId = candidate.strPatternId;
				}
				ImGui::EndCombo();
			}
			ImGui::TextDisabled("Only a stagger window may start a follow-up; the target must be PRODUCT to publish.");
		}
		if ("INSTANT_DEATH" == draft.strOutcomeKind)
			ImGui::TextDisabled("Applied to each judged player; on a stagger timeout to the whole raid.");
	}
	else if ("TRIGGER" == logic.strLogicType)
	{
		if (ImGui::BeginCombo("Trigger kind", draft.strTriggerKind.empty() ? "(name only)" : draft.strTriggerKind.c_str()))
		{
			for (const char* kind : { "", "ENTER_AREA", "HUD_ENTER", "REAL_GAZE_TELEPORT" })
				if (ImGui::Selectable(*kind == '\0' ? "(name only)" : kind, draft.strTriggerKind == kind) && draft.strTriggerKind != kind)
				{
					draft = KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION{ logic.strLogicId, logic.strDisplayName, logic.strLogicType };
					draft.strTriggerKind = kind;
					if (draft.strTriggerKind == "HUD_ENTER") draft.strHudMode = "NONE";
					if (draft.strTriggerKind == "REAL_GAZE_TELEPORT") draft.ClockHours = { 4u, 7u, 10u };
				}
			ImGui::EndCombo();
		}
		if (draft.strTriggerKind == "ENTER_AREA")
			ImGui::TextWrapped("A linked Collider supplies the region. First entry is Success once; no entry before the window ends is Timeout.");
		else if (draft.strTriggerKind == "HUD_ENTER")
		{
			if (ImGui::BeginCombo("HUD mode", draft.strHudMode.c_str()))
			{
				for (const char* mode : { "NONE", "POLYMORPH", "MARIO", "DANCE", "MAZE" })
					if (ImGui::Selectable(mode, draft.strHudMode == mode)) draft.strHudMode = mode;
				ImGui::EndCombo();
			}
		}
		else if (draft.strTriggerKind == "REAL_GAZE_TELEPORT")
		{
			float position[3] = { static_cast<float>(draft.TeleportPosition[0]), static_cast<float>(draft.TeleportPosition[1]), static_cast<float>(draft.TeleportPosition[2]) };
			if (ImGui::InputFloat3("Real boss teleport position (m)", position))
				draft.TeleportPosition = { position[0], position[1], position[2] };
			float yaw = static_cast<float>(draft.fFaceCenterYawOffsetDegrees);
			if (ImGui::InputFloat("Face center yaw offset (degrees)", &yaw, 1.f, 15.f))
				draft.fFaceCenterYawOffsetDegrees = std::clamp(yaw, -360.f, 360.f);
			if (ImGui::BeginCombo("Clone Pattern", draft.strClonePatternId.empty() ? "(select Pattern)" : draft.strClonePatternId.c_str()))
			{
				for (const auto& pattern : m_Draft.Patterns)
					if (pattern.strLoadError.empty() && ImGui::Selectable((pattern.strDisplayName + "##" + pattern.strPatternId).c_str(), draft.strClonePatternId == pattern.strPatternId))
						draft.strClonePatternId = pattern.strPatternId;
				ImGui::EndCombo();
			}
			if (draft.ClockHours.size() == 3u)
			{
				int hours[3] = { static_cast<int>(draft.ClockHours[0]), static_cast<int>(draft.ClockHours[1]), static_cast<int>(draft.ClockHours[2]) };
				if (ImGui::InputInt3("Clone clock hours", hours))
					for (std::size_t i = 0u; i < 3u; ++i) draft.ClockHours[i] = static_cast<std::uint32_t>(std::clamp(hours[i], 1, 12));
			}
		}
	}
	if (ImGui::Button("Apply Values##KoukuLogicValue"))
	{
		std::string status;
		(void)Set_LogicDefinitionValues(logic.strLogicId, draft, status);
	}
	ImGui::SameLine();
	if (ImGui::Button("Revert##KoukuLogicValue"))
		m_strLogicValueDraftId.clear();
}

void Client::CKoukuSaydonActionWorkbench::Render_LogicResources()
{
	ImGui::SeparatorText("Logic Catalog");
	ImGui::TextDisabled("%zu logics | Create names a reusable Logic; Append places its box on the selected Pattern.",
		m_Draft.Logics.size());
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const selectedPattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool_t patternEditable = nullptr != selectedPattern &&
		selectedPattern->strLoadError.empty();
	const std::string targetPatternName = nullptr == selectedPattern ?
		std::string("none") : selectedPattern->strDisplayName;

	if (ImGui::BeginChild("##KoukuLogicList", ImVec2(0.f, 180.f), ImGuiChildFlags_Borders))
	{
		if (m_Draft.Logics.empty())
			ImGui::TextDisabled("No Logic yet. Choose a type, name it, and press Create Logic.");
		for (const char_t* const type : KOUKU_SAYDON_LOGIC_TYPES)
		{
			if (!ImGui::TreeNodeEx(type, ImGuiTreeNodeFlags_DefaultOpen))
				continue;
			for (const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic : m_Draft.Logics)
			{
				if (logic.strLogicType != type)
					continue;
				const std::string label = logic.strDisplayName + "##" + logic.strLogicId;
				if (ImGui::Selectable(label.c_str(), m_strSelectedLogicId == logic.strLogicId))
					m_strSelectedLogicId = logic.strLogicId;
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s | %s | %zu box(es)", logic.strLogicId.c_str(),
						logic.strLogicType.c_str(), Count_LogicReferences(m_Draft, logic.strLogicId));
			}
			ImGui::TreePop();
		}
	}
	ImGui::EndChild();

	ImGui::SeparatorText("Create Logic");
	ImGui::TextUnformatted("Type");
	for (int32_t index = 0; index < static_cast<int32_t>(KOUKU_SAYDON_LOGIC_TYPES.size()); ++index)
	{
		if (0 != index)
			ImGui::SameLine();
		if (ImGui::RadioButton(KOUKU_SAYDON_LOGIC_TYPES[index], m_iNewLogicType == index))
			m_iNewLogicType = index;
	}
	ImGui::InputTextWithHint("##NewKoukuLogicName", "Logic display name",
		m_NewLogicName, std::size(m_NewLogicName));
	ImGui::SameLine();
	ImGui::BeginDisabled('\0' == m_NewLogicName[0] || !m_bHasDraft);
	if (ImGui::Button("Create Logic"))
	{
		std::string logicId;
		std::string status;
		if (Create_Logic(m_NewLogicName, KOUKU_SAYDON_LOGIC_TYPES[m_iNewLogicType], logicId, status))
			m_NewLogicName[0] = '\0';
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();

	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION* const selectedLogic =
		Find_Logic(m_Draft, m_strSelectedLogicId);
	if (nullptr == selectedLogic)
		return;
	/* Copies outlive the draft replacement a mutation below performs. */
	const std::string logicId = selectedLogic->strLogicId;
	const std::string logicName = selectedLogic->strDisplayName;
	const std::string logicType = selectedLogic->strLogicType;
	bool_t unresolvedReferences = false;
	const std::size_t references = Count_LogicReferences(m_Draft, logicId, &unresolvedReferences);
	ImGui::SeparatorText("Selected Logic");
	ImGui::TextWrapped("%s", logicName.c_str());
	ImGui::TextDisabled("%s | %s | %zu box(es)", logicId.c_str(), logicType.c_str(), references);
	Render_LogicDefinitionValues(*selectedLogic);
	if (nullptr == Find_Logic(m_Draft, logicId))
		return;
	ImGui::SeparatorText("Place");
	ImGui::SetNextItemWidth(120.f);
	ImGui::InputInt("Box ms##NewKoukuLogicBox", &m_iNewLogicBoxDurationMs, 100, 1000);
	m_iNewLogicBoxDurationMs = std::clamp(m_iNewLogicBoxDurationMs, 1,
		static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	ImGui::SameLine();
	ImGui::BeginDisabled(!patternEditable);
	if (ImGui::Button("Append Logic at Cursor"))
	{
		std::string occurrenceId;
		std::string status;
		(void)Append_LogicBox(m_strSelectedPatternId, logicId, m_iCursorMs,
			static_cast<std::uint32_t>(m_iNewLogicBoxDurationMs), occurrenceId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(0u != references || unresolvedReferences);
	if (ImGui::Button("Delete Logic"))
	{
		std::string status;
		(void)Delete_Logic(logicId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	if (unresolvedReferences)
		ImGui::TextDisabled("Repair invalid Pattern Logic references before deleting a Logic.");
	if (!patternEditable)
		ImGui::TextDisabled("Select an editable Pattern in Patterns to append this Logic.");
	ImGui::TextDisabled("Append target: %s | cursor %u ms", targetPatternName.c_str(), m_iCursorMs);
}

bool_t Client::CKoukuSaydonActionWorkbench::Render_LogicOutcomeSlots(
	const std::string& patternId, const std::string& occurrenceId)
{
	const auto* pattern = Find_Pattern(m_Draft, patternId);
	const auto* box = nullptr == pattern ? nullptr : Find_LogicBox(*pattern, occurrenceId);
	if (nullptr == box) return false;
	const auto* logic = Find_Logic(m_Draft, box->strLogicId);
	if (nullptr != logic && Kouku_LogicOwnsOutcomes(*logic))
	{
		/* Each outcome slot lists up to four RESULT Logics in the order the
		   Server applies them. A slot the judgement kind never ends in stays
		   disabled: gaze has no Timeout and the stagger window has no Fail. */
		ImGui::SeparatorText("Outcomes");
		if (Kouku_LogicOutcomeKind(*logic).empty())
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.24f, 1.f),
				"Give this Logic a judgement kind in Resources > Logic before the Pattern can be PRODUCT.");
		else
			ImGui::TextDisabled("%s", Kouku_LogicOutcomeKind(*logic).c_str());
		const bool_t hasResult = m_Draft.Logics.end() != std::find_if(m_Draft.Logics.begin(), m_Draft.Logics.end(),
			[](const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& candidate)
			{ return "RESULT" == candidate.strLogicType; });
		for (const KOUKU_SAYDON_OUTCOME_SLOT slot : {
			KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS, KOUKU_SAYDON_OUTCOME_SLOT::FAIL,
			KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT })
		{
			const bool_t allowed = Kouku_LogicOutcomeKind(*logic).empty() ||
				Kouku_IsOutcomeSlotAllowed(Kouku_LogicOutcomeKind(*logic), slot);
			std::vector<std::string> targets = box->Outcomes(slot);
			ImGui::PushID(static_cast<int32_t>(slot));
			ImGui::BeginDisabled(!allowed);
			ImGui::Text("%s%s", Outcome_SlotLabel(slot), allowed ? "" : " (not an outcome of this kind)");
			bool_t changed = false;
			for (std::size_t index = 0u; index < targets.size(); ++index)
			{
				ImGui::PushID(static_cast<int32_t>(index));
				const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION* const current = Find_Logic(m_Draft, targets[index]);
				ImGui::SetNextItemWidth(220.f);
				if (ImGui::BeginCombo("##KoukuOutcome",
						nullptr == current ? "(missing RESULT)" : current->strDisplayName.c_str()))
				{
					for (const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& candidate : m_Draft.Logics)
					{
						if ("RESULT" != candidate.strLogicType)
							continue;
						const std::string item = candidate.strDisplayName + "##" + candidate.strLogicId;
						if (ImGui::Selectable(item.c_str(), targets[index] == candidate.strLogicId) &&
							targets[index] != candidate.strLogicId)
						{
							targets[index] = candidate.strLogicId;
							changed = true;
						}
					}
					ImGui::EndCombo();
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("x##KoukuOutcomeRemove"))
				{
					targets.erase(targets.begin() + static_cast<std::ptrdiff_t>(index));
					changed = true;
				}
				ImGui::PopID();
				if (changed)
					break;
			}
			if (!changed && targets.size() < KOUKU_SAYDON_MAX_OUTCOMES_PER_SLOT)
			{
				ImGui::SetNextItemWidth(220.f);
				ImGui::BeginDisabled(!hasResult);
				if (ImGui::BeginCombo("##KoukuOutcomeAdd", "+ add RESULT"))
				{
					for (const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& candidate : m_Draft.Logics)
					{
						if ("RESULT" != candidate.strLogicType)
							continue;
						const std::string item = candidate.strDisplayName + "##" + candidate.strLogicId;
						if (ImGui::Selectable(item.c_str(), false))
						{
							targets.push_back(candidate.strLogicId);
							changed = true;
						}
					}
					ImGui::EndCombo();
				}
				ImGui::EndDisabled();
			}
			ImGui::EndDisabled();
			ImGui::PopID();
			if (changed)
			{
				std::string status;
				(void)Set_LogicBoxOutcomes(patternId, occurrenceId, slot, targets, status);
				return true;
			}
		}
		if (!hasResult)
			ImGui::TextDisabled("Create a RESULT Logic in Resources > Logic to wire an outcome.");
	}
	else if (nullptr != logic)
	{
		ImGui::TextDisabled("%s boxes carry no outcomes; only DURATION or ENTER_AREA owns result slots.",
			logic->strLogicType.c_str());
	}
	return false;
}

void Client::CKoukuSaydonActionWorkbench::Render_LogicBoxDetails(
	const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
{
	const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE* const box =
		Find_LogicBox(pattern, m_strSelectedLogicOccurrenceId);
	if (nullptr == box)
		return;
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION* const logic =
		Find_Logic(m_Draft, box->strLogicId);
	const std::string patternId = pattern.strPatternId;
	const std::string occurrenceId = box->strOccurrenceId;
	const std::uint32_t lifetimeMs = Pattern_DurationMs(pattern);
	ImGui::SeparatorText("Logic Box");
	ImGui::Text("%s", occurrenceId.c_str());
	ImGui::Text("%s | %s",
		nullptr == logic ? "(missing Logic)" : logic->strDisplayName.c_str(),
		nullptr == logic ? "?" : logic->strLogicType.c_str());
	ImGui::TextDisabled("%s", box->strLogicId.c_str());
	bool_t enabled = box->bEnabled;
	if (ImGui::Checkbox("Enabled in gameplay##KoukuLogicBox", &enabled))
	{
		auto candidate = m_Draft;
		auto* editablePattern = Find_Pattern(candidate, patternId);
		auto* editableBox = editablePattern ? Find_LogicBox(*editablePattern, occurrenceId) : nullptr;
		if (editableBox)
		{
			editableBox->bEnabled = enabled;
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated Logic gameplay enable state.", status);
		}
		return;
	}
	ImGui::InputInt("Start ms##KoukuLogicBox", &m_iLogicBoxStartMs, 10, 100);
	ImGui::InputInt("Lifetime ms##KoukuLogicBox", &m_iLogicBoxDurationMs, 10, 100);
	m_iLogicBoxStartMs = std::clamp(m_iLogicBoxStartMs, 0,
		static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_iLogicBoxDurationMs = std::clamp(m_iLogicBoxDurationMs, 1,
		static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	if (ImGui::Button("Apply Window##KoukuLogicBox"))
	{
		std::string status;
		(void)Set_LogicBoxWindow(patternId, occurrenceId,
			static_cast<std::uint32_t>(m_iLogicBoxStartMs),
			static_cast<std::uint32_t>(m_iLogicBoxDurationMs), status);
		return;
	}
	ImGui::TextDisabled("Pattern lifetime %u ms. Drag the box edges on the Logic lane to resize it.",
		lifetimeMs);
	if (Render_LogicOutcomeSlots(patternId, occurrenceId)) return;
	if (ImGui::Button("Delete Logic Box"))
	{
		std::string status;
		(void)Delete_LogicBox(patternId, occurrenceId, status);
	}
}

void Client::CKoukuSaydonActionWorkbench::Render_Details()
{
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (nullptr == pattern)
	{
		ImGui::TextDisabled("No Pattern selected.");
		return;
	}
	const std::string patternId = pattern->strPatternId;
	if (!pattern->strLoadError.empty())
	{
		ImGui::TextWrapped("%s", pattern->strLoadError.c_str());
		ImGui::TextWrapped("Original pattern JSON is preserved. Repair the source and Reload, or delete this pattern.");
		if (ImGui::Button("Delete Invalid Pattern"))
		{
			std::string status;
			(void)Delete_Pattern(patternId, status);
		}
		return;
	}
	ImGui::SeparatorText("Pattern");
	if (ImGui::InputText("Display Name", m_PatternName, std::size(m_PatternName),
			ImGuiInputTextFlags_EnterReturnsTrue))
	{
		std::string status;
		(void)Rename_Pattern(patternId, m_PatternName, status);
		return;
	}
	std::string requestedCategory;
	if (ImGui::BeginCombo("Category", pattern->strCategory.c_str()))
	{
		for (const char_t* const value : PATTERN_CATEGORIES)
		{
			if (ImGui::Selectable(value, pattern->strCategory == value))
			{
				requestedCategory = value;
				break;
			}
		}
		ImGui::EndCombo();
	}
	if (!requestedCategory.empty())
	{
		std::string status;
		(void)Set_PatternCategory(patternId, requestedCategory, status);
		return;
	}
	std::string requestedAuthoringStatus;
	if (ImGui::BeginCombo("Authoring", pattern->strAuthoringStatus.c_str()))
	{
		for (const char_t* const value : { "DRAFT", "PRODUCT" })
		{
			if (ImGui::Selectable(value, pattern->strAuthoringStatus == value))
			{
				requestedAuthoringStatus = value;
				break;
			}
		}
		ImGui::EndCombo();
	}
	if (!requestedAuthoringStatus.empty())
	{
		std::string status;
		(void)Set_PatternAuthoringStatus(
			patternId, requestedAuthoringStatus, status);
		return;
	}
	bool resetToSpawn = pattern->bResetBossToSpawn;
	if (ImGui::Checkbox("Reset boss to spawn at Pattern start", &resetToSpawn))
	{
		auto candidate = m_Draft;
		auto* edited = Find_Pattern(candidate, patternId);
		if (nullptr != edited)
		{
			edited->bResetBossToSpawn = resetToSpawn;
			Mark_Draft(candidate, *edited);
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated boss spawn reset for this Pattern.", status);
		}
		return;
	}
	if (ImGui::Button("Delete Pattern"))
	{
		std::string status;
		(void)Delete_Pattern(patternId, status);
		return;
	}

	if (!m_strSelectedPresentationOccurrenceId.empty())
	{
		Render_PresentationBoxDetails(*pattern);
		return;
	}
	if (!m_strSelectedWorldOccurrenceId.empty())
	{
		Render_WorldBoxDetails(*pattern);
		return;
	}
	if (!m_strSelectedSceneProfileOccurrenceId.empty())
	{
		Render_SceneProfileBoxDetails(*pattern);
		return;
	}
	if (!m_strSelectedSummonOccurrenceId.empty())
	{
		Render_SummonBoxDetails(*pattern);
		return;
	}
	if (!m_strSelectedLogicOccurrenceId.empty())
	{
		/* A Logic box owns its own Details; Stage and Animation sections stay
		   untouched until an animation box or Stage is selected again. */
		Render_LogicBoxDetails(*pattern);
		return;
	}

	ImGui::SeparatorText("Stage");
	ImGui::SetNextItemWidth(110.f);
	if (ImGui::BeginCombo("##NewStageKind", STAGE_KINDS[m_iSelectedNewStageKind]))
	{
		for (int32_t index = 0;
			index < static_cast<int32_t>(STAGE_KINDS.size()); ++index)
		{
			if (ImGui::Selectable(STAGE_KINDS[index],
					m_iSelectedNewStageKind == index))
			{
				m_iSelectedNewStageKind = index;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.f);
	ImGui::InputInt("ms##NewStageDuration", &m_iNewStageDurationMs, 100, 1000);
	m_iNewStageDurationMs = std::clamp(m_iNewStageDurationMs, 1,
		static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	ImGui::SameLine();
	if (ImGui::Button("Add Stage"))
	{
		std::string stageId;
		std::string status;
		(void)Add_Stage(patternId,
			STAGE_KINDS[m_iSelectedNewStageKind],
			static_cast<std::uint32_t>(m_iNewStageDurationMs), stageId, status);
		return;
	}

	const KOUKU_SAYDON_COMPOSITION_STAGE* stage =
		Find_Stage(*pattern, m_strSelectedStageId);
	if (nullptr == stage)
		return;
	const std::string stageId = stage->strStageId;

	ImGui::Text("%s | %s", stage->strStageId.c_str(), stage->strActionId.c_str());
	std::string requestedStageKind;
	if (ImGui::BeginCombo("Stage Kind", stage->strStageKind.c_str()))
	{
		for (const char_t* const value : STAGE_KINDS)
		{
			if (ImGui::Selectable(value, stage->strStageKind == value))
			{
				requestedStageKind = value;
				break;
			}
		}
		ImGui::EndCombo();
	}
	if (!requestedStageKind.empty())
	{
		std::string status;
		(void)Set_StageKind(patternId, stageId, requestedStageKind, status);
		return;
	}
	int32_t durationMs = static_cast<int32_t>(stage->iDurationMs);
	if (ImGui::InputInt("Duration ms", &durationMs, 10, 100))
	{
		durationMs = std::clamp(durationMs, 1, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
		std::string status;
		(void)Set_StageDuration(patternId, stageId,
			static_cast<std::uint32_t>(durationMs), status);
		return;
	}
	if (ImGui::Button("Stage Up"))
	{
		std::string status;
		(void)Move_Stage(patternId, stageId, -1, status);
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("Stage Down"))
	{
		std::string status;
		(void)Move_Stage(patternId, stageId, 1, status);
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete Stage"))
	{
		std::string status;
		(void)Delete_Stage(patternId, stageId, status);
		return;
	}
	ImGui::BeginDisabled(!m_bHasSelectedResource);
	bool_t animationBound = false;
	if (ImGui::Button("Bind Selected Animation"))
	{
		std::string occurrenceId;
		std::string status;
		(void)Bind_Animation(patternId, stageId,
			m_SelectedResource, 0u, occurrenceId, status);
		animationBound = true;
	}
	ImGui::EndDisabled();
	if (animationBound)
		return;

	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* const occurrence =
		Find_Occurrence(*pattern, m_strSelectedOccurrenceId);
	if (nullptr == occurrence)
		return;
	const std::string occurrenceId = occurrence->strOccurrenceId;
	ImGui::SeparatorText("Animation");
	ImGui::Text("%s", occurrence->strOccurrenceId.c_str());
	ImGui::Text("%s | %s", occurrence->strProfileId.c_str(),
		occurrence->strRuntimeClip.c_str());
	ImGui::InputInt("Start Offset ms", &m_iOccurrenceStartOffsetMs, 10, 100);
	m_iOccurrenceStartOffsetMs = std::clamp(m_iOccurrenceStartOffsetMs, 0,
		static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	if (ImGui::Button("Apply Move"))
	{
		std::string status;
		(void)Move_Animation(patternId, occurrenceId,
			static_cast<std::uint32_t>(m_iOccurrenceStartOffsetMs), status);
		return;
	}
	ImGui::InputInt("Source Start ms", &m_iOccurrenceSourceStartMs, 10, 100);
	ImGui::InputInt("Play ms", &m_iOccurrencePlayMs, 10, 100);
	m_iOccurrenceSourceStartMs = std::clamp(m_iOccurrenceSourceStartMs, 0,
		static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_iOccurrencePlayMs = std::clamp(m_iOccurrencePlayMs, 1,
		static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	if (ImGui::Button("Apply Trim"))
	{
		std::string status;
		(void)Trim_Animation(patternId, occurrenceId,
			static_cast<std::uint32_t>(m_iOccurrenceSourceStartMs),
			static_cast<std::uint32_t>(m_iOccurrencePlayMs), status);
		return;
	}
	std::uint32_t nativeMs = 0u;
	const bool_t nativeKnown = Resolve_NativeClipMs(*occurrence, nativeMs);
	const double windowEndMs = occurrence->iSourceStartMs +
		static_cast<double>(occurrence->iPlayMs) * occurrence->fPlayRate;
	if (nativeKnown)
	{
		ImGui::TextDisabled("Native clip %u ms | source window %u..%.0f ms",
			nativeMs, occurrence->iSourceStartMs, windowEndMs);
	}
	else
	{
		ImGui::TextDisabled("Native clip length appears after Refresh Model Clips.");
	}
	if (nativeKnown && "EXACT" == occurrence->strEndPolicy && windowEndMs > nativeMs + 1.0)
	{
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.24f, 1.f),
			"The window outruns the native clip; preview holds the last pose. Choose HOLD_LAST_POSE or LOOP_TO_WINDOW to make that explicit.");
	}
	ImGui::InputFloat("Play Rate", &m_fOccurrencePlayRate, 0.1f, 0.5f, "%.2f");
	m_fOccurrencePlayRate = std::clamp(m_fOccurrencePlayRate, 0.01f, 16.f);
	m_iOccurrenceEndPolicy = std::clamp(m_iOccurrenceEndPolicy, 0,
		static_cast<int32_t>(END_POLICIES.size()) - 1);
	if (ImGui::BeginCombo("End Policy", END_POLICIES[m_iOccurrenceEndPolicy]))
	{
		for (int32_t index = 0; index < static_cast<int32_t>(END_POLICIES.size()); ++index)
		{
			if (ImGui::Selectable(END_POLICIES[index], m_iOccurrenceEndPolicy == index))
				m_iOccurrenceEndPolicy = index;
		}
		ImGui::EndCombo();
	}
	if (ImGui::Button("Apply Playback"))
	{
		std::string status;
		(void)Set_AnimationPlayback(patternId, occurrenceId,
			m_fOccurrencePlayRate, END_POLICIES[m_iOccurrenceEndPolicy], status);
		return;
	}
	ImGui::TextDisabled("PRODUCT v1 plays whole-stage EXACT rows: source start 0, play rate 0.1..4.");
	std::string requestedTargetStageId;
	if (ImGui::BeginCombo("Move to Stage", stage->strStageId.c_str()))
	{
		for (const KOUKU_SAYDON_COMPOSITION_STAGE& candidateStage : pattern->Stages)
		{
			if (ImGui::Selectable(candidateStage.strStageId.c_str(),
					candidateStage.strStageId == stage->strStageId) &&
				candidateStage.strStageId != stage->strStageId)
			{
				requestedTargetStageId = candidateStage.strStageId;
				break;
			}
		}
		ImGui::EndCombo();
	}
	if (!requestedTargetStageId.empty())
	{
		std::string status;
		(void)Move_AnimationToStage(patternId, occurrenceId,
			requestedTargetStageId,
			static_cast<std::uint32_t>(m_iOccurrenceStartOffsetMs), status);
		return;
	}
	if (ImGui::Button("Duplicate Animation"))
	{
		std::string duplicateId;
		std::string status;
		(void)Duplicate_Animation(patternId, occurrenceId, duplicateId, status);
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete Animation"))
	{
		std::string status;
		(void)Delete_Animation(patternId, occurrenceId, status);
	}
}

void Client::CKoukuSaydonActionWorkbench::Render_ReloadConfirmation()
{
	if (m_bReloadConfirmationRequested)
	{
		ImGui::OpenPopup("Discard KoukuSaydon composition draft?");
		m_bReloadConfirmationRequested = false;
	}
	if (!ImGui::BeginPopupModal("Discard KoukuSaydon composition draft?", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}
	ImGui::TextUnformatted("Reload discards the unsaved composition candidate.");
	if (ImGui::Button("Discard and Reload"))
	{
		m_bDirty = false;
		std::string status;
		if (Reload(status))
			ImGui::CloseCurrentPopup();
		else
			m_bDirty = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		ImGui::CloseCurrentPopup();
	ImGui::EndPopup();
}

void Client::CKoukuSaydonActionWorkbench::Begin_WorkbenchFrame()
{
	m_bSharedWorkspaceActive = true;
	if (!m_bLoadAttempted)
	{
		m_bLoadAttempted = true;
		std::string status;
		(void)Reload(status);
	}
}

void Client::CKoukuSaydonActionWorkbench::Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE pane)
{
	switch (pane)
	{
	case COMPOSITION_WORKBENCH_PANE::TOOLBAR: Render_Toolbar(); break;
	case COMPOSITION_WORKBENCH_PANE::PATTERNS:
	case COMPOSITION_WORKBENCH_PANE::BOSS_PATTERN: Render_PatternsAndResources(); break;
	case COMPOSITION_WORKBENCH_PANE::RESOURCES: Render_ResourceTree(); break;
	case COMPOSITION_WORKBENCH_PANE::SEQUENCER: Render_Timeline(); break;
	case COMPOSITION_WORKBENCH_PANE::DETAILS: Render_Details(); break;
	case COMPOSITION_WORKBENCH_PANE::PREVIEW: Render_Transport(); break;
	default: break;
	}
}

void Client::CKoukuSaydonActionWorkbench::End_WorkbenchFrame()
{
	Render_ReloadConfirmation();
	m_bSharedWorkspaceActive = false;
}

void Client::CKoukuSaydonActionWorkbench::Render()
{
	Poll_PublishProcess();
	if (!m_bOpen)
		return;
	if (!m_bLoadAttempted)
	{
		m_bLoadAttempted = true;
		std::string status;
		(void)Reload(status);
	}
	ImGui::SetNextWindowSize(ImVec2(1180.f, 720.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("KoukuSaydon Composition###KoukuSaydonActionWorkbench", &m_bOpen))
	{
		ImGui::End();
		Render_ResourcesWindow();
		return;
	}
	Render_Toolbar();

	if (m_bTimelineMaximized)
	{
		Render_Timeline();
	}
	else if (ImGui::BeginTable("##KoukuWorkbenchColumns", 3,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn("Patterns / Resources", ImGuiTableColumnFlags_WidthFixed, 300.f);
		ImGui::TableSetupColumn("Stage + Animation", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthFixed, 300.f);
		ImGui::TableNextColumn();
		Render_PatternsAndResources();
		ImGui::TableNextColumn();
		Render_Timeline();
		ImGui::TableNextColumn();
		Render_Details();
		ImGui::EndTable();
	}
	Render_ReloadConfirmation();
	ImGui::End();
	Render_ResourcesWindow();
}
