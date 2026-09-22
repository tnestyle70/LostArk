#pragma once

#include "DataJson.h"
#include "SourceMapForwardMaterialParameters.h"
#include "SourceMapWaterMaterialParameters.h"
#include "SourceMapTranslucentMaterialParameters.h"
#include "BinaryAsset/ModelAssetData.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <cstdint>

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
    // Named forward families share the numbered translucent prefix.
    if (family == "source.map.translucent-tiled.v1" ||
        family == "source.map.translucent-reflection.v1" ||
        family == "source.map.translucent-bump.v1")
        return SourceMapForwardMaterial::Configure(family, parameters, result);
    if (family.rfind("source.map.translucent-", 0u) == 0u)
        return SourceMapTranslucentMaterial::Configure(family, parameters, result);
    if (family.rfind("source.map.water-", 0u) == 0u)
        return SourceMapWaterMaterial::Configure(family, parameters, result);
    if (family.rfind("source.map.", 0u) == 0u)
        return SourceMapForwardMaterial::Configure(family, parameters, result);
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
    const auto divide=[](Value a,Value b) { for(size_t i=0;i<4u;++i)a[i]/=b[i];return a; };
    const auto append=[](Value a,Value b,size_t count) { Value out{};for(size_t i=0;i<4u;++i)out[i]=i<count?a[i]:b[i-count];return out; };
    const auto periodic=[](Value a) { for(auto& v:a)v-=std::floor(v);return a; };
    const auto wave=[](Value a,bool cosine) { for(auto& v:a)v=cosine?std::cos(v):std::sin(v);return a; };
    const auto bounded=[](Value a,Value lo,Value hi) { for(size_t i=0;i<4u;++i)a[i]=(std::min)((std::max)(a[i],lo[i]),hi[i]);return a; };
    const auto vector=[](Value a) { return float4_t(a[0],a[1],a[2],a[3]); };
    // Each generated family has a separate call frame: Debug expression
    // temporaries must not accumulate across all native programs.
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS staged{};
    if (staged.program == 0u && family == "source.character.classic-skin.v1")
    {
        [&]() {
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
        }();
    }
    if (staged.program == 0u && family == "source.character.classic-variation.v1")
    {
        [&]() {
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
        }();
    }
    if (staged.program == 0u && family == "source.character.realpbr-avatar-v2.v1")
    {
        [&]() {
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
        }();
    }
    if (staged.program == 0u && family == "source.character.classic-head.v1")
    {
        [&]() {
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
        }();
    }
    if (staged.program == 0u && family == "source.character.eye.v1")
    {
        [&]() {
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
        }();
    }
    if (staged.program == 0u && family == "source.character.eyelash.v1")
    {
        [&]() {
        staged.program = 6u;
        staged.baseTextureMask = 1u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("diffusecolor"));
        staged.baseConstants[3] = float4_t(parameter("opacity")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 1u;
        staged.lightConstants[1] = vector(parameter("selectioncolor"));
        staged.lightConstants[2] = vector(parameter("diffusecolor"));
        staged.lightConstants[3] = float4_t(parameter("opacity")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.hair.v1")
    {
        [&]() {
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
        }();
    }
    if (staged.program == 0u && family == "source.character.realpbr-weapon.v1")
    {
        [&]() {
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
        }();
    }
    if (staged.program == 0u && family == "source.character.realpbr-weapon-variation.v1")
    {
        [&]() {
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
        }();
    }
    if (staged.program == 0u && family == "source.character.classic-parts-lower.v1")
    {
        [&]() {
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
        }();
    }
    if (staged.program == 0u && family == "source.character.classic-parts-upper.v1")
    {
        [&]() {
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
        }();
    }
    if (staged.program == 0u && family == "source.character.classic-head-legacy.v1")
    {
        [&]() {
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
        }();
    }
    if (staged.program == 0u && family == "source.character.classic-armor-emissive.v1")
    {
        [&]() {
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
        }();
    }
    if (staged.program == 0u && family == "source.character.classic-weapon-emissive.v1")
    {
        [&]() {
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
        }();
    }
    if (staged.program == 0u && family == "source.character.classic-parts-sp.v1")
    {
        [&]() {
        staged.program = 15u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("var_base_skincolor_ui"));
        staged.baseConstants[11] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[12] = vector(parameter("ibl_color_top"));
        staged.baseConstants[13] = vector(parameter("transcolor"));
        staged.baseConstants[14] = vector(parameter("buffcolor"));
        staged.baseConstants[15] = vector(parameter("hit_color"));
        staged.baseConstants[16] = vector(append(Value{},Value{},1u));
        staged.baseConstants[17] = vector(parameter("state_noise"));
        staged.baseConstants[18] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[19] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[20] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[21] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.baseConstants[22] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[23] = float4_t(parameter("skin_roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[24] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.baseConstants[25] = float4_t(parameter("ibl_skinlodscale")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0]);
        staged.baseConstants[26] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.baseConstants[27] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightTextureMask = 255u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("var_base_skincolor_ui"));
        staged.lightConstants[10] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[11] = vector(parameter("ibl_color_top"));
        staged.lightConstants[12] = vector(append(Value{},Value{},1u));
        staged.lightConstants[13] = vector(parameter("state_noise"));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[16] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[17] = vector(parameter("half_lambert_skin"));
        staged.lightConstants[18] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.lightConstants[19] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[20] = float4_t(parameter("skin_roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[21] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.lightConstants[22] = float4_t(parameter("ibl_skinlodscale")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0]);
        staged.lightConstants[23] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.lightConstants[24] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightConstants[25] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[26] = float4_t(parameter("specular_power_limit")[0],parameter("var_base_skinspecularpower_ui")[0],parameter("skin_specular_power")[0],parameter("skin_specular_power_min")[0]);
        staged.lightConstants[27] = float4_t(parameter("var_base_skinspecularintensity_ui")[0],parameter("skin_specular_intensity")[0],parameter("skin_specular_intensity_min")[0],parameter("beckmannspecular_constant_max")[0]);
        staged.lightConstants[28] = float4_t(parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.classic-armor-sp.v1")
    {
        [&]() {
        staged.program = 16u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[11] = vector(parameter("ibl_color_top"));
        staged.baseConstants[12] = vector(parameter("transcolor"));
        staged.baseConstants[13] = vector(parameter("buffcolor"));
        staged.baseConstants[14] = vector(parameter("hit_color"));
        staged.baseConstants[15] = vector(append(Value{},Value{},1u));
        staged.baseConstants[16] = vector(parameter("state_noise"));
        staged.baseConstants[17] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[18] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[19] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[20] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[21] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.baseConstants[22] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[23] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[24] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.baseConstants[25] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightTextureMask = 255u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[10] = vector(parameter("ibl_color_top"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[16] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[17] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.lightConstants[18] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[19] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[20] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.lightConstants[21] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[22] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightConstants[23] = float4_t(parameter("orennayar_brightness")[0],parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0]);
        staged.lightConstants[24] = float4_t(parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.classic-brush.v1")
    {
        [&]() {
        staged.program = 17u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[11] = vector(parameter("ibl_color_top"));
        staged.baseConstants[12] = vector(parameter("transcolor"));
        staged.baseConstants[13] = vector(parameter("buffcolor"));
        staged.baseConstants[14] = vector(parameter("hit_color"));
        staged.baseConstants[15] = vector(append(Value{},Value{},1u));
        staged.baseConstants[16] = vector(parameter("state_noise"));
        staged.baseConstants[17] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[18] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[19] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[20] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("roughness_power_a")[0]);
        staged.baseConstants[21] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[22] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[23] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightTextureMask = 127u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[10] = vector(parameter("ibl_color_top"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[16] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("roughness_power_a")[0]);
        staged.lightConstants[17] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[18] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[19] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightConstants[20] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[21] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.hair-two-tone.v1")
    {
        [&]() {
        staged.program = 18u;
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
        }();
    }
    if (staged.program == 0u && family == "source.character.hair-masked.v1")
    {
        [&]() {
        staged.program = 19u;
        staged.baseTextureMask = 7u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("var_base_haircolor_base_ui"));
        staged.baseConstants[4] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.baseConstants[8] = vector(parameter("hit_color"));
        staged.baseConstants[9] = vector(parameter("state"));
        staged.baseConstants[10] = vector(append(Value{},Value{},1u));
        staged.baseConstants[11] = vector(parameter("state_noise"));
        staged.baseConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[14] = vector(parameter("buffcolor"));
        staged.baseConstants[15] = vector(parameter("transcolor"));
        staged.baseConstants[16] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.baseConstants[17] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[18] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.baseConstants[19] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[20] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0]);
        staged.baseConstants[21] = float4_t(parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0]);
        staged.baseConstants[22] = float4_t(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("ambientlight")[0]);
        staged.baseConstants[23] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("opacity_clamp")[0]);
        staged.lightTextureMask = 7u;
        staged.lightConstants[2] = vector(parameter("var_base_haircolor_base_ui"));
        staged.lightConstants[3] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.lightConstants[7] = vector(parameter("state"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("state_noise"));
        staged.lightConstants[10] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[12] = vector(parameter("rimlight_color"));
        staged.lightConstants[13] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.lightConstants[14] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[15] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.lightConstants[16] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[17] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0]);
        staged.lightConstants[18] = float4_t(parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0]);
        staged.lightConstants[19] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("opacity_clamp")[0]);
        staged.lightConstants[20] = float4_t(parameter("rimlight_power")[0],parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0],parameter("specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.hair-helmet.v1")
    {
        [&]() {
        staged.program = 20u;
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
        staged.baseConstants[16] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("var_base_hairspecularintensity_ui")[0]);
        staged.baseConstants[17] = float4_t(parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0]);
        staged.baseConstants[18] = float4_t(parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[19] = float4_t(parameter("rimlight_power")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
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
        staged.lightConstants[13] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("var_base_hairspecularintensity_ui")[0]);
        staged.lightConstants[14] = float4_t(parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0]);
        staged.lightConstants[15] = float4_t(parameter("rimlight_power")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[16] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0]);
        staged.lightConstants[17] = float4_t(parameter("specular_intensity")[0],0.f,0.f,0.f);
        }();
    }

    if (staged.program == 0u && family == "source.character.monster-pbr-masked.v1")
    {
        [&]() {
        staged.program = 21u;
        // Unowned row 63 carries the authored outline clock rate to its native time expressions.
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 127u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("diffusecolor"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[7] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[8] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[9] = vector(parameter("ibl_color_top"));
        staged.baseConstants[10] = vector(parameter("emissive_color"));
        staged.baseConstants[11] = vector(parameter("transcolor"));
        staged.baseConstants[12] = vector(parameter("buffcolor"));
        staged.baseConstants[13] = vector(parameter("constantoutline_color"));
        staged.baseConstants[14] = vector(parameter("hit_color"));
        staged.baseConstants[15] = vector(append(Value{},Value{},1u));
        staged.baseConstants[16] = vector(parameter("state_noise"));
        staged.baseConstants[17] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[18] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[19] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[20] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[21] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[22] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[23] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[24] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[25] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.lightTextureMask = 223u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[7] = vector(parameter("ibl_color_top"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("state_noise"));
        staged.lightConstants[10] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[12] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[13] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[14] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[15] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[16] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[17] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[18] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-7493dcdfd412.v1")
    {
        [&]() {
        staged.program = 22u;
        staged.baseTextureMask = 7u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("hit_color"));
        staged.baseConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[6] = vector(parameter("transcolor"));
        staged.baseConstants[7] = vector(parameter("buffcolor"));
        staged.baseConstants[8] = vector(parameter("constantoutline_color"));
        staged.baseConstants[9] = vector(parameter("diffusecolor"));
        staged.baseConstants[10] = vector(parameter("state"));
        staged.baseConstants[11] = vector(append(Value{},Value{},1u));
        staged.baseConstants[12] = vector(parameter("state_noise"));
        staged.baseConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[15] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[16] = float4_t(parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.baseConstants[17] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("constantoutline_power")[0],parameter("diffuse_brightness")[0]);
        staged.lightTextureMask = 7u;
        staged.lightConstants[2] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[3] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[4] = vector(parameter("diffusecolor"));
        staged.lightConstants[5] = vector(parameter("state"));
        staged.lightConstants[6] = vector(append(Value{},Value{},1u));
        staged.lightConstants[7] = vector(parameter("state_noise"));
        staged.lightConstants[8] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[9] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[10] = vector(parameter("rimlight_color"));
        staged.lightConstants[11] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[12] = float4_t(parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.lightConstants[13] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("constantoutline_power")[0],parameter("diffuse_brightness")[0]);
        staged.lightConstants[14] = float4_t(parameter("specular_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("specular_saturation"))[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.lightConstants[15] = float4_t(parameter("rimlight_intensity")[0],parameter("rimlight_power")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-d9d6c02905c3.v1")
    {
        [&]() {
        staged.program = 23u;
        staged.baseTextureMask = 255u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("diffusecolor"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[8] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[12] = vector(parameter("ibl_color_top"));
        staged.baseConstants[13] = vector(parameter("emissive_color"));
        staged.baseConstants[14] = vector(parameter("transcolor"));
        staged.baseConstants[15] = vector(parameter("buffcolor"));
        staged.baseConstants[16] = vector(parameter("constantoutline_color"));
        staged.baseConstants[17] = vector(parameter("hit_color"));
        staged.baseConstants[18] = vector(append(Value{},Value{},1u));
        staged.baseConstants[19] = vector(parameter("state_noise"));
        staged.baseConstants[20] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[21] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[22] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[23] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[24] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[25] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[26] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[27] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[28] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[29] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.lightTextureMask = 479u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[10] = vector(parameter("ibl_color_top"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[16] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[17] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[18] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[19] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[20] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[21] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[22] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-d621a47e69ad.v1")
    {
        [&]() {
        staged.program = 24u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[11] = vector(parameter("ibl_color_top"));
        staged.baseConstants[12] = vector(parameter("transcolor"));
        staged.baseConstants[13] = vector(parameter("buffcolor"));
        staged.baseConstants[14] = vector(parameter("constantoutline_color"));
        staged.baseConstants[15] = vector(parameter("hit_color"));
        staged.baseConstants[16] = vector(append(Value{},Value{},1u));
        staged.baseConstants[17] = vector(parameter("state_noise"));
        staged.baseConstants[18] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[19] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[20] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[21] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[22] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[23] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("constantoutline")[0]);
        staged.baseConstants[24] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[25] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.lightTextureMask = 127u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[10] = vector(parameter("ibl_color_top"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[16] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[17] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[18] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("constantoutline")[0]);
        staged.lightConstants[19] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.lightConstants[20] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[21] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-be5bc0ded311.v1")
    {
        [&]() {
        staged.program = 25u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[11] = vector(parameter("ibl_color_top"));
        staged.baseConstants[12] = vector(parameter("transcolor"));
        staged.baseConstants[13] = vector(parameter("buffcolor"));
        staged.baseConstants[14] = vector(parameter("constantoutline_color"));
        staged.baseConstants[15] = vector(parameter("hit_color"));
        staged.baseConstants[16] = vector(append(Value{},Value{},1u));
        staged.baseConstants[17] = vector(parameter("state_noise"));
        staged.baseConstants[18] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[19] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[20] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[21] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[22] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[23] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[24] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("constantoutline")[0]);
        staged.baseConstants[25] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[26] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.lightTextureMask = 255u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[10] = vector(parameter("ibl_color_top"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[16] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[17] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[18] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[19] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("constantoutline")[0]);
        staged.lightConstants[20] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.lightConstants[21] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[22] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-6ff78ae19259.v1")
    {
        [&]() {
        staged.program = 26u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[8] = vector(parameter("ibl_color_top"));
        staged.baseConstants[9] = vector(parameter("transcolor"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("constantoutline_color"));
        staged.baseConstants[12] = vector(parameter("hit_color"));
        staged.baseConstants[13] = vector(append(Value{},Value{},1u));
        staged.baseConstants[14] = vector(parameter("state_noise"));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[18] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[19] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[20] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("constantoutline")[0]);
        staged.baseConstants[21] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[22] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.lightTextureMask = 127u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[7] = vector(parameter("ibl_color_top"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("state_noise"));
        staged.lightConstants[10] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[12] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[13] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[14] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[15] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("constantoutline")[0]);
        staged.lightConstants[16] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.lightConstants[17] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[18] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-4184f1980abb.v1")
    {
        [&]() {
        staged.program = 27u;
        staged.baseTextureMask = 255u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[11] = vector(parameter("ibl_color_top"));
        staged.baseConstants[12] = vector(parameter("emissive_color"));
        staged.baseConstants[13] = vector(parameter("transcolor"));
        staged.baseConstants[14] = vector(parameter("buffcolor"));
        staged.baseConstants[15] = vector(parameter("constantoutline_color"));
        staged.baseConstants[16] = vector(parameter("hit_color"));
        staged.baseConstants[17] = vector(append(Value{},Value{},1u));
        staged.baseConstants[18] = vector(parameter("state_noise"));
        staged.baseConstants[19] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[20] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[21] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[22] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[23] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[24] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[25] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[26] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0]);
        staged.baseConstants[27] = float4_t(multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0]);
        staged.lightTextureMask = 479u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[10] = vector(parameter("ibl_color_top"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[16] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[17] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[18] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[19] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[20] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0]);
        staged.lightConstants[21] = float4_t(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0]);
        staged.lightConstants[22] = float4_t(parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0]);
        staged.lightConstants[23] = float4_t(parameter("pbr_specular_intensity")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-ce6425dcbdeb.v1")
    {
        [&]() {
        staged.program = 28u;
        staged.baseTextureMask = 255u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("diffusecolor"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[7] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[8] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[9] = vector(parameter("ibl_color_top"));
        staged.baseConstants[10] = vector(parameter("emissive_color"));
        staged.baseConstants[11] = vector(parameter("transcolor"));
        staged.baseConstants[12] = vector(parameter("buffcolor"));
        staged.baseConstants[13] = vector(parameter("constantoutline_color"));
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
        staged.baseConstants[24] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[25] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[26] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.lightTextureMask = 479u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[7] = vector(parameter("ibl_color_top"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("state_noise"));
        staged.lightConstants[10] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[12] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[14] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[15] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[16] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[17] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[18] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[19] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-63e86c4fac91.v1")
    {
        [&]() {
        staged.program = 29u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[8] = vector(parameter("ibl_color_top"));
        staged.baseConstants[9] = vector(parameter("transcolor"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("constantoutline_color"));
        staged.baseConstants[12] = vector(parameter("hit_color"));
        staged.baseConstants[13] = vector(append(Value{},Value{},1u));
        staged.baseConstants[14] = vector(parameter("state_noise"));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[18] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[19] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[20] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("constantoutline")[0]);
        staged.baseConstants[21] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[22] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.baseConstants[23] = float4_t(parameter("opacity_intensity")[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0]);
        staged.lightTextureMask = 127u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[7] = vector(parameter("ibl_color_top"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("state_noise"));
        staged.lightConstants[10] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[12] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[13] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[14] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[15] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("constantoutline")[0]);
        staged.lightConstants[16] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.lightConstants[17] = float4_t(parameter("opacity_intensity")[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0]);
        staged.lightConstants[18] = float4_t(parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0]);
        staged.lightConstants[19] = float4_t(parameter("pbr_specular_intensity")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-3b27d7fb1b53.v1")
    {
        [&]() {
        staged.program = 30u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[8] = vector(parameter("ibl_color_top"));
        staged.baseConstants[9] = vector(parameter("transcolor"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("constantoutline_color"));
        staged.baseConstants[12] = vector(parameter("hit_color"));
        staged.baseConstants[13] = vector(append(Value{},Value{},1u));
        staged.baseConstants[14] = vector(parameter("state_noise"));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[18] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[19] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[20] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("constantoutline")[0]);
        staged.baseConstants[21] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[22] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.lightTextureMask = 127u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[7] = vector(parameter("ibl_color_top"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("state_noise"));
        staged.lightConstants[10] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[12] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[13] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[14] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[15] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("constantoutline")[0]);
        staged.lightConstants[16] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.lightConstants[17] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[18] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-d1e4d915a43a.v1")
    {
        [&]() {
        staged.program = 31u;
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
        staged.baseConstants[12] = vector(parameter("constantoutline_color"));
        staged.baseConstants[13] = vector(parameter("hit_color"));
        staged.baseConstants[14] = vector(append(Value{},Value{},1u));
        staged.baseConstants[15] = vector(parameter("state_noise"));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[17] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[18] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.baseConstants[19] = float4_t(parameter("skin_roughness_power")[0],parameter("roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[20] = float4_t(parameter("ibl_normal_smooth")[0],parameter("ibl_skinlodscale")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0]);
        staged.baseConstants[21] = float4_t(parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[22] = float4_t(parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.baseConstants[23] = float4_t(parameter("fresnel_radius")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.baseConstants[24] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
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
        staged.lightConstants[19] = float4_t(parameter("fresnel_radius")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.lightConstants[20] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightConstants[21] = float4_t(parameter("orennayar_brightness")[0],parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("var_base_skinspecularpower_ui")[0]);
        staged.lightConstants[22] = float4_t(parameter("skin_specular_power")[0],parameter("skin_specular_power_min")[0],parameter("var_base_skinspecularintensity_ui")[0],parameter("skin_specular_intensity")[0]);
        staged.lightConstants[23] = float4_t(parameter("skin_specular_intensity_min")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-411db5c836e7.v1")
    {
        [&]() {
        staged.program = 32u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[8] = vector(parameter("ibl_color_top"));
        staged.baseConstants[9] = vector(parameter("transcolor"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("constantoutline_color"));
        staged.baseConstants[12] = vector(parameter("hit_color"));
        staged.baseConstants[13] = vector(append(Value{},Value{},1u));
        staged.baseConstants[14] = vector(parameter("state_noise"));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[18] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[19] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[20] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("constantoutline")[0]);
        staged.baseConstants[21] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[22] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.baseConstants[23] = float4_t(parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightTextureMask = 255u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[7] = vector(parameter("ibl_color_top"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("state_noise"));
        staged.lightConstants[10] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[12] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[13] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[14] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[15] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("constantoutline")[0]);
        staged.lightConstants[16] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.lightConstants[17] = float4_t(parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightConstants[18] = float4_t(parameter("orennayar_brightness")[0],parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0]);
        staged.lightConstants[19] = float4_t(parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-862fa1000fe2.v1")
    {
        [&]() {
        staged.program = 80u;
        staged.baseTextureMask = 15u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("hit_color"));
        staged.baseConstants[4] = vector(parameter("emissive_color"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("transcolor"));
        staged.baseConstants[8] = vector(parameter("buffcolor"));
        staged.baseConstants[9] = vector(parameter("constantoutline_color"));
        staged.baseConstants[10] = vector(parameter("diffusecolor"));
        staged.baseConstants[11] = vector(parameter("state"));
        staged.baseConstants[12] = vector(append(Value{},Value{},1u));
        staged.baseConstants[13] = vector(parameter("state_noise"));
        staged.baseConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[16] = float4_t(parameter("emissive_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[17] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[18] = float4_t(Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.baseConstants[19] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("constantoutline_power")[0]);
        staged.baseConstants[20] = float4_t(parameter("diffuse_brightness")[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0]);
        staged.lightTextureMask = 29u;
        staged.lightConstants[2] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[3] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[4] = vector(parameter("diffusecolor"));
        staged.lightConstants[5] = vector(parameter("state"));
        staged.lightConstants[6] = vector(append(Value{},Value{},1u));
        staged.lightConstants[7] = vector(parameter("state_noise"));
        staged.lightConstants[8] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[9] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[10] = vector(parameter("rimlight_color"));
        staged.lightConstants[11] = float4_t(parameter("emissive_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[12] = float4_t(Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.lightConstants[13] = float4_t(parameter("diffuse_brightness")[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0]);
        staged.lightConstants[14] = float4_t(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("rimlight_intensity")[0]);
        staged.lightConstants[15] = float4_t(parameter("rimlight_power")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-3c300c108ac5.v1")
    {
        [&]() {
        staged.program = 81u;
        staged.baseTextureMask = 15u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("hit_color"));
        staged.baseConstants[4] = vector(parameter("emissive_color"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("transcolor"));
        staged.baseConstants[8] = vector(parameter("buffcolor"));
        staged.baseConstants[9] = vector(parameter("constantoutline_color"));
        staged.baseConstants[10] = vector(parameter("diffusecolor"));
        staged.baseConstants[11] = vector(parameter("state"));
        staged.baseConstants[12] = vector(append(Value{},Value{},1u));
        staged.baseConstants[13] = vector(parameter("state_noise"));
        staged.baseConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[16] = float4_t(parameter("emissive_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[17] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[18] = float4_t(Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.baseConstants[19] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("diffuse_brightness")[0]);
        staged.lightTextureMask = 29u;
        staged.lightConstants[2] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[3] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[4] = vector(parameter("diffusecolor"));
        staged.lightConstants[5] = vector(parameter("state"));
        staged.lightConstants[6] = vector(append(Value{},Value{},1u));
        staged.lightConstants[7] = vector(parameter("state_noise"));
        staged.lightConstants[8] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[9] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[10] = vector(parameter("rimlight_color"));
        staged.lightConstants[11] = float4_t(parameter("emissive_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[12] = float4_t(Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.lightConstants[13] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("diffuse_brightness")[0]);
        staged.lightConstants[14] = float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-7373ec8df226.v1")
    {
        [&]() {
        staged.program = 82u;
        staged.baseTextureMask = 7u;
        staged.baseConstants[0] = vector(parameter("selectioncolor"));
        staged.baseConstants[1] = vector(parameter("hit_color"));
        staged.baseConstants[2] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[3] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[4] = vector(parameter("transcolor"));
        staged.baseConstants[5] = vector(parameter("constantoutline_color"));
        staged.baseConstants[6] = vector(parameter("diffusecolor"));
        staged.baseConstants[7] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[8] = float4_t(parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.baseConstants[9] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("constantoutline_power")[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[10] = float4_t(parameter("dead_texture_tiling")[0],parameter("dead")[0],0.f,0.f);
        staged.lightTextureMask = 7u;
        staged.lightConstants[0] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[1] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[2] = vector(parameter("diffusecolor"));
        staged.lightConstants[3] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[4] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("constantoutline_power")[0],parameter("diffuse_brightness")[0]);
        staged.lightConstants[5] = float4_t(parameter("specular_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("specular_saturation"))[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.lightConstants[6] = float4_t(parameter("dead_texture_tiling")[0],parameter("dead")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-a2e0ec089348.v1")
    {
        [&]() {
        staged.program = 83u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("hit_color"));
        staged.baseConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[6] = vector(parameter("transcolor"));
        staged.baseConstants[7] = vector(parameter("buffcolor"));
        staged.baseConstants[8] = vector(parameter("constantoutline_color"));
        staged.baseConstants[9] = vector(parameter("diffusecolor"));
        staged.baseConstants[10] = vector(parameter("reflection_color"));
        staged.baseConstants[11] = vector(parameter("state"));
        staged.baseConstants[12] = vector(append(Value{},Value{},1u));
        staged.baseConstants[13] = vector(parameter("state_noise"));
        staged.baseConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[16] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[17] = float4_t(parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.baseConstants[18] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("diffuse_brightness")[0],parameter("reflection_intensity")[0]);
        staged.lightTextureMask = 31u;
        staged.lightConstants[2] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[3] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[4] = vector(parameter("diffusecolor"));
        staged.lightConstants[5] = vector(parameter("reflection_color"));
        staged.lightConstants[6] = vector(parameter("state"));
        staged.lightConstants[7] = vector(append(Value{},Value{},1u));
        staged.lightConstants[8] = vector(parameter("state_noise"));
        staged.lightConstants[9] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[10] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[11] = vector(parameter("rimlight_color"));
        staged.lightConstants[12] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[13] = float4_t(parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.lightConstants[14] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("diffuse_brightness")[0],parameter("reflection_intensity")[0]);
        staged.lightConstants[15] = float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-8d18db0756e4.v1")
    {
        [&]() {
        staged.program = 84u;
        staged.baseTextureMask = 15u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("transcolor"));
        staged.baseConstants[3] = vector(parameter("buffcolor"));
        staged.baseConstants[4] = vector(parameter("rimlight_color"));
        staged.baseConstants[5] = vector(parameter("object_color"));
        staged.baseConstants[6] = vector(parameter("diffuse_color"));
        staged.baseConstants[7] = vector(append(periodic(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})),periodic(multiply(Value{},Value{0.f,0.f,0.f,0.f})),1u));
        staged.baseConstants[8] = vector(append(periodic(multiply(Value{},Value{0.f,0.f,0.f,0.f})),periodic(multiply(Value{},Value{0.300000012f,0.f,0.f,0.f})),1u));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("state"));
        staged.baseConstants[12] = vector(append(Value{},Value{},1u));
        staged.baseConstants[13] = vector(parameter("hit_color"));
        staged.baseConstants[14] = vector(parameter("specular_color"));
        staged.baseConstants[15] = float4_t(parameter("rimlight_power")[0],parameter("rimlight_intensity")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0]);
        staged.baseConstants[16] = float4_t(periodic(multiply(Value{},Value{0.300000012f,0.f,0.f,0.f}))[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0]);
        staged.baseConstants[17] = float4_t(multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(Value{0.5f,0.f,0.f,0.f},Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],multiply(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f}),multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(Value{0.5f,0.f,0.f,0.f},Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f}))[0],parameter("opacity_power")[0]);
        staged.baseConstants[18] = float4_t(parameter("opacity")[0],multiply(Value{1000.f,0.f,0.f,0.f},parameter("opacity"))[0],parameter("dead")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("dead"))[0]);
        staged.lightTextureMask = 15u;
        staged.lightConstants[1] = vector(parameter("selectioncolor"));
        staged.lightConstants[2] = vector(parameter("rimlight_color"));
        staged.lightConstants[3] = vector(parameter("object_color"));
        staged.lightConstants[4] = vector(parameter("diffuse_color"));
        staged.lightConstants[5] = vector(append(periodic(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})),periodic(multiply(Value{},Value{0.f,0.f,0.f,0.f})),1u));
        staged.lightConstants[6] = vector(append(periodic(multiply(Value{},Value{0.f,0.f,0.f,0.f})),periodic(multiply(Value{},Value{0.300000012f,0.f,0.f,0.f})),1u));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("state"));
        staged.lightConstants[10] = vector(append(Value{},Value{},1u));
        staged.lightConstants[11] = vector(parameter("specular_color"));
        staged.lightConstants[12] = float4_t(parameter("rimlight_power")[0],parameter("rimlight_intensity")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0]);
        staged.lightConstants[13] = float4_t(periodic(multiply(Value{},Value{0.300000012f,0.f,0.f,0.f}))[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0]);
        staged.lightConstants[14] = float4_t(multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(Value{0.5f,0.f,0.f,0.f},Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],multiply(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f}),multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(Value{0.5f,0.f,0.f,0.f},Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f}))[0],parameter("opacity_power")[0]);
        staged.lightConstants[15] = float4_t(parameter("opacity")[0],multiply(Value{1000.f,0.f,0.f,0.f},parameter("opacity"))[0],parameter("dead")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("dead"))[0]);
        }();
    }
    if (staged.program == 0u && family == "source.vehicle.terpeion-body.v1")
    {
        [&]() {
        staged.program = 85u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 127u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("diffusecolor"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[8] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[12] = vector(parameter("ibl_color_top"));
        staged.baseConstants[13] = vector(parameter("emissive_color"));
        staged.baseConstants[14] = vector(parameter("transcolor"));
        staged.baseConstants[15] = vector(parameter("buffcolor"));
        staged.baseConstants[16] = vector(parameter("constantoutline_color"));
        staged.baseConstants[17] = vector(parameter("hit_color"));
        staged.baseConstants[18] = vector(append(Value{},Value{},1u));
        staged.baseConstants[19] = vector(parameter("state_noise"));
        staged.baseConstants[20] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[21] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[22] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[23] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[24] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[25] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[26] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[27] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[28] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.lightTextureMask = 223u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[10] = vector(parameter("ibl_color_top"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[16] = float4_t(parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[17] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[18] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[19] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[20] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[21] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.vehicle.starlight-body.v1")
    {
        [&]() {
        staged.program = 86u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
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
        staged.baseConstants[14] = vector(parameter("constantoutline_color"));
        staged.baseConstants[15] = vector(parameter("hit_color"));
        staged.baseConstants[16] = vector(parameter("occlusion_color"));
        staged.baseConstants[17] = vector(append(Value{},Value{},1u));
        staged.baseConstants[18] = vector(parameter("ssstintcolor"));
        staged.baseConstants[19] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[20] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[21] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[22] = float4_t(parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],parameter("constantoutline_blink")[0]);
        staged.baseConstants[23] = float4_t(multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.baseConstants[24] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("roughness_power")[0],parameter("ssslocalthickness")[0]);
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
        staged.lightConstants[16] = float4_t(multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.lightConstants[17] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("roughness_power")[0],parameter("ssslocalthickness")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.vehicle.mokoboard-vfx.v1")
    {
        [&]() {
        staged.program = 87u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[5] = vector(parameter("selectioncolor"));
        staged.baseConstants[6] = vector(parameter("state"));
        staged.baseConstants[7] = vector(parameter("diffusecolor"));
        staged.baseConstants[8] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[9] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[10] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[11] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[12] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[13] = vector(parameter("flow1_uv_tile/speed"));
        staged.baseConstants[14] = vector(parameter("flow1_position/uv_distort/worldpos_type_rotate"));
        staged.baseConstants[15] = vector(parameter("flow_color_r/intensity"));
        staged.baseConstants[16] = vector(parameter("flow2_uv_tile/speed"));
        staged.baseConstants[17] = vector(parameter("flow2_position/uv_distort/worldpos_type_rotate"));
        staged.baseConstants[18] = vector(parameter("flow_color_g/intensity"));
        staged.baseConstants[19] = vector(parameter("flow_blend_color_b/intensity"));
        staged.baseConstants[20] = vector(parameter("flow_blend_tile/speed"));
        staged.baseConstants[21] = vector(parameter("rainbow_tile/speed/rotate/intensity"));
        staged.baseConstants[22] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[23] = vector(parameter("ibl_color_top"));
        staged.baseConstants[24] = vector(parameter("transcolor"));
        staged.baseConstants[25] = vector(parameter("buffcolor"));
        staged.baseConstants[26] = vector(parameter("hit_color"));
        staged.baseConstants[27] = vector(append(Value{},Value{},1u));
        staged.baseConstants[28] = vector(parameter("state_noise"));
        staged.baseConstants[29] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[30] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[31] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],Value{}[0]);
        staged.baseConstants[32] = float4_t(parameter("vfx_blendtype")[0],parameter("ibl_normal_smooth")[0],parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0]);
        staged.baseConstants[33] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[34] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.baseConstants[35] = float4_t(parameter("fresnel_radius")[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0]);
        staged.lightTextureMask = 255u;
        staged.lightConstants[5] = vector(parameter("state"));
        staged.lightConstants[6] = vector(parameter("diffusecolor"));
        staged.lightConstants[7] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[8] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[9] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[10] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[11] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[12] = vector(parameter("flow1_uv_tile/speed"));
        staged.lightConstants[13] = vector(parameter("flow1_position/uv_distort/worldpos_type_rotate"));
        staged.lightConstants[14] = vector(parameter("flow_color_r/intensity"));
        staged.lightConstants[15] = vector(parameter("flow2_uv_tile/speed"));
        staged.lightConstants[16] = vector(parameter("flow2_position/uv_distort/worldpos_type_rotate"));
        staged.lightConstants[17] = vector(parameter("flow_color_g/intensity"));
        staged.lightConstants[18] = vector(parameter("flow_blend_color_b/intensity"));
        staged.lightConstants[19] = vector(parameter("flow_blend_tile/speed"));
        staged.lightConstants[20] = vector(parameter("rainbow_tile/speed/rotate/intensity"));
        staged.lightConstants[21] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[22] = vector(parameter("ibl_color_top"));
        staged.lightConstants[23] = vector(append(Value{},Value{},1u));
        staged.lightConstants[24] = vector(parameter("state_noise"));
        staged.lightConstants[25] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[26] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[27] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],Value{}[0]);
        staged.lightConstants[28] = float4_t(parameter("vfx_blendtype")[0],parameter("ibl_normal_smooth")[0],parameter("roughness_power")[0],parameter("ibl_reflect_lodbias")[0]);
        staged.lightConstants[29] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[30] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.lightConstants[31] = float4_t(parameter("fresnel_radius")[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0]);
        staged.lightConstants[32] = float4_t(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("specular_power_limit")[0]);
        staged.lightConstants[33] = float4_t(parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.vehicle.starlight-shell-translucent.v1")
    {
        [&]() {
        staged.program = 88u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("state"));
        staged.baseConstants[3] = vector(parameter("diffusecolor"));
        staged.baseConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[6] = vector(parameter("reflection_color"));
        staged.baseConstants[7] = vector(parameter("fx_panningspeed_tiling"));
        staged.baseConstants[8] = vector(parameter("fx_panning_color"));
        staged.baseConstants[9] = vector(parameter("transcolor"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("hit_color"));
        staged.baseConstants[12] = vector(append(Value{},Value{},1u));
        staged.baseConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("reflection_tiling")[0]);
        staged.baseConstants[14] = float4_t(parameter("reflection_desaturation")[0],parameter("reflection_contrast")[0],parameter("reflection_brightness")[0],parameter("reflection_intensity")[0]);
        staged.baseConstants[15] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[16] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.baseConstants[17] = float4_t(parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("roughness_power")[0],parameter("opacity_intensity")[0]);
        staged.lightTextureMask = 95u;
        staged.lightConstants[1] = vector(parameter("state"));
        staged.lightConstants[2] = vector(parameter("diffusecolor"));
        staged.lightConstants[3] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[5] = vector(parameter("reflection_color"));
        staged.lightConstants[6] = vector(append(Value{},Value{},1u));
        staged.lightConstants[7] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("reflection_tiling")[0]);
        staged.lightConstants[8] = float4_t(parameter("reflection_desaturation")[0],parameter("reflection_contrast")[0],parameter("reflection_brightness")[0],parameter("reflection_intensity")[0]);
        staged.lightConstants[9] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[10] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightConstants[11] = float4_t(parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("roughness_power")[0],parameter("opacity_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.vehicle.ancient-myth-realpbr.v1")
    {
        [&]() {
        staged.program = 89u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 127u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("state"));
        staged.baseConstants[9] = vector(parameter("emissive_color"));
        staged.baseConstants[10] = vector(parameter("fx_panningspeed_tiling"));
        staged.baseConstants[11] = vector(parameter("fx_panning_color"));
        staged.baseConstants[12] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[13] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[14] = vector(parameter("transcolor"));
        staged.baseConstants[15] = vector(parameter("buffcolor"));
        staged.baseConstants[16] = vector(parameter("constantoutline_color"));
        staged.baseConstants[17] = vector(parameter("hit_color"));
        staged.baseConstants[18] = vector(parameter("occlusion_color"));
        staged.baseConstants[19] = vector(append(Value{},Value{},1u));
        staged.baseConstants[20] = vector(parameter("ssstintcolor"));
        staged.baseConstants[21] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[22] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[23] = float4_t(parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[24] = float4_t(parameter("trans_rim_hard")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0]);
        staged.baseConstants[25] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.baseConstants[26] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0]);
        staged.baseConstants[27] = float4_t(parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[28] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.baseConstants[29] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[30] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 79u;
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
        staged.lightConstants[15] = float4_t(parameter("trans_rim_hard")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0]);
        staged.lightConstants[16] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0]);
        staged.lightConstants[17] = float4_t(parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[18] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.lightConstants[19] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[20] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.classic-armor-skin.v1")
    {
        [&]() {
        staged.program = 90u;
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
        }();
    }
    if (staged.program == 0u && family == "source.character.classic-armor-plain.v1")
    {
        [&]() {
        staged.program = 91u;
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
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-dead-16902fea865c.v1")
    {
        [&]() {
        staged.program = 92u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 127u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("hit_color"));
        staged.baseConstants[4] = vector(parameter("emissive_color"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("transcolor"));
        staged.baseConstants[8] = vector(parameter("buffcolor"));
        staged.baseConstants[9] = vector(parameter("constantoutline_color"));
        staged.baseConstants[10] = vector(parameter("diffusecolor"));
        staged.baseConstants[11] = vector(parameter("reflection_color"));
        staged.baseConstants[12] = vector(parameter("state"));
        staged.baseConstants[13] = vector(append(Value{},Value{},1u));
        staged.baseConstants[14] = vector(parameter("state_noise"));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[17] = float4_t(parameter("emissive_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[18] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[19] = float4_t(Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.baseConstants[20] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("constantoutline_power")[0]);
        staged.baseConstants[21] = float4_t(parameter("diffuse_brightness")[0],parameter("reflection_intensity")[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[22] = float4_t(parameter("dead_texture_tiling")[0],parameter("dead")[0],multiply(parameter("specular_intensity"),Value{0.333332986f,0.f,0.f,0.f})[0],parameter("rimlight_intensity")[0]);
        staged.lightTextureMask = 125u;
        staged.lightConstants[2] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[3] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[4] = vector(parameter("diffusecolor"));
        staged.lightConstants[5] = vector(parameter("reflection_color"));
        staged.lightConstants[6] = vector(parameter("state"));
        staged.lightConstants[7] = vector(append(Value{},Value{},1u));
        staged.lightConstants[8] = vector(parameter("state_noise"));
        staged.lightConstants[9] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[10] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[11] = vector(parameter("rimlight_color"));
        staged.lightConstants[12] = float4_t(parameter("emissive_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[13] = float4_t(Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.lightConstants[14] = float4_t(parameter("diffuse_brightness")[0],parameter("reflection_intensity")[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[15] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.lightConstants[16] = float4_t(parameter("dead_texture_tiling")[0],parameter("dead")[0],multiply(parameter("specular_intensity"),Value{0.333332986f,0.f,0.f,0.f})[0],parameter("rimlight_intensity")[0]);
        staged.lightConstants[17] = float4_t(parameter("rimlight_power")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-dead-e15390cf9645.v1")
    {
        [&]() {
        staged.program = 93u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 23u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("hit_color"));
        staged.baseConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[6] = vector(parameter("transcolor"));
        staged.baseConstants[7] = vector(parameter("buffcolor"));
        staged.baseConstants[8] = vector(parameter("constantoutline_color"));
        staged.baseConstants[9] = vector(parameter("diffusecolor"));
        staged.baseConstants[10] = vector(parameter("state"));
        staged.baseConstants[11] = vector(append(Value{},Value{},1u));
        staged.baseConstants[12] = vector(parameter("state_noise"));
        staged.baseConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[15] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[16] = float4_t(parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.baseConstants[17] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("constantoutline_power")[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[18] = float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0]);
        staged.lightTextureMask = 31u;
        staged.lightConstants[2] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[3] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[4] = vector(parameter("diffusecolor"));
        staged.lightConstants[5] = vector(parameter("state"));
        staged.lightConstants[6] = vector(append(Value{},Value{},1u));
        staged.lightConstants[7] = vector(parameter("state_noise"));
        staged.lightConstants[8] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[9] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[10] = vector(parameter("rimlight_color"));
        staged.lightConstants[11] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[12] = float4_t(parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.lightConstants[13] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("constantoutline_power")[0],parameter("diffuse_brightness")[0]);
        staged.lightConstants[14] = float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0]);
        staged.lightConstants[15] = float4_t(multiply(parameter("specular_intensity"),Value{0.333332986f,0.f,0.f,0.f})[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.realpbr-avatar-ddk.v1")
    {
        [&]() {
        staged.program = 94u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[7] = vector(parameter("state"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("transcolor"));
        staged.baseConstants[11] = vector(parameter("buffcolor"));
        staged.baseConstants[12] = vector(parameter("hit_color"));
        staged.baseConstants[13] = vector(parameter("occlusion_color"));
        staged.baseConstants[14] = vector(append(Value{},Value{},1u));
        staged.baseConstants[15] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[16] = vector(parameter("ssstintcolor"));
        staged.baseConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[18] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[19] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],Value{}[0],parameter("auto_pbr_oc_max")[0]);
        staged.baseConstants[20] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[21] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.baseConstants[22] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[23] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 63u;
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
        staged.lightConstants[14] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[15] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[16] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],Value{}[0],parameter("auto_pbr_oc_max")[0]);
        staged.lightConstants[17] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[18] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.lightConstants[19] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[20] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.realpbr-avatar-ddk-plate.v1")
    {
        [&]() {
        staged.program = 95u;
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
        staged.baseConstants[22] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0]);
        staged.baseConstants[23] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[24] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.baseConstants[25] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[26] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
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
        staged.lightConstants[17] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0]);
        staged.lightConstants[18] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[19] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.lightConstants[20] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[21] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.realpbr-weapon-ddk.v1")
    {
        [&]() {
        staged.program = 96u;
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
        }();
    }
    if (staged.program == 0u && family == "source.character.realpbr-wing-ddk.v1")
    {
        [&]() {
        staged.program = 97u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("state"));
        staged.baseConstants[6] = vector(parameter("emissive_color"));
        staged.baseConstants[7] = vector(parameter("fx_panningspeed_tiling"));
        staged.baseConstants[8] = vector(parameter("fx_panning_color"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("transcolor"));
        staged.baseConstants[12] = vector(parameter("buffcolor"));
        staged.baseConstants[13] = vector(parameter("hit_color"));
        staged.baseConstants[14] = vector(parameter("occlusion_color"));
        staged.baseConstants[15] = vector(append(Value{},Value{},1u));
        staged.baseConstants[16] = vector(parameter("ssstintcolor"));
        staged.baseConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[18] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[19] = float4_t(parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[20] = float4_t(parameter("trans_rim_hard")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("occlusion_power")[0]);
        staged.baseConstants[21] = float4_t(parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0],parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.baseConstants[22] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("roughness_power")[0],parameter("ssslocalthickness")[0]);
        staged.lightTextureMask = 71u;
        staged.lightConstants[2] = vector(parameter("selectioncolor"));
        staged.lightConstants[3] = vector(parameter("basecolor_color"));
        staged.lightConstants[4] = vector(parameter("state"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[7] = vector(parameter("occlusion_color"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("ssstintcolor"));
        staged.lightConstants[10] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[11] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[12] = float4_t(parameter("trans_rim_hard")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("occlusion_power")[0]);
        staged.lightConstants[13] = float4_t(parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0],parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.lightConstants[14] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("roughness_power")[0],parameter("ssslocalthickness")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.realpbr-wing-ddk-membrane.v1")
    {
        [&]() {
        staged.program = 98u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("transcolor"));
        staged.baseConstants[8] = vector(parameter("buffcolor"));
        staged.baseConstants[9] = vector(parameter("hit_color"));
        staged.baseConstants[10] = vector(parameter("occlusion_color"));
        staged.baseConstants[11] = vector(append(Value{},Value{},1u));
        staged.baseConstants[12] = vector(parameter("ssstintcolor"));
        staged.baseConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[14] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[15] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],Value{}[0],parameter("auto_pbr_oc_max")[0]);
        staged.baseConstants[16] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[17] = float4_t(parameter("roughness_power")[0],parameter("ssslocalthickness")[0],0.f,0.f);
        staged.lightTextureMask = 23u;
        staged.lightConstants[2] = vector(parameter("selectioncolor"));
        staged.lightConstants[3] = vector(parameter("basecolor_color"));
        staged.lightConstants[4] = vector(parameter("state"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[7] = vector(parameter("occlusion_color"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("ssstintcolor"));
        staged.lightConstants[10] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[11] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[12] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],Value{}[0],parameter("auto_pbr_oc_max")[0]);
        staged.lightConstants[13] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[14] = float4_t(parameter("roughness_power")[0],parameter("ssslocalthickness")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.hair-ddk.v1")
    {
        [&]() {
        staged.program = 99u;
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
        }();
    }
    if (staged.program == 0u && family == "source.vehicle.ancient-sea-realpbr.v1")
    {
        [&]() {
        staged.program = 100u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 31u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[7] = vector(parameter("state"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("transcolor"));
        staged.baseConstants[11] = vector(parameter("buffcolor"));
        staged.baseConstants[12] = vector(parameter("constantoutline_color"));
        staged.baseConstants[13] = vector(parameter("hit_color"));
        staged.baseConstants[14] = vector(parameter("occlusion_color"));
        staged.baseConstants[15] = vector(append(Value{},Value{},1u));
        staged.baseConstants[16] = vector(parameter("ssstintcolor"));
        staged.baseConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[18] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[19] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0]);
        staged.baseConstants[20] = float4_t(multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.baseConstants[21] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.baseConstants[22] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[23] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f);
        staged.lightTextureMask = 31u;
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
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[14] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[15] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0]);
        staged.lightConstants[16] = float4_t(multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.lightConstants[17] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.lightConstants[18] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[19] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.guardianknight-sk-ddk-drg-00-body-mi-dead.v1")
    {
        [&]() {
        staged.program = 101u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[7] = vector(parameter("state"));
        staged.baseConstants[8] = vector(parameter("rimlightcolor"));
        staged.baseConstants[9] = vector(append(wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false),wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[11] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[12] = vector(parameter("transcolor"));
        staged.baseConstants[13] = vector(parameter("buffcolor"));
        staged.baseConstants[14] = vector(parameter("constantoutline_color"));
        staged.baseConstants[15] = vector(parameter("hit_color"));
        staged.baseConstants[16] = vector(parameter("occlusion_color"));
        staged.baseConstants[17] = vector(append(Value{},Value{},1u));
        staged.baseConstants[18] = vector(parameter("ssstintcolor"));
        staged.baseConstants[19] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.baseConstants[20] = float4_t(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f})[0],add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f})[0],multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),true)[0]);
        staged.baseConstants[21] = float4_t(wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],parameter("rimlight_spread")[0],multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f})[0],add(multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f}),Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[22] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_intensity")[0]);
        staged.baseConstants[23] = float4_t(parameter("rimlight_hardness")[0],parameter("rimlight_turn")[0],multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f})[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[24] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[25] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.baseConstants[26] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0]);
        staged.baseConstants[27] = float4_t(parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0]);
        staged.baseConstants[28] = float4_t(parameter("dead")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[29] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[30] = float4_t(multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f,0.f);
        staged.lightTextureMask = 63u;
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
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.lightConstants[14] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[15] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.lightConstants[16] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0]);
        staged.lightConstants[17] = float4_t(parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0]);
        staged.lightConstants[18] = float4_t(parameter("dead")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[19] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[20] = float4_t(multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.guardianknight-sk-ddk-drg-00-head-mi-dead.v1")
    {
        [&]() {
        staged.program = 102u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 255u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("state"));
        staged.baseConstants[9] = vector(parameter("rimlightcolor"));
        staged.baseConstants[10] = vector(append(wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false),wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[11] = vector(parameter("emissive_color"));
        staged.baseConstants[12] = vector(parameter("fx_panningspeed_tiling"));
        staged.baseConstants[13] = vector(parameter("fx_panning_color"));
        staged.baseConstants[14] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[15] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[16] = vector(parameter("transcolor"));
        staged.baseConstants[17] = vector(parameter("buffcolor"));
        staged.baseConstants[18] = vector(parameter("constantoutline_color"));
        staged.baseConstants[19] = vector(parameter("hit_color"));
        staged.baseConstants[20] = vector(parameter("occlusion_color"));
        staged.baseConstants[21] = vector(append(Value{},Value{},1u));
        staged.baseConstants[22] = vector(parameter("ssstintcolor"));
        staged.baseConstants[23] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.baseConstants[24] = float4_t(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f})[0],add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f})[0],multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),true)[0]);
        staged.baseConstants[25] = float4_t(wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],parameter("rimlight_spread")[0],multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f})[0],add(multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f}),Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[26] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_intensity")[0]);
        staged.baseConstants[27] = float4_t(parameter("rimlight_hardness")[0],parameter("rimlight_turn")[0],multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f})[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[28] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[29] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[30] = float4_t(parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[31] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[32] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.baseConstants[33] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[34] = float4_t(parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("dye_roughness_power_tensition")[0]);
        staged.baseConstants[35] = float4_t(parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0]);
        staged.baseConstants[36] = float4_t(multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0]);
        staged.lightTextureMask = 207u;
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
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.lightConstants[14] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[15] = float4_t(parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[16] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[17] = float4_t(parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("dye_roughness_power_tensition")[0]);
        staged.lightConstants[18] = float4_t(parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0]);
        staged.lightConstants[19] = float4_t(multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.guardianknight-sk-ddk-drg-00-neck-mi-dead.v1")
    {
        [&]() {
        staged.program = 103u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 255u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("state"));
        staged.baseConstants[9] = vector(parameter("rimlightcolor"));
        staged.baseConstants[10] = vector(append(wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false),wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[11] = vector(parameter("emissive_color"));
        staged.baseConstants[12] = vector(parameter("fx_panningspeed_tiling"));
        staged.baseConstants[13] = vector(parameter("fx_panning_color"));
        staged.baseConstants[14] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[15] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[16] = vector(parameter("transcolor"));
        staged.baseConstants[17] = vector(parameter("buffcolor"));
        staged.baseConstants[18] = vector(parameter("constantoutline_color"));
        staged.baseConstants[19] = vector(parameter("hit_color"));
        staged.baseConstants[20] = vector(parameter("occlusion_color"));
        staged.baseConstants[21] = vector(append(Value{},Value{},1u));
        staged.baseConstants[22] = vector(parameter("ssstintcolor"));
        staged.baseConstants[23] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.baseConstants[24] = float4_t(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f})[0],add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f})[0],multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),true)[0]);
        staged.baseConstants[25] = float4_t(wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],parameter("rimlight_spread")[0],multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f})[0],add(multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f}),Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[26] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_intensity")[0]);
        staged.baseConstants[27] = float4_t(parameter("rimlight_hardness")[0],parameter("rimlight_turn")[0],multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f})[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[28] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[29] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[30] = float4_t(parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[31] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[32] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.baseConstants[33] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[34] = float4_t(parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("dye_roughness_power_tensition")[0]);
        staged.baseConstants[35] = float4_t(parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0]);
        staged.baseConstants[36] = float4_t(multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0]);
        staged.lightTextureMask = 207u;
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
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.lightConstants[14] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[15] = float4_t(parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[16] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[17] = float4_t(parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("dye_roughness_power_tensition")[0]);
        staged.lightConstants[18] = float4_t(parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0]);
        staged.lightConstants[19] = float4_t(multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.guardianknight-sk-ddk-drg-00-wing1-mi-dead.v1")
    {
        [&]() {
        staged.program = 104u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[7] = vector(parameter("state"));
        staged.baseConstants[8] = vector(parameter("rimlightcolor"));
        staged.baseConstants[9] = vector(append(wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false),wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[11] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[12] = vector(parameter("transcolor"));
        staged.baseConstants[13] = vector(parameter("buffcolor"));
        staged.baseConstants[14] = vector(parameter("constantoutline_color"));
        staged.baseConstants[15] = vector(parameter("hit_color"));
        staged.baseConstants[16] = vector(parameter("occlusion_color"));
        staged.baseConstants[17] = vector(append(Value{},Value{},1u));
        staged.baseConstants[18] = vector(parameter("ssstintcolor"));
        staged.baseConstants[19] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.baseConstants[20] = float4_t(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f})[0],add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f})[0],multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),true)[0]);
        staged.baseConstants[21] = float4_t(wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],parameter("rimlight_spread")[0],multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f})[0],add(multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f}),Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[22] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_intensity")[0]);
        staged.baseConstants[23] = float4_t(parameter("rimlight_hardness")[0],parameter("rimlight_turn")[0],multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f})[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[24] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[25] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.baseConstants[26] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0]);
        staged.baseConstants[27] = float4_t(parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0]);
        staged.baseConstants[28] = float4_t(parameter("dead")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[29] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[30] = float4_t(multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f,0.f);
        staged.lightTextureMask = 63u;
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
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.lightConstants[14] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[15] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.lightConstants[16] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0]);
        staged.lightConstants[17] = float4_t(parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0]);
        staged.lightConstants[18] = float4_t(parameter("dead")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[19] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[20] = float4_t(multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.guardianknight-sk-ddk-drg-00-wing-mi-dead.v1")
    {
        [&]() {
        staged.program = 105u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 255u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("state"));
        staged.baseConstants[9] = vector(parameter("rimlightcolor"));
        staged.baseConstants[10] = vector(append(wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false),wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[11] = vector(parameter("emissive_color"));
        staged.baseConstants[12] = vector(parameter("fx_panningspeed_tiling"));
        staged.baseConstants[13] = vector(parameter("fx_panning_color"));
        staged.baseConstants[14] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[15] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[16] = vector(parameter("transcolor"));
        staged.baseConstants[17] = vector(parameter("buffcolor"));
        staged.baseConstants[18] = vector(parameter("constantoutline_color"));
        staged.baseConstants[19] = vector(parameter("hit_color"));
        staged.baseConstants[20] = vector(parameter("occlusion_color"));
        staged.baseConstants[21] = vector(append(Value{},Value{},1u));
        staged.baseConstants[22] = vector(parameter("ssstintcolor"));
        staged.baseConstants[23] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.baseConstants[24] = float4_t(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f})[0],add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f})[0],multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),true)[0]);
        staged.baseConstants[25] = float4_t(wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],parameter("rimlight_spread")[0],multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f})[0],add(multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f}),Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[26] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_intensity")[0]);
        staged.baseConstants[27] = float4_t(parameter("rimlight_hardness")[0],parameter("rimlight_turn")[0],multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f})[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[28] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[29] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[30] = float4_t(parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[31] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[32] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.baseConstants[33] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[34] = float4_t(parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("dye_roughness_power_tensition")[0]);
        staged.baseConstants[35] = float4_t(parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0]);
        staged.baseConstants[36] = float4_t(multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0]);
        staged.lightTextureMask = 207u;
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
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.lightConstants[14] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[15] = float4_t(parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[16] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[17] = float4_t(parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("dye_roughness_power_tensition")[0]);
        staged.lightConstants[18] = float4_t(parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0]);
        staged.lightConstants[19] = float4_t(multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.guardianknight-sk-ddk-drr-00-body-mi.v1")
    {
        [&]() {
        staged.program = 106u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 127u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("rimlightcolor"));
        staged.baseConstants[6] = vector(append(wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false),wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[7] = vector(parameter("emissive_color"));
        staged.baseConstants[8] = vector(parameter("fx_panningspeed_tiling"));
        staged.baseConstants[9] = vector(parameter("fx_panning_color"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[11] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[12] = vector(parameter("transcolor"));
        staged.baseConstants[13] = vector(parameter("buffcolor"));
        staged.baseConstants[14] = vector(parameter("constantoutline_color"));
        staged.baseConstants[15] = vector(parameter("hit_color"));
        staged.baseConstants[16] = vector(parameter("occlusion_color"));
        staged.baseConstants[17] = vector(append(Value{},Value{},1u));
        staged.baseConstants[18] = vector(parameter("ssstintcolor"));
        staged.baseConstants[19] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.baseConstants[20] = float4_t(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f})[0],add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f})[0],multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),true)[0]);
        staged.baseConstants[21] = float4_t(wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],parameter("rimlight_spread")[0],multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f})[0],add(multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f}),Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[22] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_intensity")[0]);
        staged.baseConstants[23] = float4_t(parameter("rimlight_hardness")[0],parameter("rimlight_turn")[0],multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f})[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[24] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[25] = float4_t(Value{}[0],parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[26] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],parameter("constantoutline_blink")[0]);
        staged.baseConstants[27] = float4_t(multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.baseConstants[28] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0]);
        staged.baseConstants[29] = float4_t(parameter("roughness_power")[0],parameter("ssslocalthickness")[0],0.f,0.f);
        staged.lightTextureMask = 103u;
        staged.lightConstants[2] = vector(parameter("selectioncolor"));
        staged.lightConstants[3] = vector(parameter("basecolor_color"));
        staged.lightConstants[4] = vector(parameter("state"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[7] = vector(parameter("occlusion_color"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("ssstintcolor"));
        staged.lightConstants[10] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.lightConstants[11] = float4_t(Value{}[0],parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[12] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],parameter("constantoutline_blink")[0]);
        staged.lightConstants[13] = float4_t(multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.lightConstants[14] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0]);
        staged.lightConstants[15] = float4_t(parameter("roughness_power")[0],parameter("ssslocalthickness")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.guardianknight-sk-ddk-drr-00-head-mi.v1")
    {
        [&]() {
        staged.program = 107u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 255u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("state"));
        staged.baseConstants[9] = vector(parameter("rimlightcolor"));
        staged.baseConstants[10] = vector(append(wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false),wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[11] = vector(parameter("emissive_color"));
        staged.baseConstants[12] = vector(parameter("fx_panningspeed_tiling"));
        staged.baseConstants[13] = vector(parameter("fx_panning_color"));
        staged.baseConstants[14] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[15] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[16] = vector(parameter("transcolor"));
        staged.baseConstants[17] = vector(parameter("buffcolor"));
        staged.baseConstants[18] = vector(parameter("constantoutline_color"));
        staged.baseConstants[19] = vector(parameter("hit_color"));
        staged.baseConstants[20] = vector(parameter("occlusion_color"));
        staged.baseConstants[21] = vector(append(Value{},Value{},1u));
        staged.baseConstants[22] = vector(parameter("ssstintcolor"));
        staged.baseConstants[23] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.baseConstants[24] = float4_t(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f})[0],add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f})[0],multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),true)[0]);
        staged.baseConstants[25] = float4_t(wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],parameter("rimlight_spread")[0],multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f})[0],add(multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f}),Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[26] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_intensity")[0]);
        staged.baseConstants[27] = float4_t(parameter("rimlight_hardness")[0],parameter("rimlight_turn")[0],multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f})[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[28] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[29] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[30] = float4_t(parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[31] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0]);
        staged.baseConstants[32] = float4_t(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0]);
        staged.baseConstants[33] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[34] = float4_t(parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("roughness_power")[0]);
        staged.baseConstants[35] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 207u;
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
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.lightConstants[14] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[15] = float4_t(parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[16] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[17] = float4_t(parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("roughness_power")[0]);
        staged.lightConstants[18] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.guardianknight-sk-ddk-drr-00-neck-mi.v1")
    {
        [&]() {
        staged.program = 108u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 63u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("state"));
        staged.baseConstants[6] = vector(parameter("rimlightcolor"));
        staged.baseConstants[7] = vector(append(wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false),wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[8] = vector(parameter("emissive_color"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("transcolor"));
        staged.baseConstants[12] = vector(parameter("buffcolor"));
        staged.baseConstants[13] = vector(parameter("constantoutline_color"));
        staged.baseConstants[14] = vector(parameter("hit_color"));
        staged.baseConstants[15] = vector(parameter("occlusion_color"));
        staged.baseConstants[16] = vector(append(Value{},Value{},1u));
        staged.baseConstants[17] = vector(parameter("ssstintcolor"));
        staged.baseConstants[18] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.baseConstants[19] = float4_t(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f})[0],add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f})[0],multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f})[0],wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),true)[0]);
        staged.baseConstants[20] = float4_t(wave(multiply(add(divide(parameter("rimlight_angle"),Value{360.f,0.f,0.f,0.f}),Value{0.25f,0.f,0.f,0.f}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],parameter("rimlight_spread")[0],multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f})[0],add(multiply(parameter("rimlight_spread"),Value{-1.f,0.f,0.f,0.f}),Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[21] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_intensity")[0]);
        staged.baseConstants[22] = float4_t(parameter("rimlight_hardness")[0],parameter("rimlight_turn")[0],multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f})[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[23] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[24] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[25] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[26] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0]);
        staged.baseConstants[27] = float4_t(parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0]);
        staged.baseConstants[28] = float4_t(parameter("dead")[0],parameter("roughness_power")[0],parameter("ssslocalthickness")[0],0.f);
        staged.lightTextureMask = 55u;
        staged.lightConstants[2] = vector(parameter("selectioncolor"));
        staged.lightConstants[3] = vector(parameter("basecolor_color"));
        staged.lightConstants[4] = vector(parameter("state"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[7] = vector(parameter("occlusion_color"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("ssstintcolor"));
        staged.lightConstants[10] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_angle")[0]);
        staged.lightConstants[11] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[12] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[13] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0]);
        staged.lightConstants[14] = float4_t(parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0]);
        staged.lightConstants[15] = float4_t(parameter("dead")[0],parameter("roughness_power")[0],parameter("ssslocalthickness")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.monster-fd6df5a0ab9a.v1")
    {
        [&]() {
        staged.program = 109u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 15u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("hit_color"));
        staged.baseConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[6] = vector(parameter("transcolor"));
        staged.baseConstants[7] = vector(parameter("buffcolor"));
        staged.baseConstants[8] = vector(parameter("constantoutline_color"));
        staged.baseConstants[9] = vector(parameter("diffusecolor"));
        staged.baseConstants[10] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[11] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[12] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[13] = vector(parameter("state"));
        staged.baseConstants[14] = vector(append(Value{},Value{},1u));
        staged.baseConstants[15] = vector(parameter("state_noise"));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[17] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[18] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[19] = float4_t(parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.baseConstants[20] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("constantoutline_power")[0],parameter("diffuse_brightness")[0]);
        staged.lightTextureMask = 15u;
        staged.lightConstants[2] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[3] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[4] = vector(parameter("diffusecolor"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[7] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[8] = vector(parameter("state"));
        staged.lightConstants[9] = vector(append(Value{},Value{},1u));
        staged.lightConstants[10] = vector(parameter("state_noise"));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[13] = vector(parameter("rimlight_color"));
        staged.lightConstants[14] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[15] = float4_t(parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.lightConstants[16] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("constantoutline_power")[0],parameter("diffuse_brightness")[0]);
        staged.lightConstants[17] = float4_t(parameter("specular_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("specular_saturation"))[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.lightConstants[18] = float4_t(parameter("rimlight_intensity")[0],parameter("rimlight_power")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.guardianknight-sk-ddk-drr-01-head-st-mi-fx-dead.v1")
    {
        [&]() {
        staged.program = 110u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 31u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("state"));
        staged.baseConstants[6] = vector(parameter("rimlightcolor"));
        staged.baseConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[9] = vector(parameter("transcolor"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("constantoutline_color"));
        staged.baseConstants[12] = vector(parameter("hit_color"));
        staged.baseConstants[13] = vector(parameter("occlusion_color"));
        staged.baseConstants[14] = vector(append(Value{},Value{},1u));
        staged.baseConstants[15] = vector(parameter("ssstintcolor"));
        staged.baseConstants[16] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_intensity")[0]);
        staged.baseConstants[17] = float4_t(parameter("rimlight_hardness")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[18] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[19] = float4_t(Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.baseConstants[20] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0]);
        staged.baseConstants[21] = float4_t(parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[22] = float4_t(parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("opacity_intensity")[0],parameter("roughness_power")[0]);
        staged.baseConstants[23] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 31u;
        staged.lightConstants[3] = vector(parameter("selectioncolor"));
        staged.lightConstants[4] = vector(parameter("basecolor_color"));
        staged.lightConstants[5] = vector(parameter("state"));
        staged.lightConstants[6] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[8] = vector(parameter("occlusion_color"));
        staged.lightConstants[9] = vector(append(Value{},Value{},1u));
        staged.lightConstants[10] = vector(parameter("ssstintcolor"));
        staged.lightConstants[11] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_intensity")[0]);
        staged.lightConstants[12] = float4_t(parameter("rimlight_hardness")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[13] = float4_t(Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.lightConstants[14] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0]);
        staged.lightConstants[15] = float4_t(parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[16] = float4_t(parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("opacity_intensity")[0],parameter("roughness_power")[0]);
        staged.lightConstants[17] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.guardianknight-sk-ddk-drr-01-neck-st-mi-fx-dead.v1")
    {
        [&]() {
        staged.program = 111u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 31u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("state"));
        staged.baseConstants[6] = vector(parameter("rimlightcolor"));
        staged.baseConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[9] = vector(parameter("transcolor"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("constantoutline_color"));
        staged.baseConstants[12] = vector(parameter("hit_color"));
        staged.baseConstants[13] = vector(parameter("occlusion_color"));
        staged.baseConstants[14] = vector(append(Value{},Value{},1u));
        staged.baseConstants[15] = vector(parameter("ssstintcolor"));
        staged.baseConstants[16] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_intensity")[0]);
        staged.baseConstants[17] = float4_t(parameter("rimlight_hardness")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[18] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[19] = float4_t(Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.baseConstants[20] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0]);
        staged.baseConstants[21] = float4_t(parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[22] = float4_t(parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("opacity_intensity")[0],parameter("roughness_power")[0]);
        staged.baseConstants[23] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 31u;
        staged.lightConstants[3] = vector(parameter("selectioncolor"));
        staged.lightConstants[4] = vector(parameter("basecolor_color"));
        staged.lightConstants[5] = vector(parameter("state"));
        staged.lightConstants[6] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[8] = vector(parameter("occlusion_color"));
        staged.lightConstants[9] = vector(append(Value{},Value{},1u));
        staged.lightConstants[10] = vector(parameter("ssstintcolor"));
        staged.lightConstants[11] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_intensity")[0]);
        staged.lightConstants[12] = float4_t(parameter("rimlight_hardness")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[13] = float4_t(Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.lightConstants[14] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0]);
        staged.lightConstants[15] = float4_t(parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[16] = float4_t(parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("opacity_intensity")[0],parameter("roughness_power")[0]);
        staged.lightConstants[17] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.classic-armor-skin-masked.v1")
    {
        [&]() {
        staged.program = 112u;
        staged.baseTextureMask = 127u;
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
        staged.baseConstants[19] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[20] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.baseConstants[21] = float4_t(parameter("skin_roughness_power")[0],parameter("roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[22] = float4_t(parameter("ibl_normal_smooth")[0],parameter("ibl_skinlodscale")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0]);
        staged.baseConstants[23] = float4_t(parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[24] = float4_t(parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.baseConstants[25] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightTextureMask = 255u;
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
        staged.lightConstants[15] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[16] = vector(parameter("half_lambert_skin"));
        staged.lightConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.lightConstants[18] = float4_t(parameter("skin_roughness_power")[0],parameter("roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[19] = float4_t(parameter("ibl_normal_smooth")[0],parameter("ibl_skinlodscale")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0]);
        staged.lightConstants[20] = float4_t(parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[21] = float4_t(parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.lightConstants[22] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[23] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightConstants[24] = float4_t(parameter("orennayar_brightness")[0],parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("var_base_skinspecularpower_ui")[0]);
        staged.lightConstants[25] = float4_t(parameter("skin_specular_power")[0],parameter("skin_specular_power_min")[0],parameter("var_base_skinspecularintensity_ui")[0],parameter("skin_specular_intensity")[0]);
        staged.lightConstants[26] = float4_t(parameter("skin_specular_intensity_min")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-160.v1")
    {
        [&]() {
        staged.program = 160u;
        staged.requiredExtraUVMask = 1u;
        staged.baseTextureMask = 15u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("var_base_haircolor_base_ui"));
        staged.baseConstants[4] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.baseConstants[8] = vector(parameter("hit_color"));
        staged.baseConstants[9] = vector(parameter("buffcolor"));
        staged.baseConstants[10] = vector(parameter("transcolor"));
        staged.baseConstants[11] = vector(parameter("cutting_mask"));
        staged.baseConstants[12] = vector(parameter("state"));
        staged.baseConstants[13] = vector(append(Value{},Value{},1u));
        staged.baseConstants[14] = vector(parameter("state_noise"));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[17] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.baseConstants[18] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[19] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.baseConstants[20] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[21] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0]);
        staged.baseConstants[22] = float4_t(parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[23] = float4_t(parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0]);
        staged.lightTextureMask = 15u;
        staged.lightConstants[2] = vector(parameter("var_base_haircolor_base_ui"));
        staged.lightConstants[3] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.lightConstants[7] = vector(parameter("cutting_mask"));
        staged.lightConstants[8] = vector(parameter("rimlight_color"));
        staged.lightConstants[9] = vector(parameter("state"));
        staged.lightConstants[10] = vector(append(Value{},Value{},1u));
        staged.lightConstants[11] = vector(parameter("state_noise"));
        staged.lightConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[14] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.lightConstants[15] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[16] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.lightConstants[17] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[18] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0]);
        staged.lightConstants[19] = float4_t(parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0]);
        staged.lightConstants[20] = float4_t(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("customshade_shadowdiffuselighting")[0]);
        staged.lightConstants[21] = float4_t(parameter("specular_power")[0],parameter("specular_intensity")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-161.v1")
    {
        [&]() {
        staged.program = 161u;
        staged.requiredExtraUVMask = 1u;
        staged.baseTextureMask = 15u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("var_base_haircolor_base_ui"));
        staged.baseConstants[4] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.baseConstants[8] = vector(parameter("hit_color"));
        staged.baseConstants[9] = vector(parameter("state"));
        staged.baseConstants[10] = vector(append(Value{},Value{},1u));
        staged.baseConstants[11] = vector(parameter("state_noise"));
        staged.baseConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[14] = vector(parameter("buffcolor"));
        staged.baseConstants[15] = vector(parameter("transcolor"));
        staged.baseConstants[16] = vector(parameter("cutting_mask"));
        staged.baseConstants[17] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.baseConstants[18] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[19] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.baseConstants[20] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[21] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0]);
        staged.baseConstants[22] = float4_t(parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0]);
        staged.baseConstants[23] = float4_t(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("ambientlight")[0]);
        staged.baseConstants[24] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("opacity_clamp")[0]);
        staged.lightTextureMask = 15u;
        staged.lightConstants[2] = vector(parameter("var_base_haircolor_base_ui"));
        staged.lightConstants[3] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.lightConstants[7] = vector(parameter("state"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("state_noise"));
        staged.lightConstants[10] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[12] = vector(parameter("cutting_mask"));
        staged.lightConstants[13] = vector(parameter("rimlight_color"));
        staged.lightConstants[14] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.lightConstants[15] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[16] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.lightConstants[17] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[18] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0]);
        staged.lightConstants[19] = float4_t(parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0]);
        staged.lightConstants[20] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("opacity_clamp")[0]);
        staged.lightConstants[21] = float4_t(parameter("rimlight_power")[0],parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0],parameter("specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-162.v1")
    {
        [&]() {
        staged.program = 162u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("var_base_skincolor_ui"));
        staged.baseConstants[11] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[12] = vector(parameter("ibl_color_top"));
        staged.baseConstants[13] = vector(parameter("transcolor"));
        staged.baseConstants[14] = vector(parameter("buffcolor"));
        staged.baseConstants[15] = vector(parameter("hit_color"));
        staged.baseConstants[16] = vector(append(Value{},Value{},1u));
        staged.baseConstants[17] = vector(parameter("state_noise"));
        staged.baseConstants[18] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[19] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[20] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.baseConstants[21] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[22] = float4_t(parameter("skin_roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[23] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.baseConstants[24] = float4_t(parameter("ibl_skinlodscale")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0]);
        staged.baseConstants[25] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.baseConstants[26] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightTextureMask = 127u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("var_base_skincolor_ui"));
        staged.lightConstants[10] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[11] = vector(parameter("ibl_color_top"));
        staged.lightConstants[12] = vector(append(Value{},Value{},1u));
        staged.lightConstants[13] = vector(parameter("state_noise"));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[16] = vector(parameter("half_lambert_skin"));
        staged.lightConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.lightConstants[18] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[19] = float4_t(parameter("skin_roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[20] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.lightConstants[21] = float4_t(parameter("ibl_skinlodscale")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0]);
        staged.lightConstants[22] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.lightConstants[23] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightConstants[24] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[25] = float4_t(parameter("specular_power_limit")[0],parameter("var_base_skinspecularpower_ui")[0],parameter("skin_specular_power")[0],parameter("skin_specular_power_min")[0]);
        staged.lightConstants[26] = float4_t(parameter("var_base_skinspecularintensity_ui")[0],parameter("skin_specular_intensity")[0],parameter("skin_specular_intensity_min")[0],parameter("beckmannspecular_constant_max")[0]);
        staged.lightConstants[27] = float4_t(parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-163.v1")
    {
        [&]() {
        staged.program = 163u;
        staged.baseTextureMask = 255u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("diffusecolor"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("var_base_skincolor_ui"));
        staged.baseConstants[11] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[12] = vector(parameter("ibl_color_top"));
        staged.baseConstants[13] = vector(parameter("emissive_color"));
        staged.baseConstants[14] = vector(parameter("transcolor"));
        staged.baseConstants[15] = vector(parameter("buffcolor"));
        staged.baseConstants[16] = vector(parameter("hit_color"));
        staged.baseConstants[17] = vector(append(Value{},Value{},1u));
        staged.baseConstants[18] = vector(parameter("state_noise"));
        staged.baseConstants[19] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[20] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[21] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[22] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.baseConstants[23] = float4_t(parameter("skin_roughness_power")[0],parameter("roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[24] = float4_t(parameter("ibl_normal_smooth")[0],parameter("ibl_skinlodscale")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0]);
        staged.baseConstants[25] = float4_t(parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[26] = float4_t(parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.baseConstants[27] = float4_t(parameter("fresnel_radius")[0],parameter("emissive_intensity")[0],parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0]);
        staged.baseConstants[28] = float4_t(Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightTextureMask = 479u;
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
        staged.lightConstants[15] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[16] = vector(parameter("half_lambert_skin"));
        staged.lightConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.lightConstants[18] = float4_t(parameter("skin_roughness_power")[0],parameter("roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[19] = float4_t(parameter("ibl_normal_smooth")[0],parameter("ibl_skinlodscale")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0]);
        staged.lightConstants[20] = float4_t(parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[21] = float4_t(parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.lightConstants[22] = float4_t(parameter("fresnel_radius")[0],parameter("emissive_intensity")[0],parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0]);
        staged.lightConstants[23] = float4_t(Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[24] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightConstants[25] = float4_t(parameter("orennayar_brightness")[0],parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("var_base_skinspecularpower_ui")[0]);
        staged.lightConstants[26] = float4_t(parameter("skin_specular_power")[0],parameter("skin_specular_power_min")[0],parameter("var_base_skinspecularintensity_ui")[0],parameter("skin_specular_intensity")[0]);
        staged.lightConstants[27] = float4_t(parameter("skin_specular_intensity_min")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-164.v1")
    {
        [&]() {
        staged.program = 164u;
        staged.baseTextureMask = 127u;
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
        staged.baseConstants[19] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[20] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.baseConstants[21] = float4_t(parameter("skin_roughness_power")[0],parameter("roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[22] = float4_t(parameter("ibl_normal_smooth")[0],parameter("ibl_skinlodscale")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0]);
        staged.baseConstants[23] = float4_t(parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[24] = float4_t(parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.baseConstants[25] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightTextureMask = 255u;
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
        staged.lightConstants[15] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[16] = vector(parameter("half_lambert_skin"));
        staged.lightConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.lightConstants[18] = float4_t(parameter("skin_roughness_power")[0],parameter("roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[19] = float4_t(parameter("ibl_normal_smooth")[0],parameter("ibl_skinlodscale")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0]);
        staged.lightConstants[20] = float4_t(parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[21] = float4_t(parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.lightConstants[22] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[23] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightConstants[24] = float4_t(parameter("orennayar_brightness")[0],parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("var_base_skinspecularpower_ui")[0]);
        staged.lightConstants[25] = float4_t(parameter("skin_specular_power")[0],parameter("skin_specular_power_min")[0],parameter("var_base_skinspecularintensity_ui")[0],parameter("skin_specular_intensity")[0]);
        staged.lightConstants[26] = float4_t(parameter("skin_specular_intensity_min")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-165.v1")
    {
        [&]() {
        staged.program = 165u;
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
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-166.v1")
    {
        [&]() {
        staged.program = 166u;
        staged.requiredExtraUVMask = 1u;
        staged.baseTextureMask = 15u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("var_base_haircolor_base_ui"));
        staged.baseConstants[4] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("accessory_color"));
        staged.baseConstants[8] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.baseConstants[9] = vector(parameter("hit_color"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("transcolor"));
        staged.baseConstants[12] = vector(parameter("cutting_mask"));
        staged.baseConstants[13] = vector(parameter("state"));
        staged.baseConstants[14] = vector(append(Value{},Value{},1u));
        staged.baseConstants[15] = vector(parameter("state_noise"));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[17] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[18] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.baseConstants[19] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[20] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.baseConstants[21] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[22] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("accessory_specular_intensity")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0]);
        staged.baseConstants[23] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[24] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0]);
        staged.lightTextureMask = 15u;
        staged.lightConstants[2] = vector(parameter("var_base_haircolor_base_ui"));
        staged.lightConstants[3] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(parameter("accessory_color"));
        staged.lightConstants[7] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.lightConstants[8] = vector(parameter("cutting_mask"));
        staged.lightConstants[9] = vector(parameter("rimlight_color"));
        staged.lightConstants[10] = vector(parameter("state"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.lightConstants[16] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[17] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.lightConstants[18] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[19] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("accessory_specular_intensity")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0]);
        staged.lightConstants[20] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[21] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0]);
        staged.lightConstants[22] = float4_t(parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0],parameter("specular_intensity")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-167.v1")
    {
        [&]() {
        staged.program = 167u;
        staged.requiredExtraUVMask = 1u;
        staged.baseTextureMask = 15u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("var_base_haircolor_base_ui"));
        staged.baseConstants[4] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.baseConstants[8] = vector(parameter("hit_color"));
        staged.baseConstants[9] = vector(parameter("accessory_color"));
        staged.baseConstants[10] = vector(parameter("state"));
        staged.baseConstants[11] = vector(append(Value{},Value{},1u));
        staged.baseConstants[12] = vector(parameter("state_noise"));
        staged.baseConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[15] = vector(parameter("buffcolor"));
        staged.baseConstants[16] = vector(parameter("transcolor"));
        staged.baseConstants[17] = vector(parameter("cutting_mask"));
        staged.baseConstants[18] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.baseConstants[19] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[20] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.baseConstants[21] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[22] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("accessory_specular_intensity")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0]);
        staged.baseConstants[23] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],Value{}[0]);
        staged.baseConstants[24] = float4_t(parameter("ambientlight")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[25] = float4_t(parameter("opacity_clamp")[0],parameter("rimlight_power")[0],parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0]);
        staged.lightTextureMask = 15u;
        staged.lightConstants[2] = vector(parameter("var_base_haircolor_base_ui"));
        staged.lightConstants[3] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.lightConstants[7] = vector(parameter("accessory_color"));
        staged.lightConstants[8] = vector(parameter("state"));
        staged.lightConstants[9] = vector(append(Value{},Value{},1u));
        staged.lightConstants[10] = vector(parameter("state_noise"));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[13] = vector(parameter("cutting_mask"));
        staged.lightConstants[14] = vector(parameter("rimlight_color"));
        staged.lightConstants[15] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.lightConstants[16] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[17] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.lightConstants[18] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[19] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("accessory_specular_intensity")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0]);
        staged.lightConstants[20] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],Value{}[0]);
        staged.lightConstants[21] = float4_t(parameter("opacity_clamp")[0],parameter("rimlight_power")[0],parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0]);
        staged.lightConstants[22] = float4_t(parameter("specular_intensity")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-168.v1")
    {
        [&]() {
        staged.program = 168u;
        staged.requiredExtraUVMask = 1u;
        staged.baseTextureMask = 15u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("var_base_haircolor_base_ui"));
        staged.baseConstants[4] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("accessory_color"));
        staged.baseConstants[8] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.baseConstants[9] = vector(parameter("hit_color"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("transcolor"));
        staged.baseConstants[12] = vector(parameter("cutting_mask"));
        staged.baseConstants[13] = vector(parameter("state"));
        staged.baseConstants[14] = vector(append(Value{},Value{},1u));
        staged.baseConstants[15] = vector(parameter("state_noise"));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[17] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[18] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.baseConstants[19] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[20] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.baseConstants[21] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[22] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("accessory_specular_intensity")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0]);
        staged.baseConstants[23] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[24] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0]);
        staged.lightTextureMask = 15u;
        staged.lightConstants[2] = vector(parameter("var_base_haircolor_base_ui"));
        staged.lightConstants[3] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(parameter("accessory_color"));
        staged.lightConstants[7] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.lightConstants[8] = vector(parameter("cutting_mask"));
        staged.lightConstants[9] = vector(parameter("rimlight_color"));
        staged.lightConstants[10] = vector(parameter("state"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.lightConstants[16] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[17] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.lightConstants[18] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[19] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("accessory_specular_intensity")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0]);
        staged.lightConstants[20] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[21] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0]);
        staged.lightConstants[22] = float4_t(parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0],parameter("specular_intensity")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-169.v1")
    {
        [&]() {
        staged.program = 169u;
        staged.requiredExtraUVMask = 1u;
        staged.baseTextureMask = 7u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("var_base_haircolor_base_ui"));
        staged.baseConstants[4] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("accessory_color"));
        staged.baseConstants[8] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.baseConstants[9] = vector(parameter("hit_color"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("transcolor"));
        staged.baseConstants[12] = vector(parameter("state"));
        staged.baseConstants[13] = vector(append(Value{},Value{},1u));
        staged.baseConstants[14] = vector(parameter("state_noise"));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[17] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.baseConstants[18] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[19] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.baseConstants[20] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[21] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("accessory_specular_intensity")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0]);
        staged.baseConstants[22] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[23] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0]);
        staged.lightTextureMask = 7u;
        staged.lightConstants[2] = vector(parameter("var_base_haircolor_base_ui"));
        staged.lightConstants[3] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(parameter("accessory_color"));
        staged.lightConstants[7] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.lightConstants[8] = vector(parameter("rimlight_color"));
        staged.lightConstants[9] = vector(parameter("state"));
        staged.lightConstants[10] = vector(append(Value{},Value{},1u));
        staged.lightConstants[11] = vector(parameter("state_noise"));
        staged.lightConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[14] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.lightConstants[15] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[16] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.lightConstants[17] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[18] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("accessory_specular_intensity")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0]);
        staged.lightConstants[19] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[20] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0]);
        staged.lightConstants[21] = float4_t(parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0],parameter("specular_intensity")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-170.v1")
    {
        [&]() {
        staged.program = 170u;
        staged.requiredExtraUVMask = 1u;
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
        staged.baseConstants[20] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],parameter("lighting_intensity_max")[0]);
        staged.baseConstants[21] = float4_t(parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[22] = float4_t(parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0]);
        staged.baseConstants[23] = float4_t(parameter("specular_power")[0],parameter("specular_intensity")[0],parameter("lighting_intensity")[0],0.f);
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
        staged.lightConstants[17] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],parameter("lighting_intensity_max")[0]);
        staged.lightConstants[18] = float4_t(parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0]);
        staged.lightConstants[19] = float4_t(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("customshade_shadowdiffuselighting")[0]);
        staged.lightConstants[20] = float4_t(parameter("specular_power")[0],parameter("specular_intensity")[0],parameter("lighting_intensity")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-171.v1")
    {
        [&]() {
        staged.program = 171u;
        staged.baseTextureMask = 15u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("var_base_haircolor_base_ui"));
        staged.baseConstants[4] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.baseConstants[8] = vector(parameter("hit_color"));
        staged.baseConstants[9] = vector(parameter("buffcolor"));
        staged.baseConstants[10] = vector(parameter("transcolor"));
        staged.baseConstants[11] = vector(parameter("cutting_mask"));
        staged.baseConstants[12] = vector(parameter("state"));
        staged.baseConstants[13] = vector(append(Value{},Value{},1u));
        staged.baseConstants[14] = vector(parameter("state_noise"));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[17] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("var_base_hairspecularintensity_ui")[0]);
        staged.baseConstants[18] = float4_t(parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0]);
        staged.baseConstants[19] = float4_t(parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[20] = float4_t(parameter("rimlight_power")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightTextureMask = 15u;
        staged.lightConstants[2] = vector(parameter("var_base_haircolor_base_ui"));
        staged.lightConstants[3] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.lightConstants[7] = vector(parameter("cutting_mask"));
        staged.lightConstants[8] = vector(parameter("rimlight_color"));
        staged.lightConstants[9] = vector(parameter("state"));
        staged.lightConstants[10] = vector(append(Value{},Value{},1u));
        staged.lightConstants[11] = vector(parameter("state_noise"));
        staged.lightConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[14] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("var_base_hairspecularintensity_ui")[0]);
        staged.lightConstants[15] = float4_t(parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0]);
        staged.lightConstants[16] = float4_t(parameter("rimlight_power")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[17] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0]);
        staged.lightConstants[18] = float4_t(parameter("specular_intensity")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-172.v1")
    {
        [&]() {
        staged.program = 172u;
        staged.requiredExtraUVMask = 1u;
        staged.baseTextureMask = 7u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("var_base_haircolor_base_ui"));
        staged.baseConstants[4] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("accessory_color"));
        staged.baseConstants[8] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.baseConstants[9] = vector(parameter("hit_color"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("transcolor"));
        staged.baseConstants[12] = vector(parameter("state"));
        staged.baseConstants[13] = vector(append(Value{},Value{},1u));
        staged.baseConstants[14] = vector(parameter("state_noise"));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[16] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[17] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.baseConstants[18] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[19] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.baseConstants[20] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[21] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("accessory_specular_intensity")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0]);
        staged.baseConstants[22] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[23] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0]);
        staged.lightTextureMask = 7u;
        staged.lightConstants[2] = vector(parameter("var_base_haircolor_base_ui"));
        staged.lightConstants[3] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(parameter("accessory_color"));
        staged.lightConstants[7] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.lightConstants[8] = vector(parameter("rimlight_color"));
        staged.lightConstants[9] = vector(parameter("state"));
        staged.lightConstants[10] = vector(append(Value{},Value{},1u));
        staged.lightConstants[11] = vector(parameter("state_noise"));
        staged.lightConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[14] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.lightConstants[15] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[16] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.lightConstants[17] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[18] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("accessory_specular_intensity")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0]);
        staged.lightConstants[19] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[20] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("rimlight_power")[0],Value{}[0]);
        staged.lightConstants[21] = float4_t(parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0],parameter("specular_intensity")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-173.v1")
    {
        [&]() {
        staged.program = 173u;
        staged.baseTextureMask = 255u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("diffusecolor"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[8] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[12] = vector(parameter("ibl_color_top"));
        staged.baseConstants[13] = vector(parameter("emissive_color"));
        staged.baseConstants[14] = vector(parameter("transcolor"));
        staged.baseConstants[15] = vector(parameter("buffcolor"));
        staged.baseConstants[16] = vector(parameter("hit_color"));
        staged.baseConstants[17] = vector(append(Value{},Value{},1u));
        staged.baseConstants[18] = vector(parameter("state_noise"));
        staged.baseConstants[19] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[20] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[21] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[22] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[23] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.baseConstants[24] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[25] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[26] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.baseConstants[27] = float4_t(parameter("fresnel_radius")[0],parameter("emissive_intensity")[0],parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0]);
        staged.baseConstants[28] = float4_t(Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightTextureMask = 479u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[10] = vector(parameter("ibl_color_top"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[16] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[17] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.lightConstants[18] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[19] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[20] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.lightConstants[21] = float4_t(parameter("fresnel_radius")[0],parameter("emissive_intensity")[0],parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0]);
        staged.lightConstants[22] = float4_t(Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[23] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightConstants[24] = float4_t(parameter("orennayar_brightness")[0],parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0]);
        staged.lightConstants[25] = float4_t(parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-174.v1")
    {
        [&]() {
        staged.program = 174u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[8] = vector(parameter("ibl_color_top"));
        staged.baseConstants[9] = vector(parameter("transcolor"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("hit_color"));
        staged.baseConstants[12] = vector(append(Value{},Value{},1u));
        staged.baseConstants[13] = vector(parameter("state_noise"));
        staged.baseConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[16] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[18] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.baseConstants[19] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[20] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[21] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.baseConstants[22] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[23] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("opacity_intensity")[0],parameter("shadowfactor")[0]);
        staged.lightTextureMask = 255u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[7] = vector(parameter("ibl_color_top"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("state_noise"));
        staged.lightConstants[10] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[12] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[14] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.lightConstants[15] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[16] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[17] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.lightConstants[18] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[19] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("opacity_intensity")[0],parameter("shadowfactor")[0]);
        staged.lightConstants[20] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0],parameter("specular_power_limit")[0]);
        staged.lightConstants[21] = float4_t(parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-175.v1")
    {
        [&]() {
        staged.program = 175u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("diffusecolor"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[8] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("var_base_skincolor_ui"));
        staged.baseConstants[12] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[13] = vector(parameter("ibl_color_top"));
        staged.baseConstants[14] = vector(parameter("emissive_color"));
        staged.baseConstants[15] = vector(parameter("transcolor"));
        staged.baseConstants[16] = vector(parameter("buffcolor"));
        staged.baseConstants[17] = vector(parameter("hit_color"));
        staged.baseConstants[18] = vector(append(Value{},Value{},1u));
        staged.baseConstants[19] = vector(parameter("state_noise"));
        staged.baseConstants[20] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[21] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[22] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.baseConstants[23] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[24] = float4_t(parameter("skin_roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[25] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.baseConstants[26] = float4_t(parameter("ibl_skinlodscale")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0]);
        staged.baseConstants[27] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.baseConstants[28] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[29] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightTextureMask = 223u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("var_base_skincolor_ui"));
        staged.lightConstants[10] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[11] = vector(parameter("ibl_color_top"));
        staged.lightConstants[12] = vector(append(Value{},Value{},1u));
        staged.lightConstants[13] = vector(parameter("state_noise"));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[16] = vector(parameter("half_lambert_skin"));
        staged.lightConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.lightConstants[18] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[19] = float4_t(parameter("skin_roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[20] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.lightConstants[21] = float4_t(parameter("ibl_skinlodscale")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0]);
        staged.lightConstants[22] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.lightConstants[23] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[24] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[25] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[26] = float4_t(parameter("specular_power_limit")[0],parameter("var_base_skinspecularpower_ui")[0],parameter("skin_specular_power")[0],parameter("skin_specular_power_min")[0]);
        staged.lightConstants[27] = float4_t(parameter("var_base_skinspecularintensity_ui")[0],parameter("skin_specular_intensity")[0],parameter("skin_specular_intensity_min")[0],parameter("beckmannspecular_constant_max")[0]);
        staged.lightConstants[28] = float4_t(parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-176.v1")
    {
        [&]() {
        staged.program = 176u;
        staged.baseTextureMask = 255u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("diffusecolor"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[8] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("var_base_skincolor_ui"));
        staged.baseConstants[12] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[13] = vector(parameter("ibl_color_top"));
        staged.baseConstants[14] = vector(parameter("emissive_color"));
        staged.baseConstants[15] = vector(parameter("transcolor"));
        staged.baseConstants[16] = vector(parameter("buffcolor"));
        staged.baseConstants[17] = vector(parameter("hit_color"));
        staged.baseConstants[18] = vector(append(Value{},Value{},1u));
        staged.baseConstants[19] = vector(parameter("state_noise"));
        staged.baseConstants[20] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[21] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[22] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[23] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.baseConstants[24] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[25] = float4_t(parameter("skin_roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[26] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.baseConstants[27] = float4_t(parameter("ibl_skinlodscale")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0]);
        staged.baseConstants[28] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.baseConstants[29] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[30] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightTextureMask = 479u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("var_base_skincolor_ui"));
        staged.lightConstants[10] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[11] = vector(parameter("ibl_color_top"));
        staged.lightConstants[12] = vector(append(Value{},Value{},1u));
        staged.lightConstants[13] = vector(parameter("state_noise"));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[15] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[16] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[17] = vector(parameter("half_lambert_skin"));
        staged.lightConstants[18] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.lightConstants[19] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[20] = float4_t(parameter("skin_roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[21] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.lightConstants[22] = float4_t(parameter("ibl_skinlodscale")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0]);
        staged.lightConstants[23] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.lightConstants[24] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[25] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[26] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[27] = float4_t(parameter("specular_power_limit")[0],parameter("var_base_skinspecularpower_ui")[0],parameter("skin_specular_power")[0],parameter("skin_specular_power_min")[0]);
        staged.lightConstants[28] = float4_t(parameter("var_base_skinspecularintensity_ui")[0],parameter("skin_specular_intensity")[0],parameter("skin_specular_intensity_min")[0],parameter("beckmannspecular_constant_max")[0]);
        staged.lightConstants[29] = float4_t(parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-177.v1")
    {
        [&]() {
        staged.program = 177u;
        staged.requiredExtraUVMask = 1u;
        staged.baseTextureMask = 255u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("diffusecolor"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[7] = vector(parameter("diffuse_pattern_a_color"));
        staged.baseConstants[8] = vector(parameter("pattern_bool"));
        staged.baseConstants[9] = vector(parameter("pattern_choice_a"));
        staged.baseConstants[10] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[11] = vector(parameter("diffuse_pattern_b_color"));
        staged.baseConstants[12] = vector(parameter("pattern_choice_b"));
        staged.baseConstants[13] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[14] = vector(parameter("diffuse_pattern_c_color"));
        staged.baseConstants[15] = vector(parameter("pattern_choice_c"));
        staged.baseConstants[16] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[17] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[18] = vector(parameter("var_base_skincolor_ui"));
        staged.baseConstants[19] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[20] = vector(parameter("ibl_color_top"));
        staged.baseConstants[21] = vector(parameter("emissive_color"));
        staged.baseConstants[22] = vector(parameter("transcolor"));
        staged.baseConstants[23] = vector(parameter("buffcolor"));
        staged.baseConstants[24] = vector(parameter("hit_color"));
        staged.baseConstants[25] = vector(append(Value{},Value{},1u));
        staged.baseConstants[26] = vector(parameter("state_noise"));
        staged.baseConstants[27] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[28] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[29] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.baseConstants[30] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[31] = float4_t(parameter("skin_roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[32] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.baseConstants[33] = float4_t(parameter("ibl_skinlodscale")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0]);
        staged.baseConstants[34] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.baseConstants[35] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[36] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightTextureMask = 447u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffuse_pattern_a_color"));
        staged.lightConstants[6] = vector(parameter("pattern_bool"));
        staged.lightConstants[7] = vector(parameter("pattern_choice_a"));
        staged.lightConstants[8] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[9] = vector(parameter("diffuse_pattern_b_color"));
        staged.lightConstants[10] = vector(parameter("pattern_choice_b"));
        staged.lightConstants[11] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[12] = vector(parameter("diffuse_pattern_c_color"));
        staged.lightConstants[13] = vector(parameter("pattern_choice_c"));
        staged.lightConstants[14] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[15] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[16] = vector(parameter("var_base_skincolor_ui"));
        staged.lightConstants[17] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[18] = vector(parameter("ibl_color_top"));
        staged.lightConstants[19] = vector(append(Value{},Value{},1u));
        staged.lightConstants[20] = vector(parameter("state_noise"));
        staged.lightConstants[21] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[22] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[23] = vector(parameter("half_lambert_skin"));
        staged.lightConstants[24] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.lightConstants[25] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[26] = float4_t(parameter("skin_roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[27] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.lightConstants[28] = float4_t(parameter("ibl_skinlodscale")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0]);
        staged.lightConstants[29] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.lightConstants[30] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[31] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[32] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[33] = float4_t(parameter("specular_power_limit")[0],parameter("var_base_skinspecularpower_ui")[0],parameter("skin_specular_power")[0],parameter("skin_specular_power_min")[0]);
        staged.lightConstants[34] = float4_t(parameter("var_base_skinspecularintensity_ui")[0],parameter("skin_specular_intensity")[0],parameter("skin_specular_intensity_min")[0],parameter("beckmannspecular_constant_max")[0]);
        staged.lightConstants[35] = float4_t(parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-178.v1")
    {
        [&]() {
        staged.program = 178u;
        staged.requiredExtraUVMask = 1u;
        staged.baseTextureMask = 255u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffuse_pattern_a_color"));
        staged.baseConstants[7] = vector(parameter("pattern_bool"));
        staged.baseConstants[8] = vector(parameter("pattern_choice_a"));
        staged.baseConstants[9] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[10] = vector(parameter("diffuse_pattern_b_color"));
        staged.baseConstants[11] = vector(parameter("pattern_choice_b"));
        staged.baseConstants[12] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[13] = vector(parameter("diffuse_pattern_c_color"));
        staged.baseConstants[14] = vector(parameter("pattern_choice_c"));
        staged.baseConstants[15] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[16] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[17] = vector(parameter("var_base_skincolor_ui"));
        staged.baseConstants[18] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[19] = vector(parameter("ibl_color_top"));
        staged.baseConstants[20] = vector(parameter("transcolor"));
        staged.baseConstants[21] = vector(parameter("buffcolor"));
        staged.baseConstants[22] = vector(parameter("hit_color"));
        staged.baseConstants[23] = vector(append(Value{},Value{},1u));
        staged.baseConstants[24] = vector(parameter("state_noise"));
        staged.baseConstants[25] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[26] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[27] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[28] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.baseConstants[29] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[30] = float4_t(parameter("skin_roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[31] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.baseConstants[32] = float4_t(parameter("ibl_skinlodscale")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0]);
        staged.baseConstants[33] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.baseConstants[34] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightTextureMask = 511u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffuse_pattern_a_color"));
        staged.lightConstants[6] = vector(parameter("pattern_bool"));
        staged.lightConstants[7] = vector(parameter("pattern_choice_a"));
        staged.lightConstants[8] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[9] = vector(parameter("diffuse_pattern_b_color"));
        staged.lightConstants[10] = vector(parameter("pattern_choice_b"));
        staged.lightConstants[11] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[12] = vector(parameter("diffuse_pattern_c_color"));
        staged.lightConstants[13] = vector(parameter("pattern_choice_c"));
        staged.lightConstants[14] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[15] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[16] = vector(parameter("var_base_skincolor_ui"));
        staged.lightConstants[17] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[18] = vector(parameter("ibl_color_top"));
        staged.lightConstants[19] = vector(append(Value{},Value{},1u));
        staged.lightConstants[20] = vector(parameter("state_noise"));
        staged.lightConstants[21] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[22] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[23] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[24] = vector(parameter("half_lambert_skin"));
        staged.lightConstants[25] = float4_t(parameter("normaltex_intensity")[0],parameter("var_base_skinnormalintensity_ui")[0],parameter("skin_normal_intensity")[0],multiply(parameter("skin_normal_intensity"),parameter("var_base_skinnormalintensity_ui"))[0]);
        staged.lightConstants[26] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[27] = float4_t(parameter("skin_roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[28] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.lightConstants[29] = float4_t(parameter("ibl_skinlodscale")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0]);
        staged.lightConstants[30] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("skin_metalicness_power")[0]);
        staged.lightConstants[31] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightConstants[32] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[33] = float4_t(parameter("specular_power_limit")[0],parameter("var_base_skinspecularpower_ui")[0],parameter("skin_specular_power")[0],parameter("skin_specular_power_min")[0]);
        staged.lightConstants[34] = float4_t(parameter("var_base_skinspecularintensity_ui")[0],parameter("skin_specular_intensity")[0],parameter("skin_specular_intensity_min")[0],parameter("beckmannspecular_constant_max")[0]);
        staged.lightConstants[35] = float4_t(parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-179.v1")
    {
        [&]() {
        staged.program = 179u;
        staged.requiredExtraUVMask = 1u;
        staged.baseTextureMask = 7u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("var_base_haircolor_base_ui"));
        staged.baseConstants[4] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[7] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.baseConstants[8] = vector(parameter("hit_color"));
        staged.baseConstants[9] = vector(parameter("accessory_color"));
        staged.baseConstants[10] = vector(parameter("state"));
        staged.baseConstants[11] = vector(append(Value{},Value{},1u));
        staged.baseConstants[12] = vector(parameter("state_noise"));
        staged.baseConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[15] = vector(parameter("buffcolor"));
        staged.baseConstants[16] = vector(parameter("transcolor"));
        staged.baseConstants[17] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.baseConstants[18] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[19] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.baseConstants[20] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[21] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("accessory_specular_intensity")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0]);
        staged.baseConstants[22] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],Value{}[0]);
        staged.baseConstants[23] = float4_t(parameter("ambientlight")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[24] = float4_t(parameter("opacity_clamp")[0],parameter("rimlight_power")[0],parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0]);
        staged.lightTextureMask = 7u;
        staged.lightConstants[2] = vector(parameter("var_base_haircolor_base_ui"));
        staged.lightConstants[3] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.lightConstants[7] = vector(parameter("accessory_color"));
        staged.lightConstants[8] = vector(parameter("state"));
        staged.lightConstants[9] = vector(append(Value{},Value{},1u));
        staged.lightConstants[10] = vector(parameter("state_noise"));
        staged.lightConstants[11] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[13] = vector(parameter("rimlight_color"));
        staged.lightConstants[14] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.lightConstants[15] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[16] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.lightConstants[17] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[18] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("accessory_specular_intensity")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0]);
        staged.lightConstants[19] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],Value{}[0]);
        staged.lightConstants[20] = float4_t(parameter("opacity_clamp")[0],parameter("rimlight_power")[0],parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0]);
        staged.lightConstants[21] = float4_t(parameter("specular_intensity")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-180.v1")
    {
        [&]() {
        staged.program = 180u;
        staged.baseTextureMask = 31u;
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
        staged.baseConstants[16] = vector(parameter("ssstintcolor"));
        staged.baseConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("dyeingthreshold_low")[0],parameter("dyeingthreshold_high")[0]);
        staged.baseConstants[18] = float4_t(parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[19] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0]);
        staged.baseConstants[20] = float4_t(Value{}[0],parameter("auto_pbr_oc_max")[0],parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.baseConstants[21] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.baseConstants[22] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[23] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_d")[0],multiply(parameter("roughness_power_d"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[24] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 31u;
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
        staged.lightConstants[13] = vector(parameter("ssstintcolor"));
        staged.lightConstants[14] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("dyeingthreshold_low")[0],parameter("dyeingthreshold_high")[0]);
        staged.lightConstants[15] = float4_t(parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[16] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0]);
        staged.lightConstants[17] = float4_t(Value{}[0],parameter("auto_pbr_oc_max")[0],parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.lightConstants[18] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.lightConstants[19] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[20] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_d")[0],multiply(parameter("roughness_power_d"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[21] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-181.v1")
    {
        [&]() {
        staged.program = 181u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_d"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[8] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[9] = vector(parameter("state"));
        staged.baseConstants[10] = vector(parameter("emissive_color"));
        staged.baseConstants[11] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[12] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[13] = vector(parameter("transcolor"));
        staged.baseConstants[14] = vector(parameter("buffcolor"));
        staged.baseConstants[15] = vector(parameter("hit_color"));
        staged.baseConstants[16] = vector(parameter("occlusion_color"));
        staged.baseConstants[17] = vector(append(Value{},Value{},1u));
        staged.baseConstants[18] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[19] = vector(parameter("ssstintcolor"));
        staged.baseConstants[20] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("dyeingthreshold_low")[0],parameter("dyeingthreshold_high")[0]);
        staged.baseConstants[21] = float4_t(parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0],parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0]);
        staged.baseConstants[22] = float4_t(Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[23] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("occlusion_power")[0]);
        staged.baseConstants[24] = float4_t(parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0],parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.baseConstants[25] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.baseConstants[26] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[27] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_d")[0],multiply(parameter("roughness_power_d"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[28] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 111u;
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
        staged.lightConstants[16] = float4_t(Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[17] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("occlusion_power")[0]);
        staged.lightConstants[18] = float4_t(parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0],parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.lightConstants[19] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.lightConstants[20] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[21] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_d")[0],multiply(parameter("roughness_power_d"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[22] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-182.v1")
    {
        [&]() {
        staged.program = 182u;
        staged.requiredExtraUVMask = 1u;
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
        staged.baseConstants[13] = float4_t(parameter("rimlight_intensity")[0],parameter("rimlight_power")[0],parameter("customshade_brightness")[0],parameter("customshade_power")[0]);
        staged.baseConstants[14] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.baseConstants[15] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[16] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.baseConstants[17] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[18] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0]);
        staged.baseConstants[19] = float4_t(parameter("subspecular_power")[0],parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[20] = float4_t(parameter("trans_rim_hard")[0],Value{}[0],parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0]);
        staged.lightTextureMask = 7u;
        staged.lightConstants[1] = vector(parameter("var_base_haircolor_base_ui"));
        staged.lightConstants[2] = vector(parameter("var_base_hairtwotonecolor_ui"));
        staged.lightConstants[3] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[5] = vector(append(Value{0.f,0.f,0.f,0.f},parameter("specular_offset"),2u));
        staged.lightConstants[6] = vector(parameter("rimlight_color"));
        staged.lightConstants[7] = vector(parameter("state"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = float4_t(parameter("rimlight_intensity")[0],parameter("rimlight_power")[0],parameter("customshade_brightness")[0],parameter("customshade_power")[0]);
        staged.lightConstants[10] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("var_base_hairtwotonerangeedge_ui")[0],parameter("var_base_hairtwotonerangehardness_ui")[0],multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui"))[0]);
        staged.lightConstants[11] = float4_t(bounded(multiply(parameter("var_base_hairtwotonerangehardness_ui"),parameter("var_base_hairtwotonerangehardness_ui")),Value{0.100000001f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("hairtwotone_color_switch")[0],bounded(parameter("var_base_hairtwotonerangehardness_ui"),Value{0.100000001f,0.f,0.f,0.f},Value{0.5f,0.f,0.f,0.f})[0],bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[12] = float4_t(multiply(Value{0.5f,0.f,0.f,0.f},bounded(parameter("var_base_hairtwotonerangeedge_ui"),Value{0.200000003f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f}))[0],parameter("var_base_hairtwotonerangeb_ui")[0],multiply(parameter("var_base_hairtwotonerangeb_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerangea_ui")[0]);
        staged.lightConstants[13] = float4_t(multiply(parameter("var_base_hairtwotonerangea_ui"),Value{0.5f,0.f,0.f,0.f})[0],parameter("var_base_hairtwotonerange_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[14] = float4_t(parameter("var_base_hairspecularintensity_ui")[0],parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0]);
        staged.lightConstants[15] = float4_t(parameter("trans_rim_hard")[0],Value{}[0],parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0]);
        staged.lightConstants[16] = float4_t(parameter("specular_intensity")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-183.v1")
    {
        [&]() {
        staged.program = 183u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[7] = vector(parameter("state"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("transcolor"));
        staged.baseConstants[11] = vector(parameter("buffcolor"));
        staged.baseConstants[12] = vector(parameter("hit_color"));
        staged.baseConstants[13] = vector(parameter("occlusion_color"));
        staged.baseConstants[14] = vector(append(Value{},Value{},1u));
        staged.baseConstants[15] = vector(parameter("ssstintcolor"));
        staged.baseConstants[16] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[17] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[18] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],Value{}[0],parameter("auto_pbr_oc_max")[0]);
        staged.baseConstants[19] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[20] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.baseConstants[21] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[22] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 31u;
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
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[14] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[15] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],Value{}[0],parameter("auto_pbr_oc_max")[0]);
        staged.lightConstants[16] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[17] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.lightConstants[18] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[19] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-184.v1")
    {
        [&]() {
        staged.program = 184u;
        staged.requiredExtraUVMask = 1u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_d"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[8] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[9] = vector(parameter("state"));
        staged.baseConstants[10] = vector(parameter("emissive_color"));
        staged.baseConstants[11] = vector(parameter("fx_panningspeed_tiling"));
        staged.baseConstants[12] = vector(parameter("fx_panning_color"));
        staged.baseConstants[13] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[14] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[15] = vector(parameter("transcolor"));
        staged.baseConstants[16] = vector(parameter("buffcolor"));
        staged.baseConstants[17] = vector(parameter("hit_color"));
        staged.baseConstants[18] = vector(parameter("occlusion_color"));
        staged.baseConstants[19] = vector(append(Value{},Value{},1u));
        staged.baseConstants[20] = vector(parameter("ssstintcolor"));
        staged.baseConstants[21] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("dyeingthreshold_low")[0],parameter("dyeingthreshold_high")[0]);
        staged.baseConstants[22] = float4_t(parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0],parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0]);
        staged.baseConstants[23] = float4_t(Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0],parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0]);
        staged.baseConstants[24] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[25] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0]);
        staged.baseConstants[26] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[27] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.baseConstants[28] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[29] = float4_t(parameter("roughness_power_d")[0],multiply(parameter("roughness_power_d"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f);
        staged.lightTextureMask = 79u;
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
        staged.lightConstants[13] = vector(parameter("ssstintcolor"));
        staged.lightConstants[14] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("dyeingthreshold_low")[0],parameter("dyeingthreshold_high")[0]);
        staged.lightConstants[15] = float4_t(Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0],parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0]);
        staged.lightConstants[16] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[17] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0]);
        staged.lightConstants[18] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[19] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.lightConstants[20] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[21] = float4_t(parameter("roughness_power_d")[0],multiply(parameter("roughness_power_d"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-185.v1")
    {
        [&]() {
        staged.program = 185u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_d"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("state"));
        staged.baseConstants[9] = vector(parameter("emissive_color"));
        staged.baseConstants[10] = vector(parameter("fx_panningspeed_tiling"));
        staged.baseConstants[11] = vector(parameter("fx_panning_color"));
        staged.baseConstants[12] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[13] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[14] = vector(parameter("transcolor"));
        staged.baseConstants[15] = vector(parameter("buffcolor"));
        staged.baseConstants[16] = vector(parameter("hit_color"));
        staged.baseConstants[17] = vector(parameter("occlusion_color"));
        staged.baseConstants[18] = vector(append(Value{},Value{},1u));
        staged.baseConstants[19] = vector(parameter("ssstintcolor"));
        staged.baseConstants[20] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("dyeingthreshold_low")[0],parameter("dyeingthreshold_high")[0]);
        staged.baseConstants[21] = float4_t(parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0],Value{}[0],parameter("fx_distortion_intensity")[0]);
        staged.baseConstants[22] = float4_t(parameter("fx_panning_intensity")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[23] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0]);
        staged.baseConstants[24] = float4_t(parameter("auto_pbr_oc_max")[0],parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[25] = float4_t(parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[26] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[27] = float4_t(multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_d")[0],multiply(parameter("roughness_power_d"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0]);
        staged.lightTextureMask = 79u;
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
        staged.lightConstants[13] = vector(parameter("ssstintcolor"));
        staged.lightConstants[14] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("dyeingthreshold_low")[0],parameter("dyeingthreshold_high")[0]);
        staged.lightConstants[15] = float4_t(parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0],Value{}[0],parameter("fx_distortion_intensity")[0]);
        staged.lightConstants[16] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0]);
        staged.lightConstants[17] = float4_t(parameter("auto_pbr_oc_max")[0],parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[18] = float4_t(parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[19] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[20] = float4_t(multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_d")[0],multiply(parameter("roughness_power_d"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-186.v1")
    {
        [&]() {
        staged.program = 186u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[7] = vector(parameter("state"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("transcolor"));
        staged.baseConstants[11] = vector(parameter("buffcolor"));
        staged.baseConstants[12] = vector(parameter("hit_color"));
        staged.baseConstants[13] = vector(parameter("occlusion_color"));
        staged.baseConstants[14] = vector(append(Value{},Value{},1u));
        staged.baseConstants[15] = vector(parameter("ssstintcolor"));
        staged.baseConstants[16] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[17] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[18] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],Value{}[0],parameter("auto_pbr_oc_max")[0]);
        staged.baseConstants[19] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[20] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.baseConstants[21] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[22] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 31u;
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
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[14] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[15] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],Value{}[0],parameter("auto_pbr_oc_max")[0]);
        staged.lightConstants[16] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[17] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.lightConstants[18] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[19] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-187.v1")
    {
        [&]() {
        staged.program = 187u;
        staged.requiredExtraUVMask = 2u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("diffusecolor"));
        staged.baseConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[6] = vector(parameter("transcolor"));
        staged.baseConstants[7] = vector(parameter("buffcolor"));
        staged.baseConstants[8] = vector(parameter("hit_color"));
        staged.baseConstants[9] = vector(parameter("state"));
        staged.baseConstants[10] = vector(append(Value{},Value{},1u));
        staged.baseConstants[11] = vector(parameter("state_noise"));
        staged.baseConstants[12] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[14] = float4_t(parameter("normaltex_intensity")[0],parameter("roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.baseConstants[15] = float4_t(parameter("ibl_normal_smooth")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[16] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.baseConstants[17] = float4_t(parameter("opacity_intensity")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 53u;
        staged.lightConstants[2] = vector(parameter("selectioncolor"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[6] = vector(parameter("state"));
        staged.lightConstants[7] = vector(append(Value{},Value{},1u));
        staged.lightConstants[8] = vector(parameter("state_noise"));
        staged.lightConstants[9] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[10] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[11] = float4_t(parameter("normaltex_intensity")[0],parameter("roughness_power")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0]);
        staged.lightConstants[12] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],Value{}[0]);
        staged.lightConstants[13] = float4_t(parameter("opacity_intensity")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-188.v1")
    {
        [&]() {
        staged.program = 188u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_d"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[8] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[9] = vector(parameter("state"));
        staged.baseConstants[10] = vector(parameter("emissive_color"));
        staged.baseConstants[11] = vector(parameter("fx_panningspeed_tiling"));
        staged.baseConstants[12] = vector(parameter("fx_panning_color"));
        staged.baseConstants[13] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[14] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[15] = vector(parameter("transcolor"));
        staged.baseConstants[16] = vector(parameter("buffcolor"));
        staged.baseConstants[17] = vector(parameter("hit_color"));
        staged.baseConstants[18] = vector(parameter("occlusion_color"));
        staged.baseConstants[19] = vector(append(Value{},Value{},1u));
        staged.baseConstants[20] = vector(parameter("ssstintcolor"));
        staged.baseConstants[21] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("dyeingthreshold_low")[0],parameter("dyeingthreshold_high")[0]);
        staged.baseConstants[22] = float4_t(parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0],parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0]);
        staged.baseConstants[23] = float4_t(Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0],parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0]);
        staged.baseConstants[24] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[25] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0]);
        staged.baseConstants[26] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[27] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.baseConstants[28] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[29] = float4_t(parameter("roughness_power_d")[0],multiply(parameter("roughness_power_d"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f);
        staged.lightTextureMask = 79u;
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
        staged.lightConstants[13] = vector(parameter("ssstintcolor"));
        staged.lightConstants[14] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("dyeingthreshold_low")[0],parameter("dyeingthreshold_high")[0]);
        staged.lightConstants[15] = float4_t(Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0],parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0]);
        staged.lightConstants[16] = float4_t(parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[17] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0]);
        staged.lightConstants[18] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[19] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.lightConstants[20] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[21] = float4_t(parameter("roughness_power_d")[0],multiply(parameter("roughness_power_d"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-189.v1")
    {
        [&]() {
        staged.program = 189u;
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
        staged.baseConstants[21] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0]);
        staged.baseConstants[22] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[23] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.baseConstants[24] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[25] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
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
        staged.lightConstants[16] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0]);
        staged.lightConstants[17] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[18] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.lightConstants[19] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[20] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-190.v1")
    {
        [&]() {
        staged.program = 190u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
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
        staged.baseConstants[16] = vector(parameter("ssstintcolor"));
        staged.baseConstants[17] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[18] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[19] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[20] = float4_t(Value{}[0],parameter("pbr_specular")[0],parameter("opacity_intensity")[0],parameter("dye_roughness_power_tensition")[0]);
        staged.baseConstants[21] = float4_t(parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0]);
        staged.baseConstants[22] = float4_t(multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("auto_pbr_oc_max")[0]);
        staged.baseConstants[23] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("ssslocalthickness")[0],0.f,0.f);
        staged.lightTextureMask = 31u;
        staged.lightConstants[3] = vector(parameter("selectioncolor"));
        staged.lightConstants[4] = vector(parameter("basecolor_color"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[7] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[8] = vector(parameter("state"));
        staged.lightConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[11] = vector(parameter("occlusion_color"));
        staged.lightConstants[12] = vector(append(Value{},Value{},1u));
        staged.lightConstants[13] = vector(parameter("ssstintcolor"));
        staged.lightConstants[14] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[15] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[16] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[17] = float4_t(Value{}[0],parameter("pbr_specular")[0],parameter("opacity_intensity")[0],parameter("dye_roughness_power_tensition")[0]);
        staged.lightConstants[18] = float4_t(parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0]);
        staged.lightConstants[19] = float4_t(multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("auto_pbr_oc_max")[0]);
        staged.lightConstants[20] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("ssslocalthickness")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-191.v1")
    {
        [&]() {
        staged.program = 191u;
        staged.requiredExtraUVMask = 1u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffuse_pattern_a_color"));
        staged.baseConstants[7] = vector(parameter("pattern_bool"));
        staged.baseConstants[8] = vector(parameter("pattern_choice_a"));
        staged.baseConstants[9] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[10] = vector(parameter("diffuse_pattern_b_color"));
        staged.baseConstants[11] = vector(parameter("pattern_choice_b"));
        staged.baseConstants[12] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[13] = vector(parameter("diffuse_pattern_c_color"));
        staged.baseConstants[14] = vector(parameter("pattern_choice_c"));
        staged.baseConstants[15] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[16] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[17] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[18] = vector(parameter("ibl_color_top"));
        staged.baseConstants[19] = vector(parameter("transcolor"));
        staged.baseConstants[20] = vector(parameter("buffcolor"));
        staged.baseConstants[21] = vector(parameter("hit_color"));
        staged.baseConstants[22] = vector(append(Value{},Value{},1u));
        staged.baseConstants[23] = vector(parameter("state_noise"));
        staged.baseConstants[24] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[25] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[26] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[27] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.baseConstants[28] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[29] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[30] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.baseConstants[31] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightTextureMask = 255u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffuse_pattern_a_color"));
        staged.lightConstants[6] = vector(parameter("pattern_bool"));
        staged.lightConstants[7] = vector(parameter("pattern_choice_a"));
        staged.lightConstants[8] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[9] = vector(parameter("diffuse_pattern_b_color"));
        staged.lightConstants[10] = vector(parameter("pattern_choice_b"));
        staged.lightConstants[11] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[12] = vector(parameter("diffuse_pattern_c_color"));
        staged.lightConstants[13] = vector(parameter("pattern_choice_c"));
        staged.lightConstants[14] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[15] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[16] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[17] = vector(parameter("ibl_color_top"));
        staged.lightConstants[18] = vector(append(Value{},Value{},1u));
        staged.lightConstants[19] = vector(parameter("state_noise"));
        staged.lightConstants[20] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[21] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[22] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[23] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.lightConstants[24] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[25] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[26] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.lightConstants[27] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[28] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightConstants[29] = float4_t(parameter("orennayar_brightness")[0],parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0]);
        staged.lightConstants[30] = float4_t(parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-192.v1")
    {
        [&]() {
        staged.program = 192u;
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
        staged.baseConstants[16] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("var_base_hairspecularintensity_ui")[0]);
        staged.baseConstants[17] = float4_t(parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0]);
        staged.baseConstants[18] = float4_t(parameter("subspecular_intensity")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[19] = float4_t(parameter("rimlight_power")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
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
        staged.lightConstants[13] = float4_t(parameter("var_base_hairtwotone_bool_ui")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("var_base_hairspecularintensity_ui")[0]);
        staged.lightConstants[14] = float4_t(parameter("specular_offset")[0],parameter("var_base_hairspecularpower_ui")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("var_base_hairspecularpower_ui"))[0],parameter("subspecular_power")[0]);
        staged.lightConstants[15] = float4_t(parameter("rimlight_power")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[16] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("customshade_shadowdiffuselighting")[0],parameter("specular_power")[0]);
        staged.lightConstants[17] = float4_t(parameter("specular_intensity")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-193.v1")
    {
        [&]() {
        staged.program = 193u;
        staged.baseTextureMask = 255u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("diffusecolor"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[8] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[12] = vector(parameter("ibl_color_top"));
        staged.baseConstants[13] = vector(parameter("emissive_color"));
        staged.baseConstants[14] = vector(parameter("transcolor"));
        staged.baseConstants[15] = vector(parameter("buffcolor"));
        staged.baseConstants[16] = vector(parameter("hit_color"));
        staged.baseConstants[17] = vector(append(Value{},Value{},1u));
        staged.baseConstants[18] = vector(parameter("state_noise"));
        staged.baseConstants[19] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[20] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[21] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[22] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[23] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("roughness_power_a")[0]);
        staged.baseConstants[24] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[25] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[26] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[27] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightTextureMask = 479u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[10] = vector(parameter("ibl_color_top"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[16] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[17] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("roughness_power_a")[0]);
        staged.lightConstants[18] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[19] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[20] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[21] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[22] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[23] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-194.v1")
    {
        [&]() {
        staged.program = 194u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("diffusecolor"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[8] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[12] = vector(parameter("ibl_color_top"));
        staged.baseConstants[13] = vector(parameter("emissive_color"));
        staged.baseConstants[14] = vector(parameter("transcolor"));
        staged.baseConstants[15] = vector(parameter("buffcolor"));
        staged.baseConstants[16] = vector(parameter("hit_color"));
        staged.baseConstants[17] = vector(append(Value{},Value{},1u));
        staged.baseConstants[18] = vector(parameter("state_noise"));
        staged.baseConstants[19] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[20] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[21] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[22] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("roughness_power_a")[0]);
        staged.baseConstants[23] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.baseConstants[24] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[25] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[26] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightTextureMask = 223u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[10] = vector(parameter("ibl_color_top"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[16] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("roughness_power_a")[0]);
        staged.lightConstants[17] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("ibl_exposer")[0],parameter("ibl_intensity")[0]);
        staged.lightConstants[18] = float4_t(parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[19] = float4_t(parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0],parameter("fresnel_radius")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[20] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[21] = float4_t(parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0],parameter("orennayar_brightness")[0],parameter("orennayar")[0]);
        staged.lightConstants[22] = float4_t(parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0],parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-195.v1")
    {
        [&]() {
        staged.program = 195u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[7] = vector(parameter("state"));
        staged.baseConstants[8] = vector(parameter("emissive_color"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("transcolor"));
        staged.baseConstants[12] = vector(parameter("buffcolor"));
        staged.baseConstants[13] = vector(parameter("hit_color"));
        staged.baseConstants[14] = vector(parameter("occlusion_color"));
        staged.baseConstants[15] = vector(append(Value{},Value{},1u));
        staged.baseConstants[16] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[17] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[18] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[19] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[20] = float4_t(parameter("pbr_specular")[0],parameter("opacity_intensity")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.baseConstants[21] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[22] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],0.f,0.f);
        staged.lightTextureMask = 47u;
        staged.lightConstants[1] = vector(parameter("selectioncolor"));
        staged.lightConstants[2] = vector(parameter("basecolor_color"));
        staged.lightConstants[3] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[6] = vector(parameter("state"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("occlusion_color"));
        staged.lightConstants[10] = vector(append(Value{},Value{},1u));
        staged.lightConstants[11] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[12] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[13] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[14] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[15] = float4_t(parameter("pbr_specular")[0],parameter("opacity_intensity")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.lightConstants[16] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[17] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-196.v1")
    {
        [&]() {
        staged.program = 196u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("state"));
        staged.baseConstants[4] = vector(parameter("diffusecolor"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[10] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[11] = vector(parameter("ibl_color_top"));
        staged.baseConstants[12] = vector(parameter("transcolor"));
        staged.baseConstants[13] = vector(parameter("buffcolor"));
        staged.baseConstants[14] = vector(parameter("hit_color"));
        staged.baseConstants[15] = vector(append(Value{},Value{},1u));
        staged.baseConstants[16] = vector(parameter("state_noise"));
        staged.baseConstants[17] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[18] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[19] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[20] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.baseConstants[21] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[22] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[23] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.baseConstants[24] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightTextureMask = 127u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[10] = vector(parameter("ibl_color_top"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[16] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.lightConstants[17] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[18] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[19] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.lightConstants[20] = float4_t(parameter("fresnel_radius")[0],Value{}[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[21] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightConstants[22] = float4_t(parameter("orennayar_brightness")[0],parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0]);
        staged.lightConstants[23] = float4_t(parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-197.v1")
    {
        [&]() {
        staged.program = 197u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("diffusecolor"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[8] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("ibl_color_bottom"));
        staged.baseConstants[12] = vector(parameter("ibl_color_top"));
        staged.baseConstants[13] = vector(parameter("emissive_color"));
        staged.baseConstants[14] = vector(parameter("transcolor"));
        staged.baseConstants[15] = vector(parameter("buffcolor"));
        staged.baseConstants[16] = vector(parameter("hit_color"));
        staged.baseConstants[17] = vector(append(Value{},Value{},1u));
        staged.baseConstants[18] = vector(parameter("state_noise"));
        staged.baseConstants[19] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[20] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[21] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.baseConstants[22] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.baseConstants[23] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[24] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[25] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.baseConstants[26] = float4_t(parameter("fresnel_radius")[0],parameter("emissive_intensity")[0],parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0]);
        staged.baseConstants[27] = float4_t(Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightTextureMask = 223u;
        staged.lightConstants[2] = vector(parameter("state"));
        staged.lightConstants[3] = vector(parameter("diffusecolor"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("ibl_color_bottom"));
        staged.lightConstants[10] = vector(parameter("ibl_color_top"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("state_noise"));
        staged.lightConstants[13] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[14] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[15] = float4_t(parameter("normaltex_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("ibl_normal_smooth")[0]);
        staged.lightConstants[16] = float4_t(parameter("ibl_reflect_lodbias_a")[0],parameter("ibl_reflect_lodbias")[0],parameter("ibl_reflect_lodbias_b")[0],parameter("ibl_reflect_lodbias_c")[0]);
        staged.lightConstants[17] = float4_t(parameter("roughness_power_a")[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[18] = float4_t(parameter("ibl_exposer")[0],parameter("ibl_intensity")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0]);
        staged.lightConstants[19] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("metalicness_power")[0],parameter("fresnel_rimlightintensity")[0]);
        staged.lightConstants[20] = float4_t(parameter("fresnel_radius")[0],parameter("emissive_intensity")[0],parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0]);
        staged.lightConstants[21] = float4_t(Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[22] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("shadowfactor")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("shadowfactor"))[0]);
        staged.lightConstants[23] = float4_t(parameter("orennayar_brightness")[0],parameter("orennayar")[0],parameter("specular_power_limit")[0],parameter("beckmannspecular_constant_max")[0]);
        staged.lightConstants[24] = float4_t(parameter("pbr_specular_power")[0],parameter("pbr_specular_intensity")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-198.v1")
    {
        [&]() {
        staged.program = 198u;
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
        staged.baseConstants[22] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0]);
        staged.baseConstants[23] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[24] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.baseConstants[25] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[26] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
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
        staged.lightConstants[17] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0]);
        staged.lightConstants[18] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[19] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.lightConstants[20] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[21] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-199.v1")
    {
        [&]() {
        staged.program = 199u;
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
        staged.baseConstants[21] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0]);
        staged.baseConstants[22] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[23] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.baseConstants[24] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[25] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
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
        staged.lightConstants[16] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("auto_pbr_oc_max")[0]);
        staged.lightConstants[17] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[18] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.lightConstants[19] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[20] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.equipment-native-200.v1")
    {
        [&]() {
        staged.program = 200u;
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
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-color-mask.v1")
    {
        [&]() {
        staged.program = 208u;
        staged.baseTextureMask = 1u;
        staged.baseConstants[0] = vector(parameter("selectioncolor"));
        staged.baseConstants[1] = vector(parameter("diffuse_color"));
        staged.baseConstants[2] = float4_t(parameter("base_brightness")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 1u;
        staged.lightConstants[0] = vector(parameter("diffuse_color"));
        staged.lightConstants[1] = float4_t(parameter("base_brightness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-real-pbr-masked.v1")
    {
        [&]() {
        staged.program = 210u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 31u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("state"));
        staged.baseConstants[5] = vector(parameter("rimlightcolor"));
        staged.baseConstants[6] = vector(append(wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false),wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[9] = vector(parameter("transcolor"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("constantoutline_color"));
        staged.baseConstants[12] = vector(parameter("hit_color"));
        staged.baseConstants[13] = vector(parameter("occlusion_color"));
        staged.baseConstants[14] = vector(append(Value{},Value{},1u));
        staged.baseConstants[15] = vector(parameter("ssstintcolor"));
        staged.baseConstants[16] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[17] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[18] = float4_t(parameter("trans_rim_hard")[0],parameter("rimlight_intensity")[0],parameter("rimlight_radius")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("rimlight_radius"))[0]);
        staged.baseConstants[19] = float4_t(parameter("lightdirection_rimlight_intensity")[0],parameter("lightdirection_rimlight_radius")[0],parameter("rimlight_turn")[0],multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f})[0]);
        staged.baseConstants[20] = float4_t(wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0]);
        staged.baseConstants[21] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0]);
        staged.baseConstants[22] = float4_t(multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.baseConstants[23] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("roughness_power")[0],parameter("ssslocalthickness")[0]);
        staged.lightTextureMask = 23u;
        staged.lightConstants[2] = vector(parameter("selectioncolor"));
        staged.lightConstants[3] = vector(parameter("basecolor_color"));
        staged.lightConstants[4] = vector(parameter("state"));
        staged.lightConstants[5] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[6] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[7] = vector(parameter("occlusion_color"));
        staged.lightConstants[8] = vector(append(Value{},Value{},1u));
        staged.lightConstants[9] = vector(parameter("ssstintcolor"));
        staged.lightConstants[10] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[11] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("1.use_dyeing_sp")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[12] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0]);
        staged.lightConstants[13] = float4_t(multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.lightConstants[14] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("roughness_power")[0],parameter("ssslocalthickness")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.selection-native-211.v1")
    {
        [&]() {
        staged.program = 211u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("var_hairdecocc_colora_ui"));
        staged.baseConstants[5] = vector(parameter("var_hairdecocc_colorb_ui"));
        staged.baseConstants[6] = vector(parameter("state"));
        staged.baseConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[9] = vector(parameter("transcolor"));
        staged.baseConstants[10] = vector(parameter("buffcolor"));
        staged.baseConstants[11] = vector(parameter("hit_color"));
        staged.baseConstants[12] = vector(parameter("occlusion_color"));
        staged.baseConstants[13] = vector(append(Value{},Value{},1u));
        staged.baseConstants[14] = vector(parameter("ssstintcolor"));
        staged.baseConstants[15] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[16] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[17] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],Value{}[0],parameter("auto_pbr_oc_max")[0]);
        staged.baseConstants[18] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[19] = float4_t(parameter("roughness_power")[0],parameter("ssslocalthickness")[0],0.f,0.f);
        staged.lightTextureMask = 31u;
        staged.lightConstants[2] = vector(parameter("selectioncolor"));
        staged.lightConstants[3] = vector(parameter("basecolor_color"));
        staged.lightConstants[4] = vector(parameter("var_hairdecocc_colora_ui"));
        staged.lightConstants[5] = vector(parameter("var_hairdecocc_colorb_ui"));
        staged.lightConstants[6] = vector(parameter("state"));
        staged.lightConstants[7] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[9] = vector(parameter("occlusion_color"));
        staged.lightConstants[10] = vector(append(Value{},Value{},1u));
        staged.lightConstants[11] = vector(parameter("ssstintcolor"));
        staged.lightConstants[12] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[13] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[14] = float4_t(parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],Value{}[0],parameter("auto_pbr_oc_max")[0]);
        staged.lightConstants[15] = float4_t(parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[16] = float4_t(parameter("roughness_power")[0],parameter("ssslocalthickness")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.selection-native-212.v1")
    {
        [&]() {
        staged.program = 212u;
        staged.baseTextureMask = 1u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("diffusecolor"));
        staged.baseConstants[3] = float4_t(parameter("opacity")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 1u;
        staged.lightConstants[1] = vector(parameter("selectioncolor"));
        staged.lightConstants[2] = vector(parameter("diffusecolor"));
        staged.lightConstants[3] = float4_t(parameter("opacity")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.selection-native-213.v1")
    {
        [&]() {
        staged.program = 213u;
        staged.requiredExtraUVMask = 1u;
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
        }();
    }
    if (staged.program == 0u && family == "source.character.selection-native-235.v1")
    {
        [&]() {
        staged.program = 235u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 255u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("basecolor_color"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[7] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[8] = vector(parameter("state"));
        staged.baseConstants[9] = vector(parameter("rimlightcolor"));
        staged.baseConstants[10] = vector(append(wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false),wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[11] = vector(parameter("emissive_color"));
        staged.baseConstants[12] = vector(parameter("fx_panningspeed_tiling"));
        staged.baseConstants[13] = vector(parameter("fx_panning_color"));
        staged.baseConstants[14] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[15] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[16] = vector(parameter("transcolor"));
        staged.baseConstants[17] = vector(parameter("buffcolor"));
        staged.baseConstants[18] = vector(parameter("constantoutline_color"));
        staged.baseConstants[19] = vector(parameter("hit_color"));
        staged.baseConstants[20] = vector(parameter("occlusion_color"));
        staged.baseConstants[21] = vector(append(Value{},Value{},1u));
        staged.baseConstants[22] = vector(parameter("ssstintcolor"));
        staged.baseConstants[23] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_intensity")[0]);
        staged.baseConstants[24] = float4_t(parameter("rimlight_hardness")[0],parameter("rimlight_turn")[0],multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f})[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[25] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[26] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.baseConstants[27] = float4_t(parameter("fx_distortion_intensity")[0],parameter("fx_panning_intensity")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[28] = float4_t(parameter("trans_rim_hard")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0]);
        staged.baseConstants[29] = float4_t(multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.baseConstants[30] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0]);
        staged.baseConstants[31] = float4_t(parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.baseConstants[32] = float4_t(parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.baseConstants[33] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[34] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f);
        staged.lightTextureMask = 207u;
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
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_intensity")[0]);
        staged.lightConstants[14] = float4_t(parameter("emissive_intensitymin")[0],parameter("emissive_flicker_speed")[0],Value{}[0],parameter("1.use_emissive_flickerspeed_fixed")[0]);
        staged.lightConstants[15] = float4_t(parameter("trans_rim_hard")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("constantoutline")[0]);
        staged.lightConstants[16] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0]);
        staged.lightConstants[17] = float4_t(parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0]);
        staged.lightConstants[18] = float4_t(parameter("dead_texture_tiling")[0],parameter("dead")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0]);
        staged.lightConstants[19] = float4_t(multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[20] = float4_t(parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.selection-native-236.v1")
    {
        [&]() {
        staged.program = 236u;
        staged.baseConstants[63] = vector(parameter("constantoutline_blink"));
        staged.lightConstants[63] = vector(parameter("constantoutline_blink"));
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[7] = vector(parameter("state"));
        staged.baseConstants[8] = vector(parameter("rimlightcolor"));
        staged.baseConstants[9] = vector(append(wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false),wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[11] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[12] = vector(parameter("transcolor"));
        staged.baseConstants[13] = vector(parameter("buffcolor"));
        staged.baseConstants[14] = vector(parameter("constantoutline_color"));
        staged.baseConstants[15] = vector(parameter("hit_color"));
        staged.baseConstants[16] = vector(parameter("occlusion_color"));
        staged.baseConstants[17] = vector(append(Value{},Value{},1u));
        staged.baseConstants[18] = vector(parameter("ssstintcolor"));
        staged.baseConstants[19] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_intensity")[0]);
        staged.baseConstants[20] = float4_t(parameter("rimlight_hardness")[0],parameter("rimlight_turn")[0],multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f})[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[21] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.baseConstants[22] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.baseConstants[23] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0]);
        staged.baseConstants[24] = float4_t(multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.baseConstants[25] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0]);
        staged.baseConstants[26] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.baseConstants[27] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[28] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 63u;
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
        staged.lightConstants[13] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("rimlight_intensity")[0]);
        staged.lightConstants[14] = float4_t(multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),false))[0],wave(multiply(parameter("rimlight_turn"),Value{0.25f,0.f,0.f,0.f}),true)[0],parameter("rimlight_area")[0],parameter("fx_color_desaturation_actiontool")[0]);
        staged.lightConstants[15] = float4_t(parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0]);
        staged.lightConstants[16] = float4_t(parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0],parameter("constantoutline_blink")[0]);
        staged.lightConstants[17] = float4_t(multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],parameter("occlusionbasecolor_blend_intensity")[0]);
        staged.lightConstants[18] = float4_t(bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0],parameter("pbr_specular")[0],parameter("dead_texture_tiling")[0],parameter("dead")[0]);
        staged.lightConstants[19] = float4_t(parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power")[0]);
        staged.lightConstants[20] = float4_t(parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0],multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[21] = float4_t(parameter("ssslocalthickness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-214.v1")
    {
        [&]() {
        staged.program = 214u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[4] = vector(parameter("hit_color"));
        staged.baseConstants[5] = vector(parameter("diffuse_color"));
        staged.baseConstants[6] = vector(parameter("overlay_color"));
        staged.baseConstants[7] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[8] = float4_t(parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_wind_intensity")[0]);
        staged.baseConstants[9] = float4_t(parameter("overlay_amount")[0],add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("overlay_tiling")[0]);
        staged.baseConstants[10] = float4_t(parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0]);
        staged.baseConstants[11] = float4_t(parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("nonmetallic_brightness")[0],parameter("metallic_brightness")[0]);
        staged.baseConstants[12] = float4_t(parameter("metallic_intensity")[0],parameter("metallic_power")[0],parameter("specular_pbr_intensity")[0],parameter("roughness_intensity")[0]);
        staged.baseConstants[13] = float4_t(bounded(parameter("roughness_intensity"),Value{0.f,0.f,0.f,0.f},Value{100.f,0.f,0.f,0.f})[0],parameter("roughness_power")[0],parameter("ao_intensity")[0],parameter("ao_power")[0]);
        staged.lightTextureMask = 31u;
        staged.lightConstants[2] = vector(parameter("selectioncolor"));
        staged.lightConstants[3] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.lightConstants[4] = vector(parameter("diffuse_color"));
        staged.lightConstants[5] = vector(parameter("overlay_color"));
        staged.lightConstants[6] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.lightConstants[7] = float4_t(parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_wind_intensity")[0]);
        staged.lightConstants[8] = float4_t(parameter("overlay_amount")[0],add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("overlay_tiling")[0]);
        staged.lightConstants[9] = float4_t(parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0]);
        staged.lightConstants[10] = float4_t(parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("nonmetallic_brightness")[0],parameter("metallic_brightness")[0]);
        staged.lightConstants[11] = float4_t(parameter("metallic_intensity")[0],parameter("metallic_power")[0],parameter("specular_pbr_intensity")[0],parameter("roughness_intensity")[0]);
        staged.lightConstants[12] = float4_t(bounded(parameter("roughness_intensity"),Value{0.f,0.f,0.f,0.f},Value{100.f,0.f,0.f,0.f})[0],parameter("roughness_power")[0],parameter("ao_intensity")[0],parameter("ao_power")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-215.v1")
    {
        [&]() {
        staged.program = 215u;
        staged.baseTextureMask = 15u;
        staged.baseConstants[0] = vector(parameter("selectioncolor"));
        staged.baseConstants[1] = vector(parameter("diffuse_color"));
        staged.baseConstants[2] = vector(parameter("specular_color"));
        staged.baseConstants[3] = vector(parameter("transmission_color"));
        staged.baseConstants[4] = float4_t(parameter("normal_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[5] = float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("transmission_intensity")[0],0.f);
        staged.lightTextureMask = 15u;
        staged.lightConstants[0] = vector(parameter("diffuse_color"));
        staged.lightConstants[1] = vector(parameter("specular_color"));
        staged.lightConstants[2] = vector(parameter("transmission_color"));
        staged.lightConstants[3] = float4_t(parameter("normal_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0],parameter("diffuse_brightness")[0]);
        staged.lightConstants[4] = float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("transmission_intensity")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-216.v1")
    {
        [&]() {
        staged.program = 216u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("hit_color"));
        staged.baseConstants[5] = vector(parameter("reflection_color"));
        staged.baseConstants[6] = vector(parameter("diffuse_color"));
        staged.baseConstants[7] = vector(parameter("overlay_color"));
        staged.baseConstants[8] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_tiling")[0]);
        staged.baseConstants[9] = float4_t(parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_wind_intensity")[0],parameter("overlay_amount")[0]);
        staged.baseConstants[10] = float4_t(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0]);
        staged.baseConstants[11] = float4_t(parameter("reflection_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[12] = float4_t(parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("nonmetallic_brightness")[0]);
        staged.baseConstants[13] = float4_t(parameter("metallic_brightness")[0],parameter("metallic_intensity")[0],parameter("metallic_power")[0],parameter("specular_pbr_intensity")[0]);
        staged.baseConstants[14] = float4_t(parameter("roughness_intensity")[0],bounded(parameter("roughness_intensity"),Value{0.f,0.f,0.f,0.f},Value{100.f,0.f,0.f,0.f})[0],parameter("roughness_power")[0],parameter("ao_intensity")[0]);
        staged.baseConstants[15] = float4_t(parameter("ao_power")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 127u;
        staged.lightConstants[3] = vector(parameter("selectioncolor"));
        staged.lightConstants[4] = vector(parameter("reflection_color"));
        staged.lightConstants[5] = vector(parameter("diffuse_color"));
        staged.lightConstants[6] = vector(parameter("overlay_color"));
        staged.lightConstants[7] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_tiling")[0]);
        staged.lightConstants[8] = float4_t(parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_wind_intensity")[0],parameter("overlay_amount")[0]);
        staged.lightConstants[9] = float4_t(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0]);
        staged.lightConstants[10] = float4_t(parameter("reflection_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0],parameter("diffuse_brightness")[0]);
        staged.lightConstants[11] = float4_t(parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("nonmetallic_brightness")[0]);
        staged.lightConstants[12] = float4_t(parameter("metallic_brightness")[0],parameter("metallic_intensity")[0],parameter("metallic_power")[0],parameter("specular_pbr_intensity")[0]);
        staged.lightConstants[13] = float4_t(parameter("roughness_intensity")[0],bounded(parameter("roughness_intensity"),Value{0.f,0.f,0.f,0.f},Value{100.f,0.f,0.f,0.f})[0],parameter("roughness_power")[0],parameter("ao_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-217.v1")
    {
        [&]() {
        staged.program = 217u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("hit_color"));
        staged.baseConstants[5] = vector(parameter("reflection_color"));
        staged.baseConstants[6] = vector(parameter("diffuse_color"));
        staged.baseConstants[7] = vector(parameter("overlay_color"));
        staged.baseConstants[8] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_tiling")[0]);
        staged.baseConstants[9] = float4_t(parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_wind_intensity")[0],parameter("overlay_amount")[0]);
        staged.baseConstants[10] = float4_t(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0]);
        staged.baseConstants[11] = float4_t(parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0]);
        staged.baseConstants[12] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("nonmetallic_brightness")[0],parameter("metallic_brightness")[0],parameter("metallic_intensity")[0]);
        staged.baseConstants[13] = float4_t(parameter("metallic_power")[0],parameter("specular_pbr_intensity")[0],parameter("roughness_intensity")[0],bounded(parameter("roughness_intensity"),Value{0.f,0.f,0.f,0.f},Value{100.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[14] = float4_t(parameter("roughness_power")[0],parameter("ao_intensity")[0],parameter("ao_power")[0],0.f);
        staged.lightTextureMask = 127u;
        staged.lightConstants[3] = vector(parameter("selectioncolor"));
        staged.lightConstants[4] = vector(parameter("reflection_color"));
        staged.lightConstants[5] = vector(parameter("diffuse_color"));
        staged.lightConstants[6] = vector(parameter("overlay_color"));
        staged.lightConstants[7] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_tiling")[0]);
        staged.lightConstants[8] = float4_t(parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_wind_intensity")[0],parameter("overlay_amount")[0]);
        staged.lightConstants[9] = float4_t(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0]);
        staged.lightConstants[10] = float4_t(parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0]);
        staged.lightConstants[11] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("nonmetallic_brightness")[0],parameter("metallic_brightness")[0],parameter("metallic_intensity")[0]);
        staged.lightConstants[12] = float4_t(parameter("metallic_power")[0],parameter("specular_pbr_intensity")[0],parameter("roughness_intensity")[0],bounded(parameter("roughness_intensity"),Value{0.f,0.f,0.f,0.f},Value{100.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[13] = float4_t(parameter("roughness_power")[0],parameter("ao_intensity")[0],parameter("ao_power")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-218.v1")
    {
        [&]() {
        staged.program = 218u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("reflection_color"));
        staged.baseConstants[3] = vector(parameter("diffuse_color"));
        staged.baseConstants[4] = vector(parameter("overlay_color"));
        staged.baseConstants[5] = vector(parameter("specular_color"));
        staged.baseConstants[6] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_tiling")[0]);
        staged.baseConstants[7] = float4_t(parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0]);
        staged.baseConstants[8] = float4_t(parameter("reflection_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[9] = float4_t(parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("specular_intensity")[0]);
        staged.baseConstants[10] = float4_t(parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],0.f,0.f);
        staged.lightTextureMask = 127u;
        staged.lightConstants[1] = vector(parameter("reflection_color"));
        staged.lightConstants[2] = vector(parameter("diffuse_color"));
        staged.lightConstants[3] = vector(parameter("overlay_color"));
        staged.lightConstants[4] = vector(parameter("specular_color"));
        staged.lightConstants[5] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_tiling")[0]);
        staged.lightConstants[6] = float4_t(parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0]);
        staged.lightConstants[7] = float4_t(parameter("reflection_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0],parameter("diffuse_brightness")[0]);
        staged.lightConstants[8] = float4_t(parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("specular_intensity")[0]);
        staged.lightConstants[9] = float4_t(parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-219.v1")
    {
        [&]() {
        staged.program = 219u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[3] = vector(parameter("emissive_uv_tiling"));
        staged.baseConstants[4] = vector(parameter("emissive_color"));
        staged.baseConstants[5] = vector(parameter("reflection_color"));
        staged.baseConstants[6] = vector(parameter("diffuse_color"));
        staged.baseConstants[7] = vector(parameter("overlay_color"));
        staged.baseConstants[8] = vector(parameter("specular_color"));
        staged.baseConstants[9] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[10] = float4_t(parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_tiling")[0]);
        staged.baseConstants[11] = float4_t(parameter("emissive_intensity")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0]);
        staged.baseConstants[12] = float4_t(parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0]);
        staged.baseConstants[13] = float4_t(parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0]);
        staged.baseConstants[14] = float4_t(parameter("specular_power")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 111u;
        staged.lightConstants[1] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.lightConstants[2] = vector(parameter("reflection_color"));
        staged.lightConstants[3] = vector(parameter("diffuse_color"));
        staged.lightConstants[4] = vector(parameter("overlay_color"));
        staged.lightConstants[5] = vector(parameter("specular_color"));
        staged.lightConstants[6] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.lightConstants[7] = float4_t(parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_tiling")[0]);
        staged.lightConstants[8] = float4_t(parameter("emissive_intensity")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0]);
        staged.lightConstants[9] = float4_t(parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0]);
        staged.lightConstants[10] = float4_t(parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0]);
        staged.lightConstants[11] = float4_t(parameter("specular_power")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-220.v1")
    {
        [&]() {
        staged.program = 220u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("overlay_direction"));
        staged.baseConstants[3] = vector(parameter("hit_color"));
        staged.baseConstants[4] = vector(parameter("diffuse_color"));
        staged.baseConstants[5] = vector(parameter("overlay_color"));
        staged.baseConstants[6] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.baseConstants[7] = float4_t(parameter("overlay_amount")[0],add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("overlay_tiling")[0]);
        staged.baseConstants[8] = float4_t(parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0]);
        staged.baseConstants[9] = float4_t(parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("nonmetallic_brightness")[0],parameter("metallic_brightness")[0]);
        staged.baseConstants[10] = float4_t(parameter("metallic_intensity")[0],parameter("metallic_power")[0],parameter("specular_pbr_intensity")[0],parameter("roughness_intensity")[0]);
        staged.baseConstants[11] = float4_t(bounded(parameter("roughness_intensity"),Value{0.f,0.f,0.f,0.f},Value{100.f,0.f,0.f,0.f})[0],parameter("roughness_power")[0],parameter("ao_intensity")[0],parameter("ao_power")[0]);
        staged.lightTextureMask = 31u;
        staged.lightConstants[1] = vector(parameter("selectioncolor"));
        staged.lightConstants[2] = vector(parameter("overlay_direction"));
        staged.lightConstants[3] = vector(parameter("diffuse_color"));
        staged.lightConstants[4] = vector(parameter("overlay_color"));
        staged.lightConstants[5] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.lightConstants[6] = float4_t(parameter("overlay_amount")[0],add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("overlay_tiling")[0]);
        staged.lightConstants[7] = float4_t(parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0]);
        staged.lightConstants[8] = float4_t(parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("nonmetallic_brightness")[0],parameter("metallic_brightness")[0]);
        staged.lightConstants[9] = float4_t(parameter("metallic_intensity")[0],parameter("metallic_power")[0],parameter("specular_pbr_intensity")[0],parameter("roughness_intensity")[0]);
        staged.lightConstants[10] = float4_t(bounded(parameter("roughness_intensity"),Value{0.f,0.f,0.f,0.f},Value{100.f,0.f,0.f,0.f})[0],parameter("roughness_power")[0],parameter("ao_intensity")[0],parameter("ao_power")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-221.v1")
    {
        [&]() {
        staged.program = 221u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("hit_color"));
        staged.baseConstants[4] = vector(parameter("diffuse_color"));
        staged.baseConstants[5] = vector(parameter("overlay_color"));
        staged.baseConstants[6] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.baseConstants[7] = float4_t(parameter("overlay_wind_intensity")[0],parameter("overlay_amount")[0],add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[8] = float4_t(parameter("overlay_tiling")[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0]);
        staged.baseConstants[9] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("nonmetallic_brightness")[0],parameter("metallic_brightness")[0],parameter("metallic_intensity")[0]);
        staged.baseConstants[10] = float4_t(parameter("metallic_power")[0],parameter("specular_pbr_intensity")[0],parameter("roughness_intensity")[0],bounded(parameter("roughness_intensity"),Value{0.f,0.f,0.f,0.f},Value{100.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[11] = float4_t(parameter("roughness_power")[0],parameter("ao_intensity")[0],parameter("ao_power")[0],0.f);
        staged.lightTextureMask = 31u;
        staged.lightConstants[2] = vector(parameter("selectioncolor"));
        staged.lightConstants[3] = vector(parameter("diffuse_color"));
        staged.lightConstants[4] = vector(parameter("overlay_color"));
        staged.lightConstants[5] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.lightConstants[6] = float4_t(parameter("overlay_wind_intensity")[0],parameter("overlay_amount")[0],add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[7] = float4_t(parameter("overlay_tiling")[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0]);
        staged.lightConstants[8] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("nonmetallic_brightness")[0],parameter("metallic_brightness")[0],parameter("metallic_intensity")[0]);
        staged.lightConstants[9] = float4_t(parameter("metallic_power")[0],parameter("specular_pbr_intensity")[0],parameter("roughness_intensity")[0],bounded(parameter("roughness_intensity"),Value{0.f,0.f,0.f,0.f},Value{100.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[10] = float4_t(parameter("roughness_power")[0],parameter("ao_intensity")[0],parameter("ao_power")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-222.v1")
    {
        [&]() {
        staged.program = 222u;
        staged.baseTextureMask = 7u;
        staged.baseConstants[0] = vector(parameter("selectioncolor"));
        staged.baseConstants[1] = vector(parameter("diffuse_color"));
        staged.baseConstants[2] = vector(parameter("transmission_color"));
        staged.baseConstants[3] = float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("transmission_intensity")[0],0.f);
        staged.lightTextureMask = 7u;
        staged.lightConstants[0] = vector(parameter("diffuse_color"));
        staged.lightConstants[1] = vector(parameter("transmission_color"));
        staged.lightConstants[2] = float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("transmission_intensity")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-223.v1")
    {
        [&]() {
        staged.program = 223u;
        staged.baseConstants[62] = vector(parameter("source_primitive_bounds"));
        staged.baseConstants[63] = vector(parameter("cloud_speed"));
        staged.lightConstants[63] = vector(parameter("cloud_speed"));
        staged.baseTextureMask = 15u;
        staged.baseConstants[3] = vector(parameter("selectioncolor"));
        staged.baseConstants[4] = vector(parameter("diffuse_color"));
        staged.baseConstants[5] = vector(parameter("bottomcloud_panning"));
        staged.baseConstants[6] = vector(parameter("bottomcloud_color"));
        staged.baseConstants[7] = vector(parameter("light_direction"));
        staged.baseConstants[8] = vector(append(Value{1.f,0.f,0.f,0.f},parameter("cloud_height"),1u));
        staged.baseConstants[9] = vector(append(periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.00800000038f,0.f,0.f,0.f})),periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.f,0.f,0.f,0.f})),1u));
        staged.baseConstants[10] = vector(append(periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{-0.00400000019f,0.f,0.f,0.f})),periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.f,0.f,0.f,0.f})),1u));
        staged.baseConstants[11] = vector(parameter("cloud_rimlight_color"));
        staged.baseConstants[12] = vector(parameter("cloud_color"));
        staged.baseConstants[13] = vector(append(periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.0199999996f,0.f,0.f,0.f})),periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.f,0.f,0.f,0.f})),1u));
        staged.baseConstants[14] = float4_t(parameter("bottomcloud_tiling")[0],parameter("bottomcloud_height")[0],Value{}[0],parameter("bottomcloud_range")[0]);
        staged.baseConstants[15] = float4_t(parameter("cloud_rimlight_power")[0],parameter("cloud_speed")[0],multiply(parameter("cloud_speed"),Value{})[0],multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.00800000038f,0.f,0.f,0.f})[0]);
        staged.baseConstants[16] = float4_t(parameter("cloud_opacity")[0],parameter("base_brightness")[0],0.f,0.f);
        staged.lightTextureMask = 15u;
        staged.lightConstants[3] = vector(parameter("diffuse_color"));
        staged.lightConstants[4] = vector(parameter("bottomcloud_panning"));
        staged.lightConstants[5] = vector(parameter("bottomcloud_color"));
        staged.lightConstants[6] = vector(parameter("light_direction"));
        staged.lightConstants[7] = vector(append(Value{1.f,0.f,0.f,0.f},parameter("cloud_height"),1u));
        staged.lightConstants[8] = vector(append(periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.00800000038f,0.f,0.f,0.f})),periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.f,0.f,0.f,0.f})),1u));
        staged.lightConstants[9] = vector(append(periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{-0.00400000019f,0.f,0.f,0.f})),periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.f,0.f,0.f,0.f})),1u));
        staged.lightConstants[10] = vector(parameter("cloud_rimlight_color"));
        staged.lightConstants[11] = vector(parameter("cloud_color"));
        staged.lightConstants[12] = vector(append(periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.0199999996f,0.f,0.f,0.f})),periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.f,0.f,0.f,0.f})),1u));
        staged.lightConstants[13] = float4_t(parameter("bottomcloud_tiling")[0],parameter("bottomcloud_height")[0],Value{}[0],parameter("bottomcloud_range")[0]);
        staged.lightConstants[14] = float4_t(parameter("cloud_rimlight_power")[0],parameter("cloud_speed")[0],multiply(parameter("cloud_speed"),Value{})[0],multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.00800000038f,0.f,0.f,0.f})[0]);
        staged.lightConstants[15] = float4_t(parameter("cloud_opacity")[0],parameter("base_brightness")[0],0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-224.v1")
    {
        [&]() {
        staged.program = 224u;
        staged.baseTextureMask = 15u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("emissive_uv_tiling"));
        staged.baseConstants[3] = vector(parameter("emissive_color"));
        staged.baseConstants[4] = vector(parameter("diffuse_color"));
        staged.baseConstants[5] = float4_t(parameter("normal_intensity")[0],parameter("emissive_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0]);
        staged.baseConstants[6] = float4_t(parameter("diffuse_brightness")[0],parameter("opacity_intensity")[0],parameter("opacity_falloff")[0],0.f);
        staged.lightTextureMask = 13u;
        staged.lightConstants[1] = vector(parameter("diffuse_color"));
        staged.lightConstants[2] = float4_t(parameter("normal_intensity")[0],parameter("emissive_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0]);
        staged.lightConstants[3] = float4_t(parameter("diffuse_brightness")[0],parameter("opacity_intensity")[0],parameter("opacity_falloff")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-225.v1")
    {
        [&]() {
        staged.program = 225u;
        staged.baseTextureMask = 7u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[3] = vector(parameter("emissive_uv_tiling"));
        staged.baseConstants[4] = vector(parameter("emissive_color"));
        staged.baseConstants[5] = vector(parameter("diffuse_color"));
        staged.baseConstants[6] = vector(parameter("specular_color"));
        staged.baseConstants[7] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[8] = float4_t(parameter("emissive_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.baseConstants[9] = float4_t(parameter("opacity_intensity")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 5u;
        staged.lightConstants[1] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.lightConstants[2] = vector(parameter("diffuse_color"));
        staged.lightConstants[3] = vector(parameter("specular_color"));
        staged.lightConstants[4] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.lightConstants[5] = float4_t(parameter("emissive_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.lightConstants[6] = float4_t(parameter("opacity_intensity")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-226.v1")
    {
        [&]() {
        staged.program = 226u;
        staged.baseTextureMask = 7u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("emissive_uv_tiling"));
        staged.baseConstants[3] = vector(parameter("emissive_color"));
        staged.baseConstants[4] = vector(parameter("diffuse_color"));
        staged.baseConstants[5] = float4_t(parameter("normal_intensity")[0],parameter("emissive_intensity")[0],parameter("diffuse_brightness")[0],parameter("opacity_intensity")[0]);
        staged.lightTextureMask = 5u;
        staged.lightConstants[1] = vector(parameter("diffuse_color"));
        staged.lightConstants[2] = float4_t(parameter("normal_intensity")[0],parameter("emissive_intensity")[0],parameter("diffuse_brightness")[0],parameter("opacity_intensity")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-227.v1")
    {
        [&]() {
        staged.program = 227u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[3] = vector(parameter("overlay_direction"));
        staged.baseConstants[4] = vector(parameter("reflection_color"));
        staged.baseConstants[5] = vector(parameter("diffuse_color"));
        staged.baseConstants[6] = vector(parameter("overlay_color"));
        staged.baseConstants[7] = vector(parameter("specular_color"));
        staged.baseConstants[8] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[9] = float4_t(parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_amount")[0]);
        staged.baseConstants[10] = float4_t(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("overlay_tiling")[0],parameter("reflection_tiling")[0]);
        staged.baseConstants[11] = float4_t(parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0]);
        staged.baseConstants[12] = float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.baseConstants[13] = float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],0.f);
        staged.lightTextureMask = 63u;
        staged.lightConstants[1] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.lightConstants[2] = vector(parameter("overlay_direction"));
        staged.lightConstants[3] = vector(parameter("reflection_color"));
        staged.lightConstants[4] = vector(parameter("diffuse_color"));
        staged.lightConstants[5] = vector(parameter("overlay_color"));
        staged.lightConstants[6] = vector(parameter("specular_color"));
        staged.lightConstants[7] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.lightConstants[8] = float4_t(parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_amount")[0]);
        staged.lightConstants[9] = float4_t(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("overlay_tiling")[0],parameter("reflection_tiling")[0]);
        staged.lightConstants[10] = float4_t(parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0]);
        staged.lightConstants[11] = float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.lightConstants[12] = float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-228.v1")
    {
        [&]() {
        staged.program = 228u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("overlay_direction"));
        staged.baseConstants[3] = vector(parameter("reflection_color"));
        staged.baseConstants[4] = vector(parameter("diffuse_color"));
        staged.baseConstants[5] = vector(parameter("overlay_color"));
        staged.baseConstants[6] = vector(parameter("specular_color"));
        staged.baseConstants[7] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_tiling")[0]);
        staged.baseConstants[8] = float4_t(parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_amount")[0],add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0]);
        staged.baseConstants[9] = float4_t(multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0]);
        staged.baseConstants[10] = float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.baseConstants[11] = float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],0.f);
        staged.lightTextureMask = 127u;
        staged.lightConstants[1] = vector(parameter("overlay_direction"));
        staged.lightConstants[2] = vector(parameter("reflection_color"));
        staged.lightConstants[3] = vector(parameter("diffuse_color"));
        staged.lightConstants[4] = vector(parameter("overlay_color"));
        staged.lightConstants[5] = vector(parameter("specular_color"));
        staged.lightConstants[6] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_tiling")[0]);
        staged.lightConstants[7] = float4_t(parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_amount")[0],add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0]);
        staged.lightConstants[8] = float4_t(multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0]);
        staged.lightConstants[9] = float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.lightConstants[10] = float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-229.v1")
    {
        [&]() {
        staged.program = 229u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("reflection_color"));
        staged.baseConstants[4] = vector(parameter("diffuse_color"));
        staged.baseConstants[5] = vector(parameter("overlay_color"));
        staged.baseConstants[6] = vector(parameter("specular_color"));
        staged.baseConstants[7] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.baseConstants[8] = float4_t(parameter("overlay_wind_intensity")[0],parameter("overlay_amount")[0],add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[9] = float4_t(parameter("overlay_tiling")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0]);
        staged.baseConstants[10] = float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.baseConstants[11] = float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],0.f);
        staged.lightTextureMask = 63u;
        staged.lightConstants[2] = vector(parameter("reflection_color"));
        staged.lightConstants[3] = vector(parameter("diffuse_color"));
        staged.lightConstants[4] = vector(parameter("overlay_color"));
        staged.lightConstants[5] = vector(parameter("specular_color"));
        staged.lightConstants[6] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.lightConstants[7] = float4_t(parameter("overlay_wind_intensity")[0],parameter("overlay_amount")[0],add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[8] = float4_t(parameter("overlay_tiling")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0]);
        staged.lightConstants[9] = float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.lightConstants[10] = float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-230.v1")
    {
        [&]() {
        staged.program = 230u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[3] = vector(parameter("overlay_direction"));
        staged.baseConstants[4] = vector(parameter("reflection_color"));
        staged.baseConstants[5] = vector(parameter("diffuse_color"));
        staged.baseConstants[6] = vector(parameter("overlay_color"));
        staged.baseConstants[7] = vector(parameter("specular_color"));
        staged.baseConstants[8] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[9] = float4_t(parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_amount")[0]);
        staged.baseConstants[10] = float4_t(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("overlay_tiling")[0],parameter("reflection_tiling")[0]);
        staged.baseConstants[11] = float4_t(parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0]);
        staged.baseConstants[12] = float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.baseConstants[13] = float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],0.f);
        staged.lightTextureMask = 31u;
        staged.lightConstants[1] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.lightConstants[2] = vector(parameter("overlay_direction"));
        staged.lightConstants[3] = vector(parameter("reflection_color"));
        staged.lightConstants[4] = vector(parameter("diffuse_color"));
        staged.lightConstants[5] = vector(parameter("overlay_color"));
        staged.lightConstants[6] = vector(parameter("specular_color"));
        staged.lightConstants[7] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.lightConstants[8] = float4_t(parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_amount")[0]);
        staged.lightConstants[9] = float4_t(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("overlay_tiling")[0],parameter("reflection_tiling")[0]);
        staged.lightConstants[10] = float4_t(parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0]);
        staged.lightConstants[11] = float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.lightConstants[12] = float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-231.v1")
    {
        [&]() {
        staged.program = 231u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[3] = vector(parameter("overlay_direction"));
        staged.baseConstants[4] = vector(parameter("reflection_color"));
        staged.baseConstants[5] = vector(parameter("diffuse_color"));
        staged.baseConstants[6] = vector(parameter("overlay_color"));
        staged.baseConstants[7] = vector(parameter("specular_color"));
        staged.baseConstants[8] = float4_t(parameter("uv_rotate")[0],multiply(parameter("uv_rotate"),Value{2.00000048f,0.f,0.f,0.f})[0],wave(multiply(parameter("uv_rotate"),Value{2.00000048f,0.f,0.f,0.f}),false)[0],wave(multiply(parameter("uv_rotate"),Value{2.00000048f,0.f,0.f,0.f}),true)[0]);
        staged.baseConstants[9] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[10] = float4_t(parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_amount")[0]);
        staged.baseConstants[11] = float4_t(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("overlay_tiling")[0],parameter("reflection_tiling")[0]);
        staged.baseConstants[12] = float4_t(parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0]);
        staged.baseConstants[13] = float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.baseConstants[14] = float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],0.f);
        staged.lightTextureMask = 63u;
        staged.lightConstants[1] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.lightConstants[2] = vector(parameter("overlay_direction"));
        staged.lightConstants[3] = vector(parameter("reflection_color"));
        staged.lightConstants[4] = vector(parameter("diffuse_color"));
        staged.lightConstants[5] = vector(parameter("overlay_color"));
        staged.lightConstants[6] = vector(parameter("specular_color"));
        staged.lightConstants[7] = float4_t(parameter("uv_rotate")[0],multiply(parameter("uv_rotate"),Value{2.00000048f,0.f,0.f,0.f})[0],wave(multiply(parameter("uv_rotate"),Value{2.00000048f,0.f,0.f,0.f}),false)[0],wave(multiply(parameter("uv_rotate"),Value{2.00000048f,0.f,0.f,0.f}),true)[0]);
        staged.lightConstants[8] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.lightConstants[9] = float4_t(parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_amount")[0]);
        staged.lightConstants[10] = float4_t(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0],multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("overlay_tiling")[0],parameter("reflection_tiling")[0]);
        staged.lightConstants[11] = float4_t(parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0]);
        staged.lightConstants[12] = float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.lightConstants[13] = float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-232.v1")
    {
        [&]() {
        staged.program = 232u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("emissive_uv_tiling"));
        staged.baseConstants[3] = vector(parameter("emissive_color"));
        staged.baseConstants[4] = vector(parameter("reflection_color"));
        staged.baseConstants[5] = vector(parameter("diffuse_color"));
        staged.baseConstants[6] = vector(parameter("overlay_color"));
        staged.baseConstants[7] = vector(parameter("specular_color"));
        staged.baseConstants[8] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.baseConstants[9] = float4_t(parameter("overlay_tiling")[0],parameter("emissive_intensity")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0]);
        staged.baseConstants[10] = float4_t(parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0]);
        staged.baseConstants[11] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0]);
        staged.lightTextureMask = 111u;
        staged.lightConstants[1] = vector(parameter("reflection_color"));
        staged.lightConstants[2] = vector(parameter("diffuse_color"));
        staged.lightConstants[3] = vector(parameter("overlay_color"));
        staged.lightConstants[4] = vector(parameter("specular_color"));
        staged.lightConstants[5] = float4_t(parameter("normal_intensity")[0],parameter("detail_normal_tiling")[0],parameter("detail_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.lightConstants[6] = float4_t(parameter("overlay_tiling")[0],parameter("emissive_intensity")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0]);
        staged.lightConstants[7] = float4_t(parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0]);
        staged.lightConstants[8] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-233.v1")
    {
        [&]() {
        staged.program = 233u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[3] = vector(parameter("overlay_color"));
        staged.baseConstants[4] = vector(parameter("specular_color"));
        staged.baseConstants[5] = vector(parameter("emissive_uv_tiling"));
        staged.baseConstants[6] = vector(parameter("emissive_color"));
        staged.baseConstants[7] = vector(parameter("reflection_color"));
        staged.baseConstants[8] = vector(parameter("diffuse_color"));
        staged.baseConstants[9] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[10] = float4_t(parameter("overlay_tiling")[0],parameter("overlay_sharpeness")[0],bounded(parameter("overlay_sharpeness"),Value{0.f,0.f,0.f,0.f},Value{0.99000001f,0.f,0.f,0.f})[0],subtract(Value{1.f,0.f,0.f,0.f},bounded(parameter("overlay_sharpeness"),Value{0.f,0.f,0.f,0.f},Value{0.99000001f,0.f,0.f,0.f}))[0]);
        staged.baseConstants[11] = float4_t(divide(Value{1.f,0.f,0.f,0.f},subtract(Value{1.f,0.f,0.f,0.f},bounded(parameter("overlay_sharpeness"),Value{0.f,0.f,0.f,0.f},Value{0.99000001f,0.f,0.f,0.f})))[0],parameter("overlay_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.baseConstants[12] = float4_t(parameter("emissive_intensity")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0]);
        staged.baseConstants[13] = float4_t(parameter("diffuse_brightness")[0],0.f,0.f,0.f);
        staged.lightTextureMask = 47u;
        staged.lightConstants[1] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.lightConstants[2] = vector(parameter("overlay_color"));
        staged.lightConstants[3] = vector(parameter("specular_color"));
        staged.lightConstants[4] = vector(parameter("reflection_color"));
        staged.lightConstants[5] = vector(parameter("diffuse_color"));
        staged.lightConstants[6] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.lightConstants[7] = float4_t(parameter("overlay_tiling")[0],parameter("overlay_sharpeness")[0],bounded(parameter("overlay_sharpeness"),Value{0.f,0.f,0.f,0.f},Value{0.99000001f,0.f,0.f,0.f})[0],subtract(Value{1.f,0.f,0.f,0.f},bounded(parameter("overlay_sharpeness"),Value{0.f,0.f,0.f,0.f},Value{0.99000001f,0.f,0.f,0.f}))[0]);
        staged.lightConstants[8] = float4_t(divide(Value{1.f,0.f,0.f,0.f},subtract(Value{1.f,0.f,0.f,0.f},bounded(parameter("overlay_sharpeness"),Value{0.f,0.f,0.f,0.f},Value{0.99000001f,0.f,0.f,0.f})))[0],parameter("overlay_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.lightConstants[9] = float4_t(parameter("emissive_intensity")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0]);
        staged.lightConstants[10] = float4_t(parameter("diffuse_brightness")[0],0.f,0.f,0.f);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-234.v1")
    {
        [&]() {
        staged.program = 234u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("overlay_direction"));
        staged.baseConstants[3] = vector(parameter("reflection_color"));
        staged.baseConstants[4] = vector(parameter("diffuse_color"));
        staged.baseConstants[5] = vector(parameter("overlay_color"));
        staged.baseConstants[6] = vector(parameter("specular_color"));
        staged.baseConstants[7] = float4_t(parameter("normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_amount")[0],add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0]);
        staged.baseConstants[8] = float4_t(multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("overlay_tiling")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0]);
        staged.baseConstants[9] = float4_t(parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0]);
        staged.baseConstants[10] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0]);
        staged.lightTextureMask = 31u;
        staged.lightConstants[1] = vector(parameter("overlay_direction"));
        staged.lightConstants[2] = vector(parameter("reflection_color"));
        staged.lightConstants[3] = vector(parameter("diffuse_color"));
        staged.lightConstants[4] = vector(parameter("overlay_color"));
        staged.lightConstants[5] = vector(parameter("specular_color"));
        staged.lightConstants[6] = float4_t(parameter("normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("overlay_amount")[0],add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount"))[0]);
        staged.lightConstants[7] = float4_t(multiply(add(Value{-0.5f,0.f,0.f,0.f},parameter("overlay_amount")),Value{2.f,0.f,0.f,0.f})[0],parameter("overlay_tiling")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0]);
        staged.lightConstants[8] = float4_t(parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0]);
        staged.lightConstants[9] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0]);
        }();
    }
    if (staged.program == 0u && family == "source.character.static-map-native-237.v1")
    {
        [&]() {
        staged.program = 237u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("normal_tiling_panning"));
        staged.baseConstants[3] = vector(parameter("sparkle_tiling_panning"));
        staged.baseConstants[4] = vector(parameter("diffuse_color"));
        staged.baseConstants[5] = vector(parameter("diffuse_tiling_panning"));
        staged.baseConstants[6] = vector(parameter("reflection_color"));
        staged.baseConstants[7] = float4_t(Value{}[0],parameter("normal_intensity")[0],parameter("distortion_intensity")[0],parameter("sparkle_intensity")[0]);
        staged.baseConstants[8] = float4_t(parameter("reflection_intensity")[0],parameter("opacity")[0],parameter("depth_bias")[0],parameter("screen_distortion_intensity")[0]);
        staged.lightTextureMask = 29u;
        staged.lightConstants[1] = vector(parameter("selectioncolor"));
        staged.lightConstants[2] = vector(parameter("normal_tiling_panning"));
        staged.lightConstants[3] = vector(parameter("diffuse_color"));
        staged.lightConstants[4] = vector(parameter("diffuse_tiling_panning"));
        staged.lightConstants[5] = vector(parameter("reflection_color"));
        staged.lightConstants[6] = float4_t(Value{}[0],parameter("normal_intensity")[0],parameter("distortion_intensity")[0],parameter("sparkle_intensity")[0]);
        staged.lightConstants[7] = float4_t(parameter("reflection_intensity")[0],parameter("opacity")[0],parameter("depth_bias")[0],parameter("screen_distortion_intensity")[0]);
        }();
    }
    if (staged.program == 0u) return false;
    if (staged.program == 80u)
    {
        staged.baseTextureMask |= 31u;
        staged.baseConstants[34] = vector(parameter("selectioncolor"));
        staged.baseConstants[35] = vector(parameter("hit_color"));
        staged.baseConstants[36] = vector(parameter("emissive_color"));
        staged.baseConstants[37] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[38] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[39] = vector(parameter("transcolor"));
        staged.baseConstants[40] = vector(parameter("buffcolor"));
        staged.baseConstants[41] = vector(parameter("constantoutline_color"));
        staged.baseConstants[42] = vector(parameter("diffusecolor"));
        staged.baseConstants[43] = vector(parameter("state"));
        staged.baseConstants[44] = vector(append(Value{},Value{},1u));
        staged.baseConstants[45] = vector(parameter("state_noise"));
        staged.baseConstants[46] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[47] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[48] = float4_t(parameter("emissive_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[49] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[50] = float4_t(Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.baseConstants[51] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("constantoutline_power")[0]);
        staged.baseConstants[52] = float4_t(parameter("diffuse_brightness")[0],multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})[0],wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)[0],multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false))[0]);
        staged.baseConstants[53] = float4_t(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true)[0],parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("rimlight_intensity")[0]);
    }
    if (staged.program == 81u)
    {
        staged.baseTextureMask |= 31u;
        staged.baseConstants[34] = vector(parameter("selectioncolor"));
        staged.baseConstants[35] = vector(parameter("hit_color"));
        staged.baseConstants[36] = vector(parameter("emissive_color"));
        staged.baseConstants[37] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[38] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[39] = vector(parameter("transcolor"));
        staged.baseConstants[40] = vector(parameter("buffcolor"));
        staged.baseConstants[41] = vector(parameter("constantoutline_color"));
        staged.baseConstants[42] = vector(parameter("diffusecolor"));
        staged.baseConstants[43] = vector(parameter("state"));
        staged.baseConstants[44] = vector(append(Value{},Value{},1u));
        staged.baseConstants[45] = vector(parameter("state_noise"));
        staged.baseConstants[46] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[47] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[48] = float4_t(parameter("emissive_intensity")[0],parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0]);
        staged.baseConstants[49] = float4_t(parameter("trans_rim_inradius")[0],parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[50] = float4_t(Value{}[0],parameter("constantoutline_blink")[0],multiply(parameter("constantoutline_blink"),Value{})[0],multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f})[0]);
        staged.baseConstants[51] = float4_t(wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)[0],add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[52] = float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0]);
    }
    if (staged.program == 82u)
    {
        staged.baseTextureMask |= 7u;
        staged.baseConstants[32] = vector(parameter("selectioncolor"));
        staged.baseConstants[33] = vector(parameter("hit_color"));
        staged.baseConstants[34] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[35] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[36] = vector(parameter("transcolor"));
        staged.baseConstants[37] = vector(parameter("constantoutline_color"));
        staged.baseConstants[38] = vector(parameter("diffusecolor"));
        staged.baseConstants[39] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[40] = float4_t(parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.baseConstants[41] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("constantoutline_power")[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[42] = float4_t(parameter("specular_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("specular_saturation"))[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.baseConstants[43] = float4_t(parameter("dead_texture_tiling")[0],parameter("dead")[0],0.f,0.f);
    }
    if (staged.program == 83u)
    {
        staged.baseTextureMask |= 31u;
        staged.baseConstants[34] = vector(parameter("selectioncolor"));
        staged.baseConstants[35] = vector(parameter("hit_color"));
        staged.baseConstants[36] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[37] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[38] = vector(parameter("transcolor"));
        staged.baseConstants[39] = vector(parameter("buffcolor"));
        staged.baseConstants[40] = vector(parameter("constantoutline_color"));
        staged.baseConstants[41] = vector(parameter("diffusecolor"));
        staged.baseConstants[42] = vector(parameter("reflection_color"));
        staged.baseConstants[43] = vector(parameter("state"));
        staged.baseConstants[44] = vector(append(Value{},Value{},1u));
        staged.baseConstants[45] = vector(parameter("state_noise"));
        staged.baseConstants[46] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[47] = vector(append(wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),false),wave(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[48] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[49] = float4_t(parameter("trans_rim_hard")[0],parameter("constantoutline")[0],multiply(parameter("constantoutline"),Value{2.f,0.f,0.f,0.f})[0],Value{}[0]);
        staged.baseConstants[50] = float4_t(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false))[0],multiply(add(Value{1.5f,0.f,0.f,0.f},wave(multiply(multiply(parameter("constantoutline_blink"),Value{}),Value{6.28318548f,0.f,0.f,0.f}),false)),Value{0.400000006f,0.f,0.f,0.f})[0],parameter("diffuse_brightness")[0],parameter("reflection_intensity")[0]);
        staged.baseConstants[51] = float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0]);
    }
    if (staged.program >= 80u && staged.program <= 83u)
    {
        staged.baseConstants[63].x = parameter("constantoutline_blink")[0];
        staged.lightConstants[63].x = staged.baseConstants[63].x;
    }
    if (!valid || consumed.size()!=parameters.size()) return false;
    result=staged;
    return true;
}

// BEGIN GENERATED SOURCE CHARACTER NAMED VECTOR PATCH
// Generated from exact direct named-vector packing, including subsequent copies.
// AUTO/map programs and parameters absent from the selected source program are no-ops.
inline bool Patch_NamedVector(Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& material,
    std::string_view parameter, const float4_t& value)
{
    if (parameter != "transcolor" && parameter != "buffcolor") return false;
    for (const float component : {value.x, value.y, value.z, value.w})
        if (!std::isfinite(component) || std::abs(component) > 1000000.0f) return false;
    struct Binding { uint32_t program; uint64_t baseTrans, lightTrans, baseBuff, lightBuff; };
    static constexpr Binding bindings[] = {
        {1u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000800ull, 0x0000000000000000ull},
        {2u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000001000ull, 0x0000000000000000ull},
        {3u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000001000ull, 0x0000000000000000ull},
        {4u, 0x0000000000100000ull, 0x0000000000000000ull, 0x0000000000200000ull, 0x0000000000000000ull},
        {7u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000200ull, 0x0000000000000000ull},
        {8u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {9u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {10u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000001000ull, 0x0000000000000000ull},
        {11u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {12u, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000010ull, 0x0000000000000000ull},
        {13u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {14u, 0x0000000000002000ull, 0x0000000000000000ull, 0x0000000000004000ull, 0x0000000000000000ull},
        {15u, 0x0000000000002000ull, 0x0000000000000000ull, 0x0000000000004000ull, 0x0000000000000000ull},
        {16u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {17u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {18u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000200ull, 0x0000000000000000ull},
        {19u, 0x0000000000008000ull, 0x0000000000000000ull, 0x0000000000004000ull, 0x0000000000000000ull},
        {20u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000200ull, 0x0000000000000000ull},
        {21u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000001000ull, 0x0000000000000000ull},
        {22u, 0x0000000000000040ull, 0x0000000000000000ull, 0x0000000000000080ull, 0x0000000000000000ull},
        {23u, 0x0000000000004000ull, 0x0000000000000000ull, 0x0000000000008000ull, 0x0000000000000000ull},
        {24u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {25u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {26u, 0x0000000000000200ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {27u, 0x0000000000002000ull, 0x0000000000000000ull, 0x0000000000004000ull, 0x0000000000000000ull},
        {28u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000001000ull, 0x0000000000000000ull},
        {29u, 0x0000000000000200ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {30u, 0x0000000000000200ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {31u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000800ull, 0x0000000000000000ull},
        {32u, 0x0000000000000200ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {80u, 0x0000008000000080ull, 0x0000000000000000ull, 0x0000010000000100ull, 0x0000000000000000ull},
        {81u, 0x0000008000000080ull, 0x0000000000000000ull, 0x0000010000000100ull, 0x0000000000000000ull},
        {82u, 0x0000001000000010ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull},
        {83u, 0x0000004000000040ull, 0x0000000000000000ull, 0x0000008000000080ull, 0x0000000000000000ull},
        {84u, 0x0000000000000004ull, 0x0000000000000000ull, 0x0000000000000008ull, 0x0000000000000000ull},
        {85u, 0x0000000000004000ull, 0x0000000000000000ull, 0x0000000000008000ull, 0x0000000000000000ull},
        {86u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {87u, 0x0000000001000000ull, 0x0000000000000000ull, 0x0000000002000000ull, 0x0000000000000000ull},
        {88u, 0x0000000000000200ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {89u, 0x0000000000004000ull, 0x0000000000000000ull, 0x0000000000008000ull, 0x0000000000000000ull},
        {90u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {91u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000001000ull, 0x0000000000000000ull},
        {92u, 0x0000000000000080ull, 0x0000000000000000ull, 0x0000000000000100ull, 0x0000000000000000ull},
        {93u, 0x0000000000000040ull, 0x0000000000000000ull, 0x0000000000000080ull, 0x0000000000000000ull},
        {94u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000800ull, 0x0000000000000000ull},
        {95u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {96u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {97u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000001000ull, 0x0000000000000000ull},
        {98u, 0x0000000000000080ull, 0x0000000000000000ull, 0x0000000000000100ull, 0x0000000000000000ull},
        {99u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000200ull, 0x0000000000000000ull},
        {100u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000800ull, 0x0000000000000000ull},
        {101u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {102u, 0x0000000000010000ull, 0x0000000000000000ull, 0x0000000000020000ull, 0x0000000000000000ull},
        {103u, 0x0000000000010000ull, 0x0000000000000000ull, 0x0000000000020000ull, 0x0000000000000000ull},
        {104u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {105u, 0x0000000000010000ull, 0x0000000000000000ull, 0x0000000000020000ull, 0x0000000000000000ull},
        {106u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {107u, 0x0000000000010000ull, 0x0000000000000000ull, 0x0000000000020000ull, 0x0000000000000000ull},
        {108u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000001000ull, 0x0000000000000000ull},
        {109u, 0x0000000000000040ull, 0x0000000000000000ull, 0x0000000000000080ull, 0x0000000000000000ull},
        {110u, 0x0000000000000200ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {111u, 0x0000000000000200ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {112u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {160u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000200ull, 0x0000000000000000ull},
        {161u, 0x0000000000008000ull, 0x0000000000000000ull, 0x0000000000004000ull, 0x0000000000000000ull},
        {162u, 0x0000000000002000ull, 0x0000000000000000ull, 0x0000000000004000ull, 0x0000000000000000ull},
        {163u, 0x0000000000004000ull, 0x0000000000000000ull, 0x0000000000008000ull, 0x0000000000000000ull},
        {164u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {165u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {166u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {167u, 0x0000000000010000ull, 0x0000000000000000ull, 0x0000000000008000ull, 0x0000000000000000ull},
        {168u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {169u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {170u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000200ull, 0x0000000000000000ull},
        {171u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000200ull, 0x0000000000000000ull},
        {172u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {173u, 0x0000000000004000ull, 0x0000000000000000ull, 0x0000000000008000ull, 0x0000000000000000ull},
        {174u, 0x0000000000000200ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {175u, 0x0000000000008000ull, 0x0000000000000000ull, 0x0000000000010000ull, 0x0000000000000000ull},
        {176u, 0x0000000000008000ull, 0x0000000000000000ull, 0x0000000000010000ull, 0x0000000000000000ull},
        {177u, 0x0000000000400000ull, 0x0000000000000000ull, 0x0000000000800000ull, 0x0000000000000000ull},
        {178u, 0x0000000000100000ull, 0x0000000000000000ull, 0x0000000000200000ull, 0x0000000000000000ull},
        {179u, 0x0000000000010000ull, 0x0000000000000000ull, 0x0000000000008000ull, 0x0000000000000000ull},
        {180u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000001000ull, 0x0000000000000000ull},
        {181u, 0x0000000000002000ull, 0x0000000000000000ull, 0x0000000000004000ull, 0x0000000000000000ull},
        {182u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000200ull, 0x0000000000000000ull},
        {183u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000800ull, 0x0000000000000000ull},
        {184u, 0x0000000000008000ull, 0x0000000000000000ull, 0x0000000000010000ull, 0x0000000000000000ull},
        {185u, 0x0000000000004000ull, 0x0000000000000000ull, 0x0000000000008000ull, 0x0000000000000000ull},
        {186u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000800ull, 0x0000000000000000ull},
        {187u, 0x0000000000000040ull, 0x0000000000000000ull, 0x0000000000000080ull, 0x0000000000000000ull},
        {188u, 0x0000000000008000ull, 0x0000000000000000ull, 0x0000000000010000ull, 0x0000000000000000ull},
        {189u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {190u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000001000ull, 0x0000000000000000ull},
        {191u, 0x0000000000080000ull, 0x0000000000000000ull, 0x0000000000100000ull, 0x0000000000000000ull},
        {192u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000200ull, 0x0000000000000000ull},
        {193u, 0x0000000000004000ull, 0x0000000000000000ull, 0x0000000000008000ull, 0x0000000000000000ull},
        {194u, 0x0000000000004000ull, 0x0000000000000000ull, 0x0000000000008000ull, 0x0000000000000000ull},
        {195u, 0x0000000000000800ull, 0x0000000000000000ull, 0x0000000000001000ull, 0x0000000000000000ull},
        {196u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {197u, 0x0000000000004000ull, 0x0000000000000000ull, 0x0000000000008000ull, 0x0000000000000000ull},
        {198u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {199u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
        {200u, 0x0000000000100000ull, 0x0000000000000000ull, 0x0000000000200000ull, 0x0000000000000000ull},
        {210u, 0x0000000000000200ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {211u, 0x0000000000000200ull, 0x0000000000000000ull, 0x0000000000000400ull, 0x0000000000000000ull},
        {213u, 0x0000000000000400ull, 0x0000000000000000ull, 0x0000000000000200ull, 0x0000000000000000ull},
        {235u, 0x0000000000010000ull, 0x0000000000000000ull, 0x0000000000020000ull, 0x0000000000000000ull},
        {236u, 0x0000000000001000ull, 0x0000000000000000ull, 0x0000000000002000ull, 0x0000000000000000ull},
    };
    for (const auto& binding : bindings) {
        if (binding.program != material.program) continue;
        const bool trans = parameter == "transcolor";
        const uint64_t base = trans ? binding.baseTrans : binding.baseBuff;
        const uint64_t light = trans ? binding.lightTrans : binding.lightBuff;
        if (base == 0u && light == 0u) return false;
        for (size_t index = 0u; index < 64u; ++index) {
            const uint64_t bit = uint64_t{1} << index;
            if ((base & bit) != 0u) material.baseConstants[index] = value;
            if ((light & bit) != 0u) material.lightConstants[index] = value;
        }
        return true;
    }
    return false;
}
// END GENERATED SOURCE CHARACTER NAMED VECTOR PATCH

inline bool Configure(const std::string& family, const DATA_JSON_VALUE& parameters,
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& result)
{
    PARAMETER_VALUES values;
    return Read(parameters, values) && Configure(family, values, result);
}
}
