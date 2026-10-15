#include "AnimationAuthoringBridge.h"

// 현재 선택 대상의 Model·asset 이름·generation을 조회한다.
#include "AnimationTargetService.h"
// 선택 clip의 진행·전체 길이·tick rate·pause를 읽는다.
#include "Model.h"

// track tick을 millisecond 정수로 반올림한다.
#include <cmath>

namespace
{
	constexpr f32_t DEFAULT_TICK_RATE = 30.f;

	// track tick×1000/tickRate를 반올림한다. rate가 0 이하면 30Hz를 사용한다.
	int32_t Track_ToMilliseconds(const f32_t trackPosition, f32_t tickRate)
	{
		if (tickRate <= 0.f)
			tickRate = DEFAULT_TICK_RATE;

		return static_cast<int32_t>(
			std::lround(trackPosition * 1000.f / tickRate));
	}
}

// Model 공유 참조는 조회 동안만 보유한다. 재생 상태를 변경하지 않고 값으로 반환한다.
bool_t Client::CAnimationAuthoringBridge::Try_GetPlayheadSnapshot(
	ANIMATION_PLAYHEAD_SNAPSHOT& outSnapshot,
	std::string& outError)
{
	outSnapshot = {};
	outError.clear();

	const shared_ptr<Engine::CModel> model =
		CAnimationTargetService::Resolve_Model();
	const std::string animationAssetId =
		CAnimationTargetService::Resolve_AssetName();
	if (nullptr == model || animationAssetId.empty())
	{
		outError = "Animation target is not available.";
		return false;
	}

	const uint32_t animationIndex = model->Get_CurrentAnimIndex();
	const char_t* clipName = model->Get_AnimationName(animationIndex);
	f32_t trackPosition = {};
	f32_t trackDuration = {};
	if (nullptr == clipName ||
		!model->Get_AnimationProgress(
			animationIndex, trackPosition, trackDuration) ||
		trackDuration < 0.f)
	{
		outError = "Animation playhead is not available.";
		return false;
	}

	const f32_t tickRate =
		model->Get_AnimationTickPerSecond(animationIndex);
	outSnapshot.targetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	outSnapshot.animationAssetId = animationAssetId;
	outSnapshot.clipName = clipName;
	outSnapshot.timeMs = Track_ToMilliseconds(trackPosition, tickRate);
	outSnapshot.durationMs = Track_ToMilliseconds(trackDuration, tickRate);
	outSnapshot.isPaused = model->Is_AnimPaused();
	return true;
}
