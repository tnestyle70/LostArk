constexpr std::array<std::string_view,5> DIMENSIONMASTER_Q_TEXTURES_44 = {{"diffuse_tex","flow_tex","mask_b_tex","mask_a_tex","dissolve_tex"}};
constexpr std::array<DIMENSIONMASTER_Q_PARAMETER_DESC,62> DIMENSIONMASTER_Q_PARAMETERS_44 = {{
    {"selectioncolor", 18u, 0u, true},
    {"diff_color", 15u, 0u, true},
    {"meshemitterdynamicparameter", 17u, 0u, true},
    {"diff_rot", 2u, 3u, false},
    {"mask_b_rot", 11u, 0u, false},
    {"mask_a_rot", 8u, 3u, false},
    {"mask_color", 16u, 0u, true},
    {"flow01_tile_x", 6u, 1u, false},
    {"flow01_tile_y", 6u, 2u, false},
    {"flow01_pan_x", 5u, 2u, false},
    {"flow01_pan_y", 5u, 3u, false},
    {"flow01_str", 6u, 0u, false},
    {"flow02_tile_x", 7u, 2u, false},
    {"flow02_tile_y", 7u, 3u, false},
    {"flow02_pan_x", 6u, 3u, false},
    {"flow02_pan_y", 7u, 0u, false},
    {"flow02_str", 7u, 1u, false},
    {"diff_flow_str", 1u, 3u, false},
    {"diff_dypan_x", 1u, 1u, false},
    {"diff_dypan_y", 1u, 2u, false},
    {"diff_tile_x", 3u, 1u, false},
    {"diff_tile_y", 3u, 2u, false},
    {"diff_offset_x", 2u, 0u, false},
    {"diff_offset_y", 2u, 1u, false},
    {"diff_pow", 2u, 2u, false},
    {"diff_str", 3u, 0u, false},
    {"02.uv_xscale", 0u, 2u, false},
    {"02.uv_yscale", 0u, 3u, false},
    {"maskb_dypan_x", 13u, 1u, false},
    {"maskb_dypan_y", 13u, 2u, false},
    {"maskb_flow_strength", 13u, 3u, false},
    {"mask_b_tile_x", 11u, 2u, false},
    {"mask_b_tile_y", 11u, 3u, false},
    {"mask_b_pan_x", 10u, 1u, false},
    {"mask_b_offset_x", 9u, 3u, false},
    {"mask_b_pan_y", 10u, 2u, false},
    {"mask_b_offset_y", 10u, 0u, false},
    {"mask_b_pow", 10u, 3u, false},
    {"mask_b_str", 11u, 1u, false},
    {"01.uv_xscale", 0u, 0u, false},
    {"01.uv_yscale", 0u, 1u, false},
    {"maska_dypan_x", 12u, 2u, false},
    {"maska_dypan_y", 12u, 3u, false},
    {"maska_flow_strength", 13u, 0u, false},
    {"mask_a_tile_x", 9u, 1u, false},
    {"mask_a_tile_y", 9u, 2u, false},
    {"mask_a_offset_x", 8u, 0u, false},
    {"mask_a_offset_y", 8u, 1u, false},
    {"mask_a_pow", 8u, 2u, false},
    {"mask_a_str", 9u, 0u, false},
    {"desaturation", 1u, 0u, false},
    {"emissive_str", 5u, 1u, false},
    {"emissive_pow", 5u, 0u, false},
    {"mask_radius", 12u, 1u, false},
    {"mask_density", 12u, 0u, false},
    {"spheremask_str_max", 14u, 0u, false},
    {"spheremask_str_min", 14u, 1u, false},
    {"dissolve_pan_x", 4u, 0u, false},
    {"dissolve_tile_x", 4u, 2u, false},
    {"dissolve_tile_y", 4u, 3u, false},
    {"dissolve_pan_y", 4u, 1u, false},
    {"dissolve_hardness", 3u, 3u, false},
}};
constexpr std::array<DIMENSIONMASTER_Q_SWITCH_DESC,10> DIMENSIONMASTER_Q_SWITCHES_44 = {{
    {"99.use_flow02_radial", false},
    {"99.use_flow01_radial", false},
    {"99.use_mask_b_radial", false},
    {"99.use_mask_a_radial", false},
    {"use_mesh", true},
    {"99.use_diff_radial", false},
    {"use_radialmask", true},
    {"use_worldoffset", false},
    {"use_dissolve", true},
    {"use_depth", false},
}};

