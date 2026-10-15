// 현재 편집 문서의 검증·저장과 후보 commit을 소유한다. ImGui 행 표시나 실제 전투를 실행하지 않는다.
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>

using namespace Client::KoukuWorkbenchDetail;

namespace
{
using namespace Client;

// Commit_Candidate가 완전한 자식 재생을 약속한 부모만 동기화한다. 변경 전 gap을 보존하며 뒤 행의 시각을 이동한다.
// 후보를 부분 수정한 뒤 실패할 수 있지만 아직 m_Draft에 commit하지 않았으므로 기존 편집본은 유지된다.
bool_t Synchronize_CompleteParentWindows(const KOUKU_SAYDON_COMPOSITION_DOCUMENT &previous,
	KOUKU_SAYDON_COMPOSITION_DOCUMENT &candidate, std::string &status)
{
	// 순차/loop 부모는 각 자식의 전체 수명을 재생한다. 자유 trim/repeat 창과 달리 자식 길이 변경에 따라 늘린다.
	for (auto &parent : candidate.Patterns)
	{
		if (!parent.strLoadError.empty() ||
			(!parent.bPlayChildrenSequentially && parent.strLoopStartPatternOccurrenceId.empty()))
			continue;
		const auto *oldParent = Find_Pattern(previous, parent.strPatternId);
		if (!oldParent || !oldParent->strLoadError.empty() ||
			(!oldParent->bPlayChildrenSequentially && oldParent->strLoopStartPatternOccurrenceId.empty()))
			continue;
		bool changedChild = false;
		for (const auto &row : oldParent->PatternOccurrences)
		{
			const auto *before = Find_Pattern(previous, row.strPatternId);
			const auto *after = Find_Pattern(candidate, row.strPatternId);
			changedChild |= before && after && before->strLoadError.empty() && after->strLoadError.empty() &&
				Pattern_DurationMs(*before) != Pattern_DurationMs(*after);
		}
		if (!changedChild)
			continue;
		const auto reject = [&](const std::string &reason) {
			status = "Parent " + parent.strPatternId + ": " + reason;
			return false;
		};
		if (parent.PatternOccurrences != oldParent->PatternOccurrences || parent.iDurationMs != oldParent->iDurationMs)
			return reject("child lifetime and Parent windows were both edited; update them separately.");
		auto rows = oldParent->PatternOccurrences;
		std::sort(rows.begin(), rows.end(), [](const auto &a, const auto &b) {
			return a.iStartMs < b.iStartMs;
		});
		// shift는 앞 자식들의 새 길이-옛 길이 누적값(ms)이다. 기존 시작에 더해 자식 사이 gap을 그대로 둔다.
		std::int64_t shift = 0;
		std::uint64_t end = 0u;
		for (const auto &oldRow : rows)
		{
			const auto *before = Find_Pattern(previous, oldRow.strPatternId);
			const auto *after = Find_Pattern(candidate, oldRow.strPatternId);
			if (!before || !after || oldRow.bRepeat || oldRow.iDurationMs != Pattern_DurationMs(*before))
				return reject("cannot retime incomplete child " + oldRow.strOccurrenceId + ".");
			const auto lifetime = Pattern_DurationMs(*after);
			const auto start = std::int64_t(oldRow.iStartMs) + shift;
			if (!lifetime || start < 0 || start + lifetime > MAX_EDITOR_TIME_MS)
				return reject("child " + oldRow.strOccurrenceId + " exceeds the 1..600000 ms timeline.");
			auto row = std::find_if(
				parent.PatternOccurrences.begin(), parent.PatternOccurrences.end(), [&](const auto &value) {
					return value.strOccurrenceId == oldRow.strOccurrenceId;
				});
			row->iStartMs = static_cast<std::uint32_t>(start);
			row->iDurationMs = lifetime;
			shift += std::int64_t(lifetime) - oldRow.iDurationMs;
			end = std::uint64_t(start) + lifetime;
		}
		if (!end || end > MAX_EDITOR_TIME_MS)
			return reject("complete children leave an invalid Parent lifetime.");
		const auto oldEnd = Pattern_DurationMs(*oldParent);
		// 원래 부모 끝에 맞던 공통 행만 새 끝까지 늘린다. 더 짧은 고정 행과 source 시계·배속은 유지한다.
		const auto retimeEnds = [&](auto &currentRows, const auto &oldRows) {
			for (auto &row : currentRows)
			{
				const auto old = std::find_if(oldRows.begin(), oldRows.end(), [&](const auto &value) {
					return value.strOccurrenceId == row.strOccurrenceId;
				});
				if (old != oldRows.end() && std::uint64_t(old->iStartMs) + old->iDurationMs == oldEnd)
				{
					if (row.iStartMs != old->iStartMs || row.iDurationMs != old->iDurationMs)
						return reject(
							"end-bound row " + row.strOccurrenceId + " was also edited; its window was preserved.");
					if (row.iStartMs >= end)
						return reject(
							"new end " + std::to_string(end) + " ms precedes row " + row.strOccurrenceId + ".");
					row.iDurationMs = static_cast<std::uint32_t>(end - row.iStartMs);
				}
				if (std::uint64_t(row.iStartMs) + row.iDurationMs > end)
					return reject(
						"fixed row " + row.strOccurrenceId + " exceeds the new end " + std::to_string(end) + " ms.");
			}
			return true;
		};
		if (!retimeEnds(parent.LogicOccurrences, oldParent->LogicOccurrences) ||
			!retimeEnds(parent.SummonOccurrences, oldParent->SummonOccurrences) ||
			!retimeEnds(parent.WorldOccurrences, oldParent->WorldOccurrences) ||
			!retimeEnds(parent.SceneProfileOccurrences, oldParent->SceneProfileOccurrences) ||
			!retimeEnds(parent.PresentationOccurrences, oldParent->PresentationOccurrences))
			return false;
		parent.iDurationMs = static_cast<std::uint32_t>(end);
		Mark_Draft(candidate, parent);
	}
	return true;
}

// 삭제/복제 후 의미 없는 1개짜리 선택 group을 정리한다. Logic이 이름으로 참조하는 group은 1개여도 유지한다.
void Remove_SingletonPresentationGroups(KOUKU_SAYDON_COMPOSITION_DOCUMENT &document)
{
	for (auto &pattern : document.Patterns)
	{
		if (!pattern.strLoadError.empty())
			continue;
		// Logic이 대상 ID로 참조하는 group은 구성원 수만으로 삭제할 수 없다.
		std::unordered_set<std::string> referencedGroups;
		for (const auto &occurrence : pattern.LogicOccurrences)
			if (const auto *logic = Find_Logic(document, occurrence.strLogicId))
			{
				if (!logic->strSelectedEffectGroupId.empty())
					referencedGroups.insert(logic->strSelectedEffectGroupId);
				if (!logic->strFixedSelectionGroupId.empty())
					referencedGroups.insert(logic->strFixedSelectionGroupId);
			}
		// 참조 존재 여부는 set, 구성원 수는 map에 담는다. 둘 다 이번 후보 정리에만 쓰며 PresentationOccurrences의 저장·표시 순서는 바꾸지 않는다.
		std::unordered_map<std::string, std::size_t> counts;
		for (const auto &box : pattern.PresentationOccurrences)
			if (!box.strSelectionGroupId.empty())
				++counts[box.strSelectionGroupId];
		for (auto &box : pattern.PresentationOccurrences)
			if (!box.strSelectionGroupId.empty() && counts.at(box.strSelectionGroupId) < 2u &&
				!referencedGroups.contains(box.strSelectionGroupId))
				box.strSelectionGroupId.clear();
	}
}
} // 이 파일의 보조 구현 범위 끝

