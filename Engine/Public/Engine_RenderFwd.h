#pragma once

// 공개 광원 수신 범위 enum의 고정 폭 기반 타입을 제공한다.
#include <cstdint>

namespace Engine
{
    // 광원이 영향을 줄 수신자 범위다. Light_Manager가 pass 범위와 대조하므로 기존 수치와 의미를 유지한다.
    enum class LIGHT_RECEIVER : uint32_t
    {
        ALL = 0u,
        SOURCE_CHARACTER = 1u,
        UNBAKED = 2u
    };
    // 포인터/참조만 필요한 소비자가 큰 렌더링 정의를 포함하지 않도록 이름과 기존 별칭을 미리 선언한다.
    // 필드 접근/값 소유에는 Engine_InitTypes 또는 Engine_RenderTypes의 완전한 정의가 필요하다.
    struct tagEngineDesc;
    using ENGINE_DESC = tagEngineDesc;
    struct RENDER_ENVIRONMENT_STATE;
    struct tagLightDesc;
    using LIGHT_DESC = tagLightDesc;
    struct tagShadowLightDesc;
    using SHADOW_LIGHT_DESC = tagShadowLightDesc;
    struct tagRenderQualitySettings;
    using RENDER_QUALITY_SETTINGS = tagRenderQualitySettings;
    struct MAP_VISIBILITY_SETTINGS;
    struct RENDER_OPTIMIZATION_SETTINGS;
    struct MATERIAL_RENDER_SETTINGS;
    struct tagHeightFogSettings;
    using HEIGHT_FOG_SETTINGS = tagHeightFogSettings;
}
