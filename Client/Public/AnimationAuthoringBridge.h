#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>

NS_BEGIN(Client)

struct ANIMATION_PLAYHEAD_SNAPSHOT final
{
	uint64_t targetGeneration = {};
	std::string animationAssetId;
	std::string clipName;
	int32_t timeMs = {};
	int32_t durationMs = {};
	bool_t isPaused = false;
};

class CAnimationAuthoringBridge final
{
public:
	static bool_t Try_GetPlayheadSnapshot(
		ANIMATION_PLAYHEAD_SNAPSHOT& outSnapshot,
		std::string& outError);

private:
	CAnimationAuthoringBridge() = delete;
};

NS_END
