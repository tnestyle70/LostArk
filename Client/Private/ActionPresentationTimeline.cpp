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
		!std::isfinite(Clip.fSourceStartSeconds) ||
		Clip.fSourceStartSeconds < 0.f ||
		Clip.fSourceStartSeconds >= Clip.fModelSourceDurationSeconds ||
		!std::isfinite(Clip.fPlayRate) || Clip.fPlayRate <= 0.f)
	{
		return false;
	}
	fOutSourceDurationSeconds =
		Clip.fModelSourceDurationSeconds - Clip.fSourceStartSeconds;
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
			OutSample.fClipSourceTimeSeconds =
				Clips[iClip].fSourceStartSeconds +
				(std::min)(
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
		OutSample.fClipSourceTimeSeconds = Clips[iClip].fSourceStartSeconds +
			(std::min)(
			fRemainingWallSeconds * Clips[iClip].fPlayRate,
			fSourceDurationSeconds);
		OutSample.fStageWallTimeSeconds = fStageWallTimeSeconds;
		return true;
	}
	return false;
}

bool Client::CActionPresentationTimeline::Resolve_PreviewSequenceSample(
	const std::span<const ACTION_PRESENTATION_CLIP_TIMING> Clips,
	const std::span<const float> WallBudgets,
	const float fTimelineWallTimeSeconds,
	ACTION_PRESENTATION_SAMPLE& OutSample)
{
	if (Clips.empty() || Clips.size() != WallBudgets.size() ||
		!std::isfinite(fTimelineWallTimeSeconds) || fTimelineWallTimeSeconds < 0.f)
		return false;
	for (std::size_t i = 0u; i < Clips.size(); ++i)
	{
		float SourceDuration = 0.f, WallDuration = 0.f;
		if (!std::isfinite(WallBudgets[i]) || WallBudgets[i] <= 0.f ||
			!Resolve_ClipDuration(Clips[i], SourceDuration, WallDuration))
			return false;
	}
	float Remaining = fTimelineWallTimeSeconds;
	for (std::size_t i = 0u; i < Clips.size(); ++i)
	{
		// Millisecond stage boundaries can differ by a float rounding unit.
		if (i + 1u < Clips.size() && Remaining + 0.000001f >= WallBudgets[i])
		{
			Remaining = (std::max)(0.f, Remaining - WallBudgets[i]);
			continue;
		}
		float LocalWall = (std::min)(Remaining, WallBudgets[i]);
		if (Clips[i].bLoop && LocalWall >= WallBudgets[i])
			LocalWall = (std::max)(0.f, LocalWall - 0.000001f);
		ACTION_PRESENTATION_SAMPLE Staged;
		if (!Resolve_Sample(Clips.subspan(i, 1u), LocalWall, Staged))
			return false;
		Staged.iClipIndex = i;
		Staged.fStageWallTimeSeconds = fTimelineWallTimeSeconds;
		OutSample = Staged;
		return true;
	}
	return false;
}

bool Client::CActionPresentationTimeline::Requires_ClipOccurrenceTransition(
	const std::size_t iCurrentClipOccurrenceIndex,
	const std::size_t iExpectedClipOccurrenceIndex,
	const uint32_t iCurrentAnimationIndex,
	const uint32_t iExpectedAnimationIndex)
{
	return iCurrentClipOccurrenceIndex != iExpectedClipOccurrenceIndex ||
		iCurrentAnimationIndex != iExpectedAnimationIndex;
}

namespace
{
	enum class CUE_SOURCE_BOUNDARY
	{
		START_HALF_OPEN,
		END_INCLUSIVE
	};

	bool ResolveCueWallOffset(
		const std::span<const Client::ACTION_PRESENTATION_CLIP_TIMING> Clips,
		const std::size_t iClipIndex,
		const float fCueSourceTimeSeconds,
		const uint64_t iLoopEpoch,
		const CUE_SOURCE_BOUNDARY eBoundary,
		float& fOutStageWallTimeSeconds)
	{
		fOutStageWallTimeSeconds = 0.f;
		if (iClipIndex >= Clips.size() ||
			!std::isfinite(fCueSourceTimeSeconds) ||
			fCueSourceTimeSeconds < 0.f)
		{
			return false;
		}

		double fWallOffset = 0.0;
		for (std::size_t iClip = 0u; iClip <= iClipIndex; ++iClip)
		{
			float fSourceDurationSeconds = 0.f;
			float fWallDurationSeconds = 0.f;
			if (!Client::CActionPresentationTimeline::Resolve_ClipDuration(
					Clips[iClip], fSourceDurationSeconds,
					fWallDurationSeconds))
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

			const float fSourceStartSeconds =
				Clips[iClip].fSourceStartSeconds;
			const float fSourceEndSeconds =
				fSourceStartSeconds + fSourceDurationSeconds;
			/* Starts keep adjacent explicit source slices half-open.  A finite
			cue end may close exactly on its own slice boundary. */
			const bool bPastSourceEnd = 0u != Clips[iClip].iPlayMs ?
				(CUE_SOURCE_BOUNDARY::END_INCLUSIVE == eBoundary ?
					fCueSourceTimeSeconds > fSourceEndSeconds + 0.000001f :
					fCueSourceTimeSeconds + 0.000001f >= fSourceEndSeconds) :
				fCueSourceTimeSeconds > fSourceEndSeconds + 0.000001f;
			if (fCueSourceTimeSeconds + 0.000001f < fSourceStartSeconds ||
				bPastSourceEnd ||
				(!Clips[iClip].bLoop && 0u != iLoopEpoch))
			{
				return false;
			}
			fWallOffset += static_cast<double>(iLoopEpoch) *
				static_cast<double>(fWallDurationSeconds);
			fWallOffset += ((std::min)(
				(std::max)(fCueSourceTimeSeconds, fSourceStartSeconds),
				fSourceEndSeconds) - fSourceStartSeconds) /
				Clips[iClip].fPlayRate;
		}
		if (!std::isfinite(fWallOffset) ||
			fWallOffset >
				static_cast<double>((std::numeric_limits<float>::max)()))
		{
			return false;
		}
		fOutStageWallTimeSeconds = static_cast<float>(fWallOffset);
		return true;
	}
}

