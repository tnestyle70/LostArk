#include "ValtanCinematicCameraController.h"

#include "ActionPresentationTimeline.h"

#include <algorithm>
#include <cmath>

namespace
{
	bool_t Is_FinitePosition(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	}

	bool_t Is_ValidPose(
		const Client::VALTAN_CINEMATIC_CAMERA_POSE& pose)
	{
		if (!Is_FinitePosition(pose.vEye) ||
			!Is_FinitePosition(pose.vLookAt) ||
			!std::isfinite(pose.fFovYDegrees) ||
			pose.fFovYDegrees <= 1.f || pose.fFovYDegrees >= 179.f)
		{
			return false;
		}
		const f32_t deltaX = pose.vLookAt.x - pose.vEye.x;
		const f32_t deltaY = pose.vLookAt.y - pose.vEye.y;
		const f32_t deltaZ = pose.vLookAt.z - pose.vEye.z;
		return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ > 0.000001f;
	}

	bool_t Has_RequiredTrackingInput(
		const Client::VALTAN_CINEMATIC_CAMERA_CUE& cue,
		const Client::VALTAN_CINEMATIC_CAMERA_INPUT& input)
	{
		switch (cue.eTrackingMode)
		{
		case Client::VALTAN_CINEMATIC_TRACKING_MODE::WORLD:
			return true;
		case Client::VALTAN_CINEMATIC_TRACKING_MODE::BOSS_XZ:
			return Is_FinitePosition(input.vBossPosition);
		case Client::VALTAN_CINEMATIC_TRACKING_MODE::BOSS_FACING:
			return Is_FinitePosition(input.vBossPosition) &&
				std::isfinite(input.fBossYawDegrees);
		case Client::VALTAN_CINEMATIC_TRACKING_MODE::PLAYER_BOSS_FRAME:
			return Is_FinitePosition(input.vBossPosition) &&
				std::isfinite(input.fBossYawDegrees) &&
				input.hasLocalPlayerPosition &&
				Is_FinitePosition(input.vLocalPlayerPosition);
		default:
			return false;
		}
	}

	float3_t Resolve_BossFacingPoint(
		const float3_t& authoredPoint,
		const float3_t& authoredOrigin,
		const float3_t& bossPosition,
		const f32_t bossYawDegrees)
	{
		const f32_t radians = XMConvertToRadians(bossYawDegrees);
		const f32_t sine = std::sin(radians);
		const f32_t cosine = std::cos(radians);
		const f32_t localX = authoredPoint.x - authoredOrigin.x;
		const f32_t localY = authoredPoint.y - authoredOrigin.y;
		const f32_t localZ = authoredPoint.z - authoredOrigin.z;
		return float3_t(
			bossPosition.x + localX * cosine + localZ * sine,
			bossPosition.y + localY,
			bossPosition.z - localX * sine + localZ * cosine);
	}

	float3_t Resolve_PlayerBossFramePoint(
		const float3_t& authoredPoint,
		const float3_t& authoredOrigin,
		const Client::VALTAN_CINEMATIC_CAMERA_INPUT& input)
	{
		const f32_t deltaX =
			input.vBossPosition.x - input.vLocalPlayerPosition.x;
		const f32_t deltaZ =
			input.vBossPosition.z - input.vLocalPlayerPosition.z;
		const f32_t separation = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
		f32_t forwardX = 0.f;
		f32_t forwardZ = 1.f;
		if (separation > 0.001f)
		{
			forwardX = deltaX / separation;
			forwardZ = deltaZ / separation;
		}
		else
		{
			const f32_t yawRadians = XMConvertToRadians(input.fBossYawDegrees);
			forwardX = std::sin(yawRadians);
			forwardZ = std::cos(yawRadians);
		}
		const f32_t rightX = forwardZ;
		const f32_t rightZ = -forwardX;
		const f32_t horizontalScale =
			std::clamp(separation / 10.f, 0.8f, 2.5f);
		const f32_t verticalScale =
			std::clamp(0.75f + separation / 24.f, 1.f, 1.6f);
		const f32_t localX = authoredPoint.x - authoredOrigin.x;
		const f32_t localY = authoredPoint.y - authoredOrigin.y;
		const f32_t localZ = authoredPoint.z - authoredOrigin.z;
		const float3_t frameOrigin(
			(input.vBossPosition.x + input.vLocalPlayerPosition.x) * 0.5f,
			(std::max)(input.vBossPosition.y, input.vLocalPlayerPosition.y),
			(input.vBossPosition.z + input.vLocalPlayerPosition.z) * 0.5f);
		return float3_t(
			frameOrigin.x + rightX * localX * horizontalScale +
				forwardX * localZ * horizontalScale,
			frameOrigin.y + localY * verticalScale,
			frameOrigin.z + rightZ * localX * horizontalScale +
				forwardZ * localZ * horizontalScale);
	}

