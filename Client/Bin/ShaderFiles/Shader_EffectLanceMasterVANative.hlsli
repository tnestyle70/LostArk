// Selected W_R native RT0 material programs; see full_native_contract.json.
// Dedicated source occurrences select these material/VF contracts.
#ifndef EFFECT_LANCE_VA_NATIVE_HLSLI
#define EFFECT_LANCE_VA_NATIVE_HLSLI
#ifndef LANCE_VA_NATIVE_MODEL_ONLY
#include "Shader_EffectSliceSceneDepth.hlsli"
#include "Shader_EffectCubeSampleScene.hlsli"
#include "Shader_EffectNativeScreenUV.hlsli"
#endif

float4 g_LanceVASourceMaterialParameters[32];
float g_LanceVASourceMaterialTime = 0.f;

float4 LanceVANativeAppend(float4 a, float4 b, uint n)
{
    if (n == 1u) return float4(a.x, b.xyz);
    if (n == 2u) return float4(a.xy, b.xy);
    if (n == 3u) return float4(a.xyz, b.x);
    return a;
}
float4 LanceVANativePeriodic(float4 a) { return sign(a) * frac(abs(a)); }

struct LANCE_VA_NATIVE_INPUT
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
};

float4 LanceVANativeSample0(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 0u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 0u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture0.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearSampler, uv, lod);
}

float4 LanceVANativeSample1(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 1u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 1u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture1.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearSampler, uv, lod);
}

float4 LanceVANativeSample2(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 2u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 2u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture2.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearSampler, uv, lod);
}

float4 LanceVANativeSample3(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 3u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 3u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture3.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearSampler, uv, lod);
}

float4 LanceVANativeSample4(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 4u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 4u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture4.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearSampler, uv, lod);
}

float4 LanceVANativeSample5(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 5u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 5u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture5.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearSampler, uv, lod);
}

float4 LanceVANativeSample6(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 6u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 6u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture6.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearSampler, uv, lod);
}

float4 LanceVANativeSample7(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 7u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 7u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture7.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture7.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture7.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture7.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearSampler, uv, lod);
}

float4 LanceVANativeSample8(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 8u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 8u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture8.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture8.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture8.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture8.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture8.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture8.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture8.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture8.SampleBias(LinearSampler, uv, lod);
}

#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 512
#include "Shader_EffectLanceMasterVANativeGroup512.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 576
#include "Shader_EffectLanceMasterVANativeGroup576.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 640
#include "Shader_EffectLanceMasterVANativeGroup640.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 704
#include "Shader_EffectLanceMasterVANativeGroup704.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 768
#include "Shader_EffectLanceMasterVANativeGroup768.hlsli"
#endif




















































































































































































#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1152
#include "Shader_EffectLanceMasterVANativeGroup1152.hlsli"
#endif

#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1216
#include "Shader_EffectLanceMasterVANativeGroup1216.hlsli"
#endif

#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1280
#include "Shader_EffectLanceMasterVANativeGroup1280.hlsli"
#endif

#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1344
#include "Shader_EffectLanceMasterVANativeGroup1344.hlsli"
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
EFFECT_PS_OUT Shade_EffectLanceMasterVANative(uint profile, LANCE_VA_NATIVE_INPUT input)
{
    EFFECT_PS_OUT output=(EFFECT_PS_OUT)0;
    float4 nativeColor=0.f;
    bool additive=false;
    switch(profile)
    {
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 512
#include "Shader_EffectLanceMasterVANativeDispatchBase512.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 576
#include "Shader_EffectLanceMasterVANativeDispatchBase576.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 640
#include "Shader_EffectLanceMasterVANativeDispatchBase640.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 704
#include "Shader_EffectLanceMasterVANativeDispatchBase704.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 768
#include "Shader_EffectLanceMasterVANativeDispatchBase768.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1152
#include "Shader_EffectLanceMasterVANativeDispatchBase1152.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1216
#include "Shader_EffectLanceMasterVANativeDispatchBase1216.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1280
#include "Shader_EffectLanceMasterVANativeDispatchBase1280.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1344
#include "Shader_EffectLanceMasterVANativeDispatchBase1344.hlsli"
#endif
    default: clip(-1.f); return output;
    }
    output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity, additive ? 1.f : nativeColor.a);
    output.Distortion=0.f;
    if(g_ColorClip>0.f) clip(output.SceneColor.a-g_ColorClip);
    return output;
}
#endif

