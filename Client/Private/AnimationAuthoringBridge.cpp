#include "AnimationAuthoringBridge.h"

#include "AnimationTargetService.h"
#include "Model.h"

#include <cmath>

namespace
{
	constexpr f32_t DEFAULT_TICK_RATE = 30.f;

	int32_t Track_ToMilliseconds(const f32_t trackPosition, f32_t tickRate)
	{
		if (tickRate <= 0.f)
			tickRate = DEFAULT_TICK_RATE;

		return static_cast<int32_t>(
			std::lround(trackPosition * 1000.f / tickRate));
	}
}

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
