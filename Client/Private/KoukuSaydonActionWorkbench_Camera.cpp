// Composition의 카메라 occurrence와 Area 소유 shot 편집을 연결한다. 공유 shot의 소비자를 보여 주며 저장은 기존 Level 카메라 저작 경로에 맡긴다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"
#include "KoukuSaydonActionWorkbench_CameraLookup.h"

// 이 도메인에서 직접 사용하는 런타임/저작 자료의 선언이다.
#include "Level_KoukuSaydonArena.h"
#include "CameraTool.h"
#include "ValtanCinematicCameraController.h"

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

// 같은 stable shot을 재생하는 Composition 박스와 Area AUTO trigger를 모두 수집한다.
// 공유 shot 편집 잠금을 판단하는 두 Camera 화면이 사용한다. 참조를 가진 Pattern/Bundle 박스와
// Area 자동 재생 설정을 문자열 목록으로 복사하며 문서나 shot은 바꾸지 않는다.
std::vector<std::string> Camera_Consumers(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const CLevel_KoukuSaydonArena::KOUKU_CAMERA_SHOT &shot)
{
	// 같은 shot을 참조해도 occurrence마다 별도 소비자다. 집합으로 합치지 않고 읽은 순서의 문자열 사본을 모아 공유 편집 영향 범위를 보여 준다.
	std::vector<std::string> consumers;
	const auto collect = [&](const std::string &owner,
							 const std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE> &rows) {
		for (const auto &row : rows)
			for (const auto &resource : document.PresentationResources)
				if (resource.strResourceId == row.strResourceId &&
					resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA && resource.strAssetId == shot.strShotId)
					consumers.push_back(
						owner + " / " + row.strOccurrenceId + " @ " + std::to_string(row.iStartMs) + " ms");
	};
	for (const auto &pattern : document.Patterns)
		collect(pattern.strDisplayName, pattern.PresentationOccurrences);
	for (const auto &bundle : document.Bundles)
		collect(bundle.strDisplayName, bundle.PresentationOccurrences);
	if (!shot.bPatternOnly)
		consumers.push_back(shot.strSequenceInstanceId.empty() ? std::string("Area AUTO camera box")
															   : "Area AUTO with " + shot.strSequenceInstanceId);
	return consumers;
}
} // 이 파일의 보조 구현 범위 끝

