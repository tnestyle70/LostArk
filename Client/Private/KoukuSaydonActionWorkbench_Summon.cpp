// Summon 정의와 배치 박스를 편집한다. 행·대형 입력은 로컬 후보에 모으고 검증 후 적용한다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"

// 보존 JSON의 참조를 검사하고, 장면 캐릭터의 현재 위치를 대형 중심 입력으로 차용한다.
#include "DataJson.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "Transform.h"

// 배치 후보의 값 복사와 XZ 평면의 대형 회전에 필요한 표준 타입·수학 함수다.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace Client::KoukuWorkbenchDetail;

namespace
{
using namespace Client;

// Summon 삭제와 리소스 목록이 사용하는 참조 집계다. 정상 Pattern은 typed occurrence를, 격리 Pattern은 보존 원본을 읽는다.
// 원본 형식을 읽을 수 없으면 outUnresolved로 알리며, 참조가 없다는 추정으로 정의를 지우지 않는다.
std::size_t Count_SummonReferences(const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view summonId,
	bool_t *const outUnresolved = nullptr)
{
	std::size_t count = 0u;
	bool_t unresolved = false;
	for (const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern : document.Patterns)
	{
		if (pattern.strLoadError.empty())
		{
			for (const auto &box : pattern.SummonOccurrences)
				if (box.strSummonId == summonId)
					++count;
			continue;
		}
		DATA_JSON_VALUE preserved;
		std::string error;
		if (!CDataJson::Parse(pattern.strPreservedJson, preserved, error) || !preserved.Is_Object())
		{
			unresolved = true;
			continue;
		}
		const auto *boxes = preserved.Find("summonOccurrences");
		if (nullptr == boxes)
			continue;
		if (!boxes->Is_Array())
		{
			unresolved = true;
			continue;
		}
		for (const auto &box : boxes->Get_Array())
		{
			const auto *reference = box.Find("summonId");
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
} // 이 파일의 보조 구현 범위 끝

// Summon 리소스의 Create가 새 재사용 정의를 만든다. 후보 문서에서 ordinal과 stable ID를 배정하고 검증·commit 뒤에만 선택과 출력 ID를 바꾼다.
// 이 함수는 이름 있는 정의를 추가하며 어느 Pattern에 몇 actor를 놓을지는 별도 occurrence와 재생 정책이 소유한다.
bool_t Client::CKoukuSaydonActionWorkbench::Create_Summon(
	const std::string_view displayName, std::string &outSummonId, std::string &outStatus)
{
	if (!m_bHasDraft || m_Draft.iNextSummonOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Summon creation requires a loaded draft and an available ordinal.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION summon;
	summon.strSummonId = "koukusaydon.g1.summon." + std::to_string(candidate.iNextSummonOrdinal++);
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

// Summon 리소스의 Delete가 호출한다. 정상 문서와 격리 원본까지 참조를 조사하고 참조가 없다고 확인한 정의만 후보에서 제거한다.
// 참조 미해결이나 commit 실패는 현재 정의와 선택을 보존한다.
bool_t Client::CKoukuSaydonActionWorkbench::Delete_Summon(const std::string_view summonId, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	const auto found = std::find_if(candidate.Summons.begin(), candidate.Summons.end(),
		[summonId](const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION &summon) {
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
		outStatus = m_strStatus =
			"Delete its " + std::to_string(references) + " Summon box(es) first; a referenced Summon is preserved.";
		return false;
	}
	candidate.Summons.erase(found);
	if (!Commit_Candidate(std::move(candidate), "Deleted Summon.", outStatus))
		return false;
	if (m_strSelectedSummonId == summonId)
		m_strSelectedSummonId.clear();
	return true;
}

// 리소스 Append가 정의 ID를 선택 Pattern의 새 발생 시각·수명과 연결한다. ms 입력을 기존 Pattern 수명 안으로 제한해 occurrence를 만든다.
// 검증·commit 성공 뒤에만 새 Summon box로 선택을 전환하며 이 저작 함수 자체가 actor를 생성하지는 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_SummonBox(const std::string_view patternId,
	const std::string_view summonId, const std::uint32_t startMs, const std::uint32_t durationMs,
	std::string &outOccurrenceId, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
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
		outStatus = m_strStatus = "Append Summon requires a Pattern lifetime. Append an animation action first.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE box;
	box.strOccurrenceId = pattern->strPatternId + ".summon." + std::to_string(pattern->iNextSummonOccurrenceOrdinal++);
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
	m_strSelectedPatternOccurrenceId.clear();
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
	outOccurrenceId = occurrenceId;
	return true;
}

// 타임라인 이동·trim과 상세 시간창 편집이 공유한다. 끝 시각을 64-bit로 계산해 양수 수명과 Pattern 내부 범위를 확인한다.
// 같은 시간은 no-op이고 유효한 후보만 commit한다. 연결된 child 재생 정책의 의미 검증은 공통 문서 validator가 맡는다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_SummonBoxWindow(const std::string_view patternId,
	const std::string_view occurrenceId, const std::uint32_t startMs, const std::uint32_t durationMs,
	std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE *const box =
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
		outStatus = m_strStatus =
			"Summon box window must stay inside the Pattern lifetime of " + std::to_string(lifetimeMs) + " ms.";
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

// 상세 Delete가 선택 occurrence를 후보에서 제거한다. 나머지 참조를 포함한 문서 검증을 통과해야 교체되며 실패하면 원래 box를 보존한다.
// 성공한 뒤 삭제 대상이 현재 선택일 때만 선택을 비우고 상세 입력을 다시 동기화한다.
bool_t Client::CKoukuSaydonActionWorkbench::Delete_SummonBox(
	const std::string_view patternId, const std::string_view occurrenceId, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = m_strStatus = "Summon box delete Pattern is absent.";
		return false;
	}
	const auto found = std::find_if(pattern->SummonOccurrences.begin(), pattern->SummonOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE &box) {
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

// 리소스와 occurrence 상세가 공유하는 재생 정책 화면이다. stable Summon ID마다 원본과 미적용 draft 한 쌍을 저장한다.
// 원본 정의가 바뀌었을 때만 draft를 다시 맞추며 일반 프레임에는 작성 중 입력을 유지한다. true면 source 선택 또는 commit 시도로 호출자가 그리기를 멈춘다.
bool_t Client::CKoukuSaydonActionWorkbench::Render_SummonPolicy(
	const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION &summon, const KOUKU_SAYDON_COMPOSITION_PATTERN *parent)
{
	// source는 마지막으로 본 공유 정의, draft는 아직 Apply하지 않은 정책이다. 둘의 수명을 같은 ID에 묶는다.
	auto &[source, draft] = m_SummonPolicyDrafts[summon.strSummonId];
	if (source != summon)
	{
		source = summon;
		draft = summon;
	}
	ImGui::SeparatorText("Summon Playback");
	ImGui::TextDisabled(
		"Shared settings for %zu placed Summon box(es).", Count_SummonReferences(m_Draft, summon.strSummonId));
	const bool cross = draft.strSummonKind == "CROSS_DIRECTION_CLONES";
	if (ImGui::BeginCombo(
			"Behavior##SummonPolicy", cross ? "Four directions: one real body" : "Explicit spawned Patterns"))
	{
		if (ImGui::Selectable("Explicit spawned Patterns", !cross))
		{
			draft.strSummonKind.clear();
			draft.DirectionPatternIds.clear();
			draft.strCloneEndStageId.clear();
			draft.strRealPatternId.clear();
		}
		if (ImGui::Selectable("Four directions: one real body", cross))
		{
			draft.strSummonKind = "CROSS_DIRECTION_CLONES";
			draft.DirectionPatternIds.resize(4u);
		}
		ImGui::EndCombo();
	}
	if (draft.strSummonKind == "CROSS_DIRECTION_CLONES")
	{
		draft.DirectionPatternIds.resize(4u);
		constexpr std::array<const char *, 4u> labels{"Front Pattern", "Back Pattern", "Left Pattern", "Right Pattern"};
		for (size_t direction = 0u; direction < labels.size(); ++direction)
		{
			auto &id = draft.DirectionPatternIds[direction];
			const auto *selected = Find_Pattern(m_Draft, id);
			if (ImGui::BeginCombo(labels[direction], selected ? selected->strDisplayName.c_str() : "(select Pattern)"))
			{
				for (const auto &candidate : m_Draft.Patterns)
				{
					if (!candidate.strLoadError.empty() || candidate.strCategory != "MECHANIC" ||
						candidate.Stages.empty() || !candidate.PatternOccurrences.empty() ||
						!candidate.SummonOccurrences.empty() || !candidate.WorldOccurrences.empty() ||
						!candidate.SceneProfileOccurrences.empty() || candidate.BossMotion ||
						candidate.bResetBossToSpawn || candidate.ResetBossYawDegrees ||
						candidate.bEnterCombatOnFinish || Find_TimelineParent(m_Draft, candidate.strPatternId))
						continue;
					if (parent &&
						(candidate.strPatternId == parent->strPatternId || candidate.strGateId != parent->strGateId ||
							candidate.strActorProfileId != parent->strActorProfileId ||
							candidate.strTargetBossPlacementId != parent->strTargetBossPlacementId))
						continue;
					std::string logicStatus;
					if (!CKoukuSaydonCompositionDocument::Validate_CrossDirectionChildLogic(
							m_Draft, candidate, logicStatus))
						continue;
					if (ImGui::Selectable((candidate.strDisplayName + "##" + candidate.strPatternId).c_str(),
							id == candidate.strPatternId))
						id = candidate.strPatternId;
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			ImGui::PushID(static_cast<int>(direction));
			ImGui::BeginDisabled(!selected || !selected->strLoadError.empty());
			const bool openSource = ImGui::SmallButton("Open Source Pattern");
			ImGui::EndDisabled();
			ImGui::PopID();
			if (openSource)
			{
				const auto childId = id;
				const auto parentId = parent ? parent->strPatternId : std::string{};
				std::string status;
				if (Select_PatternById(childId, status))
					m_strParentReturnPatternId = parentId;
				return true;
			}
		}
		const auto *realPattern = Find_Pattern(m_Draft, draft.strRealPatternId);
		if (ImGui::BeginCombo("Real body##SummonPolicy",
				realPattern ? realPattern->strDisplayName.c_str() : "Automatic: nearest arena center"))
		{
			if (ImGui::Selectable("Automatic: nearest arena center", draft.strRealPatternId.empty()))
				draft.strRealPatternId.clear();
			for (const auto &id : draft.DirectionPatternIds)
				if (const auto *child = Find_Pattern(m_Draft, id))
					if (ImGui::Selectable(
							(child->strDisplayName + "##real-" + id).c_str(), draft.strRealPatternId == id))
						draft.strRealPatternId = id;
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("Clones play through##SummonPolicy",
				draft.strCloneEndStageId.empty() ? "(select Stage)" : draft.strCloneEndStageId.c_str()))
		{
			if (const auto *first = Find_Pattern(m_Draft, draft.DirectionPatternIds.front()))
				for (const auto &stage : first->Stages)
				{
					// 첫 방향의 Stage 이름을 나머지 세 방향도 가져야 한 종료 Stage로 세 clone의 수명을 표현할 수 있다.
					const bool common = std::all_of(
						draft.DirectionPatternIds.begin(), draft.DirectionPatternIds.end(), [&](const auto &id) {
							const auto *child = Find_Pattern(m_Draft, id);
							return child &&
								std::any_of(child->Stages.begin(), child->Stages.end(), [&](const auto &other) {
									return other.strStageId == stage.strStageId;
								});
						});
					if (common &&
						ImGui::Selectable(stage.strStageId.c_str(), stage.strStageId == draft.strCloneEndStageId))
						draft.strCloneEndStageId = stage.strStageId;
				}
			ImGui::EndCombo();
		}
		ImGui::TextWrapped(
			"The selected real body plays its full animation and Effects. Automatic chooses the direction ending nearest the arena center. The other three clones disappear after the selected Stage. The Summon lifetime must cover all four Patterns.");
	}
	else
		ImGui::TextWrapped("Each Summon box configures its own spawned Pattern rows. A name alone creates no actors.");
	ImGui::BeginDisabled(draft == summon || !m_bHasDraft);
	// 미완성 draft를 바로 공유 문서에 쓰지 않는다. 네 정책 필드만 후보에 복사하고 전체 참조 검증 후 교체한다.
	if (ImGui::Button("Apply Summon Playback"))
	{
		auto candidate = m_Draft;
		const auto found = std::find_if(candidate.Summons.begin(), candidate.Summons.end(), [&](const auto &value) {
			return value.strSummonId == summon.strSummonId;
		});
		if (found != candidate.Summons.end())
		{
			found->strSummonKind = draft.strSummonKind;
			found->DirectionPatternIds = draft.DirectionPatternIds;
			found->strCloneEndStageId = draft.strCloneEndStageId;
			found->strRealPatternId = draft.strRealPatternId;
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated Summon playback. Save to keep the changes.", status);
		}
		ImGui::EndDisabled();
		return true;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Reset Summon Playback"))
		draft = summon;
	return false;
}

// Resources의 Summon 탭이다. 재사용 정의 선택과 재생 정책 편집, 선택 Pattern의 발생 시각·수명 배치를 이어 준다.
// 선택 Pattern과 정의는 문서에서 차용하므로 Rename·정책 Apply·Append·Delete 뒤에는 현재 그리기를 끝낸다.
void Client::CKoukuSaydonActionWorkbench::Render_SummonResources()
{
	ImGui::SeparatorText("Summon Catalog");
	ImGui::TextDisabled("%zu summons | Create names what a Pattern spawns; Append places its spawn time and lifetime.",
		m_Draft.Summons.size());
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	const KOUKU_SAYDON_COMPOSITION_PATTERN *const selectedPattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool_t patternEditable = nullptr != selectedPattern && selectedPattern->strLoadError.empty();
	const std::string targetPatternName =
		nullptr == selectedPattern ? std::string("none") : selectedPattern->strDisplayName;
	const std::uint32_t lifetimeMs = nullptr == selectedPattern ? 0u : Pattern_DurationMs(*selectedPattern);

	if (ImGui::BeginChild("##KoukuSummonList", ImVec2(0.f, 160.f), ImGuiChildFlags_Borders))
	{
		if (m_Draft.Summons.empty())
			ImGui::TextDisabled("No Summon yet. Name it and press Create Summon.");
		for (const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION &summon : m_Draft.Summons)
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
	ImGui::InputTextWithHint(
		"##NewKoukuSummonName", "Summon display name", m_NewSummonName, std::size(m_NewSummonName));
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

	const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION *const selectedSummon =
		Find_Summon(m_Draft, m_strSelectedSummonId);
	if (nullptr == selectedSummon)
		return;
	const std::string summonId = selectedSummon->strSummonId;
	const std::string summonName = selectedSummon->strDisplayName;
	bool_t unresolvedReferences = false;
	const std::size_t references = Count_SummonReferences(m_Draft, summonId, &unresolvedReferences);
	ImGui::SeparatorText("Selected Summon");
	ImGui::TextWrapped("%s", summonName.c_str());
	if (Render_RenameControl(RENAME_TARGET::SUMMON, summonId, summonName))
		return;
	ImGui::TextDisabled("%s | %zu box(es)", summonId.c_str(), references);
	if (Render_SummonPolicy(*selectedSummon, selectedPattern))
		return;
	ImGui::Checkbox("Until Pattern end##NewKoukuSummonBox", &m_bNewSummonBoxToPatternEnd);
	if (!m_bNewSummonBoxToPatternEnd)
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(120.f);
		ImGui::InputInt("Box ms##NewKoukuSummonBox", &m_iNewSummonBoxDurationMs, 100, 1000);
		m_iNewSummonBoxDurationMs = std::clamp(m_iNewSummonBoxDurationMs, 1, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	}
	ImGui::BeginDisabled(!patternEditable);
	if (ImGui::Button("Append Summon at Cursor"))
	{
		const std::uint32_t durationMs = m_bNewSummonBoxToPatternEnd
			? (lifetimeMs > m_iCursorMs ? lifetimeMs - m_iCursorMs : 1u)
			: static_cast<std::uint32_t>(m_iNewSummonBoxDurationMs);
		std::string occurrenceId;
		std::string status;
		(void)Append_SummonBox(m_strSelectedPatternId, summonId, m_iCursorMs, durationMs, occurrenceId, status);
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
	ImGui::TextDisabled(
		"Append target: %s | cursor %u ms | lifetime %u ms", targetPatternName.c_str(), m_iCursorMs, lifetimeMs);
}

// Summon 상세가 복사한 spawn 목록의 각 행을 편집한다. choices의 Pattern 포인터는 현재 m_Draft에서 이 호출 동안만 차용한다.
// Pattern 선택·좌표·yaw·추가·삭제는 임시 spawns와 changed에만 기록하고 문서 교체는 상위 상세의 검증 경계에 남긴다.
void Client::CKoukuSaydonActionWorkbench::Render_SummonSpawnRows(
	std::vector<KOUKU_SAYDON_COMPOSITION_SUMMON_PATTERN_SPAWN> &spawns,
	const std::vector<const KOUKU_SAYDON_COMPOSITION_PATTERN *> &choices, bool &changed)
{
	for (std::size_t index = 0u; index < spawns.size(); ++index)
	{
		auto &spawn = spawns[index];
		ImGui::PushID(spawn.strSpawnId.c_str());
		const auto *selected = Find_Pattern(m_Draft, spawn.strPatternId);
		if (ImGui::BeginCombo("Pattern", selected ? selected->strDisplayName.c_str() : spawn.strPatternId.c_str()))
		{
			for (const auto *choice : choices)
				if (ImGui::Selectable((choice->strDisplayName + "##" + choice->strPatternId).c_str(),
						choice->strPatternId == spawn.strPatternId))
				{
					spawn.strPatternId = choice->strPatternId;
					changed = true;
				}
			ImGui::EndCombo();
		}
		ImGui::TextDisabled("%s", spawn.strSpawnId.c_str());
		float offset[3] = {static_cast<float>(spawn.PositionOffset[0]), static_cast<float>(spawn.PositionOffset[1]),
			static_cast<float>(spawn.PositionOffset[2])};
		if (ImGui::BeginCombo("Spawn space", spawn.strAnchorKind == "MAP" ? "Map fixed" : "Boss relative"))
		{
			if (ImGui::Selectable("Boss relative", spawn.strAnchorKind == "BOSS"))
			{
				spawn.strAnchorKind = "BOSS";
				changed = true;
			}
			if (ImGui::Selectable("Map fixed", spawn.strAnchorKind == "MAP"))
			{
				spawn.strAnchorKind = "MAP";
				changed = true;
			}
			ImGui::EndCombo();
		}
		// MAP 값은 world XYZ, BOSS 값은 owner의 시작 pose 기준 offset이다. 같은 배열의 의미를 anchor가 결정한다.
		const bool fixed = spawn.strAnchorKind == "MAP";
		const float positionLimit = fixed ? 100000.f : 1000.f;
		if (ImGui::DragFloat3(fixed ? "World position (m)" : "Position offset (m)", offset, 0.05f, -positionLimit,
				positionLimit, "%.2f", ImGuiSliderFlags_AlwaysClamp))
		{
			for (std::size_t axis = 0u; axis < 3u; ++axis)
				spawn.PositionOffset[axis] = offset[axis];
			changed = true;
		}
		const auto character = CAnimationTargetService::Resolve_SceneCharacter();
		const auto transform = character ? character->Get_Transform() : nullptr;
		if (fixed)
		{
			ImGui::BeginDisabled(!transform);
			if (ImGui::SmallButton("Use Player Position") && transform)
			{
				float3_t position;
				XMStoreFloat3(&position, transform->Get_State(STATE::POSITION));
				spawn.PositionOffset = {position.x, position.y, position.z};
				changed = true;
			}
			ImGui::EndDisabled();
		}
		float yaw = static_cast<float>(spawn.fYawOffsetDegrees);
		if (ImGui::DragFloat(fixed ? "World yaw (degrees)" : "Yaw offset (degrees)", &yaw, 0.5f, -360.f, 360.f, "%.1f",
				ImGuiSliderFlags_AlwaysClamp))
		{
			spawn.fYawOffsetDegrees = yaw;
			changed = true;
		}
		if (ImGui::SmallButton("Remove spawned Pattern"))
		{
			// erase는 뒤 원소를 당겨 spawn 참조를 무효화할 수 있다. 이번 반복을 바로 끝내고 다음 프레임에 vector와 ID를 다시 읽는다.
			spawns.erase(spawns.begin() + index);
			changed = true;
			ImGui::PopID();
			break;
		}
		ImGui::PopID();
	}
	ImGui::BeginDisabled(spawns.size() >= 16u || choices.empty());
	if (ImGui::Button("Add spawned Pattern") && !choices.empty())
	{
		KOUKU_SAYDON_COMPOSITION_SUMMON_PATTERN_SPAWN spawn;
		// vector 위치가 아닌 stable spawn ID를 만든다. 삭제 뒤에도 살아 있는 ID와 충돌하지 않는 첫 ordinal을 찾는다.
		std::uint32_t ordinal = 1u;
		do
		{
			spawn.strSpawnId = "spawn." + std::to_string(ordinal++);
		} while (std::any_of(spawns.begin(), spawns.end(), [&](const auto &row) {
			return row.strSpawnId == spawn.strSpawnId;
		}));
		spawn.strPatternId = choices.front()->strPatternId;
		spawns.push_back(std::move(spawn));
		changed = true;
	}
	ImGui::EndDisabled();
	if (spawns.empty())
		ImGui::TextDisabled(
			"No Pattern spawns: configure Four directions above or add spawned Patterns to create actors.");
	if (choices.empty())
		ImGui::TextDisabled(
			"Choose a leaf MECHANIC for this Gate and actor: Animation, BOSS Effect/Sound and airborne JUMP/SLAM are supported.");
}

// 명시적 spawn이 비어 있을 때 양쪽 다섯 명씩의 MAP 배치를 만드는 상세 입력이다. 중심·간격·회전 편집값은 Workbench 세션에 둔다.
// 버튼을 누르면 임시 목록에 위치와 안쪽을 향하는 yaw를 생성한다. 보스 spawn 조회 실패는 이전 중심값을 보존한다.
void Client::CKoukuSaydonActionWorkbench::Render_SummonFormation(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern,
	const std::string &occurrenceId, std::vector<KOUKU_SAYDON_COMPOSITION_SUMMON_PATTERN_SPAWN> &spawns,
	const std::vector<const KOUKU_SAYDON_COMPOSITION_PATTERN *> &choices, bool &changed)
{
	if (spawns.empty() && !choices.empty() && ImGui::TreeNode("Create two rows: 5 left + 5 right"))
	{
		if (m_strSummonFormationOwnerId != occurrenceId)
		{
			m_strSummonFormationOwnerId = occurrenceId;
			std::array<double, 3u> center;
			std::string status;
			if (Resolve_PresentationBossSpawnPosition(m_Draft.strAreaId, pattern, center, status))
				for (std::size_t axis = 0u; axis < 3u; ++axis)
					m_SummonFormationCenter[axis] = float(center[axis]);
		}
		const auto selected = std::find_if(choices.begin(), choices.end(), [&](const auto *choice) {
			return choice->strPatternId == m_strSummonFormationPatternId;
		});
		if (selected == choices.end())
			m_strSummonFormationPatternId = choices.front()->strPatternId;
		const auto *child = Find_Pattern(m_Draft, m_strSummonFormationPatternId);
		if (ImGui::BeginCombo("Formation Pattern", child ? child->strDisplayName.c_str() : "(choose Pattern)"))
		{
			for (const auto *choice : choices)
				if (ImGui::Selectable((choice->strDisplayName + "##formation" + choice->strPatternId).c_str(),
						choice->strPatternId == m_strSummonFormationPatternId))
					m_strSummonFormationPatternId = choice->strPatternId;
			ImGui::EndCombo();
		}
		ImGui::DragFloat3("Formation center XYZ (m)", m_SummonFormationCenter, 0.05f, -100000.f, 100000.f, "%.2f",
			ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::SmallButton("Use Boss Spawn Center"))
		{
			std::array<double, 3u> center;
			if (Resolve_PresentationBossSpawnPosition(m_Draft.strAreaId, pattern, center, m_strStatus))
				for (std::size_t axis = 0u; axis < 3u; ++axis)
					m_SummonFormationCenter[axis] = float(center[axis]);
		}
		const auto character = CAnimationTargetService::Resolve_SceneCharacter();
		const auto transform = character ? character->Get_Transform() : nullptr;
		ImGui::SameLine();
		ImGui::BeginDisabled(!transform);
		if (ImGui::SmallButton("Use Player Center") && transform)
		{
			float3_t center;
			XMStoreFloat3(&center, transform->Get_State(STATE::POSITION));
			m_SummonFormationCenter[0] = center.x;
			m_SummonFormationCenter[1] = center.y;
			m_SummonFormationCenter[2] = center.z;
		}
		ImGui::EndDisabled();
		ImGui::DragFloat("Center to each side (m)", &m_fSummonFormationHalfWidth, 0.05f, 0.1f, 1000.f, "%.2f",
			ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("Spacing along each row (m)", &m_fSummonFormationSpacing, 0.05f, 0.1f, 1000.f, "%.2f",
			ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat(
			"Layout yaw (degrees)", &m_fSummonFormationYaw, 0.5f, -360.f, 360.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::TextWrapped(
			"Layout yaw rotates both rows around the center. All ten actors face inward. Each position and yaw can be edited after creation. Creating rows extends the box and Pattern when the child needs more time.");
		if (ImGui::Button("Create 10 spawned Patterns"))
		{
			// yaw 입력은 degree다. XZ 평면에서 local (x,z)를 회전하고 m 단위 중심을 더해 world 위치를 만든다.
			// 각 행은 중심에서 좌우 half width만큼 떨어지고 index-2는 다섯 점을 가운데 기준으로 대칭 배치한다.
			const double radians = XMConvertToRadians(m_fSummonFormationYaw);
			const double cosine = std::cos(radians), sine = std::sin(radians);
			for (const int side : {-1, 1})
				for (int index = 0; index < 5; ++index)
				{
					KOUKU_SAYDON_COMPOSITION_SUMMON_PATTERN_SPAWN spawn;
					spawn.strSpawnId = std::string(side < 0 ? "left." : "right.") + std::to_string(index + 1);
					spawn.strPatternId = m_strSummonFormationPatternId;
					spawn.strAnchorKind = "MAP";
					const double x = side * m_fSummonFormationHalfWidth, z = (index - 2) * m_fSummonFormationSpacing;
					spawn.PositionOffset = {m_SummonFormationCenter[0] + x * cosine + z * sine,
						m_SummonFormationCenter[1], m_SummonFormationCenter[2] - x * sine + z * cosine};
					// 두 행이 서로 안쪽을 보도록 좌우 side에 반대 90도 yaw를 더하고 한 회전 안의 값으로 줄인다.
					spawn.fYawOffsetDegrees = std::remainder(double(m_fSummonFormationYaw) - side * 90.0, 360.0);
					spawns.push_back(std::move(spawn));
				}
			changed = true;
		}
		ImGui::TreePop();
	}
}

// Details dispatcher가 선택 Pattern을 차용해 부른다. 시간창·공유 재생 정책과 이 box만의 spawn 목록을 구별해 편집한다.
// spawn 목록은 복사본으로 편집하고 child 수명까지 맞춘 후보를 한 번 검증·commit한다. 실패하면 기존 box와 preview를 유지한다.
void Client::CKoukuSaydonActionWorkbench::Render_SummonBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE *const box =
		Find_SummonBox(pattern, m_strSelectedSummonOccurrenceId);
	if (nullptr == box)
		return;
	const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION *const summon = Find_Summon(m_Draft, box->strSummonId);
	const std::string patternId = pattern.strPatternId;
	const std::string occurrenceId = box->strOccurrenceId;
	const std::uint32_t lifetimeMs = Pattern_DurationMs(pattern);
	ImGui::SeparatorText("Summon Box");
	ImGui::Text("%s", occurrenceId.c_str());
	ImGui::Text("%s", nullptr == summon ? "(missing Summon)" : summon->strDisplayName.c_str());
	ImGui::TextDisabled("%s", box->strSummonId.c_str());
	ImGui::InputInt("Spawn ms##KoukuSummonBox", &m_iSummonBoxStartMs, 10, 100);
	ImGui::InputInt("Lifetime ms##KoukuSummonBox", &m_iSummonBoxDurationMs, 10, 100);
	m_iSummonBoxStartMs = std::clamp(m_iSummonBoxStartMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_iSummonBoxDurationMs = std::clamp(m_iSummonBoxDurationMs, 1, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	if (ImGui::Button("Apply Window##KoukuSummonBox"))
	{
		std::string status;
		(void)Set_SummonBoxWindow(patternId, occurrenceId, static_cast<std::uint32_t>(m_iSummonBoxStartMs),
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
	ImGui::TextDisabled(
		"The spawn appears at Spawn ms and despawns when Lifetime ends. Pattern lifetime %u ms.", lifetimeMs);
	if (summon && Render_SummonPolicy(*summon, &pattern))
		return;
	if (summon && summon->strSummonKind == "CROSS_DIRECTION_CLONES")
	{
		ImGui::TextWrapped(
			"This box owns all four actors through its Summon playback settings. Separate spawned Pattern rows are unnecessary.");
		if (ImGui::Button("Delete Summon Box"))
		{
			std::string status;
			(void)Delete_SummonBox(patternId, occurrenceId, status);
		}
		return;
	}
	ImGui::SeparatorText("Spawned Patterns");
	ImGui::TextWrapped(
		"Each row creates an independent actor at Spawn ms. Boss relative follows the owner's starting pose; Map fixed uses world XYZ and yaw. Animation, actor Effects and jump/landing Logic play on each actor.");
	ImGui::BeginDisabled(box->PatternSpawns.empty());
	if (ImGui::Button("World Preview##SummonPlacement"))
	{
		Queue_SummonBoxPreview(patternId, occurrenceId, true);
		// 버튼 클릭 시 아래 공통 EndDisabled에 도달하지 않으므로 열었던 UI 범위를 여기서 닫는다.
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	ImGui::TextWrapped(
		"Position and yaw edits update the existing preview actors at the paused cursor. World Preview starts at this Summon's spawn when the cursor is outside its lifetime.");
	// 아래 UI는 복사본만 편집한다. choices의 차용 포인터가 필요한 모든 그리기를 끝낸 후 candidate를 commit한다.
	auto spawns = box->PatternSpawns;
	bool changed = false;
	std::vector<const KOUKU_SAYDON_COMPOSITION_PATTERN *> choices;
	for (const auto &row : m_Draft.Patterns)
	{
		std::string status;
		if (CKoukuSaydonCompositionDocument::Validate_SummonPatternTarget(m_Draft, pattern, row.strPatternId, status))
			choices.push_back(&row);
	}
	Render_SummonSpawnRows(spawns, choices, changed);
	Render_SummonFormation(pattern, occurrenceId, spawns, choices, changed);
	if (changed)
	{
		auto candidate = m_Draft;
		auto *editedPattern = Find_Pattern(candidate, patternId);
		auto *editedBox = editedPattern ? Find_SummonBox(*editedPattern, occurrenceId) : nullptr;
		if (editedBox)
		{
			// 개수 또는 spawn/Pattern ID가 바뀌면 actor 구성을 다시 만든다. 위치·yaw만 바뀌면 기존 preview actor를 갱신한다.
			const bool rebuildPreview = editedBox->PatternSpawns.size() != spawns.size() ||
				!std::equal(editedBox->PatternSpawns.begin(), editedBox->PatternSpawns.end(), spawns.begin(),
					spawns.end(), [](const auto &before, const auto &after) {
						return before.strSpawnId == after.strSpawnId && before.strPatternId == after.strPatternId;
					});
			editedBox->PatternSpawns = std::move(spawns);
			// 새 child의 전체 재생 길이가 box에 들어가도록 수명을 늘린 뒤 상위 Pattern의 모든 lane 끝도 검증한다.
			for (const auto &spawn : editedBox->PatternSpawns)
				if (const auto *child = Find_Pattern(candidate, spawn.strPatternId))
					editedBox->iDurationMs = (std::max)(editedBox->iDurationMs, Pattern_DurationMs(*child));
			std::string status;
			if (!Extend_PatternLifetimeForAuthoredLanes(*editedPattern, status))
			{
				m_strStatus = status;
				return;
			}
			if (Commit_Candidate(std::move(candidate),
					"Updated this Summon box's spawned Patterns and preserved their complete lifetime. Save to keep the changes.",
					status))
			{
				Synchronize_EditorFields();
				Queue_SummonBoxPreview(patternId, occurrenceId, rebuildPreview);
			}
		}
		return;
	}
	if (ImGui::Button("Delete Summon Box"))
	{
		std::string status;
		(void)Delete_SummonBox(patternId, occurrenceId, status);
	}
}
