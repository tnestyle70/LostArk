// Selected ALT_V native RT0 material programs; see full_native_contract.json.
// Existing Product and grouped material programs never select these IDs.
#ifndef EFFECT_DIMENSIONMASTER_ALTV_NATIVE_HLSLI
#define EFFECT_DIMENSIONMASTER_ALTV_NATIVE_HLSLI
#ifndef ALTV_NATIVE_CAPTURE_ONLY
#include "Shader_EffectSliceSceneDepth.hlsli"
#include "Shader_EffectCubeSampleScene.hlsli"
#endif

float4 g_ALTVSourceMaterialParameters[32];
float g_ALTVSourceMaterialTime = 0.f;

#include "Shader_EffectNativeScreenUV.hlsli"

float4 ALTVNativeAppend(float4 a, float4 b, uint n)
{
    if (n == 1u) return float4(a.x, b.xyz);
    if (n == 2u) return float4(a.xy, b.xy);
    if (n == 3u) return float4(a.xyz, b.x);
    return a;
}
float4 ALTVNativePeriodic(float4 a) { return sign(a) * frac(abs(a)); }

struct ALTV_NATIVE_INPUT
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
    float3 tangentView;
    float4 color;
    float4 dynamicParameter;
    bool frontFace;
};

float4 ALTVNativeSample0(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 0u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 0u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture0.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearSampler, uv, lod);
}

float4 ALTVNativeSample1(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 1u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 1u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture1.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearSampler, uv, lod);
}

float4 ALTVNativeSample2(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 2u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 2u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture2.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearSampler, uv, lod);
}

float4 ALTVNativeSample3(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 3u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 3u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture3.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearSampler, uv, lod);
}

float4 ALTVNativeSample4(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 4u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 4u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture4.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearSampler, uv, lod);
}

float4 ALTVNativeSample5(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 5u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 5u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture5.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearSampler, uv, lod);
}

float4 ALTVNativeSample6(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 6u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 6u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture6.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearSampler, uv, lod);
}

#if !defined(ALTV_NATIVE_CAPTURE_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64)
#include "Shader_EffectDimensionMasterALTVNativeGroup064.hlsli"
#endif
#if defined(ALTV_NATIVE_CAPTURE_ONLY) || !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
#include "Shader_EffectDimensionMasterALTVNativeGroup128.hlsli"
#endif
#if !defined(ALTV_NATIVE_CAPTURE_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192)
#include "Shader_EffectDimensionMasterALTVNativeGroup192.hlsli"
#endif






















































































































































































































































// Original capture RT0, with explicit project capture-view and masked-alpha adapters.


