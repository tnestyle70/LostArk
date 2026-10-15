// 선택한 stable ID의 소유 관계를 닫아 값 스냅샷으로 복사한다. 붙여넣기/복제는 ID와 typed 참조를 함께 재작성한 후보만 commit한다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"

// 후보 값 복사, stable ID 조회, 시간/좌표 계산에 필요한 표준 타입이다.
#include <algorithm>
#include <array>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using namespace Client::KoukuWorkbenchDetail;

namespace
{
using namespace Client;

/* Ctrl+D와 Ctrl+C가 함께 사용하는 소유 관계의 확장이다. Hold 종료 시각, Logic이 지정한
	   Summon occurrence, 선택 그룹, 판정 Logic의 Collider 영역과 Object 동반 자원을 포함한다.
	   오래 유지되는 대상 카드와 검색 담당자는 공유한 채로 둔다. 함께 복사하면 같은 encounter를
	   한 번 더 시작하게 되므로 일반 timeline 자원과 구분한다. */

// 선택 stable ID 집합을 소유 관계가 더 늘지 않을 때까지 확장한다. 단순 위치 겹침이 아닌 typed 참조를 따라 복제에 필요한 행을 수집한다.
bool Expand_TimelineClosure(const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document,
	const KOUKU_SAYDON_COMPOSITION_PATTERN &source, std::unordered_set<std::string> &selected, std::string &outStatus)
{
	bool expanded = true;
	while (expanded)
	{
		expanded = false;
		for (const auto &box : source.LogicOccurrences)
		{
			if (!selected.contains(box.strOccurrenceId))
				continue;
			if (!box.strHoldLogicOccurrenceId.empty())
				expanded |= selected.insert(box.strHoldLogicOccurrenceId).second;
			if (const auto *definition = Find_Logic(document, box.strLogicId);
				definition && !definition->strSummonOccurrenceId.empty())
				expanded |= selected.insert(definition->strSummonOccurrenceId).second;
		}
		for (const auto &box : source.PresentationOccurrences)
		{
			if (selected.contains(box.strOccurrenceId) && !box.strSelectionGroupId.empty())
				for (const auto &member : source.PresentationOccurrences)
					if (member.strSelectionGroupId == box.strSelectionGroupId)
						expanded |= selected.insert(member.strOccurrenceId).second;
			const auto *resource = Find_PresentationResource(document, box.strResourceId);
			// Copy 후 리소스가 삭제된 미선택 행도 clipboard snapshot에는 남을 수 있다.
			// 이번에 선택한 행에 필요한 리소스만 확인한다.
			if (!resource)
			{
				if (selected.contains(box.strOccurrenceId))
				{
					outStatus = "Timeline resource is unavailable: " + box.strResourceId;
					return false;
				}
				continue;
			}
			if (selected.contains(box.strOccurrenceId))
			{
				if (!box.strLogicOccurrenceId.empty())
					expanded |= selected.insert(box.strLogicOccurrenceId).second;
				// WORLD anchor는 기존 gun/Object를 참조한다. 소유된 companion만 owner를 복사 의존성에 포함한다.
				if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT && box.strAnchorKind != "WORLD" &&
					!box.strWorldOccurrenceId.empty())
					expanded |= selected.insert(box.strWorldOccurrenceId).second;
			}
			// 판정 Logic은 Collider 영역 전체를 소유한다. 일부 영역만 복사하면
			// 새 정의가 이전 Logic에 연결되므로 전체 영역을 함께 수집한다.
			if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER && !box.strLogicOccurrenceId.empty() &&
				selected.contains(box.strLogicOccurrenceId))
				expanded |= selected.insert(box.strOccurrenceId).second;
			if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT && !box.strWorldOccurrenceId.empty() &&
				selected.contains(box.strWorldOccurrenceId))
				expanded |= selected.insert(box.strOccurrenceId).second;
		}
	}
	for (const auto &id : selected)
		if (!Has_TimelineOccurrence(source, id))
		{
			outStatus = "A selected box has a missing linked owner: " + id;
			return false;
		}
	return true;
}

// 복사 결과는 종류별 개수를 연결한 상태 문구이며 마지막 마침표는 붙이지 않는다.
std::string Format_ClipboardSummary(const char *verb, const std::array<std::size_t, 10u> &counts)
{
	constexpr std::array<std::pair<const char *, const char *>, 10u> labels = {
		{{"Stage", "Stages"}, {"Animation clip", "Animation clips"}, {"Pattern row", "Pattern rows"},
			{"Logic", "Logic"}, {"Summon", "Summons"}, {"World", "World"}, {"Scene Profile", "Scene Profiles"},
			{"Effect", "Effects"}, {"Collider", "Colliders"}, {"presentation box", "presentation boxes"}}};
	std::string text = verb;
	bool any = false;
	for (std::size_t kind = 0u; kind < counts.size(); ++kind)
	{
		if (!counts[kind])
			continue;
		if (any)
			text += ", ";
		text += std::to_string(counts[kind]) + " " + (counts[kind] == 1u ? labels[kind].first : labels[kind].second);
		any = true;
	}
	return any ? text : text + "nothing";
}
} // 이 파일의 보조 구현 범위 끝

