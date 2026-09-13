#include "imgui.h"
#include "MapTool_Internal.h"
#include "Camera_Free.h"
#include "DataJson.h"
#include "GameInstance.h"
#include <algorithm>
#include <array>
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




void Client::CMapTool::Render_CameraPanel()
{
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

bool_t Client::CMapTool::Load_CameraShots(
	const EDITOR_AREA_DESCRIPTOR& descriptor)
{
	End_CameraShotPreview();
	m_CameraShots.clear();
	m_strCameraShotBaselineText.clear();
	m_iSelectedCameraShot = 0u;
	if (descriptor.cameraShotDocument.empty())
	{
		m_CameraShotStatus = "No camera shot document for this Area";
		return false;
	}
	std::error_code error;
	if (!std::filesystem::is_regular_file(descriptor.cameraShotDocument, error) ||
		error)
	{
		m_CameraShotStatus =
			"No shots authored yet. Add Shot From Camera creates the first one.";
		return true;
	}

	std::string text;
	std::string parseError;
	DATA_JSON_VALUE root;
	if (!ReadTextFile(descriptor.cameraShotDocument, text) ||
		!CDataJson::Parse(text, root, parseError) || !root.Is_Object())
	{
		m_CameraShotStatus = "Camera shot parse failed: " + parseError;
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
		area->Get_String() != descriptor.areaId ||
		nullptr == shots || !shots->Is_Array())
	{
		m_CameraShotStatus = "Camera shot header is invalid";
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
			m_CameraShotStatus = "Camera shot row is invalid";
			return false;
		}
		if (nullptr == sequenceId || !sequenceId->Is_String())
		{
			m_CameraShotStatus = "Camera shot sequence binding is invalid";
			return false;
		}
		shot.shotId = shotId->Get_String();
		shot.sequenceInstanceId = sequenceId->Get_String();
		shot.displayName = shot.shotId;
		if (const auto* name = value.Find("displayName"))
		{
			if (!name->Is_String() || name->Get_String().empty() || name->Get_String().size() > 128u)
			{ m_CameraShotStatus = "Camera displayName is invalid"; return false; }
			shot.displayName = name->Get_String();
		}
		if (const auto* hold = value.Find("defaultHoldMs"))
		{
			if (!hold->Is_Number() || !std::isfinite(hold->Get_Number()) || hold->Get_Number() < 0.0 ||
				hold->Get_Number() > 600000.0 || std::floor(hold->Get_Number()) != hold->Get_Number())
			{ m_CameraShotStatus = "Camera defaultHoldMs is invalid"; return false; }
			shot.defaultHoldMs = static_cast<int32_t>(hold->Get_Number());
		}
		if (const auto* activation = value.Find("activation"))
		{
			if (!activation->Is_String() || (activation->Get_String() != "AUTO" && activation->Get_String() != "PATTERN_ONLY"))
			{ m_CameraShotStatus = "Camera activation is invalid"; return false; }
			shot.patternOnly = activation->Get_String() == "PATTERN_ONLY";
		}
		if (const auto* easing = value.Find("transitionEasing"))
		{
			if (!easing->Is_String() || (easing->Get_String() != "LINEAR" && easing->Get_String() != "SMOOTHSTEP"))
			{ m_CameraShotStatus = "Camera transitionEasing is invalid"; return false; }
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
			m_CameraShotStatus = "Camera shot numbers are invalid";
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
				m_CameraShotStatus = "Camera shot follow offsets are invalid";
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
				m_CameraShotStatus = "Camera track is invalid: " + shot.shotId;
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
					m_CameraShotStatus =
						"Camera keyframe is invalid: " + shot.shotId;
					return false;
				}
				keyframe.sceneId = sceneId->Get_String();
				keyframe.timeMs = static_cast<int32_t>(timeMs->Get_Number());
				keyframe.fovYDegrees = static_cast<f32_t>(fov->Get_Number());
				if (const auto* up = entry.Find("up"))
				{
					if (!readFloat3(up, keyframe.up))
					{ m_CameraShotStatus = "Camera keyframe up is invalid: " + shot.shotId; return false; }
					keyframe.hasUp = true;
				}
				shot.keyframes.push_back(std::move(keyframe));
			}
		}
		staged.push_back(std::move(shot));
	}

	m_CameraShots = std::move(staged);
	m_strCameraShotBaselineText = text;
	m_CameraShotStatus =
		"Loaded " + std::to_string(m_CameraShots.size()) + " shot(s)";
	return true;
}

bool_t Client::CMapTool::Save_CameraShots()
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor || descriptor->cameraShotDocument.empty())
	{
		m_CameraShotStatus = "No camera shot document for this Area";
		return false;
	}
	std::unordered_set<std::string> ids;
	for (const EDITOR_CAMERA_SHOT& shot : m_CameraShots)
	{
		if (shot.shotId.empty() || !ids.emplace(shot.shotId).second)
		{
			m_CameraShotStatus = "Shot IDs must be unique and non-empty";
			return false;
		}
		if (shot.halfExtents.x <= 0.f || shot.halfExtents.y <= 0.f ||
			shot.halfExtents.z <= 0.f)
		{
			m_CameraShotStatus = "Half extents must be positive: " + shot.shotId;
			return false;
		}
	}

	const auto number = [](const f32_t value)
	{
		char buffer[32]{};
		std::snprintf(buffer, sizeof(buffer), "%.6g",
			static_cast<double>(value));
		return std::string(buffer);
	};
	const auto vector3 = [&number](const float3_t& value)
	{
		return "[" + number(value.x) + ", " + number(value.y) + ", " +
			number(value.z) + "]";
	};

	/* Revision follows the document on disk so two saves never collide. */
	uint32_t revision = 1u;
	std::string existingText;
	if (ReadTextFile(descriptor->cameraShotDocument, existingText))
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

	std::string text;
	text += "{\n  \"schema\": \"lostark.camera-shots\",\n";
	text += "  \"formatVersion\": 1,\n";
	text += "  \"areaId\": \"" + descriptor->areaId + "\",\n";
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
	text += m_CameraShots.empty() ? "]\n}\n" : "\n  ]\n}\n";

	if (!Save_CameraShotDocumentAtomic(descriptor->cameraShotDocument,
		m_strCameraShotBaselineText, text, m_CameraShotStatus)) return false;
	m_strCameraShotBaselineText = text;
	m_CameraShotStatus = "Saved " + std::to_string(m_CameraShots.size()) +
		" shot(s) as revision " + std::to_string(revision) +
		". Publish the Area to ship it.";
	return true;
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
		(void)Load_CameraShots(*descriptor);
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
				keyframe.timeMs = shot.keyframes.empty() ? 0 :
					shot.keyframes.back().timeMs + 1000;
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
				ImGui::DragInt("Key Time (ms)", &keyframe.timeMs,
					10.f, 0, 120000);
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
