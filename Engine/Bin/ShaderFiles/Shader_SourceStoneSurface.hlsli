#ifndef LOSTARK_SOURCE_STONE_SURFACE
#define LOSTARK_SOURCE_STONE_SURFACE

// bg_base_opa, actual floor01/streetfloor02 MIC permutation:
// Base 5a58f84d..., Directional 568c3274..., LightMap f01c4d07....
// Pure sampled-value math, shared by geometry and deferred light consumers.
struct SOURCE_STONE_PARAMETERS
{
    float3 diffuseColor;
    float3 overlayColor;
    float3 specularColor;
    float normalIntensity;
    float overlayNormalIntensity;
    float overlaySharpness;
    float diffuseSaturation;
    float diffuseBrightness;
    float overlayBrightness;
    float overlaySaturation;
    float specularIntensity;
    float overlaySpecularIntensity;
    float specularPower;
};

struct SOURCE_STONE_SURFACE
{
    float3 diffuse;
    float3 specular;
    float3 baseNormal;
    float3 directMixedNormal;
    float3 bakedMixedNormal;
    float overlayWeight;
};

struct SOURCE_STONE_ENGINE_INPUTS
{
    float3 hemisphereUpper;
    float3 hemisphereLower;
    float hemisphereScale;
    float3 ambient;
    float3 selection;
    float4 diffuseTransform; // RGB additive, W multiplier (native CB2[3])
    float4 specularTransform; // RGB additive, W multiplier (native CB2[4])
};

struct SOURCE_STONE_BASE_LIGHT
{
    float3 radiance;
    float3 diffuse;
    float3 bakedIrradiance;
    float3 bakedSpecular;
};

SOURCE_STONE_ENGINE_INPUTS SourceStoneInactiveEngineInputs()
{
    // PROJECT adapter: these engine-owned source scene constants have not
    // been identified. Their identities do not claim a recovered scene tint.
    SOURCE_STONE_ENGINE_INPUTS result = (SOURCE_STONE_ENGINE_INPUTS)0;
    result.diffuseTransform.w = 1.f;
    result.specularTransform.w = 1.f;
    return result;
}

float3 SourceStoneUnit(float3 value)
{
    // Runtime degenerate-direction guard; ordinary source unit vectors retain
    // the native normalize expression. Do not normalize directMixedNormal.
    return value * rsqrt(max(dot(value, value), 1e-30f));
}

float SourceStoneLuminance(float3 value)
{
    return dot(value, float3(0.3f, 0.59f, 0.11f));
}

float3 SourceStoneSaturation(float3 value, float saturation)
{
    return lerp(SourceStoneLuminance(value).xxx, value, saturation);
}

float SourceStoneOverlayWeight(float vertexRed, float height, float sharpness)
{
    const float sharp = clamp(sharpness, 0.f, 0.99f);
    const float a = (1.f - vertexRed) * (1.f + height * sharp);
    const float b = (a - sharp) / (1.f - sharp);
    return saturate(height * (a - b) + b);
}

SOURCE_STONE_SURFACE EvaluateSourceStoneSurface(float4 diffuseSample,
    float4 normalSample, float4 overlayDiffuseSample, float4 overlayNormalSample,
    float4 sourceVertexColor, SOURCE_STONE_PARAMETERS material)
{
    SOURCE_STONE_SURFACE result;
    const float2 xy = normalSample.rg * 2.f - 1.f;
    const float z = sqrt(max(1.f - dot(xy, xy), 0.f)) + 0.00001f;
    result.baseNormal = SourceStoneUnit(float3(xy * material.normalIntensity *
        sourceVertexColor.a, z));
    const float2 overlayXY = overlayNormalSample.rg * 2.f - 1.f;
    const float3 overlayNormal = float3(overlayXY * material.overlayNormalIntensity,
        sqrt(max(1.f - dot(overlayXY, overlayXY), 0.f)) + 0.00001f);

    // All THREE selected passes use normalized base Z squared. Base/Baked
    // overwrite r0.w at `div r0.yzw` before `mul r1.x,r0.w,r0.w`.
    // It is not the earlier raw XY length still occupying that register.
    const float height = overlayDiffuseSample.a * overlayDiffuseSample.a *
        (1.f - saturate(result.baseNormal.z * result.baseNormal.z * diffuseSample.a));
    result.overlayWeight = SourceStoneOverlayWeight(sourceVertexColor.r,
        height, material.overlaySharpness);
    result.directMixedNormal = lerp(result.baseNormal, overlayNormal,
        result.overlayWeight * 0.65f);
    result.bakedMixedNormal = SourceStoneUnit(result.directMixedNormal);

    const float3 baseDiffuse = SourceStoneSaturation(diffuseSample.rgb,
        material.diffuseSaturation) * material.diffuseColor * material.diffuseBrightness;
    // Overlay tint/brightness precede saturation in the native graph.
    const float3 overlayDiffuse = SourceStoneSaturation(overlayDiffuseSample.rgb *
        material.overlayColor * material.overlayBrightness, material.overlaySaturation);
    result.diffuse = lerp(baseDiffuse, overlayDiffuse, result.overlayWeight);
    const float3 baseSpecular = diffuseSample.rgb * material.specularColor *
        material.specularIntensity;
    // This branch uses raw overlay RGB with saturation/intensity only.
    // Neither overlayColor/brightness nor specularColor tints overlay specular.
    const float3 overlaySpecular = SourceStoneSaturation(overlayDiffuseSample.rgb,
        material.overlaySaturation) * material.overlaySpecularIntensity;
    result.specular = lerp(baseSpecular, overlaySpecular, result.overlayWeight);
    return result;
}

