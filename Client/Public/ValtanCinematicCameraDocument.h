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
	std::string strSceneId;
	uint32_t iTimeMs = 0u;
	float3_t vEye = {};
	float3_t vLookAt = {};
	f32_t fFovYDegrees = 60.f;
};

/* LINEAR preserves the existing authored camera path. CATMULL_ROM connects
   the same saved scenes with a smooth product-runtime spline; the sampler
   clamps endpoint control points so no hidden scene is required. */
enum class VALTAN_CINEMATIC_CAMERA_INTERPOLATION
{
	LINEAR,
	CATMULL_ROM,
	END
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
   Y remains authored so an aerial shot does not climb twice with the leap.
   BOSS_FACING treats eye/lookAt as points in the authored boss frame whose
   pivot is vTrackingOrigin, then rotates that frame by the replicated boss yaw.
   PLAYER_BOSS_FRAME treats them as offsets in a dynamic frame centred between
   the local player and boss; this is presentation-only framing and never feeds
   a Client transform back into gameplay. */
enum class VALTAN_CINEMATIC_TRACKING_MODE
{
	WORLD,
	BOSS_XZ,
	BOSS_FACING,
	PLAYER_BOSS_FRAME,
	END
};

struct VALTAN_CINEMATIC_CAMERA_CUE final
{
	static constexpr uint32_t MAX_TRANSITION_IN_MS = 1000u;

	std::string strCueId;
	std::string strPatternId;
	std::string strStageId;
	std::string strStageActionId;
	uint32_t iStageIndex = 0u;
	uint32_t iDurationMs = 0u;
	/* Optional entry blend from the last resolved output of the immediately
	   preceding cue in the same Server pattern sequence. A late join has no
	   outgoing pose and therefore samples this cue directly. */
	uint32_t iTransitionInMs = 0u;
	/* Optional handoff from this cue's last submitted pose to the normal
	   gameplay follow pose. The presentation owner stays acquired until the
	   bounded handoff reaches that live follow target. */
	uint32_t iTransitionOutMs = 0u;
	VALTAN_CINEMATIC_CAMERA_INTERPOLATION eInterpolation =
		VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR;
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
	/* Authoring and runtime share this exact camera-only v6 document. The Tool
	   stages the camera/death rows into a copy and reparses the serialized text
	   through Parse_Text before commit. World Effects are authored separately. */
	bool_t Stage_CameraDraft(
		const std::vector<VALTAN_CINEMATIC_CAMERA_CUE>& cues,
		bool_t hasDeathCue,
		const VALTAN_CINEMATIC_CAMERA_CUE& deathCue,
		const CEncounterPatternReference& encounter,
		CValtanCinematicCameraDocument& outDocument,
		std::string& outText,
		std::string& outStatus) const;
	bool_t Serialize_Text(
		std::string& outText,
		std::string& outStatus) const;
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
	/* The clear shot has no pattern to key on, so it is looked up by the boss
	   death action instead. Null when the encounter authors none. */
	const VALTAN_CINEMATIC_CAMERA_CUE* Find_DeathCue() const
	{
		return m_hasDeathCue ? &m_DeathCue : nullptr;
	}
	bool_t Has_DeathCue() const { return m_hasDeathCue; }

private:
	std::string m_strEncounterId;
	std::vector<VALTAN_CINEMATIC_CAMERA_CUE> m_Cues;
	VALTAN_CINEMATIC_CAMERA_CUE m_DeathCue;
	bool_t m_hasDeathCue = false;
	bool_t m_isReady = false;
};

NS_END
