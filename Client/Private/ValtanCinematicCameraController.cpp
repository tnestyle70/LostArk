#include "ValtanCinematicCameraController.h"

#include "ActionPresentationTimeline.h"

#include <algorithm>
#include <cmath>

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

bool_t Client::CValtanCinematicCameraController::Update(
	const VALTAN_CINEMATIC_CAMERA_INPUT& input,
	const f32_t timeDelta,
	VALTAN_CINEMATIC_CAMERA_POSE& outPose)
{
	if (nullptr == m_pDocument || !input.isValid || 0u == input.iNetEntityId ||
		0u == input.iServerTick || 0u == input.iPatternSequence ||
		0u == input.iActionStartTick || !std::isfinite(timeDelta) ||
		timeDelta < 0.f)
	{
		m_pActiveCue = nullptr;
		m_strCueId.clear();
		m_hasCueKey = false;
		m_isCueFinished = false;
		return false;
	}

	const VALTAN_CINEMATIC_CAMERA_CUE* cue = m_pDocument->Find_Cue(
		input.strPatternId, input.iStageIndex, input.strStageActionId);
	if (nullptr == cue || (!input.strStageId.empty() &&
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