// 현재 선택과 미저장 기하 overlay를 값으로 고정한다. 이후 원본 문서 교체와 독립적인 clipboard를 만들며 draft를 commit하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Copy_TimelineSelection(const std::string_view patternId,
	const std::vector<std::string> &stageIds, const std::vector<std::string> &occurrenceIds, std::string &outStatus)
{
	const auto reject = [&](const std::string &reason) {
		outStatus = m_strStatus = reason;
		return false;
	};
	const auto *pattern = m_bHasDraft ? Find_Pattern(m_Draft, patternId) : nullptr;
	if (!pattern || !pattern->strLoadError.empty())
		return reject("Copy needs a valid Pattern; previous clipboard preserved.");
	if (stageIds.empty() && occurrenceIds.empty())
		return reject("Select timeline boxes to copy.");
	std::unordered_set<std::string> selectedStages(stageIds.begin(), stageIds.end());
	std::unordered_set<std::string> selected(occurrenceIds.begin(), occurrenceIds.end());
	if (selectedStages.size() != stageIds.size() || selected.size() != occurrenceIds.size())
		return reject("Copy selection contains duplicate IDs; previous clipboard preserved.");
	for (const auto &id : selectedStages)
		if (!Find_Stage(*pattern, id))
			return reject("Copy Stage is unavailable: " + id);
	for (const auto &id : selected)
		if (!Has_TimelineOccurrence(*pattern, id))
			return reject("Copy box is unavailable: " + id);
	auto expanded = occurrenceIds;
	Expand_PresentationSelection(*pattern, expanded);
	selected.insert(expanded.begin(), expanded.end());
	// 선택 Effect에 부착된 Collider 영역과 판정 Logic도 함께 복사한다.
	// 의존성을 추가하기 전에 사용자/그룹 선택을 고정한다. Collider 하나의 birth Effect를
	// 포함했다는 이유로 같은 Effect를 쓰는 모든 Collider까지 복사하지 않게 한다.
	const auto requestedSelection = selected;
	for (const auto &row : pattern->PresentationOccurrences)
	{
		if (row.strAnchorPresentationOccurrenceId.empty() ||
			!requestedSelection.contains(row.strAnchorPresentationOccurrenceId))
			continue;
		const auto *resource = Find_PresentationResource(m_Draft, row.strResourceId);
		if (resource && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
			selected.insert(row.strOccurrenceId);
	}
	std::string status;
	// Ctrl+D와 같은 소유 관계를 clipboard에 포함한다. Paste가 Logic의 hold/영역/summon이나
	// Object의 companion을 서로 떼어 놓지 않게 한다.
	// 다른 Pattern으로 복사할 때는 목적지에 없는 World owner와 고정 birth Effect도
	// 함께 가져와야 한다.
	for (bool grown = true; grown;)
	{
		if (!Expand_TimelineClosure(m_Draft, *pattern, selected, status))
			return reject(status + "; previous clipboard preserved.");
		grown = false;
		for (const auto &row : pattern->PresentationOccurrences)
		{
			if (!selected.contains(row.strOccurrenceId))
				continue;
			if (!row.strWorldOccurrenceId.empty())
				grown |= selected.insert(row.strWorldOccurrenceId).second;
			if (!row.strAnchorPresentationOccurrenceId.empty())
				grown |= selected.insert(row.strAnchorPresentationOccurrenceId).second;
		}
	}
	TIMELINE_CLIPBOARD clipboard;
	std::uint64_t first = MAX_EDITOR_TIME_MS, last = 0u, base = 0u;
	const auto include = [&](const std::uint64_t start, const std::uint64_t duration) {
		first = (std::min)(first, start);
		last = (std::max)(last, start + duration);
	};
	std::unordered_set<std::string> animations;
	std::vector<std::pair<std::uint64_t, const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE *>> animationOrder;
	for (const auto &stage : pattern->Stages)
	{
		const bool whole = selectedStages.contains(stage.strStageId);
		if (whole)
			include(base, stage.iDurationMs);
		for (const auto &box : stage.AnimationOccurrences)
		{
			animationOrder.emplace_back(base + box.iStartOffsetMs, &box);
			if (whole || selected.contains(box.strOccurrenceId))
			{
				animations.insert(box.strOccurrenceId);
				include(base + box.iStartOffsetMs, box.iPlayMs);
			}
		}
		base += stage.iDurationMs;
	}
	if (!animations.empty())
	{
		// transition은 양쪽 clip을 모두 복사할 때만 포함한다. 한쪽만 복사하면
		// 목적지에 없는 clip을 blend가 참조하게 된다.
		std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> blends;
		if (!CKoukuSaydonCompositionDocument::Try_ResolveAnimationBlendWindows(m_Draft, *pattern, blends, status))
			return reject("Copy cannot resolve the original animation transition: " + status);
		for (const auto &window : blends)
		{
			const bool sourceSelected = animations.contains(window.Source.strOccurrenceId);
			const bool targetSelected = animations.contains(window.Target.strOccurrenceId);
			if (sourceSelected && targetSelected)
				selected.insert(window.strLogicOccurrenceId);
			else if (sourceSelected || targetSelected)
			{
				selected.erase(window.strLogicOccurrenceId);
				++clipboard.iDetachedBoundaryBlends;
			}
		}
	}
	std::sort(animationOrder.begin(), animationOrder.end(), [](const auto &a, const auto &b) {
		return a.first != b.first ? a.first < b.first : a.second->strOccurrenceId < b.second->strOccurrenceId;
	});
	for (std::size_t i = 0u; i < animationOrder.size(); ++i)
		if (animations.contains(animationOrder[i].second->strOccurrenceId) && animationOrder[i].second->iBlendInMs &&
			(!i || !animations.contains(animationOrder[i - 1u].second->strOccurrenceId) ||
				animationOrder[i - 1u].first + animationOrder[i - 1u].second->iPlayMs != animationOrder[i].first))
			++clipboard.iDetachedBoundaryBlends;
	const auto includeLane = [&](const auto &rows) {
		for (const auto &row : rows)
			if (selected.contains(row.strOccurrenceId))
				include(row.iStartMs, row.iDurationMs);
	};
	includeLane(pattern->LogicOccurrences);
	includeLane(pattern->SummonOccurrences);
	includeLane(pattern->WorldOccurrences);
	includeLane(pattern->SceneProfileOccurrences);
	includeLane(pattern->PresentationOccurrences);
	includeLane(pattern->PatternOccurrences);
	if (last <= first || last > MAX_EDITOR_TIME_MS)
		return reject("Copy needs a positive selection span within 600000 ms.");
	// 선택의 미저장 배치를 합친 값 스냅샷이다. 이후 원본 편집이나 삭제는
	// 이미 복사한 clipboard 내용에 전파되지 않는다.
	clipboard.Pattern = *pattern;
	for (const auto &pending : m_StagedPresentationGeometry)
	{
		if (pending.strPatternId != patternId || !selected.contains(pending.Occurrence.strOccurrenceId))
			continue;
		auto row = std::find_if(clipboard.Pattern.PresentationOccurrences.begin(),
			clipboard.Pattern.PresentationOccurrences.end(), [&](const auto &value) {
				return value.strOccurrenceId == pending.Occurrence.strOccurrenceId;
			});
		if (row == clipboard.Pattern.PresentationOccurrences.end())
			continue;
		const auto *resource = Find_PresentationResource(m_Draft, row->strResourceId);
		if (!resource)
			return reject("Copy resource is unavailable: " + row->strResourceId);
		Copy_PresentationPlacement(pending.Occurrence, *row, resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
		if (!Valid_PresentationPlacement(*resource, *row) || !Valid_GameplayColliderScale(*resource, *row))
			return reject("Correct the selected presentation placement before Copy; previous clipboard preserved.");
	}
	std::unordered_set<std::string> captured;
	std::string missing;
	const auto captureResource = [&](const std::string &id) {
		if (id.empty() || !captured.insert("resource:" + id).second)
			return true;
		const auto *resource = Find_PresentationResource(m_Draft, id);
		if (!resource)
		{
			missing = "Copy Effect resource definition is unavailable: " + id;
			return false;
		}
		clipboard.Resources.push_back(*resource);
		return true;
	};
	const auto captureScene = [&](const std::string &id) {
		if (id.empty() || !captured.insert("scene:" + id).second)
			return true;
		const auto *scene = Find_SceneProfile(m_Draft, id);
		if (!scene)
		{
			missing = "Copy Scene Profile definition is unavailable: " + id;
			return false;
		}
		clipboard.SceneProfiles.push_back(*scene);
		return true;
	};
	const auto captureWorld = [&](const std::string &id) {
		if (id.empty() || !captured.insert("world:" + id).second)
			return true;
		const auto *world = Find_World(m_Draft, id);
		if (!world)
		{
			missing = "Copy World definition is unavailable: " + id;
			return false;
		}
		clipboard.Worlds.push_back(*world);
		return captureResource(world->strCompanionEffectResourceId);
	};
	const auto captureSummon = [&](const std::string &id) {
		if (id.empty() || !captured.insert("summon:" + id).second)
			return true;
		const auto *summon = Find_Summon(m_Draft, id);
		if (!summon)
		{
			missing = "Copy Summon definition is unavailable: " + id;
			return false;
		}
		clipboard.Summons.push_back(*summon);
		return true;
	};
	const auto captureLogic = [&](const std::string &id) {
		if (id.empty() || !captured.insert("logic:" + id).second)
			return true;
		const auto *logic = Find_Logic(m_Draft, id);
		if (!logic)
		{
			missing = "Copy Logic definition is unavailable: " + id;
			return false;
		}
		clipboard.Logics.push_back(*logic);
		for (const auto &visual : logic->PursuitVisualIds)
			if (!captureResource(visual))
				return false;
		return captureResource(logic->strContactVisualId) && captureResource(logic->strEffectResourceId) &&
			captureResource(logic->strLightResourceId) && captureResource(logic->strSoundResourceId) &&
			captureScene(logic->strSceneProfileId);
	};
	auto &counts = clipboard.Counts;
	counts[0] = stageIds.size();
	counts[1] = animations.size();
	for (const auto &row : pattern->PatternOccurrences)
		if (selected.contains(row.strOccurrenceId))
			++counts[2];
	for (const auto &row : pattern->LogicOccurrences)
	{
		if (!selected.contains(row.strOccurrenceId))
			continue;
		++counts[3];
		if (!captureLogic(row.strLogicId))
			return reject(missing);
		for (const auto *outcomes : {&row.OnSuccessLogicIds, &row.OnFailLogicIds, &row.OnTimeoutLogicIds})
			for (const auto &id : *outcomes)
				if (!captureLogic(id))
					return reject(missing);
	}
	for (const auto &row : pattern->SummonOccurrences)
		if (selected.contains(row.strOccurrenceId))
		{
			++counts[4];
			if (!captureSummon(row.strSummonId))
				return reject(missing);
		}
	for (const auto &row : pattern->WorldOccurrences)
		if (selected.contains(row.strOccurrenceId))
		{
			++counts[5];
			if (!captureWorld(row.strWorldId))
				return reject(missing);
		}
	for (const auto &row : pattern->SceneProfileOccurrences)
		if (selected.contains(row.strOccurrenceId))
		{
			++counts[6];
			if (!captureScene(row.strSceneProfileId))
				return reject(missing);
		}
	for (const auto &row : clipboard.Pattern.PresentationOccurrences)
	{
		if (!selected.contains(row.strOccurrenceId))
			continue;
		if (!captureResource(row.strResourceId))
			return reject(missing);
		if (row.strAnchorKind == "WORLD" && !captureWorld(row.strWorldId))
			return reject(missing);
		const auto *resource = Find_PresentationResource(m_Draft, row.strResourceId);
		++counts[resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT	  ? 7u
				: resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER ? 8u
																			  : 9u];
	}
	clipboard.strCompositionId = m_Draft.strCompositionId;
	clipboard.strAreaId = m_Draft.strAreaId;
	clipboard.strActorProfileId = pattern->strActorProfileId;
	clipboard.strGateId = pattern->strGateId;
	clipboard.strTargetBossPlacementId = pattern->strTargetBossPlacementId;
	clipboard.iNextPresentationResourceOrdinal = m_Draft.iNextPresentationResourceOrdinal;
	clipboard.iNextWorldOrdinal = m_Draft.iNextWorldOrdinal;
	clipboard.iNextLogicOrdinal = m_Draft.iNextLogicOrdinal;
	clipboard.iNextSummonOrdinal = m_Draft.iNextSummonOrdinal;
	clipboard.iNextSceneProfileOrdinal = m_Draft.iNextSceneProfileOrdinal;
	clipboard.SelectedStageIds = stageIds;
	clipboard.SelectedOccurrenceIds.assign(selected.begin(), selected.end());
	std::sort(clipboard.SelectedOccurrenceIds.begin(), clipboard.SelectedOccurrenceIds.end());
	m_TimelineClipboard = std::move(clipboard);
	outStatus = m_strStatus = Format_ClipboardSummary("Copied ", m_TimelineClipboard->Counts) +
		". Select a destination Pattern and press Ctrl+V.";
	if (m_TimelineClipboard->iDetachedBoundaryBlends)
		outStatus = m_strStatus += " Transitions to unselected clips are excluded from the copy.";
	return true;
}

// 현재 목적지와 복사 시점 정의를 대조한다. 같은 ID의 정의가 변경됐으면 거절하고 성공 후보만 하나의 Undo 단위로 commit한다.
bool_t Client::CKoukuSaydonActionWorkbench::Paste_TimelineClipboard(
	const std::string_view patternId, std::string &outStatus)
{
	const auto reject = [&](const std::string &reason) {
		outStatus = m_strStatus = reason;
		return false;
	};
	if (!m_bHasDraft || !m_TimelineClipboard)
		return reject("Copy timeline boxes (Ctrl+C) before Paste.");
	const auto &clipboard = *m_TimelineClipboard;
	if (clipboard.strCompositionId != m_Draft.strCompositionId || clipboard.strAreaId != m_Draft.strAreaId)
		return reject("The clipboard belongs to a different Composition or Area; current draft preserved.");
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	if (!pattern || !pattern->strLoadError.empty())
		return reject("Paste needs a valid destination Pattern.");
	const std::string targetId = pattern->strPatternId;
	// 삭제된 공유 정의는 snapshot에서 복구한다. Copy 이후 변경된 정의는
	// Paste가 덮어쓰지 않도록 거절한다.
	std::size_t restored = 0u;
	const auto restore = [&](const auto &snapshots, auto &rows, std::uint32_t &ordinal,
							 const std::uint32_t copiedOrdinal, const char *kind, auto &&find, auto &&idOf) {
		for (const auto &snapshot : snapshots)
		{
			if (const auto *current = find(candidate, idOf(snapshot)))
			{
				if (*current != snapshot)
					return reject(std::string("Copied ") + kind + " definition changed; copy again: " + idOf(snapshot));
			}
			else
			{
				rows.push_back(snapshot);
				ordinal = (std::max)(ordinal, copiedOrdinal);
				++restored;
			}
		}
		return true;
	};
	if (!restore(
			clipboard.Resources, candidate.PresentationResources, candidate.iNextPresentationResourceOrdinal,
			clipboard.iNextPresentationResourceOrdinal, "Effect resource",
			[](const auto &document, const std::string &id) {
				return Find_PresentationResource(document, id);
			},
			[](const auto &row) -> const std::string & {
				return row.strResourceId;
			}) ||
		!restore(
			clipboard.Worlds, candidate.Worlds, candidate.iNextWorldOrdinal, clipboard.iNextWorldOrdinal, "World",
			[](const auto &document, const std::string &id) {
				return Find_World(document, id);
			},
			[](const auto &row) -> const std::string & {
				return row.strWorldId;
			}) ||
		!restore(
			clipboard.Logics, candidate.Logics, candidate.iNextLogicOrdinal, clipboard.iNextLogicOrdinal, "Logic",
			[](const auto &document, const std::string &id) {
				return Find_Logic(document, id);
			},
			[](const auto &row) -> const std::string & {
				return row.strLogicId;
			}) ||
		!restore(
			clipboard.Summons, candidate.Summons, candidate.iNextSummonOrdinal, clipboard.iNextSummonOrdinal, "Summon",
			[](const auto &document, const std::string &id) {
				return Find_Summon(document, id);
			},
			[](const auto &row) -> const std::string & {
				return row.strSummonId;
			}) ||
		!restore(
			clipboard.SceneProfiles, candidate.SceneProfiles, candidate.iNextSceneProfileOrdinal,
			clipboard.iNextSceneProfileOrdinal, "Scene Profile",
			[](const auto &document, const std::string &id) {
				return Find_SceneProfile(document, id);
			},
			[](const auto &row) -> const std::string & {
				return row.strSceneProfileId;
			}))
		return false;
	const std::unordered_set<std::string> stageSet(
		clipboard.SelectedStageIds.begin(), clipboard.SelectedStageIds.end());
	const std::unordered_set<std::string> boxSet(
		clipboard.SelectedOccurrenceIds.begin(), clipboard.SelectedOccurrenceIds.end());
	for (const auto &stage : clipboard.Pattern.Stages)
	{
		const bool whole = stageSet.contains(stage.strStageId);
		for (const auto &row : stage.AnimationOccurrences)
		{
			if (!whole && !boxSet.contains(row.strOccurrenceId))
				continue;
			if (CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(row.strProfileId) != pattern->strActorProfileId)
				return reject("Cannot paste Animation clips onto a different actor profile; current draft preserved.");
			if (!Validate_SourceStart(row, outStatus))
				return reject(outStatus);
		}
	}
	for (const auto &row : clipboard.Pattern.PresentationOccurrences)
		if (boxSet.contains(row.strOccurrenceId) && !row.strBone.empty() && row.strAnchorKind == "BOSS" &&
			clipboard.strActorProfileId != pattern->strActorProfileId)
			return reject("Cannot paste a bone-bound Effect onto a different actor profile.");
	const bool patternRows = std::any_of(
		clipboard.Pattern.PatternOccurrences.begin(), clipboard.Pattern.PatternOccurrences.end(), [&](const auto &row) {
			return boxSet.contains(row.strOccurrenceId);
		});
	if (patternRows)
	{
		if (!Find_TimelineParent(candidate, targetId))
			return reject("Pattern rows can only be pasted into a Parent timeline; current draft preserved.");
		for (const auto &row : clipboard.Pattern.PatternOccurrences)
			if (boxSet.contains(row.strOccurrenceId) && row.strPatternId == targetId)
				return reject("Cannot paste a Parent into itself.");
		if (clipboard.strGateId != pattern->strGateId || clipboard.strActorProfileId != pattern->strActorProfileId ||
			clipboard.strTargetBossPlacementId != pattern->strTargetBossPlacementId)
			return reject("Pattern rows need a destination with the same Gate, actor and target boss.");
	}
	TIMELINE_CLONE_RESULT result;
	if (!Clone_TimelineSelectionInto(candidate, clipboard.Pattern, *pattern, clipboard.SelectedStageIds,
			clipboard.SelectedOccurrenceIds, TIMELINE_CLONE_MODE::PASTE_APPEND, result, outStatus))
		return reject(outStatus);
	if (!Extend_PatternLifetimeForAuthoredLanes(*pattern, outStatus))
		return reject(outStatus);
	Mark_Draft(candidate, *pattern);
	std::string summary = Format_ClipboardSummary("Pasted ", clipboard.Counts) +
		(result.bPlacedFromZero ? " placed from 0 ms into the empty Pattern" : " at the Pattern end");
	if (restored)
		summary += " (restored " + std::to_string(restored) + " definitions)";
	summary += ". Existing clocks preserved. Press Save.";
	if (!Commit_Candidate(std::move(candidate), summary, outStatus))
		return false;
	Select_ClonedTimelineRows(targetId, std::move(result));
	return true;
}

// Paste/Duplicate가 함께 쓰는 후보 변환이다. source는 target과 별개 값 스냅샷이어야 하며
// candidate와 outResult는 도중에 바뀔 수 있다. false를 받은 호출자는 둘을 폐기하고 현재 draft를 유지해야 한다.
// 새 occurrence/group/region ID를 먼저 대응시킨 뒤 typed 참조가 달라지는 Logic 정의만 복제한다.
bool_t Client::CKoukuSaydonActionWorkbench::Clone_TimelineSelectionInto(KOUKU_SAYDON_COMPOSITION_DOCUMENT &candidate,
	const KOUKU_SAYDON_COMPOSITION_PATTERN &source, KOUKU_SAYDON_COMPOSITION_PATTERN &target,
	const std::vector<std::string> &stageIds, const std::vector<std::string> &occurrenceIds,
	const TIMELINE_CLONE_MODE mode, TIMELINE_CLONE_RESULT &outResult, std::string &outStatus)
{
	// DUPLICATE의 source는 편집 전 target의 값 사본이다.
	// PASTE_APPEND는 clipboard snapshot과 별도 목적지 Pattern을 사용한다.
	const bool paste = mode == TIMELINE_CLONE_MODE::PASTE_APPEND;
	const bool atOriginalTime = mode == TIMELINE_CLONE_MODE::DUPLICATE_SAME_TIME;
	const std::string verb = paste ? "Paste" : "Duplicate";
	const auto reject = [&](const std::string &message) {
		outStatus = message;
		return false;
	};
	auto *pattern = &target;
	const std::string targetId = target.strPatternId;
	std::unordered_set<std::string> selectedStages(stageIds.begin(), stageIds.end());
	auto expandedIds = occurrenceIds;
	Expand_PresentationSelection(source, expandedIds);
	std::unordered_set<std::string> selected(expandedIds.begin(), expandedIds.end());
	for (const auto &id : selectedStages)
		if (!Find_Stage(source, id))
			return reject(verb + " Stage is unavailable: " + id);
	for (const auto &id : selected)
		if (!Has_TimelineOccurrence(source, id))
			return reject(verb + " box is unavailable: " + id);
	if (!Expand_TimelineClosure(candidate, source, selected, outStatus))
		return false;
	if (atOriginalTime)
	{
		if (!selectedStages.empty() || expandedIds.empty())
			return reject("Same-time duplication requires Effect or Collider boxes.");
		std::optional<KOUKU_SAYDON_PRESENTATION_KIND> selectedKind;
		for (const auto &id : expandedIds)
		{
			const auto *box = Find_PresentationBox(source, id);
			const auto *resource = box ? Find_PresentationResource(candidate, box->strResourceId) : nullptr;
			if (!resource ||
				(resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT &&
					resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER) ||
				(selectedKind && *selectedKind != resource->eKind))
				return reject("Same-time duplication requires only Effect boxes or only Collider boxes.");
			selectedKind = resource->eKind;
		}
		for (const auto &id : selected)
		{
			// Collider의 판정 Logic과 hold는 기존 복사 의존성에 속한다.
			// 영역과 함께 복사하여 원본의 contact ledger를 공유하지 않게 한다.
			if (*selectedKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER && Find_LogicBox(source, id))
				continue;
			const auto *box = Find_PresentationBox(source, id);
			const auto *resource = box ? Find_PresentationResource(candidate, box->strResourceId) : nullptr;
			if (!resource || resource->eKind != *selectedKind)
				return reject(
					"Same-time duplication supports Effect boxes or Colliders with their owned Logic only. Use Ctrl+D for a full linked sequence.");
		}
	}
	bool lanesSelected = false, animationsSelected = false;
	for (const auto &id : selected)
		if (Find_Occurrence(source, id))
			animationsSelected = true;
		else
			lanesSelected = true;
	const bool splice = !selectedStages.empty() || animationsSelected;
	std::uint64_t first = MAX_EDITOR_TIME_MS, last = 0, total = 0, insertMs = 0;
	std::size_t insertIndex = 0;
	const auto include = [&](std::uint64_t start, std::uint64_t duration) {
		first = (std::min)(first, start);
		last = (std::max)(last, start + duration);
	};
	for (const auto &stage : source.Stages)
	{
		if (selectedStages.contains(stage.strStageId))
			include(total, stage.iDurationMs);
		for (const auto &box : stage.AnimationOccurrences)
			if (selected.contains(box.strOccurrenceId))
				include(total + box.iStartOffsetMs, box.iPlayMs);
		total += stage.iDurationMs;
	}
	const auto includeLane = [&](const auto &rows) {
		for (const auto &row : rows)
			if (selected.contains(row.strOccurrenceId))
				include(row.iStartMs, row.iDurationMs);
	};
	includeLane(source.LogicOccurrences);
	includeLane(source.SummonOccurrences);
	includeLane(source.WorldOccurrences);
	includeLane(source.SceneProfileOccurrences);
	includeLane(source.PresentationOccurrences);
	includeLane(source.PatternOccurrences);
	total = (std::max)(total, std::uint64_t(source.iDurationMs));
	if (last <= first || last > MAX_EDITOR_TIME_MS)
		return reject(verb + " requires a positive selection span within 600000 ms.");
	// 복제 후 참조가 원본 행으로 돌아가지 않도록 옛 ID→새 ID 대응을 따로 둔다. 실제 Stage 삽입 순서는 아래 vector가 보존한다.
	std::unordered_map<std::string, std::string> ids, regionIds;
	auto &newStageSelection = outResult.NewStageIds;
	auto &newBoxSelection = outResult.NewBoxIds;
	std::vector<KOUKU_SAYDON_COMPOSITION_STAGE> stages;
	bool exhausted = false;
	const auto allocate = [&](std::uint32_t &ordinal, const std::string &prefix) {
		if (!ordinal || ordinal >= 1000000u)
		{
			exhausted = true;
			return std::string{};
		}
		return prefix + std::to_string(ordinal++);
	};
	const auto cloneStage = [&](KOUKU_SAYDON_COMPOSITION_STAGE copy, const bool selectStage) {
		copy.strStageId = allocate(pattern->iNextStageOrdinal, "STAGE_");
		if (copy.strStageId.empty())
			return;
		copy.strActionId = targetId + ".stage." + copy.strStageId.substr(6u);
		if (selectStage)
			newStageSelection.push_back(copy.strStageId);
		for (auto &box : copy.AnimationOccurrences)
		{
			const auto old = box.strOccurrenceId;
			box.strOccurrenceId = allocate(pattern->iNextAnimationOrdinal, targetId + ".animation.");
			ids.emplace(old, box.strOccurrenceId);
			if (!selectStage || lanesSelected)
				newBoxSelection.push_back(box.strOccurrenceId);
		}
		stages.push_back(std::move(copy));
	};
	std::uint64_t base = 0, delta = 0;
	if (paste)
	{
		// 목적지의 기존 내용 뒤에 append한다. 행이 전혀 없는 placeholder Pattern은
		// 명시된 lifetime을 유지하면서 시작점을 0으로 잡아
		// 붙여넣은 내용 앞에 불필요한 빈 구간을 만들지 않는다.
		std::uint64_t stageEnd = 0u;
		for (const auto &stage : target.Stages)
			stageEnd += stage.iDurationMs;
		std::uint64_t appendAt = (std::max)(stageEnd, std::uint64_t(Pattern_DurationMs(target)));
		const auto includeEnds = [&](const auto &rows) {
			for (const auto &row : rows)
				appendAt = (std::max)(appendAt, std::uint64_t(row.iStartMs) + row.iDurationMs);
		};
		includeEnds(target.LogicOccurrences);
		includeEnds(target.SummonOccurrences);
		includeEnds(target.WorldOccurrences);
		includeEnds(target.SceneProfileOccurrences);
		includeEnds(target.PresentationOccurrences);
		includeEnds(target.PatternOccurrences);
		if (target.BossMotion)
			appendAt = (std::max)(appendAt, std::uint64_t(target.BossMotion->iEndMs));
		const bool emptyDestination = target.Stages.empty() && target.PatternOccurrences.empty() &&
			target.LogicOccurrences.empty() && target.SummonOccurrences.empty() && target.WorldOccurrences.empty() &&
			target.SceneProfileOccurrences.empty() && target.PresentationOccurrences.empty() && !target.BossMotion;
		if (emptyDestination && appendAt)
		{
			appendAt = 0u;
			outResult.bPlacedFromZero = true;
		}
		insertMs = appendAt;
		insertIndex = target.Stages.size();
		delta = last - first;
		if (appendAt + delta > MAX_EDITOR_TIME_MS)
			return reject("Paste would exceed the 600000 ms Pattern lifetime; current draft preserved.");
		if (splice && stageEnd < appendAt)
		{
			KOUKU_SAYDON_COMPOSITION_STAGE gap;
			gap.strStageKind = "RECOVERY";
			gap.iDurationMs = static_cast<std::uint32_t>(appendAt - stageEnd);
			cloneStage(std::move(gap), false);
		}
	}
	// 여러 lane 복제와 마찬가지로 Paste도 선택 clip 사이의 간격을 유지한다.
	const bool cropToSpan = lanesSelected || paste;
	for (std::size_t index = 0; splice && index < source.Stages.size(); ++index)
	{
		const auto &original = source.Stages[index];
		const auto end = base + original.iDurationMs;
		const bool whole = selectedStages.contains(original.strStageId);
		auto copy = original;
		if (!whole)
			std::erase_if(copy.AnimationOccurrences, [&](const auto &box) {
				return !selected.contains(box.strOccurrenceId);
			});
		if (!cropToSpan)
		{
			if (whole || !copy.AnimationOccurrences.empty())
			{
				if (!whole)
				{
					std::uint32_t a = MAX_EDITOR_TIME_MS, b = 0;
					for (const auto &box : copy.AnimationOccurrences)
					{
						a = (std::min)(a, box.iStartOffsetMs);
						b = (std::max)(b, box.iStartOffsetMs + box.iPlayMs);
					}
					for (auto &box : copy.AnimationOccurrences)
						box.iStartOffsetMs -= a;
					copy.iDurationMs = b - a;
				}
				delta += copy.iDurationMs;
				cloneStage(std::move(copy), whole);
				insertIndex = index + 1u;
				insertMs = end;
			}
		}
		else if (base < last && end > first)
		{
			const auto a = (std::max)(base, first), b = (std::min)(end, last);
			copy.iDurationMs = static_cast<std::uint32_t>(b - a);
			if (!whole && copy.AnimationOccurrences.empty())
			{
				copy.bRetargetOnEnter = false;
				copy.RetargetTarget.reset();
			}
			for (auto &box : copy.AnimationOccurrences)
				box.iStartOffsetMs -= static_cast<std::uint32_t>(a - base);
			cloneStage(std::move(copy), true);
			if (!paste)
			{
				insertIndex = index + 1u;
				insertMs = end;
			}
		}
		base = end;
	}
	if (cropToSpan && splice)
	{
		delta = last - first;
		if (last > total)
		{
			KOUKU_SAYDON_COMPOSITION_STAGE tail;
			tail.strStageKind = "RECOVERY";
			tail.iDurationMs = static_cast<std::uint32_t>(last - total);
			cloneStage(std::move(tail), true);
			if (!paste)
			{
				insertIndex = source.Stages.size();
				insertMs = total;
			}
		}
	}
	if (!paste && !splice)
	{
		insertMs = atOriginalTime ? first : last;
		delta = atOriginalTime ? 0u : last - first;
	}
	if (exhausted)
		return reject(verb + " rejected; stable ID ordinals are exhausted.");
	if (!paste && ((splice && total + delta > MAX_EDITOR_TIME_MS) || (!splice && last + delta > MAX_EDITOR_TIME_MS)))
		return reject("Duplicate would exceed the 600000 ms timeline limit.");
	if (paste)
	{
		// 목적지에서 pose 연결을 다시 구성한다. 함께 복사하지 않은 clip으로 향하는
		// blend는 없는 대상을 참조하지 않도록 제외한다.
		std::vector<std::pair<std::uint64_t, KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE *>> clips;
		std::uint64_t origin = 0u;
		for (auto &stage : stages)
		{
			for (auto &box : stage.AnimationOccurrences)
				clips.emplace_back(origin + box.iStartOffsetMs, &box);
			origin += stage.iDurationMs;
		}
		for (auto &[start, box] : clips)
		{
			box->iPoseStartMs = UINT32_MAX;
			if (!box->iBlendInMs)
				continue;
			const bool continued = std::any_of(clips.begin(), clips.end(), [&](const auto &other) {
				return other.second != box && other.first + other.second->iPlayMs == start;
			});
			if (!continued)
			{
				box->iBlendInMs = 0u;
				++outResult.iDetachedBoundaryBlends;
			}
		}
	}
	if (!paste && splice && pattern->BossMotion)
	{
		auto &motion = *pattern->BossMotion;
		if (motion.iStartMs >= insertMs)
		{
			motion.iStartMs += static_cast<std::uint32_t>(delta);
			motion.iEndMs += static_cast<std::uint32_t>(delta);
		}
		else if (motion.iEndMs > insertMs)
			motion.iEndMs += static_cast<std::uint32_t>(delta);
		if (motion.iEndMs > insertMs)
			for (auto &key : motion.Keys)
				if (key.iTimeMs >= insertMs)
					key.iTimeMs += static_cast<std::uint32_t>(delta);
	}
	const auto shiftLane = [&](auto &rows) {
		for (auto &row : rows)
		{
			const std::uint64_t end = std::uint64_t(row.iStartMs) + row.iDurationMs;
			if (row.iStartMs >= insertMs)
				row.iStartMs += static_cast<std::uint32_t>(delta);
			else if (end > insertMs)
				row.iDurationMs += static_cast<std::uint32_t>(delta);
			if (std::uint64_t(row.iStartMs) + row.iDurationMs > MAX_EDITOR_TIME_MS)
				return false;
		}
		return true;
	};
	if (!paste && splice &&
		(!shiftLane(pattern->LogicOccurrences) || !shiftLane(pattern->SummonOccurrences) ||
			!shiftLane(pattern->WorldOccurrences) || !shiftLane(pattern->SceneProfileOccurrences) ||
			!shiftLane(pattern->PresentationOccurrences) || !shiftLane(pattern->PatternOccurrences)))
		return reject("Duplicate would move an existing lane beyond 600000 ms.");
	// 새 행의 index는 목적지 기준이다. source와 같은 문서인 경우는 DUPLICATE뿐이다.
	const std::size_t firstNewPresentation = pattern->PresentationOccurrences.size();
	const std::size_t firstNewLogic = pattern->LogicOccurrences.size();
	const auto cloneLane = [&](const auto &originals, auto &rows, std::uint32_t &ordinal, const char *family) {
		for (const auto &original : originals)
			if (selected.contains(original.strOccurrenceId))
			{
				auto copy = original;
				copy.strOccurrenceId = allocate(ordinal, targetId + family);
				copy.iStartMs = static_cast<std::uint32_t>(insertMs + original.iStartMs - first);
				ids.emplace(original.strOccurrenceId, copy.strOccurrenceId);
				newBoxSelection.push_back(copy.strOccurrenceId);
				rows.push_back(std::move(copy));
			}
	};
	cloneLane(
		source.PatternOccurrences, pattern->PatternOccurrences, pattern->iNextPatternOccurrenceOrdinal, ".pattern.");
	cloneLane(source.LogicOccurrences, pattern->LogicOccurrences, pattern->iNextLogicOccurrenceOrdinal, ".logic.");
	cloneLane(source.SummonOccurrences, pattern->SummonOccurrences, pattern->iNextSummonOccurrenceOrdinal, ".summon.");
	cloneLane(source.WorldOccurrences, pattern->WorldOccurrences, pattern->iNextWorldOccurrenceOrdinal, ".world.");
	cloneLane(source.SceneProfileOccurrences, pattern->SceneProfileOccurrences,
		pattern->iNextSceneProfileOccurrenceOrdinal, ".sceneprofile.");
	cloneLane(source.PresentationOccurrences, pattern->PresentationOccurrences,
		pattern->iNextPresentationOccurrenceOrdinal, ".presentation.");
	if (exhausted)
		return reject(verb + " rejected; stable ID ordinals are exhausted.");
	const auto remap = [](std::string &id, const auto &mapping) {
		const auto found = mapping.find(id);
		if (found != mapping.end())
			id = found->second;
	};
	std::unordered_map<std::string, std::string> copiedGroupIds;
	std::unordered_set<std::string> usedGroupIds;
	for (const auto &row : pattern->PresentationOccurrences)
		if (!row.strSelectionGroupId.empty())
			usedGroupIds.insert(row.strSelectionGroupId);
	for (std::size_t index = firstNewPresentation; index < pattern->PresentationOccurrences.size(); ++index)
	{
		auto &row = pattern->PresentationOccurrences[index];
		if (!row.strSelectionGroupId.empty())
		{
			auto [group, inserted] = copiedGroupIds.try_emplace(row.strSelectionGroupId);
			if (inserted)
			{
				do
				{
					const auto *resource = Find_PresentationResource(candidate, row.strResourceId);
					group->second = allocate(pattern->iNextPresentationOccurrenceOrdinal,
						targetId +
							(resource && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT
									? ".effectgroup."
									: ".collidergroup."));
					if (exhausted)
						return reject(verb + " rejected; selection group IDs are exhausted.");
				} while (!usedGroupIds.insert(group->second).second);
			}
			row.strSelectionGroupId = group->second;
		}
		remap(row.strLogicOccurrenceId, ids);
		if (!row.strRegionId.empty())
		{
			const auto old = row.strRegionId;
			row.strRegionId = row.strOccurrenceId + ".region";
			regionIds.emplace(old, row.strRegionId);
		}
		// Copy 단계에서 pending 배치를 이미 clipboard snapshot에 합쳤다.
		if (!paste)
			for (const auto &pending : m_StagedPresentationGeometry)
				if (pending.strPatternId == targetId && ids.contains(pending.Occurrence.strOccurrenceId) &&
					ids.at(pending.Occurrence.strOccurrenceId) == row.strOccurrenceId)
				{
					const auto *resource = Find_PresentationResource(candidate, row.strResourceId);
					if (!resource || !Valid_PresentationPlacement(*resource, pending.Occurrence))
						return reject("Correct or revert the selected presentation placement before Duplicate.");
					Copy_PresentationPlacement(
						pending.Occurrence, row, resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
					if (resource && !Valid_GameplayColliderScale(*resource, row))
						return reject(
							"Collider scale must be finite and positive. Adjust Scale / size before Duplicate.");
				}
		remap(row.strWorldOccurrenceId, ids);
		remap(row.strAnchorPresentationOccurrenceId, ids);
	}
	// 재사용 Logic 정의의 typed 참조는 필요한 정의만 복제해 수정한다.
	// 공유 정의와 외부 card/master/motion 참조는 유지한다.
	std::unordered_map<std::string, std::string> logicIds;
	const auto cloneLogic = [&](std::string &id) {
		if (const auto existing = logicIds.find(id); existing != logicIds.end())
		{
			id = existing->second;
			return true;
		}
		const auto found = std::find_if(candidate.Logics.begin(), candidate.Logics.end(), [&](const auto &value) {
			return value.strLogicId == id;
		});
		if (found == candidate.Logics.end())
			return false;
		auto copy = *found;
		for (auto &target : copy.TargetWorldOccurrenceIds)
			remap(target, ids);
		for (auto &target : copy.PlayerEntryEffectOccurrenceIds)
			remap(target, ids);
		for (auto &target : copy.ContactMotions)
			remap(target.strTargetWorldOccurrenceId, ids);
		remap(copy.strSummonOccurrenceId, ids);
		remap(copy.strBombPresentationOccurrenceId, ids);
		remap(copy.strBombExplosionPresentationOccurrenceId, ids);
		remap(copy.strBombSectorPresentationOccurrenceId, ids);
		remap(copy.strTargetLogicOccurrenceId, ids);
		remap(copy.strContactTargetWorldOccurrenceId, ids);
		for (auto &region : copy.RegionIds)
			remap(region, regionIds);
		if (paste)
		{
			// 다른 Pattern으로 붙여넣을 때는 원본을 가리키던 행/그룹 참조를 새 ID로 바꾼다.
			// 같은 Pattern의 Ctrl+D에서는 해당 참조를 유지한다.
			remap(copy.strTrackingPresentationOccurrenceId, ids);
			for (auto &set : copy.RandomVolleyOccurrenceSets)
				for (auto &member : set)
					remap(member, ids);
			remap(copy.strFixedSelectionGroupId, copiedGroupIds);
			remap(copy.strSelectedEffectGroupId, copiedGroupIds);
		}
		if (copy == *found)
			return true;
		copy.strLogicId = allocate(candidate.iNextLogicOrdinal, "koukusaydon.g1.logic.");
		copy.strDisplayName += " (copy)";
		logicIds.emplace(id, copy.strLogicId);
		id = copy.strLogicId;
		candidate.Logics.push_back(std::move(copy));
		return !exhausted;
	};
	for (std::size_t index = firstNewLogic; index < pattern->LogicOccurrences.size(); ++index)
	{
		auto &row = pattern->LogicOccurrences[index];
		remap(row.strHoldLogicOccurrenceId, ids);
		if (!cloneLogic(row.strLogicId))
			return reject(verb + " Logic definition is unavailable or its IDs are exhausted.");
		for (auto *outcomes : {&row.OnSuccessLogicIds, &row.OnFailLogicIds, &row.OnTimeoutLogicIds})
			for (auto &id : *outcomes)
				if (!cloneLogic(id))
					return reject(verb + " outcome definition is unavailable or its IDs are exhausted.");
	}
	if (paste)
	{
		// 호출자가 Extend_PatternLifetimeForAuthoredLanes로 전체 lane의 lifetime을 반영한다.
		pattern->Stages.insert(
			pattern->Stages.end(), std::make_move_iterator(stages.begin()), std::make_move_iterator(stages.end()));
	}
	else if (splice)
	{
		pattern->Stages.insert(pattern->Stages.begin() + insertIndex, std::make_move_iterator(stages.begin()),
			std::make_move_iterator(stages.end()));
		if (pattern->iDurationMs)
			pattern->iDurationMs += static_cast<std::uint32_t>(delta);
	}
	else if (last + delta > total)
	{
		if (pattern->iDurationMs)
			pattern->iDurationMs = static_cast<std::uint32_t>(last + delta);
		else
		{
			if (pattern->Stages.empty())
				return reject("Lane duplication requires a Pattern Stage clock.");
			pattern->Stages.back().iDurationMs += static_cast<std::uint32_t>(last + delta - total);
		}
	}
	return true;
}

// commit된 대상 Pattern이 존재한다는 호출 계약 아래 새 stable ID들을 선택한다. 이전 후보의 포인터는
// 받지 않고 m_Draft에서 행을 다시 찾아 Detail과 다중 선택을 동기화한다. 이 함수 자체는 문서를 편집하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Select_ClonedTimelineRows(
	const std::string &targetId, TIMELINE_CLONE_RESULT result)
{
	Clear_TimelineSelection();
	m_strSelectedPatternId = targetId;
	m_strTimelineSelectionPatternId = targetId;
	m_TimelineSelectedStageIds = std::move(result.NewStageIds);
	m_TimelineSelectedOccurrenceIds = std::move(result.NewBoxIds);
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedPatternOccurrenceId.clear();
	if (!m_TimelineSelectedStageIds.empty())
		m_strSelectedStageId = m_TimelineSelectedStageIds.back();
	const auto *current = Find_Pattern(m_Draft, targetId);
	for (const auto &id : m_TimelineSelectedOccurrenceIds)
		if (Find_Occurrence(*current, id))
			m_strSelectedOccurrenceId = id;
		else if (Find_PresentationBox(*current, id))
			m_strSelectedPresentationOccurrenceId = id;
	Normalize_Selection();
	Synchronize_EditorFields();
	m_bFitRequested = false;
}

// 대상 Pattern을 source 값 사본으로 고정한 뒤 같은 후보에 복제한다. 일반 복제와 같은 시각 복제의
// 선택 정책을 구분하며 commit 뒤 ID로 다시 선택한다. 같은 시각 복제는 새 ID를 담은 preview snapshot도 요청한다.
bool_t Client::CKoukuSaydonActionWorkbench::Duplicate_TimelineSelection(const std::string_view patternId,
	const std::vector<std::string> &stageIds, const std::vector<std::string> &occurrenceIds, std::string &outStatus,
	const bool_t atOriginalTime)
{
	const auto reject = [&](const std::string &message) {
		outStatus = m_strStatus = message;
		return false;
	};
	if (stageIds.empty() && occurrenceIds.empty())
		return reject("Select timeline boxes to duplicate.");
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	if (!pattern || !pattern->strLoadError.empty())
		return reject("Duplicate target Pattern is unavailable or invalid.");
	const auto source = *pattern;
	const std::string targetId = source.strPatternId;
	TIMELINE_CLONE_RESULT result;
	if (!Clone_TimelineSelectionInto(candidate, source, *pattern, stageIds, occurrenceIds,
			atOriginalTime ? TIMELINE_CLONE_MODE::DUPLICATE_SAME_TIME : TIMELINE_CLONE_MODE::DUPLICATE_AFTER, result,
			outStatus))
		return reject(outStatus);
	const auto isCopiedCollider = [&](const std::string &id) {
		const auto *box = Find_PresentationBox(*pattern, id);
		const auto *resource = box ? Find_PresentationResource(candidate, box->strResourceId) : nullptr;
		return resource && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER;
	};
	const bool colliderAtOriginalTime =
		atOriginalTime && std::any_of(result.NewBoxIds.begin(), result.NewBoxIds.end(), isCopiedCollider);
	if (colliderAtOriginalTime)
		std::erase_if(result.NewBoxIds, [&](const auto &id) {
			return !isCopiedCollider(id);
		});
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			atOriginalTime
				? "Duplicated the selected boxes at the same times with their owned links. The copies are selected; adjust their placement and Save."
				: "Duplicated selected timeline boxes and their owned links. Following clocks preserved. Press Save.",
			outStatus))
		return false;
	Select_ClonedTimelineRows(targetId, std::move(result));
	// Collider detail 복제는 새 Collider 선택을 유지하며 복사한 Logic도 해당 행이 소유한다.
	// Ctrl+D와 Effect의 기존 선택 처리는 그대로 따른다.
	if (!colliderAtOriginalTime)
		m_strSelectedPresentationOccurrenceId.clear();
	Synchronize_EditorFields();
	if (atOriginalTime)
	{
		// 실행 중 preview는 문서 snapshot을 소유한다. 뒤이은 그룹 기하 편집이
		// 새 occurrence ID를 대상으로 삼기 전에 새 snapshot을 반영한다.
		const bool pending = m_bPatternPreviewRequestPending && m_PendingPatternPreview.strPatternId == targetId;
		const bool playing = m_PreviewState.bPlaying && m_PreviewState.strPatternId == targetId;
		const auto clockMs = pending		   ? m_iPendingPreviewStartMs
			: playing						   ? m_PreviewState.iClockMs
			: m_strCursorPatternId == targetId ? m_iCursorMs
											   : 0u;
		const bool paused = pending ? m_bPendingPreviewStartPaused : !playing || m_PreviewState.bPaused;
		const std::string copiedStatus = outStatus;
		std::string previewStatus;
		if (!Request_PatternPreview(targetId, clockMs, previewStatus, paused))
			outStatus = m_strStatus = copiedStatus + " Preview is unavailable: " + previewStatus;
		else
			outStatus = m_strStatus = copiedStatus;
	}
	return true;
}

