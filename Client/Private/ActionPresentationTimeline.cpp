#include "ActionPresentationTimeline.h"

#include <algorithm>
#include <cmath>
#include <limits>

bool Client::CActionPresentationTimeline::Resolve_ClipDuration(
	const ACTION_PRESENTATION_CLIP_TIMING& Clip,
	float& fOutSourceDurationSeconds,
	float& fOutWallDurationSeconds)
{
	fOutSourceDurationSeconds = 0.f;
	fOutWallDurationSeconds = 0.f;
	if (!std::isfinite(Clip.fModelSourceDurationSeconds) ||
		Clip.fModelSourceDurationSeconds <= 0.f ||
		!std::isfinite(Clip.fPlayRate) || Clip.fPlayRate <= 0.f)
	{
		return false;
	}
	fOutSourceDurationSeconds = Clip.fModelSourceDurationSeconds;
	if (0u != Clip.iPlayMs)
	{
		fOutSourceDurationSeconds = (std::min)(
			fOutSourceDurationSeconds,
			static_cast<float>(Clip.iPlayMs) * 0.001f);
	}
	fOutWallDurationSeconds = fOutSourceDurationSeconds / Clip.fPlayRate;
	return std::isfinite(fOutSourceDurationSeconds) &&
		fOutSourceDurationSeconds > 0.f &&
		std::isfinite(fOutWallDurationSeconds) &&
		fOutWallDurationSeconds > 0.f;
}

bool Client::CActionPresentationTimeline::Resolve_Sample(
	const std::span<const ACTION_PRESENTATION_CLIP_TIMING> Clips,
	const float fStageWallTimeSeconds,
	ACTION_PRESENTATION_SAMPLE& OutSample)
{
	if (Clips.empty() || !std::isfinite(fStageWallTimeSeconds) ||
		fStageWallTimeSeconds < 0.f)
	{
		return false;
	}

	float fRemainingWallSeconds = fStageWallTimeSeconds;
	for (std::size_t iClip = 0u; iClip < Clips.size(); ++iClip)
	{
		float fSourceDurationSeconds = 0.f;
		float fWallDurationSeconds = 0.f;
		if (!Resolve_ClipDuration(
			Clips[iClip], fSourceDurationSeconds, fWallDurationSeconds))
		{
			return false;
		}

		const bool bLastClip = iClip + 1u == Clips.size();
		if (Clips[iClip].bLoop)
		{
			const double fEpoch = std::floor(
				static_cast<double>(fRemainingWallSeconds) /
				static_cast<double>(fWallDurationSeconds));
			if (!std::isfinite(fEpoch) || fEpoch < 0.0 ||
				fEpoch > static_cast<double>((std::numeric_limits<uint64_t>::max)()))
			{
				return false;
			}
			OutSample.iClipIndex = iClip;
			OutSample.iLoopEpoch = static_cast<uint64_t>(fEpoch);
			const float fLoopWallSeconds = std::fmod(
				fRemainingWallSeconds, fWallDurationSeconds);
			OutSample.fClipSourceTimeSeconds = (std::min)(
				(std::max)(0.f, fLoopWallSeconds) * Clips[iClip].fPlayRate,
				fSourceDurationSeconds);
			OutSample.fStageWallTimeSeconds = fStageWallTimeSeconds;
			return true;
		}

		/* An exact boundary belongs to the next sequential clip. This keeps
		seek and cue crossing on one unambiguous wall-time partition. */
		if (!bLastClip &&
			fRemainingWallSeconds >= fWallDurationSeconds)
		{
			fRemainingWallSeconds = (std::max)(
				0.f, fRemainingWallSeconds - fWallDurationSeconds);
			continue;
		}

		OutSample.iClipIndex = iClip;
		OutSample.iLoopEpoch = 0u;
		OutSample.fClipSourceTimeSeconds = (std::min)(
			fRemainingWallSeconds * Clips[iClip].fPlayRate,
			fSourceDurationSeconds);
		OutSample.fStageWallTimeSeconds = fStageWallTimeSeconds;
		return true;
	}
	return false;
}

bool Client::CActionPresentationTimeline::Resolve_CueWallOffset(
	const std::span<const ACTION_PRESENTATION_CLIP_TIMING> Clips,
	const std::size_t iClipIndex,
	const float fCueSourceTimeSeconds,
	const uint64_t iLoopEpoch,
	float& fOutStageWallTimeSeconds)
{
	fOutStageWallTimeSeconds = 0.f;
	if (iClipIndex >= Clips.size() ||
		!std::isfinite(fCueSourceTimeSeconds) || fCueSourceTimeSeconds < 0.f)
	{
		return false;
	}

	double fWallOffset = 0.0;
	for (std::size_t iClip = 0u; iClip <= iClipIndex; ++iClip)
	{
		float fSourceDurationSeconds = 0.f;
		float fWallDurationSeconds = 0.f;
		if (!Resolve_ClipDuration(
			Clips[iClip], fSourceDurationSeconds, fWallDurationSeconds))
		{
			return false;
		}
		if (iClip < iClipIndex)
		{
			/* A loop consumes the rest of its stage, so a later clip is not
			reachable until the Server starts a new authored stage. */
			if (Clips[iClip].bLoop)
				return false;
			fWallOffset += fWallDurationSeconds;
			continue;
		}

		if (fCueSourceTimeSeconds > fSourceDurationSeconds + 0.000001f ||
			(!Clips[iClip].bLoop && 0u != iLoopEpoch))
		{
			return false;
		}
		fWallOffset += static_cast<double>(iLoopEpoch) *
			static_cast<double>(fWallDurationSeconds);
		fWallOffset += (std::min)(
			fCueSourceTimeSeconds, fSourceDurationSeconds) /
			Clips[iClip].fPlayRate;
	}
	if (!std::isfinite(fWallOffset) ||
		fWallOffset > static_cast<double>((std::numeric_limits<float>::max)()))
	{
		return false;
	}
	fOutStageWallTimeSeconds = static_cast<float>(fWallOffset);
	return true;
}

bool Client::CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
	const uint32_t iServerTick,
	const uint32_t iActionStartTick,
	const float fServerTickHz,
	float& fOutActionAgeSeconds)
{
	fOutActionAgeSeconds = 0.f;
	if (0u == iServerTick || 0u == iActionStartTick ||
		!std::isfinite(fServerTickHz) || fServerTickHz <= 0.f)
	{
		return false;
	}
	if (iServerTick != iActionStartTick &&
		!Is_ForwardTick(iServerTick, iActionStartTick))
	{
		return false;
	}
	const uint64_t iAgeTicks = iServerTick >= iActionStartTick ?
		static_cast<uint64_t>(iServerTick - iActionStartTick) :
		static_cast<uint64_t>((std::numeric_limits<uint32_t>::max)() -
			iActionStartTick) + static_cast<uint64_t>(iServerTick);
	fOutActionAgeSeconds = static_cast<float>(iAgeTicks) / fServerTickHz;
	return std::isfinite(fOutActionAgeSeconds);
}

bool Client::CActionPresentationTimeline::Is_ForwardTick(
	const uint32_t iCandidateTick,
	const uint32_t iPreviousTick)
{
	if (0u == iCandidateTick)
		return false;
	if (0u == iPreviousTick)
		return true;
	return static_cast<int32_t>(iCandidateTick - iPreviousTick) > 0;
}
