#ifndef EFFECT_DIMENSIONMASTER_V_NATIVE_HLSLI
#define EFFECT_DIMENSIONMASTER_V_NATIVE_HLSLI
#include "Shader_EffectDimensionMasterVNativeShared.hlsli"

float4 VNative52(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[2u];
    source[2] = VNativeAppend(g_VSourceMaterialParameters[0u].wwww,g_VSourceMaterialParameters[1u].xxxx,1u);
    source[3].x = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[3].y = (g_VSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[4].x = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[4].y = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[4].z = (((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))).x;
    source[4].w = (sin(((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0)))).x;
    source[5].x = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))).x;
    source[5].y = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_VSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[5].z = (g_VSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xy, -v2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(v2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 2: mul_sat r0.xy, r0.xyxx, l(3.000000, 3.000000, 0.000000, 0.000000)
    r0.xy = (saturate((r0.xyxx)*(float4(3.000000,3.000000,0.000000,0.000000)))).xy;
    // 3: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 4: mul_sat r0.yz, v2.xxyx, l(0.000000, 3.000000, 3.000000, 0.000000)
    r0.yz = (saturate((v2.xxyx)*(float4(0.000000,3.000000,3.000000,0.000000)))).yz;
    // 5: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 6: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 7: mul r0.x, r0.x, l(1.700000)
    r0.x = ((r0.xxxx)*(float4(1.700000,1.700000,1.700000,1.700000))).x;
    // 8: mul r1.x, v2.x, cb0[2].x
    r1.x = ((v2.xxxx)*(source[2].xxxx)).x;
    // 9: mad r1.z, v2.y, cb0[2].y, cb0[3].z
    r1.z = ((v2.yyyy)*(source[2].yyyy)+(source[3].zzzz)).z;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xzxx, t0.wxyz, s0, l(0.000000)
    r0.yzw = (VNativeSample0((r1.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 11: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 12: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 13: mad r0.yzw, cb0[3].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[3].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 14: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 15: mad r0.w, cb0[4].y, l(0.300000), l(0.700000)
    r0.w = ((source[4].yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(float4(0.700000,0.700000,0.700000,0.700000))).w;
    // 16: mad r1.x, cb0[5].y, l(0.300000), l(0.700000)
    r1.x = ((source[5].yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(float4(0.700000,0.700000,0.700000,0.700000))).x;
    // 17: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 18: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 19: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 20: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 21: mul r0.w, v3.w, cb0[5].z
    r0.w = ((v3.wwww)*(source[5].zzzz)).w;
    // 22: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 23: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 24: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// bfx-d-pa-circ-01-01-dt-ad: f1ee928cd5135a43b5f7ecda979adc4e; selected map 78660aa0098523388cfb3e8846030823e481eab40c423d1a96f4a05c73bfd2e3.
float4 VNative53(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
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
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native device-Z reconstruction: runtime view-Z in source centimetres.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
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

// bfx-d-pa-flar-02-01-ad: 31a56416afc48b48975d5f75c4d52157; selected map 4d77068b2fd78f85fcdac963e7906195871e4e3e5ba7a45f5fe9ea5bb462c047.
float4 VNative54(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[2u];
    source[2].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[2].z = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_VSourceMaterialParameters[0u].wwww).x;
    source[3].x = (g_VSourceMaterialParameters[1u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
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
    // 24: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 25: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 26: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 27: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 28: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 29: mul r1.x, r0.y, l(0.159155)
    r1.x = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))).x;
    // 30: mad r1.y, r0.y, l(0.159155), l(0.500000)
    r1.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 31: mov r2.xz, l(0.500000,0,0.500000,0)
    r2.xz = (float4(0.500000,asfloat(0u),0.500000,asfloat(0u))).xz;
    // 32: mul r2.yw, v4.xxxx, l(0.000000, 0.400000, 0.000000, -0.700000)
    r2.yw = ((v4.xxxx)*(float4(0.000000,0.400000,0.000000,-0.700000))).yw;
    // 33: add r1.xyzw, r1.xyxy, r2.xyzw
    r1.xyzw = ((r1.xyxy)+(r2.xyzw)).xyzw;
    // 34: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t0.wxyz, s1, l(-1.000000)
    r0.yzw = (VNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 35: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.zwzz, t0.xyzw, s1, l(-1.000000)
    r1.xyz = (VNativeSample0((r1.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 36: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 37: add r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)+(r0.yyyy)).y;
    // 38: add r0.z, r1.y, r1.x
    r0.z = ((r1.yyyy)+(r1.xxxx)).z;
    // 39: add r0.z, r1.z, r0.z
    r0.z = ((r1.zzzz)+(r0.zzzz)).z;
    // 40: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 41: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 42: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 43: mul r0.zw, r0.zzzz, cb0[2].xxxz
    r0.zw = ((r0.zzzz)*(source[2].xxxz)).zw;
    // 44: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 45: mul r0.yz, r0.yyzy, cb0[2].wwyw
    r0.yz = ((r0.yyzy)*(source[2].wwyw)).yz;
    // 46: movc r0.w, r0.x, l(0), r0.w
    r0.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 47: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 48: mad r0.x, r0.w, r0.y, r0.x
    r0.x = ((r0.wwww)*(r0.yyyy)+(r0.xxxx)).x;
    // 49: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 50: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native device-Z reconstruction: runtime view-Z in source centimetres.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // 57: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 58: add r0.z, -cb0[3].x, l(1.000000)
    r0.z = ((-(source[3].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 59: max r0.z, -r0.z, l(0.001000)
    r0.z = (max(-(r0.zzzz),float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 60: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 61: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 62: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 63: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 64: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 65: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 66: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 67: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// bfx-j-pa-ring-07-06-ad: 2e9f758287cb5f4eb21139e861d1db8e; selected map 5fb3b5a074b6f07f0ae700b728b14e35adab76cd4529f25b2f6eec2072d92e8a.
float4 VNative55(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[5u];
    source[2] = VNativeAppend(VNativePeriodic(((g_VSourceMaterialParameters[3u].yyyy*g_VSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),VNativePeriodic(((g_VSourceMaterialParameters[3u].yyyy*g_VSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3] = g_VSourceMaterialParameters[4u];
    source[4] = VNativeAppend(VNativePeriodic(((g_VSourceMaterialParameters[1u].zzzz*g_VSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),VNativePeriodic(((g_VSourceMaterialParameters[1u].zzzz*g_VSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[5].y = (g_VSourceMaterialTime.xxxx).x;
    source[5].z = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[5].w = ((g_VSourceMaterialParameters[3u].yyyy*g_VSourceMaterialTime.xxxx)).x;
    source[6].x = (((g_VSourceMaterialParameters[3u].yyyy*g_VSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[6].y = (((g_VSourceMaterialParameters[3u].yyyy*g_VSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[6].z = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[6].w = (VNativePeriodic(((g_VSourceMaterialParameters[3u].yyyy*g_VSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[7].x = (VNativePeriodic(((g_VSourceMaterialParameters[3u].yyyy*g_VSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[7].y = (g_VSourceMaterialParameters[3u].wwww).x;
    source[7].z = (g_VSourceMaterialParameters[1u].wwww).x;
    source[7].w = (g_VSourceMaterialParameters[0u].wwww).x;
    source[8].x = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[0u].xxxx)).x;
    source[9].x = (g_VSourceMaterialParameters[2u].wwww).x;
    source[9].y = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[9].z = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[9].w = ((g_VSourceMaterialParameters[1u].zzzz*g_VSourceMaterialTime.xxxx)).x;
    source[10].x = (((g_VSourceMaterialParameters[1u].zzzz*g_VSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[10].y = (((g_VSourceMaterialParameters[1u].zzzz*g_VSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[10].z = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[10].w = (VNativePeriodic(((g_VSourceMaterialParameters[1u].zzzz*g_VSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[11].x = (VNativePeriodic(((g_VSourceMaterialParameters[1u].zzzz*g_VSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[11].z = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[11].w = (g_VSourceMaterialParameters[2u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
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
    // 30: mad r1.x, cb0[10].z, r0.y, cb0[4].x
    r1.x = ((source[10].zzzz)*(r0.yyyy)+(source[4].xxxx)).x;
    // 31: mad r2.x, cb0[6].z, r0.y, cb0[2].x
    r2.x = ((source[6].zzzz)*(r0.yyyy)+(source[2].xxxx)).x;
    // 32: mul r0.y, v2.x, cb0[7].w
    r0.y = ((v2.xxxx)*(source[7].wwww)).y;
    // 33: mad r3.x, cb0[5].y, cb0[7].z, r0.y
    r3.x = ((source[5].yyyy)*(source[7].zzzz)+(r0.yyyy)).x;
    // 34: mul r0.y, v2.y, cb0[8].x
    r0.y = ((v2.yyyy)*(source[8].xxxx)).y;
    // 35: mad r3.y, cb0[5].y, cb0[8].y, r0.y
    r3.y = ((source[5].yyyy)*(source[8].yyyy)+(r0.yyyy)).y;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r3.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (VNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 37: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 38: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 39: mul r0.z, r0.z, v4.z
    r0.z = ((r0.zzzz)*(v4.zzzz)).z;
    // 40: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 41: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 42: add r0.z, v4.x, l(-1.000000)
    r0.z = ((v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 43: mad r1.y, cb0[11].y, r0.x, r0.z
    r1.y = ((source[11].yyyy)*(r0.xxxx)+(r0.zzzz)).y;
    // 44: mad r2.y, cb0[7].y, r0.x, r0.z
    r2.y = ((source[7].yyyy)*(r0.xxxx)+(r0.zzzz)).y;
    // 45: mad r0.xz, r0.yyyy, cb0[8].wwww, r2.xxyx
    r0.xz = ((r0.yyyy)*(source[8].wwww)+(r2.xxyx)).xz;
    // 46: sample_l_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t3.xwyz, s0, cb0[5].x
    r0.xzw = (VNativeSample0((r0.xzxx).xy, (source[5].xxxx).x, true).xwyz).xzw;
    // 47: mad r1.xy, r0.yyyy, cb0[8].wwww, r1.xyxx
    r1.xy = ((r0.yyyy)*(source[8].wwww)+(r1.xyxx)).xy;
    // 48: mad r1.zw, r0.yyyy, cb0[8].wwww, v2.xxxy
    r1.zw = ((r0.yyyy)*(source[8].wwww)+(v2.xxxy)).zw;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t2.yxzw, s3, l(0.000000)
    r0.y = (VNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 50: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s2, cb0[5].x
    r1.x = (VNativeSample2((r1.xyxx).xy, (source[5].xxxx).x, true).xyzw).x;
    // 51: mul_sat r1.x, r1.x, cb0[11].z
    r1.x = (saturate((r1.xxxx)*(source[11].zzzz))).x;
    // 52: mov_sat r1.y, v4.y
    r1.y = (saturate(v4.yyyy)).y;
    // 53: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 54: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 55: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 56: mul_sat r0.y, r0.y, r1.x
    r0.y = (saturate((r0.yyyy)*(r1.xxxx))).y;
    // 57: log r1.x, r0.y
    r1.x = (log2(r0.yyyy)).x;
    // 58: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 59: mul r1.x, r1.x, cb0[11].w
    r1.x = ((r1.xxxx)*(source[11].wwww)).x;
    // 60: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 61: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 62: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 63: movc r0.y, r0.y, l(0), r1.x
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).y;
    // 64: dp3 r1.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 65: add r1.xyz, -r0.xzwx, r1.xxxx
    r1.xyz = ((-(r0.xzwx))+(r1.xxxx)).xyz;
    // 66: mad r0.xzw, cb0[9].xxxx, r1.xxyz, r0.xxzw
    r0.xzw = ((source[9].xxxx)*(r1.xxyz)+(r0.xxzw)).xzw;
    // 67: max r0.xzw, |r0.xxzw|, l(0.000001, 0.000000, 0.000001, 0.000001)
    r0.xzw = (max(abs(r0.xxzw),float4(0.000001,0.000000,0.000001,0.000001))).xzw;
    // 68: log r0.xzw, r0.xxzw
    r0.xzw = (log2(r0.xxzw)).xzw;
    // 69: mul r0.xzw, r0.xxzw, cb0[9].yyyy
    r0.xzw = ((r0.xxzw)*(source[9].yyyy)).xzw;
    // 70: exp r0.xzw, r0.xxzw
    r0.xzw = (exp2(r0.xxzw)).xzw;
    // 71: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 72: mul r0.xzw, r0.xxzw, r1.xxyz
    r0.xzw = ((r0.xxzw)*(r1.xxyz)).xzw;
    // 73: mad r0.xzw, r0.xxzw, v3.xxyz, cb0[1].xxyz
    r0.xzw = ((r0.xxzw)*(v3.xxyz)+(source[1].xxyz)).xzw;
    // 74: mul r0.xzw, r0.xxzw, v5.wwww
    r0.xzw = ((r0.xxzw)*(v5.wwww)).xzw;
    // 75: mul o0.xyz, r0.yyyy, r0.xzwx
    output.xyz = ((r0.yyyy)*(r0.xzwx)).xyz;
    // 76: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx-a-pa-gl-01-9-ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 VNative56(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
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

// fx-d-pa-atta-05-07-ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 VNative57(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[0u];
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (VNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 3: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 4: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 5: mov_sat r0.w, r0.w
    r0.w = (saturate(r0.wwww)).w;
    // 6: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 7: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 8: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx-d-pa-atta-09-02-ad: 8b4801a9801c444caa83855e137d58f2; selected map 87fda8ea57a1afa755cb1c98d9c5b13a9bb3ea2705562d75a2868daf1f283b11.
float4 VNative58(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[1u];
    source[2].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[2].y = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[2].z = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_VSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.x, cb0[2].x, v4.x, l(-1.000000)
    r0.x = ((source[2].xxxx)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, v4.x, cb0[2].x
    r0.y = ((v4.xxxx)*(source[2].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v2.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v2.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (VNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 7: mul r1.x, r1.x, cb0[2].w
    r1.x = ((r1.xxxx)*(source[2].wwww)).x;
    // 8: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 9: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 10: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 11: movc r0.w, r0.w, l(0), |r1.x|
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).w;
    // 12: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 13: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 14: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 15: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 16: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 17: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 18: mul r1.xyz, r0.xyzx, cb0[2].yyyy
    r1.xyz = ((r0.xyzx)*(source[2].yyyy)).xyz;
    // 19: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 20: mad r0.xyz, -cb0[2].yyyy, r0.xyzx, r1.wwww
    r0.xyz = ((-(source[2].yyyy))*(r0.xyzx)+(r1.wwww)).xyz;
    // 21: mad r0.xyz, cb0[2].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[2].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 22: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 23: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 24: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 25: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx-d-pa-atta-09-04-tr: aa35a091dbe2b648b7dc2944408dbb34; selected map 9bf6e85e604760ca15e93b134bf9e69a2dbdae0e7171239b6d182fc669daeefd.
float4 VNative59(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[3u];
    source[2].x = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[2].y = (g_VSourceMaterialTime.xxxx).x;
    source[2].z = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[2].w = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[3].x = (g_VSourceMaterialParameters[1u].wwww).x;
    source[3].y = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[3].z = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[3].w = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[4].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[4].z = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_VSourceMaterialParameters[1u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f;
    // 1: mul r0.xy, v2.xyxx, cb0[2].zwzz
    r0.xy = ((v2.xyxx)*(source[2].zwzz)).xy;
    // 2: mad r1.x, cb0[2].y, cb0[2].x, r0.x
    r1.x = ((source[2].yyyy)*(source[2].xxxx)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[2].y, cb0[3].x, r0.y
    r1.y = ((source[2].yyyy)*(source[3].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (VNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mad r0.xy, cb0[3].yyyy, r0.xyxx, v2.xyxx
    r0.xy = ((source[3].yyyy)*(r0.xyxx)+(v2.xyxx)).xy;
    // 6: mad r0.z, cb0[3].z, v4.x, l(-1.000000)
    r0.z = ((source[3].zzzz)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 7: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 8: mul r0.w, v4.x, cb0[3].z
    r0.w = ((v4.xxxx)*(source[3].zzzz)).w;
    // 9: mad r0.xy, r0.wwww, r0.xyxx, -r0.zzzz
    r0.xy = ((r0.wwww)*(r0.xyxx)+(-(r0.zzzz))).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (VNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 11: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 12: mul r0.w, r0.w, cb0[4].w
    r0.w = ((r0.wwww)*(source[4].wwww)).w;
    // 13: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 14: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 15: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: movc r0.w, r1.x, l(0), |r0.w|
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.wwww))).w;
    // 17: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 18: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 19: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 20: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 21: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 22: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 23: movc o0.w, r0.w, l(0), r1.x
    output.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 24: mul r1.xyz, r0.xyzx, cb0[3].wwww
    r1.xyz = ((r0.xyzx)*(source[3].wwww)).xyz;
    // 25: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 26: mad r0.xyz, -cb0[3].wwww, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[3].wwww))*(r0.xyzx)+(r0.wwww)).xyz;
    // 27: mad r0.xyz, cb0[4].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[4].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 28: mul r1.xyz, r0.xyzx, cb0[4].yyyy
    r1.xyz = ((r0.xyzx)*(source[4].yyyy)).xyz;
    // 29: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 30: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 31: mul r1.xyz, r1.xyzx, cb0[4].zzzz
    r1.xyz = ((r1.xyzx)*(source[4].zzzz)).xyz;
    // 32: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 33: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 34: add r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)+(r0.wwww)).xyz;
    // 35: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 36: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx-j-me-ringrainbow-01-2-ts-tr: b443bd19596a754c9ef3c33eee6594ed; selected map 81038f264329a0740f30fe1710125822fb77b6bc2459f4a6e3621f04735faf63.
float4 VNative60(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_VSourceMaterialParameters[9u];
    source[3] = g_VSourceMaterialParameters[7u];
    source[4] = input.dynamicParameter;
    source[5] = VNativeAppend(g_VSourceMaterialParameters[4u].yyyy,g_VSourceMaterialParameters[4u].zzzz,1u);
    source[6].x = (g_VSourceMaterialParameters[6u].wwww).x;
    source[6].y = (g_VSourceMaterialTime.xxxx).x;
    source[6].z = (g_VSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_VSourceMaterialParameters[2u].wwww).x;
    source[7].z = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[7].w = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[8].x = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[8].y = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[1u].yyyy)).x;
    source[8].w = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[9].x = (g_VSourceMaterialParameters[6u].yyyy).x;
    source[9].y = (g_VSourceMaterialParameters[6u].zzzz).x;
    source[9].z = (g_VSourceMaterialParameters[0u].wwww).x;
    source[9].w = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[10].x = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_VSourceMaterialParameters[3u].wwww).x;
    source[10].w = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[11].x = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[11].y = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[11].z = (g_VSourceMaterialParameters[4u].wwww).x;
    source[11].w = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[12].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[4u].wwww)).x;
    source[12].y = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[12].z = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[12].w = (g_VSourceMaterialParameters[5u].wwww).x;
    source[13].x = (g_VSourceMaterialParameters[6u].xxxx).x;
    source[13].y = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[13].z = (g_VSourceMaterialParameters[5u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=float4(input.uv,input.uv1.yx);
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
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
    // 10: add r0.y, cb0[4].y, l(-1.000000)
    r0.y = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 11: mad r0.z, cb0[6].y, cb0[11].w, cb0[12].x
    r0.z = ((source[6].yyyy)*(source[11].wwww)+(source[12].xxxx)).z;
    // 12: sincos r1.x, r2.x, r0.z
    r1.x = (sin(r0.zzzz)).x; r2.x = (cos(r0.zzzz)).x;
    // 13: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 14: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 15: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 16: mul r0.zw, v4.xxxy, cb0[7].zzzw
    r0.zw = ((v4.xxxy)*(source[7].zzzw)).zw;
    // 17: mul r1.x, cb0[6].x, cb0[6].y
    r1.x = ((source[6].xxxx)*(source[6].yyyy)).x;
    // 18: mad r2.x, r1.x, cb0[7].y, r0.z
    r2.x = ((r1.xxxx)*(source[7].yyyy)+(r0.zzzz)).x;
    // 19: mad r2.y, r1.x, cb0[8].x, r0.w
    r2.y = ((r1.xxxx)*(source[8].xxxx)+(r0.wwww)).y;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = (VNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 21: mad r0.zw, cb0[8].zzzz, r0.zzzw, v4.xxxy
    r0.zw = ((source[8].zzzz)*(r0.zzzw)+(v4.xxxy)).zw;
    // 22: add r1.yz, r0.zzwz, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((r0.zzwz)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 23: dp2 r1.w, r3.yxyy, r1.yzyy
    r1.w = (dot((r3.yxyy).xy,(r1.yzyy).xy).xxxx).w;
    // 24: dp2 r1.y, r3.zyzz, r1.yzyy
    r1.y = (dot((r3.zyzz).xy,(r1.yzyy).xy).xxxx).y;
    // 25: mul r2.z, r1.y, cb0[5].y
    r2.z = ((r1.yyyy)*(source[5].yyyy)).z;
    // 26: mad r2.x, r1.w, cb0[5].x, r0.y
    r2.x = ((r1.wwww)*(source[5].xxxx)+(r0.yyyy)).x;
    // 27: add r1.yz, r2.xxzx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r1.yz = ((r2.xxzx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.yzyy, t3.xzyw, s4, l(0.000000)
    r0.y = (VNativeSample4((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 29: mul r1.y, r0.w, cb0[11].x
    r1.y = ((r0.wwww)*(source[11].xxxx)).y;
    // 30: mad r2.w, r1.x, cb0[11].y, r1.y
    r2.w = ((r1.xxxx)*(source[11].yyyy)+(r1.yyyy)).w;
    // 31: mul r1.yz, r0.wwzw, cb0[10].xxwx
    r1.yz = ((r0.wwzw)*(source[10].xxwx)).yz;
    // 32: mad r2.yz, r1.xxxx, cb0[10].yyzy, r1.yyzy
    r2.yz = ((r1.xxxx)*(source[10].yyzy)+(r1.yyzy)).yz;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.zwzz, t2.xzyw, s3, l(0.000000)
    r1.y = (VNativeSample3((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 34: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 35: add r1.y, -cb0[4].x, l(1.000000)
    r1.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: mul r1.z, r0.z, cb0[9].w
    r1.z = ((r0.zzzz)*(source[9].wwww)).z;
    // 37: mad r2.x, r1.x, cb0[9].z, r1.z
    r2.x = ((r1.xxxx)*(source[9].zzzz)+(r1.zzzz)).x;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r1.z = (VNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).z;
    // 39: mad r0.y, r1.z, r0.y, -r1.y
    r0.y = ((r1.zzzz)*(r0.yyyy)+(-(r1.yyyy))).y;
    // 40: mul_sat r0.y, r0.y, cb0[12].w
    r0.y = (saturate((r0.yyyy)*(source[12].wwww))).y;
    // 41: log r1.y, r0.y
    r1.y = (log2(r0.yyyy)).y;
    // 42: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 43: mul r1.y, r1.y, cb0[13].x
    r1.y = ((r1.yyyy)*(source[13].xxxx)).y;
    // 44: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 45: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 46: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 47: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 48: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 49: mul r0.x, r0.z, cb0[6].w
    r0.x = ((r0.zzzz)*(source[6].wwww)).x;
    // 50: mad r0.x, r1.x, cb0[6].z, r0.x
    r0.x = ((r1.xxxx)*(source[6].zzzz)+(r0.xxxx)).x;
    // 51: mul r0.z, r1.x, cb0[8].w
    r0.z = ((r1.xxxx)*(source[8].wwww)).z;
    // 52: mad r0.y, cb0[7].x, r0.w, r0.z
    r0.y = ((source[7].xxxx)*(r0.wwww)+(r0.zzzz)).y;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s1, l(0.000000)
    r0.xyz = (VNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 54: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 55: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 56: mad r0.xyz, cb0[9].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[9].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 57: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 58: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 59: mul r0.xyz, r0.xyzx, cb0[9].yyyy
    r0.xyz = ((r0.xyzx)*(source[9].yyyy)).xyz;
    // 60: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 61: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 62: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 63: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 64: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx-j-pa-chromaring-01-ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 VNative61(V_NATIVE_INPUT input) { return VNative57(input); }

// fx-j-pa-circledisort-01-ad: 339dc29a99feed46b8f04c16a8b2c9df; selected map a81cc5151b71c8aae123ffa32a87fe53e7289b6fc87b8724026580cfc2228bdc.
float4 VNative62(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[0u];
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f;
    // 1: mad r0.x, -v4.y, l(0.500000), l(1.000000)
    r0.x = ((-(v4.yyyy))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 2: max r0.x, r0.x, l(0.000010)
    r0.x = (max(r0.xxxx,float4(0.000010,0.000010,0.000010,0.000010))).x;
    // 3: div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
    r0.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.xxxx)).x;
    // 4: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 5: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 6: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 7: add r0.z, -r0.y, l(1.000000)
    r0.z = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 8: mul_sat r0.x, r0.x, r0.z
    r0.x = (saturate((r0.xxxx)*(r0.zzzz))).x;
    // 9: mad r0.z, -r0.y, l(2.000000), l(1.000000)
    r0.z = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 10: mad r0.y, -r0.y, l(1.923077), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(1.923077,1.923077,1.923077,1.923077))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul_sat r0.yz, r0.yyzy, l(0.000000, 9.999998, 100.000099, 0.000000)
    r0.yz = (saturate((r0.yyzy)*(float4(0.000000,9.999998,100.000099,0.000000)))).yz;
    // 12: add r0.xy, -r0.xyxx, r0.zzzz
    r0.xy = ((-(r0.xyxx))+(r0.zzzz)).xy;
    // 13: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 14: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 15: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: mul r0.z, r0.z, l(1.500000)
    r0.z = ((r0.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 17: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 18: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 19: mad_sat r0.x, r0.y, l(0.300000), r0.x
    r0.x = (saturate((r0.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r0.xxxx))).x;
    // 20: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 21: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 22: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 23: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 24: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx-e-pa-ht-18-4-tr: a14d57c96465f346be3437ad63c9ad10; selected map 0716202d132c7fa3cb9c994bb5370a1db2710298fb3e58040e580c23ad3e1c1b.
float4 VNative63(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
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
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
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
    // Native device-Z reconstruction: runtime view-Z in source centimetres.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
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

// fx-j-rgbsplit-01-2-ad: d880c361f16370468cfdac8d4688d393; selected map e84de71c52a818919646772cd3b0a0181fe0758eaae2cdb6c89bc69d21826863.
float4 VNative64(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[1u];
    source[2] = VNativeAppend(g_VSourceMaterialParameters[0u].zzzz,g_VSourceMaterialParameters[0u].wwww,1u);
    source[3].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[3].y = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[3].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[3].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
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

// fx-d-pa-flare-03-ad: d6041fd1cc1ab44bae5b05c296b2d90f; selected map e8e8f2d00fec8e1235140f0675cd799a78a85cba5b1966b75552dfd207c86a98.
float4 VNative65(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[0u];
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: add r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)+(r0.xyxx)).xy;
    // 3: mad r0.z, -|r0.x|, |r0.y|, l(1.000000)
    r0.z = ((-(abs(r0.xxxx)))*(abs(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 4: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 5: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: log r0.y, |r0.z|
    r0.y = (log2(abs(r0.zzzz))).y;
    // 9: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 10: mul r0.y, r0.y, l(800.000000)
    r0.y = ((r0.yyyy)*(float4(800.000000,800.000000,800.000000,800.000000))).y;
    // 11: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 12: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 13: mul r0.z, r0.x, r0.x
    r0.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 14: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 15: mul r0.w, r0.w, l(0.150000)
    r0.w = ((r0.wwww)*(float4(0.150000,0.150000,0.150000,0.150000))).w;
    // 16: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 17: mad r0.x, r0.y, l(2.000000), r0.x
    r0.x = ((r0.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(r0.xxxx)).x;
    // 18: mad r1.yzw, r0.xxxx, v3.xxyz, cb0[1].xxyz
    r1.yzw = ((r0.xxxx)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 19: mul r1.yzw, r1.yyzw, v5.wwww
    r1.yzw = ((r1.yyzw)*(v5.wwww)).yzw;
    // 20: movc r0.xz, r1.xxxx, l(0,0,0,0), r0.wwzw
    r0.xz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwzw)).xz;
    // 21: mad r0.x, r0.z, r0.y, r0.x
    r0.x = ((r0.zzzz)*(r0.yyyy)+(r0.xxxx)).x;
    // 22: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 23: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 24: mul o0.xyz, r0.xxxx, r1.yzwy
    output.xyz = ((r0.xxxx)*(r1.yzwy)).xyz;
    // 25: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx-j-me-localcrack-01-07-tr: 8b228c7b319b544781cca408746673a2; selected map cb7fea6335f99773642abba576c9c60a2c351f25a720c8f239556eb64c8886fd.
float4 VNative66(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
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

// fx-j-pa-slice-01-07-tr: 12959b8a47f91c4dab8a19b9de0871ed; selected map 2695da93e1927eeaf690fdee512c9461bd4fe3fbbb9bcadcc0d1d610f14a68fa.
float4 VNative67(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
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
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
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
    // Native device-Z reconstruction: runtime view-Z in source centimetres.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
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

// fx-j-po-rgbnoise-01-01-tr: 9f4cdbbbab89f745927fe4d13bb5e5a6; selected map 55cfde56e7c9a9e99fe1223b194f3b844d6179733dd9d8b1ed3a88c6ba451164.
float4 VNative68(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[3u];
    source[2].x = (g_VSourceMaterialParameters[1u].wwww).x;
    source[2].y = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[2].z = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[3].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[3].z = (g_VSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[4].x = (g_VSourceMaterialParameters[1u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: mov r1.z, l(1.000000)
    r1.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 4: mul r1.xy, v4.xyxx, cb0[2].xyxx
    r1.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 5: mul r0.z, r1.y, cb0[3].x
    r0.z = ((r1.yyyy)*(source[3].xxxx)).z;
    // 6: mad r0.zw, r0.zzzz, r1.xxxz, r0.xxxy
    r0.zw = ((r0.zzzz)*(r1.xxxz)+(r0.xxxy)).zw;
    // 7: sample_indexable(texture2d)(float,float,float,float) r2.z, r0.zwzz, t1.xyzw, s1 (native engine SceneColor)
    r2.z = (Read_EffectSceneColorBias(LinearClampUVSampler, (r0.zwzz).xy, (0.f).x).xyzw).z;
    // 8: mul r0.zw, r1.yyyy, cb0[2].zzzw
    r0.zw = ((r1.yyyy)*(source[2].zzzw)).zw;
    // 9: mad r1.xyzw, r0.zzww, r1.xzxz, r0.xyxy
    r1.xyzw = ((r0.zzww)*(r1.xzxz)+(r0.xyxy)).xyzw;
    // 12: sample_indexable(texture2d)(float,float,float,float) r2.x, r1.xyxx, t1.xyzw, s1 (native engine SceneColor)
    r2.x = (Read_EffectSceneColorBias(LinearClampUVSampler, (r1.xyxx).xy, (0.f).x).xyzw).x;
    // 13: sample_indexable(texture2d)(float,float,float,float) r2.y, r1.zwzz, t1.xyzw, s1 (native engine SceneColor)
    r2.y = (Read_EffectSceneColorBias(LinearClampUVSampler, (r1.zwzz).xy, (0.f).x).xyzw).y;
    // 14: mul r0.yzw, r2.xxyz, v3.xxyz
    r0.yzw = ((r2.xxyz)*(v3.xxyz)).yzw;
    // 15: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 16: mad r1.xyz, -r2.xyzx, v3.xyzx, r1.xxxx
    r1.xyz = ((-(r2.xyzx))*(v3.xyzx)+(r1.xxxx)).xyz;
    // 17: mad r0.yzw, cb0[3].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[3].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 18: add r0.yzw, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)+(source[1].xxyz)).yzw;
    // 19: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // Native device-Z reconstruction: runtime view-Z in source centimetres.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // 24: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 25: add r0.y, -cb0[4].x, l(1.000000)
    r0.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 26: mul r0.y, r0.y, l(100.000000)
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 27: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 28: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 29: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 30: mul r0.yz, r0.yyzy, r0.yyzy
    r0.yz = ((r0.yyzy)*(r0.yyzy)).yz;
    // 31: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 32: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 33: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 34: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 35: mul r0.w, v4.z, cb0[3].z
    r0.w = ((v4.zzzz)*(source[3].zzzz)).w;
    // 36: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 37: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 38: mul_sat r0.z, r0.z, cb0[3].w
    r0.z = (saturate((r0.zzzz)*(source[3].wwww))).z;
    // 39: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 40: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 41: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 42: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}

// fx-o-pa-splitline-02-ad: 5dfee80075c74444bffc1d810344820c; selected map da50296bbd6ee194d7cd0a2c358d3a3d553a709ab7fefd00d27b19c9923cc3a3.
float4 VNative69(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[0u];
    source[2] = VNativeAppend(VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),VNativePeriodic((g_VSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
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
    // 10: mad r1.xy, v2.xyxx, l(0.000000, 0.700000, 0.000000, 0.000000), cb0[2].xyxx
    r1.xy = ((v2.xyxx)*(float4(0.000000,0.700000,0.000000,0.000000))+(source[2].xyxx)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s1, l(0.000000)
    r0.w = (VNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 12: add r0.w, r0.w, v4.z
    r0.w = ((r0.wwww)+(v4.zzzz)).w;
    // 13: round_ni r0.w, r0.w
    r0.w = (floor(r0.wwww)).w;
    // 14: mul r0.w, r0.w, v2.y
    r0.w = ((r0.wwww)*(v2.yyyy)).w;
    // 15: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 16: mul o0.w, r0.w, cb0[0].x
    output.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 17: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 18: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: add r0.w, v2.x, l(-0.500000)
    r0.w = ((v2.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 20: add r1.w, |r0.w|, |r0.w|
    r1.w = ((abs(r0.wwww))+(abs(r0.wwww))).w;
    // 21: lt r0.w, |r0.w|, l(0.000000)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 22: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 23: mul r1.w, r1.w, l(10.000000)
    r1.w = ((r1.wwww)*(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 24: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 25: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 26: mul r2.xy, r0.wwww, l(0.350000, 0.035000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.350000,0.035000,0.000000,0.000000))).xy;
    // 27: mad r2.zw, r0.wwww, l(0.000000, 0.000000, 0.017500, 0.017500), r1.xxxy
    r2.zw = ((r0.wwww)*(float4(0.000000,0.000000,0.017500,0.017500))+(r1.xxxy)).zw;
    // 28: mad r0.w, v4.x, l(0.010000), r2.y
    r0.w = ((v4.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))+(r2.yyyy)).w;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.zwzz, t1.xyzw, s0, l(0.000000) (native engine SceneColor)
    r3.xyz = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 30: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 31: mul r1.xy, r1.xyxx, l(0.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(0.000000,1.000000,0.000000,0.000000))).xy;
    // 32: mul r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 33: mov r4.xy, r2.wzww
    r4.xy = (r2.wzww).xy;
    // 34: mov r5.x, r3.x
    r5.x = (r3.xxxx).x;
    // 35: mov r5.y, l(0)
    r5.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 36: loop
    [loop] while (true) {
    // 37: ge r1.w, r5.y, l(3.000000)
    r1.w = (asfloat((uint4)((r5.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 38: breakc_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) break;
    // 39: mad r4.xy, -r1.yxyy, r0.wwww, r4.xyxx
    r4.xy = ((-(r1.yxyy))*(r0.wwww)+(r4.xyxx)).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r4.yxyy, t1.yzwx, s0, l(0.000000) (native engine SceneColor)
    r1.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r4.yxyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzwx).w;
    // 41: add r5.x, r1.w, r5.x
    r5.x = ((r1.wwww)+(r5.xxxx)).x;
    // 42: add r5.y, r5.y, l(1.000000)
    r5.y = ((r5.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 43: endloop
    }
    // 44: mov r2.w, r2.z
    r2.w = (r2.zzzz).w;
    // 45: mov r2.y, r4.x
    r2.y = (r4.xxxx).y;
    // 46: mov r6.x, r3.y
    r6.x = (r3.yyyy).x;
    // 47: mov r6.y, l(0)
    r6.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 48: loop
    [loop] while (true) {
    // 49: ge r1.w, r6.y, l(3.000000)
    r1.w = (asfloat((uint4)((r6.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 50: breakc_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) break;
    // 51: mad r2.yw, -r1.yyyx, r0.wwww, r2.yyyw
    r2.yw = ((-(r1.yyyx))*(r0.wwww)+(r2.yyyw)).yw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.wyww, t1.xzwy, s0, l(0.000000) (native engine SceneColor)
    r1.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.wyww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzwy).w;
    // 53: add r6.x, r1.w, r6.x
    r6.x = ((r1.wwww)+(r6.xxxx)).x;
    // 54: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 55: endloop
    }
    // 56: mov r5.y, r6.x
    r5.y = (r6.xxxx).y;
    // 57: mov r3.xy, r2.zyzz
    r3.xy = (r2.zyzz).xy;
    // 58: mov r4.x, r3.z
    r4.x = (r3.zzzz).x;
    // 59: mov r4.y, l(0)
    r4.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 60: loop
    [loop] while (true) {
    // 61: ge r1.w, r4.y, l(3.000000)
    r1.w = (asfloat((uint4)((r4.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 62: breakc_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) break;
    // 63: mad r3.xy, -r1.xyxx, r0.wwww, r3.xyxx
    r3.xy = ((-(r1.xyxx))*(r0.wwww)+(r3.xyxx)).xy;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.xyxx, t1.xywz, s0, l(0.000000) (native engine SceneColor)
    r1.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xywz).w;
    // 65: add r4.x, r1.w, r4.x
    r4.x = ((r1.wwww)+(r4.xxxx)).x;
    // 66: add r4.y, r4.y, l(1.000000)
    r4.y = ((r4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 67: endloop
    }
    // 68: mov r5.z, r4.x
    r5.z = (r4.xxxx).z;
    // 69: mul r1.xyw, r5.xyxz, l(0.250000, 0.250000, 0.000000, 0.250000)
    r1.xyw = ((r5.xyxz)*(float4(0.250000,0.250000,0.000000,0.250000))).xyw;
    // 70: dp3 r0.w, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 71: mad r2.yzw, -r5.xxyz, l(0.000000, 0.250000, 0.250000, 0.250000), r0.wwww
    r2.yzw = ((-(r5.xxyz))*(float4(0.000000,0.250000,0.250000,0.250000))+(r0.wwww)).yzw;
    // 72: mad r1.xyw, v4.yyyy, r2.yzyw, r1.xyxw
    r1.xyw = ((v4.yyyy)*(r2.yzyw)+(r1.xyxw)).xyw;
    // 73: lt r0.w, |v2.y|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(v2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 74: log r2.y, |v2.y|
    r2.y = (log2(abs(v2.yyyy))).y;
    // 75: mul r2.y, r2.y, l(15.000000)
    r2.y = ((r2.yyyy)*(float4(15.000000,15.000000,15.000000,15.000000))).y;
    // 76: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 77: movc r2.y, r0.w, l(0), r2.y
    r2.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).y;
    // 78: mul r2.z, |v2.y|, |v2.y|
    r2.z = ((abs(v2.yyyy))*(abs(v2.yyyy))).z;
    // 79: mul r2.z, r2.z, |v2.y|
    r2.z = ((r2.zzzz)*(abs(v2.yyyy))).z;
    // 80: movc r0.w, r0.w, l(0), r2.z
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).w;
    // 81: mad r0.w, r0.w, r2.x, r2.y
    r0.w = ((r0.wwww)*(r2.xxxx)+(r2.yyyy)).w;
    // 82: add r1.xyw, r0.wwww, r1.xyxw
    r1.xyw = ((r0.wwww)+(r1.xyxw)).xyw;
    // 83: mad r1.xyw, v3.xyxz, r1.xyxw, cb0[1].xyxz
    r1.xyw = ((v3.xyxz)*(r1.xyxw)+(source[1].xyxz)).xyw;
    // 84: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx-r-me-ringmaster-11-01-ts-fs-ad: ad27c0e7823a39499e6abca02d58a813; selected map 398fd3e3b79be9473ee1af419c6f65dc4bb538d315925c3ad7759e39c7e75c87.
float4 VNative70(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_VSourceMaterialParameters[11u];
    source[3] = VNativeAppend(g_VSourceMaterialParameters[2u].xxxx,g_VSourceMaterialParameters[2u].zzzz,1u);
    source[4] = VNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[5] = VNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[6] = input.dynamicParameter;
    source[7] = VNativeAppend(g_VSourceMaterialParameters[5u].yyyy,g_VSourceMaterialParameters[5u].zzzz,1u);
    source[8] = g_VSourceMaterialParameters[8u];
    source[9] = g_VSourceMaterialParameters[9u];
    source[10].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[0u].xxxx))).x;
    source[10].y = (g_VSourceMaterialParameters[0u].wwww).x;
    source[10].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].wwww)).x;
    source[10].w = (max((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[11].x = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[11].z = (g_VSourceMaterialParameters[5u].xxxx).x;
    source[11].w = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_VSourceMaterialParameters[5u].xxxx)).x;
    source[12].x = (g_VSourceMaterialParameters[4u].wwww).x;
    source[12].y = (g_VSourceMaterialParameters[7u].zzzz).x;
    source[12].z = (g_VSourceMaterialTime.xxxx).x;
    source[12].w = (g_VSourceMaterialParameters[3u].wwww).x;
    source[13].x = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[13].y = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[13].z = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[13].w = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[14].x = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[14].y = (g_VSourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_VSourceMaterialParameters[5u].yyyy).x;
    source[14].w = (g_VSourceMaterialParameters[5u].wwww).x;
    source[15].x = (g_VSourceMaterialParameters[1u].wwww).x;
    source[15].y = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[15].z = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[15].w = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[16].x = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[16].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[16].z = (g_VSourceMaterialParameters[2u].wwww).x;
    source[16].w = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[17].x = (g_VSourceMaterialParameters[6u].zzzz).x;
    source[17].y = (g_VSourceMaterialParameters[6u].wwww).x;
    source[17].z = (g_VSourceMaterialParameters[7u].xxxx).x;
    source[17].w = (g_VSourceMaterialParameters[7u].yyyy).x;
    source[18].x = (g_VSourceMaterialParameters[6u].xxxx).x;
    source[18].y = (g_VSourceMaterialParameters[6u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=float4(input.uv,input.uv1.yx);
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
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
    // 25: mul r1.xy, v4.xyxx, cb0[13].xyxx
    r1.xy = ((v4.xyxx)*(source[13].xyxx)).xy;
    // 26: mul r0.w, cb0[12].y, cb0[12].z
    r0.w = ((source[12].yyyy)*(source[12].zzzz)).w;
    // 27: mad r2.x, r0.w, cb0[12].w, r1.x
    r2.x = ((r0.wwww)*(source[12].wwww)+(r1.xxxx)).x;
    // 28: mad r2.y, r0.w, cb0[13].z, r1.y
    r2.y = ((r0.wwww)*(source[13].zzzz)+(r1.yyyy)).y;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (VNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 30: mul r1.z, cb0[6].y, cb0[13].w
    r1.z = ((source[6].yyyy)*(source[13].wwww)).z;
    // 31: mul r2.xy, r1.xyxx, r1.zzzz
    r2.xy = ((r1.xyxx)*(r1.zzzz)).xy;
    // 32: mad r3.x, r0.z, l(0.159155), r2.x
    r3.x = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(r2.xxxx)).x;
    // 33: mov r2.z, l(0.500000)
    r2.z = (float4(0.500000,0.500000,0.500000,0.500000)).z;
    // 34: mul r0.z, cb0[6].z, cb0[14].x
    r0.z = ((source[6].zzzz)*(source[14].xxxx)).z;
    // 35: dp2 r1.w, r0.xyxx, r0.xyxx
    r1.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 36: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 37: add r2.x, r1.w, r1.w
    r2.x = ((r1.wwww)+(r1.wwww)).x;
    // 38: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 39: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 40: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 41: lt r2.x, r1.w, l(0.000000)
    r2.x = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 42: mad r1.w, -r1.w, cb0[10].x, l(1.000000)
    r1.w = ((-(r1.wwww))*(source[10].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul_sat r1.w, r1.w, cb0[11].x
    r1.w = (saturate((r1.wwww)*(source[11].xxxx))).w;
    // 44: movc r3.y, r2.x, l(0), r0.z
    r3.y = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 45: add r2.xy, r2.zyzz, r3.xyxx
    r2.xy = ((r2.zyzz)+(r3.xyxx)).xy;
    // 46: mul r3.x, r2.x, cb0[7].x
    r3.x = ((r2.xxxx)*(source[7].xxxx)).x;
    // 47: mad r4.y, r2.y, cb0[7].y, cb0[6].x
    r4.y = ((r2.yyyy)*(source[7].yyyy)+(source[6].xxxx)).y;
    // 48: mul r2.xy, r2.xyxx, cb0[15].yzyy
    r2.xy = ((r2.xyxx)*(source[15].yzyy)).xy;
    // 49: mad r0.zw, r0.wwww, cb0[15].xxxw, r2.xxxy
    r0.zw = ((r0.wwww)*(source[15].xxxw)+(r2.xxxy)).zw;
    // 50: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t3.xyzw, s3, l(-1.000000)
    r2.xyz = (VNativeSample3((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 51: mul r4.x, cb0[12].z, cb0[14].w
    r4.x = ((source[12].zzzz)*(source[14].wwww)).x;
    // 52: mov r3.z, l(-1.000000)
    r3.z = (float4(-1.000000,-1.000000,-1.000000,-1.000000)).z;
    // 53: add r0.zw, r3.xxxz, r4.xxxy
    r0.zw = ((r3.xxxz)+(r4.xxxy)).zw;
    // 54: mul r0.zw, r0.zzzw, cb0[12].yyyy
    r0.zw = ((r0.zzzw)*(source[12].yyyy)).zw;
    // 55: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.xyzw, s2, l(-1.000000)
    r0.z = (VNativeSample2((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).z;
    // 56: dp2 r3.x, cb0[4].xyxx, r0.xyxx
    r3.x = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 57: dp2 r3.y, cb0[5].xyxx, r0.xyxx
    r3.y = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 58: mad r0.xy, cb0[12].xxxx, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((source[12].xxxx)*(r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 59: mad r0.xy, r1.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((r1.zzzz)*(r1.xyxx)+(r0.xyxx)).xy;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.zxyw, s1, l(0.000000)
    r0.x = (VNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 61: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 62: log r0.y, r1.w
    r0.y = (log2(r1.wwww)).y;
    // 63: lt r0.z, r1.w, l(0.000001)
    r0.z = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 64: mul r0.y, r0.y, cb0[11].y
    r0.y = ((r0.yyyy)*(source[11].yyyy)).y;
    // 65: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 66: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 67: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 68: add r0.y, r2.y, r2.x
    r0.y = ((r2.yyyy)+(r2.xxxx)).y;
    // 69: add r0.y, r2.z, r0.y
    r0.y = ((r2.zzzz)+(r0.yyyy)).y;
    // 70: mad r0.y, r0.y, l(0.333330), -r0.x
    r0.y = ((r0.yyyy)*(float4(0.333330,0.333330,0.333330,0.333330))+(-(r0.xxxx))).y;
    // 71: mad r0.y, cb0[16].x, r0.y, r0.x
    r0.y = ((source[16].xxxx)*(r0.yyyy)+(r0.xxxx)).y;
    // 72: mul_sat r0.x, r0.x, cb0[17].z
    r0.x = (saturate((r0.xxxx)*(source[17].zzzz))).x;
    // 73: mul r0.y, r0.y, cb0[16].y
    r0.y = ((r0.yyyy)*(source[16].yyyy)).y;
    // 74: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 75: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 76: mul r1.xyz, r0.zzzz, v6.xyzx
    r1.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // 77: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,-0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 78: mad r0.yz, r0.zzwz, cb0[3].xxyx, r0.yyyy
    r0.yz = ((r0.zzwz)*(source[3].xxyx)+(r0.yyyy)).yz;
    // 79: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t4.wxyz, s4, l(0.000000)
    r0.yzw = (VNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 80: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 81: add r1.xyw, -r0.yzyw, r1.xxxx
    r1.xyw = ((-(r0.yzyw))+(r1.xxxx)).xyw;
    // 82: mad r0.yzw, cb0[16].zzzz, r1.xxyw, r0.yyzw
    r0.yzw = ((source[16].zzzz)*(r1.xxyw)+(r0.yyzw)).yzw;
    // 83: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 84: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 85: mul r0.yzw, r0.yyzw, cb0[16].wwww
    r0.yzw = ((r0.yyzw)*(source[16].wwww)).yzw;
    // 86: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 87: mul r1.xyw, cb0[8].xyxz, cb0[8].wwww
    r1.xyw = ((source[8].xyxz)*(source[8].wwww)).xyw;
    // 88: mul r0.yzw, r0.yyzw, r1.xxyw
    r0.yzw = ((r0.yyzw)*(r1.xxyw)).yzw;
    // 89: dp3 r1.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 90: add r1.xyw, -r2.xyxz, r1.xxxx
    r1.xyw = ((-(r2.xyxz))+(r1.xxxx)).xyw;
    // 91: mad r1.xyw, cb0[17].xxxx, r1.xyxw, r2.xyxz
    r1.xyw = ((source[17].xxxx)*(r1.xyxw)+(r2.xyxz)).xyw;
    // 92: max r1.xyw, |r1.xyxw|, l(0.000001, 0.000001, 0.000000, 0.000001)
    r1.xyw = (max(abs(r1.xyxw),float4(0.000001,0.000001,0.000000,0.000001))).xyw;
    // 93: log r1.xyw, r1.xyxw
    r1.xyw = (log2(r1.xyxw)).xyw;
    // 94: mul r1.xyw, r1.xyxw, cb0[17].yyyy
    r1.xyw = ((r1.xyxw)*(source[17].yyyy)).xyw;
    // 95: exp r1.xyw, r1.xyxw
    r1.xyw = (exp2(r1.xyxw)).xyw;
    // 96: mul r2.xyz, cb0[9].xyzx, cb0[9].wwww
    r2.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 97: mul r1.xyw, r1.xyxw, r2.xyxz
    r1.xyw = ((r1.xyxw)*(r2.xyxz)).xyw;
    // 98: add r2.x, -|r1.z|, l(1.000000)
    r2.x = ((-(abs(r1.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 99: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 100: mad r0.yzw, r2.xxxx, r0.yyzw, r1.xxyw
    r0.yzw = ((r2.xxxx)*(r0.yyzw)+(r1.xxyw)).yzw;
    // 101: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 102: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 103: log r1.x, |r1.z|
    r1.x = (log2(abs(r1.zzzz))).x;
    // 104: lt r1.y, |r1.z|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 105: mul r1.x, r1.x, cb0[18].x
    r1.x = ((r1.xxxx)*(source[18].xxxx)).x;
    // 106: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 107: mul_sat r1.x, r1.x, cb0[18].y
    r1.x = (saturate((r1.xxxx)*(source[18].yyyy))).x;
    // 108: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 109: log r1.y, r0.x
    r1.y = (log2(r0.xxxx)).y;
    // 110: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 111: mul r1.y, r1.y, cb0[17].w
    r1.y = ((r1.yyyy)*(source[17].wwww)).y;
    // 112: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 113: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 114: mul_sat r1.x, r1.x, cb0[1].w
    r1.x = (saturate((r1.xxxx)*(source[1].wwww))).x;
    // 115: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 116: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 117: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 118: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx-r-pa-customparticle-02-01-ad: 19b7489e330ace469d3d823a77aa3f33; selected map 3b7e495871ca0cf0bbc4c5750c2e9ca6c33de7d767139aa91be8cda46ed2a09e.
float4 VNative71(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
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
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
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

// fx-r-pa-ri-04-02-ad: 22fe1d84b183a24ba061efbe70ff88fe; selected map dcb4af1b0c51e126ebfcd50d3245e9fb4ef45276b0b47937288bd666ab550511.
float4 VNative72(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[1u];
    source[2].x = (g_VSourceMaterialTime.xxxx).x;
    source[2].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (VNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 2: max r1.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 3: mul r0.xyz, r0.xyzx, cb0[2].yyyy
    r0.xyz = ((r0.xyzx)*(source[2].yyyy)).xyz;
    // 4: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 5: mul r1.xyz, r1.xyzx, l(0.700000, 0.700000, 0.700000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.700000,0.700000,0.700000,0.000000))).xyz;
    // 6: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 7: mul r1.xyz, r1.xyzx, l(50.000000, 50.000000, 50.000000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(50.000000,50.000000,50.000000,0.000000))).xyz;
    // 8: min r1.xyz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 9: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 10: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 11: mul r0.w, r0.w, v4.w
    r0.w = ((r0.wwww)*(v4.wwww)).w;
    // 12: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 13: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 14: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 15: mul r1.x, v4.x, cb0[2].x
    r1.x = ((v4.xxxx)*(source[2].xxxx)).x;
    // 16: mul r1.x, r1.x, l(6.283185)
    r1.x = ((r1.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 17: sincos r1.x, null, r1.x
    r1.x = (sin(r1.xxxx)).x;
    // 18: add r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)+(v4.yyyy)).x;
    // 19: mul r1.x, r1.x, v4.z
    r1.x = ((r1.xxxx)*(v4.zzzz)).x;
    // 20: mul r1.xyz, r0.xyzx, r1.xxxx
    r1.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 21: mad r0.xyz, cb0[2].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[2].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 22: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 23: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 24: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 25: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx-r-pa-ringmaster-12-03-dt-ad: 23890624d29cb948ab5ccb99c77741e0; selected map e2565be9f9dd8de83225ac664781c9092d69efc2aa72d2cc7976e0d6672d28cf.
float4 VNative73(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[6u];
    source[2] = g_VSourceMaterialParameters[5u];
    source[3].x = (g_VSourceMaterialParameters[4u].wwww).x;
    source[3].y = (g_VSourceMaterialTime.xxxx).x;
    source[3].z = (g_VSourceMaterialParameters[3u].yyyy).x;
    source[3].w = (g_VSourceMaterialParameters[2u].wwww).x;
    source[4].x = (g_VSourceMaterialParameters[3u].xxxx).x;
    source[4].y = (g_VSourceMaterialParameters[2u].zzzz).x;
    source[4].z = (g_VSourceMaterialParameters[3u].zzzz).x;
    source[4].w = (g_VSourceMaterialParameters[1u].yyyy).x;
    source[5].x = (g_VSourceMaterialParameters[0u].wwww).x;
    source[5].y = (g_VSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_VSourceMaterialParameters[1u].zzzz).x;
    source[5].w = (g_VSourceMaterialParameters[3u].wwww).x;
    source[6].x = (g_VSourceMaterialParameters[4u].xxxx).x;
    source[6].y = (g_VSourceMaterialParameters[1u].wwww).x;
    source[6].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[1u].wwww)).x;
    source[6].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[1u].wwww))).x;
    source[7].x = (g_VSourceMaterialParameters[2u].xxxx).x;
    source[7].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[2u].xxxx)).x;
    source[7].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[2u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[7].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[2u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[8].x = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[8].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[0u].yyyy)).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_VSourceMaterialParameters[0u].yyyy))).x;
    source[8].w = (g_VSourceMaterialParameters[0u].zzzz).x;
    source[9].x = ((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].zzzz)).x;
    source[9].y = (max((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].z = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_VSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_VSourceMaterialParameters[2u].yyyy).x;
    source[10].x = (g_VSourceMaterialParameters[4u].yyyy).x;
    source[10].y = (g_VSourceMaterialParameters[4u].zzzz).x;
    source[10].z = (g_VSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
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
    // 28: mul r0.y, r1.x, cb0[3].w
    r0.y = ((r1.xxxx)*(source[3].wwww)).y;
    // 29: mul r0.z, cb0[3].x, cb0[3].y
    r0.z = ((source[3].xxxx)*(source[3].yyyy)).z;
    // 30: mad r2.x, r0.z, cb0[3].z, r0.y
    r2.x = ((r0.zzzz)*(source[3].zzzz)+(r0.yyyy)).x;
    // 31: mul r0.y, v4.z, cb0[4].y
    r0.y = ((v4.zzzz)*(source[4].yyyy)).y;
    // 32: add r0.w, r0.x, r0.x
    r0.w = ((r0.xxxx)+(r0.xxxx)).w;
    // 33: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 34: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 35: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 36: lt r0.w, r0.x, l(0.000000)
    r0.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 37: movc r1.y, r0.w, l(0), r0.y
    r1.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 38: mul r0.y, r0.z, cb0[4].z
    r0.y = ((r0.zzzz)*(source[4].zzzz)).y;
    // 39: mad r2.y, cb0[4].x, r1.y, r0.y
    r2.y = ((source[4].xxxx)*(r1.yyyy)+(r0.yyyy)).y;
    // 40: mul r0.yw, r1.xxxy, cb0[5].xxxy
    r0.yw = ((r1.xxxy)*(source[5].xxxy)).yw;
    // 41: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t1.xyzw, s1, l(-1.000000)
    r1.xyz = (VNativeSample0((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 42: mad r2.x, r0.z, cb0[4].w, r0.y
    r2.x = ((r0.zzzz)*(source[4].wwww)+(r0.yyyy)).x;
    // 43: mad r2.y, r0.z, cb0[5].z, r0.w
    r2.y = ((r0.zzzz)*(source[5].zzzz)+(r0.wwww)).y;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r2.xyxx, t2.wxyz, s2, l(-1.000000)
    r0.yzw = (VNativeSample1((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 45: mul r2.xyz, r0.yzwy, r1.xyzx
    r2.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 46: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 47: mad r0.yzw, -r1.xxyz, r0.yyzw, r1.wwww
    r0.yzw = ((-(r1.xxyz))*(r0.yyzw)+(r1.wwww)).yzw;
    // 48: mad r0.yzw, cb0[5].wwww, r0.yyzw, r2.xxyz
    r0.yzw = ((source[5].wwww)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 49: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 50: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 51: mul r0.yzw, r0.yyzw, cb0[6].xxxx
    r0.yzw = ((r0.yyzw)*(source[6].xxxx)).yzw;
    // 52: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 53: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 54: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 55: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 56: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 57: mad r1.x, -r0.x, cb0[6].w, l(1.000000)
    r1.x = ((-(r0.xxxx))*(source[6].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 58: mad r0.x, -r0.x, cb0[8].z, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[8].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 59: mul_sat r0.x, r0.x, cb0[9].z
    r0.x = (saturate((r0.xxxx)*(source[9].zzzz))).x;
    // 60: mul_sat r1.x, r1.x, cb0[7].w
    r1.x = (saturate((r1.xxxx)*(source[7].wwww))).x;
    // 61: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 62: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 63: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 64: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 65: mul r0.x, r0.x, cb0[9].w
    r0.x = ((r0.xxxx)*(source[9].wwww)).x;
    // 66: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 67: mul_sat r0.x, r0.x, cb0[10].x
    r0.x = (saturate((r0.xxxx)*(source[10].xxxx))).x;
    // 68: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 69: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 70: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 71: mul r0.x, r0.x, cb0[10].y
    r0.x = ((r0.xxxx)*(source[10].yyyy)).x;
    // 72: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 73: div r1.yz, v7.xxyx, v7.wwww
    r1.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 74: mad r1.yz, r1.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r1.yz = ((r1.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native device-Z reconstruction: runtime view-Z in source centimetres.
    r1.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.yzyy).xy, 0.f).y * 100000.f;
    // 81: add r1.y, r1.y, -v7.w
    r1.y = ((r1.yyyy)+(-(v7.wwww))).y;
    // 82: add r1.z, -cb0[10].z, l(1.000000)
    r1.z = ((-(source[10].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 83: max r1.z, -r1.z, l(0.001000)
    r1.z = (max(-(r1.zzzz),float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 84: div_sat r1.y, r1.y, r1.z
    r1.y = (saturate((r1.yyyy)/(r1.zzzz))).y;
    // 85: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 86: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 87: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 88: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 89: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 90: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx-r-pa-ringmaster-12-04-dt-ad: 23890624d29cb948ab5ccb99c77741e0; selected map e2565be9f9dd8de83225ac664781c9092d69efc2aa72d2cc7976e0d6672d28cf.
float4 VNative74(V_NATIVE_INPUT input) { return VNative73(input); }

// fx-r-pa-slice-01-02-tr: 12959b8a47f91c4dab8a19b9de0871ed; selected map 2695da93e1927eeaf690fdee512c9461bd4fe3fbbb9bcadcc0d1d610f14a68fa.
float4 VNative75(V_NATIVE_INPUT input) { return VNative67(input); }

// fx-c-pa-zoomblur-01-tr: 3fc4c0de7f119c49b1e0478e97872fc9; selected map d759fa1ad738c7c8c48ad5775499deb1dafee1b7c91903e48fb751adeb6a9a0d.
float4 VNative76(V_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_VSourceMaterialParameters[1u];
    source[2].x = (g_VSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_VSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_VSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0=0.f, v1=0.f, v2=float4(input.uv,0.f,0.f), v3=input.color;
    float4 v4=input.dynamicParameter;
    float4 v5=float4(0.f,0.f,0.f,1.f); // Project identity fog.
    float4 v6=float4(input.tangentView,1.f);
    float4 v7=float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW);
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
    // 10: add r2.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r2.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 11: mul r1.xy, r2.xyxx, r2.xyxx
    r1.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 12: add r0.w, r1.y, r1.x
    r0.w = ((r1.yyyy)+(r1.xxxx)).w;
    // 13: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 15: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 16: mul r0.w, r0.w, cb0[2].x
    r0.w = ((r0.wwww)*(source[2].xxxx)).w;
    // 17: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 18: mul_sat r0.w, r0.w, cb0[2].y
    r0.w = (saturate((r0.wwww)*(source[2].yyyy))).w;
    // 19: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 20: mul r1.x, r0.w, v3.w
    r1.x = ((r0.wwww)*(v3.wwww)).x;
    // 21: mul o0.w, r1.x, cb0[0].x
    output.w = ((r1.xxxx)*(source[0].xxxx)).w;
    // 22: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 23: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 24: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r1.xyxx, t0.xyzw, s0 (native engine SceneColor)
    r3.xyz = (Read_EffectSceneColorBias(LinearClampUVSampler, (r1.xyxx).xy, (0.f).x).xyzw).xyz;
    // 25: mul r1.w, v4.x, l(-0.010000)
    r1.w = ((v4.xxxx)*(float4(-0.010000,-0.010000,-0.010000,-0.010000))).w;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r1.xyxx, t0.xyzw, s0, l(0.000000) (native engine SceneColor)
    r4.xyz = (Read_EffectSceneColorBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 27: dp2 r2.x, r2.zwzz, r2.zwzz
    r2.x = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).x;
    // 28: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 29: mul r5.xy, r2.xxxx, r2.zwzz
    r5.xy = ((r2.xxxx)*(r2.zwzz)).xy;
    // 30: mad r2.xy, -r2.zwzz, r2.xxxx, r2.zwzz
    r2.xy = ((-(r2.zwzz))*(r2.xxxx)+(r2.zwzz)).xy;
    // 31: mad r2.xy, r2.xyxx, l(0.900000, 0.900000, 0.000000, 0.000000), r5.xyxx
    r2.xy = ((r2.xyxx)*(float4(0.900000,0.900000,0.000000,0.000000))+(r5.xyxx)).xy;
    // 32: mul r2.xy, r1.wwww, r2.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)).xy;
    // 33: mad r2.xy, r2.xyxx, l(2.000000, -2.000000, 0.000000, 0.000000), l(-1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,-2.000000,0.000000,0.000000))+(float4(-1.000000,1.000000,0.000000,0.000000))).xy;
    // 34: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 35: mov r5.xyz, r4.xyzx
    r5.xyz = (r4.xyzx).xyz;
    // 36: mov r2.zw, r1.xxxy
    r2.zw = (r1.xxxy).zw;
    // 37: mov r1.w, l(0)
    r1.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 38: loop
    [loop] while (true) {
    // 39: ge r3.w, r1.w, l(3.000000)
    r3.w = (asfloat((uint4)((r1.wwww)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 40: breakc_nz r3.w
    if ((asuint(r3.wwww)).x != 0u) break;
    // 41: add r2.zw, r2.xxxy, r2.zzzw
    r2.zw = ((r2.xxxy)+(r2.zzzw)).zw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r2.zwzz, t0.xyzw, s0, l(0.000000) (native engine SceneColor)
    r6.xyz = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 43: add r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)+(r6.xyzx)).xyz;
    // 44: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: endloop
    }
    // 46: mad r1.xyw, r5.xyxz, l(0.250000, 0.250000, 0.000000, 0.250000), -r3.xyxz
    r1.xyw = ((r5.xyxz)*(float4(0.250000,0.250000,0.000000,0.250000))+(-(r3.xyxz))).xyw;
    // 47: mul r1.xyw, r0.wwww, r1.xyxw
    r1.xyw = ((r0.wwww)*(r1.xyxw)).xyw;
    // 48: mad r1.xyw, cb0[2].zzzz, r1.xyxw, r3.xyxz
    r1.xyw = ((source[2].zzzz)*(r1.xyxw)+(r3.xyxz)).xyw;
    // 49: mad r1.xyw, v3.xyxz, r1.xyxw, cb0[1].xyxz
    r1.xyw = ((v3.xyxz)*(r1.xyxw)+(source[1].xyxz)).xyw;
    // 50: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
bool VNativeIsAdditive(uint profile) { return profile==52u || profile==53u || profile==54u || profile==55u || profile==56u || profile==57u || profile==58u || profile==61u || profile==62u || profile==64u || profile==65u || profile==70u || profile==71u || profile==72u || profile==73u || profile==74u; }

EFFECT_PS_OUT Shade_EffectDimensionMasterVNative(uint profile, float2 uv,
    float2 sourceUV1, float2 pixelPosition, float sourceProjectionW,
    float3 tangentView, float4 particleColor, float4 rawDynamic)
{
    EFFECT_PS_OUT output=(EFFECT_PS_OUT)0;
    uint width=0u,height=0u;
    g_EffectSceneDepthTexture.GetDimensions(width,height);
    if (width==0u || height==0u || profile<52u || profile>76u)
    { clip(-1.f); return output; }
    V_NATIVE_INPUT input;
    input.uv=uv; input.uv1=sourceUV1;
    input.screenUV=pixelPosition/float2(width,height);
    input.projectionW=sourceProjectionW*100.f;
    input.tangentView=tangentView;
    input.color=particleColor; input.dynamicParameter=rawDynamic;
    float4 nativeColor=0.f;
    switch (profile)
    {
    case 52u: nativeColor=VNative52(input); break;
    case 53u: nativeColor=VNative53(input); break;
    case 54u: nativeColor=VNative54(input); break;
    case 55u: nativeColor=VNative55(input); break;
    case 56u: nativeColor=VNative56(input); break;
    case 57u: nativeColor=VNative57(input); break;
    case 58u: nativeColor=VNative58(input); break;
    case 59u: nativeColor=VNative59(input); break;
    case 60u: nativeColor=VNative60(input); break;
    case 61u: nativeColor=VNative61(input); break;
    case 62u: nativeColor=VNative62(input); break;
    case 63u: nativeColor=VNative63(input); break;
    case 64u: nativeColor=VNative64(input); break;
    case 65u: nativeColor=VNative65(input); break;
    case 66u: nativeColor=VNative66(input); break;
    case 67u: nativeColor=VNative67(input); break;
    case 69u: nativeColor=VNative69(input); break;
    case 70u: nativeColor=VNative70(input); break;
    case 71u: nativeColor=VNative71(input); break;
    case 72u: nativeColor=VNative72(input); break;
    case 73u: nativeColor=VNative73(input); break;
    case 74u: nativeColor=VNative74(input); break;
    case 75u: nativeColor=VNative75(input); break;
    }
    // Native additive PS already weights RGB by opacity and writes A=0.
    // Product additive uses SrcAlpha, so A=1 preserves that native RGB once.
    const bool additive=VNativeIsAdditive(profile);
    output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,
        additive ? 1.f : nativeColor.a);
    output.Distortion=0.f; // Separate native distortion/MRT passes not claimed.
    if (g_ColorClip>0.f) clip(output.SceneColor.a-g_ColorClip);
    return output;
}

float4 Shade_EffectDimensionMasterVPostNative(uint profile, float2 uv, float2 screenUV,
    float sourceProjectionW, float4 particleColor, float4 rawDynamic)
{
    V_NATIVE_INPUT input;
    input.uv=uv; input.uv1=0.f; input.screenUV=screenUV;
    input.projectionW=sourceProjectionW*100.f;
    input.tangentView=float3(0.f,0.f,1.f);
    input.color=particleColor; input.dynamicParameter=rawDynamic;
    if (profile==68u) return VNative68(input);
    if (profile==76u) return VNative76(input);
    clip(-1.f); return 0.f;
}

#endif