constexpr std::array<std::string_view,1> DIMENSIONMASTER_Q_TEXTURES_45 = {{"emissive_tex"}};
constexpr std::array<DIMENSIONMASTER_Q_PARAMETER_DESC,6> DIMENSIONMASTER_Q_PARAMETERS_45 = {{
    {"selectioncolor", 2u, 0u, true},
    {"uv_scale", 1u, 0u, false},
    {"dynamic_parameter_explanation", 0u, 1u, false},
    {"emissive_desaturation", 0u, 2u, false},
    {"emissive_power", 0u, 3u, false},
    {"depth_alpha_bias", 0u, 0u, false},
}};
constexpr std::array<DIMENSIONMASTER_Q_SWITCH_DESC,17> DIMENSIONMASTER_Q_SWITCHES_45 = {{
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

constexpr std::array<std::string_view,2> DIMENSIONMASTER_Q_TEXTURES_46 = {{"uv_noise_tex","alpha_tex"}};
constexpr std::array<DIMENSIONMASTER_Q_PARAMETER_DESC,26> DIMENSIONMASTER_Q_PARAMETERS_46 = {{
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
constexpr std::array<DIMENSIONMASTER_Q_SWITCH_DESC,4> DIMENSIONMASTER_Q_SWITCHES_46 = {{
    {"use_dissolve_tex", false},
    {"use_mesh", false},
    {"dissolve_rampmap", true},
    {"use_fresnelalpha", false},
}};

constexpr std::array<std::string_view,0> DIMENSIONMASTER_Q_TEXTURES_47 = {{}};
constexpr std::array<DIMENSIONMASTER_Q_PARAMETER_DESC,6> DIMENSIONMASTER_Q_PARAMETERS_47 = {{
    {"selectioncolor", 2u, 0u, true},
    {"centerglow_power", 0u, 0u, false},
    {"centerglow_str", 0u, 1u, false},
    {"glow_power", 0u, 2u, false},
    {"str", 1u, 0u, false},
    {"power", 0u, 3u, false},
}};
constexpr std::array<DIMENSIONMASTER_Q_SWITCH_DESC,0> DIMENSIONMASTER_Q_SWITCHES_47 = {{
}};

constexpr std::array<std::string_view,7> DIMENSIONMASTER_Q_TEXTURES_48 = {{"02_diffnoise_tex","01_diff_tex","01_alphabase_tex","02_alphaflow_noise_tex","03_alphaflow_noise_tex","01_alphaflow_tex","native_texture_6"}};
constexpr std::array<DIMENSIONMASTER_Q_PARAMETER_DESC,32> DIMENSIONMASTER_Q_PARAMETERS_48 = {{
    {"selectioncolor", 8u, 0u, true},
    {"02_diffnoise_pan_x", 4u, 1u, false},
    {"02_diffnoise_tile_u", 5u, 0u, false},
    {"02_diffnoise_tile_v", 5u, 1u, false},
    {"02_diffnoise_pan_y", 4u, 2u, false},
    {"02_diffnoise_str", 4u, 3u, false},
    {"01_diff_tile_u", 2u, 2u, false},
    {"01_diff_tile_v", 2u, 3u, false},
    {"01_diff_offset_x", 1u, 2u, false},
    {"01_diff_offset_y", 1u, 3u, false},
    {"01_diff_pow", 2u, 0u, false},
    {"01_diff_str", 2u, 1u, false},
    {"01_alphabase_tile_u", 0u, 2u, false},
    {"01_alphabase_tile_v", 0u, 3u, false},
    {"01_alphabase_offset_x", 0u, 0u, false},
    {"01_alphabase_offset_y", 0u, 1u, false},
    {"alphabase_str", 7u, 0u, false},
    {"alphabase_hardness", 6u, 3u, false},
    {"02_alphaflow_tile_u", 3u, 3u, false},
    {"02_alphaflow_tile_v", 4u, 0u, false},
    {"02_alphaflow_offset_x", 3u, 1u, false},
    {"02_alphaflow_offset_y", 3u, 2u, false},
    {"02_alphaflow_noise_str", 3u, 0u, false},
    {"03_alphaflow_tile_u", 6u, 1u, false},
    {"03_alphaflow_tile_v", 6u, 2u, false},
    {"03_alphaflow_offset_x", 5u, 3u, false},
    {"03_alphaflow_offset_y", 6u, 0u, false},
    {"03_alphaflow_noise_str", 5u, 2u, false},
    {"01_alphaflow_str", 1u, 1u, false},
    {"01_alphaflow_hardness", 1u, 0u, false},
    {"desaturation", 7u, 2u, false},
    {"depth", 7u, 1u, false},
}};
constexpr std::array<DIMENSIONMASTER_Q_SWITCH_DESC,7> DIMENSIONMASTER_Q_SWITCHES_48 = {{
    {"alphaflow_noise_addtexture", true},
    {"03_alphaflow_pan_use", false},
    {"02_alphaflow_pan_use", false},
    {"edge_soft_use", true},
    {"atan2_use", false},
    {"02_alphaflow_rot_deactiv", true},
    {"fakevolume_use", false},
}};

constexpr std::array<std::string_view,2> DIMENSIONMASTER_Q_TEXTURES_49 = {{"uv_noise_tex","emissive_tex"}};
constexpr std::array<DIMENSIONMASTER_Q_PARAMETER_DESC,10> DIMENSIONMASTER_Q_PARAMETERS_49 = {{
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
constexpr std::array<DIMENSIONMASTER_Q_SWITCH_DESC,5> DIMENSIONMASTER_Q_SWITCHES_49 = {{
    {"use_meshtype", false},
    {"use_subuv_tex", false},
    {"use_nonalpha_tex", false},
    {"use_emissive_vertcolor", false},
    {"use_nonuvnoise", false},
}};

constexpr std::array<std::string_view,3> DIMENSIONMASTER_Q_TEXTURES_50 = {{"normal_tex","refle_tex","native_texture_2"}};
constexpr std::array<DIMENSIONMASTER_Q_PARAMETER_DESC,17> DIMENSIONMASTER_Q_PARAMETERS_50 = {{
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
constexpr std::array<DIMENSIONMASTER_Q_SWITCH_DESC,2> DIMENSIONMASTER_Q_SWITCHES_50 = {{
    {"use_depth", false},
    {"use_dissolve", false},
}};

constexpr std::array<std::string_view,7> DIMENSIONMASTER_Q_TEXTURES_51 = {{"cracknormal_tex","native_texture_1","in_hole_texture","aura_texture","native_texture_4","native_texture_5","native_texture_6"}};
constexpr std::array<DIMENSIONMASTER_Q_PARAMETER_DESC,32> DIMENSIONMASTER_Q_PARAMETERS_51 = {{
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
constexpr std::array<DIMENSIONMASTER_Q_SWITCH_DESC,6> DIMENSIONMASTER_Q_SWITCHES_51 = {{
    {"use_dynamic_vcoord", true},
    {"uvnoise_v_pannertime", true},
    {"main_v_pannertime", false},
    {"main_u_pannertime", false},
    {"use_mesh", false},
    {"use_edgesmooth", false},
}};

// These Q51 overlays are project-authored tuning, not recovered UE3 programs.
// native-material-public-constant: DIMENSIONMASTER_R_GLASSHOLE_CORE_PROFILE_ID
// native-material-public-constant: DIMENSIONMASTER_R_GLASSHOLE_CLEAN_CORE_PROFILE_ID
// native-material-public-constant: DIMENSIONMASTER_GLASSHOLE_PURPLE_RIM_PROFILE_ID
inline constexpr auto DIMENSIONMASTER_GLASSHOLE_PURPLE_RIM_PARAMETERS = []
{
    std::array<DIMENSIONMASTER_Q_PARAMETER_DESC,34> Parameters{};
    std::copy(DIMENSIONMASTER_Q_PARAMETERS_51.begin(),
        DIMENSIONMASTER_Q_PARAMETERS_51.end(), Parameters.begin());
    Parameters[32] = {"rimIntensity", 11u, 0u, false};
    Parameters[33] = {"rimOpacity", 11u, 1u, false};
    return Parameters;
}();
inline constexpr auto DIMENSIONMASTER_R_GLASSHOLE_CORE_PARAMETERS = []
{
    std::array<DIMENSIONMASTER_Q_PARAMETER_DESC,34> Parameters{};
    std::copy(DIMENSIONMASTER_Q_PARAMETERS_51.begin(),
        DIMENSIONMASTER_Q_PARAMETERS_51.end(), Parameters.begin());
    Parameters[32] = {"blackCoreWidth", 11u, 0u, false};
    Parameters[33] = {"blackCoreSoftness", 11u, 1u, false};
    return Parameters;
}();

constexpr std::array<DIMENSIONMASTER_Q_PROGRAM_DESC,12> DIMENSIONMASTER_Q_PROGRAM_STORAGE = {{
    {44u,"effect.ue3.q-blackline-aura-native.v1","fx_m_mi_j_00.fx_mi.fx_j_me_blacklineaura_01_07_tr","fx_m_mi_j_00.fx_m.fx_j_pa_blacklineaura_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.blacklineaura.01.tr.317194a1c8e2",true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_Q_TEXTURES_44,DIMENSIONMASTER_Q_PARAMETERS_44,DIMENSIONMASTER_Q_SWITCHES_44},
    {45u,"effect.ue3.q-basic-add-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_gl_01_2_ad","fx_mastermaterial.fx_mm.fx_mm_basic_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_Q_TEXTURES_45,DIMENSIONMASTER_Q_PARAMETERS_45,DIMENSIONMASTER_Q_SWITCHES_45},
    {46u,"effect.ue3.q-missiletrail-sprite-native.v1","fx_m_mi_03.fx_mi.fx_m_pa_missiletrail_01_8_tr","fx_m_mi_03.fx_m.fx_m_pa_missiletrail_01_tr","ue3.material.fx.m.mi.03.fx.m.fx.m.pa.missiletrail.01.tr.9641f8d91e6a",false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_Q_TEXTURES_46,DIMENSIONMASTER_Q_PARAMETERS_46,DIMENSIONMASTER_Q_SWITCHES_46},
    {47u,"effect.ue3.q-center-glow-native.v1","bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad","bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad","ue3.material.bfx.m.mi.00.bfx.m.bfx.i.pa.glow.01.ad.0857e02620a0",false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_Q_TEXTURES_47,DIMENSIONMASTER_Q_PARAMETERS_47,DIMENSIONMASTER_Q_SWITCHES_47},
    {48u,"effect.ue3.q-rot-turbulence-native.v1","fx_m_mi_j_00.fx_mi.fx_j_pa_rotturbulance_01_06_ad","fx_m_mi_j_00.fx_m.fx_j_pa_rotturbulance_01_ad","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.rotturbulance.01.ad.796538ecc2e7",false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_Q_TEXTURES_48,DIMENSIONMASTER_Q_PARAMETERS_48,DIMENSIONMASTER_Q_SWITCHES_48},
    {49u,"effect.ue3.q-simple-add-native.v1","fx_m_mi_00.fx_mi.fx_a_pa_ht_01_1_ad","fx_mastermaterial.fx_mm.fx_mm_simple_01_ad","ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",false,EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_Q_TEXTURES_49,DIMENSIONMASTER_Q_PARAMETERS_49,DIMENSIONMASTER_Q_SWITCHES_49},
    {50u,"effect.ue3.q-local-crack-native.v1","fx_m_mi_j_00.fx_mi.fx_j_me_localcrack_01_04_tr","fx_m_mi_j_00.fx_m.fx_j_me_localcrack_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.me.localcrack.01.tr.a0e83a46fef9",true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_Q_TEXTURES_50,DIMENSIONMASTER_Q_PARAMETERS_50,DIMENSIONMASTER_Q_SWITCHES_50},
    {51u,"effect.ue3.q-glass-hole-native.v1","fx_m_mi_j_00.fx_mi.fx_j_pa_glasshole_02_01_tr","fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2",false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_Q_TEXTURES_51,DIMENSIONMASTER_Q_PARAMETERS_51,DIMENSIONMASTER_Q_SWITCHES_51},
    // Exact E/A material identities share the selected native program and input layout.
    {44u,"effect.ue3.a-blacklineaura-01-06-native.v1","fx_m_mi_j_00.fx_mi.fx_j_me_blacklineaura_01_06_tr","fx_m_mi_j_00.fx_m.fx_j_pa_blacklineaura_01_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.blacklineaura.01.tr.317194a1c8e2",true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_Q_TEXTURES_44,DIMENSIONMASTER_Q_PARAMETERS_44,DIMENSIONMASTER_Q_SWITCHES_44},
    {51u,DIMENSIONMASTER_GLASSHOLE_PURPLE_RIM_PROFILE_ID,"fx_m_mi_j_00.fx_mi.fx_j_pa_glasshole_02_01_tr","fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2",false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_Q_TEXTURES_51,DIMENSIONMASTER_GLASSHOLE_PURPLE_RIM_PARAMETERS,DIMENSIONMASTER_Q_SWITCHES_51},
    // Only these authored identities populate row11; native Q51 leaves it zero.
    {51u,DIMENSIONMASTER_R_GLASSHOLE_CORE_PROFILE_ID,"fx_m_mi_j_00.fx_mi.fx_j_pa_glasshole_02_01_tr","fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2",false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_Q_TEXTURES_51,DIMENSIONMASTER_R_GLASSHOLE_CORE_PARAMETERS,DIMENSIONMASTER_Q_SWITCHES_51},
    {51u,DIMENSIONMASTER_R_GLASSHOLE_CLEAN_CORE_PROFILE_ID,"fx_m_mi_j_00.fx_mi.fx_j_pa_glasshole_02_01_tr","fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr","ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2",false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_Q_TEXTURES_51,DIMENSIONMASTER_R_GLASSHOLE_CORE_PARAMETERS,DIMENSIONMASTER_Q_SWITCHES_51},
}};
