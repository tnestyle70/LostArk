#include "ValtanCinematicCameraController.h"

#include "ActionPresentationTimeline.h"

#include <algorithm>
#include <cmath>

const std::array<Client::VALTAN_CINEMATIC_SKY_LAYER_POLICY,
	Client::CValtanCinematicCameraController::SKY_LAYER_POLICY_COUNT>&
Client::CValtanCinematicCameraController::Get_SkyLayerPolicies()
{
	/* Loader already applies the model's 0.01 pre-transform. These are bounded
	   absolute placement scales, never multipliers of the authored placement.
	   Electric and streak deliberately remain admitted-but-hidden source data. */
	static const std::array<VALTAN_CINEMATIC_SKY_LAYER_POLICY,
		SKY_LAYER_POLICY_COUNT> POLICIES = {{
		{ "VALTAN_PHASE_CHAOS_CLOUD",
			VALTAN_CINEMATIC_SKY_LAYER_PROFILE::RED_CLOUD_DISC,
			true, 2.75f, 0.68f, 1.f, false },
		{ "VALTAN_PHASE_CHAOS_ELECTRIC",
			VALTAN_CINEMATIC_SKY_LAYER_PROFILE::NONE,
			false, 1.f, 1.f, 0.f, false },
		{ "VALTAN_PHASE_CHAOS_RING",
			VALTAN_CINEMATIC_SKY_LAYER_PROFILE::RED_RING,
			true, 2.45f, 0.55f, 1.35f, false },
		{ "VALTAN_PHASE_SPACEHOLE_CLOUD",
			VALTAN_CINEMATIC_SKY_LAYER_PROFILE::RED_CLOUD_DISC,
			true, 2.25f, 0.42f, -0.45f, false },
		{ "VALTAN_PHASE_SPACEHOLE_CORE",
			VALTAN_CINEMATIC_SKY_LAYER_PROFILE::DARK_APERTURE,
			true, 1.90f, 1.f, 0.68f, true },
		{ "VALTAN_PHASE_SPACEHOLE_STREAK",
			VALTAN_CINEMATIC_SKY_LAYER_PROFILE::NONE,
			false, 1.f, 1.f, 0.f, false }
	}};
	return POLICIES;
}

const Client::VALTAN_CINEMATIC_SKY_LAYER_POLICY*
Client::CValtanCinematicCameraController::Find_SkyLayerPolicy(
	const std::string_view assetId)
{
	const auto& policies = Get_SkyLayerPolicies();
	const auto found = std::find_if(
		policies.begin(), policies.end(),
		[assetId](const VALTAN_CINEMATIC_SKY_LAYER_POLICY& policy)
		{
			return policy.strAssetId == assetId;
		});
	return policies.end() == found ? nullptr : &*found;
}

const Client::VALTAN_CINEMATIC_SKY_PRESENTATION_FRAME&
Client::CValtanCinematicCameraController::Get_SkyPresentationFrame()
{
	/* This point projects close to the centre for the authored TAKEOFF/DROP
	   poses. All visible planes share this exact anchor and face down toward the
	   cinematic camera, so their procedural radial masks remain concentric. */
	static const VALTAN_CINEMATIC_SKY_PRESENTATION_FRAME FRAME = {
		float3_t(156.03f, 74.f, -122.06f),
		float4_t(-1.f, 0.f, 0.f, 0.f)
	};
	return FRAME;
}

bool_t Client::CValtanCinematicCameraController::Initialize(
	const CValtanCinematicCameraDocument* document,
	const uint32_t fixedTickHz)
{
	Reset();
	if (nullptr == document || !document->Is_Ready() || 0u == fixedTickHz)
		return false;
	m_pDocument = document;
	m_iFixedTickHz = fixedTickHz;
	return true;
}