// 첫 frame·Reload 버튼·외부 저장 연계가 호출한다. dirty composition은 명시 폐기 전 거부하고 읽기 성공 후에만 draft를 교체한다.
bool_t Client::CKoukuSaydonActionWorkbench::Reload(std::string &outStatus)
{
	m_bLoadAttempted = true;
	if (Is_CompositionDirty())
	{
		outStatus = "KoukuSaydon composition Reload requires an explicit draft discard.";
		m_strStatus = outStatus;
		return false;
	}
	// composition 읽기 실패여도 별도 읽기 전용 Action 참조는 갱신할 수 있다. 기존 draft는 여기서 교체하지 않는다.
	if (!m_Document.Reload(outStatus))
	{
		KOUKU_SAYDON_ACTION_REFERENCE_SET references;
		std::string referenceStatus;
		if (CKoukuSaydonCompositionDocument::Load_ImmutableActionReferences(references, referenceStatus))
		{
			m_ResourceReferences = std::move(references);
		}
		m_bResourceTreeDirty = true;
		m_strStatus = outStatus;
		return false;
	}

	if (Is_CompleteSequencePlaying())
		Stop_Preview();
	const std::string previousPatternId = m_strSelectedPatternId;
	m_Draft = m_Document.Get_LastGood();
	m_EditHistory.Clear();
	m_HistoryDocument.reset();
	m_PendingHistoryDirection = 0;
	m_strLogicValueDraftId.clear();
	m_InlineLogicValueDrafts.clear();
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
		if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN)
			m_ePatternSelection = KOUKU_PATTERN_SELECTION::GATE;
		m_strSelectedStageId.clear();
		m_strSelectedOccurrenceId.clear();
	}
	const KOUKU_SAYDON_COMPOSITION_PATTERN *const selectedPattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (nullptr != selectedPattern && m_strSelectedStageId.empty() && !selectedPattern->Stages.empty())
	{
		m_strSelectedStageId = selectedPattern->Stages.front().strStageId;
	}
	Normalize_Selection();
	Synchronize_EditorFields();
	m_strStatus = outStatus;
	return true;
}

