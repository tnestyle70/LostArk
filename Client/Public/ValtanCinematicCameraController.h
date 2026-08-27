#pragma once

#include "Client_Defines.h"
#include "ValtanCinematicCameraDocument.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

NS_BEGIN(Client)

struct VALTAN_CINEMATIC_CAMERA_INPUT final
{
	bool_t isValid = false;
	uint64_t iNetEntityId = 0u;
	uint32_t iServerTick = 0u;
	std::string strPatternId;
	std::string strStageId;
	std::string strStageActionId;
	uint32_t iPatternSequence = 0u;
	uint32_t iStageIndex = 0u;
	uint32_t iActionStartTick = 0u;
	/* Last committed Server snapshot pose. A tracking cue translates only from
	   this replicated value and never reads a Client GameObject transform. */
	float3_t vBossPosition = {};
	f32_t fBossYawDegrees = 0.f;
	/* PLAYER_BOSS_FRAME is the one intentional local presentation input. It is
	   sampled by Level_ValtanArena from the replicated local Character and only
	   affects the camera pose; Server targeting and boss motion remain authoritative. */
	bool_t hasLocalPlayerPosition = false;
	float3_t vLocalPlayerPosition = {};
	/* Death has no pattern to key on, so the clear shot is selected by this flag
	   and still runs off the authoritative action start tick. */
	bool_t isBossDead = false;
};

struct VALTAN_CINEMATIC_CAMERA_POSE final
{
	float3_t vEye = {};
	float3_t vLookAt = {};
	f32_t fFovYDegrees = 60.f;
};

class CValtanCinematicCameraController final
{
public:
	/* Camera Tool preview and product playback must not grow separate easing
	   implementations. This samples the authored base pose on the cue clock;
	   product tracking is applied afterwards from replicated boss state. */
	static bool_t Sample_Cue(
		const VALTAN_CINEMATIC_CAMERA_CUE& cue,
		f32_t elapsedSeconds,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose);
	/* Camera Tool actor-tracking preview and product playback share this exact
	   coordinate-frame projection. The caller must first Sample_Cue so no
	   second authored-keyframe or tracking implementation can diverge. */
	static bool_t Apply_CueTracking(
		const VALTAN_CINEMATIC_CAMERA_CUE& cue,
		const VALTAN_CINEMATIC_CAMERA_INPUT& input,
		VALTAN_CINEMATIC_CAMERA_POSE& inOutPose);
	/* Entry and gameplay-handoff blends share one bounded sampler. Keeping this
	   independent of camera ownership lets the Level retain the override until
	   it has submitted the exact final follow pose. */
	static bool_t Sample_BoundedTransition(
		const VALTAN_CINEMATIC_CAMERA_POSE& fromPose,
		const VALTAN_CINEMATIC_CAMERA_POSE& toPose,
		uint32_t durationMs,
		f32_t elapsedSeconds,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose);

	bool_t Initialize(
		const CValtanCinematicCameraDocument* document,
		uint32_t fixedTickHz);
	bool_t Update(
		const VALTAN_CINEMATIC_CAMERA_INPUT& input,
		f32_t timeDelta,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose);
	bool_t Update_ExitTransition(
		const VALTAN_CINEMATIC_CAMERA_POSE& followPose,
		f32_t timeDelta,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose);
	void Cancel_ExitTransition();
	void Reset();

	bool_t Is_Active() const { return nullptr != m_pActiveCue; }
	bool_t Is_ExitTransitionActive() const
	{
		return m_isExitTransitionActive;
	}
	f32_t Get_ElapsedSeconds() const { return m_fElapsedSeconds; }
private:
	bool_t Sample_ActiveCue(
		f32_t elapsedSeconds,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose) const;
	bool_t Apply_Tracking(
		const VALTAN_CINEMATIC_CAMERA_INPUT& input,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose) const;
	void Apply_CueTransition(
		f32_t elapsedSeconds,
		VALTAN_CINEMATIC_CAMERA_POSE& inOutPose);
	void Begin_ExitTransition(
		const VALTAN_CINEMATIC_CAMERA_CUE& outgoingCue);

private:
	const CValtanCinematicCameraDocument* m_pDocument = nullptr;
	const VALTAN_CINEMATIC_CAMERA_CUE* m_pActiveCue = nullptr;
	uint32_t m_iFixedTickHz = 0u;
	uint64_t m_iNetEntityId = 0u;
	uint32_t m_iPatternSequence = 0u;
	uint32_t m_iStageIndex = 0u;
	uint32_t m_iActionStartTick = 0u;
	uint32_t m_iLastServerTick = 0u;
	f32_t m_fElapsedSeconds = 0.f;
	std::string m_strCueId;
	bool_t m_hasCueKey = false;
	bool_t m_isCueFinished = false;
	/* Stored after tracking and any active blend, so a coordinate-frame change
	   starts from the pose that was actually submitted on the prior frame. */
	VALTAN_CINEMATIC_CAMERA_POSE m_LastOutputPose;
	VALTAN_CINEMATIC_CAMERA_POSE m_TransitionFromPose;
	bool_t m_hasLastOutputPose = false;
	bool_t m_isTransitionActive = false;
	VALTAN_CINEMATIC_CAMERA_POSE m_ExitTransitionFromPose;
	uint32_t m_iExitTransitionMs = 0u;
	f32_t m_fExitTransitionElapsedSeconds = 0.f;
	bool_t m_isExitTransitionActive = false;
};

NS_END
