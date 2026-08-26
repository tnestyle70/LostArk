#pragma once

#include "Client_Defines.h"

#include <string>

NS_BEGIN(Client)

struct MAP_FRUSTUM_CULLING_POLICY final
{
	bool_t bypass = false;
	bool_t diagnostics = false;
	f32_t baseMargin = 0.f;
	f32_t largeObjectRadiusThreshold = 0.f;
	f32_t largeObjectAbsoluteMargin = 0.f;
	f32_t largeObjectRelativeMargin = 0.f;
	uint32_t rejectHysteresisFrames = 0u;
};

struct MAP_FRUSTUM_RUNTIME_STATE final
{
	bool_t initialized = false;
	bool_t lastFrustumVisible = true;
	uint32_t rejectGraceFrames = 0u;
};

struct MAP_LOAD_SCOPE final
{
	bool_t isEnabled = false;
	bool_t includeBackground = false;
	f32_t minimumX = {};
	f32_t minimumZ = {};
	f32_t maximumX = {};
	f32_t maximumZ = {};
	std::string excludedAssetGroupId;
	MAP_FRUSTUM_CULLING_POLICY frustumCulling{};

	bool_t Contains(const float3_t& position) const
	{
		return !isEnabled ||
			(position.x >= minimumX && position.x <= maximumX &&
			 position.z >= minimumZ && position.z <= maximumZ);
	}
};

NS_END
