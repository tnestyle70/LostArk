// Exact selected native PS instructions; scene inputs are supplied by the map forward draw.
#ifndef LOSTARK_SOURCE_MAP_FORWARD_PROGRAMS
#define LOSTARK_SOURCE_MAP_FORWARD_PROGRAMS
#include "Shader_SceneHeightFog.hlsli"
Texture2D g_SourceMapSceneDepth;
float4 g_SourceMapAmbient = 0.f;
SamplerState SourceMapDepthSampler { Filter=MIN_MAG_MIP_POINT; AddressU=Clamp; AddressV=Clamp; };
// Original cloud_01_m: wrap U, mirror V.
SamplerState SourceMapSkyCloudSampler { Filter=MIN_MAG_MIP_LINEAR; AddressU=Wrap; AddressV=Mirror; AddressW=Wrap; };

// source.map.spotlight.v1: b45f2673af7b9b48b0ef92280aa46ad5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapForward33(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for(uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[0] = float4(0.f,0.f,0.f,1.f);
    source[0].x=1.f;
    source[2]=float4(g_WorldMatrix[0].xzy,0.f);
    source[3]=float4(g_WorldMatrix[2].xzy,0.f);
    source[6].x=g_SourceCharacterTime;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // mul r3.xyz, r2.xyzx, cb0[2].yyyy
    r3.xyz = ((r2.xyzx)*(source[2].yyyy)).xyz;
    // mul r2.xyz, r2.xyzx, cb0[3].yyyy
    r2.xyz = ((r2.xyzx)*(source[3].yyyy)).xyz;
    // mad r2.xyz, cb0[3].xxxx, r1.xyzx, r2.xyzx
    r2.xyz = ((source[3].xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // mad r1.xyz, cb0[2].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[2].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // mad r1.xyz, cb0[2].zzzz, r0.xyzx, r1.xyzx
    r1.xyz = ((source[2].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // mad r0.xyz, cb0[3].zzzz, r0.xyzx, r2.xyzx
    r0.xyz = ((source[3].zzzz)*(r0.xyzx)+(r2.xyzx)).xyz;
    // dp3 r0.x, -r0.xyzx, -r0.xyzx
    r0.x = (dot((-(r0.xyzx)).xyz,(-(r0.xyzx)).xyz).xxxx).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // mul r0.y, r0.x, v4.y
    r0.y = ((r0.xxxx)*(v4.yyyy)).y;
    // dp3 r0.z, -r1.xyzx, -r1.xyzx
    r0.z = (dot((-(r1.xyzx)).xyz,(-(r1.xyzx)).xyz).xxxx).z;
    // sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // mul r0.x, r0.z, v4.x
    r0.x = ((r0.zzzz)*(v4.xxxx)).x;
    // mul r0.z, cb0[6].y, cb0[6].x
    r0.z = ((source[6].yyyy)*(source[6].xxxx)).z;
    // mul r0.zw, r0.zzzz, l(0.000000, 0.000000, 0.130000, 0.100000)
    r0.zw = ((r0.zzzz)*(float4(0.000000,0.000000,0.130000,0.100000))).zw;
    // mad r0.xyzw, r0.xyxy, l(0.620000, 1.610000, 0.669000, 1.970000), r0.zzww
    r0.xyzw = ((r0.xyxy)*(float4(0.620000,1.610000,0.669000,1.970000))+(r0.zzww)).xyzw;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t0.yxzw, s1, l(0.000000)
    r0.y = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).y;
    // mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // mul r0.x, r0.x, cb0[6].z
    r0.x = ((r0.xxxx)*(source[6].zzzz)).x;
    // div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s0, l(0.000000)
    r0.y = (g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler, (r0.yzyy).xy,0.f).rrrr).y;
    // min r0.y, r0.y, l(0.999000)
    r0.y = (min(r0.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // mad r0.z, r0.y, cb2[1].z, -cb2[1].w
    r0.z = ((r0.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).z;
    // mad r0.y, r0.y, cb2[1].x, cb2[1].y
    r0.y = ((r0.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).y;
    // div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // mul_sat r0.y, r0.y, l(0.034483)
    r0.y = (saturate((r0.yyyy)*(float4(0.034483,0.034483,0.034483,0.034483)))).y;
    // dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // mul r0.z, |r0.z|, |r0.z|
    r0.z = ((abs(r0.zzzz))*(abs(r0.zzzz))).z;
    // mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // add r0.z, v4.x, l(-0.500000)
    r0.z = ((v4.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).z;
    // mad r0.z, -|r0.z|, l(2.000000), l(1.000000)
    r0.z = ((-(abs(r0.zzzz)))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // add_sat r0.z, r0.z, -v4.y
    r0.z = (saturate((r0.zzzz)+(-(v4.yyyy)))).z;
    // add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // mad r0.z, -r0.z, r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // mul r0.w, v4.y, v4.y
    r0.w = ((v4.yyyy)*(v4.yyyy)).w;
    // mul r0.w, r0.w, v4.y
    r0.w = ((r0.wwww)*(v4.yyyy)).w;
    // dp2 r0.z, r0.zzzz, r0.wwww
    r0.z = (dot((r0.zzzz).xy,(r0.wwww).xy).xxxx).z;
    // mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // mul r0.z, r0.y, cb0[6].w
    r0.z = ((r0.yyyy)*(source[6].wwww)).z;
    // mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // mad r0.yzw, cb0[5].wwww, cb0[5].xxyz, cb0[4].xxyz
    r0.yzw = ((source[5].wwww)*(source[5].xxyz)+(source[4].xxyz)).yzw;
    // mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // mul o0.xyz, r0.xxxx, r0.yzwy
    output.targets[0].xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // ret
    return output;
}

// source.map.translucent-tiled.v1: 1566c6cdf2ffc24aa1da17964a7aed32
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapForward34(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for(uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[0] = float4(0.f,0.f,0.f,1.f);
    source[8]=0.f; source[9]=0.f;
    source[10]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // mul r1.z, r1.z, cb0[6].z
    r1.z = ((r1.zzzz)*(source[6].zzzz)).z;
    // dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // mul r1.xy, r1.xyxx, cb0[5].wwww
    r1.xy = ((r1.xyxx)*(source[5].wwww)).xy;
    // mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // div r1.xyw, r2.xyxz, r1.xxxx
    r1.xyw = ((r2.xyxz)/(r1.xxxx)).xyw;
    // dp3 r2.x, r1.xywx, r1.xywx
    r2.x = (dot((r1.xywx).xyz,(r1.xywx).xyz).xxxx).x;
    // rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // mul r1.xyw, r1.xyxw, r2.xxxx
    r1.xyw = ((r1.xyxw)*(r2.xxxx)).xyw;
    // dp3 r2.x, v6.xyzx, v6.xyzx
    r2.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // mul r2.xyz, r2.xxxx, v6.xyzx
    r2.xyz = ((r2.xxxx)*(v6.xyzx)).xyz;
    // dp3 r2.w, r1.xywx, r2.xyzx
    r2.w = (dot((r1.xywx).xyz,(r2.xyzx).xyz).xxxx).w;
    // mul r3.xyz, r1.xywx, r2.wwww
    r3.xyz = ((r1.xywx)*(r2.wwww)).xyz;
    // mad r2.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r3.xyz, r2.wwww, v1.xyzx
    r3.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r4.xyz, r2.wwww, v0.xyzx
    r4.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // dp3 r3.z, r3.xyzx, r1.xywx
    r3.z = (dot((r3.xyzx).xyz,(r1.xywx).xyz).xxxx).z;
    // mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // dp3 r6.y, r5.xyzx, r2.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // dp3 r6.x, r4.xyzx, r2.xyzx
    r6.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // dp3 r3.x, r4.xyzx, r1.xywx
    r3.x = (dot((r4.xyzx).xyz,(r1.xywx).xyz).xxxx).x;
    // dp3 r3.y, r5.xyzx, r1.xywx
    r3.y = (dot((r5.xyzx).xyz,(r1.xywx).xyz).xxxx).y;
    // mul r2.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r2.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // mad r2.xy, cb0[6].xxxx, r6.xyxx, r2.xyxx
    r2.xy = ((source[6].xxxx)*(r6.xyxx)+(r2.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r2.xyz, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((r2.xyzx)*(source[3].xyzx)).xyz;
    // mad r2.xyz, cb0[6].yyyy, r2.xyzx, r2.xyzx
    r2.xyz = ((source[6].yyyy)*(r2.xyzx)+(r2.xyzx)).xyz;
    // add r2.xyz, r2.xyzx, -cb0[6].yyyy
    r2.xyz = ((r2.xyzx)+(-(source[6].yyyy))).xyz;
    // mov_sat r4.xyz, r2.xyzx
    r4.xyz = (saturate(r2.xyzx)).xyz;
    // mov_sat r2.xyz, -r2.xyzx
    r2.xyz = (saturate(-(r2.xyzx))).xyz;
    // mad r2.xyz, -r1.zzzz, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r1.zzzz))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r5.xyz, cb0[4].xyzx, cb0[6].wwww
    r5.xyz = ((source[4].xyzx)*(source[6].wwww)).xyz;
    // mul r0.xyz, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // mul_sat r0.w, r0.w, cb0[7].z
    r0.w = (saturate((r0.wwww)*(source[7].zzzz))).w;
    // mul o0.w, r0.w, cb0[0].w
    output.targets[0].w = ((r0.wwww)*(source[0].wwww)).w;
    // mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r2.xyz, r0.wwww, v7.xyzx
    r2.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // dp3 r0.w, r2.xyzx, r1.xywx
    r0.w = (dot((r2.xyzx).xyz,(r1.xywx).xyz).xxxx).w;
    // mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // mul r1.yzw, r1.yyyy, cb0[9].xxyz
    r1.yzw = ((r1.yyyy)*(source[9].xxyz)).yzw;
    // mad r1.xyz, r1.xxxx, cb0[8].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[8].xyzx)+(r1.yzwy)).xyz;
    // mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // mad r2.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r1.xyz, r0.xyzx, cb0[10].xyzx, r2.xyzx
    r1.xyz = ((r0.xyzx)*(source[10].xyzx)+(r2.xyzx)).xyz;
    // mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// source.map.translucent-reflection.v1: 10427e64f512ab4f86b47a72e36ef247
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapForward35(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for(uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[0] = float4(0.f,0.f,0.f,1.f);
    source[6]=0.f; source[7]=0.f;
    source[8]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // mul r0.xyz, r0.xyzx, cb0[4].xxwx
    r0.xyz = ((r0.xyzx)*(source[4].xxwx)).xyz;
    // mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // add r0.x, -r0.w, l(1.000000)
    r0.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // div r0.xyw, r1.xyxz, r0.xxxx
    r0.xyw = ((r1.xyxz)/(r0.xxxx)).xyw;
    // dp3 r1.x, r0.xywx, r0.xywx
    r1.x = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r0.xyw, r0.xyxw, r1.xxxx
    r0.xyw = ((r0.xyxw)*(r1.xxxx)).xyw;
    // dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v6.xyzx
    r1.xyz = ((r1.xxxx)*(v6.xyzx)).xyz;
    // dp3 r1.w, r0.xywx, r1.xyzx
    r1.w = (dot((r0.xywx).xyz,(r1.xyzx).xyz).xxxx).w;
    // mul r2.xyz, r0.xywx, r1.wwww
    r2.xyz = ((r0.xywx)*(r1.wwww)).xyz;
    // mad r1.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, v0.xyzx
    r3.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // mul r4.xyz, r2.zxyz, r3.yzxy
    r4.xyz = ((r2.zxyz)*(r3.yzxy)).xyz;
    // mad r4.xyz, r2.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r2.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // dp3 r2.z, r2.xyzx, r0.xywx
    r2.z = (dot((r2.xyzx).xyz,(r0.xywx).xyz).xxxx).z;
    // mul r4.xyz, r4.xyzx, v1.wwww
    r4.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // dp3 r5.y, r4.xyzx, r1.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // dp3 r5.x, r3.xyzx, r1.xyzx
    r5.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // dp3 r2.x, r3.xyzx, r0.xywx
    r2.x = (dot((r3.xyzx).xyz,(r0.xywx).xyz).xxxx).x;
    // dp3 r2.y, r4.xyzx, r0.xywx
    r2.y = (dot((r4.xyzx).xyz,(r0.xywx).xyz).xxxx).y;
    // mul r1.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r1.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // mad r1.xy, cb0[4].yyyy, r5.xyxx, r1.xyxx
    r1.xy = ((source[4].yyyy)*(r5.xyxx)+(r1.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // mad r1.xyz, cb0[4].zzzz, r1.xyzx, r1.xyzx
    r1.xyz = ((source[4].zzzz)*(r1.xyzx)+(r1.xyzx)).xyz;
    // add r1.xyz, r1.xyzx, -cb0[4].zzzz
    r1.xyz = ((r1.xyzx)+(-(source[4].zzzz))).xyz;
    // mov_sat r3.xyz, -r1.xyzx
    r3.xyz = (saturate(-(r1.xyzx))).xyz;
    // mov_sat r1.xyz, r1.xyzx
    r1.xyz = (saturate(r1.xyzx)).xyz;
    // mad r3.xyz, -r0.zzzz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r0.zzzz))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r4.xyz, cb0[3].xyzx, cb0[5].xxxx
    r4.xyz = ((source[3].xyzx)*(source[5].xxxx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mul_sat r1.w, r5.w, cb0[5].w
    r1.w = (saturate((r5.wwww)*(source[5].wwww))).w;
    // mul o0.w, r1.w, cb0[0].w
    output.targets[0].w = ((r1.wwww)*(source[0].wwww)).w;
    // mad r1.xyz, r0.zzzz, r1.xyzx, r4.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r4.xyzx)).xyz;
    // mul r1.xyz, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r1.xyzx)).xyz;
    // max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mul r3.xyz, r0.zzzz, v7.xyzx
    r3.xyz = ((r0.zzzz)*(v7.xyzx)).xyz;
    // dp3 r0.x, r3.xyzx, r0.xywx
    r0.x = (dot((r3.xyzx).xyz,(r0.xywx).xyz).xxxx).x;
    // mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // mul r0.yzw, r0.yyyy, cb0[7].xxyz
    r0.yzw = ((r0.yyyy)*(source[7].xxyz)).yzw;
    // mad r0.xyz, r0.xxxx, cb0[6].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[6].xyzx)+(r0.yzwy)).xyz;
    // mul r0.xyz, r0.xyzx, cb0[8].wwww
    r0.xyz = ((r0.xyzx)*(source[8].wwww)).xyz;
    // mad r3.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r3.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r0.xyz, r1.xyzx, cb0[8].xyzx, r3.xyzx
    r0.xyz = ((r1.xyzx)*(source[8].xyzx)+(r3.xyzx)).xyz;
    // mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// source.map.translucent-bump.v1: 61aadd65a765fd40a8427a76c66e8d2e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapForward36(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for(uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[0] = float4(0.f,0.f,0.f,1.f);
    source[0].x=1.f;
    source[5]=0.f; source[6]=0.f;
    source[7]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v7.xyzx
    r0.xyz = ((r0.xxxx)*(v7.xyzx)).xyz;
    // dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xy, r0.wwww, v6.xyxx
    r1.xy = ((r0.wwww)*(v6.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.w, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.w = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // add r1.z, r0.w, -cb0[3].x
    r1.z = ((r0.wwww)+(-(source[3].xxxx))).z;
    // mul r1.z, r1.z, cb0[3].y
    r1.z = ((r1.zzzz)*(source[3].yyyy)).z;
    // mad r1.xy, r1.xyxx, r1.zzzz, v4.xyxx
    r1.xy = ((r1.xyxx)*(r1.zzzz)+(v4.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.xyxx, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mad r1.xy, r1.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // mul r1.xy, r1.xyxx, cb0[3].zzzz
    r1.xy = ((r1.xyxx)*(source[3].zzzz)).xy;
    // mul r3.xy, r1.xyxx, v2.wwww
    r3.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // add r3.z, r1.x, l(0.000010)
    r3.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r1.x, r3.xyzx, r3.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // div r1.xyz, r3.xyzx, r1.xxxx
    r1.xyz = ((r3.xyzx)/(r1.xxxx)).xyz;
    // dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // dp3 r0.x, r0.xyzx, r1.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // mul r3.xyz, r0.yyyy, cb0[6].xyzx
    r3.xyz = ((r0.yyyy)*(source[6].xyzx)).xyz;
    // mad r0.xyz, r0.xxxx, cb0[5].xyzx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(source[5].xyzx)+(r3.xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, cb0[7].wwww
    r0.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // mul r3.xyz, cb0[2].xyzx, cb0[3].wwww
    r3.xyz = ((source[2].xyzx)*(source[3].wwww)).xyz;
    // mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // add_sat r1.w, r0.w, r2.w
    r1.w = (saturate((r0.wwww)+(r2.wwww))).w;
    // add_sat r0.w, r0.w, cb0[4].x
    r0.w = (saturate((r0.wwww)+(source[4].xxxx))).w;
    // mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // mul_sat r0.w, r1.w, cb0[4].w
    r0.w = (saturate((r1.wwww)*(source[4].wwww))).w;
    // mul o0.w, r0.w, cb0[0].x
    output.targets[0].w = ((r0.wwww)*(source[0].xxxx)).w;
    // mad r3.xyz, r0.xyzx, r2.xyzx, cb0[1].xyzx
    r3.xyz = ((r0.xyzx)*(r2.xyzx)+(source[1].xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r0.xyz, r2.xyzx, cb0[7].xyzx, r3.xyzx
    r0.xyz = ((r2.xyzx)*(source[7].xyzx)+(r3.xyzx)).xyz;
    // mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // dp3 r0.z, r0.xyzx, r1.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // dp3 r0.x, r2.xyzx, r1.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r0.y, r2.xyzx, r1.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// source.map.sky-simple.v1: 29b7302ad93326409979e84649171179
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapForward37(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for(uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[0] = float4(0.f,0.f,0.f,1.f);
    source[0].x=1.f;
    source[7]=float4(frac(source[63].x*g_SourceCharacterTime*0.008f),0.f,0.f,0.f);
    source[12]=0.f; source[13]=0.f;
    source[14]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // mul r0.x, cb0[10].z, l(0.017453)
    r0.x = ((source[10].zzzz)*(float4(0.017453,0.017453,0.017453,0.017453))).x;
    // sincos null, r0.x, r0.x
    r0.x = (cos(r0.xxxx)).x;
    // max r0.x, r0.x, l(-0.999990)
    r0.x = (max(r0.xxxx,float4(-0.999990,-0.999990,-0.999990,-0.999990))).x;
    // min r0.x, r0.x, l(0.999990)
    r0.x = (min(r0.xxxx,float4(0.999990,0.999990,0.999990,0.999990))).x;
    // mad r0.x, -r0.x, l(0.500000), l(0.500000)
    r0.x = ((-(r0.xxxx))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // mul r0.x, r0.x, l(0.693147)
    r0.x = ((r0.xxxx)*(float4(0.693147,0.693147,0.693147,0.693147))).x;
    // div r0.x, l(-0.301030), r0.x
    r0.x = ((float4(-0.301030,-0.301030,-0.301030,-0.301030))/(r0.xxxx)).x;
    // mul r0.y, v4.y, cb0[6].y
    r0.y = ((v4.yyyy)*(source[6].yyyy)).y;
    // mul r1.y, r0.y, l(2.000000)
    r1.y = ((r0.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))).y;
    // mov r2.xz, v4.yyxy
    r2.xz = (v4.yyxy).xz;
    // mul r1.x, r2.z, cb0[6].x
    r1.x = ((r2.zzzz)*(source[6].xxxx)).x;
    // add r0.yz, r1.xxyx, cb0[7].xxyx
    r0.yz = ((r1.xxyx)+(source[7].xxyx)).yz;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t1.wxyz, s1, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture1.SampleBias(SourceMapSkyCloudSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // mov_sat r0.yzw, r0.yyzw
    r0.yzw = (saturate(r0.yyzw)).yzw;
    // mul r1.x, r0.y, cb0[10].x
    r1.x = ((r0.yyyy)*(source[10].xxxx)).x;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // add r1.y, -r3.w, l(0.200000)
    r1.y = ((-(r3.wwww))+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // mul_sat r1.y, r1.y, l(5.000000)
    r1.y = (saturate((r1.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000)))).y;
    // mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // mul r1.yzw, r3.xxyz, cb0[2].xxyz
    r1.yzw = ((r3.xxyz)*(source[2].xxyz)).yzw;
    // mad r3.xyz, -r3.xyzx, cb0[2].xyzx, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(source[2].xyzx)+(r3.xyzx)).xyz;
    // mad r1.yzw, r3.wwww, r3.xxyz, r1.yyzw
    r1.yzw = ((r3.wwww)*(r3.xxyz)+(r1.yyzw)).yzw;
    // mul r3.xyz, cb0[5].xyzx, cb0[5].wwww
    r3.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // mad r0.yzw, r3.xxyz, r0.yyzw, -r1.yyzw
    r0.yzw = ((r3.xxyz)*(r0.yyzw)+(-(r1.yyzw))).yzw;
    // mad r0.yzw, r1.xxxx, r0.yyzw, r1.yyzw
    r0.yzw = ((r1.xxxx)*(r0.yyzw)+(r1.yyzw)).yzw;
    // mul r3.xyz, cb0[2].xyzx, cb0[10].yyyy
    r3.xyz = ((source[2].xyzx)*(source[10].yyyy)).xyz;
    // mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // mad r0.yzw, -r0.yyzw, l(0.000000, 0.499000, 0.499000, 0.499000), l(0.000000, 0.500000, 0.500000, 0.500000)
    r0.yzw = ((-(r0.yyzw))*(float4(0.000000,0.499000,0.499000,0.499000))+(float4(0.000000,0.500000,0.500000,0.500000))).yzw;
    // log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mov r2.y, cb0[8].z
    r2.y = (source[8].zzzz).y;
    // add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // ge r0.w, v4.y, cb0[8].z
    r0.w = (asfloat((uint4)((v4.yyyy)>=(source[8].zzzz)) * 0xffffffffu)).w;
    // movc r1.x, r0.w, r2.x, v4.y
    r1.x = ((asuint(r0.wwww) != 0u) ? (r2.xxxx) : (v4.yyyy)).x;
    // movc r2.x, r0.w, r2.y, cb0[8].z
    r2.x = ((asuint(r0.wwww) != 0u) ? (r2.yyyy) : (source[8].zzzz)).x;
    // movc_sat r0.w, r0.w, cb0[8].w, cb0[9].x
    r0.w = (saturate((asuint(r0.wwww) != 0u) ? (source[8].wwww) : (source[9].xxxx))).w;
    // div r1.x, r1.x, r2.x
    r1.x = ((r1.xxxx)/(r2.xxxx)).x;
    // add r2.x, -r0.w, l(1.000000)
    r2.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mad r0.w, r1.x, r2.x, r0.w
    r0.w = ((r1.xxxx)*(r2.xxxx)+(r0.wwww)).w;
    // mul r1.x, r0.w, r0.w
    r1.x = ((r0.wwww)*(r0.wwww)).x;
    // mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // mul_sat r0.w, r0.w, cb0[9].y
    r0.w = (saturate((r0.wwww)*(source[9].yyyy))).w;
    // mad r2.xyz, -cb0[11].xxxx, r0.xyzx, r0.wwww
    r2.xyz = ((-(source[11].xxxx))*(r0.xyzx)+(r0.wwww)).xyz;
    // mul r3.xyz, r0.xyzx, cb0[11].xxxx
    r3.xyz = ((r0.xyzx)*(source[11].xxxx)).xyz;
    // mad r2.xyz, cb0[11].xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((source[11].xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // mul r3.xyz, cb0[3].xyzx, cb0[3].wwww
    r3.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // mul r4.xyz, cb0[4].xyzx, cb0[4].wwww
    r4.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // mad r2.xyz, r4.xyzx, r2.xyzx, -r3.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)+(-(r3.xyzx))).xyz;
    // mad r0.xyz, r0.xyzx, r2.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // add r0.xyz, -r1.yzwy, r0.xyzx
    r0.xyz = ((-(r1.yzwy))+(r0.xyzx)).xyz;
    // add r1.x, -r0.w, l(1.000000)
    r1.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mad_sat r0.w, -r1.x, r3.w, r0.w
    r0.w = (saturate((-(r1.xxxx))*(r3.wwww)+(r0.wwww))).w;
    // mad r0.xyz, r0.wwww, r0.xyzx, r1.yzwy
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.yzwy)).xyz;
    // mul r0.xyz, r0.xyzx, cb0[10].yyyy
    r0.xyz = ((r0.xyzx)*(source[10].yyyy)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // mul r1.yzw, r1.yyyy, cb0[13].xxyz
    r1.yzw = ((r1.yyyy)*(source[13].xxyz)).yzw;
    // mad r1.xyz, r1.xxxx, cb0[12].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[12].xyzx)+(r1.yzwy)).xyz;
    // mul r1.xyz, r1.xyzx, cb0[14].wwww
    r1.xyz = ((r1.xyzx)*(source[14].wwww)).xyz;
    // mad r2.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r1.xyz, r0.xyzx, cb0[14].xyzx, r2.xyzx
    r1.xyz = ((r0.xyzx)*(source[14].xyzx)+(r2.xyzx)).xyz;
    // mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // mov o0.w, cb0[0].x
    output.targets[0].w = (source[0].xxxx).w;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

#include "Shader_SourceMapWaterPrograms.hlsli"

float4 EvaluateSourceMapWater(float2 uv,float3 worldPosition,float3 tangent,float3 binormal,
    float3 normal,float4 clipPosition,float4 vertexColor,float2 lightmapUV,float3 cameraPosition)
{
    SOURCE_CHARACTER_NATIVE_INPUT input=(SOURCE_CHARACTER_NATIVE_INPUT)0;
    const float3 t=SourceCharacterSafeUnit(tangent),b=SourceCharacterSafeUnit(binormal),n=SourceCharacterSafeUnit(normal);
    const float3 view=cameraPosition-worldPosition;
    const float3 tangentView=float3(dot(t,view),dot(b,view),dot(n,view));
    const float4 up=float4(t.y,b.y,n.y,0.f);
    // The map importer rotates native (x,y,z) into project (x,z,-y).
    const float4 sourcePosition=float4(worldPosition.x,-worldPosition.z,worldPosition.y,1.f)*float4(100.f,100.f,100.f,1.f);
    const float4x4 vp=mul(g_ViewMatrix,g_ProjMatrix);
    input.projection[0]=vp[0];input.projection[1]=-vp[2];
    input.projection[2]=vp[1];input.projection[3]=vp[3]*100.f;
    input.values[0]=float4(t.x,b.x,n.x,0.f);
    input.values[1]=float4(t.y,b.y,n.y,dot(cross(t,b),n)<0.f?-1.f:1.f);
    input.values[2]=vertexColor;input.values[3]=float4(0.f,0.f,lightmapUV);input.values[4]=float4(uv,0.f,0.f);
    input.values[5]=EvaluateSceneFog(worldPosition,cameraPosition);
    input.values[6]=float4(tangentView,1.f);
    input.values[7]=up;
    input.values[8]=g_SourceCharacterProgram==40u?clipPosition*100.f:sourcePosition;
    if(g_SourceCharacterProgram<=39u)input.values[7]=clipPosition*100.f;
    if(g_SourceCharacterProgram==43u)
    {
        input.values[4]=input.values[5];input.values[5]=float4(tangentView,1.f);
        input.values[6]=up;input.values[7]=sourcePosition;
    }
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    switch(g_SourceCharacterProgram)
    {
    case 38u:output=SourceMapWater38(input);break;
    case 39u:output=SourceMapWater39(input);break;
    case 40u:if(g_HasBakedLighting!=0u)output=SourceMapWater40Baked(input);else output=SourceMapWater40(input);break;
    case 41u:if(g_HasBakedLighting!=0u)output=SourceMapWater41Baked(input);else output=SourceMapWater41(input);break;
    case 42u:if(g_HasBakedLighting!=0u)output=SourceMapWater42Baked(input);else output=SourceMapWater42(input);break;
    case 43u:if(g_HasBakedLighting!=0u)output=SourceMapWater43Baked(input);else output=SourceMapWater43(input);break;
    }
    if(output.discarded)discard;
    return output.targets[0];
}

#include "Shader_SourceMapTranslucentPrograms.hlsli"

float4 EvaluateSourceMapTranslucent(float2 uv,float3 worldPosition,float3 tangent,float3 binormal,
    float3 normal,float4 clipPosition,float4 vertexColor,float4 extraUV,float2 lightmapUV,float3 cameraPosition)
{
    SOURCE_CHARACTER_NATIVE_INPUT input=(SOURCE_CHARACTER_NATIVE_INPUT)0;
    const float3 t=SourceCharacterSafeUnit(tangent),b=SourceCharacterSafeUnit(binormal),n=SourceCharacterSafeUnit(normal);
    const float3 view=cameraPosition-worldPosition;
    const float3 tangentView=float3(dot(t,view),dot(b,view),dot(n,view));
    input.values[0]=float4(t.x,b.x,n.x,0.f);
    input.values[1]=float4(t.y,b.y,n.y,dot(cross(t,b),n)<0.f?-1.f:1.f);
    input.values[2]=vertexColor;input.values[3]=float4(0.f,0.f,lightmapUV);
    input.values[4]=float4(uv,extraUV.yx);
    input.values[5]=EvaluateSceneFog(worldPosition,cameraPosition);
    input.values[6]=float4(tangentView,1.f);
    input.values[7]=float4(t.y,b.y,n.y,0.f);
    input.values[8]=clipPosition*100.f;
    if(g_SourceCharacterProgram==47u || g_SourceCharacterProgram==53u || g_SourceCharacterProgram==55u)
        input.values[7]=clipPosition*100.f;
    if(g_SourceCharacterProgram==61u)input.values[5]=float4(extraUV.zw,0.f,0.f);
    const float4x4 vp=mul(g_ViewMatrix,g_ProjMatrix);
    input.projection[0]=vp[0];input.projection[1]=-vp[2];
    input.projection[2]=vp[1];input.projection[3]=vp[3]*100.f;
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    switch(g_SourceCharacterProgram)
    {
    case 44u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent44Baked(input);else output=SourceMapTranslucent44Base(input);break;
    case 45u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent45Baked(input);else output=SourceMapTranslucent45Base(input);break;
    case 46u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent46Baked(input);else output=SourceMapTranslucent46Base(input);break;
    case 47u:output=SourceMapTranslucent47Base(input);break;
    case 48u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent48Baked(input);else output=SourceMapTranslucent48Base(input);break;
    case 49u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent49Baked(input);else output=SourceMapTranslucent49Base(input);break;
    case 50u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent50Baked(input);else output=SourceMapTranslucent50Base(input);break;
    case 51u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent51Baked(input);else output=SourceMapTranslucent51Base(input);break;
    case 52u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent52Baked(input);else output=SourceMapTranslucent52Base(input);break;
    case 53u:output=SourceMapTranslucent53Base(input);break;
    case 54u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent54Baked(input);else output=SourceMapTranslucent54Base(input);break;
    case 55u:output=SourceMapTranslucent55Base(input);break;
    case 56u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent56Baked(input);else output=SourceMapTranslucent56Base(input);break;
    case 57u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent57Baked(input);else output=SourceMapTranslucent57Base(input);break;
    case 58u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent58Baked(input);else output=SourceMapTranslucent58Base(input);break;
    case 59u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent59Baked(input);else output=SourceMapTranslucent59Base(input);break;
    case 60u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent60Baked(input);else output=SourceMapTranslucent60Base(input);break;
    case 61u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent61Baked(input);else output=SourceMapTranslucent61Base(input);break;
    case 62u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent62Baked(input);else output=SourceMapTranslucent62Base(input);break;
    case 63u:if(g_HasBakedLighting!=0u)output=SourceMapTranslucent63Baked(input);else output=SourceMapTranslucent63Base(input);break;
    }
    if(output.discarded)discard;
    float4 color=output.targets[0];
    // Native additive PS already multiplies coverage into radiance.
    if(g_SourceCharacterProgram==47u || g_SourceCharacterProgram==53u || g_SourceCharacterProgram==55u)color.a=1.f;
    return color;
}

#include "Shader_SourceMapDirectPrograms.hlsli"

cbuffer SourceMapForwardLighting
{
    uint g_SourceMapForwardLightCount = 0u;
    float4 g_SourceMapForwardLightPositionRange[400];
    float4 g_SourceMapForwardLightDirectionType[400];
    float4 g_SourceMapForwardLightColorExponent[400];
    float4 g_SourceMapForwardLightConeShadow[400];
};

float3 EvaluateSourceMapDirect(float2 uv,float4 extraUV,float3 worldPosition,float3 tangent,
    float3 binormal,float3 normal,float4 clipPosition,float4 vertexColor,float3 cameraPosition)
{
    if(!(g_SourceCharacterProgram==40u || g_SourceCharacterProgram==41u || g_SourceCharacterProgram==42u || g_SourceCharacterProgram==43u || g_SourceCharacterProgram==44u || g_SourceCharacterProgram==45u || g_SourceCharacterProgram==46u || g_SourceCharacterProgram==48u || g_SourceCharacterProgram==49u || g_SourceCharacterProgram==50u || g_SourceCharacterProgram==51u || g_SourceCharacterProgram==52u || g_SourceCharacterProgram==54u || g_SourceCharacterProgram==56u || g_SourceCharacterProgram==57u || g_SourceCharacterProgram==58u || g_SourceCharacterProgram==59u || g_SourceCharacterProgram==60u || g_SourceCharacterProgram==61u || g_SourceCharacterProgram==62u || g_SourceCharacterProgram==63u))return 0.f;
    const float3 t=SourceCharacterSafeUnit(tangent),b=SourceCharacterSafeUnit(binormal),n=SourceCharacterSafeUnit(normal);
    const float3 view=cameraPosition-worldPosition;
    const float4 tangentView=float4(dot(t,view),dot(b,view),dot(n,view),1.f);
    const float4 sourcePosition=float4(worldPosition.x*100.f,-worldPosition.z*100.f,worldPosition.y*100.f,1.f);
    const float staticShadow=EvaluateMapStaticShadow(extraUV.xy*g_StaticShadowScaleBias.xy+g_StaticShadowScaleBias.zw);
    const float fogTransmission=g_SourceCharacterProgram==61u?1.f:EvaluateSceneFog(worldPosition,cameraPosition).a;
    float3 radiance=0.f;
    [loop]for(uint light=0u;light<g_SourceMapForwardLightCount;++light)
    {
        const float4 directionType=g_SourceMapForwardLightDirectionType[light];
        const float4 colorExponent=g_SourceMapForwardLightColorExponent[light];
        const float4 coneShadow=g_SourceMapForwardLightConeShadow[light];
        float3 direction=-directionType.xyz;
        float attenuation=1.f;
        if(directionType.w>1.f)
        {
            const float4 positionRange=g_SourceMapForwardLightPositionRange[light];
            const float3 delta=positionRange.xyz-worldPosition;
            const float distance=length(delta);
            direction=distance>1e-6f?delta/distance:n;
            attenuation=pow(saturate((positionRange.w-distance)/max(positionRange.w,1e-6f)),colorExponent.w);
            if(directionType.w>2.f)
            {
                const float cone=saturate((dot(-direction,SourceCharacterSafeUnit(directionType.xyz))-coneShadow.y)/max(coneShadow.x-coneShadow.y,.0001f));
                attenuation*=cone*cone;
            }
        }
        direction=SourceCharacterSafeUnit(direction);
        const float4 tangentLight=float4(dot(t,direction),dot(b,direction),dot(n,direction),1.f);
        SOURCE_CHARACTER_NATIVE_INPUT input=(SOURCE_CHARACTER_NATIVE_INPUT)0;
        input.lightColor=colorExponent.rgb;
        const float4x4 vp=mul(g_ViewMatrix,g_ProjMatrix);
        input.projection[0]=vp[0];input.projection[1]=-vp[2];input.projection[2]=vp[1];input.projection[3]=vp[3]*100.f;
        SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
        switch(g_SourceCharacterProgram)
        {
        case 40u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(clipPosition*100.f).xyzw;
            output=SourceMapDirect40(input);break;
        case 41u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect41(input);break;
        case 42u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect42(input);break;
        case 43u:
            input.values[0].xyzw=(float4(t.x,b.x,n.x,0.f)).xyzw;
            input.values[1].xyzw=(float4(t.y,b.y,n.y,dot(cross(t,b),n)<0.f?-1.f:1.f)).xyzw;
            input.values[2].xyzw=(vertexColor).xyzw;
            input.values[3].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[4].xyz=(tangentLight).xyz;
            input.values[5].xyzw=(clipPosition*100.f).xyzw;
            input.values[6].xyzw=(tangentView).xyzw;
            input.values[7].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect43(input);break;
        case 44u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect44(input);break;
        case 45u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect45(input);break;
        case 46u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect46(input);break;
        case 48u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect48(input);break;
        case 49u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect49(input);break;
        case 50u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect50(input);break;
        case 51u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect51(input);break;
        case 52u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect52(input);break;
        case 54u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect54(input);break;
        case 56u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect56(input);break;
        case 57u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect57(input);break;
        case 58u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect58(input);break;
        case 59u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(clipPosition*100.f).xyzw;
            output=SourceMapDirect59(input);break;
        case 60u:
            input.values[0].xyzw=(float4(t.x,b.x,n.x,0.f)).xyzw;
            input.values[1].xyzw=(float4(t.y,b.y,n.y,dot(cross(t,b),n)<0.f?-1.f:1.f)).xyzw;
            input.values[2].xyzw=(vertexColor).xyzw;
            input.values[3].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[4].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[5].xyz=(tangentLight).xyz;
            input.values[6].xyzw=(clipPosition*100.f).xyzw;
            input.values[7].xyzw=(tangentView).xyzw;
            input.values[8].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect60(input);break;
        case 61u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyzw=(float4(extraUV.zw,0.f,0.f)).xyzw;
            input.values[4].xyz=(tangentLight).xyz;
            input.values[5].xyzw=(clipPosition*100.f).xyzw;
            input.values[6].xyzw=(tangentView).xyzw;
            input.values[7].xyzw=(clipPosition*100.f).xyzw;
            output=SourceMapDirect61(input);break;
        case 62u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(clipPosition*100.f).xyzw;
            output=SourceMapDirect62(input);break;
        case 63u:
            input.values[0].xyzw=(vertexColor).xyzw;
            input.values[1].xy=(float4(0.f,0.f,0.f,0.f)).xy;
            input.values[2].xyzw=(float4(uv,extraUV.yx)).xyzw;
            input.values[3].xyz=(tangentLight).xyz;
            input.values[4].xyzw=(clipPosition*100.f).xyzw;
            input.values[5].xyzw=(tangentView).xyzw;
            input.values[6].xyzw=(sourcePosition).xyzw;
            output=SourceMapDirect63(input);break;
        }
        if(!output.discarded)radiance+=output.targets[0].rgb*attenuation*(g_StaticShadowChannel!=0u && uint(round(coneShadow.w))==g_StaticShadowChannel?staticShadow:1.f);
    }
    return radiance*fogTransmission;
}

// Source shadow_mat PS: depth-fade modulation, including the native squared fog transfer.
float SourceMapShadowModulation(float sceneDepthCm, float clipWCm, float fogTransmission,
    float depthBias, float depthPower)
{
    float base = 1.f-saturate((sceneDepthCm-clipWCm)/max(1.f-depthBias,0.001f));
    float strength = base < 0.000001f ? 0.f : exp2(log2(base)*depthPower);
    return fogTransmission*fogTransmission*(saturate(strength)-1.f)+1.f;
}
bool IsSourceMapForward() { return (g_SourceCharacterProgram>=33u && g_SourceCharacterProgram<=63u) || g_SourceCharacterProgram==65u; }
float4 EvaluateSourceMapForward(float2 uv,float3 worldPosition,float3 tangent,float3 binormal,
    float3 normal,float4 clipPosition,float4 vertexColor,float4 extraUV,float2 lightmapUV)
{
    float3 cameraPosition=-mul((float3x3)g_ViewMatrix,g_ViewMatrix[3].xyz);
    if(g_SourceCharacterProgram==65u)
    {
        float2 screenUV=clipPosition.xy/clipPosition.w*float2(.5f,-.5f)+.5f;
        float depth=min(g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,screenUV,0.f).r,.999f);
        float depthScale=1.f/(g_ProjMatrix._43*100.f);
        float depthOffset=g_ProjMatrix._33/(g_ProjMatrix._43*100.f);
        float sceneDepthCm=1.f/(depth*depthScale-depthOffset);
        // Original shadow_mat has bAllowFog=false: supply identity transmittance.
        float factor=SourceMapShadowModulation(sceneDepthCm,clipPosition.w*100.f,
            1.f,
            g_SourceCharacterBaseConstants[3].x,g_SourceCharacterBaseConstants[3].y);
        // Source RGB is achromatic [0,1]; black alpha blending is algebraically identical to modulation.
        return float4(0.f,0.f,0.f,1.f-factor);
    }
    if(g_SourceCharacterProgram>=38u)
    {
        float4 color;
        if(g_SourceCharacterProgram>=44u)color=EvaluateSourceMapTranslucent(uv,worldPosition,tangent,binormal,normal,clipPosition,vertexColor,extraUV,lightmapUV,cameraPosition);
        else color=EvaluateSourceMapWater(uv,worldPosition,tangent,binormal,normal,clipPosition,vertexColor,lightmapUV,cameraPosition);
        color.rgb+=EvaluateSourceMapDirect(uv,extraUV,worldPosition,tangent,binormal,normal,clipPosition,vertexColor,cameraPosition);
        return color;
    }
    SOURCE_CHARACTER_NATIVE_INPUT input=MakeSourceCharacterInput(uv,0.f,worldPosition,tangent,
        binormal,normal,cameraPosition,clipPosition,mul(g_ViewMatrix,g_ProjMatrix),float3(0.f,1.f,0.f),0.f,1.f,true);
    // Native no-lightmap/fog VS layout: shared scene fog transfer, tangent view, world-up.
    float4 tangentView=input.values[5],up=input.values[6];
    input.values[5]=EvaluateSceneFog(worldPosition,cameraPosition);
    input.values[6]=tangentView;
    input.values[7]=g_SourceCharacterProgram==33u?clipPosition*100.f:up;
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    switch(g_SourceCharacterProgram)
    {
    case 33u: output=SourceMapForward33(input);break;
    case 34u: output=SourceMapForward34(input);break;
    case 35u: output=SourceMapForward35(input);break;
    case 36u: output=SourceMapForward36(input);break;
    case 37u: output=SourceMapForward37(input);break;
    }
    if(output.discarded) discard;
    float4 color=output.targets[0];
    // Native light shafts already premultiply coverage into radiance.
    if(g_SourceCharacterProgram==33u) color.a=1.f;
    return color;
}
#endif
