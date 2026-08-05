#pragma once

#include "Client_Defines.h"

#include <string>

NS_BEGIN(Client)

struct MAP_LOAD_SCOPE final
{
	bool_t isEnabled = false;
	bool_t includeBackground = false;
	f32_t minimumX = {};
	f32_t minimumZ = {};
	f32_t maximumX = {};
	f32_t maximumZ = {};
	std::string excludedAssetGroupId;

	bool_t Contains(const float3_t& position) const
	{
		return !isEnabled ||
			(position.x >= minimumX && position.x <= maximumX &&
			 position.z >= minimumZ && position.z <= maximumZ);
	}
};

NS_END