bool Client::CActionPresentationTimeline::Resolve_CueWallOffset(
	const std::span<const ACTION_PRESENTATION_CLIP_TIMING> Clips,
	const std::size_t iClipIndex,
	const float fCueSourceTimeSeconds,
	const uint64_t iLoopEpoch,
	float& fOutStageWallTimeSeconds)
{
	return ResolveCueWallOffset(Clips, iClipIndex, fCueSourceTimeSeconds,
		iLoopEpoch, CUE_SOURCE_BOUNDARY::START_HALF_OPEN,
		fOutStageWallTimeSeconds);
}

bool Client::CActionPresentationTimeline::Resolve_CueEndWallOffset(
	const std::span<const ACTION_PRESENTATION_CLIP_TIMING> Clips,
	const std::size_t iClipIndex,
	const float fCueSourceTimeSeconds,
	const uint64_t iLoopEpoch,
	float& fOutStageWallTimeSeconds)
{
	return ResolveCueWallOffset(Clips, iClipIndex, fCueSourceTimeSeconds,
		iLoopEpoch, CUE_SOURCE_BOUNDARY::END_INCLUSIVE,
		fOutStageWallTimeSeconds);
}

bool Client::CActionPresentationTimeline::Resolve_CuePreviewSample(
	const ACTION_PRESENTATION_CUE_PREVIEW_TIMING& Timing,
	const float fTimelineWallSeconds,
	ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE& OutSample)
{
	OutSample = {};
	if (!std::isfinite(Timing.fClipSourceStartSeconds) ||
		Timing.fClipSourceStartSeconds < 0.f ||
		!std::isfinite(Timing.fPlayRate) || Timing.fPlayRate <= 0.f ||
		!std::isfinite(Timing.fCueSourceStartSeconds) ||
		Timing.fCueSourceStartSeconds < Timing.fClipSourceStartSeconds ||
		!std::isfinite(fTimelineWallSeconds) || fTimelineWallSeconds < 0.f)
	{
		return false;
	}
	if (Timing.bHasCueSourceEnd &&
		(!std::isfinite(Timing.fCueSourceEndSeconds) ||
		 Timing.fCueSourceEndSeconds <= Timing.fCueSourceStartSeconds))
	{
		return false;
	}

	const double fWallStart =
		(static_cast<double>(Timing.fCueSourceStartSeconds) -
		 static_cast<double>(Timing.fClipSourceStartSeconds)) /
		static_cast<double>(Timing.fPlayRate);
	const double fWallEnd = Timing.bHasCueSourceEnd ?
		(static_cast<double>(Timing.fCueSourceEndSeconds) -
		 static_cast<double>(Timing.fClipSourceStartSeconds)) /
			static_cast<double>(Timing.fPlayRate) : 0.0;
	const double fEffectSample = (std::max)(
		0.0, (static_cast<double>(fTimelineWallSeconds) - fWallStart) *
			static_cast<double>(Timing.fPlayRate));
	if (!std::isfinite(fWallStart) || fWallStart < 0.0 ||
		fWallStart > static_cast<double>((std::numeric_limits<float>::max)()) ||
		(Timing.bHasCueSourceEnd &&
		 (!std::isfinite(fWallEnd) || fWallEnd <= fWallStart ||
		  fWallEnd > static_cast<double>((std::numeric_limits<float>::max)()))) ||
		!std::isfinite(fEffectSample) ||
		fEffectSample > static_cast<double>((std::numeric_limits<float>::max)()))
	{
		return false;
	}

	OutSample.fCueWallStartSeconds = static_cast<float>(fWallStart);
	OutSample.fCueWallEndSeconds = static_cast<float>(fWallEnd);
	OutSample.fEffectSampleSeconds = static_cast<float>(fEffectSample);
	constexpr double START_EPSILON_SECONDS = 0.000001;
	OutSample.bVisible =
		static_cast<double>(fTimelineWallSeconds) + START_EPSILON_SECONDS >=
			fWallStart &&
		(!Timing.bHasCueSourceEnd ||
		 static_cast<double>(fTimelineWallSeconds) < fWallEnd);
	return true;
}

