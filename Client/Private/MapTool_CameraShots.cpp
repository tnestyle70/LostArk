#include "imgui.h"
#include "MapTool_Internal.h"
#include "CompositionWorkbenchSession.h"
#include "CompositionTimeline.h"
#include "Camera_Free.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "WorldSequenceToolPanel.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include "Model.h"




void Client::CMapTool::Render_CameraTrackTimeline(EDITOR_CAMERA_SHOT& shot)
{
	/* The span the ruler covers. An authored duration wins; otherwise the last
	   key decides so a track that predates the field still draws. */
	int32_t spanMs = shot.trackDurationMs;
	if (!shot.keyframes.empty())
		spanMs = (std::max)(spanMs, shot.keyframes.back().timeMs);
	if (0 >= spanMs)
		spanMs = 1000;

	/* A group move keeps the authored spacing: every selected key gets the
	   same delta, bounded by the tightest unselected neighbour on either
	   side. An empty intersection rejects the gesture instead of collapsing
	   the keys. Written as a lambda so the nested key type stays private. */
	const auto resolveGroupDeltaBounds = [&shot](
		const std::vector<int32_t>& selection,
		int32_t& outMinDelta, int32_t& outMaxDelta)
	{
		outMinDelta = -120000;
		outMaxDelta = 120000;
		for (const int32_t index : selection)
		{
			if (0 > index || static_cast<size_t>(index) >= shot.keyframes.size())
				continue;
			const int32_t current = shot.keyframes[static_cast<size_t>(index)].timeMs;
			if (0 == index)
			{
				/* The runtime pins the first key at 0 ms. */
				outMinDelta = (std::max)(outMinDelta, 0);
				outMaxDelta = (std::min)(outMaxDelta, 0);
				continue;
			}
			int32_t lowerMs = 0;
			for (int32_t probe = index - 1; 0 <= probe; --probe)
			{
				if (selection.end() != std::find(selection.begin(), selection.end(), probe))
					continue;
				lowerMs = shot.keyframes[static_cast<size_t>(probe)].timeMs + 1;
				break;
			}
			int32_t upperMs = 120000;
			for (size_t probe = static_cast<size_t>(index) + 1u;
				probe < shot.keyframes.size(); ++probe)
			{
				if (selection.end() != std::find(selection.begin(), selection.end(),
					static_cast<int32_t>(probe)))
					continue;
				upperMs = shot.keyframes[probe].timeMs - 1;
				break;
			}
			outMinDelta = (std::max)(outMinDelta, lowerMs - current);
			outMaxDelta = (std::min)(outMaxDelta, upperMs - current);
		}
	};
	/* A stale index from a deleted or reloaded key must never edit another. */
	std::erase_if(m_CameraTrackSelection, [&shot](const int32_t index)
		{ return 0 > index || static_cast<size_t>(index) >= shot.keyframes.size(); });

	ImGui::SetNextItemWidth(220.f);
	ImGui::DragFloat("Zoom (px/s)##CameraTrack", &m_fCameraTrackZoomPxPerSecond,
		2.f, 8.f, 2000.f, "%.0f");
	ImGui::SameLine();
	const f32_t laneWidth = (std::max)(64.f, ImGui::GetContentRegionAvail().x - 8.f);
	if (ImGui::Button("Fit##CameraTrack"))
	{
		m_fCameraTrackZoomPxPerSecond = std::clamp(
			laneWidth * 1000.f / static_cast<f32_t>(spanMs), 8.f, 2000.f);
	}
	ImGui::SameLine();
	ImGui::TextDisabled("Selected %zu", m_CameraTrackSelection.size());

	const f32_t pxPerSecond = (std::max)(8.f, m_fCameraTrackZoomPxPerSecond);
	const f32_t scale = pxPerSecond * 0.001f;          // px per ms
	const f32_t trackWidth = (std::max)(32.f, static_cast<f32_t>(spanMs) * scale);
	constexpr f32_t RULER_HEIGHT = 22.f;
	constexpr f32_t LANE_HEIGHT = 30.f;
	constexpr f32_t KEY_HALF_WIDTH = 5.f;
	constexpr size_t MAX_CAMERA_KEYS = 64u;

	if (!ImGui::BeginChild("##CameraTrackTimeline",
		ImVec2(0.f, RULER_HEIGHT + LANE_HEIGHT + 18.f), true,
		ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImGui::EndChild();
		return;
	}
	ImDrawList* const draw = ImGui::GetWindowDrawList();
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	const ImGuiIO& io = ImGui::GetIO();
	const bool_t additive = io.KeyCtrl || io.KeyShift;

	CompositionTimeline::DrawRuler(draw, origin,
		ImVec2(origin.x + trackWidth, origin.y + RULER_HEIGHT),
		static_cast<uint32_t>(spanMs), pxPerSecond);
	ImGui::InvisibleButton("##CameraTrackRuler", ImVec2(trackWidth, RULER_HEIGHT));
	if (ImGui::IsItemActive() && 0.f <= m_fCutsceneScrubMs)
	{
		const f32_t localMs = (io.MousePos.x - origin.x) / scale;
		m_fCutsceneScrubMs = std::clamp(localMs, 0.f, static_cast<f32_t>(spanMs));
	}

	const ImVec2 laneMin(origin.x, origin.y + RULER_HEIGHT);
	const ImVec2 laneMax(origin.x + trackWidth, laneMin.y + LANE_HEIGHT);
	draw->AddRectFilled(laneMin, laneMax, IM_COL32(24, 26, 31, 255));

	/* Blend windows are shown so a key placed inside one is recognised as such.
	   They are read-only here; the numeric fields above own the values. */
	const f32_t blendInWidth = static_cast<f32_t>(shot.blendInMs) * scale;
	if (0.f < blendInWidth)
		draw->AddRectFilled(laneMin,
			ImVec2((std::min)(laneMin.x + blendInWidth, laneMax.x), laneMax.y),
			IM_COL32(70, 110, 150, 70));
	const f32_t blendOutWidth = static_cast<f32_t>(shot.blendOutMs) * scale;
	if (0.f < blendOutWidth)
		draw->AddRectFilled(
			ImVec2((std::max)(laneMax.x - blendOutWidth, laneMin.x), laneMin.y),
			laneMax, IM_COL32(150, 110, 70, 70));

	/* Empty lane: start a marquee. It only selects; it never edits a value. */
	ImGui::SetCursorScreenPos(laneMin);
	ImGui::InvisibleButton("##CameraLaneBackground",
		ImVec2(trackWidth, LANE_HEIGHT));
	if (ImGui::IsItemActivated())
	{
		m_bCameraTrackMarqueeActive = true;
		m_CameraTrackMarqueeStart = float2_t(io.MousePos.x, io.MousePos.y);
		if (!additive)
			m_CameraTrackSelection.clear();
	}
	if (m_bCameraTrackMarqueeActive && ImGui::IsItemActive())
	{
		const f32_t minX = (std::min)(m_CameraTrackMarqueeStart.x, io.MousePos.x);
		const f32_t maxX = (std::max)(m_CameraTrackMarqueeStart.x, io.MousePos.x);
		draw->AddRectFilled(ImVec2(minX, laneMin.y), ImVec2(maxX, laneMax.y),
			IM_COL32(255, 224, 92, 40));
		draw->AddRect(ImVec2(minX, laneMin.y), ImVec2(maxX, laneMax.y),
			IM_COL32(255, 224, 92, 160));
	}
	if (m_bCameraTrackMarqueeActive && ImGui::IsItemDeactivated())
	{
		const f32_t minX = (std::min)(m_CameraTrackMarqueeStart.x, io.MousePos.x);
		const f32_t maxX = (std::max)(m_CameraTrackMarqueeStart.x, io.MousePos.x);
		for (size_t index = 0; index < shot.keyframes.size(); ++index)
		{
			const f32_t keyX = origin.x +
				static_cast<f32_t>(shot.keyframes[index].timeMs) * scale;
			if (keyX < minX || keyX > maxX) continue;
			const auto typed = static_cast<int32_t>(index);
			if (m_CameraTrackSelection.end() == std::find(
				m_CameraTrackSelection.begin(), m_CameraTrackSelection.end(), typed))
				m_CameraTrackSelection.push_back(typed);
		}
		std::sort(m_CameraTrackSelection.begin(), m_CameraTrackSelection.end());
		if (!m_CameraTrackSelection.empty())
			m_iCutsceneSelectedKey = m_CameraTrackSelection.front();
		m_bCameraTrackMarqueeActive = false;
	}

	for (size_t index = 0; index < shot.keyframes.size(); ++index)
	{
		EDITOR_CAMERA_KEYFRAME& keyframe = shot.keyframes[index];
		const auto typed = static_cast<int32_t>(index);
		const f32_t centerX = origin.x + static_cast<f32_t>(keyframe.timeMs) * scale;
		const ImVec2 keyMin(centerX - KEY_HALF_WIDTH, laneMin.y + 3.f);
		const ImVec2 keyMax(centerX + KEY_HALF_WIDTH, laneMax.y - 3.f);
		const bool_t selected = m_CameraTrackSelection.end() != std::find(
			m_CameraTrackSelection.begin(), m_CameraTrackSelection.end(), typed);
		/* No trim grips: a camera key is an instant, not a span. */
		CompositionTimeline::DrawBox(draw, keyMin, keyMax,
			selected ? IM_COL32(96, 150, 210, 255) : IM_COL32(72, 108, 150, 255),
			selected, nullptr, false, false);

		ImGui::SetCursorScreenPos(keyMin);
		ImGui::PushID(static_cast<int>(index));
		ImGui::InvisibleButton("##CameraKey",
			ImVec2(KEY_HALF_WIDTH * 2.f, keyMax.y - keyMin.y));
		if (ImGui::IsItemActivated())
		{
			if (additive)
			{
				const auto found = std::find(m_CameraTrackSelection.begin(),
					m_CameraTrackSelection.end(), typed);
				if (m_CameraTrackSelection.end() == found)
					m_CameraTrackSelection.push_back(typed);
				else
					m_CameraTrackSelection.erase(found);
				std::sort(m_CameraTrackSelection.begin(), m_CameraTrackSelection.end());
			}
			else if (!selected)
			{
				m_CameraTrackSelection.assign(1, typed);
			}
			m_iCutsceneSelectedKey = typed;
			m_iCameraTrackDragKey = typed;
			m_CameraTrackDragOriginsMs.clear();
			for (const int32_t chosen : m_CameraTrackSelection)
				m_CameraTrackDragOriginsMs.push_back(
					shot.keyframes[static_cast<size_t>(chosen)].timeMs);
		}
		if (ImGui::IsItemActive() && m_iCameraTrackDragKey == typed &&
			m_CameraTrackDragOriginsMs.size() == m_CameraTrackSelection.size())
		{
			int32_t minDelta = 0;
			int32_t maxDelta = 0;
			resolveGroupDeltaBounds(m_CameraTrackSelection, minDelta, maxDelta);
			if (minDelta <= maxDelta)
			{
				const auto requested = static_cast<int32_t>(std::lround(
					ImGui::GetMouseDragDelta().x / scale));
				const int32_t applied = std::clamp(requested, minDelta, maxDelta);
				for (size_t slot = 0; slot < m_CameraTrackSelection.size(); ++slot)
				{
					const auto target =
						static_cast<size_t>(m_CameraTrackSelection[slot]);
					shot.keyframes[target].timeMs = std::clamp(
						m_CameraTrackDragOriginsMs[slot] + applied, 0, 120000);
				}
			}
		}
		if (ImGui::IsItemDeactivated())
		{
			m_iCameraTrackDragKey = -1;
			m_CameraTrackDragOriginsMs.clear();
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\n%d ms", keyframe.sceneId.c_str(), keyframe.timeMs);
		ImGui::PopID();
	}

	/* Playhead. Only drawn while the cutscene clock is actually held. */
	if (0.f <= m_fCutsceneScrubMs)
	{
		const f32_t cursorX = origin.x +
			std::clamp(m_fCutsceneScrubMs, 0.f, static_cast<f32_t>(spanMs)) * scale;
		draw->AddLine(ImVec2(cursorX, origin.y), ImVec2(cursorX, laneMax.y),
			IM_COL32(255, 224, 92, 255), 1.5f);
	}

	ImGui::SetCursorScreenPos(ImVec2(origin.x, laneMax.y + 2.f));
	ImGui::Dummy(ImVec2(trackWidth, 1.f));
	const bool_t timelineFocused =
		ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	ImGui::EndChild();

	/* Shortcuts never fire while a text field owns the keyboard, and a drag in
	   progress is cancelled rather than half applied. */
	if (io.WantTextInput)
	{
		ImGui::TextDisabled("Drag a key to retime it. Ctrl or Shift adds to the selection.");
		return;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_Escape))
	{
		m_iCameraTrackDragKey = -1;
		m_CameraTrackDragOriginsMs.clear();
		m_bCameraTrackMarqueeActive = false;
		m_CameraTrackSelection.clear();
	}
	if (timelineFocused && !m_CameraTrackSelection.empty())
	{
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D))
		{
			/* Duplicate into the gap after each selected key. Rejected whole if
			   any copy has no room or the shot would exceed the key limit. */
			auto staged = shot.keyframes;
			bool_t admitted = staged.size() + m_CameraTrackSelection.size() <= MAX_CAMERA_KEYS;
			for (auto chosen = m_CameraTrackSelection.rbegin();
				admitted && chosen != m_CameraTrackSelection.rend(); ++chosen)
			{
				const auto source = static_cast<size_t>(*chosen);
				const int32_t currentMs = staged[source].timeMs;
				const int32_t nextMs = source + 1u < staged.size() ?
					staged[source + 1u].timeMs : currentMs + 200;
				if (2 > nextMs - currentMs) { admitted = false; break; }
				auto copy = staged[source];
				copy.timeMs = currentMs + (nextMs - currentMs) / 2;
				copy.sceneId = shot.shotId + ".k" + std::to_string(staged.size()) +
					".copy" + std::to_string(currentMs);
				staged.insert(staged.begin() + static_cast<long long>(source) + 1, std::move(copy));
			}
			if (admitted)
			{
				shot.keyframes = std::move(staged);
				shot.trackDurationMs = (std::max)(shot.trackDurationMs,
					shot.keyframes.back().timeMs);
				m_CameraShotStatus = "Duplicated the selected camera keys";
				m_CameraTrackSelection.clear();
			}
			else
			{
				m_CameraShotStatus =
					"Duplicate rejected: no room between keys, or the 64 key limit.";
			}
		}
		if (ImGui::IsKeyPressed(ImGuiKey_Delete))
		{
			const bool_t holdsFirst = m_CameraTrackSelection.front() == 0;
			if (holdsFirst && m_CameraTrackSelection.size() != shot.keyframes.size())
			{
				m_CameraShotStatus =
					"Delete rejected: the first key holds 0 ms. Use Clear Keys to remove the track.";
			}
			else
			{
				for (auto chosen = m_CameraTrackSelection.rbegin();
					chosen != m_CameraTrackSelection.rend(); ++chosen)
				{
					shot.keyframes.erase(shot.keyframes.begin() +
						static_cast<long long>(*chosen));
				}
				m_CameraTrackSelection.clear();
				m_iCutsceneSelectedKey = -1;
				m_CameraShotStatus = "Deleted the selected camera keys";
			}
		}
	}
	ImGui::TextDisabled(
		"Drag a key to retime it. Ctrl or Shift adds; drag the lane to marquee. Ctrl+D duplicates, Delete removes.");
}

