#include "ActionPresentationTimeline.h"

#include <algorithm>
#include <cmath>
#include <limits>

// 저작 Tool이 모델의 ticks/rate를 전달해 원본 시작/재생 구간을 검증한다.
// play rate는 양의 유한 값인지 확인하지만 iPlayMs는 원본 시각이므로 잔여 ms 계산에서 rate로 나누지 않는다.
bool Client::CActionPresentationTimeline::Validate_AuthoredSourceWindow(
	const float fNativeDurationTicks,
	const float fNativeTicksPerSecond,
	const uint32_t iSourceStartMs,
	const uint32_t iPlayMs,
	const float fPlayRate,
	uint32_t& iOutRoundedRemainingMs)
{
	iOutRoundedRemainingMs = 0u;
	if (!std::isfinite(fNativeDurationTicks) || fNativeDurationTicks <= 0.f ||
		!std::isfinite(fNativeTicksPerSecond) || fNativeTicksPerSecond <= 0.f ||
		!std::isfinite(fPlayRate) || fPlayRate <= 0.f)
	{
		return false;
	}
	const double fNativeDurationMs =
		static_cast<double>(fNativeDurationTicks) /
		static_cast<double>(fNativeTicksPerSecond) * 1000.0;
	if (!std::isfinite(fNativeDurationMs) || fNativeDurationMs <= 0.0 ||
		static_cast<double>(iSourceStartMs) >= fNativeDurationMs)
	{
		return false;
	}
	const double fRemainingMs =
		fNativeDurationMs - static_cast<double>(iSourceStartMs);
	const double fRoundedRemainingMs = std::floor(fRemainingMs + 0.5);
	if (!std::isfinite(fRoundedRemainingMs) || fRoundedRemainingMs < 0.0 ||
		fRoundedRemainingMs >
			static_cast<double>((std::numeric_limits<uint32_t>::max)()))
	{
		return false;
	}
	// 정수 ms 계약에 맞춰 남은 길이를 가장 가까운 ms로 반올림한다. 명시 길이가 넘으면 false지만 이 출력 계산값은 유지된다.
	iOutRoundedRemainingMs = static_cast<uint32_t>(fRoundedRemainingMs);
	return 0u == iPlayMs || iPlayMs <= iOutRoundedRemainingMs;
}

// Character와 Tool이 공유하는 원본/경과 길이 계산이다. start 이후 잔여 원본을 명시 playMs로 clamp하고 play rate로 나눈다.
// 출력은 시작 때 0으로 지우지만 마지막 유효성 검사에서 실패하면 계산된 값이 남을 수 있다.
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

// 원본 길이를 경과 길이로 바꿔 앞 clip 시간을 차례로 뺀다. 성공 지점에서만 OutSample을 쓰므로 false는 기존 sample을 보존한다.
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
		// 처음 도달한 loop가 이후 시간을 모두 소비한다. 반복 회차와 나머지 시간을 분리해 원본 offset을 더한다.
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

		// 정확히 clip 끝인 시각은 다음 순차 clip에 속한다. seek와 cue가 같은 경계를 사용하도록 한다.
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

// Effect Tool의 유한 저작 목록을 위해 모든 clip/budget을 먼저 검사한다. 기본 runtime loop와 달리 budget 끝에서는 다음 occurrence로 간다.
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
		// ms 경계를 float 초로 바꾼 오차를 고려해 다음 구간 진입에 1마이크로초 여유를 둔다.
		if (i + 1u < Clips.size() && Remaining + 0.000001f >= WallBudgets[i])
		{
			Remaining = (std::max)(0.f, Remaining - WallBudgets[i]);
			continue;
		}
		// 마지막 budget 이후는 경계에 붙이고 loop 끝은 1마이크로초 앞을 표본화해 다음 반복의 시작 포즈로 튀지 않게 한다.
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

