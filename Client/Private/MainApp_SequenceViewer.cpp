#include "imgui.h"
#include "MainApp.h"

#ifdef _DEBUG
#include "DataJson.h"
#include "GameInstance.h"
#include "MapAssetCatalog.h"
#include "MapEditorWorkspaceService.h"
#include "MapTool.h"
#include "WorldGameplayDocument.h"
#include "Level_KakulSaydonArena.h"
#include "Level_ValtanArena.h"
#include "PlayerCommandSink.h"
#include "KoukuSaydonBossTool.h"
#include "ValtanBossTool.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <unordered_map>

using namespace Client;
namespace
{
constexpr const char* AREAS[] = { "LV_LUT_MIDNIGHTC_ED", "LV_LUT_HEARTRB_ED" };
constexpr const char* AREA_NAMES[] = { "쿠크세이튼", "발탄" };
constexpr const char* KINDS[] = { "트리거", "맵 시퀀스", "보스 패턴" };
using PlaybackOp = LostArk::Shared::DEBUG_WORLD_PLAYBACK_OPERATION;
using PlaybackResult = LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT;

std::string JsonText(const DATA_JSON_VALUE& value, const char* key)
{
	const auto* field = value.Find(key);
	return field && field->Is_String() ? field->Get_String() : std::string{};
}
std::string SearchText(std::string value)
{
	// ASCII folding leaves UTF-8 Hangul bytes unchanged.
	for (auto& ch : value) if (static_cast<unsigned char>(ch) < 128) ch = static_cast<char>(std::tolower(ch));
	return value;
}
std::string LocationName(const std::string& id, const int area)
{
	const auto key = SearchText(id);
	for (int stage = 1; stage <= 4; ++stage)
		if (key.find("mario" + std::to_string(stage)) != std::string::npos ||
			key.find("mario_m" + std::to_string(stage)) != std::string::npos)
			return std::to_string(stage) + "마리오";
	return AREA_NAMES[area];
}
std::string FriendlyName(const std::string& name)
{
	const auto key = SearchText(name);
	if (key == "paper_2") return "종이무대 펼침 | " + name;
	if (key == "lever_on") return "레버 당기기 | " + name;
	if (key == "lever_off") return "레버 되돌리기 | " + name;
	return name;
}
const char* ResultText(const PlaybackResult result)
{
	switch (result)
	{
	case PlaybackResult::ACCEPTED: return "서버가 실행을 승인했습니다. 같은 아레나 참가자에게 전달됩니다.";
	case PlaybackResult::DISABLED: return "서버의 Debug 기능이 비활성화되어 있습니다.";
	case PlaybackResult::WRONG_WORLD: return "선택한 보스의 아레나에 입장해야 합니다.";
	case PlaybackResult::INVALID_TARGET: return "서버 배포 데이터에 없거나 비활성화된 항목입니다. Publish 후 서버를 재시작해 주세요.";
	case PlaybackResult::INVALID_PLAYER: return "플레이어가 없거나 사망/강제 이동 중입니다.";
	case PlaybackResult::ALREADY_USED: return "한 번 실행된 트리거입니다. Replay로 다시 요청할 수 있습니다.";
	case PlaybackResult::ACTION_REJECTED: return "동작 실행이 거절됐습니다. 위치·대상·이미 활성화된 소환 상태를 확인해 주세요.";
	case PlaybackResult::STALE_REQUEST: return "중복되거나 오래된 요청입니다.";
	default: return "알 수 없는 실행 결과입니다.";
	}
}
}

