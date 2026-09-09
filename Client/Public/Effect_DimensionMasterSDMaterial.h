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

inline constexpr std::array<DIMENSIONMASTER_SD_PROGRAM_DESC,5> DIMENSIONMASTER_SD_PROGRAMS = {{
    {320u,"effect.ue3.sd-320-native.v1","fx_m_mi_01.fx_mi.fx_e_pa_gl_10_1_tr","fx_mastermaterial.fx_mm.fx_mm_maskcontrol_01_tr","ue3.material.fx.mastermaterial.fx.mm.fx.mm.maskcontrol.01.tr.30a3b56d23fe",false,"sprite",false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_320,DIMENSIONMASTER_SD_PARAMETERS_320,DIMENSIONMASTER_SD_SWITCHES_320},
    {321u,"effect.ue3.sd-321-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_spritewave_24_02_tr","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",false,"sprite",false,false,false,EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_321,DIMENSIONMASTER_SD_PARAMETERS_321,DIMENSIONMASTER_SD_SWITCHES_321},
    {322u,"effect.ue3.sd-322-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_spritewave_30_01_tr","fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr","ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",false,"sprite",false,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_322,DIMENSIONMASTER_SD_PARAMETERS_322,DIMENSIONMASTER_SD_SWITCHES_322},
    {323u,"effect.ue3.sd-323-native.v1","fx_m_mi_r_00.fx_mi.fx_r_pa_twirl_03_09_ad","fx_m_mi_03.fx_m.fx_d_pa_twirl_05_ad","ue3.material.fx.m.mi.03.fx.m.fx.d.pa.twirl.05.ad.2b8e151c3da1",false,"sprite",false,true,false,EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_323,DIMENSIONMASTER_SD_PARAMETERS_323,DIMENSIONMASTER_SD_SWITCHES_323},
    {324u,"effect.ue3.sd-324-native.v1","fx_m_mi_00.fx_mi.fx_d_me_ice_01_02_tr","fx_m_mi_00.fx_m.fx_d_me_ice_01_tr","ue3.material.fx.m.mi.00.fx.m.fx.d.me.ice.01.tr.b5fca778c01f",true,"mesh",false,false,true,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,DIMENSIONMASTER_SD_TEXTURES_324,DIMENSIONMASTER_SD_PARAMETERS_324,DIMENSIONMASTER_SD_SWITCHES_324},
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
