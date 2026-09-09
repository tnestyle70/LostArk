#pragma once
// Explicit selected S_D unlit material branches. Engine prefix/fog/depth adapters
// are in Shader_EffectDimensionMasterSDNative.hlsli, not unnamed JSON fallbacks.
#include "Effect_AuthoringDocument.h"
#include <array>
#include <span>
#include <string_view>
#include <algorithm>
#include <cmath>

NS_BEGIN(Client)
struct DIMENSIONMASTER_SD_PARAMETER_DESC final
{
    std::string_view strName;
    uint32_t iRow;
    uint32_t iLane;
    bool bVector;
};
struct DIMENSIONMASTER_SD_SWITCH_DESC final
{
    std::string_view strName;
    bool bValue;
};
struct DIMENSIONMASTER_SD_PROGRAM_DESC final
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
    EFFECT_RENDER_PROFILE eRenderProfile;
    std::span<const std::string_view> TextureNames;
    std::span<const DIMENSIONMASTER_SD_PARAMETER_DESC> Parameters;
    std::span<const DIMENSIONMASTER_SD_SWITCH_DESC> StaticSwitches;
};

inline constexpr std::array<std::string_view,0> DIMENSIONMASTER_SD_TEXTURES_320 = {{}};
inline constexpr std::array<DIMENSIONMASTER_SD_PARAMETER_DESC,10> DIMENSIONMASTER_SD_PARAMETERS_320 = {{
    {"selectioncolor", 3u, 0u, true},
    {"mask_color", 2u, 0u, true},
    {"cricle_rotation", 0u, 0u, false},
    {"uv_scale_x", 1u, 2u, false},
    {"uv_scale_y", 1u, 3u, false},
    {"uv_position_x", 1u, 0u, false},
    {"uv_position_y", 1u, 1u, false},
    {"mask_hardness", 0u, 1u, false},
    {"mask_power", 0u, 3u, false},
    {"mask_intensity", 0u, 2u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_SD_SWITCH_DESC,1> DIMENSIONMASTER_SD_SWITCHES_320 = {{
    {"use_inverse", false},
}};

inline constexpr std::array<std::string_view,4> DIMENSIONMASTER_SD_TEXTURES_321 = {{"maintex","uv_noise_tex","emissivetex02","dissolve_tex_01"}};
inline constexpr std::array<DIMENSIONMASTER_SD_PARAMETER_DESC,38> DIMENSIONMASTER_SD_PARAMETERS_321 = {{
    {"selectioncolor", 10u, 0u, true},
    {"uvnoise_move_x", 8u, 2u, false},
    {"uvnoise_move_y", 8u, 3u, false},
    {"maintex_move_x", 4u, 3u, false},
    {"maintex_move_y", 5u, 0u, false},
    {"maintex_rotator", 5u, 3u, false},
    {"dissolvetex_rotator", 1u, 3u, false},
    {"edge_color", 9u, 0u, true},
    {"maintex_panspeed_x", 5u, 1u, false},
    {"maintex_tile_x", 6u, 0u, false},
    {"maintex_tile_y", 6u, 1u, false},
    {"maintex_panspeed_y", 5u, 2u, false},
    {"maintex_dynamicpan_x_velue", 4u, 1u, false},
    {"maintex_dynamicpan_y_velue", 4u, 2u, false},
    {"uv_noisetex_pan_x", 7u, 2u, false},
    {"uv_noisetex_tile_x", 8u, 0u, false},
    {"uv_noisetex_tile_y", 8u, 1u, false},
    {"uv_noisetex_pan_y", 7u, 3u, false},
    {"uv_noise_velue", 7u, 1u, false},
    {"emissivetex02_panspeed_x", 3u, 0u, false},
    {"emissivetex02_tile_x", 3u, 2u, false},
    {"emissivetex02_tile_y", 3u, 3u, false},
    {"emissivetex02_panspeed_y", 3u, 1u, false},
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
    {"spheremask_strength", 6u, 2u, false},
    {"spheremask_strength_max", 6u, 3u, false},
    {"spheremask_strength_min", 7u, 0u, false},
    {"maintex_alpha_strength", 4u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_SD_SWITCH_DESC,20> DIMENSIONMASTER_SD_SWITCHES_321 = {{
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
    {"use_emissivetex02", true},
}};

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_SD_TEXTURES_322 = {{"maintex","uv_noise_tex","dissolve_tex_01"}};
inline constexpr std::array<DIMENSIONMASTER_SD_PARAMETER_DESC,34> DIMENSIONMASTER_SD_PARAMETERS_322 = {{
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
inline constexpr std::array<DIMENSIONMASTER_SD_SWITCH_DESC,20> DIMENSIONMASTER_SD_SWITCHES_322 = {{
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

inline constexpr std::array<std::string_view,2> DIMENSIONMASTER_SD_TEXTURES_323 = {{"01.map_e","06.map_f"}};
inline constexpr std::array<DIMENSIONMASTER_SD_PARAMETER_DESC,22> DIMENSIONMASTER_SD_PARAMETERS_323 = {{
    {"selectioncolor", 6u, 0u, true},
    {"16.emissiion_str", 5u, 0u, true},
    {"time", 4u, 3u, false},
    {"04.map_e_panning_x", 1u, 2u, false},
    {"02.map_e_uvscale_r", 1u, 0u, false},
    {"03.map_e_uvscale_g", 1u, 1u, false},
    {"01.amount", 0u, 0u, false},
    {"11.curvature", 3u, 1u, false},
    {"05.map_e_panning_y", 1u, 3u, false},
    {"09.map_f_panning_x", 2u, 2u, false},
    {"07.map_f_uvscale_r", 2u, 0u, false},
    {"08.map_f_uvscale_g", 2u, 1u, false},
    {"10.map_f_panning_y", 2u, 3u, false},
    {"12.desaturation", 3u, 3u, false},
    {"15.emissiion_power", 4u, 0u, false},
    {"21.noise_str", 4u, 1u, false},
    {"11.centermask_str", 3u, 0u, false},
    {"12.centermask_power", 3u, 2u, false},
    {"01.circle_radius", 0u, 1u, false},
    {"02.circle_hardness", 0u, 3u, false},
    {"22.noise_power", 4u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 2u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_SD_SWITCH_DESC,10> DIMENSIONMASTER_SD_SWITCHES_323 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
    {"40.usefresnal", false},
    {"00.use_uvdistort", false},
    {"20.useaddnoise", true},
    {"00.usecentermask", true},
    {"00.use_uvdistort_alpha", false},
    {"20.usecolorvari", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.uselight", false},
}};


inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_SD_TEXTURES_324 = {{"01.icemap","01.specmap","native_texture_2"}};
inline constexpr std::array<DIMENSIONMASTER_SD_PARAMETER_DESC,15> DIMENSIONMASTER_SD_PARAMETERS_324 = {{
    {"selectioncolor", 6u, 0u, true},
    {"05.specmap_uvscale.x", 1u, 1u, false},
    {"06.specmap_uvscale.y", 1u, 2u, false},
    {"02.icemap_uv.x", 0u, 0u, false},
    {"03.icemap_uv.y", 0u, 2u, false},
    {"09.specmap_color", 4u, 0u, true},
    {"06.icemap_color", 3u, 0u, true},
    {"meshemitterdynamicparameter", 5u, 0u, true},
    {"02.specmap_str", 0u, 1u, false},
    {"07.desaturation", 1u, 3u, false},
    {"08.specmap_power", 2u, 0u, false},
    {"09.ice_deep", 2u, 1u, false},
    {"04.desaturation", 0u, 3u, false},
    {"05.icemap_power", 1u, 0u, false},
    {"51.fresnal_power", 2u, 2u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_SD_SWITCH_DESC,6> DIMENSIONMASTER_SD_SWITCHES_324 = {{
    {"00.usedistortion", false},
    {"00.checkisdepthbiasalpha", false},
    {"10.usesnow", false},
    {"00.useshadow", false},
    {"--usespecullar", true},
    {"00.useemission", false},
}};


inline constexpr std::array<std::string_view,4> DIMENSIONMASTER_SD_TEXTURES_325 = {{"06.map","02.map_e","01.map_a","21.map_c"}};
inline constexpr std::array<DIMENSIONMASTER_SD_PARAMETER_DESC,25> DIMENSIONMASTER_SD_PARAMETERS_325 = {{
    {"selectioncolor", 8u, 0u, true},
    {"93.emissiion_color", 6u, 0u, true},
    {"meshemitterdynamicparameter", 7u, 0u, true},
    {"21.uvscale.x", 3u, 1u, false},
    {"22.uvscale.y", 3u, 2u, false},
    {"time", 5u, 1u, false},
    {"05.map_e_panning_x", 1u, 3u, false},
    {"03.map_e_uvscale_r", 0u, 2u, false},
    {"04.map_e_uvscale_g", 1u, 0u, false},
    {"09.map_d_panning_x", 2u, 3u, false},
    {"07.map_d_uvscale_r", 2u, 1u, false},
    {"08.map_d_uvscale_g", 2u, 2u, false},
    {"10.map_d_panning_y", 3u, 0u, false},
    {"05.distort_str", 1u, 1u, false},
    {"06.map_e_panning_y", 2u, 0u, false},
    {"91.desaturation", 4u, 3u, false},
    {"92.emissiion_power", 5u, 0u, false},
    {"04.map_a_panning_x", 0u, 3u, false},
    {"02.map_a_uvscale_r", 0u, 0u, false},
    {"03.map_a_uvscale_g", 0u, 1u, false},
    {"05.map_a_panning_y", 1u, 2u, false},
    {"29.cmap.direct", 3u, 3u, false},
    {"30.cmap.time(rotrate)", 4u, 0u, false},
    {"36.str", 4u, 1u, false},
    {"37.power", 4u, 2u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_SD_SWITCH_DESC,26> DIMENSIONMASTER_SD_SWITCHES_325 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", true},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", false},
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


inline constexpr std::array<std::string_view,6> DIMENSIONMASTER_SD_TEXTURES_326 = {{"maintex","uv_noise_tex_02","uv_noise_tex","emissivetex02","noisedissolve_tex","dissolve_tex_01"}};
inline constexpr std::array<DIMENSIONMASTER_SD_PARAMETER_DESC,46> DIMENSIONMASTER_SD_PARAMETERS_326 = {{
    {"selectioncolor", 13u, 0u, true},
    {"meshemitterdynamicparameter", 12u, 0u, true},
    {"maintex_rotator", 6u, 1u, false},
    {"maintex_move_x", 5u, 1u, false},
    {"maintex_move_y", 5u, 2u, false},
    {"dissolvetex_rotator", 1u, 3u, false},
    {"edge_color", 11u, 0u, true},
    {"maintex_panspeed_x", 5u, 3u, false},
    {"maintex_tile_x", 6u, 2u, false},
    {"maintex_tile_y", 6u, 3u, false},
    {"maintex_dynamicpan_x_velue", 4u, 3u, false},
    {"maintex_dynamicpan_y_velue", 5u, 0u, false},
    {"maintex_panspeed_y", 6u, 0u, false},
    {"uv_noisetex_pan_x", 9u, 1u, false},
    {"uv_noisetex_tile_x", 10u, 1u, false},
    {"uv_noisetex_tile_y", 10u, 2u, false},
    {"uv_noisetex_pan_02_x", 8u, 3u, false},
    {"uv_noisetex_tile_02_x", 9u, 3u, false},
    {"uv_noisetex_tile_02_y", 10u, 0u, false},
    {"uv_noisetex_pan_02_y", 9u, 0u, false},
    {"uv_noise_02_strength", 8u, 1u, false},
    {"uv_noisetex_pan_y", 9u, 2u, false},
    {"dynamic_uvnoise_x", 2u, 0u, false},
    {"dynamic_uvnoise_y", 2u, 1u, false},
    {"uv_noise_velue", 8u, 2u, false},
    {"emissivetex02_panspeed_x", 3u, 2u, false},
    {"emissivetex02_tile_x", 4u, 0u, false},
    {"emissivetex02_tile_y", 4u, 1u, false},
    {"emissivetex02_panspeed_y", 3u, 3u, false},
    {"emissive_core_power", 3u, 0u, false},
    {"emissive_core_strength", 3u, 1u, false},
    {"emissive_base", 2u, 3u, false},
    {"disslovetex_01_panspeed_x", 0u, 0u, false},
    {"disslovetex_01_tile_x", 0u, 2u, false},
    {"disslovetex_01_tile_y", 0u, 3u, false},
    {"noisetodisslovetex_01_panspeed_x", 7u, 1u, false},
    {"noisetodisslovetex_01_tile_x", 7u, 3u, false},
    {"noisetodisslovetex_01_tile_y", 8u, 0u, false},
    {"noisetodisslovetex_01_panspeed_y", 7u, 2u, false},
    {"noisedissolvetex_strength", 7u, 0u, false},
    {"disslovetex_01_panspeed_y", 0u, 1u, false},
    {"dissolvetex_move_x", 1u, 1u, false},
    {"dissolvetex_move_y", 1u, 2u, false},
    {"dissolve_hardness", 1u, 0u, false},
    {"edge_thin", 2u, 2u, false},
    {"maintex_alpha_strength", 4u, 2u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_SD_SWITCH_DESC,20> DIMENSIONMASTER_SD_SWITCHES_326 = {{
    {"use_dissolve", true},
    {"use_meshtype", true},
    {"use_uvnoise", true},
    {"achannel_use_uvnoisedynamicpan", true},
    {"use_uv_noise_02", true},
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
    {"use_sphere_alpha", false},
    {"dissloveoutline_multyply_alpha", true},
    {"use_emissivetex02", true},
}};


inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_SD_TEXTURES_327 = {{"emissive_tex"}};
inline constexpr std::array<DIMENSIONMASTER_SD_PARAMETER_DESC,8> DIMENSIONMASTER_SD_PARAMETERS_327 = {{
    {"selectioncolor", 3u, 0u, true},
    {"meshemitterdynamicparameter", 2u, 0u, true},
    {"uv_scale", 1u, 1u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 3u, false},
    {"edge_intensity", 0u, 1u, false},
    {"edge_power", 0u, 2u, false},
    {"emissive_power", 1u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_SD_SWITCH_DESC,18> DIMENSIONMASTER_SD_SWITCHES_327 = {{
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


inline constexpr std::array<std::string_view,5> DIMENSIONMASTER_SD_TEXTURES_328 = {{"06.map","02.map_e","12.map_f","01.map_a","21.map_c"}};
inline constexpr std::array<DIMENSIONMASTER_SD_PARAMETER_DESC,31> DIMENSIONMASTER_SD_PARAMETERS_328 = {{
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
    {"15.map_f_panning_x", 3u, 3u, false},
    {"13.map_f_uvscale_r", 3u, 1u, false},
    {"14.map_f_uvscale_g", 3u, 2u, false},
    {"16.map_f_panning_y", 4u, 0u, false},
    {"91.desaturation", 6u, 1u, false},
    {"92.emissiion_power", 6u, 2u, false},
    {"04.map_a_panning_x", 0u, 3u, false},
    {"02.map_a_uvscale_r", 0u, 0u, false},
    {"03.map_a_uvscale_g", 0u, 1u, false},
    {"05.map_a_panning_y", 1u, 2u, false},
    {"29.cmap.direct", 4u, 3u, false},
    {"30.cmap.time(rotrate)", 5u, 0u, false},
    {"36.str", 5u, 3u, false},
    {"37.power", 6u, 0u, false},
    {"32.fresnal_power", 5u, 1u, false},
    {"33.fresnal_str", 5u, 2u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_SD_SWITCH_DESC,26> DIMENSIONMASTER_SD_SWITCHES_328 = {{
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


inline constexpr std::array<std::string_view,4> DIMENSIONMASTER_SD_TEXTURES_329 = {{"06.map","01.map_e","06.map_f","11.edge_map"}};
inline constexpr std::array<DIMENSIONMASTER_SD_PARAMETER_DESC,36> DIMENSIONMASTER_SD_PARAMETERS_329 = {{
    {"selectioncolor", 12u, 0u, true},
    {"18.edge_color", 8u, 0u, true},
    {"meshemitterdynamicparameter", 11u, 0u, true},
    {"00.rotateuv", 0u, 0u, false},
    {"19.emissiion_color", 9u, 0u, true},
    {"20.emissiion_color", 10u, 0u, true},
    {"00.rotateuv.", 0u, 1u, false},
    {"04.map_e_panning_x", 1u, 0u, false},
    {"02.map_e_uvscale_r", 0u, 2u, false},
    {"03.map_e_uvscale_g", 0u, 3u, false},
    {"09.map_d_panning_x", 3u, 2u, false},
    {"07.map_d_uvscale_r", 2u, 0u, false},
    {"08.map_d_uvscale_g", 2u, 2u, false},
    {"10.map_d_panning_y", 4u, 0u, false},
    {"05.distort_str", 1u, 1u, false},
    {"05.map_e_panning_y", 1u, 3u, false},
    {"09.map_f_panning_x", 3u, 3u, false},
    {"07.map_f_uvscale_r", 2u, 1u, false},
    {"08.map_f_uvscale_g", 2u, 3u, false},
    {"10.map_f_panning_y", 4u, 1u, false},
    {"14.map_dd_panning_x", 5u, 3u, false},
    {"12.map_dd_uvscale_r", 5u, 1u, false},
    {"13.map_dd_uvscale_g", 5u, 2u, false},
    {"15.map_dd_panning_y", 6u, 1u, false},
    {"08.phase_str", 3u, 0u, false},
    {"09.gap_hardness", 3u, 1u, false},
    {"10.power", 4u, 2u, false},
    {"16.str", 6u, 3u, false},
    {"17.power", 7u, 0u, false},
    {"12.desaturation", 5u, 0u, false},
    {"15.emissiion_power", 6u, 0u, false},
    {"16.emissiion_str", 6u, 2u, false},
    {"11.str", 4u, 3u, false},
    {"51.power", 7u, 1u, false},
    {"52.str", 7u, 2u, false},
    {"05.distortion_str", 1u, 2u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_SD_SWITCH_DESC,20> DIMENSIONMASTER_SD_SWITCHES_329 = {{
    {"00.usedistortion", true},
    {"--use_outalphamap", true},
    {"01.bothsides_type", true},
    {"03.box_type", false},
    {"00.use_uvdistort", true},
    {"--use_dissolve", true},
    {"01.usemesh", true},
    {"01.liner_type", false},
    {"02.bothsides_type", true},
    {"03.cross_type", false},
    {"04.sweep_type", false},
    {"05.radialtype", true},
    {"06.invert", false},
    {"00.useboundry", false},
    {"02.liner_type", false},
    {"50.use_fresnal", true},
    {"10.useedgeemission", true},
    {"--use_gradient", true},
    {"--use_detailmap", true},
    {"00.checkisdepthbiasalpha", false},
}};


inline constexpr std::array<std::string_view,2> DIMENSIONMASTER_SD_TEXTURES_330 = {{"06.map","01.map_a"}};
inline constexpr std::array<DIMENSIONMASTER_SD_PARAMETER_DESC,18> DIMENSIONMASTER_SD_PARAMETERS_330 = {{
    {"selectioncolor", 5u, 0u, true},
    {"01.rotateuv", 0u, 0u, false},
    {"meshemitterdynamicparameter", 4u, 0u, true},
    {"04.map_a_pantimescale", 1u, 1u, false},
    {"02.map_a_uv.x", 0u, 2u, false},
    {"03.map_a_uv.y", 1u, 0u, false},
    {"10.edgeemission_power", 2u, 2u, false},
    {"11.edgeemission_str", 3u, 0u, false},
    {"09.map_d_panning_x", 2u, 1u, false},
    {"07.map_d_uvscale_r", 1u, 3u, false},
    {"08.map_d_uvscale_g", 2u, 0u, false},
    {"10.map_d_panning_y", 2u, 3u, false},
    {"05.distort_str", 1u, 2u, false},
    {"01.wavecount", 0u, 1u, false},
    {"02.wavestr", 0u, 3u, false},
    {"11.width", 3u, 1u, false},
    {"31.fresnel_str", 3u, 2u, false},
    {"32.fresnel_power", 3u, 3u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_SD_SWITCH_DESC,5> DIMENSIONMASTER_SD_SWITCHES_330 = {{
    {"30.usefresnel", true},
    {"50.useendfalloff", false},
    {"01.usemesh", true},
    {"00.use_uvdistort", true},
    {"00.use_wavetimebezier_in_dp", true},
}};


inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_SD_TEXTURES_331 = {{"maintex","emissivetex02","dissolve_tex_01"}};
inline constexpr std::array<DIMENSIONMASTER_SD_PARAMETER_DESC,30> DIMENSIONMASTER_SD_PARAMETERS_331 = {{
    {"selectioncolor", 9u, 0u, true},
    {"meshemitterdynamicparameter", 8u, 0u, true},
    {"maintex_rotator", 6u, 0u, false},
    {"maintex_move_x", 5u, 0u, false},
    {"maintex_move_y", 5u, 1u, false},
    {"dissolvetex_rotator", 1u, 3u, false},
    {"edge_color", 7u, 0u, true},
    {"maintex_panspeed_x", 5u, 2u, false},
    {"maintex_tile_x", 6u, 1u, false},
    {"maintex_tile_y", 6u, 2u, false},
    {"maintex_dynamicpan_x_velue", 4u, 2u, false},
    {"maintex_dynamicpan_y_velue", 4u, 3u, false},
    {"maintex_panspeed_y", 5u, 3u, false},
    {"emissivetex02_panspeed_x", 3u, 0u, false},
    {"emissivetex02_tile_x", 3u, 2u, false},
    {"emissivetex02_tile_y", 3u, 3u, false},
    {"emissivetex02_panspeed_y", 3u, 1u, false},
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
    {"fresnel_power", 4u, 0u, false},
    {"maintex_alpha_strength", 4u, 1u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_SD_SWITCH_DESC,20> DIMENSIONMASTER_SD_SWITCHES_331 = {{
    {"use_dissolve", true},
    {"use_meshtype", true},
    {"use_uvnoise", false},
    {"achannel_use_uvnoisedynamicpan", false},
    {"use_uv_noise_02", false},
    {"use_uv_noisetex_02", true},
    {"uv_noisetype", true},
    {"maintex_type_polar", false},
    {"maintex_pan_xy", true},
    {"use_dynamic_spheryxy", false},
    {"uv_sphery_y", false},
    {"uv_sphery_x", false},
    {"use_alpha_fresnel", true},
    {"use_dissolvetex_02", false},
    {"use_dissolvetex_noise", false},
    {"use_dissolvetex_noise_type", true},
    {"use_dissolvetex_02_mixtype", true},
    {"use_sphere_alpha", false},
    {"dissloveoutline_multyply_alpha", false},
    {"use_emissivetex02", true},
}};


inline constexpr std::array<std::string_view,4> DIMENSIONMASTER_SD_TEXTURES_332 = {{"maintex","uv_noise_tex","noisedissolve_tex","dissolve_tex_01"}};
inline constexpr std::array<DIMENSIONMASTER_SD_PARAMETER_DESC,42> DIMENSIONMASTER_SD_PARAMETERS_332 = {{
    {"selectioncolor", 12u, 0u, true},
    {"meshemitterdynamicparameter", 11u, 0u, true},
    {"maintex_rotator", 5u, 3u, false},
    {"maintex_move_x", 4u, 3u, false},
    {"maintex_move_y", 5u, 0u, false},
    {"dissolvetex_rotator", 2u, 0u, false},
    {"edge_color", 10u, 0u, true},
    {"maintex_panspeed_x", 5u, 1u, false},
    {"maintex_tile_x", 6u, 0u, false},
    {"maintex_tile_y", 6u, 1u, false},
    {"maintex_dynamicpan_x_velue", 4u, 1u, false},
    {"maintex_dynamicpan_y_velue", 4u, 2u, false},
    {"maintex_panspeed_y", 5u, 2u, false},
    {"uv_noisetex_pan_x", 8u, 3u, false},
    {"uv_noisetex_tile_x", 9u, 1u, false},
    {"uv_noisetex_tile_y", 9u, 2u, false},
    {"uv_noisetex_pan_y", 9u, 0u, false},
    {"dynamic_uvnoise_x", 2u, 1u, false},
    {"dynamic_uvnoise_y", 2u, 2u, false},
    {"uv_noise_velue", 8u, 2u, false},
    {"emissive_core_power", 3u, 1u, false},
    {"emissive_core_strength", 3u, 2u, false},
    {"emissive_base", 3u, 0u, false},
    {"disslovetex_01_panspeed_x", 0u, 0u, false},
    {"disslovetex_01_tile_x", 0u, 2u, false},
    {"disslovetex_01_tile_y", 0u, 3u, false},
    {"noisetodisslovetex_01_panspeed_x", 6u, 3u, false},
    {"noisetodisslovetex_01_tile_x", 7u, 1u, false},
    {"noisetodisslovetex_01_tile_y", 7u, 2u, false},
    {"noisetodisslovetex_01_panspeed_y", 7u, 0u, false},
    {"noisedissolvetex_strength", 6u, 2u, false},
    {"disslovetex_01_panspeed_y", 0u, 1u, false},
    {"dissolvetex_move_x", 1u, 1u, false},
    {"dissolvetex_move_y", 1u, 2u, false},
    {"dissolvetex_power", 1u, 3u, false},
    {"dissolve_hardness", 1u, 0u, false},
    {"edge_thin", 2u, 3u, false},
    {"fresnel_power", 3u, 3u, false},
    {"spheremask_strength", 7u, 3u, false},
    {"spheremask_strength_max", 8u, 0u, false},
    {"spheremask_strength_min", 8u, 1u, false},
    {"maintex_alpha_strength", 4u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_SD_SWITCH_DESC,19> DIMENSIONMASTER_SD_SWITCHES_332 = {{
    {"use_alpha_fresnel", true},
    {"use_meshtype", true},
    {"use_dissolvetex_noise", true},
    {"use_dissolvetex_noise_type", true},
    {"achannel_use_uvnoisedynamicpan", true},
    {"use_uv_noise_02", false},
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

inline constexpr std::array<DIMENSIONMASTER_SD_PROGRAM_DESC,13> DIMENSIONMASTER_SD_PROGRAMS = {{
    {320u,"effect.ue3.sd-320-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_gl_10_1_tr","fx_mastermaterial.fx_mm.fx_mm_maskcontrol_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.maskcontrol.01.tr.30a3b56d23fe",false,"sprite",false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_320,DIMENSIONMASTER_SD_PARAMETERS_320,DIMENSIONMASTER_SD_SWITCHES_320},
    {321u,"effect.ue3.sd-321-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_spritewave_24_02_tr","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",false,"sprite",false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_321,DIMENSIONMASTER_SD_PARAMETERS_321,DIMENSIONMASTER_SD_SWITCHES_321},
    {322u,"effect.ue3.sd-322-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_spritewave_30_01_tr","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",false,"sprite",false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_322,DIMENSIONMASTER_SD_PARAMETERS_322,DIMENSIONMASTER_SD_SWITCHES_322},
    {323u,"effect.ue3.sd-323-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_twirl_03_09_ad","fx_m_mi_03.fx_m.fx_d_pa_twirl_05_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.twirl.05.ad.2b8e151c3da1",false,"sprite",false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_323,DIMENSIONMASTER_SD_PARAMETERS_323,DIMENSIONMASTER_SD_SWITCHES_323},
    {324u,"effect.ue3.sd-324-native.v1","fx_m_mi_00.fx_mi.fx_d_me_ice_01_02_tr","fx_m_mi_00.fx_m.fx_d_me_ice_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.me.ice.01.tr.b5fca778c01f",true,"mesh",false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_324,DIMENSIONMASTER_SD_PARAMETERS_324,DIMENSIONMASTER_SD_SWITCHES_324},
    {325u,"effect.ue3.sd-325-native.v1","fx_m_mi_01.fx_mi.fx_k_me_floorstrm_08_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_325,DIMENSIONMASTER_SD_PARAMETERS_325,DIMENSIONMASTER_SD_SWITCHES_325},
    {326u,"effect.ue3.sd-326-native.v1","fx_m_mi_w_00.mi.fx_w_me_spritewave_01_77_tr","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",true,"mesh",false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_326,DIMENSIONMASTER_SD_PARAMETERS_326,DIMENSIONMASTER_SD_SWITCHES_326},
    {327u,"effect.ue3.sd-327-native.v1","fx_m_mi_m_00.fx_mi.fx_m_me_ap_07_6_ts_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",true,"mesh",false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_327,DIMENSIONMASTER_SD_PARAMETERS_327,DIMENSIONMASTER_SD_SWITCHES_327},
    {328u,"effect.ue3.sd-328-native.v1","fx_m_mi_m_00.fx_mi.fx_m_me_floorstrm_21_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_328,DIMENSIONMASTER_SD_PARAMETERS_328,DIMENSIONMASTER_SD_SWITCHES_328},
    {329u,"effect.ue3.sd-329-native.v1","fx_m_mi_g_00.fx_mi.fx_g_me_field_01_1_tr","fx_m_mi_00.fx_m.fx_d_pa_field_02_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.field.02.tr.5f5c790fded2",true,"mesh",false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_329,DIMENSIONMASTER_SD_PARAMETERS_329,DIMENSIONMASTER_SD_SWITCHES_329},
    {330u,"effect.ue3.sd-330-native.v1","fx_m_mi_00.fx_mi.fx_d_me_electric_03_13_ad","fx_m_mi_00.fx_m.fx_d_pa_electric_03_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.electric.03.ad.781f15e7340d",true,"mesh",false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_330,DIMENSIONMASTER_SD_PARAMETERS_330,DIMENSIONMASTER_SD_SWITCHES_330},
    {331u,"effect.ue3.sd-331-native.v1","fx_m_mi_s_00.fx_mi.fx_s_me_spritewave_03_18_tr","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",true,"mesh",false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_331,DIMENSIONMASTER_SD_PARAMETERS_331,DIMENSIONMASTER_SD_SWITCHES_331},
    {332u,"effect.ue3.sd-332-native.v1","fx_m_mi_m_00.fx_mi.fx_m_me_spritewave_01_8_ad","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_ad","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.ad.caabbddf8b55",true,"mesh",false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_332,DIMENSIONMASTER_SD_PARAMETERS_332,DIMENSIONMASTER_SD_SWITCHES_332},
}};

inline const DIMENSIONMASTER_SD_PROGRAM_DESC* Find_DimensionMasterSDProgram(
    const std::string_view runtimeId)
{
    for (const auto& Program : DIMENSIONMASTER_SD_PROGRAMS)
        if (Program.strRuntimeProfileId == runtimeId) return &Program;
    return nullptr;
}

inline bool Build_DimensionMasterSDParameters(const EFFECT_SOURCE_MATERIAL_DESC& Source,
    std::array<float4_t,32>& Output)
{
    const auto* Program=Find_DimensionMasterSDProgram(Source.strRuntimeShaderProfileId);
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

inline bool Has_DimensionMasterSDMaterialContract(const EFFECT_ELEMENT_DESC& Element)
{
    const auto& Source=Element.Material.SourceMaterial;
    const auto* Program=Find_DimensionMasterSDProgram(Source.strRuntimeShaderProfileId);
    if (!Program || Element.Material.Execution.bEnabled ||
        Element.Material.eRenderProfile!=Program->eRenderProfile ||
        Element.eKind!=EFFECT_ELEMENT_KIND::PARTICLE || !Element.SourceRecipe.bEnabled ||
        Element.Material.strSourceMaterialPath!=Program->strSourceMaterialPath ||
        Element.SourceRecipe.strRendererShape!=Program->strRendererShape) return false;
    const auto meshCount=std::count_if(Element.ResourceBindings.begin(),Element.ResourceBindings.end(),
        [](const auto& B){return B.strSlotId=="meshModel"&&!B.strAssetId.empty();});
    if (meshCount!=(Program->bMesh ? 1 : 0)) return false;
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
    return Build_DimensionMasterSDParameters(Source,Parameters);
}
NS_END