// 선택 행과 전용 소유 관계를 같은 후보에서 제거한다. animation 시간 절단은 관련 lane과 이동 시계에 같은 변환을 적용한다.
// 선택 Stage의 절대 시간 구간을 제거한 뒤 남은 lane과 BossMotion에 같은 시간 함수를 적용한다.
// 미선택 행·이동 키의 양수 구간이 사라지면 후보 전체를 거절한다. geometry overlay와 선택 정리는 commit 성공 뒤에만 한다.
bool_t Client::CKoukuSaydonActionWorkbench::Delete_TimelineSelection(const std::string_view patternId,
	const std::vector<std::string> &stageIds, const std::vector<std::string> &occurrenceIds, std::string &outStatus)
{
	const auto reject = [&](const std::string &reason) {
		outStatus = m_strStatus = reason;
		return false;
	};
	if (stageIds.empty() && occurrenceIds.empty())
		return reject("Select timeline boxes to delete.");
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	if (!pattern || !pattern->strLoadError.empty())
		return reject("Delete target Pattern is unavailable or invalid.");
	const std::string targetId = pattern->strPatternId;
	std::unordered_set<std::string> stages(stageIds.begin(), stageIds.end()),
		boxes(occurrenceIds.begin(), occurrenceIds.end());
	for (const auto &id : stages)
		if (!Find_Stage(*pattern, id))
			return reject("Delete Stage is unavailable: " + id);
	for (const auto &id : boxes)
		if (!Has_TimelineOccurrence(*pattern, id))
			return reject("Delete box is unavailable: " + id);
	// 단일 행 삭제와 같이 Object를 삭제할 때 소유 companion도 제거한다.
	// 남는 Trigger가 Hold를 공유하지 않을 때만 capture Trigger와 함께 Hold를 제거한다.
	for (const auto &row : pattern->LogicOccurrences)
		if (boxes.contains(row.strOccurrenceId) && !row.strHoldLogicOccurrenceId.empty() &&
			std::none_of(pattern->LogicOccurrences.begin(), pattern->LogicOccurrences.end(), [&](const auto &other) {
				return !boxes.contains(other.strOccurrenceId) &&
					other.strHoldLogicOccurrenceId == row.strHoldLogicOccurrenceId;
			}))
			boxes.insert(row.strHoldLogicOccurrenceId);
	for (auto &row : pattern->LogicOccurrences)
		if (boxes.contains(row.strHoldLogicOccurrenceId))
			row.strHoldLogicOccurrenceId.clear();
	for (const auto &row : pattern->PresentationOccurrences)
		if (!row.strLogicOccurrenceId.empty() && boxes.contains(row.strLogicOccurrenceId))
			boxes.insert(row.strOccurrenceId);
	for (const auto &row : pattern->PresentationOccurrences)
		if (!row.strWorldOccurrenceId.empty() && boxes.contains(row.strWorldOccurrenceId))
			boxes.insert(row.strOccurrenceId);
	std::vector<std::pair<std::uint64_t, std::uint64_t>> removed;
	std::uint64_t base = 0;
	for (const auto &stage : pattern->Stages)
	{
		if (stages.contains(stage.strStageId))
			removed.emplace_back(base, base + stage.iDurationMs);
		base += stage.iDurationMs;
	}
	const auto mappedTime = [&](const std::uint64_t time) {
		std::uint64_t result = time;
		for (const auto &[first, last] : removed)
			if (time > first)
				result -= (std::min)(time, last) - first;
		return result;
	};
	const auto eraseLane = [&](auto &rows) {
		std::erase_if(rows, [&](const auto &row) {
			return boxes.contains(row.strOccurrenceId);
		});
		for (auto &row : rows)
		{
			const auto begin = mappedTime(row.iStartMs),
					   end = mappedTime(std::uint64_t(row.iStartMs) + row.iDurationMs);
			if (row.iDurationMs && end == begin)
				return false;
			row.iStartMs = static_cast<std::uint32_t>(begin);
			row.iDurationMs = static_cast<std::uint32_t>(end - begin);
		}
		return true;
	};
	if (!eraseLane(pattern->LogicOccurrences) || !eraseLane(pattern->SummonOccurrences) ||
		!eraseLane(pattern->WorldOccurrences) || !eraseLane(pattern->SceneProfileOccurrences) ||
		!eraseLane(pattern->PresentationOccurrences) || !eraseLane(pattern->PatternOccurrences))
		return reject("The removed Stage contains an unselected lane box. Select that box too before Delete.");
	if (pattern->BossMotion && !removed.empty())
	{
		auto &motion = *pattern->BossMotion;
		const auto startMs = mappedTime(motion.iStartMs), endMs = mappedTime(motion.iEndMs);
		if (endMs <= startMs)
			return reject(
				"This selection removes the whole Boss Motion interval. Disable Move boss during Pattern before deleting that complete interval.");
		// Animation 절단은 위치와 yaw 저작값을 유지하면서 이동 시계를 바꾼다.
		// 다른 lane에 적용한 것과 같은 시간 변환을 사용한다.
		motion.iStartMs = static_cast<std::uint32_t>(startMs);
		motion.iEndMs = static_cast<std::uint32_t>(endMs);
		for (auto &key : motion.Keys)
			key.iTimeMs = static_cast<std::uint32_t>(mappedTime(key.iTimeMs));
		for (std::size_t i = 1u; i < motion.Keys.size(); ++i)
			if (motion.Keys[i].iTimeMs <= motion.Keys[i - 1u].iTimeMs)
				return reject(
					"Delete collapses sampled Boss Motion keys. Keep their timeline interval or disable Move boss during Pattern first.");
	}
	if (pattern->iDurationMs && !removed.empty())
		pattern->iDurationMs = static_cast<std::uint32_t>(mappedTime(pattern->iDurationMs));
	std::erase_if(pattern->Stages, [&](const auto &stage) {
		return stages.contains(stage.strStageId);
	});
	for (auto &stage : pattern->Stages)
		std::erase_if(stage.AnimationOccurrences, [&](const auto &row) {
			return boxes.contains(row.strOccurrenceId);
		});
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Deleted selected timeline boxes. Press Save.", outStatus))
		return false;
	std::erase_if(m_StagedPresentationGeometry, [&](const auto &row) {
		return row.strPatternId == targetId && boxes.contains(row.Occurrence.strOccurrenceId);
	});
	Clear_TimelineSelection();
	m_bFitRequested = false;
	return true;
}