	void Apply_CueShake(
		const Client::VALTAN_CINEMATIC_CAMERA_CUE& cue,
		const f32_t elapsedSeconds,
		Client::VALTAN_CINEMATIC_CAMERA_POSE& outPose)
	{
		if (cue.fShakeAmplitude <= 0.f || 0u == cue.iShakeDurationMs ||
			elapsedSeconds < 0.f)
		{
			return;
		}
		const f32_t duration =
			static_cast<f32_t>(cue.iShakeDurationMs) * 0.001f;
		if (elapsedSeconds >= duration)
			return;
		constexpr f32_t SHAKE_RADIANS_PER_SECOND = 84.f;
		const f32_t decay = 1.f - elapsedSeconds / duration;
		const f32_t phase = elapsedSeconds * SHAKE_RADIANS_PER_SECOND;
		const f32_t amplitude = cue.fShakeAmplitude * decay * decay;
		outPose.vEye.x += std::sin(phase) * amplitude;
		outPose.vEye.y += std::sin(phase * 1.7f) * amplitude * 0.6f;
		outPose.vEye.z += std::cos(phase * 1.3f) * amplitude;
	}
}

bool_t Client::CValtanCinematicCameraController::Sample_Cue(
	const VALTAN_CINEMATIC_CAMERA_CUE& cue,
	const f32_t elapsedSeconds,
	VALTAN_CINEMATIC_CAMERA_POSE& outPose)
{
	if (cue.Keyframes.size() < 2u || !std::isfinite(elapsedSeconds) ||
		elapsedSeconds < 0.f)
	{
		return false;
	}
	const f32_t elapsedMs = elapsedSeconds * 1000.f;
	const auto upper = std::upper_bound(
		cue.Keyframes.begin(), cue.Keyframes.end(), elapsedMs,
		[](const f32_t value, const VALTAN_CINEMATIC_CAMERA_KEYFRAME& frame)
		{ return value < static_cast<f32_t>(frame.iTimeMs); });
	if (cue.Keyframes.begin() == upper)
	{
		const auto& first = cue.Keyframes.front();
		outPose = { first.vEye, first.vLookAt, first.fFovYDegrees };
		return true;
	}
	const auto& left = *(upper - 1);
	if (cue.Keyframes.end() == upper)
	{
		outPose = { left.vEye, left.vLookAt, left.fFovYDegrees };
		return true;
	}
	const auto& right = *upper;
	const f32_t span = static_cast<f32_t>(right.iTimeMs - left.iTimeMs);
	const f32_t rawAlpha = span <= 0.f ? 0.f : (std::clamp)(
		(elapsedMs - static_cast<f32_t>(left.iTimeMs)) / span, 0.f, 1.f);
	f32_t alpha = rawAlpha;
	switch (cue.eEasing)
	{
	case VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP:
		alpha = rawAlpha * rawAlpha * (3.f - 2.f * rawAlpha);
		break;
	case VALTAN_CINEMATIC_CAMERA_EASING::HOLD:
		alpha = 0.f;
		break;
	case VALTAN_CINEMATIC_CAMERA_EASING::LINEAR:
		break;
	default:
		return false;
	}
	XMStoreFloat3(&outPose.vEye, XMVectorLerp(
		XMLoadFloat3(&left.vEye), XMLoadFloat3(&right.vEye), alpha));
	XMStoreFloat3(&outPose.vLookAt, XMVectorLerp(
		XMLoadFloat3(&left.vLookAt), XMLoadFloat3(&right.vLookAt), alpha));
	outPose.fFovYDegrees = left.fFovYDegrees +
		(right.fFovYDegrees - left.fFovYDegrees) * alpha;
	Apply_CueShake(cue, elapsedSeconds, outPose);
	return true;
}

