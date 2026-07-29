#pragma once

#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

class CWAnimationReader final
{
public:
	bool_t Read(const filesystem::path& animationPath,
		const MODEL_SKELETON_DATA& skeleton,
		MODEL_ANIMATION_DATA& outAnimation,
		MODEL_DECODE_REPORT& outReport) const;
};

NS_END
