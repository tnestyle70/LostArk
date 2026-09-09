#pragma once
// Warlord A/S/F/V native material programs with exact named source inputs.
// Runtime scene, depth and vertex-factory adapters are explicit in HLSL.
#include "Effect_AuthoringDocument.h"
#include <array>
#include <span>
#include <string_view>
#include <algorithm>
#include <cmath>

NS_BEGIN(Client)
struct WARLORD_NATIVE_PARAMETER_DESC final
{
    std::string_view strName;
    uint32_t iRow;
    uint32_t iLane;
    bool bVector;
};
struct WARLORD_NATIVE_SWITCH_DESC final
{
    std::string_view strName;
    bool bValue;
};
struct WARLORD_NATIVE_PROGRAM_DESC final
{
    uint32_t iProfileIndex;
    std::string_view strRuntimeProfileId;
    std::string_view strSourceMaterialPath;
    std::string_view strParentMaterialPath;
    std::string_view strProfileId;
    bool bMesh;
    std::string_view strRendererShape;
    bool bNeedsSceneColor;
    bool bNeedsDepthSample;
    bool bNeedsTangentView;
    bool bDynamicVertexFactory;
    EFFECT_RENDER_PROFILE eRenderProfile;
    std::span<const std::string_view> TextureNames;
    std::span<const WARLORD_NATIVE_PARAMETER_DESC> Parameters;
    std::span<const WARLORD_NATIVE_SWITCH_DESC> StaticSwitches;
};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_400 = {{"noise_tex","emissive_tex","dissolve_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,2> WARLORD_NATIVE_PARAMETERS_400 = {{
    {"selectioncolor", 1u, 0u, true},
    {"22", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,1> WARLORD_NATIVE_SWITCHES_400 = {{
    {"use_noalpha", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_401 = {{"native_texture_0","native_texture_1"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_401 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_401 = {{
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_402 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_402 = {{
    {"selectioncolor", 1u, 0u, true},
    {"uv_scale", 0u, 3u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 1u, false},
    {"emissive_power", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_402 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"non_alpha_chanel", false},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_403 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,8> WARLORD_NATIVE_PARAMETERS_403 = {{
    {"selectioncolor", 2u, 0u, true},
    {"uv_scale", 1u, 2u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 1u, 0u, false},
    {"edge_intensity", 0u, 2u, false},
    {"edge_power", 0u, 3u, false},
    {"emissive_power", 1u, 1u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_403 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", true},
    {"non_alpha_chanel", false},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_404 = {{"04.map_anew","00.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,8> WARLORD_NATIVE_PARAMETERS_404 = {{
    {"selectioncolor", 2u, 0u, true},
    {"09.map_a_panning_x", 0u, 3u, false},
    {"05.map_alod", 0u, 0u, false},
    {"07.map_a_uvscale_r", 0u, 1u, false},
    {"08.map_a_uvscale_g", 0u, 2u, false},
    {"11.desaturation", 1u, 0u, false},
    {"11.map_a_str", 1u, 1u, false},
    {"12.map_a_power", 1u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_404 = {{
    {"00.usedistortion", false},
    {"20.use_centrehole", false},
    {"00.usedepthbiasalpha", false},
    {"30.usefresnal", false},
    {"10.cameradistance", false},
    {"01.mapch.r", true},
    {"02.mapch.g", true},
    {"03.mapch.b", true},
    {"00.use_uvdistort", false},
    {"01.use_uvdistort_all", false},
    {"checkismeshemitvc", false},
    {"00.use_rgbinalphamap", true},
    {"06.use((r+g+b)*0.33)", true},
    {"15.use_panbezier", true},
    {"01.usecoordinate_index1", false},
    {"30.use_additionalmap", false},
    {"useupno_alpha", false},
    {"40.use_fanshape", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_405 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_405 = {{
    {"selectioncolor", 2u, 0u, true},
    {"uv_scale", 1u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_405 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"use_distortion_floor", false},
    {"non_alpha_chanel", false},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex_02", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_406 = {{"subuv_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_406 = {{
    {"selectioncolor", 2u, 0u, true},
    {"emissive_color&intensity", 1u, 0u, true},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_tex_desturation", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_406 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", true},
    {"use_nonalpha_tex", false},
    {"use_emissive_vertcolor", false},
    {"use_nonuvnoise", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_407 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,8> WARLORD_NATIVE_PARAMETERS_407 = {{
    {"selectioncolor", 3u, 0u, true},
    {"meshemitterdynamicparameter", 2u, 0u, true},
    {"uv_scale", 1u, 1u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 3u, false},
    {"edge_intensity", 0u, 1u, false},
    {"edge_power", 0u, 2u, false},
    {"emissive_power", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_407 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", true},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", true},
    {"non_alpha_chanel", false},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_408 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,19> WARLORD_NATIVE_PARAMETERS_408 = {{
    {"selectioncolor", 6u, 0u, true},
    {"noisecolor", 5u, 0u, true},
    {"15.map_d_uvscale_r", 2u, 3u, false},
    {"16.map_d_uvscale_g", 3u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 2u, false},
    {"03.map_a_uvscale_g", 1u, 0u, false},
    {"04.map_a_panning_x", 1u, 2u, false},
    {"05.map_a_panning_y", 2u, 0u, false},
    {"02.map_b_uvscale_r", 0u, 3u, false},
    {"03.map_b_uvscale_g", 1u, 1u, false},
    {"04.map_b_panning_x", 1u, 3u, false},
    {"05.map_b_panning_y", 2u, 1u, false},
    {"power", 3u, 3u, false},
    {"13.distortstr", 2u, 2u, false},
    {"00.noisepower", 0u, 0u, false},
    {"00.noisestr", 0u, 1u, false},
    {"biasvalue", 3u, 1u, false},
    {"str", 4u, 0u, false},
    {"depthbiasdalpha_bias", 3u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,6> WARLORD_NATIVE_SWITCHES_408 = {{
    {"checkisdepthbiasalpha", true},
    {"00.checkisnoiseon", true},
    {"12.uvnoise", true},
    {"11.uvmirroring", false},
    {"use_meshtype", false},
    {"use_multyply_noisecolor", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_409 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,8> WARLORD_NATIVE_PARAMETERS_409 = {{
    {"selectioncolor", 3u, 0u, true},
    {"meshemitterdynamicparameter", 2u, 0u, true},
    {"uv_scale", 1u, 1u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 3u, false},
    {"edge_intensity", 0u, 1u, false},
    {"edge_power", 0u, 2u, false},
    {"emissive_power", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_409 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", true},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", true},
    {"non_alpha_chanel", false},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_410 = {{"dissolve_noise_tex","emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,12> WARLORD_NATIVE_PARAMETERS_410 = {{
    {"selectioncolor", 5u, 0u, true},
    {"meshemitterdynamicparameter", 4u, 0u, true},
    {"dissolve_color&intensity", 3u, 0u, true},
    {"dissolve_noise_intensity", 0u, 0u, false},
    {"dissolve_noise_power", 0u, 1u, false},
    {"dissolve_power", 0u, 2u, false},
    {"uv_scale", 2u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 3u, false},
    {"emissive_desaturation", 1u, 2u, false},
    {"edge_intensity", 1u, 0u, false},
    {"edge_power", 1u, 1u, false},
    {"emissive_power", 1u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,14> WARLORD_NATIVE_SWITCHES_410 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_lamp_alpha", false},
    {"use_dissolve", true},
    {"use_meshtype", true},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"non_alpha_chanel", false},
    {"use_uv_noise_tex", false},
    {"use_uv_noise_tex_02", false},
    {"use_fresnel_alpha", false},
    {"use_edge_glow", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_411 = {{"rampshape_texture","uvnoise_texture","noise_texture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,7> WARLORD_NATIVE_PARAMETERS_411 = {{
    {"selectioncolor", 3u, 0u, true},
    {"uvnoise_panspeed", 0u, 3u, false},
    {"meshemitterdynamicparameter", 2u, 0u, true},
    {"ramp_str", 0u, 2u, false},
    {"noise_size", 0u, 0u, false},
    {"uvnoise_str", 1u, 0u, false},
    {"noise_str", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,9> WARLORD_NATIVE_SWITCHES_411 = {{
    {"use_polarcoordinate", false},
    {"use_uvnoise", true},
    {"use_mesh", true},
    {"use_rchannel", true},
    {"use_gchannel", true},
    {"usequadbaseimageuv", false},
    {"use_bchannel", true},
    {"use_achannel", true},
    {"useupperemissive", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_412 = {{"subuv_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_412 = {{
    {"selectioncolor", 1u, 0u, true},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"distortion_intensity", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_412 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", true},
    {"use_meshtype", false},
    {"use_subuv_tex", true},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"use_distortion_floor", false},
    {"non_alpha_chanel", true},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex_02", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_413 = {{"subuv_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_413 = {{
    {"selectioncolor", 2u, 0u, true},
    {"emissive_color&intensity", 1u, 0u, true},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_tex_desturation", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_413 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", true},
    {"use_nonalpha_tex", false},
    {"use_emissive_vertcolor", false},
    {"use_nonuvnoise", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_414 = {{"native_texture_0","native_texture_1","native_texture_2"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_414 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_414 = {{
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_415 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_415 = {{
    {"selectioncolor", 1u, 0u, true},
    {"circle radius", 0u, 0u, false},
    {"circleedge hardness", 0u, 1u, false},
    {"opacity", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_415 = {{
    {"use_particle", true},
    {"use_invertedcirclealpha", false},
    {"use_circlealpha", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_416 = {{"subuv_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_416 = {{
    {"selectioncolor", 2u, 0u, true},
    {"emissive_color&intensity", 1u, 0u, true},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_tex_desturation", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_416 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", true},
    {"use_nonalpha_tex", false},
    {"use_emissive_vertcolor", false},
    {"use_nonuvnoise", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_417 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,7> WARLORD_NATIVE_PARAMETERS_417 = {{
    {"selectioncolor", 2u, 0u, true},
    {"01.centerglow_power", 0u, 0u, false},
    {"02.centerglow_str", 0u, 2u, false},
    {"03.outpower", 0u, 3u, false},
    {"91.str", 1u, 0u, false},
    {"92.power", 1u, 1u, false},
    {"01.depthbaisalpha", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,2> WARLORD_NATIVE_SWITCHES_417 = {{
    {"00.use_distortion", false},
    {"00.use_depth_alpha", true},
}};

inline constexpr std::array<std::string_view,9> WARLORD_NATIVE_TEXTURES_418 = {{"native_texture_0","native_texture_1","native_texture_2","native_texture_3","native_texture_4","native_texture_5","native_texture_6","native_texture_7","native_texture_8"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,7> WARLORD_NATIVE_PARAMETERS_418 = {{
    {"selectioncolor", 5u, 0u, true},
    {"meshemitterdynamicparameter", 4u, 0u, true},
    {"31.specularcolor", 3u, 0u, true},
    {"02.envcolor", 2u, 0u, true},
    {"01.duffusecolor", 1u, 0u, true},
    {"worldpositionoffset_uvscale", 0u, 1u, false},
    {"worldpositionoffset_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_418 = {{
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_419 = {{"01.map.n","01.emismap","01.map.d","01.specmap","01.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,30> WARLORD_NATIVE_PARAMETERS_419 = {{
    {"selectioncolor", 11u, 0u, true},
    {"11.normalmap.str", 4u, 3u, false},
    {"03.emap_uv.x.scale", 1u, 0u, false},
    {"04.emap_uv.y.scale", 2u, 0u, false},
    {"19.emissiion_color", 8u, 0u, true},
    {"meshemitterdynamicparameter", 10u, 0u, true},
    {"05.specmap_uvscale.x", 3u, 1u, false},
    {"06.specmap_uvscale.y", 3u, 3u, false},
    {"02.uvscale.x", 0u, 3u, false},
    {"03.uvscale.y", 1u, 3u, false},
    {"93.emissiion_color", 9u, 0u, true},
    {"09.specmap_color", 7u, 0u, true},
    {"05.n.panning.x", 2u, 3u, false},
    {"02.n.uvscale.x", 0u, 1u, false},
    {"03.n.uvscale.y", 1u, 2u, false},
    {"06.n.panning.y", 3u, 2u, false},
    {"15.emissiion_power", 5u, 0u, false},
    {"91.desaturation", 5u, 3u, false},
    {"92.emissiion_power", 6u, 0u, false},
    {"02.specmap_str", 0u, 2u, false},
    {"07.desaturation", 4u, 1u, false},
    {"08.specmap_power", 4u, 2u, false},
    {"05.power", 3u, 0u, false},
    {"06.str", 4u, 0u, false},
    {"04.map_a_panning_x", 2u, 1u, false},
    {"02.map_a_uvscale_r", 0u, 0u, false},
    {"03.map_a_uvscale_g", 1u, 1u, false},
    {"05.map_a_panning_y", 2u, 2u, false},
    {"36.str", 5u, 1u, false},
    {"37.power", 5u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,23> WARLORD_NATIVE_SWITCHES_419 = {{
    {"00.usenormalmap", true},
    {"00.usealphamap", true},
    {"31.mapch.r", false},
    {"32.mapch.g", false},
    {"19.mapa+mapclamp", false},
    {"20.use_clampmap", false},
    {"00.usefakelight", true},
    {"01.invert", false},
    {"00.usediffuse", true},
    {"00.usespecullar", true},
    {"30.dependonfakeli", true},
    {"20.usefresspec", false},
    {"00.useemission", true},
    {"20.usefresnal", false},
    {"00.use_uvdistort", false},
    {"21.invert", false},
    {"use_cascade_vertexcolor", true},
    {"33.mapch.b", false},
    {"11.usemapd.uvscale.dyna", false},
    {"11.usemape.uvscale.dyna", false},
    {"01.usemesh", true},
    {"50.dissolvetype-mosaic", true},
    {"25.use_yclamp", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_420 = {{"detail texture","spread texture","center texture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_420 = {{
    {"selectioncolor", 1u, 0u, true},
    {"scale", 0u, 2u, false},
    {"detail texture tiling", 0u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,12> WARLORD_NATIVE_SWITCHES_420 = {{
    {"use_spread texture", true},
    {"use_detaildistortion", true},
    {"use_centerdistortion", false},
    {"use_meshtype", false},
    {"use_up&down", false},
    {"use_xy axis", false},
    {"use_billboard", false},
    {"use_sharp", false},
    {"use_twirl", false},
    {"use_tworing", false},
    {"use_wavering", false},
    {"use_center texture", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_421 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_421 = {{
    {"selectioncolor", 2u, 0u, true},
    {"uv_scale", 1u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_421 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"use_distortion_floor", false},
    {"non_alpha_chanel", false},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex_02", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_422 = {{"uv_noise_tex","emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,10> WARLORD_NATIVE_PARAMETERS_422 = {{
    {"selectioncolor", 3u, 0u, true},
    {"emissive_color&intensity", 2u, 0u, true},
    {"uv_noise_panning_x", 0u, 3u, false},
    {"uv_noise_panning_y", 1u, 0u, false},
    {"uv_panning_x", 1u, 2u, false},
    {"uv_panning_y", 1u, 3u, false},
    {"uv_noise_tilling", 1u, 1u, false},
    {"uv_noise_intensity", 0u, 2u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_tex_desturation", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_422 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_tex", false},
    {"use_emissive_vertcolor", false},
    {"use_nonuvnoise", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_423 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_423 = {{
    {"selectioncolor", 1u, 0u, true},
    {"uv_scale", 0u, 3u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 1u, false},
    {"emissive_power", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_423 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"use_distortion_floor", false},
    {"non_alpha_chanel", false},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex_02", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_424 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_424 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_424 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_425 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_425 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_425 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_426 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_426 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_426 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_427 = {{"01.map"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,8> WARLORD_NATIVE_PARAMETERS_427 = {{
    {"selectioncolor", 2u, 0u, true},
    {"10.timescale", 0u, 3u, false},
    {"12.desaturation", 1u, 0u, false},
    {"13.emissiion_str", 1u, 1u, false},
    {"15.emissiion_power", 1u, 2u, false},
    {"02.circlepower", 0u, 1u, false},
    {"03.circlestr", 0u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,1> WARLORD_NATIVE_SWITCHES_427 = {{
    {"00.checkisdepthbiasalpha", true},
}};

inline constexpr std::array<std::string_view,6> WARLORD_NATIVE_TEXTURES_428 = {{"native_texture_0","native_texture_1","native_texture_2","sparkle_tex","edgedeco texture01","native_texture_5"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_428 = {{
    {"selectioncolor", 1u, 0u, true},
    {"sparkle_paning", 0u, 1u, false},
    {"sparkle_tiling", 0u, 2u, false},
    {"sparkle_intensity", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_428 = {{
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_429 = {{"map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_429 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_429 = {{
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_430 = {{"native_texture_0","native_texture_1","native_texture_2","native_texture_3"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_430 = {{
    {"selectioncolor", 2u, 0u, true},
    {"colormap_ucoord", 0u, 1u, false},
    {"colormap_vcoord", 0u, 2u, false},
    {"colormap_power", 0u, 0u, false},
    {"desaturation", 0u, 3u, false},
    {"opa_gradation", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,2> WARLORD_NATIVE_SWITCHES_430 = {{
    {"use_depthbiasedalpha", false},
    {"use_down_opacity", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_431 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_431 = {{
    {"selectioncolor", 2u, 0u, true},
    {"uv_scale", 1u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_431 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"use_distortion_floor", false},
    {"non_alpha_chanel", false},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex_02", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_432 = {{"lensflaretexture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_432 = {{
    {"selectioncolor", 1u, 0u, true},
    {"select texture(0 or 0.5)", 0u, 2u, false},
    {"desaturation", 0u, 1u, false},
    {"depthbaisalpha", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_432 = {{
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_433 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,2> WARLORD_NATIVE_PARAMETERS_433 = {{
    {"selectioncolor", 1u, 0u, true},
    {"hardness", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_433 = {{
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_434 = {{"01.diffmap_a","11.diffmap_b","21.maskmap","01.alphamaskmap","native_texture_4"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,10> WARLORD_NATIVE_PARAMETERS_434 = {{
    {"selectioncolor", 3u, 0u, true},
    {"82.diffusecolor", 2u, 0u, true},
    {"02.diffmap_a.uvscale", 0u, 0u, false},
    {"12.diffmap_a.uvscale", 0u, 3u, false},
    {"22.maskmap.uvscale", 1u, 0u, false},
    {"24.maskmap.hardness", 1u, 1u, false},
    {"81.diffusedesat", 1u, 3u, false},
    {"09.str", 0u, 1u, false},
    {"10.power", 0u, 2u, false},
    {"31.dissolvemapscale", 1u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,13> WARLORD_NATIVE_SWITCHES_434 = {{
    {"01.usemesh", false},
    {"01.usevertexcolor2emissive", false},
    {"00.usealphamap", true},
    {"06.mapch.r", false},
    {"07.mapch.g", false},
    {"08.mapch.b", false},
    {"30.usedissolve", true},
    {"10.use2maps", true},
    {"00.useemission", false},
    {"00.usefakelight", false},
    {"00.usespecullar", false},
    {"00.usebump", false},
    {"00.useinvertdepthbiasalpha", false},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_435 = {{"01.heightmap","01.diffmap_a","06.normalmap","01.emismap","native_texture_4"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,14> WARLORD_NATIVE_PARAMETERS_435 = {{
    {"selectioncolor", 5u, 0u, true},
    {"82.diffusecolor", 4u, 0u, true},
    {"03.emap_uv.x.scale", 0u, 2u, false},
    {"04.emap_uv.y.scale", 1u, 0u, false},
    {"21.phasepantimescale", 2u, 0u, false},
    {"19.emissiion_color", 3u, 0u, true},
    {"02.diffmap_a.uvscale", 0u, 1u, false},
    {"04.bumpzero.pos", 0u, 3u, false},
    {"02.bumpheight", 0u, 0u, false},
    {"81.diffusedesat", 2u, 2u, false},
    {"15.emissiion_power", 1u, 3u, false},
    {"09.str", 1u, 1u, false},
    {"10.power", 1u, 2u, false},
    {"31.dissolvemapscale", 2u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,13> WARLORD_NATIVE_SWITCHES_435 = {{
    {"00.useinvertdepthbiasalpha", false},
    {"01.usemesh", false},
    {"30.usedissolve", true},
    {"00.usealphamap", false},
    {"00.usebump", true},
    {"06.mapch.r", true},
    {"07.mapch.g", true},
    {"08.mapch.b", true},
    {"00.useemission", true},
    {"10.use2maps", false},
    {"01.usevertexcolor2emissive", false},
    {"00.usefakelight", true},
    {"00.usespecullar", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_436 = {{"04.map_anew","06.map","00_map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,14> WARLORD_NATIVE_PARAMETERS_436 = {{
    {"selectioncolor", 4u, 0u, true},
    {"05.map_alod", 0u, 1u, false},
    {"09.map_a_panning_x", 1u, 2u, false},
    {"07.map_a_uvscale_r", 0u, 2u, false},
    {"08.map_a_uvscale_g", 1u, 0u, false},
    {"10.map_a_panning_y", 2u, 0u, false},
    {"09.map_d_panning_x", 1u, 3u, false},
    {"07.map_d_uvscale_r", 0u, 3u, false},
    {"08.map_d_uvscale_g", 1u, 1u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 0u, 0u, false},
    {"11.desaturation", 2u, 2u, false},
    {"11.map_a_str", 2u, 3u, false},
    {"12.map_a_power", 3u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_436 = {{
    {"00.usedistortion", false},
    {"20.use_centrehole", false},
    {"00.usedepthbiasalpha", false},
    {"30.usefresnal", false},
    {"10.cameradistance", false},
    {"01.mapch.r", true},
    {"02.mapch.g", true},
    {"03.mapch.b", true},
    {"00.use_uvdistort", true},
    {"01.use_uvdistort_all", true},
    {"checkismeshemitvc", false},
    {"00.use_rgbinalphamap", true},
    {"06.use((r+g+b)*0.33)", false},
    {"15.use_panbezier", false},
    {"01.usecoordinate_index1", false},
    {"30.use_additionalmap", false},
    {"useupno_alpha", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_437 = {{"native_texture_0","native_texture_1","native_texture_2","native_texture_3"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_437 = {{
    {"selectioncolor", 1u, 0u, true},
    {"colormap_ucoord", 0u, 1u, false},
    {"colormap_vcoord", 0u, 2u, false},
    {"colormap_power", 0u, 0u, false},
    {"desaturation", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,2> WARLORD_NATIVE_SWITCHES_437 = {{
    {"use_depthbiasedalpha", true},
    {"use_down_opacity", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_438 = {{"native_texture_0","native_texture_1"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,2> WARLORD_NATIVE_PARAMETERS_438 = {{
    {"selectioncolor", 1u, 0u, true},
    {"power", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_438 = {{
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_439 = {{"uv_noise_tex","emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,10> WARLORD_NATIVE_PARAMETERS_439 = {{
    {"selectioncolor", 3u, 0u, true},
    {"emissive_color&intensity", 2u, 0u, true},
    {"uv_noise_panning_x", 0u, 3u, false},
    {"uv_noise_panning_y", 1u, 0u, false},
    {"uv_panning_x", 1u, 2u, false},
    {"uv_panning_y", 1u, 3u, false},
    {"uv_noise_tilling", 1u, 1u, false},
    {"uv_noise_intensity", 0u, 2u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_tex_desturation", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_439 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_tex", false},
    {"use_emissive_vertcolor", false},
    {"use_nonuvnoise", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_440 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_440 = {{
    {"selectioncolor", 1u, 0u, true},
    {"01.radius", 0u, 0u, false},
    {"02.hardness", 0u, 1u, false},
    {"03.spherepower", 0u, 2u, false},
    {"04.spherestr", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_440 = {{
    {"checkisdepthbiasalpha", false},
    {"checkisvertexcolor", true},
    {"01.checkisvertexcolor", true},
    {"02.usedynamicparam", false},
    {"30.usefresnal", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_441 = {{"emissive_tex","native_texture_1","alpha_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,3> WARLORD_NATIVE_PARAMETERS_441 = {{
    {"selectioncolor", 1u, 0u, true},
    {"alpha_power", 0u, 0u, false},
    {"alpha_strength", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_441 = {{
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_442 = {{"texturesubuv"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_442 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_442 = {{
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_443 = {{"sub_texture","main_texture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,7> WARLORD_NATIVE_PARAMETERS_443 = {{
    {"selectioncolor", 2u, 0u, true},
    {"04.map_sub_panning_x", 0u, 3u, false},
    {"02.map_sub_uvscale_r", 0u, 1u, false},
    {"03.map_sub_uvscale_g", 0u, 2u, false},
    {"05.map_sub_panning_y", 1u, 0u, false},
    {"00.noise_str", 0u, 0u, false},
    {"linethikness", 1u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_443 = {{
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_444 = {{"12.map_f","06.map","02.map_e"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,27> WARLORD_NATIVE_PARAMETERS_444 = {{
    {"selectioncolor", 8u, 0u, true},
    {"93.emissiion_color", 7u, 0u, true},
    {"time", 6u, 0u, false},
    {"15.map_e_panning_x", 4u, 2u, false},
    {"13.map_e_uvscale_r", 4u, 0u, false},
    {"14.map_e_uvscale_g", 4u, 1u, false},
    {"11.uv.curvature", 3u, 3u, false},
    {"09.map_d_panning_x", 3u, 0u, false},
    {"07.map_d_uvscale_r", 2u, 2u, false},
    {"08.map_d_uvscale_g", 2u, 3u, false},
    {"10.map_d_panning_y", 3u, 2u, false},
    {"05.distort_str", 1u, 1u, false},
    {"16.map_e_panning_y", 4u, 3u, false},
    {"05.map_e_panning_x", 1u, 2u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 0u, false},
    {"06.map_e_panning_y", 1u, 3u, false},
    {"91.desaturation", 5u, 0u, false},
    {"92.emissiion_power", 5u, 1u, false},
    {"06.radius", 2u, 0u, false},
    {"07.hardness", 2u, 1u, false},
    {"02.radius", 0u, 1u, false},
    {"03.hardness", 0u, 2u, false},
    {"09.power", 3u, 1u, false},
    {"95.str", 5u, 2u, false},
    {"96.power", 5u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,27> WARLORD_NATIVE_SWITCHES_444 = {{
    {"00.usespecullar", false},
    {"11.mape+mapf", false},
    {"10.use_mapb", true},
    {"01.use_emissionmap", true},
    {"01.usemesh", false},
    {"05.useinvert", true},
    {"00.usedistortion", false},
    {"01.usesphmask", true},
    {"25.xinvert", false},
    {"20.usetwirl", false},
    {"01.usecoordinate_index1", false},
    {"94.sphmask+(texmap*phasemap)", false},
    {"01.effectonalphatex", false},
    {"11.useclamptexmap", false},
    {"91.mapch.r", true},
    {"92.mapch.g", true},
    {"00.checkisdepthbiasalpha", true},
    {"21.usephasetexmap", false},
    {"30.usefresnal", false},
    {"31.fresnal.invert", false},
    {"00.use_uvdistort", true},
    {"29.usedynamic_ypanning", false},
    {"23.usedynamicstr", false},
    {"93.mapch.b", true},
    {"10.fresnal", true},
    {"99.alphadissolve", false},
    {"94.mapch.a", true},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_445 = {{"uv_noise_texture","alpha_texture1_mask","alpha_texture2","emissive_tex_02","emissive_tex_01"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,31> WARLORD_NATIVE_PARAMETERS_445 = {{
    {"selectioncolor", 8u, 0u, true},
    {"mainalpha_move_u", 5u, 0u, false},
    {"mainalpha_move_v", 5u, 1u, false},
    {"mainalpha_rotator", 5u, 2u, false},
    {"emissive_tex_02_move_u", 3u, 2u, false},
    {"emissive_tex_02_move_v", 3u, 3u, false},
    {"emissive_tex_01_move_u", 2u, 0u, false},
    {"emissive_tex_01_move_v", 2u, 1u, false},
    {"mainalpha_tile_u", 5u, 3u, false},
    {"mainalpha_tile_v", 6u, 0u, false},
    {"uv_noise_panspeed_x", 6u, 1u, false},
    {"uv_noise_tile_u", 6u, 3u, false},
    {"uv_noise_tile_v", 7u, 0u, false},
    {"uv_noise_panspeed_y", 6u, 2u, false},
    {"uv_noise_velue", 7u, 1u, false},
    {"emissive_background_power", 0u, 1u, false},
    {"emissive_background_strength", 0u, 2u, false},
    {"emissive_tex_01_panspeed_x", 2u, 2u, false},
    {"emissive_tex_01_tile_u", 3u, 0u, false},
    {"emissive_tex_01_tile_v", 3u, 1u, false},
    {"emissive_tex_02_panspeed_x", 4u, 0u, false},
    {"emissive_tex_02_tile_u", 4u, 2u, false},
    {"emissive_tex_02_tile_v", 4u, 3u, false},
    {"emissive_tex_02_panspeed_y", 4u, 1u, false},
    {"emissive_noise_velue", 1u, 1u, false},
    {"emissive_tex_01_panspeed_y", 2u, 3u, false},
    {"emissive_power", 1u, 2u, false},
    {"emissive_core_power", 0u, 3u, false},
    {"emissive_core_strength", 1u, 0u, false},
    {"emissive_strength", 1u, 3u, false},
    {"alpha_tex_power", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,6> WARLORD_NATIVE_SWITCHES_445 = {{
    {"use_distortion", false},
    {"use_emissive_tex_02", true},
    {"emissive_tex_(01+02)/2", false},
    {"use_alphatex_type_pola", false},
    {"use_uv_noisetexture", true},
    {"use_emissive_tex02_to_emissive01_uvnoise", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_446 = {{"02.map_e","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,19> WARLORD_NATIVE_PARAMETERS_446 = {{
    {"selectioncolor", 9u, 0u, true},
    {"93.emissiion_color", 7u, 0u, true},
    {"meshemitterdynamicparameter", 8u, 0u, true},
    {"29.cmap.direct", 1u, 2u, false},
    {"21.uvscale.x", 1u, 0u, false},
    {"22.uvscale.y", 1u, 1u, false},
    {"05.map_e_panning_x", 0u, 2u, false},
    {"03.map_e_uvscale_r", 0u, 0u, false},
    {"04.map_e_uvscale_g", 0u, 1u, false},
    {"06.map_e_panning_y", 0u, 3u, false},
    {"91.desaturation", 2u, 2u, false},
    {"92.emissiion_power", 2u, 3u, false},
    {"36.str", 2u, 0u, false},
    {"37.power", 2u, 1u, false},
    {"02.xyz.uvoffset", 4u, 0u, true},
    {"01.xyz.uvscale", 3u, 0u, true},
    {"11.xyz.timescale", 5u, 0u, true},
    {"12.xyz.str", 6u, 0u, true},
    {"31.biasscale", 1u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,14> WARLORD_NATIVE_SWITCHES_446 = {{
    {"30.useobjectradius", true},
    {"99.transformworld", false},
    {"20.use_clampmap", true},
    {"31.mapch.r", false},
    {"32.mapch.g", false},
    {"33.mapch.b", false},
    {"34.mapch.a", false},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", false},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.checkisdepthbiasalpha", false},
    {"30.usefresnal", false},
    {"31.fresnal.invert", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_447 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_447 = {{
    {"selectioncolor", 2u, 0u, true},
    {"01.centerglow_power", 0u, 0u, false},
    {"02.centerglow_str", 0u, 1u, false},
    {"03.outpower", 0u, 2u, false},
    {"91.str", 0u, 3u, false},
    {"92.power", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,2> WARLORD_NATIVE_SWITCHES_447 = {{
    {"00.use_distortion", false},
    {"00.use_depth_alpha", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_448 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_448 = {{
    {"selectioncolor", 1u, 0u, true},
    {"01.radius", 0u, 0u, false},
    {"02.hardness", 0u, 1u, false},
    {"03.spherepower", 0u, 2u, false},
    {"04.sphere_str", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_448 = {{
    {"checkisdepthbiasalpha", false},
    {"01.checkisvertexcolor", true},
    {"30.usefresnal", false},
    {"02.usedynamicparam", false},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_449 = {{"native_texture_0","native_texture_1","map_a","map_b","map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,9> WARLORD_NATIVE_PARAMETERS_449 = {{
    {"selectioncolor", 2u, 0u, true},
    {"pc_circlecount", 0u, 3u, false},
    {"pc_ripplecount", 1u, 0u, false},
    {"distortuvscale", 0u, 1u, false},
    {"distortstr", 0u, 0u, false},
    {"radiusgab[0-0.45]", 1u, 2u, false},
    {"inhardness", 0u, 2u, false},
    {"ringpower", 1u, 3u, false},
    {"power", 1u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,2> WARLORD_NATIVE_SWITCHES_449 = {{
    {"checkistimedypa", true},
    {"use_fanshape", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_450 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_450 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_450 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_451 = {{"native_texture_0","01.map_uv","01.map_a","11.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,7> WARLORD_NATIVE_PARAMETERS_451 = {{
    {"selectioncolor", 2u, 0u, true},
    {"09.str", 0u, 1u, false},
    {"10.power", 0u, 2u, false},
    {"12.map_b_uvscale", 0u, 3u, false},
    {"18.str", 1u, 0u, false},
    {"19.power", 1u, 1u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,8> WARLORD_NATIVE_SWITCHES_451 = {{
    {"00.checkisdepthbiasalpha", true},
    {"01.usemesh", false},
    {"06.mapch.r", true},
    {"07.mapch.g", true},
    {"08.mapch.b", true},
    {"00.useemissionmap", false},
    {"30.usefresnal", false},
    {"31.fresnal.invert", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_672 = {{"native_texture_0","native_texture_1"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,13> WARLORD_NATIVE_PARAMETERS_672 = {{
    {"selectioncolor", 3u, 0u, true},
    {"noise speed", 2u, 0u, false},
    {"channel spread range", 0u, 2u, false},
    {"r noise strength", 2u, 3u, false},
    {"r channel strength", 2u, 2u, false},
    {"g noise strength", 1u, 3u, false},
    {"g channel strength", 1u, 2u, false},
    {"b noise strength", 0u, 1u, false},
    {"b channel strength", 0u, 0u, false},
    {"desaturation value", 1u, 1u, false},
    {"circle radius", 0u, 3u, false},
    {"circleedge hardness", 1u, 0u, false},
    {"opacity", 2u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_672 = {{
    {"use_particle", true},
    {"use_circlealpha", true},
    {"use_invertedcirclealpha", false},
    {"use_noise", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_452 = {{"01.map"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_452 = {{
    {"selectioncolor", 2u, 0u, true},
    {"01.circlepower", 0u, 0u, false},
    {"02.circlestr", 0u, 2u, false},
    {"02.circlemaskpower", 0u, 1u, false},
    {"03.circlestr", 0u, 3u, false},
    {"depthbiasdalpha_bias", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_452 = {{
    {"checkisdepthbiasalpha", true},
    {"00.useradius", true},
    {"01.usemesh", false},
    {"10.usetimebezire", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_453 = {{"uv_noise_01_tex","emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,12> WARLORD_NATIVE_PARAMETERS_453 = {{
    {"selectioncolor", 3u, 0u, true},
    {"uv_noise_01_panning_x", 1u, 2u, false},
    {"uv_noise_01_tiling_x", 2u, 0u, false},
    {"uv_noise_01_tiling_y", 2u, 1u, false},
    {"uv_noise_01_panning_y", 1u, 3u, false},
    {"uv_noise_01_intensity", 1u, 1u, false},
    {"uv_scale", 2u, 2u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 3u, false},
    {"edge_intensity", 0u, 1u, false},
    {"edge_power", 0u, 2u, false},
    {"emissive_power", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_453 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", true},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", true},
    {"non_alpha_chanel", true},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_454 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_454 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_454 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_455 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_455 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_455 = {{
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_456 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_456 = {{
    {"selectioncolor", 1u, 0u, true},
    {"uv_scale", 0u, 3u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 1u, false},
    {"emissive_power", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_456 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"use_distortion_floor", false},
    {"non_alpha_chanel", false},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex_02", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_457 = {{"31.map_e","06.map","04.map_anew","00.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,18> WARLORD_NATIVE_PARAMETERS_457 = {{
    {"selectioncolor", 5u, 0u, true},
    {"39.map_a_panning_x", 3u, 1u, false},
    {"37.mape_color", 4u, 0u, true},
    {"09.map_a_panning_x", 1u, 2u, false},
    {"05.map_alod", 0u, 1u, false},
    {"41.map_a_uvscale_r", 3u, 2u, false},
    {"42.map_a_uvscale_g", 3u, 3u, false},
    {"09.map_d_panning_x", 1u, 3u, false},
    {"07.map_d_uvscale_r", 0u, 3u, false},
    {"08.map_d_uvscale_g", 1u, 1u, false},
    {"10.map_d_panning_y", 2u, 0u, false},
    {"05.distort_str", 0u, 0u, false},
    {"33.desaturation", 2u, 3u, false},
    {"35.mape_power", 3u, 0u, false},
    {"07.map_a_uvscale_r", 0u, 2u, false},
    {"08.map_a_uvscale_g", 1u, 0u, false},
    {"11.map_a_str", 2u, 1u, false},
    {"12.map_a_power", 2u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_457 = {{
    {"00.usedistortion", false},
    {"20.use_centrehole", false},
    {"00.usedepthbiasalpha", false},
    {"30.usefresnal", false},
    {"10.cameradistance", false},
    {"01.mapch.r", true},
    {"02.mapch.g", true},
    {"03.mapch.b", true},
    {"00.use_uvdistort", true},
    {"01.use_uvdistort_all", true},
    {"checkismeshemitvc", false},
    {"00.use_rgbinalphamap", false},
    {"06.use((r+g+b)*0.33)", false},
    {"15.use_panbezier", true},
    {"01.usecoordinate_index1", false},
    {"30.use_additionalmap", true},
    {"useupno_alpha", false},
    {"40.use_fanshape", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_458 = {{"lensflaretexture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_458 = {{
    {"selectioncolor", 2u, 0u, true},
    {"x tiling", 0u, 3u, false},
    {"y tiling", 1u, 0u, false},
    {"select texture", 0u, 2u, false},
    {"desaturation", 0u, 0u, false},
    {"opacity", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_458 = {{
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_459 = {{"06.map","02.map_e","01.map_a","11.map_b","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,31> WARLORD_NATIVE_PARAMETERS_459 = {{
    {"selectioncolor", 9u, 0u, true},
    {"93.emissiion_color", 7u, 0u, true},
    {"meshemitterdynamicparameter", 8u, 0u, true},
    {"21.uvscale.x", 4u, 1u, false},
    {"22.uvscale.y", 4u, 2u, false},
    {"time", 6u, 3u, false},
    {"05.map_e_panning_x", 1u, 3u, false},
    {"03.map_e_uvscale_r", 0u, 2u, false},
    {"04.map_e_uvscale_g", 1u, 0u, false},
    {"09.map_d_panning_x", 2u, 3u, false},
    {"07.map_d_uvscale_r", 2u, 1u, false},
    {"08.map_d_uvscale_g", 2u, 2u, false},
    {"10.map_d_panning_y", 3u, 0u, false},
    {"05.distort_str", 1u, 1u, false},
    {"06.map_e_panning_y", 2u, 0u, false},
    {"91.desaturation", 6u, 1u, false},
    {"92.emissiion_power", 6u, 2u, false},
    {"04.map_a_panning_x", 0u, 3u, false},
    {"02.map_a_uvscale_r", 0u, 0u, false},
    {"03.map_a_uvscale_g", 0u, 1u, false},
    {"05.map_a_panning_y", 1u, 2u, false},
    {"14.map_b_panning_x", 3u, 3u, false},
    {"12.map_b_uvscale_r", 3u, 1u, false},
    {"13.map_b_uvscale_g", 3u, 2u, false},
    {"15.map_b_panning_y", 4u, 0u, false},
    {"29.cmap.direct", 4u, 3u, false},
    {"30.cmap.time(rotrate)", 5u, 0u, false},
    {"36.str", 5u, 3u, false},
    {"37.power", 6u, 0u, false},
    {"32.fresnal_power", 5u, 1u, false},
    {"33.fresnal_str", 5u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_459 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", false},
    {"32.mapch.g", false},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", true},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", false},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_660 = {{"subuv_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_660 = {{
    {"selectioncolor", 2u, 0u, true},
    {"emissive_color&intensity", 1u, 0u, true},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_tex_desturation", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_660 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", true},
    {"use_nonalpha_tex", false},
    {"use_emissive_vertcolor", false},
    {"use_nonuvnoise", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_661 = {{"31.map_e","06.map","04.map_anew","00.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,19> WARLORD_NATIVE_PARAMETERS_661 = {{
    {"selectioncolor", 6u, 0u, true},
    {"39.map_a_panning_x", 3u, 2u, false},
    {"37.mape_color", 5u, 0u, true},
    {"09.map_a_panning_x", 1u, 3u, false},
    {"05.map_alod", 0u, 2u, false},
    {"41.map_a_uvscale_r", 3u, 3u, false},
    {"42.map_a_uvscale_g", 4u, 0u, false},
    {"09.map_d_panning_x", 2u, 0u, false},
    {"07.map_d_uvscale_r", 1u, 0u, false},
    {"08.map_d_uvscale_g", 1u, 2u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 0u, 1u, false},
    {"33.desaturation", 3u, 0u, false},
    {"35.mape_power", 3u, 1u, false},
    {"07.map_a_uvscale_r", 0u, 3u, false},
    {"08.map_a_uvscale_g", 1u, 1u, false},
    {"11.map_a_str", 2u, 2u, false},
    {"12.map_a_power", 2u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_661 = {{
    {"00.usedistortion", false},
    {"20.use_centrehole", false},
    {"00.usedepthbiasalpha", true},
    {"30.usefresnal", false},
    {"10.cameradistance", false},
    {"01.mapch.r", true},
    {"02.mapch.g", true},
    {"03.mapch.b", true},
    {"00.use_uvdistort", true},
    {"01.use_uvdistort_all", true},
    {"checkismeshemitvc", false},
    {"00.use_rgbinalphamap", false},
    {"06.use((r+g+b)*0.33)", false},
    {"15.use_panbezier", true},
    {"01.usecoordinate_index1", false},
    {"30.use_additionalmap", true},
    {"useupno_alpha", false},
    {"40.use_fanshape", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_662 = {{"31.map_e","06.map","04.map_anew","00.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,19> WARLORD_NATIVE_PARAMETERS_662 = {{
    {"selectioncolor", 6u, 0u, true},
    {"39.map_a_panning_x", 3u, 2u, false},
    {"37.mape_color", 5u, 0u, true},
    {"09.map_a_panning_x", 1u, 3u, false},
    {"05.map_alod", 0u, 2u, false},
    {"41.map_a_uvscale_r", 3u, 3u, false},
    {"42.map_a_uvscale_g", 4u, 0u, false},
    {"09.map_d_panning_x", 2u, 0u, false},
    {"07.map_d_uvscale_r", 1u, 0u, false},
    {"08.map_d_uvscale_g", 1u, 2u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 0u, 1u, false},
    {"33.desaturation", 3u, 0u, false},
    {"35.mape_power", 3u, 1u, false},
    {"07.map_a_uvscale_r", 0u, 3u, false},
    {"08.map_a_uvscale_g", 1u, 1u, false},
    {"11.map_a_str", 2u, 2u, false},
    {"12.map_a_power", 2u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_662 = {{
    {"00.usedistortion", false},
    {"20.use_centrehole", false},
    {"00.usedepthbiasalpha", true},
    {"30.usefresnal", false},
    {"10.cameradistance", false},
    {"01.mapch.r", true},
    {"02.mapch.g", true},
    {"03.mapch.b", true},
    {"00.use_uvdistort", true},
    {"01.use_uvdistort_all", true},
    {"checkismeshemitvc", false},
    {"00.use_rgbinalphamap", false},
    {"06.use((r+g+b)*0.33)", false},
    {"15.use_panbezier", true},
    {"01.usecoordinate_index1", false},
    {"30.use_additionalmap", true},
    {"useupno_alpha", false},
    {"40.use_fanshape", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_663 = {{"06.map","01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,21> WARLORD_NATIVE_PARAMETERS_663 = {{
    {"selectioncolor", 6u, 0u, true},
    {"meshemitterdynamicparameter", 5u, 0u, true},
    {"21.uvscale.x", 2u, 2u, false},
    {"22.uvscale.y", 2u, 3u, false},
    {"time", 4u, 2u, false},
    {"04.map_a_panning_x", 0u, 3u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"09.map_d_panning_x", 2u, 0u, false},
    {"07.map_d_uvscale_r", 1u, 2u, false},
    {"08.map_d_uvscale_g", 1u, 3u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 1u, 0u, false},
    {"05.map_a_panning_y", 1u, 1u, false},
    {"29.cmap.direct", 3u, 0u, false},
    {"30.cmap.time(rotrate)", 3u, 1u, false},
    {"36.str", 4u, 0u, false},
    {"37.power", 4u, 1u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"32.fresnal_power", 3u, 2u, false},
    {"33.fresnal_str", 3u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_663 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", true},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", false},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_664 = {{"lensflaretexture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_664 = {{
    {"selectioncolor", 1u, 0u, true},
    {"select texture(0 or 0.5)", 0u, 2u, false},
    {"desaturation", 0u, 1u, false},
    {"depthbaisalpha", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_664 = {{
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_665 = {{"noisetexture","native_texture_1","wavetexture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,7> WARLORD_NATIVE_PARAMETERS_665 = {{
    {"selectioncolor", 2u, 0u, true},
    {"x_coord", 0u, 3u, false},
    {"y_coord", 1u, 0u, false},
    {"panning_speed", 0u, 1u, false},
    {"y_scale", 1u, 1u, false},
    {"widestrength", 0u, 2u, false},
    {"alphastrength", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_665 = {{
    {"use_distortion", true},
    {"use_widefloor", true},
    {"usemesh", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_666 = {{"02.map_e","12.map_f","01.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,22> WARLORD_NATIVE_PARAMETERS_666 = {{
    {"selectioncolor", 6u, 0u, true},
    {"93.emissiion_color", 5u, 0u, true},
    {"time", 4u, 3u, false},
    {"05.map_e_panning_x", 1u, 3u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 1u, false},
    {"06.map_e_panning_y", 2u, 0u, false},
    {"15.map_f_panning_x", 2u, 3u, false},
    {"13.map_f_uvscale_r", 2u, 1u, false},
    {"14.map_f_uvscale_g", 2u, 2u, false},
    {"16.map_f_panning_y", 3u, 0u, false},
    {"91.desaturation", 4u, 1u, false},
    {"92.emissiion_power", 4u, 2u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"05.map_a_panning_y", 1u, 2u, false},
    {"36.str", 3u, 3u, false},
    {"37.power", 4u, 0u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"32.fresnal_power", 3u, 1u, false},
    {"33.fresnal_str", 3u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_666 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", false},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", false},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", true},
    {"11.map_e + map_f", true},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_667 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_667 = {{
    {"selectioncolor", 2u, 0u, true},
    {"01.radius", 0u, 0u, false},
    {"02.hardness", 0u, 1u, false},
    {"03.spherepower", 0u, 2u, false},
    {"04.sphere_str", 0u, 3u, false},
    {"depthbiasdalpha_bias", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_667 = {{
    {"01.checkisvertexcolor", true},
    {"02.usedynamicparam", false},
    {"30.usefresnal", false},
    {"checkisdepthbiasalpha", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_668 = {{"06.map","02.map_e","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,23> WARLORD_NATIVE_PARAMETERS_668 = {{
    {"selectioncolor", 7u, 0u, true},
    {"93.emissiion_color", 5u, 0u, true},
    {"meshemitterdynamicparameter", 6u, 0u, true},
    {"21.uvscale.x", 2u, 1u, false},
    {"22.uvscale.y", 2u, 2u, false},
    {"time", 4u, 3u, false},
    {"05.map_e_panning_x", 0u, 3u, false},
    {"03.map_e_uvscale_r", 0u, 0u, false},
    {"04.map_e_uvscale_g", 0u, 1u, false},
    {"09.map_d_panning_x", 1u, 3u, false},
    {"07.map_d_uvscale_r", 1u, 1u, false},
    {"08.map_d_uvscale_g", 1u, 2u, false},
    {"10.map_d_panning_y", 2u, 0u, false},
    {"05.distort_str", 0u, 2u, false},
    {"06.map_e_panning_y", 1u, 0u, false},
    {"91.desaturation", 4u, 1u, false},
    {"92.emissiion_power", 4u, 2u, false},
    {"29.cmap.direct", 2u, 3u, false},
    {"30.cmap.time(rotrate)", 3u, 0u, false},
    {"36.str", 3u, 3u, false},
    {"37.power", 4u, 0u, false},
    {"32.fresnal_power", 3u, 1u, false},
    {"33.fresnal_str", 3u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_668 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", false},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", false},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_669 = {{"native_texture_0","native_texture_1","native_texture_2","native_texture_3"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_669 = {{
    {"selectioncolor", 1u, 0u, true},
    {"colormap_ucoord", 0u, 1u, false},
    {"colormap_vcoord", 0u, 2u, false},
    {"colormap_power", 0u, 0u, false},
    {"desaturation", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,2> WARLORD_NATIVE_SWITCHES_669 = {{
    {"use_depthbiasedalpha", true},
    {"use_down_opacity", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_670 = {{"01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,15> WARLORD_NATIVE_PARAMETERS_670 = {{
    {"selectioncolor", 5u, 0u, true},
    {"meshemitterdynamicparameter", 4u, 0u, true},
    {"21.uvscale.x", 1u, 0u, false},
    {"22.uvscale.y", 1u, 1u, false},
    {"time", 3u, 0u, false},
    {"04.map_a_panning_x", 0u, 2u, false},
    {"02.map_a_uvscale_r", 0u, 0u, false},
    {"03.map_a_uvscale_g", 0u, 1u, false},
    {"05.map_a_panning_y", 0u, 3u, false},
    {"29.cmap.direct", 1u, 2u, false},
    {"30.cmap.time(rotrate)", 1u, 3u, false},
    {"36.str", 2u, 2u, false},
    {"37.power", 2u, 3u, false},
    {"32.fresnal_power", 2u, 0u, false},
    {"33.fresnal_str", 2u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_670 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", false},
    {"32.mapch.g", false},
    {"20.use_clampmap", true},
    {"25.use_yclamp", true},
    {"00.use_uvdistort", false},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", false},
    {"00.use_emission", false},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_671 = {{"02.map_e","12.map_f","01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,26> WARLORD_NATIVE_PARAMETERS_671 = {{
    {"selectioncolor", 8u, 0u, true},
    {"93.emissiion_color", 6u, 0u, true},
    {"meshemitterdynamicparameter", 7u, 0u, true},
    {"21.uvscale.x", 3u, 0u, false},
    {"22.uvscale.y", 3u, 1u, false},
    {"time", 5u, 2u, false},
    {"05.map_e_panning_x", 1u, 2u, false},
    {"03.map_e_uvscale_r", 0u, 2u, false},
    {"04.map_e_uvscale_g", 1u, 0u, false},
    {"06.map_e_panning_y", 1u, 3u, false},
    {"15.map_f_panning_x", 2u, 2u, false},
    {"13.map_f_uvscale_r", 2u, 0u, false},
    {"14.map_f_uvscale_g", 2u, 1u, false},
    {"16.map_f_panning_y", 2u, 3u, false},
    {"91.desaturation", 5u, 0u, false},
    {"92.emissiion_power", 5u, 1u, false},
    {"04.map_a_panning_x", 0u, 3u, false},
    {"02.map_a_uvscale_r", 0u, 0u, false},
    {"03.map_a_uvscale_g", 0u, 1u, false},
    {"05.map_a_panning_y", 1u, 1u, false},
    {"29.cmap.direct", 3u, 2u, false},
    {"30.cmap.time(rotrate)", 3u, 3u, false},
    {"36.str", 4u, 2u, false},
    {"37.power", 4u, 3u, false},
    {"32.fresnal_power", 4u, 0u, false},
    {"33.fresnal_str", 4u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_671 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", false},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", false},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", true},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1000 = {{"flowtex","diff_tex1","diff_tex2","opacity_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,27> WARLORD_NATIVE_PARAMETERS_1000 = {{
    {"selectioncolor", 8u, 0u, true},
    {"diff_backcolor", 6u, 0u, true},
    {"flow_tile_u", 3u, 3u, false},
    {"flow_tile_v", 4u, 0u, false},
    {"flow_pan_u", 3u, 1u, false},
    {"flow_pan_v", 3u, 2u, false},
    {"meshemitterdynamicparameter", 7u, 0u, true},
    {"diff1_pan_u", 0u, 1u, false},
    {"diff1_pan_v", 0u, 2u, false},
    {"diff2_pan_u", 1u, 1u, false},
    {"diff2_pan_v", 1u, 2u, false},
    {"opacity_tile_u", 5u, 2u, false},
    {"opacity_tile_v", 5u, 3u, false},
    {"opacity_rot", 5u, 0u, false},
    {"diff1_tile_v", 1u, 0u, false},
    {"diff1_tile_u", 0u, 3u, false},
    {"flow_bias", 3u, 0u, false},
    {"diff2_tile_v", 2u, 0u, false},
    {"diff2_tile_u", 1u, 3u, false},
    {"diff_des", 2u, 1u, false},
    {"diff_pow", 2u, 2u, false},
    {"diff_str", 2u, 3u, false},
    {"cameravec_pow", 0u, 0u, false},
    {"opacity_distort_str", 4u, 2u, false},
    {"gra_pow", 4u, 1u, false},
    {"opacity_pow", 4u, 3u, false},
    {"opacity_str", 5u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_1000 = {{
    {"use_gra_r_channel", false},
    {"use_colormap", false},
    {"diff_pan_use_u", false},
    {"use_cam_vector", true},
    {"use_distortion", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1001 = {{"dissolve_noise_tex","emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,10> WARLORD_NATIVE_PARAMETERS_1001 = {{
    {"selectioncolor", 3u, 0u, true},
    {"dissolve_color&intensity", 2u, 0u, true},
    {"dissolve_noise_intensity", 0u, 1u, false},
    {"dissolve_noise_power", 0u, 2u, false},
    {"dissolve_power", 0u, 3u, false},
    {"uv_scale", 1u, 3u, false},
    {"dynamic_parameter_explanation", 1u, 0u, false},
    {"emissive_desaturation", 1u, 1u, false},
    {"emissive_power", 1u, 2u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,14> WARLORD_NATIVE_SWITCHES_1001 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_lamp_alpha", false},
    {"use_dissolve", true},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"non_alpha_chanel", true},
    {"use_uv_noise_tex", false},
    {"use_uv_noise_tex_02", false},
    {"use_fresnel_alpha", false},
    {"use_edge_glow", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1002 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_1002 = {{
    {"selectioncolor", 1u, 0u, true},
    {"01.radius", 0u, 0u, false},
    {"02.hardness", 0u, 1u, false},
    {"03.spherepower", 0u, 2u, false},
    {"04.sphere_str", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1002 = {{
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1003 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,19> WARLORD_NATIVE_PARAMETERS_1003 = {{
    {"selectioncolor", 6u, 0u, true},
    {"noisecolor", 5u, 0u, true},
    {"15.map_d_uvscale_r", 2u, 3u, false},
    {"16.map_d_uvscale_g", 3u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 2u, false},
    {"03.map_a_uvscale_g", 1u, 0u, false},
    {"04.map_a_panning_x", 1u, 2u, false},
    {"05.map_a_panning_y", 2u, 0u, false},
    {"02.map_b_uvscale_r", 0u, 3u, false},
    {"03.map_b_uvscale_g", 1u, 1u, false},
    {"04.map_b_panning_x", 1u, 3u, false},
    {"05.map_b_panning_y", 2u, 1u, false},
    {"power", 3u, 3u, false},
    {"13.distortstr", 2u, 2u, false},
    {"00.noisepower", 0u, 0u, false},
    {"00.noisestr", 0u, 1u, false},
    {"biasvalue", 3u, 1u, false},
    {"str", 4u, 0u, false},
    {"depthbiasdalpha_bias", 3u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,6> WARLORD_NATIVE_SWITCHES_1003 = {{
    {"checkisdepthbiasalpha", true},
    {"00.checkisnoiseon", true},
    {"12.uvnoise", true},
    {"11.uvmirroring", false},
    {"use_meshtype", false},
    {"use_multyply_noisecolor", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1004 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1004 = {{
    {"selectioncolor", 2u, 0u, true},
    {"centerglow_power", 0u, 0u, false},
    {"centerglow_str", 0u, 1u, false},
    {"glow_power", 0u, 2u, false},
    {"str", 1u, 0u, false},
    {"power", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1004 = {{
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1005 = {{"native_texture_0","texturesample","tex_01","opacity_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1005 = {{
    {"selectioncolor", 2u, 0u, true},
    {"meshemitterdynamicparameter", 1u, 0u, true},
    {"tex_alpha_02_r_tile", 0u, 3u, false},
    {"tex_alpha_02_g_tile", 0u, 2u, false},
    {"tex_alpha_01_r_tile", 0u, 1u, false},
    {"tex_alpha_01_g_tile", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,2> WARLORD_NATIVE_SWITCHES_1005 = {{
    {"spheremask", true},
    {"01.useparticle", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1006 = {{"flowtex","diff_tex1","diff_tex2","opacity_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,26> WARLORD_NATIVE_PARAMETERS_1006 = {{
    {"selectioncolor", 7u, 0u, true},
    {"flow_tile_u", 3u, 3u, false},
    {"flow_tile_v", 4u, 0u, false},
    {"flow_pan_u", 3u, 1u, false},
    {"flow_pan_v", 3u, 2u, false},
    {"meshemitterdynamicparameter", 6u, 0u, true},
    {"diff1_pan_u", 0u, 1u, false},
    {"diff1_pan_v", 0u, 2u, false},
    {"diff2_pan_u", 1u, 1u, false},
    {"diff2_pan_v", 1u, 2u, false},
    {"opacity_tile_u", 5u, 2u, false},
    {"opacity_tile_v", 5u, 3u, false},
    {"opacity_rot", 5u, 0u, false},
    {"diff1_tile_v", 1u, 0u, false},
    {"diff1_tile_u", 0u, 3u, false},
    {"flow_bias", 3u, 0u, false},
    {"diff2_tile_v", 2u, 0u, false},
    {"diff2_tile_u", 1u, 3u, false},
    {"diff_des", 2u, 1u, false},
    {"diff_pow", 2u, 2u, false},
    {"diff_str", 2u, 3u, false},
    {"cameravec_pow", 0u, 0u, false},
    {"opacity_distort_str", 4u, 2u, false},
    {"gra_pow", 4u, 1u, false},
    {"opacity_pow", 4u, 3u, false},
    {"opacity_str", 5u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_1006 = {{
    {"use_gra_r_channel", false},
    {"use_colormap", false},
    {"diff_pan_use_u", false},
    {"use_cam_vector", true},
    {"use_distortion", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1007 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1007 = {{
    {"selectioncolor", 2u, 0u, true},
    {"uv_scale", 1u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_1007 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"use_distortion_floor", false},
    {"non_alpha_chanel", false},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex_02", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1008 = {{"lensflaretexture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_1008 = {{
    {"selectioncolor", 1u, 0u, true},
    {"select texture(0 or 0.5)", 0u, 2u, false},
    {"desaturation", 0u, 1u, false},
    {"depthbaisalpha", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1008 = {{
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1009 = {{"lensflaretexture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_1009 = {{
    {"selectioncolor", 1u, 0u, true},
    {"select texture(0 or 0.5)", 0u, 2u, false},
    {"desaturation", 0u, 1u, false},
    {"depthbaisalpha", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1009 = {{
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1010 = {{"maintex","uv_noise_tex","noisedissolve_tex","dissolve_tex_01"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,39> WARLORD_NATIVE_PARAMETERS_1010 = {{
    {"selectioncolor", 11u, 0u, true},
    {"maintex_rotator", 4u, 3u, false},
    {"uvnoise_move_x", 8u, 3u, false},
    {"uvnoise_move_y", 9u, 0u, false},
    {"maintex_move_x", 3u, 3u, false},
    {"maintex_move_y", 4u, 0u, false},
    {"dissolvetex_rotator", 1u, 3u, false},
    {"edge_color", 10u, 0u, true},
    {"maintex_panspeed_x", 4u, 1u, false},
    {"maintex_tile_x", 5u, 0u, false},
    {"maintex_tile_y", 5u, 1u, false},
    {"maintex_dynamicpan_x_velue", 3u, 1u, false},
    {"maintex_dynamicpan_y_velue", 3u, 2u, false},
    {"maintex_panspeed_y", 4u, 2u, false},
    {"uv_noisetex_pan_x", 7u, 3u, false},
    {"uv_noisetex_tile_x", 8u, 1u, false},
    {"uv_noisetex_tile_y", 8u, 2u, false},
    {"uv_noisetex_pan_y", 8u, 0u, false},
    {"uv_noise_velue", 7u, 2u, false},
    {"emissive_core_power", 2u, 2u, false},
    {"emissive_core_strength", 2u, 3u, false},
    {"emissive_base", 2u, 1u, false},
    {"disslovetex_01_panspeed_x", 0u, 0u, false},
    {"disslovetex_01_tile_x", 0u, 2u, false},
    {"disslovetex_01_tile_y", 0u, 3u, false},
    {"noisetodisslovetex_01_panspeed_x", 5u, 3u, false},
    {"noisetodisslovetex_01_tile_x", 6u, 1u, false},
    {"noisetodisslovetex_01_tile_y", 6u, 2u, false},
    {"noisetodisslovetex_01_panspeed_y", 6u, 0u, false},
    {"noisedissolvetex_strength", 5u, 2u, false},
    {"disslovetex_01_panspeed_y", 0u, 1u, false},
    {"dissolvetex_move_x", 1u, 1u, false},
    {"dissolvetex_move_y", 1u, 2u, false},
    {"dissolve_hardness", 1u, 0u, false},
    {"edge_thin", 2u, 0u, false},
    {"spheremask_strength", 6u, 3u, false},
    {"spheremask_strength_max", 7u, 0u, false},
    {"spheremask_strength_min", 7u, 1u, false},
    {"maintex_alpha_strength", 3u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,20> WARLORD_NATIVE_SWITCHES_1010 = {{
    {"use_dissolve", true},
    {"use_meshtype", false},
    {"use_uvnoise", true},
    {"achannel_use_uvnoisedynamicpan", false},
    {"use_uv_noise_02", false},
    {"use_uv_noisetex_02", true},
    {"uv_noisetype", true},
    {"maintex_type_polar", false},
    {"maintex_pan_xy", true},
    {"use_dynamic_spheryxy", false},
    {"uv_sphery_y", false},
    {"uv_sphery_x", false},
    {"use_alpha_fresnel", false},
    {"use_dissolvetex_02", false},
    {"use_dissolvetex_noise", true},
    {"use_dissolvetex_noise_type", true},
    {"use_dissolvetex_02_mixtype", true},
    {"use_sphere_alpha", true},
    {"dissloveoutline_multyply_alpha", false},
    {"use_emissivetex02", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1011 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1011 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_1011 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1012 = {{"01.map_e","native_texture_1","01.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,16> WARLORD_NATIVE_PARAMETERS_1012 = {{
    {"selectioncolor", 6u, 0u, true},
    {"11.direct", 2u, 0u, false},
    {"meshemitterdynamicparameter", 5u, 0u, true},
    {"16.color", 4u, 0u, true},
    {"04.map_e_panning_x", 0u, 3u, false},
    {"02.map_e_uvscale_r", 0u, 1u, false},
    {"03.map_e_uvscale_g", 0u, 2u, false},
    {"05.uv.y.strech", 1u, 1u, false},
    {"05.map_e_panning_y", 1u, 0u, false},
    {"12.desaturation", 2u, 1u, false},
    {"15.emissiion_power", 2u, 2u, false},
    {"18.headstr_power", 2u, 3u, false},
    {"19.headstr", 3u, 0u, false},
    {"09.str", 1u, 2u, false},
    {"10.power", 1u, 3u, false},
    {"00.introduction", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,11> WARLORD_NATIVE_SWITCHES_1012 = {{
    {"00.usedistortion", false},
    {"50.use_fresnal", false},
    {"06.mapch.r", true},
    {"07.mapch.g", true},
    {"08.mapch.b", true},
    {"01.usemesh", true},
    {"00.use_uvdistort", false},
    {"00.useemissionmap", true},
    {"--usespecullar", false},
    {"17.useheademission", true},
    {"01.texcoord.index1", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1013 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1013 = {{
    {"selectioncolor", 2u, 0u, true},
    {"uv_scale", 1u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 1u, false},
    {"emissive_intensity", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,13> WARLORD_NATIVE_SWITCHES_1013 = {{
    {"use_dissolve_edge", false},
    {"use_uv_noise_tex", false},
    {"use_dissolve", false},
    {"use_uv_panning", false},
    {"use_meshtype", false},
    {"use_alpha_tex", false},
    {"use_fresnel_alpha", false},
    {"use_uv_rotation", false},
    {"use_mesh_world_normal", false},
    {"use_noise_tex_control", false},
    {"non_alpha_chanel", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1015 = {{"map_a","map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_1015 = {{
    {"selectioncolor", 1u, 0u, true},
    {"readme", 0u, 3u, false},
    {"map_a_uvscale", 0u, 0u, false},
    {"map_b_uvscale", 0u, 1u, false},
    {"map_str", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,7> WARLORD_NATIVE_SWITCHES_1015 = {{
    {"00.uselightshift", false},
    {"01.use_depthbiasalpha", false},
    {"btimebezier", true},
    {"00.userainbow", false},
    {"20.usespacular", false},
    {"01.usemacrouv", false},
    {"01.usemesh", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1017 = {{"uv_noise_tex_01","mainalpha_tex","dissolve_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,32> WARLORD_NATIVE_PARAMETERS_1017 = {{
    {"selectioncolor", 8u, 0u, true},
    {"uvnoisetex_move_x", 7u, 1u, false},
    {"uvnoisetex_move_y", 7u, 2u, false},
    {"noise_pan_speed", 5u, 1u, false},
    {"maintax_move_x", 4u, 1u, false},
    {"maintax_move_y", 4u, 2u, false},
    {"mainalpha_tex_panspeed", 3u, 3u, false},
    {"mainalpha_tex_rotator", 4u, 0u, false},
    {"alpha_position_x", 0u, 2u, false},
    {"alpha_position_y", 0u, 3u, false},
    {"uvnoise_texcoord_y", 7u, 0u, false},
    {"uvnoise_texcoord_x", 6u, 3u, false},
    {"uvnoise_texcoord_02_x", 6u, 1u, false},
    {"uvnoise_texcoord_02_y", 6u, 2u, false},
    {"uv_noise_velue_all", 6u, 0u, false},
    {"maintax_texcoord_y", 5u, 0u, false},
    {"maintax_texcoord_x", 4u, 3u, false},
    {"uv_noise_velue_01", 5u, 2u, false},
    {"uv_noise_velue_02", 5u, 3u, false},
    {"emissive_power", 3u, 1u, false},
    {"emissive_strength", 3u, 2u, false},
    {"emissive_background_velue", 3u, 0u, false},
    {"alpha_background_velue", 0u, 0u, false},
    {"alpha_radius", 1u, 0u, false},
    {"alpha_hardness", 0u, 1u, false},
    {"alpha_strength", 1u, 1u, false},
    {"dissolve_pan_x", 1u, 3u, false},
    {"dissolve_texcoord_x", 2u, 1u, false},
    {"dissolve_texcoord_y", 2u, 2u, false},
    {"dissolve_pan_y", 2u, 0u, false},
    {"dissolve_velue", 2u, 3u, false},
    {"dissolve_hardness", 1u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,10> WARLORD_NATIVE_SWITCHES_1017 = {{
    {"use_centeralpha", true},
    {"use_uvnoisetex_02", false},
    {"use_b_channal", false},
    {"use_g_channal", false},
    {"use_r_channal", true},
    {"use_rgb_channal", false},
    {"use_uv_noise_tex_03", false},
    {"use_alpha_channal", false},
    {"use_disslove", true},
    {"use_disslove_dynamicparameter", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1018 = {{"native_texture_0","native_texture_1","native_texture_2","native_texture_3"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1018 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1018 = {{
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1020 = {{"add_emissive_tex","twinkle_tex","twinkle_tex_01","mask_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,8> WARLORD_NATIVE_PARAMETERS_1020 = {{
    {"selectioncolor", 3u, 0u, true},
    {"emissive_color&intensity", 2u, 0u, true},
    {"emissive_paning", 0u, 1u, false},
    {"emissive_tiling", 0u, 2u, false},
    {"add_emissive_intensity", 0u, 0u, false},
    {"twinkle_paning", 1u, 0u, false},
    {"twinkle_tiling", 1u, 1u, false},
    {"twinkle_intensity", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_1020 = {{
    {"use_mask_r_chanel", false},
    {"use_emissive_r_chanel", false},
    {"use_meshtype", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1021 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_1021 = {{
    {"selectioncolor", 1u, 0u, true},
    {"01.radius", 0u, 0u, false},
    {"02.hardness", 0u, 1u, false},
    {"03.spherepower", 0u, 2u, false},
    {"04.sphere_str", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1021 = {{
    {"checkisdepthbiasalpha", false},
    {"01.checkisvertexcolor", true},
    {"30.usefresnal", false},
    {"02.usedynamicparam", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1022 = {{"subuv_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_1022 = {{
    {"selectioncolor", 2u, 0u, true},
    {"emissive_color&intensity", 1u, 0u, true},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_tex_desturation", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_1022 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", true},
    {"use_nonalpha_tex", false},
    {"use_emissive_vertcolor", false},
    {"use_nonuvnoise", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1023 = {{"uv_noise_tex","alpha_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,26> WARLORD_NATIVE_PARAMETERS_1023 = {{
    {"selectioncolor", 7u, 0u, true},
    {"emissive_tex_core_positon_r", 3u, 3u, false},
    {"emissive_tex_core_positon_g", 3u, 2u, false},
    {"alpha_tex_rotator", 1u, 3u, false},
    {"alpha_tex_positon_r", 1u, 1u, false},
    {"alpha_tex_positon_g", 1u, 0u, false},
    {"emissive_tex_core_g", 3u, 0u, false},
    {"emissive_tex_core_r", 4u, 0u, false},
    {"uvnoise_tex_01_r_texcoord", 5u, 2u, false},
    {"uvnoise_tex_01_g_texcoord", 5u, 1u, false},
    {"uv_noise_head_velue", 5u, 0u, false},
    {"uvnoise_tex_02_r_texcoord", 6u, 0u, false},
    {"uvnoise_tex_02_g_texcoord", 5u, 3u, false},
    {"emissive_tex_core_r_02", 4u, 1u, false},
    {"emissive_tex_core_g_02", 3u, 1u, false},
    {"emissive_tex_power", 4u, 2u, false},
    {"emissive_tex_strength", 4u, 3u, false},
    {"emissive_tex_backvelue", 2u, 3u, false},
    {"alpha_tex_r_texcoord", 1u, 2u, false},
    {"alpha_tex_g_texcoord", 0u, 3u, false},
    {"alpha_tex_strength", 2u, 0u, false},
    {"alpha_out_falloff", 0u, 2u, false},
    {"dissolve_r", 2u, 2u, false},
    {"alpha_disslove_tex_coord_r", 0u, 1u, false},
    {"alpha_disslove_tex_coord_g", 0u, 0u, false},
    {"dissolve_hardness", 2u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1023 = {{
    {"use_mesh", false},
    {"use_dissolve_tex", false},
    {"use_fresnelalpha", false},
    {"dissolve_rampmap", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1024 = {{"uv_noise_tex","alpha_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,26> WARLORD_NATIVE_PARAMETERS_1024 = {{
    {"selectioncolor", 7u, 0u, true},
    {"emissive_tex_core_positon_r", 3u, 3u, false},
    {"emissive_tex_core_positon_g", 3u, 2u, false},
    {"alpha_tex_rotator", 1u, 3u, false},
    {"alpha_tex_positon_r", 1u, 1u, false},
    {"alpha_tex_positon_g", 1u, 0u, false},
    {"emissive_tex_core_g", 3u, 0u, false},
    {"emissive_tex_core_r", 4u, 0u, false},
    {"uvnoise_tex_01_r_texcoord", 5u, 2u, false},
    {"uvnoise_tex_01_g_texcoord", 5u, 1u, false},
    {"uv_noise_head_velue", 5u, 0u, false},
    {"uvnoise_tex_02_r_texcoord", 6u, 0u, false},
    {"uvnoise_tex_02_g_texcoord", 5u, 3u, false},
    {"emissive_tex_core_r_02", 4u, 1u, false},
    {"emissive_tex_core_g_02", 3u, 1u, false},
    {"emissive_tex_power", 4u, 2u, false},
    {"emissive_tex_strength", 4u, 3u, false},
    {"emissive_tex_backvelue", 2u, 3u, false},
    {"alpha_tex_r_texcoord", 1u, 2u, false},
    {"alpha_tex_g_texcoord", 0u, 3u, false},
    {"alpha_tex_strength", 2u, 0u, false},
    {"alpha_out_falloff", 0u, 2u, false},
    {"dissolve_r", 2u, 2u, false},
    {"alpha_disslove_tex_coord_r", 0u, 1u, false},
    {"alpha_disslove_tex_coord_g", 0u, 0u, false},
    {"dissolve_hardness", 2u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1024 = {{
    {"use_mesh", false},
    {"use_dissolve_tex", false},
    {"use_fresnelalpha", false},
    {"dissolve_rampmap", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1025 = {{"01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,15> WARLORD_NATIVE_PARAMETERS_1025 = {{
    {"selectioncolor", 4u, 0u, true},
    {"power", 3u, 0u, false},
    {"str", 3u, 1u, false},
    {"04.map_a_panning_x", 1u, 2u, false},
    {"02.map_a_uvscale_r", 0u, 3u, false},
    {"03.map_a_uvscale_g", 1u, 1u, false},
    {"05.map_a_panning_y", 1u, 3u, false},
    {"09.map_b_panning_x", 2u, 2u, false},
    {"07.map_b_uvscale_r", 2u, 0u, false},
    {"08.map_b_uvscale_g", 2u, 1u, false},
    {"10.map_b_panning_y", 2u, 3u, false},
    {"00.noisepower", 0u, 0u, false},
    {"00.noisestr", 0u, 1u, false},
    {"01.range", 0u, 2u, false},
    {"02.power", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_1025 = {{
    {"--usefakedepthbiasalpha", true},
    {"checkisdepthbiasalpha", false},
    {"00.checkisnoiseon", true},
    {"12.uvnoise", false},
    {"11.uvmirroring", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1027 = {{"dissolve_noise_tex","uv_noise_01_tex","emissive_tex","alpha_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,16> WARLORD_NATIVE_PARAMETERS_1027 = {{
    {"selectioncolor", 5u, 0u, true},
    {"dissolve_color&intensity", 4u, 0u, true},
    {"dissolve_noise_intensity", 0u, 0u, false},
    {"dissolve_noise_power", 0u, 1u, false},
    {"dissolve_power", 0u, 2u, false},
    {"uv_noise_01_panning_x", 2u, 1u, false},
    {"uv_noise_01_tiling_x", 2u, 3u, false},
    {"uv_noise_01_tiling_y", 3u, 0u, false},
    {"uv_noise_01_panning_y", 2u, 2u, false},
    {"uv_noise_01_intensity", 2u, 0u, false},
    {"uv_scale", 3u, 1u, false},
    {"dynamic_parameter_explanation", 0u, 3u, false},
    {"emissive_desaturation", 1u, 2u, false},
    {"edge_intensity", 1u, 0u, false},
    {"edge_power", 1u, 1u, false},
    {"emissive_power", 1u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,14> WARLORD_NATIVE_SWITCHES_1027 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_lamp_alpha", false},
    {"use_dissolve", true},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", true},
    {"non_alpha_chanel", false},
    {"use_uv_noise_tex", true},
    {"use_uv_noise_tex_02", false},
    {"use_fresnel_alpha", false},
    {"use_edge_glow", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1028 = {{"dissolve_noise_tex","emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,12> WARLORD_NATIVE_PARAMETERS_1028 = {{
    {"selectioncolor", 4u, 0u, true},
    {"dissolve_color&intensity", 3u, 0u, true},
    {"dissolve_noise_intensity", 0u, 1u, false},
    {"dissolve_noise_power", 0u, 2u, false},
    {"dissolve_power", 0u, 3u, false},
    {"uv_scale", 2u, 1u, false},
    {"dynamic_parameter_explanation", 1u, 0u, false},
    {"emissive_desaturation", 1u, 3u, false},
    {"edge_intensity", 1u, 1u, false},
    {"edge_power", 1u, 2u, false},
    {"emissive_power", 2u, 0u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,14> WARLORD_NATIVE_SWITCHES_1028 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_lamp_alpha", false},
    {"use_dissolve", true},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"non_alpha_chanel", false},
    {"use_uv_noise_tex", false},
    {"use_uv_noise_tex_02", false},
    {"use_fresnel_alpha", false},
    {"use_edge_glow", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1029 = {{"native_texture_0","01.map_uv","01.map_a","11.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1029 = {{
    {"selectioncolor", 2u, 0u, true},
    {"09.str", 0u, 0u, false},
    {"10.power", 0u, 1u, false},
    {"12.map_b_uvscale", 0u, 2u, false},
    {"18.str", 0u, 3u, false},
    {"19.power", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,8> WARLORD_NATIVE_SWITCHES_1029 = {{
    {"00.checkisdepthbiasalpha", false},
    {"01.usemesh", false},
    {"06.mapch.r", false},
    {"07.mapch.g", false},
    {"08.mapch.b", false},
    {"00.useemissionmap", false},
    {"30.usefresnal", false},
    {"31.fresnal.invert", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1030 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,2> WARLORD_NATIVE_PARAMETERS_1030 = {{
    {"selectioncolor", 1u, 0u, true},
    {"hardness", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1030 = {{
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1031 = {{"native_texture_0","native_texture_1"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,2> WARLORD_NATIVE_PARAMETERS_1031 = {{
    {"selectioncolor", 1u, 0u, true},
    {"meshemitterdynamicparameter", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1031 = {{
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1032 = {{"tex_colormap","tex_diffmap","tex_maskmap"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,22> WARLORD_NATIVE_PARAMETERS_1032 = {{
    {"selectioncolor", 6u, 0u, true},
    {"colormap_color", 5u, 0u, true},
    {"mainuv_upanner", 3u, 2u, false},
    {"mainuv_ucoord", 3u, 1u, false},
    {"twist_str", 4u, 3u, false},
    {"mainuv_curve_power", 3u, 0u, false},
    {"mainuv_v_coord", 3u, 3u, false},
    {"mainuv_v_panner", 4u, 0u, false},
    {"colormap_uspeed", 1u, 0u, false},
    {"colormap_ucoord", 0u, 3u, false},
    {"colormap_vspeed", 1u, 2u, false},
    {"colormap_vcoord", 1u, 1u, false},
    {"colormap_pow", 0u, 1u, false},
    {"colormap_str", 0u, 2u, false},
    {"colormap_des", 0u, 0u, false},
    {"diff_uspeed", 2u, 0u, false},
    {"diff_ucoord", 1u, 3u, false},
    {"diff_vspeed", 2u, 2u, false},
    {"diff_vcoord", 2u, 1u, false},
    {"main_str", 2u, 3u, false},
    {"mask_power", 4u, 1u, false},
    {"mask_str", 4u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1032 = {{
    {"mainuv_v_pannertime", false},
    {"use_dynamic_twist", false},
    {"mainuv_u_pannertime", true},
    {"use_alpha", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1033 = {{"maintex","uv_noise_tex_02","uv_noise_tex","dissolve_tex_01"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,39> WARLORD_NATIVE_PARAMETERS_1033 = {{
    {"selectioncolor", 11u, 0u, true},
    {"uvnoise_move_x", 8u, 3u, false},
    {"uvnoise_move_y", 9u, 0u, false},
    {"maintex_move_x", 3u, 3u, false},
    {"maintex_move_y", 4u, 0u, false},
    {"maintex_rotator", 4u, 3u, false},
    {"dissolvetex_rotator", 1u, 3u, false},
    {"edge_color", 10u, 0u, true},
    {"maintex_panspeed_x", 4u, 1u, false},
    {"maintex_tile_x", 5u, 0u, false},
    {"maintex_tile_y", 5u, 1u, false},
    {"maintex_panspeed_y", 4u, 2u, false},
    {"maintex_dynamicpan_x_velue", 3u, 1u, false},
    {"maintex_dynamicpan_y_velue", 3u, 2u, false},
    {"uv_noisetex_pan_x", 7u, 1u, false},
    {"uv_noisetex_tile_x", 8u, 1u, false},
    {"uv_noisetex_tile_y", 8u, 2u, false},
    {"uv_noisetex_pan_02_x", 6u, 3u, false},
    {"uv_noisetex_tile_02_x", 7u, 3u, false},
    {"uv_noisetex_tile_02_y", 8u, 0u, false},
    {"uv_noisetex_pan_02_y", 7u, 0u, false},
    {"uv_noise_02_strength", 6u, 1u, false},
    {"uv_noisetex_pan_y", 7u, 2u, false},
    {"uv_noise_velue", 6u, 2u, false},
    {"emissive_core_power", 2u, 2u, false},
    {"emissive_core_strength", 2u, 3u, false},
    {"emissive_base", 2u, 1u, false},
    {"disslovetex_01_panspeed_x", 0u, 0u, false},
    {"disslovetex_01_tile_x", 0u, 2u, false},
    {"disslovetex_01_tile_y", 0u, 3u, false},
    {"disslovetex_01_panspeed_y", 0u, 1u, false},
    {"dissolvetex_move_x", 1u, 1u, false},
    {"dissolvetex_move_y", 1u, 2u, false},
    {"dissolve_hardness", 1u, 0u, false},
    {"edge_thin", 2u, 0u, false},
    {"spheremask_strength", 5u, 2u, false},
    {"spheremask_strength_max", 5u, 3u, false},
    {"spheremask_strength_min", 6u, 0u, false},
    {"maintex_alpha_strength", 3u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,20> WARLORD_NATIVE_SWITCHES_1033 = {{
    {"use_dissolve", true},
    {"use_meshtype", false},
    {"use_uvnoise", true},
    {"achannel_use_uvnoisedynamicpan", false},
    {"use_uv_noise_02", true},
    {"use_uv_noisetex_02", true},
    {"uv_noisetype", true},
    {"maintex_type_polar", true},
    {"maintex_pan_xy", false},
    {"use_dynamic_spheryxy", false},
    {"uv_sphery_y", false},
    {"uv_sphery_x", false},
    {"use_alpha_fresnel", false},
    {"use_dissolvetex_02", false},
    {"use_dissolvetex_noise", false},
    {"use_dissolvetex_noise_type", true},
    {"use_dissolvetex_02_mixtype", true},
    {"use_sphere_alpha", true},
    {"dissloveoutline_multyply_alpha", false},
    {"use_emissivetex02", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1034 = {{"dissolve_noise_tex","emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,10> WARLORD_NATIVE_PARAMETERS_1034 = {{
    {"selectioncolor", 3u, 0u, true},
    {"dissolve_color&intensity", 2u, 0u, true},
    {"dissolve_noise_intensity", 0u, 1u, false},
    {"dissolve_noise_power", 0u, 2u, false},
    {"dissolve_power", 0u, 3u, false},
    {"uv_scale", 1u, 3u, false},
    {"dynamic_parameter_explanation", 1u, 0u, false},
    {"emissive_desaturation", 1u, 1u, false},
    {"emissive_power", 1u, 2u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,14> WARLORD_NATIVE_SWITCHES_1034 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_lamp_alpha", false},
    {"use_dissolve", true},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"non_alpha_chanel", false},
    {"use_uv_noise_tex", false},
    {"use_uv_noise_tex_02", false},
    {"use_fresnel_alpha", false},
    {"use_edge_glow", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1036 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,2> WARLORD_NATIVE_PARAMETERS_1036 = {{
    {"selectioncolor", 1u, 0u, true},
    {"dynamic_parameter_explanation", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,12> WARLORD_NATIVE_SWITCHES_1036 = {{
    {"use_spread texture", false},
    {"use_detaildistortion", false},
    {"use_centerdistortion", true},
    {"use_meshtype", false},
    {"use_up&down", false},
    {"use_xy axis", false},
    {"use_billboard", false},
    {"use_sharp", false},
    {"use_twirl", false},
    {"use_tworing", true},
    {"use_wavering", false},
    {"use_center texture", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1037 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1037 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_1037 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1038 = {{"01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,20> WARLORD_NATIVE_PARAMETERS_1038 = {{
    {"selectioncolor", 5u, 0u, true},
    {"11.direct", 2u, 3u, false},
    {"50.angle", 4u, 2u, false},
    {"11.width.power", 3u, 1u, false},
    {"12.width.strength", 3u, 3u, false},
    {"01.height.power", 0u, 1u, false},
    {"02.height.strength", 0u, 2u, false},
    {"04.map_a_panning_x", 1u, 1u, false},
    {"02.map_a_uvscale_r", 0u, 3u, false},
    {"03.map_a_uvscale_g", 1u, 0u, false},
    {"05.map_a_panning_y", 1u, 2u, false},
    {"09.map_b_panning_x", 2u, 1u, false},
    {"07.map_b_uvscale_r", 1u, 3u, false},
    {"08.map_b_uvscale_g", 2u, 0u, false},
    {"10.map_b_panning_y", 2u, 2u, false},
    {"31.noisepower", 4u, 0u, false},
    {"32.noisestr", 4u, 1u, false},
    {"11.range", 3u, 0u, false},
    {"12.power", 3u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,12> WARLORD_NATIVE_SWITCHES_1038 = {{
    {"00.checkisdepthbiasalpha", true},
    {"10.usefakedepthbiasalpha", true},
    {"01.useconeuv", true},
    {"30.usefresnal", false},
    {"00.usenoise", true},
    {"00.blendingadd", false},
    {"30.use((map_a+map_b)*0.5)", false},
    {"00.useuvnoise", false},
    {"01.usemesh", false},
    {"00.usecolor", false},
    {"01.usenoise", false},
    {"10.usemapcolor", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1039 = {{"subuv_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_1039 = {{
    {"selectioncolor", 1u, 0u, true},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 1u, false},
    {"emissive_power", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1039 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", false},
    {"use_subuv_tex", true},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"non_alpha_chanel", false},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1040 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_1040 = {{
    {"selectioncolor", 1u, 0u, true},
    {"10.radius", 0u, 2u, false},
    {"04.ringthickness", 0u, 0u, false},
    {"07.power", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,7> WARLORD_NATIVE_SWITCHES_1040 = {{
    {"00.usedistortion", false},
    {"00.use_uvdistort", false},
    {"00.alphamap", false},
    {"20.usehemisphere", false},
    {"00.useemissioncolor", false},
    {"00.checkisdepthbiasalpha", false},
    {"10.usesubmap", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1041 = {{"native_texture_0","native_texture_1"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1041 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1041 = {{
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1042 = {{"tex_uv_flow","tex_alpha_01","tex_alpha_02"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,11> WARLORD_NATIVE_PARAMETERS_1042 = {{
    {"selectioncolor", 3u, 0u, true},
    {"tex_alpha_01_rotator", 1u, 1u, false},
    {"tex_alpha_01_r_tile", 1u, 0u, false},
    {"tex_alpha_01_g_tile", 0u, 3u, false},
    {"tex_flow_r_tile", 2u, 1u, false},
    {"tex_flow_g_tile", 2u, 0u, false},
    {"emissive_velue_power", 0u, 2u, false},
    {"emissive_velue_multiply", 0u, 1u, false},
    {"emissive_velue_background", 0u, 0u, false},
    {"tex_alpha_02_r_tile", 1u, 3u, false},
    {"tex_alpha_02_g_tile", 1u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,2> WARLORD_NATIVE_SWITCHES_1042 = {{
    {"use_mesh", false},
    {"alpha_01_rotator_use", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1043 = {{"native_texture_0","texturesample","tex_01","opacity_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1043 = {{
    {"selectioncolor", 2u, 0u, true},
    {"meshemitterdynamicparameter", 1u, 0u, true},
    {"tex_alpha_02_r_tile", 0u, 3u, false},
    {"tex_alpha_02_g_tile", 0u, 2u, false},
    {"tex_alpha_01_r_tile", 0u, 1u, false},
    {"tex_alpha_01_g_tile", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,2> WARLORD_NATIVE_SWITCHES_1043 = {{
    {"spheremask", true},
    {"01.useparticle", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1044 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1044 = {{
    {"selectioncolor", 2u, 0u, true},
    {"centerglow_power", 0u, 0u, false},
    {"centerglow_str", 0u, 1u, false},
    {"glow_power", 0u, 2u, false},
    {"str", 1u, 0u, false},
    {"power", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1044 = {{
    {"use_distortion", false},
    {"use_centerglow", true},
    {"use_centermask", false},
    {"use_depth_alpha", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1045 = {{"native_texture_0","native_texture_1"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,2> WARLORD_NATIVE_PARAMETERS_1045 = {{
    {"selectioncolor", 1u, 0u, true},
    {"rotatevalue", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_1045 = {{
    {"meshvertexalpha", true},
    {"meshvertexemis", true},
    {"rotate", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1046 = {{"uv_noise_01_tex","emissive_tex","alpha_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,11> WARLORD_NATIVE_PARAMETERS_1046 = {{
    {"selectioncolor", 3u, 0u, true},
    {"uv_noise_01_panning_x", 1u, 1u, false},
    {"uv_noise_01_tiling_x", 1u, 3u, false},
    {"uv_noise_01_tiling_y", 2u, 0u, false},
    {"uv_noise_01_panning_y", 1u, 2u, false},
    {"uv_noise_01_intensity", 1u, 0u, false},
    {"uv_scale", 2u, 1u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1046 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", true},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", true},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"non_alpha_chanel", false},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1047 = {{"colormap","native_texture_1","tex_main","native_texture_3"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,15> WARLORD_NATIVE_PARAMETERS_1047 = {{
    {"selectioncolor", 5u, 0u, true},
    {"colormap_pos_x", 0u, 3u, false},
    {"colormap_pos_y", 1u, 0u, false},
    {"colormap_color", 4u, 0u, true},
    {"maintex_pos_u", 1u, 3u, false},
    {"maintex_pos_v", 2u, 0u, false},
    {"maintex_speed", 2u, 1u, false},
    {"colormap_coord_x", 0u, 0u, false},
    {"colormap_coord_y", 0u, 1u, false},
    {"colormap_des", 0u, 2u, false},
    {"colormap_power", 1u, 1u, false},
    {"colormap_str", 1u, 2u, false},
    {"maintex_ucoord", 2u, 3u, false},
    {"maintex_vcoord", 3u, 0u, false},
    {"maintex_str", 2u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1047 = {{
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1048 = {{"uv_noise_tex","native_texture_1","maintex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,33> WARLORD_NATIVE_PARAMETERS_1048 = {{
    {"selectioncolor", 9u, 0u, true},
    {"meshemitterdynamicparameter", 8u, 0u, true},
    {"maintex_rotator", 5u, 1u, false},
    {"maintex_move_u", 4u, 1u, false},
    {"maintex_move_v", 4u, 2u, false},
    {"maintex_pan_u_time", 4u, 3u, false},
    {"main_panspeed_u", 2u, 3u, false},
    {"maintex_texcoord_u", 5u, 2u, false},
    {"maintex_texcoord_v", 5u, 3u, false},
    {"main_panspeed_v", 3u, 0u, false},
    {"uv_noise_pan_u", 6u, 0u, false},
    {"uv_noise_panspeed_u", 6u, 2u, false},
    {"uv_noise_texcoord_u", 7u, 0u, false},
    {"uv_noise_texcoord_v", 7u, 1u, false},
    {"uv_noise_panspeed_v", 6u, 3u, false},
    {"uv_noise_pan_v", 6u, 1u, false},
    {"uv_noise_velue", 7u, 2u, false},
    {"maintex_pan_v_time", 5u, 0u, false},
    {"maintex_desaturation", 4u, 0u, false},
    {"main_tex_power", 3u, 2u, false},
    {"main_tex_power_multiply", 3u, 3u, false},
    {"main_tex_background_velue", 3u, 1u, false},
    {"dissolve_pan_u_time", 1u, 1u, false},
    {"dissolve_pan_u_speed", 1u, 0u, false},
    {"dissolve_texcoord_u", 2u, 0u, false},
    {"dissolve_texcoord_v", 2u, 1u, false},
    {"dissolve_pan_v_speed", 1u, 2u, false},
    {"dissolve_pan_v_time", 1u, 3u, false},
    {"disslove_hardness", 0u, 3u, false},
    {"camera_vector_fresnel_velue", 0u, 1u, false},
    {"alpha_strength", 0u, 0u, false},
    {"depthbias_velue", 0u, 2u, false},
    {"disto_power", 2u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,13> WARLORD_NATIVE_SWITCHES_1048 = {{
    {"usemesh", true},
    {"use_camera_vector_fresnel", true},
    {"use_distortion", true},
    {"use_outfalloff", false},
    {"outalpha_falloff_v", true},
    {"outalpha_falloff_u", true},
    {"use_alpha_channal", false},
    {"b_channal", false},
    {"g_channal", false},
    {"r_channal", false},
    {"use_depthbias", true},
    {"use_reflction", false},
    {"uv_noise_on", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1049 = {{"native_texture_0","native_texture_1"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,2> WARLORD_NATIVE_PARAMETERS_1049 = {{
    {"selectioncolor", 1u, 0u, true},
    {"depth_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,1> WARLORD_NATIVE_SWITCHES_1049 = {{
    {"use_depth", true},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1050 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1050 = {{
    {"selectioncolor", 2u, 0u, true},
    {"01.color", 1u, 0u, true},
    {"01.radius", 0u, 0u, false},
    {"02.hardness", 0u, 1u, false},
    {"03.str", 0u, 2u, false},
    {"04.power", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1050 = {{
    {"00.usenoise", false},
    {"01.usemesh", false},
    {"00.checkisdepthbiasalpha", false},
    {"00.useworldposition", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1051 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1051 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_1051 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1052 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1052 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_1052 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1053 = {{"01.map"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1053 = {{
    {"selectioncolor", 2u, 0u, true},
    {"01.circlepower", 0u, 0u, false},
    {"02.circlestr", 0u, 2u, false},
    {"02.circlemaskpower", 0u, 1u, false},
    {"03.circlestr", 0u, 3u, false},
    {"depthbiasdalpha_bias", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1053 = {{
    {"checkisdepthbiasalpha", true},
    {"00.useradius", true},
    {"01.usemesh", false},
    {"10.usetimebezire", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1054 = {{"02.map_e"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,12> WARLORD_NATIVE_PARAMETERS_1054 = {{
    {"selectioncolor", 4u, 0u, true},
    {"93.emissiion_color", 3u, 0u, true},
    {"time", 2u, 1u, false},
    {"05.map_e_panning_x", 0u, 3u, false},
    {"03.map_e_uvscale_r", 0u, 1u, false},
    {"04.map_e_uvscale_g", 0u, 2u, false},
    {"06.map_e_panning_y", 1u, 0u, false},
    {"91.desaturation", 1u, 3u, false},
    {"92.emissiion_power", 2u, 0u, false},
    {"36.str", 1u, 1u, false},
    {"37.power", 1u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1054 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", false},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", false},
    {"10.use_mapb", false},
    {"00.use_mapa", false},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1055 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1055 = {{
    {"selectioncolor", 2u, 0u, true},
    {"uv_scale", 1u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"distortion_intensity", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_1055 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", true},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"use_distortion_floor", true},
    {"non_alpha_chanel", false},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex_02", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1056 = {{"02.map_e","22.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,22> WARLORD_NATIVE_PARAMETERS_1056 = {{
    {"selectioncolor", 7u, 0u, true},
    {"meshemitterdynamicparameter", 6u, 0u, true},
    {"93.emissiion_color", 5u, 0u, true},
    {"22.map_a_uvscale_r", 2u, 1u, false},
    {"23.map_a_uvscale_g", 2u, 2u, false},
    {"time", 4u, 2u, false},
    {"05.map_e_panning_x", 1u, 1u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 0u, false},
    {"11.uv.curvature", 2u, 0u, false},
    {"06.map_e_panning_y", 1u, 2u, false},
    {"91.desaturation", 3u, 2u, false},
    {"92.emissiion_power", 3u, 3u, false},
    {"02.radius", 0u, 1u, false},
    {"03.hardness", 0u, 2u, false},
    {"09.power", 1u, 3u, false},
    {"24.map_a_panning_x", 2u, 3u, false},
    {"95.str", 4u, 0u, false},
    {"96.power", 4u, 1u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"32.fresnal_power", 3u, 0u, false},
    {"33.fresnal_str", 3u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,27> WARLORD_NATIVE_SWITCHES_1056 = {{
    {"00.usespecullar", false},
    {"11.mape+mapf", false},
    {"10.use_mapb", false},
    {"01.use_emissionmap", true},
    {"01.usemesh", true},
    {"05.useinvert", false},
    {"00.usedistortion", false},
    {"01.usesphmask", true},
    {"25.xinvert", false},
    {"20.usetwirl", false},
    {"01.usecoordinate_index1", false},
    {"94.sphmask+(texmap*phasemap)", false},
    {"01.effectonalphatex", false},
    {"11.useclamptexmap", false},
    {"91.mapch.r", true},
    {"92.mapch.g", true},
    {"00.checkisdepthbiasalpha", true},
    {"21.usephasetexmap", true},
    {"30.usefresnal", true},
    {"31.fresnal.invert", false},
    {"00.use_uvdistort", false},
    {"29.usedynamic_ypanning", true},
    {"23.usedynamicstr", false},
    {"93.mapch.b", true},
    {"10.fresnal", true},
    {"99.alphadissolve", false},
    {"94.mapch.a", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1057 = {{"01.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,8> WARLORD_NATIVE_PARAMETERS_1057 = {{
    {"selectioncolor", 4u, 0u, true},
    {"16.color", 2u, 0u, true},
    {"meshemitterdynamicparameter", 3u, 0u, true},
    {"11.direct", 1u, 0u, false},
    {"05.uv.y.strech", 0u, 1u, false},
    {"09.str", 0u, 2u, false},
    {"10.power", 0u, 3u, false},
    {"00.introduction", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,11> WARLORD_NATIVE_SWITCHES_1057 = {{
    {"00.usedistortion", false},
    {"50.use_fresnal", false},
    {"06.mapch.r", true},
    {"07.mapch.g", true},
    {"08.mapch.b", true},
    {"01.usemesh", true},
    {"00.use_uvdistort", false},
    {"00.useemissionmap", false},
    {"--usespecullar", false},
    {"17.useheademission", false},
    {"01.texcoord.index1", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1058 = {{"01.map_e","01.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,19> WARLORD_NATIVE_PARAMETERS_1058 = {{
    {"selectioncolor", 6u, 0u, true},
    {"19.emissiion_color", 5u, 0u, true},
    {"04.map_e_panning_x", 1u, 2u, false},
    {"02.map_e_uvscale_r", 0u, 2u, false},
    {"03.map_e_uvscale_g", 1u, 0u, false},
    {"05.map_e_panning_y", 2u, 0u, false},
    {"12.desaturation", 2u, 3u, false},
    {"15.emissiion_power", 3u, 0u, false},
    {"16.emissiion_str", 3u, 1u, false},
    {"01.maxangle", 0u, 0u, false},
    {"51.fan_hardness", 4u, 0u, false},
    {"41.ring_hardness", 3u, 2u, false},
    {"42.ring_power", 3u, 3u, false},
    {"04.map_a_panning_x", 1u, 1u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 3u, false},
    {"05.map_a_panning_y", 1u, 3u, false},
    {"09.str", 2u, 1u, false},
    {"10.power", 2u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,8> WARLORD_NATIVE_SWITCHES_1058 = {{
    {"06.mapch.r", false},
    {"07.mapch.g", true},
    {"08.mapch.b", true},
    {"00.use_uvdistort", false},
    {"55.spread_side2side", false},
    {"56.spread_invert", false},
    {"00.usedistortion", false},
    {"00.checkisdepthbiasalpha", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1059 = {{"02.map_e","06.map","22.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,25> WARLORD_NATIVE_PARAMETERS_1059 = {{
    {"selectioncolor", 7u, 0u, true},
    {"93.emissiion_color", 6u, 0u, true},
    {"22.map_a_uvscale_r", 4u, 0u, false},
    {"23.map_a_uvscale_g", 4u, 1u, false},
    {"05.map_e_panning_x", 1u, 2u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 0u, false},
    {"11.uv.curvature", 3u, 3u, false},
    {"09.map_d_panning_x", 3u, 0u, false},
    {"07.map_d_uvscale_r", 2u, 2u, false},
    {"08.map_d_uvscale_g", 2u, 3u, false},
    {"10.map_d_panning_y", 3u, 2u, false},
    {"05.distort_str", 1u, 1u, false},
    {"06.map_e_panning_y", 1u, 3u, false},
    {"91.desaturation", 4u, 3u, false},
    {"92.emissiion_power", 5u, 0u, false},
    {"06.radius", 2u, 0u, false},
    {"07.hardness", 2u, 1u, false},
    {"02.radius", 0u, 1u, false},
    {"03.hardness", 0u, 2u, false},
    {"09.power", 3u, 1u, false},
    {"24.map_a_panning_x", 4u, 2u, false},
    {"95.str", 5u, 1u, false},
    {"96.power", 5u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,27> WARLORD_NATIVE_SWITCHES_1059 = {{
    {"00.usedistortion", false},
    {"30.usefresnal", false},
    {"31.fresnal.invert", false},
    {"00.checkisdepthbiasalpha", true},
    {"94.sphmask+(texmap*phasemap)", false},
    {"91.mapch.r", false},
    {"92.mapch.g", false},
    {"21.usephasetexmap", true},
    {"29.usedynamic_ypanning", true},
    {"01.usemesh", false},
    {"00.use_uvdistort", true},
    {"20.usetwirl", false},
    {"01.usecoordinate_index1", false},
    {"25.xinvert", false},
    {"23.usedynamicstr", false},
    {"11.useclamptexmap", false},
    {"01.effectonalphatex", false},
    {"01.usesphmask", true},
    {"05.useinvert", true},
    {"01.use_emissionmap", true},
    {"10.use_mapb", false},
    {"11.mape+mapf", false},
    {"00.usespecullar", false},
    {"93.mapch.b", false},
    {"10.fresnal", true},
    {"99.alphadissolve", false},
    {"94.mapch.a", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1060 = {{"map_a","map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_1060 = {{
    {"selectioncolor", 1u, 0u, true},
    {"readme", 0u, 3u, false},
    {"map_a_uvscale", 0u, 0u, false},
    {"map_b_uvscale", 0u, 1u, false},
    {"map_str", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,7> WARLORD_NATIVE_SWITCHES_1060 = {{
    {"00.uselightshift", false},
    {"01.use_depthbiasalpha", false},
    {"btimebezier", true},
    {"00.userainbow", false},
    {"20.usespacular", false},
    {"01.usemacrouv", false},
    {"01.usemesh", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1061 = {{"01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,16> WARLORD_NATIVE_PARAMETERS_1061 = {{
    {"selectioncolor", 5u, 0u, true},
    {"meshemitterdynamicparameter", 4u, 0u, true},
    {"21.uvscale.x", 1u, 1u, false},
    {"22.uvscale.y", 1u, 2u, false},
    {"time", 3u, 1u, false},
    {"04.map_a_panning_x", 0u, 2u, false},
    {"02.map_a_uvscale_r", 0u, 0u, false},
    {"03.map_a_uvscale_g", 0u, 1u, false},
    {"05.map_a_panning_y", 1u, 0u, false},
    {"29.cmap.direct", 1u, 3u, false},
    {"30.cmap.time(rotrate)", 2u, 0u, false},
    {"36.str", 2u, 3u, false},
    {"37.power", 3u, 0u, false},
    {"32.fresnal_power", 2u, 1u, false},
    {"33.fresnal_str", 2u, 2u, false},
    {"05.distortion_str", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1061 = {{
    {"00.usedistortion", true},
    {"01.usemesh", true},
    {"31.mapch.r", false},
    {"32.mapch.g", false},
    {"20.use_clampmap", true},
    {"25.use_yclamp", true},
    {"00.use_uvdistort", false},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", false},
    {"00.use_emission", false},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1062 = {{"06.map","01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,17> WARLORD_NATIVE_PARAMETERS_1062 = {{
    {"selectioncolor", 4u, 0u, true},
    {"21.uvscale.x", 2u, 1u, false},
    {"22.uvscale.y", 2u, 2u, false},
    {"time", 3u, 3u, false},
    {"04.map_a_panning_x", 0u, 2u, false},
    {"02.map_a_uvscale_r", 0u, 0u, false},
    {"03.map_a_uvscale_g", 0u, 1u, false},
    {"09.map_d_panning_x", 1u, 3u, false},
    {"07.map_d_uvscale_r", 1u, 1u, false},
    {"08.map_d_uvscale_g", 1u, 2u, false},
    {"10.map_d_panning_y", 2u, 0u, false},
    {"05.distort_str", 0u, 3u, false},
    {"05.map_a_panning_y", 1u, 0u, false},
    {"29.cmap.direct", 2u, 3u, false},
    {"30.cmap.time(rotrate)", 3u, 0u, false},
    {"36.str", 3u, 1u, false},
    {"37.power", 3u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1062 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", false},
    {"00.use_emission", false},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1063 = {{"uv_noise_01_tex","emissive_tex","alpha_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,13> WARLORD_NATIVE_PARAMETERS_1063 = {{
    {"selectioncolor", 3u, 0u, true},
    {"uv_noise_01_panning_x", 1u, 3u, false},
    {"uv_noise_01_tiling_x", 2u, 1u, false},
    {"uv_noise_01_tiling_y", 2u, 2u, false},
    {"uv_noise_01_panning_y", 2u, 0u, false},
    {"uv_noise_01_intensity", 1u, 2u, false},
    {"uv_scale", 2u, 3u, false},
    {"dynamic_parameter_explanation", 0u, 2u, false},
    {"emissive_desaturation", 0u, 3u, false},
    {"emissive_power", 1u, 0u, false},
    {"depth_alpha_bias", 0u, 1u, false},
    {"camera_distance", 0u, 0u, false},
    {"fresnel_power", 1u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1063 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", true},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", true},
    {"use_fresnel_alpha", true},
    {"use_camera_alpha", true},
    {"use_edge_glow", false},
    {"non_alpha_chanel", false},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1064 = {{"uv_noise_tex","native_texture_1","maintex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,31> WARLORD_NATIVE_PARAMETERS_1064 = {{
    {"selectioncolor", 9u, 0u, true},
    {"meshemitterdynamicparameter", 8u, 0u, true},
    {"maintex_rotator", 4u, 3u, false},
    {"maintex_move_u", 3u, 3u, false},
    {"maintex_move_v", 4u, 0u, false},
    {"maintex_pan_u_time", 4u, 1u, false},
    {"main_panspeed_u", 2u, 1u, false},
    {"maintex_texcoord_u", 5u, 0u, false},
    {"maintex_texcoord_v", 5u, 1u, false},
    {"main_panspeed_v", 2u, 2u, false},
    {"uv_noise_pan_u", 5u, 2u, false},
    {"uv_noise_panspeed_u", 6u, 0u, false},
    {"uv_noise_texcoord_u", 6u, 2u, false},
    {"uv_noise_texcoord_v", 6u, 3u, false},
    {"uv_noise_panspeed_v", 6u, 1u, false},
    {"uv_noise_pan_v", 5u, 3u, false},
    {"uv_noise_velue", 7u, 0u, false},
    {"maintex_pan_v_time", 4u, 2u, false},
    {"maintex_desaturation", 3u, 2u, false},
    {"main_tex_power", 3u, 0u, false},
    {"main_tex_power_multiply", 3u, 1u, false},
    {"main_tex_background_velue", 2u, 3u, false},
    {"dissolve_pan_u_time", 1u, 0u, false},
    {"dissolve_pan_u_speed", 0u, 3u, false},
    {"dissolve_texcoord_u", 1u, 3u, false},
    {"dissolve_texcoord_v", 2u, 0u, false},
    {"dissolve_pan_v_speed", 1u, 1u, false},
    {"dissolve_pan_v_time", 1u, 2u, false},
    {"disslove_hardness", 0u, 2u, false},
    {"alpha_strength", 0u, 0u, false},
    {"depthbias_velue", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,13> WARLORD_NATIVE_SWITCHES_1064 = {{
    {"usemesh", true},
    {"use_camera_vector_fresnel", false},
    {"use_distortion", false},
    {"use_outfalloff", false},
    {"outalpha_falloff_v", true},
    {"outalpha_falloff_u", true},
    {"use_alpha_channal", false},
    {"b_channal", false},
    {"g_channal", false},
    {"r_channal", false},
    {"use_depthbias", true},
    {"use_reflction", false},
    {"uv_noise_on", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1065 = {{"native_texture_0"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1065 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1065 = {{
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1066 = {{"06.map_anew","06.map","00.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,13> WARLORD_NATIVE_PARAMETERS_1066 = {{
    {"selectioncolor", 3u, 0u, true},
    {"06.map_alod", 0u, 1u, false},
    {"09.map_a_panning_x", 1u, 2u, false},
    {"07.map_a_uvscale_r", 0u, 2u, false},
    {"08.map_a_uvscale_g", 1u, 0u, false},
    {"10.map_a_panning_y", 2u, 0u, false},
    {"09.map_d_panning_x", 1u, 3u, false},
    {"07.map_d_uvscale_r", 0u, 3u, false},
    {"08.map_d_uvscale_g", 1u, 1u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 0u, 0u, false},
    {"11.map_a_str", 2u, 2u, false},
    {"12.map_a_power", 2u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,15> WARLORD_NATIVE_SWITCHES_1066 = {{
    {"00.usedistortion", false},
    {"20.use_centrehole", false},
    {"00.usedepthbiasalpha", false},
    {"30.usefresnal", false},
    {"10.cameradistance", false},
    {"01.mapch.r", true},
    {"02.mapch.g", true},
    {"03.mapch.b", true},
    {"00.use_uvdistort", true},
    {"01.usecoordinate_index1", false},
    {"01.use_uvdistort_all", true},
    {"15.use_panbezier", false},
    {"checkismeshemitvc", false},
    {"00.use_rgbinalphamap", false},
    {"30.use_additionalmap", false},
}};

inline constexpr std::array<std::string_view,6> WARLORD_NATIVE_TEXTURES_1067 = {{"uv_noise_tex","emissive_tex01","alpha_tex02","alpha_tex","dissolve_tex02","uv_dissolve_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,47> WARLORD_NATIVE_PARAMETERS_1067 = {{
    {"selectioncolor", 12u, 0u, true},
    {"emissive_tex_core_positon_x", 8u, 0u, false},
    {"emissive_tex_core_positon_y", 8u, 1u, false},
    {"alpha_tex_rotator", 4u, 1u, false},
    {"alpha_tex02_positon_x", 2u, 0u, false},
    {"alpha_tex02_positon_y", 2u, 1u, false},
    {"alpha_tex_positon_x", 3u, 2u, false},
    {"alpha_tex_positon_y", 3u, 3u, false},
    {"emissive_tex_core_x", 8u, 2u, false},
    {"emissive_tex_core_y", 9u, 0u, false},
    {"uvnoise_tex_01_texcoord_x", 10u, 2u, false},
    {"uvnoise_tex_01_texcoord_y", 10u, 3u, false},
    {"uv_noise_head_velue", 10u, 1u, false},
    {"uvnoise_tex_dynamicpanspeed_x", 11u, 0u, false},
    {"uvnoise_tex_dynamicpanspeed_y", 11u, 1u, false},
    {"emissive_tex01tile_dynamicpanspeed_x", 6u, 3u, false},
    {"emissive_tex01tile_dynamicpanspeed_y", 7u, 0u, false},
    {"emissive_tex_core_x_02", 8u, 3u, false},
    {"emissive_tex_core_y_02", 9u, 1u, false},
    {"emissive_tex02tile_dynamicpanspeed_x", 7u, 1u, false},
    {"emissive_tex02tile_dynamicpanspeed_y", 7u, 2u, false},
    {"emissive_tex_power", 9u, 2u, false},
    {"emissive_tex_strength", 9u, 3u, false},
    {"emissive_tex_backvelue", 7u, 3u, false},
    {"alpha_tex02_texcoord_x", 2u, 2u, false},
    {"alpha_tex02_texcoord_y", 2u, 3u, false},
    {"alpha_tex02_dynamicpanspeed_x", 1u, 2u, false},
    {"alpha_tex02_dynamicpanspeed_y", 1u, 3u, false},
    {"alpha_tex_texcoord_x", 4u, 3u, false},
    {"alpha_tex_texcoord_y", 5u, 0u, false},
    {"maintex_uv_noise_velue", 10u, 0u, false},
    {"alpha_tex_dynamicpanspeed_x", 3u, 0u, false},
    {"alpha_tex_dynamicpanspeed_y", 3u, 1u, false},
    {"alpha_tex_power", 4u, 0u, false},
    {"alpha01_tex_power", 0u, 0u, false},
    {"alpha_tex_strength", 4u, 2u, false},
    {"alpha_out_falloff", 1u, 1u, false},
    {"alpha_disslove_tex_coord_x", 0u, 3u, false},
    {"alpha_disslove_tex_coord_y", 1u, 0u, false},
    {"dissolve_tex_dynamicpanspeed_x", 6u, 1u, false},
    {"dissolve_tex_dynamicpanspeed_y", 6u, 2u, false},
    {"alpha_disslove_tex02_coord_x", 0u, 1u, false},
    {"alpha_disslove_tex02_coord_y", 0u, 2u, false},
    {"dissolve_tex02_dynamicpanspeed_x", 5u, 2u, false},
    {"dissolve_tex02_dynamicpanspeed_y", 5u, 3u, false},
    {"dissolve_tex02_strength", 6u, 0u, false},
    {"dissolve_hardness", 5u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,11> WARLORD_NATIVE_SWITCHES_1067 = {{
    {"use_mesh", false},
    {"use_fresnelalpha", false},
    {"use_dissolve_tex02", true},
    {"dissolve_rampmap", false},
    {"use_alphatex02", true},
    {"use_emissivetex01", true},
    {"use_emissivetex02", false},
    {"emissivetex_mixtype", false},
    {"use_colortex", false},
    {"use_depthbiasalpha", false},
    {"use_uv_noise_head_velue", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1068 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,8> WARLORD_NATIVE_PARAMETERS_1068 = {{
    {"selectioncolor", 3u, 0u, true},
    {"meshemitterdynamicparameter", 2u, 0u, true},
    {"uv_scale", 1u, 1u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 3u, false},
    {"edge_intensity", 0u, 1u, false},
    {"edge_power", 0u, 2u, false},
    {"emissive_power", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1068 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", true},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", true},
    {"non_alpha_chanel", false},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,7> WARLORD_NATIVE_TEXTURES_1069 = {{"diff_tex","a_mask_tex","a_noise_01_tex","a_noise_02_tex","b_mask_tex","b_noise_01_tex","b_noise_02_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,52> WARLORD_NATIVE_PARAMETERS_1069 = {{
    {"selectioncolor", 14u, 0u, true},
    {"a_rotator", 3u, 1u, false},
    {"a_noise_01_rot", 0u, 3u, false},
    {"a_noise_02_rot", 1u, 3u, false},
    {"b_rotator", 7u, 3u, false},
    {"b_noise_01_rot", 5u, 1u, false},
    {"b_noise_02_rot", 6u, 1u, false},
    {"diff_color", 13u, 0u, true},
    {"diff_tile_u", 10u, 2u, false},
    {"diff_tile_v", 10u, 3u, false},
    {"diff_pan_x", 9u, 2u, false},
    {"diff_pan_y", 9u, 3u, false},
    {"desaturation", 9u, 1u, false},
    {"a_tex_bias", 3u, 3u, false},
    {"a_sizecontrol", 3u, 2u, false},
    {"a_tile_u", 4u, 0u, false},
    {"a_tile_v", 4u, 1u, false},
    {"a_offset_x", 2u, 3u, false},
    {"a_offset_y", 3u, 0u, false},
    {"a_noise_01_tile_u", 1u, 1u, false},
    {"a_noise_01_tile_v", 1u, 2u, false},
    {"a_noise_01_str", 1u, 0u, false},
    {"a_noise_02_tile_u", 2u, 1u, false},
    {"a_noise_02_tile_v", 2u, 2u, false},
    {"a_noise_02_str", 2u, 0u, false},
    {"b_mask_debug(0or1)", 4u, 2u, false},
    {"a_mask_str", 0u, 2u, false},
    {"a_mask_pow", 0u, 1u, false},
    {"b_tex_bias", 8u, 1u, false},
    {"b_sizecontrol", 8u, 0u, false},
    {"b_tile_u", 8u, 2u, false},
    {"b_tile_v", 8u, 3u, false},
    {"b_offset_x", 7u, 1u, false},
    {"b_offset_y", 7u, 2u, false},
    {"b_noise_01_tile_u", 5u, 3u, false},
    {"b_noise_01_tile_v", 6u, 0u, false},
    {"b_noise_01_str", 5u, 2u, false},
    {"b_noise_02_tile_u", 6u, 3u, false},
    {"b_noise_02_tile_v", 7u, 0u, false},
    {"b_noise_02_str", 6u, 2u, false},
    {"a_mask_debug(0or1)", 0u, 0u, false},
    {"b_mask_str", 5u, 0u, false},
    {"b_mask_pow", 4u, 3u, false},
    {"mask_linearalpha", 11u, 2u, false},
    {"diff_str", 10u, 1u, false},
    {"diff_pow", 10u, 0u, false},
    {"opacity_pow", 12u, 0u, false},
    {"opacity_str", 12u, 1u, false},
    {"mask_radius", 11u, 3u, false},
    {"mask_density", 11u, 1u, false},
    {"depth", 9u, 0u, false},
    {"distortion", 11u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,23> WARLORD_NATIVE_SWITCHES_1069 = {{
    {"meshtype_use", false},
    {"depthalpha_use", true},
    {"meshedge_soft_use", false},
    {"coresoft_use", false},
    {"width_height_use", false},
    {"ribbon_index_use", false},
    {"mask_compose", false},
    {"b_noise_01_distort_dynuse", true},
    {"nonpolar_mask_b", true},
    {"b_pan_y_use", false},
    {"b_pan_x_dynuse", false},
    {"b_pan_x_use", false},
    {"b_blur_dyn_use", false},
    {"nonpolar_mask_a", true},
    {"a_pan_y_use", false},
    {"a_pan_x_dynuse", false},
    {"a_pan_x_use", false},
    {"a_blur_dyn_use", true},
    {"diff_polar_use", false},
    {"diff_pan_y_use", false},
    {"diff_pan_x_use", false},
    {"dissolve_use(alpha)", false},
    {"a_sizecontrol_dyuse", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1070 = {{"native_texture_0","native_texture_1"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1070 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1070 = {{
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1071 = {{"native_texture_0","native_texture_1"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1071 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1071 = {{
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1072 = {{"subuv_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_1072 = {{
    {"selectioncolor", 1u, 0u, true},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 1u, false},
    {"emissive_power", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1072 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", false},
    {"use_subuv_tex", true},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"non_alpha_chanel", true},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1073 = {{"native_texture_0","native_texture_1","native_texture_2"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1073 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1073 = {{
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1074 = {{"06.map","02.map_e","12.map_f","01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,33> WARLORD_NATIVE_PARAMETERS_1074 = {{
    {"selectioncolor", 11u, 0u, true},
    {"61.twist.xy", 8u, 0u, true},
    {"93.emissiion_color", 9u, 0u, true},
    {"meshemitterdynamicparameter", 10u, 0u, true},
    {"21.uvscale.x", 4u, 2u, false},
    {"22.uvscale.y", 4u, 3u, false},
    {"time", 7u, 0u, false},
    {"05.map_e_panning_x", 2u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 1u, false},
    {"09.map_d_panning_x", 3u, 0u, false},
    {"07.map_d_uvscale_r", 2u, 2u, false},
    {"08.map_d_uvscale_g", 2u, 3u, false},
    {"10.map_d_panning_y", 3u, 1u, false},
    {"05.distort_str", 1u, 2u, false},
    {"06.map_e_panning_y", 2u, 1u, false},
    {"15.map_f_panning_x", 4u, 0u, false},
    {"13.map_f_uvscale_r", 3u, 2u, false},
    {"14.map_f_uvscale_g", 3u, 3u, false},
    {"16.map_f_panning_y", 4u, 1u, false},
    {"91.desaturation", 6u, 2u, false},
    {"92.emissiion_power", 6u, 3u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"05.map_a_panning_y", 1u, 3u, false},
    {"29.cmap.direct", 5u, 0u, false},
    {"30.cmap.time(rotrate)", 5u, 1u, false},
    {"36.str", 6u, 0u, false},
    {"37.power", 6u, 1u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"32.fresnal_power", 5u, 2u, false},
    {"33.fresnal_str", 5u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1074 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", false},
    {"32.mapch.g", false},
    {"20.use_clampmap", true},
    {"25.use_yclamp", true},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", true},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", true},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1075 = {{"06.map","02.map_e","12.map_f","01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,33> WARLORD_NATIVE_PARAMETERS_1075 = {{
    {"selectioncolor", 11u, 0u, true},
    {"61.twist.xy", 8u, 0u, true},
    {"93.emissiion_color", 9u, 0u, true},
    {"meshemitterdynamicparameter", 10u, 0u, true},
    {"21.uvscale.x", 4u, 2u, false},
    {"22.uvscale.y", 4u, 3u, false},
    {"time", 7u, 0u, false},
    {"05.map_e_panning_x", 2u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 1u, false},
    {"09.map_d_panning_x", 3u, 0u, false},
    {"07.map_d_uvscale_r", 2u, 2u, false},
    {"08.map_d_uvscale_g", 2u, 3u, false},
    {"10.map_d_panning_y", 3u, 1u, false},
    {"05.distort_str", 1u, 2u, false},
    {"06.map_e_panning_y", 2u, 1u, false},
    {"15.map_f_panning_x", 4u, 0u, false},
    {"13.map_f_uvscale_r", 3u, 2u, false},
    {"14.map_f_uvscale_g", 3u, 3u, false},
    {"16.map_f_panning_y", 4u, 1u, false},
    {"91.desaturation", 6u, 2u, false},
    {"92.emissiion_power", 6u, 3u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"05.map_a_panning_y", 1u, 3u, false},
    {"29.cmap.direct", 5u, 0u, false},
    {"30.cmap.time(rotrate)", 5u, 1u, false},
    {"36.str", 6u, 0u, false},
    {"37.power", 6u, 1u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"32.fresnal_power", 5u, 2u, false},
    {"33.fresnal_str", 5u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1075 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", false},
    {"32.mapch.g", false},
    {"20.use_clampmap", true},
    {"25.use_yclamp", true},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", true},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", true},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1076 = {{"flowtex","diff_tex1","diff_tex2","opacity_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,26> WARLORD_NATIVE_PARAMETERS_1076 = {{
    {"selectioncolor", 7u, 0u, true},
    {"flow_tile_u", 3u, 3u, false},
    {"flow_tile_v", 4u, 0u, false},
    {"flow_pan_u", 3u, 1u, false},
    {"flow_pan_v", 3u, 2u, false},
    {"meshemitterdynamicparameter", 6u, 0u, true},
    {"diff1_pan_u", 0u, 0u, false},
    {"diff1_pan_v", 0u, 1u, false},
    {"diff2_pan_u", 1u, 0u, false},
    {"diff2_pan_v", 1u, 1u, false},
    {"opacity_tile_u", 5u, 2u, false},
    {"opacity_tile_v", 5u, 3u, false},
    {"opacity_rot", 5u, 0u, false},
    {"diff1_tile_v", 0u, 3u, false},
    {"diff1_tile_u", 0u, 2u, false},
    {"flow_bias", 3u, 0u, false},
    {"diff2_tile_v", 1u, 3u, false},
    {"diff2_tile_u", 1u, 2u, false},
    {"diff_des", 2u, 0u, false},
    {"diff_pow", 2u, 1u, false},
    {"diff_str", 2u, 2u, false},
    {"opacity_distort_str", 4u, 2u, false},
    {"gra_pow", 4u, 1u, false},
    {"opacity_pow", 4u, 3u, false},
    {"opacity_str", 5u, 1u, false},
    {"distort_str", 2u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_1076 = {{
    {"use_gra_r_channel", false},
    {"use_colormap", false},
    {"diff_pan_use_u", false},
    {"use_cam_vector", false},
    {"use_distortion", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1077 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_1077 = {{
    {"selectioncolor", 1u, 0u, true},
    {"uv_scale", 0u, 3u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 1u, false},
    {"emissive_power", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_1077 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"use_distortion_floor", false},
    {"non_alpha_chanel", false},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex_02", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1078 = {{"noise_tex","diff_tex","native_texture_2","opacity_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,27> WARLORD_NATIVE_PARAMETERS_1078 = {{
    {"selectioncolor", 9u, 0u, true},
    {"diff_high_color", 6u, 0u, true},
    {"diff_low_color", 7u, 0u, true},
    {"diff_u_tile", 1u, 2u, false},
    {"diff_v_tile", 1u, 3u, false},
    {"noise_u_tile", 2u, 3u, false},
    {"noise_v_tile", 3u, 1u, false},
    {"noise_u_pan", 2u, 2u, false},
    {"noise_v_pan", 3u, 0u, false},
    {"meshemitterdynamicparameter", 8u, 0u, true},
    {"wave_pan_speed", 5u, 0u, false},
    {"diff_rotation", 0u, 3u, false},
    {"opacity_u_tile", 4u, 1u, false},
    {"opacity_v_tile", 4u, 2u, false},
    {"opacity_rotation", 3u, 2u, false},
    {"noise_str", 2u, 1u, false},
    {"diff_u_center", 1u, 1u, false},
    {"wave_tile", 5u, 2u, false},
    {"wave_str", 5u, 1u, false},
    {"wave_noise_str", 4u, 3u, false},
    {"diff_desturation", 0u, 1u, false},
    {"diff_pow", 0u, 2u, false},
    {"diff_str", 1u, 0u, false},
    {"opacity_u_center", 4u, 0u, false},
    {"opacity_str", 3u, 3u, false},
    {"cameravec_pow", 0u, 0u, false},
    {"distortion_str", 2u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,1> WARLORD_NATIVE_SWITCHES_1078 = {{
    {"use_cameravec", true},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1079 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,7> WARLORD_NATIVE_PARAMETERS_1079 = {{
    {"selectioncolor", 2u, 0u, true},
    {"01.radius", 0u, 0u, false},
    {"02.hardness", 0u, 1u, false},
    {"03.spherepower", 0u, 2u, false},
    {"04.sphere_str", 0u, 3u, false},
    {"depthbiasdalpha_bias", 1u, 1u, false},
    {"31.fresnal_power", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1079 = {{
    {"01.checkisvertexcolor", true},
    {"02.usedynamicparam", false},
    {"30.usefresnal", true},
    {"checkisdepthbiasalpha", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1080 = {{"01.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,10> WARLORD_NATIVE_PARAMETERS_1080 = {{
    {"selectioncolor", 3u, 0u, true},
    {"time", 2u, 0u, false},
    {"04.map_a_panning_x", 0u, 2u, false},
    {"02.map_a_uvscale_r", 0u, 0u, false},
    {"03.map_a_uvscale_g", 0u, 1u, false},
    {"05.map_a_panning_y", 0u, 3u, false},
    {"36.str", 1u, 2u, false},
    {"37.power", 1u, 3u, false},
    {"32.fresnal_power", 1u, 0u, false},
    {"33.fresnal_str", 1u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1080 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", false},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", false},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", false},
    {"00.use_emission", false},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1081 = {{"02.map_e","42.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,16> WARLORD_NATIVE_PARAMETERS_1081 = {{
    {"selectioncolor", 5u, 0u, true},
    {"44.uvscale.x", 1u, 3u, false},
    {"45.uvscale.y", 2u, 0u, false},
    {"93.emissiion_color", 4u, 0u, true},
    {"time", 3u, 1u, false},
    {"05.map_e_panning_x", 0u, 3u, false},
    {"03.map_e_uvscale_r", 0u, 1u, false},
    {"04.map_e_uvscale_g", 0u, 2u, false},
    {"06.map_e_panning_y", 1u, 0u, false},
    {"49.cmap.direct", 2u, 1u, false},
    {"50.cmap.time(rotrate)", 2u, 2u, false},
    {"91.desaturation", 2u, 3u, false},
    {"92.emissiion_power", 3u, 0u, false},
    {"36.str", 1u, 1u, false},
    {"37.power", 1u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1081 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", false},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", false},
    {"10.use_mapb", false},
    {"00.use_mapa", false},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", true},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1082 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,19> WARLORD_NATIVE_PARAMETERS_1082 = {{
    {"selectioncolor", 6u, 0u, true},
    {"noisecolor", 5u, 0u, true},
    {"15.map_d_uvscale_r", 2u, 3u, false},
    {"16.map_d_uvscale_g", 3u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 2u, false},
    {"03.map_a_uvscale_g", 1u, 0u, false},
    {"04.map_a_panning_x", 1u, 2u, false},
    {"05.map_a_panning_y", 2u, 0u, false},
    {"02.map_b_uvscale_r", 0u, 3u, false},
    {"03.map_b_uvscale_g", 1u, 1u, false},
    {"04.map_b_panning_x", 1u, 3u, false},
    {"05.map_b_panning_y", 2u, 1u, false},
    {"power", 3u, 3u, false},
    {"13.distortstr", 2u, 2u, false},
    {"00.noisepower", 0u, 0u, false},
    {"00.noisestr", 0u, 1u, false},
    {"biasvalue", 3u, 1u, false},
    {"str", 4u, 0u, false},
    {"depthbiasdalpha_bias", 3u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,6> WARLORD_NATIVE_SWITCHES_1082 = {{
    {"checkisdepthbiasalpha", true},
    {"00.checkisnoiseon", true},
    {"12.uvnoise", true},
    {"11.uvmirroring", false},
    {"use_meshtype", false},
    {"use_multyply_noisecolor", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1083 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,24> WARLORD_NATIVE_PARAMETERS_1083 = {{
    {"selectioncolor", 6u, 0u, true},
    {"11.direct", 2u, 2u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 2u, false},
    {"03.map_a_uvscale_g", 0u, 3u, false},
    {"50.angle", 5u, 2u, false},
    {"17.map_d_panning_x", 4u, 2u, false},
    {"15.map_d_uvscale_r", 4u, 0u, false},
    {"16.map_d_uvscale_g", 4u, 1u, false},
    {"18.map_d_panning_y", 4u, 3u, false},
    {"13.distortstr", 3u, 3u, false},
    {"05.map_a_panning_y", 1u, 1u, false},
    {"09.map_b_panning_x", 2u, 0u, false},
    {"07.map_b_uvscale_r", 1u, 2u, false},
    {"08.map_b_uvscale_g", 1u, 3u, false},
    {"10.map_b_panning_y", 2u, 1u, false},
    {"31.noisepower", 5u, 0u, false},
    {"32.noisestr", 5u, 1u, false},
    {"11.width.power", 3u, 0u, false},
    {"12.width.strength", 3u, 2u, false},
    {"01.height.power", 0u, 0u, false},
    {"02.height.strength", 0u, 1u, false},
    {"11.range", 2u, 3u, false},
    {"12.power", 3u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,12> WARLORD_NATIVE_SWITCHES_1083 = {{
    {"00.checkisdepthbiasalpha", false},
    {"10.usefakedepthbiasalpha", true},
    {"01.useconeuv", true},
    {"30.usefresnal", false},
    {"00.usenoise", true},
    {"00.blendingadd", false},
    {"30.use((map_a+map_b)*0.5)", true},
    {"00.useuvnoise", true},
    {"00.usecolor", false},
    {"01.usenoise", false},
    {"00z.usemapcolor", false},
    {"01.usemesh", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1084 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,10> WARLORD_NATIVE_PARAMETERS_1084 = {{
    {"selectioncolor", 3u, 0u, true},
    {"powerx", 1u, 3u, false},
    {"rgb_str", 2u, 0u, false},
    {"ch_r", 0u, 2u, false},
    {"ch_g", 0u, 1u, false},
    {"ch_b", 0u, 0u, false},
    {"desaturation", 1u, 2u, false},
    {"circle radius", 0u, 3u, false},
    {"circleedge hardness", 1u, 0u, false},
    {"depth", 1u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1084 = {{
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1085 = {{"lensflaretexture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_1085 = {{
    {"selectioncolor", 1u, 0u, true},
    {"select texture(0 or 0.5)", 0u, 2u, false},
    {"desaturation", 0u, 1u, false},
    {"depthbaisalpha", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1085 = {{
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1086 = {{"maintex","uv_noise_tex_02","uv_noise_tex","noisedissolve_tex","dissolve_tex_01"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,45> WARLORD_NATIVE_PARAMETERS_1086 = {{
    {"selectioncolor", 12u, 0u, true},
    {"maintex_rotator", 5u, 2u, false},
    {"maintex_move_x", 4u, 2u, false},
    {"maintex_move_y", 4u, 3u, false},
    {"dissolvetex_rotator", 2u, 0u, false},
    {"edge_color", 11u, 0u, true},
    {"maintex_panspeed_x", 5u, 0u, false},
    {"maintex_tile_x", 5u, 3u, false},
    {"maintex_tile_y", 6u, 0u, false},
    {"maintex_dynamicpan_x_velue", 4u, 0u, false},
    {"maintex_dynamicpan_y_velue", 4u, 1u, false},
    {"maintex_panspeed_y", 5u, 1u, false},
    {"uv_noisetex_pan_x", 9u, 1u, false},
    {"uv_noisetex_tile_x", 10u, 1u, false},
    {"uv_noisetex_tile_y", 10u, 2u, false},
    {"uv_noisetex_pan_02_x", 8u, 3u, false},
    {"uv_noisetex_tile_02_x", 9u, 3u, false},
    {"uv_noisetex_tile_02_y", 10u, 0u, false},
    {"uv_noisetex_pan_02_y", 9u, 0u, false},
    {"uv_noise_02_strength", 8u, 1u, false},
    {"uv_noisetex_pan_y", 9u, 2u, false},
    {"dynamic_uvnoise_x", 2u, 1u, false},
    {"dynamic_uvnoise_y", 2u, 2u, false},
    {"uv_noise_velue", 8u, 2u, false},
    {"emissive_core_power", 3u, 1u, false},
    {"emissive_core_strength", 3u, 2u, false},
    {"emissive_base", 3u, 0u, false},
    {"disslovetex_01_panspeed_x", 0u, 0u, false},
    {"disslovetex_01_tile_x", 0u, 2u, false},
    {"disslovetex_01_tile_y", 0u, 3u, false},
    {"noisetodisslovetex_01_panspeed_x", 6u, 2u, false},
    {"noisetodisslovetex_01_tile_x", 7u, 0u, false},
    {"noisetodisslovetex_01_tile_y", 7u, 1u, false},
    {"noisetodisslovetex_01_panspeed_y", 6u, 3u, false},
    {"noisedissolvetex_strength", 6u, 1u, false},
    {"disslovetex_01_panspeed_y", 0u, 1u, false},
    {"dissolvetex_move_x", 1u, 1u, false},
    {"dissolvetex_move_y", 1u, 2u, false},
    {"dissolvetex_power", 1u, 3u, false},
    {"dissolve_hardness", 1u, 0u, false},
    {"edge_thin", 2u, 3u, false},
    {"spheremask_strength", 7u, 2u, false},
    {"spheremask_strength_max", 7u, 3u, false},
    {"spheremask_strength_min", 8u, 0u, false},
    {"maintex_alpha_strength", 3u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,19> WARLORD_NATIVE_SWITCHES_1086 = {{
    {"use_alpha_fresnel", false},
    {"use_meshtype", false},
    {"use_dissolvetex_noise", true},
    {"use_dissolvetex_noise_type", true},
    {"achannel_use_uvnoisedynamicpan", true},
    {"use_uv_noise_02", true},
    {"use_uv_noisetex_02", true},
    {"uv_noisetype", true},
    {"maintex_type_polar", false},
    {"maintex_pan_xy", true},
    {"use_dynamic_spheryxy", false},
    {"uv_sphery_y", false},
    {"uv_sphery_x", false},
    {"use_sphere_alpha", true},
    {"use_emissivetex02", false},
    {"use_dissolve", true},
    {"dissloveoutline_multyply_alpha", true},
    {"texcoordpowertype_paramordynamic", true},
    {"use_uvnoise", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1087 = {{"uv_noise_tex","emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,10> WARLORD_NATIVE_PARAMETERS_1087 = {{
    {"selectioncolor", 3u, 0u, true},
    {"emissive_color&intensity", 2u, 0u, true},
    {"uv_noise_panning_x", 0u, 3u, false},
    {"uv_noise_panning_y", 1u, 0u, false},
    {"uv_panning_x", 1u, 2u, false},
    {"uv_panning_y", 1u, 3u, false},
    {"uv_noise_tilling", 1u, 1u, false},
    {"uv_noise_intensity", 0u, 2u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_tex_desturation", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_1087 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_tex", false},
    {"use_emissive_vertcolor", false},
    {"use_nonuvnoise", false},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1088 = {{"diffuse_color","normal_color","native_texture_2","normal_tex","native_texture_4"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1088 = {{
    {"selectioncolor", 2u, 0u, true},
    {"bright", 0u, 0u, false},
    {"desaturation", 0u, 1u, false},
    {"normal_intensity", 0u, 3u, false},
    {"dissolve", 0u, 2u, false},
    {"opacity", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1088 = {{
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1089 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_1089 = {{
    {"selectioncolor", 1u, 0u, true},
    {"01.radius", 0u, 0u, false},
    {"02.hardness", 0u, 1u, false},
    {"03.spherepower", 0u, 2u, false},
    {"04.spherestr", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_1089 = {{
    {"checkisdepthbiasalpha", false},
    {"checkisvertexcolor", true},
    {"01.checkisvertexcolor", true},
    {"02.usedynamicparam", false},
    {"30.usefresnal", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1090 = {{"native_texture_0","native_texture_1"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1090 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1090 = {{
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1091 = {{"06.map_anew","06.map","00.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,12> WARLORD_NATIVE_PARAMETERS_1091 = {{
    {"selectioncolor", 3u, 0u, true},
    {"09.map_a_panning_x", 1u, 2u, false},
    {"06.map_alod", 0u, 1u, false},
    {"07.map_a_uvscale_r", 0u, 2u, false},
    {"08.map_a_uvscale_g", 1u, 0u, false},
    {"09.map_d_panning_x", 1u, 3u, false},
    {"07.map_d_uvscale_r", 0u, 3u, false},
    {"08.map_d_uvscale_g", 1u, 1u, false},
    {"10.map_d_panning_y", 2u, 0u, false},
    {"05.distort_str", 0u, 0u, false},
    {"11.map_a_str", 2u, 1u, false},
    {"12.map_a_power", 2u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,15> WARLORD_NATIVE_SWITCHES_1091 = {{
    {"00.usedistortion", false},
    {"20.use_centrehole", false},
    {"00.usedepthbiasalpha", false},
    {"30.usefresnal", false},
    {"10.cameradistance", false},
    {"01.mapch.r", true},
    {"02.mapch.g", true},
    {"03.mapch.b", true},
    {"00.use_uvdistort", true},
    {"01.usecoordinate_index1", false},
    {"01.use_uvdistort_all", true},
    {"15.use_panbezier", true},
    {"checkismeshemitvc", false},
    {"00.use_rgbinalphamap", false},
    {"30.use_additionalmap", false},
}};

inline constexpr std::array<std::string_view,6> WARLORD_NATIVE_TEXTURES_1092 = {{"noise_tex","native_texture_1","diff_tex","add_tex","edge_tex","opacity_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,18> WARLORD_NATIVE_PARAMETERS_1092 = {{
    {"selectioncolor", 5u, 0u, true},
    {"noise_tile_u", 3u, 2u, false},
    {"noise_tile_v", 3u, 3u, false},
    {"diff_tile_u", 2u, 1u, false},
    {"diff_tile_v", 2u, 2u, false},
    {"addtex_tile_u", 0u, 2u, false},
    {"addtex_tile_v", 0u, 3u, false},
    {"noise_rot", 3u, 1u, false},
    {"diff_dist_str", 1u, 2u, false},
    {"addtex_pow", 0u, 0u, false},
    {"addtex_str", 0u, 1u, false},
    {"diff_des", 1u, 1u, false},
    {"diff_pow", 1u, 3u, false},
    {"diff_str", 2u, 0u, false},
    {"edge_pow", 2u, 3u, false},
    {"edge_str", 3u, 0u, false},
    {"opacity_dist_str", 4u, 0u, false},
    {"depthalpha_bias", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,2> WARLORD_NATIVE_SWITCHES_1092 = {{
    {"use_add_tex", true},
    {"use_depthalpha", true},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1093 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1093 = {{
    {"selectioncolor", 2u, 0u, true},
    {"01.radius", 0u, 0u, false},
    {"02.hardness", 0u, 1u, false},
    {"03.spherepower", 0u, 2u, false},
    {"04.sphere_str", 0u, 3u, false},
    {"depthbiasdalpha_bias", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1093 = {{
    {"checkisdepthbiasalpha", true},
    {"01.checkisvertexcolor", true},
    {"30.usefresnal", false},
    {"02.usedynamicparam", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1094 = {{"01.map_e","01.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,20> WARLORD_NATIVE_PARAMETERS_1094 = {{
    {"selectioncolor", 6u, 0u, true},
    {"19.emissiion_color", 5u, 0u, true},
    {"04.map_e_panning_x", 1u, 3u, false},
    {"02.map_e_uvscale_r", 0u, 3u, false},
    {"03.map_e_uvscale_g", 1u, 1u, false},
    {"05.map_e_panning_y", 2u, 1u, false},
    {"12.desaturation", 3u, 0u, false},
    {"15.emissiion_power", 3u, 1u, false},
    {"16.emissiion_str", 3u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"01.maxangle", 0u, 1u, false},
    {"51.fan_hardness", 4u, 1u, false},
    {"41.ring_hardness", 3u, 3u, false},
    {"42.ring_power", 4u, 0u, false},
    {"04.map_a_panning_x", 1u, 2u, false},
    {"02.map_a_uvscale_r", 0u, 2u, false},
    {"03.map_a_uvscale_g", 1u, 0u, false},
    {"05.map_a_panning_y", 2u, 0u, false},
    {"09.str", 2u, 2u, false},
    {"10.power", 2u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,8> WARLORD_NATIVE_SWITCHES_1094 = {{
    {"06.mapch.r", false},
    {"07.mapch.g", true},
    {"08.mapch.b", true},
    {"00.use_uvdistort", false},
    {"55.spread_side2side", false},
    {"56.spread_invert", false},
    {"00.usedistortion", false},
    {"00.checkisdepthbiasalpha", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1095 = {{"uv_noise_tex","emissive_tex01","alpha_tex","uv_dissolve_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,36> WARLORD_NATIVE_PARAMETERS_1095 = {{
    {"selectioncolor", 10u, 0u, true},
    {"emissive_tex_core_positon_x", 5u, 0u, false},
    {"emissive_tex_core_positon_y", 5u, 1u, false},
    {"meshemitterdynamicparameter", 9u, 0u, true},
    {"alpha_tex_rotator", 2u, 0u, false},
    {"alpha_tex_positon_x", 1u, 2u, false},
    {"alpha_tex_positon_y", 1u, 3u, false},
    {"emissive_tex_core_x", 5u, 2u, false},
    {"emissive_tex_core_y", 6u, 0u, false},
    {"uvnoise_tex_01_texcoord_x", 7u, 2u, false},
    {"uvnoise_tex_01_texcoord_y", 7u, 3u, false},
    {"uv_noise_head_velue", 7u, 1u, false},
    {"uvnoise_tex_dynamicpanspeed_x", 8u, 0u, false},
    {"uvnoise_tex_dynamicpanspeed_y", 8u, 1u, false},
    {"emissive_tex01tile_dynamicpanspeed_x", 3u, 3u, false},
    {"emissive_tex01tile_dynamicpanspeed_y", 4u, 0u, false},
    {"emissive_tex_core_x_02", 5u, 3u, false},
    {"emissive_tex_core_y_02", 6u, 1u, false},
    {"emissive_tex02tile_dynamicpanspeed_x", 4u, 1u, false},
    {"emissive_tex02tile_dynamicpanspeed_y", 4u, 2u, false},
    {"emissive_tex_power", 6u, 2u, false},
    {"emissive_tex_strength", 6u, 3u, false},
    {"emissive_tex_backvelue", 4u, 3u, false},
    {"alpha_tex_texcoord_x", 2u, 2u, false},
    {"alpha_tex_texcoord_y", 2u, 3u, false},
    {"maintex_uv_noise_velue", 7u, 0u, false},
    {"alpha_tex_dynamicpanspeed_x", 1u, 0u, false},
    {"alpha_tex_dynamicpanspeed_y", 1u, 1u, false},
    {"alpha01_tex_power", 0u, 0u, false},
    {"alpha_tex_strength", 2u, 1u, false},
    {"alpha_out_falloff", 0u, 3u, false},
    {"alpha_disslove_tex_coord_x", 0u, 1u, false},
    {"alpha_disslove_tex_coord_y", 0u, 2u, false},
    {"dissolve_tex_dynamicpanspeed_x", 3u, 1u, false},
    {"dissolve_tex_dynamicpanspeed_y", 3u, 2u, false},
    {"dissolve_hardness", 3u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,11> WARLORD_NATIVE_SWITCHES_1095 = {{
    {"use_mesh", true},
    {"use_fresnelalpha", false},
    {"use_dissolve_tex02", false},
    {"dissolve_rampmap", false},
    {"use_alphatex02", false},
    {"use_emissivetex01", true},
    {"use_emissivetex02", false},
    {"emissivetex_mixtype", false},
    {"use_colortex", false},
    {"use_depthbiasalpha", false},
    {"use_uv_noise_head_velue", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1096 = {{"subuv_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_1096 = {{
    {"selectioncolor", 1u, 0u, true},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 1u, false},
    {"emissive_power", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1096 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", false},
    {"use_subuv_tex", true},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"non_alpha_chanel", true},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1097 = {{"maintex","uv_noise_tex","dissolve_tex_01"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,34> WARLORD_NATIVE_PARAMETERS_1097 = {{
    {"selectioncolor", 9u, 0u, true},
    {"uvnoise_move_x", 7u, 2u, false},
    {"uvnoise_move_y", 7u, 3u, false},
    {"maintex_move_x", 3u, 3u, false},
    {"maintex_move_y", 4u, 0u, false},
    {"maintex_rotator", 4u, 3u, false},
    {"dissolvetex_rotator", 1u, 3u, false},
    {"edge_color", 8u, 0u, true},
    {"maintex_panspeed_x", 4u, 1u, false},
    {"maintex_tile_x", 5u, 0u, false},
    {"maintex_tile_y", 5u, 1u, false},
    {"maintex_panspeed_y", 4u, 2u, false},
    {"maintex_dynamicpan_x_velue", 3u, 1u, false},
    {"maintex_dynamicpan_y_velue", 3u, 2u, false},
    {"uv_noisetex_pan_x", 6u, 2u, false},
    {"uv_noisetex_tile_x", 7u, 0u, false},
    {"uv_noisetex_tile_y", 7u, 1u, false},
    {"uv_noisetex_pan_y", 6u, 3u, false},
    {"uv_noise_velue", 6u, 1u, false},
    {"emissive_core_power", 2u, 2u, false},
    {"emissive_core_strength", 2u, 3u, false},
    {"emissive_base", 2u, 1u, false},
    {"disslovetex_01_panspeed_x", 0u, 0u, false},
    {"disslovetex_01_tile_x", 0u, 2u, false},
    {"disslovetex_01_tile_y", 0u, 3u, false},
    {"disslovetex_01_panspeed_y", 0u, 1u, false},
    {"dissolvetex_move_x", 1u, 1u, false},
    {"dissolvetex_move_y", 1u, 2u, false},
    {"dissolve_hardness", 1u, 0u, false},
    {"edge_thin", 2u, 0u, false},
    {"spheremask_strength", 5u, 2u, false},
    {"spheremask_strength_max", 5u, 3u, false},
    {"spheremask_strength_min", 6u, 0u, false},
    {"maintex_alpha_strength", 3u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,19> WARLORD_NATIVE_SWITCHES_1097 = {{
    {"use_alpha_fresnel", false},
    {"use_meshtype", false},
    {"use_dissolvetex_noise", false},
    {"use_dissolvetex_noise_type", true},
    {"achannel_use_uvnoisedynamicpan", false},
    {"use_uv_noise_02", false},
    {"use_uv_noisetex_02", true},
    {"uv_noisetype", true},
    {"maintex_type_polar", true},
    {"maintex_pan_xy", false},
    {"use_dynamic_spheryxy", false},
    {"uv_sphery_y", false},
    {"uv_sphery_x", false},
    {"use_sphere_alpha", true},
    {"use_emissivetex02", false},
    {"use_dissolve", true},
    {"dissloveoutline_multyply_alpha", false},
    {"use_dissolvetex_02", false},
    {"use_dissolvetex_02_mixtype", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1098 = {{"dissolve_noise_tex","uv_noise_01_tex","emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,16> WARLORD_NATIVE_PARAMETERS_1098 = {{
    {"selectioncolor", 5u, 0u, true},
    {"dissolve_color&intensity", 4u, 0u, true},
    {"dissolve_noise_intensity", 0u, 0u, false},
    {"dissolve_noise_power", 0u, 1u, false},
    {"dissolve_power", 0u, 2u, false},
    {"uv_noise_01_panning_x", 2u, 1u, false},
    {"uv_noise_01_tiling_x", 2u, 3u, false},
    {"uv_noise_01_tiling_y", 3u, 0u, false},
    {"uv_noise_01_panning_y", 2u, 2u, false},
    {"uv_noise_01_intensity", 2u, 0u, false},
    {"uv_scale", 3u, 1u, false},
    {"dynamic_parameter_explanation", 0u, 3u, false},
    {"emissive_desaturation", 1u, 2u, false},
    {"edge_intensity", 1u, 0u, false},
    {"edge_power", 1u, 1u, false},
    {"emissive_power", 1u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,14> WARLORD_NATIVE_SWITCHES_1098 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_lamp_alpha", false},
    {"use_dissolve", true},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"non_alpha_chanel", false},
    {"use_uv_noise_tex", true},
    {"use_uv_noise_tex_02", false},
    {"use_fresnel_alpha", false},
    {"use_edge_glow", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1099 = {{"31.map_e","06.map","04.map_anew","00.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,22> WARLORD_NATIVE_PARAMETERS_1099 = {{
    {"selectioncolor", 7u, 0u, true},
    {"meshemitterdynamicparameter", 6u, 0u, true},
    {"37.mape_color", 5u, 0u, true},
    {"05.map_alod", 0u, 2u, false},
    {"39.map_a_panning_x", 3u, 3u, false},
    {"41.map_a_uvscale_r", 4u, 1u, false},
    {"42.map_a_uvscale_g", 4u, 2u, false},
    {"40.map_a_panning_y", 4u, 0u, false},
    {"09.map_d_panning_x", 2u, 0u, false},
    {"07.map_d_uvscale_r", 1u, 0u, false},
    {"08.map_d_uvscale_g", 1u, 2u, false},
    {"10.map_d_panning_y", 2u, 2u, false},
    {"05.distort_str", 0u, 1u, false},
    {"33.desaturation", 3u, 1u, false},
    {"35.mape_power", 3u, 2u, false},
    {"09.map_a_panning_x", 1u, 3u, false},
    {"07.map_a_uvscale_r", 0u, 3u, false},
    {"08.map_a_uvscale_g", 1u, 1u, false},
    {"10.map_a_panning_y", 2u, 1u, false},
    {"11.map_a_str", 2u, 3u, false},
    {"12.map_a_power", 3u, 0u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1099 = {{
    {"00.usedistortion", false},
    {"20.use_centrehole", false},
    {"00.usedepthbiasalpha", true},
    {"30.usefresnal", false},
    {"10.cameradistance", false},
    {"01.mapch.r", true},
    {"02.mapch.g", true},
    {"03.mapch.b", true},
    {"00.use_uvdistort", true},
    {"01.use_uvdistort_all", false},
    {"checkismeshemitvc", true},
    {"00.use_rgbinalphamap", false},
    {"06.use((r+g+b)*0.33)", false},
    {"15.use_panbezier", false},
    {"01.usecoordinate_index1", true},
    {"30.use_additionalmap", true},
    {"useupno_alpha", false},
    {"40.use_fanshape", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1100 = {{"native_texture_0","01.map_uv","01.map_a","11.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1100 = {{
    {"selectioncolor", 2u, 0u, true},
    {"09.str", 0u, 0u, false},
    {"10.power", 0u, 1u, false},
    {"12.map_b_uvscale", 0u, 2u, false},
    {"18.str", 0u, 3u, false},
    {"19.power", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,8> WARLORD_NATIVE_SWITCHES_1100 = {{
    {"00.checkisdepthbiasalpha", false},
    {"01.usemesh", false},
    {"06.mapch.r", false},
    {"07.mapch.g", true},
    {"08.mapch.b", true},
    {"00.useemissionmap", false},
    {"30.usefresnal", false},
    {"31.fresnal.invert", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1101 = {{"02.map_e","22.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,18> WARLORD_NATIVE_PARAMETERS_1101 = {{
    {"selectioncolor", 5u, 0u, true},
    {"93.emissiion_color", 4u, 0u, true},
    {"22.map_a_uvscale_r", 2u, 0u, false},
    {"23.map_a_uvscale_g", 2u, 1u, false},
    {"time", 3u, 3u, false},
    {"05.map_e_panning_x", 1u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 2u, false},
    {"04.map_e_uvscale_g", 0u, 3u, false},
    {"11.uv.curvature", 1u, 3u, false},
    {"06.map_e_panning_y", 1u, 1u, false},
    {"91.desaturation", 2u, 3u, false},
    {"92.emissiion_power", 3u, 0u, false},
    {"02.radius", 0u, 0u, false},
    {"03.hardness", 0u, 1u, false},
    {"09.power", 1u, 2u, false},
    {"24.map_a_panning_x", 2u, 2u, false},
    {"95.str", 3u, 1u, false},
    {"96.power", 3u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,27> WARLORD_NATIVE_SWITCHES_1101 = {{
    {"00.usespecullar", false},
    {"11.mape+mapf", false},
    {"10.use_mapb", false},
    {"01.use_emissionmap", true},
    {"01.usemesh", false},
    {"05.useinvert", false},
    {"00.usedistortion", false},
    {"01.usesphmask", true},
    {"25.xinvert", false},
    {"20.usetwirl", false},
    {"01.usecoordinate_index1", false},
    {"94.sphmask+(texmap*phasemap)", false},
    {"01.effectonalphatex", false},
    {"11.useclamptexmap", false},
    {"91.mapch.r", true},
    {"92.mapch.g", true},
    {"00.checkisdepthbiasalpha", false},
    {"21.usephasetexmap", true},
    {"30.usefresnal", false},
    {"31.fresnal.invert", false},
    {"00.use_uvdistort", false},
    {"29.usedynamic_ypanning", true},
    {"23.usedynamicstr", false},
    {"93.mapch.b", true},
    {"10.fresnal", true},
    {"99.alphadissolve", false},
    {"94.mapch.a", true},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1102 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,2> WARLORD_NATIVE_PARAMETERS_1102 = {{
    {"selectioncolor", 1u, 0u, true},
    {"hardness", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,1> WARLORD_NATIVE_SWITCHES_1102 = {{
    {"use_depthbias", false},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1103 = {{"uv_noise_texture","alpha_texture1_mask","alpha_texture2","emissive_tex_01","emissive_tex_02"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,31> WARLORD_NATIVE_PARAMETERS_1103 = {{
    {"selectioncolor", 8u, 0u, true},
    {"mainalpha_move_u", 5u, 0u, false},
    {"mainalpha_move_v", 5u, 1u, false},
    {"mainalpha_rotator", 5u, 2u, false},
    {"emissive_tex_01_move_u", 2u, 0u, false},
    {"emissive_tex_01_move_v", 2u, 1u, false},
    {"emissive_tex_02_move_u", 3u, 2u, false},
    {"emissive_tex_02_move_v", 3u, 3u, false},
    {"mainalpha_tile_u", 5u, 3u, false},
    {"mainalpha_tile_v", 6u, 0u, false},
    {"uv_noise_panspeed_x", 6u, 1u, false},
    {"uv_noise_tile_u", 6u, 3u, false},
    {"uv_noise_tile_v", 7u, 0u, false},
    {"uv_noise_panspeed_y", 6u, 2u, false},
    {"uv_noise_velue", 7u, 1u, false},
    {"emissive_background_power", 0u, 1u, false},
    {"emissive_background_strength", 0u, 2u, false},
    {"emissive_tex_01_panspeed_x", 2u, 2u, false},
    {"emissive_tex_01_tile_u", 3u, 0u, false},
    {"emissive_tex_01_tile_v", 3u, 1u, false},
    {"emissive_noise_velue", 1u, 1u, false},
    {"emissive_tex_01_panspeed_y", 2u, 3u, false},
    {"emissive_tex_02_panspeed_x", 4u, 0u, false},
    {"emissive_tex_02_tile_u", 4u, 2u, false},
    {"emissive_tex_02_tile_v", 4u, 3u, false},
    {"emissive_tex_02_panspeed_y", 4u, 1u, false},
    {"emissive_power", 1u, 2u, false},
    {"emissive_core_power", 0u, 3u, false},
    {"emissive_core_strength", 1u, 0u, false},
    {"emissive_strength", 1u, 3u, false},
    {"alpha_tex_power", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,6> WARLORD_NATIVE_SWITCHES_1103 = {{
    {"use_distortion", false},
    {"use_emissive_tex_02", true},
    {"emissive_tex_(01+02)/2", false},
    {"use_alphatex_type_pola", false},
    {"use_uv_noisetexture", true},
    {"use_emissive_tex02_to_emissive01_uvnoise", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1104 = {{"06.map","02.map_e","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,22> WARLORD_NATIVE_PARAMETERS_1104 = {{
    {"selectioncolor", 7u, 0u, true},
    {"93.emissiion_color", 5u, 0u, true},
    {"meshemitterdynamicparameter", 6u, 0u, true},
    {"21.uvscale.x", 2u, 2u, false},
    {"22.uvscale.y", 2u, 3u, false},
    {"time", 4u, 2u, false},
    {"05.map_e_panning_x", 1u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 1u, false},
    {"04.map_e_uvscale_g", 0u, 2u, false},
    {"09.map_d_panning_x", 2u, 0u, false},
    {"07.map_d_uvscale_r", 1u, 2u, false},
    {"08.map_d_uvscale_g", 1u, 3u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 0u, 3u, false},
    {"06.map_e_panning_y", 1u, 1u, false},
    {"91.desaturation", 4u, 0u, false},
    {"92.emissiion_power", 4u, 1u, false},
    {"29.cmap.direct", 3u, 0u, false},
    {"30.cmap.time(rotrate)", 3u, 1u, false},
    {"36.str", 3u, 2u, false},
    {"37.power", 3u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1104 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", false},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1105 = {{"06.map","02.map_e","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,22> WARLORD_NATIVE_PARAMETERS_1105 = {{
    {"selectioncolor", 7u, 0u, true},
    {"93.emissiion_color", 5u, 0u, true},
    {"meshemitterdynamicparameter", 6u, 0u, true},
    {"21.uvscale.x", 2u, 2u, false},
    {"22.uvscale.y", 2u, 3u, false},
    {"time", 4u, 2u, false},
    {"05.map_e_panning_x", 1u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 1u, false},
    {"04.map_e_uvscale_g", 0u, 2u, false},
    {"09.map_d_panning_x", 2u, 0u, false},
    {"07.map_d_uvscale_r", 1u, 2u, false},
    {"08.map_d_uvscale_g", 1u, 3u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 0u, 3u, false},
    {"06.map_e_panning_y", 1u, 1u, false},
    {"91.desaturation", 4u, 0u, false},
    {"92.emissiion_power", 4u, 1u, false},
    {"29.cmap.direct", 3u, 0u, false},
    {"30.cmap.time(rotrate)", 3u, 1u, false},
    {"36.str", 3u, 2u, false},
    {"37.power", 3u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1105 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", false},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1106 = {{"distorta_tex","distortb_tex","emissive_tex","maska_tex","maskb_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,18> WARLORD_NATIVE_PARAMETERS_1106 = {{
    {"selectioncolor", 5u, 0u, true},
    {"emissive_tileu", 3u, 1u, false},
    {"emissive_tilev", 3u, 2u, false},
    {"distorta_tileu", 0u, 3u, false},
    {"distorta_tilev", 1u, 0u, false},
    {"distorta_panneru", 0u, 1u, false},
    {"distorta_pannerv", 0u, 2u, false},
    {"distortb_tileu", 1u, 3u, false},
    {"distortb_tilev", 2u, 0u, false},
    {"distortb_panneru", 1u, 1u, false},
    {"distortb_pannerv", 1u, 2u, false},
    {"emissive_panneru", 2u, 1u, false},
    {"emissive_pannerv", 2u, 2u, false},
    {"distort_str", 0u, 0u, false},
    {"emissive_power", 2u, 3u, false},
    {"emissive_str", 3u, 0u, false},
    {"mask_power", 3u, 3u, false},
    {"mask_str", 4u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1106 = {{
    {"use_depthbiasalpha", false},
    {"use_mesh", false},
    {"use_dynamic_lerp", true},
    {"use_fanshape_alpha", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1107 = {{"01.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,11> WARLORD_NATIVE_PARAMETERS_1107 = {{
    {"selectioncolor", 3u, 0u, true},
    {"09.lodbias", 1u, 2u, false},
    {"04.map_a_panning_x", 0u, 2u, false},
    {"02.map_a_uvscale_r", 0u, 0u, false},
    {"03.map_a_uvscale_g", 0u, 1u, false},
    {"05.map_a_panning_y", 1u, 0u, false},
    {"21.str", 2u, 0u, false},
    {"22.power", 2u, 1u, false},
    {"10.radius", 1u, 3u, false},
    {"04.ringthickness", 0u, 3u, false},
    {"07.power", 1u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,11> WARLORD_NATIVE_SWITCHES_1107 = {{
    {"00.usedistortion", false},
    {"00.use_uvdistort", false},
    {"01.usemesh", false},
    {"01.usesoftshape", true},
    {"02.useouthardness", false},
    {"00.alphamap", true},
    {"00.checkisdepthbiasalpha", false},
    {"20.usehemisphere", false},
    {"00.useemissioncolor", false},
    {"10.additionalmap", false},
    {"11.mapa*mapb", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1108 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1108 = {{
    {"selectioncolor", 2u, 0u, true},
    {"centerglow_power", 0u, 0u, false},
    {"centerglow_str", 0u, 1u, false},
    {"glow_power", 0u, 2u, false},
    {"str", 1u, 0u, false},
    {"power", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1108 = {{
    {"use_distortion", false},
    {"use_centerglow", true},
    {"use_centermask", false},
    {"use_depth_alpha", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1109 = {{"lensflaretexture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_1109 = {{
    {"selectioncolor", 1u, 0u, true},
    {"select texture(0 or 0.5)", 0u, 2u, false},
    {"desaturation", 0u, 1u, false},
    {"depthbaisalpha", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1109 = {{
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1110 = {{"native_texture_0"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1110 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1110 = {{
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1111 = {{"12.map_f","06.map","02.map_e","12.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,28> WARLORD_NATIVE_PARAMETERS_1111 = {{
    {"selectioncolor", 8u, 0u, true},
    {"93.emissiion_color", 7u, 0u, true},
    {"19.cmap.direct", 5u, 0u, false},
    {"time", 6u, 1u, false},
    {"15.map_e_panning_x", 4u, 2u, false},
    {"13.map_e_uvscale_r", 3u, 3u, false},
    {"14.map_e_uvscale_g", 4u, 0u, false},
    {"11.uv.curvature", 3u, 2u, false},
    {"09.map_d_panning_x", 2u, 3u, false},
    {"07.map_d_uvscale_r", 2u, 1u, false},
    {"08.map_d_uvscale_g", 2u, 2u, false},
    {"10.map_d_panning_y", 3u, 1u, false},
    {"05.distort_str", 1u, 0u, false},
    {"16.map_e_panning_y", 4u, 3u, false},
    {"05.map_e_panning_x", 1u, 1u, false},
    {"03.map_e_uvscale_r", 0u, 2u, false},
    {"04.map_e_uvscale_g", 0u, 3u, false},
    {"06.map_e_panning_y", 1u, 2u, false},
    {"91.desaturation", 5u, 1u, false},
    {"92.emissiion_power", 5u, 2u, false},
    {"06.radius", 1u, 3u, false},
    {"07.hardness", 2u, 0u, false},
    {"02.radius", 0u, 0u, false},
    {"03.hardness", 0u, 1u, false},
    {"09.power", 3u, 0u, false},
    {"14.uvscale.x", 4u, 1u, false},
    {"95.str", 5u, 3u, false},
    {"96.power", 6u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,27> WARLORD_NATIVE_SWITCHES_1111 = {{
    {"00.usespecullar", false},
    {"11.mape+mapf", false},
    {"10.use_mapb", true},
    {"01.use_emissionmap", true},
    {"01.usemesh", false},
    {"05.useinvert", true},
    {"00.usedistortion", false},
    {"01.usesphmask", true},
    {"25.xinvert", false},
    {"20.usetwirl", false},
    {"01.usecoordinate_index1", false},
    {"94.sphmask+(texmap*phasemap)", false},
    {"01.effectonalphatex", false},
    {"11.useclamptexmap", true},
    {"91.mapch.r", true},
    {"92.mapch.g", true},
    {"00.checkisdepthbiasalpha", false},
    {"21.usephasetexmap", false},
    {"30.usefresnal", false},
    {"31.fresnal.invert", false},
    {"00.use_uvdistort", true},
    {"29.usedynamic_ypanning", false},
    {"23.usedynamicstr", false},
    {"93.mapch.b", true},
    {"10.fresnal", true},
    {"99.alphadissolve", false},
    {"94.mapch.a", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1112 = {{"tex_uv_flow","tex_alpha_01","tex_alpha_02"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,11> WARLORD_NATIVE_PARAMETERS_1112 = {{
    {"selectioncolor", 3u, 0u, true},
    {"tex_alpha_01_rotator", 1u, 1u, false},
    {"tex_alpha_01_r_tile", 1u, 0u, false},
    {"tex_alpha_01_g_tile", 0u, 3u, false},
    {"tex_flow_r_tile", 2u, 1u, false},
    {"tex_flow_g_tile", 2u, 0u, false},
    {"emissive_velue_power", 0u, 2u, false},
    {"emissive_velue_multiply", 0u, 1u, false},
    {"emissive_velue_background", 0u, 0u, false},
    {"tex_alpha_02_r_tile", 1u, 3u, false},
    {"tex_alpha_02_g_tile", 1u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,2> WARLORD_NATIVE_SWITCHES_1112 = {{
    {"use_mesh", false},
    {"alpha_01_rotator_use", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1114 = {{"31.map_e","06.map","04.map_anew","00.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,21> WARLORD_NATIVE_PARAMETERS_1114 = {{
    {"selectioncolor", 6u, 0u, true},
    {"37.mape_color", 5u, 0u, true},
    {"05.map_alod", 0u, 1u, false},
    {"39.map_a_panning_x", 3u, 3u, false},
    {"41.map_a_uvscale_r", 4u, 1u, false},
    {"42.map_a_uvscale_g", 4u, 2u, false},
    {"40.map_a_panning_y", 4u, 0u, false},
    {"09.map_d_panning_x", 1u, 3u, false},
    {"07.map_d_uvscale_r", 0u, 3u, false},
    {"08.map_d_uvscale_g", 1u, 1u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 0u, 0u, false},
    {"33.desaturation", 3u, 1u, false},
    {"35.mape_power", 3u, 2u, false},
    {"09.map_a_panning_x", 1u, 2u, false},
    {"07.map_a_uvscale_r", 0u, 2u, false},
    {"08.map_a_uvscale_g", 1u, 0u, false},
    {"10.map_a_panning_y", 2u, 0u, false},
    {"11.map_a_str", 2u, 2u, false},
    {"12.map_a_power", 2u, 3u, false},
    {"21.centerhole_power", 3u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1114 = {{
    {"00.usedistortion", false},
    {"20.use_centrehole", true},
    {"00.usedepthbiasalpha", false},
    {"30.usefresnal", false},
    {"10.cameradistance", false},
    {"01.mapch.r", true},
    {"02.mapch.g", true},
    {"03.mapch.b", true},
    {"00.use_uvdistort", true},
    {"01.use_uvdistort_all", false},
    {"checkismeshemitvc", false},
    {"00.use_rgbinalphamap", false},
    {"06.use((r+g+b)*0.33)", false},
    {"15.use_panbezier", false},
    {"01.usecoordinate_index1", false},
    {"30.use_additionalmap", true},
    {"useupno_alpha", false},
    {"40.use_fanshape", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1115 = {{"02.map_e","06.map","22.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,25> WARLORD_NATIVE_PARAMETERS_1115 = {{
    {"selectioncolor", 7u, 0u, true},
    {"93.emissiion_color", 6u, 0u, true},
    {"22.map_a_uvscale_r", 4u, 0u, false},
    {"23.map_a_uvscale_g", 4u, 1u, false},
    {"05.map_e_panning_x", 1u, 2u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 0u, false},
    {"11.uv.curvature", 3u, 3u, false},
    {"09.map_d_panning_x", 3u, 0u, false},
    {"07.map_d_uvscale_r", 2u, 2u, false},
    {"08.map_d_uvscale_g", 2u, 3u, false},
    {"10.map_d_panning_y", 3u, 2u, false},
    {"05.distort_str", 1u, 1u, false},
    {"06.map_e_panning_y", 1u, 3u, false},
    {"91.desaturation", 4u, 3u, false},
    {"92.emissiion_power", 5u, 0u, false},
    {"06.radius", 2u, 0u, false},
    {"07.hardness", 2u, 1u, false},
    {"02.radius", 0u, 1u, false},
    {"03.hardness", 0u, 2u, false},
    {"09.power", 3u, 1u, false},
    {"24.map_a_panning_x", 4u, 2u, false},
    {"95.str", 5u, 1u, false},
    {"96.power", 5u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,27> WARLORD_NATIVE_SWITCHES_1115 = {{
    {"00.usedistortion", false},
    {"30.usefresnal", false},
    {"31.fresnal.invert", false},
    {"00.checkisdepthbiasalpha", true},
    {"94.sphmask+(texmap*phasemap)", false},
    {"91.mapch.r", false},
    {"92.mapch.g", false},
    {"21.usephasetexmap", true},
    {"29.usedynamic_ypanning", true},
    {"01.usemesh", false},
    {"00.use_uvdistort", true},
    {"20.usetwirl", false},
    {"01.usecoordinate_index1", false},
    {"25.xinvert", false},
    {"23.usedynamicstr", false},
    {"11.useclamptexmap", false},
    {"01.effectonalphatex", false},
    {"01.usesphmask", true},
    {"05.useinvert", true},
    {"01.use_emissionmap", true},
    {"10.use_mapb", false},
    {"11.mape+mapf", false},
    {"00.usespecullar", false},
    {"93.mapch.b", true},
    {"10.fresnal", true},
    {"99.alphadissolve", false},
    {"94.mapch.a", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1116 = {{"02.map_e","01.map_a","11.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,22> WARLORD_NATIVE_PARAMETERS_1116 = {{
    {"selectioncolor", 7u, 0u, true},
    {"93.emissiion_color", 5u, 0u, true},
    {"meshemitterdynamicparameter", 6u, 0u, true},
    {"time", 4u, 2u, false},
    {"05.map_e_panning_x", 2u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 1u, false},
    {"06.map_e_panning_y", 2u, 1u, false},
    {"91.desaturation", 4u, 0u, false},
    {"92.emissiion_power", 4u, 1u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"05.map_a_panning_y", 1u, 3u, false},
    {"14.map_b_panning_x", 3u, 0u, false},
    {"12.map_b_uvscale_r", 2u, 2u, false},
    {"13.map_b_uvscale_g", 2u, 3u, false},
    {"15.map_b_panning_y", 3u, 1u, false},
    {"36.str", 3u, 2u, false},
    {"37.power", 3u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"05.distortion_str", 1u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1116 = {{
    {"00.usedistortion", true},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", false},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", false},
    {"10.use_mapb", true},
    {"00.use_mapa", true},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1117 = {{"01.map_a","11.map_b","21.map_c","01.specmap"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,27> WARLORD_NATIVE_PARAMETERS_1117 = {{
    {"selectioncolor", 9u, 0u, true},
    {"05.specmap_uvscale.x", 1u, 2u, false},
    {"06.specmap_uvscale.y", 1u, 3u, false},
    {"93.emissiion_color", 7u, 0u, true},
    {"meshemitterdynamicparameter", 8u, 0u, true},
    {"21.uvscale.x", 3u, 2u, false},
    {"22.uvscale.y", 3u, 3u, false},
    {"09.specmap_color", 6u, 0u, true},
    {"time", 5u, 2u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 3u, false},
    {"05.map_a_panning_y", 1u, 1u, false},
    {"14.map_b_panning_x", 3u, 0u, false},
    {"12.map_b_uvscale_r", 2u, 2u, false},
    {"13.map_b_uvscale_g", 2u, 3u, false},
    {"15.map_b_panning_y", 3u, 1u, false},
    {"29.cmap.direct", 4u, 0u, false},
    {"30.cmap.time(rotrate)", 4u, 1u, false},
    {"02.specmap_str", 0u, 2u, false},
    {"07.desaturation", 2u, 0u, false},
    {"08.specmap_power", 2u, 1u, false},
    {"36.str", 5u, 0u, false},
    {"37.power", 5u, 1u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"32.fresnal_power", 4u, 2u, false},
    {"33.fresnal_str", 4u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1117 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", false},
    {"10.use_mapb", true},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", false},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", true},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1118 = {{"06.map","02.map_e","01.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,21> WARLORD_NATIVE_PARAMETERS_1118 = {{
    {"selectioncolor", 6u, 0u, true},
    {"93.emissiion_color", 5u, 0u, true},
    {"time", 4u, 2u, false},
    {"05.map_e_panning_x", 2u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 1u, false},
    {"09.map_d_panning_x", 3u, 0u, false},
    {"07.map_d_uvscale_r", 2u, 2u, false},
    {"08.map_d_uvscale_g", 2u, 3u, false},
    {"10.map_d_panning_y", 3u, 1u, false},
    {"05.distort_str", 1u, 2u, false},
    {"06.map_e_panning_y", 2u, 1u, false},
    {"91.desaturation", 4u, 0u, false},
    {"92.emissiion_power", 4u, 1u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"05.map_a_panning_y", 1u, 3u, false},
    {"36.str", 3u, 2u, false},
    {"37.power", 3u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1118 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"31.mapch.r", false},
    {"32.mapch.g", false},
    {"20.use_clampmap", false},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", false},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1119 = {{"01.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,12> WARLORD_NATIVE_PARAMETERS_1119 = {{
    {"selectioncolor", 3u, 0u, true},
    {"01.rotateuv", 0u, 0u, false},
    {"04.map_a_pantimescale", 1u, 1u, false},
    {"02.map_a_uv.x", 0u, 2u, false},
    {"03.map_a_uv.y", 1u, 0u, false},
    {"10.edgeemission_power", 1u, 2u, false},
    {"11.edgeemission_str", 1u, 3u, false},
    {"01.wavecount", 0u, 1u, false},
    {"02.wavestr", 0u, 3u, false},
    {"11.width", 2u, 0u, false},
    {"51.str", 2u, 1u, false},
    {"52.power", 2u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_1119 = {{
    {"30.usefresnel", false},
    {"50.useendfalloff", true},
    {"01.usemesh", false},
    {"00.use_uvdistort", false},
    {"00.use_wavetimebezier_in_dp", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1120 = {{"15.map_type"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,14> WARLORD_NATIVE_PARAMETERS_1120 = {{
    {"selectioncolor", 5u, 0u, true},
    {"01.color.a", 3u, 0u, true},
    {"02.color.b", 4u, 0u, true},
    {"07.map.rotation", 2u, 1u, false},
    {"08.map.rot.variton", 2u, 2u, false},
    {"06.noise_uvscale.x1", 2u, 0u, false},
    {"03.color_lerpstr", 0u, 3u, false},
    {"04.color_lerppow", 1u, 1u, false},
    {"03.low threshold", 1u, 0u, false},
    {"04.gain", 1u, 2u, false},
    {"05.bias", 1u, 3u, false},
    {"01.density", 0u, 0u, false},
    {"02.density_power", 0u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,9> WARLORD_NATIVE_SWITCHES_1120 = {{
    {"10.usedistancealpha", false},
    {"00.use_depthbiasalpha", true},
    {"16.invert", false},
    {"20.usetimebezire", true},
    {"30.usephase by worldposition", false},
    {"01.self shadow", false},
    {"01.use explode", false},
    {"10.use detailmap", false},
    {"00.usevaricolor", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1121 = {{"01.map_a","11.map_b","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,18> WARLORD_NATIVE_PARAMETERS_1121 = {{
    {"selectioncolor", 5u, 0u, true},
    {"meshemitterdynamicparameter", 4u, 0u, true},
    {"21.uvscale.x", 2u, 1u, false},
    {"22.uvscale.y", 2u, 2u, false},
    {"time", 3u, 3u, false},
    {"04.map_a_panning_x", 0u, 3u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"05.map_a_panning_y", 1u, 0u, false},
    {"14.map_b_panning_x", 1u, 3u, false},
    {"12.map_b_uvscale_r", 1u, 1u, false},
    {"13.map_b_uvscale_g", 1u, 2u, false},
    {"15.map_b_panning_y", 2u, 0u, false},
    {"29.cmap.direct", 2u, 3u, false},
    {"30.cmap.time(rotrate)", 3u, 0u, false},
    {"36.str", 3u, 1u, false},
    {"37.power", 3u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1121 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", false},
    {"32.mapch.g", false},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", false},
    {"10.use_mapb", true},
    {"00.use_mapa", true},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", false},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1124 = {{"01.map.n","01.emismap","01.map.d","01.specmap"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,24> WARLORD_NATIVE_PARAMETERS_1124 = {{
    {"selectioncolor", 9u, 0u, true},
    {"11.normalmap.str", 3u, 3u, false},
    {"03.emap_uv.x.scale", 0u, 3u, false},
    {"04.emap_uv.y.scale", 1u, 2u, false},
    {"19.emissiion_color", 6u, 0u, true},
    {"meshemitterdynamicparameter", 8u, 0u, true},
    {"05.specmap_uvscale.x", 2u, 1u, false},
    {"06.specmap_uvscale.y", 2u, 3u, false},
    {"02.uvscale.x", 0u, 2u, false},
    {"03.uvscale.y", 1u, 1u, false},
    {"93.emissiion_color", 7u, 0u, true},
    {"09.specmap_color", 5u, 0u, true},
    {"05.n.panning.x", 1u, 3u, false},
    {"02.n.uvscale.x", 0u, 0u, false},
    {"03.n.uvscale.y", 1u, 0u, false},
    {"06.n.panning.y", 2u, 2u, false},
    {"15.emissiion_power", 4u, 0u, false},
    {"91.desaturation", 4u, 1u, false},
    {"92.emissiion_power", 4u, 2u, false},
    {"02.specmap_str", 0u, 1u, false},
    {"07.desaturation", 3u, 1u, false},
    {"08.specmap_power", 3u, 2u, false},
    {"05.power", 2u, 0u, false},
    {"06.str", 3u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,22> WARLORD_NATIVE_SWITCHES_1124 = {{
    {"00.usenormalmap", true},
    {"00.usealphamap", false},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"33.mapch.b", true},
    {"19.mapa+mapclamp", false},
    {"20.use_clampmap", false},
    {"25.use_yclamp", false},
    {"01.usemesh", true},
    {"50.dissolvetype-mosaic", true},
    {"use_cascade_vertexcolor", true},
    {"00.usefakelight", true},
    {"01.invert", false},
    {"00.usediffuse", true},
    {"11.usemapd.uvscale.dyna", false},
    {"00.usespecullar", true},
    {"30.dependonfakeli", true},
    {"20.usefresspec", false},
    {"00.useemission", true},
    {"20.usefresnal", true},
    {"00.use_uvdistort", false},
    {"21.invert", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1125 = {{"uv_noise_01_tex","emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,11> WARLORD_NATIVE_PARAMETERS_1125 = {{
    {"selectioncolor", 3u, 0u, true},
    {"uv_noise_01_panning_x", 1u, 1u, false},
    {"uv_noise_01_tiling_x", 1u, 3u, false},
    {"uv_noise_01_tiling_y", 2u, 0u, false},
    {"uv_noise_01_panning_y", 1u, 2u, false},
    {"uv_noise_01_intensity", 1u, 0u, false},
    {"uv_scale", 2u, 1u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1125 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", true},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"non_alpha_chanel", false},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1126 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1126 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_1126 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1127 = {{"tex_uv_flow","tex_alpha_01","tex_alpha_02"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,11> WARLORD_NATIVE_PARAMETERS_1127 = {{
    {"selectioncolor", 3u, 0u, true},
    {"tex_alpha_01_rotator", 1u, 1u, false},
    {"tex_alpha_01_r_tile", 1u, 0u, false},
    {"tex_alpha_01_g_tile", 0u, 3u, false},
    {"tex_flow_r_tile", 2u, 1u, false},
    {"tex_flow_g_tile", 2u, 0u, false},
    {"emissive_velue_power", 0u, 2u, false},
    {"emissive_velue_multiply", 0u, 1u, false},
    {"emissive_velue_background", 0u, 0u, false},
    {"tex_alpha_02_r_tile", 1u, 3u, false},
    {"tex_alpha_02_g_tile", 1u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,2> WARLORD_NATIVE_SWITCHES_1127 = {{
    {"use_mesh", false},
    {"alpha_01_rotator_use", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1128 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1128 = {{
    {"selectioncolor", 2u, 0u, true},
    {"uv_scale", 1u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_1128 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"use_distortion_floor", false},
    {"non_alpha_chanel", false},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex_02", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1129 = {{"uv_noise_01_tex","emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,12> WARLORD_NATIVE_PARAMETERS_1129 = {{
    {"selectioncolor", 3u, 0u, true},
    {"uv_noise_01_panning_x", 1u, 2u, false},
    {"uv_noise_01_tiling_x", 2u, 0u, false},
    {"uv_noise_01_tiling_y", 2u, 1u, false},
    {"uv_noise_01_panning_y", 1u, 3u, false},
    {"uv_noise_01_intensity", 1u, 1u, false},
    {"uv_scale", 2u, 2u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 3u, false},
    {"edge_intensity", 0u, 1u, false},
    {"edge_power", 0u, 2u, false},
    {"emissive_power", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_1129 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", true},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", true},
    {"use_distortion_floor", true},
    {"non_alpha_chanel", true},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex_02", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1130 = {{"noise_tex","diff_tex","native_texture_2","opacity_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,26> WARLORD_NATIVE_PARAMETERS_1130 = {{
    {"selectioncolor", 9u, 0u, true},
    {"diff_high_color", 6u, 0u, true},
    {"diff_low_color", 7u, 0u, true},
    {"diff_u_tile", 1u, 1u, false},
    {"diff_v_tile", 1u, 2u, false},
    {"noise_u_tile", 2u, 2u, false},
    {"noise_v_tile", 3u, 0u, false},
    {"noise_u_pan", 2u, 1u, false},
    {"noise_v_pan", 2u, 3u, false},
    {"meshemitterdynamicparameter", 8u, 0u, true},
    {"wave_pan_speed", 4u, 3u, false},
    {"diff_rotation", 0u, 2u, false},
    {"opacity_u_tile", 4u, 0u, false},
    {"opacity_v_tile", 4u, 1u, false},
    {"opacity_rotation", 3u, 1u, false},
    {"noise_str", 2u, 0u, false},
    {"diff_u_center", 1u, 0u, false},
    {"wave_tile", 5u, 1u, false},
    {"wave_str", 5u, 0u, false},
    {"wave_noise_str", 4u, 2u, false},
    {"diff_desturation", 0u, 0u, false},
    {"diff_pow", 0u, 1u, false},
    {"diff_str", 0u, 3u, false},
    {"opacity_u_center", 3u, 3u, false},
    {"opacity_str", 3u, 2u, false},
    {"distortion_str", 1u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1130 = {{
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1131 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1131 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1131 = {{
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1132 = {{"06.map","01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,18> WARLORD_NATIVE_PARAMETERS_1132 = {{
    {"selectioncolor", 5u, 0u, true},
    {"21.uvscale.x", 2u, 2u, false},
    {"22.uvscale.y", 2u, 3u, false},
    {"time", 4u, 0u, false},
    {"04.map_a_panning_x", 0u, 3u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"09.map_d_panning_x", 2u, 0u, false},
    {"07.map_d_uvscale_r", 1u, 2u, false},
    {"08.map_d_uvscale_g", 1u, 3u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 1u, 0u, false},
    {"05.map_a_panning_y", 1u, 1u, false},
    {"29.cmap.direct", 3u, 0u, false},
    {"30.cmap.time(rotrate)", 3u, 1u, false},
    {"36.str", 3u, 2u, false},
    {"37.power", 3u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1132 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", false},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1134 = {{"02.map_e","22.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,23> WARLORD_NATIVE_PARAMETERS_1134 = {{
    {"selectioncolor", 7u, 0u, true},
    {"93.emissiion_color", 6u, 0u, true},
    {"22.map_a_uvscale_r", 2u, 2u, false},
    {"23.map_a_uvscale_g", 2u, 3u, false},
    {"24.map_a_panning_x", 3u, 0u, false},
    {"25.map_a_panning_y", 3u, 1u, false},
    {"time", 5u, 0u, false},
    {"05.map_e_panning_x", 1u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 2u, false},
    {"04.map_e_uvscale_g", 0u, 3u, false},
    {"11.uv.curvature", 2u, 1u, false},
    {"06.map_e_panning_y", 1u, 1u, false},
    {"91.desaturation", 4u, 0u, false},
    {"92.emissiion_power", 4u, 1u, false},
    {"06.radius", 1u, 2u, false},
    {"07.hardness", 1u, 3u, false},
    {"02.radius", 0u, 0u, false},
    {"03.hardness", 0u, 1u, false},
    {"09.power", 2u, 0u, false},
    {"95.str", 4u, 2u, false},
    {"96.power", 4u, 3u, false},
    {"32.fresnal_power", 3u, 2u, false},
    {"33.fresnal_str", 3u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,27> WARLORD_NATIVE_SWITCHES_1134 = {{
    {"00.usespecullar", false},
    {"11.mape+mapf", false},
    {"10.use_mapb", false},
    {"01.use_emissionmap", true},
    {"01.usemesh", false},
    {"05.useinvert", true},
    {"00.usedistortion", false},
    {"01.usesphmask", true},
    {"25.xinvert", false},
    {"20.usetwirl", false},
    {"01.usecoordinate_index1", false},
    {"94.sphmask+(texmap*phasemap)", false},
    {"01.effectonalphatex", false},
    {"11.useclamptexmap", false},
    {"91.mapch.r", true},
    {"92.mapch.g", true},
    {"00.checkisdepthbiasalpha", false},
    {"21.usephasetexmap", true},
    {"30.usefresnal", true},
    {"31.fresnal.invert", false},
    {"00.use_uvdistort", false},
    {"29.usedynamic_ypanning", false},
    {"23.usedynamicstr", false},
    {"93.mapch.b", true},
    {"10.fresnal", true},
    {"99.alphadissolve", false},
    {"94.mapch.a", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1135 = {{"01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,15> WARLORD_NATIVE_PARAMETERS_1135 = {{
    {"selectioncolor", 5u, 0u, true},
    {"meshemitterdynamicparameter", 4u, 0u, true},
    {"21.uvscale.x", 1u, 0u, false},
    {"22.uvscale.y", 1u, 1u, false},
    {"time", 3u, 0u, false},
    {"04.map_a_panning_x", 0u, 2u, false},
    {"02.map_a_uvscale_r", 0u, 0u, false},
    {"03.map_a_uvscale_g", 0u, 1u, false},
    {"05.map_a_panning_y", 0u, 3u, false},
    {"29.cmap.direct", 1u, 2u, false},
    {"30.cmap.time(rotrate)", 1u, 3u, false},
    {"36.str", 2u, 2u, false},
    {"37.power", 2u, 3u, false},
    {"32.fresnal_power", 2u, 0u, false},
    {"33.fresnal_str", 2u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1135 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", true},
    {"00.use_uvdistort", false},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", false},
    {"00.use_emission", false},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1136 = {{"addmap1_tex","uvnoise_tex","colormap2_tex","colormap_tex","main_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,39> WARLORD_NATIVE_PARAMETERS_1136 = {{
    {"selectioncolor", 11u, 0u, true},
    {"colormap2_color", 9u, 0u, true},
    {"colormap_color", 10u, 0u, true},
    {"main_tex_upos", 6u, 1u, false},
    {"main_tex_vpos", 6u, 2u, false},
    {"main_ucoord", 6u, 3u, false},
    {"twist_str", 7u, 2u, false},
    {"main_tex_upanner", 6u, 0u, false},
    {"curve_power", 4u, 1u, false},
    {"main_v_panner", 7u, 0u, false},
    {"addmap1_utile", 0u, 3u, false},
    {"addmap1_vtile", 1u, 0u, false},
    {"addmap1_panner", 0u, 0u, false},
    {"uvnoise_utile", 8u, 2u, false},
    {"uvnoise_vtile", 8u, 3u, false},
    {"uvnoise_panner", 7u, 3u, false},
    {"uvnoise_power", 8u, 0u, false},
    {"uvnoise_str", 8u, 1u, false},
    {"addmap1_power", 0u, 1u, false},
    {"addmap1_str", 0u, 2u, false},
    {"colormap2_ucoord", 2u, 0u, false},
    {"colormap2_vcoord", 2u, 1u, false},
    {"colormap2_vpanner", 2u, 2u, false},
    {"colormap2_power", 1u, 2u, false},
    {"colormap2_str", 1u, 3u, false},
    {"colormap2_destu", 1u, 1u, false},
    {"colormap_ucoord", 3u, 2u, false},
    {"colormap_vcoord", 3u, 3u, false},
    {"colormap_vpanner", 4u, 0u, false},
    {"colormap_power", 3u, 0u, false},
    {"colormap_str", 3u, 1u, false},
    {"colormap_destu", 2u, 3u, false},
    {"main_str", 5u, 3u, false},
    {"main_power", 5u, 2u, false},
    {"out_opa_range", 7u, 1u, false},
    {"in_opa_range", 4u, 3u, false},
    {"in_opa_str", 5u, 0u, false},
    {"main_alpha", 5u, 1u, false},
    {"distortion_str", 4u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1136 = {{
    {"use_distortion", false},
    {"use_halfmoon", false},
    {"multiply_addmap2", false},
    {"main_tex_useuvnoise", false},
    {"use_uvnoise", true},
    {"use_dynamic_vcoord", true},
    {"uvnoise_v_pannertime", false},
    {"main_v_pannertime", false},
    {"main_u_pannertime", false},
    {"use_colormap2", true},
    {"colormap_v_pannertime", false},
    {"colormap2_v_pannertime", false},
    {"use_addmap2", false},
    {"addmap2_v_pannertime", false},
    {"use_addmap1", true},
    {"addmap1_v_pannertime", false},
    {"use_mesh", false},
    {"use_depth_alpha", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1137 = {{"22.map_a","06.map"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,19> WARLORD_NATIVE_PARAMETERS_1137 = {{
    {"selectioncolor", 6u, 0u, true},
    {"93.emissiion_color", 5u, 0u, true},
    {"22.map_a_uvscale_r", 2u, 3u, false},
    {"23.map_a_uvscale_g", 3u, 0u, false},
    {"06.radius", 0u, 3u, false},
    {"07.hardness", 1u, 0u, false},
    {"02.radius", 0u, 0u, false},
    {"03.hardness", 0u, 1u, false},
    {"09.power", 2u, 0u, false},
    {"11.uv.curvature", 2u, 2u, false},
    {"time", 4u, 0u, false},
    {"09.map_d_panning_x", 1u, 3u, false},
    {"07.map_d_uvscale_r", 1u, 1u, false},
    {"08.map_d_uvscale_g", 1u, 2u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 0u, 2u, false},
    {"24.map_a_panning_x", 3u, 1u, false},
    {"95.str", 3u, 2u, false},
    {"96.power", 3u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,27> WARLORD_NATIVE_SWITCHES_1137 = {{
    {"00.usespecullar", false},
    {"11.mape+mapf", false},
    {"10.use_mapb", false},
    {"01.use_emissionmap", false},
    {"01.usemesh", false},
    {"05.useinvert", true},
    {"00.usedistortion", false},
    {"01.usesphmask", true},
    {"25.xinvert", false},
    {"20.usetwirl", false},
    {"01.usecoordinate_index1", false},
    {"94.sphmask+(texmap*phasemap)", false},
    {"01.effectonalphatex", false},
    {"11.useclamptexmap", false},
    {"91.mapch.r", true},
    {"92.mapch.g", true},
    {"00.checkisdepthbiasalpha", false},
    {"21.usephasetexmap", true},
    {"30.usefresnal", false},
    {"31.fresnal.invert", false},
    {"00.use_uvdistort", true},
    {"29.usedynamic_ypanning", true},
    {"23.usedynamicstr", false},
    {"93.mapch.b", true},
    {"10.fresnal", true},
    {"99.alphadissolve", false},
    {"94.mapch.a", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1138 = {{"uv_noise_01_tex","emissive_tex","alpha_tex","mask_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,14> WARLORD_NATIVE_PARAMETERS_1138 = {{
    {"selectioncolor", 4u, 0u, true},
    {"uv_panning_x", 2u, 2u, false},
    {"uv_panning_y", 2u, 3u, false},
    {"uv_noise_01_panning_x", 1u, 2u, false},
    {"uv_noise_01_tiling_x", 2u, 0u, false},
    {"uv_noise_01_tiling_y", 2u, 1u, false},
    {"uv_noise_01_panning_y", 1u, 3u, false},
    {"uv_noise_01_intensity", 1u, 1u, false},
    {"uv_scale", 3u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 3u, false},
    {"edge_intensity", 0u, 1u, false},
    {"edge_power", 0u, 2u, false},
    {"emissive_power", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,19> WARLORD_NATIVE_SWITCHES_1138 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", false},
    {"use_extra_alpha", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", true},
    {"use_uv_panning", true},
    {"use_uv_rotation", false},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex", true},
    {"use_fresnel_alpha", false},
    {"use_alpha_mask", true},
    {"use_edge_glow", true},
    {"non_alpha_chanel", false},
    {"use_uv_noise_tex_02", false},
    {"use_dynparam_extra_panning", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1139 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,23> WARLORD_NATIVE_PARAMETERS_1139 = {{
    {"selectioncolor", 6u, 0u, true},
    {"11.direct", 2u, 2u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 2u, false},
    {"03.map_a_uvscale_g", 0u, 3u, false},
    {"17.map_d_panning_x", 4u, 2u, false},
    {"15.map_d_uvscale_r", 4u, 0u, false},
    {"16.map_d_uvscale_g", 4u, 1u, false},
    {"18.map_d_panning_y", 4u, 3u, false},
    {"13.distortstr", 3u, 3u, false},
    {"05.map_a_panning_y", 1u, 1u, false},
    {"09.map_b_panning_x", 2u, 0u, false},
    {"07.map_b_uvscale_r", 1u, 2u, false},
    {"08.map_b_uvscale_g", 1u, 3u, false},
    {"10.map_b_panning_y", 2u, 1u, false},
    {"31.noisepower", 5u, 0u, false},
    {"32.noisestr", 5u, 1u, false},
    {"11.width.power", 3u, 0u, false},
    {"12.width.strength", 3u, 2u, false},
    {"01.height.power", 0u, 0u, false},
    {"02.height.strength", 0u, 1u, false},
    {"11.range", 2u, 3u, false},
    {"12.power", 3u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,12> WARLORD_NATIVE_SWITCHES_1139 = {{
    {"00.checkisdepthbiasalpha", false},
    {"10.usefakedepthbiasalpha", true},
    {"01.useconeuv", false},
    {"30.usefresnal", false},
    {"00.usenoise", true},
    {"00.blendingadd", false},
    {"30.use((map_a+map_b)*0.5)", true},
    {"00.useuvnoise", true},
    {"00.usecolor", false},
    {"01.usenoise", false},
    {"00z.usemapcolor", false},
    {"01.usemesh", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1140 = {{"01.diffmap_a","native_texture_1"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,7> WARLORD_NATIVE_PARAMETERS_1140 = {{
    {"selectioncolor", 3u, 0u, true},
    {"82.diffusecolor", 2u, 0u, true},
    {"02.diffmap_a.uvscale", 0u, 0u, false},
    {"81.diffusedesat", 1u, 0u, false},
    {"09.str", 0u, 1u, false},
    {"10.power", 0u, 2u, false},
    {"31.dissolvemapscale", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,13> WARLORD_NATIVE_SWITCHES_1140 = {{
    {"00.useinvertdepthbiasalpha", false},
    {"01.usemesh", false},
    {"30.usedissolve", true},
    {"00.usealphamap", false},
    {"00.usebump", false},
    {"06.mapch.r", true},
    {"07.mapch.g", true},
    {"08.mapch.b", true},
    {"00.useemission", false},
    {"10.use2maps", false},
    {"01.usevertexcolor2emissive", false},
    {"00.usefakelight", false},
    {"00.usespecullar", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1141 = {{"01.diffmap_a","01.emismap","native_texture_2"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,12> WARLORD_NATIVE_PARAMETERS_1141 = {{
    {"selectioncolor", 5u, 0u, true},
    {"82.diffusecolor", 4u, 0u, true},
    {"03.emap_uv.x.scale", 0u, 1u, false},
    {"04.emap_uv.y.scale", 0u, 2u, false},
    {"21.phasepantimescale", 1u, 2u, false},
    {"19.emissiion_color", 3u, 0u, true},
    {"02.diffmap_a.uvscale", 0u, 0u, false},
    {"81.diffusedesat", 2u, 0u, false},
    {"15.emissiion_power", 1u, 1u, false},
    {"09.str", 0u, 3u, false},
    {"10.power", 1u, 0u, false},
    {"31.dissolvemapscale", 1u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,13> WARLORD_NATIVE_SWITCHES_1141 = {{
    {"00.useinvertdepthbiasalpha", false},
    {"01.usemesh", false},
    {"30.usedissolve", true},
    {"00.usealphamap", false},
    {"00.usebump", false},
    {"06.mapch.r", true},
    {"07.mapch.g", true},
    {"08.mapch.b", true},
    {"00.useemission", true},
    {"10.use2maps", false},
    {"01.usevertexcolor2emissive", true},
    {"00.usefakelight", false},
    {"00.usespecullar", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1142 = {{"uv_noise_tex_02","emissivee_wave_texture","uv_noise_tex_01","worldoffset_emissive_tex_01"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,19> WARLORD_NATIVE_PARAMETERS_1142 = {{
    {"selectioncolor", 6u, 0u, true},
    {"hole_bright", 5u, 0u, true},
    {"emissive_panspeed", 0u, 2u, false},
    {"worldoffset_uv_noise_texcoord(x,y)_01", 3u, 2u, false},
    {"worldoffset_uv_noise_texcoord(x,y)_02", 3u, 3u, false},
    {"uv_noise_velue_centeralpha_02", 2u, 3u, false},
    {"worldoffset_uv_noise_texcoord(x,y)_03", 4u, 0u, false},
    {"uv_noise_velue_emissive_tex_01", 3u, 0u, false},
    {"emissive_texcoord_v", 1u, 1u, false},
    {"emissive_texcoord_u", 1u, 0u, false},
    {"uv_noise_pola_tex_velue", 2u, 1u, false},
    {"uv_noise_velue_centeralpha", 2u, 2u, false},
    {"alpha_radius", 0u, 1u, false},
    {"outline_hardness", 1u, 3u, false},
    {"worldoffset_emissive_texcoord(x,y)", 3u, 1u, false},
    {"emissive_power", 0u, 3u, false},
    {"emissive_wave_tex_stragth", 1u, 2u, false},
    {"outline_strangth", 2u, 0u, false},
    {"alpha_power_02", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1142 = {{
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1143 = {{"01.map.n","01.emismap","01.map.d"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,18> WARLORD_NATIVE_PARAMETERS_1143 = {{
    {"selectioncolor", 7u, 0u, true},
    {"11.normalmap.str", 2u, 2u, false},
    {"03.emap_uv.x.scale", 0u, 2u, false},
    {"04.emap_uv.y.scale", 1u, 1u, false},
    {"19.emissiion_color", 4u, 0u, true},
    {"meshemitterdynamicparameter", 6u, 0u, true},
    {"02.uvscale.x", 0u, 1u, false},
    {"03.uvscale.y", 1u, 0u, false},
    {"93.emissiion_color", 5u, 0u, true},
    {"05.n.panning.x", 1u, 2u, false},
    {"02.n.uvscale.x", 0u, 0u, false},
    {"03.n.uvscale.y", 0u, 3u, false},
    {"06.n.panning.y", 2u, 0u, false},
    {"15.emissiion_power", 2u, 3u, false},
    {"91.desaturation", 3u, 0u, false},
    {"92.emissiion_power", 3u, 1u, false},
    {"05.power", 1u, 3u, false},
    {"06.str", 2u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,23> WARLORD_NATIVE_SWITCHES_1143 = {{
    {"00.usenormalmap", true},
    {"00.usealphamap", false},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"19.mapa+mapclamp", false},
    {"20.use_clampmap", false},
    {"00.usefakelight", true},
    {"01.invert", false},
    {"00.usediffuse", true},
    {"00.usespecullar", false},
    {"30.dependonfakeli", true},
    {"20.usefresspec", false},
    {"00.useemission", true},
    {"20.usefresnal", true},
    {"00.use_uvdistort", false},
    {"21.invert", true},
    {"use_cascade_vertexcolor", true},
    {"33.mapch.b", true},
    {"11.usemapd.uvscale.dyna", false},
    {"11.usemape.uvscale.dyna", false},
    {"01.usemesh", true},
    {"50.dissolvetype-mosaic", true},
    {"25.use_yclamp", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1144 = {{"06.map","02.map_e","01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,25> WARLORD_NATIVE_PARAMETERS_1144 = {{
    {"selectioncolor", 7u, 0u, true},
    {"93.emissiion_color", 6u, 0u, true},
    {"21.uvscale.x", 3u, 2u, false},
    {"22.uvscale.y", 3u, 3u, false},
    {"time", 5u, 2u, false},
    {"05.map_e_panning_x", 2u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 1u, false},
    {"09.map_d_panning_x", 3u, 0u, false},
    {"07.map_d_uvscale_r", 2u, 2u, false},
    {"08.map_d_uvscale_g", 2u, 3u, false},
    {"10.map_d_panning_y", 3u, 1u, false},
    {"05.distort_str", 1u, 2u, false},
    {"06.map_e_panning_y", 2u, 1u, false},
    {"91.desaturation", 5u, 0u, false},
    {"92.emissiion_power", 5u, 1u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"05.map_a_panning_y", 1u, 3u, false},
    {"29.cmap.direct", 4u, 0u, false},
    {"30.cmap.time(rotrate)", 4u, 1u, false},
    {"36.str", 4u, 2u, false},
    {"37.power", 4u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1144 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"31.mapch.r", false},
    {"32.mapch.g", false},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", false},
    {"34.mapch.a", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1145 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1145 = {{
    {"selectioncolor", 2u, 0u, true},
    {"uv_scale", 1u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1145 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"non_alpha_chanel", false},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,6> WARLORD_NATIVE_TEXTURES_1146 = {{"native_texture_0","native_texture_1","native_texture_2","sparkle_tex","edgedeco texture01","native_texture_5"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_1146 = {{
    {"selectioncolor", 1u, 0u, true},
    {"sparkle_paning", 0u, 1u, false},
    {"sparkle_tiling", 0u, 2u, false},
    {"sparkle_intensity", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1146 = {{
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1147 = {{"01.heightmap","01.diffmap_a","01.emismap","01.alphamaskmap","native_texture_4"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,14> WARLORD_NATIVE_PARAMETERS_1147 = {{
    {"selectioncolor", 5u, 0u, true},
    {"82.diffusecolor", 4u, 0u, true},
    {"03.emap_uv.x.scale", 0u, 2u, false},
    {"04.emap_uv.y.scale", 1u, 0u, false},
    {"21.phasepantimescale", 2u, 0u, false},
    {"19.emissiion_color", 3u, 0u, true},
    {"02.diffmap_a.uvscale", 0u, 1u, false},
    {"04.bumpzero.pos", 0u, 3u, false},
    {"02.bumpheight", 0u, 0u, false},
    {"81.diffusedesat", 2u, 2u, false},
    {"15.emissiion_power", 1u, 3u, false},
    {"09.str", 1u, 1u, false},
    {"10.power", 1u, 2u, false},
    {"31.dissolvemapscale", 2u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,13> WARLORD_NATIVE_SWITCHES_1147 = {{
    {"00.useinvertdepthbiasalpha", false},
    {"01.usemesh", false},
    {"30.usedissolve", true},
    {"00.usealphamap", true},
    {"00.usebump", true},
    {"06.mapch.r", true},
    {"07.mapch.g", true},
    {"08.mapch.b", true},
    {"00.useemission", true},
    {"10.use2maps", false},
    {"01.usevertexcolor2emissive", false},
    {"00.usefakelight", false},
    {"00.usespecullar", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1148 = {{"uv_noise_tex","emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,10> WARLORD_NATIVE_PARAMETERS_1148 = {{
    {"selectioncolor", 3u, 0u, true},
    {"emissive_color&intensity", 2u, 0u, true},
    {"uv_noise_panning_x", 0u, 3u, false},
    {"uv_noise_panning_y", 1u, 0u, false},
    {"uv_panning_x", 1u, 2u, false},
    {"uv_panning_y", 1u, 3u, false},
    {"uv_noise_tilling", 1u, 1u, false},
    {"uv_noise_intensity", 0u, 2u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_tex_desturation", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_1148 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_tex", false},
    {"use_emissive_vertcolor", false},
    {"use_nonuvnoise", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1149 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_1149 = {{
    {"selectioncolor", 1u, 0u, true},
    {"uv_scale", 0u, 3u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 1u, false},
    {"emissive_power", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_1149 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"use_distortion_floor", false},
    {"non_alpha_chanel", true},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex_02", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1150 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,28> WARLORD_NATIVE_PARAMETERS_1150 = {{
    {"selectioncolor", 8u, 0u, true},
    {"02.color_a", 7u, 0u, true},
    {"11.direct", 2u, 3u, false},
    {"04.map_a_panning_x", 1u, 1u, false},
    {"02.map_a_uvscale_r", 0u, 3u, false},
    {"03.map_a_uvscale_g", 1u, 0u, false},
    {"17.map_d_panning_x", 5u, 2u, false},
    {"15.map_d_uvscale_r", 5u, 0u, false},
    {"16.map_d_uvscale_g", 5u, 1u, false},
    {"18.map_d_panning_y", 5u, 3u, false},
    {"13.distortstr", 4u, 1u, false},
    {"05.map_a_panning_y", 1u, 2u, false},
    {"09.map_b_panning_x", 2u, 1u, false},
    {"07.map_b_uvscale_r", 1u, 3u, false},
    {"08.map_b_uvscale_g", 2u, 0u, false},
    {"10.map_b_panning_y", 2u, 2u, false},
    {"12.desaturation", 3u, 2u, false},
    {"13.emissiion_str", 4u, 2u, false},
    {"15.emissiion_power", 4u, 3u, false},
    {"31.noisepower", 6u, 0u, false},
    {"32.noisestr", 6u, 1u, false},
    {"11.width.power", 3u, 1u, false},
    {"12.width.strength", 4u, 0u, false},
    {"01.height.power", 0u, 1u, false},
    {"02.height.strength", 0u, 2u, false},
    {"11.range", 3u, 0u, false},
    {"12.power", 3u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,12> WARLORD_NATIVE_SWITCHES_1150 = {{
    {"00.checkisdepthbiasalpha", true},
    {"10.usefakedepthbiasalpha", true},
    {"01.useconeuv", false},
    {"30.usefresnal", false},
    {"00.usenoise", true},
    {"00.blendingadd", false},
    {"30.use((map_a+map_b)*0.5)", true},
    {"00.useuvnoise", true},
    {"00.usecolor", true},
    {"01.usenoise", false},
    {"00z.usemapcolor", true},
    {"01.usemesh", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1151 = {{"12.map_f","06.map","02.map_e"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,29> WARLORD_NATIVE_PARAMETERS_1151 = {{
    {"selectioncolor", 8u, 0u, true},
    {"93.emissiion_color", 7u, 0u, true},
    {"time", 6u, 2u, false},
    {"15.map_e_panning_x", 4u, 2u, false},
    {"13.map_e_uvscale_r", 4u, 0u, false},
    {"14.map_e_uvscale_g", 4u, 1u, false},
    {"11.uv.curvature", 3u, 3u, false},
    {"09.map_d_panning_x", 3u, 0u, false},
    {"07.map_d_uvscale_r", 2u, 2u, false},
    {"08.map_d_uvscale_g", 2u, 3u, false},
    {"10.map_d_panning_y", 3u, 2u, false},
    {"05.distort_str", 1u, 1u, false},
    {"16.map_e_panning_y", 4u, 3u, false},
    {"05.map_e_panning_x", 1u, 2u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 0u, false},
    {"06.map_e_panning_y", 1u, 3u, false},
    {"91.desaturation", 5u, 2u, false},
    {"92.emissiion_power", 5u, 3u, false},
    {"06.radius", 2u, 0u, false},
    {"07.hardness", 2u, 1u, false},
    {"02.radius", 0u, 1u, false},
    {"03.hardness", 0u, 2u, false},
    {"09.power", 3u, 1u, false},
    {"95.str", 6u, 0u, false},
    {"96.power", 6u, 1u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"32.fresnal_power", 5u, 0u, false},
    {"33.fresnal_str", 5u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,27> WARLORD_NATIVE_SWITCHES_1151 = {{
    {"00.usespecullar", false},
    {"11.mape+mapf", true},
    {"10.use_mapb", true},
    {"01.use_emissionmap", true},
    {"01.usemesh", false},
    {"05.useinvert", true},
    {"00.usedistortion", false},
    {"01.usesphmask", true},
    {"25.xinvert", false},
    {"20.usetwirl", false},
    {"01.usecoordinate_index1", false},
    {"94.sphmask+(texmap*phasemap)", false},
    {"01.effectonalphatex", false},
    {"11.useclamptexmap", false},
    {"91.mapch.r", true},
    {"92.mapch.g", true},
    {"00.checkisdepthbiasalpha", true},
    {"21.usephasetexmap", false},
    {"30.usefresnal", true},
    {"31.fresnal.invert", false},
    {"00.use_uvdistort", true},
    {"29.usedynamic_ypanning", false},
    {"23.usedynamicstr", false},
    {"93.mapch.b", true},
    {"10.fresnal", true},
    {"99.alphadissolve", false},
    {"94.mapch.a", true},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1152 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1152 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1152 = {{
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1153 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1153 = {{
    {"selectioncolor", 2u, 0u, true},
    {"10.radius", 1u, 0u, false},
    {"04.ringthickness", 0u, 1u, false},
    {"05.shapehardness", 0u, 2u, false},
    {"07.power", 0u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,8> WARLORD_NATIVE_SWITCHES_1153 = {{
    {"00.alphamap", false},
    {"20.usehemisphere", false},
    {"00.useemissioncolor", false},
    {"00.checkisdepthbiasalpha", true},
    {"01.usesoftshape", false},
    {"02.useouthardness", false},
    {"00.use_uvdistort", false},
    {"00.usedistortion", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1154 = {{"map_a","map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1154 = {{
    {"selectioncolor", 2u, 0u, true},
    {"readme", 1u, 0u, false},
    {"map_a_uvscale", 0u, 1u, false},
    {"map_b_uvscale", 0u, 2u, false},
    {"map_str", 0u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,7> WARLORD_NATIVE_SWITCHES_1154 = {{
    {"00.uselightshift", false},
    {"01.use_depthbiasalpha", true},
    {"btimebezier", true},
    {"00.userainbow", false},
    {"20.usespacular", false},
    {"01.usemacrouv", false},
    {"01.usemesh", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1155 = {{"native_texture_0","native_texture_1","native_texture_2","native_texture_3"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1155 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1155 = {{
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1156 = {{"02.map_e","06.map","22.map_a"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,28> WARLORD_NATIVE_PARAMETERS_1156 = {{
    {"selectioncolor", 8u, 0u, true},
    {"93.emissiion_color", 7u, 0u, true},
    {"22.map_a_uvscale_r", 4u, 1u, false},
    {"23.map_a_uvscale_g", 4u, 2u, false},
    {"24.map_a_panning_x", 4u, 3u, false},
    {"25.map_a_panning_y", 5u, 0u, false},
    {"time", 6u, 1u, false},
    {"05.map_e_panning_x", 1u, 3u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 0u, false},
    {"11.uv.curvature", 4u, 0u, false},
    {"09.map_d_panning_x", 3u, 1u, false},
    {"07.map_d_uvscale_r", 2u, 3u, false},
    {"08.map_d_uvscale_g", 3u, 0u, false},
    {"10.map_d_panning_y", 3u, 3u, false},
    {"05.distort_str", 1u, 1u, false},
    {"06.map_e_panning_y", 2u, 0u, false},
    {"91.desaturation", 5u, 1u, false},
    {"92.emissiion_power", 5u, 2u, false},
    {"06.radius", 2u, 1u, false},
    {"07.hardness", 2u, 2u, false},
    {"02.radius", 0u, 1u, false},
    {"03.hardness", 0u, 2u, false},
    {"09.power", 3u, 2u, false},
    {"95.str", 5u, 3u, false},
    {"96.power", 6u, 0u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"05.distortion_str", 1u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,27> WARLORD_NATIVE_SWITCHES_1156 = {{
    {"00.usespecullar", false},
    {"11.mape+mapf", false},
    {"10.use_mapb", false},
    {"01.use_emissionmap", true},
    {"01.usemesh", false},
    {"05.useinvert", true},
    {"00.usedistortion", true},
    {"01.usesphmask", true},
    {"25.xinvert", false},
    {"20.usetwirl", false},
    {"01.usecoordinate_index1", false},
    {"94.sphmask+(texmap*phasemap)", false},
    {"01.effectonalphatex", false},
    {"11.useclamptexmap", false},
    {"91.mapch.r", true},
    {"92.mapch.g", true},
    {"00.checkisdepthbiasalpha", true},
    {"21.usephasetexmap", true},
    {"30.usefresnal", false},
    {"31.fresnal.invert", false},
    {"00.use_uvdistort", true},
    {"29.usedynamic_ypanning", false},
    {"23.usedynamicstr", false},
    {"93.mapch.b", true},
    {"10.fresnal", true},
    {"99.alphadissolve", false},
    {"94.mapch.a", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1157 = {{"uv_noise_01_tex","emissive_tex","transition texture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,17> WARLORD_NATIVE_PARAMETERS_1157 = {{
    {"selectioncolor", 5u, 0u, true},
    {"meshemitterdynamicparameter", 4u, 0u, true},
    {"total_scale", 1u, 1u, false},
    {"uv_noise_01_panning_x", 3u, 0u, false},
    {"uv_noise_01_tiling", 3u, 2u, false},
    {"uv_noise_01_panning_y", 3u, 1u, false},
    {"uv_noise_01_intensity", 2u, 3u, false},
    {"emissive_uv_scale_x", 0u, 3u, false},
    {"emissive_uv_scale_y", 1u, 0u, false},
    {"emissive_desaturation", 0u, 0u, false},
    {"emissive_intensity", 0u, 1u, false},
    {"transition_panning_x", 2u, 0u, false},
    {"transition_tiling", 2u, 2u, false},
    {"transition_panning_y", 2u, 1u, false},
    {"transition line thickness", 1u, 2u, false},
    {"emissive_line_intensity", 0u, 2u, false},
    {"transition thickness", 1u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,6> WARLORD_NATIVE_SWITCHES_1157 = {{
    {"non_alpha_chanel", true},
    {"use_distortion", false},
    {"use_fresnel_alpha", false},
    {"use_meshtype", true},
    {"use_polarcoordinate", true},
    {"use_uv_noise_02", false},
}};

inline constexpr std::array<std::string_view,6> WARLORD_NATIVE_TEXTURES_1158 = {{"06.map","02.map_e","12.map_f","01.map_a","11.map_b","01.specmap"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,35> WARLORD_NATIVE_PARAMETERS_1158 = {{
    {"selectioncolor", 10u, 0u, true},
    {"05.specmap_uvscale.x", 2u, 2u, false},
    {"06.specmap_uvscale.y", 3u, 0u, false},
    {"93.emissiion_color", 9u, 0u, true},
    {"09.specmap_color", 8u, 0u, true},
    {"time", 7u, 3u, false},
    {"05.map_e_panning_x", 2u, 1u, false},
    {"03.map_e_uvscale_r", 1u, 0u, false},
    {"04.map_e_uvscale_g", 1u, 2u, false},
    {"09.map_d_panning_x", 4u, 1u, false},
    {"07.map_d_uvscale_r", 3u, 2u, false},
    {"08.map_d_uvscale_g", 3u, 3u, false},
    {"10.map_d_panning_y", 4u, 2u, false},
    {"05.distort_str", 1u, 3u, false},
    {"06.map_e_panning_y", 2u, 3u, false},
    {"15.map_f_panning_x", 6u, 1u, false},
    {"13.map_f_uvscale_r", 5u, 1u, false},
    {"14.map_f_uvscale_g", 5u, 3u, false},
    {"16.map_f_panning_y", 6u, 2u, false},
    {"91.desaturation", 7u, 1u, false},
    {"92.emissiion_power", 7u, 2u, false},
    {"04.map_a_panning_x", 1u, 1u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 3u, false},
    {"05.map_a_panning_y", 2u, 0u, false},
    {"14.map_b_panning_x", 5u, 2u, false},
    {"12.map_b_uvscale_r", 4u, 3u, false},
    {"13.map_b_uvscale_g", 5u, 0u, false},
    {"15.map_b_panning_y", 6u, 0u, false},
    {"02.specmap_str", 0u, 2u, false},
    {"07.desaturation", 3u, 1u, false},
    {"08.specmap_power", 4u, 0u, false},
    {"36.str", 6u, 3u, false},
    {"37.power", 7u, 0u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1158 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", false},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", true},
    {"00.use_mapa", true},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", true},
    {"11.map_e + map_f", false},
    {"00.usespecullar", true},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1159 = {{"distorta_tex","distortb_tex","emissive_tex","maska_tex","maskb_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,18> WARLORD_NATIVE_PARAMETERS_1159 = {{
    {"selectioncolor", 5u, 0u, true},
    {"emissive_tileu", 3u, 1u, false},
    {"emissive_tilev", 3u, 2u, false},
    {"distorta_tileu", 0u, 3u, false},
    {"distorta_tilev", 1u, 0u, false},
    {"distorta_panneru", 0u, 1u, false},
    {"distorta_pannerv", 0u, 2u, false},
    {"distortb_tileu", 1u, 3u, false},
    {"distortb_tilev", 2u, 0u, false},
    {"distortb_panneru", 1u, 1u, false},
    {"distortb_pannerv", 1u, 2u, false},
    {"emissive_panneru", 2u, 1u, false},
    {"emissive_pannerv", 2u, 2u, false},
    {"distort_str", 0u, 0u, false},
    {"emissive_power", 2u, 3u, false},
    {"emissive_str", 3u, 0u, false},
    {"mask_power", 3u, 3u, false},
    {"mask_str", 4u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1159 = {{
    {"use_depthbiasalpha", false},
    {"use_mesh", false},
    {"use_dynamic_lerp", true},
    {"use_fanshape_alpha", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1160 = {{"native_texture_0","native_texture_1","native_texture_2"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1160 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1160 = {{
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1161 = {{"02.map_e","01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,24> WARLORD_NATIVE_PARAMETERS_1161 = {{
    {"selectioncolor", 8u, 0u, true},
    {"93.emissiion_color", 6u, 0u, true},
    {"meshemitterdynamicparameter", 7u, 0u, true},
    {"21.uvscale.x", 2u, 2u, false},
    {"22.uvscale.y", 2u, 3u, false},
    {"time", 5u, 0u, false},
    {"05.map_e_panning_x", 2u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 1u, false},
    {"06.map_e_panning_y", 2u, 1u, false},
    {"91.desaturation", 4u, 2u, false},
    {"92.emissiion_power", 4u, 3u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"05.map_a_panning_y", 1u, 3u, false},
    {"29.cmap.direct", 3u, 0u, false},
    {"30.cmap.time(rotrate)", 3u, 1u, false},
    {"36.str", 4u, 0u, false},
    {"37.power", 4u, 1u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"32.fresnal_power", 3u, 2u, false},
    {"33.fresnal_str", 3u, 3u, false},
    {"05.distortion_str", 1u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1161 = {{
    {"00.usedistortion", true},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", false},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1162 = {{"06.map","02.map_e","12.map_f","01.map_a","11.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,33> WARLORD_NATIVE_PARAMETERS_1162 = {{
    {"selectioncolor", 11u, 0u, true},
    {"61.twist.xy", 8u, 0u, true},
    {"93.emissiion_color", 9u, 0u, true},
    {"meshemitterdynamicparameter", 10u, 0u, true},
    {"time", 7u, 0u, false},
    {"05.map_e_panning_x", 2u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 1u, false},
    {"09.map_d_panning_x", 3u, 0u, false},
    {"07.map_d_uvscale_r", 2u, 2u, false},
    {"08.map_d_uvscale_g", 2u, 3u, false},
    {"10.map_d_panning_y", 3u, 1u, false},
    {"05.distort_str", 1u, 2u, false},
    {"06.map_e_panning_y", 2u, 1u, false},
    {"15.map_f_panning_x", 5u, 0u, false},
    {"13.map_f_uvscale_r", 4u, 0u, false},
    {"14.map_f_uvscale_g", 4u, 2u, false},
    {"16.map_f_panning_y", 5u, 1u, false},
    {"91.desaturation", 6u, 2u, false},
    {"92.emissiion_power", 6u, 3u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"05.map_a_panning_y", 1u, 3u, false},
    {"14.map_b_panning_x", 4u, 1u, false},
    {"12.map_b_uvscale_r", 3u, 2u, false},
    {"13.map_b_uvscale_g", 3u, 3u, false},
    {"15.map_b_panning_y", 4u, 3u, false},
    {"36.str", 6u, 0u, false},
    {"37.power", 6u, 1u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"32.fresnal_power", 5u, 2u, false},
    {"33.fresnal_str", 5u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1162 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", false},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", true},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", true},
    {"11.map_e + map_f", true},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", true},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1163 = {{"uv_noise_01_tex","emissive_tex","transition texture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,17> WARLORD_NATIVE_PARAMETERS_1163 = {{
    {"selectioncolor", 5u, 0u, true},
    {"meshemitterdynamicparameter", 4u, 0u, true},
    {"total_scale", 1u, 1u, false},
    {"uv_noise_01_panning_x", 3u, 0u, false},
    {"uv_noise_01_tiling", 3u, 2u, false},
    {"uv_noise_01_panning_y", 3u, 1u, false},
    {"uv_noise_01_intensity", 2u, 3u, false},
    {"emissive_uv_scale_x", 0u, 3u, false},
    {"emissive_uv_scale_y", 1u, 0u, false},
    {"emissive_desaturation", 0u, 0u, false},
    {"emissive_intensity", 0u, 1u, false},
    {"transition_panning_x", 2u, 0u, false},
    {"transition_tiling", 2u, 2u, false},
    {"transition_panning_y", 2u, 1u, false},
    {"transition line thickness", 1u, 2u, false},
    {"emissive_line_intensity", 0u, 2u, false},
    {"transition thickness", 1u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,6> WARLORD_NATIVE_SWITCHES_1163 = {{
    {"use_meshtype", true},
    {"use_polarcoordinate", true},
    {"non_alpha_chanel", false},
    {"use_uv_noise_02", false},
    {"use_fresnel_alpha", false},
    {"use_distortion", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1164 = {{"02.map_e","12.map_f","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,22> WARLORD_NATIVE_PARAMETERS_1164 = {{
    {"selectioncolor", 7u, 0u, true},
    {"93.emissiion_color", 5u, 0u, true},
    {"meshemitterdynamicparameter", 6u, 0u, true},
    {"21.uvscale.x", 2u, 0u, false},
    {"22.uvscale.y", 2u, 1u, false},
    {"time", 4u, 2u, false},
    {"05.map_e_panning_x", 0u, 2u, false},
    {"03.map_e_uvscale_r", 0u, 0u, false},
    {"04.map_e_uvscale_g", 0u, 1u, false},
    {"06.map_e_panning_y", 0u, 3u, false},
    {"15.map_f_panning_x", 1u, 2u, false},
    {"13.map_f_uvscale_r", 1u, 0u, false},
    {"14.map_f_uvscale_g", 1u, 1u, false},
    {"16.map_f_panning_y", 1u, 3u, false},
    {"91.desaturation", 4u, 0u, false},
    {"92.emissiion_power", 4u, 1u, false},
    {"29.cmap.direct", 2u, 2u, false},
    {"30.cmap.time(rotrate)", 2u, 3u, false},
    {"36.str", 3u, 2u, false},
    {"37.power", 3u, 3u, false},
    {"32.fresnal_power", 3u, 0u, false},
    {"33.fresnal_str", 3u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1164 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", false},
    {"32.mapch.g", false},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", false},
    {"10.use_mapb", false},
    {"00.use_mapa", false},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", false},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", true},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1165 = {{"06.map","02.map_e","12.map_f","01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,29> WARLORD_NATIVE_PARAMETERS_1165 = {{
    {"selectioncolor", 8u, 0u, true},
    {"93.emissiion_color", 7u, 0u, true},
    {"21.uvscale.x", 4u, 2u, false},
    {"22.uvscale.y", 4u, 3u, false},
    {"time", 6u, 2u, false},
    {"05.map_e_panning_x", 2u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 1u, false},
    {"09.map_d_panning_x", 3u, 0u, false},
    {"07.map_d_uvscale_r", 2u, 2u, false},
    {"08.map_d_uvscale_g", 2u, 3u, false},
    {"10.map_d_panning_y", 3u, 1u, false},
    {"05.distort_str", 1u, 2u, false},
    {"06.map_e_panning_y", 2u, 1u, false},
    {"15.map_f_panning_x", 4u, 0u, false},
    {"13.map_f_uvscale_r", 3u, 2u, false},
    {"14.map_f_uvscale_g", 3u, 3u, false},
    {"16.map_f_panning_y", 4u, 1u, false},
    {"91.desaturation", 6u, 0u, false},
    {"92.emissiion_power", 6u, 1u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"05.map_a_panning_y", 1u, 3u, false},
    {"29.cmap.direct", 5u, 0u, false},
    {"30.cmap.time(rotrate)", 5u, 1u, false},
    {"36.str", 5u, 2u, false},
    {"37.power", 5u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1165 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", true},
    {"11.map_e + map_f", true},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1166 = {{"native_texture_0","native_texture_1","emissive_tex","emissive_tex_02"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,10> WARLORD_NATIVE_PARAMETERS_1166 = {{
    {"selectioncolor", 3u, 0u, true},
    {"emissive_tiling_x", 1u, 2u, false},
    {"emissive_tiling_y", 1u, 3u, false},
    {"uv_rot", 2u, 0u, false},
    {"elec_panning", 0u, 3u, false},
    {"elec_noise_tiling", 0u, 2u, false},
    {"elec_shake_intensity", 1u, 0u, false},
    {"elec_distortion_intensity", 0u, 1u, false},
    {"elec_tick_cycle", 1u, 1u, false},
    {"alpha_power", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1166 = {{
    {"non_alpha_chanel", true},
    {"use_meshtype", false},
    {"use_dissolve", false},
    {"use_fresnel_alpha", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1167 = {{"01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,19> WARLORD_NATIVE_PARAMETERS_1167 = {{
    {"selectioncolor", 5u, 0u, true},
    {"11.direct", 2u, 2u, false},
    {"50.angle", 4u, 1u, false},
    {"11.width.power", 3u, 0u, false},
    {"12.width.strength", 3u, 2u, false},
    {"01.height.power", 0u, 0u, false},
    {"02.height.strength", 0u, 1u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 2u, false},
    {"03.map_a_uvscale_g", 0u, 3u, false},
    {"05.map_a_panning_y", 1u, 1u, false},
    {"09.map_b_panning_x", 2u, 0u, false},
    {"07.map_b_uvscale_r", 1u, 2u, false},
    {"08.map_b_uvscale_g", 1u, 3u, false},
    {"10.map_b_panning_y", 2u, 1u, false},
    {"31.noisepower", 3u, 3u, false},
    {"32.noisestr", 4u, 0u, false},
    {"11.range", 2u, 3u, false},
    {"12.power", 3u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,12> WARLORD_NATIVE_SWITCHES_1167 = {{
    {"00.checkisdepthbiasalpha", false},
    {"10.usefakedepthbiasalpha", true},
    {"01.useconeuv", true},
    {"30.usefresnal", false},
    {"00.usenoise", true},
    {"00.blendingadd", false},
    {"30.use((map_a+map_b)*0.5)", false},
    {"00.useuvnoise", false},
    {"01.usemesh", false},
    {"00.usecolor", false},
    {"01.usenoise", false},
    {"10.usemapcolor", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1168 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,19> WARLORD_NATIVE_PARAMETERS_1168 = {{
    {"selectioncolor", 6u, 0u, true},
    {"noisecolor", 5u, 0u, true},
    {"15.map_d_uvscale_r", 2u, 3u, false},
    {"16.map_d_uvscale_g", 3u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 2u, false},
    {"03.map_a_uvscale_g", 1u, 0u, false},
    {"04.map_a_panning_x", 1u, 2u, false},
    {"05.map_a_panning_y", 2u, 0u, false},
    {"02.map_b_uvscale_r", 0u, 3u, false},
    {"03.map_b_uvscale_g", 1u, 1u, false},
    {"04.map_b_panning_x", 1u, 3u, false},
    {"05.map_b_panning_y", 2u, 1u, false},
    {"power", 3u, 3u, false},
    {"13.distortstr", 2u, 2u, false},
    {"00.noisepower", 0u, 0u, false},
    {"00.noisestr", 0u, 1u, false},
    {"biasvalue", 3u, 1u, false},
    {"str", 4u, 0u, false},
    {"depthbiasdalpha_bias", 3u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,6> WARLORD_NATIVE_SWITCHES_1168 = {{
    {"checkisdepthbiasalpha", true},
    {"00.checkisnoiseon", true},
    {"11.uvmirroring", false},
    {"12.uvnoise", true},
    {"use_meshtype", false},
    {"use_multyply_noisecolor", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1169 = {{"06.map","02.map_e","01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,25> WARLORD_NATIVE_PARAMETERS_1169 = {{
    {"selectioncolor", 7u, 0u, true},
    {"93.emissiion_color", 6u, 0u, true},
    {"21.uvscale.x", 3u, 2u, false},
    {"22.uvscale.y", 3u, 3u, false},
    {"time", 5u, 2u, false},
    {"05.map_e_panning_x", 2u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 1u, false},
    {"09.map_d_panning_x", 3u, 0u, false},
    {"07.map_d_uvscale_r", 2u, 2u, false},
    {"08.map_d_uvscale_g", 2u, 3u, false},
    {"10.map_d_panning_y", 3u, 1u, false},
    {"05.distort_str", 1u, 2u, false},
    {"06.map_e_panning_y", 2u, 1u, false},
    {"91.desaturation", 5u, 0u, false},
    {"92.emissiion_power", 5u, 1u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"05.map_a_panning_y", 1u, 3u, false},
    {"29.cmap.direct", 4u, 0u, false},
    {"30.cmap.time(rotrate)", 4u, 1u, false},
    {"36.str", 4u, 2u, false},
    {"37.power", 4u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1169 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"31.mapch.r", false},
    {"32.mapch.g", false},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", false},
    {"34.mapch.a", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1170 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,18> WARLORD_NATIVE_PARAMETERS_1170 = {{
    {"selectioncolor", 5u, 0u, true},
    {"noisecolor", 4u, 0u, true},
    {"15.map_d_uvscale_r", 2u, 3u, false},
    {"16.map_d_uvscale_g", 3u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 2u, false},
    {"03.map_a_uvscale_g", 1u, 0u, false},
    {"04.map_a_panning_x", 1u, 2u, false},
    {"05.map_a_panning_y", 2u, 0u, false},
    {"02.map_b_uvscale_r", 0u, 3u, false},
    {"03.map_b_uvscale_g", 1u, 1u, false},
    {"04.map_b_panning_x", 1u, 3u, false},
    {"05.map_b_panning_y", 2u, 1u, false},
    {"power", 3u, 2u, false},
    {"13.distortstr", 2u, 2u, false},
    {"00.noisepower", 0u, 0u, false},
    {"00.noisestr", 0u, 1u, false},
    {"biasvalue", 3u, 1u, false},
    {"str", 3u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,6> WARLORD_NATIVE_SWITCHES_1170 = {{
    {"checkisdepthbiasalpha", false},
    {"00.checkisnoiseon", true},
    {"11.uvmirroring", false},
    {"12.uvnoise", true},
    {"use_meshtype", false},
    {"use_multyply_noisecolor", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1171 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,21> WARLORD_NATIVE_PARAMETERS_1171 = {{
    {"selectioncolor", 5u, 0u, true},
    {"power", 4u, 2u, false},
    {"str", 4u, 3u, false},
    {"04.map_a_panning_x", 1u, 2u, false},
    {"02.map_a_uvscale_r", 0u, 3u, false},
    {"03.map_a_uvscale_g", 1u, 1u, false},
    {"17.map_d_panning_x", 3u, 3u, false},
    {"15.map_d_uvscale_r", 3u, 1u, false},
    {"16.map_d_uvscale_g", 3u, 2u, false},
    {"18.map_d_panning_y", 4u, 0u, false},
    {"13.distortstr", 3u, 0u, false},
    {"05.map_a_panning_y", 1u, 3u, false},
    {"09.map_b_panning_x", 2u, 2u, false},
    {"07.map_b_uvscale_r", 2u, 0u, false},
    {"08.map_b_uvscale_g", 2u, 1u, false},
    {"10.map_b_panning_y", 2u, 3u, false},
    {"00.noisepower", 0u, 0u, false},
    {"00.noisestr", 0u, 1u, false},
    {"depthbiasdalpha_bias", 4u, 1u, false},
    {"01.range", 0u, 2u, false},
    {"02.power", 1u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_1171 = {{
    {"--usefakedepthbiasalpha", true},
    {"checkisdepthbiasalpha", true},
    {"00.checkisnoiseon", true},
    {"12.uvnoise", true},
    {"11.uvmirroring", false},
}};

inline constexpr std::array<std::string_view,7> WARLORD_NATIVE_TEXTURES_1172 = {{"06.map","02.map_e","12.map_f","01.map_a","11.map_b","21.map_c","01.specmap"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,39> WARLORD_NATIVE_PARAMETERS_1172 = {{
    {"selectioncolor", 11u, 0u, true},
    {"05.specmap_uvscale.x", 2u, 2u, false},
    {"06.specmap_uvscale.y", 3u, 0u, false},
    {"93.emissiion_color", 10u, 0u, true},
    {"21.uvscale.x", 6u, 3u, false},
    {"22.uvscale.y", 7u, 0u, false},
    {"09.specmap_color", 9u, 0u, true},
    {"time", 8u, 3u, false},
    {"05.map_e_panning_x", 2u, 1u, false},
    {"03.map_e_uvscale_r", 1u, 0u, false},
    {"04.map_e_uvscale_g", 1u, 2u, false},
    {"09.map_d_panning_x", 4u, 1u, false},
    {"07.map_d_uvscale_r", 3u, 2u, false},
    {"08.map_d_uvscale_g", 3u, 3u, false},
    {"10.map_d_panning_y", 4u, 2u, false},
    {"05.distort_str", 1u, 3u, false},
    {"06.map_e_panning_y", 2u, 3u, false},
    {"15.map_f_panning_x", 6u, 1u, false},
    {"13.map_f_uvscale_r", 5u, 1u, false},
    {"14.map_f_uvscale_g", 5u, 3u, false},
    {"16.map_f_panning_y", 6u, 2u, false},
    {"91.desaturation", 8u, 1u, false},
    {"92.emissiion_power", 8u, 2u, false},
    {"04.map_a_panning_x", 1u, 1u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 3u, false},
    {"05.map_a_panning_y", 2u, 0u, false},
    {"14.map_b_panning_x", 5u, 2u, false},
    {"12.map_b_uvscale_r", 4u, 3u, false},
    {"13.map_b_uvscale_g", 5u, 0u, false},
    {"15.map_b_panning_y", 6u, 0u, false},
    {"29.cmap.direct", 7u, 1u, false},
    {"30.cmap.time(rotrate)", 7u, 2u, false},
    {"02.specmap_str", 0u, 2u, false},
    {"07.desaturation", 3u, 1u, false},
    {"08.specmap_power", 4u, 0u, false},
    {"36.str", 7u, 3u, false},
    {"37.power", 8u, 0u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1172 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", true},
    {"00.use_mapa", true},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", true},
    {"11.map_e + map_f", false},
    {"00.usespecullar", true},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1173 = {{"01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,19> WARLORD_NATIVE_PARAMETERS_1173 = {{
    {"selectioncolor", 6u, 0u, true},
    {"11.direct", 2u, 3u, false},
    {"02.color_a", 5u, 0u, true},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 2u, false},
    {"03.map_a_uvscale_g", 0u, 3u, false},
    {"05.map_a_panning_y", 1u, 1u, false},
    {"09.map_b_panning_x", 2u, 0u, false},
    {"07.map_b_uvscale_r", 1u, 2u, false},
    {"08.map_b_uvscale_g", 1u, 3u, false},
    {"10.map_b_panning_y", 2u, 1u, false},
    {"11.desaturation", 2u, 2u, false},
    {"11.width.power", 3u, 1u, false},
    {"12.width.strength", 3u, 3u, false},
    {"01.height.power", 0u, 0u, false},
    {"02.height.strength", 0u, 1u, false},
    {"31.fresnal_power", 4u, 0u, false},
    {"11.range", 3u, 0u, false},
    {"12.power", 3u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,12> WARLORD_NATIVE_SWITCHES_1173 = {{
    {"00.checkisdepthbiasalpha", false},
    {"10.usefakedepthbiasalpha", true},
    {"01.useconeuv", false},
    {"30.usefresnal", true},
    {"00.usenoise", false},
    {"00.blendingadd", false},
    {"30.use((map_a+map_b)*0.5)", false},
    {"00.useuvnoise", false},
    {"01.usemesh", true},
    {"00.usecolor", true},
    {"01.usenoise", false},
    {"10.usemapcolor", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1174 = {{"01.map_a","06.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,18> WARLORD_NATIVE_PARAMETERS_1174 = {{
    {"selectioncolor", 5u, 0u, true},
    {"11.direct", 2u, 2u, false},
    {"11.width.power", 3u, 0u, false},
    {"12.width.strength", 3u, 2u, false},
    {"01.height.power", 0u, 0u, false},
    {"02.height.strength", 0u, 1u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 2u, false},
    {"03.map_a_uvscale_g", 0u, 3u, false},
    {"05.map_a_panning_y", 1u, 1u, false},
    {"09.map_b_panning_x", 2u, 0u, false},
    {"07.map_b_uvscale_r", 1u, 2u, false},
    {"08.map_b_uvscale_g", 1u, 3u, false},
    {"10.map_b_panning_y", 2u, 1u, false},
    {"31.noisepower", 3u, 3u, false},
    {"32.noisestr", 4u, 0u, false},
    {"11.range", 2u, 3u, false},
    {"12.power", 3u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,12> WARLORD_NATIVE_SWITCHES_1174 = {{
    {"00.checkisdepthbiasalpha", false},
    {"10.usefakedepthbiasalpha", true},
    {"01.useconeuv", false},
    {"30.usefresnal", false},
    {"00.usenoise", true},
    {"00.blendingadd", false},
    {"30.use((map_a+map_b)*0.5)", false},
    {"00.useuvnoise", false},
    {"01.usemesh", true},
    {"00.usecolor", false},
    {"01.usenoise", false},
    {"10.usemapcolor", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1175 = {{"31.map_e","06.map","04.map_anew","00.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,20> WARLORD_NATIVE_PARAMETERS_1175 = {{
    {"selectioncolor", 6u, 0u, true},
    {"37.mape_color", 5u, 0u, true},
    {"05.map_alod", 0u, 1u, false},
    {"39.map_a_panning_x", 3u, 2u, false},
    {"41.map_a_uvscale_r", 4u, 0u, false},
    {"42.map_a_uvscale_g", 4u, 1u, false},
    {"40.map_a_panning_y", 3u, 3u, false},
    {"09.map_d_panning_x", 1u, 3u, false},
    {"07.map_d_uvscale_r", 0u, 3u, false},
    {"08.map_d_uvscale_g", 1u, 1u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 0u, 0u, false},
    {"33.desaturation", 3u, 0u, false},
    {"35.mape_power", 3u, 1u, false},
    {"09.map_a_panning_x", 1u, 2u, false},
    {"07.map_a_uvscale_r", 0u, 2u, false},
    {"08.map_a_uvscale_g", 1u, 0u, false},
    {"10.map_a_panning_y", 2u, 0u, false},
    {"11.map_a_str", 2u, 2u, false},
    {"12.map_a_power", 2u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1175 = {{
    {"00.usedistortion", false},
    {"20.use_centrehole", false},
    {"00.usedepthbiasalpha", false},
    {"30.usefresnal", false},
    {"10.cameradistance", false},
    {"01.mapch.r", true},
    {"02.mapch.g", true},
    {"03.mapch.b", true},
    {"00.use_uvdistort", true},
    {"01.use_uvdistort_all", false},
    {"checkismeshemitvc", false},
    {"00.use_rgbinalphamap", false},
    {"06.use((r+g+b)*0.33)", false},
    {"15.use_panbezier", false},
    {"01.usecoordinate_index1", false},
    {"30.use_additionalmap", true},
    {"useupno_alpha", false},
    {"40.use_fanshape", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1176 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,9> WARLORD_NATIVE_PARAMETERS_1176 = {{
    {"selectioncolor", 2u, 0u, true},
    {"11.direct", 0u, 3u, false},
    {"11.width.power", 1u, 1u, false},
    {"12.width.strength", 1u, 3u, false},
    {"01.height.power", 0u, 1u, false},
    {"02.height.strength", 0u, 2u, false},
    {"11.range", 1u, 0u, false},
    {"12.power", 1u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,12> WARLORD_NATIVE_SWITCHES_1176 = {{
    {"00.checkisdepthbiasalpha", true},
    {"10.usefakedepthbiasalpha", true},
    {"01.useconeuv", false},
    {"30.usefresnal", false},
    {"00.usenoise", false},
    {"00.blendingadd", true},
    {"30.use((map_a+map_b)*0.5)", false},
    {"00.useuvnoise", false},
    {"01.usemesh", false},
    {"00.usecolor", false},
    {"01.usenoise", false},
    {"10.usemapcolor", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1177 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1177 = {{
    {"selectioncolor", 2u, 0u, true},
    {"power", 0u, 3u, false},
    {"str", 1u, 0u, false},
    {"depthbiasdalpha_bias", 0u, 2u, false},
    {"01.range", 0u, 0u, false},
    {"02.power", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,6> WARLORD_NATIVE_SWITCHES_1177 = {{
    {"--usefakedepthbiasalpha", true},
    {"checkisdepthbiasalpha", true},
    {"00.checkisnoiseon", false},
    {"12.uvnoise", false},
    {"11.uvmirroring", false},
    {"use_mesh", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1178 = {{"06.map","02.map_e","01.map_a","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,27> WARLORD_NATIVE_PARAMETERS_1178 = {{
    {"selectioncolor", 8u, 0u, true},
    {"93.emissiion_color", 6u, 0u, true},
    {"meshemitterdynamicparameter", 7u, 0u, true},
    {"21.uvscale.x", 3u, 1u, false},
    {"22.uvscale.y", 3u, 2u, false},
    {"time", 5u, 3u, false},
    {"05.map_e_panning_x", 1u, 3u, false},
    {"03.map_e_uvscale_r", 0u, 2u, false},
    {"04.map_e_uvscale_g", 1u, 0u, false},
    {"09.map_d_panning_x", 2u, 3u, false},
    {"07.map_d_uvscale_r", 2u, 1u, false},
    {"08.map_d_uvscale_g", 2u, 2u, false},
    {"10.map_d_panning_y", 3u, 0u, false},
    {"05.distort_str", 1u, 1u, false},
    {"06.map_e_panning_y", 2u, 0u, false},
    {"91.desaturation", 5u, 1u, false},
    {"92.emissiion_power", 5u, 2u, false},
    {"04.map_a_panning_x", 0u, 3u, false},
    {"02.map_a_uvscale_r", 0u, 0u, false},
    {"03.map_a_uvscale_g", 0u, 1u, false},
    {"05.map_a_panning_y", 1u, 2u, false},
    {"29.cmap.direct", 3u, 3u, false},
    {"30.cmap.time(rotrate)", 4u, 0u, false},
    {"36.str", 4u, 3u, false},
    {"37.power", 5u, 0u, false},
    {"32.fresnal_power", 4u, 1u, false},
    {"33.fresnal_str", 4u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_1178 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", false},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1179 = {{"04.map_anew","00.map_b"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,9> WARLORD_NATIVE_PARAMETERS_1179 = {{
    {"selectioncolor", 2u, 0u, true},
    {"05.map_alod", 0u, 0u, false},
    {"09.map_a_panning_x", 0u, 3u, false},
    {"07.map_a_uvscale_r", 0u, 1u, false},
    {"08.map_a_uvscale_g", 0u, 2u, false},
    {"10.map_a_panning_y", 1u, 0u, false},
    {"11.desaturation", 1u, 1u, false},
    {"11.map_a_str", 1u, 2u, false},
    {"12.map_a_power", 1u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1179 = {{
    {"00.usedistortion", false},
    {"20.use_centrehole", false},
    {"00.usedepthbiasalpha", false},
    {"30.usefresnal", false},
    {"10.cameradistance", false},
    {"01.mapch.r", true},
    {"02.mapch.g", true},
    {"03.mapch.b", true},
    {"00.use_uvdistort", false},
    {"01.use_uvdistort_all", false},
    {"checkismeshemitvc", false},
    {"00.use_rgbinalphamap", true},
    {"06.use((r+g+b)*0.33)", true},
    {"15.use_panbezier", false},
    {"01.usecoordinate_index1", false},
    {"30.use_additionalmap", false},
    {"useupno_alpha", false},
    {"40.use_fanshape", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1180 = {{"01.map_e","06.map","06.map_f"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,27> WARLORD_NATIVE_PARAMETERS_1180 = {{
    {"selectioncolor", 8u, 0u, true},
    {"16.emissiion_str", 7u, 0u, true},
    {"time", 6u, 0u, false},
    {"04.map_e_panning_x", 1u, 2u, false},
    {"02.map_e_uvscale_r", 1u, 0u, false},
    {"03.map_e_uvscale_g", 1u, 1u, false},
    {"09.map_d_panning_x", 3u, 1u, false},
    {"07.map_d_uvscale_r", 2u, 1u, false},
    {"08.map_d_uvscale_g", 2u, 3u, false},
    {"10.map_d_panning_y", 3u, 3u, false},
    {"05.distort_str", 1u, 3u, false},
    {"01.amount", 0u, 0u, false},
    {"11.curvature", 4u, 2u, false},
    {"05.map_e_panning_y", 2u, 0u, false},
    {"09.map_f_panning_x", 3u, 2u, false},
    {"07.map_f_uvscale_r", 2u, 2u, false},
    {"08.map_f_uvscale_g", 3u, 0u, false},
    {"10.map_f_panning_y", 4u, 0u, false},
    {"12.desaturation", 5u, 0u, false},
    {"15.emissiion_power", 5u, 1u, false},
    {"21.noise_str", 5u, 2u, false},
    {"11.centermask_str", 4u, 1u, false},
    {"12.centermask_power", 4u, 3u, false},
    {"01.circle_radius", 0u, 1u, false},
    {"02.circle_hardness", 0u, 3u, false},
    {"22.noise_power", 5u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,10> WARLORD_NATIVE_SWITCHES_1180 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"40.usefresnal", false},
    {"00.use_uvdistort", true},
    {"20.useaddnoise", true},
    {"00.usecentermask", true},
    {"00.use_uvdistort_alpha", false},
    {"20.usecolorvari", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.uselight", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1181 = {{"maintex","uv_noise_tex","dissolve_tex_01"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,34> WARLORD_NATIVE_PARAMETERS_1181 = {{
    {"selectioncolor", 9u, 0u, true},
    {"uvnoise_move_x", 7u, 2u, false},
    {"uvnoise_move_y", 7u, 3u, false},
    {"maintex_move_x", 3u, 3u, false},
    {"maintex_move_y", 4u, 0u, false},
    {"maintex_rotator", 4u, 3u, false},
    {"dissolvetex_rotator", 1u, 3u, false},
    {"edge_color", 8u, 0u, true},
    {"maintex_panspeed_x", 4u, 1u, false},
    {"maintex_tile_x", 5u, 0u, false},
    {"maintex_tile_y", 5u, 1u, false},
    {"maintex_panspeed_y", 4u, 2u, false},
    {"maintex_dynamicpan_x_velue", 3u, 1u, false},
    {"maintex_dynamicpan_y_velue", 3u, 2u, false},
    {"uv_noisetex_pan_x", 6u, 2u, false},
    {"uv_noisetex_tile_x", 7u, 0u, false},
    {"uv_noisetex_tile_y", 7u, 1u, false},
    {"uv_noisetex_pan_y", 6u, 3u, false},
    {"uv_noise_velue", 6u, 1u, false},
    {"emissive_core_power", 2u, 2u, false},
    {"emissive_core_strength", 2u, 3u, false},
    {"emissive_base", 2u, 1u, false},
    {"disslovetex_01_panspeed_x", 0u, 0u, false},
    {"disslovetex_01_tile_x", 0u, 2u, false},
    {"disslovetex_01_tile_y", 0u, 3u, false},
    {"disslovetex_01_panspeed_y", 0u, 1u, false},
    {"dissolvetex_move_x", 1u, 1u, false},
    {"dissolvetex_move_y", 1u, 2u, false},
    {"dissolve_hardness", 1u, 0u, false},
    {"edge_thin", 2u, 0u, false},
    {"spheremask_strength", 5u, 2u, false},
    {"spheremask_strength_max", 5u, 3u, false},
    {"spheremask_strength_min", 6u, 0u, false},
    {"maintex_alpha_strength", 3u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,20> WARLORD_NATIVE_SWITCHES_1181 = {{
    {"use_dissolve", true},
    {"use_meshtype", false},
    {"use_uvnoise", true},
    {"achannel_use_uvnoisedynamicpan", false},
    {"use_uv_noise_02", false},
    {"use_uv_noisetex_02", true},
    {"uv_noisetype", true},
    {"maintex_type_polar", true},
    {"maintex_pan_xy", false},
    {"use_dynamic_spheryxy", false},
    {"uv_sphery_y", false},
    {"uv_sphery_x", false},
    {"use_alpha_fresnel", false},
    {"use_dissolvetex_02", false},
    {"use_dissolvetex_noise", false},
    {"use_dissolvetex_noise_type", true},
    {"use_dissolvetex_02_mixtype", true},
    {"use_sphere_alpha", true},
    {"dissloveoutline_multyply_alpha", false},
    {"use_emissivetex02", false},
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1182 = {{"noise_tex","diff_tex","native_texture_2","opacity_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,26> WARLORD_NATIVE_PARAMETERS_1182 = {{
    {"selectioncolor", 9u, 0u, true},
    {"diff_high_color", 6u, 0u, true},
    {"diff_low_color", 7u, 0u, true},
    {"diff_u_tile", 1u, 1u, false},
    {"diff_v_tile", 1u, 2u, false},
    {"noise_u_tile", 2u, 2u, false},
    {"noise_v_tile", 3u, 0u, false},
    {"noise_u_pan", 2u, 1u, false},
    {"noise_v_pan", 2u, 3u, false},
    {"meshemitterdynamicparameter", 8u, 0u, true},
    {"wave_pan_speed", 4u, 3u, false},
    {"diff_rotation", 0u, 2u, false},
    {"opacity_u_tile", 4u, 0u, false},
    {"opacity_v_tile", 4u, 1u, false},
    {"opacity_rotation", 3u, 1u, false},
    {"noise_str", 2u, 0u, false},
    {"diff_u_center", 1u, 0u, false},
    {"wave_tile", 5u, 1u, false},
    {"wave_str", 5u, 0u, false},
    {"wave_noise_str", 4u, 2u, false},
    {"diff_desturation", 0u, 0u, false},
    {"diff_pow", 0u, 1u, false},
    {"diff_str", 0u, 3u, false},
    {"opacity_u_center", 3u, 3u, false},
    {"opacity_str", 3u, 2u, false},
    {"distortion_str", 1u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,1> WARLORD_NATIVE_SWITCHES_1182 = {{
    {"use_cameravec", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1183 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1183 = {{
    {"selectioncolor", 2u, 0u, true},
    {"uv_scale", 1u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,14> WARLORD_NATIVE_SWITCHES_1183 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_lamp_alpha", false},
    {"use_dissolve", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"non_alpha_chanel", false},
    {"use_uv_noise_tex", false},
    {"use_uv_noise_tex_02", false},
    {"use_fresnel_alpha", false},
    {"use_edge_glow", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1184 = {{"uv_noise_tex","native_texture_1","maintex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,30> WARLORD_NATIVE_PARAMETERS_1184 = {{
    {"selectioncolor", 8u, 0u, true},
    {"meshemitterdynamicparameter", 7u, 0u, true},
    {"maintex_rotator", 4u, 2u, false},
    {"maintex_move_u", 3u, 2u, false},
    {"maintex_move_v", 3u, 3u, false},
    {"maintex_pan_u_time", 4u, 0u, false},
    {"main_panspeed_u", 2u, 0u, false},
    {"maintex_texcoord_u", 4u, 3u, false},
    {"maintex_texcoord_v", 5u, 0u, false},
    {"main_panspeed_v", 2u, 1u, false},
    {"uv_noise_pan_u", 5u, 1u, false},
    {"uv_noise_panspeed_u", 5u, 3u, false},
    {"uv_noise_texcoord_u", 6u, 1u, false},
    {"uv_noise_texcoord_v", 6u, 2u, false},
    {"uv_noise_panspeed_v", 6u, 0u, false},
    {"uv_noise_pan_v", 5u, 2u, false},
    {"uv_noise_velue", 6u, 3u, false},
    {"maintex_pan_v_time", 4u, 1u, false},
    {"maintex_desaturation", 3u, 1u, false},
    {"main_tex_power", 2u, 3u, false},
    {"main_tex_power_multiply", 3u, 0u, false},
    {"main_tex_background_velue", 2u, 2u, false},
    {"dissolve_pan_u_time", 0u, 3u, false},
    {"dissolve_pan_u_speed", 0u, 2u, false},
    {"dissolve_texcoord_u", 1u, 2u, false},
    {"dissolve_texcoord_v", 1u, 3u, false},
    {"dissolve_pan_v_speed", 1u, 0u, false},
    {"dissolve_pan_v_time", 1u, 1u, false},
    {"disslove_hardness", 0u, 1u, false},
    {"alpha_strength", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,13> WARLORD_NATIVE_SWITCHES_1184 = {{
    {"usemesh", true},
    {"use_camera_vector_fresnel", false},
    {"use_distortion", false},
    {"use_outfalloff", false},
    {"outalpha_falloff_v", true},
    {"outalpha_falloff_u", true},
    {"use_alpha_channal", false},
    {"b_channal", false},
    {"g_channal", false},
    {"r_channal", false},
    {"use_depthbias", false},
    {"use_reflction", false},
    {"uv_noise_on", true},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1185 = {{"flowtex","diff_tex1","diff_tex2","mask_tex","opacity_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,25> WARLORD_NATIVE_PARAMETERS_1185 = {{
    {"selectioncolor", 7u, 0u, true},
    {"flow_tile_u", 3u, 3u, false},
    {"flow_tile_v", 4u, 0u, false},
    {"flow_pan_u", 3u, 1u, false},
    {"flow_pan_v", 3u, 2u, false},
    {"diff1_pan_u", 0u, 0u, false},
    {"diff1_pan_v", 0u, 1u, false},
    {"diff2_pan_u", 1u, 0u, false},
    {"diff2_pan_v", 1u, 1u, false},
    {"diff_backcolor", 6u, 0u, true},
    {"opacity_tile_u", 5u, 1u, false},
    {"opacity_tile_v", 5u, 2u, false},
    {"opacity_rot", 4u, 3u, false},
    {"diff_v_power", 2u, 3u, false},
    {"diff1_tile_v", 0u, 3u, false},
    {"diff1_tile_u", 0u, 2u, false},
    {"flow_bias", 3u, 0u, false},
    {"diff2_tile_v", 1u, 3u, false},
    {"diff2_tile_u", 1u, 2u, false},
    {"diff_des", 2u, 0u, false},
    {"diff_pow", 2u, 1u, false},
    {"diff_str", 2u, 2u, false},
    {"opacity_distort_str", 4u, 1u, false},
    {"opacity_pow", 4u, 2u, false},
    {"opacity_str", 5u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_1185 = {{
    {"use_colormap", false},
    {"diff_pan_use_u", false},
    {"use_distortion", false},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1186 = {{"flowtex","diff_tex1","diff_tex2","mask_tex","opacity_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,24> WARLORD_NATIVE_PARAMETERS_1186 = {{
    {"selectioncolor", 7u, 0u, true},
    {"diff1_tile_u", 0u, 2u, false},
    {"diff1_tile_v", 0u, 3u, false},
    {"flow_tile_u", 3u, 2u, false},
    {"flow_tile_v", 3u, 3u, false},
    {"flow_pan_u", 3u, 0u, false},
    {"flow_pan_v", 3u, 1u, false},
    {"diff1_pan_u", 0u, 0u, false},
    {"diff1_pan_v", 0u, 1u, false},
    {"diff2_tile_u", 1u, 2u, false},
    {"diff2_tile_v", 1u, 3u, false},
    {"diff2_pan_u", 1u, 0u, false},
    {"diff2_pan_v", 1u, 1u, false},
    {"diff_backcolor", 6u, 0u, true},
    {"opacity_tile_u", 5u, 0u, false},
    {"opacity_tile_v", 5u, 1u, false},
    {"opacity_rot", 4u, 2u, false},
    {"flow_bias", 2u, 3u, false},
    {"diff_des", 2u, 0u, false},
    {"diff_pow", 2u, 1u, false},
    {"diff_str", 2u, 2u, false},
    {"opacity_distort_str", 4u, 0u, false},
    {"opacity_pow", 4u, 1u, false},
    {"opacity_str", 4u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_1186 = {{
    {"diff_pan_use_u", false},
    {"use_distortion", false},
    {"use_colormap", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1187 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,1> WARLORD_NATIVE_PARAMETERS_1187 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1187 = {{
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1188 = {{"flowtex","diff_tex1","diff_tex2","opacity_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,26> WARLORD_NATIVE_PARAMETERS_1188 = {{
    {"selectioncolor", 7u, 0u, true},
    {"flow_tile_u", 3u, 3u, false},
    {"flow_tile_v", 4u, 0u, false},
    {"flow_pan_u", 3u, 1u, false},
    {"flow_pan_v", 3u, 2u, false},
    {"meshemitterdynamicparameter", 6u, 0u, true},
    {"diff1_pan_u", 0u, 1u, false},
    {"diff1_pan_v", 0u, 2u, false},
    {"diff2_pan_u", 1u, 1u, false},
    {"diff2_pan_v", 1u, 2u, false},
    {"opacity_tile_u", 5u, 2u, false},
    {"opacity_tile_v", 5u, 3u, false},
    {"opacity_rot", 5u, 0u, false},
    {"diff1_tile_v", 1u, 0u, false},
    {"diff1_tile_u", 0u, 3u, false},
    {"flow_bias", 3u, 0u, false},
    {"diff2_tile_v", 2u, 0u, false},
    {"diff2_tile_u", 1u, 3u, false},
    {"diff_des", 2u, 1u, false},
    {"diff_pow", 2u, 2u, false},
    {"diff_str", 2u, 3u, false},
    {"cameravec_pow", 0u, 0u, false},
    {"opacity_distort_str", 4u, 2u, false},
    {"gra_pow", 4u, 1u, false},
    {"opacity_pow", 4u, 3u, false},
    {"opacity_str", 5u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,5> WARLORD_NATIVE_SWITCHES_1188 = {{
    {"use_gra_r_channel", false},
    {"use_colormap", false},
    {"diff_pan_use_u", false},
    {"use_cam_vector", true},
    {"use_distortion", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1189 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_1189 = {{
    {"selectioncolor", 1u, 0u, true},
    {"uv_scale", 0u, 3u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 1u, false},
    {"emissive_power", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,17> WARLORD_NATIVE_SWITCHES_1189 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"use_distortion_floor", false},
    {"non_alpha_chanel", true},
    {"use_dynparam_panning", false},
    {"use_axisy", true},
    {"use_uv_noise_tex_02", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1190 = {{"lensflaretexture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_1190 = {{
    {"selectioncolor", 1u, 0u, true},
    {"select texture(0 or 0.5)", 0u, 2u, false},
    {"desaturation", 0u, 1u, false},
    {"depthbaisalpha", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1190 = {{
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1191 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,6> WARLORD_NATIVE_PARAMETERS_1191 = {{
    {"selectioncolor", 2u, 0u, true},
    {"centerglow_power", 0u, 0u, false},
    {"centerglow_str", 0u, 1u, false},
    {"glow_power", 0u, 2u, false},
    {"str", 1u, 0u, false},
    {"power", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1191 = {{
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1192 = {{"flowtex","diff_tex1","diff_tex2","mask_tex","opacity_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,24> WARLORD_NATIVE_PARAMETERS_1192 = {{
    {"selectioncolor", 7u, 0u, true},
    {"diff1_tile_u", 0u, 2u, false},
    {"diff1_tile_v", 0u, 3u, false},
    {"flow_tile_u", 3u, 2u, false},
    {"flow_tile_v", 3u, 3u, false},
    {"flow_pan_u", 3u, 0u, false},
    {"flow_pan_v", 3u, 1u, false},
    {"diff1_pan_u", 0u, 0u, false},
    {"diff1_pan_v", 0u, 1u, false},
    {"diff2_tile_u", 1u, 2u, false},
    {"diff2_tile_v", 1u, 3u, false},
    {"diff2_pan_u", 1u, 0u, false},
    {"diff2_pan_v", 1u, 1u, false},
    {"diff_backcolor", 6u, 0u, true},
    {"opacity_tile_u", 5u, 0u, false},
    {"opacity_tile_v", 5u, 1u, false},
    {"opacity_rot", 4u, 2u, false},
    {"flow_bias", 2u, 3u, false},
    {"diff_des", 2u, 0u, false},
    {"diff_pow", 2u, 1u, false},
    {"diff_str", 2u, 2u, false},
    {"opacity_distort_str", 4u, 0u, false},
    {"opacity_pow", 4u, 1u, false},
    {"opacity_str", 4u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_1192 = {{
    {"diff_pan_use_u", false},
    {"use_distortion", false},
    {"use_colormap", false},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_1193 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_1193 = {{
    {"selectioncolor", 1u, 0u, true},
    {"power", 0u, 2u, false},
    {"str", 0u, 3u, false},
    {"01.range", 0u, 0u, false},
    {"02.power", 0u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,6> WARLORD_NATIVE_SWITCHES_1193 = {{
    {"checkisdepthbiasalpha", false},
    {"00.checkisnoiseon", false},
    {"11.uvmirroring", false},
    {"12.uvnoise", false},
    {"--usefakedepthbiasalpha", true},
    {"use_mesh", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_1194 = {{"native_texture_0","native_texture_1"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,20> WARLORD_NATIVE_PARAMETERS_1194 = {{
    {"selectioncolor", 5u, 0u, true},
    {"mask_u_start_coord", 4u, 1u, false},
    {"mask_u_end_coord", 3u, 2u, false},
    {"colormap_uspeed", 2u, 1u, false},
    {"colormap_ucoord", 2u, 0u, false},
    {"colormap_vcoord", 2u, 2u, false},
    {"colormap_vspeed", 2u, 3u, false},
    {"colormap_pow", 1u, 2u, false},
    {"colormap_str", 1u, 3u, false},
    {"addmap_uspeed", 0u, 3u, false},
    {"addmap_ucoord", 0u, 2u, false},
    {"addmap_vcoord", 1u, 0u, false},
    {"addmap_vspeed", 1u, 1u, false},
    {"addmap_pow", 0u, 0u, false},
    {"addmap_str", 0u, 1u, false},
    {"core_pow", 3u, 0u, false},
    {"mask_u_start_power", 4u, 2u, false},
    {"mask_u_start_bias", 4u, 0u, false},
    {"mask_u_end_power", 3u, 3u, false},
    {"mask_u_end_bias", 3u, 1u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,1> WARLORD_NATIVE_SWITCHES_1194 = {{
    {"sw_opacity_stert_end", true},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1195 = {{"lensflaretexture"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,4> WARLORD_NATIVE_PARAMETERS_1195 = {{
    {"selectioncolor", 1u, 0u, true},
    {"select texture(0 or 0.5)", 0u, 2u, false},
    {"desaturation", 0u, 1u, false},
    {"depthbaisalpha", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_1195 = {{
}};

inline constexpr std::array<std::string_view,4> WARLORD_NATIVE_TEXTURES_1196 = {{"add_emissive_tex","twinkle_tex","twinkle_tex_01","mask_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,8> WARLORD_NATIVE_PARAMETERS_1196 = {{
    {"selectioncolor", 3u, 0u, true},
    {"emissive_color&intensity", 2u, 0u, true},
    {"emissive_paning", 0u, 1u, false},
    {"emissive_tiling", 0u, 2u, false},
    {"add_emissive_intensity", 0u, 0u, false},
    {"twinkle_paning", 1u, 0u, false},
    {"twinkle_tiling", 1u, 1u, false},
    {"twinkle_intensity", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,3> WARLORD_NATIVE_SWITCHES_1196 = {{
    {"use_mask_r_chanel", false},
    {"use_emissive_r_chanel", false},
    {"use_meshtype", false},
}};

inline constexpr std::array<std::string_view,1> WARLORD_NATIVE_TEXTURES_1197 = {{"emissive_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_1197 = {{
    {"selectioncolor", 1u, 0u, true},
    {"uv_scale", 0u, 3u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 1u, false},
    {"emissive_power", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,18> WARLORD_NATIVE_SWITCHES_1197 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", false},
    {"use_distortion", false},
    {"use_distortion_floor", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", false},
    {"use_uv_noise_tex", false},
    {"use_fresnel_alpha", false},
    {"use_camera_alpha", false},
    {"use_edge_glow", false},
    {"non_alpha_chanel", false},
    {"use_axisy", true},
    {"use_dynparam_panning", false},
    {"use_uv_noise_tex_02", false},
    {"use_rotation_angle", false},
}};

inline constexpr std::array<std::string_view,5> WARLORD_NATIVE_TEXTURES_1198 = {{"uv_noise_texture","alpha_texture1_mask","alpha_texture2","emissive_tex_01","emissive_tex_02"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,31> WARLORD_NATIVE_PARAMETERS_1198 = {{
    {"selectioncolor", 8u, 0u, true},
    {"mainalpha_move_u", 5u, 0u, false},
    {"mainalpha_move_v", 5u, 1u, false},
    {"mainalpha_rotator", 5u, 2u, false},
    {"emissive_tex_01_move_u", 2u, 0u, false},
    {"emissive_tex_01_move_v", 2u, 1u, false},
    {"emissive_tex_02_move_u", 3u, 2u, false},
    {"emissive_tex_02_move_v", 3u, 3u, false},
    {"mainalpha_tile_u", 5u, 3u, false},
    {"mainalpha_tile_v", 6u, 0u, false},
    {"uv_noise_panspeed_x", 6u, 1u, false},
    {"uv_noise_tile_u", 6u, 3u, false},
    {"uv_noise_tile_v", 7u, 0u, false},
    {"uv_noise_panspeed_y", 6u, 2u, false},
    {"uv_noise_velue", 7u, 1u, false},
    {"emissive_background_power", 0u, 1u, false},
    {"emissive_background_strength", 0u, 2u, false},
    {"emissive_tex_01_panspeed_x", 2u, 2u, false},
    {"emissive_tex_01_tile_u", 3u, 0u, false},
    {"emissive_tex_01_tile_v", 3u, 1u, false},
    {"emissive_noise_velue", 1u, 1u, false},
    {"emissive_tex_01_panspeed_y", 2u, 3u, false},
    {"emissive_tex_02_panspeed_x", 4u, 0u, false},
    {"emissive_tex_02_tile_u", 4u, 2u, false},
    {"emissive_tex_02_tile_v", 4u, 3u, false},
    {"emissive_tex_02_panspeed_y", 4u, 1u, false},
    {"emissive_power", 1u, 2u, false},
    {"emissive_core_power", 0u, 3u, false},
    {"emissive_core_strength", 1u, 0u, false},
    {"emissive_strength", 1u, 3u, false},
    {"alpha_tex_power", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,6> WARLORD_NATIVE_SWITCHES_1198 = {{
    {"use_distortion", false},
    {"use_emissive_tex_02", true},
    {"emissive_tex_(01+02)/2", false},
    {"use_alphatex_type_pola", false},
    {"use_uv_noisetexture", true},
    {"use_emissive_tex02_to_emissive01_uvnoise", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_1199 = {{"main_tex","uv_noise_texture","colormap_tex"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,22> WARLORD_NATIVE_PARAMETERS_1199 = {{
    {"selectioncolor", 6u, 0u, true},
    {"colormap_color", 5u, 0u, true},
    {"main_utile", 3u, 0u, false},
    {"twist_str", 3u, 3u, false},
    {"main_curve", 2u, 1u, false},
    {"main_vtile", 3u, 1u, false},
    {"main_panner", 2u, 2u, false},
    {"colormap_utile", 1u, 0u, false},
    {"colormap_vtile", 1u, 1u, false},
    {"colormap_panner", 0u, 1u, false},
    {"uvnoise_utile", 4u, 2u, false},
    {"uvnoise_vtile", 4u, 3u, false},
    {"uvnoise_panner", 4u, 0u, false},
    {"uvnoise_str", 4u, 1u, false},
    {"colormap_power", 0u, 2u, false},
    {"colormap_str", 0u, 3u, false},
    {"colormap_desatuation", 0u, 0u, false},
    {"main_power", 2u, 3u, false},
    {"out_opa_range", 3u, 2u, false},
    {"in_opa_range", 1u, 2u, false},
    {"in_opa_str", 1u, 3u, false},
    {"main_alpha", 2u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,4> WARLORD_NATIVE_SWITCHES_1199 = {{
    {"use_colormap", true},
    {"use_uvnoise", true},
    {"use_distortion", false},
    {"use_depth_alpha", false},
}};

inline constexpr std::array<std::string_view,2> WARLORD_NATIVE_TEXTURES_2000 = {{"uvnoisemap","flowmap"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,5> WARLORD_NATIVE_PARAMETERS_2000 = {{
    {"selectioncolor", 1u, 0u, true},
    {"panspeed", 0u, 1u, false},
    {"flowmapsize", 0u, 0u, false},
    {"uvnoisemap_size", 0u, 2u, false},
    {"uvnoisestrength", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_2000 = {{
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_2001 = {{"06.map","02.map_e","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,24> WARLORD_NATIVE_PARAMETERS_2001 = {{
    {"selectioncolor", 8u, 0u, true},
    {"93.emissiion_color", 6u, 0u, true},
    {"meshemitterdynamicparameter", 7u, 0u, true},
    {"21.uvscale.x", 2u, 2u, false},
    {"22.uvscale.y", 2u, 3u, false},
    {"time", 5u, 0u, false},
    {"05.map_e_panning_x", 1u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 1u, false},
    {"04.map_e_uvscale_g", 0u, 2u, false},
    {"09.map_d_panning_x", 2u, 0u, false},
    {"07.map_d_uvscale_r", 1u, 2u, false},
    {"08.map_d_uvscale_g", 1u, 3u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 0u, 3u, false},
    {"06.map_e_panning_y", 1u, 1u, false},
    {"91.desaturation", 4u, 2u, false},
    {"92.emissiion_power", 4u, 3u, false},
    {"29.cmap.direct", 3u, 0u, false},
    {"30.cmap.time(rotrate)", 3u, 1u, false},
    {"36.str", 4u, 0u, false},
    {"37.power", 4u, 1u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"32.fresnal_power", 3u, 2u, false},
    {"33.fresnal_str", 3u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_2001 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", false},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_2002 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,9> WARLORD_NATIVE_PARAMETERS_2002 = {{
    {"selectioncolor", 2u, 0u, true},
    {"cricle_rotation", 0u, 0u, false},
    {"uv_scale_x", 1u, 2u, false},
    {"uv_scale_y", 1u, 3u, false},
    {"uv_position_x", 1u, 0u, false},
    {"uv_position_y", 1u, 1u, false},
    {"mask_hardness", 0u, 1u, false},
    {"mask_power", 0u, 3u, false},
    {"mask_intensity", 0u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,1> WARLORD_NATIVE_SWITCHES_2002 = {{
    {"use_inverse", false},
}};

inline constexpr std::array<std::string_view,3> WARLORD_NATIVE_TEXTURES_2003 = {{"06.map","02.map_e","21.map_c"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,24> WARLORD_NATIVE_PARAMETERS_2003 = {{
    {"selectioncolor", 8u, 0u, true},
    {"93.emissiion_color", 6u, 0u, true},
    {"meshemitterdynamicparameter", 7u, 0u, true},
    {"21.uvscale.x", 2u, 2u, false},
    {"22.uvscale.y", 2u, 3u, false},
    {"time", 5u, 0u, false},
    {"05.map_e_panning_x", 1u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 0u, false},
    {"04.map_e_uvscale_g", 0u, 1u, false},
    {"09.map_d_panning_x", 2u, 0u, false},
    {"07.map_d_uvscale_r", 1u, 2u, false},
    {"08.map_d_uvscale_g", 1u, 3u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 0u, 2u, false},
    {"06.map_e_panning_y", 1u, 1u, false},
    {"91.desaturation", 4u, 2u, false},
    {"92.emissiion_power", 4u, 3u, false},
    {"29.cmap.direct", 3u, 0u, false},
    {"30.cmap.time(rotrate)", 3u, 1u, false},
    {"36.str", 4u, 0u, false},
    {"37.power", 4u, 1u, false},
    {"32.fresnal_power", 3u, 2u, false},
    {"33.fresnal_str", 3u, 3u, false},
    {"05.distortion_str", 0u, 3u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,26> WARLORD_NATIVE_SWITCHES_2003 = {{
    {"00.usedistortion", true},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", false},
    {"30.usefresnal", true},
    {"00.checkisdepthbiasalpha", false},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", true},
    {"10.use_mapf", false},
    {"11.map_e + map_f", false},
    {"00.usespecullar", false},
    {"20.usefresspec", false},
    {"40.use_clampmap", false},
    {"48.use_yclamp", false},
    {"41.map_e + map_f", false},
    {"60.twist", false},
    {"60.usedissolveemission", false},
    {"31.fresnal.invert", false},
    {"33.mapch.b", true},
    {"34.mapch.a", true},
}};

inline constexpr std::array<std::string_view,0> WARLORD_NATIVE_TEXTURES_2004 = {{}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,2> WARLORD_NATIVE_PARAMETERS_2004 = {{
    {"selectioncolor", 1u, 0u, true},
    {"01.inactivealpha", 0u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,0> WARLORD_NATIVE_SWITCHES_2004 = {{
}};


inline constexpr std::array<std::string_view,6> WARLORD_NATIVE_TEXTURES_1122 = {{"texture_normal","texture_basecolor","texture_orm","texture_color_fx_skin","texture_dead","native_texture_5"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,30> WARLORD_NATIVE_PARAMETERS_1122 = {{
    {"selectioncolor", 14u, 0u, true},
    {"basecolor_color", 4u, 0u, true},
    {"diffusecolor_a", 6u, 0u, true},
    {"diffusecolor_b", 7u, 0u, true},
    {"state", 16u, 0u, true},
    {"fx_color_intensity_buffsettool", 9u, 0u, true},
    {"fx_color_intensity_actiontool", 8u, 0u, true},
    {"transcolor", 17u, 0u, true},
    {"buffcolor", 5u, 0u, true},
    {"hit_color", 11u, 0u, true},
    {"meshemitterdynamicparameter", 12u, 0u, true},
    {"fx_dead_edgecolor", 10u, 0u, true},
    {"occlusion_color", 13u, 0u, true},
    {"ssstintcolor", 15u, 0u, true},
    {"normaltex_intensity", 1u, 1u, false},
    {"metallic_power", 1u, 0u, false},
    {"pbr_add_basecolor_to_emissive", 2u, 1u, false},
    {"fx_color_desaturation_actiontool", 0u, 2u, false},
    {"fx_color_desaturation_buffsettool", 0u, 3u, false},
    {"transcolor_rimlight ", 3u, 3u, false},
    {"trans_rim_inradius", 3u, 2u, false},
    {"trans_rim_hard", 3u, 1u, false},
    {"dead_texture_tiling", 0u, 1u, false},
    {"dead", 0u, 0u, false},
    {"occlusion_power", 1u, 3u, false},
    {"occlusion_brightness", 1u, 2u, false},
    {"occlusionbasecolor_blend_intensity", 2u, 0u, false},
    {"pbr_specular", 2u, 2u, false},
    {"roughness_power", 2u, 3u, false},
    {"ssslocalthickness", 3u, 0u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,34> WARLORD_NATIVE_SWITCHES_1122 = {{
    {"1.use_dyeing", false},
    {"2.use_wp_dyeing", false},
    {"1.use_fx_dead_worldoffset", false},
    {"is_npc", false},
    {"1.use_parts_highlight", false},
    {"1.use_hitcolordirection", false},
    {"1.use_state_material", true},
    {"1.is_playercharacter", true},
    {"1.use_color_variation", true},
    {"2.use_pattern_dyeing", false},
    {"1.use_transition", false},
    {"1.use_constant_fx", false},
    {"1.use_fx_panning", false},
    {"1.use_interaction", false},
    {"1.use_emissive", false},
    {"1.use_flicker", false},
    {"1.use_emissive_into_dyeing", false},
    {"1.use_interaction_dyeing", false},
    {"1.use_interaction_rgba_color", false},
    {"1.use_fx_panning_into_dyeing", false},
    {"1.use_uvchannel", false},
    {"1.use_parts_visibleflag", false},
    {"1.use_mask_variation", false},
    {"is_blend_translucent", false},
    {"is_blend_masked", true},
    {"1.use_fx_dead", true},
    {"1.use_opacityfalloff", false},
    {"1.use_fx_dead_edgeglow", true},
    {"1.use_rimlight", false},
    {"1.use_pbr_specular", true},
    {"1.use_fx_dead_uv2", false},
    {"1.use_auto_pbr_oc", false},
    {"1.use_rimlight_rotate", false},
    {"1.use_ssslt_texture", false},
}};

inline constexpr std::array<std::string_view,8> WARLORD_NATIVE_TEXTURES_1123 = {{"texture_normal","texture_basecolor","texture_orm","texture_color_fx_skin","texture_emissive","texture_fx_panning","texture_dead","native_texture_7"}};
inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,35> WARLORD_NATIVE_PARAMETERS_1123 = {{
    {"selectioncolor", 18u, 0u, true},
    {"basecolor_color", 5u, 0u, true},
    {"diffusecolor_a", 7u, 0u, true},
    {"diffusecolor_b", 8u, 0u, true},
    {"state", 20u, 0u, true},
    {"emissive_color", 9u, 0u, true},
    {"fx_panningspeed_tiling", 14u, 0u, true},
    {"fx_panning_color", 13u, 0u, true},
    {"fx_color_intensity_buffsettool", 11u, 0u, true},
    {"fx_color_intensity_actiontool", 10u, 0u, true},
    {"transcolor", 21u, 0u, true},
    {"buffcolor", 6u, 0u, true},
    {"hit_color", 15u, 0u, true},
    {"meshemitterdynamicparameter", 16u, 0u, true},
    {"fx_dead_edgecolor", 12u, 0u, true},
    {"occlusion_color", 17u, 0u, true},
    {"ssstintcolor", 19u, 0u, true},
    {"normaltex_intensity", 2u, 0u, false},
    {"metallic_power", 1u, 3u, false},
    {"pbr_add_basecolor_to_emissive", 3u, 0u, false},
    {"emissive_intensity", 0u, 2u, false},
    {"fx_distortion_intensity", 1u, 1u, false},
    {"fx_panning_intensity", 1u, 2u, false},
    {"transcolor_rimlight ", 4u, 1u, false},
    {"trans_rim_inradius", 4u, 0u, false},
    {"trans_rim_hard", 3u, 3u, false},
    {"fx_color_desaturation_actiontool", 0u, 3u, false},
    {"fx_color_desaturation_buffsettool", 1u, 0u, false},
    {"dead_texture_tiling", 0u, 1u, false},
    {"dead", 0u, 0u, false},
    {"occlusion_power", 2u, 2u, false},
    {"occlusion_brightness", 2u, 1u, false},
    {"occlusionbasecolor_blend_intensity", 2u, 3u, false},
    {"roughness_power", 3u, 1u, false},
    {"ssslocalthickness", 3u, 2u, false},
}};
inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,34> WARLORD_NATIVE_SWITCHES_1123 = {{
    {"1.use_dyeing", false},
    {"2.use_wp_dyeing", false},
    {"1.use_fx_dead_worldoffset", false},
    {"is_npc", false},
    {"1.use_parts_highlight", false},
    {"1.use_hitcolordirection", false},
    {"1.use_state_material", true},
    {"1.is_playercharacter", true},
    {"1.use_color_variation", true},
    {"2.use_pattern_dyeing", false},
    {"1.use_transition", false},
    {"1.use_constant_fx", false},
    {"1.use_fx_panning", true},
    {"1.use_interaction", false},
    {"1.use_emissive", true},
    {"1.use_flicker", false},
    {"1.use_emissive_into_dyeing", false},
    {"1.use_interaction_dyeing", false},
    {"1.use_interaction_rgba_color", false},
    {"1.use_fx_panning_into_dyeing", false},
    {"1.use_uvchannel", false},
    {"1.use_parts_visibleflag", false},
    {"1.use_mask_variation", false},
    {"is_blend_translucent", false},
    {"is_blend_masked", true},
    {"1.use_fx_dead", true},
    {"1.use_opacityfalloff", false},
    {"1.use_fx_dead_edgeglow", true},
    {"1.use_rimlight", false},
    {"1.use_pbr_specular", false},
    {"1.use_fx_dead_uv2", false},
    {"1.use_auto_pbr_oc", false},
    {"1.use_rimlight_rotate", false},
    {"1.use_ssslt_texture", false},
}};
inline constexpr std::array<WARLORD_NATIVE_PROGRAM_DESC,271> WARLORD_NATIVE_PROGRAMS = {{
    {1122u,"effect.ue3.warlord-1122-native.v1","fx_m_mi_w_00.mi.fx_w_wgl_gdd_pbr_02","efbasematerial_prologue.ch.realpbr.base.realpbr_wp_dead_msk","ue3.material.efbasematerial.prologue.ch.realpbr.base.realpbr.wp.dead.msk.a2f90b5232d3",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1122,WARLORD_NATIVE_PARAMETERS_1122,WARLORD_NATIVE_SWITCHES_1122},
    {1123u,"effect.ue3.warlord-1123-native.v1","fx_m_mi_w_00.mi.fx_w_wgl_gdd_pbr_01","efbasematerial_prologue.ch.realpbr.base.realpbr_wp_dead_msk","ue3.material.efbasematerial.prologue.ch.realpbr.base.realpbr.wp.dead.msk.a2f90b5232d3",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1123,WARLORD_NATIVE_PARAMETERS_1123,WARLORD_NATIVE_SWITCHES_1123},

    {400u,"effect.ue3.warlord-400-native.v1","fx_m_mi_05.fx_mi.fx_e_pa_cd_17_tr","fx_m_mi_05.fx_m.fx_e_pa_auradissolve_01","ue3.material.fx.m.mi.05.fx.m.fx.e.pa.auradissolve.01.969b1227abb3",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_400,WARLORD_NATIVE_PARAMETERS_400,WARLORD_NATIVE_SWITCHES_400},
    {401u,"effect.ue3.warlord-401-native.v1","fx_m_mi_05.fx_m.fx_c_pa_aura_02_tr","fx_m_mi_05.fx_m.fx_c_pa_aura_02_tr","ue3.material.fx.m.mi.05.fx.m.fx.c.pa.aura.02.tr.6b6be3c64e52",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_401,WARLORD_NATIVE_PARAMETERS_401,WARLORD_NATIVE_SWITCHES_401},
    {402u,"effect.ue3.warlord-402-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_ri_03_1_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_402,WARLORD_NATIVE_PARAMETERS_402,WARLORD_NATIVE_SWITCHES_402},
    {403u,"effect.ue3.warlord-403-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_ht_01_1_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_403,WARLORD_NATIVE_PARAMETERS_403,WARLORD_NATIVE_SWITCHES_403},
    {404u,"effect.ue3.warlord-404-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_ring_07_79_ad","fx_m_mi_03.fx_m.fx_d_pa_ring_07_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.ad.82e9116584b2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_404,WARLORD_NATIVE_PARAMETERS_404,WARLORD_NATIVE_SWITCHES_404},
    {405u,"effect.ue3.warlord-405-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_gl_02_4_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_405,WARLORD_NATIVE_PARAMETERS_405,WARLORD_NATIVE_SWITCHES_405},
    {406u,"effect.ue3.warlord-406-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_ht_16_1_ad","fx_mastermaterial.fx_mm.fx_mm_simple_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_406,WARLORD_NATIVE_PARAMETERS_406,WARLORD_NATIVE_SWITCHES_406},
    {407u,"effect.ue3.warlord-407-native.v1","fx_m_mi_01.fx_mi.fx_e_me_ri_03_1_ts_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_407,WARLORD_NATIVE_PARAMETERS_407,WARLORD_NATIVE_SWITCHES_407},
    {408u,"effect.ue3.warlord-408-native.v1","fx_m_mi_02.fx_mi.fx_e_pa_ht_18_4_tr","fx_m_mi_02.fx_m.fx_f_pa_shine_01_0_tr","ue3.material.fx.m.mi.02.fx.m.fx.f.pa.shine.01.0.tr.3d21bd1f3e79",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_408,WARLORD_NATIVE_PARAMETERS_408,WARLORD_NATIVE_SWITCHES_408},
    {409u,"effect.ue3.warlord-409-native.v1","fx_m_mi_01.fx_mi.fx_e_me_sy_10_1_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_409,WARLORD_NATIVE_PARAMETERS_409,WARLORD_NATIVE_SWITCHES_409},
    {410u,"effect.ue3.warlord-410-native.v1","fx_m_mi_01.fx_mi.fx_e_me_sy_10_2_tr","fx_mastermaterial.fx_mm.fx_mm_dissolve_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.dissolve.01.tr.a799c9636783",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_410,WARLORD_NATIVE_PARAMETERS_410,WARLORD_NATIVE_SWITCHES_410},
    {411u,"effect.ue3.warlord-411-native.v1","fx_m_mi_02.fx_mi.fx_j_pa_rampshape_01_2_ad","fx_m_mi_02.fx_m.fx_j_pa_rampshape_01_ad","ue3.material.fx.m.mi.02.fx.m.fx.j.pa.rampshape.01.ad.d03d08b4090e",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_411,WARLORD_NATIVE_PARAMETERS_411,WARLORD_NATIVE_SWITCHES_411},
    {412u,"effect.ue3.warlord-412-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_dist_03_1_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_412,WARLORD_NATIVE_PARAMETERS_412,WARLORD_NATIVE_SWITCHES_412},
    {413u,"effect.ue3.warlord-413-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_db_01_1_ad","fx_mastermaterial.fx_mm.fx_mm_simple_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_413,WARLORD_NATIVE_PARAMETERS_413,WARLORD_NATIVE_SWITCHES_413},
    {414u,"effect.ue3.warlord-414-native.v1","fx_m_mi_01.fx_m.fx_h_pa_tearsurface_01_tr","fx_m_mi_01.fx_m.fx_h_pa_tearsurface_01_tr","ue3.material.fx.m.mi.01.fx.m.fx.h.pa.tearsurface.01.tr.96a5fa2fe7fe",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_414,WARLORD_NATIVE_PARAMETERS_414,WARLORD_NATIVE_SWITCHES_414},
    {415u,"effect.ue3.warlord-415-native.v1","fx_post.fx_mi.fx_c_pa_zoomblur_01_tr","fx_post.fx_m.fx_c_po_zoomblur_01","ue3.material.fx.post.fx.m.fx.c.po.zoomblur.01.7068167af900",false,"screenPost",true,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_415,WARLORD_NATIVE_PARAMETERS_415,WARLORD_NATIVE_SWITCHES_415},
    {416u,"effect.ue3.warlord-416-native.v1","fx_m_mi_00.fx_mi.fx_b_pa_ht_01_2_ad","fx_mastermaterial.fx_mm.fx_mm_simple_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_416,WARLORD_NATIVE_PARAMETERS_416,WARLORD_NATIVE_SWITCHES_416},
    {417u,"effect.ue3.warlord-417-native.v1","fx_m_mi_d_00.fx_mi.fx_d_pa_glow_02_02_dt15_ad","fx_m_mi_d_00.fx_m.fx_d_pa_glow_02_ad","ue3.material.fx.m.mi.d.00.fx.m.fx.d.pa.glow.02.ad.3887cf5cc484",false,"sprite",false,true,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_417,WARLORD_NATIVE_PARAMETERS_417,WARLORD_NATIVE_SWITCHES_417},
    {418u,"effect.ue3.warlord-418-native.v1","fx_m_mi_d_00.fx_mi.fx_d_me_chain_01_101_ma","fx_m_mi_00.fx_m.fx_d_me_chain_01_ma","ue3.material.fx.m.mi.00.fx.m.fx.d.me.chain.01.ma.a8a92d2a6abc",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_418,WARLORD_NATIVE_PARAMETERS_418,WARLORD_NATIVE_SWITCHES_418},
    {419u,"effect.ue3.warlord-419-native.v1","fx_m_mi_d_00.fx_mi.fx_d_me_master_01_ph_113_msk","fx_m_mi_00.fx_m.fx_d_me_master_01_ph_msk","ue3.material.fx.m.mi.00.fx.m.fx.d.me.master.01.ph.msk.8230663740c0",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_419,WARLORD_NATIVE_PARAMETERS_419,WARLORD_NATIVE_SWITCHES_419},
    {420u,"effect.ue3.warlord-420-native.v1","fx_mastermaterial.fx_mi.fx_c_pa_dist_05_ad","fx_mastermaterial.fx_mm.fx_mm_distortion_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.distortion.01.ad.fd38a9b02e64",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_420,WARLORD_NATIVE_PARAMETERS_420,WARLORD_NATIVE_SWITCHES_420},
    {421u,"effect.ue3.warlord-421-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_atta_12_01_dt_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_421,WARLORD_NATIVE_PARAMETERS_421,WARLORD_NATIVE_SWITCHES_421},
    {422u,"effect.ue3.warlord-422-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_gl_01_9_ad","fx_mastermaterial.fx_mm.fx_mm_simple_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_422,WARLORD_NATIVE_PARAMETERS_422,WARLORD_NATIVE_SWITCHES_422},
    {423u,"effect.ue3.warlord-423-native.v1","fx_m_mi_00.fx_mi.fx_b_pa_gl_01_4_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_423,WARLORD_NATIVE_PARAMETERS_423,WARLORD_NATIVE_SWITCHES_423},
    {424u,"effect.ue3.warlord-424-native.v1","fx_m_mi_01.fx_mi.fx_j_pa_circlelenz_01_ad","fx_mastermaterial.fx_mm.fx_mm_light_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.ad.f431613b2bdf",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_424,WARLORD_NATIVE_PARAMETERS_424,WARLORD_NATIVE_SWITCHES_424},
    {425u,"effect.ue3.warlord-425-native.v1","fx_m_mi_01.fx_mi.fx_j_pa_circlelenz_02_ad","fx_mastermaterial.fx_mm.fx_mm_light_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.ad.f431613b2bdf",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_425,WARLORD_NATIVE_PARAMETERS_425,WARLORD_NATIVE_SWITCHES_425},
    {426u,"effect.ue3.warlord-426-native.v1","fx_m_mi_01.fx_mi.fx_j_pa_circlelenz_01_1_ad","fx_mastermaterial.fx_mm.fx_mm_light_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.ad.f431613b2bdf",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_426,WARLORD_NATIVE_PARAMETERS_426,WARLORD_NATIVE_SWITCHES_426},
    {427u,"effect.ue3.warlord-427-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_flar_01_04_dt_ad","fx_m_mi_00.fx_m.fx_d_pa_flare_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.flare.01.ad.d315e5109f03",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_427,WARLORD_NATIVE_PARAMETERS_427,WARLORD_NATIVE_SWITCHES_427},
    {428u,"effect.ue3.warlord-428-native.v1","fx_m_mi_01.fx_m.fx_f_pa_wind_05_tr","fx_m_mi_01.fx_m.fx_f_pa_wind_05_tr","ue3.material.fx.m.mi.01.fx.m.fx.f.pa.wind.05.tr.aa19ee0d4873",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_428,WARLORD_NATIVE_PARAMETERS_428,WARLORD_NATIVE_SWITCHES_428},
    {429u,"effect.ue3.warlord-429-native.v1","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_02_ad","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_02_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.circ.02.ad.0861ec48398e",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_429,WARLORD_NATIVE_PARAMETERS_429,WARLORD_NATIVE_SWITCHES_429},
    {430u,"effect.ue3.warlord-430-native.v1","bfx_m_mi_00.bfx_mi.bfx_i_pa_thunder_02_ad","bfx_m_mi_00.bfx_m.bfx_i_pa_thunder_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.thunder.01.ad.21cb5623910c",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_430,WARLORD_NATIVE_PARAMETERS_430,WARLORD_NATIVE_SWITCHES_430},
    {431u,"effect.ue3.warlord-431-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_gl_01_3_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_431,WARLORD_NATIVE_PARAMETERS_431,WARLORD_NATIVE_SWITCHES_431},
    {432u,"effect.ue3.warlord-432-native.v1","fx_m_mi_00.fx_mi.fx_c_pa_lensflare_01_06_ad","fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.c.pa.lensflare.01.ad.2cdc706962af",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_432,WARLORD_NATIVE_PARAMETERS_432,WARLORD_NATIVE_SWITCHES_432},
    {433u,"effect.ue3.warlord-433-native.v1","bfx_m_mi_00.bfx_m.bfx_i_pa_backglow_cl_02_tr","bfx_m_mi_00.bfx_m.bfx_i_pa_backglow_cl_02_tr","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.backglow.cl.02.tr.5de7680be180",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_433,WARLORD_NATIVE_PARAMETERS_433,WARLORD_NATIVE_SWITCHES_433},
    {434u,"effect.ue3.warlord-434-native.v1","fx_m_mi_w_00.mi.fx_w_pa_decmaster_09_tr","fx_m_mi_00.fx_m.fx_d_pa_decmaster_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.decmaster.01.tr.3d2fbe4a579e",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_434,WARLORD_NATIVE_PARAMETERS_434,WARLORD_NATIVE_SWITCHES_434},
    {435u,"effect.ue3.warlord-435-native.v1","fx_m_mi_s_00.fx_mi.fx_s_pa_decmaster_01_47_tr","fx_m_mi_00.fx_m.fx_d_pa_decmaster_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.decmaster.01.tr.3d2fbe4a579e",false,"sprite",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_435,WARLORD_NATIVE_PARAMETERS_435,WARLORD_NATIVE_SWITCHES_435},
    {436u,"effect.ue3.warlord-436-native.v1","fx_m_mi_03.fx_mi.fx_n_pa_ring_07_65_ad","fx_m_mi_03.fx_m.fx_n_pa_ring_07_ad","ue3.material.fx.m.mi.03.fx.m.fx.n.pa.ring.07.ad.89ae5b8c2a04",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_436,WARLORD_NATIVE_PARAMETERS_436,WARLORD_NATIVE_SWITCHES_436},
    {437u,"effect.ue3.warlord-437-native.v1","bfx_m_mi_00.bfx_mi.bfx_i_pa_thunder_03_ad","bfx_m_mi_00.bfx_m.bfx_i_pa_thunder_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.thunder.01.ad.21cb5623910c",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_437,WARLORD_NATIVE_PARAMETERS_437,WARLORD_NATIVE_SWITCHES_437},
    {438u,"effect.ue3.warlord-438-native.v1","fx_m_mi_o_00.fx_m.fx_o_pa_lightingdetail_01_1_ad","fx_m_mi_o_00.fx_m.fx_o_pa_lightingdetail_01_1_ad","ue3.material.fx.m.mi.o.00.fx.m.fx.o.pa.lightingdetail.01.1.ad.0729eef36669",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_438,WARLORD_NATIVE_PARAMETERS_438,WARLORD_NATIVE_SWITCHES_438},
    {439u,"effect.ue3.warlord-439-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_gl_01_6_ad","fx_mastermaterial.fx_mm.fx_mm_simple_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_439,WARLORD_NATIVE_PARAMETERS_439,WARLORD_NATIVE_SWITCHES_439},
    {440u,"effect.ue3.warlord-440-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_pa_circ_01_02_tr","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_tr","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.circ.01.tr.f3e3bcf906f5",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_440,WARLORD_NATIVE_PARAMETERS_440,WARLORD_NATIVE_SWITCHES_440},
    {441u,"effect.ue3.warlord-441-native.v1","fx_m_mi_03.fx_mi.fx_m_pa_noise_01_2_tr","fx_m_mi_03.fx_m.fx_m_pa_noise_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.m.pa.noise.01.tr.05a4fb7d3429",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_441,WARLORD_NATIVE_PARAMETERS_441,WARLORD_NATIVE_SWITCHES_441},
    {442u,"effect.ue3.warlord-442-native.v1","fx_m_mi_05.fx_m.fx_c_pa_smokeseq_01_tr","fx_m_mi_05.fx_m.fx_c_pa_smokeseq_01_tr","ue3.material.fx.m.mi.05.fx.m.fx.c.pa.smokeseq.01.tr.9bab1a59b06b",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_442,WARLORD_NATIVE_PARAMETERS_442,WARLORD_NATIVE_SWITCHES_442},
    {443u,"effect.ue3.warlord-443-native.v1","fx_m_mi_x_00.fx_m.fx_x_pa_lightinginvert_01_tr","fx_m_mi_x_00.fx_m.fx_x_pa_lightinginvert_01_tr","ue3.material.fx.m.mi.x.00.fx.m.fx.x.pa.lightinginvert.01.tr.1a3a2bc7889c",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_443,WARLORD_NATIVE_PARAMETERS_443,WARLORD_NATIVE_SWITCHES_443},
    {444u,"effect.ue3.warlord-444-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_ringmaster_01_02_ad","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.ad.f754ad06cfe6",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_444,WARLORD_NATIVE_PARAMETERS_444,WARLORD_NATIVE_SWITCHES_444},
    {445u,"effect.ue3.warlord-445-native.v1","fx_m_mi_h_00.fx_mi.fx_h_pa_worldoffset_02_39_tr","fx_m_mi_03.fx_mi.fx_m_pa_worldoffset_02_tr","ue3.material.fx.m.mi.03.fx.mi.fx.m.pa.worldoffset.02.tr.6507c4b13a9b",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_445,WARLORD_NATIVE_PARAMETERS_445,WARLORD_NATIVE_SWITCHES_445},
    {446u,"effect.ue3.warlord-446-native.v1","fx_m_mi_d_00.fx_mi.fx_d_me_worldpositionoffset_sinwave_01_04_ad","fx_m_mi_d_00.fx_m.fx_d_me_worldpositionoffset_sinwave_01_ad","ue3.material.fx.m.mi.d.00.fx.m.fx.d.me.worldpositionoffset.sinwave.01.ad.1feb93cbb95e",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_446,WARLORD_NATIVE_PARAMETERS_446,WARLORD_NATIVE_SWITCHES_446},
    {447u,"effect.ue3.warlord-447-native.v1","fx_m_mi_d_00.fx_mi.fx_d_pa_glow_02_03_ad","fx_m_mi_d_00.fx_m.fx_d_pa_glow_02_ad","ue3.material.fx.m.mi.d.00.fx.m.fx.d.pa.glow.02.ad.3887cf5cc484",false,"sprite",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_447,WARLORD_NATIVE_PARAMETERS_447,WARLORD_NATIVE_SWITCHES_447},
    {448u,"effect.ue3.warlord-448-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_glow_01_02_ad","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.circ.01.ad.0f4a4414a90a",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_448,WARLORD_NATIVE_PARAMETERS_448,WARLORD_NATIVE_SWITCHES_448},
    {449u,"effect.ue3.warlord-449-native.v1","fx_m_mi_02.fx_mi.fx_c_pa_ring_01_1_tr","fx_m_mi_03.fx_m.fx_f_pa_ring_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.f.pa.ring.01.tr.bc7c6b415930",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_449,WARLORD_NATIVE_PARAMETERS_449,WARLORD_NATIVE_SWITCHES_449},
    {450u,"effect.ue3.warlord-450-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_atta_05_07_ad","fx_mastermaterial.fx_mm.fx_mm_light_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.ad.f431613b2bdf",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_450,WARLORD_NATIVE_PARAMETERS_450,WARLORD_NATIVE_SWITCHES_450},
    {451u,"effect.ue3.warlord-451-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_turbulence_01_19_dt_tr","fx_m_mi_03.fx_m.fx_d_pa_turbulence_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.turbulence.01.tr.324978c21e60",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_451,WARLORD_NATIVE_PARAMETERS_451,WARLORD_NATIVE_SWITCHES_451},
    {672u,"effect.ue3.warlord-672-native.v1","fx_post.fx_mi.fx_c_pa_filmnoise_01_tr","fx_post.fx_m.fx_c_po_filmnoise_01","ue3.material.fx.post.fx.m.fx.c.po.filmnoise.01.ee5c810bbf85",false,"screenPost",true,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_672,WARLORD_NATIVE_PARAMETERS_672,WARLORD_NATIVE_SWITCHES_672},
    {452u,"effect.ue3.warlord-452-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_pa_flar_02_01_ad","fx_m_mi_00.fx_m.fx_d_pa_flare_02_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.flare.02.ad.7d386b4627a6",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_452,WARLORD_NATIVE_PARAMETERS_452,WARLORD_NATIVE_SWITCHES_452},
    {453u,"effect.ue3.warlord-453-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_atta_09_04_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_453,WARLORD_NATIVE_PARAMETERS_453,WARLORD_NATIVE_SWITCHES_453},
    {454u,"effect.ue3.warlord-454-native.v1","fx_m_mi_01.fx_mi.fx_j_pa_chromaring_01_ad","fx_mastermaterial.fx_mm.fx_mm_light_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.ad.f431613b2bdf",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_454,WARLORD_NATIVE_PARAMETERS_454,WARLORD_NATIVE_SWITCHES_454},
    {455u,"effect.ue3.warlord-455-native.v1","fx_m_mi_02.fx_m.fx_j_pa_circledisort_01_ad","fx_m_mi_02.fx_m.fx_j_pa_circledisort_01_ad","ue3.material.fx.m.mi.02.fx.m.fx.j.pa.circledisort.01.ad.8852527671d6",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_455,WARLORD_NATIVE_PARAMETERS_455,WARLORD_NATIVE_SWITCHES_455},
    {456u,"effect.ue3.warlord-456-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_atta_09_02_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_456,WARLORD_NATIVE_PARAMETERS_456,WARLORD_NATIVE_SWITCHES_456},
    {457u,"effect.ue3.warlord-457-native.v1","bfx_m_mi_00.bfx_mi.bfx_j_pa_ring_07_06_ad","fx_m_mi_03.fx_m.fx_d_pa_ring_07_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.ad.82e9116584b2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_457,WARLORD_NATIVE_PARAMETERS_457,WARLORD_NATIVE_SWITCHES_457},
    {458u,"effect.ue3.warlord-458-native.v1","bfx_m_mi_00.bfx_mi.bfx_c_pa_lightflare_01_ddt_4_ad","bfx_m_mi_00.bfx_m.bfx_c_pa_lightflare_01_ddt_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.c.pa.lightflare.01.ddt.ad.33b058471d6a",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_458,WARLORD_NATIVE_PARAMETERS_458,WARLORD_NATIVE_SWITCHES_458},
    {459u,"effect.ue3.warlord-459-native.v1","fx_m_mi_01.fx_mi.fx_j_me_ringrainbow_01_2_ts_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_459,WARLORD_NATIVE_PARAMETERS_459,WARLORD_NATIVE_SWITCHES_459},
    {660u,"effect.ue3.warlord-660-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_db_01_2_ad","fx_mastermaterial.fx_mm.fx_mm_simple_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_660,WARLORD_NATIVE_PARAMETERS_660,WARLORD_NATIVE_SWITCHES_660},
    {661u,"effect.ue3.warlord-661-native.v1","fx_m_mi_03.fx_mi.fx_k_pa_ring_06_ad","fx_m_mi_03.fx_m.fx_d_pa_ring_07_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.ad.82e9116584b2",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_661,WARLORD_NATIVE_PARAMETERS_661,WARLORD_NATIVE_SWITCHES_661},
    {662u,"effect.ue3.warlord-662-native.v1","fx_m_mi_03.fx_mi.fx_k_pa_ring_08_ad","fx_m_mi_03.fx_m.fx_d_pa_ring_07_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.ad.82e9116584b2",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_662,WARLORD_NATIVE_PARAMETERS_662,WARLORD_NATIVE_SWITCHES_662},
    {663u,"effect.ue3.warlord-663-native.v1","fx_m_mi_00.fx_mi.fx_d_me_master_01_078_ts_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",true,"mesh",false,true,true,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_663,WARLORD_NATIVE_PARAMETERS_663,WARLORD_NATIVE_SWITCHES_663},
    {664u,"effect.ue3.warlord-664-native.v1","fx_m_mi_00.fx_mi.fx_c_pa_lensflare_01_05_ad","fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.c.pa.lensflare.01.ad.2cdc706962af",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_664,WARLORD_NATIVE_PARAMETERS_664,WARLORD_NATIVE_SWITCHES_664},
    {665u,"effect.ue3.warlord-665-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_beam_01_01_tr","fx_m_mi_02.fx_m.fx_j_pa_beam_01_tr","ue3.material.fx.m.mi.02.fx.m.fx.j.pa.beam.01.tr.f2b0f604c767",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_665,WARLORD_NATIVE_PARAMETERS_665,WARLORD_NATIVE_SWITCHES_665},
    {666u,"effect.ue3.warlord-666-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_master_01_019_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",false,"sprite",false,true,true,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_666,WARLORD_NATIVE_PARAMETERS_666,WARLORD_NATIVE_SWITCHES_666},
    {667u,"effect.ue3.warlord-667-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_pa_circ_01_01_dt_ad","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.circ.01.ad.0f4a4414a90a",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_667,WARLORD_NATIVE_PARAMETERS_667,WARLORD_NATIVE_SWITCHES_667},
    {668u,"effect.ue3.warlord-668-native.v1","fx_m_mi_01.fx_mi.fx_j_me_floorstrm_02_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_668,WARLORD_NATIVE_PARAMETERS_668,WARLORD_NATIVE_SWITCHES_668},
    {669u,"effect.ue3.warlord-669-native.v1","bfx_m_mi_00.bfx_mi.bfx_i_pa_thunder_04_ad","bfx_m_mi_00.bfx_m.bfx_i_pa_thunder_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.thunder.01.ad.21cb5623910c",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_669,WARLORD_NATIVE_PARAMETERS_669,WARLORD_NATIVE_SWITCHES_669},
    {670u,"effect.ue3.warlord-670-native.v1","fx_m_mi_00.fx_mi.fx_d_me_master_01_020_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_670,WARLORD_NATIVE_PARAMETERS_670,WARLORD_NATIVE_SWITCHES_670},
    {671u,"effect.ue3.warlord-671-native.v1","fx_m_mi_00.fx_mi.fx_d_me_master_01_040_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_671,WARLORD_NATIVE_PARAMETERS_671,WARLORD_NATIVE_SWITCHES_671},
    {1000u,"effect.ue3.warlord-1000-native.v1","fx_m_mi_k_00.fx_mi.fx_k_me_makeflow_03_03_tr","fx_m_mi_k_00.fx_m.fx_k_me_makeflow_03_tr","ue3.material.fx.m.mi.k.00.fx.m.fx.k.me.makeflow.03.tr.6e5c0dd36299",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1000,WARLORD_NATIVE_PARAMETERS_1000,WARLORD_NATIVE_SWITCHES_1000},
    {1001u,"effect.ue3.warlord-1001-native.v1","fx_m_mi_01.fx_mi.fx_k_pa_fd_01_05_tr","fx_mastermaterial.fx_mm.fx_mm_dissolve_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.dissolve.01.tr.a799c9636783",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1001,WARLORD_NATIVE_PARAMETERS_1001,WARLORD_NATIVE_SWITCHES_1001},
    {1002u,"effect.ue3.warlord-1002-native.v1","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_ad","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.circ.01.ad.0f4a4414a90a",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1002,WARLORD_NATIVE_PARAMETERS_1002,WARLORD_NATIVE_SWITCHES_1002},
    {1003u,"effect.ue3.warlord-1003-native.v1","fx_m_mi_02.fx_mi.fx_e_pa_ht_18_1_tr","fx_m_mi_02.fx_m.fx_f_pa_shine_01_0_tr","ue3.material.fx.m.mi.02.fx.m.fx.f.pa.shine.01.0.tr.3d21bd1f3e79",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1003,WARLORD_NATIVE_PARAMETERS_1003,WARLORD_NATIVE_SWITCHES_1003},
    {1004u,"effect.ue3.warlord-1004-native.v1","bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad","bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.glow.01.ad.0857e02620a0",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1004,WARLORD_NATIVE_PARAMETERS_1004,WARLORD_NATIVE_SWITCHES_1004},
    {1005u,"effect.ue3.warlord-1005-native.v1","fx_m_mi_01.fx_mi.fx_h_me_fd_02_1_ts_tr","fx_m_mi_03.fx_m.fx_m_pa_skull_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.m.pa.skull.01.tr.fd2982a2fbb3",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1005,WARLORD_NATIVE_PARAMETERS_1005,WARLORD_NATIVE_SWITCHES_1005},
    {1006u,"effect.ue3.warlord-1006-native.v1","fx_m_mi_02.fx_mi.fx_k_me_makeflow_02_11_tr","fx_m_mi_02.fx_m.fx_k_me_makeflow_02_tr","ue3.material.fx.m.mi.02.fx.m.fx.k.me.makeflow.02.tr.5059859991f8",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1006,WARLORD_NATIVE_PARAMETERS_1006,WARLORD_NATIVE_SWITCHES_1006},
    {1007u,"effect.ue3.warlord-1007-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_gl_01_4_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1007,WARLORD_NATIVE_PARAMETERS_1007,WARLORD_NATIVE_SWITCHES_1007},
    {1008u,"effect.ue3.warlord-1008-native.v1","fx_m_mi_00.fx_mi.fx_c_pa_lensflare_01_07_ad","fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.c.pa.lensflare.01.ad.2cdc706962af",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1008,WARLORD_NATIVE_PARAMETERS_1008,WARLORD_NATIVE_SWITCHES_1008},
    {1009u,"effect.ue3.warlord-1009-native.v1","fx_m_mi_00.fx_mi.fx_c_pa_lensflare_01_13_dt5_ad","fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.c.pa.lensflare.01.ad.2cdc706962af",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1009,WARLORD_NATIVE_PARAMETERS_1009,WARLORD_NATIVE_SWITCHES_1009},
    {1010u,"effect.ue3.warlord-1010-native.v1","fx_m_mi_m_00.fx_mi.fx_m_pa_spritewave_01_30_tr","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1010,WARLORD_NATIVE_PARAMETERS_1010,WARLORD_NATIVE_SWITCHES_1010},
    {1011u,"effect.ue3.warlord-1011-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_atta_05_03_tr","fx_mastermaterial.fx_mm.fx_mm_light_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.tr.8a6435f0c4e0",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1011,WARLORD_NATIVE_PARAMETERS_1011,WARLORD_NATIVE_SWITCHES_1011},
    {1012u,"effect.ue3.warlord-1012-native.v1","fx_m_mi_00.fx_mi.fx_d_me_flow_02_05_tr","fx_m_mi_00.fx_m.fx_d_me_flow_02_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.me.flow.02.tr.ef586ed806c9",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1012,WARLORD_NATIVE_PARAMETERS_1012,WARLORD_NATIVE_SWITCHES_1012},
    {1013u,"effect.ue3.warlord-1013-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_bull_01_02_ma","fx_mastermaterial.fx_mm.fx_mm_basic_01_ma","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ma.a9c32421250b",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1013,WARLORD_NATIVE_PARAMETERS_1013,WARLORD_NATIVE_SWITCHES_1013},
    {1015u,"effect.ue3.warlord-1015-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_pa_smoke_ulit_01_05_tr","bfx_m_mi_00.bfx_m.bfx_d_pa_smokethin_unlit_01_tr","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.smokethin.unlit.01.tr.2800e357909a",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1015,WARLORD_NATIVE_PARAMETERS_1015,WARLORD_NATIVE_SWITCHES_1015},
    {1017u,"effect.ue3.warlord-1017-native.v1","fx_m_mi_03.fx_mi.fx_m_pa_skull_02_23_tr","fx_m_mi_03.fx_m.fx_m_pa_skull_02_tr","ue3.material.fx.m.mi.03.fx.m.fx.m.pa.skull.02.tr.418321884f93",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1017,WARLORD_NATIVE_PARAMETERS_1017,WARLORD_NATIVE_SWITCHES_1017},
    {1018u,"effect.ue3.warlord-1018-native.v1","fx_m_mi_05.fx_m.fx_b_pa_cd_02_tr","fx_m_mi_05.fx_m.fx_b_pa_cd_02_tr","ue3.material.fx.m.mi.05.fx.m.fx.b.pa.cd.02.tr.7f417381c98d",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1018,WARLORD_NATIVE_PARAMETERS_1018,WARLORD_NATIVE_SWITCHES_1018},
    {1020u,"effect.ue3.warlord-1020-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_fd_07_1_ad","fx_m_mi_01.fx_m.fx_e_pa_twinkle_01_ad","ue3.material.fx.m.mi.01.fx.m.fx.e.pa.twinkle.01.ad.72979792ae2b",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1020,WARLORD_NATIVE_PARAMETERS_1020,WARLORD_NATIVE_SWITCHES_1020},
    {1021u,"effect.ue3.warlord-1021-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_pa_circ_01_02_ad","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.circ.01.ad.0f4a4414a90a",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1021,WARLORD_NATIVE_PARAMETERS_1021,WARLORD_NATIVE_SWITCHES_1021},
    {1022u,"effect.ue3.warlord-1022-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_db_01_1_ad","fx_mastermaterial.fx_mm.fx_mm_simple_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1022,WARLORD_NATIVE_PARAMETERS_1022,WARLORD_NATIVE_SWITCHES_1022},
    {1023u,"effect.ue3.warlord-1023-native.v1","fx_m_mi_03.fx_mi.fx_m_pa_missiletrail_01_1_tr","fx_m_mi_03.fx_m.fx_m_pa_missiletrail_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.m.pa.missiletrail.01.tr.9641f8d91e6a",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1023,WARLORD_NATIVE_PARAMETERS_1023,WARLORD_NATIVE_SWITCHES_1023},
    {1024u,"effect.ue3.warlord-1024-native.v1","fx_m_mi_03.fx_mi.fx_m_pa_missiletrail_01_3_tr","fx_m_mi_03.fx_m.fx_m_pa_missiletrail_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.m.pa.missiletrail.01.tr.9641f8d91e6a",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1024,WARLORD_NATIVE_PARAMETERS_1024,WARLORD_NATIVE_SWITCHES_1024},
    {1025u,"effect.ue3.warlord-1025-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_pa_shine_01_11_ad","bfx_m_mi_00.bfx_m.bfx_d_pa_shine_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.shine.01.ad.d944bc82b616",false,"sprite",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1025,WARLORD_NATIVE_PARAMETERS_1025,WARLORD_NATIVE_SWITCHES_1025},
    {1027u,"effect.ue3.warlord-1027-native.v1","fx_m_mi_02.fx_mi.fx_h_pa_ap_01_1_tr","fx_mastermaterial.fx_mm.fx_mm_dissolve_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.dissolve.01.tr.a799c9636783",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1027,WARLORD_NATIVE_PARAMETERS_1027,WARLORD_NATIVE_SWITCHES_1027},
    {1028u,"effect.ue3.warlord-1028-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_fd_04_2_tr","fx_mastermaterial.fx_mm.fx_mm_dissolve_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.dissolve.01.tr.a799c9636783",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1028,WARLORD_NATIVE_PARAMETERS_1028,WARLORD_NATIVE_SWITCHES_1028},
    {1029u,"effect.ue3.warlord-1029-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_turbulence_01_06_tr","fx_m_mi_03.fx_m.fx_d_pa_turbulence_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.turbulence.01.tr.324978c21e60",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1029,WARLORD_NATIVE_PARAMETERS_1029,WARLORD_NATIVE_SWITCHES_1029},
    {1030u,"effect.ue3.warlord-1030-native.v1","bfx_m_mi_00.bfx_m.bfx_i_pa_backglow_01_tr","bfx_m_mi_00.bfx_m.bfx_i_pa_backglow_01_tr","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.backglow.01.tr.6257006fa291",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1030,WARLORD_NATIVE_PARAMETERS_1030,WARLORD_NATIVE_SWITCHES_1030},
    {1031u,"effect.ue3.warlord-1031-native.v1","fx_m_mi_05.fx_m.fx_a_me_panning_01_ts_ad","fx_m_mi_05.fx_m.fx_a_me_panning_01_ts_ad","ue3.material.fx.m.mi.05.fx.m.fx.a.me.panning.01.ts.ad.a2daab5b3ee5",true,"mesh",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1031,WARLORD_NATIVE_PARAMETERS_1031,WARLORD_NATIVE_SWITCHES_1031},
    {1032u,"effect.ue3.warlord-1032-native.v1","fx_m_mi_02.fx_mi.fx_k_pa_crackmove_05_tr","fx_m_mi_02.fx_m.fx_k_pa_crackmove_01_ts","ue3.material.fx.m.mi.02.fx.m.fx.k.pa.crackmove.01.ts.3048141f90d7",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1032,WARLORD_NATIVE_PARAMETERS_1032,WARLORD_NATIVE_SWITCHES_1032},
    {1033u,"effect.ue3.warlord-1033-native.v1","fx_m_mi_m_00.fx_mi.fx_m_pa_spritewave_01_48_tr","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1033,WARLORD_NATIVE_PARAMETERS_1033,WARLORD_NATIVE_SWITCHES_1033},
    {1034u,"effect.ue3.warlord-1034-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_fd_04_1_tr","fx_mastermaterial.fx_mm.fx_mm_dissolve_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.dissolve.01.tr.a799c9636783",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1034,WARLORD_NATIVE_PARAMETERS_1034,WARLORD_NATIVE_SWITCHES_1034},
    {1036u,"effect.ue3.warlord-1036-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_gl_03_1_ad","fx_mastermaterial.fx_mm.fx_mm_distortion_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.distortion.01.ad.fd38a9b02e64",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1036,WARLORD_NATIVE_PARAMETERS_1036,WARLORD_NATIVE_SWITCHES_1036},
    {1037u,"effect.ue3.warlord-1037-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_gl_01_2_tr","fx_mastermaterial.fx_mm.fx_mm_light_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.tr.8a6435f0c4e0",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1037,WARLORD_NATIVE_PARAMETERS_1037,WARLORD_NATIVE_SWITCHES_1037},
    {1038u,"effect.ue3.warlord-1038-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_shine_02_01_ad","fx_m_mi_02.fx_m.fx_d_pa_shine_02_ad","ue3.material.fx.m.mi.02.fx.m.fx.d.pa.shine.02.ad.29d7864aadba",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1038,WARLORD_NATIVE_PARAMETERS_1038,WARLORD_NATIVE_SWITCHES_1038},
    {1039u,"effect.ue3.warlord-1039-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_ht_07_2_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1039,WARLORD_NATIVE_PARAMETERS_1039,WARLORD_NATIVE_SWITCHES_1039},
    {1040u,"effect.ue3.warlord-1040-native.v1","fx_m_mi_k_00.fx_d_pa_ring_12_ad_inst352423sdsd","fx_m_mi_03.fx_m.fx_d_pa_ring_12_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.12.ad.6ebe3bbc0898",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1040,WARLORD_NATIVE_PARAMETERS_1040,WARLORD_NATIVE_SWITCHES_1040},
    {1041u,"effect.ue3.warlord-1041-native.v1","fx_m_mi_02.fx_m.fx_j_pa_lightinginvert_01_tr","fx_m_mi_02.fx_m.fx_j_pa_lightinginvert_01_tr","ue3.material.fx.m.mi.02.fx.m.fx.j.pa.lightinginvert.01.tr.188beedafb59",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1041,WARLORD_NATIVE_PARAMETERS_1041,WARLORD_NATIVE_SWITCHES_1041},
    {1042u,"effect.ue3.warlord-1042-native.v1","fx_m_mi_02.fx_mi.fx_h_pa_shorkwave_01_1_tr","fx_m_mi_03.fx_mi.fx_m_pa_smoke_01_tr","ue3.material.fx.m.mi.03.fx.mi.fx.m.pa.smoke.01.tr.f29f8f9b01e2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1042,WARLORD_NATIVE_PARAMETERS_1042,WARLORD_NATIVE_SWITCHES_1042},
    {1043u,"effect.ue3.warlord-1043-native.v1","fx_m_mi_01.fx_mi.fx_h_me_fd_02_3_ts_tr","fx_m_mi_03.fx_m.fx_m_pa_skull_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.m.pa.skull.01.tr.fd2982a2fbb3",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1043,WARLORD_NATIVE_PARAMETERS_1043,WARLORD_NATIVE_SWITCHES_1043},
    {1044u,"effect.ue3.warlord-1044-native.v1","bfx_m_mi_00.bfx_mi.bfx_i_pa_glow_01_01_ad","bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.glow.01.ad.0857e02620a0",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1044,WARLORD_NATIVE_PARAMETERS_1044,WARLORD_NATIVE_SWITCHES_1044},
    {1045u,"effect.ue3.warlord-1045-native.v1","fx_m_mi_05.fx_mi.fx_a_pa_panning_01_2_tr","fx_m_mi_05.fx_m.fx_a_pa_panning_01_tr","ue3.material.fx.m.mi.05.fx.m.fx.a.pa.panning.01.tr.ceb6b9f5a828",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1045,WARLORD_NATIVE_PARAMETERS_1045,WARLORD_NATIVE_SWITCHES_1045},
    {1046u,"effect.ue3.warlord-1046-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_cd_26_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1046,WARLORD_NATIVE_PARAMETERS_1046,WARLORD_NATIVE_SWITCHES_1046},
    {1047u,"effect.ue3.warlord-1047-native.v1","fx_m_mi_02.fx_m.fx_k_pa_electric_trail_02_1_ad","fx_m_mi_02.fx_m.fx_k_pa_electric_trail_02_1_ad","ue3.material.fx.m.mi.02.fx.m.fx.k.pa.electric.trail.02.1.ad.ccd3d2fd0812",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1047,WARLORD_NATIVE_PARAMETERS_1047,WARLORD_NATIVE_SWITCHES_1047},
    {1048u,"effect.ue3.warlord-1048-native.v1","fx_m_mi_x_00.fx_mi.fx_x_me_watertrail_29_02_dt_tr","fx_m_mi_03.fx_m.fx_m_me_watertrail_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.m.me.watertrail.01.tr.afa4aeba0c50",true,"mesh",false,true,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1048,WARLORD_NATIVE_PARAMETERS_1048,WARLORD_NATIVE_SWITCHES_1048},
    {1049u,"effect.ue3.warlord-1049-native.v1","fx_m_mi_m_00.fx_mi.fx_m_pa_aura_03_tr","fx_m_mi_05.fx_m.fx_c_pa_aura_02_tr","ue3.material.fx.m.mi.05.fx.m.fx.c.pa.aura.02.tr.6b6be3c64e52",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1049,WARLORD_NATIVE_PARAMETERS_1049,WARLORD_NATIVE_SWITCHES_1049},
    {1050u,"effect.ue3.warlord-1050-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_dark_05_02_tr","fx_m_mi_00.fx_m.fx_d_pa_dark_05_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.dark.05.tr.891593ac5322",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1050,WARLORD_NATIVE_PARAMETERS_1050,WARLORD_NATIVE_SWITCHES_1050},
    {1051u,"effect.ue3.warlord-1051-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_atta_05_09_ad","fx_mastermaterial.fx_mm.fx_mm_light_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.ad.f431613b2bdf",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1051,WARLORD_NATIVE_PARAMETERS_1051,WARLORD_NATIVE_SWITCHES_1051},
    {1052u,"effect.ue3.warlord-1052-native.v1","fx_m_mi_01.fx_mi.fx_j_pa_flare_01_ad","fx_mastermaterial.fx_mm.fx_mm_light_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.ad.f431613b2bdf",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1052,WARLORD_NATIVE_PARAMETERS_1052,WARLORD_NATIVE_SWITCHES_1052},
    {1053u,"effect.ue3.warlord-1053-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_flare_02_02_dt_ad","fx_m_mi_00.fx_m.fx_d_pa_flare_02_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.flare.02.ad.7d386b4627a6",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1053,WARLORD_NATIVE_PARAMETERS_1053,WARLORD_NATIVE_SWITCHES_1053},
    {1054u,"effect.ue3.warlord-1054-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_master_01_047_dt_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1054,WARLORD_NATIVE_PARAMETERS_1054,WARLORD_NATIVE_SWITCHES_1054},
    {1055u,"effect.ue3.warlord-1055-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_ri_04_1_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1055,WARLORD_NATIVE_PARAMETERS_1055,WARLORD_NATIVE_SWITCHES_1055},
    {1056u,"effect.ue3.warlord-1056-native.v1","fx_m_mi_03.fx_mi.fx_d_me_ringmaster_01_12_ts_fs_ad","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.ad.f754ad06cfe6",true,"mesh",false,true,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1056,WARLORD_NATIVE_PARAMETERS_1056,WARLORD_NATIVE_SWITCHES_1056},
    {1057u,"effect.ue3.warlord-1057-native.v1","fx_m_mi_00.fx_mi.fx_d_me_flow_02_05_ad","fx_m_mi_00.fx_m.fx_d_me_flow_02_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.me.flow.02.ad.7b3b29c74701",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1057,WARLORD_NATIVE_PARAMETERS_1057,WARLORD_NATIVE_SWITCHES_1057},
    {1058u,"effect.ue3.warlord-1058-native.v1","fx_m_mi_03.fx_mi.fx_o_pa_shockwave_02_2_ad","fx_m_mi_03.fx_m.fx_d_pa_shockwave_02_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.shockwave.02.ad.d2ce32a2f981",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1058,WARLORD_NATIVE_PARAMETERS_1058,WARLORD_NATIVE_SWITCHES_1058},
    {1059u,"effect.ue3.warlord-1059-native.v1","fx_m_mi_d_00.fx_mi.fx_d_pa_ringmaster_01_229_dt_tr","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.tr.5629c8a9bc12",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1059,WARLORD_NATIVE_PARAMETERS_1059,WARLORD_NATIVE_SWITCHES_1059},
    {1060u,"effect.ue3.warlord-1060-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_pa_smoke_ulit_01_08_tr","bfx_m_mi_00.bfx_m.bfx_d_pa_smokethin_unlit_01_tr","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.smokethin.unlit.01.tr.2800e357909a",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1060,WARLORD_NATIVE_PARAMETERS_1060,WARLORD_NATIVE_SWITCHES_1060},
    {1061u,"effect.ue3.warlord-1061-native.v1","fx_m_mi_00.fx_mi.fx_d_me_master_01_003_ds_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1061,WARLORD_NATIVE_PARAMETERS_1061,WARLORD_NATIVE_SWITCHES_1061},
    {1062u,"effect.ue3.warlord-1062-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_master_01_016_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1062,WARLORD_NATIVE_PARAMETERS_1062,WARLORD_NATIVE_SWITCHES_1062},
    {1063u,"effect.ue3.warlord-1063-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_flow_04_01_ts_dt_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",false,"sprite",false,true,true,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1063,WARLORD_NATIVE_PARAMETERS_1063,WARLORD_NATIVE_SWITCHES_1063},
    {1064u,"effect.ue3.warlord-1064-native.v1","fx_m_mi_m_00.fx_mi.fx_m_me_watertrail_01_46q_tr","fx_m_mi_03.fx_m.fx_m_me_watertrail_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.m.me.watertrail.01.tr.afa4aeba0c50",true,"mesh",false,true,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1064,WARLORD_NATIVE_PARAMETERS_1064,WARLORD_NATIVE_SWITCHES_1064},
    {1065u,"effect.ue3.warlord-1065-native.v1","fx_m_mi_05.fx_m.fx_c_pa_smokeseq_03_tr","fx_m_mi_05.fx_m.fx_c_pa_smokeseq_03_tr","ue3.material.fx.m.mi.05.fx.m.fx.c.pa.smokeseq.03.tr.eb7e375dcce2",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1065,WARLORD_NATIVE_PARAMETERS_1065,WARLORD_NATIVE_SWITCHES_1065},
    {1066u,"effect.ue3.warlord-1066-native.v1","fx_m_mi_03.fx_mi.fx_h_pa_ring_01_1_tr","fx_m_mi_03.fx_m.fx_d_pa_ring_07_tr","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.tr.ac51b180263c",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1066,WARLORD_NATIVE_PARAMETERS_1066,WARLORD_NATIVE_SWITCHES_1066},
    {1067u,"effect.ue3.warlord-1067-native.v1","fx_m_mi_03.fx_mi.fx_m_me_trail_02_4_tr","fx_m_mi_03.fx_m.fx_m_me_trail_02_tr","ue3.material.fx.m.mi.03.fx.m.fx.m.me.trail.02.tr.8742928bef93",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1067,WARLORD_NATIVE_PARAMETERS_1067,WARLORD_NATIVE_SWITCHES_1067},
    {1068u,"effect.ue3.warlord-1068-native.v1","fx_m_mi_01.fx_mi.fx_e_me_ap_02_5_ts_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1068,WARLORD_NATIVE_PARAMETERS_1068,WARLORD_NATIVE_SWITCHES_1068},
    {1069u,"effect.ue3.warlord-1069-native.v1","fx_m_mi_j_00.fx_mi.fx_j_pa_linearwave_02_16_tr","fx_m_mi_j_00.fx_m.fx_j_pa_linearwave_02_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.linearwave.02.tr.1719a2318109",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1069,WARLORD_NATIVE_PARAMETERS_1069,WARLORD_NATIVE_SWITCHES_1069},
    {1070u,"effect.ue3.warlord-1070-native.v1","fx_m_mi_00.fx_m.fx_j_pa_exsmokes_01_tr","fx_m_mi_00.fx_m.fx_j_pa_exsmokes_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.j.pa.exsmokes.01.tr.1e40d7299708",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1070,WARLORD_NATIVE_PARAMETERS_1070,WARLORD_NATIVE_SWITCHES_1070},
    {1071u,"effect.ue3.warlord-1071-native.v1","fx_m_mi_02.fx_m.fx_j_pa_smokes_1_tr","fx_m_mi_02.fx_m.fx_j_pa_smokes_1_tr","ue3.material.fx.m.mi.02.fx.m.fx.j.pa.smokes.1.tr.b3ed2b6f6415",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1071,WARLORD_NATIVE_PARAMETERS_1071,WARLORD_NATIVE_SWITCHES_1071},
    {1072u,"effect.ue3.warlord-1072-native.v1","bfx_m_mi_00.bfx_mi.bfx_e_pa_ht_05_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1072,WARLORD_NATIVE_PARAMETERS_1072,WARLORD_NATIVE_SWITCHES_1072},
    {1073u,"effect.ue3.warlord-1073-native.v1","fx_m_mi_05.fx_m.fx_b_pa_smoke_03_tr","fx_m_mi_05.fx_m.fx_b_pa_smoke_03_tr","ue3.material.fx.m.mi.05.fx.m.fx.b.pa.smoke.03.tr.01821ccdf0b2",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1073,WARLORD_NATIVE_PARAMETERS_1073,WARLORD_NATIVE_SWITCHES_1073},
    {1074u,"effect.ue3.warlord-1074-native.v1","fx_m_mi_r_00.fx_mi.fx_r_me_master_15_04_ts_dt_fs_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,true,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1074,WARLORD_NATIVE_PARAMETERS_1074,WARLORD_NATIVE_SWITCHES_1074},
    {1075u,"effect.ue3.warlord-1075-native.v1","fx_m_mi_r_00.fx_mi.fx_r_me_master_15_03_ts_dt_fs_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,true,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1075,WARLORD_NATIVE_PARAMETERS_1075,WARLORD_NATIVE_SWITCHES_1075},
    {1076u,"effect.ue3.warlord-1076-native.v1","fx_m_mi_02.fx_mi.fx_k_me_makeflow_02_03_tr","fx_m_mi_02.fx_m.fx_k_me_makeflow_02_tr","ue3.material.fx.m.mi.02.fx.m.fx.k.me.makeflow.02.tr.5059859991f8",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1076,WARLORD_NATIVE_PARAMETERS_1076,WARLORD_NATIVE_SWITCHES_1076},
    {1077u,"effect.ue3.warlord-1077-native.v1","fx_m_mi_01.fx_mi.fx_c_pa_flickline_01_1_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1077,WARLORD_NATIVE_PARAMETERS_1077,WARLORD_NATIVE_SWITCHES_1077},
    {1078u,"effect.ue3.warlord-1078-native.v1","fx_m_mi_x_00.fx_mi.fx_x_me_flowtrail_03_03_tr","fx_m_mi_02.fx_m.fx_k_me_flowtrail_01_ts_tr","ue3.material.fx.m.mi.02.fx.m.fx.k.me.flowtrail.01.ts.tr.bc0628267aaa",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1078,WARLORD_NATIVE_PARAMETERS_1078,WARLORD_NATIVE_SWITCHES_1078},
    {1079u,"effect.ue3.warlord-1079-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_pa_circ_01_04_fs_dt_ad","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.circ.01.ad.0f4a4414a90a",false,"sprite",false,true,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1079,WARLORD_NATIVE_PARAMETERS_1079,WARLORD_NATIVE_SWITCHES_1079},
    {1080u,"effect.ue3.warlord-1080-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_master_01_031_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",false,"sprite",false,false,true,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1080,WARLORD_NATIVE_PARAMETERS_1080,WARLORD_NATIVE_SWITCHES_1080},
    {1081u,"effect.ue3.warlord-1081-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_master_10_02_dt_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1081,WARLORD_NATIVE_PARAMETERS_1081,WARLORD_NATIVE_SWITCHES_1081},
    {1082u,"effect.ue3.warlord-1082-native.v1","fx_m_mi_s_00.fx_mi.fx_s_pa_ht_18_3_tr","fx_m_mi_02.fx_m.fx_f_pa_shine_01_0_tr","ue3.material.fx.m.mi.02.fx.m.fx.f.pa.shine.01.0.tr.3d21bd1f3e79",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1082,WARLORD_NATIVE_PARAMETERS_1082,WARLORD_NATIVE_SWITCHES_1082},
    {1083u,"effect.ue3.warlord-1083-native.v1","fx_m_mi_01.fx_mi.fx_j_pa_ap_23_1_tr","fx_m_mi_02.fx_m.fx_d_pa_shine_02_tr","ue3.material.fx.m.mi.02.fx.m.fx.d.pa.shine.02.tr.62e708fe3f21",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1083,WARLORD_NATIVE_PARAMETERS_1083,WARLORD_NATIVE_SWITCHES_1083},
    {1084u,"effect.ue3.warlord-1084-native.v1","fx_m_mi_j_00.fx_mi.fx_j_po_rgbnoise_01_01_tr","fx_m_mi_j_00.fx_m.fx_j_po_rgbnoise_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.po.rgbnoise.01.tr.81291483aea9",false,"screenPost",true,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1084,WARLORD_NATIVE_PARAMETERS_1084,WARLORD_NATIVE_SWITCHES_1084},
    {1085u,"effect.ue3.warlord-1085-native.v1","fx_m_mi_00.fx_mi.fx_c_pa_lensflare_01_11_dt5_ad","fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.c.pa.lensflare.01.ad.2cdc706962af",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1085,WARLORD_NATIVE_PARAMETERS_1085,WARLORD_NATIVE_SWITCHES_1085},
    {1086u,"effect.ue3.warlord-1086-native.v1","fx_m_mi_o_00.fx_mi.fx_o_pa_spritewave_01_29_tr","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_ad","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.ad.caabbddf8b55",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1086,WARLORD_NATIVE_PARAMETERS_1086,WARLORD_NATIVE_SWITCHES_1086},
    {1087u,"effect.ue3.warlord-1087-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_gl_01_6_ad","fx_mastermaterial.fx_mm.fx_mm_simple_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1087,WARLORD_NATIVE_PARAMETERS_1087,WARLORD_NATIVE_SWITCHES_1087},
    {1088u,"effect.ue3.warlord-1088-native.v1","fx_m_mi_s_00.fx_mi.fx_s_me_horn_07_tr","fx_m_mi_o_00.fx_m.fx_o_me_horn_01_tr","ue3.material.fx.m.mi.o.00.fx.m.fx.o.me.horn.01.tr.ca9cd86a26f9",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1088,WARLORD_NATIVE_PARAMETERS_1088,WARLORD_NATIVE_SWITCHES_1088},
    {1089u,"effect.ue3.warlord-1089-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_circ_01_2_tr","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_tr","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.circ.01.tr.f3e3bcf906f5",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1089,WARLORD_NATIVE_PARAMETERS_1089,WARLORD_NATIVE_SWITCHES_1089},
    {1090u,"effect.ue3.warlord-1090-native.v1","fx_m_mi_01.fx_m.fx_f_pa_dust_01_tr","fx_m_mi_01.fx_m.fx_f_pa_dust_01_tr","ue3.material.fx.m.mi.01.fx.m.fx.f.pa.dust.01.tr.0ef42417db81",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1090,WARLORD_NATIVE_PARAMETERS_1090,WARLORD_NATIVE_SWITCHES_1090},
    {1091u,"effect.ue3.warlord-1091-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_ring_07_21_tr","fx_m_mi_03.fx_m.fx_d_pa_ring_07_tr","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.tr.ac51b180263c",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1091,WARLORD_NATIVE_PARAMETERS_1091,WARLORD_NATIVE_SWITCHES_1091},
    {1092u,"effect.ue3.warlord-1092-native.v1","fx_m_mi_k_00.fx_mi.fx_k_pa_windturb_01_02_tr","fx_m_mi_02.fx_m.fx_k_pa_turbpa_01_tr","ue3.material.fx.m.mi.02.fx.m.fx.k.pa.turbpa.01.tr.415aae7d3b93",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1092,WARLORD_NATIVE_PARAMETERS_1092,WARLORD_NATIVE_SWITCHES_1092},
    {1093u,"effect.ue3.warlord-1093-native.v1","bfx_m_mi_00.bfx_mi.bfx_h_pa_circle_01_02_ad","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.circ.01.ad.0f4a4414a90a",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1093,WARLORD_NATIVE_PARAMETERS_1093,WARLORD_NATIVE_SWITCHES_1093},
    {1094u,"effect.ue3.warlord-1094-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_shockwave_02_07_dt_ad","fx_m_mi_03.fx_m.fx_d_pa_shockwave_02_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.shockwave.02.ad.d2ce32a2f981",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1094,WARLORD_NATIVE_PARAMETERS_1094,WARLORD_NATIVE_SWITCHES_1094},
    {1095u,"effect.ue3.warlord-1095-native.v1","fx_m_mi_03.fx_mi.fx_m_me_trail_02_24_tr","fx_m_mi_03.fx_m.fx_m_me_trail_02_tr","ue3.material.fx.m.mi.03.fx.m.fx.m.me.trail.02.tr.8742928bef93",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1095,WARLORD_NATIVE_PARAMETERS_1095,WARLORD_NATIVE_SWITCHES_1095},
    {1096u,"effect.ue3.warlord-1096-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_ht_05_1_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1096,WARLORD_NATIVE_PARAMETERS_1096,WARLORD_NATIVE_SWITCHES_1096},
    {1097u,"effect.ue3.warlord-1097-native.v1","fx_m_mi_h_00.fx_mi.fx_h_pa_spritewave_04_1_tr","fx_m_mi_h_00.fx_m.fx_h_pa_spritewave_02_tr","ue3.material.fx.m.mi.h.00.fx.m.fx.h.pa.spritewave.02.tr.6e910398492b",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1097,WARLORD_NATIVE_PARAMETERS_1097,WARLORD_NATIVE_SWITCHES_1097},
    {1098u,"effect.ue3.warlord-1098-native.v1","fx_m_mi_01.fx_mi.fx_m_pa_cd_06_1_tr","fx_mastermaterial.fx_mm.fx_mm_dissolve_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.dissolve.01.tr.a799c9636783",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1098,WARLORD_NATIVE_PARAMETERS_1098,WARLORD_NATIVE_SWITCHES_1098},
    {1099u,"effect.ue3.warlord-1099-native.v1","fx_m_mi_w_00.mi.fx_w_pa_ringaura_02_ad","fx_m_mi_03.fx_m.fx_d_pa_ring_07_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.ad.82e9116584b2",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1099,WARLORD_NATIVE_PARAMETERS_1099,WARLORD_NATIVE_SWITCHES_1099},
    {1100u,"effect.ue3.warlord-1100-native.v1","bfx_m_mi_00.bfx_mi.bfx_j_pa_turbulence_01_01_ad","fx_m_mi_03.fx_m.fx_d_pa_turbulence_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.turbulence.01.ad.798cb913f060",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1100,WARLORD_NATIVE_PARAMETERS_1100,WARLORD_NATIVE_SWITCHES_1100},
    {1101u,"effect.ue3.warlord-1101-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_ringmaster_01_12_ad","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.ad.f754ad06cfe6",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1101,WARLORD_NATIVE_PARAMETERS_1101,WARLORD_NATIVE_SWITCHES_1101},
    {1102u,"effect.ue3.warlord-1102-native.v1","fx_m_mi_01.fx_mi.fx_k_pa_backglow_cl_01_tr","bfx_m_mi_00.bfx_m.bfx_i_pa_backglow_cl_02_tr","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.backglow.cl.02.tr.5de7680be180",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1102,WARLORD_NATIVE_PARAMETERS_1102,WARLORD_NATIVE_SWITCHES_1102},
    {1103u,"effect.ue3.warlord-1103-native.v1","fx_m_mi_03.fx_mi.fx_m_pa_worldoffset_02_13_tr","fx_m_mi_03.fx_mi.fx_m_pa_worldoffset_02_tr","ue3.material.fx.m.mi.03.fx.mi.fx.m.pa.worldoffset.02.tr.6507c4b13a9b",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1103,WARLORD_NATIVE_PARAMETERS_1103,WARLORD_NATIVE_SWITCHES_1103},
    {1104u,"effect.ue3.warlord-1104-native.v1","fx_m_mi_w_00.mi.fx_w_pa_floorstrm_01_02_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1104,WARLORD_NATIVE_PARAMETERS_1104,WARLORD_NATIVE_SWITCHES_1104},
    {1105u,"effect.ue3.warlord-1105-native.v1","fx_m_mi_01.fx_mi.fx_m_me_floorstrm_04_1_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1105,WARLORD_NATIVE_PARAMETERS_1105,WARLORD_NATIVE_SWITCHES_1105},
    {1106u,"effect.ue3.warlord-1106-native.v1","fx_m_mi_02.fx_mi.fx_k_pa_distort_multi_09_ad","fx_m_mi_02.fx_m.fx_k_pa_distort_multi_01_ad","ue3.material.fx.m.mi.02.fx.m.fx.k.pa.distort.multi.01.ad.e74cada0a4fe",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1106,WARLORD_NATIVE_PARAMETERS_1106,WARLORD_NATIVE_SWITCHES_1106},
    {1107u,"effect.ue3.warlord-1107-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_ring_11_10_ad","fx_m_mi_03.fx_m.fx_d_pa_ring_11_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.11.ad.43bcef3b2175",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1107,WARLORD_NATIVE_PARAMETERS_1107,WARLORD_NATIVE_SWITCHES_1107},
    {1108u,"effect.ue3.warlord-1108-native.v1","fx_m_mi_01.fx_mi.fx_k_pa_glow_01_ad_dt","bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.glow.01.ad.0857e02620a0",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1108,WARLORD_NATIVE_PARAMETERS_1108,WARLORD_NATIVE_SWITCHES_1108},
    {1109u,"effect.ue3.warlord-1109-native.v1","fx_m_mi_00.fx_mi.fx_c_pa_lensflare_01_03_ad","fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.c.pa.lensflare.01.ad.2cdc706962af",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1109,WARLORD_NATIVE_PARAMETERS_1109,WARLORD_NATIVE_SWITCHES_1109},
    {1110u,"effect.ue3.warlord-1110-native.v1","fx_m_mi_03.fx_m.fx_d_markring_02_tr","fx_m_mi_03.fx_m.fx_d_markring_02_tr","ue3.material.fx.m.mi.03.fx.m.fx.d.markring.02.tr.5a6e7558a791",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1110,WARLORD_NATIVE_PARAMETERS_1110,WARLORD_NATIVE_SWITCHES_1110},
    {1111u,"effect.ue3.warlord-1111-native.v1","fx_m_mi_d_00.fx_mi.fx_d_pa_ringmaster_01_229_ad","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.ad.f754ad06cfe6",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1111,WARLORD_NATIVE_PARAMETERS_1111,WARLORD_NATIVE_SWITCHES_1111},
    {1112u,"effect.ue3.warlord-1112-native.v1","fx_m_mi_01.fx_mi.fx_m_pa_shorkwave_01_17_tr","fx_m_mi_03.fx_mi.fx_m_pa_smoke_01_tr","ue3.material.fx.m.mi.03.fx.mi.fx.m.pa.smoke.01.tr.f29f8f9b01e2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1112,WARLORD_NATIVE_PARAMETERS_1112,WARLORD_NATIVE_SWITCHES_1112},
    {1114u,"effect.ue3.warlord-1114-native.v1","fx_m_mi_w_00.mi.fx_w_pa_ring_04_02_ad","fx_m_mi_03.fx_m.fx_d_pa_ring_07_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.ad.82e9116584b2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1114,WARLORD_NATIVE_PARAMETERS_1114,WARLORD_NATIVE_SWITCHES_1114},
    {1115u,"effect.ue3.warlord-1115-native.v1","fx_m_mi_d_00.fx_mi.fx_d_pa_ringmaster_01_215_dt_tr","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.tr.5629c8a9bc12",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1115,WARLORD_NATIVE_PARAMETERS_1115,WARLORD_NATIVE_SWITCHES_1115},
    {1116u,"effect.ue3.warlord-1116-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_me_master_01_04_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1116,WARLORD_NATIVE_PARAMETERS_1116,WARLORD_NATIVE_SWITCHES_1116},
    {1117u,"effect.ue3.warlord-1117-native.v1","fx_m_mi_w_00.mi.fx_w_me_master_01_27_dt_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,true,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1117,WARLORD_NATIVE_PARAMETERS_1117,WARLORD_NATIVE_SWITCHES_1117},
    {1118u,"effect.ue3.warlord-1118-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_pa_master_01_15_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1118,WARLORD_NATIVE_PARAMETERS_1118,WARLORD_NATIVE_SWITCHES_1118},
    {1119u,"effect.ue3.warlord-1119-native.v1","fx_m_mi_w_00.mi.fx_w_pa_electric_02_ad","fx_m_mi_00.fx_m.fx_d_pa_electric_03_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.electric.03.ad.781f15e7340d",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1119,WARLORD_NATIVE_PARAMETERS_1119,WARLORD_NATIVE_SWITCHES_1119},
    {1120u,"effect.ue3.warlord-1120-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_afterburn_01_37_dt_tr","fx_m_mi_00.fx_m.fx_d_pa_afterburn_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.afterburn.01.tr.75ee17c3dfee",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1120,WARLORD_NATIVE_PARAMETERS_1120,WARLORD_NATIVE_SWITCHES_1120},
    {1121u,"effect.ue3.warlord-1121-native.v1","fx_m_mi_00.fx_mi.fx_d_me_master_01_021_ts_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1121,WARLORD_NATIVE_PARAMETERS_1121,WARLORD_NATIVE_SWITCHES_1121},
    {1124u,"effect.ue3.warlord-1124-native.v1","fx_m_mi_w_00.mi.fx_w_wg_gdd_01_01_ma","fx_m_mi_00.fx_m.fx_d_me_master_01_ma","ue3.material.fx.m.mi.00.fx.m.fx.d.me.master.01.ma.3e356747fd11",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1124,WARLORD_NATIVE_PARAMETERS_1124,WARLORD_NATIVE_SWITCHES_1124},
    {1125u,"effect.ue3.warlord-1125-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_dirt_02_01_dt_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1125,WARLORD_NATIVE_PARAMETERS_1125,WARLORD_NATIVE_SWITCHES_1125},
    {1126u,"effect.ue3.warlord-1126-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_atta_05_14_ad","fx_mastermaterial.fx_mm.fx_mm_light_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.ad.f431613b2bdf",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1126,WARLORD_NATIVE_PARAMETERS_1126,WARLORD_NATIVE_SWITCHES_1126},
    {1127u,"effect.ue3.warlord-1127-native.v1","fx_m_mi_01.fx_mi.fx_m_pa_shorkwave_01_30_tr","fx_m_mi_03.fx_mi.fx_m_pa_smoke_01_tr","ue3.material.fx.m.mi.03.fx.mi.fx.m.pa.smoke.01.tr.f29f8f9b01e2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1127,WARLORD_NATIVE_PARAMETERS_1127,WARLORD_NATIVE_SWITCHES_1127},
    {1128u,"effect.ue3.warlord-1128-native.v1","fx_m_mi_x_00.fx_mi.fx_x_pa_shockwave_05_02_dt_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1128,WARLORD_NATIVE_PARAMETERS_1128,WARLORD_NATIVE_SWITCHES_1128},
    {1129u,"effect.ue3.warlord-1129-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_atta_10_06_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1129,WARLORD_NATIVE_PARAMETERS_1129,WARLORD_NATIVE_SWITCHES_1129},
    {1130u,"effect.ue3.warlord-1130-native.v1","fx_m_mi_02.fx_mi.fx_k_me_flowtrail_01_14_tr_ts","fx_m_mi_02.fx_m.fx_k_me_flowtrail_01_ts_tr","ue3.material.fx.m.mi.02.fx.m.fx.k.me.flowtrail.01.ts.tr.bc0628267aaa",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1130,WARLORD_NATIVE_PARAMETERS_1130,WARLORD_NATIVE_SWITCHES_1130},
    {1131u,"effect.ue3.warlord-1131-native.v1","fx_m_mi_o_00.fx_mi.fx_o_pa_circledisort_01_01_ad","fx_m_mi_o_00.fx_m.fx_o_pa_circledisort_01_ad","ue3.material.fx.m.mi.o.00.fx.m.fx.o.pa.circledisort.01.ad.064914d30c99",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1131,WARLORD_NATIVE_PARAMETERS_1131,WARLORD_NATIVE_SWITCHES_1131},
    {1132u,"effect.ue3.warlord-1132-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_master_01_019_dt_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1132,WARLORD_NATIVE_PARAMETERS_1132,WARLORD_NATIVE_SWITCHES_1132},
    {1134u,"effect.ue3.warlord-1134-native.v1","fx_m_mi_o_00.fx_mi.fx_o_pa_ringmaster_01_10_ts_ad","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.ad.f754ad06cfe6",false,"sprite",false,false,true,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1134,WARLORD_NATIVE_PARAMETERS_1134,WARLORD_NATIVE_SWITCHES_1134},
    {1135u,"effect.ue3.warlord-1135-native.v1","fx_m_mi_00.fx_mi.fx_d_me_master_01_034_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1135,WARLORD_NATIVE_PARAMETERS_1135,WARLORD_NATIVE_SWITCHES_1135},
    {1136u,"effect.ue3.warlord-1136-native.v1","fx_m_mi_03.fx_mi.fx_i_pa_hit_01_ad","fx_m_mi_03.fx_m.fx_i_pa_portal_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.i.pa.portal.01.ad.258529955137",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1136,WARLORD_NATIVE_PARAMETERS_1136,WARLORD_NATIVE_SWITCHES_1136},
    {1137u,"effect.ue3.warlord-1137-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_ringmaster_01_15_ad","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.ad.f754ad06cfe6",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1137,WARLORD_NATIVE_PARAMETERS_1137,WARLORD_NATIVE_SWITCHES_1137},
    {1138u,"effect.ue3.warlord-1138-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_atta_11_09_ad","fx_mastermaterial.fx_mm.fx_mm_extraalpha_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.extraalpha.01.ad.22782accb244",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1138,WARLORD_NATIVE_PARAMETERS_1138,WARLORD_NATIVE_SWITCHES_1138},
    {1139u,"effect.ue3.warlord-1139-native.v1","fx_m_mi_u_00.fx_mi.fx_u_pa_shine_03_1_tr","fx_m_mi_02.fx_m.fx_d_pa_shine_02_tr","ue3.material.fx.m.mi.02.fx.m.fx.d.pa.shine.02.tr.62e708fe3f21",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1139,WARLORD_NATIVE_PARAMETERS_1139,WARLORD_NATIVE_SWITCHES_1139},
    {1140u,"effect.ue3.warlord-1140-native.v1","fx_m_mi_k_00.fx_mi.fx_k_pa_decmaster_01_07_tr","fx_m_mi_00.fx_m.fx_d_pa_decmaster_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.decmaster.01.tr.3d2fbe4a579e",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1140,WARLORD_NATIVE_PARAMETERS_1140,WARLORD_NATIVE_SWITCHES_1140},
    {1141u,"effect.ue3.warlord-1141-native.v1","fx_m_mi_w_00.mi.fx_w_pa_decmaster_01_19_tr","fx_m_mi_00.fx_m.fx_d_pa_decmaster_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.decmaster.01.tr.3d2fbe4a579e",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1141,WARLORD_NATIVE_PARAMETERS_1141,WARLORD_NATIVE_SWITCHES_1141},
    {1142u,"effect.ue3.warlord-1142-native.v1","fx_m_mi_w_00.mi.fx_w_pa_worldoffset_01_13_tr","fx_m_mi_03.fx_m.fx_m_pa_worldoffset_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.m.pa.worldoffset.01.tr.692cba4c40ad",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1142,WARLORD_NATIVE_PARAMETERS_1142,WARLORD_NATIVE_SWITCHES_1142},
    {1143u,"effect.ue3.warlord-1143-native.v1","fx_m_mi_00.fx_mi.fx_d_me_master_01_ph_03_msk","fx_m_mi_00.fx_m.fx_d_me_master_01_ph_msk","ue3.material.fx.m.mi.00.fx.m.fx.d.me.master.01.ph.msk.8230663740c0",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1143,WARLORD_NATIVE_PARAMETERS_1143,WARLORD_NATIVE_SWITCHES_1143},
    {1144u,"effect.ue3.warlord-1144-native.v1","fx_m_mi_d_00.fx_mi.fx_d_pa_master_01_225_dt_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1144,WARLORD_NATIVE_PARAMETERS_1144,WARLORD_NATIVE_SWITCHES_1144},
    {1145u,"effect.ue3.warlord-1145-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_cd_01_2_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1145,WARLORD_NATIVE_PARAMETERS_1145,WARLORD_NATIVE_SWITCHES_1145},
    {1146u,"effect.ue3.warlord-1146-native.v1","fx_m_mi_05.fx_m.fx_c_pa_glitter_01_tr","fx_m_mi_05.fx_m.fx_c_pa_glitter_01_tr","ue3.material.fx.m.mi.05.fx.m.fx.c.pa.glitter.01.tr.2f2891f60b47",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1146,WARLORD_NATIVE_PARAMETERS_1146,WARLORD_NATIVE_SWITCHES_1146},
    {1147u,"effect.ue3.warlord-1147-native.v1","fx_m_mi_n_00.fx_mi.fx_n_pa_decmaster_21_03_tr","fx_m_mi_00.fx_m.fx_d_pa_decmaster_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.decmaster.01.tr.3d2fbe4a579e",false,"sprite",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1147,WARLORD_NATIVE_PARAMETERS_1147,WARLORD_NATIVE_SWITCHES_1147},
    {1148u,"effect.ue3.warlord-1148-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_gl_01_2_ad","fx_mastermaterial.fx_mm.fx_mm_simple_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1148,WARLORD_NATIVE_PARAMETERS_1148,WARLORD_NATIVE_SWITCHES_1148},
    {1149u,"effect.ue3.warlord-1149-native.v1","fx_m_mi_o_00.fx_mi.fx_o_pa_ri_04_ad_2s","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1149,WARLORD_NATIVE_PARAMETERS_1149,WARLORD_NATIVE_SWITCHES_1149},
    {1150u,"effect.ue3.warlord-1150-native.v1","fx_m_mi_02.fx_mi.fx_d_pa_shine_02_07_tr","fx_m_mi_02.fx_m.fx_d_pa_shine_02_tr","ue3.material.fx.m.mi.02.fx.m.fx.d.pa.shine.02.tr.62e708fe3f21",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1150,WARLORD_NATIVE_PARAMETERS_1150,WARLORD_NATIVE_SWITCHES_1150},
    {1151u,"effect.ue3.warlord-1151-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_ringmaster_01_54_dt_ad","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.ad.f754ad06cfe6",false,"sprite",false,true,true,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1151,WARLORD_NATIVE_PARAMETERS_1151,WARLORD_NATIVE_SWITCHES_1151},
    {1152u,"effect.ue3.warlord-1152-native.v1","fx_m_mi_o_00.fx_m.fx_o_pa_circledisort_01_ad","fx_m_mi_o_00.fx_m.fx_o_pa_circledisort_01_ad","ue3.material.fx.m.mi.o.00.fx.m.fx.o.pa.circledisort.01.ad.064914d30c99",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1152,WARLORD_NATIVE_PARAMETERS_1152,WARLORD_NATIVE_SWITCHES_1152},
    {1153u,"effect.ue3.warlord-1153-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_ring_11_09_ts_tr","fx_m_mi_03.fx_m.fx_d_pa_ring_11_tr","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.11.tr.a0ed9ffc6d76",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1153,WARLORD_NATIVE_PARAMETERS_1153,WARLORD_NATIVE_SWITCHES_1153},
    {1154u,"effect.ue3.warlord-1154-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_pa_smoke_ulit_01_04_dt_tr","bfx_m_mi_00.bfx_m.bfx_d_pa_smokethin_unlit_01_tr","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.smokethin.unlit.01.tr.2800e357909a",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1154,WARLORD_NATIVE_PARAMETERS_1154,WARLORD_NATIVE_SWITCHES_1154},
    {1155u,"effect.ue3.warlord-1155-native.v1","fx_m_mi_o_00.fx_m.fx_o_pa_cd_02_tr","fx_m_mi_o_00.fx_m.fx_o_pa_cd_02_tr","ue3.material.fx.m.mi.o.00.fx.m.fx.o.pa.cd.02.tr.dfef07c1c02a",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1155,WARLORD_NATIVE_PARAMETERS_1155,WARLORD_NATIVE_SWITCHES_1155},
    {1156u,"effect.ue3.warlord-1156-native.v1","fx_m_mi_d_00.fx_mi.fx_d_pa_ringmaster_01_216_dt_ad","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.ad.f754ad06cfe6",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1156,WARLORD_NATIVE_PARAMETERS_1156,WARLORD_NATIVE_SWITCHES_1156},
    {1157u,"effect.ue3.warlord-1157-native.v1","fx_m_mi_01.fx_mi.fx_k_me_fd_01_tr_ts","fx_mastermaterial.fx_mm.fx_mm_fluid_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.fluid.01.tr.99f00cf3e57f",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1157,WARLORD_NATIVE_PARAMETERS_1157,WARLORD_NATIVE_SWITCHES_1157},
    {1158u,"effect.ue3.warlord-1158-native.v1","fx_m_mi_w_00.mi.fx_w_pa_master_01_132_dt_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",false,"sprite",false,true,true,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1158,WARLORD_NATIVE_PARAMETERS_1158,WARLORD_NATIVE_SWITCHES_1158},
    {1159u,"effect.ue3.warlord-1159-native.v1","fx_m_mi_02.fx_mi.fx_k_pa_distort_multi_15_ad","fx_m_mi_02.fx_m.fx_k_pa_distort_multi_01_ad","ue3.material.fx.m.mi.02.fx.m.fx.k.pa.distort.multi.01.ad.e74cada0a4fe",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1159,WARLORD_NATIVE_PARAMETERS_1159,WARLORD_NATIVE_SWITCHES_1159},
    {1160u,"effect.ue3.warlord-1160-native.v1","fx_m_mi_05.fx_m.fx_b_pa_smoke_03_tr","fx_m_mi_05.fx_m.fx_b_pa_smoke_03_tr","ue3.material.fx.m.mi.05.fx.m.fx.b.pa.smoke.03.tr.01821ccdf0b2",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1160,WARLORD_NATIVE_PARAMETERS_1160,WARLORD_NATIVE_SWITCHES_1160},
    {1161u,"effect.ue3.warlord-1161-native.v1","fx_m_mi_00.fx_mi.fx_d_me_master_01_008_ts_dt_ds_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",true,"mesh",false,true,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1161,WARLORD_NATIVE_PARAMETERS_1161,WARLORD_NATIVE_SWITCHES_1161},
    {1162u,"effect.ue3.warlord-1162-native.v1","fx_m_mi_00.fx_mi.fx_d_me_master_01_038_ts_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",true,"mesh",false,true,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1162,WARLORD_NATIVE_PARAMETERS_1162,WARLORD_NATIVE_SWITCHES_1162},
    {1163u,"effect.ue3.warlord-1163-native.v1","fx_m_mi_o_00.fx_mi.fx_o_me_fd_01_3_ts_tr","fx_mastermaterial.fx_mm.fx_mm_fluid_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.fluid.01.tr.99f00cf3e57f",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1163,WARLORD_NATIVE_PARAMETERS_1163,WARLORD_NATIVE_SWITCHES_1163},
    {1164u,"effect.ue3.warlord-1164-native.v1","fx_m_mi_00.fx_mi.fx_d_me_master_01_075_ts_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1164,WARLORD_NATIVE_PARAMETERS_1164,WARLORD_NATIVE_SWITCHES_1164},
    {1165u,"effect.ue3.warlord-1165-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_master_01_035_dt_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1165,WARLORD_NATIVE_PARAMETERS_1165,WARLORD_NATIVE_SWITCHES_1165},
    {1166u,"effect.ue3.warlord-1166-native.v1","fx_m_mi_w_00.mi.fx_w_pa_et_07_2_tr","fx_m_mi_05.fx_m.fx_e_pa_electric_01_tr","ue3.material.fx.m.mi.05.fx.m.fx.e.pa.electric.01.tr.8bf9fdfbb137",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1166,WARLORD_NATIVE_PARAMETERS_1166,WARLORD_NATIVE_SWITCHES_1166},
    {1167u,"effect.ue3.warlord-1167-native.v1","fx_m_mi_m_00.fx_mi.fx_m_pa_shine_02_04_ad","fx_m_mi_02.fx_m.fx_d_pa_shine_02_ad","ue3.material.fx.m.mi.02.fx.m.fx.d.pa.shine.02.ad.29d7864aadba",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1167,WARLORD_NATIVE_PARAMETERS_1167,WARLORD_NATIVE_SWITCHES_1167},
    {1168u,"effect.ue3.warlord-1168-native.v1","fx_m_mi_m_00.fx_mi.fx_m_pa_shine_04_3_tr","fx_m_mi_02.fx_m.fx_f_pa_shine_01_0_tr","ue3.material.fx.m.mi.02.fx.m.fx.f.pa.shine.01.0.tr.3d21bd1f3e79",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1168,WARLORD_NATIVE_PARAMETERS_1168,WARLORD_NATIVE_SWITCHES_1168},
    {1169u,"effect.ue3.warlord-1169-native.v1","fx_m_mi_d_00.fx_mi.fx_d_pa_master_01_225_dt_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1169,WARLORD_NATIVE_PARAMETERS_1169,WARLORD_NATIVE_SWITCHES_1169},
    {1170u,"effect.ue3.warlord-1170-native.v1","fx_m_mi_l_00.fx_mi.fx_l_pa_shine_04_3_tr","fx_m_mi_02.fx_m.fx_f_pa_shine_01_0_tr","ue3.material.fx.m.mi.02.fx.m.fx.f.pa.shine.01.0.tr.3d21bd1f3e79",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1170,WARLORD_NATIVE_PARAMETERS_1170,WARLORD_NATIVE_SWITCHES_1170},
    {1171u,"effect.ue3.warlord-1171-native.v1","fx_m_mi_01.fx_mi.fx_l_pa_ap_23_4_ad","bfx_m_mi_00.bfx_m.bfx_d_pa_shine_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.shine.01.ad.d944bc82b616",false,"sprite",false,true,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1171,WARLORD_NATIVE_PARAMETERS_1171,WARLORD_NATIVE_SWITCHES_1171},
    {1172u,"effect.ue3.warlord-1172-native.v1","fx_m_mi_w_00.mi.fx_w_pa_master_01_48_dt_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",false,"sprite",false,true,true,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1172,WARLORD_NATIVE_PARAMETERS_1172,WARLORD_NATIVE_SWITCHES_1172},
    {1173u,"effect.ue3.warlord-1173-native.v1","fx_m_mi_02.fx_mi.fx_j_me_shine_02_1_ad","fx_m_mi_02.fx_m.fx_d_pa_shine_02_ad","ue3.material.fx.m.mi.02.fx.m.fx.d.pa.shine.02.ad.29d7864aadba",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1173,WARLORD_NATIVE_PARAMETERS_1173,WARLORD_NATIVE_SWITCHES_1173},
    {1174u,"effect.ue3.warlord-1174-native.v1","fx_m_mi_01.fx_mi.fx_j_me_flame_02_ad","fx_m_mi_02.fx_m.fx_d_pa_shine_02_ad","ue3.material.fx.m.mi.02.fx.m.fx.d.pa.shine.02.ad.29d7864aadba",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1174,WARLORD_NATIVE_PARAMETERS_1174,WARLORD_NATIVE_SWITCHES_1174},
    {1175u,"effect.ue3.warlord-1175-native.v1","fx_m_mi_03.fx_mi.fx_k_pa_ring_01_ad_2s","fx_m_mi_03.fx_m.fx_d_pa_ring_07_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.ad.82e9116584b2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1175,WARLORD_NATIVE_PARAMETERS_1175,WARLORD_NATIVE_SWITCHES_1175},
    {1176u,"effect.ue3.warlord-1176-native.v1","fx_m_mi_02.fx_mi.fx_m_pa_shine_02_1_ad","fx_m_mi_02.fx_m.fx_d_pa_shine_02_ad","ue3.material.fx.m.mi.02.fx.m.fx.d.pa.shine.02.ad.29d7864aadba",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1176,WARLORD_NATIVE_PARAMETERS_1176,WARLORD_NATIVE_SWITCHES_1176},
    {1177u,"effect.ue3.warlord-1177-native.v1","fx_m_mi_d_00.fx_mi.fx_d_pa_shine_02_04_dt_tr","fx_m_mi_d_00.fx_m.fx_d_pa_shine_02_tr","ue3.material.fx.m.mi.d.00.fx.m.fx.d.pa.shine.02.tr.e9f4194f4a0b",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1177,WARLORD_NATIVE_PARAMETERS_1177,WARLORD_NATIVE_SWITCHES_1177},
    {1178u,"effect.ue3.warlord-1178-native.v1","fx_m_mi_01.fx_mi.fx_j_me_floorstrm_02_2_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1178,WARLORD_NATIVE_PARAMETERS_1178,WARLORD_NATIVE_SWITCHES_1178},
    {1179u,"effect.ue3.warlord-1179-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_ring_07_77_ad","fx_m_mi_03.fx_m.fx_d_pa_ring_07_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.ad.82e9116584b2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1179,WARLORD_NATIVE_PARAMETERS_1179,WARLORD_NATIVE_SWITCHES_1179},
    {1180u,"effect.ue3.warlord-1180-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_twirl_01_02_dt_ad","fx_m_mi_03.fx_m.fx_d_pa_twirl_05_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.twirl.05.ad.2b8e151c3da1",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1180,WARLORD_NATIVE_PARAMETERS_1180,WARLORD_NATIVE_SWITCHES_1180},
    {1181u,"effect.ue3.warlord-1181-native.v1","fx_m_mi_o_00.fx_mi.fx_o_pa_spritewave_03_04_tr","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1181,WARLORD_NATIVE_PARAMETERS_1181,WARLORD_NATIVE_SWITCHES_1181},
    {1182u,"effect.ue3.warlord-1182-native.v1","fx_m_mi_o_00.fx_mi.fx_o_me_flowtrail_01_14_tr_ts","fx_m_mi_02.fx_m.fx_k_me_flowtrail_01_ts_tr","ue3.material.fx.m.mi.02.fx.m.fx.k.me.flowtrail.01.ts.tr.bc0628267aaa",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1182,WARLORD_NATIVE_PARAMETERS_1182,WARLORD_NATIVE_SWITCHES_1182},
    {1183u,"effect.ue3.warlord-1183-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_ht_15_1_tr","fx_mastermaterial.fx_mm.fx_mm_dissolve_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.dissolve.01.tr.a799c9636783",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1183,WARLORD_NATIVE_PARAMETERS_1183,WARLORD_NATIVE_SWITCHES_1183},
    {1184u,"effect.ue3.warlord-1184-native.v1","fx_m_mi_m_00.fx_mi.fx_m_me_watertrail_01_46_tr","fx_m_mi_03.fx_m.fx_m_me_watertrail_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.m.me.watertrail.01.tr.afa4aeba0c50",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1184,WARLORD_NATIVE_PARAMETERS_1184,WARLORD_NATIVE_SWITCHES_1184},
    {1185u,"effect.ue3.warlord-1185-native.v1","fx_m_mi_o_00.fx_mi.fx_o_pa_makeflow_02_12_tr","fx_m_mi_k_00.fx_m.fx_k_pa_makeflow_02_tr","ue3.material.fx.m.mi.k.00.fx.m.fx.k.pa.makeflow.02.tr.59985f2235b2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1185,WARLORD_NATIVE_PARAMETERS_1185,WARLORD_NATIVE_SWITCHES_1185},
    {1186u,"effect.ue3.warlord-1186-native.v1","fx_m_mi_o_00.fx_mi.fx_o_pa_makeflow_01_05_tr","fx_m_mi_k_00.fx_m.fx_k_pa_makeflow_01_tr","ue3.material.fx.m.mi.k.00.fx.m.fx.k.pa.makeflow.01.tr.13159d5d398d",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1186,WARLORD_NATIVE_PARAMETERS_1186,WARLORD_NATIVE_SWITCHES_1186},
    {1187u,"effect.ue3.warlord-1187-native.v1","fx_m_mi_03.fx_m.fx_c_pa_ring_06_ad","fx_m_mi_03.fx_m.fx_c_pa_ring_06_ad","ue3.material.fx.m.mi.03.fx.m.fx.c.pa.ring.06.ad.b7c6a0ecb8c1",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1187,WARLORD_NATIVE_PARAMETERS_1187,WARLORD_NATIVE_SWITCHES_1187},
    {1188u,"effect.ue3.warlord-1188-native.v1","fx_m_mi_o_00.fx_mi.fx_o_me_makeflow_02_22_tr","fx_m_mi_02.fx_m.fx_k_me_makeflow_02_tr","ue3.material.fx.m.mi.02.fx.m.fx.k.me.makeflow.02.tr.5059859991f8",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1188,WARLORD_NATIVE_PARAMETERS_1188,WARLORD_NATIVE_SWITCHES_1188},
    {1189u,"effect.ue3.warlord-1189-native.v1","fx_m_mi_01.fx_mi.fx_i_flare_01_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1189,WARLORD_NATIVE_PARAMETERS_1189,WARLORD_NATIVE_SWITCHES_1189},
    {1190u,"effect.ue3.warlord-1190-native.v1","fx_m_mi_00.fx_mi.fx_c_pa_lensflare_01_09_ad","fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.c.pa.lensflare.01.ad.2cdc706962af",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1190,WARLORD_NATIVE_PARAMETERS_1190,WARLORD_NATIVE_SWITCHES_1190},
    {1191u,"effect.ue3.warlord-1191-native.v1","bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad","bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.glow.01.ad.0857e02620a0",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1191,WARLORD_NATIVE_PARAMETERS_1191,WARLORD_NATIVE_SWITCHES_1191},
    {1192u,"effect.ue3.warlord-1192-native.v1","fx_m_mi_k_00.fx_mi.fx_k_pa_makeflow_01_05_tr","fx_m_mi_k_00.fx_m.fx_k_pa_makeflow_01_tr","ue3.material.fx.m.mi.k.00.fx.m.fx.k.pa.makeflow.01.tr.13159d5d398d",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1192,WARLORD_NATIVE_PARAMETERS_1192,WARLORD_NATIVE_SWITCHES_1192},
    {1193u,"effect.ue3.warlord-1193-native.v1","fx_m_mi_01.fx_mi.fx_j_pa_lbeam_01_1_ad","bfx_m_mi_00.bfx_m.bfx_d_pa_shine_02_ad_dt","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.shine.02.ad.dt.0b7f7fdf1697",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1193,WARLORD_NATIVE_PARAMETERS_1193,WARLORD_NATIVE_SWITCHES_1193},
    {1194u,"effect.ue3.warlord-1194-native.v1","fx_m_mi_o_00.fx_mi.fx_o_pa_beam_01_01_ad","fx_m_mi_o_00.fx_m.fx_o_pa_beam_02_ad","ue3.material.fx.m.mi.o.00.fx.m.fx.o.pa.beam.02.ad.d352e3c17173",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1194,WARLORD_NATIVE_PARAMETERS_1194,WARLORD_NATIVE_SWITCHES_1194},
    {1195u,"effect.ue3.warlord-1195-native.v1","fx_m_mi_00.fx_mi.fx_c_pa_lensflare_01_10_dt5_ad","fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.c.pa.lensflare.01.ad.2cdc706962af",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1195,WARLORD_NATIVE_PARAMETERS_1195,WARLORD_NATIVE_SWITCHES_1195},
    {1196u,"effect.ue3.warlord-1196-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_fd_07_3_ad","fx_m_mi_01.fx_m.fx_e_pa_twinkle_01_ad","ue3.material.fx.m.mi.01.fx.m.fx.e.pa.twinkle.01.ad.72979792ae2b",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1196,WARLORD_NATIVE_PARAMETERS_1196,WARLORD_NATIVE_SWITCHES_1196},
    {1197u,"effect.ue3.warlord-1197-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_gl_01_1_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1197,WARLORD_NATIVE_PARAMETERS_1197,WARLORD_NATIVE_SWITCHES_1197},
    {1198u,"effect.ue3.warlord-1198-native.v1","fx_m_mi_03.fx_mi.fx_m_pa_worldoffset_02_38_tr","fx_m_mi_03.fx_mi.fx_m_pa_worldoffset_02_tr","ue3.material.fx.m.mi.03.fx.mi.fx.m.pa.worldoffset.02.tr.6507c4b13a9b",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1198,WARLORD_NATIVE_PARAMETERS_1198,WARLORD_NATIVE_SWITCHES_1198},
    {1199u,"effect.ue3.warlord-1199-native.v1","fx_m_mi_01.fx_mi.fx_m_pa_wave_01_2_ad","bfx_m_mi_00.bfx_m.bfx_i_pa_wave_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.wave.01.ad.517585fdd7f7",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_1199,WARLORD_NATIVE_PARAMETERS_1199,WARLORD_NATIVE_SWITCHES_1199},
    {2000u,"effect.ue3.warlord-2000-native.v1","fx_m_mi_02.fx_m.fx_j_me_gaurdmode_01_ad","fx_m_mi_02.fx_m.fx_j_me_gaurdmode_01_ad","ue3.material.fx.m.mi.02.fx.m.fx.j.me.gaurdmode.01.ad.7e55e7756ff2",true,"mesh",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_2000,WARLORD_NATIVE_PARAMETERS_2000,WARLORD_NATIVE_SWITCHES_2000},
    {2001u,"effect.ue3.warlord-2001-native.v1","fx_m_mi_o_00.fx_mi.fx_o_me_floorstrm_03_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,true,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_2001,WARLORD_NATIVE_PARAMETERS_2001,WARLORD_NATIVE_SWITCHES_2001},
    {2002u,"effect.ue3.warlord-2002-native.v1","fx_m_mi_05.fx_mi.fx_e_pa_gl_07_1_ad","fx_m_mi_05.fx_m.fx_e_pa_mask_01_ad","ue3.material.fx.m.mi.05.fx.m.fx.e.pa.mask.01.ad.f1d8ba435442",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_2002,WARLORD_NATIVE_PARAMETERS_2002,WARLORD_NATIVE_SWITCHES_2002},
    {2003u,"effect.ue3.warlord-2003-native.v1","fx_m_mi_o_00.fx_mi.fx_o_me_floorstrm_20_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_2003,WARLORD_NATIVE_PARAMETERS_2003,WARLORD_NATIVE_SWITCHES_2003},
    {2004u,"effect.ue3.warlord-2004-native.v1","fx_m_mi_y_00.fx_m.fx_y_pa_timework_01_tr","fx_m_mi_y_00.fx_m.fx_y_pa_timework_01_tr","ue3.material.fx.m.mi.y.00.fx.m.fx.y.pa.timework.01.tr.f3d0be3fc016",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_2004,WARLORD_NATIVE_PARAMETERS_2004,WARLORD_NATIVE_SWITCHES_2004},
}};

inline const WARLORD_NATIVE_PROGRAM_DESC* Find_WarlordNativeProgram(
    const std::string_view runtimeId)
{
    for (const auto& Program : WARLORD_NATIVE_PROGRAMS)
        if (Program.strRuntimeProfileId == runtimeId) return &Program;
    return nullptr;
}

inline bool Build_WarlordNativeParameters(const EFFECT_SOURCE_MATERIAL_DESC& Source,
    std::array<float4_t,32>& Output)
{
    const auto* Program=Find_WarlordNativeProgram(Source.strRuntimeShaderProfileId);
    if (!Program || !Source.bEnabled || Source.strProfileId!=Program->strProfileId ||
        Source.strParentMaterialPath!=Program->strParentMaterialPath) return false;
    std::array<float4_t,32> Candidate{};
    const auto vectorCount=std::count_if(Program->Parameters.begin(),Program->Parameters.end(),
        [](const auto& P){return P.bVector;});
    if (Source.Vectors.size()!=static_cast<std::size_t>(vectorCount) ||
        Source.Scalars.size()!=Program->Parameters.size()-static_cast<std::size_t>(vectorCount)) return false;
    for (const auto& P : Program->Parameters)
    {
        if (P.bVector)
        {
            const auto count=std::count_if(Source.Vectors.begin(),Source.Vectors.end(),
                [&](const auto& V){return V.strName==P.strName;});
            if (count!=1) return false;
            const auto it=std::find_if(Source.Vectors.begin(),Source.Vectors.end(),
                [&](const auto& V){return V.strName==P.strName;});
            const auto& V=it->vValue;
            if (!std::isfinite(V.x)||!std::isfinite(V.y)||!std::isfinite(V.z)||!std::isfinite(V.w)) return false;
            Candidate[P.iRow]=V;
        }
        else
        {
            const auto count=std::count_if(Source.Scalars.begin(),Source.Scalars.end(),
                [&](const auto& V){return V.strName==P.strName;});
            if (count!=1) return false;
            const auto it=std::find_if(Source.Scalars.begin(),Source.Scalars.end(),
                [&](const auto& V){return V.strName==P.strName;});
            if (!std::isfinite(it->fValue)) return false;
            auto& V=Candidate[P.iRow];
            if (P.iLane==0u) V.x=it->fValue;
            else if (P.iLane==1u) V.y=it->fValue;
            else if (P.iLane==2u) V.z=it->fValue;
            else V.w=it->fValue;
        }
    }
    Output=Candidate;
    return true;
}

inline bool Has_WarlordNativeMaterialContract(const EFFECT_ELEMENT_DESC& Element)
{
    const auto& Source=Element.Material.SourceMaterial;
    const auto* Program=Find_WarlordNativeProgram(Source.strRuntimeShaderProfileId);
    if (!Program || Element.Material.Execution.bEnabled ||
        Element.Material.eRenderProfile!=Program->eRenderProfile ||
        Element.eKind!=(Program->strRendererShape=="screenPost" ? EFFECT_ELEMENT_KIND::SCREEN_POST : EFFECT_ELEMENT_KIND::PARTICLE) || !Element.SourceRecipe.bEnabled ||
        Element.Material.strSourceMaterialPath!=Program->strSourceMaterialPath ||
        Element.SourceRecipe.strRendererShape!=Program->strRendererShape) return false;
    const auto meshCount=std::count_if(Element.ResourceBindings.begin(),Element.ResourceBindings.end(),
        [](const auto& B){return B.strSlotId=="meshModel"&&!B.strAssetId.empty();});
    if (meshCount!=(Program->bMesh ? 1 : 0)) return false;
    // These vertex programs consume the recovered native COLOR stream and bounds.
    if (Program->iProfileIndex==418u || Program->iProfileIndex==446u)
    {
        const std::string_view asset=Program->iProfileIndex==418u ?
            "Effect/Warlord/FullRestore/Meshes/fm_d_berchain_06.wmodel" :
            "Effect/Warlord/FullRestore/Meshes/fm_d_electric_05_vertexcolor.wmodel";
        if (std::count_if(Element.ResourceBindings.begin(),Element.ResourceBindings.end(),
            [&](const auto& B){return B.strSlotId=="meshModel"&&B.strAssetId==asset;})!=1) return false;
    }
    if (Source.Textures.size()!=Program->TextureNames.size() ||
        Source.StaticSwitches.size()!=Program->StaticSwitches.size()) return false;
    for (const auto name : Program->TextureNames)
    {
        if (std::count_if(Source.Textures.begin(),Source.Textures.end(),[&](const auto& T){
            return T.strName==name&&!T.strAssetId.empty()&&!T.strSourceObjectPath.empty();})!=1) return false;
    }
    for (const auto& S : Program->StaticSwitches)
    {
        if (std::count_if(Source.StaticSwitches.begin(),Source.StaticSwitches.end(),
            [&](const auto& V){return V.strName==S.strName&&V.bValue==S.bValue;})!=1) return false;
    }
    std::array<float4_t,32> Parameters{};
    return Build_WarlordNativeParameters(Source,Parameters);
}
NS_END