bool Client::CActionPresentationTimeline::Resolve_CuePreviewTimelineTime(
	const ACTION_PRESENTATION_CUE_PREVIEW_TIMING& Timing,
	const float fOwningClipTimelineOffsetSeconds,
	const float fEffectSampleSeconds,
	float& fOutTimelineSeconds)
{
	fOutTimelineSeconds = 0.f;
	ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
	if (!std::isfinite(fOwningClipTimelineOffsetSeconds) ||
		fOwningClipTimelineOffsetSeconds < 0.f ||
		!std::isfinite(fEffectSampleSeconds) || fEffectSampleSeconds < 0.f ||
		!Resolve_CuePreviewSample(Timing, 0.f, Sample))
	{
		return false;
	}
	const double fTimelineSeconds =
		static_cast<double>(fOwningClipTimelineOffsetSeconds) +
		static_cast<double>(Sample.fCueWallStartSeconds) +
		static_cast<double>(fEffectSampleSeconds) / Timing.fPlayRate;
	if (!std::isfinite(fTimelineSeconds) ||
		fTimelineSeconds > (std::numeric_limits<float>::max)())
	{
		return false;
	}
	fOutTimelineSeconds = static_cast<float>(fTimelineSeconds);
	return true;
}

bool Client::CActionPresentationTimeline::Resolve_CuePreviewDuration(
	const ACTION_PRESENTATION_CUE_PREVIEW_TIMING& Timing,
	const float fOwningClipTimelineOffsetSeconds,
	const float fTimelineDurationSeconds,
	const float fEffectDurationSeconds,
	float& fOutTimelineDurationSeconds)
{
	fOutTimelineDurationSeconds = 0.f;
	if (!std::isfinite(fOwningClipTimelineOffsetSeconds) ||
		fOwningClipTimelineOffsetSeconds < 0.f ||
		!std::isfinite(fTimelineDurationSeconds) || fTimelineDurationSeconds < 0.f ||
		!std::isfinite(fEffectDurationSeconds) || fEffectDurationSeconds < 0.f)
	{
		return false;
	}

	ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
	if (!Resolve_CuePreviewSample(Timing, 0.f, Sample))
		return false;

	double fEffectWallEnd = static_cast<double>(Sample.fCueWallStartSeconds) +
		static_cast<double>(fEffectDurationSeconds) /
			static_cast<double>(Timing.fPlayRate);
	if (Timing.bHasCueSourceEnd)
	{
		fEffectWallEnd = (std::min)(fEffectWallEnd,
			static_cast<double>(Sample.fCueWallEndSeconds));
	}
	const double fTimelineDuration = (std::max)(
		static_cast<double>(fTimelineDurationSeconds),
		static_cast<double>(fOwningClipTimelineOffsetSeconds) + fEffectWallEnd);
	if (!std::isfinite(fTimelineDuration) ||
		fTimelineDuration > static_cast<double>((std::numeric_limits<float>::max)()))
	{
		return false;
	}

	fOutTimelineDurationSeconds = static_cast<float>(fTimelineDuration);
	return true;
}

bool Client::CActionPresentationTimeline::Should_ReleaseCompletedAnimationClock(
	const bool bHasExplicitLoopPolicy,
	const bool bLoop,
	const bool bLastClip,
	const bool bAuthoredEndPoseHold,
	const bool bAnimationPaused,
	const float fCurrentSourceSeconds,
	const float fSourceDurationSeconds,
	const float fCurrentClipWallSeconds,
	const float fAuthoredClipWallDurationSeconds)
{
	if (!bHasExplicitLoopPolicy || !bAnimationPaused ||
		!std::isfinite(fCurrentSourceSeconds) ||
		!std::isfinite(fSourceDurationSeconds) || fSourceDurationSeconds <= 0.f)
	{
		return false;
	}
	if (bLoop)
	{
		/* LOOP_TO_STAGE_END can stop partway through its final source epoch.
		   Only the final finite wall boundary releases a natural Effect tail. */
		return bLastClip && std::isfinite(fCurrentClipWallSeconds) &&
			fCurrentClipWallSeconds >= 0.f &&
			std::isfinite(fAuthoredClipWallDurationSeconds) &&
			fAuthoredClipWallDurationSeconds > 0.f &&
			fCurrentClipWallSeconds + 0.0001f >= fAuthoredClipWallDurationSeconds;
	}
	return (bLastClip || bAuthoredEndPoseHold) &&
		fCurrentSourceSeconds + 0.0001f >= fSourceDurationSeconds;
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