void CMainApp::RefreshSequenceViewer()
{
	m_bSequenceViewerLoaded = true;
	m_iSequenceViewerPendingEditorAction = -1;
	DATA_JSON_VALUE labels;
	std::string labelsError;
	auto labelsPath = CMapAssetCatalog::Get_MapAuthoringRoot() / "SequenceViewer.labels.json";
	std::error_code labelsFileError;
	if (!std::filesystem::is_regular_file(labelsPath, labelsFileError))
		labelsPath = CMapAssetCatalog::Get_MapDataRoot().parent_path() / "World" / "SequenceViewer.labels.json";
	if (std::filesystem::is_regular_file(labelsPath, labelsFileError))
	{
		const auto size = std::filesystem::file_size(labelsPath, labelsFileError);
		std::ifstream stream(labelsPath, std::ios::binary);
		if (labelsFileError || size > 1024u * 1024u || !stream ||
			!CDataJson::Parse(std::string(std::istreambuf_iterator<char>(stream), {}), labels, labelsError) ||
			JsonText(labels, "schema") != "lostark.sequence-viewer-labels" ||
			!labels.Find("formatVersion") || !labels.Find("formatVersion")->Is_Number() ||
			labels.Find("formatVersion")->Get_Number() != 1.0 ||
			!labels.Find("entries") || !labels.Find("entries")->Is_Array())
		{
			labels = {};
			labelsError = "표시 이름 파일 오류: 원본 ID로 표시합니다. " + labelsError;
		}
	}
	for (int area = 0; area < 2; ++area)
	{
		std::vector<SEQUENCE_VIEWER_ROW> staged;
		std::string report, error;
		CWorldGameplayDocument gameplay;
		auto path = CMapAssetCatalog::Get_MapAuthoringRoot().parent_path() /
			L"Worlds" / AREAS[area] / L"Gameplay.world.json";
		std::error_code sourceError;
		if (!std::filesystem::is_regular_file(path, sourceError))
			path = CMapAssetCatalog::Get_MapDataRoot().parent_path() / "World" /
				(std::string(AREAS[area]) + ".viewer.world.json");
		if (gameplay.Load(path, AREAS[area], error))
		{
			for (const auto& trigger : gameplay.Get_Placements())
			{
				if (trigger.eKind != WORLD_PLACEMENT_KIND::TRIGGER_BOX) continue;
				SEQUENCE_VIEWER_ROW row;
				row.id = trigger.placementId; row.name = trigger.placementId;
				row.location = LocationName(row.id, area);
				row.position = trigger.position; row.hasPosition = true;
				row.enabled = trigger.isEnabled;
				if (trigger.triggerEvents.size() != 1)
				{ row.enabled = false; row.error = "지원되는 동작 한 개가 필요합니다."; }
				else
				{
					const auto& event = trigger.triggerEvents.front();
					switch (event.eKind)
					{
					case WORLD_TRIGGER_EVENT_KIND::PLAY_SEQUENCE:
						row.action = "연출 재생"; row.sequenceId = event.targetId; break;
					case WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER: row.action = "플레이어 이동 / 점프"; break;
					case WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL: row.action = "다른 맵으로 이동"; break;
					case WORLD_TRIGGER_EVENT_KIND::ACTIVATE_SPAWN_GROUP: row.action = "몬스터 그룹 소환"; break;
					case WORLD_TRIGGER_EVENT_KIND::ACTIVATE_ENCOUNTER: row.action = "보스 전투 활성화"; break;
					default: row.enabled = false; row.error = "지원하지 않는 트리거 동작"; break;
					}
					row.related = event.targetId;
				}
				if (!trigger.isEnabled) row.error = "저작 데이터에서 비활성화되어 있습니다.";
				staged.push_back(std::move(row));
			}
		}
		else
		{
			report = "트리거 읽기 실패 (이전 목록 유지): " + error;
			for (const auto& previous : m_SequenceViewerRows[area])
				if (previous.kind == 0) staged.push_back(previous);
		}

		// Inventory only: actual playback is revalidated by the existing player/server.
		auto sequencePath = CMapAssetCatalog::Get_MapAuthoringRoot() / "Authoring" / AREAS[area] /
			(std::string(AREAS[area]) + ".worldsequences.json");
		std::error_code ec;
		if (!std::filesystem::is_regular_file(sequencePath, ec))
			sequencePath = CMapAssetCatalog::Get_MapDataRoot() / (std::string(AREAS[area]) + ".worldsequences.json");
		bool sequencesLoaded = false;
		if (std::filesystem::is_regular_file(sequencePath, ec))
		{
			DATA_JSON_VALUE document;
			const auto bytes = std::filesystem::file_size(sequencePath, ec);
			std::ifstream input(sequencePath, std::ios::binary);
			std::string text;
			if (!ec && bytes <= 16u * 1024u * 1024u && input)
				text.assign(std::istreambuf_iterator<char>(input), {});
			if (CDataJson::Parse(text, document, error) && JsonText(document, "areaId") == AREAS[area] &&
				document.Find("instances") && document.Find("instances")->Is_Array() &&
				document.Find("templates") && document.Find("templates")->Is_Array())
			{
				std::unordered_map<std::string, std::string> names;
				sequencesLoaded = true;
				for (const auto& item : document.Find("templates")->Get_Array())
					names.emplace(JsonText(item, "sequenceId"), FriendlyName(JsonText(item, "displayName")));
				for (const auto& item : document.Find("instances")->Get_Array())
				{
					SEQUENCE_VIEWER_ROW row; row.kind = 1;
					row.id = JsonText(item, "instanceId"); row.sequenceId = row.id;
					const auto found = names.find(JsonText(item, "templateId"));
					row.name = found == names.end() ? row.id : found->second;
					row.location = LocationName(row.id, area); row.action = "맵 연출 재생";
					const auto* enabled = item.Find("enabled");
					row.enabled = enabled && enabled->Is_Boolean() && enabled->Get_Boolean() && found != names.end();
					if (!row.enabled) row.error = "비활성화되었거나 템플릿 연결이 없습니다.";
					for (const auto& trigger : staged)
						if (trigger.kind == 0 && trigger.sequenceId == row.id)
						{
							if (!row.hasPosition) { row.position = trigger.position; row.location = trigger.location; row.hasPosition = true; }
							if (!row.related.empty()) row.related += ", ";
							row.related += trigger.id;
						}
					if (row.related.empty()) row.related = "연결된 트리거 없음 (시퀀스 단독 재생)";
					staged.push_back(std::move(row));
				}
			}
			else report += "\n시퀀스 읽기 실패: " + error;
		}
		if (!sequencesLoaded)
			for (const auto& previous : m_SequenceViewerRows[area])
				if (previous.kind == 1) staged.push_back(previous);
		if (area == 1)
		{
			RefreshCompletePlayPatternOptions();
			for (size_t i = 0; i < m_CompletePlayPatternIds.size(); ++i)
			{
				SEQUENCE_VIEWER_ROW row; row.kind = 2; row.id = m_CompletePlayPatternIds[i];
				row.name = m_CompletePlayPatternLabels[i]; row.location = "발탄 보스 아레나";
				row.action = "서버 보스 패턴 / 연출"; staged.push_back(std::move(row));
			}
		}
		else
		{
			if (!m_pKoukuSaydonBossTool) m_pKoukuSaydonBossTool = std::make_unique<CKoukuSaydonBossTool>();
			if (!m_pKoukuSaydonBossTool->Reload(error)) report += "\n쿠크 보스 목록: " + error;
			for (const auto& pattern : m_pKoukuSaydonBossTool->Get_ProductPatterns())
			{
				SEQUENCE_VIEWER_ROW row; row.kind = 2; row.id = pattern.strPatternId;
				row.name = pattern.strDisplayName; row.location = pattern.strCategory;
				row.action = "서버 보스 패턴 / 연출"; row.error = pattern.strLoadError;
				row.enabled = row.error.empty(); staged.push_back(std::move(row));
			}
		}
		if (const auto* entries = labels.Find("entries"))
			for (const auto& label : entries->Get_Array())
			{
				if (JsonText(label, "areaId") != AREAS[area]) continue;
				const auto kind = JsonText(label, "kind");
				const int kindIndex = kind == "trigger" ? 0 : kind == "sequence" ? 1 : kind == "pattern" ? 2 : -1;
				for (auto& row : staged)
					if (row.kind == kindIndex && row.id == JsonText(label, "targetId"))
					{
						const auto name = JsonText(label, "displayName"), location = JsonText(label, "location");
						if (!name.empty()) row.name = name;
						if (!location.empty()) row.location = location;
					}
			}
		if (!labelsError.empty()) report += "\n" + labelsError;
		if (report.empty() || !staged.empty()) m_SequenceViewerRows[area] = std::move(staged);
		m_SequenceViewerLoadStatus[area] = report;
	}
}

