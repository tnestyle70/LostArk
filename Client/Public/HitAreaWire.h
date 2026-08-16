#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

struct HIT_AREA_SHAPE
{
	int32_t iAreaType = {};
	int32_t iAreaRange = {};
	int32_t iAreaAngle = {};
	int32_t iAreaHeight = {};
	int32_t iAreaOffsetX = {};
	int32_t iAreaInner = {};
};

class CHitAreaWire final
{
public:
	static void Draw(const float4x4_t& Root, const HIT_AREA_SHAPE& Shape, uint32_t iColorRgba);
};

NS_END