Client::VALTAN_CINEMATIC_SKY_STATE
Client::CValtanCinematicCameraController::Resolve_SkyState(
	const VALTAN_CINEMATIC_CAMERA_INPUT& input,
	const f32_t timeDelta)
{
	VALTAN_CINEMATIC_SKY_STATE state;
	m_LastSkyState = state;
	if (nullptr == m_pDocument || !input.isValid || 0u == m_iFixedTickHz ||
		0u == input.iNetEntityId || 0u == input.iServerTick ||
		0u == input.iPatternSequence || 0u == input.iActionStartTick ||
		!std::isfinite(timeDelta) || timeDelta < 0.f)
	{
		Reset_SkyTimeline();
		return state;
	}
	const VALTAN_CINEMATIC_SKY_CUE* cue = m_pDocument->Find_SkyCue(
		input.strPatternId, input.iStageIndex, input.strStageActionId);
	if (nullptr == cue || (!input.strStageId.empty() &&
		cue->strStageId != input.strStageId))
	{
		Reset_SkyTimeline();
		return state;
	}

	const bool_t cueChanged = !m_hasSkyCueKey ||
		m_iSkyNetEntityId != input.iNetEntityId ||
		m_iSkyPatternSequence != input.iPatternSequence ||
		m_iSkyStageIndex != input.iStageIndex ||
		m_iSkyActionStartTick != input.iActionStartTick ||
		m_strSkyCueId != cue->strCueId;
	const bool_t serverAdvanced = cueChanged ||
		CActionPresentationTimeline::Is_ForwardTick(
			input.iServerTick, m_iSkyLastServerTick);
	if (serverAdvanced)
	{
		f32_t authoritativeAge = 0.f;
		if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
			input.iServerTick, input.iActionStartTick,
			static_cast<f32_t>(m_iFixedTickHz),
			authoritativeAge))
		{
			Reset_SkyTimeline();
			return state;
		}
		m_fSkyElapsedSeconds = authoritativeAge;
		m_iSkyLastServerTick = input.iServerTick;
	}
	else if (input.iServerTick == m_iSkyLastServerTick)
	{
		m_fSkyElapsedSeconds += (std::min)(timeDelta, 0.1f);
	}

	m_iSkyNetEntityId = input.iNetEntityId;
	m_iSkyPatternSequence = input.iPatternSequence;
	m_iSkyStageIndex = input.iStageIndex;
	m_iSkyActionStartTick = input.iActionStartTick;
	m_strSkyCueId = cue->strCueId;
	m_hasSkyCueKey = true;

	const f32_t startSeconds =
		static_cast<f32_t>(cue->iStageLocalStartMs) * 0.001f;
	const f32_t endSeconds = static_cast<f32_t>(cue->iStageLocalEndMs) * 0.001f;
	if (m_fSkyElapsedSeconds < startSeconds)
	{
		return state;
	}
	const f32_t span = endSeconds - startSeconds;
	const f32_t ratio = span <= 0.f ? 1.f :
		std::clamp((m_fSkyElapsedSeconds - startSeconds) / span, 0.f, 1.f);
	state.isActive = true;
	state.strCueId = cue->strCueId;
	state.strRedCloudAssetId = cue->strRedCloudAssetId;
	state.strBlackApertureAssetId = cue->strBlackApertureAssetId;
	state.fCloudOpacity = cue->fCloudOpacityStart +
		(cue->fCloudOpacityEnd - cue->fCloudOpacityStart) * ratio;
	state.fApertureScale = cue->fApertureScaleStart +
		(cue->fApertureScaleEnd - cue->fApertureScaleStart) * ratio;
	f32_t rotationPhaseOffset = 0.f;
	for (const VALTAN_CINEMATIC_SKY_CUE& prior :
		m_pDocument->Get_SkyCues())
	{
		if (prior.strPatternId != cue->strPatternId ||
			prior.iStageIndex >= cue->iStageIndex)
		{
			continue;
		}
		rotationPhaseOffset += prior.fCloudRotationDegreesPerSecond *
			static_cast<f32_t>(
				prior.iStageLocalEndMs - prior.iStageLocalStartMs) * 0.001f;
	}
	state.fCloudRotationDegrees =
		rotationPhaseOffset + cue->fCloudRotationDegreesPerSecond *
		(m_fSkyElapsedSeconds - startSeconds);
	m_LastSkyState = state;
	return state;
}

