#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

class CEncounterPatternReference;

struct VALTAN_CINEMATIC_CAMERA_KEYFRAME final
{
	uint32_t iTimeMs = 0u;
	float3_t vEye = {};
	float3_t vLookAt = {};
	f32_t fFovYDegrees = 60.f;
};

/* Linear alone reads as a dolly running at constant speed. Smoothstep eases
   the ends so a stage can settle, and hold keeps a framing locked while the
   arena collapses in front of it. */
enum class VALTAN_CINEMATIC_CAMERA_EASING
{
	LINEAR,
	SMOOTHSTEP,
	HOLD,
	END
};

/* WORLD preserves an authored absolute shot. BOSS_XZ translates the authored
   framing by the replicated boss' horizontal displacement from vTrackingOrigin;
   Y remains authored so an aerial shot does not climb twice with the leap. */
enum class VALTAN_CINEMATIC_TRACKING_MODE
{
	WORLD,
	BOSS_XZ,
	END
};

struct VALTAN_CINEMATIC_CAMERA_CUE final
{
	std::string strCueId;
	std::string strPatternId;
	std::string strStageId;
	std::string strStageActionId;
	uint32_t iStageIndex = 0u;
	uint32_t iDurationMs = 0u;
	VALTAN_CINEMATIC_CAMERA_EASING eEasing =
		VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
	VALTAN_CINEMATIC_TRACKING_MODE eTrackingMode =
		VALTAN_CINEMATIC_TRACKING_MODE::WORLD;
	float3_t vTrackingOrigin = {};
	/* Deterministic landing jolt. Amplitude is in world units and decays to
	   nothing across its own duration, so the shake can never outlive the cue
	   or diverge between clients. */
	f32_t fShakeAmplitude = 0.f;
	uint32_t iShakeDurationMs = 0u;
	std::vector<VALTAN_CINEMATIC_CAMERA_KEYFRAME> Keyframes;
};

/* The 100-bar sky is a timed presentation layer, never a skybox swap and never
   a gameplay state: it turns on and off across the four authoritative mechanic
   stages and touches no collision or navigation. The two stable asset IDs seed
   the red-cloud and black-aperture presentation composites. */
struct VALTAN_CINEMATIC_SKY_CUE final
{
	std::string strCueId;
	std::string strPatternId;
	std::string strStageId;
	std::string strStageActionId;
	uint32_t iStageIndex = 0u;
	uint32_t iStageLocalStartMs = 0u;
	uint32_t iStageLocalEndMs = 0u;
	std::string strRedCloudAssetId;
	std::string strBlackApertureAssetId;
	f32_t fCloudOpacityStart = 0.f;
	f32_t fCloudOpacityEnd = 0.f;
	f32_t fApertureScaleStart = 0.f;
	f32_t fApertureScaleEnd = 0.f;
	f32_t fCloudRotationDegreesPerSecond = 0.f;
	VALTAN_CINEMATIC_TRACKING_MODE eTrackingMode =
		VALTAN_CINEMATIC_TRACKING_MODE::WORLD;
	float3_t vTrackingOrigin = {};
};

class CValtanCinematicCameraDocument final
{
public:
	bool_t Load(
		const std::filesystem::path& path,
		const CEncounterPatternReference& encounter,
		std::string& outStatus);
	static bool_t Parse_Text(
		std::string_view text,
		const CEncounterPatternReference& encounter,
		CValtanCinematicCameraDocument& outDocument,
		std::string& outStatus);
	void Clear();

	bool_t Is_Ready() const { return m_isReady; }
	const std::string& Get_EncounterId() const { return m_strEncounterId; }
	const std::vector<VALTAN_CINEMATIC_CAMERA_CUE>& Get_Cues() const
	{
		return m_Cues;
	}
	const VALTAN_CINEMATIC_CAMERA_CUE* Find_Cue(
		std::string_view patternId,
		uint32_t stageIndex,
		std::string_view stageActionId) const;
	const std::vector<VALTAN_CINEMATIC_SKY_CUE>& Get_SkyCues() const
	{
		return m_SkyCues;
	}
	const VALTAN_CINEMATIC_SKY_CUE* Find_SkyCue(
		std::string_view patternId,
		uint32_t stageIndex,
		std::string_view stageActionId) const;
	/* The clear shot has no pattern to key on, so it is looked up by the boss
	   death action instead. Null when the encounter authors none. */
	const VALTAN_CINEMATIC_CAMERA_CUE* Find_DeathCue() const
	{
		return m_hasDeathCue ? &m_DeathCue : nullptr;
	}

private:
	std::string m_strEncounterId;
	std::vector<VALTAN_CINEMATIC_CAMERA_CUE> m_Cues;
	std::vector<VALTAN_CINEMATIC_SKY_CUE> m_SkyCues;
	VALTAN_CINEMATIC_CAMERA_CUE m_DeathCue;
	bool_t m_hasDeathCue = false;
	bool_t m_isReady = false;
};

NS_END
