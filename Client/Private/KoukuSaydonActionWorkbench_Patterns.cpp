// 패턴·폴더·묶음의 구조 편집과 리소스 트리를 담당한다. 참조 제거와 부모 시간 변경도 같은 문서 후보에서 검증한다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"
#include "KoukuSaydonActionWorkbench_CameraLookup.h"

// 이 도메인에서 직접 사용하는 런타임/저작 자료의 선언이다.
#include "GameInstance.h"
#include "Profiler.h"

// 후보 값 복사, stable ID 조회, 시간/좌표 계산에 필요한 표준 타입이다.
#include <algorithm>
#include <array>
#include <cctype>
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

constexpr std::array<const char_t *, 3u> ACTOR_PROFILES = {"MN_RPCZ_00", "MN_RPCT_05", "MN_RPCT_06"};

std::pair<const KOUKU_SAYDON_COMPOSITION_PATTERN *, const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE *> Find_BingoBoard(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document)
{
	const auto *flow = Find_BingoFlow(document);
	std::pair<const KOUKU_SAYDON_COMPOSITION_PATTERN *, const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE *> disabled{};
	if (flow)
		for (const auto &entry : flow->Entries)
		{
			if (entry.strKind != "PATTERN")
				continue;
			const auto *pattern = Find_Pattern(document, entry.strTargetId);
			if (!pattern)
				continue;
			for (const auto &box : pattern->LogicOccurrences)
				if (const auto *logic = Find_Logic(document, box.strLogicId);
					logic && logic->strLogicType == "DURATION" && logic->strJudgementKind == "BINGO_BOARD")
				{
					if (box.bEnabled)
						return {pattern, &box};
					if (!disabled.first)
						disabled = {pattern, &box};
				}
		}
	return disabled;
}

// 마지막 Logic 박스 끝을 구한다. Pattern lifetime은 이 시각보다 줄일 수 없다.
std::uint32_t Pattern_LogicEndMs(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	std::uint64_t end = 0u;
	for (const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE &box : pattern.LogicOccurrences)
		end = (std::max)(end, static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs);
	return static_cast<std::uint32_t>((std::min)(end, static_cast<std::uint64_t>(MAX_EDITOR_TIME_MS)));
}

std::uint32_t Pattern_SummonEndMs(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	std::uint64_t end = 0u;
	for (const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE &box : pattern.SummonOccurrences)
		end = (std::max)(end, static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs);
	return static_cast<std::uint32_t>((std::min)(end, static_cast<std::uint64_t>(MAX_EDITOR_TIME_MS)));
}

/* World 박스는 시작점만 Pattern 안에 있으면 되지만 Scene Profile 박스는
	   전체 시간 구간이 Pattern 안에 있어야 한다. */
std::uint32_t Pattern_LaneEndMs(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	std::uint64_t end = 0u;
	for (const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE &box : pattern.WorldOccurrences)
		end = (std::max)(end, static_cast<std::uint64_t>(box.iStartMs) + 1u);
	for (const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE &box : pattern.SceneProfileOccurrences)
		end = (std::max)(end, static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs);
	for (const auto &box : pattern.PresentationOccurrences)
		end = (std::max)(end, static_cast<std::uint64_t>(box.iStartMs) + box.iDurationMs);
	return static_cast<std::uint32_t>((std::min)(end, static_cast<std::uint64_t>(MAX_EDITOR_TIME_MS)));
}
bool Stage_ParentTimeline(KOUKU_SAYDON_COMPOSITION_DOCUMENT &candidate, const std::string_view folderId,
	const std::string_view actorProfileId, std::string &patternId, std::string &status)
{
	auto folder = std::find_if(candidate.Folders.begin(), candidate.Folders.end(), [&](const auto &row) {
		return row.strFolderId == folderId;
	});
	if (folder == candidate.Folders.end() || !folder->strLoadError.empty())
	{
		status = "Select a valid Parent.";
		return false;
	}
	if (!folder->strTimelinePatternId.empty())
	{
		patternId = folder->strTimelinePatternId;
		return true;
	}
	if (candidate.iNextPatternOrdinal >= 1000000u)
	{
		status = "Parent timeline Pattern IDs are exhausted.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_PATTERN timeline;
	timeline.strPatternId = "KOUKUSAYDON_G1_PATTERN_" + std::to_string(candidate.iNextPatternOrdinal++);
	timeline.strActorProfileId = actorProfileId;
	timeline.strGateId = folder->strGateId;
	timeline.strTargetBossPlacementId =
		CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(timeline.strGateId, actorProfileId);
	if (timeline.strTargetBossPlacementId.empty())
	{
		status = "Choose a model supported by this Parent's Gate.";
		return false;
	}
	timeline.strFolderId = folder->strFolderId;
	timeline.strDisplayName = folder->strDisplayName;
	timeline.strCategory = "MECHANIC";
	timeline.strAuthoringStatus = "DRAFT";
	timeline.iDurationMs = 15000u;
	patternId = timeline.strPatternId;
	folder->strTimelinePatternId = patternId;
	candidate.Patterns.push_back(std::move(timeline));
	return true;
}
} // 이 파일의 보조 구현 범위 끝

bool_t Client::CKoukuSaydonActionWorkbench::Select_BingoParent(std::string &outStatus)
{
	if (m_bSequenceWorkspace || !m_bHasDraft || !Find_BingoFlow(m_Draft))
	{
		outStatus = m_strStatus = "Load the saved Bingo Pattern Flow before opening its Parent.";
		return false;
	}
	Select_Hierarchy(KOUKU_PATTERN_SELECTION::GATE, "BINGO");
	m_ePatternSelection = KOUKU_PATTERN_SELECTION::BINGO_PARENT;
	m_bBingoParentReturn = false;
	Select_BingoParentBox(0);
	outStatus = m_strStatus = "Bingo Parent: common board, special interruption and repeating Pattern Flow.";
	return true;
}

// Bingo 흐름 전체를 후보에 교체하여 공통 문서 검증을 통과한 경우에만 반영한다. UI 박스 선택 상태와 실행 흐름 정의를 구분한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_BingoFlow(
	const KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW &flow, std::string &outStatus)
{
	const auto *original = Find_BingoFlow(m_Draft);
	if (!original || flow.strGateId != "BINGO" || flow.strFlowId != original->strFlowId)
	{
		outStatus = m_strStatus = "Bingo Parent edits must retain the saved Flow identity.";
		return false;
	}
	auto candidate = m_Draft;
	for (auto &row : candidate.PatternFlows)
		if (row.strFlowId == flow.strFlowId)
			row = flow;
	return Commit_Candidate(
		std::move(candidate), "Updated Bingo Parent. Save and Publish All Patterns before Complete Play.", outStatus);
}

void Client::CKoukuSaydonActionWorkbench::Select_BingoParentBox(const int kind, const std::string &entryId)
{
	m_iBingoParentBox = kind;
	m_strBingoParentEntryId = entryId;
	Clear_TimelineSelection();
	m_strSelectedPatternId.clear();
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	m_strSelectedPatternOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedStageId.clear();
	if (kind == 0)
	{
		const auto [pattern, box] = Find_BingoBoard(m_Draft);
		if (pattern && box)
		{
			m_strSelectedPatternId = pattern->strPatternId;
			m_strSelectedLogicOccurrenceId = box->strOccurrenceId;
		}
	}
	if (const auto *flow = Find_BingoFlow(m_Draft))
		for (const auto &entry : flow->Entries)
			if (entry.strEntryId == entryId)
				m_iBingoEntryWaitMs = static_cast<int>(entry.iWaitAfterMs);
	Synchronize_EditorFields();
}