void CMainApp::ExecuteSequenceViewerAction(const int action)
{
	const int area = m_iSequenceViewerArea;
	const auto& rows = m_SequenceViewerRows[area];
	const auto found = std::find_if(rows.begin(), rows.end(), [&](const auto& row)
		{ return std::to_string(row.kind) + ":" + row.id == m_SequenceViewerSelection; });
	if (found == rows.end()) return;
	const auto& row = *found;
	auto* kouku = CLevel_KakulSaydonArena::Get_Active();
	auto* valtan = CLevel_ValtanArena::Get_Active();
	const auto sink = area == 0 ? (kouku ? kouku->Get_PlayerCommandSink() : nullptr) :
		(valtan ? valtan->Get_PlayerCommandSink() : nullptr);
	const bool editor = CMapEditorWorkspaceService::Is_Active() &&
		CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::DEVELOPMENT);
	// 0 play, 1 replay, 2 stop, 3 editor, 4 go to trigger.
	if (action == 3 && row.kind == 2)
	{
		const auto tool = area ? DEBUG_TOOL::VALTAN_BOSS : DEBUG_TOOL::KOUKU_SAYDON_BOSS;
		if (SUCCEEDED(EnsureDebugTool(tool))) SetDebugToolVisible(tool, true);
		return;
	}
	if (editor && row.kind != 2)
	{
		if (FAILED(EnsureDebugTool(DEBUG_TOOL::MAP)) || !m_pMapTool)
		{ m_SequenceViewerStatus = "Map Tool을 준비하지 못했습니다."; return; }
		SetDebugToolVisible(DEBUG_TOOL::MAP, true);
		const auto result = m_pMapTool->Debug_SequenceViewer(AREAS[area], row.sequenceId,
			row.kind == 0 ? row.id : "", action <= 1, action == 2,
			action == 4 && row.hasPosition ? &row.position : nullptr, m_SequenceViewerStatus);
		if (!result)
		{
			if (m_iSequenceViewerPendingEditorAction < 0)
				m_SequenceViewerPendingDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(60);
			m_iSequenceViewerPendingEditorAction = action;
		}
		else m_iSequenceViewerPendingEditorAction = -1;
		return;
	}
	if (action == 3)
	{ m_SequenceViewerStatus = "맵 세부 편집은 Lobby > Test에서 이 항목의 Open Editor를 눌러 주세요."; return; }
	if (!sink)
	{ m_SequenceViewerStatus = "해당 아레나 입장이 필요합니다. Enter Arena를 눌러 주세요."; return; }
	if (action == 4 && row.hasPosition)
	{
		const bool ok = area == 0 ? kouku->Get_DebugPlayerController().Request_DebugTeleportToPosition(
			LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA, row.position.x, row.position.y, row.position.z) :
			valtan->Get_DebugPlayerController().Request_DebugTeleportToPosition(
			LostArk::Shared::WORLD_ID::VALTAN_ARENA, row.position.x, row.position.y, row.position.z);
		m_SequenceViewerStatus = ok ? "트리거 위치로 이동을 요청했습니다. 도착하면 트리거가 발동할 수 있습니다." : "서버 이동 요청을 보내지 못했습니다.";
		return;
	}
	if (row.kind == 2)
	{
		if (area == 0) m_pKoukuSaydonBossTool->Play_PatternById(row.id,
			m_pKoukuSaydonBossTool->Get_SourceRevision(), m_SequenceViewerStatus);
		else if (action == 1) m_pValtanBossTool->Restart_ServerPattern(row.id, m_SequenceViewerStatus);
		else m_pValtanBossTool->Play_ServerPattern(row.id, m_SequenceViewerStatus);
		return;
	}
	LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK request{};
	request.iRequestSequence = ++m_iSequenceViewerRequest;
	request.eWorldId = area == 0 ? LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA : LostArk::Shared::WORLD_ID::VALTAN_ARENA;
	request.strTargetId = action == 2 ? row.sequenceId : row.id;
	request.eOperation = action == 2 ? PlaybackOp::STOP_SEQUENCE : row.kind == 0 ?
		(action == 1 ? PlaybackOp::REPLAY_TRIGGER : PlaybackOp::PLAY_TRIGGER) :
		(action == 1 ? PlaybackOp::REPLAY_SEQUENCE : PlaybackOp::PLAY_SEQUENCE);
	const bool sent = sink->Request_DebugWorldPlayback(request);
	m_iSequenceViewerAwaitingRequest = sent ? request.iRequestSequence : 0u;
	m_SequenceViewerReplyDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
	m_SequenceViewerStatus = sent ?
		"서버 응답 대기 중: " + row.id : "요청 전송 실패: 서버 접속과 아레나 상태를 확인해 주세요.";
}

