#pragma once

#include "DataJson.h"
#include "BinaryAsset/ModelAssetData.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <set>
#include <string>

// Generated selected-program packing. The source parameters keep their native
// names; this is shared by every avatar using the same static program, not by class.
namespace Client::SourceCharacterMaterial
{
/* The catalog states these as JSON once, but the character-creation screen moves some of them
   while the character is on screen -- skin colour, gloss, freckles, the make-up colours and the
   decal placement are all named parameters here. Keeping the read-and-validate step separate
   from the packing lets the screen change one value and re-pack, instead of the packing being
   reachable only from a document. */
using PARAMETER_VALUES = std::map<std::string, std::array<float, 4>>;

inline bool Read(const DATA_JSON_VALUE& parameters, PARAMETER_VALUES& result)
{
    if (!parameters.Is_Object()) return false;
    PARAMETER_VALUES staged;
    for (const auto& [name, value] : parameters.Get_Object()) {
        std::array<float, 4> out{};
        if (value.Is_Number()) {
            const double scalar = value.Get_Number();
            if (!std::isfinite(scalar) || std::abs(scalar) > 1000000.0) return false;
            out.fill(static_cast<float>(scalar));
        } else if (value.Is_Array() && value.Get_Array().size() == 4u) {
            for (size_t i=0;i<4u;++i) {
                const auto& component=value.Get_Array()[i];
                if (!component.Is_Number() || !std::isfinite(component.Get_Number()) ||
                    std::abs(component.Get_Number()) > 1000000.0) return false;
                out[i]=static_cast<float>(component.Get_Number());
            }
        } else return false;
        staged.emplace(name, out);
    }
    result = std::move(staged);
    return true;
}

inline bool Configure(const std::string& family, const PARAMETER_VALUES& parameters,
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& result)
{
    using Value = std::array<float, 4>;
    bool valid = true;
    std::set<std::string> consumed;
    const auto parameter = [&](const char* name) -> Value {
        consumed.insert(name);
        const auto found = parameters.find(name);
        if (found == parameters.end()) { valid = false; return Value{}; }
        return found->second;
    };
    const auto add=[](Value a,Value b) { for(size_t i=0;i<4u;++i)a[i]+=b[i];return a; };
    const auto subtract=[](Value a,Value b) { for(size_t i=0;i<4u;++i)a[i]-=b[i];return a; };
    const auto multiply=[](Value a,Value b) { for(size_t i=0;i<4u;++i)a[i]*=b[i];return a; };
    const auto append=[](Value a,Value b,size_t count) { Value out{};for(size_t i=0;i<4u;++i)out[i]=i<count?a[i]:b[i-count];return out; };
    const auto wave=[](Value a,bool cosine) { for(auto& v:a)v=cosine?std::cos(v):std::sin(v);return a; };
    const auto bounded=[](Value a,Value lo,Value hi) { for(size_t i=0;i<4u;++i)a[i]=(std::min)((std::max)(a[i],lo[i]),hi[i]);return a; };
    const auto vector=[](Value a) { return float4_t(a[0],a[1],a[2],a[3]); };
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS staged{};
    if (family == "source.character.classic-skin.v1")
    {
        staged.program = 1u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("var_base_skincolor_ui"));
        staged.baseConstants[8] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[9] = vector(parameter("ibl_color_top"));
        staged.baseConstants[10] = vector(parameter("transcolor"));
        staged.baseConstants[11] = vector(parameter("buffcolor"));
        staged.baseConstants[12] = vector(parameter("hit_color"));
        staged.baseConstants[13] = vector(append(Value{},Value{},1u));
        staged.baseConstants[14] = vector(parameter("state_noise"));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.baseConstants[18] = float4_t(parameter("skin_roughness_power")[0],parameter("roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[19] = float4_t(parameter("ibl_normal_smooth")[0],parameter("ibl_skinlodscale")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0]);
        staged.baseConstants[20] = float4_t(parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[21] = float4_t(parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.baseConstants[22] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightTextureMask = 127u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(parameter("var_base_skincolor_ui"));
        staged.lightConstants[7] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[8] = vector(parameter("ibl_color_top"));
        staged.lightConstants[9] = vector(append(Value{},Value{},1u));
        staged.lightConstants[10] = vector(parameter("state_noise"));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[13] = vector(parameter("half_lambert_skin"));
        staged.lightConstants[14] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.lightConstants[15] = float4_t(parameter("skin_roughness_power")[0],parameter("roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[16] = float4_t(parameter("ibl_normal_smooth")[0],parameter("ibl_skinlodscale")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0]);
        staged.lightConstants[17] = float4_t(parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[18] = float4_t(parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.lightConstants[19] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[20] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightConstants[21] = float4_t(parameter("orennayar_brightness")[0],parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("var_base_skinspecularpower_ui")[0]);
        staged.lightConstants[22] = float4_t(parameter("skin_specular_power")[0],parameter("skin_specular_power_min")[0],parameter("var_base_skinspecularintensity_ui")[0],parameter("skin_specular_intensity")[0]);
        staged.lightConstants[23] = float4_t(parameter("skin_specular_intensity_min")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
    }
    else if (family == "source.character.classic-variation.v1")
    {
        staged.program = 2u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[10] = vector(parameter("ibl_color_top"));
        staged.baseConstants[11] = vector(parameter("transcolor"));
        staged.baseConstants[12] = vector(parameter("buffcolor"));
        staged.baseConstants[13] = vector(parameter("hit_color"));
        staged.baseConstants[14] = vector(append(Value{},Value{},1u));
        staged.baseConstants[15] = vector(parameter("state_noise"));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[17] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[18] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[19] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[20] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[21] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[22] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightTextureMask = 255u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[8] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[9] = vector(parameter("ibl_color_top"));
        staged.lightConstants[10] = vector(append(Value{},Value{},1u));
        staged.lightConstants[11] = vector(parameter("state_noise"));
        staged.lightConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[14] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[15] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[16] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[17] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[18] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightConstants[19] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[20] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
    }
    else if (family == "source.character.realpbr-avatar-v2.v1")
    {
        staged.program = 3u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_d"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("state"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("transcolor"));
        staged.baseConstants[12] = vector(parameter("buffcolor"));
        staged.baseConstants[13] = vector(parameter("hit_color"));
        staged.baseConstants[14] = vector(parameter("occlusion_color"));
        staged.baseConstants[15] = vector(append(Value{},Value{},1u));
        staged.baseConstants[16] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[17] = vector(parameter("ssstintcolor"));
        staged.baseConstants[18] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("dyeingthreshold_low")[0],parameter("dyeingthreshold_high")[0]);
        staged.baseConstants[19] = float4_t(parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[20] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0]);
        staged.baseConstants[21] = float4_t(Value{}[0],parameter("auto_pbr_oc_max")[0],parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.baseConstants[22] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.baseConstants[23] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[24] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_d")[0],multiply(parameter("roughness_power_d"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[25] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 63u;
        staged.lightConstants[2] = vector(parameter("selectioncolor"));
        staged.lightConstants[3] = vector(parameter("basecolor_color"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_d"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[7] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[8] = vector(parameter("state"));
        staged.lightConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[11] = vector(parameter("occlusion_color"));
        staged.lightConstants[12] = vector(append(Value{},Value{},1u));
        staged.lightConstants[13] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[14] = vector(parameter("ssstintcolor"));
        staged.lightConstants[15] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("dyeingthreshold_low")[0],parameter("dyeingthreshold_high")[0]);
        staged.lightConstants[16] = float4_t(parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[17] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0]);
        staged.lightConstants[18] = float4_t(Value{}[0],parameter("auto_pbr_oc_max")[0],parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.lightConstants[19] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.lightConstants[20] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[21] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_d")[0],multiply(parameter("roughness_power_d"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[22] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
    }
    else if (family == "source.character.classic-head.v1")
    {
        staged.program = 4u;
        staged.baseTextureMask = 2047u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("var_base_skincolor_ui"));
        staged.baseConstants[5] = vector(parameter("var_makeup_lipcolor_ui"));
        staged.baseConstants[6] = vector(parameter("makeup_lip_region"));
        staged.baseConstants[7] = vector(parameter("var_makeup_eyelinecolor_ui"));
        staged.baseConstants[8] = vector(parameter("makeup_eye_region"));
        staged.baseConstants[9] = vector(parameter("var_makeup_eyeshadowcolor_ui"));
        staged.baseConstants[10] = vector(parameter("var_makeup_cheekcolor_ui"));
        staged.baseConstants[11] = vector(parameter("makeup_cheek_region"));
        staged.baseConstants[12] = vector(parameter("var_base_skinfrecklecolor_ui"));
        staged.baseConstants[13] = vector(parameter("var_eye_browcolor_ui"));
        staged.baseConstants[14] = vector(parameter("decal_region"));
        staged.baseConstants[15] = vector(append(multiply(add(parameter("var_headdeco_decalside_ui"),Value{0.f,0.f,0.f,0.f}),Value{0.300000012f,0.f,0.f,0.f}),multiply(add(parameter("var_headdeco_decalupdown_ui"),Value{-0.239999995f,0.f,0.f,0.f}),Value{0.430000007f,0.f,0.f,0.f}),1u));
        staged.baseConstants[16] = vector(append(multiply(multiply(add(parameter("var_headdeco_decalside_ui"),Value{0.f,0.f,0.f,0.f}),Value{0.300000012f,0.f,0.f,0.f}),Value{-1.f,0.f,0.f,0.f}),multiply(add(parameter("var_headdeco_decalupdown_ui"),Value{-0.239999995f,0.f,0.f,0.f}),Value{0.430000007f,0.f,0.f,0.f}),1u));
        staged.baseConstants[17] = vector(parameter("var_headdeco_decalcolor_ui"));
        staged.baseConstants[18] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[19] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[20] = vector(parameter("transcolor"));
        staged.baseConstants[21] = vector(parameter("buffcolor"));
        staged.baseConstants[22] = vector(parameter("hit_color"));
        staged.baseConstants[23] = vector(append(Value{},Value{},1u));
        staged.baseConstants[24] = vector(parameter("state_noise"));
        staged.baseConstants[25] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[26] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[27] = float4_t(parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0],parameter("var_headbase_overlaynormalintensity_ui")[0]);
        staged.baseConstants[28] = float4_t(parameter("skin_roughness_power")[0],parameter("var_makeup_cheekside_ui")[0],parameter("var_headdeco_fixedcheek_bool_private_ui")[0],multiply(Value{0.f,0.f,0.f,0.f},parameter("var_headdeco_fixedcheek_bool_private_ui"))[0]);
        staged.baseConstants[29] = float4_t(multiply(parameter("var_makeup_cheekside_ui"),Value{0.300000012f,0.f,0.f,0.f})[0],parameter("var_makeup_cheekupdown_ui")[0],multiply(parameter("var_makeup_cheekupdown_ui"),Value{0.300000012f,0.f,0.f,0.f})[0],parameter("var_makeup_cheekscale_ui")[0]);
        staged.baseConstants[30] = float4_t(multiply(parameter("var_makeup_cheekscale_ui"),Value{0.300000012f,0.f,0.f,0.f})[0],parameter("var_headdeco_decalside_ui")[0],add(parameter("var_headdeco_decalside_ui"),Value{0.f,0.f,0.f,0.f})[0],parameter("var_headdeco_decalupdown_ui")[0]);
        staged.baseConstants[31] = float4_t(add(parameter("var_headdeco_decalupdown_ui"),Value{-0.239999995f,0.f,0.f,0.f})[0],multiply(add(parameter("var_headdeco_decalupdown_ui"),Value{-0.239999995f,0.f,0.f,0.f}),Value{0.430000007f,0.f,0.f,0.f})[0],multiply(add(parameter("var_headdeco_decalside_ui"),Value{0.f,0.f,0.f,0.f}),Value{0.300000012f,0.f,0.f,0.f})[0],parameter("var_headdeco_fixeddecal_bool_private_ui")[0]);
        staged.baseConstants[32] = float4_t(parameter("var_headdeco_decalrotate_ui")[0],parameter("var_headdeco_decalscale_ui")[0],add(parameter("var_headdeco_decalscale_ui"),Value{-0.280000001f,0.f,0.f,0.f})[0],multiply(add(parameter("var_headdeco_decalscale_ui"),Value{-0.280000001f,0.f,0.f,0.f}),Value{8.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[33] = float4_t(add(multiply(add(parameter("var_headdeco_decalscale_ui"),Value{-0.280000001f,0.f,0.f,0.f}),Value{8.f,0.f,0.f,0.f}),Value{-8.f,0.f,0.f,0.f})[0],multiply(multiply(add(parameter("var_headdeco_decalside_ui"),Value{0.f,0.f,0.f,0.f}),Value{0.300000012f,0.f,0.f,0.f}),Value{-1.f,0.f,0.f,0.f})[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_headdeco_decalrotate_ui"))[0],parameter("var_headdeco_decalinverse_bool_ui")[0]);
        staged.baseConstants[34] = float4_t(parameter("var_headdeco_usedecalcolor_bool_private_ui")[0],parameter("var_headdeco_decalmirror_bool_ui")[0],parameter("ibl_normal_smooth")[0],parameter("ibl_reflect_lodbias")[0]);
        staged.baseConstants[35] = float4_t(parameter("ibl_skinlodscale")[0],multiply(parameter("ibl_skinlodscale"),parameter("ibl_reflect_lodbias"))[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[36] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.baseConstants[37] = float4_t(parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[38] = float4_t(Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0]);
        staged.lightTextureMask = 4095u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("var_base_skincolor_ui"));
        staged.lightConstants[4] = vector(parameter("var_makeup_lipcolor_ui"));
        staged.lightConstants[5] = vector(parameter("makeup_lip_region"));
        staged.lightConstants[6] = vector(parameter("var_makeup_eyelinecolor_ui"));
        staged.lightConstants[7] = vector(parameter("makeup_eye_region"));
        staged.lightConstants[8] = vector(parameter("var_makeup_eyeshadowcolor_ui"));
        staged.lightConstants[9] = vector(parameter("var_makeup_cheekcolor_ui"));
        staged.lightConstants[10] = vector(parameter("makeup_cheek_region"));
        staged.lightConstants[11] = vector(parameter("var_base_skinfrecklecolor_ui"));
        staged.lightConstants[12] = vector(parameter("var_eye_browcolor_ui"));
        staged.lightConstants[13] = vector(parameter("decal_region"));
        staged.lightConstants[14] = vector(append(multiply(add(parameter("var_headdeco_decalside_ui"),Value{0.f,0.f,0.f,0.f}),Value{0.300000012f,0.f,0.f,0.f}),multiply(add(parameter("var_headdeco_decalupdown_ui"),Value{-0.239999995f,0.f,0.f,0.f}),Value{0.430000007f,0.f,0.f,0.f}),1u));
        staged.lightConstants[15] = vector(append(multiply(multiply(add(parameter("var_headdeco_decalside_ui"),Value{0.f,0.f,0.f,0.f}),Value{0.300000012f,0.f,0.f,0.f}),Value{-1.f,0.f,0.f,0.f}),multiply(add(parameter("var_headdeco_decalupdown_ui"),Value{-0.239999995f,0.f,0.f,0.f}),Value{0.430000007f,0.f,0.f,0.f}),1u));
        staged.lightConstants[16] = vector(parameter("var_headdeco_decalcolor_ui"));
        staged.lightConstants[17] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[18] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[19] = vector(append(Value{},Value{},1u));
        staged.lightConstants[20] = vector(parameter("state_noise"));
        staged.lightConstants[21] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[22] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[23] = vector(parameter("half_lambert_skin"));
        staged.lightConstants[24] = float4_t(parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0],parameter("var_headbase_overlaynormalintensity_ui")[0]);
        staged.lightConstants[25] = float4_t(parameter("skin_roughness_power")[0],parameter("var_makeup_cheekside_ui")[0],parameter("var_headdeco_fixedcheek_bool_private_ui")[0],multiply(Value{0.f,0.f,0.f,0.f},parameter("var_headdeco_fixedcheek_bool_private_ui"))[0]);
        staged.lightConstants[26] = float4_t(multiply(parameter("var_makeup_cheekside_ui"),Value{0.300000012f,0.f,0.f,0.f})[0],parameter("var_makeup_cheekupdown_ui")[0],multiply(parameter("var_makeup_cheekupdown_ui"),Value{0.300000012f,0.f,0.f,0.f})[0],parameter("var_makeup_cheekscale_ui")[0]);
        staged.lightConstants[27] = float4_t(multiply(parameter("var_makeup_cheekscale_ui"),Value{0.300000012f,0.f,0.f,0.f})[0],parameter("var_headdeco_decalside_ui")[0],add(parameter("var_headdeco_decalside_ui"),Value{0.f,0.f,0.f,0.f})[0],parameter("var_headdeco_decalupdown_ui")[0]);
        staged.lightConstants[28] = float4_t(add(parameter("var_headdeco_decalupdown_ui"),Value{-0.239999995f,0.f,0.f,0.f})[0],multiply(add(parameter("var_headdeco_decalupdown_ui"),Value{-0.239999995f,0.f,0.f,0.f}),Value{0.430000007f,0.f,0.f,0.f})[0],multiply(add(parameter("var_headdeco_decalside_ui"),Value{0.f,0.f,0.f,0.f}),Value{0.300000012f,0.f,0.f,0.f})[0],parameter("var_headdeco_fixeddecal_bool_private_ui")[0]);
        staged.lightConstants[29] = float4_t(parameter("var_headdeco_decalrotate_ui")[0],parameter("var_headdeco_decalscale_ui")[0],add(parameter("var_headdeco_decalscale_ui"),Value{-0.280000001f,0.f,0.f,0.f})[0],multiply(add(parameter("var_headdeco_decalscale_ui"),Value{-0.280000001f,0.f,0.f,0.f}),Value{8.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[30] = float4_t(add(multiply(add(parameter("var_headdeco_decalscale_ui"),Value{-0.280000001f,0.f,0.f,0.f}),Value{8.f,0.f,0.f,0.f}),Value{-8.f,0.f,0.f,0.f})[0],multiply(multiply(add(parameter("var_headdeco_decalside_ui"),Value{0.f,0.f,0.f,0.f}),Value{0.300000012f,0.f,0.f,0.f}),Value{-1.f,0.f,0.f,0.f})[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_headdeco_decalrotate_ui"))[0],parameter("var_headdeco_decalinverse_bool_ui")[0]);
        staged.lightConstants[31] = float4_t(parameter("var_headdeco_usedecalcolor_bool_private_ui")[0],parameter("var_headdeco_decalmirror_bool_ui")[0],parameter("ibl_normal_smooth")[0],parameter("ibl_reflect_lodbias")[0]);
        staged.lightConstants[32] = float4_t(parameter("ibl_skinlodscale")[0],multiply(parameter("ibl_skinlodscale"),parameter("ibl_reflect_lodbias"))[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[33] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.lightConstants[34] = float4_t(parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[35] = float4_t(Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0]);
        staged.lightConstants[36] = float4_t(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("specular_power_limit")[0]);
        staged.lightConstants[37] = float4_t(parameter("var_base_skinspecularpower_ui")[0],parameter("skin_specular_power")[0],parameter("skin_specular_power_min")[0],parameter("var_makeup_lipspecular_ui")[0]);
        staged.lightConstants[38] = float4_t(parameter("var_base_skinspecularintensity_ui")[0],parameter("skin_specular_intensity")[0],parameter("skin_specular_intensity_min")[0],parameter("beckmannspecular_constant_max")[0]);
        staged.lightConstants[39] = float4_t(parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f,0.f);
    }
    else if (family == "source.character.eye.v1")
    {
        staged.program = 5u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[0] = vector(parameter("selectioncolor"));
        staged.baseConstants[1] = vector(parameter("state"));
        staged.baseConstants[2] = vector(append(parameter("tdspecular_size_x"),parameter("tdspecular_size_y"),1u));
        staged.baseConstants[3] = vector(append(parameter("tdspecular_offset_x"),parameter("tdspecular_offset_y"),1u));
        staged.baseConstants[4] = vector(parameter("var_eye_basecolorleft_ui"));
        staged.baseConstants[5] = vector(parameter("var_eye_iriscolorleft_ui"));
        staged.baseConstants[6] = vector(parameter("var_eye_basecolor_ui"));
        staged.baseConstants[7] = vector(parameter("var_eye_iriscolor_ui"));
        staged.baseConstants[8] = vector(append(Value{},Value{},1u));
        staged.baseConstants[9] = float4_t(parameter("iris_size_center")[0],parameter("var_eye_irissize_ui")[0],subtract(parameter("var_eye_irissize_ui"),parameter("iris_size_center"))[0],parameter("specular_offset")[0]);
        staged.baseConstants[10] = float4_t(parameter("specular_power")[0],parameter("specular_intensity")[0],parameter("tdspecular_offset_y")[0],parameter("tdspecular_offset_x")[0]);
        staged.baseConstants[11] = float4_t(parameter("tdspecular_uv_offset")[0],parameter("tdspecular_size_y")[0],parameter("tdspecular_size_x")[0],parameter("tdspecular_intensity")[0]);
        staged.baseConstants[12] = float4_t(parameter("var_eye_useordeyecolor_bool_ui")[0],Value{}[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightTextureMask = 63u;
        staged.lightConstants[0] = vector(parameter("state"));
        staged.lightConstants[1] = vector(append(parameter("tdspecular_size_x"),parameter("tdspecular_size_y"),1u));
        staged.lightConstants[2] = vector(append(parameter("tdspecular_offset_x"),parameter("tdspecular_offset_y"),1u));
        staged.lightConstants[3] = vector(parameter("var_eye_basecolorleft_ui"));
        staged.lightConstants[4] = vector(parameter("var_eye_iriscolorleft_ui"));
        staged.lightConstants[5] = vector(parameter("var_eye_basecolor_ui"));
        staged.lightConstants[6] = vector(parameter("var_eye_iriscolor_ui"));
        staged.lightConstants[7] = vector(append(Value{},Value{},1u));
        staged.lightConstants[8] = float4_t(parameter("iris_size_center")[0],parameter("var_eye_irissize_ui")[0],subtract(parameter("var_eye_irissize_ui"),parameter("iris_size_center"))[0],parameter("specular_offset")[0]);
        staged.lightConstants[9] = float4_t(parameter("specular_power")[0],parameter("specular_intensity")[0],parameter("tdspecular_offset_y")[0],parameter("tdspecular_offset_x")[0]);
        staged.lightConstants[10] = float4_t(parameter("tdspecular_uv_offset")[0],parameter("tdspecular_size_y")[0],parameter("tdspecular_size_x")[0],parameter("tdspecular_intensity")[0]);
        staged.lightConstants[11] = float4_t(parameter("var_eye_useordeyecolor_bool_ui")[0],Value{}[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
    }
    else if (family == "source.character.eyelash.v1")
    {
        staged.program = 6u;
        staged.baseTextureMask = 1u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("diffusecolor"));
        staged.baseConstants[3] = float4_t(parameter("opacity")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 1u;
        staged.lightConstants[1] = vector(parameter("selectioncolor"));
        staged.lightConstants[2] = vector(parameter("diffusecolor"));
        staged.lightConstants[3] = float4_t(parameter("opacity")[0],0.f,0.f,0.f);
    }
    else if (family == "source.character.hair.v1")
    {
        staged.program = 7u;
        staged.baseTextureMask = 7u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("var_base_haircolor_base_ui"));
        staged.baseConstants[4] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.baseConstants[8] = vector(parameter("hit_color"));
        staged.baseConstants[9] = vector(parameter("buffcolor"));
        staged.baseConstants[10] = vector(parameter("transcolor"));
        staged.baseConstants[11] = vector(parameter("state"));
        staged.baseConstants[12] = vector(append(Value{},Value{},1u));
        staged.baseConstants[13] = vector(parameter("state_noise"));
        staged.baseConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[16] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.baseConstants[17] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[18] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.baseConstants[19] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[20] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0]);
        staged.baseConstants[21] = float4_t(parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[22] = float4_t(parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0]);
        staged.lightTextureMask = 7u;
        staged.lightConstants[2] = vector(parameter("var_base_haircolor_base_ui"));
        staged.lightConstants[3] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.lightConstants[7] = vector(parameter("rimlight_color"));
        staged.lightConstants[8] = vector(parameter("state"));
        staged.lightConstants[9] = vector(append(Value{},Value{},1u));
        staged.lightConstants[10] = vector(parameter("state_noise"));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[13] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.lightConstants[14] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[15] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.lightConstants[16] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[17] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0]);
        staged.lightConstants[18] = float4_t(parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0]);
        staged.lightConstants[19] = float4_t(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("customshade_shadowdiffuselighting")[0]);
        staged.lightConstants[20] = float4_t(parameter("specular_power")[0],parameter("specular_intensity")[0],0.f,0.f);
    }
    else if (family == "source.character.realpbr-weapon.v1")
    {
        staged.program = 8u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("state"));
        staged.baseConstants[9] = vector(parameter("emissive_color"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[11] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[12] = vector(parameter("transcolor"));
        staged.baseConstants[13] = vector(parameter("buffcolor"));
        staged.baseConstants[14] = vector(parameter("hit_color"));
        staged.baseConstants[15] = vector(parameter("occlusion_color"));
        staged.baseConstants[16] = vector(append(Value{},Value{},1u));
        staged.baseConstants[17] = vector(parameter("ssstintcolor"));
        staged.baseConstants[18] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[19] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[20] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[21] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.baseConstants[22] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.baseConstants[23] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[24] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f);
        staged.lightTextureMask = 47u;
        staged.lightConstants[2] = vector(parameter("selectioncolor"));
        staged.lightConstants[3] = vector(parameter("basecolor_color"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("state"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[10] = vector(parameter("occlusion_color"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("ssstintcolor"));
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[14] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[15] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[16] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.lightConstants[17] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.lightConstants[18] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[19] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f);
    }
    else if (family == "source.character.realpbr-weapon-variation.v1")
    {
        staged.program = 9u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("state"));
        staged.baseConstants[9] = vector(parameter("emissive_color"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[11] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[12] = vector(parameter("transcolor"));
        staged.baseConstants[13] = vector(parameter("buffcolor"));
        staged.baseConstants[14] = vector(parameter("hit_color"));
        staged.baseConstants[15] = vector(parameter("occlusion_color"));
        staged.baseConstants[16] = vector(append(Value{},Value{},1u));
        staged.baseConstants[17] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[18] = vector(parameter("ssstintcolor"));
        staged.baseConstants[19] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[20] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[21] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[22] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.baseConstants[23] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.baseConstants[24] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[25] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f);
        staged.lightTextureMask = 111u;
        staged.lightConstants[2] = vector(parameter("selectioncolor"));
        staged.lightConstants[3] = vector(parameter("basecolor_color"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("state"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[10] = vector(parameter("occlusion_color"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[13] = vector(parameter("ssstintcolor"));
        staged.lightConstants[14] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[15] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[16] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[17] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.lightConstants[18] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.lightConstants[19] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[20] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f);
    }
    else if (family == "source.character.classic-parts-lower.v1")
    {
        staged.program = 10u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[10] = vector(parameter("ibl_color_top"));
        staged.baseConstants[11] = vector(parameter("transcolor"));
        staged.baseConstants[12] = vector(parameter("buffcolor"));
        staged.baseConstants[13] = vector(parameter("hit_color"));
        staged.baseConstants[14] = vector(append(Value{},Value{},1u));
        staged.baseConstants[15] = vector(parameter("state_noise"));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[17] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[18] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[19] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[20] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[21] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightTextureMask = 127u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[8] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[9] = vector(parameter("ibl_color_top"));
        staged.lightConstants[10] = vector(append(Value{},Value{},1u));
        staged.lightConstants[11] = vector(parameter("state_noise"));
        staged.lightConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[14] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[15] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[16] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[17] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightConstants[18] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[19] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
    }
    else if (family == "source.character.classic-parts-upper.v1")
    {
        staged.program = 11u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[9] = vector(parameter("var_base_skincolor_ui"));
        staged.baseConstants[10] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[11] = vector(parameter("ibl_color_top"));
        staged.baseConstants[12] = vector(parameter("transcolor"));
        staged.baseConstants[13] = vector(parameter("buffcolor"));
        staged.baseConstants[14] = vector(parameter("hit_color"));
        staged.baseConstants[15] = vector(append(Value{},Value{},1u));
        staged.baseConstants[16] = vector(parameter("state_noise"));
        staged.baseConstants[17] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[18] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[19] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.baseConstants[20] = float4_t(parameter("skin_roughness_power")[0],parameter("roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[21] = float4_t(parameter("ibl_normal_smooth")[0],parameter("ibl_skinlodscale")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0]);
        staged.baseConstants[22] = float4_t(parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[23] = float4_t(parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.baseConstants[24] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightTextureMask = 127u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[8] = vector(parameter("var_base_skincolor_ui"));
        staged.lightConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[10] = vector(parameter("ibl_color_top"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = vector(parameter("half_lambert_skin"));
        staged.lightConstants[16] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.lightConstants[17] = float4_t(parameter("skin_roughness_power")[0],parameter("roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[18] = float4_t(parameter("ibl_normal_smooth")[0],parameter("ibl_skinlodscale")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0]);
        staged.lightConstants[19] = float4_t(parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[20] = float4_t(parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.lightConstants[21] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[22] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightConstants[23] = float4_t(parameter("orennayar_brightness")[0],parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("var_base_skinspecularpower_ui")[0]);
        staged.lightConstants[24] = float4_t(parameter("skin_specular_power")[0],parameter("skin_specular_power_min")[0],parameter("var_base_skinspecularintensity_ui")[0],parameter("skin_specular_intensity")[0]);
        staged.lightConstants[25] = float4_t(parameter("skin_specular_intensity_min")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
    }
    else if (family == "source.character.classic-head-legacy.v1")
    {
        staged.program = 12u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("hit_color"));
        staged.baseConstants[4] = vector(parameter("buffcolor"));
        staged.baseConstants[5] = vector(parameter("var_base_skincolor_ui"));
        staged.baseConstants[6] = vector(parameter("decal_region"));
        staged.baseConstants[7] = vector(parameter("state"));
        staged.baseConstants[8] = vector(append(Value{},Value{},1u));
        staged.baseConstants[9] = vector(parameter("state_noise"));
        staged.baseConstants[10] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[12] = vector(parameter("globaldiffuseintensity"));
        staged.baseConstants[13] = float4_t(Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0]);
        staged.lightTextureMask = 31u;
        staged.lightConstants[2] = vector(parameter("globalrimlightintensity"));
        staged.lightConstants[3] = vector(parameter("var_base_skincolor_ui"));
        staged.lightConstants[4] = vector(parameter("decal_region"));
        staged.lightConstants[5] = vector(parameter("state"));
        staged.lightConstants[6] = vector(append(Value{},Value{},1u));
        staged.lightConstants[7] = vector(parameter("state_noise"));
        staged.lightConstants[8] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[9] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[10] = vector(parameter("skin_halflambert_color"));
        staged.lightConstants[11] = vector(parameter("skin_sss_color"));
        staged.lightConstants[12] = vector(parameter("globaldiffuseintensity"));
        staged.lightConstants[13] = vector(parameter("globalspecularintensity"));
        staged.lightConstants[14] = vector(parameter("specular_color"));
        staged.lightConstants[15] = float4_t(Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0]);
        staged.lightConstants[16] = float4_t(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("skin_orennayar_roughness")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("skin_orennayar_roughness"))[0],add(Value{1.f,0.f,0.f,0.f},parameter("skin_orennayar_roughness"))[0]);
        staged.lightConstants[17] = float4_t(parameter("var_makeup_lipspecular_ui")[0],parameter("var_makeup_lipthickness_ui")[0],bounded(parameter("var_makeup_lipthickness_ui"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],subtract(Value{1.f,0.f,0.f,0.f},bounded(parameter("var_makeup_lipthickness_ui"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0]);
        staged.lightConstants[18] = float4_t(multiply(Value{5.f,0.f,0.f,0.f},parameter("var_makeup_lipspecular_ui"))[0],parameter("var_base_skinspecularintensity_ui")[0],parameter("var_base_skinspecularpower_ui")[0],parameter("skinspecularpower_min")[0]);
        staged.lightConstants[19] = float4_t(parameter("specular_power")[0],parameter("specular_intensity")[0],parameter("subspecular_power")[0],parameter("subspecular_intensity")[0]);
    }
    else if (family == "source.character.classic-armor-emissive.v1")
    {
        staged.program = 13u;
        staged.baseTextureMask = 255u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[10] = vector(parameter("ibl_color_top"));
        staged.baseConstants[11] = vector(parameter("emissive_color"));
        staged.baseConstants[12] = vector(parameter("transcolor"));
        staged.baseConstants[13] = vector(parameter("buffcolor"));
        staged.baseConstants[14] = vector(parameter("hit_color"));
        staged.baseConstants[15] = vector(append(Value{},Value{},1u));
        staged.baseConstants[16] = vector(parameter("state_noise"));
        staged.baseConstants[17] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[18] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[19] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[20] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[21] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[22] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[23] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[24] = float4_t(Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0]);
        staged.lightTextureMask = 479u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[8] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[9] = vector(parameter("ibl_color_top"));
        staged.lightConstants[10] = vector(append(Value{},Value{},1u));
        staged.lightConstants[11] = vector(parameter("state_noise"));
        staged.lightConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[14] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[15] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[16] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[17] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[18] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[19] = float4_t(Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0]);
        staged.lightConstants[20] = float4_t(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0]);
        staged.lightConstants[21] = float4_t(parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0]);
        staged.lightConstants[22] = float4_t(parameter("pbr_specular_intensity")[0],0.f,0.f,0.f);
    }
    else if (family == "source.character.classic-weapon-emissive.v1")
    {
        staged.program = 14u;
        staged.baseTextureMask = 255u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("diffusecolor"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[11] = vector(parameter("ibl_color_top"));
        staged.baseConstants[12] = vector(parameter("emissive_color"));
        staged.baseConstants[13] = vector(parameter("transcolor"));
        staged.baseConstants[14] = vector(parameter("buffcolor"));
        staged.baseConstants[15] = vector(parameter("hit_color"));
        staged.baseConstants[16] = vector(append(Value{},Value{},1u));
        staged.baseConstants[17] = vector(parameter("state_noise"));
        staged.baseConstants[18] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[19] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[20] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[21] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[22] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[23] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[24] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[25] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightTextureMask = 479u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[8] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[9] = vector(parameter("ibl_color_top"));
        staged.lightConstants[10] = vector(append(Value{},Value{},1u));
        staged.lightConstants[11] = vector(parameter("state_noise"));
        staged.lightConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[14] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[15] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[16] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[17] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[18] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[19] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[20] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[21] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
    }
    else return false;
    if (!valid || consumed.size()!=parameters.size()) return false;
    result=staged;
    return true;
}

inline bool Configure(const std::string& family, const DATA_JSON_VALUE& parameters,
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& result)
{
    PARAMETER_VALUES values;
    return Read(parameters, values) && Configure(family, values, result);
}
}