// animation index가 같아도 occurrence index가 바뀌면 경계를 넘은 것이다. 모델 재시작/seek는 호출자가 수행한다.
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
	// cue 시작과 종료가 원본 구간 끝점을 공유할 때 사용하는 서로 다른 경계 정책이다.
	enum class CUE_SOURCE_BOUNDARY
	{
		START_HALF_OPEN,
		END_INCLUSIVE
	};

	// 앞선 non-loop 길이 합 + 현재 반복 회차 길이 + cue 원본 offset/rate를 경과 시각으로 계산한다.
	// 출력은 먼저 0으로 초기화하고 모든 검사 후에만 계산값을 대입한다.
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
				// 이 변환에서 loop는 남은 stage 시간을 차지한다. 따라서 같은 순차 목록에서 loop 뒤 clip의 cue는 도달 불가다.
				if (Clips[iClip].bLoop)
					return false;
				fWallOffset += fWallDurationSeconds;
				continue;
			}

			const float fSourceStartSeconds =
				Clips[iClip].fSourceStartSeconds;
			const float fSourceEndSeconds =
				fSourceStartSeconds + fSourceDurationSeconds;
			// 명시 원본 구간은 시작 cue에서 끝점을 제외한다. 종료 cue는 자기 구간 끝점과 1마이크로초 오차를 허용한다.
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

// Character와 cue 문서 검증이 쓰는 시작 시각 변환이다. 명시 원본 slice의 끝은 다음 slice가 소유한다.
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

// 유한 cue 종료가 자기 원본 slice의 끝과 같을 수 있도록 종료 경계 정책을 사용한다.
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

// Effect Tool의 clip 기준 경과 시간에서 cue 시작을 뺀 뒤 play rate를 곱해 Effect sample 시각을 얻는다.
// 출력은 호출 시작에 비우며 false에서도 그 초기화는 남는다. bVisible은 시작 포함/끝 제외 구간 검사다.
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
	// cue 이전의 Effect sample은 0으로 제한한다. 유한 종료 이후에도 sample 계산값은 남지만 bVisible은 false가 된다.
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

// Effect sample을 rate로 나누고 owning clip offset과 cue 시작을 더하는 역방향 변환이다.
// 결과가 cue의 보이는 구간 안인지까지 검사하지 않으며 실패 시 출력은 0이다.
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

// 기존 timeline 끝과 Effect tail 끝 중 더 긴 쪽을 선택한다. 유한 cue는 tail 기여를 자기 종료에서 제한한다.
// 입력 검증 실패 시 출력은 0이며 모델/Effect 인스턴스 수명은 바꾸지 않는다.
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

// Effect Tool이 모델 시계 대신 wall 시계를 진행할지 결정하는 조건만 반환한다.
// non-loop의 hold 여부는 호출자가 저작 wall 길이와 실제 clip 길이를 비교해 전달한다.
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
		// LOOP_TO_STAGE_END는 마지막 원본 반복 도중 멈출 수 있다. 마지막 유한 wall 경계 이후에만 자연 Effect tail용 시계 전환을 허용한다.
		return bLastClip && std::isfinite(fCurrentClipWallSeconds) &&
			fCurrentClipWallSeconds >= 0.f &&
			std::isfinite(fAuthoredClipWallDurationSeconds) &&
			fAuthoredClipWallDurationSeconds > 0.f &&
			fCurrentClipWallSeconds + 0.0001f >= fAuthoredClipWallDurationSeconds;
	}
	return (bLastClip || bAuthoredEndPoseHold) &&
		fCurrentSourceSeconds + 0.0001f >= fSourceDurationSeconds;
}

// replication/Character/보스 표시가 승인된 action 시작 tick에서 지난 초를 구한다.
// 미설정 tick이나 역방향 tick은 거부하고 예약값 0을 건너뛴 wrap 차이를 계산한다.
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

// unsigned 차이를 signed로 해석하는 순환 순서 비교다. 같은 tick은 전진이 아니며 previous 0은 첫 유효 tick을 받는다.
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