#ifdef LANCE_VA_NATIVE_MODEL_ONLY
// T 34650 source skeletal PS 479de3b6c8fa214281f96871137df0fe.
float4 LanceVANative1360(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    // Existing project scene adapters: absolute source-cm origin, current scene hemispherical lighting.
    source[0]=0.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[0]=float4(1.f,0.f,0.f,0.f); // Source global opacity.
    source[1]=input.color; // Source skeletal instance RGB and alpha modulation.
    source[2] = g_LanceVASourceMaterialParameters[9u];
    source[3] = g_LanceVASourceMaterialParameters[7u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[5].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[7].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[7].y = ((float4(1.0, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[1u].zzzz)).x;
    source[7].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[7].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[8].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[8].y = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[8].z = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[8].w = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[9].x = (g_LanceVASourceMaterialParameters[6u].yyyy).x;
    source[9].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[9].z = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[9].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[10].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[10].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[10].w = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[11].x = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[11].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[11].z = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[11].w = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 3: source device depth mapped to centimetre view depth; reconstruction at 5.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 5-8: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 9: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 10: add r0.y, -cb0[11].y, l(1.000000)
    r0.y = ((-(source[11].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 14: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 16: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 17: mul r0.z, r0.z, cb0[11].z
    r0.z = ((r0.zzzz)*(source[11].zzzz)).z;
    // 18: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 19: mul_sat r0.z, r0.z, cb0[11].w
    r0.z = (saturate((r0.zzzz)*(source[11].wwww))).z;
    // 20: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 21: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 22: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 23: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 24: mul r0.zw, v4.xxxy, cb0[6].yyyz
    r0.zw = ((v4.xxxy)*(source[6].yyyz)).zw;
    // 25: mad r0.zw, cb0[5].yyyy, cb0[6].xxxw, r0.zzzw
    r0.zw = ((source[5].yyyy)*(source[6].xxxw)+(r0.zzzw)).zw;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s1, l(0.000000)
    r0.zw = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 27: mad r0.zw, cb0[7].yyyy, r0.zzzw, v4.xxxy
    r0.zw = ((source[7].yyyy)*(r0.zzzw)+(v4.xxxy)).zw;
    // 28: mul r1.xy, r0.zwzz, cb0[10].xyxx
    r1.xy = ((r0.zwzz)*(source[10].xyxx)).xy;
    // 29: mad r2.x, cb0[5].y, cb0[9].w, r1.x
    r2.x = ((source[5].yyyy)*(source[9].wwww)+(r1.xxxx)).x;
    // 30: mad r2.y, cb0[5].y, cb0[10].z, r1.y
    r2.y = ((source[5].yyyy)*(source[10].zzzz)+(r1.yyyy)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t1.xyzw, s4, l(0.000000)
    r1.xyz = (LanceVANativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 33: add r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)+(r1.xxxx)).x;
    // 34: add r1.y, -cb0[4].x, l(1.000000)
    r1.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 35: mad r1.x, r1.x, l(0.333330), -r1.y
    r1.x = ((r1.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))+(-(r1.yyyy))).x;
    // 36: mul_sat r1.x, r1.x, cb0[10].w
    r1.x = (saturate((r1.xxxx)*(source[10].wwww))).x;
    // 37: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 38: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 39: mul r1.y, r1.y, cb0[11].x
    r1.y = ((r1.yyyy)*(source[11].xxxx)).y;
    // 40: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 41: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 42: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 43: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 44: movc o0.w, r1.x, l(0), r0.x
    output.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 45: log r0.x, |r0.y|
    r0.x = (log2(abs(r0.yyyy))).x;
    // 46: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 47: mul r0.x, r0.x, cb0[9].y
    r0.x = ((r0.xxxx)*(source[9].yyyy)).x;
    // 48: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 49: mul r0.x, r0.x, cb0[9].z
    r0.x = ((r0.xxxx)*(source[9].zzzz)).x;
    // 50: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 51: mul r1.xy, r0.zwzz, cb0[5].zwzz
    r1.xy = ((r0.zwzz)*(source[5].zwzz)).xy;
    // 52: mul r0.yz, r0.zzwz, cb0[8].xxyx
    r0.yz = ((r0.zzwz)*(source[8].xxyx)).yz;
    // 53: mad r2.x, cb0[5].y, cb0[5].x, r1.x
    r2.x = ((source[5].yyyy)*(source[5].xxxx)+(r1.xxxx)).x;
    // 54: mad r2.y, cb0[5].y, cb0[7].z, r1.y
    r2.y = ((source[5].yyyy)*(source[7].zzzz)+(r1.yyyy)).y;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t3.xyzw, s2, l(0.000000)
    r1.xyz = (LanceVANativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 56: mad r2.x, cb0[5].y, cb0[7].w, r0.y
    r2.x = ((source[5].yyyy)*(source[7].wwww)+(r0.yyyy)).x;
    // 57: mad r2.y, cb0[5].y, cb0[8].z, r0.z
    r2.y = ((source[5].yyyy)*(source[8].zzzz)+(r0.zzzz)).y;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r2.xyxx, t4.wxyz, s3, l(0.000000)
    r0.yzw = (LanceVANativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 59: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 60: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 61: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 62: mad r0.yzw, cb0[8].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[8].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 63: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 64: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 65: mul r0.yzw, r0.yyzw, cb0[9].xxxx
    r0.yzw = ((r0.yyzw)*(source[9].xxxx)).yzw;
    // 66: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 67: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 68: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 69: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 70: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 71: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

float4 Shade_LanceVAModelNative(uint profile, LANCE_VA_NATIVE_INPUT input)
{
    switch(profile)
    {
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 768
    case 772u: return LanceVANative772(input);
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 768
    case 773u: return LanceVANative773(input);
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 768
    case 774u: return LanceVANative774(input);
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 768
    case 775u: return LanceVANative775(input);
#endif
#ifdef LANCE_VA_NATIVE_MODEL_ONLY
    case 1360u: return LanceVANative1360(input);
#endif
    default: clip(-1.f); return 0.f;
    }
}
#endif