// 다중 선택과 진행 중인 marquee/group drag 상태를 함께 해제한다. Detail의 단일 선택 ID나 문서 행은
// 여기서 지우지 않으므로 호출자가 목적에 맞게 별도로 동기화한다.
void Client::CKoukuSaydonActionWorkbench::Clear_TimelineSelection()
{
	m_strTimelineSelectionPatternId.clear();
	m_TimelineSelectedStageIds.clear();
	m_TimelineSelectedOccurrenceIds.clear();
	m_bTimelineMarqueeActive = false;
	m_iTimelineDragMode = -1;
	m_strTimelineGroupDragOccurrenceId.clear();
	m_TimelineGroupDragOccurrenceIds.clear();
}

// 화면에서 클릭한 Stage/occurrence ID가 현재 Pattern에 있는지 확인하고 그룹 선택을 확장한다.
// Ctrl 토글 후 남은 ID를 Detail 종류에 맞게 연결하며, 선택만 바꾸고 저작 문서는 수정하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Select_TimelineBox(
	const std::string &stageId, const std::string &occurrenceId, const bool_t toggle)
{
	// Sequencer의 전체 lane 선택은 stable occurrence ID로 보관하고 Detail 종류를 별도로 지정한다.
	const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (!pattern)
		return;
	const auto isOccurrence = [&](const std::string &id) {
		return Find_PatternBox(*pattern, id) || Find_Occurrence(*pattern, id) || Find_LogicBox(*pattern, id) ||
			Find_SummonBox(*pattern, id) || Find_WorldBox(*pattern, id) || Find_SceneProfileBox(*pattern, id) ||
			Find_PresentationBox(*pattern, id);
	};
	if (occurrenceId.empty() ? !Find_Stage(*pattern, stageId) : !isOccurrence(occurrenceId))
		return;
	const std::string selectedId = occurrenceId.empty() ? stageId : occurrenceId;
	if (!toggle)
	{
		m_TimelineSelectedStageIds.clear();
		m_TimelineSelectedOccurrenceIds.clear();
	}
	auto &selected = occurrenceId.empty() ? m_TimelineSelectedStageIds : m_TimelineSelectedOccurrenceIds;
	std::vector<std::string> clickedIds{selectedId};
	if (!occurrenceId.empty())
		Expand_PresentationSelection(*pattern, clickedIds);
	const bool removed = toggle && std::all_of(clickedIds.begin(), clickedIds.end(), [&](const auto &id) {
		return std::find(selected.begin(), selected.end(), id) != selected.end();
	});
	for (const auto &id : clickedIds)
	{
		const auto found = std::find(selected.begin(), selected.end(), id);
		if (removed && found != selected.end())
			selected.erase(found);
		else if (!removed && found == selected.end())
			selected.push_back(id);
	}
	std::string focusStage = occurrenceId.empty() ? selectedId : std::string{};
	std::string focusOccurrence = occurrenceId.empty() ? std::string{} : selectedId;
	if (removed)
	{
		focusStage.clear();
		focusOccurrence.clear();
		if (!m_TimelineSelectedOccurrenceIds.empty())
			focusOccurrence = m_TimelineSelectedOccurrenceIds.back();
		else if (!m_TimelineSelectedStageIds.empty())
			focusStage = m_TimelineSelectedStageIds.back();
	}
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	m_strSelectedPatternOccurrenceId.clear();
	const KOUKU_SAYDON_COMPOSITION_STAGE *animationStage = nullptr;
	if (!focusOccurrence.empty())
	{
		if (Find_Occurrence(*pattern, focusOccurrence, &animationStage))
		{
			m_strSelectedStageId = animationStage->strStageId;
			m_strSelectedOccurrenceId = focusOccurrence;
		}
		else if (Find_PatternBox(*pattern, focusOccurrence))
			m_strSelectedPatternOccurrenceId = focusOccurrence;
		else if (Find_LogicBox(*pattern, focusOccurrence))
			m_strSelectedLogicOccurrenceId = focusOccurrence;
		else if (Find_SummonBox(*pattern, focusOccurrence))
			m_strSelectedSummonOccurrenceId = focusOccurrence;
		else if (Find_WorldBox(*pattern, focusOccurrence))
			m_strSelectedWorldOccurrenceId = focusOccurrence;
		else if (Find_SceneProfileBox(*pattern, focusOccurrence))
			m_strSelectedSceneProfileOccurrenceId = focusOccurrence;
		else if (Find_PresentationBox(*pattern, focusOccurrence))
			m_strSelectedPresentationOccurrenceId = focusOccurrence;
	}
	else
		m_strSelectedStageId = focusStage;
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
}