bool_t Client::CValtanCinematicCameraController::Apply_CueTracking(
	const VALTAN_CINEMATIC_CAMERA_CUE& cue,
	const VALTAN_CINEMATIC_CAMERA_INPUT& input,
	VALTAN_CINEMATIC_CAMERA_POSE& inOutPose)
{
	if (!Has_RequiredTrackingInput(cue, input))
		return false;
	switch (cue.eTrackingMode)
	{
	case VALTAN_CINEMATIC_TRACKING_MODE::WORLD:
		return true;
	case VALTAN_CINEMATIC_TRACKING_MODE::BOSS_XZ:
	{
		const f32_t offsetX = input.vBossPosition.x - cue.vTrackingOrigin.x;
		const f32_t offsetZ = input.vBossPosition.z - cue.vTrackingOrigin.z;
		inOutPose.vEye.x += offsetX;
		inOutPose.vEye.z += offsetZ;
		inOutPose.vLookAt.x += offsetX;
		inOutPose.vLookAt.z += offsetZ;
		return true;
	}
	case VALTAN_CINEMATIC_TRACKING_MODE::BOSS_FACING:
		inOutPose.vEye = Resolve_BossFacingPoint(
			inOutPose.vEye, cue.vTrackingOrigin,
			input.vBossPosition, input.fBossYawDegrees);
		inOutPose.vLookAt = Resolve_BossFacingPoint(
			inOutPose.vLookAt, cue.vTrackingOrigin,
			input.vBossPosition, input.fBossYawDegrees);
		return true;
	case VALTAN_CINEMATIC_TRACKING_MODE::PLAYER_BOSS_FRAME:
		inOutPose.vEye = Resolve_PlayerBossFramePoint(
			inOutPose.vEye, cue.vTrackingOrigin, input);
		inOutPose.vLookAt = Resolve_PlayerBossFramePoint(
			inOutPose.vLookAt, cue.vTrackingOrigin, input);
		return true;
	default:
		return false;
	}
}

