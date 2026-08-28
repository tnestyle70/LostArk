#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>

namespace Client
{

/* A reliable death can outlive network authority only for the admitted
   finite model clip. Repeated snapshots/despawns never restart this clock. */
class CDeathPresentationClock final
{
public:
	bool Start(const bool bHasPlayableClip, const float fDurationSeconds)
	{
		if (m_bStarted)
			return false;
		m_bStarted = true;
		m_bHasPlayableClip = bHasPlayableClip && std::isfinite(fDurationSeconds) &&
			fDurationSeconds > 0.f;
		m_fDurationSeconds = m_bHasPlayableClip ? fDurationSeconds : 0.f;
		return true;
	}
	void Advance(const float fDeltaSeconds)
	{
		if (!m_bStarted || !m_bHasPlayableClip)
			return;
		if (!std::isfinite(fDeltaSeconds) || fDeltaSeconds < 0.f)
		{
			m_bHasPlayableClip = false;
			return;
		}
		m_fElapsedSeconds = static_cast<float>((std::min)(
			static_cast<double>(m_fDurationSeconds),
			static_cast<double>(m_fElapsedSeconds) + fDeltaSeconds));
	}
	bool Has_Started() const { return m_bStarted; }
	bool Is_Complete() const
	{
		return m_bStarted && (!m_bHasPlayableClip ||
			m_fElapsedSeconds >= m_fDurationSeconds);
	}
private:
	bool m_bStarted = false;
	bool m_bHasPlayableClip = false;
	float m_fDurationSeconds = 0.f;
	float m_fElapsedSeconds = 0.f;
};

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
	/* Finite authoring wall budgets may hold a terminal pose or repeat a source
	   clip before advancing. Tool live seek and source-anchor history share
	   this mapping; invalid input leaves OutSample unchanged. */
	static bool Resolve_PreviewSequenceSample(
		std::span<const ACTION_PRESENTATION_CLIP_TIMING> Clips,
		std::span<const float> WallBudgets,
		float fTimelineWallTimeSeconds,
		ACTION_PRESENTATION_SAMPLE& OutSample);
	/* Sequential occurrences may intentionally reuse one model animation. The
	   occurrence identity must still force a restart/seek at their boundary. */
	static bool Requires_ClipOccurrenceTransition(
		std::size_t iCurrentClipOccurrenceIndex,
		std::size_t iExpectedClipOccurrenceIndex,
		uint32_t iCurrentAnimationIndex,
		uint32_t iExpectedAnimationIndex);
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
	/* Convert an Effect-local delay to the complete authoring timeline. The
	   inverse uses Resolve_CuePreviewSample after subtracting the clip offset;
	   native emitter delay remains a separate source-owned value. */
	static bool Resolve_CuePreviewTimelineTime(
		const ACTION_PRESENTATION_CUE_PREVIEW_TIMING& Timing,
		float fOwningClipTimelineOffsetSeconds,
		float fEffectSampleSeconds,
		float& fOutTimelineSeconds);
	/* Keep the full owning timeline and any natural Effect tail. A bounded
	   cue clips its Effect contribution before extending that timeline. */
	static bool Resolve_CuePreviewDuration(
		const ACTION_PRESENTATION_CUE_PREVIEW_TIMING& Timing,
		float fOwningClipTimelineOffsetSeconds,
		float fTimelineDurationSeconds,
		float fEffectDurationSeconds,
		float& fOutTimelineDurationSeconds);
	/* Once an explicit non-loop clip reaches its held end pose, the animation
	   clock can no longer advance.  A final clip releases the natural Effect
	   tail; a non-final clip releases only when its authored wall interval is
	   longer than the playable source window, so the wall clock can finish the
	   hold and select the next occurrence without changing the held pose.
	   A final loop releases only after its explicit finite wall budget ends;
	   zero authored wall keeps legacy and unbounded loops under animation time. */
	static bool Should_ReleaseCompletedAnimationClock(
		bool bHasExplicitLoopPolicy,
		bool bLoop,
		bool bLastClip,
		bool bAuthoredEndPoseHold,
		bool bAnimationPaused,
		float fCurrentSourceSeconds,
		float fSourceDurationSeconds,
		float fCurrentClipWallSeconds = 0.f,
		float fAuthoredClipWallDurationSeconds = 0.f);
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