void CMainApp::UpdateSequenceViewer()
{
	for (const auto& sink : {
		CLevel_KakulSaydonArena::Get_Active() ? CLevel_KakulSaydonArena::Get_Active()->Get_PlayerCommandSink() : nullptr,
		CLevel_ValtanArena::Get_Active() ? CLevel_ValtanArena::Get_Active()->Get_PlayerCommandSink() : nullptr })
	{
		LostArk::Shared::S2C_DEBUG_WORLD_PLAYBACK_RESULT result{};
		while (sink && sink->Consume_DebugWorldPlaybackResult(result))
		{
			if (result.iRequestSequence != m_iSequenceViewerAwaitingRequest) continue;
			m_iSequenceViewerAwaitingRequest = 0u;
			m_SequenceViewerStatus = std::string(ResultText(result.eResult)) + "\n" + result.strTargetId;
		}
	}
	if (m_iSequenceViewerAwaitingRequest && std::chrono::steady_clock::now() > m_SequenceViewerReplyDeadline)
	{
		m_iSequenceViewerAwaitingRequest = 0u;
		m_SequenceViewerStatus = "서버 응답이 5초 안에 도착하지 않았습니다. 실행 여부는 미확인이며 자동 재시도하지 않습니다.";
	}
	if (m_iSequenceViewerPendingEditorAction >= 0)
	{
		if (std::chrono::steady_clock::now() > m_SequenceViewerPendingDeadline)
		{ m_iSequenceViewerPendingEditorAction = -1; m_SequenceViewerStatus = "Area 준비 시간이 초과됐습니다. Map Tool 상태를 확인해 주세요."; }
		else ExecuteSequenceViewerAction(m_iSequenceViewerPendingEditorAction);
	}
}