// Save 버튼이 staged geometry를 문서 후보에 합쳐 source를 원자 저장한 뒤 외부 World 편집을 저장한다.
// 두 저장은 별개다. 뒤 World 저장 실패는 앞 composition 저장 성공을 rollback하지 않으며 World pending을 남긴다.
bool_t Client::CKoukuSaydonActionWorkbench::Save(std::string &outStatus)
{
	if (!m_bHasDraft || Is_PublishRunning())
	{
		outStatus = m_strStatus = !m_bHasDraft
			? "No composition is loaded."
			: "Publish is reading the saved source. Keep editing the draft; Save is available after publish finishes.";
		return false;
	}
	Finish_EditHistory(true);
	// 외부 World의 저장/이력 갱신은 실제 소유자 callback으로 처리한다. 실패하면 pending을 지우지 않는다.
	const auto saveWorldAnimations = [&]() {
		if (!m_WorldAnimationEditsPending)
			return true;
		if (!m_SaveWorldAnimation || !m_SaveWorldAnimation(outStatus))
		{
			m_strStatus = outStatus.empty() ? "World animation Save is unavailable. Edits are preserved." : outStatus;
			return false;
		}
		m_WorldAnimationEditsPending = false;
		if (m_CaptureWorldAnimationHistory)
			m_HistoryWorldDocument = m_CaptureWorldAnimationHistory(false);
		return true;
	};
	if (Is_CompositionDirty())
	{
		auto candidate = m_Draft;
		for (const auto &edit : m_StagedPresentationGeometry)
		{
			auto *pattern = Find_Pattern(candidate, edit.strPatternId);
			const auto *source = pattern ? Find_PresentationBox(*pattern, edit.Occurrence.strOccurrenceId) : nullptr;
			const auto *resource = source ? Find_PresentationResource(candidate, source->strResourceId) : nullptr;
			if (!source || source->strResourceId != edit.Occurrence.strResourceId || !resource ||
				(resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT &&
					resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER &&
					resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::SUBTITLE) ||
				!Valid_PresentationPlacement(*resource, edit.Occurrence))
			{
				outStatus = m_strStatus =
					"Save rejected: correct or revert the pending presentation placement. Applied draft and source are unchanged.";
				return false;
			}
			auto geometry = *source;
			Copy_PresentationPlacement(
				edit.Occurrence, geometry, resource && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
			if (!Valid_GameplayColliderScale(*resource, geometry))
			{
				outStatus = m_strStatus =
					"Save rejected: Collider scale must be finite and positive. Adjust Scale / size for " +
					edit.Occurrence.strOccurrenceId + ". Pending edits and previous source are preserved.";
				return false;
			}
			for (auto &box : pattern->PresentationOccurrences)
				if (box.strOccurrenceId == edit.Occurrence.strOccurrenceId)
				{
					Copy_PresentationPlacement(
						edit.Occurrence, box, resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
					break;
				}
		}
		// 최신 source와의 stable ID 병합·freshness·원자 교체는 Document가 책임진다. 실패 시 staged overlay도 유지한다.
		if (!m_Document.Save_Atomic(candidate, outStatus))
		{
			m_strStatus = outStatus;
			return false;
		}
		candidate.iRevision = m_Document.Get_LastGood().iRevision;
		if (candidate != m_Document.Get_LastGood())
			m_EditHistory.Clear(); // 외부 병합 결과는 이전 편집 history와 다른 기준이므로 이력을 비운다.
		m_Draft = m_Document.Get_LastGood();
		m_ResourceReferences = m_Document.Get_References();
		m_bResourceTreeDirty = true;
		m_bDirty = false;
		m_StagedPresentationGeometry.clear();
		++m_iDraftGeneration;
		Normalize_Selection();
		Synchronize_EditorFields();
		if (Has_StaleLocalPreviewSnapshot())
		{
			if (!saveWorldAnimations())
				return false;
			Stop_StaleLocalPreview(
				"Saved changes. The previous preview was reset; Play uses the saved timeline from this cursor.");
			outStatus = m_strStatus;
			return true;
		}
	}
	if (!saveWorldAnimations())
		return false;
	outStatus = m_strStatus = m_bSequenceWorkspace
		? "Saved all Sequence changes to the independent Sequencer workspace."
		: "Saved all Composition changes. Use Publish All Patterns to synchronize the F1 tree.";
	return true;
}

// 현재 draft와 고정 Action 참조를 문서 검증기에 넘긴다. 저장·게시·실행이나 UI 상태 변경은 하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Validate_Draft(std::string &outStatus) const
{
	if (!m_bHasDraft)
	{
		outStatus = "No KoukuSaydon composition draft is loaded.";
		return false;
	}
	return CKoukuSaydonCompositionDocument::Validate(m_Draft, m_ResourceReferences, outStatus);
}

// 모든 typed 편집 명령이 새 후보를 제출하는 공통 경계다. 부모 시각/참조/문서 검증 전에 기존 draft를 교체하지 않는다.
// 성공하면 history 시작, 문서 교체, generation 증가, 선택/입력 재동기화를 같은 순서로 수행한다.
// candidate를 값으로 받아 검증 중 수정해도 현재 m_Draft를 건드리지 않는다. 검증되고 값이 달라진 경우 문서를 move 교체하므로 이전 행 포인터는 버리고 stable ID로 다시 조회한다.
bool_t Client::CKoukuSaydonActionWorkbench::Commit_Candidate(
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate, const std::string_view successStatus, std::string &outStatus)
{
	std::string parentTimingStatus;
	if (!Synchronize_CompleteParentWindows(m_Draft, candidate, parentTimingStatus))
	{
		outStatus = m_strStatus = "KoukuSaydon edit rejected; draft preserved: " + parentTimingStatus;
		return false;
	}
	Remove_SingletonPresentationGroups(candidate);
	// 읽기 실패한 행의 원문은 수정할 수 없다. 삭제 또는 source 수리 후 Reload 경로만 허용한다.
	for (const auto &previous : m_Draft.Patterns)
	{
		if (previous.strLoadError.empty())
			continue;
		const auto *current = Find_Pattern(candidate, previous.strPatternId);
		if (nullptr != current && *current != previous)
		{
			outStatus = m_strStatus = "Repair and reload the invalid Pattern, or delete it before editing.";
			return false;
		}
	}
	for (const auto &previous : m_Draft.Folders)
		if (!previous.strLoadError.empty())
			for (const auto &current : candidate.Folders)
				if (current.strFolderId == previous.strFolderId && current != previous)
				{
					outStatus = m_strStatus = "Repair and Reload the invalid parent before editing.";
					return false;
				}
	for (const auto &previous : m_Draft.Bundles)
		if (!previous.strLoadError.empty())
			for (const auto &current : candidate.Bundles)
				if (current.strBundleId == previous.strBundleId && current != previous)
				{
					outStatus = m_strStatus = "Repair and Reload the invalid bundle before editing.";
					return false;
				}
	for (auto &bundle : candidate.Bundles)
		if (bundle.strLoadError.empty())
			for (const auto &member : bundle.Members)
				if (const auto *child = Find_Pattern(candidate, member.strPatternId);
					child && child->strAuthoringStatus != "PRODUCT")
					if (const auto *oldChild = Find_Pattern(m_Draft, member.strPatternId);
						oldChild && *child != *oldChild)
						bundle.strAuthoringStatus = "DRAFT";
	std::string validationStatus;
	if (!m_bHasDraft || !CKoukuSaydonCompositionDocument::Validate(candidate, m_ResourceReferences, validationStatus))
	{
		outStatus = "KoukuSaydon edit rejected; draft preserved: " + validationStatus;
		m_strStatus = outStatus;
		return false;
	}
	// 실제로 달라진 Logic 정의의 미적용 입력만 무효화한다. 다른 geometry/time 편집은 각 panel 입력을 유지한다.
	for (auto *draftId : {&m_strLogicValueDraftId, &m_strColliderLogicValueDraftId})
	{
		if (draftId->empty())
			continue;
		const auto *previous = Find_Logic(m_Draft, *draftId);
		const auto *next = Find_Logic(candidate, *draftId);
		if (!previous || !next || *previous != *next)
			draftId->clear();
	}
	std::erase_if(m_InlineLogicValueDrafts, [&](const auto &entry) {
		const auto *previous = Find_Logic(m_Draft, entry.first);
		const auto *next = Find_Logic(candidate, entry.first);
		return !previous || !next || *previous != *next;
	});
	if (candidate == m_Draft)
	{
		outStatus = m_strStatus = std::string(successStatus);
		return true;
	}
	// 모든 검증이 끝난 반영 경계다. 호출자의 후보와 이전 draft가 같으면 위에서 성공만 반환한다.
	Begin_EditHistory();
	m_Draft = std::move(candidate);
	m_bDirty = true;
	++m_iDraftGeneration;
	Normalize_Selection();
	Synchronize_EditorFields();
	outStatus = std::string(successStatus);
	m_strStatus = outStatus;
	return true;
}