#ifndef ALTV_NATIVE_CAPTURE_ONLY
EFFECT_PS_OUT Shade_EffectDimensionMasterALTVNative(uint profile, ALTV_NATIVE_INPUT input)
{
    EFFECT_PS_OUT output=(EFFECT_PS_OUT)0;
    float4 nativeColor=0.f;
    bool additive=false;
    switch(profile)
    {
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 80u: nativeColor=ALTVNative80(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 81u: nativeColor=ALTVNative81(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 82u: nativeColor=ALTVNative82(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 83u: nativeColor=ALTVNative83(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 84u: nativeColor=ALTVNative84(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 85u: nativeColor=ALTVNative85(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 86u: nativeColor=ALTVNative86(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 87u: nativeColor=ALTVNative87(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 88u: nativeColor=ALTVNative88(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 89u: nativeColor=ALTVNative89(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 90u: nativeColor=ALTVNative90(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 91u: nativeColor=ALTVNative91(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 92u: nativeColor=ALTVNative92(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 93u: nativeColor=ALTVNative93(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 94u: nativeColor=ALTVNative94(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 95u: nativeColor=ALTVNative95(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 96u: nativeColor=ALTVNative96(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 97u: nativeColor=ALTVNative97(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 98u: nativeColor=ALTVNative98(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 99u: nativeColor=ALTVNative99(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 100u: nativeColor=ALTVNative100(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 101u: nativeColor=ALTVNative101(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 102u: nativeColor=ALTVNative102(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 103u: nativeColor=ALTVNative103(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 104u: nativeColor=ALTVNative104(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 105u: nativeColor=ALTVNative105(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 106u: nativeColor=ALTVNative106(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 107u: nativeColor=ALTVNative107(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 108u: nativeColor=ALTVNative108(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 109u: nativeColor=ALTVNative109(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 110u: nativeColor=ALTVNative110(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 111u: nativeColor=ALTVNative111(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 112u: nativeColor=ALTVNative112(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 113u: nativeColor=ALTVNative113(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 114u: nativeColor=ALTVNative114(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 115u: nativeColor=ALTVNative115(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 116u: nativeColor=ALTVNative116(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 117u: nativeColor=ALTVNative117(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 118u: nativeColor=ALTVNative118(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 119u: nativeColor=ALTVNative119(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 120u: nativeColor=ALTVNative120(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 121u: nativeColor=ALTVNative121(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 122u: nativeColor=ALTVNative122(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 123u: nativeColor=ALTVNative123(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 124u: nativeColor=ALTVNative124(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 125u: nativeColor=ALTVNative125(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 126u: nativeColor=ALTVNative126(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 64
    case 127u: nativeColor=ALTVNative127(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 128u: nativeColor=ALTVNative128(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 129u: nativeColor=ALTVNative129(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 130u: nativeColor=ALTVNative130(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 131u: nativeColor=ALTVNative131(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 132u: nativeColor=ALTVNative132(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 133u: nativeColor=ALTVNative133(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 134u: nativeColor=ALTVNative134(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 135u: nativeColor=ALTVNative135(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 136u: nativeColor=ALTVNative136(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 137u: nativeColor=ALTVNative137(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 138u: nativeColor=ALTVNative138(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 139u: nativeColor=ALTVNative139(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 140u: nativeColor=ALTVNative140(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 141u: nativeColor=ALTVNative141(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 142u: nativeColor=ALTVNative142(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 143u: nativeColor=ALTVNative143(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 144u: nativeColor=ALTVNative144(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 145u: nativeColor=ALTVNative145(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 146u: nativeColor=ALTVNative146(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 147u: nativeColor=ALTVNative147(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 148u: nativeColor=ALTVNative148(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 149u: nativeColor=ALTVNative149(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 150u: nativeColor=ALTVNative150(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 151u: nativeColor=ALTVNative151(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 152u: nativeColor=ALTVNative152(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 153u: nativeColor=ALTVNative153(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 154u: nativeColor=ALTVNative154(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 155u: nativeColor=ALTVNative155(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 156u: nativeColor=ALTVNative156(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 157u: nativeColor=ALTVNative157(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 158u: nativeColor=ALTVNative158(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 159u: nativeColor=ALTVNative159(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 160u: nativeColor=ALTVNative160(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 161u: nativeColor=ALTVNative161(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 162u: nativeColor=ALTVNative162(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 163u: nativeColor=ALTVNative163(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 164u: nativeColor=ALTVNative164(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 165u: nativeColor=ALTVNative165(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 166u: nativeColor=ALTVNative166(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 168u: nativeColor=ALTVNative168(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 169u: nativeColor=ALTVNative169(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 170u: nativeColor=ALTVNative170(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 171u: nativeColor=ALTVNative171(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 172u: nativeColor=ALTVNative172(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 173u: nativeColor=ALTVNative173(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 174u: nativeColor=ALTVNative174(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 175u: nativeColor=ALTVNative175(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 176u: nativeColor=ALTVNative176(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 177u: nativeColor=ALTVNative177(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 178u: nativeColor=ALTVNative178(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 179u: nativeColor=ALTVNative179(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 180u: nativeColor=ALTVNative180(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 182u: nativeColor=ALTVNative182(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 183u: nativeColor=ALTVNative183(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 184u: nativeColor=ALTVNative184(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 185u: nativeColor=ALTVNative185(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 186u: nativeColor=ALTVNative186(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 187u: nativeColor=ALTVNative187(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 188u: nativeColor=ALTVNative188(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 189u: nativeColor=ALTVNative189(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 190u: nativeColor=ALTVNative190(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
    case 191u: nativeColor=ALTVNative191(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 192u: nativeColor=ALTVNative192(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 193u: nativeColor=ALTVNative193(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 194u: nativeColor=ALTVNative194(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 195u: nativeColor=ALTVNative195(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 196u: nativeColor=ALTVNative196(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 197u: nativeColor=ALTVNative197(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 198u: nativeColor=ALTVNative198(input); additive=false; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 199u: nativeColor=ALTVNative199(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 200u: nativeColor=ALTVNative200(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 201u: nativeColor=ALTVNative201(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 202u: nativeColor=ALTVNative202(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 203u: nativeColor=ALTVNative203(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 204u: nativeColor=ALTVNative204(input); additive=true; break;
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
    case 205u: nativeColor=ALTVNative205(input); additive=false; break;
#endif
    default: clip(-1.f); return output;
    }
    output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity, additive ? 1.f : nativeColor.a);
    output.Distortion=0.f;
    if(g_ColorClip>0.f) clip(output.SceneColor.a-g_ColorClip);
    return output;
}
#endif // ALTV_NATIVE_CAPTURE_ONLY
#endif
