#pragma once

#include "Effect_Playback.h"
#include "Engine_RenderTypes.h"

NS_BEGIN(Client)

// Consumes the CPU executor's pre-evaluated light tuple. Source emitter
// transforms, particle locations, and authored shadow flags are not resolved here.
bool_t Try_BuildEffectLightDesc(
	const EFFECT_EVALUATED_LIGHT& Evaluated,
	LIGHT_DESC& OutLight);

// Compatibility entry for map/source callers that explicitly own point lights.
bool_t Try_BuildEffectPointLightDesc(
	const EFFECT_EVALUATED_LIGHT& Evaluated,
	LIGHT_DESC& OutLight);

NS_END