void Client::CKoukuSaydonActionWorkbench::Render_BingoParentTimeline()
{
	const auto *source = Find_BingoFlow(m_Draft);
	if (!source)
	{
		ImGui::TextDisabled("The saved Bingo Flow is unavailable.");
		return;
	}
	const auto flow = *source; // UI 명령이 draft를 교체할 수 있으므로 이후에는 이 프레임의 행 포인터를 쓰지 않는다.
	ImGui::SeparatorText("Bingo Complete [Parent]");
	ImGui::TextWrapped("%s", flow.strDisplayName.c_str());
	ImGui::BeginDisabled(!Is_Dirty() || Is_PublishRunning());
	if (ImGui::Button("Save##BingoParent"))
	{
		std::string status;
		(void)Save(status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(Is_Dirty() || Is_PublishRunning() || !m_CompleteSequenceAdmission);
	if (ImGui::Button("Play Bingo Parent"))
	{
		std::string status;
		(void)m_CompleteSequenceAdmission("BINGO", status);
		m_strStatus = status;
	}
	ImGui::EndDisabled();
	ImGui::TextWrapped(
		"Common Logic runs alongside normal Patterns. Every third head mark interrupts the current Pattern with the special Parent; afterwards that Pattern restarts. Select a box to edit its details.");
	ImGui::TextDisabled(
		"Pattern lengths below describe one pass. The Server advances after actual completion and follow-ups.");
	if (!m_strStatus.empty())
		ImGui::TextWrapped("%s", m_strStatus.c_str());

	const auto card = [&](const char *id, const std::string &label, const bool selected, const ImU32 color,
						  const float height) {
		ImGui::PushID(id);
		const auto origin = ImGui::GetCursorScreenPos();
		const float width = (std::max)(80.f, ImGui::GetContentRegionAvail().x);
		const bool clicked = ImGui::InvisibleButton("box", ImVec2(width, height));
		auto *draw = ImGui::GetWindowDrawList();
		const ImVec2 end(origin.x + width, origin.y + height);
		draw->AddRectFilled(origin, end, color, 4.f);
		draw->AddRect(origin, end, selected ? IM_COL32(255, 215, 80, 255) : IM_COL32(100, 130, 150, 255), 4.f, 0,
			selected ? 2.f : 1.f);
		draw->PushClipRect(origin, end, true);
		draw->AddText(nullptr, ImGui::GetFontSize(), ImVec2(origin.x + 8, origin.y + 6), IM_COL32_WHITE, label.c_str(),
			nullptr, width - 16);
		draw->PopClipRect();
		ImGui::PopID();
		return clicked;
	};
	if (ImGui::BeginChild("BingoParentFlow", ImVec2(0, 0), ImGuiChildFlags_Borders))
	{
		ImGui::SeparatorText("Common Logic / concurrent with all Patterns");
		const auto [boardPattern, boardBox] = Find_BingoBoard(m_Draft);
		const auto *board = boardBox ? Find_Logic(m_Draft, boardBox->strLogicId) : nullptr;
		std::string boardLabel = board ? board->strDisplayName : "BINGO_BOARD is not connected to this Flow";
		if (board)
			boardLabel += "\n" + board->strBingoActiveMode + " | start " + std::to_string(boardBox->iStartMs) +
				(board->strBingoActiveMode == "WINDOW"
						? " ms | lifetime " + std::to_string(boardBox->iDurationMs) + " ms"
						: " ms | until encounter ends") +
				" | first bomb " + std::to_string(board->iBingoFirstBombDelayMs) + " ms | interval " +
				std::to_string(board->iBingoBombIntervalMs) + " ms" + (boardBox->bEnabled ? "" : " | DISABLED");
		if (card("board", boardLabel, m_iBingoParentBox == 0, IM_COL32(47, 95, 75, 255), 82))
			Select_BingoParentBox(0);
		if (board && ImGui::BeginTable("BombLifecycle", 4, ImGuiTableFlags_SizingStretchSame))
		{
			const std::array<std::pair<const char *, std::uint32_t>, 4> phases = {
				{{"First delay", board->iBingoFirstBombDelayMs}, {"Head mark", board->iBingoBombMarkMs},
					{"Drop delay", board->iBingoBombDropDelayMs}, {"Planted fuse", board->iBingoBombFuseMs}}};
			for (const auto &[name, duration] : phases)
			{
				ImGui::TableNextColumn();
				if (card(name, std::string(name) + "\n" + std::to_string(duration) + " ms", false,
						IM_COL32(38, 69, 60, 255), 60))
					Select_BingoParentBox(0);
			}
			ImGui::EndTable();
			ImGui::TextDisabled(
				"Each mark repeats at the authored interval. Fuse completion flips the centre cell and its four neighbours.");
		}

		ImGui::SeparatorText("Special Patterns / every third head mark");
		const auto *special = Find_Pattern(m_Draft, flow.strBingoSpecialPatternId);
		if (card("special",
				special
					? special->strDisplayName + "\n[Special Parent] Select to change its target or open child Logic."
					: "Select the special Parent in Box Detail",
				m_iBingoParentBox == 1, IM_COL32(110, 65, 48, 255), 72))
			Select_BingoParentBox(1);
		if (special)
			for (const auto &child : special->PatternOccurrences)
			{
				const auto *pattern = Find_Pattern(m_Draft, child.strPatternId);
				ImGui::PushID(child.strOccurrenceId.c_str());
				const auto label = std::to_string(child.iStartMs) + " - " +
					std::to_string(child.iStartMs + child.iDurationMs) + " ms | " +
					(pattern ? pattern->strDisplayName : child.strPatternId);
				if (ImGui::Selectable(label.c_str()))
				{
					std::string status;
					if (Select_PatternById(child.strPatternId, status))
						m_bBingoParentReturn = true;
					ImGui::PopID();
					ImGui::EndChild();
					return;
				}
				ImGui::PopID();
			}
		const auto loop = std::find_if(flow.Entries.begin(), flow.Entries.end(), [&](const auto &entry) {
			return entry.strEntryId == flow.strLoopStartEntryId;
		});
		const size_t loopIndex = static_cast<size_t>(loop - flow.Entries.begin());
		const auto drawSection = [&](const char *title, const size_t first, const size_t last) {
			ImGui::SeparatorText(title);
			const int columns = std::clamp(static_cast<int>(ImGui::GetContentRegionAvail().x / 260.f), 1, 4);
			if (!ImGui::BeginTable(title, columns, ImGuiTableFlags_SizingStretchSame))
				return;
			for (size_t i = first; i < last; ++i)
			{
				const auto &entry = flow.Entries[i];
				const auto *pattern = entry.strKind == "PATTERN" ? Find_Pattern(m_Draft, entry.strTargetId) : nullptr;
				// 공통 controller는 이미 위쪽에 전용 lane으로 표시했다.
				if (pattern && Is_BingoBoardControl(m_Draft, *pattern))
					continue;
				const auto foundBundle =
					std::find_if(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto &row) {
						return entry.strKind == "BUNDLE" && row.strBundleId == entry.strTargetId;
					});
				const auto *bundle = foundBundle == m_Draft.Bundles.end() ? nullptr : &*foundBundle;
				ImGui::TableNextColumn();
				const std::string name = pattern ? pattern->strDisplayName
					: bundle					 ? bundle->strDisplayName
												 : entry.strTargetId;
				std::string label = std::to_string(i + 1) + ". " + name + "\n";
				if (pattern)
					label += std::to_string(Pattern_DurationMs(*pattern)) + " ms + ";
				label += "wait " + std::to_string(entry.iWaitAfterMs) + " ms";
				if (entry.strEntryId == flow.strLoopStartEntryId)
					label += "\nLOOP START";
				if (card(entry.strEntryId.c_str(), label,
						m_iBingoParentBox == 2 && m_strBingoParentEntryId == entry.strEntryId,
						i >= loopIndex ? IM_COL32(42, 78, 109, 255) : IM_COL32(60, 64, 80, 255), 104))
					Select_BingoParentBox(2, entry.strEntryId);
			}
			ImGui::EndTable();
		};
		drawSection("Opening Patterns / once", 0, loopIndex);
		drawSection("Repeating Patterns / return to LOOP START", loopIndex, flow.Entries.size());
		if (ImGui::Button("Append Pattern to Flow"))
			ImGui::OpenPopup("BingoFlowAppend");
		if (ImGui::BeginPopup("BingoFlowAppend"))
		{
			for (const auto &pattern : m_Draft.Patterns)
			{
				if (!pattern.strLoadError.empty() || pattern.strPatternId == flow.strBingoSpecialPatternId ||
					pattern.strGateId != "BINGO")
					continue;
				if (ImGui::Selectable((pattern.strDisplayName + "##" + pattern.strPatternId).c_str()))
				{
					auto edited = flow;
					KOUKU_SAYDON_COMPOSITION_FLOW_ENTRY entry;
					unsigned ordinal = 1;
					do
					{
						entry.strEntryId = flow.strFlowId + ".entry." + std::to_string(ordinal++);
					} while (std::any_of(edited.Entries.begin(), edited.Entries.end(), [&](const auto &row) {
						return row.strEntryId == entry.strEntryId;
					}));
					entry.strKind = "PATTERN";
					entry.strTargetId = pattern.strPatternId;
					edited.Entries.push_back(entry);
					std::string status;
					if (Set_BingoFlow(edited, status))
						Select_BingoParentBox(2, entry.strEntryId);
					ImGui::EndPopup();
					ImGui::EndChild();
					return;
				}
			}
			ImGui::EndPopup();
		}
	}
	ImGui::EndChild();
}

void Client::CKoukuSaydonActionWorkbench::Render_BingoParentDetails()
{
	const auto *source = Find_BingoFlow(m_Draft);
	if (!source)
	{
		ImGui::TextDisabled("Bingo Parent is unavailable.");
		return;
	}
	const auto flow = *source;
	ImGui::SeparatorText("Bingo Parent / Box Detail");
	if (m_iBingoParentBox == 0)
	{
		const auto [pattern, box] = Find_BingoBoard(m_Draft);
		if (!pattern || !box)
		{
			ImGui::TextWrapped("Append a Pattern containing BINGO_BOARD to this Flow to connect its common Logic.");
			return;
		}
		if (m_strSelectedPatternId != pattern->strPatternId || m_strSelectedLogicOccurrenceId != box->strOccurrenceId)
			Select_BingoParentBox(0);
		ImGui::TextWrapped(
			"The common board shares the whole battle. Its Logic window does not delay the normal attack sequence. ENCOUNTER ignores Lifetime; WINDOW uses Start and Lifetime to stop new spawns.");
		if (ImGui::Button("Open Board Pattern Timeline"))
		{
			const auto patternId = pattern->strPatternId, boxId = box->strOccurrenceId;
			std::string status;
			if (Select_PatternById(patternId, status))
			{
				m_bBingoParentReturn = true;
				m_strSelectedLogicOccurrenceId = boxId;
				Synchronize_EditorFields();
			}
			return;
		}
		Render_LogicBoxDetails(*pattern);
		return;
	}
	if (m_iBingoParentBox == 1)
	{
		const auto *pattern = Find_Pattern(m_Draft, flow.strBingoSpecialPatternId);
		ImGui::TextWrapped(
			"The third bomb head mark interrupts the normal row. The special Parent owns teleport, Medusa, black hole and its child Logic.");
		std::string replacement;
		if (ImGui::BeginCombo("Special Parent", pattern ? pattern->strDisplayName.c_str() : "Select Parent"))
		{
			for (const auto &item : m_Draft.Patterns)
			{
				std::string error;
				if (!CKoukuSaydonCompositionDocument::Validate_BingoSpecialPatternTarget(
						m_Draft, item.strPatternId, error))
					continue;
				if (ImGui::Selectable((item.strDisplayName + "##" + item.strPatternId).c_str(),
						item.strPatternId == flow.strBingoSpecialPatternId))
					replacement = item.strPatternId;
			}
			ImGui::EndCombo();
		}
		if (!replacement.empty())
		{
			auto edited = flow;
			edited.strBingoSpecialPatternId = replacement;
			std::string status;
			(void)Set_BingoFlow(edited, status);
			return;
		}
		if (pattern && ImGui::Button("Open Special Parent Timeline"))
		{
			std::string status;
			if (Select_PatternById(pattern->strPatternId, status))
				m_bBingoParentReturn = true;
		}
		return;
	}
	const auto selected = std::find_if(flow.Entries.begin(), flow.Entries.end(), [&](const auto &entry) {
		return entry.strEntryId == m_strBingoParentEntryId;
	});
	if (selected == flow.Entries.end())
	{
		ImGui::TextDisabled("Select a Pattern box in the Parent.");
		return;
	}
	const auto index = static_cast<size_t>(selected - flow.Entries.begin());
	const auto *pattern = selected->strKind == "PATTERN" ? Find_Pattern(m_Draft, selected->strTargetId) : nullptr;
	ImGui::TextWrapped("%s", pattern ? pattern->strDisplayName.c_str() : selected->strTargetId.c_str());
	ImGui::TextDisabled("%s", selected->strEntryId.c_str());
	if (ImGui::Button("Open Pattern / Bundle Timeline"))
	{
		std::string status;
		const bool opened = pattern ? Select_PatternById(selected->strTargetId, status)
									: Select_BundleById(selected->strTargetId, status);
		if (opened)
			m_bBingoParentReturn = true;
		return;
	}
	ImGui::InputInt("Wait after completion (ms)", &m_iBingoEntryWaitMs, 100, 1000);
	m_iBingoEntryWaitMs = std::clamp(m_iBingoEntryWaitMs, 0, 600000);
	if (ImGui::Button("Apply Wait"))
	{
		auto edited = flow;
		edited.Entries[index].iWaitAfterMs = static_cast<unsigned>(m_iBingoEntryWaitMs);
		std::string status;
		(void)Set_BingoFlow(edited, status);
		return;
	}
	ImGui::BeginDisabled(index == 0);
	if (ImGui::Button("< Earlier##BingoFlow"))
	{
		auto edited = flow;
		std::swap(edited.Entries[index], edited.Entries[index - 1]);
		std::string status;
		(void)Set_BingoFlow(edited, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(index + 1 == flow.Entries.size());
	if (ImGui::Button("Later >##BingoFlow"))
	{
		auto edited = flow;
		std::swap(edited.Entries[index], edited.Entries[index + 1]);
		std::string status;
		(void)Set_BingoFlow(edited, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	if (ImGui::Button("Set Loop Start Here"))
	{
		auto edited = flow;
		edited.strLoopStartEntryId = selected->strEntryId;
		std::string status;
		(void)Set_BingoFlow(edited, status);
		return;
	}
	ImGui::TextWrapped(
		"Rows before Loop Start play once. The remaining rows repeat. A special interruption restarts the interrupted row afterwards.");
	const bool referenced = selected->strEntryId == flow.strLoopStartEntryId ||
		std::any_of(flow.EntryGroups.begin(), flow.EntryGroups.end(), [&](const auto &group) {
			return group.strStartEntryId == selected->strEntryId || group.strEndEntryId == selected->strEntryId;
		});
	ImGui::BeginDisabled(flow.Entries.size() <= 1 || referenced);
	if (ImGui::Button("Remove from Flow"))
	{
		auto edited = flow;
		edited.Entries.erase(edited.Entries.begin() + index);
		std::string status;
		if (Set_BingoFlow(edited, status))
			Select_BingoParentBox(0);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	if (referenced)
		ImGui::TextDisabled("Move the Loop Start or group boundary before removing this row.");
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_PatternCreationDestination(
	const std::string_view folderId, const std::string_view bundleId, std::string &outStatus)
{
	if (!m_bHasDraft)
	{
		outStatus = m_strStatus = "Load the Composition before choosing a Pattern destination.";
		return false;
	}
	if (!folderId.empty())
	{
		const auto folder = std::find_if(m_Draft.Folders.begin(), m_Draft.Folders.end(), [&](const auto &row) {
			return row.strFolderId == folderId;
		});
		if (folder == m_Draft.Folders.end() || folder->strGateId != m_strSelectedGateId ||
			!folder->strLoadError.empty())
		{
			outStatus = m_strStatus = "Choose a valid Parent from the selected Gate.";
			return false;
		}
	}
	if (!bundleId.empty())
	{
		const auto bundle = std::find_if(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto &row) {
			return row.strBundleId == bundleId;
		});
		if (bundle == m_Draft.Bundles.end() || bundle->strGateId != m_strSelectedGateId ||
			bundle->strFolderId != folderId || !bundle->strLoadError.empty())
		{
			outStatus = m_strStatus = "Choose a valid Bundle under the selected Parent, or None.";
			return false;
		}
	}
	const std::string parent(folderId), bundle(bundleId);
	m_strCreateFolderId = parent;
	m_strCreateBundleId = bundle;
	outStatus = "Pattern creation destination selected.";
	return true;
}

// 현재 관문/actor와 생성 목적지를 확인하고 새 stable ID를 문서 후보에 부여한다. 성공한 commit 이후에만 새 Pattern을 선택한다.
bool_t Client::CKoukuSaydonActionWorkbench::Create_Pattern(const std::string_view displayName,
	const std::string_view category, std::string &outPatternId, std::string &outStatus)
{
	if (!m_bHasDraft || m_Draft.iNextPatternOrdinal >= 1000000u)
	{
		outStatus = "KoukuSaydon Pattern creation requires a loaded draft and available ordinal.";
		return false;
	}
	if (!Set_PatternCreationDestination(m_strCreateFolderId, m_strCreateBundleId, outStatus))
		return false;
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
	pattern.strFolderId = m_strCreateBundleId.empty() ? m_strCreateFolderId : std::string{};
	pattern.strPatternId = "KOUKUSAYDON_G1_PATTERN_" + std::to_string(candidate.iNextPatternOrdinal++);
	pattern.strDisplayName = std::string(displayName);
	pattern.strActorProfileId = m_strCreateActorProfileId;
	pattern.strGateId = m_strSelectedGateId;
	pattern.strTargetBossPlacementId =
		CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(pattern.strGateId, pattern.strActorProfileId);
	if (pattern.strTargetBossPlacementId.empty())
	{
		outStatus = "Select a valid target boss for this Gate.";
		return false;
	}
	pattern.strAuthoringStatus = "DRAFT";
	pattern.strCategory = std::string(category);
	pattern.iNextStageOrdinal = 1u;
	pattern.iNextAnimationOrdinal = 1u;
	const std::string patternId = pattern.strPatternId;
	candidate.Patterns.push_back(std::move(pattern));
	if (!m_strCreateBundleId.empty())
	{
		auto bundle = std::find_if(candidate.Bundles.begin(), candidate.Bundles.end(), [&](const auto &item) {
			return item.strBundleId == m_strCreateBundleId;
		});
		if (bundle == candidate.Bundles.end() || bundle->strGateId != m_strSelectedGateId)
		{
			outStatus = "The selected destination bundle is unavailable.";
			return false;
		}
		bundle->Members.push_back(
			{bundle->strBundleId + ".member." + std::to_string(bundle->iNextMemberOrdinal++), patternId, 0u});
		bundle->strAuthoringStatus = "DRAFT";
	}
	Rebuild_PlayAllPatternIds(candidate);
	if (!Commit_Candidate(std::move(candidate), "Created KoukuSaydon draft Pattern " + patternId + ".", outStatus))
	{
		return false;
	}
	Stop_Preview();
	m_bPresentationPreviewRequestPending = false;
	Clear_TimelineSelection();
	m_bFitRequested = true;
	m_ePatternSelection = KOUKU_PATTERN_SELECTION::PATTERN;
	m_strSelectedFolderId = m_strCreateFolderId;
	m_strSelectedBundleId = m_strCreateBundleId;
	m_strBundleReturnId = m_strCreateBundleId;
	m_strSelectedPatternId = patternId;
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	Normalize_Selection();
	Synchronize_EditorFields();
	outPatternId = patternId;
	return true;
}

// 폴더의 순차 timeline 소유 Pattern을 후보에 만든다. 기존 부모 참조와 자식 시간 검증은 공통 commit 경로를 사용한다.
bool_t Client::CKoukuSaydonActionWorkbench::Create_ParentTimeline(
	const std::string_view folderId, std::string &outStatus)
{
	auto candidate = m_Draft;
	auto folder = std::find_if(candidate.Folders.begin(), candidate.Folders.end(), [&](const auto &row) {
		return row.strFolderId == folderId;
	});
	if (folder != candidate.Folders.end() && !folder->strTimelinePatternId.empty())
		return Select_PatternById(folder->strTimelinePatternId, outStatus);
	std::string actorProfileId = m_strCreateActorProfileId;
	for (const auto &child : candidate.Patterns)
		if (child.strFolderId == folderId && child.strLoadError.empty())
		{
			actorProfileId = child.strActorProfileId;
			break;
		}
	std::string id;
	if (!Stage_ParentTimeline(candidate, folderId, actorProfileId, id, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	if (!Commit_Candidate(
			std::move(candidate), "Created Parent timeline. Add common rows and Append Pattern, then Save.", outStatus))
		return false;
	return Select_PatternById(id, outStatus);
}

// 부모 timeline에 자식 Pattern 참조를 새 occurrence로 배치한다. 대상/시간/중첩 관계가 유효한 후보만 반영한다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_PatternBox(const std::string_view ownerId,
	const std::string_view childId, const std::uint32_t startMs, const std::uint32_t durationMs, std::string &outStatus)
{
	auto candidate = m_Draft;
	std::string parentId(ownerId);
	auto *parent = Find_Pattern(candidate, parentId);
	if (!parent)
	{
		const auto *child = Find_Pattern(candidate, childId);
		if (!child || !child->strLoadError.empty())
		{
			outStatus = m_strStatus = "Choose an available source Pattern.";
			return false;
		}
		const auto actorProfileId = child->strActorProfileId;
		if (!Stage_ParentTimeline(candidate, ownerId, actorProfileId, parentId, outStatus))
		{
			m_strStatus = outStatus;
			return false;
		}
		parent = Find_Pattern(candidate, parentId);
	}
	if (!parent || !Find_TimelineParent(candidate, parentId) || parent->iNextPatternOccurrenceOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Append Pattern requires an editable Parent timeline.";
		return false;
	}
	const std::string id = parent->strPatternId + ".pattern." + std::to_string(parent->iNextPatternOccurrenceOrdinal++);
	parent->PatternOccurrences.push_back({id, std::string(childId), startMs, durationMs, false});
	Mark_Draft(candidate, *parent);
	KOUKU_SAYDON_COMPOSITION_DOCUMENT expanded;
	if (!CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(candidate, parentId, expanded, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	if (!Commit_Candidate(std::move(candidate),
			"Appended the complete Pattern. Its source remains shared; this box owns its start, window and repeat.",
			outStatus))
		return false;
	if (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN || m_strSelectedPatternId != parentId)
		(void)Select_PatternById(parentId, outStatus);
	Select_TimelineBox({}, id, false);
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_PatternBoxWindow(const std::string_view ownerId,
	const std::string_view occurrenceId, const std::uint32_t startMs, const std::uint32_t durationMs,
	const bool_t repeat, std::string &outStatus)
{
	auto candidate = m_Draft;
	auto *parent = Find_Pattern(candidate, ownerId);
	if (!parent)
	{
		outStatus = m_strStatus = "Pattern box owner is missing.";
		return false;
	}
	const auto found =
		std::find_if(parent->PatternOccurrences.begin(), parent->PatternOccurrences.end(), [&](const auto &row) {
			return row.strOccurrenceId == occurrenceId;
		});
	if (found == parent->PatternOccurrences.end())
	{
		outStatus = m_strStatus = "Pattern box is missing.";
		return false;
	}
	found->iStartMs = startMs;
	found->iDurationMs = durationMs;
	found->bRepeat = repeat;
	Mark_Draft(candidate, *parent);
	KOUKU_SAYDON_COMPOSITION_DOCUMENT expanded;
	if (!CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(candidate, ownerId, expanded, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	return Commit_Candidate(
		std::move(candidate), "Updated Pattern box. Its remaining actions cancel at the window end.", outStatus);
}

// 부모 cycle을 펼친 snapshot에서 새 저작 Pattern을 구성한다. 실행 파생 ID를 저장용 ID로 바꾸고 원본 자식과 Logic 소유권을 분리한다.
bool_t Client::CKoukuSaydonActionWorkbench::Repeat_ParentCycle(const std::string_view ownerId,
	const std::uint32_t loopWindowMs, std::string &outCyclePatternId, std::string &outStatus)
{
	const std::string parentId(ownerId);
	const auto reject = [&](const std::string &reason) {
		outStatus = m_strStatus = "Repeat Parent Cycle rejected; draft preserved: " + reason;
		return false;
	};
	const auto *original = Find_Pattern(m_Draft, parentId);
	if (!m_bHasDraft || !original || !original->strLoadError.empty() || !Find_TimelineParent(m_Draft, parentId))
		return reject("Select an editable Parent timeline.");
	const auto cycleDuration = Pattern_DurationMs(*original);
	if (!cycleDuration || loopWindowMs < cycleDuration || loopWindowMs > MAX_EDITOR_TIME_MS)
		return reject("Loop Window must cover one complete cycle and be at most 600000 ms.");
	if (m_Draft.iNextPatternOrdinal >= 1000000u || original->iNextPatternOccurrenceOrdinal >= 1000000u)
		return reject("Parent or Pattern stable IDs are exhausted.");

	// Preview/Server Play와 같은 clipping 및 typed 참조 규칙으로 현재 cycle을 펼친다.
	// 저장된 자식 원본은 별개로 유지한다.
	auto previewSource = m_Draft;
	if (!Apply_PresentationPreviewPlacements(previewSource, m_StagedPresentationGeometry, {parentId}, outStatus))
		return reject(outStatus);
	KOUKU_SAYDON_COMPOSITION_DOCUMENT expanded;
	if (!CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(previewSource, parentId, expanded, outStatus))
		return reject(outStatus);
	const auto *flattened = Find_Pattern(expanded, parentId);
	if (!flattened || flattened->BossMotion)
		return reject("A repeating cycle cannot own an absolute boss movement path.");
	auto candidate = m_Draft;
	auto cycle = *flattened;
	const std::string cycleId = "KOUKUSAYDON_G1_PATTERN_" + std::to_string(candidate.iNextPatternOrdinal++);
	cycle.strPatternId = cycleId;
	cycle.strDisplayName += " / Cycle";
	cycle.strAuthoringStatus = "DRAFT";
	cycle.iDurationMs = cycleDuration;
	cycle.bEnterCombatOnFinish = false;
	cycle.bResetBossToSpawn = false;
	cycle.ResetBossYawDegrees.reset();
	cycle.AnimationBlendWindows.clear();
	cycle.iNextStageOrdinal = cycle.iNextAnimationOrdinal = cycle.iNextLogicOccurrenceOrdinal = 1u;
	cycle.iNextSummonOccurrenceOrdinal = cycle.iNextWorldOccurrenceOrdinal = cycle.iNextSceneProfileOccurrenceOrdinal =
		1u;
	cycle.iNextPresentationOccurrenceOrdinal = cycle.iNextPatternOccurrenceOrdinal = 1u;

	// 실행 snapshot은 파생 ID를 사용한다. 재사용할 저작 Pattern에는
	// 정식 ID를 부여하고 occurrence별 Logic 정의를 독립시킨다.
	std::unordered_map<std::string, std::string> ids;
	for (auto &stage : cycle.Stages)
	{
		const auto ordinal = std::to_string(cycle.iNextStageOrdinal++);
		stage.strStageId = "STAGE_" + ordinal;
		stage.strActionId = cycleId + ".stage." + ordinal;
		for (auto &row : stage.AnimationOccurrences)
		{
			const auto old = row.strOccurrenceId;
			row.strOccurrenceId = cycleId + ".animation." + std::to_string(cycle.iNextAnimationOrdinal++);
			ids.emplace(old, row.strOccurrenceId);
		}
	}
	const auto remapLane = [&](auto &rows, std::uint32_t &ordinal, const char *family) {
		for (auto &row : rows)
		{
			const auto old = row.strOccurrenceId;
			row.strOccurrenceId = cycleId + family + std::to_string(ordinal++);
			ids.emplace(old, row.strOccurrenceId);
		}
	};
	remapLane(cycle.LogicOccurrences, cycle.iNextLogicOccurrenceOrdinal, ".logic.");
	remapLane(cycle.SummonOccurrences, cycle.iNextSummonOccurrenceOrdinal, ".summon.");
	remapLane(cycle.WorldOccurrences, cycle.iNextWorldOccurrenceOrdinal, ".world.");
	remapLane(cycle.SceneProfileOccurrences, cycle.iNextSceneProfileOccurrenceOrdinal, ".sceneprofile.");
	remapLane(cycle.PresentationOccurrences, cycle.iNextPresentationOccurrenceOrdinal, ".presentation.");
	for (auto &row : cycle.PresentationOccurrences)
		if (!row.strRegionId.empty())
			ids.emplace(row.strRegionId, row.strOccurrenceId + ".region");
	const auto remap = [&](std::string &id) {
		const auto found = ids.find(id);
		if (found != ids.end())
			id = found->second;
	};
	std::unordered_set<std::string> usedLogic;
	for (const auto &row : cycle.LogicOccurrences)
	{
		usedLogic.insert(row.strLogicId);
		for (const auto *outcomes : {&row.OnSuccessLogicIds, &row.OnFailLogicIds, &row.OnTimeoutLogicIds})
			usedLogic.insert(outcomes->begin(), outcomes->end());
	}
	for (const auto &definition : expanded.Logics)
		if (usedLogic.contains(definition.strLogicId))
		{
			if (candidate.iNextLogicOrdinal >= 1000000u)
				return reject("Logic stable IDs are exhausted.");
			ids.emplace(definition.strLogicId, "koukusaydon.g1.logic." + std::to_string(candidate.iNextLogicOrdinal++));
		}
	for (const auto &id : usedLogic)
		if (!ids.contains(id))
			return reject("A cycle Logic definition is missing: " + id);
	std::uint32_t contactOrdinal = 1u;
	for (const auto &definition : expanded.Logics)
	{
		if (!usedLogic.contains(definition.strLogicId))
			continue;
		auto copy = definition;
		remap(copy.strLogicId);
		copy.strDisplayName += " (cycle)";
		for (auto &id : copy.RegionIds)
			remap(id);
		for (auto &id : copy.TargetWorldOccurrenceIds)
			remap(id);
		for (auto &id : copy.PlayerEntryEffectOccurrenceIds)
			remap(id);
		for (auto &motion : copy.ContactMotions)
			remap(motion.strTargetWorldOccurrenceId);
		remap(copy.strSummonOccurrenceId);
		remap(copy.strBombPresentationOccurrenceId);
		remap(copy.strBombExplosionPresentationOccurrenceId);
		remap(copy.strBombSectorPresentationOccurrenceId);
		remap(copy.strTargetLogicOccurrenceId);
		remap(copy.strContactTargetWorldOccurrenceId);
		remap(copy.strTargetWorldInstanceId);
		if (!copy.strContactGroupId.empty())
		{
			const auto [group, inserted] =
				ids.emplace(copy.strContactGroupId, cycleId + ".contact." + std::to_string(contactOrdinal++));
			copy.strContactGroupId = group->second;
		}
		candidate.Logics.push_back(std::move(copy));
	}
	for (auto &row : cycle.LogicOccurrences)
	{
		remap(row.strLogicId);
		remap(row.strHoldLogicOccurrenceId);
		for (auto *outcomes : {&row.OnSuccessLogicIds, &row.OnFailLogicIds, &row.OnTimeoutLogicIds})
			for (auto &id : *outcomes)
				remap(id);
	}
	for (auto &row : cycle.PresentationOccurrences)
	{
		remap(row.strLogicOccurrenceId);
		remap(row.strWorldOccurrenceId);
		remap(row.strRegionId);
		remap(row.strAnchorPresentationOccurrenceId);
	}
	auto *parent = Find_Pattern(candidate, parentId);
	const std::string boxId = parentId + ".pattern." + std::to_string(parent->iNextPatternOccurrenceOrdinal++);
	parent->Stages.clear();
	parent->AnimationBlendWindows.clear();
	parent->LogicOccurrences.clear();
	parent->SummonOccurrences.clear();
	parent->WorldOccurrences.clear();
	parent->SceneProfileOccurrences.clear();
	parent->PresentationOccurrences.clear();
	parent->PatternOccurrences = {{boxId, cycleId, 0u, loopWindowMs, true}};
	parent->iDurationMs = loopWindowMs;
	Mark_Draft(candidate, *parent);
	candidate.Patterns.push_back(std::move(cycle));
	Rebuild_PlayAllPatternIds(candidate);
	if (!CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(candidate, parentId, expanded, outStatus))
		return reject(outStatus);
	if (!Commit_Candidate(std::move(candidate),
			"Parent cycle repeats within its window. Open Source Pattern to edit the cycle; Apply Pattern Window changes the repeat duration. Press Save.",
			outStatus))
		return false;
	std::erase_if(m_StagedPresentationGeometry, [&](const auto &edit) {
		return edit.strPatternId == parentId;
	});
	Stop_Preview();
	Clear_TimelineSelection();
	if (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN || m_strSelectedPatternId != parentId)
		(void)Select_PatternById(parentId, outStatus);
	Select_TimelineBox({}, boxId, false);
	m_bFitRequested = true;
	outCyclePatternId = cycleId;
	outStatus = m_strStatus =
		"Parent cycle repeats within its window. Open Source Pattern to edit the cycle; Apply Pattern Window changes the repeat duration. Press Save.";
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Render_PatternReferenceControls(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	if (!Find_TimelineParent(m_Draft, pattern.strPatternId))
		return false;
	const std::string ownerId = pattern.strPatternId;
	ImGui::SeparatorText("Pattern row");
	if (Render_AppendPatternButton())
		return true;
	ImGui::TextWrapped(
		"Choose a source in Composition Resources > Pattern. Each box includes its Animation, Effect, Collider and Logic.");
	if (ImGui::Button("Repeat Parent Cycle"))
	{
		m_iParentLoopWindowMs = static_cast<int32_t>(
			(std::min)(std::uint64_t(MAX_EDITOR_TIME_MS), std::uint64_t(Pattern_DurationMs(pattern)) * 2u));
		ImGui::OpenPopup("Repeat Parent Cycle##Window");
	}
	if (ImGui::BeginPopup("Repeat Parent Cycle##Window"))
	{
		const auto duration = Pattern_DurationMs(pattern);
		ImGui::Text("One cycle: %u ms", duration);
		ImGui::InputInt("Loop Window ms", &m_iParentLoopWindowMs, 1000, 10000);
		if (duration && m_iParentLoopWindowMs > 0)
			ImGui::Text("Cycles: %.2f", double(m_iParentLoopWindowMs) / duration);
		ImGui::TextWrapped(
			"Move the current rows and a snapshot of their child Patterns into one editable Cycle Pattern. The Parent repeats it for this window; a partial last cycle stops at the window end.");
		if (ImGui::Button("Apply Parent Loop"))
		{
			std::string cycleId, status;
			if (m_iParentLoopWindowMs <= 0)
				m_strStatus = "Loop Window must be positive.";
			else
				(void)Repeat_ParentCycle(ownerId, static_cast<std::uint32_t>(m_iParentLoopWindowMs), cycleId, status);
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
			return true;
		}
		ImGui::EndPopup();
	}
	const auto *box = Find_PatternBox(pattern, m_strSelectedPatternOccurrenceId);
	if (!box)
		return false;
	const std::string boxId = box->strOccurrenceId, childId = box->strPatternId;
	const auto *child = Find_Pattern(m_Draft, childId);
	ImGui::TextWrapped("%s | source %s", boxId.c_str(), child ? child->strDisplayName.c_str() : childId.c_str());
	ImGui::InputInt("Start ms##PatternBox", &m_iPatternBoxStartMs, 100, 1000);
	ImGui::InputInt("Window ms##PatternBox", &m_iPatternBoxDurationMs, 100, 1000);
	ImGui::Checkbox("Repeat within window", &m_bPatternBoxRepeat);
	if (child)
		ImGui::TextDisabled("Source duration: %u ms", Pattern_DurationMs(*child));
	ImGui::TextWrapped(
		"The window keeps its authored end. A short window cancels unfinished actions; a longer non-repeating window leaves the boss idle after the source finishes.");
	if (ImGui::Button("Apply Pattern Window"))
	{
		std::string status;
		if (m_iPatternBoxStartMs < 0 || m_iPatternBoxDurationMs <= 0)
			m_strStatus = "Start must be nonnegative and window must be positive.";
		else
			(void)Set_PatternBoxWindow(
				ownerId, boxId, m_iPatternBoxStartMs, m_iPatternBoxDurationMs, m_bPatternBoxRepeat, status);
		return true;
	}
	if (ImGui::Button("Open Source Pattern"))
	{
		std::string status;
		if (Select_PatternById(childId, status))
			m_strParentReturnPatternId = ownerId;
		return true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Remove Pattern Box"))
	{
		std::string status;
		(void)Delete_TimelineSelection(ownerId, {}, {boxId}, status);
		return true;
	}
	ImGui::TextWrapped(
		"Editing the source changes every reference. Start, window and repeat change only this placement.");
	return false;
}

// 삭제 확인 UI가 보여 줄 실제 소비 참조를 모은다. 목록을 만드는 단계에서는 문서나 현재 선택을 변경하지 않는다.
std::vector<std::string> Client::CKoukuSaydonActionWorkbench::Collect_PatternDeleteReferences(
	const std::string_view patternId, const bool onlyBlocking) const
{
	std::vector<std::string> references;
	const auto collectIds = [&](const std::vector<std::string> &ids, const std::string &owner) {
		const auto count = std::count(ids.begin(), ids.end(), patternId);
		if (count)
			references.push_back(owner + " (" + std::to_string(count) + " references)");
	};
	for (const auto &logic : m_Draft.Logics)
	{
		const auto owner = "Logic: " + logic.strDisplayName + " [" + logic.strLogicId + "]";
		collectIds(logic.DirectionPatternIds, owner + " / directionPatternIds");
		collectIds(logic.PatternIds, owner + " / patternIds");
		if (!logic.strFollowupPatternId.empty() && logic.strFollowupPatternId == patternId)
			references.push_back(owner + " / followupPatternId");
		if (!logic.strClonePatternId.empty() && logic.strClonePatternId == patternId)
			references.push_back(owner + " / clonePatternId");
	}
	for (const auto &summon : m_Draft.Summons)
		collectIds(summon.DirectionPatternIds,
			"Summon: " + summon.strDisplayName + " [" + summon.strSummonId + "] / directionPatternIds");
	for (const auto &parent : m_Draft.Patterns)
	{
		if (parent.strPatternId == patternId)
			continue;
		for (const auto &box : parent.PatternOccurrences)
			if (box.strPatternId == patternId && (!onlyBlocking || !parent.strLoadError.empty()))
				references.push_back("Parent Pattern: " + parent.strDisplayName + " [" + parent.strPatternId + "] / " +
					box.strOccurrenceId);
		for (const auto &box : parent.SummonOccurrences)
			for (const auto &spawn : box.PatternSpawns)
				if (spawn.strPatternId == patternId)
					references.push_back("Summon in Pattern: " + parent.strDisplayName + " [" + parent.strPatternId +
						"] / " + box.strOccurrenceId + " / " + spawn.strSpawnId);
	}
	for (const auto &folder : m_Draft.Folders)
		if (folder.strTimelinePatternId == patternId && (!onlyBlocking || !folder.strLoadError.empty()))
			references.push_back("Parent timeline: " + folder.strDisplayName + " [" + folder.strFolderId + "]");
	for (const auto &bundle : m_Draft.Bundles)
		for (const auto &member : bundle.Members)
			if (member.strPatternId == patternId && (!onlyBlocking || !bundle.strLoadError.empty()))
				references.push_back(
					"Bundle: " + bundle.strDisplayName + " [" + bundle.strBundleId + "] / " + member.strMemberId);
	for (const auto &flow : m_Draft.PatternFlows)
	{
		if (!onlyBlocking && flow.strBingoSpecialPatternId == patternId)
			references.push_back("Bingo special: " + flow.strDisplayName + " [" + flow.strFlowId + "]");
		for (const auto &entry : flow.Entries)
			if (!onlyBlocking && entry.strKind == "PATTERN" && entry.strTargetId == patternId)
				references.push_back(
					"Pattern Flow: " + flow.strDisplayName + " [" + flow.strFlowId + "] / " + entry.strEntryId);
	}
	return references;
}

// 삭제할 Pattern의 소비 참조를 먼저 확인한다. 정의와 허용된 참조 정리를 하나의 후보로 검증하여 dangling ID를 남기지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Delete_Pattern(const std::string_view patternId, std::string &outStatus)
{
	// 호출자가 m_Draft 내부의 view를 넘길 수 있으므로 문서 교체 전에 값을 보존한다.
	const std::string targetId(patternId);
	if (!m_bHasDraft || Is_PublishRunning())
	{
		outStatus = m_strStatus =
			"Delete requires a loaded Composition and no publication in progress; draft preserved.";
		return false;
	}
	const auto references = Collect_PatternDeleteReferences(targetId, true);
	if (!references.empty())
	{
		outStatus = "Pattern is still used. Remove these references before deleting:";
		for (const auto &reference : references)
			outStatus += "\n- " + reference;
		m_strStatus = outStatus;
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	const auto found = std::find_if(candidate.Patterns.begin(), candidate.Patterns.end(),
		[&targetId](const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern) {
			return pattern.strPatternId == targetId;
		});
	if (found == candidate.Patterns.end())
	{
		outStatus = m_strStatus = "KoukuSaydon Pattern delete target is absent.";
		return false;
	}
	candidate.Patterns.erase(found);
	// 삭제할 정의의 배치 인스턴스다. 공유 원본 리소스는 유지하고 이 행들은
	// 같은 검증 후보에서 제거하여 Save에 끊어진 ID가 남지 않게 한다.
	for (auto &parent : candidate.Patterns)
		if (std::erase_if(parent.PatternOccurrences, [&](const auto &box) {
				return box.strPatternId == targetId;
			}))
			Mark_Draft(candidate, parent);
	for (auto &folder : candidate.Folders)
		if (folder.strTimelinePatternId == targetId)
			folder.strTimelinePatternId.clear();
	for (auto &bundle : candidate.Bundles)
		if (std::erase_if(bundle.Members, [&](const auto &member) {
				return member.strPatternId == targetId;
			}))
			bundle.strAuthoringStatus = "DRAFT";
	for (auto &flow : candidate.PatternFlows)
	{
		if (flow.strBingoSpecialPatternId == targetId)
			flow.strBingoSpecialPatternId.clear();
		std::erase_if(flow.Entries, [&](const auto &entry) {
			return entry.strKind == "PATTERN" && entry.strTargetId == targetId;
		});
		if (!flow.strLoopStartEntryId.empty() &&
			std::none_of(flow.Entries.begin(), flow.Entries.end(), [&](const auto &entry) {
				return entry.strEntryId == flow.strLoopStartEntryId;
			}))
			flow.strLoopStartEntryId.clear();
	}
	Rebuild_PlayAllPatternIds(candidate);
	if (!Commit_Candidate(std::move(candidate),
			"Deleted Pattern and its listed timeline, bundle and flow links from the draft. Save stores the change; review affected drafts before Publish All Patterns.",
			outStatus))
		return false;
	if (m_strAppendPatternId == targetId)
		m_strAppendPatternId.clear();
	if (m_strParentReturnPatternId == targetId)
		m_strParentReturnPatternId.clear();
	if (m_PreviewState.strPatternId == targetId)
		Stop_Preview();
	m_strStatus = outStatus;
	return true;
}

// 즉시 문서를 지우지 않고 삭제 대상 stable ID와 확인 창 상태를 보관한다. 실제 참조 점검/반영은 Delete_Pattern 경계에서 다시 수행한다.
void Client::CKoukuSaydonActionWorkbench::Request_PatternDelete(const std::string_view patternId)
{
	if (!m_bHasDraft || Is_PublishRunning() || !Find_Pattern(m_Draft, patternId))
		return;
	m_strDeletePatternId = std::string(patternId);
	m_iDeletePatternGeneration = m_iDraftGeneration;
	m_bPatternDeleteConfirmationRequested = true;
}

void Client::CKoukuSaydonActionWorkbench::Render_PatternDeleteContext(const std::string_view patternId)
{
	if (!ImGui::BeginPopupContextItem())
		return;
	if (ImGui::MenuItem(
			m_bSequenceWorkspace ? "Delete Sequence..." : "Delete Pattern...", nullptr, false, !Is_PublishRunning()))
		Request_PatternDelete(patternId);
	ImGui::EndPopup();
}

// 확인 창에서 삭제할 대상과 참조를 보여 준다. 확인 클릭은 공통 삭제 명령으로 보내고 취소는 예약 상태만 비운다.
void Client::CKoukuSaydonActionWorkbench::Render_PatternDeleteConfirmation()
{
	const char *const popup = m_bSequenceWorkspace ? "Delete Sequence?###SequenceDeleteConfirmation"
												   : "Delete Pattern?###PatternDeleteConfirmation";
	if (m_bPatternDeleteConfirmationRequested)
	{
		ImGui::OpenPopup(popup);
		m_bPatternDeleteConfirmationRequested = false;
	}
	ImGui::SetNextWindowSize(ImVec2(620.f, 0.f), ImGuiCond_Appearing);
	if (!ImGui::BeginPopupModal(popup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		return;
	const auto *pattern = Find_Pattern(m_Draft, m_strDeletePatternId);
	ImGui::TextWrapped("%s", pattern ? pattern->strDisplayName.c_str() : m_strDeletePatternId.c_str());
	ImGui::TextWrapped("ID: %s", m_strDeletePatternId.c_str());
	auto references = Collect_PatternDeleteReferences(m_strDeletePatternId);
	const auto blockers = Collect_PatternDeleteReferences(m_strDeletePatternId, true);
	std::erase_if(references, [&](const auto &reference) {
		return std::find(blockers.begin(), blockers.end(), reference) != blockers.end();
	});
	const bool current = pattern && m_iDeletePatternGeneration == m_iDraftGeneration;
	if (!current)
		ImGui::TextWrapped("The draft changed. Cancel and request Delete again to review the current Pattern.");
	if (!references.empty())
	{
		ImGui::TextWrapped(
			"Delete also removes the listed Pattern boxes, Bundle memberships, Flow entries and Parent timeline links. Shared resources remain available.");
		if (ImGui::BeginChild("##PatternDeleteReferences", ImVec2(0.f, 220.f), ImGuiChildFlags_Borders))
			for (const auto &reference : references)
				ImGui::TextWrapped("- %s", reference.c_str());
		ImGui::EndChild();
	}
	else
		ImGui::TextWrapped(
			"Delete this Pattern and its timeline boxes from the draft? Save stores the deletion. Shared Animation, Effect, Logic and other source resources remain available.");
	if (!blockers.empty())
	{
		ImGui::TextWrapped("These required Logic/Summon targets or invalid owners must be repaired before deletion:");
		for (const auto &blocker : blockers)
			ImGui::TextWrapped("- %s", blocker.c_str());
	}
	ImGui::BeginDisabled(!current || !blockers.empty() || Is_PublishRunning());
	if (ImGui::Button("Delete from Draft"))
	{
		std::string status;
		if (Delete_Pattern(m_strDeletePatternId, status))
		{
			m_strDeletePatternId.clear();
			ImGui::CloseCurrentPopup();
		}
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
	{
		m_strDeletePatternId.clear();
		ImGui::CloseCurrentPopup();
	}
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::EndPopup();
}

// Render_PatternProperties의 이름 입력이 patternId와 displayName을 함수 동안만 string_view로 빌려 준다. m_Draft 전체를 값 후보로 복사하는 비용을 들여 원본과 변경을 분리한다.
// 후보의 Pattern과 같은 patternId를 참조하는 Folder 표시 이름을 함께 고친 뒤 Commit_Candidate로 검증·확정을 요청한다. 검색이나 검증에 실패하면 이전 draft를 보존한다.
bool_t Client::CKoukuSaydonActionWorkbench::Rename_Pattern(
	const std::string_view patternId, const std::string_view displayName, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = "KoukuSaydon Pattern rename target is absent.";
		return false;
	}
	pattern->strDisplayName = std::string(displayName);
	for (auto &folder : candidate.Folders)
		if (folder.strTimelinePatternId == patternId)
			folder.strDisplayName = std::string(displayName);
	return Commit_Candidate(std::move(candidate), "Renamed KoukuSaydon Pattern display name.", outStatus);
}

// 종류별 stable ID로 후보의 표시 이름을 변경한다. ID를 이름으로 대체하지 않으며 문서 검증 후에만 편집 입력을 동기화한다.
bool_t Client::CKoukuSaydonActionWorkbench::Rename_Item(
	const RENAME_TARGET target, const std::string_view id, const std::string_view displayName, std::string &outStatus)
{
	if (displayName.empty() || displayName.size() >= std::size(m_RenameDisplayName) ||
		std::all_of(displayName.begin(), displayName.end(), [](const unsigned char c) {
			return std::isspace(c) != 0;
		}))
	{
		outStatus = m_strStatus = "Enter a non-empty display name of at most 255 UTF-8 bytes.";
		return false;
	}
	auto candidate = m_Draft;
	std::string *name = nullptr;
	switch (target)
	{
	case RENAME_TARGET::PATTERN:
		if (auto *row = Find_Pattern(candidate, id); row && row->strLoadError.empty())
			name = &row->strDisplayName;
		break;
	case RENAME_TARGET::FOLDER:
		for (auto &row : candidate.Folders)
			if (row.strFolderId == id && row.strLoadError.empty())
				name = &row.strDisplayName;
		break;
	case RENAME_TARGET::BUNDLE:
		for (auto &row : candidate.Bundles)
			if (row.strBundleId == id && row.strLoadError.empty())
				name = &row.strDisplayName;
		break;
	case RENAME_TARGET::LOGIC:
		for (auto &row : candidate.Logics)
			if (row.strLogicId == id)
				name = &row.strDisplayName;
		break;
	case RENAME_TARGET::SUMMON:
		for (auto &row : candidate.Summons)
			if (row.strSummonId == id)
				name = &row.strDisplayName;
		break;
	case RENAME_TARGET::WORLD:
		for (auto &row : candidate.Worlds)
			if (row.strWorldId == id)
				name = &row.strDisplayName;
		break;
	case RENAME_TARGET::SCENE_PROFILE:
		for (auto &row : candidate.SceneProfiles)
			if (row.strSceneProfileId == id)
				name = &row.strDisplayName;
		break;
	case RENAME_TARGET::PRESENTATION:
		for (auto &row : candidate.PresentationResources)
			if (row.strResourceId == id)
				name = &row.strDisplayName;
		break;
	}
	if (!m_bHasDraft || !name)
	{
		outStatus = m_strStatus = "Rename target is unavailable; the previous name is preserved.";
		return false;
	}
	if (*name == displayName)
	{
		outStatus = m_strStatus = "The display name is unchanged.";
		return true;
	}
	*name = std::string(displayName);
	for (auto &folder : candidate.Folders)
	{
		if (target == RENAME_TARGET::PATTERN && folder.strTimelinePatternId == id)
			folder.strDisplayName = std::string(displayName);
		if (target == RENAME_TARGET::FOLDER && folder.strFolderId == id && !folder.strTimelinePatternId.empty())
			if (auto *timeline = Find_Pattern(candidate, folder.strTimelinePatternId))
				timeline->strDisplayName = std::string(displayName);
	}
	return Commit_Candidate(
		std::move(candidate), "Renamed selected item. Save keeps the name and all existing connections.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Render_RenameControl(
	const RENAME_TARGET target, const std::string_view id, const std::string_view displayName)
{
	const char *const popup =
		m_bSequenceWorkspace ? "Rename selected item###SequenceRenameSelectedItem" : "Rename selected item";
	ImGui::PushID(static_cast<int>(target));
	bool_t applied = false;
	if (ImGui::Button("Rename"))
	{
		if (Copy_Text(m_RenameDisplayName, std::size(m_RenameDisplayName), displayName))
		{
			m_strRenameItemId = id;
			ImGui::OpenPopup(popup);
		}
		else
			m_strStatus = "The current name exceeds the editor capacity; the previous name is preserved.";
	}
	if (ImGui::BeginPopupModal(popup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextWrapped("%s", m_strRenameItemId.c_str());
		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere();
		ImGui::SetNextItemWidth(380.f);
		const bool enter = ImGui::InputText("Display name", m_RenameDisplayName, std::size(m_RenameDisplayName),
			ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
		ImGui::BeginDisabled(m_RenameDisplayName[0] == '\0');
		if (ImGui::Button("Apply name") || (enter && m_RenameDisplayName[0] != '\0'))
		{
			std::string status;
			applied = Rename_Item(target, m_strRenameItemId, m_RenameDisplayName, status);
			if (applied)
				ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::TextDisabled("Apply changes the draft. Save writes the composition.");
		if (!m_strStatus.empty())
			ImGui::TextWrapped("%s", m_strStatus.c_str());
		ImGui::EndPopup();
	}
	ImGui::PopID();
	return applied;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_PatternFolder(
	const std::string_view patternId, const std::string_view folderId, std::string &outStatus)
{
	const auto *source = Find_Pattern(m_Draft, patternId);
	if (!m_bHasDraft || !source || !source->strLoadError.empty())
	{
		outStatus = m_strStatus = "Choose a valid Pattern before changing its Parent.";
		return false;
	}
	if (!folderId.empty())
	{
		const auto folder = std::find_if(m_Draft.Folders.begin(), m_Draft.Folders.end(), [&](const auto &row) {
			return row.strFolderId == folderId;
		});
		if (folder == m_Draft.Folders.end() || folder->strGateId != source->strGateId || !folder->strLoadError.empty())
		{
			outStatus = m_strStatus = "Choose a valid Parent from the Pattern's Gate.";
			return false;
		}
	}
	if (source->strFolderId == folderId)
	{
		outStatus = "Pattern Parent is unchanged.";
		return true;
	}
	if (Find_TimelineParent(m_Draft, patternId))
	{
		outStatus = m_strStatus = "This timeline belongs to its Parent. Move independent child Patterns instead.";
		return false;
	}
	const std::string parent(folderId);
	const bool_t selected = m_strSelectedPatternId == patternId;
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	Find_Pattern(candidate, patternId)->strFolderId = parent;
	if (!Commit_Candidate(std::move(candidate), "Changed Pattern Parent.", outStatus))
		return false;
	if (selected)
	{
		m_strSelectedFolderId = parent;
		m_strCreateFolderId = parent;
		m_strSelectedBundleId.clear();
		m_strCreateBundleId.clear();
		m_strBundleReturnId.clear();
	}
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_PatternCategory(
	const std::string_view patternId, const std::string_view category, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
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
	return Commit_Candidate(
		std::move(candidate), "Changed KoukuSaydon Pattern category and returned it to DRAFT.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_PatternAuthoringStatus(
	const std::string_view patternId, const std::string_view authoringStatus, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = "KoukuSaydon Pattern status target is absent.";
		return false;
	}
	pattern->strAuthoringStatus = std::string(authoringStatus);
	Rebuild_PlayAllPatternIds(candidate);
	return Commit_Candidate(std::move(candidate), "Changed KoukuSaydon Pattern authoring status.", outStatus);
}

// 관문/폴더/Bundle 선택에 맞춰 Detail과 timeline 선택을 정리한다. 기존 preview와 남은 요청의 정리는 같은 세션에서 수행한다.
void Client::CKoukuSaydonActionWorkbench::Select_Hierarchy(
	const KOUKU_PATTERN_SELECTION kind, const std::string_view id)
{
	m_bBingoParentReturn = false;
	Stop_Preview();
	m_bPresentationPreviewRequestPending = false;
	m_bServerPlayRequestPending = false;
	m_strPendingBundleServerId.clear();
	m_strSelectedPatternId.clear();
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	m_strSelectedPatternOccurrenceId.clear();
	Clear_TimelineSelection();
	m_strBundleDragMemberId.clear();
	m_ePatternSelection = kind;
	m_HierarchyName[0] = '\0';
	if (kind == KOUKU_PATTERN_SELECTION::GATE)
	{
		m_strSelectedGateId = std::string(id);
		m_strSelectedFolderId.clear();
		m_strCreateFolderId.clear();
		m_strSelectedBundleId.clear();
		m_strCreateBundleId.clear();
		m_strBundleReturnId.clear();
		m_strModelViewProfile.clear();
		m_strCreateActorProfileId = id == "GATE2" ? "MN_RPCZ_00" : "MN_RPCT_05";
	}
	else if (kind == KOUKU_PATTERN_SELECTION::FOLDER)
	{
		m_strSelectedFolderId = std::string(id);
		m_strCreateFolderId = std::string(id);
		m_strSelectedBundleId.clear();
		m_strCreateBundleId.clear();
		m_strBundleReturnId.clear();
		if (const auto *folder = Find_Folder(m_Draft, id))
		{
			m_strSelectedGateId = folder->strGateId;
			Copy_Text(m_HierarchyName, std::size(m_HierarchyName), folder->strDisplayName);
			if (!folder->strTimelinePatternId.empty())
			{
				const auto timelineId = folder->strTimelinePatternId;
				std::string status;
				(void)Select_PatternById(timelineId, status);
				return;
			}
		}
	}
	else if (kind == KOUKU_PATTERN_SELECTION::BUNDLE)
	{
		m_strSelectedBundleId = std::string(id);
		m_strCreateBundleId = std::string(id);
		m_strBundleReturnId = std::string(id);
		if (const auto *bundle = Find_Bundle(m_Draft, id))
		{
			m_strSelectedGateId = bundle->strGateId;
			m_strSelectedFolderId = bundle->strFolderId;
			m_strCreateFolderId = bundle->strFolderId;
			Copy_Text(m_HierarchyName, std::size(m_HierarchyName), bundle->strDisplayName);
		}
	}
	m_strCursorPatternId = std::string(id);
	m_iCursorMs = 0;
	m_bFitRequested = true;
	m_strStatus = "Selected " + std::string(id);
}

bool_t Client::CKoukuSaydonActionWorkbench::Select_BundleById(const std::string_view id, std::string &status)
{
	if (!Find_Bundle(m_Draft, id))
	{
		status = "Bundle is not present in this document.";
		return false;
	}
	Select_Hierarchy(KOUKU_PATTERN_SELECTION::BUNDLE, id);
	status = m_strStatus;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Create_Hierarchy(const bool_t createBundle)
{
	auto candidate = m_Draft;
	std::string id, status;
	if (createBundle)
	{
		const auto *folder = Find_Folder(candidate, m_strSelectedFolderId);
		if (!folder || folder->strGateId != m_strSelectedGateId || !folder->strLoadError.empty())
		{
			m_strStatus = "Select a valid parent folder first.";
			return false;
		}
		KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
		bundle.strBundleId = "koukusaydon.bundle." + std::to_string(candidate.iNextBundleOrdinal++);
		bundle.strFolderId = folder->strFolderId;
		bundle.strGateId = folder->strGateId;
		bundle.strDisplayName = m_NewBundleName;
		id = bundle.strBundleId;
		candidate.Bundles.push_back(std::move(bundle));
	}
	else
	{
		KOUKU_SAYDON_COMPOSITION_FOLDER folder;
		folder.strFolderId = "koukusaydon.folder." + std::to_string(candidate.iNextFolderOrdinal++);
		folder.strGateId = m_strSelectedGateId;
		folder.strDisplayName = m_NewFolderName;
		id = folder.strFolderId;
		candidate.Folders.push_back(std::move(folder));
	}
	if (!Commit_Candidate(std::move(candidate), "Created hierarchy item.", status))
		return false;
	Select_Hierarchy(createBundle ? KOUKU_PATTERN_SELECTION::BUNDLE : KOUKU_PATTERN_SELECTION::FOLDER, id);
	(createBundle ? m_NewBundleName : m_NewFolderName)[0] = '\0';
	return true;
}

// 선택 Bundle이 재생할 Pattern 참조를 문서 후보에 추가한다. 실제 재생 데이터 복사/실행은 preview와 Server 준비 경로가 맡는다.
bool_t Client::CKoukuSaydonActionWorkbench::Link_BundlePattern(const std::string_view patternId)
{
	auto candidate = m_Draft;
	auto *bundle = Find_Bundle(candidate, m_strSelectedBundleId);
	const auto *pattern = Find_Pattern(candidate, patternId);
	if (!bundle || !pattern || bundle->strGateId != pattern->strGateId)
	{
		m_strStatus = "Choose a Pattern from this Gate.";
		return false;
	}
	if (std::any_of(bundle->Members.begin(), bundle->Members.end(), [&](const auto &member) {
			return member.strPatternId == patternId;
		}))
	{
		m_strStatus = "This Pattern is already linked.";
		return false;
	}
	bundle->Members.push_back(
		{bundle->strBundleId + ".member." + std::to_string(bundle->iNextMemberOrdinal++), std::string(patternId), 0u});
	bundle->strAuthoringStatus = "DRAFT";
	std::string status;
	return Commit_Candidate(std::move(candidate), "Linked existing Pattern; source clips preserved.", status);
}

bool_t Client::CKoukuSaydonActionWorkbench::Render_AppendPatternButton()
{
	ImGui::BeginDisabled(!m_bHasDraft);
	const bool requested = ImGui::Button("Append Pattern##ParentEntry");
	ImGui::EndDisabled();
	if (requested)
	{
		m_bResourcesOpen = true;
		m_bPatternResourceTabRequested = true;
		m_WorkbenchViewRequest.showResources = true;
		m_WorkbenchViewRequest.focusResources = true;
		m_WorkbenchViewRequest.restoreSequencer = true;
	}
	return requested;
}

bool_t Client::CKoukuSaydonActionWorkbench::Resolve_PatternResourceAppend(
	std::string &ownerId, std::uint32_t &startMs, std::uint32_t &durationMs, std::string &status) const
{
	ownerId.clear();
	startMs = durationMs = 0u;
	if (!m_bHasDraft || !m_Document.Is_Fresh())
	{
		status = "Load a valid Composition before adding a Pattern.";
		return false;
	}
	const auto *folder = m_ePatternSelection == KOUKU_PATTERN_SELECTION::FOLDER
		? Find_Folder(m_Draft, m_strSelectedFolderId)
		: (m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN
				  ? Find_TimelineParent(m_Draft, m_strSelectedPatternId)
				  : nullptr);
	if (!folder)
	{
		status = "Select the destination Parent in Composition Actions, then choose a source in this Pattern Tree.";
		return false;
	}
	if (!folder->strLoadError.empty())
	{
		status = folder->strLoadError;
		return false;
	}
	const auto *timeline = Find_Pattern(m_Draft, folder->strTimelinePatternId);
	if (!folder->strTimelinePatternId.empty() && (!timeline || !timeline->strLoadError.empty()))
	{
		status = timeline ? timeline->strLoadError : "The destination Parent timeline is missing.";
		return false;
	}
	const auto *source = Find_Pattern(m_Draft, m_strAppendPatternId);
	if (!source)
	{
		status = "Choose a source Pattern in the Pattern Tree below.";
		return false;
	}
	if (!source->strLoadError.empty())
	{
		status = source->strLoadError;
		return false;
	}
	if (Find_TimelineParent(m_Draft, source->strPatternId) || !source->PatternOccurrences.empty())
	{
		status = "Choose an independent source Pattern; nested Parent timelines are not supported.";
		return false;
	}
	if (source->strGateId != folder->strGateId)
	{
		status = "Source and destination must belong to the same Gate.";
		return false;
	}
	if (timeline &&
		(source->strActorProfileId != timeline->strActorProfileId ||
			source->strTargetBossPlacementId != timeline->strTargetBossPlacementId))
	{
		status = "Source and destination must use the same model and target boss.";
		return false;
	}
	if (!Pattern_DurationMs(*source))
	{
		status = "The source Pattern needs a positive lifetime before it can be appended.";
		return false;
	}
	const auto parentDuration = timeline ? Pattern_DurationMs(*timeline) : 15000u;
	startMs = timeline ? m_iCursorMs : 0u;
	if (startMs >= parentDuration)
	{
		status = "Move the cursor before the Parent end or extend its full lifetime.";
		return false;
	}
	ownerId = timeline ? timeline->strPatternId : folder->strFolderId;
	durationMs = (std::min)(Pattern_DurationMs(*source), parentDuration - startMs);
	status.clear();
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Render_PatternResources()
{
	ImGui::SeparatorText("Pattern Resources");
	const auto *target = m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN
		? Find_Pattern(m_Draft, m_strSelectedPatternId)
		: nullptr;
	const auto *folder =
		m_ePatternSelection == KOUKU_PATTERN_SELECTION::FOLDER ? Find_Folder(m_Draft, m_strSelectedFolderId) : nullptr;
	ImGui::TextWrapped("Append target: %s",
		target ? target->strDisplayName.c_str()
			   : (folder ? folder->strDisplayName.c_str() : "Select a Parent in Composition Actions"));
	ImGui::TextWrapped("Selecting a source here keeps the current timeline, cursor and preview.");
	if (ImGui::BeginChild("##KoukuPatternResourceList", ImVec2(0.f, 360.f), ImGuiChildFlags_Borders))
		Render_PatternTree(true);
	ImGui::EndChild();
	const auto *source = Find_Pattern(m_Draft, m_strAppendPatternId);
	ImGui::TextWrapped("Source: %s", source ? source->strDisplayName.c_str() : "None");
	std::string ownerId, reason;
	std::uint32_t startMs = 0u, durationMs = 0u;
	const bool canAppend = Resolve_PatternResourceAppend(ownerId, startMs, durationMs, reason);
	ImGui::BeginDisabled(!canAppend);
	const bool append = ImGui::Button("Append Pattern at Cursor");
	ImGui::EndDisabled();
	if (source)
		ImGui::Text("Source lifetime: %u ms", Pattern_DurationMs(*source));
	if (canAppend)
	{
		ImGui::Text("Start %u ms | window %u ms | end %u ms", startMs, durationMs, startMs + durationMs);
		if (durationMs < Pattern_DurationMs(*source))
			ImGui::TextWrapped(
				"The source is longer than the remaining Parent lifetime. Extend the Parent lifetime to include the complete source.");
	}
	else
		ImGui::TextWrapped("%s", reason.c_str());
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	if (append && canAppend)
	{
		const std::string sourceId = m_strAppendPatternId;
		std::string status;
		(void)Append_PatternBox(ownerId, sourceId, startMs, durationMs, status);
		return;
	}
}

// draft 세대가 바뀌면 전체 Gate/Folder/Bundle/Pattern 관계를 표시 행으로 다시 만든다.
// 선택과 필터는 캐시 key가 아니며, 이 화면용 값 복사본이 문서 소유권이나 stable ID를 대신하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Rebuild_PatternTreeView()
{
	if (m_iPatternTreeViewGeneration == m_iDraftGeneration)
		return;
	Engine::CProfilerScope rebuildScope(CGameInstance::Get().Get_Profiler(), "ImGui.Composition.PatternTree.Rebuild");
	// 화면에 여러 번 등장하는 같은 Pattern도 별도 표시 행을 가진다. 문서 포인터 대신 label·stable ID 사본을 저장해 다음 프레임의 클릭을 다시 해석한다.
	std::vector<PATTERN_TREE_GATE> view;
	const auto append = [&](PATTERN_TREE_LEAVES &leaves, const KOUKU_SAYDON_COMPOSITION_PATTERN *pattern,
							const std::string &patternId, const std::string &widgetId, const std::string_view gate) {
		PATTERN_TREE_LEAF row;
		row.patternId = patternId;
		row.exists = pattern != nullptr;
		row.matchesGate = pattern && pattern->strGateId == gate;
		row.label = (pattern ? pattern->strDisplayName + " [" +
								Actor_Label(pattern->strActorProfileId, pattern->strGateId) + "]"
							 : patternId + " [Missing]") +
			"##" + widgetId;
		leaves.hasMultilineLabel |= row.label.find('\n') != std::string::npos;
		leaves.rows.push_back(std::move(row));
	};
	for (const auto *visibleGate : {"GATE1", "GATE2", "GATE3", "BINGO"})
	{
		PATTERN_TREE_GATE gate;
		gate.id = visibleGate;
		gate.label = std::string(Gate_Label(visibleGate)) + "##" + visibleGate;
		for (const auto &folder : m_Draft.Folders)
		{
			if (folder.strGateId != visibleGate)
				continue;
			PATTERN_TREE_FOLDER folderView;
			folderView.id = folder.strFolderId;
			folderView.label = folder.strDisplayName + " [Parent]##" + folder.strFolderId;
			folderView.timelinePatternId = folder.strTimelinePatternId;
			for (const auto &bundle : m_Draft.Bundles)
			{
				if (bundle.strFolderId != folder.strFolderId || bundle.strGateId != visibleGate)
					continue;
				PATTERN_TREE_BUNDLE bundleView;
				bundleView.id = bundle.strBundleId;
				bundleView.label = bundle.strDisplayName + " [Bundle]##" + bundle.strBundleId;
				for (const auto &member : bundle.Members)
					append(bundleView.leaves, Find_Pattern(m_Draft, member.strPatternId), member.strPatternId,
						member.strMemberId, visibleGate);
				folderView.bundles.push_back(std::move(bundleView));
			}
			for (const auto &pattern : m_Draft.Patterns)
				if (pattern.strFolderId == folder.strFolderId && pattern.strPatternId != folder.strTimelinePatternId &&
					pattern.strGateId == visibleGate)
					append(folderView.leaves, &pattern, pattern.strPatternId, pattern.strPatternId, visibleGate);
			gate.folders.push_back(std::move(folderView));
		}
		for (const auto &pattern : m_Draft.Patterns)
		{
			const auto *parent = Find_Folder(m_Draft, pattern.strFolderId);
			if (pattern.strGateId != visibleGate || (parent && parent->strGateId == pattern.strGateId))
				continue;
			const bool linked = std::any_of(m_Draft.Bundles.begin(), m_Draft.Bundles.end(), [&](const auto &bundle) {
				const auto *folder = Find_Folder(m_Draft, bundle.strFolderId);
				return bundle.strGateId == visibleGate && folder && folder->strGateId == visibleGate &&
					std::any_of(bundle.Members.begin(), bundle.Members.end(), [&](const auto &member) {
						return member.strPatternId == pattern.strPatternId;
					});
			});
			if (!linked)
				append(gate.leaves, &pattern, pattern.strPatternId, pattern.strPatternId, visibleGate);
		}
		view.push_back(std::move(gate));
	}
	m_PatternTreeView = std::move(view);
	m_iPatternTreeViewGeneration = m_iDraftGeneration;
}

// 현재 draft generation의 stable resource ID를 행 index로 찾는 캐시다. 캐시가 반환하는 포인터는 다음 문서 교체 전까지만 차용한다.
const Client::KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE *Client::CKoukuSaydonActionWorkbench::
	Find_DraftPresentationResource(const std::string &resourceId)
{
	if (m_iDraftPresentationIndexGeneration != m_iDraftGeneration)
	{
		Engine::CProfilerScope indexScope(
			CGameInstance::Get().Get_Profiler(), "ImGui.Composition.PresentationIndex.Rebuild");
		// 본문 vector는 문서의 값과 순서를 소유하고 이 map은 ID→index만 소유한다. 문서 세대가 바뀌면 index를 재구축해 예전 vector 위치를 새 행으로 착각하지 않는다.
		m_DraftPresentationResourceIndices.clear();
		m_DraftPresentationResourceIndices.reserve(m_Draft.PresentationResources.size());
		for (std::size_t i = 0u; i < m_Draft.PresentationResources.size(); ++i)
			m_DraftPresentationResourceIndices.emplace(m_Draft.PresentationResources[i].strResourceId, i);
		m_iDraftPresentationIndexGeneration = m_iDraftGeneration;
	}
	const auto found = m_DraftPresentationResourceIndices.find(resourceId);
	return found == m_DraftPresentationResourceIndices.end() ? nullptr : &m_Draft.PresentationResources[found->second];
}

// generation 기반 트리 행에 화면 선택/filter를 적용한다. 항목 선택/삭제/추가 명령은 stable ID로 해당 편집 함수에 전달한다.
void Client::CKoukuSaydonActionWorkbench::Render_PatternTree(const bool_t resourcePicker)
{
	Engine::CProfilerScope treeScope(CGameInstance::Get().Get_Profiler(), "ImGui.Composition.PatternTree");
	Rebuild_PatternTreeView();
	ImGui::PushID(resourcePicker ? "PatternResourceTree" : "PatternEditorTree");
	ImGui::SeparatorText("Pattern Tree");
	const auto &selectedId = resourcePicker ? m_strAppendPatternId : m_strSelectedPatternId;
	const auto selectPattern = [&](const std::string &patternId, const std::string &bundleId) {
		if (resourcePicker)
		{
			m_strAppendPatternId = patternId;
			return;
		}
		m_strBundleReturnId = bundleId;
		m_strCreateBundleId = bundleId;
		std::string status;
		(void)Select_PatternById(patternId, status);
	};
	const auto drawLeaves = [&](const PATTERN_TREE_LEAVES &leaves, const std::string &bundleId) {
		const auto drawRow = [&](const PATTERN_TREE_LEAF &row) {
			const bool highlight =
				!resourcePicker && !bundleId.empty() && row.exists && !m_strModelViewProfile.empty() && row.matchesGate;
			if (highlight)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.45f, .85f, 1.f, 1.f));
			if (ImGui::Selectable(row.label.c_str(), selectedId == row.patternId))
				selectPattern(row.patternId, bundleId);
			if (!resourcePicker && row.exists)
				Render_PatternDeleteContext(row.patternId);
			if (highlight)
				ImGui::PopStyleColor();
		};
		// 스크롤 후에도 열린 context menu가 원래 widget/ID를 유지하게 한다.
		// 행 높이가 다른 label이므로 고정 높이를 가정하는 list clipper를 쓰지 않는다.
		if (leaves.hasMultilineLabel ||
			ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
		{
			for (const auto &row : leaves.rows)
				drawRow(row);
			return;
		}
		ImGuiListClipper clipper;
		clipper.Begin(static_cast<int>(leaves.rows.size()), ImGui::GetTextLineHeightWithSpacing());
		while (clipper.Step())
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
				drawRow(leaves.rows[i]);
	};
	for (const auto &gate : m_PatternTreeView)
	{
		if (!resourcePicker && m_bSequenceWorkspace && m_strSelectedGateId != gate.id)
			continue;
		const bool gateOpen = ImGui::TreeNodeEx(gate.label.c_str(),
			ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow |
				(!resourcePicker && m_ePatternSelection == KOUKU_PATTERN_SELECTION::GATE &&
							m_strSelectedGateId == gate.id
						? ImGuiTreeNodeFlags_Selected
						: 0));
		if (!resourcePicker && ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			Select_Hierarchy(KOUKU_PATTERN_SELECTION::GATE, gate.id);
		if (!gateOpen)
			continue;
		if (!resourcePicker && !m_bSequenceWorkspace && gate.id == "BINGO" && Find_BingoFlow(m_Draft))
		{
			if (ImGui::Selectable("Bingo Complete [Parent]##BingoFlowRoot",
					m_ePatternSelection == KOUKU_PATTERN_SELECTION::BINGO_PARENT))
			{
				std::string status;
				(void)Select_BingoParent(status);
			}
			if (!ImGui::TreeNode("Authored Patterns##BingoInventory"))
			{
				ImGui::TreePop();
				continue;
			}
		}
		for (const auto &folder : gate.folders)
		{
			const auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen |
				(!resourcePicker &&
							((m_ePatternSelection == KOUKU_PATTERN_SELECTION::FOLDER &&
								 m_strSelectedFolderId == folder.id) ||
								(!folder.timelinePatternId.empty() &&
									m_strSelectedPatternId == folder.timelinePatternId))
						? ImGuiTreeNodeFlags_Selected
						: 0);
			const bool open = ImGui::TreeNodeEx(folder.label.c_str(), flags);
			if (!resourcePicker && ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
				Select_Hierarchy(KOUKU_PATTERN_SELECTION::FOLDER, folder.id);
			if (!open)
				continue;
			for (const auto &bundle : folder.bundles)
			{
				const bool childOpen = ImGui::TreeNodeEx(bundle.label.c_str(),
					ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen |
						(!resourcePicker && m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE &&
									m_strSelectedBundleId == bundle.id
								? ImGuiTreeNodeFlags_Selected
								: 0));
				if (!resourcePicker && ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
					Select_Hierarchy(KOUKU_PATTERN_SELECTION::BUNDLE, bundle.id);
				if (childOpen)
				{
					drawLeaves(bundle.leaves, bundle.id);
					ImGui::TreePop();
				}
			}
			drawLeaves(folder.leaves, {});
			ImGui::TreePop();
		}
		drawLeaves(gate.leaves, {});
		if (!resourcePicker && !m_bSequenceWorkspace && gate.id == "BINGO" && Find_BingoFlow(m_Draft))
			ImGui::TreePop();
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void Client::CKoukuSaydonActionWorkbench::Render_PatternsAndResources()
{
	Engine::CProfilerScope paneScope(CGameInstance::Get().Get_Profiler(), "ImGui.Composition.Patterns");
	ImGui::SeparatorText(m_bSequenceWorkspace ? "Sequences by Gate" : "Boss Actions");
	ImGui::BeginDisabled(Is_PublishRunning() || Is_Dirty());
	if (ImGui::Button("Reload Patterns"))
	{
		std::string status;
		(void)Reload(status);
	}
	ImGui::EndDisabled();
	if (!m_bHasDraft)
	{
		ImGui::TextWrapped("Patterns could not load: %s", m_strStatus.c_str());
		return;
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(Is_PublishRunning() || !m_bHasDraft || !Is_Dirty());
	if (ImGui::Button("Save##PatternsByGate"))
	{
		std::string status;
		(void)Save(status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextUnformatted(Is_Dirty() ? "Unsaved changes" : "Saved");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s",
			m_bSequenceWorkspace
				? "Save stores all sequences in this independent workspace."
				: "Save stores changes across all gates, parents, bundles and patterns. Publish All Patterns synchronizes F1 separately.");
	if (m_bSequenceWorkspace && ImGui::BeginCombo("Gate##Composition", Gate_Label(m_strSelectedGateId)))
	{
		for (const auto *gate : {"GATE1", "GATE2", "GATE3", "BINGO"})
			if (ImGui::Selectable(Gate_Label(gate), m_strSelectedGateId == gate))
				Select_Hierarchy(KOUKU_PATTERN_SELECTION::GATE, gate);
		ImGui::EndCombo();
	}
	if (m_bSequenceWorkspace &&
		ImGui::BeginCombo("Model View",
			m_strModelViewProfile.empty() ? "All" : Actor_Label(m_strModelViewProfile, m_strSelectedGateId)))
	{
		std::string status;
		if (ImGui::Selectable("All", m_strModelViewProfile.empty()))
			(void)Select_ActorProfile({}, status);
		for (const auto *actor : ACTOR_PROFILES)
			if (!CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(m_strSelectedGateId, actor).empty() &&
				ImGui::Selectable(Actor_Label(actor, m_strSelectedGateId), m_strModelViewProfile == actor))
				(void)Select_ActorProfile(actor, status);
		ImGui::EndCombo();
	}
	ImGui::BeginDisabled(Is_PublishRunning() || m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN ||
		!Find_Pattern(m_Draft, m_strSelectedPatternId));
	if (ImGui::Button(m_bSequenceWorkspace ? "Delete Selected Sequence..." : "Delete Selected Pattern..."))
		Request_PatternDelete(m_strSelectedPatternId);
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("%s",
			Is_PublishRunning() ? "Wait for publication to finish before deleting a Pattern."
				: (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN ||
					  !Find_Pattern(m_Draft, m_strSelectedPatternId))
				? "Select an individual Pattern in the tree. A Gate or Bundle selection does not select a Pattern for deletion."
				: "Review the selected Pattern and the links removed with it. Save stores the deletion.");
	if (ImGui::BeginChild("##KoukuPatternList", ImVec2(0.f, 480.f), ImGuiChildFlags_Borders))
	{
		Render_PatternTree(false);
	}
	ImGui::EndChild();
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::PATTERN)
	{
		if (const auto *item = Find_Pattern(m_Draft, m_strSelectedPatternId); item && item->strLoadError.empty())
			if (Render_RenameControl(RENAME_TARGET::PATTERN, item->strPatternId, item->strDisplayName))
				return;
	}
	else if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::FOLDER)
	{
		if (const auto *item = Find_Folder(m_Draft, m_strSelectedFolderId); item && item->strLoadError.empty())
			if (Render_RenameControl(RENAME_TARGET::FOLDER, item->strFolderId, item->strDisplayName))
				return;
	}
	else if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE)
	{
		if (const auto *item = Find_Bundle(m_Draft, m_strSelectedBundleId); item && item->strLoadError.empty())
			if (Render_RenameControl(RENAME_TARGET::BUNDLE, item->strBundleId, item->strDisplayName))
				return;
	}
	if (!m_bSequenceWorkspace)
	{
		ImGui::BeginDisabled(Is_PublishRunning() || !m_bHasDraft || Is_Dirty() || !m_Document.Is_Fresh());
		if (ImGui::Button("Publish All Patterns##PatternList"))
		{
			std::string status;
			(void)Publish_AllPatterns(status);
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (Render_AppendPatternButton())
			return;
		ImGui::TextWrapped(
			"Publishes every saved Parent, Bundle and Pattern across all gates. Unavailable items stay in the F1 tree with a reason; no individual publish selection is needed.");
		if (Is_Dirty())
			ImGui::TextDisabled("Save changes before Publish All Patterns.");
		ImGui::TextDisabled(
			"After publish, use Complete Play to run the new revision. Stop an active replay before starting the updated Pattern.");
	}
	ImGui::SeparatorText("Create Parent");
	ImGui::InputTextWithHint("##NewFolder", "Parent name", m_NewFolderName, std::size(m_NewFolderName));
	ImGui::BeginDisabled(!m_bHasDraft || m_NewFolderName[0] == '\0');
	if (ImGui::Button("Create Parent"))
		(void)Create_Hierarchy(false);
	ImGui::EndDisabled();
	ImGui::InputTextWithHint("##NewBundle", "Playback bundle name", m_NewBundleName, std::size(m_NewBundleName));
	ImGui::BeginDisabled(!Find_Folder(m_Draft, m_strSelectedFolderId) || m_NewBundleName[0] == '\0');
	if (ImGui::Button("Create Bundle"))
		(void)Create_Hierarchy(true);
	ImGui::EndDisabled();
	ImGui::SeparatorText(m_bSequenceWorkspace ? "Create Sequence" : "Create Pattern");
	ImGui::InputTextWithHint("##NewKoukuPattern",
		m_bSequenceWorkspace ? "Sequence display name" : "Pattern display name", m_NewPatternName,
		std::size(m_NewPatternName));
	if (ImGui::BeginCombo("Target Boss", Actor_Label(m_strCreateActorProfileId, m_strSelectedGateId)))
	{
		for (const auto *actor : ACTOR_PROFILES)
			if (!CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(m_strSelectedGateId, actor).empty() &&
				ImGui::Selectable(Actor_Label(actor, m_strSelectedGateId), m_strCreateActorProfileId == actor))
				m_strCreateActorProfileId = actor;
		ImGui::EndCombo();
	}
	const auto *parent = Find_Folder(m_Draft, m_strCreateFolderId);
	if (ImGui::BeginCombo("Parent##CreatePattern",
			parent ? parent->strDisplayName.c_str()
				   : (m_strCreateFolderId.empty() ? "None (Gate root)" : "Missing Parent")))
	{
		std::string status;
		if (ImGui::Selectable("None (Gate root)", m_strCreateFolderId.empty()))
			(void)Set_PatternCreationDestination({}, {}, status);
		for (const auto &folder : m_Draft.Folders)
			if (folder.strGateId == m_strSelectedGateId && folder.strLoadError.empty() &&
				ImGui::Selectable((folder.strDisplayName + "##" + folder.strFolderId).c_str(),
					folder.strFolderId == m_strCreateFolderId))
				(void)Set_PatternCreationDestination(folder.strFolderId, {}, status);
		ImGui::EndCombo();
	}
	const auto *destination = Find_Bundle(m_Draft, m_strCreateBundleId);
	if (ImGui::BeginCombo("Bundle", destination ? destination->strDisplayName.c_str() : "None (Independent)"))
	{
		std::string status;
		if (ImGui::Selectable("None (Independent)", m_strCreateBundleId.empty()))
			(void)Set_PatternCreationDestination(m_strCreateFolderId, {}, status);
		for (const auto &bundle : m_Draft.Bundles)
			if (bundle.strGateId == m_strSelectedGateId && bundle.strFolderId == m_strCreateFolderId &&
				bundle.strLoadError.empty() &&
				ImGui::Selectable((bundle.strDisplayName + "##" + bundle.strBundleId).c_str(),
					bundle.strBundleId == m_strCreateBundleId))
				(void)Set_PatternCreationDestination(m_strCreateFolderId, bundle.strBundleId, status);
		ImGui::EndCombo();
	}
	parent = Find_Folder(m_Draft, m_strCreateFolderId);
	destination = Find_Bundle(m_Draft, m_strCreateBundleId);
	ImGui::TextWrapped("Destination: %s / %s / %s", Gate_Label(m_strSelectedGateId),
		parent ? parent->strDisplayName.c_str() : "Gate root",
		destination ? destination->strDisplayName.c_str()
					: (m_bSequenceWorkspace ? "Independent Sequence" : "Independent Pattern"));
	ImGui::Combo("Category##CreatePattern", &m_iNewPatternCategory, PATTERN_CATEGORIES.data(),
		static_cast<int>(PATTERN_CATEGORIES.size()));
	ImGui::BeginDisabled(!m_bHasDraft || m_NewPatternName[0] == '\0');
	if (ImGui::Button(m_bSequenceWorkspace ? "Create Sequence###Create Pattern" : "Create Pattern"))
	{
		std::string id, status;
		if (Create_Pattern(m_NewPatternName, PATTERN_CATEGORIES[m_iNewPatternCategory], id, status))
			m_NewPatternName[0] = '\0';
	}
	ImGui::EndDisabled();
}

// Bundle의 member 시간과 공통 표현 행을 그린다. Bundle은 frame 시작에 값 복사하여 편집 뒤에도 순회할 수 있게 하고, 각 member의 Pattern은 필요한 순서에서 현재 draft로 다시 조회한다.
void Client::CKoukuSaydonActionWorkbench::Render_BundleTimeline()
{
	const auto *selected = Find_Bundle(m_Draft, m_strSelectedBundleId);
	if (!selected)
	{
		ImGui::TextDisabled("The selected bundle is unavailable.");
		return;
	}
	const auto bundle = *selected;
	ImGui::BeginDisabled(Is_PublishRunning() || !m_bHasDraft || !Is_Dirty());
	if (ImGui::Button("Save##BundleSequence"))
	{
		std::string status;
		(void)Save(status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	Render_BundleTransport();
	const auto duration = Bundle_DurationMs(m_Draft, bundle);
	ImGui::SliderFloat("Zoom##BundleSequence", &m_fPixelsPerSecond, 1.f, 500.f, "%.1f px/s");
	ImGui::SameLine();
	if (ImGui::Button("Fit##Bundle"))
		m_bFitRequested = true;
	const float labelWidth = 230.f;
	if (m_bFitRequested && duration > 0)
	{
		m_fPixelsPerSecond =
			(std::clamp)((ImGui::GetContentRegionAvail().x - labelWidth - 20.f) * 1000.f / duration, 1.f, 500.f);
		m_bFitRequested = false;
	}
	const float width = (std::max)(ImGui::GetContentRegionAvail().x - labelWidth - 20.f,
		(std::max)(duration, 1000u) * m_fPixelsPerSecond / 1000.f);
	const bool active = m_PreviewState.bPlaying && m_PreviewState.strPatternId == bundle.strBundleId;
	if (ImGui::BeginChild(
			"##BundleTimeline", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
	{
		const auto origin = ImGui::GetCursorScreenPos();
		const float timeX = origin.x + labelWidth;
		auto *draw = ImGui::GetWindowDrawList();
		ImGui::SetCursorScreenPos(ImVec2(timeX, origin.y));
		ImGui::InvisibleButton("##BundleRuler", ImVec2(width, 24));
		if (ImGui::IsItemActive() || ImGui::IsItemDeactivated())
		{
			m_iCursorMs = static_cast<std::uint32_t>(
				(std::clamp)((ImGui::GetIO().MousePos.x - timeX) * 1000.f / m_fPixelsPerSecond, 0.f, float(duration)));
			(void)Request_BundleScrub(m_iCursorMs);
		}
		const std::uint32_t step = m_fPixelsPerSecond < 30.f ? 5000u : 1000u;
		for (std::uint32_t tick = 0; tick <= (std::max)(duration, 1000u); tick += step)
		{
			const float x = timeX + tick * m_fPixelsPerSecond / 1000.f;
			draw->AddText(
				ImVec2(x + 2, origin.y + 2), IM_COL32(185, 195, 210, 255), (std::to_string(tick) + " ms").c_str());
		}
		float y = origin.y + 28;
		for (const auto &member : bundle.Members)
		{
			const auto *pattern = Find_Pattern(m_Draft, member.strPatternId);
			const auto name = pattern ? pattern->strDisplayName : member.strPatternId + " [Missing]";
			ImGui::PushID(member.strMemberId.c_str());
			ImGui::SetCursorScreenPos(ImVec2(origin.x, y));
			if (ImGui::Selectable(
					name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(labelWidth - 5, 24)) &&
				ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && pattern)
			{
				m_strBundleReturnId = bundle.strBundleId;
				std::string status;
				(void)Select_PatternById(member.strPatternId, status);
			}
			const float start = timeX + member.iStartOffsetMs * m_fPixelsPerSecond / 1000.f;
			const float span =
				(std::max)(6.f, (pattern ? Pattern_DurationMs(*pattern) : 0u) * m_fPixelsPerSecond / 1000.f);
			draw->AddRectFilled(ImVec2(timeX, y), ImVec2(timeX + width, y + 24), IM_COL32(29, 35, 43, 255));
			draw->AddRectFilled(ImVec2(start, y + 2), ImVec2(start + span, y + 22),
				pattern ? IM_COL32(60, 120, 177, 255) : IM_COL32(160, 60, 60, 255), 2.f);
			ImGui::SetCursorScreenPos(ImVec2(start, y + 2));
			ImGui::InvisibleButton("##MemberBar", ImVec2(span, 20));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s\nStart %u ms. Double-click to edit child. Drag to move start.",
					member.strPatternId.c_str(), member.iStartOffsetMs);
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && pattern)
			{
				m_strBundleReturnId = bundle.strBundleId;
				std::string status;
				(void)Select_PatternById(member.strPatternId, status);
			}
			if (ImGui::IsItemActivated())
			{
				m_strBundleDragMemberId = member.strMemberId;
				m_iDragOriginOffsetMs = member.iStartOffsetMs;
			}
			if (ImGui::IsItemDeactivated() && m_strBundleDragMemberId == member.strMemberId)
			{
				const auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x * 1000.f / m_fPixelsPerSecond;
				auto candidate = m_Draft;
				auto *edit = Find_Bundle(candidate, bundle.strBundleId);
				if (edit)
					for (auto &m : edit->Members)
						if (m.strMemberId == member.strMemberId)
							m.iStartOffsetMs = static_cast<std::uint32_t>(
								(std::clamp)(m_iDragOriginOffsetMs + delta, 0.f, float(MAX_EDITOR_TIME_MS)));
				if (edit && delta != 0)
				{
					edit->strAuthoringStatus = "DRAFT";
					std::string status;
					(void)Commit_Candidate(std::move(candidate), "Changed bundle start offset.", status);
				}
				m_strBundleDragMemberId.clear();
			}
			ImGui::PopID();
			y += 28;
		}
		const auto commonRow = [&](const auto &row, const char *label) {
			draw->AddText(ImVec2(origin.x, y + 3), IM_COL32(195, 200, 210, 255), label);
			const float x = timeX + row.iStartMs * m_fPixelsPerSecond / 1000.f;
			draw->AddRectFilled(ImVec2(x, y + 2),
				ImVec2(x + (std::max)(6.f, row.iDurationMs * m_fPixelsPerSecond / 1000.f), y + 22),
				IM_COL32(121, 83, 165, 255), 2.f);
			y += 28;
		};
		for (const auto &row : bundle.SceneProfileOccurrences)
			commonRow(row, "Scene Profile (Common)");
		for (const auto &row : bundle.PresentationOccurrences)
		{
			const float rowY = y;
			commonRow(row, "Camera (Common)");
			if (const auto *resource = Find_PresentationResource(m_Draft, row.strResourceId))
				if (const auto *shot = Find_AuthoringCamera(resource->strAssetId); shot && shot->iBlendOutMs)
				{
					const float tailX = timeX + (row.iStartMs + row.iDurationMs) * m_fPixelsPerSecond / 1000.f;
					draw->AddRectFilled(ImVec2(tailX, rowY + 5),
						ImVec2(tailX + shot->iBlendOutMs * m_fPixelsPerSecond / 1000.f, rowY + 19),
						IM_COL32(121, 83, 165, 70));
					draw->AddText(ImVec2(tailX + 2, rowY + 3), IM_COL32(195, 170, 220, 255), "return");
				}
		}
		const float cursorX = timeX + (active ? m_PreviewState.iClockMs : m_iCursorMs) * m_fPixelsPerSecond / 1000.f;
		draw->AddLine(ImVec2(cursorX, origin.y), ImVec2(cursorX, y + 8), IM_COL32(250, 190, 70, 255), 2);
		ImGui::SetCursorScreenPos(ImVec2(origin.x, y + 8));
		ImGui::Dummy(ImVec2(labelWidth + width, 4));
	}
	ImGui::EndChild();
}

void Client::CKoukuSaydonActionWorkbench::Render_HierarchyDetails()
{
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::GATE)
	{
		ImGui::Text("%s", Gate_Label(m_strSelectedGateId));
		ImGui::TextWrapped("Create a parent and playback bundle, or create an independent Pattern.");
		return;
	}
	const auto *folder = Find_Folder(m_Draft, m_strSelectedFolderId);
	const auto *bundlePtr = Find_Bundle(m_Draft, m_strSelectedBundleId);
	const bool isBundle = m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE;
	if ((isBundle && !bundlePtr) || (!isBundle && !folder))
	{
		ImGui::TextDisabled("Selection is no longer in the document.");
		return;
	}
	const auto id = isBundle ? bundlePtr->strBundleId : folder->strFolderId;
	const auto error = isBundle ? bundlePtr->strLoadError : folder->strLoadError;
	ImGui::SeparatorText(isBundle ? "Playback Bundle" : "Parent Folder");
	ImGui::TextWrapped("%s | %s", Gate_Label(m_strSelectedGateId), id.c_str());
	if (!error.empty())
	{
		ImGui::TextWrapped("%s", error.c_str());
		ImGui::TextDisabled("Original JSON preserved. Repair and Reload.");
		return;
	}
	if (ImGui::InputText("Display Name##Hierarchy", m_HierarchyName, std::size(m_HierarchyName),
			ImGuiInputTextFlags_EnterReturnsTrue))
	{
		auto candidate = m_Draft;
		if (isBundle)
			Find_Bundle(candidate, id)->strDisplayName = m_HierarchyName;
		else
			Find_Folder(candidate, id)->strDisplayName = m_HierarchyName;
		std::string status;
		(void)Commit_Candidate(std::move(candidate), "Renamed hierarchy item.", status);
		return;
	}
	if (!isBundle)
	{
		ImGui::TextWrapped("Add a timeline to keep common rows running while complete child Patterns play.");
		if (ImGui::Button(folder->strTimelinePatternId.empty() ? "Create Parent Timeline" : "Open Parent Timeline"))
		{
			std::string status;
			(void)Create_ParentTimeline(id, status);
			return;
		}
		bool hasChildren = false;
		for (const auto &bundle : m_Draft.Bundles)
			if (bundle.strFolderId == id)
			{
				hasChildren = true;
				if (ImGui::Selectable((bundle.strDisplayName + "##Detail" + bundle.strBundleId).c_str()))
				{
					Select_Hierarchy(KOUKU_PATTERN_SELECTION::BUNDLE, bundle.strBundleId);
					return;
				}
			}
		for (const auto &pattern : m_Draft.Patterns)
			if (pattern.strFolderId == id)
			{
				hasChildren = true;
				if (ImGui::Selectable((pattern.strDisplayName + " [Pattern]##Detail" + pattern.strPatternId).c_str()))
				{
					m_strBundleReturnId.clear();
					std::string status;
					(void)Select_PatternById(pattern.strPatternId, status);
					return;
				}
			}
		ImGui::BeginDisabled(hasChildren);
		if (ImGui::Button("Delete Empty Parent"))
		{
			auto candidate = m_Draft;
			std::erase_if(candidate.Folders, [&](const auto &item) {
				return item.strFolderId == id;
			});
			std::string status;
			if (Commit_Candidate(std::move(candidate), "Deleted empty parent.", status))
				Select_Hierarchy(KOUKU_PATTERN_SELECTION::GATE, m_strSelectedGateId);
		}
		ImGui::EndDisabled();
		if (hasChildren)
			ImGui::TextDisabled("Move or remove child bundles and Patterns before deleting this parent.");
		return;
	}
	const auto bundle = *bundlePtr;
	if (ImGui::BeginCombo("Parent Folder", folder ? folder->strDisplayName.c_str() : "Missing"))
	{
		std::string destination;
		for (const auto &item : m_Draft.Folders)
			if (item.strGateId == bundle.strGateId &&
				ImGui::Selectable(item.strDisplayName.c_str(), item.strFolderId == bundle.strFolderId))
				destination = item.strFolderId;
		ImGui::EndCombo();
		if (!destination.empty())
		{
			auto candidate = m_Draft;
			Find_Bundle(candidate, id)->strFolderId = destination;
			std::string status;
			if (Commit_Candidate(std::move(candidate), "Moved bundle to parent.", status))
			{
				m_strSelectedFolderId = destination;
				if (m_strCreateBundleId == id)
					m_strCreateFolderId = destination;
			}
			return;
		}
	}
	if (!m_bSequenceWorkspace)
		ImGui::TextDisabled("Publish All Patterns checks this bundle and its children together.");
	ImGui::SeparatorText(m_bSequenceWorkspace ? "Connected Sequences" : "Connected Patterns");
	for (const auto &member : bundle.Members)
	{
		const auto *pattern = Find_Pattern(m_Draft, member.strPatternId);
		ImGui::PushID(member.strMemberId.c_str());
		if (ImGui::Selectable(pattern ? pattern->strDisplayName.c_str() : member.strPatternId.c_str()))
		{
			m_strBundleReturnId = id;
			std::string status;
			(void)Select_PatternById(member.strPatternId, status);
			ImGui::PopID();
			return;
		}
		if (pattern)
			ImGui::TextWrapped("%s | %s", Actor_Label(pattern->strActorProfileId, pattern->strGateId),
				pattern->strTargetBossPlacementId.c_str());
		else
			ImGui::TextColored(ImVec4(1, .35f, .35f, 1), "Missing child; bundle playback is unavailable.");
		int offset = static_cast<int>(member.iStartOffsetMs);
		if (ImGui::InputInt("Start ms", &offset))
		{
			auto candidate = m_Draft;
			auto *edit = Find_Bundle(candidate, id);
			for (auto &item : edit->Members)
				if (item.strMemberId == member.strMemberId)
					item.iStartOffsetMs = static_cast<std::uint32_t>((std::clamp)(offset, 0, int(MAX_EDITOR_TIME_MS)));
			edit->strAuthoringStatus = "DRAFT";
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Changed child start offset.", status);
			ImGui::PopID();
			return;
		}
		const auto ticks = (std::uint64_t(member.iStartOffsetMs) * m_Draft.iFixedTickHz + 999u) / 1000u;
		ImGui::TextDisabled("Scheduled tick +%llu (%.2f ms)", static_cast<unsigned long long>(ticks),
			ticks * 1000.0 / m_Draft.iFixedTickHz);
		if (ImGui::SmallButton("Remove From Bundle"))
		{
			auto candidate = m_Draft;
			auto *edit = Find_Bundle(candidate, id);
			std::erase_if(edit->Members, [&](const auto &item) {
				return item.strMemberId == member.strMemberId;
			});
			edit->strAuthoringStatus = "DRAFT";
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Removed link; original Pattern preserved.", status);
			ImGui::PopID();
			return;
		}
		ImGui::PopID();
		ImGui::Separator();
	}
	if (ImGui::BeginCombo(
			m_bSequenceWorkspace ? "Link Existing Sequence###Link Existing Pattern" : "Link Existing Pattern",
			m_bSequenceWorkspace ? "Choose a Sequence from this Gate" : "Choose a Pattern from this Gate"))
	{
		std::string target;
		for (const auto &pattern : m_Draft.Patterns)
			if (pattern.strGateId == bundle.strGateId &&
				ImGui::Selectable((pattern.strDisplayName + " [" +
					Actor_Label(pattern.strActorProfileId, pattern.strGateId) + "]##" + pattern.strPatternId)
						.c_str()))
				target = pattern.strPatternId;
		ImGui::EndCombo();
		if (!target.empty())
		{
			(void)Link_BundlePattern(target);
			return;
		}
	}
	Render_BundleCommonDetails();
	ImGui::BeginDisabled(
		!bundle.Members.empty() || !bundle.SceneProfileOccurrences.empty() || !bundle.PresentationOccurrences.empty());
	if (ImGui::Button("Delete Empty Bundle"))
	{
		auto candidate = m_Draft;
		std::erase_if(candidate.Bundles, [&](const auto &item) {
			return item.strBundleId == id;
		});
		std::string status;
		if (Commit_Candidate(std::move(candidate), "Deleted empty bundle.", status))
			Select_Hierarchy(KOUKU_PATTERN_SELECTION::FOLDER, bundle.strFolderId);
	}
	ImGui::EndDisabled();
}

void Client::CKoukuSaydonActionWorkbench::Render_BundleCommonDetails()
{
	const auto *source = Find_Bundle(m_Draft, m_strSelectedBundleId);
	if (!source)
		return;
	const auto bundle = *source;
	ImGui::SeparatorText("Common Scene Profile / Camera");
	ImGui::TextWrapped(
		"Common lanes run once on the bundle clock. Child global lanes must be moved here before publishing the bundle.");
	for (const auto &row : bundle.SceneProfileOccurrences)
	{
		ImGui::PushID(row.strOccurrenceId.c_str());
		const auto *resource = Find_SceneProfile(m_Draft, row.strSceneProfileId);
		ImGui::TextWrapped("Scene: %s", resource ? resource->strDisplayName.c_str() : row.strSceneProfileId.c_str());
		int start = int(row.iStartMs), length = int(row.iDurationMs), blend = int(row.iBlendMs);
		bool changed = ImGui::InputInt("Start ms", &start);
		changed |= ImGui::InputInt("Duration ms", &length);
		changed |= ImGui::InputInt("Blend metadata ms", &blend);
		const bool remove = ImGui::SmallButton("Remove Scene");
		ImGui::PopID();
		if (changed || remove)
		{
			auto candidate = m_Draft;
			auto *edit = Find_Bundle(candidate, bundle.strBundleId);
			edit->strAuthoringStatus = "DRAFT";
			if (remove)
				std::erase_if(edit->SceneProfileOccurrences, [&](const auto &item) {
					return item.strOccurrenceId == row.strOccurrenceId;
				});
			else
				for (auto &item : edit->SceneProfileOccurrences)
					if (item.strOccurrenceId == row.strOccurrenceId)
					{
						item.iStartMs = (std::clamp)(start, 0, int(MAX_EDITOR_TIME_MS));
						item.iDurationMs = (std::clamp)(length, 1, int(MAX_EDITOR_TIME_MS));
						item.iBlendMs = (std::clamp)(blend, 0, int(MAX_EDITOR_TIME_MS));
					}
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated common Scene Profile.", status);
			return;
		}
	}
	for (const auto &row : bundle.PresentationOccurrences)
	{
		ImGui::PushID(row.strOccurrenceId.c_str());
		const auto *resource = Find_PresentationResource(m_Draft, row.strResourceId);
		ImGui::TextWrapped("Camera: %s", resource ? resource->strDisplayName.c_str() : row.strResourceId.c_str());
		if (resource)
			Render_CameraAuthoring(resource->strAssetId);
		auto value = row;
		int start = int(row.iStartMs), length = int(row.iDurationMs);
		bool changed = ImGui::InputInt("Start ms", &start);
		changed |= ImGui::InputInt("Duration ms", &length);
		float offset[3] = {float(row.PositionOffset[0]), float(row.PositionOffset[1]), float(row.PositionOffset[2])};
		if (ImGui::DragFloat3("Camera offset", offset, .05f))
		{
			for (int i = 0; i < 3; ++i)
				value.PositionOffset[i] = offset[i];
			changed = true;
		}
		const bool remove = ImGui::SmallButton("Remove Camera");
		ImGui::PopID();
		if (changed || remove)
		{
			auto candidate = m_Draft;
			auto *edit = Find_Bundle(candidate, bundle.strBundleId);
			edit->strAuthoringStatus = "DRAFT";
			if (remove)
				std::erase_if(edit->PresentationOccurrences, [&](const auto &item) {
					return item.strOccurrenceId == row.strOccurrenceId;
				});
			else
				for (auto &item : edit->PresentationOccurrences)
					if (item.strOccurrenceId == row.strOccurrenceId)
					{
						value.iStartMs = (std::clamp)(start, 0, int(MAX_EDITOR_TIME_MS));
						value.iDurationMs = (std::clamp)(length, 1, int(MAX_EDITOR_TIME_MS));
						item = value;
					}
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated common Camera.", status);
			return;
		}
	}
	if (ImGui::BeginCombo("Append Scene Profile", "Choose a saved resource"))
	{
		std::string chosen;
		for (const auto &resource : m_Draft.SceneProfiles)
			if (ImGui::Selectable(resource.strDisplayName.c_str()))
				chosen = resource.strSceneProfileId;
		ImGui::EndCombo();
		if (!chosen.empty())
		{
			const auto duration = Bundle_DurationMs(m_Draft, bundle);
			std::string occurrenceId, status;
			(void)Append_SceneProfileBox(
				{}, chosen, m_iCursorMs, duration > m_iCursorMs ? duration - m_iCursorMs : 1000u, occurrenceId, status);
			return;
		}
	}
	if (ImGui::BeginCombo("Append Camera", "Choose a saved resource"))
	{
		std::string chosen;
		for (const auto &resource : m_Draft.PresentationResources)
			if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA &&
				ImGui::Selectable(resource.strDisplayName.c_str()))
				chosen = resource.strResourceId;
		ImGui::EndCombo();
		if (!chosen.empty())
		{
			std::string status;
			(void)Append_PresentationBox(chosen, status);
		}
	}
}

// 명시 lifetime의 최솟값은 Stage 누적 길이와 Logic/Summon/Scene/Presentation 끝, World 시작점 다음 ms로 계산한다. 빈 일반 Pattern은 첫 Stage를 만들며 나머지 후보는 공용 검증·부모 시간 조정 경계로 보낸다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_PatternDuration(
	const std::string_view patternId, const std::uint32_t durationMs, std::string &outStatus)
{
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Full lifetime requires an editable Pattern.";
		return false;
	}
	if (pattern->Stages.empty() && !pattern->iDurationMs && !Find_TimelineParent(candidate, patternId))
	{
		std::string stageId;
		return Add_Stage(patternId, "ACTIVE", durationMs, stageId, outStatus);
	}
	std::uint64_t stageEndMs = 0u;
	for (const auto &stage : pattern->Stages)
		stageEndMs += stage.iDurationMs;
	const auto minimumMs = (std::max)({std::uint64_t{1u}, stageEndMs, std::uint64_t(Pattern_LaneEndMs(*pattern)),
		std::uint64_t(Pattern_LogicEndMs(*pattern)), std::uint64_t(Pattern_SummonEndMs(*pattern))});
	if (durationMs < minimumMs || durationMs > MAX_EDITOR_TIME_MS)
	{
		outStatus = m_strStatus = "Timeline lifetime must cover the existing rows (" + std::to_string(minimumMs) +
			"..600000 ms). Resize a Stage to change boss progression.";
		return false;
	}
	pattern->iDurationMs = durationMs;
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Updated row lifetime; Stage clocks and next-pattern timing are unchanged. Press Save.", outStatus))
		return false;
	m_bFitRequested = true;
	return true;
}