void CMainApp::RenderSequenceViewer()
{
	if (!ImGui::CollapsingHeader("Sequence Viewer / 시퀀스 뷰어", ImGuiTreeNodeFlags_DefaultOpen)) return;
	if (!m_bSequenceViewerLoaded) RefreshSequenceViewer();
	ImGui::TextWrapped("현재 맵과 관계없이 목록을 볼 수 있습니다. Test = 내 화면 미리보기 / Arena = 서버 공동 실행");
	if (ImGui::Button("Refresh / 목록 새로고침")) RefreshSequenceViewer();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(300.f);
	ImGui::InputTextWithHint("##SequenceSearch", "한글 이름 / 구역 / 트리거·시퀀스 ID 검색", m_SequenceViewerSearch.data(), m_SequenceViewerSearch.size());
	ImGui::SameLine(); ImGui::SetNextItemWidth(155.f);
	ImGui::Combo("##SequenceKind", &m_iSequenceViewerKind, "전체\0트리거\0맵 시퀀스\0보스 패턴\0");
	if (ImGui::BeginTabBar("SequenceViewerAreas"))
	{
		for (int area = 0; area < 2; ++area)
		{
			if (!ImGui::BeginTabItem(area == 0 ? "KoukuSaydon / 쿠크세이튼" : "Valtan / 발탄")) continue;
			if (m_iSequenceViewerArea != area)
			{ m_iSequenceViewerArea = area; m_SequenceViewerSelection.clear(); m_iSequenceViewerPendingEditorAction = -1; }
			ImGui::TextDisabled("Area: %s | %zu items", AREAS[area], m_SequenceViewerRows[area].size());
			if (!m_SequenceViewerLoadStatus[area].empty()) ImGui::TextWrapped("%s", m_SequenceViewerLoadStatus[area].c_str());
			const auto query = SearchText(m_SequenceViewerSearch.data());
			if (ImGui::BeginTable("SequenceRows", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
				ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0.f, 270.f)))
			{
				ImGui::TableSetupColumn("구분", ImGuiTableColumnFlags_WidthFixed, 90.f);
				ImGui::TableSetupColumn("이름 / 원본 ID", ImGuiTableColumnFlags_WidthStretch, 3.f);
				ImGui::TableSetupColumn("위치 / 구역", ImGuiTableColumnFlags_WidthStretch, 1.f);
				ImGui::TableSetupColumn("동작 / 상태", ImGuiTableColumnFlags_WidthStretch, 2.f);
				ImGui::TableSetupScrollFreeze(0, 1); ImGui::TableHeadersRow();
				for (const auto& row : m_SequenceViewerRows[area])
				{
					if (m_iSequenceViewerKind && row.kind != m_iSequenceViewerKind - 1) continue;
					if (!query.empty() && SearchText(row.name + row.id + row.location + row.action + row.related).find(query) == std::string::npos) continue;
					const auto key = std::to_string(row.kind) + ":" + row.id;
					ImGui::PushID(key.c_str()); ImGui::TableNextRow(); ImGui::TableNextColumn();
					ImGui::TextUnformatted(KINDS[row.kind]); ImGui::TableNextColumn();
					if (ImGui::Selectable((row.name + "##select").c_str(), m_SequenceViewerSelection == key, ImGuiSelectableFlags_SpanAllColumns))
					{ m_SequenceViewerSelection = key; m_iSequenceViewerPendingEditorAction = -1; }
					ImGui::TextDisabled("%s", row.id.c_str()); ImGui::TableNextColumn();
					ImGui::TextUnformatted(row.location.c_str()); ImGui::TableNextColumn();
					ImGui::TextWrapped("%s", (row.enabled ? row.action : "비활성: " + row.error).c_str()); ImGui::PopID();
				}
				ImGui::EndTable();
			}
			const auto& rows = m_SequenceViewerRows[area];
			const auto row = std::find_if(rows.begin(), rows.end(), [&](const auto& r)
				{ return std::to_string(r.kind) + ":" + r.id == m_SequenceViewerSelection; });
			if (row != rows.end())
			{
				ImGui::TextWrapped("선택: %s\nID: %s\n연결: %s", row->name.c_str(), row->id.c_str(), row->related.c_str());
				if (row->hasPosition) ImGui::Text("위치 (m): %.2f, %.2f, %.2f", row->position.x, row->position.y, row->position.z);
				ImGui::BeginDisabled(!row->enabled || m_iSequenceViewerPendingEditorAction >= 0 || m_iSequenceViewerAwaitingRequest != 0u);
				if (ImGui::Button("Play / 재생")) ExecuteSequenceViewerAction(0);
				ImGui::SameLine(); if (ImGui::Button("Replay / 다시 재생")) ExecuteSequenceViewerAction(1);
				ImGui::BeginDisabled(row->sequenceId.empty());
				ImGui::SameLine(); if (ImGui::Button("Stop / 연출 정지")) ExecuteSequenceViewerAction(2);
				ImGui::EndDisabled(); ImGui::EndDisabled();
				ImGui::SameLine(); if (ImGui::Button("Open Editor / 세부 조정")) ExecuteSequenceViewerAction(3);
				ImGui::BeginDisabled(!row->hasPosition);
				ImGui::SameLine(); if (ImGui::Button("Go To / 트리거 위치")) ExecuteSequenceViewerAction(4);
				ImGui::EndDisabled();
			}
			if (ImGui::Button("Enter Arena / 해당 아레나 입장"))
				RequestDebugLevelNavigation(area == 0 ? LEVEL::KAKULSAYDON_ARENA : LEVEL::VALTAN_ARENA);
			ImGui::TextWrapped("이동·소환·보스 패턴은 서버 아레나에서 실행합니다. Go To는 내 캐릭터만 이동하며 트리거가 발동할 수 있습니다. Stop은 연출만 정지하며 전투·소환·피해를 되돌리지 않습니다.");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	if (!m_SequenceViewerStatus.empty()) ImGui::TextWrapped("%s", m_SequenceViewerStatus.c_str());
}
#endif