bool_t Client::CValtanCinematicCameraController::Sample_BoundedTransition(
	const VALTAN_CINEMATIC_CAMERA_POSE& fromPose,
	const VALTAN_CINEMATIC_CAMERA_POSE& toPose,
	const uint32_t durationMs,
	const f32_t elapsedSeconds,
	VALTAN_CINEMATIC_CAMERA_POSE& outPose)
{
	if (!Is_ValidPose(fromPose) || !Is_ValidPose(toPose) ||
		0u == durationMs ||
		durationMs > VALTAN_CINEMATIC_CAMERA_CUE::MAX_TRANSITION_IN_MS ||
		!std::isfinite(elapsedSeconds) || elapsedSeconds < 0.f)
	{
		return false;
	}
	const f32_t durationSeconds = static_cast<f32_t>(durationMs) * 0.001f;
	const f32_t rawAlpha = (std::clamp)(
		elapsedSeconds / durationSeconds, 0.f, 1.f);
	const f32_t alpha = rawAlpha * rawAlpha * (3.f - 2.f * rawAlpha);
	XMStoreFloat3(&outPose.vEye, XMVectorLerp(
		XMLoadFloat3(&fromPose.vEye), XMLoadFloat3(&toPose.vEye), alpha));
	XMStoreFloat3(&outPose.vLookAt, XMVectorLerp(
		XMLoadFloat3(&fromPose.vLookAt),
		XMLoadFloat3(&toPose.vLookAt), alpha));
	outPose.fFovYDegrees = fromPose.fFovYDegrees +
		(toPose.fFovYDegrees - fromPose.fFovYDegrees) * alpha;
	return true;
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
		Cancel_ExitTransition();
		m_pActiveCue = nullptr;
		m_strCueId.clear();
		m_hasCueKey = false;
		m_isCueFinished = false;
		m_hasLastOutputPose = false;
		m_isTransitionActive = false;
		return false;
	}

	const VALTAN_CINEMATIC_CAMERA_CUE* cue = input.isBossDead ?
		m_pDocument->Find_DeathCue() :
		m_pDocument->Find_Cue(
			input.strPatternId, input.iStageIndex, input.strStageActionId);
	if (nullptr == cue || (!input.isBossDead && !input.strStageId.empty() &&
		cue->strStageId != input.strStageId))
	{
		if (m_isCueFinished)
			return false;
		if (nullptr != m_pActiveCue)
			Begin_ExitTransition(*m_pActiveCue);
		m_pActiveCue = nullptr;
		m_isTransitionActive = false;
		if (m_isExitTransitionActive)
		{
			m_isCueFinished = true;
			return false;
		}
		m_strCueId.clear();
		m_hasCueKey = false;
		m_isCueFinished = false;
		m_hasLastOutputPose = false;
		m_isTransitionActive = false;
		return false;
	}
	if (!Has_RequiredTrackingInput(*cue, input))
	{
		Cancel_ExitTransition();
		m_pActiveCue = nullptr;
		m_strCueId.clear();
		m_hasCueKey = false;
		m_isCueFinished = false;
		m_hasLastOutputPose = false;
		m_isTransitionActive = false;
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
	/* Cross-frame continuity is valid only for the next stage of the same
	   Server-owned pattern occurrence. A late join, replay restart, stage skip,
	   entity change, or unrelated pattern samples the incoming cue directly. */
	const bool_t shouldBeginTransition = cueChanged &&
		0u != cue->iTransitionInMs && m_hasCueKey && m_hasLastOutputPose &&
		m_iNetEntityId == input.iNetEntityId &&
		m_iPatternSequence == input.iPatternSequence &&
		input.iStageIndex > 0u && m_iStageIndex == input.iStageIndex - 1u;
	if (cueChanged)
		Cancel_ExitTransition();

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
			Cancel_ExitTransition();
			m_pActiveCue = nullptr;
			m_strCueId.clear();
			m_hasCueKey = false;
			m_isCueFinished = false;
			m_hasLastOutputPose = false;
			m_isTransitionActive = false;
			return false;
		}
		m_fElapsedSeconds = authoritativeAge;
		m_iLastServerTick = input.iServerTick;
	}
	else
	{
		m_fElapsedSeconds += (std::min)(timeDelta, 0.1f);
	}
	if (cueChanged)
	{
		m_isTransitionActive = shouldBeginTransition &&
			m_fElapsedSeconds * 1000.f <
				static_cast<f32_t>(cue->iTransitionInMs);
		if (m_isTransitionActive)
			m_TransitionFromPose = m_LastOutputPose;
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
		Begin_ExitTransition(*cue);
		m_pActiveCue = nullptr;
		m_isCueFinished = true;
		m_isTransitionActive = false;
		return false;
	}
	if (!Sample_ActiveCue(m_fElapsedSeconds, outPose))
	{
		Cancel_ExitTransition();
		m_hasLastOutputPose = false;
		m_isTransitionActive = false;
		return false;
	}
	if (!Apply_Tracking(input, outPose))
	{
		Cancel_ExitTransition();
		m_hasLastOutputPose = false;
		m_isTransitionActive = false;
		return false;
	}
	Apply_CueTransition(m_fElapsedSeconds, outPose);
	m_LastOutputPose = outPose;
	m_hasLastOutputPose = true;
	return true;
}

