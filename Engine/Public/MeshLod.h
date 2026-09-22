#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

// Conservative view-space envelope of every submitted instance. It only selects
// geometry detail; visibility and the light-volume shadow pass stay with callers.
struct MESH_SCREEN_LOD_DESC final
{
    float4_t viewBounds = {}; // center XYZ, radius
    float2_t projectionPixels = {}; // abs(projection diagonal) * viewport / 2
    f32_t maximumScale = 0.f;
    f32_t nearPlane = 0.f;
    f32_t maximumPixelError = 0.25f;
    // Optional union of the visible instances' view-space sphere bounds. Unlike
    // an enclosing sphere, lateral batch separation does not expand depth.
    float2_t maximumAbsViewXY = {};
    f32_t minimumViewDepth = 0.f;
    bool_t hasTightViewBounds = false;
};

NS_END
