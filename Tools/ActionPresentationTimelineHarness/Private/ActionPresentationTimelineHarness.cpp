#include "ActionPresentationTimeline.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <span>

using Client::ACTION_PRESENTATION_CLIP_TIMING;
using Client::ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE;
using Client::ACTION_PRESENTATION_CUE_PREVIEW_TIMING;
using Client::ACTION_PRESENTATION_SAMPLE;
using Client::CActionPresentationTimeline;

namespace
{
	constexpr float EPSILON = 0.00001f;

	bool NearlyEqual(const float Left, const float Right)
	{
		return std::fabs(Left - Right) <= EPSILON;
	}

	bool Require(const bool Condition, const char* pMessage)
	{
		if (Condition)
			return true;
		std::cerr << "ActionPresentationTimelineHarness: "
			<< pMessage << '\n';
		return false;
	}

	bool VerifyAdjacentExplicitSourceWindows()
	{
		const std::array<ACTION_PRESENTATION_CLIP_TIMING, 2u> Clips{
			ACTION_PRESENTATION_CLIP_TIMING{
				0.6f, 300u, 1.f, false, 0.f },
			ACTION_PRESENTATION_CLIP_TIMING{
				0.6f, 300u, 1.f, false, 0.3f }
		};

		float fSourceDuration = 0.f;
		float fWallDuration = 0.f;
		if (!Require(CActionPresentationTimeline::Resolve_ClipDuration(
				Clips[0], fSourceDuration, fWallDuration),
			"first explicit source window did not resolve") ||
			!Require(NearlyEqual(fSourceDuration, 0.3f) &&
				NearlyEqual(fWallDuration, 0.3f),
			"first explicit source window duration changed") ||
			!Require(CActionPresentationTimeline::Resolve_ClipDuration(
				Clips[1], fSourceDuration, fWallDuration),
			"second explicit source window did not resolve") ||
			!Require(NearlyEqual(fSourceDuration, 0.3f) &&
				NearlyEqual(fWallDuration, 0.3f),
			"second explicit source window duration changed"))
		{
			return false;
		}

		ACTION_PRESENTATION_SAMPLE Sample;
		if (!Require(CActionPresentationTimeline::Resolve_Sample(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				0.3f, Sample),
			"shared wall-time boundary did not resolve") ||
			!Require(1u == Sample.iClipIndex &&
				NearlyEqual(Sample.fClipSourceTimeSeconds, 0.3f),
			"shared wall-time boundary did not select the second slice"))
		{
			return false;
		}

		float fCueWallOffset = 0.f;
		if (!Require(CActionPresentationTimeline::Resolve_CueWallOffset(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				0u, 0.299f, 0u, fCueWallOffset),
			"cue inside the first explicit slice was rejected") ||
			!Require(NearlyEqual(fCueWallOffset, 0.299f),
			"first-slice cue wall offset changed") ||
			!Require(!CActionPresentationTimeline::Resolve_CueWallOffset(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				0u, 0.3f, 0u, fCueWallOffset),
			"first explicit slice claimed its half-open end") ||
			!Require(CActionPresentationTimeline::Resolve_CueWallOffset(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				1u, 0.3f, 0u, fCueWallOffset),
			"second explicit slice rejected its inclusive start") ||
			!Require(NearlyEqual(fCueWallOffset, 0.3f),
			"second-slice boundary cue wall offset changed") ||
			!Require(!CActionPresentationTimeline::Resolve_CueWallOffset(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				1u, 0.6f, 0u, fCueWallOffset),
			"second explicit slice claimed its half-open end"))
		{
			return false;
		}

		return true;
	}

	bool VerifyLegacyNaturalEndCompatibility()
	{
		const std::array<ACTION_PRESENTATION_CLIP_TIMING, 1u> Clips{
			ACTION_PRESENTATION_CLIP_TIMING{
				0.6f, 0u, 1.f, false, 0.f }
		};
		float fCueWallOffset = 0.f;
		return Require(CActionPresentationTimeline::Resolve_CueWallOffset(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				0u, 0.6f, 0u, fCueWallOffset),
			"legacy natural clip no longer accepts a cue at model end") &&
			Require(NearlyEqual(fCueWallOffset, 0.6f),
				"legacy natural-end cue wall offset changed");
	}

	bool VerifyProductPreviewClock()
	{
		ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
		Timing.fClipSourceStartSeconds = 0.3f;
		Timing.fPlayRate = 2.f;
		Timing.fCueSourceStartSeconds = 0.4f;
		Timing.fCueSourceEndSeconds = 0.5f;
		Timing.bHasCueSourceEnd = true;

		ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
		if (!Require(CActionPresentationTimeline::Resolve_CuePreviewSample(
				Timing, 0.075f, Sample),
			"trimmed Product preview clock did not resolve") ||
			!Require(NearlyEqual(Sample.fCueWallStartSeconds, 0.05f),
				"Product cue wall start ignored sourceStart/playRate") ||
			!Require(NearlyEqual(Sample.fCueWallEndSeconds, 0.1f),
				"Product cue wall end ignored sourceStart/playRate") ||
			!Require(NearlyEqual(Sample.fEffectSampleSeconds, 0.05f),
				"Product effect sample clock ignored playRate") ||
			!Require(Sample.bVisible,
				"Product cue was hidden inside its wall-time window"))
		{
			return false;
		}

		if (!Require(CActionPresentationTimeline::Resolve_CuePreviewSample(
				Timing, 0.1f, Sample),
			"Product preview end boundary did not resolve") ||
			!Require(!Sample.bVisible,
				"Product cue claimed its half-open wall-time end"))
		{
			return false;
		}

		return true;
	}

	bool VerifyNaturalProductPreviewDurationFloor()
	{
		const ACTION_PRESENTATION_CLIP_TIMING Willowrend{
			2.767f, 0u, 1.f, false, 0.f };
		float fSourceDuration = 0.f;
		float fClipWallDuration = 0.f;
		if (!Require(CActionPresentationTimeline::Resolve_ClipDuration(
				Willowrend, fSourceDuration, fClipWallDuration),
			"natural Product clip duration floor did not resolve") ||
			!Require(NearlyEqual(fSourceDuration, 2.767f) &&
				NearlyEqual(fClipWallDuration, 2.767f),
			"natural Product clip duration floor lost the full source clip"))
		{
			return false;
		}

		const float fShortOccurrencePreview = 1.3f;
		const float fPreviewDuration = (std::max)(
			fShortOccurrencePreview, fClipWallDuration);
		return Require(NearlyEqual(fPreviewDuration, 2.767f),
			"short Product occurrence truncated the full animation preview");
	}
}

int main()
{
	if (!VerifyAdjacentExplicitSourceWindows() ||
		!VerifyLegacyNaturalEndCompatibility() ||
		!VerifyProductPreviewClock() ||
		!VerifyNaturalProductPreviewDurationFloor())
	{
		return 1;
	}

	std::cout << "ActionPresentationTimelineHarness: PASS\n";
	return 0;
}