// 공용 편집 명령을 Kouku의 값 clipboard·붙여넣기·복제에 연결한다. native 전송은 소유 관계를 모두
// 보존하고, 범용 Effect 전송은 표현 가능한 정책만 있는 선택에서 추가로 만든다. 전송 객체가 값 데이터를 소유한다.
bool Client::CKoukuSaydonActionWorkbench::Execute_CompositionEdit(
	const COMPOSITION_EDIT_COMMAND command, std::string &status)
{
	if (m_CompositionResourcesFocused && command != COMPOSITION_EDIT_COMMAND::PASTE)
	{
		status = m_strStatus =
			"Drag the selected catalog resource into the Sequencer; Copy and Duplicate here require a timeline selection.";
		return false;
	}
	const bool result = Dispatch_CompositionEdit(
		command,
		[&](std::string &message) -> COMPOSITION_TRANSFER {
			if (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN ||
				m_strTimelineSelectionPatternId != m_strSelectedPatternId)
			{
				message = "Select timeline boxes in the current Pattern before Copy.";
				return {};
			}
			if (!Copy_TimelineSelection(
					m_strSelectedPatternId, m_TimelineSelectedStageIds, m_TimelineSelectedOccurrenceIds, message))
				return {};
			auto transfer = std::make_shared<COMPOSITION_NATIVE_TRANSFER>();
			transfer->clipboard = *m_TimelineClipboard;
			transfer->label = transfer->clipboard.Pattern.strDisplayName;
			// 고유 복사 형식에는 모든 소유 관계를 보존한다. 모든 선택 박스의 정책을
			// 표현할 수 있을 때만 범용 Effect 복사 형식도 제공한다.
			bool portable =
				transfer->clipboard.SelectedStageIds.empty() && !transfer->clipboard.SelectedOccurrenceIds.empty();
			std::uint32_t origin = MAX_EDITOR_TIME_MS;
			for (const auto &id : transfer->clipboard.SelectedOccurrenceIds)
			{
				const auto *row = Find_PresentationBox(transfer->clipboard.Pattern, id);
				const auto resource = row
					? std::find_if(transfer->clipboard.Resources.begin(), transfer->clipboard.Resources.end(),
						  [&](const auto &item) {
							  return item.strResourceId == row->strResourceId;
						  })
					: transfer->clipboard.Resources.end();
				if (!row || resource == transfer->clipboard.Resources.end() ||
					resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
					(resource->strResourceKind != "V1_EFFECT" && resource->strResourceKind != "LEAF" &&
						resource->strResourceKind != "GROUP") ||
					!row->strBone.empty() || (row->strAnchorKind != "BOSS" && row->strAnchorKind != "MAP") ||
					row->bFollowBoss != (row->strAnchorKind == "BOSS") || row->iFadeInMs || row->iFadeOutMs ||
					row->iEffectSourceStartMs || row->fBrightnessMultiplier != 1.0 ||
					!row->strLogicOccurrenceId.empty() || !row->strWorldOccurrenceId.empty() ||
					!row->strWorldId.empty() || !row->strAnchorPresentationOccurrenceId.empty() ||
					!row->strSelectionGroupId.empty())
				{
					portable = false;
					break;
				}
				COMPOSITION_EFFECT_ITEM item;
				item.resourceKind = resource->strResourceKind;
				item.resourceId = resource->strAssetId;
				item.displayName = resource->strDisplayName;
				item.startMs = row->iStartMs;
				item.durationMs = row->iDurationMs;
				for (std::size_t axis = 0u; axis < 3u; ++axis)
				{
					item.position[axis] = static_cast<float>(row->PositionOffset[axis]);
					item.rotationDegrees[axis] = static_cast<float>(row->RotationDegrees[axis]);
					item.scale[axis] = static_cast<float>(row->Scale[axis]);
				}
				item.followOwner = item.inheritOwnerRotation = row->strAnchorKind == "BOSS";
				item.fitToDuration = row->bFitEffectToDuration;
				item.loopToDuration = row->bLoopEffectToDuration;
				origin = (std::min)(origin, item.startMs);
				transfer->items.push_back(std::move(item));
			}
			if (!portable)
				transfer->items.clear();
			else
				for (auto &item : transfer->items)
					item.startMs -= origin;
			return transfer;
		},
		[&](const COMPOSITION_TRANSFER &value, std::string &message) {
			return Insert_CompositionTransfer(value, message);
		},
		[&](std::string &message) {
			if (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN ||
				m_strTimelineSelectionPatternId != m_strSelectedPatternId)
			{
				message = "Select timeline boxes in the current Pattern before Duplicate.";
				return false;
			}
			return Duplicate_TimelineSelection(
				m_strSelectedPatternId, m_TimelineSelectedStageIds, m_TimelineSelectedOccurrenceIds, message);
		},
		status);
	m_strStatus = status;
	return result;
}

