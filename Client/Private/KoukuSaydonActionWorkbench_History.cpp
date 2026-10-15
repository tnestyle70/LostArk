// 문서·배치·선택·외부 World snapshot을 편집 단위로 기록하고 pane가 끝난 뒤 복원한다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"

#include <algorithm>
#include <memory>
#include <utility>

using namespace Client::KoukuWorkbenchDetail;

// Begin/Finish가 같은 draft generation의 불변 문서 snapshot을 공유한다. 선택/overlay는 현재 값으로 따로 복사한다.
Client::CKoukuSaydonActionWorkbench::EDIT_HISTORY_STATE Client::CKoukuSaydonActionWorkbench::Capture_EditHistory(
	const bool prepareWorld)
{
	if (!m_HistoryDocument || m_HistoryDocumentGeneration != m_iDraftGeneration)
	{
		// const snapshot을 shared_ptr로 소유하므로 여러 이력 항목이 같은 세대를 함께 유지할 수 있다. 현재 draft를 고쳐도 이미 기록된 문서 내용은 바뀌지 않는다.
		m_HistoryDocument = std::make_shared<const KOUKU_SAYDON_COMPOSITION_DOCUMENT>(m_Draft);
		m_HistoryDocumentGeneration = m_iDraftGeneration;
	}
	EDIT_HISTORY_STATE state;
	state.document = m_HistoryDocument;
	state.geometry = m_StagedPresentationGeometry;
	if (m_CaptureWorldAnimationHistory)
		m_HistoryWorldDocument = m_CaptureWorldAnimationHistory(prepareWorld);
	state.worldDocument = m_HistoryWorldDocument;
	state.patternSelection = m_ePatternSelection;
	state.bingoParentBox = m_iBingoParentBox;
	state.bingoEntryWaitMs = m_iBingoEntryWaitMs;
	state.bingoParentEntryId = m_strBingoParentEntryId;
	state.bingoParentReturn = m_bBingoParentReturn;
	state.gate = m_strSelectedGateId;
	state.folder = m_strSelectedFolderId;
	state.bundle = m_strSelectedBundleId;
	state.pattern = m_strSelectedPatternId;
	state.stage = m_strSelectedStageId;
	state.animation = m_strSelectedOccurrenceId;
	state.logic = m_strSelectedLogicId;
	state.logicBox = m_strSelectedLogicOccurrenceId;
	state.summon = m_strSelectedSummonId;
	state.summonBox = m_strSelectedSummonOccurrenceId;
	state.world = m_strSelectedWorldId;
	state.worldBox = m_strSelectedWorldOccurrenceId;
	state.scene = m_strSelectedSceneProfileId;
	state.sceneBox = m_strSelectedSceneProfileOccurrenceId;
	state.presentation = m_strSelectedPresentationResourceId;
	state.presentationBox = m_strSelectedPresentationOccurrenceId;
	state.patternBox = m_strSelectedPatternOccurrenceId;
	state.timelinePattern = m_strTimelineSelectionPatternId;
	state.stages = m_TimelineSelectedStageIds;
	state.occurrences = m_TimelineSelectedOccurrenceIds;
	state.cursorPattern = m_strCursorPatternId;
	state.cursor = m_iCursorMs;
	return state;
}

// drag나 숫자 입력이 끝날 때 한 편집으로 묶는다. Save/Undo의 force는 입력 중이어도 현재까지를 먼저 닫는다.
// 문서·geometry·World가 같으면 선택/cursor만 달라진 frame을 새 문서 편집으로 기록하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Finish_EditHistory(const bool force)
{
	if (!m_EditHistory.Is_Pending())
		return;
	if (!force && ImGui::GetCurrentContext() && (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsAnyItemActive()))
		return;
	m_EditHistory.Commit(Capture_EditHistory(), [](const auto &before, const auto &after) {
		return (before.document == after.document || *before.document == *after.document) &&
			before.geometry == after.geometry && before.worldDocument == after.worldDocument;
	});
}

// 첫 변경 전 상태를 한 번만 기록한다. 복원 중이거나 이미 진행 중인 drag이면 중첩 이력을 만들지 않는다.
void Client::CKoukuSaydonActionWorkbench::Begin_EditHistory(const bool prepareWorld)
{
	if (!m_HistoryRestoring && !m_EditHistory.Is_Pending())
		m_EditHistory.Begin(Capture_EditHistory(prepareWorld));
}

