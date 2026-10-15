// 타임라인의 입력/표시와 파생 행 캐시를 소유한다. 문서 편집은 기존 Workbench 명령에 맡긴다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"
#include "KoukuSaydonActionWorkbench_CameraLookup.h"
#include "KoukuSaydonActionWorkbench_TimelineTypes.h"

// 승인된 audition 상태와 공용 타임라인 그리기·Profiler만 직접 사용한다.
#include "KoukuSaydonPatternAuditionService.h"
#include "CompositionTimeline.h"
#include "GameInstance.h"
#include "Profiler.h"

#include <Windows.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace Client::KoukuWorkbenchDetail;

namespace
{
using namespace Client;

constexpr ImU32 TIMELINE_LOGIC_COLOR = CompositionTimeline::LogicColor;
constexpr ImU32 TIMELINE_SUMMON_COLOR = CompositionTimeline::SummonColor;
constexpr ImU32 TIMELINE_WORLD_COLOR = CompositionTimeline::WorldColor;
constexpr ImU32 TIMELINE_SCENE_PROFILE_COLOR = CompositionTimeline::SceneProfileColor;
} // 이 파일의 보조 구현 범위 끝

void Client::CKoukuSaydonActionWorkbench::Rebuild_PatternChildRows(const KOUKU_SAYDON_COMPOSITION_PATTERN &parent)
{
	if (m_iChildRowsGeneration == m_iDraftGeneration && m_strChildRowsPattern == parent.strPatternId)
		return;
	m_iChildRowsGeneration = m_iDraftGeneration;
	m_strChildRowsPattern = parent.strPatternId;
	m_PatternChildRows.clear();
	m_strChildRowsStatus.clear();
	if (parent.PatternOccurrences.empty())
		return;
	KOUKU_SAYDON_COMPOSITION_DOCUMENT expanded;
	if (!CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(
			m_Draft, parent.strPatternId, expanded, m_strChildRowsStatus))
		return;
	const auto *result = Find_Pattern(expanded, parent.strPatternId);
	if (!result)
		return;
	std::unordered_map<std::string, std::pair<std::uint32_t, std::uint32_t>> times;
	std::uint32_t clock = 0u;
	for (const auto &stage : result->Stages)
	{
		for (const auto &row : stage.AnimationOccurrences)
			times.emplace(row.strOccurrenceId, std::make_pair(clock + row.iStartOffsetMs, row.iPlayMs));
		clock += stage.iDurationMs;
	}
	const auto timeLane = [&](const auto &rows) {
		for (const auto &row : rows)
			times.emplace(row.strOccurrenceId, std::make_pair(row.iStartMs, row.iDurationMs));
	};
	timeLane(result->LogicOccurrences);
	timeLane(result->SummonOccurrences);
	timeLane(result->WorldOccurrences);
	timeLane(result->SceneProfileOccurrences);
	timeLane(result->PresentationOccurrences);
	for (const auto &box : parent.PatternOccurrences)
	{
		const auto *child = Find_Pattern(m_Draft, box.strPatternId);
		if (!child)
			continue; // 확장 과정에서 없는 하위 Pattern을 이미 진단했으므로 표시 행에서는 건너뛴다.
		const auto duration = Pattern_DurationMs(*child);
		if (!duration)
			continue;
		for (std::uint32_t repeat = 0u; repeat < 64u && std::uint64_t(repeat) * duration < box.iDurationMs; ++repeat)
		{
			// 기존 확장기가 실제로 생성한 ID만 조회한다. 화면 행에는 원본 소유 ID를 보관하고,
			// 반복마다 파생된 runtime ID를 저작 문서의 편집 대상으로 사용하지 않는다.
			const auto scope = box.strOccurrenceId + ".r" + std::to_string(repeat);
			const auto add = [&](const std::string &runtimeId, const std::string &stageId, const std::string &sourceId,
								 const std::string &label, const std::uint32_t color) {
				const auto found = times.find(runtimeId);
				if (found == times.end())
					return;
				m_PatternChildRows.push_back({runtimeId, child->strPatternId, stageId, sourceId,
					child->strDisplayName + " / " + label, found->second.first, found->second.second, color});
			};
			std::size_t animationIndex = 0u;
			for (const auto &stage : child->Stages)
				for (const auto &row : stage.AnimationOccurrences)
					add(scope + ".animation." + std::to_string(++animationIndex), stage.strStageId, row.strOccurrenceId,
						"Animation / " + row.strRuntimeClip, IM_COL32(80, 145, 205, 255));
			const auto lane = [&](const auto &rows, const char *kind, const char *label, const std::uint32_t color) {
				for (std::size_t i = 0u; i < rows.size(); ++i)
					add(scope + "." + kind + "." + std::to_string(i + 1u), {}, rows[i].strOccurrenceId,
						std::string(label) + " / " + rows[i].strOccurrenceId, color);
			};
			lane(child->LogicOccurrences, "logic", "Logic", IM_COL32(185, 125, 65, 255));
			lane(child->SummonOccurrences, "summon", "Summon", IM_COL32(150, 125, 85, 255));
			lane(child->WorldOccurrences, "world", "World", IM_COL32(80, 160, 100, 255));
			lane(child->SceneProfileOccurrences, "sceneprofile", "Scene Profile", IM_COL32(100, 130, 170, 255));
			for (std::size_t i = 0u; i < child->PresentationOccurrences.size(); ++i)
			{
				const auto &row = child->PresentationOccurrences[i];
				const auto *resource = Find_DraftPresentationResource(row.strResourceId);
				const auto label = resource ? resource->strDisplayName : row.strResourceId;
				add(scope + ".presentation." + std::to_string(i + 1u), {}, row.strOccurrenceId,
					std::string(resource ? Presentation_Label(resource->eKind) : "Presentation") + " / " + label,
					IM_COL32(130, 100, 190, 255));
			}
			if (!box.bRepeat)
				break;
		}
	}
}

// Render_TimelineControls와 타임라인 키보드 입력이 같은 선택 조건을 소비한다.
// Stage/Animation만 순서 변경을 허용하고 다른 lane의 pattern-relative 시각은 유지한다.
// pattern은 현재 Render_Timeline 호출 동안만 차용하며 문서나 선택을 변경하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Can_ReorderTimelineSelection(
	const KOUKU_SAYDON_COMPOSITION_PATTERN *pattern) const
{
	return nullptr != pattern && pattern->strLoadError.empty() &&
		(!m_TimelineSelectedStageIds.empty() || !m_TimelineSelectedOccurrenceIds.empty()) &&
		std::all_of(
			m_TimelineSelectedOccurrenceIds.begin(), m_TimelineSelectedOccurrenceIds.end(), [&](const auto &id) {
				return nullptr != Find_Occurrence(*pattern, id);
			});
}

// Render_Timeline이 선택·cursor와 두 재생 상태를 결정한 뒤 호출한다.
// ImGui 호출과 ID, 명령 우선순위를 기존 순서대로 유지하며 값 입력은 세션 필드에 남긴다.
// Save/삭제/복제/수명·순서 변경을 요청했으면 성공 여부와 관계없이 true를 반환한다.
// 호출자는 즉시 이번 Render_Timeline을 끝내어 commit 이전 pattern 포인터를 다시 쓰지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Render_TimelineControls(const KOUKU_SAYDON_COMPOSITION_PATTERN *pattern,
	const std::string &patternId, const bool_t patternReady, const std::uint32_t durationMs,
	const bool_t patternPreview, const bool_t serverPlayback)
{
	const bool_t canPlayPattern =
		patternReady && durationMs > 0u && !serverPlayback && !m_bServerPlayPreparationPending;
	if (serverPlayback && m_bServerSelectionFollowSuspended)
		ImGui::TextDisabled("Server playback continues. Automatic selection is paused to preserve editor input.");
	const bool_t publishing = Is_PublishRunning();
	ImGui::BeginDisabled(publishing || !m_bHasDraft || !Is_Dirty());
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
	ImGui::BeginDisabled(!m_PreviewState.bPlaying && !m_bPatternPreviewRequestPending && !m_bPreviewRequestPending);
	if (ImGui::Button("Stop##KoukuSequencer"))
		(void)Request_PreviewPause();
	ImGui::SameLine();
	ImGui::BeginDisabled(serverPlayback || m_bServerPlayPreparationPending);
	if (ImGui::Button("Reset##KoukuSequencer"))
		Request_PreviewReset();
	ImGui::EndDisabled();
	ImGui::EndDisabled();
	Render_ServerPlayButton("Play Pattern##KoukuSequencerServer");
	ImGui::SameLine();
	ImGui::TextDisabled("%s | %u / %u ms%s", Is_Dirty() ? "Unsaved changes" : "Saved",
		patternPreview ? m_PreviewState.iClockMs : m_iCursorMs, durationMs,
		patternPreview && m_PreviewState.bPaused ? " (paused)" : "");
	if (publishing)
		ImGui::TextWrapped(
			"Publishing saved rev %u (%llu s). You can edit the timeline; Save becomes available when publishing finishes.",
			m_Draft.iRevision,
			static_cast<unsigned long long>((GetTickCount64() - m_iPublishStartedAtMilliseconds) / 1000u));
	if (!m_strStatus.empty())
		ImGui::TextWrapped("%s", m_strStatus.c_str());

	ImGui::SetNextItemWidth(155.f);
	if (ImGui::SliderFloat("Zoom##KoukuSequencer", &m_fPixelsPerSecond, 1.f, 500.f, "%.1f px/s"))
		m_bFitRequested = false;
	ImGui::SameLine();
	const bool_t durationEditable = patternReady;
	ImGui::BeginDisabled(!durationEditable);
	ImGui::SetNextItemWidth(145.f);
	ImGui::InputInt("Full lifetime ms##KoukuSequencer", &m_iPatternDurationMs, 100, 1000);
	bool_t durationRequested = durationEditable && (ImGui::IsItemActive() || ImGui::IsItemDeactivated()) &&
		(ImGui::IsKeyPressed(ImGuiKey_Enter, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false));
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(
			"Lifetime of all rows. Enter or Apply preserves Stage and Animation clocks. The boss advances when its last Stage ends; remaining rows keep playing.");
	ImGui::SameLine();
	durationRequested |= ImGui::Button("Apply##KoukuSequencerLifetime");
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Fit##KoukuSequencer"))
		m_bFitRequested = true;

	ImGui::TextUnformatted("Selected Box");
	ImGui::SameLine();
	const bool_t hasSelection = !m_TimelineSelectedStageIds.empty() || !m_TimelineSelectedOccurrenceIds.empty();
	ImGui::BeginDisabled(!patternReady || !hasSelection);
	bool_t deleteRequested = ImGui::Button("Delete##KoukuSequencerSelection");
	ImGui::SameLine();
	bool_t duplicateRequested = ImGui::Button("Duplicate##KoukuSequencerSelection");
	ImGui::SameLine();
	// 순서 변경은 Stage/Animation만 처리한다. 다른 lane의 시각은 Pattern 시작 기준 ms로 유지한다.
	ImGui::BeginDisabled(!Can_ReorderTimelineSelection(pattern));
	int32_t moveRequested = 0;
	if (ImGui::Button("< Earlier##KoukuSequencerSelection"))
		moveRequested = -1;
	ImGui::SameLine();
	if (ImGui::Button("Later >##KoukuSequencerSelection"))
		moveRequested = 1;
	ImGui::EndDisabled();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled(
		"%zu Stages + %zu lane boxes", m_TimelineSelectedStageIds.size(), m_TimelineSelectedOccurrenceIds.size());
	ImGui::TextDisabled(
		"Ctrl/Shift+click: toggle any lane | drag empty space: enclose boxes across lanes | Ctrl/Shift+drag: add | Ctrl+D: duplicate | Delete: remove");
	ImGui::TextDisabled(
		"Ctrl+C: copy the selection from every lane with its owned links | Ctrl+V: append it at the end of the selected Pattern (Patterns or Sequencer focused)");
	ImGui::TextDisabled(
		"Earlier/Later and Left/Right reorder Stage/Animation selections only. Mixed lane selections keep their authored clocks.");
	ImGui::TextWrapped("%s", Get_Status().c_str());
	if (saveRequested || deleteRequested || duplicateRequested || durationRequested || 0 != moveRequested)
	{
		std::string status;
		if (0 != moveRequested)
			(void)Move_SelectedStage(patternId, moveRequested, status);
		else if (deleteRequested)
			(void)Delete_TimelineSelection(
				patternId, m_TimelineSelectedStageIds, m_TimelineSelectedOccurrenceIds, status);
		else if (duplicateRequested)
			(void)Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION, status);
		else if (durationRequested)
			(void)Set_PatternDuration(
				patternId, static_cast<std::uint32_t>((std::max)(m_iPatternDurationMs, 0)), status);
		else
			(void)Save(status);
		// 명령이 draft 전체를 교체할 수 있으므로 차용한 포인터를 더 쓰지 않고 다음 프레임에 다시 조회한다.
		return true;
	}
	return false;
}