float3 SourceStoneDirectionalWeights(float3 direction)
{
    return saturate(float3(
        dot(direction.yz, float2(0.81649658f, 0.57735027f)),
        dot(direction, float3(-0.70710678f, -0.40824829f, 0.57735027f)),
        dot(direction, float3(0.70710678f, -0.40824829f, 0.57735027f))));
}

SOURCE_STONE_BASE_LIGHT EvaluateSourceStoneBase(SOURCE_STONE_SURFACE surface,
    float3 tangentView, float3 tangentWorldUp, float3 lightmapAverage,
    float3 lightmapCoefficients, bool hasLightmap, float specularPower,
    SOURCE_STONE_ENGINE_INPUTS engine)
{
    SOURCE_STONE_BASE_LIGHT result = (SOURCE_STONE_BASE_LIGHT)0;
    result.diffuse = surface.diffuse * engine.diffuseTransform.w + engine.diffuseTransform.rgb;
    const float upDot = dot(SourceStoneUnit(tangentWorldUp), surface.bakedMixedNormal);
    const float2 hemiWeights = float2(0.5f * upDot + 0.5f, -0.5f * upDot + 0.5f);
    const float3 hemisphere = (hemiWeights.x * hemiWeights.x * engine.hemisphereUpper +
        hemiWeights.y * hemiWeights.y * engine.hemisphereLower) * engine.hemisphereScale;
    if (hasLightmap)
    {
        const float3 diffuseWeights = SourceStoneDirectionalWeights(surface.bakedMixedNormal);
        result.bakedIrradiance = lightmapAverage * dot(lightmapCoefficients,
            diffuseWeights * diffuseWeights);
        const float3 view = SourceStoneUnit(tangentView);
        const float3 reflection = 2.f * dot(surface.bakedMixedNormal, view) *
            surface.bakedMixedNormal - view;
        const float3 reflectedWeights = SourceStoneDirectionalWeights(reflection);
        // Native lightmap specular raises clamped RNM dots to power + 1.
        const float reflectedIrradiance = dot(lightmapCoefficients,
            pow(reflectedWeights, specularPower + 1.f));
        const float3 specular = surface.specular * engine.specularTransform.w +
            engine.specularTransform.rgb;
        result.bakedSpecular = lightmapAverage * specular * reflectedIrradiance;
    }
    result.radiance = result.diffuse * (hemisphere + result.bakedIrradiance +
        engine.ambient) + result.bakedSpecular + engine.selection;
    return result;
}

float3 EvaluateSourceStoneDirect(float3 diffuse, float3 specular,
    float3 baseNormal, float3 directMixedNormal, float3 viewDirection,
    float3 lightDirection, float specularPower, float3 incomingLight,
    float3 decodedShadow, float4 diffuseTransform)
{
    const float3 light = SourceStoneUnit(lightDirection);
    const float3 halfDirection = SourceStoneUnit(SourceStoneUnit(viewDirection) + light);
    const float halfDot = abs(dot(baseNormal, halfDirection));
    const float specularLobe = halfDot < 0.000001f ? 0.f :
        min(pow(halfDot, specularPower), 1.f);
    const float positiveNoL = max(dot(directMixedNormal, light), 0.f);
    const float3 radiance = diffuse * min(positiveNoL, 1.f) + specular * specularLobe;
    // Original t4 stores shadow sqrt and is squared before this function.
    // Runtime PCF already supplies linear visibility: do not square it again.
    // Unlike source program 5, this native permutation has no RGB cap of 2.
    return (radiance * diffuseTransform.w + positiveNoL * diffuseTransform.rgb) *
        decodedShadow * incomingLight;
}
#endif
