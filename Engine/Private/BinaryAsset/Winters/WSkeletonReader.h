#pragma once

#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

class CWSkeletonReader final
{
public:
	bool_t Read(const filesystem::path& skeletonPath,
		MODEL_SKELETON_DATA& outSkeleton,
		MODEL_DECODE_REPORT& outReport) const;
};

NS_END
