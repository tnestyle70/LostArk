// 로컬 preview 요청과 Server audition 관측을 연결한다. UI는 요청 스냅샷을 넘기고 승인된 실행 시계를 읽으며 전투 상태를 직접 갱신하지 않는다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"

// 이 도메인에서 직접 사용하는 런타임/저작 자료의 선언이다.
#include "KoukuSaydonPatternAuditionService.h"
#include "ActionPresentationTimeline.h"
#include "Level_KoukuSaydonArena.h"
#include "NetworkManager.h"

// 후보 값 복사, stable ID 조회, 시간/좌표 계산에 필요한 표준 타입이다.
#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>
#include <string>
#include <string_view>
#include <vector>

using namespace Client::KoukuWorkbenchDetail;

namespace
{
using namespace Client;

bool Is_EmptyParentTimeline(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	return Find_TimelineParent(document, pattern.strPatternId) && pattern.Stages.empty() &&
		pattern.PatternOccurrences.empty() && pattern.LogicOccurrences.empty() && pattern.SummonOccurrences.empty() &&
		pattern.WorldOccurrences.empty() && pattern.SceneProfileOccurrences.empty() &&
		pattern.PresentationOccurrences.empty() && !pattern.BossMotion && !pattern.bResetBossToSpawn;
}
} // 이 파일의 보조 구현 범위 끝

// MainApp이 단일 clip 요청을 한 번 가져간다. 이 요청이 연속 재생과 경쟁하지 않게 완주 목록을 취소하고 pending 값을 비운다.
bool_t Client::CKoukuSaydonActionWorkbench::Consume_AnimationPreviewRequest(
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &outRequest)
{
	if (!m_bPreviewRequestPending)
		return false;
	Cancel_CompleteSequencePlay();
	outRequest = std::move(m_PendingPreviewRequest);
	m_PendingPreviewRequest = {};
	m_bPreviewRequestPending = false;
	return true;
}

// MainApp이 요청 snapshot을 한 번 가져가는 경계다. 소비 후 pending을 비워 중복 재생을 막고 실제 실행 결과는 별도 통보를 기다린다.
bool_t Client::CKoukuSaydonActionWorkbench::Consume_PatternPreviewRequest(KOUKU_SAYDON_COMPOSITION_PATTERN &outPattern,
	std::uint32_t &outStartClockMs, bool_t &outStartPaused, std::string &outTargetAssetName)
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

// 일반 Composition의 게시 Pattern 요청만 외부 소비자에게 넘긴다. Sequence workspace는 별도 진입 절차를 사용하므로 이 경로에서 소비하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Consume_ServerPlayRequest(
	std::string &outPatternId, std::uint32_t &outSourceRevision)
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

// MainApp에 Bundle ID와 시작 ms/정지 여부를 전달하고 pending을 비운다. 실제 재생 결과가 올 때까지 결과 상태 대기를 표시한다.
bool_t Client::CKoukuSaydonActionWorkbench::Consume_BundlePreviewRequest(
	std::string &id, std::uint32_t &clockMs, bool_t &paused)
{
	if (!m_bBundlePreviewRequestPending)
		return false;
	Cancel_CompleteSequencePlay();
	id = std::move(m_strPendingBundlePreviewId);
	clockMs = m_iPendingPreviewStartMs;
	paused = m_bPendingPreviewStartPaused;
	m_bBundlePreviewRequestPending = false;
	m_bPreviewResultStatusPending = true;
	return true;
}

// 클릭 시점의 source revision, draft/world generation과 편집 문서 snapshot을 고정한다. 검증 실패는 요청 준비를 시작하지 않으며 성공 snapshot만 비동기 compiler로 넘긴다.
bool_t Client::CKoukuSaydonActionWorkbench::Request_SelectedServerPlay(std::string &outStatus)
{
	const auto reject = [&](std::string reason) {
		outStatus = m_strStatus = std::move(reason);
		return false;
	};
	if (m_bSequenceWorkspace)
		return reject(
			"Sequence playback uses its own combat-entry flow. Select a gameplay Pattern in Composition for Server Play.");
	if (!m_bHasDraft)
		return reject("Load a Composition before Play Pattern.");
	if (CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().Is_InFlight())
		return reject("Stop the active Server playback before starting another Pattern.");
	if (m_bServerPlayPreparationPending || m_bServerPlayRequestPending || !m_strPendingBundleServerId.empty())
		return reject("A Server Play request is already waiting to be submitted.");

	std::string patternId = m_strSelectedPatternId;
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::FOLDER)
	{
		const auto *folder = Find_Folder(m_Draft, m_strSelectedFolderId);
		if (!folder || !folder->strLoadError.empty() || folder->strTimelinePatternId.empty())
			return reject("Select a Parent with a valid timeline before Play Pattern.");
		patternId = folder->strTimelinePatternId;
	}
	const bool bundleSelected = m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE;
	if (bundleSelected)
	{
		const auto *bundle = Find_Bundle(m_Draft, m_strSelectedBundleId);
		if (!bundle || !bundle->strLoadError.empty() || bundle->Members.empty())
			return reject("Select a valid Bundle with connected Patterns before Play Pattern.");
	}
	else
	{
		const auto *pattern = Find_Pattern(m_Draft, patternId);
		if ((m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN &&
				m_ePatternSelection != KOUKU_PATTERN_SELECTION::FOLDER) ||
			!pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0u)
			return reject("Select a valid Pattern or Parent timeline before Play Pattern.");
		if (Is_EmptyParentTimeline(m_Draft, *pattern))
			return reject(
				"This Parent timeline is empty. Patterns listed under a Parent do not play until placed with Append Pattern. "
				"Place child Pattern boxes or add common timeline rows before playback.");
	}

	KOUKU_DRAFT_PLAY_REQUEST request;
	request.bBundle = bundleSelected;
	request.strTargetId = bundleSelected ? m_strSelectedBundleId : patternId;
	request.iSourceRevision = m_Draft.iRevision;
	request.iDraftGeneration = m_iDraftGeneration;
	request.iWorldGeneration = CNetworkManager::Get().Get_WorldInboundGeneration();
	if (bundleSelected)
	{
		const auto *bundle = Find_Bundle(m_Draft, request.strTargetId);
		request.strGateId = bundle->strGateId;
		request.BundleIds.push_back(request.strTargetId);
		for (const auto &member : bundle->Members)
		{
			const auto *pattern = Find_Pattern(m_Draft, member.strPatternId);
			if (!pattern)
				return reject("Bundle has a missing Pattern: " + member.strPatternId);
			request.TargetPlacementIds.push_back(pattern->strTargetBossPlacementId);
		}
	}
	else
	{
		const auto *pattern = Find_Pattern(m_Draft, patternId);
		request.strGateId = pattern->strGateId;
		request.PatternIds.push_back(patternId);
		request.TargetPlacementIds.push_back(pattern->strTargetBossPlacementId);
	}
	auto snapshot = m_Draft;
	std::vector<std::string> allPatterns;
	for (const auto &pattern : snapshot.Patterns)
		allPatterns.push_back(pattern.strPatternId);
	if (!Apply_PresentationPreviewPlacements(snapshot, m_StagedPresentationGeometry, allPatterns, outStatus) ||
		!CKoukuSaydonCompositionDocument::Validate(snapshot, m_Document.Get_References(), outStatus))
		return reject("Draft playback validation failed: " + outStatus);
	return Start_DraftPlayPreparation(std::move(request), snapshot, outStatus);
}

