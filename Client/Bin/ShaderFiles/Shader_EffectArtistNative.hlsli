// Artist native RT0 material programs, generated from exact source shader maps.
// The existing CModel and effect carriers provide the declared runtime inputs.
#ifndef EFFECT_ARTIST_NATIVE_HLSLI
#define EFFECT_ARTIST_NATIVE_HLSLI
#ifndef ARTIST_NATIVE_MODEL_ONLY
#include "Shader_EffectSliceSceneDepth.hlsli"
#include "Shader_EffectCubeSampleScene.hlsli"
#include "Shader_EffectNativeScreenUV.hlsli"
#endif

float4 g_ArtistSourceMaterialParameters[32];
float g_ArtistSourceMaterialTime = 0.f;
float4 g_KoukuSourceAmbient;
float4 g_KoukuSourceActorPosition;
float4x4 g_KoukuSourceProjection;
float4 g_ArtistSourceMacroUV;
float4 g_ArtistSourceWorldToLocal[3];

float4 ArtistNativeAppend(float4 a, float4 b, uint n)
{
    if (n == 1u) return float4(a.x, b.xyz);
    if (n == 2u) return float4(a.xy, b.xy);
    if (n == 3u) return float4(a.xyz, b.x);
    return a;
}
float4 ArtistNativePeriodic(float4 a) { return sign(a) * frac(abs(a)); }

struct ARTIST_NATIVE_INPUT
{
    float2 uv;
    float2 uv1;
    float2 uvNext;
    float subUVBlend;
    float3 sourceWorldPosition;
    float3 sourceBasisX;
    float3 sourceBasisZ;
    float handedness;
    float4 vertexColor;
    float2 screenUV;
    float projectionW;
    float projectionZ;
    float3 tangentView;
    float4 color;
    float4 dynamicParameter;
    bool frontFace;
    float4 sourceProjection[4];
    float3 tangentUp;
    float3 sourceCameraPosition;
    float3 sourceActorPosition;
    float3 skyUpperColor;
    float3 skyLowerColor;
    float3 ambientColor;
    float skyIntensity;
    float4 decalProjection; // Source near/far (cm), opacity.
};

float4 ArtistNativeSample0(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 0u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 0u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture0.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearSampler, uv, lod);
}

float4 ArtistNativeSample1(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 1u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 1u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture1.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearSampler, uv, lod);
}

float4 ArtistNativeSample2(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 2u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 2u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture2.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearSampler, uv, lod);
}

float4 ArtistNativeSample3(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 3u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 3u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture3.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearSampler, uv, lod);
}

float4 ArtistNativeSample4(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 4u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 4u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture4.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearSampler, uv, lod);
}

float4 ArtistNativeSample5(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 5u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 5u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture5.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearSampler, uv, lod);
}

float4 ArtistNativeSample6(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 6u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 6u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture6.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearSampler, uv, lod);
}

float4 ArtistNativeSample7(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 7u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 7u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture7.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture7.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture7.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture7.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearSampler, uv, lod);
}

float4 ArtistNativeSample8(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 8u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 8u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture8.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture8.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture8.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture8.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture8.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture8.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture8.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture8.SampleBias(LinearSampler, uv, lod);
}

float4 ArtistNativeSample9(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 9u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 9u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture9.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture9.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture9.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture9.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture9.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture9.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture9.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture9.SampleBias(LinearSampler, uv, lod);
}

#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
#include "Shader_EffectArtistNativeGroup448.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 512
#include "Shader_EffectArtistNativeGroup512.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 768
#include "Shader_EffectArtistNativeGroup768.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 832
#include "Shader_EffectArtistNativeGroup832.hlsli"
#endif

















































































































































































// BEGIN ADDITIONAL ARTIST GROUPS
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
#include "Shader_EffectArtistNativeGroup1600.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1664
#include "Shader_EffectArtistNativeGroup1664.hlsli"
#endif
// END ADDITIONAL ARTIST GROUPS

// BEGIN WORLD NATIVE GROUP
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2304
#include "Shader_EffectWorldNative.hlsli"
#endif
// END WORLD NATIVE GROUP
// BEGIN KOUKU NATIVE GROUP
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2304 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2304.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2368)
#include "Shader_EffectKoukuNativeGroup2368.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2432)
#include "Shader_EffectKoukuNativeGroup2432.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2560 || defined(EFFECT_NATIVE_DECAL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2560.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2688 || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2688.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2752 || defined(EFFECT_NATIVE_DECAL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2752.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2816 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2816.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2880)
#include "Shader_EffectKoukuNativeGroup2880.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2944 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2944.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3008 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3008.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3072)
#include "Shader_EffectKoukuNativeGroup3072.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3136 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3136.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3200 || defined(EFFECT_NATIVE_DECAL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3200.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3264 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3264.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3328 || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3328.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3584 || defined(EFFECT_NATIVE_DECAL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3584.hlsli"
#endif
// END KOUKU NATIVE GROUP
#ifndef ARTIST_NATIVE_MODEL_ONLY
EFFECT_PS_OUT Shade_EffectArtistNative(uint profile, ARTIST_NATIVE_INPUT input)
{
    EFFECT_PS_OUT output=(EFFECT_PS_OUT)0;
    float4 nativeColor=0.f;
    bool opaqueCoverage=false;
    switch(profile)
    {
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
#include "Shader_EffectArtistNativeDispatchBase448.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 512
#include "Shader_EffectArtistNativeDispatchBase512.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 768
#include "Shader_EffectArtistNativeDispatchBase768.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 832
#include "Shader_EffectArtistNativeDispatchBase832.hlsli"
#endif
// BEGIN ADDITIONAL ARTIST CASES
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
#include "Shader_EffectArtistNativeDispatchAdditionalArtistCases1600.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1664
#include "Shader_EffectArtistNativeDispatchAdditionalArtistCases1664.hlsli"
#endif
// END ADDITIONAL ARTIST CASES
// BEGIN WORLD NATIVE CASES
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2304
#include "Shader_EffectArtistNativeDispatchWorldNativeCases2304.hlsli"
#endif
// END WORLD NATIVE CASES
// BEGIN KOUKU NATIVE CASES
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2304 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2304.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2368
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2368.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2432
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2432.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2560 || defined(EFFECT_NATIVE_DECAL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2560.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3584
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3584.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2688 || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2688.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2752 || defined(EFFECT_NATIVE_DECAL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2752.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2816 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2816.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2880
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2880.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2944 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2944.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3008 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3008.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3072
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3072.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3136 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3136.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3200 || defined(EFFECT_NATIVE_DECAL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3200.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3264 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3264.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3328 || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3328.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3584 || defined(EFFECT_NATIVE_DECAL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3584Part2.hlsli"
#endif
// END KOUKU NATIVE CASES
    default: clip(-1.f); return output;
    }
    output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity, opaqueCoverage ? 1.f : nativeColor.a);
    output.Distortion=0.f;
    if(g_ColorClip>0.f) clip(output.SceneColor.a-g_ColorClip);
    return output;
}
#endif

float4 Shade_ArtistModelNative(uint profile, ARTIST_NATIVE_INPUT input)
{
    switch(profile)
    {
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 460u: return ArtistNative460(input);
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 461u: return ArtistNative461(input);
#endif
    default: clip(-1.f); return 0.f;
    }
}
#endif
