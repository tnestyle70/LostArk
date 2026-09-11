// Selected S_D native RT0 material programs; see full_native_contract.json.
// Existing Product and grouped material programs never select these IDs.
#ifndef EFFECT_DIMENSIONMASTER_SD_NATIVE_HLSLI
#define EFFECT_DIMENSIONMASTER_SD_NATIVE_HLSLI
#include "Shader_EffectDimensionMasterVNativeShared.hlsli"
#include "Shader_EffectSliceSceneDepth.hlsli"
#include "Shader_EffectCubeSampleScene.hlsli"

// Mutually exclusive V/SD programs share the existing native packet.
#define g_SDSourceMaterialParameters g_VSourceMaterialParameters
#define g_SDSourceMaterialTime g_VSourceMaterialTime

float4 SDNativeAppend(float4 a, float4 b, uint n)
{
    if (n == 1u) return float4(a.x, b.xyz);
    if (n == 2u) return float4(a.xy, b.xy);
    if (n == 3u) return float4(a.xyz, b.x);
    return a;
}
float4 SDNativePeriodic(float4 a) { return sign(a) * frac(abs(a)); }

float4 g_SDSourceWorldToLocal[3];

struct SD_NATIVE_INPUT
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
};

float4 SDNativeSample0(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 0u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 0u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture0.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearSampler, uv, lod);
}

float4 SDNativeSample1(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 1u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 1u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture1.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearSampler, uv, lod);
}

float4 SDNativeSample2(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 2u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 2u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture2.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearSampler, uv, lod);
}

float4 SDNativeSample3(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 3u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 3u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture3.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearSampler, uv, lod);
}

float4 SDNativeSample4(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 4u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 4u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture4.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearSampler, uv, lod);
}

float4 SDNativeSample5(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 5u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 5u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture5.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearSampler, uv, lod);
}

float4 SDNativeSample6(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 6u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 6u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture6.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearSampler, uv, lod);
}

// fx_e_pa_gl_10_1_tr: d465be14468f91489ecb9184a42842a4; selected map bb4d665ef4d8c477c1f8a322a50949dcd37b2f563e5fddb80dc209d6bc3462ef.
float4 SDNative320(SD_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[3u];
    source[2] = g_SDSourceMaterialParameters[2u];
    source[3] = SDNativeAppend(cos(((g_SDSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = SDNativeAppend(sin(((g_SDSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_SDSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = SDNativeAppend((float4(1.0, 0.0, 0.0, 0.0)/g_SDSourceMaterialParameters[1u].zzzz),(float4(1.0, 0.0, 0.0, 0.0)/g_SDSourceMaterialParameters[1u].wwww),1u);
    source[6] = SDNativeAppend(g_SDSourceMaterialParameters[1u].xxxx,g_SDSourceMaterialParameters[1u].yyyy,1u);
    source[7].x = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[7].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].yyyy)).x;
    source[7].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[7].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[8].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_SDSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: mad r0.xy, cb0[5].xyxx, r1.xyxx, -cb0[6].xyxx
    r0.xy = ((source[5].xyxx)*(r1.xyxx)+(-(source[6].xyxx))).xy;
    // 5: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 6: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 7: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: mul_sat r0.x, r0.x, cb0[7].w
    r0.x = (saturate((r0.xxxx)*(source[7].wwww))).x;
    // 9: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 10: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 11: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 12: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 13: mul_sat r0.y, r0.y, cb0[8].y
    r0.y = (saturate((r0.yyyy)*(source[8].yyyy))).y;
    // 14: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 15: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 16: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 17: mad r0.xyz, cb0[2].xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((source[2].xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 18: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_r_pa_spritewave_24_02_tr: cc08dcd05bd1b9439a06da104926b000; selected map 48022c59c5d7ff08f11cc6922e68fd12f116403caa0a8853190b5e754b6eccfa.
float4 SDNative321(SD_NATIVE_INPUT input)
{
    float4 source[19]; [unroll] for (uint i=0u; i<19u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[10u];
    source[2] = SDNativeAppend(g_SDSourceMaterialParameters[8u].zzzz,g_SDSourceMaterialParameters[8u].wwww,1u);
    source[3] = SDNativeAppend(g_SDSourceMaterialParameters[4u].wwww,g_SDSourceMaterialParameters[5u].xxxx,1u);
    source[4] = SDNativeAppend(cos(((g_SDSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = SDNativeAppend(sin(((g_SDSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_SDSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = SDNativeAppend(cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = SDNativeAppend(sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = g_SDSourceMaterialParameters[9u];
    source[9].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].y = (cos(((g_SDSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[9].w = (g_SDSourceMaterialTime.xxxx).x;
    source[10].x = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[10].y = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[10].z = (g_SDSourceMaterialParameters[5u].zzzz).x;
    source[10].w = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[11].x = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[11].y = (g_SDSourceMaterialParameters[7u].zzzz).x;
    source[11].z = (g_SDSourceMaterialParameters[8u].xxxx).x;
    source[11].w = (g_SDSourceMaterialParameters[8u].yyyy).x;
    source[12].x = (g_SDSourceMaterialParameters[7u].wwww).x;
    source[12].y = (g_SDSourceMaterialParameters[8u].wwww).x;
    source[12].z = (g_SDSourceMaterialParameters[8u].zzzz).x;
    source[12].w = (g_SDSourceMaterialParameters[7u].yyyy).x;
    source[13].x = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[13].y = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[13].z = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[13].w = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[14].x = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[14].y = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[14].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[14].w = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[15].x = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[15].y = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[15].z = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[15].w = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[16].x = (cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[16].y = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[16].z = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[16].w = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[17].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[17].y = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[17].z = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[17].w = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[18].x = (g_SDSourceMaterialParameters[7u].xxxx).x;
    source[18].y = (g_SDSourceMaterialParameters[4u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 5: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 6: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 7: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).x;
    // 8: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 9: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).x;
    // 10: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 11: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 12: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).x;
    // 13: lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.y, r0.x
    r1.x = (max(r0.yyyy,r0.xxxx)).x;
    // 22: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 23: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 24: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 25: mad r1.x, r0.z, l(0.159155), l(0.500000)
    r1.x = ((r0.zzzz)*(float4(asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u)))+(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).x;
    // 26: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 27: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 28: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 29: mul r0.w, r0.w, v4.w
    r0.w = ((r0.wwww)*(v4.wwww)).w;
    // 30: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 31: lt r1.z, r0.z, l(0.000001)
    r1.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 32: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 34: mul r0.z, r0.z, cb0[17].z
    r0.z = ((r0.zzzz)*(source[17].zzzz)).z;
    // 35: max r0.z, r0.z, cb0[18].x
    r0.z = (max(r0.zzzz,source[18].xxxx)).z;
    // 36: min r0.z, r0.z, cb0[17].w
    r0.z = (min(r0.zzzz,source[17].wwww)).z;
    // 37: movc r1.y, r1.z, l(0), r0.w
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 38: mul r1.xy, r1.xyxx, cb0[10].xyxx
    r1.xy = ((r1.xyxx)*(source[10].xyxx)).xy;
    // 39: mad r2.x, cb0[9].w, cb0[9].z, r1.x
    r2.x = ((source[9].wwww)*(source[9].zzzz)+(r1.xxxx)).x;
    // 40: mad r2.y, cb0[9].w, cb0[10].z, r1.y
    r2.y = ((source[9].wwww)*(source[10].zzzz)+(r1.yyyy)).y;
    // 41: mul r1.x, v4.x, cb0[10].w
    r1.x = ((v4.xxxx)*(source[10].wwww)).x;
    // 42: mul r1.y, v4.x, cb0[11].x
    r1.y = ((v4.xxxx)*(source[11].xxxx)).y;
    // 43: mad r1.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), r1.xyxx
    r1.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(r1.xyxx)).xy;
    // 44: mul r1.zw, v2.xxxy, cb0[11].zzzw
    r1.zw = ((v2.xxxy)*(source[11].zzzw)).zw;
    // 45: mad r2.x, cb0[9].w, cb0[11].y, r1.z
    r2.x = ((source[9].wwww)*(source[11].yyyy)+(r1.zzzz)).x;
    // 46: mad r2.y, cb0[9].w, cb0[12].x, r1.w
    r2.y = ((source[9].wwww)*(source[12].xxxx)+(r1.wwww)).y;
    // 47: add r1.zw, r2.xxxy, cb0[2].xxxy
    r1.zw = ((r2.xxxy)+(source[2].xxxy)).zw;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t0.yzwx, s1, l(0.000000)
    r0.w = (SDNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 49: add r1.z, v4.z, cb0[12].w
    r1.z = ((v4.zzzz)+(source[12].wwww)).z;
    // 50: mad r1.zw, r0.wwww, r1.zzzz, cb0[3].xxxy
    r1.zw = ((r0.wwww)*(r1.zzzz)+(source[3].xxxy)).zw;
    // 51: add r1.xy, r1.zwzz, r1.xyxx
    r1.xy = ((r1.zwzz)+(r1.xyxx)).xy;
    // 52: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 53: dp2 r2.x, cb0[4].xyxx, r1.xyxx
    r2.x = (dot((source[4].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 54: dp2 r2.y, cb0[5].xyxx, r1.xyxx
    r2.y = (dot((source[5].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 55: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s0, l(-1.000000)
    r0.w = (SDNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 57: mul r1.x, v2.x, cb0[13].w
    r1.x = ((v2.xxxx)*(source[13].wwww)).x;
    // 58: mad r1.x, cb0[9].w, cb0[13].z, r1.x
    r1.x = ((source[9].wwww)*(source[13].zzzz)+(r1.xxxx)).x;
    // 59: mul r1.z, v2.y, cb0[14].x
    r1.z = ((v2.yyyy)*(source[14].xxxx)).z;
    // 60: mad r1.y, cb0[9].w, cb0[14].y, r1.z
    r1.y = ((source[9].wwww)*(source[14].yyyy)+(r1.zzzz)).y;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s2, l(0.000000)
    r1.x = (SDNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 62: mul r1.x, r0.w, r1.x
    r1.x = ((r0.wwww)*(r1.xxxx)).x;
    // 63: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 64: mul r1.y, r1.y, cb0[14].z
    r1.y = ((r1.yyyy)*(source[14].zzzz)).y;
    // 65: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 66: mul r1.y, r1.y, cb0[14].w
    r1.y = ((r1.yyyy)*(source[14].wwww)).y;
    // 67: lt r1.z, |r1.x|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 68: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 69: mad r1.x, r1.x, cb0[15].x, r1.y
    r1.x = ((r1.xxxx)*(source[15].xxxx)+(r1.yyyy)).x;
    // 70: dp2 r2.x, cb0[6].xyxx, r0.xyxx
    r2.x = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 71: dp2 r2.y, cb0[7].xyxx, r0.xyxx
    r2.y = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 72: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 73: mul r0.xy, r0.xyxx, cb0[15].zwzz
    r0.xy = ((r0.xyxx)*(source[15].zwzz)).xy;
    // 74: mad r2.x, cb0[9].w, cb0[15].y, r0.x
    r2.x = ((source[9].wwww)*(source[15].yyyy)+(r0.xxxx)).x;
    // 75: mad r2.y, cb0[9].w, cb0[16].y, r0.y
    r2.y = ((source[9].wwww)*(source[16].yyyy)+(r0.yyyy)).y;
    // 76: add r0.xy, r2.xyxx, cb0[16].zwzz
    r0.xy = ((r2.xyxx)+(source[16].zwzz)).xy;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (SDNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 78: add r0.x, r0.x, l(0.100000)
    r0.x = ((r0.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 79: add r0.y, v4.y, l(-1.000000)
    r0.y = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 80: add_sat r0.x, -r0.y, r0.x
    r0.x = (saturate((-(r0.yyyy))+(r0.xxxx))).x;
    // 81: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 82: mul r0.y, r0.y, cb0[17].y
    r0.y = ((r0.yyyy)*(source[17].yyyy)).y;
    // 83: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 84: mul_sat r0.y, r0.y, cb0[17].x
    r0.y = (saturate((r0.yyyy)*(source[17].xxxx))).y;
    // 85: lt r1.y, r0.x, l(0.000001)
    r1.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 86: mul r0.x, r0.x, cb0[17].x
    r0.x = ((r0.xxxx)*(source[17].xxxx)).x;
    // 87: movc r0.y, r1.y, l(-0.000000), -r0.y
    r0.y = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).y;
    // 88: mov_sat r1.y, r0.x
    r1.y = (saturate(r0.xxxx)).y;
    // 89: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 90: mul r0.x, r0.x, cb0[18].y
    r0.x = ((r0.xxxx)*(source[18].yyyy)).x;
    // 91: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 92: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 93: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 94: add r0.x, r0.y, r1.y
    r0.x = ((r0.yyyy)+(r1.yyyy)).x;
    // 95: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 96: mad r0.xyz, r0.xxxx, cb0[8].xyzx, r1.xxxx
    r0.xyz = ((r0.xxxx)*(source[8].xyzx)+(r1.xxxx)).xyz;
    // 97: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 98: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_r_pa_spritewave_30_01_tr: ba9d1903ac568249a1c453c51d687cb4; selected map c655df40522ac64d3b17bfec39072b8c149841b647d82f7ad4a48a3ef6171a37.
float4 SDNative322(SD_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[9u];
    source[2] = SDNativeAppend(g_SDSourceMaterialParameters[7u].zzzz,g_SDSourceMaterialParameters[7u].wwww,1u);
    source[3] = SDNativeAppend(g_SDSourceMaterialParameters[3u].wwww,g_SDSourceMaterialParameters[4u].xxxx,1u);
    source[4] = SDNativeAppend(cos(((g_SDSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = SDNativeAppend(sin(((g_SDSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_SDSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = SDNativeAppend(cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = SDNativeAppend(sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = g_SDSourceMaterialParameters[8u];
    source[9].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].y = (cos(((g_SDSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[9].w = (g_SDSourceMaterialTime.xxxx).x;
    source[10].x = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[10].y = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[10].z = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[10].w = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[11].x = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[11].y = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[11].z = (g_SDSourceMaterialParameters[7u].xxxx).x;
    source[11].w = (g_SDSourceMaterialParameters[7u].yyyy).x;
    source[12].x = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_SDSourceMaterialParameters[7u].wwww).x;
    source[12].z = (g_SDSourceMaterialParameters[7u].zzzz).x;
    source[12].w = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[13].x = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[13].y = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[13].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[14].x = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[14].y = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[14].z = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[14].w = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[15].x = (cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[15].y = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[15].z = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[15].w = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[16].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[16].y = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[16].z = (g_SDSourceMaterialParameters[5u].zzzz).x;
    source[16].w = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[17].x = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[17].y = (g_SDSourceMaterialParameters[3u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 5: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 6: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 7: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).x;
    // 8: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 9: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).x;
    // 10: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 11: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 12: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).x;
    // 13: lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.y, r0.x
    r1.x = (max(r0.yyyy,r0.xxxx)).x;
    // 22: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 23: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 24: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 25: mad r1.x, r0.z, l(0.159155), l(0.500000)
    r1.x = ((r0.zzzz)*(float4(asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u)))+(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).x;
    // 26: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 27: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 28: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 29: mul r0.w, r0.w, v4.w
    r0.w = ((r0.wwww)*(v4.wwww)).w;
    // 30: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 31: lt r1.z, r0.z, l(0.000001)
    r1.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 32: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 34: mul r0.z, r0.z, cb0[16].z
    r0.z = ((r0.zzzz)*(source[16].zzzz)).z;
    // 35: max r0.z, r0.z, cb0[17].x
    r0.z = (max(r0.zzzz,source[17].xxxx)).z;
    // 36: min r0.z, r0.z, cb0[16].w
    r0.z = (min(r0.zzzz,source[16].wwww)).z;
    // 37: movc r1.y, r1.z, l(0), r0.w
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 38: mul r1.xy, r1.xyxx, cb0[10].xyxx
    r1.xy = ((r1.xyxx)*(source[10].xyxx)).xy;
    // 39: mad r2.x, cb0[9].w, cb0[9].z, r1.x
    r2.x = ((source[9].wwww)*(source[9].zzzz)+(r1.xxxx)).x;
    // 40: mad r2.y, cb0[9].w, cb0[10].z, r1.y
    r2.y = ((source[9].wwww)*(source[10].zzzz)+(r1.yyyy)).y;
    // 41: mul r1.x, v4.x, cb0[10].w
    r1.x = ((v4.xxxx)*(source[10].wwww)).x;
    // 42: mul r1.y, v4.x, cb0[11].x
    r1.y = ((v4.xxxx)*(source[11].xxxx)).y;
    // 43: mad r1.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), r1.xyxx
    r1.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(r1.xyxx)).xy;
    // 44: mul r1.zw, v2.xxxy, cb0[11].zzzw
    r1.zw = ((v2.xxxy)*(source[11].zzzw)).zw;
    // 45: mad r2.x, cb0[9].w, cb0[11].y, r1.z
    r2.x = ((source[9].wwww)*(source[11].yyyy)+(r1.zzzz)).x;
    // 46: mad r2.y, cb0[9].w, cb0[12].x, r1.w
    r2.y = ((source[9].wwww)*(source[12].xxxx)+(r1.wwww)).y;
    // 47: add r1.zw, r2.xxxy, cb0[2].xxxy
    r1.zw = ((r2.xxxy)+(source[2].xxxy)).zw;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t0.yzwx, s1, l(0.000000)
    r0.w = (SDNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 49: add r1.z, v4.z, cb0[12].w
    r1.z = ((v4.zzzz)+(source[12].wwww)).z;
    // 50: mad r1.zw, r0.wwww, r1.zzzz, cb0[3].xxxy
    r1.zw = ((r0.wwww)*(r1.zzzz)+(source[3].xxxy)).zw;
    // 51: add r1.xy, r1.zwzz, r1.xyxx
    r1.xy = ((r1.zwzz)+(r1.xyxx)).xy;
    // 52: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 53: dp2 r2.x, cb0[4].xyxx, r1.xyxx
    r2.x = (dot((source[4].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 54: dp2 r2.y, cb0[5].xyxx, r1.xyxx
    r2.y = (dot((source[5].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 55: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s0, l(-1.000000)
    r0.w = (SDNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 57: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 58: mul r1.x, r1.x, cb0[13].z
    r1.x = ((r1.xxxx)*(source[13].zzzz)).x;
    // 59: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 60: mul r1.x, r1.x, cb0[13].w
    r1.x = ((r1.xxxx)*(source[13].wwww)).x;
    // 61: lt r1.y, |r0.w|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 62: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 63: mad r1.x, r0.w, cb0[14].x, r1.x
    r1.x = ((r0.wwww)*(source[14].xxxx)+(r1.xxxx)).x;
    // 64: dp2 r2.x, cb0[6].xyxx, r0.xyxx
    r2.x = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 65: dp2 r2.y, cb0[7].xyxx, r0.xyxx
    r2.y = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 66: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: mul r0.xy, r0.xyxx, cb0[14].zwzz
    r0.xy = ((r0.xyxx)*(source[14].zwzz)).xy;
    // 68: mad r2.x, cb0[9].w, cb0[14].y, r0.x
    r2.x = ((source[9].wwww)*(source[14].yyyy)+(r0.xxxx)).x;
    // 69: mad r2.y, cb0[9].w, cb0[15].y, r0.y
    r2.y = ((source[9].wwww)*(source[15].yyyy)+(r0.yyyy)).y;
    // 70: add r0.xy, r2.xyxx, cb0[15].zwzz
    r0.xy = ((r2.xyxx)+(source[15].zwzz)).xy;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (SDNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 72: add r0.x, r0.x, l(0.100000)
    r0.x = ((r0.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 73: add r0.y, v4.y, l(-1.000000)
    r0.y = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 74: add_sat r0.x, -r0.y, r0.x
    r0.x = (saturate((-(r0.yyyy))+(r0.xxxx))).x;
    // 75: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 76: mul r0.y, r0.y, cb0[16].y
    r0.y = ((r0.yyyy)*(source[16].yyyy)).y;
    // 77: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 78: mul_sat r0.y, r0.y, cb0[16].x
    r0.y = (saturate((r0.yyyy)*(source[16].xxxx))).y;
    // 79: lt r1.y, r0.x, l(0.000001)
    r1.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 80: mul r0.x, r0.x, cb0[16].x
    r0.x = ((r0.xxxx)*(source[16].xxxx)).x;
    // 81: movc r0.y, r1.y, l(-0.000000), -r0.y
    r0.y = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).y;
    // 82: mov_sat r1.y, r0.x
    r1.y = (saturate(r0.xxxx)).y;
    // 83: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 84: mul r0.x, r0.x, cb0[17].y
    r0.x = ((r0.xxxx)*(source[17].yyyy)).x;
    // 85: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 86: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 87: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 88: add r0.x, r0.y, r1.y
    r0.x = ((r0.yyyy)+(r1.yyyy)).x;
    // 89: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 90: mad r0.xyz, r0.xxxx, cb0[8].xyzx, r1.xxxx
    r0.xyz = ((r0.xxxx)*(source[8].xyzx)+(r1.xxxx)).xyz;
    // 91: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 92: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_r_pa_twirl_03_09_ad: 1f377575d000c741acc21359a3b57bff; selected map 891633497c18c98704b26f55b0266657677b4f2793e6d9085d01e3f266d8838a.
float4 SDNative323(SD_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[6u];
    source[2] = g_SDSourceMaterialParameters[5u];
    source[3].x = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[3].y = (g_SDSourceMaterialTime.xxxx).x;
    source[3].z = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[3].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[4].x = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[4].w = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[5].x = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[5].y = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[5].w = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[6].x = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[6].y = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[6].z = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[6].w = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[7].x = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[7].y = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[7].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_SDSourceMaterialParameters[0u].yyyy)).x;
    source[7].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_SDSourceMaterialParameters[0u].yyyy))).x;
    source[8].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].wwww)).x;
    source[8].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].x = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[9].y = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[9].z = ((float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].zzzz)).x;
    source[9].w = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].zzzz))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
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
    // 10: add r0.y, -cb0[9].w, l(1.000000)
    r0.y = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, l(100.000000)
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 12: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 13: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 14: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 15: add r1.xy, r0.yzyy, r0.yzyy
    r1.xy = ((r0.yzyy)+(r0.yzyy)).xy;
    // 16: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 17: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 18: max r0.z, |r1.x|, |r1.y|
    r0.z = (max(abs(r1.xxxx),abs(r1.yyyy))).z;
    // 19: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 20: min r0.w, |r1.x|, |r1.y|
    r0.w = (min(abs(r1.xxxx),abs(r1.yyyy))).w;
    // 21: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 22: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 23: mad r1.z, r0.w, l(0.020835), l(-0.085133)
    r1.z = ((r0.wwww)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).z;
    // 24: mad r1.z, r0.w, r1.z, l(0.180141)
    r1.z = ((r0.wwww)*(r1.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 25: mad r1.z, r0.w, r1.z, l(-0.330299)
    r1.z = ((r0.wwww)*(r1.zzzz)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).z;
    // 26: mad r0.w, r0.w, r1.z, l(0.999866)
    r0.w = ((r0.wwww)*(r1.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 27: mul r1.z, r0.w, r0.z
    r1.z = ((r0.wwww)*(r0.zzzz)).z;
    // 28: mad r1.z, r1.z, l(-2.000000), l(1.570796)
    r1.z = ((r1.zzzz)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).z;
    // 29: lt r1.w, |r1.x|, |r1.y|
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(abs(r1.yyyy))) * 0xffffffffu)).w;
    // 30: and r1.z, r1.w, r1.z
    r1.z = (asfloat(asuint(r1.wwww) & asuint(r1.zzzz))).z;
    // 31: mad r0.z, r0.z, r0.w, r1.z
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.zzzz)).z;
    // 32: lt r0.w, r1.x, -r1.x
    r0.w = (asfloat((uint4)((r1.xxxx)<(-(r1.xxxx))) * 0xffffffffu)).w;
    // 33: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 34: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 35: min r0.w, r1.x, r1.y
    r0.w = (min(r1.xxxx,r1.yyyy)).w;
    // 36: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 37: max r1.z, r1.x, r1.y
    r1.z = (max(r1.xxxx,r1.yyyy)).z;
    // 38: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 39: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 40: ge r1.y, r1.z, -r1.z
    r1.y = (asfloat((uint4)((r1.zzzz)>=(-(r1.zzzz))) * 0xffffffffu)).y;
    // 41: and r0.w, r0.w, r1.y
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.yyyy))).w;
    // 42: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 43: add r0.w, -r0.y, l(0.500000)
    r0.w = ((-(r0.yyyy))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 44: add r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)+(r0.wwww)).w;
    // 45: mul r1.y, v4.w, cb0[4].y
    r1.y = ((v4.wwww)*(source[4].yyyy)).y;
    // 46: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 47: log r1.y, |r0.w|
    r1.y = (log2(abs(r0.wwww))).y;
    // 48: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 49: mul r1.y, r1.y, cb0[4].z
    r1.y = ((r1.yyyy)*(source[4].zzzz)).y;
    // 50: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 51: movc r0.w, r0.w, l(0), r1.y
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).w;
    // 52: mad r2.x, r0.z, l(0.318310), r0.w
    r2.x = ((r0.zzzz)*(float4(asfloat(0x3ea2f983u),asfloat(0x3ea2f983u),asfloat(0x3ea2f983u),asfloat(0x3ea2f983u)))+(r0.wwww)).x;
    // 53: mul r0.z, r2.x, cb0[3].w
    r0.z = ((r2.xxxx)*(source[3].wwww)).z;
    // 54: mul r0.w, cb0[3].x, cb0[3].y
    r0.w = ((source[3].xxxx)*(source[3].yyyy)).w;
    // 55: mad r3.x, r0.w, cb0[3].z, r0.z
    r3.x = ((r0.wwww)*(source[3].zzzz)+(r0.zzzz)).x;
    // 56: add r0.z, r0.y, r0.y
    r0.z = ((r0.yyyy)+(r0.yyyy)).z;
    // 57: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 58: mul r0.z, r0.z, v4.z
    r0.z = ((r0.zzzz)*(v4.zzzz)).z;
    // 59: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 60: lt r1.y, r0.y, l(0.000000)
    r1.y = (asfloat((uint4)((r0.yyyy)<(float4(asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu)))) * 0xffffffffu)).y;
    // 61: mad r0.y, -r0.y, cb0[7].w, l(1.000000)
    r0.y = ((-(r0.yyyy))*(source[7].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 62: mul_sat r0.y, r0.y, cb0[8].w
    r0.y = (saturate((r0.yyyy)*(source[8].wwww))).y;
    // 63: movc r2.y, r1.y, l(0), r0.z
    r2.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 64: mul r0.z, r2.y, cb0[4].x
    r0.z = ((r2.yyyy)*(source[4].xxxx)).z;
    // 65: mul r1.yz, r2.xxyx, cb0[5].yyzy
    r1.yz = ((r2.xxyx)*(source[5].yyzy)).yz;
    // 66: mad r1.yz, r0.wwww, cb0[5].xxwx, r1.yyzy
    r1.yz = ((r0.wwww)*(source[5].xxwx)+(r1.yyzy)).yz;
    // 67: mad r3.y, r0.w, cb0[4].w, r0.z
    r3.y = ((r0.wwww)*(source[4].wwww)+(r0.zzzz)).y;
    // 68: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r3.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xyz = (SDNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).xyz;
    // 69: sample_l_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t1.wxyz, s2, l(0.000000)
    r1.yzw = (SDNativeSample1((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).wxyz).yzw;
    // 70: add r0.z, -r2.x, r1.y
    r0.z = ((-(r2.xxxx))+(r1.yyyy)).z;
    // 71: mad r0.z, r0.z, l(0.500000), r2.x
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(r2.xxxx)).z;
    // 72: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: mul r0.w, r1.x, r0.w
    r0.w = ((r1.xxxx)*(r0.wwww)).w;
    // 74: mul_sat r1.x, r1.x, cb0[6].w
    r1.x = (saturate((r1.xxxx)*(source[6].wwww))).x;
    // 75: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 76: log r0.w, r1.x
    r0.w = (log2(r1.xxxx)).w;
    // 77: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 78: mul r0.w, r0.w, cb0[7].x
    r0.w = ((r0.wwww)*(source[7].xxxx)).w;
    // 79: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 80: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 81: movc r0.y, r1.x, l(0), r0.y
    r0.y = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 82: mad_sat r0.y, r0.z, cb0[6].z, r0.y
    r0.y = (saturate((r0.zzzz)*(source[6].zzzz)+(r0.yyyy))).y;
    // 83: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 84: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 85: mul r0.z, r0.z, cb0[9].x
    r0.z = ((r0.zzzz)*(source[9].xxxx)).z;
    // 86: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 87: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 88: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 89: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 90: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 91: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 92: mul r0.yzw, r1.yyzw, r2.xxyz
    r0.yzw = ((r1.yyzw)*(r2.xxyz)).yzw;
    // 93: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 94: mad r1.xyz, -r2.xyzx, r1.yzwy, r1.xxxx
    r1.xyz = ((-(r2.xyzx))*(r1.yzwy)+(r1.xxxx)).xyz;
    // 95: mad r0.yzw, cb0[6].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[6].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 96: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 97: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 98: mul r0.yzw, r0.yyzw, cb0[6].yyyy
    r0.yzw = ((r0.yyzw)*(source[6].yyyy)).yzw;
    // 99: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 100: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 101: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 102: mad r0.yzw, v3.xxyz, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)*(r0.yyzw)+(source[1].xxyz)).yzw;
    // 103: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 104: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 105: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// Exact D Ice MIC map dcf57a2f..., flocal PS046f090de8eb2f408bbb8debf92fd28f.
// RT0 only: source actor opacity is one; TEXCOORD4 fog is the neutral project adapter.
float4 SDNative324(SD_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[6u];
    source[3] = SDNativeAppend(g_SDSourceMaterialParameters[1u].yyyy,g_SDSourceMaterialParameters[1u].zzzz,1u);
    source[4] = SDNativeAppend(g_SDSourceMaterialParameters[0u].xxxx,g_SDSourceMaterialParameters[0u].zzzz,1u);
    source[5] = g_SDSourceMaterialParameters[4u];
    source[6] = g_SDSourceMaterialParameters[3u];
    source[7] = input.dynamicParameter;
    source[8].x = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[8].y = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[8].z = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[8].w = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[9].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[9].y = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[9].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.yzw, r0.xxxx, v6.xxyz
    r0.yzw = ((r0.xxxx)*(v6.xxyz)).yzw;
    // 4: mul r1.xy, v4.xyxx, cb0[4].xyxx
    r1.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t1.xyzw, s0, l(0.000000)
    r2.xyz = (SDNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 6: mul r3.xyz, r2.xxyx, cb0[8].wxxw
    r3.xyz = ((r2.xxyx)*(source[8].wxxw)).xyz;
    // 7: mad r1.z, r3.x, l(0.100000), l(-0.100000)
    r1.z = ((r3.xxxx)*(float4(0.100000,0.100000,0.100000,0.100000))+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).z;
    // 8: mad r3.xy, -r0.yzyy, cb0[3].xyxx, r3.yzyy
    r3.xy = ((-(r0.yzyy))*(source[3].xyxx)+(r3.yzyy)).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t2.xyzw, s1, l(0.000000)
    r3.xyz = (SDNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 10: mad r0.yz, r1.zzzz, r0.yyzy, r1.xxyx
    r0.yz = ((r1.zzzz)*(r0.yyzy)+(r1.xxyx)).yz;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.wwww, t0.yzwx, s2, l(0.000000)
    r0.w = (SDNativeSample2((r0.wwww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t1.xyzw, s0, l(0.000000)
    r1.xyz = (SDNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 13: add r4.xyzw, -r2.xxyz, r1.xxyz
    r4.xyzw = ((-(r2.xxyz))+(r1.xxyz)).xyzw;
    // 14: mad r2.xyzw, r4.xyzw, l(0.750000, 0.750000, 0.750000, 0.750000), r2.xxyz
    r2.xyzw = ((r4.xyzw)*(float4(0.750000,0.750000,0.750000,0.750000))+(r2.xxyz)).xyzw;
    // 15: mad r2.xyzw, r0.wwww, l(0.174400, 0.174400, 0.174400, 0.174400), r2.xyzw
    r2.xyzw = ((r0.wwww)*(float4(0.174400,0.174400,0.174400,0.174400))+(r2.xyzw)).xyzw;
    // 16: mad r1.xyzw, r1.xxyz, r1.xxyz, -r2.xyzw
    r1.xyzw = ((r1.xxyz)*(r1.xxyz)+(-(r2.xyzw))).xyzw;
    // 17: mad r1.xyzw, r0.wwww, r1.xyzw, r2.xyzw
    r1.xyzw = ((r0.wwww)*(r1.xyzw)+(r2.xyzw)).xyzw;
    // 18: dp3 r0.y, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 19: add r0.yzw, -r1.yyzw, r0.yyyy
    r0.yzw = ((-(r1.yyzw))+(r0.yyyy)).yzw;
    // 20: mad r0.yzw, cb0[9].xxxx, r0.yyzw, r1.yyzw
    r0.yzw = ((source[9].xxxx)*(r0.yyzw)+(r1.yyzw)).yzw;
    // 21: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 22: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 23: mul r0.yzw, r0.yyzw, cb0[9].yyyy
    r0.yzw = ((r0.yyzw)*(source[9].yyyy)).yzw;
    // 24: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 25: mul r0.yzw, r0.yyzw, cb0[6].xxyz
    r0.yzw = ((r0.yyzw)*(source[6].xxyz)).yzw;
    // 26: dp3 r1.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 27: add r1.yzw, -r3.xxyz, r1.yyyy
    r1.yzw = ((-(r3.xxyz))+(r1.yyyy)).yzw;
    // 28: mad r1.yzw, cb0[8].yyyy, r1.yyzw, r3.xxyz
    r1.yzw = ((source[8].yyyy)*(r1.yyzw)+(r3.xxyz)).yzw;
    // 29: max r1.yzw, |r1.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r1.yzw = (max(abs(r1.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 30: log r1.yzw, r1.yyzw
    r1.yzw = (log2(r1.yyzw)).yzw;
    // 31: mul r1.yzw, r1.yyzw, cb0[8].zzzz
    r1.yzw = ((r1.yyzw)*(source[8].zzzz)).yzw;
    // 32: exp r1.yzw, r1.yyzw
    r1.yzw = (exp2(r1.yyzw)).yzw;
    // 33: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 34: mad r0.yzw, r1.yyzw, r2.xxyz, r0.yyzw
    r0.yzw = ((r1.yyzw)*(r2.xxyz)+(r0.yyzw)).yzw;
    // 35: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 36: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 37: mad_sat r0.x, v6.z, r0.x, r1.x
    r0.x = (saturate((v6.zzzz)*(r0.xxxx)+(r1.xxxx))).x;
    // 38: add r0.y, -r1.x, l(2.000000)
    r0.y = ((-(r1.xxxx))+(float4(2.000000,2.000000,2.000000,2.000000))).y;
    // 39: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 40: lt r0.x, r0.x, l(0.000001)
    r0.x = asfloat((r0.x < 0.000001f) ? 0xffffffffu : 0u);
    // 41: mul r0.z, r0.z, cb0[9].z
    r0.z = ((r0.zzzz)*(source[9].zzzz)).z;
    // 42: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 43: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 44: add r0.w, -cb0[7].y, l(1.000000)
    r0.w = ((-(source[7].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: add_sat r0.y, -r0.w, r0.y
    r0.y = (saturate((-(r0.wwww))+(r0.yyyy))).y;
    // 46: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 47: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 48: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 49: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}


// fx_k_me_floorstrm_08_ad: d4f70bb914cad04f99b4cb9a153bba61; selected map 2e7618e243a2509a2f1f25602e58c8965365a302f511f01c98c0463044d7b1ae.
float4 SDNative325(SD_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[8u];
    source[3] = g_SDSourceMaterialParameters[6u];
    source[4] = input.dynamicParameter;
    source[5] = SDNativeAppend(g_SDSourceMaterialParameters[3u].yyyy,g_SDSourceMaterialParameters[3u].zzzz,1u);
    source[6].x = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[6].y = (g_SDSourceMaterialTime.xxxx).x;
    source[6].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[7].z = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[7].w = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[8].x = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[8].y = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[1u].yyyy)).x;
    source[8].w = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[9].x = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[9].y = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[9].z = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[9].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[10].x = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[10].w = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[11].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[3u].wwww)).x;
    source[11].y = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[11].z = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[11].w = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[12].x = (g_SDSourceMaterialParameters[4u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.x, cb0[4].y, l(-1.000000)
    r0.x = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mad r0.y, cb0[6].y, cb0[10].w, cb0[11].x
    r0.y = ((source[6].yyyy)*(source[10].wwww)+(source[11].xxxx)).y;
    // 3: sincos r1.x, r2.x, r0.y
    r1.x = (sin(r0.yyyy)).x; r2.x = (cos(r0.yyyy)).x;
    // 4: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 5: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 6: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 7: mul r0.yz, v4.xxyx, cb0[7].zzwz
    r0.yz = ((v4.xxyx)*(source[7].zzwz)).yz;
    // 8: mul r0.w, cb0[6].x, cb0[6].y
    r0.w = ((source[6].xxxx)*(source[6].yyyy)).w;
    // 9: mad r1.x, r0.w, cb0[7].y, r0.y
    r1.x = ((r0.wwww)*(source[7].yyyy)+(r0.yyyy)).x;
    // 10: mad r1.y, r0.w, cb0[8].x, r0.z
    r1.y = ((r0.wwww)*(source[8].xxxx)+(r0.zzzz)).y;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r1.xyxx, t0.zxyw, s0, l(0.000000)
    r0.yz = (SDNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 12: mad r0.yz, cb0[8].zzzz, r0.yyzy, v4.xxyx
    r0.yz = ((source[8].zzzz)*(r0.yyzy)+(v4.xxyx)).yz;
    // 13: add r1.xy, r0.yzyy, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.yzyy)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 14: dp2 r1.z, r3.yxyy, r1.xyxx
    r1.z = (dot((r3.yxyy).xy,(r1.xyxx).xy).xxxx).z;
    // 15: dp2 r1.x, r3.zyzz, r1.xyxx
    r1.x = (dot((r3.zyzz).xy,(r1.xyxx).xy).xxxx).x;
    // 16: mul r2.z, r1.x, cb0[5].y
    r2.z = ((r1.xxxx)*(source[5].yyyy)).z;
    // 17: mad r2.x, r1.z, cb0[5].x, r0.x
    r2.x = ((r1.zzzz)*(source[5].xxxx)+(r0.xxxx)).x;
    // 18: add r1.xy, r2.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (SDNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 20: mul r1.x, r0.y, cb0[9].w
    r1.x = ((r0.yyyy)*(source[9].wwww)).x;
    // 21: mad r1.x, r0.w, cb0[9].z, r1.x
    r1.x = ((r0.wwww)*(source[9].zzzz)+(r1.xxxx)).x;
    // 22: mul r1.z, r0.z, cb0[10].x
    r1.z = ((r0.zzzz)*(source[10].xxxx)).z;
    // 23: mad r1.y, r0.w, cb0[10].y, r1.z
    r1.y = ((r0.wwww)*(source[10].yyyy)+(r1.zzzz)).y;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.x = (SDNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 25: add r1.y, -cb0[4].x, l(1.000000)
    r1.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 26: mad r0.x, r1.x, r0.x, -r1.y
    r0.x = ((r1.xxxx)*(r0.xxxx)+(-(r1.yyyy))).x;
    // 27: mul_sat r0.x, r0.x, cb0[11].w
    r0.x = (saturate((r0.xxxx)*(source[11].wwww))).x;
    // 28: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 29: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 30: mul r1.x, r1.x, cb0[12].x
    r1.x = ((r1.xxxx)*(source[12].xxxx)).x;
    // 31: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 32: mul_sat r1.x, r1.x, cb0[1].w
    r1.x = (saturate((r1.xxxx)*(source[1].wwww))).x;
    // 33: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 34: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 35: mul r0.y, r0.y, cb0[6].w
    r0.y = ((r0.yyyy)*(source[6].wwww)).y;
    // 36: mad r1.x, r0.w, cb0[6].z, r0.y
    r1.x = ((r0.wwww)*(source[6].zzzz)+(r0.yyyy)).x;
    // 37: mul r0.y, r0.w, cb0[8].w
    r0.y = ((r0.wwww)*(source[8].wwww)).y;
    // 38: mad r1.y, cb0[7].x, r0.z, r0.y
    r1.y = ((source[7].xxxx)*(r0.zzzz)+(r0.yyyy)).y;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t3.wxyz, s1, l(0.000000)
    r0.yzw = (SDNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 40: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 41: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 42: mad r0.yzw, cb0[9].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[9].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 43: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 44: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 45: mul r0.yzw, r0.yyzw, cb0[9].yyyy
    r0.yzw = ((r0.yyzw)*(source[9].yyyy)).yzw;
    // 46: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 47: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 48: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 49: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 50: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 51: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 52: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_w_me_spritewave_01_77_tr: 8d891f77d199a04487b762fd8e6c14cb; selected map 02b5d06f9beff4bd2ba7e2bb9c65aa8cd1e76c4859766b48953e0fa2ac085260.
float4 SDNative326(SD_NATIVE_INPUT input)
{
    float4 source[25]; [unroll] for (uint i=0u; i<25u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[13u];
    source[3] = input.dynamicParameter;
    source[4] = SDNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[6u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[6u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = SDNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[6u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[6u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = SDNativeAppend(g_SDSourceMaterialParameters[5u].yyyy,g_SDSourceMaterialParameters[5u].zzzz,1u);
    source[7] = SDNativeAppend(cos(((g_SDSourceMaterialParameters[6u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[6u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = SDNativeAppend(sin(((g_SDSourceMaterialParameters[6u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_SDSourceMaterialParameters[6u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = SDNativeAppend(cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = SDNativeAppend(sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11] = g_SDSourceMaterialParameters[11u];
    source[12].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[6u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[12].y = (cos(((g_SDSourceMaterialParameters[6u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[12].z = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[12].w = (g_SDSourceMaterialTime.xxxx).x;
    source[13].x = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[13].y = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[13].z = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[13].w = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[14].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[6u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[14].z = (g_SDSourceMaterialParameters[9u].yyyy).x;
    source[14].w = (g_SDSourceMaterialParameters[10u].yyyy).x;
    source[15].x = (g_SDSourceMaterialParameters[10u].zzzz).x;
    source[15].y = (g_SDSourceMaterialParameters[8u].wwww).x;
    source[15].z = (g_SDSourceMaterialParameters[9u].wwww).x;
    source[15].w = (g_SDSourceMaterialParameters[10u].xxxx).x;
    source[16].x = (g_SDSourceMaterialParameters[9u].xxxx).x;
    source[16].y = (g_SDSourceMaterialParameters[8u].yyyy).x;
    source[16].z = (g_SDSourceMaterialParameters[9u].zzzz).x;
    source[16].w = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[17].x = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[17].y = (g_SDSourceMaterialParameters[8u].zzzz).x;
    source[17].z = (g_SDSourceMaterialParameters[5u].zzzz).x;
    source[17].w = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[18].x = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[18].y = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[18].z = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[18].w = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[19].x = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[19].y = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[19].z = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[19].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[20].x = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[20].y = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[20].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[20].w = ((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[21].x = (sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[21].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[21].z = (cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[21].w = (g_SDSourceMaterialParameters[7u].yyyy).x;
    source[22].x = (g_SDSourceMaterialParameters[7u].wwww).x;
    source[22].y = (g_SDSourceMaterialParameters[8u].xxxx).x;
    source[22].z = (g_SDSourceMaterialParameters[7u].zzzz).x;
    source[22].w = (g_SDSourceMaterialParameters[7u].xxxx).x;
    source[23].x = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[23].y = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[23].z = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[23].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[24].x = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[24].y = (g_SDSourceMaterialParameters[4u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[15].zwzz
    r0.xy = ((v4.xyxx)*(source[15].zwzz)).xy;
    // 2: mad r1.x, cb0[12].w, cb0[15].y, r0.x
    r1.x = ((source[12].wwww)*(source[15].yyyy)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[12].w, cb0[16].x, r0.y
    r1.y = ((source[12].wwww)*(source[16].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (SDNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: mad r0.xy, r0.xxxx, cb0[16].yyyy, v4.xyxx
    r0.xy = ((r0.xxxx)*(source[16].yyyy)+(v4.xyxx)).xy;
    // 6: mul r0.z, cb0[12].w, cb0[14].z
    r0.z = ((source[12].wwww)*(source[14].zzzz)).z;
    // 7: mad r1.x, cb0[14].w, r0.x, r0.z
    r1.x = ((source[14].wwww)*(r0.xxxx)+(r0.zzzz)).x;
    // 8: mul r0.x, cb0[12].w, cb0[16].z
    r0.x = ((source[12].wwww)*(source[16].zzzz)).x;
    // 9: mad r1.y, cb0[15].x, r0.y, r0.x
    r1.y = ((source[15].xxxx)*(r0.yyyy)+(r0.xxxx)).y;
    // 10: mul r0.x, cb0[3].w, cb0[16].w
    r0.x = ((source[3].wwww)*(source[16].wwww)).x;
    // 11: mul r0.y, cb0[3].w, cb0[17].x
    r0.y = ((source[3].wwww)*(source[17].xxxx)).y;
    // 12: add r0.xy, r0.xyxx, r1.xyxx
    r0.xy = ((r0.xyxx)+(r1.xyxx)).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (SDNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 14: add r0.y, cb0[3].z, cb0[17].y
    r0.y = ((source[3].zzzz)+(source[17].yyyy)).y;
    // 15: mad r0.xy, r0.xxxx, r0.yyyy, cb0[6].xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(source[6].xyxx)).xy;
    // 16: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 17: dp2 r1.x, cb0[4].xyxx, r0.zwzz
    r1.x = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 18: dp2 r1.y, cb0[5].xyxx, r0.zwzz
    r1.y = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 19: mad r1.xy, cb0[13].zwzz, cb0[3].xxxx, r1.xyxx
    r1.xy = ((source[13].zwzz)*(source[3].xxxx)+(r1.xyxx)).xy;
    // 20: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 21: mul r1.xy, r1.xyxx, cb0[13].xyxx
    r1.xy = ((r1.xyxx)*(source[13].xyxx)).xy;
    // 22: mad r2.x, cb0[12].w, cb0[12].z, r1.x
    r2.x = ((source[12].wwww)*(source[12].zzzz)+(r1.xxxx)).x;
    // 23: mad r2.y, cb0[12].w, cb0[14].y, r1.y
    r2.y = ((source[12].wwww)*(source[14].yyyy)+(r1.yyyy)).y;
    // 24: add r0.xy, r0.xyxx, r2.xyxx
    r0.xy = ((r0.xyxx)+(r2.xyxx)).xy;
    // 25: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 26: dp2 r1.x, cb0[7].xyxx, r0.xyxx
    r1.x = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 27: dp2 r1.y, cb0[8].xyxx, r0.xyxx
    r1.y = (dot((source[8].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 28: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 29: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s0, l(-1.000000)
    r0.x = (SDNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 30: mul r1.xy, v4.xyxx, cb0[18].yzyy
    r1.xy = ((v4.xyxx)*(source[18].yzyy)).xy;
    // 31: mad r1.xy, cb0[12].wwww, cb0[18].xwxx, r1.xyxx
    r1.xy = ((source[12].wwww)*(source[18].xwxx)+(r1.xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t5.yxzw, s3, l(0.000000)
    r0.y = (SDNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 33: mul r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)*(r0.xxxx)).y;
    // 34: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 35: mul r1.x, r1.x, cb0[19].x
    r1.x = ((r1.xxxx)*(source[19].xxxx)).x;
    // 36: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 37: mul r1.x, r1.x, cb0[19].y
    r1.x = ((r1.xxxx)*(source[19].yyyy)).x;
    // 38: lt r1.y, |r0.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 39: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 40: mad r0.y, r0.y, cb0[19].z, r1.x
    r0.y = ((r0.yyyy)*(source[19].zzzz)+(r1.xxxx)).y;
    // 41: dp2 r1.x, cb0[9].xyxx, r0.zwzz
    r1.x = (dot((source[9].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 42: dp2 r1.y, cb0[10].xyxx, r0.zwzz
    r1.y = (dot((source[10].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 43: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 44: mul r1.xy, v4.xyxx, cb0[22].xyxx
    r1.xy = ((v4.xyxx)*(source[22].xyxx)).xy;
    // 45: mad r2.x, cb0[12].w, cb0[21].w, r1.x
    r2.x = ((source[12].wwww)*(source[21].wwww)+(r1.xxxx)).x;
    // 46: mad r2.y, cb0[12].w, cb0[22].z, r1.y
    r2.y = ((source[12].wwww)*(source[22].zzzz)+(r1.yyyy)).y;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r2.xyxx, t3.xyzw, s4, l(0.000000)
    r1.x = (SDNativeSample4((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 48: mad r0.zw, r1.xxxx, cb0[22].wwww, r0.zzzw
    r0.zw = ((r1.xxxx)*(source[22].wwww)+(r0.zzzw)).zw;
    // 49: mul r0.zw, r0.zzzw, cb0[20].xxxy
    r0.zw = ((r0.zzzw)*(source[20].xxxy)).zw;
    // 50: mad r1.x, cb0[12].w, cb0[19].w, r0.z
    r1.x = ((source[12].wwww)*(source[19].wwww)+(r0.zzzz)).x;
    // 51: mad r1.y, cb0[12].w, cb0[23].x, r0.w
    r1.y = ((source[12].wwww)*(source[23].xxxx)+(r0.wwww)).y;
    // 52: add r0.zw, r1.xxxy, cb0[23].yyyz
    r0.zw = ((r1.xxxy)+(source[23].yyyz)).zw;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t4.yzxw, s5, l(0.000000)
    r0.z = (SDNativeSample5((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 54: add r0.z, r0.z, l(0.100000)
    r0.z = ((r0.zzzz)+(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 55: add r0.w, cb0[3].y, l(-1.000000)
    r0.w = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 56: add_sat r0.z, -r0.w, r0.z
    r0.z = (saturate((-(r0.wwww))+(r0.zzzz))).z;
    // 57: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 58: mul r0.w, r0.w, cb0[24].x
    r0.w = ((r0.wwww)*(source[24].xxxx)).w;
    // 59: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 60: mul_sat r0.w, r0.w, cb0[23].w
    r0.w = (saturate((r0.wwww)*(source[23].wwww))).w;
    // 61: lt r1.x, r0.z, l(0.000001)
    r1.x = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 62: mul r0.z, r0.z, cb0[23].w
    r0.z = ((r0.zzzz)*(source[23].wwww)).z;
    // 63: movc r0.w, r1.x, l(-0.000000), -r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.wwww))).w;
    // 64: mov_sat r1.x, r0.z
    r1.x = (saturate(r0.zzzz)).x;
    // 65: mul r0.z, r0.z, r0.x
    r0.z = ((r0.zzzz)*(r0.xxxx)).z;
    // 66: mul r0.z, r0.z, cb0[24].y
    r0.z = ((r0.zzzz)*(source[24].yyyy)).z;
    // 67: mul_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)*(source[1].wwww))).z;
    // 68: mul o0.w, r0.z, cb0[0].x
    output.w = ((r0.zzzz)*(source[0].xxxx)).w;
    // 69: add r0.z, r0.w, r1.x
    r0.z = ((r0.wwww)+(r1.xxxx)).z;
    // 70: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 71: mul r1.xyz, r0.zzzz, cb0[11].xyzx
    r1.xyz = ((r0.zzzz)*(source[11].xyzx)).xyz;
    // 72: mad r0.xyz, r0.xxxx, r1.xyzx, r0.yyyy
    r0.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yyyy)).xyz;
    // 73: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 74: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_me_ap_07_6_ts_tr: f3e012370cd62d4f9a62c9c2d507b775; selected map 3f47e48618d77fb4eb098e3fb3db46f5a5252affc813ce1120fd377c1569187b.
float4 SDNative327(SD_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[3u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_SDSourceMaterialParameters[1u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.x, cb0[4].x, cb0[3].x, l(-1.000000)
    r0.x = ((source[4].xxxx)*(source[3].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, cb0[3].x, cb0[4].x
    r0.y = ((source[3].xxxx)*(source[4].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v4.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v4.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (SDNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 7: mul r1.x, r1.x, cb0[5].y
    r1.x = ((r1.xxxx)*(source[5].yyyy)).x;
    // 8: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 9: mul r1.x, r1.x, cb0[1].w
    r1.x = ((r1.xxxx)*(source[1].wwww)).x;
    // 10: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 11: movc r0.w, r0.w, l(0), |r1.x|
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).w;
    // 12: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 13: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 14: mul r1.x, r1.x, cb0[3].y
    r1.x = ((r1.xxxx)*(source[3].yyyy)).x;
    // 15: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 16: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 17: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 18: movc o0.w, r0.w, l(0), r1.x
    output.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 19: mul r1.xyz, r0.xyzx, cb0[4].yyyy
    r1.xyz = ((r0.xyzx)*(source[4].yyyy)).xyz;
    // 20: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: mad r0.xyz, -cb0[4].yyyy, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[4].yyyy))*(r0.xyzx)+(r0.wwww)).xyz;
    // 22: mad r0.xyz, cb0[4].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[4].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: mul r1.xyz, r0.xyzx, cb0[4].wwww
    r1.xyz = ((r0.xyzx)*(source[4].wwww)).xyz;
    // 24: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 25: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 26: mul r1.xyz, r1.xyzx, cb0[5].xxxx
    r1.xyz = ((r1.xyzx)*(source[5].xxxx)).xyz;
    // 27: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 28: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 29: add r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)+(r0.wwww)).xyz;
    // 30: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 31: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_me_floorstrm_21_ad: 259a473908f05249ada316be7fd9e944; selected map 3d73f296b453dceeff00a160cddd21fd8ec8b1258c00c57a3ff9f9495f0aed4e.
float4 SDNative328(SD_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[9u];
    source[3] = g_SDSourceMaterialParameters[7u];
    source[4] = input.dynamicParameter;
    source[5] = SDNativeAppend(g_SDSourceMaterialParameters[4u].yyyy,g_SDSourceMaterialParameters[4u].zzzz,1u);
    source[6].x = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[6].y = (g_SDSourceMaterialTime.xxxx).x;
    source[6].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[7].z = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[7].w = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[8].x = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[8].y = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[1u].yyyy)).x;
    source[8].w = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[9].x = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[9].y = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[9].w = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[10].x = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[10].y = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[10].z = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[10].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[11].x = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[11].y = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[11].z = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[11].w = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[12].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[4u].wwww)).x;
    source[12].y = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[12].z = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[12].w = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[13].x = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[13].y = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[13].z = (g_SDSourceMaterialParameters[5u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.x, r0.x, v6.z
    r0.x = ((r0.xxxx)*(v6.zzzz)).x;
    // 4: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 5: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 6: mul r0.y, r0.y, cb0[13].y
    r0.y = ((r0.yyyy)*(source[13].yyyy)).y;
    // 7: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 8: mul_sat r0.y, r0.y, cb0[13].z
    r0.y = (saturate((r0.yyyy)*(source[13].zzzz))).y;
    // 9: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 10: add r0.y, -cb0[4].x, l(1.000000)
    r0.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: add r0.z, cb0[4].y, l(-1.000000)
    r0.z = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 12: mad r0.w, cb0[6].y, cb0[11].w, cb0[12].x
    r0.w = ((source[6].yyyy)*(source[11].wwww)+(source[12].xxxx)).w;
    // 13: sincos r1.x, r2.x, r0.w
    r1.x = (sin(r0.wwww)).x; r2.x = (cos(r0.wwww)).x;
    // 14: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 15: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 16: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 17: mul r1.xy, v4.xyxx, cb0[7].zwzz
    r1.xy = ((v4.xyxx)*(source[7].zwzz)).xy;
    // 18: mul r0.w, cb0[6].x, cb0[6].y
    r0.w = ((source[6].xxxx)*(source[6].yyyy)).w;
    // 19: mad r2.x, r0.w, cb0[7].y, r1.x
    r2.x = ((r0.wwww)*(source[7].yyyy)+(r1.xxxx)).x;
    // 20: mad r2.y, r0.w, cb0[8].x, r1.y
    r2.y = ((r0.wwww)*(source[8].xxxx)+(r1.yyyy)).y;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (SDNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 22: mad r1.xy, cb0[8].zzzz, r1.xyxx, v4.xyxx
    r1.xy = ((source[8].zzzz)*(r1.xyxx)+(v4.xyxx)).xy;
    // 23: add r1.zw, r1.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r1.zw = ((r1.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 24: dp2 r2.x, r3.yxyy, r1.zwzz
    r2.x = (dot((r3.yxyy).xy,(r1.zwzz).xy).xxxx).x;
    // 25: dp2 r1.z, r3.zyzz, r1.zwzz
    r1.z = (dot((r3.zyzz).xy,(r1.zwzz).xy).xxxx).z;
    // 26: mul r3.z, r1.z, cb0[5].y
    r3.z = ((r1.zzzz)*(source[5].yyyy)).z;
    // 27: mad r3.x, r2.x, cb0[5].x, r0.z
    r3.x = ((r2.xxxx)*(source[5].xxxx)+(r0.zzzz)).x;
    // 28: add r1.zw, r3.xxxz, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r3.xxxz)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t2.yzxw, s4, l(0.000000)
    r0.z = (SDNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 30: mul r1.z, r1.x, cb0[10].w
    r1.z = ((r1.xxxx)*(source[10].wwww)).z;
    // 31: mad r2.x, r0.w, cb0[10].z, r1.z
    r2.x = ((r0.wwww)*(source[10].zzzz)+(r1.zzzz)).x;
    // 32: mul r1.z, r1.y, cb0[11].x
    r1.z = ((r1.yyyy)*(source[11].xxxx)).z;
    // 33: mad r2.y, r0.w, cb0[11].y, r1.z
    r2.y = ((r0.wwww)*(source[11].yyyy)+(r1.zzzz)).y;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r2.xyxx, t1.yzxw, s3, l(0.000000)
    r1.z = (SDNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 35: mad r0.y, r1.z, r0.z, -r0.y
    r0.y = ((r1.zzzz)*(r0.zzzz)+(-(r0.yyyy))).y;
    // 36: mul_sat r0.y, r0.y, cb0[12].w
    r0.y = (saturate((r0.yyyy)*(source[12].wwww))).y;
    // 37: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 38: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 39: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 40: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 41: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 42: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 43: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 44: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 45: mul r0.y, r1.x, cb0[6].w
    r0.y = ((r1.xxxx)*(source[6].wwww)).y;
    // 46: mad r2.x, r0.w, cb0[6].z, r0.y
    r2.x = ((r0.wwww)*(source[6].zzzz)+(r0.yyyy)).x;
    // 47: mul r0.y, r0.w, cb0[8].w
    r0.y = ((r0.wwww)*(source[8].wwww)).y;
    // 48: mad r2.y, cb0[7].x, r1.y, r0.y
    r2.y = ((source[7].xxxx)*(r1.yyyy)+(r0.yyyy)).y;
    // 49: mul r0.yz, r1.xxyx, cb0[9].yyzy
    r0.yz = ((r1.xxyx)*(source[9].yyzy)).yz;
    // 50: mad r0.yz, r0.wwww, cb0[9].xxwx, r0.yyzy
    r0.yz = ((r0.wwww)*(source[9].xxwx)+(r0.yyzy)).yz;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t4.wxyz, s2, l(0.000000)
    r0.yzw = (SDNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t3.xyzw, s1, l(0.000000)
    r1.xyz = (SDNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 54: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 55: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 56: mad r0.yzw, cb0[10].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[10].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 57: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 58: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 59: mul r0.yzw, r0.yyzw, cb0[10].yyyy
    r0.yzw = ((r0.yyzw)*(source[10].yyyy)).yzw;
    // 60: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 61: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 62: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 63: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 64: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 65: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 66: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_g_me_field_01_1_tr: 77adcdc49647a94bb58bfa143ba20152; selected map 321195b0658adca3996ab019766c311b2053ba9ff98addb7061807c2c03a0557.
float4 SDNative329(SD_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[12u];
    source[3] = g_SDSourceMaterialParameters[8u];
    source[4] = input.dynamicParameter;
    source[5] = SDNativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[6] = g_SDSourceMaterialParameters[9u];
    source[7] = g_SDSourceMaterialParameters[10u];
    source[8] = SDNativeAppend(sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[0u].yyyy)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[0u].yyyy)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[9].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[9].y = (g_SDSourceMaterialTime.xxxx).x;
    source[9].z = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[9].w = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[10].x = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[10].y = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[10].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[10].w = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[11].x = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[11].y = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[11].z = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[11].w = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[12].x = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[12].y = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[12].z = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[12].w = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[13].x = (g_SDSourceMaterialParameters[5u].zzzz).x;
    source[13].y = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[13].z = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[13].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[14].x = (cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[14].z = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[15].x = (g_SDSourceMaterialParameters[7u].xxxx).x;
    source[15].y = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[15].z = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[15].w = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[16].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[0u].yyyy)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[16].y = (cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[0u].yyyy)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[16].z = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[16].w = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[17].x = (g_SDSourceMaterialParameters[7u].yyyy).x;
    source[17].y = (g_SDSourceMaterialParameters[7u].zzzz).x;
    source[17].z = (g_SDSourceMaterialParameters[1u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: mov_sat r0.x, cb0[4].x
    r0.x = (saturate(source[4].xxxx)).x;
    // 2: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 3: mul r0.yz, v4.xxyx, cb0[10].yyzy
    r0.yz = ((v4.xxyx)*(source[10].yyzy)).yz;
    // 4: mad r0.yz, cb0[9].yyyy, cb0[10].xxwx, r0.yyzy
    r0.yz = ((source[9].yyyy)*(source[10].xxwx)+(r0.yyzy)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (SDNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mad r0.yz, r0.yyyy, cb0[11].xxxx, v4.xxyx
    r0.yz = ((r0.yyyy)*(source[11].xxxx)+(v4.xxyx)).yz;
    // 7: mul r0.w, r0.z, cb0[13].x
    r0.w = ((r0.zzzz)*(source[13].xxxx)).w;
    // 8: mad r1.w, cb0[9].y, cb0[13].y, r0.w
    r1.w = ((source[9].yyyy)*(source[13].yyyy)+(r0.wwww)).w;
    // 9: mul r2.xy, r0.zyzz, cb0[12].xwxx
    r2.xy = ((r0.zyzz)*(source[12].xwxx)).xy;
    // 10: mad r1.yz, cb0[9].yyyy, cb0[12].yyzy, r2.xxyx
    r1.yz = ((source[9].yyyy)*(source[12].yyzy)+(r2.xxyx)).yz;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t3.xyzw, s3, l(0.000000)
    r2.xyz = (SDNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 12: mul r0.w, cb0[9].y, cb0[11].z
    r0.w = ((source[9].yyyy)*(source[11].zzzz)).w;
    // 13: mad r1.x, cb0[11].w, r0.y, r0.w
    r1.x = ((source[11].wwww)*(r0.yyyy)+(r0.wwww)).x;
    // 14: mul r0.yz, r0.yyzy, cb0[9].zzwz
    r0.yz = ((r0.yyzy)*(source[9].zzwz)).yz;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (SDNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 16: mad r3.x, cb0[9].y, cb0[9].x, r0.y
    r3.x = ((source[9].yyyy)*(source[9].xxxx)+(r0.yyyy)).x;
    // 17: mad r3.y, cb0[9].y, cb0[11].y, r0.z
    r3.y = ((source[9].yyyy)*(source[11].yyyy)+(r0.zzzz)).y;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r3.xyxx, t1.wxyz, s1, l(0.000000)
    r0.yzw = (SDNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 19: mad r3.xy, r0.yyyy, r1.xxxx, r2.xxxx
    r3.xy = ((r0.yyyy)*(r1.xxxx)+(r2.xxxx)).xy;
    // 20: add r4.xyzw, v4.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r4.xyzw = ((v4.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 21: dp4 r1.w, cb0[5].xxyy, r4.zzww
    r1.w = (dot((source[5].xxyy).xyzw,(r4.zzww).xyzw).xxxx).w;
    // 22: dp4 r2.w, cb0[8].xyxy, r4.xyzw
    r2.w = (dot((source[8].xyxy).xyzw,(r4.xyzw).xyzw).xxxx).w;
    // 23: add r1.w, |r1.w|, l(0.100000)
    r1.w = ((abs(r1.wwww))+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 24: mad r3.yz, r3.xxyx, cb0[13].zzzz, r1.wwww
    r3.yz = ((r3.xxyx)*(source[13].zzzz)+(r1.wwww)).yz;
    // 25: add r3.yz, -r0.xxxx, r3.yyzy
    r3.yz = ((-(r0.xxxx))+(r3.yyzy)).yz;
    // 26: mul r0.x, cb0[4].z, cb0[14].y
    r0.x = ((source[4].zzzz)*(source[14].yyyy)).x;
    // 27: mul_sat r3.yz, r0.xxxx, r3.yyzy
    r3.yz = (saturate((r0.xxxx)*(r3.yyzy))).yz;
    // 28: log r4.xy, r3.yzyy
    r4.xy = (log2(r3.yzyy)).xy;
    // 29: lt r3.yz, r3.yyzy, l(0.000000, 0.000001, 0.000001, 0.000000)
    r3.yz = (asfloat((uint4)((r3.yyzy)<(float4(0.000000,0.000001,0.000001,0.000000))) * 0xffffffffu)).yz;
    // 30: mul r4.xy, r4.xyxx, cb0[14].zzzz
    r4.xy = ((r4.xyxx)*(source[14].zzzz)).xy;
    // 31: exp r4.xy, r4.xyxx
    r4.xy = (exp2(r4.xyxx)).xy;
    // 32: movc r3.yz, r3.yyzy, l(0,0,0,0), r4.xxyx
    r3.yz = ((asuint(r3.yyzy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxyx)).yz;
    // 33: add r0.x, -r3.z, l(1.000000)
    r0.x = ((-(r3.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 34: mad r2.xyz, r3.zzzz, r0.xxxx, r2.xyzx
    r2.xyz = ((r3.zzzz)*(r0.xxxx)+(r2.xyzx)).xyz;
    // 35: mul r0.x, r0.x, r3.z
    r0.x = ((r0.xxxx)*(r3.zzzz)).x;
    // 36: mul r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 37: mul r2.xyz, r2.xyzx, cb0[14].wwww
    r2.xyz = ((r2.xyzx)*(source[14].wwww)).xyz;
    // 38: mul_sat r2.xyz, r2.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000)
    r2.xyz = (saturate((r2.xyzx)*(float4(4.000000,4.000000,4.000000,0.000000)))).xyz;
    // 39: max r2.xyz, r2.xyzx, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 40: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 41: mul r2.xyz, r2.xyzx, cb0[15].xxxx
    r2.xyz = ((r2.xyzx)*(source[15].xxxx)).xyz;
    // 42: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 43: mul r4.xyz, r1.xyzx, r0.yzwy
    r4.xyz = ((r1.xyzx)*(r0.yzwy)).xyz;
    // 44: dp3 r0.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 45: mad r0.xyz, -r0.yzwy, r1.xyzx, r0.xxxx
    r0.xyz = ((-(r0.yzwy))*(r1.xyzx)+(r0.xxxx)).xyz;
    // 46: mad r0.xyz, cb0[15].yyyy, r0.xyzx, r4.xyzx
    r0.xyz = ((source[15].yyyy)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 47: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 48: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 49: mul r0.xyz, r0.xyzx, cb0[15].zzzz
    r0.xyz = ((r0.xyzx)*(source[15].zzzz)).xyz;
    // 50: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 51: mul_sat r0.xyz, r0.xyzx, cb0[15].wwww
    r0.xyz = (saturate((r0.xyzx)*(source[15].wwww))).xyz;
    // 52: mul r1.xyz, cb0[6].xyzx, cb0[6].wwww
    r1.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 53: mad r4.xyz, cb0[7].wwww, cb0[7].xyzx, -r1.xyzx
    r4.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r1.xyzx))).xyz;
    // 54: mad r0.xyz, r0.xyzx, r4.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 55: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 56: mul r1.xyz, r1.xyzx, cb0[4].yyyy
    r1.xyz = ((r1.xyzx)*(source[4].yyyy)).xyz;
    // 57: mad r0.xyz, r1.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 58: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 59: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 60: mul r0.x, |r2.w|, |r2.w|
    r0.x = ((abs(r2.wwww))*(abs(r2.wwww))).x;
    // 61: mad r0.x, -|r2.w|, r0.x, l(1.000000)
    r0.x = ((-(abs(r2.wwww)))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 62: lt r0.y, |r2.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 63: movc r0.x, r0.y, l(1.000000), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.xxxx)).x;
    // 64: add r0.y, r0.x, r3.x
    r0.y = ((r0.xxxx)+(r3.xxxx)).y;
    // 65: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 66: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 67: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 68: mul r0.x, r0.x, cb0[16].z
    r0.x = ((r0.xxxx)*(source[16].zzzz)).x;
    // 69: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 70: mul_sat r0.x, r0.x, cb0[16].w
    r0.x = (saturate((r0.xxxx)*(source[16].wwww))).x;
    // 71: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 72: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 73: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 74: lt r0.w, |r0.z|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 75: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 76: mul r0.z, r0.z, cb0[17].x
    r0.z = ((r0.zzzz)*(source[17].xxxx)).z;
    // 77: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 78: mul_sat r0.z, r0.z, cb0[17].y
    r0.z = (saturate((r0.zzzz)*(source[17].yyyy))).z;
    // 79: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 80: or r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) | asuint(r0.wwww))).y;
    // 81: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 82: mul r0.x, r0.x, r3.y
    r0.x = ((r0.xxxx)*(r3.yyyy)).x;
    // 83: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 84: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 85: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

// fx_d_me_electric_03_13_ad: 37f0fc0820aa8047a9777d4765e071a4; selected map 1b5945d178cb7a08bac79876a6f33f050ebf73634f7ab73346c631f87eca80c2.
float4 SDNative330(SD_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[5u];
    source[3] = SDNativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = SDNativeAppend(sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5] = input.dynamicParameter;
    source[6] = SDNativeAppend(SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[7] = SDNativeAppend(g_SDSourceMaterialParameters[0u].zzzz,g_SDSourceMaterialParameters[1u].xxxx,1u);
    source[8].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[8].y = (cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[8].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[9].x = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[9].y = (g_SDSourceMaterialTime.xxxx).x;
    source[9].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[9].w = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[10].x = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[10].y = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[10].w = ((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].yyyy)).x;
    source[11].x = (((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[11].y = (((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[11].z = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[11].w = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[12].x = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[12].y = (SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[12].z = (SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[13].x = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[13].y = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[13].z = (g_SDSourceMaterialParameters[3u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mul r0.x, cb0[5].x, cb0[11].w
    r0.x = ((source[5].xxxx)*(source[11].wwww)).x;
    // 2: add r1.xyzw, v4.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r1.xyzw = ((v4.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 3: dp2 r0.y, cb0[4].xyxx, r1.zwzz
    r0.y = (dot((source[4].xyxx).xy,(r1.zwzz).xy).xxxx).y;
    // 4: dp4 r0.z, cb0[3].xyxy, r1.xyzw
    r0.z = (dot((source[3].xyxy).xyzw,(r1.xyzw).xyzw).xxxx).z;
    // 5: add r1.y, r0.y, l(0.500000)
    r1.y = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 6: add r0.y, r1.y, cb0[5].y
    r0.y = ((r1.yyyy)+(source[5].yyyy)).y;
    // 7: mul r0.y, r0.y, cb0[11].z
    r0.y = ((r0.yyyy)*(source[11].zzzz)).y;
    // 8: mul r0.y, r0.y, l(6.283185)
    r0.y = ((r0.yyyy)*(float4(asfloat(0x40c90fdbu),asfloat(0x40c90fdbu),asfloat(0x40c90fdbu),asfloat(0x40c90fdbu)))).y;
    // 9: sincos r0.y, null, r0.y
    r0.y = (sin(r0.yyyy)).y;
    // 10: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 11: add r0.y, -r1.y, l(1.000000)
    r0.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 12: mul r0.w, r0.y, r1.y
    r0.w = ((r0.yyyy)*(r1.yyyy)).w;
    // 13: mul r0.w, r0.w, l(4.000000)
    r0.w = ((r0.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 14: mad r0.x, r0.x, r0.w, r0.z
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.zzzz)).x;
    // 15: div r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)/(r0.yyyy)).x;
    // 16: mad_sat r0.x, r0.x, l(0.500000), l(0.500000)
    r0.x = (saturate((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000)))).x;
    // 17: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 18: mad_sat r1.x, r0.x, cb0[12].x, l(0.500000)
    r1.x = (saturate((r0.xxxx)*(source[12].xxxx)+(float4(0.500000,0.500000,0.500000,0.500000)))).x;
    // 19: add r0.xz, r1.xxyx, cb0[6].xxyx
    r0.xz = ((r1.xxyx)+(source[6].xxyx)).xz;
    // 20: mul r0.xz, r0.xxzx, cb0[7].xxyx
    r0.xz = ((r0.xxzx)*(source[7].xxyx)).xz;
    // 21: mul r1.xy, v4.xyxx, cb0[9].zwzz
    r1.xy = ((v4.xyxx)*(source[9].zwzz)).xy;
    // 22: mad r2.x, cb0[9].y, cb0[9].x, r1.x
    r2.x = ((source[9].yyyy)*(source[9].xxxx)+(r1.xxxx)).x;
    // 23: mad r2.y, cb0[9].y, cb0[10].x, r1.y
    r2.y = ((source[9].yyyy)*(source[10].xxxx)+(r1.yyyy)).y;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.x = (SDNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 25: mul r1.y, cb0[5].w, cb0[10].y
    r1.y = ((source[5].wwww)*(source[10].yyyy)).y;
    // 26: mad r0.xz, r1.xxxx, r1.yyyy, r0.xxzx
    r0.xz = ((r1.xxxx)*(r1.yyyy)+(r0.xxzx)).xz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t1.xyzw, s1, l(0.000000)
    r0.x = (SDNativeSample1((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 28: add r0.z, -cb0[5].z, l(1.000000)
    r0.z = ((-(source[5].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: add_sat r0.x, -r0.z, r0.x
    r0.x = (saturate((-(r0.zzzz))+(r0.xxxx))).x;
    // 30: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 31: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 32: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 33: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 34: mul_sat r0.z, |r0.z|, cb0[13].y
    r0.z = (saturate((abs(r0.zzzz))*(source[13].yyyy))).z;
    // 35: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 36: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 37: mul r0.w, r0.w, cb0[13].z
    r0.w = ((r0.wwww)*(source[13].zzzz)).w;
    // 38: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 39: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 40: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 41: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 42: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 43: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 44: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 45: mul r0.z, r0.z, cb0[8].z
    r0.z = ((r0.zzzz)*(source[8].zzzz)).z;
    // 46: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 47: mad r0.z, r0.z, cb0[8].w, l(1.000000)
    r0.z = ((r0.zzzz)*(source[8].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 48: movc r0.y, r0.y, l(1.000000), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.zzzz)).y;
    // 49: mad r0.yzw, r0.yyyy, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyyy)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 50: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 51: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 52: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_s_me_spritewave_03_18_tr: d57edd39f1227745b96a1326b1740a33; selected map 9f98b24cce381fc6ce6cdab21b064ee4f0e10b0c2585d9ec7a3cff2fafe709f5.
float4 SDNative331(SD_NATIVE_INPUT input)
{
    float4 source[21]; [unroll] for (uint i=0u; i<21u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[9u];
    source[3] = input.dynamicParameter;
    source[4] = SDNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[6u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[6u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = SDNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[6u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[6u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = (float4(0.0, 0.0, 0.0, 0.0)+SDNativeAppend(g_SDSourceMaterialParameters[5u].xxxx,g_SDSourceMaterialParameters[5u].yyyy,1u));
    source[7] = SDNativeAppend(cos(((g_SDSourceMaterialParameters[6u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[6u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = SDNativeAppend(sin(((g_SDSourceMaterialParameters[6u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_SDSourceMaterialParameters[6u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = SDNativeAppend(cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = SDNativeAppend(sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11] = g_SDSourceMaterialParameters[7u];
    source[12].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[6u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[12].y = (cos(((g_SDSourceMaterialParameters[6u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[12].z = (g_SDSourceMaterialParameters[5u].zzzz).x;
    source[12].w = (g_SDSourceMaterialTime.xxxx).x;
    source[13].x = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[13].y = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[13].z = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[13].w = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[14].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[6u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[14].z = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[14].w = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[15].x = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[15].y = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[15].z = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[15].w = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[16].x = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[16].y = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[16].z = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[16].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[17].x = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[17].y = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[17].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[17].w = ((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[18].x = (sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[18].z = (cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].w = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[19].x = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[19].y = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[19].z = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[19].w = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[20].x = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[20].y = (g_SDSourceMaterialParameters[4u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[15].yzyy
    r0.xy = ((v4.xyxx)*(source[15].yzyy)).xy;
    // 2: mad r0.xy, cb0[12].wwww, cb0[15].xwxx, r0.xyxx
    r0.xy = ((source[12].wwww)*(source[15].xwxx)+(r0.xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.x = (SDNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 5: dp2 r1.x, cb0[4].xyxx, r0.yzyy
    r1.x = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 6: dp2 r1.y, cb0[5].xyxx, r0.yzyy
    r1.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 7: mad r1.xy, cb0[13].zwzz, cb0[3].xxxx, r1.xyxx
    r1.xy = ((source[13].zwzz)*(source[3].xxxx)+(r1.xyxx)).xy;
    // 8: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 9: mul r1.xy, r1.xyxx, cb0[13].xyxx
    r1.xy = ((r1.xyxx)*(source[13].xyxx)).xy;
    // 10: mad r2.x, cb0[12].w, cb0[12].z, r1.x
    r2.x = ((source[12].wwww)*(source[12].zzzz)+(r1.xxxx)).x;
    // 11: mad r2.y, cb0[12].w, cb0[14].y, r1.y
    r2.y = ((source[12].wwww)*(source[14].yyyy)+(r1.yyyy)).y;
    // 12: add r1.xy, r2.xyxx, cb0[6].xyxx
    r1.xy = ((r2.xyxx)+(source[6].xyxx)).xy;
    // 13: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 14: dp2 r2.x, cb0[7].xyxx, r1.xyxx
    r2.x = (dot((source[7].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 15: dp2 r2.y, cb0[8].xyxx, r1.xyxx
    r2.y = (dot((source[8].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 16: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 17: sample_l_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s0, l(-1.000000)
    r0.w = (SDNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 18: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 19: log r1.x, |r0.x|
    r1.x = (log2(abs(r0.xxxx))).x;
    // 20: mul r1.x, r1.x, cb0[16].x
    r1.x = ((r1.xxxx)*(source[16].xxxx)).x;
    // 21: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 22: mul r1.x, r1.x, cb0[16].y
    r1.x = ((r1.xxxx)*(source[16].yyyy)).x;
    // 23: lt r1.y, |r0.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 24: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 25: mad r0.x, r0.x, cb0[16].z, r1.x
    r0.x = ((r0.xxxx)*(source[16].zzzz)+(r1.xxxx)).x;
    // 26: dp2 r1.x, cb0[9].xyxx, r0.yzyy
    r1.x = (dot((source[9].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 27: dp2 r1.y, cb0[10].xyxx, r0.yzyy
    r1.y = (dot((source[10].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 28: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 29: mul r0.yz, r0.yyzy, cb0[17].xxyx
    r0.yz = ((r0.yyzy)*(source[17].xxyx)).yz;
    // 30: mad r1.x, cb0[12].w, cb0[16].w, r0.y
    r1.x = ((source[12].wwww)*(source[16].wwww)+(r0.yyyy)).x;
    // 31: mad r1.y, cb0[12].w, cb0[18].w, r0.z
    r1.y = ((source[12].wwww)*(source[18].wwww)+(r0.zzzz)).y;
    // 32: add r0.yz, r1.xxyx, cb0[19].xxyx
    r0.yz = ((r1.xxyx)+(source[19].xxyx)).yz;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s2, l(0.000000)
    r0.y = (SDNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 34: add r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)+(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 35: add r0.z, cb0[3].y, l(-1.000000)
    r0.z = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 36: add_sat r0.y, -r0.z, r0.y
    r0.y = (saturate((-(r0.zzzz))+(r0.yyyy))).y;
    // 37: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 38: mul r0.z, r0.z, cb0[19].w
    r0.z = ((r0.zzzz)*(source[19].wwww)).z;
    // 39: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 40: mul_sat r0.z, r0.z, cb0[19].z
    r0.z = (saturate((r0.zzzz)*(source[19].zzzz))).z;
    // 41: lt r1.x, r0.y, l(0.000001)
    r1.x = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 42: mul r0.y, r0.y, cb0[19].z
    r0.y = ((r0.yyyy)*(source[19].zzzz)).y;
    // 43: movc r0.z, r1.x, l(-0.000000), -r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.zzzz))).z;
    // 44: mov_sat r1.x, r0.y
    r1.x = (saturate(r0.yyyy)).x;
    // 45: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 46: mul r0.y, r0.y, cb0[20].y
    r0.y = ((r0.yyyy)*(source[20].yyyy)).y;
    // 47: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 48: add r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)+(r1.xxxx)).z;
    // 49: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 50: mad r0.xzw, r0.zzzz, cb0[11].xxyz, r0.xxxx
    r0.xzw = ((r0.zzzz)*(source[11].xxyz)+(r0.xxxx)).xzw;
    // 51: mad r0.xzw, r0.xxzw, cb0[1].xxyz, cb0[2].xxyz
    r0.xzw = ((r0.xxzw)*(source[1].xxyz)+(source[2].xxyz)).xzw;
    // 52: mad o0.xyz, r0.xzwx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xzwx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 53: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 54: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 55: mul r0.x, r0.x, v6.z
    r0.x = ((r0.xxxx)*(v6.zzzz)).x;
    // 56: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 57: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 58: mul r0.z, r0.z, cb0[20].x
    r0.z = ((r0.zzzz)*(source[20].xxxx)).z;
    // 59: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 60: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 61: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 62: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}

// fx_m_me_spritewave_01_8_ad: 778d9d5539a274478ce69ba8c43069bf; selected map f12736698d565bab7661db555834d6abb23f00bd216f9d5dc64e4e6d276e986e.
float4 SDNative332(SD_NATIVE_INPUT input)
{
    float4 source[24]; [unroll] for (uint i=0u; i<24u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[12u];
    source[3] = input.dynamicParameter;
    source[4] = SDNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = SDNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = SDNativeAppend(g_SDSourceMaterialParameters[4u].wwww,g_SDSourceMaterialParameters[5u].xxxx,1u);
    source[7] = SDNativeAppend(cos(((g_SDSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = SDNativeAppend(sin(((g_SDSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_SDSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = SDNativeAppend(cos((g_SDSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = SDNativeAppend(sin((g_SDSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11] = g_SDSourceMaterialParameters[10u];
    source[12].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[12].y = (cos(((g_SDSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[12].z = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[12].w = (g_SDSourceMaterialTime.xxxx).x;
    source[13].x = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[13].y = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[13].z = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[13].w = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[14].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_SDSourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_SDSourceMaterialParameters[8u].wwww).x;
    source[14].w = (g_SDSourceMaterialParameters[9u].yyyy).x;
    source[15].x = (g_SDSourceMaterialParameters[9u].zzzz).x;
    source[15].y = (g_SDSourceMaterialParameters[9u].xxxx).x;
    source[15].z = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[15].w = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[16].x = (g_SDSourceMaterialParameters[8u].zzzz).x;
    source[16].y = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[16].z = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[16].w = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[17].x = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[17].y = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[17].z = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[17].w = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[18].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[18].y = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[18].z = ((g_SDSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[18].w = (sin((g_SDSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[19].y = (cos((g_SDSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].z = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[19].w = (g_SDSourceMaterialParameters[7u].yyyy).x;
    source[20].x = (g_SDSourceMaterialParameters[7u].zzzz).x;
    source[20].y = (g_SDSourceMaterialParameters[7u].xxxx).x;
    source[20].z = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[20].w = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[21].x = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[21].y = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[21].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[21].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[22].x = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[22].y = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[22].z = (g_SDSourceMaterialParameters[7u].wwww).x;
    source[22].w = (g_SDSourceMaterialParameters[8u].xxxx).x;
    source[23].x = (g_SDSourceMaterialParameters[8u].yyyy).x;
    source[23].y = (g_SDSourceMaterialParameters[4u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mul r0.x, v4.x, cb0[19].w
    r0.x = ((v4.xxxx)*(source[19].wwww)).x;
    // 2: mad r0.x, cb0[12].w, cb0[19].z, r0.x
    r0.x = ((source[12].wwww)*(source[19].zzzz)+(r0.xxxx)).x;
    // 3: mul r0.zw, cb0[12].wwww, cb0[20].yyyw
    r0.zw = ((source[12].wwww)*(source[20].yyyw)).zw;
    // 4: mad r0.y, cb0[20].x, v4.y, r0.z
    r0.y = ((source[20].xxxx)*(v4.yyyy)+(r0.zzzz)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (SDNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 7: dp2 r1.x, cb0[9].xyxx, r0.yzyy
    r1.x = (dot((source[9].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 8: dp2 r1.y, cb0[10].xyxx, r0.yzyy
    r1.y = (dot((source[10].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 9: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 10: mad r1.xy, r0.xxxx, cb0[20].zzzz, r1.xyxx
    r1.xy = ((r0.xxxx)*(source[20].zzzz)+(r1.xyxx)).xy;
    // 11: mad r2.y, cb0[18].x, r1.y, r0.w
    r2.y = ((source[18].xxxx)*(r1.yyyy)+(r0.wwww)).y;
    // 12: mul r0.x, r1.x, cb0[17].w
    r0.x = ((r1.xxxx)*(source[17].wwww)).x;
    // 13: mad r2.x, cb0[12].w, cb0[17].z, r0.x
    r2.x = ((source[12].wwww)*(source[17].zzzz)+(r0.xxxx)).x;
    // 14: add r0.xw, r2.xxxy, cb0[21].xxxy
    r0.xw = ((r2.xxxy)+(source[21].xxxy)).xw;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xwxx, t3.xyzw, s3, l(0.000000)
    r0.x = (SDNativeSample3((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 16: add r0.x, r0.x, l(0.100000)
    r0.x = ((r0.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 17: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 18: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 19: mul r0.w, r0.w, cb0[21].z
    r0.w = ((r0.wwww)*(source[21].zzzz)).w;
    // 20: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 21: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 22: add r0.w, cb0[3].y, l(-1.000000)
    r0.w = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 23: add_sat r0.x, -r0.w, r0.x
    r0.x = (saturate((-(r0.wwww))+(r0.xxxx))).x;
    // 24: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 25: mul r0.w, r0.w, cb0[22].x
    r0.w = ((r0.wwww)*(source[22].xxxx)).w;
    // 26: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 27: mul_sat r0.w, r0.w, cb0[21].w
    r0.w = (saturate((r0.wwww)*(source[21].wwww))).w;
    // 28: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: mul r0.x, r0.x, cb0[21].w
    r0.x = ((r0.xxxx)*(source[21].wwww)).x;
    // 30: movc r0.w, r1.x, l(-0.000000), -r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.wwww))).w;
    // 31: mov_sat r1.x, r0.x
    r1.x = (saturate(r0.xxxx)).x;
    // 32: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 33: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: mul r1.xyz, r0.wwww, cb0[11].xyzx
    r1.xyz = ((r0.wwww)*(source[11].xyzx)).xyz;
    // 35: mul r0.w, cb0[12].w, cb0[14].z
    r0.w = ((source[12].wwww)*(source[14].zzzz)).w;
    // 36: mad r2.x, cb0[14].w, v4.x, r0.w
    r2.x = ((source[14].wwww)*(v4.xxxx)+(r0.wwww)).x;
    // 37: mul r0.w, v4.y, cb0[15].x
    r0.w = ((v4.yyyy)*(source[15].xxxx)).w;
    // 38: mad r2.y, cb0[12].w, cb0[15].y, r0.w
    r2.y = ((source[12].wwww)*(source[15].yyyy)+(r0.wwww)).y;
    // 39: mad r2.xy, cb0[3].wwww, cb0[15].zwzz, r2.xyxx
    r2.xy = ((source[3].wwww)*(source[15].zwzz)+(r2.xyxx)).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t0.yzwx, s1, l(0.000000)
    r0.w = (SDNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 41: add r1.w, cb0[3].z, cb0[16].x
    r1.w = ((source[3].zzzz)+(source[16].xxxx)).w;
    // 42: mad r2.xy, r0.wwww, r1.wwww, cb0[6].xyxx
    r2.xy = ((r0.wwww)*(r1.wwww)+(source[6].xyxx)).xy;
    // 43: dp2 r3.x, cb0[4].xyxx, r0.yzyy
    r3.x = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 44: dp2 r3.y, cb0[5].xyxx, r0.yzyy
    r3.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 45: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 46: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 47: mad r0.y, -r0.y, l(2.000000), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 48: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 49: mul r0.y, r0.y, cb0[22].z
    r0.y = ((r0.yyyy)*(source[22].zzzz)).y;
    // 50: max r0.y, r0.y, cb0[23].x
    r0.y = (max(r0.yyyy,source[23].xxxx)).y;
    // 51: min r0.y, r0.y, cb0[22].w
    r0.y = (min(r0.yyyy,source[22].wwww)).y;
    // 52: mad r0.zw, cb0[13].zzzw, cb0[3].xxxx, r3.xxxy
    r0.zw = ((source[13].zzzw)*(source[3].xxxx)+(r3.xxxy)).zw;
    // 53: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 54: mul r0.zw, r0.zzzw, cb0[13].xxxy
    r0.zw = ((r0.zzzw)*(source[13].xxxy)).zw;
    // 55: mad r3.x, cb0[12].w, cb0[12].z, r0.z
    r3.x = ((source[12].wwww)*(source[12].zzzz)+(r0.zzzz)).x;
    // 56: mad r3.y, cb0[12].w, cb0[14].y, r0.w
    r3.y = ((source[12].wwww)*(source[14].yyyy)+(r0.wwww)).y;
    // 57: add r0.zw, r2.xxxy, r3.xxxy
    r0.zw = ((r2.xxxy)+(r3.xxxy)).zw;
    // 58: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 59: dp2 r2.x, cb0[7].xyxx, r0.zwzz
    r2.x = (dot((source[7].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 60: dp2 r2.y, cb0[8].xyxx, r0.zwzz
    r2.y = (dot((source[8].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 61: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 62: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s0, l(-1.000000)
    r0.z = (SDNativeSample0((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).z;
    // 63: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 64: mul r0.w, r0.w, cb0[16].w
    r0.w = ((r0.wwww)*(source[16].wwww)).w;
    // 65: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 66: mul r0.w, r0.w, cb0[17].x
    r0.w = ((r0.wwww)*(source[17].xxxx)).w;
    // 67: lt r1.w, |r0.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 68: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 69: mad r0.w, r0.z, cb0[17].y, r0.w
    r0.w = ((r0.zzzz)*(source[17].yyyy)+(r0.wwww)).w;
    // 70: mad r1.xyz, r0.zzzz, r1.xyzx, r0.wwww
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r0.wwww)).xyz;
    // 71: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 72: mul r0.x, r0.x, cb0[23].y
    r0.x = ((r0.xxxx)*(source[23].yyyy)).x;
    // 73: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 74: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 75: mad r0.yzw, r1.xxyz, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r1.xxyz)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 76: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 77: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 78: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 79: mul r1.x, r1.x, v6.z
    r1.x = ((r1.xxxx)*(v6.zzzz)).x;
    // 80: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 81: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 82: mul r1.y, r1.y, cb0[22].y
    r1.y = ((r1.yyyy)*(source[22].yyyy)).y;
    // 83: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 84: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 85: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 86: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 87: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 88: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}


float4 SDNativeSample7(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 7u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 7u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture7.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture7.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture7.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture7.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearSampler, uv, lod);
}

// fx_t_pa_worldoffset_02_14_tr: 2cce8741b31d5049ab2b10e26293372d; selected map b62b4d84068ddcdad35cfacb576ce799a955f5824aac67e7740221f123d6be23.
float4 SDNative341(SD_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    // Original shader object binds CameraWorldPos at CB0[0].xyz and
    // WorldToLocal at CB0[1..3]. The VS camera-relative world plus camera
    // is precomposed as absolute centimetre world here; opacity is .w.
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[1]=g_SDSourceWorldToLocal[0];
    source[2]=g_SDSourceWorldToLocal[1];
    source[3]=g_SDSourceWorldToLocal[2];
    float4 output=0.f;
    source[4] = g_SDSourceMaterialParameters[8u];
    source[5] = SDNativeAppend(g_SDSourceMaterialParameters[5u].xxxx,g_SDSourceMaterialParameters[5u].yyyy,1u);
    source[6] = SDNativeAppend(cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = SDNativeAppend(sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = SDNativeAppend(g_SDSourceMaterialParameters[3u].zzzz,g_SDSourceMaterialParameters[3u].wwww,1u);
    source[9] = SDNativeAppend(g_SDSourceMaterialParameters[2u].xxxx,g_SDSourceMaterialParameters[2u].yyyy,1u);
    source[10].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[10].y = (cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].z = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[10].w = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[11].x = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[11].y = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[11].z = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[11].w = (g_SDSourceMaterialTime.xxxx).x;
    source[12].x = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_SDSourceMaterialParameters[7u].xxxx).x;
    source[12].z = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[12].w = (g_SDSourceMaterialParameters[7u].yyyy).x;
    source[13].x = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[13].y = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[13].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[14].x = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[14].y = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[14].z = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[15].x = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[15].y = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[15].z = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[15].w = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[16].x = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[16].y = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[16].z = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[16].w = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[17].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[17].y = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[17].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[17].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native camera-relative world + CameraWorldPos
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xyz, v7.xyzx, cb0[0].xyzx
    r0.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 2: mul r0.xyz, r0.xyzx, l(0.003906, 0.003906, 0.003906, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(asfloat(0x3b800000u),asfloat(0x3b800000u),asfloat(0x3b800000u),asfloat(0x00000000u)))).xyz;
    // 3: mul r0.yw, r0.yyyy, cb0[2].xxxy
    r0.yw = ((r0.yyyy)*(source[2].xxxy)).yw;
    // 4: mad r0.xy, cb0[1].xyxx, r0.xxxx, r0.ywyy
    r0.xy = ((source[1].xyxx)*(r0.xxxx)+(r0.ywyy)).xy;
    // 5: mad r0.xy, cb0[3].xyxx, r0.zzzz, r0.xyxx
    r0.xy = ((source[3].xyxx)*(r0.zzzz)+(r0.xyxx)).xy;
    // 6: mul r0.zw, r0.xxxy, cb0[14].zzzw
    r0.zw = ((r0.xxxy)*(source[14].zzzw)).zw;
    // 7: mad r1.x, cb0[11].w, cb0[14].y, r0.z
    r1.x = ((source[11].wwww)*(source[14].yyyy)+(r0.zzzz)).x;
    // 8: mad r1.y, cb0[11].w, cb0[15].x, r0.w
    r1.y = ((source[11].wwww)*(source[15].xxxx)+(r0.wwww)).y;
    // 9: add r0.zw, r1.xxxy, cb0[8].xxxy
    r0.zw = ((r1.xxxy)+(source[8].xxxy)).zw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s3, l(0.000000)
    r0.z = (SDNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 11: mad r1.xy, cb0[15].wwww, r0.zzzz, r0.xyxx
    r1.xy = ((source[15].wwww)*(r0.zzzz)+(r0.xyxx)).xy;
    // 12: mul r0.xy, r0.xyxx, cb0[12].xyxx
    r0.xy = ((r0.xyxx)*(source[12].xyxx)).xy;
    // 13: mul r0.w, r1.x, cb0[13].w
    r0.w = ((r1.xxxx)*(source[13].wwww)).w;
    // 14: mul r1.x, r1.y, cb0[14].x
    r1.x = ((r1.yyyy)*(source[14].xxxx)).x;
    // 15: mad r1.y, cb0[11].w, cb0[16].x, r1.x
    r1.y = ((source[11].wwww)*(source[16].xxxx)+(r1.xxxx)).y;
    // 16: mad r1.x, cb0[11].w, cb0[13].z, r0.w
    r1.x = ((source[11].wwww)*(source[13].zzzz)+(r0.wwww)).x;
    // 17: add r1.xy, r1.xyxx, cb0[9].xyxx
    r1.xy = ((r1.xyxx)+(source[9].xyxx)).xy;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t4.yzwx, s4, l(0.000000)
    r0.w = (SDNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 19: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 20: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 21: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 22: mul r0.w, r0.w, cb0[16].w
    r0.w = ((r0.wwww)*(source[16].wwww)).w;
    // 23: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 24: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 25: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 26: mul r0.w, r0.w, cb0[17].x
    r0.w = ((r0.wwww)*(source[17].xxxx)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: mul r0.w, r0.w, cb0[17].y
    r0.w = ((r0.wwww)*(source[17].yyyy)).w;
    // 29: lt r1.x, r0.z, l(0.000001)
    r1.x = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 30: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 31: mad r0.z, r0.z, cb0[17].z, r0.w
    r0.z = ((r0.zzzz)*(source[17].zzzz)+(r0.wwww)).z;
    // 32: mad r1.x, cb0[11].w, cb0[11].z, r0.x
    r1.x = ((source[11].wwww)*(source[11].zzzz)+(r0.xxxx)).x;
    // 33: mad r1.y, cb0[11].w, cb0[12].z, r0.y
    r1.y = ((source[11].wwww)*(source[12].zzzz)+(r0.yyyy)).y;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (SDNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 35: mad r1.xy, cb0[10].zwzz, v2.xyxx, cb0[5].xyxx
    r1.xy = ((source[10].zwzz)*(v2.xyxx)+(source[5].xyxx)).xy;
    // 36: mul r0.w, v4.x, cb0[12].w
    r0.w = ((v4.xxxx)*(source[12].wwww)).w;
    // 37: mad r1.xy, r0.wwww, r0.xyxx, r1.xyxx
    r1.xy = ((r0.wwww)*(r0.xyxx)+(r1.xyxx)).xy;
    // 38: mad r1.z, v4.y, l(0.500000), r1.y
    r1.z = ((v4.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r1.yyyy)).z;
    // 39: add r0.xy, r1.xzxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xzxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 40: dp2 r1.x, cb0[6].xyxx, r0.xyxx
    r1.x = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 41: dp2 r1.y, cb0[7].xyxx, r0.xyxx
    r1.y = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 42: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (SDNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (SDNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 45: add r0.y, r1.y, r1.x
    r0.y = ((r1.yyyy)+(r1.xxxx)).y;
    // 46: add r0.y, r1.z, r0.y
    r0.y = ((r1.zzzz)+(r0.yyyy)).y;
    // 47: mul r0.y, r0.y, l(0.333330)
    r0.y = ((r0.yyyy)*(float4(0.333330,0.333330,0.333330,0.333330))).y;
    // 48: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 49: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 50: mul r0.y, r0.y, cb0[13].x
    r0.y = ((r0.yyyy)*(source[13].xxxx)).y;
    // 51: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 52: mul r0.y, r0.y, cb0[13].y
    r0.y = ((r0.yyyy)*(source[13].yyyy)).y;
    // 53: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 54: movc r0.y, r0.w, l(0), r0.y
    r0.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 55: mad r0.y, r0.x, r0.z, r0.y
    r0.y = ((r0.xxxx)*(r0.zzzz)+(r0.yyyy)).y;
    // 56: mul r0.x, r0.x, cb0[17].w
    r0.x = ((r0.xxxx)*(source[17].wwww)).x;
    // 57: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 58: mad r0.yzw, r0.yyyy, v3.xxyz, cb0[4].xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)+(source[4].xxyz)).yzw;
    // 59: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 60: add r0.yz, -v2.xxyx, l(0.000000, 1.000000, 1.000000, 0.000000)
    r0.yz = ((-(v2.xxyx))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 61: mul r0.yz, r0.yyzy, v2.xxyx
    r0.yz = ((r0.yyzy)*(v2.xxyx)).yz;
    // 62: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 63: mul_sat r0.y, r0.y, l(50.000000)
    r0.y = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).y;
    // 64: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 65: mul o0.w, r0.x, cb0[0].w
    output.w = ((r0.xxxx)*(source[0].wwww)).w;
    return output;
}

// fx_s_pa_glasshole_01_01_tr: 7b93641a7efece49b83cf5555e5b9b63; selected map ff0c51bf206901318de55c8923ee9086f64e74cfbc9a1370469e9295616bee3a.
float4 SDNative342(SD_NATIVE_INPUT input)
{
    float4 source[21]; [unroll] for (uint i=0u; i<21u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[9u];
    source[2] = SDNativeAppend((g_SDSourceMaterialParameters[3u].zzzz*g_SDSourceMaterialTime.xxxx),(g_SDSourceMaterialParameters[3u].wwww*g_SDSourceMaterialTime.xxxx),1u);
    source[3] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.00999999978, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),1u);
    source[4] = SDNativeAppend(g_SDSourceMaterialParameters[0u].xxxx,g_SDSourceMaterialParameters[0u].yyyy,1u);
    source[5] = SDNativeAppend(g_SDSourceMaterialParameters[1u].yyyy,g_SDSourceMaterialParameters[1u].zzzz,1u);
    source[6] = SDNativeAppend(SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[5u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[5u].zzzz)*float4(-0.230000004, 0.0, 0.0, 0.0))),1u);
    source[7] = SDNativeAppend(SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[5u].zzzz)*float4(-0.5, 0.0, 0.0, 0.0))),SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.370000005, 0.0, 0.0, 0.0))),1u);
    source[8] = g_SDSourceMaterialParameters[8u];
    source[9] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))),1u);
    source[10] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0))),1u);
    source[11] = g_SDSourceMaterialParameters[7u];
    source[12] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[13].x = ((g_SDSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[13].y = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[13].z = ((g_SDSourceMaterialParameters[2u].yyyy*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[13].w = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[14].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[14].y = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[14].z = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[14].w = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[15].x = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[15].y = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[15].z = ((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[15].w = ((g_SDSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))).x;
    source[16].x = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[16].y = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[16].z = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[16].w = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[17].x = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[17].y = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[17].z = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[17].w = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[18].x = ((g_SDSourceMaterialParameters[6u].xxxx*g_SDSourceMaterialTime.xxxx)).x;
    source[18].y = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0)))).x;
    source[18].z = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[18].w = ((g_SDSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0))).x;
    source[19].x = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0)))).x;
    source[19].y = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[19].z = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[19].w = ((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[20].x = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[20].y = (g_SDSourceMaterialParameters[2u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v2.yxyy, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.yxyy)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 5: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 6: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 7: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).x;
    // 8: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 9: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).x;
    // 10: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 11: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 12: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).x;
    // 13: lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.y, r0.x
    r1.x = (max(r0.yyyy,r0.xxxx)).x;
    // 22: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 24: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 25: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 26: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 27: mul r0.y, r0.y, cb0[16].x
    r0.y = ((r0.yyyy)*(source[16].xxxx)).y;
    // 28: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: dp2 r0.z, r0.zzzz, cb0[16].yyyy
    r0.z = (dot((r0.zzzz).xy,(source[16].yyyy).xy).xxxx).z;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(asfloat(0x3ea2f983u),asfloat(0x3ea2f983u),asfloat(0x3ea2f983u),asfloat(0x3ea2f983u)))+(r0.zzzz)).y;
    // 31: add r0.y, r0.y, cb0[16].z
    r0.y = ((r0.yyyy)+(source[16].zzzz)).y;
    // 32: mul r1.x, r0.y, cb0[17].y
    r1.x = ((r0.yyyy)*(source[17].yyyy)).x;
    // 33: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 34: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 35: mul r0.y, r0.y, cb0[16].w
    r0.y = ((r0.yyyy)*(source[16].wwww)).y;
    // 36: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 37: mul r0.y, r0.y, v4.w
    r0.y = ((r0.yyyy)*(v4.wwww)).y;
    // 38: lt r0.z, r0.x, l(0.000000)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu)))) * 0xffffffffu)).z;
    // 39: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 40: add r0.z, v4.z, cb0[17].x
    r0.z = ((v4.zzzz)+(source[17].xxxx)).z;
    // 41: mad r0.y, r0.z, l(-0.400000), r0.y
    r0.y = ((r0.zzzz)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.yyyy)).y;
    // 42: mul r0.z, v4.w, cb0[18].x
    r0.z = ((v4.wwww)*(source[18].xxxx)).z;
    // 43: mad r1.y, r0.y, cb0[17].z, r0.z
    r1.y = ((r0.yyyy)*(source[17].zzzz)+(r0.zzzz)).y;
    // 44: add r0.yz, r1.xxyx, cb0[9].xxyx
    r0.yz = ((r1.xxyx)+(source[9].xxyx)).yz;
    // 45: add r1.xy, r1.xyxx, cb0[10].xyxx
    r1.xy = ((r1.xyxx)+(source[10].xyxx)).xy;
    // 46: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t6.xyzw, s5, l(-1.000000)
    r1.xyz = (SDNativeSample4((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t5.wxyz, s4, l(-1.000000)
    r0.yzw = (SDNativeSample3((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 48: mul r2.xyz, r1.xyzx, r0.yzwy
    r2.xyz = ((r1.xyzx)*(r0.yzwy)).xyz;
    // 49: add r0.yzw, r1.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)+(r0.yyzw)).yzw;
    // 50: max r1.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 51: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 52: mul r1.xyz, r1.xyzx, cb0[19].yyyy
    r1.xyz = ((r1.xyzx)*(source[19].yyyy)).xyz;
    // 53: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 54: add r2.xy, v2.xyxx, -cb0[4].xyxx
    r2.xy = ((v2.xyxx)+(-(source[4].xyxx))).xy;
    // 55: max r1.w, |r2.y|, |r2.x|
    r1.w = (max(abs(r2.yyyy),abs(r2.xxxx))).w;
    // 56: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 57: min r2.z, |r2.y|, |r2.x|
    r2.z = (min(abs(r2.yyyy),abs(r2.xxxx))).z;
    // 58: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 59: mul r2.z, r1.w, r1.w
    r2.z = ((r1.wwww)*(r1.wwww)).z;
    // 60: mad r2.w, r2.z, l(0.020835), l(-0.085133)
    r2.w = ((r2.zzzz)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).w;
    // 61: mad r2.w, r2.z, r2.w, l(0.180141)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(0.180141,0.180141,0.180141,0.180141))).w;
    // 62: mad r2.w, r2.z, r2.w, l(-0.330299)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).w;
    // 63: mad r2.z, r2.z, r2.w, l(0.999866)
    r2.z = ((r2.zzzz)*(r2.wwww)+(float4(0.999866,0.999866,0.999866,0.999866))).z;
    // 64: mul r2.w, r1.w, r2.z
    r2.w = ((r1.wwww)*(r2.zzzz)).w;
    // 65: mad r2.w, r2.w, l(-2.000000), l(1.570796)
    r2.w = ((r2.wwww)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).w;
    // 66: lt r3.x, |r2.y|, |r2.x|
    r3.x = (asfloat((uint4)((abs(r2.yyyy))<(abs(r2.xxxx))) * 0xffffffffu)).x;
    // 67: and r2.w, r2.w, r3.x
    r2.w = (asfloat(asuint(r2.wwww) & asuint(r3.xxxx))).w;
    // 68: mad r1.w, r1.w, r2.z, r2.w
    r1.w = ((r1.wwww)*(r2.zzzz)+(r2.wwww)).w;
    // 69: lt r2.z, r2.y, -r2.y
    r2.z = (asfloat((uint4)((r2.yyyy)<(-(r2.yyyy))) * 0xffffffffu)).z;
    // 70: and r2.z, r2.z, l(0xc0490fdb)
    r2.z = (asfloat(asuint(r2.zzzz) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).z;
    // 71: add r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)+(r2.zzzz)).w;
    // 72: min r2.z, r2.y, r2.x
    r2.z = (min(r2.yyyy,r2.xxxx)).z;
    // 73: max r2.x, r2.y, r2.x
    r2.x = (max(r2.yyyy,r2.xxxx)).x;
    // 74: ge r2.x, r2.x, -r2.x
    r2.x = (asfloat((uint4)((r2.xxxx)>=(-(r2.xxxx))) * 0xffffffffu)).x;
    // 75: lt r2.y, r2.z, -r2.z
    r2.y = (asfloat((uint4)((r2.zzzz)<(-(r2.zzzz))) * 0xffffffffu)).y;
    // 76: and r2.x, r2.x, r2.y
    r2.x = (asfloat(asuint(r2.xxxx) & asuint(r2.yyyy))).x;
    // 77: movc r1.w, r2.x, -r1.w, r1.w
    r1.w = ((asuint(r2.xxxx) != 0u) ? (-(r1.wwww)) : (r1.wwww)).w;
    // 78: mad r2.x, r1.w, l(0.159155), l(0.500000)
    r2.x = ((r1.wwww)*(float4(asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u)))+(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).x;
    // 79: mad r3.xy, v2.xyxx, cb0[5].xyxx, cb0[6].xyxx
    r3.xy = ((v2.xyxx)*(source[5].xyxx)+(source[6].xyxx)).xy;
    // 80: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r1.w = (SDNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 81: mad r1.w, r1.w, l(2.000000), l(-1.000000)
    r1.w = ((r1.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 82: mad r3.xy, v2.xyxx, cb0[5].xyxx, cb0[7].xyxx
    r3.xy = ((v2.xyxx)*(source[5].xyxx)+(source[7].xyxx)).xy;
    // 83: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r2.w = (SDNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 84: mad r2.w, r2.w, l(2.000000), l(-1.000000)
    r2.w = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 85: add r3.xy, v4.xyxx, l(-0.500000, -0.900000, 0.000000, 0.000000)
    r3.xy = ((v4.xyxx)+(float4(-0.500000,-0.900000,0.000000,0.000000))).xy;
    // 86: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 87: mad r1.w, r3.y, r1.w, r2.w
    r1.w = ((r3.yyyy)*(r1.wwww)+(r2.wwww)).w;
    // 88: mad r2.w, r0.x, l(3.000000), -r3.x
    r2.w = ((r0.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))+(-(r3.xxxx))).w;
    // 89: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 90: max r2.w, r2.w, l(-0.100000)
    r2.w = (max(r2.wwww,float4(-0.100000,-0.100000,-0.100000,-0.100000))).w;
    // 91: min r2.y, r2.w, l(10.000000)
    r2.y = (min(r2.wwww,float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 92: mul r1.w, r1.w, cb0[14].x
    r1.w = ((r1.wwww)*(source[14].xxxx)).w;
    // 93: mad r3.xy, r0.xxxx, r1.wwww, r2.xyxx
    r3.xy = ((r0.xxxx)*(r1.wwww)+(r2.xyxx)).xy;
    // 94: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 95: sample_l_indexable(texture2d)(float,float,float,float) r2.yz, r3.xyxx, t1.yzxw, s2, l(-1.000000)
    r2.yz = (SDNativeSample1((r3.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).yz;
    // 96: mul r3.xyz, r0.yzwy, r2.zzzz
    r3.xyz = ((r0.yzwy)*(r2.zzzz)).xyz;
    // 97: mul r3.xyz, r3.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 98: mad r1.xyz, cb0[19].zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((source[19].zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 99: mad r2.xw, r2.xxxz, l(7.000000, 0.000000, 0.000000, 1.000000), cb0[12].xxxy
    r2.xw = ((r2.xxxz)*(float4(7.000000,0.000000,0.000000,1.000000))+(source[12].xxxy)).xw;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xwxx, t7.xyzw, s6, l(0.000000)
    r3.xyz = (SDNativeSample5((r2.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 101: mul r3.xyz, r2.zzzz, r3.xyzx
    r3.xyz = ((r2.zzzz)*(r3.xyzx)).xyz;
    // 102: mul_sat r3.xyz, r3.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000)
    r3.xyz = (saturate((r3.xyzx)*(float4(4.000000,4.000000,4.000000,0.000000)))).xyz;
    // 103: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 104: mad r1.xyz, r1.xyzx, cb0[11].xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(source[11].xyzx)+(r3.xyzx)).xyz;
    // 105: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 106: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 107: mul r2.xw, r0.wwww, v6.xxxy
    r2.xw = ((r0.wwww)*(v6.xxxy)).xw;
    // 108: mad r0.yz, r0.yyzy, l(0.000000, 0.200000, 0.200000, 0.000000), r2.xxwx
    r0.yz = ((r0.yyzy)*(float4(0.000000,0.200000,0.200000,0.000000))+(r2.xxwx)).yz;
    // 109: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t8.wxyz, s7, l(0.000000)
    r0.yzw = (SDNativeSample6((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 110: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 111: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 112: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 113: mad r0.yzw, cb0[20].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[20].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 114: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 115: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 116: source device depth mapped to centimetre view depth; reconstruction at 118.
    r1.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 118-121: reconstructed view depth is supplied by the runtime adapter.
    r1.x = r1.x;
    // 122: add r1.x, r1.x, -v7.w
    r1.x = ((r1.xxxx)+(-(v7.wwww))).x;
    // 123: mul_sat r1.xy, r1.xxxx, l(0.034483, 0.066667, 0.000000, 0.000000)
    r1.xy = (saturate((r1.xxxx)*(float4(asfloat(0x3d0d3dcbu),asfloat(0x3d888889u),asfloat(0x00000000u),asfloat(0x00000000u))))).xy;
    // 124: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 125: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 126: lt r1.y, r1.y, l(0.000001)
    r1.y = (asfloat((uint4)((r1.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 127: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 128: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 129: mul r3.xyz, r1.zzzz, l(10.000000, 10.000000, 20.000000, 0.000000)
    r3.xyz = ((r1.zzzz)*(float4(10.000000,10.000000,20.000000,0.000000))).xyz;
    // 130: movc r1.yzw, r1.yyyy, l(0,0,0,0), r3.xxyz
    r1.yzw = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxyz)).yzw;
    // 131: mad r0.yzw, r2.yyyy, r0.yyzw, r1.yyzw
    r0.yzw = ((r2.yyyy)*(r0.yyzw)+(r1.yyzw)).yzw;
    // 132: mul r0.yzw, r0.yyzw, v3.xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)).yzw;
    // 133: log r1.y, r0.x
    r1.y = (log2(r0.xxxx)).y;
    // 134: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 135: mul r1.y, r1.y, cb0[13].x
    r1.y = ((r1.yyyy)*(source[13].xxxx)).y;
    // 136: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 137: mul r1.y, r1.y, cb0[13].z
    r1.y = ((r1.yyyy)*(source[13].zzzz)).y;
    // 138: add r1.zw, -v2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((-(v2.xxxy))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 139: mul r1.yz, r1.zzwz, r1.yyyy
    r1.yz = ((r1.zzwz)*(r1.yyyy)).yz;
    // 140: movc r1.yz, r0.xxxx, l(0,0,0,0), r1.yyzy
    r1.yz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyzy)).yz;
    // 141: add r1.yz, r1.yyzy, v2.xxyx
    r1.yz = ((r1.yyzy)+(v2.xxyx)).yz;
    // 142: mad r1.yz, cb0[13].wwww, r1.yyzy, cb0[2].xxyx
    r1.yz = ((source[13].wwww)*(r1.yyzy)+(source[2].xxyx)).yz;
    // 143: add r1.yz, r1.yyzy, cb0[3].xxyx
    r1.yz = ((r1.yyzy)+(source[3].xxyx)).yz;
    // 144: add r0.x, -r2.y, l(1.000000)
    r0.x = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 145: add_sat r1.w, r0.x, r2.z
    r1.w = (saturate((r0.xxxx)+(r2.zzzz))).w;
    // 146: mul r1.x, r1.x, r1.w
    r1.x = ((r1.xxxx)*(r1.wwww)).x;
    // 147: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 148: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v2.xyxx, t0.zxyw, s1, l(0.000000)
    r2.yz = (SDNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 149: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 150: mul r2.yz, r0.xxxx, r2.yyzy
    r2.yz = ((r0.xxxx)*(r2.yyzy)).yz;
    // 151: mad r1.yz, cb0[14].yyyy, r2.yyzy, r1.yyzy
    r1.yz = ((source[14].yyyy)*(r2.yyzy)+(r1.yyzy)).yz;
    // 152: mad r1.w, cb0[14].z, l(0.500000), l(-0.250000)
    r1.w = ((source[14].zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(-0.250000,-0.250000,-0.250000,-0.250000))).w;
    // 153: mad r1.yz, r1.wwww, r2.xxwx, r1.yyzy
    r1.yz = ((r1.wwww)*(r2.xxwx)+(r1.yyzy)).yz;
    // 154: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t4.wxyz, s3, l(0.000000)
    r1.yzw = (SDNativeSample2((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 155: max r1.yzw, |r1.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r1.yzw = (max(abs(r1.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 156: log r1.yzw, r1.yyzw
    r1.yzw = (log2(r1.yyzw)).yzw;
    // 157: mul r1.yzw, r1.yyzw, cb0[14].wwww
    r1.yzw = ((r1.yyzw)*(source[14].wwww)).yzw;
    // 158: exp r1.yzw, r1.yyzw
    r1.yzw = (exp2(r1.yyzw)).yzw;
    // 159: mul r2.xyz, r1.yzwy, cb0[15].xxxx
    r2.xyz = ((r1.yzwy)*(source[15].xxxx)).xyz;
    // 160: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 161: mad r1.yzw, -cb0[15].xxxx, r1.yyzw, r2.wwww
    r1.yzw = ((-(source[15].xxxx))*(r1.yyzw)+(r2.wwww)).yzw;
    // 162: mad r1.yzw, cb0[15].yyyy, r1.yyzw, r2.xxyz
    r1.yzw = ((source[15].yyyy)*(r1.yyzw)+(r2.xxyz)).yzw;
    // 163: mul r1.yzw, r1.yyzw, cb0[8].xxyz
    r1.yzw = ((r1.yyzw)*(source[8].xxyz)).yzw;
    // 164: mad r0.xyz, r0.xxxx, r1.yzwy, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.yzwy)+(r0.yzwy)).xyz;
    // 165: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 166: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 167: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t3.xyzw, s8, l(0.000000)
    r0.x = (SDNativeSample7((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 168: mul_sat r0.x, r1.x, r0.x
    r0.x = (saturate((r1.xxxx)*(r0.xxxx))).x;
    // 169: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

// fx_s_pa_glasshole_01_02_tr: 7b93641a7efece49b83cf5555e5b9b63; selected map ff0c51bf206901318de55c8923ee9086f64e74cfbc9a1370469e9295616bee3a.
float4 SDNative343(SD_NATIVE_INPUT input)
{
    float4 source[21]; [unroll] for (uint i=0u; i<21u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[9u];
    source[2] = SDNativeAppend((g_SDSourceMaterialParameters[3u].zzzz*g_SDSourceMaterialTime.xxxx),(g_SDSourceMaterialParameters[3u].wwww*g_SDSourceMaterialTime.xxxx),1u);
    source[3] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.00999999978, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),1u);
    source[4] = SDNativeAppend(g_SDSourceMaterialParameters[0u].xxxx,g_SDSourceMaterialParameters[0u].yyyy,1u);
    source[5] = SDNativeAppend(g_SDSourceMaterialParameters[1u].yyyy,g_SDSourceMaterialParameters[1u].zzzz,1u);
    source[6] = SDNativeAppend(SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[5u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[5u].zzzz)*float4(-0.230000004, 0.0, 0.0, 0.0))),1u);
    source[7] = SDNativeAppend(SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[5u].zzzz)*float4(-0.5, 0.0, 0.0, 0.0))),SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.370000005, 0.0, 0.0, 0.0))),1u);
    source[8] = g_SDSourceMaterialParameters[8u];
    source[9] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))),1u);
    source[10] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0))),1u);
    source[11] = g_SDSourceMaterialParameters[7u];
    source[12] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[13].x = ((g_SDSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[13].y = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[13].z = ((g_SDSourceMaterialParameters[2u].yyyy*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[13].w = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[14].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[14].y = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[14].z = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[14].w = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[15].x = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[15].y = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[15].z = ((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[15].w = ((g_SDSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))).x;
    source[16].x = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[16].y = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[16].z = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[16].w = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[17].x = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[17].y = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[17].z = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[17].w = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[18].x = ((g_SDSourceMaterialParameters[6u].xxxx*g_SDSourceMaterialTime.xxxx)).x;
    source[18].y = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0)))).x;
    source[18].z = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[18].w = ((g_SDSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0))).x;
    source[19].x = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0)))).x;
    source[19].y = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[19].z = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[19].w = ((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[20].x = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[20].y = (g_SDSourceMaterialParameters[2u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v2.yxyy, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.yxyy)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 5: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 6: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 7: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).x;
    // 8: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 9: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).x;
    // 10: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 11: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 12: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).x;
    // 13: lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.y, r0.x
    r1.x = (max(r0.yyyy,r0.xxxx)).x;
    // 22: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 24: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 25: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 26: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 27: mul r0.y, r0.y, cb0[16].x
    r0.y = ((r0.yyyy)*(source[16].xxxx)).y;
    // 28: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: dp2 r0.z, r0.zzzz, cb0[16].yyyy
    r0.z = (dot((r0.zzzz).xy,(source[16].yyyy).xy).xxxx).z;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(asfloat(0x3ea2f983u),asfloat(0x3ea2f983u),asfloat(0x3ea2f983u),asfloat(0x3ea2f983u)))+(r0.zzzz)).y;
    // 31: add r0.y, r0.y, cb0[16].z
    r0.y = ((r0.yyyy)+(source[16].zzzz)).y;
    // 32: mul r1.x, r0.y, cb0[17].y
    r1.x = ((r0.yyyy)*(source[17].yyyy)).x;
    // 33: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 34: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 35: mul r0.y, r0.y, cb0[16].w
    r0.y = ((r0.yyyy)*(source[16].wwww)).y;
    // 36: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 37: mul r0.y, r0.y, v4.w
    r0.y = ((r0.yyyy)*(v4.wwww)).y;
    // 38: lt r0.z, r0.x, l(0.000000)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu)))) * 0xffffffffu)).z;
    // 39: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 40: add r0.z, v4.z, cb0[17].x
    r0.z = ((v4.zzzz)+(source[17].xxxx)).z;
    // 41: mad r0.y, r0.z, l(-0.400000), r0.y
    r0.y = ((r0.zzzz)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.yyyy)).y;
    // 42: mul r0.z, v4.w, cb0[18].x
    r0.z = ((v4.wwww)*(source[18].xxxx)).z;
    // 43: mad r1.y, r0.y, cb0[17].z, r0.z
    r1.y = ((r0.yyyy)*(source[17].zzzz)+(r0.zzzz)).y;
    // 44: add r0.yz, r1.xxyx, cb0[9].xxyx
    r0.yz = ((r1.xxyx)+(source[9].xxyx)).yz;
    // 45: add r1.xy, r1.xyxx, cb0[10].xyxx
    r1.xy = ((r1.xyxx)+(source[10].xyxx)).xy;
    // 46: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t6.xyzw, s5, l(-1.000000)
    r1.xyz = (SDNativeSample4((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t5.wxyz, s4, l(-1.000000)
    r0.yzw = (SDNativeSample3((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 48: mul r2.xyz, r1.xyzx, r0.yzwy
    r2.xyz = ((r1.xyzx)*(r0.yzwy)).xyz;
    // 49: add r0.yzw, r1.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)+(r0.yyzw)).yzw;
    // 50: max r1.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 51: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 52: mul r1.xyz, r1.xyzx, cb0[19].yyyy
    r1.xyz = ((r1.xyzx)*(source[19].yyyy)).xyz;
    // 53: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 54: add r2.xy, v2.xyxx, -cb0[4].xyxx
    r2.xy = ((v2.xyxx)+(-(source[4].xyxx))).xy;
    // 55: max r1.w, |r2.y|, |r2.x|
    r1.w = (max(abs(r2.yyyy),abs(r2.xxxx))).w;
    // 56: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 57: min r2.z, |r2.y|, |r2.x|
    r2.z = (min(abs(r2.yyyy),abs(r2.xxxx))).z;
    // 58: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 59: mul r2.z, r1.w, r1.w
    r2.z = ((r1.wwww)*(r1.wwww)).z;
    // 60: mad r2.w, r2.z, l(0.020835), l(-0.085133)
    r2.w = ((r2.zzzz)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).w;
    // 61: mad r2.w, r2.z, r2.w, l(0.180141)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(0.180141,0.180141,0.180141,0.180141))).w;
    // 62: mad r2.w, r2.z, r2.w, l(-0.330299)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).w;
    // 63: mad r2.z, r2.z, r2.w, l(0.999866)
    r2.z = ((r2.zzzz)*(r2.wwww)+(float4(0.999866,0.999866,0.999866,0.999866))).z;
    // 64: mul r2.w, r1.w, r2.z
    r2.w = ((r1.wwww)*(r2.zzzz)).w;
    // 65: mad r2.w, r2.w, l(-2.000000), l(1.570796)
    r2.w = ((r2.wwww)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).w;
    // 66: lt r3.x, |r2.y|, |r2.x|
    r3.x = (asfloat((uint4)((abs(r2.yyyy))<(abs(r2.xxxx))) * 0xffffffffu)).x;
    // 67: and r2.w, r2.w, r3.x
    r2.w = (asfloat(asuint(r2.wwww) & asuint(r3.xxxx))).w;
    // 68: mad r1.w, r1.w, r2.z, r2.w
    r1.w = ((r1.wwww)*(r2.zzzz)+(r2.wwww)).w;
    // 69: lt r2.z, r2.y, -r2.y
    r2.z = (asfloat((uint4)((r2.yyyy)<(-(r2.yyyy))) * 0xffffffffu)).z;
    // 70: and r2.z, r2.z, l(0xc0490fdb)
    r2.z = (asfloat(asuint(r2.zzzz) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).z;
    // 71: add r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)+(r2.zzzz)).w;
    // 72: min r2.z, r2.y, r2.x
    r2.z = (min(r2.yyyy,r2.xxxx)).z;
    // 73: max r2.x, r2.y, r2.x
    r2.x = (max(r2.yyyy,r2.xxxx)).x;
    // 74: ge r2.x, r2.x, -r2.x
    r2.x = (asfloat((uint4)((r2.xxxx)>=(-(r2.xxxx))) * 0xffffffffu)).x;
    // 75: lt r2.y, r2.z, -r2.z
    r2.y = (asfloat((uint4)((r2.zzzz)<(-(r2.zzzz))) * 0xffffffffu)).y;
    // 76: and r2.x, r2.x, r2.y
    r2.x = (asfloat(asuint(r2.xxxx) & asuint(r2.yyyy))).x;
    // 77: movc r1.w, r2.x, -r1.w, r1.w
    r1.w = ((asuint(r2.xxxx) != 0u) ? (-(r1.wwww)) : (r1.wwww)).w;
    // 78: mad r2.x, r1.w, l(0.159155), l(0.500000)
    r2.x = ((r1.wwww)*(float4(asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u)))+(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).x;
    // 79: mad r3.xy, v2.xyxx, cb0[5].xyxx, cb0[6].xyxx
    r3.xy = ((v2.xyxx)*(source[5].xyxx)+(source[6].xyxx)).xy;
    // 80: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r1.w = (SDNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 81: mad r1.w, r1.w, l(2.000000), l(-1.000000)
    r1.w = ((r1.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 82: mad r3.xy, v2.xyxx, cb0[5].xyxx, cb0[7].xyxx
    r3.xy = ((v2.xyxx)*(source[5].xyxx)+(source[7].xyxx)).xy;
    // 83: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r2.w = (SDNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 84: mad r2.w, r2.w, l(2.000000), l(-1.000000)
    r2.w = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 85: add r3.xy, v4.xyxx, l(-0.500000, -0.900000, 0.000000, 0.000000)
    r3.xy = ((v4.xyxx)+(float4(-0.500000,-0.900000,0.000000,0.000000))).xy;
    // 86: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 87: mad r1.w, r3.y, r1.w, r2.w
    r1.w = ((r3.yyyy)*(r1.wwww)+(r2.wwww)).w;
    // 88: mad r2.w, r0.x, l(3.000000), -r3.x
    r2.w = ((r0.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))+(-(r3.xxxx))).w;
    // 89: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 90: max r2.w, r2.w, l(-0.100000)
    r2.w = (max(r2.wwww,float4(-0.100000,-0.100000,-0.100000,-0.100000))).w;
    // 91: min r2.y, r2.w, l(10.000000)
    r2.y = (min(r2.wwww,float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 92: mul r1.w, r1.w, cb0[14].x
    r1.w = ((r1.wwww)*(source[14].xxxx)).w;
    // 93: mad r3.xy, r0.xxxx, r1.wwww, r2.xyxx
    r3.xy = ((r0.xxxx)*(r1.wwww)+(r2.xyxx)).xy;
    // 94: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 95: sample_l_indexable(texture2d)(float,float,float,float) r2.yz, r3.xyxx, t1.yzxw, s2, l(-1.000000)
    r2.yz = (SDNativeSample1((r3.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).yz;
    // 96: mul r3.xyz, r0.yzwy, r2.zzzz
    r3.xyz = ((r0.yzwy)*(r2.zzzz)).xyz;
    // 97: mul r3.xyz, r3.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 98: mad r1.xyz, cb0[19].zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((source[19].zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 99: mad r2.xw, r2.xxxz, l(7.000000, 0.000000, 0.000000, 1.000000), cb0[12].xxxy
    r2.xw = ((r2.xxxz)*(float4(7.000000,0.000000,0.000000,1.000000))+(source[12].xxxy)).xw;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xwxx, t7.xyzw, s6, l(0.000000)
    r3.xyz = (SDNativeSample5((r2.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 101: mul r3.xyz, r2.zzzz, r3.xyzx
    r3.xyz = ((r2.zzzz)*(r3.xyzx)).xyz;
    // 102: mul_sat r3.xyz, r3.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000)
    r3.xyz = (saturate((r3.xyzx)*(float4(4.000000,4.000000,4.000000,0.000000)))).xyz;
    // 103: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 104: mad r1.xyz, r1.xyzx, cb0[11].xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(source[11].xyzx)+(r3.xyzx)).xyz;
    // 105: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 106: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 107: mul r2.xw, r0.wwww, v6.xxxy
    r2.xw = ((r0.wwww)*(v6.xxxy)).xw;
    // 108: mad r0.yz, r0.yyzy, l(0.000000, 0.200000, 0.200000, 0.000000), r2.xxwx
    r0.yz = ((r0.yyzy)*(float4(0.000000,0.200000,0.200000,0.000000))+(r2.xxwx)).yz;
    // 109: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t8.wxyz, s7, l(0.000000)
    r0.yzw = (SDNativeSample6((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 110: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 111: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 112: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 113: mad r0.yzw, cb0[20].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[20].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 114: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 115: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 116: source device depth mapped to centimetre view depth; reconstruction at 118.
    r1.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 118-121: reconstructed view depth is supplied by the runtime adapter.
    r1.x = r1.x;
    // 122: add r1.x, r1.x, -v7.w
    r1.x = ((r1.xxxx)+(-(v7.wwww))).x;
    // 123: mul_sat r1.xy, r1.xxxx, l(0.034483, 0.066667, 0.000000, 0.000000)
    r1.xy = (saturate((r1.xxxx)*(float4(asfloat(0x3d0d3dcbu),asfloat(0x3d888889u),asfloat(0x00000000u),asfloat(0x00000000u))))).xy;
    // 124: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 125: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 126: lt r1.y, r1.y, l(0.000001)
    r1.y = (asfloat((uint4)((r1.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 127: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 128: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 129: mul r3.xyz, r1.zzzz, l(10.000000, 10.000000, 20.000000, 0.000000)
    r3.xyz = ((r1.zzzz)*(float4(10.000000,10.000000,20.000000,0.000000))).xyz;
    // 130: movc r1.yzw, r1.yyyy, l(0,0,0,0), r3.xxyz
    r1.yzw = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxyz)).yzw;
    // 131: mad r0.yzw, r2.yyyy, r0.yyzw, r1.yyzw
    r0.yzw = ((r2.yyyy)*(r0.yyzw)+(r1.yyzw)).yzw;
    // 132: mul r0.yzw, r0.yyzw, v3.xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)).yzw;
    // 133: log r1.y, r0.x
    r1.y = (log2(r0.xxxx)).y;
    // 134: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 135: mul r1.y, r1.y, cb0[13].x
    r1.y = ((r1.yyyy)*(source[13].xxxx)).y;
    // 136: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 137: mul r1.y, r1.y, cb0[13].z
    r1.y = ((r1.yyyy)*(source[13].zzzz)).y;
    // 138: add r1.zw, -v2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((-(v2.xxxy))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 139: mul r1.yz, r1.zzwz, r1.yyyy
    r1.yz = ((r1.zzwz)*(r1.yyyy)).yz;
    // 140: movc r1.yz, r0.xxxx, l(0,0,0,0), r1.yyzy
    r1.yz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyzy)).yz;
    // 141: add r1.yz, r1.yyzy, v2.xxyx
    r1.yz = ((r1.yyzy)+(v2.xxyx)).yz;
    // 142: mad r1.yz, cb0[13].wwww, r1.yyzy, cb0[2].xxyx
    r1.yz = ((source[13].wwww)*(r1.yyzy)+(source[2].xxyx)).yz;
    // 143: add r1.yz, r1.yyzy, cb0[3].xxyx
    r1.yz = ((r1.yyzy)+(source[3].xxyx)).yz;
    // 144: add r0.x, -r2.y, l(1.000000)
    r0.x = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 145: add_sat r1.w, r0.x, r2.z
    r1.w = (saturate((r0.xxxx)+(r2.zzzz))).w;
    // 146: mul r1.x, r1.x, r1.w
    r1.x = ((r1.xxxx)*(r1.wwww)).x;
    // 147: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 148: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v2.xyxx, t0.zxyw, s1, l(0.000000)
    r2.yz = (SDNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 149: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 150: mul r2.yz, r0.xxxx, r2.yyzy
    r2.yz = ((r0.xxxx)*(r2.yyzy)).yz;
    // 151: mad r1.yz, cb0[14].yyyy, r2.yyzy, r1.yyzy
    r1.yz = ((source[14].yyyy)*(r2.yyzy)+(r1.yyzy)).yz;
    // 152: mad r1.w, cb0[14].z, l(0.500000), l(-0.250000)
    r1.w = ((source[14].zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(-0.250000,-0.250000,-0.250000,-0.250000))).w;
    // 153: mad r1.yz, r1.wwww, r2.xxwx, r1.yyzy
    r1.yz = ((r1.wwww)*(r2.xxwx)+(r1.yyzy)).yz;
    // 154: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t4.wxyz, s3, l(0.000000)
    r1.yzw = (SDNativeSample2((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 155: max r1.yzw, |r1.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r1.yzw = (max(abs(r1.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 156: log r1.yzw, r1.yyzw
    r1.yzw = (log2(r1.yyzw)).yzw;
    // 157: mul r1.yzw, r1.yyzw, cb0[14].wwww
    r1.yzw = ((r1.yyzw)*(source[14].wwww)).yzw;
    // 158: exp r1.yzw, r1.yyzw
    r1.yzw = (exp2(r1.yyzw)).yzw;
    // 159: mul r2.xyz, r1.yzwy, cb0[15].xxxx
    r2.xyz = ((r1.yzwy)*(source[15].xxxx)).xyz;
    // 160: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 161: mad r1.yzw, -cb0[15].xxxx, r1.yyzw, r2.wwww
    r1.yzw = ((-(source[15].xxxx))*(r1.yyzw)+(r2.wwww)).yzw;
    // 162: mad r1.yzw, cb0[15].yyyy, r1.yyzw, r2.xxyz
    r1.yzw = ((source[15].yyyy)*(r1.yyzw)+(r2.xxyz)).yzw;
    // 163: mul r1.yzw, r1.yyzw, cb0[8].xxyz
    r1.yzw = ((r1.yyzw)*(source[8].xxyz)).yzw;
    // 164: mad r0.xyz, r0.xxxx, r1.yzwy, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.yzwy)+(r0.yzwy)).xyz;
    // 165: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 166: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 167: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t3.xyzw, s8, l(0.000000)
    r0.x = (SDNativeSample7((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 168: mul_sat r0.x, r1.x, r0.x
    r0.x = (saturate((r1.xxxx)*(r0.xxxx))).x;
    // 169: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

// fx_m_phantomblade_finish_01_tr_inst: 2cf0d3d4d4b14140a4111a6eed929f99; selected map 1477056689880f3fb79170ff468b9bfb4d0367f8c84ae62f4cedbe1371158f80.
float4 SDNative344(SD_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[17u];
    source[2].x = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[2].y = (g_SDSourceMaterialParameters[13u].wwww).x;
    source[2].z = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[2].w = (g_SDSourceMaterialParameters[8u].yyyy).x;
    source[3].x = (g_SDSourceMaterialTime.xxxx).x;
    source[3].y = ((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[8u].yyyy)).x;
    source[3].z = (g_SDSourceMaterialParameters[9u].yyyy).x;
    source[3].w = (g_SDSourceMaterialParameters[7u].xxxx).x;
    source[4].x = (g_SDSourceMaterialParameters[12u].yyyy).x;
    source[4].y = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[4].z = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[4].w = ((g_SDSourceMaterialParameters[7u].xxxx*g_SDSourceMaterialTime.xxxx)).x;
    source[5].x = (g_SDSourceMaterialParameters[10u].wwww).x;
    source[5].y = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[5].z = (g_SDSourceMaterialParameters[14u].xxxx).x;
    source[5].w = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[6].x = (g_SDSourceMaterialParameters[9u].zzzz).x;
    source[6].y = (g_SDSourceMaterialParameters[12u].zzzz).x;
    source[6].z = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[6].w = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[7].x = (g_SDSourceMaterialParameters[11u].xxxx).x;
    source[7].y = (g_SDSourceMaterialParameters[15u].wwww).x;
    source[7].z = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[7].w = (g_SDSourceMaterialParameters[14u].yyyy).x;
    source[8].x = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[8].y = (g_SDSourceMaterialParameters[9u].wwww).x;
    source[8].z = (g_SDSourceMaterialParameters[12u].wwww).x;
    source[8].w = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[9].x = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[9].y = (g_SDSourceMaterialParameters[11u].yyyy).x;
    source[9].z = (g_SDSourceMaterialParameters[16u].xxxx).x;
    source[9].w = (g_SDSourceMaterialParameters[15u].yyyy).x;
    source[10].x = (g_SDSourceMaterialParameters[15u].zzzz).x;
    source[10].y = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[10].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[10].w = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[11].x = (g_SDSourceMaterialParameters[13u].xxxx).x;
    source[11].y = (g_SDSourceMaterialParameters[5u].zzzz).x;
    source[11].z = (g_SDSourceMaterialParameters[7u].zzzz).x;
    source[11].w = ((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[7u].zzzz)).x;
    source[12].x = (g_SDSourceMaterialParameters[8u].zzzz).x;
    source[12].y = (g_SDSourceMaterialParameters[7u].yyyy).x;
    source[12].z = (g_SDSourceMaterialParameters[11u].zzzz).x;
    source[12].w = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[13].x = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[13].y = ((g_SDSourceMaterialParameters[7u].yyyy*g_SDSourceMaterialTime.xxxx)).x;
    source[13].z = (g_SDSourceMaterialParameters[10u].xxxx).x;
    source[13].w = (g_SDSourceMaterialParameters[14u].zzzz).x;
    source[14].x = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[14].y = (g_SDSourceMaterialParameters[13u].yyyy).x;
    source[14].z = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[14].w = (g_SDSourceMaterialParameters[8u].xxxx).x;
    source[15].x = ((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[8u].xxxx)).x;
    source[15].y = (g_SDSourceMaterialParameters[8u].wwww).x;
    source[15].z = (g_SDSourceMaterialParameters[7u].wwww).x;
    source[15].w = (g_SDSourceMaterialParameters[11u].wwww).x;
    source[16].x = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[16].y = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[16].z = ((g_SDSourceMaterialParameters[7u].wwww*g_SDSourceMaterialTime.xxxx)).x;
    source[16].w = (g_SDSourceMaterialParameters[10u].yyyy).x;
    source[17].x = (g_SDSourceMaterialParameters[14u].wwww).x;
    source[17].y = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[17].z = (g_SDSourceMaterialParameters[13u].zzzz).x;
    source[17].w = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[18].x = (g_SDSourceMaterialParameters[9u].xxxx).x;
    source[18].y = (g_SDSourceMaterialParameters[12u].xxxx).x;
    source[18].z = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[18].w = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[19].x = (g_SDSourceMaterialParameters[10u].zzzz).x;
    source[19].y = (g_SDSourceMaterialParameters[15u].xxxx).x;
    source[19].z = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[19].w = (g_SDSourceMaterialParameters[5u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 5: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 6: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 7: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).x;
    // 8: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 9: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).x;
    // 10: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 11: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 12: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).x;
    // 13: lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.y, r0.x
    r1.x = (max(r0.yyyy,r0.xxxx)).x;
    // 22: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 24: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 25: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 26: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 27: mad r0.y, r0.y, l(0.159155), l(0.500000)
    r0.y = ((r0.yyyy)*(float4(asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u)))+(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).y;
    // 28: mad r0.z, v4.y, cb0[8].z, cb0[8].w
    r0.z = ((v4.yyyy)*(source[8].zzzz)+(source[8].wwww)).z;
    // 29: mad r0.z, cb0[9].x, r0.y, r0.z
    r0.z = ((source[9].xxxx)*(r0.yyyy)+(r0.zzzz)).z;
    // 30: mad r0.w, v4.y, cb0[7].w, cb0[8].x
    r0.w = ((v4.yyyy)*(source[7].wwww)+(source[8].xxxx)).w;
    // 31: mad r0.w, r0.x, cb0[7].z, r0.w
    r0.w = ((r0.xxxx)*(source[7].zzzz)+(r0.wwww)).w;
    // 32: mad r1.x, cb0[8].y, r0.w, r0.z
    r1.x = ((source[8].yyyy)*(r0.wwww)+(r0.zzzz)).x;
    // 33: mad r1.y, r0.z, cb0[9].y, r0.w
    r1.y = ((r0.zzzz)*(source[9].yyyy)+(r0.wwww)).y;
    // 34: sample_l_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t1.zwxy, s2, l(-1.000000)
    r0.zw = (SDNativeSample2((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).zwxy).zw;
    // 35: mul r0.zw, r0.zzzw, cb0[9].zzzz
    r0.zw = ((r0.zzzw)*(source[9].zzzz)).zw;
    // 36: mad r1.x, v4.y, cb0[6].y, cb0[6].z
    r1.x = ((v4.yyyy)*(source[6].yyyy)+(source[6].zzzz)).x;
    // 37: mad r1.x, cb0[6].w, r0.y, r1.x
    r1.x = ((source[6].wwww)*(r0.yyyy)+(r1.xxxx)).x;
    // 38: mad r1.y, v4.y, cb0[5].z, cb0[5].w
    r1.y = ((v4.yyyy)*(source[5].zzzz)+(source[5].wwww)).y;
    // 39: mad r1.y, r0.x, cb0[5].y, r1.y
    r1.y = ((r0.xxxx)*(source[5].yyyy)+(r1.yyyy)).y;
    // 40: mad r2.x, cb0[6].x, r1.y, r1.x
    r2.x = ((source[6].xxxx)*(r1.yyyy)+(r1.xxxx)).x;
    // 41: mad r2.y, r1.x, cb0[7].x, r1.y
    r2.y = ((r1.xxxx)*(source[7].xxxx)+(r1.yyyy)).y;
    // 42: sample_l_indexable(texture2d)(float,float,float,float) r1.xy, r2.xyxx, t0.xyzw, s1, l(-1.000000)
    r1.xy = (SDNativeSample1((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
    // 43: mad r0.zw, cb0[7].yyyy, r1.xxxy, r0.zzzw
    r0.zw = ((source[7].yyyy)*(r1.xxxy)+(r0.zzzw)).zw;
    // 44: mul r0.zw, r0.zzzw, v4.zzzz
    r0.zw = ((r0.zzzw)*(v4.zzzz)).zw;
    // 45: mad r1.x, v4.x, cb0[12].z, cb0[12].w
    r1.x = ((v4.xxxx)*(source[12].zzzz)+(source[12].wwww)).x;
    // 46: mad r1.x, cb0[13].x, r0.y, r1.x
    r1.x = ((source[13].xxxx)*(r0.yyyy)+(r1.xxxx)).x;
    // 47: add r1.x, r1.x, cb0[13].y
    r1.x = ((r1.xxxx)+(source[13].yyyy)).x;
    // 48: mad r1.y, v4.x, cb0[11].x, cb0[11].y
    r1.y = ((v4.xxxx)*(source[11].xxxx)+(source[11].yyyy)).y;
    // 49: mad r1.y, r0.x, cb0[10].w, r1.y
    r1.y = ((r0.xxxx)*(source[10].wwww)+(r1.yyyy)).y;
    // 50: add r1.y, r1.y, cb0[11].w
    r1.y = ((r1.yyyy)+(source[11].wwww)).y;
    // 51: mad r2.x, cb0[12].x, r1.y, r1.x
    r2.x = ((source[12].xxxx)*(r1.yyyy)+(r1.xxxx)).x;
    // 52: mad r2.y, r1.x, cb0[13].z, r1.y
    r2.y = ((r1.xxxx)*(source[13].zzzz)+(r1.yyyy)).y;
    // 53: mad r1.xy, cb0[13].wwww, r0.zwzz, r2.xyxx
    r1.xy = ((source[13].wwww)*(r0.zwzz)+(r2.xyxx)).xy;
    // 54: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s3, l(-1.000000)
    r1.x = (SDNativeSample3((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 55: mad r1.y, v4.x, cb0[15].w, cb0[16].x
    r1.y = ((v4.xxxx)*(source[15].wwww)+(source[16].xxxx)).y;
    // 56: mad r1.y, cb0[16].y, r0.y, r1.y
    r1.y = ((source[16].yyyy)*(r0.yyyy)+(r1.yyyy)).y;
    // 57: add r1.y, r1.y, cb0[16].z
    r1.y = ((r1.yyyy)+(source[16].zzzz)).y;
    // 58: mad r1.z, v4.x, cb0[14].y, cb0[14].z
    r1.z = ((v4.xxxx)*(source[14].yyyy)+(source[14].zzzz)).z;
    // 59: mad r1.z, r0.x, cb0[14].x, r1.z
    r1.z = ((r0.xxxx)*(source[14].xxxx)+(r1.zzzz)).z;
    // 60: add r1.z, r1.z, cb0[15].x
    r1.z = ((r1.zzzz)+(source[15].xxxx)).z;
    // 61: mad r2.x, cb0[15].y, r1.z, r1.y
    r2.x = ((source[15].yyyy)*(r1.zzzz)+(r1.yyyy)).x;
    // 62: mad r2.y, r1.y, cb0[16].w, r1.z
    r2.y = ((r1.yyyy)*(source[16].wwww)+(r1.zzzz)).y;
    // 63: mad r1.yz, cb0[17].xxxx, r0.zzwz, r2.xxyx
    r1.yz = ((source[17].xxxx)*(r0.zzwz)+(r2.xxyx)).yz;
    // 64: sample_l_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t3.yxzw, s4, l(-1.000000)
    r1.y = (SDNativeSample4((r1.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yxzw).y;
    // 65: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 66: mad r1.y, v4.x, cb0[18].y, cb0[18].z
    r1.y = ((v4.xxxx)*(source[18].yyyy)+(source[18].zzzz)).y;
    // 67: mad r1.y, cb0[18].w, r0.y, r1.y
    r1.y = ((source[18].wwww)*(r0.yyyy)+(r1.yyyy)).y;
    // 68: mad r1.z, v4.x, cb0[17].z, cb0[17].w
    r1.z = ((v4.xxxx)*(source[17].zzzz)+(source[17].wwww)).z;
    // 69: mad r1.z, r0.x, cb0[17].y, r1.z
    r1.z = ((r0.xxxx)*(source[17].yyyy)+(r1.zzzz)).z;
    // 70: mad r2.x, cb0[18].x, r1.z, r1.y
    r2.x = ((source[18].xxxx)*(r1.zzzz)+(r1.yyyy)).x;
    // 71: mad r2.y, r1.y, cb0[19].x, r1.z
    r2.y = ((r1.yyyy)*(source[19].xxxx)+(r1.zzzz)).y;
    // 72: mad r1.yz, cb0[19].yyyy, r0.zzwz, r2.xxyx
    r1.yz = ((source[19].yyyy)*(r0.zzwz)+(r2.xxyx)).yz;
    // 73: sample_l_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t4.yxzw, s5, l(-1.000000)
    r1.y = (SDNativeSample5((r1.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yxzw).y;
    // 74: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 75: max r1.x, |r1.x|, l(0.000001)
    r1.x = (max(abs(r1.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 76: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 77: mul r1.y, v4.w, cb0[19].z
    r1.y = ((v4.wwww)*(source[19].zzzz)).y;
    // 78: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 79: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 80: mul_sat r1.x, r1.x, cb0[19].w
    r1.x = (saturate((r1.xxxx)*(source[19].wwww))).x;
    // 81: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 82: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 83: mad r1.y, v4.x, cb0[4].x, cb0[4].y
    r1.y = ((v4.xxxx)*(source[4].xxxx)+(source[4].yyyy)).y;
    // 84: mad r0.y, cb0[4].z, r0.y, r1.y
    r0.y = ((source[4].zzzz)*(r0.yyyy)+(r1.yyyy)).y;
    // 85: add r0.y, r0.y, cb0[4].w
    r0.y = ((r0.yyyy)+(source[4].wwww)).y;
    // 86: mad r1.y, v4.x, cb0[2].y, cb0[2].z
    r1.y = ((v4.xxxx)*(source[2].yyyy)+(source[2].zzzz)).y;
    // 87: mad r0.x, r0.x, cb0[2].x, r1.y
    r0.x = ((r0.xxxx)*(source[2].xxxx)+(r1.yyyy)).x;
    // 88: add r0.x, r0.x, cb0[3].y
    r0.x = ((r0.xxxx)+(source[3].yyyy)).x;
    // 89: mad r2.x, cb0[3].z, r0.x, r0.y
    r2.x = ((source[3].zzzz)*(r0.xxxx)+(r0.yyyy)).x;
    // 90: mad r2.y, r0.y, cb0[5].x, r0.x
    r2.y = ((r0.yyyy)*(source[5].xxxx)+(r0.xxxx)).y;
    // 91: mad r0.xy, cb0[9].wwww, r0.zwzz, r2.xyxx
    r0.xy = ((source[9].wwww)*(r0.zwzz)+(r2.xyxx)).xy;
    // 92: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s0, l(-1.000000)
    r0.xyz = (SDNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 93: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 94: add r1.yzw, -r0.xxyz, r0.wwww
    r1.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 95: mad r0.xyz, cb0[10].xxxx, r1.yzwy, r0.xyzx
    r0.xyz = ((source[10].xxxx)*(r1.yzwy)+(r0.xyzx)).xyz;
    // 96: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 97: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 98: mul r0.xyz, r0.xyzx, cb0[10].yyyy
    r0.xyz = ((r0.xyzx)*(source[10].yyyy)).xyz;
    // 99: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 100: mul r0.xyz, r0.xyzx, cb0[10].zzzz
    r0.xyz = ((r0.xyzx)*(source[10].zzzz)).xyz;
    // 101: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 102: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 103: mul o0.xyz, r1.xxxx, r0.xyzx
    output.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 104: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_w_pa_fd_01_3_tr: 28af9ecbaa26d040820b16dea7d6fbdd; selected map 7b903d701d364ecd68b56da9eb8569e4558885ac9e5c7d5cfbb0fd6bf5a89843.
float4 SDNative345(SD_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[4u];
    source[2].x = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[2].y = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[2].z = (g_SDSourceMaterialTime.xxxx).x;
    source[2].w = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[3].x = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[3].y = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[3].z = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[4].x = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[4].y = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[4].z = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[5].x = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[5].y = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[5].z = ((float4(-0.100000001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[1u].zzzz)).x;
    source[5].w = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[6].y = ((float4(-1.0, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[1u].wwww)).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: add r0.x, v4.x, cb0[5].z
    r0.x = ((v4.xxxx)+(source[5].zzzz)).x;
    // 2: add r0.y, cb0[2].x, l(-1.000000)
    r0.y = ((source[2].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 3: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 4: mad r0.yz, cb0[2].xxxx, v2.xxyx, -r0.yyyy
    r0.yz = ((source[2].xxxx)*(v2.xxyx)+(-(r0.yyyy))).yz;
    // 5: add r1.xy, r0.yzyy, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.yzyy)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 6: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 7: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 8: mad r0.w, r0.w, l(2.000000), l(1.000000)
    r0.w = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 9: add r0.x, -r0.x, r0.w
    r0.x = ((-(r0.xxxx))+(r0.wwww)).x;
    // 10: add_sat r0.x, r0.x, r0.x
    r0.x = (saturate((r0.xxxx)+(r0.xxxx))).x;
    // 11: mul r1.xy, r0.yzyy, cb0[4].wwww
    r1.xy = ((r0.yzyy)*(source[4].wwww)).xy;
    // 12: mad r2.x, cb0[2].z, cb0[4].z, r1.x
    r2.x = ((source[2].zzzz)*(source[4].zzzz)+(r1.xxxx)).x;
    // 13: mad r2.y, cb0[2].z, cb0[5].x, r1.y
    r2.y = ((source[2].zzzz)*(source[5].xxxx)+(r1.yyyy)).y;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (SDNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 15: mad r2.xyz, r0.xxxx, r1.xyzx, r0.xxxx
    r2.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.xxxx)).xyz;
    // 16: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 17: mul r3.xyz, r2.xyzx, r2.xyzx
    r3.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 18: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 19: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 20: min r2.xyz, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 21: add r0.x, r0.w, -v4.x
    r0.x = ((r0.wwww)+(-(v4.xxxx))).x;
    // 22: add_sat r0.x, r0.x, r0.x
    r0.x = (saturate((r0.xxxx)+(r0.xxxx))).x;
    // 23: mad r3.xyz, r0.xxxx, r1.xyzx, r0.xxxx
    r3.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.xxxx)).xyz;
    // 24: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 25: mul r4.xyz, r3.xyzx, r3.xyzx
    r4.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 26: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 27: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 28: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 29: add r2.xyz, r2.xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)+(-(r3.xyzx))).xyz;
    // 30: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 31: mul r4.xyz, r2.xyzx, r2.xyzx
    r4.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 32: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 33: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 34: mul r4.xy, r0.yzyy, cb0[2].wwww
    r4.xy = ((r0.yzyy)*(source[2].wwww)).xy;
    // 35: mad r5.x, cb0[2].z, cb0[2].y, r4.x
    r5.x = ((source[2].zzzz)*(source[2].yyyy)+(r4.xxxx)).x;
    // 36: mad r5.y, cb0[2].z, cb0[3].x, r4.y
    r5.y = ((source[2].zzzz)*(source[3].xxxx)+(r4.yyyy)).y;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, r5.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = (SDNativeSample0((r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 38: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 39: mad r0.xy, cb0[3].yyyy, r4.xyxx, r0.yzyy
    r0.xy = ((source[3].yyyy)*(r4.xyxx)+(r0.yzyy)).xy;
    // 40: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 41: mad r0.xy, r0.xyxx, cb0[3].zwzz, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(source[3].zwzz)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (SDNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 43: dp3 r1.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 44: add r4.xyz, -r0.xyzx, r1.wwww
    r4.xyz = ((-(r0.xyzx))+(r1.wwww)).xyz;
    // 45: mad r0.xyz, cb0[4].xxxx, r4.xyzx, r0.xyzx
    r0.xyz = ((source[4].xxxx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 46: mul r0.xyz, r0.xyzx, cb0[4].yyyy
    r0.xyz = ((r0.xyzx)*(source[4].yyyy)).xyz;
    // 47: mad r0.xyz, r3.xyzx, -r0.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(-(r0.xyzx))+(r0.xyzx)).xyz;
    // 48: mul r2.xyz, r2.xyzx, r0.xyzx
    r2.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 49: mad r0.xyz, cb0[5].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[5].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 50: add r1.w, v4.x, cb0[6].y
    r1.w = ((v4.xxxx)+(source[6].yyyy)).w;
    // 51: add r0.w, r0.w, -r1.w
    r0.w = ((r0.wwww)+(-(r1.wwww))).w;
    // 52: add_sat r0.w, r0.w, r0.w
    r0.w = (saturate((r0.wwww)+(r0.wwww))).w;
    // 53: mad r1.xyz, r0.wwww, r1.xyzx, r0.wwww
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r0.wwww)).xyz;
    // 54: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 55: mul r2.xyz, r1.xyzx, r1.xyzx
    r2.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 56: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 57: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 58: min r1.xyz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 59: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 60: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: mad r0.xyz, v3.xyzx, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 62: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 63: log r0.x, |r0.w|
    r0.x = (log2(abs(r0.wwww))).x;
    // 64: lt r0.y, |r0.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 65: mul r0.x, r0.x, v4.y
    r0.x = ((r0.xxxx)*(v4.yyyy)).x;
    // 66: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 67: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 68: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 69: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}

// fx_s_pa_glasshole_02_01: a236f7b1d9bf3b41b50036b7512648ee; selected map 0d74b580697fd5dbcb0ef414366c3c449f375b9d702345364c7f48859f4fdd97.
float4 SDNative347(SD_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[2u];
    source[2] = SDNativeAppend(g_SDSourceMaterialParameters[0u].xxxx,g_SDSourceMaterialParameters[0u].yyyy,1u);
    source[3] = SDNativeAppend(g_SDSourceMaterialParameters[0u].wwww,g_SDSourceMaterialParameters[1u].xxxx,1u);
    source[4] = SDNativeAppend(SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].zzzz)*float4(-0.230000004, 0.0, 0.0, 0.0))),1u);
    source[5] = SDNativeAppend(SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].zzzz)*float4(-0.5, 0.0, 0.0, 0.0))),SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].zzzz)*float4(0.370000005, 0.0, 0.0, 0.0))),1u);
    source[6] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))),1u);
    source[7] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0))),1u);
    source[8] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[9].x = (SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].zzzz)*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[9].z = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0)))).x;
    source[9].w = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[10].x = (g_SDSourceMaterialParameters[1u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xy, v2.xyxx, -cb0[2].xyxx
    r0.xy = ((v2.xyxx)+(-(source[2].xyxx))).xy;
    // 2: max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 5: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 6: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 7: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).x;
    // 8: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 9: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).x;
    // 10: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 11: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 12: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).x;
    // 13: lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // 20: max r0.x, r0.y, r0.x
    r0.x = (max(r0.yyyy,r0.xxxx)).x;
    // 21: ge r0.x, r0.x, -r0.x
    r0.x = (asfloat((uint4)((r0.xxxx)>=(-(r0.xxxx))) * 0xffffffffu)).x;
    // 22: lt r0.y, r0.w, -r0.w
    r0.y = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).y;
    // 23: and r0.x, r0.x, r0.y
    r0.x = (asfloat(asuint(r0.xxxx) & asuint(r0.yyyy))).x;
    // 24: movc r0.x, r0.x, -r0.z, r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).x;
    // 25: mad r0.x, r0.x, l(0.159155), l(0.500000)
    r0.x = ((r0.xxxx)*(float4(asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u)))+(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).x;
    // 26: mad r1.xy, v2.xyxx, cb0[3].xyxx, cb0[4].xyxx
    r1.xy = ((v2.xyxx)*(source[3].xyxx)+(source[4].xyxx)).xy;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s2, l(0.000000)
    r0.w = (SDNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 28: mad r0.w, r0.w, l(2.000000), l(-1.000000)
    r0.w = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 29: mad r1.xy, v2.xyxx, cb0[3].xyxx, cb0[5].xyxx
    r1.xy = ((v2.xyxx)*(source[3].xyxx)+(source[5].xyxx)).xy;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s2, l(0.000000)
    r1.x = (SDNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 31: mad r1.x, r1.x, l(2.000000), l(-1.000000)
    r1.x = ((r1.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 32: add r1.yz, v4.xxyx, l(0.000000, -0.500000, -0.900000, 0.000000)
    r1.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.900000,0.000000))).yz;
    // 33: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 34: mad r0.w, r1.z, r0.w, r1.x
    r0.w = ((r1.zzzz)*(r0.wwww)+(r1.xxxx)).w;
    // 35: mul r0.w, r0.w, cb0[9].y
    r0.w = ((r0.wwww)*(source[9].yyyy)).w;
    // 36: add r1.xz, v2.xxyx, l(-0.500000, 0.000000, -0.500000, 0.000000)
    r1.xz = ((v2.xxyx)+(float4(-0.500000,0.000000,-0.500000,0.000000))).xz;
    // 37: dp2 r1.x, r1.xzxx, r1.xzxx
    r1.x = (dot((r1.xzxx).xy,(r1.xzxx).xy).xxxx).x;
    // 38: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 39: mul r1.x, r1.x, v4.z
    r1.x = ((r1.xxxx)*(v4.zzzz)).x;
    // 40: mad r1.z, -r1.x, l(2.000000), l(1.000000)
    r1.z = ((-(r1.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 41: mad r1.x, r1.x, l(3.000000), -r1.y
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))+(-(r1.yyyy))).x;
    // 42: max r1.x, r1.x, l(-0.100000)
    r1.x = (max(r1.xxxx,float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 43: min r0.y, r1.x, l(10.000000)
    r0.y = (min(r1.xxxx,float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 44: mad r1.xyzw, r1.zzzz, r0.wwww, r0.xyxy
    r1.xyzw = ((r1.zzzz)*(r0.wwww)+(r0.xyxy)).xyzw;
    // 45: sample_l_indexable(texture2d)(float,float,float,float) r0.yz, r1.zwzz, t1.yzxw, s4, l(-1.000000)
    r0.yz = (SDNativeSample3((r1.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).yz;
    // 46: mad r0.xw, r0.xxxz, l(7.000000, 0.000000, 0.000000, 1.000000), cb0[8].xxxy
    r0.xw = ((r0.xxxz)*(float4(7.000000,0.000000,0.000000,1.000000))+(source[8].xxxy)).xw;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.xwxx, t5.xyzw, s5, l(0.000000)
    r2.xyz = (SDNativeSample4((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 48: mul r2.xyz, r0.zzzz, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r2.xyzx)).xyz;
    // 49: mul_sat r2.xyz, r2.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000)
    r2.xyz = (saturate((r2.xyzx)*(float4(4.000000,4.000000,4.000000,0.000000)))).xyz;
    // 50: mad r0.xw, r1.xxxy, l(7.000000, 0.000000, 0.000000, 1.000000), cb0[6].xxxy
    r0.xw = ((r1.xxxy)*(float4(7.000000,0.000000,0.000000,1.000000))+(source[6].xxxy)).xw;
    // 51: mad r1.xy, r1.zwzz, l(7.000000, 0.500000, 0.000000, 0.000000), cb0[7].xyxx
    r1.xy = ((r1.zwzz)*(float4(7.000000,0.500000,0.000000,0.000000))+(source[7].xyxx)).xy;
    // 52: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s3, l(-1.000000)
    r1.xyz = (SDNativeSample2((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 53: sample_l_indexable(texture2d)(float,float,float,float) r3.xyz, r0.xwxx, t3.xyzw, s1, l(-1.000000)
    r3.xyz = (SDNativeSample0((r0.xwxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 54: mul r4.xyz, r1.xyzx, r3.xyzx
    r4.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 55: add r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)+(r3.xyzx)).xyz;
    // 56: mul r3.xyz, r0.zzzz, r1.xyzx
    r3.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 57: mul r0.xw, r1.xxxy, l(0.200000, 0.000000, 0.000000, 0.200000)
    r0.xw = ((r1.xxxy)*(float4(0.200000,0.000000,0.000000,0.200000))).xw;
    // 58: mul r1.xyz, r3.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = ((r3.xyzx)*(float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 59: mad r1.xyz, r4.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000), r1.xyzx
    r1.xyz = ((r4.xyzx)*(float4(4.000000,4.000000,4.000000,0.000000))+(r1.xyzx)).xyz;
    // 60: mad r1.xyz, r2.xyzx, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 61: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 62: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 63: mad r0.xw, v6.xxxy, r1.wwww, r0.xxxw
    r0.xw = ((v6.xxxy)*(r1.wwww)+(r0.xxxw)).xw;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.xwxx, t6.xyzw, s6, l(0.000000)
    r2.xyz = (SDNativeSample5((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 65: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 66: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 67: add r2.xyz, -r1.xyzx, r0.xxxx
    r2.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 68: mad r1.xyz, cb0[10].xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((source[10].xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 69: div r0.xw, v7.xxxy, v7.wwww
    r0.xw = ((v7.xxxy)/(v7.wwww)).xw;
    // 70: mad r0.xw, r0.xxxw, cb2[0].xxxy, cb2[0].wwwz
    r0.xw = ((r0.xxxw)*(passValues[0].xxxy)+(passValues[0].wwwz)).xw;
    // Native 71: source device depth mapped to centimetre view depth; reconstruction at 73.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xwxx).xy, 0.f).y * 100000.f;
    // Native 73-76: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 77: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 78: mul_sat r0.xw, r0.xxxx, l(0.034483, 0.000000, 0.000000, 0.066667)
    r0.xw = (saturate((r0.xxxx)*(float4(asfloat(0x3d0d3dcbu),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x3d888889u))))).xw;
    // 79: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 80: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 81: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 82: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 83: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 84: mul r2.xyz, r1.wwww, l(10.000000, 10.000000, 20.000000, 0.000000)
    r2.xyz = ((r1.wwww)*(float4(10.000000,10.000000,20.000000,0.000000))).xyz;
    // 85: movc r2.xyz, r0.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 86: mad r1.xyz, r0.yyyy, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 87: add r0.y, -r0.y, r0.z
    r0.y = ((-(r0.yyyy))+(r0.zzzz)).y;
    // 88: add_sat r0.y, r0.y, l(1.000000)
    r0.y = (saturate((r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000)))).y;
    // 89: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 90: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 91: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 92: mad r0.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 93: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_y_pa_hole_10_01_tr: 50de0bd74036c64da64c66ca562a5232; selected map 90b68a0a59b6521506262e2d2131052aad4064ff76ca63c982c91b483cfe3597.
float4 SDNative348(SD_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[10u];
    source[2] = g_SDSourceMaterialParameters[9u];
    source[3] = g_SDSourceMaterialParameters[7u];
    source[4] = SDNativeAppend(g_SDSourceMaterialParameters[0u].yyyy,g_SDSourceMaterialParameters[1u].xxxx,1u);
    source[5] = g_SDSourceMaterialParameters[8u];
    source[6].x = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[6].y = (g_SDSourceMaterialTime.xxxx).x;
    source[6].z = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[6].w = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[7].y = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[7].z = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[7].w = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[8].x = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[8].y = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[8].z = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[8].w = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[9].x = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[9].y = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[9].z = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[9].w = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[10].x = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[10].y = (g_SDSourceMaterialParameters[5u].zzzz).x;
    source[10].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[10].w = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[11].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[11].y = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[11].z = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[11].w = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[12].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[12].y = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[12].z = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[13].x = (g_SDSourceMaterialParameters[5u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mul r0.xy, v2.xyxx, cb0[7].yzyy
    r0.xy = ((v2.xyxx)*(source[7].yzyy)).xy;
    // 2: mad r0.xy, cb0[6].yyyy, cb0[7].xwxx, r0.xyxx
    r0.xy = ((source[6].yyyy)*(source[7].xwxx)+(r0.xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.xyz = (SDNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 4: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 5: add r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)+(r0.xxxx)).x;
    // 6: mul r0.x, r0.x, cb0[8].x
    r0.x = ((r0.xxxx)*(source[8].xxxx)).x;
    // 7: mad r0.x, r0.x, l(0.330000), cb0[8].y
    r0.x = ((r0.xxxx)*(float4(0.330000,0.330000,0.330000,0.330000))+(source[8].yyyy)).x;
    // 8: mad r0.x, r0.x, l(0.050000), l(-0.025000)
    r0.x = ((r0.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).x;
    // 9: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 10: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 11: mul r0.yz, r0.yyyy, v6.xxyx
    r0.yz = ((r0.yyyy)*(v6.xxyx)).yz;
    // 12: mad r0.xy, r0.xxxx, r0.yzyy, v2.xyxx
    r0.xy = ((r0.xxxx)*(r0.yzyy)+(v2.xyxx)).xy;
    // 13: mul r0.zw, r0.xxxy, cb0[6].zzzw
    r0.zw = ((r0.xxxy)*(source[6].zzzw)).zw;
    // 14: mad r1.x, cb0[6].y, cb0[6].x, r0.z
    r1.x = ((source[6].yyyy)*(source[6].xxxx)+(r0.zzzz)).x;
    // 15: mad r1.y, cb0[6].y, cb0[8].z, r0.w
    r1.y = ((source[6].yyyy)*(source[8].zzzz)+(r0.wwww)).y;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t3.yzxw, s2, l(0.000000)
    r0.z = (SDNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 17: mul r1.xy, r0.xyxx, cb0[9].yzyy
    r1.xy = ((r0.xyxx)*(source[9].yzyy)).xy;
    // 18: mov_sat r0.xy, r0.xyxx
    r0.xy = (saturate(r0.xyxx)).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s3, l(0.000000)
    r0.x = (SDNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 20: mul_sat r0.x, r0.x, cb0[10].z
    r0.x = (saturate((r0.xxxx)*(source[10].zzzz))).x;
    // 21: mad r0.yw, cb0[6].yyyy, cb0[9].xxxw, r1.xxxy
    r0.yw = ((source[6].yyyy)*(source[9].xxxw)+(r1.xxxy)).yw;
    // 22: mul r1.x, v4.w, cb0[8].w
    r1.x = ((v4.wwww)*(source[8].wwww)).x;
    // 23: mad r0.yz, r0.zzzz, r1.xxxx, r0.yywy
    r0.yz = ((r0.zzzz)*(r1.xxxx)+(r0.yywy)).yz;
    // 24: mul r1.xy, r0.yzyy, l(2.300000, 2.300000, 0.000000, 0.000000)
    r1.xy = ((r0.yzyy)*(float4(2.300000,2.300000,0.000000,0.000000))).xy;
    // 25: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t4.wxyz, s0, l(-1.000000)
    r0.yzw = (SDNativeSample0((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 26: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s0, l(-1.000000)
    r1.xyz = (SDNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 27: mul r1.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 28: mad r0.yzw, r0.yyzw, l(0.000000, 0.500000, 0.500000, 0.500000), r1.xxyz
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.500000,0.500000,0.500000))+(r1.xxyz)).yzw;
    // 29: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 30: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 31: mad r0.yzw, cb0[10].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[10].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 32: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 33: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 34: mul r0.yzw, r0.yyzw, cb0[10].yyyy
    r0.yzw = ((r0.yyzw)*(source[10].yyyy)).yzw;
    // 35: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 36: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 37: mul r2.xyz, r0.yzwy, r1.xyzx
    r2.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 38: mul r3.xyz, cb0[3].xyzx, cb0[3].wwww
    r3.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 39: mul_sat r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = (saturate((r2.xyzx)*(r3.xyzx))).xyz;
    // 40: mad r0.yzw, r0.yyzw, r1.xxyz, -r2.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)+(-(r2.xxyz))).yzw;
    // 41: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 42: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 43: mul r1.x, r1.x, cb0[10].w
    r1.x = ((r1.xxxx)*(source[10].wwww)).x;
    // 44: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 45: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 46: mad r0.xyz, r0.xxxx, r0.yzwy, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 47: mul r1.xy, v2.xyxx, cb0[4].xyxx
    r1.xy = ((v2.xyxx)*(source[4].xyxx)).xy;
    // 48: mul r1.zw, r1.xxxy, l(0.000000, 0.000000, 1.660000, 1.660000)
    r1.zw = ((r1.xxxy)*(float4(0.000000,0.000000,1.660000,1.660000))).zw;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t5.xyzw, s4, l(0.000000)
    r2.xyz = (SDNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.zwzz, t5.xyzw, s4, l(0.000000)
    r1.xyz = (SDNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 51: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 52: mul r2.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r2.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 53: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 54: mad r1.xyz, -r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r0.wwww
    r1.xyz = ((-(r1.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(r0.wwww)).xyz;
    // 55: mad r1.xyz, cb0[11].zzzz, r1.xyzx, r2.xyzx
    r1.xyz = ((source[11].zzzz)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 56: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 57: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 58: mul r1.xyz, r1.xyzx, cb0[11].wwww
    r1.xyz = ((r1.xyzx)*(source[11].wwww)).xyz;
    // 59: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 60: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 61: mad r1.xyz, r1.xyzx, r2.xyzx, -r0.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 62: add r2.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 63: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 64: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 65: mad r1.w, r0.w, l(2.000000), -v4.x
    r1.w = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(v4.xxxx))).w;
    // 66: mad r0.w, -r0.w, l(2.000000), v4.y
    r0.w = ((-(r0.wwww))*(float4(2.000000,2.000000,2.000000,2.000000))+(v4.yyyy)).w;
    // 67: add_sat r0.w, r0.w, l(1.000000)
    r0.w = (saturate((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 68: add_sat r1.w, r1.w, l(1.000000)
    r1.w = (saturate((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 69: mul r2.xy, v2.xyxx, cb0[12].xxxx
    r2.xy = ((v2.xyxx)*(source[12].xxxx)).xy;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.xyxx, t1.xyzw, s5, l(0.000000)
    r2.x = (SDNativeSample5((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 71: add r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)+(r2.xxxx)).w;
    // 72: add r0.w, r0.w, r2.x
    r0.w = ((r0.wwww)+(r2.xxxx)).w;
    // 73: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 74: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 75: mul r2.x, r2.x, cb0[12].y
    r2.x = ((r2.xxxx)*(source[12].yyyy)).x;
    // 76: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 77: mul_sat r2.x, r2.x, cb0[12].z
    r2.x = (saturate((r2.xxxx)*(source[12].zzzz))).x;
    // 78: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 79: mad r0.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 80: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 81: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 82: log r0.x, |r0.w|
    r0.x = (log2(abs(r0.wwww))).x;
    // 83: lt r0.y, |r0.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 84: mul r0.x, r0.x, cb0[12].y
    r0.x = ((r0.xxxx)*(source[12].yyyy)).x;
    // 85: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 86: mul_sat r0.x, r0.x, cb0[12].z
    r0.x = (saturate((r0.xxxx)*(source[12].zzzz))).x;
    // 87: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 88: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.yxzw, s3, l(0.000000)
    r0.y = (SDNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 89: mul_sat r0.y, r0.y, cb0[12].w
    r0.y = (saturate((r0.yyyy)*(source[12].wwww))).y;
    // 90: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 91: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 92: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 93: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 94: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 95: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 96: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 97: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}

// fx_w_pa_ringmaster_01_20_dt_ad: 88f6b6f9a7431345acb5fda186b51747; selected map 284480d85988cdfd094c406349538aa6eaa7d01a923d16902b5bcfc37254a3ce.
float4 SDNative349(SD_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[7u];
    source[2] = g_SDSourceMaterialParameters[6u];
    source[3] = SDNativeAppend(g_SDSourceMaterialParameters[3u].zzzz,g_SDSourceMaterialParameters[3u].wwww,1u);
    source[4].x = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[4].y = (g_SDSourceMaterialTime.xxxx).x;
    source[4].z = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[5].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[5].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[5].w = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[6].x = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[6].y = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[6].z = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[7].x = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[7].y = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[7].z = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[7].w = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_SDSourceMaterialParameters[0u].yyyy)).x;
    source[8].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_SDSourceMaterialParameters[0u].yyyy))).x;
    source[8].y = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].zzzz)).x;
    source[8].w = (max((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].x = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[9].z = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[9].w = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[10].x = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[10].y = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[10].z = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[10].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
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
    // 10: add r0.y, -cb0[10].w, l(1.000000)
    r0.y = ((-(source[10].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 14: max r0.w, |r0.z|, |r0.y|
    r0.w = (max(abs(r0.zzzz),abs(r0.yyyy))).w;
    // 15: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 16: min r1.x, |r0.z|, |r0.y|
    r1.x = (min(abs(r0.zzzz),abs(r0.yyyy))).x;
    // 17: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 18: mul r1.x, r0.w, r0.w
    r1.x = ((r0.wwww)*(r0.wwww)).x;
    // 19: mad r1.y, r1.x, l(0.020835), l(-0.085133)
    r1.y = ((r1.xxxx)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).y;
    // 20: mad r1.y, r1.x, r1.y, l(0.180141)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(0.180141,0.180141,0.180141,0.180141))).y;
    // 21: mad r1.y, r1.x, r1.y, l(-0.330299)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).y;
    // 22: mad r1.x, r1.x, r1.y, l(0.999866)
    r1.x = ((r1.xxxx)*(r1.yyyy)+(float4(0.999866,0.999866,0.999866,0.999866))).x;
    // 23: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 24: mad r1.y, r1.y, l(-2.000000), l(1.570796)
    r1.y = ((r1.yyyy)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).y;
    // 25: lt r1.z, |r0.z|, |r0.y|
    r1.z = (asfloat((uint4)((abs(r0.zzzz))<(abs(r0.yyyy))) * 0xffffffffu)).z;
    // 26: and r1.y, r1.z, r1.y
    r1.y = (asfloat(asuint(r1.zzzz) & asuint(r1.yyyy))).y;
    // 27: mad r0.w, r0.w, r1.x, r1.y
    r0.w = ((r0.wwww)*(r1.xxxx)+(r1.yyyy)).w;
    // 28: lt r1.x, r0.z, -r0.z
    r1.x = (asfloat((uint4)((r0.zzzz)<(-(r0.zzzz))) * 0xffffffffu)).x;
    // 29: and r1.x, r1.x, l(0xc0490fdb)
    r1.x = (asfloat(asuint(r1.xxxx) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).x;
    // 30: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 31: min r1.x, r0.z, r0.y
    r1.x = (min(r0.zzzz,r0.yyyy)).x;
    // 32: lt r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)<(-(r1.xxxx))) * 0xffffffffu)).x;
    // 33: max r1.y, r0.z, r0.y
    r1.y = (max(r0.zzzz,r0.yyyy)).y;
    // 34: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 35: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 36: ge r0.z, r1.y, -r1.y
    r0.z = (asfloat((uint4)((r1.yyyy)>=(-(r1.yyyy))) * 0xffffffffu)).z;
    // 37: and r0.z, r0.z, r1.x
    r0.z = (asfloat(asuint(r0.zzzz) & asuint(r1.xxxx))).z;
    // 38: movc r0.z, r0.z, -r0.w, r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (-(r0.wwww)) : (r0.wwww)).z;
    // 39: mad r1.x, r0.z, l(0.159155), l(0.500000)
    r1.x = ((r0.zzzz)*(float4(asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u)))+(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).x;
    // 40: mul r0.z, v2.x, cb0[5].w
    r0.z = ((v2.xxxx)*(source[5].wwww)).z;
    // 41: mul r0.w, cb0[4].x, cb0[4].y
    r0.w = ((source[4].xxxx)*(source[4].yyyy)).w;
    // 42: mad r2.x, r0.w, cb0[5].z, r0.z
    r2.x = ((r0.wwww)*(source[5].zzzz)+(r0.zzzz)).x;
    // 43: mul r1.zw, r0.wwww, cb0[6].yyyw
    r1.zw = ((r0.wwww)*(source[6].yyyw)).zw;
    // 44: mad r2.y, cb0[6].x, v2.y, r1.z
    r2.y = ((source[6].xxxx)*(v2.yyyy)+(r1.zzzz)).y;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s2, l(0.000000)
    r2.xy = (SDNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 46: mul r0.z, v4.y, cb0[6].z
    r0.z = ((v4.yyyy)*(source[6].zzzz)).z;
    // 47: add r1.z, r0.y, r0.y
    r1.z = ((r0.yyyy)+(r0.yyyy)).z;
    // 48: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 49: mul r2.z, v4.z, cb0[5].y
    r2.z = ((v4.zzzz)*(source[5].yyyy)).z;
    // 50: mul r1.z, r1.z, r2.z
    r1.z = ((r1.zzzz)*(r2.zzzz)).z;
    // 51: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 52: lt r2.z, r0.y, l(0.000000)
    r2.z = (asfloat((uint4)((r0.yyyy)<(float4(asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu)))) * 0xffffffffu)).z;
    // 53: mad r0.y, -r0.y, cb0[8].x, l(1.000000)
    r0.y = ((-(r0.yyyy))*(source[8].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 54: mul_sat r0.y, r0.y, cb0[9].x
    r0.y = (saturate((r0.yyyy)*(source[9].xxxx))).y;
    // 55: movc r1.y, r2.z, l(0), r1.z
    r1.y = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 56: mad r1.xy, r0.zzzz, r2.xyxx, r1.xyxx
    r1.xy = ((r0.zzzz)*(r2.xyxx)+(r1.xyxx)).xy;
    // 57: mul r2.x, r1.x, cb0[3].x
    r2.x = ((r1.xxxx)*(source[3].xxxx)).x;
    // 58: mad r3.y, r1.y, cb0[3].y, v4.x
    r3.y = ((r1.yyyy)*(source[3].yyyy)+(v4.xxxx)).y;
    // 59: mul r3.x, cb0[4].y, cb0[10].x
    r3.x = ((source[4].yyyy)*(source[10].xxxx)).x;
    // 60: mov r2.z, l(-1.000000)
    r2.z = (float4(-1.000000,-1.000000,-1.000000,-1.000000)).z;
    // 61: add r2.xy, r2.xzxx, r3.xyxx
    r2.xy = ((r2.xzxx)+(r3.xyxx)).xy;
    // 62: mul r2.xy, r2.xyxx, cb0[4].xxxx
    r2.xy = ((r2.xyxx)*(source[4].xxxx)).xy;
    // 63: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r2.xyxx, t1.yzxw, s3, l(-1.000000)
    r0.z = (SDNativeSample2((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).z;
    // 64: log r1.z, r0.y
    r1.z = (log2(r0.yyyy)).z;
    // 65: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 66: mul r1.z, r1.z, cb0[9].y
    r1.z = ((r1.zzzz)*(source[9].yyyy)).z;
    // 67: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 68: movc r0.y, r0.y, l(0), r1.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 69: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 70: mul_sat r0.y, r0.y, cb0[10].y
    r0.y = (saturate((r0.yyyy)*(source[10].yyyy))).y;
    // 71: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 72: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 73: mul r0.z, r0.z, cb0[10].z
    r0.z = ((r0.zzzz)*(source[10].zzzz)).z;
    // 74: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 75: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 76: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 77: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 78: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 79: mul r0.y, r1.x, cb0[4].w
    r0.y = ((r1.xxxx)*(source[4].wwww)).y;
    // 80: mad r1.y, cb0[5].x, r1.y, r1.w
    r1.y = ((source[5].xxxx)*(r1.yyyy)+(r1.wwww)).y;
    // 81: mad r1.x, r0.w, cb0[4].z, r0.y
    r1.x = ((r0.wwww)*(source[4].zzzz)+(r0.yyyy)).x;
    // 82: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t3.wxyz, s1, l(-1.000000)
    r0.yzw = (SDNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 83: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 84: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 85: mad r0.yzw, cb0[7].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[7].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 86: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 87: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 88: mul r0.yzw, r0.yyzw, cb0[7].yyyy
    r0.yzw = ((r0.yyzw)*(source[7].yyyy)).yzw;
    // 89: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 90: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 91: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 92: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 93: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 94: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 95: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_s_pa_turbulence_01_11_dt_tr: 370ed5c5ef0e3f4fa5bdde242eb7dcf9; selected map e87406786dfc80da1e45887c90351b1856c31745afd89d3620ad3840df376785.
float4 SDNative351(SD_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[4u];
    source[2] = g_SDSourceMaterialParameters[3u];
    source[3].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[3].y = (g_SDSourceMaterialTime.xxxx).x;
    source[3].z = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[3].w = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[4].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[4].z = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[4].w = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[5].x = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[5].y = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[5].w = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[6].x = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[6].y = ((float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].xxxx)).x;
    source[6].z = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].xxxx))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
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
    // 10: add r0.y, -cb0[6].z, l(1.000000)
    r0.y = ((-(source[6].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, l(100.000000)
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 12: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 13: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, v2.xyxx, t1.zxyw, s1, l(0.000000)
    r0.yz = (SDNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 15: add r1.xyzw, -r0.yzyz, v2.xyxy
    r1.xyzw = ((-(r0.yzyz))+(v2.xyxy)).xyzw;
    // 16: mad r1.xyzw, v4.xxyy, r1.xyzw, r0.yzyz
    r1.xyzw = ((v4.xxyy)*(r1.xyzw)+(r0.yzyz)).xyzw;
    // 17: mul r0.yz, r1.xxyx, cb0[5].yyyy
    r0.yz = ((r1.xxyx)*(source[5].yyyy)).yz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t3.yxzw, s5, l(0.000000)
    r0.y = (SDNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 19: mul_sat r0.y, r0.y, cb0[5].z
    r0.y = (saturate((r0.yyyy)*(source[5].zzzz))).y;
    // 20: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 21: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 22: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 23: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 24: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t2.yzxw, s4, l(0.000000)
    r0.z = (SDNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 26: mul r1.xy, r1.zwzz, cb0[3].zwzz
    r1.xy = ((r1.zwzz)*(source[3].zwzz)).xy;
    // 27: mul_sat r0.z, r0.z, cb0[4].w
    r0.z = (saturate((r0.zzzz)*(source[4].wwww))).z;
    // 28: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 29: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 30: mul r0.w, r0.w, cb0[5].x
    r0.w = ((r0.wwww)*(source[5].xxxx)).w;
    // 31: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 32: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 33: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 34: mad r0.y, -r0.z, r0.y, r0.z
    r0.y = ((-(r0.zzzz))*(r0.yyyy)+(r0.zzzz)).y;
    // 35: mad r0.y, v4.z, r0.y, r0.w
    r0.y = ((v4.zzzz)*(r0.yyyy)+(r0.wwww)).y;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v2.xyxx, t0.yzxw, s3, l(0.000000)
    r0.z = (SDNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 37: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 38: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 39: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 40: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 41: mad r0.x, cb0[3].y, cb0[3].x, r1.x
    r0.x = ((source[3].yyyy)*(source[3].xxxx)+(r1.xxxx)).x;
    // 42: mad r0.y, cb0[3].y, cb0[4].x, r1.y
    r0.y = ((source[3].yyyy)*(source[4].xxxx)+(r1.yyyy)).y;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s2, l(0.000000)
    r0.xyz = (SDNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 44: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 45: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 46: mad r0.xyz, cb0[4].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[4].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 47: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 48: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 49: mul r0.xyz, r0.xyzx, cb0[4].zzzz
    r0.xyz = ((r0.xyzx)*(source[4].zzzz)).xyz;
    // 50: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 51: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 52: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 53: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 54: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_d_pa_afterburn_01_101_dt_tr: 461f29f977b1c241bca533ae89e12229; selected map c4a929c9743b7944f0da5d3ecee3fa714e385c37e49b60b2f61e9740403a64cb.
float4 SDNative352(SD_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[5u];
    source[2] = g_SDSourceMaterialParameters[3u];
    source[3] = g_SDSourceMaterialParameters[4u];
    source[4] = SDNativeAppend(cos((g_SDSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[5] = SDNativeAppend(sin((g_SDSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[6] = SDNativeAppend(cos(((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[7] = SDNativeAppend(sin(((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[8] = SDNativeAppend(cos((((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[9] = SDNativeAppend(sin((((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos((((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[10].x = (cos((g_SDSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[10].y = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[10].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[10].w = ((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)).x;
    source[11].x = (cos((((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[11].z = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[11].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[12].x = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[12].y = ((g_SDSourceMaterialParameters[1u].zzzz*float4(3.0, 0.0, 0.0, 0.0))).x;
    source[12].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[12].w = ((g_SDSourceMaterialParameters[1u].wwww*float4(20.0, 0.0, 0.0, 0.0))).x;
    source[13].x = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[13].y = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[13].z = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[13].w = ((float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].yyyy)).x;
    source[14].x = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].yyyy))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mul r0.x, v4.w, cb0[10].y
    r0.x = ((v4.wwww)*(source[10].yyyy)).x;
    // 2: mad r0.yz, r0.xxxx, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.xxxx)*(v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 3: mul r1.xyzw, r0.xxxx, l(1.330000, 1.330000, 1.768900, 1.768900)
    r1.xyzw = ((r0.xxxx)*(float4(asfloat(0x3faa3d71u),asfloat(0x3faa3d71u),asfloat(0x3fe26b52u),asfloat(0x3fe26b52u)))).xyzw;
    // 4: mad r1.xyzw, r1.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r1.xyzw = ((r1.xyzw)*(v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 5: dp2 r0.x, cb0[5].xyxx, r0.yzyy
    r0.x = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 6: dp2 r2.x, cb0[4].xyxx, r0.yzyy
    r2.x = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 7: mad r2.y, v4.x, l(0.020000), r0.x
    r2.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.xxxx)).y;
    // 8: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (SDNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 10: dp2 r0.y, cb0[7].xyxx, r1.xyxx
    r0.y = (dot((source[7].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 11: mad r2.y, v4.x, l(0.020000), r0.y
    r2.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.yyyy)).y;
    // 12: dp2 r2.x, cb0[6].xyxx, r1.xyxx
    r2.x = (dot((source[6].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 13: add r0.yz, r2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (SDNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: mul r0.y, r0.y, l(0.333300)
    r0.y = ((r0.yyyy)*(float4(0.333300,0.333300,0.333300,0.333300))).y;
    // 16: mad r0.x, r0.x, l(0.333300), r0.y
    r0.x = ((r0.xxxx)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.yyyy)).x;
    // 17: dp2 r0.y, cb0[9].xyxx, r1.zwzz
    r0.y = (dot((source[9].xyxx).xy,(r1.zwzz).xy).xxxx).y;
    // 18: dp2 r1.x, cb0[8].xyxx, r1.zwzz
    r1.x = (dot((source[8].xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 19: mad r1.y, v4.x, l(0.020000), r0.y
    r1.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.yyyy)).y;
    // 20: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (SDNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 22: mad r0.x, r0.y, l(0.333300), r0.x
    r0.x = ((r0.yyyy)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.xxxx)).x;
    // 23: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 24: mul r0.y, r0.y, cb0[12].y
    r0.y = ((r0.yyyy)*(source[12].yyyy)).y;
    // 25: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 26: mul r0.y, r0.y, cb0[12].w
    r0.y = ((r0.yyyy)*(source[12].wwww)).y;
    // 27: mov_sat r0.z, v4.z
    r0.z = (saturate(v4.zzzz)).z;
    // 28: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 30: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 31: movc r0.y, r0.z, l(-0.000000), -r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).y;
    // 32: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 33: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 34: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 35: mad r0.z, -r0.z, l(1.428571), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(asfloat(0x3fb6db6eu),asfloat(0x3fb6db6eu),asfloat(0x3fb6db6eu),asfloat(0x3fb6db6eu)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).z;
    // 36: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 37: add r0.w, r0.z, r0.x
    r0.w = ((r0.zzzz)+(r0.xxxx)).w;
    // 38: mul_sat r0.x, r0.x, cb0[11].y
    r0.x = (saturate((r0.xxxx)*(source[11].yyyy))).x;
    // 39: add r0.w, r0.w, -cb0[11].w
    r0.w = ((r0.wwww)+(-(source[11].wwww))).w;
    // 40: mad r0.y, r0.w, r0.z, r0.y
    r0.y = ((r0.wwww)*(r0.zzzz)+(r0.yyyy)).y;
    // 41: mul_sat r0.y, r0.y, cb0[13].x
    r0.y = (saturate((r0.yyyy)*(source[13].xxxx))).y;
    // 42: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 43: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 44: mul r0.z, r0.z, cb0[13].y
    r0.z = ((r0.zzzz)*(source[13].yyyy)).z;
    // 45: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 46: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 47: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 48: source device depth mapped to centimetre view depth; reconstruction at 50.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 50-53: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 54: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 55: add r1.x, -cb0[14].x, l(1.000000)
    r1.x = ((-(source[14].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 56: mul r1.x, r1.x, l(100.000000)
    r1.x = ((r1.xxxx)*(float4(100.000000,100.000000,100.000000,100.000000))).x;
    // 57: max r1.x, r1.x, l(0.001000)
    r1.x = (max(r1.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 58: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 59: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 60: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 61: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 62: movc o0.w, r0.y, l(0), r0.z
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).w;
    // 63: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 64: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 65: mul r0.y, r0.y, cb0[11].z
    r0.y = ((r0.yyyy)*(source[11].zzzz)).y;
    // 66: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 67: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 68: mul r0.yzw, cb0[2].xxyz, cb0[2].wwww
    r0.yzw = ((source[2].xxyz)*(source[2].wwww)).yzw;
    // 69: mad r1.xyz, cb0[3].wwww, cb0[3].xyzx, -r0.yzwy
    r1.xyz = ((source[3].wwww)*(source[3].xyzx)+(-(r0.yzwy))).xyz;
    // 70: mad r0.xyz, r0.xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 71: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 72: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_b_pa_gl_01_1_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 SDNative360(SD_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[3u];
    source[2] = g_SDSourceMaterialParameters[2u];
    source[3] = SDNativeAppend(g_SDSourceMaterialParameters[0u].wwww,g_SDSourceMaterialParameters[1u].xxxx,1u);
    source[4] = SDNativeAppend(g_SDSourceMaterialParameters[1u].zzzz,g_SDSourceMaterialParameters[1u].wwww,1u);
    source[5].x = (g_SDSourceMaterialTime.xxxx).x;
    source[5].y = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_SDSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xy, cb0[5].xxxx, cb0[3].xyxx, v2.xyxx
    r0.xy = ((source[5].xxxx)*(source[3].xyxx)+(v2.xyxx)).xy;
    // 2: mul r0.xy, r0.xyxx, cb0[5].wwww
    r0.xy = ((r0.xyxx)*(source[5].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (SDNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r0.zw, cb0[5].xxxx, cb0[4].xxxy, v2.xxxy
    r0.zw = ((source[5].xxxx)*(source[4].xxxy)+(v2.xxxy)).zw;
    // 5: mad r0.xy, cb0[6].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[6].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = (SDNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 7: mul r1.xyz, r0.xyzx, cb0[6].wwww
    r1.xyz = ((r0.xyzx)*(source[6].wwww)).xyz;
    // 8: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 9: mad r0.xyz, -cb0[6].wwww, r0.xyzx, r1.wwww
    r0.xyz = ((-(source[6].wwww))*(r0.xyzx)+(r1.wwww)).xyz;
    // 10: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 11: mad r0.xyz, cb0[7].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[7].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 12: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 13: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 14: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 15: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 16: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 17: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 18: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 19: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 20: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 21: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 22: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 23: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_w_pa_worldoffset_02_40_tr: 2cce8741b31d5049ab2b10e26293372d; selected map b62b4d84068ddcdad35cfacb576ce799a955f5824aac67e7740221f123d6be23.
float4 SDNative361(SD_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    // Same serialized CameraWorldPos / WorldToLocal binding as native341.
    // Absolute source-centimeter world precomposes camera-relative world + camera.
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[1]=g_SDSourceWorldToLocal[0];
    source[2]=g_SDSourceWorldToLocal[1];
    source[3]=g_SDSourceWorldToLocal[2];
    float4 output=0.f;
    source[4] = g_SDSourceMaterialParameters[8u];
    source[5] = SDNativeAppend(g_SDSourceMaterialParameters[5u].xxxx,g_SDSourceMaterialParameters[5u].yyyy,1u);
    source[6] = SDNativeAppend(cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = SDNativeAppend(sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = SDNativeAppend(g_SDSourceMaterialParameters[3u].zzzz,g_SDSourceMaterialParameters[3u].wwww,1u);
    source[9] = SDNativeAppend(g_SDSourceMaterialParameters[2u].xxxx,g_SDSourceMaterialParameters[2u].yyyy,1u);
    source[10].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[10].y = (cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].z = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[10].w = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[11].x = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[11].y = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[11].z = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[11].w = (g_SDSourceMaterialTime.xxxx).x;
    source[12].x = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_SDSourceMaterialParameters[7u].xxxx).x;
    source[12].z = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[12].w = (g_SDSourceMaterialParameters[7u].yyyy).x;
    source[13].x = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[13].y = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[13].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[14].x = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[14].y = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[14].z = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[15].x = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[15].y = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[15].z = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[15].w = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[16].x = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[16].y = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[16].z = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[16].w = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[17].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[17].y = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[17].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[17].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native camera-relative world + CameraWorldPos
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xyz, v7.xyzx, cb0[0].xyzx
    r0.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 2: mul r0.xyz, r0.xyzx, l(0.003906, 0.003906, 0.003906, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(asfloat(0x3b800000u),asfloat(0x3b800000u),asfloat(0x3b800000u),asfloat(0x00000000u)))).xyz;
    // 3: mul r0.yw, r0.yyyy, cb0[2].xxxy
    r0.yw = ((r0.yyyy)*(source[2].xxxy)).yw;
    // 4: mad r0.xy, cb0[1].xyxx, r0.xxxx, r0.ywyy
    r0.xy = ((source[1].xyxx)*(r0.xxxx)+(r0.ywyy)).xy;
    // 5: mad r0.xy, cb0[3].xyxx, r0.zzzz, r0.xyxx
    r0.xy = ((source[3].xyxx)*(r0.zzzz)+(r0.xyxx)).xy;
    // 6: mul r0.zw, r0.xxxy, cb0[14].zzzw
    r0.zw = ((r0.xxxy)*(source[14].zzzw)).zw;
    // 7: mad r1.x, cb0[11].w, cb0[14].y, r0.z
    r1.x = ((source[11].wwww)*(source[14].yyyy)+(r0.zzzz)).x;
    // 8: mad r1.y, cb0[11].w, cb0[15].x, r0.w
    r1.y = ((source[11].wwww)*(source[15].xxxx)+(r0.wwww)).y;
    // 9: add r0.zw, r1.xxxy, cb0[8].xxxy
    r0.zw = ((r1.xxxy)+(source[8].xxxy)).zw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s3, l(0.000000)
    r0.z = (SDNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 11: mad r1.xy, cb0[15].wwww, r0.zzzz, r0.xyxx
    r1.xy = ((source[15].wwww)*(r0.zzzz)+(r0.xyxx)).xy;
    // 12: mul r0.xy, r0.xyxx, cb0[12].xyxx
    r0.xy = ((r0.xyxx)*(source[12].xyxx)).xy;
    // 13: mul r0.w, r1.x, cb0[13].w
    r0.w = ((r1.xxxx)*(source[13].wwww)).w;
    // 14: mul r1.x, r1.y, cb0[14].x
    r1.x = ((r1.yyyy)*(source[14].xxxx)).x;
    // 15: mad r1.y, cb0[11].w, cb0[16].x, r1.x
    r1.y = ((source[11].wwww)*(source[16].xxxx)+(r1.xxxx)).y;
    // 16: mad r1.x, cb0[11].w, cb0[13].z, r0.w
    r1.x = ((source[11].wwww)*(source[13].zzzz)+(r0.wwww)).x;
    // 17: add r1.xy, r1.xyxx, cb0[9].xyxx
    r1.xy = ((r1.xyxx)+(source[9].xyxx)).xy;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t4.yzwx, s4, l(0.000000)
    r0.w = (SDNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 19: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 20: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 21: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 22: mul r0.w, r0.w, cb0[16].w
    r0.w = ((r0.wwww)*(source[16].wwww)).w;
    // 23: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 24: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 25: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 26: mul r0.w, r0.w, cb0[17].x
    r0.w = ((r0.wwww)*(source[17].xxxx)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: mul r0.w, r0.w, cb0[17].y
    r0.w = ((r0.wwww)*(source[17].yyyy)).w;
    // 29: lt r1.x, r0.z, l(0.000001)
    r1.x = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 30: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 31: mad r0.z, r0.z, cb0[17].z, r0.w
    r0.z = ((r0.zzzz)*(source[17].zzzz)+(r0.wwww)).z;
    // 32: mad r1.x, cb0[11].w, cb0[11].z, r0.x
    r1.x = ((source[11].wwww)*(source[11].zzzz)+(r0.xxxx)).x;
    // 33: mad r1.y, cb0[11].w, cb0[12].z, r0.y
    r1.y = ((source[11].wwww)*(source[12].zzzz)+(r0.yyyy)).y;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (SDNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 35: mad r1.xy, cb0[10].zwzz, v2.xyxx, cb0[5].xyxx
    r1.xy = ((source[10].zwzz)*(v2.xyxx)+(source[5].xyxx)).xy;
    // 36: mul r0.w, v4.x, cb0[12].w
    r0.w = ((v4.xxxx)*(source[12].wwww)).w;
    // 37: mad r1.xy, r0.wwww, r0.xyxx, r1.xyxx
    r1.xy = ((r0.wwww)*(r0.xyxx)+(r1.xyxx)).xy;
    // 38: mad r1.z, v4.y, l(0.500000), r1.y
    r1.z = ((v4.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r1.yyyy)).z;
    // 39: add r0.xy, r1.xzxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xzxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 40: dp2 r1.x, cb0[6].xyxx, r0.xyxx
    r1.x = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 41: dp2 r1.y, cb0[7].xyxx, r0.xyxx
    r1.y = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 42: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (SDNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (SDNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 45: add r0.y, r1.y, r1.x
    r0.y = ((r1.yyyy)+(r1.xxxx)).y;
    // 46: add r0.y, r1.z, r0.y
    r0.y = ((r1.zzzz)+(r0.yyyy)).y;
    // 47: mul r0.y, r0.y, l(0.333330)
    r0.y = ((r0.yyyy)*(float4(0.333330,0.333330,0.333330,0.333330))).y;
    // 48: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 49: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 50: mul r0.y, r0.y, cb0[13].x
    r0.y = ((r0.yyyy)*(source[13].xxxx)).y;
    // 51: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 52: mul r0.y, r0.y, cb0[13].y
    r0.y = ((r0.yyyy)*(source[13].yyyy)).y;
    // 53: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 54: movc r0.y, r0.w, l(0), r0.y
    r0.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 55: mad r0.y, r0.x, r0.z, r0.y
    r0.y = ((r0.xxxx)*(r0.zzzz)+(r0.yyyy)).y;
    // 56: mul r0.x, r0.x, cb0[17].w
    r0.x = ((r0.xxxx)*(source[17].wwww)).x;
    // 57: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 58: mad r0.yzw, r0.yyyy, v3.xxyz, cb0[4].xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)+(source[4].xxyz)).yzw;
    // 59: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 60: add r0.yz, -v2.xxyx, l(0.000000, 1.000000, 1.000000, 0.000000)
    r0.yz = ((-(v2.xxyx))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 61: mul r0.yz, r0.yyzy, v2.xxyx
    r0.yz = ((r0.yyzy)*(v2.xxyx)).yz;
    // 62: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 63: mul_sat r0.y, r0.y, l(50.000000)
    r0.y = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).y;
    // 64: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 65: mul o0.w, r0.x, cb0[0].w
    output.w = ((r0.xxxx)*(source[0].wwww)).w;
    return output;
}

// fx_w_me_master_06_00_tr: 4899a4f7a8a03d43a31b1d767bb9dae4; selected map 80f64e2f9ed4c0a78b4bf911f5c0602e149c5d5b06b7e11a6037393c539b8daf.
float4 SDNative362(SD_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[6u];
    source[3] = g_SDSourceMaterialParameters[4u];
    source[4] = g_SDSourceMaterialParameters[5u];
    source[5] = SDNativeAppend(g_SDSourceMaterialParameters[3u].yyyy,g_SDSourceMaterialParameters[3u].zzzz,1u);
    source[6].x = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[6].y = (g_SDSourceMaterialTime.xxxx).x;
    source[6].z = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[6].w = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[7].x = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[7].y = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[7].z = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[7].w = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[8].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[8].y = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[8].z = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[8].w = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[9].x = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[9].y = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[9].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[9].w = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[10].x = (g_SDSourceMaterialParameters[3u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[6].zwzz
    r0.xy = ((v4.xyxx)*(source[6].zwzz)).xy;
    // 2: mad r1.x, cb0[6].y, cb0[6].x, r0.x
    r1.x = ((source[6].yyyy)*(source[6].xxxx)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[6].y, cb0[7].x, r0.y
    r1.y = ((source[6].yyyy)*(source[7].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (SDNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: mul r0.zw, r0.xxxy, cb0[7].yyyy
    r0.zw = ((r0.xxxy)*(source[7].yyyy)).zw;
    // 7: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 8: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 9: mad r1.zw, r1.xxxy, l(0.000000, 0.000000, 12.000000, 12.000000), r0.zzzw
    r1.zw = ((r1.xxxy)*(float4(0.000000,0.000000,12.000000,12.000000))+(r0.zzzw)).zw;
    // 10: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), r0.zzzw
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(r0.zzzw)).zw;
    // 11: mul r1.xy, r1.xyxx, l(12.000000, 12.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(12.000000,12.000000,0.000000,0.000000))).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t4.xyzw, s1, l(0.000000)
    r2.xyz = (SDNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.zwzz, t5.xyzw, s2, l(0.000000)
    r3.xyz = (SDNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.zwzz, t4.xyzw, s1, l(0.000000)
    r1.xyz = (SDNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 15: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 16: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 17: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 18: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 19: mad r1.xyz, r1.xyzx, l(5.000000, 5.000000, 5.000000, 0.000000), r3.xyzx
    r1.xyz = ((r1.xyzx)*(float4(5.000000,5.000000,5.000000,0.000000))+(r3.xyzx)).xyz;
    // 20: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 21: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 22: mad r1.xyz, r2.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r1.xyzx)).xyz;
    // 23: mad r0.zw, cb0[7].yyyy, r0.xxxy, v4.xxxy
    r0.zw = ((source[7].yyyy)*(r0.xxxy)+(v4.xxxy)).zw;
    // 24: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 25: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 26: mul r2.xyz, r1.wwww, v6.xyzx
    r2.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 27: mad r2.xy, r2.xyxx, l(-1.525000, -1.525000, 0.000000, 0.000000), r0.zwzz
    r2.xy = ((r2.xyxx)*(float4(-1.525000,-1.525000,0.000000,0.000000))+(r0.zwzz)).xy;
    // 28: add r1.w, -|r2.z|, l(1.000000)
    r1.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: add r2.xy, r2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 30: dp2 r2.x, r2.xyxx, r2.xyxx
    r2.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 31: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 32: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 33: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 34: mul r2.y, r2.x, r2.x
    r2.y = ((r2.xxxx)*(r2.xxxx)).y;
    // 35: mul r2.y, r2.y, r2.x
    r2.y = ((r2.yyyy)*(r2.xxxx)).y;
    // 36: lt r2.x, r2.x, l(0.000001)
    r2.x = (asfloat((uint4)((r2.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 37: movc r2.x, r2.x, l(0), r2.y
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).x;
    // 38: mul r2.xyz, r1.xyzx, r2.xxxx
    r2.xyz = ((r1.xyzx)*(r2.xxxx)).xyz;
    // 39: mul r2.xyz, r2.xyzx, l(0.900000, 0.900000, 0.900000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(0.900000,0.900000,0.900000,0.000000))).xyz;
    // 40: mad r1.xyz, r1.xyzx, l(0.100000, 0.100000, 0.100000, 0.000000), r2.xyzx
    r1.xyz = ((r1.xyzx)*(float4(0.100000,0.100000,0.100000,0.000000))+(r2.xyzx)).xyz;
    // 41: mul r2.xyz, cb0[3].xyzx, cb0[3].wwww
    r2.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 42: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 43: mul r2.xy, v4.xyxx, cb0[5].xyxx
    r2.xy = ((v4.xyxx)*(source[5].xyxx)).xy;
    // 44: mad r0.xy, cb0[7].yyyy, r0.xyxx, |r2.xyxx|
    r0.xy = ((source[7].yyyy)*(r0.xyxx)+(abs(r2.xyxx))).xy;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyz = (SDNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 46: add r0.x, r2.y, r2.x
    r0.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 47: add r0.x, r2.z, r0.x
    r0.x = ((r2.zzzz)+(r0.xxxx)).x;
    // 48: mad r0.y, -r0.x, l(0.333330), l(1.000000)
    r0.y = ((-(r0.xxxx))*(float4(0.333330,0.333330,0.333330,0.333330))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: log r2.x, |r0.y|
    r2.x = (log2(abs(r0.yyyy))).x;
    // 50: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 51: mul r2.x, r2.x, l(17.000000)
    r2.x = ((r2.xxxx)*(float4(17.000000,17.000000,17.000000,17.000000))).x;
    // 52: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 53: movc r0.y, r0.y, l(0), r2.x
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).y;
    // 54: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 55: mad r2.xyz, r0.yyyy, r2.xyzx, -r1.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 56: mad r1.xyz, r0.yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 57: mad r1.xyz, r1.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 58: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 59: mul r1.xy, r0.zwzz, cb0[8].yzyy
    r1.xy = ((r0.zwzz)*(source[8].yzyy)).xy;
    // 60: mul r0.yz, r0.zzwz, cb0[9].yyzy
    r0.yz = ((r0.zzwz)*(source[9].yyzy)).yz;
    // 61: mad r0.yz, cb0[6].yyyy, cb0[9].xxwx, r0.yyzy
    r0.yz = ((source[6].yyyy)*(source[9].xxwx)+(r0.yyzy)).yz;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t2.wxyz, s5, l(0.000000)
    r0.yzw = (SDNativeSample5((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 63: mad r1.xy, cb0[6].yyyy, cb0[8].xwxx, r1.xyxx
    r1.xy = ((source[6].yyyy)*(source[8].xwxx)+(r1.xyxx)).xy;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s4, l(0.000000)
    r1.xyz = (SDNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 65: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 66: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 67: add r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)+(r0.yyyy)).y;
    // 68: mul r0.z, r0.y, l(0.333330)
    r0.z = ((r0.yyyy)*(float4(0.333330,0.333330,0.333330,0.333330))).z;
    // 69: mad r0.y, r0.y, l(0.333330), l(1.000000)
    r0.y = ((r0.yyyy)*(float4(0.333330,0.333330,0.333330,0.333330))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 70: dp2 r0.z, r0.zzzz, r0.zzzz
    r0.z = (dot((r0.zzzz).xy,(r0.zzzz).xy).xxxx).z;
    // 71: mad r0.z, r0.x, l(0.333330), r0.z
    r0.z = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))+(r0.zzzz)).z;
    // 72: mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 73: mad r0.x, r0.z, r0.x, -cb0[10].x
    r0.x = ((r0.zzzz)*(r0.xxxx)+(-(source[10].xxxx))).x;
    // 74: mad r0.x, -r1.w, r1.w, r0.x
    r0.x = ((-(r1.wwww))*(r1.wwww)+(r0.xxxx)).x;
    // 75: mul_sat r0.x, r0.x, l(200.000000)
    r0.x = (saturate((r0.xxxx)*(float4(200.000000,200.000000,200.000000,200.000000)))).x;
    // 76: mov_sat r0.z, cb0[1].w
    r0.z = (saturate(source[1].wwww)).z;
    // 77: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 78: mad r0.x, -r0.z, r0.y, r0.x
    r0.x = ((-(r0.zzzz))*(r0.yyyy)+(r0.xxxx)).x;
    // 79: mul_sat r0.x, r0.x, l(50.000000)
    r0.x = (saturate((r0.xxxx)*(float4(50.000000,50.000000,50.000000,50.000000)))).x;
    // 80: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

// fx_w_me_flowtrail_03_09_ts_ad: 746a6ce85a57a349baf52511395151c0; selected map 2e7043978d63f784df52fe9eef7a5133a216e629048b345083c252013a4841aa.
float4 SDNative363(SD_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[9u];
    source[3] = g_SDSourceMaterialParameters[6u];
    source[4] = g_SDSourceMaterialParameters[7u];
    source[5] = SDNativeAppend(g_SDSourceMaterialParameters[1u].yyyy,g_SDSourceMaterialParameters[1u].zzzz,1u);
    source[6] = SDNativeAppend(g_SDSourceMaterialParameters[2u].zzzz,g_SDSourceMaterialParameters[3u].xxxx,1u);
    source[7] = SDNativeAppend((g_SDSourceMaterialParameters[2u].yyyy*g_SDSourceMaterialTime.xxxx),(g_SDSourceMaterialParameters[2u].wwww*g_SDSourceMaterialTime.xxxx),1u);
    source[8] = input.dynamicParameter;
    source[9] = SDNativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_SDSourceMaterialParameters[4u].wwww*g_SDSourceMaterialTime.xxxx),1u);
    source[10] = SDNativeAppend(cos((g_SDSourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = SDNativeAppend(sin((g_SDSourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12] = SDNativeAppend(g_SDSourceMaterialParameters[4u].xxxx,g_SDSourceMaterialParameters[4u].yyyy,1u);
    source[13] = SDNativeAppend(cos((g_SDSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[14] = SDNativeAppend(sin((g_SDSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[15].x = ((g_SDSourceMaterialParameters[2u].wwww*g_SDSourceMaterialTime.xxxx)).x;
    source[15].y = ((g_SDSourceMaterialParameters[2u].yyyy*g_SDSourceMaterialTime.xxxx)).x;
    source[15].z = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[15].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[16].x = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[16].y = ((g_SDSourceMaterialParameters[4u].wwww*g_SDSourceMaterialTime.xxxx)).x;
    source[16].z = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[16].w = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[17].x = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[17].y = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[17].z = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[17].w = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[18].x = (cos((g_SDSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].y = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[18].z = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[18].w = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[19].x = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[19].y = (g_SDSourceMaterialParameters[1u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.x, v4.y, cb0[9].y
    r0.x = ((v4.yyyy)+(source[9].yyyy)).x;
    // 2: mul r0.x, r0.x, cb0[16].z
    r0.x = ((r0.xxxx)*(source[16].zzzz)).x;
    // 3: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(asfloat(0x40c90fdbu),asfloat(0x40c90fdbu),asfloat(0x40c90fdbu),asfloat(0x40c90fdbu)))).x;
    // 4: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 5: mul r0.y, cb0[8].z, cb0[16].w
    r0.y = ((source[8].zzzz)*(source[16].wwww)).y;
    // 6: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 7: add r0.y, -v4.y, l(1.000000)
    r0.y = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 8: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 9: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 10: add r0.y, v4.x, l(-0.500000)
    r0.y = ((v4.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 11: mad r0.x, r0.x, l(4.000000), r0.y
    r0.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))+(r0.yyyy)).x;
    // 12: mad r0.zw, v4.xxxy, cb0[6].xxxy, cb0[7].xxxy
    r0.zw = ((v4.xxxy)*(source[6].xxxy)+(source[7].xxxy)).zw;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t2.zwxy, s0, l(0.000000)
    r0.zw = (SDNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 14: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 15: mad r1.xy, v4.xyxx, cb0[5].xyxx, l(0.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)*(source[5].xyxx)+(float4(0.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: mad r0.zw, cb0[15].zzzz, r0.zzzw, r1.xxxy
    r0.zw = ((source[15].zzzz)*(r0.zzzw)+(r1.xxxy)).zw;
    // 17: add r1.x, r0.z, cb0[15].w
    r1.x = ((r0.zzzz)+(source[15].wwww)).x;
    // 18: add r1.y, r0.w, cb0[8].x
    r1.y = ((r0.wwww)+(source[8].xxxx)).y;
    // 19: mov r0.y, v4.y
    r0.y = (v4.yyyy).y;
    // 20: mad r0.xy, cb0[17].xxxx, r0.xyxx, r1.xyxx
    r0.xy = ((source[17].xxxx)*(r0.xyxx)+(r1.xyxx)).xy;
    // 21: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 22: dp2 r1.x, cb0[10].xyxx, r0.xyxx
    r1.x = (dot((source[10].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: dp2 r1.y, cb0[11].xyxx, r0.xyxx
    r1.y = (dot((source[11].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 24: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = (SDNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 26: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 27: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 28: mad r0.xyz, cb0[17].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[17].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 29: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 30: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 31: mul r0.xyz, r0.xyzx, cb0[17].zzzz
    r0.xyz = ((r0.xyzx)*(source[17].zzzz)).xyz;
    // 32: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 33: mul r0.xyz, r0.xyzx, cb0[17].wwww
    r0.xyz = ((r0.xyzx)*(source[17].wwww)).xyz;
    // 34: add r1.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r1.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 35: mad r1.xyz, v4.yyyy, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((v4.yyyy)*(r1.xyzx)+(source[3].xyzx)).xyz;
    // 36: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 37: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 38: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 39: mul r0.w, v4.x, cb0[12].x
    r0.w = ((v4.xxxx)*(source[12].xxxx)).w;
    // 40: add r1.x, r0.w, cb0[18].w
    r1.x = ((r0.wwww)+(source[18].wwww)).x;
    // 41: mad r1.z, v4.y, cb0[12].y, cb0[8].y
    r1.z = ((v4.yyyy)*(source[12].yyyy)+(source[8].yyyy)).z;
    // 42: add r1.xy, r1.xzxx, l(-0.500000, -1.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xzxx)+(float4(-0.500000,-1.500000,0.000000,0.000000))).xy;
    // 43: dp2 r2.x, cb0[13].xyxx, r1.xyxx
    r2.x = (dot((source[13].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 44: dp2 r2.y, cb0[14].xyxx, r1.xyxx
    r2.y = (dot((source[14].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 45: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s3, l(0.000000)
    r0.w = (SDNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v4.xyxx, t0.xyzw, s2, l(0.000000)
    r1.x = (SDNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 48: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 49: mul_sat r0.w, r0.w, cb0[19].x
    r0.w = (saturate((r0.wwww)*(source[19].xxxx))).w;
    // 50: mul r0.w, r0.w, cb0[1].w
    r0.w = ((r0.wwww)*(source[1].wwww)).w;
    // 51: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 52: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 53: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_w_pa_spritewave_01_106_tr: 9497e2284e9ad44998098c0a57d12c11; selected map 59850b7176e6a88744c508004a5973759a03b88082a7d84d85dc8a1707430a23.
float4 SDNative364(SD_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[9u];
    source[2] = SDNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = SDNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = SDNativeAppend(g_SDSourceMaterialParameters[4u].yyyy,g_SDSourceMaterialParameters[4u].zzzz,1u);
    source[5] = SDNativeAppend(cos(((g_SDSourceMaterialParameters[5u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[5u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = SDNativeAppend(sin(((g_SDSourceMaterialParameters[5u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_SDSourceMaterialParameters[5u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = SDNativeAppend(cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = SDNativeAppend(sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = g_SDSourceMaterialParameters[8u];
    source[10].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[5u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[10].y = (cos(((g_SDSourceMaterialParameters[5u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].z = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[10].w = (g_SDSourceMaterialTime.xxxx).x;
    source[11].x = (g_SDSourceMaterialParameters[5u].zzzz).x;
    source[11].y = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[11].z = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[11].w = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[12].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[12].z = (g_SDSourceMaterialParameters[7u].xxxx).x;
    source[12].w = (g_SDSourceMaterialParameters[7u].zzzz).x;
    source[13].x = (g_SDSourceMaterialParameters[7u].wwww).x;
    source[13].y = (g_SDSourceMaterialParameters[7u].yyyy).x;
    source[13].z = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[13].w = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[14].x = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[14].y = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[14].z = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[14].w = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[15].x = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[15].y = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[15].z = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[15].w = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[16].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[16].y = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[16].z = ((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[16].w = (sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[17].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[17].y = (cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[17].z = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[17].w = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[18].x = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[18].y = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[18].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[18].w = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[19].x = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[19].y = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[19].z = (g_SDSourceMaterialParameters[3u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mul r0.x, cb0[10].w, cb0[12].z
    r0.x = ((source[10].wwww)*(source[12].zzzz)).x;
    // 2: mad r0.x, cb0[12].w, v2.x, r0.x
    r0.x = ((source[12].wwww)*(v2.xxxx)+(r0.xxxx)).x;
    // 3: mul r0.z, v2.y, cb0[13].x
    r0.z = ((v2.yyyy)*(source[13].xxxx)).z;
    // 4: mad r0.y, cb0[10].w, cb0[13].y, r0.z
    r0.y = ((source[10].wwww)*(source[13].yyyy)+(r0.zzzz)).y;
    // 5: mad r0.xy, v4.wwww, cb0[13].zwzz, r0.xyxx
    r0.xy = ((v4.wwww)*(source[13].zwzz)+(r0.xyxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (SDNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: add r0.y, v4.z, cb0[14].x
    r0.y = ((v4.zzzz)+(source[14].xxxx)).y;
    // 8: mad r0.xy, r0.xxxx, r0.yyyy, cb0[4].xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(source[4].xyxx)).xy;
    // 9: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 10: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 11: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 12: mad r1.xy, cb0[11].zwzz, v4.xxxx, r1.xyxx
    r1.xy = ((source[11].zwzz)*(v4.xxxx)+(r1.xyxx)).xy;
    // 13: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 14: mul r1.xy, r1.xyxx, cb0[11].xyxx
    r1.xy = ((r1.xyxx)*(source[11].xyxx)).xy;
    // 15: mad r2.x, cb0[10].w, cb0[10].z, r1.x
    r2.x = ((source[10].wwww)*(source[10].zzzz)+(r1.xxxx)).x;
    // 16: mad r2.y, cb0[10].w, cb0[12].y, r1.y
    r2.y = ((source[10].wwww)*(source[12].yyyy)+(r1.yyyy)).y;
    // 17: add r0.xy, r0.xyxx, r2.xyxx
    r0.xy = ((r0.xyxx)+(r2.xyxx)).xy;
    // 18: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 19: dp2 r1.x, cb0[5].xyxx, r0.xyxx
    r1.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 20: dp2 r1.y, cb0[6].xyxx, r0.xyxx
    r1.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 21: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 22: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s0, l(-1.000000)
    r0.x = (SDNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 23: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 24: mul r0.y, r0.y, cb0[14].w
    r0.y = ((r0.yyyy)*(source[14].wwww)).y;
    // 25: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 26: mul r0.y, r0.y, cb0[15].x
    r0.y = ((r0.yyyy)*(source[15].xxxx)).y;
    // 27: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 28: movc r0.y, r1.x, l(0), r0.y
    r0.y = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 29: mad r0.y, r0.x, cb0[15].y, r0.y
    r0.y = ((r0.xxxx)*(source[15].yyyy)+(r0.yyyy)).y;
    // 30: dp2 r1.x, cb0[7].xyxx, r0.zwzz
    r1.x = (dot((source[7].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 31: dp2 r1.y, cb0[8].xyxx, r0.zwzz
    r1.y = (dot((source[8].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 32: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 33: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 34: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 35: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 36: mul r0.z, r0.z, cb0[18].w
    r0.z = ((r0.zzzz)*(source[18].wwww)).z;
    // 37: max r0.z, r0.z, cb0[19].y
    r0.z = (max(r0.zzzz,source[19].yyyy)).z;
    // 38: min r0.z, r0.z, cb0[19].x
    r0.z = (min(r0.zzzz,source[19].xxxx)).z;
    // 39: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 40: mul r0.w, r1.x, cb0[15].w
    r0.w = ((r1.xxxx)*(source[15].wwww)).w;
    // 41: mul r1.x, r1.y, cb0[16].x
    r1.x = ((r1.yyyy)*(source[16].xxxx)).x;
    // 42: mad r1.x, cb0[10].w, cb0[17].z, r1.x
    r1.x = ((source[10].wwww)*(source[17].zzzz)+(r1.xxxx)).x;
    // 43: add r1.y, r1.x, cb0[18].x
    r1.y = ((r1.xxxx)+(source[18].xxxx)).y;
    // 44: mad r0.w, cb0[10].w, cb0[15].z, r0.w
    r0.w = ((source[10].wwww)*(source[15].zzzz)+(r0.wwww)).w;
    // 45: add r1.x, r0.w, cb0[17].w
    r1.x = ((r0.wwww)+(source[17].wwww)).x;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s2, l(0.000000)
    r0.w = (SDNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 47: add r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 48: add r1.x, v4.y, l(-1.000000)
    r1.x = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 49: add_sat r0.w, r0.w, -r1.x
    r0.w = (saturate((r0.wwww)+(-(r1.xxxx)))).w;
    // 50: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 51: mul r1.x, r1.x, cb0[18].z
    r1.x = ((r1.xxxx)*(source[18].zzzz)).x;
    // 52: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 53: mul_sat r1.x, r1.x, cb0[18].y
    r1.x = (saturate((r1.xxxx)*(source[18].yyyy))).x;
    // 54: lt r1.y, r0.w, l(0.000001)
    r1.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 55: mul r0.w, r0.w, cb0[18].y
    r0.w = ((r0.wwww)*(source[18].yyyy)).w;
    // 56: movc r1.x, r1.y, l(-0.000000), -r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r1.xxxx))).x;
    // 57: mov_sat r1.y, r0.w
    r1.y = (saturate(r0.wwww)).y;
    // 58: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 59: mul r0.x, r0.x, cb0[19].z
    r0.x = ((r0.xxxx)*(source[19].zzzz)).x;
    // 60: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 61: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 62: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 63: add r0.x, r1.x, r1.y
    r0.x = ((r1.xxxx)+(r1.yyyy)).x;
    // 64: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 65: mad r0.xyz, r0.xxxx, cb0[9].xyzx, r0.yyyy
    r0.xyz = ((r0.xxxx)*(source[9].xyzx)+(r0.yyyy)).xyz;
    // 66: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 67: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_j_me_splitline_01_1_ad: 97638bedbf7a4b49ab33a1ee2c8a688b; selected map e26d460f95b0214c7c0301599fbe2a21161ff06bf3013bbbb31782640e256f02.
float4 SDNative365(SD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[1u];
    source[3] = input.dynamicParameter;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 4: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 7: mul r0.w, r0.y, r1.x
    r0.w = ((r0.yyyy)*(r1.xxxx)).w;
    // 8: mad r0.w, r0.x, r1.y, -r0.w
    r0.w = ((r0.xxxx)*(r1.yyyy)+(-(r0.wwww))).w;
    // 9: mul r0.y, r0.w, v1.w
    r0.y = ((r0.wwww)*(v1.wwww)).y;
    // 10: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul_sat r0.w, r0.w, v6.z
    r0.w = (saturate((r0.wwww)*(v6.zzzz))).w;
    // 13: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 14: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 15: source device depth mapped to centimetre view depth; reconstruction at 17.
    r1.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 17-20: reconstructed view depth is supplied by the runtime adapter.
    r1.w = r1.w;
    // 21: add r1.w, r1.w, -v7.w
    r1.w = ((r1.wwww)+(-(v7.wwww))).w;
    // 22: mul_sat r1.w, r1.w, l(0.036364)
    r1.w = (saturate((r1.wwww)*(float4(asfloat(0x3d14f209u),asfloat(0x3d14f209u),asfloat(0x3d14f209u),asfloat(0x3d14f209u))))).w;
    // 23: mul r2.x, r0.w, v4.y
    r2.x = ((r0.wwww)*(v4.yyyy)).x;
    // 24: mul r2.x, r2.x, cb0[1].w
    r2.x = ((r2.xxxx)*(source[1].wwww)).x;
    // 25: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 26: mul o0.w, r1.w, cb0[0].x
    output.w = ((r1.wwww)*(source[0].xxxx)).w;
    // 27: add r1.w, v4.x, l(-0.500000)
    r1.w = ((v4.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 28: add r2.x, |r1.w|, |r1.w|
    r2.x = ((abs(r1.wwww))+(abs(r1.wwww))).x;
    // 29: lt r1.w, |r1.w|, l(0.000000)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu)))) * 0xffffffffu)).w;
    // 30: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 31: mul r2.x, r2.x, l(10.000000)
    r2.x = ((r2.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 32: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 33: mul r2.x, r2.x, l(0.035000)
    r2.x = ((r2.xxxx)*(float4(0.035000,0.035000,0.035000,0.035000))).x;
    // 34: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 35: mad r1.w, cb0[3].x, l(0.010000), r1.w
    r1.w = ((source[3].xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))+(r1.wwww)).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.xyz = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 37: add r3.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r3.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 38: mul r3.xy, r3.xyxx, l(0.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(0.000000,1.000000,0.000000,0.000000))).xy;
    // 39: mul r1.w, r1.w, l(0.100000)
    r1.w = ((r1.wwww)*(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 40: mov r3.zw, r1.yyyx
    r3.zw = (r1.yyyx).zw;
    // 41: mov r4.x, r2.x
    r4.x = (r2.xxxx).x;
    // 42: mov r4.y, l(0)
    r4.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 43: loop
    [loop] while (true) {
    // 44: ge r2.w, r4.y, l(3.000000)
    r2.w = (asfloat((uint4)((r4.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 45: breakc_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) break;
    // 46: mad r3.zw, -r3.yyyx, r1.wwww, r3.zzzw
    r3.zw = ((-(r3.yyyx))*(r1.wwww)+(r3.zzzw)).zw;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.wzww, t1.yzwx, s1, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.wzww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzwx).w;
    // 48: add r4.x, r2.w, r4.x
    r4.x = ((r2.wwww)+(r4.xxxx)).x;
    // 49: add r4.y, r4.y, l(1.000000)
    r4.y = ((r4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 50: endloop
    }
    // 51: mov r2.w, r1.x
    r2.w = (r1.xxxx).w;
    // 52: mov r2.x, r3.z
    r2.x = (r3.zzzz).x;
    // 53: mov r5.x, r2.y
    r5.x = (r2.yyyy).x;
    // 54: mov r5.y, l(0)
    r5.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 55: loop
    [loop] while (true) {
    // 56: ge r1.y, r5.y, l(3.000000)
    r1.y = (asfloat((uint4)((r5.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).y;
    // 57: breakc_nz r1.y
    if ((asuint(r1.yyyy)).x != 0u) break;
    // 58: mad r2.xw, -r3.yyyx, r1.wwww, r2.xxxw
    r2.xw = ((-(r3.yyyx))*(r1.wwww)+(r2.xxxw)).xw;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.wxww, t1.xyzw, s1, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.wxww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).y;
    // 60: add r5.x, r1.y, r5.x
    r5.x = ((r1.yyyy)+(r5.xxxx)).x;
    // 61: add r5.y, r5.y, l(1.000000)
    r5.y = ((r5.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 62: endloop
    }
    // 63: mov r4.y, r5.x
    r4.y = (r5.xxxx).y;
    // 64: mov r5.x, r1.x
    r5.x = (r1.xxxx).x;
    // 65: mov r5.y, r2.x
    r5.y = (r2.xxxx).y;
    // 66: mov r6.x, r2.z
    r6.x = (r2.zzzz).x;
    // 67: mov r6.y, l(0)
    r6.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 68: loop
    [loop] while (true) {
    // 69: ge r1.y, r6.y, l(3.000000)
    r1.y = (asfloat((uint4)((r6.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).y;
    // 70: breakc_nz r1.y
    if ((asuint(r1.yyyy)).x != 0u) break;
    // 71: mad r5.xy, -r3.xyxx, r1.wwww, r5.xyxx
    r5.xy = ((-(r3.xyxx))*(r1.wwww)+(r5.xyxx)).xy;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r5.xyxx, t1.xzyw, s1, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).y;
    // 73: add r6.x, r1.y, r6.x
    r6.x = ((r1.yyyy)+(r6.xxxx)).x;
    // 74: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 75: endloop
    }
    // 76: mov r4.z, r6.x
    r4.z = (r6.xxxx).z;
    // 77: mul r1.xyw, r4.xyxz, l(0.250000, 0.250000, 0.000000, 0.250000)
    r1.xyw = ((r4.xyxz)*(float4(0.250000,0.250000,0.000000,0.250000))).xyw;
    // 78: dp3 r2.x, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 79: mad r2.xyz, -r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r2.xxxx
    r2.xyz = ((-(r4.xyzx))*(float4(0.250000,0.250000,0.250000,0.000000))+(r2.xxxx)).xyz;
    // 80: mad r1.xyw, cb0[3].yyyy, r2.xyxz, r1.xyxw
    r1.xyw = ((source[3].yyyy)*(r2.xyxz)+(r1.xyxw)).xyw;
    // 81: lt r2.x, r0.w, l(0.000001)
    r2.x = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 82: mul r2.y, r0.w, r0.w
    r2.y = ((r0.wwww)*(r0.wwww)).y;
    // 83: mul r2.y, r0.w, r2.y
    r2.y = ((r0.wwww)*(r2.yyyy)).y;
    // 84: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: lt r2.z, r0.w, l(0.000001)
    r2.z = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 86: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 87: mul r0.w, r0.w, r2.y
    r0.w = ((r0.wwww)*(r2.yyyy)).w;
    // 88: or r2.x, r2.x, r2.z
    r2.x = (asfloat(asuint(r2.xxxx) | asuint(r2.zzzz))).x;
    // 89: movc r0.w, r2.x, l(0), r0.w
    r0.w = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 90: add r1.xyw, r0.wwww, r1.xyxw
    r1.xyw = ((r0.wwww)+(r1.xyxw)).xyw;
    // 91: mad r1.xyw, cb0[1].xyxz, r1.xyxw, cb0[2].xyxz
    r1.xyw = ((source[1].xyxz)*(r1.xyxw)+(source[2].xyxz)).xyw;
    // 92: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_o_me_master_01_04_ds_ad: 7c31909c9df918499fdae3c98350c9f0; selected map e6b94a707b8b7967de27c17af2eb32747eccd5231bef403de3ffc34c35a9a3ac.
float4 SDNative366(SD_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[5u];
    source[3] = input.dynamicParameter;
    source[4] = SDNativeAppend(g_SDSourceMaterialParameters[1u].yyyy,g_SDSourceMaterialParameters[1u].zzzz,1u);
    source[5].x = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[5].y = (g_SDSourceMaterialTime.xxxx).x;
    source[5].z = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[7].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[1u].wwww)).x;
    source[7].y = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[7].z = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[7].w = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[8].x = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[8].y = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[8].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_SDSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.x, cb0[3].y, l(-1.000000)
    r0.x = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mad r0.y, cb0[5].y, cb0[6].w, cb0[7].x
    r0.y = ((source[5].yyyy)*(source[6].wwww)+(source[7].xxxx)).y;
    // 3: sincos r1.x, r2.x, r0.y
    r1.x = (sin(r0.yyyy)).x; r2.x = (cos(r0.yyyy)).x;
    // 4: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 5: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 6: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 7: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 8: dp2 r0.w, r3.yxyy, r0.yzyy
    r0.w = (dot((r3.yxyy).xy,(r0.yzyy).xy).xxxx).w;
    // 9: dp2 r0.y, r3.zyzz, r0.yzyy
    r0.y = (dot((r3.zyzz).xy,(r0.yzyy).xy).xxxx).y;
    // 10: mad r0.z, r0.y, cb0[4].y, r0.x
    r0.z = ((r0.yyyy)*(source[4].yyyy)+(r0.xxxx)).z;
    // 11: mul r0.x, r0.w, cb0[4].x
    r0.x = ((r0.wwww)*(source[4].xxxx)).x;
    // 12: add r0.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.zxyw, s1, l(0.000000)
    r0.x = (SDNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 14: mul r0.y, v4.x, cb0[5].w
    r0.y = ((v4.xxxx)*(source[5].wwww)).y;
    // 15: mul r0.z, cb0[5].x, cb0[5].y
    r0.z = ((source[5].xxxx)*(source[5].yyyy)).z;
    // 16: mad r1.x, r0.z, cb0[5].z, r0.y
    r1.x = ((r0.zzzz)*(source[5].zzzz)+(r0.yyyy)).x;
    // 17: mul r0.y, v4.y, cb0[6].x
    r0.y = ((v4.yyyy)*(source[6].xxxx)).y;
    // 18: mad r1.y, r0.z, cb0[6].y, r0.y
    r1.y = ((r0.zzzz)*(source[6].yyyy)+(r0.yyyy)).y;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.xzyw, s0, l(0.000000)
    r0.y = (SDNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 20: add r0.z, -cb0[3].x, l(1.000000)
    r0.z = ((-(source[3].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 21: mad r0.x, r0.y, r0.x, -r0.z
    r0.x = ((r0.yyyy)*(r0.xxxx)+(-(r0.zzzz))).x;
    // 22: mul_sat r0.x, r0.x, cb0[7].w
    r0.x = (saturate((r0.xxxx)*(source[7].wwww))).x;
    // 23: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 24: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 25: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 26: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 27: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 28: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 29: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 30: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 31: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 32: mul r0.w, r0.w, cb0[8].y
    r0.w = ((r0.wwww)*(source[8].yyyy)).w;
    // 33: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 34: mul_sat r0.w, r0.w, cb0[8].z
    r0.w = (saturate((r0.wwww)*(source[8].zzzz))).w;
    // 35: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 36: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 37: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 38: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 39: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 40: add r0.yzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((source[1].xxyz)+(source[2].xxyz)).yzw;
    // 41: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 42: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 43: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_w_me_master_01_3_ad: 1be5975fb69ff54db1af2725a42ee67a; selected map b8a26c9ecdca75a3ea4acdf868bbb1ced2752962de7e6db00c5469af1039a328.
float4 SDNative367(SD_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[8u];
    source[3] = g_SDSourceMaterialParameters[6u];
    source[4] = input.dynamicParameter;
    source[5] = SDNativeAppend(g_SDSourceMaterialParameters[3u].yyyy,g_SDSourceMaterialParameters[3u].zzzz,1u);
    source[6].x = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[6].y = (g_SDSourceMaterialTime.xxxx).x;
    source[6].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[7].y = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[7].w = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[8].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[8].y = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[8].z = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[8].w = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[9].x = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[9].y = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[9].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[9].w = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[10].x = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[10].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[3u].wwww)).x;
    source[10].w = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[11].x = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[11].y = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[11].z = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[11].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
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
    // 10: add r0.y, -cb0[11].w, l(1.000000)
    r0.y = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: add r0.y, cb0[4].y, l(-1.000000)
    r0.y = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 14: mad r0.z, cb0[6].y, cb0[10].y, cb0[10].z
    r0.z = ((source[6].yyyy)*(source[10].yyyy)+(source[10].zzzz)).z;
    // 15: sincos r1.x, r2.x, r0.z
    r1.x = (sin(r0.zzzz)).x; r2.x = (cos(r0.zzzz)).x;
    // 16: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 17: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 18: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 19: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 20: dp2 r1.x, r3.zyzz, r0.zwzz
    r1.x = (dot((r3.zyzz).xy,(r0.zwzz).xy).xxxx).x;
    // 21: dp2 r0.z, r3.yxyy, r0.zwzz
    r0.z = (dot((r3.yxyy).xy,(r0.zwzz).xy).xxxx).z;
    // 22: mad r2.x, r0.z, cb0[5].x, r0.y
    r2.x = ((r0.zzzz)*(source[5].xxxx)+(r0.yyyy)).x;
    // 23: mul r2.z, r1.x, cb0[5].y
    r2.z = ((r1.xxxx)*(source[5].yyyy)).z;
    // 24: add r0.yz, r2.xxzx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxzx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s4, l(0.000000)
    r0.y = (SDNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 26: mul r0.zw, v4.xxxy, cb0[9].yyyz
    r0.zw = ((v4.xxxy)*(source[9].yyyz)).zw;
    // 27: mul r1.x, cb0[6].x, cb0[6].y
    r1.x = ((source[6].xxxx)*(source[6].yyyy)).x;
    // 28: mad r0.zw, r1.xxxx, cb0[9].xxxw, r0.zzzw
    r0.zw = ((r1.xxxx)*(source[9].xxxw)+(r0.zzzw)).zw;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s3, l(0.000000)
    r0.z = (SDNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 30: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 31: add r0.z, -cb0[4].x, l(1.000000)
    r0.z = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 32: mul r1.yz, v4.xxyx, cb0[8].yyzy
    r1.yz = ((v4.xxyx)*(source[8].yyzy)).yz;
    // 33: mad r1.yz, r1.xxxx, cb0[8].xxwx, r1.yyzy
    r1.yz = ((r1.xxxx)*(source[8].xxwx)+(r1.yyzy)).yz;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.yzyy, t0.yzwx, s2, l(0.000000)
    r0.w = (SDNativeSample1((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 35: mad r0.y, r0.w, r0.y, -r0.z
    r0.y = ((r0.wwww)*(r0.yyyy)+(-(r0.zzzz))).y;
    // 36: mul_sat r0.y, r0.y, cb0[11].y
    r0.y = (saturate((r0.yyyy)*(source[11].yyyy))).y;
    // 37: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 38: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 39: mul r0.z, r0.z, cb0[11].z
    r0.z = ((r0.zzzz)*(source[11].zzzz)).z;
    // 40: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 41: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 42: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 43: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 44: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 45: mul r0.y, v4.x, cb0[6].w
    r0.y = ((v4.xxxx)*(source[6].wwww)).y;
    // 46: mad r2.x, r1.x, cb0[6].z, r0.y
    r2.x = ((r1.xxxx)*(source[6].zzzz)+(r0.yyyy)).x;
    // 47: mul r0.y, v4.y, cb0[7].x
    r0.y = ((v4.yyyy)*(source[7].xxxx)).y;
    // 48: mad r2.y, r1.x, cb0[7].y, r0.y
    r2.y = ((r1.xxxx)*(source[7].yyyy)+(r0.yyyy)).y;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r2.xyxx, t4.wxyz, s1, l(0.000000)
    r0.yzw = (SDNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 50: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 51: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 52: mad r0.yzw, cb0[7].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[7].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 53: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 54: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 55: mul r0.yzw, r0.yyzw, cb0[7].wwww
    r0.yzw = ((r0.yyzw)*(source[7].wwww)).yzw;
    // 56: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 57: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 58: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 59: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 60: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 61: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 62: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_d_pa_shine_01_10_ad: 7fb294803e1ea947bdf338f6e540a413; selected map 47f79a44bf04aa419bfb470eb8b02c2c1ffbbf10c34be2ed3c14eeef7d497a2c.
float4 SDNative368(SD_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[5u];
    source[2].x = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[2].y = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[2].z = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[2].w = (g_SDSourceMaterialTime.xxxx).x;
    source[3].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[3].y = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[3].z = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[3].w = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[4].x = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[4].y = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[4].z = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[5].x = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[5].y = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[5].w = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[6].x = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[6].y = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[6].z = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mul r0.x, v2.x, cb0[3].w
    r0.x = ((v2.xxxx)*(source[3].wwww)).x;
    // 2: mad r0.x, cb0[2].w, cb0[3].z, r0.x
    r0.x = ((source[2].wwww)*(source[3].zzzz)+(r0.xxxx)).x;
    // 3: mul r0.z, cb0[2].w, cb0[4].y
    r0.z = ((source[2].wwww)*(source[4].yyyy)).z;
    // 4: mad r0.y, cb0[4].x, v2.y, r0.z
    r0.y = ((source[4].xxxx)*(v2.yyyy)+(r0.zzzz)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (SDNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[4].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[4].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 7: mul r0.zw, r0.xxxy, cb0[3].xxxy
    r0.zw = ((r0.xxxy)*(source[3].xxxy)).zw;
    // 8: mul r0.xy, r0.xyxx, cb0[5].yzyy
    r0.xy = ((r0.xyxx)*(source[5].yzyy)).xy;
    // 9: mad r0.xy, cb0[2].wwww, cb0[5].xwxx, r0.xyxx
    r0.xy = ((source[2].wwww)*(source[5].xwxx)+(r0.xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (SDNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: mad r1.x, cb0[2].w, cb0[2].z, r0.z
    r1.x = ((source[2].wwww)*(source[2].zzzz)+(r0.zzzz)).x;
    // 12: mad r1.y, cb0[2].w, cb0[4].w, r0.w
    r1.y = ((source[2].wwww)*(source[4].wwww)+(r0.wwww)).y;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t1.yxzw, s1, l(0.000000)
    r0.y = (SDNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 14: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 15: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 16: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 17: mul r0.y, r0.y, cb0[6].x
    r0.y = ((r0.yyyy)*(source[6].xxxx)).y;
    // 18: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 19: mul r0.y, r0.y, cb0[6].y
    r0.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 20: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 21: add r0.y, v2.x, l(-0.500000)
    r0.y = ((v2.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 22: add r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)+(r0.yyyy)).y;
    // 23: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 24: mul r0.z, r0.y, v2.y
    r0.z = ((r0.yyyy)*(v2.yyyy)).z;
    // 25: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 26: mad r0.y, r0.y, l(0.500000), r0.z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.zzzz)).y;
    // 27: sqrt r0.z, v2.y
    r0.z = (sqrt(v2.yyyy)).z;
    // 28: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 29: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 30: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 31: mul r0.z, r0.z, cb0[2].x
    r0.z = ((r0.zzzz)*(source[2].xxxx)).z;
    // 32: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 33: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 34: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 35: mad_sat r0.x, r0.y, cb0[2].y, r0.x
    r0.x = (saturate((r0.yyyy)*(source[2].yyyy)+(r0.xxxx))).x;
    // 36: add r0.y, -v2.y, l(1.000000)
    r0.y = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 37: mul_sat r0.y, r0.y, cb0[6].z
    r0.y = (saturate((r0.yyyy)*(source[6].zzzz))).y;
    // 38: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 39: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 40: mul r0.z, r0.z, cb0[6].w
    r0.z = ((r0.zzzz)*(source[6].wwww)).z;
    // 41: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 42: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 43: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 44: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 45: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 46: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 47: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 48: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 49: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 50: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 51: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 52: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 53: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 54: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 55: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_w_pa_spritewave_01_105_tr: 9497e2284e9ad44998098c0a57d12c11; selected map 59850b7176e6a88744c508004a5973759a03b88082a7d84d85dc8a1707430a23.
float4 SDNative369(SD_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[9u];
    source[2] = SDNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = SDNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = SDNativeAppend(g_SDSourceMaterialParameters[4u].yyyy,g_SDSourceMaterialParameters[4u].zzzz,1u);
    source[5] = SDNativeAppend(cos(((g_SDSourceMaterialParameters[5u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[5u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = SDNativeAppend(sin(((g_SDSourceMaterialParameters[5u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_SDSourceMaterialParameters[5u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = SDNativeAppend(cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = SDNativeAppend(sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = g_SDSourceMaterialParameters[8u];
    source[10].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[5u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[10].y = (cos(((g_SDSourceMaterialParameters[5u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].z = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[10].w = (g_SDSourceMaterialTime.xxxx).x;
    source[11].x = (g_SDSourceMaterialParameters[5u].zzzz).x;
    source[11].y = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[11].z = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[11].w = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[12].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[12].z = (g_SDSourceMaterialParameters[7u].xxxx).x;
    source[12].w = (g_SDSourceMaterialParameters[7u].zzzz).x;
    source[13].x = (g_SDSourceMaterialParameters[7u].wwww).x;
    source[13].y = (g_SDSourceMaterialParameters[7u].yyyy).x;
    source[13].z = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[13].w = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[14].x = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[14].y = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[14].z = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[14].w = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[15].x = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[15].y = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[15].z = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[15].w = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[16].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[16].y = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[16].z = ((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[16].w = (sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[17].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[17].y = (cos((g_SDSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[17].z = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[17].w = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[18].x = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[18].y = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[18].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[18].w = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[19].x = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[19].y = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[19].z = (g_SDSourceMaterialParameters[3u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mul r0.x, cb0[10].w, cb0[12].z
    r0.x = ((source[10].wwww)*(source[12].zzzz)).x;
    // 2: mad r0.x, cb0[12].w, v2.x, r0.x
    r0.x = ((source[12].wwww)*(v2.xxxx)+(r0.xxxx)).x;
    // 3: mul r0.z, v2.y, cb0[13].x
    r0.z = ((v2.yyyy)*(source[13].xxxx)).z;
    // 4: mad r0.y, cb0[10].w, cb0[13].y, r0.z
    r0.y = ((source[10].wwww)*(source[13].yyyy)+(r0.zzzz)).y;
    // 5: mad r0.xy, v4.wwww, cb0[13].zwzz, r0.xyxx
    r0.xy = ((v4.wwww)*(source[13].zwzz)+(r0.xyxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (SDNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: add r0.y, v4.z, cb0[14].x
    r0.y = ((v4.zzzz)+(source[14].xxxx)).y;
    // 8: mad r0.xy, r0.xxxx, r0.yyyy, cb0[4].xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(source[4].xyxx)).xy;
    // 9: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 10: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 11: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 12: mad r1.xy, cb0[11].zwzz, v4.xxxx, r1.xyxx
    r1.xy = ((source[11].zwzz)*(v4.xxxx)+(r1.xyxx)).xy;
    // 13: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 14: mul r1.xy, r1.xyxx, cb0[11].xyxx
    r1.xy = ((r1.xyxx)*(source[11].xyxx)).xy;
    // 15: mad r2.x, cb0[10].w, cb0[10].z, r1.x
    r2.x = ((source[10].wwww)*(source[10].zzzz)+(r1.xxxx)).x;
    // 16: mad r2.y, cb0[10].w, cb0[12].y, r1.y
    r2.y = ((source[10].wwww)*(source[12].yyyy)+(r1.yyyy)).y;
    // 17: add r0.xy, r0.xyxx, r2.xyxx
    r0.xy = ((r0.xyxx)+(r2.xyxx)).xy;
    // 18: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 19: dp2 r1.x, cb0[5].xyxx, r0.xyxx
    r1.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 20: dp2 r1.y, cb0[6].xyxx, r0.xyxx
    r1.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 21: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 22: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s0, l(-1.000000)
    r0.x = (SDNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 23: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 24: mul r0.y, r0.y, cb0[14].w
    r0.y = ((r0.yyyy)*(source[14].wwww)).y;
    // 25: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 26: mul r0.y, r0.y, cb0[15].x
    r0.y = ((r0.yyyy)*(source[15].xxxx)).y;
    // 27: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 28: movc r0.y, r1.x, l(0), r0.y
    r0.y = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 29: mad r0.y, r0.x, cb0[15].y, r0.y
    r0.y = ((r0.xxxx)*(source[15].yyyy)+(r0.yyyy)).y;
    // 30: dp2 r1.x, cb0[7].xyxx, r0.zwzz
    r1.x = (dot((source[7].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 31: dp2 r1.y, cb0[8].xyxx, r0.zwzz
    r1.y = (dot((source[8].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 32: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 33: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 34: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 35: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 36: mul r0.z, r0.z, cb0[18].w
    r0.z = ((r0.zzzz)*(source[18].wwww)).z;
    // 37: max r0.z, r0.z, cb0[19].y
    r0.z = (max(r0.zzzz,source[19].yyyy)).z;
    // 38: min r0.z, r0.z, cb0[19].x
    r0.z = (min(r0.zzzz,source[19].xxxx)).z;
    // 39: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 40: mul r0.w, r1.x, cb0[15].w
    r0.w = ((r1.xxxx)*(source[15].wwww)).w;
    // 41: mul r1.x, r1.y, cb0[16].x
    r1.x = ((r1.yyyy)*(source[16].xxxx)).x;
    // 42: mad r1.x, cb0[10].w, cb0[17].z, r1.x
    r1.x = ((source[10].wwww)*(source[17].zzzz)+(r1.xxxx)).x;
    // 43: add r1.y, r1.x, cb0[18].x
    r1.y = ((r1.xxxx)+(source[18].xxxx)).y;
    // 44: mad r0.w, cb0[10].w, cb0[15].z, r0.w
    r0.w = ((source[10].wwww)*(source[15].zzzz)+(r0.wwww)).w;
    // 45: add r1.x, r0.w, cb0[17].w
    r1.x = ((r0.wwww)+(source[17].wwww)).x;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s2, l(0.000000)
    r0.w = (SDNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 47: add r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 48: add r1.x, v4.y, l(-1.000000)
    r1.x = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 49: add_sat r0.w, r0.w, -r1.x
    r0.w = (saturate((r0.wwww)+(-(r1.xxxx)))).w;
    // 50: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 51: mul r1.x, r1.x, cb0[18].z
    r1.x = ((r1.xxxx)*(source[18].zzzz)).x;
    // 52: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 53: mul_sat r1.x, r1.x, cb0[18].y
    r1.x = (saturate((r1.xxxx)*(source[18].yyyy))).x;
    // 54: lt r1.y, r0.w, l(0.000001)
    r1.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 55: mul r0.w, r0.w, cb0[18].y
    r0.w = ((r0.wwww)*(source[18].yyyy)).w;
    // 56: movc r1.x, r1.y, l(-0.000000), -r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r1.xxxx))).x;
    // 57: mov_sat r1.y, r0.w
    r1.y = (saturate(r0.wwww)).y;
    // 58: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 59: mul r0.x, r0.x, cb0[19].z
    r0.x = ((r0.xxxx)*(source[19].zzzz)).x;
    // 60: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 61: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 62: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 63: add r0.x, r1.x, r1.y
    r0.x = ((r1.xxxx)+(r1.yyyy)).x;
    // 64: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 65: mad r0.xyz, r0.xxxx, cb0[9].xyzx, r0.yyyy
    r0.xyz = ((r0.xxxx)*(source[9].xyzx)+(r0.yyyy)).xyz;
    // 66: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 67: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// bfx_d_pa_afterburn_01_39_tr: 72932af96239ac43bcb824d25b2469a1; selected map bfc8213a9369a954275cc04bf09c627febfecf337e968ac148f68cc0bdd69a4c.
float4 SDNative370(SD_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[5u];
    source[2] = g_SDSourceMaterialParameters[3u];
    source[3] = g_SDSourceMaterialParameters[4u];
    source[4] = SDNativeAppend(cos((g_SDSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[5] = SDNativeAppend(sin((g_SDSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[6] = SDNativeAppend(cos(((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[7] = SDNativeAppend(sin(((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[8] = SDNativeAppend(cos((((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[9] = SDNativeAppend(sin((((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos((((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[10].x = (cos((g_SDSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[10].y = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[10].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[10].w = ((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)).x;
    source[11].x = (cos((((g_SDSourceMaterialParameters[2u].yyyy+g_SDSourceMaterialParameters[2u].zzzz)+g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[11].z = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[11].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[12].x = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[12].y = ((g_SDSourceMaterialParameters[1u].zzzz*float4(3.0, 0.0, 0.0, 0.0))).x;
    source[12].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[12].w = ((g_SDSourceMaterialParameters[1u].wwww*float4(20.0, 0.0, 0.0, 0.0))).x;
    source[13].x = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[13].y = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[13].z = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[13].w = ((float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].yyyy)).x;
    source[14].x = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].yyyy))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mul r0.x, v4.w, cb0[10].y
    r0.x = ((v4.wwww)*(source[10].yyyy)).x;
    // 2: mad r0.yz, r0.xxxx, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.xxxx)*(v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 3: mul r1.xyzw, r0.xxxx, l(1.330000, 1.330000, 1.768900, 1.768900)
    r1.xyzw = ((r0.xxxx)*(float4(asfloat(0x3faa3d71u),asfloat(0x3faa3d71u),asfloat(0x3fe26b52u),asfloat(0x3fe26b52u)))).xyzw;
    // 4: mad r1.xyzw, r1.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r1.xyzw = ((r1.xyzw)*(v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 5: dp2 r0.x, cb0[5].xyxx, r0.yzyy
    r0.x = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 6: dp2 r2.x, cb0[4].xyxx, r0.yzyy
    r2.x = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 7: mad r2.y, v4.x, l(0.020000), r0.x
    r2.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.xxxx)).y;
    // 8: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (SDNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 10: dp2 r0.y, cb0[7].xyxx, r1.xyxx
    r0.y = (dot((source[7].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 11: mad r2.y, v4.x, l(0.020000), r0.y
    r2.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.yyyy)).y;
    // 12: dp2 r2.x, cb0[6].xyxx, r1.xyxx
    r2.x = (dot((source[6].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 13: add r0.yz, r2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (SDNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: mul r0.y, r0.y, l(0.333300)
    r0.y = ((r0.yyyy)*(float4(0.333300,0.333300,0.333300,0.333300))).y;
    // 16: mad r0.x, r0.x, l(0.333300), r0.y
    r0.x = ((r0.xxxx)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.yyyy)).x;
    // 17: dp2 r0.y, cb0[9].xyxx, r1.zwzz
    r0.y = (dot((source[9].xyxx).xy,(r1.zwzz).xy).xxxx).y;
    // 18: dp2 r1.x, cb0[8].xyxx, r1.zwzz
    r1.x = (dot((source[8].xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 19: mad r1.y, v4.x, l(0.020000), r0.y
    r1.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.yyyy)).y;
    // 20: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (SDNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 22: mad r0.x, r0.y, l(0.333300), r0.x
    r0.x = ((r0.yyyy)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.xxxx)).x;
    // 23: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 24: mul r0.y, r0.y, cb0[12].y
    r0.y = ((r0.yyyy)*(source[12].yyyy)).y;
    // 25: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 26: mul r0.y, r0.y, cb0[12].w
    r0.y = ((r0.yyyy)*(source[12].wwww)).y;
    // 27: mov_sat r0.z, v4.z
    r0.z = (saturate(v4.zzzz)).z;
    // 28: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 30: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 31: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 32: movc r0.y, r0.z, l(-0.000000), -r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).y;
    // 33: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 34: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 35: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 36: mad r0.z, -r0.z, l(1.428571), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(asfloat(0x3fb6db6eu),asfloat(0x3fb6db6eu),asfloat(0x3fb6db6eu),asfloat(0x3fb6db6eu)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).z;
    // 37: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 38: add r0.w, r0.z, r0.x
    r0.w = ((r0.zzzz)+(r0.xxxx)).w;
    // 39: mul_sat r0.x, r0.x, cb0[11].y
    r0.x = (saturate((r0.xxxx)*(source[11].yyyy))).x;
    // 40: add r0.w, r0.w, -cb0[11].w
    r0.w = ((r0.wwww)+(-(source[11].wwww))).w;
    // 41: mad r0.y, r0.w, r0.z, r0.y
    r0.y = ((r0.wwww)*(r0.zzzz)+(r0.yyyy)).y;
    // 42: mul_sat r0.y, r0.y, cb0[13].x
    r0.y = (saturate((r0.yyyy)*(source[13].xxxx))).y;
    // 43: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 44: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 45: mul r0.z, r0.z, cb0[13].y
    r0.z = ((r0.zzzz)*(source[13].yyyy)).z;
    // 46: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 47: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 48: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 49: source device depth mapped to centimetre view depth; reconstruction at 51.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 51-54: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 55: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 56: add r1.x, -cb0[14].x, l(1.000000)
    r1.x = ((-(source[14].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 57: mul r1.x, r1.x, l(100.000000)
    r1.x = ((r1.xxxx)*(float4(100.000000,100.000000,100.000000,100.000000))).x;
    // 58: max r1.x, r1.x, l(0.001000)
    r1.x = (max(r1.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 59: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 60: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 61: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 62: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 63: movc o0.w, r0.y, l(0), r0.z
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).w;
    // 64: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 65: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 66: mul r0.y, r0.y, cb0[11].z
    r0.y = ((r0.yyyy)*(source[11].zzzz)).y;
    // 67: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 68: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 69: mul r0.yzw, cb0[2].xxyz, cb0[2].wwww
    r0.yzw = ((source[2].xxyz)*(source[2].wwww)).yzw;
    // 70: mad r1.xyz, cb0[3].wwww, cb0[3].xyzx, -r0.yzwy
    r1.xyz = ((source[3].wwww)*(source[3].xyzx)+(-(r0.yzwy))).xyz;
    // 71: mad r0.xyz, r0.xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 72: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 73: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_d_pa_ring_11_09_ts_tr: 14465aa42edf404dafced7434bca7e8a; selected map 01d7f91374213b751435f61477a95c92aa20c2401f42bff27704c5421c25ef4c.
float4 SDNative371(SD_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[2u];
    source[2].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_SDSourceMaterialParameters[1u].xxxx)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_SDSourceMaterialParameters[1u].xxxx))).x;
    source[2].w = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[3].x = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[3].w = ((float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].xxxx)).x;
    source[4].x = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].xxxx))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mul_sat r0.x, v4.y, cb0[2].w
    r0.x = (saturate((v4.yyyy)*(source[2].wwww))).x;
    // 2: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 3: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 4: max r0.x, r0.x, l(0.000010)
    r0.x = (max(r0.xxxx,float4(0.000010,0.000010,0.000010,0.000010))).x;
    // 5: div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
    r0.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.xxxx)).x;
    // 6: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 7: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 8: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 9: mad r0.y, -r0.y, cb0[2].z, l(1.000000)
    r0.y = ((-(r0.yyyy))*(source[2].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 10: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 11: add r0.y, -r0.x, l(1.000000)
    r0.y = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 12: mul_sat r0.y, r0.y, cb0[3].x
    r0.y = (saturate((r0.yyyy)*(source[3].xxxx))).y;
    // 13: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 14: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 15: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: mul r0.z, v4.x, cb0[3].y
    r0.z = ((v4.xxxx)*(source[3].yyyy)).z;
    // 17: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 18: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 19: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 20: mov_sat r0.y, v4.z
    r0.y = (saturate(v4.zzzz)).y;
    // 21: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 22: add_sat r0.x, -r0.y, r0.x
    r0.x = (saturate((-(r0.yyyy))+(r0.xxxx))).x;
    // 23: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 24: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 25: source device depth mapped to centimetre view depth; reconstruction at 27.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 27-30: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 31: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 32: add r0.z, -cb0[4].x, l(1.000000)
    r0.z = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 34: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 35: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 36: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 37: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 38: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 39: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 40: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_w_pa_blackvoid_01_01_tr: 20d3d597ffa09a4ebf975b656274c88a; selected map 100eb4d71d3eef90c271b688df5a89211ad7e148c223f2fc36f231e5aa0143e3.
float4 SDNative372(SD_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[4u];
    source[2] = SDNativeAppend(cos((g_SDSourceMaterialParameters[1u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[1u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = SDNativeAppend(sin((g_SDSourceMaterialParameters[1u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[1u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = g_SDSourceMaterialParameters[3u];
    source[5].x = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_SDSourceMaterialParameters[0u].xxxx)).x;
    source[5].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_SDSourceMaterialParameters[0u].xxxx))).x;
    source[5].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[6].y = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[7].y = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_SDSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.x, v4.x, cb0[6].w
    r0.x = ((v4.xxxx)+(source[6].wwww)).x;
    // 2: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 3: dp2 r1.x, cb0[2].xyxx, r0.yzyy
    r1.x = (dot((source[2].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 4: dp2 r1.y, cb0[3].xyxx, r0.yzyy
    r1.y = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 5: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 6: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 7: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 8: mad r0.x, r0.w, cb0[6].y, r0.x
    r0.x = ((r0.wwww)*(source[6].yyyy)+(r0.xxxx)).x;
    // 9: mad r1.x, r0.z, cb0[6].x, cb0[6].z
    r1.x = ((r0.zzzz)*(source[6].xxxx)+(source[6].zzzz)).x;
    // 10: mov r0.z, l(-1.000000)
    r0.z = (float4(-1.000000,-1.000000,-1.000000,-1.000000)).z;
    // 11: add r1.y, r0.x, r0.z
    r1.y = ((r0.xxxx)+(r0.zzzz)).y;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (SDNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 13: mad r0.zw, v2.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((v2.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 14: dp2 r0.z, -r0.zwzz, -r0.zwzz
    r0.z = (dot((-(r0.zwzz)).xy,(-(r0.zwzz)).xy).xxxx).z;
    // 15: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 16: mad_sat r0.z, -r0.z, cb0[5].z, l(1.000000)
    r0.z = (saturate((-(r0.zzzz))*(source[5].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000)))).z;
    // 17: mul r1.xy, v4.wzww, cb0[7].xzxx
    r1.xy = ((v4.wzww)*(source[7].xzxx)).xy;
    // 18: mad r0.x, r0.x, r1.x, r0.z
    r0.x = ((r0.xxxx)*(r1.xxxx)+(r0.zzzz)).x;
    // 19: mad r0.x, -cb0[7].y, v4.y, r0.x
    r0.x = ((-(source[7].yyyy))*(v4.yyyy)+(r0.xxxx)).x;
    // 20: mov_sat r0.z, r0.x
    r0.z = (saturate(r0.xxxx)).z;
    // 21: div_sat r0.x, |r0.x|, r1.y
    r0.x = (saturate((abs(r0.xxxx))/(r1.yyyy))).x;
    // 22: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: mul r1.xyz, r0.xxxx, cb0[4].xyzx
    r1.xyz = ((r0.xxxx)*(source[4].xyzx)).xyz;
    // 24: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 25: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 26: mul r1.xyz, r1.xyzx, cb0[7].wwww
    r1.xyz = ((r1.xyzx)*(source[7].wwww)).xyz;
    // 27: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 28: round_pi r0.x, r0.z
    r0.x = (ceil(r0.zzzz)).x;
    // 29: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 30: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 31: mul r0.z, r0.z, l(9.000000)
    r0.z = ((r0.zzzz)*(float4(9.000000,9.000000,9.000000,9.000000))).z;
    // 32: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 33: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 34: lt r0.z, r0.y, l(0.000000)
    r0.z = (asfloat((uint4)((r0.yyyy)<(float4(asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu)))) * 0xffffffffu)).z;
    // 35: add r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)+(r0.yyyy)).y;
    // 36: mul r0.w, r0.y, r0.y
    r0.w = ((r0.yyyy)*(r0.yyyy)).w;
    // 37: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 38: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 39: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 40: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 41: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 42: max r0.xyz, v3.xyzx, l(-5.000000, -5.000000, -5.000000, 0.000000)
    r0.xyz = (max(v3.xyzx,float4(-5.000000,-5.000000,-5.000000,0.000000))).xyz;
    // 43: min r0.xyz, r0.xyzx, l(1000.000000, 1000.000000, 1000.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1000.000000,1000.000000,1000.000000,0.000000))).xyz;
    // 44: mad r0.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 45: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_w_pa_afterburn_01_27_dt_tr: eaee74bbdf5b2f43954baa440b420a7c; selected map 1e28a165d1d3816a06358c9b91ab5678691cb234309b4ec0e65ded5365184046.
float4 SDNative373(SD_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[7u];
    source[2] = SDNativeAppend(cos((g_SDSourceMaterialParameters[2u].wwww*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[2u].wwww*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = SDNativeAppend(sin((g_SDSourceMaterialParameters[2u].wwww*float4(1.0, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[2u].wwww*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4] = SDNativeAppend(SDNativePeriodic((((g_SDSourceMaterialTime.xxxx+float4(0.0, 0.0, 0.0, 0.0))*g_SDSourceMaterialParameters[3u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic((((g_SDSourceMaterialTime.xxxx+float4(0.0, 0.0, 0.0, 0.0))*g_SDSourceMaterialParameters[3u].yyyy)*float4(0.0199999996, 0.0, 0.0, 0.0))),1u);
    source[5] = SDNativeAppend(cos(((g_SDSourceMaterialParameters[2u].wwww+g_SDSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialParameters[2u].wwww+g_SDSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[6] = SDNativeAppend(sin(((g_SDSourceMaterialParameters[2u].wwww+g_SDSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((g_SDSourceMaterialParameters[2u].wwww+g_SDSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[7] = SDNativeAppend(cos((((g_SDSourceMaterialParameters[2u].wwww+g_SDSourceMaterialParameters[3u].xxxx)+g_SDSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((((g_SDSourceMaterialParameters[2u].wwww+g_SDSourceMaterialParameters[3u].xxxx)+g_SDSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[8] = SDNativeAppend(sin((((g_SDSourceMaterialParameters[2u].wwww+g_SDSourceMaterialParameters[3u].xxxx)+g_SDSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos((((g_SDSourceMaterialParameters[2u].wwww+g_SDSourceMaterialParameters[3u].xxxx)+g_SDSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[9] = g_SDSourceMaterialParameters[6u];
    source[10] = g_SDSourceMaterialParameters[4u];
    source[11] = g_SDSourceMaterialParameters[5u];
    source[12].x = (sin((g_SDSourceMaterialParameters[2u].wwww*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[12].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[2u].wwww*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[12].z = (cos((g_SDSourceMaterialParameters[2u].wwww*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[12].w = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[13].x = (cos((((g_SDSourceMaterialParameters[2u].wwww+g_SDSourceMaterialParameters[3u].xxxx)+g_SDSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[13].y = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[13].z = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[13].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[14].x = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[14].y = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[14].z = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[14].w = ((g_SDSourceMaterialParameters[2u].xxxx*float4(3.0, 0.0, 0.0, 0.0))).x;
    source[15].x = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[15].y = ((g_SDSourceMaterialParameters[2u].yyyy*float4(20.0, 0.0, 0.0, 0.0))).x;
    source[15].z = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[15].w = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[16].x = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[16].y = ((float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].yyyy)).x;
    source[16].z = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].yyyy))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
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
    // 10: add r0.y, -cb0[16].z, l(1.000000)
    r0.y = ((-(source[16].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, l(100.000000)
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 12: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 13: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 14: mul r0.y, v4.w, cb0[12].w
    r0.y = ((v4.wwww)*(source[12].wwww)).y;
    // 15: mad r0.zw, r0.yyyy, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.yyyy)*(v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 16: mul r1.xyzw, r0.yyyy, l(1.330000, 1.330000, 1.768900, 1.768900)
    r1.xyzw = ((r0.yyyy)*(float4(asfloat(0x3faa3d71u),asfloat(0x3faa3d71u),asfloat(0x3fe26b52u),asfloat(0x3fe26b52u)))).xyzw;
    // 17: mad r1.xyzw, r1.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r1.xyzw = ((r1.xyzw)*(v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 18: dp2 r2.x, cb0[2].xyxx, r0.zwzz
    r2.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 19: dp2 r2.y, cb0[3].xyxx, r0.zwzz
    r2.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 20: add r0.yz, r2.xxyx, cb0[4].xxyx
    r0.yz = ((r2.xxyx)+(source[4].xxyx)).yz;
    // 21: add r0.yz, r0.yyzy, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (SDNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 23: dp2 r2.x, cb0[5].xyxx, r1.xyxx
    r2.x = (dot((source[5].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 24: dp2 r2.y, cb0[6].xyxx, r1.xyxx
    r2.y = (dot((source[6].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 25: add r0.zw, r2.xxxy, cb0[4].xxxy
    r0.zw = ((r2.xxxy)+(source[4].xxxy)).zw;
    // 26: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (SDNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 28: mul r0.z, r0.z, l(0.333300)
    r0.z = ((r0.zzzz)*(float4(0.333300,0.333300,0.333300,0.333300))).z;
    // 29: mad r0.y, r0.y, l(0.333300), r0.z
    r0.y = ((r0.yyyy)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.zzzz)).y;
    // 30: dp2 r1.x, cb0[7].xyxx, r1.zwzz
    r1.x = (dot((source[7].xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 31: dp2 r1.y, cb0[8].xyxx, r1.zwzz
    r1.y = (dot((source[8].xyxx).xy,(r1.zwzz).xy).xxxx).y;
    // 32: add r0.zw, r1.xxxy, cb0[4].xxxy
    r0.zw = ((r1.xxxy)+(source[4].xxxy)).zw;
    // 33: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (SDNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 35: mad r0.y, r0.z, l(0.333300), r0.y
    r0.y = ((r0.zzzz)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.yyyy)).y;
    // 36: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 37: mul r0.z, r0.z, cb0[14].w
    r0.z = ((r0.zzzz)*(source[14].wwww)).z;
    // 38: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 39: mul r0.z, r0.z, cb0[15].y
    r0.z = ((r0.zzzz)*(source[15].yyyy)).z;
    // 40: mov_sat r0.w, v4.z
    r0.w = (saturate(v4.zzzz)).w;
    // 41: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 43: lt r0.w, |r0.y|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 44: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 45: movc r0.z, r0.w, l(-0.000000), -r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.zzzz))).z;
    // 46: add r1.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 47: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 48: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 49: mad r0.w, -r0.w, l(1.428571), l(1.000000)
    r0.w = ((-(r0.wwww))*(float4(asfloat(0x3fb6db6eu),asfloat(0x3fb6db6eu),asfloat(0x3fb6db6eu),asfloat(0x3fb6db6eu)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 50: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 51: add r1.x, r0.w, r0.y
    r1.x = ((r0.wwww)+(r0.yyyy)).x;
    // 52: add r1.x, r1.x, -cb0[14].y
    r1.x = ((r1.xxxx)+(-(source[14].yyyy))).x;
    // 53: mad r0.z, r1.x, r0.w, r0.z
    r0.z = ((r1.xxxx)*(r0.wwww)+(r0.zzzz)).z;
    // 54: mul_sat r0.z, r0.z, cb0[15].z
    r0.z = (saturate((r0.zzzz)*(source[15].zzzz))).z;
    // 55: log r1.x, r0.z
    r1.x = (log2(r0.zzzz)).x;
    // 56: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 57: mul r1.x, r1.x, cb0[15].w
    r1.x = ((r1.xxxx)*(source[15].wwww)).x;
    // 58: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 59: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 60: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 61: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 62: movc o0.w, r0.z, l(0), r0.x
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 63: add r0.x, -r0.y, l(1.000000)
    r0.x = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 64: mul_sat r0.y, r0.y, cb0[13].w
    r0.y = (saturate((r0.yyyy)*(source[13].wwww))).y;
    // 65: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 66: mul r0.x, r0.x, cb0[13].y
    r0.x = ((r0.xxxx)*(source[13].yyyy)).x;
    // 67: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 68: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 69: mul r0.z, r0.z, cb0[13].z
    r0.z = ((r0.zzzz)*(source[13].zzzz)).z;
    // 70: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 71: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 72: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 73: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 74: mul r0.z, r0.z, cb0[14].x
    r0.z = ((r0.zzzz)*(source[14].xxxx)).z;
    // 75: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 76: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 77: mul r1.xyz, cb0[10].xyzx, cb0[10].wwww
    r1.xyz = ((source[10].xyzx)*(source[10].wwww)).xyz;
    // 78: mad r2.xyz, cb0[11].wwww, cb0[11].xyzx, -r1.xyzx
    r2.xyz = ((source[11].wwww)*(source[11].xyzx)+(-(r1.xyzx))).xyz;
    // 79: mad r0.yzw, r0.yyyy, r2.xxyz, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r2.xxyz)+(r1.xxyz)).yzw;
    // 80: mul r1.xyz, cb0[9].xyzx, cb0[9].wwww
    r1.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 81: mul r1.xyz, r1.xyzx, v4.yyyy
    r1.xyz = ((r1.xyzx)*(v4.yyyy)).xyz;
    // 82: mad r0.xyz, r0.xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 83: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 84: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_j_pa_hologram_01_01_tr: e0b5fe98a973904985c18256b8eb8f8b; selected map 9cbd4e991b8518651a155d9a17d7f315189883f1c0cd060f1d3b6ce469d7dc06.
float4 SDNative375(SD_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[0]=float4(0.f,0.f,0.f,1.f); // Absolute source-world varying, no pre-view translation; engine opacity is W.
    source[1] = g_SDSourceMaterialParameters[7u];
    source[2] = g_SDSourceMaterialParameters[6u];
    source[3] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[4] = SDNativeAppend(g_SDSourceMaterialParameters[1u].xxxx,g_SDSourceMaterialParameters[1u].yyyy,1u);
    source[5] = SDNativeAppend(g_SDSourceMaterialParameters[0u].yyyy,g_SDSourceMaterialParameters[0u].zzzz,1u);
    source[6] = SDNativeAppend(cos(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[4u].yyyy)*float4(-5.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[4u].yyyy)*float4(-5.0, 0.0, 0.0, 0.0)))),1u);
    source[7] = SDNativeAppend(sin(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[4u].yyyy)*float4(-5.0, 0.0, 0.0, 0.0))),cos(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[4u].yyyy)*float4(-5.0, 0.0, 0.0, 0.0))),1u);
    source[8].x = (g_SDSourceMaterialTime.xxxx).x;
    source[8].y = ((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[8].z = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[8].w = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[9].x = ((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[3u].wwww)).x;
    source[9].y = (((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[3u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[9].z = (sin(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[3u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[10].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[10].y = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[10].z = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[10].w = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[11].x = (sin(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[3u].yyyy)*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[11].z = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[11].w = ((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[4u].yyyy)).x;
    source[12].x = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[12].y = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[12].w = ((g_SDSourceMaterialTime.xxxx+g_SDSourceMaterialParameters[4u].wwww)).x;
    source[13].x = ((sin(((((g_SDSourceMaterialTime.xxxx+g_SDSourceMaterialTime.xxxx)/g_SDSourceMaterialParameters[4u].wwww)+g_SDSourceMaterialParameters[4u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*g_SDSourceMaterialParameters[4u].wwww)).x;
    source[13].y = (sin(((((g_SDSourceMaterialTime.xxxx+g_SDSourceMaterialParameters[4u].wwww)/(g_SDSourceMaterialTime.xxxx+g_SDSourceMaterialParameters[4u].wwww))+(((g_SDSourceMaterialTime.xxxx+g_SDSourceMaterialTime.xxxx)/g_SDSourceMaterialParameters[4u].wwww)+g_SDSourceMaterialParameters[4u].wwww))*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[13].z = ((sin(((((g_SDSourceMaterialTime.xxxx+g_SDSourceMaterialParameters[4u].wwww)/(g_SDSourceMaterialTime.xxxx+g_SDSourceMaterialParameters[4u].wwww))+(((g_SDSourceMaterialTime.xxxx+g_SDSourceMaterialTime.xxxx)/g_SDSourceMaterialParameters[4u].wwww)+g_SDSourceMaterialParameters[4u].wwww))*float4(6.28318548, 0.0, 0.0, 0.0)))*(sin(((((g_SDSourceMaterialTime.xxxx+g_SDSourceMaterialTime.xxxx)/g_SDSourceMaterialParameters[4u].wwww)+g_SDSourceMaterialParameters[4u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*g_SDSourceMaterialParameters[4u].wwww))).x;
    source[13].w = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[14].x = ((float4(3.0, 0.0, 0.0, 0.0)+(sin(((((g_SDSourceMaterialTime.xxxx+g_SDSourceMaterialParameters[4u].wwww)/(g_SDSourceMaterialTime.xxxx+g_SDSourceMaterialParameters[4u].wwww))+(((g_SDSourceMaterialTime.xxxx+g_SDSourceMaterialTime.xxxx)/g_SDSourceMaterialParameters[4u].wwww)+g_SDSourceMaterialParameters[4u].wwww))*float4(6.28318548, 0.0, 0.0, 0.0)))*(sin(((((g_SDSourceMaterialTime.xxxx+g_SDSourceMaterialTime.xxxx)/g_SDSourceMaterialParameters[4u].wwww)+g_SDSourceMaterialParameters[4u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*g_SDSourceMaterialParameters[4u].wwww)))).x;
    source[14].y = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[14].z = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[14].w = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[15].x = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[15].y = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[15].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[15].w = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[16].x = (g_SDSourceMaterialParameters[2u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xy, v2.xyxx, cb0[3].xyxx
    r0.xy = ((v2.xyxx)+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t2.yzxw, s2, l(0.000000)
    r0.z = (SDNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (SDNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 5: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: max r0.x, r0.x, cb0[9].z
    r0.x = (max(r0.xxxx,source[9].zzzz)).x;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xxxx, t1.xyzw, s1, l(0.000000)
    r0.x = (SDNativeSample1((r0.xxxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: mad r0.x, r0.x, l(2.000000), l(-1.000000)
    r0.x = ((r0.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 9: add r0.y, -r0.z, l(1.000000)
    r0.y = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 10: max r0.y, r0.y, cb0[11].x
    r0.y = (max(r0.yyyy,source[11].xxxx)).y;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yyyy, t3.yxzw, s3, l(0.000000)
    r0.y = (SDNativeSample3((r0.yyyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 12: mad r0.y, r0.y, l(2.000000), l(-1.000000)
    r0.y = ((r0.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 13: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 14: mad r1.xy, cb0[10].xxxx, r0.zwzz, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((source[10].xxxx)*(r0.zwzz)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 15: mad r1.xy, r1.xyxx, cb0[4].xyxx, cb0[5].xyxx
    r1.xy = ((r1.xyxx)*(source[4].xyxx)+(source[5].xyxx)).xy;
    // 16: mad r1.xy, r0.xxxx, cb0[9].wwww, r1.xyxx
    r1.xy = ((r0.xxxx)*(source[9].wwww)+(r1.xyxx)).xy;
    // 17: mad r0.xy, r0.yyyy, cb0[11].yyyy, r1.xyxx
    r0.xy = ((r0.yyyy)*(source[11].yyyy)+(r1.xyxx)).xy;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t4.yxzw, s4, l(0.000000)
    r0.x = (SDNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 19: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 20: mul r1.x, |r0.x|, |r0.x|
    r1.x = ((abs(r0.xxxx))*(abs(r0.xxxx))).x;
    // 21: movc r0.y, r0.y, l(0), r1.x
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).y;
    // 22: dp2 r1.x, cb0[6].xyxx, r0.zwzz
    r1.x = (dot((source[6].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 23: dp2 r1.y, cb0[7].xyxx, r0.zwzz
    r1.y = (dot((source[7].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 24: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t6.yzxw, s5, l(0.000000)
    r0.z = (SDNativeSample5((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 26: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 27: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 28: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 29: mul r0.z, r0.z, l(1.500000)
    r0.z = ((r0.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 30: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 31: mul r0.z, r0.z, cb0[12].x
    r0.z = ((r0.zzzz)*(source[12].xxxx)).z;
    // 32: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 33: add r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)+(r0.xxxx)).y;
    // 34: mad r0.yzw, r0.yyyy, cb0[2].xxyz, r0.yyyy
    r0.yzw = ((r0.yyyy)*(source[2].xxyz)+(r0.yyyy)).yzw;
    // 35: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 36: mad r2.xyz, r1.xyzx, r0.yzwy, -r0.yzwy
    r2.xyz = ((r1.xyzx)*(r0.yzwy)+(-(r0.yzwy))).xyz;
    // 37: mad r0.yzw, cb0[12].yyyy, r2.xxyz, r0.yyzw
    r0.yzw = ((source[12].yyyy)*(r2.xxyz)+(r0.yyzw)).yzw;
    // 38: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 39: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 40: mul r1.w, r1.w, v6.z
    r1.w = ((r1.wwww)*(v6.zzzz)).w;
    // 41: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 42: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul r2.x, |r1.w|, |r1.w|
    r2.x = ((abs(r1.wwww))*(abs(r1.wwww))).x;
    // 44: mul r2.x, |r1.w|, r2.x
    r2.x = ((abs(r1.wwww))*(r2.xxxx)).x;
    // 45: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 46: mul r1.xyz, r1.xyzx, r2.xxxx
    r1.xyz = ((r1.xyzx)*(r2.xxxx)).xyz;
    // 47: movc r1.xyz, r1.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 48: add r1.w, cb0[14].x, l(-1.000000)
    r1.w = ((source[14].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 49: mad r1.w, cb0[13].w, r1.w, l(1.000000)
    r1.w = ((source[13].wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 51: mad r0.yzw, r1.wwww, r0.yyzw, r1.xxyz
    r0.yzw = ((r1.wwww)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 52: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 53: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 54: mad r0.y, v2.y, l(2.000000), l(-1.000000)
    r0.y = ((v2.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 55: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 56: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 57: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 58: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 59: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 60: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 61: add r0.z, -v2.y, l(1.000000)
    r0.z = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 62: add r0.z, -|r0.z|, l(1.000000)
    r0.z = ((-(abs(r0.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 63: mul r0.z, r0.z, l(30.000000)
    r0.z = ((r0.zzzz)*(float4(30.000000,30.000000,30.000000,30.000000))).z;
    // 64: round_pi r0.z, r0.z
    r0.z = (ceil(r0.zzzz)).z;
    // 65: add r0.w, -v4.x, l(1.000000)
    r0.w = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 66: mad r0.z, r0.z, l(0.033333), -r0.w
    r0.z = ((r0.zzzz)*(float4(asfloat(0x3d088889u),asfloat(0x3d088889u),asfloat(0x3d088889u),asfloat(0x3d088889u)))+(-(r0.wwww))).z;
    // 67: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 68: mul_sat r0.y, r0.y, l(20.000000)
    r0.y = (saturate((r0.yyyy)*(float4(20.000000,20.000000,20.000000,20.000000)))).y;
    // 69: dp3 r0.z, v1.xyzx, v1.xyzx
    r0.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 70: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 71: mul r1.xyzw, r0.zzzz, v1.zxxy
    r1.xyzw = ((r0.zzzz)*(v1.zxxy)).xyzw;
    // 72: dp3 r0.z, v0.xyzx, v0.xyzx
    r0.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 73: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 74: mul r2.xyzw, r0.zzzz, v0.xyzx
    r2.xyzw = ((r0.zzzz)*(v0.xyzx)).xyzw;
    // 75: mul r0.zw, r1.zzzw, r2.zzzw
    r0.zw = ((r1.zzzw)*(r2.zzzw)).zw;
    // 76: mad r0.zw, r1.xxxy, r2.xxxy, -r0.zzzw
    r0.zw = ((r1.xxxy)*(r2.xxxy)+(-(r0.zzzw))).zw;
    // 77: mul r3.xy, r0.zwzz, v1.wwww
    r3.xy = ((r0.zwzz)*(v1.wwww)).xy;
    // 78: add r4.xyz, v7.xyzx, cb0[0].xyzx
    r4.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 79: mul r0.z, r3.x, r4.y
    r0.z = ((r3.xxxx)*(r4.yyyy)).z;
    // 80: mad r0.z, r4.x, r2.y, r0.z
    r0.z = ((r4.xxxx)*(r2.yyyy)+(r0.zzzz)).z;
    // 81: mov r3.z, r2.z
    r3.z = (r2.zzzz).z;
    // 82: mad r0.z, r4.z, r1.w, r0.z
    r0.z = ((r4.zzzz)*(r1.wwww)+(r0.zzzz)).z;
    // 83: mov r3.w, r1.x
    r3.w = (r1.xxxx).w;
    // 84: mul r0.w, cb0[8].x, cb0[14].w
    r0.w = ((source[8].xxxx)*(source[14].wwww)).w;
    // 85: mad r0.z, cb0[14].y, r0.z, r0.w
    r0.z = ((source[14].yyyy)*(r0.zzzz)+(r0.wwww)).z;
    // 86: mul r0.z, r0.z, l(3.141593)
    r0.z = ((r0.zzzz)*(float4(asfloat(0x40490fdbu),asfloat(0x40490fdbu),asfloat(0x40490fdbu),asfloat(0x40490fdbu)))).z;
    // 87: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 88: add r0.w, -cb0[15].y, cb0[15].x
    r0.w = ((-(source[15].yyyy))+(source[15].xxxx)).w;
    // 89: mad r0.z, r0.z, r0.w, cb0[15].y
    r0.z = ((r0.zzzz)*(r0.wwww)+(source[15].yyyy)).z;
    // 90: mul r0.w, |r0.z|, |r0.z|
    r0.w = ((abs(r0.zzzz))*(abs(r0.zzzz))).w;
    // 91: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 92: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 93: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 94: movc r0.z, r0.z, l(1.000000), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.wwww)).z;
    // 95: mul r0.z, r0.z, cb0[15].z
    r0.z = ((r0.zzzz)*(source[15].zzzz)).z;
    // 96: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t5.yzwx, s6, l(0.000000)
    r0.w = (SDNativeSample6((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 97: add r1.x, -cb0[15].w, l(1.000000)
    r1.x = ((-(source[15].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 98: mad_sat r0.w, r0.w, r1.x, cb0[15].w
    r0.w = (saturate((r0.wwww)*(r1.xxxx)+(source[15].wwww))).w;
    // 99: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 100: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 101: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 102: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 103: mul o0.w, r0.x, cb0[0].w
    output.w = ((r0.xxxx)*(source[0].wwww)).w;
    return output;
}

// fx_k_pa_shine_01_tr: a14d57c96465f346be3437ad63c9ad10; selected map 0716202d132c7fa3cb9c994bb5370a1db2710298fb3e58040e580c23ad3e1c1b.
float4 SDNative389(SD_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[6u];
    source[2] = g_SDSourceMaterialParameters[5u];
    source[3] = SDNativeAppend(g_SDSourceMaterialParameters[2u].wwww,g_SDSourceMaterialParameters[3u].xxxx,1u);
    source[4] = SDNativeAppend(g_SDSourceMaterialParameters[0u].zzzz,g_SDSourceMaterialParameters[1u].xxxx,1u);
    source[5] = SDNativeAppend(SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[6] = SDNativeAppend(SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[7] = SDNativeAppend(g_SDSourceMaterialParameters[0u].wwww,g_SDSourceMaterialParameters[1u].yyyy,1u);
    source[8] = SDNativeAppend(SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].wwww)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[9] = SDNativeAppend(SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[2u].yyyy)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[10].x = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_SDSourceMaterialTime.xxxx).x;
    source[10].w = ((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].zzzz)).x;
    source[11].x = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[11].y = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[11].w = (SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].x = (SDNativePeriodic(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[12].w = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[13].y = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[13].z = ((float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[3u].zzzz)).x;
    source[13].w = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[3u].zzzz))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xy, cb0[3].xyxx, v2.xyxx, v4.xyxx
    r0.xy = ((source[3].xyxx)*(v2.xyxx)+(v4.xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (SDNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mad r0.xy, cb0[11].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[11].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 4: mad r1.x, cb0[4].x, r0.x, cb0[5].x
    r1.x = ((source[4].xxxx)*(r0.xxxx)+(source[5].xxxx)).x;
    // 5: mad r1.y, cb0[4].y, r0.y, cb0[6].y
    r1.y = ((source[4].yyyy)*(r0.yyyy)+(source[6].yyyy)).y;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (SDNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 7: mad r1.x, cb0[7].x, r0.x, cb0[8].x
    r1.x = ((source[7].xxxx)*(r0.xxxx)+(source[8].xxxx)).x;
    // 8: mad r1.y, cb0[7].y, r0.y, cb0[9].y
    r1.y = ((source[7].yyyy)*(r0.yyyy)+(source[9].yyyy)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (SDNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 10: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 11: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 12: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 13: mul r0.y, r0.y, cb0[12].y
    r0.y = ((r0.yyyy)*(source[12].yyyy)).y;
    // 14: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, cb0[12].z
    r0.y = ((r0.yyyy)*(source[12].zzzz)).y;
    // 16: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 17: add r0.y, v2.x, l(-0.500000)
    r0.y = ((v2.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 18: add r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)+(r0.yyyy)).y;
    // 19: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 20: mul r0.z, r0.y, v2.y
    r0.z = ((r0.yyyy)*(v2.yyyy)).z;
    // 21: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 22: mad r0.y, r0.y, l(0.500000), r0.z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.zzzz)).y;
    // 23: sqrt r0.z, v2.y
    r0.z = (sqrt(v2.yyyy)).z;
    // 24: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 25: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 26: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 27: mul r0.z, r0.z, cb0[10].x
    r0.z = ((r0.zzzz)*(source[10].xxxx)).z;
    // 28: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 29: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 30: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 31: mad_sat r0.y, r0.y, cb0[13].x, r0.x
    r0.y = (saturate((r0.yyyy)*(source[13].xxxx)+(r0.xxxx))).y;
    // 32: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 33: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 34: source device depth mapped to centimetre view depth; reconstruction at 36.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 36-39: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 40: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 41: add r0.w, -cb0[13].w, l(1.000000)
    r0.w = ((-(source[13].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 43: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 44: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 45: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 46: add r0.z, -v2.y, l(1.000000)
    r0.z = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 47: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 48: mul_sat r0.z, r0.z, cb0[12].w
    r0.z = (saturate((r0.zzzz)*(source[12].wwww))).z;
    // 49: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 50: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 51: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 52: mul r0.yzw, cb0[2].xxyz, cb0[2].wwww
    r0.yzw = ((source[2].xxyz)*(source[2].wwww)).yzw;
    // 53: mad r0.xyz, r0.xxxx, r0.yzwy, v3.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(v3.xyzx)).xyz;
    // 54: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 55: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_j_pa_shine_02_04_tr: d1577402f8cbd5448676eaa31751f2da; selected map be3059ca938dd88bd35f88ed347217a055510c06319e49167d5e4b823f8e0bfe.
float4 SDNative390(SD_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[7u];
    source[2] = g_SDSourceMaterialParameters[6u];
    source[3] = SDNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = SDNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_SDSourceMaterialTime.xxxx).x;
    source[5].z = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[6].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[6].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_SDSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[6].w = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[7].y = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[7].z = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[7].w = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[8].x = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[8].y = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[8].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[8].w = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[9].x = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[9].y = (g_SDSourceMaterialParameters[5u].yyyy).x;
    source[9].z = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[9].w = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[10].x = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[10].y = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[10].z = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[10].w = (g_SDSourceMaterialParameters[3u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.z, r0.x, cb0[6].w
    r0.z = ((r0.xxxx)*(source[6].wwww)).z;
    // 6: mad r1.x, cb0[5].y, cb0[6].z, r0.z
    r1.x = ((source[5].yyyy)*(source[6].zzzz)+(r0.zzzz)).x;
    // 7: mul r0.z, cb0[5].y, cb0[7].y
    r0.z = ((source[5].yyyy)*(source[7].yyyy)).z;
    // 8: mad r1.y, cb0[7].x, r0.y, r0.z
    r1.y = ((source[7].xxxx)*(r0.yyyy)+(r0.zzzz)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = (SDNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 10: mad r0.zw, cb0[7].zzzz, r0.zzzw, r0.xxxy
    r0.zw = ((source[7].zzzz)*(r0.zzzw)+(r0.xxxy)).zw;
    // 11: mul r1.xy, r0.zwzz, cb0[5].zwzz
    r1.xy = ((r0.zwzz)*(source[5].zwzz)).xy;
    // 12: mul r0.zw, r0.zzzw, cb0[8].yyyz
    r0.zw = ((r0.zzzw)*(source[8].yyyz)).zw;
    // 13: mad r0.zw, cb0[5].yyyy, cb0[8].xxxw, r0.zzzw
    r0.zw = ((source[5].yyyy)*(source[8].xxxw)+(r0.zzzw)).zw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s2, l(0.000000)
    r0.z = (SDNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 15: mad r2.x, cb0[5].y, cb0[5].x, r1.x
    r2.x = ((source[5].yyyy)*(source[5].xxxx)+(r1.xxxx)).x;
    // 16: mad r2.y, cb0[5].y, cb0[7].w, r1.y
    r2.y = ((source[5].yyyy)*(source[7].wwww)+(r1.yyyy)).y;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (SDNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 18: add r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)+(r0.wwww)).z;
    // 19: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 20: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 21: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 22: mul r0.w, r0.w, cb0[9].x
    r0.w = ((r0.wwww)*(source[9].xxxx)).w;
    // 23: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 24: mul r0.w, r0.w, cb0[9].y
    r0.w = ((r0.wwww)*(source[9].yyyy)).w;
    // 25: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 26: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 27: mul r0.w, r0.w, cb0[10].x
    r0.w = ((r0.wwww)*(source[10].xxxx)).w;
    // 28: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 29: mul r0.w, r0.w, cb0[10].y
    r0.w = ((r0.wwww)*(source[10].yyyy)).w;
    // 30: lt r1.x, |r0.y|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 31: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 32: add r1.xy, -r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r0.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 33: mul_sat r0.x, r0.x, r1.x
    r0.x = (saturate((r0.xxxx)*(r1.xxxx))).x;
    // 34: mul_sat r0.y, r1.y, cb0[10].z
    r0.y = (saturate((r1.yyyy)*(source[10].zzzz))).y;
    // 35: mul r1.x, r0.x, l(4.000000)
    r1.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 36: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(asfloat(0x348637bdu),asfloat(0x348637bdu),asfloat(0x348637bdu),asfloat(0x348637bdu)))) * 0xffffffffu)).x;
    // 37: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 38: mul r1.x, r1.x, cb0[9].z
    r1.x = ((r1.xxxx)*(source[9].zzzz)).x;
    // 39: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 40: mul r1.x, r1.x, cb0[9].w
    r1.x = ((r1.xxxx)*(source[9].wwww)).x;
    // 41: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 42: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 43: mul_sat r0.x, r0.x, r0.z
    r0.x = (saturate((r0.xxxx)*(r0.zzzz))).x;
    // 44: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 45: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 46: mul r0.z, r0.z, cb0[10].w
    r0.z = ((r0.zzzz)*(source[10].wwww)).z;
    // 47: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 48: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 49: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 50: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 51: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 52: mul r0.xyz, cb0[2].xyzx, cb0[2].wwww
    r0.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 53: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 54: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_j_me_makeflow_03_06_tr: 5db7b7be4bce824eb486c9d7ae062e1f; selected map 5a1d0845cce484fa6b92bf759529b3193bb35f872d747fe030f96a967ee97f46.
float4 SDNative391(SD_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[8u];
    source[3] = g_SDSourceMaterialParameters[6u];
    source[4] = SDNativeAppend(g_SDSourceMaterialParameters[3u].wwww,g_SDSourceMaterialParameters[4u].xxxx,1u);
    source[5] = SDNativeAppend((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[3u].yyyy),(g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[3u].zzzz),1u);
    source[6] = input.dynamicParameter;
    source[7] = SDNativeAppend((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[0u].yyyy),(g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[0u].zzzz),1u);
    source[8] = SDNativeAppend((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].yyyy),(g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].zzzz),1u);
    source[9] = SDNativeAppend(g_SDSourceMaterialParameters[5u].zzzz,g_SDSourceMaterialParameters[5u].wwww,1u);
    source[10] = SDNativeAppend(cos((g_SDSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = SDNativeAppend(sin((g_SDSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[12].y = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[12].z = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[12].w = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[13].x = ((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[0u].zzzz)).x;
    source[13].y = ((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[0u].yyyy)).x;
    source[13].z = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[13].w = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[14].x = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[14].y = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[14].z = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[14].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[15].x = (cos((g_SDSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[15].y = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[15].z = (g_SDSourceMaterialParameters[5u].zzzz).x;
    source[15].w = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[16].x = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[16].y = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[16].z = (g_SDSourceMaterialParameters[5u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mov r0.y, cb0[12].z
    r0.y = (source[12].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.zw, v4.xxxy, cb0[4].xxxy, cb0[5].xxxy
    r0.zw = ((v4.xxxy)*(source[4].xxxy)+(source[5].xxxy)).zw;
    // 4: add r1.xyzw, r0.zwzw, r0.yxxy
    r1.xyzw = ((r0.zwzw)+(r0.yxxy)).xyzw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.zwzz, t0.xyzw, s0, l(0.000000)
    r0.x = (SDNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (SDNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (SDNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 8: add r1.xy, -r0.xxxx, r0.yzyy
    r1.xy = ((-(r0.xxxx))+(r0.yzyy)).xy;
    // 9: mul r0.xy, r1.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: mov r0.z, l(0)
    r0.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 11: add r0.xyz, -r0.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 12: dp3 r0.z, r0.xyzx, r0.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 13: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 14: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 15: mad r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: lt r0.z, |cb0[12].x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(source[12].xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 17: mul r0.w, |cb0[12].x|, |cb0[12].x|
    r0.w = ((abs(source[12].xxxx))*(abs(source[12].xxxx))).w;
    // 18: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 19: mul r1.y, r0.z, v4.y
    r1.y = ((r0.zzzz)*(v4.yyyy)).y;
    // 20: mul r1.x, v4.x, cb0[12].y
    r1.x = ((v4.xxxx)*(source[12].yyyy)).x;
    // 21: mad r0.zw, cb0[6].xxxx, r0.xxxy, r1.xxxy
    r0.zw = ((source[6].xxxx)*(r0.xxxy)+(r1.xxxy)).zw;
    // 22: add r0.w, r0.w, cb0[6].w
    r0.w = ((r0.wwww)+(source[6].wwww)).w;
    // 23: add r0.zw, r0.zzzw, cb0[7].xxxy
    r0.zw = ((r0.zzzw)+(source[7].xxxy)).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s1, l(0.000000)
    r1.xyz = (SDNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 25: lt r0.z, |cb0[13].z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(source[13].zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r0.w, |cb0[13].z|, |cb0[13].z|
    r0.w = ((abs(source[13].zzzz))*(abs(source[13].zzzz))).w;
    // 27: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 28: mul r2.y, r0.z, v4.y
    r2.y = ((r0.zzzz)*(v4.yyyy)).y;
    // 29: mul r2.x, v4.x, cb0[13].w
    r2.x = ((v4.xxxx)*(source[13].wwww)).x;
    // 30: mad r0.zw, cb0[6].xxxx, r0.xxxy, r2.xxxy
    r0.zw = ((source[6].xxxx)*(r0.xxxy)+(r2.xxxy)).zw;
    // 31: mul r0.xy, r0.xyxx, cb0[6].xxxx
    r0.xy = ((r0.xyxx)*(source[6].xxxx)).xy;
    // 32: add r0.zw, r0.zzzw, cb0[8].xxxy
    r0.zw = ((r0.zzzw)+(source[8].xxxy)).zw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t3.xyzw, s2, l(0.000000)
    r2.xyz = (SDNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 34: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 35: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 36: mad r1.xyz, -r1.xyzx, r2.xyzx, r0.zzzz
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r0.zzzz)).xyz;
    // 37: mad r1.xyz, cb0[14].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[14].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 38: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 39: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 40: mul r1.xyz, r1.xyzx, cb0[14].yyyy
    r1.xyz = ((r1.xyzx)*(source[14].yyyy)).xyz;
    // 41: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 42: mul r1.xyz, r1.xyzx, cb0[14].zzzz
    r1.xyz = ((r1.xyzx)*(source[14].zzzz)).xyz;
    // 43: mad r1.xyz, cb0[1].xyzx, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((source[1].xyzx)*(r1.xyzx)+(source[3].xyzx)).xyz;
    // 44: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 45: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 46: mad r0.zw, v4.xxxy, cb0[9].xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)*(source[9].xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 47: dp2 r1.x, cb0[10].xyxx, r0.zwzz
    r1.x = (dot((source[10].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 48: dp2 r1.y, cb0[11].xyxx, r0.zwzz
    r1.y = (dot((source[11].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 49: add r0.zw, r1.xxxy, cb0[6].zzzy
    r0.zw = ((r1.xxxy)+(source[6].zzzy)).zw;
    // 50: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 51: mad r0.xy, cb0[15].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[15].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (SDNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 53: mad r0.y, v4.y, l(2.000000), l(-1.000000)
    r0.y = ((v4.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 54: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 55: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 56: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 57: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 58: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 59: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 60: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 61: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 62: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 63: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 64: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 65: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 66: mul r0.z, r0.z, cb0[14].w
    r0.z = ((r0.zzzz)*(source[14].wwww)).z;
    // 67: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 68: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 69: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 70: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 71: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 72: mul r0.x, r0.x, cb0[16].y
    r0.x = ((r0.xxxx)*(source[16].yyyy)).x;
    // 73: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 74: mul_sat r0.x, r0.x, cb0[16].z
    r0.x = (saturate((r0.xxxx)*(source[16].zzzz))).x;
    // 75: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 76: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

// fx_j_me_linearflow_02_14_tr: a1991629e6776e40b3eeeae7c262e8ae; selected map d774239272a22634237d30b9d7e0e7bdc8b5b71c7969da40ca6a1e14dc1799f9.
float4 SDNative392(SD_NATIVE_INPUT input)
{
    float4 source[30]; [unroll] for (uint i=0u; i<30u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[16u];
    source[3] = input.dynamicParameter;
    source[4] = SDNativeAppend(cos((g_SDSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = SDNativeAppend(sin((g_SDSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = SDNativeAppend(g_SDSourceMaterialParameters[2u].yyyy,g_SDSourceMaterialParameters[2u].zzzz,1u);
    source[7] = SDNativeAppend(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].zzzz)+g_SDSourceMaterialParameters[1u].xxxx),((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].wwww)+g_SDSourceMaterialParameters[1u].yyyy),1u);
    source[8] = g_SDSourceMaterialParameters[13u];
    source[9] = SDNativeAppend(cos((g_SDSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = SDNativeAppend(sin((g_SDSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11] = SDNativeAppend(g_SDSourceMaterialParameters[6u].zzzz,g_SDSourceMaterialParameters[6u].wwww,1u);
    source[12] = SDNativeAppend(((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[5u].wwww)+g_SDSourceMaterialParameters[5u].yyyy),((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[6u].xxxx)+g_SDSourceMaterialParameters[5u].zzzz),1u);
    source[13] = g_SDSourceMaterialParameters[14u];
    source[14] = SDNativeAppend(cos((g_SDSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[15] = SDNativeAppend(sin((g_SDSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[16].x = (g_SDSourceMaterialParameters[9u].yyyy).x;
    source[16].y = (g_SDSourceMaterialParameters[9u].zzzz).x;
    source[16].z = (g_SDSourceMaterialParameters[9u].xxxx).x;
    source[16].w = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[17].x = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[17].y = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[17].z = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[17].w = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[18].x = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[18].y = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[18].z = (g_SDSourceMaterialTime.xxxx).x;
    source[18].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[19].x = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[19].y = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[19].z = (floor(g_SDSourceMaterialParameters[4u].yyyy)).x;
    source[19].w = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[20].x = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[20].y = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[20].z = (g_SDSourceMaterialParameters[7u].zzzz).x;
    source[20].w = ((g_SDSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[21].x = (sin((g_SDSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[21].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[21].z = (cos((g_SDSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[21].w = (g_SDSourceMaterialParameters[7u].wwww).x;
    source[22].x = (g_SDSourceMaterialParameters[8u].xxxx).x;
    source[22].y = (g_SDSourceMaterialParameters[8u].yyyy).x;
    source[22].z = (g_SDSourceMaterialParameters[7u].xxxx).x;
    source[22].w = (g_SDSourceMaterialParameters[7u].yyyy).x;
    source[23].x = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[23].y = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[23].z = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[23].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[24].x = (floor(g_SDSourceMaterialParameters[0u].xxxx)).x;
    source[24].y = (g_SDSourceMaterialParameters[5u].xxxx).x;
    source[24].z = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[24].w = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[25].x = (g_SDSourceMaterialParameters[8u].wwww).x;
    source[25].y = (g_SDSourceMaterialParameters[8u].zzzz).x;
    source[25].z = (g_SDSourceMaterialParameters[11u].wwww).x;
    source[25].w = (g_SDSourceMaterialParameters[12u].zzzz).x;
    source[26].x = (g_SDSourceMaterialParameters[12u].wwww).x;
    source[26].y = (g_SDSourceMaterialParameters[12u].xxxx).x;
    source[26].z = (g_SDSourceMaterialParameters[11u].zzzz).x;
    source[26].w = (g_SDSourceMaterialParameters[10u].xxxx).x;
    source[27].x = (g_SDSourceMaterialParameters[10u].wwww).x;
    source[27].y = (g_SDSourceMaterialParameters[11u].xxxx).x;
    source[27].z = (g_SDSourceMaterialParameters[10u].zzzz).x;
    source[27].w = ((g_SDSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[28].x = (sin((g_SDSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[28].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[28].z = (cos((g_SDSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[28].w = (g_SDSourceMaterialParameters[10u].yyyy).x;
    source[29].x = (g_SDSourceMaterialParameters[9u].wwww).x;
    source[29].y = (g_SDSourceMaterialParameters[12u].yyyy).x;
    source[29].z = (g_SDSourceMaterialParameters[11u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mad r0.xy, v4.wzww, cb0[11].xyxx, cb0[12].xyxx
    r0.xy = ((v4.wzww)*(source[11].xyxx)+(source[12].xyxx)).xy;
    // 2: add r1.xyzw, r0.xyxy, l(0.200000, 0.000000, 0.000000, 0.200000)
    r1.xyzw = ((r0.xyxy)+(float4(0.200000,0.000000,0.000000,0.200000))).xyzw;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s4, l(0.000000)
    r0.x = (SDNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t2.yxzw, s4, l(0.000000)
    r0.y = (SDNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t2.yzxw, s4, l(0.000000)
    r0.z = (SDNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 6: add r1.xy, -r0.xxxx, r0.yzyy
    r1.xy = ((-(r0.xxxx))+(r0.yzyy)).xy;
    // 7: mul r0.xy, r1.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 8: mov r0.z, l(0)
    r0.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 9: add r0.xyz, -r0.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 10: dp3 r0.z, r0.xyzx, r0.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 11: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 12: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 13: mad r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 14: add r0.z, cb0[3].y, cb0[22].w
    r0.z = ((source[3].yyyy)+(source[22].wwww)).z;
    // 15: add r1.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 16: mad r1.zw, cb0[21].wwww, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((source[21].wwww)*(r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 17: mad r2.y, r1.w, cb0[22].y, r0.z
    r2.y = ((r1.wwww)*(source[22].yyyy)+(r0.zzzz)).y;
    // 18: mad r2.x, r1.z, cb0[22].x, cb0[22].z
    r2.x = ((r1.zzzz)*(source[22].xxxx)+(source[22].zzzz)).x;
    // 19: add r0.zw, r2.xxxy, l(0.000000, 0.000000, -0.500000, -1.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,-0.500000,-1.500000))).zw;
    // 20: dp2 r2.x, cb0[9].xyxx, r0.zwzz
    r2.x = (dot((source[9].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 21: dp2 r2.y, cb0[10].xyxx, r0.zwzz
    r2.y = (dot((source[10].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 22: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 23: mul r1.z, cb0[3].w, cb0[23].z
    r1.z = ((source[3].wwww)*(source[23].zzzz)).z;
    // 24: mad r0.xy, r1.zzzz, r0.xyxx, r0.zwzz
    r0.xy = ((r1.zzzz)*(r0.xyxx)+(r0.zwzz)).xy;
    // 25: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s3, l(-1.000000)
    r0.xyz = (SDNativeSample3((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 26: mad r0.xyzw, cb0[24].xxxx, -r0.xxyz, r0.xxyz
    r0.xyzw = ((source[24].xxxx)*(-(r0.xxyz))+(r0.xxyz)).xyzw;
    // 27: mul r0.xyzw, r0.xyzw, cb0[24].yyyy
    r0.xyzw = ((r0.xyzw)*(source[24].yyyy)).xyzw;
    // 28: max r0.xyzw, |r0.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 29: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 30: mul r0.xyzw, r0.xyzw, cb0[24].zzzz
    r0.xyzw = ((r0.xyzw)*(source[24].zzzz)).xyzw;
    // 31: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 32: mad r1.zw, v4.wwwz, cb0[6].xxxy, cb0[7].xxxy
    r1.zw = ((v4.wwwz)*(source[6].xxxy)+(source[7].xxxy)).zw;
    // 33: add r2.xyzw, r1.zwzw, l(0.200000, 0.000000, 0.000000, 0.200000)
    r2.xyzw = ((r1.zwzw)+(float4(0.200000,0.000000,0.000000,0.200000))).xyzw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t0.yzxw, s2, l(0.000000)
    r1.z = (SDNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t0.yzwx, s2, l(0.000000)
    r1.w = (SDNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.zwzz, t0.xyzw, s2, l(0.000000)
    r2.x = (SDNativeSample2((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 37: add r2.y, -r1.z, r2.x
    r2.y = ((-(r1.zzzz))+(r2.xxxx)).y;
    // 38: add r2.x, -r1.z, r1.w
    r2.x = ((-(r1.zzzz))+(r1.wwww)).x;
    // 39: mul r2.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 40: mov r2.z, l(0)
    r2.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 41: add r2.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 42: dp3 r1.z, r2.xyzx, r2.xyzx
    r1.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 43: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 44: div r1.zw, r2.xxxy, r1.zzzz
    r1.zw = ((r2.xxxy)/(r1.zzzz)).zw;
    // 45: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 46: add r2.x, cb0[3].x, cb0[18].x
    r2.x = ((source[3].xxxx)+(source[18].xxxx)).x;
    // 47: mad r2.yz, cb0[17].xxxx, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((source[17].xxxx)*(r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 48: mad r3.y, r2.z, cb0[17].z, r2.x
    r3.y = ((r2.zzzz)*(source[17].zzzz)+(r2.xxxx)).y;
    // 49: mad r3.x, r2.y, cb0[17].y, cb0[17].w
    r3.x = ((r2.yyyy)*(source[17].yyyy)+(source[17].wwww)).x;
    // 50: add r2.xy, r3.xyxx, l(-0.500000, -1.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(-0.500000,-1.500000,0.000000,0.000000))).xy;
    // 51: dp2 r3.x, cb0[4].xyxx, r2.xyxx
    r3.x = (dot((source[4].xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 52: dp2 r3.y, cb0[5].xyxx, r2.xyxx
    r3.y = (dot((source[5].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 53: add r2.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 54: mul r2.z, cb0[3].z, cb0[19].x
    r2.z = ((source[3].zzzz)*(source[19].xxxx)).z;
    // 55: mad r1.zw, r2.zzzz, r1.zzzw, r2.xxxy
    r1.zw = ((r2.zzzz)*(r1.zzzw)+(r2.xxxy)).zw;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t1.xyzw, s1, l(-1.000000)
    r2.xyz = (SDNativeSample1((r1.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 57: mad r2.xyzw, cb0[19].zzzz, -r2.xxyz, r2.xxyz
    r2.xyzw = ((source[19].zzzz)*(-(r2.xxyz))+(r2.xxyz)).xyzw;
    // 58: mul r2.xyzw, r2.xyzw, cb0[19].wwww
    r2.xyzw = ((r2.xyzw)*(source[19].wwww)).xyzw;
    // 59: max r2.xyzw, |r2.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r2.xyzw = (max(abs(r2.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 60: log r2.xyzw, r2.xyzw
    r2.xyzw = (log2(r2.xyzw)).xyzw;
    // 61: mul r2.xyzw, r2.xyzw, cb0[20].xxxx
    r2.xyzw = ((r2.xyzw)*(source[20].xxxx)).xyzw;
    // 62: exp r2.xyzw, r2.xyzw
    r2.xyzw = (exp2(r2.xyzw)).xyzw;
    // 63: add r0.x, r0.x, -r2.x
    r0.x = ((r0.xxxx)+(-(r2.xxxx))).x;
    // 64: mad r0.x, cb0[25].z, r0.x, r2.x
    r0.x = ((source[25].zzzz)*(r0.xxxx)+(r2.xxxx)).x;
    // 65: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 66: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 67: mul r0.x, r0.x, cb0[25].w
    r0.x = ((r0.xxxx)*(source[25].wwww)).x;
    // 68: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 69: mul r0.x, r0.x, cb0[26].x
    r0.x = ((r0.xxxx)*(source[26].xxxx)).x;
    // 70: mad r1.z, cb0[26].y, l(10.000000), l(10.000000)
    r1.z = ((source[26].yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).z;
    // 71: dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 72: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 74: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 75: mul r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)*(r2.xxxx)).z;
    // 76: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 77: mul r1.z, r1.z, cb0[26].z
    r1.z = ((r1.zzzz)*(source[26].zzzz)).z;
    // 78: movc r1.z, r1.w, l(0), r1.z
    r1.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 79: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 80: dp2 r3.x, cb0[14].xyxx, r1.xyxx
    r3.x = (dot((source[14].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 81: dp2 r3.y, cb0[15].xyxx, r1.xyxx
    r3.y = (dot((source[15].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 82: add r1.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 83: mul r1.xy, r1.xyxx, cb0[27].xyxx
    r1.xy = ((r1.xyxx)*(source[27].xyxx)).xy;
    // 84: mad r3.x, cb0[18].z, cb0[26].w, r1.x
    r3.x = ((source[18].zzzz)*(source[26].wwww)+(r1.xxxx)).x;
    // 85: mad r3.y, cb0[18].z, cb0[28].w, r1.y
    r3.y = ((source[18].zzzz)*(source[28].wwww)+(r1.yyyy)).y;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r3.xyxx, t4.xyzw, s5, l(0.000000)
    r1.x = (SDNativeSample5((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 87: add r1.x, r1.x, l(0.200000)
    r1.x = ((r1.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 88: add r1.y, -cb0[1].w, l(1.000000)
    r1.y = ((-(source[1].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 89: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 90: mul_sat r1.x, r1.x, cb0[29].x
    r1.x = (saturate((r1.xxxx)*(source[29].xxxx))).x;
    // 91: mul r1.x, r1.x, cb0[1].w
    r1.x = ((r1.xxxx)*(source[1].wwww)).x;
    // 92: mul_sat r0.x, r0.x, r1.x
    r0.x = (saturate((r0.xxxx)*(r1.xxxx))).x;
    // 93: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 94: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 95: mul r1.x, r1.x, v6.z
    r1.x = ((r1.xxxx)*(v6.zzzz)).x;
    // 96: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 97: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 98: mul r1.y, r1.y, cb0[29].y
    r1.y = ((r1.yyyy)*(source[29].yyyy)).y;
    // 99: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 100: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 101: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 102: movc o0.w, r1.x, l(0), r0.x
    output.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 103: dp3 r0.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 104: add r1.xyz, -r2.yzwy, r0.xxxx
    r1.xyz = ((-(r2.yzwy))+(r0.xxxx)).xyz;
    // 105: mad r1.xyz, cb0[20].yyyy, r1.xyzx, r2.yzwy
    r1.xyz = ((source[20].yyyy)*(r1.xyzx)+(r2.yzwy)).xyz;
    // 106: mad r2.xy, v4.xyxx, cb0[16].xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((v4.xyxx)*(source[16].xyxx)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 107: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t5.xyzw, s0, l(-1.000000)
    r2.xyz = (SDNativeSample0((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 108: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 109: add r3.xyz, -r2.xyzx, r0.xxxx
    r3.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 110: mad r2.xyz, cb0[16].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[16].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 111: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 112: dp3 r0.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 113: add r3.xyz, -r0.yzwy, r0.xxxx
    r3.xyz = ((-(r0.yzwy))+(r0.xxxx)).xyz;
    // 114: mad r0.xyz, cb0[24].wwww, r3.xyzx, r0.yzwy
    r0.xyz = ((source[24].wwww)*(r3.xyzx)+(r0.yzwy)).xyz;
    // 115: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 116: mul r0.xyz, r0.xyzx, cb0[13].xyzx
    r0.xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 117: mad r0.xyz, r1.xyzx, cb0[8].xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(source[8].xyzx)+(r0.xyzx)).xyz;
    // 118: mul r0.xyz, r0.xyzx, cb0[25].xxxx
    r0.xyz = ((r0.xyzx)*(source[25].xxxx)).xyz;
    // 119: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 120: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 121: mul r0.xyz, r0.xyzx, cb0[25].yyyy
    r0.xyz = ((r0.xyzx)*(source[25].yyyy)).xyz;
    // 122: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 123: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 124: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_s_pa_ring_01_1_ts_tr: 0c82fede9116814d8f31e43c95489cc4; selected map 1246b783fe5b63fa77fbeac10079b81fa189d6f0740367e22ef9db88aa16992a.
float4 SDNative393(SD_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[3u];
    source[2] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.300000012, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[3] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[4].y = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[4].z = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.300000012, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[5].z = ((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[5].w = ((g_SDSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[6].x = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[6].y = (SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[6].w = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[7].y = (g_SDSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xy, v2.xyxx, l(0.800000, 0.500000, 0.000000, 0.000000), cb0[3].xyxx
    r0.xy = ((v2.xyxx)*(float4(0.800000,0.500000,0.000000,0.000000))+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (SDNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 4: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 5: mul r0.x, r0.x, l(1.200000)
    r0.x = ((r0.xxxx)*(float4(1.200000,1.200000,1.200000,1.200000))).x;
    // 6: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 7: mul r0.x, r0.x, cb0[5].y
    r0.x = ((r0.xxxx)*(source[5].yyyy)).x;
    // 8: add r0.yz, v2.xxyx, cb0[2].xxyx
    r0.yz = ((v2.xxyx)+(source[2].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s2, l(0.000000)
    r0.y = (SDNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mul r0.y, r0.y, cb0[5].y
    r0.y = ((r0.yyyy)*(source[5].yyyy)).y;
    // 11: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 12: add r0.yz, v7.xxyx, cb0[0].xxyx
    r0.yz = ((v7.xxyx)+(source[0].xxyx)).yz;
    // 13: mul r0.yz, r0.yyzy, cb0[4].yyzy
    r0.yz = ((r0.yyzy)*(source[4].yyzy)).yz;
    // 14: mul r0.yz, r0.yyzy, l(0.000000, 0.003000, 0.003000, 0.000000)
    r0.yz = ((r0.yyzy)*(float4(0.000000,0.003000,0.003000,0.000000))).yz;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (SDNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 16: mul r0.y, r0.y, cb0[4].w
    r0.y = ((r0.yyyy)*(source[4].wwww)).y;
    // 17: mul r0.y, r0.y, l(0.300000)
    r0.y = ((r0.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))).y;
    // 18: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 19: mad r0.x, r0.x, l(0.200000), r0.y
    r0.x = ((r0.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))+(r0.yyyy)).x;
    // 20: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 21: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 22: mul r0.x, r0.x, cb0[6].z
    r0.x = ((r0.xxxx)*(source[6].zzzz)).x;
    // 23: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 24: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 25: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 26: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 27: mad r0.y, -r0.y, l(2.000000), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 28: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 29: mul r0.z, r0.y, r0.y
    r0.z = ((r0.yyyy)*(r0.yyyy)).z;
    // 30: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 31: log r0.w, r0.y
    r0.w = (log2(r0.yyyy)).w;
    // 32: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 33: mul r0.w, r0.w, cb0[6].w
    r0.w = ((r0.wwww)*(source[6].wwww)).w;
    // 34: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 35: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 36: mul_sat r0.z, r0.z, cb0[7].x
    r0.z = (saturate((r0.zzzz)*(source[7].xxxx))).z;
    // 37: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 38: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 39: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (SDNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 41: max r0.y, |r0.y|, l(0.000001)
    r0.y = (max(abs(r0.yyyy),float4(0.000001,0.000001,0.000001,0.000001))).y;
    // 42: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 43: mul r0.y, r0.y, cb0[4].x
    r0.y = ((r0.yyyy)*(source[4].xxxx)).y;
    // 44: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 45: mad r0.x, r0.y, l(2.000000), r0.x
    r0.x = ((r0.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(r0.xxxx)).x;
    // 46: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 47: mul o0.w, r0.x, cb0[0].w
    output.w = ((r0.xxxx)*(source[0].wwww)).w;
    // 48: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 49: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_o_me_makeflow_03_06_tr: 5db7b7be4bce824eb486c9d7ae062e1f; selected map 5a1d0845cce484fa6b92bf759529b3193bb35f872d747fe030f96a967ee97f46.
float4 SDNative394(SD_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_SDSourceMaterialParameters[8u];
    source[3] = g_SDSourceMaterialParameters[6u];
    source[4] = SDNativeAppend(g_SDSourceMaterialParameters[3u].wwww,g_SDSourceMaterialParameters[4u].xxxx,1u);
    source[5] = SDNativeAppend((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[3u].yyyy),(g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[3u].zzzz),1u);
    source[6] = input.dynamicParameter;
    source[7] = SDNativeAppend((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[0u].yyyy),(g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[0u].zzzz),1u);
    source[8] = SDNativeAppend((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].yyyy),(g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[1u].zzzz),1u);
    source[9] = SDNativeAppend(g_SDSourceMaterialParameters[5u].zzzz,g_SDSourceMaterialParameters[5u].wwww,1u);
    source[10] = SDNativeAppend(cos((g_SDSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = SDNativeAppend(sin((g_SDSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[12].y = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[12].z = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[12].w = (g_SDSourceMaterialParameters[4u].xxxx).x;
    source[13].x = ((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[0u].zzzz)).x;
    source[13].y = ((g_SDSourceMaterialTime.xxxx*g_SDSourceMaterialParameters[0u].yyyy)).x;
    source[13].z = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[13].w = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[14].x = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[14].y = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[14].z = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[14].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[15].x = (cos((g_SDSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[15].y = (g_SDSourceMaterialParameters[5u].wwww).x;
    source[15].z = (g_SDSourceMaterialParameters[5u].zzzz).x;
    source[15].w = (g_SDSourceMaterialParameters[4u].zzzz).x;
    source[16].x = (g_SDSourceMaterialParameters[4u].yyyy).x;
    source[16].y = (g_SDSourceMaterialParameters[4u].wwww).x;
    source[16].z = (g_SDSourceMaterialParameters[5u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mov r0.y, cb0[12].z
    r0.y = (source[12].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.zw, v4.xxxy, cb0[4].xxxy, cb0[5].xxxy
    r0.zw = ((v4.xxxy)*(source[4].xxxy)+(source[5].xxxy)).zw;
    // 4: add r1.xyzw, r0.zwzw, r0.yxxy
    r1.xyzw = ((r0.zwzw)+(r0.yxxy)).xyzw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.zwzz, t0.xyzw, s0, l(0.000000)
    r0.x = (SDNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (SDNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (SDNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 8: add r1.xy, -r0.xxxx, r0.yzyy
    r1.xy = ((-(r0.xxxx))+(r0.yzyy)).xy;
    // 9: mul r0.xy, r1.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: mov r0.z, l(0)
    r0.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 11: add r0.xyz, -r0.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 12: dp3 r0.z, r0.xyzx, r0.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 13: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 14: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 15: mad r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: lt r0.z, |cb0[12].x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(source[12].xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 17: mul r0.w, |cb0[12].x|, |cb0[12].x|
    r0.w = ((abs(source[12].xxxx))*(abs(source[12].xxxx))).w;
    // 18: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 19: mul r1.y, r0.z, v4.y
    r1.y = ((r0.zzzz)*(v4.yyyy)).y;
    // 20: mul r1.x, v4.x, cb0[12].y
    r1.x = ((v4.xxxx)*(source[12].yyyy)).x;
    // 21: mad r0.zw, cb0[6].xxxx, r0.xxxy, r1.xxxy
    r0.zw = ((source[6].xxxx)*(r0.xxxy)+(r1.xxxy)).zw;
    // 22: add r0.w, r0.w, cb0[6].w
    r0.w = ((r0.wwww)+(source[6].wwww)).w;
    // 23: add r0.zw, r0.zzzw, cb0[7].xxxy
    r0.zw = ((r0.zzzw)+(source[7].xxxy)).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s1, l(0.000000)
    r1.xyz = (SDNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 25: lt r0.z, |cb0[13].z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(source[13].zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r0.w, |cb0[13].z|, |cb0[13].z|
    r0.w = ((abs(source[13].zzzz))*(abs(source[13].zzzz))).w;
    // 27: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 28: mul r2.y, r0.z, v4.y
    r2.y = ((r0.zzzz)*(v4.yyyy)).y;
    // 29: mul r2.x, v4.x, cb0[13].w
    r2.x = ((v4.xxxx)*(source[13].wwww)).x;
    // 30: mad r0.zw, cb0[6].xxxx, r0.xxxy, r2.xxxy
    r0.zw = ((source[6].xxxx)*(r0.xxxy)+(r2.xxxy)).zw;
    // 31: mul r0.xy, r0.xyxx, cb0[6].xxxx
    r0.xy = ((r0.xyxx)*(source[6].xxxx)).xy;
    // 32: add r0.zw, r0.zzzw, cb0[8].xxxy
    r0.zw = ((r0.zzzw)+(source[8].xxxy)).zw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t3.xyzw, s2, l(0.000000)
    r2.xyz = (SDNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 34: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 35: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 36: mad r1.xyz, -r1.xyzx, r2.xyzx, r0.zzzz
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r0.zzzz)).xyz;
    // 37: mad r1.xyz, cb0[14].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[14].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 38: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 39: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 40: mul r1.xyz, r1.xyzx, cb0[14].yyyy
    r1.xyz = ((r1.xyzx)*(source[14].yyyy)).xyz;
    // 41: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 42: mul r1.xyz, r1.xyzx, cb0[14].zzzz
    r1.xyz = ((r1.xyzx)*(source[14].zzzz)).xyz;
    // 43: mad r1.xyz, cb0[1].xyzx, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((source[1].xyzx)*(r1.xyzx)+(source[3].xyzx)).xyz;
    // 44: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 45: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 46: mad r0.zw, v4.xxxy, cb0[9].xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)*(source[9].xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 47: dp2 r1.x, cb0[10].xyxx, r0.zwzz
    r1.x = (dot((source[10].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 48: dp2 r1.y, cb0[11].xyxx, r0.zwzz
    r1.y = (dot((source[11].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 49: add r0.zw, r1.xxxy, cb0[6].zzzy
    r0.zw = ((r1.xxxy)+(source[6].zzzy)).zw;
    // 50: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 51: mad r0.xy, cb0[15].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[15].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (SDNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 53: mad r0.y, v4.y, l(2.000000), l(-1.000000)
    r0.y = ((v4.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 54: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 55: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 56: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 57: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 58: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 59: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 60: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 61: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 62: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 63: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 64: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 65: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 66: mul r0.z, r0.z, cb0[14].w
    r0.z = ((r0.zzzz)*(source[14].wwww)).z;
    // 67: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 68: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 69: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 70: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 71: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 72: mul r0.x, r0.x, cb0[16].y
    r0.x = ((r0.xxxx)*(source[16].yyyy)).x;
    // 73: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 74: mul_sat r0.x, r0.x, cb0[16].z
    r0.x = (saturate((r0.xxxx)*(source[16].zzzz))).x;
    // 75: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 76: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

// fx_s_pa_trail_03_01_tr: 1946dbc9412ed54793435682a2c4fd1b; selected map 9ac0b942df1b47467d9f5e2ac2e1b67fab5086c7d944b608589a0521cf39c9f2.
float4 SDNative395(SD_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[4u];
    source[2] = SDNativeAppend(g_SDSourceMaterialParameters[2u].xxxx,g_SDSourceMaterialParameters[2u].yyyy,1u);
    source[3] = SDNativeAppend(cos((g_SDSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = SDNativeAppend(sin((g_SDSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5].x = (cos((g_SDSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[5].z = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[5].w = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[6].z = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[6].w = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[7].x = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[7].y = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[7].w = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[8].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_SDSourceMaterialParameters[0u].zzzz)).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_SDSourceMaterialParameters[0u].zzzz))).x;
    source[8].z = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].xxxx)).x;
    source[9].x = (max((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[9].w = (g_SDSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 5: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 6: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 7: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).x;
    // 8: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 9: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).x;
    // 10: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 11: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 12: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).x;
    // 13: lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.y, r0.x
    r1.x = (max(r0.yyyy,r0.xxxx)).x;
    // 22: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 24: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 25: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 26: mad r0.z, r0.y, l(0.318310), l(1.000000)
    r0.z = ((r0.yyyy)*(float4(asfloat(0x3ea2f986u),asfloat(0x3ea2f986u),asfloat(0x3ea2f986u),asfloat(0x3ea2f986u)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).z;
    // 27: mad r0.y, -r0.y, l(0.318310), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(asfloat(0x3ea2f986u),asfloat(0x3ea2f986u),asfloat(0x3ea2f986u),asfloat(0x3ea2f986u)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).y;
    // 28: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: mul r1.y, r0.z, cb0[6].y
    r1.y = ((r0.zzzz)*(source[6].yyyy)).y;
    // 30: mul r1.x, r0.z, cb0[5].w
    r1.x = ((r0.zzzz)*(source[5].wwww)).x;
    // 31: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 32: mul r0.z, r0.z, l(0.800000)
    r0.z = ((r0.zzzz)*(float4(0.800000,0.800000,0.800000,0.800000))).z;
    // 33: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 34: lt r0.w, r0.x, l(0.000001)
    r0.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 35: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 36: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 37: mul r1.zw, r0.zzzz, cb0[6].xxxz
    r1.zw = ((r0.zzzz)*(source[6].xxxz)).zw;
    // 38: mad r0.zw, v4.zzzz, l(0.000000, 0.000000, 0.800000, -0.400000), r1.yyyw
    r0.zw = ((v4.zzzz)*(float4(0.000000,0.000000,0.800000,-0.400000))+(r1.yyyw)).zw;
    // 39: mad r1.yw, v4.zzzz, l(0.000000, 0.500000, 0.000000, -0.200000), r1.yyyw
    r1.yw = ((v4.zzzz)*(float4(0.000000,0.500000,0.000000,-0.200000))+(r1.yyyw)).yw;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r1.ywyy, t0.yxzw, s0, l(0.000000)
    r1.y = (SDNativeSample0((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (SDNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: add r0.z, r1.y, r0.z
    r0.z = ((r1.yyyy)+(r0.zzzz)).z;
    // 43: mul r0.w, r1.y, cb0[6].w
    r0.w = ((r1.yyyy)*(source[6].wwww)).w;
    // 44: mad r1.yz, r0.wwww, v4.wwww, r1.xxzx
    r1.yz = ((r0.wwww)*(v4.wwww)+(r1.xxzx)).yz;
    // 45: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 46: mad r1.xw, v4.zzzz, l(0.400000, 0.000000, 0.000000, -0.600000), r1.yyyz
    r1.xw = ((v4.zzzz)*(float4(0.400000,0.000000,0.000000,-0.600000))+(r1.yyyz)).xw;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xwxx, t2.yzwx, s2, l(0.000000)
    r0.w = (SDNativeSample2((r1.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 48: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 49: mad r0.z, r0.z, l(0.500000), r0.w
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.wwww)).z;
    // 50: mad r1.x, -r0.x, l(2.000000), l(1.000000)
    r1.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 51: mad r0.x, -r0.x, cb0[8].y, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[8].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mul_sat r0.x, r0.x, cb0[9].y
    r0.x = (saturate((r0.xxxx)*(source[9].yyyy))).x;
    // 53: mul r0.x, r0.x, cb0[9].z
    r0.x = ((r0.xxxx)*(source[9].zzzz)).x;
    // 54: mul r1.x, r1.x, l(0.666667)
    r1.x = ((r1.xxxx)*(float4(asfloat(0x3f2aaaabu),asfloat(0x3f2aaaabu),asfloat(0x3f2aaaabu),asfloat(0x3f2aaaabu)))).x;
    // 55: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 56: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 57: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 58: add r1.xw, v4.xxxy, l(-1.000000, 0.000000, 0.000000, -1.000000)
    r1.xw = ((v4.xxxy)+(float4(-1.000000,0.000000,0.000000,-1.000000))).xw;
    // 59: mad r0.y, r0.z, r0.y, -r1.w
    r0.y = ((r0.zzzz)*(r0.yyyy)+(-(r1.wwww))).y;
    // 60: mad r1.xy, r1.xxxx, l(0.500000, -0.400000, 0.000000, 0.000000), r1.yzyy
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.400000,0.000000,0.000000))+(r1.yzyy)).xy;
    // 61: add r1.xy, r1.xyxx, cb0[2].xyxx
    r1.xy = ((r1.xyxx)+(source[2].xyxx)).xy;
    // 62: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 63: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 64: dp2 r2.x, cb0[3].xyxx, r1.xyxx
    r2.x = (dot((source[3].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 65: dp2 r2.y, cb0[4].xyxx, r1.xyxx
    r2.y = (dot((source[4].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 66: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s1, l(0.000000)
    r0.z = (SDNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 68: mul_sat r1.x, r0.z, l(20.000000)
    r1.x = (saturate((r0.zzzz)*(float4(20.000000,20.000000,20.000000,20.000000)))).x;
    // 69: mad_sat r0.x, r1.x, r0.y, -r0.x
    r0.x = (saturate((r1.xxxx)*(r0.yyyy)+(-(r0.xxxx)))).x;
    // 70: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 71: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 72: mul r0.x, r0.w, r0.z
    r0.x = ((r0.wwww)*(r0.zzzz)).x;
    // 73: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 74: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 75: mul r0.y, r0.y, cb0[7].x
    r0.y = ((r0.yyyy)*(source[7].xxxx)).y;
    // 76: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 77: mul r0.y, r0.y, cb0[7].y
    r0.y = ((r0.yyyy)*(source[7].yyyy)).y;
    // 78: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 79: mad r0.x, r0.z, cb0[7].z, r0.x
    r0.x = ((r0.zzzz)*(source[7].zzzz)+(r0.xxxx)).x;
    // 80: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 81: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_pa_trail_01_6_tr: 045a98e64142ef478d38469879c97683; selected map 6871052ff4f0cd7123bb61bdeeaf7aae296ac86538beef9601ca92ccc20fbde5.
float4 SDNative396(SD_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[4u];
    source[2] = SDNativeAppend(g_SDSourceMaterialParameters[2u].yyyy,g_SDSourceMaterialParameters[2u].zzzz,1u);
    source[3] = SDNativeAppend(cos((g_SDSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = SDNativeAppend(sin((g_SDSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5].x = (cos((g_SDSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[5].z = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[5].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[6].z = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[6].w = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[7].x = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[7].y = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[7].z = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[8].x = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[8].y = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[8].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_SDSourceMaterialParameters[0u].zzzz)).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_SDSourceMaterialParameters[0u].zzzz))).x;
    source[9].x = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].xxxx)).x;
    source[9].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_SDSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[10].x = (g_SDSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 5: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 6: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 7: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).x;
    // 8: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 9: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).x;
    // 10: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 11: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 12: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).x;
    // 13: lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.y, r0.x
    r1.x = (max(r0.yyyy,r0.xxxx)).x;
    // 22: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 24: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 25: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 26: mad r0.z, r0.y, l(0.318310), l(1.000000)
    r0.z = ((r0.yyyy)*(float4(asfloat(0x3ea2f986u),asfloat(0x3ea2f986u),asfloat(0x3ea2f986u),asfloat(0x3ea2f986u)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).z;
    // 27: mad r0.y, -r0.y, l(0.318310), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(asfloat(0x3ea2f986u),asfloat(0x3ea2f986u),asfloat(0x3ea2f986u),asfloat(0x3ea2f986u)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).y;
    // 28: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 30: mul r0.w, r0.w, l(0.800000)
    r0.w = ((r0.wwww)*(float4(0.800000,0.800000,0.800000,0.800000))).w;
    // 31: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 32: lt r2.x, r0.x, l(0.000001)
    r2.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 33: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 34: movc r0.w, r2.x, l(0), r0.w
    r0.w = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 35: mul r1.yzw, r0.zzww, cb0[6].yyxz
    r1.yzw = ((r0.zzww)*(source[6].yyxz)).yzw;
    // 36: mul r2.y, r0.w, cb0[7].y
    r2.y = ((r0.wwww)*(source[7].yyyy)).y;
    // 37: mad r2.zw, v4.zzzz, l(0.000000, 0.000000, 0.800000, -0.400000), r1.yyyw
    r2.zw = ((v4.zzzz)*(float4(0.000000,0.000000,0.800000,-0.400000))+(r1.yyyw)).zw;
    // 38: mad r1.yw, v4.zzzz, l(0.000000, 0.500000, 0.000000, -0.200000), r1.yyyw
    r1.yw = ((v4.zzzz)*(float4(0.000000,0.500000,0.000000,-0.200000))+(r1.yyyw)).yw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.ywyy, t0.yzwx, s0, l(0.000000)
    r0.w = (SDNativeSample0((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.zwzz, t0.yxzw, s0, l(0.000000)
    r1.y = (SDNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 41: add r1.y, r0.w, r1.y
    r1.y = ((r0.wwww)+(r1.yyyy)).y;
    // 42: mul r0.w, r0.w, cb0[6].w
    r0.w = ((r0.wwww)*(source[6].wwww)).w;
    // 43: mul r2.x, r0.z, cb0[7].x
    r2.x = ((r0.zzzz)*(source[7].xxxx)).x;
    // 44: mul r1.x, r0.z, cb0[5].w
    r1.x = ((r0.zzzz)*(source[5].wwww)).x;
    // 45: mad r2.xy, r0.wwww, v4.wwww, r2.xyxx
    r2.xy = ((r0.wwww)*(v4.wwww)+(r2.xyxx)).xy;
    // 46: mad r0.zw, r0.wwww, v4.wwww, r1.xxxz
    r0.zw = ((r0.wwww)*(v4.wwww)+(r1.xxxz)).zw;
    // 47: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 48: mad r1.xz, v4.zzzz, l(0.400000, 0.000000, -0.600000, 0.000000), r2.xxyx
    r1.xz = ((v4.zzzz)*(float4(0.400000,0.000000,-0.600000,0.000000))+(r2.xxyx)).xz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xzxx, t2.xyzw, s2, l(0.000000)
    r1.x = (SDNativeSample2((r1.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 50: add r1.y, -r1.x, r1.y
    r1.y = ((-(r1.xxxx))+(r1.yyyy)).y;
    // 51: mad r1.y, r1.y, l(0.500000), r1.x
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r1.xxxx)).y;
    // 52: mad r1.z, -r0.x, l(2.000000), l(1.000000)
    r1.z = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 53: mad r0.x, -r0.x, cb0[8].w, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[8].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: mul_sat r0.x, r0.x, cb0[9].w
    r0.x = (saturate((r0.xxxx)*(source[9].wwww))).x;
    // 55: mul r0.x, r0.x, cb0[10].x
    r0.x = ((r0.xxxx)*(source[10].xxxx)).x;
    // 56: mul r1.z, r1.z, l(0.666667)
    r1.z = ((r1.zzzz)*(float4(asfloat(0x3f2aaaabu),asfloat(0x3f2aaaabu),asfloat(0x3f2aaaabu),asfloat(0x3f2aaaabu)))).z;
    // 57: max r1.z, r1.z, l(0.000000)
    r1.z = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 58: mul r0.y, r0.y, r1.z
    r0.y = ((r0.yyyy)*(r1.zzzz)).y;
    // 59: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 60: add r1.zw, v4.xxxy, l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 61: mad r0.y, r1.y, r0.y, -r1.w
    r0.y = ((r1.yyyy)*(r0.yyyy)+(-(r1.wwww))).y;
    // 62: mad r0.zw, r1.zzzz, l(0.000000, 0.000000, 0.500000, -0.400000), r0.zzzw
    r0.zw = ((r1.zzzz)*(float4(0.000000,0.000000,0.500000,-0.400000))+(r0.zzzw)).zw;
    // 63: add r0.zw, r0.zzzw, cb0[2].xxxy
    r0.zw = ((r0.zzzw)+(source[2].xxxy)).zw;
    // 64: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 65: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 66: dp2 r2.x, cb0[3].xyxx, r0.zwzz
    r2.x = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 67: dp2 r2.y, cb0[4].xyxx, r0.zwzz
    r2.y = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 68: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (SDNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 70: mul_sat r0.w, r0.z, l(20.000000)
    r0.w = (saturate((r0.zzzz)*(float4(20.000000,20.000000,20.000000,20.000000)))).w;
    // 71: mad_sat r0.x, r0.w, r0.y, -r0.x
    r0.x = (saturate((r0.wwww)*(r0.yyyy)+(-(r0.xxxx)))).x;
    // 72: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 73: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 74: mul r0.x, r1.x, r0.z
    r0.x = ((r1.xxxx)*(r0.zzzz)).x;
    // 75: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 76: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 77: mul r0.y, r0.y, cb0[7].z
    r0.y = ((r0.yyyy)*(source[7].zzzz)).y;
    // 78: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 79: mul r0.y, r0.y, cb0[7].w
    r0.y = ((r0.yyyy)*(source[7].wwww)).y;
    // 80: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 81: mad r0.x, r0.z, cb0[8].x, r0.x
    r0.x = ((r0.zzzz)*(source[8].xxxx)+(r0.xxxx)).x;
    // 82: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 83: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_j_pa_utillight_01_01_tr: 62a18b81a515194191cd80d15c327174; selected map 6b4ec748b41d4df685850ac67d57e243d38d90ad8739b06f9433adae4f3590e8.
float4 SDNative397(SD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[2u];
    source[2].x = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_SDSourceMaterialParameters[1u].xxxx).x;
    source[2].w = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[3].x = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[3].y = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[3].z = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[3].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: max r1.x, |r0.w|, |r0.z|
    r1.x = (max(abs(r0.wwww),abs(r0.zzzz))).x;
    // 3: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 4: min r1.y, |r0.w|, |r0.z|
    r1.y = (min(abs(r0.wwww),abs(r0.zzzz))).y;
    // 5: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 6: mul r1.y, r1.x, r1.x
    r1.y = ((r1.xxxx)*(r1.xxxx)).y;
    // 7: mad r1.z, r1.y, l(0.020835), l(-0.085133)
    r1.z = ((r1.yyyy)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).z;
    // 8: mad r1.z, r1.y, r1.z, l(0.180141)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 9: mad r1.z, r1.y, r1.z, l(-0.330299)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).z;
    // 10: mad r1.y, r1.y, r1.z, l(0.999866)
    r1.y = ((r1.yyyy)*(r1.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).y;
    // 11: mul r1.z, r1.y, r1.x
    r1.z = ((r1.yyyy)*(r1.xxxx)).z;
    // 12: mad r1.z, r1.z, l(-2.000000), l(1.570796)
    r1.z = ((r1.zzzz)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).z;
    // 13: lt r2.xy, |r0.wzww|, |r0.zwzz|
    r2.xy = (asfloat((uint4)((abs(r0.wzww))<(abs(r0.zwzz))) * 0xffffffffu)).xy;
    // 14: and r1.zw, r1.zzzz, r2.xxxy
    r1.zw = (asfloat(asuint(r1.zzzz) & asuint(r2.xxxy))).zw;
    // 15: mad r1.xy, r1.xxxx, r1.yyyy, r1.zwzz
    r1.xy = ((r1.xxxx)*(r1.yyyy)+(r1.zwzz)).xy;
    // 16: lt r1.zw, r0.wwwz, -r0.wwwz
    r1.zw = (asfloat((uint4)((r0.wwwz)<(-(r0.wwwz))) * 0xffffffffu)).zw;
    // 17: and r1.zw, r1.zzzw, l(0, 0, 0xc0490fdb, 0xc0490fdb)
    r1.zw = (asfloat(asuint(r1.zzzw) & uint4(0u,0u,0xc0490fdbu,0xc0490fdbu))).zw;
    // 18: add r1.xy, r1.zwzz, r1.xyxx
    r1.xy = ((r1.zwzz)+(r1.xyxx)).xy;
    // 19: min r1.z, r0.w, r0.z
    r1.z = (min(r0.wwww,r0.zzzz)).z;
    // 20: lt r1.z, r1.z, -r1.z
    r1.z = (asfloat((uint4)((r1.zzzz)<(-(r1.zzzz))) * 0xffffffffu)).z;
    // 21: max r1.w, r0.w, r0.z
    r1.w = (max(r0.wwww,r0.zzzz)).w;
    // 22: ge r1.w, r1.w, -r1.w
    r1.w = (asfloat((uint4)((r1.wwww)>=(-(r1.wwww))) * 0xffffffffu)).w;
    // 23: and r1.z, r1.w, r1.z
    r1.z = (asfloat(asuint(r1.wwww) & asuint(r1.zzzz))).z;
    // 24: movc r1.xy, r1.zzzz, -r1.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.zzzz) != 0u) ? (-(r1.xyxx)) : (r1.xyxx)).xy;
    // 25: mul r1.y, r1.y, l(0.159155)
    r1.y = ((r1.yyyy)*(float4(asfloat(0x3e22f984u),asfloat(0x3e22f984u),asfloat(0x3e22f984u),asfloat(0x3e22f984u)))).y;
    // 26: mad r1.x, r1.x, l(0.318310), l(1.000000)
    r1.x = ((r1.xxxx)*(float4(asfloat(0x3ea2f986u),asfloat(0x3ea2f986u),asfloat(0x3ea2f986u),asfloat(0x3ea2f986u)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 27: frc r1.y, r1.y
    r1.y = (frac(r1.yyyy)).y;
    // 28: mov r2.xz, l(0,0,0,0)
    r2.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 29: mul r2.yw, v4.xxxx, l(0.000000, -0.100000, 0.000000, 0.100000)
    r2.yw = ((v4.xxxx)*(float4(0.000000,-0.100000,0.000000,0.100000))).yw;
    // 30: add r1.yz, r1.yyyy, r2.xxyx
    r1.yz = ((r1.yyyy)+(r2.xxyx)).yz;
    // 31: sample_l_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t2.yxzw, s2, l(-1.000000)
    r1.y = (SDNativeSample1((r1.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yxzw).y;
    // 32: mul r1.y, r1.y, cb0[2].z
    r1.y = ((r1.yyyy)*(source[2].zzzz)).y;
    // 33: lt r1.z, |r1.y|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 34: log r1.y, |r1.y|
    r1.y = (log2(abs(r1.yyyy))).y;
    // 35: add r1.w, v4.y, cb0[2].w
    r1.w = ((v4.yyyy)+(source[2].wwww)).w;
    // 36: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 37: mul r1.y, r1.y, r1.w
    r1.y = ((r1.yyyy)*(r1.wwww)).y;
    // 38: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 39: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 40: dp2 r1.z, r0.zwzz, r0.zwzz
    r1.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 41: mul r0.xyzw, r0.xyzw, r0.xyzw
    r0.xyzw = ((r0.xyzw)*(r0.xyzw)).xyzw;
    // 42: add r0.xy, r0.ywyy, r0.xzxx
    r0.xy = ((r0.ywyy)+(r0.xzxx)).xy;
    // 43: mul r0.zw, r1.zzzz, l(0.000000, 0.000000, 3.750000, 40.000000)
    r0.zw = ((r1.zzzz)*(float4(0.000000,0.000000,3.750000,40.000000))).zw;
    // 44: mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // 45: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 1.442545, 1.442545)
    r0.zw = ((r0.zzzw)*(float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x3fb8a554u),asfloat(0x3fb8a554u)))).zw;
    // 46: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 47: div r0.z, l(0.950000), r0.z
    r0.z = ((float4(0.950000,0.950000,0.950000,0.950000))/(r0.zzzz)).z;
    // 48: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 49: lt r2.x, l(0.000000), r1.z
    r2.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))<(r1.zzzz)) * 0xffffffffu)).x;
    // 50: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 51: movc r0.zw, r2.xxxx, r0.zzzw, l(0,0,1.000000,1.000000)
    r0.zw = ((asuint(r2.xxxx) != 0u) ? (r0.zzzw) : (float4(asfloat(0u),asfloat(0u),1.000000,1.000000))).zw;
    // 52: add r0.w, r0.z, r0.w
    r0.w = ((r0.zzzz)+(r0.wwww)).w;
    // 53: mul r2.x, r1.y, r0.z
    r2.x = ((r1.yyyy)*(r0.zzzz)).x;
    // 54: mad r0.z, r0.z, r1.y, l(-0.100000)
    r0.z = ((r0.zzzz)*(r1.yyyy)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).z;
    // 55: mad r0.z, r2.x, r0.z, l(0.100000)
    r0.z = ((r2.xxxx)*(r0.zzzz)+(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 56: lt r1.y, r0.w, l(0.000001)
    r1.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 57: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 58: movc r0.w, r1.y, l(0), r0.w
    r0.w = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 59: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 60: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 61: add r2.xy, r0.yyyy, r2.zwzz
    r2.xy = ((r0.yyyy)+(r2.zwzz)).xy;
    // 62: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r2.xyxx, t2.yxzw, s2, l(-1.000000)
    r0.y = (SDNativeSample1((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yxzw).y;
    // 63: mul r0.y, r0.y, cb0[2].z
    r0.y = ((r0.yyyy)*(source[2].zzzz)).y;
    // 64: log r1.y, |r0.y|
    r1.y = (log2(abs(r0.yyyy))).y;
    // 65: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 66: mul r1.y, r1.y, r1.w
    r1.y = ((r1.yyyy)*(r1.wwww)).y;
    // 67: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 68: movc r0.y, r0.y, l(0), r1.y
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 69: mad r1.y, r0.w, r0.y, -r0.z
    r1.y = ((r0.wwww)*(r0.yyyy)+(-(r0.zzzz))).y;
    // 70: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 71: mad r0.y, r0.y, r1.y, r0.z
    r0.y = ((r0.yyyy)*(r1.yyyy)+(r0.zzzz)).y;
    // 72: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 73: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 74: mul r0.z, r0.z, cb0[3].x
    r0.z = ((r0.zzzz)*(source[3].xxxx)).z;
    // 75: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 76: mul r2.xyz, r0.zzzz, v3.xyzx
    r2.xyz = ((r0.zzzz)*(v3.xyzx)).xyz;
    // 77: movc r0.yzw, r0.yyyy, l(0,0,0,0), r2.xxyz
    r0.yzw = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyz)).yzw;
    // 78: sqrt r1.y, r1.z
    r1.y = (sqrt(r1.zzzz)).y;
    // 79: mul r1.z, r1.z, r1.y
    r1.z = ((r1.zzzz)*(r1.yyyy)).z;
    // 80: lt r1.y, r1.y, l(0.000001)
    r1.y = (asfloat((uint4)((r1.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 81: mul r1.z, r1.z, l(0.100000)
    r1.z = ((r1.zzzz)*(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 82: movc r2.y, r1.y, l(0), r1.z
    r2.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 83: mul r1.y, v4.z, cb0[2].x
    r1.y = ((v4.zzzz)*(source[2].xxxx)).y;
    // 84: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 85: mul r2.x, r1.x, l(0.500000)
    r2.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 86: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t1.xyzw, s1, l(-1.000000)
    r1.xyz = (SDNativeSample0((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 87: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 88: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 89: mad r1.xyz, cb0[2].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[2].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 90: mad r0.yzw, r1.xxyz, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r1.xxyz)*(r0.yyzw)+(source[1].xxyz)).yzw;
    // 91: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 92: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 93: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 94: source device depth mapped to centimetre view depth; reconstruction at 96.
    r1.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 96-99: reconstructed view depth is supplied by the runtime adapter.
    r1.x = r1.x;
    // 100: add r1.x, r1.x, -v7.w
    r1.x = ((r1.xxxx)+(-(v7.wwww))).x;
    // 101: add r1.y, -cb0[3].w, l(1.000000)
    r1.y = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 102: mul r1.y, r1.y, l(100.000000)
    r1.y = ((r1.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 103: max r1.y, r1.y, l(0.001000)
    r1.y = (max(r1.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 104: div_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)/(r1.yyyy))).x;
    // 105: log r1.y, |r0.x|
    r1.y = (log2(abs(r0.xxxx))).y;
    // 106: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 107: mad r1.z, cb0[3].y, l(10.000000), l(10.000000)
    r1.z = ((source[3].yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).z;
    // 108: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 109: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 110: mul r1.y, r1.y, cb0[3].z
    r1.y = ((r1.yyyy)*(source[3].zzzz)).y;
    // 111: mul r1.y, r1.y, v3.w
    r1.y = ((r1.yyyy)*(v3.wwww)).y;
    // 112: mul_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)*(r1.yyyy))).x;
    // 113: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 114: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 115: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 116: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_j_pa_uvcoreoffset_01_06_tr: ad382d96296ad04fae3874797093fdee; selected map 30b0028d2c136e12f1bb3f56d35b9388dddf23a728c098840505c75e40fe795a.
float4 SDNative399(SD_NATIVE_INPUT input)
{
    float4 source[26]; [unroll] for (uint i=0u; i<26u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[12u];
    source[2] = SDNativeAppend(g_SDSourceMaterialParameters[5u].zzzz,g_SDSourceMaterialParameters[5u].wwww,1u);
    source[3] = (SDNativeAppend((g_SDSourceMaterialParameters[4u].wwww*g_SDSourceMaterialTime.xxxx),(g_SDSourceMaterialParameters[5u].xxxx*g_SDSourceMaterialTime.xxxx),1u)+SDNativeAppend(g_SDSourceMaterialParameters[4u].yyyy,g_SDSourceMaterialParameters[4u].zzzz,1u));
    source[4] = SDNativeAppend(cos((g_SDSourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = SDNativeAppend(sin((g_SDSourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = SDNativeAppend(g_SDSourceMaterialParameters[10u].zzzz,g_SDSourceMaterialParameters[10u].wwww,1u);
    source[7] = (SDNativeAppend((g_SDSourceMaterialParameters[9u].zzzz*g_SDSourceMaterialTime.xxxx),(g_SDSourceMaterialParameters[9u].wwww*g_SDSourceMaterialTime.xxxx),1u)+SDNativeAppend(g_SDSourceMaterialParameters[9u].xxxx,g_SDSourceMaterialParameters[9u].yyyy,1u));
    source[8] = SDNativeAppend(cos((g_SDSourceMaterialParameters[10u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[10u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = SDNativeAppend(sin((g_SDSourceMaterialParameters[10u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[10u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10] = SDNativeAppend(g_SDSourceMaterialParameters[3u].zzzz,g_SDSourceMaterialParameters[3u].wwww,1u);
    source[11] = SDNativeAppend(cos((g_SDSourceMaterialParameters[3u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[3u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[12] = SDNativeAppend(sin((g_SDSourceMaterialParameters[3u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[3u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[13] = g_SDSourceMaterialParameters[11u];
    source[14] = SDNativeAppend(g_SDSourceMaterialParameters[7u].zzzz,g_SDSourceMaterialParameters[7u].wwww,1u);
    source[15] = SDNativeAppend(cos((g_SDSourceMaterialParameters[7u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[7u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[16] = SDNativeAppend(sin((g_SDSourceMaterialParameters[7u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialParameters[7u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[17].x = ((g_SDSourceMaterialParameters[9u].zzzz*g_SDSourceMaterialTime.xxxx)).x;
    source[17].y = (g_SDSourceMaterialParameters[9u].yyyy).x;
    source[17].z = (g_SDSourceMaterialParameters[9u].xxxx).x;
    source[17].w = (g_SDSourceMaterialParameters[10u].yyyy).x;
    source[18].x = (g_SDSourceMaterialParameters[2u].xxxx).x;
    source[18].y = (g_SDSourceMaterialParameters[3u].xxxx).x;
    source[18].z = ((g_SDSourceMaterialParameters[3u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[18].w = (sin((g_SDSourceMaterialParameters[3u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialParameters[3u].xxxx*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[19].y = (cos((g_SDSourceMaterialParameters[3u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].z = (g_SDSourceMaterialParameters[1u].zzzz).x;
    source[19].w = (g_SDSourceMaterialParameters[2u].yyyy).x;
    source[20].x = (g_SDSourceMaterialParameters[1u].wwww).x;
    source[20].y = (g_SDSourceMaterialParameters[2u].zzzz).x;
    source[20].z = (g_SDSourceMaterialParameters[3u].wwww).x;
    source[20].w = (g_SDSourceMaterialParameters[3u].zzzz).x;
    source[21].x = (g_SDSourceMaterialParameters[2u].wwww).x;
    source[21].y = (g_SDSourceMaterialParameters[3u].yyyy).x;
    source[21].z = (g_SDSourceMaterialParameters[1u].yyyy).x;
    source[21].w = (g_SDSourceMaterialParameters[6u].xxxx).x;
    source[22].x = (cos((g_SDSourceMaterialParameters[7u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[22].y = (g_SDSourceMaterialParameters[6u].yyyy).x;
    source[22].z = (g_SDSourceMaterialParameters[6u].wwww).x;
    source[22].w = (g_SDSourceMaterialParameters[6u].zzzz).x;
    source[23].x = (g_SDSourceMaterialParameters[7u].xxxx).x;
    source[23].y = (g_SDSourceMaterialParameters[7u].wwww).x;
    source[23].z = (g_SDSourceMaterialParameters[7u].zzzz).x;
    source[23].w = (g_SDSourceMaterialParameters[8u].xxxx).x;
    source[24].x = (g_SDSourceMaterialParameters[8u].yyyy).x;
    source[24].y = (g_SDSourceMaterialParameters[8u].zzzz).x;
    source[24].z = (g_SDSourceMaterialParameters[8u].wwww).x;
    source[24].w = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[25].x = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[25].y = (g_SDSourceMaterialParameters[0u].wwww).x;
    source[25].z = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[25].w = (g_SDSourceMaterialParameters[1u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r0.y, r0.xyxx, r0.xyxx
    r0.y = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 3: sqrt r0.z, r0.y
    r0.z = (sqrt(r0.yyyy)).z;
    // 4: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 5: div r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)/(r0.zzzz)).x;
    // 6: mad r1.y, -r0.z, l(2.000000), l(1.000000)
    r1.y = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 7: mad r0.z, |r0.x|, l(-0.018729), l(0.074261)
    r0.z = ((abs(r0.xxxx))*(float4(asfloat(0xbc996e30u),asfloat(0xbc996e30u),asfloat(0xbc996e30u),asfloat(0xbc996e30u)))+(float4(asfloat(0x3d981627u),asfloat(0x3d981627u),asfloat(0x3d981627u),asfloat(0x3d981627u)))).z;
    // 8: mad r0.z, r0.z, |r0.x|, l(-0.212114)
    r0.z = ((r0.zzzz)*(abs(r0.xxxx))+(float4(asfloat(0xbe593484u),asfloat(0xbe593484u),asfloat(0xbe593484u),asfloat(0xbe593484u)))).z;
    // 9: mad r0.z, r0.z, |r0.x|, l(1.570729)
    r0.z = ((r0.zzzz)*(abs(r0.xxxx))+(float4(asfloat(0x3fc90da4u),asfloat(0x3fc90da4u),asfloat(0x3fc90da4u),asfloat(0x3fc90da4u)))).z;
    // 10: add r0.w, -|r0.x|, l(1.000000)
    r0.w = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 11: lt r0.x, r0.x, -r0.x
    r0.x = (asfloat((uint4)((r0.xxxx)<(-(r0.xxxx))) * 0xffffffffu)).x;
    // 12: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 13: mul r1.z, r0.w, r0.z
    r1.z = ((r0.wwww)*(r0.zzzz)).z;
    // 14: mad r1.z, r1.z, l(-2.000000), l(3.141593)
    r1.z = ((r1.zzzz)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x40490fdbu),asfloat(0x40490fdbu),asfloat(0x40490fdbu),asfloat(0x40490fdbu)))).z;
    // 15: and r0.x, r0.x, r1.z
    r0.x = (asfloat(asuint(r0.xxxx) & asuint(r1.zzzz))).x;
    // 16: mad r0.x, r0.z, r0.w, r0.x
    r0.x = ((r0.zzzz)*(r0.wwww)+(r0.xxxx)).x;
    // 17: mul r0.x, r0.x, l(0.159155)
    r0.x = ((r0.xxxx)*(float4(asfloat(0x3e22f986u),asfloat(0x3e22f986u),asfloat(0x3e22f986u),asfloat(0x3e22f986u)))).x;
    // 18: frc r0.x, r0.x
    r0.x = (frac(r0.xxxx)).x;
    // 19: mad r0.z, -r0.x, l(2.000000), l(1.000000)
    r0.z = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 20: ge r0.w, v2.y, l(0.500000)
    r0.w = (asfloat((uint4)((v2.yyyy)>=(float4(0.500000,0.500000,0.500000,0.500000))) * 0xffffffffu)).w;
    // 21: movc r0.w, r0.w, l(0), l(1.000000)
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 22: mad r1.x, r0.z, r0.w, r0.x
    r1.x = ((r0.zzzz)*(r0.wwww)+(r0.xxxx)).x;
    // 23: mad r0.xz, r1.xxyx, cb0[6].xxyx, cb0[7].xxyx
    r0.xz = ((r1.xxyx)*(source[6].xxyx)+(source[7].xxyx)).xz;
    // 24: add r0.xz, r0.xxzx, l(-0.500000, 0.000000, -0.500000, 0.000000)
    r0.xz = ((r0.xxzx)+(float4(-0.500000,0.000000,-0.500000,0.000000))).xz;
    // 25: dp2 r2.x, cb0[8].xyxx, r0.xzxx
    r2.x = (dot((source[8].xyxx).xy,(r0.xzxx).xy).xxxx).x;
    // 26: dp2 r2.y, cb0[9].xyxx, r0.xzxx
    r2.y = (dot((source[9].xyxx).xy,(r0.xzxx).xy).xxxx).y;
    // 27: add r0.xz, r2.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r0.xz = ((r2.xxyx)+(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 28: sample_l_indexable(texture2d)(float,float,float,float) r0.xz, r0.xzxx, t0.xzyw, s2, l(-1.000000)
    r0.xz = (SDNativeSample2((r0.xzxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xzyw).xz;
    // 29: mad r1.zw, r1.xxxy, cb0[2].xxxy, cb0[3].xxxy
    r1.zw = ((r1.xxxy)*(source[2].xxxy)+(source[3].xxxy)).zw;
    // 30: add r1.zw, r1.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r1.zw = ((r1.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 31: dp2 r2.x, cb0[4].xyxx, r1.zwzz
    r2.x = (dot((source[4].xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 32: dp2 r2.y, cb0[5].xyxx, r1.zwzz
    r2.y = (dot((source[5].xyxx).xy,(r1.zwzz).xy).xxxx).y;
    // 33: add r1.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 34: mad r0.xz, cb0[17].wwww, r0.xxzx, r1.zzwz
    r0.xz = ((source[17].wwww)*(r0.xxzx)+(r1.zzwz)).xz;
    // 35: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t1.xyzw, s1, l(-1.000000)
    r0.x = (SDNativeSample1((r0.xzxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 36: mul r0.x, r0.x, v4.y
    r0.x = ((r0.xxxx)*(v4.yyyy)).x;
    // 37: mad r2.x, cb0[19].z, v4.z, cb0[19].w
    r2.x = ((source[19].zzzz)*(v4.zzzz)+(source[19].wwww)).x;
    // 38: mad r2.y, cb0[20].x, v4.z, cb0[20].y
    r2.y = ((source[20].xxxx)*(v4.zzzz)+(source[20].yyyy)).y;
    // 39: mad r0.zw, r1.xxxy, cb0[10].xxxy, r2.xxxy
    r0.zw = ((r1.xxxy)*(source[10].xxxy)+(r2.xxxy)).zw;
    // 40: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 41: dp2 r2.x, cb0[11].xyxx, r0.zwzz
    r2.x = (dot((source[11].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 42: dp2 r2.y, cb0[12].xyxx, r0.zwzz
    r2.y = (dot((source[12].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 43: mad r0.zw, r0.xxxx, cb0[18].xxxx, r2.xxxy
    r0.zw = ((r0.xxxx)*(source[18].xxxx)+(r2.xxxy)).zw;
    // 44: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 45: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t3.xyzw, s0, l(-1.000000)
    r2.xyz = (SDNativeSample0((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 46: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 47: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 48: mul r2.xyz, r2.xyzx, cb0[21].xxxx
    r2.xyz = ((r2.xyzx)*(source[21].xxxx)).xyz;
    // 49: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 50: mul r2.xyz, r2.xyzx, cb0[21].yyyy
    r2.xyz = ((r2.xyzx)*(source[21].yyyy)).xyz;
    // 51: mul r3.xyz, r2.xyzx, cb0[13].xyzx
    r3.xyz = ((r2.xyzx)*(source[13].xyzx)).xyz;
    // 52: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 53: mad r2.xyz, -r2.xyzx, cb0[13].xyzx, r0.zzzz
    r2.xyz = ((-(r2.xyzx))*(source[13].xyzx)+(r0.zzzz)).xyz;
    // 54: mad r2.xyz, cb0[21].zzzz, r2.xyzx, r3.xyzx
    r2.xyz = ((source[21].zzzz)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 55: mad r3.x, cb0[22].y, v4.x, cb0[22].z
    r3.x = ((source[22].yyyy)*(v4.xxxx)+(source[22].zzzz)).x;
    // 56: mad r3.y, cb0[22].w, v4.x, cb0[23].x
    r3.y = ((source[22].wwww)*(v4.xxxx)+(source[23].xxxx)).y;
    // 57: mad r0.zw, r1.xxxy, cb0[14].xxxy, r3.xxxy
    r0.zw = ((r1.xxxy)*(source[14].xxxy)+(r3.xxxy)).zw;
    // 58: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 59: dp2 r1.x, cb0[15].xyxx, r0.zwzz
    r1.x = (dot((source[15].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 60: dp2 r1.y, cb0[16].xyxx, r0.zwzz
    r1.y = (dot((source[16].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 61: mad r0.xz, r0.xxxx, cb0[21].wwww, r1.xxyx
    r0.xz = ((r0.xxxx)*(source[21].wwww)+(r1.xxyx)).xz;
    // 62: add r0.xz, r0.xxzx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r0.xz = ((r0.xxzx)+(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 63: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t2.xyzw, s3, l(-1.000000)
    r0.x = (SDNativeSample3((r0.xzxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 64: add r0.z, -r0.x, l(1.000000)
    r0.z = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 65: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 66: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 67: mul r0.w, r0.w, cb0[23].w
    r0.w = ((r0.wwww)*(source[23].wwww)).w;
    // 68: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 69: mul r0.w, r0.w, cb0[24].x
    r0.w = ((r0.wwww)*(source[24].xxxx)).w;
    // 70: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 71: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 72: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 73: mad r1.xy, cb0[24].ywyy, l(10.000000, 10.000000, 0.000000, 0.000000), l(10.000000, 10.000000, 0.000000, 0.000000)
    r1.xy = ((source[24].ywyy)*(float4(10.000000,10.000000,0.000000,0.000000))+(float4(10.000000,10.000000,0.000000,0.000000))).xy;
    // 74: mul r1.xy, r0.wwww, r1.xyxx
    r1.xy = ((r0.wwww)*(r1.xyxx)).xy;
    // 75: exp r1.xy, r1.xyxx
    r1.xy = (exp2(r1.xyxx)).xy;
    // 76: mul r1.y, r1.y, cb0[25].x
    r1.y = ((r1.yyyy)*(source[25].xxxx)).y;
    // 77: mul r1.x, r1.x, cb0[24].z
    r1.x = ((r1.xxxx)*(source[24].zzzz)).x;
    // 78: movc r1.xy, r0.yyyy, l(0,0,0,0), r1.xyxx
    r1.xy = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyxx)).xy;
    // 79: mad r1.y, r0.x, r1.x, r1.y
    r1.y = ((r0.xxxx)*(r1.xxxx)+(r1.yyyy)).y;
    // 80: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 81: mad r1.xyz, r0.zzzz, r1.yyyy, r2.xyzx
    r1.xyz = ((r0.zzzz)*(r1.yyyy)+(r2.xyzx)).xyz;
    // 82: mad r1.xyz, v3.xyzx, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((v3.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 83: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 84: mad r0.z, cb0[25].y, l(10.000000), l(10.000000)
    r0.z = ((source[25].yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).z;
    // 85: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 86: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 87: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 88: lt r0.z, r0.y, l(0.000001)
    r0.z = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 89: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 90: mul r0.y, r0.y, cb0[25].z
    r0.y = ((r0.yyyy)*(source[25].zzzz)).y;
    // 91: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 92: mad r0.y, -r0.y, cb0[25].w, l(1.000000)
    r0.y = ((-(r0.yyyy))*(source[25].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 93: movc r0.y, r0.z, l(1.000000), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.yyyy)).y;
    // 94: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 95: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 96: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

// fx_mm_onelayerdistortion_02_01_ad: eb2bcd5c8f3c6c49805ab689687b14f6; selected map 5cdc3f3921ce9c64ed1a2b7836cd323203952b794cb58c5f718e69ddfab95aee.
float4 SDNative374(SD_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    source[10].x=0.f; // Project neutral scene-attenuation adapter, NOT a recovered source engine default.
    // At zero distortion/emission, opaque OneLayer must preserve SceneColor: Scene*(1-x)=Scene forces x=0.
    float4 output=0.f;
    source[1] = g_SDSourceMaterialParameters[1u];
    source[2] = SDNativeAppend(SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),SDNativePeriodic((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[3] = SDNativeAppend(cos((g_SDSourceMaterialTime.xxxx*float4(0.699999988, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialTime.xxxx*float4(0.699999988, 0.0, 0.0, 0.0)))),1u);
    source[4] = SDNativeAppend(sin((g_SDSourceMaterialTime.xxxx*float4(0.699999988, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialTime.xxxx*float4(0.699999988, 0.0, 0.0, 0.0))),1u);
    source[5] = SDNativeAppend(cos((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))),1u);
    source[6] = SDNativeAppend(sin((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),cos((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[7].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialTime.xxxx*float4(0.699999988, 0.0, 0.0, 0.0))))).x;
    source[7].y = (cos((g_SDSourceMaterialTime.xxxx*float4(0.699999988, 0.0, 0.0, 0.0)))).x;
    source[7].z = (g_SDSourceMaterialParameters[0u].xxxx).x;
    source[7].w = (sin((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[8].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))))).x;
    source[8].y = (cos((g_SDSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[8].z = (g_SDSourceMaterialParameters[0u].yyyy).x;
    source[8].w = ((g_SDSourceMaterialParameters[0u].wwww*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[9].x = (g_SDSourceMaterialParameters[0u].zzzz).x;
    source[9].y = ((float4(10.0, 0.0, 0.0, 0.0)/g_SDSourceMaterialParameters[0u].zzzz)).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xy, v2.xyxx, l(0xbf000000,0xbf000000,0x00000000,0x00000000)
    r0.xy = ((v2.xyxx)+(float4(asfloat(0xbf000000u),asfloat(0xbf000000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 2: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 5: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 6: div r0.zw, r1.xxxy, r0.zzzz
    r0.zw = ((r1.xxxy)/(r0.zzzz)).zw;
    // 7: mad r1.x, |r0.z|, l(0xbc996e30), l(0x3d981627)
    r1.x = ((abs(r0.zzzz))*(float4(asfloat(0xbc996e30u),asfloat(0xbc996e30u),asfloat(0xbc996e30u),asfloat(0xbc996e30u)))+(float4(asfloat(0x3d981627u),asfloat(0x3d981627u),asfloat(0x3d981627u),asfloat(0x3d981627u)))).x;
    // 8: mad r1.x, r1.x, |r0.z|, l(0xbe593484)
    r1.x = ((r1.xxxx)*(abs(r0.zzzz))+(float4(asfloat(0xbe593484u),asfloat(0xbe593484u),asfloat(0xbe593484u),asfloat(0xbe593484u)))).x;
    // 9: mad r1.x, r1.x, |r0.z|, l(0x3fc90da4)
    r1.x = ((r1.xxxx)*(abs(r0.zzzz))+(float4(asfloat(0x3fc90da4u),asfloat(0x3fc90da4u),asfloat(0x3fc90da4u),asfloat(0x3fc90da4u)))).x;
    // 10: add r1.y, -|r0.z|, l(0x3f800000)
    r1.y = ((-(abs(r0.zzzz)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).y;
    // 11: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 12: mul r1.z, r1.y, r1.x
    r1.z = ((r1.yyyy)*(r1.xxxx)).z;
    // 13: mad r1.z, r1.z, l(0xc0000000), l(0x40490fdb)
    r1.z = ((r1.zzzz)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x40490fdbu),asfloat(0x40490fdbu),asfloat(0x40490fdbu),asfloat(0x40490fdbu)))).z;
    // 14: lt r0.z, r0.z, -r0.z
    r0.z = (asfloat((uint4)((r0.zzzz)<(-(r0.zzzz))) * 0xffffffffu)).z;
    // 15: and r0.z, r0.z, r1.z
    r0.z = (asfloat(asuint(r0.zzzz) & asuint(r1.zzzz))).z;
    // 16: mad r0.z, r1.x, r1.y, r0.z
    r0.z = ((r1.xxxx)*(r1.yyyy)+(r0.zzzz)).z;
    // 17: add r1.x, -r0.z, l(0x3fc90fdb)
    r1.x = ((-(r0.zzzz))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).x;
    // 18: mad r0.z, r0.z, l(0x3ea2f983), l(0x3f800000)
    r0.z = ((r0.zzzz)*(float4(asfloat(0x3ea2f983u),asfloat(0x3ea2f983u),asfloat(0x3ea2f983u),asfloat(0x3ea2f983u)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).z;
    // 19: mul r0.z, r0.z, l(0x3f000000)
    r0.z = ((r0.zzzz)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).z;
    // 20: mad r1.x, r1.x, l(0x3f22f984), l(0x3f800000)
    r1.x = ((r1.xxxx)*(float4(asfloat(0x3f22f984u),asfloat(0x3f22f984u),asfloat(0x3f22f984u),asfloat(0x3f22f984u)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 21: mul r1.x, r1.x, l(0x3e800000)
    r1.x = ((r1.xxxx)*(float4(asfloat(0x3e800000u),asfloat(0x3e800000u),asfloat(0x3e800000u),asfloat(0x3e800000u)))).x;
    // 22: lt r1.y, l(0x00000000), r0.w
    r1.y = (asfloat((uint4)((float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))<(r0.wwww)) * 0xffffffffu)).y;
    // 23: ge r0.w, r0.w, l(0x00000000)
    r0.w = (asfloat((uint4)((r0.wwww)>=(float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))) * 0xffffffffu)).w;
    // 24: movc r1.x, r1.y, r1.x, r0.z
    r1.x = ((asuint(r1.yyyy) != 0u) ? (r1.xxxx) : (r0.zzzz)).x;
    // 25: movc r0.z, r0.w, r1.x, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (r1.xxxx) : (r0.zzzz)).z;
    // 26: mul r0.z, r0.z, cb0[7].z
    r0.z = ((r0.zzzz)*(source[7].zzzz)).z;
    // 27: mul r0.z, r0.z, l(0x40c90fdb)
    r0.z = ((r0.zzzz)*(float4(asfloat(0x40c90fdbu),asfloat(0x40c90fdbu),asfloat(0x40c90fdbu),asfloat(0x40c90fdbu)))).z;
    // 28: dp2 r1.x, cb0[5].xyxx, r0.xyxx
    r1.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 29: dp2 r1.y, cb0[6].xyxx, r0.xyxx
    r1.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 30: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 31: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 32: div r1.xy, r1.xyxx, r0.wwww
    r1.xy = ((r1.xyxx)/(r0.wwww)).xy;
    // 33: mad r0.w, |r1.x|, l(0xbc996e30), l(0x3d981627)
    r0.w = ((abs(r1.xxxx))*(float4(asfloat(0xbc996e30u),asfloat(0xbc996e30u),asfloat(0xbc996e30u),asfloat(0xbc996e30u)))+(float4(asfloat(0x3d981627u),asfloat(0x3d981627u),asfloat(0x3d981627u),asfloat(0x3d981627u)))).w;
    // 34: mad r0.w, r0.w, |r1.x|, l(0xbe593484)
    r0.w = ((r0.wwww)*(abs(r1.xxxx))+(float4(asfloat(0xbe593484u),asfloat(0xbe593484u),asfloat(0xbe593484u),asfloat(0xbe593484u)))).w;
    // 35: mad r0.w, r0.w, |r1.x|, l(0x3fc90da4)
    r0.w = ((r0.wwww)*(abs(r1.xxxx))+(float4(asfloat(0x3fc90da4u),asfloat(0x3fc90da4u),asfloat(0x3fc90da4u),asfloat(0x3fc90da4u)))).w;
    // 36: add r1.z, -|r1.x|, l(0x3f800000)
    r1.z = ((-(abs(r1.xxxx)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).z;
    // 37: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 38: mul r1.w, r0.w, r1.z
    r1.w = ((r0.wwww)*(r1.zzzz)).w;
    // 39: mad r1.w, r1.w, l(0xc0000000), l(0x40490fdb)
    r1.w = ((r1.wwww)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x40490fdbu),asfloat(0x40490fdbu),asfloat(0x40490fdbu),asfloat(0x40490fdbu)))).w;
    // 40: lt r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)<(-(r1.xxxx))) * 0xffffffffu)).x;
    // 41: and r1.x, r1.x, r1.w
    r1.x = (asfloat(asuint(r1.xxxx) & asuint(r1.wwww))).x;
    // 42: mad r0.w, r0.w, r1.z, r1.x
    r0.w = ((r0.wwww)*(r1.zzzz)+(r1.xxxx)).w;
    // 43: add r1.x, -r0.w, l(0x3fc90fdb)
    r1.x = ((-(r0.wwww))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).x;
    // 44: mad r0.w, r0.w, l(0x3ea2f983), l(0x3f800000)
    r0.w = ((r0.wwww)*(float4(asfloat(0x3ea2f983u),asfloat(0x3ea2f983u),asfloat(0x3ea2f983u),asfloat(0x3ea2f983u)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 45: mul r0.w, r0.w, l(0x3f000000)
    r0.w = ((r0.wwww)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).w;
    // 46: mad r1.x, r1.x, l(0x3f22f984), l(0x3f800000)
    r1.x = ((r1.xxxx)*(float4(asfloat(0x3f22f984u),asfloat(0x3f22f984u),asfloat(0x3f22f984u),asfloat(0x3f22f984u)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 47: mul r1.x, r1.x, l(0x3e800000)
    r1.x = ((r1.xxxx)*(float4(asfloat(0x3e800000u),asfloat(0x3e800000u),asfloat(0x3e800000u),asfloat(0x3e800000u)))).x;
    // 48: lt r1.z, l(0x00000000), r1.y
    r1.z = (asfloat((uint4)((float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))<(r1.yyyy)) * 0xffffffffu)).z;
    // 49: ge r1.y, r1.y, l(0x00000000)
    r1.y = (asfloat((uint4)((r1.yyyy)>=(float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))) * 0xffffffffu)).y;
    // 50: movc r1.x, r1.z, r1.x, r0.w
    r1.x = ((asuint(r1.zzzz) != 0u) ? (r1.xxxx) : (r0.wwww)).x;
    // 51: movc r0.w, r1.y, r1.x, r0.w
    r0.w = ((asuint(r1.yyyy) != 0u) ? (r1.xxxx) : (r0.wwww)).w;
    // 52: mul r0.w, r0.w, cb0[8].z
    r0.w = ((r0.wwww)*(source[8].zzzz)).w;
    // 53: mul r0.w, r0.w, l(0x40c90fdb)
    r0.w = ((r0.wwww)*(float4(asfloat(0x40c90fdbu),asfloat(0x40c90fdbu),asfloat(0x40c90fdbu),asfloat(0x40c90fdbu)))).w;
    // 54: sincos r0.zw, null, r0.zzzw
    r0.zw = (sin(r0.zzzw)).zw;
    // 55: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 56: mul r0.z, r0.z, cb0[8].w
    r0.z = ((r0.zzzz)*(source[8].wwww)).z;
    // 57: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 58: add r0.w, -r0.w, l(0x3f800000)
    r0.w = ((-(r0.wwww))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 59: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 60: lt r0.w, |r0.w|, l(0x358637bd)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu)))) * 0xffffffffu)).w;
    // 61: mul r1.x, r1.x, l(0x41200000)
    r1.x = ((r1.xxxx)*(float4(asfloat(0x41200000u),asfloat(0x41200000u),asfloat(0x41200000u),asfloat(0x41200000u)))).x;
    // 62: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 63: movc r0.w, r0.w, l(0x00000000), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))) : (r1.xxxx)).w;
    // 64: add r0.w, r0.w, v4.x
    r0.w = ((r0.wwww)+(v4.xxxx)).w;
    // 65: add r0.w, r0.w, l(0xbf000000)
    r0.w = ((r0.wwww)+(float4(asfloat(0xbf000000u),asfloat(0xbf000000u),asfloat(0xbf000000u),asfloat(0xbf000000u)))).w;
    // 66: mad r0.z, r0.z, l(0x3f000000), r0.w
    r0.z = ((r0.zzzz)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))+(r0.wwww)).z;
    // 67: mul_sat r0.z, |r0.z|, cb0[9].y
    r0.z = (saturate((abs(r0.zzzz))*(source[9].yyyy))).z;
    // 68: add r0.z, -r0.z, l(0x3f800000)
    r0.z = ((-(r0.zzzz))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).z;
    // 69: mad r1.xyz, v4.xxxx, l(0x3f000000,0x3f000000,0x3f000000,0x00000000), l(0x3e4cccce,0x3e2e147c,0x3e99999a,0x00000000)
    r1.xyz = ((v4.xxxx)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x00000000u)))+(float4(asfloat(0x3e4cccceu),asfloat(0x3e2e147cu),asfloat(0x3e99999au),asfloat(0x00000000u)))).xyz;
    // 70: max r1.xyz, r1.xyzx, l(0x3727c5ac,0x3727c5ac,0x3727c5ac,0x00000000)
    r1.xyz = (max(r1.xyzx,float4(asfloat(0x3727c5acu),asfloat(0x3727c5acu),asfloat(0x3727c5acu),asfloat(0x00000000u)))).xyz;
    // 71: div r1.xyz, l(0x3f800000,0x3f800000,0x3f800000,0x3f800000), r1.xyzx
    r1.xyz = ((float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))/(r1.xyzx)).xyz;
    // 72: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 73: mul r0.xy, r0.xyxx, l(0x3fd9999a,0x3fd9999a,0x00000000,0x00000000)
    r0.xy = ((r0.xyxx)*(float4(asfloat(0x3fd9999au),asfloat(0x3fd9999au),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 74: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 75: mad r1.xyz, -r0.wwww, r1.xyzx, l(0x3f800000,0x3f800000,0x3f800000,0x00000000)
    r1.xyz = ((-(r0.wwww))*(r1.xyzx)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x00000000u)))).xyz;
    // 76: mad r0.w, -r0.w, l(0x40000000), l(0x3f800000)
    r0.w = ((-(r0.wwww))*(float4(asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x40000000u)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 77: add_sat r0.w, r0.w, r0.w
    r0.w = (saturate((r0.wwww)+(r0.wwww))).w;
    // 78: max r1.yz, r1.yyzy, l(0x00000000,0x00000000,0x00000000,0x00000000)
    r1.yz = (max(r1.yyzy,float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))).yz;
    // 79: mul_sat r1.x, r1.x, l(0x40a00001)
    r1.x = (saturate((r1.xxxx)*(float4(asfloat(0x40a00001u),asfloat(0x40a00001u),asfloat(0x40a00001u),asfloat(0x40a00001u))))).x;
    // 80: lt r1.w, l(0x00000000), r1.y
    r1.w = (asfloat((uint4)((float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))<(r1.yyyy)) * 0xffffffffu)).w;
    // 81: movc r0.z, r1.w, l(0x00000000), r0.z
    r0.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))) : (r0.zzzz)).z;
    // 82: rsq r1.w, r1.z
    r1.w = (rsqrt(r1.zzzz)).w;
    // 83: div r1.w, l(0x3f800000,0x3f800000,0x3f800000,0x3f800000), r1.w
    r1.w = ((float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))/(r1.wwww)).w;
    // 84: mul r1.w, r1.w, v4.w
    r1.w = ((r1.wwww)*(v4.wwww)).w;
    // 85: lt r1.z, r1.z, l(0x358637bd)
    r1.z = (asfloat((uint4)((r1.zzzz)<(float4(asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu)))) * 0xffffffffu)).z;
    // 86: add r1.y, -r1.y, l(0x3f800000)
    r1.y = ((-(r1.yyyy))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).y;
    // 87: movc r1.z, r1.z, l(0x80000000), -r1.w
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0x80000000u),asfloat(0x80000000u),asfloat(0x80000000u),asfloat(0x80000000u))) : (-(r1.wwww))).z;
    // 88: add r0.z, r0.z, r1.z
    r0.z = ((r0.zzzz)+(r1.zzzz)).z;
    // 89: max r1.z, |r0.y|, |r0.x|
    r1.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 90: div r1.z, l(0x3f800000,0x3f800000,0x3f800000,0x3f800000), r1.z
    r1.z = ((float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))/(r1.zzzz)).z;
    // 91: min r1.w, |r0.y|, |r0.x|
    r1.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 92: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 93: mul r1.w, r1.z, r1.z
    r1.w = ((r1.zzzz)*(r1.zzzz)).w;
    // 94: mad r2.x, r1.w, l(0x3caaae5f), l(0xbdae5a36)
    r2.x = ((r1.wwww)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).x;
    // 95: mad r2.x, r1.w, r2.x, l(0x3e3876e2)
    r2.x = ((r1.wwww)*(r2.xxxx)+(float4(asfloat(0x3e3876e2u),asfloat(0x3e3876e2u),asfloat(0x3e3876e2u),asfloat(0x3e3876e2u)))).x;
    // 96: mad r2.x, r1.w, r2.x, l(0xbea91d04)
    r2.x = ((r1.wwww)*(r2.xxxx)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).x;
    // 97: mad r1.w, r1.w, r2.x, l(0x3f7ff738)
    r1.w = ((r1.wwww)*(r2.xxxx)+(float4(asfloat(0x3f7ff738u),asfloat(0x3f7ff738u),asfloat(0x3f7ff738u),asfloat(0x3f7ff738u)))).w;
    // 98: mul r2.x, r1.w, r1.z
    r2.x = ((r1.wwww)*(r1.zzzz)).x;
    // 99: mad r2.x, r2.x, l(0xc0000000), l(0x3fc90fdb)
    r2.x = ((r2.xxxx)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).x;
    // 100: lt r2.y, |r0.y|, |r0.x|
    r2.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 101: and r2.x, r2.y, r2.x
    r2.x = (asfloat(asuint(r2.yyyy) & asuint(r2.xxxx))).x;
    // 102: mad r1.z, r1.z, r1.w, r2.x
    r1.z = ((r1.zzzz)*(r1.wwww)+(r2.xxxx)).z;
    // 103: lt r1.w, r0.y, -r0.y
    r1.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 104: and r1.w, r1.w, l(0xc0490fdb)
    r1.w = (asfloat(asuint(r1.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 105: add r1.z, r1.w, r1.z
    r1.z = ((r1.wwww)+(r1.zzzz)).z;
    // 106: min r1.w, r0.y, r0.x
    r1.w = (min(r0.yyyy,r0.xxxx)).w;
    // 107: lt r1.w, r1.w, -r1.w
    r1.w = (asfloat((uint4)((r1.wwww)<(-(r1.wwww))) * 0xffffffffu)).w;
    // 108: max r2.x, r0.y, r0.x
    r2.x = (max(r0.yyyy,r0.xxxx)).x;
    // 109: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 110: ge r0.y, r2.x, -r2.x
    r0.y = (asfloat((uint4)((r2.xxxx)>=(-(r2.xxxx))) * 0xffffffffu)).y;
    // 111: and r0.y, r0.y, r1.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r1.wwww))).y;
    // 112: movc r0.y, r0.y, -r1.z, r1.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r1.zzzz)) : (r1.zzzz)).y;
    // 113: mad r0.y, r0.y, l(0x3f22f986), l(0x3f800000)
    r0.y = ((r0.yyyy)*(float4(asfloat(0x3f22f986u),asfloat(0x3f22f986u),asfloat(0x3f22f986u),asfloat(0x3f22f986u)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).y;
    // 114: mul r2.x, r0.y, l(0x3f000000)
    r2.x = ((r0.yyyy)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).x;
    // 115: lt r0.y, r0.x, l(0x358637bd)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu)))) * 0xffffffffu)).y;
    // 116: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 117: movc r2.y, r0.y, l(0x00000000), r0.x
    r2.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))) : (r0.xxxx)).y;
    // 118: add r0.xy, r2.xyxx, cb0[2].xyxx
    r0.xy = ((r2.xyxx)+(source[2].xyxx)).xy;
    // 119: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0x00000000)
    r0.x = (SDNativeSample0((r0.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x, false).xyzw).x;
    // 120: mul r0.x, r1.x, r0.x
    r0.x = ((r1.xxxx)*(r0.xxxx)).x;
    // 121: mad r0.x, r0.x, r1.y, r0.z
    r0.x = ((r0.xxxx)*(r1.yyyy)+(r0.zzzz)).x;
    // 122: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 123: mul r0.x, r0.x, -v4.z
    r0.x = ((r0.xxxx)*(-(v4.zzzz))).x;
    // 124: mul r0.x, r0.x, v4.y
    r0.x = ((r0.xxxx)*(v4.yyyy)).x;
    // 125: div r0.y, l(0x44c00000), v7.z
    r0.y = ((float4(asfloat(0x44c00000u),asfloat(0x44c00000u),asfloat(0x44c00000u),asfloat(0x44c00000u)))/(v7.zzzz)).y;
    // 126: mul r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)*(r0.xxxx)).y;
    // 127: mov r0.x, l(0xc2800000)
    r0.x = (float4(asfloat(0xc2800000u),asfloat(0xc2800000u),asfloat(0xc2800000u),asfloat(0xc2800000u))).x;
    // 128: max r0.xy, r0.xyxx, l(0x00000000,0xc2800000,0x00000000,0x00000000)
    r0.xy = (max(r0.xyxx,float4(asfloat(0x00000000u),asfloat(0xc2800000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 129: min r0.xy, r0.xyxx, l(0x42800000,0x42800000,0x00000000,0x00000000)
    r0.xy = (min(r0.xyxx,float4(asfloat(0x42800000u),asfloat(0x42800000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 130: mad r0.xy, r0.xyxx, l(0x3c008081,0x3c008081,0x00000000,0x00000000), l(0xbf800000,0x3f800000,0x00000000,0x00000000)
    r0.xy = ((r0.xyxx)*(float4(asfloat(0x3c008081u),asfloat(0x3c008081u),asfloat(0x00000000u),asfloat(0x00000000u)))+(float4(asfloat(0xbf800000u),asfloat(0x3f800000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 131: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 132: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 133: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 134: add r0.xy, r0.xyxx, r0.zwzz
    r0.xy = ((r0.xyxx)+(r0.zwzz)).xy;
    // 135: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r0.xyz = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r0.xyxx).xy).xyzw).xyz;
    // 136: add r0.w, -cb0[10].x, l(0x3f800000)
    r0.w = ((-(source[10].xxxx))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 137: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 138: mul r1.xyz, v5.wwww, cb0[1].xyzx
    r1.xyz = ((v5.wwww)*(source[1].xyzx)).xyz;
    // 139: mad o0.xyz, r1.xyzx, cb0[0].xxxx, r0.xyzx
    output.xyz = ((r1.xyzx)*(source[0].xxxx)+(r0.xyzx)).xyz;
    // 140: mov o0.w, l(0x00000000)
    output.w = (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).w;
    return output;
}

EFFECT_PS_OUT Shade_EffectDimensionMasterSDNative(uint profile, SD_NATIVE_INPUT input)
{
    EFFECT_PS_OUT output=(EFFECT_PS_OUT)0;
    float4 nativeColor=0.f;
    bool additive=false;
    switch(profile)
    {
    case 320u: nativeColor=SDNative320(input); additive=false; break;
    case 321u: nativeColor=SDNative321(input); additive=false; break;
    case 322u: nativeColor=SDNative322(input); additive=false; break;
    case 323u: nativeColor=SDNative323(input); additive=true; break;
    case 324u: nativeColor=SDNative324(input); additive=false; break;
    case 325u: nativeColor=SDNative325(input); additive=true; break;
    case 326u: nativeColor=SDNative326(input); additive=false; break;
    case 327u: nativeColor=SDNative327(input); additive=false; break;
    case 328u: nativeColor=SDNative328(input); additive=true; break;
    case 329u: nativeColor=SDNative329(input); additive=false; break;
    case 330u: nativeColor=SDNative330(input); additive=true; break;
    case 331u: nativeColor=SDNative331(input); additive=false; break;
    case 332u: nativeColor=SDNative332(input); additive=true; break;
    case 341u: nativeColor=SDNative341(input); additive=false; break;
    case 342u: nativeColor=SDNative342(input); additive=false; break;
    case 343u: nativeColor=SDNative343(input); additive=false; break;
    case 344u: nativeColor=SDNative344(input); additive=true; break;
    case 345u: nativeColor=SDNative345(input); additive=false; break;
    case 347u: nativeColor=SDNative347(input); additive=false; break;
    case 348u: nativeColor=SDNative348(input); additive=false; break;
    case 349u: nativeColor=SDNative349(input); additive=true; break;
    case 351u: nativeColor=SDNative351(input); additive=false; break;
    case 352u: nativeColor=SDNative352(input); additive=false; break;
    case 360u: nativeColor=SDNative360(input); additive=true; break;
    case 361u: nativeColor=SDNative361(input); additive=false; break;
    case 362u: nativeColor=SDNative362(input); additive=false; break;
    case 363u: nativeColor=SDNative363(input); additive=true; break;
    case 364u: nativeColor=SDNative364(input); additive=false; break;
    case 365u: nativeColor=SDNative365(input); additive=false; break;
    case 366u: nativeColor=SDNative366(input); additive=true; break;
    case 367u: nativeColor=SDNative367(input); additive=true; break;
    case 368u: nativeColor=SDNative368(input); additive=true; break;
    case 369u: nativeColor=SDNative369(input); additive=false; break;
    case 370u: nativeColor=SDNative370(input); additive=false; break;
    case 371u: nativeColor=SDNative371(input); additive=false; break;
    case 372u: nativeColor=SDNative372(input); additive=false; break;
    case 373u: nativeColor=SDNative373(input); additive=false; break;
    case 375u: nativeColor=SDNative375(input); additive=false; break;
    case 374u: nativeColor=SDNative374(input); additive=true; break;
    case 389u: nativeColor=SDNative389(input); additive=false; break;
    case 390u: nativeColor=SDNative390(input); additive=false; break;
    case 391u: nativeColor=SDNative391(input); additive=false; break;
    case 392u: nativeColor=SDNative392(input); additive=false; break;
    case 393u: nativeColor=SDNative393(input); additive=false; break;
    case 394u: nativeColor=SDNative394(input); additive=false; break;
    case 395u: nativeColor=SDNative395(input); additive=false; break;
    case 396u: nativeColor=SDNative396(input); additive=false; break;
    case 397u: nativeColor=SDNative397(input); additive=true; break;
    case 399u: nativeColor=SDNative399(input); additive=false; break;
    default: clip(-1.f); return output;
    }
    // OneLayer already includes SceneColor; preserve the source RT0 alpha.
    output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity, (374u == profile || !additive) ? nativeColor.a : 1.f);
    output.Distortion=0.f;
    if(g_ColorClip>0.f) clip(output.SceneColor.a-g_ColorClip);
    return output;
}
#endif