// Undo/Redo가 후보 문서와 geometry를 검증하고 외부 World의 expected snapshot 일치를 확인한다.
// 외부 복원 거부까지는 현재 draft를 유지한다. 성공 뒤에는 preview를 중지하고 stable 선택을 복원한다.
bool Client::CKoukuSaydonActionWorkbench::Restore_EditHistory(
	const EDIT_HISTORY_STATE &state, const EDIT_HISTORY_STATE &expected, std::string &status)
{
	if (!state.document || !m_bHasDraft || Is_PublishRunning() || m_bServerPlayPreparationPending)
	{
		status = "Finish the active save or playback preparation before Undo / Redo.";
		return false;
	}
	auto candidate = *state.document;
	// Undo는 메모리 편집본을 바꾼다. 저장 revision은 현재 baseline에 맞추며 디스크를 과거 파일로 되돌리지 않는다.
	candidate.iRevision = m_Document.Get_LastGood().iRevision;
	for (const auto &edit : state.geometry)
	{
		auto *pattern = Find_Pattern(candidate, edit.strPatternId);
		const auto *resource = Find_PresentationResource(candidate, edit.Occurrence.strResourceId);
		if (!pattern || !resource || !Valid_PresentationPlacement(*resource, edit.Occurrence) ||
			!Valid_GameplayColliderScale(*resource, edit.Occurrence))
		{
			status = "Undo / Redo placement failed validation; draft and history are preserved.";
			return false;
		}
		const auto box = std::find_if(
			pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(), [&](const auto &row) {
				return row.strOccurrenceId == edit.Occurrence.strOccurrenceId &&
					row.strResourceId == edit.Occurrence.strResourceId;
			});
		if (box == pattern->PresentationOccurrences.end())
		{
			status = "Undo / Redo placement identity is unavailable; draft and history are preserved.";
			return false;
		}
		Copy_PresentationPlacement(edit.Occurrence, *box, resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
	}
	if (!CKoukuSaydonCompositionDocument::Validate(candidate, m_ResourceReferences, status))
		return false;
	// 아래 선택 문자열은 move할 수 있도록 이력 원본을 복사한다. shared snapshot 자체는 계속 불변으로 남는다.
	auto restored = state;
	if (state.worldDocument && expected.worldDocument && state.worldDocument != expected.worldDocument)
	{
		if (!m_RestoreWorldAnimationHistory ||
			!m_RestoreWorldAnimationHistory(expected.worldDocument, state.worldDocument, status))
			return false;
		m_HistoryWorldDocument = state.worldDocument;
		m_WorldAnimationEditsPending = true;
	}
	// 외부 World 복원까지 성공한 뒤 local 반영을 시작한다. 새 Undo 항목이 생기지 않게 복원 중 flag를 둔다.
	m_HistoryRestoring = true;
	Stop_Preview();
	m_Draft = std::move(candidate);
	m_StagedPresentationGeometry.clear();
	m_bDirty = m_Draft != m_Document.Get_LastGood();
	++m_iDraftGeneration;
	m_ePatternSelection = state.patternSelection;
	m_iBingoParentBox = state.bingoParentBox;
	m_iBingoEntryWaitMs = state.bingoEntryWaitMs;
	m_strBingoParentEntryId = state.bingoParentEntryId;
	m_bBingoParentReturn = state.bingoParentReturn;
	m_strSelectedGateId = std::move(restored.gate);
	m_strSelectedFolderId = std::move(restored.folder);
	m_strSelectedBundleId = std::move(restored.bundle);
	m_strSelectedPatternId = std::move(restored.pattern);
	m_strSelectedStageId = std::move(restored.stage);
	m_strSelectedOccurrenceId = std::move(restored.animation);
	m_strSelectedLogicId = std::move(restored.logic);
	m_strSelectedLogicOccurrenceId = std::move(restored.logicBox);
	m_strSelectedSummonId = std::move(restored.summon);
	m_strSelectedSummonOccurrenceId = std::move(restored.summonBox);
	m_strSelectedWorldId = std::move(restored.world);
	m_strSelectedWorldOccurrenceId = std::move(restored.worldBox);
	m_strSelectedSceneProfileId = std::move(restored.scene);
	m_strSelectedSceneProfileOccurrenceId = std::move(restored.sceneBox);
	m_strSelectedPresentationResourceId = std::move(restored.presentation);
	m_strSelectedPresentationOccurrenceId = std::move(restored.presentationBox);
	m_strSelectedPatternOccurrenceId = std::move(restored.patternBox);
	m_strTimelineSelectionPatternId = std::move(restored.timelinePattern);
	m_TimelineSelectedStageIds = std::move(restored.stages);
	m_TimelineSelectedOccurrenceIds = std::move(restored.occurrences);
	m_strCursorPatternId = std::move(restored.cursorPattern);
	m_iCursorMs = state.cursor;
	// history에는 적용된 정의가 들어 있으므로 오래된 미적용 Logic 입력과 drag 기준은 다시 만든다.
	m_strLogicValueDraftId.clear();
	m_strColliderLogicValueDraftId.clear();
	m_InlineLogicValueDrafts.clear();
	m_bResourceTreeDirty = true;
	m_iTimelineDragMode = 0;
	m_strTimelineGroupDragOccurrenceId.clear();
	Normalize_Selection();
	Synchronize_EditorFields();
	m_HistoryRestoring = false;
	status = "Restored draft and selection. Save keeps this state.";
	return true;
}

// 진행 중 편집을 먼저 닫고 history의 이전 상태를 복원한다. 복원 callback이 거부하면 history 이동도 완료되지 않는다.
bool Client::CKoukuSaydonActionWorkbench::Undo_Edit(std::string &status)
{
	Finish_EditHistory(true);
	const bool changed = m_EditHistory.Undo([&](const auto &state, const auto &expected) {
		return Restore_EditHistory(state, expected, status);
	});
	m_strStatus = status;
	return changed;
}

// Undo 이후 다음 상태를 같은 검증/복원 경계로 적용한다. 결과 진단을 세션에 남긴다.
bool Client::CKoukuSaydonActionWorkbench::Redo_Edit(std::string &status)
{
	Finish_EditHistory(true);
	const bool changed = m_EditHistory.Redo([&](const auto &state, const auto &expected) {
		return Restore_EditHistory(state, expected, status);
	});
	m_strStatus = status;
	return changed;
}

// 버튼과 Ctrl+Z/Y는 방향만 예약한다. pane가 문서 행을 읽는 동안 바로 Undo하여 포인터를 무효화하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Render_EditHistoryButtons(const char *id)
{
	m_HistoryWindowFocused = m_HistoryWindowFocused || ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	ImGui::PushID(id);
	const bool busy = Is_PublishRunning() || m_bServerPlayPreparationPending;
	ImGui::BeginDisabled(busy || !m_EditHistory.Can_Undo());
	if (ImGui::Button("Undo"))
		m_PendingHistoryDirection = -1;
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(busy || !m_EditHistory.Can_Redo());
	if (ImGui::Button("Redo"))
		m_PendingHistoryDirection = 1;
	ImGui::EndDisabled();
	ImGui::PopID();
	if (!busy && m_HistoryWindowFocused && !ImGui::GetIO().WantTextInput && !ImGui::IsAnyItemActive() &&
		ImGui::GetIO().KeyCtrl)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_Z, false))
			m_PendingHistoryDirection = ImGui::GetIO().KeyShift ? 1 : -1;
		else if (ImGui::IsKeyPressed(ImGuiKey_Y, false))
			m_PendingHistoryDirection = 1;
	}
}

// End_WorkbenchFrame 또는 독립 Render 끝에서 요청을 한 번 소비한다. draft 교체 전에 pane-local 참조가 모두 끝나야 한다.
void Client::CKoukuSaydonActionWorkbench::Process_EditHistoryRequest()
{
	Finish_EditHistory();
	const int direction = std::exchange(m_PendingHistoryDirection, 0);
	if (direction < 0)
		(void)Undo_Edit(m_strStatus);
	else if (direction > 0)
		(void)Redo_Edit(m_strStatus);
}
