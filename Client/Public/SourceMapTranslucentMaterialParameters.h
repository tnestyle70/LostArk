#pragma once

#include "DataJson.h"
#include "BinaryAsset/ModelAssetData.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <set>
#include <string>

namespace Client::SourceMapTranslucentMaterial
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
    if(family=="source.map.translucent-44.v1")
    {
        staged.program=44u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(parameter("reflection_color"));
        staged.baseConstants[3]=vector(parameter("diffuse_color"));
        staged.baseConstants[4]=float4_t(parameter("normal_intensity")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[5]=float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("opacity_intensity")[0],0.f);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(parameter("reflection_color"));
        staged.baseConstants[35]=vector(parameter("diffuse_color"));
        staged.baseConstants[36]=vector(parameter("specular_color"));
        staged.baseConstants[37]=float4_t(parameter("normal_intensity")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[38]=float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("opacity_intensity")[0],0.f);
        staged.baseTextureMask=7u;
        staged.lightConstants[1]=vector(parameter("reflection_color"));
        staged.lightConstants[2]=vector(parameter("diffuse_color"));
        staged.lightConstants[3]=vector(parameter("specular_color"));
        staged.lightConstants[4]=float4_t(parameter("normal_intensity")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0]);
        staged.lightConstants[5]=float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("opacity_intensity")[0],0.f);
        staged.lightTextureMask=7u;
    }
    else if(family=="source.map.translucent-45.v1")
    {
        staged.program=45u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(parameter("diffuse_color"));
        staged.baseConstants[3]=float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.baseConstants[4]=float4_t(parameter("min_opacity")[0],parameter("occludedopacity")[0],0.f,0.f);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(parameter("diffuse_color"));
        staged.baseConstants[35]=vector(parameter("specular_color"));
        staged.baseConstants[36]=float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.baseConstants[37]=float4_t(parameter("min_opacity")[0],parameter("occludedopacity")[0],0.f,0.f);
        staged.baseTextureMask=7u;
        staged.lightConstants[1]=vector(parameter("diffuse_color"));
        staged.lightConstants[2]=vector(parameter("specular_color"));
        staged.lightConstants[3]=float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.lightConstants[4]=float4_t(parameter("min_opacity")[0],parameter("occludedopacity")[0],0.f,0.f);
        staged.lightTextureMask=7u;
    }
    else if(family=="source.map.translucent-46.v1")
    {
        staged.program=46u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(parameter("emissive_uv_tiling"));
        staged.baseConstants[3]=vector(parameter("emissive_color"));
        staged.baseConstants[4]=vector(parameter("diffuse_color"));
        staged.baseConstants[5]=float4_t(parameter("normal_intensity")[0],parameter("emissive_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0]);
        staged.baseConstants[6]=float4_t(parameter("specular_power")[0],parameter("opacity_intensity")[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0]);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(parameter("emissive_uv_tiling"));
        staged.baseConstants[35]=vector(parameter("emissive_color"));
        staged.baseConstants[36]=vector(parameter("diffuse_color"));
        staged.baseConstants[37]=vector(parameter("specular_color"));
        staged.baseConstants[38]=float4_t(parameter("normal_intensity")[0],parameter("emissive_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0]);
        staged.baseConstants[39]=float4_t(parameter("specular_power")[0],parameter("opacity_intensity")[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0]);
        staged.baseTextureMask=7u;
        staged.lightConstants[1]=vector(parameter("diffuse_color"));
        staged.lightConstants[2]=vector(parameter("specular_color"));
        staged.lightConstants[3]=vector(parameter("rimlight_color"));
        staged.lightConstants[4]=float4_t(parameter("normal_intensity")[0],parameter("emissive_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0]);
        staged.lightConstants[5]=float4_t(parameter("specular_power")[0],parameter("opacity_intensity")[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0]);
        staged.lightTextureMask=5u;
    }
    else if(family=="source.map.translucent-47.v1")
    {
        staged.program=47u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(append(parameter("05.specmap_uvscale.x"),parameter("06.specmap_uvscale.y"),1u));
        staged.baseConstants[3]=vector(parameter("09.specmap_color"));
        staged.baseConstants[4]=vector(parameter("93.emissiion_color"));
        staged.baseConstants[5]=float4_t(divide(Value{1.f,0.f,0.f,0.f},maximumValue(Value{9.99999975e-06f,0.f,0.f,0.f},parameter("06.radius")))[0],parameter("07.hardness")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("07.hardness"))[0],maximumValue(subtract(Value{1.f,0.f,0.f,0.f},parameter("07.hardness")),Value{9.99999975e-06f,0.f,0.f,0.f})[0]);
        staged.baseConstants[6]=float4_t(divide(Value{1.f,0.f,0.f,0.f},maximumValue(subtract(Value{1.f,0.f,0.f,0.f},parameter("07.hardness")),Value{9.99999975e-06f,0.f,0.f,0.f}))[0],parameter("02.radius")[0],maximumValue(Value{9.99999975e-06f,0.f,0.f,0.f},parameter("02.radius"))[0],divide(Value{1.f,0.f,0.f,0.f},maximumValue(Value{9.99999975e-06f,0.f,0.f,0.f},parameter("02.radius")))[0]);
        staged.baseConstants[7]=float4_t(parameter("03.hardness")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("03.hardness"))[0],maximumValue(subtract(Value{1.f,0.f,0.f,0.f},parameter("03.hardness")),Value{9.99999975e-06f,0.f,0.f,0.f})[0],divide(Value{1.f,0.f,0.f,0.f},maximumValue(subtract(Value{1.f,0.f,0.f,0.f},parameter("03.hardness")),Value{9.99999975e-06f,0.f,0.f,0.f}))[0]);
        staged.baseConstants[8]=float4_t(parameter("09.power")[0],parameter("time")[0],Value{}[0],parameter("05.map_e_panning_x")[0]);
        staged.baseConstants[9]=float4_t(parameter("03.map_e_uvscale_r")[0],parameter("04.map_e_uvscale_g")[0],parameter("11.uv.curvature")[0],parameter("06.map_e_panning_y")[0]);
        staged.baseConstants[10]=float4_t(parameter("03.distortionrate")[0],parameter("02.specmap_str")[0],parameter("07.desaturation")[0],parameter("08.specmap_power")[0]);
        staged.baseConstants[11]=float4_t(parameter("91.desaturation")[0],parameter("92.emissiion_power")[0],parameter("95.str")[0],parameter("96.power")[0]);
        staged.baseConstants[12]=float4_t(parameter("01.depthbiasdalpha_bias")[0],0.f,0.f,0.f);
        staged.baseTextureMask=3u;
    }
    else if(family=="source.map.translucent-48.v1")
    {
        staged.program=48u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(parameter("reflection_color"));
        staged.baseConstants[3]=vector(parameter("diffuse_color"));
        staged.baseConstants[4]=float4_t(parameter("normal_intensity")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[5]=float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("opacity_intensity")[0],0.f);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(parameter("reflection_color"));
        staged.baseConstants[35]=vector(parameter("diffuse_color"));
        staged.baseConstants[36]=vector(parameter("specular_color"));
        staged.baseConstants[37]=float4_t(parameter("normal_intensity")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[38]=float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("opacity_intensity")[0],0.f);
        staged.baseTextureMask=15u;
        staged.lightConstants[1]=vector(parameter("reflection_color"));
        staged.lightConstants[2]=vector(parameter("diffuse_color"));
        staged.lightConstants[3]=vector(parameter("specular_color"));
        staged.lightConstants[4]=float4_t(parameter("normal_intensity")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0]);
        staged.lightConstants[5]=float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("opacity_intensity")[0],0.f);
        staged.lightTextureMask=15u;
    }
    else if(family=="source.map.translucent-49.v1")
    {
        staged.program=49u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(parameter("diffuse_color"));
        staged.baseConstants[3]=vector(parameter("overlay_color"));
        staged.baseConstants[4]=float4_t(parameter("normal_intensity")[0],parameter("overlay_tiling")[0],parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.baseConstants[5]=float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.baseConstants[6]=float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],parameter("min_opacity")[0]);
        staged.baseConstants[7]=float4_t(parameter("occludedopacity")[0],0.f,0.f,0.f);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(parameter("diffuse_color"));
        staged.baseConstants[35]=vector(parameter("overlay_color"));
        staged.baseConstants[36]=vector(parameter("specular_color"));
        staged.baseConstants[37]=float4_t(parameter("normal_intensity")[0],parameter("overlay_tiling")[0],parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.baseConstants[38]=float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.baseConstants[39]=float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],parameter("min_opacity")[0]);
        staged.baseConstants[40]=float4_t(parameter("occludedopacity")[0],0.f,0.f,0.f);
        staged.baseTextureMask=15u;
        staged.lightConstants[1]=vector(parameter("diffuse_color"));
        staged.lightConstants[2]=vector(parameter("overlay_color"));
        staged.lightConstants[3]=vector(parameter("specular_color"));
        staged.lightConstants[4]=float4_t(parameter("normal_intensity")[0],parameter("overlay_tiling")[0],parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.lightConstants[5]=float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.lightConstants[6]=float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],parameter("min_opacity")[0]);
        staged.lightConstants[7]=float4_t(parameter("occludedopacity")[0],0.f,0.f,0.f);
        staged.lightTextureMask=15u;
    }
    else if(family=="source.map.translucent-50.v1")
    {
        staged.program=50u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[3]=vector(parameter("diffuse_color"));
        staged.baseConstants[4]=vector(parameter("overlay_color"));
        staged.baseConstants[5]=float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[6]=float4_t(parameter("overlay_tiling")[0],parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[7]=float4_t(parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("specular_intensity")[0]);
        staged.baseConstants[8]=float4_t(parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],parameter("min_opacity")[0],parameter("occludedopacity")[0]);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[35]=vector(parameter("diffuse_color"));
        staged.baseConstants[36]=vector(parameter("overlay_color"));
        staged.baseConstants[37]=vector(parameter("specular_color"));
        staged.baseConstants[38]=float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[39]=float4_t(parameter("overlay_tiling")[0],parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[40]=float4_t(parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("specular_intensity")[0]);
        staged.baseConstants[41]=float4_t(parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],parameter("min_opacity")[0],parameter("occludedopacity")[0]);
        staged.baseTextureMask=15u;
        staged.lightConstants[1]=vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.lightConstants[2]=vector(parameter("diffuse_color"));
        staged.lightConstants[3]=vector(parameter("overlay_color"));
        staged.lightConstants[4]=vector(parameter("specular_color"));
        staged.lightConstants[5]=float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.lightConstants[6]=float4_t(parameter("overlay_tiling")[0],parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0],parameter("diffuse_brightness")[0]);
        staged.lightConstants[7]=float4_t(parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0],parameter("specular_intensity")[0]);
        staged.lightConstants[8]=float4_t(parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],parameter("min_opacity")[0],parameter("occludedopacity")[0]);
        staged.lightTextureMask=15u;
    }
    else if(family=="source.map.translucent-51.v1")
    {
        staged.program=51u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(parameter("diffuse_color"));
        staged.baseConstants[3]=vector(parameter("reflection_color"));
        staged.baseConstants[4]=float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("reflection_intensity")[0],parameter("specular_intensity")[0]);
        staged.baseConstants[5]=float4_t(parameter("specular_power")[0],parameter("opacity_intensity")[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0]);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(parameter("diffuse_color"));
        staged.baseConstants[35]=vector(parameter("reflection_color"));
        staged.baseConstants[36]=vector(parameter("specular_color"));
        staged.baseConstants[37]=float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("reflection_intensity")[0],parameter("specular_intensity")[0]);
        staged.baseConstants[38]=float4_t(parameter("specular_power")[0],parameter("opacity_intensity")[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0]);
        staged.baseTextureMask=7u;
        staged.lightConstants[1]=vector(parameter("diffuse_color"));
        staged.lightConstants[2]=vector(parameter("reflection_color"));
        staged.lightConstants[3]=vector(parameter("specular_color"));
        staged.lightConstants[4]=vector(parameter("rimlight_color"));
        staged.lightConstants[5]=float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("reflection_intensity")[0],parameter("specular_intensity")[0]);
        staged.lightConstants[6]=float4_t(parameter("specular_power")[0],parameter("opacity_intensity")[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0]);
        staged.lightTextureMask=7u;
    }
    else if(family=="source.map.translucent-52.v1")
    {
        staged.program=52u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[3]=vector(parameter("reflection_color"));
        staged.baseConstants[4]=vector(parameter("diffuse_color"));
        staged.baseConstants[5]=float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[6]=float4_t(parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0]);
        staged.baseConstants[7]=float4_t(parameter("specular_power")[0],parameter("opacity_intensity")[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0]);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[35]=vector(parameter("reflection_color"));
        staged.baseConstants[36]=vector(parameter("diffuse_color"));
        staged.baseConstants[37]=vector(parameter("specular_color"));
        staged.baseConstants[38]=float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[39]=float4_t(parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0]);
        staged.baseConstants[40]=float4_t(parameter("specular_power")[0],parameter("opacity_intensity")[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0]);
        staged.baseTextureMask=7u;
        staged.lightConstants[1]=vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.lightConstants[2]=vector(parameter("reflection_color"));
        staged.lightConstants[3]=vector(parameter("diffuse_color"));
        staged.lightConstants[4]=vector(parameter("specular_color"));
        staged.lightConstants[5]=vector(parameter("rimlight_color"));
        staged.lightConstants[6]=float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.lightConstants[7]=float4_t(parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0]);
        staged.lightConstants[8]=float4_t(parameter("specular_power")[0],parameter("opacity_intensity")[0],parameter("rimlight_intensity")[0],parameter("rimlight_power")[0]);
        staged.lightTextureMask=7u;
    }
    else if(family=="source.map.translucent-53.v1")
    {
        staged.program=53u;
        staged.baseConstants[2]=vector(parameter("selectioncolor"));
        staged.baseConstants[3]=vector(parameter("93.emissiion_color"));
        staged.baseConstants[4]=vector(parameter("meshemitterdynamicparameter"));
        staged.baseConstants[5]=vector(append(parameter("21.uvscale.x"),parameter("22.uvscale.y"),1u));
        staged.baseConstants[6]=float4_t(parameter("time")[0],Value{}[0],parameter("05.map_e_panning_x")[0],parameter("03.map_e_uvscale_r")[0]);
        staged.baseConstants[7]=float4_t(parameter("04.map_e_uvscale_g")[0],parameter("09.map_d_panning_x")[0],parameter("07.map_d_uvscale_r")[0],parameter("08.map_d_uvscale_g")[0]);
        staged.baseConstants[8]=float4_t(parameter("10.map_d_panning_y")[0],parameter("05.distort_str")[0],multiply(Value{1.f,0.f,0.f,0.f},parameter("05.distort_str"))[0],parameter("06.map_e_panning_y")[0]);
        staged.baseConstants[9]=float4_t(parameter("15.map_f_panning_x")[0],parameter("13.map_f_uvscale_r")[0],parameter("14.map_f_uvscale_g")[0],parameter("16.map_f_panning_y")[0]);
        staged.baseConstants[10]=float4_t(parameter("91.desaturation")[0],parameter("92.emissiion_power")[0],parameter("29.cmap.direct")[0],parameter("30.cmap.time(rotrate)")[0]);
        staged.baseConstants[11]=float4_t(multiply(Value{-0.523599029f,0.f,0.f,0.f},parameter("29.cmap.direct"))[0],parameter("22.uvscale.y")[0],parameter("21.uvscale.x")[0],parameter("36.str")[0]);
        staged.baseConstants[12]=float4_t(parameter("37.power")[0],parameter("01.depthbiasdalpha_bias")[0],parameter("32.fresnal_power")[0],parameter("33.fresnal_str")[0]);
        staged.baseTextureMask=15u;
    }
    else if(family=="source.map.translucent-54.v1")
    {
        staged.program=54u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(parameter("diffuse_color"));
        staged.baseConstants[3]=float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.baseConstants[4]=float4_t(parameter("opacity_intensity")[0],parameter("min_opacity")[0],parameter("occludedopacity")[0],0.f);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(parameter("diffuse_color"));
        staged.baseConstants[35]=vector(parameter("specular_color"));
        staged.baseConstants[36]=float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.baseConstants[37]=float4_t(parameter("opacity_intensity")[0],parameter("min_opacity")[0],parameter("occludedopacity")[0],0.f);
        staged.baseTextureMask=7u;
        staged.lightConstants[1]=vector(parameter("diffuse_color"));
        staged.lightConstants[2]=vector(parameter("specular_color"));
        staged.lightConstants[3]=float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.lightConstants[4]=float4_t(parameter("opacity_intensity")[0],parameter("min_opacity")[0],parameter("occludedopacity")[0],0.f);
        staged.lightTextureMask=7u;
    }
    else if(family=="source.map.translucent-55.v1")
    {
        staged.program=55u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(parameter("93.emissiion_color"));
        staged.baseConstants[3]=vector(append(parameter("21.uvscale.x"),parameter("22.uvscale.y"),1u));
        staged.baseConstants[4]=float4_t(parameter("time")[0],Value{}[0],parameter("05.map_e_panning_x")[0],parameter("03.map_e_uvscale_r")[0]);
        staged.baseConstants[5]=float4_t(parameter("04.map_e_uvscale_g")[0],parameter("06.map_e_panning_y")[0],parameter("15.map_f_panning_x")[0],parameter("13.map_f_uvscale_r")[0]);
        staged.baseConstants[6]=float4_t(parameter("14.map_f_uvscale_g")[0],parameter("16.map_f_panning_y")[0],parameter("91.desaturation")[0],parameter("92.emissiion_power")[0]);
        staged.baseConstants[7]=float4_t(parameter("29.cmap.direct")[0],parameter("30.cmap.time(rotrate)")[0],multiply(Value{-0.523599029f,0.f,0.f,0.f},parameter("29.cmap.direct"))[0],parameter("22.uvscale.y")[0]);
        staged.baseConstants[8]=float4_t(parameter("21.uvscale.x")[0],parameter("36.str")[0],parameter("37.power")[0],0.f);
        staged.baseTextureMask=7u;
    }
    else if(family=="source.map.translucent-56.v1")
    {
        staged.program=56u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(parameter("diffuse_color"));
        staged.baseConstants[3]=float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.baseConstants[4]=float4_t(parameter("min_opacity")[0],parameter("occludedopacity")[0],0.f,0.f);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(parameter("diffuse_color"));
        staged.baseConstants[35]=vector(parameter("specular_color"));
        staged.baseConstants[36]=float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.baseConstants[37]=float4_t(parameter("min_opacity")[0],parameter("occludedopacity")[0],0.f,0.f);
        staged.baseTextureMask=3u;
        staged.lightConstants[1]=vector(parameter("diffuse_color"));
        staged.lightConstants[2]=vector(parameter("specular_color"));
        staged.lightConstants[3]=float4_t(parameter("normal_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0]);
        staged.lightConstants[4]=float4_t(parameter("min_opacity")[0],parameter("occludedopacity")[0],0.f,0.f);
        staged.lightTextureMask=3u;
    }
    else if(family=="source.map.translucent-57.v1")
    {
        staged.program=57u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(parameter("diffuse_color"));
        staged.baseConstants[3]=vector(parameter("overlay_color"));
        staged.baseConstants[4]=float4_t(parameter("normal_intensity")[0],parameter("overlay_tiling")[0],parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.baseConstants[5]=float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.baseConstants[6]=float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],parameter("min_opacity")[0]);
        staged.baseConstants[7]=float4_t(parameter("occludedopacity")[0],0.f,0.f,0.f);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(parameter("diffuse_color"));
        staged.baseConstants[35]=vector(parameter("overlay_color"));
        staged.baseConstants[36]=vector(parameter("specular_color"));
        staged.baseConstants[37]=float4_t(parameter("normal_intensity")[0],parameter("overlay_tiling")[0],parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.baseConstants[38]=float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.baseConstants[39]=float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],parameter("min_opacity")[0]);
        staged.baseConstants[40]=float4_t(parameter("occludedopacity")[0],0.f,0.f,0.f);
        staged.baseTextureMask=31u;
        staged.lightConstants[1]=vector(parameter("diffuse_color"));
        staged.lightConstants[2]=vector(parameter("overlay_color"));
        staged.lightConstants[3]=vector(parameter("specular_color"));
        staged.lightConstants[4]=float4_t(parameter("normal_intensity")[0],parameter("overlay_tiling")[0],parameter("overlay_normal_intensity")[0],parameter("overlay_sharpeness")[0]);
        staged.lightConstants[5]=float4_t(parameter("diffuse_brightness")[0],parameter("overlay_brightness")[0],parameter("overlay_saturation")[0],subtract(Value{1.f,0.f,0.f,0.f},parameter("overlay_saturation"))[0]);
        staged.lightConstants[6]=float4_t(parameter("specular_intensity")[0],parameter("overlay_specular_intensity")[0],parameter("specular_power")[0],parameter("min_opacity")[0]);
        staged.lightConstants[7]=float4_t(parameter("occludedopacity")[0],0.f,0.f,0.f);
        staged.lightTextureMask=31u;
    }
    else if(family=="source.map.translucent-58.v1")
    {
        staged.program=58u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(parameter("specular_color"));
        staged.baseConstants[3]=vector(parameter("diffuse_color"));
        staged.baseConstants[4]=float4_t(parameter("normal_intensity")[0],parameter("specular_intensity")[0],parameter("subspecular_intensity")[0],parameter("subspecular_power")[0]);
        staged.baseConstants[5]=float4_t(parameter("diffuse_brightness")[0],parameter("specular_power")[0],parameter("min_opacity")[0],parameter("occludedopacity")[0]);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(parameter("specular_color"));
        staged.baseConstants[35]=vector(parameter("diffuse_color"));
        staged.baseConstants[36]=float4_t(parameter("normal_intensity")[0],parameter("specular_intensity")[0],parameter("subspecular_intensity")[0],parameter("subspecular_power")[0]);
        staged.baseConstants[37]=float4_t(parameter("diffuse_brightness")[0],parameter("specular_power")[0],parameter("min_opacity")[0],parameter("occludedopacity")[0]);
        staged.baseTextureMask=3u;
        staged.lightConstants[1]=vector(parameter("specular_color"));
        staged.lightConstants[2]=vector(parameter("diffuse_color"));
        staged.lightConstants[3]=float4_t(parameter("normal_intensity")[0],parameter("specular_intensity")[0],parameter("subspecular_intensity")[0],parameter("subspecular_power")[0]);
        staged.lightConstants[4]=float4_t(parameter("diffuse_brightness")[0],parameter("specular_power")[0],parameter("min_opacity")[0],parameter("occludedopacity")[0]);
        staged.lightTextureMask=3u;
    }
    else if(family=="source.map.translucent-59.v1")
    {
        staged.program=59u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(append(trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[3]=vector(append(trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false),trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[4]=vector(append(parameter("layer01_tiling"),multiply(parameter("layer01_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[5]=vector(parameter("layer01_color"));
        staged.baseConstants[6]=vector(parameter("layer01_specular_color"));
        staged.baseConstants[7]=float4_t(parameter("normal_intensity")[0],parameter("layer01_rotation")[0],multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f})[0],trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[8]=float4_t(multiply(parameter("layer01_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("layer01_desaturation")[0],parameter("layer01_brightness")[0],parameter("layer01_specular_inensity")[0]);
        staged.baseConstants[9]=float4_t(parameter("layer01_specular_power")[0],parameter("depthbias")[0],0.f,0.f);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(append(trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[35]=vector(append(trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false),trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[36]=vector(append(parameter("layer01_tiling"),multiply(parameter("layer01_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[37]=vector(parameter("layer01_color"));
        staged.baseConstants[38]=vector(parameter("layer01_specular_color"));
        staged.baseConstants[39]=float4_t(parameter("normal_intensity")[0],parameter("layer01_rotation")[0],multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f})[0],trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false)[0]);
        staged.baseConstants[40]=float4_t(multiply(parameter("layer01_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("layer01_desaturation")[0],parameter("layer01_brightness")[0],parameter("layer01_specular_inensity")[0]);
        staged.baseConstants[41]=float4_t(parameter("layer01_specular_power")[0],parameter("depthbias")[0],0.f,0.f);
        staged.baseTextureMask=7u;
        staged.lightConstants[1]=vector(parameter("selectioncolor"));
        staged.lightConstants[2]=vector(append(trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[3]=vector(append(trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false),trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[4]=vector(append(parameter("layer01_tiling"),multiply(parameter("layer01_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.lightConstants[5]=vector(parameter("layer01_color"));
        staged.lightConstants[6]=vector(parameter("layer01_specular_color"));
        staged.lightConstants[7]=float4_t(parameter("normal_intensity")[0],parameter("layer01_rotation")[0],multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f})[0],trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false)[0]);
        staged.lightConstants[8]=float4_t(multiply(parameter("layer01_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("layer01_desaturation")[0],parameter("layer01_brightness")[0],parameter("layer01_specular_inensity")[0]);
        staged.lightConstants[9]=float4_t(parameter("layer01_specular_power")[0],parameter("depthbias")[0],0.f,0.f);
        staged.lightTextureMask=7u;
    }
    else if(family=="source.map.translucent-60.v1")
    {
        staged.program=60u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(parameter("reflection_color"));
        staged.baseConstants[3]=vector(parameter("diffuse_color"));
        staged.baseConstants[4]=float4_t(parameter("normal_intensity")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0]);
        staged.baseConstants[5]=float4_t(parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("opacity_intensity")[0]);
        staged.baseConstants[6]=float4_t(parameter("min_opacity")[0],parameter("occludedopacity")[0],0.f,0.f);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(parameter("reflection_color"));
        staged.baseConstants[35]=vector(parameter("diffuse_color"));
        staged.baseConstants[36]=vector(parameter("specular_color"));
        staged.baseConstants[37]=float4_t(parameter("normal_intensity")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0]);
        staged.baseConstants[38]=float4_t(parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("opacity_intensity")[0]);
        staged.baseConstants[39]=float4_t(parameter("min_opacity")[0],parameter("occludedopacity")[0],0.f,0.f);
        staged.baseTextureMask=15u;
        staged.lightConstants[1]=vector(parameter("reflection_color"));
        staged.lightConstants[2]=vector(parameter("diffuse_color"));
        staged.lightConstants[3]=vector(parameter("specular_color"));
        staged.lightConstants[4]=float4_t(parameter("normal_intensity")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0]);
        staged.lightConstants[5]=float4_t(parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("opacity_intensity")[0]);
        staged.lightConstants[6]=float4_t(parameter("min_opacity")[0],parameter("occludedopacity")[0],0.f,0.f);
        staged.lightTextureMask=15u;
    }
    else if(family=="source.map.translucent-61.v1")
    {
        staged.program=61u;
        staged.requiredExtraUVMask=2u; // Native sky consumes TEXCOORD2.
        staged.baseConstants[0]=vector(parameter("selectioncolor"));
        staged.baseConstants[1]=vector(append(periodic(multiply(Value{},Value{0.0199999996f,0.f,0.f,0.f})),periodic(multiply(Value{},Value{0.0199999996f,0.f,0.f,0.f})),1u));
        staged.baseConstants[2]=vector(parameter("starcolor"));
        staged.baseConstants[3]=vector(parameter("zenithcolor"));
        staged.baseConstants[4]=vector(parameter("horizoncolor"));
        staged.baseConstants[5]=vector(append(periodic(multiply(multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})),Value{0.00100000005f,0.f,0.f,0.f})),periodic(multiply(multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})),Value{0.f,0.f,0.f,0.f})),1u));
        staged.baseConstants[6]=vector(append(periodic(multiply(multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})),Value{-0.00200000009f,0.f,0.f,0.f})),periodic(multiply(multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})),Value{0.f,0.f,0.f,0.f})),1u));
        staged.baseConstants[7]=vector(append(periodic(multiply(multiply(parameter("speed"),Value{}),Value{-0.0199999996f,0.f,0.f,0.f})),periodic(multiply(multiply(parameter("speed"),Value{}),Value{0.f,0.f,0.f,0.f})),1u));
        staged.baseConstants[8]=float4_t(Value{}[0],multiply(Value{},Value{0.0199999996f,0.f,0.f,0.f})[0],parameter("startiling")[0],periodic(multiply(Value{},Value{0.0199999996f,0.f,0.f,0.f}))[0]);
        staged.baseConstants[9]=float4_t(parameter("desaturation")[0],parameter("speed")[0],multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})[0],multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f}))[0]);
        staged.baseConstants[10]=float4_t(multiply(multiply(parameter("speed"),Value{}),Value{0.f,0.f,0.f,0.f})[0],periodic(multiply(multiply(parameter("speed"),Value{}),Value{0.f,0.f,0.f,0.f}))[0],periodic(multiply(multiply(parameter("speed"),Value{}),Value{-0.0199999996f,0.f,0.f,0.f}))[0],parameter("cloudopacity")[0]);
        staged.baseConstants[11]=float4_t(parameter("skybrightness")[0],parameter("cloudbrightness")[0],parameter("clouddarkness")[0],parameter("rimbrightness")[0]);
        staged.baseConstants[32]=vector(parameter("selectioncolor"));
        staged.baseConstants[33]=vector(append(periodic(multiply(Value{},Value{0.0199999996f,0.f,0.f,0.f})),periodic(multiply(Value{},Value{0.0199999996f,0.f,0.f,0.f})),1u));
        staged.baseConstants[34]=vector(parameter("starcolor"));
        staged.baseConstants[35]=vector(parameter("zenithcolor"));
        staged.baseConstants[36]=vector(parameter("horizoncolor"));
        staged.baseConstants[37]=vector(append(periodic(multiply(multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})),Value{0.00100000005f,0.f,0.f,0.f})),periodic(multiply(multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})),Value{0.f,0.f,0.f,0.f})),1u));
        staged.baseConstants[38]=vector(append(periodic(multiply(multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})),Value{-0.00200000009f,0.f,0.f,0.f})),periodic(multiply(multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})),Value{0.f,0.f,0.f,0.f})),1u));
        staged.baseConstants[39]=vector(append(periodic(multiply(multiply(parameter("speed"),Value{}),Value{-0.0199999996f,0.f,0.f,0.f})),periodic(multiply(multiply(parameter("speed"),Value{}),Value{0.f,0.f,0.f,0.f})),1u));
        staged.baseConstants[40]=float4_t(Value{}[0],multiply(Value{},Value{0.0199999996f,0.f,0.f,0.f})[0],parameter("startiling")[0],periodic(multiply(Value{},Value{0.0199999996f,0.f,0.f,0.f}))[0]);
        staged.baseConstants[41]=float4_t(parameter("desaturation")[0],parameter("speed")[0],multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})[0],multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f}))[0]);
        staged.baseConstants[42]=float4_t(multiply(multiply(parameter("speed"),Value{}),Value{0.f,0.f,0.f,0.f})[0],periodic(multiply(multiply(parameter("speed"),Value{}),Value{0.f,0.f,0.f,0.f}))[0],periodic(multiply(multiply(parameter("speed"),Value{}),Value{-0.0199999996f,0.f,0.f,0.f}))[0],parameter("cloudopacity")[0]);
        staged.baseConstants[43]=float4_t(parameter("skybrightness")[0],parameter("cloudbrightness")[0],parameter("clouddarkness")[0],parameter("rimbrightness")[0]);
        staged.baseConstants[60]=vector(parameter("speed"));
        staged.baseTextureMask=3u;
        staged.lightConstants[0]=vector(append(periodic(multiply(multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})),Value{0.00100000005f,0.f,0.f,0.f})),periodic(multiply(multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})),Value{0.f,0.f,0.f,0.f})),1u));
        staged.lightConstants[1]=vector(append(periodic(multiply(multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})),Value{-0.00200000009f,0.f,0.f,0.f})),periodic(multiply(multiply(Value{},multiply(parameter("speed"),Value{4.f,0.f,0.f,0.f})),Value{0.f,0.f,0.f,0.f})),1u));
        staged.lightConstants[2]=vector(append(periodic(multiply(multiply(parameter("speed"),Value{}),Value{-0.0199999996f,0.f,0.f,0.f})),periodic(multiply(multiply(parameter("speed"),Value{}),Value{0.f,0.f,0.f,0.f})),1u));
        staged.lightConstants[3]=vector(parameter("rimcolor"));
        staged.lightConstants[4]=vector(parameter("sun"));
        staged.lightConstants[5]=float4_t(multiply(multiply(parameter("speed"),Value{}),Value{0.f,0.f,0.f,0.f})[0],periodic(multiply(multiply(parameter("speed"),Value{}),Value{0.f,0.f,0.f,0.f}))[0],periodic(multiply(multiply(parameter("speed"),Value{}),Value{-0.0199999996f,0.f,0.f,0.f}))[0],parameter("cloudopacity")[0]);
        staged.lightConstants[6]=float4_t(parameter("skybrightness")[0],parameter("cloudbrightness")[0],parameter("clouddarkness")[0],parameter("rimbrightness")[0]);
        staged.lightConstants[60]=vector(parameter("speed"));
        staged.lightTextureMask=6u;
    }
    else if(family=="source.map.translucent-62.v1")
    {
        staged.program=62u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(append(trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[3]=vector(append(trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false),trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[4]=vector(append(parameter("layer01_tiling"),multiply(parameter("layer01_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[5]=vector(parameter("layer01_color"));
        staged.baseConstants[6]=vector(parameter("layer01_specular_color"));
        staged.baseConstants[7]=float4_t(parameter("layer01_desaturation")[0],parameter("layer01_brightness")[0],parameter("layer01_specular_inensity")[0],parameter("layer01_specular_power")[0]);
        staged.baseConstants[8]=float4_t(parameter("depthbias")[0],0.f,0.f,0.f);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(append(trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false)),1u));
        staged.baseConstants[35]=vector(append(trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false),trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),true),1u));
        staged.baseConstants[36]=vector(append(parameter("layer01_tiling"),multiply(parameter("layer01_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[37]=vector(parameter("layer01_color"));
        staged.baseConstants[38]=vector(parameter("layer01_specular_color"));
        staged.baseConstants[39]=float4_t(parameter("layer01_desaturation")[0],parameter("layer01_brightness")[0],parameter("layer01_specular_inensity")[0],parameter("layer01_specular_power")[0]);
        staged.baseConstants[40]=float4_t(parameter("depthbias")[0],0.f,0.f,0.f);
        staged.baseTextureMask=3u;
        staged.lightConstants[1]=vector(parameter("selectioncolor"));
        staged.lightConstants[2]=vector(append(trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),true),multiply(Value{-1.f,0.f,0.f,0.f},trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false)),1u));
        staged.lightConstants[3]=vector(append(trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),false),trigonometric(multiply(parameter("layer01_rotation"),Value{3.1400001f,0.f,0.f,0.f}),true),1u));
        staged.lightConstants[4]=vector(append(parameter("layer01_tiling"),multiply(parameter("layer01_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.lightConstants[5]=vector(parameter("layer01_color"));
        staged.lightConstants[6]=vector(parameter("layer01_specular_color"));
        staged.lightConstants[7]=float4_t(parameter("layer01_desaturation")[0],parameter("layer01_brightness")[0],parameter("layer01_specular_inensity")[0],parameter("layer01_specular_power")[0]);
        staged.lightConstants[8]=float4_t(parameter("depthbias")[0],0.f,0.f,0.f);
        staged.lightTextureMask=3u;
    }
    else if(family=="source.map.translucent-63.v1")
    {
        staged.program=63u;
        staged.baseConstants[1]=vector(parameter("selectioncolor"));
        staged.baseConstants[2]=vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[3]=vector(parameter("reflection_color"));
        staged.baseConstants[4]=vector(parameter("diffuse_color"));
        staged.baseConstants[5]=float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[6]=float4_t(parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0]);
        staged.baseConstants[7]=float4_t(parameter("specular_power")[0],parameter("opacity_intensity")[0],0.f,0.f);
        staged.baseConstants[33]=vector(parameter("selectioncolor"));
        staged.baseConstants[34]=vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[35]=vector(parameter("reflection_color"));
        staged.baseConstants[36]=vector(parameter("diffuse_color"));
        staged.baseConstants[37]=vector(parameter("specular_color"));
        staged.baseConstants[38]=float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[39]=float4_t(parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0]);
        staged.baseConstants[40]=float4_t(parameter("specular_power")[0],parameter("opacity_intensity")[0],0.f,0.f);
        staged.baseTextureMask=15u;
        staged.lightConstants[1]=vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.lightConstants[2]=vector(parameter("reflection_color"));
        staged.lightConstants[3]=vector(parameter("diffuse_color"));
        staged.lightConstants[4]=vector(parameter("specular_color"));
        staged.lightConstants[5]=float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.lightConstants[6]=float4_t(parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0],parameter("specular_intensity")[0]);
        staged.lightConstants[7]=float4_t(parameter("specular_power")[0],parameter("opacity_intensity")[0],0.f,0.f);
        staged.lightTextureMask=15u;
    }
    else return false;
    if(!valid || consumed.size()!=parameters.size())return false;
    result=staged;return true;
}
}
