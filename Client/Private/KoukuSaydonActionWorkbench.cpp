// 세션 수명과 선택, 공용 shell의 pane dispatch를 소유한다.
// 문서 commit/이력/게시와 종류별 저작 UI는 같은 Workbench의 분리 구현에 위임한다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"
#include "KoukuSaydonActionWorkbench_TimelineTypes.h" // 소멸자가 unique_ptr cache의 완전한 타입을 읽는다.
#include "KoukuSaydonPatternAuditionService.h"
#include "GameInstance.h"
#include "Profiler.h"

#include <Windows.h> // 비동기 작업 관찰 handle의 CloseHandle만 사용한다.
#include <algorithm>
#include <array>
#include <utility>

// Internal.h는 여러 CPP가 공유하는 선언이고, 이 using은 이름 조회만 줄인다. 새 객체나 별도 편집 상태를 만들지 않는다.
using namespace Client::KoukuWorkbenchDetail;

// 이름 없는 namespace의 상수·보조 함수는 이 CPP 안에서만 보인다. 분리된 Workbench CPP들이 함께 쓰는 구현은 Helpers.cpp의 이름 있는 범위에 둔다.
namespace
{
using namespace Client;

// 선택지가 고정된 Stage 종류는 크기를 타입에 담는 array로 순서를 보존한다. 문자열 리터럴은 프로그램 수명 동안 살아 있어 포인터만 보관한다.
constexpr std::array<const char_t *, 3u> STAGE_KINDS = {"WINDUP", "ACTIVE", "RECOVERY"};
// 탭은 저작 종류를 고르는 UI다. 각 종류의 재생은 기존 preview 소유자에게 요청한다.
const auto RESOURCE_CATEGORIES = [] {
	std::vector<const char *> result(COMPOSITION_RESOURCE_CATEGORIES.begin(), COMPOSITION_RESOURCE_CATEGORIES.end());
	result.push_back("Subtitle");
	return result;
}();
} // 이 파일의 보조 구현 범위 끝

// MainApp의 두 인스턴스가 서로 다른 source를 읽도록 생성 시 workspace와 문서 경로를 고정한다. 실제 로드는 첫 frame에서 한다.
Client::CKoukuSaydonActionWorkbench::CKoukuSaydonActionWorkbench(const bool sequenceWorkspace)
	: m_bSequenceWorkspace(sequenceWorkspace),
	  m_Document(sequenceWorkspace ? CKoukuSaydonCompositionDocument::Resolve_SequencePath()
								   : CKoukuSaydonCompositionDocument::Resolve_Path())
{
}

// 독립 창을 다시 열고 아직 없는 Animation inventory만 요청한다. 기존 draft와 선택은 유지한다.
void Client::CKoukuSaydonActionWorkbench::Open()
{
	m_bOpen = true;
	m_bResourcesOpen = true;
	if (m_ModelResources.empty())
		m_bResourceRefreshRequested = true;
}

// MainApp이 세션을 해제할 때 관찰 handle을 닫는다. 외부 transaction의 완료/rollback 책임을 강제 종료로 끊지 않는다.
Client::CKoukuSaydonActionWorkbench::~CKoukuSaydonActionWorkbench()
{
	// handle 해제는 process 종료가 아니다. build-domain 소유자가 진행 중인 저장 transaction을 마무리한다.
	if (nullptr != m_hPublishProcess)
		CloseHandle(static_cast<HANDLE>(m_hPublishProcess));
	if (m_hDraftPlayProcess)
		CloseHandle(static_cast<HANDLE>(m_hDraftPlayProcess));
}

// Model View 필터를 바꾼다. 선택이 필터 밖이면 Gate로 돌아가지만 저장된 Pattern의 actor 대상은 수정하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Select_ActorProfile(
	const std::string_view actorProfileId, std::string &outStatus)
{
	if (!actorProfileId.empty() &&
		CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(m_strSelectedGateId, actorProfileId).empty())
	{
		outStatus = "This model is not a target of the selected Gate.";
		return false;
	}
	const std::string model(actorProfileId);
	const auto matchesBundle = [&](const auto &bundle) {
		return model.empty() || std::any_of(bundle.Members.begin(), bundle.Members.end(), [&](const auto &member) {
			const auto *child = Find_Pattern(m_Draft, member.strPatternId);
			return child && child->strGateId == m_strSelectedGateId && child->strActorProfileId == model;
		});
	};
	bool visible = true;
	if (!model.empty() && m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN)
	{
		const auto *selected = Find_Pattern(m_Draft, m_strSelectedPatternId);
		visible = selected &&
			(selected->strActorProfileId == model ||
				std::any_of(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto &bundle) {
					return matchesBundle(bundle) &&
						std::any_of(bundle.Members.begin(), bundle.Members.end(), [&](const auto &member) {
							return member.strPatternId == m_strSelectedPatternId;
						});
				}));
	}
	else if (!model.empty() && m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE)
		visible = std::any_of(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto &bundle) {
			return bundle.strBundleId == m_strSelectedBundleId && matchesBundle(bundle);
		});
	else if (!model.empty() && m_ePatternSelection == KOUKU_PATTERN_SELECTION::FOLDER)
		visible = std::any_of(m_Draft.Bundles.begin(), m_Draft.Bundles.end(),
					  [&](const auto &bundle) {
						  return bundle.strFolderId == m_strSelectedFolderId && matchesBundle(bundle);
					  }) ||
			std::any_of(m_Draft.Patterns.begin(), m_Draft.Patterns.end(), [&](const auto &pattern) {
				return pattern.strGateId == m_strSelectedGateId && pattern.strFolderId == m_strSelectedFolderId &&
					pattern.strActorProfileId == model;
			});
	if (!visible)
		Select_Hierarchy(KOUKU_PATTERN_SELECTION::GATE, m_strSelectedGateId);
	m_strModelViewProfile = model;
	if (!model.empty())
		m_strCreateActorProfileId = model;
	outStatus = "Model View changed; saved targets are unchanged.";
	return true;
}

// 공용 shell의 보스 종류를 저작 Gate 선택으로 변환한다. Bingo는 부모 Flow를 선택하며 서버 월드 전환은 요청하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Select_WorkbenchBoss(const COMPOSITION_WORKBENCH_BOSS boss)
{
	std::string gate = "GATE1";
	switch (boss)
	{
	case COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_GATE2:
		gate = "GATE2";
		break;
	case COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_GATE3:
		gate = "GATE3";
		break;
	case COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_ENCORE:
		gate = "BINGO";
		break;
	default:
		break;
	}
	if (gate != m_strSelectedGateId)
	{
		Select_Hierarchy(KOUKU_PATTERN_SELECTION::GATE, gate);
		if (gate == "BINGO")
		{
			std::string status;
			(void)Select_BingoParent(status);
		}
	}
}

