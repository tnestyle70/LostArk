#include "imgui.h"

#include "KoukuSaydonActionWorkbench.h"
#include "CompositionTimeline.h"
#include "Level_KakulSaydonArena.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "AnimationTargetService.h"
#include "Model.h"
#include "Effect_Catalog.h"
#include "Effect_AuthoringDocument.h"

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
#include <utility>

namespace
{
	using namespace Client;

	const CLevel_KakulSaydonArena::KAKUL_CAMERA_SHOT* Find_AuthoringCamera(const std::string_view shotId)
	{
		auto* level = CLevel_KakulSaydonArena::Get_Active();
		std::string status;
		if (!level || !level->Ensure_CameraShotAuthoring(status)) return nullptr;
		const auto& shots = level->Get_CameraShots();
		const auto found = std::find_if(shots.begin(), shots.end(), [&](const auto& shot) { return shot.strShotId == shotId; });
		return found != shots.end() ? &*found : nullptr;
	}
	std::uint32_t Camera_DefaultDuration(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource)
	{
		if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
			if (const auto* shot = Find_AuthoringCamera(resource.strAssetId)) return shot->iBlendInMs + shot->iDefaultHoldMs;
		return resource.iDurationMs;
	}


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

	template<class Rows>
	bool Has_TimelineBox(const Rows& rows, const std::string_view id)
	{
		return std::any_of(rows.begin(), rows.end(), [&](const auto& row) { return row.strOccurrenceId == id; });
	}

	bool Has_TimelineOccurrence(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const std::string_view id)
	{
		return Find_Occurrence(pattern, id) || Has_TimelineBox(pattern.LogicOccurrences, id) ||
			Has_TimelineBox(pattern.SummonOccurrences, id) || Has_TimelineBox(pattern.WorldOccurrences, id) ||
			Has_TimelineBox(pattern.SceneProfileOccurrences, id) || Has_TimelineBox(pattern.PresentationOccurrences, id);
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
			outStatus = "Create and select a child or independent Pattern before Append.";
			return nullptr;
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
		for (auto& bundle : document.Bundles)
			if (std::any_of(bundle.Members.begin(), bundle.Members.end(), [&](const auto& member) { return member.strPatternId == pattern.strPatternId; }))
				bundle.strAuthoringStatus = "DRAFT";
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
		bool_t unresolved = false;
		auto count = Count_LaneReferences(document, sceneProfileId, "sceneProfileOccurrences", "sceneProfileId", &unresolved);
		for (const auto& bundle : document.Bundles)
		{
			if (bundle.strLoadError.empty())
			{
				for (const auto& box : bundle.SceneProfileOccurrences) if (box.strSceneProfileId == sceneProfileId) ++count;
				continue;
			}
			DATA_JSON_VALUE preserved; std::string error;
			if (!CDataJson::Parse(bundle.strPreservedJson, preserved, error) || !preserved.Is_Object()) { unresolved = true; continue; }
			const auto* boxes = preserved.Find("sceneProfileOccurrences");
			if (!boxes) continue;
			if (!boxes->Is_Array()) { unresolved = true; continue; }
			for (const auto& box : boxes->Get_Array())
			{
				const auto* reference = box.Find("sceneProfileId");
				if (!reference || !reference->Is_String()) unresolved = true;
				else if (reference->Get_String() == sceneProfileId) ++count;
			}
		}
		if (outUnresolved) *outUnresolved = unresolved;
		return count;
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
	bool Valid_PresentationGeometry(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& value)
	{
		const auto finite = [](const auto& values, const double minimum, const double maximum) {
			return std::all_of(values.begin(), values.end(), [=](const double v) {
				return std::isfinite(v) && v >= minimum && v <= maximum; }); };
		return finite(value.PositionOffset, -100000.0, 100000.0) &&
			finite(value.RotationDegrees, -36000.0, 36000.0) && finite(value.Scale, 0.001, 10000.0);
	}

	bool Valid_GameplayColliderScale(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource,
		const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& value)
	{
		return resource.eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER ||
			std::all_of(value.Scale.begin(), value.Scale.end(), [](double scale) {
				return std::isfinite(scale) && scale >= .001 && scale <= 10000.0; });
	}

	void Copy_PresentationGeometry(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& source,
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& target)
	{
		target.PositionOffset = source.PositionOffset;
		target.RotationDegrees = source.RotationDegrees;
		target.Scale = source.Scale;
	}
}

Client::CKoukuSaydonActionWorkbench::CKoukuSaydonActionWorkbench(const bool sequenceWorkspace)
	: m_bSequenceWorkspace(sequenceWorkspace)
	, m_Document(sequenceWorkspace ? CKoukuSaydonCompositionDocument::Resolve_SequencePath() :
		CKoukuSaydonCompositionDocument::Resolve_Path())
{
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
	if (Is_Dirty())
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

	if (Is_CompleteSequencePlaying()) Stop_Preview();
	const std::string previousPatternId = m_strSelectedPatternId;
	m_Draft = m_Document.Get_LastGood();
	m_strLogicValueDraftId.clear();
	m_strColliderLogicValueDraftId.clear();
	m_ResourceReferences = m_Document.Get_References();
	m_bResourceTreeDirty = true;
	m_bHasDraft = true;
	m_bDirty = false;
	++m_iDraftGeneration;
	m_strSelectedPatternId = previousPatternId;
	if (nullptr == Find_Pattern(m_Draft, m_strSelectedPatternId))
	{
		m_strSelectedPatternId.clear();
		if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN) m_ePatternSelection = KOUKU_PATTERN_SELECTION::GATE;
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
	if (!m_bHasDraft || Is_PublishRunning())
	{
		outStatus = m_strStatus = !m_bHasDraft ? "No composition is loaded." : "Saved data is still being applied.";
		return false;
	}
	if (Is_Dirty())
	{
		auto candidate = m_Draft;
		for (const auto& edit : m_StagedPresentationGeometry)
		{
			auto* pattern = Find_Pattern(candidate, edit.strPatternId);
			const auto* source = pattern ? Find_PresentationBox(*pattern, edit.Occurrence.strOccurrenceId) : nullptr;
			const auto* resource = source ? Find_PresentationResource(candidate, source->strResourceId) : nullptr;
			if (!source || source->strResourceId != edit.Occurrence.strResourceId || !resource ||
				(resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT && resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER) ||
				!Valid_PresentationGeometry(edit.Occurrence))
			{
				outStatus = m_strStatus = "Save rejected: correct or revert the pending presentation geometry. Applied draft and source are unchanged.";
				return false;
			}
			auto geometry = *source;
			Copy_PresentationGeometry(edit.Occurrence, geometry);
			if (!Valid_GameplayColliderScale(*resource, geometry))
			{
				outStatus = m_strStatus = "Save rejected: Collider scale must be finite and positive. Adjust Scale / size for " +
					edit.Occurrence.strOccurrenceId + ". Pending edits and previous source are preserved.";
				return false;
			}
			for (auto& box : pattern->PresentationOccurrences)
				if (box.strOccurrenceId == edit.Occurrence.strOccurrenceId)
				{ Copy_PresentationGeometry(edit.Occurrence, box); break; }
		}
		if (!m_Document.Save_Atomic(candidate, outStatus))
		{ m_strStatus = outStatus; return false; }
		m_Draft = m_Document.Get_LastGood();
		m_ResourceReferences = m_Document.Get_References();
		m_bResourceTreeDirty = true;
		m_bDirty = false;
		m_StagedPresentationGeometry.clear();
		++m_iDraftGeneration;
		Normalize_Selection();
		Synchronize_EditorFields();
	}
	outStatus = m_strStatus = m_bSequenceWorkspace ?
		"Saved all Sequence changes to the independent Sequencer workspace." :
		"Saved all Composition changes. Use Publish All Patterns to synchronize the F1 tree.";
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Publish_AllPatterns(
	std::string& outStatus)
{
	if (m_bSequenceWorkspace)
	{
		outStatus = m_strStatus = "The Sequencer workspace supports local preview and Save only.";
		return false;
	}
	if (!m_bHasDraft || Is_Dirty() || !m_Document.Is_Fresh())
	{
		outStatus =
			"Save all Composition changes before Publish All Patterns; reload if the saved source changed.";
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
	std::array<wchar_t, MAX_PATH> systemDirectory{};
	const UINT systemLength = GetSystemDirectoryW(systemDirectory.data(), static_cast<UINT>(systemDirectory.size()));
	const std::filesystem::path powershell = systemLength && systemLength < systemDirectory.size() ?
		std::filesystem::path(systemDirectory.data()) / L"WindowsPowerShell" / L"v1.0" / L"powershell.exe" : std::filesystem::path{};
	if (powershell.empty() || !std::filesystem::is_regular_file(powershell, error) || error)
	{
		outStatus = m_strStatus = "The Windows PowerShell executable for Product publish could not be resolved.";
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
		L"\"" + powershell.wstring() + L"\" -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -File \"" +
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
		powershell.c_str(), mutableCommand.data(), nullptr, nullptr, TRUE,
		CREATE_NO_WINDOW, nullptr, projectRoot.c_str(), &startup, &process);
	const DWORD createError = created ? ERROR_SUCCESS : GetLastError();
	CloseHandle(input);
	CloseHandle(output);
	if (!created)
	{
		outStatus = "Could not start the KoukuSaydon Product publisher (Win32 " +
			std::to_string(createError) + "): " + powershell.string() + ". Preserved log: " + m_PublishDiagnosticPath.string();
		std::ofstream diagnostic(m_PublishDiagnosticPath, std::ios::binary | std::ios::app);
		if (diagnostic) diagnostic << outStatus << '\n';
		m_strStatus = outStatus;
		return false;
	}
	CloseHandle(process.hThread);
	m_hPublishProcess = process.hProcess;
	m_iPublishStartedAtMilliseconds = GetTickCount64();
	outStatus =
		"Publishing saved Patterns and Kouku world placements. Editing is locked until completion.";
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
		m_bProductInventoryRefreshRequested = true;
		m_strStatus =
			"Published saved Patterns. The next Complete Play admits this revision on the Server; a running replay keeps its original revision. F1 shows unavailable items with their reasons. World placement edits still require a Server restart.";
		return;
	}
	m_strStatus = "Publish All Patterns failed. Previous runtime data is retained unless the log reports a rollback failure. Use Publish All Patterns to retry";
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
	if (!actorProfileId.empty() && CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(m_strSelectedGateId, actorProfileId).empty())
	{ outStatus = "This model is not a target of the selected Gate."; return false; }
	const std::string model(actorProfileId);
	const auto matchesBundle = [&](const auto& bundle) {
		return model.empty() || std::any_of(bundle.Members.begin(), bundle.Members.end(), [&](const auto& member) {
			const auto* child = Find_Pattern(m_Draft, member.strPatternId);
			return child && child->strGateId == m_strSelectedGateId && child->strActorProfileId == model;
		});
	};
	bool visible = true;
	if (!model.empty() && m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN)
	{
		const auto* selected = Find_Pattern(m_Draft, m_strSelectedPatternId);
		visible = selected && (selected->strActorProfileId == model || std::any_of(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto& bundle) {
			return matchesBundle(bundle) && std::any_of(bundle.Members.begin(), bundle.Members.end(), [&](const auto& member) { return member.strPatternId == m_strSelectedPatternId; });
		}));
	}
	else if (!model.empty() && m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE)
		visible = std::any_of(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto& bundle) { return bundle.strBundleId == m_strSelectedBundleId && matchesBundle(bundle); });
	else if (!model.empty() && m_ePatternSelection == KOUKU_PATTERN_SELECTION::FOLDER)
		visible = std::any_of(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto& bundle) { return bundle.strFolderId == m_strSelectedFolderId && matchesBundle(bundle); }) ||
			std::any_of(m_Draft.Patterns.begin(), m_Draft.Patterns.end(), [&](const auto& pattern) {
				return pattern.strGateId == m_strSelectedGateId && pattern.strFolderId == m_strSelectedFolderId && pattern.strActorProfileId == model;
			});
	if (!visible) Select_Hierarchy(KOUKU_PATTERN_SELECTION::GATE, m_strSelectedGateId);
	m_strModelViewProfile = model;
	if (!model.empty()) m_strCreateActorProfileId = model;
	outStatus = "Model View changed; saved targets are unchanged.";
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Select_WorkbenchBoss(const COMPOSITION_WORKBENCH_BOSS boss)
{
	std::string gate = "GATE1";
	switch (boss)
	{
	case COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_GATE2: gate = "GATE2"; break;
	case COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_GATE3: gate = "GATE3"; break;
	case COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_ENCORE: gate = "BINGO"; break;
	default: break;
	}
	if (gate != m_strSelectedGateId) Select_Hierarchy(KOUKU_PATTERN_SELECTION::GATE, gate);
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
	if (m_strSelectedPatternId != patternId || m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN)
	{
		Stop_Preview();
		m_bPresentationPreviewRequestPending = false;
	}
	m_ePatternSelection = KOUKU_PATTERN_SELECTION::PATTERN;
	m_strSelectedGateId = pattern->strGateId;
	m_strCreateActorProfileId = pattern->strActorProfileId;
	const auto context = std::find_if(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto& bundle) {
		return bundle.strBundleId == m_strBundleReturnId && std::any_of(bundle.Members.begin(), bundle.Members.end(),
			[&](const auto& member) { return member.strPatternId == patternId; });
	});
	m_strSelectedFolderId = context == m_Draft.Bundles.end() ? pattern->strFolderId : context->strFolderId;
	m_strCreateFolderId = m_strSelectedFolderId;
	m_strSelectedBundleId = context == m_Draft.Bundles.end() ? std::string{} : context->strBundleId;
	m_strCreateBundleId = m_strSelectedBundleId;
	m_strBundleReturnId = m_strSelectedBundleId;
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
	Cancel_CompleteSequencePlay();
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
	m_strExpandedV1EffectId.clear();
	m_strV1ElementResourceStatus.clear();
	m_V1ElementResources.clear();
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
	Cancel_CompleteSequencePlay();
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
	if (Is_CompleteSequencePlaying() &&
		m_CompleteSequencePatternIds[m_iCompleteSequenceIndex] != m_PendingPatternPreview.strPatternId)
		Cancel_CompleteSequencePlay();
	outPattern = std::move(m_PendingPatternPreview);
	outStartClockMs = m_iPendingPreviewStartMs;
	outStartPaused = m_bPendingPreviewStartPaused;
	outTargetAssetName = std::move(m_strPendingPreviewTargetAsset);
	m_strPendingPreviewTargetAsset.clear();
	m_PendingPatternPreview = {};
	m_bPatternPreviewRequestPending = false;
	m_bPreviewResultStatusPending = true;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Consume_ServerPlayRequest(
	std::string& outPatternId,
	std::uint32_t& outSourceRevision)
{
	if (m_bSequenceWorkspace || !m_bServerPlayRequestPending)
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
	for (const auto& previous : m_Draft.Folders)
		if (!previous.strLoadError.empty())
			for (const auto& current : candidate.Folders) if (current.strFolderId == previous.strFolderId && current != previous)
			{ outStatus = m_strStatus = "Repair and Reload the invalid parent before editing."; return false; }
	for (const auto& previous : m_Draft.Bundles)
		if (!previous.strLoadError.empty())
			for (const auto& current : candidate.Bundles) if (current.strBundleId == previous.strBundleId && current != previous)
			{ outStatus = m_strStatus = "Repair and Reload the invalid bundle before editing."; return false; }
	for (auto& bundle : candidate.Bundles)
		if (bundle.strLoadError.empty()) for (const auto& member : bundle.Members)
			if (const auto* child = Find_Pattern(candidate, member.strPatternId); child && child->strAuthoringStatus != "PRODUCT")
				if (const auto* oldChild = Find_Pattern(m_Draft, member.strPatternId); oldChild && *child != *oldChild) bundle.strAuthoringStatus = "DRAFT";
	std::string validationStatus;
	if (!m_bHasDraft ||
		!CKoukuSaydonCompositionDocument::Validate(
			candidate, m_ResourceReferences, validationStatus))
	{
		outStatus = "KoukuSaydon edit rejected; draft preserved: " + validationStatus;
		m_strStatus = outStatus;
		return false;
	}
	// Refresh only a definition that another committed edit actually changed;
	// unrelated geometry/timing edits must keep either panel's pending inputs.
	for (auto* draftId : { &m_strLogicValueDraftId, &m_strColliderLogicValueDraftId })
	{
		if (draftId->empty()) continue;
		const auto* previous = Find_Logic(m_Draft, *draftId);
		const auto* next = Find_Logic(candidate, *draftId);
		if (!previous || !next || *previous != *next) draftId->clear();
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

bool_t Client::CKoukuSaydonActionWorkbench::Set_PatternCreationDestination(
	const std::string_view folderId, const std::string_view bundleId, std::string& outStatus)
{
	if (!m_bHasDraft)
	{ outStatus = m_strStatus = "Load the Composition before choosing a Pattern destination."; return false; }
	if (!folderId.empty())
	{
		const auto folder = std::find_if(m_Draft.Folders.begin(), m_Draft.Folders.end(),
			[&](const auto& row) { return row.strFolderId == folderId; });
		if (folder == m_Draft.Folders.end() || folder->strGateId != m_strSelectedGateId || !folder->strLoadError.empty())
		{ outStatus = m_strStatus = "Choose a valid Parent from the selected Gate."; return false; }
	}
	if (!bundleId.empty())
	{
		const auto bundle = std::find_if(m_Draft.Bundles.begin(), m_Draft.Bundles.end(),
			[&](const auto& row) { return row.strBundleId == bundleId; });
		if (bundle == m_Draft.Bundles.end() || bundle->strGateId != m_strSelectedGateId ||
			bundle->strFolderId != folderId || !bundle->strLoadError.empty())
		{ outStatus = m_strStatus = "Choose a valid Bundle under the selected Parent, or None."; return false; }
	}
	const std::string parent(folderId), bundle(bundleId);
	m_strCreateFolderId = parent;
	m_strCreateBundleId = bundle;
	outStatus = "Pattern creation destination selected.";
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
	if (!Set_PatternCreationDestination(m_strCreateFolderId, m_strCreateBundleId, outStatus)) return false;
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
	pattern.strFolderId = m_strCreateBundleId.empty() ? m_strCreateFolderId : std::string{};
	pattern.strPatternId = "KAKULSAYDON_G1_PATTERN_" +
		std::to_string(candidate.iNextPatternOrdinal++);
	pattern.strDisplayName = std::string(displayName);
	pattern.strActorProfileId = m_strCreateActorProfileId;
	pattern.strGateId = m_strSelectedGateId;
	pattern.strTargetBossPlacementId = CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(pattern.strGateId, pattern.strActorProfileId);
	if (pattern.strTargetBossPlacementId.empty()) { outStatus = "Select a valid target boss for this Gate."; return false; }
	pattern.strAuthoringStatus = "DRAFT";
	pattern.strCategory = std::string(category);
	pattern.iNextStageOrdinal = 1u;
	pattern.iNextAnimationOrdinal = 1u;
	const std::string patternId = pattern.strPatternId;
	candidate.Patterns.push_back(std::move(pattern));
	if (!m_strCreateBundleId.empty())
	{
		auto bundle = std::find_if(candidate.Bundles.begin(), candidate.Bundles.end(), [&](const auto& item) { return item.strBundleId == m_strCreateBundleId; });
		if (bundle == candidate.Bundles.end() || bundle->strGateId != m_strSelectedGateId)
		{ outStatus = "The selected destination bundle is unavailable."; return false; }
		bundle->Members.push_back({ bundle->strBundleId + ".member." + std::to_string(bundle->iNextMemberOrdinal++), patternId, 0u });
		bundle->strAuthoringStatus = "DRAFT";
	}
	Rebuild_PlayAllPatternIds(candidate);
	if (!Commit_Candidate(std::move(candidate),
			"Created KoukuSaydon draft Pattern " + patternId + ".", outStatus))
	{
		return false;
	}
	Stop_Preview();
	m_bPresentationPreviewRequestPending = false;
	Clear_TimelineSelection();
	m_bFitRequested = true;
	m_ePatternSelection = KOUKU_PATTERN_SELECTION::PATTERN;
	m_strSelectedFolderId = m_strCreateFolderId;
	m_strSelectedBundleId = m_strCreateBundleId;
	m_strBundleReturnId = m_strCreateBundleId;
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
	for (const auto& bundle : m_Draft.Bundles)
		for (const auto& member : bundle.Members)
			if (member.strPatternId == patternId)
			{ outStatus = m_strStatus = "Remove the reference from bundle " + bundle.strDisplayName + " before deleting the Pattern."; return false; }
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

bool_t Client::CKoukuSaydonActionWorkbench::Rename_Item(const RENAME_TARGET target,
	const std::string_view id, const std::string_view displayName, std::string& outStatus)
{
	if (displayName.empty() || displayName.size() >= std::size(m_RenameDisplayName) ||
		std::all_of(displayName.begin(), displayName.end(), [](const unsigned char c) { return std::isspace(c) != 0; }))
	{ outStatus = m_strStatus = "Enter a non-empty display name of at most 255 UTF-8 bytes."; return false; }
	auto candidate = m_Draft;
	std::string* name = nullptr;
	switch (target)
	{
	case RENAME_TARGET::PATTERN:
		if (auto* row = Find_Pattern(candidate, id); row && row->strLoadError.empty()) name = &row->strDisplayName;
		break;
	case RENAME_TARGET::FOLDER:
		for (auto& row : candidate.Folders) if (row.strFolderId == id && row.strLoadError.empty()) name = &row.strDisplayName;
		break;
	case RENAME_TARGET::BUNDLE:
		for (auto& row : candidate.Bundles) if (row.strBundleId == id && row.strLoadError.empty()) name = &row.strDisplayName;
		break;
	case RENAME_TARGET::LOGIC:
		for (auto& row : candidate.Logics) if (row.strLogicId == id) name = &row.strDisplayName;
		break;
	case RENAME_TARGET::SUMMON:
		for (auto& row : candidate.Summons) if (row.strSummonId == id) name = &row.strDisplayName;
		break;
	case RENAME_TARGET::WORLD:
		for (auto& row : candidate.Worlds) if (row.strWorldId == id) name = &row.strDisplayName;
		break;
	case RENAME_TARGET::SCENE_PROFILE:
		for (auto& row : candidate.SceneProfiles) if (row.strSceneProfileId == id) name = &row.strDisplayName;
		break;
	case RENAME_TARGET::PRESENTATION:
		for (auto& row : candidate.PresentationResources) if (row.strResourceId == id) name = &row.strDisplayName;
		break;
	}
	if (!m_bHasDraft || !name)
	{ outStatus = m_strStatus = "Rename target is unavailable; the previous name is preserved."; return false; }
	if (*name == displayName)
	{ outStatus = m_strStatus = "The display name is unchanged."; return true; }
	*name = std::string(displayName);
	return Commit_Candidate(std::move(candidate), "Renamed selected item. Save keeps the name and all existing connections.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Render_RenameControl(const RENAME_TARGET target,
	const std::string_view id, const std::string_view displayName)
{
	const char* const popup = m_bSequenceWorkspace ?
		"Rename selected item###SequenceRenameSelectedItem" : "Rename selected item";
	ImGui::PushID(static_cast<int>(target));
	bool_t applied = false;
	if (ImGui::Button("Rename"))
	{
		if (Copy_Text(m_RenameDisplayName, std::size(m_RenameDisplayName), displayName))
		{
			m_strRenameItemId = id;
			ImGui::OpenPopup(popup);
		}
		else m_strStatus = "The current name exceeds the editor capacity; the previous name is preserved.";
	}
	if (ImGui::BeginPopupModal(popup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextWrapped("%s", m_strRenameItemId.c_str());
		if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
		ImGui::SetNextItemWidth(380.f);
		const bool enter = ImGui::InputText("Display name", m_RenameDisplayName,
			std::size(m_RenameDisplayName), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
		ImGui::BeginDisabled(m_RenameDisplayName[0] == '\0');
		if (ImGui::Button("Apply name") || (enter && m_RenameDisplayName[0] != '\0'))
		{
			std::string status;
			applied = Rename_Item(target, m_strRenameItemId, m_RenameDisplayName, status);
			if (applied) ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
		ImGui::TextDisabled("Apply changes the draft. Save writes the composition.");
		if (!m_strStatus.empty()) ImGui::TextWrapped("%s", m_strStatus.c_str());
		ImGui::EndPopup();
	}
	ImGui::PopID();
	return applied;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_PatternFolder(
	const std::string_view patternId, const std::string_view folderId, std::string& outStatus)
{
	const auto* source = Find_Pattern(m_Draft, patternId);
	if (!m_bHasDraft || !source || !source->strLoadError.empty())
	{ outStatus = m_strStatus = "Choose a valid Pattern before changing its Parent."; return false; }
	if (!folderId.empty())
	{
		const auto folder = std::find_if(m_Draft.Folders.begin(), m_Draft.Folders.end(),
			[&](const auto& row) { return row.strFolderId == folderId; });
		if (folder == m_Draft.Folders.end() || folder->strGateId != source->strGateId || !folder->strLoadError.empty())
		{ outStatus = m_strStatus = "Choose a valid Parent from the Pattern's Gate."; return false; }
	}
	if (source->strFolderId == folderId)
	{ outStatus = "Pattern Parent is unchanged."; return true; }
	const std::string parent(folderId);
	const bool_t selected = m_strSelectedPatternId == patternId;
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	Find_Pattern(candidate, patternId)->strFolderId = parent;
	if (!Commit_Candidate(std::move(candidate), "Changed Pattern Parent.", outStatus)) return false;
	if (selected)
	{
		m_strSelectedFolderId = parent;
		m_strCreateFolderId = parent;
		m_strSelectedBundleId.clear();
		m_strCreateBundleId.clear();
		m_strBundleReturnId.clear();
	}
	return true;
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
	// Detail and timeline Delete share the same lane/movement clock splice.
	return Delete_TimelineSelection(patternId, { std::string(stageId) }, {}, outStatus);
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
	if (0 == direction || (m_TimelineSelectedStageIds.empty() && m_TimelineSelectedOccurrenceIds.empty()))
	{
		outStatus = m_strStatus = "Select Stage or animation boxes and a direction to move.";
		return false;
	}
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	if (!pattern || !pattern->strLoadError.empty() || m_strTimelineSelectionPatternId != patternId)
	{
		outStatus = m_strStatus = "Move selection target Pattern is unavailable or invalid.";
		return false;
	}
	std::unordered_set<std::string> selectedStages;
	for (const auto& id : m_TimelineSelectedStageIds)
	{
		if (!Find_Stage(*pattern, id))
		{
			outStatus = m_strStatus = "Move rejected; Stage is unavailable: " + id;
			return false;
		}
		selectedStages.insert(id);
	}
	for (const auto& id : m_TimelineSelectedOccurrenceIds)
	{
		const auto occurrence = Find_Occurrence(*pattern, id);
		if (!occurrence.pStage)
		{
			outStatus = m_strStatus = "Move rejected; animation box is unavailable: " + id;
			return false;
		}
		selectedStages.insert(occurrence.pStage->strStageId);
	}
	auto& stages = pattern->Stages;
	if (stages.empty() || selectedStages.contains(direction < 0 ?
		stages.front().strStageId : stages.back().strStageId))
	{
		outStatus = m_strStatus = "Selected Stages are already at that edge; selection preserved.";
		return false;
	}
	// Walk toward the move direction so every selected run crosses one
	// unselected neighbour without swapping its own members.
	if (direction < 0)
	{
		for (std::size_t index = 1u; index < stages.size(); ++index)
			if (selectedStages.contains(stages[index].strStageId) &&
				!selectedStages.contains(stages[index - 1u].strStageId))
				std::swap(stages[index], stages[index - 1u]);
	}
	else
	{
		for (std::size_t index = stages.size() - 1u; index > 0u; --index)
			if (selectedStages.contains(stages[index - 1u].strStageId) &&
				!selectedStages.contains(stages[index].strStageId))
				std::swap(stages[index - 1u], stages[index]);
	}
	Mark_Draft(candidate, *pattern);
	// Commit normalizes the existing stable selection; do not collapse it
	// through the single-box selection helper after a batch move.
	return Commit_Candidate(std::move(candidate), direction < 0 ?
		"Moved selected Stages one slot earlier with their animation boxes. Press Save." :
		"Moved selected Stages one slot later with their animation boxes. Press Save.", outStatus);
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

bool_t Client::CKoukuSaydonActionWorkbench::Set_StageRetargetOnEnter(
	const std::string_view patternId,
	const std::string_view stageId,
	const bool_t retargetOnEnter,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_STAGE* const stage = nullptr == pattern ?
		nullptr : Find_Stage(*pattern, stageId);
	if (nullptr == stage)
	{
		outStatus = "KoukuSaydon Stage retarget target is absent.";
		return false;
	}
	stage->bRetargetOnEnter = retargetOnEnter;
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Changed Stage entry retarget. Press Save.", outStatus);
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
	m_bBundlePreviewRequestPending = false;
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
	m_bBundlePreviewRequestPending = false;
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
	if (!m_bHasDraft || nullptr == pattern ||
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

bool_t Client::CKoukuSaydonActionWorkbench::Set_AnimationBlend(const std::string_view patternId,
	const std::string_view occurrenceId, const std::uint32_t blendInMs, std::string& outStatus)
{
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	const auto found = pattern ? Find_Occurrence(*pattern, occurrenceId) : MUTABLE_OCCURRENCE{};
	if (!pattern || !pattern->strLoadError.empty() || !found.pOccurrence || blendInMs > 1000u)
	{ outStatus = m_strStatus = "Choose an editable Animation box and Blend In between 0 and 1000 ms."; return false; }
	found.pOccurrence->iBlendInMs = blendInMs;
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Updated Animation Blend In. Zero uses an immediate transition.", outStatus);
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
	const bool missingBundle = m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE &&
		std::none_of(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto& item) { return item.strBundleId == m_strSelectedBundleId; });
	const bool missingFolder = m_ePatternSelection == KOUKU_PATTERN_SELECTION::FOLDER &&
		std::none_of(m_Draft.Folders.begin(), m_Draft.Folders.end(), [&](const auto& item) { return item.strFolderId == m_strSelectedFolderId; });
	if (missingBundle || missingFolder || (m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN && !Find_Pattern(m_Draft, m_strSelectedPatternId)))
		Select_Hierarchy(KOUKU_PATTERN_SELECTION::GATE, m_strSelectedGateId);
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
	if (!m_strColliderLogicValueDraftId.empty() && nullptr == Find_Logic(m_Draft, m_strColliderLogicValueDraftId))
		m_strColliderLogicValueDraftId.clear();
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
		{ return !Has_TimelineOccurrence(*pattern, id); });
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
	Cancel_PresentationGeometryPreview(false);
	std::erase_if(m_StagedPresentationGeometry, [&](const auto& row) {
		const auto* owner = Find_Pattern(m_Draft, row.strPatternId);
		const auto* box = owner ? Find_PresentationBox(*owner, row.Occurrence.strOccurrenceId) : nullptr;
		return !box || box->strResourceId != row.Occurrence.strResourceId ||
			(box->PositionOffset == row.Occurrence.PositionOffset && box->RotationDegrees == row.Occurrence.RotationDegrees && box->Scale == row.Occurrence.Scale);
	});
	m_PatternName[0] = '\0';
	m_iPatternDurationMs = 0;
	m_iOccurrenceStartOffsetMs = 0;
	m_iOccurrenceSourceStartMs = 0;
	m_iOccurrencePlayMs = 1;
	m_fOccurrencePlayRate = 1.f;
	m_iOccurrenceEndPolicy = 0;
	m_iOccurrenceBlendInMs = 0;
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
	{
		m_PresentationBoxEdit = *box;
		const auto staged = std::find_if(m_StagedPresentationGeometry.begin(), m_StagedPresentationGeometry.end(),
			[&](const auto& row) { return row.strPatternId == pattern->strPatternId && row.Occurrence.strOccurrenceId == box->strOccurrenceId; });
		if (staged != m_StagedPresentationGeometry.end()) Copy_PresentationGeometry(staged->Occurrence, m_PresentationBoxEdit);
	}
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
	m_iOccurrenceBlendInMs = static_cast<int32_t>(occurrence->iBlendInMs);
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
		if (Is_Dirty())
			m_bReloadConfirmationRequested = true;
		else
		{
			std::string status;
			(void)Reload(status);
		}
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(publishing || !m_bHasDraft || !m_Document.Is_Fresh() || !Is_Dirty());
	if (ImGui::Button("Save"))
	{
		std::string status;
		(void)Save(status);
	}
	ImGui::EndDisabled();

	if (!m_bSequenceWorkspace)
	{
		ImGui::SameLine();
		const auto* selectedPattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
		const auto bundle = std::find_if(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto& item) { return item.strBundleId == m_strSelectedBundleId; });
		const bool bundleSelected = m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE && bundle != m_Draft.Bundles.end();
		const bool executionSelected = bundleSelected ? bundle->strLoadError.empty() :
			(m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN && selectedPattern && selectedPattern->strLoadError.empty());
		ImGui::BeginDisabled(publishing || Is_Dirty() || !m_Document.Is_Fresh() || !executionSelected);
		if (ImGui::Button("Complete Play (Server)"))
		{
			if (bundleSelected) { m_strPendingBundleServerId = bundle->strBundleId; m_iPendingBundleServerRevision = m_Draft.iRevision; }
			else { m_strPendingServerPlayPatternId = m_strSelectedPatternId; m_iPendingServerPlaySourceRevision = m_Draft.iRevision; m_bServerPlayRequestPending = true; }
			m_strStatus = "Requested the saved execution unit on the Server.";
		}
		ImGui::EndDisabled();
	}
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
		freshness, Is_Dirty() ? " | dirty" : "", m_strStatus.c_str());
}

namespace
{
	const char* Gate_Label(const std::string_view gate)
	{
		if (gate == "GATE1") return "\x31\xEA\xB4\x80\xEB\xAC\xB8";
		if (gate == "GATE2") return "\x32\xEA\xB4\x80\xEB\xAC\xB8";
		if (gate == "GATE3") return "\x33\xEA\xB4\x80\xEB\xAC\xB8";
		if (gate == "BINGO") return "\xEB\xB9\x99\xEA\xB3\xA0";
		return "Unknown Gate";
	}
	template<class Document> auto Find_Bundle(Document& document, const std::string_view id) -> decltype(&document.Bundles.front())
	{
		auto it = std::find_if(document.Bundles.begin(), document.Bundles.end(), [&](const auto& item) { return item.strBundleId == id; });
		return it == document.Bundles.end() ? nullptr : &*it;
	}
	template<class Document> auto Find_Folder(Document& document, const std::string_view id) -> decltype(&document.Folders.front())
	{
		auto it = std::find_if(document.Folders.begin(), document.Folders.end(), [&](const auto& item) { return item.strFolderId == id; });
		return it == document.Folders.end() ? nullptr : &*it;
	}
	std::uint32_t Bundle_DurationMs(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& doc, const KOUKU_SAYDON_COMPOSITION_BUNDLE& bundle)
	{
		std::uint64_t end = 0;
		for (const auto& member : bundle.Members)
			if (const auto* pattern = Find_Pattern(doc, member.strPatternId))
				end = (std::max)(end, std::uint64_t(member.iStartOffsetMs) + Pattern_DurationMs(*pattern));
		for (const auto& row : bundle.SceneProfileOccurrences) end = (std::max)(end, std::uint64_t(row.iStartMs) + row.iDurationMs);
		for (const auto& row : bundle.PresentationOccurrences) end = (std::max)(end, std::uint64_t(row.iStartMs) + row.iDurationMs);
		return static_cast<std::uint32_t>((std::min)(end, std::uint64_t(MAX_EDITOR_TIME_MS)));
	}
}

void Client::CKoukuSaydonActionWorkbench::Select_Hierarchy(const KOUKU_PATTERN_SELECTION kind, const std::string_view id)
{
	Stop_Preview();
	m_bPresentationPreviewRequestPending = false;
	m_bServerPlayRequestPending = false;
	m_strPendingBundleServerId.clear();
	m_strSelectedPatternId.clear();
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	Clear_TimelineSelection();
	m_strBundleDragMemberId.clear();
	m_ePatternSelection = kind;
	m_HierarchyName[0] = '\0';
	if (kind == KOUKU_PATTERN_SELECTION::GATE)
	{
		m_strSelectedGateId = std::string(id);
		m_strSelectedFolderId.clear(); m_strCreateFolderId.clear(); m_strSelectedBundleId.clear(); m_strCreateBundleId.clear(); m_strBundleReturnId.clear();
		m_strModelViewProfile.clear();
		m_strCreateActorProfileId = id == "GATE2" ? "MN_RPCZ_00" : "MN_RPCT_05";
	}
	else if (kind == KOUKU_PATTERN_SELECTION::FOLDER)
	{
		m_strSelectedFolderId = std::string(id); m_strCreateFolderId = std::string(id); m_strSelectedBundleId.clear(); m_strCreateBundleId.clear(); m_strBundleReturnId.clear();
		if (const auto* folder = Find_Folder(m_Draft, id))
		{ m_strSelectedGateId = folder->strGateId; Copy_Text(m_HierarchyName, std::size(m_HierarchyName), folder->strDisplayName); }
	}
	else if (kind == KOUKU_PATTERN_SELECTION::BUNDLE)
	{
		m_strSelectedBundleId = std::string(id); m_strCreateBundleId = std::string(id); m_strBundleReturnId = std::string(id);
		if (const auto* bundle = Find_Bundle(m_Draft, id))
		{ m_strSelectedGateId = bundle->strGateId; m_strSelectedFolderId = bundle->strFolderId; m_strCreateFolderId = bundle->strFolderId; Copy_Text(m_HierarchyName, std::size(m_HierarchyName), bundle->strDisplayName); }
	}
	m_strCursorPatternId = std::string(id);
	m_iCursorMs = 0; m_bFitRequested = true;
	m_strStatus = "Selected " + std::string(id);
}

bool_t Client::CKoukuSaydonActionWorkbench::Select_BundleById(const std::string_view id, std::string& status)
{
	if (!Find_Bundle(m_Draft, id)) { status = "Bundle is not present in this document."; return false; }
	Select_Hierarchy(KOUKU_PATTERN_SELECTION::BUNDLE, id); status = m_strStatus; return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Create_Hierarchy(const bool_t createBundle)
{
	auto candidate = m_Draft;
	std::string id, status;
	if (createBundle)
	{
		const auto* folder = Find_Folder(candidate, m_strSelectedFolderId);
		if (!folder || folder->strGateId != m_strSelectedGateId || !folder->strLoadError.empty()) { m_strStatus = "Select a valid parent folder first."; return false; }
		KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
		bundle.strBundleId = "kakulsaydon.bundle." + std::to_string(candidate.iNextBundleOrdinal++);
		bundle.strFolderId = folder->strFolderId; bundle.strGateId = folder->strGateId; bundle.strDisplayName = m_NewBundleName;
		id = bundle.strBundleId; candidate.Bundles.push_back(std::move(bundle));
	}
	else
	{
		KOUKU_SAYDON_COMPOSITION_FOLDER folder;
		folder.strFolderId = "kakulsaydon.folder." + std::to_string(candidate.iNextFolderOrdinal++);
		folder.strGateId = m_strSelectedGateId; folder.strDisplayName = m_NewFolderName;
		id = folder.strFolderId; candidate.Folders.push_back(std::move(folder));
	}
	if (!Commit_Candidate(std::move(candidate), "Created hierarchy item.", status)) return false;
	Select_Hierarchy(createBundle ? KOUKU_PATTERN_SELECTION::BUNDLE : KOUKU_PATTERN_SELECTION::FOLDER, id);
	(createBundle ? m_NewBundleName : m_NewFolderName)[0] = '\0';
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Link_BundlePattern(const std::string_view patternId)
{
	auto candidate = m_Draft;
	auto* bundle = Find_Bundle(candidate, m_strSelectedBundleId);
	const auto* pattern = Find_Pattern(candidate, patternId);
	if (!bundle || !pattern || bundle->strGateId != pattern->strGateId) { m_strStatus = "Choose a Pattern from this Gate."; return false; }
	if (std::any_of(bundle->Members.begin(), bundle->Members.end(), [&](const auto& member) { return member.strPatternId == patternId; }))
	{ m_strStatus = "This Pattern is already linked."; return false; }
	bundle->Members.push_back({bundle->strBundleId + ".member." + std::to_string(bundle->iNextMemberOrdinal++), std::string(patternId), 0u});
	bundle->strAuthoringStatus = "DRAFT";
	std::string status; return Commit_Candidate(std::move(candidate), "Linked existing Pattern; source clips preserved.", status);
}

void Client::CKoukuSaydonActionWorkbench::Render_PatternsAndResources()
{
	ImGui::SeparatorText(m_bSequenceWorkspace ? "Sequences by Gate" : "Patterns by Gate");
	ImGui::BeginDisabled(Is_PublishRunning() || !m_bHasDraft || !m_Document.Is_Fresh() || !Is_Dirty());
	if (ImGui::Button("Save##PatternsByGate"))
	{
		std::string status;
		(void)Save(status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextUnformatted(Is_Dirty() ? "Unsaved changes" : "Saved");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", m_bSequenceWorkspace ?
			"Save stores all sequences in this independent workspace." :
			"Save stores changes across all gates, parents, bundles and patterns. Publish All Patterns synchronizes F1 separately.");
	if (ImGui::BeginCombo("Gate##Composition", Gate_Label(m_strSelectedGateId)))
	{
		for (const auto* gate : {"GATE1", "GATE2", "GATE3", "BINGO"})
			if (ImGui::Selectable(Gate_Label(gate), m_strSelectedGateId == gate)) Select_Hierarchy(KOUKU_PATTERN_SELECTION::GATE, gate);
		ImGui::EndCombo();
	}
	if (ImGui::BeginCombo("Model View", m_strModelViewProfile.empty() ? "All" : Actor_Label(m_strModelViewProfile)))
	{
		std::string status;
		if (ImGui::Selectable("All", m_strModelViewProfile.empty())) (void)Select_ActorProfile({}, status);
		for (const auto* actor : ACTOR_PROFILES)
			if (!CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(m_strSelectedGateId, actor).empty() &&
				ImGui::Selectable(Actor_Label(actor), m_strModelViewProfile == actor)) (void)Select_ActorProfile(actor, status);
		ImGui::EndCombo();
	}
	const auto matches = [&](const auto& pattern) { return pattern.strGateId == m_strSelectedGateId && (m_strModelViewProfile.empty() || pattern.strActorProfileId == m_strModelViewProfile); };
	const auto bundleMatches = [&](const auto& bundle) { return m_strModelViewProfile.empty() || std::any_of(bundle.Members.begin(), bundle.Members.end(), [&](const auto& member) { const auto* p = Find_Pattern(m_Draft, member.strPatternId); return p && matches(*p); }); };
	if (ImGui::BeginChild("##KoukuPatternList", ImVec2(0.f, 480.f), ImGuiChildFlags_Borders))
	{
		if (ImGui::Selectable(Gate_Label(m_strSelectedGateId), m_ePatternSelection == KOUKU_PATTERN_SELECTION::GATE)) Select_Hierarchy(KOUKU_PATTERN_SELECTION::GATE, m_strSelectedGateId);
		for (const auto& folder : m_Draft.Folders)
		{
			if (folder.strGateId != m_strSelectedGateId) continue;
			if (!m_strModelViewProfile.empty() &&
				!std::any_of(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto& b) { return b.strFolderId == folder.strFolderId && bundleMatches(b); }) &&
				!std::any_of(m_Draft.Patterns.begin(), m_Draft.Patterns.end(), [&](const auto& p) { return p.strFolderId == folder.strFolderId && matches(p); })) continue;
			const auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen |
				(m_ePatternSelection == KOUKU_PATTERN_SELECTION::FOLDER && m_strSelectedFolderId == folder.strFolderId ? ImGuiTreeNodeFlags_Selected : 0);
			const bool open = ImGui::TreeNodeEx((folder.strDisplayName + " [Parent]##" + folder.strFolderId).c_str(), flags);
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) Select_Hierarchy(KOUKU_PATTERN_SELECTION::FOLDER, folder.strFolderId);
			if (open)
			{
				for (const auto& bundle : m_Draft.Bundles)
				{
					if (bundle.strFolderId != folder.strFolderId || !bundleMatches(bundle)) continue;
					const bool childOpen = ImGui::TreeNodeEx((bundle.strDisplayName + " [Bundle]##" + bundle.strBundleId).c_str(),
						ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE && m_strSelectedBundleId == bundle.strBundleId ? ImGuiTreeNodeFlags_Selected : 0));
					if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) Select_Hierarchy(KOUKU_PATTERN_SELECTION::BUNDLE, bundle.strBundleId);
					if (childOpen)
					{
						for (const auto& member : bundle.Members)
						{
							const auto* pattern = Find_Pattern(m_Draft, member.strPatternId);
							const auto label = pattern ? pattern->strDisplayName + " [" + Actor_Label(pattern->strActorProfileId) + "]" : member.strPatternId + " [Missing]";
							if (pattern && !m_strModelViewProfile.empty() && matches(*pattern)) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.45f, .85f, 1.f, 1.f));
							if (ImGui::Selectable((label + "##" + member.strMemberId).c_str(), m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN && m_strSelectedPatternId == member.strPatternId))
							{
								m_strBundleReturnId = bundle.strBundleId; m_strCreateBundleId = bundle.strBundleId;
								std::string status; (void)Select_PatternById(member.strPatternId, status);
							}
							if (pattern && !m_strModelViewProfile.empty() && matches(*pattern)) ImGui::PopStyleColor();
						}
						ImGui::TreePop();
					}
				}
				for (const auto& pattern : m_Draft.Patterns)
				{
					if (pattern.strFolderId != folder.strFolderId || !matches(pattern)) continue;
					const auto label = pattern.strDisplayName + " [" + Actor_Label(pattern.strActorProfileId) + "]##" + pattern.strPatternId;
					if (ImGui::Selectable(label.c_str(), m_strSelectedPatternId == pattern.strPatternId))
					{ m_strBundleReturnId.clear(); std::string status; (void)Select_PatternById(pattern.strPatternId, status); }
				}
				ImGui::TreePop();
			}
		}
		for (const auto& pattern : m_Draft.Patterns)
		{
			const auto* parent = Find_Folder(m_Draft, pattern.strFolderId);
			if (!matches(pattern) || (parent && parent->strGateId == pattern.strGateId)) continue;
			const bool linked = std::any_of(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto& b) { return std::any_of(b.Members.begin(), b.Members.end(), [&](const auto& m) { return m.strPatternId == pattern.strPatternId; }); });
			if (linked) continue;
			if (ImGui::Selectable((pattern.strDisplayName + " [" + Actor_Label(pattern.strActorProfileId) + "]##" + pattern.strPatternId).c_str(), m_strSelectedPatternId == pattern.strPatternId))
			{ m_strBundleReturnId.clear(); m_strCreateBundleId.clear(); std::string status; (void)Select_PatternById(pattern.strPatternId, status); }
		}
	}
	ImGui::EndChild();
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN)
	{
		if (const auto* item = Find_Pattern(m_Draft, m_strSelectedPatternId); item && item->strLoadError.empty())
			if (Render_RenameControl(RENAME_TARGET::PATTERN, item->strPatternId, item->strDisplayName)) return;
	}
	else if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::FOLDER)
	{
		if (const auto* item = Find_Folder(m_Draft, m_strSelectedFolderId); item && item->strLoadError.empty())
			if (Render_RenameControl(RENAME_TARGET::FOLDER, item->strFolderId, item->strDisplayName)) return;
	}
	else if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE)
	{
		if (const auto* item = Find_Bundle(m_Draft, m_strSelectedBundleId); item && item->strLoadError.empty())
			if (Render_RenameControl(RENAME_TARGET::BUNDLE, item->strBundleId, item->strDisplayName)) return;
	}
	if (!m_bSequenceWorkspace)
	{
		ImGui::BeginDisabled(Is_PublishRunning() || !m_bHasDraft || Is_Dirty() || !m_Document.Is_Fresh());
		if (ImGui::Button("Publish All Patterns##PatternList"))
		{
			std::string status;
			(void)Publish_AllPatterns(status);
		}
		ImGui::EndDisabled();
		ImGui::TextWrapped("Publishes every saved Parent, Bundle and Pattern across all gates. Unavailable items stay in the F1 tree with a reason; no individual publish selection is needed.");
		if (Is_Dirty()) ImGui::TextDisabled("Save changes before Publish All Patterns.");
		ImGui::TextDisabled("After publish, use Complete Play to run the new revision. Stop an active replay before starting the updated Pattern.");
	}
	ImGui::SeparatorText("Create Parent");
	ImGui::InputTextWithHint("##NewFolder", "Parent name", m_NewFolderName, std::size(m_NewFolderName));
	ImGui::BeginDisabled(!m_bHasDraft || m_NewFolderName[0] == '\0');
	if (ImGui::Button("Create Parent")) (void)Create_Hierarchy(false);
	ImGui::EndDisabled();
	ImGui::InputTextWithHint("##NewBundle", "Playback bundle name", m_NewBundleName, std::size(m_NewBundleName));
	ImGui::BeginDisabled(!Find_Folder(m_Draft, m_strSelectedFolderId) || m_NewBundleName[0] == '\0');
	if (ImGui::Button("Create Bundle")) (void)Create_Hierarchy(true);
	ImGui::EndDisabled();
	ImGui::SeparatorText(m_bSequenceWorkspace ? "Create Sequence" : "Create Pattern");
	ImGui::InputTextWithHint("##NewKoukuPattern", m_bSequenceWorkspace ? "Sequence display name" : "Pattern display name", m_NewPatternName, std::size(m_NewPatternName));
	if (ImGui::BeginCombo("Target Boss", Actor_Label(m_strCreateActorProfileId)))
	{
		for (const auto* actor : ACTOR_PROFILES)
			if (!CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(m_strSelectedGateId, actor).empty() && ImGui::Selectable(Actor_Label(actor), m_strCreateActorProfileId == actor)) m_strCreateActorProfileId = actor;
		ImGui::EndCombo();
	}
	const auto* parent = Find_Folder(m_Draft, m_strCreateFolderId);
	if (ImGui::BeginCombo("Parent##CreatePattern", parent ? parent->strDisplayName.c_str() :
		(m_strCreateFolderId.empty() ? "None (Gate root)" : "Missing Parent")))
	{
		std::string status;
		if (ImGui::Selectable("None (Gate root)", m_strCreateFolderId.empty()))
			(void)Set_PatternCreationDestination({}, {}, status);
		for (const auto& folder : m_Draft.Folders)
			if (folder.strGateId == m_strSelectedGateId && folder.strLoadError.empty() &&
				ImGui::Selectable((folder.strDisplayName + "##" + folder.strFolderId).c_str(), folder.strFolderId == m_strCreateFolderId))
				(void)Set_PatternCreationDestination(folder.strFolderId, {}, status);
		ImGui::EndCombo();
	}
	const auto* destination = Find_Bundle(m_Draft, m_strCreateBundleId);
	if (ImGui::BeginCombo("Bundle", destination ? destination->strDisplayName.c_str() : "None (Independent)"))
	{
		std::string status;
		if (ImGui::Selectable("None (Independent)", m_strCreateBundleId.empty()))
			(void)Set_PatternCreationDestination(m_strCreateFolderId, {}, status);
		for (const auto& bundle : m_Draft.Bundles)
			if (bundle.strGateId == m_strSelectedGateId && bundle.strFolderId == m_strCreateFolderId && bundle.strLoadError.empty() &&
				ImGui::Selectable((bundle.strDisplayName + "##" + bundle.strBundleId).c_str(), bundle.strBundleId == m_strCreateBundleId))
				(void)Set_PatternCreationDestination(m_strCreateFolderId, bundle.strBundleId, status);
		ImGui::EndCombo();
	}
	parent = Find_Folder(m_Draft, m_strCreateFolderId);
	destination = Find_Bundle(m_Draft, m_strCreateBundleId);
	ImGui::TextWrapped("Destination: %s / %s / %s", Gate_Label(m_strSelectedGateId),
		parent ? parent->strDisplayName.c_str() : "Gate root", destination ? destination->strDisplayName.c_str() :
		(m_bSequenceWorkspace ? "Independent Sequence" : "Independent Pattern"));
	ImGui::Combo("Category##CreatePattern", &m_iNewPatternCategory, PATTERN_CATEGORIES.data(), static_cast<int>(PATTERN_CATEGORIES.size()));
	ImGui::BeginDisabled(!m_bHasDraft || m_NewPatternName[0] == '\0');
	if (ImGui::Button(m_bSequenceWorkspace ? "Create Sequence###Create Pattern" : "Create Pattern")) { std::string id, status; if (Create_Pattern(m_NewPatternName, PATTERN_CATEGORIES[m_iNewPatternCategory], id, status)) m_NewPatternName[0] = '\0'; }
	ImGui::EndDisabled();
}

