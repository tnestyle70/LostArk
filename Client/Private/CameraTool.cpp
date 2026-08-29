#include "imgui.h"

#include "CameraTool.h"

#include "Camera_Free.h"
#include "Bounding_Sphere.h"
#include "Collider.h"
#include "GameInstance.h"
#include "MainApp.h"
#include "ProjectDataRoot.h"
#include "ValtanCinematicCameraController.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>

namespace
{
	using namespace Client;
	constexpr f32_t MAX_CAMERA_SHAKE_AMPLITUDE = 2.f;
	constexpr uint32_t MAX_CAMERA_SHAKE_DURATION_MS = 1000u;
	constexpr uint32_t DEFAULT_CAMERA_SHAKE_DURATION_MS = 250u;
	constexpr f32_t MIN_LOOK_AT_DUMMY_RADIUS = 0.05f;
	constexpr f32_t MAX_LOOK_AT_DUMMY_RADIUS = 5.f;
	/* Both mirror the document validator so a bad new cut fails here with a
	   reason instead of only at the strict save reparse. */
	constexpr size_t MAX_DOCUMENT_CUE_COUNT = 32u;
	constexpr size_t MAX_DOCUMENT_KEYFRAME_COUNT = 64u;
	constexpr size_t MAX_CUT_NAME_LENGTH = 128u;
	constexpr uint32_t DEFAULT_DEATH_CUT_DURATION_MS = 10000u;

	bool_t Is_ValidCutName(const std::string& value)
	{
		return !value.empty() && value.size() <= MAX_CUT_NAME_LENGTH &&
			std::all_of(value.begin(), value.end(), [](const char_t character)
			{
				return (character >= 'a' && character <= 'z') ||
					(character >= 'A' && character <= 'Z') ||
					(character >= '0' && character <= '9') ||
					'_' == character || '-' == character || '.' == character;
			});
	}

	class SCOPED_WIN32_HANDLE final
	{
	public:
		explicit SCOPED_WIN32_HANDLE(const HANDLE handle) : m_Handle(handle) {}
		~SCOPED_WIN32_HANDLE()
		{
			if (INVALID_HANDLE_VALUE != m_Handle && nullptr != m_Handle)
				CloseHandle(m_Handle);
		}

		SCOPED_WIN32_HANDLE(const SCOPED_WIN32_HANDLE&) = delete;
		SCOPED_WIN32_HANDLE& operator=(const SCOPED_WIN32_HANDLE&) = delete;

		bool_t Is_Valid() const
		{
			return INVALID_HANDLE_VALUE != m_Handle && nullptr != m_Handle;
		}

	private:
		HANDLE m_Handle = INVALID_HANDLE_VALUE;
	};

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

	bool_t Reserve_UniqueSiblingPath(
		const std::filesystem::path& destination,
		const wchar_t* marker,
		std::filesystem::path& outPath,
		std::string& outStatus)
	{
		static std::atomic_uint64_t sequence{ 0u };
		for (uint32_t attempt = 0u; attempt < 1024u; ++attempt)
		{
			std::filesystem::path candidate = destination;
			candidate += std::wstring(marker) +
				std::to_wstring(GetCurrentProcessId()) + L"." +
				std::to_wstring(++sequence);
			SCOPED_WIN32_HANDLE reservation(CreateFileW(
				candidate.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
				nullptr, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, nullptr));
			if (reservation.Is_Valid())
			{
				outPath = std::move(candidate);
				return true;
			}
			const DWORD error = GetLastError();
			if (ERROR_FILE_EXISTS != error && ERROR_ALREADY_EXISTS != error)
			{
				outStatus = "Failed to reserve a unique sibling save path (Win32 " +
					std::to_string(error) + ").";
				return false;
			}
		}
		outStatus = "Failed to reserve a unique sibling save path after 1024 collisions.";
		return false;
	}

