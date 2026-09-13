#pragma once

#include <cstdint>

namespace Engine
{
    enum class LIGHT_RECEIVER : uint32_t { ALL = 0u, SOURCE_CHARACTER = 1u, UNBAKED = 2u };
    struct tagEngineDesc;
    using ENGINE_DESC = tagEngineDesc;
    struct RENDER_ENVIRONMENT_STATE;
    struct tagLightDesc;
    using LIGHT_DESC = tagLightDesc;
    struct tagShadowLightDesc;
    using SHADOW_LIGHT_DESC = tagShadowLightDesc;
    struct tagRenderQualitySettings;
    using RENDER_QUALITY_SETTINGS = tagRenderQualitySettings;
    struct MATERIAL_RENDER_SETTINGS;
    struct tagHeightFogSettings;
    using HEIGHT_FOG_SETTINGS = tagHeightFogSettings;
}