bool_t Client::CValtanCinematicCameraController::Update_ExitTransition(
	const VALTAN_CINEMATIC_CAMERA_POSE& followPose,
	const f32_t timeDelta,
	VALTAN_CINEMATIC_CAMERA_POSE& outPose)
{
	if (!m_isExitTransitionActive || 0u == m_iExitTransitionMs ||
		!std::isfinite(timeDelta) || timeDelta < 0.f ||
		!Is_ValidPose(followPose))
	{
		return false;
	}
	/* The target is resolved again by the Level every frame, while the clamped
	   local presentation clock guarantees convergence inside the authored
	   bound even if a render hitch supplies an unusually large delta. */
	m_fExitTransitionElapsedSeconds += (std::min)(timeDelta, 0.1f);
	if (!Sample_BoundedTransition(
		m_ExitTransitionFromPose, followPose, m_iExitTransitionMs,
		m_fExitTransitionElapsedSeconds, outPose))
	{
		Cancel_ExitTransition();
		return false;
	}
	m_LastOutputPose = outPose;
	m_hasLastOutputPose = true;
	if (m_fExitTransitionElapsedSeconds * 1000.f >=
		static_cast<f32_t>(m_iExitTransitionMs))
	{
		m_isExitTransitionActive = false;
	}
	return true;
}

void Client::CValtanCinematicCameraController::Cancel_ExitTransition()
{
	m_ExitTransitionFromPose = VALTAN_CINEMATIC_CAMERA_POSE{};
	m_iExitTransitionMs = 0u;
	m_fExitTransitionElapsedSeconds = 0.f;
	m_isExitTransitionActive = false;
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
	m_LastOutputPose = VALTAN_CINEMATIC_CAMERA_POSE{};
	m_TransitionFromPose = VALTAN_CINEMATIC_CAMERA_POSE{};
	m_hasLastOutputPose = false;
	m_isTransitionActive = false;
	Cancel_ExitTransition();
}

bool_t Client::CValtanCinematicCameraController::Sample_ActiveCue(
	const f32_t elapsedSeconds,
	VALTAN_CINEMATIC_CAMERA_POSE& outPose) const
{
	if (nullptr == m_pActiveCue || m_pActiveCue->Keyframes.size() < 2u)
		return false;
	return Sample_Cue(*m_pActiveCue, elapsedSeconds, outPose);
}

bool_t Client::CValtanCinematicCameraController::Apply_Tracking(
	const VALTAN_CINEMATIC_CAMERA_INPUT& input,
	VALTAN_CINEMATIC_CAMERA_POSE& outPose) const
{
	if (nullptr == m_pActiveCue)
		return false;
	return Apply_CueTracking(*m_pActiveCue, input, outPose);
}

void Client::CValtanCinematicCameraController::Apply_CueTransition(
	const f32_t elapsedSeconds,
	VALTAN_CINEMATIC_CAMERA_POSE& inOutPose)
{
	if (!m_isTransitionActive || nullptr == m_pActiveCue ||
		0u == m_pActiveCue->iTransitionInMs)
	{
		return;
	}
	VALTAN_CINEMATIC_CAMERA_POSE blendedPose{};
	if (!Sample_BoundedTransition(
		m_TransitionFromPose, inOutPose, m_pActiveCue->iTransitionInMs,
		elapsedSeconds, blendedPose))
	{
		m_isTransitionActive = false;
		return;
	}
	inOutPose = blendedPose;
	if (elapsedSeconds * 1000.f >=
		static_cast<f32_t>(m_pActiveCue->iTransitionInMs))
	{
		m_isTransitionActive = false;
	}
}

void Client::CValtanCinematicCameraController::Begin_ExitTransition(
	const VALTAN_CINEMATIC_CAMERA_CUE& outgoingCue)
{
	Cancel_ExitTransition();
	if (0u == outgoingCue.iTransitionOutMs ||
		outgoingCue.iTransitionOutMs >
			VALTAN_CINEMATIC_CAMERA_CUE::MAX_TRANSITION_IN_MS ||
		!m_hasLastOutputPose)
		return;
	m_ExitTransitionFromPose = m_LastOutputPose;
	m_iExitTransitionMs = outgoingCue.iTransitionOutMs;
	m_fExitTransitionElapsedSeconds = 0.f;
	m_isExitTransitionActive = true;
}
