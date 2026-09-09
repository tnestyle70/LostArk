#pragma once
// Twenty-five explicit selected V material branches. Engine prefix/fog/depth adapters
// are in Shader_EffectDimensionMasterVNative.hlsli, not unnamed JSON fallbacks.
#include "Effect_AuthoringDocument.h"
#include <array>
#include <span>
#include <string_view>
#include <algorithm>
#include <cmath>

NS_BEGIN(Client)
struct DIMENSIONMASTER_V_PARAMETER_DESC final
{
    std::string_view strName;
    uint32_t iRow;
    uint32_t iLane;
    bool bVector;
};
struct DIMENSIONMASTER_V_SWITCH_DESC final
{
    std::string_view strName;
    bool bValue;
};
struct DIMENSIONMASTER_V_PROGRAM_DESC final
{
    uint32_t iProfileIndex;
    std::string_view strRuntimeProfileId;
    std::string_view strSourceMaterialPath;
    std::string_view strParentMaterialPath;
    std::string_view strProfileId;
    bool bMesh;
    bool bScreenPost;
    bool bRequiresSourceUV1;
    bool bRequiresTangentView;
    bool bRequiresClipW;
    bool bRequiresSceneColor;
    bool bRequiresDepth;
    EFFECT_RENDER_PROFILE eRenderProfile;
    std::span<const std::string_view> TextureNames;
    std::span<const DIMENSIONMASTER_V_PARAMETER_DESC> Parameters;
    std::span<const DIMENSIONMASTER_V_SWITCH_DESC> StaticSwitches;
};


inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_V_TEXTURES_52 = {{"lensflaretexture"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,6> DIMENSIONMASTER_V_PARAMETERS_52 = {{
    {"selectioncolor", 2u, 0u, true},
    {"x tiling", 0u, 3u, false},
    {"y tiling", 1u, 0u, false},
    {"select texture", 0u, 2u, false},
    {"desaturation", 0u, 0u, false},
    {"opacity", 0u, 1u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,0> DIMENSIONMASTER_V_SWITCHES_52 = {{
}};

inline constexpr std::array<std::string_view,0> DIMENSIONMASTER_V_TEXTURES_53 = {{}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,6> DIMENSIONMASTER_V_PARAMETERS_53 = {{
    {"selectioncolor", 2u, 0u, true},
    {"01.radius", 0u, 0u, false},
    {"02.hardness", 0u, 1u, false},
    {"03.spherepower", 0u, 2u, false},
    {"04.sphere_str", 0u, 3u, false},
    {"depthbiasdalpha_bias", 1u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,4> DIMENSIONMASTER_V_SWITCHES_53 = {{
    {"01.checkisvertexcolor", true},
    {"02.usedynamicparam", false},
    {"30.usefresnal", false},
    {"checkisdepthbiasalpha", true},
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_V_TEXTURES_54 = {{"01.map"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,6> DIMENSIONMASTER_V_PARAMETERS_54 = {{
    {"selectioncolor", 2u, 0u, true},
    {"01.circlepower", 0u, 0u, false},
    {"02.circlestr", 0u, 2u, false},
    {"02.circlemaskpower", 0u, 1u, false},
    {"03.circlestr", 0u, 3u, false},
    {"depthbiasdalpha_bias", 1u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,4> DIMENSIONMASTER_V_SWITCHES_54 = {{
    {"checkisdepthbiasalpha", true},
    {"00.useradius", true},
    {"01.usemesh", false},
    {"10.usetimebezire", true},
}};

inline constexpr std::array<std::string_view,4> DIMENSIONMASTER_V_TEXTURES_55 = {{"31.map_e","06.map","04.map_anew","00.map_b"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,18> DIMENSIONMASTER_V_PARAMETERS_55 = {{
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
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,18> DIMENSIONMASTER_V_SWITCHES_55 = {{
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

inline constexpr std::array<std::string_view,2> DIMENSIONMASTER_V_TEXTURES_56 = {{"uv_noise_tex","emissive_tex"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,10> DIMENSIONMASTER_V_PARAMETERS_56 = {{
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
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,5> DIMENSIONMASTER_V_SWITCHES_56 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_tex", false},
    {"use_emissive_vertcolor", false},
    {"use_nonuvnoise", false},
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_V_TEXTURES_57 = {{"emissive_tex"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,1> DIMENSIONMASTER_V_PARAMETERS_57 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,3> DIMENSIONMASTER_V_SWITCHES_57 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", false},
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_V_TEXTURES_58 = {{"emissive_tex"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,5> DIMENSIONMASTER_V_PARAMETERS_58 = {{
    {"selectioncolor", 1u, 0u, true},
    {"uv_scale", 0u, 3u, false},
    {"dynamic_parameter_explanation", 0u, 0u, false},
    {"emissive_desaturation", 0u, 1u, false},
    {"emissive_power", 0u, 2u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,17> DIMENSIONMASTER_V_SWITCHES_58 = {{
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

inline constexpr std::array<std::string_view,2> DIMENSIONMASTER_V_TEXTURES_59 = {{"uv_noise_01_tex","emissive_tex"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,12> DIMENSIONMASTER_V_PARAMETERS_59 = {{
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
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,18> DIMENSIONMASTER_V_SWITCHES_59 = {{
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

inline constexpr std::array<std::string_view,5> DIMENSIONMASTER_V_TEXTURES_60 = {{"06.map","02.map_e","01.map_a","11.map_b","21.map_c"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,31> DIMENSIONMASTER_V_PARAMETERS_60 = {{
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
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,26> DIMENSIONMASTER_V_SWITCHES_60 = {{
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

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_V_TEXTURES_61 = {{"emissive_tex"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,1> DIMENSIONMASTER_V_PARAMETERS_61 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,3> DIMENSIONMASTER_V_SWITCHES_61 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_chanel", false},
}};

inline constexpr std::array<std::string_view,0> DIMENSIONMASTER_V_TEXTURES_62 = {{}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,1> DIMENSIONMASTER_V_PARAMETERS_62 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,0> DIMENSIONMASTER_V_SWITCHES_62 = {{
}};

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_V_TEXTURES_63 = {{"14.map_d","01.map_a","06.map_b"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,19> DIMENSIONMASTER_V_PARAMETERS_63 = {{
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
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,6> DIMENSIONMASTER_V_SWITCHES_63 = {{
    {"checkisdepthbiasalpha", true},
    {"00.checkisnoiseon", true},
    {"12.uvnoise", true},
    {"11.uvmirroring", false},
    {"use_meshtype", false},
    {"use_multyply_noisecolor", false},
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_V_TEXTURES_64 = {{"main_tex"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,5> DIMENSIONMASTER_V_PARAMETERS_64 = {{
    {"selectioncolor", 1u, 0u, true},
    {"maintex_coordx", 0u, 2u, false},
    {"maintex_coordy", 0u, 3u, false},
    {"bright", 0u, 0u, false},
    {"desaturation", 0u, 1u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,4> DIMENSIONMASTER_V_SWITCHES_64 = {{
    {"use_5waysplit", false},
    {"use_alphach", false},
    {"use_maintex_paningtile", false},
    {"split_type", true},
}};

inline constexpr std::array<std::string_view,0> DIMENSIONMASTER_V_TEXTURES_65 = {{}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,1> DIMENSIONMASTER_V_PARAMETERS_65 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,0> DIMENSIONMASTER_V_SWITCHES_65 = {{
}};

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_V_TEXTURES_66 = {{"normal_tex","refle_tex","native_texture_2"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,17> DIMENSIONMASTER_V_PARAMETERS_66 = {{
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
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,2> DIMENSIONMASTER_V_SWITCHES_66 = {{
    {"use_depth", false},
    {"use_dissolve", false},
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_V_TEXTURES_67 = {{"slice_flow_texture"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,11> DIMENSIONMASTER_V_PARAMETERS_67 = {{
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
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,0> DIMENSIONMASTER_V_SWITCHES_67 = {{
}};

inline constexpr std::array<std::string_view,0> DIMENSIONMASTER_V_TEXTURES_68 = {{}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,10> DIMENSIONMASTER_V_PARAMETERS_68 = {{
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
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,0> DIMENSIONMASTER_V_SWITCHES_68 = {{
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_V_TEXTURES_69 = {{"native_texture_0"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,1> DIMENSIONMASTER_V_PARAMETERS_69 = {{
    {"selectioncolor", 0u, 0u, true},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,0> DIMENSIONMASTER_V_SWITCHES_69 = {{
}};

inline constexpr std::array<std::string_view,5> DIMENSIONMASTER_V_TEXTURES_70 = {{"06.map","12.map_c","22.map_a","02.map_e","01.specmap"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,35> DIMENSIONMASTER_V_PARAMETERS_70 = {{
    {"selectioncolor", 11u, 0u, true},
    {"05.specmap_uvscale.x", 2u, 0u, false},
    {"06.specmap_uvscale.y", 2u, 2u, false},
    {"19.cmap.direct", 5u, 0u, false},
    {"meshemitterdynamicparameter", 10u, 0u, true},
    {"22.map_a_uvscale_r", 5u, 1u, false},
    {"23.map_a_uvscale_g", 5u, 2u, false},
    {"09.specmap_color", 8u, 0u, true},
    {"93.emissiion_color", 9u, 0u, true},
    {"02.radius", 0u, 0u, false},
    {"03.hardness", 0u, 3u, false},
    {"09.power", 4u, 0u, false},
    {"14.uvscale.x", 4u, 3u, false},
    {"time", 7u, 2u, false},
    {"09.map_d_panning_x", 3u, 3u, false},
    {"07.map_d_uvscale_r", 3u, 0u, false},
    {"08.map_d_uvscale_g", 3u, 1u, false},
    {"10.map_d_panning_y", 4u, 1u, false},
    {"05.distort_str", 1u, 2u, false},
    {"11.uv.curvature", 4u, 2u, false},
    {"24.map_a_panning_x", 5u, 3u, false},
    {"05.map_e_panning_x", 1u, 3u, false},
    {"03.map_e_uvscale_r", 1u, 0u, false},
    {"04.map_e_uvscale_g", 1u, 1u, false},
    {"06.map_e_panning_y", 2u, 1u, false},
    {"03.distortionrate", 0u, 2u, false},
    {"02.specmap_str", 0u, 1u, false},
    {"07.desaturation", 2u, 3u, false},
    {"08.specmap_power", 3u, 2u, false},
    {"91.desaturation", 6u, 2u, false},
    {"92.emissiion_power", 6u, 3u, false},
    {"95.str", 7u, 0u, false},
    {"96.power", 7u, 1u, false},
    {"32.fresnal_power", 6u, 0u, false},
    {"33.fresnal_str", 6u, 1u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,27> DIMENSIONMASTER_V_SWITCHES_70 = {{
    {"00.usespecullar", true},
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
    {"01.effectonalphatex", true},
    {"11.useclamptexmap", true},
    {"91.mapch.r", false},
    {"92.mapch.g", false},
    {"00.checkisdepthbiasalpha", false},
    {"21.usephasetexmap", true},
    {"30.usefresnal", true},
    {"31.fresnal.invert", false},
    {"00.use_uvdistort", true},
    {"29.usedynamic_ypanning", true},
    {"23.usedynamicstr", false},
    {"93.mapch.b", true},
    {"10.fresnal", true},
    {"99.alphadissolve", false},
    {"94.mapch.a", true},
}};

inline constexpr std::array<std::string_view,3> DIMENSIONMASTER_V_TEXTURES_71 = {{"diff_tex","diff_noise_tex","a_mask_tex"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,26> DIMENSIONMASTER_V_PARAMETERS_71 = {{
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
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,13> DIMENSIONMASTER_V_SWITCHES_71 = {{
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

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_V_TEXTURES_72 = {{"texture"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,3> DIMENSIONMASTER_V_PARAMETERS_72 = {{
    {"selectioncolor", 1u, 0u, true},
    {"tex_intensity", 0u, 1u, false},
    {"glimmer_opacity", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,0> DIMENSIONMASTER_V_SWITCHES_72 = {{
}};

inline constexpr std::array<std::string_view,2> DIMENSIONMASTER_V_TEXTURES_73 = {{"12.map_f","02.map_e"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,22> DIMENSIONMASTER_V_PARAMETERS_73 = {{
    {"selectioncolor", 6u, 0u, true},
    {"93.emissiion_color", 5u, 0u, true},
    {"time", 4u, 3u, false},
    {"15.map_e_panning_x", 3u, 1u, false},
    {"13.map_e_uvscale_r", 2u, 3u, false},
    {"14.map_e_uvscale_g", 3u, 0u, false},
    {"11.uv.curvature", 2u, 2u, false},
    {"16.map_e_panning_y", 3u, 2u, false},
    {"05.map_e_panning_x", 1u, 1u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 0u, false},
    {"06.map_e_panning_y", 1u, 2u, false},
    {"91.desaturation", 3u, 3u, false},
    {"92.emissiion_power", 4u, 0u, false},
    {"06.radius", 1u, 3u, false},
    {"07.hardness", 2u, 0u, false},
    {"02.radius", 0u, 1u, false},
    {"03.hardness", 0u, 2u, false},
    {"09.power", 2u, 1u, false},
    {"95.str", 4u, 1u, false},
    {"96.power", 4u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,27> DIMENSIONMASTER_V_SWITCHES_73 = {{
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
    {"00.use_uvdistort", false},
    {"29.usedynamic_ypanning", false},
    {"23.usedynamicstr", false},
    {"93.mapch.b", true},
    {"10.fresnal", true},
    {"99.alphadissolve", false},
    {"94.mapch.a", true},
}};

inline constexpr std::array<std::string_view,2> DIMENSIONMASTER_V_TEXTURES_74 = {{"12.map_f","02.map_e"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,22> DIMENSIONMASTER_V_PARAMETERS_74 = {{
    {"selectioncolor", 6u, 0u, true},
    {"93.emissiion_color", 5u, 0u, true},
    {"time", 4u, 3u, false},
    {"15.map_e_panning_x", 3u, 1u, false},
    {"13.map_e_uvscale_r", 2u, 3u, false},
    {"14.map_e_uvscale_g", 3u, 0u, false},
    {"11.uv.curvature", 2u, 2u, false},
    {"16.map_e_panning_y", 3u, 2u, false},
    {"05.map_e_panning_x", 1u, 1u, false},
    {"03.map_e_uvscale_r", 0u, 3u, false},
    {"04.map_e_uvscale_g", 1u, 0u, false},
    {"06.map_e_panning_y", 1u, 2u, false},
    {"91.desaturation", 3u, 3u, false},
    {"92.emissiion_power", 4u, 0u, false},
    {"06.radius", 1u, 3u, false},
    {"07.hardness", 2u, 0u, false},
    {"02.radius", 0u, 1u, false},
    {"03.hardness", 0u, 2u, false},
    {"09.power", 2u, 1u, false},
    {"95.str", 4u, 1u, false},
    {"96.power", 4u, 2u, false},
    {"01.depthbiasdalpha_bias", 0u, 0u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,27> DIMENSIONMASTER_V_SWITCHES_74 = {{
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
    {"00.use_uvdistort", false},
    {"29.usedynamic_ypanning", false},
    {"23.usedynamicstr", false},
    {"93.mapch.b", true},
    {"10.fresnal", true},
    {"99.alphadissolve", false},
    {"94.mapch.a", true},
}};

inline constexpr std::array<std::string_view,1> DIMENSIONMASTER_V_TEXTURES_75 = {{"slice_flow_texture"}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,11> DIMENSIONMASTER_V_PARAMETERS_75 = {{
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
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,0> DIMENSIONMASTER_V_SWITCHES_75 = {{
}};

inline constexpr std::array<std::string_view,0> DIMENSIONMASTER_V_TEXTURES_76 = {{}};
inline constexpr std::array<DIMENSIONMASTER_V_PARAMETER_DESC,4> DIMENSIONMASTER_V_PARAMETERS_76 = {{
    {"selectioncolor", 1u, 0u, true},
    {"circle radius", 0u, 0u, false},
    {"circleedge hardness", 0u, 1u, false},
    {"opacity", 0u, 2u, false},
}};
inline constexpr std::array<DIMENSIONMASTER_V_SWITCH_DESC,3> DIMENSIONMASTER_V_SWITCHES_76 = {{
    {"use_particle", true},
    {"use_invertedcirclealpha", false},
    {"use_circlealpha", true},
}};

inline constexpr std::array<DIMENSIONMASTER_V_PROGRAM_DESC,26> DIMENSIONMASTER_V_PROGRAMS = {{
    {52u,"effect.ue3.v-bfx-c-pa-lightflare-01-ddt-4-ad-native.v1","bfx_m_mi_00.bfx_mi.bfx_c_pa_lightflare_01_ddt_4_ad","bfx_m_mi_00.bfx_m.bfx_c_pa_lightflare_01_ddt_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.c.pa.lightflare.01.ddt.ad.33b058471d6a",false,false,false,false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_52,DIMENSIONMASTER_V_PARAMETERS_52,DIMENSIONMASTER_V_SWITCHES_52},
    {53u,"effect.ue3.v-bfx-d-pa-circ-01-01-dt-ad-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_pa_circ_01_01_dt_ad","bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.circ.01.ad.0f4a4414a90a",false,false,false,false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_53,DIMENSIONMASTER_V_PARAMETERS_53,DIMENSIONMASTER_V_SWITCHES_53},
    {54u,"effect.ue3.v-bfx-d-pa-flar-02-01-ad-native.v1","bfx_m_mi_00.bfx_mi.bfx_d_pa_flar_02_01_ad","fx_m_mi_00.fx_m.fx_d_pa_flare_02_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.flare.02.ad.7d386b4627a6",false,false,false,false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_54,DIMENSIONMASTER_V_PARAMETERS_54,DIMENSIONMASTER_V_SWITCHES_54},
    {55u,"effect.ue3.v-bfx-j-pa-ring-07-06-ad-native.v1","bfx_m_mi_00.bfx_mi.bfx_j_pa_ring_07_06_ad","fx_m_mi_03.fx_m.fx_d_pa_ring_07_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.ad.82e9116584b2",false,false,false,false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_55,DIMENSIONMASTER_V_PARAMETERS_55,DIMENSIONMASTER_V_SWITCHES_55},
    {56u,"effect.ue3.v-fx-a-pa-gl-01-9-ad-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_gl_01_9_ad","fx_mastermaterial.fx_mm.fx_mm_simple_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",false,false,false,false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_56,DIMENSIONMASTER_V_PARAMETERS_56,DIMENSIONMASTER_V_SWITCHES_56},
    {57u,"effect.ue3.v-fx-d-pa-atta-05-07-ad-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_atta_05_07_ad","fx_mastermaterial.fx_mm.fx_mm_light_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.ad.f431613b2bdf",false,false,false,false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_57,DIMENSIONMASTER_V_PARAMETERS_57,DIMENSIONMASTER_V_SWITCHES_57},
    {58u,"effect.ue3.v-fx-d-pa-atta-09-02-ad-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_atta_09_02_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,false,false,false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_58,DIMENSIONMASTER_V_PARAMETERS_58,DIMENSIONMASTER_V_SWITCHES_58},
    {59u,"effect.ue3.v-fx-d-pa-atta-09-04-tr-native.v1","fx_m_mi_00.fx_mi.fx_d_pa_atta_09_04_tr","fx_mastermaterial.fx_mm.fx_mm_basic_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",false,false,false,false,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_59,DIMENSIONMASTER_V_PARAMETERS_59,DIMENSIONMASTER_V_SWITCHES_59},
    {60u,"effect.ue3.v-fx-j-me-ringrainbow-01-2-ts-tr-native.v1","fx_m_mi_01.fx_mi.fx_j_me_ringrainbow_01_2_ts_tr","fx_m_mi_00.fx_m.fx_d_pa_master_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",true,false,false,true,false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_60,DIMENSIONMASTER_V_PARAMETERS_60,DIMENSIONMASTER_V_SWITCHES_60},
    {61u,"effect.ue3.v-fx-j-pa-chromaring-01-ad-native.v1","fx_m_mi_01.fx_mi.fx_j_pa_chromaring_01_ad","fx_mastermaterial.fx_mm.fx_mm_light_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.ad.f431613b2bdf",false,false,false,false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_61,DIMENSIONMASTER_V_PARAMETERS_61,DIMENSIONMASTER_V_SWITCHES_61},
    {62u,"effect.ue3.v-fx-j-pa-circledisort-01-ad-native.v1","fx_m_mi_02.fx_m.fx_j_pa_circledisort_01_ad","fx_m_mi_02.fx_m.fx_j_pa_circledisort_01_ad","ue3.material.fx.m.mi.02.fx.m.fx.j.pa.circledisort.01.ad.8852527671d6",false,false,false,false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_62,DIMENSIONMASTER_V_PARAMETERS_62,DIMENSIONMASTER_V_SWITCHES_62},
    {63u,"effect.ue3.v-fx-e-pa-ht-18-4-tr-native.v1","fx_m_mi_02.fx_mi.fx_e_pa_ht_18_4_tr","fx_m_mi_02.fx_m.fx_f_pa_shine_01_0_tr","ue3.material.fx.m.mi.02.fx.m.fx.f.pa.shine.01.0.tr.3d21bd1f3e79",false,false,false,false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_63,DIMENSIONMASTER_V_PARAMETERS_63,DIMENSIONMASTER_V_SWITCHES_63},
    {64u,"effect.ue3.v-fx-j-rgbsplit-01-2-ad-native.v1","fx_m_mi_02.fx_mi.fx_j_rgbsplit_01_2_ad","fx_m_mi_02.fx_m.fx_j_rgbsplit_01_ad","ue3.material.fx.m.mi.02.fx.m.fx.j.rgbsplit.01.ad.b59195d23f11",false,false,false,false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_64,DIMENSIONMASTER_V_PARAMETERS_64,DIMENSIONMASTER_V_SWITCHES_64},
    {65u,"effect.ue3.v-fx-d-pa-flare-03-ad-native.v1","fx_m_mi_d_00.fx_m.fx_d_pa_flare_03_ad","fx_m_mi_d_00.fx_m.fx_d_pa_flare_03_ad","ue3.material.fx.m.mi.d.00.fx.m.fx.d.pa.flare.03.ad.4d380200a8e7",false,false,false,false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_65,DIMENSIONMASTER_V_PARAMETERS_65,DIMENSIONMASTER_V_SWITCHES_65},
    {66u,"effect.ue3.v-fx-j-me-localcrack-01-07-tr-native.v1","fx_m_mi_j_00.fx_mi.fx_j_me_localcrack_01_07_tr","fx_m_mi_j_00.fx_m.fx_j_me_localcrack_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.me.localcrack.01.tr.a0e83a46fef9",true,false,false,true,true,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_66,DIMENSIONMASTER_V_PARAMETERS_66,DIMENSIONMASTER_V_SWITCHES_66},
    {67u,"effect.ue3.v-fx-j-pa-slice-01-07-tr-native.v1","fx_m_mi_j_00.fx_mi.fx_j_pa_slice_01_07_tr","fx_m_mi_j_00.fx_m.fx_j_pa_slice_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.slice.01.tr.afc439ecf232",false,false,false,false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_67,DIMENSIONMASTER_V_PARAMETERS_67,DIMENSIONMASTER_V_SWITCHES_67},
    {68u,"effect.ue3.v-fx-j-po-rgbnoise-01-01-tr-native.v1","fx_m_mi_j_00.fx_mi.fx_j_po_rgbnoise_01_01_tr","fx_m_mi_j_00.fx_m.fx_j_po_rgbnoise_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.po.rgbnoise.01.tr.81291483aea9",false,true,false,false,true,true,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_68,DIMENSIONMASTER_V_PARAMETERS_68,DIMENSIONMASTER_V_SWITCHES_68},
    {69u,"effect.ue3.v-fx-o-pa-splitline-02-ad-native.v1","fx_m_mi_o_00.fx_m.fx_o_pa_splitline_02_ad","fx_m_mi_o_00.fx_m.fx_o_pa_splitline_02_ad","ue3.material.fx.m.mi.o.00.fx.m.fx.o.pa.splitline.02.ad.84f529b72b79",false,false,false,false,true,true,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_69,DIMENSIONMASTER_V_PARAMETERS_69,DIMENSIONMASTER_V_SWITCHES_69},
    {70u,"effect.ue3.v-fx-r-me-ringmaster-11-01-ts-fs-ad-native.v1","fx_m_mi_r_00.fx_mi.fx_r_me_ringmaster_11_01_ts_fs_ad","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.ad.f754ad06cfe6",true,false,false,true,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_70,DIMENSIONMASTER_V_PARAMETERS_70,DIMENSIONMASTER_V_SWITCHES_70},
    {71u,"effect.ue3.v-fx-r-pa-customparticle-02-01-ad-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_customparticle_02_01_ad","fx_m_mi_j_00.fx_m.fx_j_pa_customparticle_01_ad","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.customparticle.01.ad.e6b959010967",false,false,false,false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_71,DIMENSIONMASTER_V_PARAMETERS_71,DIMENSIONMASTER_V_SWITCHES_71},
    {72u,"effect.ue3.v-fx-r-pa-ri-04-02-ad-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_ri_04_02_ad","fx_m_mi_01.fx_m.fx_f_pa_ri_01_ad","ue3.material.fx.m.mi.01.fx.m.fx.f.pa.ri.01.ad.9e58d8091d6a",false,false,false,false,false,false,false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_72,DIMENSIONMASTER_V_PARAMETERS_72,DIMENSIONMASTER_V_SWITCHES_72},
    {73u,"effect.ue3.v-fx-r-pa-ringmaster-12-03-dt-ad-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_ringmaster_12_03_dt_ad","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.ad.f754ad06cfe6",false,false,false,false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_73,DIMENSIONMASTER_V_PARAMETERS_73,DIMENSIONMASTER_V_SWITCHES_73},
    {74u,"effect.ue3.v-fx-r-pa-ringmaster-12-04-dt-ad-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_ringmaster_12_04_dt_ad","fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.ad.f754ad06cfe6",false,false,false,false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_74,DIMENSIONMASTER_V_PARAMETERS_74,DIMENSIONMASTER_V_SWITCHES_74},
    {75u,"effect.ue3.v-fx-r-pa-slice-01-02-tr-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_slice_01_02_tr","fx_m_mi_j_00.fx_m.fx_j_pa_slice_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.slice.01.tr.afc439ecf232",false,false,false,false,true,false,true,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_75,DIMENSIONMASTER_V_PARAMETERS_75,DIMENSIONMASTER_V_SWITCHES_75},
    {76u,"effect.ue3.v-fx-c-pa-zoomblur-01-tr-native.v1","fx_post.fx_mi.fx_c_pa_zoomblur_01_tr","fx_post.fx_m.fx_c_po_zoomblur_01","ue3.material.fx.post.fx.m.fx.c.po.zoomblur.01.7068167af900",false,true,false,false,true,true,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_76,DIMENSIONMASTER_V_PARAMETERS_76,DIMENSIONMASTER_V_SWITCHES_76},
    // Exact F material aliases reuse existing native program inputs.
    {54u,"effect.ue3.f-flare-07-02-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_flare_07_02_dt_ad","fx_m_mi_00.fx_m.fx_d_pa_flare_02_ad","ue3.material.fx.m.mi.00.fx.m.fx.d.pa.flare.02.ad.7d386b4627a6",false,false,false,false,true,false,true,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_V_TEXTURES_54,DIMENSIONMASTER_V_PARAMETERS_54,DIMENSIONMASTER_V_SWITCHES_54},
}};

inline const DIMENSIONMASTER_V_PROGRAM_DESC* Find_DimensionMasterVProgram(
    const std::string_view runtimeId)
{
    for (const auto& Program : DIMENSIONMASTER_V_PROGRAMS)
        if (Program.strRuntimeProfileId == runtimeId) return &Program;
    return nullptr;
}

inline bool Build_DimensionMasterVParameters(const EFFECT_SOURCE_MATERIAL_DESC& Source,
    std::array<float4_t,32>& Output)
{
    const auto* Program=Find_DimensionMasterVProgram(Source.strRuntimeShaderProfileId);
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

inline bool Has_DimensionMasterVMaterialContract(const EFFECT_ELEMENT_DESC& Element)
{
    const auto& Source=Element.Material.SourceMaterial;
    const auto* Program=Find_DimensionMasterVProgram(Source.strRuntimeShaderProfileId);
    if (!Program || Element.Material.Execution.bEnabled || Element.Material.Execution.bFailClosed ||
        Element.Material.eRenderProfile!=Program->eRenderProfile ||
        Element.eKind!=(Program->bScreenPost ? EFFECT_ELEMENT_KIND::SCREEN_POST : EFFECT_ELEMENT_KIND::PARTICLE) ||
        !Element.SourceRecipe.bEnabled ||
        Element.Material.strSourceMaterialPath!=Program->strSourceMaterialPath ||
        Element.SourceRecipe.strRendererShape!=(Program->bScreenPost ? "screenPost" : Program->bMesh ? "mesh" : "sprite")) return false;
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
    return Build_DimensionMasterVParameters(Source,Parameters);
}
NS_END
