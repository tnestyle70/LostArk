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
    r1.x = ((r0.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).x;
    // 8: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 9: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).x;
    // 10: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 11: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 12: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
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
    r1.x = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
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
    r1.x = ((r0.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).x;
    // 8: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 9: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).x;
    // 10: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 11: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 12: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
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
    r1.x = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
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
    r1.z = ((r0.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 24: mad r1.z, r0.w, r1.z, l(0.180141)
    r1.z = ((r0.wwww)*(r1.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 25: mad r1.z, r0.w, r1.z, l(-0.330299)
    r1.z = ((r0.wwww)*(r1.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 26: mad r0.w, r0.w, r1.z, l(0.999866)
    r0.w = ((r0.wwww)*(r1.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 27: mul r1.z, r0.w, r0.z
    r1.z = ((r0.wwww)*(r0.zzzz)).z;
    // 28: mad r1.z, r1.z, l(-2.000000), l(1.570796)
    r1.z = ((r1.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
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
    r2.x = ((r0.zzzz)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.wwww)).x;
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
    r1.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
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
    r0.y = ((r0.yyyy)*(float4(6.283185,6.283185,6.283185,6.283185))).y;
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
    default: clip(-1.f); return output;
    }
    output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity, additive ? 1.f : nativeColor.a);
    output.Distortion=0.f;
    if(g_ColorClip>0.f) clip(output.SceneColor.a-g_ColorClip);
    return output;
}
#endif