// Workbench 프레임 진입에서 승인된 audition의 epoch/revision을 확인하고 UI 선택과 ms cursor를 따라간다.
// 입력을 시작한 실행은 자동 선택을 중단한다. background 작업이나 gameplay tick을 구동하는 함수는 아니다.
void Client::CKoukuSaydonActionWorkbench::Synchronize_ServerPatternPlayback()
{
	if (m_bSequenceWorkspace || m_strServerFollowRootPatternId.empty() || m_bServerPlayRequestPending)
		return;
	if (m_bServerPlayPreparationPending || m_hDraftPlayProcess || m_bDraftPlayReady)
		return;
	const auto &audition = CKoukuSaydonPatternAuditionService::Get().Get_Snapshot();
	if (audition.eOperation == LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_OPERATION::STOP && audition.Is_InFlight())
	{
		m_strStatus = audition.strStatus;
		return;
	}
	if (audition.iRequestSequence == m_iServerFollowPreviousRequest ||
		audition.strRequestedPatternId != m_strServerFollowRootPatternId ||
		audition.iExpectedSourceRevision != m_Draft.iRevision)
	{
		m_strServerFollowRootPatternId.clear();
		m_strServerFollowLivePatternId.clear();
		return;
	}
	m_strStatus = audition.strStatus;
	if (!audition.Is_InFlight())
	{
		if (audition.eState == KOUKU_SAYDON_PATTERN_AUDITION_STATE::COMPLETED)
			if (const auto *pattern = m_strSelectedPatternId == m_strServerFollowLivePatternId
					? Find_Pattern(m_Draft, m_strServerFollowLivePatternId)
					: nullptr)
				m_iCursorMs = Pattern_DurationMs(*pattern);
		m_strServerFollowRootPatternId.clear();
		m_strServerFollowLivePatternId.clear();
		return;
	}
	// 선택과 playhead는 승인된 실행을 관측한다. gameplay를 구동하지 않는다.
	// 미적용 문자/숫자 buffer는 Is_Dirty()에 포함되지 않으므로 한 번 편집하면
	// 입력 포커스를 잃어도 이번 실행의 자동 선택 추종은 멈춘 상태를 유지한다.
	if (m_iDraftGeneration != m_iServerFollowDraftGeneration || ImGui::GetIO().WantTextInput ||
		ImGui::IsAnyItemActive())
		m_bServerSelectionFollowSuspended = true;
	if (m_bServerSelectionFollowSuspended)
		return;
	const auto *arena = CLevel_KoukuSaydonArena::Get_Active();
	if (!arena)
		return;
	const auto &run = arena->Get_KoukuBundleState();
	if (run.iRunEpoch != audition.iRoomAuditionEpoch || run.iPinnedSourceRevision != m_Draft.iRevision)
		return;
	for (const auto &member : run.Members)
	{
		if (member.iBossNetEntityId != audition.iBossNetEntityId || member.strPatternId.empty() ||
			member.eState != LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE)
			continue;
		if (m_strServerFollowLivePatternId != member.strPatternId)
		{
			std::string status;
			if (!Select_PatternById(member.strPatternId, status))
			{
				m_strStatus = status;
				return;
			}
			m_strServerFollowLivePatternId = member.strPatternId;
		}
		if (m_strSelectedPatternId != member.strPatternId)
			return;
		const auto *pattern = Find_Pattern(m_Draft, member.strPatternId);
		float ageSeconds = 0.f;
		if (pattern &&
			CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
				(std::max)(run.iServerTick, arena->Get_PresentationServerTick()), member.iStartTick, 30.f, ageSeconds))
		{
			m_strCursorPatternId = member.strPatternId;
			m_iCursorMs =
				static_cast<std::uint32_t>((std::min)(ageSeconds * 1000.f, float(Pattern_DurationMs(*pattern))));
		}
		return;
	}
}

void Client::CKoukuSaydonActionWorkbench::Render_ServerPlayButton(const char_t *label)
{
	if (m_bSequenceWorkspace)
	{
		ImGui::SameLine();
		Render_EditHistoryButtons(label);
		return;
	}
	const auto &audition = CKoukuSaydonPatternAuditionService::Get().Get_Snapshot();
	ImGui::SameLine();
	ImGui::BeginDisabled(m_bServerPlayPreparationPending || audition.Is_InFlight());
	if (ImGui::Button(label))
	{
		std::string status;
		(void)Request_SelectedServerPlay(status);
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		const char_t *targetName = "No executable selection";
		const char_t *targetId = "";
		if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE)
		{
			if (const auto *bundle = Find_Bundle(m_Draft, m_strSelectedBundleId))
			{
				targetName = bundle->strDisplayName.c_str();
				targetId = bundle->strBundleId.c_str();
			}
		}
		else
		{
			const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
			if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::FOLDER)
			{
				const auto *folder = Find_Folder(m_Draft, m_strSelectedFolderId);
				pattern = folder ? Find_Pattern(m_Draft, folder->strTimelinePatternId) : nullptr;
			}
			if (pattern)
			{
				targetName = pattern->strDisplayName.c_str();
				targetId = pattern->strPatternId.c_str();
			}
		}
		ImGui::SetTooltip(
			"Server target: %s\n%s\nValidates and runs the current applied draft from zero on the Server, including Collider, Logic and Sound timing. Save and Publish are separate.",
			targetName, targetId);
	}
	ImGui::SameLine();
	Render_EditHistoryButtons(label);
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_bServerPlayPreparationPending && !audition.Can_Stop());
	ImGui::PushID(label);
	if (ImGui::Button("Stop Pattern"))
	{
		if (m_bServerPlayPreparationPending)
			m_bServerPlayCancelRequested = true;
		else
			(void)CKoukuSaydonPatternAuditionService::Get().Stop(m_strStatus);
	}
	ImGui::PopID();
	ImGui::EndDisabled();
}