// Ensure_TimelineLayout이 캐시를 비운 뒤 WORLD occurrence의 animation 표시 구간을 채운다.
// world/slot별 행을 분리해 화면상 겹침이 다른 actor의 clip 소유권을 섞지 않게 한다.
// 원본 sequence와 pattern은 호출 중 차용하고 표시용 ID·시간·편집 요청 값만 캐시에 복사한다.
// 불완전하거나 길이가 없는 clip은 기존과 같이 표시 대상에서 제외하며 문서는 수정하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Collect_TimelineWorldClipRows(const KOUKU_SAYDON_COMPOSITION_PATTERN *pattern)
{
	auto &cache = *m_pTimelineLayout;
	auto &worldClipLanes = cache.worldClipLanes;
	auto &worldClipRows = cache.worldClipRows;
	for (const auto &box : pattern->WorldOccurrences)
	{
		const auto *world = Find_World(m_Draft, box.strWorldId);
		if (!world)
			continue;
		const auto source =
			std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(), [&](const auto &row) {
				return row.strInstanceId == world->strSequenceInstanceId;
			});
		if (source == m_WorldSequenceResources.end() || box.fPlaybackSpeed <= 0.f)
			continue;
		for (std::size_t index = 0u; index < source->AnimationTracks.size(); ++index)
		{
			const auto &clip = source->AnimationTracks[index];
			const auto start = static_cast<std::uint32_t>(std::llround(clip.fStartMs / box.fPlaybackSpeed));
			const auto end =
				(std::min)(box.iDurationMs, static_cast<std::uint32_t>(std::llround(clip.fEndMs / box.fPlaybackSpeed)));
			if (end <= start)
				continue;
			TIMELINE_LAYOUT_CACHE::WORLD_CLIP_ROW row;
			row.id = box.strOccurrenceId + ".clip-info." + std::to_string(index);
			row.occurrenceId = box.strOccurrenceId;
			row.label = clip.strDisplayName.empty() ? clip.strClipName : clip.strDisplayName;
			row.actorLabel = !source->strObjectDisplayName.empty()
				? source->strObjectDisplayName
				: (!world->strDisplayName.empty() ? world->strDisplayName : source->strDisplayName);
			row.edit = {source->strInstanceId, clip.strSlotId, clip.strClipName, clip.iLocalStartMs, clip.iLocalStartMs,
				clip.iSourceStartMs, clip.iSourceEndMs};
			row.localSpeed = clip.fInstanceSpeed * box.fPlaybackSpeed;
			row.nativeSpeed = clip.fPlaybackRate * box.fPlaybackSpeed;
			row.trackRate = clip.fTrackPlaybackRate;
			row.editable = !clip.bLoop && clip.iSourceEndMs > clip.iSourceStartMs && row.localSpeed > 0. &&
				row.nativeSpeed > 0. && bool(m_EditWorldAnimation);
			row.startMs = box.iStartMs + start;
			row.durationMs = end - start;
			auto actorLane = std::find_if(worldClipLanes.begin(), worldClipLanes.end(), [&](const auto &lane) {
				return lane.occurrenceId == box.strOccurrenceId && lane.slotId == clip.strSlotId;
			});
			if (actorLane == worldClipLanes.end())
			{
				worldClipLanes.push_back({box.strOccurrenceId, clip.strSlotId, row.actorLabel});
				actorLane = std::prev(worldClipLanes.end());
			}
			actorLane->intervals.push_back({row.id, double(row.startMs), double(row.durationMs)});
			worldClipRows.push_back(std::move(row));
		}
	}
}

// Ensure_TimelineLayout이 모은 구간을 기존 AllocateDisplayRows로 배치한다.
// 첫 행은 Stage strip이고 나머지 행 번호는 화면 좌표 계산만을 위한 파생 값이다.
// 캐시 interval을 소비해 layout과 nextRow를 갱신하며 저장된 lane·occurrence는 변경하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Allocate_TimelineDisplayRows(const std::uint64_t minimumBoxMs)
{
	constexpr std::size_t animationLane = 0u, patternLane = 11u;
	auto &cache = *m_pTimelineLayout;
	auto &lanes = cache.lanes;
	auto &worldClipLanes = cache.worldClipLanes;
	// 모든 box 종류가 같은 표시 구간·행 배치 알고리즘을 사용한다.
	// 파생 행은 원래 이름 있는 track 안에서만 늘어나며 저장 문서에 새 track을 만들지 않는다.
	std::size_t nextRow = 1u; // 순차 Stage strip이 첫 행을 사용하므로 box 행은 1부터 시작한다.
	for (auto &lane : lanes)
	{
		if (!cache.hasPatternLane && &lane == &lanes[patternLane])
		{
			lane.firstRow = nextRow;
			lane.rowCount = 0u;
			continue;
		}
		lane.layout = CompositionTimeline::AllocateDisplayRows(std::move(lane.intervals), minimumBoxMs);
		if (&lane == &lanes[animationLane] && !worldClipLanes.empty())
		{
			if (!cache.hasBossAnimations)
				lane.layout.rowCount = 0u;
			for (auto &actorLane : worldClipLanes)
			{
				actorLane.firstRow = lane.layout.rowCount;
				const auto actorLayout =
					CompositionTimeline::AllocateDisplayRows(std::move(actorLane.intervals), minimumBoxMs);
				for (const auto &[id, row] : actorLayout.occurrenceRows)
					lane.layout.occurrenceRows.emplace(id, actorLane.firstRow + row);
				lane.layout.rowCount += actorLayout.rowCount;
			}
		}
		lane.rowCount = lane.layout.rowCount;
		lane.firstRow = nextRow;
		nextRow += lane.rowCount;
	}
	cache.nextRow = nextRow;
}

