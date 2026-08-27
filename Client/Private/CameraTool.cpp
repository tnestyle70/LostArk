#include "imgui.h"

#include "CameraTool.h"

#include "Camera_Free.h"
#include "GameInstance.h"
#include "ProjectDataRoot.h"
#include "ValtanCinematicCameraController.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <fstream>
#include <sstream>

namespace
{
	using namespace Client;

	CAMERA_TOOL_ACTOR_PREVIEW_CONTEXT g_ActorPreviewContext;

	bool_t Is_FiniteActorPosition(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	}

	bool_t Read_TextFile(
		const std::filesystem::path& path,
		std::string& outText,
		std::string& outStatus)
	{
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input.is_open())
		{
			outStatus = "Camera source is unreadable: " + path.string();
			return false;
		}
		std::ostringstream buffer;
		buffer << input.rdbuf();
		if (input.bad())
		{
			outStatus = "Camera source read failed: " + path.string();
			return false;
		}
		outText = buffer.str();
		return true;
	}

	std::filesystem::path Make_TemporaryPath(
		const std::filesystem::path& destination)
	{
		static std::atomic_uint64_t sequence{ 0u };
		std::filesystem::path temporary = destination;
		temporary += L".tmp." + std::to_wstring(GetCurrentProcessId()) + L"." +
			std::to_wstring(++sequence);
		return temporary;
	}

	void Remove_Temporary(const std::filesystem::path& path)
	{
		std::error_code error;
		std::filesystem::remove(path, error);
	}

	const VALTAN_CINEMATIC_CAMERA_CUE* Find_DocumentCue(
		const CValtanCinematicCameraDocument& document,
		const std::string& cueId)
	{
		const auto found = std::find_if(
			document.Get_Cues().begin(), document.Get_Cues().end(),
			[&cueId](const VALTAN_CINEMATIC_CAMERA_CUE& cue)
			{ return cue.strCueId == cueId; });
		if (document.Get_Cues().end() != found)
			return &*found;
		const VALTAN_CINEMATIC_CAMERA_CUE* death = document.Find_DeathCue();
		return nullptr != death && death->strCueId == cueId ? death : nullptr;
	}

	const char_t* Easing_Label(
		const VALTAN_CINEMATIC_CAMERA_EASING easing)
	{
		switch (easing)
		{
		case VALTAN_CINEMATIC_CAMERA_EASING::LINEAR: return "LINEAR";
		case VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP: return "SMOOTHSTEP";
		case VALTAN_CINEMATIC_CAMERA_EASING::HOLD: return "HOLD";
		default: return "INVALID";
		}
	}

	const char_t* Tracking_Label(
		const VALTAN_CINEMATIC_TRACKING_MODE tracking)
	{
		switch (tracking)
		{
		case VALTAN_CINEMATIC_TRACKING_MODE::WORLD: return "WORLD";
		case VALTAN_CINEMATIC_TRACKING_MODE::BOSS_XZ: return "BOSS_XZ";
		case VALTAN_CINEMATIC_TRACKING_MODE::BOSS_FACING:
			return "BOSS_FACING";
		case VALTAN_CINEMATIC_TRACKING_MODE::PLAYER_BOSS_FRAME:
			return "PLAYER_BOSS_FRAME";
		default: return "INVALID";
		}
	}
}

void Client::CCameraTool::Publish_ActorPreviewContext(
	const CAMERA_TOOL_ACTOR_PREVIEW_CONTEXT& context)
{
	if (!context.isValid)
	{
		if (g_ActorPreviewContext.iLevelIndex == context.iLevelIndex)
			g_ActorPreviewContext = {};
		return;
	}
	if (!Is_FiniteActorPosition(context.vBossPosition) ||
		!std::isfinite(context.fBossYawDegrees) ||
		(context.hasLocalPlayerPosition &&
			!Is_FiniteActorPosition(context.vLocalPlayerPosition)))
	{
		if (g_ActorPreviewContext.iLevelIndex == context.iLevelIndex)
			g_ActorPreviewContext = {};
		return;
	}
	g_ActorPreviewContext = context;
}

void Client::CCameraTool::Clear_ActorPreviewContext(
	const uint32_t levelIndex)
{
	if (g_ActorPreviewContext.iLevelIndex == levelIndex)
		g_ActorPreviewContext = {};
}

Client::CCameraTool::~CCameraTool()
{
	Deactivate();
}