void Client::CMapTool::Render_IntegratedCutsceneView()
{
	/* The session, its draft and its save owner stay with the Sequencer shell.
	   This tool only borrows the pane bodies for one frame. Which shell that
	   is follows the Area being edited: the Sequence session reads the
	   KoukuSaydon composition and holds nothing for Valtan. */
	const EDITOR_AREA_DESCRIPTOR* sessionArea = Get_ActiveEditorArea();
	const std::string sessionAreaId =
		nullptr != sessionArea ? sessionArea->areaId : std::string();
	ICompositionWorkbenchSession* selected = nullptr;
	if ("LV_LUT_HEARTRB_ED" == sessionAreaId)
		selected = m_pValtanCompositionSession;
	else if (KAKUL_AREA_ID == sessionAreaId)
		selected = m_pSequenceCompositionSession;
	if (nullptr == selected)
	{
		if (sessionAreaId.empty())
			ImGui::TextDisabled("편집 중인 Area가 없어 컷신을 열 수 없습니다.");
		else if ("LV_LUT_HEARTRB_ED" == sessionAreaId ||
			KAKUL_AREA_ID == sessionAreaId)
		{
			ImGui::TextDisabled(
				"이 Area의 Composition 저작 세션을 이 빌드에서 사용할 수 없습니다: %s",
				sessionAreaId.c_str());
		}
		else
		{
			ImGui::TextDisabled(
				"통합 컷신 편집은 발탄과 쿠크 Area만 지원합니다. 현재 Area: %s",
				sessionAreaId.c_str());
		}
		return;
	}
	m_pHostedCompositionSession = selected;

	/* Source only: this saves the authoring documents the panes below edit and
	   never runs a publisher. Runtime data keeps the last explicit publish. */
	const bool_t anyDirty = m_bIntegratedSequenceDirty || m_bIntegratedObjectDirty;
	ImGui::BeginDisabled(!anyDirty);
	if (ImGui::Button("Save Changes##IntegratedCutscene"))
		m_bIntegratedSaveRequested = true;
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (anyDirty)
	{
		ImGui::Text("Unsaved: %s%s%s",
			m_bIntegratedSequenceDirty ? "Composition" : "",
			(m_bIntegratedSequenceDirty && m_bIntegratedObjectDirty) ? " + " : "",
			m_bIntegratedObjectDirty ? "World Object" : "");
	}
	else
	{
		ImGui::TextDisabled("No unsaved authoring changes.");
	}
	if (!m_IntegratedSaveStatus.empty())
		ImGui::TextWrapped("%s", m_IntegratedSaveStatus.c_str());
	ImGui::Separator();
	ICompositionWorkbenchSession& session = *selected;
	session.Begin_WorkbenchFrame();
	m_bHostingCompositionSession = true;

	const f32_t availableHeight = ImGui::GetContentRegionAvail().y;
	const f32_t upperHeight = (std::max)(180.f, availableHeight * 0.58f);
	if (ImGui::BeginTable("##MapToolIntegratedCutscene", 3,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV,
		ImVec2(0.f, upperHeight)))
	{
		ImGui::TableSetupColumn("Sequences", ImGuiTableColumnFlags_WidthStretch, 0.24f);
		ImGui::TableSetupColumn("Timeline", ImGuiTableColumnFlags_WidthStretch, 0.52f);
		ImGui::TableSetupColumn("Selected", ImGuiTableColumnFlags_WidthStretch, 0.24f);
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		if (ImGui::BeginChild("##IntegratedPatterns", ImVec2(0.f, 0.f), false))
			session.Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE::PATTERNS);
		ImGui::EndChild();

		ImGui::TableSetColumnIndex(1);
		if (ImGui::BeginChild("##IntegratedTimeline", ImVec2(0.f, 0.f), false))
			session.Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE::SEQUENCER);
		ImGui::EndChild();

		ImGui::TableSetColumnIndex(2);
		if (ImGui::BeginChild("##IntegratedDetails", ImVec2(0.f, 0.f), false))
			session.Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE::DETAILS);
		ImGui::EndChild();
		ImGui::EndTable();
	}

	/* The view request the panes raise is consumed here so a focus or expand
	   ask cannot leak into the Sequencer shell on the next frame. */
	(void)session.Consume_WorkbenchViewRequest();
	session.End_WorkbenchFrame();

	/* Selecting a WORLD box and asking to edit its Motion loads that Motion
	   into the Object session. Its detail is shown here so the Motion, the
	   timeline and the camera stay on one screen. */
	ImGui::Separator();
	if (nullptr == m_pObjectCompositionSession)
	{
		ImGui::TextDisabled("The Object authoring session is unavailable.");
		return;
	}
	ImGui::TextUnformatted("Selected Motion");
	ICompositionWorkbenchSession& objectSession = *m_pObjectCompositionSession;
	objectSession.Begin_WorkbenchFrame();
	m_bHostingObjectSession = true;
	if (ImGui::BeginChild("##IntegratedObjectDetails", ImVec2(0.f, 0.f), true))
		objectSession.Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE::DETAILS);
	ImGui::EndChild();
	(void)objectSession.Consume_WorkbenchViewRequest();
	objectSession.End_WorkbenchFrame();
}

void Client::CMapTool::Render_CameraPanel()
{
	/* Cleared every frame; only the frame that actually opens the session
	   frame below may claim the host flag. */
	m_bHostingCompositionSession = false;
	m_bHostingObjectSession = false;
	m_pHostedCompositionSession = nullptr;
	ImGui::Checkbox("통합 컷신 편집##MapToolIntegratedCutscene",
		&m_bIntegratedCutsceneView);
	if (m_bIntegratedCutsceneView)
	{
		ImGui::Separator();
		Render_IntegratedCutsceneView();
		return;
	}
	ImGui::Separator();
	ImGui::TextUnformatted("Player Camera");
	ImGui::Separator();

	const shared_ptr<CCamera_Free> camera =
		m_pAssetTestCamera.lock();
	if (nullptr == camera)
	{
		ImGui::TextUnformatted("ASSET_TEST camera is unavailable.");
		if (ImGui::Button("Find Camera"))
			Find_AssetTestCamera();
		ImGui::TextWrapped("%s", m_CameraStatus.c_str());
		return;
	}

	ImGui::Text(
		"Mode: %s",
		camera->Is_FollowEnabled() ?
			"Follow Player" :
			"Free Camera");
	ImGui::SameLine();
	if (camera->Is_FollowEnabled())
	{
		if (ImGui::Button("Switch to Free Camera"))
		{
			camera->Set_FollowEnabled(false);
			m_CameraStatus = "Free Camera";
		}
	}
	else
	{
		if (ImGui::Button("Follow Player"))
		{
			camera->Set_FollowEnabled(true);
			m_CameraStatus = "Following Player";
		}
	}

	ImGui::Separator();
	ImGui::BeginDisabled(!camera->Is_FollowEnabled());
	float3_t positionOffset = camera->Get_PositionOffset();
	if (ImGui::DragFloat3(
		"Position Offset",
		&positionOffset.x,
		0.1f,
		-100.f,
		100.f,
		"%.2f"))
	{
		camera->Set_PositionOffset(positionOffset);
		m_CameraStatus = "Position Offset applied";
	}
	ImGui::EndDisabled();
	if (!camera->Is_FollowEnabled())
		ImGui::TextDisabled("Follow Player mode previews Position Offset");

	if (!camera->Is_FollowEnabled())
	{
		ImGui::Text(
			"Mouse Look: %s",
			camera->Is_MouseLookEnabled() ? "Enabled" : "Locked");
	}

	ImGui::TextUnformatted(m_CameraStatus.c_str());
	ImGui::TextDisabled(
		"Tab: toggle Free Camera mouse look");
	ImGui::TextDisabled(
		"Free Camera: WASD move");

	ImGui::Separator();
	/* The cutscene list comes first: it is the thing the editor picks,
	   and the shot list below is the material each cut points at. */
	Render_CutsceneSection();
	Render_CameraShotSection();
}

