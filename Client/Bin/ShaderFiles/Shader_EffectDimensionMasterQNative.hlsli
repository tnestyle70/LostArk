// Selected Q native RT0 material programs; see full_native_contract.json.
// Existing Product and grouped material programs never select these IDs.
#ifndef EFFECT_DIMENSIONMASTER_Q_NATIVE_HLSLI
#define EFFECT_DIMENSIONMASTER_Q_NATIVE_HLSLI
#include "Shader_EffectSliceSceneDepth.hlsli"

float4 g_QSourceMaterialParameters[32];
float g_QSourceMaterialTime = 0.f;

float4 QNativeAppend(float4 a, float4 b, uint n)
{
    if (n == 1u) return float4(a.x, b.xyz);
    if (n == 2u) return float4(a.xy, b.xy);
    if (n == 3u) return float4(a.xyz, b.x);
    return a;
}
float4 QNativePeriodic(float4 a) { return sign(a) * frac(abs(a)); }

struct Q_NATIVE_INPUT
{
    float2 uv;
    float2 uv1;
    float2 screenUV;
    float projectionW;
    float3 tangentView;
    float4 color;
    float4 dynamicParameter;
};

float4 QNativeSample0(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 0u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 0u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture0.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearSampler, uv, lod);
}

float4 QNativeSample1(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 1u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 1u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture1.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearSampler, uv, lod);
}

float4 QNativeSample2(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 2u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 2u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture2.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearSampler, uv, lod);
}

float4 QNativeSample3(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 3u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 3u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture3.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearSampler, uv, lod);
}

float4 QNativeSample4(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 4u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 4u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture4.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearSampler, uv, lod);
}

float4 QNativeSample5(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 5u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 5u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture5.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearSampler, uv, lod);
}

float4 QNativeSample6(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 6u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 6u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture6.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearSampler, uv, lod);
}