// Render_Timeline의 zoom/카메라 꼬리 계산 뒤 호출하며 표시 행의 파생 캐시만 소유한다.
// draft·World 목록 세대뿐 아니라 pattern ID, 최소 표시 폭, World 편집 가능 여부와
// camera blend-out 목록이 모두 같아야 기존 행을 재사용한다. 픽셀 위치·선택은 캐시하지 않는다.
// m_pTimelineLayout 생성 후 하위 helper를 호출하므로 그 참조는 이번 호출 동안 유효하다.
// cameraBlendOutMs는 재구축한 경우에만 기존처럼 캐시로 move하며 문서·저장 상태는 변경하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Ensure_TimelineLayout(const KOUKU_SAYDON_COMPOSITION_PATTERN *pattern,
	const std::string &patternId, const std::uint64_t minimumBoxMs, std::vector<std::uint32_t> &cameraBlendOutMs)
{
	constexpr std::size_t animationLane = 0u, logicLane = 1u, summonLane = 2u, worldLane = 3u, sceneLane = 4u;
	constexpr std::size_t presentationLaneBegin = 5u, patternLane = 11u, laneCount = 12u;
	const auto presentationLane = [](const KOUKU_SAYDON_PRESENTATION_KIND kind) {
		return presentationLaneBegin +
			(kind == KOUKU_SAYDON_PRESENTATION_KIND::SUBTITLE ? 5u : static_cast<std::size_t>(kind));
	};
	// unique_ptr가 캐시 객체 수명을 소유하고 아래 참조는 그 객체를 복사하지 않는 별칭이다. helper는 같은 객체를 채우며 소유권을 가져가지 않는다.
	if (!m_pTimelineLayout)
		m_pTimelineLayout = std::make_unique<TIMELINE_LAYOUT_CACHE>();
	auto &cache = *m_pTimelineLayout;
	auto &lanes = cache.lanes;
	auto &worldClipLanes = cache.worldClipLanes;
	auto &worldClipRows = cache.worldClipRows;
	// 픽셀 위치와 선택은 매 프레임 계산하고 문서에서 파생한 행 배치만 재사용한다.
	// 별도 소유자인 카메라 저작값과 World 목록의 변경도 아래 캐시 조건에 포함한다.
	if (!cache.Matches(m_iDraftGeneration, m_iWorldSequenceResourceGeneration, patternId, minimumBoxMs,
			bool(m_EditWorldAnimation), cameraBlendOutMs))
	{
		Engine::CProfilerScope rebuildScope(
			CGameInstance::Get().Get_Profiler(), "ImGui.Composition.Timeline.Layout.Rebuild");
		lanes = {};
		worldClipLanes.clear();
		worldClipRows.clear();
		cache.hasPatternLane = Find_TimelineParent(m_Draft, patternId) != nullptr;
		static_assert(patternLane + 1u == laneCount);
		std::uint64_t stageBaseMs = 0u;
		for (const auto &stage : pattern->Stages)
		{
			for (const auto &box : stage.AnimationOccurrences)
				lanes[animationLane].intervals.push_back(
					{box.strOccurrenceId, double(stageBaseMs + box.iStartOffsetMs), double(box.iPlayMs)});
			stageBaseMs += stage.iDurationMs;
		}
		// occurrence/slot별 표시 행을 따로 둔다. 빈 시간 구간이 겹치더라도 다른 actor의
		// clip을 같은 소유 행으로 합쳐 보이지 않게 한다.
		cache.hasBossAnimations = !lanes[animationLane].intervals.empty();
		Collect_TimelineWorldClipRows(pattern);
		const auto appendIntervals = [&](const std::size_t lane, const auto &boxes) {
			for (const auto &box : boxes)
				lanes[lane].intervals.push_back({box.strOccurrenceId, double(box.iStartMs), double(box.iDurationMs)});
		};
		appendIntervals(patternLane, pattern->PatternOccurrences);
		appendIntervals(logicLane, pattern->LogicOccurrences);
		appendIntervals(summonLane, pattern->SummonOccurrences);
		appendIntervals(worldLane, pattern->WorldOccurrences);
		appendIntervals(sceneLane, pattern->SceneProfileOccurrences);
		for (const auto &box : pattern->PresentationOccurrences)
		{
			const auto *resource = Find_DraftPresentationResource(box.strResourceId);
			if (!resource)
				continue;
			std::uint32_t tailMs = 0u;
			if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
				if (const auto *shot = Find_AuthoringCamera(resource->strAssetId))
					tailMs = shot->iBlendOutMs;
			lanes[presentationLane(resource->eKind)].intervals.push_back({box.strOccurrenceId, double(box.iStartMs),
				double(box.iDurationMs), double(tailMs), box.strSelectionGroupId});
		}
		Allocate_TimelineDisplayRows(minimumBoxMs);
		cache.draftGeneration = m_iDraftGeneration;
		cache.worldGeneration = m_iWorldSequenceResourceGeneration;
		cache.patternId = patternId;
		cache.minimumBoxMs = minimumBoxMs;
		cache.canEditWorldAnimation = bool(m_EditWorldAnimation);
		cache.cameraBlendOutMs = std::move(cameraBlendOutMs);
	}
}