// 전송의 typed 종류에 따라 기존 Animation/Native/Effect 입력 경계로 보낸다. native clipboard는
// Paste 호출 동안만 바꾼 뒤 원래 값을 복원한다. 범용 Effect는 cursor 기준 ms·정책을 후보에서 모두 검증한 뒤 한 번 commit한다.
bool Client::CKoukuSaydonActionWorkbench::Insert_CompositionTransfer(
	const COMPOSITION_TRANSFER &transfer, std::string &status)
{
	const auto reject = [&](const std::string &reason) {
		status = m_strStatus = reason;
		return false;
	};
	if (const auto animation = std::dynamic_pointer_cast<const COMPOSITION_ANIMATION_TRANSFER>(transfer))
		return Append_CompositionAnimationResource(animation->resource, false, status);
	if (const auto native = std::dynamic_pointer_cast<const COMPOSITION_NATIVE_TRANSFER>(transfer))
	{
		const auto previous = m_TimelineClipboard;
		m_TimelineClipboard = native->clipboard;
		const bool result = Paste_TimelineClipboard(m_strSelectedPatternId, status);
		m_TimelineClipboard = previous;
		return result;
	}
	const auto effects = std::dynamic_pointer_cast<const COMPOSITION_EFFECT_TRANSFER>(transfer);
	if (!effects || effects->items.empty())
		return reject("This selection contains owner-specific lanes or Effect policies that Kouku cannot import.");
	if (!m_bHasDraft || Is_PublishRunning() || m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN)
		return reject("Select an editable Kouku Pattern before inserting Effects.");
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, m_strSelectedPatternId);
	if (!pattern || !pattern->strLoadError.empty())
		return reject("The destination Pattern is unavailable.");
	const std::string patternId = pattern->strPatternId;
	std::vector<std::string> inserted;
	for (const auto &item : effects->items)
	{
		if (!item.followOwner)
			return reject(
				"Cross-owner Effect paste cannot distinguish absolute World placement from an owner snapshot; use a following root Effect.");
		if (!item.bone.empty() || item.followOwner != item.inheritOwnerRotation || !item.durationMs ||
			(item.resourceKind != "V1_EFFECT" && item.resourceKind != "LEAF" && item.resourceKind != "GROUP"))
			return reject(
				"Kouku requires complete V1/V2 Effects, an explicit duration and a supported root rotation policy; bone transfers are not compatible.");
		if (item.startMs > MAX_EDITOR_TIME_MS || m_iCursorMs > MAX_EDITOR_TIME_MS - item.startMs ||
			item.durationMs > MAX_EDITOR_TIME_MS - (m_iCursorMs + item.startMs))
			return reject("Transferred Effect timing exceeds the Pattern range.");
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE source;
		source.strAssetId = item.resourceId;
		source.strDisplayName = item.displayName.empty() ? item.resourceId : item.displayName;
		source.strResourceKind = item.resourceKind;
		source.iDurationMs = item.durationMs;
		source.strDefaultAnchorKind = item.followOwner ? "BOSS" : "MAP";
		std::string resourceId;
		if (!Stage_PresentationSource(candidate, source, resourceId, status))
			return reject(status);
		if (pattern->iNextPresentationOccurrenceOrdinal >= 1000000u)
			return reject("Presentation occurrence IDs are exhausted.");
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE row;
		row.strOccurrenceId =
			patternId + ".presentation." + std::to_string(pattern->iNextPresentationOccurrenceOrdinal++);
		row.strResourceId = resourceId;
		row.iStartMs = m_iCursorMs + item.startMs;
		row.iDurationMs = item.durationMs;
		row.strAnchorKind = item.followOwner ? "BOSS" : "MAP";
		row.bFollowBoss = item.followOwner;
		row.bFitEffectToDuration = item.fitToDuration;
		row.bLoopEffectToDuration = item.loopToDuration;
		for (std::size_t axis = 0u; axis < 3u; ++axis)
		{
			row.PositionOffset[axis] = item.position[axis];
			row.RotationDegrees[axis] = item.rotationDegrees[axis];
			row.Scale[axis] = item.scale[axis];
		}
		inserted.push_back(row.strOccurrenceId);
		pattern->PresentationOccurrences.push_back(std::move(row));
	}
	if (!Extend_PatternLifetimeForAuthoredLanes(*pattern, status))
		return reject(status);
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Inserted Effect occurrences atomically. Press Save.", status))
		return false;
	Clear_TimelineSelection();
	m_strTimelineSelectionPatternId = patternId;
	m_TimelineSelectedOccurrenceIds = std::move(inserted);
	m_strSelectedPresentationOccurrenceId = m_TimelineSelectedOccurrenceIds.back();
	m_PresentationBoxEdit =
		*Find_PresentationBox(*Find_Pattern(m_Draft, patternId), m_strSelectedPresentationOccurrenceId);
	return true;
}

