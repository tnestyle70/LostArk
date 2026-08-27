#pragma once

#include "Client_Defines.h"
#include "EncounterPatternReference.h"
#include "ValtanCinematicCameraDocument.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CCamera_Free;

struct CAMERA_TOOL_OPEN_REQUEST final
{
	std::string strCueId;
};

/* Level-owned, read-only actor sample for Debug preview. Product playback and
   gameplay authority stay in Level_ValtanArena; the Tool receives only the
   same replicated presentation inputs needed by the public camera sampler. */
struct CAMERA_TOOL_ACTOR_PREVIEW_CONTEXT final
{
	bool_t isValid = false;
	uint32_t iLevelIndex = 0u;
	float3_t vBossPosition = {};
	f32_t fBossYawDegrees = 0.f;
	bool_t hasLocalPlayerPosition = false;
	float3_t vLocalPlayerPosition = {};
};

/* Debug authoring session for the exact product cinematic-camera document.
   It owns draft/UI/preview state, but never owns encounter gameplay or a
   second camera playback format. */
class CCameraTool final
{
public:
	CCameraTool() = default;
	~CCameraTool();

	void Open();
	bool_t Open_Cue(const CAMERA_TOOL_OPEN_REQUEST& request);
	void Update(f32_t timeDelta, bool_t toolVisible);
	void Render();
	void Deactivate();
	void On_LevelChanged();
	static void Publish_ActorPreviewContext(
		const CAMERA_TOOL_ACTOR_PREVIEW_CONTEXT& context);
	static void Clear_ActorPreviewContext(uint32_t levelIndex);

private:
	bool_t Reload();
	bool_t Validate_Draft(
		CValtanCinematicCameraDocument& outDocument,
		std::string& outText,
		std::string& outStatus) const;
	bool_t Save();
	void Commit_LoadedDocument(
		CEncounterPatternReference encounter,
		CValtanCinematicCameraDocument document,
		std::string sourceText,
		const std::string& preferredCueId);

	VALTAN_CINEMATIC_CAMERA_CUE* Find_DraftCue(
		const std::string& cueId);
	const VALTAN_CINEMATIC_CAMERA_CUE* Find_DraftCue(
		const std::string& cueId) const;
	VALTAN_CINEMATIC_CAMERA_CUE* Get_SelectedCue();
	const VALTAN_CINEMATIC_CAMERA_CUE* Get_SelectedCue() const;
	bool_t Is_DeathCueSelected() const;
	void Select_Cue(const std::string& cueId);

	std::shared_ptr<CCamera_Free> Find_CurrentCamera() const;
	bool_t Capture_CurrentPose(
		VALTAN_CINEMATIC_CAMERA_KEYFRAME& outKeyframe) const;
	bool_t Acquire_PreviewCamera();
	bool_t Apply_PreviewPose();
	void Stop_Preview(bool_t resetCursor);
	void Handle_PreviewPreemption();

	void Render_CueList();
	void Render_CueEditor();
	void Render_KeyframeEditor(VALTAN_CINEMATIC_CAMERA_CUE& cue);
	void Render_PreviewControls(VALTAN_CINEMATIC_CAMERA_CUE& cue);
	void Mark_Dirty(const char_t* status);
	bool_t Insert_Keyframe(VALTAN_CINEMATIC_CAMERA_CUE& cue);
	bool_t Delete_SelectedKeyframe(VALTAN_CINEMATIC_CAMERA_CUE& cue);

private:
	static constexpr uint64_t PREVIEW_OWNER_ID = 0x43414D455241544Cull;

	CEncounterPatternReference m_Encounter;
	CValtanCinematicCameraDocument m_LoadedDocument;
	CValtanCinematicCameraDocument m_PreviewDocument;
	std::vector<VALTAN_CINEMATIC_CAMERA_CUE> m_DraftCues;
	VALTAN_CINEMATIC_CAMERA_CUE m_DraftDeathCue;
	bool_t m_hasDraftDeathCue = false;

	std::filesystem::path m_CameraPath;
	std::string m_strBaselineText;
	std::string m_strSelectedCueId;
	std::string m_strPendingOpenCueId;
	std::string m_strStatus = "Reload the Valtan cinematic camera document.";
	int32_t m_iSelectedKeyframe = -1;
	f32_t m_fPreviewSeconds = 0.f;
	std::weak_ptr<CCamera_Free> m_pPreviewCamera;
	bool_t m_bLoaded = false;
	bool_t m_bDirty = false;
	bool_t m_bOpen = false;
	bool_t m_bFocusPending = false;
	bool_t m_bPreviewOwned = false;
	bool_t m_bPreviewDraftStale = true;
	bool_t m_bPlaying = false;
};

NS_END
