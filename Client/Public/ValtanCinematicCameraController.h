#pragma once

#include "Client_Defines.h"
#include "ValtanCinematicCameraDocument.h"

#include <array>
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
	float3_t vAnchor = {};
};

/* A renderer-independent policy for the six authored Valtan proxy planes.
   Level_ValtanArena and the focused harness consume this same table, so a raw
   square layer cannot become visible through a second, untested mapping. */
enum class VALTAN_CINEMATIC_SKY_LAYER_PROFILE : uint32_t
{
	NONE = 0u,
	DARK_APERTURE = 1u,
	RED_RING = 2u,
	RED_CLOUD_DISC = 3u,
	END
};

struct VALTAN_CINEMATIC_SKY_LAYER_POLICY final
{
	std::string_view strAssetId;
	VALTAN_CINEMATIC_SKY_LAYER_PROFILE eProfile =
		VALTAN_CINEMATIC_SKY_LAYER_PROFILE::NONE;
	bool_t isPresentationVisible = false;
	f32_t fAbsoluteUniformScale = 1.f;
	f32_t fOpacityMultiplier = 1.f;
	f32_t fRotationMultiplier = 0.f;
	bool_t scalesWithAperture = false;
};

struct VALTAN_CINEMATIC_SKY_PRESENTATION_FRAME final
{
	float3_t vAnchor = {};
	float4_t vFacingQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
};

class CValtanCinematicCameraController final
{
public:
	static constexpr size_t SKY_LAYER_POLICY_COUNT = 6u;
	static constexpr f32_t SKY_MIN_ABSOLUTE_UNIFORM_SCALE = 0.5f;
	static constexpr f32_t SKY_MAX_ABSOLUTE_UNIFORM_SCALE = 3.f;

	static const std::array<VALTAN_CINEMATIC_SKY_LAYER_POLICY,
		SKY_LAYER_POLICY_COUNT>& Get_SkyLayerPolicies();
	static const VALTAN_CINEMATIC_SKY_LAYER_POLICY* Find_SkyLayerPolicy(
		std::string_view assetId);
	static const VALTAN_CINEMATIC_SKY_PRESENTATION_FRAME&
		Get_SkyPresentationFrame();

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
	/* Resolved from the same input the camera consumes. A new Server tick seeks
	   its authoritative age, while duplicate snapshots advance by a bounded
	   local delta so rotation and fades remain smooth between fixed ticks. */
	VALTAN_CINEMATIC_SKY_STATE Resolve_SkyState(
		const VALTAN_CINEMATIC_CAMERA_INPUT& input,
		f32_t timeDelta);
	const VALTAN_CINEMATIC_SKY_STATE& Get_LastSkyState() const
	{
		return m_LastSkyState;
	}

private:
	bool_t Sample_ActiveCue(
		f32_t elapsedSeconds,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose) const;
	bool_t Apply_Tracking(
		const VALTAN_CINEMATIC_CAMERA_INPUT& input,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose) const;
	void Apply_ImpactShake(
		f32_t elapsedSeconds,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose) const;
	void Reset_SkyTimeline();

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
	uint64_t m_iSkyNetEntityId = 0u;
	uint32_t m_iSkyPatternSequence = 0u;
	uint32_t m_iSkyStageIndex = 0u;
	uint32_t m_iSkyActionStartTick = 0u;
	uint32_t m_iSkyLastServerTick = 0u;
	f32_t m_fSkyElapsedSeconds = 0.f;
	std::string m_strSkyCueId;
	bool_t m_hasSkyCueKey = false;
	VALTAN_CINEMATIC_SKY_STATE m_LastSkyState;
};

NS_END