// Timeline에 이번 프레임 focus가 있고 텍스트 입력·드래그·팝업·마우스 조작이 없을 때만 Ctrl+C/V를
// 처리한다. 처리 프레임을 저장해 같은 프레임의 여러 pane 호출이 동일 단축키를 중복 소비하지 않게 한다.
void Client::CKoukuSaydonActionWorkbench::Process_TimelineClipboardShortcuts()
{
	const auto &io = ImGui::GetIO();
	if (!m_bTimelineClipboardFocusThisFrame || !m_bHasDraft ||
		m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN || !io.KeyCtrl || io.KeyAlt || io.KeyShift ||
		io.KeySuper || io.WantTextInput || ImGui::IsAnyItemActive() || m_bTimelineMarqueeActive ||
		ImGui::GetDragDropPayload() != nullptr ||
		ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel) ||
		m_iTimelineClipboardShortcutFrame == ImGui::GetFrameCount())
		return;
	for (int button = 0; button < ImGuiMouseButton_COUNT; ++button)
		if (ImGui::IsMouseDown(button))
			return;
	const bool copy = ImGui::IsKeyPressed(ImGuiKey_C, false);
	const bool paste = ImGui::IsKeyPressed(ImGuiKey_V, false);
	if (!copy && !paste)
		return;
	m_iTimelineClipboardShortcutFrame = ImGui::GetFrameCount();
	std::string status;
	(void)Execute_CompositionEdit(copy ? COMPOSITION_EDIT_COMMAND::COPY : COMPOSITION_EDIT_COMMAND::PASTE, status);
}