bool_t Client::CKoukuSaydonActionWorkbench::Consume_BundlePreviewRequest(std::string& id, std::uint32_t& clockMs, bool_t& paused)
{
	if (!m_bBundlePreviewRequestPending) return false;
	Cancel_CompleteSequencePlay();
	id = std::move(m_strPendingBundlePreviewId); clockMs = m_iPendingPreviewStartMs; paused = m_bPendingPreviewStartPaused;
	m_bBundlePreviewRequestPending = false;
	m_bPreviewResultStatusPending = true;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Consume_BundleServerPlayRequest(std::string& id, std::uint32_t& revision)
{
	if (m_bSequenceWorkspace || m_strPendingBundleServerId.empty()) return false;
	id = std::move(m_strPendingBundleServerId); m_strPendingBundleServerId.clear(); revision = m_iPendingBundleServerRevision; return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Request_BundlePreview(const std::uint32_t clockMs, const bool_t paused)
{
	const auto* bundle = Find_Bundle(m_Draft, m_strSelectedBundleId);
	if (!bundle || !bundle->strLoadError.empty() || bundle->Members.empty()) { m_strStatus = "Select a valid bundle with connected Patterns."; return false; }
	for (const auto& member : bundle->Members)
	{
		const auto* pattern = Find_Pattern(m_Draft, member.strPatternId);
		if (!pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0)
		{ m_strStatus = "Bundle cannot play: missing, invalid or empty child " + member.strPatternId; return false; }
	}
	m_PendingPresentationGeometryPreviews.clear();
	m_strPresentationGeometryPreviewPatternId.clear();
	m_strPresentationGeometryPreviewOccurrenceId.clear();
	m_bPatternPreviewRequestPending = false; m_bPreviewRequestPending = false; m_bPresentationPreviewRequestPending = false;
	m_bBundlePreviewRequestPending = true; m_strPendingBundlePreviewId = bundle->strBundleId;
	for (const auto& edit : m_StagedPresentationGeometry)
	{
		if (!Valid_PresentationGeometry(edit.Occurrence) || !std::any_of(bundle->Members.begin(), bundle->Members.end(),
			[&](const auto& member) { return member.strPatternId == edit.strPatternId; })) continue;
		const auto* owner = Find_Pattern(m_Draft, edit.strPatternId);
		const auto* source = owner ? Find_PresentationBox(*owner, edit.Occurrence.strOccurrenceId) : nullptr;
		const auto* resource = source ? Find_PresentationResource(m_Draft, source->strResourceId) : nullptr;
		if (!source || source->strResourceId != edit.Occurrence.strResourceId || !resource) continue;
		KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST request{ edit.strPatternId, *source };
		Copy_PresentationGeometry(edit.Occurrence, request.Occurrence);
		if (Valid_GameplayColliderScale(*resource, request.Occurrence))
			m_PendingPresentationGeometryPreviews.push_back(std::move(request));
	}
	m_iPendingPreviewStartMs = paused ? (std::min)(clockMs, Bundle_DurationMs(m_Draft, *bundle)) :
		(clockMs < Bundle_DurationMs(m_Draft, *bundle) ? clockMs : 0);
	m_bPendingPreviewStartPaused = paused; m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
	m_strStatus = "Bundle preview requested for " + std::to_string(bundle->Members.size()) + " actors.";
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Render_BundleTransport()
{
	const auto* bundle = Find_Bundle(m_Draft, m_strSelectedBundleId);
	if (!bundle) { ImGui::TextDisabled("Select a playback bundle."); return; }
	const bool active = m_PreviewState.bPlaying && m_PreviewState.strPatternId == bundle->strBundleId;
	bool ready = bundle->strLoadError.empty() && !bundle->Members.empty();
	for (const auto& member : bundle->Members) { const auto* p = Find_Pattern(m_Draft, member.strPatternId); ready &= p && p->strLoadError.empty() && Pattern_DurationMs(*p) > 0; }
	ImGui::BeginDisabled(!ready);
	if (ImGui::Button("Play Bundle")) (void)Request_BundlePreview(m_iCursorMs);
	ImGui::EndDisabled(); ImGui::SameLine();
	ImGui::BeginDisabled(!active);
	if (ImGui::Button(m_PreviewState.bPaused ? "Resume" : "Pause"))
	{
		if (m_PreviewState.bPaused) m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::RESUME;
		else (void)Request_PreviewPause();
	}
	ImGui::SameLine(); if (ImGui::Button("Stop Bundle")) (void)Request_PreviewPause();
	ImGui::SameLine(); if (ImGui::Button("Reset Bundle")) Stop_Preview();
	ImGui::EndDisabled();
	ImGui::SameLine(); ImGui::Text("%zu actors | %u / %u ms", bundle->Members.size(), active ? m_PreviewState.iClockMs : m_iCursorMs, Bundle_DurationMs(m_Draft, *bundle));
	if (!ready) ImGui::TextDisabled("Connect valid child Patterns with a positive lifetime before Play.");
}

void Client::CKoukuSaydonActionWorkbench::Render_BundleTimeline()
{
	const auto* selected = Find_Bundle(m_Draft, m_strSelectedBundleId);
	if (!selected) { ImGui::TextDisabled("The selected bundle is unavailable."); return; }
	const auto bundle = *selected;
	ImGui::BeginDisabled(Is_PublishRunning() || !m_bHasDraft || !m_Document.Is_Fresh() || !Is_Dirty());
	if (ImGui::Button("Save##BundleSequence")) { std::string status; (void)Save(status); }
	ImGui::EndDisabled(); ImGui::SameLine(); Render_BundleTransport();
	const auto duration = Bundle_DurationMs(m_Draft, bundle);
	ImGui::SliderFloat("Zoom##BundleSequence", &m_fPixelsPerSecond, 1.f, 500.f, "%.1f px/s");
	ImGui::SameLine(); if (ImGui::Button("Fit##Bundle")) m_bFitRequested = true;
	const float labelWidth = 230.f;
	if (m_bFitRequested && duration > 0)
	{ m_fPixelsPerSecond = (std::clamp)((ImGui::GetContentRegionAvail().x - labelWidth - 20.f) * 1000.f / duration, 1.f, 500.f); m_bFitRequested = false; }
	const float width = (std::max)(ImGui::GetContentRegionAvail().x - labelWidth - 20.f, (std::max)(duration, 1000u) * m_fPixelsPerSecond / 1000.f);
	const bool active = m_PreviewState.bPlaying && m_PreviewState.strPatternId == bundle.strBundleId;
	if (ImGui::BeginChild("##BundleTimeline", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
	{
		const auto origin = ImGui::GetCursorScreenPos();
		const float timeX = origin.x + labelWidth;
		auto* draw = ImGui::GetWindowDrawList();
		ImGui::SetCursorScreenPos(ImVec2(timeX, origin.y));
		ImGui::InvisibleButton("##BundleRuler", ImVec2(width, 24));
		if (ImGui::IsItemActive() || ImGui::IsItemDeactivated())
		{
			m_iCursorMs = static_cast<std::uint32_t>((std::clamp)((ImGui::GetIO().MousePos.x - timeX) * 1000.f / m_fPixelsPerSecond, 0.f, float(duration)));
			(void)Request_BundleScrub(m_iCursorMs);
		}
		const std::uint32_t step = m_fPixelsPerSecond < 30.f ? 5000u : 1000u;
		for (std::uint32_t tick = 0; tick <= (std::max)(duration, 1000u); tick += step)
		{
			const float x = timeX + tick * m_fPixelsPerSecond / 1000.f;
			draw->AddText(ImVec2(x + 2, origin.y + 2), IM_COL32(185, 195, 210, 255), (std::to_string(tick) + " ms").c_str());
		}
		float y = origin.y + 28;
		for (const auto& member : bundle.Members)
		{
			const auto* pattern = Find_Pattern(m_Draft, member.strPatternId);
			const auto name = pattern ? pattern->strDisplayName : member.strPatternId + " [Missing]";
			ImGui::PushID(member.strMemberId.c_str());
			ImGui::SetCursorScreenPos(ImVec2(origin.x, y));
			if (ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(labelWidth - 5, 24)) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && pattern)
			{ m_strBundleReturnId = bundle.strBundleId; std::string status; (void)Select_PatternById(member.strPatternId, status); }
			const float start = timeX + member.iStartOffsetMs * m_fPixelsPerSecond / 1000.f;
			const float span = (std::max)(6.f, (pattern ? Pattern_DurationMs(*pattern) : 0u) * m_fPixelsPerSecond / 1000.f);
			draw->AddRectFilled(ImVec2(timeX, y), ImVec2(timeX + width, y + 24), IM_COL32(29, 35, 43, 255));
			draw->AddRectFilled(ImVec2(start, y + 2), ImVec2(start + span, y + 22), pattern ? IM_COL32(60, 120, 177, 255) : IM_COL32(160, 60, 60, 255), 2.f);
			ImGui::SetCursorScreenPos(ImVec2(start, y + 2)); ImGui::InvisibleButton("##MemberBar", ImVec2(span, 20));
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\nStart %u ms. Double-click to edit child. Drag to move start.", member.strPatternId.c_str(), member.iStartOffsetMs);
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && pattern)
			{ m_strBundleReturnId = bundle.strBundleId; std::string status; (void)Select_PatternById(member.strPatternId, status); }
			if (ImGui::IsItemActivated()) { m_strBundleDragMemberId = member.strMemberId; m_iDragOriginOffsetMs = member.iStartOffsetMs; }
			if (ImGui::IsItemDeactivated() && m_strBundleDragMemberId == member.strMemberId)
			{
				const auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x * 1000.f / m_fPixelsPerSecond;
				auto candidate = m_Draft; auto* edit = Find_Bundle(candidate, bundle.strBundleId);
				if (edit) for (auto& m : edit->Members) if (m.strMemberId == member.strMemberId) m.iStartOffsetMs = static_cast<std::uint32_t>((std::clamp)(m_iDragOriginOffsetMs + delta, 0.f, float(MAX_EDITOR_TIME_MS)));
				if (edit && delta != 0) { edit->strAuthoringStatus = "DRAFT"; std::string status; (void)Commit_Candidate(std::move(candidate), "Changed bundle start offset.", status); }
				m_strBundleDragMemberId.clear();
			}
			ImGui::PopID(); y += 28;
		}
		const auto commonRow = [&](const auto& row, const char* label)
		{
			draw->AddText(ImVec2(origin.x, y + 3), IM_COL32(195, 200, 210, 255), label);
			const float x = timeX + row.iStartMs * m_fPixelsPerSecond / 1000.f;
			draw->AddRectFilled(ImVec2(x, y + 2), ImVec2(x + (std::max)(6.f, row.iDurationMs * m_fPixelsPerSecond / 1000.f), y + 22), IM_COL32(121, 83, 165, 255), 2.f);
			y += 28;
		};
		for (const auto& row : bundle.SceneProfileOccurrences) commonRow(row, "Scene Profile (Common)");
		for (const auto& row : bundle.PresentationOccurrences)
		{
			const float rowY = y;
			commonRow(row, "Camera (Common)");
			if (const auto* resource = Find_PresentationResource(m_Draft, row.strResourceId))
				if (const auto* shot = Find_AuthoringCamera(resource->strAssetId); shot && shot->iBlendOutMs)
				{
					const float tailX = timeX + (row.iStartMs + row.iDurationMs) * m_fPixelsPerSecond / 1000.f;
					draw->AddRectFilled(ImVec2(tailX, rowY + 5), ImVec2(tailX + shot->iBlendOutMs * m_fPixelsPerSecond / 1000.f, rowY + 19), IM_COL32(121, 83, 165, 70));
					draw->AddText(ImVec2(tailX + 2, rowY + 3), IM_COL32(195, 170, 220, 255), "return");
				}
		}
		const float cursorX = timeX + (active ? m_PreviewState.iClockMs : m_iCursorMs) * m_fPixelsPerSecond / 1000.f;
		draw->AddLine(ImVec2(cursorX, origin.y), ImVec2(cursorX, y + 8), IM_COL32(250, 190, 70, 255), 2);
		ImGui::SetCursorScreenPos(ImVec2(origin.x, y + 8)); ImGui::Dummy(ImVec2(labelWidth + width, 4));
	}
	ImGui::EndChild();
}

void Client::CKoukuSaydonActionWorkbench::Render_HierarchyDetails()
{
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::GATE)
	{ ImGui::Text("%s", Gate_Label(m_strSelectedGateId)); ImGui::TextWrapped("Create a parent and playback bundle, or create an independent Pattern."); return; }
	const auto* folder = Find_Folder(m_Draft, m_strSelectedFolderId);
	const auto* bundlePtr = Find_Bundle(m_Draft, m_strSelectedBundleId);
	const bool isBundle = m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE;
	if ((isBundle && !bundlePtr) || (!isBundle && !folder)) { ImGui::TextDisabled("Selection is no longer in the document."); return; }
	const auto id = isBundle ? bundlePtr->strBundleId : folder->strFolderId;
	const auto error = isBundle ? bundlePtr->strLoadError : folder->strLoadError;
	ImGui::SeparatorText(isBundle ? "Playback Bundle" : "Parent Folder");
	ImGui::TextWrapped("%s | %s", Gate_Label(m_strSelectedGateId), id.c_str());
	if (!error.empty()) { ImGui::TextWrapped("%s", error.c_str()); ImGui::TextDisabled("Original JSON preserved. Repair and Reload."); return; }
	if (ImGui::InputText("Display Name##Hierarchy", m_HierarchyName, std::size(m_HierarchyName), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		auto candidate = m_Draft;
		if (isBundle) Find_Bundle(candidate, id)->strDisplayName = m_HierarchyName;
		else Find_Folder(candidate, id)->strDisplayName = m_HierarchyName;
		std::string status; (void)Commit_Candidate(std::move(candidate), "Renamed hierarchy item.", status); return;
	}
	if (!isBundle)
	{
		bool hasChildren = false;
		for (const auto& bundle : m_Draft.Bundles) if (bundle.strFolderId == id)
		{ hasChildren = true; if (ImGui::Selectable((bundle.strDisplayName + "##Detail" + bundle.strBundleId).c_str())) { Select_Hierarchy(KOUKU_PATTERN_SELECTION::BUNDLE, bundle.strBundleId); return; } }
		for (const auto& pattern : m_Draft.Patterns) if (pattern.strFolderId == id)
		{
			hasChildren = true;
			if (ImGui::Selectable((pattern.strDisplayName + " [Pattern]##Detail" + pattern.strPatternId).c_str()))
			{ m_strBundleReturnId.clear(); std::string status; (void)Select_PatternById(pattern.strPatternId, status); return; }
		}
		ImGui::BeginDisabled(hasChildren);
		if (ImGui::Button("Delete Empty Parent"))
		{
			auto candidate = m_Draft; std::erase_if(candidate.Folders, [&](const auto& item) { return item.strFolderId == id; });
			std::string status; if (Commit_Candidate(std::move(candidate), "Deleted empty parent.", status)) Select_Hierarchy(KOUKU_PATTERN_SELECTION::GATE, m_strSelectedGateId);
		}
		ImGui::EndDisabled();
		if (hasChildren) ImGui::TextDisabled("Move or remove child bundles and Patterns before deleting this parent.");
		return;
	}
	const auto bundle = *bundlePtr;
	if (ImGui::BeginCombo("Parent Folder", folder ? folder->strDisplayName.c_str() : "Missing"))
	{
		std::string destination;
		for (const auto& item : m_Draft.Folders)
			if (item.strGateId == bundle.strGateId && ImGui::Selectable(item.strDisplayName.c_str(), item.strFolderId == bundle.strFolderId)) destination = item.strFolderId;
		ImGui::EndCombo();
		if (!destination.empty())
		{
			auto candidate = m_Draft;
			Find_Bundle(candidate, id)->strFolderId = destination;
			std::string status;
			if (Commit_Candidate(std::move(candidate), "Moved bundle to parent.", status))
			{
				m_strSelectedFolderId = destination;
				if (m_strCreateBundleId == id) m_strCreateFolderId = destination;
			}
			return;
		}
	}
	if (!m_bSequenceWorkspace)
		ImGui::TextDisabled("Publish All Patterns checks this bundle and its children together.");
	ImGui::SeparatorText(m_bSequenceWorkspace ? "Connected Sequences" : "Connected Patterns");
	for (const auto& member : bundle.Members)
	{
		const auto* pattern = Find_Pattern(m_Draft, member.strPatternId);
		ImGui::PushID(member.strMemberId.c_str());
		if (ImGui::Selectable(pattern ? pattern->strDisplayName.c_str() : member.strPatternId.c_str()))
		{ m_strBundleReturnId = id; std::string status; (void)Select_PatternById(member.strPatternId, status); ImGui::PopID(); return; }
		if (pattern) ImGui::TextWrapped("%s | %s", Actor_Label(pattern->strActorProfileId), pattern->strTargetBossPlacementId.c_str());
		else ImGui::TextColored(ImVec4(1,.35f,.35f,1), "Missing child; bundle playback is unavailable.");
		int offset = static_cast<int>(member.iStartOffsetMs);
		if (ImGui::InputInt("Start ms", &offset))
		{
			auto candidate = m_Draft; auto* edit = Find_Bundle(candidate, id);
			for (auto& item : edit->Members) if (item.strMemberId == member.strMemberId) item.iStartOffsetMs = static_cast<std::uint32_t>((std::clamp)(offset, 0, int(MAX_EDITOR_TIME_MS)));
			edit->strAuthoringStatus = "DRAFT"; std::string status; (void)Commit_Candidate(std::move(candidate), "Changed child start offset.", status); ImGui::PopID(); return;
		}
		const auto ticks = (std::uint64_t(member.iStartOffsetMs) * m_Draft.iFixedTickHz + 999u) / 1000u;
		ImGui::TextDisabled("Scheduled tick +%llu (%.2f ms)", static_cast<unsigned long long>(ticks), ticks * 1000.0 / m_Draft.iFixedTickHz);
		if (ImGui::SmallButton("Remove From Bundle"))
		{
			auto candidate = m_Draft; auto* edit = Find_Bundle(candidate, id);
			std::erase_if(edit->Members, [&](const auto& item) { return item.strMemberId == member.strMemberId; }); edit->strAuthoringStatus = "DRAFT";
			std::string status; (void)Commit_Candidate(std::move(candidate), "Removed link; original Pattern preserved.", status); ImGui::PopID(); return;
		}
		ImGui::PopID(); ImGui::Separator();
	}
	if (ImGui::BeginCombo(m_bSequenceWorkspace ? "Link Existing Sequence###Link Existing Pattern" : "Link Existing Pattern",
		m_bSequenceWorkspace ? "Choose a Sequence from this Gate" : "Choose a Pattern from this Gate"))
	{
		std::string target;
		for (const auto& pattern : m_Draft.Patterns)
			if (pattern.strGateId == bundle.strGateId && ImGui::Selectable((pattern.strDisplayName + " [" + Actor_Label(pattern.strActorProfileId) + "]##" + pattern.strPatternId).c_str())) target = pattern.strPatternId;
		ImGui::EndCombo(); if (!target.empty()) { (void)Link_BundlePattern(target); return; }
	}
	Render_BundleCommonDetails();
	ImGui::BeginDisabled(!bundle.Members.empty() || !bundle.SceneProfileOccurrences.empty() || !bundle.PresentationOccurrences.empty());
	if (ImGui::Button("Delete Empty Bundle"))
	{
		auto candidate = m_Draft; std::erase_if(candidate.Bundles, [&](const auto& item) { return item.strBundleId == id; });
		std::string status; if (Commit_Candidate(std::move(candidate), "Deleted empty bundle.", status)) Select_Hierarchy(KOUKU_PATTERN_SELECTION::FOLDER, bundle.strFolderId);
	}
	ImGui::EndDisabled();
}

void Client::CKoukuSaydonActionWorkbench::Render_BundleCommonDetails()
{
	const auto* source = Find_Bundle(m_Draft, m_strSelectedBundleId);
	if (!source) return;
	const auto bundle = *source;
	ImGui::SeparatorText("Common Scene Profile / Camera");
	ImGui::TextWrapped("Common lanes run once on the bundle clock. Child global lanes must be moved here before publishing the bundle.");
	for (const auto& row : bundle.SceneProfileOccurrences)
	{
		ImGui::PushID(row.strOccurrenceId.c_str());
		const auto* resource = Find_SceneProfile(m_Draft, row.strSceneProfileId);
		ImGui::TextWrapped("Scene: %s", resource ? resource->strDisplayName.c_str() : row.strSceneProfileId.c_str());
		int start = int(row.iStartMs), length = int(row.iDurationMs), blend = int(row.iBlendMs);
		bool changed = ImGui::InputInt("Start ms", &start); changed |= ImGui::InputInt("Duration ms", &length); changed |= ImGui::InputInt("Blend metadata ms", &blend);
		const bool remove = ImGui::SmallButton("Remove Scene"); ImGui::PopID();
		if (changed || remove)
		{
			auto candidate = m_Draft; auto* edit = Find_Bundle(candidate, bundle.strBundleId); edit->strAuthoringStatus = "DRAFT";
			if (remove) std::erase_if(edit->SceneProfileOccurrences, [&](const auto& item) { return item.strOccurrenceId == row.strOccurrenceId; });
			else for (auto& item : edit->SceneProfileOccurrences) if (item.strOccurrenceId == row.strOccurrenceId)
			{ item.iStartMs = (std::clamp)(start, 0, int(MAX_EDITOR_TIME_MS)); item.iDurationMs = (std::clamp)(length, 1, int(MAX_EDITOR_TIME_MS)); item.iBlendMs = (std::clamp)(blend, 0, int(MAX_EDITOR_TIME_MS)); }
			std::string status; (void)Commit_Candidate(std::move(candidate), "Updated common Scene Profile.", status); return;
		}
	}
	for (const auto& row : bundle.PresentationOccurrences)
	{
		ImGui::PushID(row.strOccurrenceId.c_str());
		const auto* resource = Find_PresentationResource(m_Draft, row.strResourceId);
		ImGui::TextWrapped("Camera: %s", resource ? resource->strDisplayName.c_str() : row.strResourceId.c_str());
		if (resource) Render_CameraAuthoring(resource->strAssetId);
		auto value = row; int start = int(row.iStartMs), length = int(row.iDurationMs);
		bool changed = ImGui::InputInt("Start ms", &start); changed |= ImGui::InputInt("Duration ms", &length);
		float offset[3] = {float(row.PositionOffset[0]), float(row.PositionOffset[1]), float(row.PositionOffset[2])};
		if (ImGui::DragFloat3("Camera offset", offset, .05f)) { for (int i=0;i<3;++i) value.PositionOffset[i]=offset[i]; changed=true; }
		const bool remove = ImGui::SmallButton("Remove Camera"); ImGui::PopID();
		if (changed || remove)
		{
			auto candidate = m_Draft; auto* edit = Find_Bundle(candidate, bundle.strBundleId); edit->strAuthoringStatus = "DRAFT";
			if (remove) std::erase_if(edit->PresentationOccurrences, [&](const auto& item) { return item.strOccurrenceId == row.strOccurrenceId; });
			else for (auto& item : edit->PresentationOccurrences) if (item.strOccurrenceId == row.strOccurrenceId)
			{ value.iStartMs = (std::clamp)(start,0,int(MAX_EDITOR_TIME_MS)); value.iDurationMs = (std::clamp)(length,1,int(MAX_EDITOR_TIME_MS)); item=value; }
			std::string status; (void)Commit_Candidate(std::move(candidate), "Updated common Camera.", status); return;
		}
	}
	if (ImGui::BeginCombo("Append Scene Profile", "Choose a saved resource"))
	{
		std::string chosen;
		for (const auto& resource : m_Draft.SceneProfiles) if (ImGui::Selectable(resource.strDisplayName.c_str())) chosen = resource.strSceneProfileId;
		ImGui::EndCombo();
		if (!chosen.empty())
		{
			const auto duration = Bundle_DurationMs(m_Draft, bundle);
			std::string occurrenceId, status;
			(void)Append_SceneProfileBox({}, chosen, m_iCursorMs, duration > m_iCursorMs ? duration - m_iCursorMs : 1000u, occurrenceId, status);
			return;
		}
	}
	if (ImGui::BeginCombo("Append Camera", "Choose a saved resource"))
	{
		std::string chosen;
		for (const auto& resource : m_Draft.PresentationResources) if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA && ImGui::Selectable(resource.strDisplayName.c_str())) chosen=resource.strResourceId;
		ImGui::EndCombo();
		if (!chosen.empty()) { std::string status; (void)Append_PresentationBox(chosen,status); }
	}
}

void Client::CKoukuSaydonActionWorkbench::Render_ResourcesWindow()
{
	if (!m_bResourcesOpen || !m_bOpen) return;
	ImGui::SetNextWindowSize(ImVec2(540.f, 820.f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(m_bSequenceWorkspace ? "Composition Resources###KoukuSequenceResources" :
		"Composition Resources###KoukuCompositionResources", &m_bResourcesOpen))
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
	const std::string targetPatternName = nullptr == selectedPattern ? "Select a Pattern" : selectedPattern->strDisplayName;

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
			(patternEditable && targetActorProfile == actionActor);
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
	const std::string_view patternId, const std::uint32_t startClockMs, std::string& outStatus, const bool_t startPaused)
{
	const auto* pattern = Find_Pattern(m_Draft, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0u)
	{
		outStatus = m_strStatus = "Select an editable Pattern with a positive lifetime to play.";
		return false;
	}
	Cancel_CompleteSequencePlay();
	const auto durationMs = Pattern_DurationMs(*pattern);
	m_PendingPresentationGeometryPreviews.clear();
	m_strPresentationGeometryPreviewPatternId.clear();
	m_strPresentationGeometryPreviewOccurrenceId.clear();
	m_bBundlePreviewRequestPending = false;
	m_bPresentationPreviewRequestPending = false;
	m_PendingPresentationPreviewRequest = {};
	m_PendingPatternPreview = *pattern;
	for (const auto& edit : m_StagedPresentationGeometry)
		if (edit.strPatternId == patternId && Valid_PresentationGeometry(edit.Occurrence))
			for (auto& box : m_PendingPatternPreview.PresentationOccurrences)
				if (box.strOccurrenceId == edit.Occurrence.strOccurrenceId && box.strResourceId == edit.Occurrence.strResourceId)
				{
					const auto* resource = Find_PresentationResource(m_Draft, box.strResourceId);
					auto geometry = box;
					Copy_PresentationGeometry(edit.Occurrence, geometry);
					if (resource && Valid_GameplayColliderScale(*resource, geometry)) box = std::move(geometry);
					break;
				}
	m_strPendingPreviewTargetAsset.clear();
	m_bPatternPreviewRequestPending = true;
	m_bPreviewRequestPending = false;
	m_iPendingPreviewStartMs = startPaused ? (std::min)(startClockMs, durationMs) : (startClockMs < durationMs ? startClockMs : 0u);
	m_bPendingPreviewStartPaused = startPaused;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
	m_strCursorPatternId = pattern->strPatternId;
	m_iCursorMs = m_iPendingPreviewStartMs;
	outStatus = m_strStatus = "Pattern preview requested from " + std::to_string(m_iCursorMs) + " ms.";
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Request_CompleteSequencePlay(std::string& outStatus)
{
	if (!m_bSequenceWorkspace || !m_bHasDraft)
	{ outStatus = m_strStatus = "Complete sequence playback requires the Sequence workspace."; return false; }
	std::vector<std::string> patternIds;
	for (const auto& pattern : m_Draft.Patterns)
	{
		if (pattern.strGateId != m_strSelectedGateId) continue;
		if (!pattern.strLoadError.empty() || Pattern_DurationMs(pattern) == 0u)
		{ outStatus = m_strStatus = "Complete Play cannot queue invalid sequence: " + pattern.strPatternId; return false; }
		patternIds.push_back(pattern.strPatternId);
	}
	if (patternIds.empty())
	{ outStatus = m_strStatus = "The selected Gate has no sequences to play."; return false; }
	return Queue_CompleteSequenceItem(std::move(patternIds), 0u, outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Queue_CompleteSequenceItem(
	std::vector<std::string> patternIds, const std::size_t index, std::string& outStatus)
{
	const std::string patternId = patternIds.at(index);
	// Normal selection clears the previous request; restore only this validated run afterwards.
	if (!Select_PatternById(patternId, outStatus) ||
		!Request_PatternPreview(patternId, 0u, outStatus))
	{
		Cancel_CompleteSequencePlay();
		outStatus = m_strStatus = "Complete Play stopped: " + outStatus;
		return false;
	}
	m_CompleteSequencePatternIds = std::move(patternIds);
	m_iCompleteSequenceIndex = index;
	m_bCompleteSequenceAdmitted = false;
	outStatus = m_strStatus = "Complete Play: sequence " + std::to_string(index + 1u) +
		" / " + std::to_string(m_CompleteSequencePatternIds.size()) + " from 0 ms.";
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Notify_SequencePreviewAdmission(
	const bool_t succeeded, const std::string& status)
{
	if (!Is_CompleteSequencePlaying()) return;
	if (succeeded) { m_bCompleteSequenceAdmitted = true; return; }
	Cancel_CompleteSequencePlay();
	m_strStatus = "Complete Play stopped: " + status;
}

bool_t Client::CKoukuSaydonActionWorkbench::Advance_CompleteSequencePlay(
	const std::string_view completedPatternId)
{
	if (!m_bCompleteSequenceAdmitted || !Is_CompleteSequencePlaying() ||
		m_CompleteSequencePatternIds[m_iCompleteSequenceIndex] != completedPatternId) return false;
	const auto next = m_iCompleteSequenceIndex + 1u;
	if (next == m_CompleteSequencePatternIds.size())
	{
		Cancel_CompleteSequencePlay();
		m_strStatus = "Complete Play finished all sequences in the selected Gate.";
		return true;
	}
	std::string status;
	(void)Queue_CompleteSequenceItem(m_CompleteSequencePatternIds, next, status);
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Cancel_CompleteSequencePlay()
{
	if (Is_CompleteSequencePlaying()) m_bPatternPreviewRequestPending = false;
	m_CompleteSequencePatternIds.clear();
	m_iCompleteSequenceIndex = 0u;
	m_bCompleteSequenceAdmitted = false;
}

void Client::CKoukuSaydonActionWorkbench::Render_CompleteSequenceTransport()
{
	if (!m_bSequenceWorkspace) return;
	ImGui::BeginDisabled(!m_bHasDraft);
	if (ImGui::Button("Complete Play"))
	{ std::string status; (void)Request_CompleteSequencePlay(status); }
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Play every sequence in the selected Gate in source order, starting at 0 ms.");
	if (!Is_CompleteSequencePlaying()) return;
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_PreviewState.bPlaying && !m_bPatternPreviewRequestPending);
	if (ImGui::Button(m_PreviewState.bPaused ? "Resume##CompleteSequence" : "Pause##CompleteSequence"))
	{
		if (m_PreviewState.bPaused) m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::RESUME;
		else (void)Request_PreviewPause();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Stop##CompleteSequence")) Stop_Preview();
	else
	{
		ImGui::SameLine();
		ImGui::Text("Sequence %zu / %zu", m_iCompleteSequenceIndex + 1u, m_CompleteSequencePatternIds.size());
	}
}

bool_t Client::CKoukuSaydonActionWorkbench::Request_PreviewPause()
{
	if (m_bPatternPreviewRequestPending || m_bBundlePreviewRequestPending)
	{
		m_bPendingPreviewStartPaused = true;
		m_iCursorMs = m_iPendingPreviewStartMs;
		m_iPendingSeekMs = m_iCursorMs;
		return true;
	}
	if (!m_PreviewState.bPlaying && !m_bPreviewRequestPending) return false;
	m_iCursorMs = m_PreviewState.iClockMs;
	m_iPendingSeekMs = m_iCursorMs;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::PAUSE;
	m_strStatus = "Stopped at the current pose. Drag the ruler to scrub; Reset releases the preview.";
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Request_PatternScrub(
	const std::string_view patternId, const std::uint32_t clockMs, std::string& outStatus)
{
	const auto* pattern = Find_Pattern(m_Draft, patternId);
	if (!pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0u)
	{ outStatus = m_strStatus = "Select a valid Pattern to scrub."; return false; }
	const auto cursor = (std::min)(clockMs, Pattern_DurationMs(*pattern));
	if (m_PreviewState.bPlaying && m_PreviewState.strPatternId == patternId)
	{
		m_iCursorMs = cursor; m_strCursorPatternId = std::string(patternId);
		m_iPendingSeekMs = cursor; m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::SEEK;
		outStatus = m_strStatus = "Preview cursor moved to " + std::to_string(cursor) + " ms.";
		return true;
	}
	return Request_PatternPreview(patternId, cursor, outStatus, true);
}

bool_t Client::CKoukuSaydonActionWorkbench::Request_BundleScrub(const std::uint32_t clockMs)
{
	const auto* bundle = Find_Bundle(m_Draft, m_strSelectedBundleId);
	if (!bundle || !bundle->strLoadError.empty() || bundle->Members.empty()) return false;
	const auto cursor = (std::min)(clockMs, Bundle_DurationMs(m_Draft, *bundle));
	if (m_PreviewState.bPlaying && m_PreviewState.strPatternId == bundle->strBundleId)
	{
		m_iCursorMs = cursor; m_iPendingSeekMs = cursor;
		m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::SEEK;
		return true;
	}
	if (!Request_BundlePreview(cursor, true)) return false;
	m_iCursorMs = cursor;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Request_PresentationGeometryPreview(
	const std::string_view patternId, const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& value,
	std::string& outStatus)
{
	const auto reject = [&](const char* reason) { outStatus = m_strStatus = reason; return false; };
	const auto* pattern = Find_Pattern(m_Draft, patternId);
	if (!pattern || !pattern->strLoadError.empty() || !Pattern_DurationMs(*pattern))
		return reject("Collider geometry preview needs a valid Pattern.");
	const auto* source = Find_PresentationBox(*pattern, value.strOccurrenceId);
	const auto* resource = Find_PresentationResource(m_Draft, value.strResourceId);
	if (!source || source->strResourceId != value.strResourceId || !resource ||
		(resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER && resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT))
		return reject("Presentation geometry preview needs an owned Collider or Effect box.");
	{
		const auto staged = std::find_if(m_StagedPresentationGeometry.begin(), m_StagedPresentationGeometry.end(),
			[&](const auto& row) { return row.strPatternId == patternId && row.Occurrence.strOccurrenceId == value.strOccurrenceId; });
		if (value.PositionOffset == source->PositionOffset && value.RotationDegrees == source->RotationDegrees && value.Scale == source->Scale)
		{
			if (staged != m_StagedPresentationGeometry.end()) m_StagedPresentationGeometry.erase(staged);
		}
		else
		{
			KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST pending{ std::string(patternId), *source };
			pending.Occurrence.PositionOffset = value.PositionOffset;
			pending.Occurrence.RotationDegrees = value.RotationDegrees;
			pending.Occurrence.Scale = value.Scale;
			if (staged == m_StagedPresentationGeometry.end()) m_StagedPresentationGeometry.push_back(std::move(pending));
			else *staged = std::move(pending);
		}
	}
	if (!Valid_PresentationGeometry(value))
		return reject("Geometry requires a finite position, rotation and positive scale. Save keeps the last applied source until corrected.");
	auto geometry = *source;
	Copy_PresentationGeometry(value, geometry);
	if (!Valid_GameplayColliderScale(*resource, geometry))
		return reject("Collider scale must be finite and positive. Adjust Scale / size; pending edits and previous source are preserved.");

	// Other unapplied Detail fields must not change timing, anchors or Logic during a drag.
	KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST request{ std::string(patternId), *source };
	request.Occurrence.PositionOffset = value.PositionOffset;
	request.Occurrence.RotationDegrees = value.RotationDegrees;
	request.Occurrence.Scale = value.Scale;
	const auto containsPattern = [&](const std::string_view ownerId) {
		if (ownerId == patternId) return true;
		const auto* bundle = Find_Bundle(m_Draft, ownerId);
		return bundle && std::any_of(bundle->Members.begin(), bundle->Members.end(),
			[&](const auto& member) { return member.strPatternId == patternId; });
	};
	const bool hasOwner = m_bPatternPreviewRequestPending ? m_PendingPatternPreview.strPatternId == patternId :
		m_bBundlePreviewRequestPending ? containsPattern(m_strPendingBundlePreviewId) :
		m_PreviewState.bPlaying && containsPattern(m_PreviewState.strPatternId);
	if (!hasOwner)
	{
		const auto clockMs = m_strCursorPatternId == patternId ? m_iCursorMs : 0u;
		if (!Request_PatternPreview(patternId, clockMs, outStatus, true)) return false;
	}
	if (m_strPresentationGeometryPreviewPatternId != patternId ||
		m_strPresentationGeometryPreviewOccurrenceId != value.strOccurrenceId)
		Cancel_PresentationGeometryPreview(false);
	m_strPresentationGeometryPreviewPatternId = std::string(patternId);
	m_strPresentationGeometryPreviewOccurrenceId = value.strOccurrenceId;
	if (m_bPatternPreviewRequestPending && m_PendingPatternPreview.strPatternId == patternId)
		for (auto& pending : m_PendingPatternPreview.PresentationOccurrences)
			if (pending.strOccurrenceId == value.strOccurrenceId)
			{
				pending.PositionOffset = request.Occurrence.PositionOffset;
				pending.RotationDegrees = request.Occurrence.RotationDegrees;
				pending.Scale = request.Occurrence.Scale;
				break;
			}
	const auto queued = std::find_if(m_PendingPresentationGeometryPreviews.begin(), m_PendingPresentationGeometryPreviews.end(),
		[&](const auto& row) { return row.strPatternId == patternId && row.Occurrence.strOccurrenceId == value.strOccurrenceId; });
	if (queued == m_PendingPresentationGeometryPreviews.end()) m_PendingPresentationGeometryPreviews.push_back(std::move(request));
	else *queued = std::move(request);
	outStatus = m_strStatus = "Presentation geometry updated at the actor cursor. Save keeps the edited geometry.";
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Consume_PresentationGeometryPreviewRequest(
	KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST& outRequest)
{
	if (m_PendingPresentationGeometryPreviews.empty()) return false;
	outRequest = std::move(m_PendingPresentationGeometryPreviews.front());
	m_PendingPresentationGeometryPreviews.erase(m_PendingPresentationGeometryPreviews.begin());
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Cancel_PresentationGeometryPreview(const bool_t discardStagedGeometry)
{
	if (m_strPresentationGeometryPreviewPatternId.empty()) return;
	const auto* pattern = Find_Pattern(m_Draft, m_strPresentationGeometryPreviewPatternId);
	const auto* source = pattern ? Find_PresentationBox(*pattern, m_strPresentationGeometryPreviewOccurrenceId) : nullptr;
	const auto* resource = source ? Find_PresentationResource(m_Draft, source->strResourceId) : nullptr;
	if (resource && (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
		resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER))
	{
		if (!discardStagedGeometry)
		{
			m_strPresentationGeometryPreviewPatternId.clear();
			m_strPresentationGeometryPreviewOccurrenceId.clear();
			return;
		}
		std::erase_if(m_StagedPresentationGeometry, [&](const auto& row) {
			return row.strPatternId == m_strPresentationGeometryPreviewPatternId &&
				row.Occurrence.strOccurrenceId == m_strPresentationGeometryPreviewOccurrenceId; });
	}
	if (source)
	{
		KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST request{ m_strPresentationGeometryPreviewPatternId, *source };
		if (m_bPatternPreviewRequestPending && m_PendingPatternPreview.strPatternId == request.strPatternId)
			for (auto& pending : m_PendingPatternPreview.PresentationOccurrences)
				if (pending.strOccurrenceId == source->strOccurrenceId)
				{
					pending.PositionOffset = source->PositionOffset;
					pending.RotationDegrees = source->RotationDegrees;
					pending.Scale = source->Scale;
					break;
				}
		const auto queued = std::find_if(m_PendingPresentationGeometryPreviews.begin(), m_PendingPresentationGeometryPreviews.end(),
			[&](const auto& row) { return row.strPatternId == request.strPatternId && row.Occurrence.strOccurrenceId == source->strOccurrenceId; });
		if (queued == m_PendingPresentationGeometryPreviews.end()) m_PendingPresentationGeometryPreviews.push_back(std::move(request));
		else *queued = std::move(request);
	}
	m_strPresentationGeometryPreviewPatternId.clear();
	m_strPresentationGeometryPreviewOccurrenceId.clear();
}

bool_t Client::CKoukuSaydonActionWorkbench::Request_ColliderBoxPreview(
	const std::string_view patternId, const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& value,
	std::string& outStatus)
{
	const auto reject = [&](const char* reason) { outStatus = m_strStatus = reason; return false; };
	const auto* pattern = Find_Pattern(m_Draft, patternId);
	if (!pattern || !pattern->strLoadError.empty() || pattern->strActorProfileId.empty() ||
		CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(pattern->strActorProfileId).empty())
		return reject("Collider preview needs an available Pattern actor.");
	const auto box = std::find_if(pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(),
		[&](const auto& row) { return row.strOccurrenceId == value.strOccurrenceId; });
	const auto resource = std::find_if(m_Draft.PresentationResources.begin(), m_Draft.PresentationResources.end(),
		[&](const auto& row) { return row.strResourceId == value.strResourceId && row.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER; });
	if (box == pattern->PresentationOccurrences.end() || box->strResourceId != value.strResourceId ||
		resource == m_Draft.PresentationResources.end())
		return reject("Collider preview box or resource identity is unavailable.");
	if (value.strAnchorKind != "BOSS" || !value.bFollowBoss || value.strBone.empty() ||
		value.strBone.size() > 128u || value.strBone == "." || value.strBone == ".." ||
		(value.strBoneTarget != "BODY" && value.strBoneTarget != "WEAPON") ||
		!value.strWorldId.empty() || !value.strWorldOccurrenceId.empty() ||
		!std::all_of(value.strBone.begin(), value.strBone.end(), [](const unsigned char ch) {
			return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
				(ch >= '0' && ch <= '9') || ch == '_' || ch == '-' || ch == '.'; }))
		return reject("Collider preview needs a named BODY/WEAPON Bone following its Boss anchor.");
	const auto finite = [](const auto& values, const double minimum, const double maximum) {
		return std::all_of(values.begin(), values.end(), [=](const double v) {
			return std::isfinite(v) && v >= minimum && v <= maximum; }); };
	if (!value.iDurationMs || std::uint64_t(value.iStartMs) + value.iDurationMs > Pattern_DurationMs(*pattern) ||
		!finite(value.PositionOffset, -100000.0, 100000.0) ||
		!finite(value.RotationDegrees, -36000.0, 36000.0) || !finite(value.Scale, 0.001, 10000.0))
		return reject("Collider preview needs a finite transform and a window inside its Pattern.");
	bool hasAnimation = false;
	for (const auto& stage : pattern->Stages)
		for (const auto& animation : stage.AnimationOccurrences)
		{
			if (CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(animation.strProfileId) != pattern->strActorProfileId ||
				animation.strRuntimeClip.empty())
				return reject("Collider preview animation does not match its Pattern actor.");
			hasAnimation = true;
		}
	if (!hasAnimation) return reject("Collider Bone preview needs its Pattern animation.");

	// Keep the actor and all animation clocks; only this temporary box consumes Detail edits.
	const auto index = static_cast<std::size_t>(box - pattern->PresentationOccurrences.begin());
	const bool_t keepPaused = m_PreviewState.bPlaying && m_PreviewState.bPaused && m_PreviewState.strPatternId == patternId;
	const auto previewClock = keepPaused ? m_PreviewState.iClockMs : value.iStartMs;
	if (!Request_PatternPreview(patternId, previewClock, outStatus, keepPaused)) return false;
	m_PendingPatternPreview.PresentationOccurrences[index] = value;
	outStatus = m_strStatus = "Collider Bone preview requested with Pattern animation from " +
		std::to_string(m_iPendingPreviewStartMs) + " ms.";
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Stop_Preview()
{
	Cancel_CompleteSequencePlay();
	m_PendingPresentationGeometryPreviews.clear();
	m_strPresentationGeometryPreviewPatternId.clear();
	m_strPresentationGeometryPreviewOccurrenceId.clear();
	m_bPatternPreviewRequestPending = false;
	m_bBundlePreviewRequestPending = false;
	m_bPreviewRequestPending = false;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::STOP;
	m_iCursorMs = 0u;
	m_strStatus = "Preview stop requested; the Pattern cursor is at zero.";
}

void Client::CKoukuSaydonActionWorkbench::Render_Transport()
{
	Render_CompleteSequenceTransport();
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE) { Render_BundleTransport(); return; }
	if (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN) { ImGui::TextDisabled("Select a playback bundle or Pattern."); return; }
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
	if (ImGui::Button(m_bSequenceWorkspace ? "Play Sequence###Play Pattern" : "Play Pattern"))
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
		if (m_PreviewState.bPaused) m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::RESUME;
		else (void)Request_PreviewPause();
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop"))
	{ if (m_bSequenceWorkspace) Stop_Preview(); else (void)Request_PreviewPause(); }
	ImGui::SameLine();
	if (ImGui::Button("Reset")) Stop_Preview();
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
		ImGui::SetTooltip("Drag the ruler to sample the pose, including while stopped. Stop holds this time; Reset releases the preview. Drag box edges to trim, the body to move, the Stage edge to resize.");

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
	const auto reject = [&](const std::string& message) { outStatus = m_strStatus = message; return false; };
	if (stageIds.empty() && occurrenceIds.empty()) return reject("Select timeline boxes to duplicate.");
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	if (!pattern || !pattern->strLoadError.empty()) return reject("Duplicate target Pattern is unavailable or invalid.");
	const auto source = *pattern;
	const std::string targetId = source.strPatternId;
	std::unordered_set<std::string> selectedStages(stageIds.begin(), stageIds.end());
	std::unordered_set<std::string> selected(occurrenceIds.begin(), occurrenceIds.end());
	for (const auto& id : selectedStages)
		if (!Find_Stage(source, id)) return reject("Duplicate Stage is unavailable: " + id);
	for (const auto& id : selected)
		if (!Has_TimelineOccurrence(source, id)) return reject("Duplicate box is unavailable: " + id);
	// Close only placement ownership. Long-lived target cards and the search
	// master remain shared; copying those would start a second encounter.
	bool expanded = true;
	while (expanded)
	{
		expanded = false;
		for (const auto& box : source.LogicOccurrences)
			if (selected.contains(box.strOccurrenceId) && !box.strHoldLogicOccurrenceId.empty())
				expanded |= selected.insert(box.strHoldLogicOccurrenceId).second;
		for (const auto& box : source.PresentationOccurrences)
		{
			const auto* resource = Find_PresentationResource(candidate, box.strResourceId);
			if (!resource) return reject("Duplicate resource is unavailable: " + box.strResourceId);
			if (selected.contains(box.strOccurrenceId))
			{
				if (!box.strLogicOccurrenceId.empty()) expanded |= selected.insert(box.strLogicOccurrenceId).second;
				if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT && !box.strWorldOccurrenceId.empty())
					expanded |= selected.insert(box.strWorldOccurrenceId).second;
			}
			// A judging Logic owns its full set of Collider regions. Copying only
			// one region would leave its new definition linked to the old Logic.
			if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER &&
				!box.strLogicOccurrenceId.empty() && selected.contains(box.strLogicOccurrenceId))
				expanded |= selected.insert(box.strOccurrenceId).second;
			if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT &&
				!box.strWorldOccurrenceId.empty() && selected.contains(box.strWorldOccurrenceId))
				expanded |= selected.insert(box.strOccurrenceId).second;
		}
	}
	for (const auto& id : selected)
		if (!Has_TimelineOccurrence(source, id)) return reject("A selected box has a missing linked owner: " + id);
	bool lanesSelected = false, animationsSelected = false;
	for (const auto& id : selected)
		if (Find_Occurrence(source, id)) animationsSelected = true; else lanesSelected = true;
	const bool splice = !selectedStages.empty() || animationsSelected;
	std::uint64_t first = MAX_EDITOR_TIME_MS, last = 0, total = 0, insertMs = 0;
	std::size_t insertIndex = 0;
	const auto include = [&](std::uint64_t start, std::uint64_t duration) {
		first = (std::min)(first, start); last = (std::max)(last, start + duration);
	};
	for (const auto& stage : source.Stages)
	{
		if (selectedStages.contains(stage.strStageId)) include(total, stage.iDurationMs);
		for (const auto& box : stage.AnimationOccurrences)
			if (selected.contains(box.strOccurrenceId)) include(total + box.iStartOffsetMs, box.iPlayMs);
		total += stage.iDurationMs;
	}
	const auto includeLane = [&](const auto& rows) {
		for (const auto& row : rows) if (selected.contains(row.strOccurrenceId)) include(row.iStartMs, row.iDurationMs);
	};
	includeLane(source.LogicOccurrences); includeLane(source.SummonOccurrences);
	includeLane(source.WorldOccurrences); includeLane(source.SceneProfileOccurrences); includeLane(source.PresentationOccurrences);
	if (last <= first || last > MAX_EDITOR_TIME_MS) return reject("Duplicate requires a positive selection span within 600000 ms.");
	std::unordered_map<std::string, std::string> ids, regionIds;
	std::vector<std::string> newStageSelection, newBoxSelection;
	std::vector<KOUKU_SAYDON_COMPOSITION_STAGE> stages;
	bool exhausted = false;
	const auto allocate = [&](std::uint32_t& ordinal, const std::string& prefix) {
		if (ordinal >= 1000000u) { exhausted = true; return std::string{}; }
		return prefix + std::to_string(ordinal++);
	};
	const auto cloneStage = [&](KOUKU_SAYDON_COMPOSITION_STAGE copy, const bool selectStage) {
		copy.strStageId = allocate(pattern->iNextStageOrdinal, "STAGE_");
		if (copy.strStageId.empty()) return;
		copy.strActionId = targetId + ".stage." + copy.strStageId.substr(6u);
		if (selectStage) newStageSelection.push_back(copy.strStageId);
		for (auto& box : copy.AnimationOccurrences)
		{
			const auto old = box.strOccurrenceId;
			box.strOccurrenceId = allocate(pattern->iNextAnimationOrdinal, targetId + ".animation.");
			ids.emplace(old, box.strOccurrenceId);
			if (!selectStage || lanesSelected) newBoxSelection.push_back(box.strOccurrenceId);
		}
		stages.push_back(std::move(copy));
	};
	std::uint64_t base = 0, delta = 0;
	for (std::size_t index = 0; splice && index < source.Stages.size(); ++index)
	{
		const auto& original = source.Stages[index];
		const auto end = base + original.iDurationMs;
		const bool whole = selectedStages.contains(original.strStageId);
		auto copy = original;
		if (!whole) std::erase_if(copy.AnimationOccurrences, [&](const auto& box) { return !selected.contains(box.strOccurrenceId); });
		if (!lanesSelected)
		{
			if (whole || !copy.AnimationOccurrences.empty())
			{
				if (!whole)
				{
					std::uint32_t a = MAX_EDITOR_TIME_MS, b = 0;
					for (const auto& box : copy.AnimationOccurrences) { a = (std::min)(a, box.iStartOffsetMs); b = (std::max)(b, box.iStartOffsetMs + box.iPlayMs); }
					for (auto& box : copy.AnimationOccurrences) box.iStartOffsetMs -= a;
					copy.iDurationMs = b - a;
				}
				delta += copy.iDurationMs; cloneStage(std::move(copy), whole);
				insertIndex = index + 1u; insertMs = end;
			}
		}
		else if (base < last && end > first)
		{
			const auto a = (std::max)(base, first), b = (std::min)(end, last);
			copy.iDurationMs = static_cast<std::uint32_t>(b - a);
			if (!whole && copy.AnimationOccurrences.empty()) copy.bRetargetOnEnter = false;
			for (auto& box : copy.AnimationOccurrences) box.iStartOffsetMs -= static_cast<std::uint32_t>(a - base);
			cloneStage(std::move(copy), true);
			insertIndex = index + 1u; insertMs = end;
		}
		base = end;
	}
	if (lanesSelected && splice)
	{
		delta = last - first;
		if (last > total)
		{
			KOUKU_SAYDON_COMPOSITION_STAGE tail;
			tail.strStageKind = "RECOVERY";
			tail.iDurationMs = static_cast<std::uint32_t>(last - total);
			cloneStage(std::move(tail), true);
			insertIndex = source.Stages.size(); insertMs = total;
		}
	}
	if (!splice) { insertMs = last; delta = last - first; }
	if (exhausted) return reject("Duplicate rejected; stable ID ordinals are exhausted.");
	if ((splice && total + delta > MAX_EDITOR_TIME_MS) || (!splice && last + delta > MAX_EDITOR_TIME_MS))
		return reject("Duplicate would exceed the 600000 ms timeline limit.");
	if (splice && pattern->BossMotion)
	{
		auto& motion = *pattern->BossMotion;
		if (motion.iStartMs >= insertMs) { motion.iStartMs += static_cast<std::uint32_t>(delta); motion.iEndMs += static_cast<std::uint32_t>(delta); }
		else if (motion.iEndMs > insertMs) motion.iEndMs += static_cast<std::uint32_t>(delta);
	}
	const auto shiftLane = [&](auto& rows) {
		for (auto& row : rows)
		{
			const std::uint64_t end = std::uint64_t(row.iStartMs) + row.iDurationMs;
			if (row.iStartMs >= insertMs) row.iStartMs += static_cast<std::uint32_t>(delta);
			else if (end > insertMs) row.iDurationMs += static_cast<std::uint32_t>(delta);
			if (std::uint64_t(row.iStartMs) + row.iDurationMs > MAX_EDITOR_TIME_MS) return false;
		}
		return true;
	};
	if (splice && (!shiftLane(pattern->LogicOccurrences) || !shiftLane(pattern->SummonOccurrences) ||
		!shiftLane(pattern->WorldOccurrences) || !shiftLane(pattern->SceneProfileOccurrences) || !shiftLane(pattern->PresentationOccurrences)))
		return reject("Duplicate would move an existing lane beyond 600000 ms.");
	const auto cloneLane = [&](const auto& originals, auto& rows, std::uint32_t& ordinal, const char* family) {
		for (const auto& original : originals) if (selected.contains(original.strOccurrenceId))
		{
			auto copy = original;
			copy.strOccurrenceId = allocate(ordinal, targetId + family);
			copy.iStartMs = static_cast<std::uint32_t>(insertMs + original.iStartMs - first);
			ids.emplace(original.strOccurrenceId, copy.strOccurrenceId);
			newBoxSelection.push_back(copy.strOccurrenceId);
			rows.push_back(std::move(copy));
		}
	};
	cloneLane(source.LogicOccurrences, pattern->LogicOccurrences, pattern->iNextLogicOccurrenceOrdinal, ".logic.");
	cloneLane(source.SummonOccurrences, pattern->SummonOccurrences, pattern->iNextSummonOccurrenceOrdinal, ".summon.");
	cloneLane(source.WorldOccurrences, pattern->WorldOccurrences, pattern->iNextWorldOccurrenceOrdinal, ".world.");
	cloneLane(source.SceneProfileOccurrences, pattern->SceneProfileOccurrences, pattern->iNextSceneProfileOccurrenceOrdinal, ".sceneprofile.");
	cloneLane(source.PresentationOccurrences, pattern->PresentationOccurrences, pattern->iNextPresentationOccurrenceOrdinal, ".presentation.");
	if (exhausted) return reject("Duplicate rejected; stable ID ordinals are exhausted.");
	const auto remap = [](std::string& id, const auto& mapping) { const auto found = mapping.find(id); if (found != mapping.end()) id = found->second; };
	for (std::size_t index = source.PresentationOccurrences.size(); index < pattern->PresentationOccurrences.size(); ++index)
	{
		auto& row = pattern->PresentationOccurrences[index];
		remap(row.strLogicOccurrenceId, ids); remap(row.strWorldOccurrenceId, ids);
		if (!row.strRegionId.empty()) { const auto old = row.strRegionId; row.strRegionId = row.strOccurrenceId + ".region"; regionIds.emplace(old, row.strRegionId); }
		for (const auto& pending : m_StagedPresentationGeometry)
			if (pending.strPatternId == targetId && ids.contains(pending.Occurrence.strOccurrenceId) && ids.at(pending.Occurrence.strOccurrenceId) == row.strOccurrenceId)
			{
				if (!Valid_PresentationGeometry(pending.Occurrence)) return reject("Correct or revert the selected presentation geometry before Duplicate.");
				Copy_PresentationGeometry(pending.Occurrence, row);
				const auto* resource = Find_PresentationResource(candidate, row.strResourceId);
				if (resource && !Valid_GameplayColliderScale(*resource, row))
					return reject("Collider scale must be finite and positive. Adjust Scale / size before Duplicate.");
			}
	}
	// Copy-on-write for typed references in reusable Logic definitions. Shared
	// definitions and external card/master/motion references remain untouched.
	std::unordered_map<std::string, std::string> logicIds;
	const auto cloneLogic = [&](std::string& id) {
		if (const auto existing = logicIds.find(id); existing != logicIds.end()) { id = existing->second; return true; }
		const auto found = std::find_if(candidate.Logics.begin(), candidate.Logics.end(), [&](const auto& value) { return value.strLogicId == id; });
		if (found == candidate.Logics.end()) return false;
		auto copy = *found;
		for (auto& target : copy.TargetWorldOccurrenceIds) remap(target, ids);
		for (auto& target : copy.ContactMotions) remap(target.strTargetWorldOccurrenceId, ids);
		remap(copy.strTargetLogicOccurrenceId, ids); remap(copy.strContactTargetWorldOccurrenceId, ids);
		for (auto& region : copy.RegionIds) remap(region, regionIds);
		if (copy == *found) return true;
		copy.strLogicId = allocate(candidate.iNextLogicOrdinal, "kakulsaydon.g1.logic.");
		copy.strDisplayName += " (copy)";
		logicIds.emplace(id, copy.strLogicId); id = copy.strLogicId;
		candidate.Logics.push_back(std::move(copy));
		return !exhausted;
	};
	for (std::size_t index = source.LogicOccurrences.size(); index < pattern->LogicOccurrences.size(); ++index)
	{
		auto& row = pattern->LogicOccurrences[index];
		remap(row.strHoldLogicOccurrenceId, ids);
		if (!cloneLogic(row.strLogicId)) return reject("Duplicate Logic definition is unavailable or its IDs are exhausted.");
		for (auto* outcomes : { &row.OnSuccessLogicIds, &row.OnFailLogicIds, &row.OnTimeoutLogicIds })
			for (auto& id : *outcomes) if (!cloneLogic(id)) return reject("Duplicate outcome definition is unavailable or its IDs are exhausted.");
	}
	if (splice) pattern->Stages.insert(pattern->Stages.begin() + insertIndex, std::make_move_iterator(stages.begin()), std::make_move_iterator(stages.end()));
	else if (last + delta > total)
	{
		if (pattern->Stages.empty()) return reject("Lane duplication requires a Pattern Stage clock.");
		pattern->Stages.back().iDurationMs += static_cast<std::uint32_t>(last + delta - total);
	}
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Duplicated selected timeline boxes and their owned links. Following clocks preserved. Press Save.", outStatus)) return false;
	Clear_TimelineSelection();
	m_strSelectedPatternId = targetId; m_strTimelineSelectionPatternId = targetId;
	m_TimelineSelectedStageIds = std::move(newStageSelection); m_TimelineSelectedOccurrenceIds = std::move(newBoxSelection);
	m_strSelectedStageId.clear(); m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear(); m_strSelectedSummonOccurrenceId.clear(); m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear(); m_strSelectedPresentationOccurrenceId.clear();
	if (!m_TimelineSelectedStageIds.empty()) m_strSelectedStageId = m_TimelineSelectedStageIds.back();
	for (const auto& id : m_TimelineSelectedOccurrenceIds)
		if (Find_Occurrence(*Find_Pattern(m_Draft, targetId), id).pOccurrence) m_strSelectedOccurrenceId = id;
	Normalize_Selection(); Synchronize_EditorFields(); m_bFitRequested = true;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_TimelineSelection(
	const std::string_view patternId, const std::vector<std::string>& stageIds,
	const std::vector<std::string>& occurrenceIds, std::string& outStatus)
{
	const auto reject = [&](const std::string& reason) { outStatus = m_strStatus = reason; return false; };
	if (stageIds.empty() && occurrenceIds.empty()) return reject("Select timeline boxes to delete.");
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	if (!pattern || !pattern->strLoadError.empty()) return reject("Delete target Pattern is unavailable or invalid.");
	const std::string targetId = pattern->strPatternId;
	std::unordered_set<std::string> stages(stageIds.begin(), stageIds.end()), boxes(occurrenceIds.begin(), occurrenceIds.end());
	for (const auto& id : stages) if (!Find_Stage(*pattern, id)) return reject("Delete Stage is unavailable: " + id);
	for (const auto& id : boxes) if (!Has_TimelineOccurrence(*pattern, id)) return reject("Delete box is unavailable: " + id);
	// Deleting an Object removes its owned companion, as the single-row command does.
	// A capture Trigger owns its Hold only while no surviving Trigger shares it.
	for (const auto& row : pattern->LogicOccurrences)
		if (boxes.contains(row.strOccurrenceId) && !row.strHoldLogicOccurrenceId.empty() &&
			std::none_of(pattern->LogicOccurrences.begin(), pattern->LogicOccurrences.end(), [&](const auto& other) {
				return !boxes.contains(other.strOccurrenceId) && other.strHoldLogicOccurrenceId == row.strHoldLogicOccurrenceId; }))
			boxes.insert(row.strHoldLogicOccurrenceId);
	for (auto& row : pattern->LogicOccurrences)
		if (boxes.contains(row.strHoldLogicOccurrenceId)) row.strHoldLogicOccurrenceId.clear();
	for (const auto& row : pattern->PresentationOccurrences)
		if (!row.strLogicOccurrenceId.empty() && boxes.contains(row.strLogicOccurrenceId)) boxes.insert(row.strOccurrenceId);
	for (const auto& row : pattern->PresentationOccurrences)
		if (!row.strWorldOccurrenceId.empty() && boxes.contains(row.strWorldOccurrenceId)) boxes.insert(row.strOccurrenceId);
	std::vector<std::pair<std::uint64_t, std::uint64_t>> removed;
	std::uint64_t base = 0;
	for (const auto& stage : pattern->Stages)
	{
		if (stages.contains(stage.strStageId)) removed.emplace_back(base, base + stage.iDurationMs);
		base += stage.iDurationMs;
	}
	const auto mappedTime = [&](const std::uint64_t time) {
		std::uint64_t result = time;
		for (const auto& [first, last] : removed)
			if (time > first) result -= (std::min)(time, last) - first;
		return result;
	};
	const auto eraseLane = [&](auto& rows) {
		std::erase_if(rows, [&](const auto& row) { return boxes.contains(row.strOccurrenceId); });
		for (auto& row : rows)
		{
			const auto begin = mappedTime(row.iStartMs), end = mappedTime(std::uint64_t(row.iStartMs) + row.iDurationMs);
			if (row.iDurationMs && end == begin) return false;
			row.iStartMs = static_cast<std::uint32_t>(begin); row.iDurationMs = static_cast<std::uint32_t>(end - begin);
		}
		return true;
	};
	if (!eraseLane(pattern->LogicOccurrences) || !eraseLane(pattern->SummonOccurrences) ||
		!eraseLane(pattern->WorldOccurrences) || !eraseLane(pattern->SceneProfileOccurrences) || !eraseLane(pattern->PresentationOccurrences))
		return reject("The removed Stage contains an unselected lane box. Select that box too before Delete.");
	if (pattern->BossMotion && !removed.empty())
	{
		auto& motion = *pattern->BossMotion;
		const auto startMs = mappedTime(motion.iStartMs), endMs = mappedTime(motion.iEndMs);
		if (endMs <= startMs)
			return reject("This selection removes the whole Boss Motion interval. Disable Move boss during Pattern before deleting that complete interval.");
		// Animation splices change the movement clock, preserving authored
		// positions and yaw. The same time mapping already retimes other lanes.
		motion.iStartMs = static_cast<std::uint32_t>(startMs);
		motion.iEndMs = static_cast<std::uint32_t>(endMs);
	}
	std::erase_if(pattern->Stages, [&](const auto& stage) { return stages.contains(stage.strStageId); });
	for (auto& stage : pattern->Stages)
		std::erase_if(stage.AnimationOccurrences, [&](const auto& row) { return boxes.contains(row.strOccurrenceId); });
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Deleted selected timeline boxes. Press Save.", outStatus)) return false;
	std::erase_if(m_StagedPresentationGeometry, [&](const auto& row) { return row.strPatternId == targetId && boxes.contains(row.Occurrence.strOccurrenceId); });
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
	// All-lane Sequencer selection: stable occurrence IDs, typed Detail focus.
	const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (!pattern) return;
	const auto isOccurrence = [&](const std::string& id) {
		return Find_Occurrence(*pattern, id) || Find_LogicBox(*pattern, id) ||
			Find_SummonBox(*pattern, id) || Find_WorldBox(*pattern, id) ||
			Find_SceneProfileBox(*pattern, id) || Find_PresentationBox(*pattern, id);
	};
	if (occurrenceId.empty() ? !Find_Stage(*pattern, stageId) : !isOccurrence(occurrenceId)) return;
	const std::string selectedId = occurrenceId.empty() ? stageId : occurrenceId;
	if (!toggle)
	{
		m_TimelineSelectedStageIds.clear();
		m_TimelineSelectedOccurrenceIds.clear();
	}
	auto& selected = occurrenceId.empty() ? m_TimelineSelectedStageIds : m_TimelineSelectedOccurrenceIds;
	const auto found = std::find(selected.begin(), selected.end(), selectedId);
	const bool removed = toggle && found != selected.end();
	if (removed) selected.erase(found);
	else if (found == selected.end()) selected.push_back(selectedId);
	std::string focusStage = occurrenceId.empty() ? selectedId : std::string{};
	std::string focusOccurrence = occurrenceId.empty() ? std::string{} : selectedId;
	if (removed)
	{
		focusStage.clear(); focusOccurrence.clear();
		if (!m_TimelineSelectedOccurrenceIds.empty()) focusOccurrence = m_TimelineSelectedOccurrenceIds.back();
		else if (!m_TimelineSelectedStageIds.empty()) focusStage = m_TimelineSelectedStageIds.back();
	}
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	const KOUKU_SAYDON_COMPOSITION_STAGE* animationStage = nullptr;
	if (!focusOccurrence.empty())
	{
		if (Find_Occurrence(*pattern, focusOccurrence, &animationStage))
		{
			m_strSelectedStageId = animationStage->strStageId;
			m_strSelectedOccurrenceId = focusOccurrence;
		}
		else if (Find_LogicBox(*pattern, focusOccurrence)) m_strSelectedLogicOccurrenceId = focusOccurrence;
		else if (Find_SummonBox(*pattern, focusOccurrence)) m_strSelectedSummonOccurrenceId = focusOccurrence;
		else if (Find_WorldBox(*pattern, focusOccurrence)) m_strSelectedWorldOccurrenceId = focusOccurrence;
		else if (Find_SceneProfileBox(*pattern, focusOccurrence)) m_strSelectedSceneProfileOccurrenceId = focusOccurrence;
		else if (Find_PresentationBox(*pattern, focusOccurrence)) m_strSelectedPresentationOccurrenceId = focusOccurrence;
	}
	else m_strSelectedStageId = focusStage;
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
}

void Client::CKoukuSaydonActionWorkbench::Render_Timeline()
{
	Render_CompleteSequenceTransport();
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE) { Render_BundleTimeline(); return; }
	if (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN) { ImGui::TextDisabled("Select a playback bundle or Pattern to edit its sequence."); return; }
	if (m_strTimelineSelectionPatternId != m_strSelectedPatternId)
	{
		Clear_TimelineSelection();
		m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	}
	const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool_t patternReady = nullptr != pattern && pattern->strLoadError.empty();
	const std::string patternId = nullptr == pattern ? std::string{} : pattern->strPatternId;
	// All-lane Sequencer selection: stable occurrence IDs, typed Detail focus.
	// Append/Detail paths may initially focus a box without a batch selection.
	if (patternReady && m_TimelineSelectedStageIds.empty() && m_TimelineSelectedOccurrenceIds.empty())
		for (const auto* id : { &m_strSelectedOccurrenceId, &m_strSelectedLogicOccurrenceId,
			&m_strSelectedSummonOccurrenceId, &m_strSelectedWorldOccurrenceId,
			&m_strSelectedSceneProfileOccurrenceId, &m_strSelectedPresentationOccurrenceId })
			if (!id->empty()) { m_TimelineSelectedOccurrenceIds.push_back(*id); break; }
	const auto canReorderSelection = [&]() {
		return patternReady && (!m_TimelineSelectedStageIds.empty() || !m_TimelineSelectedOccurrenceIds.empty()) &&
			std::all_of(m_TimelineSelectedOccurrenceIds.begin(), m_TimelineSelectedOccurrenceIds.end(),
				[&](const auto& id) { return nullptr != Find_Occurrence(*pattern, id); });
	};
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
	ImGui::BeginDisabled(publishing || !m_bHasDraft || !m_Document.Is_Fresh() || !Is_Dirty());
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
	if (ImGui::Button("Stop##KoukuSequencer"))
	{ if (m_bSequenceWorkspace) Stop_Preview(); else (void)Request_PreviewPause(); }
	ImGui::SameLine(); if (ImGui::Button("Reset##KoukuSequencer")) Stop_Preview();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("%s | %u / %u ms%s", Is_Dirty() ? "Unsaved changes" : "Saved",
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
	const bool_t hasSelection = !m_TimelineSelectedStageIds.empty() || !m_TimelineSelectedOccurrenceIds.empty();
	ImGui::BeginDisabled(publishing || !patternReady || !hasSelection);
	bool_t deleteRequested = ImGui::Button("Delete##KoukuSequencerSelection");
	ImGui::SameLine();
	bool_t duplicateRequested = ImGui::Button("Duplicate##KoukuSequencerSelection");
	ImGui::SameLine();
	// Reorder owns Stage/Animation order; other lane clocks are pattern-relative.
	ImGui::BeginDisabled(!canReorderSelection());
	int32_t moveRequested = 0;
	if (ImGui::Button("< Earlier##KoukuSequencerSelection")) moveRequested = -1;
	ImGui::SameLine();
	if (ImGui::Button("Later >##KoukuSequencerSelection")) moveRequested = 1;
	ImGui::EndDisabled();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("%zu Stages + %zu lane boxes",
		m_TimelineSelectedStageIds.size(), m_TimelineSelectedOccurrenceIds.size());
	ImGui::TextDisabled("Ctrl+click: toggle any lane | drag empty space: enclose boxes across lanes | Ctrl+drag: add | Ctrl+D: duplicate | Delete: remove");
	ImGui::TextDisabled("Earlier/Later and Left/Right reorder Stage/Animation selections only. Mixed lane selections keep their authored clocks.");
	ImGui::TextWrapped("%s", Get_Status().c_str());
	if (saveRequested || deleteRequested || duplicateRequested || durationRequested ||
		0 != moveRequested)
	{
		std::string status;
		if (0 != moveRequested)
			(void)Move_SelectedStage(patternId, moveRequested, status);
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
	std::uint32_t cameraDisplayEnd = durationMs;
	for (const auto& row : pattern->PresentationOccurrences)
		if (const auto* resource = Find_PresentationResource(m_Draft, row.strResourceId);
			resource && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
			if (const auto* shot = Find_AuthoringCamera(resource->strAssetId))
				cameraDisplayEnd = (std::max)(cameraDisplayEnd, row.iStartMs + row.iDurationMs + shot->iBlendOutMs);
	const f32_t timelineWidth = (std::max)(available.x, labelWidth + cameraDisplayEnd * scale + 24.f);
	constexpr std::size_t animationLane = 0u, logicLane = 1u, summonLane = 2u, worldLane = 3u, sceneLane = 4u;
	constexpr std::size_t presentationLaneBegin = 5u;
	struct TIMELINE_INTERVAL
	{
		std::string occurrenceId;
		std::uint64_t startMs, durationMs;
		std::uint32_t tailMs = 0u;
	};
	struct TIMELINE_LANE
	{
		std::vector<TIMELINE_INTERVAL> intervals;
		std::unordered_map<std::string, std::size_t> occurrenceRows;
		std::size_t firstRow = 0u, rowCount = 1u;
	};
	std::array<TIMELINE_LANE, presentationLaneBegin + 5u> lanes;
	std::uint64_t stageBaseMs = 0u;
	for (const auto& stage : pattern->Stages)
	{
		for (const auto& box : stage.AnimationOccurrences)
			lanes[animationLane].intervals.push_back({ box.strOccurrenceId, stageBaseMs + box.iStartOffsetMs, box.iPlayMs });
		stageBaseMs += stage.iDurationMs;
	}
	const auto appendIntervals = [&](const std::size_t lane, const auto& boxes)
	{
		for (const auto& box : boxes)
			lanes[lane].intervals.push_back({ box.strOccurrenceId, box.iStartMs, box.iDurationMs });
	};
	appendIntervals(logicLane, pattern->LogicOccurrences);
	appendIntervals(summonLane, pattern->SummonOccurrences);
	appendIntervals(worldLane, pattern->WorldOccurrences);
	appendIntervals(sceneLane, pattern->SceneProfileOccurrences);
	for (const auto& box : pattern->PresentationOccurrences)
	{
		const auto* resource = Find_PresentationResource(m_Draft, box.strResourceId);
		if (!resource) continue;
		std::uint32_t tailMs = 0u;
		if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
			if (const auto* shot = Find_AuthoringCamera(resource->strAssetId)) tailMs = shot->iBlendOutMs;
		lanes[presentationLaneBegin + static_cast<std::size_t>(resource->eKind)].intervals.push_back(
			{ box.strOccurrenceId, box.iStartMs, box.iDurationMs, tailMs });
	}
	// All box families use the same visual intervals and row allocation.
	// Rows belong to their one named track; they do not create new saved tracks.
	const auto minimumBoxMs = static_cast<std::uint64_t>(std::ceil(8.f / scale));
	std::size_t nextRow = 1u; // The sequential Stage strip precedes the box tracks.
	for (auto& lane : lanes)
	{
		std::stable_sort(lane.intervals.begin(), lane.intervals.end(),
			[](const auto& left, const auto& right) { return left.startMs < right.startMs; });
		std::vector<std::uint64_t> rowEnds;
		for (const auto& interval : lane.intervals)
		{
			std::size_t row = 0u;
			while (row < rowEnds.size() && rowEnds[row] > interval.startMs) ++row;
			const auto end = interval.startMs + (std::max)(interval.durationMs, minimumBoxMs) + interval.tailMs;
			if (row == rowEnds.size()) rowEnds.push_back(end); else rowEnds[row] = end;
			lane.occurrenceRows.emplace(interval.occurrenceId, row);
		}
		lane.rowCount = (std::max)(std::size_t{ 1u }, rowEnds.size());
		lane.firstRow = nextRow;
		nextRow += lane.rowCount;
	}
	const f32_t height = rulerHeight + TIMELINE_LANE_HEIGHT * static_cast<f32_t>(nextRow);
	if (!ImGui::BeginChild("##KoukuTimeline", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders,
		ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImGui::EndChild();
		return;
	}
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	ImDrawList* draw = ImGui::GetWindowDrawList();
	const auto laneY = [&](const std::size_t lane) {
		return origin.y + rulerHeight + TIMELINE_LANE_HEIGHT * static_cast<f32_t>(lanes[lane].firstRow);
	};
	const auto boxY = [&](const std::size_t lane, const std::string& occurrenceId) {
		return laneY(lane) + TIMELINE_LANE_HEIGHT * static_cast<f32_t>(lanes[lane].occurrenceRows.at(occurrenceId));
	};
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
	/* The ruler samples the selected time even before Play or after Stop. */
	ImGui::SetCursorScreenPos(ImVec2(origin.x + labelWidth, origin.y));
	ImGui::InvisibleButton("##KoukuRuler",
		ImVec2((std::max)(8.f, durationMs * scale), rulerHeight));
	if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActive() && durationMs > 0u)
	{
		const f32_t mouseMs = (ImGui::GetIO().MousePos.x - (origin.x + labelWidth)) / scale;
		const auto clockMs = static_cast<std::uint32_t>(std::clamp(
			std::llround(mouseMs), 0ll, static_cast<long long>(durationMs)));
		std::string scrubStatus;
		(void)Request_PatternScrub(patternId, clockMs, scrubStatus);
	}
	const std::uint32_t playheadMs = patternPreview ? m_PreviewState.iClockMs : m_iCursorMs;
	if (playheadMs <= durationMs)
	{
		const f32_t playheadX = origin.x + labelWidth + playheadMs * scale;
		draw->AddLine(ImVec2(playheadX, origin.y), ImVec2(playheadX, origin.y + height),
			patternPreview ? IM_COL32(255, 220, 72, 230) : IM_COL32(200, 200, 200, 140), 1.5f);
	}
	draw->AddText(ImVec2(origin.x + 4.f, origin.y + rulerHeight + 4.f), IM_COL32_WHITE, "Stages");
	draw->AddText(ImVec2(origin.x + 4.f, laneY(animationLane) + 4.f),
		IM_COL32(240, 188, 98, 255), "Animation");
	draw->AddText(ImVec2(origin.x + 4.f, laneY(logicLane) + 4.f),
		IM_COL32(236, 170, 110, 255), "Logic");

	std::string editStageId, editOccurrenceId, editLogicBoxId;
	std::uint32_t newOffset = 0u, newSourceStart = 0u, newPlayMs = 0u, newStageDuration = 0u;
	std::uint32_t newLogicStartMs = 0u, newLogicDurationMs = 0u;
	const f32_t summonLaneY = laneY(summonLane);
	draw->AddText(ImVec2(origin.x + 4.f, summonLaneY + 4.f),
		IM_COL32(150, 220, 180, 255), "Summon");
	std::string editSummonBoxId;
	std::uint32_t newSummonStartMs = 0u, newSummonDurationMs = 0u;
	const f32_t worldLaneY = laneY(worldLane);
	draw->AddText(ImVec2(origin.x + 4.f, worldLaneY + 4.f),
		IM_COL32(150, 190, 240, 255), "World");
	std::string editWorldBoxId;
	std::uint32_t newWorldStartMs = 0u, newWorldDurationMs = 0u;
	const f32_t sceneLaneY = laneY(sceneLane);
	draw->AddText(ImVec2(origin.x + 4.f, sceneLaneY + 4.f),
		IM_COL32(200, 170, 240, 255), "Scene Profile");
	std::string editSceneBoxId;
	std::uint32_t newSceneStartMs = 0u, newSceneDurationMs = 0u;
	std::string editPresentationBoxId;
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE editedPresentationBox;
	for (int i = 0; i < 5; ++i)
		draw->AddText(ImVec2(origin.x + 4.f, laneY(presentationLaneBegin + static_cast<std::size_t>(i)) + 4.f),
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
			const f32_t y = boxY(animationLane, occurrence.strOccurrenceId);
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
	/* Every occurrence family keeps its authored clock and joins the same selection. */
	for (const auto& box : pattern->LogicOccurrences)
	{
		const f32_t logicBoxY = boxY(logicLane, box.strOccurrenceId);
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, logicBoxY));
		ImGui::InvisibleButton("##LogicBox", ImVec2(width, 22.f));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture = CompositionTimeline::HitBoxGesture(
				ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START ? 1 :
				(gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			Select_TimelineBox({}, box.strOccurrenceId, ImGui::GetIO().KeyCtrl);
			if (ImGui::GetIO().KeyCtrl) m_iTimelineDragMode = -1;
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
		CompositionTimeline::DrawBox(draw, ImVec2(shownX, logicBoxY),
			ImVec2(shownX + shownWidth, logicBoxY + 22.f), TIMELINE_LOGIC_COLOR,
			contains(m_TimelineSelectedOccurrenceIds, box.strOccurrenceId), label.c_str());
		hitBoxes.push_back({ {}, box.strOccurrenceId, ImVec2(shownX, logicBoxY),
			ImVec2(shownX + shownWidth, logicBoxY + 22.f) });
		ImGui::PopID();
	}
	/* Summon boxes: spawn at the left edge, despawn at the right edge. */
	for (const auto& box : pattern->SummonOccurrences)
	{
		const f32_t summonBoxY = boxY(summonLane, box.strOccurrenceId);
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, summonBoxY));
		ImGui::InvisibleButton("##SummonBox", ImVec2(width, 22.f));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture = CompositionTimeline::HitBoxGesture(
				ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START ? 1 :
				(gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			Select_TimelineBox({}, box.strOccurrenceId, ImGui::GetIO().KeyCtrl);
			if (ImGui::GetIO().KeyCtrl) m_iTimelineDragMode = -1;
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
		CompositionTimeline::DrawBox(draw, ImVec2(shownX, summonBoxY),
			ImVec2(shownX + shownWidth, summonBoxY + 22.f), TIMELINE_SUMMON_COLOR,
			contains(m_TimelineSelectedOccurrenceIds, box.strOccurrenceId), label.c_str());
		hitBoxes.push_back({ {}, box.strOccurrenceId, ImVec2(shownX, summonBoxY),
			ImVec2(shownX + shownWidth, summonBoxY + 22.f) });
		ImGui::PopID();
	}
	/* World boxes: the sequence starts at the left edge; the width is the
	   authored shown span. Scene Profile boxes: the profile holds for the box. */
	for (const auto& box : pattern->WorldOccurrences)
	{
		const f32_t worldBoxY = boxY(worldLane, box.strOccurrenceId);
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, worldBoxY));
		ImGui::InvisibleButton("##WorldBox", ImVec2(width, 22.f));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture = CompositionTimeline::HitBoxGesture(
				ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START ? 1 :
				(gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			Select_TimelineBox({}, box.strOccurrenceId, ImGui::GetIO().KeyCtrl);
			if (ImGui::GetIO().KeyCtrl) m_iTimelineDragMode = -1;
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
		CompositionTimeline::DrawBox(draw, ImVec2(shownX, worldBoxY),
			ImVec2(shownX + shownWidth, worldBoxY + 22.f), TIMELINE_WORLD_COLOR,
			contains(m_TimelineSelectedOccurrenceIds, box.strOccurrenceId), label.c_str());
		hitBoxes.push_back({ {}, box.strOccurrenceId, ImVec2(shownX, worldBoxY),
			ImVec2(shownX + shownWidth, worldBoxY + 22.f) });
		ImGui::PopID();
	}
	for (const auto& box : pattern->SceneProfileOccurrences)
	{
		const f32_t sceneBoxY = boxY(sceneLane, box.strOccurrenceId);
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, sceneBoxY));
		ImGui::InvisibleButton("##SceneProfileBox", ImVec2(width, 22.f));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture = CompositionTimeline::HitBoxGesture(
				ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START ? 1 :
				(gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			Select_TimelineBox({}, box.strOccurrenceId, ImGui::GetIO().KeyCtrl);
			if (ImGui::GetIO().KeyCtrl) m_iTimelineDragMode = -1;
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
		CompositionTimeline::DrawBox(draw, ImVec2(shownX, sceneBoxY),
			ImVec2(shownX + shownWidth, sceneBoxY + 22.f), TIMELINE_SCENE_PROFILE_COLOR,
			contains(m_TimelineSelectedOccurrenceIds, box.strOccurrenceId), label.c_str());
		hitBoxes.push_back({ {}, box.strOccurrenceId, ImVec2(shownX, sceneBoxY),
			ImVec2(shownX + shownWidth, sceneBoxY + 22.f) });
		ImGui::PopID();
	}
	for (const auto& box : pattern->PresentationOccurrences)
	{
		const auto* resource = Find_PresentationResource(m_Draft, box.strResourceId);
		if (nullptr == resource) continue;
		const f32_t presentationLaneY = boxY(
			presentationLaneBegin + static_cast<std::size_t>(resource->eKind), box.strOccurrenceId);
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
			Select_TimelineBox({}, box.strOccurrenceId, ImGui::GetIO().KeyCtrl);
			if (ImGui::GetIO().KeyCtrl) m_iTimelineDragMode = -1;
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
		if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
			if (const auto* shot = Find_AuthoringCamera(resource->strAssetId); shot && shot->iBlendOutMs)
			{
				const float endX = shownX + shownWidth;
				draw->AddRectFilled(ImVec2(endX, presentationLaneY + 4.f),
					ImVec2(endX + float(shot->iBlendOutMs) * scale, presentationLaneY + 18.f), IM_COL32(94, 165, 151, 70));
				draw->AddText(ImVec2(endX + 3.f, presentationLaneY + 3.f), IM_COL32(154, 205, 191, 255), "return");
			}
		CompositionTimeline::DrawBox(draw, ImVec2(shownX, presentationLaneY),
			ImVec2(shownX + shownWidth, presentationLaneY + 22.f), IM_COL32(94, 165, 151, 255),
			contains(m_TimelineSelectedOccurrenceIds, box.strOccurrenceId), label.c_str());
		hitBoxes.push_back({ {}, box.strOccurrenceId, ImVec2(shownX, presentationLaneY),
			ImVec2(shownX + shownWidth, presentationLaneY + 22.f) });
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
		std::string focusStage, focusOccurrence;
		bool focusHit = false;
		for (const auto& box : hitBoxes)
		{
			// Enclose the whole time interval so a short strike selection does
			// not also copy persistent encounter Logic or World objects.
			if (marqueeMin.x > box.min.x || marqueeMax.x < box.max.x ||
				marqueeMax.y <= box.min.y || marqueeMin.y >= box.max.y)
				continue;
			draw->AddRect(box.min, box.max, IM_COL32(255, 224, 92, 255), 3.f, 0, 2.f);
			if (released)
			{
				auto& selected = box.occurrenceId.empty() ?
					m_TimelineSelectedStageIds : m_TimelineSelectedOccurrenceIds;
				const auto& id = box.occurrenceId.empty() ? box.stageId : box.occurrenceId;
				if (!contains(selected, id)) selected.push_back(id);
				focusStage = box.stageId;
				focusOccurrence = box.occurrenceId;
				focusHit = true;
			}
		}
		if (released)
		{
			m_bTimelineMarqueeActive = false;
			if (focusHit)
			{
				// Reuse typed Detail focus without replacing the complete marquee batch.
				auto selectedStages = m_TimelineSelectedStageIds;
				auto selectedOccurrences = m_TimelineSelectedOccurrenceIds;
				Select_TimelineBox(focusStage, focusOccurrence, false);
				m_TimelineSelectedStageIds = std::move(selectedStages);
				m_TimelineSelectedOccurrenceIds = std::move(selectedOccurrences);
			}
			else Synchronize_EditorFields();
		}
		else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
			m_bTimelineMarqueeActive = false;
	}
	if (canInteract && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
		!m_bTimelineMarqueeActive && !ImGui::IsAnyItemActive() &&
		(!m_TimelineSelectedStageIds.empty() || !m_TimelineSelectedOccurrenceIds.empty()))
	{
		if (ImGui::IsKeyPressed(ImGuiKey_Delete, false)) deleteRequested = true;
		else if (ImGui::GetIO().KeyCtrl && !ImGui::GetIO().KeyAlt && ImGui::IsKeyPressed(ImGuiKey_D, false))
			duplicateRequested = true;
		else if (!ImGui::GetIO().KeyCtrl && canReorderSelection())
		{
			if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false)) moveStageDirection = -1;
			else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, false)) moveStageDirection = 1;
		}
	}
	ImGui::SetCursorScreenPos(origin);
	ImGui::Dummy(ImVec2(timelineWidth, height + 48.f));
	ImGui::EndChild();
	std::string status;
	if (deleteRequested)
	{
		// No frame-local Pattern, Stage or occurrence pointer is consumed after commit.
		(void)Delete_TimelineSelection(patternId,
			m_TimelineSelectedStageIds, m_TimelineSelectedOccurrenceIds, status);
	}
	else if (duplicateRequested)
		(void)Duplicate_TimelineSelection(patternId,
			m_TimelineSelectedStageIds, m_TimelineSelectedOccurrenceIds, status);
	else if (0 != moveStageDirection)
		(void)Move_SelectedStage(patternId, moveStageDirection, status);
	else if (!editLogicBoxId.empty())
		(void)Set_LogicBoxWindow(patternId, editLogicBoxId, newLogicStartMs, newLogicDurationMs, status);
	else if (!editSummonBoxId.empty())
		(void)Set_SummonBoxWindow(patternId, editSummonBoxId, newSummonStartMs, newSummonDurationMs, status);
	else if (!editWorldBoxId.empty())
	{
		const auto* const edited = Find_WorldBox(*pattern, editWorldBoxId);
		(void)Set_WorldBoxWindow(patternId, editWorldBoxId, newWorldStartMs, newWorldDurationMs,
			nullptr == edited ? 1.f : edited->fPlaybackSpeed, status);
	}
	else if (!editSceneBoxId.empty())
	{
		const auto* const edited = Find_SceneProfileBox(*pattern, editSceneBoxId);
		(void)Set_SceneProfileBoxWindow(patternId, editSceneBoxId, newSceneStartMs, newSceneDurationMs,
			nullptr == edited ? 500u : edited->iBlendMs, status);
	}
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
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Updated Logic window and its linked Collider boxes.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_LogicBoxHold(
	const std::string_view patternId, const std::string_view occurrenceId,
	const std::string_view holdOccurrenceId, std::string& outStatus)
{
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	auto* box = pattern ? Find_LogicBox(*pattern, occurrenceId) : nullptr;
	const auto* logic = box ? Find_Logic(candidate, box->strLogicId) : nullptr;
	if (!logic || logic->strTriggerKind != "ENTER_AREA")
	{ outStatus = m_strStatus = "Select an ENTER_AREA Trigger window to connect its Hold."; return false; }
	box->strHoldLogicOccurrenceId = holdOccurrenceId;
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Updated capture Hold connection. Press Save.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_LogicBox(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	std::string& outStatus)
{
	return Delete_TimelineSelection(patternId, {}, {std::string(occurrenceId)}, outStatus);
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
		outStatus = m_strStatus = "Only a Duration or Collider Trigger window owns result slots.";
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
	Mark_Draft(candidate, *pattern);
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
	// Editing a shared definition may make its users temporarily incomplete.
	// Publish All Patterns evaluates readiness after the source has been saved.
	for (auto& pattern : candidate.Patterns)
	{
		if (!pattern.strLoadError.empty()) continue;
		const bool usesDefinition = std::any_of(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(),
			[&](const auto& box) {
				if (box.strLogicId == logicId) return true;
				for (const auto slot : { KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS,
					KOUKU_SAYDON_OUTCOME_SLOT::FAIL, KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT })
					if (std::find(box.Outcomes(slot).begin(), box.Outcomes(slot).end(), logicId) != box.Outcomes(slot).end())
						return true;
				return false;
			});
		if (usesDefinition)
		{
			for (auto& box : pattern.LogicOccurrences)
			{
				if (box.strLogicId == logicId && found->strTriggerKind != "ENTER_AREA") box.strHoldLogicOccurrenceId.clear();
				if (const auto* hold = Find_LogicBox(pattern, box.strHoldLogicOccurrenceId);
					hold && hold->strLogicId == logicId && found->strJudgementKind != "ATTACHMENT_HOLD") box.strHoldLogicOccurrenceId.clear();
				if (box.strLogicId == logicId && found->strJudgementKind == "ATTACHMENT_HOLD")
				{
					box.OnSuccessLogicIds.clear(); box.OnFailLogicIds.clear(); box.OnTimeoutLogicIds.clear();
					for (auto& collider : pattern.PresentationOccurrences)
						if (collider.strLogicOccurrenceId == box.strOccurrenceId) collider.strLogicOccurrenceId.clear();
				}
			}
			Mark_Draft(candidate, pattern);
		}
	}
	return Commit_Candidate(std::move(candidate),
		m_bSequenceWorkspace ? "Applied the Logic values. Save stores them in the Sequence workspace." :
		"Applied the Logic values. Save stores them; Publish All Patterns updates the F1 tree.",
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
	if (Render_RenameControl(RENAME_TARGET::SUMMON, summonId, summonName)) return;
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

bool_t Client::CKoukuSaydonActionWorkbench::Stage_NewWorldPlacement(
	const std::string_view instanceId, std::optional<KOUKU_SAYDON_WORLD_PLACEMENT>& outPlacement,
	std::string& outStatus) const
{
	outPlacement.reset();
	const auto source = std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(),
		[&](const auto& row) { return row.strInstanceId == instanceId; });
	if (source == m_WorldSequenceResources.end() || !source->bSupportsPlacement) return true;
	if (source->strAnchorKind == "BOSS" || source->strAnchorKind == "PLAYER")
	{ outPlacement = KOUKU_SAYDON_WORLD_PLACEMENT{}; return true; }
	if (!source->bEnabled || !m_WorldPlacementResolver)
	{ outStatus = "A live character placement is required to append this Object."; return false; }
	KOUKU_SAYDON_WORLD_PLACEMENT placement;
	if (!m_WorldPlacementResolver(placement, outStatus)) return false;
	outPlacement = placement;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_WorldBoxPlacement(
	const std::string_view patternId, const std::string_view occurrenceId,
	const KOUKU_SAYDON_WORLD_PLACEMENT& placement, std::string& outStatus)
{
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	if (!pattern || !pattern->strLoadError.empty())
	{ outStatus = m_strStatus = "Select an editable Pattern for this Object placement."; return false; }
	const auto found = std::find_if(pattern->WorldOccurrences.begin(), pattern->WorldOccurrences.end(),
		[&](const auto& row) { return row.strOccurrenceId == occurrenceId; });
	if (found == pattern->WorldOccurrences.end())
	{ outStatus = m_strStatus = "The World occurrence no longer exists."; return false; }
	if (found->Placement && *found->Placement == placement)
	{ outStatus = "World occurrence placement is unchanged."; return true; }
	found->Placement = placement;
	if (!Commit_Candidate(std::move(candidate), "Updated this Object box's Transform. Save keeps this placement.", outStatus)) return false;
	Queue_WorldBoxPreview(patternId, occurrenceId);
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Place_WorldBoxNearCharacter(
	const std::string_view patternId, const std::string_view occurrenceId, std::string& outStatus)
{
	const auto* pattern = Find_Pattern(m_Draft, patternId);
	const auto* box = pattern ? Find_WorldBox(*pattern, occurrenceId) : nullptr;
	const auto* world = box ? Find_World(m_Draft, box->strWorldId) : nullptr;
	if (!world)
	{ outStatus = m_strStatus = "Select an existing World box to place near the character."; return false; }
	std::optional<KOUKU_SAYDON_WORLD_PLACEMENT> placement;
	if (!Stage_NewWorldPlacement(world->strSequenceInstanceId, placement, outStatus))
	{ m_strStatus = outStatus; return false; }
	if (!placement)
	{ outStatus = m_strStatus = "Independent placement requires a WORLD Object resource; existing map bindings retain their saved placement."; return false; }
	if (box->Placement)
	{
		placement->RotationDegrees = box->Placement->RotationDegrees;
		placement->Scale = box->Placement->Scale;
	}
	return Set_WorldBoxPlacement(patternId, occurrenceId, *placement, outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Append_WorldObject(
	const std::string_view objectResourceId, std::string& outStatus)
{
	const auto source = std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(),
		[&](const auto& row) { return row.strObjectResourceId == objectResourceId && row.bDefaultMotion; });
	if (objectResourceId.empty() || source == m_WorldSequenceResources.end() ||
		!source->bEnabled || source->strInstanceId.empty())
	{ outStatus = m_strStatus = "Set and Save this Object's default animation in Object Tool before Append."; return false; }
	return Append_WorldResource(source->strInstanceId, outStatus, true);
}

bool_t Client::CKoukuSaydonActionWorkbench::Append_WorldResource(
	const std::string_view instanceId, std::string& outStatus, const bool_t asObject)
{
	const auto source = std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(),
		[instanceId](const auto& row) { return row.strInstanceId == instanceId; });
	if (source == m_WorldSequenceResources.end() || source->strInstanceId.empty() || !source->bEnabled ||
		(asObject && (!source->bDefaultMotion || source->strObjectResourceId.empty())))
	{ outStatus = m_strStatus = "Select an Object with an enabled saved default animation first."; return false; }
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, m_strSelectedPatternId);
	if (!m_bHasDraft || !pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0u ||
		pattern->iNextWorldOccurrenceOrdinal >= 1000000u)
	{ outStatus = m_strStatus = "Append needs an editable Pattern with an animation lifetime."; return false; }
	auto world = std::find_if(candidate.Worlds.begin(), candidate.Worlds.end(),
		[&](const auto& row) { return row.strSequenceInstanceId == instanceId &&
			(asObject ? row.strObjectResourceId == source->strObjectResourceId : row.strObjectResourceId.empty()); });
	if (world == candidate.Worlds.end())
	{
		if (candidate.iNextWorldOrdinal >= 1000000u)
		{ outStatus = m_strStatus = "World resource stable ID ordinals are exhausted."; return false; }
		KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION value;
		value.strWorldId = "kakulsaydon.g1.world." + std::to_string(candidate.iNextWorldOrdinal++);
		value.strDisplayName = asObject ? source->strObjectDisplayName : source->strDisplayName;
		value.strSequenceInstanceId = source->strInstanceId;
		if (asObject) value.strObjectResourceId = source->strObjectResourceId;
		candidate.Worlds.push_back(std::move(value));
		world = candidate.Worlds.end() - 1;
	}
	KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE box;
	box.strOccurrenceId = pattern->strPatternId + ".world." + std::to_string(pattern->iNextWorldOccurrenceOrdinal++);
	box.strWorldId = world->strWorldId;
	box.iStartMs = (std::min)(m_iCursorMs, Pattern_DurationMs(*pattern));
	box.iDurationMs = asObject ? (std::max)(1u, Pattern_DurationMs(*pattern) - box.iStartMs) :
		(std::max)(1u, source->iDurationMs);
	if (!Stage_NewWorldPlacement(instanceId, box.Placement, outStatus))
	{ m_strStatus = outStatus; return false; }
	const std::string worldId = box.strWorldId;
	const std::string occurrenceId = box.strOccurrenceId;
	pattern->WorldOccurrences.push_back(box);
	if (!Append_WorldCompanionEffect(*pattern, *world, box, outStatus))
	{ m_strStatus = outStatus; return false; }
	if (!asObject) Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), asObject ? "Appended Object with its default animation. Adjust this box's Transform, then Save." :
		"Appended saved World Object state at the cursor. Tune lifetime in Box Detail.", outStatus)) return false;
	m_strSelectedWorldId = worldId;
	m_strSelectedWorldOccurrenceId = occurrenceId;
	m_strSelectedStageId.clear(); m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear(); m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear(); m_strSelectedPresentationOccurrenceId.clear();
	m_TimelineSelectedStageIds.clear(); m_TimelineSelectedOccurrenceIds.clear();
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
	if (asObject && box.Placement) Queue_WorldBoxPreview(m_strSelectedPatternId, occurrenceId);
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
	if (!Stage_NewWorldPlacement(world->strSequenceInstanceId, box.Placement, outStatus))
	{ m_strStatus = outStatus; return false; }
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
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE && patternId.empty())
	{
		auto candidate = m_Draft; auto* bundle = Find_Bundle(candidate, m_strSelectedBundleId);
		if (!bundle || !bundle->strLoadError.empty() || !Find_SceneProfile(candidate, sceneProfileId) || bundle->iNextSceneProfileOccurrenceOrdinal >= 1000000u)
		{ outStatus = m_strStatus = "Select an editable bundle and a valid Scene Profile resource."; return false; }
		KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE row;
		row.strOccurrenceId = bundle->strBundleId + ".sceneprofile." + std::to_string(bundle->iNextSceneProfileOccurrenceOrdinal++);
		row.strSceneProfileId = std::string(sceneProfileId);
		row.iStartMs = (std::min)(startMs, MAX_EDITOR_TIME_MS - 1u);
		row.iDurationMs = (std::clamp)(durationMs, 1u, MAX_EDITOR_TIME_MS - row.iStartMs);
		row.iBlendMs = 500u;
		const auto id = row.strOccurrenceId;
		bundle->SceneProfileOccurrences.push_back(std::move(row)); bundle->strAuthoringStatus = "DRAFT";
		if (!Commit_Candidate(std::move(candidate), "Appended common Scene Profile on the bundle clock.", outStatus)) return false;
		outOccurrenceId = id; return true;
	}
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
			"Appended Scene Profile box. The profile applies instantly at the box start and restores when the box ends. Blend ms is reserved.",
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
	return Append_PresentationCandidate(m_Draft, resourceId, outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Append_PresentationSource(
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& source, std::string& outStatus)
{
	auto candidate = m_Draft;
	const auto existing = std::find_if(candidate.PresentationResources.begin(), candidate.PresentationResources.end(),
		[&](const auto& item) {
			return item.eKind == source.eKind && item.strAssetId == source.strAssetId &&
				item.strResourceKind == source.strResourceKind && item.strElementId == source.strElementId &&
				(item.eKind != KOUKU_SAYDON_PRESENTATION_KIND::LIGHT ||
					item.strDefaultAnchorKind == source.strDefaultAnchorKind);
		});
	std::string resourceId;
	if (existing != candidate.PresentationResources.end()) resourceId = existing->strResourceId;
	else
	{
		if (candidate.iNextPresentationResourceOrdinal >= 1000000u)
		{ outStatus = m_strStatus = "Presentation resource IDs are exhausted."; return false; }
		auto resource = source;
		resourceId = "kakulsaydon.g1.presentation." + std::to_string(candidate.iNextPresentationResourceOrdinal++);
		resource.strResourceId = resourceId;
		candidate.PresentationResources.push_back(std::move(resource));
	}
	// The resource reference and its occurrence enter the draft in one commit.
	// A GROUP retains its native ID; its children and their tuning stay owned by V2.
	if (!Append_PresentationCandidate(std::move(candidate), resourceId, outStatus)) return false;
	m_strSelectedPresentationResourceId = resourceId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Append_PresentationCandidate(
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate, const std::string_view resourceId, std::string& outStatus)
{
	if (!m_bHasDraft)
	{ outStatus = m_strStatus = "Load a composition before appending a presentation resource."; return false; }
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE)
	{
		auto* bundle = Find_Bundle(candidate, m_strSelectedBundleId);
		const auto* resource = Find_PresentationResource(candidate, resourceId);
		if (!bundle || !bundle->strLoadError.empty() || bundle->iNextPresentationOccurrenceOrdinal >= 1000000u || !resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
		{ outStatus = m_strStatus = "A bundle accepts common Camera and Scene Profile lanes. Select a child for actor lanes."; return false; }
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE row;
		row.strOccurrenceId = bundle->strBundleId + ".presentation." + std::to_string(bundle->iNextPresentationOccurrenceOrdinal++);
		row.strResourceId = std::string(resourceId); row.iStartMs = (std::min)(m_iCursorMs, MAX_EDITOR_TIME_MS - 1u);
		row.iDurationMs = (std::clamp)(Camera_DefaultDuration(*resource), 1u, MAX_EDITOR_TIME_MS - row.iStartMs); row.strAnchorKind = "BOSS"; row.bFollowBoss = false;
		bundle->PresentationOccurrences.push_back(row); bundle->strAuthoringStatus = "DRAFT";
		return Commit_Candidate(std::move(candidate), "Appended common Camera.", outStatus);
	}
	auto* pattern = Find_Pattern(candidate, m_strSelectedPatternId);
	const auto* resource = Find_PresentationResource(candidate, resourceId);
	if (nullptr == pattern || !pattern->strLoadError.empty() || nullptr == resource ||
		pattern->iNextPresentationOccurrenceOrdinal >= 1000000u || Pattern_DurationMs(*pattern) == 0u)
	{ outStatus = m_strStatus = "Append requires a valid resource and a Pattern with a lifetime."; return false; }
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE row;
	row.strOccurrenceId = pattern->strPatternId + ".presentation." + std::to_string(pattern->iNextPresentationOccurrenceOrdinal++);
	row.strResourceId = std::string(resourceId);
	row.iStartMs = (std::min)(m_iCursorMs, Pattern_DurationMs(*pattern) - 1u);
	row.iDurationMs = (std::min)(Camera_DefaultDuration(*resource), Pattern_DurationMs(*pattern) - row.iStartMs);
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
	const auto previousWindowId = found->strLogicOccurrenceId;
	*found = value;
	if (!previousWindowId.empty() && value.strLogicOccurrenceId.empty() &&
		std::none_of(pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(),
			[&](const auto& row) { return row.strLogicOccurrenceId == previousWindowId; }))
	{
		const auto* previous = Find_LogicBox(*pattern, previousWindowId);
		const auto* logic = previous ? Find_Logic(candidate, previous->strLogicId) : nullptr;
		if (logic && logic->strTriggerKind == "ENTER_AREA")
			std::erase_if(pattern->LogicOccurrences, [&](const auto& row) { return row.strOccurrenceId == previousWindowId; });
	}
	const auto* resource = Find_PresentationResource(candidate, found->strResourceId);
	if (resource && !Valid_GameplayColliderScale(*resource, *found))
	{
		outStatus = m_strStatus = "Apply rejected: Collider scale must be finite and positive. Adjust Scale / size; pending edits and previous draft are preserved.";
		return false;
	}
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
	const auto* logic = Find_Logic(m_Draft, logicId);
	if (!logic) { outStatus = m_strStatus = "Selected Logic definition is unavailable; previous Collider preserved."; return false; }
	return Set_ColliderLogicValues(patternId, occurrence, *logic, outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_ColliderLogicValues(
	const std::string_view patternId, const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence,
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& values, std::string& outStatus)
{
	const auto reject = [&](const char* reason) { outStatus = m_strStatus = reason; return false; };
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	const auto definition = std::find_if(candidate.Logics.begin(), candidate.Logics.end(),
		[&](const auto& logic) { return logic.strLogicId == values.strLogicId; });
	const auto* resource = Find_PresentationResource(candidate, occurrence.strResourceId);
	if (!m_bHasDraft || !pattern || !pattern->strLoadError.empty() || definition == candidate.Logics.end() ||
		!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
		return reject("Select an existing Collider and Logic definition; previous draft preserved.");
	const auto collider = std::find_if(pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(),
		[&](const auto& row) { return row.strOccurrenceId == occurrence.strOccurrenceId && row.strResourceId == occurrence.strResourceId; });
	if (collider == pattern->PresentationOccurrences.end())
		return reject("Collider identity changed; previous draft preserved.");
	auto updated = values;
	updated.strLogicId = definition->strLogicId;
	updated.strDisplayName = definition->strDisplayName;
	updated.strLogicType = definition->strLogicType;
	if (!Kouku_LogicAcceptsColliders(updated))
		return reject("Collider connection needs a Duration or ENTER_AREA/OBJECT_CONTACT Trigger. Complete the Trigger settings before Apply; previous draft preserved.");
	*definition = std::move(updated);

	// Explicit Shared selection owns its clock. The current link or a single
	// unconnected window adopts this Collider's clock without replacing its ID.
	bool_t adoptColliderClock = false;
	auto window = pattern->LogicOccurrences.end();
	if (!occurrence.strLogicOccurrenceId.empty())
	{
		window = std::find_if(pattern->LogicOccurrences.begin(), pattern->LogicOccurrences.end(),
			[&](const auto& row) { return row.strOccurrenceId == occurrence.strLogicOccurrenceId; });
		if (window == pattern->LogicOccurrences.end()) return reject("Selected shared Logic window is unavailable; previous draft preserved.");
		if (window->strLogicId != definition->strLogicId) window = pattern->LogicOccurrences.end();
	}
	if (window == pattern->LogicOccurrences.end())
		for (auto row = pattern->LogicOccurrences.begin(); row != pattern->LogicOccurrences.end(); ++row)
			if (row->strLogicId == definition->strLogicId && row->iStartMs == occurrence.iStartMs && row->iDurationMs == occurrence.iDurationMs)
			{
				if (window != pattern->LogicOccurrences.end()) return reject("Several Logic windows match this interval. Select the intended Shared Logic window; previous draft preserved.");
				window = row;
			}
	if (window == pattern->LogicOccurrences.end())
	{
		for (auto row = pattern->LogicOccurrences.begin(); row != pattern->LogicOccurrences.end(); ++row)
			if (row->strLogicId == definition->strLogicId &&
				std::none_of(pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(),
					[&](const auto& other) { return other.strLogicOccurrenceId == row->strOccurrenceId; }))
			{
				if (window != pattern->LogicOccurrences.end()) return reject("Several unconnected Logic windows use this definition. Select the intended Shared Logic window; previous draft preserved.");
				window = row;
				adoptColliderClock = true;
			}
	}
	if (window == pattern->LogicOccurrences.end())
	{
		if (pattern->iNextLogicOccurrenceOrdinal >= 1000000u) return reject("Logic window IDs are exhausted; previous draft preserved.");
		KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE created;
		created.strOccurrenceId = std::string(patternId) + ".logic." + std::to_string(pattern->iNextLogicOccurrenceOrdinal++);
		created.strLogicId = definition->strLogicId;
		created.iStartMs = occurrence.iStartMs; created.iDurationMs = occurrence.iDurationMs;
		pattern->LogicOccurrences.push_back(std::move(created));
		window = std::prev(pattern->LogicOccurrences.end());
	}
	if (adoptColliderClock || collider->strLogicOccurrenceId == window->strOccurrenceId)
	{
		window->iStartMs = occurrence.iStartMs; window->iDurationMs = occurrence.iDurationMs;
		for (auto& other : pattern->PresentationOccurrences)
			if (other.strLogicOccurrenceId == window->strOccurrenceId)
			{ other.iStartMs = occurrence.iStartMs; other.iDurationMs = occurrence.iDurationMs; }
	}
	*collider = occurrence;
	collider->strLogicOccurrenceId = window->strOccurrenceId;
	collider->iStartMs = window->iStartMs; collider->iDurationMs = window->iDurationMs;
	if (!Valid_GameplayColliderScale(*resource, *collider))
		return reject("Apply rejected: Collider scale must be finite and positive. Adjust Scale / size; pending edits and previous draft are preserved.");
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Applied Collider values, Logic definition and window link together. Save keeps the connection.", outStatus)) return false;
	m_strColliderExecutionEditId.clear();
	m_bColliderDamageDirty = false;
	return true;
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
	const KOUKU_COLLIDER_DAMAGE_SETTINGS& settings, std::string& outStatus)
{
	const auto percent = static_cast<std::uint32_t>(settings.iPercent);
	if (settings.iPercent < 1 || settings.iPercent > 100 ||
		(settings.bRearmOnExit && settings.bRepeatAfterKnockback) ||
		(settings.bRepeatAfterKnockback && settings.fPushRangeM <= 0.0) ||
		!std::isfinite(settings.fPushRangeM) || settings.fPushRangeM < 0.0 || settings.fPushRangeM > 20.0 ||
		settings.iPushMs > MAX_EDITOR_TIME_MS || ((settings.fPushRangeM == 0.0) != (settings.iPushMs == 0u)) ||
		(settings.strPushDirection != "AWAY_FROM_BOSS" && settings.strPushDirection != "BOSS_FORWARD") ||
		(settings.strPushDirection == "BOSS_FORWARD" && settings.fPushRangeM == 0.0))
	{ outStatus = m_strStatus = "Damage settings require bounded paired push values and one repeat policy."; return false; }
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
		auto logic = std::find_if(candidate.Logics.begin(), candidate.Logics.end(), [&](const auto& row) {
			KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION desired;
			desired.strLogicId = row.strLogicId; desired.strDisplayName = row.strDisplayName;
			desired.strLogicType = "TRIGGER"; desired.strTriggerKind = "ENTER_AREA";
			desired.bRearmOnExit = settings.bRearmOnExit; desired.bRepeatAfterKnockback = settings.bRepeatAfterKnockback;
			return row == desired; });
		if (logic == candidate.Logics.end())
		{
			if (candidate.iNextLogicOrdinal >= 1000000u)
			{ outStatus = m_strStatus = "Logic IDs are exhausted."; return false; }
			KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION created;
			created.strLogicId = "kakulsaydon.g1.logic." + std::to_string(candidate.iNextLogicOrdinal++);
			created.strDisplayName = resource->strDisplayName + " Enter Area";
			created.strLogicType = "TRIGGER"; created.strTriggerKind = "ENTER_AREA";
			created.bRearmOnExit = settings.bRearmOnExit; created.bRepeatAfterKnockback = settings.bRepeatAfterKnockback;
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
	// Changing this box never mutates a shared Trigger definition.
	const auto* currentTrigger = Find_Logic(candidate, linked->strLogicId);
	if (!currentTrigger) { outStatus = m_strStatus = "Damage Trigger is missing."; return false; }
	if (currentTrigger->bRearmOnExit != settings.bRearmOnExit || currentTrigger->bRepeatAfterKnockback != settings.bRepeatAfterKnockback)
	{
		if (candidate.iNextLogicOrdinal >= 1000000u) { outStatus = m_strStatus = "Logic IDs are exhausted."; return false; }
		auto changed = *currentTrigger;
		changed.strLogicId = "kakulsaydon.g1.logic." + std::to_string(candidate.iNextLogicOrdinal++);
		changed.strDisplayName = resource->strDisplayName + " Damage contact";
		changed.bRearmOnExit = settings.bRearmOnExit;
		changed.bRepeatAfterKnockback = settings.bRepeatAfterKnockback;
		linked->strLogicId = changed.strLogicId;
		candidate.Logics.push_back(std::move(changed));
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
		return row.strLogicType == "RESULT" && row.strOutcomeKind == "MAX_HP_PERCENT_DAMAGE" && row.iPercent == percent &&
			row.fPushRangeM == settings.fPushRangeM && row.iPushMs == settings.iPushMs && row.strPushDirection == settings.strPushDirection; });
	if (damage == candidate.Logics.end())
	{
		if (candidate.iNextLogicOrdinal >= 1000000u)
		{ outStatus = m_strStatus = "Logic IDs are exhausted."; return false; }
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION result;
		result.strLogicId = "kakulsaydon.g1.logic." + std::to_string(candidate.iNextLogicOrdinal++);
		result.strDisplayName = resource->strDisplayName + " Damage " + std::to_string(percent) + "% max HP";
		result.strLogicType = "RESULT"; result.strOutcomeKind = "MAX_HP_PERCENT_DAMAGE"; result.iPercent = percent;
		result.fPushRangeM = settings.fPushRangeM; result.iPushMs = settings.iPushMs; result.strPushDirection = settings.strPushDirection;
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
	if (!Valid_GameplayColliderScale(*resource, *collider))
	{
		outStatus = m_strStatus = "Apply rejected: Collider scale must be finite and positive. Adjust Scale / size; pending edits and previous draft are preserved.";
		return false;
	}
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

void Client::CKoukuSaydonActionWorkbench::Queue_WorldBoxPreview(
	const std::string_view patternId, const std::string_view occurrenceId)
{
	const auto* pattern = Find_Pattern(m_Draft, patternId);
	const auto* box = pattern ? Find_WorldBox(*pattern, occurrenceId) : nullptr;
	const auto* world = box ? Find_World(m_Draft, box->strWorldId) : nullptr;
	if (!world || !box->Placement) return;
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE resource;
	resource.eKind = KOUKU_SAYDON_PRESENTATION_KIND::WORLD;
	resource.strResourceId = world->strWorldId;
	resource.strAssetId = world->strSequenceInstanceId;
	resource.strDisplayName = world->strDisplayName;
	resource.iDurationMs = MAX_EDITOR_TIME_MS;
	Queue_PresentationPreview(resource);
	m_PendingPresentationPreviewRequest.strEditedOccurrenceId = std::string(occurrenceId);
	for (auto placed : pattern->WorldOccurrences)
	{
		if (!placed.Placement) continue;
		placed.iStartMs = 0u;
		placed.iDurationMs = MAX_EDITOR_TIME_MS;
		m_PendingPresentationPreviewRequest.WorldBoxes.push_back(std::move(placed));
	}
}

void Client::CKoukuSaydonActionWorkbench::Queue_PresentationPreview(
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE* occurrence)
{
	if (nullptr != occurrence && resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER &&
		(!occurrence->strBone.empty() || occurrence->strBoneTarget == "WEAPON"))
	{
		std::string status;
		(void)Request_ColliderBoxPreview(m_strSelectedPatternId, *occurrence, status);
		return;
	}
	if (nullptr != occurrence && resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
	{
		std::string status;
		(void)Request_PresentationGeometryPreview(m_strSelectedPatternId, *occurrence, status);
		return;
	}
	m_bBundlePreviewRequestPending = false;
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
	m_PendingPresentationPreviewRequest = {};
	m_PendingPresentationPreviewRequest.Resource = resource;
	m_PendingPresentationPreviewRequest.Occurrence = nullptr != occurrence ? *occurrence :
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE{};
	if (nullptr == occurrence)
	{
		m_PendingPresentationPreviewRequest.Occurrence.strResourceId = resource.strResourceId;
		m_PendingPresentationPreviewRequest.Occurrence.iDurationMs = Camera_DefaultDuration(resource);
		if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT)
		{
			m_PendingPresentationPreviewRequest.Occurrence.strAnchorKind = resource.strDefaultAnchorKind;
			m_PendingPresentationPreviewRequest.Occurrence.bFollowBoss = resource.strDefaultAnchorKind != "MAP";
		}
	}
	m_bPresentationPreviewRequestPending = true;
	m_strStatus = std::string(Presentation_Label(resource.eKind)) + " preview requested: " + resource.strDisplayName;
}

void Client::CKoukuSaydonActionWorkbench::Render_CameraAuthoring(const std::string_view shotId)
{
	auto* level = CLevel_KakulSaydonArena::Get_Active();
	const auto* selected = Find_AuthoringCamera(shotId);
	if (!level || !selected) { ImGui::TextDisabled("Enter KoukuSaydon to edit this Area Camera shot."); return; }
	auto shot = *selected;
	ImGui::PushID(shot.strShotId.c_str());
	ImGui::SeparatorText("Camera Shot / Area source");
	ImGui::TextDisabled("%s | %s", shot.strShotId.c_str(), shot.bPatternOnly ? "PATTERN_ONLY" : "AUTO (existing Area trigger)");
	char name[129]{}; (void)Copy_Text(name, std::size(name), shot.strDisplayName);
	bool changed = ImGui::InputText("Camera name", name, std::size(name));
	if (changed) shot.strDisplayName = name;
	int entry = static_cast<int>(shot.iBlendInMs), hold = static_cast<int>(shot.iDefaultHoldMs), exit = static_cast<int>(shot.iBlendOutMs);
	if (ImGui::InputInt("Blend in ms", &entry)) { shot.iBlendInMs = std::clamp(entry, 0, 10000); changed = true; }
	if (ImGui::InputInt("Default hold ms", &hold)) { shot.iDefaultHoldMs = std::clamp(hold, 0, 600000); changed = true; }
	if (ImGui::InputInt("Return ms", &exit)) { shot.iBlendOutMs = std::clamp(exit, 0, 10000); changed = true; }
	int easing = shot.eTransitionEasing == VALTAN_CINEMATIC_CAMERA_EASING::LINEAR ? 0 : 1;
	if (ImGui::Combo("Transition", &easing, "LINEAR\0SMOOTHSTEP\0"))
	{ shot.eTransitionEasing = easing == 0 ? VALTAN_CINEMATIC_CAMERA_EASING::LINEAR : VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP; changed = true; }
	int anchor = shot.followsPlayer ? 1 : 0;
	if (ImGui::Combo("Shot anchor", &anchor, "WORLD\0PLAYER\0"))
	{
		shot.followsPlayer = anchor == 1;
		if (shot.followsPlayer) { shot.vFollowEyeOffset = shot.vEye; shot.vFollowLookAtOffset = shot.vLookAt; }
		changed = true;
	}
	if (changed) (void)level->Update_CameraShot(shot, m_strStatus);
	ImGui::Text("Eye (%.3f, %.3f, %.3f)", shot.vEye.x, shot.vEye.y, shot.vEye.z);
	ImGui::Text("Look at (%.3f, %.3f, %.3f), FOV %.2f", shot.vLookAt.x, shot.vLookAt.y, shot.vLookAt.z, shot.fFovYDegrees);
	if (shot.hasCameraTrack) ImGui::TextWrapped("This shot has a camera track. Set Camera Pos explicitly replaces that track with the captured static pose.");
	if (ImGui::Button("Set Camera Pos / Capture view"))
		(void)level->Capture_CameraShot(shot.strShotId, m_strStatus);
	ImGui::SameLine();
	if (ImGui::Button("Save Camera"))
	{
		if (level->Save_CameraShots(m_strStatus)) m_bPresentationResourceRefreshRequested = true;
	}
	ImGui::TextWrapped("Set Camera Pos captures eye, lookAt and FOV together. PLAYER captures offsets from the local player. Save writes the Area source immediately; Preview needs no publish.");
	ImGui::TextWrapped("Append uses blend-in + default hold. The box end begins the separate return tail. Use F6 Follow before Play to keep gameplay movement and return to the moving player.");
	ImGui::PopID();
}

void Client::CKoukuSaydonActionWorkbench::Render_PresentationResources(const KOUKU_SAYDON_PRESENTATION_KIND kind)
{
	ImGui::PushID(static_cast<int>(kind));
	ImGui::SeparatorText(Presentation_Label(kind));
	if (ImGui::Button("Refresh Resources")) m_bPresentationResourceRefreshRequested = true;
	ImGui::TextWrapped("%s", m_strPresentationResourceStatus.c_str());
	std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> sources;
	const bool lightFamily = kind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT;
	const bool effectFamily = kind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT;
	const char* lightAnchors[] = { "MAP", "PLAYER", "BOSS" };
	if (lightFamily) ImGui::Combo("Category##LightResources", &m_iLightResourceCategory, "Map\0Character\0Boss\0");
	if (effectFamily)
	{
		if (ImGui::RadioButton("V2", m_iEffectResourceVersion == 0)) m_iEffectResourceVersion = 0;
		ImGui::SameLine();
		if (ImGui::RadioButton("V1", m_iEffectResourceVersion == 1)) m_iEffectResourceVersion = 1;
		ImGui::InputTextWithHint("##EffectSourceSearch", "Search saved Effects", m_PresentationResourceSearch, std::size(m_PresentationResourceSearch));
	}
	const auto versionMatches = [&](const auto& item) {
		return !effectFamily || (m_iEffectResourceVersion == 0 ? item.strResourceKind == "GROUP" :
			(item.strResourceKind == "V1_EFFECT" || item.strResourceKind == "V1_ELEMENT"));
	};
	for (const auto& item : m_PresentationResourceInventory)
		if (item.eKind == kind && versionMatches(item) &&
			(!lightFamily || item.strDefaultAnchorKind == lightAnchors[m_iLightResourceCategory]) &&
			(!effectFamily || ContainsInsensitive(item.strDisplayName, m_PresentationResourceSearch) ||
				ContainsInsensitive(item.strAssetId, m_PresentationResourceSearch) || ContainsInsensitive(item.strElementId, m_PresentationResourceSearch))) sources.push_back(item);
	if (kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
	{
		auto* level = CLevel_KakulSaydonArena::Get_Active();
		ImGui::InputText("New Camera name", m_NewPresentationName, std::size(m_NewPresentationName));
		ImGui::BeginDisabled(!level || !m_NewPresentationName[0]);
		if (ImGui::Button("Create Camera"))
		{
			std::string id;
			if (level->Create_CameraShot(m_NewPresentationName, id, m_strStatus))
			{
				m_strSelectedPresentationSourceId = std::to_string(static_cast<int>(kind)) + "::" + id;
				m_bPresentationResourceRefreshRequested = true;
			}
		}
		ImGui::EndDisabled();
		if (level && level->Ensure_CameraShotAuthoring(m_strPresentationResourceStatus))
		{
			sources.clear();
			for (const auto& shot : level->Get_CameraShots())
			{
				KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE source;
				source.eKind = kind; source.strResourceKind.clear();
				source.strAssetId = shot.strShotId; source.strDisplayName = shot.strDisplayName;
				source.iDurationMs = shot.iBlendInMs + shot.iDefaultHoldMs;
				sources.push_back(std::move(source));
			}
		}
	}
	if (kind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
	{
		for (int i = 0; i < 6; ++i)
		{
			KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE preset;
			preset.eKind = kind;
			preset.strResourceId = "preset.collider." + std::to_string(i);
			preset.strDisplayName = i == 0 ? "Rectangle" : i == 1 ? "Semicircle" : i == 2 ? "Sector" : i == 3 ? "Roulette Card Region" : i == 4 ? "Circle" : "Reverse Sector";
			preset.strShape = i == 0 ? "BOX" : i == 4 ? "CIRCLE" : i == 5 ? "REVERSE_SECTOR" : "SECTOR";
			preset.fHalfAngleDegrees = i == 1 ? 90.0 : i == 3 ? 22.5 : 45.0;
			preset.strColliderKind = i == 3 ? "ROULETTE_CARD_REGION" : "GEOMETRY";
			preset.strResourceKind.clear();
			sources.push_back(preset);
		}
		ImGui::TextDisabled("Box Detail supports a general region or direct damage settings. Advanced Logic remains available.");
	}
	const auto sourceId = [](const auto& item) {
		return std::to_string(static_cast<int>(item.eKind)) + ":" + item.strResourceKind + ":" +
			(item.strAssetId.empty() ? item.strResourceId : item.strAssetId) +
			(item.strElementId.empty() ? "" : ":" + item.strElementId);
	};
	const auto selectSource = [&](const auto& item) {
		m_strSelectedPresentationSourceId = sourceId(item);
		(void)Copy_Text(m_NewPresentationName, std::size(m_NewPresentationName), item.strDisplayName);
	};
	const auto drawSource = [&](const auto& item) {
		const std::string id = sourceId(item);
		const std::string label = (kind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ? item.strResourceKind + "  " : "") +
			item.strDisplayName + "##" + id;
		if (ImGui::Selectable(label.c_str(), m_strSelectedPresentationSourceId == id))
			selectSource(item);
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s%s%s | native %u ms", item.strAssetId.c_str(),
			item.strElementId.empty() ? "" : " | ", item.strElementId.c_str(), item.iDurationMs);
	};
	ImGui::BeginChild("##PresentationSources", ImVec2(0.f, effectFamily ? 360.f : 180.f), ImGuiChildFlags_Borders);
	for (auto& item : sources)
	{
		if (!effectFamily || item.strResourceKind != "V1_EFFECT")
		{ drawSource(item); continue; }
		ImGui::SetNextItemOpen(m_strExpandedV1EffectId == item.strAssetId, ImGuiCond_Always);
		const auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
			(m_strSelectedPresentationSourceId == sourceId(item) ? ImGuiTreeNodeFlags_Selected : 0);
		const bool open = ImGui::TreeNodeEx((item.strDisplayName + "##" + sourceId(item)).c_str(), flags);
		const bool select = ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen();
		if (select) selectSource(item);
		if ((open || select) && m_strExpandedV1EffectId != item.strAssetId)
		{
			m_strExpandedV1EffectId = item.strAssetId;
			m_V1ElementResources.clear();
			m_strV1ElementResourceStatus.clear();
			if (const auto document = CEffectCatalog::Find(item.strAssetId))
			{
				std::uint32_t durationMs = 1u;
				for (const auto& element : document->Elements)
				{
					auto child = item;
					child.strResourceKind = "V1_ELEMENT";
					child.strElementId = element.strElementId;
					child.strDisplayName = element.strDisplayName.empty() ? element.strElementId : element.strDisplayName;
					const auto& timing = element.Detail.Timing;
					child.iDurationMs = static_cast<std::uint32_t>(std::clamp(
						std::ceil(1000.0 * (timing.fStartDelaySeconds + timing.fLifeTimeSeconds + timing.fAfterImageSeconds)),
						1.0, static_cast<double>(MAX_EDITOR_TIME_MS)));
					durationMs = (std::max)(durationMs, child.iDurationMs);
					m_V1ElementResources.push_back(std::move(child));
				}
				for (const auto& cue : document->ModelCues)
					durationMs = (std::max)(durationMs, static_cast<std::uint32_t>(std::clamp(
						std::ceil(1000.0 * (cue.fStartDelaySeconds + cue.fDurationSeconds)), 1.0, static_cast<double>(MAX_EDITOR_TIME_MS))));
				item.iDurationMs = durationMs;
				for (auto& row : m_PresentationResourceInventory)
					if (row.strAssetId == item.strAssetId && row.strResourceKind == "V1_EFFECT") row.iDurationMs = durationMs;
			}
			else m_strV1ElementResourceStatus = "Effect unavailable: " + CEffectCatalog::Get_Status();
		}
		if (!open)
		{
			if (!select && ImGui::IsItemToggledOpen() && m_strExpandedV1EffectId == item.strAssetId)
				m_strExpandedV1EffectId.clear();
			continue;
		}
		if (m_strExpandedV1EffectId == item.strAssetId)
		{
			if (!m_strV1ElementResourceStatus.empty()) ImGui::TextWrapped("%s", m_strV1ElementResourceStatus.c_str());
			for (const auto& child : m_V1ElementResources) drawSource(child);
		}
		ImGui::TreePop();
	}
	ImGui::EndChild();
	if (effectFamily && m_iEffectResourceVersion == 1)
		sources.insert(sources.end(), m_V1ElementResources.begin(), m_V1ElementResources.end());
	const auto source = std::find_if(sources.begin(), sources.end(), [&](const auto& item) {
		return m_strSelectedPresentationSourceId == sourceId(item); });
	ImGui::InputText("Display name", m_NewPresentationName, std::size(m_NewPresentationName));
	ImGui::BeginDisabled(source == sources.end());
	if (ImGui::Button("Preview Source") && source != sources.end()) Queue_PresentationPreview(*source);
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_bHasDraft || m_NewPresentationName[0] == '\0');
	if (ImGui::Button(kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA ? "Register Camera Resource" : "Create") && source != sources.end())
	{
		std::string status;
		(void)Create_PresentationResource(*source, m_NewPresentationName, status);
	}
	ImGui::EndDisabled();
	ImGui::EndDisabled();
	if (kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA && source != sources.end()) Render_CameraAuthoring(source->strAssetId);
	if (lightFamily || kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA || kind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
	{
		const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
		const bool cameraBundle = kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA && m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE;
		ImGui::BeginDisabled(!m_bHasDraft || source == sources.end() || (!cameraBundle &&
			(m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE || !pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0u)));
		const char* appendLabel = lightFamily ? "Append selected Light" :
			kind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ? "Append Effect at Cursor" : "Append Camera at Cursor";
		if (ImGui::Button(appendLabel) && source != sources.end())
		{
			std::string status;
			(void)Append_PresentationSource(*source, status);
		}
		ImGui::EndDisabled();
		if (kind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
			ImGui::TextWrapped(m_iEffectResourceVersion == 0 ?
				"GROUP appends its saved children together. Select a child Pattern with animation timing." :
				"V1 Effect appends the complete saved effect; V1 Element appends the selected stable element. Source tuning is preserved.");
		if (lightFamily) ImGui::TextDisabled("Light shape and RGB are saved in Rendering Workbench. Boxes own timing and anchor.");
	}
	ImGui::SeparatorText("Created Resources");
	ImGui::BeginChild("##CreatedPresentationResources", ImVec2(0.f, effectFamily ? 110.f : 140.f), ImGuiChildFlags_Borders);
	for (const auto& item : m_Draft.PresentationResources)
	{
		if (item.eKind != kind || !versionMatches(item) || (lightFamily && item.strDefaultAnchorKind != lightAnchors[m_iLightResourceCategory])) continue;
		const auto label = item.strDisplayName + "##" + item.strResourceId;
		if (ImGui::Selectable(label.c_str(), m_strSelectedPresentationResourceId == item.strResourceId))
			m_strSelectedPresentationResourceId = item.strResourceId;
	}
	ImGui::EndChild();
	const auto* selected = Find_PresentationResource(m_Draft, m_strSelectedPresentationResourceId);
	if (nullptr != selected && selected->eKind == kind && versionMatches(*selected))
	{
		const auto resource = *selected;
		if (Render_RenameControl(RENAME_TARGET::PRESENTATION, resource.strResourceId, resource.strDisplayName))
		{ ImGui::PopID(); return; }
		if (kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA && source == sources.end()) Render_CameraAuthoring(resource.strAssetId);
		if (ImGui::Button("Preview")) Queue_PresentationPreview(resource);
		ImGui::SameLine();
		const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
		const auto* bundle = m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE ? Find_Bundle(m_Draft, m_strSelectedBundleId) : nullptr;
		const bool appendable = (pattern && pattern->strLoadError.empty() && Pattern_DurationMs(*pattern) > 0u) ||
			(kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA && bundle && bundle->strLoadError.empty());
		ImGui::BeginDisabled(!appendable);
		if (ImGui::Button("Append at Cursor")) { std::string status; (void)Append_PresentationBox(resource.strResourceId, status); }
		ImGui::EndDisabled();
	}
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::PopID();
}

void Client::CKoukuSaydonActionWorkbench::Render_PresentationAnchor(
	const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& edit, const bool_t effect)
{
	if (ImGui::BeginCombo("Anchor", edit.strAnchorKind.c_str()))
	{
		for (const char* kind : { "BOSS", "WORLD" })
			if (ImGui::Selectable(kind, edit.strAnchorKind == kind))
			{
				edit.strAnchorKind = kind;
				edit.strWorldOccurrenceId.clear();
				if (edit.strAnchorKind == "BOSS") edit.strWorldId.clear();
				else { edit.strBone.clear(); edit.strBoneTarget = "BODY"; }
			}
		ImGui::EndCombo();
	}
	if (edit.strAnchorKind == "WORLD")
	{
		if (ImGui::BeginCombo("World anchor", edit.strWorldId.empty() ? "(world position)" : edit.strWorldId.c_str()))
		{
			if (ImGui::Selectable("(world position)", edit.strWorldId.empty()))
			{ edit.strWorldId.clear(); edit.strWorldOccurrenceId.clear(); }
			for (const auto& world : m_Draft.Worlds)
				if (ImGui::Selectable((world.strDisplayName + "##" + world.strWorldId).c_str(), edit.strWorldId == world.strWorldId))
				{ edit.strWorldId = world.strWorldId; edit.strWorldOccurrenceId.clear(); }
			ImGui::EndCombo();
		}
		if (!edit.strWorldId.empty() && ImGui::BeginCombo("World box##Anchor", edit.strWorldOccurrenceId.empty() ? "(single matching box)" : edit.strWorldOccurrenceId.c_str()))
		{
			if (ImGui::Selectable("(single matching box)", edit.strWorldOccurrenceId.empty())) edit.strWorldOccurrenceId.clear();
			for (const auto& worldBox : pattern.WorldOccurrences)
				if (worldBox.strWorldId == edit.strWorldId && ImGui::Selectable(worldBox.strOccurrenceId.c_str(), edit.strWorldOccurrenceId == worldBox.strOccurrenceId))
					edit.strWorldOccurrenceId = worldBox.strOccurrenceId;
			ImGui::EndCombo();
		}
	}
	if (edit.strAnchorKind == "BOSS")
	{
		ANIMATION_MODEL_TARGET_VIEW bodyView, weaponView;
		const bool matchingTarget = CAnimationTargetService::Resolve_AssetName() == pattern.strActorProfileId;
		const bool hasBody = matchingTarget && CAnimationTargetService::Resolve_ModelTarget(ANIMATION_BONE_TARGET::BODY, bodyView);
		const bool hasWeapon = matchingTarget && CAnimationTargetService::Resolve_ModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
		ImGui::BeginDisabled(!hasBody);
		if (ImGui::BeginCombo("Bone target##PresentationAnchor", edit.strBoneTarget.c_str()))
		{
			if (ImGui::Selectable("BODY", edit.strBoneTarget == "BODY"))
			{ edit.strBoneTarget = "BODY"; edit.strBone.clear(); }
			ImGui::BeginDisabled(!hasWeapon);
			if (ImGui::Selectable("WEAPON", edit.strBoneTarget == "WEAPON"))
			{ edit.strBoneTarget = "WEAPON"; edit.strBone.clear(); edit.bFollowBoss = true; }
			ImGui::EndDisabled();
			ImGui::EndCombo();
		}
		const auto& targetView = edit.strBoneTarget == "WEAPON" ? weaponView : bodyView;
		const auto bones = targetView.Model ? targetView.Model->Get_BoneNames() : std::vector<std::string>{};
		if (ImGui::BeginCombo("Bone##PresentationAnchor", edit.strBone.empty() ? "(pivot)" : edit.strBone.c_str()))
		{
			if (edit.strBoneTarget == "BODY" && ImGui::Selectable("(pivot)", edit.strBone.empty())) edit.strBone.clear();
			for (const auto& bone : bones)
				if (ImGui::Selectable(bone.c_str(), edit.strBone == bone))
				{ edit.strBone = bone; edit.bFollowBoss = true; }
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();
		if (!hasBody) ImGui::TextDisabled("Preview this Pattern's model to choose its actual bones.");
		else if (edit.strBoneTarget == "WEAPON" && !hasWeapon) ImGui::TextDisabled("This preview model has no registered weapon.");
		else if (!edit.strBone.empty() && std::find(bones.begin(), bones.end(), edit.strBone) == bones.end())
			ImGui::TextDisabled("The saved bone is unavailable on this target.");
		ImGui::TextWrapped("BODY (pivot) follows the boss body. Named bones follow their position; rotation uses boss facing and this box's rotation.");
	}
	if (effect && ImGui::BeginCombo("Copy Collider anchor", "(choose Collider box)"))
	{
		for (const auto& collider : pattern.PresentationOccurrences)
		{
			const auto* resource = Find_PresentationResource(m_Draft, collider.strResourceId);
			if (!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER) continue;
			if (ImGui::Selectable((resource->strDisplayName + " | " + collider.strOccurrenceId).c_str()))
			{
				edit.strAnchorKind = collider.strAnchorKind;
				edit.strBoneTarget = collider.strBoneTarget;
				edit.strBone = collider.strBone;
				edit.strWorldId = collider.strWorldId;
				edit.strWorldOccurrenceId = collider.strWorldOccurrenceId;
				edit.bFollowBoss = collider.bFollowBoss;
				edit.PositionOffset = collider.PositionOffset;
				edit.RotationDegrees = collider.RotationDegrees;
			}
		}
		ImGui::EndCombo();
	}
	if (effect) ImGui::TextDisabled("Copy takes anchor, position offset and rotation. Apply and Save keep it.");
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
	const bool cameraBox = definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA;
	if (cameraBox) Render_CameraAuthoring(definition.strAssetId);
	if (cameraBox)
		if (const auto* shot = Find_AuthoringCamera(definition.strAssetId))
			ImGui::Text("Entry %u ms | hold %u ms | return ends %u ms", shot->iBlendInMs,
				box->iDurationMs > shot->iBlendInMs ? box->iDurationMs - shot->iBlendInMs : 0u,
				box->iStartMs + box->iDurationMs + shot->iBlendOutMs);
	ImGui::TextWrapped("%s", definition.strDisplayName.c_str());
	ImGui::TextDisabled("%s", occurrenceId.c_str());
	auto& edit = m_PresentationBoxEdit;
	if (edit.strOccurrenceId != occurrenceId) edit = *box;
	int start = static_cast<int>(edit.iStartMs), duration = static_cast<int>(edit.iDurationMs);
	if (ImGui::InputInt("Start ms##PresentationBox", &start)) edit.iStartMs = static_cast<std::uint32_t>((std::max)(0, start));
	if (ImGui::InputInt(cameraBox ? "Entry + hold ms##PresentationBox" : "Lifetime ms##PresentationBox", &duration)) edit.iDurationMs = static_cast<std::uint32_t>((std::max)(1, duration));
	const auto vectorControl = [](const char* label, std::array<double, 3u>& values, const float minimum, const float maximum) {
		float v[3] = { static_cast<float>(values[0]), static_cast<float>(values[1]), static_cast<float>(values[2]) };
		if (!ImGui::DragFloat3(label, v, 0.05f, minimum, maximum, "%.3f")) return false;
		for (std::size_t i = 0u; i < 3u; ++i) values[i] = v[i];
		return true;
	};
	bool geometryChanged = vectorControl("Position offset (m)##PresentationBox", edit.PositionOffset, -100000.f, 100000.f);
	const bool gameplayCollider = definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER &&
		(!edit.strLogicOccurrenceId.empty() || !m_strColliderLogicDefinitionId.empty() || (m_strColliderExecutionEditId == occurrenceId && m_bColliderDamageMode));
	if (gameplayCollider)
	{
		float yaw = static_cast<float>(edit.RotationDegrees[1]);
		if (ImGui::DragFloat("Yaw (degrees)##PresentationBox", &yaw, 0.05f, -36000.f, 36000.f, "%.3f"))
		{ edit.RotationDegrees[1] = yaw; geometryChanged = true; }
		if (edit.RotationDegrees[0] != 0.0 || edit.RotationDegrees[2] != 0.0)
		{
			ImGui::TextWrapped("Gameplay Collider supports Y rotation only. X %.3f / Z %.3f makes this Pattern unavailable after publish.", edit.RotationDegrees[0], edit.RotationDegrees[2]);
			if (ImGui::Button("Clear X/Z rotation##GameplayCollider"))
			{ edit.RotationDegrees[0] = edit.RotationDegrees[2] = 0.0; geometryChanged = true; }
		}
	}
	else if (!cameraBox) geometryChanged |= vectorControl("Rotation (degrees)##PresentationBox", edit.RotationDegrees, -36000.f, 36000.f);
	const bool light = definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT;
	if (!light && !cameraBox) geometryChanged |= vectorControl("Scale / size##PresentationBox", edit.Scale, 0.001f, 10000.f);
	const auto previewGeometry = [&]() {
		std::string status;
		(void)Request_PresentationGeometryPreview(patternId, edit, status);
	};
	if (geometryChanged && (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER ||
		definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)) previewGeometry();
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
	if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT || definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
		Render_PresentationAnchor(pattern, edit, definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
	if (!cameraBox && (!light || edit.strAnchorKind == "BOSS")) ImGui::Checkbox("Follow anchor##PresentationBox", &edit.bFollowBoss);
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
			m_ColliderDamageSettings = {};
			m_bColliderDamageMode = false; m_bColliderDetachDamage = false;
			if (linkedLogic)
			{
				m_ColliderDamageSettings.bRearmOnExit = linkedLogic->bRearmOnExit;
				m_ColliderDamageSettings.bRepeatAfterKnockback = linkedLogic->bRepeatAfterKnockback;
			}
			m_bColliderDamageDirty = false;
			if (nullptr != linkedBox)
				for (const auto& id : linkedBox->OnSuccessLogicIds)
					if (const auto* result = Find_Logic(m_Draft, id); result && result->strOutcomeKind == "MAX_HP_PERCENT_DAMAGE")
					{
						m_ColliderDamageSettings.iPercent = static_cast<int32_t>(result->iPercent);
						m_ColliderDamageSettings.fPushRangeM = result->fPushRangeM;
						m_ColliderDamageSettings.iPushMs = result->iPushMs;
						m_ColliderDamageSettings.strPushDirection = result->strPushDirection;
						m_bColliderDamageMode = linkedLogic && linkedLogic->strTriggerKind == "ENTER_AREA";
						break;
					}
		}
		int purpose = m_bColliderDamageMode ? 1 : 0;
		ImGui::BeginDisabled(definition.strColliderKind != "GEOMETRY");
		if (ImGui::Combo("Collider purpose", &purpose, "General region\0Damage collider\0"))
		{
			m_bColliderDamageMode = purpose == 1;
			m_bColliderDetachDamage = purpose == 0;
			m_bColliderDamageDirty = true;
		}
		ImGui::EndDisabled();
		if (m_bColliderDamageMode)
		{
			auto& damage = m_ColliderDamageSettings;
			if (ImGui::InputInt("Damage (% max HP)", &damage.iPercent, 1, 10))
			{ damage.iPercent = std::clamp(damage.iPercent, 1, 100); m_bColliderDamageDirty = true; }
			int repeat = damage.bRepeatAfterKnockback ? 2 : damage.bRearmOnExit ? 1 : 0;
			if (ImGui::Combo("Repeat hits", &repeat, "Once per box\0On exit and re-entry\0After knockback finishes\0"))
			{
				damage.bRearmOnExit = repeat == 1; damage.bRepeatAfterKnockback = repeat == 2;
				if (repeat == 2 && damage.fPushRangeM == 0.0) { damage.fPushRangeM = 2.0; damage.iPushMs = 242u; }
				m_bColliderDamageDirty = true;
			}
			bool push = damage.fPushRangeM > 0.0;
			if (ImGui::Checkbox("Push on hit", &push))
			{
				damage.fPushRangeM = push ? 2.0 : 0.0; damage.iPushMs = push ? 242u : 0u;
				if (!push) { damage.strPushDirection = "AWAY_FROM_BOSS"; damage.bRepeatAfterKnockback = false; }
				m_bColliderDamageDirty = true;
			}
			if (push)
			{
				float distance = static_cast<float>(damage.fPushRangeM); int time = static_cast<int>(damage.iPushMs);
				if (ImGui::DragFloat("Push distance (m)", &distance, .05f, .01f, 20.f))
				{ damage.fPushRangeM = std::clamp(distance, .01f, 20.f); m_bColliderDamageDirty = true; }
				if (ImGui::InputInt("Push time (ms)", &time, 10, 100))
				{ damage.iPushMs = static_cast<std::uint32_t>(std::clamp(time, 1, static_cast<int>(MAX_EDITOR_TIME_MS))); m_bColliderDamageDirty = true; }
				int direction = damage.strPushDirection == "BOSS_FORWARD" ? 1 : 0;
				if (ImGui::Combo("Push direction", &direction, "Away from boss\0Boss current forward\0"))
				{ damage.strPushDirection = direction == 1 ? "BOSS_FORWARD" : "AWAY_FROM_BOSS"; m_bColliderDamageDirty = true; }
			}
			ImGui::TextDisabled("Apply saves contact, damage and push together for this box.");
		}
		if (ImGui::CollapsingHeader("Advanced Logic / conditions"))
		{
			if (ImGui::BeginCombo("Execution", m_strColliderExecutionType.c_str()))
			{
				for (const char* type : { "DURATION", "TRIGGER" })
					if (ImGui::Selectable(type, m_strColliderExecutionType == type))
					{
						m_strColliderExecutionType = type;
						m_strColliderLogicDefinitionId.clear();
						m_bColliderDamageDirty = false;
					}
				ImGui::EndCombo();
			}
			const auto compatibleWindowCount = std::count_if(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(),
				[&](const auto& window) {
					const auto* logic = Find_Logic(m_Draft, window.strLogicId);
					return logic && logic->strLogicType == m_strColliderExecutionType &&
						(Kouku_LogicAcceptsColliders(*logic) || (logic->strLogicType == "TRIGGER" && logic->strTriggerKind.empty()));
				});
			if (ImGui::BeginCombo("Shared Logic window", nullptr == linkedBox ? "(none)" : linkedBox->strOccurrenceId.c_str()))
			{
				if (ImGui::Selectable("(none)", edit.strLogicOccurrenceId.empty()))
				{ edit.strLogicOccurrenceId.clear(); m_strColliderLogicDefinitionId.clear(); m_bColliderDamageDirty = false; }
				for (const auto& window : pattern.LogicOccurrences)
				{
					const auto* logic = Find_Logic(m_Draft, window.strLogicId);
					const bool nameOnlyTrigger = logic && logic->strLogicType == "TRIGGER" && logic->strTriggerKind.empty();
					if (nullptr == logic || logic->strLogicType != m_strColliderExecutionType ||
						(!Kouku_LogicAcceptsColliders(*logic) && !nameOnlyTrigger)) continue;
					const auto label = logic->strDisplayName + (nameOnlyTrigger ? " (set Trigger kind)" : "") + " | " + window.strOccurrenceId;
					if (ImGui::Selectable(label.c_str(), edit.strLogicOccurrenceId == window.strOccurrenceId))
					{
						edit.strLogicOccurrenceId = window.strOccurrenceId;
						m_strColliderLogicDefinitionId = window.strLogicId;
						m_bColliderDamageDirty = false;
						edit.iStartMs = window.iStartMs;
						edit.iDurationMs = window.iDurationMs;
					}
				}
				ImGui::EndCombo();
			}
			if (m_strColliderExecutionType == "TRIGGER" && compatibleWindowCount == 0)
				ImGui::TextWrapped("No reusable Trigger window is placed yet. Choose a Logic definition below and Apply Values to create its window and connect this Collider.");
			if (m_strColliderExecutionType == "TRIGGER")
				ImGui::TextWrapped("ENTER_AREA detects players entering this Collider, including a following boss-body region. OBJECT_CONTACT detects authored World card placements for hammer reactions.");
			const auto* definitionLogic = Find_Logic(m_Draft, m_strColliderLogicDefinitionId);
			std::string definitionLabel = definitionLogic ? definitionLogic->strDisplayName : "(select Logic)";
			if (definitionLogic && definitionLogic->strLogicType == "TRIGGER" && definitionLogic->strTriggerKind.empty())
				definitionLabel += " (set Trigger kind)";
			if (ImGui::BeginCombo("Logic definition", definitionLabel.c_str()))
			{
				for (const auto& logic : m_Draft.Logics)
				{
					const bool nameOnlyTrigger = logic.strLogicType == "TRIGGER" && logic.strTriggerKind.empty();
					if (logic.strLogicType != m_strColliderExecutionType ||
						(!Kouku_LogicAcceptsColliders(logic) && !nameOnlyTrigger)) continue;
					const std::string label = logic.strDisplayName + (nameOnlyTrigger ? " (set Trigger kind)" : "") + "##" + logic.strLogicId;
					if (ImGui::Selectable(label.c_str(), m_strColliderLogicDefinitionId == logic.strLogicId))
					{ m_strColliderLogicDefinitionId = logic.strLogicId; m_bColliderDamageDirty = false; }
				}
				ImGui::EndCombo();
			}
			// A selection above may have changed the definition in this same frame.
			definitionLogic = Find_Logic(m_Draft, m_strColliderLogicDefinitionId);
			if (definitionLogic && definitionLogic->strLogicType == "TRIGGER")
			{
				const auto definitionCopy = *definitionLogic;
				const auto pendingPresentation = edit;
				const auto generation = m_iDraftGeneration;
				Render_LogicDefinitionValues(definitionCopy, patternId, &pendingPresentation);
				// One Apply commits the edited definition, interval and Collider link.
				if (generation != m_iDraftGeneration) return;
				m_PresentationBoxEdit = pendingPresentation;
			}
			const auto* connectionLogic = definitionLogic;
			if (definitionLogic && definitionLogic->strLogicType == "TRIGGER" && m_strColliderLogicValueDraftId == definitionLogic->strLogicId)
				connectionLogic = &m_ColliderLogicValueDraft;
			ImGui::BeginDisabled(!connectionLogic || !Kouku_LogicAcceptsColliders(*connectionLogic));
			if (ImGui::Button("Append / reuse Logic window"))
			{
				std::string status;
				const auto value = edit;
				(void)Set_ColliderLogicValues(patternId, value, *connectionLogic, status);
				ImGui::EndDisabled();
				return;
			}
			ImGui::EndDisabled();
			ImGui::TextWrapped("Apply keeps this Collider's 3D size and synchronizes its start/lifetime with the current Logic window and linked Colliders. Selecting a different Shared Logic window first adopts that window's time. Result slots remain shared.");
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
		}
		bool debugRender = edit.bDebugRender;
		if (ImGui::Checkbox("Debug Render##ColliderBox", &debugRender))
		{
			std::string status;
			(void)Set_PresentationBoxDebugRender(patternId, occurrenceId, debugRender, status);
			return;
		}
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
			{
				for (std::size_t i = 0u; i < 3u; ++i) edit.Scale[i] = fullSize[i] / (2.0 * definition.HalfExtents[i]);
				previewGeometry();
			}
		}
		else
		{
			if (definition.strShape == "CIRCLE")
			{
				float radius = static_cast<float>(definition.fRadiusM * (std::max)(edit.Scale[0], edit.Scale[2]));
				if (ImGui::DragFloat("Radius (m)", &radius, .05f, .01f, 10000.f))
				{ edit.Scale[0] = edit.Scale[2] = radius / definition.fRadiusM; previewGeometry(); }
			}
			else
			{
				float axes[2] = { static_cast<float>(definition.fRadiusM * edit.Scale[0]), static_cast<float>(definition.fRadiusM * edit.Scale[2]) };
				if (ImGui::DragFloat2("Radius X / Z (m)", axes, .05f, .01f, 10000.f))
				{ edit.Scale[0] = axes[0] / definition.fRadiusM; edit.Scale[2] = axes[1] / definition.fRadiusM; previewGeometry(); }
				float angle = static_cast<float>(definition.fHalfAngleDegrees * 2.0);
				if (ImGui::DragFloat(definition.strShape == "REVERSE_SECTOR" ? "Safe angle (degrees)" : "Sector angle (degrees)",
					&angle, .1f, definition.strShape == "REVERSE_SECTOR" ? 0.f : .002f, 360.f))
				{
					auto candidate = m_Draft;
					for (auto& item : candidate.PresentationResources)
						if (item.strResourceId == definition.strResourceId) item.fHalfAngleDegrees = std::clamp(angle, definition.strShape == "REVERSE_SECTOR" ? 0.f : .002f, 360.f) * .5;
					for (auto& affected : candidate.Patterns)
						if (std::any_of(affected.PresentationOccurrences.begin(), affected.PresentationOccurrences.end(),
							[&](const auto& row) { return row.strResourceId == definition.strResourceId; })) Mark_Draft(candidate, affected);
					std::string status;
					const auto pending = edit;
					(void)Commit_Candidate(std::move(candidate), "Updated shared Collider angle.", status);
					m_PresentationBoxEdit = pending;
					return;
				}
				if (definition.strShape == "REVERSE_SECTOR") ImGui::TextDisabled("Safe angle 0 = full ellipse; 360 = empty. X/Z scale changes each axis independently.");
			}
		}
		ImGui::TextDisabled("Dimensions affect this box. Rotation sets the region direction.");
	}
	if (ImGui::Button("Apply##PresentationBox"))
	{
		std::string status;
		const auto value = edit;
		if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER &&
			m_bColliderDamageMode)
			(void)Set_ColliderTriggerDamage(patternId, value, m_ColliderDamageSettings, status);
		else if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER && m_bColliderDetachDamage)
		{
			auto general = value;
			general.strLogicOccurrenceId.clear();
			(void)Set_PresentationBox(patternId, general, status);
			m_strColliderExecutionEditId.clear();
		}
		else if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER && !m_strColliderLogicDefinitionId.empty())
		{
			const auto* selected = Find_Logic(m_Draft, m_strColliderLogicDefinitionId);
			const auto* linked = Find_LogicBox(pattern, value.strLogicOccurrenceId);
			if (selected && selected->strLogicType == "TRIGGER" && m_strColliderLogicValueDraftId == selected->strLogicId)
				(void)Set_ColliderLogicValues(patternId, value, m_ColliderLogicValueDraft, status);
			else if (selected && linked && linked->strLogicId == selected->strLogicId)
				(void)Set_PresentationBox(patternId, value, status);
			else (void)Connect_ColliderLogic(patternId, value, m_strColliderLogicDefinitionId, status);
		}
		else (void)Set_PresentationBox(patternId, value, status);
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("Preview##PresentationBox")) Queue_PresentationPreview(definition, &edit);
	if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER || definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
	{
		ImGui::SameLine();
		if (ImGui::Button("Revert geometry##PresentationBox"))
		{
			std::string status;
			(void)Request_PresentationGeometryPreview(patternId, *box, status);
			edit.PositionOffset = box->PositionOffset;
			edit.RotationDegrees = box->RotationDegrees;
			edit.Scale = box->Scale;
		}
		ImGui::TextDisabled("Geometry previews at the actor cursor. Save keeps geometry from all edited Effect and Collider boxes; Apply keeps other fields.");
	}
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
	ImGui::SeparatorText("World Objects");
	ImGui::InputTextWithHint("##WorldObjectFilter", "Search Objects", m_WorldObjectFilter, std::size(m_WorldObjectFilter));
	const auto* selectedPattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool patternEditable = selectedPattern && selectedPattern->strLoadError.empty() &&
		Pattern_DurationMs(*selectedPattern) > 0u;
	const KOUKU_WORLD_SEQUENCE_RESOURCE* selected = nullptr;
	if (ImGui::BeginChild("##WorldObjects", ImVec2(0.f, 230.f), ImGuiChildFlags_Borders))
	{
		std::size_t shown = 0u;
		for (const auto& resource : m_WorldSequenceResources)
		{
			if (resource.strObjectResourceId.empty() || (!resource.bDefaultMotion && !resource.strInstanceId.empty())) continue;
			if (resource.strObjectResourceId == m_strNewWorldObjectId) selected = &resource;
			if (!ContainsInsensitive(resource.strObjectDisplayName, m_WorldObjectFilter) &&
				!ContainsInsensitive(resource.strObjectResourceId, m_WorldObjectFilter)) continue;
			++shown;
			ImGui::PushID(resource.strObjectResourceId.c_str());
			const std::string label = resource.strObjectDisplayName + "###WorldObject";
			if (ImGui::Selectable(label.c_str(), m_strNewWorldObjectId == resource.strObjectResourceId))
			{
				m_strNewWorldObjectId = resource.strObjectResourceId;
				selected = &resource;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s%s", resource.strObjectResourceId.c_str(),
				resource.bEnabled && resource.bDefaultMotion ? "" : " | Set an enabled default animation in Object Tool, then Save.");
			ImGui::PopID();
		}
		if (!shown) ImGui::TextDisabled("No matching Objects. Create and Save them in Object Tool.");
	}
	ImGui::EndChild();
	if (!selected)
	{
		for (const auto& resource : m_WorldSequenceResources)
			if (resource.strObjectResourceId == m_strNewWorldObjectId && (resource.bDefaultMotion || resource.strInstanceId.empty()))
			{ selected = &resource; break; }
	}
	const bool ready = selected && selected->bEnabled && selected->bDefaultMotion && !selected->strInstanceId.empty();
	if (selected)
	{
		ImGui::TextUnformatted(selected->strObjectDisplayName.c_str());
		ImGui::TextDisabled("%s", selected->strAnchorKind == "BOSS" ? "Anchor / Boss BODY Bone" : selected->strAnchorKind == "PLAYER" ? "Anchor / Character" : "Map");
		if (ready) ImGui::TextWrapped("Default: %s", selected->strDisplayName.c_str());
		else ImGui::TextWrapped("Set an enabled default animation in Object Tool, then Save.");
	}
	ImGui::BeginDisabled(!ready || !patternEditable);
	if (ImGui::Button("Append Object"))
	{
		std::string status;
		(void)Append_WorldObject(m_strNewWorldObjectId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!ready);
	if (ImGui::Button("Preview Object"))
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE preview;
		preview.eKind = KOUKU_SAYDON_PRESENTATION_KIND::WORLD;
		preview.strAssetId = selected->strInstanceId;
		preview.strDisplayName = selected->strObjectDisplayName;
		preview.iDurationMs = (std::max)(1u, selected->iDurationMs);
		Queue_PresentationPreview(preview);
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled("Pattern: %s | Start: %u ms", selectedPattern ? selectedPattern->strDisplayName.c_str() : "Select a Pattern", m_iCursorMs);
	ImGui::TextWrapped("Append creates one Object box. Select the box to adjust its Transform and view its linked animations, then Save.");
	ImGui::SeparatorText("Composition World Resources");
	ImGui::BeginChild("##CompositionWorldNames", ImVec2(0.f, 110.f), ImGuiChildFlags_Borders);
	for (const auto& world : m_Draft.Worlds)
		if (ImGui::Selectable((world.strDisplayName + "##" + world.strWorldId).c_str(), m_strSelectedWorldId == world.strWorldId))
			m_strSelectedWorldId = world.strWorldId;
	ImGui::EndChild();
	if (const auto* world = Find_World(m_Draft, m_strSelectedWorldId))
		if (Render_RenameControl(RENAME_TARGET::WORLD, world->strWorldId, world->strDisplayName)) return;
	if (!m_strWorldSequenceResourceStatus.empty()) ImGui::TextDisabled("%s", m_strWorldSequenceResourceStatus.c_str());
	if (!m_strStatus.empty()) ImGui::TextWrapped("%s", m_strStatus.c_str());
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
	ImGui::SeparatorText("Object Box");
	ImGui::Text("%s", nullptr == world ? "(missing World)" : world->strDisplayName.c_str());
	ImGui::TextDisabled("%s -> %s", box->strWorldId.c_str(),
		nullptr == world ? "?" : world->strSequenceInstanceId.c_str());
	if (nullptr != world)
	{
		ImGui::SeparatorText("Transform");
		const auto source = std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(),
			[&](const auto& row) { return row.strInstanceId == world->strSequenceInstanceId; });
		const bool supportsPlacement = source != m_WorldSequenceResources.end() && source->bSupportsPlacement;
		const bool localPlacement = supportsPlacement && source->strAnchorKind != "WORLD";
		ImGui::BeginDisabled(!supportsPlacement || localPlacement);
		const bool placeNear = ImGui::Button("Place near character##WorldBox");
		ImGui::EndDisabled();
		if (placeNear)
		{
			std::string status;
			(void)Place_WorldBoxNearCharacter(patternId, occurrenceId, status);
			return;
		}
		if (box->Placement || localPlacement)
		{
			ImGui::SameLine();
			if (ImGui::Button("Preview placements##WorldBox")) Queue_WorldBoxPreview(patternId, occurrenceId);
			auto placement = box->Placement.value_or(KOUKU_SAYDON_WORLD_PLACEMENT{});
			const auto vectorControl = [](const char* label, std::array<double, 3u>& values,
				const float step, const float minimum, const float maximum) {
				float v[3] = { static_cast<float>(values[0]), static_cast<float>(values[1]), static_cast<float>(values[2]) };
				if (!ImGui::DragFloat3(label, v, step, minimum, maximum, "%.3f")) return false;
				for (std::size_t i = 0u; i < 3u; ++i) values[i] = v[i];
				return true;
			};
			bool changed = vectorControl(localPlacement ? "Local position (m)##WorldBox" : "World position (m)##WorldBox", placement.Position, .05f, -100000.f, 100000.f);
			changed |= vectorControl("Rotation (degrees)##WorldBox", placement.RotationDegrees, .5f, -36000.f, 36000.f);
			changed |= vectorControl("Scale##WorldBox", placement.Scale, .01f, .001f, 1000.f);
			if (changed)
			{
				std::string status;
				(void)Set_WorldBoxPlacement(patternId, occurrenceId, placement, status);
				return;
			}
			ImGui::TextWrapped(localPlacement ?
				"This box adds a local transform after its saved Motion and before the live Boss/Character anchor. Scale multiplies the authored Object size." :
				"This box owns its world transform. Other boxes and the saved Object Motion are independent. Scale multiplies the Object's authored size.");
		}
		else
		{
			ImGui::TextWrapped(supportsPlacement ?
				"This older box uses the saved Motion position and World definition offset. Place near character to create an independent transform for this box." :
				"This World uses existing map placements or a Character anchor. Its saved placement is preserved; resource Preview can be played near the character.");
		}
	}
	if (world)
	{
		const auto initial = std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(),
			[&](const auto& row) { return row.strInstanceId == world->strSequenceInstanceId; });
		const std::string objectId = !world->strObjectResourceId.empty() ? world->strObjectResourceId :
			(initial != m_WorldSequenceResources.end() ? initial->strObjectResourceId : std::string{});
		if (initial != m_WorldSequenceResources.end()) ImGui::TextWrapped("Initial animation: %s", initial->strDisplayName.c_str());
		if (!objectId.empty() && ImGui::CollapsingHeader("Linked animations", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (const auto& motion : m_WorldSequenceResources)
			{
				if (motion.strObjectResourceId != objectId || motion.strInstanceId.empty()) continue;
				ImGui::BulletText("%s%s", motion.strDisplayName.c_str(), motion.bEnabled ? "" : " (disabled)");
				for (const auto& clip : motion.AnimationClips) ImGui::TextDisabled("    %s", clip.c_str());
				if (motion.AnimationClips.empty()) ImGui::TextDisabled("    Transform animation");
			}
			ImGui::TextDisabled("Edit animations in Object Tool. Collider Logic selects the reaction for this Object.");
		}
		if (ImGui::CollapsingHeader("Attached Effect"))
			if (Render_WorldCompanionSelector(*world)) return;
	}
	ImGui::SeparatorText("Timing");
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
	ImGui::TextWrapped(m_bSequenceWorkspace ?
		"Preview uses this box placement at Start ms with its playback speed. Lifetime ms limits the active World box; sequence motion uses its authored length and speed. Sequence lifetime %u ms." :
		"Preview and Server Play use this box placement at Start ms with its playback speed. Lifetime ms limits the active World box; sequence motion uses its authored length and speed. Pattern lifetime %u ms.",
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
	const auto* selectedBundle = m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE ? Find_Bundle(m_Draft, m_strSelectedBundleId) : nullptr;
	const bool_t patternEditable = (selectedPattern && selectedPattern->strLoadError.empty()) || (selectedBundle && selectedBundle->strLoadError.empty());
	const std::string targetPatternName = selectedBundle ? selectedBundle->strDisplayName + " (Common)" : selectedPattern ? selectedPattern->strDisplayName : "none";
	const std::uint32_t lifetimeMs = selectedBundle ? Bundle_DurationMs(m_Draft, *selectedBundle) : selectedPattern ? Pattern_DurationMs(*selectedPattern) : 0u;

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
	if (Render_RenameControl(RENAME_TARGET::SCENE_PROFILE, sceneProfileId, profileName)) return;
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
	ImGui::Checkbox("Until sequence end##NewKoukuSceneProfileBox", &m_bNewSceneProfileBoxToPatternEnd);
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
	ImGui::InputInt("Blend ms (reserved)##KoukuSceneProfileBox", &m_iSceneProfileBoxBlendMs, 10, 100);
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
	ImGui::TextWrapped("The profile applies instantly at Start ms and the previous profile returns when Lifetime ends. Blend ms is reserved. Pattern lifetime %u ms.",
		lifetimeMs);
	ImGui::TextWrapped("Brightness: Rendering Workbench -> Light Resources -> Scene Profile -> Light Detail -> Map Light Intensity Multiplier.");
	if (ImGui::Button("Delete Scene Profile Box"))
	{
		std::string status;
		(void)Delete_SceneProfileBox(patternId, occurrenceId, status);
	}
}

void Client::CKoukuSaydonActionWorkbench::Render_LogicDefinitionValues(
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic,
	const std::string_view colliderPatternId,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE* collider)
{
	// Resources and Collider Detail can render different Logics in one frame.
	// Each keeps its pending values until its own Apply or Revert.
	auto& draftId = collider ? m_strColliderLogicValueDraftId : m_strLogicValueDraftId;
	auto& draft = collider ? m_ColliderLogicValueDraft : m_LogicValueDraft;
	if (draftId != logic.strLogicId)
	{
		draft = logic;
		draftId = logic.strLogicId;
	}
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
					else if ("EXTERNAL_SIGNAL" == draft.strJudgementKind || "COUNTER_WINDOW" == draft.strJudgementKind) draft.bEndsPatternOnSuccess = true;
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
		else if ("OBJECT_OVERLAP" == draft.strJudgementKind)
		{
			if (ImGui::BeginCombo("Target World Object##KoukuObjectOverlap",
				draft.strTargetWorldInstanceId.empty() ? "(pick saved target)" : draft.strTargetWorldInstanceId.c_str()))
			{
				for (const auto& candidate : m_WorldSequenceResources)
					if (candidate.bEnabled && !candidate.strInstanceId.empty() && !candidate.strObjectResourceId.empty() &&
						ImGui::Selectable((candidate.strDisplayName + "##" + candidate.strInstanceId).c_str(),
							draft.strTargetWorldInstanceId == candidate.strInstanceId))
						draft.strTargetWorldInstanceId = candidate.strInstanceId;
				ImGui::EndCombo();
			}
			float radius = static_cast<float>(draft.fTargetRadiusM);
			if (ImGui::InputFloat("Target radius (m)##KoukuObjectOverlap", &radius, .1f, 1.f, "%.2f"))
				draft.fTargetRadiusM = std::clamp(radius, .01f, 1000.f);
			if (ImGui::BeginCombo("Overlap outcome##KoukuObjectOverlap", draft.strInsideOutcome.c_str()))
			{
				for (const char* outcome : { "SUCCESS", "FAIL" })
					if (ImGui::Selectable(outcome, draft.strInsideOutcome == outcome)) draft.strInsideOutcome = outcome;
				ImGui::EndCombo();
			}
			ImGui::TextWrapped("The Server checks linked Colliders against this authored fixed circle every tick. First overlap runs the selected Result once; no overlap runs Timeout. Keep one static target WORLD box alive for the whole window. Animation pose does not change this circle.");
		}
		else if ("EXTERNAL_SIGNAL" == draft.strJudgementKind)
		{
			ImGui::Checkbox("End Pattern on success", &draft.bEndsPatternOnSuccess);
			ImGui::TextWrapped("Waits for COMPLETE_LOGIC_WINDOW from a contact Trigger. Success runs this window's Results and cancels its Timeout. Without a signal, Timeout runs at the deadline.");
		}
		else if ("COUNTER_WINDOW" == draft.strJudgementKind)
		{
			ImGui::Checkbox("End Pattern on counter success", &draft.bEndsPatternOnSuccess);
			ImGui::TextWrapped("A Server-approved counter hit during this window runs Success. Connect a FOLLOWUP_PATTERN Result to play groggy; without a counter the window runs Timeout.");
		}
		else if ("ATTACHMENT_HOLD" == draft.strJudgementKind)
			ImGui::TextWrapped("Keeps captured players attached until this window ends. It has no Collider or outcome slots. Select this window in the capture Trigger's Hold connection.");
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
			if (ImGui::BeginCombo("Facing boss outcome##KoukuLogicValue", draft.strInsideOutcome.c_str()))
			{
				for (const char* outcome : { "SUCCESS", "FAIL" })
					if (ImGui::Selectable(outcome, draft.strInsideOutcome == outcome)) draft.strInsideOutcome = outcome;
				ImGui::EndCombo();
			}
			ImGui::TextDisabled("At the window end, the real boss inside this cone runs the selected Result slot; outside runs the opposite slot.");
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
					if ("FEAR" == draft.strOutcomeKind) draft.iDurationMs = 3000u;
					if ("CAPTURE_PLAYER" == draft.strOutcomeKind) draft.strAttachmentSlot = "BOSS_LEFT_HAND";
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
		if ("MAX_HP_PERCENT_DAMAGE" == draft.strOutcomeKind)
		{
			bool knockback = draft.fPushRangeM > 0.0;
			if (ImGui::Checkbox("Knockback##KoukuLogicValue", &knockback))
			{
				draft.fPushRangeM = knockback ? 2.0 : 0.0;
				draft.iPushMs = knockback ? 242u : 0u;
				if (!knockback) draft.strPushDirection = "AWAY_FROM_BOSS";
			}
			if (knockback)
			{
				float distance = static_cast<float>(draft.fPushRangeM);
				int timeMs = static_cast<int>(draft.iPushMs);
				if (ImGui::InputFloat("Knockback distance (m)", &distance, .1f, 1.f, "%.2f"))
					draft.fPushRangeM = std::clamp(distance, .01f, 20.f);
				if (ImGui::InputInt("Knockback time (ms)", &timeMs, 10, 100))
					draft.iPushMs = static_cast<std::uint32_t>(std::clamp(timeMs, 1, static_cast<int>(MAX_EDITOR_TIME_MS)));
				int direction = draft.strPushDirection == "BOSS_FORWARD" ? 1 : 0;
				if (ImGui::Combo("Push direction##Logic", &direction, "Away from boss\0Boss current forward\0"))
					draft.strPushDirection = direction == 1 ? "BOSS_FORWARD" : "AWAY_FROM_BOSS";
				ImGui::TextWrapped("Uses the Server hit reaction, navigation and collision rules.");
			}
		}
		if ("CLOWN_TRANSFORM" == draft.strOutcomeKind)
		{
			int32_t durationMs = static_cast<int32_t>(draft.iDurationMs);
			if (ImGui::InputInt("Hold ms (0 = encounter policy)##KoukuLogicValue", &durationMs, 100, 1000))
				draft.iDurationMs = static_cast<std::uint32_t>(std::clamp(durationMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS)));
		}
		if ("CAPTURE_PLAYER" == draft.strOutcomeKind)
		{
			ImGui::TextUnformatted("Attachment: BOSS_LEFT_HAND");
			float offset[3] = {float(draft.GripLocalOffset[0]), float(draft.GripLocalOffset[1]), float(draft.GripLocalOffset[2])};
			if (ImGui::InputFloat3("Grip forward / up / right (m)", offset, "%.3f"))
				for (std::size_t axis = 0u; axis < 3u; ++axis) draft.GripLocalOffset[axis] = std::clamp(offset[axis], -10.f, 10.f);
			ImGui::TextWrapped("Connect this Result alone to ENTER_AREA Success and select that Trigger's Hold window. Zero offset uses the left-hand socket. Release happens at the Hold end, even for players captured later.");
		}
		if ("FEAR" == draft.strOutcomeKind)
		{
			int durationMs = static_cast<int>(draft.iDurationMs);
			if (ImGui::InputInt("Fear duration ms", &durationMs, 100, 1000))
				draft.iDurationMs = static_cast<std::uint32_t>(std::clamp(durationMs, 1, static_cast<int>(MAX_EDITOR_TIME_MS)));
			const auto* profile = Find_SceneProfile(m_Draft, draft.strSceneProfileId);
			if (ImGui::BeginCombo("Fear Scene Profile", profile ? profile->strDisplayName.c_str() :
				(draft.strSceneProfileId.empty() ? "(none)" : "(missing saved profile)")))
			{
				if (ImGui::Selectable("(none)", draft.strSceneProfileId.empty())) draft.strSceneProfileId.clear();
				for (const auto& item : m_Draft.SceneProfiles)
					if (ImGui::Selectable((item.strDisplayName + "##" + item.strSceneProfileId).c_str(), draft.strSceneProfileId == item.strSceneProfileId))
						draft.strSceneProfileId = item.strSceneProfileId;
				ImGui::EndCombo();
			}
			const auto* effect = Find_PresentationResource(m_Draft, draft.strEffectResourceId);
			const auto* light = Find_PresentationResource(m_Draft, draft.strLightResourceId);
			if (ImGui::BeginCombo("Fear Character Light", light ? light->strDisplayName.c_str() :
				(draft.strLightResourceId.empty() ? "(none)" : "(missing saved Light)")))
			{
				if (ImGui::Selectable("(none)", draft.strLightResourceId.empty())) draft.strLightResourceId.clear();
				for (const auto& item : m_Draft.PresentationResources)
					if (item.eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT && item.strDefaultAnchorKind == "PLAYER" &&
						ImGui::Selectable((item.strDisplayName + "##" + item.strResourceId).c_str(), draft.strLightResourceId == item.strResourceId))
						draft.strLightResourceId = item.strResourceId;
				ImGui::EndCombo();
			}
			if (ImGui::BeginCombo("Fear Effect", effect ? effect->strDisplayName.c_str() :
				(draft.strEffectResourceId.empty() ? "(none)" : "(missing saved Effect)")))
			{
				if (ImGui::Selectable("(none)", draft.strEffectResourceId.empty()))
				{ draft.strEffectResourceId.clear(); draft.iEffectDelayMs = 0u; }
				for (const auto& item : m_Draft.PresentationResources)
					if (item.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT &&
						ImGui::Selectable((item.strDisplayName + "##" + item.strResourceId).c_str(), draft.strEffectResourceId == item.strResourceId))
						draft.strEffectResourceId = item.strResourceId;
				ImGui::EndCombo();
			}
			ImGui::BeginDisabled(draft.strEffectResourceId.empty());
			int delayMs = static_cast<int>(draft.iEffectDelayMs);
			if (ImGui::InputInt("Effect delay from Fear start ms", &delayMs, 100, 1000))
				draft.iEffectDelayMs = static_cast<std::uint32_t>(std::clamp(delayMs, 0,
					(std::max)(0, static_cast<int>(draft.iDurationMs) - 1)));
			ImGui::EndDisabled();
			ImGui::TextWrapped("Server Fear locks movement and skills and plays the character's fear animation. The affected local player sees this Scene Profile and optional delayed Effect for the same duration. A full-screen face belongs to a Screen Effect resource.");
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
			if (!m_bSequenceWorkspace)
				ImGui::TextDisabled("Counter, stagger and supported success windows may start a follow-up; Publish All Patterns validates the target.");
		}
		if ("PLAY_WORLD_OBJECT_MOTION" == draft.strOutcomeKind)
		{
			const auto findWorld = [this](const std::string& id) -> const KOUKU_WORLD_SEQUENCE_RESOURCE*
			{
				const auto found = std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(),
					[&id](const auto& row) { return !id.empty() && row.bEnabled && row.strInstanceId == id; });
				return found == m_WorldSequenceResources.end() ? nullptr : &*found;
			};
			const auto* target = findWorld(draft.strTargetWorldInstanceId);
			const char* targetLabel = target ? target->strDisplayName.c_str() : "(pick spawned World Object)";
			if (ImGui::BeginCombo("Target World Object##KoukuMotionResult", targetLabel))
			{
				for (const auto& candidate : m_WorldSequenceResources)
				{
					if (!candidate.bEnabled || candidate.strInstanceId.empty() || candidate.strObjectResourceId.empty()) continue;
					const std::string label = candidate.strDisplayName + "##" + candidate.strInstanceId;
					if (ImGui::Selectable(label.c_str(), draft.strTargetWorldInstanceId == candidate.strInstanceId))
					{
						draft.strTargetWorldInstanceId = candidate.strInstanceId;
						target = &candidate;
						const auto* motion = findWorld(draft.strMotionInstanceId);
						if (!motion || motion->strObjectResourceId != candidate.strObjectResourceId)
							draft.strMotionInstanceId.clear();
					}
				}
				ImGui::EndCombo();
			}
			const auto* motion = findWorld(draft.strMotionInstanceId);
			if (ImGui::BeginCombo("Saved Motion##KoukuMotionResult", motion ? motion->strDisplayName.c_str() : "(pick same-object Motion)"))
			{
				if (target && !target->strObjectResourceId.empty())
					for (const auto& candidate : m_WorldSequenceResources)
						if (candidate.bEnabled && !candidate.strInstanceId.empty() && candidate.strObjectResourceId == target->strObjectResourceId &&
							ImGui::Selectable((candidate.strDisplayName + "##" + candidate.strInstanceId).c_str(),
								draft.strMotionInstanceId == candidate.strInstanceId))
							draft.strMotionInstanceId = candidate.strInstanceId;
				ImGui::EndCombo();
			}
			ImGui::TextWrapped("Server applies this saved Motion once per window to the existing target instance. Start that target in the WORLD lane first.");
			if (!target || !motion || target->strObjectResourceId.empty() || target->strObjectResourceId != motion->strObjectResourceId)
				ImGui::TextDisabled("Choose a target and Motion bound to the same World Object in this Area.");
		}
		if (draft.strOutcomeKind == "PLAY_CONTACT_WORLD_OBJECT_MOTION")
		{
			const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
			if (ImGui::BeginCombo("Copy targets from Trigger", "Choose OBJECT_CONTACT"))
			{
				if (pattern)
					for (const auto& window : pattern->LogicOccurrences)
					{
						const auto* trigger = Find_Logic(m_Draft, window.strLogicId);
						if (!trigger || trigger->strTriggerKind != "OBJECT_CONTACT") continue;
						if (ImGui::Selectable((trigger->strDisplayName + "##" + window.strOccurrenceId).c_str()))
						{
							std::vector<KOUKU_SAYDON_CONTACT_MOTION> mappings;
							for (const auto& target : trigger->TargetWorldOccurrenceIds)
							{
								const auto existing = std::find_if(draft.ContactMotions.begin(), draft.ContactMotions.end(),
									[&](const auto& row) { return row.strTargetWorldOccurrenceId == target; });
								mappings.push_back(existing == draft.ContactMotions.end() ? KOUKU_SAYDON_CONTACT_MOTION{target, {}} : *existing);
							}
							draft.ContactMotions = std::move(mappings);
						}
					}
				ImGui::EndCombo();
			}
			for (auto& mapping : draft.ContactMotions)
			{
				const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE* targetBox = nullptr;
				if (pattern)
					for (const auto& row : pattern->WorldOccurrences)
						if (row.strOccurrenceId == mapping.strTargetWorldOccurrenceId) { targetBox = &row; break; }
				const auto* world = targetBox ? Find_World(m_Draft, targetBox->strWorldId) : nullptr;
				const auto targetResource = std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(),
					[&](const auto& row) { return world && row.strInstanceId == world->strSequenceInstanceId; });
				const auto motionResource = std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(),
					[&](const auto& row) { return row.strInstanceId == mapping.strMotionInstanceId; });
				ImGui::PushID(mapping.strTargetWorldOccurrenceId.c_str());
				ImGui::TextWrapped("%s | %s", world ? world->strDisplayName.c_str() : "Missing WORLD placement", mapping.strTargetWorldOccurrenceId.c_str());
				if (ImGui::BeginCombo("Reaction Motion", motionResource == m_WorldSequenceResources.end() ? "Choose saved Motion" : motionResource->strDisplayName.c_str()))
				{
					for (const auto& candidate : m_WorldSequenceResources)
						if (targetResource != m_WorldSequenceResources.end() && candidate.bEnabled && !candidate.strInstanceId.empty() &&
							!candidate.strObjectResourceId.empty() && candidate.strObjectResourceId == targetResource->strObjectResourceId &&
							ImGui::Selectable((candidate.strDisplayName + "##" + candidate.strInstanceId).c_str(), candidate.strInstanceId == mapping.strMotionInstanceId))
							mapping.strMotionInstanceId = candidate.strInstanceId;
					ImGui::EndCombo();
				}
				ImGui::PopID();
			}
			ImGui::TextWrapped("Choose a saved reaction Motion for every selected card placement. On contact, only the hit card receives its Motion. Use Flip for the center Trigger and Hop for the edge Trigger.");
		}
		else if (draft.strOutcomeKind == "COMPLETE_LOGIC_WINDOW")
		{
			const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
			const auto* target = pattern ? Find_LogicBox(*pattern, draft.strTargetLogicOccurrenceId) : nullptr;
			const auto* targetLogic = target ? Find_Logic(m_Draft, target->strLogicId) : nullptr;
			if (ImGui::BeginCombo("Complete Duration window", targetLogic ? targetLogic->strDisplayName.c_str() : "Choose EXTERNAL_SIGNAL window"))
			{
				if (pattern)
					for (const auto& window : pattern->LogicOccurrences)
					{
						const auto* definition = Find_Logic(m_Draft, window.strLogicId);
						if (window.bEnabled && definition && definition->strLogicType == "DURATION" && definition->strJudgementKind == "EXTERNAL_SIGNAL" &&
							ImGui::Selectable((definition->strDisplayName + "##" + window.strOccurrenceId).c_str(), draft.strTargetLogicOccurrenceId == window.strOccurrenceId))
							draft.strTargetLogicOccurrenceId = window.strOccurrenceId;
					}
				ImGui::EndCombo();
			}
			if (ImGui::BeginCombo("Only when this card is hit", draft.strContactTargetWorldOccurrenceId.empty() ? "Any selected target" : draft.strContactTargetWorldOccurrenceId.c_str()))
			{
				if (ImGui::Selectable("Any selected target", draft.strContactTargetWorldOccurrenceId.empty())) draft.strContactTargetWorldOccurrenceId.clear();
				if (pattern)
					for (const auto& worldBox : pattern->WorldOccurrences)
					{
						const auto* world = Find_World(m_Draft, worldBox.strWorldId);
						if (world && ImGui::Selectable((world->strDisplayName + " | " + worldBox.strOccurrenceId).c_str(), draft.strContactTargetWorldOccurrenceId == worldBox.strOccurrenceId))
							draft.strContactTargetWorldOccurrenceId = worldBox.strOccurrenceId;
					}
				ImGui::EndCombo();
			}
			ImGui::TextWrapped("For Joker Search, select the Joker placement here. Add this Result after the card Motion in the center Trigger's Success slot. The receiving Duration runs its own Success Results and cannot later time out.");
		}
		if ("INSTANT_DEATH" == draft.strOutcomeKind)
			ImGui::TextDisabled("Applied to each judged player; on a stagger timeout to the whole raid.");
	}
	else if ("TRIGGER" == logic.strLogicType)
	{
		const std::array<std::pair<const char*, const char*>, 5u> triggerKinds = {{
			{"", "(choose what activates this Trigger)"},
			{"ENTER_AREA", "Player contact (ENTER_AREA)"},
			{"OBJECT_CONTACT", "World object contact (OBJECT_CONTACT)"},
			{"HUD_ENTER", "Switch player HUD at start (HUD_ENTER)"},
			{"REAL_GAZE_TELEPORT", "Teleport real Saydon and spawn decoys (REAL_GAZE_TELEPORT)"}
		}};
		const auto selectedKind = std::find_if(triggerKinds.begin(), triggerKinds.end(),
			[&](const auto& entry) { return draft.strTriggerKind == entry.first; });
		if (ImGui::BeginCombo("Trigger kind", selectedKind == triggerKinds.end() ? draft.strTriggerKind.c_str() : selectedKind->second))
		{
			for (const auto& [kind, label] : triggerKinds)
			{
				if (collider && *kind != '\0' && std::string_view(kind) != "ENTER_AREA" && std::string_view(kind) != "OBJECT_CONTACT") continue;
				if (ImGui::Selectable(label, draft.strTriggerKind == kind) && draft.strTriggerKind != kind)
				{
					draft = KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION{ logic.strLogicId, logic.strDisplayName, logic.strLogicType };
					draft.strTriggerKind = kind;
					if (draft.strTriggerKind == "HUD_ENTER") draft.strHudMode = "NONE";
					if (draft.strTriggerKind == "OBJECT_CONTACT") draft.fTargetRadiusM = 1.0;
					if (draft.strTriggerKind == "REAL_GAZE_TELEPORT") draft.ClockHours = { 4u, 7u, 10u };
				}
			}
			ImGui::EndCombo();
		}
		if (collider)
			ImGui::TextWrapped("This Collider supplies the contact shape, position and size. Apply Values also matches the linked Trigger's start/lifetime to the Collider. HUD and teleport Triggers are placed on the Logic lane without a Collider.");
		if (draft.strTriggerKind == "ENTER_AREA")
		{
			int repeat = draft.bRepeatAfterKnockback ? 2 : draft.bRearmOnExit ? 1 : 0;
			if (ImGui::Combo("Contact repetition", &repeat, "Once per player\0On exit and re-entry\0After knockback finishes (while inside)\0"))
			{ draft.bRearmOnExit = repeat == 1; draft.bRepeatAfterKnockback = repeat == 2; }
			float distance = static_cast<float>(draft.fBossChargeDistanceM);
			if (ImGui::InputFloat("Charge distance (m)", &distance, 0.5f, 1.f, "%.2f"))
				{
                    draft.fBossChargeDistanceM = std::clamp(distance, 0.f, 1000.f);
                    if (draft.fBossChargeDistanceM == 0.0) draft.fChargeYawOffsetDegrees = 0.0;
                }
            if (draft.fBossChargeDistanceM > 0.0)
            {
                float yawOffset = static_cast<float>(draft.fChargeYawOffsetDegrees);
                if (ImGui::InputFloat("Charge facing offset (degrees)", &yawOffset, 5.f, 90.f, "%.1f"))
                    draft.fChargeYawOffsetDegrees = std::clamp(yawOffset, -360.f, 360.f);
                ImGui::TextWrapped("Rotates the boss body without changing its captured travel direction.");
            }
			ImGui::TextWrapped("A linked Collider supplies the player-contact region. Entry runs Success for each player. After knockback finishes repeats while inside, with no hit during knockback. Connect one damage Result with Knockback enabled. Exit/re-entry mode requires leaving all linked Colliders first. No entry before the window ends is Timeout. Charge distance 0 keeps the boss still; positive distance uses this Trigger window and captures its target direction once at the window start.");
		}
		else if (draft.strTriggerKind == "OBJECT_CONTACT")
		{
			float radius = static_cast<float>(draft.fTargetRadiusM);
			if (ImGui::InputFloat("Card target radius (m)", &radius, .1f, 1.f, "%.2f"))
				draft.fTargetRadiusM = std::clamp(radius, .01f, 1000.f);
			char group[256]{}; Copy_Text(group, std::size(group), draft.strContactGroupId);
			if (ImGui::InputText("Strike group (optional)", group, std::size(group))) draft.strContactGroupId = group;
			int priority = static_cast<int>(draft.iContactPriority);
			if (ImGui::InputInt("Contact priority", &priority)) draft.iContactPriority = static_cast<std::uint32_t>(std::clamp(priority, 0, 1000));
			ImGui::TextWrapped("Use the same group and time for one strike's center/edge windows; higher priority wins for each card. The group may be reused at another strike time.");
			ImGui::SeparatorText("Target card placements");
			const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
			if (pattern)
				for (const auto& worldBox : pattern->WorldOccurrences)
				{
					const auto* world = Find_World(m_Draft, worldBox.strWorldId);
					const auto resource = std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(),
						[&](const auto& row) { return world && row.strInstanceId == world->strSequenceInstanceId && row.bEnabled && !row.strObjectResourceId.empty(); });
					if (resource == m_WorldSequenceResources.end()) continue;
					bool selected = std::find(draft.TargetWorldOccurrenceIds.begin(), draft.TargetWorldOccurrenceIds.end(), worldBox.strOccurrenceId) != draft.TargetWorldOccurrenceIds.end();
					if (ImGui::Checkbox((world->strDisplayName + " | " + worldBox.strOccurrenceId).c_str(), &selected))
					{
						if (selected && draft.TargetWorldOccurrenceIds.size() < 64u) draft.TargetWorldOccurrenceIds.push_back(worldBox.strOccurrenceId);
						else if (!selected) std::erase(draft.TargetWorldOccurrenceIds, worldBox.strOccurrenceId);
					}
				}
			ImGui::TextWrapped("Append card WORLD boxes to this Pattern first. Select the placements that may be hit. Each is judged once during the Trigger window; no contact produces no reaction. Bone anchors follow the hammer tip in XZ. Match the window to the strike moment; height alone does not activate contact.");
		}
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
		if (collider) (void)Set_ColliderLogicValues(colliderPatternId, *collider, draft, status);
		else (void)Set_LogicDefinitionValues(logic.strLogicId, draft, status);
	}
	ImGui::SameLine();
	if (ImGui::Button("Revert##KoukuLogicValue"))
		draftId.clear();
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
	if (Render_RenameControl(RENAME_TARGET::LOGIC, logicId, logicName)) return;
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
	if (logic && logic->strJudgementKind == "ATTACHMENT_HOLD")
		ImGui::TextWrapped("Attachment Hold ends at %u ms. It owns the release deadline and has no Collider or outcome slots.", box->iStartMs + box->iDurationMs);
	if (logic && logic->strTriggerKind == "ENTER_AREA")
	{
		const auto* hold = Find_LogicBox(*pattern, box->strHoldLogicOccurrenceId);
		const auto* holdLogic = hold ? Find_Logic(m_Draft, hold->strLogicId) : nullptr;
		std::string selectedHold = box->strHoldLogicOccurrenceId;
		bool changed = false;
		if (ImGui::BeginCombo("Capture Hold window", holdLogic ? holdLogic->strDisplayName.c_str() : "(none)"))
		{
			if (ImGui::Selectable("(none)", selectedHold.empty())) { selectedHold.clear(); changed = true; }
			for (const auto& candidate : pattern->LogicOccurrences)
			{
				const auto* definition = Find_Logic(m_Draft, candidate.strLogicId);
				if (definition && definition->strLogicType == "DURATION" && definition->strJudgementKind == "ATTACHMENT_HOLD" &&
					ImGui::Selectable((definition->strDisplayName + " | " + std::to_string(candidate.iStartMs) + ".." +
						std::to_string(candidate.iStartMs + candidate.iDurationMs) + " ms##" + candidate.strOccurrenceId).c_str(),
						selectedHold == candidate.strOccurrenceId))
				{ selectedHold = candidate.strOccurrenceId; changed = true; }
			}
			ImGui::EndCombo();
		}
		if (changed) { std::string status; (void)Set_LogicBoxHold(patternId, occurrenceId, selectedHold, status); return true; }
	}
	if (nullptr != logic && Kouku_LogicOwnsOutcomes(*logic))
	{
		/* Each outcome slot lists up to four RESULT Logics in the order the
		   Server applies them. A slot the judgement kind never ends in stays
		   disabled: gaze has no Timeout and the stagger window has no Fail. */
		ImGui::SeparatorText("Outcomes");
		if (Kouku_LogicOutcomeKind(*logic).empty())
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.24f, 1.f),
				"Give this Logic a judgement kind in Resources > Logic before this Pattern can run on the Server.");
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
		ImGui::TextDisabled("%s boxes carry no outcomes; only Duration or Collider Trigger owns result slots.",
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
	if (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN) { Render_HierarchyDetails(); return; }
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (nullptr == pattern)
	{
		ImGui::TextDisabled("%s", m_bSequenceWorkspace ? "No Sequence selected." : "No Pattern selected.");
		return;
	}
	const std::string patternId = pattern->strPatternId;
	if (!pattern->strLoadError.empty())
	{
		ImGui::TextWrapped("%s", pattern->strLoadError.c_str());
		ImGui::TextWrapped("Original pattern JSON is preserved. Repair the source and Reload, or delete this pattern.");
		if (ImGui::Button(m_bSequenceWorkspace ? "Delete Invalid Sequence###Delete Invalid Pattern" : "Delete Invalid Pattern"))
		{
			std::string status;
			(void)Delete_Pattern(patternId, status);
		}
		return;
	}
	if (!m_strBundleReturnId.empty() && Find_Bundle(m_Draft, m_strBundleReturnId))
	{
		if (ImGui::Button("Back to Bundle")) { Select_Hierarchy(KOUKU_PATTERN_SELECTION::BUNDLE, m_strBundleReturnId); return; }
	}
	ImGui::TextWrapped("%s | %s", Gate_Label(pattern->strGateId), pattern->strTargetBossPlacementId.c_str());
	ImGui::TextDisabled("%s ID: %s", m_bSequenceWorkspace ? "Sequence" : "Pattern", patternId.c_str());
	ImGui::SeparatorText(m_bSequenceWorkspace ? "Sequence" : "Pattern");
	if (ImGui::InputText("Display Name", m_PatternName, std::size(m_PatternName),
			ImGuiInputTextFlags_EnterReturnsTrue))
	{
		std::string status;
		(void)Rename_Pattern(patternId, m_PatternName, status);
		return;
	}
	const auto* parent = Find_Folder(m_Draft, pattern->strFolderId);
	std::string requestedParent;
	bool_t changeParent = false;
	if (ImGui::BeginCombo("Parent##Pattern", parent ? parent->strDisplayName.c_str() : "None (Gate root)"))
	{
		if (ImGui::Selectable("None (Gate root)", pattern->strFolderId.empty())) changeParent = true;
		for (const auto& folder : m_Draft.Folders)
			if (folder.strGateId == pattern->strGateId && folder.strLoadError.empty() &&
				ImGui::Selectable((folder.strDisplayName + "##" + folder.strFolderId).c_str(), folder.strFolderId == pattern->strFolderId))
			{ requestedParent = folder.strFolderId; changeParent = true; }
		ImGui::EndCombo();
	}
	if (changeParent)
	{
		std::string status;
		(void)Set_PatternFolder(patternId, requestedParent, status);
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
	if (!m_bSequenceWorkspace)
		ImGui::TextDisabled("Publish All Patterns checks whether this Pattern is ready for Server playback.");
	bool resetToSpawn = pattern->bResetBossToSpawn;
	if (ImGui::Checkbox("Reset boss to spawn at Pattern start", &resetToSpawn))
	{
		auto candidate = m_Draft;
		auto* edited = Find_Pattern(candidate, patternId);
		if (nullptr != edited)
		{
			edited->bResetBossToSpawn = resetToSpawn;
			if (resetToSpawn) edited->BossMotion.reset();
			if (!resetToSpawn)
				edited->ResetBossYawDegrees.reset();
			Mark_Draft(candidate, *edited);
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated boss spawn reset for this Pattern.", status);
		}
		return;
	}
	if (resetToSpawn)
	{
		bool resetYaw = pattern->ResetBossYawDegrees.has_value();
		float yaw = static_cast<float>(pattern->ResetBossYawDegrees.value_or(0.0));
		bool yawChanged = ImGui::Checkbox("Reset boss facing at Pattern start", &resetYaw);
		if (resetYaw)
			yawChanged = ImGui::DragFloat("Boss reset yaw (degrees)", &yaw, 0.5f, -360.f, 360.f, "%.2f") || yawChanged;
		if (yawChanged)
		{
			auto candidate = m_Draft;
			auto* edited = Find_Pattern(candidate, patternId);
			if (nullptr != edited)
			{
				edited->ResetBossYawDegrees = resetYaw ? std::optional<double>{ yaw } : std::nullopt;
				Mark_Draft(candidate, *edited);
				std::string status;
				(void)Commit_Candidate(std::move(candidate), "Updated fixed boss facing for this Pattern.", status);
			}
			return;
		}
	}
	float rootVerticalScale = float(pattern->fAnimationRootVerticalScale);
	if (ImGui::DragFloat("Animation jump height", &rootVerticalScale, 0.01f, 0.f, 1.f, "%.2fx", ImGuiSliderFlags_AlwaysClamp))
	{
		auto candidate = m_Draft;
		if (auto* edited = Find_Pattern(candidate, patternId))
		{
			edited->fAnimationRootVerticalScale = rootVerticalScale;
			Mark_Draft(candidate, *edited);
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated animation jump height for this Pattern.", status);
		}
		return;
	}
	ImGui::TextDisabled("Scales root height only; body size and animation timing stay the same.");
	bool moveBoss = pattern->BossMotion.has_value();
	if (ImGui::Checkbox("Move boss during Pattern", &moveBoss))
	{
		auto candidate = m_Draft;
		auto* edited = Find_Pattern(candidate, patternId);
		if (edited)
		{
			if (moveBoss)
			{
				edited->BossMotion = KOUKU_SAYDON_BOSS_MOTION{};
				edited->BossMotion->iEndMs = (std::max)(1u, Pattern_DurationMs(*edited));
				edited->bResetBossToSpawn = false;
				edited->ResetBossYawDegrees.reset();
			}
			else edited->BossMotion.reset();
			Mark_Draft(candidate, *edited);
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated boss movement for this Pattern.", status);
		}
		return;
	}
	if (pattern->BossMotion)
	{
		auto motion = *pattern->BossMotion;
		int startMs = int(motion.iStartMs), endMs = int(motion.iEndMs);
		float startXZ[2] = {float(motion.StartPosition[0]), float(motion.StartPosition[2])};
		float endXZ[2] = {float(motion.EndPosition[0]), float(motion.EndPosition[2])};
		float baseY = float(motion.StartPosition[1]), yaw = float(motion.fYawDegrees);
		bool changed = ImGui::DragInt("Move start (ms)", &startMs, 1.f, 0, (std::max)(0, endMs - 1));
		changed = ImGui::DragInt("Move end (ms)", &endMs, 1.f, startMs + 1, int(Pattern_DurationMs(*pattern))) || changed;
		changed = ImGui::DragFloat2("Boss start XZ", startXZ, 0.05f, -100000.f, 100000.f) || changed;
		changed = ImGui::DragFloat2("Boss end XZ", endXZ, 0.05f, -100000.f, 100000.f) || changed;
		changed = ImGui::DragFloat("Boss base Y", &baseY, 0.05f, -100000.f, 100000.f) || changed;
		changed = ImGui::DragFloat("Boss motion yaw", &yaw, 0.1f, -360.f, 360.f, "%.4f") || changed;
		ImGui::TextDisabled("Holds the end position. Original animation supplies vertical pose.");
		if (changed)
		{
			motion.iStartMs = std::uint32_t(startMs); motion.iEndMs = std::uint32_t(endMs);
			motion.StartPosition = {startXZ[0], baseY, startXZ[1]};
			motion.EndPosition = {endXZ[0], baseY, endXZ[1]}; motion.fYawDegrees = yaw;
			auto candidate = m_Draft;
			auto* edited = Find_Pattern(candidate, patternId);
			if (edited)
			{
				edited->BossMotion = motion;
				Mark_Draft(candidate, *edited);
				std::string status;
				(void)Commit_Candidate(std::move(candidate), "Updated boss movement interval and positions.", status);
			}
			return;
		}
	}
	if (ImGui::Button(m_bSequenceWorkspace ? "Delete Sequence###Delete Pattern" : "Delete Pattern"))
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
	bool retargetOnEnter = stage->bRetargetOnEnter;
	if (ImGui::Checkbox("Retarget player on enter", &retargetOnEnter))
	{
		std::string status;
		(void)Set_StageRetargetOnEnter(patternId, stageId, retargetOnEnter, status);
		return;
	}
	ImGui::TextDisabled("Choose a living player at Stage entry, then keep that facing.");
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
	ImGui::InputInt("Blend In ms", &m_iOccurrenceBlendInMs, 10, 100);
	m_iOccurrenceBlendInMs = std::clamp(m_iOccurrenceBlendInMs, 0, 1000);
	if (ImGui::Button("Apply Blend In"))
	{
		std::string status;
		(void)Set_AnimationBlend(patternId, occurrenceId, static_cast<std::uint32_t>(m_iOccurrenceBlendInMs), status);
		return;
	}
	ImGui::TextWrapped("%s", m_bSequenceWorkspace ?
		"Blends the previous animation into this box. Compare 0 ms and 100 ms in Preview, then Save." :
		"Blends the previous animation into this box. Compare 0 ms and 100 ms, then Save and Publish for Server playback.");
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
	const char* const popup = m_bSequenceWorkspace ?
		"Discard Sequence draft?###SequenceReloadConfirmation" : "Discard KoukuSaydon composition draft?";
	if (m_bReloadConfirmationRequested)
	{
		ImGui::OpenPopup(popup);
		m_bReloadConfirmationRequested = false;
	}
	if (!ImGui::BeginPopupModal(popup, nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}
	ImGui::TextUnformatted("Reload discards the unsaved composition candidate.");
	if (ImGui::Button("Discard and Reload"))
	{
		const bool_t wasDirty = m_bDirty;
		auto geometry = std::move(m_StagedPresentationGeometry);
		m_StagedPresentationGeometry.clear();
		m_bDirty = false;
		std::string status;
		if (Reload(status))
			ImGui::CloseCurrentPopup();
		else
		{
			m_bDirty = wasDirty;
			m_StagedPresentationGeometry = std::move(geometry);
		}
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
	if (!ImGui::Begin(m_bSequenceWorkspace ? "Sequencer Benchmark###KoukuSequenceWorkbench" :
		"KoukuSaydon Composition###KoukuSaydonActionWorkbench", &m_bOpen))
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
		ImGui::TableSetupColumn(m_bSequenceWorkspace ? "Sequences / Resources" : "Patterns / Resources", ImGuiTableColumnFlags_WidthFixed, 300.f);
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