// tree/deep link가 Pattern을 고른다. 존재 확인 후 부모·Bundle 문맥과 box 선택을 재구성하고 다른 Pattern의 preview를 중지 요청한다.
bool_t Client::CKoukuSaydonActionWorkbench::Select_PatternById(const std::string_view patternId, std::string &outStatus)
{
	const KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(m_Draft, patternId);
	if (nullptr == pattern)
	{
		outStatus = "KoukuSaydon Pattern selection is not in the authoring document.";
		return false;
	}
	const bool patternChanged =
		m_strSelectedPatternId != patternId || m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN;
	if (patternChanged)
	{
		Stop_Preview();
		m_bPresentationPreviewRequestPending = false;
	}
	m_ePatternSelection = KOUKU_PATTERN_SELECTION::PATTERN;
	m_strSelectedGateId = pattern->strGateId;
	m_strCreateActorProfileId = pattern->strActorProfileId;
	const auto context = std::find_if(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto &bundle) {
		return bundle.strBundleId == m_strBundleReturnId &&
			std::any_of(bundle.Members.begin(), bundle.Members.end(), [&](const auto &member) {
				return member.strPatternId == patternId;
			});
	});
	m_strSelectedFolderId = context == m_Draft.Bundles.end() ? pattern->strFolderId : context->strFolderId;
	m_strCreateFolderId = m_strSelectedFolderId;
	m_strSelectedBundleId = context == m_Draft.Bundles.end() ? std::string{} : context->strBundleId;
	m_strCreateBundleId = m_strSelectedBundleId;
	m_strBundleReturnId = m_strSelectedBundleId;
	Clear_TimelineSelection();
	if (!Find_PatternBox(*pattern, m_strSelectedPatternOccurrenceId))
		m_strSelectedPatternOccurrenceId.clear();
	m_strSelectedActorProfileId = pattern->strActorProfileId;
	if (patternChanged)
		m_bFitRequested = true;
	m_strSelectedPatternId = std::string(patternId);
	m_strSelectedStageId = pattern->Stages.empty() ? std::string{} : pattern->Stages.front().strStageId;
	m_strSelectedOccurrenceId.clear();
	Normalize_Selection();
	Synchronize_EditorFields();
	outStatus = "Selected KoukuSaydon Pattern " + m_strSelectedPatternId + ".";
	return true;
}

// 월드 picking 결과를 stable ID로 선택한다. 이미 같은 box면 미적용 상세 입력을 재동기화하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Select_WorldBoxById(
	const std::string_view patternId, const std::string_view occurrenceId, std::string &outStatus)
{
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	if (!pattern || !Find_WorldBox(*pattern, occurrenceId))
	{
		outStatus = "The selected World box is unavailable in this authoring document.";
		return false;
	}
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN && m_strSelectedPatternId == patternId &&
		m_strSelectedWorldOccurrenceId == occurrenceId)
	{
		outStatus = "The World box is already selected; pending edits are preserved.";
		return true;
	}
	const std::string selectedPattern(patternId), selectedOccurrence(occurrenceId);
	if (m_strSelectedPatternId != selectedPattern || m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN)
		if (!Select_PatternById(selectedPattern, outStatus))
			return false;
	Select_TimelineBox({}, selectedOccurrence, false);
	m_WorkbenchViewRequest.restoreSequencer = true;
	outStatus = m_strStatus = "Selected World box " + selectedOccurrence + ".";
	return true;
}

// 외부 picking/deep link를 Presentation 선택으로 연결한다. 같은 box 재선택도 저장 group을 다시 확장하되 입력은 유지한다.
bool_t Client::CKoukuSaydonActionWorkbench::Select_PresentationBoxById(
	const std::string_view patternId, const std::string_view occurrenceId, std::string &outStatus)
{
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	const auto *box = pattern ? Find_PresentationBox(*pattern, occurrenceId) : nullptr;
	if (!box)
	{
		outStatus = "The selected presentation box is unavailable in this authoring document.";
		return false;
	}
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN && m_strSelectedPatternId == patternId &&
		m_strSelectedPresentationOccurrenceId == occurrenceId)
	{
		// 같은 행을 다시 열어도 저장된 group 전체를 선택 집합으로 다시 확장한다.
		if (!box->strSelectionGroupId.empty())
		{
			m_TimelineSelectedStageIds.clear();
			m_TimelineSelectedOccurrenceIds = {std::string(occurrenceId)};
			Expand_PresentationSelection(*pattern, m_TimelineSelectedOccurrenceIds);
			m_strTimelineSelectionPatternId = std::string(patternId);
		}
		outStatus = "The presentation box is already selected; pending edits are preserved.";
		return true;
	}
	const std::string selectedPattern(patternId), selectedOccurrence(occurrenceId);
	if (m_strSelectedPatternId != selectedPattern || m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN)
		if (!Select_PatternById(selectedPattern, outStatus))
			return false;
	Select_TimelineBox({}, selectedOccurrence, false);
	m_WorkbenchViewRequest.restoreSequencer = true;
	outStatus = m_strStatus = "Selected presentation box " + selectedOccurrence + ".";
	return true;
}

// group을 삭제하지 않고 그 안의 Collider 하나만 상세 조절 대상으로 좁힌다. 일반 선택은 이후 다시 group을 확장할 수 있다.
bool_t Client::CKoukuSaydonActionWorkbench::Select_ColliderGroupMemberById(
	const std::string_view patternId, const std::string_view occurrenceId, std::string &outStatus)
{
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	const auto *box = pattern ? Find_PresentationBox(*pattern, occurrenceId) : nullptr;
	const auto *resource = box ? Find_PresentationResource(m_Draft, box->strResourceId) : nullptr;
	if (!pattern || !pattern->strLoadError.empty() || !resource ||
		resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER || box->strSelectionGroupId.empty())
	{
		outStatus = m_strStatus = "Select a Collider that belongs to a saved group.";
		return false;
	}
	const std::string memberId(occurrenceId);
	if (!Select_PresentationBoxById(patternId, memberId, outStatus))
		return false;
	// 저장된 group 소속은 유지한다. 현재 조절 선택만 좁히고 일반 click/Back to Group은 전체를 다시 고른다.
	m_TimelineSelectedStageIds.clear();
	m_TimelineSelectedOccurrenceIds = {memberId};
	m_strColliderSelectionTransformKey.clear();
	m_WorkbenchViewRequest.restoreSequencer = true;
	outStatus = m_strStatus = "Editing one Collider. Its group is preserved; Back to Group selects every member.";
	return true;
}

