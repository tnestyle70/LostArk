#pragma once

#include "DataJson.h"
#include "BinaryAsset/ModelAssetData.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <set>
#include <string>

namespace Client::SourceMapForwardMaterial
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
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS staged{};
    if (family == "source.map.spotlight.v1")
    {
        staged.program = 33u;
        staged.baseTextureMask = 1u;
        staged.baseConstants[4] = vector(parameter("selectioncolor"));
        staged.baseConstants[5] = vector(parameter("color"));
        staged.baseConstants[6] = float4_t(Value{}[0],parameter("dust_speed")[0],parameter("dust_density")[0],parameter("opacity")[0]);
    }
    else if (family == "source.map.translucent-tiled.v1")
    {
        staged.program = 34u;
        staged.baseTextureMask = 7u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(append(parameter("uv_tiling"),multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale")),1u));
        staged.baseConstants[3] = vector(parameter("reflection_color"));
        staged.baseConstants[4] = vector(parameter("diffuse_color"));
        staged.baseConstants[5] = float4_t(parameter("uv_tiling_y_scale")[0],parameter("uv_tiling")[0],multiply(parameter("uv_tiling"),parameter("uv_tiling_y_scale"))[0],parameter("normal_intensity")[0]);
        staged.baseConstants[6] = float4_t(parameter("reflection_tiling")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[7] = float4_t(parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("opacity_intensity")[0],0.f);
    }
    else if (family == "source.map.translucent-reflection.v1")
    {
        staged.program = 35u;
        staged.baseTextureMask = 7u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("reflection_color"));
        staged.baseConstants[3] = vector(parameter("diffuse_color"));
        staged.baseConstants[4] = float4_t(parameter("normal_intensity")[0],parameter("reflection_tiling")[0],parameter("reflection_contrast")[0],parameter("reflection_intensity")[0]);
        staged.baseConstants[5] = float4_t(parameter("diffuse_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("opacity_intensity")[0]);
    }
    else if (family == "source.map.translucent-bump.v1")
    {
        staged.program = 36u;
        staged.baseTextureMask = 3u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("diffuse_color"));
        staged.baseConstants[3] = float4_t(parameter("bump_offset")[0],parameter("bump_intensity")[0],parameter("normal_intensity")[0],parameter("diffuse_brightness")[0]);
        staged.baseConstants[4] = float4_t(parameter("bump_brightness")[0],parameter("specular_intensity")[0],parameter("specular_power")[0],parameter("opacity_intensity")[0]);
    }
    else if (family == "source.map.sky-simple.v1")
    {
        staged.program = 37u;
        staged.baseTextureMask = 3u;
        staged.baseConstants[1] = vector(parameter("selectioncolor"));
        staged.baseConstants[2] = vector(parameter("diffuse_color"));
        staged.baseConstants[3] = vector(parameter("opposite_light_color"));
        staged.baseConstants[4] = vector(parameter("light_inscattering_color"));
        staged.baseConstants[5] = vector(parameter("cloud_color"));
        staged.baseConstants[6] = vector(append(Value{1.f,0.f,0.f,0.f},parameter("cloud_height"),1u));
        staged.baseConstants[7] = vector(append(periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.00800000038f,0.f,0.f,0.f})),periodic(multiply(multiply(parameter("cloud_speed"),Value{}),Value{0.f,0.f,0.f,0.f})),1u));
        staged.baseConstants[8] = float4_t(parameter("fog_height")[0],bounded(parameter("fog_height"),Value{0.00100000005f,0.f,0.f,0.f},Value{0.999000013f,0.f,0.f,0.f})[0],subtract(Value{1.f,0.f,0.f,0.f},bounded(parameter("fog_height"),Value{0.00100000005f,0.f,0.f,0.f},Value{0.999000013f,0.f,0.f,0.f}))[0],parameter("opposite_light_intensity")[0]);
        staged.baseConstants[9] = float4_t(parameter("light_inscattering_intensity")[0],parameter("fog_opacity")[0],parameter("lightterminatorangle")[0],subtract(Value{180.f,0.f,0.f,0.f},parameter("lightterminatorangle"))[0]);
        staged.baseConstants[10] = float4_t(parameter("cloud_opacity")[0],parameter("base_brightness")[0],bounded(subtract(Value{180.f,0.f,0.f,0.f},parameter("lightterminatorangle")),Value{0.f,0.f,0.f,0.f},Value{180.f,0.f,0.f,0.f})[0],multiply(bounded(subtract(Value{180.f,0.f,0.f,0.f},parameter("lightterminatorangle")),Value{0.f,0.f,0.f,0.f},Value{180.f,0.f,0.f,0.f}),Value{0.00555599993f,0.f,0.f,0.f})[0]);
        staged.baseConstants[11] = float4_t(subtract(Value{1.f,0.f,0.f,0.f},multiply(bounded(subtract(Value{180.f,0.f,0.f,0.f},parameter("lightterminatorangle")),Value{0.f,0.f,0.f,0.f},Value{180.f,0.f,0.f,0.f}),Value{0.00555599993f,0.f,0.f,0.f}))[0],0.f,0.f,0.f);
        staged.baseConstants[63].x = parameter("cloud_speed")[0];
    }
    else if (family == "source.map.black.v1" || family == "source.map.shadow-modulate.v1")
    {
        staged.program = family == "source.map.black.v1" ? 64u : 65u;
        const auto selection = parameter("selectioncolor");
        if (selection != Value{0.f,0.f,0.f,1.f}) return false;
        if (staged.program == 65u)
        {
            if (parameter("shadow_color") != Value{0.f,0.f,0.f,1.f}) return false;
            const float bias = parameter("depth_bias")[0];
            const float power = parameter("depth_power")[0];
            if (!std::isfinite(bias) || !std::isfinite(power) || power <= 0.f) return false;
            staged.baseConstants[3] = float4_t(bias, power, 0.f, 0.f);
        }
    }
    else return false;
    if (!valid || consumed.size()!=parameters.size()) return false;
    result=staged;
    return true;
}
}