bool_t Client::CValtanCinematicCameraController::Update(
	const VALTAN_CINEMATIC_CAMERA_INPUT& input,
	const f32_t timeDelta,
	VALTAN_CINEMATIC_CAMERA_POSE& outPose)
{
	/* A dead boss carries no pattern sequence, so the clear shot is the one cue
	   that does not require one. Everything else still does. */
	if (nullptr == m_pDocument || !input.isValid || 0u == input.iNetEntityId ||
		0u == input.iServerTick ||
		(0u == input.iPatternSequence && !input.isBossDead) ||
		0u == input.iActionStartTick || !std::isfinite(timeDelta) ||
		timeDelta < 0.f)
	{
		m_pActiveCue = nullptr;
		m_strCueId.clear();
		m_hasCueKey = false;
		m_isCueFinished = false;
		return false;
	}

	const VALTAN_CINEMATIC_CAMERA_CUE* cue = input.isBossDead ?
		m_pDocument->Find_DeathCue() :
		m_pDocument->Find_Cue(
			input.strPatternId, input.iStageIndex, input.strStageActionId);
	if (nullptr == cue || (!input.isBossDead && !input.strStageId.empty() &&
		cue->strStageId != input.strStageId))
	{
		m_pActiveCue = nullptr;
		m_strCueId.clear();
		m_hasCueKey = false;
		m_isCueFinished = false;
		return false;
	}

	const bool_t cueChanged = !m_hasCueKey ||
		m_iNetEntityId != input.iNetEntityId ||
		m_iPatternSequence != input.iPatternSequence ||
		m_iStageIndex != input.iStageIndex ||
		m_iActionStartTick != input.iActionStartTick ||
		m_strCueId != cue->strCueId;
	if (!cueChanged && m_isCueFinished)
		return false;

	const bool_t serverAdvanced = cueChanged ||
		Client::CActionPresentationTimeline::Is_ForwardTick(
			input.iServerTick, m_iLastServerTick);
	if (serverAdvanced)
	{
		f32_t authoritativeAge = 0.f;
		if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
			input.iServerTick, input.iActionStartTick,
			static_cast<f32_t>(m_iFixedTickHz), authoritativeAge))
		{
			m_pActiveCue = nullptr;
			m_strCueId.clear();
			m_hasCueKey = false;
			m_isCueFinished = false;
			return false;
		}
		m_fElapsedSeconds = authoritativeAge;
		m_iLastServerTick = input.iServerTick;
	}
	else
	{
		m_fElapsedSeconds += (std::min)(timeDelta, 0.1f);
	}

	m_pActiveCue = cue;
	m_iNetEntityId = input.iNetEntityId;
	m_iPatternSequence = input.iPatternSequence;
	m_iStageIndex = input.iStageIndex;
	m_iActionStartTick = input.iActionStartTick;
	m_strCueId = cue->strCueId;
	m_hasCueKey = true;
	m_isCueFinished = false;
	if (m_fElapsedSeconds * 1000.f >= static_cast<f32_t>(cue->iDurationMs))
	{
		m_pActiveCue = nullptr;
		m_isCueFinished = true;
		return false;
	}
	return Sample_ActiveCue(m_fElapsedSeconds, outPose);
}

void Client::CValtanCinematicCameraController::Reset()
{
	m_pDocument = nullptr;
	m_pActiveCue = nullptr;
	m_iFixedTickHz = 0u;
	m_iNetEntityId = 0u;
	m_iPatternSequence = 0u;
	m_iStageIndex = 0u;
	m_iActionStartTick = 0u;
	m_iLastServerTick = 0u;
	m_fElapsedSeconds = 0.f;
	m_strCueId.clear();
	m_hasCueKey = false;
	m_isCueFinished = false;
	Reset_SkyTimeline();
}

void Client::CValtanCinematicCameraController::Reset_SkyTimeline()
{
	m_iSkyNetEntityId = 0u;
	m_iSkyPatternSequence = 0u;
	m_iSkyStageIndex = 0u;
	m_iSkyActionStartTick = 0u;
	m_iSkyLastServerTick = 0u;
	m_fSkyElapsedSeconds = 0.f;
	m_strSkyCueId.clear();
	m_hasSkyCueKey = false;
	m_LastSkyState = VALTAN_CINEMATIC_SKY_STATE{};
}

