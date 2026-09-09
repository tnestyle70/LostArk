#pragma once
// Explicit selected W/R unlit material branches. Engine prefix/fog/depth adapters
// are in Shader_EffectDimensionMasterWRNative.hlsli, not unnamed JSON fallbacks.
#include "Effect_AuthoringDocument.h"
#include <array>
#include <span>
#include <string_view>
#include <algorithm>
#include <cmath>

NS_BEGIN(Client)
struct DIMENSIONMASTER_WR_PARAMETER_DESC final
{
    std::string_view strName;
    uint32_t iRow;
    uint32_t iLane;
    bool bVector;
};
struct DIMENSIONMASTER_WR_SWITCH_DESC final
{
    std::string_view strName;
    bool bValue;
};
struct DIMENSIONMASTER_WR_PROGRAM_DESC final
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
    std::span<const DIMENSIONMASTER_WR_PARAMETER_DESC> Parameters;
    std::span<const DIMENSIONMASTER_WR_SWITCH_DESC> StaticSwitches;
};

inline constexpr std::array<std::string_view,2> DIMENSIONMASTER_WR_TEXTURES_208 = {{"flow_texture","alpha_texture"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,17> DIMENSIONMASTER_WR_PARAMETERS_208 = {{
    {"selectioncolor", 4u, 0u, true},
    {"alpha_tileu", 1u, 0u, false},
    {"alpha_tilev", 1u, 1u, false},
    {"alpha_offsetx", 0u, 0u, false},
    {"alpha_offsety", 0u, 1u, false},
    {"flow_tileu", 2u, 3u, false},
    {"flow_tilev", 3u, 0u, false},
    {"flow_panx", 2u, 0u, false},
    {"flow_pany", 2u, 1u, false},
    {"flow_str", 2u, 2u, false},
    {"invert_maxstr", 3u, 1u, false},
    {"alpha_pow", 0u, 2u, false},
    {"alpha_str", 0u, 3u, false},
    {"invert_minstr", 3u, 2u, false},
    {"emissive_pow", 1u, 2u, false},
    {"emissive_str", 1u, 3u, false},
    {"sinetime", 3u, 3u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,2> DIMENSIONMASTER_WR_SWITCHES_208 = {{
    {"use_alphapan", false},
    {"use_sinetime", true},
}};

inline constexpr std::array<std::string_view,0> DIMENSIONMASTER_WR_TEXTURES_209 = {{}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,1> DIMENSIONMASTER_WR_PARAMETERS_209 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,0> DIMENSIONMASTER_WR_SWITCHES_209 = {{
}};

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_WR_TEXTURES_210 = {{"04.map_anew","06.map","00.map_b"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,14> DIMENSIONMASTER_WR_PARAMETERS_210 = {{
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
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,18> DIMENSIONMASTER_WR_SWITCHES_210 = {{
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
    {"40.use_fanshape", false},
}};

inline constexpr std::array<std::string_view,0> DIMENSIONMASTER_WR_TEXTURES_211 = {{}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,6> DIMENSIONMASTER_WR_PARAMETERS_211 = {{
    {"selectioncolor", 2u, 0u, true},
    {"01.radius", 0u, 0u, false},
    {"02.hardness", 0u, 1u, false},
    {"03.spherepower", 0u, 2u, false},
    {"04.spherestr", 0u, 3u, false},
    {"depthbiasdalpha_bias", 1u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,5> DIMENSIONMASTER_WR_SWITCHES_211 = {{
    {"checkisdepthbiasalpha", true},
    {"checkisvertexcolor", true},
    {"01.checkisvertexcolor", true},
    {"02.usedynamicparam", false},
    {"30.usefresnal", false},
}};

inline constexpr std::array<std::string_view,0> DIMENSIONMASTER_WR_TEXTURES_212 = {{}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,6> DIMENSIONMASTER_WR_PARAMETERS_212 = {{
    {"selectioncolor", 2u, 0u, true},
    {"01.radius", 0u, 0u, false},
    {"02.hardness", 0u, 1u, false},
    {"03.spherepower", 0u, 2u, false},
    {"04.sphere_str", 0u, 3u, false},
    {"depthbiasdalpha_bias", 1u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,4> DIMENSIONMASTER_WR_SWITCHES_212 = {{
    {"01.checkisvertexcolor", true},
    {"02.usedynamicparam", false},
    {"30.usefresnal", false},
    {"checkisdepthbiasalpha", true},
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_WR_TEXTURES_213 = {{"slice_flow_texture"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,11> DIMENSIONMASTER_WR_PARAMETERS_213 = {{
    {"selectioncolor", 3u, 0u, true},
    {"slice_flow_rot", 1u, 2u, false},
    {"slice_rot", 2u, 1u, false},
    {"slice_flow_tileu", 1u, 3u, false},
    {"slice_flow_tilev", 2u, 0u, false},
    {"slice_flow_offsetx", 1u, 0u, false},
    {"slice_flow_offsety", 1u, 1u, false},
    {"flow_str", 0u, 2u, false},
    {"depth", 0u, 0u, false},
    {"opacity_radius", 0u, 3u, false},
    {"distortion", 0u, 1u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,0> DIMENSIONMASTER_WR_SWITCHES_213 = {{
}};

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_WR_TEXTURES_214 = {{"normal_tex","refle_tex","native_texture_2"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,17> DIMENSIONMASTER_WR_PARAMETERS_214 = {{
    {"selectioncolor", 7u, 0u, true},
    {"refle_panspeed", 1u, 3u, false},
    {"refle_color", 6u, 0u, true},
    {"meshemitterdynamicparameter", 4u, 0u, true},
    {"in_color", 3u, 0u, true},
    {"out_color", 5u, 0u, true},
    {"normal_tileu", 0u, 2u, false},
    {"normal_tilev", 0u, 3u, false},
    {"refle_vector_divide", 2u, 3u, false},
    {"refle_tileu", 2u, 1u, false},
    {"refle_tilev", 2u, 2u, false},
    {"refle_offsetx", 1u, 1u, false},
    {"refle_offsety", 1u, 2u, false},
    {"refle_desaturation", 1u, 0u, false},
    {"refle_pow", 2u, 0u, false},
    {"fresnel_pow", 0u, 1u, false},
    {"distortion", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,2> DIMENSIONMASTER_WR_SWITCHES_214 = {{
    {"use_depth", false},
    {"use_dissolve", false},
}};

inline constexpr std::array<std::string_view,6> DIMENSIONMASTER_WR_TEXTURES_215 = {{"mask_noisemap","mask_tex_l","mask_tex_r","01.map_e","06.map","06.map_f"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,38> DIMENSIONMASTER_WR_PARAMETERS_215 = {{
    {"selectioncolor", 12u, 0u, true},
    {"02_round_color", 9u, 0u, true},
    {"mask_noise_tile_x", 7u, 1u, false},
    {"mask_noise_tile_y", 7u, 2u, false},
    {"mask_pan_speed", 8u, 1u, false},
    {"21.uv_offset.x", 6u, 2u, false},
    {"22.uv_offset.y", 6u, 3u, false},
    {"19.emission_color", 10u, 0u, true},
    {"20.base_color", 11u, 0u, true},
    {"01_thickness", 0u, 2u, false},
    {"mask_noise_str", 7u, 0u, false},
    {"mask_offset_y", 8u, 0u, false},
    {"mask_offset_x", 7u, 3u, false},
    {"04.map_e_panning_x", 2u, 2u, false},
    {"02.map_e_uvscale_r", 1u, 1u, false},
    {"03.map_e_uvscale_g", 2u, 1u, false},
    {"09.map_d_panning_x", 4u, 3u, false},
    {"07.map_d_uvscale_r", 3u, 3u, false},
    {"08.map_d_uvscale_g", 4u, 1u, false},
    {"01.depth", 0u, 0u, false},
    {"03.circleradius_offset", 1u, 3u, false},
    {"02.curvartuer", 0u, 3u, false},
    {"10.map_d_panning_y", 5u, 1u, false},
    {"05.distort_str", 3u, 0u, false},
    {"05.map_e_panning_y", 3u, 1u, false},
    {"09.map_f_panning_x", 5u, 0u, false},
    {"07.map_f_uvscale_r", 4u, 0u, false},
    {"08.map_f_uvscale_g", 4u, 2u, false},
    {"10.map_f_panning_y", 5u, 2u, false},
    {"12.desaturation", 5u, 3u, false},
    {"04.str", 2u, 3u, false},
    {"15.emissiion_power", 6u, 0u, false},
    {"16.emissiion_str", 6u, 1u, false},
    {"01.power", 0u, 1u, false},
    {"02.str", 1u, 2u, false},
    {"03.innerthickness", 2u, 0u, false},
    {"02.depth", 1u, 0u, false},
    {"05.power", 3u, 2u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,17> DIMENSIONMASTER_WR_SWITCHES_215 = {{
    {"00.use_texmap", false},
    {"03.addphase", false},
    {"10.usebump", false},
    {"01.checkismeshemitvc", false},
    {"--.use_uvdistort", true},
    {"00.foremission", true},
    {"10.usetwirl", false},
    {"11.multifly", true},
    {"06.mapch.r", true},
    {"07.mapch.g", true},
    {"08.mapch.b", true},
    {"00.use_thickness", false},
    {"00.uselight", false},
    {"11.use_addnoise", false},
    {"00.useboundry", false},
    {"00.useshadow", true},
    {"00.useemission", true},
}};

inline constexpr std::array<std::string_view,7> DIMENSIONMASTER_WR_TEXTURES_216 = {{"cracknormal_tex","native_texture_1","in_hole_texture","aura_texture","native_texture_4","native_texture_5","native_texture_6"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,32> DIMENSIONMASTER_WR_PARAMETERS_216 = {{
    {"selectioncolor", 10u, 0u, true},
    {"in_hole_panx", 4u, 0u, false},
    {"in_hole_pany", 4u, 1u, false},
    {"alpha_tile_x", 0u, 2u, false},
    {"alpha_tile_y", 0u, 3u, false},
    {"alpha_offsetx", 0u, 0u, false},
    {"alpha_offsety", 0u, 1u, false},
    {"cracknormal_tile_x", 1u, 3u, false},
    {"cracknormal_tile_y", 2u, 0u, false},
    {"time", 6u, 0u, false},
    {"in_hole_color", 9u, 0u, true},
    {"aura_color", 8u, 0u, true},
    {"distortionpower", 2u, 2u, false},
    {"distortionscale", 2u, 3u, false},
    {"scale", 5u, 3u, false},
    {"cracknormal_str", 1u, 2u, false},
    {"in_hole_crackuv", 3u, 1u, false},
    {"in_hole_height", 3u, 3u, false},
    {"in_hole_pow", 4u, 2u, false},
    {"in_hole_str", 4u, 3u, false},
    {"in_hole_desaturation", 3u, 2u, false},
    {"main_ucoord", 5u, 1u, false},
    {"twist_str", 6u, 1u, false},
    {"main_tex_upanner", 5u, 0u, false},
    {"curve_power", 2u, 1u, false},
    {"main_v_panner", 5u, 2u, false},
    {"uvnoise_utile", 6u, 3u, false},
    {"uvnoise_vtile", 7u, 0u, false},
    {"uvnoise_pan", 6u, 2u, false},
    {"aura_pow", 1u, 0u, false},
    {"aura_str", 1u, 1u, false},
    {"edge_crack_desaturation", 3u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,6> DIMENSIONMASTER_WR_SWITCHES_216 = {{
    {"use_dynamic_vcoord", true},
    {"uvnoise_v_pannertime", true},
    {"main_v_pannertime", false},
    {"main_u_pannertime", false},
    {"use_mesh", false},
    {"use_edgesmooth", false},
}};

inline constexpr std::array<std::string_view,5> DIMENSIONMASTER_WR_TEXTURES_217 = {{"flow_1_tex","flow_2_tex","diff_tex","opacity_tex","mask_tex"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,35> DIMENSIONMASTER_WR_PARAMETERS_217 = {{
    {"selectioncolor", 10u, 0u, true},
    {"color_1", 8u, 0u, true},
    {"color_2", 9u, 0u, true},
    {"diff_u", 1u, 0u, false},
    {"diff_v", 1u, 1u, false},
    {"flow_1_tile_u", 3u, 0u, false},
    {"flow_1_tile_v", 3u, 1u, false},
    {"flow_1_pan_x", 2u, 0u, false},
    {"flow_1_offset_x", 1u, 2u, false},
    {"flow_1_pan_y", 2u, 1u, false},
    {"flow_1_offset_y", 1u, 3u, false},
    {"flow_2_tile_u", 5u, 0u, false},
    {"flow_2_tile_v", 5u, 1u, false},
    {"flow_2_pan_x", 4u, 0u, false},
    {"flow_2_offset_x", 3u, 2u, false},
    {"flow_2_pan_y", 4u, 1u, false},
    {"flow_2_offset_y", 3u, 3u, false},
    {"opacity_u", 7u, 2u, false},
    {"opacity_v", 7u, 3u, false},
    {"mask_u", 6u, 2u, false},
    {"mask_v", 6u, 3u, false},
    {"flow_1_sizecontrol", 2u, 2u, false},
    {"flow_1_str", 2u, 3u, false},
    {"flow_2_sizecontrol", 4u, 2u, false},
    {"flow_2_str", 4u, 3u, false},
    {"desaturation", 0u, 1u, false},
    {"diff_pow", 0u, 2u, false},
    {"diff_str", 0u, 3u, false},
    {"opacity_pow", 7u, 0u, false},
    {"opacity_str", 7u, 1u, false},
    {"mask_noisestr", 5u, 3u, false},
    {"mask_pow", 6u, 0u, false},
    {"mask_str", 6u, 1u, false},
    {"gra_pow", 5u, 2u, false},
    {"depth", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,3> DIMENSIONMASTER_WR_SWITCHES_217 = {{
    {"use_mesh", false},
    {"depthalpha_use", true},
    {"noise_compose", false},
}};

inline constexpr std::array<std::string_view,2> DIMENSIONMASTER_WR_TEXTURES_218 = {{"01.map_a","06.map_b"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,19> DIMENSIONMASTER_WR_PARAMETERS_218 = {{
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
    {"31.noisepower", 4u, 0u, false},
    {"32.noisestr", 4u, 1u, false},
    {"31.fresnal_power", 3u, 3u, false},
    {"11.range", 2u, 3u, false},
    {"12.power", 3u, 1u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,12> DIMENSIONMASTER_WR_SWITCHES_218 = {{
    {"00.checkisdepthbiasalpha", false},
    {"10.usefakedepthbiasalpha", true},
    {"01.useconeuv", false},
    {"30.usefresnal", true},
    {"00.usenoise", true},
    {"00.blendingadd", true},
    {"30.use((map_a+map_b)*0.5)", false},
    {"00.useuvnoise", false},
    {"01.usemesh", false},
    {"00.usecolor", false},
    {"01.usenoise", false},
    {"10.usemapcolor", false},
}};

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_WR_TEXTURES_220 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,19> DIMENSIONMASTER_WR_PARAMETERS_220 = {{
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
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,6> DIMENSIONMASTER_WR_SWITCHES_220 = {{
    {"checkisdepthbiasalpha", true},
    {"00.checkisnoiseon", true},
    {"12.uvnoise", true},
    {"11.uvmirroring", false},
    {"use_meshtype", false},
    {"use_multyply_noisecolor", false},
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_WR_TEXTURES_221 = {{"main_tex"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,5> DIMENSIONMASTER_WR_PARAMETERS_221 = {{
    {"selectioncolor", 1u, 0u, true},
    {"maintex_coordx", 0u, 2u, false},
    {"maintex_coordy", 0u, 3u, false},
    {"bright", 0u, 0u, false},
    {"desaturation", 0u, 1u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,4> DIMENSIONMASTER_WR_SWITCHES_221 = {{
    {"use_5waysplit", false},
    {"use_alphach", false},
    {"use_maintex_paningtile", false},
    {"split_type", true},
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_WR_TEXTURES_222 = {{"native_texture_0"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,7> DIMENSIONMASTER_WR_PARAMETERS_222 = {{
    {"selectioncolor", 2u, 0u, true},
    {"u_scale", 0u, 2u, false},
    {"v_scale", 0u, 3u, false},
    {"wave_uv", 1u, 1u, false},
    {"wave_str", 1u, 0u, false},
    {"main_des", 0u, 0u, false},
    {"main_pow", 0u, 1u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,2> DIMENSIONMASTER_WR_SWITCHES_222 = {{
    {"use_turns", true},
    {"mask_up/down", true},
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_WR_TEXTURES_223 = {{"emissive_tex"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,7> DIMENSIONMASTER_WR_PARAMETERS_223 = {{
    {"selectioncolor", 2u, 0u, true},
    {"uv_rotation_angle", 1u, 0u, false},
    {"uv_scale", 1u, 1u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,17> DIMENSIONMASTER_WR_SWITCHES_223 = {{
    {"use_mesh_world_normal", false},
    {"use_depth_alpha", true},
    {"use_distortion", false},
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_alpha_tex", false},
    {"use_uv_panning", false},
    {"use_uv_rotation", true},
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

inline constexpr std::array<std::string_view,2> DIMENSIONMASTER_WR_TEXTURES_224 = {{"uv_noise_tex","emissive_tex"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,10> DIMENSIONMASTER_WR_PARAMETERS_224 = {{
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
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,5> DIMENSIONMASTER_WR_SWITCHES_224 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_tex", false},
    {"use_emissive_vertcolor", false},
    {"use_nonuvnoise", false},
}};

inline constexpr std::array<std::string_view,0> DIMENSIONMASTER_WR_TEXTURES_225 = {{}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,6> DIMENSIONMASTER_WR_PARAMETERS_225 = {{
    {"selectioncolor", 2u, 0u, true},
    {"01.color", 1u, 0u, true},
    {"01.radius", 0u, 0u, false},
    {"02.hardness", 0u, 1u, false},
    {"03.str", 0u, 2u, false},
    {"04.power", 0u, 3u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,4> DIMENSIONMASTER_WR_SWITCHES_225 = {{
    {"00.usenoise", false},
    {"01.usemesh", false},
    {"00.checkisdepthbiasalpha", false},
    {"00.useworldposition", false},
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_WR_TEXTURES_226 = {{"lensflaretexture"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,4> DIMENSIONMASTER_WR_PARAMETERS_226 = {{
    {"selectioncolor", 1u, 0u, true},
    {"select texture(0 or 0.5)", 0u, 2u, false},
    {"desaturation", 0u, 1u, false},
    {"depthbaisalpha", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,0> DIMENSIONMASTER_WR_SWITCHES_226 = {{
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_WR_TEXTURES_229 = {{"emissive_tex"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,1> DIMENSIONMASTER_WR_PARAMETERS_229 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,3> DIMENSIONMASTER_WR_SWITCHES_229 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", false},
}};

inline constexpr std::array<std::string_view,5> DIMENSIONMASTER_WR_TEXTURES_230 = {{"maintex","uv_noise_tex","emissivetex02","noisedissolve_tex","dissolve_tex_01"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,44> DIMENSIONMASTER_WR_PARAMETERS_230 = {{
    {"selectioncolor", 12u, 0u, true},
    {"maintex_move_x", 5u, 1u, false},
    {"maintex_move_y", 5u, 2u, false},
    {"maintex_rotator", 6u, 1u, false},
    {"dissolvetex_rotator", 1u, 3u, false},
    {"edge_color", 11u, 0u, true},
    {"maintex_panspeed_x", 5u, 3u, false},
    {"maintex_tile_x", 6u, 3u, false},
    {"maintex_tile_y", 7u, 0u, false},
    {"maintex_texcoordpower", 6u, 2u, false},
    {"maintex_panspeed_y", 6u, 0u, false},
    {"maintex_dynamicpan_x_velue", 4u, 3u, false},
    {"maintex_dynamicpan_y_velue", 5u, 0u, false},
    {"uv_noisetex_pan_x", 9u, 2u, false},
    {"uv_noisetex_tile_x", 10u, 0u, false},
    {"uv_noisetex_tile_y", 10u, 1u, false},
    {"uv_noisetex_pan_y", 9u, 3u, false},
    {"dynamic_uvnoise_x", 2u, 0u, false},
    {"dynamic_uvnoise_y", 2u, 1u, false},
    {"uv_noise_velue", 9u, 1u, false},
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
    {"noisetodisslovetex_01_panspeed_x", 7u, 2u, false},
    {"noisetodisslovetex_01_tile_x", 8u, 0u, false},
    {"noisetodisslovetex_01_tile_y", 8u, 1u, false},
    {"noisetodisslovetex_01_panspeed_y", 7u, 3u, false},
    {"noisedissolvetex_strength", 7u, 1u, false},
    {"disslovetex_01_panspeed_y", 0u, 1u, false},
    {"dissolvetex_move_x", 1u, 1u, false},
    {"dissolvetex_move_y", 1u, 2u, false},
    {"dissolve_hardness", 1u, 0u, false},
    {"edge_thin", 2u, 2u, false},
    {"spheremask_strength", 8u, 2u, false},
    {"spheremask_strength_max", 8u, 3u, false},
    {"spheremask_strength_min", 9u, 0u, false},
    {"maintex_alpha_strength", 4u, 2u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,20> DIMENSIONMASTER_WR_SWITCHES_230 = {{
    {"use_dissolve", true},
    {"use_meshtype", false},
    {"use_uvnoise", true},
    {"achannel_use_uvnoisedynamicpan", true},
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
    {"use_dissolvetex_noise", true},
    {"use_dissolvetex_noise_type", true},
    {"use_dissolvetex_02_mixtype", true},
    {"use_sphere_alpha", true},
    {"dissloveoutline_multyply_alpha", false},
    {"use_emissivetex02", true},
}};

inline constexpr std::array<std::string_view,5> DIMENSIONMASTER_WR_TEXTURES_231 = {{"maintex","uv_noise_tex","emissivetex02","noisedissolve_tex","dissolve_tex_01"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,41> DIMENSIONMASTER_WR_PARAMETERS_231 = {{
    {"selectioncolor", 11u, 0u, true},
    {"maintex_rotator", 6u, 2u, false},
    {"maintex_move_x", 5u, 2u, false},
    {"maintex_move_y", 5u, 3u, false},
    {"dissolvetex_rotator", 1u, 3u, false},
    {"edge_color", 10u, 0u, true},
    {"maintex_panspeed_x", 6u, 0u, false},
    {"maintex_tile_x", 6u, 3u, false},
    {"maintex_tile_y", 7u, 0u, false},
    {"maintex_dynamicpan_x_velue", 5u, 0u, false},
    {"maintex_dynamicpan_y_velue", 5u, 1u, false},
    {"maintex_panspeed_y", 6u, 1u, false},
    {"uv_noisetex_pan_x", 8u, 3u, false},
    {"uv_noisetex_tile_x", 9u, 1u, false},
    {"uv_noisetex_tile_y", 9u, 2u, false},
    {"uv_noisetex_pan_y", 9u, 0u, false},
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
    {"noisetodisslovetex_01_panspeed_x", 7u, 2u, false},
    {"noisetodisslovetex_01_tile_x", 8u, 0u, false},
    {"noisetodisslovetex_01_tile_y", 8u, 1u, false},
    {"noisetodisslovetex_01_panspeed_y", 7u, 3u, false},
    {"noisedissolvetex_strength", 7u, 1u, false},
    {"disslovetex_01_panspeed_y", 0u, 1u, false},
    {"dissolvetex_move_x", 1u, 1u, false},
    {"dissolvetex_move_y", 1u, 2u, false},
    {"dissolve_hardness", 1u, 0u, false},
    {"edge_thin", 2u, 2u, false},
    {"fresnel_power", 4u, 2u, false},
    {"maintex_alpha_strength", 4u, 3u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,20> DIMENSIONMASTER_WR_SWITCHES_231 = {{
    {"use_dissolve", true},
    {"use_meshtype", false},
    {"use_uvnoise", true},
    {"achannel_use_uvnoisedynamicpan", true},
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
    {"use_dissolvetex_noise", true},
    {"use_dissolvetex_noise_type", true},
    {"use_dissolvetex_02_mixtype", true},
    {"use_sphere_alpha", false},
    {"dissloveoutline_multyply_alpha", false},
    {"use_emissivetex02", true},
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_WR_TEXTURES_232 = {{"emissive_tex"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,1> DIMENSIONMASTER_WR_PARAMETERS_232 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,3> DIMENSIONMASTER_WR_SWITCHES_232 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", true},
}};

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_WR_TEXTURES_233 = {{"diff_tex","diff_noise_tex","a_mask_tex"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,26> DIMENSIONMASTER_WR_PARAMETERS_233 = {{
    {"selectioncolor", 7u, 0u, true},
    {"diff_color", 6u, 0u, true},
    {"mask_rot", 4u, 3u, false},
    {"diff_noise_tile_u", 2u, 3u, false},
    {"diff_noise_tile_v", 3u, 0u, false},
    {"diff_noise_offset_y", 2u, 1u, false},
    {"diff_noise_str", 2u, 2u, false},
    {"diff_tile_u", 3u, 3u, false},
    {"diff_tile_v", 4u, 0u, false},
    {"desaturation", 2u, 0u, false},
    {"diff_str", 3u, 2u, false},
    {"diff_pow", 3u, 1u, false},
    {"cast_fov", 0u, 3u, false},
    {"curlflow_x", 1u, 2u, false},
    {"cast_speed", 1u, 1u, false},
    {"cast_particle", 1u, 0u, false},
    {"cast_dirinout", 0u, 2u, false},
    {"cast_center", 0u, 1u, false},
    {"step_min", 5u, 3u, false},
    {"step_max", 5u, 2u, false},
    {"curlflow_y", 1u, 3u, false},
    {"particlesize", 5u, 1u, false},
    {"mask_bias", 4u, 1u, false},
    {"a_sizecontrol", 0u, 0u, false},
    {"mask_value", 5u, 0u, false},
    {"mask_pow", 4u, 2u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,13> DIMENSIONMASTER_WR_SWITCHES_233 = {{
    {"diff_pan_x_use", false},
    {"diff_pan_y_use", false},
    {"diff_polar_use", false},
    {"diff_use", true},
    {"mesh_use", false},
    {"particle_polar_use", true},
    {"mask_use", true},
    {"flow_pan_x_dynuse", false},
    {"flow_pan_x_use", false},
    {"flow_pan_y_use", false},
    {"use_volumeflow", false},
    {"use_flowradial", true},
    {"use_dy_curlflow_y", false},
}};

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_WR_TEXTURES_234 = {{"maintex","uv_noise_tex","dissolve_tex_01"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,34> DIMENSIONMASTER_WR_PARAMETERS_234 = {{
    {"selectioncolor", 10u, 0u, true},
    {"meshemitterdynamicparameter", 9u, 0u, true},
    {"maintex_rotator", 5u, 3u, false},
    {"maintex_move_x", 4u, 3u, false},
    {"maintex_move_y", 5u, 0u, false},
    {"dissolvetex_rotator", 2u, 0u, false},
    {"edge_color", 8u, 0u, true},
    {"maintex_panspeed_x", 5u, 1u, false},
    {"maintex_tile_x", 6u, 0u, false},
    {"maintex_tile_y", 6u, 1u, false},
    {"maintex_dynamicpan_x_velue", 4u, 1u, false},
    {"maintex_dynamicpan_y_velue", 4u, 2u, false},
    {"maintex_panspeed_y", 5u, 2u, false},
    {"uv_noisetex_pan_x", 6u, 3u, false},
    {"uv_noisetex_tile_x", 7u, 1u, false},
    {"uv_noisetex_tile_y", 7u, 2u, false},
    {"uv_noisetex_pan_y", 7u, 0u, false},
    {"dynamic_uvnoise_x", 2u, 1u, false},
    {"dynamic_uvnoise_y", 2u, 2u, false},
    {"uv_noise_velue", 6u, 2u, false},
    {"emissive_core_power", 3u, 1u, false},
    {"emissive_core_strength", 3u, 2u, false},
    {"emissive_base", 3u, 0u, false},
    {"disslovetex_01_panspeed_x", 0u, 0u, false},
    {"disslovetex_01_tile_x", 0u, 2u, false},
    {"disslovetex_01_tile_y", 0u, 3u, false},
    {"disslovetex_01_panspeed_y", 0u, 1u, false},
    {"dissolvetex_move_x", 1u, 1u, false},
    {"dissolvetex_move_y", 1u, 2u, false},
    {"dissolvetex_power", 1u, 3u, false},
    {"dissolve_hardness", 1u, 0u, false},
    {"edge_thin", 2u, 3u, false},
    {"fresnel_power", 3u, 3u, false},
    {"maintex_alpha_strength", 4u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,19> DIMENSIONMASTER_WR_SWITCHES_234 = {{
    {"use_alpha_fresnel", true},
    {"use_meshtype", true},
    {"use_dissolvetex_noise", false},
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
    {"use_sphere_alpha", false},
    {"use_emissivetex02", false},
    {"use_dissolve", true},
    {"dissloveoutline_multyply_alpha", true},
    {"texcoordpowertype_paramordynamic", true},
    {"use_uvnoise", true},
}};

inline constexpr std::array<std::string_view,6> DIMENSIONMASTER_WR_TEXTURES_235 = {{"crackcore_tex","crackcore_normal","native_texture_2","native_texture_3","crack_linetexture","crack_uvtexture"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,17> DIMENSIONMASTER_WR_PARAMETERS_235 = {{
    {"selectioncolor", 6u, 0u, true},
    {"refle_panu", 2u, 3u, false},
    {"refle_panv", 3u, 0u, false},
    {"reflection_color", 5u, 0u, true},
    {"crack_linetile_x", 0u, 3u, false},
    {"crack_linetile_y", 1u, 0u, false},
    {"crack_uvtile_x", 1u, 3u, false},
    {"crack_uvtile_y", 2u, 0u, false},
    {"crack_color", 4u, 0u, true},
    {"crack_core_str", 0u, 2u, false},
    {"reflection_power", 3u, 1u, false},
    {"crack_pow", 1u, 1u, false},
    {"crack_str", 1u, 2u, false},
    {"circle radius", 0u, 0u, false},
    {"circleedge hardness", 0u, 1u, false},
    {"opacity", 2u, 2u, false},
    {"distortion", 2u, 1u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,4> DIMENSIONMASTER_WR_SWITCHES_235 = {{
    {"use_circlealpha", true},
    {"use_invertedcirclealpha", false},
    {"use_clamp", true},
    {"use_mesh", false},
}};

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_WR_TEXTURES_238 = {{"normal_tex","refle_tex","native_texture_2"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,17> DIMENSIONMASTER_WR_PARAMETERS_238 = {{
    {"selectioncolor", 7u, 0u, true},
    {"refle_panspeed", 1u, 3u, false},
    {"refle_color", 6u, 0u, true},
    {"meshemitterdynamicparameter", 4u, 0u, true},
    {"in_color", 3u, 0u, true},
    {"out_color", 5u, 0u, true},
    {"normal_tileu", 0u, 2u, false},
    {"normal_tilev", 0u, 3u, false},
    {"refle_vector_divide", 2u, 3u, false},
    {"refle_tileu", 2u, 1u, false},
    {"refle_tilev", 2u, 2u, false},
    {"refle_offsetx", 1u, 1u, false},
    {"refle_offsety", 1u, 2u, false},
    {"refle_desaturation", 1u, 0u, false},
    {"refle_pow", 2u, 0u, false},
    {"fresnel_pow", 0u, 1u, false},
    {"distortion", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,2> DIMENSIONMASTER_WR_SWITCHES_238 = {{
    {"use_depth", false},
    {"use_dissolve", false},
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_WR_TEXTURES_239 = {{"emissive_tex"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,6> DIMENSIONMASTER_WR_PARAMETERS_239 = {{
    {"selectioncolor", 2u, 0u, true},
    {"uv_scale", 1u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,17> DIMENSIONMASTER_WR_SWITCHES_239 = {{
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

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_WR_TEXTURES_240 = {{"lensflaretexture"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,4> DIMENSIONMASTER_WR_PARAMETERS_240 = {{
    {"selectioncolor", 1u, 0u, true},
    {"select texture(0 or 0.5)", 0u, 2u, false},
    {"desaturation", 0u, 1u, false},
    {"depthbaisalpha", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,0> DIMENSIONMASTER_WR_SWITCHES_240 = {{
}};

inline constexpr std::array<std::string_view,0> DIMENSIONMASTER_WR_TEXTURES_241 = {{}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,6> DIMENSIONMASTER_WR_PARAMETERS_241 = {{
    {"selectioncolor", 2u, 0u, true},
    {"centerglow_power", 0u, 0u, false},
    {"centerglow_str", 0u, 1u, false},
    {"glow_power", 0u, 2u, false},
    {"str", 1u, 0u, false},
    {"power", 0u, 3u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,4> DIMENSIONMASTER_WR_SWITCHES_241 = {{
    {"use_distortion", false},
    {"use_centerglow", true},
    {"use_centermask", false},
    {"use_depth_alpha", true},
}};

inline constexpr std::array<std::string_view,7> DIMENSIONMASTER_WR_TEXTURES_242 = {{"cracknormal_tex","native_texture_1","in_hole_texture","aura_texture","native_texture_4","native_texture_5","native_texture_6"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,32> DIMENSIONMASTER_WR_PARAMETERS_242 = {{
    {"selectioncolor", 10u, 0u, true},
    {"in_hole_panx", 4u, 0u, false},
    {"in_hole_pany", 4u, 1u, false},
    {"alpha_tile_x", 0u, 2u, false},
    {"alpha_tile_y", 0u, 3u, false},
    {"alpha_offsetx", 0u, 0u, false},
    {"alpha_offsety", 0u, 1u, false},
    {"cracknormal_tile_x", 1u, 3u, false},
    {"cracknormal_tile_y", 2u, 0u, false},
    {"time", 6u, 0u, false},
    {"in_hole_color", 9u, 0u, true},
    {"aura_color", 8u, 0u, true},
    {"distortionpower", 2u, 2u, false},
    {"distortionscale", 2u, 3u, false},
    {"scale", 5u, 3u, false},
    {"cracknormal_str", 1u, 2u, false},
    {"in_hole_crackuv", 3u, 1u, false},
    {"in_hole_height", 3u, 3u, false},
    {"in_hole_pow", 4u, 2u, false},
    {"in_hole_str", 4u, 3u, false},
    {"in_hole_desaturation", 3u, 2u, false},
    {"main_ucoord", 5u, 1u, false},
    {"twist_str", 6u, 1u, false},
    {"main_tex_upanner", 5u, 0u, false},
    {"curve_power", 2u, 1u, false},
    {"main_v_panner", 5u, 2u, false},
    {"uvnoise_utile", 6u, 3u, false},
    {"uvnoise_vtile", 7u, 0u, false},
    {"uvnoise_pan", 6u, 2u, false},
    {"aura_pow", 1u, 0u, false},
    {"aura_str", 1u, 1u, false},
    {"edge_crack_desaturation", 3u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,6> DIMENSIONMASTER_WR_SWITCHES_242 = {{
    {"use_dynamic_vcoord", true},
    {"uvnoise_v_pannertime", true},
    {"main_v_pannertime", false},
    {"main_u_pannertime", false},
    {"use_mesh", false},
    {"use_edgesmooth", false},
}};

inline constexpr std::array<std::string_view,2> DIMENSIONMASTER_WR_TEXTURES_243 = {{"01.map_a","21.map_c"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,15> DIMENSIONMASTER_WR_PARAMETERS_243 = {{
    {"selectioncolor", 5u, 0u, true},
    {"93.emissiion_color", 3u, 0u, true},
    {"meshemitterdynamicparameter", 4u, 0u, true},
    {"21.uvscale.x", 1u, 1u, false},
    {"22.uvscale.y", 1u, 2u, false},
    {"time", 2u, 3u, false},
    {"04.map_a_panning_x", 0u, 3u, false},
    {"02.map_a_uvscale_r", 0u, 1u, false},
    {"03.map_a_uvscale_g", 0u, 2u, false},
    {"05.map_a_panning_y", 1u, 0u, false},
    {"29.cmap.direct", 1u, 3u, false},
    {"30.cmap.time(rotrate)", 2u, 0u, false},
    {"36.str", 2u, 1u, false},
    {"37.power", 2u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,26> DIMENSIONMASTER_WR_SWITCHES_243 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
    {"25.use_yclamp", false},
    {"00.use_uvdistort", false},
    {"10.use_mapb", false},
    {"00.use_mapa", true},
    {"30.usefresnal", false},
    {"00.checkisdepthbiasalpha", true},
    {"00.use_emission", true},
    {"50.usefresemission", false},
    {"01.use_emissionmap", false},
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

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_WR_TEXTURES_244 = {{"12.map_f","06.map","02.map_e"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,29> DIMENSIONMASTER_WR_PARAMETERS_244 = {{
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
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,27> DIMENSIONMASTER_WR_SWITCHES_244 = {{
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

inline constexpr std::array<std::string_view,2> DIMENSIONMASTER_WR_TEXTURES_245 = {{"01.map_a","06.map_b"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,19> DIMENSIONMASTER_WR_PARAMETERS_245 = {{
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
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,12> DIMENSIONMASTER_WR_SWITCHES_245 = {{
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

inline constexpr std::array<std::string_view,0> DIMENSIONMASTER_WR_TEXTURES_246 = {{}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,6> DIMENSIONMASTER_WR_PARAMETERS_246 = {{
    {"selectioncolor", 2u, 0u, true},
    {"centerglow_power", 0u, 0u, false},
    {"centerglow_str", 0u, 1u, false},
    {"glow_power", 0u, 2u, false},
    {"str", 1u, 0u, false},
    {"power", 0u, 3u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,0> DIMENSIONMASTER_WR_SWITCHES_246 = {{
}};

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_WR_TEXTURES_247 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,19> DIMENSIONMASTER_WR_PARAMETERS_247 = {{
    {"selectioncolor", 5u, 0u, true},
    {"power", 4u, 0u, false},
    {"str", 4u, 1u, false},
    {"04.map_a_panning_x", 1u, 0u, false},
    {"02.map_a_uvscale_r", 0u, 2u, false},
    {"03.map_a_uvscale_g", 0u, 3u, false},
    {"17.map_d_panning_x", 3u, 1u, false},
    {"15.map_d_uvscale_r", 2u, 3u, false},
    {"16.map_d_uvscale_g", 3u, 0u, false},
    {"18.map_d_panning_y", 3u, 2u, false},
    {"13.distortstr", 2u, 2u, false},
    {"05.map_a_panning_y", 1u, 1u, false},
    {"09.map_b_panning_x", 2u, 0u, false},
    {"07.map_b_uvscale_r", 1u, 2u, false},
    {"08.map_b_uvscale_g", 1u, 3u, false},
    {"10.map_b_panning_y", 2u, 1u, false},
    {"00.noisepower", 0u, 0u, false},
    {"00.noisestr", 0u, 1u, false},
    {"depthbiasdalpha_bias", 3u, 3u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,5> DIMENSIONMASTER_WR_SWITCHES_247 = {{
    {"--usefakedepthbiasalpha", false},
    {"checkisdepthbiasalpha", true},
    {"00.checkisnoiseon", true},
    {"12.uvnoise", true},
    {"11.uvmirroring", false},
}};

inline constexpr std::array<std::string_view,4> DIMENSIONMASTER_WR_TEXTURES_248 = {{"diff_texture","dissolve_texture","flow_texture","alpha_texture"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,25> DIMENSIONMASTER_WR_PARAMETERS_248 = {{
    {"selectioncolor", 8u, 0u, true},
    {"diff_color", 6u, 0u, true},
    {"dissolve_color", 7u, 0u, true},
    {"desaturation", 2u, 0u, false},
    {"dissolve_tilex", 3u, 2u, false},
    {"dissolve_tiley", 3u, 3u, false},
    {"dissolve_panning_x", 2u, 3u, false},
    {"dissolve_offsetx", 2u, 1u, false},
    {"dissolve_panning_y", 3u, 0u, false},
    {"dissolve_offsety", 2u, 2u, false},
    {"dissolve_str", 3u, 1u, false},
    {"substep_str", 5u, 1u, false},
    {"flow_tilex", 4u, 3u, false},
    {"flow_tiley", 5u, 0u, false},
    {"flow_panx", 4u, 0u, false},
    {"flow_pany", 4u, 1u, false},
    {"flow_str", 4u, 2u, false},
    {"alpha_tilex", 1u, 2u, false},
    {"alpha_tiley", 1u, 3u, false},
    {"alpha_dy_offsetx", 0u, 0u, false},
    {"alpha_offsetx", 0u, 3u, false},
    {"alpha_dy_offsety", 0u, 1u, false},
    {"alpha_offsety", 1u, 0u, false},
    {"alpha_hardness", 0u, 2u, false},
    {"alpha_str", 1u, 1u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,2> DIMENSIONMASTER_WR_SWITCHES_248 = {{
    {"use_mesh", false},
    {"use_steptexcoord_y", true},
}};

inline constexpr std::array<std::string_view,5> DIMENSIONMASTER_WR_TEXTURES_251 = {{"01.map_uv","01.map_e","native_texture_2","01.map_a","11.map_b"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,13> DIMENSIONMASTER_WR_PARAMETERS_251 = {{
    {"selectioncolor", 4u, 0u, true},
    {"19.emissiion_color", 3u, 0u, true},
    {"04.map_e_panning_x", 0u, 2u, false},
    {"02.map_e_uvscale_r", 0u, 0u, false},
    {"03.map_e_uvscale_g", 0u, 1u, false},
    {"05.map_e_panning_y", 0u, 3u, false},
    {"12.desaturation", 1u, 2u, false},
    {"15.emissiion_power", 2u, 0u, false},
    {"09.str", 1u, 0u, false},
    {"10.power", 1u, 1u, false},
    {"12.map_b_uvscale", 1u, 3u, false},
    {"18.str", 2u, 1u, false},
    {"19.power", 2u, 2u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,8> DIMENSIONMASTER_WR_SWITCHES_251 = {{
    {"00.checkisdepthbiasalpha", false},
    {"01.usemesh", false},
    {"06.mapch.r", false},
    {"07.mapch.g", false},
    {"08.mapch.b", false},
    {"00.useemissionmap", true},
    {"30.usefresnal", false},
    {"31.fresnal.invert", false},
}};

inline constexpr std::array<std::string_view,2> DIMENSIONMASTER_WR_TEXTURES_252 = {{"01.map_e","06.map_f"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,22> DIMENSIONMASTER_WR_PARAMETERS_252 = {{
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
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,10> DIMENSIONMASTER_WR_SWITCHES_252 = {{
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

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_WR_TEXTURES_253 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,19> DIMENSIONMASTER_WR_PARAMETERS_253 = {{
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
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,6> DIMENSIONMASTER_WR_SWITCHES_253 = {{
    {"checkisdepthbiasalpha", true},
    {"00.checkisnoiseon", true},
    {"12.uvnoise", true},
    {"11.uvmirroring", false},
    {"use_meshtype", false},
    {"use_multyply_noisecolor", false},
}};

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_WR_TEXTURES_254 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,24> DIMENSIONMASTER_WR_PARAMETERS_254 = {{
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
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,12> DIMENSIONMASTER_WR_SWITCHES_254 = {{
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

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_WR_TEXTURES_255 = {{"01.map_e","01.map_a","21.map_m"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,17> DIMENSIONMASTER_WR_PARAMETERS_255 = {{
    {"selectioncolor", 5u, 0u, true},
    {"01.rotateuv", 0u, 0u, false},
    {"04.map_a_pantimescale", 1u, 3u, false},
    {"02.map_a_uv.x", 0u, 2u, false},
    {"03.map_a_uv.y", 1u, 1u, false},
    {"19.emissiion_color", 4u, 0u, true},
    {"04.map_e_panning_x", 2u, 0u, false},
    {"02.map_e_uvscale_r", 0u, 3u, false},
    {"03.map_e_uvscale_g", 1u, 2u, false},
    {"01.wavecount", 0u, 1u, false},
    {"02.wavestr", 1u, 0u, false},
    {"11.width", 3u, 0u, false},
    {"05.map_e_panning_y", 2u, 1u, false},
    {"12.desaturation", 3u, 1u, false},
    {"15.emissiion_power", 3u, 2u, false},
    {"09.str", 2u, 2u, false},
    {"10.power", 2u, 3u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,6> DIMENSIONMASTER_WR_SWITCHES_255 = {{
    {"01.usemesh", false},
    {"30.usefresnel", false},
    {"50.useendfalloff", false},
    {"00.useemismap", true},
    {"00.use_wavetimebezier_in_dp", true},
    {"00.use_uvdistort", false},
}};

inline constexpr std::array<std::string_view,5> DIMENSIONMASTER_WR_TEXTURES_256 = {{"maintex","uv_noise_tex_02","uv_noise_tex","noisedissolve_tex","dissolve_tex_01"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,45> DIMENSIONMASTER_WR_PARAMETERS_256 = {{
    {"selectioncolor", 12u, 0u, true},
    {"maintex_rotator", 5u, 0u, false},
    {"uvnoise_move_x", 10u, 1u, false},
    {"uvnoise_move_y", 10u, 2u, false},
    {"maintex_move_x", 4u, 0u, false},
    {"maintex_move_y", 4u, 1u, false},
    {"dissolvetex_rotator", 2u, 0u, false},
    {"edge_color", 11u, 0u, true},
    {"maintex_panspeed_x", 4u, 2u, false},
    {"maintex_tile_x", 5u, 1u, false},
    {"maintex_tile_y", 5u, 2u, false},
    {"maintex_dynamicpan_x_velue", 3u, 2u, false},
    {"maintex_dynamicpan_y_velue", 3u, 3u, false},
    {"maintex_panspeed_y", 4u, 3u, false},
    {"uv_noisetex_pan_x", 8u, 3u, false},
    {"uv_noisetex_tile_x", 9u, 3u, false},
    {"uv_noisetex_tile_y", 10u, 0u, false},
    {"uv_noisetex_pan_02_x", 8u, 1u, false},
    {"uv_noisetex_tile_02_x", 9u, 1u, false},
    {"uv_noisetex_tile_02_y", 9u, 2u, false},
    {"uv_noisetex_pan_02_y", 8u, 2u, false},
    {"uv_noise_02_strength", 7u, 3u, false},
    {"uv_noisetex_pan_y", 9u, 0u, false},
    {"uv_noise_velue", 8u, 0u, false},
    {"emissive_core_power", 2u, 3u, false},
    {"emissive_core_strength", 3u, 0u, false},
    {"emissive_base", 2u, 2u, false},
    {"disslovetex_01_panspeed_x", 0u, 0u, false},
    {"disslovetex_01_tile_x", 0u, 2u, false},
    {"disslovetex_01_tile_y", 0u, 3u, false},
    {"noisetodisslovetex_01_panspeed_x", 6u, 0u, false},
    {"noisetodisslovetex_01_tile_x", 6u, 2u, false},
    {"noisetodisslovetex_01_tile_y", 6u, 3u, false},
    {"noisetodisslovetex_01_panspeed_y", 6u, 1u, false},
    {"noisedissolvetex_strength", 5u, 3u, false},
    {"disslovetex_01_panspeed_y", 0u, 1u, false},
    {"dissolvetex_move_x", 1u, 1u, false},
    {"dissolvetex_move_y", 1u, 2u, false},
    {"dissolvetex_power", 1u, 3u, false},
    {"dissolve_hardness", 1u, 0u, false},
    {"edge_thin", 2u, 1u, false},
    {"spheremask_strength", 7u, 0u, false},
    {"spheremask_strength_max", 7u, 1u, false},
    {"spheremask_strength_min", 7u, 2u, false},
    {"maintex_alpha_strength", 3u, 1u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,19> DIMENSIONMASTER_WR_SWITCHES_256 = {{
    {"use_alpha_fresnel", false},
    {"use_meshtype", false},
    {"use_dissolvetex_noise", true},
    {"use_dissolvetex_noise_type", true},
    {"achannel_use_uvnoisedynamicpan", false},
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
    {"dissloveoutline_multyply_alpha", false},
    {"texcoordpowertype_paramordynamic", true},
    {"use_uvnoise", true},
}};

inline constexpr std::array<std::string_view,6> DIMENSIONMASTER_WR_TEXTURES_257 = {{"native_texture_0","native_texture_1","native_texture_2","sparkle_tex","edgedeco texture01","native_texture_5"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,4> DIMENSIONMASTER_WR_PARAMETERS_257 = {{
    {"selectioncolor", 1u, 0u, true},
    {"sparkle_paning", 0u, 1u, false},
    {"sparkle_tiling", 0u, 2u, false},
    {"sparkle_intensity", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,0> DIMENSIONMASTER_WR_SWITCHES_257 = {{
}};

inline constexpr std::array<std::string_view,7> DIMENSIONMASTER_WR_TEXTURES_258 = {{"06.map","02.map_e","12.map_f","01.map_a","11.map_b","21.map_c","01.specmap"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,43> DIMENSIONMASTER_WR_PARAMETERS_258 = {{
    {"selectioncolor", 14u, 0u, true},
    {"05.specmap_uvscale.x", 2u, 2u, false},
    {"06.specmap_uvscale.y", 3u, 0u, false},
    {"61.twist.xy", 11u, 0u, true},
    {"93.emissiion_color", 12u, 0u, true},
    {"meshemitterdynamicparameter", 13u, 0u, true},
    {"21.uvscale.x", 6u, 3u, false},
    {"22.uvscale.y", 7u, 0u, false},
    {"09.specmap_color", 10u, 0u, true},
    {"time", 9u, 1u, false},
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
    {"91.desaturation", 8u, 3u, false},
    {"92.emissiion_power", 9u, 0u, false},
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
    {"36.str", 8u, 1u, false},
    {"37.power", 8u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
    {"32.fresnal_power", 7u, 3u, false},
    {"33.fresnal_str", 8u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,26> DIMENSIONMASTER_WR_SWITCHES_258 = {{
    {"00.usedistortion", false},
    {"01.usemesh", true},
    {"31.mapch.r", true},
    {"32.mapch.g", true},
    {"20.use_clampmap", true},
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
    {"00.usespecullar", true},
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

inline constexpr std::array<std::string_view,4> DIMENSIONMASTER_WR_TEXTURES_259 = {{"06.map","02.map_e","12.map_f","21.map_c"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,25> DIMENSIONMASTER_WR_PARAMETERS_259 = {{
    {"selectioncolor", 7u, 0u, true},
    {"93.emissiion_color", 6u, 0u, true},
    {"21.uvscale.x", 3u, 2u, false},
    {"22.uvscale.y", 3u, 3u, false},
    {"time", 5u, 2u, false},
    {"05.map_e_panning_x", 1u, 0u, false},
    {"03.map_e_uvscale_r", 0u, 1u, false},
    {"04.map_e_uvscale_g", 0u, 2u, false},
    {"09.map_d_panning_x", 2u, 0u, false},
    {"07.map_d_uvscale_r", 1u, 2u, false},
    {"08.map_d_uvscale_g", 1u, 3u, false},
    {"10.map_d_panning_y", 2u, 1u, false},
    {"05.distort_str", 0u, 3u, false},
    {"06.map_e_panning_y", 1u, 1u, false},
    {"15.map_f_panning_x", 3u, 0u, false},
    {"13.map_f_uvscale_r", 2u, 2u, false},
    {"14.map_f_uvscale_g", 2u, 3u, false},
    {"16.map_f_panning_y", 3u, 1u, false},
    {"91.desaturation", 5u, 0u, false},
    {"92.emissiion_power", 5u, 1u, false},
    {"29.cmap.direct", 4u, 0u, false},
    {"30.cmap.time(rotrate)", 4u, 1u, false},
    {"36.str", 4u, 2u, false},
    {"37.power", 4u, 3u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,26> DIMENSIONMASTER_WR_SWITCHES_259 = {{
    {"00.usedistortion", false},
    {"01.usemesh", false},
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

inline constexpr std::array<std::string_view,4> DIMENSIONMASTER_WR_TEXTURES_260 = {{"maintex","uv_noise_tex","noisedissolve_tex","dissolve_tex_01"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,40> DIMENSIONMASTER_WR_PARAMETERS_260 = {{
    {"selectioncolor", 12u, 0u, true},
    {"meshemitterdynamicparameter", 11u, 0u, true},
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
// Native CPU uniform rotation occupies a free row of the existing V packet.
inline constexpr uint32_t DIMENSIONMASTER_WR_260_CPU_ROTATION_ROW = 13u;
static_assert(std::all_of(DIMENSIONMASTER_WR_PARAMETERS_260.begin(),
    DIMENSIONMASTER_WR_PARAMETERS_260.end(), [](const auto& Parameter)
    { return Parameter.iRow < DIMENSIONMASTER_WR_260_CPU_ROTATION_ROW; }));

inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,20> DIMENSIONMASTER_WR_SWITCHES_260 = {{
    {"use_dissolve", true},
    {"use_meshtype", true},
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

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_WR_TEXTURES_262 = {{"subuv_tex"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,4> DIMENSIONMASTER_WR_PARAMETERS_262 = {{
    {"selectioncolor", 2u, 0u, true},
    {"emissive_color&intensity", 1u, 0u, true},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_tex_desturation", 0u, 1u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,5> DIMENSIONMASTER_WR_SWITCHES_262 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", true},
    {"use_nonalpha_tex", false},
    {"use_emissive_vertcolor", false},
    {"use_nonuvnoise", false},
}};

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_WR_TEXTURES_263 = {{"normal_tex","refle_tex","native_texture_2"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,17> DIMENSIONMASTER_WR_PARAMETERS_263 = {{
    {"selectioncolor", 7u, 0u, true},
    {"refle_panspeed", 1u, 3u, false},
    {"refle_color", 6u, 0u, true},
    {"meshemitterdynamicparameter", 4u, 0u, true},
    {"in_color", 3u, 0u, true},
    {"out_color", 5u, 0u, true},
    {"normal_tileu", 0u, 2u, false},
    {"normal_tilev", 0u, 3u, false},
    {"refle_vector_divide", 2u, 3u, false},
    {"refle_tileu", 2u, 1u, false},
    {"refle_tilev", 2u, 2u, false},
    {"refle_offsetx", 1u, 1u, false},
    {"refle_offsety", 1u, 2u, false},
    {"refle_desaturation", 1u, 0u, false},
    {"refle_pow", 2u, 0u, false},
    {"fresnel_pow", 0u, 1u, false},
    {"distortion", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,2> DIMENSIONMASTER_WR_SWITCHES_263 = {{
    {"use_depth", false},
    {"use_dissolve", false},
}};


inline constexpr std::array<std::string_view,4> DIMENSIONMASTER_WR_TEXTURES_277 = {{"flowtex","diff_tex1","diff_tex2","opacity_tex"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,27> DIMENSIONMASTER_WR_PARAMETERS_277 = {{
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
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,5> DIMENSIONMASTER_WR_SWITCHES_277 = {{
    {"use_gra_r_channel", false},
    {"use_colormap", false},
    {"diff_pan_use_u", false},
    {"use_cam_vector", true},
    {"use_distortion", false},
}};

inline constexpr std::array<std::string_view,2> DIMENSIONMASTER_WR_TEXTURES_278 = {{"native_texture_0","native_texture_1"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,2> DIMENSIONMASTER_WR_PARAMETERS_278 = {{
    {"selectioncolor", 1u, 0u, true},
    {"meshemitterdynamicparameter", 0u, 0u, true},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,0> DIMENSIONMASTER_WR_SWITCHES_278 = {{
}};

inline constexpr std::array<std::string_view,6> DIMENSIONMASTER_WR_TEXTURES_279 = {{"diff_tex","a_mask_tex","a_noise_01_tex","b_mask_tex","b_noise_01_tex","dissolve_tex"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,56> DIMENSIONMASTER_WR_PARAMETERS_279 = {{
    {"selectioncolor", 16u, 0u, true},
    {"meshemitterdynamicparameter", 15u, 0u, true},
    {"a_rotator", 3u, 1u, false},
    {"a_noise_01_tile_u", 2u, 1u, false},
    {"a_noise_01_tile_v", 2u, 2u, false},
    {"a_noise_01_pan_x", 1u, 2u, false},
    {"a_noise_01_offset_x", 1u, 0u, false},
    {"a_noise_01_pan_y", 1u, 3u, false},
    {"a_noise_01_offset_y", 1u, 1u, false},
    {"a_mask_color", 13u, 0u, true},
    {"b_rotator", 7u, 2u, false},
    {"b_noise_01_tile_u", 6u, 2u, false},
    {"b_noise_01_tile_v", 6u, 3u, false},
    {"b_noise_01_pan_x", 5u, 3u, false},
    {"b_noise_01_offset_x", 5u, 1u, false},
    {"b_noise_01_pan_y", 6u, 0u, false},
    {"b_noise_01_offset_y", 5u, 2u, false},
    {"b_mask_color", 14u, 0u, true},
    {"dissolve_rot", 10u, 2u, false},
    {"diff_tile_u", 9u, 1u, false},
    {"diff_tile_v", 9u, 2u, false},
    {"diff_tex_desaturation", 9u, 0u, false},
    {"a_sizecontrol", 3u, 2u, false},
    {"a_tile_u", 3u, 3u, false},
    {"a_tile_v", 4u, 0u, false},
    {"a_offset_x", 2u, 3u, false},
    {"a_offset_y", 3u, 0u, false},
    {"a_noise_01_str", 2u, 0u, false},
    {"b_mask_debug(0or1)", 4u, 1u, false},
    {"a_mask_str", 0u, 3u, false},
    {"a_mask_pow", 0u, 2u, false},
    {"a_mask_desaturation", 0u, 1u, false},
    {"b_sizecontrol", 7u, 3u, false},
    {"b_tile_u", 8u, 0u, false},
    {"b_tile_v", 8u, 1u, false},
    {"b_offset_x", 7u, 0u, false},
    {"b_offset_y", 7u, 1u, false},
    {"b_noise_01_str", 6u, 1u, false},
    {"a_mask_debug(0or1)", 0u, 0u, false},
    {"b_mask_str", 5u, 0u, false},
    {"b_mask_pow", 4u, 3u, false},
    {"b_mask_desaturation", 4u, 2u, false},
    {"diff_str", 8u, 3u, false},
    {"diff_pow", 8u, 2u, false},
    {"mask_linearalpha", 11u, 3u, false},
    {"opacity_pow", 12u, 2u, false},
    {"opacity_str", 12u, 3u, false},
    {"mask_radius", 12u, 0u, false},
    {"mask_density", 11u, 2u, false},
    {"dissolve_pan_x", 10u, 0u, false},
    {"dissolve_tile_x", 10u, 3u, false},
    {"dissolve_tile_y", 11u, 0u, false},
    {"dissolve_pan_y", 10u, 1u, false},
    {"dissolve_hardness", 9u, 3u, false},
    {"meshedgefade", 12u, 1u, false},
    {"distortion", 11u, 1u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,21> DIMENSIONMASTER_WR_SWITCHES_279 = {{
    {"meshtype_use", true},
    {"mesh_billboard_use", false},
    {"depthalpha_use", false},
    {"meshedge_soft_use", false},
    {"coresoft_use", false},
    {"width_height_use", false},
    {"mask_compose", false},
    {"b_sizecontrol_dyuse", false},
    {"nonpolar_mask_b", true},
    {"b_pan_y_use", false},
    {"b_pan_x_dynuse", false},
    {"b_pan_x_use", false},
    {"a_sizecontrol_dyuse", false},
    {"nonpolar_mask_a", true},
    {"a_pan_y_use", false},
    {"a_pan_x_dynuse", false},
    {"a_pan_x_use", false},
    {"diff_polar_use", false},
    {"diff_pan_y_use", false},
    {"diff_pan_x_use", false},
    {"dissolve_use", true},
}};

inline constexpr std::array<std::string_view,4> DIMENSIONMASTER_WR_TEXTURES_280 = {{"flowtex","diff_tex1","diff_tex2","opacity_tex"}};
inline constexpr std::array<DIMENSIONMASTER_WR_PARAMETER_DESC,27> DIMENSIONMASTER_WR_PARAMETERS_280 = {{
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
inline constexpr std::array<DIMENSIONMASTER_WR_SWITCH_DESC,5> DIMENSIONMASTER_WR_SWITCHES_280 = {{
    {"use_gra_r_channel", false},
    {"use_colormap", false},
    {"diff_pan_use_u", false},
    {"use_cam_vector", true},
    {"use_distortion", false},
}};

inline constexpr std::array<DIMENSIONMASTER_WR_PROGRAM_DESC,56> DIMENSIONMASTER_WR_PROGRAMS = {{
    {208u,"effect.ue3.wr-208-native.v1","fx_m_mi_j_00.fx_mi.fx_j_pa_spriteinvert_01_02_tr","fx_m_mi_j_00.fx_m.fx_j_pa_spriteinvert_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.spriteinvert.01.tr.0f09ed5738d6",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_208,DIMENSIONMASTER_WR_PARAMETERS_208,DIMENSIONMASTER_WR_SWITCHES_208},
    {209u,"effect.ue3.wr-209-native.v1","fx_m_mi_02.fx_m.fx_j_pa_dot_ad_01","fx_m_mi_02.fx_m.fx_j_pa_dot_ad_01","ue3.material.fx.m.mi.02.fx.m.fx.j.pa.dot.ad.01.3568dcff5c7c",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_209,DIMENSIONMASTER_WR_PARAMETERS_209,DIMENSIONMASTER_WR_SWITCHES_209},
    {210u,"effect.ue3.wr-210-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_ring_07_26_ad","fx_m_mi_03.fx_m.fx_d_pa_ring_07_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.ad.82e9116584b2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_210,DIMENSIONMASTER_WR_PARAMETERS_210,DIMENSIONMASTER_WR_SWITCHES_210},
    {211u,"effect.ue3.wr-211-native.v1","fx_m_mi_m_00.fx_mi.fx_m_pa_bloodcliff_glow_depthfade_01_02_tr","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_tr","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.circ.01.tr.f3e3bcf906f5",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_211,DIMENSIONMASTER_WR_PARAMETERS_211,DIMENSIONMASTER_WR_SWITCHES_211},
    {212u,"effect.ue3.wr-212-native.v1","fx_m_mi_m_00.fx_mi.fx_m_pa_bloodcliff_glow_depthfade_01_02_ad","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.circ.01.ad.0f4a4414a90a",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_212,DIMENSIONMASTER_WR_PARAMETERS_212,DIMENSIONMASTER_WR_SWITCHES_212},
    {213u,"effect.ue3.wr-213-native.v1","fx_m_mi_k_00.fx_mi.fx_k_pa_slice_01_02_tr","fx_m_mi_j_00.fx_m.fx_j_pa_slice_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.slice.01.tr.afc439ecf232",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_213,DIMENSIONMASTER_WR_PARAMETERS_213,DIMENSIONMASTER_WR_SWITCHES_213},
    {214u,"effect.ue3.wr-214-native.v1","fx_m_mi_j_00.fx_mi.fx_j_me_localcrack_01_04_tr","fx_m_mi_j_00.fx_m.fx_j_me_localcrack_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.me.localcrack.01.tr.a0e83a46fef9",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_214,DIMENSIONMASTER_WR_PARAMETERS_214,DIMENSIONMASTER_WR_SWITCHES_214},
    {215u,"effect.ue3.wr-215-native.v1","fx_m_mi_k_00.fx_mi.fx_k_pa_crackholev2_01_01","fx_m_mi_k_00.fx_m.fx_k_crackholev2_01","ue3.material.fx.m.mi.k.00.fx.m.fx.k.crackholev2.01.3aac97e0fcad",false,"sprite",false,false,true,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_215,DIMENSIONMASTER_WR_PARAMETERS_215,DIMENSIONMASTER_WR_SWITCHES_215},
    {216u,"effect.ue3.wr-216-native.v1","fx_m_mi_k_00.fx_mi.fx_k_pa_glasshole_02_01_tr","fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2",false,"sprite",false,true,true,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_216,DIMENSIONMASTER_WR_PARAMETERS_216,DIMENSIONMASTER_WR_SWITCHES_216},
    {217u,"effect.ue3.wr-217-native.v1","fx_m_mi_k_00.fx_mi.fx_k_pa_fluidninja_01_07_tr","fx_m_mi_k_00.fx_m.fx_k_pa_fluidninja_01_tr","ue3.material.fx.m.mi.k.00.fx.m.fx.k.pa.fluidninja.01.tr.534340d78128",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_217,DIMENSIONMASTER_WR_PARAMETERS_217,DIMENSIONMASTER_WR_SWITCHES_217},
    {218u,"effect.ue3.wr-218-native.v1","fx_m_mi_02.fx_mi.fx_m_pa_shine_02_3_ad","fx_m_mi_02.fx_m.fx_d_pa_shine_02_ad","ue3.material.fx.m.mi.02.fx.m.fx.d.pa.shine.02.ad.29d7864aadba",false,"sprite",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_218,DIMENSIONMASTER_WR_PARAMETERS_218,DIMENSIONMASTER_WR_SWITCHES_218},
    {220u,"effect.ue3.wr-220-native.v1","fx_m_mi_02.fx_mi.fx_e_pa_ht_18_1_tr","fx_m_mi_02.fx_m.fx_f_pa_shine_01_0_tr","ue3.material.fx.m.mi.02.fx.m.fx.f.pa.shine.01.0.tr.3d21bd1f3e79",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_220,DIMENSIONMASTER_WR_PARAMETERS_220,DIMENSIONMASTER_WR_SWITCHES_220},
    {221u,"effect.ue3.wr-221-native.v1","fx_m_mi_02.fx_mi.fx_j_rgbsplit_01_2_ad","fx_m_mi_02.fx_m.fx_j_rgbsplit_01_ad","ue3.material.fx.m.mi.02.fx.m.fx.j.rgbsplit.01.ad.b59195d23f11",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_221,DIMENSIONMASTER_WR_PARAMETERS_221,DIMENSIONMASTER_WR_SWITCHES_221},
    {222u,"effect.ue3.wr-222-native.v1","fx_m_mi_n_00.fx_n_pa_db_01_04_ad","fx_m_mi_n_00.fx_m.fx_n_pa_db_01_01_ad","ue3.material.fx.m.mi.n.00.fx.m.fx.n.pa.db.01.01.ad.e600b7b6e487",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_222,DIMENSIONMASTER_WR_PARAMETERS_222,DIMENSIONMASTER_WR_SWITCHES_222},
    {223u,"effect.ue3.wr-223-native.v1","fx_m_mi_m_00.fx_mi.fx_m_bloodcliff_01_10_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_223,DIMENSIONMASTER_WR_PARAMETERS_223,DIMENSIONMASTER_WR_SWITCHES_223},
    {224u,"effect.ue3.wr-224-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_ht_08_1_ad","fx_mastermaterial.fx_mm.fx_mm_simple_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_224,DIMENSIONMASTER_WR_PARAMETERS_224,DIMENSIONMASTER_WR_SWITCHES_224},
    {225u,"effect.ue3.wr-225-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_dark_05_02_tr","fx_m_mi_00.fx_m.fx_d_pa_dark_05_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.dark.05.tr.891593ac5322",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_225,DIMENSIONMASTER_WR_PARAMETERS_225,DIMENSIONMASTER_WR_SWITCHES_225},
    {226u,"effect.ue3.wr-226-native.v1","fx_m_mi_00.fx_mi.fx_c_pa_lensflare_01_05_ad","fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.c.pa.lensflare.01.ad.2cdc706962af",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_226,DIMENSIONMASTER_WR_PARAMETERS_226,DIMENSIONMASTER_WR_SWITCHES_226},
    {229u,"effect.ue3.wr-229-native.v1","fx_m_mi_03.fx_mi.fx_j_pa_ringline_01_tr","fx_mastermaterial.fx_mm.fx_mm_light_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.tr.8a6435f0c4e0",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_229,DIMENSIONMASTER_WR_PARAMETERS_229,DIMENSIONMASTER_WR_SWITCHES_229},
    {230u,"effect.ue3.wr-230-native.v1","fx_m_mi_l_00.fx_mi.fx_l_pa_spritewave_01_85_tr","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_230,DIMENSIONMASTER_WR_PARAMETERS_230,DIMENSIONMASTER_WR_SWITCHES_230},
    {231u,"effect.ue3.wr-231-native.v1","fx_m_mi_s_00.fx_s_me_spritewave_01_01_tr","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",false,"sprite",false,false,true,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_231,DIMENSIONMASTER_WR_PARAMETERS_231,DIMENSIONMASTER_WR_SWITCHES_231},
    {232u,"effect.ue3.wr-232-native.v1","fx_m_mi_m_00.fx_mi.fx_m_pa_agent_05_20e","fx_mastermaterial.fx_mm.fx_mm_light_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.tr.8a6435f0c4e0",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_232,DIMENSIONMASTER_WR_PARAMETERS_232,DIMENSIONMASTER_WR_SWITCHES_232},
    {233u,"effect.ue3.wr-233-native.v1","fx_m_mi_j_00.fx_mi.fx_j_pa_customparticle_01_06_ad","fx_m_mi_j_00.fx_m.fx_j_pa_customparticle_01_ad","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.customparticle.01.ad.e6b959010967",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_233,DIMENSIONMASTER_WR_PARAMETERS_233,DIMENSIONMASTER_WR_SWITCHES_233},
    {234u,"effect.ue3.wr-234-native.v1","fx_m_mi_k_00.fx_mi.fx_k_me_spritewave_01_45_ad","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_ad","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.ad.caabbddf8b55",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_234,DIMENSIONMASTER_WR_PARAMETERS_234,DIMENSIONMASTER_WR_SWITCHES_234},
    {235u,"effect.ue3.wr-235-native.v1","fx_m_mi_j_00.fx_mi.fx_j_pa_crackbase_02_03","fx_m_mi_j_00.fx_m.fx_j_pa_crackbase_02","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.crackbase.02.bce3f90a86f0",false,"sprite",true,false,true,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_235,DIMENSIONMASTER_WR_PARAMETERS_235,DIMENSIONMASTER_WR_SWITCHES_235},
    {238u,"effect.ue3.wr-238-native.v1","fx_m_mi_j_00.fx_mi.fx_j_me_localcrack_01_03_tr","fx_m_mi_j_00.fx_m.fx_j_me_localcrack_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.me.localcrack.01.tr.a0e83a46fef9",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_238,DIMENSIONMASTER_WR_PARAMETERS_238,DIMENSIONMASTER_WR_SWITCHES_238},
    {239u,"effect.ue3.wr-239-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_gl_01_4_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_239,DIMENSIONMASTER_WR_PARAMETERS_239,DIMENSIONMASTER_WR_SWITCHES_239},
    {240u,"effect.ue3.wr-240-native.v1","fx_m_mi_00.fx_mi.fx_c_pa_lensflare_01_02_ad","fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.c.pa.lensflare.01.ad.2cdc706962af",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_240,DIMENSIONMASTER_WR_PARAMETERS_240,DIMENSIONMASTER_WR_SWITCHES_240},
    {241u,"effect.ue3.wr-241-native.v1","fx_m_mi_01.fx_mi.fx_k_pa_glow_01_ad_dt","bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.glow.01.ad.0857e02620a0",false,"sprite",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_241,DIMENSIONMASTER_WR_PARAMETERS_241,DIMENSIONMASTER_WR_SWITCHES_241},
    {242u,"effect.ue3.wr-242-native.v1","fx_m_mi_k_00.fx_mi.fx_k_pa_glasshole_02_01_tr","fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2",false,"sprite",false,true,true,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_242,DIMENSIONMASTER_WR_PARAMETERS_242,DIMENSIONMASTER_WR_SWITCHES_242},
    {243u,"effect.ue3.wr-243-native.v1","fx_m_mi_w_00.mi.fx_w_me_master_01_1_ad","fx_m_mi_00.fx_m.fx_d_pa_master_01_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f",true,"mesh",false,true,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_243,DIMENSIONMASTER_WR_PARAMETERS_243,DIMENSIONMASTER_WR_SWITCHES_243},
    {244u,"effect.ue3.wr-244-native.v1","fx_m_mi_k_00.fx_mi.fx_k_pa_ringmaster_01_19_ad","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.ad.f754ad06cfe6",false,"sprite",false,true,true,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_244,DIMENSIONMASTER_WR_PARAMETERS_244,DIMENSIONMASTER_WR_SWITCHES_244},
    {245u,"effect.ue3.wr-245-native.v1","fx_m_mi_k_00.fx_mi.fx_j_me_shine_02_01_ad","fx_m_mi_02.fx_m.fx_d_pa_shine_02_ad","ue3.material.fx.m.mi.02.fx.m.fx.d.pa.shine.02.ad.29d7864aadba",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_245,DIMENSIONMASTER_WR_PARAMETERS_245,DIMENSIONMASTER_WR_SWITCHES_245},
    {246u,"effect.ue3.wr-246-native.v1","bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad","bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.glow.01.ad.0857e02620a0",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_246,DIMENSIONMASTER_WR_PARAMETERS_246,DIMENSIONMASTER_WR_SWITCHES_246},
    {247u,"effect.ue3.wr-247-native.v1","fx_m_mi_j_00.fx_mi.fx_j_pa_shine_01_01_ad","bfx_m_mi_00.bfx_m.bfx_d_pa_shine_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.shine.01.ad.d944bc82b616",false,"sprite",false,true,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_247,DIMENSIONMASTER_WR_PARAMETERS_247,DIMENSIONMASTER_WR_SWITCHES_247},
    {248u,"effect.ue3.wr-248-native.v1","fx_m_mi_j_00.fx_mi.fx_j_pa_movedissolve_01_01_tr","fx_m_mi_j_00.fx_m.fx_j_pa_movedissolve_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.movedissolve.01.tr.1684ec18e9f6",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_248,DIMENSIONMASTER_WR_PARAMETERS_248,DIMENSIONMASTER_WR_SWITCHES_248},
    {251u,"effect.ue3.wr-251-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_turbulence_01_08_tr","fx_m_mi_03.fx_m.fx_d_pa_turbulence_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.turbulence.01.tr.324978c21e60",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_251,DIMENSIONMASTER_WR_PARAMETERS_251,DIMENSIONMASTER_WR_SWITCHES_251},
    {252u,"effect.ue3.wr-252-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_twirl_03_03_ad","fx_m_mi_03.fx_m.fx_d_pa_twirl_05_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.twirl.05.ad.2b8e151c3da1",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_252,DIMENSIONMASTER_WR_PARAMETERS_252,DIMENSIONMASTER_WR_SWITCHES_252},
    {253u,"effect.ue3.wr-253-native.v1","fx_m_mi_02.fx_mi.fx_s_pa_shine_01_1_tr","fx_m_mi_02.fx_m.fx_f_pa_shine_01_0_tr","ue3.material.fx.m.mi.02.fx.m.fx.f.pa.shine.01.0.tr.3d21bd1f3e79",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_253,DIMENSIONMASTER_WR_PARAMETERS_253,DIMENSIONMASTER_WR_SWITCHES_253},
    {254u,"effect.ue3.wr-254-native.v1","fx_m_mi_01.fx_mi.fx_j_pa_ap_23_1_tr","fx_m_mi_02.fx_m.fx_d_pa_shine_02_tr","ue3.material.fx.m.mi.02.fx.m.fx.d.pa.shine.02.tr.62e708fe3f21",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_254,DIMENSIONMASTER_WR_PARAMETERS_254,DIMENSIONMASTER_WR_SWITCHES_254},
    {255u,"effect.ue3.wr-255-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_pa_sinewave_01_04_tr","fx_m_mi_01.fx_m.fx_d_pa_sinewave_01_tr","ue3.material.fx.m.mi.01.fx.m.fx.d.pa.sinewave.01.tr.08569d2a3816",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_255,DIMENSIONMASTER_WR_PARAMETERS_255,DIMENSIONMASTER_WR_SWITCHES_255},
    {256u,"effect.ue3.wr-256-native.v1","fx_m_mi_m_00.fx_mi.fx_m_pa_spritewave_01_7_ad","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_ad","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.ad.caabbddf8b55",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_256,DIMENSIONMASTER_WR_PARAMETERS_256,DIMENSIONMASTER_WR_SWITCHES_256},
    {257u,"effect.ue3.wr-257-native.v1","fx_m_mi_05.fx_m.fx_c_pa_glitter_01_tr","fx_m_mi_05.fx_m.fx_c_pa_glitter_01_tr","ue3.material.fx.m.mi.05.fx.m.fx.c.pa.glitter.01.tr.2f2891f60b47",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_257,DIMENSIONMASTER_WR_PARAMETERS_257,DIMENSIONMASTER_WR_SWITCHES_257},
    {258u,"effect.ue3.wr-258-native.v1","fx_m_mi_w_00.mi.fx_w_me_master_02_243_dt_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",true,"mesh",false,true,true,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_258,DIMENSIONMASTER_WR_PARAMETERS_258,DIMENSIONMASTER_WR_SWITCHES_258},
    {259u,"effect.ue3.wr-259-native.v1","fx_m_mi_w_00.mi.fx_w_pa_master_01_05_dt_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_259,DIMENSIONMASTER_WR_PARAMETERS_259,DIMENSIONMASTER_WR_SWITCHES_259},
    {260u,"effect.ue3.wr-260-native.v1","fx_m_mi_w_00.mi.fx_w_pa_spritewave_01_05_tr","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_260,DIMENSIONMASTER_WR_PARAMETERS_260,DIMENSIONMASTER_WR_SWITCHES_260},
    {262u,"effect.ue3.wr-262-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_db_01_2_ad","fx_mastermaterial.fx_mm.fx_mm_simple_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_262,DIMENSIONMASTER_WR_PARAMETERS_262,DIMENSIONMASTER_WR_SWITCHES_262},
    {263u,"effect.ue3.wr-263-native.v1","fx_m_mi_s_00.fx_mi.fx_s_me_localcrack_01_01_tr","fx_m_mi_j_00.fx_m.fx_j_me_localcrack_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.me.localcrack.01.tr.a0e83a46fef9",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_263,DIMENSIONMASTER_WR_PARAMETERS_263,DIMENSIONMASTER_WR_SWITCHES_263},
    // Exact E/A material identities share the selected native program and input layout.
    {259u,"effect.ue3.e-master-01-04-native.v1","fx_m_mi_w_00.mi.fx_w_pa_master_01_04_dt_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_259,DIMENSIONMASTER_WR_PARAMETERS_259,DIMENSIONMASTER_WR_SWITCHES_259},
    // Exact F material aliases reuse existing native program inputs.
    {210u,"effect.ue3.f-ring-07-64-native.v1","fx_m_mi_03.fx_mi.fx_d_pa_ring_07_64_ad","fx_m_mi_03.fx_m.fx_d_pa_ring_07_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.ad.82e9116584b2",false,"sprite",false,false,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_210,DIMENSIONMASTER_WR_PARAMETERS_210,DIMENSIONMASTER_WR_SWITCHES_210},
    {232u,"effect.ue3.f-ht-20-1-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_ht_20_1_tr","fx_mastermaterial.fx_mm.fx_mm_light_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.tr.8a6435f0c4e0",false,"sprite",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_232,DIMENSIONMASTER_WR_PARAMETERS_232,DIMENSIONMASTER_WR_SWITCHES_232},
    {259u,"effect.ue3.f-master-01-05-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_master_01_05_dt_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",false,"sprite",false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_259,DIMENSIONMASTER_WR_PARAMETERS_259,DIMENSIONMASTER_WR_SWITCHES_259},
    {277u,"effect.ue3.wr-277-native.v1","fx_m_mi_k_00.fx_mi.fx_k_me_makeflow_03_05_tr","fx_m_mi_k_00.fx_m.fx_k_me_makeflow_03_tr","ue3.material.fx.m.mi.k.00.fx.m.fx.k.me.makeflow.03.tr.6e5c0dd36299",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_277,DIMENSIONMASTER_WR_PARAMETERS_277,DIMENSIONMASTER_WR_SWITCHES_277},
    {278u,"effect.ue3.wr-278-native.v1","fx_m_mi_01.fx_m.fx_h_me_swing_01_1_tr","fx_m_mi_01.fx_m.fx_h_me_swing_01_1_tr","ue3.material.fx.m.mi.01.fx.m.fx.h.me.swing.01.1.tr.57320e4ddad0",true,"mesh",false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_278,DIMENSIONMASTER_WR_PARAMETERS_278,DIMENSIONMASTER_WR_SWITCHES_278},
    {279u,"effect.ue3.wr-279-native.v1","fx_m_mi_j_00.fx_mi.fx_j_me_linearflow_02_12_tr","fx_m_mi_j_00.fx_m.fx_j_pa_linearflow_02_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.linearflow.02.tr.ac0bdcfd95f7",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_279,DIMENSIONMASTER_WR_PARAMETERS_279,DIMENSIONMASTER_WR_SWITCHES_279},
    {280u,"effect.ue3.wr-280-native.v1","fx_m_mi_o_00.fx_mi.fx_o_me_makeflow_03_10_tr","fx_m_mi_k_00.fx_m.fx_k_me_makeflow_03_tr","ue3.material.fx.m.mi.k.00.fx.m.fx.k.me.makeflow.03.tr.6e5c0dd36299",true,"mesh",false,false,true,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_WR_TEXTURES_280,DIMENSIONMASTER_WR_PARAMETERS_280,DIMENSIONMASTER_WR_SWITCHES_280},
}};

inline const DIMENSIONMASTER_WR_PROGRAM_DESC* Find_DimensionMasterWRProgram(
    const std::string_view runtimeId)
{
    for (const auto& Program : DIMENSIONMASTER_WR_PROGRAMS)
        if (Program.strRuntimeProfileId == runtimeId) return &Program;
    return nullptr;
}

inline bool Build_DimensionMasterWRParameters(const EFFECT_SOURCE_MATERIAL_DESC& Source,
    std::array<float4_t,32>& Output)
{
    const auto* Program=Find_DimensionMasterWRProgram(Source.strRuntimeShaderProfileId);
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
    if (Program->iProfileIndex == 260u)
    {
        // Source FMaterialUniformExpressionSine evaluates this material-only
        // rotation on the CPU before upload, including edited rotator values.
        const float fDissolveRadians = Candidate[1u].w * 0.25f;
        auto& Rotation = Candidate[DIMENSIONMASTER_WR_260_CPU_ROTATION_ROW];
        Rotation.x = std::cos(fDissolveRadians);
        Rotation.y = std::sin(fDissolveRadians);
    }
    Output=Candidate;
    return true;
}

inline bool Has_DimensionMasterWRMaterialContract(const EFFECT_ELEMENT_DESC& Element)
{
    const auto& Source=Element.Material.SourceMaterial;
    const auto* Program=Find_DimensionMasterWRProgram(Source.strRuntimeShaderProfileId);
    if (!Program || Element.Material.Execution.bEnabled || Element.Material.Execution.bFailClosed ||
        Element.Material.eRenderProfile!=Program->eRenderProfile ||
        Element.eKind!=(Program->strRendererShape=="screenPost" ? EFFECT_ELEMENT_KIND::SCREEN_POST : EFFECT_ELEMENT_KIND::PARTICLE) || !Element.SourceRecipe.bEnabled ||
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
    return Build_DimensionMasterWRParameters(Source,Parameters);
}
NS_END
