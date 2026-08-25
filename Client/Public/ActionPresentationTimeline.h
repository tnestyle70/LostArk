#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace Client
{

struct ACTION_PRESENTATION_CLIP_TIMING final
{
	float fModelSourceDurationSeconds = 0.f;
	uint32_t iPlayMs = 0u;
	float fPlayRate = 1.f;
	bool bLoop = false;
	/* Absolute source-track offset.  It is last to keep existing Character
	aggregate initializers source-compatible; their default remains zero. */
	float fSourceStartSeconds = 0.f;
};

struct ACTION_PRESENTATION_SAMPLE final
{
	std::size_t iClipIndex = 0u;
	float fClipSourceTimeSeconds = 0.f;
	float fStageWallTimeSeconds = 0.f;
	uint64_t iLoopEpoch = 0u;
};

/* A selected Product cue is authored in the clip's source clock while the
   Effect Tool scrubber runs in wall time.  Keeping this conversion pure makes
   boss authoring preview use the same play-rate contract as runtime. */
struct ACTION_PRESENTATION_CUE_PREVIEW_TIMING final
{
	float fClipSourceStartSeconds = 0.f;
	float fPlayRate = 1.f;
	float fCueSourceStartSeconds = 0.f;
	float fCueSourceEndSeconds = 0.f;
	bool bHasCueSourceEnd = false;
};

struct ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE final
{
	float fCueWallStartSeconds = 0.f;
	float fCueWallEndSeconds = 0.f;
	float fEffectSampleSeconds = 0.f;
	bool bVisible = false;
};

/* Pure wall/source-clock conversion shared by Character presentation and the
runtime harness. Network authority stays in the Server; this helper only maps
an approved stage age onto its authored sequential clip timeline. */
class CActionPresentationTimeline final
{
public:
	static bool Resolve_ClipDuration(
		const ACTION_PRESENTATION_CLIP_TIMING& Clip,
		float& fOutSourceDurationSeconds,
		float& fOutWallDurationSeconds);
	static bool Resolve_Sample(
		std::span<const ACTION_PRESENTATION_CLIP_TIMING> Clips,
		float fStageWallTimeSeconds,
		ACTION_PRESENTATION_SAMPLE& OutSample);
	static bool Resolve_CueWallOffset(
		std::span<const ACTION_PRESENTATION_CLIP_TIMING> Clips,
		std::size_t iClipIndex,
		float fCueSourceTimeSeconds,
		uint64_t iLoopEpoch,
		float& fOutStageWallTimeSeconds);
	/* Cue starts own a half-open source slice, while a finite cue end may
	   coincide with that slice's authored end boundary. */
	static bool Resolve_CueEndWallOffset(
		std::span<const ACTION_PRESENTATION_CLIP_TIMING> Clips,
		std::size_t iClipIndex,
		float fCueSourceTimeSeconds,
		uint64_t iLoopEpoch,
		float& fOutStageWallTimeSeconds);
	static bool Resolve_CuePreviewSample(
		const ACTION_PRESENTATION_CUE_PREVIEW_TIMING& Timing,
		float fTimelineWallSeconds,
		ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE& OutSample);
	/* Once a final explicit non-loop clip reaches its held end pose, the
	   animation clock can no longer advance a natural Effect tail.  The
	   authoring preview hands ownership back to its wall clock at that exact
	   boundary while retaining the final animation pose. */
	static bool Should_ReleaseCompletedAnimationClock(
		bool bHasExplicitLoopPolicy,
		bool bLoop,
		bool bLastClip,
		bool bAnimationPaused,
		float fCurrentSourceSeconds,
		float fSourceDurationSeconds);
	static bool Try_ResolveActionAgeSeconds(
		uint32_t iServerTick,
		uint32_t iActionStartTick,
		float fServerTickHz,
		float& fOutActionAgeSeconds);
	static bool Is_ForwardTick(
		uint32_t iCandidateTick,
		uint32_t iPreviousTick);
};

}
