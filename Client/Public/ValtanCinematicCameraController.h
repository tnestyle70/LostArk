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
};

NS_END
