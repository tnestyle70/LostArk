#pragma once

#include "DataJson.h"
#include "BinaryAsset/ModelAssetData.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <set>
#include <string>

namespace Client::SourceMapWaterMaterial
{
inline bool Configure(const std::string& family,
    const std::map<std::string, std::array<float, 4>>& parameters,
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
    const auto append=[](Value a,Value b,size_t n) { Value o{};for(size_t i=0;i<4u;++i)o[i]=i<n?a[i]:b[i-n];return o; };
    const auto bounded=[](Value a,Value lo,Value hi) { for(size_t i=0;i<4u;++i)a[i]=(std::min)((std::max)(a[i],lo[i]),hi[i]);return a; };
    const auto periodic=[](Value a) { for(auto& v:a)v-=std::floor(v);return a; };
    const auto vector=[](Value a) { return float4_t(a[0],a[1],a[2],a[3]); };
    const auto trigonometric=[](Value a,bool cosine) { for(auto& v:a)v=cosine?std::cos(v):std::sin(v);return a; };
    const auto divide=[](Value a,Value b) { for(size_t i=0;i<4u;++i)a[i]/=b[i];return a; };
    const auto minimumValue=[](Value a,Value b){for(size_t i=0;i<4;++i)a[i]=(std::min)(a[i],b[i]);return a;};
    const auto maximumValue=[](Value a,Value b){for(size_t i=0;i<4;++i)a[i]=(std::max)(a[i],b[i]);return a;};
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS staged{};
    if (family == "source.map.water-38.v1")
    {
        staged.program = 38u;
        staged.baseTextureMask = 63u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("water_color"));
        staged.baseConstants[3] = vector(parameter("rim_color"));
        staged.baseConstants[4] = vector(append(periodic(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})),periodic(multiply(Value{},Value{0.100000001f,0.f,0.f,0.f})),1u));
        staged.baseConstants[5] = vector(append(periodic(multiply(Value{},Value{-0.100000001f,0.f,0.f,0.f})),periodic(multiply(Value{},Value{-0.100000001f,0.f,0.f,0.f})),1u));
        staged.baseConstants[6] = vector(append(periodic(multiply(Value{},Value{0.000500000024f,0.f,0.f,0.f})),periodic(multiply(Value{},Value{0.00499999989f,0.f,0.f,0.f})),1u));
        staged.baseConstants[7] = float4_t(parameter("normal_tex_tilling")[0],multiply(Value{0.75f,0.f,0.f,0.f},parameter("normal_tex_tilling"))[0],Value{}[0],parameter("water_wave_speed_x_1")[0]);
        staged.baseConstants[8] = float4_t(parameter("water_wave_speed_y_1")[0],parameter("water_wave_speed_x_2")[0],parameter("water_wave_speed_y_2")[0],parameter("normal_intensity")[0]);
        staged.baseConstants[9] = float4_t(parameter("cube")[0],parameter("rim_power")[0],parameter("rim_intensity")[0],parameter("depth_tex_area")[0]);
        staged.baseConstants[10] = float4_t(parameter("water_spec_power")[0],parameter("water_spec_intensity")[0],parameter("water_opacity")[0],parameter("water_depth")[0]);
    }
    else if (family == "source.map.water-39.v1")
    {
        staged.program = 39u;
        staged.baseTextureMask = 31u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("water_color"));
        staged.baseConstants[3] = vector(parameter("rim_color"));
        staged.baseConstants[4] = vector(append(periodic(multiply(Value{},Value{0.00100000005f,0.f,0.f,0.f})),periodic(multiply(Value{},Value{0.00499999989f,0.f,0.f,0.f})),1u));
        staged.baseConstants[5] = vector(append(periodic(multiply(Value{},Value{0.000500000024f,0.f,0.f,0.f})),periodic(multiply(Value{},Value{0.00499999989f,0.f,0.f,0.f})),1u));
        staged.baseConstants[6] = float4_t(parameter("normal_tex_tilling")[0],multiply(Value{0.75f,0.f,0.f,0.f},parameter("normal_tex_tilling"))[0],Value{}[0],parameter("water_wave_speed")[0]);
        staged.baseConstants[7] = float4_t(multiply(Value{2.f,0.f,0.f,0.f},parameter("water_wave_speed"))[0],parameter("normal_intensity")[0],parameter("cube_desturation")[0],parameter("rim_power")[0]);
        staged.baseConstants[8] = float4_t(parameter("rim_intensity")[0],parameter("depth_tex_area")[0],multiply(Value{},Value{0.00100000005f,0.f,0.f,0.f})[0],multiply(Value{},Value{0.00499999989f,0.f,0.f,0.f})[0]);
        staged.baseConstants[9] = float4_t(periodic(multiply(Value{},Value{0.00499999989f,0.f,0.f,0.f}))[0],periodic(multiply(Value{},Value{0.00100000005f,0.f,0.f,0.f}))[0],multiply(parameter("water_wave_speed"),Value{-1.f,0.f,0.f,0.f})[0],multiply(Value{},Value{0.000500000024f,0.f,0.f,0.f})[0]);
        staged.baseConstants[10] = float4_t(periodic(multiply(Value{},Value{0.000500000024f,0.f,0.f,0.f}))[0],parameter("water_spec_power")[0],parameter("water_spec_intensity")[0],parameter("water_opacity")[0]);
        staged.baseConstants[11] = float4_t(parameter("water_depth")[0],parameter("total_distortion_intensity")[0],0.f,0.f);
    }
    else if (family == "source.map.water-40.v1")
    {
        staged.program = 40u;
        // Native Baked material values occupy slots 32..59.
        staged.baseConstants[33] = vector(parameter("selectioncolor"));
        staged.baseConstants[34] = vector(parameter("normal_tiling_panning"));
        staged.baseConstants[35] = vector(parameter("detail_normal_tiling_panning"));
        staged.baseConstants[36] = vector(parameter("diffuse_color"));
        staged.baseConstants[37] = vector(parameter("reflection_color"));
        staged.baseConstants[38] = vector(parameter("reflection_tiling_panning"));
        staged.baseConstants[39] = float4_t(parameter("uv_tiling")[0],Value{}[0],parameter("normal_intensity")[0],parameter("normal_distortion_intensity")[0]);
        staged.baseConstants[40] = float4_t(parameter("fresnel_power")[0],parameter("fresnel_intensity")[0],parameter("detail_normal_intensity")[0],parameter("screen_distortion_intensity")[0]);
        staged.baseConstants[41] = float4_t(parameter("depth_bias")[0],parameter("opacity_power")[0],parameter("opacity")[0],parameter("diffuse_tiling")[0]);
        staged.baseConstants[42] = float4_t(parameter("reflection_uv")[0],parameter("reflection_intensity")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.baseTextureMask = 15u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("normal_tiling_panning"));
        staged.baseConstants[3] = vector(parameter("detail_normal_tiling_panning"));
        staged.baseConstants[4] = vector(parameter("diffuse_color"));
        staged.baseConstants[5] = vector(parameter("reflection_color"));
        staged.baseConstants[6] = vector(parameter("reflection_tiling_panning"));
        staged.baseConstants[7] = float4_t(parameter("uv_tiling")[0],Value{}[0],parameter("normal_intensity")[0],parameter("normal_distortion_intensity")[0]);
        staged.baseConstants[8] = float4_t(parameter("fresnel_power")[0],parameter("fresnel_intensity")[0],parameter("detail_normal_intensity")[0],parameter("screen_distortion_intensity")[0]);
        staged.baseConstants[9] = float4_t(parameter("depth_bias")[0],parameter("opacity_power")[0],parameter("opacity")[0],parameter("diffuse_tiling")[0]);
        staged.baseConstants[10] = float4_t(parameter("reflection_uv")[0],parameter("reflection_intensity")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.lightConstants[1]=vector(parameter("normal_tiling_panning"));
        staged.lightConstants[2]=vector(parameter("detail_normal_tiling_panning"));
        staged.lightConstants[3]=vector(parameter("diffuse_color"));
        staged.lightConstants[4]=vector(parameter("reflection_color"));
        staged.lightConstants[5]=vector(parameter("reflection_tiling_panning"));
        staged.lightConstants[6]=float4_t(parameter("uv_tiling")[0],Value{}[0],parameter("normal_intensity")[0],parameter("normal_distortion_intensity")[0]);
        staged.lightConstants[7]=float4_t(parameter("fresnel_power")[0],parameter("fresnel_intensity")[0],parameter("detail_normal_intensity")[0],parameter("screen_distortion_intensity")[0]);
        staged.lightConstants[8]=float4_t(parameter("depth_bias")[0],parameter("opacity_power")[0],parameter("opacity")[0],parameter("diffuse_tiling")[0]);
        staged.lightConstants[9]=float4_t(parameter("reflection_uv")[0],parameter("reflection_intensity")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.lightTextureMask=15u;
    }
    else if (family == "source.map.water-41.v1")
    {
        staged.program = 41u;
        // Native Baked material values occupy slots 32..59.
        staged.baseConstants[33] = vector(parameter("selectioncolor"));
        staged.baseConstants[34] = vector(parameter("normal_tiling_panning"));
        staged.baseConstants[35] = vector(parameter("detail_normal_tiling_panning"));
        staged.baseConstants[36] = vector(parameter("diffuse_color"));
        staged.baseConstants[37] = vector(parameter("reflection_color"));
        staged.baseConstants[38] = vector(parameter("reflection_tiling_panning"));
        staged.baseConstants[39] = float4_t(Value{}[0],parameter("normal_intensity")[0],parameter("normal_distortion_intensity")[0],parameter("fresnel_power")[0]);
        staged.baseConstants[40] = float4_t(parameter("fresnel_intensity")[0],parameter("detail_normal_intensity")[0],parameter("screen_distortion_intensity")[0],parameter("depth_bias")[0]);
        staged.baseConstants[41] = float4_t(parameter("opacity_power")[0],parameter("opacity")[0],parameter("diffuse_tiling")[0],parameter("reflection_uv")[0]);
        staged.baseConstants[42] = float4_t(parameter("reflection_intensity")[0],parameter("specular_intensity")[0],parameter("specular_power")[0],0.f);
        staged.baseTextureMask = 15u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("normal_tiling_panning"));
        staged.baseConstants[3] = vector(parameter("detail_normal_tiling_panning"));
        staged.baseConstants[4] = vector(parameter("diffuse_color"));
        staged.baseConstants[5] = vector(parameter("reflection_color"));
        staged.baseConstants[6] = vector(parameter("reflection_tiling_panning"));
        staged.baseConstants[7] = float4_t(Value{}[0],parameter("normal_intensity")[0],parameter("normal_distortion_intensity")[0],parameter("fresnel_power")[0]);
        staged.baseConstants[8] = float4_t(parameter("fresnel_intensity")[0],parameter("detail_normal_intensity")[0],parameter("screen_distortion_intensity")[0],parameter("depth_bias")[0]);
        staged.baseConstants[9] = float4_t(parameter("opacity_power")[0],parameter("opacity")[0],parameter("diffuse_tiling")[0],parameter("reflection_uv")[0]);
        staged.baseConstants[10] = float4_t(parameter("reflection_intensity")[0],parameter("specular_intensity")[0],parameter("specular_power")[0],0.f);
        staged.lightConstants[1]=vector(parameter("normal_tiling_panning"));
        staged.lightConstants[2]=vector(parameter("detail_normal_tiling_panning"));
        staged.lightConstants[3]=vector(parameter("diffuse_color"));
        staged.lightConstants[4]=vector(parameter("reflection_color"));
        staged.lightConstants[5]=vector(parameter("reflection_tiling_panning"));
        staged.lightConstants[6]=float4_t(Value{}[0],parameter("normal_intensity")[0],parameter("normal_distortion_intensity")[0],parameter("fresnel_power")[0]);
        staged.lightConstants[7]=float4_t(parameter("fresnel_intensity")[0],parameter("detail_normal_intensity")[0],parameter("screen_distortion_intensity")[0],parameter("depth_bias")[0]);
        staged.lightConstants[8]=float4_t(parameter("opacity_power")[0],parameter("opacity")[0],parameter("diffuse_tiling")[0],parameter("reflection_uv")[0]);
        staged.lightConstants[9]=float4_t(parameter("reflection_intensity")[0],parameter("specular_intensity")[0],parameter("specular_power")[0],0.f);
        staged.lightTextureMask=15u;
    }
    else if (family == "source.map.water-42.v1")
    {
        staged.program = 42u;
        // Native Baked material values occupy slots 32..59.
        staged.baseConstants[33] = vector(parameter("selectioncolor"));
        staged.baseConstants[34] = vector(parameter("normal_tiling_panning"));
        staged.baseConstants[35] = vector(parameter("detail_normal_tiling_panning"));
        staged.baseConstants[36] = vector(parameter("sky_color"));
        staged.baseConstants[37] = vector(parameter("sky_tiling_panning"));
        staged.baseConstants[38] = vector(parameter("reflection_color"));
        staged.baseConstants[39] = vector(parameter("reflection_tiling_panning"));
        staged.baseConstants[40] = vector(parameter("fresnel_color"));
        staged.baseConstants[41] = vector(parameter("diffuse_color"));
        staged.baseConstants[42] = vector(parameter("diffuse_tiling_panning"));
        staged.baseConstants[43] = float4_t(Value{}[0],parameter("normal_intensity")[0],parameter("normal_distortion_intensity")[0],parameter("detail_normal_intensity")[0]);
        staged.baseConstants[44] = float4_t(parameter("fresnel_power")[0],parameter("distortion_intensity")[0],parameter("sky_power")[0],parameter("sky_intensity")[0]);
        staged.baseConstants[45] = float4_t(parameter("reflection_uv")[0],parameter("reflection_power")[0],parameter("reflection_intensity")[0],parameter("fresnel_tiling")[0]);
        staged.baseConstants[46] = float4_t(parameter("fresnel_intensity")[0],parameter("mask_distortion_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0]);
        staged.baseConstants[47] = float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("depth_bias")[0],parameter("opacity")[0]);
        staged.baseConstants[48] = float4_t(parameter("opacity_power")[0],parameter("screen_distortion_intensity")[0],0.f,0.f);
        staged.baseTextureMask = 127u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("normal_tiling_panning"));
        staged.baseConstants[3] = vector(parameter("detail_normal_tiling_panning"));
        staged.baseConstants[4] = vector(parameter("sky_color"));
        staged.baseConstants[5] = vector(parameter("sky_tiling_panning"));
        staged.baseConstants[6] = vector(parameter("reflection_color"));
        staged.baseConstants[7] = vector(parameter("reflection_tiling_panning"));
        staged.baseConstants[8] = vector(parameter("fresnel_color"));
        staged.baseConstants[9] = vector(parameter("diffuse_color"));
        staged.baseConstants[10] = vector(parameter("diffuse_tiling_panning"));
        staged.baseConstants[11] = float4_t(Value{}[0],parameter("normal_intensity")[0],parameter("normal_distortion_intensity")[0],parameter("detail_normal_intensity")[0]);
        staged.baseConstants[12] = float4_t(parameter("fresnel_power")[0],parameter("distortion_intensity")[0],parameter("sky_power")[0],parameter("sky_intensity")[0]);
        staged.baseConstants[13] = float4_t(parameter("reflection_uv")[0],parameter("reflection_power")[0],parameter("reflection_intensity")[0],parameter("fresnel_tiling")[0]);
        staged.baseConstants[14] = float4_t(parameter("fresnel_intensity")[0],parameter("mask_distortion_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0]);
        staged.baseConstants[15] = float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("depth_bias")[0],parameter("opacity")[0]);
        staged.baseConstants[16] = float4_t(parameter("opacity_power")[0],parameter("screen_distortion_intensity")[0],0.f,0.f);
        staged.lightConstants[1]=vector(parameter("selectioncolor"));
        staged.lightConstants[2]=vector(parameter("normal_tiling_panning"));
        staged.lightConstants[3]=vector(parameter("detail_normal_tiling_panning"));
        staged.lightConstants[4]=vector(parameter("reflection_color"));
        staged.lightConstants[5]=vector(parameter("reflection_tiling_panning"));
        staged.lightConstants[6]=vector(parameter("fresnel_color"));
        staged.lightConstants[7]=vector(parameter("diffuse_color"));
        staged.lightConstants[8]=vector(parameter("diffuse_tiling_panning"));
        staged.lightConstants[9]=float4_t(Value{}[0],parameter("normal_intensity")[0],parameter("normal_distortion_intensity")[0],parameter("detail_normal_intensity")[0]);
        staged.lightConstants[10]=float4_t(parameter("fresnel_power")[0],parameter("distortion_intensity")[0],parameter("sky_power")[0],parameter("sky_intensity")[0]);
        staged.lightConstants[11]=float4_t(parameter("reflection_uv")[0],parameter("reflection_power")[0],parameter("reflection_intensity")[0],parameter("fresnel_tiling")[0]);
        staged.lightConstants[12]=float4_t(parameter("fresnel_intensity")[0],parameter("mask_distortion_intensity")[0],parameter("diffuse_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("diffuse_saturation"))[0]);
        staged.lightConstants[13]=float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("depth_bias")[0],parameter("opacity")[0]);
        staged.lightConstants[14]=float4_t(parameter("opacity_power")[0],parameter("screen_distortion_intensity")[0],0.f,0.f);
        staged.lightTextureMask=123u;
    }
    else if (family == "source.map.water-43.v1")
    {
        staged.program = 43u;
        // Native Baked material values occupy slots 32..59.
        staged.baseConstants[33] = vector(parameter("selectioncolor"));
        staged.baseConstants[34] = vector(parameter("01.normalscale(2)_&_normalspeed(2)"));
        staged.baseConstants[35] = vector(parameter("03.normal_multiply(3)"));
        staged.baseConstants[36] = vector(append(append(trigonometric(multiply(parameter("02.refectionmaprot"),Value{6.28318548f,0.f,0.f,0.f}),true),trigonometric(multiply(parameter("02.refectionmaprot"),Value{6.28318548f,0.f,0.f,0.f}),false),1u),Value{0.0500000007f,0.f,0.f,0.f},2u));
        staged.baseConstants[37] = vector(parameter("03.skycolor"));
        staged.baseConstants[38] = vector(parameter("01.color_low"));
        staged.baseConstants[39] = vector(parameter("02.color_high"));
        staged.baseConstants[40] = vector(parameter("01.color_foam"));
        staged.baseConstants[41] = float4_t(Value{}[0],parameter("depth_bias")[0],parameter("01.refractionstr")[0],parameter("15.sunuvdistortionstr")[0]);
        staged.baseTextureMask = 15u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("01.normalscale(2)_&_normalspeed(2)"));
        staged.baseConstants[3] = vector(parameter("03.normal_multiply(3)"));
        staged.baseConstants[4] = vector(append(append(trigonometric(multiply(parameter("02.refectionmaprot"),Value{6.28318548f,0.f,0.f,0.f}),true),trigonometric(multiply(parameter("02.refectionmaprot"),Value{6.28318548f,0.f,0.f,0.f}),false),1u),Value{0.0500000007f,0.f,0.f,0.f},2u));
        staged.baseConstants[5] = vector(parameter("03.skycolor"));
        staged.baseConstants[6] = vector(parameter("01.color_low"));
        staged.baseConstants[7] = vector(parameter("02.color_high"));
        staged.baseConstants[8] = vector(parameter("01.color_foam"));
        staged.baseConstants[9] = float4_t(Value{}[0],parameter("depth_bias")[0],parameter("01.refractionstr")[0],parameter("15.sunuvdistortionstr")[0]);
        staged.baseConstants[63].x=parameter("01.scale")[0];
        staged.lightConstants[1]=vector(parameter("01.normalscale(2)_&_normalspeed(2)"));
        staged.lightConstants[2]=vector(parameter("03.normal_multiply(3)"));
        staged.lightConstants[3]=vector(append(trigonometric(multiply(add(Value{-1.57000005f,0.f,0.f,0.f},multiply(Value{6.28000021f,0.f,0.f,0.f},parameter("02.refectionmaprot"))),Value{1.f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},trigonometric(multiply(add(Value{-1.57000005f,0.f,0.f,0.f},multiply(Value{6.28000021f,0.f,0.f,0.f},parameter("02.refectionmaprot"))),Value{1.f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[4]=vector(append(trigonometric(multiply(add(Value{-1.57000005f,0.f,0.f,0.f},multiply(Value{6.28000021f,0.f,0.f,0.f},parameter("02.refectionmaprot"))),Value{1.f,0.f,0.f,0.f}),false),trigonometric(multiply(add(Value{-1.57000005f,0.f,0.f,0.f},multiply(Value{6.28000021f,0.f,0.f,0.f},parameter("02.refectionmaprot"))),Value{1.f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[5]=vector(append(append(trigonometric(multiply(parameter("02.refectionmaprot"),Value{6.28318548f,0.f,0.f,0.f}),true),trigonometric(multiply(parameter("02.refectionmaprot"),Value{6.28318548f,0.f,0.f,0.f}),false),1u),Value{0.0500000007f,0.f,0.f,0.f},2u));
        staged.lightConstants[6]=vector(parameter("11.sumcolor"));
        staged.lightConstants[7]=vector(append(trigonometric(multiply(add(multiply(Value{-6.28000021f,0.f,0.f,0.f},parameter("02.refectionmaprot")),Value{4.71000004f,0.f,0.f,0.f}),Value{-1.f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},trigonometric(multiply(add(multiply(Value{-6.28000021f,0.f,0.f,0.f},parameter("02.refectionmaprot")),Value{4.71000004f,0.f,0.f,0.f}),Value{-1.f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[8]=vector(append(trigonometric(multiply(add(multiply(Value{-6.28000021f,0.f,0.f,0.f},parameter("02.refectionmaprot")),Value{4.71000004f,0.f,0.f,0.f}),Value{-1.f,0.f,0.f,0.f}),false),trigonometric(multiply(add(multiply(Value{-6.28000021f,0.f,0.f,0.f},parameter("02.refectionmaprot")),Value{4.71000004f,0.f,0.f,0.f}),Value{-1.f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[9]=vector(parameter("03.skycolor"));
        staged.lightConstants[10]=float4_t(Value{}[0],parameter("depth_bias")[0],parameter("01.refractionstr")[0],parameter("15.sunuvdistortionstr")[0]);
        staged.lightConstants[11]=float4_t(trigonometric(multiply(parameter("02.refectionmaprot"),Value{6.28318548f,0.f,0.f,0.f}),false)[0],trigonometric(multiply(parameter("02.refectionmaprot"),Value{6.28318548f,0.f,0.f,0.f}),true)[0],parameter("13.sunscatteringstr")[0],parameter("12.sundissolve")[0]);
        staged.lightTextureMask=127u;
    }
    else return false;
    if(!valid || consumed.size()!=parameters.size())return false;
    result=staged;return true;
}
}
