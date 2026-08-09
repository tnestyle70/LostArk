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
};

struct ACTION_PRESENTATION_SAMPLE final
{
	std::size_t iClipIndex = 0u;
	float fClipSourceTimeSeconds = 0.f;
	float fStageWallTimeSeconds = 0.f;
	uint64_t iLoopEpoch = 0u;
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
