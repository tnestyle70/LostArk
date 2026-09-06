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
	/* Resources family tabs in display order. Animation and Logic own an
	   authoring flow; the others only name the family until their consumer lands. */
	constexpr std::array<const char_t*, 8u> RESOURCE_CATEGORIES = {
		"Animation", "Logic", "Summon", "Effect", "Collider", "Scene Profile", "Sound", "Camera" };
	constexpr ImU32 TIMELINE_LOGIC_COLOR = IM_COL32(196, 118, 64, 255);
	constexpr ImU32 TIMELINE_SUMMON_COLOR = IM_COL32(88, 156, 116, 255);

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
					if (box.strOnSuccessLogicId == logicId)
						++count;
					if (box.strOnTimeoutLogicId == logicId)
						++count;
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

	bool_t Copy_Text(char_t* const destination, const std::size_t capacity,
		const std::string_view source)
	{
		if (nullptr == destination || 0u == capacity || source.size() >= capacity)
			return false;
		memcpy(destination, source.data(), source.size());
		destination[source.size()] = '\0';
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
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (nullptr == pattern)
		return;
	(void)Copy_Text(m_PatternName, std::size(m_PatternName), pattern->strDisplayName);
	m_iPatternDurationMs = static_cast<int32_t>(Pattern_DurationMs(*pattern));
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
		if (ImGui::TreeNodeEx("1\xEA\xB4\x80\xEB\xAC\xB8##KoukuGate1", ImGuiTreeNodeFlags_DefaultOpen))
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
	if (nullptr == pattern || !pattern->strLoadError.empty() ||
		!std::any_of(pattern->Stages.begin(), pattern->Stages.end(),
			[](const auto& stage) { return !stage.AnimationOccurrences.empty(); }))
	{
		outStatus = m_strStatus = "Select an editable Pattern with animation boxes to play.";
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
	if (nullptr == pattern) ImGui::TextDisabled("Select a Pattern in Gate 1 to play its Animation family.");
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
	if (ImGui::Button("Play Family: Animation"))
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
	if (nullptr == pattern || !pattern->strLoadError.empty() || pattern->Stages.empty())
	{
		outStatus = m_strStatus = "Full lifetime requires an editable Pattern with at least one Stage.";
		return false;
	}
	auto& finalStage = pattern->Stages.back();
	const std::uint64_t earlierMs = Pattern_DurationMs(*pattern) - finalStage.iDurationMs;
	std::uint64_t occupiedEndMs = 1u;
	for (const auto& box : finalStage.AnimationOccurrences)
		occupiedEndMs = (std::max)(occupiedEndMs,
			static_cast<std::uint64_t>(box.iStartOffsetMs) + box.iPlayMs);
	const auto minimumMs = (std::max)(earlierMs + occupiedEndMs,
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
	const bool_t hasAnimation = patternReady && std::any_of(
		pattern->Stages.begin(), pattern->Stages.end(),
		[](const auto& stage) { return !stage.AnimationOccurrences.empty(); });
	const bool_t publishing = Is_PublishRunning();
	ImGui::BeginDisabled(publishing || !m_bHasDraft || !m_bDirty || !m_Document.Is_Fresh());
	const bool_t saveRequested = ImGui::Button("Save##KoukuSequencer");
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!hasAnimation);
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
	const bool_t durationEditable = !publishing && patternReady && !pattern->Stages.empty();
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
		!m_strSelectedSummonOccurrenceId.empty();
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
	ImGui::TextDisabled("%zu Stages + %zu animation boxes%s",
		m_TimelineSelectedStageIds.size(), m_TimelineSelectedOccurrenceIds.size(),
		m_strSelectedLogicOccurrenceId.empty() ?
			(m_strSelectedSummonOccurrenceId.empty() ? "" : " + 1 summon box") : " + 1 logic box");
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
		if (nullptr == pattern) ImGui::TextDisabled("Select a Pattern in Gate 1.");
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
	// Stages lane + animation rows + one Logic lane + one Summon lane.
	const f32_t height = rulerHeight + TIMELINE_LANE_HEIGHT * static_cast<f32_t>(3u + rowCount);
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
		if (const auto* const successLogic = Find_Logic(m_Draft, box.strOnSuccessLogicId);
			nullptr != successLogic)
			label += "  ok>" + successLogic->strDisplayName;
		if (const auto* const timeoutLogic = Find_Logic(m_Draft, box.strOnTimeoutLogicId);
			nullptr != timeoutLogic)
			label += "  fail>" + timeoutLogic->strDisplayName;
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
	if (deleteRequested)
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
	const bool_t wasProduct = "PRODUCT" == pattern->strAuthoringStatus;
	pattern->LogicOccurrences.push_back(std::move(box));
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			wasProduct ?
				"Appended Logic box and returned the Pattern to DRAFT; Logic has no Server consumer yet." :
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
	return Commit_Candidate(std::move(candidate), "Updated Logic box window.", outStatus);
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

bool_t Client::CKoukuSaydonActionWorkbench::Set_LogicBoxOutcome(
	const std::string_view patternId,
	const std::string_view occurrenceId,
	const bool_t success,
	const std::string_view resultLogicId,
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
	if (nullptr == owner || "DURATION" != owner->strLogicType)
	{
		outStatus = m_strStatus = "Only a DURATION Logic box owns success and timeout outcomes.";
		return false;
	}
	if (!resultLogicId.empty())
	{
		const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION* const result = Find_Logic(candidate, resultLogicId);
		if (nullptr == result || "RESULT" != result->strLogicType)
		{
			outStatus = m_strStatus = "An outcome must name an existing RESULT Logic.";
			return false;
		}
	}
	std::string& slot = success ? box->strOnSuccessLogicId : box->strOnTimeoutLogicId;
	if (slot == resultLogicId)
	{
		outStatus = m_strStatus = "Logic box outcome is unchanged.";
		return true;
	}
	slot = std::string(resultLogicId);
	return Commit_Candidate(std::move(candidate),
		success ? "Wired the success outcome. Press Save to keep it." :
			"Wired the timeout outcome. Press Save to keep it.",
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
	const bool_t wasProduct = "PRODUCT" == pattern->strAuthoringStatus;
	pattern->SummonOccurrences.push_back(std::move(box));
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			wasProduct ?
				"Appended Summon box and returned the Pattern to DRAFT; Summon has no Server consumer yet." :
				"Appended Summon box. Its lifetime ends where the spawn despawns; drag the edges on the Summon lane.",
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
	if (nullptr != logic && "DURATION" == logic->strLogicType)
	{
		/* Success and timeout each pick one RESULT Logic from the catalog. The
		   combo lists only RESULT definitions, so a window can never end in a
		   trigger or another window. */
		ImGui::SeparatorText("Outcomes");
		const std::string successId = box->strOnSuccessLogicId;
		const std::string timeoutId = box->strOnTimeoutLogicId;
		const auto renderOutcome = [this, &patternId, &occurrenceId](
			const char_t* const label, const bool_t success,
			const std::string& currentId) -> bool_t
		{
			const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION* const current =
				currentId.empty() ? nullptr : Find_Logic(m_Draft, currentId);
			const char_t* const preview = currentId.empty() ? "(none)" :
				(nullptr == current ? "(missing RESULT)" : current->strDisplayName.c_str());
			std::string requestedId;
			bool_t requested = false;
			if (ImGui::BeginCombo(label, preview))
			{
				if (ImGui::Selectable("(none)", currentId.empty()) && !currentId.empty())
					requested = true;
				for (const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& candidate : m_Draft.Logics)
				{
					if ("RESULT" != candidate.strLogicType)
						continue;
					const std::string item = candidate.strDisplayName + "##" + candidate.strLogicId;
					if (ImGui::Selectable(item.c_str(), currentId == candidate.strLogicId) &&
						currentId != candidate.strLogicId)
					{
						requestedId = candidate.strLogicId;
						requested = true;
					}
				}
				ImGui::EndCombo();
			}
			if (!requested)
				return false;
			std::string status;
			(void)Set_LogicBoxOutcome(patternId, occurrenceId, success, requestedId, status);
			return true;
		};
		if (renderOutcome("Success##KoukuLogicOutcome", true, successId))
			return;
		if (renderOutcome("Timeout##KoukuLogicOutcome", false, timeoutId))
			return;
		if (m_Draft.Logics.end() == std::find_if(m_Draft.Logics.begin(), m_Draft.Logics.end(),
				[](const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& candidate)
				{ return "RESULT" == candidate.strLogicType; }))
		{
			ImGui::TextDisabled("Create a RESULT Logic in Resources > Logic to wire an outcome.");
		}
	}
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
	if (ImGui::Button("Delete Pattern"))
	{
		std::string status;
		(void)Delete_Pattern(patternId, status);
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