bool_t Client::CValtanCinematicCameraController::Sample_ActiveCue(
	const f32_t elapsedSeconds,
	VALTAN_CINEMATIC_CAMERA_POSE& outPose) const
{
	if (nullptr == m_pActiveCue || m_pActiveCue->Keyframes.size() < 2u)
		return false;
	const f32_t elapsedMs = elapsedSeconds * 1000.f;
	const auto upper = std::upper_bound(
		m_pActiveCue->Keyframes.begin(), m_pActiveCue->Keyframes.end(), elapsedMs,
		[](const f32_t value, const VALTAN_CINEMATIC_CAMERA_KEYFRAME& frame)
		{ return value < static_cast<f32_t>(frame.iTimeMs); });
	if (m_pActiveCue->Keyframes.begin() == upper)
	{
		const auto& first = m_pActiveCue->Keyframes.front();
		outPose = { first.vEye, first.vLookAt, first.fFovYDegrees };
		return true;
	}
	const auto& left = *(upper - 1);
	if (m_pActiveCue->Keyframes.end() == upper)
	{
		outPose = { left.vEye, left.vLookAt, left.fFovYDegrees };
		return true;
	}
	const auto& right = *upper;
	const f32_t span = static_cast<f32_t>(right.iTimeMs - left.iTimeMs);
	const f32_t rawAlpha = span <= 0.f ? 0.f : (std::clamp)(
		(elapsedMs - static_cast<f32_t>(left.iTimeMs)) / span, 0.f, 1.f);
	f32_t alpha = rawAlpha;
	switch (m_pActiveCue->eEasing)
	{
	case VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP:
		alpha = rawAlpha * rawAlpha * (3.f - 2.f * rawAlpha);
		break;
	case VALTAN_CINEMATIC_CAMERA_EASING::HOLD:
		alpha = 0.f;
		break;
	default:
		break;
	}
	XMStoreFloat3(&outPose.vEye, XMVectorLerp(
		XMLoadFloat3(&left.vEye), XMLoadFloat3(&right.vEye), alpha));
	XMStoreFloat3(&outPose.vLookAt, XMVectorLerp(
		XMLoadFloat3(&left.vLookAt), XMLoadFloat3(&right.vLookAt), alpha));
	outPose.fFovYDegrees = left.fFovYDegrees +
		(right.fFovYDegrees - left.fFovYDegrees) * alpha;
	Apply_ImpactShake(elapsedSeconds, outPose);
	return true;
}

void Client::CValtanCinematicCameraController::Apply_ImpactShake(
	const f32_t elapsedSeconds,
	VALTAN_CINEMATIC_CAMERA_POSE& outPose) const
{
	if (nullptr == m_pActiveCue || m_pActiveCue->fShakeAmplitude <= 0.f ||
		0u == m_pActiveCue->iShakeDurationMs || elapsedSeconds < 0.f)
	{
		return;
	}
	const f32_t duration =
		static_cast<f32_t>(m_pActiveCue->iShakeDurationMs) * 0.001f;
	if (elapsedSeconds >= duration)
		return;
	/* A fixed frequency against the cue clock, decaying linearly to zero. No
	   RNG and no frame-time dependence, so every client shakes identically and
	   the jolt cannot outlive the stage. */
	constexpr f32_t SHAKE_RADIANS_PER_SECOND = 84.f;
	const f32_t decay = 1.f - elapsedSeconds / duration;
	const f32_t phase = elapsedSeconds * SHAKE_RADIANS_PER_SECOND;
	const f32_t amplitude = m_pActiveCue->fShakeAmplitude * decay * decay;
	outPose.vEye.x += std::sin(phase) * amplitude;
	outPose.vEye.y += std::sin(phase * 1.7f) * amplitude * 0.6f;
	outPose.vEye.z += std::cos(phase * 1.3f) * amplitude;
}