bool_t Client::CKoukuSaydonActionWorkbench::Consume_BundleServerPlayRequest(std::string &id, std::uint32_t &revision)
{
	if (m_bSequenceWorkspace || m_strPendingBundleServerId.empty())
		return false;
	id = std::move(m_strPendingBundleServerId);
	m_strPendingBundleServerId.clear();
	revision = m_iPendingBundleServerRevision;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Request_BundlePreview(const std::uint32_t clockMs, const bool_t paused)
{
	return Queue_BundleDocumentPreview(m_strSelectedBundleId, clockMs, paused);
}

// Bundle의 자식과 공통 표현을 preview 문서로 펼치는 경계다. 잘못된 참조/배치는 pending 요청으로 넘기기 전에 거절한다.
bool_t Client::CKoukuSaydonActionWorkbench::Queue_BundleDocumentPreview(
	const std::string_view bundleId, const std::uint32_t clockMs, const bool_t paused)
{
	if (!m_bSequenceWorkspace &&
		(m_bServerPlayPreparationPending || CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().Is_InFlight()))
	{
		m_strStatus = "Stop Pattern before starting or scrubbing a local preview.";
		return false;
	}
	const auto *bundle = Find_Bundle(m_Draft, bundleId);
	if (!bundle || !bundle->strLoadError.empty() || bundle->Members.empty())
	{
		m_strStatus = "Select a valid bundle with connected Patterns.";
		return false;
	}
	for (const auto &member : bundle->Members)
	{
		const auto *pattern = Find_Pattern(m_Draft, member.strPatternId);
		if (!pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0)
		{
			m_strStatus = "Bundle cannot play: missing, invalid or empty child " + member.strPatternId;
			return false;
		}
	}
	// 중첩 Pattern 박스도 편집한 anchor를 사용하도록 확장 전에 배치를 반영한다.
	auto previewDraft = m_Draft;
	std::vector<std::string> previewPatternIds;
	for (const auto &member : bundle->Members)
		previewPatternIds.push_back(member.strPatternId);
	if (!Apply_PresentationPreviewPlacements(
			previewDraft, m_StagedPresentationGeometry, previewPatternIds, m_strStatus))
		return false;
	KOUKU_SAYDON_COMPOSITION_DOCUMENT expanded = previewDraft;
	for (const auto &member : bundle->Members)
	{
		KOUKU_SAYDON_COMPOSITION_DOCUMENT memberDocument;
		if (!CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(
				previewDraft, member.strPatternId, memberDocument, m_strStatus))
			return false;
		const auto *memberPattern = Find_Pattern(memberDocument, member.strPatternId);
		auto *destination = Find_Pattern(expanded, member.strPatternId);
		if (!memberPattern || !destination)
		{
			m_strStatus = "Expanded Bundle member is missing.";
			return false;
		}
		*destination = *memberPattern;
		for (const auto &logic : memberDocument.Logics)
			if (std::none_of(m_Draft.Logics.begin(), m_Draft.Logics.end(), [&](const auto &authored) {
					return authored.strLogicId == logic.strLogicId;
				}))
			{
				const auto duplicate =
					std::find_if(expanded.Logics.begin(), expanded.Logics.end(), [&](const auto &existing) {
						return existing.strLogicId == logic.strLogicId;
					});
				if (duplicate != expanded.Logics.end())
				{
					m_strStatus = "Expanded Bundle Logic ID collides: " + logic.strLogicId;
					return false;
				}
				expanded.Logics.push_back(logic);
			}
	}
	m_ExpandedPatternPreview = std::move(expanded);
	m_PendingPresentationGeometryPreviews.clear();
	m_strPresentationGeometryPreviewPatternId.clear();
	m_strPresentationGeometryPreviewOccurrenceId.clear();
	m_bPatternPreviewRequestPending = false;
	m_bPreviewRequestPending = false;
	m_bPresentationPreviewRequestPending = false;
	m_bBundlePreviewRequestPending = true;
	m_strPendingBundlePreviewId = bundle->strBundleId;
	for (const auto &edit : m_StagedPresentationGeometry)
	{
		if (!Valid_PresentationGeometry(edit.Occurrence) ||
			!std::any_of(bundle->Members.begin(), bundle->Members.end(), [&](const auto &member) {
				return member.strPatternId == edit.strPatternId;
			}))
			continue;
		const auto *owner = Find_Pattern(m_Draft, edit.strPatternId);
		const auto *source = owner ? Find_PresentationBox(*owner, edit.Occurrence.strOccurrenceId) : nullptr;
		const auto *resource = source ? Find_PresentationResource(m_Draft, source->strResourceId) : nullptr;
		if (!source || source->strResourceId != edit.Occurrence.strResourceId || !resource)
			continue;
		KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST request{edit.strPatternId, *source};
		Copy_PresentationPlacement(
			edit.Occurrence, request.Occurrence, resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
		if (Valid_PresentationPlacement(*resource, request.Occurrence) &&
			Valid_GameplayColliderScale(*resource, request.Occurrence))
			m_PendingPresentationGeometryPreviews.push_back(std::move(request));
	}
	m_iPendingPreviewStartMs = paused ? (std::min)(clockMs, Bundle_DurationMs(m_Draft, *bundle))
									  : (clockMs < Bundle_DurationMs(m_Draft, *bundle) ? clockMs : 0);
	m_bPendingPreviewStartPaused = paused;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
	m_strLocalPreviewSnapshotId = bundle->strBundleId;
	// snapshot 세대는 재생에 넘긴 값이 어느 draft에서 왔는지 표시한다. 문서 변경 때 재생 중 객체를 직접 고치지 않고 이후 refresh 여부를 판단한다.
	m_iLocalPreviewSnapshotGeneration = m_iDraftGeneration;
	m_bLocalPreviewSnapshotIsBundle = true;
	m_strStatus = "Bundle preview requested for " + std::to_string(bundle->Members.size()) + " actors.";
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Render_BundleTransport()
{
	const auto *bundle = Find_Bundle(m_Draft, m_strSelectedBundleId);
	if (!bundle)
	{
		ImGui::TextDisabled("Select a playback bundle.");
		return;
	}
	const bool active = m_PreviewState.bPlaying && m_PreviewState.strPatternId == bundle->strBundleId;
	bool ready = bundle->strLoadError.empty() && !bundle->Members.empty();
	for (const auto &member : bundle->Members)
	{
		const auto *p = Find_Pattern(m_Draft, member.strPatternId);
		ready &= p && p->strLoadError.empty() && Pattern_DurationMs(*p) > 0;
	}
	ImGui::BeginDisabled(!ready);
	if (ImGui::Button("Play Bundle"))
	{
		(void)Request_BundlePreview(m_iCursorMs);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!active);
	if (ImGui::Button(m_PreviewState.bPaused ? "Resume" : "Pause"))
	{
		if (m_PreviewState.bPaused)
			(void)Request_PreviewResume();
		// 현재 선택과 preview 상태를 유지한 채 PAUSE transport를 요청한다. reset에 해당하는 Stop_Preview와는 별도 명령이다.
		else
			(void)Request_PreviewPause();
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop Bundle"))
		(void)Request_PreviewPause();
	ImGui::SameLine();
	if (ImGui::Button("Reset Bundle"))
		Request_PreviewReset();
	ImGui::EndDisabled();
	Render_ServerPlayButton("Play Pattern##BundleServer");
	ImGui::SameLine();
	ImGui::Text("%zu actors | %u / %u ms", bundle->Members.size(), active ? m_PreviewState.iClockMs : m_iCursorMs,
		Bundle_DurationMs(m_Draft, *bundle));
	if (!ready)
		ImGui::TextDisabled("Connect valid child Patterns with a positive lifetime before Play.");
	if (!m_strStatus.empty())
		ImGui::TextWrapped("%s", m_strStatus.c_str());
}

// 로컬 Play/Scrub의 진입점이다. 일반 Composition은 Server audition 중 요청을 거절한다. 선택 박스 override는 문서 사본에서 검증한 뒤 요청한다.
bool_t Client::CKoukuSaydonActionWorkbench::Request_PatternPreview(const std::string_view patternId,
	const std::uint32_t startClockMs, std::string &outStatus, const bool_t startPaused,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE *presentationOverride)
{
	if (!m_bSequenceWorkspace &&
		(m_bServerPlayPreparationPending || CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().Is_InFlight()))
	{
		outStatus = m_strStatus = "Stop Pattern before starting or scrubbing a local preview.";
		return false;
	}
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0u)
	{
		outStatus = m_strStatus = "Select an editable Pattern with a positive lifetime to play.";
		return false;
	}
	auto previewDraft = m_Draft;
	if (presentationOverride)
	{
		auto *owner = Find_Pattern(previewDraft, patternId);
		const auto box = std::find_if(
			owner->PresentationOccurrences.begin(), owner->PresentationOccurrences.end(), [&](const auto &row) {
				return row.strOccurrenceId == presentationOverride->strOccurrenceId &&
					row.strResourceId == presentationOverride->strResourceId;
			});
		if (box == owner->PresentationOccurrences.end())
		{
			outStatus = m_strStatus = "Preview override needs an existing box in this Pattern.";
			return false;
		}
		*box = *presentationOverride;
		if (!CKoukuSaydonCompositionDocument::Validate(previewDraft, m_ResourceReferences, outStatus))
		{
			m_strStatus = outStatus;
			return false;
		}
	}
	return Queue_PatternDocumentPreview(std::move(previewDraft), patternId, startClockMs, outStatus, startPaused);
}

// 기하 overlay를 먼저 합친 뒤 부모 Pattern을 확장한다. 확장 성공 후에만 기존 pending을 정리하고 독립 snapshot·cursor·draft generation을 기록한다.
bool_t Client::CKoukuSaydonActionWorkbench::Queue_PatternDocumentPreview(KOUKU_SAYDON_COMPOSITION_DOCUMENT previewDraft,
	const std::string_view patternId, const std::uint32_t startClockMs, std::string &outStatus,
	const bool_t startPaused)
{
	if (!Apply_PresentationPreviewPlacements(
			previewDraft, m_StagedPresentationGeometry, {std::string(patternId)}, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT expanded;
	if (!CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(previewDraft, patternId, expanded, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	const auto *expandedPattern = Find_Pattern(expanded, patternId);
	if (!expandedPattern)
	{
		outStatus = m_strStatus = "Expanded Parent Pattern is missing.";
		return false;
	}
	Cancel_CompleteSequencePlay();
	const auto durationMs = Pattern_DurationMs(*expandedPattern);
	m_PendingPresentationGeometryPreviews.clear();
	m_strPresentationGeometryPreviewPatternId.clear();
	m_strPresentationGeometryPreviewOccurrenceId.clear();
	m_bBundlePreviewRequestPending = false;
	m_bPresentationPreviewRequestPending = false;
	m_PendingPresentationPreviewRequest = {};
	m_PendingPatternPreview = *expandedPattern;
	m_ExpandedPatternPreview = std::move(expanded);
	m_strPendingPreviewTargetAsset.clear();
	m_bPatternPreviewRequestPending = true;
	m_bPreviewRequestPending = false;
	m_iPendingPreviewStartMs =
		startPaused ? (std::min)(startClockMs, durationMs) : (startClockMs < durationMs ? startClockMs : 0u);
	m_bPendingPreviewStartPaused = startPaused;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
	m_strCursorPatternId = std::string(patternId);
	m_iCursorMs = m_iPendingPreviewStartMs;
	m_strLocalPreviewSnapshotId = std::string(patternId);
	// snapshot 세대는 재생에 넘긴 값이 어느 draft에서 왔는지 표시한다. 문서 변경 때 재생 중 객체를 직접 고치지 않고 이후 refresh 여부를 판단한다.
	m_iLocalPreviewSnapshotGeneration = m_iDraftGeneration;
	m_bLocalPreviewSnapshotIsBundle = false;
	outStatus = m_strStatus = "Pattern preview requested from " + std::to_string(m_iCursorMs) + " ms.";
	return true;
}

// Sequence 전용 Complete Play 요청이다. 연결된 admission callback을 우선 사용하고, callback이 없을 때만 관문의 유일한 전투 진입 sequence를 확인해 예약한다.
bool_t Client::CKoukuSaydonActionWorkbench::Request_CompleteSequencePlay(
	std::string &outStatus, const std::string_view gateId)
{
	if (!m_bSequenceWorkspace || !m_bHasDraft)
	{
		outStatus = m_strStatus = "Complete sequence playback requires the Sequence workspace.";
		return false;
	}
	std::vector<std::string> patternIds;
	const std::string gate = gateId.empty() ? m_strSelectedGateId : std::string(gateId);
	if (!CKoukuSaydonCompositionDocument::Is_KnownGate(gate))
	{
		outStatus = m_strStatus = "Unknown Gate for Complete Play.";
		return false;
	}
	// 관문 사이 movie와 Bingo를 포함한 관문 intro는 Server plan이 소유한다.
	if (m_CompleteSequenceAdmission)
	{
		const bool submitted = m_CompleteSequenceAdmission(gate, outStatus);
		m_strStatus = outStatus;
		return submitted;
	}
	for (const auto &pattern : m_Draft.Patterns)
	{
		if (pattern.strGateId != gate || !pattern.bEnterCombatOnFinish)
			continue;
		if (!pattern.strLoadError.empty() || Pattern_DurationMs(pattern) == 0u)
		{
			outStatus = m_strStatus = "Complete Play cannot queue invalid sequence: " + pattern.strPatternId;
			return false;
		}
		patternIds.push_back(pattern.strPatternId);
	}
	if (patternIds.size() != 1u)
	{
		outStatus = m_strStatus = "Complete Play requires exactly one valid combat entry sequence in this Gate.";
		return false;
	}
	return Queue_CompleteSequenceItem(std::move(patternIds), 0u, outStatus);
}

// 선택과 preview 요청이 모두 성공해야 완주 목록/index를 보관한다. 실패하면 예약을 취소해 다음 sequence로 잘못 진행하지 않게 한다.
bool_t Client::CKoukuSaydonActionWorkbench::Queue_CompleteSequenceItem(
	std::vector<std::string> patternIds, const std::size_t index, std::string &outStatus)
{
	const std::string patternId = patternIds.at(index);
	// 일반 선택은 이전 요청을 비우므로 그 뒤에 검증된 이번 실행만 복원한다.
	if (!Select_PatternById(patternId, outStatus) || !Request_PatternPreview(patternId, 0u, outStatus))
	{
		Cancel_CompleteSequencePlay();
		outStatus = m_strStatus = "Complete Play stopped: " + outStatus;
		return false;
	}
	m_CompleteSequencePatternIds = std::move(patternIds);
	m_iCompleteSequenceIndex = index;
	m_bCompleteSequenceAdmitted = false;
	outStatus = m_strStatus = "Complete Play: sequence " + std::to_string(index + 1u) + " / " +
		std::to_string(m_CompleteSequencePatternIds.size()) + " from 0 ms.";
	return true;
}

// MainApp의 preview 준비 결과를 받는다. 완주 실행은 성공 시 진행을 허용하고 실패 시 예약을 취소한다. 완주 밖의 로컬 snapshot 재시작 실패는 Stop_StaleLocalPreview로 정리한다.
void Client::CKoukuSaydonActionWorkbench::Notify_SequencePreviewAdmission(
	const bool_t succeeded, const std::string &status)
{
	if (!Is_CompleteSequencePlaying())
	{
		if (!succeeded && !m_strLocalPreviewSnapshotId.empty())
			Stop_StaleLocalPreview("Preview could not restart and was stopped: " + status);
		return;
	}
	if (succeeded)
	{
		m_bCompleteSequenceAdmitted = true;
		return;
	}
	Cancel_CompleteSequencePlay();
	m_strStatus = "Complete Play stopped: " + status;
}

// 승인된 실행과 완료 Pattern ID가 일치할 때만 다음 항목으로 진행한다. 마지막 항목은 관문 완료를 반환하고 완주 상태를 비운다.
bool_t Client::CKoukuSaydonActionWorkbench::Advance_CompleteSequencePlay(
	const std::string_view completedPatternId, std::string *completedGate)
{
	if (completedGate)
		completedGate->clear();
	if (!m_bCompleteSequenceAdmitted || !Is_CompleteSequencePlaying() ||
		m_CompleteSequencePatternIds[m_iCompleteSequenceIndex] != completedPatternId)
		return false;
	const auto next = m_iCompleteSequenceIndex + 1u;
	if (next == m_CompleteSequencePatternIds.size())
	{
		if (completedGate)
			*completedGate = m_strSelectedGateId;
		Cancel_CompleteSequencePlay();
		m_strStatus = "Complete Play finished all sequences in the selected Gate.";
		return true;
	}
	std::string status;
	(void)Queue_CompleteSequenceItem(m_CompleteSequencePatternIds, next, status);
	return true;
}

// 완주 예약과 승인 상태를 비운다. 현재 화면의 모든 preview 해제는 별도 Stop_Preview가 맡는다.
void Client::CKoukuSaydonActionWorkbench::Cancel_CompleteSequencePlay()
{
	if (Is_CompleteSequencePlaying())
		m_bPatternPreviewRequestPending = false;
	m_CompleteSequencePatternIds.clear();
	m_iCompleteSequenceIndex = 0u;
	m_bCompleteSequenceAdmitted = false;
}

void Client::CKoukuSaydonActionWorkbench::Render_CompleteSequenceTransport()
{
	if (!m_bSequenceWorkspace)
		return;
	ImGui::BeginDisabled(!m_bHasDraft);
	if (ImGui::Button(m_strSelectedGateId == "BINGO" ? "Complete Play - Bingo Loop" : "Complete Play"))
	{
		std::string status;
		(void)Request_CompleteSequencePlay(status);
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(m_strSelectedGateId == "BINGO"
				? "Start the Server Bingo combat loop. Select a cutscene and use Play below to preview that cutscene."
				: "Play this Gate's Server raid entry, then start its Saved Pattern Flow.");
	if (!Is_CompleteSequencePlaying())
		return;
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_PreviewState.bPlaying && !m_bPatternPreviewRequestPending);
	if (ImGui::Button(m_PreviewState.bPaused ? "Resume##CompleteSequence" : "Pause##CompleteSequence"))
	{
		if (m_PreviewState.bPaused)
			(void)Request_PreviewResume();
		else
			(void)Request_PreviewPause();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Stop##CompleteSequence"))
		(void)Request_PreviewPause();
	ImGui::SameLine();
	if (ImGui::Button("Reset##CompleteSequence"))
		Request_PreviewReset();
	else
	{
		ImGui::SameLine();
		ImGui::Text("Sequence %zu / %zu", m_iCompleteSequenceIndex + 1u, m_CompleteSequencePatternIds.size());
	}
}

// 대기/재생 중인 로컬 snapshot의 draft generation을 확인한다. 완주 재생과 Server가 관측 중인 실행은 로컬 자동 재시작 대상에서 제외한다.
bool_t Client::CKoukuSaydonActionWorkbench::Has_StaleLocalPreviewSnapshot() const
{
	const bool pending = m_bPatternPreviewRequestPending || m_bBundlePreviewRequestPending;
	const bool playing = m_PreviewState.bPlaying && m_PreviewState.strPatternId == m_strLocalPreviewSnapshotId;
	return (pending || playing) && !Is_CompleteSequencePlaying() && !m_bServerPlayPreparationPending &&
		(m_strServerFollowRootPatternId.empty() ||
			!CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().Is_InFlight()) &&
		!m_strLocalPreviewSnapshotId.empty() && m_iLocalPreviewSnapshotGeneration != m_iDraftGeneration;
}

// 오래된 snapshot을 해제하되 사용자가 보고 있던 Pattern ID와 ms cursor를 복원하여 실패 위치를 유지한다.
void Client::CKoukuSaydonActionWorkbench::Stop_StaleLocalPreview(const std::string_view status)
{
	const bool pending = m_bPatternPreviewRequestPending || m_bBundlePreviewRequestPending;
	const auto cursor = pending ? m_iPendingPreviewStartMs : m_PreviewState.iClockMs;
	const auto patternId = pending ? m_strLocalPreviewSnapshotId : m_PreviewState.strPatternId;
	const std::string message(status);
	Stop_Preview();
	m_iCursorMs = cursor;
	m_strCursorPatternId = patternId;
	m_PreviewState.bPlaying = false;
	m_PreviewState.bPaused = true;
	m_PreviewState.strStatus = message;
	m_strStatus = message;
}

// 현재 cursor와 pause 상태에서 최신 Pattern/Bundle snapshot을 다시 요청한다. 재구성 실패 시 이전 실행은 정리하고 실패 위치/진단을 남긴다.
bool_t Client::CKoukuSaydonActionWorkbench::Refresh_LocalPreviewSnapshot(
	const std::uint32_t clockMs, const bool_t paused, std::string &outStatus)
{
	const auto id = m_strLocalPreviewSnapshotId;
	const bool bundle = m_bLocalPreviewSnapshotIsBundle;
	const bool refreshed = bundle ? Queue_BundleDocumentPreview(id, clockMs, paused)
								  : Request_PatternPreview(id, clockMs, outStatus, paused);
	if (!refreshed)
	{
		const auto error = m_strStatus;
		Stop_StaleLocalPreview("Changed preview could not restart and was stopped: " + error);
		outStatus = m_strStatus;
		return false;
	}
	outStatus = m_strStatus;
	return true;
}

// Server audition과 충돌하지 않는 로컬 실행만 재개한다. 편집 후 snapshot이 낡았으면 현재 ms에서 다시 준비한다.
bool_t Client::CKoukuSaydonActionWorkbench::Request_PreviewResume()
{
	if (!m_bSequenceWorkspace &&
		(m_bServerPlayPreparationPending || CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().Is_InFlight()))
	{
		m_strStatus = "Stop Pattern before resuming a local preview.";
		return false;
	}
	if (m_ePendingTransport == KOUKU_PREVIEW_TRANSPORT::STOP || !m_PreviewState.bPlaying)
		return false;
	if (Has_StaleLocalPreviewSnapshot())
		return Refresh_LocalPreviewSnapshot(m_PreviewState.iClockMs, false, m_strStatus);
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::RESUME;
	return true;
}

// 아직 소비하지 않은 요청은 시작부터 정지하도록 바꾼다. 실행 중 요청은 현재 ms를 유지한 PAUSE를 보내며 Reset과 구분한다.
bool_t Client::CKoukuSaydonActionWorkbench::Request_PreviewPause()
{
	if (m_bPatternPreviewRequestPending || m_bBundlePreviewRequestPending)
	{
		m_bPendingPreviewStartPaused = true;
		m_iCursorMs = m_iPendingPreviewStartMs;
		m_iPendingSeekMs = m_iCursorMs;
		return true;
	}
	if (!m_PreviewState.bPlaying && !m_bPreviewRequestPending)
		return false;
	m_iCursorMs = m_PreviewState.iClockMs;
	m_iPendingSeekMs = m_iCursorMs;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::PAUSE;
	m_strStatus = "Stopped at the current pose. Drag the ruler to scrub; Reset releases the preview.";
	return true;
}

// ruler 입력을 Pattern 수명 안의 ms로 제한한다. 같은 최신 snapshot이면 SEEK를 보내고, 없거나 낡았으면 해당 시점의 preview를 다시 준비한다.
bool_t Client::CKoukuSaydonActionWorkbench::Request_PatternScrub(
	const std::string_view patternId, const std::uint32_t clockMs, std::string &outStatus)
{
	if (!m_bSequenceWorkspace &&
		(m_bServerPlayPreparationPending || CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().Is_InFlight()))
	{
		outStatus = m_strStatus = "Stop Pattern before scrubbing a local preview.";
		return false;
	}
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	if (!pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0u)
	{
		m_strStatus = "Select a valid Pattern to scrub.";
		if (m_strLocalPreviewSnapshotId == patternId && Has_StaleLocalPreviewSnapshot())
			Stop_StaleLocalPreview("Changed Pattern is unavailable; its previous preview was stopped.");
		outStatus = m_strStatus;
		return false;
	}
	const auto cursor = (std::min)(clockMs, Pattern_DurationMs(*pattern));
	if (m_PreviewState.bPlaying && m_PreviewState.strPatternId == patternId)
	{
		if (Has_StaleLocalPreviewSnapshot())
			return Refresh_LocalPreviewSnapshot(cursor, m_PreviewState.bPaused, outStatus);
		m_iCursorMs = cursor;
		m_strCursorPatternId = std::string(patternId);
		m_iPendingSeekMs = cursor;
		m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::SEEK;
		outStatus = m_strStatus = "Preview cursor moved to " + std::to_string(cursor) + " ms.";
		return true;
	}
	return Request_PatternPreview(patternId, cursor, outStatus, true);
}

bool_t Client::CKoukuSaydonActionWorkbench::Request_BundleScrub(const std::uint32_t clockMs)
{
	if (!m_bSequenceWorkspace &&
		(m_bServerPlayPreparationPending || CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().Is_InFlight()))
	{
		m_strStatus = "Stop Pattern before scrubbing a local preview.";
		return false;
	}
	const auto *bundle = Find_Bundle(m_Draft, m_strSelectedBundleId);
	if (!bundle || !bundle->strLoadError.empty() || bundle->Members.empty())
	{
		if (m_strLocalPreviewSnapshotId == m_strSelectedBundleId && Has_StaleLocalPreviewSnapshot())
			Stop_StaleLocalPreview("Changed Bundle is unavailable; its previous preview was stopped.");
		return false;
	}
	const auto cursor = (std::min)(clockMs, Bundle_DurationMs(m_Draft, *bundle));
	if (m_PreviewState.bPlaying && m_PreviewState.strPatternId == bundle->strBundleId)
	{
		if (Has_StaleLocalPreviewSnapshot())
			return Refresh_LocalPreviewSnapshot(cursor, m_PreviewState.bPaused, m_strStatus);
		m_iCursorMs = cursor;
		m_iPendingSeekMs = cursor;
		m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::SEEK;
		return true;
	}
	if (!Request_BundlePreview(cursor, true))
		return false;
	m_iCursorMs = cursor;
	return true;
}

// 선택 occurrence의 배치를 먼저 undo 대상 overlay에 기록한 뒤 유효한 값만 preview 소비자에게 보낸다. 잘못된 값도 수정할 수 있게 overlay에는 남고 저작 원본은 유지한다. 시간/fade는 별도 Apply 대상이다.
bool_t Client::CKoukuSaydonActionWorkbench::Request_PresentationGeometryPreview(const std::string_view patternId,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &value, std::string &outStatus)
{
	const auto reject = [&](const char *reason) {
		outStatus = m_strStatus = reason;
		return false;
	};
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	if (!pattern || !pattern->strLoadError.empty() || !Pattern_DurationMs(*pattern))
		return reject("Collider geometry preview needs a valid Pattern.");
	const auto *source = Find_PresentationBox(*pattern, value.strOccurrenceId);
	const auto *resource = Find_PresentationResource(m_Draft, value.strResourceId);
	if (!source || source->strResourceId != value.strResourceId || !resource ||
		(resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER &&
			resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT &&
			resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::SUBTITLE))
		return reject("Presentation geometry preview needs an owned Collider, Effect or Subtitle box.");
	const bool effect = resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT;
	auto placement = *source;
	Copy_PresentationPlacement(value, placement, effect);
	Begin_EditHistory();
	{
		const auto staged = std::find_if(
			m_StagedPresentationGeometry.begin(), m_StagedPresentationGeometry.end(), [&](const auto &row) {
				return row.strPatternId == patternId && row.Occurrence.strOccurrenceId == value.strOccurrenceId;
			});
		if (Same_PresentationPlacement(placement, *source, effect))
		{
			if (staged != m_StagedPresentationGeometry.end())
				m_StagedPresentationGeometry.erase(staged);
		}
		else
		{
			KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST pending{std::string(patternId), placement};
			if (staged == m_StagedPresentationGeometry.end())
				m_StagedPresentationGeometry.push_back(std::move(pending));
			else
				*staged = std::move(pending);
		}
	}
	if (!Valid_PresentationGeometry(placement))
		return reject(
			"Geometry requires a finite position, rotation and positive scale. Save keeps the last applied source until corrected.");
	if (!Valid_PresentationPlacement(*resource, placement))
		return reject(
			"Effect placement requires BOSS, WORLD or fixed MAP without follow, bone or world dependencies. Previous source is preserved.");
	if (!Valid_GameplayColliderScale(*resource, placement))
		return reject(
			"Collider scale must be finite and positive. Adjust Scale / size; pending edits and previous source are preserved.");

	// Effect 배치, Collider 기하, Subtitle layout만 stage한다. 시간/fade는 여전히 Apply가 필요하다.
	KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST request{std::string(patternId), placement};
	const auto containsPattern = [&](const std::string_view ownerId) {
		if (ownerId == patternId)
			return true;
		const auto *bundle = Find_Bundle(m_Draft, ownerId);
		return bundle && std::any_of(bundle->Members.begin(), bundle->Members.end(), [&](const auto &member) {
			return member.strPatternId == patternId;
		});
	};
	const bool hasOwner = m_bPatternPreviewRequestPending ? m_PendingPatternPreview.strPatternId == patternId
		: m_bBundlePreviewRequestPending ? containsPattern(m_strPendingBundlePreviewId)
										 : m_PreviewState.bPlaying && containsPattern(m_PreviewState.strPatternId);
	if (!hasOwner)
	{
		const auto clockMs = resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::SUBTITLE
			? source->iStartMs
			: (m_strCursorPatternId == patternId ? m_iCursorMs : 0u);
		if (!Request_PatternPreview(patternId, clockMs, outStatus, true))
			return false;
	}
	if (m_strPresentationGeometryPreviewPatternId != patternId ||
		m_strPresentationGeometryPreviewOccurrenceId != value.strOccurrenceId)
		Cancel_PresentationGeometryPreview(false);
	m_strPresentationGeometryPreviewPatternId = std::string(patternId);
	m_strPresentationGeometryPreviewOccurrenceId = value.strOccurrenceId;
	if (m_bPatternPreviewRequestPending && m_PendingPatternPreview.strPatternId == patternId)
		for (auto &pending : m_PendingPatternPreview.PresentationOccurrences)
			if (pending.strOccurrenceId == value.strOccurrenceId)
			{
				Copy_PresentationPlacement(request.Occurrence, pending, effect);
				break;
			}
	const auto queued = std::find_if(m_PendingPresentationGeometryPreviews.begin(),
		m_PendingPresentationGeometryPreviews.end(), [&](const auto &row) {
			return row.strPatternId == patternId && row.Occurrence.strOccurrenceId == value.strOccurrenceId;
		});
	if (queued == m_PendingPresentationGeometryPreviews.end())
		m_PendingPresentationGeometryPreviews.push_back(std::move(request));
	else
		*queued = std::move(request);
	outStatus = m_strStatus =
		"Presentation placement updated at the preview cursor. Save keeps Effect placement, Collider geometry and Subtitle layout.";
	return true;
}

// 선택 Summon 박스의 임시 배치를 preview 요청으로 만든다. actor가 존재하는 Server 30 Hz tick을 기준으로 시작 위치를 맞춘다.
void Client::CKoukuSaydonActionWorkbench::Queue_SummonBoxPreview(
	const std::string_view patternId, const std::string_view occurrenceId, const bool rebuild)
{
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	const auto *box = pattern ? Find_SummonBox(*pattern, occurrenceId) : nullptr;
	if (!box || box->PatternSpawns.empty() || !box->iDurationMs)
		return;
	if (!m_strServerFollowRootPatternId.empty() &&
		CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().Is_InFlight())
	{
		m_strStatus = "Summon placement saved in the draft. Stop Server playback to view local placement edits.";
		return;
	}
	const bool playing = m_PreviewState.bPlaying && m_PreviewState.strPatternId == patternId;
	const auto clock = playing				? m_PreviewState.iClockMs
		: m_strCursorPatternId == patternId ? m_iCursorMs
											: box->iStartMs;
	// Summon 생성은 Server의 30 Hz tick에 맞춰진다. 선택 cursor에서 이 박스가
	// 보이면 유지하고, 아니면 actor가 실제로 생기는 첫 tick을 표시한다.
	const auto startTick = (std::uint64_t(box->iStartMs) * 30u + 999u) / 1000u;
	const auto firstVisibleMs = static_cast<std::uint32_t>((startTick * 1000u + 29u) / 30u);
	const auto endMs = box->iStartMs + box->iDurationMs;
	const auto previewMs = clock >= firstVisibleMs && clock < endMs ? clock : firstVisibleMs;
	if (rebuild || !playing || m_bPatternPreviewRequestPending || m_bBundlePreviewRequestPending)
	{
		m_PendingSummonPlacementPreview.reset();
		std::string status;
		(void)Request_PatternPreview(patternId, previewMs, status, true);
		return;
	}
	m_PendingSummonPlacementPreview = KOUKU_SUMMON_PLACEMENT_PREVIEW_REQUEST{std::string(patternId), *box};
	m_iCursorMs = previewMs;
	m_strCursorPatternId = std::string(patternId);
	m_iPendingSeekMs = previewMs;
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::SEEK;
	m_strStatus = "Summon position and yaw updated in World Preview. Save keeps these placements.";
}

bool_t Client::CKoukuSaydonActionWorkbench::Consume_SummonPlacementPreviewRequest(
	KOUKU_SUMMON_PLACEMENT_PREVIEW_REQUEST &outRequest)
{
	if (!m_PendingSummonPlacementPreview)
		return false;
	outRequest = std::move(*m_PendingSummonPlacementPreview);
	m_PendingSummonPlacementPreview.reset();
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Consume_PresentationGeometryPreviewRequest(
	KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST &outRequest)
{
	if (m_PendingPresentationGeometryPreviews.empty())
		return false;
	outRequest = std::move(m_PendingPresentationGeometryPreviews.front());
	m_PendingPresentationGeometryPreviews.erase(m_PendingPresentationGeometryPreviews.begin());
	return true;
}

// Effect/Collider의 단순 선택 해제는 overlay와 대기 요청을 유지하고 선택 ID만 비운다. 폐기 요청은 해당 overlay를 지우고 원본 배치를 복원하는 요청을 보내며, Subtitle도 원본 복원 경로를 사용한다.
void Client::CKoukuSaydonActionWorkbench::Cancel_PresentationGeometryPreview(const bool_t discardStagedGeometry)
{
	if (m_strPresentationGeometryPreviewPatternId.empty())
		return;
	const auto *pattern = Find_Pattern(m_Draft, m_strPresentationGeometryPreviewPatternId);
	const auto *source =
		pattern ? Find_PresentationBox(*pattern, m_strPresentationGeometryPreviewOccurrenceId) : nullptr;
	const auto *resource = source ? Find_PresentationResource(m_Draft, source->strResourceId) : nullptr;
	if (resource &&
		(resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
			resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER))
	{
		if (!discardStagedGeometry)
		{
			m_strPresentationGeometryPreviewPatternId.clear();
			m_strPresentationGeometryPreviewOccurrenceId.clear();
			return;
		}
		Begin_EditHistory();
		std::erase_if(m_StagedPresentationGeometry, [&](const auto &row) {
			return row.strPatternId == m_strPresentationGeometryPreviewPatternId &&
				row.Occurrence.strOccurrenceId == m_strPresentationGeometryPreviewOccurrenceId;
		});
	}
	if (source)
	{
		KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST request{m_strPresentationGeometryPreviewPatternId, *source};
		if (m_bPatternPreviewRequestPending && m_PendingPatternPreview.strPatternId == request.strPatternId)
			for (auto &pending : m_PendingPatternPreview.PresentationOccurrences)
				if (pending.strOccurrenceId == source->strOccurrenceId)
				{
					Copy_PresentationPlacement(
						*source, pending, resource && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
					break;
				}
		const auto queued = std::find_if(m_PendingPresentationGeometryPreviews.begin(),
			m_PendingPresentationGeometryPreviews.end(), [&](const auto &row) {
				return row.strPatternId == request.strPatternId &&
					row.Occurrence.strOccurrenceId == source->strOccurrenceId;
			});
		if (queued == m_PendingPresentationGeometryPreviews.end())
			m_PendingPresentationGeometryPreviews.push_back(std::move(request));
		else
			*queued = std::move(request);
	}
	m_strPresentationGeometryPreviewPatternId.clear();
	m_strPresentationGeometryPreviewOccurrenceId.clear();
}

bool_t Client::CKoukuSaydonActionWorkbench::Request_ColliderBoxPreview(const std::string_view patternId,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &value, std::string &outStatus)
{
	const auto reject = [&](const char *reason) {
		outStatus = m_strStatus = reason;
		return false;
	};
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	if (!pattern || !pattern->strLoadError.empty() || pattern->strActorProfileId.empty() ||
		CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(pattern->strActorProfileId).empty())
		return reject("Collider preview needs an available Pattern actor.");
	const auto box = std::find_if(
		pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(), [&](const auto &row) {
			return row.strOccurrenceId == value.strOccurrenceId;
		});
	const auto resource =
		std::find_if(m_Draft.PresentationResources.begin(), m_Draft.PresentationResources.end(), [&](const auto &row) {
			return row.strResourceId == value.strResourceId && row.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER;
		});
	if (box == pattern->PresentationOccurrences.end() || box->strResourceId != value.strResourceId ||
		resource == m_Draft.PresentationResources.end())
		return reject("Collider preview box or resource identity is unavailable.");
	if (value.strAnchorKind != "BOSS" || !value.bFollowBoss || value.strBone.empty() || value.strBone.size() > 128u ||
		value.strBone == "." || value.strBone == ".." ||
		(value.strBoneTarget != "BODY" && value.strBoneTarget != "WEAPON") || !value.strWorldId.empty() ||
		!value.strWorldOccurrenceId.empty() ||
		!std::all_of(value.strBone.begin(), value.strBone.end(), [](const unsigned char ch) {
			return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' ||
				ch == '-' || ch == '.';
		}))
		return reject("Collider preview needs a named BODY/WEAPON Bone following its Boss anchor.");
	const auto finite = [](const auto &values, const double minimum, const double maximum) {
		return std::all_of(values.begin(), values.end(), [=](const double v) {
			return std::isfinite(v) && v >= minimum && v <= maximum;
		});
	};
	if (!value.iDurationMs || std::uint64_t(value.iStartMs) + value.iDurationMs > Pattern_DurationMs(*pattern) ||
		!finite(value.PositionOffset, -100000.0, 100000.0) || !finite(value.RotationDegrees, -36000.0, 36000.0) ||
		!finite(value.Scale, 0.001, 10000.0))
		return reject("Collider preview needs a finite transform and a window inside its Pattern.");
	bool hasAnimation = false;
	for (const auto &stage : pattern->Stages)
		for (const auto &animation : stage.AnimationOccurrences)
		{
			if (CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(animation.strProfileId) !=
					pattern->strActorProfileId ||
				animation.strRuntimeClip.empty())
				return reject("Collider preview animation does not match its Pattern actor.");
			hasAnimation = true;
		}
	if (!hasAnimation)
		return reject("Collider Bone preview needs its Pattern animation.");

	// actor와 모든 animation 시계를 유지하며 이 임시 박스에만 Detail 입력을 반영한다.
	const auto index = static_cast<std::size_t>(box - pattern->PresentationOccurrences.begin());
	const bool_t keepPaused =
		m_PreviewState.bPlaying && m_PreviewState.bPaused && m_PreviewState.strPatternId == patternId;
	const auto previewClock = keepPaused ? m_PreviewState.iClockMs : value.iStartMs;
	if (!Request_PatternPreview(patternId, previewClock, outStatus, keepPaused))
		return false;
	m_PendingPatternPreview.PresentationOccurrences[index] = value;
	outStatus = m_strStatus = "Collider Bone preview requested with Pattern animation from " +
		std::to_string(m_iPendingPreviewStartMs) + " ms.";
	return true;
}

// Reset 버튼의 경계다. 완주 예약 취소와 Stop_Preview를 통해 외부 소비자가 실행을 해제하게 한다.
void Client::CKoukuSaydonActionWorkbench::Request_PreviewReset()
{
	Stop_Preview();
	m_bExplicitPreviewReset = true;
}

// Pattern/Bundle/Animation/Presentation의 대기 재생 플래그와 로컬 snapshot 식별자를 비우고 STOP transport를 예약한다. 장면 해제는 소비자가 수행하며 staged geometry와 별도 Summon 배치 요청은 여기서 지우지 않는다.
void Client::CKoukuSaydonActionWorkbench::Stop_Preview()
{
	m_bExplicitPreviewReset = false;
	Cancel_CompleteSequencePlay();
	m_PendingPresentationGeometryPreviews.clear();
	m_strPresentationGeometryPreviewPatternId.clear();
	m_strPresentationGeometryPreviewOccurrenceId.clear();
	m_bPatternPreviewRequestPending = false;
	m_bBundlePreviewRequestPending = false;
	m_bPreviewRequestPending = false;
	m_bPresentationPreviewRequestPending = false;
	m_PendingPresentationPreviewRequest = {};
	m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::STOP;
	m_strLocalPreviewSnapshotId.clear();
	m_iLocalPreviewSnapshotGeneration = UINT64_MAX;
	m_iCursorMs = 0u;
	m_strStatus = "Preview stop requested; the Pattern cursor is at zero.";
}

// 일반 Pattern의 재생/정지/초기화 UI다. Stop은 현재 pose를 유지하는 Pause이고 Reset은 preview 해제라는 기존 연결을 유지한다.
void Client::CKoukuSaydonActionWorkbench::Render_Transport()
{
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BINGO_PARENT)
	{
		ImGui::TextWrapped(
			"Use Play Bingo Parent to run the complete Server battle. Open a child Pattern for animation / resource preview.");
		return;
	}
	Render_CompleteSequenceTransport();
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE)
	{
		Render_BundleTransport();
		return;
	}
	if (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN)
	{
		ImGui::TextDisabled("Select a playback bundle or Pattern.");
		return;
	}
	const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool patternReady = nullptr != pattern && pattern->strLoadError.empty();
	if (nullptr == pattern)
		ImGui::TextDisabled("Select a Pattern to play all authored lanes.");
	else if (!pattern->strLoadError.empty())
		ImGui::TextWrapped("%s", pattern->strLoadError.c_str());
	const auto durationMs = patternReady ? Pattern_DurationMs(*pattern) : 0u;
	if (patternReady && m_strCursorPatternId != pattern->strPatternId)
	{
		m_strCursorPatternId = pattern->strPatternId;
		m_iCursorMs = 0u;
	}
	m_iCursorMs = (std::min)(m_iCursorMs, durationMs);
	const bool_t patternPreview =
		patternReady && m_PreviewState.bPlaying && m_PreviewState.strPatternId == pattern->strPatternId;
	ImGui::BeginDisabled(!patternReady);
	if (ImGui::Button(m_bSequenceWorkspace ? "Play Sequence###Play Pattern" : "Play Preview"))
	{
		std::string status;
		(void)Request_PatternPreview(pattern->strPatternId, m_iCursorMs, status);
	}
	if (!m_bSequenceWorkspace && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip(
			"Preview the current timeline from the cursor. Pause, Resume and ruler dragging control only this local preview. Use Play Pattern for Server Collider and Logic testing.");
	ImGui::SameLine();
	const auto *selectedOccurrence = patternReady ? Find_Occurrence(*pattern, m_strSelectedOccurrenceId) : nullptr;
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
		if (m_PreviewState.bPaused)
			(void)Request_PreviewResume();
		else
			(void)Request_PreviewPause();
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop"))
	{
		(void)Request_PreviewPause();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
		Request_PreviewReset();
	ImGui::EndDisabled();
	Render_ServerPlayButton("Play Pattern##TransportServer");
	ImGui::SameLine();
	if (m_PreviewState.bPlaying)
	{
		ImGui::Text("%s: %u / %u ms%s", patternPreview ? "Pattern" : "Other preview", m_PreviewState.iClockMs,
			m_PreviewState.iDurationMs, m_PreviewState.bPaused ? " (paused)" : "");
	}
	else
	{
		ImGui::TextDisabled(
			"cursor %u / %u ms | %zu stages", m_iCursorMs, durationMs, patternReady ? pattern->Stages.size() : 0u);
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(
			"Drag the ruler to sample the pose, including while stopped. Stop holds this time; Reset releases the preview. Drag box edges to trim, the body to move, the Stage edge to resize.");
}