void Client::CCameraTool::Open()
{
	m_bOpen = true;
	m_bFocusPending = true;
	if (!m_bLoaded)
		(void)Reload();
}

bool_t Client::CCameraTool::Open_Cue(
	const CAMERA_TOOL_OPEN_REQUEST& request)
{
	if (request.strCueId.empty())
	{
		m_strStatus = "Camera deep link rejected an empty cue ID.";
		return false;
	}
	m_strPendingOpenCueId = request.strCueId;
	Open();
	if (!m_bLoaded)
		return false;
	if (nullptr == Find_DraftCue(request.strCueId))
	{
		m_strStatus = "Camera cue was not found: " + request.strCueId;
		return false;
	}
	Select_Cue(request.strCueId);
	m_strPendingOpenCueId.clear();
	m_strStatus = "Opened camera cue from Boss Tool: " + request.strCueId;
	return true;
}

void Client::CCameraTool::Update(
	const f32_t timeDelta,
	const bool_t toolVisible)
{
	if (!toolVisible || !m_bOpen)
	{
		Deactivate();
		return;
	}
	Handle_PreviewPreemption();
	if (!m_bPlaying || !m_bPreviewOwned || !std::isfinite(timeDelta) ||
		timeDelta < 0.f)
	{
		return;
	}
	const VALTAN_CINEMATIC_CAMERA_CUE* cue = Get_SelectedCue();
	if (nullptr == cue || 0u == cue->iDurationMs)
	{
		m_bPlaying = false;
		return;
	}
	m_fPreviewSeconds += (std::min)(timeDelta, 0.1f);
	const f32_t durationSeconds = static_cast<f32_t>(cue->iDurationMs) * 0.001f;
	if (m_fPreviewSeconds >= durationSeconds)
	{
		m_fPreviewSeconds = durationSeconds;
		m_bPlaying = false;
	}
	if (!Apply_PreviewPose())
		m_bPlaying = false;
}

