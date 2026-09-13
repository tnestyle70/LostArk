// W/R selected source RT0 operations; exact ALT_V matches reuse existing programs.
#ifndef EFFECT_DIMENSIONMASTER_WR_NATIVE_HLSLI
#define EFFECT_DIMENSIONMASTER_WR_NATIVE_HLSLI
#include "Shader_EffectDimensionMasterVNativeShared.hlsli"
// Mutually exclusive W/R draws reuse V parameter transport and sampler helpers.
struct WR_NATIVE_INPUT
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

// fx_m_mi_j_00.fx_mi.fx_j_pa_spriteinvert_01_02_tr; source PS 743c811e01b5c74fabe2852946ed88f0.
float4 WRNative208(WR_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[4u];
    source[2].x = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[2].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[2].w = ((g_VSourceMaterialParameters[0u].xxxx+float4(0.0, 0.0, 0.0, 0.0))).x;
    source[3].x = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[3].y = ((g_VSourceMaterialParameters[0u].yyyy+float4(0.0, 0.0, 0.0, 0.0))).x;
    source[3].z = (g_VSourceMaterialParameters[2u].wwww).x;
    source[3].w = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[4].x = (g_VSourceMaterialTime.xxxx).x;
    source[4].y = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[4].z = ((g_VSourceMaterialParameters[2u].xxxx*g_VSourceMaterialTime.xxxx)).x;
    source[4].w = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[5].x = ((g_VSourceMaterialParameters[2u].yyyy*g_VSourceMaterialTime.xxxx)).x;
    source[5].y = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[5].z = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[5].w = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[6].z = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_VSourceMaterialParameters[1u].wwww).x;
    source[7].x = ((sin(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[3u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[7].y = (abs((sin(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[3u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0)))).x;
    source[7].z = (fmod(abs((sin(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[3u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))),float4(1.5, 0.0, 0.0, 0.0))).x;
    source[7].w = ((fmod(abs((sin(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[3u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))),float4(1.5, 0.0, 0.0, 0.0))+float4(0.300000012, 0.0, 0.0, 0.0))).x;
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
    // 1: mad r0.x, v2.x, cb0[3].z, cb0[4].z
    r0.x = ((v2.xxxx)*(source[3].zzzz)+(source[4].zzzz)).x;
    // 2: mad r0.y, v2.y, cb0[3].w, cb0[5].x
    r0.y = ((v2.yyyy)*(source[3].wwww)+(source[5].xxxx)).y;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (VNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 5: mul r0.y, r0.y, v4.x
    r0.y = ((r0.yyyy)*(v4.xxxx)).y;
    // 6: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 7: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 8: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 9: add r0.z, v4.y, l(-1.000000)
    r0.z = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 10: mul r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)*(source[5].yyyy)).z;
    // 11: mad r1.x, v2.x, cb0[2].x, cb0[2].w
    r1.x = ((v2.xxxx)*(source[2].xxxx)+(source[2].wwww)).x;
    // 12: mad r1.y, v2.y, cb0[2].y, cb0[3].y
    r1.y = ((v2.yyyy)*(source[2].yyyy)+(source[3].yyyy)).y;
    // 13: mad r0.xz, r0.xxxx, r0.zzzz, r1.xxyx
    r0.xz = ((r0.xxxx)*(r0.zzzz)+(r1.xxyx)).xz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t1.xyzw, s1, l(0.000000)
    r0.x = (VNativeSample1((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 15: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 16: mul r0.y, r0.x, cb0[5].z
    r0.y = ((r0.xxxx)*(source[5].zzzz)).y;
    // 17: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 18: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 19: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 20: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 21: mul_sat r0.z, r0.z, cb0[6].x
    r0.z = (saturate((r0.zzzz)*(source[6].xxxx))).z;
    // 22: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 23: mad_sat r0.x, -r0.x, cb0[6].y, r0.y
    r0.x = (saturate((-(r0.xxxx))*(source[6].yyyy)+(r0.yyyy))).x;
    // 24: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 25: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 26: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 27: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 28: mul r0.y, r0.y, cb0[6].z
    r0.y = ((r0.yyyy)*(source[6].zzzz)).y;
    // 29: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 30: mul r0.y, r0.y, cb0[6].w
    r0.y = ((r0.yyyy)*(source[6].wwww)).y;
    // 31: mul r0.y, r0.y, cb0[7].w
    r0.y = ((r0.yyyy)*(source[7].wwww)).y;
    // 32: mul r0.yzw, r0.yyyy, v3.xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)).yzw;
    // 33: movc r0.xyz, r0.xxxx, l(0,0,0,0), r0.yzwy
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yzwy)).xyz;
    // 34: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 35: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_mi_02.fx_m.fx_j_pa_dot_ad_01; source PS cdfe9a7700a1b745b352dd8c9a1c9318.
float4 WRNative209(WR_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[0u];
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f;
    // 1: add r0.x, -v2.x, v3.w
    r0.x = ((-(v2.xxxx))+(v3.wwww)).x;
    // 2: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 3: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 4: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 5: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 6: mul o0.xyz, r0.xyzx, cb0[0].xxxx
    output.xyz = ((r0.xyzx)*(source[0].xxxx)).xyz;
    // 7: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_03.fx_mi.fx_d_pa_ring_07_26_ad; source PS b35a09a584c8f141b96670a072707144.
float4 WRNative210(WR_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[4u];
    source[2].x = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[2].z = (g_VSourceMaterialTime.xxxx).x;
    source[2].w = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[3].x = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[3].y = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[3].z = (g_VSourceMaterialParameters[1u].wwww).x;
    source[3].w = (g_VSourceMaterialParameters[0u].wwww).x;
    source[4].x = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[4].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[4].w = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[5].x = (g_VSourceMaterialParameters[2u].wwww).x;
    source[5].y = (g_VSourceMaterialParameters[3u].xxxx).x;
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
    // 22: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 24: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 25: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 26: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 27: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 28: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 29: mad r0.y, r0.y, l(0.159155), l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 30: mul r0.y, r0.y, cb0[2].w
    r0.y = ((r0.yyyy)*(source[2].wwww)).y;
    // 31: mad r1.x, cb0[2].z, cb0[2].y, r0.y
    r1.x = ((source[2].zzzz)*(source[2].yyyy)+(r0.yyyy)).x;
    // 32: mul r0.y, v2.y, cb0[4].x
    r0.y = ((v2.yyyy)*(source[4].xxxx)).y;
    // 33: mad r2.y, cb0[2].z, cb0[4].y, r0.y
    r2.y = ((source[2].zzzz)*(source[4].yyyy)+(r0.yyyy)).y;
    // 34: mul r0.yz, cb0[2].zzzz, cb0[3].yyzy
    r0.yz = ((source[2].zzzz)*(source[3].yyzy)).yz;
    // 35: mad r2.x, cb0[3].w, v2.x, r0.z
    r2.x = ((source[3].wwww)*(v2.xxxx)+(r0.zzzz)).x;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.xyxx, t0.yzxw, s1, l(0.000000)
    r0.z = (VNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 37: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 38: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 39: mul r0.w, r0.w, v4.z
    r0.w = ((r0.wwww)*(v4.zzzz)).w;
    // 40: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 41: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 42: mad r1.y, cb0[3].x, r0.x, r0.y
    r1.y = ((source[3].xxxx)*(r0.xxxx)+(r0.yyyy)).y;
    // 43: mul r0.x, v4.x, cb0[4].z
    r0.x = ((v4.xxxx)*(source[4].zzzz)).x;
    // 44: mad r0.yw, r0.zzzz, r0.xxxx, r1.xxxy
    r0.yw = ((r0.zzzz)*(r0.xxxx)+(r1.xxxy)).yw;
    // 45: mad r0.xz, r0.zzzz, r0.xxxx, v2.xxyx
    r0.xz = ((r0.zzzz)*(r0.xxxx)+(v2.xxyx)).xz;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t2.xyzw, s2, l(0.000000)
    r0.x = (VNativeSample2((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.ywyy, t1.wxyz, s0, cb0[2].x
    r0.yzw = (VNativeSample0((r0.ywyy).xy, (source[2].xxxx).x, true).wxyz).yzw;
    // 48: mul_sat r1.x, r0.y, cb0[5].x
    r1.x = (saturate((r0.yyyy)*(source[5].xxxx))).x;
    // 49: mov_sat r1.y, v4.y
    r1.y = (saturate(v4.yyyy)).y;
    // 50: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 51: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 52: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 53: mul_sat r0.x, r0.x, r1.x
    r0.x = (saturate((r0.xxxx)*(r1.xxxx))).x;
    // 54: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 55: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 56: mul r1.x, r1.x, cb0[5].y
    r1.x = ((r1.xxxx)*(source[5].yyyy)).x;
    // 57: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 58: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 59: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 60: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 61: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 62: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 63: mad r0.yzw, cb0[4].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[4].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 64: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 65: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 66: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 67: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_m_00.fx_mi.fx_m_pa_bloodcliff_glow_depthfade_01_02_tr; source PS f4c884edfebbfc468ebc706159d8d1c6.
float4 WRNative211(WR_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[2u];
    source[2].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[0u].xxxx)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[0u].xxxx))).x;
    source[2].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[3].x = ((g_VSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)-(g_VSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (max((float4(1.0, 0.0, 0.0, 0.0)-(g_VSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-(g_VSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[4].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[4].y = ((g_VSourceMaterialParameters[0u].zzzz*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_VSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[1u].xxxx)).x;
    source[5].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[1u].xxxx))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
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
    // 10: add r0.y, -cb0[5].y, l(1.000000)
    r0.y = ((-(source[5].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, l(100.000000)
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 12: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 13: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 14: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 15: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 16: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 17: mad r0.y, -r0.y, cb0[2].z, l(1.000000)
    r0.y = ((-(r0.yyyy))*(source[2].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 18: mul_sat r0.y, r0.y, cb0[3].w
    r0.y = (saturate((r0.yyyy)*(source[3].wwww))).y;
    // 19: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 20: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 21: mul r0.z, r0.z, cb0[4].y
    r0.z = ((r0.zzzz)*(source[4].yyyy)).z;
    // 22: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 23: mul_sat r0.z, r0.z, cb0[4].z
    r0.z = (saturate((r0.zzzz)*(source[4].zzzz))).z;
    // 24: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 25: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 26: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 27: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 28: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 29: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_mi_m_00.fx_mi.fx_m_pa_bloodcliff_glow_depthfade_01_02_ad; source PS f1ee928cd5135a43b5f7ecda979adc4e.
float4 WRNative212(WR_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[2u];
    source[2].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[0u].xxxx)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[0u].xxxx))).x;
    source[2].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[3].x = ((g_VSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)-(g_VSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (max((float4(1.0, 0.0, 0.0, 0.0)-(g_VSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-(g_VSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[4].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[4].y = ((g_VSourceMaterialParameters[0u].zzzz*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_VSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[1u].xxxx)).x;
    source[5].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[1u].xxxx))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f;
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
    // 10: add r0.y, -cb0[5].y, l(1.000000)
    r0.y = ((-(source[5].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, l(100.000000)
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 12: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 13: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 14: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 15: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 16: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 17: mad r0.y, -r0.y, cb0[2].z, l(1.000000)
    r0.y = ((-(r0.yyyy))*(source[2].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 18: mul_sat r0.y, r0.y, cb0[3].w
    r0.y = (saturate((r0.yyyy)*(source[3].wwww))).y;
    // 19: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 20: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 21: mul r0.z, r0.z, cb0[4].y
    r0.z = ((r0.zzzz)*(source[4].yyyy)).z;
    // 22: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 23: mul_sat r0.z, r0.z, cb0[4].z
    r0.z = (saturate((r0.zzzz)*(source[4].zzzz))).z;
    // 24: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 25: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 26: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 27: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 28: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 29: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 30: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 31: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_k_00.fx_mi.fx_k_pa_slice_01_02_tr; source PS 12959b8a47f91c4dab8a19b9de0871ed.
float4 WRNative213(WR_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[3u];
    source[2] = VNativeAppend(cos((g_VSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = VNativeAppend(sin((g_VSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_VSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = VNativeAppend(cos((g_VSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = VNativeAppend(sin((g_VSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_VSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[6].y = (cos((g_VSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_VSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[7].x = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[7].w = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[8].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_VSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 8: mad r0.yz, r0.yyzy, cb0[6].zzwz, cb0[7].xxyx
    r0.yz = ((r0.yyzy)*(source[6].zzwz)+(source[7].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (VNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mul r0.z, v4.z, cb0[7].z
    r0.z = ((v4.zzzz)*(source[7].zzzz)).z;
    // 11: mad r1.xy, r0.yyyy, r0.zzzz, v2.xyxx
    r1.xy = ((r0.yyyy)*(r0.zzzz)+(v2.xyxx)).xy;
    // 12: add r1.zw, -r1.xxxx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((-(r1.xxxx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 13: add r0.yz, r1.wwyw, r1.zzyz
    r0.yz = ((r1.wwyw)+(r1.zzyz)).yz;
    // 14: add r0.yz, r0.yyzy, l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 15: dp2 r0.w, cb0[4].xyxx, r0.yzyy
    r0.w = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).w;
    // 16: dp2 r0.y, cb0[5].xyxx, r0.yzyy
    r0.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 17: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 18: div r0.y, l(0.010000), r0.y
    r0.y = ((float4(0.010000,0.010000,0.010000,0.010000))/(r0.yyyy)).y;
    // 19: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 20: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 21: mad r0.w, cb0[8].x, l(10.000000), l(10.000000)
    r0.w = ((source[8].xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 22: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 23: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 24: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 25: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 26: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 27: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 28: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 29: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 30: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 31: source device depth mapped to centimetre view depth; reconstruction at 33.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 33-36: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 37: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 38: add r0.w, -cb0[7].w, l(1.000000)
    r0.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 40: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 41: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 42: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 43: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 44: mad r0.xz, v2.xxyx, l(-1.000000, 0.000000, 1.000000, 0.000000), l(1.000000, 0.000000, 0.000000, 0.000000)
    r0.xz = ((v2.xxyx)*(float4(-1.000000,0.000000,1.000000,0.000000))+(float4(1.000000,0.000000,0.000000,0.000000))).xz;
    // 45: mad r0.xz, r0.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r0.xz = ((r0.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 46: dp2 r0.x, r0.xzxx, r0.xzxx
    r0.x = (dot((r0.xzxx).xy,(r0.xzxx).xy).xxxx).x;
    // 47: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 48: add r0.x, r0.x, -v4.x
    r0.x = ((r0.xxxx)+(-(v4.xxxx))).x;
    // 49: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 50: div r0.x, l(0.100000), |r0.x|
    r0.x = ((float4(0.100000,0.100000,0.100000,0.100000))/(abs(r0.xxxx))).x;
    // 51: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 53: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_mi_j_00.fx_mi.fx_j_me_localcrack_01_04_tr; source PS 8b228c7b319b544781cca408746673a2.
float4 WRNative214(WR_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_VSourceMaterialParameters[7u];
    source[3] = VNativeAppend(VNativePeriodic(((g_VSourceMaterialParameters[1u].wwww*g_VSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),VNativePeriodic(((g_VSourceMaterialParameters[1u].wwww*g_VSourceMaterialTime.xxxx)*float4(0.300000012, 0.0, 0.0, 0.0))),1u);
    source[4] = g_VSourceMaterialParameters[6u];
    source[5] = input.dynamicParameter;
    source[6] = g_VSourceMaterialParameters[3u];
    source[7] = g_VSourceMaterialParameters[5u];
    source[8].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[8].y = (g_VSourceMaterialParameters[0u].wwww).x;
    source[8].z = (g_VSourceMaterialTime.xxxx).x;
    source[8].w = (g_VSourceMaterialParameters[1u].wwww).x;
    source[9].x = (VNativePeriodic(((g_VSourceMaterialParameters[1u].wwww*g_VSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_VSourceMaterialParameters[2u].wwww).x;
    source[9].z = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[9].w = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[10].x = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[10].y = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[10].w = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[11].x = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[11].y = (g_VSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: mul r0.zw, v4.xxxy, cb0[8].xxxy
    r0.zw = ((v4.xxxy)*(source[8].xxxy)).zw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s0, l(0.000000)
    r0.zw = (VNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    r2.xyw = (VNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
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
    r3.xyz = (VNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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

// fx_m_mi_k_00.fx_mi.fx_k_pa_crackholev2_01_01; source PS f007847cf5f8ad44942c1b631669c131.
float4 WRNative215(WR_NATIVE_INPUT input)
{
    float4 source[19]; [unroll] for (uint i=0u; i<19u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[12u];
    source[2] = g_VSourceMaterialParameters[9u];
    source[3] = VNativeAppend(g_VSourceMaterialParameters[7u].yyyy,g_VSourceMaterialParameters[7u].zzzz,1u);
    source[4] = VNativeAppend(VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[8u].yyyy)*float4(-0.5, 0.0, 0.0, 0.0))),VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[8u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5] = VNativeAppend(VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[8u].yyyy)*float4(0.5, 0.0, 0.0, 0.0))),VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[8u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[6] = VNativeAppend(g_VSourceMaterialParameters[6u].zzzz,g_VSourceMaterialParameters[6u].wwww,1u);
    source[7] = g_VSourceMaterialParameters[10u];
    source[8] = g_VSourceMaterialParameters[11u];
    source[9].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[9].y = (g_VSourceMaterialParameters[8u].yyyy).x;
    source[9].z = (g_VSourceMaterialTime.xxxx).x;
    source[9].w = ((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[8u].yyyy)).x;
    source[10].x = (VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[8u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[10].y = (VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[8u].yyyy)*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[10].z = (g_VSourceMaterialParameters[7u].xxxx).x;
    source[10].w = (((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[8u].yyyy)*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[11].x = (VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[8u].yyyy)*float4(0.5, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_VSourceMaterialParameters[8u].xxxx).x;
    source[11].z = (g_VSourceMaterialParameters[7u].wwww).x;
    source[11].w = ((g_VSourceMaterialParameters[7u].wwww+g_VSourceMaterialParameters[8u].xxxx)).x;
    source[12].x = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[12].y = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[12].z = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[12].w = (g_VSourceMaterialParameters[4u].wwww).x;
    source[13].x = (g_VSourceMaterialParameters[3u].wwww).x;
    source[13].y = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[13].z = (g_VSourceMaterialParameters[6u].wwww).x;
    source[13].w = (g_VSourceMaterialParameters[6u].zzzz).x;
    source[14].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[14].y = (g_VSourceMaterialParameters[1u].wwww).x;
    source[14].z = (g_VSourceMaterialParameters[0u].wwww).x;
    source[14].w = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[15].x = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[15].y = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[15].z = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[15].w = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[16].x = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[16].y = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[16].z = (g_VSourceMaterialParameters[5u].wwww).x;
    source[16].w = (g_VSourceMaterialParameters[2u].wwww).x;
    source[17].x = (g_VSourceMaterialParameters[6u].xxxx).x;
    source[17].y = (g_VSourceMaterialParameters[6u].yyyy).x;
    source[17].z = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[17].w = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[18].x = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[18].y = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[18].z = (g_VSourceMaterialParameters[3u].zzzz).x;
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
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xy, r0.xxxx, v6.xyxx
    r0.xy = ((r0.xxxx)*(v6.xyxx)).xy;
    // 4: add r0.zw, v2.xxxy, cb0[6].xxxy
    r0.zw = ((v2.xxxy)+(source[6].xxxy)).zw;
    // 5: mul r1.x, v4.x, cb0[14].x
    r1.x = ((v4.xxxx)*(source[14].xxxx)).x;
    // 6: mad r1.y, r1.x, l(0.050000), l(-0.025000)
    r1.y = ((r1.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).y;
    // 7: mul r1.x, r1.x, l(0.040000)
    r1.x = ((r1.xxxx)*(float4(0.040000,0.040000,0.040000,0.040000))).x;
    // 8: add r1.x, |r1.x|, cb0[14].y
    r1.x = ((abs(r1.xxxx))+(source[14].yyyy)).x;
    // 9: add r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 10: max r1.x, r1.x, l(0.000010)
    r1.x = (max(r1.xxxx,float4(0.000010,0.000010,0.000010,0.000010))).x;
    // 11: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 12: mad r0.zw, r1.yyyy, r0.xxxy, r0.zzzw
    r0.zw = ((r1.yyyy)*(r0.xxxy)+(r0.zzzw)).zw;
    // 13: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 14: max r1.y, |r0.w|, |r0.z|
    r1.y = (max(abs(r0.wwww),abs(r0.zzzz))).y;
    // 15: div r1.y, l(1.000000, 1.000000, 1.000000, 1.000000), r1.y
    r1.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.yyyy)).y;
    // 16: min r1.z, |r0.w|, |r0.z|
    r1.z = (min(abs(r0.wwww),abs(r0.zzzz))).z;
    // 17: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 18: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 19: mad r1.w, r1.z, l(0.020835), l(-0.085133)
    r1.w = ((r1.zzzz)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).w;
    // 20: mad r1.w, r1.z, r1.w, l(0.180141)
    r1.w = ((r1.zzzz)*(r1.wwww)+(float4(0.180141,0.180141,0.180141,0.180141))).w;
    // 21: mad r1.w, r1.z, r1.w, l(-0.330299)
    r1.w = ((r1.zzzz)*(r1.wwww)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).w;
    // 22: mad r1.z, r1.z, r1.w, l(0.999866)
    r1.z = ((r1.zzzz)*(r1.wwww)+(float4(0.999866,0.999866,0.999866,0.999866))).z;
    // 23: mul r1.w, r1.z, r1.y
    r1.w = ((r1.zzzz)*(r1.yyyy)).w;
    // 24: mad r1.w, r1.w, l(-2.000000), l(1.570796)
    r1.w = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).w;
    // 25: lt r2.x, |r0.w|, |r0.z|
    r2.x = (asfloat((uint4)((abs(r0.wwww))<(abs(r0.zzzz))) * 0xffffffffu)).x;
    // 26: and r1.w, r1.w, r2.x
    r1.w = (asfloat(asuint(r1.wwww) & asuint(r2.xxxx))).w;
    // 27: mad r1.y, r1.y, r1.z, r1.w
    r1.y = ((r1.yyyy)*(r1.zzzz)+(r1.wwww)).y;
    // 28: lt r1.z, r0.w, -r0.w
    r1.z = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).z;
    // 29: and r1.z, r1.z, l(0xc0490fdb)
    r1.z = (asfloat(asuint(r1.zzzz) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).z;
    // 30: add r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)+(r1.yyyy)).y;
    // 31: min r1.z, r0.w, r0.z
    r1.z = (min(r0.wwww,r0.zzzz)).z;
    // 32: lt r1.z, r1.z, -r1.z
    r1.z = (asfloat((uint4)((r1.zzzz)<(-(r1.zzzz))) * 0xffffffffu)).z;
    // 33: max r1.w, r0.w, r0.z
    r1.w = (max(r0.wwww,r0.zzzz)).w;
    // 34: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 35: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 36: mad r0.z, -r0.z, r1.x, l(1.000000)
    r0.z = ((-(r0.zzzz))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 37: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 38: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 39: ge r0.w, r1.w, -r1.w
    r0.w = (asfloat((uint4)((r1.wwww)>=(-(r1.wwww))) * 0xffffffffu)).w;
    // 40: and r0.w, r0.w, r1.z
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.zzzz))).w;
    // 41: movc r0.w, r0.w, -r1.y, r1.y
    r0.w = ((asuint(r0.wwww) != 0u) ? (-(r1.yyyy)) : (r1.yyyy)).w;
    // 42: mad r0.w, r0.w, l(0.318310), l(1.000000)
    r0.w = ((r0.wwww)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul r1.x, r0.w, l(0.500000)
    r1.x = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 44: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 45: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 46: mul r1.z, r0.w, cb0[14].z
    r1.z = ((r0.wwww)*(source[14].zzzz)).z;
    // 47: mul r0.w, r0.w, cb0[17].z
    r0.w = ((r0.wwww)*(source[17].zzzz)).w;
    // 48: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 49: mul_sat r0.w, r0.w, cb0[17].w
    r0.w = (saturate((r0.wwww)*(source[17].wwww))).w;
    // 50: movc r0.w, r0.z, l(0), r0.w
    r0.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 51: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 52: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 53: movc r1.y, r0.z, l(0), r1.z
    r1.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 54: mul r1.zw, r1.xxxy, cb0[13].xxxy
    r1.zw = ((r1.xxxy)*(source[13].xxxy)).zw;
    // 55: mad r2.x, cb0[9].z, cb0[12].w, r1.z
    r2.x = ((source[9].zzzz)*(source[12].wwww)+(r1.zzzz)).x;
    // 56: mad r2.y, cb0[9].z, cb0[14].w, r1.w
    r2.y = ((source[9].zzzz)*(source[14].wwww)+(r1.wwww)).y;
    // 57: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r2.xyxx, t3.yzxw, s4, l(0.000000)
    r0.z = (VNativeSample4((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yzxw).z;
    // 58: mul r1.z, v4.z, cb0[15].x
    r1.z = ((v4.zzzz)*(source[15].xxxx)).z;
    // 59: mad r1.xy, r0.zzzz, r1.zzzz, r1.xyxx
    r1.xy = ((r0.zzzz)*(r1.zzzz)+(r1.xyxx)).xy;
    // 60: mul r0.z, cb0[9].z, cb0[15].z
    r0.z = ((source[9].zzzz)*(source[15].zzzz)).z;
    // 61: mad r2.x, cb0[15].w, r1.x, r0.z
    r2.x = ((source[15].wwww)*(r1.xxxx)+(r0.zzzz)).x;
    // 62: mul r0.z, r1.y, cb0[16].x
    r0.z = ((r1.yyyy)*(source[16].xxxx)).z;
    // 63: mul r1.xy, r1.xyxx, cb0[12].yzyy
    r1.xy = ((r1.xyxx)*(source[12].yzyy)).xy;
    // 64: mad r2.y, cb0[9].z, cb0[16].y, r0.z
    r2.y = ((source[9].zzzz)*(source[16].yyyy)+(r0.zzzz)).y;
    // 65: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r2.xyz = (VNativeSample5((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).xyz;
    // 66: mad r3.x, cb0[9].z, cb0[12].x, r1.x
    r3.x = ((source[9].zzzz)*(source[12].xxxx)+(r1.xxxx)).x;
    // 67: mad r3.y, cb0[9].z, cb0[15].y, r1.y
    r3.y = ((source[9].zzzz)*(source[15].yyyy)+(r1.yyyy)).y;
    // 68: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r3.xyxx, t4.xyzw, s3, l(0.000000)
    r1.xyz = (VNativeSample3((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).xyz;
    // 69: mul r3.xyz, r2.xyzx, r1.xyzx
    r3.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 70: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 71: mad r1.xyz, -r1.xyzx, r2.xyzx, r0.zzzz
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r0.zzzz)).xyz;
    // 72: mad r1.xyz, cb0[16].zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((source[16].zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 73: max r2.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 74: mul_sat r1.xyz, r1.xyzx, cb0[16].wwww
    r1.xyz = (saturate((r1.xyzx)*(source[16].wwww))).xyz;
    // 75: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 76: mul r2.xyz, r2.xyzx, cb0[17].xxxx
    r2.xyz = ((r2.xyzx)*(source[17].xxxx)).xyz;
    // 77: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 78: mul r2.xyz, r2.xyzx, cb0[17].yyyy
    r2.xyz = ((r2.xyzx)*(source[17].yyyy)).xyz;
    // 79: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 80: mul r3.xyz, cb0[7].xyzx, cb0[7].wwww
    r3.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 81: mul r4.xyz, cb0[8].xyzx, cb0[8].wwww
    r4.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 82: mad r2.xyz, r2.xyzx, r3.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 83: mul r2.xyz, r2.xyzx, v3.xyzx
    r2.xyz = ((r2.xyzx)*(v3.xyzx)).xyz;
    // 84: mad r0.z, cb0[18].y, l(0.050000), l(-0.050000)
    r0.z = ((source[18].yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.050000,-0.050000,-0.050000,-0.050000))).z;
    // 85: mad r0.xy, r0.zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 86: mad r0.zw, v2.xxxy, cb0[3].xxxy, cb0[5].xxxy
    r0.zw = ((v2.xxxy)*(source[3].xxxy)+(source[5].xxxy)).zw;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (VNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 88: mul r0.z, r0.z, cb0[10].z
    r0.z = ((r0.zzzz)*(source[10].zzzz)).z;
    // 89: mad r3.xy, v2.xyxx, cb0[3].xyxx, cb0[4].xyxx
    r3.xy = ((v2.xyxx)*(source[3].xyxx)+(source[4].xyxx)).xy;
    // 90: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r3.xyxx, t0.yzwx, s0, l(0.000000)
    r0.w = (VNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 91: mad r0.z, r0.w, cb0[10].z, r0.z
    r0.z = ((r0.wwww)*(source[10].zzzz)+(r0.zzzz)).z;
    // 92: mad r0.zw, r0.zzzz, l(0.000000, 0.000000, 0.100000, -0.100000), cb0[11].wwww
    r0.zw = ((r0.zzzz)*(float4(0.000000,0.000000,0.100000,-0.100000))+(source[11].wwww)).zw;
    // 93: add r0.xy, r0.zwzz, r0.xyxx
    r0.xy = ((r0.zwzz)+(r0.xyxx)).xy;
    // 94: add r0.zw, r0.zzzw, v2.xxxy
    r0.zw = ((r0.zzzw)+(v2.xxxy)).zw;
    // 95: add r1.w, v4.y, cb0[18].x
    r1.w = ((v4.yyyy)+(source[18].xxxx)).w;
    // 96: mul r3.xy, r1.wwww, l(0.200000, -0.200000, 0.000000, 0.000000)
    r3.xy = ((r1.wwww)*(float4(0.200000,-0.200000,0.000000,0.000000))).xy;
    // 97: mov r3.z, l(0)
    r3.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 98: add r3.xyzw, r0.xyxy, r3.xzyz
    r3.xyzw = ((r0.xyxy)+(r3.xzyz)).xyzw;
    // 99: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (VNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r3.zwzz, t2.yxzw, s2, l(0.000000)
    r0.y = (VNativeSample2((r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 101: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 102: mad r3.xz, v4.yyyy, l(0.200000, 0.000000, -0.200000, 0.000000), l(-0.080000, 0.000000, 0.080000, 0.000000)
    r3.xz = ((v4.yyyy)*(float4(0.200000,0.000000,-0.200000,0.000000))+(float4(-0.080000,0.000000,0.080000,0.000000))).xz;
    // 103: mov r3.yw, l(0,0,0,0)
    r3.yw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).yw;
    // 104: add r3.xyzw, r0.zwzw, r3.xyzw
    r3.xyzw = ((r0.zwzw)+(r3.xyzw)).xyzw;
    // 105: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r3.xyxx, t1.yxzw, s1, l(0.000000)
    r0.y = (VNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 106: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.zwzz, t2.yzwx, s2, l(0.000000)
    r1.w = (VNativeSample2((r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 107: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 108: mul_sat r1.w, r0.y, v3.w
    r1.w = (saturate((r0.yyyy)*(v3.wwww))).w;
    // 109: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 110: mul o0.w, r1.w, cb0[0].x
    output.w = ((r1.wwww)*(source[0].xxxx)).w;
    // 111: mul r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 112: add r0.x, v4.y, -cb0[9].x
    r0.x = ((v4.yyyy)+(-(source[9].xxxx))).x;
    // 113: add r0.x, r0.x, l(-0.400000)
    r0.x = ((r0.xxxx)+(float4(-0.400000,-0.400000,-0.400000,-0.400000))).x;
    // 114: mul r3.xy, r0.xxxx, l(0.200000, -0.200000, 0.000000, 0.000000)
    r3.xy = ((r0.xxxx)*(float4(0.200000,-0.200000,0.000000,0.000000))).xy;
    // 115: mov r3.z, l(0)
    r3.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 116: add r3.xyzw, r0.zwzw, r3.xzyz
    r3.xyzw = ((r0.zwzw)+(r3.xzyz)).xyzw;
    // 117: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (VNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 118: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r3.zwzz, t2.yzxw, s2, l(0.000000)
    r0.z = (VNativeSample2((r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 119: mad r0.x, -r0.x, r0.z, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 120: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 121: mul r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)*(r0.xxxx)).xyz;
    // 122: mad r0.xyz, cb0[2].xyzx, r0.xyzx, r2.xyzx
    r0.xyz = ((source[2].xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 123: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 124: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_mi_k_00.fx_mi.fx_k_pa_glasshole_02_01_tr; source PS 377108e10f08cc488de94c405e789871.
float4 WRNative216(WR_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[10u];
    source[2] = VNativeAppend((g_VSourceMaterialParameters[4u].xxxx*g_VSourceMaterialTime.xxxx),(g_VSourceMaterialParameters[4u].yyyy*g_VSourceMaterialTime.xxxx),1u);
    source[3] = VNativeAppend(VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.00999999978, 0.0, 0.0, 0.0))),VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),1u);
    source[4] = VNativeAppend(g_VSourceMaterialParameters[0u].zzzz,g_VSourceMaterialParameters[0u].wwww,1u);
    source[5] = VNativeAppend(g_VSourceMaterialParameters[0u].xxxx,g_VSourceMaterialParameters[0u].yyyy,1u);
    source[6] = VNativeAppend(g_VSourceMaterialParameters[1u].wwww,g_VSourceMaterialParameters[2u].xxxx,1u);
    source[7] = VNativeAppend(VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[6u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[6u].xxxx)*float4(-0.230000004, 0.0, 0.0, 0.0))),1u);
    source[8] = VNativeAppend(VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[6u].xxxx)*float4(-0.5, 0.0, 0.0, 0.0))),VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[6u].xxxx)*float4(0.370000005, 0.0, 0.0, 0.0))),1u);
    source[9] = g_VSourceMaterialParameters[9u];
    source[10] = VNativeAppend(VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))),1u);
    source[11] = VNativeAppend(VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0))),1u);
    source[12] = g_VSourceMaterialParameters[8u];
    source[13] = VNativeAppend(VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[14].x = ((g_VSourceMaterialParameters[2u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[14].y = (g_VSourceMaterialParameters[2u].wwww).x;
    source[14].z = ((g_VSourceMaterialParameters[2u].wwww*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[14].w = (g_VSourceMaterialParameters[5u].wwww).x;
    source[15].x = (VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[6u].xxxx)*float4(0.370000005, 0.0, 0.0, 0.0)))).x;
    source[15].y = (VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[6u].xxxx)*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[15].z = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[15].w = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[16].x = (g_VSourceMaterialParameters[3u].wwww).x;
    source[16].y = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[16].z = (g_VSourceMaterialParameters[4u].wwww).x;
    source[16].w = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[17].x = ((g_VSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[17].y = ((g_VSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))).x;
    source[17].z = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[17].w = (g_VSourceMaterialParameters[6u].yyyy).x;
    source[18].x = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[18].y = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[18].z = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[18].w = (g_VSourceMaterialParameters[6u].wwww).x;
    source[19].x = (g_VSourceMaterialParameters[7u].xxxx).x;
    source[19].y = (g_VSourceMaterialParameters[6u].zzzz).x;
    source[19].z = ((g_VSourceMaterialParameters[6u].zzzz*g_VSourceMaterialTime.xxxx)).x;
    source[19].w = (VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0)))).x;
    source[20].x = (VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[20].y = ((g_VSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0))).x;
    source[20].z = (VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0)))).x;
    source[20].w = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[21].x = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[21].y = ((g_VSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[21].z = (VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[21].w = (g_VSourceMaterialParameters[3u].xxxx).x;
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
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).y;
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
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
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
    r1.xyz = (VNativeSample4((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 49: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t4.wxyz, s4, l(-1.000000)
    r0.yzw = (VNativeSample3((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
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
    r2.w = ((r2.zzzz)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).w;
    // 63: mad r2.w, r2.z, r2.w, l(0.180141)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(0.180141,0.180141,0.180141,0.180141))).w;
    // 64: mad r2.w, r2.z, r2.w, l(-0.330299)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).w;
    // 65: mad r2.z, r2.z, r2.w, l(0.999866)
    r2.z = ((r2.zzzz)*(r2.wwww)+(float4(0.999866,0.999866,0.999866,0.999866))).z;
    // 66: mul r2.w, r1.w, r2.z
    r2.w = ((r1.wwww)*(r2.zzzz)).w;
    // 67: mad r2.w, r2.w, l(-2.000000), l(1.570796)
    r2.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).w;
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
    r2.x = ((r1.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 81: mad r3.xy, v2.xyxx, cb0[6].xyxx, cb0[7].xyxx
    r3.xy = ((v2.xyxx)*(source[6].xyxx)+(source[7].xyxx)).xy;
    // 82: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r1.w = (VNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 83: mad r1.w, r1.w, l(2.000000), l(-1.000000)
    r1.w = ((r1.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 84: mad r3.xy, v2.xyxx, cb0[6].xyxx, cb0[8].xyxx
    r3.xy = ((v2.xyxx)*(source[6].xyxx)+(source[8].xyxx)).xy;
    // 85: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r2.w = (VNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r2.yz = (VNativeSample1((r3.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).yz;
    // 100: mul r3.xyz, r0.yzwy, r2.zzzz
    r3.xyz = ((r0.yzwy)*(r2.zzzz)).xyz;
    // 101: mul r3.xyz, r3.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 102: mad r1.xyz, cb0[21].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[21].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 103: mad r2.xw, r2.xxxz, l(7.000000, 0.000000, 0.000000, 1.000000), cb0[13].xxxy
    r2.xw = ((r2.xxxz)*(float4(7.000000,0.000000,0.000000,1.000000))+(source[13].xxxy)).xw;
    // 104: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xwxx, t6.xyzw, s6, l(0.000000)
    r3.xyz = (VNativeSample5((r2.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.yzw = (VNativeSample6((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
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
    // Native 120: source device depth mapped to centimetre view depth; reconstruction at 122.
    r1.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 122-125: reconstructed view depth is supplied by the runtime adapter.
    r1.x = r1.x;
    // 126: add r1.x, r1.x, -v7.w
    r1.x = ((r1.xxxx)+(-(v7.wwww))).x;
    // 127: mul_sat r1.xy, r1.xxxx, l(0.034483, 0.066667, 0.000000, 0.000000)
    r1.xy = (saturate((r1.xxxx)*(float4(0.034483,0.066667,0.000000,0.000000)))).xy;
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
    r2.yz = (VNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
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
    r1.xzw = (VNativeSample2((r1.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
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

// fx_m_mi_k_00.fx_mi.fx_k_pa_fluidninja_01_07_tr; source PS 692bbf41f335834f90217d007e7fe5f9.
float4 WRNative217(WR_NATIVE_INPUT input)
{
    float4 source[19]; [unroll] for (uint i=0u; i<19u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[10u];
    source[2] = g_VSourceMaterialParameters[8u];
    source[3] = g_VSourceMaterialParameters[9u];
    source[4] = VNativeAppend(g_VSourceMaterialParameters[1u].xxxx,g_VSourceMaterialParameters[1u].yyyy,1u);
    source[5] = VNativeAppend(g_VSourceMaterialParameters[3u].xxxx,g_VSourceMaterialParameters[3u].yyyy,1u);
    source[6] = VNativeAppend(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[2u].xxxx)+g_VSourceMaterialParameters[1u].zzzz),((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[2u].yyyy)+g_VSourceMaterialParameters[1u].wwww),1u);
    source[7] = VNativeAppend(g_VSourceMaterialParameters[5u].xxxx,g_VSourceMaterialParameters[5u].yyyy,1u);
    source[8] = VNativeAppend(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[4u].xxxx)+g_VSourceMaterialParameters[3u].zzzz),((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[4u].yyyy)+g_VSourceMaterialParameters[3u].wwww),1u);
    source[9] = VNativeAppend(g_VSourceMaterialParameters[7u].zzzz,g_VSourceMaterialParameters[7u].wwww,1u);
    source[10] = VNativeAppend(g_VSourceMaterialParameters[6u].zzzz,g_VSourceMaterialParameters[6u].wwww,1u);
    source[11].x = ((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[2u].yyyy)).x;
    source[11].y = (((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[2u].yyyy)+g_VSourceMaterialParameters[1u].wwww)).x;
    source[11].z = (((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[2u].xxxx)+g_VSourceMaterialParameters[1u].zzzz)).x;
    source[11].w = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[12].x = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[12].y = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[12].z = (g_VSourceMaterialParameters[2u].wwww).x;
    source[12].w = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[13].x = ((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[4u].yyyy)).x;
    source[13].y = (((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[4u].yyyy)+g_VSourceMaterialParameters[3u].wwww)).x;
    source[13].z = (((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[4u].xxxx)+g_VSourceMaterialParameters[3u].zzzz)).x;
    source[13].w = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[14].x = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[14].y = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[14].z = (g_VSourceMaterialParameters[4u].wwww).x;
    source[14].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[15].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[15].y = (g_VSourceMaterialParameters[0u].wwww).x;
    source[15].z = (g_VSourceMaterialParameters[7u].wwww).x;
    source[15].w = (g_VSourceMaterialParameters[7u].zzzz).x;
    source[16].x = (g_VSourceMaterialParameters[7u].xxxx).x;
    source[16].y = (g_VSourceMaterialParameters[7u].yyyy).x;
    source[16].z = (g_VSourceMaterialParameters[6u].wwww).x;
    source[16].w = (g_VSourceMaterialParameters[6u].zzzz).x;
    source[17].x = (g_VSourceMaterialParameters[5u].wwww).x;
    source[17].y = (g_VSourceMaterialParameters[6u].xxxx).x;
    source[17].z = (g_VSourceMaterialParameters[6u].yyyy).x;
    source[17].w = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[18].x = (g_VSourceMaterialParameters[0u].xxxx).x;
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
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: mad r0.zw, cb0[13].wwww, r0.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((source[13].wwww)*(r0.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 3: mad r0.xy, cb0[11].wwww, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((source[11].wwww)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 4: mad r0.xy, r0.xyxx, cb0[5].xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)*(source[5].xyxx)+(source[6].xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (VNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 7: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 8: mul r0.xy, r0.xyxx, cb0[12].zzzz
    r0.xy = ((r0.xyxx)*(source[12].zzzz)).xy;
    // 9: mad r0.zw, r0.zzzw, cb0[7].xxxy, cb0[8].xxxy
    r0.zw = ((r0.zzzw)*(source[7].xxxy)+(source[8].xxxy)).zw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s2, l(0.000000)
    r0.zw = (VNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 11: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 12: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 13: mad r0.zw, -r0.zzzw, cb0[14].zzzz, r0.xxxy
    r0.zw = ((-(r0.zzzw))*(source[14].zzzz)+(r0.xxxy)).zw;
    // 14: mad r0.xy, r0.zwzz, l(0.500000, 0.500000, 0.000000, 0.000000), -r0.xyxx
    r0.xy = ((r0.zwzz)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(r0.xyxx))).xy;
    // 15: mad r0.zw, -v2.xxxy, cb0[9].xxxy, r0.xxxy
    r0.zw = ((-(v2.xxxy))*(source[9].xxxy)+(r0.xxxy)).zw;
    // 16: mad r0.xy, -v2.xyxx, cb0[4].xyxx, r0.xyxx
    r0.xy = ((-(v2.xyxx))*(source[4].xyxx)+(r0.xyxx)).xy;
    // 17: mul r1.xy, v2.xyxx, cb0[9].xyxx
    r1.xy = ((v2.xyxx)*(source[9].xyxx)).xy;
    // 18: mad r0.zw, v4.xxxx, r0.zzzw, r1.xxxy
    r0.zw = ((v4.xxxx)*(r0.zzzw)+(r1.xxxy)).zw;
    // 19: add r0.zw, r0.zzzw, v4.yyyy
    r0.zw = ((r0.zzzw)+(v4.yyyy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s4, l(0.000000)
    r1.xyz = (VNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: add r0.z, r1.y, r1.x
    r0.z = ((r1.yyyy)+(r1.xxxx)).z;
    // 22: add r0.z, r1.z, r0.z
    r0.z = ((r1.zzzz)+(r0.zzzz)).z;
    // 23: mul r0.z, r0.z, l(0.333330)
    r0.z = ((r0.zzzz)*(float4(0.333330,0.333330,0.333330,0.333330))).z;
    // 24: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 25: mul r0.w, r0.w, cb0[16].x
    r0.w = ((r0.wwww)*(source[16].xxxx)).w;
    // 26: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 27: mul r0.w, r0.w, cb0[16].y
    r0.w = ((r0.wwww)*(source[16].yyyy)).w;
    // 28: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: mul r0.z, r0.z, cb0[17].x
    r0.z = ((r0.zzzz)*(source[17].xxxx)).z;
    // 30: mad r1.yz, v2.xxyx, cb0[10].xxyx, r0.zzzz
    r1.yz = ((v2.xxyx)*(source[10].xxyx)+(r0.zzzz)).yz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.yzyy, t3.yzxw, s5, l(0.000000)
    r0.z = (VNativeSample4((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 32: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 33: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 34: mul r0.z, r0.z, cb0[17].y
    r0.z = ((r0.zzzz)*(source[17].yyyy)).z;
    // 35: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 36: mul r0.z, r0.z, cb0[17].z
    r0.z = ((r0.zzzz)*(source[17].zzzz)).z;
    // 37: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 38: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 39: mad r1.xy, v2.yxyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.yxyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 40: add r1.xy, -|r1.xyxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(abs(r1.xyxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 41: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 42: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 43: mul r1.zw, r1.zzzw, cb0[17].wwww
    r1.zw = ((r1.zzzw)*(source[17].wwww)).zw;
    // 44: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 45: movc r1.xy, r1.xyxx, l(0,0,0,0), r1.zwzz
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zwzz)).xy;
    // 46: mul r0.w, r1.y, r1.x
    r0.w = ((r1.yyyy)*(r1.xxxx)).w;
    // 47: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 48: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 50: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 51: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 52: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 53: source device depth mapped to centimetre view depth; reconstruction at 55.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 55-58: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 59: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 60: add r1.x, -cb0[18].x, l(1.000000)
    r1.x = ((-(source[18].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 61: mul r1.x, r1.x, l(100.000000)
    r1.x = ((r1.xxxx)*(float4(100.000000,100.000000,100.000000,100.000000))).x;
    // 62: max r1.x, r1.x, l(0.001000)
    r1.x = (max(r1.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 63: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 64: mul_sat r0.z, r0.w, r0.z
    r0.z = (saturate((r0.wwww)*(r0.zzzz))).z;
    // 65: mul o0.w, r0.z, cb0[0].x
    output.w = ((r0.zzzz)*(source[0].xxxx)).w;
    // 66: mul r0.zw, v2.xxxy, cb0[4].xxxy
    r0.zw = ((v2.xxxy)*(source[4].xxxy)).zw;
    // 67: add r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)+(r0.zzzw)).zw;
    // 68: mad r0.xy, v4.xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((v4.xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s3, l(0.000000)
    r0.xyz = (VNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 70: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 71: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 72: mad r0.xyz, cb0[14].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[14].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 73: add r1.xyz, -cb0[2].xyzx, cb0[3].xyzx
    r1.xyz = ((-(source[2].xyzx))+(source[3].xyzx)).xyz;
    // 74: mad r0.xyz, r0.xyzx, r1.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[2].xyzx)).xyz;
    // 75: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 76: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 77: mul r0.xyz, r0.xyzx, cb0[15].xxxx
    r0.xyz = ((r0.xyzx)*(source[15].xxxx)).xyz;
    // 78: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 79: mul r0.xyz, r0.xyzx, cb0[15].yyyy
    r0.xyz = ((r0.xyzx)*(source[15].yyyy)).xyz;
    // 80: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 81: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_mi_02.fx_mi.fx_m_pa_shine_02_3_ad; source PS f23a10673cbc1f42909194326959c3d8.
float4 WRNative218(WR_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[5u];
    source[2] = VNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = VNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[4].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[4].z = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[5].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_VSourceMaterialTime.xxxx).x;
    source[6].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_VSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[7].x = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[7].y = (g_VSourceMaterialParameters[1u].wwww).x;
    source[7].z = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[7].w = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[8].x = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[8].y = (g_VSourceMaterialParameters[3u].wwww).x;
    source[8].z = (g_VSourceMaterialParameters[2u].wwww).x;
    source[8].w = (g_VSourceMaterialParameters[3u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r1.x, cb0[2].xyxx, r0.xyxx
    r1.x = (dot((source[2].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[3].xyxx, r0.xyxx
    r1.y = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: add r0.zw, -r0.xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r0.xxxy))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 6: mul_sat r0.z, r0.x, r0.z
    r0.z = (saturate((r0.xxxx)*(r0.zzzz))).z;
    // 7: mul_sat r0.w, r0.w, cb0[8].z
    r0.w = (saturate((r0.wwww)*(source[8].zzzz))).w;
    // 8: mul r1.x, r0.z, l(4.000000)
    r1.x = ((r0.zzzz)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 9: lt r0.z, r0.z, l(0.000000)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 10: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 11: mul r1.x, r1.x, cb0[4].z
    r1.x = ((r1.xxxx)*(source[4].zzzz)).x;
    // 12: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 13: mul r1.x, r1.x, cb0[4].w
    r1.x = ((r1.xxxx)*(source[4].wwww)).x;
    // 14: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 15: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 16: mul r1.x, r1.x, cb0[5].x
    r1.x = ((r1.xxxx)*(source[5].xxxx)).x;
    // 17: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 18: mul r1.x, r1.x, cb0[5].y
    r1.x = ((r1.xxxx)*(source[5].yyyy)).x;
    // 19: lt r1.y, |r0.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 20: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 21: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 22: mul r1.xy, r0.xyxx, cb0[6].xyxx
    r1.xy = ((r0.xyxx)*(source[6].xyxx)).xy;
    // 23: mul r0.xy, r0.xyxx, cb0[7].xyxx
    r0.xy = ((r0.xyxx)*(source[7].xyxx)).xy;
    // 24: mad r2.x, cb0[5].w, cb0[5].z, r1.x
    r2.x = ((source[5].wwww)*(source[5].zzzz)+(r1.xxxx)).x;
    // 25: mad r2.y, cb0[5].w, cb0[6].z, r1.y
    r2.y = ((source[5].wwww)*(source[6].zzzz)+(r1.yyyy)).y;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.x = (VNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 27: mad r2.x, cb0[5].w, cb0[6].w, r0.x
    r2.x = ((source[5].wwww)*(source[6].wwww)+(r0.xxxx)).x;
    // 28: mad r2.y, cb0[5].w, cb0[7].z, r0.y
    r2.y = ((source[5].wwww)*(source[7].zzzz)+(r0.yyyy)).y;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (VNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 30: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 31: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 32: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 33: mul r0.x, r0.x, cb0[7].w
    r0.x = ((r0.xxxx)*(source[7].wwww)).x;
    // 34: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 35: mul r0.x, r0.x, cb0[8].x
    r0.x = ((r0.xxxx)*(source[8].xxxx)).x;
    // 36: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 37: mad_sat r0.x, r0.x, r0.z, r0.z
    r0.x = (saturate((r0.xxxx)*(r0.zzzz)+(r0.zzzz))).x;
    // 38: log r0.y, r0.w
    r0.y = (log2(r0.wwww)).y;
    // 39: lt r0.z, r0.w, l(0.000001)
    r0.z = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 40: mul r0.y, r0.y, cb0[8].w
    r0.y = ((r0.yyyy)*(source[8].wwww)).y;
    // 41: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 42: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 43: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 44: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 45: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 46: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 47: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 48: mul r0.w, r0.w, cb0[8].y
    r0.w = ((r0.wwww)*(source[8].yyyy)).w;
    // 49: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 50: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 51: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 52: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 53: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 54: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 55: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 56: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 57: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 58: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_02.fx_mi.fx_e_pa_ht_18_1_tr; source PS a14d57c96465f346be3437ad63c9ad10.
float4 WRNative220(WR_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[6u];
    source[2] = g_VSourceMaterialParameters[5u];
    source[3] = VNativeAppend(g_VSourceMaterialParameters[2u].wwww,g_VSourceMaterialParameters[3u].xxxx,1u);
    source[4] = VNativeAppend(g_VSourceMaterialParameters[0u].zzzz,g_VSourceMaterialParameters[1u].xxxx,1u);
    source[5] = VNativeAppend(VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[6] = VNativeAppend(VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[7] = VNativeAppend(g_VSourceMaterialParameters[0u].wwww,g_VSourceMaterialParameters[1u].yyyy,1u);
    source[8] = VNativeAppend(VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].wwww)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[9] = VNativeAppend(VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[2u].yyyy)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[10].x = (g_VSourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_VSourceMaterialTime.xxxx).x;
    source[10].w = ((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].zzzz)).x;
    source[11].x = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[11].y = (g_VSourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[11].w = (VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].x = (VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[12].w = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[13].y = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[13].z = ((float4(100.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[3u].zzzz)).x;
    source[13].w = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[3u].zzzz))).x;
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
    // 1: mad r0.xy, cb0[3].xyxx, v2.xyxx, v4.xyxx
    r0.xy = ((source[3].xyxx)*(v2.xyxx)+(v4.xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (VNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mad r0.xy, cb0[11].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[11].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 4: mad r1.x, cb0[4].x, r0.x, cb0[5].x
    r1.x = ((source[4].xxxx)*(r0.xxxx)+(source[5].xxxx)).x;
    // 5: mad r1.y, cb0[4].y, r0.y, cb0[6].y
    r1.y = ((source[4].yyyy)*(r0.yyyy)+(source[6].yyyy)).y;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (VNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 7: mad r1.x, cb0[7].x, r0.x, cb0[8].x
    r1.x = ((source[7].xxxx)*(r0.xxxx)+(source[8].xxxx)).x;
    // 8: mad r1.y, cb0[7].y, r0.y, cb0[9].y
    r1.y = ((source[7].yyyy)*(r0.yyyy)+(source[9].yyyy)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (VNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

// fx_m_mi_02.fx_mi.fx_j_rgbsplit_01_2_ad; source PS d880c361f16370468cfdac8d4688d393.
float4 WRNative221(WR_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[1u];
    source[2] = VNativeAppend(g_VSourceMaterialParameters[0u].zzzz,g_VSourceMaterialParameters[0u].wwww,1u);
    source[3].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[3].y = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[3].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[3].w = (g_VSourceMaterialParameters[0u].yyyy).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[2].xyxx
    r0.xy = ((v2.xyxx)*(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (VNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mul r0.xyz, r0.xxxx, l(0.100000, 0.500000, 0.100000, 0.000000)
    r0.xyz = ((r0.xxxx)*(float4(0.100000,0.500000,0.100000,0.000000))).xyz;
    // 4: mul r1.xz, v4.xxxx, l(0.100000, 0.000000, -0.100000, 0.000000)
    r1.xz = ((v4.xxxx)*(float4(0.100000,0.000000,-0.100000,0.000000))).xz;
    // 5: mov r1.yw, l(0,0,0,0)
    r1.yw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).yw;
    // 6: mad r1.xyzw, v2.xyxy, cb0[2].xyxy, r1.xyzw
    r1.xyzw = ((v2.xyxy)*(source[2].xyxy)+(r1.xyzw)).xyzw;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s0, l(0.000000)
    r0.w = (VNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.zwzz, t0.xyzw, s0, l(0.000000)
    r1.x = (VNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 9: mad r0.xyz, r0.wwww, l(0.500000, 0.100000, 0.100000, 0.000000), r0.xyzx
    r0.xyz = ((r0.wwww)*(float4(0.500000,0.100000,0.100000,0.000000))+(r0.xyzx)).xyz;
    // 10: mad r0.xyz, r1.xxxx, l(0.100000, 0.100000, 0.500000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xxxx)*(float4(0.100000,0.100000,0.500000,0.000000))+(r0.xyzx)).xyz;
    // 11: mul r1.xyz, r0.xyzx, cb0[3].zzzz
    r1.xyz = ((r0.xyzx)*(source[3].zzzz)).xyz;
    // 12: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 13: mad r0.xyz, -cb0[3].zzzz, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[3].zzzz))*(r0.xyzx)+(r0.wwww)).xyz;
    // 14: mad r0.xyz, cb0[3].wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((source[3].wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 15: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 16: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 17: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 18: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 19: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_n_00.fx_n_pa_db_01_04_ad; source PS 47c91b5fc620c54d82733fea081b4511.
float4 WRNative222(WR_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[2u];
    source[2].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_VSourceMaterialParameters[0u].wwww).x;
    source[2].z = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[2].w = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_VSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.x, v2.x, l(-0.500000)
    r0.x = ((v2.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 2: mad r0.x, r0.x, cb0[2].x, l(0.500000)
    r0.x = ((r0.xxxx)*(source[2].xxxx)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.z, v2.y, cb0[2].z
    r0.z = ((v2.yyyy)*(source[2].zzzz)).z;
    // 4: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 5: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 6: mul r1.xy, v4.yzyy, cb0[2].ywyy
    r1.xy = ((v4.yzyy)*(source[2].ywyy)).xy;
    // 7: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 8: add r0.w, v2.y, -v4.x
    r0.w = ((v2.yyyy)+(-(v4.xxxx))).w;
    // 9: add r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 10: mad r0.y, r0.w, r1.x, l(0.500000)
    r0.y = ((r0.wwww)*(r1.xxxx)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 11: mad r0.w, -cb0[2].z, v2.y, l(1.000000)
    r0.w = ((-(source[2].zzzz))*(v2.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: mad r0.xy, r0.wwww, r0.zzzz, r0.xyxx
    r0.xy = ((r0.wwww)*(r0.zzzz)+(r0.xyxx)).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (VNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 14: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 15: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 16: mad r0.xyz, cb0[3].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[3].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 17: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 18: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 19: mul r0.xyz, r0.xyzx, cb0[3].yyyy
    r0.xyz = ((r0.xyzx)*(source[3].yyyy)).xyz;
    // 20: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 21: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 22: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 23: add r0.w, -v2.y, l(1.000000)
    r0.w = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: mul r0.w, r0.w, v2.y
    r0.w = ((r0.wwww)*(v2.yyyy)).w;
    // 25: mul r1.x, |r0.w|, |r0.w|
    r1.x = ((abs(r0.wwww))*(abs(r0.wwww))).x;
    // 26: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 27: mul r1.x, r1.x, l(100.000000)
    r1.x = ((r1.xxxx)*(float4(100.000000,100.000000,100.000000,100.000000))).x;
    // 28: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 29: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 30: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 31: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 32: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 33: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_m_00.fx_mi.fx_m_bloodcliff_01_10_ad; source PS 7e6c26222a3d6242b24ea8ab803ecbb0.
float4 WRNative223(WR_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[2u];
    source[2] = VNativeAppend(cos(((g_VSourceMaterialParameters[1u].xxxx*float4(6.28299999, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_VSourceMaterialParameters[1u].xxxx*float4(6.28299999, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = VNativeAppend(sin(((g_VSourceMaterialParameters[1u].xxxx*float4(6.28299999, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_VSourceMaterialParameters[1u].xxxx*float4(6.28299999, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_VSourceMaterialParameters[1u].xxxx*float4(6.28299999, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[4].y = (cos(((g_VSourceMaterialParameters[1u].xxxx*float4(6.28299999, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[4].z = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[4].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_VSourceMaterialParameters[0u].wwww).x;
    source[5].z = (g_VSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[5].z, l(1.000000)
    r0.y = ((-(source[5].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 14: dp2 r1.x, cb0[2].xyxx, r0.yzyy
    r1.x = (dot((source[2].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 15: dp2 r1.y, cb0[3].xyxx, r0.yzyy
    r1.y = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 16: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 17: mad r0.w, cb0[4].z, v4.x, l(-1.000000)
    r0.w = ((source[4].zzzz)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 18: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 19: mul r1.x, v4.x, cb0[4].z
    r1.x = ((v4.xxxx)*(source[4].zzzz)).x;
    // 20: mad r0.yz, r1.xxxx, r0.yyzy, -r0.wwww
    r0.yz = ((r1.xxxx)*(r0.yyzy)+(-(r0.wwww))).yz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t0.xyzw, s1, l(0.000000)
    r1.xyzw = (VNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 22: log r0.y, |r1.w|
    r0.y = (log2(abs(r1.wwww))).y;
    // 23: mul r0.y, r0.y, cb0[5].y
    r0.y = ((r0.yyyy)*(source[5].yyyy)).y;
    // 24: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 25: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 26: lt r0.z, |r1.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 27: movc r0.y, r0.z, l(0), |r0.y|
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).y;
    // 28: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 29: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 30: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 31: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 32: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 33: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 34: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 35: mul r0.yzw, r1.xxyz, cb0[4].wwww
    r0.yzw = ((r1.xxyz)*(source[4].wwww)).yzw;
    // 36: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: mad r1.xyz, -cb0[4].wwww, r1.xyzx, r1.wwww
    r1.xyz = ((-(source[4].wwww))*(r1.xyzx)+(r1.wwww)).xyz;
    // 38: mad r0.yzw, cb0[5].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[5].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 39: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 40: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 41: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 42: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_01.fx_mi.fx_e_pa_ht_08_1_ad; source PS c058e92770ba0b488812c453984a6c6c.
float4 WRNative224(WR_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[3u];
    source[2] = g_VSourceMaterialParameters[2u];
    source[3] = VNativeAppend(g_VSourceMaterialParameters[0u].wwww,g_VSourceMaterialParameters[1u].xxxx,1u);
    source[4] = VNativeAppend(g_VSourceMaterialParameters[1u].zzzz,g_VSourceMaterialParameters[1u].wwww,1u);
    source[5].x = (g_VSourceMaterialTime.xxxx).x;
    source[5].y = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_VSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_VSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_VSourceMaterialParameters[0u].yyyy).x;
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
    // 1: mad r0.xy, cb0[5].xxxx, cb0[3].xyxx, v2.xyxx
    r0.xy = ((source[5].xxxx)*(source[3].xyxx)+(v2.xyxx)).xy;
    // 2: mul r0.xy, r0.xyxx, cb0[5].wwww
    r0.xy = ((r0.xyxx)*(source[5].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (VNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r0.zw, cb0[5].xxxx, cb0[4].xxxy, v2.xxxy
    r0.zw = ((source[5].xxxx)*(source[4].xxxy)+(v2.xxxy)).zw;
    // 5: mad r0.xy, cb0[6].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[6].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = (VNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

// fx_m_mi_00.fx_mi.fx_d_pa_dark_05_02_tr; source PS 28f1c571ae72fd4481756aab91e9b38f.
float4 WRNative225(WR_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[2u];
    source[2] = g_VSourceMaterialParameters[1u];
    source[3].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[3].z = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_VSourceMaterialParameters[0u].wwww).x;
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
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 4: mul r0.y, v4.x, cb0[3].x
    r0.y = ((v4.xxxx)*(source[3].xxxx)).y;
    // 5: max r0.y, r0.y, l(0.000010)
    r0.y = (max(r0.yyyy,float4(0.000010,0.000010,0.000010,0.000010))).y;
    // 6: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 7: mad r0.x, -r0.x, r0.y, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: mad r0.y, -cb0[3].y, v4.y, l(1.000000)
    r0.y = ((-(source[3].yyyy))*(v4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 9: max r0.y, r0.y, l(0.000010)
    r0.y = (max(r0.yyyy,float4(0.000010,0.000010,0.000010,0.000010))).y;
    // 10: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 11: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 12: mul_sat r0.x, r0.x, cb0[3].z
    r0.x = (saturate((r0.xxxx)*(source[3].zzzz))).x;
    // 13: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 14: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 15: mul r0.y, r0.y, cb0[3].w
    r0.y = ((r0.yyyy)*(source[3].wwww)).y;
    // 16: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 17: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 18: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 19: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 20: mad r0.xyz, cb0[2].xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((source[2].xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 21: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_mi_00.fx_mi.fx_c_pa_lensflare_01_05_ad; source PS 55883598a583ae4389d79bb0fdf1d392.
float4 WRNative226(WR_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[1u];
    source[2].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_VSourceMaterialTime.xxxx).x;
    source[2].z = ((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))).x;
    source[2].w = (((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[3].x = (sin(((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[3].z = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[3].w = (((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))).x;
    source[4].x = (sin(((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0)))).x;
    source[4].y = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))).x;
    source[4].z = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[4].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 3: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 4: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 5: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 6: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 7: mul r0.y, r0.y, l(20.000000)
    r0.y = ((r0.yyyy)*(float4(20.000000,20.000000,20.000000,20.000000))).y;
    // 8: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 9: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 10: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, l(1.700000)
    r0.y = ((r0.yyyy)*(float4(1.700000,1.700000,1.700000,1.700000))).y;
    // 12: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 13: mad r1.y, v2.y, l(0.500000), cb0[2].x
    r1.y = ((v2.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].xxxx)).y;
    // 14: mov r1.x, v2.x
    r1.x = (v2.xxxx).x;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t1.wxyz, s1, l(0.000000)
    r0.yzw = (VNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 16: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 17: mad r0.w, cb0[3].z, l(0.300000), l(0.700000)
    r0.w = ((source[3].zzzz)*(float4(0.300000,0.300000,0.300000,0.300000))+(float4(0.700000,0.700000,0.700000,0.700000))).w;
    // 18: mad r1.x, cb0[4].z, l(0.300000), l(0.700000)
    r1.x = ((source[4].zzzz)*(float4(0.300000,0.300000,0.300000,0.300000))+(float4(0.700000,0.700000,0.700000,0.700000))).x;
    // 19: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 20: mul r1.xyz, r0.xyzx, r0.wwww
    r1.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 21: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 22: mad r0.xyz, -r0.wwww, r0.xyzx, r1.wwww
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r1.wwww)).xyz;
    // 23: mad r0.xyz, cb0[4].wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((source[4].wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 24: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 25: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 26: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 27: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 28: source device depth mapped to centimetre view depth; reconstruction at 30.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 30-33: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 34: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 35: add r1.x, -cb0[5].x, l(1.000000)
    r1.x = ((-(source[5].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: max r1.x, r1.x, l(0.001000)
    r1.x = (max(r1.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 37: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 38: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 39: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 40: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 41: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_03.fx_mi.fx_j_pa_ringline_01_tr; source PS 1166c98a352b934594aa881b9923e0f3.
float4 WRNative229(WR_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[0u];
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (VNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 3: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 4: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 5: mov_sat r0.w, r0.w
    r0.w = (saturate(r0.wwww)).w;
    // 6: mul o0.w, r0.w, cb0[0].x
    output.w = ((r0.wwww)*(source[0].xxxx)).w;
    return output;
}

// fx_m_mi_l_00.fx_mi.fx_l_pa_spritewave_01_85_tr; source PS 47410d5ec7254248abecb44e367cd009.
float4 WRNative230(WR_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[12u];
    source[2] = VNativeAppend(g_VSourceMaterialParameters[5u].yyyy,g_VSourceMaterialParameters[5u].zzzz,1u);
    source[3] = VNativeAppend(cos(((g_VSourceMaterialParameters[6u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_VSourceMaterialParameters[6u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = VNativeAppend(sin(((g_VSourceMaterialParameters[6u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_VSourceMaterialParameters[6u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = VNativeAppend(cos((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = VNativeAppend(sin((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = g_VSourceMaterialParameters[11u];
    source[8].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_VSourceMaterialParameters[6u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[8].y = (cos(((g_VSourceMaterialParameters[6u].yyyy*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[8].z = (g_VSourceMaterialParameters[5u].wwww).x;
    source[8].w = (g_VSourceMaterialTime.xxxx).x;
    source[9].x = (g_VSourceMaterialParameters[6u].wwww).x;
    source[9].y = (g_VSourceMaterialParameters[7u].xxxx).x;
    source[9].z = (g_VSourceMaterialParameters[6u].zzzz).x;
    source[9].w = (g_VSourceMaterialParameters[6u].xxxx).x;
    source[10].x = (g_VSourceMaterialParameters[4u].wwww).x;
    source[10].y = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[10].z = (g_VSourceMaterialParameters[9u].zzzz).x;
    source[10].w = (g_VSourceMaterialParameters[10u].xxxx).x;
    source[11].x = (g_VSourceMaterialParameters[10u].yyyy).x;
    source[11].y = (g_VSourceMaterialParameters[9u].wwww).x;
    source[11].z = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[11].w = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[12].x = (g_VSourceMaterialParameters[9u].yyyy).x;
    source[12].y = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[12].z = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[12].w = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[13].x = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[13].y = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[13].z = (g_VSourceMaterialParameters[3u].wwww).x;
    source[13].w = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[14].x = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[14].y = (g_VSourceMaterialParameters[2u].wwww).x;
    source[14].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[14].w = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[15].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[15].y = (g_VSourceMaterialParameters[1u].wwww).x;
    source[15].z = ((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[15].w = (sin((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[16].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[16].y = (cos((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[16].z = (g_VSourceMaterialParameters[7u].zzzz).x;
    source[16].w = (g_VSourceMaterialParameters[8u].xxxx).x;
    source[17].x = (g_VSourceMaterialParameters[8u].yyyy).x;
    source[17].y = (g_VSourceMaterialParameters[7u].wwww).x;
    source[17].z = (g_VSourceMaterialParameters[7u].yyyy).x;
    source[17].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[18].x = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[18].y = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[18].z = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[18].w = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[19].x = (g_VSourceMaterialParameters[8u].zzzz).x;
    source[19].y = (g_VSourceMaterialParameters[8u].wwww).x;
    source[19].z = (g_VSourceMaterialParameters[9u].xxxx).x;
    source[19].w = (g_VSourceMaterialParameters[4u].zzzz).x;
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
    // 29: mul r0.w, r0.w, cb0[9].z
    r0.w = ((r0.wwww)*(source[9].zzzz)).w;
    // 30: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 31: lt r1.z, r0.z, l(0.000001)
    r1.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 32: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 34: mul r0.z, r0.z, cb0[19].x
    r0.z = ((r0.zzzz)*(source[19].xxxx)).z;
    // 35: max r0.z, r0.z, cb0[19].z
    r0.z = (max(r0.zzzz,source[19].zzzz)).z;
    // 36: min r0.z, r0.z, cb0[19].y
    r0.z = (min(r0.zzzz,source[19].yyyy)).z;
    // 37: movc r1.y, r1.z, l(0), r0.w
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 38: mul r1.xy, r1.xyxx, cb0[9].xyxx
    r1.xy = ((r1.xyxx)*(source[9].xyxx)).xy;
    // 39: mad r2.x, cb0[8].w, cb0[8].z, r1.x
    r2.x = ((source[8].wwww)*(source[8].zzzz)+(r1.xxxx)).x;
    // 40: mad r2.y, cb0[8].w, cb0[9].w, r1.y
    r2.y = ((source[8].wwww)*(source[9].wwww)+(r1.yyyy)).y;
    // 41: add r1.xy, r2.xyxx, r2.xyxx
    r1.xy = ((r2.xyxx)+(r2.xyxx)).xy;
    // 42: mad r1.xy, cb0[10].xyxx, v4.xxxx, r1.xyxx
    r1.xy = ((source[10].xyxx)*(v4.xxxx)+(r1.xyxx)).xy;
    // 43: mul r0.w, v2.x, cb0[10].w
    r0.w = ((v2.xxxx)*(source[10].wwww)).w;
    // 44: mad r2.x, cb0[8].w, cb0[10].z, r0.w
    r2.x = ((source[8].wwww)*(source[10].zzzz)+(r0.wwww)).x;
    // 45: mul r0.w, v2.y, cb0[11].x
    r0.w = ((v2.yyyy)*(source[11].xxxx)).w;
    // 46: mad r2.y, cb0[8].w, cb0[11].y, r0.w
    r2.y = ((source[8].wwww)*(source[11].yyyy)+(r0.wwww)).y;
    // 47: mad r1.zw, v4.wwww, cb0[11].zzzw, r2.xxxy
    r1.zw = ((v4.wwww)*(source[11].zzzw)+(r2.xxxy)).zw;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t0.yzwx, s1, l(0.000000)
    r0.w = (VNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 49: add r1.z, v4.z, cb0[12].x
    r1.z = ((v4.zzzz)+(source[12].xxxx)).z;
    // 50: mad r1.zw, r0.wwww, r1.zzzz, cb0[2].xxxy
    r1.zw = ((r0.wwww)*(r1.zzzz)+(source[2].xxxy)).zw;
    // 51: add r1.xy, r1.zwzz, r1.xyxx
    r1.xy = ((r1.zwzz)+(r1.xyxx)).xy;
    // 52: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 53: dp2 r2.x, cb0[3].xyxx, r1.xyxx
    r2.x = (dot((source[3].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 54: dp2 r2.y, cb0[4].xyxx, r1.xyxx
    r2.y = (dot((source[4].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 55: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s0, l(-1.000000)
    r0.w = (VNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 57: mul r1.xy, v2.xyxx, cb0[13].xyxx
    r1.xy = ((v2.xyxx)*(source[13].xyxx)).xy;
    // 58: mad r2.x, cb0[8].w, cb0[12].w, r1.x
    r2.x = ((source[8].wwww)*(source[12].wwww)+(r1.xxxx)).x;
    // 59: mad r2.y, cb0[8].w, cb0[13].z, r1.y
    r2.y = ((source[8].wwww)*(source[13].zzzz)+(r1.yyyy)).y;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r2.xyxx, t4.xyzw, s2, l(0.000000)
    r1.x = (VNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 61: mul r1.x, r0.w, r1.x
    r1.x = ((r0.wwww)*(r1.xxxx)).x;
    // 62: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 63: mul r1.y, r1.y, cb0[13].w
    r1.y = ((r1.yyyy)*(source[13].wwww)).y;
    // 64: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 65: mul r1.y, r1.y, cb0[14].x
    r1.y = ((r1.yyyy)*(source[14].xxxx)).y;
    // 66: lt r1.z, |r1.x|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 67: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 68: mad r1.x, r1.x, cb0[14].y, r1.y
    r1.x = ((r1.xxxx)*(source[14].yyyy)+(r1.yyyy)).x;
    // 69: dp2 r2.x, cb0[5].xyxx, r0.xyxx
    r2.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 70: dp2 r2.y, cb0[6].xyxx, r0.xyxx
    r2.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 71: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 72: mul r1.y, v2.x, cb0[16].w
    r1.y = ((v2.xxxx)*(source[16].wwww)).y;
    // 73: mad r2.x, cb0[8].w, cb0[16].z, r1.y
    r2.x = ((source[8].wwww)*(source[16].zzzz)+(r1.yyyy)).x;
    // 74: mul r1.yz, cb0[8].wwww, cb0[17].yywy
    r1.yz = ((source[8].wwww)*(source[17].yywy)).yz;
    // 75: mad r2.y, cb0[17].x, v2.y, r1.y
    r2.y = ((source[17].xxxx)*(v2.yyyy)+(r1.yyyy)).y;
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t2.yxzw, s3, l(0.000000)
    r1.y = (VNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 77: mad r0.xy, r1.yyyy, cb0[17].zzzz, r0.xyxx
    r0.xy = ((r1.yyyy)*(source[17].zzzz)+(r0.xyxx)).xy;
    // 78: mad r2.y, cb0[15].x, r0.y, r1.z
    r2.y = ((source[15].xxxx)*(r0.yyyy)+(r1.zzzz)).y;
    // 79: mul r0.x, r0.x, cb0[14].w
    r0.x = ((r0.xxxx)*(source[14].wwww)).x;
    // 80: mad r2.x, cb0[8].w, cb0[14].z, r0.x
    r2.x = ((source[8].wwww)*(source[14].zzzz)+(r0.xxxx)).x;
    // 81: add r0.xy, r2.xyxx, cb0[18].xyxx
    r0.xy = ((r2.xyxx)+(source[18].xyxx)).xy;
    // 82: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s4, l(0.000000)
    r0.x = (VNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 83: add r0.x, r0.x, l(0.100000)
    r0.x = ((r0.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 84: add r0.y, v4.y, l(-1.000000)
    r0.y = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 85: add_sat r0.x, -r0.y, r0.x
    r0.x = (saturate((-(r0.yyyy))+(r0.xxxx))).x;
    // 86: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 87: mul r0.y, r0.y, cb0[18].w
    r0.y = ((r0.yyyy)*(source[18].wwww)).y;
    // 88: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 89: mul_sat r0.y, r0.y, cb0[18].z
    r0.y = (saturate((r0.yyyy)*(source[18].zzzz))).y;
    // 90: lt r1.y, r0.x, l(0.000001)
    r1.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 91: mul r0.x, r0.x, cb0[18].z
    r0.x = ((r0.xxxx)*(source[18].zzzz)).x;
    // 92: movc r0.y, r1.y, l(-0.000000), -r0.y
    r0.y = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).y;
    // 93: mov_sat r1.y, r0.x
    r1.y = (saturate(r0.xxxx)).y;
    // 94: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 95: mul r0.x, r0.x, cb0[19].w
    r0.x = ((r0.xxxx)*(source[19].wwww)).x;
    // 96: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 97: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 98: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 99: add r0.x, r0.y, r1.y
    r0.x = ((r0.yyyy)+(r1.yyyy)).x;
    // 100: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 101: mad r0.xyz, r0.xxxx, cb0[7].xyzx, r1.xxxx
    r0.xyz = ((r0.xxxx)*(source[7].xyzx)+(r1.xxxx)).xyz;
    // 102: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 103: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_mi_s_00.fx_s_me_spritewave_01_01_tr; source PS 9a1394e98fff254ab3e440007404ba93.
float4 WRNative231(WR_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[11u];
    source[2] = VNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[6u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[6u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = VNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[6u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[6u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = VNativeAppend(g_VSourceMaterialParameters[5u].zzzz,g_VSourceMaterialParameters[5u].wwww,1u);
    source[5] = VNativeAppend(cos(((g_VSourceMaterialParameters[6u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_VSourceMaterialParameters[6u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = VNativeAppend(sin(((g_VSourceMaterialParameters[6u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_VSourceMaterialParameters[6u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = VNativeAppend(cos((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = VNativeAppend(sin((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = g_VSourceMaterialParameters[10u];
    source[10].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_VSourceMaterialParameters[6u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[10].y = (cos(((g_VSourceMaterialParameters[6u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].z = (g_VSourceMaterialParameters[6u].xxxx).x;
    source[10].w = (g_VSourceMaterialTime.xxxx).x;
    source[11].x = (g_VSourceMaterialParameters[6u].wwww).x;
    source[11].y = (g_VSourceMaterialParameters[7u].xxxx).x;
    source[11].z = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[11].w = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[12].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[6u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_VSourceMaterialParameters[6u].yyyy).x;
    source[12].z = (g_VSourceMaterialParameters[8u].wwww).x;
    source[12].w = (g_VSourceMaterialParameters[9u].yyyy).x;
    source[13].x = (g_VSourceMaterialParameters[9u].zzzz).x;
    source[13].y = (g_VSourceMaterialParameters[9u].xxxx).x;
    source[13].z = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[13].w = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[14].x = (g_VSourceMaterialParameters[8u].zzzz).x;
    source[14].y = (g_VSourceMaterialParameters[5u].wwww).x;
    source[14].z = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[14].w = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[15].x = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[15].y = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[15].z = (g_VSourceMaterialParameters[3u].wwww).x;
    source[15].w = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[16].x = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[16].y = (g_VSourceMaterialParameters[2u].wwww).x;
    source[16].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[16].w = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[17].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[17].y = (g_VSourceMaterialParameters[1u].wwww).x;
    source[17].z = ((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[17].w = (sin((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[18].y = (cos((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].z = (g_VSourceMaterialParameters[7u].zzzz).x;
    source[18].w = (g_VSourceMaterialParameters[8u].xxxx).x;
    source[19].x = (g_VSourceMaterialParameters[8u].yyyy).x;
    source[19].y = (g_VSourceMaterialParameters[7u].wwww).x;
    source[19].z = (g_VSourceMaterialParameters[7u].yyyy).x;
    source[19].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[20].x = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[20].y = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[20].z = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[20].w = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[21].x = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[21].y = (g_VSourceMaterialParameters[4u].wwww).x;
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
    r0.x = (VNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r0.x = (VNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 23: mul r1.xy, v2.xyxx, cb0[15].xyxx
    r1.xy = ((v2.xyxx)*(source[15].xyxx)).xy;
    // 24: mad r2.x, cb0[10].w, cb0[14].w, r1.x
    r2.x = ((source[10].wwww)*(source[14].wwww)+(r1.xxxx)).x;
    // 25: mad r2.y, cb0[10].w, cb0[15].z, r1.y
    r2.y = ((source[10].wwww)*(source[15].zzzz)+(r1.yyyy)).y;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xyxx, t4.yxzw, s2, l(0.000000)
    r0.y = (VNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 27: mul r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)*(r0.xxxx)).y;
    // 28: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 29: mul r1.x, r1.x, cb0[15].w
    r1.x = ((r1.xxxx)*(source[15].wwww)).x;
    // 30: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 31: mul r1.x, r1.x, cb0[16].x
    r1.x = ((r1.xxxx)*(source[16].xxxx)).x;
    // 32: lt r1.y, |r0.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 33: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 34: mad r0.y, r0.y, cb0[16].y, r1.x
    r0.y = ((r0.yyyy)*(source[16].yyyy)+(r1.xxxx)).y;
    // 35: dp2 r1.x, cb0[7].xyxx, r0.zwzz
    r1.x = (dot((source[7].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 36: dp2 r1.y, cb0[8].xyxx, r0.zwzz
    r1.y = (dot((source[8].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 37: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 38: mul r1.x, v2.x, cb0[18].w
    r1.x = ((v2.xxxx)*(source[18].wwww)).x;
    // 39: mad r1.x, cb0[10].w, cb0[18].z, r1.x
    r1.x = ((source[10].wwww)*(source[18].zzzz)+(r1.xxxx)).x;
    // 40: mul r1.zw, cb0[10].wwww, cb0[19].yyyw
    r1.zw = ((source[10].wwww)*(source[19].yyyw)).zw;
    // 41: mad r1.y, cb0[19].x, v2.y, r1.z
    r1.y = ((source[19].xxxx)*(v2.yyyy)+(r1.zzzz)).y;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r1.x = (VNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 43: mad r0.zw, r1.xxxx, cb0[19].zzzz, r0.zzzw
    r0.zw = ((r1.xxxx)*(source[19].zzzz)+(r0.zzzw)).zw;
    // 44: mad r1.y, cb0[17].x, r0.w, r1.w
    r1.y = ((source[17].xxxx)*(r0.wwww)+(r1.wwww)).y;
    // 45: mul r0.z, r0.z, cb0[16].w
    r0.z = ((r0.zzzz)*(source[16].wwww)).z;
    // 46: mad r1.x, cb0[10].w, cb0[16].z, r0.z
    r1.x = ((source[10].wwww)*(source[16].zzzz)+(r0.zzzz)).x;
    // 47: add r0.zw, r1.xxxy, cb0[20].xxxy
    r0.zw = ((r1.xxxy)+(source[20].xxxy)).zw;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s4, l(0.000000)
    r0.z = (VNativeSample4((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 49: add r0.z, r0.z, l(0.100000)
    r0.z = ((r0.zzzz)+(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 50: add r0.w, v4.y, l(-1.000000)
    r0.w = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 51: add_sat r0.z, -r0.w, r0.z
    r0.z = (saturate((-(r0.wwww))+(r0.zzzz))).z;
    // 52: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 53: mul r0.w, r0.w, cb0[20].w
    r0.w = ((r0.wwww)*(source[20].wwww)).w;
    // 54: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 55: mul_sat r0.w, r0.w, cb0[20].z
    r0.w = (saturate((r0.wwww)*(source[20].zzzz))).w;
    // 56: lt r1.x, r0.z, l(0.000001)
    r1.x = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 57: mul r0.z, r0.z, cb0[20].z
    r0.z = ((r0.zzzz)*(source[20].zzzz)).z;
    // 58: movc r0.w, r1.x, l(-0.000000), -r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.wwww))).w;
    // 59: mov_sat r1.x, r0.z
    r1.x = (saturate(r0.zzzz)).x;
    // 60: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 61: mul r0.x, r0.x, cb0[21].y
    r0.x = ((r0.xxxx)*(source[21].yyyy)).x;
    // 62: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 63: add r0.z, r0.w, r1.x
    r0.z = ((r0.wwww)+(r1.xxxx)).z;
    // 64: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 65: mad r0.yzw, r0.zzzz, cb0[9].xxyz, r0.yyyy
    r0.yzw = ((r0.zzzz)*(source[9].xxyz)+(r0.yyyy)).yzw;
    // 66: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 67: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 68: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 69: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 70: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 71: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 72: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 73: mul r0.z, r0.z, cb0[21].x
    r0.z = ((r0.zzzz)*(source[21].xxxx)).z;
    // 74: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 75: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 76: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 77: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}

// fx_m_mi_m_00.fx_mi.fx_m_pa_agent_05_20e; source PS 9b7d0436b976c84d8292abbeeb27f843.
float4 WRNative232(WR_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[0u];
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (VNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 2: mul r0.w, r0.x, v3.w
    r0.w = ((r0.xxxx)*(v3.wwww)).w;
    // 3: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 4: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 5: mov_sat r0.w, r0.w
    r0.w = (saturate(r0.wwww)).w;
    // 6: mul o0.w, r0.w, cb0[0].x
    output.w = ((r0.wwww)*(source[0].xxxx)).w;
    return output;
}

// fx_m_mi_j_00.fx_mi.fx_j_pa_customparticle_01_06_ad; source PS 19b7489e330ace469d3d823a77aa3f33.
float4 WRNative233(WR_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[7u];
    source[2] = g_VSourceMaterialParameters[6u];
    source[3] = VNativeAppend(cos((g_VSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = VNativeAppend(sin((g_VSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_VSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_VSourceMaterialParameters[2u].wwww).x;
    source[5].y = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[5].z = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[5].w = (g_VSourceMaterialTime.xxxx).x;
    source[6].x = ((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[2u].yyyy)).x;
    source[6].y = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[6].z = (g_VSourceMaterialParameters[3u].wwww).x;
    source[6].w = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[7].y = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[7].z = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[7].w = (g_VSourceMaterialParameters[0u].wwww).x;
    source[8].x = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[8].z = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[8].w = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[9].x = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[9].y = (g_VSourceMaterialParameters[5u].wwww).x;
    source[9].z = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[9].w = (g_VSourceMaterialParameters[1u].wwww).x;
    source[10].x = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[10].y = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[10].z = (g_VSourceMaterialParameters[4u].wwww).x;
    source[10].w = ((g_VSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[11].x = (sin((g_VSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[11].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[11].z = (cos((g_VSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[11].w = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[12].x = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[12].y = (g_VSourceMaterialParameters[4u].zzzz).x;
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
    // 25: mul r1.xy, v4.zxzz, cb0[8].xyxx
    r1.xy = ((v4.zxzz)*(source[8].xyxx)).xy;
    // 26: mul r0.w, r1.y, l(0.400000)
    r0.w = ((r1.yyyy)*(float4(0.400000,0.400000,0.400000,0.400000))).w;
    // 27: mad r1.y, r0.z, l(3.183101), r0.w
    r1.y = ((r0.zzzz)*(float4(3.183101,3.183101,3.183101,3.183101))+(r0.wwww)).y;
    // 28: mul r0.z, r0.z, l(0.318310)
    r0.z = ((r0.zzzz)*(float4(0.318310,0.318310,0.318310,0.318310))).z;
    // 29: sincos null, r1.y, r1.y
    r1.y = (cos(r1.yyyy)).y;
    // 30: mul r1.y, r1.y, cb0[9].w
    r1.y = ((r1.yyyy)*(source[9].wwww)).y;
    // 31: mul r1.z, v4.y, cb0[7].w
    r1.z = ((v4.yyyy)*(source[7].wwww)).z;
    // 32: dp2 r1.w, r0.xyxx, r0.xyxx
    r1.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 33: mul r0.xy, r0.xyxx, cb0[11].wwww
    r0.xy = ((r0.xyxx)*(source[11].wwww)).xy;
    // 34: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 35: add r2.x, r1.w, cb0[9].x
    r2.x = ((r1.wwww)+(source[9].xxxx)).x;
    // 36: mad r1.w, -r1.w, l(2.000000), l(1.000000)
    r1.w = ((-(r1.wwww))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 38: div r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)/(r2.xxxx)).z;
    // 39: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 40: mad r1.y, r1.y, l(0.500000), r1.z
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r1.zzzz)).y;
    // 41: mad r1.z, r1.z, l(10.000000), r0.w
    r1.z = ((r1.zzzz)*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.wwww)).z;
    // 42: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 43: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 44: mad r0.z, r1.x, l(0.500000), r0.z
    r0.z = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.zzzz)).z;
    // 45: mul r0.z, r0.z, cb0[8].z
    r0.z = ((r0.zzzz)*(source[8].zzzz)).z;
    // 46: mul r1.x, r1.y, cb0[8].w
    r1.x = ((r1.yyyy)*(source[8].wwww)).x;
    // 47: round_ni r1.y, r0.z
    r1.y = (floor(r0.zzzz)).y;
    // 48: frc r0.z, r0.z
    r0.z = (frac(r0.zzzz)).z;
    // 49: mul r0.z, r0.z, l(3.141590)
    r0.z = ((r0.zzzz)*(float4(3.141590,3.141590,3.141590,3.141590))).z;
    // 50: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 51: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 52: mul r1.yz, r1.yyyy, l(0.000000, 215.399994, 33.099998, 0.000000)
    r1.yz = ((r1.yyyy)*(float4(0.000000,215.399994,33.099998,0.000000))).yz;
    // 53: sincos null, r1.z, r1.z
    r1.z = (cos(r1.zzzz)).z;
    // 54: sincos r1.y, null, r1.y
    r1.y = (sin(r1.yyyy)).y;
    // 55: mad r1.z, r1.z, l(0.300000), l(0.700000)
    r1.z = ((r1.zzzz)*(float4(0.300000,0.300000,0.300000,0.300000))+(float4(0.700000,0.700000,0.700000,0.700000))).z;
    // 56: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 57: mad r0.w, r1.x, l(0.050000), r0.w
    r0.w = ((r1.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))+(r0.wwww)).w;
    // 58: add r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)+(r0.wwww)).w;
    // 59: frc r0.w, r0.w
    r0.w = (frac(r0.wwww)).w;
    // 60: mad r1.x, r1.z, l(-60.000000), l(95.000000)
    r1.x = ((r1.zzzz)*(float4(-60.000000,-60.000000,-60.000000,-60.000000))+(float4(95.000000,95.000000,95.000000,95.000000))).x;
    // 61: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 62: mul r0.w, r0.w, cb0[10].x
    r0.w = ((r0.wwww)*(source[10].xxxx)).w;
    // 63: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 64: mad r0.w, r0.w, r0.w, -cb0[9].y
    r0.w = ((r0.wwww)*(r0.wwww)+(-(source[9].yyyy))).w;
    // 65: add r1.x, -cb0[9].y, cb0[9].z
    r1.x = ((-(source[9].yyyy))+(source[9].zzzz)).x;
    // 66: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 67: mul_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)*(r1.xxxx))).w;
    // 68: mad r1.x, r0.w, l(-2.000000), l(3.000000)
    r1.x = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 69: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 70: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 71: mul r0.w, r0.w, l(3.141590)
    r0.w = ((r0.wwww)*(float4(3.141590,3.141590,3.141590,3.141590))).w;
    // 72: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 73: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 74: mul r0.w, r0.w, l(5.000000)
    r0.w = ((r0.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 75: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 76: mul r0.z, r2.x, r0.z
    r0.z = ((r2.xxxx)*(r0.zzzz)).z;
    // 77: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 78: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 79: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 80: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s2, cb0[10].y
    r0.x = (VNativeSample2((r0.xyxx).xy, (source[10].yyyy).x, true).xyzw).x;
    // 81: mul r0.x, r0.x, cb0[12].x
    r0.x = ((r0.xxxx)*(source[12].xxxx)).x;
    // 82: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 83: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 84: mul r0.x, r0.x, cb0[12].y
    r0.x = ((r0.xxxx)*(source[12].yyyy)).x;
    // 85: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 86: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 87: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 88: add r0.y, -v2.y, l(1.000000)
    r0.y = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 89: mul r0.y, r0.y, v2.y
    r0.y = ((r0.yyyy)*(v2.yyyy)).y;
    // 90: mul r0.y, r0.y, l(20.000000)
    r0.y = ((r0.yyyy)*(float4(20.000000,20.000000,20.000000,20.000000))).y;
    // 91: mul r0.y, r1.w, r0.y
    r0.y = ((r1.wwww)*(r0.yyyy)).y;
    // 92: mul r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)*(r0.xxxx)).y;
    // 93: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 94: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 95: mul r1.x, v2.x, cb0[5].x
    r1.x = ((v2.xxxx)*(source[5].xxxx)).x;
    // 96: mad r1.y, v2.y, cb0[5].y, cb0[6].x
    r1.y = ((v2.yyyy)*(source[5].yyyy)+(source[6].xxxx)).y;
    // 97: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = (VNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 98: mad r0.zw, cb0[6].yyyy, r0.zzzw, v2.xxxy
    r0.zw = ((source[6].yyyy)*(r0.zzzw)+(v2.xxxy)).zw;
    // 99: mad r0.zw, r0.zzzw, cb0[6].zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((r0.zzzw)*(source[6].zzzw)+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 100: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s0, l(-1.000000)
    r1.xyz = (VNativeSample0((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 101: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 102: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 103: mad r1.xyz, cb0[7].xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((source[7].xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 104: mul r1.xyz, r1.xyzx, cb0[7].yyyy
    r1.xyz = ((r1.xyzx)*(source[7].yyyy)).xyz;
    // 105: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 106: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 107: mul r1.xyz, r1.xyzx, cb0[7].zzzz
    r1.xyz = ((r1.xyzx)*(source[7].zzzz)).xyz;
    // 108: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 109: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 110: mul r0.xzw, r0.xxxx, r1.xxyz
    r0.xzw = ((r0.xxxx)*(r1.xxyz)).xzw;
    // 111: mad r0.xzw, r0.xxzw, v3.xxyz, cb0[1].xxyz
    r0.xzw = ((r0.xxzw)*(v3.xxyz)+(source[1].xxyz)).xzw;
    // 112: mul r0.xzw, r0.xxzw, v5.wwww
    r0.xzw = ((r0.xxzw)*(v5.wwww)).xzw;
    // 113: mul o0.xyz, r0.yyyy, r0.xzwx
    output.xyz = ((r0.yyyy)*(r0.xzwx)).xyz;
    // 114: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_k_00.fx_mi.fx_k_me_spritewave_01_45_ad; source PS 400c0d19dc79dd408f6933c0203464e3.
float4 WRNative234(WR_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_VSourceMaterialParameters[10u];
    source[3] = input.dynamicParameter;
    source[4] = VNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = VNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = VNativeAppend(g_VSourceMaterialParameters[4u].wwww,g_VSourceMaterialParameters[5u].xxxx,1u);
    source[7] = VNativeAppend(cos(((g_VSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_VSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = VNativeAppend(sin(((g_VSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_VSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = VNativeAppend(cos((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = VNativeAppend(sin((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11] = g_VSourceMaterialParameters[8u];
    source[12].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_VSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[12].y = (cos(((g_VSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[12].z = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[12].w = (g_VSourceMaterialTime.xxxx).x;
    source[13].x = (g_VSourceMaterialParameters[6u].xxxx).x;
    source[13].y = (g_VSourceMaterialParameters[6u].yyyy).x;
    source[13].z = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[13].w = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[14].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_VSourceMaterialParameters[6u].wwww).x;
    source[14].w = (g_VSourceMaterialParameters[7u].yyyy).x;
    source[15].x = (g_VSourceMaterialParameters[7u].zzzz).x;
    source[15].y = (g_VSourceMaterialParameters[7u].xxxx).x;
    source[15].z = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[15].w = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[16].x = (g_VSourceMaterialParameters[6u].zzzz).x;
    source[16].y = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[16].z = (g_VSourceMaterialParameters[4u].wwww).x;
    source[16].w = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[17].x = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[17].y = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[17].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[17].w = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[18].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[18].y = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[18].z = ((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[18].w = (sin((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[19].y = (cos((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].z = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[19].w = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[20].x = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[20].y = (g_VSourceMaterialParameters[1u].wwww).x;
    source[20].z = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[20].w = (g_VSourceMaterialParameters[2u].wwww).x;
    source[21].x = (g_VSourceMaterialParameters[3u].wwww).x;
    source[21].y = (g_VSourceMaterialParameters[4u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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
    // 1: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r1.x, cb0[9].xyxx, r0.xyxx
    r1.x = (dot((source[9].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[10].xyxx, r0.xyxx
    r1.y = (dot((source[10].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 5: mul r0.z, r0.z, cb0[17].w
    r0.z = ((r0.zzzz)*(source[17].wwww)).z;
    // 6: mul r0.w, r0.w, cb0[18].x
    r0.w = ((r0.wwww)*(source[18].xxxx)).w;
    // 7: mad r0.w, cb0[12].w, cb0[19].z, r0.w
    r0.w = ((source[12].wwww)*(source[19].zzzz)+(r0.wwww)).w;
    // 8: add r1.y, r0.w, cb0[20].x
    r1.y = ((r0.wwww)+(source[20].xxxx)).y;
    // 9: mad r0.z, cb0[12].w, cb0[17].z, r0.z
    r0.z = ((source[12].wwww)*(source[17].zzzz)+(r0.zzzz)).z;
    // 10: add r1.x, r0.z, cb0[19].w
    r1.x = ((r0.zzzz)+(source[19].wwww)).x;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t2.yzxw, s2, l(0.000000)
    r0.z = (VNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 12: add r0.z, r0.z, l(0.100000)
    r0.z = ((r0.zzzz)+(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 13: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 14: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 15: mul r0.w, r0.w, cb0[20].y
    r0.w = ((r0.wwww)*(source[20].yyyy)).w;
    // 16: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 17: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 18: add r0.w, cb0[3].y, l(-1.000000)
    r0.w = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 19: add_sat r0.z, -r0.w, r0.z
    r0.z = (saturate((-(r0.wwww))+(r0.zzzz))).z;
    // 20: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 21: mul r0.w, r0.w, cb0[20].w
    r0.w = ((r0.wwww)*(source[20].wwww)).w;
    // 22: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 23: mul_sat r0.w, r0.w, cb0[20].z
    r0.w = (saturate((r0.wwww)*(source[20].zzzz))).w;
    // 24: lt r1.x, r0.z, l(0.000001)
    r1.x = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 25: mul r0.z, r0.z, cb0[20].z
    r0.z = ((r0.zzzz)*(source[20].zzzz)).z;
    // 26: movc r0.w, r1.x, l(-0.000000), -r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.wwww))).w;
    // 27: mov_sat r1.x, r0.z
    r1.x = (saturate(r0.zzzz)).x;
    // 28: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 29: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 30: mul r1.xyz, r0.wwww, cb0[11].xyzx
    r1.xyz = ((r0.wwww)*(source[11].xyzx)).xyz;
    // 31: dp2 r2.x, cb0[4].xyxx, r0.xyxx
    r2.x = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 32: dp2 r2.y, cb0[5].xyxx, r0.xyxx
    r2.y = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 33: mad r0.xy, cb0[13].zwzz, cb0[3].xxxx, r2.xyxx
    r0.xy = ((source[13].zwzz)*(source[3].xxxx)+(r2.xyxx)).xy;
    // 34: add r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 35: mul r0.xy, r0.xyxx, cb0[13].xyxx
    r0.xy = ((r0.xyxx)*(source[13].xyxx)).xy;
    // 36: mad r2.x, cb0[12].w, cb0[12].z, r0.x
    r2.x = ((source[12].wwww)*(source[12].zzzz)+(r0.xxxx)).x;
    // 37: mad r2.y, cb0[12].w, cb0[14].y, r0.y
    r2.y = ((source[12].wwww)*(source[14].yyyy)+(r0.yyyy)).y;
    // 38: mul r0.x, cb0[12].w, cb0[14].z
    r0.x = ((source[12].wwww)*(source[14].zzzz)).x;
    // 39: mad r0.x, cb0[14].w, v4.x, r0.x
    r0.x = ((source[14].wwww)*(v4.xxxx)+(r0.xxxx)).x;
    // 40: mul r0.w, v4.y, cb0[15].x
    r0.w = ((v4.yyyy)*(source[15].xxxx)).w;
    // 41: mad r0.y, cb0[12].w, cb0[15].y, r0.w
    r0.y = ((source[12].wwww)*(source[15].yyyy)+(r0.wwww)).y;
    // 42: mad r0.xy, cb0[3].wwww, cb0[15].zwzz, r0.xyxx
    r0.xy = ((source[3].wwww)*(source[15].zwzz)+(r0.xyxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (VNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 44: add r0.y, cb0[3].z, cb0[16].x
    r0.y = ((source[3].zzzz)+(source[16].xxxx)).y;
    // 45: mad r0.xy, r0.xxxx, r0.yyyy, cb0[6].xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(source[6].xyxx)).xy;
    // 46: add r0.xy, r0.xyxx, r2.xyxx
    r0.xy = ((r0.xyxx)+(r2.xyxx)).xy;
    // 47: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 48: dp2 r2.x, cb0[7].xyxx, r0.xyxx
    r2.x = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 49: dp2 r2.y, cb0[8].xyxx, r0.xyxx
    r2.y = (dot((source[8].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 50: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 51: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s0, l(-1.000000)
    r0.x = (VNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 52: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 53: mul r0.y, r0.y, cb0[16].w
    r0.y = ((r0.yyyy)*(source[16].wwww)).y;
    // 54: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 55: mul r0.y, r0.y, cb0[17].x
    r0.y = ((r0.yyyy)*(source[17].xxxx)).y;
    // 56: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 57: movc r0.y, r0.w, l(0), r0.y
    r0.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 58: mad r0.y, r0.x, cb0[17].y, r0.y
    r0.y = ((r0.xxxx)*(source[17].yyyy)+(r0.yyyy)).y;
    // 59: mad r1.xyz, r0.xxxx, r1.xyzx, r0.yyyy
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yyyy)).xyz;
    // 60: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 61: mul r0.x, r0.x, cb0[21].y
    r0.x = ((r0.xxxx)*(source[21].yyyy)).x;
    // 62: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 63: mad r0.yzw, r1.xxyz, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r1.xxyz)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 64: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 65: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 66: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 67: mul r1.x, r1.x, v6.z
    r1.x = ((r1.xxxx)*(v6.zzzz)).x;
    // 68: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 69: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 70: mul r1.y, r1.y, cb0[21].x
    r1.y = ((r1.yyyy)*(source[21].xxxx)).y;
    // 71: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 72: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 73: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 74: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 75: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 76: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_j_00.fx_mi.fx_j_pa_crackbase_02_03; source PS d3eb4c2eccedae4ea2d3d184bda0478f.
float4 WRNative235(WR_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[6u];
    source[2] = VNativeAppend((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[2u].wwww),(g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[3u].xxxx),1u);
    source[3] = g_VSourceMaterialParameters[5u];
    source[4] = VNativeAppend(g_VSourceMaterialParameters[0u].wwww,g_VSourceMaterialParameters[1u].xxxx,1u);
    source[5] = VNativeAppend(g_VSourceMaterialParameters[1u].wwww,g_VSourceMaterialParameters[2u].xxxx,1u);
    source[6] = g_VSourceMaterialParameters[4u];
    source[7].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[7].y = (g_VSourceMaterialParameters[2u].wwww).x;
    source[7].z = (g_VSourceMaterialTime.xxxx).x;
    source[7].w = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[8].x = ((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[3u].xxxx)).x;
    source[8].y = ((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[2u].wwww)).x;
    source[8].z = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[8].w = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[9].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[9].y = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[9].z = (g_VSourceMaterialParameters[1u].wwww).x;
    source[9].w = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[10].x = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[10].y = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[10].z = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[10].w = (g_VSourceMaterialParameters[2u].zzzz).x;
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
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v2.xyxx, t1.xyzw, s2, l(0.000000)
    r1.xy = (VNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 7: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 8: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 9: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 10: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 11: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 12: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 13: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 14: mul r0.x, r0.x, l(5.000000)
    r0.x = ((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 15: add r0.yz, v2.xxyx, v2.xxyx
    r0.yz = ((v2.xxyx)+(v2.xxyx)).yz;
    // 16: mad r0.yz, cb0[7].xxxx, r1.xxyx, r0.yyzy
    r0.yz = ((source[7].xxxx)*(r1.xxyx)+(r0.yyzy)).yz;
    // 17: add r0.yz, r0.yyzy, cb0[2].xxyx
    r0.yz = ((r0.yyzy)+(source[2].xxyx)).yz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t2.wxyz, s3, l(0.000000)
    r0.yzw = (VNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 19: mul_sat r1.xyzw, r0.yyzw, cb0[3].xxyz
    r1.xyzw = (saturate((r0.yyzw)*(source[3].xxyz))).xyzw;
    // 20: max r1.xyzw, r1.xyzw, l(0.000001, 0.000001, 0.000001, 0.000001)
    r1.xyzw = (max(r1.xyzw,float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 21: log r1.xyzw, r1.xyzw
    r1.xyzw = (log2(r1.xyzw)).xyzw;
    // 22: mul r1.xyzw, r1.xyzw, cb0[8].zzzz
    r1.xyzw = ((r1.xyzw)*(source[8].zzzz)).xyzw;
    // 23: exp r1.xyzw, r1.xyzw
    r1.xyzw = (exp2(r1.xyzw)).xyzw;
    // 24: mul r1.xyzw, r1.xyzw, v4.xxxx
    r1.xyzw = ((r1.xyzw)*(v4.xxxx)).xyzw;
    // 25: mul r0.xyzw, r0.xxxx, r1.xyzw
    r0.xyzw = ((r0.xxxx)*(r1.xyzw)).xyzw;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v2.xyxx, t3.yxzw, s4, l(0.000000)
    r1.x = (VNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 27: mad r1.x, r1.x, l(2.000000), l(-1.000000)
    r1.x = ((r1.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 28: mul r1.x, r1.x, l(0.200000)
    r1.x = ((r1.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 29: div r1.yz, v7.xxyx, v7.wwww
    r1.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 30: mad r1.yz, r1.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r1.yz = ((r1.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 31: mad r1.xy, r1.xxxx, r1.yzyy, r1.yzyy
    r1.xy = ((r1.xxxx)*(r1.yzyy)+(r1.yzyy)).xy;
    // 32: sample_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r1.xyz = (Read_EffectSceneColor(LinearClampUVSampler, (r1.xyxx).xy).xyzw).xyz;
    // 33: mul r1.xyzw, r1.xxyz, v4.yyyy
    r1.xyzw = ((r1.xxyz)*(v4.yyyy)).xyzw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r2.x, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r2.x = (VNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 35: mad r0.xyzw, r2.xxxx, r0.xyzw, r1.xyzw
    r0.xyzw = ((r2.xxxx)*(r0.xyzw)+(r1.xyzw)).xyzw;
    // 36: add r1.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r1.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 37: max r2.x, |r1.z|, |r1.w|
    r2.x = (max(abs(r1.zzzz),abs(r1.wwww))).x;
    // 38: div r2.x, l(1.000000, 1.000000, 1.000000, 1.000000), r2.x
    r2.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.xxxx)).x;
    // 39: min r2.y, |r1.z|, |r1.w|
    r2.y = (min(abs(r1.zzzz),abs(r1.wwww))).y;
    // 40: mul r2.x, r2.x, r2.y
    r2.x = ((r2.xxxx)*(r2.yyyy)).x;
    // 41: mul r2.y, r2.x, r2.x
    r2.y = ((r2.xxxx)*(r2.xxxx)).y;
    // 42: mad r2.z, r2.y, l(0.020835), l(-0.085133)
    r2.z = ((r2.yyyy)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 43: mad r2.z, r2.y, r2.z, l(0.180141)
    r2.z = ((r2.yyyy)*(r2.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 44: mad r2.z, r2.y, r2.z, l(-0.330299)
    r2.z = ((r2.yyyy)*(r2.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 45: mad r2.y, r2.y, r2.z, l(0.999866)
    r2.y = ((r2.yyyy)*(r2.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).y;
    // 46: mul r2.z, r2.y, r2.x
    r2.z = ((r2.yyyy)*(r2.xxxx)).z;
    // 47: mad r2.z, r2.z, l(-2.000000), l(1.570796)
    r2.z = ((r2.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 48: lt r2.w, |r1.z|, |r1.w|
    r2.w = (asfloat((uint4)((abs(r1.zzzz))<(abs(r1.wwww))) * 0xffffffffu)).w;
    // 49: and r2.z, r2.w, r2.z
    r2.z = (asfloat(asuint(r2.wwww) & asuint(r2.zzzz))).z;
    // 50: mad r2.x, r2.x, r2.y, r2.z
    r2.x = ((r2.xxxx)*(r2.yyyy)+(r2.zzzz)).x;
    // 51: lt r2.y, r1.z, -r1.z
    r2.y = (asfloat((uint4)((r1.zzzz)<(-(r1.zzzz))) * 0xffffffffu)).y;
    // 52: and r2.y, r2.y, l(0xc0490fdb)
    r2.y = (asfloat(asuint(r2.yyyy) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).y;
    // 53: add r2.x, r2.y, r2.x
    r2.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 54: min r2.y, r1.z, r1.w
    r2.y = (min(r1.zzzz,r1.wwww)).y;
    // 55: lt r2.y, r2.y, -r2.y
    r2.y = (asfloat((uint4)((r2.yyyy)<(-(r2.yyyy))) * 0xffffffffu)).y;
    // 56: max r2.z, r1.z, r1.w
    r2.z = (max(r1.zzzz,r1.wwww)).z;
    // 57: ge r2.z, r2.z, -r2.z
    r2.z = (asfloat((uint4)((r2.zzzz)>=(-(r2.zzzz))) * 0xffffffffu)).z;
    // 58: and r2.y, r2.z, r2.y
    r2.y = (asfloat(asuint(r2.zzzz) & asuint(r2.yyyy))).y;
    // 59: movc r2.x, r2.y, -r2.x, r2.x
    r2.x = ((asuint(r2.yyyy) != 0u) ? (-(r2.xxxx)) : (r2.xxxx)).x;
    // 60: mad r2.x, r2.x, l(0.318471), l(1.000000)
    r2.x = ((r2.xxxx)*(float4(0.318471,0.318471,0.318471,0.318471))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 61: mul r2.x, r2.x, l(0.500000)
    r2.x = ((r2.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 62: dp2 r1.z, -r1.zwzz, -r1.zwzz
    r1.z = (dot((-(r1.zwzz)).xy,(-(r1.zwzz)).xy).xxxx).z;
    // 63: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 64: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 65: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 66: sqrt r2.y, r1.z
    r2.y = (sqrt(r1.zzzz)).y;
    // 67: mov r3.xz, l(0,0,0,0)
    r3.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 68: mul r3.yw, v4.wwwz, l(0.000000, 0.050000, 0.000000, -0.500000)
    r3.yw = ((v4.wwwz)*(float4(0.000000,0.050000,0.000000,-0.500000))).yw;
    // 69: mad r1.yz, cb0[5].xxyx, r2.xxyx, r3.xxyx
    r1.yz = ((source[5].xxyx)*(r2.xxyx)+(r3.xxyx)).yz;
    // 70: sample_l_indexable(texture2d)(float,float,float,float) r1.yz, r1.yzyy, t5.zxyw, s6, l(-1.000000)
    r1.yz = (VNativeSample5((r1.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).zxyw).yz;
    // 71: mul r1.yz, r1.yyzy, l(0.000000, 0.600000, 0.600000, 0.000000)
    r1.yz = ((r1.yyzy)*(float4(0.000000,0.600000,0.600000,0.000000))).yz;
    // 72: mad r1.yz, r2.xxyx, cb0[4].xxyx, r1.yyzy
    r1.yz = ((r2.xxyx)*(source[4].xxyx)+(r1.yyzy)).yz;
    // 73: add r1.yz, r1.yyzy, r3.zzwz
    r1.yz = ((r1.yyzy)+(r3.zzwz)).yz;
    // 74: sample_l_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t6.wxyz, s5, l(-1.000000)
    r1.yzw = (VNativeSample4((r1.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 75: max r2.xyzw, |r1.yyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r2.xyzw = (max(abs(r1.yyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 76: log r2.xyzw, r2.xyzw
    r2.xyzw = (log2(r2.xyzw)).xyzw;
    // 77: mul r2.xyzw, r2.xyzw, cb0[9].wwww
    r2.xyzw = ((r2.xyzw)*(source[9].wwww)).xyzw;
    // 78: exp r2.xyzw, r2.xyzw
    r2.xyzw = (exp2(r2.xyzw)).xyzw;
    // 79: mul r2.xyzw, r2.xyzw, cb0[10].xxxx
    r2.xyzw = ((r2.xyzw)*(source[10].xxxx)).xyzw;
    // 80: mad r0.xyzw, r2.xyzw, cb0[6].xxyz, r0.xyzw
    r0.xyzw = ((r2.xyzw)*(source[6].xxyz)+(r0.xyzw)).xyzw;
    // 81: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 82: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 83: mul r1.y, r1.y, cb0[10].y
    r1.y = ((r1.yyyy)*(source[10].yyyy)).y;
    // 84: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 85: mul_sat r1.y, r1.y, cb0[10].z
    r1.y = (saturate((r1.yyyy)*(source[10].zzzz))).y;
    // 86: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 87: mul r1.y, r1.x, cb0[10].w
    r1.y = ((r1.xxxx)*(source[10].wwww)).y;
    // 88: mul r0.xyzw, r0.xyzw, r1.yxxx
    r0.xyzw = ((r0.xyzw)*(r1.yxxx)).xyzw;
    // 89: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 90: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 91: mul_sat r0.xyz, r0.yzwy, v3.xyzx
    r0.xyz = (saturate((r0.yzwy)*(v3.xyzx))).xyz;
    // 92: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 93: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_mi_00.fx_mi.fx_a_pa_gl_01_4_ad; source PS b0d551a53864c9408d2c03215f9ef88f.
float4 WRNative239(WR_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[2u];
    source[2].x = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_VSourceMaterialParameters[0u].wwww).x;
    source[3].x = (g_VSourceMaterialParameters[0u].xxxx).x;
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
    r1.xyzw = (VNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

// fx_m_mi_01.fx_mi.fx_k_pa_glow_01_ad_dt; source PS 2bd85c08a26e594b945c597997daffea.
float4 WRNative241(WR_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[2u];
    source[2].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_VSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
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
    // 29: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 30: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 31: source device depth mapped to centimetre view depth; reconstruction at 33.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 33-36: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 37: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 38: mul_sat r0.w, r0.w, l(0.020000)
    r0.w = (saturate((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000)))).w;
    // 39: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 40: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 41: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 42: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_w_00.mi.fx_w_me_master_01_1_ad; source PS 248e74fc7772154691d1ccd53c98e1d1.
float4 WRNative243(WR_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_VSourceMaterialParameters[5u];
    source[3] = g_VSourceMaterialParameters[3u];
    source[4] = input.dynamicParameter;
    source[5] = VNativeAppend(g_VSourceMaterialParameters[1u].yyyy,g_VSourceMaterialParameters[1u].zzzz,1u);
    source[6].x = (g_VSourceMaterialParameters[2u].wwww).x;
    source[6].y = (g_VSourceMaterialTime.xxxx).x;
    source[6].z = (g_VSourceMaterialParameters[0u].wwww).x;
    source[6].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[7].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[7].y = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[7].z = (g_VSourceMaterialParameters[1u].wwww).x;
    source[7].w = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[8].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[1u].wwww)).x;
    source[8].y = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[8].z = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[8].w = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[9].x = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[9].y = (g_VSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.x, cb0[4].y, l(-1.000000)
    r0.x = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mad r0.y, cb0[6].y, cb0[7].w, cb0[8].x
    r0.y = ((source[6].yyyy)*(source[7].wwww)+(source[8].xxxx)).y;
    // 3: sincos r1.x, r2.x, r0.y
    r1.x = (sin(r0.yyyy)).x; r2.x = (cos(r0.yyyy)).x;
    // 4: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 5: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 6: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 7: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 8: dp2 r0.w, r3.zyzz, r0.yzyy
    r0.w = (dot((r3.zyzz).xy,(r0.yzyy).xy).xxxx).w;
    // 9: dp2 r0.y, r3.yxyy, r0.yzyy
    r0.y = (dot((r3.yxyy).xy,(r0.yzyy).xy).xxxx).y;
    // 10: mad r0.x, r0.y, cb0[5].x, r0.x
    r0.x = ((r0.yyyy)*(source[5].xxxx)+(r0.xxxx)).x;
    // 11: mul r0.z, r0.w, cb0[5].y
    r0.z = ((r0.wwww)*(source[5].yyyy)).z;
    // 12: add r0.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (VNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 14: mul r0.y, v4.x, cb0[6].w
    r0.y = ((v4.xxxx)*(source[6].wwww)).y;
    // 15: mul r0.z, cb0[6].x, cb0[6].y
    r0.z = ((source[6].xxxx)*(source[6].yyyy)).z;
    // 16: mad r1.x, r0.z, cb0[6].z, r0.y
    r1.x = ((r0.zzzz)*(source[6].zzzz)+(r0.yyyy)).x;
    // 17: mul r0.y, v4.y, cb0[7].x
    r0.y = ((v4.yyyy)*(source[7].xxxx)).y;
    // 18: mad r1.y, r0.z, cb0[7].y, r0.y
    r1.y = ((r0.zzzz)*(source[7].yyyy)+(r0.yyyy)).y;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (VNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 20: add r0.z, -cb0[4].x, l(1.000000)
    r0.z = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 21: mad r0.x, r0.y, r0.x, -r0.z
    r0.x = ((r0.yyyy)*(r0.xxxx)+(-(r0.zzzz))).x;
    // 22: mul_sat r0.x, r0.x, cb0[8].w
    r0.x = (saturate((r0.xxxx)*(source[8].wwww))).x;
    // 23: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 24: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 25: mul r0.y, r0.y, cb0[9].x
    r0.y = ((r0.yyyy)*(source[9].xxxx)).y;
    // 26: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 27: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 28: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 29: source device depth mapped to centimetre view depth; reconstruction at 31.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 31-34: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 35: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 36: add r0.w, -cb0[9].y, l(1.000000)
    r0.w = ((-(source[9].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: max r0.w, -r0.w, l(0.001000)
    r0.w = (max(-(r0.wwww),float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 38: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 39: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 40: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 41: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 42: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 43: mul r0.yzw, cb0[3].xxyz, cb0[3].wwww
    r0.yzw = ((source[3].xxyz)*(source[3].wwww)).yzw;
    // 44: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 45: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 46: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 47: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_k_00.fx_mi.fx_k_pa_ringmaster_01_19_ad; source PS 393b6244b5ab25409fe047d7a112d339.
float4 WRNative244(WR_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[8u];
    source[2] = g_VSourceMaterialParameters[7u];
    source[3].x = (g_VSourceMaterialParameters[6u].zzzz).x;
    source[3].y = (g_VSourceMaterialTime.xxxx).x;
    source[3].z = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[3].w = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[4].x = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[4].y = (g_VSourceMaterialParameters[3u].wwww).x;
    source[4].z = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[5].x = (g_VSourceMaterialParameters[2u].wwww).x;
    source[5].y = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[5].z = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[5].w = (g_VSourceMaterialParameters[4u].wwww).x;
    source[6].x = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[6].y = (g_VSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_VSourceMaterialParameters[1u].wwww).x;
    source[7].x = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[7].y = (g_VSourceMaterialParameters[5u].wwww).x;
    source[7].z = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[2u].xxxx)).x;
    source[8].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[2u].xxxx))).x;
    source[8].y = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[2u].yyyy)).x;
    source[8].w = (max((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[2u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].x = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[2u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[9].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[0u].yyyy)).x;
    source[9].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[0u].yyyy))).x;
    source[10].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[10].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].zzzz)).x;
    source[10].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[10].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[11].x = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[11].y = (g_VSourceMaterialParameters[6u].xxxx).x;
    source[11].z = (g_VSourceMaterialParameters[6u].yyyy).x;
    source[11].w = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[12].x = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[12].y = (g_VSourceMaterialParameters[5u].yyyy).x;
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
    // 27: mad r1.x, r0.y, l(0.159155), l(0.500000)
    r1.x = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 28: mul r0.y, v4.y, cb0[5].z
    r0.y = ((v4.yyyy)*(source[5].zzzz)).y;
    // 29: mul r0.z, v2.x, cb0[4].w
    r0.z = ((v2.xxxx)*(source[4].wwww)).z;
    // 30: mul r0.w, cb0[3].x, cb0[3].y
    r0.w = ((source[3].xxxx)*(source[3].yyyy)).w;
    // 31: mad r2.x, r0.w, cb0[4].z, r0.z
    r2.x = ((r0.wwww)*(source[4].zzzz)+(r0.zzzz)).x;
    // 32: mul r1.zw, r0.wwww, cb0[5].yyyw
    r1.zw = ((r0.wwww)*(source[5].yyyw)).zw;
    // 33: mad r2.y, cb0[5].x, v2.y, r1.z
    r2.y = ((source[5].xxxx)*(v2.yyyy)+(r1.zzzz)).y;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xy = (VNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 35: mul r0.z, v4.z, cb0[4].y
    r0.z = ((v4.zzzz)*(source[4].yyyy)).z;
    // 36: add r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)+(r0.xxxx)).z;
    // 37: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 38: mul r0.z, r0.z, r1.z
    r0.z = ((r0.zzzz)*(r1.zzzz)).z;
    // 39: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 40: lt r1.z, r0.x, l(0.000000)
    r1.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 41: movc r1.y, r1.z, l(0), r0.z
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 42: mad r0.yz, r0.yyyy, r2.xxyx, r1.xxyx
    r0.yz = ((r0.yyyy)*(r2.xxyx)+(r1.xxyx)).yz;
    // 43: mul r1.x, r0.y, cb0[3].w
    r1.x = ((r0.yyyy)*(source[3].wwww)).x;
    // 44: mad r1.x, r0.w, cb0[3].z, r1.x
    r1.x = ((r0.wwww)*(source[3].zzzz)+(r1.xxxx)).x;
    // 45: mad r1.y, cb0[4].x, r0.z, r1.w
    r1.y = ((source[4].xxxx)*(r0.zzzz)+(r1.wwww)).y;
    // 46: mul r0.yz, r0.yyzy, cb0[6].yyzy
    r0.yz = ((r0.yyzy)*(source[6].yyzy)).yz;
    // 47: mad r0.yz, r0.wwww, cb0[6].xxwx, r0.yyzy
    r0.yz = ((r0.wwww)*(source[6].xxwx)+(r0.yyzy)).yz;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t3.wxyz, s3, l(-1.000000)
    r0.yzw = (VNativeSample2((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 49: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s1, l(-1.000000)
    r1.xyz = (VNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 50: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 51: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 52: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 53: mad r0.yzw, cb0[7].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[7].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 54: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 55: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 56: mul r0.yzw, r0.yyzw, cb0[7].yyyy
    r0.yzw = ((r0.yyzw)*(source[7].yyyy)).yzw;
    // 57: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 58: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 59: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 60: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 61: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 62: mad r1.x, -r0.x, cb0[8].x, l(1.000000)
    r1.x = ((-(r0.xxxx))*(source[8].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 63: mad r0.x, -r0.x, cb0[9].w, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[9].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 64: mul_sat r0.x, r0.x, cb0[10].w
    r0.x = (saturate((r0.xxxx)*(source[10].wwww))).x;
    // 65: mul_sat r1.x, r1.x, cb0[9].x
    r1.x = (saturate((r1.xxxx)*(source[9].xxxx))).x;
    // 66: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 67: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 68: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 69: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 70: mul r0.x, r0.x, cb0[11].x
    r0.x = ((r0.xxxx)*(source[11].xxxx)).x;
    // 71: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 72: mul_sat r0.x, r0.x, cb0[11].y
    r0.x = (saturate((r0.xxxx)*(source[11].yyyy))).x;
    // 73: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 74: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 75: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 76: mul r0.x, r0.x, cb0[11].z
    r0.x = ((r0.xxxx)*(source[11].zzzz)).x;
    // 77: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 78: div r1.yz, v7.xxyx, v7.wwww
    r1.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 79: mad r1.yz, r1.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r1.yz = ((r1.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 80: source device depth mapped to centimetre view depth; reconstruction at 82.
    r1.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.yzyy).xy, 0.f).y * 100000.f;
    // Native 82-85: reconstructed view depth is supplied by the runtime adapter.
    r1.y = r1.y;
    // 86: add r1.y, r1.y, -v7.w
    r1.y = ((r1.yyyy)+(-(v7.wwww))).y;
    // 87: add r1.z, -cb0[11].w, l(1.000000)
    r1.z = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 88: max r1.z, -r1.z, l(0.001000)
    r1.z = (max(-(r1.zzzz),float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 89: div_sat r1.y, r1.y, r1.z
    r1.y = (saturate((r1.yyyy)/(r1.zzzz))).y;
    // 90: dp3 r1.z, v6.xyzx, v6.xyzx
    r1.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 91: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 92: mul r1.z, r1.z, v6.z
    r1.z = ((r1.zzzz)*(v6.zzzz)).z;
    // 93: log r1.w, |r1.z|
    r1.w = (log2(abs(r1.zzzz))).w;
    // 94: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 95: mul r1.w, r1.w, cb0[12].x
    r1.w = ((r1.wwww)*(source[12].xxxx)).w;
    // 96: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 97: mul_sat r1.w, r1.w, cb0[12].y
    r1.w = (saturate((r1.wwww)*(source[12].yyyy))).w;
    // 98: mul r1.y, r1.y, r1.w
    r1.y = ((r1.yyyy)*(r1.wwww)).y;
    // 99: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 100: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 101: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 102: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 103: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 104: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 105: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_k_00.fx_mi.fx_j_me_shine_02_01_ad; source PS c23e4ebaf28ad84aa6a005f5bcb0f3bd.
float4 WRNative245(WR_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_VSourceMaterialParameters[6u];
    source[3] = VNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = VNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5] = g_VSourceMaterialParameters[5u];
    source[6].x = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_VSourceMaterialTime.xxxx).x;
    source[6].z = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_VSourceMaterialParameters[0u].wwww).x;
    source[7].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[7].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[7].z = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[7].w = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_VSourceMaterialParameters[1u].wwww).x;
    source[8].z = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[9].x = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[9].y = (g_VSourceMaterialParameters[3u].wwww).x;
    source[9].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[9].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[10].x = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[10].y = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[10].z = (g_VSourceMaterialParameters[3u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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
    // 1: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: add r0.zw, -r0.xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r0.xxxy))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 6: mul_sat r0.z, r0.x, r0.z
    r0.z = (saturate((r0.xxxx)*(r0.zzzz))).z;
    // 7: mul_sat r0.w, r0.w, cb0[10].y
    r0.w = (saturate((r0.wwww)*(source[10].yyyy))).w;
    // 8: mul r1.x, r0.z, l(4.000000)
    r1.x = ((r0.zzzz)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 9: lt r0.z, r0.z, l(0.000000)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 10: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 11: mul r1.x, r1.x, cb0[9].x
    r1.x = ((r1.xxxx)*(source[9].xxxx)).x;
    // 12: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 13: mul r1.x, r1.x, cb0[9].y
    r1.x = ((r1.xxxx)*(source[9].yyyy)).x;
    // 14: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 15: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 16: mul r1.x, r1.x, cb0[9].z
    r1.x = ((r1.xxxx)*(source[9].zzzz)).x;
    // 17: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 18: mul r1.x, r1.x, cb0[9].w
    r1.x = ((r1.xxxx)*(source[9].wwww)).x;
    // 19: lt r1.y, |r0.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 20: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 21: mul_sat r0.z, r0.z, r1.x
    r0.z = (saturate((r0.zzzz)*(r1.xxxx))).z;
    // 22: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 23: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 24: mul r1.x, r1.x, cb0[10].z
    r1.x = ((r1.xxxx)*(source[10].zzzz)).x;
    // 25: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 26: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 27: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 28: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 29: mul r1.x, r1.x, v6.z
    r1.x = ((r1.xxxx)*(v6.zzzz)).x;
    // 30: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 31: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 32: mul r1.y, r1.y, cb0[10].x
    r1.y = ((r1.yyyy)*(source[10].xxxx)).y;
    // 33: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 34: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 35: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 36: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 37: mul_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)*(source[1].wwww))).z;
    // 38: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 39: mul r1.xy, r0.xyxx, cb0[6].zwzz
    r1.xy = ((r0.xyxx)*(source[6].zwzz)).xy;
    // 40: mul r0.xy, r0.xyxx, cb0[8].xyxx
    r0.xy = ((r0.xyxx)*(source[8].xyxx)).xy;
    // 41: mad r2.x, cb0[6].y, cb0[6].x, r1.x
    r2.x = ((source[6].yyyy)*(source[6].xxxx)+(r1.xxxx)).x;
    // 42: mad r2.y, cb0[6].y, cb0[7].z, r1.y
    r2.y = ((source[6].yyyy)*(source[7].zzzz)+(r1.yyyy)).y;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyz = (VNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 44: mad r2.x, cb0[6].y, cb0[7].w, r0.x
    r2.x = ((source[6].yyyy)*(source[7].wwww)+(r0.xxxx)).x;
    // 45: mad r2.y, cb0[6].y, cb0[8].z, r0.y
    r2.y = ((source[6].yyyy)*(source[8].zzzz)+(r0.yyyy)).y;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r2.xyxx, t1.xywz, s1, l(0.000000)
    r0.xyw = (VNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 47: mul r2.xyz, r0.xywx, r1.xyzx
    r2.xyz = ((r0.xywx)*(r1.xyzx)).xyz;
    // 48: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 49: mad r0.xyw, -r1.xyxz, r0.xyxw, r1.wwww
    r0.xyw = ((-(r1.xyxz))*(r0.xyxw)+(r1.wwww)).xyw;
    // 50: mad r0.xyw, cb0[8].wwww, r0.xyxw, r2.xyxz
    r0.xyw = ((source[8].wwww)*(r0.xyxw)+(r2.xyxz)).xyw;
    // 51: mul r1.xyz, cb0[5].xyzx, cb0[5].wwww
    r1.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 52: mul r0.xyw, r0.xyxw, r1.xyxz
    r0.xyw = ((r0.xyxw)*(r1.xyxz)).xyw;
    // 53: mad r0.xyw, r0.xyxw, cb0[1].xyxz, cb0[2].xyxz
    r0.xyw = ((r0.xyxw)*(source[1].xyxz)+(source[2].xyxz)).xyw;
    // 54: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 55: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 56: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad; source PS ff1194d8453ded4fba5fe87ac55b4348.
float4 WRNative246(WR_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[2u];
    source[2].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_VSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
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

// fx_m_mi_j_00.fx_mi.fx_j_pa_shine_01_01_ad; source PS 1f3e2fdc0f35504493c2d7fca9bd10b1.
float4 WRNative247(WR_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[5u];
    source[2].x = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[2].y = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[2].z = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[2].w = (g_VSourceMaterialTime.xxxx).x;
    source[3].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_VSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[3].w = (g_VSourceMaterialParameters[2u].wwww).x;
    source[4].x = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[4].y = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[4].z = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[4].w = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[5].x = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[5].z = (g_VSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[6].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[6].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[6].z = (g_VSourceMaterialParameters[3u].wwww).x;
    source[6].w = ((float4(100.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[3u].wwww)).x;
    source[7].x = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[3u].wwww))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
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
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (VNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[4].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[4].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 7: mul r0.zw, r0.xxxy, cb0[3].xxxy
    r0.zw = ((r0.xxxy)*(source[3].xxxy)).zw;
    // 8: mul r0.xy, r0.xyxx, cb0[5].yzyy
    r0.xy = ((r0.xyxx)*(source[5].yzyy)).xy;
    // 9: mad r0.xy, cb0[2].wwww, cb0[5].xwxx, r0.xyxx
    r0.xy = ((source[2].wwww)*(source[5].xwxx)+(r0.xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (VNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: mad r1.x, cb0[2].w, cb0[2].z, r0.z
    r1.x = ((source[2].wwww)*(source[2].zzzz)+(r0.zzzz)).x;
    // 12: mad r1.y, cb0[2].w, cb0[4].w, r0.w
    r1.y = ((source[2].wwww)*(source[4].wwww)+(r0.wwww)).y;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t1.yxzw, s2, l(0.000000)
    r0.y = (VNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    // 36: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 37: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 38: source device depth mapped to centimetre view depth; reconstruction at 40.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 40-43: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 44: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 45: add r0.z, -cb0[7].x, l(1.000000)
    r0.z = ((-(source[7].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 47: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 48: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 49: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 50: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 51: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 52: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 53: mul r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 54: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 55: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 56: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 57: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 58: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 59: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 60: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 61: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_j_00.fx_mi.fx_j_pa_movedissolve_01_01_tr; source PS 6d074377ba6fcc4a9c9df12b81f1e545.
float4 WRNative248(WR_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[8u];
    source[2] = g_VSourceMaterialParameters[6u];
    source[3] = g_VSourceMaterialParameters[7u];
    source[4].x = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[4].y = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[4].z = (g_VSourceMaterialParameters[3u].wwww).x;
    source[4].w = (g_VSourceMaterialTime.xxxx).x;
    source[5].x = (g_VSourceMaterialParameters[2u].wwww).x;
    source[5].y = ((g_VSourceMaterialParameters[2u].wwww*g_VSourceMaterialTime.xxxx)).x;
    source[5].z = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[5].w = ((g_VSourceMaterialParameters[2u].yyyy+(g_VSourceMaterialParameters[2u].wwww*g_VSourceMaterialTime.xxxx))).x;
    source[6].x = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[6].y = ((g_VSourceMaterialParameters[3u].xxxx*g_VSourceMaterialTime.xxxx)).x;
    source[6].z = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[6].w = ((g_VSourceMaterialParameters[2u].zzzz+(g_VSourceMaterialParameters[3u].xxxx*g_VSourceMaterialTime.xxxx))).x;
    source[7].x = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[7].y = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[7].z = (g_VSourceMaterialParameters[4u].wwww).x;
    source[7].w = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[8].x = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[8].y = ((g_VSourceMaterialParameters[4u].xxxx*g_VSourceMaterialTime.xxxx)).x;
    source[8].z = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[8].w = ((g_VSourceMaterialParameters[4u].yyyy*g_VSourceMaterialTime.xxxx)).x;
    source[9].x = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[9].y = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[9].z = (g_VSourceMaterialParameters[1u].wwww).x;
    source[9].w = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[10].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[10].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[10].z = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[10].w = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[11].x = (g_VSourceMaterialParameters[1u].yyyy).x;
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
    // 1: add r0.x, -v2.y, l(1.000000)
    r0.x = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 2: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 3: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 4: mul r0.y, r0.y, l(1.200000)
    r0.y = ((r0.yyyy)*(float4(1.200000,1.200000,1.200000,1.200000))).y;
    // 5: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 6: mul r0.y, r0.y, l(10.000000)
    r0.y = ((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 7: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 8: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 9: mov r0.y, v2.x
    r0.y = (v2.xxxx).y;
    // 10: mad r1.x, r0.y, cb0[4].y, cb0[5].w
    r1.x = ((r0.yyyy)*(source[4].yyyy)+(source[5].wwww)).x;
    // 11: mul r0.y, v2.y, v4.y
    r0.y = ((v2.yyyy)*(v4.yyyy)).y;
    // 12: mad r1.y, r0.y, cb0[4].z, cb0[6].w
    r1.y = ((r0.yyyy)*(source[4].zzzz)+(source[6].wwww)).y;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t2.yxzw, s1, l(0.000000)
    r0.y = (VNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 14: mul r0.y, r0.y, cb0[7].x
    r0.y = ((r0.yyyy)*(source[7].xxxx)).y;
    // 15: mul r0.z, r0.x, r0.y
    r0.z = ((r0.xxxx)*(r0.yyyy)).z;
    // 16: mad_sat r0.z, -r0.z, l(0.700000), l(1.000000)
    r0.z = (saturate((-(r0.zzzz))*(float4(0.700000,0.700000,0.700000,0.700000))+(float4(1.000000,1.000000,1.000000,1.000000)))).z;
    // 17: ge r0.z, r0.z, v4.x
    r0.z = (asfloat((uint4)((r0.zzzz)>=(v4.xxxx)) * 0xffffffffu)).z;
    // 18: mad_sat r0.w, -r0.y, r0.x, l(1.000000)
    r0.w = (saturate((-(r0.yyyy))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 19: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 20: mul r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 21: ge r0.w, r0.w, v4.x
    r0.w = (asfloat((uint4)((r0.wwww)>=(v4.xxxx)) * 0xffffffffu)).w;
    // 22: movc r0.zw, r0.zzzw, l(0,0,-0.000000,0), l(0,0,-1.000000,1.000000)
    r0.zw = ((asuint(r0.zzzw) != 0u) ? (float4(asfloat(0u),asfloat(0u),-0.000000,asfloat(0u))) : (float4(asfloat(0u),asfloat(0u),-1.000000,1.000000))).zw;
    // 23: add r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)+(r0.wwww)).z;
    // 24: mul r1.xyz, v3.xyzx, cb0[3].xyzx
    r1.xyz = ((v3.xyzx)*(source[3].xyzx)).xyz;
    // 25: mul r1.xyz, r1.xyzx, l(20.000000, 20.000000, 20.000000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(20.000000,20.000000,20.000000,0.000000))).xyz;
    // 26: mad r2.xyz, r0.zzzz, r1.xyzx, r0.wwww
    r2.xyz = ((r0.zzzz)*(r1.xyzx)+(r0.wwww)).xyz;
    // 27: add r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)+(r0.wwww)).z;
    // 28: mul r0.w, r0.y, l(0.700000)
    r0.w = ((r0.yyyy)*(float4(0.700000,0.700000,0.700000,0.700000))).w;
    // 29: min r0.yw, r0.yyyw, l(0.000000, 1.000000, 0.000000, 1.000000)
    r0.yw = (min(r0.yyyw,float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 30: ge r0.yw, r0.yyyw, cb0[7].yyyy
    r0.yw = (asfloat((uint4)((r0.yyyw)>=(source[7].yyyy)) * 0xffffffffu)).yw;
    // 31: movc r0.yw, r0.yyyw, l(0,0,0,-0.000000), l(0,1.000000,0,-1.000000)
    r0.yw = ((asuint(r0.yyyw) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),-0.000000)) : (float4(asfloat(0u),1.000000,asfloat(0u),-1.000000))).yw;
    // 32: add r0.w, r0.w, r0.y
    r0.w = ((r0.wwww)+(r0.yyyy)).w;
    // 33: mad r1.xyz, r0.wwww, r1.xyzx, r0.yyyy
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r0.yyyy)).xyz;
    // 34: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 35: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v2.xyxx, t3.xyzw, s0, l(0.000000)
    r2.xyz = (VNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 37: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 38: add r3.xyz, -r2.xyzx, r0.zzzz
    r3.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 39: mad r2.xyz, cb0[4].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[4].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 40: mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 41: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 42: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 43: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 44: mad r0.zw, v2.xxxy, cb0[7].zzzw, cb0[8].yyyw
    r0.zw = ((v2.xxxy)*(source[7].zzzw)+(source[8].yyyw)).zw;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s2, l(0.000000)
    r0.zw = (VNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 46: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 47: add r1.x, v4.z, cb0[9].x
    r1.x = ((v4.zzzz)+(source[9].xxxx)).x;
    // 48: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 49: mad r0.zw, r1.xxxx, r0.zzzw, v2.xxxy
    r0.zw = ((r1.xxxx)*(r0.zzzw)+(v2.xxxy)).zw;
    // 50: mad r1.x, v4.w, cb0[9].w, cb0[10].x
    r1.x = ((v4.wwww)*(source[9].wwww)+(source[10].xxxx)).x;
    // 51: mad r1.y, v4.w, cb0[10].y, cb0[10].z
    r1.y = ((v4.wwww)*(source[10].yyyy)+(source[10].zzzz)).y;
    // 52: mad r0.zw, r0.zzzw, cb0[9].yyyz, r1.xxxy
    r0.zw = ((r0.zzzw)*(source[9].yyyz)+(r1.xxxy)).zw;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s3, l(0.000000)
    r0.z = (VNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 54: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 55: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 56: log r0.y, |r0.z|
    r0.y = (log2(abs(r0.zzzz))).y;
    // 57: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 58: mul r0.y, r0.y, cb0[10].w
    r0.y = ((r0.yyyy)*(source[10].wwww)).y;
    // 59: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 60: mul r0.y, r0.y, cb0[11].x
    r0.y = ((r0.yyyy)*(source[11].xxxx)).y;
    // 61: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 62: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 63: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

// fx_m_mi_03.fx_mi.fx_d_pa_turbulence_01_08_tr; source PS f2ecc1c1e40aa34d879ceeabe82dfc21.
float4 WRNative251(WR_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[4u];
    source[2] = g_VSourceMaterialParameters[3u];
    source[3].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_VSourceMaterialTime.xxxx).x;
    source[3].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[3].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[4].z = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[5].x = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_VSourceMaterialParameters[1u].wwww).x;
    source[5].z = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[5].w = (g_VSourceMaterialParameters[2u].zzzz).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v2.xyxx, t1.xyzw, s0, l(0.000000)
    r0.xy = (VNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 2: add r1.xyzw, -r0.xyxy, v2.xyxy
    r1.xyzw = ((-(r0.xyxy))+(v2.xyxy)).xyzw;
    // 3: mad r0.xyzw, v4.xxyy, r1.xyzw, r0.xyxy
    r0.xyzw = ((v4.xxyy)*(r1.xyzw)+(r0.xyxy)).xyzw;
    // 4: mul r1.xy, r0.xyxx, cb0[5].yyyy
    r1.xy = ((r0.xyxx)*(source[5].yyyy)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r1.x = (VNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: mul_sat r1.x, r1.x, cb0[5].z
    r1.x = (saturate((r1.xxxx)*(source[5].zzzz))).x;
    // 7: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 8: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 9: mul r1.y, r1.y, cb0[5].w
    r1.y = ((r1.yyyy)*(source[5].wwww)).y;
    // 10: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 11: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.wxyz, s3, l(0.000000)
    r0.x = (VNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).x;
    // 13: mul r0.yz, r0.zzwz, cb0[3].zzwz
    r0.yz = ((r0.zzwz)*(source[3].zzwz)).yz;
    // 14: mul_sat r0.x, r0.x, cb0[4].w
    r0.x = (saturate((r0.xxxx)*(source[4].wwww))).x;
    // 15: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 16: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 17: mul r0.w, r0.w, cb0[5].x
    r0.w = ((r0.wwww)*(source[5].xxxx)).w;
    // 18: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 19: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 20: mul r0.w, r1.x, r0.x
    r0.w = ((r1.xxxx)*(r0.xxxx)).w;
    // 21: mad r0.x, -r0.x, r1.x, r0.x
    r0.x = ((-(r0.xxxx))*(r1.xxxx)+(r0.xxxx)).x;
    // 22: mad r0.x, v4.z, r0.x, r0.w
    r0.x = ((v4.zzzz)*(r0.xxxx)+(r0.wwww)).x;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t0.yzwx, s2, l(0.000000)
    r0.w = (VNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 24: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 25: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 26: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 27: mad r0.x, cb0[3].y, cb0[3].x, r0.y
    r0.x = ((source[3].yyyy)*(source[3].xxxx)+(r0.yyyy)).x;
    // 28: mad r0.y, cb0[3].y, cb0[4].x, r0.z
    r0.y = ((source[3].yyyy)*(source[4].xxxx)+(r0.zzzz)).y;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s1, l(0.000000)
    r0.xyz = (VNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 30: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 31: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 32: mad r0.xyz, cb0[4].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[4].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 33: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 34: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 35: mul r0.xyz, r0.xyzx, cb0[4].zzzz
    r0.xyz = ((r0.xyzx)*(source[4].zzzz)).xyz;
    // 36: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 37: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 38: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 39: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 40: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_mi_r_00.fx_mi.fx_r_pa_twirl_03_03_ad; source PS 1f377575d000c741acc21359a3b57bff.
float4 WRNative252(WR_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[6u];
    source[2] = g_VSourceMaterialParameters[5u];
    source[3].x = (g_VSourceMaterialParameters[4u].wwww).x;
    source[3].y = (g_VSourceMaterialTime.xxxx).x;
    source[3].z = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[3].w = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[4].x = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[4].w = (g_VSourceMaterialParameters[1u].wwww).x;
    source[5].x = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[5].y = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[5].w = (g_VSourceMaterialParameters[2u].wwww).x;
    source[6].x = (g_VSourceMaterialParameters[3u].wwww).x;
    source[6].y = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[6].z = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[6].w = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[7].x = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[7].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[7].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[0u].yyyy)).x;
    source[7].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[0u].yyyy))).x;
    source[8].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].wwww)).x;
    source[8].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].x = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[9].y = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[9].z = ((float4(100.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].zzzz)).x;
    source[9].w = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].zzzz))).x;
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
    r2.xyz = (VNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).xyz;
    // 69: sample_l_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t1.wxyz, s2, l(0.000000)
    r1.yzw = (VNativeSample1((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).wxyz).yzw;
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

// fx_m_mi_01.fx_mi.fx_j_pa_ap_23_1_tr; source PS 24c5c5047410b4479bd6d5df1fb9d896.
float4 WRNative254(WR_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[6u];
    source[2] = VNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = VNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_VSourceMaterialTime.xxxx).x;
    source[4].z = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_VSourceMaterialParameters[0u].wwww).x;
    source[5].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[5].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[5].z = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[5].w = (clamp(g_VSourceMaterialParameters[5u].zzzz,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[6].x = ((float4(1.0, 0.0, 0.0, 0.0)-clamp(g_VSourceMaterialParameters[5u].zzzz,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[6].y = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[6].z = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[6].w = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[7].x = (g_VSourceMaterialParameters[4u].wwww).x;
    source[7].y = (g_VSourceMaterialParameters[3u].wwww).x;
    source[7].z = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[7].w = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_VSourceMaterialParameters[1u].wwww).x;
    source[8].z = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[9].x = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[9].y = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[9].z = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[9].w = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[10].x = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_VSourceMaterialParameters[2u].wwww).x;
    source[10].z = (g_VSourceMaterialParameters[3u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp4 r0.x, cb0[2].xyxy, r0.xyzw
    r0.x = (dot((source[2].xyxy).xyzw,(r0.xyzw).xyzw).xxxx).x;
    // 3: dp2 r0.y, cb0[3].xyxx, r0.zwzz
    r0.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: add r1.y, r0.y, l(0.500000)
    r1.y = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 5: add r0.y, -r1.y, l(1.000000)
    r0.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 6: mad r0.z, r0.y, cb0[5].w, cb0[6].x
    r0.z = ((r0.yyyy)*(source[5].wwww)+(source[6].xxxx)).z;
    // 7: mul_sat r0.y, r0.y, cb0[10].y
    r0.y = (saturate((r0.yyyy)*(source[10].yyyy))).y;
    // 8: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 9: div r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)/(r0.zzzz)).x;
    // 10: mad_sat r1.x, r0.x, l(0.500000), l(0.500000)
    r1.x = (saturate((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000)))).x;
    // 11: mul r0.xz, r1.xxyx, cb0[6].zzwz
    r0.xz = ((r1.xxyx)*(source[6].zzwz)).xz;
    // 12: mad r2.x, cb0[4].y, cb0[6].y, r0.x
    r2.x = ((source[4].yyyy)*(source[6].yyyy)+(r0.xxxx)).x;
    // 13: mad r2.y, cb0[4].y, cb0[7].x, r0.z
    r2.y = ((source[4].yyyy)*(source[7].xxxx)+(r0.zzzz)).y;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.xz, r2.xyxx, t0.xzyw, s0, l(0.000000)
    r0.xz = (VNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).xz;
    // 15: mad r0.xz, cb0[7].yyyy, r0.xxzx, r1.xxyx
    r0.xz = ((source[7].yyyy)*(r0.xxzx)+(r1.xxyx)).xz;
    // 16: mul r1.zw, r0.xxxz, cb0[4].zzzw
    r1.zw = ((r0.xxxz)*(source[4].zzzw)).zw;
    // 17: mul r0.xz, r0.xxzx, cb0[8].xxyx
    r0.xz = ((r0.xxzx)*(source[8].xxyx)).xz;
    // 18: mad r2.x, cb0[4].y, cb0[4].x, r1.z
    r2.x = ((source[4].yyyy)*(source[4].xxxx)+(r1.zzzz)).x;
    // 19: mad r2.y, cb0[4].y, cb0[7].z, r1.w
    r2.y = ((source[4].yyyy)*(source[7].zzzz)+(r1.wwww)).y;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (VNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 21: mad r2.x, cb0[4].y, cb0[7].w, r0.x
    r2.x = ((source[4].yyyy)*(source[7].wwww)+(r0.xxxx)).x;
    // 22: mad r2.y, cb0[4].y, cb0[8].z, r0.z
    r2.y = ((source[4].yyyy)*(source[8].zzzz)+(r0.zzzz)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (VNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 24: add r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)+(r0.wwww)).x;
    // 25: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 26: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 27: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 28: mul r0.z, r0.z, cb0[8].w
    r0.z = ((r0.zzzz)*(source[8].wwww)).z;
    // 29: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 30: mul r0.z, r0.z, cb0[9].x
    r0.z = ((r0.zzzz)*(source[9].xxxx)).z;
    // 31: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 32: log r0.z, |r1.y|
    r0.z = (log2(abs(r1.yyyy))).z;
    // 33: lt r0.w, |r1.y|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 34: mul r0.z, r0.z, cb0[9].w
    r0.z = ((r0.zzzz)*(source[9].wwww)).z;
    // 35: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 36: mul r0.z, r0.z, cb0[10].x
    r0.z = ((r0.zzzz)*(source[10].xxxx)).z;
    // 37: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 38: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r0.w, r1.x, r0.w
    r0.w = ((r1.xxxx)*(r0.wwww)).w;
    // 40: lt r1.x, r0.w, l(0.000000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 41: mul r0.w, r0.w, l(4.000000)
    r0.w = ((r0.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 42: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 43: mul r0.w, r0.w, cb0[9].y
    r0.w = ((r0.wwww)*(source[9].yyyy)).w;
    // 44: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 45: mul r0.w, r0.w, cb0[9].z
    r0.w = ((r0.wwww)*(source[9].zzzz)).w;
    // 46: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 47: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 48: mul_sat r0.x, r0.z, r0.x
    r0.x = (saturate((r0.zzzz)*(r0.xxxx))).x;
    // 49: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 50: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 51: mul r0.z, r0.z, cb0[10].z
    r0.z = ((r0.zzzz)*(source[10].zzzz)).z;
    // 52: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 53: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 54: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 55: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 56: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 57: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 58: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// bfx_m_mi_00.bfx_mi.bfx_d_pa_sinewave_01_04_tr; source PS a05548ae069d094885b533ae48707c21.
float4 WRNative255(WR_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[5u];
    source[2] = VNativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = VNativeAppend(sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4] = VNativeAppend(VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].wwww)*float4(0.0, 0.0, 0.0, 0.0))),VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].wwww)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[5] = VNativeAppend(g_VSourceMaterialParameters[0u].zzzz,g_VSourceMaterialParameters[1u].yyyy,1u);
    source[6] = g_VSourceMaterialParameters[4u];
    source[7].x = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[7].y = (g_VSourceMaterialTime.xxxx).x;
    source[7].z = (g_VSourceMaterialParameters[0u].wwww).x;
    source[7].w = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[8].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[8].y = (cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[8].z = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[8].w = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[9].x = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[9].y = (VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].wwww)*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[9].z = (VNativePeriodic(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].wwww)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[10].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[10].y = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[10].z = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[10].w = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[11].x = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[11].y = (g_VSourceMaterialParameters[2u].wwww).x;
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
    // 1: mul r0.x, v4.x, cb0[8].w
    r0.x = ((v4.xxxx)*(source[8].wwww)).x;
    // 2: add r1.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r1.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 3: dp2 r0.y, cb0[3].xyxx, r1.zwzz
    r0.y = (dot((source[3].xyxx).xy,(r1.zwzz).xy).xxxx).y;
    // 4: dp4 r0.z, cb0[2].xyxy, r1.xyzw
    r0.z = (dot((source[2].xyxy).xyzw,(r1.xyzw).xyzw).xxxx).z;
    // 5: add r1.y, r0.y, l(0.500000)
    r1.y = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 6: add r1.zw, r1.yyyy, v4.yyyz
    r1.zw = ((r1.yyyy)+(v4.yyyz)).zw;
    // 7: mul r0.y, r1.z, cb0[8].z
    r0.y = ((r1.zzzz)*(source[8].zzzz)).y;
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
    // 18: mad_sat r1.x, r0.x, cb0[9].x, l(0.500000)
    r1.x = (saturate((r0.xxxx)*(source[9].xxxx)+(float4(0.500000,0.500000,0.500000,0.500000)))).x;
    // 19: add r0.xy, r1.xwxx, l(0.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r1.xwxx)+(float4(0.000000,-1.000000,0.000000,0.000000))).xy;
    // 20: add r1.xy, r1.xyxx, cb0[4].xyxx
    r1.xy = ((r1.xyxx)+(source[4].xyxx)).xy;
    // 21: mul r1.xy, r1.xyxx, cb0[5].xyxx
    r1.xy = ((r1.xyxx)*(source[5].xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (VNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (VNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 24: mul r1.xy, r1.xyxx, cb0[7].zwzz
    r1.xy = ((r1.xyxx)*(source[7].zwzz)).xy;
    // 25: add r0.z, -v3.w, l(1.000000)
    r0.z = ((-(v3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 26: add_sat r0.y, -r0.z, r0.y
    r0.y = (saturate((-(r0.zzzz))+(r0.yyyy))).y;
    // 27: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 28: mul_sat r0.x, r0.x, cb0[11].x
    r0.x = (saturate((r0.xxxx)*(source[11].xxxx))).x;
    // 29: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 30: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 31: mul r0.y, r0.y, cb0[11].y
    r0.y = ((r0.yyyy)*(source[11].yyyy)).y;
    // 32: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 33: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 34: mul_sat r0.x, r0.w, r0.x
    r0.x = (saturate((r0.wwww)*(r0.xxxx))).x;
    // 35: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 36: mad r0.x, cb0[7].y, cb0[7].x, r1.x
    r0.x = ((source[7].yyyy)*(source[7].xxxx)+(r1.xxxx)).x;
    // 37: mad r0.y, cb0[7].y, cb0[10].y, r1.y
    r0.y = ((source[7].yyyy)*(source[10].yyyy)+(r1.yyyy)).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s0, l(0.000000)
    r0.xyz = (VNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 39: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 41: mad r0.xyz, cb0[10].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[10].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 42: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 43: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 44: mul r0.xyz, r0.xyzx, cb0[10].wwww
    r0.xyz = ((r0.xyzx)*(source[10].wwww)).xyz;
    // 45: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 46: mul r1.xyz, cb0[6].xyzx, cb0[6].wwww
    r1.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 47: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 48: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 49: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_mi_m_00.fx_mi.fx_m_pa_spritewave_01_7_ad; source PS a91210cfa2eb954eae0fc5675ac88ced.
float4 WRNative256(WR_NATIVE_INPUT input)
{
    float4 source[24]; [unroll] for (uint i=0u; i<24u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[12u];
    source[2] = VNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = VNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = VNativeAppend(g_VSourceMaterialParameters[10u].yyyy,g_VSourceMaterialParameters[10u].zzzz,1u);
    source[5] = VNativeAppend(g_VSourceMaterialParameters[4u].xxxx,g_VSourceMaterialParameters[4u].yyyy,1u);
    source[6] = VNativeAppend(cos(((g_VSourceMaterialParameters[5u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_VSourceMaterialParameters[5u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = VNativeAppend(sin(((g_VSourceMaterialParameters[5u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_VSourceMaterialParameters[5u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = VNativeAppend(cos((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = VNativeAppend(sin((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10] = g_VSourceMaterialParameters[11u];
    source[11].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_VSourceMaterialParameters[5u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[11].y = (cos(((g_VSourceMaterialParameters[5u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[11].z = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[11].w = (g_VSourceMaterialTime.xxxx).x;
    source[12].x = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[12].y = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[12].z = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[12].w = (g_VSourceMaterialParameters[3u].wwww).x;
    source[13].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[13].y = (g_VSourceMaterialParameters[4u].wwww).x;
    source[13].z = (g_VSourceMaterialParameters[8u].wwww).x;
    source[13].w = (g_VSourceMaterialParameters[9u].wwww).x;
    source[14].x = (g_VSourceMaterialParameters[10u].xxxx).x;
    source[14].y = (g_VSourceMaterialParameters[8u].yyyy).x;
    source[14].z = (g_VSourceMaterialParameters[9u].yyyy).x;
    source[14].w = (g_VSourceMaterialParameters[9u].zzzz).x;
    source[15].x = (g_VSourceMaterialParameters[8u].zzzz).x;
    source[15].y = (g_VSourceMaterialParameters[7u].wwww).x;
    source[15].z = (g_VSourceMaterialParameters[9u].xxxx).x;
    source[15].w = (g_VSourceMaterialParameters[10u].zzzz).x;
    source[16].x = (g_VSourceMaterialParameters[10u].yyyy).x;
    source[16].y = (g_VSourceMaterialParameters[8u].xxxx).x;
    source[16].z = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[16].w = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[17].x = (g_VSourceMaterialParameters[2u].wwww).x;
    source[17].y = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[17].z = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[17].w = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[18].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[18].y = (g_VSourceMaterialParameters[0u].wwww).x;
    source[18].z = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[18].w = ((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[19].x = (sin((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[19].z = (cos((g_VSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].w = (g_VSourceMaterialParameters[6u].xxxx).x;
    source[20].x = (g_VSourceMaterialParameters[6u].zzzz).x;
    source[20].y = (g_VSourceMaterialParameters[6u].wwww).x;
    source[20].z = (g_VSourceMaterialParameters[6u].yyyy).x;
    source[20].w = (g_VSourceMaterialParameters[5u].wwww).x;
    source[21].x = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[21].y = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[21].z = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[21].w = (g_VSourceMaterialParameters[1u].wwww).x;
    source[22].x = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[22].y = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[22].z = (g_VSourceMaterialParameters[7u].xxxx).x;
    source[22].w = (g_VSourceMaterialParameters[7u].yyyy).x;
    source[23].x = (g_VSourceMaterialParameters[7u].zzzz).x;
    source[23].y = (g_VSourceMaterialParameters[3u].yyyy).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[14].zwzz
    r0.xy = ((v2.xyxx)*(source[14].zwzz)).xy;
    // 2: mad r1.x, cb0[11].w, cb0[14].y, r0.x
    r1.x = ((source[11].wwww)*(source[14].yyyy)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[11].w, cb0[15].x, r0.y
    r1.y = ((source[11].wwww)*(source[15].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (VNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: mad r0.xy, r0.xxxx, cb0[15].yyyy, v2.xyxx
    r0.xy = ((r0.xxxx)*(source[15].yyyy)+(v2.xyxx)).xy;
    // 6: mul r0.z, cb0[11].w, cb0[13].z
    r0.z = ((source[11].wwww)*(source[13].zzzz)).z;
    // 7: mad r1.x, cb0[13].w, r0.x, r0.z
    r1.x = ((source[13].wwww)*(r0.xxxx)+(r0.zzzz)).x;
    // 8: mul r0.x, cb0[11].w, cb0[15].z
    r0.x = ((source[11].wwww)*(source[15].zzzz)).x;
    // 9: mad r1.y, cb0[14].x, r0.y, r0.x
    r1.y = ((source[14].xxxx)*(r0.yyyy)+(r0.xxxx)).y;
    // 10: add r0.xy, r1.xyxx, cb0[4].xyxx
    r0.xy = ((r1.xyxx)+(source[4].xyxx)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (VNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 12: add r0.y, v4.z, cb0[16].y
    r0.y = ((v4.zzzz)+(source[16].yyyy)).y;
    // 13: mad r0.xy, r0.xxxx, r0.yyyy, cb0[5].xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(source[5].xyxx)).xy;
    // 14: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 15: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 16: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 17: mad r1.xy, cb0[12].zwzz, v4.xxxx, r1.xyxx
    r1.xy = ((source[12].zwzz)*(v4.xxxx)+(r1.xyxx)).xy;
    // 18: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 19: mul r1.xy, r1.xyxx, cb0[12].xyxx
    r1.xy = ((r1.xyxx)*(source[12].xyxx)).xy;
    // 20: mad r2.x, cb0[11].w, cb0[11].z, r1.x
    r2.x = ((source[11].wwww)*(source[11].zzzz)+(r1.xxxx)).x;
    // 21: mad r2.y, cb0[11].w, cb0[13].y, r1.y
    r2.y = ((source[11].wwww)*(source[13].yyyy)+(r1.yyyy)).y;
    // 22: add r0.xy, r0.xyxx, r2.xyxx
    r0.xy = ((r0.xyxx)+(r2.xyxx)).xy;
    // 23: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 24: dp2 r1.x, cb0[6].xyxx, r0.xyxx
    r1.x = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 25: dp2 r1.y, cb0[7].xyxx, r0.xyxx
    r1.y = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 26: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 27: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s0, l(-1.000000)
    r0.x = (VNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 28: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 29: mul r0.y, r0.y, cb0[17].x
    r0.y = ((r0.yyyy)*(source[17].xxxx)).y;
    // 30: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 31: mul r0.y, r0.y, cb0[17].y
    r0.y = ((r0.yyyy)*(source[17].yyyy)).y;
    // 32: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 33: movc r0.y, r1.x, l(0), r0.y
    r0.y = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 34: mad r0.y, r0.x, cb0[17].z, r0.y
    r0.y = ((r0.xxxx)*(source[17].zzzz)+(r0.yyyy)).y;
    // 35: dp2 r1.x, cb0[8].xyxx, r0.zwzz
    r1.x = (dot((source[8].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 36: dp2 r1.y, cb0[9].xyxx, r0.zwzz
    r1.y = (dot((source[9].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 37: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 38: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 39: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 40: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 41: mul r0.z, r0.z, cb0[22].z
    r0.z = ((r0.zzzz)*(source[22].zzzz)).z;
    // 42: max r0.z, r0.z, cb0[23].x
    r0.z = (max(r0.zzzz,source[23].xxxx)).z;
    // 43: min r0.z, r0.z, cb0[22].w
    r0.z = (min(r0.zzzz,source[22].wwww)).z;
    // 44: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 45: mul r1.zw, v2.xxxy, cb0[20].xxxy
    r1.zw = ((v2.xxxy)*(source[20].xxxy)).zw;
    // 46: mad r2.x, cb0[11].w, cb0[19].w, r1.z
    r2.x = ((source[11].wwww)*(source[19].wwww)+(r1.zzzz)).x;
    // 47: mad r2.y, cb0[11].w, cb0[20].z, r1.w
    r2.y = ((source[11].wwww)*(source[20].zzzz)+(r1.wwww)).y;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t3.yzwx, s3, l(0.000000)
    r0.w = (VNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 49: mad r1.xy, r0.wwww, cb0[20].wwww, r1.xyxx
    r1.xy = ((r0.wwww)*(source[20].wwww)+(r1.xyxx)).xy;
    // 50: mul r1.xy, r1.xyxx, cb0[18].xyxx
    r1.xy = ((r1.xyxx)*(source[18].xyxx)).xy;
    // 51: mad r2.x, cb0[11].w, cb0[17].w, r1.x
    r2.x = ((source[11].wwww)*(source[17].wwww)+(r1.xxxx)).x;
    // 52: mad r2.y, cb0[11].w, cb0[21].x, r1.y
    r2.y = ((source[11].wwww)*(source[21].xxxx)+(r1.yyyy)).y;
    // 53: add r1.xy, r2.xyxx, cb0[21].yzyy
    r1.xy = ((r2.xyxx)+(source[21].yzyy)).xy;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t4.yzwx, s4, l(0.000000)
    r0.w = (VNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 55: add r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 56: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 57: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 58: mul r1.x, r1.x, cb0[21].w
    r1.x = ((r1.xxxx)*(source[21].wwww)).x;
    // 59: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 60: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 61: add r1.x, v4.y, l(-1.000000)
    r1.x = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 62: add_sat r0.w, r0.w, -r1.x
    r0.w = (saturate((r0.wwww)+(-(r1.xxxx)))).w;
    // 63: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 64: mul r1.x, r1.x, cb0[22].y
    r1.x = ((r1.xxxx)*(source[22].yyyy)).x;
    // 65: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 66: mul_sat r1.x, r1.x, cb0[22].x
    r1.x = (saturate((r1.xxxx)*(source[22].xxxx))).x;
    // 67: lt r1.y, r0.w, l(0.000001)
    r1.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 68: mul r0.w, r0.w, cb0[22].x
    r0.w = ((r0.wwww)*(source[22].xxxx)).w;
    // 69: movc r1.x, r1.y, l(-0.000000), -r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r1.xxxx))).x;
    // 70: mov_sat r1.y, r0.w
    r1.y = (saturate(r0.wwww)).y;
    // 71: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 72: mul r0.x, r0.x, cb0[23].y
    r0.x = ((r0.xxxx)*(source[23].yyyy)).x;
    // 73: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 74: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 75: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 76: add r0.z, r1.x, r1.y
    r0.z = ((r1.xxxx)+(r1.yyyy)).z;
    // 77: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 78: mad r0.yzw, r0.zzzz, cb0[10].xxyz, r0.yyyy
    r0.yzw = ((r0.zzzz)*(source[10].xxyz)+(r0.yyyy)).yzw;
    // 79: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 80: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 81: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 82: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_mi_05.fx_m.fx_c_pa_glitter_01_tr; source PS bd8398a6efa91243bb7de7f4bed27970.
float4 WRNative257(WR_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[1u];
    source[2].x = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_VSourceMaterialTime.xxxx).x;
    source[2].z = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[2].w = ((g_VSourceMaterialParameters[0u].zzzz*float4(3.5, 0.0, 0.0, 0.0))).x;
    source[3].x = (g_VSourceMaterialParameters[0u].xxxx).x;
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
    // 1: mul r0.xyzw, v2.xyxy, l(0.500000, 0.500000, 0.700000, 0.700000)
    r0.xyzw = ((v2.xyxy)*(float4(0.500000,0.500000,0.700000,0.700000))).xyzw;
    // 2: mad r0.xy, v4.wwww, l(0.100000, -0.100000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v4.wwww)*(float4(0.100000,-0.100000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (VNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.yz, r0.xxxx, l(0.000000, 0.400000, 0.400000, 0.000000), r0.zzwz
    r0.yz = ((r0.xxxx)*(float4(0.000000,0.400000,0.400000,0.000000))+(r0.zzwz)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yzyy, t1.yzwx, s4, l(0.000000)
    r0.w = (VNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t4.yxzw, s1, l(0.000000)
    r0.y = (VNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: add r0.z, r0.w, r0.w
    r0.z = ((r0.wwww)+(r0.wwww)).z;
    // 8: mul r0.w, r0.x, l(0.200000)
    r0.w = ((r0.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 9: add r1.xy, r0.xxxx, v2.xyxx
    r1.xy = ((r0.xxxx)+(v2.xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t5.xyzw, s2, l(0.000000)
    r0.x = (VNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: mad r1.xy, v2.xyxx, l(0.950000, 0.950000, 0.000000, 0.000000), r0.wwww
    r1.xy = ((v2.xyxx)*(float4(0.950000,0.950000,0.000000,0.000000))+(r0.wwww)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s5, l(0.000000)
    r0.w = (VNativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 13: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 14: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 15: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 16: mul r1.x, r0.z, l(5.000000)
    r1.x = ((r0.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 17: mad r0.w, r0.w, l(150.000000), r1.x
    r0.w = ((r0.wwww)*(float4(150.000000,150.000000,150.000000,150.000000))+(r1.xxxx)).w;
    // 18: mul r1.xyzw, v2.xyxy, cb0[2].zzww
    r1.xyzw = ((v2.xyxy)*(source[2].zzww)).xyzw;
    // 19: mad r1.xy, cb0[2].yyyy, cb0[2].xxxx, r1.xyxx
    r1.xy = ((source[2].yyyy)*(source[2].xxxx)+(r1.xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t3.yzxw, s3, l(0.000000)
    r1.z = (VNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r1.x = (VNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 22: mul r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)*(r1.xxxx)).x;
    // 23: mul r1.x, r1.x, cb0[3].x
    r1.x = ((r1.xxxx)*(source[3].xxxx)).x;
    // 24: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 25: mul r1.x, r0.x, r1.x
    r1.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 26: mad r0.y, r1.x, l(30.000000), r0.y
    r0.y = ((r1.xxxx)*(float4(30.000000,30.000000,30.000000,30.000000))+(r0.yyyy)).y;
    // 27: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 28: mad r1.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 29: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 30: mul r0.x, r0.w, r0.z
    r0.x = ((r0.wwww)*(r0.zzzz)).x;
    // 31: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 32: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

// fx_m_mi_w_00.mi.fx_w_me_master_02_243_dt_tr; source PS fd7e6ab0d8148c40b19db546d0fe0948.
float4 WRNative258(WR_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_VSourceMaterialParameters[14u];
    source[3] = VNativeAppend(g_VSourceMaterialParameters[2u].zzzz,g_VSourceMaterialParameters[3u].xxxx,1u);
    source[4] = g_VSourceMaterialParameters[11u];
    source[5] = g_VSourceMaterialParameters[12u];
    source[6] = input.dynamicParameter;
    source[7] = VNativeAppend(g_VSourceMaterialParameters[6u].wwww,g_VSourceMaterialParameters[7u].xxxx,1u);
    source[8] = g_VSourceMaterialParameters[10u];
    source[9].x = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[9].y = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[9].z = (g_VSourceMaterialParameters[9u].yyyy).x;
    source[9].w = (g_VSourceMaterialTime.xxxx).x;
    source[10].x = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[10].y = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[10].z = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[10].w = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[11].x = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[11].y = (g_VSourceMaterialParameters[3u].wwww).x;
    source[11].z = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[11].w = (g_VSourceMaterialParameters[1u].wwww).x;
    source[12].x = ((float4(1.0, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[1u].wwww)).x;
    source[12].y = (g_VSourceMaterialParameters[2u].wwww).x;
    source[12].z = (g_VSourceMaterialParameters[6u].yyyy).x;
    source[12].w = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[13].x = (g_VSourceMaterialParameters[5u].wwww).x;
    source[13].y = (g_VSourceMaterialParameters[6u].zzzz).x;
    source[13].z = (g_VSourceMaterialParameters[8u].wwww).x;
    source[13].w = (g_VSourceMaterialParameters[9u].xxxx).x;
    source[14].x = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[14].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[14].z = (g_VSourceMaterialParameters[0u].wwww).x;
    source[14].w = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[15].x = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[15].y = (g_VSourceMaterialParameters[4u].wwww).x;
    source[15].z = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[15].w = (g_VSourceMaterialParameters[6u].xxxx).x;
    source[16].x = (g_VSourceMaterialParameters[7u].yyyy).x;
    source[16].y = (g_VSourceMaterialParameters[7u].zzzz).x;
    source[16].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[7u].yyyy)).x;
    source[16].w = (g_VSourceMaterialParameters[7u].xxxx).x;
    source[17].x = (g_VSourceMaterialParameters[6u].wwww).x;
    source[17].y = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[17].z = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[17].w = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[18].x = (g_VSourceMaterialParameters[8u].yyyy).x;
    source[18].y = (g_VSourceMaterialParameters[8u].zzzz).x;
    source[18].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[18].w = (g_VSourceMaterialParameters[7u].wwww).x;
    source[19].x = (g_VSourceMaterialParameters[8u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[11].xyxx
    r0.xy = ((v4.xyxx)*(source[11].xyxx)).xy;
    // 2: mul r0.z, cb0[9].z, cb0[9].w
    r0.z = ((source[9].zzzz)*(source[9].wwww)).z;
    // 3: mad r1.x, r0.z, cb0[10].w, r0.x
    r1.x = ((r0.zzzz)*(source[10].wwww)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[11].z, r0.y
    r1.y = ((r0.zzzz)*(source[11].zzzz)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (VNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[12].xxxx, r0.xyxx, v4.xyxx
    r0.xy = ((source[12].xxxx)*(r0.xyxx)+(v4.xyxx)).xy;
    // 7: mad r1.xy, v4.yxyy, cb0[4].xyxx, r0.xyxx
    r1.xy = ((v4.yxyy)*(source[4].xyxx)+(r0.xyxx)).xy;
    // 8: mul r0.w, r0.z, cb0[12].z
    r0.w = ((r0.zzzz)*(source[12].zzzz)).w;
    // 9: mad r2.x, cb0[12].w, r1.x, r0.w
    r2.x = ((source[12].wwww)*(r1.xxxx)+(r0.wwww)).x;
    // 10: mul r0.w, r1.y, cb0[13].x
    r0.w = ((r1.yyyy)*(source[13].xxxx)).w;
    // 11: mul r1.xy, r1.xyxx, cb0[10].yzyy
    r1.xy = ((r1.xyxx)*(source[10].yzyy)).xy;
    // 12: mad r2.y, r0.z, cb0[13].y, r0.w
    r2.y = ((r0.zzzz)*(source[13].yyyy)+(r0.wwww)).y;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t6.xyzw, s3, l(0.000000)
    r2.xyz = (VNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 14: mad r3.x, r0.z, cb0[10].x, r1.x
    r3.x = ((r0.zzzz)*(source[10].xxxx)+(r1.xxxx)).x;
    // 15: mad r3.y, r0.z, cb0[12].y, r1.y
    r3.y = ((r0.zzzz)*(source[12].yyyy)+(r1.yyyy)).y;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r3.xyxx, t5.xyzw, s2, l(0.000000)
    r1.xyz = (VNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 17: add r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)+(r1.xyzx)).xyz;
    // 18: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 19: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 20: mad r1.xyz, cb0[13].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[13].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 21: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 22: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 23: mul r1.xyz, r1.xyzx, cb0[13].wwww
    r1.xyz = ((r1.xyzx)*(source[13].wwww)).xyz;
    // 24: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 25: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 26: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 27: mul r2.xy, r0.xyxx, cb0[15].yzyy
    r2.xy = ((r0.xyxx)*(source[15].yzyy)).xy;
    // 28: mad r2.xy, r0.zzzz, cb0[15].xwxx, r2.xyxx
    r2.xy = ((r0.zzzz)*(source[15].xwxx)+(r2.xyxx)).xy;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t2.yzwx, s5, l(0.000000)
    r0.w = (VNativeSample4((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 30: add r2.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 31: mul r0.xy, r0.xyxx, cb0[14].yzyy
    r0.xy = ((r0.xyxx)*(source[14].yzyy)).xy;
    // 32: mad r0.xy, r0.zzzz, cb0[14].xwxx, r0.xyxx
    r0.xy = ((r0.zzzz)*(source[14].xwxx)+(r0.xyxx)).xy;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s4, l(0.000000)
    r0.x = (VNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 34: mad r0.y, cb0[9].w, cb0[16].y, cb0[16].z
    r0.y = ((source[9].wwww)*(source[16].yyyy)+(source[16].zzzz)).y;
    // 35: sincos r3.x, r4.x, r0.y
    r3.x = (sin(r0.yyyy)).x; r4.x = (cos(r0.yyyy)).x;
    // 36: mov r5.x, -r3.x
    r5.x = (-(r3.xxxx)).x;
    // 37: mov r5.y, r4.x
    r5.y = (r4.xxxx).y;
    // 38: mov r5.z, r3.x
    r5.z = (r3.xxxx).z;
    // 39: dp2 r0.y, r5.zyzz, r2.xyxx
    r0.y = (dot((r5.zyzz).xy,(r2.xyxx).xy).xxxx).y;
    // 40: dp2 r0.z, r5.yxyy, r2.xyxx
    r0.z = (dot((r5.yxyy).xy,(r2.xyxx).xy).xxxx).z;
    // 41: mul r2.z, r0.y, cb0[7].y
    r2.z = ((r0.yyyy)*(source[7].yyyy)).z;
    // 42: add r0.y, cb0[6].y, l(-1.000000)
    r0.y = ((source[6].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 43: mad r2.x, r0.z, cb0[7].x, r0.y
    r2.x = ((r0.zzzz)*(source[7].xxxx)+(r0.yyyy)).x;
    // 44: add r0.yz, r2.xxzx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxzx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t3.yxzw, s6, l(0.000000)
    r0.y = (VNativeSample5((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 46: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 47: mad r0.zw, r0.xxxx, r0.yyyy, r1.xxxy
    r0.zw = ((r0.xxxx)*(r0.yyyy)+(r1.xxxy)).zw;
    // 48: mul r0.zw, r0.zzzw, cb0[17].yyyy
    r0.zw = ((r0.zzzw)*(source[17].yyyy)).zw;
    // 49: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 50: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 51: mul r2.xyz, r1.wwww, v6.xyzx
    r2.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 52: mad r0.zw, r2.xxxy, cb0[3].xxxy, r0.zzzw
    r0.zw = ((r2.xxxy)*(source[3].xxxy)+(r0.zzzw)).zw;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r2.xyw, r0.zwzz, t7.xywz, s7, l(0.000000)
    r2.xyw = (VNativeSample6((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 54: dp3 r0.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 55: add r3.xyz, -r2.xywx, r0.zzzz
    r3.xyz = ((-(r2.xywx))+(r0.zzzz)).xyz;
    // 56: mad r2.xyw, cb0[17].zzzz, r3.xyxz, r2.xyxw
    r2.xyw = ((source[17].zzzz)*(r3.xyxz)+(r2.xyxw)).xyw;
    // 57: max r2.xyw, |r2.xyxw|, l(0.000001, 0.000001, 0.000000, 0.000001)
    r2.xyw = (max(abs(r2.xyxw),float4(0.000001,0.000001,0.000000,0.000001))).xyw;
    // 58: log r2.xyw, r2.xyxw
    r2.xyw = (log2(r2.xyxw)).xyw;
    // 59: mul r2.xyw, r2.xyxw, cb0[17].wwww
    r2.xyw = ((r2.xyxw)*(source[17].wwww)).xyw;
    // 60: exp r2.xyw, r2.xyxw
    r2.xyw = (exp2(r2.xyxw)).xyw;
    // 61: mul r3.xyz, cb0[8].xyzx, cb0[8].wwww
    r3.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 62: mad r1.xyz, r2.xywx, r3.xyzx, r1.xyzx
    r1.xyz = ((r2.xywx)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 63: mad r1.xyz, r1.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 64: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 65: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 66: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 67: source device depth mapped to centimetre view depth; reconstruction at 69.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 69-72: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 73: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 74: add r0.w, -cb0[18].z, l(1.000000)
    r0.w = ((-(source[18].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 75: max r0.w, -r0.w, l(0.001000)
    r0.w = (max(-(r0.wwww),float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 76: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 77: log r0.w, |r2.z|
    r0.w = (log2(abs(r2.zzzz))).w;
    // 78: lt r1.x, |r2.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 79: mul r0.w, r0.w, cb0[18].w
    r0.w = ((r0.wwww)*(source[18].wwww)).w;
    // 80: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 81: mul_sat r0.w, r0.w, cb0[19].x
    r0.w = (saturate((r0.wwww)*(source[19].xxxx))).w;
    // 82: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 83: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 84: add r0.w, -cb0[6].x, l(1.000000)
    r0.w = ((-(source[6].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mad r0.x, r0.x, r0.y, -r0.w
    r0.x = ((r0.xxxx)*(r0.yyyy)+(-(r0.wwww))).x;
    // 86: mul_sat r0.x, r0.x, cb0[18].x
    r0.x = (saturate((r0.xxxx)*(source[18].xxxx))).x;
    // 87: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 88: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 89: mul r0.y, r0.y, cb0[18].y
    r0.y = ((r0.yyyy)*(source[18].yyyy)).y;
    // 90: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 91: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 92: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 93: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 94: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}

// fx_m_mi_w_00.mi.fx_w_pa_master_01_05_dt_tr; source PS dcf7eced1d7dab4b8c9e4cf5924e53e5.
float4 WRNative259(WR_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[7u];
    source[2] = g_VSourceMaterialParameters[6u];
    source[3] = VNativeAppend(g_VSourceMaterialParameters[3u].zzzz,g_VSourceMaterialParameters[3u].wwww,1u);
    source[4].x = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[4].y = (g_VSourceMaterialTime.xxxx).x;
    source[4].z = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[4].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[5].w = (g_VSourceMaterialParameters[1u].wwww).x;
    source[6].x = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[6].y = (g_VSourceMaterialParameters[0u].wwww).x;
    source[6].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[0u].wwww)).x;
    source[6].w = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[7].x = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_VSourceMaterialParameters[2u].wwww).x;
    source[7].w = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[8].x = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[8].y = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[8].z = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[8].w = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[9].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[4u].xxxx)).x;
    source[9].y = (g_VSourceMaterialParameters[3u].wwww).x;
    source[9].z = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[9].w = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[10].x = (g_VSourceMaterialParameters[4u].wwww).x;
    source[10].y = (g_VSourceMaterialParameters[0u].xxxx).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[5].zwzz
    r0.xy = ((v2.xyxx)*(source[5].zwzz)).xy;
    // 2: mul r0.z, cb0[4].x, cb0[4].y
    r0.z = ((source[4].xxxx)*(source[4].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[5].y, r0.x
    r1.x = ((r0.zzzz)*(source[5].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[6].x, r0.y
    r1.y = ((r0.zzzz)*(source[6].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (VNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[6].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[6].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 7: mul r0.w, r0.x, cb0[4].w
    r0.w = ((r0.xxxx)*(source[4].wwww)).w;
    // 8: mad r1.x, r0.z, cb0[4].z, r0.w
    r1.x = ((r0.zzzz)*(source[4].zzzz)+(r0.wwww)).x;
    // 9: mul r0.w, r0.z, cb0[6].w
    r0.w = ((r0.zzzz)*(source[6].wwww)).w;
    // 10: mad r1.y, cb0[5].x, r0.y, r0.w
    r1.y = ((source[5].xxxx)*(r0.yyyy)+(r0.wwww)).y;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s2, l(0.000000)
    r1.xyz = (VNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 12: mul r2.xy, r0.xyxx, cb0[7].yzyy
    r2.xy = ((r0.xyxx)*(source[7].yzyy)).xy;
    // 13: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 14: mad r0.zw, r0.zzzz, cb0[7].xxxw, r2.xxxy
    r0.zw = ((r0.zzzz)*(source[7].xxxw)+(r2.xxxy)).zw;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t4.xyzw, s3, l(0.000000)
    r2.xyz = (VNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 16: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 17: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 18: mad r1.xyz, -r1.xyzx, r2.xyzx, r0.zzzz
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r0.zzzz)).xyz;
    // 19: mad r1.xyz, cb0[8].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[8].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 20: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 21: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 22: mul r1.xyz, r1.xyzx, cb0[8].yyyy
    r1.xyz = ((r1.xyzx)*(source[8].yyyy)).xyz;
    // 23: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 24: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 25: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 26: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 27: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 28: mad r0.z, cb0[4].y, cb0[8].w, cb0[9].x
    r0.z = ((source[4].yyyy)*(source[8].wwww)+(source[9].xxxx)).z;
    // 29: sincos r1.x, r2.x, r0.z
    r1.x = (sin(r0.zzzz)).x; r2.x = (cos(r0.zzzz)).x;
    // 30: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 31: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 32: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 33: dp2 r0.z, r3.zyzz, r0.xyxx
    r0.z = (dot((r3.zyzz).xy,(r0.xyxx).xy).xxxx).z;
    // 34: dp2 r0.x, r3.yxyy, r0.xyxx
    r0.x = (dot((r3.yxyy).xy,(r0.xyxx).xy).xxxx).x;
    // 35: mul r1.z, r0.z, cb0[3].y
    r1.z = ((r0.zzzz)*(source[3].yyyy)).z;
    // 36: add r0.y, v4.y, l(-1.000000)
    r0.y = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 37: mad r1.x, r0.x, cb0[3].x, r0.y
    r1.x = ((r0.xxxx)*(source[3].xxxx)+(r0.yyyy)).x;
    // 38: add r0.xy, r1.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s4, l(0.000000)
    r0.x = (VNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 40: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 41: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 42: mul_sat r0.x, r0.x, cb0[9].w
    r0.x = (saturate((r0.xxxx)*(source[9].wwww))).x;
    // 43: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 44: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 45: mul r0.x, r0.x, cb0[10].x
    r0.x = ((r0.xxxx)*(source[10].xxxx)).x;
    // 46: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 47: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 48: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 49: source device depth mapped to centimetre view depth; reconstruction at 51.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 51-54: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 55: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 56: add r0.w, -cb0[10].y, l(1.000000)
    r0.w = ((-(source[10].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 57: max r0.w, -r0.w, l(0.001000)
    r0.w = (max(-(r0.wwww),float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 58: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 59: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 60: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 61: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 62: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}

// fx_m_mi_w_00.mi.fx_w_pa_spritewave_01_05_tr; source PS 39f7e63594b10f4a9237dc9eb19a1dfc.
float4 WRNative260(WR_NATIVE_INPUT input)
{
    float4 source[24]; [unroll] for (uint i=0u; i<24u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_VSourceMaterialParameters[12u];
    source[3] = input.dynamicParameter;
    source[4] = VNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = VNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = VNativeAppend(g_VSourceMaterialParameters[8u].wwww,g_VSourceMaterialParameters[9u].xxxx,1u);
    source[7] = VNativeAppend(g_VSourceMaterialParameters[3u].wwww,g_VSourceMaterialParameters[4u].xxxx,1u);
    source[8] = VNativeAppend(cos(((g_VSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_VSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = VNativeAppend(sin(((g_VSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_VSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    // Original CPU uniform sin/cos; packet row13 is prepared from the editable dissolve rotator.
    source[10] = float4(g_VSourceMaterialParameters[13u].x, -g_VSourceMaterialParameters[13u].y, 0.f, 0.f);
    source[11] = float4(g_VSourceMaterialParameters[13u].y, g_VSourceMaterialParameters[13u].x, 1.f, 1.f);
    source[12] = g_VSourceMaterialParameters[10u];
    source[13].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_VSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[13].y = (cos(((g_VSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[13].z = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[13].w = (g_VSourceMaterialTime.xxxx).x;
    source[14].x = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[14].y = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[14].z = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[14].w = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[15].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[15].y = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[15].z = (g_VSourceMaterialParameters[7u].wwww).x;
    source[15].w = (g_VSourceMaterialParameters[8u].yyyy).x;
    source[16].x = (g_VSourceMaterialParameters[8u].zzzz).x;
    source[16].y = (g_VSourceMaterialParameters[8u].xxxx).x;
    source[16].z = (g_VSourceMaterialParameters[9u].xxxx).x;
    source[16].w = (g_VSourceMaterialParameters[8u].wwww).x;
    source[17].x = (g_VSourceMaterialParameters[7u].zzzz).x;
    source[17].y = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[17].z = (g_VSourceMaterialParameters[3u].wwww).x;
    source[17].w = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[18].x = (g_VSourceMaterialParameters[2u].wwww).x;
    source[18].y = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[18].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[18].w = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[19].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[19].y = (g_VSourceMaterialParameters[1u].wwww).x;
    source[19].z = ((g_VSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[19].w = g_VSourceMaterialParameters[13u].y;
    source[20].x = -g_VSourceMaterialParameters[13u].y;
    source[20].y = g_VSourceMaterialParameters[13u].x;
    source[20].z = (g_VSourceMaterialParameters[5u].wwww).x;
    source[20].w = (g_VSourceMaterialParameters[6u].yyyy).x;
    source[21].x = (g_VSourceMaterialParameters[6u].zzzz).x;
    source[21].y = (g_VSourceMaterialParameters[6u].xxxx).x;
    source[21].z = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[21].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[22].x = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[22].y = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[22].z = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[22].w = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[23].x = (g_VSourceMaterialParameters[6u].wwww).x;
    source[23].y = (g_VSourceMaterialParameters[7u].xxxx).x;
    source[23].z = (g_VSourceMaterialParameters[7u].yyyy).x;
    source[23].w = (g_VSourceMaterialParameters[3u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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
    // 1: mul r0.x, cb0[13].w, cb0[15].z
    r0.x = ((source[13].wwww)*(source[15].zzzz)).x;
    // 2: mad r0.x, cb0[15].w, v4.x, r0.x
    r0.x = ((source[15].wwww)*(v4.xxxx)+(r0.xxxx)).x;
    // 3: mul r0.z, v4.y, cb0[16].x
    r0.z = ((v4.yyyy)*(source[16].xxxx)).z;
    // 4: mad r0.y, cb0[13].w, cb0[16].y, r0.z
    r0.y = ((source[13].wwww)*(source[16].yyyy)+(r0.zzzz)).y;
    // 5: add r0.xy, r0.xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)+(source[6].xyxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (VNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: add r0.y, cb0[3].z, cb0[17].x
    r0.y = ((source[3].zzzz)+(source[17].xxxx)).y;
    // 8: mad r0.xy, r0.xxxx, r0.yyyy, cb0[7].xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(source[7].xyxx)).xy;
    // 9: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 10: dp2 r1.x, cb0[4].xyxx, r0.zwzz
    r1.x = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 11: dp2 r1.y, cb0[5].xyxx, r0.zwzz
    r1.y = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 12: mad r1.xy, cb0[14].zwzz, cb0[3].xxxx, r1.xyxx
    r1.xy = ((source[14].zwzz)*(source[3].xxxx)+(r1.xyxx)).xy;
    // 13: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 14: mul r1.xy, r1.xyxx, cb0[14].xyxx
    r1.xy = ((r1.xyxx)*(source[14].xyxx)).xy;
    // 15: mad r2.x, cb0[13].w, cb0[13].z, r1.x
    r2.x = ((source[13].wwww)*(source[13].zzzz)+(r1.xxxx)).x;
    // 16: mad r2.y, cb0[13].w, cb0[15].y, r1.y
    r2.y = ((source[13].wwww)*(source[15].yyyy)+(r1.yyyy)).y;
    // 17: add r0.xy, r0.xyxx, r2.xyxx
    r0.xy = ((r0.xyxx)+(r2.xyxx)).xy;
    // 18: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 19: dp2 r1.x, cb0[8].xyxx, r0.xyxx
    r1.x = (dot((source[8].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 20: dp2 r1.y, cb0[9].xyxx, r0.xyxx
    r1.y = (dot((source[9].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 21: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 22: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s0, l(-1.000000)
    r0.x = (VNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 23: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 24: mul r0.y, r0.y, cb0[17].w
    r0.y = ((r0.yyyy)*(source[17].wwww)).y;
    // 25: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 26: mul r0.y, r0.y, cb0[18].x
    r0.y = ((r0.yyyy)*(source[18].xxxx)).y;
    // 27: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 28: movc r0.y, r1.x, l(0), r0.y
    r0.y = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 29: mad r0.y, r0.x, cb0[18].y, r0.y
    r0.y = ((r0.xxxx)*(source[18].yyyy)+(r0.yyyy)).y;
    // 30: dp2 r1.x, cb0[10].xyxx, r0.zwzz
    r1.x = (dot((source[10].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 31: dp2 r1.y, cb0[11].xyxx, r0.zwzz
    r1.y = (dot((source[11].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 32: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 33: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 34: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 35: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 36: mul r0.z, r0.z, cb0[23].x
    r0.z = ((r0.zzzz)*(source[23].xxxx)).z;
    // 37: max r0.z, r0.z, cb0[23].z
    r0.z = (max(r0.zzzz,source[23].zzzz)).z;
    // 38: min r0.z, r0.z, cb0[23].y
    r0.z = (min(r0.zzzz,source[23].yyyy)).z;
    // 39: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 40: mul r0.w, v4.x, cb0[20].w
    r0.w = ((v4.xxxx)*(source[20].wwww)).w;
    // 41: mad r2.x, cb0[13].w, cb0[20].z, r0.w
    r2.x = ((source[13].wwww)*(source[20].zzzz)+(r0.wwww)).x;
    // 42: mul r1.zw, cb0[13].wwww, cb0[21].yyyw
    r1.zw = ((source[13].wwww)*(source[21].yyyw)).zw;
    // 43: mad r2.y, cb0[21].x, v4.y, r1.z
    r2.y = ((source[21].xxxx)*(v4.yyyy)+(r1.zzzz)).y;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t2.yzwx, s2, l(0.000000)
    r0.w = (VNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 45: mad r1.xy, r0.wwww, cb0[21].zzzz, r1.xyxx
    r1.xy = ((r0.wwww)*(source[21].zzzz)+(r1.xyxx)).xy;
    // 46: mad r2.y, cb0[19].x, r1.y, r1.w
    r2.y = ((source[19].xxxx)*(r1.yyyy)+(r1.wwww)).y;
    // 47: mul r0.w, r1.x, cb0[18].w
    r0.w = ((r1.xxxx)*(source[18].wwww)).w;
    // 48: mad r2.x, cb0[13].w, cb0[18].z, r0.w
    r2.x = ((source[13].wwww)*(source[18].zzzz)+(r0.wwww)).x;
    // 49: add r1.xy, r2.xyxx, cb0[22].xyxx
    r1.xy = ((r2.xyxx)+(source[22].xyxx)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t3.yzwx, s3, l(0.000000)
    r0.w = (VNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 51: add r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 52: add r1.x, cb0[3].y, l(-1.000000)
    r1.x = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 53: add_sat r0.w, r0.w, -r1.x
    r0.w = (saturate((r0.wwww)+(-(r1.xxxx)))).w;
    // 54: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 55: mul r1.x, r1.x, cb0[22].w
    r1.x = ((r1.xxxx)*(source[22].wwww)).x;
    // 56: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 57: mul_sat r1.x, r1.x, cb0[22].z
    r1.x = (saturate((r1.xxxx)*(source[22].zzzz))).x;
    // 58: lt r1.y, r0.w, l(0.000001)
    r1.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 59: mul r0.w, r0.w, cb0[22].z
    r0.w = ((r0.wwww)*(source[22].zzzz)).w;
    // 60: movc r1.x, r1.y, l(-0.000000), -r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r1.xxxx))).x;
    // 61: mov_sat r1.y, r0.w
    r1.y = (saturate(r0.wwww)).y;
    // 62: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 63: mul r0.x, r0.x, cb0[23].w
    r0.x = ((r0.xxxx)*(source[23].wwww)).x;
    // 64: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 65: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 66: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 67: add r0.x, r1.x, r1.y
    r0.x = ((r1.xxxx)+(r1.yyyy)).x;
    // 68: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 69: mad r0.xyz, r0.xxxx, cb0[12].xyzx, r0.yyyy
    r0.xyz = ((r0.xxxx)*(source[12].xyzx)+(r0.yyyy)).xyz;
    // 70: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 71: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_mi_00.fx_mi.fx_a_pa_db_01_2_ad; source PS 1f6b66cb17f56f4d8ec7cb004d70bd5b.
float4 WRNative262(WR_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[2u];
    source[2] = g_VSourceMaterialParameters[1u];
    source[3].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.zwzz, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (VNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (VNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 3: add r0.xyzw, r0.wxyz, -r1.wxyz
    r0.xyzw = ((r0.wxyz)+(-(r1.wxyz))).xyzw;
    // 4: mad r0.xyzw, v0.wwww, r0.xyzw, r1.wxyz
    r0.xyzw = ((v0.wwww)*(r0.xyzw)+(r1.wxyz)).xyzw;
    // 5: mul r1.xyz, r0.yzwy, cb0[3].xxxx
    r1.xyz = ((r0.yzwy)*(source[3].xxxx)).xyz;
    // 6: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 7: mad r0.yzw, -cb0[3].xxxx, r0.yyzw, r1.wwww
    r0.yzw = ((-(source[3].xxxx))*(r0.yyzw)+(r1.wwww)).yzw;
    // 8: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 9: mad r0.yzw, cb0[3].yyyy, r0.yyzw, r1.xxyz
    r0.yzw = ((source[3].yyyy)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 10: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 11: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 12: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 13: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 14: log r1.x, |r0.x|
    r1.x = (log2(abs(r0.xxxx))).x;
    // 15: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 17: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 18: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 19: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 20: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 21: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_k_me_makeflow_03_05_tr: 5db7b7be4bce824eb486c9d7ae062e1f; selected map 5a1d0845cce484fa6b92bf759529b3193bb35f872d747fe030f96a967ee97f46.
float4 WRNative277(WR_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_VSourceMaterialParameters[8u];
    source[3] = g_VSourceMaterialParameters[6u];
    source[4] = VNativeAppend(g_VSourceMaterialParameters[3u].wwww,g_VSourceMaterialParameters[4u].xxxx,1u);
    source[5] = VNativeAppend((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[3u].yyyy),(g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[3u].zzzz),1u);
    source[6] = input.dynamicParameter;
    source[7] = VNativeAppend((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[0u].yyyy),(g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[0u].zzzz),1u);
    source[8] = VNativeAppend((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].yyyy),(g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].zzzz),1u);
    source[9] = VNativeAppend(g_VSourceMaterialParameters[5u].zzzz,g_VSourceMaterialParameters[5u].wwww,1u);
    source[10] = VNativeAppend(cos((g_VSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = VNativeAppend(sin((g_VSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_VSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[12].y = (g_VSourceMaterialParameters[0u].wwww).x;
    source[12].z = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[12].w = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[13].x = ((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[0u].zzzz)).x;
    source[13].y = ((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[0u].yyyy)).x;
    source[13].z = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[13].w = (g_VSourceMaterialParameters[1u].wwww).x;
    source[14].x = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[14].y = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[14].z = (g_VSourceMaterialParameters[2u].wwww).x;
    source[14].w = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[15].x = (cos((g_VSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[15].y = (g_VSourceMaterialParameters[5u].wwww).x;
    source[15].z = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[15].w = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[16].x = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[16].y = (g_VSourceMaterialParameters[4u].wwww).x;
    source[16].z = (g_VSourceMaterialParameters[5u].yyyy).x;
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
    // 1: mov r0.y, cb0[12].z
    r0.y = (source[12].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.zw, v4.xxxy, cb0[4].xxxy, cb0[5].xxxy
    r0.zw = ((v4.xxxy)*(source[4].xxxy)+(source[5].xxxy)).zw;
    // 4: add r1.xyzw, r0.zwzw, r0.yxxy
    r1.xyzw = ((r0.zwzw)+(r0.yxxy)).xyzw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.zwzz, t0.xyzw, s0, l(0.000000)
    r0.x = (VNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (VNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (VNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (VNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r2.xyz = (VNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (VNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

// fx_h_me_swing_01_1_tr: 0cbb7a45d8aa1a4ca69a1907b36fff29; selected map 19e4fda4abd98b350bd102a7bd1c5399c2104033636b3652c9c162016ef55aee.
float4 WRNative278(WR_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_VSourceMaterialParameters[1u];
    source[3] = VNativeAppend(VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[4] = input.dynamicParameter;
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
    // 1: add r0.xyzw, v4.xyyx, l(-0.020000, -0.100000, -1.000000, -0.050000)
    r0.xyzw = ((v4.xyyx)+(float4(-0.020000,-0.100000,-1.000000,-0.050000))).xyzw;
    // 2: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 3: mul r1.x, r1.x, cb0[4].x
    r1.x = ((r1.xxxx)*(source[4].xxxx)).x;
    // 4: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 5: mad_sat r1.y, -r0.z, l(20.000000), l(1.000000)
    r1.y = (saturate((-(r0.zzzz))*(float4(20.000000,20.000000,20.000000,20.000000))+(float4(1.000000,1.000000,1.000000,1.000000)))).y;
    // 6: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 7: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 8: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 9: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 10: movc r0.y, r0.y, l(0), r1.x
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).y;
    // 11: mul r0.xz, r0.xxzx, l(10.000000, 0.000000, 10.000000, 0.000000)
    r0.xz = ((r0.xxzx)*(float4(10.000000,0.000000,10.000000,0.000000))).xz;
    // 12: mul r1.x, |r0.z|, |r0.z|
    r1.x = ((abs(r0.zzzz))*(abs(r0.zzzz))).x;
    // 13: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 14: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 15: add r0.xw, -|r0.xxxw|, l(1.000000, 0.000000, 0.000000, 1.000000)
    r0.xw = ((-(abs(r0.xxxw)))+(float4(1.000000,0.000000,0.000000,1.000000))).xw;
    // 16: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 17: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 18: mul r1.x, |r0.w|, |r0.w|
    r1.x = ((abs(r0.wwww))*(abs(r0.wwww))).x;
    // 19: mul r1.x, |r0.w|, r1.x
    r1.x = ((abs(r0.wwww))*(r1.xxxx)).x;
    // 20: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 21: mul r1.yzw, v4.xxxy, l(0.000000, 10.000000, 1.500000, 1.000000)
    r1.yzw = ((v4.xxxy)*(float4(0.000000,10.000000,1.500000,1.000000))).yzw;
    // 22: mov_sat r1.y, r1.y
    r1.y = (saturate(r1.yyyy)).y;
    // 23: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 24: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 25: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 26: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 27: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 28: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 30: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 31: mul r1.x, r1.x, l(15.000000)
    r1.x = ((r1.xxxx)*(float4(15.000000,15.000000,15.000000,15.000000))).x;
    // 32: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 33: mul r1.x, r1.x, cb0[4].y
    r1.x = ((r1.xxxx)*(source[4].yyyy)).x;
    // 34: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 35: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 36: mad r0.x, r0.x, r0.z, r0.w
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.wwww)).x;
    // 37: mad r0.yz, v4.xxyx, l(0.000000, 0.500000, 0.300000, 0.000000), cb0[3].xxyx
    r0.yz = ((v4.xxyx)*(float4(0.000000,0.500000,0.300000,0.000000))+(source[3].xxyx)).yz;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (VNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 39: mad r0.yz, r0.yyyy, l(0.000000, 0.100000, 0.100000, 0.000000), r1.zzwz
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.100000,0.100000,0.000000))+(r1.zzwz)).yz;
    // 40: add r0.yz, r0.yyzy, l(0.000000, 0.000000, 0.050000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,0.000000,0.050000,0.000000))).yz;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (VNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 42: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 43: mad r0.y, r0.y, l(100.000000), r0.x
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))+(r0.xxxx)).y;
    // 44: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 45: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 46: mad r0.xyz, r0.yyyy, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.yyyy)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 47: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_j_me_linearflow_02_12_tr: a1991629e6776e40b3eeeae7c262e8ae; selected map d774239272a22634237d30b9d7e0e7bdc8b5b71c7969da40ca6a1e14dc1799f9.
float4 WRNative279(WR_NATIVE_INPUT input)
{
    float4 source[30]; [unroll] for (uint i=0u; i<30u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_VSourceMaterialParameters[16u];
    source[3] = input.dynamicParameter;
    source[4] = VNativeAppend(cos((g_VSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = VNativeAppend(sin((g_VSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_VSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = VNativeAppend(g_VSourceMaterialParameters[2u].yyyy,g_VSourceMaterialParameters[2u].zzzz,1u);
    source[7] = VNativeAppend(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].zzzz)+g_VSourceMaterialParameters[1u].xxxx),((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[1u].wwww)+g_VSourceMaterialParameters[1u].yyyy),1u);
    source[8] = g_VSourceMaterialParameters[13u];
    source[9] = VNativeAppend(cos((g_VSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = VNativeAppend(sin((g_VSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_VSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11] = VNativeAppend(g_VSourceMaterialParameters[6u].zzzz,g_VSourceMaterialParameters[6u].wwww,1u);
    source[12] = VNativeAppend(((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[5u].wwww)+g_VSourceMaterialParameters[5u].yyyy),((g_VSourceMaterialTime.xxxx*g_VSourceMaterialParameters[6u].xxxx)+g_VSourceMaterialParameters[5u].zzzz),1u);
    source[13] = g_VSourceMaterialParameters[14u];
    source[14] = VNativeAppend(cos((g_VSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[15] = VNativeAppend(sin((g_VSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_VSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[16].x = (g_VSourceMaterialParameters[9u].yyyy).x;
    source[16].y = (g_VSourceMaterialParameters[9u].zzzz).x;
    source[16].z = (g_VSourceMaterialParameters[9u].xxxx).x;
    source[16].w = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[17].x = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[17].y = (g_VSourceMaterialParameters[3u].wwww).x;
    source[17].z = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[17].w = (g_VSourceMaterialParameters[2u].wwww).x;
    source[18].x = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[18].y = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[18].z = (g_VSourceMaterialTime.xxxx).x;
    source[18].w = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[19].x = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[19].y = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[19].z = (floor(g_VSourceMaterialParameters[4u].yyyy)).x;
    source[19].w = (g_VSourceMaterialParameters[0u].wwww).x;
    source[20].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[20].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[20].z = (g_VSourceMaterialParameters[7u].zzzz).x;
    source[20].w = ((g_VSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[21].x = (sin((g_VSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[21].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[21].z = (cos((g_VSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[21].w = (g_VSourceMaterialParameters[7u].wwww).x;
    source[22].x = (g_VSourceMaterialParameters[8u].xxxx).x;
    source[22].y = (g_VSourceMaterialParameters[8u].yyyy).x;
    source[22].z = (g_VSourceMaterialParameters[7u].xxxx).x;
    source[22].w = (g_VSourceMaterialParameters[7u].yyyy).x;
    source[23].x = (g_VSourceMaterialParameters[6u].wwww).x;
    source[23].y = (g_VSourceMaterialParameters[6u].zzzz).x;
    source[23].z = (g_VSourceMaterialParameters[6u].yyyy).x;
    source[23].w = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[24].x = (floor(g_VSourceMaterialParameters[0u].xxxx)).x;
    source[24].y = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[24].z = (g_VSourceMaterialParameters[4u].wwww).x;
    source[24].w = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[25].x = (g_VSourceMaterialParameters[8u].wwww).x;
    source[25].y = (g_VSourceMaterialParameters[8u].zzzz).x;
    source[25].z = (g_VSourceMaterialParameters[11u].wwww).x;
    source[25].w = (g_VSourceMaterialParameters[12u].zzzz).x;
    source[26].x = (g_VSourceMaterialParameters[12u].wwww).x;
    source[26].y = (g_VSourceMaterialParameters[12u].xxxx).x;
    source[26].z = (g_VSourceMaterialParameters[11u].zzzz).x;
    source[26].w = (g_VSourceMaterialParameters[10u].xxxx).x;
    source[27].x = (g_VSourceMaterialParameters[10u].wwww).x;
    source[27].y = (g_VSourceMaterialParameters[11u].xxxx).x;
    source[27].z = (g_VSourceMaterialParameters[10u].zzzz).x;
    source[27].w = ((g_VSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[28].x = (sin((g_VSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[28].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_VSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[28].z = (cos((g_VSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[28].w = (g_VSourceMaterialParameters[10u].yyyy).x;
    source[29].x = (g_VSourceMaterialParameters[9u].wwww).x;
    source[29].y = (g_VSourceMaterialParameters[12u].yyyy).x;
    source[29].z = (g_VSourceMaterialParameters[11u].yyyy).x;
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
    // 1: mad r0.xy, v4.wzww, cb0[11].xyxx, cb0[12].xyxx
    r0.xy = ((v4.wzww)*(source[11].xyxx)+(source[12].xyxx)).xy;
    // 2: add r1.xyzw, r0.xyxy, l(0.200000, 0.000000, 0.000000, 0.200000)
    r1.xyzw = ((r0.xyxy)+(float4(0.200000,0.000000,0.000000,0.200000))).xyzw;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s4, l(0.000000)
    r0.x = (VNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t2.yxzw, s4, l(0.000000)
    r0.y = (VNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t2.yzxw, s4, l(0.000000)
    r0.z = (VNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r0.xyz = (VNativeSample3((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
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
    r1.z = (VNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t0.yzwx, s2, l(0.000000)
    r1.w = (VNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.zwzz, t0.xyzw, s2, l(0.000000)
    r2.x = (VNativeSample2((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r2.xyz = (VNativeSample1((r1.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
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
    r1.x = (VNativeSample5((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r2.xyz = (VNativeSample0((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
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


EFFECT_PS_OUT Shade_EffectDimensionMasterWRNative(uint profile, WR_NATIVE_INPUT input)
{
    EFFECT_PS_OUT output=(EFFECT_PS_OUT)0;
    float4 nativeColor=0.f;
    bool additive=false;
    switch(profile)
    {
    case 208u: nativeColor=WRNative208(input); additive=false; break;
    case 209u: nativeColor=WRNative209(input); additive=true; break;
    case 210u: nativeColor=WRNative210(input); additive=true; break;
    case 211u: nativeColor=WRNative211(input); additive=false; break;
    case 212u: nativeColor=WRNative212(input); additive=true; break;
    case 213u: nativeColor=WRNative213(input); additive=false; break;
    case 214u: nativeColor=WRNative214(input); additive=false; break;
    case 215u: nativeColor=WRNative215(input); additive=false; break;
    case 216u: nativeColor=WRNative216(input); additive=false; break;
    case 217u: nativeColor=WRNative217(input); additive=false; break;
    case 218u: nativeColor=WRNative218(input); additive=true; break;
    case 220u: nativeColor=WRNative220(input); additive=false; break;
    case 221u: nativeColor=WRNative221(input); additive=true; break;
    case 222u: nativeColor=WRNative222(input); additive=true; break;
    case 223u: nativeColor=WRNative223(input); additive=true; break;
    case 224u: nativeColor=WRNative224(input); additive=true; break;
    case 225u: nativeColor=WRNative225(input); additive=false; break;
    case 226u: nativeColor=WRNative226(input); additive=true; break;
    case 229u: nativeColor=WRNative229(input); additive=false; break;
    case 230u: nativeColor=WRNative230(input); additive=false; break;
    case 231u: nativeColor=WRNative231(input); additive=false; break;
    case 232u: nativeColor=WRNative232(input); additive=false; break;
    case 233u: nativeColor=WRNative233(input); additive=true; break;
    case 234u: nativeColor=WRNative234(input); additive=true; break;
    case 235u: nativeColor=WRNative235(input); additive=false; break;
    case 238u: nativeColor=WRNative214(input); additive=false; break;
    case 239u: nativeColor=WRNative239(input); additive=true; break;
    case 240u: nativeColor=WRNative226(input); additive=true; break;
    case 241u: nativeColor=WRNative241(input); additive=true; break;
    case 242u: nativeColor=WRNative216(input); additive=false; break;
    case 243u: nativeColor=WRNative243(input); additive=true; break;
    case 244u: nativeColor=WRNative244(input); additive=true; break;
    case 245u: nativeColor=WRNative245(input); additive=true; break;
    case 246u: nativeColor=WRNative246(input); additive=true; break;
    case 247u: nativeColor=WRNative247(input); additive=true; break;
    case 248u: nativeColor=WRNative248(input); additive=false; break;
    case 251u: nativeColor=WRNative251(input); additive=false; break;
    case 252u: nativeColor=WRNative252(input); additive=true; break;
    case 253u: nativeColor=WRNative220(input); additive=false; break;
    case 254u: nativeColor=WRNative254(input); additive=false; break;
    case 255u: nativeColor=WRNative255(input); additive=false; break;
    case 256u: nativeColor=WRNative256(input); additive=true; break;
    case 257u: nativeColor=WRNative257(input); additive=false; break;
    case 258u: nativeColor=WRNative258(input); additive=false; break;
    case 259u: nativeColor=WRNative259(input); additive=false; break;
    case 260u: nativeColor=WRNative260(input); additive=false; break;
    case 262u: nativeColor=WRNative262(input); additive=true; break;
    case 263u: nativeColor=WRNative214(input); additive=false; break;
    case 277u: nativeColor=WRNative277(input); additive=false; break;
    case 278u: nativeColor=WRNative278(input); additive=false; break;
    case 279u: nativeColor=WRNative279(input); additive=false; break;
    case 280u: nativeColor=WRNative277(input); additive=false; break;
    default: clip(-1.f); return output;
    }
    output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity, additive ? 1.f : nativeColor.a);
    output.Distortion=0.f; // Separate native distortion/MRT passes are not admitted.
    if(g_ColorClip>0.f) clip(output.SceneColor.a-g_ColorClip);
    return output;
}
#endif