// commit/reload/Undo 뒤 stable ID를 현재 문서에서 다시 검증한다. 사라진 행만 비워 교체 전 vector 위치를 재사용하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Normalize_Selection()
{
	const bool missingBundle = m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE &&
		std::none_of(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto &item) {
			return item.strBundleId == m_strSelectedBundleId;
		});
	const bool missingFolder = m_ePatternSelection == KOUKU_PATTERN_SELECTION::FOLDER &&
		std::none_of(m_Draft.Folders.begin(), m_Draft.Folders.end(), [&](const auto &item) {
			return item.strFolderId == m_strSelectedFolderId;
		});
	if (missingBundle || missingFolder ||
		(m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN && !Find_Pattern(m_Draft, m_strSelectedPatternId)))
		Select_Hierarchy(KOUKU_PATTERN_SELECTION::GATE, m_strSelectedGateId);
	if (!m_strSelectedLogicId.empty() && nullptr == Find_Logic(m_Draft, m_strSelectedLogicId))
		m_strSelectedLogicId.clear();
	if (!m_strSelectedSummonId.empty() && nullptr == Find_Summon(m_Draft, m_strSelectedSummonId))
		m_strSelectedSummonId.clear();
	if (!m_strSelectedWorldId.empty() && nullptr == Find_World(m_Draft, m_strSelectedWorldId))
		m_strSelectedWorldId.clear();
	if (!m_strSelectedSceneProfileId.empty() && nullptr == Find_SceneProfile(m_Draft, m_strSelectedSceneProfileId))
		m_strSelectedSceneProfileId.clear();
	if (!m_strSelectedPresentationResourceId.empty() &&
		nullptr == Find_PresentationResource(m_Draft, m_strSelectedPresentationResourceId))
		m_strSelectedPresentationResourceId.clear();
	if (!m_strLogicValueDraftId.empty() && nullptr == Find_Logic(m_Draft, m_strLogicValueDraftId))
		m_strLogicValueDraftId.clear();
	if (!m_strColliderLogicValueDraftId.empty() && nullptr == Find_Logic(m_Draft, m_strColliderLogicValueDraftId))
		m_strColliderLogicValueDraftId.clear();
	const KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
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
		m_strSelectedPatternOccurrenceId.clear();
		return;
	}
	if (!m_strSelectedLogicOccurrenceId.empty() && nullptr == Find_LogicBox(*pattern, m_strSelectedLogicOccurrenceId))
	{
		m_strSelectedLogicOccurrenceId.clear();
	}
	if (!m_strSelectedSummonOccurrenceId.empty() &&
		nullptr == Find_SummonBox(*pattern, m_strSelectedSummonOccurrenceId))
	{
		m_strSelectedSummonOccurrenceId.clear();
	}
	if (!m_strSelectedWorldOccurrenceId.empty() && nullptr == Find_WorldBox(*pattern, m_strSelectedWorldOccurrenceId))
	{
		m_strSelectedWorldOccurrenceId.clear();
	}
	if (!m_strSelectedSceneProfileOccurrenceId.empty() &&
		nullptr == Find_SceneProfileBox(*pattern, m_strSelectedSceneProfileOccurrenceId))
		m_strSelectedSceneProfileOccurrenceId.clear();
	if (!m_strSelectedPresentationOccurrenceId.empty() &&
		nullptr == Find_PresentationBox(*pattern, m_strSelectedPresentationOccurrenceId))
		m_strSelectedPresentationOccurrenceId.clear();
	if (!Find_PatternBox(*pattern, m_strSelectedPatternOccurrenceId))
		m_strSelectedPatternOccurrenceId.clear();
	m_strSelectedActorProfileId = pattern->strActorProfileId;
	if (m_strTimelineSelectionPatternId != m_strSelectedPatternId)
	{
		Clear_TimelineSelection();
		m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	}
	std::erase_if(m_TimelineSelectedStageIds, [pattern](const auto &id) {
		return nullptr == Find_Stage(*pattern, id);
	});
	std::erase_if(m_TimelineSelectedOccurrenceIds, [pattern](const auto &id) {
		return !Has_TimelineOccurrence(*pattern, id);
	});
	if (!m_strSelectedStageId.empty() && nullptr == Find_Stage(*pattern, m_strSelectedStageId))
	{
		m_strSelectedStageId.clear();
	}
	if (!m_strSelectedOccurrenceId.empty())
	{
		const KOUKU_SAYDON_COMPOSITION_STAGE *occurrenceStage = nullptr;
		if (nullptr == Find_Occurrence(*pattern, m_strSelectedOccurrenceId, &occurrenceStage))
		{
			m_strSelectedOccurrenceId.clear();
		}
		else if (m_strSelectedStageId.empty() && nullptr != occurrenceStage)
		{
			m_strSelectedStageId = occurrenceStage->strStageId;
		}
	}
}