// Resources와 Presentation Box Detail에서 선택한 shot의 간단한 편집기를 그린다.
// 입력 shot ID는 Level 목록을 찾는 동안만 차용하고, 수정은 shot 사본을 Update_CameraShot에 제출한다.
// Composition 박스 ID는 별도 Camera 창의 시간 문맥으로만 보관하며 Area 저장은 Save Camera가 수행한다.
void Client::CKoukuSaydonActionWorkbench::Render_CameraAuthoring(
	const std::string_view shotId, const std::string_view patternId, const std::string_view occurrenceId)
{
	if (ImGui::Button("Open Composition Camera"))
	{
		m_strCameraWindowShotId = std::string(shotId);
		m_strCameraKeyId.clear();
		m_bCameraWindowOpen = true;
		// Sequence 박스에서 열면 그 시계를 함께 사용하고, Resource 행에서 열면 shot만 편집한다.
		m_strCameraWindowPatternId = std::string(patternId);
		m_strCameraWindowOccurrenceId = std::string(occurrenceId);
	}
	auto *level = CLevel_KoukuSaydonArena::Get_Active();
	const auto *selected = Find_AuthoringCamera(shotId);
	if (!level || !selected)
	{
		ImGui::TextDisabled("Enter KoukuSaydon to edit this Area Camera shot.");
		return;
	}
	auto shot = *selected;
	const auto consumers = Camera_Consumers(m_Draft, shot);
	const bool locked = consumers.size() > 1u && m_strCameraSharedEditShotId != shot.strShotId;
	ImGui::PushID(shot.strShotId.c_str());
	ImGui::SeparatorText("Camera Shot / Area source");
	ImGui::TextDisabled(
		"%s | %s", shot.strShotId.c_str(), shot.bPatternOnly ? "PATTERN_ONLY" : "AUTO (existing Area trigger)");
	if (locked)
		ImGui::TextWrapped(
			"Shared by %zu consumers. Open Composition Camera to review them, then allow shared edits or make a dedicated shot for this box.",
			consumers.size());
	ImGui::BeginDisabled(locked);
	char name[129]{};
	(void)Copy_Text(name, std::size(name), shot.strDisplayName);
	bool changed = ImGui::InputText("Camera name", name, std::size(name));
	if (changed)
		shot.strDisplayName = name;
	int entry = static_cast<int>(shot.iBlendInMs), hold = static_cast<int>(shot.iDefaultHoldMs),
		exit = static_cast<int>(shot.iBlendOutMs);
	if (ImGui::InputInt("Blend in ms", &entry))
	{
		shot.iBlendInMs = std::clamp(entry, 0, 10000);
		changed = true;
	}
	if (ImGui::InputInt("Default hold ms", &hold))
	{
		shot.iDefaultHoldMs = std::clamp(hold, 0, 600000);
		changed = true;
	}
	if (ImGui::InputInt("Return ms", &exit))
	{
		shot.iBlendOutMs = std::clamp(exit, 0, 10000);
		changed = true;
	}
	int easing = shot.eTransitionEasing == VALTAN_CINEMATIC_CAMERA_EASING::LINEAR ? 0 : 1;
	if (ImGui::Combo("Transition", &easing, "LINEAR\0SMOOTHSTEP\0"))
	{
		shot.eTransitionEasing =
			easing == 0 ? VALTAN_CINEMATIC_CAMERA_EASING::LINEAR : VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
		changed = true;
	}
	int anchor = shot.followsPlayer ? 1 : 0;
	// PLAYER 구도는 track을 제거하므로 track을 가진 shot에서는 WORLD를 유지한다.
	ImGui::BeginDisabled(shot.hasCameraTrack && !shot.followsPlayer);
	if (ImGui::Combo("Shot anchor", &anchor, "WORLD\0PLAYER\0"))
	{
		shot.followsPlayer = anchor == 1;
		if (shot.followsPlayer)
		{
			shot.hasCameraTrack = false;
			shot.vFollowEyeOffset = shot.vEye;
			shot.vFollowLookAtOffset = shot.vLookAt;
		}
		changed = true;
	}
	ImGui::EndDisabled();
	if (changed)
		(void)level->Update_CameraShot(shot, m_strStatus);
	ImGui::EndDisabled();
	ImGui::Text("Eye (%.3f, %.3f, %.3f)", shot.vEye.x, shot.vEye.y, shot.vEye.z);
	ImGui::Text(
		"Look at (%.3f, %.3f, %.3f), FOV %.2f", shot.vLookAt.x, shot.vLookAt.y, shot.vLookAt.z, shot.fFovYDegrees);
	ImGui::BeginDisabled(locked);
	if (shot.hasCameraTrack)
	{
		ImGui::TextWrapped(
			"This shot has a camera track. Edit one position in Composition Camera; replacing the whole track is a separate confirmed action.");
		if (ImGui::Button("Replace whole track with current view..."))
			ImGui::OpenPopup("Replace Camera Track##CameraAuthoring");
		if (ImGui::BeginPopupModal("Replace Camera Track##CameraAuthoring", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextWrapped(
				"Every position of %s becomes one static pose captured from the current view.", shot.strShotId.c_str());
			if (ImGui::Button("Replace whole track"))
			{
				(void)level->Capture_CameraShot(shot.strShotId, m_strStatus);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}
	else if (ImGui::Button("Set Camera Pos / Capture view"))
		(void)level->Capture_CameraShot(shot.strShotId, m_strStatus);
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Save Camera"))
	{
		if (level->Save_CameraShots(m_strStatus))
			m_bPresentationResourceRefreshRequested = true;
	}
	ImGui::TextWrapped(
		"Set Camera Pos captures eye, lookAt and FOV together. PLAYER captures offsets from the local player. Save writes the Area source immediately; Preview needs no publish.");
	ImGui::TextWrapped(
		"Append uses blend-in + default hold. The box end begins the separate return tail. Use F6 Follow before Play to keep gameplay movement and return to the moving player.");
	ImGui::PopID();
}

// 별도 Camera 창의 shot/key 편집 흐름이다. Area 소유 shot과 Composition occurrence 연결을 구분하며 공유 shot 수정과 저장은 기존 Level API에 맡긴다.
// Render/End_WorkbenchFrame이 호출하는 독립 창이다. shot과 key 선택 ID는 세션에 남기고
// Level 소유 shot은 매 프레임 복사해 편집한다. Update가 거절되면 기존 shot을 다시 조회해 preview한다.
// 전용 shot 생성으로 Composition이 교체되는 분기는 ImGui ID/창 scope를 닫고 즉시 끝낸다.
void Client::CKoukuSaydonActionWorkbench::Render_CameraWindow()
{
	if (!m_bCameraWindowOpen)
		return;
	ImGui::SetNextWindowSize({660.f, 740.f}, ImGuiCond_FirstUseEver);
	const char *title = m_bSequenceWorkspace ? "Composition Camera###SequenceCompositionCamera"
											 : "Composition Camera###ActionCompositionCamera";
	if (!ImGui::Begin(title, &m_bCameraWindowOpen))
	{
		ImGui::End();
		return;
	}
	auto *level = CLevel_KoukuSaydonArena::Get_Active();
	if (!level)
	{
		ImGui::TextWrapped("Enter KoukuSaydon to author its camera actions.");
		ImGui::End();
		return;
	}
	if (ImGui::Button("Reload Cameras"))
		if (level->Reload_CameraShotAuthoring(m_strStatus))
			m_bPresentationResourceRefreshRequested = true;
	if (!level->Ensure_CameraShotAuthoring(m_strStatus))
	{
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		ImGui::TextWrapped("Repair the Camera source, then use Reload Cameras to retry.");
		ImGui::End();
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("Create Camera Action"))
		ImGui::OpenPopup("Create Camera Action##Camera");
	if (ImGui::BeginPopupModal("Create Camera Action##Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::InputText("Name", m_NewCameraActionName, std::size(m_NewCameraActionName));
		ImGui::TextWrapped(
			"The current view becomes the first position. Capture later positions at their arrival times.");
		ImGui::BeginDisabled(!m_NewCameraActionName[0]);
		if (ImGui::Button("Create"))
		{
			std::string id;
			if (level->Create_CameraShot(m_NewCameraActionName, id, m_strStatus))
			{
				m_strCameraWindowShotId = id;
				m_strCameraKeyId.clear();
				m_iCameraCaptureMs = 1000;
				m_strCameraWindowPatternId.clear();
				m_strCameraWindowOccurrenceId.clear();
				m_strSelectedPresentationSourceId =
					std::to_string(static_cast<int>(KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)) + "::" + id;
				m_NewCameraActionName[0] = '\0';
				m_bPresentationResourceRefreshRequested = true;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Save Camera"))
		if (level->Save_CameraShots(m_strStatus))
			m_bPresentationResourceRefreshRequested = true;
	const auto &shots = level->Get_CameraShots();
	const auto *selected = Find_AuthoringCamera(m_strCameraWindowShotId);
	if (ImGui::BeginCombo("Camera Action", selected ? selected->strDisplayName.c_str() : "Choose camera action"))
	{
		for (const auto &item : shots)
			if (ImGui::Selectable(
					(item.strDisplayName + "###" + item.strShotId).c_str(), item.strShotId == m_strCameraWindowShotId))
			{
				m_strCameraWindowShotId = item.strShotId;
				m_strCameraKeyId.clear();
				m_strCameraWindowPatternId.clear();
				m_strCameraWindowOccurrenceId.clear();
			}
		ImGui::EndCombo();
	}
	selected = Find_AuthoringCamera(m_strCameraWindowShotId);
	if (!selected)
	{
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		ImGui::End();
		return;
	}
	auto shot = *selected;
	auto cue = CLevel_KoukuSaydonArena::CameraShot_ToCue(shot);
	ImGui::PushID(shot.strShotId.c_str());
	ImGui::TextDisabled("%s | %zu positions | %u ms", shot.strShotId.c_str(), cue.Keyframes.size(), cue.iDurationMs);
	// 아래 편집은 같은 stable shot을 변경하므로 먼저 영향을 받는 소비자를 보여 준다.
	const auto consumers = Camera_Consumers(m_Draft, shot);
	const bool sharedShot = consumers.size() > 1u;
	const bool locked = sharedShot && m_strCameraSharedEditShotId != shot.strShotId;
	const auto *contextPattern = Find_Pattern(m_Draft, m_strCameraWindowPatternId);
	const auto *contextBox =
		contextPattern ? Find_PresentationBox(*contextPattern, m_strCameraWindowOccurrenceId) : nullptr;
	const auto *contextResource = contextBox ? Find_PresentationResource(m_Draft, contextBox->strResourceId) : nullptr;
	const bool boxContext = contextResource && contextResource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA &&
		contextResource->strAssetId == shot.strShotId;
	ImGui::Text("Played by %zu", consumers.size());
	for (const auto &consumer : consumers)
		ImGui::BulletText("%s", consumer.c_str());
	if (sharedShot)
	{
		bool allowShared = !locked;
		if (ImGui::Checkbox("Edit this shared shot for every consumer above", &allowShared))
			m_strCameraSharedEditShotId = allowShared ? shot.strShotId : std::string();
		if (boxContext && ImGui::Button("Make dedicated shot for this box") &&
			Make_DedicatedCameraShot(contextPattern->strPatternId, contextBox->strOccurrenceId, m_strStatus))
		{
			ImGui::TextWrapped("%s", m_strStatus.c_str());
			ImGui::PopID();
			ImGui::End();
			return;
		}
	}
	if (boxContext)
		ImGui::TextWrapped(
			"Sequence box %s / %s starts at %u ms for %u ms. Shot time T plays at Sequence time %u + T ms.",
			contextPattern->strDisplayName.c_str(), contextBox->strOccurrenceId.c_str(), contextBox->iStartMs,
			contextBox->iDurationMs, contextBox->iStartMs);
	else
		ImGui::TextDisabled("Open from a Sequence CAMERA box to map shot time to Sequence time.");
	if (boxContext && m_PreviewState.bPlaying && m_PreviewState.strPatternId == contextPattern->strPatternId)
		ImGui::Text("Sequence preview %u ms%s = shot %lld ms", m_PreviewState.iClockMs,
			m_PreviewState.bPaused ? " (paused)" : "",
			static_cast<long long>(m_PreviewState.iClockMs) - static_cast<long long>(contextBox->iStartMs));
	if (shot.followsPlayer)
	{
		ImGui::TextWrapped(
			"This action follows PLAYER. Capture a world path explicitly to replace that follow framing.");
		ImGui::BeginDisabled(locked);
		if (ImGui::Button("Convert to world camera path"))
		{
			VALTAN_CINEMATIC_CAMERA_POSE pose;
			if (CCameraTool::Capture_ViewPose(pose))
			{
				shot.followsPlayer = false;
				shot.hasCameraTrack = true;
				cue.Keyframes = {
					{shot.strShotId + ".p1", 0u, pose.vEye, pose.vLookAt, pose.fFovYDegrees, pose.vUp, true}};
				shot.CameraTrack = cue;
				shot.vEye = pose.vEye;
				shot.vLookAt = pose.vLookAt;
				shot.fFovYDegrees = pose.fFovYDegrees;
				(void)level->Update_CameraShot(shot, m_strStatus);
			}
			else
				m_strStatus = "Current camera pose is unavailable; the action was preserved.";
		}
		ImGui::EndDisabled();
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		ImGui::PopID();
		ImGui::End();
		return;
	}
	bool changed = false;
	ImGui::BeginDisabled(locked);
	char name[129]{};
	(void)Copy_Text(name, std::size(name), shot.strDisplayName);
	if (ImGui::InputText("Action name", name, std::size(name)))
	{
		shot.strDisplayName = name;
		changed = true;
	}
	int entry = static_cast<int>(shot.iBlendInMs), exit = static_cast<int>(shot.iBlendOutMs);
	if (ImGui::InputInt("Entry blend ms", &entry))
	{
		shot.iBlendInMs = std::clamp(entry, 0, 10000);
		changed = true;
	}
	if (ImGui::InputInt("Return ms", &exit))
	{
		shot.iBlendOutMs = std::clamp(exit, 0, 10000);
		changed = true;
	}
	int interpolation = static_cast<int>(cue.eInterpolation), easing = static_cast<int>(cue.eEasing);
	if (ImGui::Combo("Position path", &interpolation, "Linear\0Catmull-Rom\0"))
	{
		cue.eInterpolation = static_cast<VALTAN_CINEMATIC_CAMERA_INTERPOLATION>(interpolation);
		changed = true;
	}
	if (ImGui::Combo("Segment easing", &easing, "Linear\0Smoothstep\0Hold\0"))
	{
		cue.eEasing = static_cast<VALTAN_CINEMATIC_CAMERA_EASING>(easing);
		changed = true;
	}
	ImGui::InputInt("Capture arrival ms", &m_iCameraCaptureMs);
	m_iCameraCaptureMs = std::clamp(m_iCameraCaptureMs, 0, 120000);
	if (ImGui::Button("Capture Pos + Rot"))
	{
		VALTAN_CINEMATIC_CAMERA_POSE pose;
		if (!CCameraTool::Capture_ViewPose(pose))
			m_strStatus = "Current camera pose is unavailable; the action was preserved.";
		else
		{
			const auto time = static_cast<std::uint32_t>(m_iCameraCaptureMs);
			auto found = std::find_if(cue.Keyframes.begin(), cue.Keyframes.end(), [time](const auto &key) {
				return key.iTimeMs == time;
			});
			if (found == cue.Keyframes.end() && cue.Keyframes.size() >= 64u)
				m_strStatus = "Camera position limit is 64.";
			else
			{
				VALTAN_CINEMATIC_CAMERA_KEYFRAME key;
				key.iTimeMs = time;
				key.vEye = pose.vEye;
				key.vLookAt = pose.vLookAt;
				key.fFovYDegrees = pose.fFovYDegrees;
				key.vUp = pose.vUp;
				key.hasUp = true;
				if (found != cue.Keyframes.end())
				{
					key.strSceneId = found->strSceneId;
					*found = key;
				}
				else
				{
					for (std::uint32_t ordinal = 1u; ordinal <= 65u; ++ordinal)
					{
						key.strSceneId = shot.strShotId + ".p" + std::to_string(ordinal);
						if (std::none_of(cue.Keyframes.begin(), cue.Keyframes.end(), [&](const auto &row) {
								return row.strSceneId == key.strSceneId;
							}))
							break;
					}
					cue.Keyframes.push_back(key);
				}
				m_strCameraKeyId = key.strSceneId;
				changed = true;
				m_iCameraCaptureMs = (std::min)(120000, m_iCameraCaptureMs + 1000);
			}
		}
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Stop camera preview"))
	{
		m_bPresentationPreviewRequestPending = false;
		level->Stop_CompositionCamera(true);
	}
	if (ImGui::BeginListBox("Positions", {-1.f, 150.f}))
	{
		for (const auto &key : cue.Keyframes)
		{
			const auto label = std::to_string(key.iTimeMs) + " ms / " + key.strSceneId;
			if (ImGui::Selectable(label.c_str(), m_strCameraKeyId == key.strSceneId))
				m_strCameraKeyId = key.strSceneId;
		}
		ImGui::EndListBox();
	}
	auto key = std::find_if(cue.Keyframes.begin(), cue.Keyframes.end(), [&](const auto &item) {
		return item.strSceneId == m_strCameraKeyId;
	});
	if (key == cue.Keyframes.end() && !cue.Keyframes.empty())
	{
		key = cue.Keyframes.begin();
		m_strCameraKeyId = key->strSceneId;
	}
	if (boxContext && key != cue.Keyframes.end())
	{
		const auto sequenceMs = (std::min)(contextBox->iStartMs + key->iTimeMs, Pattern_DurationMs(*contextPattern));
		ImGui::Text("Selected position %u ms = Sequence %u ms", key->iTimeMs, contextBox->iStartMs + key->iTimeMs);
		// 실행 중 Sequence는 PAUSE로 먼저 seek하고, 없으면 해당 key에서 정지한 Sequence를 연다.
		if (ImGui::Button("Pause Sequence at this position"))
		{
			if (m_PreviewState.bPlaying && m_PreviewState.strPatternId == contextPattern->strPatternId)
			{
				m_strCursorPatternId = contextPattern->strPatternId;
				m_iCursorMs = sequenceMs;
				m_iPendingSeekMs = sequenceMs;
				m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::PAUSE;
				m_strStatus =
					"Sequence paused at " + std::to_string(sequenceMs) + " ms; camera edits re-evaluate at this time.";
			}
			else
				(void)Request_PatternPreview(contextPattern->strPatternId, sequenceMs, m_strStatus, true);
		}
	}
	ImGui::BeginDisabled(locked);
	if (key != cue.Keyframes.end())
	{
		int time = static_cast<int>(key->iTimeMs);
		if (ImGui::InputInt("Arrival time ms", &time))
		{
			key->iTimeMs = std::clamp(time, 0, 120000);
			changed = true;
		}
		const auto direction = XMLoadFloat3(&key->vLookAt) - XMLoadFloat3(&key->vEye);
		const auto forward = XMVector3Normalize(direction);
		float3_t look;
		XMStoreFloat3(&look, forward);
		const float pitch = std::asin(std::clamp(-look.y, -1.f, 1.f)), yaw = std::atan2(look.x, look.z);
		const auto basis = XMMatrixRotationRollPitchYaw(pitch, yaw, 0.f);
		const auto up = XMLoadFloat3(&key->vUp);
		const float roll = key->hasUp
			? std::atan2(-XMVectorGetX(XMVector3Dot(up, basis.r[0])), XMVectorGetX(XMVector3Dot(up, basis.r[1])))
			: 0.f;
		float3_t rotation{XMConvertToDegrees(pitch), XMConvertToDegrees(yaw), XMConvertToDegrees(roll)};
		float3_t position = key->vEye;
		const bool positionChanged = ImGui::DragFloat3("Pos", &position.x, .05f);
		const bool rotationChanged = ImGui::DragFloat3("Rot / pitch yaw roll", &rotation.x, .1f);
		if (positionChanged || rotationChanged)
		{
			const auto edited = XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));
			key->vEye = position;
			XMStoreFloat3(
				&key->vLookAt, XMLoadFloat3(&position) + edited.r[2] * XMVectorGetX(XMVector3Length(direction)));
			XMStoreFloat3(&key->vUp, edited.r[1]);
			key->hasUp = true;
			changed = true;
		}
		changed |= ImGui::DragFloat("FOV Y", &key->fFovYDegrees, .1f, 1.01f, 178.99f);
		if (key != cue.Keyframes.begin())
		{
			const auto &previous = *std::prev(key);
			const float distance =
				XMVectorGetX(XMVector3Length(XMLoadFloat3(&key->vEye) - XMLoadFloat3(&previous.vEye)));
			const auto span = static_cast<int64_t>(key->iTimeMs) - previous.iTimeMs;
			ImGui::Text("Segment: %.3f units / %.3f s = %.3f units/s (straight-line average)", distance, span * .001,
				span > 0 ? distance * 1000.0 / span : 0.0);
		}
		ImGui::BeginDisabled(cue.Keyframes.size() <= 1u);
		if (ImGui::Button("Delete position"))
		{
			cue.Keyframes.erase(key);
			m_strCameraKeyId.clear();
			changed = true;
			const auto firstTime = cue.Keyframes.front().iTimeMs;
			for (auto &row : cue.Keyframes)
				row.iTimeMs -= firstTime;
		}
		ImGui::EndDisabled();
	}
	if (cue.Keyframes.size() == 1u)
	{
		int duration = static_cast<int>(cue.iDurationMs);
		if (ImGui::InputInt("Static duration ms", &duration))
		{
			cue.iDurationMs = std::clamp(duration, 1, 120000);
			changed = true;
		}
	}
	ImGui::EndDisabled();
	if (changed)
	{
		std::stable_sort(cue.Keyframes.begin(), cue.Keyframes.end(), [](const auto &left, const auto &right) {
			return left.iTimeMs < right.iTimeMs;
		});
		if (cue.Keyframes.size() > 1u)
			cue.iDurationMs = cue.Keyframes.back().iTimeMs;
		shot.hasCameraTrack = true;
		shot.CameraTrack = cue;
		shot.iDefaultHoldMs = cue.iDurationMs > shot.iBlendInMs ? cue.iDurationMs - shot.iBlendInMs : 0u;
		shot.vEye = cue.Keyframes.front().vEye;
		shot.vLookAt = cue.Keyframes.front().vLookAt;
		shot.fFovYDegrees = cue.Keyframes.front().fFovYDegrees;
		if (shot.iBlendInMs > cue.iDurationMs)
			m_strStatus = "Entry blend exceeds the camera action duration; the previous action was preserved.";
		else
			(void)level->Update_CameraShot(shot, m_strStatus);
	}
	// 검증 뒤 다시 조회하여 거절된 입력이 preview나 resource에 사용되지 않게 한다.
	if (const auto *committed = Find_AuthoringCamera(m_strCameraWindowShotId))
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE resource;
		resource.eKind = KOUKU_SAYDON_PRESENTATION_KIND::CAMERA;
		resource.strResourceKind.clear();
		resource.strAssetId = committed->strShotId;
		resource.strDisplayName = committed->strDisplayName;
		resource.iDurationMs = Camera_DefaultDuration(resource);
		if (ImGui::Button("Play Camera"))
			Queue_PresentationPreview(resource);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(
				"Camera only: World boxes of the Sequence do not play. Use Pause Sequence at this position for the full scene.");
		ImGui::SameLine();
		ImGui::BeginDisabled(!m_bHasDraft);
		if (ImGui::Button("Append Camera at Cursor"))
		{
			std::string status;
			(void)Append_PresentationSource(resource, status);
		}
		ImGui::EndDisabled();
	}
	ImGui::TextWrapped(
		"F6 Free camera: place the view, then capture. Times set travel speed; rotation includes roll. Save Camera writes the Area source. Append and Composition Save keep the stable shot reference for Sequencer Play.");
	ImGui::TextWrapped(
		"Save Camera writes Data/Maps/Authoring/<Area>/<Area>.camerashots.json. Sequence Save writes the CAMERA box start, length and shot reference in the Composition.");
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::PopID();
	ImGui::End();
}

// Camera 창에서 공유 shot을 현재 박스 전용으로 분리할 때 호출한다. Area에는 shot 사본을 만들고,
// Composition 후보에는 새 resource와 해당 occurrence의 참조만 추가한다. 후보 commit이 실패하면
// Discard_UnsavedCameraShot으로 방금 만든 shot의 폐기를 요청하며 기존 Composition 연결은 유지한다.
bool_t Client::CKoukuSaydonActionWorkbench::Make_DedicatedCameraShot(
	const std::string_view patternIdView, const std::string_view occurrenceIdView, std::string &outStatus)
{
	// 입력 view가 m_Draft 내부를 가리킬 수 있으므로 Commit_Candidate 교체 전에 복사한다.
	const std::string patternId(patternIdView), occurrenceId(occurrenceIdView);
	auto *level = CLevel_KoukuSaydonArena::Get_Active();
	if (!level || !m_bHasDraft)
	{
		outStatus = m_strStatus = "Enter KoukuSaydon with a loaded Composition to split a Camera shot.";
		return false;
	}
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	if (!pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "The CAMERA box Pattern is unavailable; the Composition was preserved.";
		return false;
	}
	const auto box = std::find_if(
		pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(), [&](const auto &row) {
			return row.strOccurrenceId == occurrenceId;
		});
	const auto *resource = box == pattern->PresentationOccurrences.end()
		? nullptr
		: Find_PresentationResource(candidate, box->strResourceId);
	if (!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
	{
		outStatus = m_strStatus = "Select an existing CAMERA box before splitting its shot.";
		return false;
	}
	if (candidate.iNextPresentationResourceOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Presentation resource IDs are exhausted.";
		return false;
	}
	auto dedicated = *resource;
	dedicated.strDisplayName = resource->strDisplayName + " / " + occurrenceId;
	std::string shotId;
	if (!level->Duplicate_CameraShot(resource->strAssetId, dedicated.strDisplayName, shotId, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	if (dedicated.strDisplayName.size() > 255u)
		dedicated.strDisplayName = shotId;
	dedicated.strResourceId =
		"koukusaydon.g1.presentation." + std::to_string(candidate.iNextPresentationResourceOrdinal++);
	dedicated.strAssetId = shotId;
	// 이 occurrence만 새 리소스로 옮기고 다른 박스는 기존 공유 리소스를 유지한다.
	box->strResourceId = dedicated.strResourceId;
	const auto resourceId = dedicated.strResourceId;
	candidate.PresentationResources.push_back(std::move(dedicated));
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(
			std::move(candidate), "CAMERA box now uses a dedicated shot. Save Camera, then Sequence Save.", outStatus))
	{
		std::string ignored;
		(void)level->Discard_UnsavedCameraShot(shotId, ignored);
		return false;
	}
	if (m_PresentationBoxEdit.strOccurrenceId == occurrenceId)
		m_PresentationBoxEdit.strResourceId = resourceId;
	m_strCameraWindowShotId = shotId;
	m_strCameraKeyId.clear();
	m_strCameraSharedEditShotId.clear();
	m_bPresentationResourceRefreshRequested = true;
	return true;
}
