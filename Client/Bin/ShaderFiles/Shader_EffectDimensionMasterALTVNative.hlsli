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

float4 ALTVNativeSample2CapturedScene(float2 uv, float lod, bool explicitLod)
{
    g_EffectSceneSampleUsed = true;
    const float4 scene = ALTVNativeSample2(uv, lod, explicitLod);
    if (g_EffectSceneReadMode == 0u) return scene;
    if (g_EffectSceneReadMode == 2u) return float4(0.f, 0.f, 0.f, scene.a);
    const uint mode = (((g_SourceTextureClampVMask >> 2u) & 1u) << 1u) |
        ((g_SourceTextureClampUMask >> 2u) & 1u);
    float3 bloom;
    if (mode == 1u) bloom = explicitLod ? g_EffectStartingSceneBloomTexture.SampleLevel(LinearClampUSampler, uv, lod).rgb : g_EffectStartingSceneBloomTexture.SampleBias(LinearClampUSampler, uv, lod).rgb;
    else if (mode == 2u) bloom = explicitLod ? g_EffectStartingSceneBloomTexture.SampleLevel(LinearClampVSampler, uv, lod).rgb : g_EffectStartingSceneBloomTexture.SampleBias(LinearClampVSampler, uv, lod).rgb;
    else if (mode == 3u) bloom = explicitLod ? g_EffectStartingSceneBloomTexture.SampleLevel(LinearClampUVSampler, uv, lod).rgb : g_EffectStartingSceneBloomTexture.SampleBias(LinearClampUVSampler, uv, lod).rgb;
    else bloom = explicitLod ? g_EffectStartingSceneBloomTexture.SampleLevel(LinearSampler, uv, lod).rgb : g_EffectStartingSceneBloomTexture.SampleBias(LinearSampler, uv, lod).rgb;
    return float4(bloom, scene.a);
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
#include "Shader_EffectDimensionMasterALTVNativeDispatchBase64.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 128
#include "Shader_EffectDimensionMasterALTVNativeDispatchBase128.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 192
#include "Shader_EffectDimensionMasterALTVNativeDispatchBase192.hlsli"
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