void Client::CCameraTool::Render()
{
	if (!m_bOpen)
		return;
	if (m_bFocusPending)
	{
		ImGui::SetNextWindowFocus();
		m_bFocusPending = false;
	}
	bool_t open = m_bOpen;
	if (!ImGui::Begin("Cinematic Camera Tool", &open,
		ImGuiWindowFlags_NoCollapse))
	{
		ImGui::End();
		m_bOpen = open;
		if (!m_bOpen)
			Deactivate();
		return;
	}

	if (ImGui::Button("Reload"))
	{
		if (m_bDirty)
			ImGui::OpenPopup("Discard camera draft?");
		else
			(void)Reload();
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_bLoaded || !m_bDirty);
	if (ImGui::Button("Save"))
		(void)Save();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_bLoaded);
	if (ImGui::Button("Validate"))
	{
		CValtanCinematicCameraDocument staged;
		std::string text;
		std::string status;
		m_strStatus = Validate_Draft(staged, text, status) ?
			"Camera draft validation passed." : "Validation failed: " + status;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("%s", m_bDirty ? "UNSAVED DRAFT" : "SOURCE MATCHED");

	if (ImGui::BeginPopupModal(
		"Discard camera draft?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextWrapped(
			"Reload discards the in-memory camera draft. The source file is not changed.");
		if (ImGui::Button("Discard and Reload"))
		{
			(void)Reload();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	ImGui::Separator();
	if (!m_bLoaded)
	{
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		ImGui::End();
		m_bOpen = open;
		if (!m_bOpen)
			Deactivate();
		return;
	}

	const ImVec2 available = ImGui::GetContentRegionAvail();
	ImGui::BeginChild("##cameraCueList", ImVec2(260.f, available.y - 42.f), true);
	Render_CueList();
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("##cameraCueEditor", ImVec2(0.f, available.y - 42.f), true);
	Render_CueEditor();
	ImGui::EndChild();
	ImGui::Separator();
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::End();

	m_bOpen = open;
	if (!m_bOpen)
		Deactivate();
}

void Client::CCameraTool::Deactivate()
{
	if (m_bPreviewOwned)
		Stop_Preview(false);
	m_bPlaying = false;
}

void Client::CCameraTool::On_LevelChanged()
{
	Deactivate();
	m_pPreviewCamera.reset();
	m_strStatus = "Level changed. Preview was restored; reload or select a cue.";
}

bool_t Client::CCameraTool::Reload()
{
	const std::filesystem::path encounterPath = CProjectDataRoot::Resolve(
		L"Encounters/Valtan/ValtanEncounter.json");
	const std::filesystem::path cameraPath = CProjectDataRoot::Resolve(
		L"Encounters/Valtan/ValtanCinematicCamera.json");
	CEncounterPatternReference stagedEncounter;
	std::string status;
	if (!stagedEncounter.Load(encounterPath, status))
	{
		m_strStatus = "Encounter reload failed: " + status;
		return false;
	}
	std::string sourceText;
	if (!Read_TextFile(cameraPath, sourceText, status))
	{
		m_strStatus = status;
		return false;
	}
	CValtanCinematicCameraDocument stagedDocument;
	if (!CValtanCinematicCameraDocument::Parse_Text(
		sourceText, stagedEncounter, stagedDocument, status))
	{
		m_strStatus = "Camera reload failed: " + status;
		return false;
	}
	const std::string preferred = !m_strPendingOpenCueId.empty() ?
		m_strPendingOpenCueId : m_strSelectedCueId;
	m_CameraPath = cameraPath;
	Commit_LoadedDocument(
		std::move(stagedEncounter), std::move(stagedDocument),
		std::move(sourceText), preferred);
	m_strPendingOpenCueId.clear();
	m_strStatus = "Reloaded canonical Valtan cinematic camera document.";
	return true;
}

bool_t Client::CCameraTool::Validate_Draft(
	CValtanCinematicCameraDocument& outDocument,
	std::string& outText,
	std::string& outStatus) const
{
	if (!m_bLoaded || !m_Encounter.Is_Ready() ||
		!m_LoadedDocument.Is_Ready())
	{
		outStatus = "Camera Tool has no validated source baseline";
		return false;
	}
	return m_LoadedDocument.Stage_CameraDraft(
		m_DraftCues, m_hasDraftDeathCue, m_DraftDeathCue,
		m_Encounter, outDocument, outText, outStatus);
}

bool_t Client::CCameraTool::Save()
{
	CValtanCinematicCameraDocument stagedDocument;
	std::string serialized;
	std::string status;
	if (!Validate_Draft(stagedDocument, serialized, status))
	{
		m_strStatus = "Save rejected invalid camera draft: " + status;
		return false;
	}
	std::string currentText;
	if (!Read_TextFile(m_CameraPath, currentText, status))
	{
		m_strStatus = "Save CAS read failed: " + status;
		return false;
	}
	if (currentText != m_strBaselineText)
	{
		m_strStatus =
			"Save conflict: source bytes changed after Reload. Draft and preview were preserved.";
		return false;
	}

	const std::filesystem::path temporary = Make_TemporaryPath(m_CameraPath);
	{
		std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
		if (!output.is_open())
		{
			m_strStatus = "Save failed to create sibling temporary file.";
			return false;
		}
		output.write(serialized.data(), static_cast<std::streamsize>(serialized.size()));
		output.flush();
		if (!output.good())
		{
			output.close();
			Remove_Temporary(temporary);
			m_strStatus = "Save failed while flushing the temporary camera document.";
			return false;
		}
	}
	const HANDLE temporaryHandle = CreateFileW(
		temporary.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
		nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == temporaryHandle ||
		FALSE == FlushFileBuffers(temporaryHandle))
	{
		if (INVALID_HANDLE_VALUE != temporaryHandle)
			CloseHandle(temporaryHandle);
		Remove_Temporary(temporary);
		m_strStatus = "Save failed to durably flush the temporary camera document.";
		return false;
	}
	CloseHandle(temporaryHandle);

	std::string temporaryText;
	CValtanCinematicCameraDocument temporaryDocument;
	if (!Read_TextFile(temporary, temporaryText, status) ||
		temporaryText != serialized ||
		!CValtanCinematicCameraDocument::Parse_Text(
			temporaryText, m_Encounter, temporaryDocument, status))
	{
		Remove_Temporary(temporary);
		m_strStatus = "Save rejected temporary camera document: " + status;
		return false;
	}

	currentText.clear();
	if (!Read_TextFile(m_CameraPath, currentText, status) ||
		currentText != m_strBaselineText)
	{
		Remove_Temporary(temporary);
		m_strStatus =
			"Save conflict before atomic replace. Disk, draft, and preview were preserved.";
		return false;
	}
	if (FALSE == MoveFileExW(
		temporary.c_str(), m_CameraPath.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		Remove_Temporary(temporary);
		m_strStatus = "Camera atomic replace failed. Destination was preserved.";
		return false;
	}

	m_LoadedDocument = std::move(temporaryDocument);
	m_strBaselineText = std::move(serialized);
	m_DraftCues = m_LoadedDocument.Get_Cues();
	m_hasDraftDeathCue = m_LoadedDocument.Has_DeathCue();
	m_DraftDeathCue = m_hasDraftDeathCue ?
		*m_LoadedDocument.Find_DeathCue() : VALTAN_CINEMATIC_CAMERA_CUE{};
	m_bDirty = false;
	m_bPreviewDraftStale = true;
	m_strStatus =
		"Saved canonical cinematic camera document atomically. Reload the encounter to consume it.";
	return true;
}

void Client::CCameraTool::Commit_LoadedDocument(
	CEncounterPatternReference encounter,
	CValtanCinematicCameraDocument document,
	std::string sourceText,
	const std::string& preferredCueId)
{
	Stop_Preview(false);
	m_Encounter = std::move(encounter);
	m_LoadedDocument = std::move(document);
	m_DraftCues = m_LoadedDocument.Get_Cues();
	m_hasDraftDeathCue = m_LoadedDocument.Has_DeathCue();
	m_DraftDeathCue = m_hasDraftDeathCue ?
		*m_LoadedDocument.Find_DeathCue() : VALTAN_CINEMATIC_CAMERA_CUE{};
	m_strBaselineText = std::move(sourceText);
	m_bLoaded = true;
	m_bDirty = false;
	m_bPreviewDraftStale = true;
	m_iSelectedKeyframe = -1;
	m_fPreviewSeconds = 0.f;
	if (!preferredCueId.empty() && nullptr != Find_DraftCue(preferredCueId))
		m_strSelectedCueId = preferredCueId;
	else if (!m_DraftCues.empty())
		m_strSelectedCueId = m_DraftCues.front().strCueId;
	else if (m_hasDraftDeathCue)
		m_strSelectedCueId = m_DraftDeathCue.strCueId;
	else
		m_strSelectedCueId.clear();
	if (const VALTAN_CINEMATIC_CAMERA_CUE* cue = Get_SelectedCue())
		m_iSelectedKeyframe = cue->Keyframes.empty() ? -1 : 0;
}

Client::VALTAN_CINEMATIC_CAMERA_CUE* Client::CCameraTool::Find_DraftCue(
	const std::string& cueId)
{
	const auto found = std::find_if(
		m_DraftCues.begin(), m_DraftCues.end(),
		[&cueId](const VALTAN_CINEMATIC_CAMERA_CUE& cue)
		{ return cue.strCueId == cueId; });
	if (m_DraftCues.end() != found)
		return &*found;
	return m_hasDraftDeathCue && m_DraftDeathCue.strCueId == cueId ?
		&m_DraftDeathCue : nullptr;
}

const Client::VALTAN_CINEMATIC_CAMERA_CUE*
Client::CCameraTool::Find_DraftCue(const std::string& cueId) const
{
	const auto found = std::find_if(
		m_DraftCues.begin(), m_DraftCues.end(),
		[&cueId](const VALTAN_CINEMATIC_CAMERA_CUE& cue)
		{ return cue.strCueId == cueId; });
	if (m_DraftCues.end() != found)
		return &*found;
	return m_hasDraftDeathCue && m_DraftDeathCue.strCueId == cueId ?
		&m_DraftDeathCue : nullptr;
}

Client::VALTAN_CINEMATIC_CAMERA_CUE* Client::CCameraTool::Get_SelectedCue()
{
	return Find_DraftCue(m_strSelectedCueId);
}

const Client::VALTAN_CINEMATIC_CAMERA_CUE*
Client::CCameraTool::Get_SelectedCue() const
{
	return Find_DraftCue(m_strSelectedCueId);
}

bool_t Client::CCameraTool::Is_DeathCueSelected() const
{
	return m_hasDraftDeathCue &&
		m_DraftDeathCue.strCueId == m_strSelectedCueId;
}

void Client::CCameraTool::Select_Cue(const std::string& cueId)
{
	if (m_strSelectedCueId == cueId || nullptr == Find_DraftCue(cueId))
		return;
	Stop_Preview(true);
	m_strSelectedCueId = cueId;
	m_iSelectedKeyframe = Get_SelectedCue()->Keyframes.empty() ? -1 : 0;
}

std::shared_ptr<Client::CCamera_Free>
Client::CCameraTool::Find_CurrentCamera() const
{
	const uint32_t level = CGameInstance::Get().Get_CurrentLevelID();
	return std::dynamic_pointer_cast<CCamera_Free>(
		CGameInstance::Get().Get_GameObject(level, TEXT("Layer_Camera"), 0));
}

bool_t Client::CCameraTool::Capture_CurrentPose(
	VALTAN_CINEMATIC_CAMERA_KEYFRAME& outKeyframe) const
{
	CGameInstance& gameInstance = CGameInstance::Get();
	const float4x4_t* inverseView =
		gameInstance.Get_InverseTransform(D3DTS::VIEW);
	const float4x4_t* projection = gameInstance.Get_Transform(D3DTS::PROJ);
	if (nullptr == inverseView || nullptr == projection ||
		!std::isfinite(projection->_22) || std::abs(projection->_22) <= 0.000001f)
	{
		return false;
	}
	const matrix_t cameraWorld = XMLoadFloat4x4(inverseView);
	const vector_t eye = cameraWorld.r[3];
	const vector_t look = XMVector3Normalize(cameraWorld.r[2]);
	if (XMVectorGetX(XMVector3LengthSq(look)) <= 0.000001f)
		return false;
	XMStoreFloat3(&outKeyframe.vEye, eye);
	XMStoreFloat3(&outKeyframe.vLookAt, eye + look * 10.f);
	outKeyframe.fFovYDegrees = XMConvertToDegrees(
		2.f * std::atan(1.f / projection->_22));
	return std::isfinite(outKeyframe.fFovYDegrees);
}

bool_t Client::CCameraTool::Acquire_PreviewCamera()
{
	CValtanCinematicCameraDocument staged;
	std::string text;
	std::string status;
	if (m_bPreviewDraftStale)
	{
		if (!Validate_Draft(staged, text, status))
		{
			m_strStatus = "Preview rejected invalid draft: " + status;
			return false;
		}
		m_PreviewDocument = std::move(staged);
		m_bPreviewDraftStale = false;
	}

	std::shared_ptr<CCamera_Free> camera = Find_CurrentCamera();
	if (nullptr == camera)
	{
		m_strStatus = "Current level has no CCamera_Free in Layer_Camera.";
		return false;
	}
	std::shared_ptr<CCamera_Free> previous = m_pPreviewCamera.lock();
	if (nullptr != previous && previous != camera)
		Stop_Preview(false);
	if (!camera->Begin_PresentationOverride(
		PREVIEW_OWNER_ID,
		CCamera::PRESENTATION_PRIORITY::AUTHORING_PREVIEW))
	{
		m_strStatus =
			"Preview refused: a Server cinematic or another presentation owner is active.";
		return false;
	}
	m_pPreviewCamera = camera;
	m_bPreviewOwned = true;
	return true;
}

bool_t Client::CCameraTool::Apply_PreviewPose()
{
	if (!Acquire_PreviewCamera())
		return false;
	const VALTAN_CINEMATIC_CAMERA_CUE* cue =
		Find_DocumentCue(m_PreviewDocument, m_strSelectedCueId);
	if (nullptr == cue)
	{
		m_strStatus = "Preview cue disappeared during strict staging.";
		Stop_Preview(false);
		return false;
	}
	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	if (!CValtanCinematicCameraController::Sample_Cue(
		*cue, m_fPreviewSeconds, pose))
	{
		m_strStatus = "Product camera sampler rejected the selected cue.";
		Stop_Preview(false);
		return false;
	}
	if (VALTAN_CINEMATIC_TRACKING_MODE::WORLD != cue->eTrackingMode)
	{
		const uint32_t currentLevel =
			CGameInstance::Get().Get_CurrentLevelID();
		if (!g_ActorPreviewContext.isValid ||
			g_ActorPreviewContext.iLevelIndex != currentLevel)
		{
			m_strStatus = std::string("Preview unavailable: ") +
				Tracking_Label(cue->eTrackingMode) +
				" requires the current replicated Valtan actor frame.";
			Stop_Preview(false);
			return false;
		}

		VALTAN_CINEMATIC_CAMERA_INPUT trackingInput{};
		trackingInput.vBossPosition =
			g_ActorPreviewContext.vBossPosition;
		trackingInput.fBossYawDegrees =
			g_ActorPreviewContext.fBossYawDegrees;
		trackingInput.hasLocalPlayerPosition =
			g_ActorPreviewContext.hasLocalPlayerPosition;
		trackingInput.vLocalPlayerPosition =
			g_ActorPreviewContext.vLocalPlayerPosition;
		if (!CValtanCinematicCameraController::Apply_CueTracking(
			*cue, trackingInput, pose))
		{
			m_strStatus = std::string("Preview unavailable: current actor frame does not satisfy ") +
				Tracking_Label(cue->eTrackingMode) + ".";
			Stop_Preview(false);
			return false;
		}
	}
	std::shared_ptr<CCamera_Free> camera = m_pPreviewCamera.lock();
	if (nullptr == camera ||
		!camera->Apply_PresentationPose(
			PREVIEW_OWNER_ID, pose.vEye, pose.vLookAt, pose.fFovYDegrees))
	{
		if (nullptr != camera &&
			camera->Is_PresentationOverrideOwnedBy(PREVIEW_OWNER_ID))
		{
			Stop_Preview(false);
			m_strStatus =
				"Preview pose was rejected; the prior camera pose was restored.";
		}
		else
		{
			Handle_PreviewPreemption();
		}
		return false;
	}
	m_strStatus = VALTAN_CINEMATIC_TRACKING_MODE::WORLD == cue->eTrackingMode ?
		"Previewing the exact product camera sampler." :
		std::string("Previewing exact product ") +
			Tracking_Label(cue->eTrackingMode) +
			" tracking from the current replicated actor frame.";
	return true;
}

void Client::CCameraTool::Stop_Preview(const bool_t resetCursor)
{
	std::shared_ptr<CCamera_Free> camera = m_pPreviewCamera.lock();
	if (nullptr != camera &&
		camera->Is_PresentationOverrideOwnedBy(PREVIEW_OWNER_ID))
	{
		(void)camera->End_PresentationOverride(PREVIEW_OWNER_ID);
	}
	m_pPreviewCamera.reset();
	m_bPreviewOwned = false;
	m_bPlaying = false;
	if (resetCursor)
		m_fPreviewSeconds = 0.f;
}

void Client::CCameraTool::Handle_PreviewPreemption()
{
	if (!m_bPreviewOwned)
		return;
	std::shared_ptr<CCamera_Free> camera = m_pPreviewCamera.lock();
	if (nullptr != camera &&
		camera->Is_PresentationOverrideOwnedBy(PREVIEW_OWNER_ID))
	{
		return;
	}
	m_pPreviewCamera.reset();
	m_bPreviewOwned = false;
	m_bPlaying = false;
	m_strStatus =
		"Camera preview yielded to a higher-priority Server cinematic owner.";
}

void Client::CCameraTool::Render_CueList()
{
	ImGui::SeparatorText("Cue List");
	for (const VALTAN_CINEMATIC_CAMERA_CUE& cue : m_DraftCues)
	{
		if (ImGui::Selectable(
			cue.strCueId.c_str(), cue.strCueId == m_strSelectedCueId))
		{
			Select_Cue(cue.strCueId);
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s / %s", cue.strPatternId.c_str(), cue.strStageId.c_str());
	}
	if (m_hasDraftDeathCue)
	{
		ImGui::SeparatorText("Death");
		if (ImGui::Selectable(
			m_DraftDeathCue.strCueId.c_str(), Is_DeathCueSelected()))
		{
			Select_Cue(m_DraftDeathCue.strCueId);
		}
	}
}

void Client::CCameraTool::Render_CueEditor()
{
	VALTAN_CINEMATIC_CAMERA_CUE* cue = Get_SelectedCue();
	if (nullptr == cue)
	{
		ImGui::TextDisabled("Select a camera cue.");
		return;
	}
	ImGui::SeparatorText("Cue Draft");
	ImGui::Text("%s", cue->strCueId.c_str());
	if (!Is_DeathCueSelected())
	{
		ImGui::TextDisabled("Pattern %s  |  Stage %s",
			cue->strPatternId.c_str(), cue->strStageId.c_str());
	}

	uint32_t duration = cue->iDurationMs;
	if (ImGui::InputScalar("Duration (ms)", ImGuiDataType_U32, &duration))
	{
		cue->iDurationMs = duration;
		if (!cue->Keyframes.empty())
			cue->Keyframes.back().iTimeMs = duration;
		m_fPreviewSeconds = (std::min)(
			m_fPreviewSeconds, static_cast<f32_t>(duration) * 0.001f);
		Mark_Dirty("Edited camera cue duration.");
	}

	int easing = static_cast<int>(cue->eEasing);
	const char_t* easings[] = { "LINEAR", "SMOOTHSTEP", "HOLD" };
	if (ImGui::Combo("Easing", &easing, easings, 3))
	{
		cue->eEasing = static_cast<VALTAN_CINEMATIC_CAMERA_EASING>(easing);
		Mark_Dirty("Edited camera cue easing.");
	}

	if (!Is_DeathCueSelected())
	{
		int tracking = static_cast<int>(cue->eTrackingMode);
		const char_t* modes[] = {
			"WORLD", "BOSS_XZ", "BOSS_FACING", "PLAYER_BOSS_FRAME"
		};
		if (ImGui::Combo("Tracking", &tracking, modes, 4))
		{
			cue->eTrackingMode =
				static_cast<VALTAN_CINEMATIC_TRACKING_MODE>(tracking);
			Mark_Dirty("Edited camera cue tracking mode.");
		}
		if (VALTAN_CINEMATIC_TRACKING_MODE::WORLD != cue->eTrackingMode &&
			ImGui::InputFloat3("Tracking Origin", &cue->vTrackingOrigin.x, "%.3f"))
		{
			Mark_Dirty("Edited camera tracking origin.");
		}
	}
	if (ImGui::InputFloat(
		"Shake Amplitude", &cue->fShakeAmplitude, 0.01f, 0.1f, "%.3f"))
	{
		Mark_Dirty("Edited camera shake amplitude.");
	}
	if (ImGui::InputScalar(
		"Shake Duration (ms)", ImGuiDataType_U32, &cue->iShakeDurationMs))
	{
		Mark_Dirty("Edited camera shake duration.");
	}
	ImGui::TextDisabled("Current easing: %s", Easing_Label(cue->eEasing));

	Render_PreviewControls(*cue);
	Render_KeyframeEditor(*cue);
}

void Client::CCameraTool::Render_KeyframeEditor(
	VALTAN_CINEMATIC_CAMERA_CUE& cue)
{
	ImGui::SeparatorText("Keyframes");
	if (ImGui::Button("Insert Key"))
		(void)Insert_Keyframe(cue);
	ImGui::SameLine();
	ImGui::BeginDisabled(m_iSelectedKeyframe <= 0 ||
		m_iSelectedKeyframe >= static_cast<int32_t>(cue.Keyframes.size()) - 1);
	if (ImGui::Button("Delete Key"))
		(void)Delete_SelectedKeyframe(cue);
	ImGui::EndDisabled();

	if (ImGui::BeginListBox("##cameraKeys", ImVec2(220.f, 150.f)))
	{
		for (size_t index = 0u; index < cue.Keyframes.size(); ++index)
		{
			const std::string label = std::to_string(index) + " | " +
				std::to_string(cue.Keyframes[index].iTimeMs) + " ms";
			if (ImGui::Selectable(
				label.c_str(), static_cast<int32_t>(index) == m_iSelectedKeyframe))
			{
				m_iSelectedKeyframe = static_cast<int32_t>(index);
				m_fPreviewSeconds =
					static_cast<f32_t>(cue.Keyframes[index].iTimeMs) * 0.001f;
			}
		}
		ImGui::EndListBox();
	}
	if (m_iSelectedKeyframe < 0 ||
		m_iSelectedKeyframe >= static_cast<int32_t>(cue.Keyframes.size()))
	{
		return;
	}
	VALTAN_CINEMATIC_CAMERA_KEYFRAME& key =
		cue.Keyframes[static_cast<size_t>(m_iSelectedKeyframe)];
	const bool_t endpoint = 0 == m_iSelectedKeyframe ||
		m_iSelectedKeyframe == static_cast<int32_t>(cue.Keyframes.size()) - 1;
	ImGui::BeginDisabled(endpoint);
	uint32_t timeMs = key.iTimeMs;
	if (ImGui::InputScalar("Key Time (ms)", ImGuiDataType_U32, &timeMs))
	{
		key.iTimeMs = timeMs;
		Mark_Dirty("Edited camera key time.");
	}
	ImGui::EndDisabled();
	if (ImGui::InputFloat3("Eye", &key.vEye.x, "%.3f"))
		Mark_Dirty("Edited camera key eye.");
	if (ImGui::InputFloat3("LookAt", &key.vLookAt.x, "%.3f"))
		Mark_Dirty("Edited camera key lookAt.");
	if (ImGui::InputFloat("FOV Y", &key.fFovYDegrees, 0.25f, 1.f, "%.2f"))
		Mark_Dirty("Edited camera key FOV.");
	if (ImGui::Button("Capture Current Camera"))
	{
		Stop_Preview(false);
		if (Capture_CurrentPose(key))
			Mark_Dirty("Captured current camera pose into the selected key.");
		else
			m_strStatus = "Camera capture failed because the pipeline pose is unavailable.";
	}
	if (endpoint)
		ImGui::TextDisabled("First and final key times are fixed to 0 and cue duration.");
}

void Client::CCameraTool::Render_PreviewControls(
	VALTAN_CINEMATIC_CAMERA_CUE& cue)
{
	ImGui::SeparatorText("Product Sampler Preview");
	if (ImGui::Button(m_bPlaying ? "Pause" : "Play"))
	{
		if (m_bPlaying)
			m_bPlaying = false;
		else if (Apply_PreviewPose())
			m_bPlaying = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop / Restore"))
		Stop_Preview(true);
	const f32_t maximum = static_cast<f32_t>(cue.iDurationMs) * 0.001f;
	f32_t cursor = (std::clamp)(m_fPreviewSeconds, 0.f, maximum);
	if (ImGui::SliderFloat("Time (s)", &cursor, 0.f, maximum, "%.3f"))
	{
		m_fPreviewSeconds = cursor;
		m_bPlaying = false;
		(void)Apply_PreviewPose();
	}
}

void Client::CCameraTool::Mark_Dirty(const char_t* status)
{
	m_bDirty = true;
	m_bPreviewDraftStale = true;
	m_strStatus = status;
}

bool_t Client::CCameraTool::Insert_Keyframe(
	VALTAN_CINEMATIC_CAMERA_CUE& cue)
{
	if (cue.Keyframes.size() < 2u)
	{
		m_strStatus = "Insert requires two valid endpoint keys.";
		return false;
	}
	size_t right = 1u;
	if (m_iSelectedKeyframe > 0)
	{
		right = (std::min)(
			static_cast<size_t>(m_iSelectedKeyframe + 1),
			cue.Keyframes.size() - 1u);
	}
	const size_t left = right - 1u;
	const uint32_t leftTime = cue.Keyframes[left].iTimeMs;
	const uint32_t rightTime = cue.Keyframes[right].iTimeMs;
	if (rightTime <= leftTime + 1u)
	{
		m_strStatus = "No millisecond gap is available for another key.";
		return false;
	}
	VALTAN_CINEMATIC_CAMERA_KEYFRAME key = cue.Keyframes[left];
	key.iTimeMs = leftTime + (rightTime - leftTime) / 2u;
	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	if (CValtanCinematicCameraController::Sample_Cue(
		cue, static_cast<f32_t>(key.iTimeMs) * 0.001f, pose))
	{
		key.vEye = pose.vEye;
		key.vLookAt = pose.vLookAt;
		key.fFovYDegrees = pose.fFovYDegrees;
	}
	cue.Keyframes.insert(cue.Keyframes.begin() + right, key);
	m_iSelectedKeyframe = static_cast<int32_t>(right);
	Mark_Dirty("Inserted a sampled camera key into the cue draft.");
	return true;
}

bool_t Client::CCameraTool::Delete_SelectedKeyframe(
	VALTAN_CINEMATIC_CAMERA_CUE& cue)
{
	if (cue.Keyframes.size() <= 2u || m_iSelectedKeyframe <= 0 ||
		m_iSelectedKeyframe >= static_cast<int32_t>(cue.Keyframes.size()) - 1)
	{
		m_strStatus = "Only interior keys can be deleted.";
		return false;
	}
	cue.Keyframes.erase(cue.Keyframes.begin() + m_iSelectedKeyframe);
	m_iSelectedKeyframe = (std::min)(
		m_iSelectedKeyframe,
		static_cast<int32_t>(cue.Keyframes.size()) - 1);
	Mark_Dirty("Deleted the selected interior camera key.");
	return true;
}
