#pragma once

#include "Client_Defines.h"
#include "EncounterPatternReference.h"
#include "ValtanCinematicCameraDocument.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Engine
{
	class CCollider;
}

NS_BEGIN(Client)

class CCamera_Free;
struct VALTAN_CINEMATIC_CAMERA_INPUT;
struct VALTAN_CINEMATIC_CAMERA_POSE;

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
		const VALTAN_CINEMATIC_CAMERA_CUE& cue,
		VALTAN_CINEMATIC_CAMERA_KEYFRAME& outKeyframe) const;
	bool_t Build_TrackingInput(
		const VALTAN_CINEMATIC_CAMERA_CUE& cue,
		VALTAN_CINEMATIC_CAMERA_INPUT& outInput) const;
	bool_t Resolve_SceneWorldPose(
		const VALTAN_CINEMATIC_CAMERA_CUE& cue,
		const VALTAN_CINEMATIC_CAMERA_KEYFRAME& scene,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose) const;
	bool_t Acquire_PreviewCamera();
	bool_t Apply_PreviewPose();
	void Stop_Preview(bool_t resetCursor);
	void Handle_PreviewPreemption();

	void Render_CueList();
	void Render_CutManagement();
	void Render_CueEditor();
	void Render_KeyframeEditor(VALTAN_CINEMATIC_CAMERA_CUE& cue);
	void Render_LookAtDummyEditor(VALTAN_CINEMATIC_CAMERA_CUE& cue);
	void Render_PreviewControls(VALTAN_CINEMATIC_CAMERA_CUE& cue);
	void Mark_Dirty(const char_t* status);
	bool_t Insert_Keyframe(VALTAN_CINEMATIC_CAMERA_CUE& cue);
	bool_t Insert_CapturedScene(VALTAN_CINEMATIC_CAMERA_CUE& cue);
	bool_t Delete_SelectedKeyframe(VALTAN_CINEMATIC_CAMERA_CUE& cue);
	bool_t Create_Cut();
	bool_t Delete_SelectedCut();
	std::string Make_UniqueSceneId(
		const VALTAN_CINEMATIC_CAMERA_CUE& cue) const;
	f32_t Calculate_SegmentArcLength(
		const VALTAN_CINEMATIC_CAMERA_CUE& cue,
		size_t rightSceneIndex) const;
	bool_t Apply_SelectedSegmentSpeed(
		VALTAN_CINEMATIC_CAMERA_CUE& cue,
		f32_t targetSpeed);
	uint32_t Get_MaximumCueDuration(
		const VALTAN_CINEMATIC_CAMERA_CUE& cue) const;
	bool_t Ensure_LookAtDummy();
	void Update_LookAtDummy();
	void Reset_LookAtDummy();
	void Disable_LookAtDummy(const char_t* status);
	bool_t Sync_DummyFromSelectedScene(
		const VALTAN_CINEMATIC_CAMERA_CUE& cue);
	bool_t Apply_DummyToSelectedScene(
		VALTAN_CINEMATIC_CAMERA_CUE& cue);

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
	std::string m_strCompletePlayStatus =
		"Complete Play uses the workspace's selected saved Server pattern.";
	int32_t m_iSelectedKeyframe = -1;
	f32_t m_fPreviewSeconds = 0.f;
	f32_t m_fPreviewRate = 1.f;
	f32_t m_fTargetSegmentSpeed = 5.f;
	/* New-cut authoring inputs. The typed name becomes the stable cueId and the
	   pattern/stage selection satisfies the document's one-cue-per-stage rule. */
	char_t m_szNewCutName[129] = {};
	int32_t m_iNewCutPatternIndex = 0;
	int32_t m_iNewCutStageIndex = 0;
	bool_t m_bNewCutTargetsDeath = false;
	float3_t m_vLookAtDummyWorld = {};
	f32_t m_fLookAtDummyRadius = 0.35f;
	std::shared_ptr<Engine::CCollider> m_pLookAtDummyCollider;
	uint32_t m_iLookAtDummyLevel = static_cast<uint32_t>(-1);
	std::weak_ptr<CCamera_Free> m_pPreviewCamera;
	bool_t m_bLoaded = false;
	bool_t m_bDirty = false;
	bool_t m_bOpen = false;
	bool_t m_bFocusPending = false;
	bool_t m_bPreviewOwned = false;
	bool_t m_bPreviewDraftStale = true;
	bool_t m_bPlaying = false;
	bool_t m_bLookAtDummyEnabled = false;
};

NS_END