void Client::CKoukuSaydonActionWorkbench::Render_Timeline()
{
	if (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN &&
		m_ePatternSelection != KOUKU_PATTERN_SELECTION::BUNDLE)
		Render_EditHistoryButtons("EmptyTimelineHistory");
	const auto &io = ImGui::GetIO();
	const bool additiveSelection = io.KeyCtrl || io.KeyShift;
	Engine::CProfilerScope paneScope(CGameInstance::Get().Get_Profiler(), "ImGui.Composition.Timeline");
	Render_CompleteSequenceTransport();
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BINGO_PARENT)
	{
		Render_BingoParentTimeline();
		return;
	}
	if (m_bBingoParentReturn && ImGui::Button("Back to Bingo Parent"))
	{
		std::string status;
		(void)Select_BingoParent(status);
		return;
	}
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE)
	{
		Render_BundleTimeline();
		return;
	}
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::FOLDER)
	{
		const auto *folder = Find_Folder(m_Draft, m_strSelectedFolderId);
		if (!folder)
		{
			ImGui::TextDisabled("The selected Parent is unavailable.");
			return;
		}
		ImGui::SeparatorText("Parent Timeline");
		ImGui::TextWrapped("%s", folder->strDisplayName.c_str());
		if (!folder->strLoadError.empty())
		{
			ImGui::TextWrapped("%s", folder->strLoadError.c_str());
			return;
		}
		ImGui::TextWrapped(
			"This Parent has no timeline yet. Use Append Pattern beside Publish All Patterns to place a complete child Pattern, or create an empty timeline to edit common Animation, Effect, Logic and World rows.");
		ImGui::BeginDisabled(!m_bHasDraft || !m_Document.Is_Fresh());
		if (ImGui::Button("Create Parent Timeline##Sequencer"))
		{
			const auto folderId = folder->strFolderId;
			std::string status;
			(void)Create_ParentTimeline(folderId, status);
		}
		ImGui::EndDisabled();
		if (!m_strStatus.empty())
			ImGui::TextWrapped("%s", m_strStatus.c_str());
		return;
	}
	if (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN)
	{
		ImGui::TextDisabled("Select a Parent, playback bundle or Pattern to edit its sequence.");
		return;
	}
	if (m_strTimelineSelectionPatternId != m_strSelectedPatternId)
	{
		Clear_TimelineSelection();
		m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	}
	const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool_t patternReady = nullptr != pattern && pattern->strLoadError.empty();
	const std::string patternId = nullptr == pattern ? std::string{} : pattern->strPatternId;
	if (patternReady && Find_TimelineParent(m_Draft, patternId))
	{
		ImGui::TextDisabled("Parent child Pattern boxes: %zu", pattern->PatternOccurrences.size());
		if (pattern->PatternOccurrences.empty())
		{
			std::vector<KOUKU_SAYDON_COMPOSITION_CROSS_DIRECTION_WINDOW> directions;
			std::string resolveStatus;
			if (CKoukuSaydonCompositionDocument::Try_ResolveCrossDirectionWindows(
					m_Draft, *pattern, directions, resolveStatus) &&
				!directions.empty())
				ImGui::TextWrapped(
					"This Parent runs its four-direction Summon. Separate child Pattern boxes are unnecessary; the Summon chooses one real body and three clones.");
			else if (!pattern->Stages.empty() || !pattern->LogicOccurrences.empty() ||
				!pattern->SummonOccurrences.empty() || !pattern->WorldOccurrences.empty() ||
				!pattern->SceneProfileOccurrences.empty() || !pattern->PresentationOccurrences.empty())
				ImGui::TextWrapped(
					"This Parent plays its common timeline rows. A named Summon needs a configured playback policy or explicit spawned Patterns to create actors.");
			else
				ImGui::TextWrapped(
					"This Parent has no playback rows yet. Append a Pattern or add an Animation, Effect, Logic, Summon or World row.");
		}
	}
	// 모든 lane의 선택은 stable occurrence ID로 보관하고 Detail 종류는 별도로 정한다.
	// Append/Detail에서 box만 먼저 선택한 경우 현재 batch 선택에도 그 ID를 반영한다.
	if (patternReady && m_TimelineSelectedStageIds.empty() && m_TimelineSelectedOccurrenceIds.empty())
		for (const auto *id :
			{&m_strSelectedOccurrenceId, &m_strSelectedLogicOccurrenceId, &m_strSelectedSummonOccurrenceId,
				&m_strSelectedWorldOccurrenceId, &m_strSelectedSceneProfileOccurrenceId,
				&m_strSelectedPresentationOccurrenceId, &m_strSelectedPatternOccurrenceId})
			if (!id->empty())
			{
				m_TimelineSelectedOccurrenceIds.push_back(*id);
				break;
			}
	const auto durationMs = patternReady ? Pattern_DurationMs(*pattern) : 0u;
	if (m_strCursorPatternId != patternId)
	{
		m_strCursorPatternId = patternId;
		m_iCursorMs = 0u;
	}
	m_iCursorMs = (std::min)(m_iCursorMs, durationMs);
	const bool_t patternPreview = patternReady && m_PreviewState.bPlaying && m_PreviewState.strPatternId == patternId;
	const bool_t serverPlayback =
		!m_bSequenceWorkspace && CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().Is_InFlight();
	if (Render_TimelineControls(pattern, patternId, patternReady, durationMs, patternPreview, serverPlayback))
		return;
	// 툴바 명령은 위에서 처리하고 반환했으므로, 이후 키보드 요청은 별도 false 상태에서 모은다.
	bool_t deleteRequested = false;
	bool_t duplicateRequested = false;
	if (!patternReady)
	{
		if (nullptr == pattern)
			ImGui::TextDisabled("Select a Pattern.");
		else
			ImGui::TextWrapped("%s", pattern->strLoadError.c_str());
		return;
	}

	std::optional<Engine::CProfilerScope> layoutScope;
	layoutScope.emplace(CGameInstance::Get().Get_Profiler(), "ImGui.Composition.Timeline.Layout");
	Rebuild_PatternChildRows(*pattern);
	if (!pattern->PatternOccurrences.empty())
	{
		ImGui::Checkbox("Pattern 하위 Animation / Logic / Effect 표시", &m_bShowPatternChildRows);
		if (!m_strChildRowsStatus.empty())
			ImGui::TextWrapped("%s", m_strChildRowsStatus.c_str());
	}
	constexpr f32_t labelWidth = CompositionTimeline::LabelWidth;
	const auto metrics = CompositionTimeline::GetCompactRowMetrics();
	const f32_t rulerHeight = metrics.rulerHeight;
	const f32_t laneHeight = metrics.laneHeight;
	const f32_t boxHeight = metrics.boxHeight;
	const ImVec2 available = ImGui::GetContentRegionAvail();
	if (m_bFitRequested && durationMs > 0u)
	{
		m_fPixelsPerSecond = CompositionTimeline::FitPixelsPerSecond(available.x - labelWidth - 24.f, durationMs);
		m_bFitRequested = false;
	}
	const f32_t scale = m_fPixelsPerSecond * 0.001f;
	std::uint32_t cameraDisplayEnd = durationMs;
	std::vector<std::uint32_t> cameraBlendOutMs;
	for (const auto &row : pattern->PresentationOccurrences)
		if (const auto *resource = Find_DraftPresentationResource(row.strResourceId);
			resource && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
		{
			const auto *shot = Find_AuthoringCamera(resource->strAssetId);
			const auto tailMs = shot ? shot->iBlendOutMs : 0u;
			cameraBlendOutMs.push_back(tailMs);
			if (shot)
				cameraDisplayEnd = (std::max)(cameraDisplayEnd, row.iStartMs + row.iDurationMs + tailMs);
		}
	const f32_t timelineWidth = (std::max)(available.x, labelWidth + cameraDisplayEnd * scale + 24.f);
	constexpr std::size_t animationLane = 0u, logicLane = 1u, summonLane = 2u, worldLane = 3u, sceneLane = 4u;
	constexpr std::size_t presentationLaneBegin = 5u, patternLane = 11u, laneCount = 12u;
	const auto presentationLane = [](const KOUKU_SAYDON_PRESENTATION_KIND kind) {
		return presentationLaneBegin +
			(kind == KOUKU_SAYDON_PRESENTATION_KIND::SUBTITLE ? 5u : static_cast<std::size_t>(kind));
	};
	const auto minimumBoxMs = static_cast<std::uint64_t>(std::ceil(CompositionTimeline::MinimumBoxWidth / scale));
	Ensure_TimelineLayout(pattern, patternId, minimumBoxMs, cameraBlendOutMs);
	auto &cache = *m_pTimelineLayout;
	auto &lanes = cache.lanes;
	auto &worldClipLanes = cache.worldClipLanes;
	auto &worldClipRows = cache.worldClipRows;
	std::size_t nextRow = cache.nextRow;
	std::vector<std::size_t> childDisplayRows(m_PatternChildRows.size());
	std::map<std::string, std::size_t> childAnimationLanes;
	if (m_bShowPatternChildRows)
		for (std::size_t i = 0; i < m_PatternChildRows.size(); ++i)
		{
			const auto &row = m_PatternChildRows[i];
			if (row.stageId.empty())
			{
				childDisplayRows[i] = nextRow++;
				continue;
			}
			// 확장된 Animation은 부모 occurrence별 행을 공유한다.
			// 화면용 반복 ID와 클릭 시 편집할 원본 box ID는 따로 유지한다.
			const auto repeatAt = row.runtimeId.rfind(".r");
			const auto key = row.runtimeId.substr(0, repeatAt) + ":" + row.patternId;
			const auto [lane, inserted] = childAnimationLanes.try_emplace(key, nextRow);
			if (inserted)
				++nextRow;
			childDisplayRows[i] = lane->second;
		}
	const f32_t height = rulerHeight + laneHeight * static_cast<f32_t>(nextRow);
	layoutScope.reset();
	if (!ImGui::BeginChild(
			"##KoukuTimeline", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImGui::EndChild();
		return;
	}
	std::optional<Engine::CProfilerScope> drawScope;
	drawScope.emplace(CGameInstance::Get().Get_Profiler(), "ImGui.Composition.Timeline.Draw");
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	ImDrawList *draw = ImGui::GetWindowDrawList();
	const auto drawBox = [&](const ImVec2 min, const ImVec2 max, const ImU32 color, const bool selected,
							 const char *label, const bool leftGrip = true, const bool rightGrip = true) {
		CompositionTimeline::DrawBox(draw, min, max, color, selected, label, leftGrip, rightGrip);
	};
	const auto stageGeometry = CompositionTimeline::DrawCategoryLane(draw, ImVec2(origin.x, origin.y + rulerHeight),
		timelineWidth, labelWidth, 1u, laneHeight, "Stages", CompositionTimeline::StageLabelColor);
	std::array<const char *, laneCount> laneLabels{"Animation", "Logic", "Summon", "World", "Scene Profile"};
	std::array<ImU32, laneCount> laneLabelColors{CompositionTimeline::AnimationLabelColor,
		CompositionTimeline::LogicLabelColor, CompositionTimeline::SummonLabelColor,
		CompositionTimeline::WorldLabelColor, CompositionTimeline::SceneProfileLabelColor};
	for (std::size_t i = 0u; i < 6u; ++i)
	{
		laneLabels[presentationLaneBegin + i] = Presentation_Label(
			i == 5u ? KOUKU_SAYDON_PRESENTATION_KIND::SUBTITLE : static_cast<KOUKU_SAYDON_PRESENTATION_KIND>(i));
		laneLabelColors[presentationLaneBegin + i] = CompositionTimeline::PresentationLabelColor;
	}
	laneLabels[patternLane] = "Pattern";
	laneLabelColors[patternLane] = IM_COL32(180, 160, 240, 255);
	std::array<CompositionTimeline::CATEGORY_LANE_GEOMETRY, laneCount> laneGeometry{};
	for (std::size_t lane = 0u; lane < laneCount; ++lane)
		laneGeometry[lane] = CompositionTimeline::DrawCategoryLane(draw,
			ImVec2(origin.x, origin.y + rulerHeight + laneHeight * static_cast<f32_t>(lanes[lane].firstRow)),
			timelineWidth, labelWidth, lanes[lane].rowCount, laneHeight, laneLabels[lane], laneLabelColors[lane],
			lane % 2u == 0u);
	const auto boxY = [&](const std::size_t lane, const std::string &occurrenceId) {
		return laneGeometry[lane].SubrowOrigin(lanes[lane].layout.occurrenceRows.at(occurrenceId)).y;
	};
	// publisher는 저장 파일을 읽으며 선택·scrub·box 편집은 메모리 draft/preview에 반영한다.
	// publish 중에는 Save 명령을 막는다. source-write 잠금 자체의 수명은 Save_Atomic 호출 범위다.
	const bool_t canInteract = patternReady && !ImGui::GetIO().WantTextInput;
	PRESENTATION_TIMELINE_MOVE groupMove;
	bool groupMoveReady = false;
	if (m_iTimelineDragMode == 3 && !m_strTimelineGroupDragOccurrenceId.empty())
	{
		if (!canInteract || m_bTimelineMarqueeActive || m_iTimelineGroupDragGeneration != m_iDraftGeneration ||
			m_TimelineSelectedOccurrenceIds != m_TimelineGroupDragOccurrenceIds || !m_TimelineSelectedStageIds.empty())
		{
			m_iTimelineDragMode = -1;
			m_strStatus =
				"Presentation selection drag canceled because input, draft or selection changed; previous timing preserved.";
		}
		else
		{
			std::string status;
			groupMoveReady = Prepare_PresentationTimelineMove(m_Draft, *pattern, m_TimelineGroupDragOccurrenceIds,
				static_cast<std::int64_t>(std::llround(ImGui::GetMouseDragDelta().x / scale)), groupMove, status);
			if (!groupMoveReady)
			{
				m_iTimelineDragMode = -1;
				m_strStatus = status;
			}
		}
	}
	for (const auto &lane : lanes)
		for (const auto &group : lane.layout.groups)
		{
			const bool movingGroup = groupMoveReady &&
				std::any_of(groupMove.occurrenceIds.begin(), groupMove.occurrenceIds.end(), [&](const auto &id) {
					const auto *member = Find_PresentationBox(*pattern, id);
					return member && member->strSelectionGroupId == group.groupId;
				});
			const auto delta = movingGroup ? groupMove.deltaMs : 0;
			const float x = origin.x + labelWidth + static_cast<float>(std::int64_t(group.startMs) + delta) * scale;
			const float endX = origin.x + labelWidth + static_cast<float>(std::int64_t(group.endMs) + delta) * scale;
			const float y = origin.y + rulerHeight + static_cast<float>(lane.firstRow + group.firstRow) * laneHeight;
			const float bottom = y + static_cast<float>(group.rowCount) * laneHeight;
			draw->AddRectFilled(ImVec2(x - 2.f, y), ImVec2(endX + 2.f, bottom), IM_COL32(95, 186, 180, 25), 3.f);
			draw->AddRect(ImVec2(x - 2.f, y), ImVec2(endX + 2.f, bottom), IM_COL32(125, 205, 198, 125), 3.f);
		}
	struct TIMELINE_HIT_BOX
	{
		std::string stageId;
		std::string occurrenceId;
		ImVec2 min;
		ImVec2 max;
	};
	std::vector<TIMELINE_HIT_BOX> hitBoxes;
	const auto contains = [](const auto &ids, const std::string &id) {
		return std::find(ids.begin(), ids.end(), id) != ids.end();
	};
	CompositionTimeline::DrawRuler(draw, ImVec2(origin.x + labelWidth, origin.y),
		ImVec2(origin.x + timelineWidth, origin.y + rulerHeight), durationMs, m_fPixelsPerSecond);
	/* Play 전이나 Stop 뒤에도 ruler에서 선택한 ms 시각의 preview를 요청한다. */
	ImGui::SetCursorScreenPos(ImVec2(origin.x + labelWidth, origin.y));
	ImGui::InvisibleButton("##KoukuRuler", ImVec2((std::max)(8.f, durationMs * scale), rulerHeight));
	if (!serverPlayback && canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActive() && durationMs > 0u)
	{
		const f32_t mouseMs = (ImGui::GetIO().MousePos.x - (origin.x + labelWidth)) / scale;
		const auto clockMs =
			static_cast<std::uint32_t>(std::clamp(std::llround(mouseMs), 0ll, static_cast<long long>(durationMs)));
		std::string scrubStatus;
		(void)Request_PatternScrub(patternId, clockMs, scrubStatus);
	}
	const std::uint32_t playheadMs = patternPreview ? m_PreviewState.iClockMs : m_iCursorMs;
	if (playheadMs <= durationMs)
	{
		const f32_t playheadX = origin.x + labelWidth + playheadMs * scale;
		draw->AddLine(ImVec2(playheadX, origin.y), ImVec2(playheadX, origin.y + height),
			(patternPreview || serverPlayback) ? IM_COL32(255, 220, 72, 230) : IM_COL32(200, 200, 200, 140), 1.5f);
	}
	for (const auto &actorLane : worldClipLanes)
	{
		const float y = laneGeometry[animationLane].SubrowOrigin(actorLane.firstRow).y;
		if (ImGui::IsMouseHoveringRect(ImVec2(origin.x, y), ImVec2(origin.x + labelWidth, y + laneHeight)))
			ImGui::SetTooltip("Animation: %s\nWorld occurrence: %s\nSlot: %s\nClips keep this actor's own timing.",
				actorLane.label.c_str(), actorLane.occurrenceId.c_str(), actorLane.slotId.c_str());
	}

	std::string editStageId, editOccurrenceId, editLogicBoxId, editPatternBoxId;
	std::uint32_t newPatternStartMs = 0u, newPatternDurationMs = 0u;
	bool_t newPatternRepeat = false;
	std::uint32_t newOffset = 0u, newSourceStart = 0u, newSourceEnd = 0u, newPlayMs = 0u, newStageDuration = 0u;
	std::uint32_t newLogicStartMs = 0u, newLogicDurationMs = 0u;
	std::string editSummonBoxId;
	std::uint32_t newSummonStartMs = 0u, newSummonDurationMs = 0u;
	std::string editWorldBoxId;
	std::uint32_t newWorldStartMs = 0u, newWorldDurationMs = 0u;
	std::string editSceneBoxId;
	std::uint32_t newSceneStartMs = 0u, newSceneDurationMs = 0u;
	bool commitGroupMove = false;
	std::string editPresentationBoxId;
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE editedPresentationBox;
	int32_t moveStageDirection = 0;
	std::uint32_t stageStartMs = 0u;
	for (const auto &stage : pattern->Stages)
	{
		const f32_t stageX = origin.x + labelWidth + stageStartMs * scale;
		const f32_t stageY = stageGeometry.SubrowOrigin(0u).y;
		const f32_t stageWidth = (std::max)(8.f, stage.iDurationMs * scale);
		ImGui::PushID(stage.strStageId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(stageX, stageY));
		ImGui::InvisibleButton("##StageBox", ImVec2(stageWidth, boxHeight));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = stage.iDurationMs;
			m_iTimelineDragMode = ImGui::GetIO().MousePos.x > stageX + stageWidth - 7.f ? 3 : 0;
			Select_TimelineBox(stage.strStageId, {}, additiveSelection);
			if (additiveSelection)
				m_iTimelineDragMode = -1;
		}
		f32_t shownStageWidth = stageWidth;
		if (canInteract && !m_bTimelineMarqueeActive && (ImGui::IsItemActive() || ImGui::IsItemDeactivated()) &&
			3 == m_iTimelineDragMode)
		{
			std::uint32_t occupiedEnd = 1u;
			for (const auto &box : stage.AnimationOccurrences)
				occupiedEnd = (std::max)(occupiedEnd, box.iStartOffsetMs + box.iPlayMs);
			const auto delta = static_cast<int64_t>(std::llround(ImGui::GetMouseDragDelta().x / scale));
			const auto changed =
				static_cast<std::uint32_t>(std::clamp(static_cast<int64_t>(m_iDragOriginOffsetMs) + delta,
					static_cast<int64_t>(occupiedEnd), static_cast<int64_t>(MAX_EDITOR_TIME_MS)));
			shownStageWidth = (std::max)(8.f, changed * scale);
			if (ImGui::IsItemDeactivated() && changed != stage.iDurationMs)
			{
				editStageId = stage.strStageId;
				newStageDuration = changed;
			}
		}
		drawBox(ImVec2(stageX, stageY), ImVec2(stageX + shownStageWidth, stageY + boxHeight),
			CompositionTimeline::StageColor, contains(m_TimelineSelectedStageIds, stage.strStageId),
			stage.strStageId.c_str(), false, true);
		hitBoxes.push_back(
			{stage.strStageId, {}, ImVec2(stageX, stageY), ImVec2(stageX + shownStageWidth, stageY + boxHeight)});
		ImGui::PopID();

		for (const auto &occurrence : stage.AnimationOccurrences)
		{
			const f32_t y = boxY(animationLane, occurrence.strOccurrenceId);
			const f32_t x = stageX + occurrence.iStartOffsetMs * scale;
			const f32_t width = (std::max)(8.f, occurrence.iPlayMs * scale);
			ImGui::PushID(occurrence.strOccurrenceId.c_str());
			ImGui::SetCursorScreenPos(ImVec2(x, y));
			ImGui::InvisibleButton("##AnimationBox", ImVec2(width, boxHeight));
			if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
			{
				m_iDragOriginOffsetMs = occurrence.iStartOffsetMs;
				m_iDragOriginSourceMs = occurrence.iSourceStartMs;
				m_iDragOriginPlayMs = occurrence.iPlayMs;
				const f32_t mouse = ImGui::GetIO().MousePos.x;
				const auto gesture = CompositionTimeline::HitBoxGesture(mouse, x, x + width, 6.f, true, true);
				m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START
					? 1
					: (gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
				Select_TimelineBox(stage.strStageId, occurrence.strOccurrenceId, additiveSelection);
				if (additiveSelection)
					m_iTimelineDragMode = -1;
			}
			f32_t shownX = x, shownWidth = width;
			if (canInteract && !m_bTimelineMarqueeActive && m_iTimelineDragMode >= 0 &&
				(ImGui::IsItemActive() || ImGui::IsItemDeactivated()))
			{
				int64_t offset = m_iDragOriginOffsetMs, source = m_iDragOriginSourceMs, play = m_iDragOriginPlayMs;
				const auto delta = static_cast<int64_t>(std::llround(ImGui::GetMouseDragDelta().x / scale));
				auto trimmed = occurrence;
				trimmed.iStartOffsetMs = static_cast<std::uint32_t>(offset);
				trimmed.iSourceStartMs = static_cast<std::uint32_t>(source);
				trimmed.iPlayMs = static_cast<std::uint32_t>(play);
				if (1 == m_iTimelineDragMode || 2 == m_iTimelineDragMode)
				{
					std::uint32_t nativeMs = 0u;
					(void)Resolve_NativeClipMs(occurrence, nativeMs);
					(void)CKoukuSaydonCompositionDocument::Trim_AnimationWindow(
						trimmed, delta, 1 == m_iTimelineDragMode, stage.iDurationMs, nativeMs);
					offset = trimmed.iStartOffsetMs;
					source = trimmed.iSourceStartMs;
					play = trimmed.iPlayMs;
				}
				else
					offset = std::clamp(offset + delta, int64_t{0}, static_cast<int64_t>(stage.iDurationMs) - play);
				shownX = stageX + static_cast<f32_t>(offset) * scale;
				shownWidth = (std::max)(8.f, static_cast<f32_t>(play) * scale);
				if (ImGui::IsItemDeactivated() &&
					(offset != occurrence.iStartOffsetMs || source != occurrence.iSourceStartMs ||
						play != occurrence.iPlayMs))
				{
					editOccurrenceId = occurrence.strOccurrenceId;
					newOffset = static_cast<std::uint32_t>(offset);
					newSourceStart = static_cast<std::uint32_t>(source);
					newSourceEnd = trimmed.iSourceEndMs;
					newPlayMs = static_cast<std::uint32_t>(play);
				}
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s\n%s | offset %u ms | source %u ms | play %u ms | x%.2f | %s",
					occurrence.strOccurrenceId.c_str(), stage.strStageId.c_str(), occurrence.iStartOffsetMs,
					occurrence.iSourceStartMs, occurrence.iPlayMs, occurrence.fPlayRate,
					occurrence.strEndPolicy.c_str());
			drawBox(ImVec2(shownX, y), ImVec2(shownX + shownWidth, y + boxHeight), CompositionTimeline::AnimationColor,
				contains(m_TimelineSelectedOccurrenceIds, occurrence.strOccurrenceId) ||
					contains(m_TimelineSelectedStageIds, stage.strStageId),
				occurrence.strRuntimeClip.c_str());
			hitBoxes.push_back({stage.strStageId, occurrence.strOccurrenceId, ImVec2(shownX, y),
				ImVec2(shownX + shownWidth, y + boxHeight)});
			ImGui::PopID();
		}
		stageStartMs += stage.iDurationMs;
	}
	/* 종류별 저작 시각은 유지하면서 모든 occurrence가 같은 선택 집합에 참여한다. */
	for (const auto &box : pattern->LogicOccurrences)
	{
		const f32_t logicBoxY = boxY(logicLane, box.strOccurrenceId);
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, logicBoxY));
		ImGui::InvisibleButton("##LogicBox", ImVec2(width, boxHeight));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture =
				CompositionTimeline::HitBoxGesture(ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START
				? 1
				: (gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			Select_TimelineBox({}, box.strOccurrenceId, additiveSelection);
			if (additiveSelection)
				m_iTimelineDragMode = -1;
		}
		f32_t shownX = x, shownWidth = width;
		if (groupMoveReady &&
			std::find(groupMove.logicIds.begin(), groupMove.logicIds.end(), box.strOccurrenceId) !=
				groupMove.logicIds.end())
			shownX += static_cast<float>(groupMove.deltaMs) * scale;
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
			if (ImGui::IsItemDeactivated() && (start != box.iStartMs || length != box.iDurationMs))
			{
				editLogicBoxId = box.strOccurrenceId;
				newLogicStartMs = static_cast<std::uint32_t>(start);
				newLogicDurationMs = static_cast<std::uint32_t>(length);
			}
		}
		const auto *const logic = Find_Logic(m_Draft, box.strLogicId);
		std::string label =
			nullptr == logic ? box.strLogicId : logic->strDisplayName + " [" + logic->strLogicType + "]";
		for (const KOUKU_SAYDON_OUTCOME_SLOT slot :
			{KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS, KOUKU_SAYDON_OUTCOME_SLOT::FAIL, KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT})
		{
			const std::vector<std::string> &targets = box.Outcomes(slot);
			if (targets.empty())
				continue;
			const auto *const first = Find_Logic(m_Draft, targets.front());
			label += std::string("  ") + Outcome_SlotLabel(slot) + ">" +
				(nullptr == first ? targets.front() : first->strDisplayName) +
				(targets.size() > 1u ? " +" + std::to_string(targets.size() - 1u) : "");
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\n%s | start %u ms | lifetime %u ms", box.strOccurrenceId.c_str(), label.c_str(),
				box.iStartMs, box.iDurationMs);
		drawBox(ImVec2(shownX, logicBoxY), ImVec2(shownX + shownWidth, logicBoxY + boxHeight), TIMELINE_LOGIC_COLOR,
			contains(m_TimelineSelectedOccurrenceIds, box.strOccurrenceId), label.c_str());
		hitBoxes.push_back(
			{{}, box.strOccurrenceId, ImVec2(shownX, logicBoxY), ImVec2(shownX + shownWidth, logicBoxY + boxHeight)});
		ImGui::PopID();
	}
	/* Summon box의 양 끝은 저작된 시작·종료 ms다. 여기서는 그 구간을 표시하고 편집한다. */
	for (const auto &box : pattern->SummonOccurrences)
	{
		const f32_t summonBoxY = boxY(summonLane, box.strOccurrenceId);
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, summonBoxY));
		ImGui::InvisibleButton("##SummonBox", ImVec2(width, boxHeight));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture =
				CompositionTimeline::HitBoxGesture(ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START
				? 1
				: (gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			Select_TimelineBox({}, box.strOccurrenceId, additiveSelection);
			if (additiveSelection)
				m_iTimelineDragMode = -1;
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
			if (ImGui::IsItemDeactivated() && (start != box.iStartMs || length != box.iDurationMs))
			{
				editSummonBoxId = box.strOccurrenceId;
				newSummonStartMs = static_cast<std::uint32_t>(start);
				newSummonDurationMs = static_cast<std::uint32_t>(length);
			}
		}
		const auto *const summon = Find_Summon(m_Draft, box.strSummonId);
		const std::string label = nullptr == summon ? box.strSummonId : summon->strDisplayName;
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\n%s | spawn %u ms | lifetime %u ms", box.strOccurrenceId.c_str(), label.c_str(),
				box.iStartMs, box.iDurationMs);
		drawBox(ImVec2(shownX, summonBoxY), ImVec2(shownX + shownWidth, summonBoxY + boxHeight), TIMELINE_SUMMON_COLOR,
			contains(m_TimelineSelectedOccurrenceIds, box.strOccurrenceId), label.c_str());
		hitBoxes.push_back(
			{{}, box.strOccurrenceId, ImVec2(shownX, summonBoxY), ImVec2(shownX + shownWidth, summonBoxY + boxHeight)});
		ImGui::PopID();
	}
	/* Pattern box의 고정 실행 구간에는 하위 Logic도 포함된다. */
	for (const auto &box : pattern->PatternOccurrences)
	{
		const f32_t patternBoxY = boxY(patternLane, box.strOccurrenceId);
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, patternBoxY));
		ImGui::InvisibleButton("##PatternBox", ImVec2(width, boxHeight));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture =
				CompositionTimeline::HitBoxGesture(ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START
				? 1
				: (gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			Select_TimelineBox({}, box.strOccurrenceId, additiveSelection);
			if (additiveSelection)
				m_iTimelineDragMode = -1;
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
			if (ImGui::IsItemDeactivated() && (start != box.iStartMs || length != box.iDurationMs))
			{
				editPatternBoxId = box.strOccurrenceId;
				newPatternStartMs = static_cast<std::uint32_t>(start);
				newPatternDurationMs = static_cast<std::uint32_t>(length);
				newPatternRepeat = box.bRepeat;
			}
		}
		const auto *const child = Find_Pattern(m_Draft, box.strPatternId);
		const std::string label =
			(nullptr == child ? box.strPatternId : child->strDisplayName) + (box.bRepeat ? " [Repeat]" : "");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\n%s | spawn %u ms | lifetime %u ms", box.strOccurrenceId.c_str(), label.c_str(),
				box.iStartMs, box.iDurationMs);
		drawBox(ImVec2(shownX, patternBoxY), ImVec2(shownX + shownWidth, patternBoxY + boxHeight),
			IM_COL32(115, 90, 170, 255), contains(m_TimelineSelectedOccurrenceIds, box.strOccurrenceId), label.c_str());
		hitBoxes.push_back({{}, box.strOccurrenceId, ImVec2(shownX, patternBoxY),
			ImVec2(shownX + shownWidth, patternBoxY + boxHeight)});
		ImGui::PopID();
	}

	if (m_bShowPatternChildRows)
	{
		for (std::size_t i = 0u; i < m_PatternChildRows.size(); ++i)
		{
			const auto &row = m_PatternChildRows[i];
			const float y = origin.y + rulerHeight + laneHeight * static_cast<float>(childDisplayRows[i]);
			if (i == 0u ||
				std::find(childDisplayRows.begin(), childDisplayRows.begin() + i, childDisplayRows[i]) ==
					childDisplayRows.begin() + i)
				draw->AddText(ImVec2(origin.x + 12.f, y + 4.f), IM_COL32(180, 160, 220, 255),
					row.stageId.empty() ? "  > Child" : "  > Animation");
			const float x = origin.x + labelWidth + row.startMs * scale;
			const float width = (std::max)(8.f, row.durationMs * scale);
			ImGui::PushID(row.runtimeId.c_str());
			ImGui::SetCursorScreenPos(ImVec2(x, y));
			const bool openSource = ImGui::InvisibleButton("##ExpandedChild", ImVec2(width, boxHeight));
			drawBox(ImVec2(x, y), ImVec2(x + width, y + boxHeight), row.color, false, row.label.c_str());
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s\n%u..%u ms in Parent\nSource: %s / %s\nClick to edit the shared source box.",
					row.label.c_str(), row.startMs, row.startMs + row.durationMs, row.patternId.c_str(),
					row.occurrenceId.c_str());
			ImGui::PopID();
			if (openSource)
			{
				const auto selected = row;
				std::string status;
				if (Select_PatternById(selected.patternId, status))
				{
					m_strParentReturnPatternId = patternId;
					Select_TimelineBox(selected.stageId, selected.occurrenceId, false);
				}
				ImGui::EndChild();
				return;
			}
		}
	}

	for (const auto &row : worldClipRows)
	{
		const ImVec2 top(origin.x + labelWidth + row.startMs * scale, boxY(animationLane, row.id));
		const float width = (std::max)(8.f, row.durationMs * scale);
		ImGui::PushID(row.id.c_str());
		ImGui::SetCursorScreenPos(top);
		ImGui::InvisibleButton("##WorldAnimationClip", ImVec2(width, boxHeight));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_TimelineSelectedStageIds.clear();
			m_TimelineSelectedOccurrenceIds.clear();
			m_SelectedWorldAnimationId = row.id;
			const auto gesture =
				CompositionTimeline::HitBoxGesture(ImGui::GetIO().MousePos.x, top.x, top.x + width, 6.f, true, true);
			m_WorldAnimationDragMode = !row.editable
				? -1
				: (gesture == CompositionTimeline::BoxGesture::TRIM_START		 ? 1
						  : gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2
																				 : 0);
		}
		float shownX = top.x, shownWidth = width;
		if (canInteract && row.editable && !m_bTimelineMarqueeActive && m_WorldAnimationDragMode >= 0 &&
			m_SelectedWorldAnimationId == row.id && (ImGui::IsItemActive() || ImGui::IsItemDeactivated()))
		{
			auto changed = row.edit;
			const double deltaMs = ImGui::GetMouseDragDelta().x / scale;
			if (m_WorldAnimationDragMode == 1)
			{
				const int64_t minimum = -(std::min)(int64_t(changed.startMs),
					static_cast<int64_t>(std::floor(changed.sourceInMs / row.trackRate)));
				const int64_t maximum =
					static_cast<int64_t>(std::floor((changed.sourceOutMs - changed.sourceInMs - 1u) / row.trackRate));
				const int64_t delta =
					std::clamp(static_cast<int64_t>(std::llround(deltaMs * row.localSpeed)), minimum, maximum);
				changed.startMs = static_cast<uint32_t>(int64_t(changed.startMs) + delta);
				changed.sourceInMs = static_cast<uint32_t>(
					std::clamp(int64_t(changed.sourceInMs) + static_cast<int64_t>(std::llround(delta * row.trackRate)),
						int64_t{0}, int64_t(changed.sourceOutMs) - 1));
			}
			else if (m_WorldAnimationDragMode == 2)
				changed.sourceOutMs = static_cast<uint32_t>(std::clamp(
					int64_t(changed.sourceOutMs) + static_cast<int64_t>(std::llround(deltaMs * row.nativeSpeed)),
					int64_t(changed.sourceInMs) + 1, int64_t{600000}));
			else
				changed.startMs = static_cast<uint32_t>(
					std::clamp(int64_t(changed.startMs) + static_cast<int64_t>(std::llround(deltaMs * row.localSpeed)),
						int64_t{0}, int64_t{599999}));
			shownX += static_cast<float>((int64_t(changed.startMs) - row.edit.startMs) / row.localSpeed) * scale;
			shownWidth = (std::max)(8.f,
				static_cast<float>((changed.sourceOutMs - changed.sourceInMs) / row.nativeSpeed) * scale);
			if (ImGui::IsItemDeactivated())
			{
				if (changed.startMs != row.edit.startMs || changed.sourceInMs != row.edit.sourceInMs ||
					changed.sourceOutMs != row.edit.sourceOutMs)
				{
					std::string status;
					Begin_EditHistory(true);
					if (m_EditWorldAnimation(changed, status))
					{
						m_WorldAnimationEditsPending = true;
						++m_iDraftGeneration;
					}
					m_strStatus = std::move(status);
				}
				m_WorldAnimationDragMode = -1;
			}
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(row.editable ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_Arrow);
			ImGui::SetTooltip("%s\nActor: %s\nWorld occurrence: %s | Slot: %s\n%u - %u ms | Source %u - %u ms\n%s",
				row.label.c_str(), row.actorLabel.c_str(), row.occurrenceId.c_str(), row.edit.slotId.c_str(),
				row.startMs, row.startMs + row.durationMs, row.edit.sourceInMs, row.edit.sourceOutMs,
				row.editable ? "Drag center: move. Drag either edge: trim this clip. Save commits the animation."
							 : "This legacy clip has no explicit Source Out.");
		}
		drawBox(ImVec2(shownX, top.y), ImVec2(shownX + shownWidth, top.y + boxHeight), IM_COL32(105, 151, 186, 240),
			m_SelectedWorldAnimationId == row.id, row.label.c_str());
		if (row.editable)
		{
			draw->AddLine(ImVec2(shownX + 3.f, top.y + 4.f), ImVec2(shownX + 3.f, top.y + boxHeight - 4.f),
				IM_COL32(235, 245, 255, 210), 2.f);
			draw->AddLine(ImVec2(shownX + shownWidth - 3.f, top.y + 4.f),
				ImVec2(shownX + shownWidth - 3.f, top.y + boxHeight - 4.f), IM_COL32(235, 245, 255, 210), 2.f);
		}
		ImGui::PopID();
	}
	/* World box의 왼쪽은 sequence 시작 ms, 폭은 저작된 표시 구간이다.
	   Scene Profile box의 폭은 해당 profile을 유지하는 저작 구간이다. */
	for (const auto &box : pattern->WorldOccurrences)
	{
		const f32_t worldBoxY = boxY(worldLane, box.strOccurrenceId);
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, worldBoxY));
		ImGui::InvisibleButton("##WorldBox", ImVec2(width, boxHeight));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture =
				CompositionTimeline::HitBoxGesture(ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START
				? 1
				: (gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			Select_TimelineBox({}, box.strOccurrenceId, additiveSelection);
			if (additiveSelection)
				m_iTimelineDragMode = -1;
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
			if (ImGui::IsItemDeactivated() && (start != box.iStartMs || length != box.iDurationMs))
			{
				editWorldBoxId = box.strOccurrenceId;
				newWorldStartMs = static_cast<std::uint32_t>(start);
				newWorldDurationMs = static_cast<std::uint32_t>(length);
			}
		}
		const auto *const world = Find_World(m_Draft, box.strWorldId);
		char speedText[32]{};
		(void)snprintf(speedText, sizeof(speedText), " x%.2f", box.fPlaybackSpeed);
		const std::string label = (nullptr == world ? box.strWorldId : world->strDisplayName) + speedText;
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\n%s | start %u ms | shown %u ms | speed x%.2f", box.strOccurrenceId.c_str(),
				label.c_str(), box.iStartMs, box.iDurationMs, box.fPlaybackSpeed);
		drawBox(ImVec2(shownX, worldBoxY), ImVec2(shownX + shownWidth, worldBoxY + boxHeight), TIMELINE_WORLD_COLOR,
			contains(m_TimelineSelectedOccurrenceIds, box.strOccurrenceId), label.c_str());
		hitBoxes.push_back(
			{{}, box.strOccurrenceId, ImVec2(shownX, worldBoxY), ImVec2(shownX + shownWidth, worldBoxY + boxHeight)});
		ImGui::PopID();
	}
	for (const auto &box : pattern->SceneProfileOccurrences)
	{
		const f32_t sceneBoxY = boxY(sceneLane, box.strOccurrenceId);
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, sceneBoxY));
		ImGui::InvisibleButton("##SceneProfileBox", ImVec2(width, boxHeight));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			const auto gesture =
				CompositionTimeline::HitBoxGesture(ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START
				? 1
				: (gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			Select_TimelineBox({}, box.strOccurrenceId, additiveSelection);
			if (additiveSelection)
				m_iTimelineDragMode = -1;
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
			if (ImGui::IsItemDeactivated() && (start != box.iStartMs || length != box.iDurationMs))
			{
				editSceneBoxId = box.strOccurrenceId;
				newSceneStartMs = static_cast<std::uint32_t>(start);
				newSceneDurationMs = static_cast<std::uint32_t>(length);
			}
		}
		const auto *const profile = Find_SceneProfile(m_Draft, box.strSceneProfileId);
		const std::string label = nullptr == profile ? box.strSceneProfileId : profile->strDisplayName;
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\n%s | start %u ms | lifetime %u ms | blend %u ms", box.strOccurrenceId.c_str(),
				label.c_str(), box.iStartMs, box.iDurationMs, box.iBlendMs);
		drawBox(ImVec2(shownX, sceneBoxY), ImVec2(shownX + shownWidth, sceneBoxY + boxHeight),
			TIMELINE_SCENE_PROFILE_COLOR, contains(m_TimelineSelectedOccurrenceIds, box.strOccurrenceId),
			label.c_str());
		hitBoxes.push_back(
			{{}, box.strOccurrenceId, ImVec2(shownX, sceneBoxY), ImVec2(shownX + shownWidth, sceneBoxY + boxHeight)});
		ImGui::PopID();
	}
	for (const auto &box : pattern->PresentationOccurrences)
	{
		const auto *resource = Find_DraftPresentationResource(box.strResourceId);
		if (nullptr == resource)
			continue;
		const f32_t presentationLaneY = boxY(presentationLane(resource->eKind), box.strOccurrenceId);
		const f32_t x = origin.x + labelWidth + box.iStartMs * scale;
		const f32_t width = (std::max)(8.f, box.iDurationMs * scale);
		ImGui::PushID(box.strOccurrenceId.c_str());
		ImGui::SetCursorScreenPos(ImVec2(x, presentationLaneY));
		ImGui::InvisibleButton("##PresentationBox", ImVec2(width, boxHeight));
		if (canInteract && !m_bTimelineMarqueeActive && ImGui::IsItemActivated())
		{
			m_iDragOriginOffsetMs = box.iStartMs;
			m_iDragOriginPlayMs = box.iDurationMs;
			m_iDragOriginSourceMs = resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ? box.iEffectSourceStartMs
																							  : box.iSoundSourceStartMs;
			const auto gesture =
				CompositionTimeline::HitBoxGesture(ImGui::GetIO().MousePos.x, x, x + width, 6.f, true, true);
			m_iTimelineDragMode = gesture == CompositionTimeline::BoxGesture::TRIM_START
				? 1
				: (gesture == CompositionTimeline::BoxGesture::TRIM_END ? 2 : 0);
			const bool keepPresentationSelection = !additiveSelection && m_iTimelineDragMode == 0 &&
				(resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
					resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::SOUND) &&
				m_TimelineSelectedStageIds.empty() && m_strTimelineSelectionPatternId == patternId &&
				contains(m_TimelineSelectedOccurrenceIds, box.strOccurrenceId) &&
				m_TimelineSelectedOccurrenceIds.size() > 1u &&
				std::all_of(m_TimelineSelectedOccurrenceIds.begin(), m_TimelineSelectedOccurrenceIds.end(),
					[&](const auto &id) {
						const auto *selectedBox = Find_PresentationBox(*pattern, id);
						const auto *selectedResource =
							selectedBox ? Find_DraftPresentationResource(selectedBox->strResourceId) : nullptr;
						return selectedResource &&
							(selectedResource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
								selectedResource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::SOUND);
					});
			if (!keepPresentationSelection)
				Select_TimelineBox({}, box.strOccurrenceId, additiveSelection);
			m_strTimelineGroupDragOccurrenceId.clear();
			m_TimelineGroupDragOccurrenceIds.clear();
			if (additiveSelection)
				m_iTimelineDragMode = -1;
			else if (m_TimelineSelectedOccurrenceIds.size() > 1u)
			{
				std::string status;
				if (m_iTimelineDragMode == 0 &&
					Prepare_PresentationTimelineMove(
						m_Draft, *pattern, m_TimelineSelectedOccurrenceIds, 0, groupMove, status))
				{
					m_iTimelineDragMode = 3;
					m_strTimelineGroupDragOccurrenceId = box.strOccurrenceId;
					m_TimelineGroupDragOccurrenceIds = m_TimelineSelectedOccurrenceIds;
					m_iTimelineGroupDragGeneration = m_iDraftGeneration;
					groupMoveReady = true;
				}
				else
				{
					m_iTimelineDragMode = -1;
					m_strStatus = status.empty()
						? "Drag the middle of a grouped box to move every member; Ungroup before trimming."
						: status;
				}
			}
		}
		f32_t shownX = x, shownWidth = width;
		if (groupMoveReady &&
			std::find(groupMove.occurrenceIds.begin(), groupMove.occurrenceIds.end(), box.strOccurrenceId) !=
				groupMove.occurrenceIds.end())
		{
			shownX += static_cast<float>(groupMove.deltaMs) * scale;
			if (box.strOccurrenceId == m_strTimelineGroupDragOccurrenceId && ImGui::IsItemDeactivated())
				commitGroupMove = true;
		}
		if (canInteract && !m_bTimelineMarqueeActive && m_iTimelineDragMode >= 0 && m_iTimelineDragMode < 3 &&
			(ImGui::IsItemActive() || ImGui::IsItemDeactivated()))
		{
			int64_t start = m_iDragOriginOffsetMs, length = m_iDragOriginPlayMs;
			const auto delta = static_cast<int64_t>(std::llround(ImGui::GetMouseDragDelta().x / scale));
			const auto lifetime = static_cast<int64_t>(durationMs);
			auto mediaWindow = box;
			mediaWindow.iStartMs = static_cast<std::uint32_t>(start);
			mediaWindow.iDurationMs = static_cast<std::uint32_t>(length);
			const bool effect = resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT;
			const bool sound = resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::SOUND;
			if (effect)
				mediaWindow.iEffectSourceStartMs = m_iDragOriginSourceMs;
			else if (sound)
				mediaWindow.iSoundSourceStartMs = m_iDragOriginSourceMs;
			if ((sound || effect) && m_iTimelineDragMode != 0)
			{
				Trim_PresentationWindow(mediaWindow, delta, m_iTimelineDragMode == 1, durationMs,
					effect ? resource->iDurationMs : Resolve_SoundDurationMs(*resource), effect);
				start = mediaWindow.iStartMs;
				length = mediaWindow.iDurationMs;
			}
			else if (1 == m_iTimelineDragMode)
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
			if (ImGui::IsItemDeactivated() && (start != box.iStartMs || length != box.iDurationMs))
			{
				editPresentationBoxId = box.strOccurrenceId;
				editedPresentationBox = box;
				editedPresentationBox.iStartMs = static_cast<std::uint32_t>(start);
				editedPresentationBox.iDurationMs = static_cast<std::uint32_t>(length);
				if (effect)
				{
					editedPresentationBox.iEffectSourceStartMs = mediaWindow.iEffectSourceStartMs;
					editedPresentationBox.EffectSourceTimeKeys = mediaWindow.EffectSourceTimeKeys;
				}
				else if (sound)
					editedPresentationBox.iSoundSourceStartMs = mediaWindow.iSoundSourceStartMs;
				editedPresentationBox.iFadeInMs = (std::min)(box.iFadeInMs, editedPresentationBox.iDurationMs);
				editedPresentationBox.iFadeOutMs =
					(std::min)(box.iFadeOutMs, editedPresentationBox.iDurationMs - editedPresentationBox.iFadeInMs);
			}
		}
		const std::string &label = resource->strDisplayName;
		if (ImGui::IsItemHovered())
		{
			if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::SOUND)
				ImGui::SetTooltip("%s\n%s\nTimeline %u..%u ms | WAV %u..%u ms\nLeft/right edge: trim | Middle: move",
					box.strOccurrenceId.c_str(), label.c_str(), box.iStartMs, box.iStartMs + box.iDurationMs,
					box.iSoundSourceStartMs, box.iSoundSourceStartMs + box.iDurationMs);
			else if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
				ImGui::SetTooltip(
					"%s\n%s\nTimeline %u..%u ms | Effect source in %u ms\nLeft/right edge: trim | Middle: move%s%s",
					box.strOccurrenceId.c_str(), label.c_str(), box.iStartMs, box.iStartMs + box.iDurationMs,
					box.iEffectSourceStartMs,
					box.strSelectionGroupId.empty() ? "" : "\nGroup: ", box.strSelectionGroupId.c_str());
			else
				ImGui::SetTooltip("%s\n%s | start %u ms | lifetime %u ms%s%s", box.strOccurrenceId.c_str(),
					label.c_str(), box.iStartMs, box.iDurationMs,
					box.strSelectionGroupId.empty() ? "" : "\nGroup: ", box.strSelectionGroupId.c_str());
		}
		if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
			if (const auto *shot = Find_AuthoringCamera(resource->strAssetId); shot && shot->iBlendOutMs)
			{
				const float endX = shownX + shownWidth;
				draw->AddRectFilled(ImVec2(endX, presentationLaneY + 4.f),
					ImVec2(endX + float(shot->iBlendOutMs) * scale, presentationLaneY + boxHeight - 4.f),
					IM_COL32(94, 165, 151, 70));
				draw->AddText(ImVec2(endX + 3.f, presentationLaneY + 3.f), IM_COL32(154, 205, 191, 255), "return");
			}
		drawBox(ImVec2(shownX, presentationLaneY), ImVec2(shownX + shownWidth, presentationLaneY + boxHeight),
			CompositionTimeline::PresentationColor, contains(m_TimelineSelectedOccurrenceIds, box.strOccurrenceId),
			label.c_str());
		hitBoxes.push_back({{}, box.strOccurrenceId, ImVec2(shownX, presentationLaneY),
			ImVec2(shownX + shownWidth, presentationLaneY + boxHeight)});
		ImGui::PopID();
	}
	if (!canInteract)
	{
		m_bTimelineMarqueeActive = false;
		m_iTimelineDragMode = -1;
	}
	// 배경 입력은 box/ruler 뒤에 등록해 앞선 항목이 hit test를 먼저 받게 한다.
	// 빈 공간 클릭은 활성 item을 소유해 창 이동 대신 marquee 선택으로 처리한다.
	ImGui::SetCursorScreenPos(origin);
	const f32_t canvasHeight = (std::max)(height + 2.f * laneHeight, ImGui::GetWindowSize().y + ImGui::GetScrollY());
	ImGui::InvisibleButton("##KoukuMarquee", ImVec2(timelineWidth, canvasHeight));
	if (canInteract && ImGui::IsItemActivated())
	{
		if (!additiveSelection)
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
			m_strSelectedPatternOccurrenceId.clear();
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
		for (const auto &box : hitBoxes)
		{
			// 시간 구간 전체를 감싼 box만 선택한다. 짧은 공격 구간을 고를 때
			// 오래 유지되는 encounter Logic/World까지 함께 복사하지 않기 위한 조건이다.
			if (marqueeMin.x > box.min.x || marqueeMax.x < box.max.x || marqueeMax.y <= box.min.y ||
				marqueeMin.y >= box.max.y)
				continue;
			draw->AddRect(box.min, box.max, IM_COL32(255, 224, 92, 255), 3.f, 0, 2.f);
			if (released)
			{
				auto &selected =
					box.occurrenceId.empty() ? m_TimelineSelectedStageIds : m_TimelineSelectedOccurrenceIds;
				const auto &id = box.occurrenceId.empty() ? box.stageId : box.occurrenceId;
				if (!contains(selected, id))
					selected.push_back(id);
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
				// marquee로 모은 전체 선택은 유지하고 마지막 box의 종류에 맞춰 Detail focus만 맞춘다.
				auto selectedStages = m_TimelineSelectedStageIds;
				Expand_PresentationSelection(*pattern, m_TimelineSelectedOccurrenceIds);
				auto selectedOccurrences = m_TimelineSelectedOccurrenceIds;
				Select_TimelineBox(focusStage, focusOccurrence, false);
				m_TimelineSelectedStageIds = std::move(selectedStages);
				m_TimelineSelectedOccurrenceIds = std::move(selectedOccurrences);
			}
			else
				Synchronize_EditorFields();
		}
		else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
			m_bTimelineMarqueeActive = false;
	}
	if (canInteract && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !m_bTimelineMarqueeActive &&
		!ImGui::IsAnyItemActive() && (!m_TimelineSelectedStageIds.empty() || !m_TimelineSelectedOccurrenceIds.empty()))
	{
		if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
			deleteRequested = true;
		else if (!m_bSharedWorkspaceActive && ImGui::GetIO().KeyCtrl && !ImGui::GetIO().KeyAlt &&
			ImGui::IsKeyPressed(ImGuiKey_D, false))
			duplicateRequested = true;
		else if (!ImGui::GetIO().KeyCtrl && Can_ReorderTimelineSelection(pattern))
		{
			if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false))
				moveStageDirection = -1;
			else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, false))
				moveStageDirection = 1;
		}
	}
	ImGui::SetCursorScreenPos(origin);
	ImGui::Dummy(ImVec2(timelineWidth, height + 2.f * laneHeight));
	ImGui::EndChild();
	drawScope.reset();
	std::string status;
	if (deleteRequested)
	{
		// commit 뒤에는 이번 프레임에서 차용한 Pattern/Stage/occurrence 포인터를 다시 사용하지 않는다.
		(void)Delete_TimelineSelection(patternId, m_TimelineSelectedStageIds, m_TimelineSelectedOccurrenceIds, status);
	}
	else if (duplicateRequested)
		(void)Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION, status);
	else if (0 != moveStageDirection)
		(void)Move_SelectedStage(patternId, moveStageDirection, status);
	else if (!editPatternBoxId.empty())
		(void)Set_PatternBoxWindow(
			patternId, editPatternBoxId, newPatternStartMs, newPatternDurationMs, newPatternRepeat, status);
	else if (!editLogicBoxId.empty())
		(void)Set_LogicBoxWindow(patternId, editLogicBoxId, newLogicStartMs, newLogicDurationMs, status);
	else if (!editSummonBoxId.empty())
		(void)Set_SummonBoxWindow(patternId, editSummonBoxId, newSummonStartMs, newSummonDurationMs, status);
	else if (!editWorldBoxId.empty())
	{
		const auto *const edited = Find_WorldBox(*pattern, editWorldBoxId);
		(void)Set_WorldBoxWindow(patternId, editWorldBoxId, newWorldStartMs, newWorldDurationMs,
			nullptr == edited ? 1.f : edited->fPlaybackSpeed, status);
	}
	else if (!editSceneBoxId.empty())
	{
		const auto *const edited = Find_SceneProfileBox(*pattern, editSceneBoxId);
		(void)Set_SceneProfileBoxWindow(patternId, editSceneBoxId, newSceneStartMs, newSceneDurationMs,
			nullptr == edited ? 500u : edited->iBlendMs, status);
	}
	else if (commitGroupMove)
	{
		if (groupMove.deltaMs != 0)
			(void)Move_PresentationTimelineSelection(
				patternId, m_TimelineGroupDragOccurrenceIds, groupMove.deltaMs, m_iTimelineGroupDragGeneration, status);
		m_iTimelineDragMode = -1;
		m_strTimelineGroupDragOccurrenceId.clear();
		m_TimelineGroupDragOccurrenceIds.clear();
	}
	else if (!editPresentationBoxId.empty())
		(void)Set_PresentationBox(patternId, editedPresentationBox, status);
	else if (!editStageId.empty())
		(void)Set_StageDuration(m_strSelectedPatternId, editStageId, newStageDuration, status);
	else if (!editOccurrenceId.empty())
	{
		auto candidate = m_Draft;
		auto *editedPattern = Find_Pattern(candidate, m_strSelectedPatternId);
		const auto found = Find_Occurrence(*editedPattern, editOccurrenceId);
		found.pOccurrence->iStartOffsetMs = newOffset;
		found.pOccurrence->iSourceStartMs = newSourceStart;
		found.pOccurrence->iSourceEndMs = newSourceEnd;
		found.pOccurrence->iBlendInMs = (std::min)(found.pOccurrence->iBlendInMs, newPlayMs);
		found.pOccurrence->iPlayMs = newPlayMs;
		std::string policyNote;
		(void)Normalize_EndPolicyForWindow(*found.pOccurrence, policyNote);
		Mark_Draft(candidate, *editedPattern);
		(void)Commit_Candidate(std::move(candidate), "Updated animation box timing." + policyNote, status);
	}
}