bool_t Client::CMapTool::Save_CameraShotDocumentAtomic(const std::filesystem::path& path,
	const std::string_view expectedText, const std::string_view text, std::string& outStatus)
{
	std::error_code error;
	std::filesystem::create_directories(path.parent_path(), error);
	if (error) { outStatus = "Cannot create camera authoring directory."; return false; }
	auto lockPath = path; lockPath += L".lock";
	const HANDLE lock = CreateFileW(lockPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
		OPEN_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
	if (lock == INVALID_HANDLE_VALUE)
	{ outStatus = "Camera authoring is being saved by another editor."; return false; }
	const auto sourceMatches = [&]() {
		std::error_code inspectError;
		if (!std::filesystem::exists(path, inspectError)) return !inspectError && expectedText.empty();
		std::ifstream input(path, std::ios::binary);
		if (!input) return false;
		const std::string current(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>{});
		return !input.bad() && current == expectedText;
	};
	if (!sourceMatches())
	{
		CloseHandle(lock);
		outStatus = "Camera source changed on disk. Re-enter the Area before saving; existing source was preserved.";
		return false;
	}
	auto temporary = path; temporary += L".tmp." + std::to_wstring(GetCurrentProcessId());
	bool written = false;
	{
		std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
		if (output) { output.write(text.data(), static_cast<std::streamsize>(text.size())); output.flush(); written = output.good(); output.close(); written = written && !output.fail(); }
	}
	const bool committed = written && sourceMatches() &&
		MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	if (!committed) std::filesystem::remove(temporary, error);
	CloseHandle(lock);
	if (!committed) outStatus = "Camera atomic save failed; source preserved.";
	return committed;
}

bool_t Client::CMapTool::Parse_CameraShotDocument(const std::string& text,
	const std::string& areaId, std::vector<EDITOR_CAMERA_SHOT>& outShots,
	std::vector<EDITOR_CUTSCENE>& outCutscenes, std::string& outError)
{
	outShots.clear();
	outCutscenes.clear();
	std::string parseError;
	DATA_JSON_VALUE root;
	if (!CDataJson::Parse(text, root, parseError) || !root.Is_Object())
	{
		outError = "Camera shot parse failed: " + parseError;
		return false;
	}
	const DATA_JSON_VALUE* schema = root.Find("schema");
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* area = root.Find("areaId");
	const DATA_JSON_VALUE* shots = root.Find("shots");
	if (nullptr == schema || !schema->Is_String() ||
		schema->Get_String() != "lostark.camera-shots" ||
		nullptr == version || !version->Is_Number() ||
		version->Get_Number() != 1.0 ||
		nullptr == area || !area->Is_String() ||
		area->Get_String() != areaId ||
		nullptr == shots || !shots->Is_Array())
	{
		outError = "Camera shot header is invalid";
		return false;
	}

	const auto readFloat3 = [](const DATA_JSON_VALUE* value, float3_t& out)
	{
		if (nullptr == value || !value->Is_Array() ||
			3u != value->Get_Array().size())
		{
			return false;
		}
		f32_t parts[3]{};
		for (size_t index = 0; index < 3u; ++index)
		{
			const DATA_JSON_VALUE& part = value->Get_Array()[index];
			if (!part.Is_Number())
				return false;
			parts[index] = static_cast<f32_t>(part.Get_Number());
		}
		out = float3_t(parts[0], parts[1], parts[2]);
		return true;
	};

	std::vector<EDITOR_CAMERA_SHOT> staged;
	for (const DATA_JSON_VALUE& value : shots->Get_Array())
	{
		EDITOR_CAMERA_SHOT shot;
		const DATA_JSON_VALUE* shotId = value.Find("shotId");
		const DATA_JSON_VALUE* sequenceId = value.Find("sequenceInstanceId");
		const DATA_JSON_VALUE* box = value.Find("box");
		if (nullptr == shotId || !shotId->Is_String() ||
			nullptr == box || !box->Is_Object() ||
			!readFloat3(box->Find("center"), shot.center) ||
			!readFloat3(box->Find("halfExtents"), shot.halfExtents) ||
			!readFloat3(value.Find("eye"), shot.eye) ||
			!readFloat3(value.Find("lookAt"), shot.lookAt))
		{
			outError = "Camera shot row is invalid";
			return false;
		}
		if (nullptr == sequenceId || !sequenceId->Is_String())
		{
			outError = "Camera shot sequence binding is invalid";
			return false;
		}
		shot.shotId = shotId->Get_String();
		shot.sequenceInstanceId = sequenceId->Get_String();
		shot.displayName = shot.shotId;
		if (const auto* name = value.Find("displayName"))
		{
			if (!name->Is_String() || name->Get_String().empty() || name->Get_String().size() > 128u)
			{ outError = "Camera displayName is invalid"; return false; }
			shot.displayName = name->Get_String();
		}
		if (const auto* hold = value.Find("defaultHoldMs"))
		{
			if (!hold->Is_Number() || !std::isfinite(hold->Get_Number()) || hold->Get_Number() < 0.0 ||
				hold->Get_Number() > 600000.0 || std::floor(hold->Get_Number()) != hold->Get_Number())
			{ outError = "Camera defaultHoldMs is invalid"; return false; }
			shot.defaultHoldMs = static_cast<int32_t>(hold->Get_Number());
		}
		if (const auto* activation = value.Find("activation"))
		{
			if (!activation->Is_String() || (activation->Get_String() != "AUTO" && activation->Get_String() != "PATTERN_ONLY"))
			{ outError = "Camera activation is invalid"; return false; }
			shot.patternOnly = activation->Get_String() == "PATTERN_ONLY";
		}
		if (const auto* easing = value.Find("transitionEasing"))
		{
			if (!easing->Is_String() || (easing->Get_String() != "LINEAR" && easing->Get_String() != "SMOOTHSTEP"))
			{ outError = "Camera transitionEasing is invalid"; return false; }
			shot.linearTransition = easing->Get_String() == "LINEAR";
		}
		const DATA_JSON_VALUE* yaw = box->Find("yawDegrees");
		const DATA_JSON_VALUE* fov = value.Find("fovYDegrees");
		const DATA_JSON_VALUE* blendIn = value.Find("blendInMs");
		const DATA_JSON_VALUE* blendOut = value.Find("blendOutMs");
		const DATA_JSON_VALUE* priority = value.Find("priority");
		if (nullptr == yaw || !yaw->Is_Number() ||
			nullptr == fov || !fov->Is_Number() ||
			nullptr == blendIn || !blendIn->Is_Number() ||
			nullptr == blendOut || !blendOut->Is_Number() ||
			nullptr == priority || !priority->Is_Number())
		{
			outError = "Camera shot numbers are invalid";
			return false;
		}
		shot.yawDegrees = static_cast<f32_t>(yaw->Get_Number());
		shot.fovYDegrees = static_cast<f32_t>(fov->Get_Number());
		shot.blendInMs = static_cast<int32_t>(blendIn->Get_Number());
		shot.blendOutMs = static_cast<int32_t>(blendOut->Get_Number());
		shot.priority = static_cast<int32_t>(priority->Get_Number());
		const DATA_JSON_VALUE* follow = value.Find("follow");
		if (nullptr != follow && follow->Is_Object())
		{
			if (!readFloat3(follow->Find("eyeOffset"), shot.followEyeOffset) ||
				!readFloat3(follow->Find("lookAtOffset"),
					shot.followLookAtOffset))
			{
				outError = "Camera shot follow offsets are invalid";
				return false;
			}
			shot.followsPlayer = true;
		}
		const DATA_JSON_VALUE* cameraTrack = value.Find("cameraTrack");
		if (nullptr != cameraTrack && cameraTrack->Is_Object())
		{
			const DATA_JSON_VALUE* durationMs = cameraTrack->Find("durationMs");
			const DATA_JSON_VALUE* interpolation =
				cameraTrack->Find("interpolation");
			const DATA_JSON_VALUE* easing = cameraTrack->Find("easing");
			const DATA_JSON_VALUE* keyframes = cameraTrack->Find("keyframes");
			if (nullptr == durationMs || !durationMs->Is_Number() ||
				nullptr == interpolation || !interpolation->Is_String() ||
				nullptr == easing || !easing->Is_String() ||
				nullptr == keyframes || !keyframes->Is_Array())
			{
				outError = "Camera track is invalid: " + shot.shotId;
				return false;
			}
			shot.trackDurationMs =
				static_cast<int32_t>(durationMs->Get_Number());
			shot.interpolationIndex =
				"LINEAR" == interpolation->Get_String() ? 0 : 1;
			shot.easingIndex = "LINEAR" == easing->Get_String() ? 0 :
				("HOLD" == easing->Get_String() ? 2 : 1);
			for (const DATA_JSON_VALUE& entry : keyframes->Get_Array())
			{
				EDITOR_CAMERA_KEYFRAME keyframe{};
				const DATA_JSON_VALUE* sceneId = entry.Find("sceneId");
				const DATA_JSON_VALUE* timeMs = entry.Find("timeMs");
				const DATA_JSON_VALUE* fov = entry.Find("fovYDegrees");
				if (nullptr == sceneId || !sceneId->Is_String() ||
					nullptr == timeMs || !timeMs->Is_Number() ||
					nullptr == fov || !fov->Is_Number() ||
					!readFloat3(entry.Find("eye"), keyframe.eye) ||
					!readFloat3(entry.Find("lookAt"), keyframe.lookAt))
				{
					outError =
						"Camera keyframe is invalid: " + shot.shotId;
					return false;
				}
				keyframe.sceneId = sceneId->Get_String();
				keyframe.timeMs = static_cast<int32_t>(timeMs->Get_Number());
				keyframe.fovYDegrees = static_cast<f32_t>(fov->Get_Number());
				if (const auto* up = entry.Find("up"))
				{
					if (!readFloat3(up, keyframe.up))
					{ outError = "Camera keyframe up is invalid: " + shot.shotId; return false; }
					keyframe.hasUp = true;
				}
				shot.keyframes.push_back(std::move(keyframe));
			}
		}
		staged.push_back(std::move(shot));
	}

	/* Optional cutscenes. A document without them keeps the previous
	   single-shot editing, so older Areas load exactly as before. */
	std::vector<EDITOR_CUTSCENE> stagedCutscenes;
	const DATA_JSON_VALUE* cutscenes = root.Find("cutscenes");
	if (nullptr != cutscenes)
	{
		if (!cutscenes->Is_Array())
		{
			outError = "Cutscene list is invalid";
			return false;
		}
		std::unordered_set<std::string> shotIds;
		for (const EDITOR_CAMERA_SHOT& shot : staged)
			shotIds.insert(shot.shotId);
		std::unordered_set<std::string> cutsceneIds;
		for (const DATA_JSON_VALUE& value : cutscenes->Get_Array())
		{
			EDITOR_CUTSCENE cutscene;
			const DATA_JSON_VALUE* cutsceneId = value.Find("cutsceneId");
			const DATA_JSON_VALUE* durationMs = value.Find("durationMs");
			const DATA_JSON_VALUE* cuts = value.Find("cameraCuts");
			if (nullptr == cutsceneId || !cutsceneId->Is_String() ||
				cutsceneId->Get_String().empty() ||
				nullptr == durationMs || !durationMs->Is_Number() ||
				!std::isfinite(durationMs->Get_Number()) ||
				durationMs->Get_Number() < 1.0 ||
				durationMs->Get_Number() > 600000.0 ||
				nullptr == cuts || !cuts->Is_Array())
			{
				outError = "Cutscene row is invalid";
				return false;
			}
			cutscene.cutsceneId = cutsceneId->Get_String();
			if (!cutsceneIds.insert(cutscene.cutsceneId).second)
			{
				outError =
					"Cutscene id is duplicate: " + cutscene.cutsceneId;
				return false;
			}
			cutscene.durationMs = static_cast<int32_t>(durationMs->Get_Number());
			cutscene.displayName = cutscene.cutsceneId;
			if (const auto* name = value.Find("displayName"))
			{
				if (!name->Is_String() || name->Get_String().empty() ||
					name->Get_String().size() > 128u)
				{
					outError = "Cutscene displayName is invalid: " +
						cutscene.cutsceneId;
					return false;
				}
				cutscene.displayName = name->Get_String();
			}
			/* Cuts are stored in start order and may not overlap: two shots
			   owning one instant is an authoring mistake, not a priority
			   contest the preview should quietly resolve. */
			int32_t previousEndMs = 0;
			std::unordered_set<std::string> cutIds;
			for (const DATA_JSON_VALUE& entry : cuts->Get_Array())
			{
				EDITOR_CUTSCENE_CUT cut;
				const DATA_JSON_VALUE* cutId = entry.Find("cutId");
				const DATA_JSON_VALUE* shotId = entry.Find("shotId");
				const DATA_JSON_VALUE* startMs = entry.Find("startMs");
				if (nullptr == cutId || !cutId->Is_String() ||
					cutId->Get_String().empty() ||
					nullptr == shotId || !shotId->Is_String() ||
					nullptr == startMs || !startMs->Is_Number() ||
					!std::isfinite(startMs->Get_Number()) ||
					startMs->Get_Number() < 0.0 ||
					startMs->Get_Number() > 600000.0)
				{
					outError = "Cutscene cut is invalid: " +
						cutscene.cutsceneId;
					return false;
				}
				cut.cutId = cutId->Get_String();
				cut.shotId = shotId->Get_String();
				cut.startMs = static_cast<int32_t>(startMs->Get_Number());
				if (!cutIds.insert(cut.cutId).second)
				{
					outError = "Cutscene cut id is duplicate: " +
						cut.cutId;
					return false;
				}
				if (shotIds.end() == shotIds.find(cut.shotId))
				{
					outError = "Cutscene cut names an unknown shot: " +
						cut.shotId;
					return false;
				}
				if (cut.startMs < previousEndMs)
				{
					outError = "Cutscene cuts overlap: " + cut.cutId;
					return false;
				}
				const auto found = std::find_if(staged.begin(), staged.end(),
					[&cut](const EDITOR_CAMERA_SHOT& shot)
					{ return shot.shotId == cut.shotId; });
				previousEndMs = cut.startMs +
					(staged.end() != found ? found->trackDurationMs : 0);
				cutscene.cameraCuts.push_back(std::move(cut));
			}
			if (const auto* instances = value.Find("worldInstanceIds"))
			{
				if (!instances->Is_Array())
				{
					outError = "Cutscene world list is invalid: " +
						cutscene.cutsceneId;
					return false;
				}
				for (const DATA_JSON_VALUE& entry : instances->Get_Array())
				{
					if (!entry.Is_String() || entry.Get_String().empty())
					{
						outError =
							"Cutscene world instance is invalid: " +
							cutscene.cutsceneId;
						return false;
					}
					cutscene.worldInstanceIds.push_back(entry.Get_String());
				}
			}
			stagedCutscenes.push_back(std::move(cutscene));
		}
	}

	outShots = std::move(staged);
	outCutscenes = std::move(stagedCutscenes);
	return true;
}

bool_t Client::CMapTool::Load_CameraShots(
	const EDITOR_AREA_DESCRIPTOR& descriptor)
{
	/* Reading and validating come first and change nothing that is loaded.
	   Only a document that parsed completely replaces the draft, so a bad
	   Reload of the same Area keeps the edits, the selection and the reason
	   on screen. A different Area never keeps the previous one's rows. */
	const bool_t sameArea = !m_strCameraShotAreaId.empty() &&
		m_strCameraShotAreaId == descriptor.areaId;
	const auto clearLoaded = [this, &descriptor]()
	{
		Stop_EditorCutscene();
		End_CameraShotPreview();
		m_Cutscenes.clear();
		m_iSelectedCutscene = 0u;
		m_CutsceneStatus.clear();
		m_CameraShots.clear();
		m_strCameraShotBaselineText.clear();
		m_iSelectedCameraShot = 0u;
		m_strCameraShotAreaId = descriptor.areaId;
		m_strCameraShotLoadedDocumentText =
			Build_CameraShotDocumentText(descriptor.areaId, 0u);
		m_bCameraShotReloadConfirmPending = false;
	};
	if (descriptor.cameraShotDocument.empty())
	{
		clearLoaded();
		m_CameraShotStatus = "No camera shot document for this Area";
		return false;
	}
	std::error_code error;
	if (!std::filesystem::is_regular_file(descriptor.cameraShotDocument, error) ||
		error)
	{
		clearLoaded();
		m_CameraShotStatus =
			"No shots authored yet. Add Shot From Camera creates the first one.";
		return true;
	}

	std::string text;
	std::string parseError;
	std::vector<EDITOR_CAMERA_SHOT> staged;
	std::vector<EDITOR_CUTSCENE> stagedCutscenes;
	bool_t parsed = false;
	if (!ReadTextFile(descriptor.cameraShotDocument, text))
		parseError = "Camera shot document could not be read";
	else
	{
		parsed = Parse_CameraShotDocument(text, descriptor.areaId, staged,
			stagedCutscenes, parseError);
	}
	if (!parsed)
	{
		if (sameArea)
		{
			m_bCameraShotReloadConfirmPending = false;
			m_CameraShotStatus =
				"Reload failed; the loaded shots and cutscenes are kept: " +
				parseError;
			return false;
		}
		clearLoaded();
		m_CameraShotStatus = parseError;
		return false;
	}

	/* A live session points at rows that are about to be replaced, so it
	   hands the camera and its actors back before the swap. */
	const std::string selectedCutsceneId =
		m_iSelectedCutscene < m_Cutscenes.size() ?
		m_Cutscenes[m_iSelectedCutscene].cutsceneId : std::string();
	Stop_EditorCutscene();
	End_CameraShotPreview();
	m_CutsceneStatus.clear();
	m_CameraShots = std::move(staged);
	m_Cutscenes = std::move(stagedCutscenes);
	m_iSelectedCutscene = 0u;
	if (sameArea)
	{
		for (size_t index = 0; index < m_Cutscenes.size(); ++index)
		{
			if (m_Cutscenes[index].cutsceneId == selectedCutsceneId)
			{
				m_iSelectedCutscene = index;
				break;
			}
		}
	}
	if (!sameArea || m_iSelectedCameraShot >= m_CameraShots.size())
		m_iSelectedCameraShot = 0u;
	m_strCameraShotBaselineText = text;
	m_strCameraShotAreaId = descriptor.areaId;
	m_strCameraShotLoadedDocumentText =
		Build_CameraShotDocumentText(descriptor.areaId, 0u);
	m_bCameraShotReloadConfirmPending = false;
	m_CameraShotStatus =
		"Loaded " + std::to_string(m_CameraShots.size()) + " shot(s)";
	if (!m_Cutscenes.empty())
	{
		m_CameraShotStatus += " and " + std::to_string(m_Cutscenes.size()) +
			" cutscene(s)";
	}
	return true;
}

std::string Client::CMapTool::Build_CameraShotDocumentText(
	const std::string& areaId, const uint32_t revision) const
{
	/* Shortest text that reads back as the same float. The old %.6g kept
	   six significant digits, which cuts a 150 m coordinate to the millimetre;
	   to_chars keeps every float the draft holds and still writes 0.1 as 0.1,
	   so a value that came from a %.6g file is written back unchanged. */
	const auto number = [](const f32_t value)
	{
		char buffer[32]{};
		const std::to_chars_result result =
			std::to_chars(buffer, buffer + sizeof(buffer), value);
		return std::errc{} == result.ec ?
			std::string(buffer, result.ptr) : std::string("0");
	};
	const auto vector3 = [&number](const float3_t& value)
	{
		return "[" + number(value.x) + ", " + number(value.y) + ", " +
			number(value.z) + "]";
	};

	std::string text;
	text += "{\n  \"schema\": \"lostark.camera-shots\",\n";
	text += "  \"formatVersion\": 1,\n";
	text += "  \"areaId\": \"" + areaId + "\",\n";
	text += "  \"revision\": " + std::to_string(revision) + ",\n";
	text += "  \"shots\": [";
	for (size_t index = 0; index < m_CameraShots.size(); ++index)
	{
		const EDITOR_CAMERA_SHOT& shot = m_CameraShots[index];
		text += 0u == index ? "\n" : ",\n";
		text += "    {\n";
		text += "      \"shotId\": \"" + shot.shotId + "\",\n";
		text += "      \"displayName\": \"" + CDataJson::Escape(shot.displayName.empty() ? shot.shotId : shot.displayName) + "\",\n";
		text += "      \"defaultHoldMs\": " + std::to_string(shot.defaultHoldMs) + ",\n";
		text += "      \"transitionEasing\": \"" + std::string(shot.linearTransition ? "LINEAR" : "SMOOTHSTEP") + "\",\n";
		text += "      \"activation\": \"" + std::string(shot.patternOnly ? "PATTERN_ONLY" : "AUTO") + "\",\n";
		text += "      \"sequenceInstanceId\": \"" +
			shot.sequenceInstanceId + "\",\n";
		text += "      \"box\": { \"center\": " + vector3(shot.center) +
			", \"halfExtents\": " + vector3(shot.halfExtents) +
			", \"yawDegrees\": " + number(shot.yawDegrees) + " },\n";
		text += "      \"eye\": " + vector3(shot.eye) + ",\n";
		text += "      \"lookAt\": " + vector3(shot.lookAt) + ",\n";
		text += "      \"fovYDegrees\": " + number(shot.fovYDegrees) + ",\n";
		text += "      \"blendInMs\": " + std::to_string(shot.blendInMs) + ",\n";
		text += "      \"blendOutMs\": " + std::to_string(shot.blendOutMs) + ",\n";
		text += "      \"priority\": " + std::to_string(shot.priority);
		if (shot.followsPlayer)
		{
			text += ",\n      \"follow\": { \"eyeOffset\": " +
				vector3(shot.followEyeOffset) + ", \"lookAtOffset\": " +
				vector3(shot.followLookAtOffset) + " }";
		}
		if (!shot.keyframes.empty())
		{
			text += ",\n      \"cameraTrack\": {\n";
			text += "        \"durationMs\": " +
				std::to_string(shot.trackDurationMs) + ",\n";
			text += "        \"interpolation\": \"" +
				std::string(0 == shot.interpolationIndex ?
					"LINEAR" : "CATMULL_ROM") + "\",\n";
			text += "        \"easing\": \"" +
				std::string(0 == shot.easingIndex ? "LINEAR" :
					(1 == shot.easingIndex ? "SMOOTHSTEP" : "HOLD")) + "\",\n";
			text += "        \"keyframes\": [\n";
			for (size_t keyIndex = 0; keyIndex < shot.keyframes.size(); ++keyIndex)
			{
				const EDITOR_CAMERA_KEYFRAME& keyframe = shot.keyframes[keyIndex];
				text += "          { \"sceneId\": \"" + keyframe.sceneId +
					"\", \"timeMs\": " + std::to_string(keyframe.timeMs) +
					", \"eye\": " + vector3(keyframe.eye) +
					", \"lookAt\": " + vector3(keyframe.lookAt) +
					", \"fovYDegrees\": " + number(keyframe.fovYDegrees) +
					(keyframe.hasUp ? ", \"up\": " + vector3(keyframe.up) : "") + " }";
				text += (keyIndex + 1u == shot.keyframes.size()) ? "\n" : ",\n";
			}
			text += "        ]\n      }";
		}
		text += "\n    }";
	}
	text += m_CameraShots.empty() ? "]" : "\n  ]";
	/* Cutscenes are written only when the Area has them, so a document that
	   never carried the section keeps its previous shape byte for byte. */
	if (!m_Cutscenes.empty())
	{
		text += ",\n  \"cutscenes\": [";
		for (size_t index = 0; index < m_Cutscenes.size(); ++index)
		{
			const EDITOR_CUTSCENE& cutscene = m_Cutscenes[index];
			text += 0u == index ? "\n" : ",\n";
			text += "    {\n";
			text += "      \"cutsceneId\": \"" + cutscene.cutsceneId + "\",\n";
			text += "      \"displayName\": \"" +
				CDataJson::Escape(cutscene.displayName.empty() ?
					cutscene.cutsceneId : cutscene.displayName) + "\",\n";
			text += "      \"durationMs\": " +
				std::to_string(cutscene.durationMs) + ",\n";
			text += "      \"cameraCuts\": [";
			for (size_t cutIndex = 0; cutIndex < cutscene.cameraCuts.size();
				++cutIndex)
			{
				const EDITOR_CUTSCENE_CUT& cut = cutscene.cameraCuts[cutIndex];
				text += 0u == cutIndex ? "\n" : ",\n";
				text += "        { \"cutId\": \"" + cut.cutId +
					"\", \"shotId\": \"" + cut.shotId +
					"\", \"startMs\": " + std::to_string(cut.startMs) + " }";
			}
			text += cutscene.cameraCuts.empty() ? "]" : "\n      ]";
			text += ",\n      \"worldInstanceIds\": [";
			for (size_t worldIndex = 0;
				worldIndex < cutscene.worldInstanceIds.size(); ++worldIndex)
			{
				text += 0u == worldIndex ? "" : ", ";
				text += "\"" + cutscene.worldInstanceIds[worldIndex] + "\"";
			}
			text += "]\n    }";
		}
		text += "\n  ]";
	}
	text += "\n}\n";
	return text;
}

bool_t Client::CMapTool::Is_CameraShotDraftDirty() const
{
	if (m_strCameraShotAreaId.empty())
		return false;
	return Build_CameraShotDocumentText(m_strCameraShotAreaId, 0u) !=
		m_strCameraShotLoadedDocumentText;
}

bool_t Client::CMapTool::Validate_CameraShotDraft(std::string& outError) const
{
	std::unordered_set<std::string> ids;
	for (const EDITOR_CAMERA_SHOT& shot : m_CameraShots)
	{
		if (shot.shotId.empty() || !ids.emplace(shot.shotId).second)
		{
			outError = "Shot IDs must be unique and non-empty";
			return false;
		}
		if (shot.halfExtents.x <= 0.f || shot.halfExtents.y <= 0.f ||
			shot.halfExtents.z <= 0.f)
		{
			outError = "Half extents must be positive: " + shot.shotId;
			return false;
		}
	}
	return true;
}

namespace
{
	/* Revision follows the document on disk so two saves never collide. */
	uint32_t ReadNextCameraShotRevision(const std::filesystem::path& path)
	{
		uint32_t revision = 1u;
		std::string existingText;
		if (ReadTextFile(path, existingText))
		{
			DATA_JSON_VALUE existing;
			std::string ignored;
			if (CDataJson::Parse(existingText, existing, ignored) &&
				existing.Is_Object())
			{
				const DATA_JSON_VALUE* current = existing.Find("revision");
				if (nullptr != current && current->Is_Number() &&
					current->Get_Number() >= 1.0 &&
					current->Get_Number() < 4294967295.0)
				{
					revision = static_cast<uint32_t>(current->Get_Number()) + 1u;
				}
			}
		}
		return revision;
	}
}

bool_t Client::CMapTool::Save_CameraShots()
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor || descriptor->cameraShotDocument.empty())
	{
		m_CameraShotStatus = "No camera shot document for this Area";
		return false;
	}
	if (!Validate_CameraShotDraft(m_CameraShotStatus))
		return false;
	const uint32_t revision =
		ReadNextCameraShotRevision(descriptor->cameraShotDocument);
	const std::string text =
		Build_CameraShotDocumentText(descriptor->areaId, revision);
	if (!Save_CameraShotDocumentAtomic(descriptor->cameraShotDocument,
		m_strCameraShotBaselineText, text, m_CameraShotStatus)) return false;
	m_strCameraShotBaselineText = text;
	m_strCameraShotAreaId = descriptor->areaId;
	m_strCameraShotLoadedDocumentText =
		Build_CameraShotDocumentText(descriptor->areaId, 0u);
	m_CameraShotStatus = "Saved " + std::to_string(m_CameraShots.size()) +
		" shot(s) as revision " + std::to_string(revision) +
		". Publish the Area to ship it.";
	return true;
}

