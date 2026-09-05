#include "imgui.h"

#include "KoukuSaydonActionWorkbench.h"
#include "CompositionTimeline.h"
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
	/* The Gate 1 boss body. Every profile can be browsed and previewed on its
	   own preview body, but only this profile's clips bind, because projector
	   and Server play the boss with this model. */
	constexpr const char_t* BOSS_BODY_PROFILE_ID = "MN_RPCZ_00";

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
		Find_Pattern(candidate, patternId);
	if (nullptr == pattern || pattern->iNextStageOrdinal >= 1000000u ||
		pattern->iNextAnimationOrdinal >= 1000000u)
	{
		outStatus =
			"Append target Pattern or its stable ID ordinal is unavailable.";
		return false;
	}

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

	m_strSelectedPatternId = std::string(patternId);
	m_strSelectedStageId = stageId;
	m_strSelectedOccurrenceId = occurrenceId;
	Synchronize_EditorFields();
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
	if (source.strProfileId != BOSS_BODY_PROFILE_ID)
	{
		outStatus = std::string("Only ") + BOSS_BODY_PROFILE_ID +
			" clips bind to the Gate 1 boss body; other profiles preview on their own body.";
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
	if (resource.strTargetAssetName != BOSS_BODY_PROFILE_ID ||
		resource.strProfileId != BOSS_BODY_PROFILE_ID)
	{
		outStatus = "KoukuSaydon patterns use MN_RPCZ_00. Other bodies are available for preview.";
		return false;
	}
	const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = "Select an editable KoukuSaydon Pattern to append.";
		return false;
	}
	if (!asNewStage && nullptr == Find_Stage(*pattern, m_strSelectedStageId))
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
	if (profileId != BOSS_BODY_PROFILE_ID)
	{
		outStatus = m_strStatus = std::string("Only ") + BOSS_BODY_PROFILE_ID +
			" actions bind to the Gate 1 boss body.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = m_strStatus = "Append target Pattern is unavailable.";
		return false;
	}
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
	m_strSelectedPatternId = std::string(patternId);
	m_strSelectedStageId = lastStageId;
	m_strSelectedOccurrenceId = lastOccurrenceId;
	Synchronize_EditorFields();
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
	if (profileId != BOSS_BODY_PROFILE_ID)
	{
		outStatus = m_strStatus = std::string("Only ") + BOSS_BODY_PROFILE_ID +
			" actions bind to the Gate 1 boss body.";
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
	m_strSelectedPatternId = std::string(patternId);
	m_strSelectedStageId = targetStageId;
	m_strSelectedOccurrenceId = lastOccurrenceId;
	Synchronize_EditorFields();
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
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (nullptr == pattern)
	{
		m_strSelectedPatternId.clear();
		m_strSelectedStageId.clear();
		m_strSelectedOccurrenceId.clear();
		return;
	}
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
	m_iOccurrenceStartOffsetMs = 0;
	m_iOccurrenceSourceStartMs = 0;
	m_iOccurrencePlayMs = 1;
	m_fOccurrencePlayRate = 1.f;
	m_iOccurrenceEndPolicy = 0;
	const KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern =
		Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (nullptr == pattern)
		return;
	(void)Copy_Text(m_PatternName, std::size(m_PatternName), pattern->strDisplayName);
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
	ImGui::SeparatorText("Patterns");
	if (ImGui::BeginChild("##KoukuPatternList", ImVec2(0.f, 150.f),
		ImGuiChildFlags_Borders))
	{
		if (ImGui::TreeNodeEx("1\xEA\xB4\x80\xEB\xAC\xB8##KoukuGate1", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (const auto& pattern : m_Draft.Patterns)
			{
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
		0u == m_iSelectedResourceActionId ? nullptr :
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
		const bool_t bodyProfile = selectedDocument->strProfileId == BOSS_BODY_PROFILE_ID;
		if (ImGui::Button("Preview Action"))
			Queue_ActionPreview(*selectedDocument, *selectedAction, nullptr);
		ImGui::SameLine();
		ImGui::BeginDisabled(!bodyProfile || !patternEditable);
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
		if (!bodyProfile)
		{
			ImGui::TextDisabled("Only %s clips bind to the Gate 1 boss body; this profile previews on its own body.",
				BOSS_BODY_PROFILE_ID);
		}
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
		const bool_t clipBodyProfile = m_SelectedResource.strProfileId == BOSS_BODY_PROFILE_ID;
		ImGui::BeginDisabled(!clipBodyProfile || !patternEditable);
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
		m_PendingPatternPreview = *pattern;
		m_strPendingPreviewTargetAsset.clear();
		m_bPatternPreviewRequestPending = true;
		m_bPreviewRequestPending = false;
		m_iPendingPreviewStartMs = m_iCursorMs < durationMs ? m_iCursorMs : 0u;
		m_bPendingPreviewStartPaused = false;
		m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
		m_strStatus = "Playing Animation family in the local preview body.";
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
		m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::STOP;
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

void Client::CKoukuSaydonActionWorkbench::Render_Timeline()
{
	const auto* pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (nullptr == pattern)
	{
		ImGui::TextDisabled("Select a Pattern in Gate 1.");
		return;
	}
	if (!pattern->strLoadError.empty())
	{
		ImGui::TextWrapped("%s", pattern->strLoadError.c_str());
		return;
	}
	const auto durationMs = Pattern_DurationMs(*pattern);
	if (m_strCursorPatternId != pattern->strPatternId)
	{
		m_strCursorPatternId = pattern->strPatternId;
		m_iCursorMs = 0u;
	}
	m_iCursorMs = (std::min)(m_iCursorMs, durationMs);
	const bool_t patternPreview = m_PreviewState.bPlaying &&
		m_PreviewState.strPatternId == pattern->strPatternId;
	if (!m_bSharedWorkspaceActive) Render_Transport();

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
	std::size_t rowCount = 0u;
	for (const auto& stage : pattern->Stages) rowCount += stage.AnimationOccurrences.size();
	const f32_t height = rulerHeight + TIMELINE_LANE_HEIGHT * static_cast<f32_t>(2u + rowCount);
	if (!ImGui::BeginChild("##KoukuTimeline", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders,
		ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImGui::EndChild();
		return;
	}
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	ImDrawList* draw = ImGui::GetWindowDrawList();
	CompositionTimeline::DrawRuler(draw, ImVec2(origin.x + labelWidth, origin.y),
		ImVec2(origin.x + timelineWidth, origin.y + rulerHeight), durationMs, m_fPixelsPerSecond);
	/* The ruler is the transport surface: press/drag scrubs a running preview
	   and otherwise places the cursor Play starts from. */
	ImGui::SetCursorScreenPos(ImVec2(origin.x + labelWidth, origin.y));
	ImGui::InvisibleButton("##KoukuRuler",
		ImVec2((std::max)(8.f, durationMs * scale), rulerHeight));
	if (ImGui::IsItemActive() && durationMs > 0u)
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

	std::string editStageId, editOccurrenceId;
	std::uint32_t newOffset = 0u, newSourceStart = 0u, newPlayMs = 0u, newStageDuration = 0u;
	std::uint32_t stageStartMs = 0u;
	std::size_t row = 0u;
	for (const auto& stage : pattern->Stages)
	{
		const f32_t stageX = origin.x + labelWidth + stageStartMs * scale;
		const f32_t stageY = origin.y + rulerHeight;
		const f32_t stageWidth = (std::max)(8.f, stage.iDurationMs * scale);
		ImGui::PushID(stage.strStageId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(stageX, stageY));
		ImGui::InvisibleButton("##StageBox", ImVec2(stageWidth, TIMELINE_LANE_HEIGHT - 2.f));
		if (ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = stage.iDurationMs;
			m_iTimelineDragMode = ImGui::GetIO().MousePos.x > stageX + stageWidth - 7.f ? 3 : 0;
			m_strSelectedStageId = stage.strStageId;
			m_strSelectedOccurrenceId.clear();
			Synchronize_EditorFields();
		}
		f32_t shownStageWidth = stageWidth;
		if ((ImGui::IsItemActive() || ImGui::IsItemDeactivated()) && 3 == m_iTimelineDragMode)
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
			stage.strStageId == m_strSelectedStageId, stage.strStageId.c_str(), false, true);
		ImGui::PopID();

		for (const auto& occurrence : stage.AnimationOccurrences)
		{
			const f32_t y = origin.y + rulerHeight + TIMELINE_LANE_HEIGHT * static_cast<f32_t>(2u + row++);
			const f32_t x = stageX + occurrence.iStartOffsetMs * scale;
			const f32_t width = (std::max)(8.f, occurrence.iPlayMs * scale);
			ImGui::PushID(occurrence.strOccurrenceId.c_str());
			ImGui::SetCursorScreenPos(ImVec2(origin.x + 3.f, y));
			const std::string rowLabel = occurrence.strRuntimeClip + "##row";
			if (ImGui::Selectable(rowLabel.c_str(), m_strSelectedOccurrenceId == occurrence.strOccurrenceId,
				0, ImVec2(labelWidth - 8.f, 22.f)))
			{
				m_strSelectedStageId = stage.strStageId;
				m_strSelectedOccurrenceId = occurrence.strOccurrenceId;
				Synchronize_EditorFields();
			}
			ImGui::SetCursorScreenPos(ImVec2(x, y));
			ImGui::InvisibleButton("##AnimationBox", ImVec2(width, 22.f));
			if (ImGui::IsItemActivated())
			{
				m_iDragOriginOffsetMs = occurrence.iStartOffsetMs;
				m_iDragOriginSourceMs = occurrence.iSourceStartMs;
				m_iDragOriginPlayMs = occurrence.iPlayMs;
				const f32_t mouse = ImGui::GetIO().MousePos.x;
				const auto gesture = CompositionTimeline::HitBoxGesture(mouse, x, x + width, 6.f, true, true);
				m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START ? 1 :
					(gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
				m_strSelectedStageId = stage.strStageId;
				m_strSelectedOccurrenceId = occurrence.strOccurrenceId;
				Synchronize_EditorFields();
			}
			f32_t shownX = x, shownWidth = width;
			if (ImGui::IsItemActive() || ImGui::IsItemDeactivated())
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
				m_strSelectedOccurrenceId == occurrence.strOccurrenceId, occurrence.strRuntimeClip.c_str());
			ImGui::PopID();
		}
		stageStartMs += stage.iDurationMs;
	}
	ImGui::SetCursorScreenPos(origin);
	ImGui::Dummy(ImVec2(timelineWidth, height + 4.f));
	ImGui::EndChild();
	std::string status;
	if (!editStageId.empty())
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