	std::filesystem::path Make_SaveLockPath(
		const std::filesystem::path& destination)
	{
		std::filesystem::path lockPath = destination;
		lockPath += L".save.lock";
		return lockPath;
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

	const char_t* Interpolation_Label(
		const VALTAN_CINEMATIC_CAMERA_INTERPOLATION interpolation)
	{
		switch (interpolation)
		{
		case VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR:
			return "LINEAR";
		case VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM:
			return "CATMULL_ROM";
		default:
			return "INVALID";
		}
	}

	f32_t Distance(const float3_t& left, const float3_t& right)
	{
		const f32_t deltaX = right.x - left.x;
		const f32_t deltaY = right.y - left.y;
		const f32_t deltaZ = right.z - left.z;
		return std::sqrt(
			deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
	}

	bool_t Is_ValidAuthoringPosition(const float3_t& value)
	{
		constexpr f32_t MAX_ABSOLUTE_POSITION = 100000.f;
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z) &&
			std::abs(value.x) <= MAX_ABSOLUTE_POSITION &&
			std::abs(value.y) <= MAX_ABSOLUTE_POSITION &&
			std::abs(value.z) <= MAX_ABSOLUTE_POSITION;
	}

	bool_t Is_ValidAuthoringPose(
		const VALTAN_CINEMATIC_CAMERA_POSE& pose)
	{
		return Is_ValidAuthoringPosition(pose.vEye) &&
			Is_ValidAuthoringPosition(pose.vLookAt) &&
			std::isfinite(pose.fFovYDegrees) &&
			pose.fFovYDegrees >= 10.f && pose.fFovYDegrees <= 120.f &&
			Distance(pose.vEye, pose.vLookAt) > 0.001f;
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
	Update_LookAtDummy();
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
	m_fPreviewSeconds +=
		(std::min)(timeDelta, 0.1f) * m_fPreviewRate;
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
#ifdef _DEBUG
	if (ImGui::Button("Complete Play (Server/Arena)##CameraTool"))
	{
		if (CMainApp* const app = CMainApp::Get_Active())
			(void)app->Debug_CompletePlaySelected(m_strCompletePlayStatus);
		else
			m_strCompletePlayStatus = "Complete Play workspace is unavailable.";
	}
	ImGui::SameLine();
	ImGui::TextDisabled("%s", m_strCompletePlayStatus.c_str());
	ImGui::SameLine();
#endif
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
	Reset_LookAtDummy();
}

void Client::CCameraTool::On_LevelChanged()
{
	Deactivate();
	m_pPreviewCamera.reset();
	m_bLookAtDummyEnabled = false;
	m_vLookAtDummyWorld = {};
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
	const std::filesystem::path saveLockPath = Make_SaveLockPath(m_CameraPath);
	SCOPED_WIN32_HANDLE saveLock(CreateFileW(
		saveLockPath.c_str(), GENERIC_READ | GENERIC_WRITE | DELETE, 0,
		nullptr, OPEN_ALWAYS,
		FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr));
	if (!saveLock.Is_Valid())
	{
		m_strStatus =
			"Save is already in progress for this camera document. Draft and preview were preserved.";
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

	std::filesystem::path temporary;
	if (!Reserve_UniqueSiblingPath(
		m_CameraPath, L".tmp.", temporary, status))
	{
		m_strStatus = "Save failed to reserve a temporary camera document: " + status;
		return false;
	}
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
	std::filesystem::path replacedBackup;
	if (!Reserve_UniqueSiblingPath(
		m_CameraPath, L".replaced-backup.", replacedBackup, status))
	{
		Remove_Temporary(temporary);
		m_strStatus = "Save failed to reserve a displaced-byte backup: " + status;
		return false;
	}
	if (FALSE == ReplaceFileW(
		m_CameraPath.c_str(), temporary.c_str(), replacedBackup.c_str(),
		0u, nullptr, nullptr))
	{
		Remove_Temporary(temporary);
		m_strStatus =
			"Camera atomic replace failed. Destination and draft were not committed; any OS recovery backup was preserved at: " +
			replacedBackup.string();
		return false;
	}

	/* ReplaceFile preserves the exact destination bytes that were displaced.
	   Comparing that backup closes the read/rename race left by a plain
	   check-then-MoveFileEx sequence. An uncooperative text editor can still
	   write without observing our sidecar lock; if that happens, restore or
	   preserve its bytes instead of silently losing them. */
	std::string replacedText;
	if (!Read_TextFile(replacedBackup, replacedText, status) ||
		replacedText != m_strBaselineText)
	{
		std::string destinationAfterReplace;
		const bool_t destinationIsOurDraft =
			Read_TextFile(m_CameraPath, destinationAfterReplace, status) &&
			destinationAfterReplace == serialized;
		if (destinationIsOurDraft)
		{
			std::filesystem::path displacedRecovery;
			if (!Reserve_UniqueSiblingPath(
				m_CameraPath, L".conflict-recovery.",
				displacedRecovery, status))
			{
				m_strStatus =
					"Save conflict detected, but rollback recovery could not be reserved. Concurrent source bytes remain preserved at: " +
					replacedBackup.string();
				return false;
			}
			if (FALSE != ReplaceFileW(
				m_CameraPath.c_str(), replacedBackup.c_str(),
				displacedRecovery.c_str(), 0u, nullptr, nullptr))
			{
				std::string displacedText;
				if (Read_TextFile(displacedRecovery, displacedText, status) &&
					displacedText == serialized)
				{
					Remove_Temporary(displacedRecovery);
					m_strStatus =
						"Save conflict detected during replace. Concurrent source bytes were restored; draft and preview were preserved.";
				}
				else
				{
					m_strStatus =
						"Save conflict detected during rollback. Concurrent bytes were preserved at: " +
						displacedRecovery.string();
				}
				return false;
			}
		}
		m_strStatus =
			"Save conflict detected during atomic replace. The displaced concurrent source was preserved at: " +
			replacedBackup.string();
		return false;
	}
	Remove_Temporary(replacedBackup);

	m_LoadedDocument = std::move(temporaryDocument);
	m_strBaselineText = std::move(serialized);
	m_DraftCues = m_LoadedDocument.Get_Cues();
	m_hasDraftDeathCue = m_LoadedDocument.Has_DeathCue();
	m_DraftDeathCue = m_hasDraftDeathCue ?
		*m_LoadedDocument.Find_DeathCue() : VALTAN_CINEMATIC_CAMERA_CUE{};
	m_bDirty = false;
	m_bPreviewDraftStale = true;
	m_strStatus =
		"Saved canonical cinematic camera document with locked conflict recovery. Reload the encounter to consume it.";
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
	if (m_bLookAtDummyEnabled &&
		(nullptr == Get_SelectedCue() ||
			!Sync_DummyFromSelectedScene(*Get_SelectedCue())))
	{
		m_bLookAtDummyEnabled = false;
		Reset_LookAtDummy();
	}
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
	if (m_bLookAtDummyEnabled &&
		(nullptr == Get_SelectedCue() ||
			!Sync_DummyFromSelectedScene(*Get_SelectedCue())))
	{
		m_bLookAtDummyEnabled = false;
		Reset_LookAtDummy();
		m_strStatus =
			"LookAt Dummy was disabled because the selected cue has no valid tracking frame.";
	}
}

std::shared_ptr<Client::CCamera_Free>
Client::CCameraTool::Find_CurrentCamera() const
{
	const uint32_t level = CGameInstance::Get().Get_CurrentLevelID();
	return std::dynamic_pointer_cast<CCamera_Free>(
		CGameInstance::Get().Get_GameObject(level, TEXT("Layer_Camera"), 0));
}

bool_t Client::CCameraTool::Build_TrackingInput(
	const VALTAN_CINEMATIC_CAMERA_CUE& cue,
	VALTAN_CINEMATIC_CAMERA_INPUT& outInput) const
{
	outInput = {};
	if (VALTAN_CINEMATIC_TRACKING_MODE::WORLD == cue.eTrackingMode)
		return true;
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (!g_ActorPreviewContext.isValid ||
		g_ActorPreviewContext.iLevelIndex != currentLevel)
	{
		return false;
	}
	outInput.vBossPosition = g_ActorPreviewContext.vBossPosition;
	outInput.fBossYawDegrees = g_ActorPreviewContext.fBossYawDegrees;
	outInput.hasLocalPlayerPosition =
		g_ActorPreviewContext.hasLocalPlayerPosition;
	outInput.vLocalPlayerPosition =
		g_ActorPreviewContext.vLocalPlayerPosition;
	return true;
}

bool_t Client::CCameraTool::Resolve_SceneWorldPose(
	const VALTAN_CINEMATIC_CAMERA_CUE& cue,
	const VALTAN_CINEMATIC_CAMERA_KEYFRAME& scene,
	VALTAN_CINEMATIC_CAMERA_POSE& outPose) const
{
	outPose = { scene.vEye, scene.vLookAt, scene.fFovYDegrees };
	if (VALTAN_CINEMATIC_TRACKING_MODE::WORLD == cue.eTrackingMode)
		return true;
	VALTAN_CINEMATIC_CAMERA_INPUT input{};
	return Build_TrackingInput(cue, input) &&
		CValtanCinematicCameraController::Apply_CueTracking(
			cue, input, outPose);
}

bool_t Client::CCameraTool::Capture_CurrentPose(
	const VALTAN_CINEMATIC_CAMERA_CUE& cue,
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
	const vector_t lookBasis = cameraWorld.r[2];
	const f32_t lookLengthSq = XMVectorGetX(XMVector3LengthSq(lookBasis));
	if (!std::isfinite(lookLengthSq) || lookLengthSq <= 0.000001f)
		return false;
	const vector_t look = XMVector3Normalize(lookBasis);
	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	XMStoreFloat3(&pose.vEye, eye);
	if (m_bLookAtDummyEnabled)
		pose.vLookAt = m_vLookAtDummyWorld;
	else
		XMStoreFloat3(&pose.vLookAt, eye + look * 10.f);
	pose.fFovYDegrees = XMConvertToDegrees(
		2.f * std::atan(1.f / projection->_22));
	if (!Is_ValidAuthoringPose(pose))
		return false;
	if (VALTAN_CINEMATIC_TRACKING_MODE::WORLD != cue.eTrackingMode)
	{
		VALTAN_CINEMATIC_CAMERA_INPUT input{};
		if (!Build_TrackingInput(cue, input) ||
			!CValtanCinematicCameraController::Remove_CueTracking(
				cue, input, pose))
		{
			return false;
		}
	}
	outKeyframe.vEye = pose.vEye;
	outKeyframe.vLookAt = pose.vLookAt;
	outKeyframe.fFovYDegrees = pose.fFovYDegrees;
	return Is_ValidAuthoringPose(pose);
}

bool_t Client::CCameraTool::Ensure_LookAtDummy()
{
	if (!m_bLookAtDummyEnabled)
		return false;
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (nullptr != m_pLookAtDummyCollider &&
		m_iLookAtDummyLevel == currentLevel)
	{
		return true;
	}
	Reset_LookAtDummy();
	if (!std::isfinite(m_fLookAtDummyRadius))
	{
		Disable_LookAtDummy(
			"LookAt Dummy was disabled because its radius is invalid.");
		return false;
	}
	m_fLookAtDummyRadius = std::clamp(
		m_fLookAtDummyRadius,
		MIN_LOOK_AT_DUMMY_RADIUS,
		MAX_LOOK_AT_DUMMY_RADIUS);
	Engine::CBounding_Sphere::BOUNDING_SPHERE_DESC descriptor{};
	descriptor.vCenter = float3_t(0.f, 0.f, 0.f);
	descriptor.fRadius = m_fLookAtDummyRadius;
	m_pLookAtDummyCollider = std::dynamic_pointer_cast<Engine::CCollider>(
		CGameInstance::Get().Clone_Prototype(
			currentLevel,
			TEXT("Prototype_Component_Collider_WorldEntity"),
			&descriptor));
	if (nullptr == m_pLookAtDummyCollider)
		return false;
	m_iLookAtDummyLevel = currentLevel;
	return true;
}

void Client::CCameraTool::Update_LookAtDummy()
{
	if (!m_bLookAtDummyEnabled)
		return;
	if (!Is_ValidAuthoringPosition(m_vLookAtDummyWorld))
	{
		m_bLookAtDummyEnabled = false;
		Reset_LookAtDummy();
		m_strStatus = "LookAt Dummy was disabled because its world position is invalid.";
		return;
	}
	if (!Ensure_LookAtDummy())
		return;
	m_pLookAtDummyCollider->Update(XMMatrixTranslation(
		m_vLookAtDummyWorld.x,
		m_vLookAtDummyWorld.y,
		m_vLookAtDummyWorld.z));
#ifdef _DEBUG
	(void)CGameInstance::Get().Add_DebugComponent(m_pLookAtDummyCollider);
#endif
}

void Client::CCameraTool::Reset_LookAtDummy()
{
	m_pLookAtDummyCollider.reset();
	m_iLookAtDummyLevel = static_cast<uint32_t>(-1);
}

void Client::CCameraTool::Disable_LookAtDummy(const char_t* status)
{
	m_bLookAtDummyEnabled = false;
	m_vLookAtDummyWorld = {};
	Reset_LookAtDummy();
	if (nullptr != status)
		m_strStatus = status;
}

bool_t Client::CCameraTool::Sync_DummyFromSelectedScene(
	const VALTAN_CINEMATIC_CAMERA_CUE& cue)
{
	if (m_iSelectedKeyframe < 0 ||
		m_iSelectedKeyframe >= static_cast<int32_t>(cue.Keyframes.size()))
	{
		return false;
	}
	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	if (!Resolve_SceneWorldPose(
		cue, cue.Keyframes[static_cast<size_t>(m_iSelectedKeyframe)], pose) ||
		!Is_ValidAuthoringPose(pose))
	{
		return false;
	}
	m_vLookAtDummyWorld = pose.vLookAt;
	m_bLookAtDummyEnabled = true;
	return true;
}

bool_t Client::CCameraTool::Apply_DummyToSelectedScene(
	VALTAN_CINEMATIC_CAMERA_CUE& cue)
{
	if (!m_bLookAtDummyEnabled || m_iSelectedKeyframe < 0 ||
		m_iSelectedKeyframe >= static_cast<int32_t>(cue.Keyframes.size()))
	{
		return false;
	}
	VALTAN_CINEMATIC_CAMERA_KEYFRAME& scene =
		cue.Keyframes[static_cast<size_t>(m_iSelectedKeyframe)];
	if (!Is_ValidAuthoringPosition(m_vLookAtDummyWorld))
		return false;
	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	if (!Resolve_SceneWorldPose(cue, scene, pose))
		return false;
	pose.vLookAt = m_vLookAtDummyWorld;
	if (!Is_ValidAuthoringPose(pose))
		return false;
	if (VALTAN_CINEMATIC_TRACKING_MODE::WORLD != cue.eTrackingMode)
	{
		VALTAN_CINEMATIC_CAMERA_INPUT input{};
		if (!Build_TrackingInput(cue, input) ||
			!CValtanCinematicCameraController::Remove_CueTracking(
				cue, input, pose))
		{
			return false;
		}
	}
	if (!Is_ValidAuthoringPose(pose))
		return false;
	scene.vLookAt = pose.vLookAt;
	return true;
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
	Render_CutManagement();
	ImGui::SeparatorText("Cut List");
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

void Client::CCameraTool::Render_CutManagement()
{
	ImGui::SeparatorText("Cut Management");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##newCutName", "cut name (a-z 0-9 . _ -)",
		m_szNewCutName, sizeof(m_szNewCutName));
	ImGui::Checkbox("Death (clear) cut", &m_bNewCutTargetsDeath);
	if (!m_bNewCutTargetsDeath)
	{
		const std::vector<ENCOUNTER_PATTERN_REFERENCE>& patterns =
			m_Encounter.Get_Patterns();
		if (patterns.empty())
		{
			ImGui::TextDisabled("Encounter reference has no patterns.");
		}
		else
		{
			m_iNewCutPatternIndex = (std::clamp)(
				m_iNewCutPatternIndex, 0,
				static_cast<int32_t>(patterns.size()) - 1);
			const ENCOUNTER_PATTERN_REFERENCE& pattern =
				patterns[static_cast<size_t>(m_iNewCutPatternIndex)];
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::BeginCombo("##newCutPattern", pattern.patternId.c_str()))
			{
				for (size_t index = 0u; index < patterns.size(); ++index)
				{
					if (ImGui::Selectable(
						patterns[index].patternId.c_str(),
						static_cast<int32_t>(index) == m_iNewCutPatternIndex))
					{
						m_iNewCutPatternIndex = static_cast<int32_t>(index);
						m_iNewCutStageIndex = 0;
					}
				}
				ImGui::EndCombo();
			}
			if (pattern.stages.empty())
			{
				ImGui::TextDisabled("Selected pattern has no stages.");
			}
			else
			{
				m_iNewCutStageIndex = (std::clamp)(
					m_iNewCutStageIndex, 0,
					static_cast<int32_t>(pattern.stages.size()) - 1);
				const std::string selectedStage =
					pattern.stages[static_cast<size_t>(m_iNewCutStageIndex)].stageId +
					" | " + std::to_string(pattern.stages[
						static_cast<size_t>(m_iNewCutStageIndex)].iDurationMs) + " ms";
				ImGui::SetNextItemWidth(-1.f);
				if (ImGui::BeginCombo("##newCutStage", selectedStage.c_str()))
				{
					for (size_t index = 0u; index < pattern.stages.size(); ++index)
					{
						const std::string label = pattern.stages[index].stageId +
							" | " + std::to_string(pattern.stages[index].iDurationMs) +
							" ms";
						if (ImGui::Selectable(label.c_str(),
							static_cast<int32_t>(index) == m_iNewCutStageIndex))
						{
							m_iNewCutStageIndex = static_cast<int32_t>(index);
						}
					}
					ImGui::EndCombo();
				}
			}
		}
	}
	if (ImGui::Button("New Cut"))
		(void)Create_Cut();
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == Get_SelectedCue());
	if (ImGui::Button("Delete Cut"))
		ImGui::OpenPopup("Delete camera cut?");
	ImGui::EndDisabled();
	if (ImGui::BeginPopupModal(
		"Delete camera cut?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextWrapped(
			"Delete '%s' from the draft? Save commits the removal; a cut that "
			"the encounter or automated camera contract still expects will "
			"fail those gates until they are updated.",
			m_strSelectedCueId.c_str());
		if (ImGui::Button("Delete"))
		{
			(void)Delete_SelectedCut();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void Client::CCameraTool::Render_CueEditor()
{
	VALTAN_CINEMATIC_CAMERA_CUE* cue = Get_SelectedCue();
	if (nullptr == cue)
	{
		ImGui::TextDisabled("Select a camera cut.");
		return;
	}
	ImGui::SeparatorText("Cut Draft");
	ImGui::Text("%s", cue->strCueId.c_str());
	if (!Is_DeathCueSelected())
	{
		ImGui::TextDisabled("Pattern %s  |  Stage %s",
			cue->strPatternId.c_str(), cue->strStageId.c_str());
	}

	/* Simple flow first: capture positions, review them in the pos list,
	   fine-tune the selected one, then Start plays the connected path. The
	   full editor stays available under Advanced. */
	if (ImGui::Button("Capture Pos"))
		(void)Append_CapturedPos(*cue);
	ImGui::SameLine();
	ImGui::BeginDisabled(m_iSelectedKeyframe < 0 || cue->Keyframes.size() <= 2u);
	if (ImGui::Button("Delete Pos"))
		(void)Delete_SelectedPos(*cue);
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Start"))
	{
		m_fPreviewSeconds = 0.f;
		if (Apply_PreviewPose())
			m_bPlaying = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop / Restore"))
		Stop_Preview(true);
	const f32_t maximum = static_cast<f32_t>(cue->iDurationMs) * 0.001f;
	f32_t cursor = (std::clamp)(m_fPreviewSeconds, 0.f, maximum);
	if (ImGui::SliderFloat("Time (s)", &cursor, 0.f, maximum, "%.3f"))
	{
		m_fPreviewSeconds = cursor;
		m_bPlaying = false;
		(void)Apply_PreviewPose();
	}

	Render_KeyframeEditor(*cue);
	Render_AdvancedEditor(*cue);
}

void Client::CCameraTool::Render_AdvancedEditor(
	VALTAN_CINEMATIC_CAMERA_CUE& cueReference)
{
	if (!ImGui::CollapsingHeader("Advanced (full editor)"))
		return;
	VALTAN_CINEMATIC_CAMERA_CUE* cue = &cueReference;

	uint32_t duration = cue->iDurationMs;
	if (ImGui::InputScalar("Duration (ms)", ImGuiDataType_U32, &duration))
	{
		uint32_t minimumDuration = (std::max)(
			1u, (std::max)(cue->iTransitionInMs, cue->iShakeDurationMs));
		if (cue->Keyframes.size() >= 2u)
		{
			minimumDuration = (std::max)(minimumDuration,
				cue->Keyframes[cue->Keyframes.size() - 2u].iTimeMs + 1u);
		}
		const uint32_t maximumDuration = Get_MaximumCueDuration(*cue);
		if (minimumDuration > maximumDuration)
		{
			m_strStatus =
				"Duration edit rejected: transition, shake, or scene timing exceeds the bound stage.";
			return;
		}
		cue->iDurationMs = (std::clamp)(duration, minimumDuration, maximumDuration);
		if (!cue->Keyframes.empty())
			cue->Keyframes.back().iTimeMs = cue->iDurationMs;
		m_fPreviewSeconds = (std::min)(
			m_fPreviewSeconds,
			static_cast<f32_t>(cue->iDurationMs) * 0.001f);
		Mark_Dirty("Edited camera cue duration.");
	}

	int interpolation = static_cast<int>(cue->eInterpolation);
	const char_t* interpolations[] = { "LINEAR", "CATMULL_ROM" };
	if (ImGui::Combo("Path Interpolation", &interpolation,
		interpolations, 2))
	{
		cue->eInterpolation =
			static_cast<VALTAN_CINEMATIC_CAMERA_INTERPOLATION>(interpolation);
		Mark_Dirty("Edited camera cue path interpolation.");
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
			if (m_bLookAtDummyEnabled && !Sync_DummyFromSelectedScene(*cue))
			{
				m_bLookAtDummyEnabled = false;
				Reset_LookAtDummy();
				m_strStatus =
					"LookAt Dummy was disabled because the new tracking frame is unavailable.";
			}
		}
		if (VALTAN_CINEMATIC_TRACKING_MODE::WORLD != cue->eTrackingMode &&
			ImGui::InputFloat3("Tracking Origin", &cue->vTrackingOrigin.x, "%.3f"))
		{
			Mark_Dirty("Edited camera tracking origin.");
			if (m_bLookAtDummyEnabled && !Sync_DummyFromSelectedScene(*cue))
			{
				m_bLookAtDummyEnabled = false;
				Reset_LookAtDummy();
				m_strStatus =
					"LookAt Dummy was disabled because the tracking origin is invalid.";
			}
		}
	}
	if (ImGui::InputFloat(
		"Shake Amplitude", &cue->fShakeAmplitude, 0.01f, 0.1f, "%.3f"))
	{
		cue->fShakeAmplitude = (std::clamp)(
			cue->fShakeAmplitude, 0.f, MAX_CAMERA_SHAKE_AMPLITUDE);
		if (cue->fShakeAmplitude <= 0.f)
			cue->iShakeDurationMs = 0u;
		else if (0u == cue->iShakeDurationMs)
			cue->iShakeDurationMs = (std::min)(
				DEFAULT_CAMERA_SHAKE_DURATION_MS, cue->iDurationMs);
		Mark_Dirty("Edited camera shake amplitude.");
	}
	if (ImGui::InputScalar(
		"Shake Duration (ms)", ImGuiDataType_U32, &cue->iShakeDurationMs))
	{
		cue->iShakeDurationMs = (std::min)(
			cue->iShakeDurationMs,
			(std::min)(MAX_CAMERA_SHAKE_DURATION_MS, cue->iDurationMs));
		if (0u == cue->iShakeDurationMs)
			cue->fShakeAmplitude = 0.f;
		else if (cue->fShakeAmplitude <= 0.f)
			cue->fShakeAmplitude = 0.1f;
		Mark_Dirty("Edited camera shake duration.");
	}
	ImGui::TextDisabled("Path %s | Easing %s",
		Interpolation_Label(cue->eInterpolation), Easing_Label(cue->eEasing));
	ImGui::SliderFloat(
		"Preview Rate (authoring only)", &m_fPreviewRate,
		0.1f, 4.f, "%.2fx");

	if (m_iSelectedKeyframe >= 0 &&
		m_iSelectedKeyframe < static_cast<int32_t>(cue->Keyframes.size()))
	{
		VALTAN_CINEMATIC_CAMERA_KEYFRAME& key =
			cue->Keyframes[static_cast<size_t>(m_iSelectedKeyframe)];
		const bool_t endpoint = 0 == m_iSelectedKeyframe ||
			m_iSelectedKeyframe == static_cast<int32_t>(cue->Keyframes.size()) - 1;
		ImGui::BeginDisabled(endpoint);
		uint32_t timeMs = key.iTimeMs;
		if (ImGui::InputScalar("Key Time (ms)", ImGuiDataType_U32, &timeMs))
		{
			const size_t selected = static_cast<size_t>(m_iSelectedKeyframe);
			const uint32_t minimumTime = cue->Keyframes[selected - 1u].iTimeMs + 1u;
			const uint32_t maximumTime = cue->Keyframes[selected + 1u].iTimeMs - 1u;
			key.iTimeMs = (std::clamp)(timeMs, minimumTime, maximumTime);
			Mark_Dirty("Edited camera key time.");
		}
		ImGui::EndDisabled();
		if (endpoint)
			ImGui::TextDisabled(
				"First and final scene times stay bound to 0 and cue duration.");
	}
	if (ImGui::Button("Insert Sampled Scene"))
		(void)Insert_Keyframe(*cue);
	ImGui::SameLine();
	if (ImGui::Button("Capture New Scene"))
		(void)Insert_CapturedScene(*cue);
	ImGui::SameLine();
	ImGui::BeginDisabled(m_iSelectedKeyframe <= 0 ||
		m_iSelectedKeyframe >= static_cast<int32_t>(cue->Keyframes.size()) - 1);
	if (ImGui::Button("Delete Scene"))
		(void)Delete_SelectedKeyframe(*cue);
	ImGui::EndDisabled();
}

void Client::CCameraTool::Render_KeyframeEditor(
	VALTAN_CINEMATIC_CAMERA_CUE& cue)
{
	ImGui::SeparatorText("Pos List");
	if (ImGui::BeginListBox("##cameraPosList", ImVec2(-1.f, 150.f)))
	{
		for (size_t index = 0u; index < cue.Keyframes.size(); ++index)
		{
			const std::string label = "P" + std::to_string(index + 1u) + " | " +
				std::to_string(cue.Keyframes[index].iTimeMs) + " ms | " +
				cue.Keyframes[index].strSceneId;
			if (ImGui::Selectable(
				label.c_str(), static_cast<int32_t>(index) == m_iSelectedKeyframe))
			{
				m_iSelectedKeyframe = static_cast<int32_t>(index);
				m_bPlaying = false;
				m_fPreviewSeconds =
					static_cast<f32_t>(cue.Keyframes[index].iTimeMs) * 0.001f;
				(void)Apply_PreviewPose();
				if (m_bLookAtDummyEnabled && !Sync_DummyFromSelectedScene(cue))
				{
					Disable_LookAtDummy(
						"LookAt Dummy was disabled because the selected scene has no valid world pose.");
				}
			}
		}
		ImGui::EndListBox();
	}
	ImGui::TextDisabled(
		"Capture Pos appends here and clicking a pos moves the camera to it.");
	if (m_iSelectedKeyframe < 0 ||
		m_iSelectedKeyframe >= static_cast<int32_t>(cue.Keyframes.size()))
	{
		return;
	}
	VALTAN_CINEMATIC_CAMERA_KEYFRAME& key =
		cue.Keyframes[static_cast<size_t>(m_iSelectedKeyframe)];
	ImGui::SeparatorText("Fine Adjust");
	ImGui::TextDisabled("Scene ID: %s", key.strSceneId.c_str());
	if (ImGui::InputFloat3("Eye", &key.vEye.x, "%.3f"))
		Mark_Dirty("Edited camera key eye.");
	if (ImGui::InputFloat3("LookAt", &key.vLookAt.x, "%.3f"))
	{
		Mark_Dirty("Edited camera key lookAt.");
		if (m_bLookAtDummyEnabled && !Sync_DummyFromSelectedScene(cue))
		{
			Disable_LookAtDummy(
				"LookAt Dummy was disabled because the edited scene has no valid world pose.");
		}
	}
	if (ImGui::InputFloat("FOV Y", &key.fFovYDegrees, 0.25f, 1.f, "%.2f"))
		Mark_Dirty("Edited camera key FOV.");
	if (ImGui::Button("Capture / Replace Selected Scene"))
	{
		Stop_Preview(false);
		if (Capture_CurrentPose(cue, key))
			Mark_Dirty("Captured current camera pose into the selected key.");
		else
			m_strStatus =
				"Camera capture failed: pipeline pose or replicated tracking frame is unavailable.";
	}
	ImGui::SameLine();
	if (ImGui::Button("Go To Scene"))
	{
		m_bPlaying = false;
		m_fPreviewSeconds = static_cast<f32_t>(key.iTimeMs) * 0.001f;
		(void)Apply_PreviewPose();
	}

	if (m_iSelectedKeyframe > 0)
	{
		const size_t right = static_cast<size_t>(m_iSelectedKeyframe);
		const uint32_t deltaMs = key.iTimeMs - cue.Keyframes[right - 1u].iTimeMs;
		const f32_t arcLength = Calculate_SegmentArcLength(cue, right);
		const f32_t currentSpeed = 0u == deltaMs ? 0.f :
			arcLength / (static_cast<f32_t>(deltaMs) * 0.001f);
		ImGui::SeparatorText("Speed");
		ImGui::TextDisabled("Arc %.3f units | Current average %.3f units/s",
			arcLength, currentSpeed);
		ImGui::InputFloat(
			"Target Speed (units/s)", &m_fTargetSegmentSpeed,
			0.25f, 1.f, "%.3f");
		ImGui::SameLine();
		if (ImGui::Button("Apply Segment Speed"))
			(void)Apply_SelectedSegmentSpeed(cue, m_fTargetSegmentSpeed);
	}

	Render_LookAtDummyEditor(cue);
}

void Client::CCameraTool::Render_LookAtDummyEditor(
	VALTAN_CINEMATIC_CAMERA_CUE& cue)
{
	ImGui::SeparatorText("LookAt Dummy Collider");
	bool_t enabled = m_bLookAtDummyEnabled;
	if (ImGui::Checkbox("Show LookAt Dummy", &enabled))
	{
		m_bLookAtDummyEnabled = enabled;
		if (enabled)
		{
			if (!Sync_DummyFromSelectedScene(cue))
			{
				m_bLookAtDummyEnabled = false;
				Reset_LookAtDummy();
				m_strStatus =
					"LookAt Dummy needs the current replicated tracking frame.";
			}
			else if (!Ensure_LookAtDummy())
				m_strStatus =
					"LookAt values remain editable, but the debug collider prototype is unavailable.";
		}
		else
		{
			Reset_LookAtDummy();
		}
	}
	if (!m_bLookAtDummyEnabled)
	{
		ImGui::TextDisabled(
			"Enable the dummy to capture its world position as the scene LookAt.");
		return;
	}
	if (ImGui::DragFloat3(
		"Dummy World Position", &m_vLookAtDummyWorld.x, 0.05f,
		-100000.f, 100000.f, "%.3f"))
	{
		if (Apply_DummyToSelectedScene(cue))
		{
			Mark_Dirty("Applied the moving LookAt Dummy to the selected scene.");
			m_bPlaying = false;
			if (m_iSelectedKeyframe >= 0 &&
				m_iSelectedKeyframe < static_cast<int32_t>(cue.Keyframes.size()))
			{
				m_fPreviewSeconds = static_cast<f32_t>(
					cue.Keyframes[static_cast<size_t>(m_iSelectedKeyframe)].iTimeMs) *
					0.001f;
				(void)Apply_PreviewPose();
			}
		}
		else
		{
			m_strStatus =
				"LookAt Dummy move was rejected because its pose or tracking frame is invalid.";
		}
	}
	f32_t radius = m_fLookAtDummyRadius;
	if (ImGui::DragFloat(
		"Dummy Radius", &radius, 0.01f,
		MIN_LOOK_AT_DUMMY_RADIUS, MAX_LOOK_AT_DUMMY_RADIUS, "%.2f"))
	{
		if (!std::isfinite(radius))
		{
			m_strStatus =
				"Dummy Radius was rejected because it is not finite.";
		}
		else
		{
			m_fLookAtDummyRadius = std::clamp(
				radius,
				MIN_LOOK_AT_DUMMY_RADIUS,
				MAX_LOOK_AT_DUMMY_RADIUS);
			Reset_LookAtDummy();
		}
	}
	if (ImGui::Button("Dummy <- Selected Scene LookAt"))
	{
		if (Sync_DummyFromSelectedScene(cue))
			m_strStatus = "Moved LookAt Dummy to the selected scene.";
		else
			Disable_LookAtDummy(
				"LookAt Dummy was disabled because the selected scene has no valid tracking frame.");
	}
	ImGui::SameLine();
	if (ImGui::Button("Selected Scene LookAt <- Dummy"))
	{
		if (Apply_DummyToSelectedScene(cue))
			Mark_Dirty("Applied LookAt Dummy to the selected scene.");
		else
			m_strStatus =
				"LookAt apply failed: dummy overlaps Eye or tracking frame is unavailable.";
	}
	ImGui::TextDisabled(
		"Dragging the dummy updates LookAt live. Its collision is Debug visualization only.");
}

void Client::CCameraTool::Mark_Dirty(const char_t* status)
{
	m_bDirty = true;
	m_bPreviewDraftStale = true;
	m_strStatus = status;
}

std::string Client::CCameraTool::Make_UniqueSceneId(
	const VALTAN_CINEMATIC_CAMERA_CUE&) const
{
	const auto exists = [this](const std::string& sceneId)
	{
		const auto cueContains = [&sceneId](
			const VALTAN_CINEMATIC_CAMERA_CUE& candidate)
		{
			return std::any_of(
				candidate.Keyframes.begin(), candidate.Keyframes.end(),
				[&sceneId](const VALTAN_CINEMATIC_CAMERA_KEYFRAME& scene)
				{ return scene.strSceneId == sceneId; });
		};
		return std::any_of(m_DraftCues.begin(), m_DraftCues.end(), cueContains) ||
			(m_hasDraftDeathCue && cueContains(m_DraftDeathCue));
	};
	for (uint32_t candidate = 1u; candidate <= 999999u; ++candidate)
	{
		const std::string sceneId = "camera.scene.auto." +
			std::to_string(candidate);
		if (!exists(sceneId))
			return sceneId;
	}
	return {};
}

f32_t Client::CCameraTool::Calculate_SegmentArcLength(
	const VALTAN_CINEMATIC_CAMERA_CUE& cue,
	const size_t rightSceneIndex) const
{
	if (0u == rightSceneIndex || rightSceneIndex >= cue.Keyframes.size())
		return 0.f;
	VALTAN_CINEMATIC_CAMERA_CUE sampleCue = cue;
	sampleCue.fShakeAmplitude = 0.f;
	sampleCue.iShakeDurationMs = 0u;
	VALTAN_CINEMATIC_CAMERA_INPUT trackingInput{};
	const bool_t tracked =
		VALTAN_CINEMATIC_TRACKING_MODE::WORLD != sampleCue.eTrackingMode;
	if (tracked && !Build_TrackingInput(sampleCue, trackingInput))
		return 0.f;
	const uint32_t startMs = sampleCue.Keyframes[rightSceneIndex - 1u].iTimeMs;
	const uint32_t endMs = sampleCue.Keyframes[rightSceneIndex].iTimeMs;
	if (endMs <= startMs)
		return 0.f;
	constexpr uint32_t SAMPLE_COUNT = 24u;
	VALTAN_CINEMATIC_CAMERA_POSE previous{};
	if (!CValtanCinematicCameraController::Sample_Cue(
		sampleCue, static_cast<f32_t>(startMs) * 0.001f, previous))
	{
		return Distance(
			sampleCue.Keyframes[rightSceneIndex - 1u].vEye,
			sampleCue.Keyframes[rightSceneIndex].vEye);
	}
	if (tracked &&
		!CValtanCinematicCameraController::Apply_CueTracking(
			sampleCue, trackingInput, previous))
	{
		return 0.f;
	}
	f32_t length = 0.f;
	for (uint32_t sample = 1u; sample <= SAMPLE_COUNT; ++sample)
	{
		const f32_t alpha = static_cast<f32_t>(sample) /
			static_cast<f32_t>(SAMPLE_COUNT);
		const f32_t timeMs = static_cast<f32_t>(startMs) +
			static_cast<f32_t>(endMs - startMs) * alpha;
		VALTAN_CINEMATIC_CAMERA_POSE current{};
		if (!CValtanCinematicCameraController::Sample_Cue(
			sampleCue, timeMs * 0.001f, current))
		{
			return 0.f;
		}
		if (tracked &&
			!CValtanCinematicCameraController::Apply_CueTracking(
				sampleCue, trackingInput, current))
		{
			return 0.f;
		}
		length += Distance(previous.vEye, current.vEye);
		previous = current;
	}
	return length;
}

uint32_t Client::CCameraTool::Get_MaximumCueDuration(
	const VALTAN_CINEMATIC_CAMERA_CUE& cue) const
{
	if (Is_DeathCueSelected())
		return CEncounterPatternReference::MAX_STAGE_DURATION_MS;
	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_Encounter.Find_Pattern(cue.strPatternId);
	if (nullptr == pattern)
		return CEncounterPatternReference::MAX_STAGE_DURATION_MS;
	const auto stage = std::find_if(
		pattern->stages.begin(), pattern->stages.end(),
		[&cue](const ENCOUNTER_STAGE_REFERENCE& candidate)
		{ return candidate.stageId == cue.strStageId; });
	return pattern->stages.end() == stage ?
		CEncounterPatternReference::MAX_STAGE_DURATION_MS : stage->iDurationMs;
}

bool_t Client::CCameraTool::Apply_SelectedSegmentSpeed(
	VALTAN_CINEMATIC_CAMERA_CUE& cue,
	const f32_t targetSpeed)
{
	if (m_iSelectedKeyframe <= 0 ||
		m_iSelectedKeyframe >= static_cast<int32_t>(cue.Keyframes.size()) ||
		!std::isfinite(targetSpeed) || targetSpeed <= 0.01f)
	{
		m_strStatus = "Select a non-first scene and enter a positive speed.";
		return false;
	}
	const size_t right = static_cast<size_t>(m_iSelectedKeyframe);
	const f32_t arcLength = Calculate_SegmentArcLength(cue, right);
	if (!std::isfinite(arcLength) || arcLength <= 0.0001f)
	{
		m_strStatus = "Selected segment has no measurable camera movement.";
		return false;
	}
	const uint32_t oldDelta = cue.Keyframes[right].iTimeMs -
		cue.Keyframes[right - 1u].iTimeMs;
	const double desiredMilliseconds = static_cast<double>(arcLength) /
		static_cast<double>(targetSpeed) * 1000.0;
	if (!std::isfinite(desiredMilliseconds) || desiredMilliseconds < 1.0 ||
		desiredMilliseconds > static_cast<double>(
			CEncounterPatternReference::MAX_STAGE_DURATION_MS))
	{
		m_strStatus = "Target speed produces an out-of-range segment duration.";
		return false;
	}
	const int64_t newDelta = (std::max)(
		int64_t{ 1 }, static_cast<int64_t>(std::llround(desiredMilliseconds)));
	const int64_t shift = newDelta -
		static_cast<int64_t>(oldDelta);
	const int64_t newDuration = static_cast<int64_t>(cue.iDurationMs) + shift;
	if (newDuration <= 0 ||
		newDuration > static_cast<int64_t>(Get_MaximumCueDuration(cue)) ||
		newDuration < static_cast<int64_t>(cue.iTransitionInMs) ||
		newDuration < static_cast<int64_t>(cue.iShakeDurationMs))
	{
		m_strStatus =
			"Segment speed would move this cue beyond its authored stage duration.";
		return false;
	}
	for (size_t index = right; index < cue.Keyframes.size(); ++index)
	{
		const int64_t shifted =
			static_cast<int64_t>(cue.Keyframes[index].iTimeMs) + shift;
		if (shifted < 0 || shifted > newDuration)
		{
			m_strStatus = "Segment retime overflowed the camera scene clock.";
			return false;
		}
	}
	for (size_t index = right; index < cue.Keyframes.size(); ++index)
	{
		cue.Keyframes[index].iTimeMs = static_cast<uint32_t>(
			static_cast<int64_t>(cue.Keyframes[index].iTimeMs) + shift);
	}
	cue.iDurationMs = static_cast<uint32_t>(newDuration);
	m_fPreviewSeconds = static_cast<f32_t>(cue.Keyframes[right].iTimeMs) * 0.001f;
	Mark_Dirty("Retimed the selected camera segment from its target speed.");
	return true;
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
	key.strSceneId = Make_UniqueSceneId(cue);
	if (key.strSceneId.empty())
	{
		m_strStatus = "No stable scene ID is available for another scene.";
		return false;
	}
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

bool_t Client::CCameraTool::Insert_CapturedScene(
	VALTAN_CINEMATIC_CAMERA_CUE& cue)
{
	if (cue.Keyframes.size() < 2u)
	{
		m_strStatus = "Capture insert requires two valid endpoint scenes.";
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
		m_strStatus = "No millisecond gap is available for another scene.";
		return false;
	}
	VALTAN_CINEMATIC_CAMERA_KEYFRAME scene = cue.Keyframes[left];
	scene.strSceneId = Make_UniqueSceneId(cue);
	if (scene.strSceneId.empty())
	{
		m_strStatus = "No stable scene ID is available for another scene.";
		return false;
	}
	scene.iTimeMs = leftTime + (rightTime - leftTime) / 2u;
	Stop_Preview(false);
	if (!Capture_CurrentPose(cue, scene))
	{
		m_strStatus =
			"Scene capture failed: pipeline pose or tracking frame is unavailable.";
		return false;
	}
	cue.Keyframes.insert(cue.Keyframes.begin() + right, scene);
	m_iSelectedKeyframe = static_cast<int32_t>(right);
	m_fPreviewSeconds = static_cast<f32_t>(scene.iTimeMs) * 0.001f;
	Mark_Dirty("Captured and inserted a new camera scene.");
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
	if (m_bLookAtDummyEnabled && !Sync_DummyFromSelectedScene(cue))
	{
		m_bLookAtDummyEnabled = false;
		Reset_LookAtDummy();
	}
	Mark_Dirty("Deleted the selected interior camera key.");
	return true;
}

bool_t Client::CCameraTool::Create_Cut()
{
	if (!m_bLoaded || !m_Encounter.Is_Ready())
	{
		m_strStatus = "Reload the camera document before creating a cut.";
		return false;
	}
	const std::string name(m_szNewCutName);
	if (!Is_ValidCutName(name))
	{
		m_strStatus =
			"Cut name must be 1-128 characters of a-z A-Z 0-9 . _ -";
		return false;
	}
	if (nullptr != Find_DraftCue(name))
	{
		m_strStatus = "Cut name is already used: " + name;
		return false;
	}

	VALTAN_CINEMATIC_CAMERA_CUE cue;
	cue.strCueId = name;
	if (m_bNewCutTargetsDeath)
	{
		if (m_hasDraftDeathCue)
		{
			m_strStatus =
				"A death cut already exists: " + m_DraftDeathCue.strCueId;
			return false;
		}
		cue.iDurationMs = DEFAULT_DEATH_CUT_DURATION_MS;
	}
	else
	{
		const std::vector<ENCOUNTER_PATTERN_REFERENCE>& patterns =
			m_Encounter.Get_Patterns();
		if (patterns.empty() || m_iNewCutPatternIndex < 0 ||
			m_iNewCutPatternIndex >= static_cast<int32_t>(patterns.size()))
		{
			m_strStatus = "Select a valid pattern for the new cut.";
			return false;
		}
		if (m_DraftCues.size() >= MAX_DOCUMENT_CUE_COUNT)
		{
			m_strStatus =
				"The document already holds the maximum 32 pattern cuts.";
			return false;
		}
		const ENCOUNTER_PATTERN_REFERENCE& pattern =
			patterns[static_cast<size_t>(m_iNewCutPatternIndex)];
		if (pattern.stages.empty() || m_iNewCutStageIndex < 0 ||
			m_iNewCutStageIndex >= static_cast<int32_t>(pattern.stages.size()))
		{
			m_strStatus = "Select a valid stage for the new cut.";
			return false;
		}
		const ENCOUNTER_STAGE_REFERENCE& stage =
			pattern.stages[static_cast<size_t>(m_iNewCutStageIndex)];
		if (0u == stage.iDurationMs)
		{
			m_strStatus = "Selected stage has no authored duration.";
			return false;
		}
		const auto taken = std::find_if(
			m_DraftCues.begin(), m_DraftCues.end(),
			[&pattern, &stage](const VALTAN_CINEMATIC_CAMERA_CUE& existing)
			{
				return existing.strPatternId == pattern.patternId &&
					existing.strStageId == stage.stageId;
			});
		if (m_DraftCues.end() != taken)
		{
			m_strStatus = "Stage already owns a cut: " + taken->strCueId;
			return false;
		}
		cue.strPatternId = pattern.patternId;
		cue.strStageId = stage.stageId;
		cue.strStageActionId = stage.actionId;
		cue.iStageIndex = static_cast<uint32_t>(m_iNewCutStageIndex);
		cue.iDurationMs = stage.iDurationMs;
	}

	/* Seed the two mandatory endpoint scenes from the current camera when a
	   pipeline pose exists; the static fallback stays a valid authoring pose. */
	VALTAN_CINEMATIC_CAMERA_KEYFRAME first{};
	first.vEye = float3_t(0.f, 10.f, -10.f);
	first.vLookAt = float3_t(0.f, 0.f, 0.f);
	first.fFovYDegrees = 60.f;
	(void)Capture_CurrentPose(cue, first);
	VALTAN_CINEMATIC_CAMERA_KEYFRAME last = first;
	first.iTimeMs = 0u;
	last.iTimeMs = cue.iDurationMs;

	/* Register the cut before generating scene IDs so each generated ID sees
	   the previously committed one and stays globally unique. */
	if (m_bNewCutTargetsDeath)
	{
		m_DraftDeathCue = std::move(cue);
		m_hasDraftDeathCue = true;
	}
	else
	{
		m_DraftCues.push_back(std::move(cue));
	}
	VALTAN_CINEMATIC_CAMERA_CUE* draft = Find_DraftCue(name);
	first.strSceneId = Make_UniqueSceneId(*draft);
	draft->Keyframes.push_back(first);
	last.strSceneId = Make_UniqueSceneId(*draft);
	if (first.strSceneId.empty() || last.strSceneId.empty())
	{
		if (m_bNewCutTargetsDeath)
		{
			m_hasDraftDeathCue = false;
			m_DraftDeathCue = VALTAN_CINEMATIC_CAMERA_CUE{};
		}
		else
		{
			m_DraftCues.pop_back();
		}
		m_strStatus = "No stable scene ID is available for the new cut.";
		return false;
	}
	draft->Keyframes.push_back(last);

	m_strSelectedCueId.clear();
	Select_Cue(name);
	m_szNewCutName[0] = '\0';
	Mark_Dirty("Created a new camera cut draft. Save commits it.");
	return true;
}

bool_t Client::CCameraTool::Delete_SelectedCut()
{
	VALTAN_CINEMATIC_CAMERA_CUE* cue = Get_SelectedCue();
	if (nullptr == cue)
	{
		m_strStatus = "Select a camera cut to delete.";
		return false;
	}
	if (Is_DeathCueSelected())
	{
		m_hasDraftDeathCue = false;
		m_DraftDeathCue = VALTAN_CINEMATIC_CAMERA_CUE{};
	}
	else
	{
		if (m_DraftCues.size() <= 1u)
		{
			m_strStatus =
				"The document must keep at least one pattern cut.";
			return false;
		}
		const std::string cueId = m_strSelectedCueId;
		m_DraftCues.erase(std::find_if(
			m_DraftCues.begin(), m_DraftCues.end(),
			[&cueId](const VALTAN_CINEMATIC_CAMERA_CUE& candidate)
			{ return candidate.strCueId == cueId; }));
	}
	Stop_Preview(true);
	m_strSelectedCueId.clear();
	m_iSelectedKeyframe = -1;
	if (m_bLookAtDummyEnabled)
		Disable_LookAtDummy(nullptr);
	if (!m_DraftCues.empty())
		Select_Cue(m_DraftCues.front().strCueId);
	else if (m_hasDraftDeathCue)
		Select_Cue(m_DraftDeathCue.strCueId);
	Mark_Dirty("Deleted the selected camera cut draft. Save commits the removal.");
	return true;
}

void Client::CCameraTool::Respace_PosTimes(
	VALTAN_CINEMATIC_CAMERA_CUE& cue)
{
	const size_t count = cue.Keyframes.size();
	if (count < 2u)
		return;
	for (size_t index = 0u; index < count; ++index)
	{
		cue.Keyframes[index].iTimeMs = static_cast<uint32_t>(
			(static_cast<uint64_t>(cue.iDurationMs) * index) / (count - 1u));
	}
}

bool_t Client::CCameraTool::Append_CapturedPos(
	VALTAN_CINEMATIC_CAMERA_CUE& cue)
{
	if (cue.Keyframes.size() < 2u)
	{
		m_strStatus = "The cut is missing its two endpoint scenes.";
		return false;
	}
	Stop_Preview(false);
	VALTAN_CINEMATIC_CAMERA_KEYFRAME captured = cue.Keyframes.back();
	if (!Capture_CurrentPose(cue, captured))
	{
		m_strStatus =
			"Pos capture failed: pipeline pose or replicated tracking frame is unavailable.";
		return false;
	}
	/* A fresh cut seeds two identical endpoint scenes. The first capture then
	   becomes the path's end instead of a third duplicate entry; every later
	   capture appends and the whole path is respaced evenly, so capture the
	   positions first and tune per-segment speeds afterwards. */
	const VALTAN_CINEMATIC_CAMERA_KEYFRAME& first = cue.Keyframes.front();
	VALTAN_CINEMATIC_CAMERA_KEYFRAME& last = cue.Keyframes.back();
	const bool_t pristinePair = 2u == cue.Keyframes.size() &&
		Distance(first.vEye, last.vEye) <= 0.0001f &&
		Distance(first.vLookAt, last.vLookAt) <= 0.0001f &&
		std::abs(first.fFovYDegrees - last.fFovYDegrees) <= 0.0001f;
	if (pristinePair)
	{
		last.vEye = captured.vEye;
		last.vLookAt = captured.vLookAt;
		last.fFovYDegrees = captured.fFovYDegrees;
		m_iSelectedKeyframe = 1;
	}
	else
	{
		if (cue.Keyframes.size() >= MAX_DOCUMENT_KEYFRAME_COUNT)
		{
			m_strStatus = "The cut already holds the maximum 64 positions.";
			return false;
		}
		if (cue.iDurationMs < cue.Keyframes.size())
		{
			m_strStatus =
				"Cue duration is too short to keep every pos one millisecond apart.";
			return false;
		}
		captured.strSceneId = Make_UniqueSceneId(cue);
		if (captured.strSceneId.empty())
		{
			m_strStatus = "No stable scene ID is available for another pos.";
			return false;
		}
		cue.Keyframes.push_back(captured);
		Respace_PosTimes(cue);
		m_iSelectedKeyframe = static_cast<int32_t>(cue.Keyframes.size()) - 1;
	}
	m_fPreviewSeconds = static_cast<f32_t>(cue.Keyframes[
		static_cast<size_t>(m_iSelectedKeyframe)].iTimeMs) * 0.001f;
	if (m_bLookAtDummyEnabled && !Sync_DummyFromSelectedScene(cue))
	{
		m_bLookAtDummyEnabled = false;
		Reset_LookAtDummy();
	}
	Mark_Dirty("Captured the current camera into the pos list.");
	return true;
}

bool_t Client::CCameraTool::Delete_SelectedPos(
	VALTAN_CINEMATIC_CAMERA_CUE& cue)
{
	if (m_iSelectedKeyframe < 0 ||
		m_iSelectedKeyframe >= static_cast<int32_t>(cue.Keyframes.size()))
	{
		m_strStatus = "Select a pos to delete.";
		return false;
	}
	if (cue.Keyframes.size() <= 2u)
	{
		m_strStatus = "A cut keeps at least its start and end pos.";
		return false;
	}
	cue.Keyframes.erase(cue.Keyframes.begin() + m_iSelectedKeyframe);
	Respace_PosTimes(cue);
	m_iSelectedKeyframe = (std::min)(
		m_iSelectedKeyframe,
		static_cast<int32_t>(cue.Keyframes.size()) - 1);
	m_fPreviewSeconds = static_cast<f32_t>(cue.Keyframes[
		static_cast<size_t>(m_iSelectedKeyframe)].iTimeMs) * 0.001f;
	if (m_bLookAtDummyEnabled && !Sync_DummyFromSelectedScene(cue))
	{
		m_bLookAtDummyEnabled = false;
		Reset_LookAtDummy();
	}
	Mark_Dirty("Deleted the selected pos and respaced the path.");
	return true;
}