// 선택한 문서값을 ImGui 입력 buffer로 옮긴다. 끝난 picking을 취소하고 아직 다른 geometry overlay는 유지해 상세 편집 손실을 막는다.
void Client::CKoukuSaydonActionWorkbench::Synchronize_EditorFields()
{
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BINGO_PARENT && m_iBingoParentBox == 2)
		if (const auto *flow = Find_BingoFlow(m_Draft))
			for (const auto &entry : flow->Entries)
				if (entry.strEntryId == m_strBingoParentEntryId)
					m_iBingoEntryWaitMs = static_cast<int>(entry.iWaitAfterMs);
	// 문서/선택을 다시 읽는 순간의 이전 picking 결과는 현재 입력에 적용할 수 없다.
	Cancel_MapEffectPlacementRequest();
	Cancel_PresentationGeometryPreview(false);
	// 사라진 box 또는 이미 문서에 반영된 배치만 overlay에서 제거한다.
	std::erase_if(m_StagedPresentationGeometry, [&](const auto &row) {
		const auto *owner = Find_Pattern(m_Draft, row.strPatternId);
		const auto *box = owner ? Find_PresentationBox(*owner, row.Occurrence.strOccurrenceId) : nullptr;
		const auto *resource = box ? Find_PresentationResource(m_Draft, box->strResourceId) : nullptr;
		return !box || box->strResourceId != row.Occurrence.strResourceId ||
			Same_PresentationPlacement(
				*box, row.Occurrence, resource && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
	});
	m_PatternName[0] = '\0';
	m_iPatternDurationMs = 0;
	m_iOccurrenceStartOffsetMs = 0;
	m_iOccurrenceSourceStartMs = 0;
	m_iOccurrenceSourceEndMs = 0;
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
	const KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (nullptr == pattern)
		return;
	(void)Copy_Text(m_PatternName, std::size(m_PatternName), pattern->strDisplayName);
	m_iPatternDurationMs = static_cast<int32_t>(Pattern_DurationMs(*pattern));
	if (const auto *box = Find_PatternBox(*pattern, m_strSelectedPatternOccurrenceId))
	{
		m_iPatternBoxStartMs = static_cast<int32_t>(box->iStartMs);
		m_iPatternBoxDurationMs = static_cast<int32_t>(box->iDurationMs);
		m_bPatternBoxRepeat = box->bRepeat;
	}
	if (const auto *box = Find_PresentationBox(*pattern, m_strSelectedPresentationOccurrenceId))
	{
		m_PresentationBoxEdit = *box;
		const auto staged = std::find_if(
			m_StagedPresentationGeometry.begin(), m_StagedPresentationGeometry.end(), [&](const auto &row) {
				return row.strPatternId == pattern->strPatternId &&
					row.Occurrence.strOccurrenceId == box->strOccurrenceId;
			});
		const auto *resource = Find_PresentationResource(m_Draft, box->strResourceId);
		if (staged != m_StagedPresentationGeometry.end())
			Copy_PresentationPlacement(staged->Occurrence, m_PresentationBoxEdit,
				resource && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
	}
	if (const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE *const logicBox =
			Find_LogicBox(*pattern, m_strSelectedLogicOccurrenceId);
		nullptr != logicBox)
	{
		m_iLogicBoxStartMs = static_cast<int32_t>(logicBox->iStartMs);
		m_iLogicBoxDurationMs = static_cast<int32_t>(logicBox->iDurationMs);
	}
	if (const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE *const summonBox =
			Find_SummonBox(*pattern, m_strSelectedSummonOccurrenceId);
		nullptr != summonBox)
	{
		m_iSummonBoxStartMs = static_cast<int32_t>(summonBox->iStartMs);
		m_iSummonBoxDurationMs = static_cast<int32_t>(summonBox->iDurationMs);
	}
	if (const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE *const worldBox =
			Find_WorldBox(*pattern, m_strSelectedWorldOccurrenceId);
		nullptr != worldBox)
	{
		m_iWorldBoxStartMs = static_cast<int32_t>(worldBox->iStartMs);
		m_iWorldBoxDurationMs = static_cast<int32_t>(worldBox->iDurationMs);
		m_fWorldBoxPlaybackSpeed = worldBox->fPlaybackSpeed;
	}
	if (const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE *const sceneBox =
			Find_SceneProfileBox(*pattern, m_strSelectedSceneProfileOccurrenceId);
		nullptr != sceneBox)
	{
		m_iSceneProfileBoxStartMs = static_cast<int32_t>(sceneBox->iStartMs);
		m_iSceneProfileBoxDurationMs = static_cast<int32_t>(sceneBox->iDurationMs);
		m_iSceneProfileBoxBlendMs = static_cast<int32_t>(sceneBox->iBlendMs);
	}
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE *const occurrence =
		Find_Occurrence(*pattern, m_strSelectedOccurrenceId);
	if (nullptr == occurrence)
		return;
	m_iOccurrenceStartOffsetMs = static_cast<int32_t>(occurrence->iStartOffsetMs);
	m_iOccurrenceSourceStartMs = static_cast<int32_t>(occurrence->iSourceStartMs);
	m_iOccurrenceSourceEndMs = static_cast<int32_t>(occurrence->iSourceEndMs);
	m_iOccurrencePlayMs = static_cast<int32_t>(occurrence->iPlayMs);
	m_iOccurrenceBlendInMs = static_cast<int32_t>(occurrence->iBlendInMs);
	m_fOccurrencePlayRate = occurrence->fPlayRate;
	for (int32_t index = 0; index < static_cast<int32_t>(END_POLICIES.size()); ++index)
	{
		if (occurrence->strEndPolicy == END_POLICIES[index])
			m_iOccurrenceEndPolicy = index;
	}
}

// shell toolbar 또는 독립 창이 호출한다. 저장·재생 요청과 표시 설정만 전달하고 실제 실행 완료는 background/preview 상태로 읽는다.
void Client::CKoukuSaydonActionWorkbench::Render_Toolbar()
{
	Engine::CProfilerScope paneScope(CGameInstance::Get().Get_Profiler(), "ImGui.Composition.Toolbar");
	const bool_t publishing = Is_PublishRunning();
	if (ImGui::Button("Composition Camera"))
		m_bCameraWindowOpen = true;
	ImGui::SameLine();
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
	ImGui::BeginDisabled(publishing || !m_bHasDraft || !Is_Dirty());
	if (ImGui::Button("Save"))
	{
		std::string status;
		(void)Save(status);
	}
	ImGui::EndDisabled();

	if (!m_bSequenceWorkspace)
	{
		ImGui::SameLine();
		const auto *selectedPattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
		const auto bundle = std::find_if(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto &item) {
			return item.strBundleId == m_strSelectedBundleId;
		});
		const bool bundleSelected =
			m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE && bundle != m_Draft.Bundles.end();
		const bool executionSelected = bundleSelected ? bundle->strLoadError.empty()
													  : (m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN &&
															selectedPattern && selectedPattern->strLoadError.empty());
		ImGui::BeginDisabled(publishing || Is_Dirty() || !m_Document.Is_Fresh() || !executionSelected);
		if (selectedPattern && selectedPattern->strGateId == "GATE3")
		{
			auto &audition = CKoukuSaydonPatternAuditionService::Get();
			int testStage = audition.Get_MarioTestStage(), testSeed = static_cast<int>(audition.Get_MarioTestSeed());
			ImGui::SetNextItemWidth(100.f);
			bool changed = ImGui::SliderInt("Mario test stage (0 = live)", &testStage, 0, 4);
			ImGui::SetNextItemWidth(100.f);
			changed = ImGui::InputInt("Mario test seed", &testSeed) || changed;
			if (changed)
				audition.Set_MarioTest(
					static_cast<std::uint8_t>(testStage), static_cast<std::uint32_t>((std::max)(0, testSeed)));
			ImGui::TextDisabled(
				"Test fixture applies only to an entry pattern. Enter its collider to test the same Server route.");
		}
		ImGui::EndDisabled();
		if (ImGui::Button("Complete Play (Server)"))
		{
			std::string status;
			(void)Request_SelectedServerPlay(status);
		}
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
	ImGui::SliderFloat(
		"Zoom##KoukuTimeline", &m_fPixelsPerSecond, 1.f, 500.f, "%.0f px/s", ImGuiSliderFlags_Logarithmic);

	const char_t *const freshness =
		m_Document.Is_Fresh() ? "fresh" : (m_Document.Has_LastGood() ? "stale/last-good" : "cold");
	ImGui::Text("rev %u | %s%s | %s", m_bHasDraft ? m_Draft.iRevision : 0u, freshness, Is_Dirty() ? " | dirty" : "",
		m_strStatus.c_str());
}

// 독립 Render 경로의 보조 창이다. 공유 shell에서는 별도 RESOURCES pane가 같은 내용을 호스팅한다.
void Client::CKoukuSaydonActionWorkbench::Render_ResourcesWindow()
{
	if (!m_bResourcesOpen || !m_bOpen)
		return;
	ImGui::SetNextWindowSize(ImVec2(540.f, 820.f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(m_bSequenceWorkspace ? "Composition Resources###KoukuSequenceResources"
										  : "Composition Resources###KoukuCompositionResources",
			&m_bResourcesOpen))
		Render_ResourceTree();
	ImGui::End();
}

// 자원 탭의 선택을 종류별 저작 UI로 보낸다. 자원 선택과 현재 timeline 편집 대상은 분리해 유지한다.
void Client::CKoukuSaydonActionWorkbench::Render_ResourceTree()
{
	Engine::CProfilerScope paneScope(CGameInstance::Get().Get_Profiler(), "ImGui.Composition.Resources");
	// Resources에서 고르는 source가 현재 편집 timeline의 선택을 바꾸지는 않는다.
	if (!ImGui::BeginTabBar("##KoukuResourceCategories"))
		return;
	for (int32_t index = 0; index < static_cast<int32_t>(RESOURCE_CATEGORIES.size()); ++index)
	{
		const bool selectPatternTab =
			m_bPatternResourceTabRequested && std::string_view(RESOURCE_CATEGORIES[index]) == "Pattern";
		if (!ImGui::BeginTabItem(RESOURCE_CATEGORIES[index], nullptr,
				selectPatternTab ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
			continue;
		if (selectPatternTab)
			m_bPatternResourceTabRequested = false;
		m_iSelectedResourceCategory = index;
		const std::string_view category = RESOURCE_CATEGORIES[index];
		if ("Pattern" == category)
			Render_PatternResources();
		else if ("Animation" == category)
			Render_AnimationResources();
		else if ("Logic" == category)
			Render_LogicResources();
		else if ("Summon" == category)
			Render_SummonResources();
		else if ("World" == category)
			Render_WorldResources();
		else if ("Scene Profile" == category)
			Render_SceneProfileResources();
		else if ("Effect" == category)
			Render_PresentationResources(KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
		else if ("Collider" == category)
			Render_PresentationResources(KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER);
		else if ("Sound" == category)
			Render_PresentationResources(KOUKU_SAYDON_PRESENTATION_KIND::SOUND);
		else if ("Subtitle" == category)
			Render_PresentationResources(KOUKU_SAYDON_PRESENTATION_KIND::SUBTITLE);
		else if ("Camera" == category)
			Render_PresentationResources(KOUKU_SAYDON_PRESENTATION_KIND::CAMERA);
		else if ("Light" == category)
			Render_PresentationResources(KOUKU_SAYDON_PRESENTATION_KIND::LIGHT);
		else
			ImGui::TextDisabled(
				"%s rows are not authorable yet. Their save and playback consumer lands in a later slice.",
				RESOURCE_CATEGORIES[index]);
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();
}

// 선택 종류에 따라 전용 상세 편집기를 고르고, Pattern 공통값 뒤 Stage/Animation 입력을 표시한다.
// 편집 함수가 draft를 교체할 수 있으므로 적용한 분기는 즉시 return하여 이전 pattern/stage/occurrence 포인터를 다시 읽지 않는다.
void Client::CKoukuSaydonActionWorkbench::Render_Details()
{
	Engine::CProfilerScope paneScope(CGameInstance::Get().Get_Profiler(), "ImGui.Composition.Details");
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BINGO_PARENT)
	{
		Render_BingoParentDetails();
		return;
	}
	if (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN)
	{
		Render_HierarchyDetails();
		return;
	}
	const KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
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
		if (ImGui::Button(
				m_bSequenceWorkspace ? "Delete Invalid Sequence###Delete Invalid Pattern" : "Delete Invalid Pattern"))
		{
			Request_PatternDelete(patternId);
		}
		return;
	}
	// 복수 선택과 Collider 단일 선택은 전용 상세가 선점한다. 이후 공통 입력으로 같은 frame에 중복 적용하지 않는다.
	if (Render_ColliderGroupDetails(*pattern) || Render_EffectGroupDetails(*pattern) ||
		Render_SoundGroupDetails(*pattern))
		return;
	if (const auto *selected = Find_PresentationBox(*pattern, m_strSelectedPresentationOccurrenceId))
		if (const auto *resource = Find_PresentationResource(m_Draft, selected->strResourceId);
			resource && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
		{
			Render_ColliderBoxDetails(*pattern);
			return;
		}
	if (!m_strParentReturnPatternId.empty() && m_strParentReturnPatternId != patternId &&
		Find_Pattern(m_Draft, m_strParentReturnPatternId))
	{
		if (ImGui::Button("Back to Parent"))
		{
			const auto returnId = m_strParentReturnPatternId;
			m_strParentReturnPatternId.clear();
			std::string status;
			(void)Select_PatternById(returnId, status);
			return;
		}
	}
	if (!m_strBundleReturnId.empty() && Find_Bundle(m_Draft, m_strBundleReturnId))
	{
		if (ImGui::Button("Back to Bundle"))
		{
			Select_Hierarchy(KOUKU_PATTERN_SELECTION::BUNDLE, m_strBundleReturnId);
			return;
		}
	}
	if (const auto *owner = Find_TimelineParent(m_Draft, patternId))
	{
		ImGui::TextWrapped("Parent timeline: %s", owner->strDisplayName.c_str());
		const bool empty = pattern->Stages.empty() && pattern->PatternOccurrences.empty() &&
			pattern->LogicOccurrences.empty() && pattern->PresentationOccurrences.empty() &&
			pattern->WorldOccurrences.empty() && pattern->SceneProfileOccurrences.empty() &&
			pattern->SummonOccurrences.empty();
		if (empty && ImGui::Button("Remove Empty Parent Timeline"))
		{
			const auto folderId = owner->strFolderId;
			auto candidate = m_Draft;
			for (auto &folder : candidate.Folders)
				if (folder.strFolderId == folderId)
					folder.strTimelinePatternId.clear();
			std::erase_if(candidate.Patterns, [&](const auto &row) {
				return row.strPatternId == patternId;
			});
			Rebuild_PlayAllPatternIds(candidate);
			std::string status;
			if (Commit_Candidate(
					std::move(candidate), "Removed the empty timeline; Parent and child Patterns remain.", status))
				Select_Hierarchy(KOUKU_PATTERN_SELECTION::FOLDER, folderId);
			return;
		}
	}
	if (Render_PatternProperties(pattern, patternId))
		return;

	if (Render_PatternReferenceControls(*pattern))
		return;
	if (!m_strSelectedPatternOccurrenceId.empty())
		return;
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
		// Logic box 전용 상세를 그린다. Stage/Animation 선택으로 돌아가기 전에는 아래 입력을 함께 그리지 않는다.
		Render_LogicBoxDetails(*pattern);
		return;
	}

	const auto *stage = Render_StageProperties(pattern, patternId);
	if (nullptr == stage)
		return;
	Render_AnimationProperties(pattern, stage, patternId);
}

// Pattern 공통 속성을 표시한다. true면 편집/삭제 요청 분기를 처리했으므로 호출자가 이번 상세 frame을 끝낸다.
// pattern은 caller의 현재 draft 차용이다. commit 뒤 true로 즉시 돌아가 이전 행을 더 이상 사용하지 않는다.
bool Client::CKoukuSaydonActionWorkbench::Render_PatternProperties(
	const KOUKU_SAYDON_COMPOSITION_PATTERN *pattern, const std::string &patternId)
{
	ImGui::TextWrapped("%s | %s", Gate_Label(pattern->strGateId), pattern->strTargetBossPlacementId.c_str());
	ImGui::TextDisabled("%s ID: %s", m_bSequenceWorkspace ? "Sequence" : "Pattern", patternId.c_str());
	ImGui::SeparatorText(m_bSequenceWorkspace ? "Sequence" : "Pattern");
	if (ImGui::InputText("Display Name", m_PatternName, std::size(m_PatternName), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		std::string status;
		(void)Rename_Pattern(patternId, m_PatternName, status);
		return true;
	}
	const auto *parent = Find_Folder(m_Draft, pattern->strFolderId);
	std::string requestedParent;
	bool_t changeParent = false;
	if (ImGui::BeginCombo("Parent##Pattern", parent ? parent->strDisplayName.c_str() : "None (Gate root)"))
	{
		if (!Find_TimelineParent(m_Draft, patternId) &&
			ImGui::Selectable("None (Gate root)", pattern->strFolderId.empty()))
			changeParent = true;
		for (const auto &folder : m_Draft.Folders)
			if (folder.strGateId == pattern->strGateId && folder.strLoadError.empty() &&
				ImGui::Selectable((folder.strDisplayName + "##" + folder.strFolderId).c_str(),
					folder.strFolderId == pattern->strFolderId))
			{
				requestedParent = folder.strFolderId;
				changeParent = true;
			}
		ImGui::EndCombo();
	}
	if (changeParent)
	{
		std::string status;
		(void)Set_PatternFolder(patternId, requestedParent, status);
		return true;
	}
	std::string requestedCategory;
	if (ImGui::BeginCombo("Category", pattern->strCategory.c_str()))
	{
		// value는 읽기 전용 문자를 가리키는 재지정 불가 포인터다. 이 반복에서는 공용 선택지의 글자와 포인터를 모두 바꾸지 않는다.
		for (const char_t *const value : PATTERN_CATEGORIES)
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
		return true;
	}
	if (!m_bSequenceWorkspace)
		ImGui::TextDisabled("Publish All Patterns checks whether this Pattern is ready for Server playback.");
	const bool hasStartAnimation = !pattern->Stages.empty() && !pattern->Stages.front().AnimationOccurrences.empty();
	int32_t startOffsetMs = 0;
	if (hasStartAnimation)
	{
		const auto &animations = pattern->Stages.front().AnimationOccurrences;
		startOffsetMs = static_cast<int32_t>(
			std::min_element(animations.begin(), animations.end(), [](const auto &a, const auto &b) {
				return a.iStartOffsetMs < b.iStartOffsetMs;
			})->iStartOffsetMs);
	}
	ImGui::BeginDisabled(!hasStartAnimation);
	const bool startOffsetChanged = ImGui::InputInt("Pattern Start Offset ms", &startOffsetMs, 10, 100);
	ImGui::EndDisabled();
	if (startOffsetChanged)
	{
		std::string status;
		(void)Set_PatternStartOffset(patternId, static_cast<std::uint32_t>((std::max)(0, startOffsetMs)), status);
		return true;
	}
	ImGui::TextWrapped(hasStartAnimation
			? "Delays animation and following Stages. Effect, Logic and other box times stay fixed. The first pose is held during the delay."
			: "Add an animation to the first Stage to set its start delay.");
	bool resetToSpawn = pattern->bResetBossToSpawn;
	if (ImGui::Checkbox("Reset boss to spawn at Pattern start", &resetToSpawn))
	{
		auto candidate = m_Draft;
		auto *edited = Find_Pattern(candidate, patternId);
		if (nullptr != edited)
		{
			edited->bResetBossToSpawn = resetToSpawn;
			if (resetToSpawn)
				edited->BossMotion.reset();
			if (!resetToSpawn)
				edited->ResetBossYawDegrees.reset();
			Mark_Draft(candidate, *edited);
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated boss spawn reset for this Pattern.", status);
		}
		return true;
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
			auto *edited = Find_Pattern(candidate, patternId);
			if (nullptr != edited)
			{
				edited->ResetBossYawDegrees = resetYaw ? std::optional<double>{yaw} : std::nullopt;
				Mark_Draft(candidate, *edited);
				std::string status;
				(void)Commit_Candidate(std::move(candidate), "Updated fixed boss facing for this Pattern.", status);
			}
			return true;
		}
	}
	float rootVerticalScale = float(pattern->fAnimationRootVerticalScale);
	if (ImGui::DragFloat(
			"Animation jump height", &rootVerticalScale, 0.01f, 0.f, 1.f, "%.2fx", ImGuiSliderFlags_AlwaysClamp))
	{
		auto candidate = m_Draft;
		if (auto *edited = Find_Pattern(candidate, patternId))
		{
			edited->fAnimationRootVerticalScale = rootVerticalScale;
			Mark_Draft(candidate, *edited);
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated animation jump height for this Pattern.", status);
		}
		return true;
	}
	ImGui::TextDisabled("Scales root height only; body size and animation timing stay the same.");
	float rootHorizontalScale = float(pattern->fAnimationRootHorizontalScale);
	if (ImGui::DragFloat(
			"Animation ground travel", &rootHorizontalScale, 0.01f, 0.f, 1.f, "%.2fx", ImGuiSliderFlags_AlwaysClamp))
	{
		auto candidate = m_Draft;
		if (auto *edited = Find_Pattern(candidate, patternId))
		{
			edited->fAnimationRootHorizontalScale = rootHorizontalScale;
			Mark_Draft(candidate, *edited);
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated animation ground travel for this Pattern.", status);
		}
		return true;
	}
	ImGui::TextDisabled("0 keeps this attack in place; explicit charge movement is edited in its Logic box.");
	bool moveBoss = pattern->BossMotion.has_value();
	if (ImGui::Checkbox("Move boss during Pattern", &moveBoss))
	{
		auto candidate = m_Draft;
		auto *edited = Find_Pattern(candidate, patternId);
		if (edited)
		{
			if (moveBoss)
			{
				edited->BossMotion = KOUKU_SAYDON_BOSS_MOTION{};
				edited->BossMotion->iEndMs = (std::max)(1u, Pattern_DurationMs(*edited));
				edited->bResetBossToSpawn = false;
				edited->ResetBossYawDegrees.reset();
			}
			else
				edited->BossMotion.reset();
			Mark_Draft(candidate, *edited);
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated boss movement for this Pattern.", status);
		}
		return true;
	}
	// 가져온 위치 key가 있으면 시간/위치를 원본대로 보존하고 yaw만 편집한다.
	if (pattern->BossMotion)
	{
		auto motion = *pattern->BossMotion;
		int startMs = int(motion.iStartMs), endMs = int(motion.iEndMs);
		float startXZ[2] = {float(motion.StartPosition[0]), float(motion.StartPosition[2])};
		float endXZ[2] = {float(motion.EndPosition[0]), float(motion.EndPosition[2])};
		float baseY = float(motion.StartPosition[1]), yaw = float(motion.fYawDegrees);
		const bool sampled = !motion.Keys.empty();
		ImGui::BeginDisabled(sampled);
		bool changed = ImGui::DragInt("Move start (ms)", &startMs, 1.f, 0, (std::max)(0, endMs - 1));
		changed =
			ImGui::DragInt("Move end (ms)", &endMs, 1.f, startMs + 1, int(Pattern_DurationMs(*pattern))) || changed;
		changed = ImGui::DragFloat2("Boss start XZ", startXZ, 0.05f, -100000.f, 100000.f) || changed;
		changed = ImGui::DragFloat2("Boss end XZ", endXZ, 0.05f, -100000.f, 100000.f) || changed;
		changed = ImGui::DragFloat("Boss base Y", &baseY, 0.05f, -100000.f, 100000.f) || changed;
		ImGui::EndDisabled();
		if (sampled)
			ImGui::TextDisabled(
				"%zu imported position keys; timeline and XYZ are preserved on Save.", motion.Keys.size());
		changed = ImGui::DragFloat("Boss motion yaw", &yaw, 0.1f, -360.f, 360.f, "%.4f") || changed;
		ImGui::TextDisabled("Holds the end position. Original animation supplies vertical pose.");
		if (changed)
		{
			if (!sampled)
			{
				motion.iStartMs = std::uint32_t(startMs);
				motion.iEndMs = std::uint32_t(endMs);
				motion.StartPosition = {startXZ[0], baseY, startXZ[1]};
				motion.EndPosition = {endXZ[0], baseY, endXZ[1]};
			}
			motion.fYawDegrees = yaw;
			auto candidate = m_Draft;
			auto *edited = Find_Pattern(candidate, patternId);
			if (edited)
			{
				edited->BossMotion = motion;
				Mark_Draft(candidate, *edited);
				std::string status;
				(void)Commit_Candidate(std::move(candidate), "Updated boss movement interval and positions.", status);
			}
			return true;
		}
	}
	if (ImGui::Button(m_bSequenceWorkspace ? "Delete Sequence###Delete Pattern" : "Delete Pattern"))
	{
		Request_PatternDelete(patternId);
		return true;
	}

	return false;
}

// Stage 생성·속성·순서·Animation 연결을 표시한다. 변경/선택 부재는 nullptr로 상세 frame 종료를 알린다.
// 그대로 유지된 Stage만 반환해 다음 Animation 상세가 유효한 차용 포인터를 읽도록 한다.
const Client::KOUKU_SAYDON_COMPOSITION_STAGE *Client::CKoukuSaydonActionWorkbench::Render_StageProperties(
	const KOUKU_SAYDON_COMPOSITION_PATTERN *pattern, const std::string &patternId)
{
	ImGui::SeparatorText("Stage");
	ImGui::SetNextItemWidth(110.f);
	if (ImGui::BeginCombo("##NewStageKind", STAGE_KINDS[m_iSelectedNewStageKind]))
	{
		for (int32_t index = 0; index < static_cast<int32_t>(STAGE_KINDS.size()); ++index)
		{
			if (ImGui::Selectable(STAGE_KINDS[index], m_iSelectedNewStageKind == index))
			{
				m_iSelectedNewStageKind = index;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.f);
	ImGui::InputInt("ms##NewStageDuration", &m_iNewStageDurationMs, 100, 1000);
	m_iNewStageDurationMs = std::clamp(m_iNewStageDurationMs, 1, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	ImGui::SameLine();
	if (ImGui::Button("Add Stage"))
	{
		std::string stageId;
		std::string status;
		(void)Add_Stage(patternId, STAGE_KINDS[m_iSelectedNewStageKind],
			static_cast<std::uint32_t>(m_iNewStageDurationMs), stageId, status);
		return nullptr;
	}

	const KOUKU_SAYDON_COMPOSITION_STAGE *stage = Find_Stage(*pattern, m_strSelectedStageId);
	if (nullptr == stage)
		return nullptr;
	const std::string stageId = stage->strStageId;

	ImGui::Text("%s | %s", stage->strStageId.c_str(), stage->strActionId.c_str());
	std::string requestedStageKind;
	if (ImGui::BeginCombo("Stage Kind", stage->strStageKind.c_str()))
	{
		for (const char_t *const value : STAGE_KINDS)
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
		return nullptr;
	}
	bool retargetOnEnter = stage->bRetargetOnEnter;
	if (ImGui::Checkbox("Retarget player on enter", &retargetOnEnter))
	{
		std::string status;
		(void)Set_StageRetargetOnEnter(patternId, stageId, retargetOnEnter, status);
		return nullptr;
	}
	ImGui::TextDisabled(stage->RetargetTarget == "NEAREST_ALIVE"
			? "Choose the nearest living player at Stage entry, then keep that facing."
			: "Choose a living player at Stage entry, then keep that facing.");
	int32_t durationMs = static_cast<int32_t>(stage->iDurationMs);
	if (ImGui::InputInt("Duration ms", &durationMs, 10, 100))
	{
		durationMs = std::clamp(durationMs, 1, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
		std::string status;
		(void)Set_StageDuration(patternId, stageId, static_cast<std::uint32_t>(durationMs), status);
		return nullptr;
	}
	ImGui::TextDisabled("Effect and other lane times stay unchanged. Pattern lifetime includes their tails.");
	// Stage-local offset + play 길이의 최댓값이 Animation을 담을 최소 Stage 길이(ms)다.
	std::uint64_t animationEndMs = 0u;
	for (const auto &animation : stage->AnimationOccurrences)
		animationEndMs = (std::max)(animationEndMs, std::uint64_t(animation.iStartOffsetMs) + animation.iPlayMs);
	ImGui::BeginDisabled(
		animationEndMs == 0u || animationEndMs > MAX_EDITOR_TIME_MS || animationEndMs == stage->iDurationMs);
	const bool fitStage = ImGui::Button("Fit Stage to Animation");
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(
			"Fit this Stage to its animation. Effect and other lane start times and durations stay unchanged.");
	if (fitStage)
	{
		std::string status;
		(void)Set_StageDuration(patternId, stageId, static_cast<std::uint32_t>(animationEndMs), status);
		return nullptr;
	}
	if (ImGui::Button("Stage Up"))
	{
		std::string status;
		(void)Move_Stage(patternId, stageId, -1, status);
		return nullptr;
	}
	ImGui::SameLine();
	if (ImGui::Button("Stage Down"))
	{
		std::string status;
		(void)Move_Stage(patternId, stageId, 1, status);
		return nullptr;
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete Stage"))
	{
		std::string status;
		(void)Delete_Stage(patternId, stageId, status);
		return nullptr;
	}
	ImGui::BeginDisabled(!m_bHasSelectedResource);
	bool_t animationBound = false;
	if (ImGui::Button("Bind Selected Animation"))
	{
		std::string occurrenceId;
		std::string status;
		(void)Bind_Animation(patternId, stageId, m_SelectedResource, 0u, occurrenceId, status);
		animationBound = true;
	}
	ImGui::EndDisabled();
	if (animationBound)
		return nullptr;

	return stage;
}

// Stage 상세가 변경 없이 끝난 경우만 Animation occurrence를 조회한다. 입력 buffer를 검증/commit 함수에 넘긴 뒤 즉시 반환한다.
// source in/out, timeline play 길이, rate, blend가 서로 다른 시간 계약임을 유지하고 기존 ImGui scope 순서를 보존한다.
void Client::CKoukuSaydonActionWorkbench::Render_AnimationProperties(const KOUKU_SAYDON_COMPOSITION_PATTERN *pattern,
	const KOUKU_SAYDON_COMPOSITION_STAGE *stage, const std::string &patternId)
{
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE *const occurrence =
		Find_Occurrence(*pattern, m_strSelectedOccurrenceId);
	if (nullptr == occurrence)
		return;
	const std::string occurrenceId = occurrence->strOccurrenceId;
	ImGui::SeparatorText("Animation");
	ImGui::Text("%s", occurrence->strOccurrenceId.c_str());
	ImGui::Text("%s | %s", occurrence->strProfileId.c_str(), occurrence->strRuntimeClip.c_str());
	ImGui::InputInt("Start Offset ms", &m_iOccurrenceStartOffsetMs, 10, 100);
	m_iOccurrenceStartOffsetMs = std::clamp(m_iOccurrenceStartOffsetMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	if (ImGui::Button("Apply Move"))
	{
		std::string status;
		(void)Move_Animation(patternId, occurrenceId, static_cast<std::uint32_t>(m_iOccurrenceStartOffsetMs), status);
		return;
	}
	ImGui::InputInt("Source In ms", &m_iOccurrenceSourceStartMs, 10, 100);
	ImGui::InputInt("Source Out ms (0 = native end)", &m_iOccurrenceSourceEndMs, 10, 100);
	m_iOccurrenceSourceEndMs = std::clamp(m_iOccurrenceSourceEndMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	ImGui::InputInt("Play ms", &m_iOccurrencePlayMs, 10, 100);
	m_iOccurrenceSourceStartMs = std::clamp(m_iOccurrenceSourceStartMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_iOccurrencePlayMs = std::clamp(m_iOccurrencePlayMs, 1, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	if (ImGui::Button("Apply Trim"))
	{
		std::string status;
		(void)Trim_Animation(patternId, occurrenceId, static_cast<std::uint32_t>(m_iOccurrenceSourceStartMs),
			static_cast<std::uint32_t>(m_iOccurrencePlayMs), status,
			static_cast<std::uint32_t>(m_iOccurrenceSourceEndMs));
		return;
	}
	std::uint32_t nativeMs = 0u;
	const bool_t nativeKnown = Resolve_NativeClipMs(*occurrence, nativeMs);
	// timeline의 play 길이에 무차원 배속을 곱해 소비할 clip source 끝(ms)을 계산한다.
	const double windowEndMs =
		occurrence->iSourceStartMs + static_cast<double>(occurrence->iPlayMs) * occurrence->fPlayRate;
	if (nativeKnown)
	{
		ImGui::TextDisabled("Native clip %u ms | source window %u..%.0f ms", nativeMs, occurrence->iSourceStartMs,
			occurrence->iSourceEndMs ? double(occurrence->iSourceEndMs) : double(nativeMs));
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
	m_iOccurrenceEndPolicy = std::clamp(m_iOccurrenceEndPolicy, 0, static_cast<int32_t>(END_POLICIES.size()) - 1);
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
		(void)Set_AnimationPlayback(
			patternId, occurrenceId, m_fOccurrencePlayRate, END_POLICIES[m_iOccurrenceEndPolicy], status);
		return;
	}
	if (ImGui::Button("Repeat Selected Range to Stage End"))
	{
		std::string status;
		(void)Loop_AnimationToStageEnd(patternId, occurrenceId, status);
		return;
	}
	ImGui::TextWrapped(
		"Source In/Out select the clip segment. Play ms sets its timeline window. LOOP_TO_WINDOW repeats only that segment; extend the stage to repeat longer. Product playback rate: 0.1..4. Before enabling loop, the left edge trims source and timeline together; Source In/Out changes only the source range. Loop edges resize only its timeline window.");
	ImGui::InputInt("Blend In ms", &m_iOccurrenceBlendInMs, 10, 100);
	m_iOccurrenceBlendInMs = std::clamp(m_iOccurrenceBlendInMs, 0, 1000);
	if (ImGui::Button("Apply Blend In"))
	{
		std::string status;
		(void)Set_AnimationBlend(patternId, occurrenceId, static_cast<std::uint32_t>(m_iOccurrenceBlendInMs), status);
		return;
	}
	ImGui::TextWrapped("%s",
		m_bSequenceWorkspace
			? "Blends the previous animation into this box. Compare 0 ms and 100 ms in Preview, then Save."
			: "Blends the previous animation into this box. Compare 0 ms and 100 ms, then Save and Publish for Server playback.");
	std::string requestedTargetStageId;
	if (ImGui::BeginCombo("Move to Stage", stage->strStageId.c_str()))
	{
		for (const KOUKU_SAYDON_COMPOSITION_STAGE &candidateStage : pattern->Stages)
		{
			if (ImGui::Selectable(candidateStage.strStageId.c_str(), candidateStage.strStageId == stage->strStageId) &&
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
		(void)Move_AnimationToStage(patternId, occurrenceId, requestedTargetStageId,
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

// 명시 폐기 확인 뒤에만 dirty/geometry를 잠시 비워 Reload한다. 읽기 실패면 미저장 표시와 overlay를 되돌려 재시도할 수 있게 한다.
void Client::CKoukuSaydonActionWorkbench::Render_ReloadConfirmation()
{
	const char *const popup = m_bSequenceWorkspace ? "Discard Sequence draft?###SequenceReloadConfirmation"
												   : "Discard KoukuSaydon composition draft?";
	if (m_bReloadConfirmationRequested)
	{
		ImGui::OpenPopup(popup);
		m_bReloadConfirmationRequested = false;
	}
	if (!ImGui::BeginPopupModal(popup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
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

// SequencerTool 또는 MapTool이 pane보다 먼저 한 번 호출한다. 첫 로드와 서버 선택 추적을 처리하고 frame의 focus 수집을 초기화한다.
void Client::CKoukuSaydonActionWorkbench::Begin_WorkbenchFrame()
{
	m_HistoryWindowFocused = false;
	m_CompositionResourcesFocused = false;
	m_bSharedWorkspaceActive = true;
	m_bTimelineClipboardFocusThisFrame = false;
	if (!m_bLoadAttempted)
	{
		m_bLoadAttempted = true;
		std::string status;
		(void)Reload(status);
	}
	Synchronize_ServerPatternPlayback();
}

// shell이 창 배치·visibility를 소유하고 이 함수는 pane 내용만 그린다. focus는 clipboard 명령의 대상 범위를 정한다.
void Client::CKoukuSaydonActionWorkbench::Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE pane)
{
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
		m_CompositionResourcesFocused = pane == COMPOSITION_WORKBENCH_PANE::RESOURCES;
	switch (pane)
	{
	case COMPOSITION_WORKBENCH_PANE::TOOLBAR:
		Render_Toolbar();
		break;
	case COMPOSITION_WORKBENCH_PANE::PATTERNS:
	case COMPOSITION_WORKBENCH_PANE::BOSS_PATTERN:
		Render_PatternsAndResources();
		break;
	case COMPOSITION_WORKBENCH_PANE::RESOURCES:
		Render_ResourceTree();
		break;
	case COMPOSITION_WORKBENCH_PANE::SEQUENCER:
		Render_Timeline();
		break;
	case COMPOSITION_WORKBENCH_PANE::DETAILS:
		Render_Details();
		break;
	case COMPOSITION_WORKBENCH_PANE::PREVIEW:
		Render_Transport();
		break;
	default:
		break;
	}
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
		m_bTimelineClipboardFocusThisFrame = pane == COMPOSITION_WORKBENCH_PANE::PATTERNS ||
			pane == COMPOSITION_WORKBENCH_PANE::BOSS_PATTERN || pane == COMPOSITION_WORKBENCH_PANE::SEQUENCER;
}

// 자원 열기·timeline 복원 등 shell UI 요청을 값으로 한 번 넘기고 비운다. 문서 내용에는 영향이 없다.
Client::COMPOSITION_WORKBENCH_VIEW_REQUEST Client::CKoukuSaydonActionWorkbench::Consume_WorkbenchViewRequest()
{
	return std::exchange(m_WorkbenchViewRequest, {});
}

// 모든 pane가 행 참조를 사용한 뒤 popup·camera를 마무리하고 지연된 Undo/Redo를 적용한다.
void Client::CKoukuSaydonActionWorkbench::End_WorkbenchFrame()
{
	Render_ReloadConfirmation();
	Render_PatternDeleteConfirmation();
	Render_CameraWindow();
	// 모든 pane의 Pattern/occurrence 차용이 끝난 뒤 history가 draft를 교체할 수 있다.
	m_bSharedWorkspaceActive = false;
	Process_EditHistoryRequest();
}

// 별도 host 없이 사용할 때의 독립 창 경로다. 공용 Action Workbench는 Begin/Pane/End를 호출하므로 중복 렌더하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Render()
{
	m_HistoryWindowFocused = false;
	m_bTimelineClipboardFocusThisFrame = false;
	Poll_PublishProcess();
	Poll_DraftPlayProcess();
	if (!m_bOpen)
		return;
	if (!m_bLoadAttempted)
	{
		m_bLoadAttempted = true;
		std::string status;
		(void)Reload(status);
	}
	Synchronize_ServerPatternPlayback();
	ImGui::SetNextWindowSize(ImVec2(1180.f, 720.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(m_bSequenceWorkspace ? "Sequencer Benchmark###KoukuSequenceWorkbench"
										   : "KoukuSaydon Composition###KoukuSaydonActionWorkbench",
			&m_bOpen))
	{
		ImGui::End();
		Render_ResourcesWindow();
		Render_CameraWindow();
		Process_EditHistoryRequest();
		return;
	}
	Render_Toolbar();

	if (m_bTimelineMaximized)
	{
		Render_Timeline();
	}
	else if (ImGui::BeginTable("##KoukuWorkbenchColumns", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn(m_bSequenceWorkspace ? "Sequences / Resources" : "Patterns / Resources",
			ImGuiTableColumnFlags_WidthFixed, 300.f);
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
	Render_PatternDeleteConfirmation();
	m_bTimelineClipboardFocusThisFrame = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	ImGui::End();
	Render_ResourcesWindow();
	Render_CameraWindow();
	Process_TimelineClipboardShortcuts();
	Process_EditHistoryRequest();
}