// blackline-aura: 0a37df187fa6494387432fa2db19d4b1; selected map 3607793858bc73a0926f499049188a8a9044974f30728822e28aed03a24b44d0.
float4 QNative44(Q_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_QSourceMaterialParameters[18u];
    source[3] = g_QSourceMaterialParameters[15u];
    source[4] = input.dynamicParameter;
    source[5] = QNativeAppend(cos((g_QSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_QSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = QNativeAppend(sin((g_QSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_QSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = QNativeAppend(cos((g_QSourceMaterialParameters[11u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_QSourceMaterialParameters[11u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = QNativeAppend(sin((g_QSourceMaterialParameters[11u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_QSourceMaterialParameters[11u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = QNativeAppend(cos((g_QSourceMaterialParameters[8u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_QSourceMaterialParameters[8u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = QNativeAppend(sin((g_QSourceMaterialParameters[8u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_QSourceMaterialParameters[8u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11] = g_QSourceMaterialParameters[16u];
    source[12].x = (cos((g_QSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_QSourceMaterialParameters[6u].yyyy).x;
    source[12].z = (g_QSourceMaterialParameters[6u].zzzz).x;
    source[12].w = (g_QSourceMaterialTime.xxxx).x;
    source[13].x = (g_QSourceMaterialParameters[5u].zzzz).x;
    source[13].y = ((g_QSourceMaterialParameters[5u].zzzz*g_QSourceMaterialTime.xxxx)).x;
    source[13].z = (g_QSourceMaterialParameters[5u].wwww).x;
    source[13].w = ((g_QSourceMaterialParameters[5u].wwww*g_QSourceMaterialTime.xxxx)).x;
    source[14].x = (g_QSourceMaterialParameters[6u].xxxx).x;
    source[14].y = (g_QSourceMaterialParameters[7u].zzzz).x;
    source[14].z = (g_QSourceMaterialParameters[7u].wwww).x;
    source[14].w = (g_QSourceMaterialParameters[6u].wwww).x;
    source[15].x = ((g_QSourceMaterialParameters[6u].wwww*g_QSourceMaterialTime.xxxx)).x;
    source[15].y = (g_QSourceMaterialParameters[7u].xxxx).x;
    source[15].z = ((g_QSourceMaterialParameters[7u].xxxx*g_QSourceMaterialTime.xxxx)).x;
    source[15].w = (g_QSourceMaterialParameters[7u].yyyy).x;
    source[16].x = (g_QSourceMaterialParameters[1u].wwww).x;
    source[16].y = (g_QSourceMaterialParameters[1u].yyyy).x;
    source[16].z = (g_QSourceMaterialParameters[1u].zzzz).x;
    source[16].w = (g_QSourceMaterialParameters[3u].yyyy).x;
    source[17].x = (g_QSourceMaterialParameters[3u].zzzz).x;
    source[17].y = (g_QSourceMaterialParameters[2u].xxxx).x;
    source[17].z = (g_QSourceMaterialParameters[2u].yyyy).x;
    source[17].w = (g_QSourceMaterialParameters[2u].zzzz).x;
    source[18].x = (g_QSourceMaterialParameters[3u].xxxx).x;
    source[18].y = (g_QSourceMaterialParameters[11u].xxxx).x;
    source[18].z = ((g_QSourceMaterialParameters[11u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[18].w = (sin((g_QSourceMaterialParameters[11u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_QSourceMaterialParameters[11u].xxxx*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[19].y = (cos((g_QSourceMaterialParameters[11u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].z = (g_QSourceMaterialParameters[0u].zzzz).x;
    source[19].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_QSourceMaterialParameters[0u].zzzz)).x;
    source[20].x = (g_QSourceMaterialParameters[0u].wwww).x;
    source[20].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_QSourceMaterialParameters[0u].wwww)).x;
    source[20].z = (g_QSourceMaterialParameters[13u].yyyy).x;
    source[20].w = (g_QSourceMaterialParameters[13u].zzzz).x;
    source[21].x = (g_QSourceMaterialParameters[13u].wwww).x;
    source[21].y = (g_QSourceMaterialParameters[11u].zzzz).x;
    source[21].z = (g_QSourceMaterialParameters[11u].wwww).x;
    source[21].w = (g_QSourceMaterialParameters[10u].yyyy).x;
    source[22].x = (g_QSourceMaterialParameters[9u].wwww).x;
    source[22].y = ((g_QSourceMaterialParameters[10u].yyyy*g_QSourceMaterialTime.xxxx)).x;
    source[22].z = (((g_QSourceMaterialParameters[10u].yyyy*g_QSourceMaterialTime.xxxx)+g_QSourceMaterialParameters[9u].wwww)).x;
    source[22].w = (g_QSourceMaterialParameters[10u].zzzz).x;
    source[23].x = (g_QSourceMaterialParameters[10u].xxxx).x;
    source[23].y = ((g_QSourceMaterialParameters[10u].zzzz*g_QSourceMaterialTime.xxxx)).x;
    source[23].z = (((g_QSourceMaterialParameters[10u].zzzz*g_QSourceMaterialTime.xxxx)+g_QSourceMaterialParameters[10u].xxxx)).x;
    source[23].w = (g_QSourceMaterialParameters[10u].wwww).x;
    source[24].x = (g_QSourceMaterialParameters[11u].yyyy).x;
    source[24].y = (g_QSourceMaterialParameters[8u].wwww).x;
    source[24].z = ((g_QSourceMaterialParameters[8u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[24].w = (sin((g_QSourceMaterialParameters[8u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[25].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_QSourceMaterialParameters[8u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[25].y = (cos((g_QSourceMaterialParameters[8u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[25].z = (g_QSourceMaterialParameters[0u].xxxx).x;
    source[25].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_QSourceMaterialParameters[0u].xxxx)).x;
    source[26].x = (g_QSourceMaterialParameters[0u].yyyy).x;
    source[26].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_QSourceMaterialParameters[0u].yyyy)).x;
    source[26].z = (g_QSourceMaterialParameters[12u].zzzz).x;
    source[26].w = (g_QSourceMaterialParameters[12u].wwww).x;
    source[27].x = (g_QSourceMaterialParameters[13u].xxxx).x;
    source[27].y = (g_QSourceMaterialParameters[9u].yyyy).x;
    source[27].z = (g_QSourceMaterialParameters[9u].zzzz).x;
    source[27].w = (g_QSourceMaterialParameters[8u].xxxx).x;
    source[28].x = (g_QSourceMaterialParameters[8u].yyyy).x;
    source[28].y = (g_QSourceMaterialParameters[8u].zzzz).x;
    source[28].z = (g_QSourceMaterialParameters[9u].xxxx).x;
    source[28].w = (g_QSourceMaterialParameters[1u].xxxx).x;
    source[29].x = (g_QSourceMaterialParameters[5u].yyyy).x;
    source[29].y = (g_QSourceMaterialParameters[5u].xxxx).x;
    source[29].z = (g_QSourceMaterialParameters[12u].yyyy).x;
    source[29].w = (g_QSourceMaterialParameters[12u].xxxx).x;
    source[30].x = (g_QSourceMaterialParameters[14u].xxxx).x;
    source[30].y = (g_QSourceMaterialParameters[14u].yyyy).x;
    source[30].z = (g_QSourceMaterialParameters[4u].xxxx).x;
    source[30].w = (g_QSourceMaterialParameters[4u].zzzz).x;
    source[31].x = (g_QSourceMaterialParameters[4u].wwww).x;
    source[31].y = (g_QSourceMaterialParameters[4u].yyyy).x;
    source[31].z = (g_QSourceMaterialParameters[3u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=float4(input.uv,input.uv1.yx);
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mad r0.xy, cb0[4].wwww, cb0[16].yzyy, v4.xyxx
    r0.xy = ((source[4].wwww)*(source[16].yzyy)+(v4.xyxx)).xy;
    // 2: mul r1.x, r0.x, cb0[16].w
    r1.x = ((r0.xxxx)*(source[16].wwww)).x;
    // 3: mul r1.y, r0.y, cb0[17].x
    r1.y = ((r0.yyyy)*(source[17].xxxx)).y;
    // 4: add r0.xy, r1.xyxx, cb0[17].yzyy
    r0.xy = ((r1.xyxx)+(source[17].yzyy)).xy;
    // 5: mad r0.zw, v4.xxxy, cb0[14].yyyz, cb0[15].xxxz
    r0.zw = ((v4.xxxy)*(source[14].yyyz)+(source[15].xxxz)).zw;
    // 6: sample_l_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s1, l(-1.000000)
    r0.zw = (QNativeSample1((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).zwxy).zw;
    // 7: mul r0.zw, r0.zzzw, cb0[15].wwww
    r0.zw = ((r0.zzzw)*(source[15].wwww)).zw;
    // 8: mad r1.xy, v4.xyxx, cb0[12].yzyy, cb0[13].ywyy
    r1.xy = ((v4.xyxx)*(source[12].yzyy)+(source[13].ywyy)).xy;
    // 9: sample_l_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s1, l(-1.000000)
    r1.xy = (QNativeSample1((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
    // 10: mad r0.zw, cb0[14].xxxx, r1.xxxy, r0.zzzw
    r0.zw = ((source[14].xxxx)*(r1.xxxy)+(r0.zzzw)).zw;
    // 11: mad r0.xy, cb0[16].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[16].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 12: mul r0.zw, r0.zzzw, cb0[4].yyyy
    r0.zw = ((r0.zzzw)*(source[4].yyyy)).zw;
    // 13: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 1.010000, 1.010000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,1.010000,1.010000))).zw;
    // 14: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 15: dp2 r1.x, cb0[5].xyxx, r0.xyxx
    r1.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 16: dp2 r1.y, cb0[6].xyxx, r0.xyxx
    r1.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 17: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 18: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t4.xyzw, s0, l(-1.000000)
    r1.xyz = (QNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 19: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 20: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 21: mul r1.xyz, r1.xyzx, cb0[17].wwww
    r1.xyz = ((r1.xyzx)*(source[17].wwww)).xyz;
    // 22: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 23: mul r1.xyz, r1.xyzx, cb0[18].xxxx
    r1.xyz = ((r1.xyzx)*(source[18].xxxx)).xyz;
    // 24: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 25: mad r2.x, r0.x, cb0[19].w, l(0.500000)
    r2.x = ((r0.xxxx)*(source[19].wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 26: mad r2.y, r0.y, cb0[20].y, l(0.500000)
    r2.y = ((r0.yyyy)*(source[20].yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 27: mad r2.xy, cb0[4].zzzz, cb0[20].zwzz, r2.xyxx
    r2.xy = ((source[4].zzzz)*(source[20].zwzz)+(r2.xyxx)).xy;
    // 28: mad r2.xy, cb0[21].xxxx, r0.zwzz, r2.xyxx
    r2.xy = ((source[21].xxxx)*(r0.zwzz)+(r2.xyxx)).xy;
    // 29: mad r3.x, r2.x, cb0[21].y, cb0[22].z
    r3.x = ((r2.xxxx)*(source[21].yyyy)+(source[22].zzzz)).x;
    // 30: mad r3.y, r2.y, cb0[21].z, cb0[23].z
    r3.y = ((r2.yyyy)*(source[21].zzzz)+(source[23].zzzz)).y;
    // 31: add r2.xy, r3.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 32: dp2 r3.x, cb0[7].xyxx, r2.xyxx
    r3.x = (dot((source[7].xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 33: dp2 r3.y, cb0[8].xyxx, r2.xyxx
    r3.y = (dot((source[8].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 34: add r2.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 35: sample_l_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t2.yzwx, s2, l(-1.000000)
    r1.w = (QNativeSample2((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 36: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 37: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 38: mul r2.x, r2.x, cb0[23].w
    r2.x = ((r2.xxxx)*(source[23].wwww)).x;
    // 39: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 40: mul r2.x, r2.x, cb0[24].x
    r2.x = ((r2.xxxx)*(source[24].xxxx)).x;
    // 41: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 42: mad r2.x, r0.x, cb0[25].w, l(0.500000)
    r2.x = ((r0.xxxx)*(source[25].wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 43: mad r2.y, r0.y, cb0[26].y, l(0.500000)
    r2.y = ((r0.yyyy)*(source[26].yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 44: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 45: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 46: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 47: mad r2.xy, cb0[4].xxxx, cb0[26].zwzz, r2.xyxx
    r2.xy = ((source[4].xxxx)*(source[26].zwzz)+(r2.xyxx)).xy;
    // 48: mad r0.yz, cb0[27].xxxx, r0.zzwz, r2.xxyx
    r0.yz = ((source[27].xxxx)*(r0.zzwz)+(r2.xxyx)).yz;
    // 49: mad r2.x, r0.y, cb0[27].y, cb0[27].w
    r2.x = ((r0.yyyy)*(source[27].yyyy)+(source[27].wwww)).x;
    // 50: mad r2.y, r0.z, cb0[27].z, cb0[28].x
    r2.y = ((r0.zzzz)*(source[27].zzzz)+(source[28].xxxx)).y;
    // 51: add r0.yz, r2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 52: dp2 r2.x, cb0[9].xyxx, r0.yzyy
    r2.x = (dot((source[9].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 53: dp2 r2.y, cb0[10].xyxx, r0.yzyy
    r2.y = (dot((source[10].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 54: add r0.yz, r2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 55: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s3, l(-1.000000)
    r0.y = (QNativeSample3((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yxzw).y;
    // 56: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 57: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 58: mul r0.y, r0.y, cb0[28].y
    r0.y = ((r0.yyyy)*(source[28].yyyy)).y;
    // 59: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 60: mul r0.y, r0.y, cb0[28].z
    r0.y = ((r0.yyyy)*(source[28].zzzz)).y;
    // 61: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 62: add r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)+(r1.wwww)).z;
    // 63: mul r2.xyz, r1.xyzx, r0.zzzz
    r2.xyz = ((r1.xyzx)*(r0.zzzz)).xyz;
    // 64: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: mad r1.xyz, -r0.zzzz, r1.xyzx, r0.wwww
    r1.xyz = ((-(r0.zzzz))*(r1.xyzx)+(r0.wwww)).xyz;
    // 66: mad r1.xyz, cb0[28].wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((source[28].wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 67: mul r0.z, r0.y, r0.y
    r0.z = ((r0.yyyy)*(r0.yyyy)).z;
    // 68: mad r0.z, r0.y, r1.w, r0.z
    r0.z = ((r0.yyyy)*(r1.wwww)+(r0.zzzz)).z;
    // 69: mad r0.y, -r0.y, r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 70: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 71: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 72: mov_sat r0.z, r0.z
    r0.z = (saturate(r0.zzzz)).z;
    // 73: mul r0.z, r0.z, cb0[1].w
    r0.z = ((r0.zzzz)*(source[1].wwww)).z;
    // 74: mul r0.y, r0.y, cb0[29].x
    r0.y = ((r0.yyyy)*(source[29].xxxx)).y;
    // 75: lt r0.w, |r0.y|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 76: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 77: mul r0.y, r0.y, cb0[29].y
    r0.y = ((r0.yyyy)*(source[29].yyyy)).y;
    // 78: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 79: mul r2.xyz, r0.yyyy, cb0[11].xyzx
    r2.xyz = ((r0.yyyy)*(source[11].xyzx)).xyz;
    // 80: mul r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(source[1].xyzx)).xyz;
    // 81: movc r2.xyz, r0.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 82: mad r1.xyz, cb0[3].xyzx, r1.xyzx, r2.xyzx
    r1.xyz = ((source[3].xyzx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 83: mad r1.xyz, r1.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 84: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 85: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 86: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 87: mad r0.w, cb0[29].z, l(10.000000), l(10.000000)
    r0.w = ((source[29].zzzz)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 88: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 89: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 90: mul r0.y, r0.y, cb0[29].w
    r0.y = ((r0.yyyy)*(source[29].wwww)).y;
    // 91: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 92: max r0.x, r0.x, cb0[30].y
    r0.x = (max(r0.xxxx,source[30].yyyy)).x;
    // 93: min r0.x, r0.x, cb0[30].x
    r0.x = (min(r0.xxxx,source[30].xxxx)).x;
    // 94: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 95: mul r0.y, v4.w, cb0[30].w
    r0.y = ((v4.wwww)*(source[30].wwww)).y;
    // 96: mad r1.x, cb0[12].w, cb0[30].z, r0.y
    r1.x = ((source[12].wwww)*(source[30].zzzz)+(r0.yyyy)).x;
    // 97: mul r0.y, v4.z, cb0[31].x
    r0.y = ((v4.zzzz)*(source[31].xxxx)).y;
    // 98: mad r1.y, cb0[12].w, cb0[31].y, r0.y
    r1.y = ((source[12].wwww)*(source[31].yyyy)+(r0.yyyy)).y;
    // 99: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t3.yxzw, s4, l(0.000000)
    r0.y = (QNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 100: add r0.y, r0.y, l(0.200000)
    r0.y = ((r0.yyyy)+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 101: add r0.z, -cb0[1].w, l(1.000000)
    r0.z = ((-(source[1].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 102: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 103: mul_sat r0.y, r0.y, cb0[31].z
    r0.y = (saturate((r0.yyyy)*(source[31].zzzz))).y;
    // 104: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 105: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

// basic-add: b0d551a53864c9408d2c03215f9ef88f; selected map 4dd93e1b4ffae10c2db3ed0c4c9132c6db631aa6aa2fdbffcc7ea3e1b5d89ab4.
float4 QNative45(Q_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_QSourceMaterialParameters[2u];
    source[2].x = (g_QSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_QSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_QSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_QSourceMaterialParameters[0u].wwww).x;
    source[3].x = (g_QSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 3-8: device-Z reconstruction replaced by documented Target_Depth.y view-Z adapter.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // 9: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 10: add r0.y, -cb0[3].x, l(1.000000)
    r0.y = ((-(source[3].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: mad r0.y, cb0[2].x, v4.x, l(-1.000000)
    r0.y = ((source[2].xxxx)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 14: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 15: mul r0.z, v4.x, cb0[2].x
    r0.z = ((v4.xxxx)*(source[2].xxxx)).z;
    // 16: mad r0.yz, r0.zzzz, v2.xxyx, -r0.yyyy
    r0.yz = ((r0.zzzz)*(v2.xxyx)+(-(r0.yyyy))).yz;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t0.xyzw, s1, l(0.000000)
    r1.xyzw = (QNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 18: log r0.y, |r1.w|
    r0.y = (log2(abs(r1.wwww))).y;
    // 19: mul r0.y, r0.y, cb0[2].w
    r0.y = ((r0.yyyy)*(source[2].wwww)).y;
    // 20: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 21: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 22: lt r0.z, |r1.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 23: movc r0.y, r0.z, l(0), |r0.y|
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).y;
    // 24: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 25: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 26: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 27: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 28: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 29: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 30: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 31: mul r0.yzw, r1.xxyz, cb0[2].yyyy
    r0.yzw = ((r1.xxyz)*(source[2].yyyy)).yzw;
    // 32: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 33: mad r1.xyz, -cb0[2].yyyy, r1.xyzx, r1.wwww
    r1.xyz = ((-(source[2].yyyy))*(r1.xyzx)+(r1.wwww)).xyz;
    // 34: mad r0.yzw, cb0[2].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[2].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 35: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 36: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 37: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 38: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// missiletrail-sprite: cdef428a9b88db42a4d7a5fa664c6446; selected map 071ff3626fe9535a255d8f0a0fa0bae4a7336aa6056f63c8362302af85cfbf02.
float4 QNative46(Q_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_QSourceMaterialParameters[7u];
    source[2] = QNativeAppend(g_QSourceMaterialParameters[3u].wwww,g_QSourceMaterialParameters[3u].zzzz,1u);
    source[3] = QNativeAppend(cos((g_QSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_QSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = QNativeAppend(sin((g_QSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_QSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = QNativeAppend(g_QSourceMaterialParameters[1u].yyyy,g_QSourceMaterialParameters[1u].xxxx,1u);
    source[6].x = (cos((g_QSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[6].y = (g_QSourceMaterialParameters[3u].xxxx).x;
    source[6].z = (g_QSourceMaterialParameters[4u].xxxx).x;
    source[6].w = (g_QSourceMaterialParameters[3u].zzzz).x;
    source[7].x = (g_QSourceMaterialParameters[3u].wwww).x;
    source[7].y = (g_QSourceMaterialParameters[5u].zzzz).x;
    source[7].z = (g_QSourceMaterialParameters[5u].yyyy).x;
    source[7].w = (g_QSourceMaterialParameters[5u].xxxx).x;
    source[8].x = (g_QSourceMaterialParameters[6u].xxxx).x;
    source[8].y = (g_QSourceMaterialParameters[5u].wwww).x;
    source[8].z = (g_QSourceMaterialParameters[4u].yyyy).x;
    source[8].w = (g_QSourceMaterialParameters[3u].yyyy).x;
    source[9].x = (g_QSourceMaterialParameters[4u].zzzz).x;
    source[9].y = (g_QSourceMaterialParameters[4u].wwww).x;
    source[9].z = (g_QSourceMaterialParameters[2u].wwww).x;
    source[9].w = (g_QSourceMaterialParameters[1u].zzzz).x;
    source[10].x = (g_QSourceMaterialParameters[0u].wwww).x;
    source[10].y = (g_QSourceMaterialParameters[1u].xxxx).x;
    source[10].z = (g_QSourceMaterialParameters[1u].yyyy).x;
    source[10].w = (g_QSourceMaterialParameters[2u].xxxx).x;
    source[11].x = (g_QSourceMaterialParameters[0u].zzzz).x;
    source[11].y = (g_QSourceMaterialParameters[2u].zzzz).x;
    source[11].z = (g_QSourceMaterialParameters[0u].yyyy).x;
    source[11].w = (g_QSourceMaterialParameters[0u].xxxx).x;
    source[12].x = (g_QSourceMaterialParameters[2u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mul r0.x, v2.x, cb0[9].w
    r0.x = ((v2.xxxx)*(source[9].wwww)).x;
    // 2: mul r0.y, v2.y, cb0[10].x
    r0.y = ((v2.yyyy)*(source[10].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, cb0[5].xyxx
    r0.xy = ((r0.xyxx)+(source[5].xyxx)).xy;
    // 4: mul r0.zw, v4.zzzz, l(0.000000, 0.000000, 0.080000, -0.100000)
    r0.zw = ((v4.zzzz)*(float4(0.000000,0.000000,0.080000,-0.100000))).zw;
    // 5: mad r0.zw, v2.xxxy, cb0[7].yyyz, r0.zzzw
    r0.zw = ((v2.xxxy)*(source[7].yyyz)+(r0.zzzw)).zw;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (QNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 7: mov r1.xz, r0.zzzz
    r1.xz = (r0.zzzz).xz;
    // 8: mul r2.xyzw, v2.xyxy, cb0[8].xyzw
    r2.xyzw = ((v2.xyxy)*(source[8].xyzw)).xyzw;
    // 9: mad r0.zw, v4.zzzz, l(0.000000, 0.000000, -0.050000, -0.050000), r2.xxxy
    r0.zw = ((v4.zzzz)*(float4(0.000000,0.000000,-0.050000,-0.050000))+(r2.xxxy)).zw;
    // 10: mad r2.xy, v4.xxxx, l(0.080000, 3.000000, 0.000000, 0.000000), r2.zwzz
    r2.xy = ((v4.xxxx)*(float4(0.080000,3.000000,0.000000,0.000000))+(r2.zwzz)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.x = (QNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 12: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.xzyw, s0, l(0.000000)
    r0.z = (QNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).z;
    // 14: mov r1.yw, r0.zzzz
    r1.yw = (r0.zzzz).yw;
    // 15: mad r0.z, v2.y, cb0[7].z, cb0[7].w
    r0.z = ((v2.yyyy)*(source[7].zzzz)+(source[7].wwww)).z;
    // 16: mul r1.xyzw, r1.xyzw, r0.zzzz
    r1.xyzw = ((r1.xyzw)*(r0.zzzz)).xyzw;
    // 17: mul r1.xyzw, r1.xyzw, v4.yyyy
    r1.xyzw = ((r1.xyzw)*(v4.yyyy)).xyzw;
    // 18: mad r0.xy, r1.xyxx, l(0.400000, 0.400000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r1.xyxx)*(float4(0.400000,0.400000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 19: add r1.xy, v4.xwxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v4.xwxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 20: mad r0.z, r1.x, l(0.300000), r0.y
    r0.z = ((r1.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))+(r0.yyyy)).z;
    // 21: add r0.xy, r0.xzxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 22: dp2 r3.x, cb0[3].xyxx, r0.xyxx
    r3.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: dp2 r3.y, cb0[4].xyxx, r0.xyxx
    r3.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 24: add r0.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (QNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 26: mul_sat r0.x, r0.x, cb0[10].w
    r0.x = (saturate((r0.xxxx)*(source[10].wwww))).x;
    // 27: mul r0.yz, v2.xxyx, cb0[11].zzwz
    r0.yz = ((v2.xxyx)*(source[11].zzwz)).yz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (QNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 29: add r0.z, -v2.y, l(1.000000)
    r0.z = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 30: mad r0.y, r0.z, cb0[11].y, r0.y
    r0.y = ((r0.zzzz)*(source[11].yyyy)+(r0.yyyy)).y;
    // 31: mul r0.z, r0.z, v2.y
    r0.z = ((r0.zzzz)*(v2.yyyy)).z;
    // 32: mul_sat r0.z, r0.z, cb0[11].x
    r0.z = (saturate((r0.zzzz)*(source[11].xxxx))).z;
    // 33: add r0.y, -r1.y, r0.y
    r0.y = ((-(r1.yyyy))+(r0.yyyy)).y;
    // 34: mul r0.w, r1.x, cb0[6].y
    r0.w = ((r1.xxxx)*(source[6].yyyy)).w;
    // 35: mul_sat r0.y, r0.y, cb0[12].x
    r0.y = (saturate((r0.yyyy)*(source[12].xxxx))).y;
    // 36: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 37: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 38: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 39: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 40: mad r0.xy, v2.xyxx, cb0[6].zyzz, cb0[2].xyxx
    r0.xy = ((v2.xyxx)*(source[6].zyzz)+(source[2].xyxx)).xy;
    // 41: mad r0.xy, r1.zwzz, l(0.600000, 0.600000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r1.zwzz)*(float4(0.600000,0.600000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 42: mad r0.z, r0.w, l(0.300000), r0.y
    r0.z = ((r0.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r0.yyyy)).z;
    // 43: add r0.xy, r0.xzxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 44: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 45: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 46: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (QNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 48: mul r0.y, r2.x, r0.x
    r0.y = ((r2.xxxx)*(r0.xxxx)).y;
    // 49: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 50: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 51: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 52: mul r0.y, r0.y, cb0[9].x
    r0.y = ((r0.yyyy)*(source[9].xxxx)).y;
    // 53: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 54: mul r0.y, r0.y, cb0[9].y
    r0.y = ((r0.yyyy)*(source[9].yyyy)).y;
    // 55: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 56: mad r0.x, r0.x, cb0[9].z, r0.y
    r0.x = ((r0.xxxx)*(source[9].zzzz)+(r0.yyyy)).x;
    // 57: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 58: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// center-glow: ff1194d8453ded4fba5fe87ac55b4348; selected map 76880f6eef92d922eb8d9e84ec7ec5d9972d9282f897cd088bc864f08680555f.
float4 QNative47(Q_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_QSourceMaterialParameters[2u];
    source[2].x = (g_QSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_QSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_QSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_QSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_QSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 4: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 5: max r0.y, r0.x, l(0.000000)
    r0.y = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 6: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 9: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 10: mul r0.z, r0.z, cb0[2].x
    r0.z = ((r0.zzzz)*(source[2].xxxx)).z;
    // 11: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 12: mul r0.z, r0.z, cb0[2].y
    r0.z = ((r0.zzzz)*(source[2].yyyy)).z;
    // 13: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 14: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 15: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: mul r0.z, r0.z, cb0[2].z
    r0.z = ((r0.zzzz)*(source[2].zzzz)).z;
    // 17: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 18: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 19: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 20: mul r0.x, r0.x, cb0[2].w
    r0.x = ((r0.xxxx)*(source[2].wwww)).x;
    // 21: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 22: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 23: mul r0.y, r0.y, cb0[3].x
    r0.y = ((r0.yyyy)*(source[3].xxxx)).y;
    // 24: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 25: mul r0.yzw, r0.yyyy, v3.xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)).yzw;
    // 26: movc r0.xyz, r0.xxxx, l(0,0,0,0), r0.yzwy
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yzwy)).xyz;
    // 27: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 28: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 29: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 30: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 31: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// rot-turbulence: 4fe721e700949f4891b94dffc5f19da5; selected map d00414bb63739d4321c2bed6aa422075f5e8e97c589072244f2e0c129ea78639.
float4 QNative48(Q_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_QSourceMaterialParameters[8u];
    source[2].x = (g_QSourceMaterialParameters[4u].yyyy).x;
    source[2].y = (g_QSourceMaterialTime.xxxx).x;
    source[2].z = (g_QSourceMaterialParameters[5u].xxxx).x;
    source[2].w = (g_QSourceMaterialParameters[5u].yyyy).x;
    source[3].x = (g_QSourceMaterialParameters[4u].zzzz).x;
    source[3].y = (g_QSourceMaterialParameters[4u].wwww).x;
    source[3].z = (g_QSourceMaterialParameters[2u].zzzz).x;
    source[3].w = (g_QSourceMaterialParameters[2u].wwww).x;
    source[4].x = (g_QSourceMaterialParameters[1u].zzzz).x;
    source[4].y = (g_QSourceMaterialParameters[1u].wwww).x;
    source[4].z = (g_QSourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_QSourceMaterialParameters[2u].yyyy).x;
    source[5].x = (g_QSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_QSourceMaterialParameters[0u].wwww).x;
    source[5].z = (g_QSourceMaterialParameters[0u].xxxx).x;
    source[5].w = (g_QSourceMaterialParameters[0u].yyyy).x;
    source[6].x = (g_QSourceMaterialParameters[7u].xxxx).x;
    source[6].y = (g_QSourceMaterialParameters[6u].wwww).x;
    source[6].z = (g_QSourceMaterialParameters[3u].wwww).x;
    source[6].w = (g_QSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_QSourceMaterialParameters[3u].yyyy).x;
    source[7].y = (g_QSourceMaterialParameters[3u].zzzz).x;
    source[7].z = (g_QSourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_QSourceMaterialParameters[6u].yyyy).x;
    source[8].x = (g_QSourceMaterialParameters[6u].zzzz).x;
    source[8].y = (g_QSourceMaterialParameters[5u].wwww).x;
    source[8].z = (g_QSourceMaterialParameters[6u].xxxx).x;
    source[8].w = (g_QSourceMaterialParameters[5u].zzzz).x;
    source[9].x = (g_QSourceMaterialParameters[1u].yyyy).x;
    source[9].y = (g_QSourceMaterialParameters[1u].xxxx).x;
    source[9].z = (g_QSourceMaterialParameters[7u].zzzz).x;
    source[9].w = (g_QSourceMaterialParameters[7u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: mul r0.x, v2.x, cb0[7].w
    r0.x = ((v2.xxxx)*(source[7].wwww)).x;
    // 2: mul r0.y, v2.y, cb0[8].x
    r0.y = ((v2.yyyy)*(source[8].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, cb0[8].yzyy
    r0.xy = ((r0.xyxx)+(source[8].yzyy)).xy;
    // 4: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.zw, v4.xxxx, l(0.000000, 0.000000, 0.250000, -0.100000)
    r0.zw = ((v4.xxxx)*(float4(0.000000,0.000000,0.250000,-0.100000))).zw;
    // 6: sincos r1.x, r2.x, r0.w
    r1.x = (sin(r0.wwww)).x; r2.x = (cos(r0.wwww)).x;
    // 7: sincos r3.x, r4.x, r0.z
    r3.x = (sin(r0.zzzz)).x; r4.x = (cos(r0.zzzz)).x;
    // 8: mov r5.x, -r1.x
    r5.x = (-(r1.xxxx)).x;
    // 9: mov r5.y, r2.x
    r5.y = (r2.xxxx).y;
    // 10: mov r5.z, r1.x
    r5.z = (r1.xxxx).z;
    // 11: dp2 r1.y, r5.zyzz, r0.xyxx
    r1.y = (dot((r5.zyzz).xy,(r0.xyxx).xy).xxxx).y;
    // 12: dp2 r1.x, r5.yxyy, r0.xyxx
    r1.x = (dot((r5.yxyy).xy,(r0.xyxx).xy).xxxx).x;
    // 13: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t6.xyzw, s5, l(0.000000)
    r0.x = (QNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 15: mul r0.x, r0.x, cb0[8].w
    r0.x = ((r0.xxxx)*(source[8].wwww)).x;
    // 16: mul r0.y, v2.y, cb0[6].w
    r0.y = ((v2.yyyy)*(source[6].wwww)).y;
    // 17: mad r1.y, cb0[7].y, v4.y, r0.y
    r1.y = ((source[7].yyyy)*(v4.yyyy)+(r0.yyyy)).y;
    // 18: mad r1.x, v2.x, cb0[6].z, cb0[7].x
    r1.x = ((v2.xxxx)*(source[6].zzzz)+(source[7].xxxx)).x;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t5.yxzw, s4, l(0.000000)
    r0.y = (QNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 20: mad r0.x, r0.y, cb0[7].z, r0.x
    r0.x = ((r0.yyyy)*(source[7].zzzz)+(r0.xxxx)).x;
    // 21: add r0.xy, r0.xxxx, v2.xyxx
    r0.xy = ((r0.xxxx)+(v2.xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t7.xyzw, s6, l(0.000000)
    r0.x = (QNativeSample5((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 23: mul r0.x, r0.x, cb0[9].x
    r0.x = ((r0.xxxx)*(source[9].xxxx)).x;
    // 24: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 25: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 26: mul r0.y, r0.y, cb0[9].y
    r0.y = ((r0.yyyy)*(source[9].yyyy)).y;
    // 27: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 28: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 29: mov r1.x, -r3.x
    r1.x = (-(r3.xxxx)).x;
    // 30: mad r0.yz, v2.xxyx, cb0[5].xxyx, cb0[5].zzwz
    r0.yz = ((v2.xxyx)*(source[5].xxyx)+(source[5].zzwz)).yz;
    // 31: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 32: mov r1.y, r4.x
    r1.y = (r4.xxxx).y;
    // 33: mov r1.z, r3.x
    r1.z = (r3.xxxx).z;
    // 34: dp2 r2.y, r1.zyzz, r0.yzyy
    r2.y = (dot((r1.zyzz).xy,(r0.yzyy).xy).xxxx).y;
    // 35: dp2 r2.x, r1.yxyy, r0.yzyy
    r2.x = (dot((r1.yxyy).xy,(r0.yzyy).xy).xxxx).x;
    // 36: add r0.yz, r2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t4.yxzw, s3, l(0.000000)
    r0.y = (QNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 38: mul r0.y, r0.y, cb0[6].x
    r0.y = ((r0.yyyy)*(source[6].xxxx)).y;
    // 39: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 40: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 41: mul r0.y, r0.y, cb0[6].y
    r0.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 42: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 43: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 44: mul r0.z, r0.x, r0.y
    r0.z = ((r0.xxxx)*(r0.yyyy)).z;
    // 45: mul r1.xy, v2.xyxx, cb0[2].zwzz
    r1.xy = ((v2.xyxx)*(source[2].zwzz)).xy;
    // 46: mad r2.x, cb0[2].y, cb0[2].x, r1.x
    r2.x = ((source[2].yyyy)*(source[2].xxxx)+(r1.xxxx)).x;
    // 47: mad r2.y, cb0[2].y, cb0[3].x, r1.y
    r2.y = ((source[2].yyyy)*(source[3].xxxx)+(r1.yyyy)).y;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t2.yzwx, s1, l(0.000000)
    r0.w = (QNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 49: mad r1.xy, r0.wwww, cb0[3].yyyy, v2.xyxx
    r1.xy = ((r0.wwww)*(source[3].yyyy)+(v2.xyxx)).xy;
    // 50: mad r1.xy, r1.xyxx, cb0[3].zwzz, cb0[4].xyxx
    r1.xy = ((r1.xyxx)*(source[3].zwzz)+(source[4].xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t3.yzwx, s2, l(0.000000)
    r0.w = (QNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 52: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 53: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 54: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 55: mul r1.x, r1.x, cb0[4].z
    r1.x = ((r1.xxxx)*(source[4].zzzz)).x;
    // 56: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 57: mul r1.y, v4.z, cb0[4].w
    r1.y = ((v4.zzzz)*(source[4].wwww)).y;
    // 58: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 59: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 60: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 61: mul r1.x, r0.z, r0.w
    r1.x = ((r0.zzzz)*(r0.wwww)).x;
    // 62: dp3 r1.y, r1.xxxx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r1.xxxx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 63: mad r0.z, -r0.w, r0.z, r1.y
    r0.z = ((-(r0.wwww))*(r0.zzzz)+(r1.yyyy)).z;
    // 64: mad r0.z, cb0[9].z, r0.z, r1.x
    r0.z = ((source[9].zzzz)*(r0.zzzz)+(r1.xxxx)).z;
    // 65: mad r0.x, r0.y, r0.x, r0.z
    r0.x = ((r0.yyyy)*(r0.xxxx)+(r0.zzzz)).x;
    // 66: mad r0.xyz, v3.xyzx, r0.xxxx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)*(r0.xxxx)+(source[1].xyzx)).xyz;
    // 67: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 68: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 69: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 70-75: device-Z reconstruction replaced by documented Target_Depth.y view-Z adapter.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // 76: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 77: add r1.x, -cb0[9].w, l(1.000000)
    r1.x = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 78: mul r1.x, r1.x, l(100.000000)
    r1.x = ((r1.xxxx)*(float4(100.000000,100.000000,100.000000,100.000000))).x;
    // 79: max r1.x, r1.x, l(0.001000)
    r1.x = (max(r1.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 80: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v2.xyxx, t0.xyzw, s7, l(0.000000)
    r1.x = (QNativeSample6((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 82: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 83: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 84: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 85: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 86: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 87: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// simple-add: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 QNative49(Q_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_QSourceMaterialParameters[3u];
    source[2] = g_QSourceMaterialParameters[2u];
    source[3] = QNativeAppend(g_QSourceMaterialParameters[0u].wwww,g_QSourceMaterialParameters[1u].xxxx,1u);
    source[4] = QNativeAppend(g_QSourceMaterialParameters[1u].zzzz,g_QSourceMaterialParameters[1u].wwww,1u);
    source[5].x = (g_QSourceMaterialTime.xxxx).x;
    source[5].y = (g_QSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_QSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_QSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_QSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_QSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_QSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_QSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_QSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xy, cb0[5].xxxx, cb0[3].xyxx, v2.xyxx
    r0.xy = ((source[5].xxxx)*(source[3].xyxx)+(v2.xyxx)).xy;
    // 2: mul r0.xy, r0.xyxx, cb0[5].wwww
    r0.xy = ((r0.xyxx)*(source[5].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (QNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r0.zw, cb0[5].xxxx, cb0[4].xxxy, v2.xxxy
    r0.zw = ((source[5].xxxx)*(source[4].xxxy)+(v2.xxxy)).zw;
    // 5: mad r0.xy, cb0[6].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[6].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = (QNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

// local-crack: 8b228c7b319b544781cca408746673a2; selected map cb7fea6335f99773642abba576c9c60a2c351f25a720c8f239556eb64c8886fd.
float4 QNative50(Q_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_QSourceMaterialParameters[7u];
    source[3] = QNativeAppend(QNativePeriodic(((g_QSourceMaterialParameters[1u].wwww*g_QSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),QNativePeriodic(((g_QSourceMaterialParameters[1u].wwww*g_QSourceMaterialTime.xxxx)*float4(0.300000012, 0.0, 0.0, 0.0))),1u);
    source[4] = g_QSourceMaterialParameters[6u];
    source[5] = input.dynamicParameter;
    source[6] = g_QSourceMaterialParameters[3u];
    source[7] = g_QSourceMaterialParameters[5u];
    source[8].x = (g_QSourceMaterialParameters[0u].zzzz).x;
    source[8].y = (g_QSourceMaterialParameters[0u].wwww).x;
    source[8].z = (g_QSourceMaterialTime.xxxx).x;
    source[8].w = (g_QSourceMaterialParameters[1u].wwww).x;
    source[9].x = (QNativePeriodic(((g_QSourceMaterialParameters[1u].wwww*g_QSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_QSourceMaterialParameters[2u].wwww).x;
    source[9].z = (g_QSourceMaterialParameters[2u].yyyy).x;
    source[9].w = (g_QSourceMaterialParameters[2u].zzzz).x;
    source[10].x = (g_QSourceMaterialParameters[1u].yyyy).x;
    source[10].y = (g_QSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_QSourceMaterialParameters[1u].xxxx).x;
    source[10].w = (g_QSourceMaterialParameters[2u].xxxx).x;
    source[11].x = (g_QSourceMaterialParameters[0u].yyyy).x;
    source[11].y = (g_QSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=float4(input.uv,input.uv1.yx);
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: mul r0.zw, v4.xxxy, cb0[8].xxxy
    r0.zw = ((v4.xxxy)*(source[8].xxxy)).zw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s0, l(0.000000)
    r0.zw = (QNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 5: mad r1.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 7: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 8: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 9: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 10: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 11: dp3 r0.z, r1.xyzx, r1.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 12: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 13: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 14: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 15: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 16: mul r2.xyz, r0.zzzz, v6.xyzx
    r2.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // 17: dp3 r0.z, r1.xyzx, r2.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 18: mul r3.xyz, r0.zzzz, r1.xyzx
    r3.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 19: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 20: dp3 r0.z, r2.xyzx, r3.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 21: add r2.xy, r3.xyxx, cb0[3].xyxx
    r2.xy = ((r3.xyxx)+(source[3].xyxx)).xy;
    // 22: div r2.xy, r2.xyxx, cb0[9].yyyy
    r2.xy = ((r2.xyxx)/(source[9].yyyy)).xy;
    // 23: mad r2.xy, r2.xyxx, cb0[9].zwzz, cb0[10].xyxx
    r2.xy = ((r2.xyxx)*(source[9].zwzz)+(source[10].xyxx)).xy;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r2.xyw, r2.xyxx, t1.xywz, s1, l(0.000000)
    r2.xyw = (QNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 25: max r0.w, r2.z, l(0.000000)
    r0.w = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 26: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: add r1.w, -|r0.z|, l(1.000000)
    r1.w = ((-(abs(r0.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: mad r0.xy, r1.wwww, cb0[5].xxxx, r0.xyxx
    r0.xy = ((r1.wwww)*(source[5].xxxx)+(r0.xyxx)).xy;
    // 29: add r1.w, cb0[5].y, cb0[5].y
    r1.w = ((source[5].yyyy)+(source[5].yyyy)).w;
    // 30: div r0.xy, r0.xyxx, r1.wwww
    r0.xy = ((r0.xyxx)/(r1.wwww)).xy;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = (QNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 33: add r4.xyz, -r3.xyzx, r0.xxxx
    r4.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 34: mad r3.xyz, r4.xyzx, l(0.880000, 0.880000, 0.880000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.880000,0.880000,0.880000,0.000000))+(r3.xyzx)).xyz;
    // 35: max r4.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 36: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 37: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 38: mad r3.xyz, -r4.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000), r3.xyzx
    r3.xyz = ((-(r4.xyzx))*(float4(4.000000,4.000000,4.000000,0.000000))+(r3.xyzx)).xyz;
    // 39: mul r4.xyz, r4.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(4.000000,4.000000,4.000000,0.000000))).xyz;
    // 40: mad_sat r0.xyz, |r0.zzzz|, r3.xyzx, r4.xyzx
    r0.xyz = (saturate((abs(r0.zzzz))*(r3.xyzx)+(r4.xyzx))).xyz;
    // 41: mul r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)).xyz;
    // 42: mul r1.w, |r0.w|, |r0.w|
    r1.w = ((abs(r0.wwww))*(abs(r0.wwww))).w;
    // 43: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 44: mul r1.w, |r0.w|, r1.w
    r1.w = ((abs(r0.wwww))*(r1.wwww)).w;
    // 45: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 46: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 47: mul r3.xyz, r0.wwww, cb0[7].xyzx
    r3.xyz = ((r0.wwww)*(source[7].xyzx)).xyz;
    // 48: mul r0.w, r0.w, l(5.000000)
    r0.w = ((r0.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 49: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mad r0.xyz, r0.xyzx, cb0[6].xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(source[6].xyzx)+(r3.xyzx)).xyz;
    // 51: dp3 r1.w, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 52: add r3.xyz, -r2.xywx, r1.wwww
    r3.xyz = ((-(r2.xywx))+(r1.wwww)).xyz;
    // 53: mad r2.xyz, cb0[10].zzzz, r3.xyzx, r2.xywx
    r2.xyz = ((source[10].zzzz)*(r3.xyzx)+(r2.xywx)).xyz;
    // 54: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 55: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 56: mul r2.xyz, r2.xyzx, cb0[10].wwww
    r2.xyz = ((r2.xyzx)*(source[10].wwww)).xyz;
    // 57: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 58: mad r0.xyz, r2.xyzx, cb0[4].xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(source[4].xyzx)+(r0.xyzx)).xyz;
    // 59: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 60: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 61: log r0.x, r0.w
    r0.x = (log2(r0.wwww)).x;
    // 62: lt r0.y, r0.w, l(0.000001)
    r0.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 63: mul r0.x, r0.x, cb0[11].x
    r0.x = ((r0.xxxx)*(source[11].xxxx)).x;
    // 64: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 65: mov_sat r0.z, cb0[1].w
    r0.z = (saturate(source[1].wwww)).z;
    // 66: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 67: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 68: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}

// glass-hole: 377108e10f08cc488de94c405e789871; selected map 126881131d06544eff5f47c996bae29975b64dda72346c14c0b7e28b17025d9f.
float4 QNative51(Q_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_QSourceMaterialParameters[10u];
    source[2] = QNativeAppend((g_QSourceMaterialParameters[4u].xxxx*g_QSourceMaterialTime.xxxx),(g_QSourceMaterialParameters[4u].yyyy*g_QSourceMaterialTime.xxxx),1u);
    source[3] = QNativeAppend(QNativePeriodic((g_QSourceMaterialTime.xxxx*float4(0.00999999978, 0.0, 0.0, 0.0))),QNativePeriodic((g_QSourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),1u);
    source[4] = QNativeAppend(g_QSourceMaterialParameters[0u].zzzz,g_QSourceMaterialParameters[0u].wwww,1u);
    source[5] = QNativeAppend(g_QSourceMaterialParameters[0u].xxxx,g_QSourceMaterialParameters[0u].yyyy,1u);
    source[6] = QNativeAppend(g_QSourceMaterialParameters[1u].wwww,g_QSourceMaterialParameters[2u].xxxx,1u);
    source[7] = QNativeAppend(QNativePeriodic(((g_QSourceMaterialTime.xxxx*g_QSourceMaterialParameters[6u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),QNativePeriodic(((g_QSourceMaterialTime.xxxx*g_QSourceMaterialParameters[6u].xxxx)*float4(-0.230000004, 0.0, 0.0, 0.0))),1u);
    source[8] = QNativeAppend(QNativePeriodic(((g_QSourceMaterialTime.xxxx*g_QSourceMaterialParameters[6u].xxxx)*float4(-0.5, 0.0, 0.0, 0.0))),QNativePeriodic(((g_QSourceMaterialTime.xxxx*g_QSourceMaterialParameters[6u].xxxx)*float4(0.370000005, 0.0, 0.0, 0.0))),1u);
    source[9] = g_QSourceMaterialParameters[9u];
    source[10] = QNativeAppend(QNativePeriodic((g_QSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),QNativePeriodic((g_QSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))),1u);
    source[11] = QNativeAppend(QNativePeriodic((g_QSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),QNativePeriodic((g_QSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0))),1u);
    source[12] = g_QSourceMaterialParameters[8u];
    source[13] = QNativeAppend(QNativePeriodic((g_QSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),QNativePeriodic((g_QSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[14].x = ((g_QSourceMaterialParameters[2u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[14].y = (g_QSourceMaterialParameters[2u].wwww).x;
    source[14].z = ((g_QSourceMaterialParameters[2u].wwww*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[14].w = (g_QSourceMaterialParameters[5u].wwww).x;
    source[15].x = (QNativePeriodic(((g_QSourceMaterialTime.xxxx*g_QSourceMaterialParameters[6u].xxxx)*float4(0.370000005, 0.0, 0.0, 0.0)))).x;
    source[15].y = (QNativePeriodic(((g_QSourceMaterialTime.xxxx*g_QSourceMaterialParameters[6u].xxxx)*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[15].z = (g_QSourceMaterialParameters[1u].zzzz).x;
    source[15].w = (g_QSourceMaterialParameters[3u].yyyy).x;
    source[16].x = (g_QSourceMaterialParameters[3u].wwww).x;
    source[16].y = (g_QSourceMaterialParameters[4u].zzzz).x;
    source[16].z = (g_QSourceMaterialParameters[4u].wwww).x;
    source[16].w = (g_QSourceMaterialParameters[3u].zzzz).x;
    source[17].x = ((g_QSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[17].y = ((g_QSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))).x;
    source[17].z = (g_QSourceMaterialParameters[5u].yyyy).x;
    source[17].w = (g_QSourceMaterialParameters[6u].yyyy).x;
    source[18].x = (g_QSourceMaterialParameters[5u].xxxx).x;
    source[18].y = (g_QSourceMaterialParameters[2u].yyyy).x;
    source[18].z = (g_QSourceMaterialParameters[5u].zzzz).x;
    source[18].w = (g_QSourceMaterialParameters[6u].wwww).x;
    source[19].x = (g_QSourceMaterialParameters[7u].xxxx).x;
    source[19].y = (g_QSourceMaterialParameters[6u].zzzz).x;
    source[19].z = ((g_QSourceMaterialParameters[6u].zzzz*g_QSourceMaterialTime.xxxx)).x;
    source[19].w = (QNativePeriodic((g_QSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0)))).x;
    source[20].x = (QNativePeriodic((g_QSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[20].y = ((g_QSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0))).x;
    source[20].z = (QNativePeriodic((g_QSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0)))).x;
    source[20].w = (g_QSourceMaterialParameters[1u].xxxx).x;
    source[21].x = (g_QSourceMaterialParameters[1u].yyyy).x;
    source[21].y = ((g_QSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[21].z = (QNativePeriodic((g_QSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[21].w = (g_QSourceMaterialParameters[3u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
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
    // 27: mul r0.y, r0.y, cb0[17].z
    r0.y = ((r0.yyyy)*(source[17].zzzz)).y;
    // 28: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: dp2 r0.z, r0.zzzz, cb0[17].wwww
    r0.z = (dot((r0.zzzz).xy,(source[17].wwww).xy).xxxx).z;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(asfloat(0x3ea2f983u),asfloat(0x3ea2f983u),asfloat(0x3ea2f983u),asfloat(0x3ea2f983u)))+(r0.zzzz)).y;
    // 31: add r0.y, r0.y, cb0[18].x
    r0.y = ((r0.yyyy)+(source[18].xxxx)).y;
    // 32: mul r1.x, r0.y, cb0[18].w
    r1.x = ((r0.yyyy)*(source[18].wwww)).x;
    // 33: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 34: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 35: mul r0.y, r0.y, cb0[18].y
    r0.y = ((r0.yyyy)*(source[18].yyyy)).y;
    // 36: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 37: mul r0.y, r0.y, v4.w
    r0.y = ((r0.yyyy)*(v4.wwww)).y;
    // 38: lt r0.z, r0.x, l(0.000000)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu),asfloat(0x350637bdu)))) * 0xffffffffu)).z;
    // 39: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 40: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 41: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 42: add r0.z, v4.z, cb0[18].z
    r0.z = ((v4.zzzz)+(source[18].zzzz)).z;
    // 43: mad r0.y, r0.z, l(-0.400000), r0.y
    r0.y = ((r0.zzzz)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.yyyy)).y;
    // 44: mul r0.z, v4.w, cb0[19].z
    r0.z = ((v4.wwww)*(source[19].zzzz)).z;
    // 45: mad r1.y, r0.y, cb0[19].x, r0.z
    r1.y = ((r0.yyyy)*(source[19].xxxx)+(r0.zzzz)).y;
    // 46: add r0.yz, r1.xxyx, cb0[10].xxyx
    r0.yz = ((r1.xxyx)+(source[10].xxyx)).yz;
    // 47: add r1.xy, r1.xyxx, cb0[11].xyxx
    r1.xy = ((r1.xyxx)+(source[11].xyxx)).xy;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s5, l(-1.000000)
    r1.xyz = (QNativeSample4((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 49: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t4.wxyz, s4, l(-1.000000)
    r0.yzw = (QNativeSample3((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 50: mul r2.xyz, r1.xyzx, r0.yzwy
    r2.xyz = ((r1.xyzx)*(r0.yzwy)).xyz;
    // 51: add r0.yzw, r1.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)+(r0.yyzw)).yzw;
    // 52: max r1.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 53: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 54: mul r1.xyz, r1.xyzx, cb0[20].wwww
    r1.xyz = ((r1.xyzx)*(source[20].wwww)).xyz;
    // 55: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 56: add r2.xy, v2.xyxx, -cb0[4].xyxx
    r2.xy = ((v2.xyxx)+(-(source[4].xyxx))).xy;
    // 57: max r1.w, |r2.y|, |r2.x|
    r1.w = (max(abs(r2.yyyy),abs(r2.xxxx))).w;
    // 58: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 59: min r2.z, |r2.y|, |r2.x|
    r2.z = (min(abs(r2.yyyy),abs(r2.xxxx))).z;
    // 60: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 61: mul r2.z, r1.w, r1.w
    r2.z = ((r1.wwww)*(r1.wwww)).z;
    // 62: mad r2.w, r2.z, l(0.020835), l(-0.085133)
    r2.w = ((r2.zzzz)*(float4(asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu),asfloat(0x3caaae5fu)))+(float4(asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u),asfloat(0xbdae5a36u)))).w;
    // 63: mad r2.w, r2.z, r2.w, l(0.180141)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(0.180141,0.180141,0.180141,0.180141))).w;
    // 64: mad r2.w, r2.z, r2.w, l(-0.330299)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u),asfloat(0xbea91d04u)))).w;
    // 65: mad r2.z, r2.z, r2.w, l(0.999866)
    r2.z = ((r2.zzzz)*(r2.wwww)+(float4(0.999866,0.999866,0.999866,0.999866))).z;
    // 66: mul r2.w, r1.w, r2.z
    r2.w = ((r1.wwww)*(r2.zzzz)).w;
    // 67: mad r2.w, r2.w, l(-2.000000), l(1.570796)
    r2.w = ((r2.wwww)*(float4(asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u),asfloat(0xc0000000u)))+(float4(asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu),asfloat(0x3fc90fdbu)))).w;
    // 68: lt r3.x, |r2.y|, |r2.x|
    r3.x = (asfloat((uint4)((abs(r2.yyyy))<(abs(r2.xxxx))) * 0xffffffffu)).x;
    // 69: and r2.w, r2.w, r3.x
    r2.w = (asfloat(asuint(r2.wwww) & asuint(r3.xxxx))).w;
    // 70: mad r1.w, r1.w, r2.z, r2.w
    r1.w = ((r1.wwww)*(r2.zzzz)+(r2.wwww)).w;
    // 71: lt r2.z, r2.y, -r2.y
    r2.z = (asfloat((uint4)((r2.yyyy)<(-(r2.yyyy))) * 0xffffffffu)).z;
    // 72: and r2.z, r2.z, l(0xc0490fdb)
    r2.z = (asfloat(asuint(r2.zzzz) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).z;
    // 73: add r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)+(r2.zzzz)).w;
    // 74: min r2.z, r2.y, r2.x
    r2.z = (min(r2.yyyy,r2.xxxx)).z;
    // 75: max r2.x, r2.y, r2.x
    r2.x = (max(r2.yyyy,r2.xxxx)).x;
    // 76: ge r2.x, r2.x, -r2.x
    r2.x = (asfloat((uint4)((r2.xxxx)>=(-(r2.xxxx))) * 0xffffffffu)).x;
    // 77: lt r2.y, r2.z, -r2.z
    r2.y = (asfloat((uint4)((r2.zzzz)<(-(r2.zzzz))) * 0xffffffffu)).y;
    // 78: and r2.x, r2.x, r2.y
    r2.x = (asfloat(asuint(r2.xxxx) & asuint(r2.yyyy))).x;
    // 79: movc r1.w, r2.x, -r1.w, r1.w
    r1.w = ((asuint(r2.xxxx) != 0u) ? (-(r1.wwww)) : (r1.wwww)).w;
    // 80: mad r2.x, r1.w, l(0.159155), l(0.500000)
    r2.x = ((r1.wwww)*(float4(asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u),asfloat(0x3e22f983u)))+(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).x;
    // 81: mad r3.xy, v2.xyxx, cb0[6].xyxx, cb0[7].xyxx
    r3.xy = ((v2.xyxx)*(source[6].xyxx)+(source[7].xyxx)).xy;
    // 82: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r1.w = (QNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 83: mad r1.w, r1.w, l(2.000000), l(-1.000000)
    r1.w = ((r1.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 84: mad r3.xy, v2.xyxx, cb0[6].xyxx, cb0[8].xyxx
    r3.xy = ((v2.xyxx)*(source[6].xyxx)+(source[8].xyxx)).xy;
    // 85: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r2.w = (QNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 86: mad r2.w, r2.w, l(2.000000), l(-1.000000)
    r2.w = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 87: add r3.xy, v4.xyxx, l(-0.500000, -0.900000, 0.000000, 0.000000)
    r3.xy = ((v4.xyxx)+(float4(-0.500000,-0.900000,0.000000,0.000000))).xy;
    // 88: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 89: mad r1.w, r3.y, r1.w, r2.w
    r1.w = ((r3.yyyy)*(r1.wwww)+(r2.wwww)).w;
    // 90: mul r1.w, r1.w, cb0[15].z
    r1.w = ((r1.wwww)*(source[15].zzzz)).w;
    // 91: add r3.yz, v2.xxyx, -cb0[5].xxyx
    r3.yz = ((v2.xxyx)+(-(source[5].xxyx))).yz;
    // 92: dp2 r2.w, r3.yzyy, r3.yzyy
    r2.w = (dot((r3.yzyy).xy,(r3.yzyy).xy).xxxx).w;
    // 93: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 94: mad r3.y, -r2.w, l(2.000000), l(1.000000)
    r3.y = ((-(r2.wwww))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 95: mad r2.w, r2.w, l(3.000000), -r3.x
    r2.w = ((r2.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))+(-(r3.xxxx))).w;
    // 96: max r2.w, r2.w, l(-0.100000)
    r2.w = (max(r2.wwww,float4(-0.100000,-0.100000,-0.100000,-0.100000))).w;
    // 97: min r2.y, r2.w, l(10.000000)
    r2.y = (min(r2.wwww,float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 98: mad r3.xy, r3.yyyy, r1.wwww, r2.xyxx
    r3.xy = ((r3.yyyy)*(r1.wwww)+(r2.xyxx)).xy;
    // 99: sample_l_indexable(texture2d)(float,float,float,float) r2.yz, r3.xyxx, t1.yzxw, s2, l(-1.000000)
    r2.yz = (QNativeSample1((r3.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).yz;
    // 100: mul r3.xyz, r0.yzwy, r2.zzzz
    r3.xyz = ((r0.yzwy)*(r2.zzzz)).xyz;
    // 101: mul r3.xyz, r3.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 102: mad r1.xyz, cb0[21].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[21].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 103: mad r2.xw, r2.xxxz, l(7.000000, 0.000000, 0.000000, 1.000000), cb0[13].xxxy
    r2.xw = ((r2.xxxz)*(float4(7.000000,0.000000,0.000000,1.000000))+(source[13].xxxy)).xw;
    // 104: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xwxx, t6.xyzw, s6, l(0.000000)
    r3.xyz = (QNativeSample5((r2.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 105: mul r3.xyz, r2.zzzz, r3.xyzx
    r3.xyz = ((r2.zzzz)*(r3.xyzx)).xyz;
    // 106: mul_sat r3.xyz, r3.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000)
    r3.xyz = (saturate((r3.xyzx)*(float4(4.000000,4.000000,4.000000,0.000000)))).xyz;
    // 107: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 108: mad r1.xyz, r1.xyzx, cb0[12].xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(source[12].xyzx)+(r3.xyzx)).xyz;
    // 109: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 110: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 111: mul r2.xw, r0.wwww, v6.xxxy
    r2.xw = ((r0.wwww)*(v6.xxxy)).xw;
    // 112: mad r0.yz, r0.yyzy, l(0.000000, 0.200000, 0.200000, 0.000000), r2.xxwx
    r0.yz = ((r0.yyzy)*(float4(0.000000,0.200000,0.200000,0.000000))+(r2.xxwx)).yz;
    // 113: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t7.wxyz, s7, l(0.000000)
    r0.yzw = (QNativeSample6((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 114: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 115: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 116: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 117: mad r0.yzw, cb0[21].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[21].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 118: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 119: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 120-125: device-Z reconstruction replaced by documented Target_Depth.y view-Z adapter.
    r1.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // 126: add r1.x, r1.x, -v7.w
    r1.x = ((r1.xxxx)+(-(v7.wwww))).x;
    // 127: mul_sat r1.xy, r1.xxxx, l(0.034483, 0.066667, 0.000000, 0.000000)
    r1.xy = (saturate((r1.xxxx)*(float4(asfloat(0x3d0d3dcbu),asfloat(0x3d888889u),asfloat(0x00000000u),asfloat(0x00000000u))))).xy;
    // 128: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 129: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 130: lt r1.y, r1.y, l(0.000001)
    r1.y = (asfloat((uint4)((r1.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 131: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 132: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 133: mul r3.xyz, r1.zzzz, l(10.000000, 10.000000, 20.000000, 0.000000)
    r3.xyz = ((r1.zzzz)*(float4(10.000000,10.000000,20.000000,0.000000))).xyz;
    // 134: movc r1.yzw, r1.yyyy, l(0,0,0,0), r3.xxyz
    r1.yzw = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxyz)).yzw;
    // 135: mad r0.yzw, r2.yyyy, r0.yyzw, r1.yyzw
    r0.yzw = ((r2.yyyy)*(r0.yyzw)+(r1.yyzw)).yzw;
    // 136: mul r0.yzw, r0.yyzw, v3.xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)).yzw;
    // 137: add r1.y, -r2.y, l(1.000000)
    r1.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 138: add_sat r1.z, r1.y, r2.z
    r1.z = (saturate((r1.yyyy)+(r2.zzzz))).z;
    // 139: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 140: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 141: mul o0.w, r1.x, cb0[0].x
    output.w = ((r1.xxxx)*(source[0].xxxx)).w;
    // 142: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 143: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 144: mul r1.x, r1.x, cb0[14].x
    r1.x = ((r1.xxxx)*(source[14].xxxx)).x;
    // 145: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 146: mul r1.x, r1.x, cb0[14].z
    r1.x = ((r1.xxxx)*(source[14].zzzz)).x;
    // 147: add r1.zw, -v2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((-(v2.xxxy))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 148: mul r1.xz, r1.zzwz, r1.xxxx
    r1.xz = ((r1.zzwz)*(r1.xxxx)).xz;
    // 149: movc r1.xz, r0.xxxx, l(0,0,0,0), r1.xxzx
    r1.xz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxzx)).xz;
    // 150: add r1.xz, r1.xxzx, v2.xxyx
    r1.xz = ((r1.xxzx)+(v2.xxyx)).xz;
    // 151: mad r1.xz, cb0[14].wwww, r1.xxzx, cb0[2].xxyx
    r1.xz = ((source[14].wwww)*(r1.xxzx)+(source[2].xxyx)).xz;
    // 152: add r1.xz, r1.xxzx, cb0[3].xxyx
    r1.xz = ((r1.xxzx)+(source[3].xxyx)).xz;
    // 153: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v2.xyxx, t0.zxyw, s1, l(0.000000)
    r2.yz = (QNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 154: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 155: mul r2.yz, r1.yyyy, r2.yyzy
    r2.yz = ((r1.yyyy)*(r2.yyzy)).yz;
    // 156: mad r1.xz, cb0[15].wwww, r2.yyzy, r1.xxzx
    r1.xz = ((source[15].wwww)*(r2.yyzy)+(r1.xxzx)).xz;
    // 157: mad r0.x, cb0[16].x, l(0.500000), l(-0.250000)
    r0.x = ((source[16].xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(-0.250000,-0.250000,-0.250000,-0.250000))).x;
    // 158: mad r1.xz, r0.xxxx, r2.xxwx, r1.xxzx
    r1.xz = ((r0.xxxx)*(r2.xxwx)+(r1.xxzx)).xz;
    // 159: sample_b_indexable(texture2d)(float,float,float,float) r1.xzw, r1.xzxx, t3.xwyz, s3, l(0.000000)
    r1.xzw = (QNativeSample2((r1.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 160: max r1.xzw, |r1.xxzw|, l(0.000001, 0.000000, 0.000001, 0.000001)
    r1.xzw = (max(abs(r1.xxzw),float4(0.000001,0.000000,0.000001,0.000001))).xzw;
    // 161: log r1.xzw, r1.xxzw
    r1.xzw = (log2(r1.xxzw)).xzw;
    // 162: mul r1.xzw, r1.xxzw, cb0[16].yyyy
    r1.xzw = ((r1.xxzw)*(source[16].yyyy)).xzw;
    // 163: exp r1.xzw, r1.xxzw
    r1.xzw = (exp2(r1.xxzw)).xzw;
    // 164: mul r2.xyz, r1.xzwx, cb0[16].zzzz
    r2.xyz = ((r1.xzwx)*(source[16].zzzz)).xyz;
    // 165: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 166: mad r1.xzw, -cb0[16].zzzz, r1.xxzw, r0.xxxx
    r1.xzw = ((-(source[16].zzzz))*(r1.xxzw)+(r0.xxxx)).xzw;
    // 167: mad r1.xzw, cb0[16].wwww, r1.xxzw, r2.xxyz
    r1.xzw = ((source[16].wwww)*(r1.xxzw)+(r2.xxyz)).xzw;
    // 168: mul r1.xzw, r1.xxzw, cb0[9].xxyz
    r1.xzw = ((r1.xxzw)*(source[9].xxyz)).xzw;
    // 169: mad r0.xyz, r1.yyyy, r1.xzwx, r0.yzwy
    r0.xyz = ((r1.yyyy)*(r1.xzwx)+(r0.yzwy)).xyz;
    // 170: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 171: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// R authoring overlay: preserve the original fill by default. The clean-core
// identity additionally suppresses native grain only inside the authored coverage.
float4 Apply_ProjectTunedRBlackCore(float4 nativeColor, float2 uv,
    float sourceDepthFade, float particleAlpha, float2 coreParameters,
    float cleanSurface = 0.f)
{
    if (coreParameters.x <= 0.f)
        return nativeColor;
    const float2 centered = (uv - 0.5f) / float2(0.48f, coreParameters.x);
    const float coverage = 1.f - smoothstep(
        1.f - coreParameters.y, 1.f, length(centered));
    const float coreAlpha = saturate(coverage * sourceDepthFade * particleAlpha);
    if (coreAlpha <= 0.f)
        return nativeColor;
    const float combinedAlpha = nativeColor.a + coreAlpha * (1.f - nativeColor.a);
    const float nativeContribution = 1.f - saturate(coverage * cleanSurface);
    return float4(nativeColor.rgb * (nativeColor.a / combinedAlpha) *
        nativeContribution, combinedAlpha);
}

EFFECT_PS_OUT Shade_EffectDimensionMasterQNative(uint profile, float2 uv,
    float2 sourceUV1, float2 pixelPosition, float sourceProjectionW,
    float3 tangentView, float4 particleColor, float4 rawDynamic)
{
    EFFECT_PS_OUT output=(EFFECT_PS_OUT)0;
    uint width=0u,height=0u;
    g_EffectSceneDepthTexture.GetDimensions(width,height);
    if (width==0u || height==0u || profile<44u || profile>51u)
    { clip(-1.f); return output; }
    Q_NATIVE_INPUT input;
    input.uv=uv; input.uv1=sourceUV1;
    input.screenUV=pixelPosition/float2(width,height);
    input.projectionW=sourceProjectionW*100.f;
    input.tangentView=tangentView;
    input.color=particleColor; input.dynamicParameter=rawDynamic;
    float4 nativeColor=0.f;
    switch (profile)
    {
    case 44u: nativeColor=QNative44(input); break;
    case 45u: nativeColor=QNative45(input); break;
    case 46u: nativeColor=QNative46(input); break;
    case 47u: nativeColor=QNative47(input); break;
    case 48u: nativeColor=QNative48(input); break;
    case 49u: nativeColor=QNative49(input); break;
    case 50u: nativeColor=QNative50(input); break;
    case 51u: nativeColor=QNative51(input); break;
    }
    if (profile == 51u && g_QSourceMaterialParameters[11u].z > 1.5f)
    {
        // Authored dust treatment: native black has zero coverage. Keep the
        // moving native boundary, its depth fade and particle fade, in purple.
        const float rim = saturate(max(nativeColor.r,
            max(nativeColor.g, nativeColor.b)) * g_QSourceMaterialParameters[11u].x);
        nativeColor = float4(float3(0.48f, 0.12f, 0.90f),
            nativeColor.a * rim * g_QSourceMaterialParameters[11u].y);
    }
    else if (profile == 51u && g_QSourceMaterialParameters[11u].x > 0.f)
    {
        // Match Q51 RT0 opacity's 29 cm source depth fade; the fill must not
        // survive an occluding surface or the source particle alpha fade.
        const float sceneDepthCM = g_EffectSceneDepthTexture.SampleLevel(
            EffectSliceDepthSampler, input.screenUV, 0.f).y * 100000.f;
        const float sourceDepthFade = saturate(
            (sceneDepthCM - input.projectionW) * 0.034483f);
        nativeColor = Apply_ProjectTunedRBlackCore(nativeColor, input.uv,
            sourceDepthFade, input.color.a, g_QSourceMaterialParameters[11u].xy,
            g_QSourceMaterialParameters[11u].z);
    }
    // Native additive PS already weights RGB by opacity and writes A=0.
    // Product additive uses SrcAlpha, so A=1 preserves that native RGB once.
    const bool additive=profile==45u || profile==47u || profile==48u || profile==49u;
    output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,
        additive ? 1.f : nativeColor.a);
    output.Distortion=0.f; // Separate native distortion/MRT passes not claimed.
    if (g_ColorClip>0.f) clip(output.SceneColor.a-g_ColorClip);
    return output;
}
#endif
