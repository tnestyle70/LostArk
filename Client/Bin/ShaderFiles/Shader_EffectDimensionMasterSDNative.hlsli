// Selected S_D native RT0 material programs; see full_native_contract.json.
// Existing Product and grouped material programs never select these IDs.
#ifndef EFFECT_DIMENSIONMASTER_SD_NATIVE_HLSLI
#define EFFECT_DIMENSIONMASTER_SD_NATIVE_HLSLI
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
    default: clip(-1.f); return output;
    }
    output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity, additive ? 1.f : nativeColor.a);
    output.Distortion=0.f;
    if(g_ColorClip>0.f) clip(output.SceneColor.a-g_ColorClip);
    return output;
}
#endif
