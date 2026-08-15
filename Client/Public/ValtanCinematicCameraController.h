#pragma once

#include "Client_Defines.h"
#include "ValtanCinematicCameraDocument.h"

#include <cstdint>
#include <string>

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

/* Presentation-only sky state resolved from the same authoritative tuple as the
   camera, so there is one time axis. It never touches collision or navigation,
   and it is inactive whenever no authored window covers the current action age. */
struct VALTAN_CINEMATIC_SKY_STATE final
{
	bool_t isActive = false;
	std::string strCueId;
	std::string strRedCloudAssetId;
	std::string strBlackApertureAssetId;
	f32_t fCloudOpacity = 0.f;
	f32_t fApertureScale = 0.f;
	f32_t fCloudRotationDegrees = 0.f;
};

class CValtanCinematicCameraController final
{
public:
	bool_t Initialize(
		const CValtanCinematicCameraDocument* document,
		uint32_t fixedTickHz);
	bool_t Update(
		const VALTAN_CINEMATIC_CAMERA_INPUT& input,
		f32_t timeDelta,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose);
	void Reset();

	bool_t Is_Active() const { return nullptr != m_pActiveCue; }
	f32_t Get_ElapsedSeconds() const { return m_fElapsedSeconds; }
	/* Resolved from the same input the camera consumes. Safe to call every frame
	   and returns an inactive state when no window covers the current age. */
	VALTAN_CINEMATIC_SKY_STATE Resolve_SkyState(
		const VALTAN_CINEMATIC_CAMERA_INPUT& input) const;
	const VALTAN_CINEMATIC_SKY_STATE& Get_LastSkyState() const
	{
		return m_LastSkyState;
	}

private:
	bool_t Sample_ActiveCue(
		f32_t elapsedSeconds,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose) const;
	void Apply_ImpactShake(
		f32_t elapsedSeconds,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose) const;

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
	mutable VALTAN_CINEMATIC_SKY_STATE m_LastSkyState;
};

NS_END