bool_t Client::CMapTool::Save_CutsceneAuthoring()
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor || descriptor->cameraShotDocument.empty())
	{
		m_CutsceneSaveStatus = "이 Area 에는 카메라 문서가 없어 컷신을 저장할 수 없습니다.";
		return false;
	}
	const bool_t hasWorld = nullptr != m_pWorldSequenceToolPanel &&
		m_pWorldSequenceToolPanel->Is_Ready() &&
		m_pWorldSequenceToolPanel->Get_Document().Get_AreaId() ==
			descriptor->areaId;
	const bool_t cameraDirty = Is_CameraShotDraftDirty();
	const bool_t worldDirty = hasWorld && m_pWorldSequenceToolPanel->Is_Dirty();
	if (!cameraDirty && !worldDirty)
	{
		m_CutsceneSaveStatus = "저장할 변경이 없습니다.";
		return true;
	}

	/* 1. Both drafts are validated before any byte is written. The camera
	   text is parsed back with the loader's own rules, so what is saved is
	   exactly what the next Reload accepts. */
	const uint32_t revision =
		ReadNextCameraShotRevision(descriptor->cameraShotDocument);
	const std::string cameraText =
		Build_CameraShotDocumentText(descriptor->areaId, revision);
	std::string status;
	if (cameraDirty)
	{
		std::vector<EDITOR_CAMERA_SHOT> checkShots;
		std::vector<EDITOR_CUTSCENE> checkCutscenes;
		if (!Validate_CameraShotDraft(status) ||
			!Parse_CameraShotDocument(cameraText, descriptor->areaId,
				checkShots, checkCutscenes, status))
		{
			m_CutsceneSaveStatus =
				"카메라 draft 검증 실패 - 아무 파일도 쓰지 않았습니다: " + status;
			return false;
		}
	}
	if (worldDirty && !m_pWorldSequenceToolPanel->Validate(
		m_Catalog, Authoring_Placements(), Authoring_Deploy(), status))
	{
		m_CutsceneSaveStatus =
			"World draft 검증 실패 - 아무 파일도 쓰지 않았습니다: " + status;
		return false;
	}

	/* 2. The same lock and pending-transaction recovery the linked Map Tool
	   save uses for this Area's World file. */
	const std::filesystem::path sequencePath = Get_WorldSequencePath();
	SCOPED_AUTHORING_SAVE_LOCK authoringLock;
	if (!authoringLock.Acquire(sequencePath, status) ||
		!RecoverAuthoringTransactionUnderLock(
			descriptor->sourcePlacements, sequencePath, status))
	{
		m_CutsceneSaveStatus = "저장 잠금을 얻지 못했습니다: " + status;
		return false;
	}

	/* 3. Each file must still be what this editor loaded or last saved. */
	if (cameraDirty)
	{
		std::string diskText;
		std::error_code error;
		const bool_t exists =
			std::filesystem::is_regular_file(descriptor->cameraShotDocument, error) &&
			!error;
		if ((exists && (!ReadTextFile(descriptor->cameraShotDocument, diskText) ||
			diskText != m_strCameraShotBaselineText)) ||
			(!exists && !m_strCameraShotBaselineText.empty()))
		{
			m_CutsceneSaveStatus =
				"카메라 문서가 디스크에서 바뀌었습니다. Reload 후 다시 저장하세요 (아무것도 쓰지 않음).";
			return false;
		}
	}
	if (worldDirty && !m_pWorldSequenceToolPanel->Matches_SequenceBaseline(status))
	{
		m_CutsceneSaveStatus = status +
			". World 문서를 다시 읽은 뒤 저장하세요 (아무것도 쓰지 않음).";
		return false;
	}

	/* 4. Byte backups of both files, restored if the second write fails. */
	AUTHORING_FILE_BACKUP cameraBackup;
	AUTHORING_FILE_BACKUP sequenceBackup;
	if (cameraDirty && !PrepareAuthoringBackup(
		descriptor->cameraShotDocument, cameraBackup, status))
	{
		m_CutsceneSaveStatus = "카메라 백업 실패: " + status;
		return false;
	}
	if (worldDirty && !PrepareAuthoringBackup(sequencePath, sequenceBackup, status))
	{
		if (cameraDirty)
			DiscardAuthoringBackup(cameraBackup);
		m_CutsceneSaveStatus = "World 백업 실패: " + status;
		return false;
	}

	/* 5. Camera first, then World. */
	if (cameraDirty && !Save_CameraShotDocumentAtomic(descriptor->cameraShotDocument,
		m_strCameraShotBaselineText, cameraText, status))
	{
		DiscardAuthoringBackup(cameraBackup);
		if (worldDirty)
			DiscardAuthoringBackup(sequenceBackup);
		m_CutsceneSaveStatus = "카메라 저장 실패 - 두 파일 모두 그대로입니다: " + status;
		return false;
	}
	if (worldDirty)
	{
		bool_t previousExists = false;
		std::string previousBytes;
		m_pWorldSequenceToolPanel->Get_SequenceBaseline(previousExists, previousBytes);
		if (!m_pWorldSequenceToolPanel->Save_SequenceChecked(
			m_Catalog, Authoring_Placements(), Authoring_Deploy(), status))
		{
			const bool_t cameraRestored = !cameraDirty ||
				RestoreAuthoringBackup(cameraBackup);
			const bool_t sequenceRestored = RestoreAuthoringBackup(sequenceBackup);
			m_pWorldSequenceToolPanel->Restore_SequenceBaseline(
				previousExists, std::move(previousBytes), true);
			if (cameraRestored && sequenceRestored)
			{
				if (cameraDirty)
					DiscardAuthoringBackup(cameraBackup);
				DiscardAuthoringBackup(sequenceBackup);
				m_CutsceneSaveStatus =
					"World 저장 실패 - 카메라 저장도 되돌려 두 파일 모두 이전 상태입니다: " +
					status;
			}
			else
			{
				m_CutsceneSaveStatus =
					"World 저장 실패, 되돌리기도 실패했습니다. 백업 파일 "
					"(*.world-sequence-transaction.bak)을 남겨 두었습니다: " + status;
			}
			return false;
		}
	}

	/* 6. Both on disk: drop the backups and move the baselines forward. */
	if (cameraDirty)
	{
		DiscardAuthoringBackup(cameraBackup);
		m_strCameraShotBaselineText = cameraText;
		m_strCameraShotAreaId = descriptor->areaId;
		m_strCameraShotLoadedDocumentText =
			Build_CameraShotDocumentText(descriptor->areaId, 0u);
	}
	if (worldDirty)
		DiscardAuthoringBackup(sequenceBackup);
	m_CutsceneSaveStatus = std::string("저장했습니다 (저작본만, 게시 안 함): ") +
		(cameraDirty ? "카메라 revision " + std::to_string(revision) : std::string()) +
		((cameraDirty && worldDirty) ? " + " : "") +
		(worldDirty ? "World Sequence" : "") +
		". 이 편집기는 저작본을 바로 재생하며, 게임 런타임 파일은 별도 Publish 전까지 이전 상태입니다.";
	return true;
}

