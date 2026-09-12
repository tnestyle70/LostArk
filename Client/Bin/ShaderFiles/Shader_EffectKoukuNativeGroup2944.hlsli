// Original Kouku material programs 2944..3007; native IDs and expressions are unchanged.
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_cardfanshape_01_3_ts_tr: 44a52ecceb50ff4c916943be691e9164; selected map 0a0f7df30f7dbca00ef142f4af9ddd3d0de60d1d73ef1eb3ba984d8220add7cd.
float4 ArtistNative2944(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].y = ((float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[2].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[2].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)))).x;
    source[3].x = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[3].y = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: add r0.xy, -v2.yxyy, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((-(v2.yxyy))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
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
    // 25: mad r0.x, r0.x, l(0.159155), l(0.250000)
    r0.x = ((r0.xxxx)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.250000,0.250000,0.250000,0.250000))).x;
    // 26: mad r0.x, -|r0.x|, l(4.000000), l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))*(float4(4.000000,4.000000,4.000000,4.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 27: add r0.x, r0.x, -cb0[3].w
    r0.x = ((r0.xxxx)+(-(source[3].wwww))).x;
    // 28: mul_sat r0.x, r0.x, cb0[4].x
    r0.x = (saturate((r0.xxxx)*(source[4].xxxx))).x;
    // 29: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 30: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 31: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 32: mad r0.z, -r0.y, cb0[2].w, l(1.000000)
    r0.z = ((-(r0.yyyy))*(source[2].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: mul_sat r0.z, r0.z, l(50.000050)
    r0.z = (saturate((r0.zzzz)*(float4(50.000050,50.000050,50.000050,50.000050)))).z;
    // 34: mul r0.z, r0.z, l(0.040000)
    r0.z = ((r0.zzzz)*(float4(0.040000,0.040000,0.040000,0.040000))).z;
    // 35: mad r0.w, -r0.y, l(2.000000), l(1.000000)
    r0.w = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: mad r0.y, -r0.y, l(2.008032), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.008032,2.008032,2.008032,2.008032))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 37: mul_sat r0.y, r0.y, l(200.000000)
    r0.y = (saturate((r0.yyyy)*(float4(200.000000,200.000000,200.000000,200.000000)))).y;
    // 38: max r1.x, r0.w, l(0.000000)
    r1.x = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 39: mul_sat r1.yz, r0.wwww, l(0.000000, 100.000099, 200.000000, 0.000000)
    r1.yz = (saturate((r0.wwww)*(float4(0.000000,100.000099,200.000000,0.000000)))).yz;
    // 40: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 42: mad r0.z, r0.z, r0.w, r1.y
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.yyyy)).z;
    // 43: add r0.y, -r0.y, r1.z
    r0.y = ((-(r0.yyyy))+(r1.zzzz)).y;
    // 44: mul r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)*(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 45: mad r0.w, cb0[3].y, l(-0.100000), l(1.100000)
    r0.w = ((source[3].yyyy)*(float4(-0.100000,-0.100000,-0.100000,-0.100000))+(float4(1.100000,1.100000,1.100000,1.100000))).w;
    // 46: mad r0.y, r0.z, r0.w, r0.y
    r0.y = ((r0.zzzz)*(r0.wwww)+(r0.yyyy)).y;
    // 47: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 48: mad r0.yz, v2.xxyx, l(0.000000, 2.500000, 2.500000, 0.000000), l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)*(float4(0.000000,2.500000,2.500000,0.000000))+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 49: dp2 r0.w, l(-1.000000, -0.000001, 0.000000, 0.000000), r0.yzyy
    r0.w = (dot((float4(-1.000000,-0.000001,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).w;
    // 50: dp2 r0.y, l(0.000001, -1.000000, 0.000000, 0.000000), r0.yzyy
    r0.y = (dot((float4(0.000001,-1.000000,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).y;
    // 51: add r1.xy, r0.wyww, l(1.250000, 2.000000, 0.000000, 0.000000)
    r1.xy = ((r0.wyww)+(float4(1.250000,2.000000,0.000000,0.000000))).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.xwyz, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 53: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 54: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 55: mul r0.x, r0.x, cb0[4].y
    r0.x = ((r0.xxxx)*(source[4].yyyy)).x;
    // 56: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 57: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 58: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_cardfanshape_01_1_ts_tr: 44a52ecceb50ff4c916943be691e9164; selected map 0a0f7df30f7dbca00ef142f4af9ddd3d0de60d1d73ef1eb3ba984d8220add7cd.
float4 ArtistNative2945(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].y = ((float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[2].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[2].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)))).x;
    source[3].x = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[3].y = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: add r0.xy, -v2.yxyy, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((-(v2.yxyy))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
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
    // 25: mad r0.x, r0.x, l(0.159155), l(0.250000)
    r0.x = ((r0.xxxx)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.250000,0.250000,0.250000,0.250000))).x;
    // 26: mad r0.x, -|r0.x|, l(4.000000), l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))*(float4(4.000000,4.000000,4.000000,4.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 27: add r0.x, r0.x, -cb0[3].w
    r0.x = ((r0.xxxx)+(-(source[3].wwww))).x;
    // 28: mul_sat r0.x, r0.x, cb0[4].x
    r0.x = (saturate((r0.xxxx)*(source[4].xxxx))).x;
    // 29: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 30: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 31: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 32: mad r0.z, -r0.y, cb0[2].w, l(1.000000)
    r0.z = ((-(r0.yyyy))*(source[2].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: mul_sat r0.z, r0.z, l(50.000050)
    r0.z = (saturate((r0.zzzz)*(float4(50.000050,50.000050,50.000050,50.000050)))).z;
    // 34: mul r0.z, r0.z, l(0.040000)
    r0.z = ((r0.zzzz)*(float4(0.040000,0.040000,0.040000,0.040000))).z;
    // 35: mad r0.w, -r0.y, l(2.000000), l(1.000000)
    r0.w = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: mad r0.y, -r0.y, l(2.008032), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.008032,2.008032,2.008032,2.008032))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 37: mul_sat r0.y, r0.y, l(200.000000)
    r0.y = (saturate((r0.yyyy)*(float4(200.000000,200.000000,200.000000,200.000000)))).y;
    // 38: max r1.x, r0.w, l(0.000000)
    r1.x = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 39: mul_sat r1.yz, r0.wwww, l(0.000000, 100.000099, 200.000000, 0.000000)
    r1.yz = (saturate((r0.wwww)*(float4(0.000000,100.000099,200.000000,0.000000)))).yz;
    // 40: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 42: mad r0.z, r0.z, r0.w, r1.y
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.yyyy)).z;
    // 43: add r0.y, -r0.y, r1.z
    r0.y = ((-(r0.yyyy))+(r1.zzzz)).y;
    // 44: mul r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)*(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 45: mad r0.w, cb0[3].y, l(-0.100000), l(1.100000)
    r0.w = ((source[3].yyyy)*(float4(-0.100000,-0.100000,-0.100000,-0.100000))+(float4(1.100000,1.100000,1.100000,1.100000))).w;
    // 46: mad r0.y, r0.z, r0.w, r0.y
    r0.y = ((r0.zzzz)*(r0.wwww)+(r0.yyyy)).y;
    // 47: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 48: mad r0.yz, v2.xxyx, l(0.000000, 2.500000, 2.500000, 0.000000), l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)*(float4(0.000000,2.500000,2.500000,0.000000))+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 49: dp2 r0.w, l(-1.000000, -0.000001, 0.000000, 0.000000), r0.yzyy
    r0.w = (dot((float4(-1.000000,-0.000001,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).w;
    // 50: dp2 r0.y, l(0.000001, -1.000000, 0.000000, 0.000000), r0.yzyy
    r0.y = (dot((float4(0.000001,-1.000000,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).y;
    // 51: add r1.xy, r0.wyww, l(1.250000, 2.000000, 0.000000, 0.000000)
    r1.xy = ((r0.wyww)+(float4(1.250000,2.000000,0.000000,0.000000))).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.xwyz, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 53: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 54: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 55: mul r0.x, r0.x, cb0[4].y
    r0.x = ((r0.xxxx)*(source[4].yyyy)).x;
    // 56: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 57: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 58: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_cardfanshape_01_2_ts_tr: 44a52ecceb50ff4c916943be691e9164; selected map 0a0f7df30f7dbca00ef142f4af9ddd3d0de60d1d73ef1eb3ba984d8220add7cd.
float4 ArtistNative2946(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].y = ((float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[2].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[2].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)))).x;
    source[3].x = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[3].y = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: add r0.xy, -v2.yxyy, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((-(v2.yxyy))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
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
    // 25: mad r0.x, r0.x, l(0.159155), l(0.250000)
    r0.x = ((r0.xxxx)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.250000,0.250000,0.250000,0.250000))).x;
    // 26: mad r0.x, -|r0.x|, l(4.000000), l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))*(float4(4.000000,4.000000,4.000000,4.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 27: add r0.x, r0.x, -cb0[3].w
    r0.x = ((r0.xxxx)+(-(source[3].wwww))).x;
    // 28: mul_sat r0.x, r0.x, cb0[4].x
    r0.x = (saturate((r0.xxxx)*(source[4].xxxx))).x;
    // 29: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 30: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 31: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 32: mad r0.z, -r0.y, cb0[2].w, l(1.000000)
    r0.z = ((-(r0.yyyy))*(source[2].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: mul_sat r0.z, r0.z, l(50.000050)
    r0.z = (saturate((r0.zzzz)*(float4(50.000050,50.000050,50.000050,50.000050)))).z;
    // 34: mul r0.z, r0.z, l(0.040000)
    r0.z = ((r0.zzzz)*(float4(0.040000,0.040000,0.040000,0.040000))).z;
    // 35: mad r0.w, -r0.y, l(2.000000), l(1.000000)
    r0.w = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: mad r0.y, -r0.y, l(2.008032), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.008032,2.008032,2.008032,2.008032))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 37: mul_sat r0.y, r0.y, l(200.000000)
    r0.y = (saturate((r0.yyyy)*(float4(200.000000,200.000000,200.000000,200.000000)))).y;
    // 38: max r1.x, r0.w, l(0.000000)
    r1.x = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 39: mul_sat r1.yz, r0.wwww, l(0.000000, 100.000099, 200.000000, 0.000000)
    r1.yz = (saturate((r0.wwww)*(float4(0.000000,100.000099,200.000000,0.000000)))).yz;
    // 40: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 42: mad r0.z, r0.z, r0.w, r1.y
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.yyyy)).z;
    // 43: add r0.y, -r0.y, r1.z
    r0.y = ((-(r0.yyyy))+(r1.zzzz)).y;
    // 44: mul r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)*(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 45: mad r0.w, cb0[3].y, l(-0.100000), l(1.100000)
    r0.w = ((source[3].yyyy)*(float4(-0.100000,-0.100000,-0.100000,-0.100000))+(float4(1.100000,1.100000,1.100000,1.100000))).w;
    // 46: mad r0.y, r0.z, r0.w, r0.y
    r0.y = ((r0.zzzz)*(r0.wwww)+(r0.yyyy)).y;
    // 47: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 48: mad r0.yz, v2.xxyx, l(0.000000, 2.500000, 2.500000, 0.000000), l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)*(float4(0.000000,2.500000,2.500000,0.000000))+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 49: dp2 r0.w, l(-1.000000, -0.000001, 0.000000, 0.000000), r0.yzyy
    r0.w = (dot((float4(-1.000000,-0.000001,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).w;
    // 50: dp2 r0.y, l(0.000001, -1.000000, 0.000000, 0.000000), r0.yzyy
    r0.y = (dot((float4(0.000001,-1.000000,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).y;
    // 51: add r1.xy, r0.wyww, l(1.250000, 2.000000, 0.000000, 0.000000)
    r1.xy = ((r0.wyww)+(float4(1.250000,2.000000,0.000000,0.000000))).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.xwyz, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 53: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 54: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 55: mul r0.x, r0.x, cb0[4].y
    r0.x = ((r0.xxxx)*(source[4].yyyy)).x;
    // 56: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 57: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 58: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_cardfanshape_01_ts_tr: 44a52ecceb50ff4c916943be691e9164; selected map 0a0f7df30f7dbca00ef142f4af9ddd3d0de60d1d73ef1eb3ba984d8220add7cd.
float4 ArtistNative2947(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].y = ((float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[2].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[2].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)))).x;
    source[3].x = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[3].y = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: add r0.xy, -v2.yxyy, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((-(v2.yxyy))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
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
    // 25: mad r0.x, r0.x, l(0.159155), l(0.250000)
    r0.x = ((r0.xxxx)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.250000,0.250000,0.250000,0.250000))).x;
    // 26: mad r0.x, -|r0.x|, l(4.000000), l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))*(float4(4.000000,4.000000,4.000000,4.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 27: add r0.x, r0.x, -cb0[3].w
    r0.x = ((r0.xxxx)+(-(source[3].wwww))).x;
    // 28: mul_sat r0.x, r0.x, cb0[4].x
    r0.x = (saturate((r0.xxxx)*(source[4].xxxx))).x;
    // 29: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 30: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 31: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 32: mad r0.z, -r0.y, cb0[2].w, l(1.000000)
    r0.z = ((-(r0.yyyy))*(source[2].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: mul_sat r0.z, r0.z, l(50.000050)
    r0.z = (saturate((r0.zzzz)*(float4(50.000050,50.000050,50.000050,50.000050)))).z;
    // 34: mul r0.z, r0.z, l(0.040000)
    r0.z = ((r0.zzzz)*(float4(0.040000,0.040000,0.040000,0.040000))).z;
    // 35: mad r0.w, -r0.y, l(2.000000), l(1.000000)
    r0.w = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: mad r0.y, -r0.y, l(2.008032), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.008032,2.008032,2.008032,2.008032))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 37: mul_sat r0.y, r0.y, l(200.000000)
    r0.y = (saturate((r0.yyyy)*(float4(200.000000,200.000000,200.000000,200.000000)))).y;
    // 38: max r1.x, r0.w, l(0.000000)
    r1.x = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 39: mul_sat r1.yz, r0.wwww, l(0.000000, 100.000099, 200.000000, 0.000000)
    r1.yz = (saturate((r0.wwww)*(float4(0.000000,100.000099,200.000000,0.000000)))).yz;
    // 40: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 42: mad r0.z, r0.z, r0.w, r1.y
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.yyyy)).z;
    // 43: add r0.y, -r0.y, r1.z
    r0.y = ((-(r0.yyyy))+(r1.zzzz)).y;
    // 44: mul r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)*(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 45: mad r0.w, cb0[3].y, l(-0.100000), l(1.100000)
    r0.w = ((source[3].yyyy)*(float4(-0.100000,-0.100000,-0.100000,-0.100000))+(float4(1.100000,1.100000,1.100000,1.100000))).w;
    // 46: mad r0.y, r0.z, r0.w, r0.y
    r0.y = ((r0.zzzz)*(r0.wwww)+(r0.yyyy)).y;
    // 47: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 48: mad r0.yz, v2.xxyx, l(0.000000, 2.500000, 2.500000, 0.000000), l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)*(float4(0.000000,2.500000,2.500000,0.000000))+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 49: dp2 r0.w, l(-1.000000, -0.000001, 0.000000, 0.000000), r0.yzyy
    r0.w = (dot((float4(-1.000000,-0.000001,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).w;
    // 50: dp2 r0.y, l(0.000001, -1.000000, 0.000000, 0.000000), r0.yzyy
    r0.y = (dot((float4(0.000001,-1.000000,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).y;
    // 51: add r1.xy, r0.wyww, l(1.250000, 2.000000, 0.000000, 0.000000)
    r1.xy = ((r0.wyww)+(float4(1.250000,2.000000,0.000000,0.000000))).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.xwyz, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 53: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 54: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 55: mul r0.x, r0.x, cb0[4].y
    r0.x = ((r0.xxxx)*(source[4].yyyy)).x;
    // 56: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 57: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 58: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_ring_01_tr: b81160355d97fb4c9919e34cefbce3f4; selected map 558e3aa396dd63d9a5d127f25267cf8a033d091dec2b4203796ec9ca9b034db8.
float4 ArtistNative2948(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[2].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[2].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 5: mul_sat r0.x, r0.x, cb0[2].w
    r0.x = (saturate((r0.xxxx)*(source[2].wwww))).x;
    // 6: add r0.y, -r0.x, l(1.000000)
    r0.y = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 7: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 8: mul r0.x, r0.x, l(4.100000)
    r0.x = ((r0.xxxx)*(float4(4.100000,4.100000,4.100000,4.100000))).x;
    // 9: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 10: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 11: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 12: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 13: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_worldoffset_02_11_tr: a0f7f5723e826143b3970e44f6a045f2; selected map 556a53205748259b14772cac15cc51ca4b99fc29a3e8ad2940bdffdafc2ff11e.
float4 ArtistNative2949(ARTIST_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[4] = g_ArtistSourceMaterialParameters[8u];
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].xxxx,g_ArtistSourceMaterialParameters[5u].yyyy,1u);
    source[6] = ArtistNativeAppend(cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[10].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[10].y = (cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].z = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[11].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[12].w = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[14].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[14].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[14].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[15].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[15].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[16].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[16].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[16].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[16].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[17].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[17].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[17].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[17].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: add r0.xyz, v7.xyzx, cb0[0].xyzx
    r0.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 2: mul r0.xyz, r0.xyzx, l(0.003906, 0.003906, 0.003906, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.003906,0.003906,0.003906,0.000000))).xyz;
    // 3: mul r0.yw, r0.yyyy, cb0[2].xxxy
    r0.yw = ((r0.yyyy)*(source[2].xxxy)).yw;
    // 4: mad r0.xy, cb0[1].xyxx, r0.xxxx, r0.ywyy
    r0.xy = ((source[1].xyxx)*(r0.xxxx)+(r0.ywyy)).xy;
    // 5: mad r0.xy, cb0[3].xyxx, r0.zzzz, r0.xyxx
    r0.xy = ((source[3].xyxx)*(r0.zzzz)+(r0.xyxx)).xy;
    // 6: mul r0.z, r0.x, cb0[13].w
    r0.z = ((r0.xxxx)*(source[13].wwww)).z;
    // 7: mad r1.x, cb0[11].w, cb0[13].z, r0.z
    r1.x = ((source[11].wwww)*(source[13].zzzz)+(r0.zzzz)).x;
    // 8: mul r0.z, r0.y, cb0[14].x
    r0.z = ((r0.yyyy)*(source[14].xxxx)).z;
    // 9: mad r1.y, cb0[11].w, cb0[14].z, r0.z
    r1.y = ((source[11].wwww)*(source[14].zzzz)+(r0.zzzz)).y;
    // 10: add r0.zw, r1.xxxy, cb0[8].xxxy
    r0.zw = ((r1.xxxy)+(source[8].xxxy)).zw;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 12: mul r1.xy, r0.xyxx, cb0[15].zwzz
    r1.xy = ((r0.xyxx)*(source[15].zwzz)).xy;
    // 13: mul r0.xy, r0.xyxx, cb0[12].xyxx
    r0.xy = ((r0.xyxx)*(source[12].xyxx)).xy;
    // 14: mad r2.x, cb0[11].w, cb0[15].y, r1.x
    r2.x = ((source[11].wwww)*(source[15].yyyy)+(r1.xxxx)).x;
    // 15: mad r2.y, cb0[11].w, cb0[16].x, r1.y
    r2.y = ((source[11].wwww)*(source[16].xxxx)+(r1.yyyy)).y;
    // 16: add r1.xy, r2.xyxx, cb0[9].xyxx
    r1.xy = ((r2.xyxx)+(source[9].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t4.yzwx, s4, l(0.000000)
    r0.w = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 18: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 19: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 20: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 21: mul r0.w, r0.w, cb0[16].w
    r0.w = ((r0.wwww)*(source[16].wwww)).w;
    // 22: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 23: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 24: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 25: mul r0.w, r0.w, cb0[17].x
    r0.w = ((r0.wwww)*(source[17].xxxx)).w;
    // 26: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 27: mul r0.w, r0.w, cb0[17].y
    r0.w = ((r0.wwww)*(source[17].yyyy)).w;
    // 28: lt r1.x, r0.z, l(0.000001)
    r1.x = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 30: mad r0.z, r0.z, cb0[17].z, r0.w
    r0.z = ((r0.zzzz)*(source[17].zzzz)+(r0.wwww)).z;
    // 31: mad r1.x, cb0[11].w, cb0[11].z, r0.x
    r1.x = ((source[11].wwww)*(source[11].zzzz)+(r0.xxxx)).x;
    // 32: mad r1.y, cb0[11].w, cb0[12].z, r0.y
    r1.y = ((source[11].wwww)*(source[12].zzzz)+(r0.yyyy)).y;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 34: mad r1.xy, cb0[10].zwzz, v2.xyxx, cb0[5].xyxx
    r1.xy = ((source[10].zwzz)*(v2.xyxx)+(source[5].xyxx)).xy;
    // 35: mul r0.w, v4.x, cb0[12].w
    r0.w = ((v4.xxxx)*(source[12].wwww)).w;
    // 36: mad r1.xy, r0.wwww, r0.xyxx, r1.xyxx
    r1.xy = ((r0.wwww)*(r0.xyxx)+(r1.xyxx)).xy;
    // 37: mad r1.z, v4.y, l(0.500000), r1.y
    r1.z = ((v4.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r1.yyyy)).z;
    // 38: add r0.xy, r1.xzxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xzxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 39: dp2 r1.x, cb0[6].xyxx, r0.xyxx
    r1.x = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 40: dp2 r1.y, cb0[7].xyxx, r0.xyxx
    r1.y = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 41: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 44: add r0.y, r1.y, r1.x
    r0.y = ((r1.yyyy)+(r1.xxxx)).y;
    // 45: add r0.y, r1.z, r0.y
    r0.y = ((r1.zzzz)+(r0.yyyy)).y;
    // 46: mul r0.y, r0.y, l(0.333330)
    r0.y = ((r0.yyyy)*(float4(0.333330,0.333330,0.333330,0.333330))).y;
    // 47: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 48: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 49: mul r0.y, r0.y, cb0[13].x
    r0.y = ((r0.yyyy)*(source[13].xxxx)).y;
    // 50: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 51: mul r0.y, r0.y, cb0[13].y
    r0.y = ((r0.yyyy)*(source[13].yyyy)).y;
    // 52: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 53: movc r0.y, r0.w, l(0), r0.y
    r0.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 54: mad r0.y, r0.x, r0.z, r0.y
    r0.y = ((r0.xxxx)*(r0.zzzz)+(r0.yyyy)).y;
    // 55: mul r0.x, r0.x, cb0[17].w
    r0.x = ((r0.xxxx)*(source[17].wwww)).x;
    // 56: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 57: mad r0.yzw, r0.yyyy, v3.xxyz, cb0[4].xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)+(source[4].xxyz)).yzw;
    // 58: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 59: add r0.yz, -v2.xxyx, l(0.000000, 1.000000, 1.000000, 0.000000)
    r0.yz = ((-(v2.xxyx))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 60: mul r0.yz, r0.yyzy, v2.xxyx
    r0.yz = ((r0.yyzy)*(v2.xxyx)).yz;
    // 61: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 62: mul_sat r0.y, r0.y, l(50.000000)
    r0.y = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).y;
    // 63: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 64: mul o0.w, r0.x, cb0[0].w
    output.w = ((r0.xxxx)*(source[0].wwww)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_h_pa_rotgather_01_1_tr: ac53f5392214cd479b3ec6dc1118ee43; selected map 5135f532cdf7a7aeb18e345899743aa9128f214288e3d84a28daafff5aea9dc8.
float4 ArtistNative2950(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[3].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: mul r0.xy, v2.xyxx, cb0[2].zwzz
    r0.xy = ((v2.xyxx)*(source[2].zwzz)).xy;
    // 2: mad r1.x, cb0[2].y, cb0[2].x, r0.x
    r1.x = ((source[2].yyyy)*(source[2].xxxx)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[2].y, cb0[3].x, r0.y
    r1.y = ((source[2].yyyy)*(source[3].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t3.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: mul r0.x, r0.x, v4.w
    r0.x = ((r0.xxxx)*(v4.wwww)).x;
    // 6: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 7: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 8: mul r0.y, r0.y, cb0[3].y
    r0.y = ((r0.yyyy)*(source[3].yyyy)).y;
    // 9: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 10: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 11: mul r0.y, cb0[2].y, cb0[3].z
    r0.y = ((source[2].yyyy)*(source[3].zzzz)).y;
    // 12: mad r1.x, cb0[3].w, v2.x, r0.y
    r1.x = ((source[3].wwww)*(v2.xxxx)+(r0.yyyy)).x;
    // 13: mul r0.y, v2.y, cb0[4].x
    r0.y = ((v2.yyyy)*(source[4].xxxx)).y;
    // 14: mad r1.y, cb0[2].y, cb0[4].y, r0.y
    r1.y = ((source[2].yyyy)*(source[4].yyyy)+(r0.yyyy)).y;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 16: mad r0.yz, r0.yyyy, cb0[4].zzzz, v2.xxyx
    r0.yz = ((r0.yyyy)*(source[4].zzzz)+(v2.xxyx)).yz;
    // 17: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 18: sincos r1.x, r2.x, v4.x
    r1.x = (sin(v4.xxxx)).x; r2.x = (cos(v4.xxxx)).x;
    // 19: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 20: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 21: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 22: dp2 r1.y, r3.zyzz, r0.yzyy
    r1.y = (dot((r3.zyzz).xy,(r0.yzyy).xy).xxxx).y;
    // 23: dp2 r1.x, r3.yxyy, r0.yzyy
    r1.x = (dot((r3.yxyy).xy,(r0.yzyy).xy).xxxx).x;
    // 24: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t1.yxwz, s3, l(0.000000)
    r0.yz = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxwz).yz;
    // 26: mul r0.y, r0.y, cb0[4].w
    r0.y = ((r0.yyyy)*(source[4].wwww)).y;
    // 27: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 28: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 29: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 30: mul r0.w, r0.w, v4.y
    r0.w = ((r0.wwww)*(v4.yyyy)).w;
    // 31: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 32: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 33: mul r1.xy, v2.xyxx, cb0[5].yzyy
    r1.xy = ((v2.xyxx)*(source[5].yzyy)).xy;
    // 34: mad r1.xy, cb0[2].yyyy, cb0[5].xwxx, r1.xyxx
    r1.xy = ((source[2].yyyy)*(source[5].xwxx)+(r1.xyxx)).xy;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t4.yzwx, s4, l(0.000000)
    r0.w = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: mul r0.w, r0.w, cb0[6].x
    r0.w = ((r0.wwww)*(source[6].xxxx)).w;
    // 37: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 38: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 39: mul r1.x, r1.x, v4.z
    r1.x = ((r1.xxxx)*(v4.zzzz)).x;
    // 40: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 41: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 42: mad r0.x, r0.y, r0.w, r0.x
    r0.x = ((r0.yyyy)*(r0.wwww)+(r0.xxxx)).x;
    // 43: mad r0.xyw, r0.xxxx, v3.xyxz, cb0[1].xyxz
    r0.xyw = ((r0.xxxx)*(v3.xyxz)+(source[1].xyxz)).xyw;
    // 44: mad o0.xyz, r0.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 45: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 46: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 47: source device depth mapped to centimetre view depth; reconstruction at 49.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 49-52: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 53: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 54: add r0.y, -cb0[6].y, l(1.000000)
    r0.y = ((-(source[6].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 55: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 56: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 57: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 58: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_shine_01_3_ad: f308e6c8bf787a4fbb242a07ff277211; selected map 7300c3c9b40788b80b30a5d639bda8170624b700959cd1ce48b334633e0f6586.
float4 ArtistNative2951(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = ArtistNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[4].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[4].w = (clamp(g_ArtistSourceMaterialParameters[4u].yyyy,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[5].x = ((float4(1.0, 0.0, 0.0, 0.0)-clamp(g_ArtistSourceMaterialParameters[4u].yyyy,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 4: add r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 5: add r0.z, -r0.y, l(1.000000)
    r0.z = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 6: mad r0.w, r0.z, cb0[4].w, cb0[5].x
    r0.w = ((r0.zzzz)*(source[4].wwww)+(source[5].xxxx)).w;
    // 7: mul_sat r0.z, r0.z, cb0[9].x
    r0.z = (saturate((r0.zzzz)*(source[9].xxxx))).z;
    // 8: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 9: div r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)/(r0.wwww)).x;
    // 10: mad_sat r0.x, r0.x, l(0.500000), l(0.500000)
    r0.x = (saturate((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000)))).x;
    // 11: add r0.w, -r0.x, l(1.000000)
    r0.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: mul r0.w, r0.x, r0.w
    r0.w = ((r0.xxxx)*(r0.wwww)).w;
    // 13: mul r1.x, r0.w, l(4.000000)
    r1.x = ((r0.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 14: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 15: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 16: mul r1.x, r1.x, cb0[5].y
    r1.x = ((r1.xxxx)*(source[5].yyyy)).x;
    // 17: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 18: mul r1.x, r1.x, cb0[5].z
    r1.x = ((r1.xxxx)*(source[5].zzzz)).x;
    // 19: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 20: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 21: mul r1.x, r1.x, cb0[5].w
    r1.x = ((r1.xxxx)*(source[5].wwww)).x;
    // 22: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 23: mul r1.x, r1.x, cb0[6].x
    r1.x = ((r1.xxxx)*(source[6].xxxx)).x;
    // 24: lt r1.y, |r0.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 25: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 26: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 27: mul r1.x, r0.x, cb0[6].w
    r1.x = ((r0.xxxx)*(source[6].wwww)).x;
    // 28: mad r1.x, cb0[6].z, cb0[6].y, r1.x
    r1.x = ((source[6].zzzz)*(source[6].yyyy)+(r1.xxxx)).x;
    // 29: mul r1.zw, cb0[6].zzzz, cb0[7].yyyz
    r1.zw = ((source[6].zzzz)*(source[7].yyyz)).zw;
    // 30: mad r1.y, cb0[7].x, r0.y, r1.z
    r1.y = ((source[7].xxxx)*(r0.yyyy)+(r1.zzzz)).y;
    // 31: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 32: mad r2.y, cb0[6].z, cb0[8].y, r0.y
    r2.y = ((source[6].zzzz)*(source[8].yyyy)+(r0.yyyy)).y;
    // 33: mad r2.x, cb0[7].w, r0.x, r1.w
    r2.x = ((source[7].wwww)*(r0.xxxx)+(r1.wwww)).x;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 36: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 37: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 38: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 39: mul r0.x, r0.x, cb0[8].z
    r0.x = ((r0.xxxx)*(source[8].zzzz)).x;
    // 40: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 41: mul r0.x, r0.x, cb0[8].w
    r0.x = ((r0.xxxx)*(source[8].wwww)).x;
    // 42: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 43: mul_sat r0.x, r0.x, r0.w
    r0.x = (saturate((r0.xxxx)*(r0.wwww))).x;
    // 44: log r0.y, r0.z
    r0.y = (log2(r0.zzzz)).y;
    // 45: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 46: mul r0.y, r0.y, cb0[9].y
    r0.y = ((r0.yyyy)*(source[9].yyyy)).y;
    // 47: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 48: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 49: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 50: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 51: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_o_pa_screwflow_01_7_tr: 38a40c107251d6439fe88a279f60c7f5; selected map 659e8af14fcdf2596c9dc6a7174e7ed457810a6ad4afb814a90e8b1515db6a50.
float4 ArtistNative2952(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].w = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[3].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[3].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy)).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[6].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: log r0.x, |v2.x|
    r0.x = (log2(abs(v2.xxxx))).x;
    // 2: mul r0.x, r0.x, l(10.000000)
    r0.x = ((r0.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 3: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 4: mul r0.x, r0.x, l(200.000000)
    r0.x = ((r0.xxxx)*(float4(200.000000,200.000000,200.000000,200.000000))).x;
    // 5: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: lt r0.y, |v2.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(v2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 7: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 8: mad r0.yz, v2.yyxy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((v2.yyxy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 9: lt r0.w, l(0.000000), r0.y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))<(r0.yyyy)) * 0xffffffffu)).w;
    // 10: and r0.w, r0.w, l(0x3f800000)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).w;
    // 11: ge r1.x, r0.y, l(0.000000)
    r1.x = (asfloat((uint4)((r0.yyyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 12: movc r0.w, r1.x, r0.w, l(-1.000000)
    r0.w = ((asuint(r1.xxxx) != 0u) ? (r0.wwww) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 13: mul r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 14: mad r0.z, -r0.z, r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 15: div r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)/(r0.zzzz)).y;
    // 16: rsq r0.z, |r0.y|
    r0.z = (rsqrt(abs(r0.yyyy))).z;
    // 17: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 18: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 19: mad r0.z, r0.z, r0.w, l(1.000000)
    r0.z = ((r0.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 20: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 21: movc r0.y, r0.y, l(0.500000), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(0.500000,0.500000,0.500000,0.500000)) : (r0.zzzz)).y;
    // 22: add r1.xyz, v2.xxyx, l(1.000000, -0.500000, -0.500000, 0.000000)
    r1.xyz = ((v2.xxyx)+(float4(1.000000,-0.500000,-0.500000,0.000000))).xyz;
    // 23: dp2 r0.z, r1.yzyy, r1.yzyy
    r0.z = (dot((r1.yzyy).xy,(r1.yzyy).xy).xxxx).z;
    // 24: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 25: mad r0.z, -r0.z, l(1.992032), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(1.992032,1.992032,1.992032,1.992032))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 26: mul_sat r0.z, r0.z, l(100000.000000)
    r0.z = (saturate((r0.zzzz)*(float4(100000.000000,100000.000000,100000.000000,100000.000000)))).z;
    // 27: mad r0.y, r0.y, r0.z, -v2.y
    r0.y = ((r0.yyyy)*(r0.zzzz)+(-(v2.yyyy))).y;
    // 28: mad r0.y, r0.x, r0.y, v2.y
    r0.y = ((r0.xxxx)*(r0.yyyy)+(v2.yyyy)).y;
    // 29: mad r2.y, cb0[3].x, r0.y, cb0[3].z
    r2.y = ((source[3].xxxx)*(r0.yyyy)+(source[3].zzzz)).y;
    // 30: dp2 r0.z, l(-1.000000, -0.000001, 0.000000, 0.000000), r1.yzyy
    r0.z = (dot((float4(-1.000000,-0.000001,0.000000,0.000000)).xy,(r1.yzyy).xy).xxxx).z;
    // 31: add r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 32: mad r2.x, r0.z, cb0[2].x, cb0[2].w
    r2.x = ((r0.zzzz)*(source[2].xxxx)+(source[2].wwww)).x;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s0, l(0.000000)
    r2.xyz = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 34: mul r2.xyz, r2.xyzx, cb0[3].wwww
    r2.xyz = ((r2.xyzx)*(source[3].wwww)).xyz;
    // 35: mul r0.x, r1.x, cb0[4].z
    r0.x = ((r1.xxxx)*(source[4].zzzz)).x;
    // 36: mad r0.z, -|r1.z|, l(2.000000), l(1.000000)
    r0.z = ((-(abs(r1.zzzz)))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xyzw = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 38: mad r0.x, r1.y, cb0[4].w, cb0[5].y
    r0.x = ((r1.yyyy)*(source[4].wwww)+(source[5].yyyy)).x;
    // 39: mad r0.y, r1.z, cb0[5].z, cb0[6].x
    r0.y = ((r1.zzzz)*(source[5].zzzz)+(source[6].xxxx)).y;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t2.xywz, s2, l(0.000000)
    r0.xyw = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 41: mul r0.xyw, r0.xyxw, r1.xxxx
    r0.xyw = ((r0.xyxw)*(r1.xxxx)).xyw;
    // 42: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 43: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 44: log r1.y, |r0.z|
    r1.y = (log2(abs(r0.zzzz))).y;
    // 45: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 46: mul r1.y, r1.y, cb0[4].x
    r1.y = ((r1.yyyy)*(source[4].xxxx)).y;
    // 47: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 48: mul r1.y, r1.y, cb0[4].y
    r1.y = ((r1.yyyy)*(source[4].yyyy)).y;
    // 49: mad r1.zw, v2.xxxy, l(0.000000, 0.000000, 0.400000, 1.000000), l(0.000000, 0.000000, 0.100000, -0.500000)
    r1.zw = ((v2.xxxy)*(float4(0.000000,0.000000,0.400000,1.000000))+(float4(0.000000,0.000000,0.100000,-0.500000))).zw;
    // 50: dp2 r1.z, r1.zwzz, r1.zwzz
    r1.z = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).z;
    // 51: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 52: mad r1.z, -r1.z, l(2.000000), l(1.000000)
    r1.z = ((-(r1.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 53: mul_sat r1.z, r1.z, l(9.999998)
    r1.z = (saturate((r1.zzzz)*(float4(9.999998,9.999998,9.999998,9.999998)))).z;
    // 54: mul_sat r1.y, r1.z, r1.y
    r1.y = (saturate((r1.zzzz)*(r1.yyyy))).y;
    // 55: movc r0.z, r0.z, l(0), r1.y
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).z;
    // 56: mad r0.xyw, r0.zzzz, r2.xyxz, r0.xyxw
    r0.xyw = ((r0.zzzz)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 57: mad r0.z, r1.x, l(2.500000), r0.z
    r0.z = ((r1.xxxx)*(float4(2.500000,2.500000,2.500000,2.500000))+(r0.zzzz)).z;
    // 58: mad r0.xyw, r0.xyxw, v3.xyxz, cb0[1].xyxz
    r0.xyw = ((r0.xyxw)*(v3.xyxz)+(source[1].xyxz)).xyw;
    // 59: mad o0.xyz, r0.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 60: mul_sat r0.x, v2.x, cb0[6].y
    r0.x = (saturate((v2.xxxx)*(source[6].yyyy))).x;
    // 61: mul_sat r0.x, r0.x, r0.z
    r0.x = (saturate((r0.xxxx)*(r0.zzzz))).x;
    // 62: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 63: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_trail_04_1_ad: 3b999e7a42dc8b4796049085d5534f0d; selected map ddfa6780fd232822c32a57bef702012872510d7c744aec89366ef16885fdc498.
float4 ArtistNative2953(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: add r0.x, cb0[2].x, l(-1.000000)
    r0.x = ((source[2].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mad r0.xy, cb0[2].xxxx, v2.xyxx, -r0.xxxx
    r0.xy = ((source[2].xxxx)*(v2.xyxx)+(-(r0.xxxx))).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t1.xywz, s0, l(0.000000)
    r0.xyw = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 6: log r1.x, |r0.z|
    r1.x = (log2(abs(r0.zzzz))).x;
    // 7: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 8: mul r1.x, r1.x, cb0[2].w
    r1.x = ((r1.xxxx)*(source[2].wwww)).x;
    // 9: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 10: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 11: movc r0.z, r0.z, l(0), |r1.x|
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).z;
    // 12: lt r1.x, r0.z, l(0.000001)
    r1.x = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 13: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 14: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 15: mul r1.xyz, r0.xywx, cb0[2].yyyy
    r1.xyz = ((r0.xywx)*(source[2].yyyy)).xyz;
    // 16: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 17: mad r0.xyw, -cb0[2].yyyy, r0.xyxw, r1.wwww
    r0.xyw = ((-(source[2].yyyy))*(r0.xyxw)+(r1.wwww)).xyw;
    // 18: mad r0.xyw, cb0[2].zzzz, r0.xyxw, r1.xyxz
    r0.xyw = ((source[2].zzzz)*(r0.xyxw)+(r1.xyxz)).xyw;
    // 19: mad r0.xyw, r0.xyxw, v3.xyxz, cb0[1].xyxz
    r0.xyw = ((r0.xyxw)*(v3.xyxz)+(source[1].xyxz)).xyw;
    // 20: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 21: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 22: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_line_01_01_tr: 79f39324bf884e469ac073eae1084195; selected map faf9df96f881ac5b626f2b4dbcd0ce1621728c24cf40b152823a8b0546694caf.
float4 ArtistNative2954(ARTIST_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[10u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].zzzz,g_ArtistSourceMaterialParameters[4u].wwww,1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].zzzz,g_ArtistSourceMaterialParameters[6u].wwww,1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[7u].yyyy,g_ArtistSourceMaterialParameters[7u].zzzz,1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].yyyy,g_ArtistSourceMaterialParameters[5u].zzzz,1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[8u].yyyy,g_ArtistSourceMaterialParameters[8u].zzzz,1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[9u].xxxx,g_ArtistSourceMaterialParameters[9u].yyyy,1u);
    source[10] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,float4(1.0, 0.0, 0.0, 0.0),1u);
    source[11] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,float4(1.0, 0.0, 0.0, 0.0),1u);
    source[12].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[13].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[13].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[14].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[14].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[15].y = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[15].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[16].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[16].y = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[16].z = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[16].w = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[17].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[17].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[17].z = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[17].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[18].x = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[18].y = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[18].z = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[18].w = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[19].x = (g_ArtistSourceMaterialParameters[9u].yyyy).x;
    source[19].y = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[19].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[19].w = (g_ArtistSourceMaterialParameters[9u].zzzz).x;
    source[20].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[20].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[20].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[20].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[21].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[21].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[21].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[21].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mul r0.xyzw, v2.xyxy, l(1.500000, 1.500000, 3.000000, 3.000000)
    r0.xyzw = ((v2.xyxy)*(float4(1.500000,1.500000,3.000000,3.000000))).xyzw;
    // 2: mad r0.xy, cb0[12].xxxx, cb0[2].xyxx, r0.xyxx
    r0.xy = ((source[12].xxxx)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 3: mad r0.zw, cb0[12].xxxx, cb0[7].xxxy, r0.zzzw
    r0.zw = ((source[12].xxxx)*(source[7].xxxy)+(r0.zzzw)).zw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t5.yzxw, s5, l(0.000000)
    r0.z = (ArtistNativeSample5((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 7: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 8: mul r0.y, r0.y, cb0[12].w
    r0.y = ((r0.yyyy)*(source[12].wwww)).y;
    // 9: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 10: mul r0.y, r0.y, cb0[13].x
    r0.y = ((r0.yyyy)*(source[13].xxxx)).y;
    // 11: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 12: mad r0.yw, cb0[12].xxxx, cb0[3].xxxy, v2.xxxy
    r0.yw = ((source[12].xxxx)*(source[3].xxxy)+(v2.xxxy)).yw;
    // 13: mul r0.yw, r0.yyyw, cb0[13].wwww
    r0.yw = ((r0.yyyw)*(source[13].wwww)).yw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, r0.ywyy, t1.zxwy, s1, l(0.000000)
    r0.yw = (ArtistNativeSample1((r0.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxwy).yw;
    // 15: add r1.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 16: mul r1.zw, r1.xxxy, cb0[4].xxxy
    r1.zw = ((r1.xxxy)*(source[4].xxxy)).zw;
    // 17: mad r0.yw, cb0[14].xxxx, r0.yyyw, r1.zzzw
    r0.yw = ((source[14].xxxx)*(r0.yyyw)+(r1.zzzw)).yw;
    // 18: add r0.yw, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.ywyy, t2.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r0.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 20: mul r3.xyz, r2.xyzx, cb0[14].wwww
    r3.xyz = ((r2.xyzx)*(source[14].wwww)).xyz;
    // 21: mul r0.xyw, r0.xxxx, r3.xyxz
    r0.xyw = ((r0.xxxx)*(r3.xyxz)).xyw;
    // 22: mad r2.xyz, cb0[14].wwww, r2.xyzx, -r0.xywx
    r2.xyz = ((source[14].wwww)*(r2.xyzx)+(-(r0.xywx))).xyz;
    // 23: mad r0.xyw, cb0[15].xxxx, r2.xyxz, r0.xyxw
    r0.xyw = ((source[15].xxxx)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 24: log r1.z, |r0.z|
    r1.z = (log2(abs(r0.zzzz))).z;
    // 25: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r1.z, r1.z, cb0[17].z
    r1.z = ((r1.zzzz)*(source[17].zzzz)).z;
    // 27: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 28: mul r1.z, r1.z, cb0[17].w
    r1.z = ((r1.zzzz)*(source[17].wwww)).z;
    // 29: movc r0.z, r0.z, l(0), r1.z
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 30: mad r1.zw, cb0[12].xxxx, cb0[5].xxxy, v2.xxxy
    r1.zw = ((source[12].xxxx)*(source[5].xxxy)+(v2.xxxy)).zw;
    // 31: mul r1.zw, r1.zzzw, cb0[15].wwww
    r1.zw = ((r1.zzzw)*(source[15].wwww)).zw;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t3.zwxy, s3, l(0.000000)
    r1.zw = (ArtistNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 33: mul r2.xy, r1.xyxx, cb0[6].xyxx
    r2.xy = ((r1.xyxx)*(source[6].xyxx)).xy;
    // 34: mul r1.xy, r1.xyxx, cb0[9].xyxx
    r1.xy = ((r1.xyxx)*(source[9].xyxx)).xy;
    // 35: mad r1.zw, cb0[16].xxxx, r1.zzzw, r2.xxxy
    r1.zw = ((source[16].xxxx)*(r1.zzzw)+(r2.xxxy)).zw;
    // 36: add r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r1.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t4.xyzw, s4, l(0.000000)
    r2.xyz = (ArtistNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 38: mul r2.xyz, r2.xyzx, cb0[16].wwww
    r2.xyz = ((r2.xyzx)*(source[16].wwww)).xyz;
    // 39: mad r0.xyz, r0.zzzz, r2.xyzx, r0.xywx
    r0.xyz = ((r0.zzzz)*(r2.xyzx)+(r0.xywx)).xyz;
    // 40: mad r1.zw, cb0[12].xxxx, cb0[8].xxxy, v2.xxxy
    r1.zw = ((source[12].xxxx)*(source[8].xxxy)+(v2.xxxy)).zw;
    // 41: mul r1.zw, r1.zzzw, cb0[18].zzzz
    r1.zw = ((r1.zzzw)*(source[18].zzzz)).zw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t6.zwxy, s6, l(0.000000)
    r1.zw = (ArtistNativeSample6((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 43: mad r1.xy, cb0[18].wwww, r1.zwzz, r1.xyxx
    r1.xy = ((source[18].wwww)*(r1.zwzz)+(r1.xyxx)).xy;
    // 44: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t7.xyzw, s7, l(0.000000)
    r1.xyz = (ArtistNativeSample7((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 46: mad r0.xyz, cb0[19].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[19].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 47: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 48: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 49: mul r0.y, r0.y, cb0[19].w
    r0.y = ((r0.yyyy)*(source[19].wwww)).y;
    // 50: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 51: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 52: mad r0.z, v2.x, cb0[11].x, cb0[21].x
    r0.z = ((v2.xxxx)*(source[11].xxxx)+(source[21].xxxx)).z;
    // 53: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 54: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 55: lt r0.xz, |r0.xxzx|, l(0.000001, 0.000000, 0.000001, 0.000000)
    r0.xz = (asfloat((uint4)((abs(r0.xxzx))<(float4(0.000001,0.000000,0.000001,0.000000))) * 0xffffffffu)).xz;
    // 56: mul r0.w, r0.w, cb0[21].y
    r0.w = ((r0.wwww)*(source[21].yyyy)).w;
    // 57: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 58: mad r1.x, v2.x, cb0[10].x, cb0[20].y
    r1.x = ((v2.xxxx)*(source[10].xxxx)+(source[20].yyyy)).x;
    // 59: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 60: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 61: or r0.x, r0.x, r1.x
    r0.x = (asfloat(asuint(r0.xxxx) | asuint(r1.xxxx))).x;
    // 62: or r0.x, r0.x, r0.z
    r0.x = (asfloat(asuint(r0.xxxx) | asuint(r0.zzzz))).x;
    // 63: mul r0.z, r1.y, cb0[20].z
    r0.z = ((r1.yyyy)*(source[20].zzzz)).z;
    // 64: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 65: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 66: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 67: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 68: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 69: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 70: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 71: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_o_pa_et_08_1_tr: 9d3c5b3aaf2e1243bed18b96afaad621; selected map 633d4ae0affc7fa27315cbbbd36d9cb6b5887dd7864bc27f182160654d4fb436.
float4 ArtistNative2955(ARTIST_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[9u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].yyyy,g_ArtistSourceMaterialParameters[1u].zzzz,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].yyyy,g_ArtistSourceMaterialParameters[5u].zzzz,1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].xxxx,g_ArtistSourceMaterialParameters[6u].yyyy,1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].xxxx,g_ArtistSourceMaterialParameters[4u].yyyy,1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[7u].xxxx,g_ArtistSourceMaterialParameters[7u].yyyy,1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[7u].wwww,g_ArtistSourceMaterialParameters[8u].xxxx,1u);
    source[10] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].yyyy,float4(1.0, 0.0, 0.0, 0.0),1u);
    source[11].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[11].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[12].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[14].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[14].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[14].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[15].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[15].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[15].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[15].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[16].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[16].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[16].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[16].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[17].x = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[17].y = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[17].z = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[17].w = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[18].x = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[18].y = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[18].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[18].w = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[19].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[19].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[19].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mul r0.xyzw, v2.xyxy, l(1.500000, 1.500000, 3.000000, 3.000000)
    r0.xyzw = ((v2.xyxy)*(float4(1.500000,1.500000,3.000000,3.000000))).xyzw;
    // 2: mad r0.xy, cb0[11].xxxx, cb0[2].xyxx, r0.xyxx
    r0.xy = ((source[11].xxxx)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 3: mad r0.zw, cb0[11].xxxx, cb0[7].xxxy, r0.zzzw
    r0.zw = ((source[11].xxxx)*(source[7].xxxy)+(r0.zzzw)).zw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t5.yzxw, s5, l(0.000000)
    r0.z = (ArtistNativeSample5((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 7: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 8: mul r0.y, r0.y, cb0[11].w
    r0.y = ((r0.yyyy)*(source[11].wwww)).y;
    // 9: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 10: mul r0.y, r0.y, cb0[12].x
    r0.y = ((r0.yyyy)*(source[12].xxxx)).y;
    // 11: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 12: mad r0.yw, cb0[11].xxxx, cb0[3].xxxy, v2.xxxy
    r0.yw = ((source[11].xxxx)*(source[3].xxxy)+(v2.xxxy)).yw;
    // 13: mul r0.yw, r0.yyyw, cb0[12].wwww
    r0.yw = ((r0.yyyw)*(source[12].wwww)).yw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, r0.ywyy, t1.zxwy, s1, l(0.000000)
    r0.yw = (ArtistNativeSample1((r0.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxwy).yw;
    // 15: add r1.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 16: mul r1.zw, r1.xxxy, cb0[4].xxxy
    r1.zw = ((r1.xxxy)*(source[4].xxxy)).zw;
    // 17: mad r0.yw, cb0[13].xxxx, r0.yyyw, r1.zzzw
    r0.yw = ((source[13].xxxx)*(r0.yyyw)+(r1.zzzw)).yw;
    // 18: add r0.yw, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.ywyy, t2.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r0.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 20: mul r3.xyz, r2.xyzx, cb0[13].wwww
    r3.xyz = ((r2.xyzx)*(source[13].wwww)).xyz;
    // 21: mul r0.xyw, r0.xxxx, r3.xyxz
    r0.xyw = ((r0.xxxx)*(r3.xyxz)).xyw;
    // 22: mad r2.xyz, cb0[13].wwww, r2.xyzx, -r0.xywx
    r2.xyz = ((source[13].wwww)*(r2.xyzx)+(-(r0.xywx))).xyz;
    // 23: mad r0.xyw, cb0[14].xxxx, r2.xyxz, r0.xyxw
    r0.xyw = ((source[14].xxxx)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 24: log r1.z, |r0.z|
    r1.z = (log2(abs(r0.zzzz))).z;
    // 25: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r1.z, r1.z, cb0[16].z
    r1.z = ((r1.zzzz)*(source[16].zzzz)).z;
    // 27: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 28: mul r1.z, r1.z, cb0[16].w
    r1.z = ((r1.zzzz)*(source[16].wwww)).z;
    // 29: movc r0.z, r0.z, l(0), r1.z
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 30: mad r1.zw, cb0[11].xxxx, cb0[5].xxxy, v2.xxxy
    r1.zw = ((source[11].xxxx)*(source[5].xxxy)+(v2.xxxy)).zw;
    // 31: mul r1.zw, r1.zzzw, cb0[14].wwww
    r1.zw = ((r1.zzzw)*(source[14].wwww)).zw;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t3.zwxy, s3, l(0.000000)
    r1.zw = (ArtistNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 33: mul r2.xy, r1.xyxx, cb0[6].xyxx
    r2.xy = ((r1.xyxx)*(source[6].xyxx)).xy;
    // 34: mul r1.xy, r1.xyxx, cb0[9].xyxx
    r1.xy = ((r1.xyxx)*(source[9].xyxx)).xy;
    // 35: mad r1.zw, cb0[15].xxxx, r1.zzzw, r2.xxxy
    r1.zw = ((source[15].xxxx)*(r1.zzzw)+(r2.xxxy)).zw;
    // 36: add r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r1.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t4.xyzw, s4, l(0.000000)
    r2.xyz = (ArtistNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 38: mul r2.xyz, r2.xyzx, cb0[15].wwww
    r2.xyz = ((r2.xyzx)*(source[15].wwww)).xyz;
    // 39: mad r0.xyz, r0.zzzz, r2.xyzx, r0.xywx
    r0.xyz = ((r0.zzzz)*(r2.xyzx)+(r0.xywx)).xyz;
    // 40: mad r1.zw, cb0[11].xxxx, cb0[8].xxxy, v2.xxxy
    r1.zw = ((source[11].xxxx)*(source[8].xxxy)+(v2.xxxy)).zw;
    // 41: mul r1.zw, r1.zzzw, cb0[17].zzzz
    r1.zw = ((r1.zzzw)*(source[17].zzzz)).zw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t6.zwxy, s6, l(0.000000)
    r1.zw = (ArtistNativeSample6((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 43: mad r1.xy, cb0[17].wwww, r1.zwzz, r1.xyxx
    r1.xy = ((source[17].wwww)*(r1.zwzz)+(r1.xyxx)).xy;
    // 44: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t7.xyzw, s7, l(0.000000)
    r1.xyz = (ArtistNativeSample7((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 46: mad r0.xyz, cb0[18].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[18].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 47: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 48: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 49: mul r0.y, r0.y, cb0[18].w
    r0.y = ((r0.yyyy)*(source[18].wwww)).y;
    // 50: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 51: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 52: mad r0.z, v2.x, cb0[10].x, cb0[19].y
    r0.z = ((v2.xxxx)*(source[10].xxxx)+(source[19].yyyy)).z;
    // 53: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 54: lt r0.xz, |r0.xxzx|, l(0.000001, 0.000000, 0.000001, 0.000000)
    r0.xz = (asfloat((uint4)((abs(r0.xxzx))<(float4(0.000001,0.000000,0.000001,0.000000))) * 0xffffffffu)).xz;
    // 55: or r0.x, r0.x, r0.z
    r0.x = (asfloat(asuint(r0.xxxx) | asuint(r0.zzzz))).x;
    // 56: mul r0.z, r0.w, cb0[19].z
    r0.z = ((r0.wwww)*(source[19].zzzz)).z;
    // 57: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 58: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 59: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 60: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 61: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 62: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 63: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_pa_ringmaster_01_50_tr: 9713defe8f7bd046a7aec9639d52c2f6; selected map 232303d0203c62f1286ca5e16ee8fee0c86f50f6e6ddd2a3386ad58eb06acde8.
float4 ArtistNative2956(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[4].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[4].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)).x;
    source[4].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].xxxx))).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].x = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[9].y = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].z = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 27: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 28: add r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)+(r0.zzzz)).z;
    // 29: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 30: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 31: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 32: mul r1.xy, v4.wzww, cb0[4].xwxx
    r1.xy = ((v4.wzww)*(source[4].xwxx)).xy;
    // 33: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 34: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 35: mul r0.w, r0.w, cb0[4].z
    r0.w = ((r0.wwww)*(source[4].zzzz)).w;
    // 36: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 37: mad r0.y, r0.y, l(0.159155), r0.z
    r0.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(r0.zzzz)).y;
    // 38: add r2.x, r0.y, l(0.500000)
    r2.x = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 39: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 40: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 41: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 42: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 43: lt r0.z, r0.x, l(0.000000)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 44: mad r0.x, -r0.x, cb0[8].z, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[8].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 45: mul_sat r0.x, r0.x, cb0[9].z
    r0.x = (saturate((r0.xxxx)*(source[9].zzzz))).x;
    // 46: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 47: add r2.y, r0.y, r2.x
    r2.y = ((r0.yyyy)+(r2.xxxx)).y;
    // 48: mul r0.yz, v2.xxyx, cb0[5].yyzy
    r0.yz = ((v2.xxyx)*(source[5].yyzy)).yz;
    // 49: mad r0.yz, cb0[3].yyyy, cb0[5].xxwx, r0.yyzy
    r0.yz = ((source[3].yyyy)*(source[5].xxwx)+(r0.yyzy)).yz;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t0.zxyw, s1, l(0.000000)
    r0.yz = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 51: mul r0.w, v4.y, cb0[6].x
    r0.w = ((v4.yyyy)*(source[6].xxxx)).w;
    // 52: mad r0.yz, r0.wwww, r0.yyzy, r2.xxyx
    r0.yz = ((r0.wwww)*(r0.yyzy)+(r2.xxyx)).yz;
    // 53: mul r0.w, cb0[3].y, cb0[6].z
    r0.w = ((source[3].yyyy)*(source[6].zzzz)).w;
    // 54: mad r1.x, cb0[6].w, r0.y, r0.w
    r1.x = ((source[6].wwww)*(r0.yyyy)+(r0.wwww)).x;
    // 55: mul r0.w, r0.z, cb0[7].x
    r0.w = ((r0.zzzz)*(source[7].xxxx)).w;
    // 56: mul r0.yz, r0.yyzy, cb0[3].zzwz
    r0.yz = ((r0.yyzy)*(source[3].zzwz)).yz;
    // 57: mad r1.y, cb0[3].y, cb0[7].y, r0.w
    r1.y = ((source[3].yyyy)*(source[7].yyyy)+(r0.wwww)).y;
    // 58: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, l(-1.000000)
    r1.xyz = (ArtistNativeSample2((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 59: mad r2.x, cb0[3].y, cb0[3].x, r0.y
    r2.x = ((source[3].yyyy)*(source[3].xxxx)+(r0.yyyy)).x;
    // 60: mad r2.y, cb0[3].y, cb0[6].y, r0.z
    r2.y = ((source[3].yyyy)*(source[6].yyyy)+(r0.zzzz)).y;
    // 61: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r2.xyxx, t1.wxyz, s0, l(-1.000000)
    r0.yzw = (ArtistNativeSample0((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 62: add r0.yzw, r1.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)+(r0.yyzw)).yzw;
    // 63: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 64: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 65: mad r0.yzw, cb0[7].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[7].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 66: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 67: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 68: mul r0.yzw, r0.yyzw, cb0[7].wwww
    r0.yzw = ((r0.yyzw)*(source[7].wwww)).yzw;
    // 69: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 70: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 71: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 72: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 73: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 74: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 75: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 76: mul r0.y, r0.y, cb0[9].w
    r0.y = ((r0.yyyy)*(source[9].wwww)).y;
    // 77: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 78: mul_sat r0.y, r0.y, cb0[10].x
    r0.y = (saturate((r0.yyyy)*(source[10].xxxx))).y;
    // 79: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 80: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 81: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 82: mul r0.x, r0.x, cb0[10].y
    r0.x = ((r0.xxxx)*(source[10].yyyy)).x;
    // 83: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 84: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 85: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 86: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_sineflow_03_02_tr: b3c29a7fecef754a8aebaf55f4306a6c; selected map 85abc797cc5ac50d813ba5a969807e2f46b8010ea71a3bf914098735b0cfafdc.
float4 ArtistNative2957(ARTIST_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].xxxx,g_ArtistSourceMaterialParameters[3u].yyyy,1u);
    source[4] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].wwww),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[6] = g_ArtistSourceMaterialParameters[5u];
    source[7] = g_ArtistSourceMaterialParameters[6u];
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].zzzz,g_ArtistSourceMaterialParameters[4u].wwww,1u);
    source[9].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[10].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[10].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[10].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[11].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[11].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[12].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 22: dp2 r0.x, -r0.xyxx, -r0.xyxx
    r0.x = (dot((-(r0.xyxx)).xy,(-(r0.xyxx)).xy).xxxx).x;
    // 23: sqrt r0.y, r0.x
    r0.y = (sqrt(r0.xxxx)).y;
    // 24: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 25: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 26: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 27: mad r0.z, r0.z, l(0.318471), l(1.000000)
    r0.z = ((r0.zzzz)*(float4(0.318471,0.318471,0.318471,0.318471))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 28: mul r0.x, r0.z, l(0.500000)
    r0.x = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 29: mul r0.zw, r0.xxxy, cb0[3].xxxy
    r0.zw = ((r0.xxxy)*(source[3].xxxy)).zw;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 31: mov r1.y, cb0[9].z
    r1.y = (source[9].zzzz).y;
    // 32: mov r1.xz, l(0,0,0,0)
    r1.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 33: mad r2.xyzw, r0.xyxy, cb0[3].xyxy, r1.yxxy
    r2.xyzw = ((r0.xyxy)*(source[3].xyxy)+(r1.yxxy)).xyzw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r2.zwzz, t1.xyzw, s0, l(0.000000)
    r1.x = (ArtistNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 36: add r1.y, -r0.z, r1.x
    r1.y = ((-(r0.zzzz))+(r1.xxxx)).y;
    // 37: add r1.x, -r0.z, r0.w
    r1.x = ((-(r0.zzzz))+(r0.wwww)).x;
    // 38: mul r2.xy, r1.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r2.xy = ((r1.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 39: mov r2.z, l(0)
    r2.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 40: add r2.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 41: dp3 r0.z, r2.xyzx, r2.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 42: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 43: div r0.zw, r2.xxxy, r0.zzzz
    r0.zw = ((r2.xxxy)/(r0.zzzz)).zw;
    // 44: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 45: mul r0.zw, r0.zzzw, v4.xxxx
    r0.zw = ((r0.zzzw)*(v4.xxxx)).zw;
    // 46: mad r1.xy, r0.xyxx, cb0[2].xyxx, r0.zwzz
    r1.xy = ((r0.xyxx)*(source[2].xyxx)+(r0.zwzz)).xy;
    // 47: mov r1.w, v4.y
    r1.w = (v4.yyyy).w;
    // 48: add r1.xy, r1.zwzz, r1.xyxx
    r1.xy = ((r1.zwzz)+(r1.xyxx)).xy;
    // 49: add r1.xy, r1.xyxx, cb0[4].xyxx
    r1.xy = ((r1.xyxx)+(source[4].xyxx)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 51: mad r2.xy, r0.xyxx, cb0[5].xyxx, r0.zwzz
    r2.xy = ((r0.xyxx)*(source[5].xyxx)+(r0.zwzz)).xy;
    // 52: mad r0.xy, r0.xyxx, cb0[8].xyxx, r0.zwzz
    r0.xy = ((r0.xyxx)*(source[8].xyxx)+(r0.zwzz)).xy;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xyz = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t4.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 55: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 56: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 57: mad r1.xyz, -r1.xyzx, r2.xyzx, r0.wwww
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r0.wwww)).xyz;
    // 58: mad r1.xyz, cb0[10].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[10].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 59: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 60: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 61: mul r1.xyz, r1.xyzx, cb0[10].yyyy
    r1.xyz = ((r1.xyzx)*(source[10].yyyy)).xyz;
    // 62: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 63: mul r1.xyz, r1.xyzx, cb0[10].zzzz
    r1.xyz = ((r1.xyzx)*(source[10].zzzz)).xyz;
    // 64: max r0.yzw, |r0.xxyz|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.xxyz),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 65: add r0.x, r0.x, cb0[11].w
    r0.x = ((r0.xxxx)+(source[11].wwww)).x;
    // 66: mul r0.x, r0.x, cb0[12].x
    r0.x = ((r0.xxxx)*(source[12].xxxx)).x;
    // 67: mul r0.x, r0.x, v4.z
    r0.x = ((r0.xxxx)*(v4.zzzz)).x;
    // 68: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 69: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 70: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 71: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 72: mul r0.x, r0.x, cb0[12].y
    r0.x = ((r0.xxxx)*(source[12].yyyy)).x;
    // 73: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 74: mul_sat r0.x, r0.x, cb0[12].z
    r0.x = (saturate((r0.xxxx)*(source[12].zzzz))).x;
    // 75: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 76: mul r0.yzw, r0.yyzw, cb0[11].yyyy
    r0.yzw = ((r0.yyzw)*(source[11].yyyy)).yzw;
    // 77: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 78: mad r0.yzw, -cb0[11].zzzz, r0.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r0.yzw = ((-(source[11].zzzz))*(r0.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 79: add r2.xyz, -cb0[6].xyzx, cb0[7].xyzx
    r2.xyz = ((-(source[6].xyzx))+(source[7].xyzx)).xyz;
    // 80: mad r0.yzw, r0.yyzw, r2.xxyz, cb0[6].xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)+(source[6].xxyz)).yzw;
    // 81: mad r0.yzw, r1.xxyz, v3.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(v3.xxyz)+(r0.yyzw)).yzw;
    // 82: add r0.yzw, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)+(source[1].xxyz)).yzw;
    // 83: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 84: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample4((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 85: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 86: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 87: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_pa_ghost_01_tr: 2ba207e30274774f837e5c046c916567; selected map 66ea780feee3694af430325c80122bfcbd878986a418c393b5717c67b6f29b31.
float4 ArtistNative2958(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3] = g_ArtistSourceMaterialParameters[2u];
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[6] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[8].z = ((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[8].w = (sin((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 10: mul_sat r0.x, r0.x, l(0.016667)
    r0.x = (saturate((r0.xxxx)*(float4(0.016667,0.016667,0.016667,0.016667)))).x;
    // 11: add r0.yz, v2.xxyx, cb0[4].xxyx
    r0.yz = ((v2.xxyx)+(source[4].xxyx)).yz;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 13: mad r0.yz, r0.yyyy, l(0.000000, 0.050000, 0.050000, 0.000000), v2.xxyx
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.050000,0.050000,0.000000))+(v2.xxyx)).yz;
    // 14: mad r0.yz, r0.yyzy, cb0[5].xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)*(source[5].xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 15: dp2 r1.x, cb0[6].xyxx, r0.yzyy
    r1.x = (dot((source[6].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 16: dp2 r1.y, cb0[7].xyxx, r0.yzyy
    r1.y = (dot((source[7].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 17: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 19: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 20: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 21: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 22: log r0.x, |v2.y|
    r0.x = (log2(abs(v2.yyyy))).x;
    // 23: mul r0.x, r0.x, cb0[8].x
    r0.x = ((r0.xxxx)*(source[8].xxxx)).x;
    // 24: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 25: add r0.yzw, -cb0[2].xxyz, cb0[3].xxyz
    r0.yzw = ((-(source[2].xxyz))+(source[3].xxyz)).yzw;
    // 26: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 27: lt r0.w, |v2.y|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(v2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 28: movc r0.xyz, r0.wwww, l(0,0,0,0), r0.xyzx
    r0.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xyzx)).xyz;
    // 29: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 30: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 31: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_o_pa_sy_love_01_1_tr: bf1dc0954e42444696e59573663c1a8e; selected map 95a8606936618b4ca879e653d6b40912575055f68250016f64adfd16176ad924.
float4 ArtistNative2959(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 6: mul r0.z, v4.w, cb0[3].z
    r0.z = ((v4.wwww)*(source[3].zzzz)).z;
    // 7: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 8: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 9: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 10: mad r0.y, cb0[2].x, v4.x, l(-1.000000)
    r0.y = ((source[2].xxxx)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 11: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 12: mul r0.z, v4.x, cb0[2].x
    r0.z = ((v4.xxxx)*(source[2].xxxx)).z;
    // 13: mad r0.yz, r0.zzzz, v2.xxyx, -r0.yyyy
    r0.yz = ((r0.zzzz)*(v2.xxyx)+(-(r0.yyyy))).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 15: log r0.y, |r1.w|
    r0.y = (log2(abs(r1.wwww))).y;
    // 16: mul r0.y, r0.y, cb0[3].y
    r0.y = ((r0.yyyy)*(source[3].yyyy)).y;
    // 17: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 18: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 19: lt r0.z, |r1.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 20: movc r0.y, r0.z, l(0), |r0.y|
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).y;
    // 21: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 22: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 23: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 24: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 25: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 26: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 27: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 28: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 29: mul r0.xyz, r1.xyzx, cb0[2].yyyy
    r0.xyz = ((r1.xyzx)*(source[2].yyyy)).xyz;
    // 30: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 31: mad r1.xyz, -cb0[2].yyyy, r1.xyzx, r0.wwww
    r1.xyz = ((-(source[2].yyyy))*(r1.xyzx)+(r0.wwww)).xyz;
    // 32: mad r0.xyz, cb0[2].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[2].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 33: mul r1.xyz, r0.xyzx, cb0[2].wwww
    r1.xyz = ((r0.xyzx)*(source[2].wwww)).xyz;
    // 34: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 35: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 36: mul r1.xyz, r1.xyzx, cb0[3].xxxx
    r1.xyz = ((r1.xyzx)*(source[3].xxxx)).xyz;
    // 37: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 38: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 39: add r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)+(r0.wwww)).xyz;
    // 40: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 41: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_pa_shockwave_02_4_ad: 9028c32167632a45a5a22a4fc6ba324c; selected map 90cd63a191d704129e0b9ca16079790692223dacd1503eb24e6b9929607f2279.
float4 ArtistNative2960(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[5].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: mad r0.xy, v2.xyxx, l(1.000000, 0.500000, 0.000000, 0.000000), l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)*(float4(1.000000,0.500000,0.000000,0.000000))+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.x|, |r0.y|
    r0.z = (max(abs(r0.xxxx),abs(r0.yyyy))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.x|, |r0.y|
    r0.w = (min(abs(r0.xxxx),abs(r0.yyyy))).w;
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
    // 13: lt r1.y, |r0.x|, |r0.y|
    r1.y = (asfloat((uint4)((abs(r0.xxxx))<(abs(r0.yyyy))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.x, -r0.x
    r0.w = (asfloat((uint4)((r0.xxxx)<(-(r0.xxxx))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.x, r0.y
    r0.w = (min(r0.xxxx,r0.yyyy)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.x, r0.y
    r1.x = (max(r0.xxxx,r0.yyyy)).x;
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
    // 27: mad r0.yz, r0.yyyy, l(0.000000, 0.159155, 0.159155, 0.000000), l(0.000000, 0.500000, 0.250000, 0.000000)
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.159155,0.159155,0.000000))+(float4(0.000000,0.500000,0.250000,0.000000))).yz;
    // 28: mad r0.z, -|r0.z|, l(4.000000), l(1.000000)
    r0.z = ((-(abs(r0.zzzz)))*(float4(4.000000,4.000000,4.000000,4.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: add r1.x, r0.y, r0.y
    r1.x = ((r0.yyyy)+(r0.yyyy)).x;
    // 30: mov_sat r0.y, v4.x
    r0.y = (saturate(v4.xxxx)).y;
    // 31: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 32: add r0.w, -r0.y, r0.z
    r0.w = ((-(r0.yyyy))+(r0.zzzz)).w;
    // 33: add r0.z, r0.z, -cb0[5].y
    r0.z = ((r0.zzzz)+(-(source[5].yyyy))).z;
    // 34: mul_sat r0.z, r0.z, cb0[5].z
    r0.z = (saturate((r0.zzzz)*(source[5].zzzz))).z;
    // 35: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 37: mul_sat r0.y, r0.y, r0.w
    r0.y = (saturate((r0.yyyy)*(r0.wwww))).y;
    // 38: mul r0.y, r0.y, l(10.000000)
    r0.y = ((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 39: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 41: mad r0.z, -r0.x, l(2.000000), l(1.000000)
    r0.z = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 42: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 43: add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mul_sat r0.z, r0.z, cb0[5].w
    r0.z = (saturate((r0.zzzz)*(source[5].wwww))).z;
    // 45: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 46: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 47: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 48: mul r0.w, r0.w, cb0[6].x
    r0.w = ((r0.wwww)*(source[6].xxxx)).w;
    // 49: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 50: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 51: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 52: add r0.z, r0.x, r0.x
    r0.z = ((r0.xxxx)+(r0.xxxx)).z;
    // 53: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 54: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 55: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 56: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 57: movc r1.y, r0.x, l(0), r0.z
    r1.y = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 58: mul r0.xz, r1.xxyx, cb0[6].zzwz
    r0.xz = ((r1.xxyx)*(source[6].zzwz)).xz;
    // 59: mul r1.xy, r1.xyxx, cb0[3].zwzz
    r1.xy = ((r1.xyxx)*(source[3].zwzz)).xy;
    // 60: mad r2.x, cb0[3].y, cb0[6].y, r0.x
    r2.x = ((source[3].yyyy)*(source[6].yyyy)+(r0.xxxx)).x;
    // 61: mad r2.y, cb0[3].y, cb0[7].x, r0.z
    r2.y = ((source[3].yyyy)*(source[7].xxxx)+(r0.zzzz)).y;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t0.yxzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 63: mov_sat r0.z, v3.w
    r0.z = (saturate(v3.wwww)).z;
    // 64: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 65: add r0.x, -r0.z, r0.x
    r0.x = ((-(r0.zzzz))+(r0.xxxx)).x;
    // 66: mul_sat r0.x, r0.x, cb0[7].y
    r0.x = (saturate((r0.xxxx)*(source[7].yyyy))).x;
    // 67: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 68: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 69: mul r0.z, r0.z, cb0[7].z
    r0.z = ((r0.zzzz)*(source[7].zzzz)).z;
    // 70: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 71: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 72: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 73: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 74: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 75: mad r2.x, cb0[3].y, cb0[3].x, r1.x
    r2.x = ((source[3].yyyy)*(source[3].xxxx)+(r1.xxxx)).x;
    // 76: mad r2.y, cb0[3].y, cb0[4].x, r1.y
    r2.y = ((source[3].yyyy)*(source[4].xxxx)+(r1.yyyy)).y;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r2.xyxx, t1.wxyz, s0, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 78: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 79: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 80: mad r0.yzw, cb0[4].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[4].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 81: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 82: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 83: mul r0.yzw, r0.yyzw, cb0[4].zzzz
    r0.yzw = ((r0.yyzw)*(source[4].zzzz)).yzw;
    // 84: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 85: mul r0.yzw, r0.yyzw, cb0[4].wwww
    r0.yzw = ((r0.yyzw)*(source[4].wwww)).yzw;
    // 86: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 87: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 88: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 89: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 90: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 91: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_atta_12_13_ad: 5ea6abf36654c94eae8b523062def76d; selected map ddfa6780fd232822c32a57bef702012872510d7c744aec89366ef16885fdc498.
float4 ArtistNative2961(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: mad r0.x, cb0[2].x, v4.x, l(-1.000000)
    r0.x = ((source[2].xxxx)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, v4.x, cb0[2].x
    r0.y = ((v4.xxxx)*(source[2].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v2.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v2.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t1.xywz, s0, l(0.000000)
    r0.xyw = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 7: log r1.x, |r0.z|
    r1.x = (log2(abs(r0.zzzz))).x;
    // 8: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 9: mul r1.x, r1.x, cb0[2].w
    r1.x = ((r1.xxxx)*(source[2].wwww)).x;
    // 10: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 11: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 12: movc r0.z, r0.z, l(0), |r1.x|
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).z;
    // 13: log r1.x, r0.z
    r1.x = (log2(r0.zzzz)).x;
    // 14: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 15: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 16: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 17: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 18: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 19: mul r1.xyz, r0.xywx, cb0[2].yyyy
    r1.xyz = ((r0.xywx)*(source[2].yyyy)).xyz;
    // 20: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: mad r0.xyw, -cb0[2].yyyy, r0.xyxw, r1.wwww
    r0.xyw = ((-(source[2].yyyy))*(r0.xyxw)+(r1.wwww)).xyw;
    // 22: mad r0.xyw, cb0[2].zzzz, r0.xyxw, r1.xyxz
    r0.xyw = ((source[2].zzzz)*(r0.xyxw)+(r1.xyxz)).xyw;
    // 23: mad r0.xyw, r0.xyxw, v3.xyxz, cb0[1].xyxz
    r0.xyw = ((r0.xyxw)*(v3.xyxz)+(source[1].xyxz)).xyw;
    // 24: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 25: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 26: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_shield_01_tr: d7c20717d6ff1c4c8cb99f362ec579b8; selected map e39e8eabf3e3d217910e2deedc0d8b83bf7a019455b4417535e2a4cde78e7289.
float4 ArtistNative2962(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: mad r0.x, v4.x, l(3.000000), l(-2.000000)
    r0.x = ((v4.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))+(float4(-2.000000,-2.000000,-2.000000,-2.000000))).x;
    // 2: add r0.yzw, v2.xxyy, l(0.000000, -0.500000, -0.500000, -0.300000)
    r0.yzw = ((v2.xxyy)+(float4(0.000000,-0.500000,-0.500000,-0.300000))).yzw;
    // 3: dp2 r1.x, l(0.707388, -0.706825, 0.000000, 0.000000), r0.yzyy
    r1.x = (dot((float4(0.707388,-0.706825,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).x;
    // 4: add r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)+(r1.xxxx)).x;
    // 5: add r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 6: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: mul_sat r0.x, r0.x, cb0[2].x
    r0.x = (saturate((r0.xxxx)*(source[2].xxxx))).x;
    // 8: mul r1.x, r0.x, r0.x
    r1.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 9: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 10: mul r1.x, r0.x, r1.x
    r1.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 11: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 12: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 13: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 15: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 16: mul r0.z, |r0.w|, |r0.w|
    r0.z = ((abs(r0.wwww))*(abs(r0.wwww))).z;
    // 17: mul r0.z, r0.z, |r0.w|
    r0.z = ((r0.zzzz)*(abs(r0.wwww))).z;
    // 18: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 19: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 20: mul r0.x, r0.x, v4.y
    r0.x = ((r0.xxxx)*(v4.yyyy)).x;
    // 21: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 22: mad r0.z, -r0.y, l(2.000000), l(1.000000)
    r0.z = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 23: mad r0.y, -r0.y, l(2.061856), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.061856,2.061856,2.061856,2.061856))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 24: mul_sat r0.y, r0.y, l(50.000050)
    r0.y = (saturate((r0.yyyy)*(float4(50.000050,50.000050,50.000050,50.000050)))).y;
    // 25: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 26: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 28: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 29: mul r0.w, r0.w, cb0[2].z
    r0.w = ((r0.wwww)*(source[2].zzzz)).w;
    // 30: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 31: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 32: add r0.w, -v2.y, l(1.000000)
    r0.w = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 34: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 35: mul r1.x, r1.x, cb0[2].y
    r1.x = ((r1.xxxx)*(source[2].yyyy)).x;
    // 36: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 37: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 38: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 39: mad r0.x, r0.z, cb0[2].w, r0.x
    r0.x = ((r0.zzzz)*(source[2].wwww)+(r0.xxxx)).x;
    // 40: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 41: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 42: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 43: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 44: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_g_pa_sy_love_01_tr: 991ecc5522859c49bb034318a827af71; selected map fd85ade76eaa888dbc097dfdfc3900cacd64ae68812cae449a674d712986917f.
float4 ArtistNative2963(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: mad r0.x, cb0[2].x, v4.x, l(-1.000000)
    r0.x = ((source[2].xxxx)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, v4.x, cb0[2].x
    r0.y = ((v4.xxxx)*(source[2].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v2.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v2.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
    // 16: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 17: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 18: movc o0.w, r0.w, l(0), r1.x
    output.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 19: mul r1.xyz, r0.xyzx, cb0[2].yyyy
    r1.xyz = ((r0.xyzx)*(source[2].yyyy)).xyz;
    // 20: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: mad r0.xyz, -cb0[2].yyyy, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[2].yyyy))*(r0.xyzx)+(r0.wwww)).xyz;
    // 22: mad r0.xyz, cb0[2].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[2].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 24: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_b_pa_cd_01_1_tr: ab141c44af464947b2a7b26cf17b684e; selected map 57f7bdc060687b68ad0e018532e3ece3f52bcad8bed8239d4b4b2d2ae1c4dbcb.
float4 ArtistNative2964(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[3] = g_ArtistSourceMaterialParameters[4u];
    source[4].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: mul r0.xy, v2.xyxx, cb0[4].zwzz
    r0.xy = ((v2.xyxx)*(source[4].zwzz)).xy;
    // 2: mad r1.x, cb0[4].y, cb0[4].x, r0.x
    r1.x = ((source[4].yyyy)*(source[4].xxxx)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[4].y, cb0[5].x, r0.y
    r1.y = ((source[4].yyyy)*(source[5].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mad r0.xy, cb0[5].yyyy, r0.xyxx, v2.xyxx
    r0.xy = ((source[5].yyyy)*(r0.xyxx)+(v2.xyxx)).xy;
    // 6: mad r0.z, cb0[5].z, v4.x, l(-1.000000)
    r0.z = ((source[5].zzzz)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 7: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 8: mul r0.w, v4.x, cb0[5].z
    r0.w = ((v4.xxxx)*(source[5].zzzz)).w;
    // 9: mad r0.xy, r0.wwww, r0.xyxx, -r0.zzzz
    r0.xy = ((r0.wwww)*(r0.xyxx)+(-(r0.zzzz))).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t1.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t5.xywz, s2, l(0.000000)
    r0.xyw = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 12: log r1.x, |r0.z|
    r1.x = (log2(abs(r0.zzzz))).x;
    // 13: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 14: mul r1.x, r1.x, cb0[6].y
    r1.x = ((r1.xxxx)*(source[6].yyyy)).x;
    // 15: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 16: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 17: movc r0.z, r0.z, l(0), |r1.x|
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).z;
    // 18: log r1.x, r0.z
    r1.x = (log2(r0.zzzz)).x;
    // 19: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 20: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 21: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 22: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 23: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 24: add r1.zw, r1.xxxy, -cb0[0].xxxy
    r1.zw = ((r1.xxxy)+(-(source[0].xxxy))).zw;
    // 25: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 26: source device depth mapped to centimetre view depth; reconstruction at 37.
    r1.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // 28: mad r1.yz, r1.zzwz, cb0[1].zzwz, l(0.000000, 0.500000, 0.500000, 0.000000)
    r1.yz = ((r1.zzwz)*(source[1].zzwz)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 29: mul r1.y, r1.y, cb0[6].w
    r1.y = ((r1.yyyy)*(source[6].wwww)).y;
    // 30: mul r1.z, r1.z, cb0[7].x
    r1.z = ((r1.zzzz)*(source[7].xxxx)).z;
    // 31: mad r2.y, cb0[4].y, cb0[7].y, r1.z
    r2.y = ((source[4].yyyy)*(source[7].yyyy)+(r1.zzzz)).y;
    // 32: mad r2.x, cb0[4].y, cb0[6].z, r1.y
    r2.x = ((source[4].yyyy)*(source[6].zzzz)+(r1.yyyy)).x;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t2.yxzw, s4, l(0.000000)
    r1.y = (ArtistNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 34: add r1.y, r1.y, l(-1.000000)
    r1.y = ((r1.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 35: mad r1.y, cb0[7].z, r1.y, l(1.000000)
    r1.y = ((source[7].zzzz)*(r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // Native 37-40: reconstructed view depth is supplied by the runtime adapter.
    r1.x = r1.x;
    // 41: add r1.x, r1.x, -v7.w
    r1.x = ((r1.xxxx)+(-(v7.wwww))).x;
    // 42: add r1.y, -cb0[7].w, l(1.000000)
    r1.y = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 43: max r1.y, r1.y, l(0.001000)
    r1.y = (max(r1.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 44: div_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)/(r1.yyyy))).x;
    // 45: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v2.xyxx, t4.xyzw, s5, l(0.000000)
    r1.x = (ArtistNativeSample4((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 47: mul_sat r0.z, r0.z, r1.x
    r0.z = (saturate((r0.zzzz)*(r1.xxxx))).z;
    // 48: mul o0.w, r0.z, cb0[2].x
    output.w = ((r0.zzzz)*(source[2].xxxx)).w;
    // 49: mul r1.xyz, r0.xywx, cb0[5].wwww
    r1.xyz = ((r0.xywx)*(source[5].wwww)).xyz;
    // 50: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 51: mad r0.xyz, -cb0[5].wwww, r0.xywx, r0.zzzz
    r0.xyz = ((-(source[5].wwww))*(r0.xywx)+(r0.zzzz)).xyz;
    // 52: mad r0.xyz, cb0[6].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[6].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 53: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[3].xyzx)).xyz;
    // 54: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_b_pa_ring_01_tr: e72f987c3f4d874e87d3a827aead9488; selected map 4a82c695651cad6036c7d1927c7b8a5424def0a5d2ad16485aba7d59357c0f45.
float4 ArtistNative2965(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.400000006, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.200000003, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.150000006, 0.0, 0.0, 0.0))),1u);
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: mad r0.xy, v2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), cb0[2].xyxx
    r0.xy = ((v2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.yxzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 3: mad r0.yz, v2.xxyx, l(0.000000, 3.000000, 3.000000, 0.000000), cb0[3].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,3.000000,3.000000,0.000000))+(source[3].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: add r0.y, v4.x, l(0.070000)
    r0.y = ((v4.xxxx)+(float4(0.070000,0.070000,0.070000,0.070000))).y;
    // 7: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, v2.xyxx, t0.wxyz, s0, l(0.000000)
    r0.yzw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 9: ge r0.x, r0.y, r0.x
    r0.x = (asfloat((uint4)((r0.yyyy)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 10: mad r0.yzw, v3.xxyz, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)*(r0.yyzw)+(source[1].xxyz)).yzw;
    // 11: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 12: and r0.x, r0.x, l(0x3f800000)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).x;
    // 13: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 14: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_pa_ghostface_01_1_tr: 2ba207e30274774f837e5c046c916567; selected map 66ea780feee3694af430325c80122bfcbd878986a418c393b5717c67b6f29b31.
float4 ArtistNative2966(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3] = g_ArtistSourceMaterialParameters[2u];
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[6] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[8].z = ((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[8].w = (sin((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 10: mul_sat r0.x, r0.x, l(0.016667)
    r0.x = (saturate((r0.xxxx)*(float4(0.016667,0.016667,0.016667,0.016667)))).x;
    // 11: add r0.yz, v2.xxyx, cb0[4].xxyx
    r0.yz = ((v2.xxyx)+(source[4].xxyx)).yz;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 13: mad r0.yz, r0.yyyy, l(0.000000, 0.050000, 0.050000, 0.000000), v2.xxyx
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.050000,0.050000,0.000000))+(v2.xxyx)).yz;
    // 14: mad r0.yz, r0.yyzy, cb0[5].xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)*(source[5].xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 15: dp2 r1.x, cb0[6].xyxx, r0.yzyy
    r1.x = (dot((source[6].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 16: dp2 r1.y, cb0[7].xyxx, r0.yzyy
    r1.y = (dot((source[7].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 17: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 19: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 20: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 21: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 22: log r0.x, |v2.y|
    r0.x = (log2(abs(v2.yyyy))).x;
    // 23: mul r0.x, r0.x, cb0[8].x
    r0.x = ((r0.xxxx)*(source[8].xxxx)).x;
    // 24: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 25: add r0.yzw, -cb0[2].xxyz, cb0[3].xxyz
    r0.yzw = ((-(source[2].xxyz))+(source[3].xxyz)).yzw;
    // 26: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 27: lt r0.w, |v2.y|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(v2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 28: movc r0.xyz, r0.wwww, l(0,0,0,0), r0.xyzx
    r0.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xyzx)).xyz;
    // 29: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 30: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 31: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_pa_questsymbol_01_6_ma: 5399190d0f4fa442b20a25d7175bfd2c; selected map 5ef241437ff26f8443c57b6041b329cfd91361af6980e946aadd7a7e444754b4.
float4 ArtistNative2967(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[0u];
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 2: mul r0.w, r0.x, v3.w
    r0.w = ((r0.xxxx)*(v3.wwww)).w;
    // 3: mad o0.xyz, r0.xyzx, v3.xyzx, cb0[0].xyzx
    output.xyz = ((r0.xyzx)*(v3.xyzx)+(source[0].xyzx)).xyz;
    // 4: mov_sat r0.w, r0.w
    r0.w = (saturate(r0.wwww)).w;
    // 5: add r0.x, r0.w, l(-0.333300)
    r0.x = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 6: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 7: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 8: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 9: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_glitter_01_tr: bd8398a6efa91243bb7de7f4bed27970; selected map 41c5e36935706c430baa850216169d8ff6ba9cb558897004f6967d0a0c2bf07e.
float4 ArtistNative2968(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].w = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(3.5, 0.0, 0.0, 0.0))).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.yz, r0.xxxx, l(0.000000, 0.400000, 0.400000, 0.000000), r0.zzwz
    r0.yz = ((r0.xxxx)*(float4(0.000000,0.400000,0.400000,0.000000))+(r0.zzwz)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yzyy, t1.yzwx, s4, l(0.000000)
    r0.w = (ArtistNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t4.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: add r0.z, r0.w, r0.w
    r0.z = ((r0.wwww)+(r0.wwww)).z;
    // 8: mul r0.w, r0.x, l(0.200000)
    r0.w = ((r0.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 9: add r1.xy, r0.xxxx, v2.xyxx
    r1.xy = ((r0.xxxx)+(v2.xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t5.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: mad r1.xy, v2.xyxx, l(0.950000, 0.950000, 0.000000, 0.000000), r0.wwww
    r1.xy = ((v2.xyxx)*(float4(0.950000,0.950000,0.000000,0.000000))+(r0.wwww)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s5, l(0.000000)
    r0.w = (ArtistNativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r1.z = (ArtistNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r1.x = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_shockwave_02_08_dt_ad: 6033128b6c4ee04d82e4f97482f74225; selected map a73efd614545ce6271931cca24df851a78cd2cc43933adc74e46f60b97e0d7eb.
float4 ArtistNative2969(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[5].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: mad r0.xy, v2.xyxx, l(1.000000, 0.500000, 0.000000, 0.000000), l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)*(float4(1.000000,0.500000,0.000000,0.000000))+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.x|, |r0.y|
    r0.z = (max(abs(r0.xxxx),abs(r0.yyyy))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.x|, |r0.y|
    r0.w = (min(abs(r0.xxxx),abs(r0.yyyy))).w;
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
    // 13: lt r1.y, |r0.x|, |r0.y|
    r1.y = (asfloat((uint4)((abs(r0.xxxx))<(abs(r0.yyyy))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.x, -r0.x
    r0.w = (asfloat((uint4)((r0.xxxx)<(-(r0.xxxx))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.x, r0.y
    r0.w = (min(r0.xxxx,r0.yyyy)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.x, r0.y
    r1.x = (max(r0.xxxx,r0.yyyy)).x;
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
    // 27: mad r0.yz, r0.yyyy, l(0.000000, 0.159155, 0.159155, 0.000000), l(0.000000, 0.500000, 0.250000, 0.000000)
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.159155,0.159155,0.000000))+(float4(0.000000,0.500000,0.250000,0.000000))).yz;
    // 28: mad r0.z, -|r0.z|, l(4.000000), l(1.000000)
    r0.z = ((-(abs(r0.zzzz)))*(float4(4.000000,4.000000,4.000000,4.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: add r1.x, r0.y, r0.y
    r1.x = ((r0.yyyy)+(r0.yyyy)).x;
    // 30: mov_sat r0.y, v4.x
    r0.y = (saturate(v4.xxxx)).y;
    // 31: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 32: add r0.w, -r0.y, r0.z
    r0.w = ((-(r0.yyyy))+(r0.zzzz)).w;
    // 33: add r0.z, r0.z, -cb0[5].z
    r0.z = ((r0.zzzz)+(-(source[5].zzzz))).z;
    // 34: mul_sat r0.z, r0.z, cb0[5].w
    r0.z = (saturate((r0.zzzz)*(source[5].wwww))).z;
    // 35: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 37: mul_sat r0.y, r0.y, r0.w
    r0.y = (saturate((r0.yyyy)*(r0.wwww))).y;
    // 38: mul r0.y, r0.y, l(10.000000)
    r0.y = ((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 39: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 41: mad r0.z, -r0.x, l(2.000000), l(1.000000)
    r0.z = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 42: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 43: add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mul_sat r0.z, r0.z, cb0[6].x
    r0.z = (saturate((r0.zzzz)*(source[6].xxxx))).z;
    // 45: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 46: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 47: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 48: mul r0.w, r0.w, cb0[6].y
    r0.w = ((r0.wwww)*(source[6].yyyy)).w;
    // 49: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 50: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 51: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 52: add r0.z, r0.x, r0.x
    r0.z = ((r0.xxxx)+(r0.xxxx)).z;
    // 53: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 54: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 55: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 56: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 57: movc r1.y, r0.x, l(0), r0.z
    r1.y = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 58: mul r0.x, r1.y, cb0[7].x
    r0.x = ((r1.yyyy)*(source[7].xxxx)).x;
    // 59: mul r0.zw, r1.xxxy, cb0[3].zzzw
    r0.zw = ((r1.xxxy)*(source[3].zzzw)).zw;
    // 60: mul r1.x, r1.x, cb0[6].w
    r1.x = ((r1.xxxx)*(source[6].wwww)).x;
    // 61: mad r1.x, cb0[3].y, cb0[6].z, r1.x
    r1.x = ((source[3].yyyy)*(source[6].zzzz)+(r1.xxxx)).x;
    // 62: mad r1.y, cb0[3].y, cb0[7].y, r0.x
    r1.y = ((source[3].yyyy)*(source[7].yyyy)+(r0.xxxx)).y;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t1.yxzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 64: mov_sat r1.x, v3.w
    r1.x = (saturate(v3.wwww)).x;
    // 65: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 66: add r0.x, r0.x, -r1.x
    r0.x = ((r0.xxxx)+(-(r1.xxxx))).x;
    // 67: mul_sat r0.x, r0.x, cb0[7].z
    r0.x = (saturate((r0.xxxx)*(source[7].zzzz))).x;
    // 68: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 69: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 70: mul r1.x, r1.x, cb0[7].w
    r1.x = ((r1.xxxx)*(source[7].wwww)).x;
    // 71: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 72: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 73: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 74: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 75: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 76: source device depth mapped to centimetre view depth; reconstruction at 78.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 78-81: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 82: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 83: add r1.x, -cb0[5].x, l(1.000000)
    r1.x = ((-(source[5].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 84: max r1.x, -r1.x, l(0.001000)
    r1.x = (max(-(r1.xxxx),float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 85: div_sat r0.y, r0.y, r1.x
    r0.y = (saturate((r0.yyyy)/(r1.xxxx))).y;
    // 86: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 87: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 88: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 89: mad r1.x, cb0[3].y, cb0[3].x, r0.z
    r1.x = ((source[3].yyyy)*(source[3].xxxx)+(r0.zzzz)).x;
    // 90: mad r1.y, cb0[3].y, cb0[4].x, r0.w
    r1.y = ((source[3].yyyy)*(source[4].xxxx)+(r0.wwww)).y;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t2.wxyz, s1, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 92: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 93: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 94: mad r0.yzw, cb0[4].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[4].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 95: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 96: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 97: mul r0.yzw, r0.yyzw, cb0[4].zzzz
    r0.yzw = ((r0.yyzw)*(source[4].zzzz)).yzw;
    // 98: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 99: mul r0.yzw, r0.yyzw, cb0[4].wwww
    r0.yzw = ((r0.yyzw)*(source[4].wwww)).yzw;
    // 100: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 101: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 102: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 103: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 104: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 105: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_shockwave_02_07_dt_ad: 6033128b6c4ee04d82e4f97482f74225; selected map a73efd614545ce6271931cca24df851a78cd2cc43933adc74e46f60b97e0d7eb.
float4 ArtistNative2970(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[5].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: mad r0.xy, v2.xyxx, l(1.000000, 0.500000, 0.000000, 0.000000), l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)*(float4(1.000000,0.500000,0.000000,0.000000))+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.x|, |r0.y|
    r0.z = (max(abs(r0.xxxx),abs(r0.yyyy))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.x|, |r0.y|
    r0.w = (min(abs(r0.xxxx),abs(r0.yyyy))).w;
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
    // 13: lt r1.y, |r0.x|, |r0.y|
    r1.y = (asfloat((uint4)((abs(r0.xxxx))<(abs(r0.yyyy))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.x, -r0.x
    r0.w = (asfloat((uint4)((r0.xxxx)<(-(r0.xxxx))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.x, r0.y
    r0.w = (min(r0.xxxx,r0.yyyy)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.x, r0.y
    r1.x = (max(r0.xxxx,r0.yyyy)).x;
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
    // 27: mad r0.yz, r0.yyyy, l(0.000000, 0.159155, 0.159155, 0.000000), l(0.000000, 0.500000, 0.250000, 0.000000)
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.159155,0.159155,0.000000))+(float4(0.000000,0.500000,0.250000,0.000000))).yz;
    // 28: mad r0.z, -|r0.z|, l(4.000000), l(1.000000)
    r0.z = ((-(abs(r0.zzzz)))*(float4(4.000000,4.000000,4.000000,4.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: add r1.x, r0.y, r0.y
    r1.x = ((r0.yyyy)+(r0.yyyy)).x;
    // 30: mov_sat r0.y, v4.x
    r0.y = (saturate(v4.xxxx)).y;
    // 31: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 32: add r0.w, -r0.y, r0.z
    r0.w = ((-(r0.yyyy))+(r0.zzzz)).w;
    // 33: add r0.z, r0.z, -cb0[5].z
    r0.z = ((r0.zzzz)+(-(source[5].zzzz))).z;
    // 34: mul_sat r0.z, r0.z, cb0[5].w
    r0.z = (saturate((r0.zzzz)*(source[5].wwww))).z;
    // 35: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 37: mul_sat r0.y, r0.y, r0.w
    r0.y = (saturate((r0.yyyy)*(r0.wwww))).y;
    // 38: mul r0.y, r0.y, l(10.000000)
    r0.y = ((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 39: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 41: mad r0.z, -r0.x, l(2.000000), l(1.000000)
    r0.z = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 42: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 43: add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mul_sat r0.z, r0.z, cb0[6].x
    r0.z = (saturate((r0.zzzz)*(source[6].xxxx))).z;
    // 45: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 46: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 47: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 48: mul r0.w, r0.w, cb0[6].y
    r0.w = ((r0.wwww)*(source[6].yyyy)).w;
    // 49: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 50: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 51: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 52: add r0.z, r0.x, r0.x
    r0.z = ((r0.xxxx)+(r0.xxxx)).z;
    // 53: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 54: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 55: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 56: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 57: movc r1.y, r0.x, l(0), r0.z
    r1.y = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 58: mul r0.x, r1.y, cb0[7].x
    r0.x = ((r1.yyyy)*(source[7].xxxx)).x;
    // 59: mul r0.zw, r1.xxxy, cb0[3].zzzw
    r0.zw = ((r1.xxxy)*(source[3].zzzw)).zw;
    // 60: mul r1.x, r1.x, cb0[6].w
    r1.x = ((r1.xxxx)*(source[6].wwww)).x;
    // 61: mad r1.x, cb0[3].y, cb0[6].z, r1.x
    r1.x = ((source[3].yyyy)*(source[6].zzzz)+(r1.xxxx)).x;
    // 62: mad r1.y, cb0[3].y, cb0[7].y, r0.x
    r1.y = ((source[3].yyyy)*(source[7].yyyy)+(r0.xxxx)).y;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t1.yxzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 64: mov_sat r1.x, v3.w
    r1.x = (saturate(v3.wwww)).x;
    // 65: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 66: add r0.x, r0.x, -r1.x
    r0.x = ((r0.xxxx)+(-(r1.xxxx))).x;
    // 67: mul_sat r0.x, r0.x, cb0[7].z
    r0.x = (saturate((r0.xxxx)*(source[7].zzzz))).x;
    // 68: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 69: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 70: mul r1.x, r1.x, cb0[7].w
    r1.x = ((r1.xxxx)*(source[7].wwww)).x;
    // 71: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 72: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 73: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 74: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 75: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 76: source device depth mapped to centimetre view depth; reconstruction at 78.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 78-81: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 82: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 83: add r1.x, -cb0[5].x, l(1.000000)
    r1.x = ((-(source[5].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 84: max r1.x, -r1.x, l(0.001000)
    r1.x = (max(-(r1.xxxx),float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 85: div_sat r0.y, r0.y, r1.x
    r0.y = (saturate((r0.yyyy)/(r1.xxxx))).y;
    // 86: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 87: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 88: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 89: mad r1.x, cb0[3].y, cb0[3].x, r0.z
    r1.x = ((source[3].yyyy)*(source[3].xxxx)+(r0.zzzz)).x;
    // 90: mad r1.y, cb0[3].y, cb0[4].x, r0.w
    r1.y = ((source[3].yyyy)*(source[4].xxxx)+(r0.wwww)).y;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t2.wxyz, s1, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 92: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 93: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 94: mad r0.yzw, cb0[4].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[4].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 95: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 96: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 97: mul r0.yzw, r0.yyzw, cb0[4].zzzz
    r0.yzw = ((r0.yyzw)*(source[4].zzzz)).yzw;
    // 98: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 99: mul r0.yzw, r0.yyzw, cb0[4].wwww
    r0.yzw = ((r0.yyzw)*(source[4].wwww)).yzw;
    // 100: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 101: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 102: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 103: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 104: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 105: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_o_pa_shine_01_01_ad: f2fc3c3dd927ed45bd7001ed07bca0c2; selected map 622b7fdefa885be22e04e772b1d9b264eb4d8e49632c075d24ea3ccbdbd8a081.
float4 ArtistNative2971(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: add r0.x, v2.x, l(-0.500000)
    r0.x = ((v2.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 2: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 3: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 4: mul r0.y, r0.x, v2.y
    r0.y = ((r0.xxxx)*(v2.yyyy)).y;
    // 5: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 6: mad r0.x, r0.x, l(0.500000), r0.y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.yyyy)).x;
    // 7: sqrt r0.y, v2.y
    r0.y = (sqrt(v2.yyyy)).y;
    // 8: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 9: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 10: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 11: mul r0.y, r0.y, cb0[2].x
    r0.y = ((r0.yyyy)*(source[2].xxxx)).y;
    // 12: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 13: mul_sat r0.y, r0.y, cb0[2].y
    r0.y = (saturate((r0.yyyy)*(source[2].yyyy))).y;
    // 14: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 15: add r0.y, -v2.y, l(1.000000)
    r0.y = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 16: mul_sat r0.y, r0.y, cb0[2].z
    r0.y = (saturate((r0.yyyy)*(source[2].zzzz))).y;
    // 17: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 18: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 19: mul r0.z, r0.z, cb0[2].w
    r0.z = ((r0.zzzz)*(source[2].wwww)).z;
    // 20: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 21: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 22: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 23: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 24: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 25: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 26: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 28: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 29: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 30: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 31: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 32: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 33: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 34: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_gl_02_2_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 ArtistNative2972(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[5].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r0.zw, cb0[5].xxxx, cb0[4].xxxy, v2.xxxy
    r0.zw = ((source[5].xxxx)*(source[4].xxxy)+(v2.xxxy)).zw;
    // 5: mad r0.xy, cb0[6].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[6].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_pa_holomatrix_02_4_ad: c673e1e673b4e4489da85d8c43d10bc1; selected map c4f13c68df7e2ef9cb78952d094374dc225a2b31d4de381ace18d8b43cc78531.
float4 ArtistNative2973(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0199999996, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.150000006, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[7] = g_ArtistSourceMaterialParameters[0u];
    source[8] = g_ArtistSourceMaterialParameters[1u];
    source[9].x = (((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[9].y = ((float4(0.00499999989, 0.0, 0.0, 0.0)+((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[9].z = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[9].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[10].x = (((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.0199999996, 0.0, 0.0, 0.0))).x;
    source[10].y = ((float4(0.100000001, 0.0, 0.0, 0.0)+((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[10].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[10].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[11].x = ((float4(1.5, 0.0, 0.0, 0.0)+(sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))))).x;
    source[11].y = (((float4(1.5, 0.0, 0.0, 0.0)+(sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: add r0.xy, v2.xyxx, cb0[3].xyxx
    r0.xy = ((v2.xyxx)+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mul r0.x, r0.x, cb0[10].y
    r0.x = ((r0.xxxx)*(source[10].yyyy)).x;
    // 4: mad r0.yz, v2.xxyx, l(0.000000, 1.000000, 3.000000, 0.000000), cb0[2].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,1.000000,3.000000,0.000000))+(source[2].xxyx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mad r0.x, r0.y, cb0[9].y, r0.x
    r0.x = ((r0.yyyy)*(source[9].yyyy)+(r0.xxxx)).x;
    // 7: add r0.xy, r0.xxxx, v2.xyxx
    r0.xy = ((r0.xxxx)+(v2.xyxx)).xy;
    // 8: mul r0.zw, r0.xxxy, l(0.000000, 0.000000, 8.000000, 8.000000)
    r0.zw = ((r0.xxxy)*(float4(0.000000,0.000000,8.000000,8.000000))).zw;
    // 9: add r0.xy, r0.xyxx, cb0[4].xyxx
    r0.xy = ((r0.xyxx)+(source[4].xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 12: frc r0.yz, r0.zzwz
    r0.yz = (frac(r0.zzwz)).yz;
    // 13: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 14: mul r0.yz, r0.yyzy, r0.yyzy
    r0.yz = ((r0.yyzy)*(r0.yyzy)).yz;
    // 15: mul r1.xy, r0.yzyy, l(4.000000, 4.000000, 0.000000, 0.000000)
    r1.xy = ((r0.yzyy)*(float4(4.000000,4.000000,0.000000,0.000000))).xy;
    // 16: lt r0.yz, r0.yyzy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.yz = (asfloat((uint4)((r0.yyzy)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 17: log r1.xy, r1.xyxx
    r1.xy = (log2(r1.xyxx)).xy;
    // 18: mul r1.xy, r1.xyxx, l(25.000000, 25.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(25.000000,25.000000,0.000000,0.000000))).xy;
    // 19: exp r1.xy, r1.xyxx
    r1.xy = (exp2(r1.xyxx)).xy;
    // 20: movc r0.yz, r0.yyzy, l(0,0,0,0), r1.xxyx
    r0.yz = ((asuint(r0.yyzy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxyx)).yz;
    // 21: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 22: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 23: dp2 r1.x, l(0.707107, -0.707107, 0.000000, 0.000000), r0.zwzz
    r1.x = (dot((float4(0.707107,-0.707107,0.000000,0.000000)).xy,(r0.zwzz).xy).xxxx).x;
    // 24: dp2 r1.y, l(0.707107, 0.707107, 0.000000, 0.000000), r0.zwzz
    r1.y = (dot((float4(0.707107,0.707107,0.000000,0.000000)).xy,(r0.zwzz).xy).xxxx).y;
    // 25: mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // 26: add r1.x, |r1.x|, |r1.y|
    r1.x = ((abs(r1.xxxx))+(abs(r1.yyyy))).x;
    // 27: mul r1.yz, r0.zzwz, l(0.000000, 4.000000, 4.000000, 0.000000)
    r1.yz = ((r0.zzwz)*(float4(0.000000,4.000000,4.000000,0.000000))).yz;
    // 28: lt r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.zw = (asfloat((uint4)((r0.zzzw)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).zw;
    // 29: log r1.yz, r1.yyzy
    r1.yz = (log2(r1.yyzy)).yz;
    // 30: mul r1.yz, r1.yyzy, l(0.000000, 10.000000, 10.000000, 0.000000)
    r1.yz = ((r1.yyzy)*(float4(0.000000,10.000000,10.000000,0.000000))).yz;
    // 31: exp r1.yz, r1.yyzy
    r1.yz = (exp2(r1.yyzy)).yz;
    // 32: movc r0.zw, r0.zzzw, l(0,0,0,0), r1.yyyz
    r0.zw = ((asuint(r0.zzzw) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyz)).zw;
    // 33: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 34: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 35: mul r0.w, r0.z, r1.x
    r0.w = ((r0.zzzz)*(r1.xxxx)).w;
    // 36: mul_sat r0.w, r0.w, l(1.650000)
    r0.w = (saturate((r0.wwww)*(float4(1.650000,1.650000,1.650000,1.650000)))).w;
    // 37: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 38: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 39: mul r1.x, r1.x, l(15.000000)
    r1.x = ((r1.xxxx)*(float4(15.000000,15.000000,15.000000,15.000000))).x;
    // 40: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 41: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 42: add r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)+(r0.wwww)).y;
    // 43: mul r1.xyz, r0.yyyy, cb0[8].xyzx
    r1.xyz = ((r0.yyyy)*(source[8].xyzx)).xyz;
    // 44: mad r0.yw, v2.xxxy, l(0.000000, 3.000000, 0.000000, 2.000000), r0.xxxx
    r0.yw = ((v2.xxxy)*(float4(0.000000,3.000000,0.000000,2.000000))+(r0.xxxx)).yw;
    // 45: mad r2.xy, v2.xyxx, l(5.000000, 3.000000, 0.000000, 0.000000), r0.xxxx
    r2.xy = ((v2.xyxx)*(float4(5.000000,3.000000,0.000000,0.000000))+(r0.xxxx)).xy;
    // 46: add r2.xy, r2.xyxx, cb0[6].xyxx
    r2.xy = ((r2.xyxx)+(source[6].xyxx)).xy;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 48: add r0.xy, r0.ywyy, cb0[5].xyxx
    r0.xy = ((r0.ywyy)+(source[5].xyxx)).xy;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t2.xywz, s2, l(0.000000)
    r0.xyw = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 50: mul r0.xyw, r2.xyxz, r0.xyxw
    r0.xyw = ((r2.xyxz)*(r0.xyxw)).xyw;
    // 51: mul r0.xyw, r0.xyxw, cb0[7].xyxz
    r0.xyw = ((r0.xyxw)*(source[7].xyxz)).xyw;
    // 52: mul r0.xyw, r0.xyxw, l(40.000000, 40.000000, 0.000000, 40.000000)
    r0.xyw = ((r0.xyxw)*(float4(40.000000,40.000000,0.000000,40.000000))).xyw;
    // 53: mul r2.xyz, r0.zzzz, cb0[7].xyzx
    r2.xyz = ((r0.zzzz)*(source[7].xyzx)).xyz;
    // 54: mul r0.z, r0.z, cb0[11].y
    r0.z = ((r0.zzzz)*(source[11].yyyy)).z;
    // 55: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 56: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 57: mul r2.xyz, r2.xyzx, l(0.050000, 0.050000, 0.050000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(0.050000,0.050000,0.050000,0.000000))).xyz;
    // 58: mad_sat r0.xyw, r0.xyxw, r1.xyxz, r2.xyxz
    r0.xyw = (saturate((r0.xyxw)*(r1.xyxz)+(r2.xyxz))).xyw;
    // 59: mad r0.xyw, r0.xyxw, v3.xyxz, cb0[1].xyxz
    r0.xyw = ((r0.xyxw)*(v3.xyxz)+(source[1].xyxz)).xyw;
    // 60: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 61: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 62: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_master_01_10_ad: 44852449b864a740997637a819f2a3f8; selected map 0aab1ecb5cdb7d8291e06f501fa6c84fe6f53ac21344dc2d7e401752307cbdac.
float4 ArtistNative2974(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = g_ArtistSourceMaterialParameters[4u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[4].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: add r0.x, v4.y, l(-1.000000)
    r0.x = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mad r0.y, cb0[4].y, cb0[6].w, cb0[7].x
    r0.y = ((source[4].yyyy)*(source[6].wwww)+(source[7].xxxx)).y;
    // 3: sincos r1.x, r2.x, r0.y
    r1.x = (sin(r0.yyyy)).x; r2.x = (cos(r0.yyyy)).x;
    // 4: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 5: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 6: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 7: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 8: dp2 r0.w, r3.zyzz, r0.yzyy
    r0.w = (dot((r3.zyzz).xy,(r0.yzyy).xy).xxxx).w;
    // 9: dp2 r0.y, r3.yxyy, r0.yzyy
    r0.y = (dot((r3.yxyy).xy,(r0.yzyy).xy).xxxx).y;
    // 10: mad r0.x, r0.y, cb0[3].x, r0.x
    r0.x = ((r0.yyyy)*(source[3].xxxx)+(r0.xxxx)).x;
    // 11: mul r0.z, r0.w, cb0[3].y
    r0.z = ((r0.wwww)*(source[3].yyyy)).z;
    // 12: add r0.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 14: mul r0.y, v2.y, cb0[6].x
    r0.y = ((v2.yyyy)*(source[6].xxxx)).y;
    // 15: mul r0.z, cb0[4].x, cb0[4].y
    r0.z = ((source[4].xxxx)*(source[4].yyyy)).z;
    // 16: mad r1.w, r0.z, cb0[6].y, r0.y
    r1.w = ((r0.zzzz)*(source[6].yyyy)+(r0.yyyy)).w;
    // 17: mul r0.yw, v2.yyyx, cb0[5].xxxw
    r0.yw = ((v2.yyyx)*(source[5].xxxw)).yw;
    // 18: mad r1.yz, r0.zzzz, cb0[5].yyzy, r0.yywy
    r1.yz = ((r0.zzzz)*(source[5].yyzy)+(r0.yywy)).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 20: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 21: mul r0.y, v2.x, cb0[4].w
    r0.y = ((v2.xxxx)*(source[4].wwww)).y;
    // 22: mad r1.x, r0.z, cb0[4].z, r0.y
    r1.x = ((r0.zzzz)*(source[4].zzzz)+(r0.yyyy)).x;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 24: add r0.z, -v4.x, l(1.000000)
    r0.z = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 25: mad r0.x, r0.y, r0.x, -r0.z
    r0.x = ((r0.yyyy)*(r0.xxxx)+(-(r0.zzzz))).x;
    // 26: mul_sat r0.x, r0.x, cb0[7].w
    r0.x = (saturate((r0.xxxx)*(source[7].wwww))).x;
    // 27: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 28: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 30: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 31: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 32: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 33: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 34: mul r0.yzw, cb0[2].xxyz, cb0[2].wwww
    r0.yzw = ((source[2].xxyz)*(source[2].wwww)).yzw;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_circ_01_02_ad: f223e0c7e9a78643ab33afec0a30fd52; selected map daf9dc99efc3b51b9abd1280ed355304a129abffa6febba2650d6878216675d4.
float4 ArtistNative2975(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].xxxx))).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].x = ((g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (max((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].y = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 4: mad r0.x, -r0.x, cb0[2].z, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[2].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 5: mul_sat r0.x, r0.x, cb0[3].w
    r0.x = (saturate((r0.xxxx)*(source[3].wwww))).x;
    // 6: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 7: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 8: mul r0.y, r0.y, cb0[4].y
    r0.y = ((r0.yyyy)*(source[4].yyyy)).y;
    // 9: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 10: mul_sat r0.y, r0.y, cb0[4].z
    r0.y = (saturate((r0.yyyy)*(source[4].zzzz))).y;
    // 11: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 12: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 13: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 14: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 15: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 16: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 17: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_me_transition_04_5_ma: 42ebb4e66c0c0b4b92db497fcd69ccc2; selected map b8401645d4917a672f72d638bd2a891a0c48db04e09fef0da640805ec3f98110.
float4 ArtistNative2976(ARTIST_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[10]=float4(input.skyUpperColor,0.f);
    source[11]=float4(input.skyLowerColor,0.f);
    source[12]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3] = g_ArtistSourceMaterialParameters[4u];
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[6].x = (((float4(1.5, 0.0, 0.0, 0.0)+sin((g_ArtistSourceMaterialTime.xxxx*float4(3.14159274, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[6].y = ((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[6].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].x = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[7].y = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[5]; [unroll] for(uint passIndex=0u;passIndex<5u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.
    passValues[4]=float4(0.f,0.f,0.f,1.f); // Neutral specular override; only the original secondary MRT consumes it.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 4: add r0.x, -r0.x, l(1.300000)
    r0.x = ((-(r0.xxxx))+(float4(1.300000,1.300000,1.300000,1.300000))).x;
    // 5: mad r0.x, -cb0[6].w, l(1.300000), r0.x
    r0.x = ((-(source[6].wwww))*(float4(1.300000,1.300000,1.300000,1.300000))+(r0.xxxx)).x;
    // 6: mul_sat r0.x, r0.x, l(4.000000)
    r0.x = (saturate((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000)))).x;
    // 7: mad r1.xyzw, v4.xyxy, l(1.500000, 1.500000, 5.250000, 5.250000), cb0[5].xyxy
    r1.xyzw = ((v4.xyxy)*(float4(1.500000,1.500000,5.250000,5.250000))+(source[5].xyxy)).xyzw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t2.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t2.yzxw, s4, l(0.000000)
    r0.z = (ArtistNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 10: mul r0.z, r0.z, r0.y
    r0.z = ((r0.zzzz)*(r0.yyyy)).z;
    // 11: mul r0.z, r0.z, cb0[7].z
    r0.z = ((r0.zzzz)*(source[7].zzzz)).z;
    // 12: mad r0.w, r0.y, r0.x, r0.x
    r0.w = ((r0.yyyy)*(r0.xxxx)+(r0.xxxx)).w;
    // 13: mul r0.y, r0.x, r0.y
    r0.y = ((r0.xxxx)*(r0.yyyy)).y;
    // 14: mul r1.x, |r0.w|, |r0.w|
    r1.x = ((abs(r0.wwww))*(abs(r0.wwww))).x;
    // 15: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 16: mul r1.x, |r0.w|, r1.x
    r1.x = ((abs(r0.wwww))*(r1.xxxx)).x;
    // 17: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 18: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 19: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r1.xyzw = (ArtistNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 21: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 22: mad r0.w, r0.w, cb0[8].y, l(-0.333000)
    r0.w = ((r0.wwww)*(source[8].yyyy)+(float4(-0.333000,-0.333000,-0.333000,-0.333000))).w;
    // 23: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 24: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) clip(-1.f);
    // 25: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 26: add r2.xy, v4.xyxx, cb0[4].xyxx
    r2.xy = ((v4.xyxx)+(source[4].xyxx)).xy;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t5.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 28: mul r1.w, cb0[6].w, l(1.300000)
    r1.w = ((source[6].wwww)*(float4(1.300000,1.300000,1.300000,1.300000))).w;
    // 29: mad r2.x, -r0.w, r1.w, l(1.000000)
    r2.x = ((-(r0.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 30: mad r2.yz, r0.wwww, r1.wwww, v4.xxyx
    r2.yz = ((r0.wwww)*(r1.wwww)+(v4.xxyx)).yz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r2.yzw, r2.yzyy, t1.wxyz, s3, l(0.000000)
    r2.yzw = (ArtistNativeSample3((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 32: mad r0.x, r2.x, r0.x, r0.y
    r0.x = ((r2.xxxx)*(r0.xxxx)+(r0.yyyy)).x;
    // 33: mul r0.y, |r0.x|, |r0.x|
    r0.y = ((abs(r0.xxxx))*(abs(r0.xxxx))).y;
    // 34: mul r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 35: mul r0.y, r0.y, |r0.x|
    r0.y = ((r0.yyyy)*(abs(r0.xxxx))).y;
    // 36: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 37: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 38: add r0.y, r0.y, l(-1.000000)
    r0.y = ((r0.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 39: movc r0.x, r0.x, l(1.000000), -r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (-(r0.yyyy))).x;
    // 40: max r3.xyz, |r2.yzwy|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r2.yzwy),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 41: mul r2.xyz, r2.yzwy, l(5.000000, 5.000000, 5.000000, 0.000000)
    r2.xyz = ((r2.yzwy)*(float4(5.000000,5.000000,5.000000,0.000000))).xyz;
    // 42: mul r4.xyz, r3.xyzx, r3.xyzx
    r4.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 43: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 44: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 45: mad r2.xyz, r3.xyzx, l(250.000000, 250.000000, 250.000000, 0.000000), r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(250.000000,250.000000,250.000000,0.000000))+(r2.xyzx)).xyz;
    // 46: mul r0.yzw, r0.zzzz, r2.xxyz
    r0.yzw = ((r0.zzzz)*(r2.xxyz)).yzw;
    // 47: mul r2.xyz, cb0[3].xyzx, cb0[3].wwww
    r2.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 48: mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // 49: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 50: mul r0.xyz, r0.xyzx, l(20.000000, 20.000000, 20.000000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(20.000000,20.000000,20.000000,0.000000))).xyz;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 52: mul r2.xyz, r2.xyzx, cb0[6].xxxx
    r2.xyz = ((r2.xyzx)*(source[6].xxxx)).xyz;
    // 53: mul r3.xyz, cb0[2].xyzx, cb0[2].wwww
    r3.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 54: mad r0.xyz, r3.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 55: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 57: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 58: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 59: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 60: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 61: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 62: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 63: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 64: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 65: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 66: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 67: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 68: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 69: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 70: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 71: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 72: mul r3.yzw, r3.yyyy, cb0[11].xxyz
    r3.yzw = ((r3.yyyy)*(source[11].xxyz)).yzw;
    // 73: mad r3.xyz, r3.xxxx, cb0[10].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[10].xyzx)+(r3.yzwy)).xyz;
    // 74: mul r3.xyz, r3.xyzx, cb0[12].wwww
    r3.xyz = ((r3.xyzx)*(source[12].wwww)).xyz;
    // 75: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 76: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 77: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 78: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 79: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 81: mad o0.xyz, r1.xyzx, cb0[12].xyzx, r0.xyzx
    output.xyz = ((r1.xyzx)*(source[12].xyzx)+(r0.xyzx)).xyz;
    // 83: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_pa_holomatrix_02_ad: fb6f0054b2bc094ab3b058418968b930; selected map 2c09cbb0d17f9c33729df595991cc7720593b3174b589e1ac3c334dc8c0eb826.
float4 ArtistNative2977(ARTIST_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0199999996, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.150000006, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[7] = g_ArtistSourceMaterialParameters[1u];
    source[8] = g_ArtistSourceMaterialParameters[2u];
    source[9] = g_ArtistSourceMaterialParameters[4u];
    source[10].x = (((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[10].y = ((float4(0.00499999989, 0.0, 0.0, 0.0)+((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[10].z = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[10].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[11].x = (((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.0199999996, 0.0, 0.0, 0.0))).x;
    source[11].y = ((float4(0.100000001, 0.0, 0.0, 0.0)+((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[11].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[11].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[12].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[12].z = ((float4(1.5, 0.0, 0.0, 0.0)+(sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))))).x;
    source[12].w = (((float4(1.5, 0.0, 0.0, 0.0)+(sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    float4 passValues[7]; [unroll] for(uint passIndex=0u;passIndex<7u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    uint viewportWidth, viewportHeight; g_EffectSceneDepthTexture.GetDimensions(viewportWidth,viewportHeight);
    passValues[6]=float4(max(float2(viewportWidth,viewportHeight),1.f),0.f,0.f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v2.xyxx, cb0[3].xyxx
    r0.xy = ((v2.xyxx)+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mul r0.x, r0.x, cb0[11].y
    r0.x = ((r0.xxxx)*(source[11].yyyy)).x;
    // 4: mad r0.yz, v2.xxyx, l(0.000000, 1.000000, 3.000000, 0.000000), cb0[2].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,1.000000,3.000000,0.000000))+(source[2].xxyx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mad r0.x, r0.y, cb0[10].y, r0.x
    r0.x = ((r0.yyyy)*(source[10].yyyy)+(r0.xxxx)).x;
    // 7: add r0.xy, r0.xxxx, v2.xyxx
    r0.xy = ((r0.xxxx)+(v2.xyxx)).xy;
    // 8: add r0.zw, r0.xxxy, cb0[4].xxxy
    r0.zw = ((r0.xxxy)+(source[4].xxxy)).zw;
    // 9: mul r0.xy, r0.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // 11: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 14: mul r0.z, r0.z, l(1.500000)
    r0.z = ((r0.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 15: mad r1.xy, v2.xyxx, l(3.000000, 2.000000, 0.000000, 0.000000), r0.zzzz
    r1.xy = ((v2.xyxx)*(float4(3.000000,2.000000,0.000000,0.000000))+(r0.zzzz)).xy;
    // 16: mad r0.zw, v2.xxxy, l(0.000000, 0.000000, 5.000000, 3.000000), r0.zzzz
    r0.zw = ((v2.xxxy)*(float4(0.000000,0.000000,5.000000,3.000000))+(r0.zzzz)).zw;
    // 17: add r0.zw, r0.zzzw, cb0[6].xxxy
    r0.zw = ((r0.zzzw)+(source[6].xxxy)).zw;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 19: add r0.zw, r1.xxxy, cb0[5].xxxy
    r0.zw = ((r1.xxxy)+(source[5].xxxy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 22: mul r1.xyz, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r1.xyzx)*(source[7].xyzx)).xyz;
    // 23: mul r1.xyz, r1.xyzx, l(40.000000, 40.000000, 40.000000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(40.000000,40.000000,40.000000,0.000000))).xyz;
    // 24: mul r0.zw, r0.xxxy, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.xxxy)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 25: lt r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((r0.xyxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 26: log r0.zw, r0.zzzw
    r0.zw = (log2(r0.zzzw)).zw;
    // 27: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 25.000000, 25.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,25.000000,25.000000))).zw;
    // 28: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 29: movc r0.xy, r0.xyxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 30: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 31: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 32: mul r2.xy, r0.yzyy, r0.yzyy
    r2.xy = ((r0.yzyy)*(r0.yzyy)).xy;
    // 33: mul r2.zw, r2.xxxy, l(0.000000, 0.000000, 4.000000, 4.000000)
    r2.zw = ((r2.xxxy)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 34: lt r2.xy, r2.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r2.xyxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 35: log r2.zw, r2.zzzw
    r2.zw = (log2(r2.zzzw)).zw;
    // 36: mul r2.zw, r2.zzzw, l(0.000000, 0.000000, 10.000000, 10.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,10.000000,10.000000))).zw;
    // 37: exp r2.zw, r2.zzzw
    r2.zw = (exp2(r2.zzzw)).zw;
    // 38: movc r2.xy, r2.xyxx, l(0,0,0,0), r2.zwzz
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zwzz)).xy;
    // 39: add r0.w, r2.y, r2.x
    r0.w = ((r2.yyyy)+(r2.xxxx)).w;
    // 40: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: dp2 r1.w, l(0.707107, -0.707107, 0.000000, 0.000000), r0.yzyy
    r1.w = (dot((float4(0.707107,-0.707107,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).w;
    // 42: dp2 r2.x, l(0.707107, 0.707107, 0.000000, 0.000000), r0.yzyy
    r2.x = (dot((float4(0.707107,0.707107,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).x;
    // 43: add r1.w, |r1.w|, |r2.x|
    r1.w = ((abs(r1.wwww))+(abs(r2.xxxx))).w;
    // 44: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 45: mul_sat r1.w, r1.w, l(1.650000)
    r1.w = (saturate((r1.wwww)*(float4(1.650000,1.650000,1.650000,1.650000)))).w;
    // 46: log r2.x, r1.w
    r2.x = (log2(r1.wwww)).x;
    // 47: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 48: mul r2.x, r2.x, l(15.000000)
    r2.x = ((r2.xxxx)*(float4(15.000000,15.000000,15.000000,15.000000))).x;
    // 49: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 50: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 51: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 52: mul r2.xyz, r1.wwww, cb0[8].xyzx
    r2.xyz = ((r1.wwww)*(source[8].xyzx)).xyz;
    // 53: div r1.w, cb2[6].x, cb2[6].y
    r1.w = ((passValues[6].xxxx)/(passValues[6].yyyy)).w;
    // 54: mul r1.w, r1.w, cb0[12].x
    r1.w = ((r1.wwww)*(source[12].xxxx)).w;
    // 55: mul r3.x, r0.y, r1.w
    r3.x = ((r0.yyyy)*(r1.wwww)).x;
    // 56: mul r3.y, r0.z, cb0[12].y
    r3.y = ((r0.zzzz)*(source[12].yyyy)).y;
    // 57: add r0.yz, r3.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r3.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t3.xwyz, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 59: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 60: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 61: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 62: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 63: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 64: mad r0.x, r0.x, r0.y, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)+(r0.yyyy)).x;
    // 65: mul r0.xyz, r0.xxxx, cb0[9].xyzx
    r0.xyz = ((r0.xxxx)*(source[9].xyzx)).xyz;
    // 66: mad r0.xyz, r1.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 67: mul r1.xyz, r0.wwww, cb0[7].xyzx
    r1.xyz = ((r0.wwww)*(source[7].xyzx)).xyz;
    // 68: mul r0.w, r0.w, cb0[12].w
    r0.w = ((r0.wwww)*(source[12].wwww)).w;
    // 69: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 70: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 71: mad_sat r0.xyz, r1.xyzx, l(0.050000, 0.050000, 0.050000, 0.000000), r0.xyzx
    r0.xyz = (saturate((r1.xyzx)*(float4(0.050000,0.050000,0.050000,0.000000))+(r0.xyzx))).xyz;
    // 72: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 73: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 74: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 75: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_ht_05_1_ad: 32cc9fb5a59d0a40bf45e83c5d359574; selected map aed8a2daebfa06101bafac31a70e0401280d4bde00e6a2cc605b86ec552e4f2f.
float4 ArtistNative2978(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t1.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 2: mul r0.x, r0.x, cb0[3].x
    r0.x = ((r0.xxxx)*(source[3].xxxx)).x;
    // 3: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 4: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 5: mul r0.y, r0.y, cb0[3].y
    r0.y = ((r0.yyyy)*(source[3].yyyy)).y;
    // 6: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 7: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 8: mad r0.y, v4.x, l(2.000000), l(-1.000000)
    r0.y = ((v4.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 9: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 10: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 11: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 12: mul r0.y, r0.y, cb0[3].z
    r0.y = ((r0.yyyy)*(source[3].zzzz)).y;
    // 13: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 14: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 16: mad r0.y, cb0[3].w, v4.z, l(-1.000000)
    r0.y = ((source[3].wwww)*(v4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 17: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 18: mul r0.z, v4.z, cb0[3].w
    r0.z = ((v4.zzzz)*(source[3].wwww)).z;
    // 19: mad r0.yz, r0.zzzz, v2.xxyx, -r0.yyyy
    r0.yz = ((r0.zzzz)*(v2.xxyx)+(-(r0.yyyy))).yz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t0.xyzw, s1, l(0.000000)
    r1.xyzw = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 21: log r0.y, |r1.w|
    r0.y = (log2(abs(r1.wwww))).y;
    // 22: mul r0.y, r0.y, cb0[4].z
    r0.y = ((r0.yyyy)*(source[4].zzzz)).y;
    // 23: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 24: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 25: lt r0.z, |r1.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: movc r0.y, r0.z, l(0), |r0.y|
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).y;
    // 27: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 28: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 29: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 30: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 31: mul r0.z, r0.x, r0.z
    r0.z = ((r0.xxxx)*(r0.zzzz)).z;
    // 32: ge r0.x, l(0.250000), r0.x
    r0.x = (asfloat((uint4)((float4(0.250000,0.250000,0.250000,0.250000))>=(r0.xxxx)) * 0xffffffffu)).x;
    // 33: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 34: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 35: mul r2.xyz, r1.xyzx, cb0[4].xxxx
    r2.xyz = ((r1.xyzx)*(source[4].xxxx)).xyz;
    // 36: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 37: mad r1.xyz, -cb0[4].xxxx, r1.xyzx, r0.zzzz
    r1.xyz = ((-(source[4].xxxx))*(r1.xyzx)+(r0.zzzz)).xyz;
    // 38: mad r1.xyz, cb0[4].yyyy, r1.xyzx, r2.xyzx
    r1.xyz = ((source[4].yyyy)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 39: mul r1.xyz, r1.xyzx, v3.xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)).xyz;
    // 40: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 41: movc r0.xzw, r0.xxxx, r2.xxyz, r1.xxyz
    r0.xzw = ((asuint(r0.xxxx) != 0u) ? (r2.xxyz) : (r1.xxyz)).xzw;
    // 42: add r0.xzw, r0.xxzw, cb0[1].xxyz
    r0.xzw = ((r0.xxzw)+(source[1].xxyz)).xzw;
    // 43: mul r0.xzw, r0.xxzw, v5.wwww
    r0.xzw = ((r0.xxzw)*(v5.wwww)).xzw;
    // 44: mul o0.xyz, r0.yyyy, r0.xzwx
    output.xyz = ((r0.yyyy)*(r0.xzwx)).xyz;
    // 45: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_h_pa_trails_02_tr: af23fe2db02b2545ba5f2dcfed2754d8; selected map f4003a50607a6e4b778720c3fa243fffd5714872bd43d6428b1b1500d8251462.
float4 ArtistNative2979(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialTime.xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: add r0.xy, v2.yxyy, l(-0.500000, -0.200000, 0.000000, 0.000000)
    r0.xy = ((v2.yxyy)+(float4(-0.500000,-0.200000,0.000000,0.000000))).xy;
    // 2: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 3: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 4: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 5: mul r0.z, r0.x, r0.y
    r0.z = ((r0.xxxx)*(r0.yyyy)).z;
    // 6: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 7: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 8: mad r0.x, r0.x, l(0.500000), r0.z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.zzzz)).x;
    // 9: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 10: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 11: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 12: mul r0.z, r0.y, l(10.000000)
    r0.z = ((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).z;
    // 13: mul r0.y, r0.y, cb0[2].x
    r0.y = ((r0.yyyy)*(source[2].xxxx)).y;
    // 14: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, cb0[2].y
    r0.y = ((r0.yyyy)*(source[2].yyyy)).y;
    // 16: movc r0.y, r0.x, l(0), r0.y
    r0.y = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 17: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 18: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 19: mul r0.z, r0.x, l(-50.000000)
    r0.z = ((r0.xxxx)*(float4(-50.000000,-50.000000,-50.000000,-50.000000))).z;
    // 20: max r0.z, r0.z, l(-1.000000)
    r0.z = (max(r0.zzzz,float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 21: add r0.z, -r0.y, r0.z
    r0.z = ((-(r0.yyyy))+(r0.zzzz)).z;
    // 22: max r0.z, r0.z, l(-0.100000)
    r0.z = (max(r0.zzzz,float4(-0.100000,-0.100000,-0.100000,-0.100000))).z;
    // 23: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 24: mad r0.yzw, r0.yyyy, v3.xxyz, r0.zzzz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)+(r0.zzzz)).yzw;
    // 25: mul r1.x, v4.x, cb0[2].z
    r1.x = ((v4.xxxx)*(source[2].zzzz)).x;
    // 26: mul r1.yz, r1.xxxx, l(0.000000, -0.500000, -0.050000, 0.000000)
    r1.yz = ((r1.xxxx)*(float4(0.000000,-0.500000,-0.050000,0.000000))).yz;
    // 27: mad r1.yz, v2.xxyx, l(0.000000, 1.500000, 1.000000, 0.000000), r1.yyzy
    r1.yz = ((v2.xxyx)*(float4(0.000000,1.500000,1.000000,0.000000))+(r1.yyzy)).yz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t0.yxzw, s0, l(0.000000)
    r1.y = (ArtistNativeSample0((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 29: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 30: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 31: mul r1.z, r1.z, l(0.800000)
    r1.z = ((r1.zzzz)*(float4(0.800000,0.800000,0.800000,0.800000))).z;
    // 32: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 33: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 34: mul r2.xyz, v2.xxyx, l(5.000000, 1.000000, 1.500000, 0.000000)
    r2.xyz = ((v2.xxyx)*(float4(5.000000,1.000000,1.500000,0.000000))).xyz;
    // 35: mad r1.xz, r1.xxxx, l(-0.350000, 0.000000, 0.050000, 0.000000), r2.yyzy
    r1.xz = ((r1.xxxx)*(float4(-0.350000,0.000000,0.050000,0.000000))+(r2.yyzy)).xz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xzxx, t0.xyzw, s0, l(0.000000)
    r1.x = (ArtistNativeSample0((r1.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 37: log r1.z, |r1.x|
    r1.z = (log2(abs(r1.xxxx))).z;
    // 38: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 39: mul r1.z, r1.z, l(0.800000)
    r1.z = ((r1.zzzz)*(float4(0.800000,0.800000,0.800000,0.800000))).z;
    // 40: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 41: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 42: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 43: mul r1.x, r1.x, l(25.000000)
    r1.x = ((r1.xxxx)*(float4(25.000000,25.000000,25.000000,25.000000))).x;
    // 44: mad r0.yzw, r1.xxxx, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r1.xxxx)*(r0.yyzw)+(source[1].xxyz)).yzw;
    // 45: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 46: log r0.y, |r2.x|
    r0.y = (log2(abs(r2.xxxx))).y;
    // 47: lt r0.z, |r2.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 48: mul r0.y, r0.y, l(1.200000)
    r0.y = ((r0.yyyy)*(float4(1.200000,1.200000,1.200000,1.200000))).y;
    // 49: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 50: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 51: add r0.z, -v2.y, l(1.000000)
    r0.z = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 52: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 53: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 54: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 55: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_master_01_04_ma: de86fd3fa6797846b5c8cd2d2319cf95; selected map 1abe0285a4383ccaf73a29523f0f1ddff922a64a56a2a7e7564c9fdcc80d2b70.
float4 ArtistNative2980(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[3u];
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xy, v6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((v6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 2: frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // 3: mov_sat r0.z, v3.w
    r0.z = (saturate(v3.wwww)).z;
    // 4: add r0.xy, r0.zzzz, r0.xyxx
    r0.xy = ((r0.zzzz)+(r0.xyxx)).xy;
    // 5: round_ni r0.xy, r0.xyxx
    r0.xy = (floor(r0.xyxx)).xy;
    // 6: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 7: frc r0.yw, v6.xxxy
    r0.yw = (frac(v6.xxxy)).yw;
    // 8: add r0.yz, r0.zzzz, r0.yywy
    r0.yz = ((r0.zzzz)+(r0.yywy)).yz;
    // 9: round_ni r0.yz, r0.yyzy
    r0.yz = (floor(r0.yyzy)).yz;
    // 10: mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // 11: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: mul r0.yz, v2.xxyx, cb0[2].zzwz
    r0.yz = ((v2.xxyx)*(source[2].zzwz)).yz;
    // 13: mad r1.x, cb0[2].y, cb0[2].x, r0.y
    r1.x = ((source[2].yyyy)*(source[2].xxxx)+(r0.yyyy)).x;
    // 14: mad r1.y, cb0[2].y, cb0[3].x, r0.z
    r1.y = ((source[2].yyyy)*(source[3].xxxx)+(r0.zzzz)).y;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 16: mul_sat r0.y, r0.y, cb0[3].y
    r0.y = (saturate((r0.yyyy)*(source[3].yyyy))).y;
    // 17: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 18: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 19: mul r0.z, r0.z, cb0[3].z
    r0.z = ((r0.zzzz)*(source[3].zzzz)).z;
    // 20: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 21: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 22: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: add r0.x, r0.x, l(-0.166000)
    r0.x = ((r0.xxxx)+(float4(-0.166000,-0.166000,-0.166000,-0.166000))).x;
    // 24: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 25: or r0.x, r0.x, r0.y
    r0.x = (asfloat(asuint(r0.xxxx) | asuint(r0.yyyy))).x;
    // 26: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 27: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 28: mul r0.xyz, cb0[1].xyzx, cb0[1].wwww
    r0.xyz = ((source[1].xyzx)*(source[1].wwww)).xyz;
    // 29: mad o0.xyz, r0.xyzx, v3.xyzx, cb0[0].xyzx
    output.xyz = ((r0.xyzx)*(v3.xyzx)+(source[0].xyzx)).xyz;
    // 30: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_master_01_050_ad: dbae20996679df47b23fb48bc3c8c05f; selected map f9eee02cdeb184b461d1f0bb00694a339f7f85d16f18d4b558de93bcf4d99968.
float4 ArtistNative2981(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].yyyy)).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mul r0.xy, v2.xyxx, cb0[4].zwzz
    r0.xy = ((v2.xyxx)*(source[4].zwzz)).xy;
    // 2: mul r0.z, cb0[3].x, cb0[3].y
    r0.z = ((source[3].xxxx)*(source[3].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[4].y, r0.x
    r1.x = ((r0.zzzz)*(source[4].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[5].x, r0.y
    r1.y = ((r0.zzzz)*(source[5].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[5].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[5].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 7: mul r0.w, r0.x, cb0[3].w
    r0.w = ((r0.xxxx)*(source[3].wwww)).w;
    // 8: mad r1.x, r0.z, cb0[3].z, r0.w
    r1.x = ((r0.zzzz)*(source[3].zzzz)+(r0.wwww)).x;
    // 9: mul r0.w, r0.z, cb0[5].w
    r0.w = ((r0.zzzz)*(source[5].wwww)).w;
    // 10: mad r1.y, cb0[4].x, r0.y, r0.w
    r1.y = ((source[4].xxxx)*(r0.yyyy)+(r0.wwww)).y;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 12: mul r2.xy, r0.xyxx, cb0[6].yzyy
    r2.xy = ((r0.xyxx)*(source[6].yzyy)).xy;
    // 13: mad r2.xy, r0.zzzz, cb0[6].xwxx, r2.xyxx
    r2.xy = ((r0.zzzz)*(source[6].xwxx)+(r2.xyxx)).xy;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 15: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 16: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 17: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 18: mad r1.xyz, cb0[7].xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((source[7].xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 19: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 20: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 21: mul r1.xyz, r1.xyzx, cb0[7].yyyy
    r1.xyz = ((r1.xyzx)*(source[7].yyyy)).xyz;
    // 22: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 23: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 24: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 25: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 26: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 27: mul r0.x, r0.x, cb0[7].w
    r0.x = ((r0.xxxx)*(source[7].wwww)).x;
    // 28: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 29: mad r2.y, r0.z, cb0[8].y, r0.y
    r2.y = ((r0.zzzz)*(source[8].yyyy)+(r0.yyyy)).y;
    // 30: mad r2.x, r0.z, cb0[7].z, r0.x
    r2.x = ((r0.zzzz)*(source[7].zzzz)+(r0.xxxx)).x;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 32: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 33: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 34: mul_sat r0.x, r0.x, cb0[8].z
    r0.x = (saturate((r0.xxxx)*(source[8].zzzz))).x;
    // 35: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 36: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 37: mul r0.x, r0.x, cb0[8].w
    r0.x = ((r0.xxxx)*(source[8].wwww)).x;
    // 38: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 39: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 40: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 41: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 42: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 43: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_pa_holomatrix_02_1_ad: fb6f0054b2bc094ab3b058418968b930; selected map 5162752975f31492ad1c2807790783042d9035ac5287367c6566647ab8cc81d6.
float4 ArtistNative2982(ARTIST_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0199999996, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.150000006, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[7] = g_ArtistSourceMaterialParameters[1u];
    source[8] = g_ArtistSourceMaterialParameters[2u];
    source[9] = g_ArtistSourceMaterialParameters[4u];
    source[10].x = (((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[10].y = ((float4(0.00499999989, 0.0, 0.0, 0.0)+((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[10].z = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[10].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[11].x = (((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.0199999996, 0.0, 0.0, 0.0))).x;
    source[11].y = ((float4(0.100000001, 0.0, 0.0, 0.0)+((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[11].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[11].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[12].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[12].z = ((float4(1.5, 0.0, 0.0, 0.0)+(sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))))).x;
    source[12].w = (((float4(1.5, 0.0, 0.0, 0.0)+(sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    float4 passValues[7]; [unroll] for(uint passIndex=0u;passIndex<7u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    uint viewportWidth, viewportHeight; g_EffectSceneDepthTexture.GetDimensions(viewportWidth,viewportHeight);
    passValues[6]=float4(max(float2(viewportWidth,viewportHeight),1.f),0.f,0.f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v2.xyxx, cb0[3].xyxx
    r0.xy = ((v2.xyxx)+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mul r0.x, r0.x, cb0[11].y
    r0.x = ((r0.xxxx)*(source[11].yyyy)).x;
    // 4: mad r0.yz, v2.xxyx, l(0.000000, 1.000000, 3.000000, 0.000000), cb0[2].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,1.000000,3.000000,0.000000))+(source[2].xxyx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mad r0.x, r0.y, cb0[10].y, r0.x
    r0.x = ((r0.yyyy)*(source[10].yyyy)+(r0.xxxx)).x;
    // 7: add r0.xy, r0.xxxx, v2.xyxx
    r0.xy = ((r0.xxxx)+(v2.xyxx)).xy;
    // 8: add r0.zw, r0.xxxy, cb0[4].xxxy
    r0.zw = ((r0.xxxy)+(source[4].xxxy)).zw;
    // 9: mul r0.xy, r0.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // 11: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 14: mul r0.z, r0.z, l(1.500000)
    r0.z = ((r0.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 15: mad r1.xy, v2.xyxx, l(3.000000, 2.000000, 0.000000, 0.000000), r0.zzzz
    r1.xy = ((v2.xyxx)*(float4(3.000000,2.000000,0.000000,0.000000))+(r0.zzzz)).xy;
    // 16: mad r0.zw, v2.xxxy, l(0.000000, 0.000000, 5.000000, 3.000000), r0.zzzz
    r0.zw = ((v2.xxxy)*(float4(0.000000,0.000000,5.000000,3.000000))+(r0.zzzz)).zw;
    // 17: add r0.zw, r0.zzzw, cb0[6].xxxy
    r0.zw = ((r0.zzzw)+(source[6].xxxy)).zw;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 19: add r0.zw, r1.xxxy, cb0[5].xxxy
    r0.zw = ((r1.xxxy)+(source[5].xxxy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 22: mul r1.xyz, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r1.xyzx)*(source[7].xyzx)).xyz;
    // 23: mul r1.xyz, r1.xyzx, l(40.000000, 40.000000, 40.000000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(40.000000,40.000000,40.000000,0.000000))).xyz;
    // 24: mul r0.zw, r0.xxxy, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.xxxy)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 25: lt r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((r0.xyxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 26: log r0.zw, r0.zzzw
    r0.zw = (log2(r0.zzzw)).zw;
    // 27: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 25.000000, 25.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,25.000000,25.000000))).zw;
    // 28: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 29: movc r0.xy, r0.xyxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 30: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 31: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 32: mul r2.xy, r0.yzyy, r0.yzyy
    r2.xy = ((r0.yzyy)*(r0.yzyy)).xy;
    // 33: mul r2.zw, r2.xxxy, l(0.000000, 0.000000, 4.000000, 4.000000)
    r2.zw = ((r2.xxxy)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 34: lt r2.xy, r2.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r2.xyxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 35: log r2.zw, r2.zzzw
    r2.zw = (log2(r2.zzzw)).zw;
    // 36: mul r2.zw, r2.zzzw, l(0.000000, 0.000000, 10.000000, 10.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,10.000000,10.000000))).zw;
    // 37: exp r2.zw, r2.zzzw
    r2.zw = (exp2(r2.zzzw)).zw;
    // 38: movc r2.xy, r2.xyxx, l(0,0,0,0), r2.zwzz
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zwzz)).xy;
    // 39: add r0.w, r2.y, r2.x
    r0.w = ((r2.yyyy)+(r2.xxxx)).w;
    // 40: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: dp2 r1.w, l(0.707107, -0.707107, 0.000000, 0.000000), r0.yzyy
    r1.w = (dot((float4(0.707107,-0.707107,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).w;
    // 42: dp2 r2.x, l(0.707107, 0.707107, 0.000000, 0.000000), r0.yzyy
    r2.x = (dot((float4(0.707107,0.707107,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).x;
    // 43: add r1.w, |r1.w|, |r2.x|
    r1.w = ((abs(r1.wwww))+(abs(r2.xxxx))).w;
    // 44: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 45: mul_sat r1.w, r1.w, l(1.650000)
    r1.w = (saturate((r1.wwww)*(float4(1.650000,1.650000,1.650000,1.650000)))).w;
    // 46: log r2.x, r1.w
    r2.x = (log2(r1.wwww)).x;
    // 47: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 48: mul r2.x, r2.x, l(15.000000)
    r2.x = ((r2.xxxx)*(float4(15.000000,15.000000,15.000000,15.000000))).x;
    // 49: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 50: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 51: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 52: mul r2.xyz, r1.wwww, cb0[8].xyzx
    r2.xyz = ((r1.wwww)*(source[8].xyzx)).xyz;
    // 53: div r1.w, cb2[6].x, cb2[6].y
    r1.w = ((passValues[6].xxxx)/(passValues[6].yyyy)).w;
    // 54: mul r1.w, r1.w, cb0[12].x
    r1.w = ((r1.wwww)*(source[12].xxxx)).w;
    // 55: mul r3.x, r0.y, r1.w
    r3.x = ((r0.yyyy)*(r1.wwww)).x;
    // 56: mul r3.y, r0.z, cb0[12].y
    r3.y = ((r0.zzzz)*(source[12].yyyy)).y;
    // 57: add r0.yz, r3.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r3.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t3.xwyz, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 59: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 60: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 61: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 62: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 63: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 64: mad r0.x, r0.x, r0.y, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)+(r0.yyyy)).x;
    // 65: mul r0.xyz, r0.xxxx, cb0[9].xyzx
    r0.xyz = ((r0.xxxx)*(source[9].xyzx)).xyz;
    // 66: mad r0.xyz, r1.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 67: mul r1.xyz, r0.wwww, cb0[7].xyzx
    r1.xyz = ((r0.wwww)*(source[7].xyzx)).xyz;
    // 68: mul r0.w, r0.w, cb0[12].w
    r0.w = ((r0.wwww)*(source[12].wwww)).w;
    // 69: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 70: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 71: mad_sat r0.xyz, r1.xyzx, l(0.050000, 0.050000, 0.050000, 0.000000), r0.xyzx
    r0.xyz = (saturate((r1.xyzx)*(float4(0.050000,0.050000,0.050000,0.000000))+(r0.xyzx))).xyz;
    // 72: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 73: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 74: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 75: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_pa_holomatrix_02_2_ad: fb6f0054b2bc094ab3b058418968b930; selected map 5162752975f31492ad1c2807790783042d9035ac5287367c6566647ab8cc81d6.
float4 ArtistNative2983(ARTIST_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0199999996, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.150000006, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[7] = g_ArtistSourceMaterialParameters[1u];
    source[8] = g_ArtistSourceMaterialParameters[2u];
    source[9] = g_ArtistSourceMaterialParameters[4u];
    source[10].x = (((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[10].y = ((float4(0.00499999989, 0.0, 0.0, 0.0)+((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[10].z = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[10].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[11].x = (((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.0199999996, 0.0, 0.0, 0.0))).x;
    source[11].y = ((float4(0.100000001, 0.0, 0.0, 0.0)+((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[11].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[11].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[12].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[12].z = ((float4(1.5, 0.0, 0.0, 0.0)+(sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))))).x;
    source[12].w = (((float4(1.5, 0.0, 0.0, 0.0)+(sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    float4 passValues[7]; [unroll] for(uint passIndex=0u;passIndex<7u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    uint viewportWidth, viewportHeight; g_EffectSceneDepthTexture.GetDimensions(viewportWidth,viewportHeight);
    passValues[6]=float4(max(float2(viewportWidth,viewportHeight),1.f),0.f,0.f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v2.xyxx, cb0[3].xyxx
    r0.xy = ((v2.xyxx)+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mul r0.x, r0.x, cb0[11].y
    r0.x = ((r0.xxxx)*(source[11].yyyy)).x;
    // 4: mad r0.yz, v2.xxyx, l(0.000000, 1.000000, 3.000000, 0.000000), cb0[2].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,1.000000,3.000000,0.000000))+(source[2].xxyx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mad r0.x, r0.y, cb0[10].y, r0.x
    r0.x = ((r0.yyyy)*(source[10].yyyy)+(r0.xxxx)).x;
    // 7: add r0.xy, r0.xxxx, v2.xyxx
    r0.xy = ((r0.xxxx)+(v2.xyxx)).xy;
    // 8: add r0.zw, r0.xxxy, cb0[4].xxxy
    r0.zw = ((r0.xxxy)+(source[4].xxxy)).zw;
    // 9: mul r0.xy, r0.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // 11: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 14: mul r0.z, r0.z, l(1.500000)
    r0.z = ((r0.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 15: mad r1.xy, v2.xyxx, l(3.000000, 2.000000, 0.000000, 0.000000), r0.zzzz
    r1.xy = ((v2.xyxx)*(float4(3.000000,2.000000,0.000000,0.000000))+(r0.zzzz)).xy;
    // 16: mad r0.zw, v2.xxxy, l(0.000000, 0.000000, 5.000000, 3.000000), r0.zzzz
    r0.zw = ((v2.xxxy)*(float4(0.000000,0.000000,5.000000,3.000000))+(r0.zzzz)).zw;
    // 17: add r0.zw, r0.zzzw, cb0[6].xxxy
    r0.zw = ((r0.zzzw)+(source[6].xxxy)).zw;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 19: add r0.zw, r1.xxxy, cb0[5].xxxy
    r0.zw = ((r1.xxxy)+(source[5].xxxy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 22: mul r1.xyz, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r1.xyzx)*(source[7].xyzx)).xyz;
    // 23: mul r1.xyz, r1.xyzx, l(40.000000, 40.000000, 40.000000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(40.000000,40.000000,40.000000,0.000000))).xyz;
    // 24: mul r0.zw, r0.xxxy, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.xxxy)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 25: lt r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((r0.xyxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 26: log r0.zw, r0.zzzw
    r0.zw = (log2(r0.zzzw)).zw;
    // 27: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 25.000000, 25.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,25.000000,25.000000))).zw;
    // 28: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 29: movc r0.xy, r0.xyxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 30: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 31: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 32: mul r2.xy, r0.yzyy, r0.yzyy
    r2.xy = ((r0.yzyy)*(r0.yzyy)).xy;
    // 33: mul r2.zw, r2.xxxy, l(0.000000, 0.000000, 4.000000, 4.000000)
    r2.zw = ((r2.xxxy)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 34: lt r2.xy, r2.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r2.xyxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 35: log r2.zw, r2.zzzw
    r2.zw = (log2(r2.zzzw)).zw;
    // 36: mul r2.zw, r2.zzzw, l(0.000000, 0.000000, 10.000000, 10.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,10.000000,10.000000))).zw;
    // 37: exp r2.zw, r2.zzzw
    r2.zw = (exp2(r2.zzzw)).zw;
    // 38: movc r2.xy, r2.xyxx, l(0,0,0,0), r2.zwzz
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zwzz)).xy;
    // 39: add r0.w, r2.y, r2.x
    r0.w = ((r2.yyyy)+(r2.xxxx)).w;
    // 40: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: dp2 r1.w, l(0.707107, -0.707107, 0.000000, 0.000000), r0.yzyy
    r1.w = (dot((float4(0.707107,-0.707107,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).w;
    // 42: dp2 r2.x, l(0.707107, 0.707107, 0.000000, 0.000000), r0.yzyy
    r2.x = (dot((float4(0.707107,0.707107,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).x;
    // 43: add r1.w, |r1.w|, |r2.x|
    r1.w = ((abs(r1.wwww))+(abs(r2.xxxx))).w;
    // 44: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 45: mul_sat r1.w, r1.w, l(1.650000)
    r1.w = (saturate((r1.wwww)*(float4(1.650000,1.650000,1.650000,1.650000)))).w;
    // 46: log r2.x, r1.w
    r2.x = (log2(r1.wwww)).x;
    // 47: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 48: mul r2.x, r2.x, l(15.000000)
    r2.x = ((r2.xxxx)*(float4(15.000000,15.000000,15.000000,15.000000))).x;
    // 49: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 50: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 51: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 52: mul r2.xyz, r1.wwww, cb0[8].xyzx
    r2.xyz = ((r1.wwww)*(source[8].xyzx)).xyz;
    // 53: div r1.w, cb2[6].x, cb2[6].y
    r1.w = ((passValues[6].xxxx)/(passValues[6].yyyy)).w;
    // 54: mul r1.w, r1.w, cb0[12].x
    r1.w = ((r1.wwww)*(source[12].xxxx)).w;
    // 55: mul r3.x, r0.y, r1.w
    r3.x = ((r0.yyyy)*(r1.wwww)).x;
    // 56: mul r3.y, r0.z, cb0[12].y
    r3.y = ((r0.zzzz)*(source[12].yyyy)).y;
    // 57: add r0.yz, r3.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r3.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t3.xwyz, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 59: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 60: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 61: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 62: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 63: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 64: mad r0.x, r0.x, r0.y, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)+(r0.yyyy)).x;
    // 65: mul r0.xyz, r0.xxxx, cb0[9].xyzx
    r0.xyz = ((r0.xxxx)*(source[9].xyzx)).xyz;
    // 66: mad r0.xyz, r1.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 67: mul r1.xyz, r0.wwww, cb0[7].xyzx
    r1.xyz = ((r0.wwww)*(source[7].xyzx)).xyz;
    // 68: mul r0.w, r0.w, cb0[12].w
    r0.w = ((r0.wwww)*(source[12].wwww)).w;
    // 69: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 70: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 71: mad_sat r0.xyz, r1.xyzx, l(0.050000, 0.050000, 0.050000, 0.000000), r0.xyzx
    r0.xyz = (saturate((r1.xyzx)*(float4(0.050000,0.050000,0.050000,0.000000))+(r0.xyzx))).xyz;
    // 72: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 73: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 74: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 75: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_pa_holomatrix_02_3_ad: fb6f0054b2bc094ab3b058418968b930; selected map 5162752975f31492ad1c2807790783042d9035ac5287367c6566647ab8cc81d6.
float4 ArtistNative2984(ARTIST_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0199999996, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.150000006, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[7] = g_ArtistSourceMaterialParameters[1u];
    source[8] = g_ArtistSourceMaterialParameters[2u];
    source[9] = g_ArtistSourceMaterialParameters[4u];
    source[10].x = (((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[10].y = ((float4(0.00499999989, 0.0, 0.0, 0.0)+((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[10].z = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[10].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[11].x = (((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.0199999996, 0.0, 0.0, 0.0))).x;
    source[11].y = ((float4(0.100000001, 0.0, 0.0, 0.0)+((sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[11].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[11].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[12].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[12].z = ((float4(1.5, 0.0, 0.0, 0.0)+(sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))))).x;
    source[12].w = (((float4(1.5, 0.0, 0.0, 0.0)+(sin(((float4(3.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    float4 passValues[7]; [unroll] for(uint passIndex=0u;passIndex<7u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    uint viewportWidth, viewportHeight; g_EffectSceneDepthTexture.GetDimensions(viewportWidth,viewportHeight);
    passValues[6]=float4(max(float2(viewportWidth,viewportHeight),1.f),0.f,0.f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v2.xyxx, cb0[3].xyxx
    r0.xy = ((v2.xyxx)+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mul r0.x, r0.x, cb0[11].y
    r0.x = ((r0.xxxx)*(source[11].yyyy)).x;
    // 4: mad r0.yz, v2.xxyx, l(0.000000, 1.000000, 3.000000, 0.000000), cb0[2].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,1.000000,3.000000,0.000000))+(source[2].xxyx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mad r0.x, r0.y, cb0[10].y, r0.x
    r0.x = ((r0.yyyy)*(source[10].yyyy)+(r0.xxxx)).x;
    // 7: add r0.xy, r0.xxxx, v2.xyxx
    r0.xy = ((r0.xxxx)+(v2.xyxx)).xy;
    // 8: add r0.zw, r0.xxxy, cb0[4].xxxy
    r0.zw = ((r0.xxxy)+(source[4].xxxy)).zw;
    // 9: mul r0.xy, r0.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // 11: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 14: mul r0.z, r0.z, l(1.500000)
    r0.z = ((r0.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 15: mad r1.xy, v2.xyxx, l(3.000000, 2.000000, 0.000000, 0.000000), r0.zzzz
    r1.xy = ((v2.xyxx)*(float4(3.000000,2.000000,0.000000,0.000000))+(r0.zzzz)).xy;
    // 16: mad r0.zw, v2.xxxy, l(0.000000, 0.000000, 5.000000, 3.000000), r0.zzzz
    r0.zw = ((v2.xxxy)*(float4(0.000000,0.000000,5.000000,3.000000))+(r0.zzzz)).zw;
    // 17: add r0.zw, r0.zzzw, cb0[6].xxxy
    r0.zw = ((r0.zzzw)+(source[6].xxxy)).zw;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 19: add r0.zw, r1.xxxy, cb0[5].xxxy
    r0.zw = ((r1.xxxy)+(source[5].xxxy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 22: mul r1.xyz, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r1.xyzx)*(source[7].xyzx)).xyz;
    // 23: mul r1.xyz, r1.xyzx, l(40.000000, 40.000000, 40.000000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(40.000000,40.000000,40.000000,0.000000))).xyz;
    // 24: mul r0.zw, r0.xxxy, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.xxxy)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 25: lt r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((r0.xyxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 26: log r0.zw, r0.zzzw
    r0.zw = (log2(r0.zzzw)).zw;
    // 27: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 25.000000, 25.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,25.000000,25.000000))).zw;
    // 28: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 29: movc r0.xy, r0.xyxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 30: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 31: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 32: mul r2.xy, r0.yzyy, r0.yzyy
    r2.xy = ((r0.yzyy)*(r0.yzyy)).xy;
    // 33: mul r2.zw, r2.xxxy, l(0.000000, 0.000000, 4.000000, 4.000000)
    r2.zw = ((r2.xxxy)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 34: lt r2.xy, r2.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r2.xyxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 35: log r2.zw, r2.zzzw
    r2.zw = (log2(r2.zzzw)).zw;
    // 36: mul r2.zw, r2.zzzw, l(0.000000, 0.000000, 10.000000, 10.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,10.000000,10.000000))).zw;
    // 37: exp r2.zw, r2.zzzw
    r2.zw = (exp2(r2.zzzw)).zw;
    // 38: movc r2.xy, r2.xyxx, l(0,0,0,0), r2.zwzz
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zwzz)).xy;
    // 39: add r0.w, r2.y, r2.x
    r0.w = ((r2.yyyy)+(r2.xxxx)).w;
    // 40: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: dp2 r1.w, l(0.707107, -0.707107, 0.000000, 0.000000), r0.yzyy
    r1.w = (dot((float4(0.707107,-0.707107,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).w;
    // 42: dp2 r2.x, l(0.707107, 0.707107, 0.000000, 0.000000), r0.yzyy
    r2.x = (dot((float4(0.707107,0.707107,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).x;
    // 43: add r1.w, |r1.w|, |r2.x|
    r1.w = ((abs(r1.wwww))+(abs(r2.xxxx))).w;
    // 44: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 45: mul_sat r1.w, r1.w, l(1.650000)
    r1.w = (saturate((r1.wwww)*(float4(1.650000,1.650000,1.650000,1.650000)))).w;
    // 46: log r2.x, r1.w
    r2.x = (log2(r1.wwww)).x;
    // 47: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 48: mul r2.x, r2.x, l(15.000000)
    r2.x = ((r2.xxxx)*(float4(15.000000,15.000000,15.000000,15.000000))).x;
    // 49: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 50: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 51: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 52: mul r2.xyz, r1.wwww, cb0[8].xyzx
    r2.xyz = ((r1.wwww)*(source[8].xyzx)).xyz;
    // 53: div r1.w, cb2[6].x, cb2[6].y
    r1.w = ((passValues[6].xxxx)/(passValues[6].yyyy)).w;
    // 54: mul r1.w, r1.w, cb0[12].x
    r1.w = ((r1.wwww)*(source[12].xxxx)).w;
    // 55: mul r3.x, r0.y, r1.w
    r3.x = ((r0.yyyy)*(r1.wwww)).x;
    // 56: mul r3.y, r0.z, cb0[12].y
    r3.y = ((r0.zzzz)*(source[12].yyyy)).y;
    // 57: add r0.yz, r3.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r3.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t3.xwyz, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 59: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 60: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 61: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 62: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 63: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 64: mad r0.x, r0.x, r0.y, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)+(r0.yyyy)).x;
    // 65: mul r0.xyz, r0.xxxx, cb0[9].xyzx
    r0.xyz = ((r0.xxxx)*(source[9].xyzx)).xyz;
    // 66: mad r0.xyz, r1.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 67: mul r1.xyz, r0.wwww, cb0[7].xyzx
    r1.xyz = ((r0.wwww)*(source[7].xyzx)).xyz;
    // 68: mul r0.w, r0.w, cb0[12].w
    r0.w = ((r0.wwww)*(source[12].wwww)).w;
    // 69: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 70: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 71: mad_sat r0.xyz, r1.xyzx, l(0.050000, 0.050000, 0.050000, 0.000000), r0.xyzx
    r0.xyz = (saturate((r1.xyzx)*(float4(0.050000,0.050000,0.050000,0.000000))+(r0.xyzx))).xyz;
    // 72: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 73: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 74: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 75: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_me_transition_04_4_ma: 496aee5a3382744d97f9dcdb89b6ae6c; selected map f9be08d5e66242bc426fcb73591edacf680f68a2acf227bb3d834507dd494ecc.
float4 ArtistNative2985(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.150000006, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.150000006, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = g_ArtistSourceMaterialParameters[3u];
    source[7] = input.dynamicParameter;
    source[8] = g_ArtistSourceMaterialParameters[1u];
    source[9] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[10].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v4.xyxx, cb0[9].xyxx
    r0.xy = ((v4.xyxx)+(source[9].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s5, l(0.000000)
    r0.xy = (ArtistNativeSample5((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mul r0.zw, r0.xxxy, cb0[10].wwww
    r0.zw = ((r0.xxxy)*(source[10].wwww)).zw;
    // 4: mad r0.xy, cb0[10].zzzz, r0.xyxx, v4.xyxx
    r0.xy = ((source[10].zzzz)*(r0.xyxx)+(v4.xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s5, l(0.000000)
    r0.x = (ArtistNativeSample5((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: mad r0.y, -cb0[10].y, cb0[7].x, l(1.000000)
    r0.y = ((-(source[10].yyyy))*(source[7].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 7: mad r0.yz, r0.yyyy, r0.zzwz, v4.xxyx
    r0.yz = ((r0.yyyy)*(r0.zzwz)+(v4.xxyx)).yz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t2.xyzw, s6, l(0.000000)
    r1.xyzw = (ArtistNativeSample6((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 9: mul r0.y, r1.w, l(1.150000)
    r0.y = ((r1.wwww)*(float4(1.150000,1.150000,1.150000,1.150000))).y;
    // 10: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 11: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 12: mul r0.z, r0.z, l(1.150000)
    r0.z = ((r0.zzzz)*(float4(1.150000,1.150000,1.150000,1.150000))).z;
    // 13: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 14: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 15: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 16: mul r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)*(r0.xxxx)).y;
    // 17: mul r0.z, cb0[7].x, cb0[10].y
    r0.z = ((source[7].xxxx)*(source[10].yyyy)).z;
    // 18: mad r0.w, r0.z, l(0.100000), l(0.900000)
    r0.w = ((r0.zzzz)*(float4(0.100000,0.100000,0.100000,0.100000))+(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 19: mad r0.y, r0.z, r0.y, r0.w
    r0.y = ((r0.zzzz)*(r0.yyyy)+(r0.wwww)).y;
    // 20: max r0.y, r0.y, l(0.900000)
    r0.y = (max(r0.yyyy,float4(0.900000,0.900000,0.900000,0.900000))).y;
    // 21: round_ni r0.y, r0.y
    r0.y = (floor(r0.yyyy)).y;
    // 22: min r1.w, r0.y, l(1.000000)
    r1.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: mad r1.w, cb0[0].w, r1.w, l(-0.333000)
    r1.w = ((source[0].wwww)*(r1.wwww)+(float4(-0.333000,-0.333000,-0.333000,-0.333000))).w;
    // 24: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 25: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) clip(-1.f);
    // 26: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 27: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 28: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: mul r1.w, r1.w, cb0[11].x
    r1.w = ((r1.wwww)*(source[11].xxxx)).w;
    // 30: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 31: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 32: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 33: add_sat r0.x, r0.x, r0.w
    r0.x = (saturate((r0.xxxx)+(r0.wwww))).x;
    // 34: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 35: add r0.x, -r0.x, r0.y
    r0.x = ((-(r0.xxxx))+(r0.yyyy)).x;
    // 36: mul r0.yzw, cb0[8].xxyz, cb0[8].wwww
    r0.yzw = ((source[8].xxyz)*(source[8].wwww)).yzw;
    // 37: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 38: mad r2.xy, v4.xyxx, l(0.800000, 0.300000, 0.000000, 0.000000), cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(float4(0.800000,0.300000,0.000000,0.000000))+(source[2].xyxx)).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t3.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 40: mul r2.xy, v4.xyxx, l(0.800000, 0.600000, 0.000000, 0.000000)
    r2.xy = ((v4.xyxx)*(float4(0.800000,0.600000,0.000000,0.000000))).xy;
    // 41: mad r2.xy, r0.wwww, l(0.800000, 0.800000, 0.000000, 0.000000), r2.xyxx
    r2.xy = ((r0.wwww)*(float4(0.800000,0.800000,0.000000,0.000000))+(r2.xyxx)).xy;
    // 42: add r2.xy, r2.xyxx, cb0[3].xyxx
    r2.xy = ((r2.xyxx)+(source[3].xyxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t4.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 44: mad r2.xy, v4.xyxx, l(0.600000, 0.300000, 0.000000, 0.000000), cb0[4].xyxx
    r2.xy = ((v4.xyxx)*(float4(0.600000,0.300000,0.000000,0.000000))+(source[4].xyxx)).xy;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t3.yzwx, s1, l(0.000000)
    r1.w = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 46: mad r2.xy, v4.xyxx, l(0.750000, 0.500000, 0.000000, 0.000000), r1.wwww
    r2.xy = ((v4.xyxx)*(float4(0.750000,0.500000,0.000000,0.000000))+(r1.wwww)).xy;
    // 47: add r2.xy, r2.xyxx, cb0[5].xyxx
    r2.xy = ((r2.xyxx)+(source[5].xyxx)).xy;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t5.xywz, s3, l(0.000000)
    r1.w = (ArtistNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).w;
    // 49: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 50: mul r0.w, r0.w, l(12.000000)
    r0.w = ((r0.wwww)*(float4(12.000000,12.000000,12.000000,12.000000))).w;
    // 51: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 52: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 53: mul r1.w, r1.w, l(1.250000)
    r1.w = ((r1.wwww)*(float4(1.250000,1.250000,1.250000,1.250000))).w;
    // 54: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 55: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r1.w, v4.xyxx, t6.yzwx, s4, l(0.000000)
    r1.w = (ArtistNativeSample4((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 57: mul r2.xyz, r1.wwww, cb0[6].xyzx
    r2.xyz = ((r1.wwww)*(source[6].xyzx)).xyz;
    // 58: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 59: mad r0.xyz, cb0[7].yyyy, r2.xyzx, r0.xyzx
    r0.xyz = ((source[7].yyyy)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 60: mad r0.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r0.xyzx)).xyz;
    // 61: mad o0.xyz, r0.xyzx, cb0[0].xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)*(source[0].xyzx)+(source[1].xyzx)).xyz;
    // 62: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_glow_01_ad_dt: 2bd85c08a26e594b945c597997daffea; selected map 721739ad9730ede6ef4b38fd43a1d852210faff01a4e05c1b6e0ae8ddb4d8df6.
float4 ArtistNative2986(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_master_01_04_ma: de86fd3fa6797846b5c8cd2d2319cf95; selected map 1abe0285a4383ccaf73a29523f0f1ddff922a64a56a2a7e7564c9fdcc80d2b70.
float4 ArtistNative2987(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[3u];
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xy, v6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((v6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 2: frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // 3: mov_sat r0.z, v3.w
    r0.z = (saturate(v3.wwww)).z;
    // 4: add r0.xy, r0.zzzz, r0.xyxx
    r0.xy = ((r0.zzzz)+(r0.xyxx)).xy;
    // 5: round_ni r0.xy, r0.xyxx
    r0.xy = (floor(r0.xyxx)).xy;
    // 6: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 7: frc r0.yw, v6.xxxy
    r0.yw = (frac(v6.xxxy)).yw;
    // 8: add r0.yz, r0.zzzz, r0.yywy
    r0.yz = ((r0.zzzz)+(r0.yywy)).yz;
    // 9: round_ni r0.yz, r0.yyzy
    r0.yz = (floor(r0.yyzy)).yz;
    // 10: mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // 11: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: mul r0.yz, v2.xxyx, cb0[2].zzwz
    r0.yz = ((v2.xxyx)*(source[2].zzwz)).yz;
    // 13: mad r1.x, cb0[2].y, cb0[2].x, r0.y
    r1.x = ((source[2].yyyy)*(source[2].xxxx)+(r0.yyyy)).x;
    // 14: mad r1.y, cb0[2].y, cb0[3].x, r0.z
    r1.y = ((source[2].yyyy)*(source[3].xxxx)+(r0.zzzz)).y;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 16: mul_sat r0.y, r0.y, cb0[3].y
    r0.y = (saturate((r0.yyyy)*(source[3].yyyy))).y;
    // 17: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 18: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 19: mul r0.z, r0.z, cb0[3].z
    r0.z = ((r0.zzzz)*(source[3].zzzz)).z;
    // 20: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 21: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 22: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: add r0.x, r0.x, l(-0.166000)
    r0.x = ((r0.xxxx)+(float4(-0.166000,-0.166000,-0.166000,-0.166000))).x;
    // 24: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 25: or r0.x, r0.x, r0.y
    r0.x = (asfloat(asuint(r0.xxxx) | asuint(r0.yyyy))).x;
    // 26: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 27: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 28: mul r0.xyz, cb0[1].xyzx, cb0[1].wwww
    r0.xyz = ((source[1].xyzx)*(source[1].wwww)).xyz;
    // 29: mad o0.xyz, r0.xyzx, v3.xyzx, cb0[0].xyzx
    output.xyz = ((r0.xyzx)*(v3.xyzx)+(source[0].xyzx)).xyz;
    // 30: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_master_01_050_ad: dbae20996679df47b23fb48bc3c8c05f; selected map f9eee02cdeb184b461d1f0bb00694a339f7f85d16f18d4b558de93bcf4d99968.
float4 ArtistNative2988(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].yyyy)).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mul r0.xy, v2.xyxx, cb0[4].zwzz
    r0.xy = ((v2.xyxx)*(source[4].zwzz)).xy;
    // 2: mul r0.z, cb0[3].x, cb0[3].y
    r0.z = ((source[3].xxxx)*(source[3].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[4].y, r0.x
    r1.x = ((r0.zzzz)*(source[4].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[5].x, r0.y
    r1.y = ((r0.zzzz)*(source[5].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[5].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[5].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 7: mul r0.w, r0.x, cb0[3].w
    r0.w = ((r0.xxxx)*(source[3].wwww)).w;
    // 8: mad r1.x, r0.z, cb0[3].z, r0.w
    r1.x = ((r0.zzzz)*(source[3].zzzz)+(r0.wwww)).x;
    // 9: mul r0.w, r0.z, cb0[5].w
    r0.w = ((r0.zzzz)*(source[5].wwww)).w;
    // 10: mad r1.y, cb0[4].x, r0.y, r0.w
    r1.y = ((source[4].xxxx)*(r0.yyyy)+(r0.wwww)).y;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 12: mul r2.xy, r0.xyxx, cb0[6].yzyy
    r2.xy = ((r0.xyxx)*(source[6].yzyy)).xy;
    // 13: mad r2.xy, r0.zzzz, cb0[6].xwxx, r2.xyxx
    r2.xy = ((r0.zzzz)*(source[6].xwxx)+(r2.xyxx)).xy;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 15: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 16: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 17: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 18: mad r1.xyz, cb0[7].xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((source[7].xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 19: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 20: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 21: mul r1.xyz, r1.xyzx, cb0[7].yyyy
    r1.xyz = ((r1.xyzx)*(source[7].yyyy)).xyz;
    // 22: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 23: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 24: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 25: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 26: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 27: mul r0.x, r0.x, cb0[7].w
    r0.x = ((r0.xxxx)*(source[7].wwww)).x;
    // 28: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 29: mad r2.y, r0.z, cb0[8].y, r0.y
    r2.y = ((r0.zzzz)*(source[8].yyyy)+(r0.yyyy)).y;
    // 30: mad r2.x, r0.z, cb0[7].z, r0.x
    r2.x = ((r0.zzzz)*(source[7].zzzz)+(r0.xxxx)).x;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 32: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 33: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 34: mul_sat r0.x, r0.x, cb0[8].z
    r0.x = (saturate((r0.xxxx)*(source[8].zzzz))).x;
    // 35: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 36: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 37: mul r0.x, r0.x, cb0[8].w
    r0.x = ((r0.xxxx)*(source[8].wwww)).x;
    // 38: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 39: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 40: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 41: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 42: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 43: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_me_transition_04_3_ma: 496aee5a3382744d97f9dcdb89b6ae6c; selected map f9be08d5e66242bc426fcb73591edacf680f68a2acf227bb3d834507dd494ecc.
float4 ArtistNative2989(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.150000006, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.150000006, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = g_ArtistSourceMaterialParameters[3u];
    source[7] = input.dynamicParameter;
    source[8] = g_ArtistSourceMaterialParameters[1u];
    source[9] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[10].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v4.xyxx, cb0[9].xyxx
    r0.xy = ((v4.xyxx)+(source[9].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s5, l(0.000000)
    r0.xy = (ArtistNativeSample5((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mul r0.zw, r0.xxxy, cb0[10].wwww
    r0.zw = ((r0.xxxy)*(source[10].wwww)).zw;
    // 4: mad r0.xy, cb0[10].zzzz, r0.xyxx, v4.xyxx
    r0.xy = ((source[10].zzzz)*(r0.xyxx)+(v4.xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s5, l(0.000000)
    r0.x = (ArtistNativeSample5((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: mad r0.y, -cb0[10].y, cb0[7].x, l(1.000000)
    r0.y = ((-(source[10].yyyy))*(source[7].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 7: mad r0.yz, r0.yyyy, r0.zzwz, v4.xxyx
    r0.yz = ((r0.yyyy)*(r0.zzwz)+(v4.xxyx)).yz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t2.xyzw, s6, l(0.000000)
    r1.xyzw = (ArtistNativeSample6((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 9: mul r0.y, r1.w, l(1.150000)
    r0.y = ((r1.wwww)*(float4(1.150000,1.150000,1.150000,1.150000))).y;
    // 10: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 11: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 12: mul r0.z, r0.z, l(1.150000)
    r0.z = ((r0.zzzz)*(float4(1.150000,1.150000,1.150000,1.150000))).z;
    // 13: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 14: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 15: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 16: mul r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)*(r0.xxxx)).y;
    // 17: mul r0.z, cb0[7].x, cb0[10].y
    r0.z = ((source[7].xxxx)*(source[10].yyyy)).z;
    // 18: mad r0.w, r0.z, l(0.100000), l(0.900000)
    r0.w = ((r0.zzzz)*(float4(0.100000,0.100000,0.100000,0.100000))+(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 19: mad r0.y, r0.z, r0.y, r0.w
    r0.y = ((r0.zzzz)*(r0.yyyy)+(r0.wwww)).y;
    // 20: max r0.y, r0.y, l(0.900000)
    r0.y = (max(r0.yyyy,float4(0.900000,0.900000,0.900000,0.900000))).y;
    // 21: round_ni r0.y, r0.y
    r0.y = (floor(r0.yyyy)).y;
    // 22: min r1.w, r0.y, l(1.000000)
    r1.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: mad r1.w, cb0[0].w, r1.w, l(-0.333000)
    r1.w = ((source[0].wwww)*(r1.wwww)+(float4(-0.333000,-0.333000,-0.333000,-0.333000))).w;
    // 24: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 25: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) clip(-1.f);
    // 26: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 27: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 28: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: mul r1.w, r1.w, cb0[11].x
    r1.w = ((r1.wwww)*(source[11].xxxx)).w;
    // 30: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 31: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 32: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 33: add_sat r0.x, r0.x, r0.w
    r0.x = (saturate((r0.xxxx)+(r0.wwww))).x;
    // 34: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 35: add r0.x, -r0.x, r0.y
    r0.x = ((-(r0.xxxx))+(r0.yyyy)).x;
    // 36: mul r0.yzw, cb0[8].xxyz, cb0[8].wwww
    r0.yzw = ((source[8].xxyz)*(source[8].wwww)).yzw;
    // 37: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 38: mad r2.xy, v4.xyxx, l(0.800000, 0.300000, 0.000000, 0.000000), cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(float4(0.800000,0.300000,0.000000,0.000000))+(source[2].xyxx)).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t3.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 40: mul r2.xy, v4.xyxx, l(0.800000, 0.600000, 0.000000, 0.000000)
    r2.xy = ((v4.xyxx)*(float4(0.800000,0.600000,0.000000,0.000000))).xy;
    // 41: mad r2.xy, r0.wwww, l(0.800000, 0.800000, 0.000000, 0.000000), r2.xyxx
    r2.xy = ((r0.wwww)*(float4(0.800000,0.800000,0.000000,0.000000))+(r2.xyxx)).xy;
    // 42: add r2.xy, r2.xyxx, cb0[3].xyxx
    r2.xy = ((r2.xyxx)+(source[3].xyxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t4.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 44: mad r2.xy, v4.xyxx, l(0.600000, 0.300000, 0.000000, 0.000000), cb0[4].xyxx
    r2.xy = ((v4.xyxx)*(float4(0.600000,0.300000,0.000000,0.000000))+(source[4].xyxx)).xy;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t3.yzwx, s1, l(0.000000)
    r1.w = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 46: mad r2.xy, v4.xyxx, l(0.750000, 0.500000, 0.000000, 0.000000), r1.wwww
    r2.xy = ((v4.xyxx)*(float4(0.750000,0.500000,0.000000,0.000000))+(r1.wwww)).xy;
    // 47: add r2.xy, r2.xyxx, cb0[5].xyxx
    r2.xy = ((r2.xyxx)+(source[5].xyxx)).xy;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t5.xywz, s3, l(0.000000)
    r1.w = (ArtistNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).w;
    // 49: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 50: mul r0.w, r0.w, l(12.000000)
    r0.w = ((r0.wwww)*(float4(12.000000,12.000000,12.000000,12.000000))).w;
    // 51: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 52: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 53: mul r1.w, r1.w, l(1.250000)
    r1.w = ((r1.wwww)*(float4(1.250000,1.250000,1.250000,1.250000))).w;
    // 54: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 55: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r1.w, v4.xyxx, t6.yzwx, s4, l(0.000000)
    r1.w = (ArtistNativeSample4((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 57: mul r2.xyz, r1.wwww, cb0[6].xyzx
    r2.xyz = ((r1.wwww)*(source[6].xyzx)).xyz;
    // 58: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 59: mad r0.xyz, cb0[7].yyyy, r2.xyzx, r0.xyzx
    r0.xyz = ((source[7].yyyy)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 60: mad r0.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r0.xyzx)).xyz;
    // 61: mad o0.xyz, r0.xyzx, cb0[0].xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)*(source[0].xyzx)+(source[1].xyzx)).xyz;
    // 62: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_lightingdetail_01_ad: 8ddb6fc857a29444983261f5b1bc6979; selected map 9ae110a8f65014e80e87ef16e8f5d6a1f573b9f6fa8b8fef3c8e5523a7d156eb.
float4 ArtistNative2990(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].y = ((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[2].z = (((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[2].w = (sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[3].x = ((sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[3].y = (abs((sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (fmod(abs((sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))),float4(1.5, 0.0, 0.0, 0.0))).x;
    source[3].w = ((fmod(abs((sin(((g_ArtistSourceMaterialTime.xxxx*float4(2.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))),float4(1.5, 0.0, 0.0, 0.0))+float4(0.300000012, 0.0, 0.0, 0.0))).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f;
    // 1: mul r0.x, v4.y, cb0[2].x
    r0.x = ((v4.yyyy)*(source[2].xxxx)).x;
    // 2: mul r0.y, r0.x, l(0.500000)
    r0.y = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 3: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 4: mad r0.xy, v2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v2.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 7: mul r0.y, r0.y, v4.x
    r0.y = ((r0.yyyy)*(v4.xxxx)).y;
    // 8: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 9: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 10: mad r0.xw, r0.xxxx, l(0.100000, 0.000000, 0.000000, 0.100000), v2.xxxy
    r0.xw = ((r0.xxxx)*(float4(0.100000,0.000000,0.000000,0.100000))+(v2.xxxy)).xw;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xwxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 12: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 13: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 14: mul r0.xyz, r0.xxxx, v3.xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)).xyz;
    // 15: mad r0.xyz, cb0[3].wwww, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((source[3].wwww)*(r0.xyzx)+(source[1].xyzx)).xyz;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_worldoffset_01_3_tr: 6c98ffeb71f43947910f9d868cbb2b55; selected map 79769f1abea8cc35854fb7d3866b2a28d5bd21554c666037a4264f4987718a6b.
float4 ArtistNative2991(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0149999997, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0209999997, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0109999999, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0199999996, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(-0.200000003, 0.0, 0.0, 0.0))),1u);
    source[6].x = ((float4(0.00100000005, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].zzzz)).x;
    source[6].y = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0209999997, 0.0, 0.0, 0.0)))).x;
    source[6].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0149999997, 0.0, 0.0, 0.0)))).x;
    source[6].w = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.0109999999, 0.0, 0.0, 0.0))).x;
    source[7].x = ((g_ArtistSourceMaterialTime.xxxx*float4(0.0199999996, 0.0, 0.0, 0.0))).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[7].z = ((float4(0.00100000005, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].wwww)).x;
    source[7].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[8].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0109999999, 0.0, 0.0, 0.0)))).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[8].w = ((float4(0.00100000005, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].xxxx)).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].z = ((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[9].w = (((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[10].x = (((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(-0.200000003, 0.0, 0.0, 0.0))).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(-0.200000003, 0.0, 0.0, 0.0)))).x;
    source[11].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[11].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[11].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[12].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[12].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy))).x;
    source[12].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[12].w = ((float4(-1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].wwww)).x;
    source[13].x = ((float4(1.0, 0.0, 0.0, 0.0)-(float4(-1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].wwww))).x;
    source[13].y = (max((float4(1.0, 0.0, 0.0, 0.0)-(float4(-1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].wwww)),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[13].z = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-(float4(-1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].wwww)),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[13].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[14].x = ((float4(0.00100000005, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].yyyy)).x;
    source[14].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[14].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[14].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[15].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: add r0.xyzw, v2.xyxy, l(-0.020000, -0.030000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.020000,-0.030000,-0.500000,-0.500000))).xyzw;
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
    r1.z = ((r1.yyyy)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 8: mad r1.z, r1.y, r1.z, l(0.180141)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 9: mad r1.z, r1.y, r1.z, l(-0.330299)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 10: mad r1.y, r1.y, r1.z, l(0.999866)
    r1.y = ((r1.yyyy)*(r1.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).y;
    // 11: mul r1.z, r1.y, r1.x
    r1.z = ((r1.yyyy)*(r1.xxxx)).z;
    // 12: mad r1.z, r1.z, l(-2.000000), l(1.570796)
    r1.z = ((r1.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 13: lt r1.w, |r0.w|, |r0.z|
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(abs(r0.zzzz))) * 0xffffffffu)).w;
    // 14: and r1.z, r1.w, r1.z
    r1.z = (asfloat(asuint(r1.wwww) & asuint(r1.zzzz))).z;
    // 15: mad r1.x, r1.x, r1.y, r1.z
    r1.x = ((r1.xxxx)*(r1.yyyy)+(r1.zzzz)).x;
    // 16: lt r1.y, r0.w, -r0.w
    r1.y = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).y;
    // 17: and r1.y, r1.y, l(0xc0490fdb)
    r1.y = (asfloat(asuint(r1.yyyy) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).y;
    // 18: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 19: min r1.y, r0.w, r0.z
    r1.y = (min(r0.wwww,r0.zzzz)).y;
    // 20: lt r1.y, r1.y, -r1.y
    r1.y = (asfloat((uint4)((r1.yyyy)<(-(r1.yyyy))) * 0xffffffffu)).y;
    // 21: max r1.z, r0.w, r0.z
    r1.z = (max(r0.wwww,r0.zzzz)).z;
    // 22: ge r1.z, r1.z, -r1.z
    r1.z = (asfloat((uint4)((r1.zzzz)>=(-(r1.zzzz))) * 0xffffffffu)).z;
    // 23: and r1.y, r1.z, r1.y
    r1.y = (asfloat(asuint(r1.zzzz) & asuint(r1.yyyy))).y;
    // 24: movc r1.x, r1.y, -r1.x, r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (-(r1.xxxx)) : (r1.xxxx)).x;
    // 25: mad r1.x, r1.x, l(0.159155), l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 26: mul r1.x, r1.x, cb0[10].y
    r1.x = ((r1.xxxx)*(source[10].yyyy)).x;
    // 27: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 28: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 29: mul r0.z, r0.z, l(1.500000)
    r0.z = ((r0.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 30: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 31: mul r1.y, r0.z, cb0[10].z
    r1.y = ((r0.zzzz)*(source[10].zzzz)).y;
    // 32: add r0.zw, r1.xxxy, cb0[5].xxxy
    r0.zw = ((r1.xxxy)+(source[5].xxxy)).zw;
    // 33: dp3 r1.x, v1.xyzx, v1.xyzx
    r1.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 34: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 35: mul r1.xyz, r1.xxxx, v1.zxyz
    r1.xyz = ((r1.xxxx)*(v1.zxyz)).xyz;
    // 36: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 37: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 38: mul r2.xyz, r1.wwww, v0.yzxy
    r2.xyz = ((r1.wwww)*(v0.yzxy)).xyz;
    // 39: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 40: mad r3.xyz, r1.zxyz, r2.yzxy, -r3.xyzx
    r3.xyz = ((r1.zxyz)*(r2.yzxy)+(-(r3.xyzx))).xyz;
    // 41: mul r3.xyz, r3.xzyx, v1.wwww
    r3.xyz = ((r3.xzyx)*(v1.wwww)).xyz;
    // 42: add r4.xyz, v7.xyzx, cb0[0].xyzx
    r4.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 43: mul r4.yw, r3.xxxz, r4.yyyy
    r4.yw = ((r3.xxxz)*(r4.yyyy)).yw;
    // 44: mad r2.xz, r4.xxxx, r2.zzxz, r4.yywy
    r2.xz = ((r4.xxxx)*(r2.zzxz)+(r4.yywy)).xz;
    // 45: mov r3.x, r2.y
    r3.x = (r2.yyyy).x;
    // 46: mad r1.yz, r4.zzzz, r1.yyzy, r2.xxzx
    r1.yz = ((r4.zzzz)*(r1.yyzy)+(r2.xxzx)).yz;
    // 47: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 48: mad r1.xw, cb0[6].xxxx, r1.yyyz, cb0[3].xxxy
    r1.xw = ((source[6].xxxx)*(r1.yyyz)+(source[3].xxxy)).xw;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r1.xw, r1.xwxx, t0.xzwy, s0, l(0.000000)
    r1.xw = (ArtistNativeSample0((r1.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzwy).xw;
    // 50: mad r2.xy, cb0[7].zzzz, r1.yzyy, cb0[4].xyxx
    r2.xy = ((source[7].zzzz)*(r1.yzyy)+(source[4].xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 52: mul r1.xw, r1.xxxw, r2.xxxy
    r1.xw = ((r1.xxxw)*(r2.xxxy)).xw;
    // 53: mad r0.zw, cb0[9].xxxx, r1.xxxw, r0.zzzw
    r0.zw = ((source[9].xxxx)*(r1.xxxw)+(r0.zzzw)).zw;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 55: mul r0.w, r0.z, cb0[11].y
    r0.w = ((r0.zzzz)*(source[11].yyyy)).w;
    // 56: mad r2.yz, cb0[8].wwww, r1.yyzy, r0.wwww
    r2.yz = ((source[8].wwww)*(r1.yyzy)+(r0.wwww)).yz;
    // 57: mul r1.yz, r1.yyzy, cb0[14].xxxx
    r1.yz = ((r1.yyzy)*(source[14].xxxx)).yz;
    // 58: mad r1.yz, r1.xxwx, l(0.000000, 0.200000, 0.200000, 0.000000), r1.yyzy
    r1.yz = ((r1.xxwx)*(float4(0.000000,0.200000,0.200000,0.000000))+(r1.yyzy)).yz;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r1.yzyy, t3.xyzw, s3, l(0.000000)
    r4.xyz = (ArtistNativeSample3((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r1.yz, r2.yzyy, t2.zxyw, s2, l(0.000000)
    r1.yz = (ArtistNativeSample2((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 61: mad r1.yz, r1.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r1.yz = ((r1.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 62: mul r1.yz, r1.yyzy, cb0[11].zzzz
    r1.yz = ((r1.yyzy)*(source[11].zzzz)).yz;
    // 63: mad r1.xy, cb0[8].yyyy, r1.xwxx, r1.yzyy
    r1.xy = ((source[8].yyyy)*(r1.xwxx)+(r1.yzyy)).xy;
    // 64: add r0.xy, r0.xyxx, r1.xyxx
    r0.xy = ((r0.xyxx)+(r1.xyxx)).xy;
    // 65: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 66: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 67: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 68: mad r0.x, -r0.x, cb0[12].y, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[12].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 69: mul_sat r0.x, r0.x, cb0[13].z
    r0.x = (saturate((r0.xxxx)*(source[13].zzzz))).x;
    // 70: log r0.y, |r2.x|
    r0.y = (log2(abs(r2.xxxx))).y;
    // 71: lt r0.w, |r2.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 72: mul r0.y, r0.y, cb0[15].x
    r0.y = ((r0.yyyy)*(source[15].xxxx)).y;
    // 73: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 74: mul r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)*(r0.xxxx)).y;
    // 75: mul r0.y, r0.y, l(40.000000)
    r0.y = ((r0.yyyy)*(float4(40.000000,40.000000,40.000000,40.000000))).y;
    // 76: movc r0.y, r0.w, l(0), r0.y
    r0.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 77: mul r1.xy, r0.xxxx, l(30.000000, 28.000000, 0.000000, 0.000000)
    r1.xy = ((r0.xxxx)*(float4(30.000000,28.000000,0.000000,0.000000))).xy;
    // 78: min r1.xy, r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = (min(r1.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 79: add r0.w, -r1.y, r1.x
    r0.w = ((-(r1.yyyy))+(r1.xxxx)).w;
    // 80: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 81: mad r0.y, r0.w, cb0[14].w, r0.y
    r0.y = ((r0.wwww)*(source[14].wwww)+(r0.yyyy)).y;
    // 82: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 83: add r0.w, v4.x, l(-1.000000)
    r0.w = ((v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 84: add r0.w, -r0.w, r4.x
    r0.w = ((-(r0.wwww))+(r4.xxxx)).w;
    // 85: mul_sat r0.w, r0.w, l(30.000000)
    r0.w = (saturate((r0.wwww)*(float4(30.000000,30.000000,30.000000,30.000000)))).w;
    // 86: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 87: mul o0.w, r0.y, cb0[0].w
    output.w = ((r0.yyyy)*(source[0].wwww)).w;
    // 88: log r0.y, |r0.z|
    r0.y = (log2(abs(r0.zzzz))).y;
    // 89: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 90: mul r0.y, r0.y, cb0[14].y
    r0.y = ((r0.yyyy)*(source[14].yyyy)).y;
    // 91: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 92: mul r0.y, r0.y, cb0[14].z
    r0.y = ((r0.yyyy)*(source[14].zzzz)).y;
    // 93: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 94: mul r0.z, r0.x, r0.x
    r0.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 95: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 96: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 97: mul r1.xyz, r0.zzzz, cb0[2].xyzx
    r1.xyz = ((r0.zzzz)*(source[2].xyzx)).xyz;
    // 98: movc r0.xzw, r0.xxxx, l(0,0,0,0), r1.xxyz
    r0.xzw = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxyz)).xzw;
    // 99: mad r0.xzw, r4.xxyz, l(0.500000, 0.000000, 0.500000, 0.500000), r0.xxzw
    r0.xzw = ((r4.xxyz)*(float4(0.500000,0.000000,0.500000,0.500000))+(r0.xxzw)).xzw;
    // 100: mad r0.xyz, r0.yyyy, r0.xzwx, r0.xzwx
    r0.xyz = ((r0.yyyy)*(r0.xzwx)+(r0.xzwx)).xyz;
    // 101: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 102: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_smoke_01_8_tr: 5c707b8a1ddaf5478de6e37c4b55bf26; selected map f1f290972b5b31eec74b516c45ee383023e1c16a6224be179cff954bdcae7562.
float4 ArtistNative2992(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = (cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 23: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 24: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 25: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 26: mad r0.y, r0.y, l(0.318310), l(1.000000)
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 27: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 28: mul r1.x, r0.y, cb0[6].x
    r1.x = ((r0.yyyy)*(source[6].xxxx)).x;
    // 29: mul r2.xy, r0.yyyy, cb0[4].ywyy
    r2.xy = ((r0.yyyy)*(source[4].ywyy)).xy;
    // 30: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 31: mul r0.y, r0.y, l(0.800000)
    r0.y = ((r0.yyyy)*(float4(0.800000,0.800000,0.800000,0.800000))).y;
    // 32: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 33: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 34: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 35: mad r0.x, -r0.x, l(2.222222), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.222222,2.222222,2.222222,2.222222))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mul r0.x, r0.x, l(0.333333)
    r0.x = ((r0.xxxx)*(float4(0.333333,0.333333,0.333333,0.333333))).x;
    // 37: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 38: mul r0.x, r0.x, l(4.000000)
    r0.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 39: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 40: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 41: mul r2.z, r0.y, cb0[5].x
    r2.z = ((r0.yyyy)*(source[5].xxxx)).z;
    // 42: mad r0.zw, v4.zzzz, l(0.000000, 0.000000, -0.030000, -0.100000), r2.yyyz
    r0.zw = ((v4.zzzz)*(float4(0.000000,0.000000,-0.030000,-0.100000))+(r2.yyyz)).zw;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s0, l(0.000000)
    r0.zw = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 44: mad r3.xyzw, r0.zwzw, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.500000, -1.500000)
    r3.xyzw = ((r0.zwzw)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.500000,-1.500000))).xyzw;
    // 45: mul r1.y, r0.y, cb0[6].y
    r1.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 46: mul r2.w, r0.y, cb0[4].z
    r2.w = ((r0.yyyy)*(source[4].zzzz)).w;
    // 47: mad r0.yz, r3.xxyx, l(0.000000, 0.080000, 0.080000, 0.000000), r1.xxyx
    r0.yz = ((r3.xxyx)*(float4(0.000000,0.080000,0.080000,0.000000))+(r1.xxyx)).yz;
    // 48: mad r0.yz, v4.zzzz, l(0.000000, -0.050000, -0.100000, 0.000000), r0.yyzy
    r0.yz = ((v4.zzzz)*(float4(0.000000,-0.050000,-0.100000,0.000000))+(r0.yyzy)).yz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 50: add r0.zw, v4.yyyx, l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((v4.yyyx)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 51: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 52: add_sat r0.y, r0.y, r0.y
    r0.y = (saturate((r0.yyyy)+(r0.yyyy))).y;
    // 53: dp2 r1.x, l(0.000796, -1.000000, 0.000000, 0.000000), r3.zwzz
    r1.x = (dot((float4(0.000796,-1.000000,0.000000,0.000000)).xy,(r3.zwzz).xy).xxxx).x;
    // 54: dp2 r1.y, l(1.000000, 0.000796, 0.000000, 0.000000), r3.zwzz
    r1.y = (dot((float4(1.000000,0.000796,0.000000,0.000000)).xy,(r3.zwzz).xy).xxxx).y;
    // 55: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 56: mad r1.xy, r0.zzzz, r1.xyxx, r2.xwxx
    r1.xy = ((r0.zzzz)*(r1.xyxx)+(r2.xwxx)).xy;
    // 57: mad r1.z, v4.w, l(0.200000), r1.y
    r1.z = ((v4.wwww)*(float4(0.200000,0.200000,0.200000,0.200000))+(r1.yyyy)).z;
    // 58: add r0.zw, r1.xxxz, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r1.xxxz)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 59: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 60: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 61: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 63: add r0.z, r1.x, r1.x
    r0.z = ((r1.xxxx)+(r1.xxxx)).z;
    // 64: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 65: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 66: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 67: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 68: max r0.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 69: mul r1.xyz, r1.xyzx, cb0[5].wwww
    r1.xyz = ((r1.xyzx)*(source[5].wwww)).xyz;
    // 70: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 71: mul r0.xyz, r0.xyzx, cb0[5].yyyy
    r0.xyz = ((r0.xyzx)*(source[5].yyyy)).xyz;
    // 72: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 73: mad r0.xyz, cb0[5].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[5].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 74: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 75: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_i_pa_shockwave_07_ad: 8238a1a9b601624d808980b887eeacac; selected map 956768caed8befb013af997f30e867ea8f2b0fcfa5cfa8b3b3f56a89f89aed97.
float4 ArtistNative2993(ARTIST_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[11u];
    source[2] = g_ArtistSourceMaterialParameters[9u];
    source[3] = g_ArtistSourceMaterialParameters[10u];
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].yyyy,g_ArtistSourceMaterialParameters[6u].zzzz,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[9].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[10].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[11].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[12].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[12].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[12].w = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[13].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[13].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[13].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.x|, |r0.y|
    r0.z = (max(abs(r0.xxxx),abs(r0.yyyy))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.x|, |r0.y|
    r0.w = (min(abs(r0.xxxx),abs(r0.yyyy))).w;
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
    // 13: lt r1.y, |r0.x|, |r0.y|
    r1.y = (asfloat((uint4)((abs(r0.xxxx))<(abs(r0.yyyy))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.x, -r0.x
    r0.w = (asfloat((uint4)((r0.xxxx)<(-(r0.xxxx))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.x, r0.y
    r0.w = (min(r0.xxxx,r0.yyyy)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.x, r0.y
    r1.x = (max(r0.xxxx,r0.yyyy)).x;
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
    // 27: mul r0.y, r0.y, cb0[5].x
    r0.y = ((r0.yyyy)*(source[5].xxxx)).y;
    // 28: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: dp2 r0.z, r0.zzzz, cb0[5].yyyy
    r0.z = (dot((r0.zzzz).xy,(source[5].yyyy).xy).xxxx).z;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).y;
    // 31: add r1.x, r0.y, cb0[5].z
    r1.x = ((r0.yyyy)+(source[5].zzzz)).x;
    // 32: mul r2.x, r1.x, cb0[8].w
    r2.x = ((r1.xxxx)*(source[8].wwww)).x;
    // 33: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 34: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 35: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 36: mul r0.y, r0.y, cb0[5].w
    r0.y = ((r0.yyyy)*(source[5].wwww)).y;
    // 37: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 38: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 39: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 40: add r0.y, v4.x, cb0[6].x
    r0.y = ((v4.xxxx)+(source[6].xxxx)).y;
    // 41: mad r1.y, r0.y, l(-0.400000), r0.x
    r1.y = ((r0.yyyy)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.xxxx)).y;
    // 42: mul r0.xz, r1.xxyx, cb0[7].xxyx
    r0.xz = ((r1.xxyx)*(source[7].xxyx)).xz;
    // 43: mad r0.y, cb0[7].z, v4.y, r0.z
    r0.y = ((source[7].zzzz)*(v4.yyyy)+(r0.zzzz)).y;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(-1.000000)
    r0.xy = (ArtistNativeSample1((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
    // 45: max r0.xy, |r0.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (max(abs(r0.xyxx),float4(0.000001,0.000001,0.000000,0.000000))).xy;
    // 46: log r0.xy, r0.xyxx
    r0.xy = (log2(r0.xyxx)).xy;
    // 47: mul r0.xy, r0.xyxx, cb0[7].wwww
    r0.xy = ((r0.xyxx)*(source[7].wwww)).xy;
    // 48: exp r0.xy, r0.xyxx
    r0.xy = (exp2(r0.xyxx)).xy;
    // 49: mad r2.y, r1.y, cb0[9].x, cb0[9].y
    r2.y = ((r1.yyyy)*(source[9].xxxx)+(source[9].yyyy)).y;
    // 50: mad r0.zw, cb0[8].xxxx, r0.xxxy, r2.xxxy
    r0.zw = ((source[8].xxxx)*(r0.xxxy)+(r2.xxxy)).zw;
    // 51: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t2.xyzw, s2, l(-1.000000)
    r2.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 52: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 53: add r3.xyz, -r2.xyzx, r0.zzzz
    r3.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 54: mad r2.xyz, r3.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r2.xyzx)).xyz;
    // 55: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 56: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 57: mul r2.xyz, r2.xyzx, cb0[9].zzzz
    r2.xyz = ((r2.xyzx)*(source[9].zzzz)).xyz;
    // 58: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 59: mul r3.xyz, r2.xyzx, cb0[9].wwww
    r3.xyz = ((r2.xyzx)*(source[9].wwww)).xyz;
    // 60: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 61: mad r2.xyz, -cb0[9].wwww, r2.xyzx, r0.zzzz
    r2.xyz = ((-(source[9].wwww))*(r2.xyzx)+(r0.zzzz)).xyz;
    // 62: mad r2.xyz, cb0[10].xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((source[10].xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 63: mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 64: mul r3.x, r1.x, cb0[10].y
    r3.x = ((r1.xxxx)*(source[10].yyyy)).x;
    // 65: mad r3.y, r1.y, cb0[10].z, cb0[10].w
    r3.y = ((r1.yyyy)*(source[10].zzzz)+(source[10].wwww)).y;
    // 66: mad r0.zw, cb0[8].xxxx, r0.xxxy, r3.xxxy
    r0.zw = ((source[8].xxxx)*(r0.xxxy)+(r3.xxxy)).zw;
    // 67: sample_l_indexable(texture2d)(float,float,float,float) r3.xyz, r0.zwzz, t3.xyzw, s3, l(-1.000000)
    r3.xyz = (ArtistNativeSample3((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 68: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 69: add r4.xyz, -r3.xyzx, r0.zzzz
    r4.xyz = ((-(r3.xyzx))+(r0.zzzz)).xyz;
    // 70: mad r3.xyz, r4.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r3.xyzx)).xyz;
    // 71: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 72: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 73: mul r3.xyz, r3.xyzx, cb0[11].xxxx
    r3.xyz = ((r3.xyzx)*(source[11].xxxx)).xyz;
    // 74: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 75: mul r4.xyz, r3.xyzx, cb0[11].yyyy
    r4.xyz = ((r3.xyzx)*(source[11].yyyy)).xyz;
    // 76: dp3 r0.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 77: mad r3.xyz, -cb0[11].yyyy, r3.xyzx, r0.zzzz
    r3.xyz = ((-(source[11].yyyy))*(r3.xyzx)+(r0.zzzz)).xyz;
    // 78: mad r3.xyz, cb0[11].zzzz, r3.xyzx, r4.xyzx
    r3.xyz = ((source[11].zzzz)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 79: mul r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)*(source[3].xyzx)).xyz;
    // 80: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 81: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 1.000000, 0.800000), cb0[4].xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,1.000000,0.800000))+(source[4].xxxy)).zw;
    // 82: mul r1.xz, r1.xxyx, cb0[6].yyzy
    r1.xz = ((r1.xxyx)*(source[6].yyzy)).xz;
    // 83: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t4.yzxw, s4, l(-1.000000)
    r0.z = (ArtistNativeSample4((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).z;
    // 84: mul r2.xyz, r2.xyzx, r0.zzzz
    r2.xyz = ((r2.xyzx)*(r0.zzzz)).xyz;
    // 85: mul r3.xyz, r2.xyzx, cb0[12].yyyy
    r3.xyz = ((r2.xyzx)*(source[12].yyyy)).xyz;
    // 86: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 87: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 88: mul r3.xyz, r3.xyzx, cb0[12].zzzz
    r3.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 89: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 90: mad r1.y, cb0[6].w, v4.y, r1.z
    r1.y = ((source[6].wwww)*(v4.yyyy)+(r1.zzzz)).y;
    // 91: mad r0.xy, cb0[8].xxxx, r0.xyxx, r1.xyxx
    r0.xy = ((source[8].xxxx)*(r0.xyxx)+(r1.xyxx)).xy;
    // 92: mul r0.xy, r0.xyxx, l(1.500000, 0.250000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(1.500000,0.250000,0.000000,0.000000))).xy;
    // 93: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s0, l(-1.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 94: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 95: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 96: mul r0.x, r0.x, cb0[8].y
    r0.x = ((r0.xxxx)*(source[8].yyyy)).x;
    // 97: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 98: mul r0.x, r0.x, cb0[8].z
    r0.x = ((r0.xxxx)*(source[8].zzzz)).x;
    // 99: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 100: mad r0.xyz, r0.xxxx, r2.xyzx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 101: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 102: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 103: mad r1.xy, v2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 104: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 105: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 106: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 107: mul r1.x, r1.x, cb0[13].x
    r1.x = ((r1.xxxx)*(source[13].xxxx)).x;
    // 108: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 109: mul r1.x, r1.x, cb0[13].y
    r1.x = ((r1.xxxx)*(source[13].yyyy)).x;
    // 110: lt r1.y, r0.w, l(0.000001)
    r1.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 111: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 113: log r1.y, r0.w
    r1.y = (log2(r0.wwww)).y;
    // 114: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 115: mul r1.y, r1.y, cb0[12].w
    r1.y = ((r1.yyyy)*(source[12].wwww)).y;
    // 116: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 117: movc r0.w, r0.w, l(0), r1.y
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).w;
    // 118: mul_sat r0.w, r1.x, r0.w
    r0.w = (saturate((r1.xxxx)*(r0.wwww))).w;
    // 119: mul r0.w, r0.w, cb0[13].z
    r0.w = ((r0.wwww)*(source[13].zzzz)).w;
    // 120: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 121: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 122: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 123: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_crackmove_01_02_tr: 0a5a7ebca2041e418f40b9e0aae26309; selected map 31a2db99f0537a635a2e725c31cd4ce624630eab51e7af2cc5e0a88b7628cb4b.
float4 ArtistNative2994(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[3].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[4].x = ((g_ArtistSourceMaterialParameters[3u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 2: max r0.z, |r0.x|, |r0.y|
    r0.z = (max(abs(r0.xxxx),abs(r0.yyyy))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.x|, |r0.y|
    r0.w = (min(abs(r0.xxxx),abs(r0.yyyy))).w;
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
    // 13: lt r1.y, |r0.x|, |r0.y|
    r1.y = (asfloat((uint4)((abs(r0.xxxx))<(abs(r0.yyyy))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.x, -r0.x
    r0.w = (asfloat((uint4)((r0.xxxx)<(-(r0.xxxx))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.x, r0.y
    r0.w = (min(r0.xxxx,r0.yyyy)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.x, r0.y
    r1.x = (max(r0.xxxx,r0.yyyy)).x;
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
    // 27: mul r0.y, r0.y, cb0[3].z
    r0.y = ((r0.yyyy)*(source[3].zzzz)).y;
    // 28: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: dp2 r0.z, r0.zzzz, cb0[3].wwww
    r0.z = (dot((r0.zzzz).xy,(source[3].wwww).xy).xxxx).z;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).y;
    // 31: add r0.y, r0.y, cb0[4].x
    r0.y = ((r0.yyyy)+(source[4].xxxx)).y;
    // 32: add r0.z, r0.y, cb0[5].y
    r0.z = ((r0.yyyy)+(source[5].yyyy)).z;
    // 33: add r0.y, r0.y, cb0[7].z
    r0.y = ((r0.yyyy)+(source[7].zzzz)).y;
    // 34: mul r1.x, r0.y, cb0[7].w
    r1.x = ((r0.yyyy)*(source[7].wwww)).x;
    // 35: mul r2.x, r0.z, cb0[5].z
    r2.x = ((r0.zzzz)*(source[5].zzzz)).x;
    // 36: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 37: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 38: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 39: mul r0.y, r0.y, cb0[4].y
    r0.y = ((r0.yyyy)*(source[4].yyyy)).y;
    // 40: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 41: mul r0.y, r0.y, cb0[4].z
    r0.y = ((r0.yyyy)*(source[4].zzzz)).y;
    // 42: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 43: add r0.y, v4.x, cb0[4].w
    r0.y = ((v4.xxxx)+(source[4].wwww)).y;
    // 44: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 45: add r0.y, r0.x, cb0[6].x
    r0.y = ((r0.xxxx)+(source[6].xxxx)).y;
    // 46: add r0.x, r0.x, cb0[8].y
    r0.x = ((r0.xxxx)+(source[8].yyyy)).x;
    // 47: mul r1.y, r0.x, cb0[8].z
    r1.y = ((r0.xxxx)*(source[8].zzzz)).y;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r0.xzw, r1.xyxx, t0.xwyz, s1, l(-1.000000)
    r0.xzw = (ArtistNativeSample1((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xwyz).xzw;
    // 49: mul r2.y, r0.y, cb0[6].y
    r2.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s0, l(0.000000)
    r1.xyz = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 51: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 52: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 53: mul r1.xyz, r1.xyzx, cb0[6].zzzz
    r1.xyz = ((r1.xyzx)*(source[6].zzzz)).xyz;
    // 54: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 55: mul r2.xyz, r1.xyzx, cb0[6].wwww
    r2.xyz = ((r1.xyzx)*(source[6].wwww)).xyz;
    // 56: dp3 r0.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 57: mad r1.xyz, -cb0[6].wwww, r1.xyzx, r0.yyyy
    r1.xyz = ((-(source[6].wwww))*(r1.xyzx)+(r0.yyyy)).xyz;
    // 58: mad r1.xyz, cb0[7].xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((source[7].xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 59: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 60: mul r0.yzw, r0.xxzw, r1.xxyz
    r0.yzw = ((r0.xxzw)*(r1.xxyz)).yzw;
    // 61: mul r0.yzw, r0.yyzw, cb0[8].wwww
    r0.yzw = ((r0.yyzw)*(source[8].wwww)).yzw;
    // 62: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 63: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.xwyz, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 65: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 66: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 67: mul r0.z, r0.z, cb0[9].x
    r0.z = ((r0.zzzz)*(source[9].xxxx)).z;
    // 68: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 69: mul r0.z, r0.z, cb0[9].y
    r0.z = ((r0.zzzz)*(source[9].yyyy)).z;
    // 70: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 71: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 72: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 73: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_elecline_01_ad: 1d2f1757fa22f548a36324db01dd2e35; selected map 5232400830336d03874420cad0a8dcaf2cb62a049b83f81ab64eafa8c8ded93c.
float4 ArtistNative2995(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].y = ((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))).x;
    source[2].z = (((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[2].w = (sin(((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[3].x = ((sin(((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[3].y = (abs((sin(((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (fmod(abs((sin(((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))),float4(1.5, 0.0, 0.0, 0.0))).x;
    source[3].w = ((fmod(abs((sin(((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))),float4(1.5, 0.0, 0.0, 0.0))+float4(0.300000012, 0.0, 0.0, 0.0))).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f;
    // 1: mul r0.x, v4.z, cb0[2].x
    r0.x = ((v4.zzzz)*(source[2].xxxx)).x;
    // 2: mul r0.y, r0.x, l(-0.500000)
    r0.y = ((r0.xxxx)*(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 3: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 4: mad r0.xy, v2.xyxx, l(2.000000, 0.250000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v2.xyxx)*(float4(2.000000,0.250000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: mad r0.yz, v2.xxyx, l(0.000000, 1.000000, 0.800000, 0.000000), l(0.000000, 0.000000, 0.100000, 0.000000)
    r0.yz = ((v2.xxyx)*(float4(0.000000,1.000000,0.800000,0.000000))+(float4(0.000000,0.000000,0.100000,0.000000))).yz;
    // 7: mad r0.yz, r0.xxxx, v4.wwww, r0.yyzy
    r0.yz = ((r0.xxxx)*(v4.wwww)+(r0.yyzy)).yz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 9: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 10: mul r0.y, v2.y, l(0.800000)
    r0.y = ((v2.yyyy)*(float4(0.800000,0.800000,0.800000,0.800000))).y;
    // 11: mul r0.y, |r0.y|, l(60.000000)
    r0.y = ((abs(r0.yyyy))*(float4(60.000000,60.000000,60.000000,60.000000))).y;
    // 12: round_pi r0.y, r0.y
    r0.y = (ceil(r0.yyyy)).y;
    // 13: mad r0.y, -r0.y, l(0.016667), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(0.016667,0.016667,0.016667,0.016667))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 14: add r0.z, -v4.x, l(1.000000)
    r0.z = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 15: add_sat r0.y, -r0.z, r0.y
    r0.y = (saturate((-(r0.zzzz))+(r0.yyyy))).y;
    // 16: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 17: mul r0.y, r0.y, cb0[3].w
    r0.y = ((r0.yyyy)*(source[3].wwww)).y;
    // 18: mad r0.yzw, r0.yyyy, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 19: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 20: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 21: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 22: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 23: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_me_transition_04_1_ma: 496aee5a3382744d97f9dcdb89b6ae6c; selected map f9be08d5e66242bc426fcb73591edacf680f68a2acf227bb3d834507dd494ecc.
float4 ArtistNative2996(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.150000006, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.150000006, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = g_ArtistSourceMaterialParameters[3u];
    source[7] = input.dynamicParameter;
    source[8] = g_ArtistSourceMaterialParameters[1u];
    source[9] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[10].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v4.xyxx, cb0[9].xyxx
    r0.xy = ((v4.xyxx)+(source[9].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s5, l(0.000000)
    r0.xy = (ArtistNativeSample5((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mul r0.zw, r0.xxxy, cb0[10].wwww
    r0.zw = ((r0.xxxy)*(source[10].wwww)).zw;
    // 4: mad r0.xy, cb0[10].zzzz, r0.xyxx, v4.xyxx
    r0.xy = ((source[10].zzzz)*(r0.xyxx)+(v4.xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s5, l(0.000000)
    r0.x = (ArtistNativeSample5((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: mad r0.y, -cb0[10].y, cb0[7].x, l(1.000000)
    r0.y = ((-(source[10].yyyy))*(source[7].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 7: mad r0.yz, r0.yyyy, r0.zzwz, v4.xxyx
    r0.yz = ((r0.yyyy)*(r0.zzwz)+(v4.xxyx)).yz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t2.xyzw, s6, l(0.000000)
    r1.xyzw = (ArtistNativeSample6((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 9: mul r0.y, r1.w, l(1.150000)
    r0.y = ((r1.wwww)*(float4(1.150000,1.150000,1.150000,1.150000))).y;
    // 10: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 11: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 12: mul r0.z, r0.z, l(1.150000)
    r0.z = ((r0.zzzz)*(float4(1.150000,1.150000,1.150000,1.150000))).z;
    // 13: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 14: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 15: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 16: mul r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)*(r0.xxxx)).y;
    // 17: mul r0.z, cb0[7].x, cb0[10].y
    r0.z = ((source[7].xxxx)*(source[10].yyyy)).z;
    // 18: mad r0.w, r0.z, l(0.100000), l(0.900000)
    r0.w = ((r0.zzzz)*(float4(0.100000,0.100000,0.100000,0.100000))+(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 19: mad r0.y, r0.z, r0.y, r0.w
    r0.y = ((r0.zzzz)*(r0.yyyy)+(r0.wwww)).y;
    // 20: max r0.y, r0.y, l(0.900000)
    r0.y = (max(r0.yyyy,float4(0.900000,0.900000,0.900000,0.900000))).y;
    // 21: round_ni r0.y, r0.y
    r0.y = (floor(r0.yyyy)).y;
    // 22: min r1.w, r0.y, l(1.000000)
    r1.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: mad r1.w, cb0[0].w, r1.w, l(-0.333000)
    r1.w = ((source[0].wwww)*(r1.wwww)+(float4(-0.333000,-0.333000,-0.333000,-0.333000))).w;
    // 24: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 25: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) clip(-1.f);
    // 26: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 27: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 28: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: mul r1.w, r1.w, cb0[11].x
    r1.w = ((r1.wwww)*(source[11].xxxx)).w;
    // 30: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 31: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 32: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 33: add_sat r0.x, r0.x, r0.w
    r0.x = (saturate((r0.xxxx)+(r0.wwww))).x;
    // 34: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 35: add r0.x, -r0.x, r0.y
    r0.x = ((-(r0.xxxx))+(r0.yyyy)).x;
    // 36: mul r0.yzw, cb0[8].xxyz, cb0[8].wwww
    r0.yzw = ((source[8].xxyz)*(source[8].wwww)).yzw;
    // 37: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 38: mad r2.xy, v4.xyxx, l(0.800000, 0.300000, 0.000000, 0.000000), cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(float4(0.800000,0.300000,0.000000,0.000000))+(source[2].xyxx)).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t3.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 40: mul r2.xy, v4.xyxx, l(0.800000, 0.600000, 0.000000, 0.000000)
    r2.xy = ((v4.xyxx)*(float4(0.800000,0.600000,0.000000,0.000000))).xy;
    // 41: mad r2.xy, r0.wwww, l(0.800000, 0.800000, 0.000000, 0.000000), r2.xyxx
    r2.xy = ((r0.wwww)*(float4(0.800000,0.800000,0.000000,0.000000))+(r2.xyxx)).xy;
    // 42: add r2.xy, r2.xyxx, cb0[3].xyxx
    r2.xy = ((r2.xyxx)+(source[3].xyxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t4.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 44: mad r2.xy, v4.xyxx, l(0.600000, 0.300000, 0.000000, 0.000000), cb0[4].xyxx
    r2.xy = ((v4.xyxx)*(float4(0.600000,0.300000,0.000000,0.000000))+(source[4].xyxx)).xy;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t3.yzwx, s1, l(0.000000)
    r1.w = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 46: mad r2.xy, v4.xyxx, l(0.750000, 0.500000, 0.000000, 0.000000), r1.wwww
    r2.xy = ((v4.xyxx)*(float4(0.750000,0.500000,0.000000,0.000000))+(r1.wwww)).xy;
    // 47: add r2.xy, r2.xyxx, cb0[5].xyxx
    r2.xy = ((r2.xyxx)+(source[5].xyxx)).xy;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t5.xywz, s3, l(0.000000)
    r1.w = (ArtistNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).w;
    // 49: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 50: mul r0.w, r0.w, l(12.000000)
    r0.w = ((r0.wwww)*(float4(12.000000,12.000000,12.000000,12.000000))).w;
    // 51: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 52: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 53: mul r1.w, r1.w, l(1.250000)
    r1.w = ((r1.wwww)*(float4(1.250000,1.250000,1.250000,1.250000))).w;
    // 54: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 55: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r1.w, v4.xyxx, t6.yzwx, s4, l(0.000000)
    r1.w = (ArtistNativeSample4((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 57: mul r2.xyz, r1.wwww, cb0[6].xyzx
    r2.xyz = ((r1.wwww)*(source[6].xyzx)).xyz;
    // 58: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 59: mad r0.xyz, cb0[7].yyyy, r2.xyzx, r0.xyzx
    r0.xyz = ((source[7].yyyy)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 60: mad r0.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r0.xyzx)).xyz;
    // 61: mad o0.xyz, r0.xyzx, cb0[0].xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)*(source[0].xyzx)+(source[1].xyzx)).xyz;
    // 62: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_de_ap_01_2_tr: aacf33d926f3884493fb98d76d43506c; selected map 9e5e2d4b6711cec9a3193ae4fdb6c267fe2d912c22b7615b8a6d44e4afcc6609.
float4 ArtistNative2997(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(input.decalProjection.xy,0.f,0.f);
    source[1]=input.color; // Source decal material color, including particle color modules.
    source[2].x=input.decalProjection.z;
    source[3] = g_ArtistSourceMaterialParameters[3u];
    source[4] = g_ArtistSourceMaterialParameters[2u];
    source[5].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.
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
    // 1: add r0.xy, -v4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(v4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 2: mul r0.xy, r0.xyxx, v4.xyxx
    r0.xy = ((r0.xyxx)*(v4.xyxx)).xy;
    // 3: add r1.x, v4.w, -cb0[0].x
    r1.x = ((v4.wwww)+(-(source[0].xxxx))).x;
    // 4: add r0.z, r1.x, l(0.001000)
    r0.z = ((r1.xxxx)+(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 5: add r1.x, -v4.w, cb0[0].y
    r1.x = ((-(v4.wwww))+(source[0].yyyy)).x;
    // 6: add r0.w, r1.x, l(0.001000)
    r0.w = ((r1.xxxx)+(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 7: lt r0.xyzw, r0.xyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyzw = (asfloat((uint4)((r0.xyzw)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xyzw;
    // 8: or r0.xy, r0.zwzz, r0.xyxx
    r0.xy = (asfloat(asuint(r0.zwzz) | asuint(r0.xyxx))).xy;
    // 9: or r0.x, r0.y, r0.x
    r0.x = (asfloat(asuint(r0.yyyy) | asuint(r0.xxxx))).x;
    // 10: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 11: add r0.x, cb0[5].x, l(-1.000000)
    r0.x = ((source[5].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 12: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 13: mad r0.xy, cb0[5].xxxx, v4.xyxx, -r0.xxxx
    r0.xy = ((source[5].xxxx)*(v4.xyxx)+(-(r0.xxxx))).xy;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 15: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 16: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 17: mad r0.xyz, cb0[5].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[5].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 18: mul r1.xyz, r0.xyzx, cb0[5].zzzz
    r1.xyz = ((r0.xyzx)*(source[5].zzzz)).xyz;
    // 19: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 20: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 21: mul r1.xyz, r1.xyzx, cb0[5].wwww
    r1.xyz = ((r1.xyzx)*(source[5].wwww)).xyz;
    // 22: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 23: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 24: add r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)+(r1.xxxx)).xyz;
    // 25: mul r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)).xyz;
    // 26: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 27: mad r0.xyz, r1.xyzx, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)+(source[3].xyzx)).xyz;
    // 28: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 29: log r0.x, |r0.w|
    r0.x = (log2(abs(r0.wwww))).x;
    // 30: lt r0.y, |r0.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 31: mul r0.x, r0.x, cb0[6].x
    r0.x = ((r0.xxxx)*(source[6].xxxx)).x;
    // 32: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 33: mul r0.x, r0.x, cb0[6].y
    r0.x = ((r0.xxxx)*(source[6].yyyy)).x;
    // 34: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 35: mul r0.x, r0.x, cb0[2].x
    r0.x = ((r0.xxxx)*(source[2].xxxx)).x;
    // 36: add r0.z, -|v4.w|, cb0[0].y
    r0.z = ((-(abs(v4.wwww)))+(source[0].yyyy)).z;
    // 37: mul r0.z, r0.z, l(5.000000)
    r0.z = ((r0.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 38: div_sat r0.z, r0.z, cb0[0].y
    r0.z = (saturate((r0.zzzz)/(source[0].yyyy))).z;
    // 39: mul r0.z, r0.z, v4.z
    r0.z = ((r0.zzzz)*(v4.zzzz)).z;
    // 40: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 41: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_h_de_ht_01_1_tr: 2f90e06395943342a0358e5a108cc044; selected map 9a6f1fc9e4fe5015321d753c912b8a6302a00dabafd97c601893358b09a32ef0.
float4 ArtistNative2998(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(input.decalProjection.xy,0.f,0.f);
    source[1]=input.color; // Source decal material color, including particle color modules.
    source[2].x=input.decalProjection.z;
    source[3] = g_ArtistSourceMaterialParameters[4u];
    source[4] = g_ArtistSourceMaterialParameters[3u];
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].yyyy*float4(3.14159203, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(3.14159203, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[7] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[0u].yyyy*float4(3.14159203, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[0u].yyyy*float4(3.14159203, 0.0, 0.0, 0.0))),1u);
    source[8] = g_ArtistSourceMaterialParameters[2u];
    source[9].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.
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
    // 1: add r0.x, v4.w, -cb0[0].x
    r0.x = ((v4.wwww)+(-(source[0].xxxx))).x;
    // 2: add r0.z, r0.x, l(0.001000)
    r0.z = ((r0.xxxx)+(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 3: add r1.x, -v4.w, cb0[0].y
    r1.x = ((-(v4.wwww))+(source[0].yyyy)).x;
    // 4: add r0.w, r1.x, l(0.001000)
    r0.w = ((r1.xxxx)+(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 5: add r1.xyzw, -v4.xyxy, l(1.000000, 1.000000, 0.500000, 0.500000)
    r1.xyzw = ((-(v4.xyxy))+(float4(1.000000,1.000000,0.500000,0.500000))).xyzw;
    // 6: mul r0.xy, r1.xyxx, v4.xyxx
    r0.xy = ((r1.xyxx)*(v4.xyxx)).xy;
    // 7: mad r1.xy, cb0[6].xyxx, r1.zwzz, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((source[6].xyxx)*(r1.zwzz)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 8: lt r0.xyzw, r0.xyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyzw = (asfloat((uint4)((r0.xyzw)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xyzw;
    // 9: or r0.xy, r0.zwzz, r0.xyxx
    r0.xy = (asfloat(asuint(r0.zwzz) | asuint(r0.xyxx))).xy;
    // 10: or r0.x, r0.y, r0.x
    r0.x = (asfloat(asuint(r0.yyyy) | asuint(r0.xxxx))).x;
    // 11: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r1.xyxx, t2.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t0.xyzw, s2, l(0.000000)
    r1.xyzw = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 14: add r2.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 15: dp2 r0.w, cb0[7].xyxx, r2.xyxx
    r0.w = (dot((source[7].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 16: dp2 r2.x, cb0[5].xyxx, r2.xyxx
    r2.x = (dot((source[5].xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 17: add r2.x, r2.x, l(0.500000)
    r2.x = ((r2.xxxx)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 18: add r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 19: mad r2.yz, -cb0[1].xxyx, l(0.000000, 3.500000, 3.500000, 0.000000), l(0.000000, 1.000000, 1.000000, 0.000000)
    r2.yz = ((-(source[1].xxyx))*(float4(0.000000,3.500000,3.500000,0.000000))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 20: mad r0.w, r2.y, l(-2.000000), r0.w
    r0.w = ((r2.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(r0.wwww)).w;
    // 21: mad r2.x, r2.z, l(-2.000000), r2.x
    r2.x = ((r2.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(r2.xxxx)).x;
    // 22: add r2.x, r2.x, l(1.000000)
    r2.x = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xxxx, t1.xyzw, s0, l(0.000000)
    r2.xyz = (ArtistNativeSample0((r2.xxxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 24: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.wwww, t1.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r0.wwww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 26: log r2.w, |r0.w|
    r2.w = (log2(abs(r0.wwww))).w;
    // 27: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 28: mul r2.w, r2.w, cb0[9].x
    r2.w = ((r2.wwww)*(source[9].xxxx)).w;
    // 29: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 30: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: movc r0.w, r0.w, l(0), r2.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 32: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 33: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 34: mul r0.w, r0.w, cb0[1].w
    r0.w = ((r0.wwww)*(source[1].wwww)).w;
    // 35: mul r0.w, r0.w, cb0[9].z
    r0.w = ((r0.wwww)*(source[9].zzzz)).w;
    // 36: mul r0.w, r0.w, cb0[2].x
    r0.w = ((r0.wwww)*(source[2].xxxx)).w;
    // 37: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 38: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 39: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 40: mad r1.xyz, cb0[9].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[9].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 41: mul r2.xyz, cb0[8].xyzx, cb0[8].wwww
    r2.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 42: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 43: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 44: mad r0.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 45: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 46: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 47: add r0.x, -|v4.w|, cb0[0].y
    r0.x = ((-(abs(v4.wwww)))+(source[0].yyyy)).x;
    // 48: mul r0.x, r0.x, l(5.000000)
    r0.x = ((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 49: div_sat r0.x, r0.x, cb0[0].y
    r0.x = (saturate((r0.xxxx)/(source[0].yyyy))).x;
    // 50: mul r0.x, r0.x, v4.z
    r0.x = ((r0.xxxx)*(v4.zzzz)).x;
    // 51: mul o0.w, r0.x, r0.w
    output.w = ((r0.xxxx)*(r0.wwww)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_pa_sy_09_1_tr: 5d016e3a03a0bd47bdcbdf53e041895a; selected map c03fbe6f95732220e284e7a5e11c86cec93a86453490a2dcf8a3b2d908811a05.
float4 ArtistNative2999(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.zwzz, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 3: add r0.xyzw, r0.wxyz, -r1.wxyz
    r0.xyzw = ((r0.wxyz)+(-(r1.wxyz))).xyzw;
    // 4: mad r0.xyzw, v0.wwww, r0.xyzw, r1.wxyz
    r0.xyzw = ((v0.wwww)*(r0.xyzw)+(r1.wxyz)).xyzw;
    // 5: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 6: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 7: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 8: mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // 9: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_master_01_034_tr: 4a3531bef578fc45a409304782c6ea1d; selected map 8446586b25116261a35b1d8a7905e3794e78fb8cc760829b384e25cff09f6887.
float4 ArtistNative3000(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].zzzz)).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    // 39: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 40: add r0.xyz, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((source[1].xyzx)+(source[2].xyzx)).xyz;
    // 41: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_o_pa_ap_23_1_tr: c9e2873553a59d42857bb5e72fae97c3; selected map 10507a3a0c024475158714574bdf0b83d9009d1538f833f7cc434126ffd9fcec.
float4 ArtistNative3001(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = ArtistNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[5].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[5].z = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[5].w = (clamp(g_ArtistSourceMaterialParameters[5u].wwww,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[6].x = ((float4(1.0, 0.0, 0.0, 0.0)-clamp(g_ArtistSourceMaterialParameters[5u].wwww,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[10].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[11].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[11].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx))).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.xz, r2.xyxx, t0.xzyw, s1, l(0.000000)
    r0.xz = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).xz;
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
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 21: mad r2.x, cb0[4].y, cb0[7].w, r0.x
    r2.x = ((source[4].yyyy)*(source[7].wwww)+(r0.xxxx)).x;
    // 22: mad r2.y, cb0[4].y, cb0[8].z, r0.z
    r2.y = ((source[4].yyyy)*(source[8].zzzz)+(r0.zzzz)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    // 49: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 50: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 51: source device depth mapped to centimetre view depth; reconstruction at 53.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 53-56: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 57: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 58: add r0.w, -cb0[11].y, l(1.000000)
    r0.w = ((-(source[11].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 60: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 61: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 62: log r0.w, r0.y
    r0.w = (log2(r0.yyyy)).w;
    // 63: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 64: mul r0.w, r0.w, cb0[10].z
    r0.w = ((r0.wwww)*(source[10].zzzz)).w;
    // 65: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 66: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 67: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 68: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 69: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 70: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 71: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 72: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// defaultparticle: 28cf3fbe8cf0284bb4763666903e6592; selected map da488990b2a381a016b6c591c2176e26b57a0a8555a1303e4bdf4fecc3cb5a55.
float4 ArtistNative3002(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[0u];
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: add o0.xyz, r0.xyzx, cb0[0].xyzx
    output.xyz = ((r0.xyzx)+(source[0].xyzx)).xyz;
    // 4: lt r0.x, r0.w, l(0.000000)
    r0.x = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_lightingdetail_02_ad: e77f5eec01304e4781e66f7d65f60233; selected map b4573c2d6c75a504cf2adeb6118edc702b43f3f6d959b26b17c3c3724d2a099a.
float4 ArtistNative3003(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].y = ((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))).x;
    source[2].z = (((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[2].w = (sin(((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[3].x = ((sin(((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[3].y = (abs((sin(((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (fmod(abs((sin(((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))),float4(1.5, 0.0, 0.0, 0.0))).x;
    source[3].w = ((fmod(abs((sin(((g_ArtistSourceMaterialTime.xxxx*float4(1.5, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))),float4(1.5, 0.0, 0.0, 0.0))+float4(0.300000012, 0.0, 0.0, 0.0))).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f;
    // 1: mul r0.x, v4.y, cb0[2].x
    r0.x = ((v4.yyyy)*(source[2].xxxx)).x;
    // 2: mul r0.y, r0.x, l(-0.500000)
    r0.y = ((r0.xxxx)*(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 3: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 4: mad r0.xy, v2.xyxx, l(1.000000, 0.250000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v2.xyxx)*(float4(1.000000,0.250000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 7: mul r0.y, r0.y, v4.x
    r0.y = ((r0.yyyy)*(v4.xxxx)).y;
    // 8: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 9: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 10: mad r0.xw, r0.xxxx, l(0.010000, 0.000000, 0.000000, 0.010000), v2.xxxy
    r0.xw = ((r0.xxxx)*(float4(0.010000,0.000000,0.000000,0.010000))+(v2.xxxy)).xw;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xwxx, t1.zxyw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 12: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 13: mul r0.z, |r0.x|, |r0.x|
    r0.z = ((abs(r0.xxxx))*(abs(r0.xxxx))).z;
    // 14: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 15: add r0.w, -v2.y, l(1.000000)
    r0.w = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 16: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 17: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 18: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 19: mul r0.xyz, r0.xxxx, v3.xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)).xyz;
    // 20: mad r0.xyz, cb0[3].wwww, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((source[3].wwww)*(r0.xyzx)+(source[1].xyzx)).xyz;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_lbeam_01_1_ad: e916b8b428ce2d4f9e6c9c1f921545f6; selected map c6dd09439718068dd0b1df175b964425adda3531defec8efa86787f044491394.
float4 ArtistNative3004(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: add r0.x, v2.x, l(-0.500000)
    r0.x = ((v2.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 2: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 3: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 4: mul r0.y, r0.x, v2.y
    r0.y = ((r0.xxxx)*(v2.yyyy)).y;
    // 5: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 6: mad r0.x, r0.x, l(0.500000), r0.y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.yyyy)).x;
    // 7: sqrt r0.y, v2.y
    r0.y = (sqrt(v2.yyyy)).y;
    // 8: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 9: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 10: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 11: mul r0.y, r0.y, cb0[2].x
    r0.y = ((r0.yyyy)*(source[2].xxxx)).y;
    // 12: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 13: mul_sat r0.y, r0.y, cb0[2].y
    r0.y = (saturate((r0.yyyy)*(source[2].yyyy))).y;
    // 14: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 15: add r0.y, -v2.y, l(1.000000)
    r0.y = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 16: mul_sat r0.y, r0.y, cb0[2].z
    r0.y = (saturate((r0.yyyy)*(source[2].zzzz))).y;
    // 17: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 18: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 19: mul r0.z, r0.z, cb0[2].w
    r0.z = ((r0.zzzz)*(source[2].wwww)).z;
    // 20: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 21: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 22: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 23: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 24: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 25: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 26: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 27: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 28: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_shorkwave_01_17_tr: 5c707b8a1ddaf5478de6e37c4b55bf26; selected map f1f290972b5b31eec74b516c45ee383023e1c16a6224be179cff954bdcae7562.
float4 ArtistNative3005(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = (cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 23: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 24: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 25: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 26: mad r0.y, r0.y, l(0.318310), l(1.000000)
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 27: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 28: mul r1.x, r0.y, cb0[6].x
    r1.x = ((r0.yyyy)*(source[6].xxxx)).x;
    // 29: mul r2.xy, r0.yyyy, cb0[4].ywyy
    r2.xy = ((r0.yyyy)*(source[4].ywyy)).xy;
    // 30: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 31: mul r0.y, r0.y, l(0.800000)
    r0.y = ((r0.yyyy)*(float4(0.800000,0.800000,0.800000,0.800000))).y;
    // 32: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 33: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 34: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 35: mad r0.x, -r0.x, l(2.222222), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.222222,2.222222,2.222222,2.222222))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mul r0.x, r0.x, l(0.333333)
    r0.x = ((r0.xxxx)*(float4(0.333333,0.333333,0.333333,0.333333))).x;
    // 37: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 38: mul r0.x, r0.x, l(4.000000)
    r0.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 39: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 40: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 41: mul r2.z, r0.y, cb0[5].x
    r2.z = ((r0.yyyy)*(source[5].xxxx)).z;
    // 42: mad r0.zw, v4.zzzz, l(0.000000, 0.000000, -0.030000, -0.100000), r2.yyyz
    r0.zw = ((v4.zzzz)*(float4(0.000000,0.000000,-0.030000,-0.100000))+(r2.yyyz)).zw;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s0, l(0.000000)
    r0.zw = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 44: mad r3.xyzw, r0.zwzw, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.500000, -1.500000)
    r3.xyzw = ((r0.zwzw)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.500000,-1.500000))).xyzw;
    // 45: mul r1.y, r0.y, cb0[6].y
    r1.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 46: mul r2.w, r0.y, cb0[4].z
    r2.w = ((r0.yyyy)*(source[4].zzzz)).w;
    // 47: mad r0.yz, r3.xxyx, l(0.000000, 0.080000, 0.080000, 0.000000), r1.xxyx
    r0.yz = ((r3.xxyx)*(float4(0.000000,0.080000,0.080000,0.000000))+(r1.xxyx)).yz;
    // 48: mad r0.yz, v4.zzzz, l(0.000000, -0.050000, -0.100000, 0.000000), r0.yyzy
    r0.yz = ((v4.zzzz)*(float4(0.000000,-0.050000,-0.100000,0.000000))+(r0.yyzy)).yz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 50: add r0.zw, v4.yyyx, l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((v4.yyyx)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 51: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 52: add_sat r0.y, r0.y, r0.y
    r0.y = (saturate((r0.yyyy)+(r0.yyyy))).y;
    // 53: dp2 r1.x, l(0.000796, -1.000000, 0.000000, 0.000000), r3.zwzz
    r1.x = (dot((float4(0.000796,-1.000000,0.000000,0.000000)).xy,(r3.zwzz).xy).xxxx).x;
    // 54: dp2 r1.y, l(1.000000, 0.000796, 0.000000, 0.000000), r3.zwzz
    r1.y = (dot((float4(1.000000,0.000796,0.000000,0.000000)).xy,(r3.zwzz).xy).xxxx).y;
    // 55: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 56: mad r1.xy, r0.zzzz, r1.xyxx, r2.xwxx
    r1.xy = ((r0.zzzz)*(r1.xyxx)+(r2.xwxx)).xy;
    // 57: mad r1.z, v4.w, l(0.200000), r1.y
    r1.z = ((v4.wwww)*(float4(0.200000,0.200000,0.200000,0.200000))+(r1.yyyy)).z;
    // 58: add r0.zw, r1.xxxz, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r1.xxxz)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 59: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 60: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 61: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 63: add r0.z, r1.x, r1.x
    r0.z = ((r1.xxxx)+(r1.xxxx)).z;
    // 64: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 65: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 66: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 67: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 68: max r0.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 69: mul r1.xyz, r1.xyzx, cb0[5].wwww
    r1.xyz = ((r1.xyzx)*(source[5].wwww)).xyz;
    // 70: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 71: mul r0.xyz, r0.xyzx, cb0[5].yyyy
    r0.xyz = ((r0.xyzx)*(source[5].yyyy)).xyz;
    // 72: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 73: mad r0.xyz, cb0[5].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[5].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 74: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 75: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_ribbonmaster_01_8_tr: b6db3d037e632e4facb4d84956e8d55b; selected map 3bab04ecc04f0ae6a0ed99503bc46aeaa09ac2ac0e23effbf13179b8adba5bae.
float4 ArtistNative3006(ARTIST_NATIVE_INPUT input)
{
    float4 source[19]; [unroll] for (uint i=0u; i<19u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].xxxx,g_ArtistSourceMaterialParameters[4u].yyyy,1u);
    source[3] = ArtistNativeAppend(cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[8] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].wwww,g_ArtistSourceMaterialParameters[3u].xxxx,1u);
    source[10].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[10].y = (cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[11].x = ((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)).x;
    source[11].y = (((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))).x;
    source[11].z = (((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[11].w = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[12].x = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[12].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0)))).x;
    source[12].w = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[13].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[13].z = ((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[13].w = (((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))).x;
    source[14].x = (((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[14].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[14].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[14].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[15].x = ((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)).x;
    source[15].y = (((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))).x;
    source[15].z = (((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[15].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[16].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[16].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[16].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0)))).x;
    source[16].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[17].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[17].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[17].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[17].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[18].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[18].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: mul r0.x, v2.z, cb0[15].w
    r0.x = ((v2.zzzz)*(source[15].wwww)).x;
    // 2: mul r0.y, v2.w, cb0[16].x
    r0.y = ((v2.wwww)*(source[16].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, cb0[8].xyxx
    r0.xy = ((r0.xyxx)+(source[8].xyxx)).xy;
    // 4: add r0.xy, r0.xyxx, cb0[9].xyxx
    r0.xy = ((r0.xyxx)+(source[9].xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: mad r0.yz, cb0[14].yyzy, v2.zzwz, cb0[6].xxyx
    r0.yz = ((source[14].yyzy)*(v2.zzwz)+(source[6].xxyx)).yz;
    // 7: add r0.yz, r0.yyzy, cb0[7].xxyx
    r0.yz = ((r0.yyzy)+(source[7].xxyx)).yz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 9: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 10: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 11: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 12: mul r0.y, r0.y, cb0[17].y
    r0.y = ((r0.yyyy)*(source[17].yyyy)).y;
    // 13: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 14: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 15: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 16: mul r0.y, r0.y, cb0[17].z
    r0.y = ((r0.yyyy)*(source[17].zzzz)).y;
    // 17: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 18: mul r0.y, r0.y, cb0[17].w
    r0.y = ((r0.yyyy)*(source[17].wwww)).y;
    // 19: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 20: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 21: mad r0.x, r0.x, cb0[18].x, r0.y
    r0.x = ((r0.xxxx)*(source[18].xxxx)+(r0.yyyy)).x;
    // 22: mad r0.yz, cb0[10].zzwz, v2.xxyx, cb0[2].xxyx
    r0.yz = ((source[10].zzwz)*(v2.xxyx)+(source[2].xxyx)).yz;
    // 23: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 24: dp2 r1.x, cb0[3].xyxx, r0.yzyy
    r1.x = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 25: dp2 r1.y, cb0[4].xyxx, r0.yzyy
    r1.y = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 26: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 27: mul r1.x, v2.z, cb0[11].w
    r1.x = ((v2.zzzz)*(source[11].wwww)).x;
    // 28: mul r1.y, v2.w, cb0[12].x
    r1.y = ((v2.wwww)*(source[12].xxxx)).y;
    // 29: add r1.xy, r1.xyxx, cb0[5].xyxx
    r1.xy = ((r1.xyxx)+(source[5].xyxx)).xy;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 31: mul r1.xy, r1.xyxx, cb0[12].wwww
    r1.xy = ((r1.xyxx)*(source[12].wwww)).xy;
    // 32: mad_sat r0.w, v2.x, l(5.000000), l(-0.400000)
    r0.w = (saturate((v2.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(float4(-0.400000,-0.400000,-0.400000,-0.400000)))).w;
    // 33: mad r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((r0.wwww)*(r1.xxyx)+(r0.yyzy)).yz;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 35: mad r0.x, r0.y, cb0[13].x, r0.x
    r0.x = ((r0.yyyy)*(source[13].xxxx)+(r0.xxxx)).x;
    // 36: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 37: mul r0.y, r0.y, cb0[18].y
    r0.y = ((r0.yyyy)*(source[18].yyyy)).y;
    // 38: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 39: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 40: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 41: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_v_pa_waterribbon_03_11_tr: 59a22eeec5a51f439595f929dddfe8bf; selected map 81d674443d6cdbfbf3ac4bbc2bffabd834c314ebbba584dd43fb398eca9cb191.
float4 ArtistNative3007(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[8u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[4] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww),float4(0.0, 0.0, 0.0, 0.0),1u);
    source[5] = g_ArtistSourceMaterialParameters[6u];
    source[6] = g_ArtistSourceMaterialParameters[7u];
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].yyyy,g_ArtistSourceMaterialParameters[5u].zzzz,1u);
    source[8] = ArtistNativeAppend(float4(0.100000001, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[9] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww),float4(0.0, 0.0, 0.0, 0.0),1u);
    source[10].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[11].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[11].w = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[12].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[13].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[13].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww)).x;
    source[13].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[14].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[15].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
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
    // 1: mov r0.y, cb0[10].z
    r0.y = (source[10].zzzz).y;
    // 2: mov r0.xz, l(0,0,1.000000,0)
    r0.xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 3: mad r1.xyzw, v2.zwzw, cb0[3].xyxy, r0.yxxy
    r1.xyzw = ((v2.zwzw)*(source[3].xyxy)+(r0.yxxy)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r1.xy, v2.zwzz, cb0[3].xyxx
    r1.xy = ((v2.zwzz)*(source[3].xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r1.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: add r2.xy, r0.xyxx, -r1.xxxx
    r2.xy = ((r0.xyxx)+(-(r1.xxxx))).xy;
    // 9: mul r1.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: mov r1.z, l(0)
    r1.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 11: add r1.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 12: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 13: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 14: div r0.xy, r1.xyxx, r0.xxxx
    r0.xy = ((r1.xyxx)/(r0.xxxx)).xy;
    // 15: mad r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: mov r0.w, v4.x
    r0.w = (v4.xxxx).w;
    // 17: mul r0.xy, r0.zwzz, r0.xyxx
    r0.xy = ((r0.zwzz)*(r0.xyxx)).xy;
    // 18: add r0.zw, v2.zzzw, cb0[8].xxxy
    r0.zw = ((v2.zzzw)+(source[8].xxxy)).zw;
    // 19: mov r1.x, v4.x
    r1.x = (v4.xxxx).x;
    // 20: mov r1.y, l(0.200000)
    r1.y = (float4(0.200000,0.200000,0.200000,0.200000)).y;
    // 21: mad r0.zw, r1.xxxy, r0.xxxy, r0.zzzw
    r0.zw = ((r1.xxxy)*(r0.xxxy)+(r0.zzzw)).zw;
    // 22: mul r0.xy, r0.xyxx, r1.xyxx
    r0.xy = ((r0.xyxx)*(r1.xyxx)).xy;
    // 23: add r0.zw, r0.zzzw, cb0[9].xxxy
    r0.zw = ((r0.zzzw)+(source[9].xxxy)).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 25: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 26: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 27: mul r0.z, r0.z, cb0[13].z
    r0.z = ((r0.zzzz)*(source[13].zzzz)).z;
    // 28: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 29: mad r1.xy, v2.zwzz, cb0[7].xyxx, r0.xyxx
    r1.xy = ((v2.zwzz)*(source[7].xyxx)+(r0.xyxx)).xy;
    // 30: mad r0.xy, v2.zwzz, cb0[2].xyxx, r0.xyxx
    r0.xy = ((v2.zwzz)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 31: add r0.xy, r0.xyxx, cb0[4].xyxx
    r0.xy = ((r0.xyxx)+(source[4].xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t3.xywz, s1, l(0.000000)
    r0.xyw = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 34: add r1.w, r1.x, cb0[13].w
    r1.w = ((r1.xxxx)+(source[13].wwww)).w;
    // 35: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 36: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 37: mul r1.xyz, r1.xyzx, cb0[12].yyyy
    r1.xyz = ((r1.xyzx)*(source[12].yyyy)).xyz;
    // 38: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 39: mad r1.xyz, -cb0[12].zzzz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[12].zzzz))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 40: mul r1.w, r1.w, cb0[14].x
    r1.w = ((r1.wwww)*(source[14].xxxx)).w;
    // 41: mul r1.w, r1.w, v4.z
    r1.w = ((r1.wwww)*(v4.zzzz)).w;
    // 42: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 43: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 44: max r1.w, |r1.w|, l(0.000001)
    r1.w = (max(abs(r1.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 45: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 46: mul r1.w, r1.w, cb0[14].y
    r1.w = ((r1.wwww)*(source[14].yyyy)).w;
    // 47: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 48: mul_sat r1.w, r1.w, cb0[14].z
    r1.w = (saturate((r1.wwww)*(source[14].zzzz))).w;
    // 49: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 50: mul r0.z, r0.z, cb0[14].w
    r0.z = ((r0.zzzz)*(source[14].wwww)).z;
    // 51: mad r1.w, v2.z, l(2.000000), l(-1.000000)
    r1.w = ((v2.zzzz)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 52: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 54: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 55: mul r2.x, r2.x, cb0[15].x
    r2.x = ((r2.xxxx)*(source[15].xxxx)).x;
    // 56: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 57: mul r2.x, r2.x, l(3.000000)
    r2.x = ((r2.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 58: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 59: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 60: mul_sat r0.z, r0.z, r1.w
    r0.z = (saturate((r0.zzzz)*(r1.wwww))).z;
    // 61: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 62: mul o0.w, r0.z, cb0[0].x
    output.w = ((r0.zzzz)*(source[0].xxxx)).w;
    // 63: dp3 r0.z, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 64: add r2.xyz, -r0.xywx, r0.zzzz
    r2.xyz = ((-(r0.xywx))+(r0.zzzz)).xyz;
    // 65: mad r0.xyz, cb0[11].xxxx, r2.xyzx, r0.xywx
    r0.xyz = ((source[11].xxxx)*(r2.xyzx)+(r0.xywx)).xyz;
    // 66: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 67: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 68: mul r0.xyz, r0.xyzx, cb0[11].yyyy
    r0.xyz = ((r0.xyzx)*(source[11].yyyy)).xyz;
    // 69: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 70: mul r0.xyz, r0.xyzx, cb0[11].zzzz
    r0.xyz = ((r0.xyzx)*(source[11].zzzz)).xyz;
    // 71: add r2.xyz, -cb0[5].xyzx, cb0[6].xyzx
    r2.xyz = ((-(source[5].xyzx))+(source[6].xyzx)).xyz;
    // 72: mad r1.xyz, r1.xyzx, r2.xyzx, cb0[5].xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(source[5].xyzx)).xyz;
    // 73: mad r0.xyz, r0.xyzx, v3.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(r1.xyzx)).xyz;
    // 74: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 75: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2949Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: mul r1.xyz, v5.yyyy, cb1[1].xywx
    r1.xyz = ((v5.yyyy)*(projection[1].xywx)).xyz;
    // 7: mad r1.xyz, cb1[0].xywx, v5.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v5.xxxx)+(r1.xyzx)).xyz;
    // 8: mad r1.xyz, cb1[2].xywx, v5.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v5.zzzz)+(r1.xyzx)).xyz;
    // 9: mad r1.xyz, cb1[3].xywx, v5.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v5.wwww)+(r1.xyzx)).xyz;
    // 10: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 11: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 12: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 13: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 14: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 15: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 16: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 17: source device depth mapped to centimetre view depth; reconstruction at 19.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 19-22: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 23: ge r0.x, r1.z, r0.x
    r0.x = (asfloat((uint4)((r1.zzzz)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 24: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 25: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 26: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 27: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2953Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2954Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].zzzz,g_ArtistSourceMaterialParameters[4u].wwww,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].zzzz,g_ArtistSourceMaterialParameters[6u].wwww,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[7u].yyyy,g_ArtistSourceMaterialParameters[7u].zzzz,1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].yyyy,g_ArtistSourceMaterialParameters[5u].zzzz,1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[8u].yyyy,g_ArtistSourceMaterialParameters[8u].zzzz,1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[9u].xxxx,g_ArtistSourceMaterialParameters[9u].yyyy,1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,float4(1.0, 0.0, 0.0, 0.0),1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,float4(1.0, 0.0, 0.0, 0.0),1u);
    source[10].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[10].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[12].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[12].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[13].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[14].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[14].y = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[14].w = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[15].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[15].z = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[15].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[16].x = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[16].y = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[16].z = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[16].w = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[17].x = (g_ArtistSourceMaterialParameters[9u].yyyy).x;
    source[17].y = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[17].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[17].w = (g_ArtistSourceMaterialParameters[9u].zzzz).x;
    source[18].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[18].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[18].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[18].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[19].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[19].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[19].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[19].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mul r0.xyzw, v1.xyxy, l(1.500000, 1.500000, 3.000000, 3.000000)
    r0.xyzw = ((v1.xyxy)*(float4(1.500000,1.500000,3.000000,3.000000))).xyzw;
    // 2: mad r0.xy, cb0[10].xxxx, cb0[0].xyxx, r0.xyxx
    r0.xy = ((source[10].xxxx)*(source[0].xyxx)+(r0.xyxx)).xy;
    // 3: mad r0.zw, cb0[10].xxxx, cb0[5].xxxy, r0.zzzw
    r0.zw = ((source[10].xxxx)*(source[5].xxxy)+(r0.zzzw)).zw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t5.yzxw, s6, l(0.000000)
    r0.z = (ArtistNativeSample5((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 7: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 8: mul r0.y, r0.y, cb0[10].w
    r0.y = ((r0.yyyy)*(source[10].wwww)).y;
    // 9: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 10: mul r0.y, r0.y, cb0[11].x
    r0.y = ((r0.yyyy)*(source[11].xxxx)).y;
    // 11: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 12: mad r0.yw, cb0[10].xxxx, cb0[1].xxxy, v1.xxxy
    r0.yw = ((source[10].xxxx)*(source[1].xxxy)+(v1.xxxy)).yw;
    // 13: mul r0.yw, r0.yyyw, cb0[11].wwww
    r0.yw = ((r0.yyyw)*(source[11].wwww)).yw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, r0.ywyy, t1.zxwy, s2, l(0.000000)
    r0.yw = (ArtistNativeSample1((r0.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxwy).yw;
    // 15: add r1.xy, v1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 16: mul r1.zw, r1.xxxy, cb0[2].xxxy
    r1.zw = ((r1.xxxy)*(source[2].xxxy)).zw;
    // 17: mad r0.yw, cb0[12].xxxx, r0.yyyw, r1.zzzw
    r0.yw = ((source[12].xxxx)*(r0.yyyw)+(r1.zzzw)).yw;
    // 18: add r0.yw, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.ywyy, t2.xyzw, s3, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r0.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 20: mul r3.xyz, r2.xyzx, cb0[12].wwww
    r3.xyz = ((r2.xyzx)*(source[12].wwww)).xyz;
    // 21: mul r0.xyw, r0.xxxx, r3.xyxz
    r0.xyw = ((r0.xxxx)*(r3.xyxz)).xyw;
    // 22: mad r2.xyz, cb0[12].wwww, r2.xyzx, -r0.xywx
    r2.xyz = ((source[12].wwww)*(r2.xyzx)+(-(r0.xywx))).xyz;
    // 23: mad r0.xyw, cb0[13].xxxx, r2.xyxz, r0.xyxw
    r0.xyw = ((source[13].xxxx)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 24: log r1.z, |r0.z|
    r1.z = (log2(abs(r0.zzzz))).z;
    // 25: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r1.z, r1.z, cb0[15].z
    r1.z = ((r1.zzzz)*(source[15].zzzz)).z;
    // 27: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 28: mul r1.z, r1.z, cb0[15].w
    r1.z = ((r1.zzzz)*(source[15].wwww)).z;
    // 29: movc r0.z, r0.z, l(0), r1.z
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 30: mad r1.zw, cb0[10].xxxx, cb0[3].xxxy, v1.xxxy
    r1.zw = ((source[10].xxxx)*(source[3].xxxy)+(v1.xxxy)).zw;
    // 31: mul r1.zw, r1.zzzw, cb0[13].wwww
    r1.zw = ((r1.zzzw)*(source[13].wwww)).zw;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t3.zwxy, s4, l(0.000000)
    r1.zw = (ArtistNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 33: mul r2.xy, r1.xyxx, cb0[4].xyxx
    r2.xy = ((r1.xyxx)*(source[4].xyxx)).xy;
    // 34: mul r1.xy, r1.xyxx, cb0[7].xyxx
    r1.xy = ((r1.xyxx)*(source[7].xyxx)).xy;
    // 35: mad r1.zw, cb0[14].xxxx, r1.zzzw, r2.xxxy
    r1.zw = ((source[14].xxxx)*(r1.zzzw)+(r2.xxxy)).zw;
    // 36: add r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r1.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t4.xyzw, s5, l(0.000000)
    r2.xyz = (ArtistNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 38: mul r2.xyz, r2.xyzx, cb0[14].wwww
    r2.xyz = ((r2.xyzx)*(source[14].wwww)).xyz;
    // 39: mad r0.xyz, r0.zzzz, r2.xyzx, r0.xywx
    r0.xyz = ((r0.zzzz)*(r2.xyzx)+(r0.xywx)).xyz;
    // 40: mad r1.zw, cb0[10].xxxx, cb0[6].xxxy, v1.xxxy
    r1.zw = ((source[10].xxxx)*(source[6].xxxy)+(v1.xxxy)).zw;
    // 41: mul r1.zw, r1.zzzw, cb0[16].zzzz
    r1.zw = ((r1.zzzw)*(source[16].zzzz)).zw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t6.zwxy, s7, l(0.000000)
    r1.zw = (ArtistNativeSample6((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 43: mad r1.xy, cb0[16].wwww, r1.zwzz, r1.xyxx
    r1.xy = ((source[16].wwww)*(r1.zwzz)+(r1.xyxx)).xy;
    // 44: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t7.xyzw, s8, l(0.000000)
    r1.xyz = (ArtistNativeSample7((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 46: mad r0.xyz, cb0[17].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[17].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 47: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 48: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 49: mul r0.y, r0.y, cb0[17].w
    r0.y = ((r0.yyyy)*(source[17].wwww)).y;
    // 50: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 51: mul_sat r0.y, r0.y, v2.w
    r0.y = (saturate((r0.yyyy)*(v2.wwww))).y;
    // 52: mad r0.z, v1.x, cb0[9].x, cb0[19].x
    r0.z = ((v1.xxxx)*(source[9].xxxx)+(source[19].xxxx)).z;
    // 53: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 54: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 55: lt r0.xz, |r0.xxzx|, l(0.000001, 0.000000, 0.000001, 0.000000)
    r0.xz = (asfloat((uint4)((abs(r0.xxzx))<(float4(0.000001,0.000000,0.000001,0.000000))) * 0xffffffffu)).xz;
    // 56: mul r0.w, r0.w, cb0[19].y
    r0.w = ((r0.wwww)*(source[19].yyyy)).w;
    // 57: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 58: mad r1.x, v1.x, cb0[8].x, cb0[18].y
    r1.x = ((v1.xxxx)*(source[8].xxxx)+(source[18].yyyy)).x;
    // 59: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 60: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 61: or r0.x, r0.x, r1.x
    r0.x = (asfloat(asuint(r0.xxxx) | asuint(r1.xxxx))).x;
    // 62: or r0.x, r0.x, r0.z
    r0.x = (asfloat(asuint(r0.xxxx) | asuint(r0.zzzz))).x;
    // 63: mul r0.z, r1.y, cb0[18].z
    r0.z = ((r1.yyyy)*(source[18].zzzz)).z;
    // 64: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 65: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 66: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 67: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 68: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 69: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 70: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 71: mul r0.y, r0.y, cb0[19].z
    r0.y = ((r0.yyyy)*(source[19].zzzz)).y;
    // 72: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 73: mul r0.y, r0.y, cb0[19].w
    r0.y = ((r0.yyyy)*(source[19].wwww)).y;
    // 74: mul r0.y, r0.y, v2.w
    r0.y = ((r0.yyyy)*(v2.wwww)).y;
    // 75: mad r1.xyzw, r0.yyyy, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r1.xyzw = ((r0.yyyy)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 76: movc r0.xyzw, r0.xxxx, l(-1.000000,1.000000,-1.000000,1.000000), r1.xyzw
    r0.xyzw = ((asuint(r0.xxxx) != 0u) ? (float4(-1.000000,1.000000,-1.000000,1.000000)) : (r1.xyzw)).xyzw;
    // 77: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 78: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 79: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 80: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 81: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 82: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 83: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 84: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 85: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 86: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 87: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 88: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 89: source device depth mapped to centimetre view depth; reconstruction at 91.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 91-94: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 95: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 96: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 97: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 98: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 99: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2955Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2956Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2959Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2960Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2961Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2963Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2964Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2969Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2970Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2974Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2978Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2981Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2986Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2988Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2993Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[9u];
    source[1] = g_ArtistSourceMaterialParameters[10u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].yyyy,g_ArtistSourceMaterialParameters[6u].zzzz,1u);
    source[3].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[3].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[3].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[10].w = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[11].x = ((float4(0.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xy, v1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.x|, |r0.y|
    r0.z = (max(abs(r0.xxxx),abs(r0.yyyy))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.x|, |r0.y|
    r0.w = (min(abs(r0.xxxx),abs(r0.yyyy))).w;
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
    // 13: lt r1.y, |r0.x|, |r0.y|
    r1.y = (asfloat((uint4)((abs(r0.xxxx))<(abs(r0.yyyy))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.x, -r0.x
    r0.w = (asfloat((uint4)((r0.xxxx)<(-(r0.xxxx))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.x, r0.y
    r0.w = (min(r0.xxxx,r0.yyyy)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.x, r0.y
    r1.x = (max(r0.xxxx,r0.yyyy)).x;
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
    // 27: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 28: add r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)+(r0.zzzz)).z;
    // 29: mul r0.yz, r0.yyzy, cb0[3].xxyx
    r0.yz = ((r0.yyzy)*(source[3].xxyx)).yz;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).y;
    // 31: add r1.x, r0.y, cb0[3].z
    r1.x = ((r0.yyyy)+(source[3].zzzz)).x;
    // 32: mul r2.x, r1.x, cb0[6].w
    r2.x = ((r1.xxxx)*(source[6].wwww)).x;
    // 33: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 34: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 35: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 36: mul r0.y, r0.y, cb0[3].w
    r0.y = ((r0.yyyy)*(source[3].wwww)).y;
    // 37: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 38: mul r0.y, r0.y, v3.y
    r0.y = ((r0.yyyy)*(v3.yyyy)).y;
    // 39: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 40: add r0.y, v3.x, cb0[4].x
    r0.y = ((v3.xxxx)+(source[4].xxxx)).y;
    // 41: mad r1.y, r0.y, l(-0.400000), r0.x
    r1.y = ((r0.yyyy)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.xxxx)).y;
    // 42: mul r0.xz, r1.xxyx, cb0[5].xxyx
    r0.xz = ((r1.xxyx)*(source[5].xxyx)).xz;
    // 43: mad r0.y, cb0[5].z, v3.y, r0.z
    r0.y = ((source[5].zzzz)*(v3.yyyy)+(r0.zzzz)).y;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s2, l(-1.000000)
    r0.xy = (ArtistNativeSample1((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
    // 45: max r0.xy, |r0.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (max(abs(r0.xyxx),float4(0.000001,0.000001,0.000000,0.000000))).xy;
    // 46: log r0.xy, r0.xyxx
    r0.xy = (log2(r0.xyxx)).xy;
    // 47: mul r0.xy, r0.xyxx, cb0[5].wwww
    r0.xy = ((r0.xyxx)*(source[5].wwww)).xy;
    // 48: exp r0.xy, r0.xyxx
    r0.xy = (exp2(r0.xyxx)).xy;
    // 49: mad r2.y, r1.y, cb0[7].x, cb0[7].y
    r2.y = ((r1.yyyy)*(source[7].xxxx)+(source[7].yyyy)).y;
    // 50: mad r0.zw, cb0[6].xxxx, r0.xxxy, r2.xxxy
    r0.zw = ((source[6].xxxx)*(r0.xxxy)+(r2.xxxy)).zw;
    // 51: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t2.xyzw, s3, l(-1.000000)
    r2.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 52: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 53: add r3.xyz, -r2.xyzx, r0.zzzz
    r3.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 54: mad r2.xyz, r3.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r2.xyzx)).xyz;
    // 55: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 56: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 57: mul r2.xyz, r2.xyzx, cb0[7].zzzz
    r2.xyz = ((r2.xyzx)*(source[7].zzzz)).xyz;
    // 58: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 59: mul r3.xyz, r2.xyzx, cb0[7].wwww
    r3.xyz = ((r2.xyzx)*(source[7].wwww)).xyz;
    // 60: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 61: mad r2.xyzw, -cb0[7].wwww, r2.xyxy, r0.zzzz
    r2.xyzw = ((-(source[7].wwww))*(r2.xyxy)+(r0.zzzz)).xyzw;
    // 62: mad r2.xyzw, cb0[8].xxxx, r2.xyzw, r3.xyxy
    r2.xyzw = ((source[8].xxxx)*(r2.xyzw)+(r3.xyxy)).xyzw;
    // 63: mul r2.xyzw, r2.xyzw, cb0[0].xyxy
    r2.xyzw = ((r2.xyzw)*(source[0].xyxy)).xyzw;
    // 64: mul r3.x, r1.x, cb0[8].y
    r3.x = ((r1.xxxx)*(source[8].yyyy)).x;
    // 65: mad r3.y, r1.y, cb0[8].z, cb0[8].w
    r3.y = ((r1.yyyy)*(source[8].zzzz)+(source[8].wwww)).y;
    // 66: mad r0.zw, cb0[6].xxxx, r0.xxxy, r3.xxxy
    r0.zw = ((source[6].xxxx)*(r0.xxxy)+(r3.xxxy)).zw;
    // 67: sample_l_indexable(texture2d)(float,float,float,float) r3.xyz, r0.zwzz, t3.xyzw, s4, l(-1.000000)
    r3.xyz = (ArtistNativeSample3((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 68: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 69: add r4.xyz, -r3.xyzx, r0.zzzz
    r4.xyz = ((-(r3.xyzx))+(r0.zzzz)).xyz;
    // 70: mad r3.xyz, r4.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r3.xyzx)).xyz;
    // 71: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 72: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 73: mul r3.xyz, r3.xyzx, cb0[9].xxxx
    r3.xyz = ((r3.xyzx)*(source[9].xxxx)).xyz;
    // 74: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 75: mul r4.xyz, r3.xyzx, cb0[9].yyyy
    r4.xyz = ((r3.xyzx)*(source[9].yyyy)).xyz;
    // 76: dp3 r0.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 77: mad r3.xyzw, -cb0[9].yyyy, r3.xyxy, r0.zzzz
    r3.xyzw = ((-(source[9].yyyy))*(r3.xyxy)+(r0.zzzz)).xyzw;
    // 78: mad r3.xyzw, cb0[9].zzzz, r3.xyzw, r4.xyxy
    r3.xyzw = ((source[9].zzzz)*(r3.xyzw)+(r4.xyxy)).xyzw;
    // 79: mul r3.xyzw, r3.xyzw, cb0[1].xyxy
    r3.xyzw = ((r3.xyzw)*(source[1].xyxy)).xyzw;
    // 80: mul r2.xyzw, r2.xyzw, r3.xyzw
    r2.xyzw = ((r2.xyzw)*(r3.xyzw)).xyzw;
    // 81: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 1.000000, 0.800000), cb0[2].xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,1.000000,0.800000))+(source[2].xxxy)).zw;
    // 82: mul r1.xz, r1.xxyx, cb0[4].yyzy
    r1.xz = ((r1.xxyx)*(source[4].yyzy)).xz;
    // 83: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t4.yzxw, s5, l(-1.000000)
    r0.z = (ArtistNativeSample4((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).z;
    // 84: mul r2.xyzw, r2.xyzw, r0.zzzz
    r2.xyzw = ((r2.xyzw)*(r0.zzzz)).xyzw;
    // 85: mul r3.xyzw, r2.xyzw, cb0[10].yyyy
    r3.xyzw = ((r2.xyzw)*(source[10].yyyy)).xyzw;
    // 86: max r3.xyzw, |r3.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r3.xyzw = (max(abs(r3.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 87: log r3.xyzw, r3.xyzw
    r3.xyzw = (log2(r3.xyzw)).xyzw;
    // 88: mul r3.xyzw, r3.xyzw, cb0[10].zzzz
    r3.xyzw = ((r3.xyzw)*(source[10].zzzz)).xyzw;
    // 89: exp r3.xyzw, r3.xyzw
    r3.xyzw = (exp2(r3.xyzw)).xyzw;
    // 90: mad r1.y, cb0[4].w, v3.y, r1.z
    r1.y = ((source[4].wwww)*(v3.yyyy)+(r1.zzzz)).y;
    // 91: mad r0.xy, cb0[6].xxxx, r0.xyxx, r1.xyxx
    r0.xy = ((source[6].xxxx)*(r0.xyxx)+(r1.xyxx)).xy;
    // 92: mul r0.xy, r0.xyxx, l(1.500000, 0.250000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(1.500000,0.250000,0.000000,0.000000))).xy;
    // 93: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(-1.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 94: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 95: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 96: mul r0.x, r0.x, cb0[6].y
    r0.x = ((r0.xxxx)*(source[6].yyyy)).x;
    // 97: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 98: mul r0.x, r0.x, cb0[6].z
    r0.x = ((r0.xxxx)*(source[6].zzzz)).x;
    // 99: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 100: mad r0.xyzw, r0.xxxx, r2.zwzw, r3.xyzw
    r0.xyzw = ((r0.xxxx)*(r2.zwzw)+(r3.xyzw)).xyzw;
    // 101: mul r0.xyzw, r0.xyzw, cb0[11].xxxx
    r0.xyzw = ((r0.xyzw)*(source[11].xxxx)).xyzw;
    // 102: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 103: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 104: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 105: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 106: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 107: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 108: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 109: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 110: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 111: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 112: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 113: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 114: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 115: source device depth mapped to centimetre view depth; reconstruction at 117.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 117-120: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 121: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 122: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 123: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 124: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 125: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2997Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(input.decalProjection.xy,0.f,0.f);
    source[1]=input.color; // Source decal material color, including particle color modules.
    source[2].x=input.decalProjection.z;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v5.xyxx, v5.wwww
    r1.xy = ((v5.xyxx)/(v5.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v5.w, r0.x
    r0.x = (asfloat((uint4)((v5.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3000Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xyzw, cb2[0].xyxy, l(-1.000000, 1.000000, -1.000000, 1.000000), cb2[0].wzwz
    r0.xyzw = ((passValues[0].xyxy)*(float4(-1.000000,1.000000,-1.000000,1.000000))+(passValues[0].wzwz)).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 4: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 6: div r1.xy, v3.xyxx, v3.wwww
    r1.xy = ((v3.xyxx)/(v3.wwww)).xy;
    // 7: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 8: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 9: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 10: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 11: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 12: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 13: source device depth mapped to centimetre view depth; reconstruction at 15.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 15-18: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 19: ge r0.x, v3.w, r0.x
    r0.x = (asfloat((uint4)((v3.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 20: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 21: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 22: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 23: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3007Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].yyyy,g_ArtistSourceMaterialParameters[5u].zzzz,1u);
    source[2] = ArtistNativeAppend(float4(0.100000001, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[3] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww),float4(0.0, 0.0, 0.0, 0.0),1u);
    source[4].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[5].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww)).x;
    source[5].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mov r0.y, cb0[4].z
    r0.y = (source[4].zzzz).y;
    // 2: mov r0.xz, l(0,0,1.000000,0)
    r0.xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 3: mad r1.xyzw, v1.zwzw, cb0[0].xyxy, r0.yxxy
    r1.xyzw = ((v1.zwzw)*(source[0].xyxy)+(r0.yxxy)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r1.xy, v1.zwzz, cb0[0].xyxx
    r1.xy = ((v1.zwzz)*(source[0].xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: add r2.xy, r0.xyxx, -r1.xxxx
    r2.xy = ((r0.xyxx)+(-(r1.xxxx))).xy;
    // 9: mul r1.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: mov r1.z, l(0)
    r1.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 11: add r1.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 12: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 13: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 14: div r0.xy, r1.xyxx, r0.xxxx
    r0.xy = ((r1.xyxx)/(r0.xxxx)).xy;
    // 15: mad r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: mov r0.w, v3.x
    r0.w = (v3.xxxx).w;
    // 17: mul r0.xy, r0.zwzz, r0.xyxx
    r0.xy = ((r0.zwzz)*(r0.xyxx)).xy;
    // 18: mov r1.x, v3.x
    r1.x = (v3.xxxx).x;
    // 19: mov r1.y, l(0.200000)
    r1.y = (float4(0.200000,0.200000,0.200000,0.200000)).y;
    // 20: mul r0.zw, r0.xxxy, r1.xxxy
    r0.zw = ((r0.xxxy)*(r1.xxxy)).zw;
    // 21: mad r0.zw, v1.zzzw, cb0[1].xxxy, r0.zzzw
    r0.zw = ((v1.zzzw)*(source[1].xxxy)+(r0.zzzw)).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t2.zwxy, s2, l(0.000000)
    r0.zw = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 23: add r2.xyzw, r0.zwzw, cb0[5].wwww
    r2.xyzw = ((r0.zwzw)+(source[5].wwww)).xyzw;
    // 24: mul r2.xyzw, r2.xyzw, cb0[6].xxxx
    r2.xyzw = ((r2.xyzw)*(source[6].xxxx)).xyzw;
    // 25: mul r2.xyzw, r2.xyzw, v3.zzzz
    r2.xyzw = ((r2.xyzw)*(v3.zzzz)).xyzw;
    // 26: mul r2.xyzw, r2.xyzw, l(6.283185, 6.283185, 6.283185, 6.283185)
    r2.xyzw = ((r2.xyzw)*(float4(6.283185,6.283185,6.283185,6.283185))).xyzw;
    // 27: sincos r2.xyzw, null, r2.xyzw
    r2.xyzw = (sin(r2.xyzw)).xyzw;
    // 28: max r2.xyzw, |r2.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r2.xyzw = (max(abs(r2.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 29: log r2.xyzw, r2.xyzw
    r2.xyzw = (log2(r2.xyzw)).xyzw;
    // 30: mul r2.xyzw, r2.xyzw, cb0[6].yyyy
    r2.xyzw = ((r2.xyzw)*(source[6].yyyy)).xyzw;
    // 31: exp r2.xyzw, r2.xyzw
    r2.xyzw = (exp2(r2.xyzw)).xyzw;
    // 32: mul_sat r2.xyzw, r2.xyzw, cb0[6].zzzz
    r2.xyzw = (saturate((r2.xyzw)*(source[6].zzzz))).xyzw;
    // 33: add r0.zw, v1.zzzw, cb0[2].xxxy
    r0.zw = ((v1.zzzw)+(source[2].xxxy)).zw;
    // 34: mad r0.xy, r1.xyxx, r0.xyxx, r0.zwzz
    r0.xy = ((r1.xyxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 35: add r0.xy, r0.xyxx, cb0[3].xyxx
    r0.xy = ((r0.xyxx)+(source[3].xyxx)).xy;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.xy = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 37: max r0.xyzw, |r0.xyxy|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyxy),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 38: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 39: mul r0.xyzw, r0.xyzw, cb0[5].zzzz
    r0.xyzw = ((r0.xyzw)*(source[5].zzzz)).xyzw;
    // 40: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 41: mul r0.xyzw, r2.xyzw, r0.xyzw
    r0.xyzw = ((r2.xyzw)*(r0.xyzw)).xyzw;
    // 42: mul r0.xyzw, r0.xyzw, cb0[6].wwww
    r0.xyzw = ((r0.xyzw)*(source[6].wwww)).xyzw;
    // 43: mad r1.x, v1.z, l(2.000000), l(-1.000000)
    r1.x = ((v1.zzzz)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 44: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 45: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 46: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 47: mul r1.y, r1.y, cb0[7].x
    r1.y = ((r1.yyyy)*(source[7].xxxx)).y;
    // 48: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 49: mul r1.y, r1.y, l(3.000000)
    r1.y = ((r1.yyyy)*(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 50: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 51: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 52: mul_sat r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = (saturate((r0.xyzw)*(r1.xxxx))).xyzw;
    // 53: mul r0.xyzw, r0.xyzw, v2.wwww
    r0.xyzw = ((r0.xyzw)*(v2.wwww)).xyzw;
    // 54: mul r0.xyzw, r0.xyzw, cb0[7].yyyy
    r0.xyzw = ((r0.xyzw)*(source[7].yyyy)).xyzw;
    // 55: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 56: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 57: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 58: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 59: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 60: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 61: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 62: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 63: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 64: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 65: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 66: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 67: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 68: source device depth mapped to centimetre view depth; reconstruction at 70.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 70-73: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 74: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 75: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 76: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 77: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 78: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif
