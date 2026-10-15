// Stage/clip의 원본 참조와 시간 편집을 담당한다. 변경 후보는 공용 검증·확정 경계인 Commit_Candidate로 넘긴다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"

// 후보 값 복사, stable ID 조회, 시간/좌표 계산에 필요한 표준 타입이다.
#include <algorithm>
#include <cmath>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <string>
#include <string_view>
#include <vector>

using namespace Client::KoukuWorkbenchDetail;

namespace
{
using namespace Client;

// Stage 편집 후보에서 pose 시작 시각을 누적 ms로 다시 계산한다. blend가 같은 두 pose를 잇는지 재검증하며 실패 후보는 호출자가 버린다.
bool_t Retime_AnimationBlendWindows(const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document,
	const KOUKU_SAYDON_COMPOSITION_PATTERN &previous, KOUKU_SAYDON_COMPOSITION_PATTERN &candidate, std::string &status,
	const bool_t clampToPoseSpan = false)
{
	std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> previousWindows;
	if (!CKoukuSaydonCompositionDocument::Try_ResolveAnimationBlendWindows(document, previous, previousWindows, status))
		return false;
	if (previousWindows.empty())
		return true;
	// Stage 경계가 이동해도 blend의 두 pose owner는 유지되어야 한다.
	// Stage vector 순서로 origin을 누적하고 clip의 시작 offset(동률이면 ID)으로 첫 pose를 고른다. 첫 pose는 Stage 시작에 맞추고 나머지는 offset을 더한다.
	// 이 임시 map은 occurrence ID로 계산된 시작 시각을 찾는 데 쓰며 map의 순회 순서가 재생 순서가 되지 않는다.
	std::unordered_map<std::string, std::uint64_t> poseStarts;
	std::uint64_t origin = 0u;
	for (const auto &stage : candidate.Stages)
	{
		const auto first = std::min_element(
			stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(), [](const auto &a, const auto &b) {
				return a.iStartOffsetMs != b.iStartOffsetMs ? a.iStartOffsetMs < b.iStartOffsetMs
															: a.strOccurrenceId < b.strOccurrenceId;
			});
		for (const auto &animation : stage.AnimationOccurrences)
			poseStarts.emplace(animation.strOccurrenceId,
				origin + (animation.strOccurrenceId == first->strOccurrenceId ? 0u : animation.iStartOffsetMs));
		origin += stage.iDurationMs;
	}
	for (const auto &window : previousWindows)
	{
		const auto target = poseStarts.find(window.Target.strOccurrenceId);
		const auto box =
			std::find_if(candidate.LogicOccurrences.begin(), candidate.LogicOccurrences.end(), [&](const auto &row) {
				return row.strOccurrenceId == window.strLogicOccurrenceId;
			});
		if (target == poseStarts.end() || box == candidate.LogicOccurrences.end())
		{
			status = "Animation blend retime lost its pose owner or Logic box.";
			return false;
		}
		std::int64_t start = std::int64_t(window.iStartMs) + std::int64_t(target->second) - window.Target.iPoseStartMs;
		std::int64_t end = start + window.iDurationMs;
		if (clampToPoseSpan)
		{
			const auto source = poseStarts.find(window.Source.strOccurrenceId);
			if (source == poseStarts.end() || source->second >= target->second)
			{
				status = "Stage resize cannot keep the Animation Blend's distinct pose owners.";
				return false;
			}
			std::uint64_t limit = candidate.iDurationMs ? candidate.iDurationMs : origin;
			std::uint64_t sourceLimit = source->second;
			for (const auto &[id, time] : poseStarts)
			{
				if (time > target->second)
					limit = (std::min)(limit, time - 1u);
				// 첫 pose가 아닌 source는 자체 전환점이다. 양끝을 포함하는 이 구간에서
				// source를 제외해 target 전환만 들어오게 한다.
				if (time < source->second)
					sourceLimit = source->second + 1u;
			}
			start = (std::max)(start, std::int64_t(sourceLimit));
			end = (std::min)(end, std::int64_t(limit));
			if (start >= end)
			{
				status = "Stage resize leaves no time for the existing Animation Blend.";
				return false;
			}
		}
		if (start < 0 || end > MAX_EDITOR_TIME_MS)
		{
			status = "Animation blend retime would leave the 0..600000 ms timeline.";
			return false;
		}
		box->iStartMs = static_cast<std::uint32_t>(start);
		box->iDurationMs = static_cast<std::uint32_t>(end - start);
	}
	std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> nextWindows;
	if (!CKoukuSaydonCompositionDocument::Try_ResolveAnimationBlendWindows(document, candidate, nextWindows, status))
		return false;
	for (const auto &previousWindow : previousWindows)
	{
		const auto next = std::find_if(nextWindows.begin(), nextWindows.end(), [&](const auto &window) {
			return window.strLogicOccurrenceId == previousWindow.strLogicOccurrenceId;
		});
		if (next == nextWindows.end() || next->Source.strOccurrenceId != previousWindow.Source.strOccurrenceId ||
			next->Target.strOccurrenceId != previousWindow.Target.strOccurrenceId)
		{
			status = "Animation blend retime would change its consecutive pose owners.";
			return false;
		}
	}
	return true;
}

// 아래 원본 조회 함수들은 reference catalog를 차용한다. document/action/stage/slot의 stable 키를 따라가며
// 없는 키는 nullptr로 알린다. catalog 재로드 후에는 이전 포인터를 재사용하지 않는다.
const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT *Find_Reference(
	const KOUKU_SAYDON_ACTION_REFERENCE_SET &references, const std::string_view profileId)
{
	const auto found = std::find_if(references.Documents.begin(), references.Documents.end(),
		[profileId](const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT &document) {
			return document.strProfileId == profileId;
		});
	return found == references.Documents.end() ? nullptr : &*found;
}

const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE *Find_SourceSlot(const KOUKU_SAYDON_ACTION_REFERENCE_SET &references,
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &source,
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT **const ppReference = nullptr)
{
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT *const reference =
		Find_Reference(references, source.strProfileId);
	if (nullptr == reference)
		return nullptr;
	const auto action = std::find_if(reference->Actions.begin(), reference->Actions.end(),
		[&source](const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE &value) {
			return value.iSourceActionId == source.iSourceActionId;
		});
	if (action == reference->Actions.end())
	{
		return nullptr;
	}
	const auto stage = std::find_if(action->Stages.begin(), action->Stages.end(),
		[&source](const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE &value) {
			return value.strStageId == source.strSourceStageId;
		});
	if (stage == action->Stages.end())
		return nullptr;
	const auto slot = std::find_if(
		stage->Slots.begin(), stage->Slots.end(), [&source](const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE &value) {
			return value.strSlotId == source.strSourceSlotId;
		});
	if (slot == stage->Slots.end())
		return nullptr;
	if (nullptr != ppReference)
		*ppReference = reference;
	return &*slot;
}

// append 대상은 후보 문서에 이미 존재하는 편집 가능한 Pattern이어야 한다.
// 선택 ID와 원본 actor가 일치할 때만 차용 포인터를 반환하며 문서나 영구 ID를 만들지 않는다.
KOUKU_SAYDON_COMPOSITION_PATTERN *Find_AppendPattern(KOUKU_SAYDON_COMPOSITION_DOCUMENT &candidate,
	const std::string_view patternId, const std::string_view sourceProfileId, const std::string_view displayName,
	std::string &outStatus)
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
	auto *pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = "Select an editable Pattern for this model.";
		return nullptr;
	}
	if (pattern->strActorProfileId != actor)
	{
		outStatus =
			"Action belongs to " + std::string(Actor_Label(actor)) + ". Select that model's Pattern before appending.";
		return nullptr;
	}
	return pattern;
}

const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE *Find_Action(
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT &reference, const std::uint32_t sourceActionId)
{
	const auto found = std::find_if(reference.Actions.begin(), reference.Actions.end(),
		[sourceActionId](const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE &action) {
			return action.iSourceActionId == sourceActionId;
		});
	return found == reference.Actions.end() ? nullptr : &*found;
}

// 원본 slot의 출처·revision·재생 구간을 편집 가능한 값으로 복사한다. occurrence ID와 Stage 내 위치는
// Preview/Append 호출자가 목적지에 맞게 채우므로 여기서는 원본 catalog를 변경하지 않는다.
KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE Build_ReferenceOccurrence(
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT &reference,
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE &action, const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE &stage,
	const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE &slot)
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
} // 이 파일의 보조 구현 범위 끝

// Detail의 Stage 추가를 후보 문서에서 처리한다. ordinal도 후보에서만 소비하고 lifetime 검증 뒤 commit하므로
// 실패하면 기존 draft와 출력 ID를 유지한다. 성공 뒤에는 값으로 보관한 ID로 선택을 다시 맞춘다.
bool_t Client::CKoukuSaydonActionWorkbench::Add_Stage(const std::string_view patternId,
	const std::string_view stageKind, const std::uint32_t durationMs, std::string &outStageId, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
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
	if (!Extend_PatternLifetimeForAuthoredLanes(*pattern, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Added KoukuSaydon Stage " + stageId + ".", outStatus))
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
	const std::string_view patternId, const std::string_view stageId, std::string &outStatus)
{
	// Detail과 timeline Delete가 같은 lane/이동 시계 절단 규칙을 사용한다.
	return Delete_TimelineSelection(patternId, {std::string(stageId)}, {}, outStatus);
}

// 리소스 추가 명령은 원본 clip 사용 가능 여부와 선택 Pattern의 actor를 먼저 확인한다. 하나의 Stage와
// clip을 후보에 만들고 길이·종료 정책을 맞춘 뒤 commit하며, 성공한 새 ID만 선택과 출력에 넘긴다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_AnimationAsStage(const std::string_view patternId,
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &source, std::string &outStageId, std::string &outOccurrenceId,
	std::string &outStatus)
{
	if (!Is_AppendAdmitted(source, outStatus))
		return false;

	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern =
		Find_AppendPattern(candidate, patternId, source.strProfileId, source.strRuntimeClip, outStatus);
	if (nullptr == pattern || pattern->iNextStageOrdinal >= 1000000u || pattern->iNextAnimationOrdinal >= 1000000u)
	{
		outStatus = "Append target Pattern or its stable ID ordinal is unavailable.";
		return false;
	}

	const std::string targetPatternId = pattern->strPatternId;
	const std::uint32_t stageOrdinal = pattern->iNextStageOrdinal++;
	const std::uint32_t animationOrdinal = pattern->iNextAnimationOrdinal++;
	KOUKU_SAYDON_COMPOSITION_STAGE stage;
	stage.strStageId = "STAGE_" + std::to_string(stageOrdinal);
	stage.strActionId = pattern->strPatternId + ".stage." + std::to_string(stageOrdinal);
	stage.strStageKind = "ACTIVE";
	stage.iDurationMs = source.iPlayMs;

	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence = source;
	occurrence.strOccurrenceId = pattern->strPatternId + ".animation." + std::to_string(animationOrdinal);
	occurrence.iStartOffsetMs = 0u;
	std::string policyNote;
	(void)Normalize_EndPolicyForWindow(occurrence, policyNote);

	const std::string stageId = stage.strStageId;
	const std::string occurrenceId = occurrence.strOccurrenceId;
	stage.AnimationOccurrences.push_back(std::move(occurrence));
	pattern->Stages.push_back(std::move(stage));
	if (!Extend_PatternLifetimeForAuthoredLanes(*pattern, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(
			std::move(candidate), "Appended boss animation as Stage " + stageId + "." + policyNote, outStatus))
	{
		return false;
	}

	Clear_TimelineSelection();
	m_bFitRequested = true;
	m_strSelectedActorProfileId =
		std::string(CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(source.strProfileId));
	m_strSelectedPatternId = targetPatternId;
	Select_TimelineBox(stageId, occurrenceId, false);
	outStageId = stageId;
	outOccurrenceId = occurrenceId;
	return true;
}

// 단일 Stage를 바로 앞/뒤 이웃과 교환한다. clip의 Stage 내부 offset과 다른 lane의 절대 시간은 여기서
// 이동시키지 않으며, 후보 전체의 유효성은 Commit_Candidate가 확인한다.
bool_t Client::CKoukuSaydonActionWorkbench::Move_Stage(
	const std::string_view patternId, const std::string_view stageId, const int32_t direction, std::string &outStatus)
{
	if (0 == direction)
	{
		outStatus = "KoukuSaydon Stage move direction is zero.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = "KoukuSaydon Stage move Pattern is absent.";
		return false;
	}
	const auto found = std::find_if(
		pattern->Stages.begin(), pattern->Stages.end(), [stageId](const KOUKU_SAYDON_COMPOSITION_STAGE &stage) {
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

// 다중 선택에 포함된 clip은 소유 Stage로 승격해 함께 이동한다. 경계에 닿은 선택은 전체 거절하고
// 성공 시 같은 stable ID 집합을 유지해 이동 후에도 다중 선택이 단일 박스로 축소되지 않게 한다.
bool_t Client::CKoukuSaydonActionWorkbench::Move_SelectedStage(
	const std::string_view patternId, const int32_t direction, std::string &outStatus)
{
	if (0 == direction || (m_TimelineSelectedStageIds.empty() && m_TimelineSelectedOccurrenceIds.empty()))
	{
		outStatus = m_strStatus = "Select Stage or animation boxes and a direction to move.";
		return false;
	}
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	if (!pattern || !pattern->strLoadError.empty() || m_strTimelineSelectionPatternId != patternId)
	{
		outStatus = m_strStatus = "Move selection target Pattern is unavailable or invalid.";
		return false;
	}
	std::unordered_set<std::string> selectedStages;
	for (const auto &id : m_TimelineSelectedStageIds)
	{
		if (!Find_Stage(*pattern, id))
		{
			outStatus = m_strStatus = "Move rejected; Stage is unavailable: " + id;
			return false;
		}
		selectedStages.insert(id);
	}
	for (const auto &id : m_TimelineSelectedOccurrenceIds)
	{
		const auto occurrence = Find_Occurrence(*pattern, id);
		if (!occurrence.pStage)
		{
			outStatus = m_strStatus = "Move rejected; animation box is unavailable: " + id;
			return false;
		}
		selectedStages.insert(occurrence.pStage->strStageId);
	}
	auto &stages = pattern->Stages;
	if (stages.empty() || selectedStages.contains(direction < 0 ? stages.front().strStageId : stages.back().strStageId))
	{
		outStatus = m_strStatus = "Selected Stages are already at that edge; selection preserved.";
		return false;
	}
	// 이동 방향으로 순회하여 연속 선택 구간이 미선택 이웃 하나를 건너게 한다.
	// 선택 구간 내부의 순서는 바꾸지 않는다.
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
	// Commit이 기존 stable ID 선택을 정규화한다. 묶음 이동 후 단일 박스 선택
	// helper를 호출하면 다중 선택이 사라지므로 그대로 유지한다.
	return Commit_Candidate(std::move(candidate),
		direction < 0 ? "Moved selected Stages one slot earlier with their animation boxes. Press Save."
					  : "Moved selected Stages one slot later with their animation boxes. Press Save.",
		outStatus);
}

// 첫 Stage의 가장 이른 clip 시작을 입력 ms로 옮기고 같은 Stage의 clip 및 Stage 길이에 같은 차이를 더한다.
// 다른 lane의 절대 시간은 유지하되 Animation Blend는 원래 두 pose를 연결하도록 재시간화한 후보만 확정한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_PatternStartOffset(
	const std::string_view patternId, const std::uint32_t startOffsetMs, std::string &outStatus)
{
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	if (!pattern || pattern->Stages.empty() || pattern->Stages.front().AnimationOccurrences.empty())
	{
		outStatus = m_strStatus = "Pattern Start Offset needs an animation in the first Stage.";
		return false;
	}
	const auto previous = *pattern;
	auto &firstStage = pattern->Stages.front();
	const auto first = std::min_element(firstStage.AnimationOccurrences.begin(), firstStage.AnimationOccurrences.end(),
		[](const auto &a, const auto &b) {
			return a.iStartOffsetMs < b.iStartOffsetMs;
		});
	const std::int64_t delta = std::int64_t(startOffsetMs) - first->iStartOffsetMs;
	const std::int64_t stageDuration = std::int64_t(firstStage.iDurationMs) + delta;
	const std::int64_t patternDuration = std::int64_t(Pattern_DurationMs(*pattern)) + delta;
	if (startOffsetMs > MAX_EDITOR_TIME_MS || stageDuration < 1 || stageDuration > MAX_EDITOR_TIME_MS ||
		patternDuration > MAX_EDITOR_TIME_MS)
	{
		outStatus = m_strStatus = "Pattern Start Offset would exceed the 600-second timeline.";
		return false;
	}
	if (delta == 0)
	{
		outStatus = m_strStatus = "Pattern Start Offset is unchanged.";
		return true;
	}
	for (auto &animation : firstStage.AnimationOccurrences)
		animation.iStartOffsetMs = static_cast<std::uint32_t>(std::int64_t(animation.iStartOffsetMs) + delta);
	firstStage.iDurationMs = static_cast<std::uint32_t>(stageDuration);
	if (!Retime_AnimationBlendWindows(m_Draft, previous, *pattern, outStatus))
	{
		outStatus = m_strStatus = "KoukuSaydon edit rejected; draft preserved: " + outStatus;
		return false;
	}
	if (!Extend_PatternLifetimeForAuthoredLanes(*pattern, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate),
		"Changed Pattern Start Offset and kept Animation Blend boxes with their transitions. Other lane times are preserved. Press Save.",
		outStatus);
}

// Stage 길이는 1..600000 ms이며 clip의 offset·재생 길이·blend 길이는 새 Stage 안으로 줄인다.
// 다른 lane의 끝은 그대로 두고 Pattern lifetime에 포함한다. 범위 실패는 후보만 버리고 draft를 보존한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_StageDuration(const std::string_view patternId,
	const std::string_view stageId, const std::uint32_t durationMs, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_STAGE *const stage = nullptr == pattern ? nullptr : Find_Stage(*pattern, stageId);
	if (nullptr == stage)
	{
		outStatus = "KoukuSaydon Stage duration target is absent.";
		return false;
	}
	const auto reject = [&](const std::string &reason) {
		outStatus = m_strStatus = "KoukuSaydon edit rejected; draft preserved: " + reason;
		return false;
	};
	if (!durationMs || durationMs > MAX_EDITOR_TIME_MS)
		return reject("Stage duration must be 1..600000 ms.");
	std::uint64_t totalMs = 0u;
	for (const auto &row : pattern->Stages)
		totalMs += row.iDurationMs;
	const auto deltaMs = std::int64_t(durationMs) - stage->iDurationMs;
	if (std::int64_t(totalMs) + deltaMs > MAX_EDITOR_TIME_MS)
		return reject("Stage resize would exceed the 600000 ms Pattern clock.");
	stage->iDurationMs = durationMs;
	for (auto &animation : stage->AnimationOccurrences)
	{
		animation.iStartOffsetMs = (std::min)(animation.iStartOffsetMs, durationMs - 1u);
		animation.iPlayMs = (std::min)(animation.iPlayMs, durationMs - animation.iStartOffsetMs);
		animation.iBlendInMs = (std::min)(animation.iBlendInMs, animation.iPlayMs);
	}
	if (!Extend_PatternLifetimeForAuthoredLanes(*pattern, outStatus))
		return reject(outStatus);
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate),
		"Changed Stage duration. Effect and other lane times are preserved; Pattern lifetime includes their tails. Press Save.",
		outStatus);
}

// Stage 속성 입력은 선택한 후보 행에만 적용한다. 아래 retarget 설정도 같은 commit 경계를 사용하며
// retarget을 끌 때는 남아 있던 대상 override까지 제거해 비활성 설정이 다시 사용되지 않게 한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_StageKind(const std::string_view patternId,
	const std::string_view stageId, const std::string_view stageKind, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_STAGE *const stage = nullptr == pattern ? nullptr : Find_Stage(*pattern, stageId);
	if (nullptr == stage)
	{
		outStatus = "KoukuSaydon Stage kind target is absent.";
		return false;
	}
	stage->strStageKind = std::string(stageKind);
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Changed KoukuSaydon Stage kind.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_StageRetargetOnEnter(const std::string_view patternId,
	const std::string_view stageId, const bool_t retargetOnEnter, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_STAGE *const stage = nullptr == pattern ? nullptr : Find_Stage(*pattern, stageId);
	if (nullptr == stage)
	{
		outStatus = "KoukuSaydon Stage retarget target is absent.";
		return false;
	}
	stage->bRetargetOnEnter = retargetOnEnter;
	if (!retargetOnEnter)
		stage->RetargetTarget.reset();
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Changed Stage entry retarget. Press Save.", outStatus);
}

// 선택 Stage에 원본 clip의 값 사본을 추가한다. startOffsetMs는 Stage 시작 기준이며 필요한 Stage·Pattern
// 길이를 후보에서 늘린다. vector에 추가한 뒤 기존 occurrence 포인터를 보관하지 않고 새 stable ID로 결과를 전달한다.
bool_t Client::CKoukuSaydonActionWorkbench::Bind_Animation(const std::string_view patternId,
	const std::string_view targetStageId, const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &source,
	const std::uint32_t startOffsetMs, std::string &outOccurrenceId, std::string &outStatus)
{
	if (!Is_AppendAdmitted(source, outStatus))
		return false;

	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_STAGE *const stage = nullptr == pattern ? nullptr : Find_Stage(*pattern, targetStageId);
	if (nullptr == stage || pattern->iNextAnimationOrdinal >= 1000000u)
	{
		outStatus = "KoukuSaydon animation Bind target or ordinal is invalid.";
		return false;
	}
	const std::string occurrenceId =
		pattern->strPatternId + ".animation." + std::to_string(pattern->iNextAnimationOrdinal++);
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence = source;
	occurrence.strOccurrenceId = occurrenceId;
	occurrence.iStartOffsetMs = startOffsetMs;
	std::string policyNote;
	(void)Normalize_EndPolicyForWindow(occurrence, policyNote);
	stage->iDurationMs = (std::max)(stage->iDurationMs, startOffsetMs + occurrence.iPlayMs);
	stage->AnimationOccurrences.push_back(std::move(occurrence));
	if (!Extend_PatternLifetimeForAuthoredLanes(*pattern, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(
			std::move(candidate), "Bound KoukuSaydon animation " + occurrenceId + "." + policyNote, outStatus))
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

// clip 이동은 소유 Stage 기준 offset만 바꾼다. 범위를 임의 보정하지 않으므로 Stage를 벗어난 후보의
// 수용 여부는 공용 검증 경계에서 결정하며 실패한 후보가 현재 draft를 덮어쓰지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Move_Animation(const std::string_view patternId,
	const std::string_view occurrenceId, const std::uint32_t startOffsetMs, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	const MUTABLE_OCCURRENCE found =
		nullptr == pattern ? MUTABLE_OCCURRENCE{} : Find_Occurrence(*pattern, occurrenceId);
	if (nullptr == found.pOccurrence)
	{
		outStatus = "KoukuSaydon animation move target is absent.";
		return false;
	}
	found.pOccurrence->iStartOffsetMs = startOffsetMs;
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Moved KoukuSaydon animation box.", outStatus);
}

// Source Start/End는 원본 clip 시간이고 Play는 타임라인 재생 창 길이(ms)다. 원본 메타데이터로 잘린 구간을
// 검사한 뒤 EXACT가 원본 끝을 넘는 경우 종료 정책을 보정한다. 입력 실패는 현재 clip을 유지한다.
bool_t Client::CKoukuSaydonActionWorkbench::Trim_Animation(const std::string_view patternId,
	const std::string_view occurrenceId, const std::uint32_t sourceStartMs, const std::uint32_t playMs,
	std::string &outStatus, const std::uint32_t sourceEndMs)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	const MUTABLE_OCCURRENCE found =
		nullptr == pattern ? MUTABLE_OCCURRENCE{} : Find_Occurrence(*pattern, occurrenceId);
	if (nullptr == found.pOccurrence)
	{
		outStatus = "KoukuSaydon animation trim target is absent.";
		return false;
	}
	found.pOccurrence->iSourceStartMs = sourceStartMs;
	found.pOccurrence->iSourceEndMs = sourceEndMs;
	found.pOccurrence->iPlayMs = playMs;
	if (!Validate_SourceStart(*found.pOccurrence, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	std::string policyNote;
	(void)Normalize_EndPolicyForWindow(*found.pOccurrence, policyNote);
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Trimmed KoukuSaydon animation box." + policyNote, outStatus);
}

// 복제할 clip을 먼저 값으로 복사한 뒤 같은 Stage vector에 새 ID로 추가한다. push_back 뒤에는 이전
// occurrence 포인터를 읽지 않으며 commit 성공 후 보관한 새 ID를 선택한다.
bool_t Client::CKoukuSaydonActionWorkbench::Duplicate_Animation(const std::string_view patternId,
	const std::string_view occurrenceId, std::string &outOccurrenceId, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	const MUTABLE_OCCURRENCE found =
		nullptr == pattern ? MUTABLE_OCCURRENCE{} : Find_Occurrence(*pattern, occurrenceId);
	if (nullptr == found.pOccurrence || pattern->iNextAnimationOrdinal >= 1000000u)
	{
		outStatus = "KoukuSaydon animation duplicate target or ordinal is invalid.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE duplicate = *found.pOccurrence;
	duplicate.strOccurrenceId =
		pattern->strPatternId + ".animation." + std::to_string(pattern->iNextAnimationOrdinal++);
	const std::string duplicateId = duplicate.strOccurrenceId;
	found.pStage->AnimationOccurrences.push_back(std::move(duplicate));
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Duplicated KoukuSaydon animation box " + duplicateId + ".", outStatus))
	{
		return false;
	}
	m_strSelectedOccurrenceId = duplicateId;
	Synchronize_EditorFields();
	outOccurrenceId = duplicateId;
	return true;
}

// 단일 clip 삭제는 후보 Stage의 해당 행만 제거한다. 남은 참조의 유효성은 commit 검증이 판단하며
// 성공 후에만 선택을 정리한다. Stage 전체 삭제는 별도 Delete_TimelineSelection의 시간 절단 규칙을 사용한다.
bool_t Client::CKoukuSaydonActionWorkbench::Delete_Animation(
	const std::string_view patternId, const std::string_view occurrenceId, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
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
	const auto eraseAt =
		std::find_if(found.pStage->AnimationOccurrences.begin(), found.pStage->AnimationOccurrences.end(),
			[occurrenceId](const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &occurrence) {
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

// 원본 clip을 값으로 확보한 후 원래 vector에서 제거하고 목적지 Stage에 삽입한다. offset은 목적지 기준이며
// 같은 Stage로 이동해도 제거된 occurrence 포인터를 재사용하지 않는다. 후보 확정 후 선택 Stage를 맞춘다.
bool_t Client::CKoukuSaydonActionWorkbench::Move_AnimationToStage(const std::string_view patternId,
	const std::string_view occurrenceId, const std::string_view targetStageId, const std::uint32_t startOffsetMs,
	std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = "KoukuSaydon animation Stage-move Pattern is absent.";
		return false;
	}
	MUTABLE_OCCURRENCE found = Find_Occurrence(*pattern, occurrenceId);
	KOUKU_SAYDON_COMPOSITION_STAGE *const target = Find_Stage(*pattern, targetStageId);
	if (nullptr == found.pOccurrence || nullptr == found.pStage || nullptr == target)
	{
		outStatus = "KoukuSaydon animation Stage-move source or target is absent.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE moved = *found.pOccurrence;
	moved.iStartOffsetMs = startOffsetMs;
	const auto eraseAt =
		std::find_if(found.pStage->AnimationOccurrences.begin(), found.pStage->AnimationOccurrences.end(),
			[occurrenceId](const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &occurrence) {
				return occurrence.strOccurrenceId == occurrenceId;
			});
	found.pStage->AnimationOccurrences.erase(eraseAt);
	target->AnimationOccurrences.push_back(std::move(moved));
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Moved KoukuSaydon animation box to Stage " + std::string(targetStageId) + ".", outStatus))
	{
		return false;
	}
	m_strSelectedStageId = std::string(targetStageId);
	return true;
}

// Preview 호스트가 대기 중인 transport와 seek ms를 한 번 소비한다. 명령이 없으면 출력 인자를 건드리지
// 않고, 소비 성공 시 요청·seek·명시적 reset 상태를 비워 같은 명령이 다음 프레임에 반복되지 않게 한다.
bool_t Client::CKoukuSaydonActionWorkbench::Consume_PreviewTransportRequest(
	KOUKU_PREVIEW_TRANSPORT &outTransport, std::uint32_t &outSeekMs)
{
	if (KOUKU_PREVIEW_TRANSPORT::NONE == m_ePendingTransport)
		return false;
	outTransport = m_ePendingTransport;
	outSeekMs = m_iPendingSeekMs;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
	m_iPendingSeekMs = 0u;
	m_bExplicitPreviewReset = false;
	return true;
}

// RAW clip은 현재 모델 목록, 추출 clip은 reference slot에서 존재와 clip 이름을 대조한다.
// Workbench가 아는 profile만 허용하며 검사 과정은 선택이나 draft를 변경하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Is_AppendAdmitted(
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &source, std::string &outStatus) const
{
	const bool raw = 0u == source.iSourceActionId && "RAW" == source.strSourceStageId;
	const auto *sourceSlot = Find_SourceSlot(m_ResourceReferences, source);
	const bool available = raw
		? std::any_of(m_ModelResources.begin(), m_ModelResources.end(),
			  [&](const auto &item) {
				  return item.strRuntimeClip == source.strRuntimeClip && item.strProfileId == source.strProfileId;
			  })
		: (nullptr != sourceSlot && sourceSlot->strRuntimeClip == source.strRuntimeClip);
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
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &occurrence, std::uint32_t &outNativeMs) const
{
	/* 원본 clip 길이는 모델 메타데이터에서만 가져온다. reference slot의 길이는
	   저작자가 잘라 둔 재생 구간이므로 원본 clip의 전체 길이를 대신할 수 없다. */
	const auto found = std::find_if(
		m_ModelResources.begin(), m_ModelResources.end(), [&occurrence](const COMPOSITION_ANIMATION_RESOURCE &item) {
			return item.strProfileId == occurrence.strProfileId && item.strRuntimeClip == occurrence.strRuntimeClip;
		});
	if (found == m_ModelResources.end() || 0u == found->iDurationMs)
		return false;
	outNativeMs = found->iDurationMs;
	return true;
}

// 잘라 쓸 구간을 원본 clip 길이와 비교한다. HOLD_LAST_POSE는 시작이 원본 끝을 넘는 held tail을 허용하며
// Source End가 지정됐으면 시작보다 뒤인지도 검사한다. 원본 길이가 없을 때의 기존 부분 검증 범위를 유지한다.
bool_t Client::CKoukuSaydonActionWorkbench::Validate_SourceStart(
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &occurrence, std::string &outStatus) const
{
	std::uint32_t nativeMs = 0u;
	if ("HOLD_LAST_POSE" != occurrence.strEndPolicy && Resolve_NativeClipMs(occurrence, nativeMs) &&
		occurrence.iSourceStartMs >= nativeMs)
	{
		outStatus = "Source Start must be inside the native clip (" + std::to_string(nativeMs) +
			" ms). Use HOLD_LAST_POSE to keep only the held tail.";
		return false;
	}
	if (occurrence.iSourceEndMs &&
		(occurrence.iSourceEndMs <= occurrence.iSourceStartMs ||
			(Resolve_NativeClipMs(occurrence, nativeMs) && occurrence.iSourceEndMs > nativeMs + 1u)))
	{
		outStatus = "Source End must follow Source Start and remain inside the native clip.";
		return false;
	}
	return true;
}

// 단일 clip 요청을 세션 값으로 복사하고 다른 종류의 대기 preview 선택을 해제한다.
// 실제 모델 재생은 호스트가 요청을 소비할 때 수행하므로 이 함수는 문서나 장면 객체를 직접 실행하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Queue_AnimationPreview(
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &occurrence)
{
	m_bBundlePreviewRequestPending = false;
	m_PendingPreviewRequest = occurrence;
	m_bPreviewRequestPending = true;
	m_bPatternPreviewRequestPending = false;
	m_strPendingPreviewTargetAsset.clear();
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
}

// Play(ms)에 배속을 곱한 원본 시간 끝이 Source End 또는 native 끝을 넘는지 확인한다.
// EXACT인 입력만 HOLD_LAST_POSE로 바꾸며 native 길이가 없거나 이미 다른 정책이면 변경하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Normalize_EndPolicyForWindow(
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &occurrence, std::string &outNote) const
{
	if ("EXACT" != occurrence.strEndPolicy)
		return false;
	std::uint32_t nativeMs = 0u;
	if (!Resolve_NativeClipMs(occurrence, nativeMs))
		return false;
	const double windowEndMs =
		occurrence.iSourceStartMs + static_cast<double>(occurrence.iPlayMs) * occurrence.fPlayRate;
	if (windowEndMs <= (occurrence.iSourceEndMs ? (std::min)(nativeMs, occurrence.iSourceEndMs) : nativeMs) + 1.0)
		return false;
	occurrence.strEndPolicy = "HOLD_LAST_POSE";
	outNote += " " + occurrence.strRuntimeClip + " outruns its native clip; end policy became HOLD_LAST_POSE.";
	return true;
}

// catalog에서 만든 임시 Pattern을 세션의 대기 값으로 이동한다. 기존 expanded snapshot을 해제하고
// 시작 0 ms·재생 상태로 요청하며, 저작 문서의 Pattern이나 선택 포인터를 요청 안에 보관하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Queue_ResourcePatternPreview(
	KOUKU_SAYDON_COMPOSITION_PATTERN pattern, const std::string &targetAssetName)
{
	m_bBundlePreviewRequestPending = false;
	m_ExpandedPatternPreview.reset();
	m_PendingPatternPreview = std::move(pattern);
	m_strPendingPreviewTargetAsset = targetAssetName;
	m_iPendingPreviewStartMs = 0u;
	m_bPendingPreviewStartPaused = false;
	m_bPatternPreviewRequestPending = true;
	m_bPreviewRequestPending = false;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
}

// 모델 clip 길이가 준비됐을 때만 단일 Stage의 임시 Pattern을 만든다. 선택 clip과 target asset은 값으로
// 보관하므로 목록 포인터의 수명을 넘기지 않으며 길이가 없으면 기존 요청 대신 상태 메시지만 갱신한다.
void Client::CKoukuSaydonActionWorkbench::Queue_ModelResourcePreview(const COMPOSITION_ANIMATION_RESOURCE &resource)
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

// sequence의 clip들을 순서대로 임시 Stage에 담고 전체 시간을 600000 ms 안에서 누적한다.
// 모든 clip의 양수 길이와 누적 범위를 확인한 뒤에만 선택·대기 요청을 교체하므로 중간 실패로 반쪽 요청이 남지 않는다.
void Client::CKoukuSaydonActionWorkbench::Queue_SequencePreview(const COMPOSITION_ANIMATION_SEQUENCE_RESOURCE &sequence)
{
	KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
	pattern.strPatternId = "RESOURCE_PREVIEW";
	pattern.strDisplayName = sequence.strDisplayName;
	std::uint32_t totalMs = 0u;
	for (const auto &resource : sequence.Clips)
	{
		if (0u == resource.iDurationMs || resource.iDurationMs > MAX_EDITOR_TIME_MS ||
			totalMs > MAX_EDITOR_TIME_MS - resource.iDurationMs)
		{
			m_strStatus =
				"Sequence preview requires available clip timing within 600 seconds: " + resource.strRuntimeClip;
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

// 선택한 원본 slot을 값으로 복사해 단일 preview 요청과 리소스 선택에 함께 반영한다.
// reference/action/stage 포인터는 호출 동안만 차용하고 요청에는 출처 ID와 clip 값만 남긴다.
void Client::CKoukuSaydonActionWorkbench::Queue_SlotPreview(
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT &reference,
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE &action, const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE &stage,
	const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE &slot)
{
	const auto actor = CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(reference.strProfileId);
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE request = Build_ReferenceOccurrence(reference, action, stage, slot);
	m_strSelectedResourceProfileId = reference.strProfileId;
	m_iSelectedResourceActionId = action.iSourceActionId;
	m_strSelectedResourceTargetAsset.clear();
	m_strSelectedSequenceResourceId.clear();
	m_SelectedResource = request;
	m_bHasSelectedResource = true;
	Queue_AnimationPreview(request);
	m_strStatus = "Animation preview requested: " + slot.strRuntimeClip;
}

// action 전체 또는 지정 Stage의 slot을 저작 순서대로 임시 Pattern에 모은다. 각 slot의 offset은 해당
// Stage 안에서 누적하며 비어 있으면 요청을 교체하지 않는다. 리소스 선택 필드는 요청 생성 전에 갱신된다.
void Client::CKoukuSaydonActionWorkbench::Queue_ActionPreview(
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT &reference,
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE &action,
	const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE *const selectedStage)
{
	const auto actor = CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(reference.strProfileId);
	m_strSelectedResourceProfileId = reference.strProfileId;
	m_iSelectedResourceActionId = action.iSourceActionId;
	KOUKU_SAYDON_COMPOSITION_PATTERN preview;
	preview.strPatternId = "KOUKUSAYDON_RESOURCE_PREVIEW";
	preview.strDisplayName = action.strDisplayName;
	preview.strAuthoringStatus = "DRAFT";
	preview.strCategory = "NORMAL";
	std::uint32_t occurrenceOrdinal = 1u;
	std::uint32_t stageOrdinal = 1u;
	for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE &sourceStage : action.Stages)
	{
		if (nullptr != selectedStage && selectedStage->strStageId != sourceStage.strStageId)
		{
			continue;
		}
		if (sourceStage.Slots.empty())
			continue;
		KOUKU_SAYDON_COMPOSITION_STAGE stage;
		stage.strStageId = "STAGE_" + std::to_string(stageOrdinal);
		stage.strActionId = "koukusaydon.preview." + reference.strProfileId + "." +
			std::to_string(action.iSourceActionId) + "." + std::to_string(stageOrdinal);
		stage.strStageKind = "ACTIVE";
		std::uint32_t offsetMs = 0u;
		for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE &slot : sourceStage.Slots)
		{
			KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence =
				Build_ReferenceOccurrence(reference, action, sourceStage, slot);
			occurrence.strOccurrenceId = preview.strPatternId + ".animation." + std::to_string(occurrenceOrdinal++);
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
	m_ExpandedPatternPreview.reset();
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
	m_strStatus = nullptr == selectedStage ? "Action preview requested." : "Source Stage preview requested.";
}

// 호스트의 모델 메타데이터 갱신 결과를 세션이 소유한다. 비어 있는 결과는 기존 목록을 보존하고
// 새 목록을 받으면 오래된 RAW 선택을 해제한다. 상태 메시지와 검색 트리 재구성 표시는 두 경우 모두 갱신한다.
// resources를 값으로 받는 경계에서는 호출자가 넘긴 목록을 move하여 세션에 보관할 수 있다. 아래 조회 함수의 const 참조 인자는 호출 동안만 읽는다.
void Client::CKoukuSaydonActionWorkbench::Set_ModelResources(
	std::vector<COMPOSITION_ANIMATION_RESOURCE> resources, std::string status)
{
	if (!resources.empty())
	{
		m_ModelResources = std::move(resources);
		// Raw 선택에는 시간 정보의 사본이 있다. 모델 교체나 재시간화 후 refresh에서
		// 오래된 구간을 다시 append하지 않도록 참조를 갱신한다.
		if (!m_strSelectedResourceTargetAsset.empty())
		{
			m_bHasSelectedResource = false;
			m_strSelectedResourceTargetAsset.clear();
		}
	}
	m_strResourceStatus = std::move(status);
	m_bResourceTreeDirty = true;
}

// 공용 리소스 전송을 받기 전에 actor·선택 Pattern·필요한 Stage를 확인한다.
// 이 검사는 문서를 바꾸지 않으며 실제 추가는 다음 함수가 기존 Append/Bind 명령으로 위임한다.
bool Client::CKoukuSaydonActionWorkbench::Can_AppendCompositionAnimationResource(
	const COMPOSITION_ANIMATION_RESOURCE &resource, const bool asNewStage, std::string &outStatus) const
{
	const auto actor = CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(resource.strProfileId);
	if (actor.empty() || resource.strTargetAssetName != actor)
	{
		outStatus = "Select a clip from the Kouku, Saydon or Large Saydon model.";
		return false;
	}
	const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
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
	const COMPOSITION_ANIMATION_RESOURCE &resource, const bool asNewStage, std::string &outStatus)
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
	const bool result = asNewStage
		? Append_AnimationAsStage(m_strSelectedPatternId, source, stageId, occurrenceId, outStatus)
		: Bind_Animation(m_strSelectedPatternId, m_strSelectedStageId, source, 0u, occurrenceId, outStatus);
	m_strStatus = outStatus;
	return result;
}

// 검색어로 reference action/sequence와 물리 clip의 표시 트리를 다시 만든다. leaf의 숫자는 현재 목록의
// 화면 조회 index일 뿐 저장 ID가 아니므로 목록이나 검색어가 바뀌면 leaf와 두 트리를 함께 재구성한다.
void Client::CKoukuSaydonActionWorkbench::Rebuild_ResourceTree()
{
	m_strResourceTreeQuery = m_ResourceSearch;
	m_bResourceTreeDirty = false;
	// leaf에는 원본 목록의 위치만 보관하므로 leaf 목록과 표시 트리를 함께 비운다. 캐시 index를 원본 Action/clip ID 대신 파일에 저장하지 않는다.
	m_ResourceLeaves.clear();
	m_ResourceTree = {};
	const std::string_view query = m_strResourceTreeQuery;
	for (std::size_t documentIndex = 0u; documentIndex < m_ResourceReferences.Documents.size(); ++documentIndex)
	{
		const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT &reference =
			m_ResourceReferences.Documents[documentIndex];
		if (reference.strProfileId.empty())
			continue;
		for (std::size_t actionIndex = 0u; actionIndex < reference.Actions.size(); ++actionIndex)
		{
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE &action = reference.Actions[actionIndex];
			bool_t hasSlot = false;
			bool_t matches = query.empty() || ContainsInsensitive(action.strDisplayName, query) ||
				ContainsInsensitive(reference.strProfileId, query) ||
				ContainsInsensitive(std::to_string(action.iSourceActionId), query);
			for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE &stage : action.Stages)
			{
				for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE &slot : stage.Slots)
				{
					hasSlot = true;
					if (!matches && ContainsInsensitive(slot.strRuntimeClip, query))
						matches = true;
				}
			}
			if (!hasSlot || !matches)
				continue;
			const std::size_t leafIndex = m_ResourceLeaves.size();
			m_ResourceLeaves.push_back({documentIndex, actionIndex});
			InsertResourceTree(m_ResourceTree,
				{CKoukuSaydonAnimationActionDocument::Resolve_ActionCategory(
					 reference.strProfileId, action.strDisplayName),
					reference.strProfileId},
				leafIndex);
		}
	}
	for (std::size_t index = 0u; index < m_SequenceResources.size(); ++index)
	{
		const auto &sequence = m_SequenceResources[index];
		const bool matches = query.empty() || ContainsInsensitive(sequence.strDisplayName, query) ||
			ContainsInsensitive(sequence.strProfileId, query) || ContainsInsensitive(sequence.strStableId, query) ||
			std::any_of(sequence.Clips.begin(), sequence.Clips.end(), [&](const auto &clip) {
				return ContainsInsensitive(clip.strRuntimeClip, query);
			});
		if (!matches)
			continue;
		const auto leafIndex = m_ResourceLeaves.size();
		RESOURCE_ACTION_LEAF leaf;
		leaf.iAction = index;
		leaf.bSequence = true;
		m_ResourceLeaves.push_back(leaf);
		InsertResourceTree(m_ResourceTree, {"Valtan", "Valtan Sequences"}, leafIndex);
	}
	(void)FinalizeResourceTree(m_ResourceTree);
	m_PhysicalResourceTree = {};
	for (std::size_t index = 0u; index < m_ModelResources.size(); ++index)
	{
		const auto &resource = m_ModelResources[index];
		if (!query.empty() && !ContainsInsensitive(resource.strRuntimeClip, query) &&
			!ContainsInsensitive(resource.strTargetAssetName, query) &&
			!ContainsInsensitive(resource.strProfileId, query))
			continue;
		InsertResourceTree(m_PhysicalResourceTree, {resource.strTargetAssetName}, index);
	}
	(void)FinalizeResourceTree(m_PhysicalResourceTree);
}

// 원본 action의 비어 있지 않은 Stage를 후보에 순서대로 추가한다. 각 slot은 새 ID를 받고 원본 참조를
// 값으로 복사한다. 모든 Stage의 길이·ordinal 검증과 commit이 끝나야 선택을 마지막 추가 행으로 바꾼다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_ActionAsStages(const std::string_view patternId,
	const std::string_view profileId, const std::uint32_t sourceActionId, std::string &outStatus)
{
	const auto *reference = Find_Reference(m_ResourceReferences, profileId);
	const auto *action = nullptr == reference ? nullptr : Find_Action(*reference, sourceActionId);
	if (nullptr == action)
	{
		outStatus = m_strStatus = "Select an extracted action first.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern =
		Find_AppendPattern(candidate, patternId, profileId, action->strDisplayName, outStatus);
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
	for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE &sourceStage : action->Stages)
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
		for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE &slot : sourceStage.Slots)
		{
			if (pattern->iNextAnimationOrdinal >= 1000000u || slot.iPlayMs > MAX_EDITOR_TIME_MS ||
				offsetMs > MAX_EDITOR_TIME_MS - slot.iPlayMs)
			{
				outStatus = m_strStatus = "Action exceeds the Stage duration or stable ID limit; draft preserved.";
				return false;
			}
			KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence =
				Build_ReferenceOccurrence(*reference, *action, sourceStage, slot);
			occurrence.strOccurrenceId =
				pattern->strPatternId + ".animation." + std::to_string(pattern->iNextAnimationOrdinal++);
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
	if (!Extend_PatternLifetimeForAuthoredLanes(*pattern, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Appended " + std::to_string(appendedStages) + " Stage(s) from action " + std::to_string(sourceActionId) +
				"." + policyNote,
			outStatus))
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

// 원본 action의 slot들을 선택 Stage의 기존 clip 끝 뒤에 연결한다. 원본 Stage 구분은 목적지의 단일
// Stage 안에서 순차 offset으로 펼치며, 길이 확장과 후보 검증이 성공해야 새 clip 선택을 반영한다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_ActionToStage(const std::string_view patternId,
	const std::string_view stageId, const std::string_view profileId, const std::uint32_t sourceActionId,
	std::string &outStatus)
{
	const auto *reference = Find_Reference(m_ResourceReferences, profileId);
	const auto *action = nullptr == reference ? nullptr : Find_Action(*reference, sourceActionId);
	if (nullptr == action)
	{
		outStatus = m_strStatus = "Select an extracted action first.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_STAGE *const stage = nullptr == pattern ? nullptr : Find_Stage(*pattern, stageId);
	if (nullptr == stage)
	{
		outStatus = m_strStatus = "Append target Stage is unavailable.";
		return false;
	}
	std::uint32_t offsetMs = 0u;
	for (const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &row : stage->AnimationOccurrences)
		offsetMs = (std::max)(offsetMs, row.iStartOffsetMs + row.iPlayMs);
	std::string policyNote;
	std::size_t appendedClips = 0u;
	std::string lastOccurrenceId;
	for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE &sourceStage : action->Stages)
	{
		for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE &slot : sourceStage.Slots)
		{
			if (pattern->iNextAnimationOrdinal >= 1000000u)
			{
				outStatus = m_strStatus = "Pattern stable ID ordinals are exhausted.";
				return false;
			}
			KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence =
				Build_ReferenceOccurrence(*reference, *action, sourceStage, slot);
			occurrence.strOccurrenceId =
				pattern->strPatternId + ".animation." + std::to_string(pattern->iNextAnimationOrdinal++);
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
	if (!Extend_PatternLifetimeForAuthoredLanes(*pattern, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Appended " + std::to_string(appendedClips) + " clip(s) to Stage " + targetStageId + "." + policyNote,
			outStatus))
	{
		return false;
	}
	Clear_TimelineSelection();
	m_bFitRequested = true;
	m_strSelectedPatternId = std::string(patternId);
	Select_TimelineBox(targetStageId, lastOccurrenceId, false);
	return true;
}

// clip의 진입 blend 길이를 제한된 ms 범위로 편집한다. 0은 즉시 전환이며 원본 clip을 바꾸지 않는다.
// 아래 playback 편집은 배속·종료 정책과 source 구간을 함께 검사해 EXACT가 원본 끝을 넘는 후보를 거절한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_AnimationBlend(const std::string_view patternId,
	const std::string_view occurrenceId, const std::uint32_t blendInMs, std::string &outStatus)
{
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	const auto found = pattern ? Find_Occurrence(*pattern, occurrenceId) : MUTABLE_OCCURRENCE{};
	if (!pattern || !pattern->strLoadError.empty() || !found.pOccurrence || blendInMs > 1000u)
	{
		outStatus = m_strStatus = "Choose an editable Animation box and Blend In between 0 and 1000 ms.";
		return false;
	}
	found.pOccurrence->iBlendInMs = blendInMs;
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(
		std::move(candidate), "Updated Animation Blend In. Zero uses an immediate transition.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_AnimationPlayback(const std::string_view patternId,
	const std::string_view occurrenceId, const f32_t playRate, const std::string_view endPolicy, std::string &outStatus)
{
	const bool_t knownPolicy =
		std::any_of(END_POLICIES.begin(), END_POLICIES.end(), [endPolicy](const char_t *const value) {
			return endPolicy == value;
		});
	if (!knownPolicy || !std::isfinite(playRate) || playRate < 0.01f || playRate > 16.f)
	{
		outStatus = m_strStatus = "Play rate must be 0.01..16 and end policy EXACT, HOLD_LAST_POSE or LOOP_TO_WINDOW.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	const MUTABLE_OCCURRENCE found =
		nullptr == pattern ? MUTABLE_OCCURRENCE{} : Find_Occurrence(*pattern, occurrenceId);
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
		const double windowEndMs =
			found.pOccurrence->iSourceStartMs + static_cast<double>(found.pOccurrence->iPlayMs) * playRate;
		if (windowEndMs >
			(found.pOccurrence->iSourceEndMs ? (std::min)(nativeMs, found.pOccurrence->iSourceEndMs) : nativeMs) + 1.0)
		{
			outStatus = m_strStatus = "EXACT cannot outrun the native clip (" + std::to_string(nativeMs) +
				" ms). Shorten the box, or choose HOLD_LAST_POSE or LOOP_TO_WINDOW.";
			return false;
		}
	}
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Updated animation playback.", outStatus);
}

// 원본 연출 키를 시간순 편집용 Stage로 추가한다. 동시 트랙 가중치나 actor 이동을 재현하는 경로가
// 아니며 모든 키의 사용 가능 여부를 확인한 후보만 commit한다. 원본 catalog는 차용 후 그대로 보존한다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_CinematicGroup(const KOUKU_CINEMATIC_ANIMATION_GROUP &group)
{
	std::string status;
	for (const auto &key : group.Keys)
		if (!key.strStatus.empty() || !Is_AppendAdmitted(key.Animation, status))
		{
			m_strStatus = !key.strStatus.empty() ? key.strStatus : status;
			return false;
		}
	if (group.Keys.empty())
	{
		m_strStatus = "This cinematic group has no animation keys.";
		return false;
	}
	auto candidate = m_Draft;
	auto *pattern = Find_AppendPattern(candidate, m_strSelectedPatternId, group.strProfileId,
		group.strDisplayName + " / " + group.strGroupName, status);
	if (!pattern)
	{
		m_strStatus = status;
		return false;
	}
	const std::string patternId = pattern->strPatternId;
	std::string stageId, occurrenceId;
	for (const auto &key : group.Keys)
	{
		if (pattern->iNextStageOrdinal >= 1000000u || pattern->iNextAnimationOrdinal >= 1000000u)
		{
			m_strStatus = "Pattern stable ID ordinals are exhausted; draft preserved.";
			return false;
		}
		KOUKU_SAYDON_COMPOSITION_STAGE stage;
		const auto ordinal = pattern->iNextStageOrdinal++;
		stage.strStageId = "STAGE_" + std::to_string(ordinal);
		stage.strActionId = patternId + ".stage." + std::to_string(ordinal);
		stage.strStageKind = "ACTIVE";
		stage.iDurationMs = key.Animation.iPlayMs;
		auto animation = key.Animation;
		animation.strOccurrenceId = patternId + ".animation." + std::to_string(pattern->iNextAnimationOrdinal++);
		animation.iStartOffsetMs = 0u;
		stageId = stage.strStageId;
		occurrenceId = animation.strOccurrenceId;
		stage.AnimationOccurrences.push_back(std::move(animation));
		pattern->Stages.push_back(std::move(stage));
	}
	if (!Extend_PatternLifetimeForAuthoredLanes(*pattern, status))
	{
		m_strStatus = status;
		return false;
	}
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Appended cinematic keys as chronological editing Stages. Original concurrent track weights remain in the reference catalog.",
			status))
		return false;
	Clear_TimelineSelection();
	m_bFitRequested = true;
	m_strSelectedActorProfileId =
		std::string(CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(group.strProfileId));
	m_strSelectedPatternId = patternId;
	Select_TimelineBox(stageId, occurrenceId, false);
	return true;
}

// 연출 catalog는 세션에서 필요할 때 로드하고 scene/group/track/key 순으로 선택 UI를 구성한다.
// Append 버튼을 처리하면 ImGui scope를 닫은 뒤 true를 돌려 호출자에게 버튼 처리 여부를 알린다.
bool_t Client::CKoukuSaydonActionWorkbench::Render_CinematicResources()
{
	if (!m_bCinematicCatalogLoaded)
	{
		m_bCinematicCatalogLoaded = true;
		(void)CKoukuCinematicAnimationCatalog::Load(m_CinematicGroups, m_strCinematicStatus);
	}
	if (!ImGui::CollapsingHeader("연출 / Cinematic", ImGuiTreeNodeFlags_DefaultOpen))
		return false;
	ImGui::TextWrapped("%s", m_strCinematicStatus.c_str());
	ImGui::TextWrapped(
		"원본 배우와 A/B/C 트랙의 키 목록입니다. 전체 추가는 시간순 편집본이며 동시 트랙의 가중치 혼합과 배우 이동을 재현하지 않습니다.");
	bool appended = false;
	ImGui::BeginChild("##CinematicReferenceTree", ImVec2(0.f, 250.f), ImGuiChildFlags_Borders);
	std::vector<std::string> scenes;
	for (const auto &group : m_CinematicGroups)
		if (std::find(scenes.begin(), scenes.end(), group.strSceneId) == scenes.end())
			scenes.push_back(group.strSceneId);
	for (const auto &scene : scenes)
	{
		const auto first = std::find_if(m_CinematicGroups.begin(), m_CinematicGroups.end(), [&](const auto &group) {
			return group.strSceneId == scene;
		});
		ImGui::PushID(scene.c_str());
		if (ImGui::TreeNode(first->strDisplayName.c_str()))
		{
			for (const auto &group : m_CinematicGroups)
			{
				if (group.strSceneId != scene)
					continue;
				bool matches = ContainsInsensitive(group.strDisplayName, m_ResourceSearch) ||
					ContainsInsensitive(group.strGroupName, m_ResourceSearch) ||
					ContainsInsensitive(group.strProfileId, m_ResourceSearch);
				for (const auto &key : group.Keys)
					matches |= ContainsInsensitive(key.Animation.strRuntimeClip, m_ResourceSearch);
				if (!matches)
					continue;
				ImGui::PushID(group.strId.c_str());
				const std::string label = group.strGroupName + " / " + group.strProfileId +
					(group.strRuntimeProfileId != group.strProfileId ? " -> " + group.strRuntimeProfileId : "");
				if (ImGui::TreeNode(label.c_str()))
				{
					ImGui::TextDisabled("%s | Matinee %u | Group %u", group.strSourceScene.c_str(),
						group.iMatineeExport, group.iGroupExport);
					if (!group.strBindingStatus.empty())
						ImGui::TextWrapped("%s", group.strBindingStatus.c_str());
					bool ready = m_bHasDraft && !group.Keys.empty();
					std::string reason;
					const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
					if (pattern &&
						pattern->strActorProfileId !=
							CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(group.strProfileId))
						ready = false;
					for (const auto &key : group.Keys)
						ready &= key.strStatus.empty() && Is_AppendAdmitted(key.Animation, reason);
					ImGui::BeginDisabled(!ready);
					if (ImGui::SmallButton("시간순 편집본으로 전체 Append"))
					{
						(void)Append_CinematicGroup(group);
						appended = true;
					}
					ImGui::EndDisabled();
					std::vector<std::uint32_t> tracks;
					for (const auto &key : group.Keys)
						if (std::find(tracks.begin(), tracks.end(), key.iTrackExport) == tracks.end())
							tracks.push_back(key.iTrackExport);
					for (const auto track : tracks)
					{
						const auto key = std::find_if(group.Keys.begin(), group.Keys.end(), [&](const auto &value) {
							return value.iTrackExport == track;
						});
						const std::string trackLabel = "Track " + std::to_string(track) + " / " + key->strSlot;
						if (ImGui::TreeNode(trackLabel.c_str()))
						{
							for (const auto &value : group.Keys)
							{
								if (value.iTrackExport != track)
									continue;
								ImGui::PushID(value.strId.c_str());
								const auto &animation = value.Animation;
								const std::string keyLabel = std::to_string(static_cast<int>(value.fLocalStartMs)) +
									" ms | " +
									(animation.strRuntimeClip.empty() ? value.strSourceClip : animation.strRuntimeClip);
								const bool admitted = value.strStatus.empty() && Is_AppendAdmitted(animation, reason);
								ImGui::BeginDisabled(!admitted);
								if (ImGui::Selectable(keyLabel.c_str(),
										m_bHasSelectedResource && m_SelectedResource.strSourceSlotId == value.strId))
								{
									m_SelectedResource = animation;
									m_bHasSelectedResource = true;
									m_strSelectedResourceTargetAsset.clear();
									m_strSelectedResourceProfileId.clear();
									m_strSelectedSequenceResourceId.clear();
								}
								ImGui::EndDisabled();
								if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
									ImGui::SetTooltip(
										"Source key %.3f ms | effective source %u..%u ms | rate %.6f | %s\nNative Source In %.3f ms / End Offset %.3f ms | loop %d / reverse %d\n%s",
										value.fNativeStartMs, animation.iSourceStartMs, animation.iSourceEndMs,
										animation.fPlayRate, animation.strEndPolicy.c_str(), value.fSourceInMs,
										value.fSourceEndOffsetMs, value.bSourceLoop, value.bSourceReverse,
										value.strStatus.empty() ? reason.c_str() : value.strStatus.c_str());
								if (!value.strStatus.empty())
									ImGui::TextWrapped("%s", value.strStatus.c_str());
								if (!value.strNotice.empty())
									ImGui::TextWrapped("%s", value.strNotice.c_str());
								ImGui::PopID();
							}
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
				if (appended)
					break;
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
		if (appended)
			break;
	}
	ImGui::EndChild();
	return appended;
}

// Resources pane에서 원본 clip을 선택/추가하는 진입점이다. 선택 목록은 참조 문서를 읽고 추가 명령만 현재 draft에 전달한다.
void Client::CKoukuSaydonActionWorkbench::Render_AnimationResources()
{
	ImGui::SeparatorText("Animation Resources");
	if (ImGui::Button("Refresh Animation Resources"))
	{
		m_bResourceRefreshRequested = true;
		m_bCinematicCatalogLoaded = false;
	}
	ImGui::SameLine();
	ImGui::TextDisabled("%zu actions | %zu physical clips", m_ResourceLeaves.size(), m_ModelResources.size());
	ImGui::TextWrapped("%s", m_strResourceStatus.c_str());
	if (!m_strSequenceResourceStatus.empty())
		ImGui::TextWrapped("%s", m_strSequenceResourceStatus.c_str());
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::InputTextWithHint(
			"##KoukuResourceSearch", "Search action, profile or clip...", m_ResourceSearch, sizeof(m_ResourceSearch)))
	{
		m_bResourceTreeDirty = true;
	}
	if (m_bResourceTreeDirty || m_strResourceTreeQuery != m_ResourceSearch)
		Rebuild_ResourceTree();

	const KOUKU_SAYDON_COMPOSITION_PATTERN *const selectedPattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool_t patternEditable = nullptr != selectedPattern && selectedPattern->strLoadError.empty();
	const bool_t stageSelected = patternEditable && !m_strSelectedStageId.empty();
	const std::string targetActorProfile =
		nullptr == selectedPattern ? std::string{} : selectedPattern->strActorProfileId;
	const std::string targetPatternName =
		nullptr == selectedPattern ? "Select a Pattern" : selectedPattern->strDisplayName;

	const auto selectedSequence =
		std::find_if(m_SequenceResources.begin(), m_SequenceResources.end(), [this](const auto &item) {
			return item.strStableId == m_strSelectedSequenceResourceId;
		});
	if (selectedSequence != m_SequenceResources.end())
	{
		ImGui::SeparatorText("Selected Sequence");
		ImGui::TextWrapped("%s", selectedSequence->strDisplayName.c_str());
		if (ImGui::Button("Preview Sequence"))
			Queue_SequencePreview(*selectedSequence);
		ImGui::TextDisabled("Valtan sequence previews on its own model; select a Kouku body clip to append here.");
	}
	/* 선택한 추출 action은 원본 저작자가 구성한 Stage/slot 순서로 표시한다. */
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT *const selectedDocument =
		m_strSelectedResourceProfileId.empty() ? nullptr
											   : Find_Reference(m_ResourceReferences, m_strSelectedResourceProfileId);
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE *const selectedAction =
		nullptr == selectedDocument ? nullptr : Find_Action(*selectedDocument, m_iSelectedResourceActionId);
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
		for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE &stage : selectedAction->Stages)
		{
			for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE &slot : stage.Slots)
			{
				ImGui::BulletText("%02u  %s  | %u ms%s", ++clipOrdinal, slot.strRuntimeClip.c_str(), slot.iPlayMs,
					slot.bLoop ? " loop" : "");
			}
		}
		ImGui::EndChild();
		const auto actionActor =
			CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(selectedDocument->strProfileId);
		const bool_t canAppendAction =
			m_bHasDraft && !actionActor.empty() && (patternEditable && targetActorProfile == actionActor);
		if (ImGui::Button("Preview Action"))
			Queue_ActionPreview(*selectedDocument, *selectedAction, nullptr);
		ImGui::SameLine();
		ImGui::BeginDisabled(!canAppendAction);
		if (ImGui::Button("Append Action as Stages"))
		{
			std::string status;
			(void)Append_ActionAsStages(
				m_strSelectedPatternId, selectedDocument->strProfileId, selectedAction->iSourceActionId, status);
			m_strStatus = status;
		}
		ImGui::BeginDisabled(!stageSelected);
		if (ImGui::Button("Append Action to Selected Stage"))
		{
			std::string status;
			(void)Append_ActionToStage(m_strSelectedPatternId, m_strSelectedStageId, selectedDocument->strProfileId,
				selectedAction->iSourceActionId, status);
			m_strStatus = status;
		}
		ImGui::EndDisabled();
		ImGui::EndDisabled();
		ImGui::TextWrapped(
			"Append target: %s / %s", Actor_Label(actionActor, m_strSelectedGateId), targetPatternName.c_str());
	}
	if (m_bHasSelectedResource)
	{
		ImGui::SeparatorText("Selected Clip");
		ImGui::TextDisabled("%s | %s | %u ms | %s", m_SelectedResource.strProfileId.c_str(),
			m_SelectedResource.strRuntimeClip.c_str(), m_SelectedResource.iPlayMs,
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
			(void)Append_AnimationAsStage(m_strSelectedPatternId, m_SelectedResource, stageId, occurrenceId, status);
			m_strStatus = status;
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(!stageSelected);
		if (ImGui::Button("Add Animation Row"))
		{
			std::string occurrenceId, status;
			(void)Bind_Animation(
				m_strSelectedPatternId, m_strSelectedStageId, m_SelectedResource, 0u, occurrenceId, status);
			m_strStatus = status;
		}
		ImGui::EndDisabled();
		ImGui::EndDisabled();
	}
	if (!ImGui::BeginChild("##KoukuAnimationResources", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders))
	{
		ImGui::EndChild();
		return;
	}

	if (!m_bSharedWorkspaceActive && ImGui::TreeNodeEx("Physical Clips", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (m_ModelResources.empty())
			ImGui::TextDisabled("Refresh Animation Resources reads the installed model clip headers.");
		RenderResourceTree(m_PhysicalResourceTree, [this](const std::size_t index) {
			const auto &resource = m_ModelResources[index];
			ImGui::PushID(resource.strTargetAssetName.c_str());
			ImGui::PushID(resource.strRuntimeClip.c_str());
			if (ImGui::Selectable(resource.strRuntimeClip.c_str(),
					m_bHasSelectedResource && m_strSelectedResourceTargetAsset == resource.strTargetAssetName &&
						m_SelectedResource.strRuntimeClip == resource.strRuntimeClip))
				Queue_ModelResourcePreview(resource);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s | %u ms\n%s\nClick to preview", resource.strTargetAssetName.c_str(),
					resource.iDurationMs, resource.strSourceAssetId.c_str());
			ImGui::PopID();
			ImGui::PopID();
		});
		ImGui::TreePop();
	}

	if (m_ResourceLeaves.empty())
		ImGui::TextDisabled("No extracted action matches. Action references load with the composition.");
	RenderResourceTree(m_ResourceTree, [this](const std::size_t leafIndex) {
		const RESOURCE_ACTION_LEAF &leaf = m_ResourceLeaves[leafIndex];
		if (leaf.bSequence)
		{
			const auto &sequence = m_SequenceResources[leaf.iAction];
			ImGui::PushID(sequence.strStableId.c_str());
			const std::string sequencePath = "SEQUENCE/" + sequence.strStableId;
			ImGui::SetNextItemOpen(m_strExpandedResourceActionId == sequencePath);
			const bool open = ImGui::TreeNodeEx("##Sequence", ImGuiTreeNodeFlags_SpanAvailWidth, "%s | %zu clips",
				sequence.strDisplayName.c_str(), sequence.Clips.size());
			if (ImGui::IsItemToggledOpen())
			{
				m_strExpandedResourceActionId = open ? sequencePath : std::string{};
				m_strExpandedResourceStageId.clear();
			}
			if (ImGui::IsItemClicked())
				Queue_SequencePreview(sequence);
			if (open)
			{
				for (std::size_t index = 0u; index < sequence.Clips.size(); ++index)
				{
					const auto &clip = sequence.Clips[index];
					ImGui::PushID(static_cast<int>(index));
					if (ImGui::Selectable(clip.strRuntimeClip.c_str()))
						Queue_ModelResourcePreview(clip);
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("%u ms | %s", clip.iDurationMs, clip.strEndPolicy.c_str());
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
			return;
		}
		const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT &reference =
			m_ResourceReferences.Documents[leaf.iDocument];
		const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE &action = reference.Actions[leaf.iAction];
		ImGui::PushID(reference.strProfileId.c_str());
		ImGui::PushID(std::to_string(action.iSourceActionId).c_str());
		const bool_t actionSelected = m_iSelectedResourceActionId == action.iSourceActionId &&
			m_strSelectedResourceProfileId == reference.strProfileId;
		std::uint32_t totalMs = 0u;
		std::size_t clipCount = 0u;
		for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE &stage : action.Stages)
		{
			for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE &slot : stage.Slots)
			{
				totalMs += slot.iPlayMs;
				++clipCount;
			}
		}
		const std::string actionLabel = action.strDisplayName + " (" + std::to_string(action.iSourceActionId) + ") | " +
			std::to_string(clipCount) + " clips | " + std::to_string(totalMs) + " ms";
		const std::string actionPath =
			"ACTION/" + reference.strProfileId + "/" + std::to_string(action.iSourceActionId);
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
			for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE &stage : action.Stages)
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
					for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE &slot : stage.Slots)
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

// 선택 source 구간을 Stage의 남은 재생 창에 반복한다. native 메타데이터가 있어야 미지정 Source End를
// 확정할 수 있고, 유효한 양수 원본 구간을 검증한 후보에서만 LOOP_TO_WINDOW와 Play 길이를 바꾼다.
bool_t Client::CKoukuSaydonActionWorkbench::Loop_AnimationToStageEnd(
	const std::string_view patternId, const std::string_view occurrenceId, std::string &outStatus)
{
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	const auto found = pattern ? Find_Occurrence(*pattern, occurrenceId) : MUTABLE_OCCURRENCE{};
	if (!found.pOccurrence || !found.pStage)
	{
		outStatus = "Animation loop target is absent.";
		return false;
	}
	auto &row = *found.pOccurrence;
	std::uint32_t nativeMs = 0u;
	if (!Resolve_NativeClipMs(row, nativeMs))
	{
		outStatus = m_strStatus = "Refresh Model Clips before repeating a selected source range.";
		return false;
	}
	if (!row.iSourceEndMs)
		row.iSourceEndMs = static_cast<std::uint32_t>(
			std::llround((std::min)(double(nativeMs), row.iSourceStartMs + double(row.iPlayMs) * row.fPlayRate)));
	if (!row.iSourceEndMs || row.iSourceEndMs <= row.iSourceStartMs)
	{
		outStatus = m_strStatus = "Select a source range of at least 1 ms before repeating it.";
		return false;
	}
	row.strEndPolicy = "LOOP_TO_WINDOW";
	row.iPlayMs = found.pStage->iDurationMs - row.iStartOffsetMs;
	if (!Validate_SourceStart(row, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Repeating the selected source range to the stage end.", outStatus);
}