void Client::CMapTool::Render_CutsceneSection()
{
	if (m_Cutscenes.empty())
		return;
	ImGui::Separator();
	ImGui::TextUnformatted("컷신");
	ImGui::TextDisabled(
		"컷신 하나를 고르고 Play 를 누르면 카메라 컷과 배우가 같은 시계로 재생됩니다.");

	if (m_iSelectedCutscene >= m_Cutscenes.size())
		m_iSelectedCutscene = 0u;
	if (ImGui::BeginCombo("컷신##MapToolCutsceneSelect",
		m_Cutscenes[m_iSelectedCutscene].displayName.c_str()))
	{
		for (size_t index = 0; index < m_Cutscenes.size(); ++index)
		{
			const bool_t selected = index == m_iSelectedCutscene;
			if (ImGui::Selectable(m_Cutscenes[index].displayName.c_str(),
				selected))
			{
				/* Showing another cutscene while the previous one keeps driving
				   the camera and actors would judge the wrong thing: the old
				   session is stopped before the new row is shown. */
				if (index != m_iSelectedCutscene &&
					(EDITOR_CUTSCENE_STATE::STOPPED != m_eCutsceneState ||
						!m_CutsceneSessionInstanceIds.empty()))
				{
					Stop_EditorCutscene();
					m_CutsceneStatus = "이전 컷신 미리보기를 정지했습니다.";
				}
				if (index != m_iSelectedCutscene)
				{
					m_iSelectedCutsceneActor = 0u;
					m_iCutsceneSelectedActorKey = 0;
					m_CutsceneEditInstanceSnapshot.reset();
					m_CutsceneEditTemplateSnapshot.reset();
				}
				m_iSelectedCutscene = index;
			}
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	const EDITOR_CUTSCENE& cutscene = m_Cutscenes[m_iSelectedCutscene];
	const bool_t isSession = m_strCutsceneSessionId == cutscene.cutsceneId &&
		EDITOR_CUTSCENE_STATE::STOPPED != m_eCutsceneState;

	if (ImGui::Button("Play##MapToolCutscenePlay"))
		(void)Play_EditorCutscene(cutscene.cutsceneId);
	ImGui::SameLine();
	ImGui::BeginDisabled(!isSession);
	if (EDITOR_CUTSCENE_STATE::PLAYING == m_eCutsceneState)
	{
		if (ImGui::Button("Pause##MapToolCutscenePause"))
			m_eCutsceneState = EDITOR_CUTSCENE_STATE::PAUSED;
	}
	else
	{
		if (ImGui::Button("Resume##MapToolCutsceneResume"))
			m_eCutsceneState = EDITOR_CUTSCENE_STATE::PLAYING;
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop##MapToolCutsceneStop"))
		Stop_EditorCutscene();
	ImGui::EndDisabled();

	/* Scrubbing pauses first: dragging while the clock advances fights the
	   drag, and the editor is asking to hold one instant. */
	f32_t scrub = isSession ? m_fCutsceneSessionMs : 0.f;
	ImGui::SetNextItemWidth(360.f);
	ImGui::BeginDisabled(!isSession);
	if (ImGui::SliderFloat("Time (ms)##MapToolCutsceneTime", &scrub, 0.f,
		static_cast<f32_t>((std::max)(1, cutscene.durationMs)), "%.0f"))
	{
		m_eCutsceneState = EDITOR_CUTSCENE_STATE::PAUSED;
		m_fCutsceneSessionMs = std::clamp(scrub, 0.f,
			static_cast<f32_t>(cutscene.durationMs));
	}
	ImGui::EndDisabled();

	/* The camera hands back before the cutscene ends whenever the source did,
	   so both numbers are shown rather than one "length". */
	int32_t cameraEndMs = 0;
	for (const EDITOR_CUTSCENE_CUT& cut : cutscene.cameraCuts)
	{
		const EDITOR_CAMERA_SHOT* shot = Find_CameraShot(cut.shotId);
		if (nullptr != shot)
		{
			cameraEndMs = (std::max)(cameraEndMs,
				cut.startMs + shot->trackDurationMs);
		}
	}
	/* Registered is what the document names; active is what this session has
	   on stage right now. The two differ exactly when something failed. */
	size_t activeActors = 0u;
	if (isSession)
	{
		for (const std::string& instanceId : m_CutsceneSessionInstanceIds)
		{
			if (m_ArenaRisePlayer.Is_Playing(instanceId))
				++activeActors;
		}
	}
	ImGui::Text("전체 %d ms / 카메라 종료 %d ms / 컷 %zu개 / World 등록 %zu개 · 활성 %zu개",
		cutscene.durationMs, cameraEndMs, cutscene.cameraCuts.size(),
		cutscene.worldInstanceIds.size(), activeActors);
	if (cutscene.worldInstanceIds.empty())
		ImGui::TextDisabled("World 배우 없음 — 카메라만 재생합니다.");
	else if (isSession && !m_CutsceneWorldSource.empty())
		ImGui::TextDisabled("배우 출처: %s", m_CutsceneWorldSource.c_str());
	if (m_bCutsceneWorldFailed)
	{
		ImGui::TextColored(ImVec4(1.f, 0.45f, 0.35f, 1.f),
			"World 배우 실패 - 배우는 모두 해제됐습니다. 아래 사유를 확인하세요.");
	}
	if (!m_CutsceneStatus.empty())
		ImGui::TextWrapped("%s", m_CutsceneStatus.c_str());

	/* One Save for this Area's camera shots and World sequences, so a camera
	   edit is never reported as saved while its actor edit is not. */
	const EDITOR_AREA_DESCRIPTOR* saveArea = Get_ActiveEditorArea();
	const bool_t cameraDirty = Is_CameraShotDraftDirty();
	const bool_t worldDirty = nullptr != saveArea &&
		nullptr != m_pWorldSequenceToolPanel &&
		m_pWorldSequenceToolPanel->Is_Ready() &&
		m_pWorldSequenceToolPanel->Get_Document().Get_AreaId() == saveArea->areaId &&
		m_pWorldSequenceToolPanel->Is_Dirty();
	ImGui::BeginDisabled(!cameraDirty && !worldDirty);
	if (ImGui::Button("컷신 저장 (카메라 + World 저작본)##MapToolCutsceneSave"))
		(void)Save_CutsceneAuthoring();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (cameraDirty || worldDirty)
	{
		ImGui::Text("미저장: %s%s%s", cameraDirty ? "카메라" : "",
			(cameraDirty && worldDirty) ? " + " : "", worldDirty ? "World" : "");
	}
	else
	{
		ImGui::TextDisabled("저작본과 같음. 게임 런타임 게시는 이 버튼이 하지 않습니다.");
	}
	if (!m_CutsceneSaveStatus.empty())
		ImGui::TextWrapped("%s", m_CutsceneSaveStatus.c_str());

	/* The cut table is the timetable the plan asks for: which shot owns which
	   stretch, and which one is on screen right now. */
	if (ImGui::BeginTable("##MapToolCutsceneCuts", 5,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn("컷");
		ImGui::TableSetupColumn("샷");
		ImGui::TableSetupColumn("시작");
		ImGui::TableSetupColumn("길이");
		ImGui::TableSetupColumn("키");
		ImGui::TableHeadersRow();
		f32_t activeLocalMs = 0.f;
		const EDITOR_CUTSCENE_CUT* activeCut = isSession ?
			Find_CutsceneCutAt(cutscene, m_fCutsceneSessionMs, activeLocalMs) :
			nullptr;
		for (const EDITOR_CUTSCENE_CUT& cut : cutscene.cameraCuts)
		{
			const EDITOR_CAMERA_SHOT* shot = Find_CameraShot(cut.shotId);
			ImGui::TableNextRow();
			if (nullptr != activeCut && activeCut->cutId == cut.cutId)
			{
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
					IM_COL32(52, 82, 120, 255));
			}
			ImGui::TableSetColumnIndex(0);
			/* Selecting a cut opens its shot below, so the key editor and the
			   timetable always talk about the same thing. */
			if (ImGui::Selectable(cut.cutId.c_str(), false,
				ImGuiSelectableFlags_SpanAllColumns))
			{
				for (size_t index = 0; index < m_CameraShots.size(); ++index)
				{
					if (m_CameraShots[index].shotId == cut.shotId)
					{
						m_iSelectedCameraShot = index;
						break;
					}
				}
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(cut.shotId.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%d ms", cut.startMs);
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%d ms", nullptr != shot ? shot->trackDurationMs : 0);
			ImGui::TableSetColumnIndex(4);
			ImGui::Text("%zu", nullptr != shot ? shot->keyframes.size() : 0u);
		}
		ImGui::EndTable();
	}
	Render_CutsceneActorSection(cutscene, isSession);
}

void Client::CMapTool::Render_CutsceneActorSection(
	const EDITOR_CUTSCENE& cutscene, const bool_t isSession)
{
	if (cutscene.worldInstanceIds.empty())
		return;
	ImGui::Separator();
	ImGui::TextUnformatted("World 배우");
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	const bool_t hasDraft = nullptr != descriptor &&
		nullptr != m_pWorldSequenceToolPanel &&
		m_pWorldSequenceToolPanel->Is_Ready() &&
		m_pWorldSequenceToolPanel->Get_Document().Get_AreaId() ==
			descriptor->areaId;
	if (!hasDraft)
	{
		ImGui::TextDisabled(
			"이 Area 의 World Sequence 저작 draft 가 없어 배우를 편집할 수 없습니다. 재생은 게시본으로 합니다.");
		return;
	}
	if (!m_strWorldObjectPrototypeStatus.empty())
		ImGui::TextDisabled("%s", m_strWorldObjectPrototypeStatus.c_str());
	CWorldSequenceDocument& document =
		m_pWorldSequenceToolPanel->Get_MutableDocument();
	if (m_iSelectedCutsceneActor >= cutscene.worldInstanceIds.size())
		m_iSelectedCutsceneActor = 0u;

	/* One row per actor: what the document registers, what this session has
	   on stage, and whether its key track shows or hides it at T. A hidden
	   stretch is authored hiding, not a failure, and is labelled that way. */
	const auto localTimeOf = [this](const WORLD_SEQUENCE_INSTANCE& instance)
	{
		return (m_fCutsceneSessionMs -
			static_cast<f32_t>(instance.startDelayMs)) * instance.playbackSpeed;
	};
	const auto actorState = [&](const std::string& instanceId) -> std::string
	{
		const WORLD_SEQUENCE_INSTANCE* instance = document.Find_Instance(instanceId);
		const WORLD_SEQUENCE_TEMPLATE* sequence = nullptr == instance ? nullptr :
			document.Find_Template(instance->templateId);
		if (nullptr == instance || nullptr == sequence)
			return "문서에 없음 (등록 실패)";
		if (!isSession)
			return "등록됨 (정지)";
		const bool_t owned = m_CutsceneSessionInstanceIds.end() != std::find(
			m_CutsceneSessionInstanceIds.begin(),
			m_CutsceneSessionInstanceIds.end(), instanceId);
		if (!owned || !m_ArenaRisePlayer.Is_Playing(instanceId))
			return m_bCutsceneWorldFailed ? "실패 (해제됨)" : "비활성";
		const f32_t localMs = localTimeOf(*instance);
		if (localMs < 0.f)
			return "활성 - 시작 전 (정상 숨김)";
		bool_t visible = true;
		if (!sequence->tracks.empty())
		{
			for (const WORLD_SEQUENCE_TRANSFORM_KEY& key :
				sequence->tracks.front().keys)
			{
				if (static_cast<f32_t>(key.timeMs) > localMs)
					break;
				visible = key.visible;
			}
		}
		return visible ? "활성 - 표시" : "활성 - 키 visible=false (정상 숨김)";
	};

	if (ImGui::BeginTable("##MapToolCutsceneActors", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn("배우 (instance)");
		ImGui::TableSetupColumn("템플릿");
		ImGui::TableSetupColumn("모델");
		ImGui::TableSetupColumn("상태 (키 기준)");
		ImGui::TableHeadersRow();
		for (size_t index = 0; index < cutscene.worldInstanceIds.size(); ++index)
		{
			const std::string& instanceId = cutscene.worldInstanceIds[index];
			const WORLD_SEQUENCE_INSTANCE* instance = document.Find_Instance(instanceId);
			std::string model = "-";
			if (nullptr != instance)
			{
				for (const WORLD_SEQUENCE_BINDING& binding : instance->bindings)
				{
					if (WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE != binding.targetKind)
						continue;
					if (const auto* resource = document.Find_ObjectResource(binding.targetId))
					{
						model = resource->modelAssetId;
						break;
					}
				}
			}
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::PushID(static_cast<int>(index));
			if (ImGui::Selectable(instanceId.c_str(),
				index == m_iSelectedCutsceneActor,
				ImGuiSelectableFlags_SpanAllColumns))
			{
				m_iSelectedCutsceneActor = index;
				m_iCutsceneSelectedActorKey = 0;
				m_CutsceneEditInstanceSnapshot.reset();
				m_CutsceneEditTemplateSnapshot.reset();
			}
			ImGui::PopID();
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(nullptr != instance ? instance->templateId.c_str() : "-");
			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(model.c_str());
			ImGui::TableSetColumnIndex(3);
			ImGui::TextUnformatted(actorState(instanceId).c_str());
		}
		ImGui::EndTable();
	}

	const std::string instanceId = cutscene.worldInstanceIds[m_iSelectedCutsceneActor];
	WORLD_SEQUENCE_INSTANCE* instance = document.Find_Instance(instanceId);
	WORLD_SEQUENCE_TEMPLATE* sequence = nullptr == instance ? nullptr :
		document.Find_Template(instance->templateId);
	if (nullptr == instance || nullptr == sequence)
	{
		ImGui::TextDisabled("선택한 배우가 World 문서에 없습니다: %s", instanceId.c_str());
		return;
	}
	if (isSession)
	{
		ImGui::TextDisabled("플레이어 상태: %s",
			m_ArenaRisePlayer.Get_ObjectSampleStatus(instanceId).c_str());
	}

	/* The edit contract: while no widget is being dragged or typed into, the
	   selected actor's instance and template are snapshotted. A finished edit
	   is validated against the whole document; a failure restores the
	   snapshot, a success marks the World draft dirty and re-prepares the
	   preview at the current time. The document is only ever the draft - no
	   file is written until Save. */
	if (!ImGui::IsAnyItemActive())
	{
		m_CutsceneEditInstanceSnapshot = *instance;
		m_CutsceneEditTemplateSnapshot = *sequence;
	}
	const auto commitEdit = [&]()
	{
		std::string status;
		if (!m_pWorldSequenceToolPanel->Validate(m_Catalog,
			Authoring_Placements(), Authoring_Deploy(), status))
		{
			if (m_CutsceneEditInstanceSnapshot &&
				m_CutsceneEditInstanceSnapshot->instanceId == instance->instanceId)
			{
				*instance = *m_CutsceneEditInstanceSnapshot;
			}
			if (m_CutsceneEditTemplateSnapshot &&
				m_CutsceneEditTemplateSnapshot->sequenceId == sequence->sequenceId)
			{
				*sequence = *m_CutsceneEditTemplateSnapshot;
			}
			m_CutsceneActorEditStatus = "되돌림 - 문서 검증 실패: " + status;
			return;
		}
		m_pWorldSequenceToolPanel->Mark_ExternalEdit();
		m_bCutsceneWorldPreviewStale = true;
		m_CutsceneActorEditStatus =
			"적용됨 (미저장). 미리보기는 현재 시각에서 다시 준비됩니다.";
	};
	const auto commitIfEdited = [&]()
	{
		if (ImGui::IsItemDeactivatedAfterEdit())
			commitEdit();
	};

	ImGui::Separator();
	ImGui::Text("선택 배우: %s", instanceId.c_str());

	/* A template shared by several instances changes all of them. The editor
	   sees who else moves, and only an explicit copy makes it private. */
	std::vector<std::string> sharers;
	for (const WORLD_SEQUENCE_INSTANCE& other : document.Get_Instances())
	{
		if (other.templateId == instance->templateId &&
			other.instanceId != instance->instanceId)
		{
			sharers.push_back(other.instanceId);
		}
	}
	if (!sharers.empty())
	{
		ImGui::TextColored(ImVec4(1.f, 0.8f, 0.3f, 1.f),
			"이 템플릿은 다른 인스턴스 %zu개와 공유됩니다. 키·클립 편집은 모두에 반영됩니다:",
			sharers.size());
		for (const std::string& sharer : sharers)
			ImGui::BulletText("%s", sharer.c_str());
		if (ImGui::Button("이 배우 전용 템플릿으로 복제##MapToolCutsceneActorFork"))
		{
			std::string copyId = sequence->sequenceId + ".own";
			for (uint32_t suffix = 2u; nullptr != document.Find_Template(copyId); ++suffix)
				copyId = sequence->sequenceId + ".own" + std::to_string(suffix);
			if (!CWorldSequenceDocument::Is_ValidStableId(copyId))
			{
				m_CutsceneActorEditStatus = "복제 템플릿 ID 가 유효하지 않습니다: " + copyId;
				return;
			}
			const std::string previousTemplateId = instance->templateId;
			WORLD_SEQUENCE_TEMPLATE copy = *sequence;
			copy.sequenceId = copyId;
			document.Get_Templates().push_back(std::move(copy));
			/* push_back may have moved every template; look the instance up
			   again instead of using the pointers taken above. */
			WORLD_SEQUENCE_INSTANCE* owner = document.Find_Instance(instanceId);
			owner->templateId = copyId;
			std::string status;
			if (!m_pWorldSequenceToolPanel->Validate(m_Catalog,
				Authoring_Placements(), Authoring_Deploy(), status))
			{
				owner->templateId = previousTemplateId;
				auto& templates = document.Get_Templates();
				templates.erase(std::remove_if(templates.begin(), templates.end(),
					[&copyId](const WORLD_SEQUENCE_TEMPLATE& value)
					{ return value.sequenceId == copyId; }), templates.end());
				m_CutsceneActorEditStatus = "복제 되돌림 - 문서 검증 실패: " + status;
				return;
			}
			m_pWorldSequenceToolPanel->Mark_ExternalEdit();
			m_bCutsceneWorldPreviewStale = true;
			m_CutsceneEditInstanceSnapshot.reset();
			m_CutsceneEditTemplateSnapshot.reset();
			m_CutsceneActorEditStatus = "전용 템플릿으로 복제했습니다 (미저장): " + copyId;
			return;
		}
	}

	/* The instance anchor is absolute world space; every key offset below is
	   relative to it, and key rotation turns the actor about its own pivot,
	   never about the world origin. */
	ImGui::SetNextItemWidth(260.f);
	ImGui::DragFloat3("앵커 위치 (월드 절대, m)##MapToolCutsceneActorAnchor",
		&instance->position.x, 0.01f, 0.f, 0.f, "%.4f");
	commitIfEdited();
	ImGui::SetNextItemWidth(160.f);
	const uint32_t delayStep = 10u;
	const uint32_t delayFastStep = 100u;
	ImGui::InputScalar("시작 지연 (ms)##MapToolCutsceneActorDelay",
		ImGuiDataType_U32, &instance->startDelayMs, &delayStep, &delayFastStep);
	commitIfEdited();
	ImGui::SetNextItemWidth(160.f);
	ImGui::DragFloat("재생 속도##MapToolCutsceneActorSpeed",
		&instance->playbackSpeed, 0.01f, 0.05f, 8.f, "%.3f");
	commitIfEdited();

	ImGui::Text("템플릿 %s - 길이 %u ms - 보간 %s", sequence->sequenceId.c_str(),
		sequence->durationMs,
		CWorldSequenceDocument::Interpolation_ToString(sequence->interpolation));
	if (!sequence->tracks.empty())
	{
		WORLD_SEQUENCE_TRACK& transform = sequence->tracks.front();
		const int32_t keyCount = static_cast<int32_t>(transform.keys.size());
		m_iCutsceneSelectedActorKey =
			std::clamp(m_iCutsceneSelectedActorKey, 0, (std::max)(0, keyCount - 1));
		ImGui::Text("변환 키 (슬롯 %s, %d개): 오프셋은 앵커 기준, 회전은 배우 자기 중심",
			transform.slotId.c_str(), keyCount);
		ImGui::SetNextItemWidth(220.f);
		ImGui::SliderInt("키 번호##MapToolCutsceneActorKey",
			&m_iCutsceneSelectedActorKey, 0, (std::max)(0, keyCount - 1));
		const f32_t localMs = isSession ? localTimeOf(*instance) : -1.f;
		ImGui::SameLine();
		ImGui::BeginDisabled(localMs < 0.f);
		if (ImGui::Button("현재 시각의 키##MapToolCutsceneActorKeyAtT"))
		{
			for (int32_t keyIndex = 0; keyIndex < keyCount; ++keyIndex)
			{
				if (static_cast<f32_t>(transform.keys[keyIndex].timeMs) > localMs)
					break;
				m_iCutsceneSelectedActorKey = keyIndex;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("현재 시각에 키 추가##MapToolCutsceneActorKeyAdd"))
		{
			const uint32_t insertMs = static_cast<uint32_t>(std::clamp(
				localMs, 0.f, static_cast<f32_t>(sequence->durationMs)) + 0.5f);
			const auto at = std::find_if(transform.keys.begin(), transform.keys.end(),
				[insertMs](const WORLD_SEQUENCE_TRANSFORM_KEY& key)
				{ return key.timeMs >= insertMs; });
			if (transform.keys.end() != at && at->timeMs == insertMs)
			{
				m_iCutsceneSelectedActorKey =
					static_cast<int32_t>(at - transform.keys.begin());
				m_CutsceneActorEditStatus = "그 시각에는 이미 키가 있어 선택만 했습니다.";
			}
			else if (transform.keys.begin() != at)
			{
				/* The new key starts as a copy of the key before it, so the pose
				   does not jump until the editor changes it. */
				WORLD_SEQUENCE_TRANSFORM_KEY key = *(at - 1);
				key.timeMs = insertMs;
				const auto inserted = transform.keys.insert(at, key);
				m_iCutsceneSelectedActorKey =
					static_cast<int32_t>(inserted - transform.keys.begin());
				commitEdit();
			}
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		const bool_t boundaryKey = 0 == m_iCutsceneSelectedActorKey ||
			keyCount - 1 == m_iCutsceneSelectedActorKey;
		ImGui::BeginDisabled(boundaryKey || keyCount <= 2);
		if (ImGui::Button("선택 키 삭제##MapToolCutsceneActorKeyDelete"))
		{
			transform.keys.erase(transform.keys.begin() + m_iCutsceneSelectedActorKey);
			m_iCutsceneSelectedActorKey = (std::max)(0, m_iCutsceneSelectedActorKey - 1);
			commitEdit();
		}
		ImGui::EndDisabled();

		if (m_iCutsceneSelectedActorKey < static_cast<int32_t>(transform.keys.size()))
		{
			WORLD_SEQUENCE_TRANSFORM_KEY& key =
				transform.keys[static_cast<size_t>(m_iCutsceneSelectedActorKey)];
			const bool_t fixedTime = 0 == m_iCutsceneSelectedActorKey ||
				static_cast<int32_t>(transform.keys.size()) - 1 == m_iCutsceneSelectedActorKey;
			ImGui::BeginDisabled(fixedTime);
			ImGui::SetNextItemWidth(160.f);
			ImGui::InputScalar("키 시각 (템플릿 로컬 ms)##MapToolCutsceneActorKeyTime",
				ImGuiDataType_U32, &key.timeMs);
			commitIfEdited();
			ImGui::EndDisabled();
			if (fixedTime)
			{
				ImGui::SameLine();
				ImGui::TextDisabled("첫 키는 0, 마지막 키는 길이로 고정");
			}
			ImGui::SetNextItemWidth(260.f);
			ImGui::DragFloat3("키 오프셋 (앵커 기준, m)##MapToolCutsceneActorKeyOffset",
				&key.positionOffset.x, 0.01f, 0.f, 0.f, "%.4f");
			commitIfEdited();
			/* Only a pure yaw quaternion is edited as degrees. Anything with
			   pitch or roll is shown, not rewritten, so it is never flattened. */
			const float4_t& quaternion = key.rotationQuaternion;
			if (std::abs(quaternion.x) < 1e-4f && std::abs(quaternion.z) < 1e-4f)
			{
				f32_t yawDegrees = XMConvertToDegrees(
					2.f * std::atan2(quaternion.y, quaternion.w));
				ImGui::SetNextItemWidth(160.f);
				if (ImGui::DragFloat("yaw (도, 배우 자기 중심)##MapToolCutsceneActorKeyYaw",
					&yawDegrees, 0.25f, -360.f, 360.f, "%.2f"))
				{
					const f32_t half = XMConvertToRadians(yawDegrees) * 0.5f;
					key.rotationQuaternion = float4_t(0.f, std::sin(half), 0.f, std::cos(half));
				}
				commitIfEdited();
			}
			else
			{
				ImGui::TextDisabled("회전 (x,y,z,w) %.4f %.4f %.4f %.4f - pitch/roll 이 있어 yaw 로 편집하지 않습니다",
					quaternion.x, quaternion.y, quaternion.z, quaternion.w);
			}
			ImGui::SetNextItemWidth(260.f);
			ImGui::DragFloat3("크기 배율##MapToolCutsceneActorKeyScale",
				&key.scaleMultiplier.x, 0.01f, 0.01f, 100.f, "%.3f");
			commitIfEdited();
			bool_t visible = key.visible;
			if (ImGui::Checkbox("보이기 (visible)##MapToolCutsceneActorKeyVisible", &visible))
			{
				key.visible = visible;
				commitEdit();
			}
		}
	}

	if (!sequence->animationTracks.empty())
	{
		ImGui::TextUnformatted("애니메이션 클립 (같은 슬롯은 시작 시각 순으로 이어 재생)");
		ImGui::TextDisabled("클립 이름이 모델에 없으면 재생 중 'World Object clip is absent' 로 실패가 표시됩니다.");
		if (ImGui::BeginTable("##MapToolCutsceneActorClips", 6,
			ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("클립");
			ImGui::TableSetupColumn("시작 ms");
			ImGui::TableSetupColumn("소스 시작 ms");
			ImGui::TableSetupColumn("배속");
			ImGui::TableSetupColumn("loop");
			ImGui::TableSetupColumn("끝 유지");
			ImGui::TableHeadersRow();
			std::unordered_set<std::string> seenSlots;
			for (size_t index = 0; index < sequence->animationTracks.size(); ++index)
			{
				WORLD_SEQUENCE_ANIMATION_TRACK& clip = sequence->animationTracks[index];
				const bool_t firstOfSlot = seenSlots.insert(clip.slotId).second;
				ImGui::PushID(static_cast<int>(index));
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				char clipBuffer[129]{};
				std::snprintf(clipBuffer, sizeof(clipBuffer), "%s", clip.clipName.c_str());
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (ImGui::InputText("##clip", clipBuffer, sizeof(clipBuffer)))
					clip.clipName = clipBuffer;
				commitIfEdited();
				ImGui::TableSetColumnIndex(1);
				ImGui::BeginDisabled(firstOfSlot);
				ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::InputScalar("##start", ImGuiDataType_U32, &clip.startMs);
				commitIfEdited();
				ImGui::EndDisabled();
				ImGui::TableSetColumnIndex(2);
				ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::InputScalar("##source", ImGuiDataType_U32, &clip.sourceStartMs);
				commitIfEdited();
				ImGui::TableSetColumnIndex(3);
				ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::DragFloat("##rate", &clip.playbackRate, 0.01f, 0.05f, 8.f, "%.3f");
				commitIfEdited();
				ImGui::TableSetColumnIndex(4);
				bool_t loop = clip.loop;
				if (ImGui::Checkbox("##loop", &loop))
				{
					clip.loop = loop;
					commitEdit();
				}
				ImGui::TableSetColumnIndex(5);
				bool_t hold = clip.holdLastFrame;
				if (ImGui::Checkbox("##hold", &hold))
				{
					clip.holdLastFrame = hold;
					commitEdit();
				}
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}
	if (!m_CutsceneActorEditStatus.empty())
		ImGui::TextWrapped("%s", m_CutsceneActorEditStatus.c_str());
}

void Client::CMapTool::End_CameraShotPreview()
{
	const shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (m_bCameraShotPreviewActive && nullptr != camera)
		camera->End_PresentationOverride(CAMERA_SHOT_PREVIEW_OWNER_ID);
	m_bCameraShotPreviewActive = false;
}

void Client::CMapTool::Render_CameraShotSection()
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	ImGui::TextUnformatted("Camera Shots");
	if (nullptr == descriptor || descriptor->cameraShotDocument.empty())
	{
		ImGui::TextDisabled("This Area declares no camera shot document.");
		return;
	}
	const shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	ImGui::TextDisabled(
		"Fly the Free Camera to the framing you want, then capture. The product level holds that pose while a player stands in the box.");
	ImGui::TextWrapped("%s", m_CameraShotStatus.c_str());

	/* The workspace camera pose is the authored pose, so read it from the
	   pipeline and a capture records exactly what is on screen. */
	const float4_t* cameraPosition = CGameInstance::Get().Get_CamPosition();
	const float4x4_t* viewMatrix =
		CGameInstance::Get().Get_Transform(D3DTS::VIEW);
	float3_t editorEye{};
	float3_t editorLookAt{};
	bool_t hasEditorPose = false;
	if (nullptr != cameraPosition && nullptr != viewMatrix)
	{
		editorEye = float3_t(
			cameraPosition->x, cameraPosition->y, cameraPosition->z);
		const vector_t forward = XMVector3Normalize(XMVectorSet(
			viewMatrix->_13, viewMatrix->_23, viewMatrix->_33, 0.f));
		XMStoreFloat3(&editorLookAt,
			XMLoadFloat3(&editorEye) + forward * 10.f);
		hasEditorPose = true;
		ImGui::Text("Editor camera: %.2f, %.2f, %.2f",
			editorEye.x, editorEye.y, editorEye.z);
	}

	ImGui::BeginDisabled(!hasEditorPose || m_CameraShots.size() >= 64u);
	if (ImGui::Button("Add Shot From Camera"))
	{
		EDITOR_CAMERA_SHOT shot;
		size_t suffix = m_CameraShots.size() + 1u;
		do
		{
			shot.shotId = "shot." + std::to_string(suffix++);
		}
		while (m_CameraShots.end() != std::find_if(
			m_CameraShots.begin(), m_CameraShots.end(),
			[&shot](const EDITOR_CAMERA_SHOT& value)
			{
				return value.shotId == shot.shotId;
			}));
		shot.eye = editorEye;
		shot.lookAt = editorLookAt;
		/* The box starts where the shot looks, which is the ground the shot
		   was framed for. */
		shot.center = editorLookAt;
		m_CameraShots.push_back(std::move(shot));
		m_iSelectedCameraShot = m_CameraShots.size() - 1u;
		m_CameraShotStatus = "Added a shot from the editor camera";
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Save Shots"))
		(void)Save_CameraShots();
	ImGui::SameLine();
	if (ImGui::Button("Reload Shots"))
	{
		/* Reload replaces the draft, so unsaved camera edits are dropped only
		   on a second, explicit press. */
		if (Is_CameraShotDraftDirty() && !m_bCameraShotReloadConfirmPending)
		{
			m_bCameraShotReloadConfirmPending = true;
			m_CameraShotStatus =
				"미저장 카메라 변경이 있습니다. Reload Shots 를 한 번 더 누르면 버리고 다시 읽습니다.";
		}
		else
		{
			(void)Load_CameraShots(*descriptor);
		}
	}
	if (m_bCameraShotPreviewActive)
	{
		ImGui::SameLine();
		if (ImGui::Button("Stop Preview"))
			End_CameraShotPreview();
	}

	bool_t hasRemoval = false;
	size_t removeIndex = 0u;
	for (size_t index = 0; index < m_CameraShots.size(); ++index)
	{
		EDITOR_CAMERA_SHOT& shot = m_CameraShots[index];
		ImGui::PushID(static_cast<int>(index));
		if (ImGui::CollapsingHeader(shot.shotId.c_str()))
		{
			/* The open header is the shot being edited, so the track preview
			   and the key buttons below judge the same shot. */
			m_iSelectedCameraShot = index;
			char idBuffer[129]{};
			std::snprintf(idBuffer, sizeof(idBuffer), "%s", shot.shotId.c_str());
			if (ImGui::InputText("Shot ID", idBuffer, sizeof(idBuffer)))
				shot.shotId = idBuffer;
			char sequenceBuffer[129]{};
			std::snprintf(sequenceBuffer, sizeof(sequenceBuffer), "%s",
				shot.sequenceInstanceId.c_str());
			if (ImGui::InputText("Sequence Instance (empty = use box)",
				sequenceBuffer, sizeof(sequenceBuffer)))
			{
				shot.sequenceInstanceId = sequenceBuffer;
			}
			ImGui::DragFloat3("Box Center", &shot.center.x, 0.1f);
			ImGui::DragFloat3("Box Half Extents", &shot.halfExtents.x,
				0.1f, 0.1f, 1000.f);
			ImGui::DragFloat("Box Yaw", &shot.yawDegrees, 0.5f, -360.f, 360.f);
			ImGui::DragFloat3("Eye", &shot.eye.x, 0.1f);
			ImGui::DragFloat3("Look At", &shot.lookAt.x, 0.1f);
			ImGui::DragFloat("Fov Y", &shot.fovYDegrees, 0.25f, 5.f, 170.f);
			ImGui::DragInt("Blend In (ms)", &shot.blendInMs, 10.f, 0, 10000);
			ImGui::DragInt("Blend Out (ms)", &shot.blendOutMs, 10.f, 0, 10000);
			ImGui::DragInt("Priority", &shot.priority, 1.f, 0, 1000);

			ImGui::SeparatorText("Follow");
			ImGui::TextDisabled(
				"On: the shot slides with the local Character. Both offsets are added to that Character's position.");
			ImGui::Checkbox("Follows Player", &shot.followsPlayer);
			ImGui::BeginDisabled(!shot.followsPlayer);
			ImGui::DragFloat3("Follow Eye Offset", &shot.followEyeOffset.x, 0.1f);
			ImGui::DragFloat3("Follow Look Offset",
				&shot.followLookAtOffset.x, 0.1f);
			ImGui::EndDisabled();

			ImGui::SeparatorText("Camera Track");
			ImGui::TextDisabled(
				"Two or more keys make the shot move on the bound sequence clock. Fewer keeps the single pose.");

			/* Hold the running cutscene so a key is judged against the frame it
			   actually governs instead of guessed from a still. */
			bool_t cutscenePlaying = Is_ShotCutsceneClockPlaying(shot);
			const bool_t cutsceneHeld = 0.f <= m_fCutsceneScrubMs;
			if (ImGui::Button(cutsceneHeld ? "Resume Cutscene" : "Hold Cutscene"))
			{
				if (cutsceneHeld)
				{
					m_fCutsceneScrubMs = -1.f;
					m_fCutsceneLoopStartMs = -1.f;
					m_fCutsceneLoopEndMs = -1.f;
					m_CameraShotStatus = "Cutscene resumed";
				}
				else
				{
					/* Start the cutscene here if no other panel has, so the
					   camera work never depends on visiting another tab. */
					/* Both previews hold the camera at the same priority, and
					   holding a clock is a request to see the track, so the
					   walkthrough yields here instead of blocking silently. */
					Stop_MarioIntro();
					if (!cutscenePlaying)
						cutscenePlaying = Ensure_ShotCutsceneClock(shot);
					f32_t nowMs = 0.f;
					if (!shot.sequenceInstanceId.empty())
					{
						(void)m_ArenaRisePlayer.Try_GetElapsedMs(
							shot.sequenceInstanceId, nowMs);
					}
					m_fCutsceneScrubMs = (std::max)(0.f, nowMs);
					/* On failure Ensure_ShotCutsceneClock already wrote the
					   reason, so leave it where the panel shows it. */
					if (cutscenePlaying)
						m_CameraShotStatus = "Cutscene held for editing";
				}
			}
			if (cutsceneHeld)
			{
				const f32_t span = (std::max)(1.f,
					m_ArenaRisePlayer.Get_LongestElapsedSpanMs());
				f32_t scrub = m_fCutsceneScrubMs;
				ImGui::SetNextItemWidth(360.f);
				if (ImGui::SliderFloat("Cutscene Time (ms)", &scrub,
					0.f, span, "%.0f"))
				{
					m_fCutsceneScrubMs = std::clamp(scrub, 0.f, span);
				}
				ImGui::SameLine();
				if (ImGui::Button("-100"))
				{
					m_fCutsceneScrubMs =
						(std::max)(0.f, m_fCutsceneScrubMs - 100.f);
				}
				ImGui::SameLine();
				if (ImGui::Button("+100"))
				{
					m_fCutsceneScrubMs =
						(std::min)(span, m_fCutsceneScrubMs + 100.f);
				}
			}
			if (0.f <= m_fCutsceneLoopStartMs)
			{
				ImGui::SameLine();
				ImGui::Text("Looping %.0f - %.0f ms", m_fCutsceneLoopStartMs,
					m_fCutsceneLoopEndMs);
			}

			ImGui::Text("Keys: %zu", shot.keyframes.size());
			Render_CameraTrackTimeline(shot);
			ImGui::DragInt("Track Duration (ms)", &shot.trackDurationMs,
				10.f, 0, 120000);
			ImGui::Combo("Interpolation", &shot.interpolationIndex,
				"LINEAR\0CATMULL_ROM\0\0");
			ImGui::Combo("Easing", &shot.easingIndex,
				"LINEAR\0SMOOTHSTEP\0HOLD\0\0");
			ImGui::BeginDisabled(!hasEditorPose);
			if (ImGui::Button("Append Key From Camera"))
			{
				EDITOR_CAMERA_KEYFRAME keyframe{};
				char sceneBuffer[192]{};
				(void)std::snprintf(sceneBuffer, sizeof(sceneBuffer), "%s.k%02zu",
					shot.shotId.c_str(), shot.keyframes.size());
				keyframe.sceneId = sceneBuffer;
				/* A held cutscene gives the exact moment the framing was judged
				   against, so the key lands there. Without a held clock the old
				   one-second step stays, and the first key always starts at 0. */
				if (shot.keyframes.empty())
				{
					keyframe.timeMs = 0;
				}
				else if (0.f <= m_fCutsceneScrubMs)
				{
					keyframe.timeMs = std::clamp(
						static_cast<int32_t>(std::lround(m_fCutsceneScrubMs)),
						shot.keyframes.back().timeMs + 1, 120000);
				}
				else
				{
					keyframe.timeMs = shot.keyframes.back().timeMs + 1000;
				}
				keyframe.eye = editorEye;
				keyframe.lookAt = editorLookAt;
				keyframe.fovYDegrees = shot.fovYDegrees;
				shot.trackDurationMs = (std::max)(shot.trackDurationMs,
					keyframe.timeMs);
				shot.keyframes.push_back(std::move(keyframe));
				m_CameraShotStatus = "Appended a camera key to " + shot.shotId;
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Clear Keys"))
			{
				shot.keyframes.clear();
				shot.trackDurationMs = 0;
				m_CameraShotStatus = "Cleared the camera track of " + shot.shotId;
			}
			if (m_iCutsceneSelectedKey >=
				static_cast<int32_t>(shot.keyframes.size()))
			{
				m_iCutsceneSelectedKey = -1;
			}
			if (!shot.keyframes.empty())
			{
				ImGui::TextDisabled(
					"Pick a key, jump to its moment, then drag Eye/Look/Fov and watch the frame.");
				if (ImGui::BeginListBox("Keys##cameratrack",
					ImVec2(420.f, 140.f)))
				{
					for (size_t keyIndex = 0; keyIndex < shot.keyframes.size();
						++keyIndex)
					{
						const EDITOR_CAMERA_KEYFRAME& row =
							shot.keyframes[keyIndex];
						char label[224]{};
						(void)std::snprintf(label, sizeof(label),
							"%2zu   %6d ms   eye %.1f %.1f %.1f   fov %.0f",
							keyIndex, row.timeMs, row.eye.x, row.eye.y,
							row.eye.z, row.fovYDegrees);
						const bool_t selected = m_iCutsceneSelectedKey ==
							static_cast<int32_t>(keyIndex);
						if (ImGui::Selectable(label, selected))
						{
							m_iCutsceneSelectedKey =
								static_cast<int32_t>(keyIndex);
							if (cutscenePlaying)
							{
								m_fCutsceneScrubMs =
									static_cast<f32_t>(row.timeMs);
							}
						}
					}
					ImGui::EndListBox();
				}
			}
			if (0 <= m_iCutsceneSelectedKey &&
				m_iCutsceneSelectedKey <
					static_cast<int32_t>(shot.keyframes.size()))
			{
				EDITOR_CAMERA_KEYFRAME& keyframe =
					shot.keyframes[static_cast<size_t>(m_iCutsceneSelectedKey)];
				ImGui::PushID(4200 + m_iCutsceneSelectedKey);
				ImGui::Text("Editing %s", keyframe.sceneId.c_str());
				ImGui::BeginDisabled(!cutscenePlaying);
				if (ImGui::Button("Jump To This Key"))
				{
					m_fCutsceneScrubMs = static_cast<f32_t>(keyframe.timeMs);
					m_CameraShotStatus = "Held at " + keyframe.sceneId;
				}
				ImGui::EndDisabled();
				ImGui::SameLine();
				const bool_t looping = 0.f <= m_fCutsceneLoopStartMs;
				if (ImGui::Button(looping ? "Stop Key Loop" : "Play This Key"))
				{
					if (looping)
					{
						m_fCutsceneLoopStartMs = -1.f;
						m_fCutsceneLoopEndMs = -1.f;
						m_CameraShotStatus = "Key loop stopped";
					}
					else
					{
						/* A loop over a clock that never started would sit on one
						   frame and hide the reason, so stop here and keep it. */
						Stop_MarioIntro();
						if (!Is_ShotCutsceneClockPlaying(shot) &&
							!Ensure_ShotCutsceneClock(shot))
						{
							ImGui::PopID();
							return;
						}
						/* The key owns the stretch from the previous key to the
						   next one, so the loop shows the whole move it steers
						   rather than a single frozen instant. */
						const size_t self =
							static_cast<size_t>(m_iCutsceneSelectedKey);
						const f32_t startMs = 0u == self ?
							static_cast<f32_t>(shot.keyframes.front().timeMs) :
							static_cast<f32_t>(shot.keyframes[self - 1u].timeMs);
						const f32_t endMs = self + 1u < shot.keyframes.size() ?
							static_cast<f32_t>(shot.keyframes[self + 1u].timeMs) :
							static_cast<f32_t>(keyframe.timeMs) + 1000.f;
						m_fCutsceneLoopStartMs = startMs;
						m_fCutsceneLoopEndMs = (std::max)(startMs + 100.f, endMs);
						m_fCutsceneScrubMs = startMs;
						m_CameraShotStatus = "Looping " + keyframe.sceneId;
					}
				}
				ImGui::SameLine();
				ImGui::BeginDisabled(!hasEditorPose);
				if (ImGui::Button("Set From Free Camera"))
				{
					keyframe.eye = editorEye;
					keyframe.lookAt = editorLookAt;
					m_CameraShotStatus = "Set " + keyframe.sceneId +
						" from the free camera";
				}
				ImGui::EndDisabled();
				/* The runtime requires the first key at 0 ms and strictly rising
				   times, so the numeric field obeys the same neighbour bounds the
				   timeline drag does. */
				const size_t keyIndexForTime =
					static_cast<size_t>(m_iCutsceneSelectedKey);
				const int32_t keyTimeLowerMs = 0u == keyIndexForTime ? 0 :
					shot.keyframes[keyIndexForTime - 1u].timeMs + 1;
				const int32_t keyTimeUpperMs = 0u == keyIndexForTime ? 0 :
					(keyIndexForTime + 1u < shot.keyframes.size() ?
						shot.keyframes[keyIndexForTime + 1u].timeMs - 1 : 120000);
				ImGui::BeginDisabled(0u == keyIndexForTime);
				if (ImGui::DragInt("Key Time (ms)", &keyframe.timeMs,
					10.f, keyTimeLowerMs, (std::max)(keyTimeLowerMs, keyTimeUpperMs)))
				{
					keyframe.timeMs = std::clamp(keyframe.timeMs,
						keyTimeLowerMs, (std::max)(keyTimeLowerMs, keyTimeUpperMs));
				}
				ImGui::EndDisabled();
				if (0u == keyIndexForTime)
					ImGui::TextDisabled("The first key stays at 0 ms.");
				ImGui::DragFloat3("Key Eye", &keyframe.eye.x, 0.05f);
				ImGui::DragFloat3("Key Look At", &keyframe.lookAt.x, 0.05f);
				ImGui::DragFloat("Key Fov Y", &keyframe.fovYDegrees,
					0.25f, 5.f, 170.f);
				/* Orbit the key around what it looks at, so framing can be
				   nudged by angle instead of by three raw axes. */
				const float3_t toEye(keyframe.eye.x - keyframe.lookAt.x,
					keyframe.eye.y - keyframe.lookAt.y,
					keyframe.eye.z - keyframe.lookAt.z);
				f32_t orbitRadius = std::sqrt(toEye.x * toEye.x +
					toEye.y * toEye.y + toEye.z * toEye.z);
				f32_t orbitYaw =
					XMConvertToDegrees(std::atan2(toEye.z, toEye.x));
				f32_t orbitPitch = orbitRadius > 0.0001f ?
					XMConvertToDegrees(std::asin(
						std::clamp(toEye.y / orbitRadius, -1.f, 1.f))) : 0.f;
				bool_t orbitChanged = false;
				if (ImGui::DragFloat("Orbit Yaw", &orbitYaw, 0.5f, -360.f, 360.f))
					orbitChanged = true;
				if (ImGui::DragFloat("Orbit Pitch", &orbitPitch, 0.5f, -89.f, 89.f))
					orbitChanged = true;
				if (ImGui::DragFloat("Orbit Distance", &orbitRadius,
					0.05f, 0.05f, 400.f))
				{
					orbitChanged = true;
				}
				if (orbitChanged)
				{
					const f32_t yaw = XMConvertToRadians(orbitYaw);
					const f32_t pitch = XMConvertToRadians(
						std::clamp(orbitPitch, -89.f, 89.f));
					keyframe.eye = float3_t(
						keyframe.lookAt.x +
							orbitRadius * std::cos(pitch) * std::cos(yaw),
						keyframe.lookAt.y + orbitRadius * std::sin(pitch),
						keyframe.lookAt.z +
							orbitRadius * std::cos(pitch) * std::sin(yaw));
				}
				if (ImGui::Button("Delete This Key"))
				{
					shot.keyframes.erase(shot.keyframes.begin() +
						m_iCutsceneSelectedKey);
					m_iCutsceneSelectedKey = -1;
					m_CameraShotStatus =
						"Deleted a camera key from " + shot.shotId;
				}
				ImGui::PopID();
			}

			ImGui::BeginDisabled(!hasEditorPose);
			if (ImGui::Button("Capture Camera Into Shot"))
			{
				shot.eye = editorEye;
				shot.lookAt = editorLookAt;
				m_CameraShotStatus = "Captured the editor camera into " +
					shot.shotId;
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(nullptr == camera);
			if (ImGui::Button("Preview Shot"))
			{
				if (nullptr != camera &&
					camera->Begin_PresentationOverride(
						CAMERA_SHOT_PREVIEW_OWNER_ID,
						CCamera::PRESENTATION_PRIORITY::AUTHORING_PREVIEW) &&
					camera->Apply_PresentationPose(
						CAMERA_SHOT_PREVIEW_OWNER_ID,
						shot.eye, shot.lookAt, shot.fovYDegrees))
				{
					m_bCameraShotPreviewActive = true;
					m_CameraShotStatus = "Previewing " + shot.shotId;
				}
				else
				{
					m_CameraShotStatus = "Preview was rejected: " + shot.shotId;
				}
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Remove Shot"))
			{
				hasRemoval = true;
				removeIndex = index;
			}
		}
		ImGui::PopID();
	}
	if (hasRemoval)
	{
		m_CameraShots.erase(
			m_CameraShots.begin() + static_cast<std::ptrdiff_t>(removeIndex));
		m_CameraShotStatus = "Removed a shot";
	}
}
