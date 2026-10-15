#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>

namespace Client
{

// Valtan 사망 연출의 유한한 모델 clip 시계다. 네트워크 객체 정리 뒤에도 남는 표시 시간을 관리하며 반복 Start는 시계를 재시작하지 않는다.
class CDeathPresentationClock final
{
public:
	// 첫 호출은 playable 여부와 길이를 고정하고 true를 반환한다. 재호출 false는 기존 상태를 그대로 둔다.
	// clip이 없거나 길이가 잘못되면 시작 상태는 남기되 즉시 완료로 판단한다.
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
	// 표시 경과 시간을 기간 끝으로 제한한다. 잘못된 delta는 playable 상태를 해제하여 완료로 처리한다.
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
	bool Has_Started() const
	{
		return m_bStarted;
	}
	// 아직 Start되지 않았다면 false다. 재생할 clip이 없거나 유한 길이에 도달하면 true다.
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

// 모델 전체 길이와 시작 offset은 원본 초, iPlayMs는 원본 구간 ms, fPlayRate는 원본 시간/경과 시간 비율이다.
// 목록은 호출자가 소유하고 각 함수는 span을 호출 중에만 차용한다.
struct ACTION_PRESENTATION_CLIP_TIMING final
{
	float fModelSourceDurationSeconds = 0.f;
	uint32_t iPlayMs = 0u;
	float fPlayRate = 1.f;
	bool bLoop = false;
	// 원본 track에서 시작하는 절대 시간(초)이다. 기존 aggregate 초기화 순서와 기본값 0을 유지하도록 마지막 필드에 둔다.
	float fSourceStartSeconds = 0.f;
};

// 선택한 occurrence index, 모델에 적용할 절대 원본 시각, stage 경과 시간과 반복 회차를 값으로 반환한다.
struct ACTION_PRESENTATION_SAMPLE final
{
	std::size_t iClipIndex = 0u;
	float fClipSourceTimeSeconds = 0.f;
	float fStageWallTimeSeconds = 0.f;
	uint64_t iLoopEpoch = 0u;
};

// cue는 clip 원본 시각으로 저작하고 Effect Tool scrubber는 경과 시간으로 움직인다.
// 이 입력은 두 시계의 차이를 play rate로 변환하며 모델이나 Effect 인스턴스를 소유하지 않는다.
struct ACTION_PRESENTATION_CUE_PREVIEW_TIMING final
{
	float fClipSourceStartSeconds = 0.f;
	float fPlayRate = 1.f;
	float fCueSourceStartSeconds = 0.f;
	float fCueSourceEndSeconds = 0.f;
	bool bHasCueSourceEnd = false;
};

// cue의 경과 시간 시작/끝과 Effect 원본 sample 시각이다. true 반환과 실제 보임(bVisible)은 별개다.
struct ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE final
{
	float fCueWallStartSeconds = 0.f;
	float fCueWallEndSeconds = 0.f;
	float fEffectSampleSeconds = 0.f;
	bool bVisible = false;
};

// Character/Valtan 표시와 저작 Tool이 공유하는 시간 변환이다. 전달된 stage 경과 시간을 clip 원본 시각으로 바꾸며 Server 권위를 수정하지 않는다.
class CActionPresentationTimeline final
{
public:
	// 저작 검증은 모델 ticks/rate로 남은 원본 구간을 계산한다. runtime의 방어적 clamp와 달리 명시 playMs가 반올림한 잔여 ms를 넘으면 거부한다.
	// 출력 잔여 ms는 처음 0으로 초기화되며 playMs 범위 실패에서는 계산값이 남을 수 있다.
	static bool Validate_AuthoredSourceWindow(
		float fNativeDurationTicks,
		float fNativeTicksPerSecond,
		uint32_t iSourceStartMs,
		uint32_t iPlayMs,
		float fPlayRate,
		uint32_t& iOutRoundedRemainingMs);
	// 원본 잔여 길이를 iPlayMs로 제한하고 play rate로 나눠 경과 길이를 구한다. 출력 두 값은 먼저 0으로 초기화한다.
	static bool Resolve_ClipDuration(
		const ACTION_PRESENTATION_CLIP_TIMING& Clip,
		float& fOutSourceDurationSeconds,
		float& fOutWallDurationSeconds);
	// Character 등이 요청한 stage 경과 시각을 순차 clip에 대응시킨다. loop는 남은 stage를 차지하고 마지막 non-loop는 끝 포즈를 유지한다.
	// false는 OutSample을 보존하며 선택 지점 뒤의 clip까지 미리 검증하지는 않는다.
	static bool Resolve_Sample(
		std::span<const ACTION_PRESENTATION_CLIP_TIMING> Clips,
		float fStageWallTimeSeconds,
		ACTION_PRESENTATION_SAMPLE& OutSample);
	// 유한한 WallBudgets 동안 끝 포즈를 유지하거나 clip을 반복한 뒤 다음 항목으로 진행한다.
	// Tool seek가 사용하며 입력 실패 시 OutSample은 유지한다.
	static bool Resolve_PreviewSequenceSample(
		std::span<const ACTION_PRESENTATION_CLIP_TIMING> Clips,
		std::span<const float> WallBudgets,
		float fTimelineWallTimeSeconds,
		ACTION_PRESENTATION_SAMPLE& OutSample);
	// 같은 모델 animation을 연속 사용해도 occurrence가 달라지면 다시 시작/seek해야 한다. 두 index 중 하나라도 다르면 true다.
	static bool Requires_ClipOccurrenceTransition(
		std::size_t iCurrentClipOccurrenceIndex,
		std::size_t iExpectedClipOccurrenceIndex,
		uint32_t iCurrentAnimationIndex,
		uint32_t iExpectedAnimationIndex);
	// cue 원본 시각에 도달하는 stage 경과 시각을 구한다. 출력은 먼저 0으로 지우며 앞선 loop 뒤의 clip은 도달 불가로 거부한다.
	static bool Resolve_CueWallOffset(
		std::span<const ACTION_PRESENTATION_CLIP_TIMING> Clips,
		std::size_t iClipIndex,
		float fCueSourceTimeSeconds,
		uint64_t iLoopEpoch,
		float& fOutStageWallTimeSeconds);
	// 명시 playMs 구간의 cue 시작은 끝점을 제외하고, 유한 cue 종료는 자기 구간 끝점까지 허용한다.
	// playMs가 0인 원래 전체 구간은 시작 판정도 끝점 오차 범위를 허용한다.
	static bool Resolve_CueEndWallOffset(
		std::span<const ACTION_PRESENTATION_CLIP_TIMING> Clips,
		std::size_t iClipIndex,
		float fCueSourceTimeSeconds,
		uint64_t iLoopEpoch,
		float& fOutStageWallTimeSeconds);
	// Effect Tool의 clip 기준 경과 시간을 cue 기준 Effect sample로 바꾼다. 출력은 먼저 초기화하며 구간 밖이어도 유효한 변환이면 true다.
	static bool Resolve_CuePreviewSample(
		const ACTION_PRESENTATION_CUE_PREVIEW_TIMING& Timing,
		float fTimelineWallSeconds,
		ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE& OutSample);
	// Effect 내부 sample 시각을 clip의 전체 timeline offset과 cue 시작 시각에 더한다.
	// 역변환은 clip offset을 뺀 뒤 Resolve_CuePreviewSample을 쓰며 emitter 자체 delay와는 별도다.
	static bool Resolve_CuePreviewTimelineTime(
		const ACTION_PRESENTATION_CUE_PREVIEW_TIMING& Timing,
		float fOwningClipTimelineOffsetSeconds,
		float fEffectSampleSeconds,
		float& fOutTimelineSeconds);
	// 기존 전체 timeline 길이를 줄이지 않고 자연 Effect tail이 더 길면 확장한다. 종료가 정해진 cue는 자기 끝에서 tail 기여를 자른다.
	static bool Resolve_CuePreviewDuration(
		const ACTION_PRESENTATION_CUE_PREVIEW_TIMING& Timing,
		float fOwningClipTimelineOffsetSeconds,
		float fTimelineDurationSeconds,
		float fEffectDurationSeconds,
		float& fOutTimelineDurationSeconds);
	// 명시 loop 정책과 paused 상태를 확인해 Tool이 모델 시계 의존을 끝내도 되는지 판단한다.
	// non-loop는 마지막 clip 또는 호출자가 확인한 끝 포즈 hold에서 원본 끝에 도달해야 한다.
	// loop는 마지막 clip의 명시 유한 wall 기간이 끝나야 하며, 실제 모델이나 Tool 시계를 이 함수가 직접 바꾸지는 않는다.
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
	// replication이 받은 tick과 action 시작 tick의 차이를 초로 바꾼다. wrap의 예약 tick 0은 건너뛰며 실패 전 출력은 0으로 초기화한다.
	static bool Try_ResolveActionAgeSeconds(
		uint32_t iServerTick,
		uint32_t iActionStartTick,
		float fServerTickHz,
		float& fOutActionAgeSeconds);
	// 32-bit 순환 tick의 signed 차이가 양수인지 검사한다. 0은 미설정 값이며 반 범위 이상 떨어진 시각을 일반 정수 순서로 비교하지 않는다.
	static bool Is_ForwardTick(
		uint32_t iCandidateTick,
		uint32_t iPreviousTick);
};

}
